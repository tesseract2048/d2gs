#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include "d2gs.h"
#include "vars.h"
#include "eventlog.h"

static FILE *gestrm = NULL;			/* for game engine */

#define DEBUG_ON_CONSOLE 1

FILE* getstrm()
{
	return gestrm;
}

/*********************************************************************
 * Purpose: to initialize the envet log system
 * Return: TRUE(success) or FALSE(failed)
 *********************************************************************/
int D2GEEventLogInitialize(int d2ge_id)
{
	char buf[255];
	sprintf(buf, "d2ge-%d.log", d2ge_id);
	gestrm    = fopen(buf, "a");

	if (gestrm) return TRUE;
	else return FALSE;

} /* End of D2GEEventLogInitialize() */


/*********************************************************************
 * Purpose: to cleanup the envet log system
 * Return: none
 *********************************************************************/
void D2GEEventLogCleanup(void)
{
	if (gestrm) fclose(gestrm);

	gestrm = NULL;

	return;

} /* End of D2GEEventLogCleanup() */


/*********************************************************************
 * Purpose: to log the event to the file or the standard output, for GE
 * Return: none
 *********************************************************************/
void D2GEEventLog(char const * module, char const * fmt, ...)
{
	va_list		args;
	char		time_string[EVENT_TIME_MAXLEN];
	SYSTEMTIME	st;

    if (!d2gsconf.enablegelog) return;
	if (!gestrm) return;

	GetLocalTime(&st);
	sprintf(time_string, "%02d/%02d %02d:%02d:%02d.%03d", st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    
	if (!module) {
		fprintf(gestrm,"%s eventlog: got NULL module\n", time_string);
		fflush(gestrm);
#ifdef DEBUG_ON_CONSOLE
		fprintf(stdout,"%s eventlog: got NULL module\n", time_string);
#endif
		return;
	}
	if (!fmt) {
		fprintf(gestrm,"%s eventlog: got NULL fmt\n", time_string);
		fflush(gestrm);
#ifdef DEBUG_ON_CONSOLE
		fprintf(stdout,"%s eventlog: got NULL fmt\n", time_string);
#endif
		return;
	}
    
	fprintf(gestrm,"%s %s: ", time_string, module);
#ifdef DEBUG_ON_CONSOLE
	fprintf(stdout,"%s %s: ", time_string, module);
#endif
	va_start(args, fmt);
	vfprintf(gestrm, fmt, args);
#ifdef DEBUG_ON_CONSOLE
	vfprintf(stdout, fmt, args);
#endif
	va_end(args);
	fprintf(gestrm, "\n");
	fflush(gestrm);
#ifdef DEBUG_ON_CONSOLE
	fprintf(stdout, "\n");
#endif
	return;

} /* End of D2GEEventLog() */ 


/*********************************************************************
 * Purpose: to log the argument
 * Return: none
 *********************************************************************/
void LogAP(LPCSTR lpModule, LPCSTR lpFormat, va_list ap)
{
	SYSTEMTIME	st;
	DWORD		len;
	char		msg[MAX_LINE_LEN];
	char		tmp[MAX_LINE_LEN];

	if (!d2gsconf.enablegemsg) return;
	if (!lpModule || !lpFormat)  return;
	GetLocalTime(&st);
	len = wsprintf(msg, "%02d/%02d %02d:%02d:%02d.%03d", st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	len += vsprintf(tmp, lpFormat, ap);
	strcat(msg, tmp);
	strcat(msg, "\n");
	len += strlen("\n");
	fprintf(gestrm, "%s", msg);
	fflush(gestrm);
#ifdef DEBUG_ON_CONSOLE
	fprintf(stdout, "%s", msg);
#endif
	return;

} /* End of LogAP */

