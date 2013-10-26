#ifndef INCLUDED_EVENTLOG_H
#define INCLUDED_EVENTLOG_H


#define EVENT_TIME_FORMAT "% %d %H:%M:%S"
#define EVENT_TIME_MAXLEN 48


/* functions */
int  D2GSEventLogInitialize(void);
void D2GSEventLogCleanup(void);
void D2GSEventLog(char const *module, char const *fmt, ...);
void LogAP(LPCSTR lpModule, LPCSTR lpFormat, va_list ap);
void PortraitDump(LPCSTR lpAccountName, LPCSTR lpCharName, LPCSTR lpCharPortrait);


#endif
