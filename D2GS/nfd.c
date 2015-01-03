#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "nfd.h"

//#define NFD_LEVEL_INACTIVE
#define NFD_LEVEL_ACTIVE
#define NFD_LEVEL_ENFORCE
#define NFD_LEVEL_STRICT

int nfd_filter(char const* data, int start, int end)
{
#ifdef NFD_LEVEL_INACTIVE
	return 0;
#endif
	int i;
	if (start < 0) start = 0;
	for (i = start; i <= end; i ++)
	{
		int dt = *((int*)&data[i]);
#ifdef NFD_LEVEL_ACTIVE
		if (dt == 0x00104D4A)
		{
			return -1;
		}
		if (dt == 0x20104D4A)
		{
			return -1;
		}
		if (dt == 0x00074D4A)
		{
			return -1;
		}
		if (dt == 0x08104D4A)
		{
			return -1;
		}
		if (dt == 0xAA55AA55)
		{
			return -1;
		}
#endif
#ifdef NFD_LEVEL_ENFORCE
		if (dt == 0x02033327 || dt == 0x02133327 || dt == 0x02233327 || dt == 0x02333327 || dt == 0x02432327 || dt == 0x02532327 || dt == 0x02632327 || dt == 0x02732327 || dt == 0x02832327 || dt == 0x02932327)
		{
			return -1;
		}
#endif
#ifdef NFD_LEVEL_STRICT
		if (dt == 0x00343272 || dt == 0x00353272 || dt == 0x00363272 || dt == 0x00373272 || dt == 0x00383272 || dt == 0x00393272 || dt == 0x00303372 || dt == 0x00313372 || dt == 0x00323372 || dt == 0x00333372)
		{
			return -1;
		}
		if (dt == 0x72323400 || dt == 0x72323500 || dt == 0x72323600 || dt == 0x72323700 || dt == 0x72323800 || dt == 0x72323900 || dt == 0x72333000 || dt == 0x72333100 || dt == 0x72333200 || dt == 0x72333300)
		{
			return -1;
		}
#endif
	}
	return 0;
}