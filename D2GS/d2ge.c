#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "d2gs.h"
#include "d2ge.h"
#include "eventlog.h"
#include "vars.h"
#include "list.h"
#include "d2gelib/d2server.h"
#include "trunk.h"
#include "hexdump.h"
#include "d2gamelist.h"

#include "d2gs_d2ge_protocol.h"
#include "Tlhelp32.h"
#include "psapi.h"
#include "handle_s2s.h"

#define tsfGameId(x,geid) ((((geid) & 0xf) << 0x1c) + (x))
#define RtsfGameId(x) ((x) & 0xFFFFFFF)
#define geGameId(x) ((x) >> 0x1c)

#define SEQ_TABLE_SIZE 4096
#define MAX_GE_NUM 10
//#define C2G_SIZE 65536

/* variables */
static char						ge_path[MAX_PATH];
static struct GEINFO *			id2info[MAX_GE_NUM];
static struct GEINFO *			ge_head = NULL;
static SOCKET					ge_comm;
static u_char					config_file[255];
static unsigned long			seqcount = 0;
static void *					seqtable[SEQ_TABLE_SIZE];
static unsigned int				init_count = 0;
static unsigned int				curr_maxgame = 0;
static CRITICAL_SECTION			packetHandler;
static CRITICAL_SECTION			seqHandler;
static CRITICAL_SECTION			GEList;
//static CRITICAL_SECTION			GELock;

DWORD WINAPI					GECommThread(LPVOID lpParam);
DWORD WINAPI					GEListenLoop();
void *							WaitPacketReturn(int seqno, struct GEINFO* geinfo, char* debug);
void							GEPanic(struct GEINFO* geinfo);
void							PrependGEInfo(struct GEINFO* geinfo);
void							RemoveGEInfo(struct GEINFO* geinfo);
void							refresh_max_game();
HANDLE							StartGEProcess(int d2ge_id);
void							StartBenchmark();
void *							GetSeq(int index);
void							SetSeq(int index, void* value);

struct GEINFO* GetGEById(int d2ge_id)
{
	return id2info[d2ge_id];
}

void * GetSeq(int index)
{
	void * rtn;
	EnterCriticalSection(&seqHandler);
	rtn = seqtable[index];
	LeaveCriticalSection(&seqHandler);
	return rtn;
}

void SetSeq(int index, void * value)
{
	EnterCriticalSection(&seqHandler);
	seqtable[index] = value;
	LeaveCriticalSection(&seqHandler);
}

void refresh_max_game()
{
	int tot = 0;
	struct GEINFO* ge_list = ge_head;
	while(ge_list)
	{
		if (ge_list->enabled) tot += ge_list->max_game;
		ge_list = ge_list->next;
	}
	if (curr_maxgame != tot)
	{
		curr_maxgame = tot;
		D2GSSetD2CSMaxGameNumber(curr_maxgame);
	}
}

int CleanD2GE(void)
{
	PROCESSENTRY32 ME32;
	HANDLE hProcessSnap;
	INT TheLoop;
	HANDLE hProcess;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	ME32.dwSize = sizeof(ME32);
	TheLoop = Process32First(hProcessSnap, &ME32);
	while(TheLoop){
		TheLoop = Process32Next(hProcessSnap, &ME32);
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, FALSE, ME32.th32ProcessID);
		if(hProcess){
			char ModulePath[MAX_PATH];
			char* buf = NULL;
			int i;
			int len = GetModuleFileNameExA(hProcess, NULL, ModulePath, MAX_PATH);
			for(i = 0; i < len; i ++)
			{
				if (ModulePath[i] == '\\') buf = &ModulePath[i+1];
			}
			if (buf != NULL)
				if (strcmpi(buf, "D2GE.EXE") == 0)
					TerminateProcess(hProcess, 0);
			CloseHandle(hProcess);
		}
	}
	CloseHandle(hProcessSnap);
}

struct GEINFO* GetClientIdOnGE(int dwClientId){
	return id2info[geGameId(dwClientId)];
}

void GEPanic(struct GEINFO* geinfo)
{
	if (!geinfo->enabled) return;
	geinfo->enabled = 0;
	geinfo->max_game = 0;
	geinfo->curr_game = 0;
	geinfo->curr_player = 0;
	geinfo->trunk_port = 0;
	closesocket(geinfo->ge_sock);
	TerminateProcess(geinfo->ge_proc, 0);
	refresh_max_game();
	CleanupGEChar(geinfo->id);
	CleanupGEGame(geinfo->id);
	SendEmergency(2, geinfo->id);
	Sleep(1000);
	geinfo->ge_proc = StartGEProcess(geinfo->id);
}

void PrependGEInfo(struct GEINFO* geinfo)
{
	EnterCriticalSection(&GEList);
	geinfo->prev = NULL;
	if (ge_head == NULL)
	{
		geinfo->next = NULL;
	}else{
		geinfo->next = ge_head;
		ge_head->prev = geinfo;
	}
	ge_head = geinfo;
	LeaveCriticalSection(&GEList);
}

void RemoveGEInfo(struct GEINFO* geinfo)
{
	EnterCriticalSection(&GEList);
	if (geinfo->prev != NULL)
	{
		if (geinfo->next != NULL)
		{
			geinfo->prev->next = geinfo->next;
			geinfo->next->prev = geinfo->prev;
		}else{
			geinfo->prev->next = NULL;
		}
	}else{
		ge_head = geinfo->next;
	}
	free(geinfo);
	LeaveCriticalSection(&GEList);
}

HANDLE StartGEProcess(int d2ge_id){
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	char arg_buf[255];
	si.dwFlags = STARTF_USESHOWWINDOW;
	sprintf(arg_buf, "\"%s\" %d", ge_path, d2ge_id);
	D2GSEventLog(__FUNCTION__, "Starting up GE Process by command line %s", arg_buf);
	CreateProcessA(ge_path, arg_buf, NULL, NULL, FALSE, REALTIME_PRIORITY_CLASS, NULL, NULL, &si, &pi);
	return pi.hProcess;
}

int D2GEStartup(void)
{
	DWORD		dwThreadId;
	struct sockaddr_in server;
	char		curr_dir[255];
	int i;
	DWORD dwGameId;
	if(bGERunning == TRUE){
		return TRUE;
	}
	srand(time(NULL));
	CleanD2GE();

	InitializeCriticalSection(&GEList);
	//InitializeCriticalSection(&GELock);

	GetModuleFileName(NULL, curr_dir, 255);
	D2GSEventLog(__FUNCTION__, "Current path: %s", curr_dir);
	for(i = strlen(curr_dir) - 1; i >= 0; i --)
	{
		if (curr_dir[i] == '\\'){
			curr_dir[i] = 0;
			break;
		}
	}
	sprintf(ge_path, "%s\\D2GE.EXE", curr_dir);
	D2GSEventLog(__FUNCTION__, "GE path: %s", ge_path);
	for(i = 0; i < d2gsconf.gethread; i ++)
	{
		struct GEINFO *geinfo = (struct GEINFO*)malloc(sizeof(struct GEINFO));
		memset(geinfo, 0, sizeof(struct GEINFO));
		InterlockedIncrement(&init_count);
		geinfo->ge_proc = StartGEProcess(i);
		D2GSEventLog(__FUNCTION__, "GE Handle[%d]: %x", i, geinfo->ge_proc);
		geinfo->id = i;
		geinfo->trunk_port = 0;
		id2info[i] = geinfo;
		InitializeCriticalSection(&geinfo->packet_section);
		PrependGEInfo(geinfo);
	}
	curr_maxgame = 0;

	server.sin_family = AF_INET;
	server.sin_port = htons(12835);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(server.sin_zero, 0 ,8);

	InitializeCriticalSection(&packetHandler);
	InitializeCriticalSection(&seqHandler);
	
	if((ge_comm = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		return FALSE;
	if(bind(ge_comm, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
		return FALSE;
	listen(ge_comm, SOMAXCONN);

	CreateThread(NULL, 0, GEListenLoop, NULL, 0, &dwThreadId);

	if (!StartupTrunk()) {
		D2GSEventLog(__FUNCTION__, "Failed Startup Trunk");
		return -1;
	}

	/*Sleep(5000);
	StartBenchmark();*/
/*

	Sleep(4000);
	for(i = 0; i < 10; i ++)
	{
		int geid;
		char buf[16];
		sprintf(buf, "g%d", i);
		D2GSEventLog(__FUNCTION__, "Creating %s...", buf);
		dwGameId = 0;
		geid = D2GSNewEmptyGame("asd", "", "", 0x300004, 0x11, 0x22, 0x33, &dwGameId);
		D2GSEventLog(__FUNCTION__, "Created on ge %d, GameId: %x", geid, dwGameId);
	}
	D2GSEventLog(__FUNCTION__, "All done.");
*/
	return TRUE;
} /* End of D2GEStartup() */


void* WaitPacketReturn(int seqno, struct GEINFO* geinfo, char* debug)
{
	unsigned int stime = GetTickCount();
	unsigned int ntime;
	void* rpacket;
	while(GetSeq(seqno%SEQ_TABLE_SIZE) == NULL)
	{
		ntime = GetTickCount();
		if (ntime - stime > 30000)
		{
			D2GSEventLog(__FUNCTION__, "Internal error occured in GE %d while waiting packet %s sequence %d", geinfo->id, debug, seqno);
			GEPanic(geinfo);
			return NULL;
		}
		Sleep(1);
	}
	rpacket = GetSeq(seqno%SEQ_TABLE_SIZE);
	SetSeq(seqno%SEQ_TABLE_SIZE, NULL);
	return rpacket;
}

BOOL WINAPI GEListenLoop()
{
	HANDLE hThread;
	SOCKET AcceptSocket;
	DWORD dwThreadId;
	struct sockaddr_in client;
	while(1)
	{
		AcceptSocket = accept(ge_comm, NULL, NULL);
		if(AcceptSocket == INVALID_SOCKET)
			continue;
		hThread = CreateThread(NULL, 0, GECommThread, (LPVOID)AcceptSocket, NULL, &dwThreadId);
		if(hThread)
			CloseHandle(hThread);
	}
}

BOOL SendGEPacket(struct GEINFO* geinfo, char* buf, int len){
	int nBytesLeft = len;
	int nBytesSent = 0;
	int ret;
	EnterCriticalSection(&geinfo->packet_section);
	send(geinfo->ge_sock, &len, 4, 0);
	while(nBytesLeft > 0)
	{
		ret = send(geinfo->ge_sock, buf + nBytesSent, nBytesLeft, 0);
		if(ret <= 0){
			int error = WSAGetLastError();
			D2GSEventLog(__FUNCTION__, "Error %d occured while sending GE packet", error);
			LeaveCriticalSection(&geinfo->packet_section);
			GEPanic(geinfo);
			return FALSE;
		}
		nBytesSent += ret;
		nBytesLeft -= ret;
	}
	LeaveCriticalSection(&geinfo->packet_section);
	//D2GSEventLog(__FUNCTION__, "Sent %d bytes to GE %d", len, geinfo->id);
	return TRUE;
}

struct GEINFO* handle_greet(SOCKET client, char* buf)
{
	t_d2ge_d2gs_greet *greet = buf;
	t_d2gs_d2ge_greet_reply rpacket;
	struct GEINFO* ge_list = ge_head;
	struct GEINFO* geinfo = NULL;
	
	EnterCriticalSection(&GEList);
	while(ge_list)
	{
		if(ge_list->id == greet->d2ge_id){
			geinfo = ge_list;
			break;
		}
		ge_list = ge_list->next;
	}
	LeaveCriticalSection(&GEList);

	if (!geinfo)
	{
		return NULL;
	}
	if (geinfo->enabled)
	{
		D2GSEventLog(__FUNCTION__, "Previous GE detected, terminating by panic signal...", geinfo->id);
		GEPanic(geinfo);
	}
	geinfo->ge_sock = client;
	geinfo->max_game = 0;
	geinfo->curr_game = 0;
	geinfo->enabled = 1;
	D2GSEventLog(__FUNCTION__, "D2GE [id: %d] activated", geinfo->id);

	rpacket.h.seqno = 0;
	rpacket.h.type = D2GS_D2GE_GREET_REPLY;
	memcpy(&rpacket.d2gsconfig, &d2gsconf, sizeof(D2GSCONFIGS));
	strcpy(rpacket.configfile, config_file);
	strcpy(rpacket.acstring, "");
	SendGEPacket(geinfo, &rpacket, sizeof(t_d2gs_d2ge_greet_reply));

	return geinfo;
}

void handle_max_game(SOCKET client, char* buf, struct GEINFO* geinfo)
{
	t_d2ge_d2gs_max_game *max_game = buf;
	geinfo->max_game = max_game->max_game;
	geinfo->curr_game = 0;
	geinfo->curr_player = 0;
	geinfo->trunk_port = max_game->port;
	D2GSEventLog(__FUNCTION__, "D2GE [id: %d, port: %d] setting max game to %d (total %d ge)", max_game->d2ge_id, max_game->port, max_game->max_game, init_count);
	refresh_max_game();
}

void handle_function_return(SOCKET client, char* buf, int size)
{
	char* result = (char*)malloc(size);
	t_d2gs_d2ge_header *rtn = result;
	memcpy(result, buf, size);
	SetSeq(rtn->seqno%SEQ_TABLE_SIZE, result);
}

void handle_closegame(SOCKET client, char* buf, struct GEINFO* geinfo)
{
	t_d2ge_d2gs_closegame *packet = buf;
	geinfo->curr_game -= 1;
	D2GSCBCloseGame(tsfGameId(packet->wGameId, geinfo->id));
}

void handle_leavegame(SOCKET client, char* buf, struct GEINFO* geinfo)
{
	t_d2ge_d2gs_leavegame *packet = buf;
	char* data = buf + sizeof(t_d2ge_d2gs_leavegame);
	geinfo->curr_player -= 1;
	D2GSCBLeaveGame(&packet->lpGameData, tsfGameId(packet->wGameId, geinfo->id), packet->wCharClass, packet->dwCharLevel, packet->dwExpLow, packet->dwExpHigh, packet->wCharStatus, packet->charname, data, packet->bUnlock, packet->dwZero1, packet->dwZero2, packet->accountname, packet->PlayerData, packet->PlayerMark);
}

void handle_getchar(SOCKET client, char* buf)
{
	t_d2ge_d2gs_getchar *packet = buf;
	D2GSCBGetDatabaseCharacter(&packet->lpGameData, packet->charname, tsfGameId(packet->dwClientId, packet->d2ge_id), packet->accountname);
}

void handle_savechar(SOCKET client, char* buf)
{
	t_d2ge_d2gs_savechar *packet = buf;
	char* data = buf + sizeof(t_d2ge_d2gs_savechar);
	D2GSCBSaveDatabaseCharacter(&packet->lpGameData, packet->charname, packet->accountname, data, packet->dwSize, packet->PlayerData);
}

void handle_entergame(SOCKET client, char* buf, struct GEINFO* geinfo)
{
	t_d2ge_d2gs_entergame *packet = buf;
	geinfo->curr_player += 1;
	D2GSCBEnterGame(tsfGameId(packet->wGameId, geinfo->id), packet->charname, packet->wCharClass, packet->dwCharLevel, packet->dwReserved);
}

void handle_updateladder(SOCKET client, char* buf)
{
	t_d2ge_d2gs_updateladder *packet = buf;
	D2GSUpdateCharacterLadder(packet->charname, packet->wCharClass, packet->dwCharLevel, packet->dwCharExpLow, packet->dwCharExpHigh, packet->wCharStatus);
}

void handle_updateinfo(SOCKET client, char* buf, struct GEINFO* geinfo)
{
	t_d2ge_d2gs_updateinfo *packet = buf;
	D2GSCBUpdateGameInformation(tsfGameId(packet->wGameId, geinfo->id), packet->charname, packet->wCharClass, packet->dwCharLevel);
}

void handle_findtoken(SOCKET client, char* buf, struct GEINFO* geinfo)
{
	t_d2ge_d2gs_findtoken *packet = buf;
	t_d2gs_d2ge_findtoken_callback rpacket;
	rpacket.h.seqno = packet->h.seqno;
	rpacket.h.type = D2GS_D2GE_FINDTOKEN_CALLBACK;
	rpacket.result = D2GSCBFindPlayerToken(packet->charname, packet->dwToken, tsfGameId(packet->wGameId, geinfo->id), rpacket.accountname, &rpacket.lpPlayerData);
	SendGEPacket(geinfo, &rpacket, sizeof(t_d2gs_d2ge_findtoken_callback));
}

DWORD WINAPI GECommThread(LPVOID lpParam)
{
	struct GEINFO* geinfo = NULL;
	SOCKET client = (SOCKET)lpParam;
	char buf[32768];
	int packetlen;
	BOOL inpacket = FALSE;
	int bufpos = 0;
	int Block = 0;
	char NoDelay = 1;
	t_d2gs_d2ge_header *header;
	if(ioctlsocket(client, FIONBIO, &Block) == SOCKET_ERROR)
		return -1;
	setsockopt(ge_comm, IPPROTO_TCP, TCP_NODELAY, &NoDelay, sizeof(char));
	while(1)
	{
		if (geinfo && !geinfo->enabled) return 0;
		if (!inpacket)
		{
			int recvlen = recv(client, &packetlen, 4, MSG_WAITALL);
			bufpos = 0;
			inpacket = TRUE;
		}else{
			int recvlen = recv(client, &buf[bufpos], packetlen, 0);
			if(recvlen == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				D2GSEventLog(__FUNCTION__, "Error %d occured while receiving GE packet", error);
				GEPanic(geinfo);
				return 0;
			}
			packetlen -= recvlen;
			bufpos += recvlen;
			if (packetlen > 0) continue;
			header = buf;
			EnterCriticalSection(&packetHandler);
			//D2GSEventLog(__FUNCTION__, "seqno = %d", header->seqno);
			switch (header->type)
			{
			case D2GE_D2GS_GREET:
				//D2GSEventLog(__FUNCTION__, "D2GE_D2GS_GREET");
				geinfo = handle_greet(client, buf);
				if (!geinfo)
				{
					closesocket(client);
					return 0;
				}
				break;
			case D2GE_D2GS_MAX_GAME:
				//D2GSEventLog(__FUNCTION__, "D2GE_D2GS_MAX_GAME");
				handle_max_game(client, buf, geinfo);
				break;
			case D2GE_D2GS_NEWEMPTYGAME_RETURN:
				//D2GSEventLog(__FUNCTION__, "D2GE_D2GS_NEWEMPTYGAME_RETURN");
				handle_function_return(client, buf, sizeof(t_d2ge_d2gs_newemptygame_return));
				break;
			case D2GE_D2GS_SENDCHAR_RETURN:
				//D2GSEventLog(__FUNCTION__, "D2GE_D2GS_SENDCHAR_RETURN");
				handle_function_return(client, buf, sizeof(t_d2ge_d2gs_sendchar_return));
				break;
			case D2GE_D2GS_CHATMSG_RETURN:
				//D2GSEventLog(__FUNCTION__, "D2GE_D2GS_CHATMSG_RETURN");
				//handle_function_return(client, buf, sizeof(t_d2ge_d2gs_chatmsg_return));
				break;
			case D2GE_D2GS_CLOSEGAME:
				//D2GSEventLog(__FUNCTION__, "D2GE_D2GS_CLOSEGAME");
				handle_closegame(client, buf, geinfo);
				break;
			case D2GE_D2GS_ENTERGAME:
				//D2GSEventLog(__FUNCTION__, "D2GE_D2GS_ENTERGAME");
				handle_entergame(client, buf, geinfo);
				break;
			case D2GE_D2GS_LEAVEGAME:
				//D2GSEventLog(__FUNCTION__, "D2GE_D2GS_LEAVEGAME");
				handle_leavegame(client, buf, geinfo);
				break;
			case D2GE_D2GS_GETCHAR:
				//D2GSEventLog(__FUNCTION__, "D2GE_D2GS_GETCHAR");
				handle_getchar(client, buf);
				break;
			case D2GE_D2GS_SAVECHAR:
				//D2GSEventLog(__FUNCTION__, "D2GE_D2GS_SAVECHAR");
				handle_savechar(client, buf);
				break;
			case D2GE_D2GS_FINDTOKEN:
				//D2GSEventLog(__FUNCTION__, "D2GE_D2GS_FINDTOKEN");
				handle_findtoken(client, buf, geinfo);
				break;
			case D2GE_D2GS_UPDATELADDER:
				//D2GSEventLog(__FUNCTION__, "D2GE_D2GS_UPDATELADDER");
				handle_updateladder(client, buf);
				break;
			case D2GE_D2GS_UPDATEINFO:
				//D2GSEventLog(__FUNCTION__, "D2GE_D2GS_UPDATEINFO");
				handle_updateinfo(client, buf, geinfo);
				break;
			}
			LeaveCriticalSection(&packetHandler);
			inpacket = FALSE;
		}
	}
	return 0;
}

/*********************************************************************
 * Purpose: to shutdown the D2 Game Engine
 * Return: TRUE(success) or FALSE(failed)
 *********************************************************************/
int D2GECleanup(void)
{
	CleanD2GE();
	return TRUE;

} /* End of D2GEShutdown() */

void D2GELoadConfig(LPCSTR configfile){
	strcpy(config_file, configfile);
}

void D2GSEndAllGames(){
	struct GEINFO* ge_list = ge_head;
	EnterCriticalSection(&GEList);
	while(ge_list)
	{
		if (ge_list->enabled && ge_list->max_game > 0)
		{
			t_d2gs_d2ge_endallgames packet;
			packet.h.seqno = InterlockedIncrement(&seqcount);
			packet.h.type = D2GS_D2GE_ENDALLGAMES;
			SendGEPacket(ge_list, &packet, sizeof(t_d2gs_d2ge_endallgames));
		}
		ge_list = ge_list->next;
	}
	LeaveCriticalSection(&GEList);
}

BOOL D2GSSendDatabaseCharacter(DWORD dwClientId, LPVOID lpSaveData,
					DWORD dwSize, DWORD dwTotalSize, BOOL bLock,
					DWORD dwReserved1, LPPLAYERINFO lpPlayerInfo, DWORD dwReserved2)
{
	BOOL rtn;
	struct GEINFO* geinfo = GetClientIdOnGE(dwClientId);
	t_d2gs_d2ge_sendchar packet;
	t_d2ge_d2gs_sendchar_return *rpacket;
	if (!geinfo->enabled) return FALSE;
	packet.h.seqno = InterlockedIncrement(&seqcount);
	packet.h.type = D2GS_D2GE_SENDCHAR;
	packet.dwClientId = RtsfGameId(dwClientId);
	packet.dwSize = dwSize;
	packet.dwTotalSize = dwTotalSize;
	packet.bLock = bLock;
	packet.dwReserved1 = dwReserved1;
	packet.dwReserved2 = dwReserved2;
	packet.SaveDataLen = dwSize;
	if (lpPlayerInfo != NULL)
		memcpy(&packet.lpPlayerInfo, lpPlayerInfo, sizeof(PLAYERINFO));
	else
		memset(&packet.lpPlayerInfo, 0, sizeof(PLAYERINFO));
	//EnterCriticalSection(&GELock);
	if (lpSaveData == NULL)
	{
		if (!SendGEPacket(geinfo, &packet, sizeof(t_d2gs_d2ge_sendchar)))
		{
			return FALSE;
		}
	}else{
		char* buf = (char*)malloc(sizeof(t_d2gs_d2ge_sendchar) + dwTotalSize);
		memcpy(buf, &packet, sizeof(t_d2gs_d2ge_sendchar));
		memcpy(buf+sizeof(t_d2gs_d2ge_sendchar), lpSaveData, packet.SaveDataLen);
		SetSeq(packet.h.seqno%SEQ_TABLE_SIZE, NULL);
		if (!SendGEPacket(geinfo, buf, sizeof(t_d2gs_d2ge_sendchar) + packet.SaveDataLen))
		{
			free(buf);
			return FALSE;
		}
		free(buf);
	}
	rpacket = WaitPacketReturn(packet.h.seqno, geinfo, "SendChar");
	//LeaveCriticalSection(&GELock);
	if(rpacket){
		rtn = rpacket->result;
		free(rpacket);
	}else{
		return FALSE;
	}
	return rtn;
};

BOOL D2GSRemoveClientFromGame(DWORD dwClientId){
	BOOL rtn;
	struct GEINFO* geinfo = GetClientIdOnGE(dwClientId);
	t_d2gs_d2ge_removeclient packet;
	if (!geinfo->enabled) return FALSE;
	packet.h.seqno = InterlockedIncrement(&seqcount);
	packet.h.type = D2GS_D2GE_REMOVECLIENT;
	packet.dwClientId = RtsfGameId(dwClientId);
	SendGEPacket(geinfo, &packet, sizeof(t_d2gs_d2ge_removeclient));
	return TRUE;
};

int D2GSNewEmptyGame(LPCSTR lpGameName, LPCSTR lpGamePass,
					LPCSTR lpGameDesc, DWORD dwGameFlag,
					BYTE  bTemplate, BYTE bReserved1,
					BYTE bReserved2, LPDWORD pwGameId)
{
	int rtn;
	struct GEINFO* geinfo = NULL;
	struct GEINFO* ge_list = ge_head;
	t_d2gs_d2ge_newemptygame packet;
	t_d2ge_d2gs_newemptygame_return *rpacket;
	EnterCriticalSection(&GEList);
	while(ge_list)
	{
		if (ge_list->enabled && ge_list->curr_game < ge_list->max_game)
		{
			if (!geinfo || (geinfo->curr_game > ge_list->curr_game))
				geinfo = ge_list;
		}
		ge_list = ge_list->next;
	}
	LeaveCriticalSection(&GEList);

	if (!geinfo)
	{
		D2GSEventLog(__FUNCTION__, "No more slot on all GEs, refuse creating game");
		return -1;
	}

	D2GSEventLog(__FUNCTION__, "Creating %s on ge %d", lpGameName, geinfo->id);
	packet.h.seqno = InterlockedIncrement(&seqcount);
	packet.h.type = D2GS_D2GE_NEWEMPTYGAME;
	strcpy(packet.gamename, lpGameName);
	strcpy(packet.gamepass, lpGamePass);
	strcpy(packet.gamedesc, lpGameDesc);
	packet.dwGameFlag = dwGameFlag;
	packet.bTemplate = bTemplate;
	packet.bReserved1 = bReserved1;
	packet.bReserved2 = bReserved2;
	packet.GameId = *pwGameId;
	//EnterCriticalSection(&GELock);
	D2GSEventLog(__FUNCTION__, "Sending New Empty Game Packet seqno = %d", packet.h.seqno);
	SetSeq(packet.h.seqno%SEQ_TABLE_SIZE, NULL);
	if(!SendGEPacket(geinfo, &packet, sizeof(t_d2gs_d2ge_newemptygame)))
	{
		return FALSE;
	}
	rpacket = WaitPacketReturn(packet.h.seqno, geinfo, "NewEmptyGame");
	if (rpacket == NULL) return FALSE;
	//LeaveCriticalSection(&GELock);
	*pwGameId = tsfGameId(rpacket->GameId, geinfo->id);
	
	if(rpacket){
		if (rpacket->result == FALSE)
			rtn = -1;
		else{
			rtn = geinfo->id;
			geinfo->curr_game += 1;
		}
		free(rpacket);
	}else{
		return -1;
	}
	return rtn;
};

DWORD D2GSSendClientChatMessage(DWORD dwClientId, DWORD dwType, DWORD dwColor, LPCSTR lpName, LPCSTR lpText){
	DWORD rtn = 0;
	struct GEINFO* geinfo = GetClientIdOnGE(dwClientId);
	char* buf = (char*)malloc(sizeof(t_d2gs_d2ge_chatmsg) + strlen(lpText)+1);
	t_d2gs_d2ge_chatmsg packet;
	t_d2ge_d2gs_chatmsg_return *rpacket;
	if (!geinfo->enabled) return FALSE;
	packet.h.seqno = InterlockedIncrement(&seqcount);
	packet.h.type = D2GS_D2GE_CHATMSG;
	packet.dwClientId = RtsfGameId(dwClientId);
	packet.dwType = dwType;
	packet.dwColor = dwColor;
	strcpy(packet.name, lpName);
	packet.TextLen = strlen(lpText);
	memcpy(buf, &packet, sizeof(t_d2gs_d2ge_chatmsg));
	strcpy(buf+sizeof(t_d2gs_d2ge_chatmsg), lpText);
	//EnterCriticalSection(&GELock);
	//D2GSEventLog(__FUNCTION__, "Sending Chat Message Packet seqno = %d", packet.h.seqno);
	//SetSeq(packet.h.seqno%SEQ_TABLE_SIZE, NULL);
	SendGEPacket(geinfo, buf, sizeof(t_d2gs_d2ge_chatmsg) + strlen(lpText) + 1);
	free(buf);
	//rpacket = WaitPacketReturn(packet.h.seqno, geinfo, "ChatMsg");
	//LeaveCriticalSection(&GELock);
	//if(rpacket){
	//	rtn = rpacket->result;
	//	free(rpacket);
	//}else{
	//	rtn = FALSE;
	//}
	return rtn;
}

void D2GSNewClientComing(struct GEINFO* ge, SOCKET s) {
	t_d2gs_d2ge_incoming_client packet;
	packet.h.seqno = InterlockedIncrement(&seqcount);
	packet.h.type = D2GS_D2GE_INCOMING_CLIENT;
	packet.s = s;
 	SendGEPacket(ge, (char*)&packet, sizeof(t_d2gs_d2ge_incoming_client));
 	D2GSEventLog(__FUNCTION__, "Incoming client (socket: 0x%x) sent to GE", (int)s);
}