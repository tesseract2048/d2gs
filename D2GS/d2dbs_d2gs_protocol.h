#ifndef INCLUDED_D2DBS_D2GS_PROTOCOL_H
#define INCLUDED_D2DBS_D2GS_PROTOCOL_H


#include "bn_types.h"
#include "itemscan.h"
#pragma pack(push, pack02, 1)

/* packet */
typedef struct {
	bn_short	size;
	bn_byte		type;
	bn_int		seqno;			/* seqno, set by the sender */
} t_d2dbs_d2gs_header;

typedef struct {
	t_d2dbs_d2gs_header	h;
} t_d2dbs_d2gs_generic;

#define D2GS_D2DBS_SAVE_CHARSAVE_REQUEST		0x30
typedef struct { 
  t_d2dbs_d2gs_header  h; 
  bn_short      datalen;
  bn_short		ist;
  bn_short		changedist;
  /* AccountName */
  /* CharName */
  /* GameName(if charsave) */
  /* IPAddr(if charsave) */
  /* ItemRecord(if charsave) */
  /* data */
} t_d2gs_d2dbs_save_charsave_request; 

#define D2GS_D2DBS_SAVE_CHARINFO_REQUEST		0x40
typedef struct { 
  t_d2dbs_d2gs_header  h; 
  bn_short      datalen;
  /* AccountName */
  /* CharName */
  /* data */
} t_d2gs_d2dbs_save_charinfo_request; 

#define D2GS_DATA_CHARSAVE    0x01
#define D2GS_DATA_PORTRAIT    0x02 

#define D2DBS_D2GS_SAVE_DATA_REPLY			0x30
typedef struct {
	t_d2dbs_d2gs_header	h;
	bn_byte				result;
	bn_byte			datatype;
	/* CharName */
} t_d2dbs_d2gs_save_data_reply;
#define D2DBS_SAVE_DATA_SUCCESS		0
#define D2DBS_SAVE_DATA_FAILED		1


#define D2GS_D2DBS_GET_DATA_REQUEST		0x31
typedef struct { 
  t_d2dbs_d2gs_header  h; 
  bn_byte      datatype;
  /* AccountName */
  /* CharName */
} t_d2gs_d2dbs_get_data_request; 


#define D2DBS_D2GS_GET_DATA_REPLY		0x31
typedef struct { 
  t_d2dbs_d2gs_header  h; 
  bn_byte        result; 
  bn_int    charcreatetime;
  bn_byte    allowladder;
  bn_byte      datatype;
  bn_short      datalen; 
  /* CharName */
  /* data */
} t_d2dbs_d2gs_get_data_reply; 
#define D2DBS_GET_DATA_SUCCESS		0
#define D2DBS_GET_DATA_FAILED		1
#define D2DBS_GET_DATA_CHARLOCKED	2


#define D2GS_D2DBS_UPDATE_LADDER		0x32
typedef struct { 
 t_d2dbs_d2gs_header h; 
 bn_byte  charlevel; 
 bn_int  charexplow; 
 bn_int  charexphigh; 
 bn_byte charclass; 
 bn_short charstatus; 
 /* CharName */
} t_d2gs_d2dbs_update_ladder; 


#define D2GS_D2DBS_CHAR_LOCK		0x33
typedef struct { 
 t_d2dbs_d2gs_header h; 
 bn_byte  lockstatus; 
 /* CharName */
} t_d2gs_d2dbs_char_lock; 


#define D2DBS_D2GS_ECHOREQUEST		0x34
typedef struct {
	t_d2dbs_d2gs_header	h;
} t_d2dbs_d2gs_echoreq;


#define D2GS_D2DBS_ECHOREPLY		0x34
typedef struct {
	t_d2dbs_d2gs_header	h;
} t_d2gs_d2dbs_echoreply;

#define D2GS_D2DBS_CLOSESIGNAL		0x37
typedef struct {
	t_d2dbs_d2gs_header	h;
	bn_byte ifclose;
    /* Fixed GameName */
} t_d2gs_d2dbs_closesignal;

#define D2DBS_D2GS_KICKREQUEST		0x38
typedef struct {
	t_d2dbs_d2gs_header	h;
	/* CharName */
} t_d2dbs_d2gs_kickrequest;

#define D2GS_D2DBS_KICKREPLY		0x39
typedef struct {
	t_d2dbs_d2gs_header	h;
	bn_byte result;
	/* CharName */
} t_d2gs_d2dbs_kickreply;

#define D2GS_D2DBS_KICK_SUCCESS		0x01

#define D2GS_D2DBS_PERFORMANCE_COUNTER		0x91
typedef struct {
	t_d2dbs_d2gs_header	h;
	bn_int busy_pool;
	bn_int idle_pool;
    bn_int thread_num;
    bn_int handle_num;
    bn_int ge_num;
} t_d2gs_d2dbs_performance_counter;

#define D2GS_D2DBS_RATE_EXCEEDED		0x92
typedef struct {
	t_d2dbs_d2gs_header	h;
	bn_int request_rate;
	/* AccountName */
    /* CharName */
    /* IPAddr */
} t_d2gs_d2dbs_rate_exceeded;

#define D2GS_D2DBS_EMERGENCY		0x93
typedef struct {
	t_d2dbs_d2gs_header	h;
	bn_int emergency_type;
	bn_int param;
} t_d2gs_d2dbs_emergency;

#define D2GS_D2DBS_SOJ_COUNTER_UPDATE		0x96
typedef struct {
	t_d2dbs_d2gs_header	h;
	bn_int increment;
	/* RealmName */
} t_d2gs_d2dbs_soj_counter_update;

#define D2DBS_D2GS_SOJ_COUNTER_UPDATE		0x97
typedef struct {
	t_d2dbs_d2gs_header	h;
	bn_int soj_counter;
} t_d2dbs_d2gs_soj_counter_update;

#define D2DBS_D2GS_DC_TRIGGER		0x98
typedef struct {
	t_d2dbs_d2gs_header	h;
} t_d2dbs_d2gs_dc_trigger;

#pragma pack(pop, pack02)


#endif /* INCLUDED_D2DBS_D2GS_PROTOCOL_H */