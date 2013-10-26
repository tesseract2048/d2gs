#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "d2gs.h"
#include "d2ge.h"
#include "eventlog.h"
#include "vars.h"
#include "net.h"
#include "handle_s2s.h"
#include "handle_d2game.h"
#include "debug.h"
#include "d2gamelist.h"
#include "d2game_protocol.h"
#include "trunk.h"
#include "nfd.h"

HANDLE trunkPipe;

DWORD WINAPI TrunkWorker(LPVOID lpParam)
{
	char charname[17];
	int bytesRead;
	while (1)
	{
		if (ReadFile(trunkPipe, charname, 17, (LPDWORD)&bytesRead, NULL))
		{
			short port = 0;
#ifdef _STRESS_TEST
			DWORD dwGameId;
			int ge;
			printf("Creating %s\n", charname);
			ge = D2GSNewEmptyGame(charname, "", "", 0x300004, 0x11, 0x22, 0x33, &dwGameId);
			D2GSGameListInsert((UCHAR*)charname, (UCHAR*)"", (UCHAR*)"", (UCHAR*)charname, (UCHAR*)charname, (UCHAR*)"127.0.0.1", 1, 1,
					0, 0, dwGameId, ge);
			port = GetGEById(ge)->trunk_port;
#else
			D2CHARINFO* player = D2GSFindPendingCharByCharName((UCHAR*)charname);
			if (player != NULL)
			{
				port = GetGEById(player->ge)->trunk_port;
			}
#endif
			WriteFile(trunkPipe, &port, 2, (LPDWORD)&bytesRead, NULL);
		}
		else
		{
			int error = GetLastError();
			HANDLE StopEvent;
			if (error == ERROR_BROKEN_PIPE || error == ERROR_INVALID_HANDLE)
			{
				D2GSEventLog(__FUNCTION__, "Broken trunk detected, self destruction activated.");
				StopEvent = CreateEventA(0, 1, 0, D2GS_STOP_EVENT_NAME);
				if (StopEvent)
				{
					SetEvent(StopEvent);
					CloseHandle(StopEvent);
				}
				return 0;
			}
		}
	}
}

int StartupTrunk()
{
	D2GSEventLog(__FUNCTION__, "Starting up Trunk...");
	trunkPipe = CreateFileA("\\\\.\\pipe\\d2gs_trunk_pipe_13", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (!trunkPipe) return -1;
	CreateThread(NULL, 0, TrunkWorker, NULL, NULL, NULL);
	return 1;
}
