#include <windows.h>
#include <stdio.h>
#include "d2gelib/d2server.h"
#include "callback.h"
#include "eventlog.h"
#include "net.h"


#define _D(v) #v,v


EVENTCALLBACKTABLE	gEventCallbackTable;


extern void __fastcall CloseGame(WORD wGameId)
{
	send_closegame(wGameId);
	return;
}


extern void __fastcall LeaveGame(LPGAMEDATA lpGameData, WORD wGameId, WORD wCharClass, 
				DWORD dwCharLevel, DWORD dwExpLow, DWORD dwExpHigh,
				WORD wCharStatus, LPCSTR lpCharName, LPCSTR lpCharPortrait,
				BOOL bUnlock, DWORD dwZero1, DWORD dwZero2,
				LPCSTR lpAccountName, PLAYERDATA PlayerData,
				PLAYERMARK PlayerMark)
{
	send_leavegame(lpGameData, wGameId, wCharClass, dwCharLevel, dwExpLow, dwExpHigh, wCharStatus, lpCharName, lpCharPortrait, bUnlock, dwZero1, dwZero2, lpAccountName, PlayerData, PlayerMark);
	return;
}


extern void __fastcall GetDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
						DWORD dwClientId, LPCSTR lpAccountName)
{
	send_getchar(lpGameData, lpCharName, dwClientId, lpAccountName);
	return;
}


extern void __fastcall SaveDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
					LPCSTR lpAccountName, LPVOID lpSaveData,
					DWORD dwSize, PLAYERDATA PlayerData)
{
	send_savechar(lpGameData, lpCharName, lpAccountName, lpSaveData, dwSize, PlayerData);
	return;
}


extern void __cdecl	ServerLogMessage(DWORD dwCount, LPCSTR lpFormat, ...)
{
	va_list     ap;

	va_start(ap,lpFormat);
	LogAP("ServerLogMessage", lpFormat, ap);
	va_end(ap);
	return;
}


extern void __fastcall EnterGame(WORD wGameId, LPCSTR lpCharName, WORD wCharClass, 
				DWORD dwCharLevel, DWORD dwReserved)

{
	send_entergame(wGameId, lpCharName, wCharClass, dwCharLevel, dwReserved);
	return;
}


extern BOOL __fastcall FindPlayerToken(LPCSTR lpCharName, DWORD dwToken, WORD wGameId,
					LPSTR lpAccountName, LPPLAYERDATA lpPlayerData)
{
	return send_findtoken(lpCharName, dwToken, wGameId, lpAccountName, lpPlayerData);
}


extern void __fastcall UnlockDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
						LPCSTR lpAccountName)
{
	return;
}


extern void __fastcall RelockDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
						LPCSTR lpAccountName)
{
	return;
}


extern void __fastcall UpdateCharacterLadder(LPCSTR lpCharName, WORD wCharClass, 
					DWORD dwCharLevel, DWORD dwCharExpLow, 
					DWORD dwCharExpHigh,  WORD wCharStatus,
					PLAYERMARK PlayerMark)
{
	send_updateladder(lpCharName, wCharClass, dwCharLevel, dwCharExpLow, dwCharExpHigh, wCharStatus, PlayerMark);
	return;
}


extern void __fastcall UpdateGameInformation(WORD wGameId, LPCSTR lpCharName, 
					WORD wCharClass, DWORD dwCharLevel)
{
	send_updateinfo(wGameId, lpCharName, wCharClass, dwCharLevel);
	return;
}


extern GAMEDATA __fastcall SetGameData(void)
{
	return (GAMEDATA) 0x87654321;
}


extern void __fastcall SaveDatabaseGuild(DWORD dwReserved1, DWORD dwReserved2,
					DWORD dwReserved3)

{
	return;
}


extern void __fastcall ReservedCallback1(DWORD dwReserved1, DWORD dwReserved2)
{
	return;
}


extern void __fastcall ReservedCallback2(DWORD dwReserved1, DWORD dwReserved2, 
					DWORD dwReserved3)
{
	return;
}
	

extern void __fastcall LoadComplete(WORD wGameId, LPCSTR lpCharName, BOOL bExpansion)
{
	return;
}



extern PEVENTCALLBACKTABLE EventCallbackTableInit(void)
{
	gEventCallbackTable.fpCloseGame=CloseGame;
	gEventCallbackTable.fpLeaveGame=LeaveGame;
	gEventCallbackTable.fpGetDatabaseCharacter=GetDatabaseCharacter;
	gEventCallbackTable.fpSaveDatabaseCharacter=SaveDatabaseCharacter;
	gEventCallbackTable.fpServerLogMessage=ServerLogMessage;
	gEventCallbackTable.fpEnterGame=EnterGame;
	gEventCallbackTable.fpFindPlayerToken=FindPlayerToken;
	gEventCallbackTable.fpUnlockDatabaseCharacter=UnlockDatabaseCharacter;
	gEventCallbackTable.fpRelockDatabaseCharacter=RelockDatabaseCharacter;
	gEventCallbackTable.fpUpdateCharacterLadder=UpdateCharacterLadder;
	gEventCallbackTable.fpUpdateGameInformation=UpdateGameInformation;
	gEventCallbackTable.fpSetGameData=SetGameData;
	gEventCallbackTable.fpReserved1=ReservedCallback1;
	gEventCallbackTable.fpReserved2=ReservedCallback2;
	gEventCallbackTable.fpSaveDatabaseGuild=SaveDatabaseGuild;
	gEventCallbackTable.fpLoadComplete=LoadComplete;
	return &gEventCallbackTable;
}
