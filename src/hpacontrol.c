#include "hpacontrol.h"

#include "hpa.h"
#include "devconfig.h"
#include "sec.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ata.h>

extern int errno;

#define STATUS_CMD				"status"

#define SETMAX_CMD				"setmax"
#define MAXPASSWORD_CMD			"maxpassword"
#define MAXLOCK_CMD				"maxlock"
#define MAXUNLOCK_CMD			"maxunlock"
#define MAXFREEZELOCK_CMD		"maxfreezelock"

#define SETSECPASSWORD_CMD		"secpassword"
#define SECUNLOCK_CMD			"secunlock"
#define SECERASE_CMD			"secerase"
#define SECFREEZELOCK_CMD		"secfreezelock"
#define SECDISABLEPASSWORD_CMD	"secdisablepassword"

//TODO 	Determine if all the reinitChannel calls are necessary.  I know that it is needed for SET MAX, but I am unsure 
//		on the others.
//TODO 	Under the DEVICE CONFIGURATION command there is a bit that says if the drive is allowed to report HPA support.  
//		I guess to hide HPA support.
//TODO	Fix/Test all 48bit LBA support
//TODO	Find a solution to the HPA problem where calls can not be immediately preceded by a call to 
//		READ_NATIVE_MAX_ADDRESS.  If I don't use my 'status option between calls, all password, lock, and unlock calls
//		seem to work correctly.  'status' adds calls to READ_NATIVE_MAX_ADDRESS which messes them up.  Unfortunately,
//		not all drives support the NOP command that could have been used to fix this problem.

void sectorsToReadable(u_int64_t uiSectors, float* p_fSize, char* p_chrUnits) {
	//TODO	This code is probably more complicated than it really needs to be... can probably be done with shift 
	//		operators.  See mmls.c from the SleuthKit.
	if (p_fSize && p_chrUnits) {
		*p_fSize = uiSectors*4.54747350888e-13; // (float)512/powf(2, 50)
		*p_chrUnits = 'P';
		if (*p_fSize  < 1) { 
			*p_fSize = uiSectors*4.65661287306e-10; // (float)512/powf(2, 40)
			*p_chrUnits = 'T';
			if (*p_fSize  < 1) {
				*p_fSize = uiSectors*4.76837158203e-07; // (float)512/powf(2, 30)
				*p_chrUnits = 'G';
				if (*p_fSize  < 1) {
					*p_fSize = uiSectors*0.00048828125; // (float)512/powf(2, 20)
					*p_chrUnits = 'M';
					if (*p_fSize  < 1) {
						*p_fSize = uiSectors*0.5; // (float)512/powf(2, 10)
						*p_chrUnits = 'K';
						if (*p_fSize  < 1) { 
							*p_fSize = uiSectors*512;
							*p_chrUnits = 'B';
						}
					}
				}
			}
		}
	} else {
		ERR("Invalid pointer, unable to convert sectors to readable format");
	}
}

HPASTATUS sendNop(struct dev_container* pDevice) {
	HPASTATUS rv = HPAERROR;

	if (pDevice) {
		if (pDevice->bNOPSupport == TRUE) {
			struct ata_cmd iocmd;
			bzero(&iocmd, sizeof(struct ata_cmd));
			
			iocmd.channel = pDevice->iChannelID;
			iocmd.device = pDevice->iDeviceID;
			iocmd.cmd = ATAREQUEST;
			iocmd.u.request.flags = ATA_CMD_CONTROL;
			iocmd.u.request.u.ata.command = ATA_NOP;
			iocmd.u.request.timeout = 5;
			
			ioctl(pDevice->fdATAController, IOCATA, &iocmd);
			rv = HPASUCCESS;
		} else {
			DEBUGERR("sendNop", "Device does not support NOP");
		}
	} else {
		DEBUGERR("sendNop", "Invalid dev_container pointer");
	}
	
	return rv;
}

HPASTATUS reinitChannel(struct dev_container* pDevice) {
	HPASTATUS rv = HPAERROR;

	if (pDevice) {
		struct ata_cmd iocmd;
		bzero(&iocmd, sizeof(struct ata_cmd));
		
		iocmd.channel = pDevice->iChannelID;
		iocmd.cmd = ATAREINIT;
		if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
			rv = HPASUCCESS;
		} else {
			DEBUGERR("reinitChannel", strerror(errno));
		}
	} else {
		DEBUGERR("reinitChannel", "Invalid dev_container pointer");
	}
	
	return rv;
}

HPASTATUS getDevice(struct dev_container* pDevice) {
	HPASTATUS rv = HPAERROR;
	
	if (pDevice) { 
		if ((pDevice->fdATAController = open("/dev/ata", O_RDWR)) != -1) {
			if (pDevice->iDeviceID == 0 || pDevice->iDeviceID == 1) {
				struct ata_cmd iocmd;
				bzero(&iocmd, sizeof(struct ata_cmd));
				
				iocmd.channel = pDevice->iChannelID;
				iocmd.device = pDevice->iDeviceID;
				iocmd.cmd = ATAGPARM;
				if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
					pDevice->bHPASupport = (iocmd.u.param.params[pDevice->iDeviceID].support.command1 & ATA_SUPPORT_PROTECTED ? TRUE : FALSE);
					pDevice->bHPAEnabled = (iocmd.u.param.params[pDevice->iDeviceID].enabled.command1 & ATA_SUPPORT_PROTECTED ? TRUE : FALSE);
					pDevice->bHPASecuritySupport = (iocmd.u.param.params[pDevice->iDeviceID].support.command2 & ATA_SUPPORT_MAXSECURITY ? TRUE : FALSE);
					pDevice->bHPASecurityEnabled = (iocmd.u.param.params[pDevice->iDeviceID].enabled.command2 & ATA_SUPPORT_MAXSECURITY ? TRUE : FALSE);
					pDevice->bLBA48Support = (iocmd.u.param.params[pDevice->iDeviceID].support.command2 & ATA_SUPPORT_ADDRESS48 ? TRUE : FALSE);
					pDevice->bNOPSupport = (iocmd.u.param.params[pDevice->iDeviceID].support.command1 & ATA_SUPPORT_NOP ? TRUE : FALSE);
					if (pDevice->bLBA48Support == TRUE) {
						pDevice->uiMaxUserAddress =	(((u_int64_t)iocmd.u.param.params[pDevice->iDeviceID].lba_size48_1) |
													((u_int64_t)iocmd.u.param.params[pDevice->iDeviceID].lba_size48_2 << 16) |
													((u_int64_t)iocmd.u.param.params[pDevice->iDeviceID].lba_size48_3 << 32) |
													((u_int64_t)iocmd.u.param.params[pDevice->iDeviceID].lba_size48_4 << 48)) - 1;
					} else {
						pDevice->uiMaxUserAddress =	(((u_int64_t)iocmd.u.param.params[pDevice->iDeviceID].lba_size_1) |
													((u_int64_t)iocmd.u.param.params[pDevice->iDeviceID].lba_size_2 << 16)) - 1;
					}
					bzero(&pDevice->cstrDevName, 32);
					strcpy(pDevice->cstrDevName, iocmd.u.param.name[pDevice->iDeviceID]);
					bzero(&pDevice->cstrModel, 41);
					strncpy(pDevice->cstrModel, iocmd.u.param.params[pDevice->iDeviceID].model, 40);
					bzero(&pDevice->cstrSerial, 21);
					strncpy(pDevice->cstrSerial, iocmd.u.param.params[pDevice->iDeviceID].serial, 20);
					bzero(&pDevice->cstrRevision, 9);
					strncpy(pDevice->cstrRevision, iocmd.u.param.params[pDevice->iDeviceID].revision, 8);
					pDevice->bDCOSupport = (iocmd.u.param.params[pDevice->iDeviceID].support.command2 & ATA_SUPPORT_OVERLAY ? TRUE : FALSE);
					pDevice->bDCOEnabled = (iocmd.u.param.params[pDevice->iDeviceID].enabled.command2 & ATA_SUPPORT_OVERLAY ? TRUE : FALSE);
					pDevice->bSecSupport = (iocmd.u.param.params[pDevice->iDeviceID].support.command1 & ATA_SUPPORT_SECURITY ? TRUE : FALSE);
					pDevice->bSecEnabled = (iocmd.u.param.params[pDevice->iDeviceID].enabled.command1 & ATA_SUPPORT_SECURITY ? TRUE : FALSE);
					pDevice->uiMasterPasswordRev = iocmd.u.param.params[pDevice->iDeviceID].master_passwd_revision;
					pDevice->uiEraseTime = iocmd.u.param.params[pDevice->iDeviceID].erase_time;
					pDevice->uiEnhancedEraseTime = iocmd.u.param.params[pDevice->iDeviceID].enhanced_erase_time;
					pDevice->bSecMaxSecurity = ((iocmd.u.param.params[pDevice->iDeviceID].security_status >> 8) & 0x1 ? TRUE : FALSE);
					pDevice->bSecEnhancedEraseSupport = ((iocmd.u.param.params[pDevice->iDeviceID].security_status >> 5) & 0x1 ? TRUE : FALSE);
					pDevice->bSecCountExpired = ((iocmd.u.param.params[pDevice->iDeviceID].security_status >> 4) & 0x1 ? TRUE : FALSE);
					pDevice->bSecFrozen = ((iocmd.u.param.params[pDevice->iDeviceID].security_status >> 3) & 0x1 ? TRUE : FALSE);
					pDevice->bSecLocked = ((iocmd.u.param.params[pDevice->iDeviceID].security_status >> 2) & 0x1 ? TRUE : FALSE);
					
					rv = HPASUCCESS;
				} else {
					DEBUGERR("getDeviceParams", strerror(errno));
				}
			} else {
				DEBUGERR("getDeviceParams", "Invalid device ID");
			}
		} else {
			DEBUGERR("getDevice", strerror(errno));
		}
	} else {
		DEBUGERR("getDevice", "Invalid dev_container pointer");
	}
	
	return rv;
}

HPASTATUS closeDevice(struct dev_container* pDevice) {
	HPASTATUS rv = HPAERROR;
	
	if (pDevice) {
		if (pDevice->fdATAController) {
			if (close(pDevice->fdATAController) != -1) {
				rv = HPASUCCESS;
			} else {
				DEBUGERR("closeDevice", strerror(errno));
			}
		}
		bzero(pDevice, sizeof(struct dev_container));
	} else {
		DEBUGERR("closeDevice", "Invalid dev_container pointer");
	}
	
	return rv;
}

void usage(char* cstrCmd) {
	if (cstrCmd) {
		if (strcmp(cstrCmd, SETMAX_CMD) == 0) {
			printf("usage: %s <channel> <device> <address> <volatile>\n", SETMAX_CMD);
		} else if (strcmp(cstrCmd, MAXPASSWORD_CMD) == 0) {
			printf("usage: %s <channel> <device> <password>\n", MAXPASSWORD_CMD);
		} else if (strcmp(cstrCmd, MAXLOCK_CMD) == 0) {
			printf("usage: %s <channel> <device>\n", MAXLOCK_CMD);
		} else if (strcmp(cstrCmd, MAXFREEZELOCK_CMD) == 0) {
			printf("usage: %s <channel> <device>\n", MAXFREEZELOCK_CMD);
		} else if (strcmp(cstrCmd, MAXUNLOCK_CMD) == 0) {
			printf("usage: %s <channel> <device> <password>\n", MAXUNLOCK_CMD);
		} else if (strcmp(cstrCmd, SETSECPASSWORD_CMD) == 0) {
			printf("usage: %s <channel> <device> <password> <master> <maxsecurity> <revision>\n", SETSECPASSWORD_CMD);
		} else if (strcmp(cstrCmd, SECUNLOCK_CMD) == 0) {
			printf("usage: %s <channel> <device> <password> <master>\n", SECUNLOCK_CMD);
		} else if (strcmp(cstrCmd, SECERASE_CMD) == 0) {
			printf("usage: %s <channel> <device> <password> <master> <enhanced>\n", SECERASE_CMD);
		} else if (strcmp(cstrCmd, SECFREEZELOCK_CMD) == 0) {
			printf("usage: %s <channel> <device>\n", SECFREEZELOCK_CMD);
		} else if (strcmp(cstrCmd, SECDISABLEPASSWORD_CMD) == 0) {
			printf("usage: %s <channel> <device> <password> <master>\n", SECDISABLEPASSWORD_CMD);
		} else {
			MSG("usage: hpacontrol <command> <channel> <device>");
		}
	} else {
		MSG("usage: hpacontrol <command> <channel> <device>");
	}
}

int main(int argc, char** argv) {
	int rv = EXIT_FAILURE;

	char* cstrCmd = NULL;
	char* cstrChannel = NULL;
	char* cstrDevice = NULL;

	while (getopt(argc, argv, "") != -1) {
		switch (optopt) {
			case '?':
			default:
				break;
		}
	}
	
	if (optind < argc) { 
		cstrCmd = argv[optind];
		optind++;

		if (optind < argc) {
			cstrChannel = argv[optind];
			optind++;

			if (optind < argc) {
				cstrDevice = argv[optind];
				optind++;
			}
		}
	}

	if (cstrCmd && cstrChannel && cstrDevice) {
		struct dev_container device;
		bzero(&device, sizeof(struct dev_container));
		device.iChannelID = strtoul(cstrChannel, NULL, 10);
		device.iDeviceID = strtoul(cstrDevice, NULL, 10);
		
		if (getDevice(&device) != HPAERROR) {
			printf("ATA channel %d, %s, device %s:\n\n", device.iChannelID, (device.iDeviceID ? "Slave" : "Master"), device.cstrDevName);
			if (strcmp(cstrCmd, STATUS_CMD) == 0) {
				printf("Model Number\t\t\t\t\t%s\n", device.cstrModel);
				printf("Serial Number\t\t\t\t\t%s\n", device.cstrSerial);
				printf("Firmware Revision\t\t\t\t%s\n", device.cstrRevision);
				
				printf("\nDevice Configuration Overlay\n");
				printf("\tDevice Configuration Overlay Support\t%s / %s\n", (device.bDCOSupport == TRUE ? "supported" : "unsupported"), (device.bDCOEnabled == TRUE ? "enabled" : "disabled"));
				if (device.bDCOSupport == TRUE) {
					BOOL bAllowHPAReporting = FALSE;
					if (devconfigGetAllowHPAReporting(&device, &bAllowHPAReporting) != HPAERROR) {
						printf("\tHost Protected Area Reporting\t\t%s\n", (bAllowHPAReporting == TRUE ? "allowed" : "not allowed"));
					} else {
						ERR("Failed to determine if HPA reporting is allowed");
					}
					BOOL bAllowLBA48Reporting = FALSE;
					if (devconfigGetAllowLBA48Reporting(&device, &bAllowLBA48Reporting) != HPAERROR) {
						printf("\t48-bit Address Reporting\t\t%s\n", (bAllowLBA48Reporting == TRUE ? "allowed" : "not allowed"));
					} else {
						ERR("Failed to determine if 48-bit Address reporting is allowed");
					}
					BOOL bAllowSecurityReporting = FALSE;
					if (devconfigGetAllowSecurityReporting(&device, &bAllowSecurityReporting) != HPAERROR) {
						printf("\tSecurity Mode Reporting\t\t\t%s\n", (bAllowLBA48Reporting == TRUE ? "allowed" : "not allowed"));
					} else {
						ERR("Failed to determine if Security Mode reporting is allowed");
					}
				}
				
				printf("\nHost Protected Area\n");
				printf("\tHost Protected Area Support\t\t%s / %s\n", (device.bHPASupport == TRUE ? "supported" : "unsupported"), (device.bHPAEnabled == TRUE ? "enabled" : "disabled"));
				if (device.bHPASupport == TRUE) {
					printf("\tHost Protected Area Security Support\t%s / %s\n", (device.bHPASecuritySupport == TRUE ? "supported" : "unsupported"), (device.bHPASecurityEnabled == TRUE ? "enabled" : "disabled"));
					float fSize = 0;
					char chrUnits = '?';
					sectorsToReadable(device.uiMaxUserAddress, &fSize, &chrUnits);
					printf("\tMax User Addressable Sector\t\t%llu (%.1f%c)\n", device.uiMaxUserAddress, fSize, chrUnits);

					u_int64_t uiNativeMaxAddress = 0;
					if (readNativeMaxAddress(&device, &uiNativeMaxAddress) != HPAERROR) {
						fSize = 0;
						chrUnits = '?';
						sectorsToReadable(uiNativeMaxAddress, &fSize, &chrUnits);
						printf("\tMax Native Addressable Sector\t\t%llu (%.1f%c)\n", uiNativeMaxAddress, fSize, chrUnits);
						if (uiNativeMaxAddress > device.uiMaxUserAddress) {
							u_int64_t uiHPALen = uiNativeMaxAddress - device.uiMaxUserAddress;
							fSize = 0;
							chrUnits = '?';
							sectorsToReadable(uiHPALen, &fSize, &chrUnits);
							printf("\tHost Protected Area Size\t\t%llu (%.1f%c)\n", uiHPALen, fSize, chrUnits);
						}
					} else {
						ERR("Failed to read native maximum address");
					}
				}
				
				printf("\nSecurity Mode\n");
				printf("\tSecurity Support\t\t\t%s / %s\n", (device.bSecSupport == TRUE ? "supported" : "unsupported"), (device.bSecEnabled == TRUE ? "enabled" : "disabled"));
				if (device.bSecSupport == TRUE) {
					printf("\tMaster Password Revision Code\t\t0x%x\n", device.uiMasterPasswordRev);
					printf("\tSecurity Level\t\t\t\t%s\n", (device.bSecMaxSecurity == TRUE ? "maximum" : "high"));
					printf("\tFrozen\t\t\t\t\t%s\n", (device.bSecFrozen == TRUE ? "true" : "false"));
					printf("\tLocked\t\t\t\t\t%s\n", (device.bSecLocked == TRUE ? "true" : "false"));
					printf("\tCount Expired\t\t\t\t%s\n", (device.bSecCountExpired == TRUE ? "true" : "false"));
					printf("\tErase Time\t\t\t\t");
					if (device.uiEraseTime == 0) {
						printf("unspecified");
					} else if (device.uiEraseTime < 255) {
						printf("%umin (%uhr)", device.uiEraseTime * 2, (device.uiEraseTime * 2) / 60);
					} else {
						printf(">508min (8.5hr)");
					}
					printf("\n");
					printf("\tEnhanced Erase Support\t\t\t%s\n", (device.bSecEnhancedEraseSupport == TRUE ? "supported" : "unsupported"));
					if (device.bSecEnhancedEraseSupport == TRUE) {
						printf("\t\tEnhanced Erase Time\t\t");
						if (device.uiEnhancedEraseTime == 0) {
							printf("unspecified");
						} else if (device.uiEnhancedEraseTime < 255) {
							printf("%umin (%uhr)", device.uiEnhancedEraseTime * 2, (device.uiEnhancedEraseTime * 2) / 60);
						} else {
							printf(">508min (8.5hr)");
						}
						printf("\n");
					}
				}
				rv = EXIT_SUCCESS;
			} else if (strcmp(cstrCmd, SETMAX_CMD) == 0) {
				if (optind < argc) {
					u_int64_t uiNewMaxAddress = strtoull(argv[optind], NULL, 10);
					optind++;

					if (optind < argc) {
						BOOL bVolatile = (strtol(argv[optind], NULL, 10) > 0 ? TRUE : FALSE);
						optind++;

						if (setMaxAddress(&device, uiNewMaxAddress, bVolatile) != HPAERROR) {
							reinitChannel(&device);
							MSG("Set Max Address Successful");
							rv = EXIT_SUCCESS;
						} else {
							ERR("Set Max Address Failed");
						}
					} else {
						usage(cstrCmd);
					}
				} else {
					usage(cstrCmd);
				}
			} else if (strcmp(cstrCmd, MAXPASSWORD_CMD) == 0) {
				char* cstrPassword = NULL;
				if (optind < argc) {
					cstrPassword = argv[optind];
					optind++;
				}

				if (setMaxSetPassword(&device, cstrPassword) != HPAERROR) {
					reinitChannel(&device);
					MSG("Set Max Set Password Successful");
					rv = EXIT_SUCCESS;
				} else {
					ERR("Set Max Set Password Failed");
				}
			} else if (strcmp(cstrCmd, MAXLOCK_CMD) == 0) {
				if (setMaxLock(&device) != HPAERROR) {
					reinitChannel(&device);
					MSG("Set Max Lock Successful");
					rv = EXIT_SUCCESS;					
				} else {
					ERR("Set Max Lock Failed");
				}
			} else if (strcmp(cstrCmd, MAXFREEZELOCK_CMD) == 0) {
				if (setMaxFreezeLock(&device) != HPAERROR) {
					reinitChannel(&device);
					MSG("Set Max Freeze Lock Successful");
					rv = EXIT_SUCCESS;					
				} else {
					ERR("Set Max Freeze Lock Failed");
				}
			} else if (strcmp(cstrCmd, MAXUNLOCK_CMD) == 0) {
				char* cstrPassword = NULL;
				if (optind < argc) {
					cstrPassword = argv[optind];
					optind++;
				}

				if (setMaxUnlock(&device, cstrPassword) != HPAERROR) {
					reinitChannel(&device);
					MSG("Set Max Unlock Successful");
					rv = EXIT_SUCCESS;					
				} else {
					ERR("Set Max Unlock Failed");
				}
			} else if (strcmp(cstrCmd, SETSECPASSWORD_CMD) == 0) {
				char* cstrPassword = NULL;
				if (optind < argc) {
					cstrPassword = argv[optind];
					optind++;
					DEBUGMSG1ARG("main", "Password =", cstrPassword);
				}
				
				if (optind < argc) {
					BOOL bMaster = (strtol(argv[optind], NULL, 10) > 0 ? TRUE : FALSE);
					optind++;
					DEBUGMSG1ARG("main", "Master =", (bMaster == TRUE ? "TRUE" : "FALSE"));
					
					if (optind < argc) {
						BOOL bMaxSecurity = (strtol(argv[optind], NULL, 10) > 0 ? TRUE : FALSE);
						optind++;
						DEBUGMSG1ARG("main", "Max Security =", (bMaxSecurity == TRUE ? "TRUE" : "FALSE"));
						
						if (optind < argc) {
							u_int16_t uiMasterPasswordRev = strtoul(argv[optind], NULL, 0);
							optind++;
							
							if (securitySetPassword(&device, cstrPassword, bMaster, bMaxSecurity, uiMasterPasswordRev) != HPAERROR) {
								MSG("Security Set Password Successful");
								rv = EXIT_SUCCESS;
							} else {
								ERR("Security Set Password Failed");
							}
						} else {
							usage(cstrCmd);
						}
					} else {
						usage(cstrCmd);
					}
				} else {
					usage(cstrCmd);
				}
			} else if (strcmp(cstrCmd, SECUNLOCK_CMD) == 0) {
				char* cstrPassword = NULL;
				if (optind < argc) {
					cstrPassword = argv[optind];
					optind++;
					DEBUGMSG1ARG("main", "Password =", cstrPassword);
				}
				
				if (optind < argc) {
					BOOL bMaster = (strtol(argv[optind], NULL, 10) > 0 ? TRUE : FALSE);
					optind++;
					DEBUGMSG1ARG("main", "Master =", (bMaster == TRUE ? "TRUE" : "FALSE"));
					
					if (securityUnlock(&device, cstrPassword, bMaster) != HPAERROR) {
						MSG("Security Unlock Successful");
						rv = EXIT_SUCCESS;
					} else {
						ERR("Security Unlock Failed");
					}
				} else {
					usage(cstrCmd);
				}
			} else if (strcmp(cstrCmd, SECERASE_CMD) == 0) {
				char* cstrPassword = NULL;
				if (optind < argc) {
					cstrPassword = argv[optind];
					optind++;
					DEBUGMSG1ARG("main", "Password =", cstrPassword);
				}
				
				if (optind < argc) {
					BOOL bMaster = (strtol(argv[optind], NULL, 10) > 0 ? TRUE : FALSE);
					optind++;
					DEBUGMSG1ARG("main", "Master =", (bMaster == TRUE ? "TRUE" : "FALSE"));
					
					if (optind < argc) {
						BOOL bEnhancedErase = (strtol(argv[optind], NULL, 10) > 0 ? TRUE : FALSE);
						optind++;
						DEBUGMSG1ARG("main", "Enhanced Erase =", (bEnhancedErase == TRUE ? "TRUE" : "FALSE"));
						
						printf("WARNING! This command will destroy all data on the target drive. Do you want to continue? (y/n) ");
						char charResponse = 0;
						scanf("%1c", &charResponse);
						
						if (charResponse == 'y' || charResponse == 'Y') {
							if (securityErase(&device, cstrPassword, bMaster, bEnhancedErase) != HPAERROR) {
								MSG("Security Erase Successful");
								rv = EXIT_SUCCESS;
							} else {
								ERR("Security Erase Failed");
							}
						}
						
					} else {
						usage(cstrCmd);
					}
				} else {
					usage(cstrCmd);
				}
			} else if (strcmp(cstrCmd, SECFREEZELOCK_CMD) == 0) {
				if (securityFreezeLock(&device) != HPAERROR) {
					MSG("Security Freeze Lock Successful");
					rv = EXIT_SUCCESS;
				} else {
					ERR("Security Freeze Lock Failed");
				}
			} else if (strcmp(cstrCmd, SECDISABLEPASSWORD_CMD) == 0) {
				char* cstrPassword = NULL;
				if (optind < argc) {
					cstrPassword = argv[optind];
					optind++;
				}
				
				if (optind < argc) {
					BOOL bMaster = (strtol(argv[optind], NULL, 10) > 0 ? TRUE : FALSE);
					optind++;
					
					if (securityDisablePassword(&device, cstrPassword, bMaster) != HPAERROR) {
						MSG("Security Disable Password Successful");
						rv = EXIT_SUCCESS;
					} else {
						ERR("Security Disable Password Failed");
					}
				} else {
					usage(cstrCmd);
				}
			} else {
				usage(cstrCmd);
			}
		} else {
			ERR("Failed opening device");
		}
		closeDevice(&device);
	} else {
		usage(cstrCmd);
	}
	
	return rv;
}
