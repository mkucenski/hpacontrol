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
