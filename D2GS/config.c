#include <winsock2.h>
#include <windows.h>
#include <winreg.h>
#include <stdio.h>
#include <stdlib.h>
#include "d2gs.h"
#include "config.h"
#include "eventlog.h"
#include "vars.h"
#include "d2gelib/d2server.h"


/*********************************************************************
 * Purpose: to read configurations to the D2GSCONFIGS structure
 * Return: TRUE(success) or FALSE(failed)
 *********************************************************************/
int D2GSReadConfig(void)
{
	HKEY		hKey;
	BOOL		result;
	u_long		ipaddr;
	DWORD		dwval;
	char		strbuf[256];

	result = FALSE;
	if (!RegkeyOpen(HKEY_LOCAL_MACHINE, REGKEY_ROOT, &hKey, KEY_READ)) {
		D2GSEventLog("D2GSReadConfig", "Can't open registry key '\\\\HKEY_LOCAL_MACHINE\\%s'", REGKEY_ROOT);
		//return result;
	}

	/* D2CSIP */
	if (!RegkeyReadString(hKey, REGKEY_D2CSIP, strbuf, sizeof(strbuf))) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s'", REGKEY_D2CSIP);
		goto tocloseregkey;
	}
	printf(":%s", strbuf);
	ipaddr = inet_addr(strbuf);
	if (ipaddr==INADDR_NONE) {
		printf("i");
		D2GSEventLog("D2GSReadConfig", "Invalid D2CSIP '%s'", strbuf);
		goto tocloseregkey;
	}
	d2gsconf.d2csip = ipaddr;

	/* D2DBSIP */
	if (!RegkeyReadString(hKey, REGKEY_D2DBSIP, strbuf, sizeof(strbuf))) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s'", REGKEY_D2DBSIP);
		goto tocloseregkey;
	}
	ipaddr = inet_addr(strbuf);
	if (ipaddr==INADDR_NONE) {
		D2GSEventLog("D2GSReadConfig", "Invalid D2DBSIP '%s'", strbuf);
		goto tocloseregkey;
	}
	d2gsconf.d2dbsip = ipaddr;

	/* D2CSPORT */
	if (!RegkeyReadDWORD(hKey, REGKEY_D2CSPORT, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_D2CSPORT, DEFAULT_D2CS_PORT);
		d2gsconf.d2csport = DEFAULT_D2CS_PORT;
	} else
		d2gsconf.d2csport = htons((u_short)dwval);

	/* D2DBSPORT */
	if (!RegkeyReadDWORD(hKey, REGKEY_D2DBSPORT, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_D2DBSPORT, DEFAULT_D2DBS_PORT);
		d2gsconf.d2csport = DEFAULT_D2DBS_PORT;
	} else
		d2gsconf.d2dbsport = htons((short)dwval);

	/* MAXGAMES */
	if (!RegkeyReadDWORD(hKey, REGKEY_MAXGAMES, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_MAXGAMES, DEFAULT_MAX_GAMES);
		d2gsconf.gemaxgames = DEFAULT_MAX_GAMES;
	} else
		d2gsconf.gemaxgames = dwval;
	d2gsconf.gsmaxgames = 0;

	/* ENABLENTMODE */
	if (!RegkeyReadDWORD(hKey, REGKEY_ENABLENTMODE, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_ENABLENTMODE, DEFAULT_NT_MODE);
		d2gsconf.enablentmode = DEFAULT_NT_MODE;
	} else
		d2gsconf.enablentmode = (BOOL)dwval;

	/* ENABLEGEPATCH */
	if (!RegkeyReadDWORD(hKey, REGKEY_ENABLEGEPATCH, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_ENABLEGEPATCH, DEFAULT_GE_PATCH);
		d2gsconf.enablegepatch = DEFAULT_GE_PATCH;
	} else
		d2gsconf.enablegepatch = (BOOL)dwval;

	/* ENABLEPRECACHEMODE */
	if (!RegkeyReadDWORD(hKey, REGKEY_ENABLEPRECACHEMODE, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_ENABLEPRECACHEMODE, DEFAULT_PRECACHE_MODE);
		d2gsconf.enableprecachemode = DEFAULT_PRECACHE_MODE;
	} else
		d2gsconf.enableprecachemode = (BOOL)dwval;

	/* ENABLEGELOG */
	if (!RegkeyReadDWORD(hKey, REGKEY_ENABLEGELOG, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_ENABLEGELOG, DEFAULT_GE_LOG);
		d2gsconf.enablegelog = DEFAULT_GE_LOG;
	} else
		d2gsconf.enablegelog = (BOOL)dwval;

	/* ENABLEGSLOG */
	if (!RegkeyReadDWORD(hKey, REGKEY_ENABLEGSLOG, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_ENABLEGELOG, DEFAULT_GS_LOG);
		d2gsconf.enablegslog = DEFAULT_GS_LOG;
	} else
		d2gsconf.enablegslog = (BOOL)dwval;

	/* ENABLEGEMSG */
	if (!RegkeyReadDWORD(hKey, REGKEY_ENABLEGEMSG, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_ENABLEGEMSG, DEFAULT_GE_MSG);
		d2gsconf.enablegemsg = DEFAULT_GE_MSG;
	} else
		d2gsconf.enablegemsg = (BOOL)dwval;

	/* DEBUGNETPACKET */
	if (!RegkeyReadDWORD(hKey, REGKEY_DEBUGNETPACKET, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_DEBUGNETPACKET, DEFAULT_DEBUGNETPACKET);
		d2gsconf.debugnetpacket = DEFAULT_DEBUGNETPACKET;
	} else
		d2gsconf.debugnetpacket = (BOOL)dwval;

	/* DEBUGEVENTCALLBACK */
	if (!RegkeyReadDWORD(hKey, REGKEY_DEBUGEVENTCALLBACK, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_DEBUGEVENTCALLBACK, DEFAULT_DEBUGEVENTCALLBACK);
		d2gsconf.debugeventcallback = DEFAULT_DEBUGEVENTCALLBACK;
	} else
		d2gsconf.debugeventcallback = (BOOL)dwval;

	/* IDLESLEEP */
	if (!RegkeyReadDWORD(hKey, REGKEY_IDLESLEEP, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_IDLESLEEP, DEFAULT_IDLE_SLEEP);
		d2gsconf.idlesleep = DEFAULT_IDLE_SLEEP;
	} else
		d2gsconf.idlesleep = dwval;

	/* BUSYSLEEP */
	if (!RegkeyReadDWORD(hKey, REGKEY_BUSYSLEEP, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_BUSYSLEEP, DEFAULT_BUSY_SLEEP);
		d2gsconf.busysleep = DEFAULT_BUSY_SLEEP;
	} else
		d2gsconf.busysleep = dwval;

	/* CHARPENDINGTIMEOUT */
	if (!RegkeyReadDWORD(hKey, REGKEY_CHARPENDINGTIMEOUT, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_CHARPENDINGTIMEOUT, DEFAULT_CHARPENDINGTIMEOUT);
		d2gsconf.charpendingtimeout = DEFAULT_CHARPENDINGTIMEOUT;
	} else
		d2gsconf.charpendingtimeout = dwval;

	/* INTERVALRECONNECTD2CS */
	if (!RegkeyReadDWORD(hKey, REGKEY_INTERVALRECONNECTD2CS, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_INTERVALRECONNECTD2CS, DEFAULT_INTERVALRECONNECTD2CS);
		d2gsconf.intervalreconnectd2cs = DEFAULT_INTERVALRECONNECTD2CS;
	} else
		d2gsconf.intervalreconnectd2cs = dwval;

	/* ADMINPWD */
	ZeroMemory(d2gsconf.adminpwd, sizeof(d2gsconf.adminpwd));
	if (!RegkeyReadString(hKey, REGKEY_ADMINPWD, strbuf, sizeof(strbuf))) {
		strcpy(d2gsconf.adminpwd, "d2gsdmin");
	} else
		strncpy(d2gsconf.adminpwd, strbuf, sizeof(d2gsconf.adminpwd)-1);

	/* D2CSSECRECT */
	ZeroMemory(d2gsconf.d2cssecrect, sizeof(d2gsconf.d2cssecrect));
	if (!RegkeyReadString(hKey, REGKEY_D2CSSECRECT, strbuf, sizeof(strbuf))) {
		strcpy(d2gsconf.d2cssecrect, "");
	} else
		strncpy(d2gsconf.d2cssecrect, strbuf, sizeof(d2gsconf.d2cssecrect)-1);

	/* ADMINPORT */
	if (!RegkeyReadDWORD(hKey, REGKEY_ADMINPORT, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_ADMINPORT, DEFAULT_ADMIN_PORT);
		d2gsconf.adminport = DEFAULT_D2DBS_PORT;
	} else
		d2gsconf.adminport = htons((short)dwval);

	/* ADMINTIMEOUT */
	if (!RegkeyReadDWORD(hKey, REGKEY_ADMINTIMEOUT, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_ADMINTIMEOUT, DEFAULT_ADMIN_TIMEOUT);
		d2gsconf.admintimeout = DEFAULT_ADMIN_TIMEOUT;
	} else
		d2gsconf.admintimeout = dwval;

	/* MAXGAMELIFE */
	if (!RegkeyReadDWORD(hKey, REGKEY_MAXGAMELIFE, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_MAXGAMELIFE, DEFAULT_MAXGAMELIFE);
		d2gsconf.maxgamelife = DEFAULT_MAXGAMELIFE;
	} else
		d2gsconf.maxgamelife = dwval;

	/* GSSHUTDOWNINTERVAL */
	if (!RegkeyReadDWORD(hKey, REGKEY_GSSHUTDOWNINTERVAL, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_GSSHUTDOWNINTERVAL, DEFAULT_GS_SHUTDOWN_INTERVAL);
		d2gsconf.gsshutdowninterval = DEFAULT_GS_SHUTDOWN_INTERVAL;
	} else
		d2gsconf.gsshutdowninterval = dwval;

	/* MULTICPUMASK */
	if (!RegkeyReadDWORD(hKey, REGKEY_MULTICPUMASK, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_MULTICPUMASK, DEFAULT_MULTICPUMASK);
		d2gsconf.multicpumask = DEFAULT_MULTICPUMASK;
	} else
		d2gsconf.multicpumask = dwval;

	/* LISTENPORT */
	if (!RegkeyReadDWORD(hKey, REGKEY_LISTENPORT, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_LISTENPORT, DEFAULT_LISTENPORT);
		d2gsconf.listenport = DEFAULT_LISTENPORT;
	} else
		d2gsconf.listenport = dwval;

	/* LISTENADDR */
	ZeroMemory(d2gsconf.listenaddr, sizeof(d2gsconf.listenaddr));
	if (!RegkeyReadString(hKey, REGKEY_LISTENADDR, strbuf, sizeof(strbuf))) {
		strcpy(d2gsconf.listenaddr, "0.0.0.0");
	} else
		strncpy(d2gsconf.listenaddr, strbuf, sizeof(d2gsconf.listenaddr)-1);

	/* GETHREAD */
	if (!RegkeyReadDWORD(hKey, REGKEY_GETHREAD, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_GETHREAD, DEFAULT_GETHREAD);
		d2gsconf.gethread = DEFAULT_GETHREAD;
	} else
		d2gsconf.gethread = dwval;

	/* SYNPROT */
	if (!RegkeyReadDWORD(hKey, REGKEY_SYNPROT, &dwval)) {
		D2GSEventLog("D2GSReadConfig", "Can't read key '%s', set to default %d",
				REGKEY_SYNPROT, DEFAULT_SYNPROT);
		d2gsconf.synprot = DEFAULT_SYNPROT;
	} else
		d2gsconf.synprot = dwval;

	/* MOTD */
	ZeroMemory(d2gsconf.motd, sizeof(d2gsconf.motd));
	if (RegkeyReadString(hKey, REGKEY_MOTD, strbuf, sizeof(strbuf)))
		strncpy(d2gsconf.motd, strbuf, sizeof(d2gsconf.motd)-1);
	strcpy(d2gsconf.motd, strbuf);
	string_color(d2gsconf.motd);

	d2gsconf.eventmotd[0] = 0;
	d2gsconf.roomMotd[0] = 0;

	result = TRUE;

tocloseregkey:
	RegkeyClose(hKey);
	return result;

} /* End of D2GSReadConfig() */


/*********************************************************************
 * Purpose: to open an existing registry key
 * Return: TRUE(success) or FALSE(failed)
 *********************************************************************/
int RegkeyOpen(HKEY hKeyRoot, LPCTSTR lpSubKey, PHKEY hKey, REGSAM sam)
{
	int error = RegOpenKeyEx(hKeyRoot, lpSubKey, 0, sam, hKey);
	if (error==ERROR_SUCCESS)
		return TRUE;
	else
		return FALSE;

} /* End of RegkeyOpen() */


/*********************************************************************
 * Purpose: to close an opened registry key
 * Return: none
 *********************************************************************/
void RegkeyClose(HKEY hKey)
{
	if (hKey)
		RegCloseKey(hKey);
	return;

} /* End of RegkeyClose() */


/*********************************************************************
 * Purpose: to read a string key to the buffer
 * Return: TRUE(success) or FALSE(failed)
 *********************************************************************/
int RegkeyReadString(HKEY hKey, LPCTSTR name, char *buf, DWORD buflen)
{
	DWORD	dwType, dwLen;
	LONG	lReturn;

	if ((!hKey) || (!name))
		return FALSE;
	dwLen = buflen;
	ZeroMemory(buf, buflen);
	lReturn = RegQueryValueEx(hKey, name, NULL, &dwType, buf, &dwLen);
	if (lReturn==ERROR_SUCCESS) {
		*(buf+buflen-1) = 0;
		return TRUE;
	} else
		return FALSE;

} /* End of RegkeyReadString() */


/*********************************************************************
 * Purpose: to read a dword key to the variable
 * Return: TRUE(success) or FALSE(failed)
 *********************************************************************/
int RegkeyReadDWORD(HKEY hKey, LPCTSTR name, DWORD *val)
{
	DWORD	dwType, dwLen, dwVal;
	LONG	lReturn;

	if ((!hKey) || (!name))
		return FALSE;
	dwLen = sizeof(dwVal);
	lReturn = RegQueryValueEx(hKey, name, NULL, &dwType, (LPBYTE)&dwVal, &dwLen);
	if (lReturn==ERROR_SUCCESS) {
		*val = dwVal;
		return TRUE;
	} else
		return FALSE;

} /* End of RegkeyReadDWORD() */


/*********************************************************************
 * Purpose: to write a string key from the buffer
 * Return: TRUE(success) or FALSE(failed)
 *********************************************************************/
int RegkeyWriteString(HKEY hKey, LPCTSTR name, LPCSTR buf)
{
	DWORD	dwLen;
	LONG	lReturn;

	if ((!hKey) || (!name))	return FALSE;
	dwLen = strlen(buf)+1;
	lReturn = RegSetValueEx(hKey, name, 0L, REG_SZ, (const BYTE*)buf, dwLen);
	if (lReturn==ERROR_SUCCESS)
		return TRUE;
	else
		return FALSE;

} /* End of RegkeyWriteString() */


/*********************************************************************
 * Purpose: to write a dword key from the variable
 * Return: TRUE(success) or FALSE(failed)
 *********************************************************************/
int RegkeyWriteDWORD(HKEY hKey, LPCTSTR name, DWORD val)
{
	DWORD	dwVal, dwLen;
	LONG	lReturn;

	if ((!hKey) || (!name))	return FALSE;
	dwVal = val;
	dwLen = sizeof(dwVal);
	lReturn = RegSetValueEx(hKey, name, 0L, REG_DWORD, (const BYTE*)&dwVal, dwLen);
	if (lReturn==ERROR_SUCCESS)
		return TRUE;
	else
		return FALSE;

} /* End of RegkeyWriteDWORD() */


/*********************************************************************
 * Purpose: to set the string value to the registry
 * Return: TRUE(success) or FALSE(failed)
 *********************************************************************/
int D2GSSetConfigDWORD(LPCSTR keyname, DWORD dwVal)
{
	HKEY		hKey;

	if (!RegkeyOpen(HKEY_LOCAL_MACHINE, REGKEY_ROOT, &hKey, KEY_SET_VALUE))
		return FALSE;
	if (!RegkeyWriteDWORD(hKey, keyname, dwVal)) {
		RegkeyClose(hKey);
		return FALSE;
	} else {
		RegkeyClose(hKey);
		return TRUE;
	}

} /* End of D2GSSetConfigDWORD() */


/*********************************************************************
 * Purpose: to set the string value to the registry
 * Return: TRUE(success) or FALSE(failed)
 *********************************************************************/
int D2GSSetConfigString(LPCSTR keyname, LPCSTR str)
{
	HKEY		hKey;

	if (!RegkeyOpen(HKEY_LOCAL_MACHINE, REGKEY_ROOT, &hKey, KEY_SET_VALUE))
		return FALSE;
	if (!RegkeyWriteString(hKey, keyname, str)) {
		RegkeyClose(hKey);
		return FALSE;
	} else {
		RegkeyClose(hKey);
		return TRUE;
	}

} /* End of D2GSSetConfigString() */


/*********************************************************************
 * Purpose: to set the MaxGameLife
 * Return: TRUE(success) or FALSE(failed)
 *********************************************************************/
int D2GSSetMaxGameLife(DWORD maxgamelife)
{
	d2gsconf.maxgamelife = maxgamelife;
	return D2GSSetConfigDWORD(REGKEY_MAXGAMELIFE, maxgamelife);

} /* End of D2GSSetMaxGameLife() */


/*********************************************************************
 * Purpose: to set the AdminPadssword
 * Return: TRUE(success) or FALSE(failed)
 *********************************************************************/
int D2GSSetAdminPassword(LPCSTR password)
{
	return D2GSSetConfigString(REGKEY_ADMINPWD, password);

} /* End of D2GSSetAdminPassword() */


/*********************************************************************
 * Purpose: to set the MaxGames
 * Return: TRUE(success) or FALSE(failed)
 *********************************************************************/
int D2GSSetMaxGames(DWORD maxgames)
{
	d2gsconf.gsmaxgames = maxgames;
	return D2GSSetConfigDWORD(REGKEY_MAXGAMES, maxgames);

} /* End of D2GSSetMaxGameLife() */


