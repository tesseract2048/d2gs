#ifndef INCLUDED_TRUNK_H
#define INCLUDED_TRUNK_H

#define MaxClient 4096
#define RecvBufferSize 4096
#define SendBufferSize 8192
#define HashTableSize 524288
#define ConcurrentNum 2
#define MERGE_PACKET 1
#define ENABLE_HSAUTH 0

#include "d2gamelist.h"

typedef struct
{
	WSAOVERLAPPED Overlapped;
	int Id;
	char Type;
} IOCPOverlapped;

typedef struct
{
	int Len;
	char* Data;
	void* Next;
} IOCPQueueNode;

typedef struct
{
	int Count;
	IOCPQueueNode *Root;
} IOCPQueue;

typedef struct
{
	SOCKET Peer;
	char SendState;
	WSABUF RecvBuffer;
	char* SendQueue;
	char* SendBuf;
	int QueuePos;
	IOCPOverlapped RecvEvent;
	IOCPOverlapped SendEvent;
	IOCPOverlapped DisconnectEvent;
	IOCPOverlapped ConnectEvent;
	CRITICAL_SECTION QueueLock;
	int RecvOffset;
	WSABUF TempRecvBuffer;
} IOCPPeer;

typedef struct
{
	int Id;
	time_t ConnTime;
	char Flag;
	IOCPPeer Client;
	IOCPPeer Uplink;
	int RequestRate;
	int RemoteAddr;
	CRITICAL_SECTION Lock;
	D2CHARINFO *Player;
	time_t LastActive;
} IOCPClient;

typedef struct
{
	int Count;
	IOCPClient* Buffer[MaxClient];
	int Bottom;
	CRITICAL_SECTION Lock;
} IOCPReuseStack;

typedef struct
{
	int *Keys;
	int *Values;
	CRITICAL_SECTION Lock;
} Hashtable;

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

#pragma pack(pop, pack01)

int StartupTrunk();

#endif