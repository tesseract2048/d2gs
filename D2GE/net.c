#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "d2gelib/d2server.h"
#include "d2gs.h"
#include "eventlog.h"
#include "net.h"
#include "vars.h"
#include "../D2GS/d2gs_d2ge_protocol.h"
#include "client.h"

#define SEQ_TABLE_SIZE 1024

static SOCKET					ge_comm;

struct sockaddr_in gs;

DWORD WINAPI GECommThread();

static int id;

static unsigned long			seqcount = 0;
static void *					seqtable[SEQ_TABLE_SIZE];
static CRITICAL_SECTION			packetHandler;
static CRITICAL_SECTION			packetSender;

BOOL SendGSPacket(char* buf, int len){
	int nBytesLeft = len;
	int nBytesSent = 0;
	int ret;
	EnterCriticalSection(&packetSender);
	send(ge_comm, &len, 4, 0);
	while(nBytesLeft > 0)
	{
		ret = send(ge_comm, buf + nBytesSent, nBytesLeft, 0);
		if(ret <= 0){
			int error = WSAGetLastError();
			D2GEEventLog(__FUNCTION__, "Error %d occured while sending GE packet", error);
			closesocket(ge_comm);
			ExitProcess(0);
			LeaveCriticalSection(&packetSender);
			return FALSE;
		}
		//D2GEEventLog(__FUNCTION__, "Sent %d bytes to GS", ret);
		nBytesSent += ret;
		nBytesLeft -= ret;
	}
	LeaveCriticalSection(&packetSender);
	return TRUE;
}

void handle_greet_reply(char* buf)
{
	t_d2gs_d2ge_greet_reply *packet = buf;
	t_d2ge_d2gs_max_game rpacket;
	memcpy(&d2gsconf, &packet->d2gsconfig, sizeof(D2GSCONFIGS));
	D2GEStartup();
	D2GSLoadConfig(&packet->configfile);
	D2GSInitConfig();
	D2GSSetTickCount(time(NULL));
	D2GSSetACData("bogus_ac_string");
	rpacket.h.type = D2GE_D2GS_MAX_GAME;
	rpacket.h.seqno = 0;
	rpacket.d2ge_id = id;
	rpacket.max_game = d2gsconf.gemaxgames;
	rpacket.port = get_trunk_port();
	SendGSPacket(&rpacket, sizeof(t_d2ge_d2gs_max_game));
	D2GSEndAllGames();
}

BOOL my_CreateEmptyGame(LPCSTR lpGameName, LPCSTR lpGamePass, LPCSTR lpGameDesc, DWORD dwGameFlag, BYTE  bTemplate, BYTE bReserved1, BYTE bReserved2, LPWORD pwGameId)
{
	__asm{
		mov edi, dwGameFlag
	}
	return D2GSNewEmptyGame(lpGameName, lpGamePass, lpGameDesc, dwGameFlag, bTemplate, bReserved1, bReserved2, pwGameId);
}

void handle_newemptygame(char* buf)
{
	t_d2gs_d2ge_newemptygame *packet = buf;
	t_d2ge_d2gs_newemptygame_return rpacket;
	DWORD GameID = packet->GameId;
	rpacket.result = my_CreateEmptyGame(packet->gamename, packet->gamepass, packet->gamedesc, packet->dwGameFlag, packet->bTemplate, packet->bReserved1, packet->bReserved2, &GameID);
	rpacket.h.type = D2GE_D2GS_NEWEMPTYGAME_RETURN;
	rpacket.h.seqno = packet->h.seqno;
	rpacket.GameId = GameID;
	SendGSPacket(&rpacket, sizeof(t_d2ge_d2gs_newemptygame_return));
}

void handle_sendchar(char* buf)
{
	t_d2gs_d2ge_sendchar *packet = buf;
	t_d2ge_d2gs_sendchar_return rpacket;
	LPPLAYERINFO pi;
	char* data = buf + sizeof(t_d2gs_d2ge_sendchar);
	if (!packet->dwSize)
	{
		data = NULL;
	}
	if (packet->lpPlayerInfo.CharName[0] == 0)
		pi = NULL;
	else
		pi = &packet->lpPlayerInfo;
	rpacket.h.type = D2GE_D2GS_SENDCHAR_RETURN;
	rpacket.h.seqno = packet->h.seqno;
	rpacket.result = D2GSSendDatabaseCharacter(packet->dwClientId, data, packet->dwSize, packet->dwTotalSize, packet->bLock, packet->dwReserved1, pi, packet->dwReserved2);
	//printf("Sending SENDCHAR return %d\n", rpacket.h.seqno);
	SendGSPacket(&rpacket, sizeof(t_d2ge_d2gs_sendchar_return));
}

void handle_removeclient(char* buf)
{
	t_d2gs_d2ge_removeclient *packet = buf;
	D2GSRemoveClientFromGame(packet->dwClientId);
}

void handle_endallgames(char* buf)
{
	D2GSEndAllGames();
}

void handle_chatmsg(char* buf)
{
	t_d2gs_d2ge_chatmsg *packet = buf;
	t_d2ge_d2gs_chatmsg_return rpacket;
	char* data = buf + sizeof(t_d2gs_d2ge_chatmsg);
	rpacket.h.type = D2GE_D2GS_CHATMSG_RETURN;
	rpacket.h.seqno = packet->h.seqno;
	rpacket.result = D2GSSendClientChatMessage(packet->dwClientId, packet->dwType, packet->dwColor, packet->name, data);
	SendGSPacket(&rpacket, sizeof(t_d2ge_d2gs_chatmsg_return));
}

void handle_callback(char* buf, int size)
{
	char* result = (char*)malloc(size);
	t_d2gs_d2ge_header *rtn = result;
	memcpy(result, buf, size);
	seqtable[rtn->seqno%SEQ_TABLE_SIZE] = result;
}

BOOL send_findtoken(LPCSTR lpCharName, DWORD dwToken, WORD wGameId,
					LPSTR lpAccountName, LPPLAYERDATA lpPlayerData)
{
	BOOL rtn;
	t_d2ge_d2gs_findtoken packet;
	t_d2gs_d2ge_findtoken_callback *rpacket;
	packet.h.seqno = InterlockedIncrement(&seqcount);
	packet.h.type = D2GE_D2GS_FINDTOKEN;
	strcpy(packet.charname, lpCharName);
	packet.dwToken = dwToken;
	packet.wGameId = wGameId;
	seqtable[packet.h.seqno%SEQ_TABLE_SIZE] = NULL;
	if (!SendGSPacket(&packet, sizeof(t_d2ge_d2gs_findtoken)))
	{
		return FALSE;
	}
	while(seqtable[packet.h.seqno%SEQ_TABLE_SIZE] == NULL)
	{
		Sleep(1);
	}
	rpacket = seqtable[packet.h.seqno%SEQ_TABLE_SIZE];
	strcpy(lpAccountName, rpacket->accountname);
	memcpy(lpPlayerData, &rpacket->lpPlayerData, sizeof(PLAYERDATA));
	rtn = rpacket->result;
	free(rpacket);
	return rtn;
};


void send_closegame(WORD wGameId)
{
	t_d2ge_d2gs_closegame packet;
	packet.h.seqno = InterlockedIncrement(&seqcount);
	packet.h.type = D2GE_D2GS_CLOSEGAME;
	packet.wGameId = wGameId;
	SendGSPacket(&packet, sizeof(t_d2ge_d2gs_closegame));
};

void send_getchar(LPGAMEDATA lpGameData, LPCSTR lpCharName,
					DWORD dwClientId, LPCSTR lpAccountName)
{
	t_d2ge_d2gs_getchar packet;
	packet.h.seqno = InterlockedIncrement(&seqcount);
	packet.h.type = D2GE_D2GS_GETCHAR;
	memcpy(&packet.lpGameData, lpGameData, sizeof(GAMEDATA));
	strcpy(packet.charname, lpCharName);
	packet.dwClientId = dwClientId;
	packet.d2ge_id = id;
	strcpy(packet.accountname, lpAccountName);
	SendGSPacket(&packet, sizeof(t_d2ge_d2gs_getchar));
};

void send_savechar(LPGAMEDATA lpGameData, LPCSTR lpCharName,
					LPCSTR lpAccountName, LPVOID lpSaveData,
					DWORD dwSize, PLAYERDATA PlayerData)
{
	char* buf;
	t_d2ge_d2gs_savechar packet;
	packet.h.seqno = InterlockedIncrement(&seqcount);
	packet.h.type = D2GE_D2GS_SAVECHAR;
	memcpy(&packet.lpGameData, lpGameData, sizeof(GAMEDATA));
	strcpy(packet.charname, lpCharName);
	strcpy(packet.accountname, lpAccountName);
	packet.dwSize = dwSize;
	memcpy(&packet.PlayerData, &PlayerData, sizeof(PLAYERDATA));
	packet.SaveDataLen = dwSize;
	buf = (char*)malloc(sizeof(t_d2ge_d2gs_savechar) + packet.SaveDataLen);
	memcpy(buf, &packet, sizeof(t_d2ge_d2gs_savechar));
	memcpy(buf + sizeof(t_d2ge_d2gs_savechar), lpSaveData, packet.SaveDataLen);
	SendGSPacket(buf, sizeof(t_d2ge_d2gs_savechar) + packet.SaveDataLen);
	free(buf);
};

void send_entergame(WORD wGameId, LPCSTR lpCharName, WORD wCharClass, 
					DWORD dwCharLevel, DWORD dwReserved)
{
	t_d2ge_d2gs_entergame packet;
	packet.h.seqno = InterlockedIncrement(&seqcount);
	packet.h.type = D2GE_D2GS_ENTERGAME;
	packet.wGameId = wGameId;
	strcpy(packet.charname, lpCharName);
	packet.wCharClass = wCharClass;
	packet.dwCharLevel = dwCharLevel;
	packet.dwReserved = dwReserved;
	SendGSPacket(&packet, sizeof(t_d2ge_d2gs_entergame));
};

void send_updateladder(LPCSTR lpCharName, WORD wCharClass, 
						DWORD dwCharLevel, DWORD dwCharExpLow, 
						DWORD dwCharExpHigh,  WORD wCharStatus,
						PLAYERMARK PlayerMark)
{
	t_d2ge_d2gs_updateladder packet;
	packet.h.seqno = InterlockedIncrement(&seqcount);
	packet.h.type = D2GE_D2GS_UPDATELADDER;
	strcpy(packet.charname, lpCharName);
	packet.wCharClass = wCharClass;
	packet.dwCharLevel = dwCharLevel;
	packet.dwCharExpLow = dwCharExpLow;
	packet.dwCharExpHigh = dwCharExpHigh;
	packet.wCharStatus = wCharStatus;
	memcpy(&packet.PlayerMark, &PlayerMark, sizeof(PLAYERMARK));
	SendGSPacket(&packet, sizeof(t_d2ge_d2gs_updateladder));
};

void send_updateinfo(WORD wGameId, LPCSTR lpCharName, 
					WORD wCharClass, DWORD dwCharLevel)
{
	t_d2ge_d2gs_updateinfo packet;
	packet.h.seqno = InterlockedIncrement(&seqcount);
	packet.h.type = D2GE_D2GS_UPDATEINFO;
	packet.wGameId = wGameId;
	strcpy(packet.charname, lpCharName);
	packet.wCharClass = wCharClass;
	packet.dwCharLevel = dwCharLevel;
	SendGSPacket(&packet, sizeof(t_d2ge_d2gs_updateinfo));
};

void send_leavegame(LPGAMEDATA lpGameData, WORD wGameId, WORD wCharClass, 
					DWORD dwCharLevel, DWORD dwExpLow, DWORD dwExpHigh,
					WORD wCharStatus, LPCSTR lpCharName, LPCSTR lpCharPortrait,
					BOOL bUnlock, DWORD dwZero1, DWORD dwZero2,
					LPCSTR lpAccountName, PLAYERDATA PlayerData,
					PLAYERMARK PlayerMark)
{
	char* buf;
	t_d2ge_d2gs_leavegame packet;
	packet.h.seqno = InterlockedIncrement(&seqcount);
	packet.h.type = D2GE_D2GS_LEAVEGAME;
	memcpy(&packet.lpGameData, lpGameData, sizeof(GAMEDATA));
	packet.wGameId = wGameId;
	packet.wCharClass = wCharClass;
	packet.dwCharLevel = dwCharLevel;
	packet.dwExpLow = dwExpLow;
	packet.dwExpHigh = dwExpHigh;
	packet.wCharStatus = wCharStatus;
	strcpy(packet.charname, lpCharName);
	packet.bUnlock = bUnlock;
	packet.dwZero1 = dwZero1;
	packet.dwZero2 = dwZero2;
	strcpy(packet.accountname, lpAccountName);
	memcpy(&packet.PlayerData, &PlayerData, sizeof(PLAYERDATA));
	memcpy(&packet.PlayerMark, &PlayerMark, sizeof(PLAYERMARK));
	packet.PortraitLen = strlen(lpCharPortrait);
	buf = (char*)malloc(sizeof(t_d2ge_d2gs_leavegame) + packet.PortraitLen + 1);
	memcpy(buf, &packet, sizeof(t_d2ge_d2gs_leavegame));
	strcpy(buf + sizeof(t_d2ge_d2gs_leavegame), lpCharPortrait);
	seqtable[packet.h.seqno%SEQ_TABLE_SIZE] = NULL;
	SendGSPacket(buf, sizeof(t_d2ge_d2gs_leavegame) + packet.PortraitLen + 1);
	free(buf);
};

void handle_incoming_client(char* buf)
{
	t_d2gs_d2ge_incoming_client *packet = buf;
	push_client((SOCKET)packet->s);
	D2GEEventLog(__FUNCTION__, "Incoming client (socket: 0x%x) received", (int)packet->s);
}
DWORD WINAPI GECommThread()
{
	char buf[32768];
	char NoDelay = 1;
	int packetlen;
	BOOL inpacket = FALSE;
	int bufpos = 0;
	t_d2gs_d2ge_header *header;
	int Block = 0;
	if(ioctlsocket(ge_comm, FIONBIO, &Block) == SOCKET_ERROR)
		return FALSE;
	setsockopt(ge_comm, IPPROTO_TCP, TCP_NODELAY, &NoDelay, sizeof(char));
	while(1)
	{
		if (!inpacket)
		{
			int recvlen = recv(ge_comm, &packetlen, 4, MSG_WAITALL);
			//D2GEEventLog(__FUNCTION__, "Notified a packet of %d bytes", packetlen);
			bufpos = 0;
			inpacket = TRUE;
		}else{
			int recvlen = recv(ge_comm, &buf[bufpos], packetlen, 0);
			if(recvlen == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				D2GEEventLog(__FUNCTION__, "Error %d occured while receiving GE packet", error);
				closesocket(ge_comm);
				ExitProcess(0);
				return 0;
			}
			//D2GEEventLog(__FUNCTION__, "Recevied %d bytes from GS, packetlen = %d, bufpos = %d", recvlen, packetlen, bufpos);
			packetlen -= recvlen;
			bufpos += recvlen;
			if (packetlen > 0) continue;
			header = buf;
			EnterCriticalSection(&packetHandler);
			//D2GEEventLog(__FUNCTION__, "seqno = %d", header->seqno);
			switch (header->type)
			{
			case D2GS_D2GE_GREET_REPLY:
				//D2GEEventLog(__FUNCTION__, "D2GS_D2GE_GREET_REPLY");
				handle_greet_reply(buf);
				break;
			case D2GS_D2GE_NEWEMPTYGAME:
				//D2GEEventLog(__FUNCTION__, "D2GS_D2GE_NEWEMPTYGAME");
				handle_newemptygame(buf);
				break;
			case D2GS_D2GE_SENDCHAR:
				//D2GEEventLog(__FUNCTION__, "D2GS_D2GE_SENDCHAR");
				handle_sendchar(buf);
				break;
			case D2GS_D2GE_REMOVECLIENT:
				//D2GEEventLog(__FUNCTION__, "D2GS_D2GE_REMOVECLIENT");
				handle_removeclient(buf);
				break;
			case D2GS_D2GE_ENDALLGAMES:
				//D2GEEventLog(__FUNCTION__, "D2GS_D2GE_ENDALLGAMES");
				handle_endallgames(buf);
				break;
			case D2GS_D2GE_CHATMSG:
				//D2GEEventLog(__FUNCTION__, "D2GS_D2GE_CHATMSG");
				handle_chatmsg(buf);
				break;
			case D2GS_D2GE_FINDTOKEN_CALLBACK:
				//D2GEEventLog(__FUNCTION__, "D2GS_D2GE_FINDTOKEN_CALLBACK");
				handle_callback(buf, sizeof(t_d2gs_d2ge_findtoken_callback));
				break;
			case D2GS_D2GE_INCOMING_CLIENT:
				handle_incoming_client(buf);
				break;
			}
			LeaveCriticalSection(&packetHandler);
			inpacket = FALSE;
		}
	}
	return 0;
}

int D2GSNetInitialize(int d2ge_id)
{
	WORD		wVersionRequested;
	WSADATA		wsaData;
	DWORD		dwThreadId;
	unsigned long  Block = 1;
	t_d2ge_d2gs_greet packet;
	int			err;
	id = d2ge_id;
	wVersionRequested = MAKEWORD(2, 0);
	err = WSAStartup(wVersionRequested, &wsaData);
	d2gsconf.enablegelog = 1;
	if (err!=0 ) {
		D2GEEventLog("D2GSNetInitialize", "Error in initializing Winsock DLL");
	    return FALSE;
	}
	InitializeCriticalSection(&packetHandler);
	InitializeCriticalSection(&packetSender);
	gs.sin_family = AF_INET;
	gs.sin_port = htons(12835);
	gs.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(gs.sin_zero, 0 ,8);
	if((ge_comm = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		return FALSE;
	if(connect(ge_comm, (struct sockaddr *)&gs, sizeof(struct sockaddr_in)) == SOCKET_ERROR){
		err = WSAGetLastError();
		D2GEEventLog(__FUNCTION__, "Error %d occured while connecting to GS", err);
		return FALSE;
	}
	packet.d2ge_id = d2ge_id;
	packet.h.type = D2GE_D2GS_GREET;
	packet.h.seqno = 0;
	SendGSPacket(&packet, sizeof(t_d2ge_d2gs_greet));
	CreateThread(NULL, 0, GECommThread, NULL, 0, &dwThreadId);
	return TRUE;

} /* End of D2NetInitialize() */

int get_id()
{
	return id;
}