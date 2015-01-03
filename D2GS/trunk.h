#ifndef INCLUDED_TRUNK_H
#define INCLUDED_TRUNK_H

typedef struct 
{
	SOCKET client;
	SOCKET uplink;
} t_d2game_connection;

#pragma pack(push, pack01, 1)

typedef struct
{
	unsigned char auth[10];
} GSPacket0x91;

typedef struct
{
	CHAR type;
	DWORD hash;
	WORD token;
	CHAR charid;
	DWORD ver;
	DWORD dwReserved1;
	DWORD dwReserved2;
	CHAR bReserved;
	CHAR charname[16];
} GSPacket0x68;


typedef struct
{
	CHAR type;
	CHAR suffix;
} GSPacket0xAF;

typedef struct
{
	CHAR type;
} GSPacket0x6A;

BOOL StartupTrunk();
DWORD WINAPI MakeBenchmarkConnect(LPVOID lpParam);

#pragma pack(pop, pack01)
#endif