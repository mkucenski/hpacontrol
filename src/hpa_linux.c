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
