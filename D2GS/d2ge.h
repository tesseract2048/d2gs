#ifndef INCLUDED_D2GE_H
#define INCLUDED_D2GE_H

#include "d2gelib/d2server.h"
/* const */
#define D2GE_INIT_TIMEOUT		(60*1000)
#define D2GE_SHUT_TIMEOUT		(5*1000)

#pragma warning(disable : 4996)

/* functions */
int   D2GEStartup(void);
int   D2GECleanup(void);
void  D2GELoadConfig(LPCSTR configfile);
void D2GSDCTrigger();
void D2GSSOJCounterUpdate(int soj_counter);
void D2GSEndAllGames();
BOOL D2GSSendDatabaseCharacter(DWORD dwClientId, LPVOID lpSaveData,
					DWORD dwSize, DWORD dwTotalSize, BOOL bLock,
					DWORD dwReserved1, LPPLAYERINFO lpPlayerInfo, DWORD dwReserved2);
BOOL D2GSRemoveClientFromGame(DWORD dwClientId);
int D2GSNewEmptyGame(LPCSTR lpGameName, LPCSTR lpGamePass,
					LPCSTR lpGameDesc, DWORD dwGameFlag,
					BYTE  bTemplate, BYTE bReserved1,
					BYTE bReserved2, LPWORD pwGameId);
DWORD D2GSSendClientChatMessage(DWORD dwClientId,
		DWORD dwType, DWORD dwColor, LPCSTR lpName, LPCSTR lpText);
void D2GSNewClientComing(struct GEINFO* ge, SOCKET s);
struct GEINFO* GetClientIdOnGE(int dwClientId);
struct GEINFO* GetGEById(int d2ge_id);

struct GEINFO
{
	int					id;
	SOCKET				ge_sock;
	HANDLE				ge_proc;
	int					enabled;
	int					max_game;
	int					curr_game;
	int					curr_player;
	short				trunk_port;
	CRITICAL_SECTION	packet_section;

	struct GEINFO		*prev;
	struct GEINFO		*next;
};

#endif /* INCLUDED_D2GE_H */