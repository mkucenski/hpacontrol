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

#include "hpa.h"

#include <stdio.h>
#include <string.h>
#include <sys/ata.h>

extern int errno;

HPASTATUS readNativeMaxAddress(struct dev_container* pDevice, u_int64_t* p_uiNativeMaxAddress) {
	HPASTATUS rv = HPAERROR;

	if (pDevice) {
		if (pDevice->bHPASupport == TRUE) {
			if (pDevice->bLBA48Support == TRUE) {
				//TODO
				fprintf(stderr, "ERROR: No support for LBA48 drives\n");
			} //else {
				struct ata_cmd iocmd;
				bzero(&iocmd, sizeof(struct ata_cmd));
		
				iocmd.channel = pDevice->iChannelID;
				iocmd.device = pDevice->iDeviceID;
				iocmd.cmd = ATAREQUEST;
				iocmd.u.request.flags = ATA_CMD_CONTROL;
				iocmd.u.request.u.ata.command = (pDevice->bLBA48Support == TRUE ? READ_NATIVE_MAX_ADDRESS_EXT : READ_NATIVE_MAX_ADDRESS);
				iocmd.u.request.timeout = 5;
				
				if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
					*p_uiNativeMaxAddress = iocmd.u.request.u.ata.lba;
					rv = HPASUCCESS;
				} else {
					if (errno != 0) {
						DEBUGERR("readNativeMaxAddress", strerror(errno));
					}
					if (((iocmd.u.request.error >> 2) & 0x1) != 0) {
						DEBUGERR("readNativeMaxAddress", "ATA command returned ABRT error");
					}
				}
			//}
		} else {
			DEBUGERR("readNativeMaxAddress", "No HPA support");
		}
	} else {
		DEBUGERR("readNativeMaxAddress", "Invalid dev_container pointer");
	}
	
	return rv;
}

HPASTATUS setMaxAddress(struct dev_container* pDevice, u_int64_t uiNewMaxAddress, BOOL bVolatile) {
	int rv = HPAERROR;
	
	if (pDevice) {
		if (pDevice->bHPASupport == TRUE) {
			u_int64_t uiNativeMaxAddress = 0;
			if (readNativeMaxAddress(pDevice, &uiNativeMaxAddress) != HPAERROR) {
				if (uiNewMaxAddress <= uiNativeMaxAddress) {
					if (pDevice->bLBA48Support == TRUE) {
						//TODO
						fprintf(stderr, "ERROR: No support for LBA48 drives\n");
					} //else {
						struct ata_cmd iocmd;
						bzero(&iocmd, sizeof(struct ata_cmd));
				
						iocmd.channel = pDevice->iChannelID;
						iocmd.device = pDevice->iDeviceID;
						iocmd.cmd = ATAREQUEST;
						iocmd.u.request.flags = ATA_CMD_CONTROL;
						iocmd.u.request.u.ata.command = (pDevice->bLBA48Support == TRUE ? SET_MAX_ADDRESS_EXT : SET_MAX_ADDRESS);
						iocmd.u.request.u.ata.lba = uiNewMaxAddress;
						iocmd.u.request.u.ata.count = (bVolatile == TRUE ? 1 : 0);
						iocmd.u.request.timeout = 5;
						
						if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
							rv = HPASUCCESS;
						} else {
							if (errno != 0) {
								DEBUGERR("setMaxAddress", strerror(errno));
							}
							if (((iocmd.u.request.error >> 2) & 0x1) != 0) {
								DEBUGERR("setMaxAddress", "ATA command returned ABRT error");
							}
							if (((iocmd.u.request.error >> 4) & 0x1) != 0) {
								DEBUGERR("setMaxAddress", "ATA command returned IDNF error");
							}
						}
					//}
				} else {
					DEBUGERR("setMaxAddress", "New max address greater than native max address");
				}
			} else {
				DEBUGERR("setMaxAddress", "Failed to read native max address");
			}
		} else {
			DEBUGERR("setMaxAddress", "No HPA support");
		}
	} else {
		DEBUGERR("setMaxAddress", "Invalid dev_container pointer");
	}
	
	return rv;
}

HPASTATUS setMaxSetPassword(struct dev_container* pDevice, char* cstrPassword) {
	HPASTATUS rv = HPAERROR;

	if (pDevice) {
		if (pDevice->bHPASupport == TRUE) {
			if (pDevice->bHPASecuritySupport == TRUE) {
				struct ata_cmd iocmd;
				bzero(&iocmd, sizeof(struct ata_cmd));
				
				char data[512];
				bzero(&data, 512);
				if (cstrPassword) {
					if (strlen(cstrPassword) > 32) {
						DEBUGERR("setMaxSetPassword", "Password too long (> 32 characters), truncated");
					}
					strncpy(&data[2], cstrPassword, 32);
				}
			
				iocmd.channel = pDevice->iChannelID;
				iocmd.device = pDevice->iDeviceID;
				iocmd.cmd = ATAREQUEST;
				iocmd.u.request.flags = ATA_CMD_WRITE;
	
				//	Description
				// 		This command requests a transfer of a single sector of data from the host. 
				// 		Table 46 defines the content of this sector of information. The password is 
				// 		retained by the device until the next power cycle. When the device accepts this 
				// 		command the device is in Set_Max_Unlocked state.
				iocmd.u.request.u.ata.command = SET_MAX_ADDRESS;
				iocmd.u.request.u.ata.feature = SET_MAX_SET_PASSWORD;
	
				iocmd.u.request.data = data;
				iocmd.u.request.count = 512;
				iocmd.u.request.timeout = 5;
				
				if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
					rv = HPASUCCESS;
				} else {
					if (errno != 0) {
						DEBUGERR("setMaxSetPassword", strerror(errno));
					}
					if (((iocmd.u.request.error >> 2) & 0x1) != 0) {
						DEBUGERR("setMaxSetPassword", "ATA command returned ABRT error");
					}
				}
			} else {
				DEBUGERR("setMaxSetPassword", "No HPA security support");
			}
		} else {
			DEBUGERR("setMaxSetPassword", "No HPA support");
		}
	} else {
		DEBUGERR("setMaxSetPassword", "Invalid dev_container pointer");
	}
	
	return rv;
}

HPASTATUS setMaxLock(struct dev_container* pDevice) {
	HPASTATUS rv = HPAERROR;

	if (pDevice) {
		if (pDevice->bHPASupport == TRUE) {
			if (pDevice->bHPASecuritySupport == TRUE) {
				struct ata_cmd iocmd;
				bzero(&iocmd, sizeof(struct ata_cmd));
				
				iocmd.channel = pDevice->iChannelID;
				iocmd.device = pDevice->iDeviceID;
				iocmd.cmd = ATAREQUEST;
				iocmd.u.request.flags = ATA_CMD_CONTROL;
	
				//	Description
				// 		The SET MAX LOCK command sets the device into Set_Max_Locked state. After 
				// 		this command is completed any other SET MAX commands except SET MAX UNLOCK 
				// 		and SET MAX FREEZE LOCK shall be command aborted. The device shall remain in 
				// 		this state until a power cycle or command completion without error of a 
				// 		SET MAX UNLOCK or SET MAX FREEZE LOCK command.
				iocmd.u.request.u.ata.command = SET_MAX_ADDRESS;
				iocmd.u.request.u.ata.feature = SET_MAX_LOCK;
	
				iocmd.u.request.timeout = 5;
				
				if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
					rv = HPASUCCESS;
				} else {
					if (errno != 0) {
						DEBUGERR("setMaxLock", strerror(errno));
					}
					if (((iocmd.u.request.error >> 2) & 0x1) != 0) {
						DEBUGERR("setMaxLock", "ATA command returned ABRT error");
					}
				}
			} else {
				DEBUGERR("setMaxLock", "No HPA security support");
			}
		} else {
			DEBUGERR("setMaxLock", "No HPA support");
		}
	} else {
		DEBUGERR("setMaxLock", "Invalid dev_container pointer");
	}
	
	return rv;
}

HPASTATUS setMaxUnlock(struct dev_container* pDevice, char* cstrPassword) {
	HPASTATUS rv = HPAERROR;

	if (pDevice) { 
		if (pDevice->bHPASupport == TRUE) {
			if (pDevice->bHPASecuritySupport == TRUE) {
				struct ata_cmd iocmd;
				bzero(&iocmd, sizeof(struct ata_cmd));
				
				char data[512];
				bzero(&data, 512);
				if (cstrPassword) {
					strcpy(&data[2], cstrPassword);
				}
	
				iocmd.channel = pDevice->iChannelID;
				iocmd.device = pDevice->iDeviceID;
				iocmd.cmd = ATAREQUEST;
				iocmd.u.request.flags = ATA_CMD_WRITE;
			
				//	Description
				// 		This command requests a transfer of a single sector of data from the host. 
				// 		Table 46 defines the content of this sector of information. 
				//
				// 		The password supplied in the sector of data transferred shall be compared 
				// 		with the stored SET MAX password. 
				// 
				// 		If the password compare fails, then the device shall return 
				// 		command aborted and decrement the unlock counter. On the acceptance of the 
				// 		SET MAX LOCK command, this counter is set to a value of five and shall be 
				// 		decremented for each password mismatch when SET MAX UNLOCK is issued and the 
				// 		device is locked. When this counter reaches zero, then the SET MAX UNLOCK 
				// 		command shall return command aborted until a power cycle. 
				// 
				// 		If the password compare matches, then the device shall make a transition to 
				// 		the Set_Max_Unlocked state and all SET MAX commands shall be accepted.
				iocmd.u.request.u.ata.command = SET_MAX_ADDRESS;
				iocmd.u.request.u.ata.feature = SET_MAX_UNLOCK;
	
				iocmd.u.request.data = data;
				iocmd.u.request.count = 512;
				iocmd.u.request.timeout = 5;
				
				if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
					rv = HPASUCCESS;
				} else {
					if (errno != 0) {
						DEBUGERR("setMaxUnlock", strerror(errno));
					}
					if (((iocmd.u.request.error >> 2) & 0x1) != 0) {
						DEBUGERR("setMaxUnlock", "ATA command returned ABRT error");
					}
				}
			} else {
				DEBUGERR("setMaxUnlock", "No HPA security support");
			}
		} else {
			DEBUGERR("setMaxUnlock", "No HPA support");
		}
	} else {
		DEBUGERR("setMaxUnlock", "Invalid dev_container pointer");
	}
	
	return rv;
}

HPASTATUS setMaxFreezeLock(struct dev_container* pDevice) {
	HPASTATUS rv = HPAERROR;

	if (pDevice) {
		if (pDevice->bHPASupport == TRUE) {
			if (pDevice->bHPASecuritySupport == TRUE) {
				struct ata_cmd iocmd;
				bzero(&iocmd, sizeof(struct ata_cmd));
				
				iocmd.channel = pDevice->iChannelID;
				iocmd.device = pDevice->iDeviceID;
				iocmd.cmd = ATAREQUEST;
				iocmd.u.request.flags = ATA_CMD_CONTROL;
	
				//	Description	
				// 		The SET MAX FREEZE LOCK command sets the device to Set_Max_Frozen state. 
				// 		After command completion any subsequent SET MAX commands shall be command aborted.
				// 			
				// 		Commands disabled by SET MAX FREEZE LOCK are:   
				// 			- SET MAX ADDRESS			
				// 			- SET MAX SET PASSWORD
				// 			- SET MAX LOCK
				// 			- SET MAX UNLOCK
				iocmd.u.request.u.ata.command = SET_MAX_ADDRESS;
				iocmd.u.request.u.ata.feature = SET_MAX_FREEZE_LOCK;
				
				iocmd.u.request.timeout = 5;
				
				if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
					rv = HPASUCCESS;
				} else {
					if (errno != 0) {
						DEBUGERR("setMaxFreezeLock", strerror(errno));
					}
					if (((iocmd.u.request.error >> 2) & 0x1) != 0) {
						DEBUGERR("setMaxFreeLock", "ATA command returned ABRT error");
					}
				}
			} else {
				DEBUGERR("setMaxFreezeLock", "No HPA security support");
			}
		} else {
			DEBUGERR("setMaxFreezeLock", "No HPA support");
		}
	} else {
		DEBUGERR("setMaxFreezeLock", "Invalid dev_container pointer");
	}
	
	return rv;
}

#endif
