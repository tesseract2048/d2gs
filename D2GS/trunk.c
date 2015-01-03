#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "d2gs.h"
#include "d2ge.h"
#include "eventlog.h"
#include "vars.h"
#include "net.h"
#include "handle_s2s.h"
#include "debug.h"
#include "d2gamelist.h"
#include "trunk.h"
#include "nfd.h"

#define TRUNK_BUFSIZE 8192

static SOCKET strunk;

BOOL WINAPI TrunkLoop();
BOOL WINAPI TrunkClientLoop(LPVOID lpParam);

SOCKET ConnectHost(DWORD dwIP, WORD wPort)
{
	char nodelay = 1;
	SOCKET sockid;
	struct sockaddr_in srv_addr;
	if ((sockid = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET)
		return 0;
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.S_un.S_addr = dwIP;
	srv_addr.sin_port = htons(wPort);
	if (connect(sockid,(struct sockaddr*)&srv_addr,sizeof(struct sockaddr_in)) == SOCKET_ERROR)
		goto error;
	setsockopt(sockid, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(char));
	return sockid;
error:
	closesocket(sockid);
	return 0;
}

BOOL StartupTrunk()
{
	HANDLE hThread;
	DWORD dwThreadId;
	struct sockaddr_in srv_addr = {0};
	if ((strunk = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET)
		return FALSE;
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.S_un.S_addr = inet_addr(d2gsconf.listenaddr);
	srv_addr.sin_port = htons(d2gsconf.listenport);
	if (bind(strunk, (struct sockaddr*)&srv_addr, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
		return FALSE;
	listen(strunk, 3);
	hThread = CreateThread(NULL, 0, TrunkLoop, NULL, NULL, &dwThreadId);
	if(hThread)
		CloseHandle(hThread);
	return TRUE;
}

int DataSend(SOCKET s, char *DataBuf, int DataLen)
{
	int nBytesLeft = DataLen;
	int nBytesSent = 0;
	int ret;
	int iMode = 0;
	int i;
	ioctlsocket(s, FIONBIO, (u_long FAR*) &iMode);
	while(nBytesLeft > 0)
	{
		ret = send(s, DataBuf + nBytesSent, nBytesLeft, 0);
		if(ret <= 0)
			break;
		nBytesSent += ret;
		nBytesLeft -= ret;
	}
	return nBytesSent;
}

int DataRecv(SOCKET s, char *DataBuf, int DataLen, int flags)
{
	int i;
	int rtn;
	rtn = recv(s, DataBuf, DataLen, flags);
	return rtn;
}

static unsigned char HSAuth[] = {0x13, 0x49, 0x03, 0xE8, 0x9C, 0x21, 0x65, 0x77, 0x8A, 0xBE};

BOOL WINAPI TrunkClientLoop(LPVOID lpParam)
{
	SOCKET Client = (SOCKET)lpParam;
	SOCKET Server;
	BOOL ServerConnect = FALSE;
	char RecvBuf[TRUNK_BUFSIZE];
	char nfdBuf[TRUNK_BUFSIZE*2+3];
	fd_set Fd_Read;
	int ret, nRecv;
	int timeout = 4000;
	int i;
	int nfdpos = 0;
	char nodelay = 1;
	char GSSeed;
	char GSKey[9];
	GSPacket0xAF rpacket;
	memset(nfdBuf, 0, 3);
	setsockopt(Client, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(char));
	setsockopt(Server, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

	//Send Key
	/*srand(GetTickCount());
	GSSeed = rand() & 0xFF;
	for (i = 0; i < 9; i ++)
	{
		GSKey[i] = rand() & 0xFF;
	}
	DataSend(Client, &GSSeed, 1);
	DataSend(Client, GSKey, 9);

	//Recv Handshake
	nRecv = DataRecv(Client, RecvBuf, 10, MSG_WAITALL);
	if (nRecv != 10)
	{
		goto error;
	}

	for (i = 0; i < 10; i ++)
	{
		char c = HSAuth[i] ^ (char)((GSKey[i % 9] + i * i * GSSeed + 0x03) & 0xFF);
		if (RecvBuf[i] != c)
		{
			goto error;  
		}
	}*/

	rpacket.type = 0xAF;
	rpacket.suffix = 0x01;
	DataSend(Client, &rpacket, sizeof(GSPacket0xAF), 0);
	while(1)
	{
		FD_ZERO(&Fd_Read);
		FD_SET(Client, &Fd_Read);
		if (ServerConnect)
			FD_SET(Server, &Fd_Read);
		ret = select(0, &Fd_Read, NULL, NULL, NULL);
		if(ret <= 0)
			goto error;
		if(FD_ISSET(Client, &Fd_Read))
		{
			if (ServerConnect)
			{
				nRecv = DataRecv(Client, RecvBuf, TRUNK_BUFSIZE, 0);
				if (nRecv > 0)
				{
					memcpy(&nfdBuf[nfdpos], RecvBuf, nRecv);
					if (nfd_filter(nfdBuf, nfdpos - 3, nfdpos + nRecv - 1) != 0)
					{
						goto error;
					}
					nfdpos += nRecv;
					if (nfdpos > TRUNK_BUFSIZE-1)
					{
						memcpy(nfdBuf, &nfdBuf[nfdpos-3], 3);
						nfdpos = 3;
					}
				}
			}
			else
			{
				nRecv = DataRecv(Client, RecvBuf, sizeof(GSPacket0x68), MSG_WAITALL);
				if (nRecv != sizeof(GSPacket0x68)) goto error;
			}
			if(nRecv <= 0)
				goto error;
			if (!ServerConnect)
			{
				D2CHARINFO		*lpChar;
				GSPacket0x68    *packet = RecvBuf;
				struct GEINFO   *ge;
				int iMode = 0;
				
				if(packet->type != 0x68) goto error;
				//RecvBuf[0] = 0x68;
				lpChar = D2GSFindPendingCharByCharName(packet->charname);
				if (!lpChar) {
					goto error;
				}
				ge = GetGEById(lpChar->ge);
				Server = ConnectHost(inet_addr("127.0.0.1"), ge->trunk_port);
				if (!Server) {
					goto error;
				}
				if(ioctlsocket(Server, FIONBIO, &iMode) == SOCKET_ERROR){
					goto error;
				}
				recv(Server, &rpacket, sizeof(GSPacket0xAF), MSG_WAITALL);
				ServerConnect = TRUE;
			}
			if (ServerConnect)
			{
				ret = DataSend(Server, RecvBuf, nRecv);
				if(ret == 0 || ret != nRecv)
					goto error;
			}
		}
		if (ServerConnect)
		{
			if(FD_ISSET(Server, &Fd_Read))
			{
				nRecv = recv(Server, RecvBuf, TRUNK_BUFSIZE, 0);
				if(nRecv <= 0)
					goto error;
				ret = DataSend(Client, RecvBuf, nRecv);
				if(ret == 0 || ret != nRecv)
					goto error;
			}
		}
		Sleep(1);
	}
error:
	closesocket(Client);
	if (ServerConnect)
		closesocket(Server);
	return 0;
}

BOOL WINAPI TrunkLoop()
{
	HANDLE hThread;
	SOCKET AcceptSocket;
	DWORD dwThreadId;
	while(1)
	{
		AcceptSocket = accept(strunk, NULL, NULL);
		if(AcceptSocket == INVALID_SOCKET)
			continue;
		hThread = CreateThread(NULL, 0, TrunkClientLoop, (LPVOID)AcceptSocket, NULL, &dwThreadId);
		if(hThread)
			CloseHandle(hThread);
	}
}

DWORD WINAPI MakeBenchmarkConnect(LPVOID lpParam)
{
	DWORD* param = lpParam;
	DWORD dwToken = param[0];
	char* CharName = param[1];
	char eos = 0;
	SOCKET s = ConnectHost(inet_addr("127.0.0.1"), 4000);
	GSPacket0xAF packet0xAF;
	GSPacket0x68 packet0x68;
	GSPacket0x6A packet0x6A;
	int iMode = 0;
	ioctlsocket(s, FIONBIO, (u_long FAR*) &iMode);

	recv(s, &packet0xAF, sizeof(GSPacket0xAF), MSG_WAITALL);
	packet0x68.type = 0x68;
	packet0x68.hash = 0xfb03094e;
	packet0x68.token = dwToken;
	packet0x68.charid = 0x03;
	packet0x68.ver = 0x0B;
	packet0x68.dwReserved1 = 0xED5DCC50;
	packet0x68.dwReserved2 = 0x91A519B6;
	packet0x68.bReserved = 9;
	strcpy(packet0x68.charname, CharName);
	send(s, &packet0x68, sizeof(GSPacket0x68), 0);
	send(s, &eos, 1, 0);
	recv(s, &packet0xAF, sizeof(GSPacket0xAF), MSG_WAITALL);
	packet0x6A.type = 0x6A;
	send(s, &packet0x6A, sizeof(GSPacket0x6A), 0);
}