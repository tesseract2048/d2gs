#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "d2gs.h"
#include "d2ge.h"
#include "eventlog.h"
#include "vars.h"
#include "net.h"
#include "handle_s2s.h"
#include "handle_d2game.h"
#include "debug.h"
#include "d2gamelist.h"
#include "d2game_protocol.h"
#include "trunk.h"
#include "nfd.h"

static HANDLE PortListen;
static HANDLE PortClientArray[ConcurrentNum];
static HANDLE PortUplinkArray[ConcurrentNum];
static HANDLE ThreadListen[ConcurrentNum];
static HANDLE ThreadClient[ConcurrentNum];
static HANDLE ThreadUplink[ConcurrentNum];
static HANDLE ThreadKill;

static LPFN_ACCEPTEX lpAcceptEx = NULL;
static LPFN_DISCONNECTEX lpDisconnectEx = NULL;
static LPFN_CONNECTEX lpConnectEx = NULL;

static IOCPReuseStack ReuseStack;

static Hashtable ConnectRate;

static struct sockaddr_in bind_local_addr;

SOCKET Listener;
IOCPClient *ClientPool;

GSPacket0xAF AFPacket;

char HSAuthDummy[] = {0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};

void PushData(IOCPPeer *Peer, char* Data, int Len);
void SendQueueData(IOCPPeer *Peer);
void ResetPeer(IOCPPeer *Peer);
void CloseClientByFlag(IOCPClient *Client);

void HashSet(Hashtable *Hash, int Key, int Value);
int HashGet(Hashtable *Hash, int Key);
int HashIncrement(Hashtable *Hash, int Key, int Step);

int PopReuse(IOCPReuseStack *Stack)
{
	int Id = -1;
	EnterCriticalSection(&Stack->Lock);
	if (Stack->Count == 0)
	{
		return -1;
	}
	Id = Stack->Buffer[Stack->Bottom++];
	Stack->Count --;
	LeaveCriticalSection(&Stack->Lock);
	return Id;
}

void PushReuse(int Id, BOOL WaitMSL)
{
	D2GSEventLog("IOCPPool", "Release pool %d", Id);
	ClientPool[Id].Flag = ClientPool[Id].Flag | 0xC0;
	if (WaitMSL)
	{
		ClientPool[Id].ConnTime = time(NULL);
	}
	else
	{
		ClientPool[Id].ConnTime = 0;
	}
}

void AcceptClient()
{
	DWORD dwBytes = 0;
	int Id;
	while ((Id = PopReuse(&ReuseStack)) == -1)
	{
		Sleep(100);
	}
	D2GSEventLog("IOCPPool", "Accepting connection on pool %d", Id);
	ResetPeer(&ClientPool[Id].Client);
	ClientPool[Id].Client.Peer = InitalizeSocket(NULL, NULL);
	//lpAcceptEx(Listener, ClientPool[Id].Client.Peer, ClientPool[Id].Client.RecvBuffer.buf, RecvBufferSize - ((sizeof(SOCKADDR_IN)+16)*2), sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &dwBytes, (LPOVERLAPPED)&(ClientPool[Id].Client.RecvEvent));
	lpAcceptEx(Listener, ClientPool[Id].Client.Peer, ClientPool[Id].Client.RecvBuffer.buf, 0, sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &dwBytes, (LPOVERLAPPED)&(ClientPool[Id].Client.RecvEvent));
}

int Incoming(IOCPClient *Client)
{
	D2GSEventLog("IOCPPool", "Incoming on pool %d", Client->Id);
	
	if (ENABLE_HSAUTH) PushData(&Client->Client, HSAuthDummy, 10);
	PushData(&Client->Client, &AFPacket, sizeof(GSPacket0xAF));

	return 0;
}

int Dispatch(IOCPClient *Client, char* Buffer, int Len)
{
	if (Client->Flag & 0x03)
	{
		return 0;
	}
	Client->RouteTimer = GetTickCount();
	if ((Client->Flag & 0x10) == 0x10)
	{
		//Uplink connected
		int Pointer = 0;
		if (Len <= 0) return -1;
		while (Pointer < Len)
		{
			int HandleRet = handle_d2game(&Buffer[Pointer], Len - Pointer, Client);
			if (HandleRet < 0)
			{
				return -1;
			}
			if (HandleRet == 0) break;
			Pointer += HandleRet;
		}
		PushData(&Client->Uplink, Buffer, Pointer);
		return Pointer;
	}
	else
	{
		if (!ENABLE_HSAUTH || ((Client->Flag & 0x80) == 0x80))
		{
			struct sockaddr_in srv_addr;
			struct GEINFO *ge;
			D2CHARINFO *lpChar;
			GSPacket0x68 *packet = Buffer;
			DWORD dwBytes = 0;
			if (Len < sizeof(GSPacket0x68)) return 0;
			if(packet->type != 0x68) return -1;
			lpChar = D2GSFindPendingCharByCharName(packet->charname);
			if (!lpChar) return -1;
			memset(&srv_addr, 0, sizeof(struct sockaddr_in));
			ge = GetGEById(lpChar->ge);
			srv_addr.sin_family = AF_INET;
			srv_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
			srv_addr.sin_port = htons(ge->trunk_port);
			EnterCriticalSection(&Client->Lock);
			Client->Flag = Client->Flag | 0x10;
			LeaveCriticalSection(&Client->Lock);
			Client->Uplink.SendState = -1;
			/*memcpy(Client->Uplink.SendBuffer.buf, Buffer, Len);
			Client->Uplink.SendBuffer.len = Len;*/
			PushData(&Client->Uplink, Buffer, Len);
			lpConnectEx(Client->Uplink.Peer, &srv_addr, sizeof(struct sockaddr_in), NULL, 0, &dwBytes, (LPWSAOVERLAPPED)&Client->Uplink.ConnectEvent);
			//printf("ConnectEx of pool %d: Error %d\n", Client->Id, WSAGetLastError());
			return sizeof(GSPacket0x68);
		}
		else
		{
			if (Len < 10) return 0;
			EnterCriticalSection(&Client->Lock);
			Client->Flag = Client->Flag | 0x80;
			LeaveCriticalSection(&Client->Lock);
			return 10;
		}
	}
}

int DispatchCallback(IOCPClient *Client, char* Buffer, int Len)
{
	if (Client->Flag & 0x0C)
	{
		return 0;
	}
	printf("Route Timer: %d ms\n", GetTickCount() - Client->RouteTimer);
	if ((Client->Flag & 0x20) == 0x20)
	{
		PushData(&Client->Client, Buffer, Len);
	}
	else
	{
		EnterCriticalSection(&Client->Lock);
		Client->Flag = Client->Flag | 0x20;
		LeaveCriticalSection(&Client->Lock);
		Client->Uplink.SendState = 0;
		SendQueueData(&Client->Uplink);
	}
	return 0;
}

void ResetPeer(IOCPPeer *Peer)
{
	memset(&Peer->RecvEvent, 0, sizeof(OVERLAPPED));
	memset(&Peer->SendEvent, 0, sizeof(OVERLAPPED));
	memset(&Peer->DisconnectEvent, 0, sizeof(OVERLAPPED));
	memset(&Peer->ConnectEvent, 0, sizeof(OVERLAPPED));
	Peer->QueuePos = 0;
	Peer->SendState = 0;
	Peer->RecvOffset = 0;
}

DWORD WINAPI ThreadListenLoop(LPVOID lpParam)
{
	int ConcurrentId = (int)lpParam;
	void* IOKey;
	IOCPOverlapped* Overlapped;
	IOCPClient *Client;
	DWORD RecvLen = 0;
	DWORD Flags = 0;
	int ret = 0;
	char NoDelay = 1;
	struct sockaddr_in *remote_addr;
	int addr;

	AcceptClient();
	while (TRUE)
	{
		ret = GetQueuedCompletionStatus(PortListen, &RecvLen, (PULONG_PTR)&IOKey, (LPOVERLAPPED*)&Overlapped, INFINITE);

		Client = &ClientPool[Overlapped->Id];

		remote_addr = &Client->Client.RecvBuffer.buf[38];
		addr = remote_addr->sin_addr.S_un.S_addr;
		
		if (HashIncrement(&ConnectRate, addr, 1) > 20)
		{
			HashIncrement(&ConnectRate, addr, -1);
			closesocket(Client->Client.Peer);
			EnterCriticalSection(&ReuseStack.Lock);
			ReuseStack.Buffer[--ReuseStack.Bottom] = Client->Id;
			ReuseStack.Count ++;
			LeaveCriticalSection(&ReuseStack.Lock);
			AcceptClient();
			continue;
		}

		CreateIoCompletionPort((HANDLE)Client->Client.Peer, PortClientArray[Client->Id % ConcurrentNum], (ULONG_PTR)Client, 0);

		setsockopt(Client->Client.Peer, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&Listener, sizeof(Listener));
		ResetPeer(&Client->Client);
		ResetPeer(&Client->Uplink);
		Client->Uplink.Peer = InitalizeSocket(PortUplinkArray[Client->Id % ConcurrentNum], Client);
		bind(Client->Uplink.Peer, &bind_local_addr, sizeof(struct sockaddr_in));

		Client->RemoteAddr = addr;
		Client->Flag = 0;
		Client->ConnTime = time(NULL);
		Client->RequestRate = 0;
		Incoming(Client);
		WSARecv(Client->Client.Peer, &Client->Client.RecvBuffer, 1, &RecvLen, &Flags, (LPWSAOVERLAPPED)&(Client->Client.RecvEvent), NULL);

		AcceptClient();
	}
}

void CloseClientByFlag(IOCPClient *Client)
{
	int ret;
	if ((Client->Flag & 0x40) == 0x40) return;
	EnterCriticalSection(&Client->Lock);
	if ((Client->Flag & 0x08) == 0)
	{
		//Disconnect client
		Client->Flag = Client->Flag | 0x08;
		WSACancelAsyncRequest(Client->Client.Peer);
		closesocket(Client->Client.Peer);
		Client->Flag = Client->Flag | 0x04;
	}
	if ((Client->Flag & 0x02) == 0)
	{
		//Disconnect uplink
		Client->Flag = Client->Flag | 0x02;
		WSACancelAsyncRequest(Client->Uplink.Peer);
		closesocket(Client->Uplink.Peer);
		Client->Flag = Client->Flag | 0x01;
	}
	if ((Client->Flag & 0x05) == 0x05)
	{
		HashIncrement(&ConnectRate, Client->RemoteAddr, -1);
		Client->Flag = Client->Flag | 0x40;
		PushReuse(Client->Id, FALSE);
	}
	LeaveCriticalSection(&Client->Lock);
}

void SendQueueData(IOCPPeer *Peer)
{
	DWORD dwBytes = 0;
	DWORD dwFlags = 0;
	int ret = 0;
	/*IOCPQueueNode *Node;
	IOCPQueueNode *Next;
	IOCPQueueNode *NewNode;*/
	if (Peer->QueuePos > 0)
	{
		//HOW TO DELAY 1 MS.................................... Sleep(1);
		Peer->SendState = 1;
		//Sleep(5);
		EnterCriticalSection(&Peer->QueueLock);
		memcpy(Peer->SendBuffer.buf, Peer->SendQueue.buf, Peer->QueuePos);
		Peer->SendBuffer.len = Peer->QueuePos;
		Peer->QueuePos = 0;
		memset(&Peer->SendEvent, 0, sizeof(OVERLAPPED));
		ret = WSASend(Peer->Peer, &Peer->SendBuffer, 1, &dwBytes, dwFlags, (LPWSAOVERLAPPED)&(Peer->SendEvent), NULL);
		/*if (ret == 0)
		{
			Peer->SendState = 0;
		}
		else
		{
			printf("***** WSASend returned %d (%d)\n", ret, WSAGetLastError());
		}*/
		LeaveCriticalSection(&Peer->QueueLock);
	}
	else
	{
		Peer->SendState = 0;
	}
	/*
	if (Peer->SendQueue.Count == 0)
	{
		Peer->SendState = 0;
	}
	else
	{
		int Pointer = 0;
		Peer->SendState = 1;
		Node = Peer->SendQueue.Root;
		//FIXME: DO NOT MERGE PACKET  TO REDUSE LAG
		while (Node != NULL)
		{
			if (!MERGE_PACKET && Pointer > 0) break;
			if (Pointer + Node->Len <= SendBufferSize)
			{
				Next = (IOCPQueueNode*)Node->Next;
				memcpy(&Peer->SendBuffer.buf[Pointer], Node->Data, Node->Len);
				Pointer += Node->Len;
				free(Node);
				Node = Next;
				Peer->SendQueue.Count --;
				Peer->SendQueue.Root = Node;
			}
			else
			{
				if (SendBufferSize - Pointer <= 0)
				{
					break;
				}
				NewNode = (IOCPQueueNode*)malloc(sizeof(IOCPQueueNode));
				NewNode->Len = Node->Len - (SendBufferSize - Pointer);
				NewNode->Data = (char*)malloc(NewNode->Len);
				NewNode->Next = Node->Next;
				memcpy(&Peer->SendBuffer.buf[Pointer], Node->Data, SendBufferSize - Pointer);
				memcpy(NewNode->Data, &Node->Data[SendBufferSize - Pointer], NewNode->Len);
				free(Node);
				Pointer = SendBufferSize;
				Peer->SendQueue.Root = NewNode;
				Node = NULL;
			}
		}
		Peer->SendBuffer.len = Pointer;
		memset(&Peer->SendEvent, 0, sizeof(OVERLAPPED));
		WSASend(Peer->Peer, &Peer->SendBuffer, 1, &dwBytes, dwFlags, (LPWSAOVERLAPPED)&(Peer->SendEvent), NULL);
	}
	*/
}

void PushData(IOCPPeer *Peer, char* Data, int Len)
{
	//IOCPQueueNode *NewNode;
	EnterCriticalSection(&Peer->QueueLock);
	if (Len + Peer->QueuePos > SendBufferSize)
	{
		printf("****** NO ENOUGH BUFFER FOR PushData ******\n");
		return -1;
	}
	memcpy(&Peer->SendQueue.buf[Peer->QueuePos], Data, Len);
	Peer->QueuePos += Len;
	/*
	NewNode = (IOCPQueueNode*)malloc(sizeof(IOCPQueueNode));
	NewNode->Len = Len;
	NewNode->Data = (char*)malloc(Len);
	NewNode->Next = NULL;
	memcpy(NewNode->Data, Data, Len);
	Peer->SendQueue.Count ++;
	if (Peer->SendQueue.Root != NULL)
	{
		IOCPQueueNode *Node = Peer->SendQueue.Root;
		while (Node->Next != NULL)
		{
			Node = (IOCPQueueNode*)Node->Next;
		}
		Node->Next = NewNode;
	}
	else
	{
		Peer->SendQueue.Root = NewNode;
	}
	*/
	LeaveCriticalSection(&Peer->QueueLock);
	if (Peer->SendState == 0)
	{
		SendQueueData(Peer);
	}
}

DWORD WINAPI ThreadClientLoop(LPVOID lpParam)
{
	int ConcurrentId = (int)lpParam;
	SOCKET s;
	void* IOKey;
	IOCPClient *Client;
	IOCPOverlapped *Overlapped;
	DWORD RecvLen = 0;
	DWORD Flags = 0;
	int ret = 0;
	int i = 0;
	int DispatchRet = 0;
	int LeftLen = 0;

	while (TRUE)
	{
		ret = GetQueuedCompletionStatus(PortClientArray[ConcurrentId], &RecvLen, (PULONG_PTR)&Client, (LPOVERLAPPED*)&Overlapped, INFINITE);
		Client = &ClientPool[Overlapped->Id];
		if (ret == 0 || (RecvLen <= 0 && Overlapped->Type < 2))
		{
			//printf("Connection closed by client on pool %d.\n", Client->Id);
			CloseClientByFlag(Client);
			continue;
		}
		if ((Client->Flag & 0x40) == 0x40)
		{
			//Oh nonono, I can't handle dead client
			//printf("Client peer of pool %d: Tried to handle a dead client.\n", Client->Id);
			continue;
		}
		//printf("Client peer of pool %d: Type %d, Bytes %d.\n", Client->Id, Overlapped->Type, RecvLen);

		if (Overlapped->Type == 0)
		{
			//Recv
			memset(&Client->Client.RecvEvent, 0, sizeof(OVERLAPPED));
			DispatchRet = Dispatch(Client, Client->Client.RecvBuffer.buf, RecvLen + Client->Client.RecvOffset);
			if (DispatchRet < 0)
			{
				CloseClientByFlag(Client);
				continue;
			}
			if (DispatchRet < RecvLen + Client->Client.RecvOffset)
			{
				LeftLen = (RecvLen + Client->Client.RecvOffset - DispatchRet);
				memcpy(Client->Client.RecvBuffer.buf, &Client->Client.RecvBuffer.buf[DispatchRet], LeftLen); 
				Client->Client.TempRecvBuffer.len = Client->Client.RecvBuffer.len - LeftLen;
				Client->Client.TempRecvBuffer.buf = Client->Client.RecvBuffer.buf + LeftLen;
				Client->Client.RecvOffset = LeftLen;
				WSARecv(Client->Client.Peer, &Client->Client.TempRecvBuffer, 1, &RecvLen, &Flags, (LPWSAOVERLAPPED)&(Client->Client.RecvEvent), NULL);
			}
			else
			{
				Client->Client.RecvOffset = 0;
				WSARecv(Client->Client.Peer, &Client->Client.RecvBuffer, 1, &RecvLen, &Flags, (LPWSAOVERLAPPED)&(Client->Client.RecvEvent), NULL);
			}
		}
		else if (Overlapped->Type == 1)
		{
			//Send
			SendQueueData(&Client->Client);
		}
		else if (Overlapped->Type == 2)
		{
			//UNUSED
			//Disconnect
			//Flag: Bits ABCD stands for Closing Client, Closed Client, Closing Uplink, Closed Uplink
			Client->Flag = Client->Flag | 0x0C;
			CloseClientByFlag(Client);
		}
	}
	return 0;
}

DWORD WINAPI ThreadUplinkLoop(LPVOID lpParam)
{
	int ConcurrentId = (int)lpParam;
	SOCKET s;
	void* IOKey;
	IOCPClient *Client;
	IOCPOverlapped *Overlapped;
	DWORD RecvLen = 0;
	DWORD Flags = 0;
	int ret = 0;
	int i = 0;
	int DispatchRet = 0;
	int LeftLen = 0;

	while (TRUE)
	{
		ret = GetQueuedCompletionStatus(PortUplinkArray[ConcurrentId], &RecvLen, (PULONG_PTR)&s, (LPOVERLAPPED*)&Overlapped, INFINITE);
		Client = &ClientPool[Overlapped->Id];
		if (ret == 0 || (RecvLen <= 0 && Overlapped->Type < 2))
		{
			//printf("Connection closed by uplink on pool %d.\n", Client->Id);
			CloseClientByFlag(Client);
			continue;
		}
		if ((Client->Flag & 0x40) == 0x40)
		{
			//Oh nonono, I can't handle dead client
			//printf("Uplink peer of pool %d: Tried to handle a dead client.\n", Client->Id);
			continue;
		}
		//printf("Uplink peer of pool %d: Type %d, Bytes %d.\n", Client->Id, Overlapped->Type, RecvLen);
		if (Overlapped->Type == 0)
		{
			//Recv
			memset(&Client->Uplink.RecvEvent, 0, sizeof(OVERLAPPED));
			DispatchCallback(Client, Client->Uplink.RecvBuffer.buf, RecvLen);
			WSARecv(Client->Uplink.Peer, &Client->Uplink.RecvBuffer, 1, &RecvLen, &Flags, (LPWSAOVERLAPPED)&(Client->Uplink.RecvEvent), NULL);
		}
		else if (Overlapped->Type == 1)
		{
			//Send
			SendQueueData(&Client->Uplink);
		}
		else if (Overlapped->Type == 2)
		{
			//UNUSED
			//Disconnect
			//Flag: Bits ABCD stands for Closing Client, Closed Client, Closing Uplink, Closed Uplink
			//printf("TCP_TIMEWAIT finished on pool %d\n", Client->Id);
			Client->Flag = Client->Flag | 0x03;
			CloseClientByFlag(Client);
		}
		else if (Overlapped->Type == 3)
		{
			//Connect
			/*if (Client->Uplink.SendBuffer.len > 0)
			{
				int dwBytes = 0;
				ret = WSASend(Client->Uplink.Peer, &Client->Uplink.SendBuffer, 1, &dwBytes, 0, NULL, NULL);
			}*/
			SendQueueData(&Client->Uplink);
			memset(&Client->Uplink.RecvEvent, 0, sizeof(OVERLAPPED));
			ret = WSARecv(Client->Uplink.Peer, &Client->Uplink.RecvBuffer, 1, &RecvLen, &Flags, (LPWSAOVERLAPPED)&(Client->Uplink.RecvEvent), NULL);
			if (ret == FALSE && WSAGetLastError() != 997)
			{
				CloseClientByFlag(Client);
			}
		}
	}
	return 0;
}

DWORD WINAPI ThreadKillLoop(LPVOID lpParam)
{
	int i;
	while (TRUE)
	{
		Sleep(1000);
		//D2GSEventLog("IOCPPool", "I'm going to loop");
		for (i = 0; i < MaxClient; i ++)
		{
			if ((ClientPool[i].Flag & 0x10) == 0x10 && (ClientPool[i].Flag & 0x40) == 0)
			{
				if (ClientPool[i].RequestRate > 6)
				{
					D2GSEventLog("RateLimit", "Rate exceeded on pool %d, rate %d, addr %d", ClientPool[i].Id, ClientPool[i].RequestRate, ClientPool[i].RemoteAddr);
					CloseClientByFlag(&ClientPool[i]);
				}
				InterlockedExchange(&ClientPool[i].RequestRate, 0);
			}
			if (ClientPool[i].Flag == 0 && time(NULL) - ClientPool[i].ConnTime > 5)
			{
				D2GSEventLog("IOCPPool", "Kill idle peer on pool %d", ClientPool[i].Id);
				CloseClientByFlag(&ClientPool[i]);
			}
			if ((ClientPool[i].Flag & 0xC0) == 0xC0 && time(NULL) - ClientPool[i].ConnTime > 61)
			{
				D2GSEventLog("IOCPPool", "Okay to reuse pool %d", ClientPool[i].Id);
				EnterCriticalSection(&ClientPool[i].Lock);
				ClientPool[i].Flag = 0x40;
				EnterCriticalSection(&ReuseStack.Lock);
				ReuseStack.Buffer[--ReuseStack.Bottom] = ClientPool[i].Id;
				ReuseStack.Count ++;
				LeaveCriticalSection(&ReuseStack.Lock);
				LeaveCriticalSection(&ClientPool[i].Lock);
			}
		}
	}
	return 0;
}

SOCKET InitalizeSocket(HANDLE IOCPPort, ULONG_PTR Key)
{
	char NoDelay = 1;
	int StackSendBuf = 0;
	struct linger s_linger = {1, 0};
	SOCKET Peer;
	Peer = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	setsockopt(Peer, IPPROTO_TCP, TCP_NODELAY, &NoDelay, sizeof(char));
	setsockopt(Peer, SOL_SOCKET, SO_LINGER, &s_linger, sizeof(struct linger));
	setsockopt(Peer, SOL_SOCKET, SO_SNDBUF, &StackSendBuf, sizeof(int));
	if (IOCPPort != NULL) CreateIoCompletionPort((HANDLE)Peer, IOCPPort, Key, ConcurrentNum);
	return Peer;
}

void InitalizePeer(IOCPPeer *Peer, int Id, HANDLE IOCPPort, ULONG_PTR Key, BOOL InitSocket)
{
	Peer->RecvEvent.Id = Id;
	Peer->RecvEvent.Type = 0;
	Peer->SendEvent.Id = Id;
	Peer->SendEvent.Type = 1;
	Peer->DisconnectEvent.Id = Id;
	Peer->DisconnectEvent.Type = 2;
	Peer->ConnectEvent.Id = Id;
	Peer->ConnectEvent.Type = 3;
	Peer->RecvBuffer.len = RecvBufferSize;
	Peer->RecvBuffer.buf = (char*)VirtualAlloc(NULL, RecvBufferSize, MEM_COMMIT, PAGE_READWRITE);
	Peer->SendBuffer.len = SendBufferSize;
	Peer->SendBuffer.buf = (char*)VirtualAlloc(NULL, SendBufferSize, MEM_COMMIT, PAGE_READWRITE);
	Peer->SendQueue.len = SendBufferSize;
	Peer->SendQueue.buf = (char*)VirtualAlloc(NULL, SendBufferSize, MEM_COMMIT, PAGE_READWRITE);
	InitializeCriticalSection(&Peer->QueueLock);
	if (InitSocket) Peer->Peer = InitalizeSocket(IOCPPort, Key);
}

void InitalizeHashTable(Hashtable *Hash)
{
	//Fill with -1
	Hash->Keys = (int*)VirtualAlloc(NULL, sizeof(int) * HashTableSize, MEM_COMMIT, PAGE_READWRITE);
	memset(Hash->Keys, 0xFF, sizeof(int) * HashTableSize);

	//Fill with 0
	Hash->Values = (int*)VirtualAlloc(NULL, sizeof(int) * HashTableSize, MEM_COMMIT, PAGE_READWRITE);
	memset(Hash->Values, 0, sizeof(int) * HashTableSize);

	InitializeCriticalSection(&Hash->Lock);
}

int HashIndex(Hashtable *Hash, unsigned int Key)
{
	unsigned int Index = Key % HashTableSize;
	while (Hash->Keys[Index] > -1 && Hash->Keys[Index] != Key)
	{
		Index = (Index + 37) % HashTableSize;
	}
	Hash->Keys[Index] == Key;
	return Index;
}

void HashSet(Hashtable *Hash, unsigned int Key, int Value)
{
	int Index;
	EnterCriticalSection(&Hash->Lock);
	Index = HashIndex(Hash, Key);
	Hash->Values[Index] = Value;
	LeaveCriticalSection(&Hash->Lock);
}

int HashGet(Hashtable *Hash, unsigned int Key)
{
	int Index, Value;
	EnterCriticalSection(&Hash->Lock);
	Index = HashIndex(Hash, Key);
	Value = Hash->Values[Index];
	LeaveCriticalSection(&Hash->Lock);
	return Value;
}

int HashIncrement(Hashtable *Hash, unsigned int Key, int Step)
{
	int Index, Value;
	EnterCriticalSection(&Hash->Lock);
	Index = HashIndex(Hash, Key);
	Hash->Values[Index] = Hash->Values[Index] + Step;
	Value = Hash->Values[Index];
	LeaveCriticalSection(&Hash->Lock);
	return Value;
}

int StartupTrunk()
{
	WSADATA data;
	SOCKADDR_IN addr;
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	GUID guidDisconnectEx = WSAID_DISCONNECTEX;
	GUID guidConnectEx = WSAID_CONNECTEX;
	DWORD dwBytes = 0;
	int i = 0;
	int ReuseAddr = 1;
	struct linger s_linger = {1, 0};

	AFPacket.type = 0xAF;
	AFPacket.suffix = 0x01;

	InitalizeHashTable(&ConnectRate);
	memset(&bind_local_addr, 0, sizeof(struct sockaddr_in));
	bind_local_addr.sin_family = AF_INET;
	bind_local_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind_local_addr.sin_port = 0;

	ClientPool = (IOCPClient*)VirtualAlloc(NULL, sizeof(IOCPClient) * MaxClient, MEM_COMMIT, PAGE_READWRITE);

	WSAStartup(MAKEWORD(2,2), &data);
	PortListen = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, ConcurrentNum);

	for (i = 0; i < ConcurrentNum; i ++)
	{
		PortClientArray[i] = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, ConcurrentNum);
		PortUplinkArray[i] = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, ConcurrentNum);
	}

	Listener = WSASocket(AF_INET,SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	CreateIoCompletionPort((HANDLE)Listener, PortListen, NULL, 0);

	ReuseStack.Bottom = 0;
	ReuseStack.Count = MaxClient;
	InitializeCriticalSection(&ReuseStack.Lock);

	for (i = 0; i < MaxClient; i ++)
	{
		InitializeCriticalSection(&ClientPool[i].Lock);
		ReuseStack.Buffer[i] = i;
		ClientPool[i].Id = i;
		ClientPool[i].Flag = 0x40;
		InitalizePeer(&ClientPool[i].Client, i, PortClientArray[i % ConcurrentNum], (ULONG_PTR)&ClientPool[i], FALSE);
		InitalizePeer(&ClientPool[i].Uplink, i, PortUplinkArray[i % ConcurrentNum], (ULONG_PTR)&ClientPool[i], FALSE);
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(d2gsconf.listenaddr);
	addr.sin_port = htons(d2gsconf.listenport);
	setsockopt(Listener, SOL_SOCKET, SO_LINGER, &s_linger, sizeof(struct linger));

	bind(Listener, (PSOCKADDR)&addr, sizeof(addr));
	listen(Listener, 64);

	WSAIoctl(Listener, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(guidAcceptEx), &lpAcceptEx, sizeof(lpAcceptEx), &dwBytes, NULL, NULL);
	WSAIoctl(Listener, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidDisconnectEx, sizeof(guidDisconnectEx), &lpDisconnectEx, sizeof(lpDisconnectEx), &dwBytes, NULL, NULL);
	WSAIoctl(Listener, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidConnectEx, sizeof(guidConnectEx), &lpConnectEx, sizeof(lpConnectEx), &dwBytes, NULL, NULL);

	for (i = 0; i < ConcurrentNum; i ++)
	{
		ThreadListen[i] = CreateThread(NULL, 0, ThreadListenLoop, (LPVOID)i, 0, NULL);
		ThreadClient[i] = CreateThread(NULL, 0, ThreadClientLoop, (LPVOID)i, 0, NULL);
		ThreadUplink[i] = CreateThread(NULL, 0, ThreadUplinkLoop, (LPVOID)i, 0, NULL);
	}

	ThreadKill = CreateThread(NULL, 0, ThreadKillLoop, NULL, 0, NULL);

	return 1;
}
