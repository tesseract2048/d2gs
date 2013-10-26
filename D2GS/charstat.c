#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "charstat.h"

unsigned int charstat_getbit(const char* pos, const int start, const int len){
	int result = 0;
	int i;
	for(i = start; i < start + len; i ++){
		unsigned char* cur = (unsigned char*)pos + (i / 8);
		result = (result << 1);
		if((*cur & (1 << (i % 8))) != 0)
			result ++;
	}
	return result;
}

unsigned int charstat_to_be(const unsigned int len, const unsigned int le){
	unsigned int i;
	unsigned int result = 0;
	for(i = 0; i < len; i ++){
		result = (result << 1);
		if((le & (1 << i)) != 0)
			result ++;
	}
	return result;
}

unsigned int get_d2_int(const char* data, int* pos, const int len){
	int val = charstat_to_be(len, charstat_getbit(data, *pos, len));
	*pos += len;
	return val;
}

struct t_charstat get_charstat(char* data, unsigned int datalen){
	struct t_charstat r;
	char* buf = data + 0x2FF  +2;
	int pos = 0;
	memset(&r, 0, sizeof(struct t_charstat)); 
	r.charclass = data[0x28 + 2];
	while(pos < 54*8){
		int statid = get_d2_int(buf, &pos, 9);
		switch(statid){
			case 0:
				r.str = get_d2_int(buf, &pos, 10);
				break;
			case 1:
				r.ene = get_d2_int(buf, &pos, 10);
				break;
			case 2:
				r.dex = get_d2_int(buf, &pos, 10);
				break;
			case 3:
				r.vit = get_d2_int(buf, &pos, 10);
				break;
			case 4:
				r.statpoint = get_d2_int(buf, &pos, 10);
				break;
			case 5:
				r.skillpoint = get_d2_int(buf, &pos, 8);
				break;
			case 6:
				r.currlife = get_d2_int(buf, &pos, 21);
				r.currlife /= 256;
				break;
			case 7:
				r.maxlife = get_d2_int(buf, &pos, 21);
				r.maxlife /= 256;
				break;
			case 8:
				r.currmana = get_d2_int(buf, &pos, 21);
				r.currmana /= 256;
				break;
			case 9:
				r.maxmana = get_d2_int(buf, &pos, 21);
				r.maxmana /= 256;
				break;
			case 10:
				r.currsta = get_d2_int(buf, &pos, 21);
				r.currsta /= 256;
				break;
			case 11:
				r.maxsta = get_d2_int(buf, &pos, 21);
				r.maxsta /= 256;
				break;
			case 12:
				r.level = get_d2_int(buf, &pos, 7);
				break;
			case 13:
				r.experience = get_d2_int(buf, &pos, 32);
				break;
			case 14:
				r.gold1 = get_d2_int(buf, &pos, 25);
				break;
			case 15:
				r.gold2 = get_d2_int(buf, &pos, 25);
				break;
			default:
				goto ok;
				break;
		}
	}
ok:
	return r;
}

