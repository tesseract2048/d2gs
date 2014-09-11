#ifndef INCLUDED_CLIENT_H
#define INCLUDED_CLIENT_H

#include <winsock2.h>

void init_client();

void push_client(SOCKET s);

int wait_client(DWORD timeout);

SOCKET get_client();

void destroy_client();

#endif