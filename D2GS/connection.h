#ifndef INCLUDED_CONNECTION_H
#define INCLUDED_CONNECTION_H


/* for identifying by bnetd server, send just after connected */
typedef struct
{
	bn_byte		bnclass;
} t_d2gs_connect;
#define CONNECT_CLASS_D2GS_TO_D2CS		0x91
#define CONNECT_CLASS_D2GS_TO_D2DBS		0x88


#endif /* INCLUDED_CONNECTION_H */