#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "d2gs.h"
#include "config.h"
#include "eventlog.h"
#include "vars.h"
#include "itemscan.h"

const int rwcode[] = {0x5025, 0x5027, 0x5028, 0x502e, 0x5034, 0x503b, 0x503f, 0x5040, 0x5043, 0x5049, 0x504d, 0x5056, 0x505f, 0x5075, 0x5080, 0x509b};
const char* rwname[] = {"BotD", "CTA", "CoH", "Death", "Doom", "TP+", "Exile", "Faith", "Fortitude", "Grief", "Oak", "Infinity", "LastWith", "Obedience", "Phoenix", "Spirit"};

struct t_item_info{
	int equip_id;
	char* name;
	int itemtype;
	int quality_attr1;
	int quality_attr2;
};

const int set_item_count = 2;
const struct t_item_info set_table[] = {
	{0, "7+", 0, 0x004f, 0},
	{1, "TLX", 0, 0x004d, 0}
};

const int rare_item_count = 2;
const struct t_item_info rare_table[] = {
	{2, "RAM", 0x20756d61, 0, 0},
	{3, "RI", 0x206e6972, 0, 0}
};

const int unique_item_count = 22;
const struct t_item_info unique_table[] = {
	{4, "SOJ", 0, 0x007a, 0},
	{5, "UR", 0x206e6972, 0, 0},
	{6, "JEW", 0x2077656a, 0, 0},
	{7, "USC", 0x20316d63, 0, 0},
	{8, "ULC", 0x20326d63, 0, 0},
	{9, "UGC", 0x20336d63, 0, 0},
	{10, "HSP", 0, 0x00d2, 0},
	{11, "JNYD", 0, 0x0175, 0},
	{12, "SB", 0, 0x0171, 0},
	{13, "WT", 0, 0x00f0, 0},
	{14, "NJ", 0, 0x0158, 0},
	{15, "SKO", 0, 0x00f8, 0},
	{16, "AN", 0, 0x0159, 0},
	{17, "GLF", 0, 0x0150, 0},
	{18, "WF", 0, 0x010a, 0},
	{19, "TT", 0, 0x0119, 0},
	{20, "SZ", 0, 0x0146, 0},
	{21, "CY", 0, 0x0162, 0},
	{22, "GYZQ", 0, 0x016f, 0},
	{23, "ORB", 0, 0x011c, 0},
	{24, "PY", 0, 0x012b, 0},
	{25, "UAM", 0x20756d61, 0, 0}
};

const int magic_item_count = 4;
const struct t_item_info magic_table[] = {
	{26, "PXD", 0x20746975, 0x01a6, 0x00ad}, //PXD(uit)
	{27, "PXD", 0x20626170, 0x01a6, 0x00ad},  //PXD(pad)
	{28, "WEN+", 0, 0x01a6, 0x0108},
	{29, "X+", 0, 0x01a6, 0x0140}
};


unsigned int getbit(char* pos, int start, int len){
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

unsigned int to_be(unsigned int le){
	int i;
	unsigned int result = 0;
	for(i = 0; i < 32; i ++){
		result = (result << 1);
		if((le & (1 << i)) != 0)
			result ++;
	}
	return result;
}

struct t_scan_result scandata(char* buf, int len){
	struct t_scan_result result;
	memset(&result, 0, sizeof(struct t_scan_result));
	if(len > 130)
	{
		unsigned char *data = (unsigned char*)buf;
		unsigned long  size = len;

		unsigned int i = 0;
		unsigned int j = 0;

		unsigned int ist = 0;

		while(i < size)
		{
			if(data[i] == 'J' && data[i+1] == 'M')
			{
				unsigned int quest_item = getbit((char*)&data[i], 16, 1);
				unsigned int starter_item = getbit((char*)&data[i], 33, 1);
				unsigned int simple_item = getbit((char*)&data[i], 37, 1);
				unsigned int item_location = getbit((char*)&data[i], 58, 3);
				if(quest_item == 0 && starter_item == 0 && item_location != 3){
					unsigned int itemtype = to_be(getbit((char*)&data[i], 76, 32));
					if(simple_item == 0){ //scan equip & rw
						unsigned int quality = getbit((char*)&data[i], 150, 4);
						unsigned int has_rw = getbit((char*)&data[i], 42, 1);
						unsigned int guid = to_be(getbit((char*)&data[i], 111, 32));
						unsigned int graphic_info = getbit((char*)&data[i], 154, 1);
						unsigned int offset = 0;
						unsigned int class_info;
						if(graphic_info == 1) offset += 3;
						class_info = getbit((char*)&data[i], 155 + offset, 1);
						if(class_info == 1) offset += 11;
						if(has_rw == 1){ //scan rw
							unsigned int rune_word = 159 + offset;
							if(quality != 12){
								rune_word -= 3;
							}
							rune_word = getbit((char*)&data[i], rune_word, 16);
							rune_word = to_be(rune_word) >> 16;
							for(j = 0; j < RW_NUM; j++){
								if(rwcode[j] == rune_word){
									result.rw[j] ++;
									break;
								}
							}
						}else{ //scan equip
							if(quality == 10){	//set
								unsigned int set_id = to_be(getbit((char*)&data[i], 156 + offset, 12)) >> 20;
								for(j = 0; j < set_item_count; j ++){
									if((!set_table[j].quality_attr1 || set_id == set_table[j].quality_attr1) && (!set_table[j].itemtype || itemtype == set_table[j].itemtype)){
										result.equip[set_table[j].equip_id] ++;
										break;
									}
								}
							}
							if(quality == 6){	//rare
								unsigned int rare_id = to_be(getbit((char*)&data[i], 156 + offset, 12)) >> 20;
								for(j = 0; j < rare_item_count; j ++){
									if((!rare_table[j].quality_attr1 || rare_id == rare_table[j].quality_attr1) && (!rare_table[j].itemtype || itemtype == rare_table[j].itemtype)){
										result.equip[rare_table[j].equip_id] ++;
										break;
									}
								}
							}
							if(quality == 14){	//unique
								unsigned int unique_id = to_be(getbit((char*)&data[i], 156 + offset, 12)) >> 20;
								for(j = 0; j < unique_item_count; j ++){
									if((!unique_table[j].quality_attr1 || unique_id == unique_table[j].quality_attr1) && (!unique_table[j].itemtype || itemtype == unique_table[j].itemtype)){
										result.equip[unique_table[j].equip_id] ++;
										break;
									}
								}
							}
							if(quality == 2){	//magic
								unsigned int magic_prefix = to_be(getbit((char*)&data[i], 156 + offset, 11)) >> 21;
								unsigned int magic_suffex = to_be(getbit((char*)&data[i], 167 + offset, 11)) >> 21;
								for(j = 0; j < magic_item_count; j ++){
									if((!magic_table[j].quality_attr2 || magic_suffex == magic_table[j].quality_attr2) && (!magic_table[j].quality_attr1 || magic_prefix == magic_table[j].quality_attr1) && (!magic_table[j].itemtype || itemtype == magic_table[j].itemtype)){
										result.equip[magic_table[j].equip_id] ++;
										break;
									}
								}
							}
						}
					}else{ //scan rune
						unsigned int stored_in = getbit((char*)&data[i], 73, 3);
						int rune_id = -1;
						if(itemtype == 0x20343272) rune_id = 0;
						if(itemtype == 0x20353272) rune_id = 1;
						if(itemtype == 0x20363272) rune_id = 2;
						if(itemtype == 0x20373272) rune_id = 3;
						if(itemtype == 0x20383272) rune_id = 4;
						if(itemtype == 0x20393272) rune_id = 5;
						if(itemtype == 0x20303372) rune_id = 6;
						if(itemtype == 0x20313372) rune_id = 7;
						if(itemtype == 0x20323372) rune_id = 8;
						if(itemtype == 0x20333372) rune_id = 9;
						if(rune_id > -1){
							ist += (1 << rune_id);
							result.rune[rune_id] ++;
						}
					}
				}
			}
			i++;
		}
		result.ist  = ist;
	}
	return result;
}

void scan_tostring(char* buf, struct t_scan_result result){
	char* pos = buf;
	int i;
	buf[0] = 0;
	for(i = 0; i < RUNE_NUM; i ++){
		if(result.rune[i] != 0)
			pos += sprintf(pos, "%d#(%d) ", i + 24, result.rune[i]);
	}
	for(i = 0; i < set_item_count; i ++){
		if(result.equip[set_table[i].equip_id] != 0)
			pos += sprintf(pos, "%s(%d) ", set_table[i].name, result.equip[set_table[i].equip_id]);
	}
	for(i = 0; i < unique_item_count; i ++){
		if(result.equip[unique_table[i].equip_id] != 0)
			pos += sprintf(pos, "%s(%d) ", unique_table[i].name, result.equip[unique_table[i].equip_id]);
	}
	for(i = 0; i < rare_item_count; i ++){
		if(result.equip[rare_table[i].equip_id] != 0)
			pos += sprintf(pos, "%s(%d) ", rare_table[i].name, result.equip[rare_table[i].equip_id]);
	}
	for(i = 0; i < magic_item_count; i ++){
		if(result.equip[magic_table[i].equip_id] != 0)
			pos += sprintf(pos, "%s(%d) ", magic_table[i].name, result.equip[magic_table[i].equip_id]);
	}
	for(i = 0; i < RW_NUM; i ++){
		if(result.rw[i] != 0)
			pos += sprintf(pos, "%s(%d) ", rwname[i], result.rw[i]);
	}
}