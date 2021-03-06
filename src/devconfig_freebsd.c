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

#include "devconfig.h"

#include <sys/ata.h>
#include <string.h>
#include <stdio.h>

extern int errno;

HPASTATUS devconfigGetAllowHPAReporting(struct dev_container* pDevice, BOOL* p_bAllowHPAReporting) {
	HPASTATUS rv = HPAERROR;

	if (pDevice) {
		char data[512];
		bzero(&data, 512);

		struct ata_cmd iocmd;
		bzero(&iocmd, sizeof(struct ata_cmd));

		iocmd.u.request.u.ata.command = DEVICE_CONFIG;
		iocmd.u.request.u.ata.feature = DEVICE_CONFIG_IDENTIFY;

		iocmd.channel = pDevice->iChannelID;
		iocmd.device = pDevice->iDeviceID;
		iocmd.cmd = ATAREQUEST;
		iocmd.u.request.flags = ATA_CMD_READ;
		iocmd.u.request.data = data;
		iocmd.u.request.count = 512;
		iocmd.u.request.timeout = 5;
		
		if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
			*p_bAllowHPAReporting = (((data[14] >> 7) & 0x1) == 1 ? TRUE : FALSE);
			rv = HPASUCCESS;
		} else {
			DEBUGERR("devconfigGetAllowHPAReporting", strerror(errno));
		}
	} else {
		DEBUGERR("devconfigGetAllowHPAReporting", "Invalid dev_container pointer");
	}
	
	return rv;
}

HPASTATUS devconfigGetAllowLBA48Reporting(struct dev_container* pDevice, BOOL* p_bAllowLBA48Reporting) {
	HPASTATUS rv = HPAERROR;

	if (pDevice) {
		char data[512];
		bzero(&data, 512);

		struct ata_cmd iocmd;
		bzero(&iocmd, sizeof(struct ata_cmd));

		iocmd.u.request.u.ata.command = DEVICE_CONFIG;
		iocmd.u.request.u.ata.feature = DEVICE_CONFIG_IDENTIFY;

		iocmd.channel = pDevice->iChannelID;
		iocmd.device = pDevice->iDeviceID;
		iocmd.cmd = ATAREQUEST;
		iocmd.u.request.flags = ATA_CMD_READ;
		iocmd.u.request.data = data;
		iocmd.u.request.count = 512;
		iocmd.u.request.timeout = 5;
		
		if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
			*p_bAllowLBA48Reporting = ((data[15] & 0x1) == 1 ? TRUE : FALSE);
			rv = HPASUCCESS;
		} else {
			DEBUGERR("devconfigGetAllowLBA48Reporting", strerror(errno));
		}
	} else {
		DEBUGERR("devconfigGetAllowLBA48Reporting", "Invalid dev_container pointer");
	}
	
	return rv;
}

HPASTATUS devconfigGetAllowSecurityReporting(struct dev_container* pDevice, BOOL* p_bAllowSecurityReporting) {
	HPASTATUS rv = HPAERROR;

	if (pDevice) {
		char data[512];
		bzero(&data, 512);

		struct ata_cmd iocmd;
		bzero(&iocmd, sizeof(struct ata_cmd));

		iocmd.u.request.u.ata.command = DEVICE_CONFIG;
		iocmd.u.request.u.ata.feature = DEVICE_CONFIG_IDENTIFY;

		iocmd.channel = pDevice->iChannelID;
		iocmd.device = pDevice->iDeviceID;
		iocmd.cmd = ATAREQUEST;
		iocmd.u.request.flags = ATA_CMD_READ;
		iocmd.u.request.data = data;
		iocmd.u.request.count = 512;
		iocmd.u.request.timeout = 5;
		
		if (ioctl(pDevice->fdATAController, IOCATA, &iocmd) != -1 && iocmd.u.request.error == 0) {
			*p_bAllowSecurityReporting = (((data[14] >> 3) & 0x1) == 1 ? TRUE : FALSE);
			rv = HPASUCCESS;
		} else {
			DEBUGERR("devconfigGetAllowSecurityReporting", strerror(errno));
		}
	} else {
		DEBUGERR("devconfigGetAllowSecurityReporting", "Invalid dev_container pointer");
	}
	
	return rv;
}

#endif
