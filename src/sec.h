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
