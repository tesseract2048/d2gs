/*
 * vars.h: header file for vars.c, declare global variables here
 */

#ifndef INCLUDED_VARS_H
#define INCLUDED_VARS_H


#include <windows.h>
#include "d2gelib/d2server.h"
#include "d2gs.h"


/* variales */
extern D2GSCONFIGS			d2gsconf;

extern D2GSStartFunc		 			D2GSStart;
extern D2GSSendDatabaseCharacterFunc 	D2GSSendDatabaseCharacter;
extern D2GSRemoveClientFromGameFunc		D2GSRemoveClientFromGame;
extern D2GSNewEmptyGameFunc				D2GSNewEmptyGame;
extern D2GSEndAllGamesFunc				D2GSEndAllGames;
extern D2GSSendClientChatMessageFunc	D2GSSendClientChatMessage;
extern D2GSSetTickCountFunc				D2GSSetTickCount;
extern D2GSSetACDataFunc				D2GSSetACData;
extern D2GSLoadConfigFunc				D2GSLoadConfig;
extern D2GSInitConfigFunc				D2GSInitConfig;
/* functions */


#endif /* INCLUDED_VARS_H */