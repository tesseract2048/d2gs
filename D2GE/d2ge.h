#ifndef INCLUDED_D2GE_H
#define INCLUDED_D2GE_H


/* const */
#define D2GE_INIT_TIMEOUT		(60*1000)
#define D2GE_SHUT_TIMEOUT		(5*1000)

/* functions */
int   D2GEStartup(void);
int   D2GECleanup(void);
int   D2GEThreadInit(void);
void  D2GESetInitInfo(LPCSTR acstring);
void  D2GELoadConfig(LPCSTR configfile);
DWORD WINAPI D2GEThread(LPVOID lpParameter);

/* local function */
static BOOL D2GSGetInterface(void);
static DWORD __stdcall D2GSErrorHandle(void);
void TriggerDC();
void ShowSOJMsg(int counter);

void D2GEPatch();
short get_trunk_port();

/* functions in d2server.dll, got by QueryInterface() */
D2GSStartFunc		 			D2GSStart;						//DONE
D2GSSendDatabaseCharacterFunc 	D2GSSendDatabaseCharacter;
D2GSRemoveClientFromGameFunc	D2GSRemoveClientFromGame;
D2GSNewEmptyGameFunc			D2GSNewEmptyGame;				//DONE
D2GSEndAllGamesFunc				D2GSEndAllGames;
D2GSSendClientChatMessageFunc	D2GSSendClientChatMessage;
D2GSSetTickCountFunc			D2GSSetTickCount;				//DONE
D2GSSetACDataFunc				D2GSSetACData;					//DONE
D2GSLoadConfigFunc				D2GSLoadConfig;					//DONE
D2GSInitConfigFunc				D2GSInitConfig;					//DONE

#endif /* INCLUDED_D2GE_H */