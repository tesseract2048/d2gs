#ifndef INCLUDED_NET_H
#define INCLUDED_NET_H

int   D2GSNetInitialize(int geid);
void send_leavegame(LPGAMEDATA lpGameData, WORD wGameId, WORD wCharClass, 
					DWORD dwCharLevel, DWORD dwExpLow, DWORD dwExpHigh,
					WORD wCharStatus, LPCSTR lpCharName, LPCSTR lpCharPortrait,
					BOOL bUnlock, DWORD dwZero1, DWORD dwZero2,
					LPCSTR lpAccountName, PLAYERDATA PlayerData,
					PLAYERMARK PlayerMark);
void send_updateinfo(WORD wGameId, LPCSTR lpCharName, 
					WORD wCharClass, DWORD dwCharLevel);
void send_updateladder(LPCSTR lpCharName, WORD wCharClass, 
						DWORD dwCharLevel, DWORD dwCharExpLow, 
						DWORD dwCharExpHigh,  WORD wCharStatus,
						PLAYERMARK PlayerMark);
void send_entergame(WORD wGameId, LPCSTR lpCharName, WORD wCharClass, 
					DWORD dwCharLevel, DWORD dwReserved);
void send_savechar(LPGAMEDATA lpGameData, LPCSTR lpCharName,
					LPCSTR lpAccountName, LPVOID lpSaveData,
					DWORD dwSize, PLAYERDATA PlayerData);
void send_getchar(LPGAMEDATA lpGameData, LPCSTR lpCharName,
					DWORD dwClientId, LPCSTR lpAccountName);
void send_closegame(WORD wGameId);
BOOL send_findtoken(LPCSTR lpCharName, DWORD dwToken, WORD wGameId,
					LPSTR lpAccountName, LPPLAYERDATA lpPlayerData);
void send_soj_counter_update(int increment);

int get_id();

#endif /* INCLUDED_NET_H */