#ifndef INCLUDED_CHARSTAT_H
#define INCLUDED_CHARSTAT_H
struct t_charstat{
	unsigned int charclass;
	unsigned int str;
	unsigned int ene;
	unsigned int dex;
	unsigned int vit;
	unsigned int statpoint;
	unsigned int skillpoint;
	double currlife;
	double maxlife;
	double currmana;
	double maxmana;
	double currsta;
	double maxsta;
	unsigned int level;
	unsigned int experience;
	unsigned int gold1;
	unsigned int gold2;
	unsigned int skill[30];
};
struct t_charstat get_charstat(void* data, unsigned int datalen);

#endif