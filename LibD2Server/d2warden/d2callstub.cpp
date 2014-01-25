#include <windows.h>
#include "d2ptrs.h"

DWORD __declspec(naked) __fastcall D2GAME_GetClient_STUB(DWORD ClientID)
{
	__asm {
		push esi
		mov esi, ecx
		call D2GAME_GetClient_I
		pop esi
		ret
	}
}


DWORD __declspec(naked) __fastcall D2GAME_LeaveCriticalSection(DWORD unk)
{
	__asm {
		push eax
		mov eax, ecx
		call D2GAME_LeaveCriticalSection_I
		pop eax
		ret
	}
}

DWORD __declspec(naked) __fastcall D2GAME_Send0XAEPacket_STUB(void *ptPlayer,DWORD Length,DWORD *Packet)
{
	__asm {
		mov esi, ecx
		mov eax, edx
		push Packet
		call D2GAME_Send0XAEPacket_I
		ret
	}
}


