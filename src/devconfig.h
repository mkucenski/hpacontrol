#ifndef _DEVCONFIG_H_
#define _DEVCONFIG_H_

#include "hpacontrol.h"

//ATA Command Codes
#define DEVICE_CONFIG				0xb1
#define		DEVICE_CONFIG_RESTORE	0xc0
#define		DEVICE_CONFIG_FREEZE	0xc1
#define		DEVICE_CONFIG_IDENTIFY	0xc2
#define		DEVICE_CONFIG_SET		0xc3

HPASTATUS devconfigGetAllowHPAReporting(struct dev_container* pDevice, BOOL* p_bAllowHPAReporting);
HPASTATUS devconfigGetAllowLBA48Reporting(struct dev_container* pDevice, BOOL* p_bAllowLBA48Reporting);
HPASTATUS devconfigGetAllowSecurityReporting(struct dev_container* pDevice, BOOL* p_bAllowSecurityReporting);

#endif //_DEVCONFIG_H_
