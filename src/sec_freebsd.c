// Copyright 2007 Matthew A. Kucenski
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifdef __FreeBSD__

#include "sec.h"

#include <stdio.h>
#include <string.h>
#include <sys/ata.h>

extern int errno;

HPASTATUS securitySetPassword(struct dev_container* pDevice, char* cstrPassword, BOOL bMasterPassword, BOOL bMaxSecurity, u_int16_t uiMasterPasswordRev) {
	HPASTATUS rv = HPAERROR;

	if (pDevice) {
		if (pDevice->bSecSupport == TRUE) {
			char data[512];
			bzero(&data, 512);
			data[0] = (bMasterPassword == TRUE ? 1 : 0);
			data[1] = (bMaxSecurity == TRUE ? 1 : 0);
			if (cstrPassword) {
				if (strlen(cstrPassword) > 32) {
					DEBUGERR("securitySetPassword", "Password too long (> 32 characters), truncated");
				}
				strncpy(&data[2], cstrPassword, 32);
			}
			if (bMasterPassword == TRUE) {
				data[34] = uiMasterPasswordRev & 0xff;
				data[35] = (uiMasterPasswordRev >> 8) & 0xff;
			}

			struct ata_cmd iocmd;
			bzero(&iocmd, sizeof(struct ata_cmd));

			//	Description
			//		This command transfers 512 bytes of data from the host. Table 39 
			//		defines the content of this information. The data transferred controls 
			//		the function of this command. Table 40 defines the interaction of the 
			//		identifier and security level bits. 
			//
			//		The revision code field shall be returned in the IDENTIFY DEVICE data 
			//		word 92. The valid revision codes are 0001h through FFFEh. A value of 
			//		0000h or FFFFh indicates that the Master Password Revision Code is not 
			//		supported.
			iocmd.u.request.u.ata.command = SECURITY_SET_PASSWORD;

			iocmd.channel = pDevice->iChannelID;
			iocmd.device = pDevice->iDeviceID;
			iocmd.cmd = ATAREQUEST;
			iocmd.u.request.flags = ATA_CMD_WRITE;
			iocmd.u.request.data = data;
			iocmd.u.request.count = 512;
			iocmd.u.request.timeout = 5;
			
			if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
				rv = HPASUCCESS;
			} else {
				if (errno != 0) {
					DEBUGERR("securitySetPassword", strerror(errno));
				}
				if (((iocmd.u.request.error >> 2) & 0x1) != 0) {
					DEBUGERR("securitySetPassword", "ATA command returned ABRT error");
				}
			}
		} else {
			DEBUGERR("securitySetPassword", "No security support");
		}
	} else {
		DEBUGERR("securitySetPassword", "Invalid dev_container pointer");
	}
	
	return rv;
}

HPASTATUS securityUnlock(struct dev_container* pDevice, char* cstrPassword, BOOL bMasterPassword) {
	HPASTATUS rv = HPAERROR;

	if (pDevice) {
		if (pDevice->bSecSupport == TRUE) {
			char data[512];
			bzero(&data, 512);
			data[0] = (bMasterPassword == TRUE ? 1 : 0);
			if (cstrPassword) {
				if (strlen(cstrPassword) > 32) {
					DEBUGERR("securityUnlock", "Password too long (> 32 characters), truncated");
				}
				strncpy(&data[2], cstrPassword, 32);
			}

			struct ata_cmd iocmd;
			bzero(&iocmd, sizeof(struct ata_cmd));

			//	Description
			//		This command transfers 512 bytes of data from the host. Table 37 
			//		defines the content of this information. 
			//
			//		If the Identifier bit is set to Master and the device is in high 
			//		security level, then the password supplied shall be compared with the 
			//		stored Master password. If the device is in maximum security level 
			//		then the unlock shall be rejected. 
			//
			//		If the Identifier bit is set to user then the device shall compare the 
			//		supplied password with the stored User password. 
			//
			//		If the password compare fails then the device shall return command 
			//		aborted to the host and decrements the unlock counter. This counter 
			//		shall be initially set to five and shall be decremented for each 
			//		password mismatch when SECURITY UNLOCK is issued and the device is 
			//		locked. When this counter reaches zero then SECURITY UNLOCK and 
			//		SECURITY ERASE UNIT commands shall be command aborted until a power-on 
			//		reset or a hardware reset. SECURITY UNLOCK commands issued when the 
			//		device is unlocked have no effect on the unlock counter.			
			iocmd.u.request.u.ata.command = SECURITY_UNLOCK;

			iocmd.channel = pDevice->iChannelID;
			iocmd.device = pDevice->iDeviceID;
			iocmd.cmd = ATAREQUEST;
			iocmd.u.request.flags = ATA_CMD_WRITE;
			iocmd.u.request.data = data;
			iocmd.u.request.count = 512;
			iocmd.u.request.timeout = 5;
			
			if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
				rv = HPASUCCESS;
			} else {
				DEBUGERR("securityUnlock", strerror(errno));
			}
		} else {
			DEBUGERR("securityUnlock", "No security support");
		}
	} else {
		DEBUGERR("securityUnlock", "Invalid dev_container pointer");
	}
	
	return rv;
}

HPASTATUS securityErase(struct dev_container* pDevice, char* cstrPassword, BOOL bMasterPassword, BOOL bEnhancedErase) {
	HPASTATUS rv = HPAERROR;
	
	if (pDevice) {
		if (pDevice->bSecSupport == TRUE) {
			if (bEnhancedErase == FALSE || (bEnhancedErase == TRUE && pDevice->bSecEnhancedEraseSupport == TRUE)) {
				struct ata_cmd iocmd;
				bzero(&iocmd, sizeof(struct ata_cmd));
	
				//	Description
				//		The SECURITY ERASE PREPARE command shall be issued immediately before 
				//		the SECURITY ERASE UNIT command to enable device erasing and unlocking. 
				//		This command prevents accidental loss of data on the device.
				iocmd.u.request.u.ata.command = SECURITY_ERASE_PREPARE;
	
				iocmd.channel = pDevice->iChannelID;
				iocmd.device = pDevice->iDeviceID;
				iocmd.cmd = ATAREQUEST;
				iocmd.u.request.flags = ATA_CMD_CONTROL;
				iocmd.u.request.timeout = 5;
				
				if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
					DEBUGMSG("securityErase", "SECURITY ERASE PREPARE command executed successfully");
					
					char data[512];
					bzero(&data, 512);
					data[0] |= (bMasterPassword == TRUE ? 0x1 : 0x0);
					data[0] |= (bEnhancedErase == TRUE ? 0x2 : 0x0);
					if (cstrPassword) {
						if (strlen(cstrPassword) > 32) {
							DEBUGERR("securityErase", "Password too long (> 32 characters), truncated");
						}
						strncpy(&data[2], cstrPassword, 32);
					}
					
					bzero(&iocmd, sizeof(struct ata_cmd));
		
					//	Description
					//		This command transfers 512 bytes of data from the host. Table 38 
					//		defines the content of this information. If the password does not match 
					//		the password previously saved by the device, the device shall reject 
					//		the command with command aborted. 
					//
					//		The SECURITY ERASE PREPARE command shall be completed immediately prior 
					//		to the SECURITY ERASE UNIT command. If the device receives a 
					//		SECURITY ERASE UNIT command without an immediately prior 
					//		SECURITY ERASE PREPARE command, the device shall command abort the 
					//		SECURITY ERASE UNIT command. 
					//
					//		When Normal Erase mode is specified, the SECURITY ERASE UNIT command 
					//		shall write binary zeroes to all user data areas. The Enhanced Erase 
					//		mode is optional. When Enhanced Erase mode isspecified, the device 
					//		shall write predetermined data patterns to all user data areas. In 
					//		Enhanced Erase mode, all previously written user data shall be 
					//		overwritten, including sectors that are no longer in use due to 
					//		reallocation. 
					//
					//		This command shall disable the device Lock mode, however, the 
					//		Masterpassword shall still be stored internally within the device and 
					//		may be reactivated later when a new User password is set.
					iocmd.u.request.u.ata.command = SECURITY_ERASE_UNIT;
		
					iocmd.channel = pDevice->iChannelID;
					iocmd.device = pDevice->iDeviceID;
					iocmd.cmd = ATAREQUEST;
					iocmd.u.request.flags = ATA_CMD_WRITE;
					iocmd.u.request.data = data;
					iocmd.u.request.count = 512;
					iocmd.u.request.timeout = 100000;	//Approximately one day because we don't know exactly how long
														//it will take to erase a particular drive (depends on drive
														//size).
									
					if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
						rv = HPASUCCESS;
					} else {
						if (errno != 0) {
							DEBUGERR("securityErase", strerror(errno));
						}
						if (((iocmd.u.request.error >> 2) & 0x1) != 0) {
							DEBUGERR("securityErase", "ATA command returned ABRT error");
						}
					}	
				} else {
					if (errno != 0) {
						DEBUGERR("securityErase (Prepare)", strerror(errno));
					}
					if (((iocmd.u.request.error >> 2) & 0x1) != 0) {
						DEBUGERR("securityErase (Prepare)", "ATA command returned ABRT error");
					}
				}	
			} else {
				DEBUGERR("securityErase", "No enhanced erase support");
			}
		} else {
			DEBUGERR("securityErase", "No security support");
		}
	} else {
		DEBUGERR("securityErase", "Invalid dev_container pointer");
	}

	return rv;
}

HPASTATUS securityFreezeLock(struct dev_container* pDevice) {
	HPASTATUS rv = HPAERROR;

	if (pDevice) {
		if (pDevice->bSecSupport == TRUE) {
			struct ata_cmd iocmd;
			bzero(&iocmd, sizeof(struct ata_cmd));

			//	Description
			//		The SECURITY FREEZE LOCK command shall set the device to Frozen mode. 
			//		After command completion any other commands that update the device 
			//		Lock mode shall be command aborted. Frozen mode shall be disabled by 
			//		power-off or hardware reset. If SECURITY FREEZE LOCK shall be issued 
			//		when the device is in Frozen mode, the command executes and the device 
			//		shall remain in Frozen mode. 
			//
			//		Commands disabled by SECURITY FREEZE LOCK are:   
			//			SECURITY SET PASSWORD   
			//			SECURITY UNLOCK   
			//			SECURITY DISABLE PASSWORD   
			//			SECURITY ERASE PREPARE   
			//			SECURITY ERASE UNIT
			iocmd.u.request.u.ata.command = SECURITY_FREEZE_LOCK;

			iocmd.channel = pDevice->iChannelID;
			iocmd.device = pDevice->iDeviceID;
			iocmd.cmd = ATAREQUEST;
			iocmd.u.request.flags = ATA_CMD_CONTROL;
			iocmd.u.request.timeout = 5;
			
			if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
				rv = HPASUCCESS;
			} else {
				if (errno != 0) {
					DEBUGERR("securityFreezeLock", strerror(errno));
				}
				if (((iocmd.u.request.error >> 2) & 0x1) != 0) {
					DEBUGERR("securityFreezeLock", "ATA command returned ABRT error");
				}
			}
		} else {
			DEBUGERR("securityFreezeLock", "No security support");
		}
	} else {
		DEBUGERR("securityFreezeLock", "Invalid dev_container pointer");
	}
	
	return rv;
}

HPASTATUS securityDisablePassword(struct dev_container* pDevice, char* cstrPassword, BOOL bMasterPassword) {
	HPASTATUS rv = HPAERROR;
	
	if (pDevice) {
		if (pDevice->bSecSupport == TRUE) {
			char data[512];
			bzero(&data, 512);
			data[0] = (bMasterPassword == TRUE ? 1 : 0);
			if (cstrPassword) {
				if (strlen(cstrPassword) > 32) {
					DEBUGERR("securityDisablePassword", "Password too long (> 32 characters), truncated");
				}
				strncpy(&data[2], cstrPassword, 32);
			}

			struct ata_cmd iocmd;
			bzero(&iocmd, sizeof(struct ata_cmd));

			//	Description
			//		The SECURITY DISABLE PASSWORD command transfers 512 bytes of data from 
			//		the host. Table 37 defines the content of this information. If the 
			//		password selected by word 0 matches the password previously saved by 
			//		the device, the device shall disable the Lock mode. This command shall 
			//		not change the Masterpassword. The Master password shall be reactivated 
			//		when a User password is set(See 4.7).
			iocmd.u.request.u.ata.command = SECURITY_DISABLE_PASSWORD;

			iocmd.channel = pDevice->iChannelID;
			iocmd.device = pDevice->iDeviceID;
			iocmd.cmd = ATAREQUEST;
			iocmd.u.request.flags = ATA_CMD_WRITE;
			iocmd.u.request.data = data;
			iocmd.u.request.count = 512;
			iocmd.u.request.timeout = 5;
			
			if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
				rv = HPASUCCESS;
			} else {
				if (errno != 0) {
					DEBUGERR("securityDisablePassword", strerror(errno));
				}
				if (((iocmd.u.request.error >> 2) & 0x1) != 0) {
					DEBUGERR("securityDisablePassword", "ATA command returned ABRT error");
				}
			}
		} else {
			DEBUGERR("securityDisablePassword", "No security support");
		}
	} else {
		DEBUGERR("securityDisablePassword", "Invalid dev_container pointer");
	}

	return rv;
}

#endif
