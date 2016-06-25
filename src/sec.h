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

#ifndef _SEC_H_
#define _SEC_H_

#include "hpacontrol.h"

//ATA Command Codes
#define SECURITY_SET_PASSWORD		0xf1
#define SECURITY_UNLOCK				0xf2
#define SECURITY_ERASE_PREPARE		0xf3
#define SECURITY_ERASE_UNIT			0xf4
#define SECURITY_FREEZE_LOCK		0xf5
#define SECURITY_DISABLE_PASSWORD	0xf6

HPASTATUS securitySetPassword(struct dev_container* pDevice, char* cstrPassword, BOOL bMasterPassword, BOOL bMaxSecurity, u_int16_t uiMasterPasswordRev);
HPASTATUS securityUnlock(struct dev_container* pDevice, char* cstrPassword, BOOL bMasterPassword);
HPASTATUS securityErase(struct dev_container* pDevice, char* cstrPassword, BOOL bMasterPassword, BOOL bEnhancedErase);
HPASTATUS securityFreezeLock(struct dev_container* pDevice);
HPASTATUS securityDisablePassword(struct dev_container* pDevice, char* cstrPassword, BOOL bMasterPassword);

#endif //_SEC_H_
