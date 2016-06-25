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

#ifndef _HPA_H_
#define _HPA_H_

#include "hpacontrol.h"

//ATA Command Codes
#define READ_NATIVE_MAX_ADDRESS		0xf8
#define READ_NATIVE_MAX_ADDRESS_EXT	0x27
#define SET_MAX_ADDRESS				0xf9
#define SET_MAX_ADDRESS_EXT			0x37
#define	SET_MAX_SET_PASSWORD		0x01
#define	SET_MAX_LOCK				0x02
#define	SET_MAX_UNLOCK				0x03
#define	SET_MAX_FREEZE_LOCK			0x04

HPASTATUS readNativeMaxAddress(struct dev_container* pDevice, u_int64_t* p_uiNativeMaxAddress);
HPASTATUS setMaxAddress(struct dev_container* pDevice, u_int64_t uiMaxAddress, BOOL bVolatile);
HPASTATUS setMaxSetPassword(struct dev_container* pDevice, char* cstrPassword);
HPASTATUS setMaxLock(struct dev_container* pDevice);
HPASTATUS setMaxUnlock(struct dev_container* pDevice, char* cstrPassword);
HPASTATUS setMaxFreezeLock(struct dev_container* pDevice);

#endif //_HPA_H_
