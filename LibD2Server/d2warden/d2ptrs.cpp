#include <windows.h>

DWORD GetDllOffset(char *dll, int offset)
{
	HMODULE hmod = GetModuleHandle(dll);
	if (!hmod)
		hmod = LoadLibrary(dll);
	if (!hmod) return 0;
	if (offset < 0) {
		return (DWORD)GetProcAddress(hmod, (LPCSTR)-offset);
	}
	return ((DWORD)hmod)+offset;
}

DWORD GetDllOffset(int num)
{
	static char *dlls[] = {"D2CLIENT.DLL", "D2COMMON.DLL", "D2GFX.DLL", "D2WIN.DLL", "D2LANG.DLL", "D2CMP.DLL", "D2MULTI.DLL", "BNCLIENT.DLL", "D2NET.DLL", "STORM.DLL", "FOG.DLL", "D2GAME.DLL"};
	return GetDllOffset(dlls[num&0xff], num>>8);
}

BOOL RelocD2Ptrs(DWORD* pPtrsStart, DWORD* pPtrsEnd)
{
	DWORD *p = pPtrsStart, ret = TRUE;
	do {
		if (!(*p = GetDllOffset(*p))) ret = FALSE;
	} while (++p <= pPtrsEnd);
	return ret;
}
