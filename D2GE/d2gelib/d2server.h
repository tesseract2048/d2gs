/*  
	Diablo2 Game Server Library
  	Copyright (C) 2000, 2001  Onlyer(onlyer@263.net)

	This library is based on original Diablo2 game library,
	Diablo2 is a trademark of Blizzard Entertainment.
	This library is done by volunteers and is neither 
	supported by nor otherwise affiliated with Blizzard Entertainment.
	You should NEVER use the library on communicate use.

	This program is distributed WITHOUT ANY WARRANTY,
	use it at your own risk.
*/

#ifndef INCLUDED_D2SERVER_H
#define INCLUDED_D2SERVER_H

#include "colorcode.h"

typedef struct 
{
	void * 	fpCloseGame;
	void * 	fpLeaveGame;
	void * 	fpGetDatabaseCharacter;
	void * 	fpSaveDatabaseCharacter;
	void * 	fpServerLogMessage;
	void *	fpEnterGame;
	void * 	fpFindPlayerToken;
	void * 	fpSaveDatabaseGuild;
	void * 	fpUnlockDatabaseCharacter;
	void * 	fpReserved1;
	void * 	fpUpdateCharacterLadder;
	void * 	fpUpdateGameInformation;
	void * 	fpReserved2;
	void * 	fpSetGameData;
	void * 	fpRelockDatabaseCharacter;
	void *	fpLoadComplete;
	void * 	fpReservedDebug[10]; 	/* ignore this,just for internal debug */
} EVENTCALLBACKTABLE,  * PEVENTCALLBACKTABLE, * LPEVENTCALLBACKTABLE;

typedef VOID ( __cdecl * EventLogFunc) (LPCSTR lpModule, LPCSTR lpFormat, ...);

typedef struct 
{
	LPCSTR					szVersion;
	DWORD					dwLibVersion;
	BOOL					bStop;
	BOOL					bIsNT;
	BOOL					bEnablePatch;
	HANDLE					hEventInited;
	EventLogFunc			fpEventLog;
	FARPROC					fpErrorHandle;
	LPEVENTCALLBACKTABLE	fpCallback;
	BOOL					bPreCache;
	DWORD					dwIdleSleep;
	DWORD					dwBusySleep;
	DWORD					dwMaxGame;
	DWORD					dwReserved0;
	DWORD					dwReserved[24];
} D2GSINFO, * PD2GSINFO, * LPD2GSINFO;

#define D2GS_LIBRARY_VERSION	0x010A0304
#define	DEFAULT_IDLE_SLEEP		10
#define	DEFAULT_BUSY_SLEEP		30
#define DEFAULT_MAX_GAME		100

typedef DWORD 	GAMEDATA,   * PGAMEDATA, 	* LPGAMEDATA;
typedef DWORD	PLAYERDATA, * PPLAYERDATA, 	* LPPLAYERDATA;
typedef DWORD	PLAYERMARK, * PPLAYERMARK, 	* LPPLAYERMARK;

typedef struct
{
	PLAYERMARK	PlayerMark;
	DWORD		dwReserved;
	UCHAR		CharName[16];
	UCHAR		AcctName[16];
} PLAYERINFO, * PPLAYERINFO, * LPPLAYERINFO;

typedef BOOL  (__stdcall * D2GSStartFunc ) (LPD2GSINFO lpD2GSInfo);
typedef BOOL  (__stdcall * D2GSNewEmptyGameFunc) (LPCSTR lpGameName, LPCSTR lpGamePass,
					LPCSTR lpGameDesc, DWORD dwGameFlag,
					BYTE  bTemplate, BYTE bReserved1,
					BYTE bReserved2, LPWORD pwGameId);
typedef BOOL  (__stdcall * D2GSSendDatabaseCharacterFunc)(DWORD dwClientId, LPVOID lpSaveData,
					DWORD dwSize, DWORD dwTotalSize, BOOL bLock,
					DWORD dwReserved1, LPPLAYERINFO lpPlayerInfo, DWORD dwReserved2);
typedef VOID  (__stdcall * D2GSRemoveClientFromGameFunc)(DWORD dwClientId);
typedef VOID  (__stdcall * D2GSEndAllGamesFunc) (VOID);

typedef DWORD (__stdcall * D2GSSendClientChatMessageFunc)(DWORD dwClientId,
		DWORD dwType, DWORD dwColor, LPCSTR lpName, LPCSTR lpText);

typedef DWORD (__stdcall * D2GSSetTickCountFunc)(int TickCount);
typedef DWORD (__stdcall * D2GSSetACDataFunc)(LPCSTR acstring);
typedef DWORD (__stdcall * D2GSLoadConfigFunc)(LPCSTR configfile);
typedef DWORD (__stdcall * D2GSInitConfigFunc)();

typedef struct 
{
	DWORD							Reserved;		/* Reserved, ignore it */
	DWORD							Reserved2;		/* Reserved, ignore it */
	D2GSStartFunc		 			D2GSStart;
	D2GSSendDatabaseCharacterFunc 	D2GSSendDatabaseCharacter;
	D2GSRemoveClientFromGameFunc	D2GSRemoveClientFromGame;
	D2GSNewEmptyGameFunc			D2GSNewEmptyGame;
	D2GSEndAllGamesFunc				D2GSEndAllGames;
	D2GSSendClientChatMessageFunc	D2GSSendClientChatMessage;
	D2GSSetTickCountFunc			D2GSSetTickCount;
	D2GSSetACDataFunc				D2GSSetACData;
	DWORD							Reserved3;
	D2GSLoadConfigFunc				D2GSLoadConfig;
	DWORD							Reserved4;
	D2GSInitConfigFunc				D2GSInitConfig;
	DWORD							Reserved5;
	DWORD							Reserved6;
} D2GSINTERFACE, * PD2GSINTERFACE, * LPD2GSINTERFACE;


#ifdef __cplusplus
extern "C" {
#endif
extern LPD2GSINTERFACE QueryInterface(VOID);

#ifdef __cplusplus
}
#endif


/* callback functions */
extern VOID __fastcall 	CloseGame(WORD wGameId);

extern VOID __fastcall 	LeaveGame(LPGAMEDATA lpGameData, WORD wGameId, WORD wCharClass,
				DWORD dwCharLevel, DWORD dwExpLow, DWORD dwExpHigh,
				WORD wCharStatus, LPCSTR lpCharName, LPCSTR lpCharPortrait,
				BOOL bUnlock, DWORD dwZero1, DWORD dwZero2,
				LPCSTR lpAccountName, PLAYERDATA PlayerData,
				PLAYERMARK PlayerMark);

extern VOID __fastcall 	GetDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
						DWORD dwClientId, LPCSTR lpAccountName);

extern VOID __fastcall 	SaveDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
					LPCSTR lpAccountName, LPVOID lpSaveData,
					DWORD dwSize,PLAYERDATA PlayerData);

extern VOID __cdecl	ServerLogMessage(DWORD dwCount, LPCSTR lpFormat, ...);

extern VOID __fastcall 	EnterGame(WORD wGameId, LPCSTR lpCharName, WORD wCharClass,
				DWORD dwCharLevel, DWORD dwZero);

extern BOOL __fastcall FindPlayerToken(LPCSTR lpCharName, DWORD dwToken, WORD wGameId,
					LPSTR lpAccountName, LPPLAYERDATA lpPlayerData);

extern VOID __fastcall 	UnlockDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
						LPCSTR lpAccountName);

extern VOID __fastcall 	RelockDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
						LPCSTR lpAccountName);

extern VOID __fastcall 	UpdateCharacterLadder(LPCSTR lpCharName, WORD wCharClass,
					DWORD dwCharLevel, DWORD dwCharExpLow,
					DWORD dwCharExpHigh,  WORD wCharStatus,
					PLAYERMARK PlayerMark);	

extern VOID __fastcall 	UpdateGameInformation(WORD wGameId, LPCSTR lpCharName,
					WORD wCharClass, DWORD dwCharLevel);

extern GAMEDATA __fastcall SetGameData(VOID);

extern VOID __fastcall 	SaveDatabaseGuild(DWORD dwReserved1, DWORD dwReserved2,
					DWORD dwReserved3);

extern VOID __fastcall 	ReservedCallback1(DWORD dwReserved1, DWORD dwReserved2);

extern VOID __fastcall 	ReservedCallback2(DWORD dwReserved1, DWORD dwReserved2,
					DWORD dwReserved3);

extern VOID __fastcall	LoadComplete(WORD wGameId, LPCSTR lpCharName, BOOL bExpansion);

/* Error Codes */
#define 	D2GS_ERROR_BASE			0x10000000
#define		D2GS_ERROR_INTERNAL		(D2GS_ERROR_BASE+1)
#define		D2GS_ERROR_PARAMS		(D2GS_ERROR_BASE+2)
#define		D2GS_ERROR_DATA_FILE	(D2GS_ERROR_BASE+3)
#define		D2GS_ERROR_LANGUAGE		(D2GS_ERROR_BASE+4)
#define		D2GS_ERROR_LIBRARY		(D2GS_ERROR_BASE+5)
#define		D2GS_ERROR_DLL_FILE		(D2GS_ERROR_BASE+6)
#define		D2GS_ERROR_NETWORK		(D2GS_ERROR_BASE+7)
#define		D2GS_ERROR_VERSION		(D2GS_ERROR_BASE+8)

#define		D2GS_GAMETYPE_CLOSE_HOST		0
#define		D2GS_GAMETYPE_OPEN_HOST			1
#define		D2GS_GAMETYPE_OPEN_NO_HOST		2
#define		D2GS_GAMETYPE_CLOSE_NO_HOST		3

#define		CHAT_MESSAGE_MAX_LEN			0x100
#define		CHAT_MESSAGE_TYPE_CHAT			0x01
#define		CHAT_MESSAGE_TYPE_WHISPER_TO	0x02
#define		CHAT_MESSAGE_TYPE_SYS_MESSAGE	0x04
#define		CHAT_MESSAGE_TYPE_WHISPER_FROM	0x06
#define		CHAT_MESSAGE_TYPE_SCROLL		0x07

#endif
