#ifndef INCLUDED_D2GS_D2GE_PROTOCOL_H
#define INCLUDED_D2GS_D2GE_PROTOCOL_H

#pragma pack(push, pack01, 1)

typedef struct
{
	unsigned short type;
	unsigned int   seqno;
} t_d2gs_d2ge_header;

#define D2GE_D2GS_MAX_GAME 0x70
typedef struct
{
	t_d2gs_d2ge_header	h;
	unsigned short d2ge_id;
	unsigned int max_game;
	unsigned short port;
} t_d2ge_d2gs_max_game;

#define D2GE_D2GS_GREET 0x71
typedef struct
{
	t_d2gs_d2ge_header	h;
	unsigned short d2ge_id;
} t_d2ge_d2gs_greet;

#define D2GS_D2GE_GREET_REPLY 0x71
typedef struct
{
	t_d2gs_d2ge_header	h;
	D2GSCONFIGS			d2gsconfig;
	u_char				acstring[255];
	u_char				configfile[255];
} t_d2gs_d2ge_greet_reply;

#define D2GS_D2GE_NEWEMPTYGAME 0x72
typedef struct
{
	t_d2gs_d2ge_header	h;
	u_char				gamename[33];
	u_char				gamepass[33];
	u_char				gamedesc[33];
	DWORD				dwGameFlag;
	BYTE				bTemplate;
	BYTE				bReserved1;
	BYTE				bReserved2;
	DWORD				GameId;
} t_d2gs_d2ge_newemptygame;

#define D2GE_D2GS_NEWEMPTYGAME_RETURN 0x72
typedef struct
{
	t_d2gs_d2ge_header	h;
	BOOL				result;
	DWORD				GameId;
} t_d2ge_d2gs_newemptygame_return;

#define D2GS_D2GE_SENDCHAR 0x73
typedef struct
{
	t_d2gs_d2ge_header	h;
	DWORD				dwClientId;
	DWORD				dwSize;
	DWORD				dwTotalSize;
	BOOL				bLock;
	DWORD				dwReserved1;
	PLAYERINFO			lpPlayerInfo;
	DWORD				dwReserved2;
	DWORD				SaveDataLen;
} t_d2gs_d2ge_sendchar;

#define D2GE_D2GS_SENDCHAR_RETURN 0x73
typedef struct
{
	t_d2gs_d2ge_header	h;
	BOOL				result;
} t_d2ge_d2gs_sendchar_return;

#define D2GS_D2GE_REMOVECLIENT 0x74
typedef struct
{
	t_d2gs_d2ge_header	h;
	DWORD				dwClientId;
} t_d2gs_d2ge_removeclient;

#define D2GS_D2GE_ENDALLGAMES 0x75
typedef struct
{
	t_d2gs_d2ge_header	h;
} t_d2gs_d2ge_endallgames;


#define D2GS_D2GE_CHATMSG 0x76
typedef struct
{
	t_d2gs_d2ge_header	h;
	DWORD				dwClientId;
	DWORD				dwType;
	DWORD				dwColor;
	u_char				name[33];
	DWORD				TextLen;
} t_d2gs_d2ge_chatmsg;

#define D2GE_D2GS_CHATMSG_RETURN 0x76
typedef struct
{
	t_d2gs_d2ge_header	h;
	DWORD				result;
} t_d2ge_d2gs_chatmsg_return;

#define D2GE_D2GS_CLOSEGAME 0x80
typedef struct
{
	t_d2gs_d2ge_header	h;
	WORD				wGameId;
} t_d2ge_d2gs_closegame;

#define D2GE_D2GS_LEAVEGAME 0x81
typedef struct
{
	t_d2gs_d2ge_header	h;
	GAMEDATA			lpGameData;
	WORD				wGameId;
	WORD				wCharClass;
	DWORD				dwCharLevel;
	DWORD				dwExpLow;
	DWORD				dwExpHigh;
	WORD				wCharStatus;
	u_char				charname[17];
	BOOL				bUnlock;
	DWORD				dwZero1;
	DWORD				dwZero2;
	u_char				accountname[17];
	PLAYERDATA			PlayerData;
	PLAYERMARK			PlayerMark;
	int					PortraitLen;
} t_d2ge_d2gs_leavegame;

#define D2GE_D2GS_GETCHAR 0x82
typedef struct
{
	t_d2gs_d2ge_header	h;
	GAMEDATA			lpGameData;
	u_char				charname[17];
	DWORD				dwClientId;
	u_char				accountname[17];
	int					d2ge_id;
} t_d2ge_d2gs_getchar;

#define D2GE_D2GS_SAVECHAR 0x83
typedef struct
{
	t_d2gs_d2ge_header	h;
	GAMEDATA			lpGameData;
	u_char				charname[17];
	u_char				accountname[17];
	DWORD				dwSize;
	PLAYERDATA			PlayerData;
	int					SaveDataLen;
} t_d2ge_d2gs_savechar;

#define D2GE_D2GS_ENTERGAME 0x84
typedef struct
{
	t_d2gs_d2ge_header	h;
	WORD				wGameId;
	u_char				charname[17];
	WORD				wCharClass;
	DWORD				dwCharLevel;
	DWORD				dwReserved;
} t_d2ge_d2gs_entergame;

#define D2GE_D2GS_FINDTOKEN 0x85
typedef struct
{
	t_d2gs_d2ge_header	h;
	u_char				charname[17];
	DWORD				dwToken;
	WORD				wGameId;
} t_d2ge_d2gs_findtoken;

#define D2GS_D2GE_FINDTOKEN_CALLBACK 0x85
typedef struct
{
	t_d2gs_d2ge_header	h;
	BOOL				result;
	u_char				accountname[17];
	PLAYERDATA			lpPlayerData;
} t_d2gs_d2ge_findtoken_callback;


#define D2GE_D2GS_UPDATELADDER 0x86
typedef struct
{
	t_d2gs_d2ge_header	h;
	u_char				charname[17];
	WORD				wCharClass;
	DWORD				dwCharLevel;
	DWORD				dwCharExpLow;
	DWORD				dwCharExpHigh;
	WORD				wCharStatus;
	PLAYERMARK			PlayerMark;
} t_d2ge_d2gs_updateladder;

#define D2GE_D2GS_UPDATEINFO 0x87
typedef struct
{
	t_d2gs_d2ge_header	h;
	WORD				wGameId;
	u_char				charname[17];
	WORD				wCharClass;
	DWORD				dwCharLevel;
} t_d2ge_d2gs_updateinfo;

#define D2GS_D2GE_INCOMING_CLIENT 0x88
typedef struct
{
	t_d2gs_d2ge_header	h;
	SOCKET				s;
} t_d2gs_d2ge_incoming_client;

#pragma pack(pop, pack01)

#endif
