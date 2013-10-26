#ifndef INCLUDED_ITEMSCAN_H
#define INCLUDED_ITEMSCAN_H

#define RUNE_NUM 10
#define RW_NUM 16
#define EQUIP_NUM 30

struct t_scan_result{
	char rune[RUNE_NUM]; //24# - 33#
	char rw[RW_NUM];
	char equip[EQUIP_NUM];
	signed int ist;
};



struct t_scan_result scandata(char* buf, int len);
void scan_tostring(char* buf, struct t_scan_result result);

#endif