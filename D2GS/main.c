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
#include <dbghelp.h>
#include "d2gs.h"
#include "eventlog.h"
#include "vars.h"
#include "config.h"
#include "d2ge.h"
#include "net.h"
#include "timer.h"
#include "d2gamelist.h"
#include "handle_s2s.h"
#include "trunk.h"
#include "hexdump.h"


/* function declarations */
int  DoCleanup(void);
BOOL D2GSCheckRunning(void);
int  CleanupRoutineForServerMutex(void);
/* CTRL+C or CTRL+Break signal handler */
BOOL WINAPI ControlHandler(DWORD dwCtrlType);


/* some variables used just in this file */
static HANDLE			hD2GSMutex  = NULL;
static HANDLE			hStopEvent  = NULL;
static CLEANUP_RT_ITEM	*pCleanupRT = NULL;
static LONG WINAPI except_handler(PEXCEPTION_POINTERS lpEP);
/********************************************************************************
 * Main procedure begins here
 ********************************************************************************/
#ifdef _DEBUG
int main(int argc, char **argv)
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
#endif
{
	char	curr_dir[255];
	char	patch_d2[255];
	DWORD	dwWait;
	HANDLE	handle;
	int i;
	int size;
	SetUnhandledExceptionFilter(except_handler);

	/* reset cleanup routine list */
	pCleanupRT = NULL;

	/* init log system first */
	if (!D2GSEventLogInitialize()) return -1;
	
	GetModuleFileName(NULL, curr_dir, 255);
	for(i = strlen(curr_dir) - 1; i >= 0; i --)
	{
		if (curr_dir[i] == '\\'){
			curr_dir[i] = 0;
			break;
		}
	}
	sprintf(patch_d2, "%s\\Patch_D2.MPQ", curr_dir);
	handle = CreateFile(patch_d2, FILE_GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (handle == INVALID_HANDLE_VALUE)
	{
		D2GSEventLog("main", "no Patch_D2.MPQ found");
		//return -1;
	}
	size = GetFileSize(handle, NULL);
	if (size != 2399610)
	{
		D2GSEventLog("main", "Patch_D2.MPQ has incorrect size, should be 2399610");
		//CloseHandle(handle);
		//return -1;
	}
	CloseHandle(handle);

	//memcpy(0x2134, &hD2GSMutex, 256);
	/* setup signal capture */
	SetConsoleCtrlHandler(ControlHandler, TRUE);

	hD2GSMutex = CreateMutex(NULL, TRUE, D2GSERVER_MUTEX_NAME);

	/* create a name event, for "d2gssvc" server controler to terminate me */
	hStopEvent= CreateEvent(NULL, TRUE, FALSE, D2GS_STOP_EVENT_NAME);
	if (!hStopEvent) {
		D2GSEventLog("main", "failed create stop event object");
		DoCleanup();
		return -1;
	}

	/* init variables */
	if (!D2GSVarsInitialize()) {
		D2GSEventLog("main", "Failed initialize global variables");
		DoCleanup();
		return -1;
	}

	sprintf(d2gsconf.versionMotd, "Welcome to 91D2. Nice hunting!", BUILDDATE);

	/* read configurations */
	if (!D2GSReadConfig()) {
		D2GSEventLog("main", "Failed getting configurations from registry");
		DoCleanup();
		return -1;
	}

	/* create timer */
	if (!D2GSTimerInitialize()) {
		D2GSEventLog("main", "Failed Startup Timer");
		DoCleanup();
		return -1;
	}

	/*if (!D2GEStartup()) {
		D2GSEventLog("main", "Failed Startup Game Engine");
		DoCleanup();
		return -1;
	}*/

	/* initialize the net connection */
	if (!D2GSNetInitialize()) {
		D2GSEventLog("main", "Failed Startup Net Connector");
		DoCleanup();
		return -1;
	}

	/* administration console */
	/*if (!D2GSAdminInitialize()) {
		D2GSEventLog("main", "Failed Startup Administration Console");
		DoCleanup();
		return -1;
	}*/

	/* main server loop */
	D2GSEventLog("main", "Entering Main Server Loop");

	while(TRUE) {
		dwWait = WaitForSingleObject(hStopEvent, 1000);
		if (dwWait!=WAIT_OBJECT_0) continue;
		/* service controler tell me to stop now. "Yes, sir!" */
		D2GSEndAllGames();
		D2GSSetD2CSMaxGameNumber(0);
		D2GSActive(FALSE);
		D2GSEventLog("main", "I am going to stop");
		Sleep(5000);
		break;
	}

	DoCleanup();
	return 0;

} /* End of main() */


/*********************************************************************
 * Purpose: to add an cleanup routine item to the list
 * Return: TRUE(success) or FALSE(failed)
 *********************************************************************/
int CleanupRoutineInsert(CLEANUP_ROUTINE pRoutine, char *comment)
{
	CLEANUP_RT_ITEM		*pitem;

	if (pRoutine==NULL) return FALSE;
	pitem = (CLEANUP_RT_ITEM *)malloc(sizeof(CLEANUP_RT_ITEM));
	if (!pitem) {
		D2GSEventLog("CleanupRoutineInsert", "Can't alloc memory");
		return FALSE;
	}
	ZeroMemory(pitem, sizeof(CLEANUP_RT_ITEM));

	/* fill the structure */
	if (comment)
		strncpy(pitem->comment, comment, sizeof(pitem->comment)-1);
	else
		strncpy(pitem->comment, "unknown", sizeof(pitem->comment)-1);
	pitem->cleanup = pRoutine;
	pitem->next = pCleanupRT;
	pCleanupRT = pitem;

	return TRUE;

} /* End of CleanupRoutineInsert() */


/*********************************************************************
 * Purpose: call the cleanup routine to do real cleanup work
 * Return: TRUE or FALSE
 *********************************************************************/
int DoCleanup(void)
{
	CloseHandle(hD2GSMutex);
	D2GECleanup();
	return TRUE;

} /* End of DoCleanup() */

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
			D2GSEventLog("ControlHandler", "CTRL_BREAK or CTRL_C event caught");
			DoCleanup();
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

void dump_callstack( CONTEXT *context )
{
    STACKFRAME sf;
    DWORD machineType = IMAGE_FILE_MACHINE_I386;
    
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();
	
	BYTE symbolBuffer[ sizeof( SYMBOL_INFO ) + 1024 ];
    PSYMBOL_INFO pSymbol = ( PSYMBOL_INFO ) symbolBuffer;
    DWORD64 symDisplacement = 0;
    IMAGEHLP_LINE lineInfo = { sizeof(IMAGEHLP_LINE) };
    DWORD dwLineDisplacement;

	HANDLE dump;
	dump = fopen("D2GSCrash.log", "w");

    memset( &sf, 0, sizeof( STACKFRAME ) );
    
    sf.AddrPC.Offset = context->Eip;
    sf.AddrPC.Mode = AddrModeFlat;
    sf.AddrStack.Offset = context->Esp;
    sf.AddrStack.Mode = AddrModeFlat;
    sf.AddrFrame.Offset = context->Ebp;
    sf.AddrFrame.Mode = AddrModeFlat;
    
	fprintf(dump, "Fatal Address: 0x%x", context->Eip);
	fflush(dump);
	fprintf(dump, "eax: %x  ebx: %x  ecx: %x  edx: %x  flg: %x", context->Eax, context->Ebx, context->Ecx, context->Edx, context->EFlags);
	fflush(dump);
	fprintf(dump, "ebp: %x  edi: %x  eip: %x  esi: %x  esp: %x", context->Ebp, context->Edi, context->Eip, context->Esi, context->Esp);
	fflush(dump);
	fprintf(dump, "Stack walk:");
	fflush(dump);
    for( ; ; )
    {
        if( !StackWalk(machineType, hProcess, hThread, &sf, context, 0, SymFunctionTableAccess, SymGetModuleBase, 0 ) )
        {
            break;
        }
        
        if( sf.AddrFrame.Offset == 0 )
        {
            break;
        }
        
        pSymbol->SizeOfStruct = sizeof( symbolBuffer );
        pSymbol->MaxNameLen = 1024;
        
        fprintf(dump, "  Call stack: 0x%x", sf.AddrPC.Offset);
	fflush(dump);

        if( SymFromAddr( hProcess, sf.AddrPC.Offset, 0, pSymbol ) )
        {
			fprintf(dump, "    Symbol: %s (base: 0x%x)", pSymbol->Name, pSymbol->ModBase);
	fflush(dump);
        }
        
        
        if( SymGetLineFromAddr( hProcess, sf.AddrPC.Offset, &dwLineDisplacement, &lineInfo ) )
        {
			fprintf(dump, "    Source: Line %d in %s", lineInfo.LineNumber, lineInfo.FileName);
	fflush(dump);
        }
    }

	fclose(dump);

}

static LONG WINAPI except_handler(PEXCEPTION_POINTERS lpEP)
{
	SetErrorMode(SEM_NOGPFAULTERRORBOX);
	SymInitialize(GetCurrentProcess(), NULL, TRUE);
	dump_callstack(lpEP->ContextRecord);
    SymCleanup(GetCurrentProcess());
	ExitProcess(0);
	return EXCEPTION_EXECUTE_HANDLER;
}
