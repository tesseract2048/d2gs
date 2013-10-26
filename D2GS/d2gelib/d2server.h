/*  
	Diablo2 Game Server Library
  	Copyright (C) 2000, 2001  Onlyer(onlyer@263.net)

	This library is based on original Diablo2 game library,
	Diablo2 is a trademark of Blizzard Entertainment.
	This library is done by volunteers and is neither 
	supported by nor otherwise affiliated with Blizzard Entertainment.
	You should NEVER use the library on communicate use.

	This program is distributed WITHOUT ANY WARRANTY,
	use it at your own risk.
*/

#ifndef INCLUDED_D2SERVER_H
#define INCLUDED_D2SERVER_H

#include "colorcode.h"

#define	DEFAULT_IDLE_SLEEP		10
#define	DEFAULT_BUSY_SLEEP		30
#define DEFAULT_MAX_GAME		100

typedef DWORD 	GAMEDATA,   * PGAMEDATA, 	* LPGAMEDATA;
typedef DWORD	PLAYERDATA, * PPLAYERDATA, 	* LPPLAYERDATA;
typedef DWORD	PLAYERMARK, * PPLAYERMARK, 	* LPPLAYERMARK;

typedef struct
{
	PLAYERMARK	PlayerMark;
	DWORD		dwReserved;
	UCHAR		CharName[16];
	UCHAR		AcctName[16];
} PLAYERINFO, * PPLAYERINFO, * LPPLAYERINFO;

#define		D2GS_GAMETYPE_CLOSE_HOST		0
#define		D2GS_GAMETYPE_OPEN_HOST			1
#define		D2GS_GAMETYPE_OPEN_NO_HOST		2
#define		D2GS_GAMETYPE_CLOSE_NO_HOST		3

#define		CHAT_MESSAGE_MAX_LEN			0x100
#define		CHAT_MESSAGE_TYPE_CHAT			0x01
#define		CHAT_MESSAGE_TYPE_WHISPER_TO	0x02
#define		CHAT_MESSAGE_TYPE_SYS_MESSAGE	0x04
#define		CHAT_MESSAGE_TYPE_WHISPER_FROM	0x06
#define		CHAT_MESSAGE_TYPE_SCROLL		0x07

#endif
