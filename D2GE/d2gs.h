/*
 * d2gs.h: declaration of structures and constants
 *
 * 2001-08-20 faster
 *   modify
 */

#ifndef INCLUDED_D2GS_H
#define INCLUDED_D2GS_H


#include <windows.h>

/* constants */
#define D2CSERVER					0x01
#define D2DBSERVER					0x02
#define D2GSERVER					0x04

/* version */
#define D2GS_VERSION				0x01110000
#define VERNUM						"1.10.3.6"
#define BUILDDATE					__DATE__ " " __TIME__
#define D2GS_VERSION_STRING			D2COLOR_GOLDEN "D2GS Version " VERNUM ", build on " BUILDDATE

#define MAX_LINE_LEN				1024
#define	DEBUG_DUMPSIZE				0x40

/* string lengtn */
#define MAX_GAMENAME_LEN			16
#define MAX_GAMEPASS_LEN			16
#define MAX_GAMEDESC_LEN			32
#define MAX_ACCTNAME_LEN			16
#define MAX_CHARNAME_LEN			16
#define MAX_IPADDR_LEN			16
#define MAX_REALMNAME_LEN			32

#define MAX_CHAR_IN_GAME			8
#define TIMER_TICK_IN_MS			100
#define GET_DATA_TIMEOUT			1000
#define SEND_MOTD_INTERVAL			2000

/* some error code */
#define D2GSERROR_NOT_ENOUGH_MEMORY		0x80
#define D2GSERROR_BAD_PARAMETER			0x81
#define D2GSERROR_GAME_IS_FULL			0x82
#define D2GSERROR_CHAR_ALREADY_IN_GAME	0x83


/* utilities */
#define YESNO(n) (n)?"Yes":"No"
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define NELEMS(array) (sizeof(array)/sizeof(array[0]))

typedef struct RAW_D2GSCONFIGS {
	DWORD		d2csip;		/* in network order */
	DWORD		d2dbsip;	/* in network order */
	u_short		d2csport;	/* in network order */
	u_short		d2dbsport;	/* in network order */
	BOOL		enablentmode;
	BOOL		enableprecachemode;
	BOOL		enablegepatch;
	BOOL		enablegelog;
	BOOL		enablegemsg;
	BOOL		debugnetpacket;
	BOOL		debugeventcallback;
	DWORD		checksum;
	DWORD		gemaxgames;
	DWORD		gsmaxgames;
	DWORD		idlesleep;
	DWORD		busysleep;
	DWORD		charpendingtimeout;
	DWORD		intervalreconnectd2cs;
	DWORD		admintimeout;
	DWORD		maxgamelife;
	DWORD		gsshutdowninterval;
	DWORD		multicpumask;
	u_char		adminpwd[64];
	u_short		adminport;
	u_char		d2cssecrect[32];
	u_char		motd[256];
	u_char		eventmotd[256];
	u_char		versionMotd[256];
	u_char		roomMotd[256];
	BOOL		enablegslog;
	u_char		listenaddr[32];
	DWORD		listenport;
	DWORD		gethread;
	DWORD		synprot;
} D2GSCONFIGS, *PD2GSCONFIGS;


#define PACKET_PEER_RECV_FROM_D2CS		0x01
#define PACKET_PEER_SEND_TO_D2CS		0x02
#define PACKET_PEER_RECV_FROM_D2DBS		0x03
#define PACKET_PEER_SEND_TO_D2DBS		0x04
typedef struct RAW_D2GSPACKET {
	u_short		peer;		/* where packet from or to? D2CS or D2DBS */
	u_short		datalen;	/* valid date length */
	u_char		data[65536];
} D2GSPACKET, *PD2GSPACKET;

/* function */
void patchAddr(DWORD addr, BYTE val);
DWORD searchAndPatch(DWORD startAddr, DWORD searchRange, char* pattern, DWORD patternLen, char* patch, DWORD patchLen);

#endif /* INCLUDED_D2GS_H */