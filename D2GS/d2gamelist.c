#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "d2gs.h"
#include "vars.h"
#include "eventlog.h"
#include "charlist.h"
#include "d2gamelist.h"
#include "handle_s2s.h"
#include "utils.h"
#include "d2ge.h"


/* vars */
static D2GAMEINFO		*lpGameInfoHead   = NULL;
static D2CHARINFO		*lpPendingChar    = NULL;
static D2GETDATAREQUEST	*lpGetDataReqHead = NULL;
static DWORD			currentgamenum    = 0;

static char desc_game_difficulty[][32] = {
	"normal", "nightmare", "hell"
};
static char desc_char_class[][16] = {
	"Ama", "Sor", "Nec", "Pal", "Bar", "Dur", "Ass"
};

static LIST_HEAD(list_motd);

void CleanupGEGame(int d2ge_id)
{
	D2GAMEINFO	*lpGame;
	D2GAMEINFO  *pending[2048];
	int pending_count = 0;
	int i;

	EnterCriticalSection(&csGameList);
	lpGame = lpGameInfoHead;
	while(lpGame)
	{
		if (lpGame->ge == d2ge_id)
		{
			pending[pending_count++] = lpGame;
		}
		lpGame = lpGame->next;
	}
	LeaveCriticalSection(&csGameList);

	for(i = 0; i < pending_count; i ++)
	{
		D2GSCBCloseGame(pending[i]->GameId);
	}
}

void CleanupGEChar(int d2ge_id)
{
	D2CHARINFO		*lpChar;
	D2CHARINFO	    *pending[2048];
	int pending_count = 0;
	int i;

	EnterCriticalSection(&csGameList);
	lpChar = lpPendingChar;
	while(lpChar)
	{
		if (lpChar->ge == d2ge_id)
		{
			pending[pending_count++] = lpChar;
		}
		lpChar = lpChar->next;
	}
	LeaveCriticalSection(&csGameList);

	for(i = 0; i < pending_count; i ++)
	{
		D2GSDeletePendingChar(pending[i]);
	}
	return;
}

/*********************************************************************
 * Purpose: to reset the GameList
 * Return: None
 *********************************************************************/
void D2GSResetGameList(void)
{
	D2GAMEINFO		*ph, *pnext;
	D2CHARINFO		*pChar, *pCharNext;

	/* to release all the memory */
	EnterCriticalSection(&csGameList);
	ph = lpGameInfoHead;
	while(ph)
	{
		pnext = ph->next;
		D2GSDeleteAllCharInGame(ph);
		free(ph);
		ph = pnext;
	}
	pChar = lpPendingChar;
	while(pChar)
	{
		pCharNext = pChar->next;
		free(pChar);
		pChar = pCharNext;
	}
	charlist_flush();
	lpGameInfoHead = NULL;
	lpPendingChar  = NULL;
	currentgamenum = 0;
	LeaveCriticalSection(&csGameList);
	D2GSEventLog("D2GSResetGameList", "End all game in the Game List and in the GE");
	D2GSEndAllGames();
	return;

} /* End of D2GSResetGameList() */


/*********************************************************************
 * Purpose: to get current game number
 * Return: int
 *********************************************************************/
int D2GSGetCurrentGameNumber(void)
{
	return currentgamenum;

} /* End of D2GSGetCurrentGameNumber() */


/*********************************************************************
 * Purpose: to get current game number and user numbers in games
 * Return: int
 *********************************************************************/
int D2GSGetCurrentGameStatistic(DWORD *gamenum, DWORD *usernum)
{
	D2GAMEINFO		*lpGame;
	int				gamecount, charcount;

	gamecount = charcount = 0;
	EnterCriticalSection(&csGameList);
	lpGame = lpGameInfoHead;
	while(lpGame)
	{
		gamecount++;
		charcount += lpGame->CharCount;
		lpGame = lpGame->next;
		if (gamecount>500) break;
	}
	LeaveCriticalSection(&csGameList);

	*gamenum = gamecount;
	*usernum = charcount;
	return 0;

} /* End of D2GSGetCurrentGameStatistic() */


/*********************************************************************
 * Purpose: to kick all char out of a game
 * Return: None
 *********************************************************************/
void D2GSDeleteAllCharInGame(D2GAMEINFO *lpGameInfo)
{
	D2CHARINFO		*ph, *pnext;
	WORD			count;

	if (!lpGameInfo) return;
	ph = lpGameInfo->lpCharInfo;
	count = 0;
	while(ph)
	{
		pnext = ph->next;
		if (ph->CharLockStatus) {
			D2GSEventLog("D2GSDeleteAllCharInGame",
					"Unlock char %s(*%s) with unfinished loading status",
					ph->CharName, ph->AcctName);
			D2GSUnlockChar(ph->AcctName, ph->CharName);
		}
		D2GSEventLog("D2GSDeleteAllCharInGame",
				"Delete zombie char %s(*%s)", ph->CharName, ph->AcctName);
		charlist_delete(ph->CharName);
		free(ph);
		count++;
		ph = pnext;
	}
	lpGameInfo->lpCharInfo = NULL;
	D2GSEventLog("D2GSDeleteAllCharInGame",
		"Delete %u(%u) character in game '%s' (%u)",
		count, lpGameInfo->CharCount, lpGameInfo->GameName, lpGameInfo->GameId);
	lpGameInfo->CharCount = 0;
	return;

} /* End of D2GSDeleteAllCharInGame() */


/*********************************************************************
 * Purpose: to insert a new game info the game info list
 * Return: 0(success), other(failed)
 *********************************************************************/
int D2GSGameListInsert(UCHAR *GameName, UCHAR* GamePass, UCHAR* GameDesc, 
						UCHAR *createAcctName, UCHAR* createCharName, UCHAR* createIpAddr, UCHAR expansion,
						UCHAR ladder, UCHAR difficulty, UCHAR hardcore, DWORD dwGameId, int ge)
{
	D2GAMEINFO		*lpGameInfo;
	D2GAMEINFO		*lpTemp;

	if (!GameName) return D2GSERROR_BAD_PARAMETER;

	/* alloc memory for the structure */
	lpGameInfo = (D2GAMEINFO *)malloc(sizeof(D2GAMEINFO));
	if (!lpGameInfo) return D2GSERROR_NOT_ENOUGH_MEMORY;

	/* fill the fields */
	ZeroMemory(lpGameInfo, sizeof(D2GAMEINFO));
	strncpy(lpGameInfo->GameName, GameName, MAX_GAMENAME_LEN);
	strncpy(lpGameInfo->GamePass, GamePass, MAX_GAMEPASS_LEN);
	strncpy(lpGameInfo->GameDesc, GameDesc, MAX_GAMEDESC_LEN);
	strncpy(lpGameInfo->createAcctName, createAcctName, MAX_ACCTNAME_LEN);
	strncpy(lpGameInfo->createCharName, createCharName, MAX_CHARNAME_LEN);
	strncpy(lpGameInfo->createIpAddr, createIpAddr, MAX_IPADDR_LEN);
	lpGameInfo->expansion  = expansion;
	lpGameInfo->ladder     = ladder;
	lpGameInfo->difficulty = difficulty;
	lpGameInfo->hardcore   = hardcore;
	lpGameInfo->GameId     = dwGameId;
	lpGameInfo->CharCount  = 0;
	lpGameInfo->CreateTime = time(NULL);
	lpGameInfo->disable    = FALSE;
	lpGameInfo->ge         = ge;

	EnterCriticalSection(&csGameList);

	/* add to game list */
	lpTemp = lpGameInfoHead;
	lpGameInfoHead = lpGameInfo;
	if (lpTemp) {
		lpGameInfo->next = lpTemp;
		lpTemp->prev = lpGameInfo;
	}
	D2GSIncCurrentGameNumber();

	LeaveCriticalSection(&csGameList);

	D2GSEventLog("D2GSGameListInsert",
		"Insert into game list '%s' (%u)", GameName, dwGameId);
	return 0;

} /* End of D2GSGameListInsert() */


/*********************************************************************
 * Purpose: to delete a game from the game info list
 * Return: 0(success), other(failed)
 *********************************************************************/
int D2GSGameListDelete(D2GAMEINFO *lpGameInfo)
{
	D2GAMEINFO		*lpPrev, *lpNext;

	if (!lpGameInfo) return 0;	/* nothing to be deleted */

	EnterCriticalSection(&csGameList);
	D2GSDeleteAllCharInGame(lpGameInfo);
	lpPrev = lpGameInfo->prev;
	lpNext = lpGameInfo->next;
	if (lpPrev) lpPrev->next = lpNext;
	else lpGameInfoHead = lpNext;
	if (lpNext) lpNext->prev = lpPrev;
	lpGameInfo->GameId = 0;
	free(lpGameInfo);
	D2GSDecCurrentGameNumber();
	LeaveCriticalSection(&csGameList);

	return 0;

} /* End of D2GSGameListDelete() */


/*********************************************************************
 * Purpose: to insert a char into the game
 * Return: 0(success), other(failed)
 *********************************************************************/
int D2GSInsertCharIntoGameInfo(D2GAMEINFO *lpGameInfo, DWORD token, UCHAR *AcctName,
			UCHAR *CharName, UCHAR *IpAddr, DWORD CharLevel, WORD CharClass, WORD EnterGame, unsigned int vip_expire)
{
	D2CHARINFO		*lpCharInfo;
	D2CHARINFO		*lpTemp;
	int				val;

	if (!lpGameInfo) return D2GSERROR_BAD_PARAMETER;
	if (!AcctName || !CharName || !IpAddr) return D2GSERROR_BAD_PARAMETER;
	if (lpGameInfo->CharCount >= MAX_CHAR_IN_GAME) return D2GSERROR_GAME_IS_FULL;

	/* alloc memory */
	lpCharInfo = (D2CHARINFO *)malloc(sizeof(D2CHARINFO));
	if (!lpCharInfo) return D2GSERROR_NOT_ENOUGH_MEMORY;

	/* fill the fields */
	ZeroMemory(lpCharInfo, sizeof(D2CHARINFO));
	strncpy(lpCharInfo->AcctName, AcctName, MAX_ACCTNAME_LEN);
	strncpy(lpCharInfo->CharName, CharName, MAX_CHARNAME_LEN);
	strncpy(lpCharInfo->IpAddr, IpAddr, MAX_IPADDR_LEN);
	lpCharInfo->token          = token;
	lpCharInfo->CharLevel      = CharLevel;
	lpCharInfo->CharClass      = CharClass;
	lpCharInfo->EnterGame      = EnterGame;
	lpCharInfo->AllowLadder    = FALSE;
	lpCharInfo->CharLockStatus = FALSE;
	lpCharInfo->EnterTime      = time(NULL);
	lpCharInfo->CharCreateTime = lpCharInfo->EnterTime;
	lpCharInfo->GameId         = lpGameInfo->GameId;
	lpCharInfo->lpGameInfo     = lpGameInfo;
	lpCharInfo->ge             = lpGameInfo->ge;
	lpCharInfo->ClientId       = 0;
	lpCharInfo->vip_expire     = vip_expire;


	EnterCriticalSection(&csGameList);

	/* insert char into char list table */
	if ((val=charlist_insert(CharName, lpCharInfo, lpGameInfo))!=0) {
		D2GSEventLog("D2GSInsertCharIntoGameInfo",
				"failed insert info charlist for %s(*%s), code: %d", CharName, AcctName, val);
		free(lpCharInfo);
		LeaveCriticalSection(&csGameList);
		return D2GSERROR_CHAR_ALREADY_IN_GAME;
	}

	/* add to game info */
	lpTemp = lpGameInfo->lpCharInfo;
	lpGameInfo->lpCharInfo = lpCharInfo;
	if (lpTemp) {
		lpCharInfo->next = lpTemp;
		lpTemp->prev = lpCharInfo;
	}
	(lpGameInfo->CharCount)++;

	LeaveCriticalSection(&csGameList);

	return 0;

} /* End of D2GSInsertCharIntoGameInfo() */


/*********************************************************************
 * Purpose: to delete a char from the game
 * Return: 0(success), other(failed)
 *********************************************************************/
int D2GSDeleteCharFromGameInfo(D2GAMEINFO *lpGameInfo, D2CHARINFO *lpCharInfo, int enforce)
{
	D2CHARINFO		*lpPrev, *lpNext;

	if (!lpGameInfo || !lpCharInfo) return D2GSERROR_BAD_PARAMETER;
	if (lpCharInfo->lpGameInfo != lpGameInfo) return D2GSERROR_BAD_PARAMETER;

	EnterCriticalSection(&csGameList);
	lpPrev = lpCharInfo->prev;
	lpNext = lpCharInfo->next;
	if (lpPrev) lpPrev->next = lpNext;
	else lpGameInfo->lpCharInfo = lpNext;
	if (lpNext) lpNext->prev = lpPrev;
	(lpGameInfo->CharCount)--;

	charlist_delete(lpCharInfo->CharName);
	free(lpCharInfo);

	LeaveCriticalSection(&csGameList);

	return 0;

} /* End of D2GSDeleteCharFromGameInfo() */


/*********************************************************************
 * Purpose: to insert a char into the pending char list
 *          (receive join game request, but not EnterGame event callback)
 * Return: 0(success), other(failed)
 *********************************************************************/
int D2GSInsertCharIntoPendingList(DWORD token, UCHAR *AcctName,
					UCHAR *CharName, UCHAR *IpAddr, DWORD CharLevel, WORD CharClass, D2GAMEINFO *lpGame, unsigned int vip_expire)
{
	D2CHARINFO		*lpCharInfo;
	D2CHARINFO		*lpTemp;

	if (!AcctName || !CharName || !lpGame || !IpAddr) return D2GSERROR_BAD_PARAMETER;

	/* alloc memory */
	lpCharInfo = (D2CHARINFO *)malloc(sizeof(D2CHARINFO));
	if (!lpCharInfo) return D2GSERROR_NOT_ENOUGH_MEMORY;

	/* fill the fields */
	ZeroMemory(lpCharInfo, sizeof(D2CHARINFO));
	strncpy(lpCharInfo->AcctName, AcctName, MAX_ACCTNAME_LEN);
	strncpy(lpCharInfo->CharName, CharName, MAX_CHARNAME_LEN);
	strncpy(lpCharInfo->IpAddr, IpAddr, MAX_IPADDR_LEN);
	lpCharInfo->token      = token;
	lpCharInfo->CharLevel  = CharLevel;
	lpCharInfo->CharClass  = CharClass;
	lpCharInfo->EnterGame  = FALSE;
	lpCharInfo->EnterTime  = 0;
	lpCharInfo->GameId     = lpGame->GameId;
	lpCharInfo->ge         = lpGame->ge;
	lpCharInfo->lpGameInfo = lpGame;
	lpCharInfo->ClientId   = 0;
	lpCharInfo->vip_expire = vip_expire;

	EnterCriticalSection(&csGameList);

	/* add to pending char list */
	lpTemp = lpPendingChar;
	lpPendingChar = lpCharInfo;
	if (lpTemp) {
		lpCharInfo->next = lpTemp;
		lpTemp->prev = lpCharInfo;
	}
	(lpGame->CharCount)++;

	LeaveCriticalSection(&csGameList);

	return 0;

} /* End of D2GSInsertCharIntoPendingList() */


/*********************************************************************
 * Purpose: to delete a char from the pending char list
 * Return: 0(success), other(failed)
 *********************************************************************/
int D2GSDeletePendingChar(D2CHARINFO *lpCharInfo)
{
	D2CHARINFO		*lpPrev, *lpNext;

	if (!lpCharInfo) return 0;	/* nothing to be deleted */

	EnterCriticalSection(&csGameList);
	lpPrev = lpCharInfo->prev;
	lpNext = lpCharInfo->next;
	if (lpPrev)	lpPrev->next = lpNext;
	else lpPendingChar = lpNext;
	if (lpNext) lpNext->prev = lpPrev;
	if ((!IsBadReadPtr(lpCharInfo->lpGameInfo, sizeof(D2GAMEINFO)))
					&& (lpCharInfo->lpGameInfo->GameId == lpCharInfo->GameId)) {
		(lpCharInfo->lpGameInfo->CharCount)--;
	} else {
		D2GSEventLog("D2GSDeletePendingChar",
			"Delete a pending char %s(*%s) in an already closed game",
			lpCharInfo->CharName, lpCharInfo->AcctName);
	}
	free(lpCharInfo);
	LeaveCriticalSection(&csGameList);

	return 0;

} /* End of D2GSGameListDelete() */


/*********************************************************************
 * Purpose: to insert get data request to the list
 * Return: 0(success), other(failed)
 *********************************************************************/
int D2GSInsertGetDataRequest(UCHAR *AcctName, UCHAR *CharName, DWORD dwClientId, DWORD dwSeqno)
{
	D2GETDATAREQUEST	*lpGetDataReq;
	D2GETDATAREQUEST	*lpTemp;

	if (!AcctName || !CharName) return D2GSERROR_BAD_PARAMETER;

	/* alloc memory */
	lpGetDataReq = (D2GETDATAREQUEST*)malloc(sizeof(D2GETDATAREQUEST));
	if (!lpGetDataReq) return D2GSERROR_NOT_ENOUGH_MEMORY;

	/* fill the fields */
	ZeroMemory(lpGetDataReq, sizeof(D2GETDATAREQUEST));
	strncpy(lpGetDataReq->AcctName, AcctName, MAX_ACCTNAME_LEN);
	strncpy(lpGetDataReq->CharName, CharName, MAX_CHARNAME_LEN);
	lpGetDataReq->ClientId  = dwClientId;
	lpGetDataReq->Seqno     = dwSeqno;
	lpGetDataReq->TickCount = 0;

	EnterCriticalSection(&csGameList);

	/* add to request list */
	lpTemp = lpGetDataReqHead;
	lpGetDataReqHead = lpGetDataReq;
	if (lpTemp) {
		lpGetDataReq->next = lpTemp;
		lpTemp->prev = lpGetDataReq;
	}

	LeaveCriticalSection(&csGameList);

	return 0;

} /* End of D2GSInsertGetDataRequest() */


/*********************************************************************
 * Purpose: to delete a get data request from the list
 * Return: 0(success), other(failed)
 *********************************************************************/
int D2GSDeleteGetDataRequest(D2GETDATAREQUEST *lpGetDataReq)
{
	D2GETDATAREQUEST	*lpPrev, *lpNext;

	if (!lpGetDataReq) return 0;	/* nothing to be deleted */

	EnterCriticalSection(&csGameList);
	lpPrev = lpGetDataReq->prev;
	lpNext = lpGetDataReq->next;
	if (lpPrev)	lpPrev->next = lpNext;
	else lpGetDataReqHead = lpNext;
	if (lpNext) lpNext->prev = lpPrev;
	free(lpGetDataReq);
	LeaveCriticalSection(&csGameList);

	return 0;

} /* End of D2GSDeleteGetDataRequest() */


/*********************************************************************
 * Purpose: to find a game info by game id
 * Return: NULL(not found or failed), other(success)
 *********************************************************************/
D2GAMEINFO *D2GSFindGameInfoByGameId(DWORD GameId)
{
	D2GAMEINFO	*lpGame;

	EnterCriticalSection(&csGameList);
	lpGame = lpGameInfoHead;
	while(lpGame)
	{
		if (lpGame->GameId == GameId) break;
		lpGame = lpGame->next;
	}
	LeaveCriticalSection(&csGameList);
	return lpGame;

} /* End of D2GSFindGameInfoByGameId() */


/*********************************************************************
 * Purpose: to find a game info by game name
 * Return: NULL(not found or failed), other(success)
 *********************************************************************/
D2GAMEINFO *D2GSFindGameInfoByGameName(UCHAR *GameName)
{
	D2GAMEINFO	*lpGame;

	if (!GameName) return NULL;

	EnterCriticalSection(&csGameList);
	lpGame = lpGameInfoHead;
	while(lpGame)
	{
		if (!strcmp(lpGame->GameName, GameName)) break;
		lpGame = lpGame->next;
	}
	LeaveCriticalSection(&csGameList);
	return lpGame;

} /* End of D2GSFindGameInfoByGameName() */


/*********************************************************************
 * Purpose: to find a char by char name in pending char list
 * Return: NULL(not found or failed), other(success)
 *********************************************************************/
D2CHARINFO *D2GSFindCharInGameByCharName(D2GAMEINFO *lpGame, UCHAR *CharName)
{
	D2CHARINFO		*lpChar;

	if (!CharName || !lpGame) return NULL;

	EnterCriticalSection(&csGameList);
	lpChar = lpGame->lpCharInfo;
	while(lpChar)
	{
		if (!strcmp(lpChar->CharName, CharName)) break;
		lpChar = lpChar->next;
	}
	LeaveCriticalSection(&csGameList);
	return lpChar;

} /* End of D2GSFindCharInGameByCharName() */


/*********************************************************************
 * Purpose: to find a char by its token in pending char list
 * Return: NULL(not found or failed), other(success)
 *********************************************************************/
D2CHARINFO *D2GSFindPendingCharByToken(DWORD token)
{
	D2CHARINFO		*lpChar;

	EnterCriticalSection(&csGameList);
	lpChar = lpPendingChar;
	while(lpChar)
	{
		if (lpChar->token == token) break;
		lpChar = lpChar->next;
	}
	LeaveCriticalSection(&csGameList);
	return lpChar;

} /* End of D2GSFindPendingCharByToken() */


/*********************************************************************
 * Purpose: to find a char by char name in pending char list
 * Return: NULL(not found or failed), other(success)
 *********************************************************************/
D2CHARINFO *D2GSFindPendingCharByCharName(UCHAR *CharName)
{
	D2CHARINFO		*lpChar;

	if (!CharName) return NULL;

	EnterCriticalSection(&csGameList);
	lpChar = lpPendingChar;
	while(lpChar)
	{
		if (!strcmp(lpChar->CharName, CharName)) break;
		lpChar = lpChar->next;
	}
	LeaveCriticalSection(&csGameList);
	return lpChar;

} /* End of D2GSFindPendingCharByCharName() */


/*********************************************************************
 * Purpose: to find a get data request from the list
 * Return: NULL(not found or failed), other(success)
 *********************************************************************/
D2GETDATAREQUEST *D2GSFindGetDataRequestBySeqno(DWORD dwSeqno)
{
	D2GETDATAREQUEST	*lpGetDataReq;

	EnterCriticalSection(&csGameList);
	lpGetDataReq = lpGetDataReqHead;
	while(lpGetDataReq)
	{
		if (lpGetDataReq->Seqno==dwSeqno) break;
		lpGetDataReq = lpGetDataReq->next;
	}
	LeaveCriticalSection(&csGameList);
	return lpGetDataReq;

} /* End of D2GSFindGetDataRequestBySeqno() */

D2GETDATAREQUEST *D2GSFindGetDataRequestByCharName(char* CharName)
{
	D2GETDATAREQUEST	*lpGetDataReq;
	D2GETDATAREQUEST	*found = NULL;
	EnterCriticalSection(&csGameList);
	lpGetDataReq = lpGetDataReqHead;
	while(lpGetDataReq)
	{
		if (strcmpi(lpGetDataReq->CharName, CharName) == 0) 
		{
			found = lpGetDataReq;
			break;
		}
		lpGetDataReq = lpGetDataReq->next;
	}
	LeaveCriticalSection(&csGameList);
	return found;

}

/*********************************************************************
 * Purpose: to do sth in the timer for pending char list
 * Return: None
 *********************************************************************/
void D2GSPendingCharTimerRoutine(void)
{
	D2CHARINFO		*lpChar, *lpTimeoutChar;

	EnterCriticalSection(&csGameList);
	lpChar = lpPendingChar;
	while(lpChar)
	{
		lpChar->TickCount++;
		if ((lpChar->TickCount) > (d2gsconf.charpendingtimeout))
			lpTimeoutChar = lpChar;
		else
			lpTimeoutChar = NULL;
		lpChar = lpChar->next;
		if (lpTimeoutChar)
			D2GSDeletePendingChar(lpTimeoutChar);
	}
	LeaveCriticalSection(&csGameList);

	return;

} /* End of D2GSPendingCharTimerRoutine() */


/*********************************************************************
 * Purpose: to do sth in the timer for get data request list
 * Return: None
 *********************************************************************/
void D2GSGetDataRequestTimerRoutine(void)
{
	D2GETDATAREQUEST	*lpGetDataReq, *lpTimeOutReq, *lpTimeOutReqs[512];
	DWORD				dwClientId;
	u_char				AcctName[MAX_ACCTNAME_LEN+1];
	u_char				CharName[MAX_CHARNAME_LEN+1];
	D2GAMEINFO			*lpGameInfo;
	D2CHARINFO			*lpCharInfo;
	int					reqNum = 0;
	int					i;

	EnterCriticalSection(&csGameList);
	lpGetDataReq = lpGetDataReqHead;
	while(lpGetDataReq)
	{
		lpGetDataReq->TickCount++;
		if ((lpGetDataReq->TickCount) > GET_DATA_TIMEOUT)
			lpTimeOutReqs[reqNum++] = lpGetDataReq;
		lpGetDataReq = lpGetDataReq->next;
	}
	LeaveCriticalSection(&csGameList);
	for(i = 0; i < reqNum; i ++)
	{
		lpTimeOutReq = lpTimeOutReqs[i];
		dwClientId = lpTimeOutReq->ClientId;
		D2GSSendDatabaseCharacter(dwClientId, NULL, 0, 0, TRUE, 0, NULL, 1);
		D2GSEventLog("D2GSGetDataRequestTimerRoutine",
			"Failed get CHARSAVE data for '%s'(*%s)",
			lpTimeOutReq->CharName, lpTimeOutReq->AcctName);
		/* check if this char is still in the list? if so, delete it */
		strncpy(AcctName, lpTimeOutReq->AcctName, MAX_ACCTNAME_LEN);
		AcctName[MAX_ACCTNAME_LEN] = '\0';
		strncpy(CharName, lpTimeOutReq->CharName, MAX_CHARNAME_LEN);
		CharName[MAX_CHARNAME_LEN] = '\0';
		lpGameInfo = (D2GAMEINFO*)charlist_getdata(CharName, CHARLIST_GET_GAMEINFO);
		lpCharInfo = (D2CHARINFO*)charlist_getdata(CharName, CHARLIST_GET_CHARINFO);
		if (lpCharInfo && lpGameInfo &&
			!IsBadReadPtr(lpCharInfo, sizeof(D2CHARINFO)) &&
			!IsBadReadPtr(lpGameInfo, sizeof(D2GAMEINFO)) &&
			(lpCharInfo->lpGameInfo == lpGameInfo) &&
			(lpCharInfo->GameId == lpGameInfo->GameId) &&
			(lpCharInfo->ClientId == dwClientId)) {
				D2GSDeleteCharFromGameInfo(lpGameInfo, lpCharInfo, TRUE);
				D2GSEventLog("D2GSGetDataRequestTimerRoutine",
					"delete char %s(*%s) still in game '%s'(%u)",
					CharName, AcctName, lpGameInfo->GameName, lpGameInfo->GameId);
		} else {
			D2GSEventLog("D2GSGetDataRequestTimerRoutine",
				"char %s(*%s) NOT in game now",	CharName, AcctName);
		}
		/* delete the overdue quest */
		D2GSDeleteGetDataRequest(lpTimeOutReq);
	}
	return;

} /* End of D2GSGetDataRequestTimerRoutine() */


/*-------------------------------------------------------------------*/

void FormatTimeString(long t, u_char *buf, int len)
{
	struct tm		*tm;
	long			now;

	ZeroMemory(buf, len);
	now = t;
	tm = localtime(&now);
	_snprintf(buf, len-1, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
	return;

} /* End of FormatTimeString() */

void D2GSSysAnn(char* val){
	D2GAMEINFO		*lpGame;
	D2CHARINFO		*lpChar;
	EnterCriticalSection(&csGameList);
	lpGame = lpGameInfoHead;
	while(lpGame){
		lpChar = lpGame->lpCharInfo;
		while(lpChar)
		{
			chat_message_announce_char3(CHAT_MESSAGE_TYPE_SYS_MESSAGE, lpChar->ClientId, val, D2COLOR_ID_GREEN);
			lpChar = lpChar->next;
		}
		lpGame = lpGame->next;
	}
	LeaveCriticalSection(&csGameList);
}

/*********************************************************************
 * Purpose: to announce to all
 * Return: 0 or -1
 *********************************************************************/
int chat_message_announce_all(DWORD dwMsgType, const char *msg)
{
	D2GAMEINFO		*lpGame;
	int				gamecount;

	EnterCriticalSection(&csGameList);
	gamecount = 0;
	lpGame = lpGameInfoHead;
	while(lpGame)
	{
		gamecount++;
		chat_message_announce_game2(dwMsgType, lpGame, msg);
		lpGame = lpGame->next;
		if (gamecount>500) break;
	}
	LeaveCriticalSection(&csGameList);
	return 0;
} /* End of chat_message_announce_all() */


/*********************************************************************
 * Purpose: to announce to a game
 * Return: 0 or -1
 *********************************************************************/
int chat_message_announce_game(DWORD dwMsgType, DWORD GameId, const char *msg)
{
	D2GAMEINFO	*lpGame;
	
	EnterCriticalSection(&csGameList);
	lpGame = D2GSFindGameInfoByGameId(GameId);
	if (lpGame==NULL) {
		LeaveCriticalSection(&csGameList);
		return -1;
	}
	chat_message_announce_game2(dwMsgType, lpGame, msg);
	LeaveCriticalSection(&csGameList);
	return 0;
} /* End of chat_message_announce_game() */


/*********************************************************************
 * Purpose: to announce to a game
 * Return: 0 or -1
 *********************************************************************/
int chat_message_announce_game2(DWORD dwMsgType, D2GAMEINFO *lpGame, const char *msg)
{
	D2CHARINFO		*lpChar;
	int				charcount;

	EnterCriticalSection(&csGameList);
	charcount = 0;
	lpChar = lpGame->lpCharInfo;
	while(lpChar)
	{
		charcount++;
		D2GSSendClientChatMessage(lpChar->ClientId, dwMsgType,
				D2COLOR_ID_RED, "[administrator]", msg);
		lpChar = lpChar->next;
		if (charcount>8) break;
	}
	LeaveCriticalSection(&csGameList);
	return 0;
} /* End of chat_message_announce_game2() */


/*********************************************************************
 * Purpose: to announce to a char
 * Return: 0 or -1
 *********************************************************************/
int chat_message_announce_char(DWORD dwMsgType, const char *CharName, const char *msg)
{
	D2CHARINFO		*lpChar;

	EnterCriticalSection(&csGameList);
	lpChar = charlist_getdata(CharName, CHARLIST_GET_CHARINFO);
	if (!lpChar) {
		LeaveCriticalSection(&csGameList);
		return -1;
	}
	D2GSSendClientChatMessage(lpChar->ClientId, dwMsgType,
			D2COLOR_ID_RED, "[administrator]", msg);
	LeaveCriticalSection(&csGameList);
	return 0;
} /* End of chat_message_announce_char() */


/*********************************************************************
 * Purpose: to announce to a char
 * Return: 0 or -1
 *********************************************************************/
int chat_message_announce_char2(DWORD dwMsgType, DWORD dwClientId, const char *msg)
{
	D2GSSendClientChatMessage(dwClientId, dwMsgType, D2COLOR_ID_RED, "[administrator]", msg);
	return 0;
} /* End of chat_message_announce_char2() */

int chat_message_announce_char3(DWORD dwMsgType, DWORD dwClientId, const char *msg, DWORD color)
{
	D2GSSendClientChatMessage(dwClientId, dwMsgType, color, "[administrator]", msg);
	return 0;
} /* End of chat_message_announce_char2() */


/*********************************************************************
 * Purpose: to add a client to the MOTD list
 * Return: 0 or -1
 *********************************************************************/
int D2GSMOTDAdd(DWORD dwClientId, unsigned int vip_expire)
{
	MOTDCLIENT	*pmotd;

	pmotd = (MOTDCLIENT*)malloc(sizeof(MOTDCLIENT));
	if (pmotd==NULL) return -1;
	pmotd->ClientId = dwClientId;
	pmotd->vip_expire = vip_expire;
	EnterCriticalSection(&csGameList);
	list_add_tail((struct list_head*)pmotd, &list_motd);
	LeaveCriticalSection(&csGameList);
	return 0;
} /* End of D2GSMOTDAdd() */


/*********************************************************************
 * Purpose: to add a client to the MOTD list
 * Return: 0 or -1
 *********************************************************************/
int D2GSSendMOTD(void)
{
	static int			count = 0;

	struct list_head	*p, *ptemp;
	MOTDCLIENT			*pmotd;
	char				event_str[256];

	/* we do this one time per second */
	count++;
	if (count<(SEND_MOTD_INTERVAL/TIMER_TICK_IN_MS)) return 0;
	count = 0;

	if(d2gsconf.eventmotd[0] != 0){
		strcpy(event_str, d2gsconf.eventmotd);
		string_color(event_str);
	}

	EnterCriticalSection(&csGameList);
	list_for_each_safe(p, ptemp, &list_motd)
	{
		struct GEINFO* geinfo;
		char motdbuf[256];
		pmotd = list_entry(p, MOTDCLIENT, list);
		geinfo = GetClientIdOnGE(pmotd->ClientId);
		chat_message_announce_char3(CHAT_MESSAGE_TYPE_SYS_MESSAGE, pmotd->ClientId, d2gsconf.versionMotd, D2COLOR_ID_DARK_YELLOW);
		chat_message_announce_char3(CHAT_MESSAGE_TYPE_SYS_MESSAGE, pmotd->ClientId, d2gsconf.roomMotd, D2COLOR_ID_DARK_YELLOW);
		if(d2gsconf.eventmotd[0] != 0){
			chat_message_announce_char2(CHAT_MESSAGE_TYPE_SYS_MESSAGE, pmotd->ClientId, event_str);
		}
		//chat_message_announce_char2(CHAT_MESSAGE_TYPE_SYS_MESSAGE, pmotd->ClientId, d2gsconf.motd);
		if(pmotd->vip_expire < time(NULL)){
			chat_message_announce_char3(CHAT_MESSAGE_TYPE_SYS_MESSAGE, pmotd->ClientId, "您尚未開通暗金帳號，開通暗金帳號可享有多項特權，詳情請登錄http://bbs.91d2.cn/查看。", D2COLOR_ID_GREEN);
		}else{
			char buf[128];
			time_t      now;
			struct tm * tmnow;
			now = pmotd->vip_expire;
			if ((!(tmnow = localtime(&now))))
				strcpy(buf,"?");
			else
				strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",tmnow);
			sprintf(motdbuf, "Your unique account expires on %s, thanks for appreciate.", buf, pmotd->vip_expire);
			chat_message_announce_char3(CHAT_MESSAGE_TYPE_SYS_MESSAGE, pmotd->ClientId, motdbuf, D2COLOR_ID_GREEN);
		}
		//sprintf(motdbuf, "91D2-GSTrunk: Currently on GE %d", geinfo->id);
		//chat_message_announce_char3(CHAT_MESSAGE_TYPE_SYS_MESSAGE, pmotd->ClientId, motdbuf, D2COLOR_ID_RED);
		list_del(p);
		free(pmotd);
	}
	LeaveCriticalSection(&csGameList);
	return 0;

} /* End of D2GSSendMOTD() */

int D2GSCheckGameLife(void)
{
	D2GAMEINFO		*lpGame;
	D2CHARINFO		*ph, *pnext;

	EnterCriticalSection(&csGameList);
	lpGame = lpGameInfoHead;
	while(lpGame)
	{
		DWORD now = time(NULL);
		DWORD remaining = d2gsconf.maxgamelife - (now - lpGame->CreateTime);
		if (remaining <= 0)
		{
			ph = lpGame->lpCharInfo;
			while(ph)
			{
				D2GSRemoveClientFromGame(ph->ClientId);
				ph = ph->next;
			}
		}
		else if (remaining == 3)
		{
			chat_message_announce_game2(CHAT_MESSAGE_TYPE_SYS_MESSAGE, lpGame, "TimeLimit: The game is shutting down.");
		}
		else if (remaining <= 60 && remaining % 5 == 0)
		{
			char buf[128];
			sprintf(buf, "TimeLimit: The game will end in %d second(s).", remaining);
			chat_message_announce_game2(CHAT_MESSAGE_TYPE_SYS_MESSAGE, lpGame, buf);
		}
		else if (remaining % 3600 == 0 && remaining < d2gsconf.maxgamelife)
		{
			char buf[128];
			sprintf(buf, "TimeLimit: %d hour(s) remaining for this game.", remaining / 3600);
			chat_message_announce_game2(CHAT_MESSAGE_TYPE_SYS_MESSAGE, lpGame, buf);
		}
		lpGame = lpGame->next;
	}
	LeaveCriticalSection(&csGameList);
	return 0;

} /* End of D2GSSendMOTD() */