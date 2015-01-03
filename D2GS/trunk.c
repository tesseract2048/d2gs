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

static SOCKET strunk;

BOOL WINAPI TrunkLoop();
BOOL WINAPI TrunkClientLoop(LPVOID lpParam);

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

BOOL WINAPI TrunkClientLoop(LPVOID lpParam)
{
	SOCKET Client = (SOCKET)lpParam;
	int ret, nRecv;
	int timeout = 4000;
	int i;
	GSPacket0xAF rpacket;
	GSPacket0x68 packet;
	D2CHARINFO		*lpChar;
	struct GEINFO   *ge;
	SOCKET dup_socket;
	DWORD start_count;
	rpacket.type = 0xAF;
	rpacket.suffix = 0x01;
	send(Client, (char*)&rpacket, sizeof(GSPacket0xAF), 0);
	start_count = GetTickCount();
	while (ioctlsocket(Client, FIONREAD, (u_long*)&nRecv) == 0) {
		if (nRecv == sizeof(GSPacket0x68)) break;
		if (GetTickCount() - start_count > 3000) {
			D2GSEventLog(__FUNCTION__, "Invalid packet or no packet received when waiting for 0x68");
			goto error;
		}
		Sleep(32);
	}
	nRecv = recv(Client, (char*)&packet, sizeof(GSPacket0x68), MSG_PEEK);
	if (nRecv != sizeof(GSPacket0x68)) {
		D2GSEventLog(__FUNCTION__, "Invalid packet received instead of 0x68");
		goto error;
	}
	if(packet.type != 0x68) goto error;
	lpChar = D2GSFindPendingCharByCharName((UCHAR*)packet.charname);
	if (!lpChar) {
		D2GSEventLog(__FUNCTION__, "Char not found: %s", packet.charname);
		goto error;
	}
	ge = GetGEById(lpChar->ge);
	if (DuplicateHandle(GetCurrentProcess(), (HANDLE)Client, ge->ge_proc, (HANDLE*)&dup_socket, 0, TRUE, DUPLICATE_SAME_ACCESS) == FALSE) {
		D2GSEventLog(__FUNCTION__, "Cannot duplicate handle");
		goto error;
	}
	D2GSNewClientComing(ge, dup_socket);
error:
	closesocket(Client);
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
