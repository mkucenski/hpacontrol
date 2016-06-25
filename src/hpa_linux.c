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

#ifdef __Linux__

#include "hpa.h"

HPASTATUS readNativeMaxAddress(struct dev_container* pDevice, u_int64_t* p_uiNativeMaxAddress) {
	HPASTATUS rv = HPAUNSUPPORTED;
	return rv;
}

HPASTATUS setMaxAddress(struct dev_container* pDevice, u_int64_t uiMaxAddress, BOOL bVolatile) {
	HPASTATUS rv = HPAUNSUPPORTED;
	return rv;
}

HPASTATUS setMaxSetPassword(struct dev_container* pDevice, char* cstrPassword) {
	HPASTATUS rv = HPAUNSUPPORTED;
	return rv;
}

HPASTATUS sendNop(struct dev_container* pDevice) {
	HPASTATUS rv = HPAUNSUPPORTED;
	return rv;
}

HPASTATUS setMaxLock(struct dev_container* pDevice) {
	HPASTATUS rv = HPAUNSUPPORTED;
	return rv;
}

HPASTATUS setMaxUnlock(struct dev_container* pDevice, char* cstrPassword) {
	HPASTATUS rv = HPAUNSUPPORTED;
	return rv;
}

HPASTATUS setMaxFreezeLock(struct dev_container* pDevice) {
	HPASTATUS rv = HPAUNSUPPORTED;
	return rv;
}

HPASTATUS getDevice(struct dev_container* pDevice) {
	HPASTATUS rv = HPAUNSUPPORTED;
	return rv;
}

HPASTATUS closeDevice(struct dev_container* pDevice) {
	HPASTATUS rv = HPAUNSUPPORTED;
	return rv;
}

HPASTATUS reinitChannel(struct dev_container* pDevice) {
	HPASTATUS rv = HPAUNSUPPORTED;
	return rv;
}

#endif
