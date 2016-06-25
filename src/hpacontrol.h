#ifndef _HPACONTROL_H_
#define _HPACONTROL_H_

#include <sys/types.h>

typedef enum {FALSE, TRUE} BOOL;
typedef enum {HPAERROR, HPASUCCESS, HPAUNSUPPORTED} HPASTATUS;

#define MSG(desc) printf("%s\n", desc)
#define ERR(desc) fprintf(stderr, "ERROR: %s\n", desc)

#ifdef DEBUG
#define DEBUGERR(func, desc) fprintf(stderr, "(DEBUG) ERROR: %s() - %s\n", func, desc)
#define DEBUGMSG(func, desc) fprintf(stderr, "(DEBUG) %s() - %s\n", func, desc)
#define DEBUGMSG1ARG(func, desc, arg1) fprintf(stderr, "(DEBUG) %s() - %s %s\n", func, desc, arg1)
#else
#define DEBUGERR(func, desc)
#define DEBUGMSG(func, desc)
#define DEBUGMSG1ARG(func, desc, arg1)
#endif

struct dev_container {
	int fdATAController;
	int iChannelID;
	int iDeviceID;
	char cstrDevName[32];
	char cstrModel[41];
	char cstrSerial[21];
	char cstrRevision[9];
	
	BOOL bLBA48Support;
	BOOL bNOPSupport;

	//Host Protected Area Flags
	BOOL bHPASupport;
	BOOL bHPAEnabled;
	BOOL bHPASecuritySupport;
	BOOL bHPASecurityEnabled;
	
	//Device Configuration Overlay Flags
	BOOL bDCOSupport;
	BOOL bDCOEnabled;

	//Drive Security Flags
	BOOL bSecSupport;
	BOOL bSecEnabled;
	u_int16_t uiMasterPasswordRev;
	u_int16_t uiEraseTime;
	u_int16_t uiEnhancedEraseTime;
	BOOL bSecMaxSecurity;
	BOOL bSecEnhancedEraseSupport;
	BOOL bSecCountExpired;
	BOOL bSecFrozen;
	BOOL bSecLocked;
	
	u_int64_t uiMaxUserAddress;
};

HPASTATUS sendNop(struct dev_container* pDevice);
HPASTATUS getDevice(struct dev_container* pDevice);
HPASTATUS closeDevice(struct dev_container* pDevice);
HPASTATUS reinitChannel(struct dev_container* pDevice);

#endif //_HPACONTROL_H_
