/*
 * main.c: main routine of this program
 * 
 * 2001-08-20 faster
 *   add initialization routine and main loop of this program
 */

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <conio.h>
#include "d2gelib/d2server.h"
#include "d2gs.h"
#include "eventlog.h"
#include "config.h"
#include "d2ge.h"
#include "net.h"
#include "client.h"
#include "hp.h"

/* CTRL+C or CTRL+Break signal handler */
BOOL WINAPI ControlHandler(DWORD dwCtrlType);

/********************************************************************************
 * Main procedure begins here
 ********************************************************************************/
#ifdef _DEBUG
int main(int argc, char **argv)
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
#endif
{
	DWORD	dwWait;
	int geid;
#ifdef _DEBUG
	if (argc != 2) return;
	geid = atoi(argv[1]);
#else
	geid = atoi(lpCmdLine);
#endif
	/* init log system first */
	if (!D2GEEventLogInitialize(geid)) return -1;

	/* setup signal capture */
	SetConsoleCtrlHandler(ControlHandler, TRUE);
	
	/* init client */
	init_client();

	/* init hack prevent */
	hp_init();

	/* initialize the net connection */
	if (!D2GSNetInitialize(geid)) {
		D2GEEventLog("main", "Failed Startup Net Connector");
		return -1;
	}
	
	/*CreateThread(NULL, 0, DCStressTest, NULL, NULL, NULL);
	CreateThread(NULL, 0, DCStressTest, NULL, NULL, NULL);
	CreateThread(NULL, 0, DCStressTest, NULL, NULL, NULL);
	CreateThread(NULL, 0, DCStressTest, NULL, NULL, NULL);
	CreateThread(NULL, 0, DCStressTest, NULL, NULL, NULL);
	CreateThread(NULL, 0, DCStressTest, NULL, NULL, NULL);*/

	while(1) {
		Sleep(1000);
	}

	return 0;

} /* End of main() */


/*********************************************************************
 * Purpose: catch CTRL+C or CTRL+Break signal
 * Return: TRUE or FALSE
 *********************************************************************/
BOOL WINAPI ControlHandler(DWORD dwCtrlType)
{
	switch( dwCtrlType )
	{
		case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
		case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
			D2GEEventLog("ControlHandler", "CTRL_BREAK or CTRL_C event caught");
			ExitProcess(0);
			return TRUE;
			break;
    }
    return FALSE;

} /* End of ControlHandler */


void patchAddr(DWORD addr, BYTE val){
	DWORD oldFlag, dummy;
	VirtualProtect((LPVOID)addr, 1, PAGE_EXECUTE_READWRITE, &oldFlag);
	memcpy(addr, &val, 1);
	VirtualProtect((LPVOID)addr, 1, oldFlag, &dummy);
}

DWORD searchAndPatch(DWORD startAddr, DWORD searchRange, unsigned char* pattern, DWORD patternLen, unsigned char* patch, DWORD patchLen){
	unsigned char buf[255];
	DWORD range = 0;
	DWORD dstAddr = startAddr;
	memcpy(buf, startAddr, 5);
	if(buf[0] == 0xE9){
		DWORD jmprange;
		memcpy(&jmprange, &buf[1], 4);
		dstAddr += jmprange;
	}
	while(range < searchRange && ++dstAddr && ++range){
		if(memcmp(dstAddr, pattern, patternLen) == 0){
			DWORD oldFlag, dummy;
			VirtualProtect((LPVOID)dstAddr, patchLen, PAGE_EXECUTE_READWRITE, &oldFlag);
			memcpy(dstAddr, patch, patchLen);
			VirtualProtect((LPVOID)dstAddr, patchLen, oldFlag, &dummy);
			return dstAddr;
		}
	}
	return -1;
}
