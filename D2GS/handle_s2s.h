#ifndef INCLUDED_HANDLE_S2S_H
#define INCLUDED_HANDLE_S2S_H

#include "d2gelib/d2server.h"

#pragma warning(disable : 4996)
/* structure */
typedef struct {
	UCHAR		realmname[MAX_REALMNAME_LEN];
	DWORD		sessionnum;
	DWORD		gsactive;
} D2GSPARAM, *PD2GSPARAM, *LPD2GAPARAM;


/* for check sum calculation */
#ifndef ROTL
#define ROTL(x,n,w) ( ((n)%(w)) ? (((x)<<((n)%(w))) | ((x)>>((w)-((n)%(w))))) : (x) )
#endif

/* functions */
int D2GSInitializeS2S(void);
void D2GSActive(int flag);
void str2lower(unsigned char *str);
DWORD D2GSGetSequence(void);
DWORD D2GSGetCheckSum(void);
void D2GSSendClassToD2CS(void);
void D2GSSendClassToD2DBS(void);
void D2GSHandleS2SPacket(D2GSPACKET *lpPacket);
void D2GSAuthreq(LPVOID *lpdata);
void D2GSAuthReply(LPVOID *lpdata);
void D2GSSetD2CSMaxGameNumber(DWORD maxgamenum);
void D2XSEchoReply(int peer);
void D2CSCreateEmptyGame(LPVOID *lpdata);
void D2CSClientJoinGameRequest(LPVOID *lpdata);
void D2CSSetInitInfo(LPVOID *lpdata);
void D2CSSetConfFile(LPVOID *lpdata);
void D2CSOperate(LPVOID *lpdata);
/* by callback function */
BOOL D2GSCBFindPlayerToken(LPCSTR lpCharName, DWORD dwToken, DWORD dwGameId,
				LPSTR lpAccountName, LPPLAYERDATA lpPlayerData);
void D2GSCBEnterGame(DWORD dwGameId, LPCSTR lpCharName, WORD wCharClass,
				DWORD dwCharLevel, DWORD dwReserved);
void D2GSCBLeaveGame(LPGAMEDATA lpGameData, DWORD dwGameId, WORD wCharClass,
				DWORD dwCharLevel, DWORD dwExpLow, DWORD dwExpHigh,
				WORD wCharStatus, LPCSTR lpCharName, LPCSTR lpCharPortrait,
				BOOL bUnlock, DWORD dwZero1, DWORD dwZero2,
				LPCSTR lpAccountName, PLAYERDATA PlayerData,
				PLAYERMARK PlayerMark);
void D2GSCBCloseGame(DWORD dwGameId);
void D2GSCBUpdateGameInformation(DWORD dwGameId, LPCSTR lpCharName, 
				WORD wCharClass, DWORD dwCharLevel);
void D2GSCBGetDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
						DWORD dwClientId, LPCSTR lpAccountName);
void D2GSCBSaveDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
					LPCSTR lpAccountName, LPVOID lpSaveData,
					DWORD dwSize, PLAYERDATA PlayerData);
void D2GSWriteCharInfoFile(LPCSTR lpAccountName, LPCSTR lpCharName,
					WORD wCharClass, DWORD dwCharLevel, DWORD dwExpLow,
					WORD wCharStatus, LPCSTR lpCharPortrait);
void D2GSUpdateCharacterLadder(LPCSTR lpCharName, WORD wCharClass, DWORD dwCharLevel,
					DWORD dwCharExpLow, DWORD dwCharExpHigh,  WORD wCharStatus);
void D2GSLoadComplete(DWORD dwGameId, LPCSTR lpCharName, BOOL bExpansion);

/* by d2dbs */
void D2DBSSaveDataReply(LPVOID *lpdata);
void D2DBSGetDataReply(LPVOID *lpdata);
void D2GSSetCharLockStatus(LPCSTR lpAccountName, LPCSTR lpCharName, UCHAR *RealmName, DWORD CharLockStatus);
void D2GSUnlockChar(LPCSTR lpAccountName, LPCSTR lpCharName);
void D2DBSKickRequest(LPVOID *lpdata);
void S2STick();
void SendPerformanceCounter(int busy_pool, int idle_pool, int thread_num, int handle_num, int ge_num);
void SendRateExceeded(int request_rate, LPCSTR account, LPCSTR charname, LPCSTR ipaddr);
void SendEmergency(int emergency_type, int param);

#endif /* INCLUDED_HANDLE_S2S_H */