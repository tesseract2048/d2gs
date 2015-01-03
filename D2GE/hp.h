#ifndef INCLUDED_HP_H
#define INCLUDED_HP_H

#include <Windows.h>

void hp_init();
void hp_setup(SOCKET s);
void hp_handle_game_packet(int s, char* buf, int len);

#endif
