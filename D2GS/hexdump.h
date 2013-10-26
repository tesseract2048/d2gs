#ifndef INCLUDED_HEXDUMP_H
#define INCLUDED_HEXDUMP_H

#include <stdio.h>

extern FILE * hexstrm;
extern void hexdump(void const * data, unsigned int len);

#endif
