/*
 * vars.h: header file for vars.c, declare global variables here
 */

#ifndef INCLUDED_VARS_H
#define INCLUDED_VARS_H


#include <windows.h>
#include "d2gs.h"


/* variales */
extern D2GSCONFIGS			d2gsconf;
extern BOOL					bGERunning;
extern CRITICAL_SECTION		csGameList;

/* functions */
int  D2GSVarsInitialize(void);
int  CleanupRoutineForVars(void);


#endif /* INCLUDED_VARS_H */