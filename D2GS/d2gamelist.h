#ifndef INCLUDED_D2GAMELIST_H
#define INCLUDED_D2GAMELIST_H

/*
 * define structure to store game info, store character info in the game
 * and the functions to maintain the game and character list
 */

#include <windows.h>
#include "d2gs.h"
#include "bn_types.h"
#include "list.h"
#include "itemscan.h"

/* structures */
typedef struct RAW_D2CHARINFO {
	bn_char		AcctName[MAX_ACCTNAME_LEN+1];	/* account name */
	bn_char		CharName[MAX_CHARNAME_LEN+1];	/* char name */
	bn_char		IpAddr[MAX_IPADDR_LEN+1];	/* ip address */
	bn_int		token;
	bn_int		CharLevel;
	bn_short	CharClass;
	bn_short	TickCount;
	bn_short	EnterGame;
	bn_short	AllowLadder;
	bn_short	CharLockStatus;
	DWORD		EnterTime;
	DWORD		CharCreateTime;
	DWORD		ClientId;
	DWORD		GameId;
	int			ge;
	unsigned int vip_expire;
	struct t_scan_result    ItemRecord;
	struct RAW_D2GAMEINFO	*lpGameInfo;	/* pointer to the GAMEINFO */
	struct RAW_D2CHARINFO	*prev;
	struct RAW_D2CHARINFO	*next;
} D2CHARINFO, *PD2CHARINFO, *LPD2CHARINFO;

typedef struct RAW_D2GAMEINFO {
	bn_char		GameName[MAX_GAMENAME_LEN+1];
	bn_char		GamePass[MAX_GAMEPASS_LEN+1];
	bn_char		GameDesc[MAX_GAMEDESC_LEN+1];
	bn_char		createAcctName[MAX_ACCTNAME_LEN+1];
	bn_char		createCharName[MAX_CHARNAME_LEN+1];
	bn_char		createIpAddr[MAX_IPADDR_LEN+1];
	bn_byte		expansion;
	bn_byte		ladder;
	bn_byte		difficulty;
	bn_byte		hardcore;
	bn_byte		reserved;
	DWORD		GameId;
	WORD		CharCount;
	DWORD		CreateTime;
	DWORD		disable;
	int			ge;
	struct RAW_D2CHARINFO	*lpCharInfo;
	struct RAW_D2GAMEINFO	*prev;
	struct RAW_D2GAMEINFO	*next;
} D2GAMEINFO, *PD2GAMEINFO, *LPD2GAMEINFO;


typedef struct RAW_D2GETDATAREQUEST {
	bn_char		AcctName[MAX_ACCTNAME_LEN+1];	/* account name */
	bn_char		CharName[MAX_CHARNAME_LEN+1];	/* char name */
	DWORD		ClientId;
	DWORD		Seqno;
	bn_short	TickCount;
	struct RAW_D2GETDATAREQUEST *prev;
	struct RAW_D2GETDATAREQUEST *next;
} D2GETDATAREQUEST, *PD2GETDATAREQUEST, *LPD2GETDATAREQUEST;

typedef struct RAW_MOTDCLIENT {
	struct list_head	list;
	DWORD				ClientId;
	unsigned int		vip_expire;
} MOTDCLIENT, *PMOTDCLIENT, *LPMOTDCLIENT;

/* macro defination */
#define D2GSIncCurrentGameNumber()		{ currentgamenum++; }
#define D2GSDecCurrentGameNumber()		{ currentgamenum--; }

/* functions */
void D2GSResetGameList(void);
int  D2GSGetCurrentGameNumber(void);
int  D2GSGetCurrentGameStatistic(DWORD *gamenum, DWORD *usernum);
void D2GSDeleteAllCharInGame(D2GAMEINFO *lpGameInfo);
int  D2GSGameListInsert(UCHAR *GameName, UCHAR* GamePass, UCHAR* GameDesc, 
						UCHAR *createAcctName, UCHAR* createCharName, UCHAR* createIpAddr, UCHAR expansion,
						UCHAR ladder, UCHAR difficulty, UCHAR hardcore, DWORD dwGameId, int ge);
int  D2GSGameListDelete(D2GAMEINFO *lpGameInfo);
int  D2GSInsertCharIntoGameInfo(D2GAMEINFO *lpGameInfo, DWORD token, UCHAR *AcctName,
					UCHAR *CharName, UCHAR *IpAddr, DWORD CharLevel, WORD CharClass, WORD EnterGame, unsigned int vip_expire);
int  D2GSDeleteCharFromGameInfo(D2GAMEINFO *lpGameInfo, D2CHARINFO *lpCharInfo, int enforce);
int  D2GSInsertCharIntoPendingList(DWORD token, UCHAR *AcctName,
					UCHAR *CharName, UCHAR *IpAddr, DWORD CharLevel, WORD CharClass, D2GAMEINFO *lpGame, unsigned int vip_expire);
int  D2GSDeletePendingChar(D2CHARINFO *lpCharInfo);
int  D2GSInsertGetDataRequest(UCHAR *AcctName, UCHAR *CharName, DWORD dwClientId, DWORD dwSeqno);
int  D2GSDeleteGetDataRequest(D2GETDATAREQUEST *lpGetDataReq);
D2GAMEINFO *D2GSFindGameInfoByGameId(DWORD GameId);
D2GAMEINFO *D2GSFindGameInfoByGameName(UCHAR *GameName);
D2CHARINFO *D2GSFindCharInGameByCharName(D2GAMEINFO *lpGame, UCHAR *CharName);
D2CHARINFO *D2GSFindPendingCharByToken(DWORD token);
D2CHARINFO *D2GSFindPendingCharByCharName(UCHAR *CharName);
D2GETDATAREQUEST *D2GSFindGetDataRequestBySeqno(DWORD dwSeqno);
void D2GSPendingCharTimerRoutine(void);
void D2GSGetDataRequestTimerRoutine(void);
void FormatTimeString(long t, u_char *buf, int len);
void CleanupGEGame(int d2ge_id);
void CleanupGEChar(int d2ge_id);
int chat_message_announce_all(DWORD dwMsgType, const char *msg);
int chat_message_announce_game(DWORD dwMsgType, DWORD GameId, const char *msg);
int chat_message_announce_game2(DWORD dwMsgType, D2GAMEINFO *lpGame, const char *msg);
int chat_message_announce_char(DWORD dwMsgType, const char *CharName, const char *msg);
int chat_message_announce_char2(DWORD dwMsgType, DWORD dwClientId, const char *msg);

int D2GSMOTDAdd(DWORD dwClientId, unsigned int vip_expire);
int D2GSSendMOTD(void);
void D2GSSysAnn(char* val);

#endif /* INCLUDED_D2GAMELIST_H */