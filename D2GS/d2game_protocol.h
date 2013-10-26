#ifndef INCLUDED_D2GAME_PROTOCOL_H
#define INCLUDED_D2GAME_PROTOCOL_H

#include "bn_types.h"

#pragma pack(push, pack01, 1)

#define D2GAME_WALKTOLOCATION 0x01
typedef struct
{
	bn_short	x;
	bn_short	y;
} t_d2game_walktolocation;

#define D2GAME_WALKTOENTITY 0x02
typedef struct
{
	bn_int		type;
	bn_int		id;
} t_d2game_walktoentity;

#define D2GAME_RUNTOLOCATION 0x03
typedef struct
{
	bn_short	x;
	bn_short	y;
} t_d2game_runtolocation;

#define D2GAME_RUNTOENTITY 0x04
typedef struct
{
	bn_int		type;
	bn_int		id;
} t_d2game_runtoentity;

#define D2GAME_LEFTSKILLONLOCATION 0x05
typedef struct
{
	bn_short	x;
	bn_short	y;
} t_d2game_leftskillonlocation;

#define D2GAME_LEFTSKILLONENTITY 0x06
typedef struct
{
	bn_int		type;
	bn_int		id;
} t_d2game_leftskillonentity;

#define D2GAME_LEFTSKILLONENTITYEX 0x07
typedef struct
{
	bn_int		type;
	bn_int		id;
} t_d2game_leftskillonentityex;


#define D2GAME_LEFTSKILLONLOCATIONEX 0x08
typedef struct
{
	bn_short	x;
	bn_short	y;
} t_d2game_leftskillonlocationex;

#define D2GAME_LEFTSKILLONENTITYEX2 0x09
typedef struct
{
	bn_int		type;
	bn_int		id;
} t_d2game_leftskillonentityex2;

#define D2GAME_LEFTSKILLONENTITYEX3 0x0A
typedef struct
{
	bn_int		type;
	bn_int		id;
} t_d2game_leftskillonentityex3;

#define D2GAME_SKILLUNK1 0x0B

#define D2GAME_RIGHTSKILLONLOCATION 0x0C
typedef struct
{
	bn_short	x;
	bn_short	y;
} t_d2game_rightskillonlocation;

#define D2GAME_RIGHTSKILLONENTITY 0x0D
typedef struct
{
	bn_int		type;
	bn_int		id;
} t_d2game_rightskillonentity;

#define D2GAME_RIGHTSKILLONENTITYEX 0x0E
typedef struct
{
	bn_int		type;
	bn_int		id;
} t_d2game_rightskillonentityex;

#define D2GAME_RIGHTSKILLONLOCATIONEX 0x0F
typedef struct
{
	bn_short	x;
	bn_short	y;
} t_d2game_rightskillonlocationex;

#define D2GAME_RIGHTSKILLONENTITYEX2 0x10
typedef struct
{
	bn_int		type;
	bn_int		id;
} t_d2game_rightskillonentityex2;

#define D2GAME_RIGHTSKILLONENTITYEX3 0x11
typedef struct
{
	bn_int		type;
	bn_int		id;
} t_d2game_rightskillonentityex3;

#define D2GAME_SKILLUNK2 0x12

#define D2GAME_INTERACTWITHENTITY 0x13
typedef struct
{
	bn_int		type;
	bn_int		id;
} t_d2game_interactwithentity;

#define D2GAME_OVERHEADMESSAGE 0x14
//FIXME: String
typedef struct
{
	bn_short	unk1;
} t_d2game_overheadmessage;

#define D2GAME_CHATMESSAGE 0x15
//FIXME: String
typedef struct
{
	bn_byte		type;
	bn_byte		unk1;
} t_d2game_chatmessage;

#define D2GAME_PICKUPITEM 0x16
typedef struct
{
	bn_int		type;
	bn_int		unit;
	bn_int		action;
} t_d2game_pickupitem;

#define D2GAME_DROPITEM 0x17
typedef struct
{
	bn_int		id;
} t_d2game_dropitem;

#define D2GAME_ITEMTOBUFFER 0x18
typedef struct
{
	bn_int		id;
	bn_int		x;
	bn_int		y;
	bn_int		type;
} t_d2game_itemtobuffer;

#define D2GAME_PICKUPBUFFERITEM 0x19
typedef struct
{
	bn_int		id;
} t_d2game_pickupbufferitem;

#define D2GAME_ITEMTOBODY 0x1A
typedef struct
{
	bn_int		id;
	bn_int		location;
} t_d2game_itemtobody;

#define D2GAME_SWAP2HANDEDITEM 0x1B
typedef struct
{
	bn_int		id;
	bn_int		location;
} t_d2game_swap2handeditem;

#define D2GAME_PICKUPBODYITEM 0x1C
typedef struct
{
	bn_short	location;
} t_d2game_pickupbodyitem;

#define D2GAME_SWITCHBODYITEM 0x1D
typedef struct
{
	bn_int		id;
	bn_int		location;
} t_d2game_switchbodyitem;

#define D2GAME_SWITCHBODYITEM2 0x1E
typedef struct
{
	bn_int		id;
	bn_int		location;
} t_d2game_switchbodyitem2;

#define D2GAME_SWITCHINVENTORYITEM 0x1F
typedef struct
{
	bn_int		id_inventory;
	bn_int		id_replace;
	bn_int		x;
	bn_int		y;
} t_d2game_switchinventoryitem;

#define D2GAME_USEITEM 0x20
typedef struct
{
	bn_int		id;
	bn_int		x;
	bn_int		y;
} t_d2game_useitem;

#define D2GAME_STACKITEM 0x21
typedef struct
{
	bn_int		id_stack;
	bn_int		id_target;
} t_d2game_stackitem;

#define D2GAME_REMOVESTACKITEM 0x22
typedef struct
{
	bn_int		id;
} t_d2game_removestackitem;

#define D2GAME_ITEMTOBELT 0x23
typedef struct
{
	bn_int		id;
	bn_int		location;
} t_d2game_itemtobelt;

#define D2GAME_REMOVEBELTITEM 0x24
typedef struct
{
	bn_int		location;
} t_d2game_removebeltitem;

#define D2GAME_SWITCHBELTITEM 0x25
typedef struct
{
	bn_int		id_cursor;
	bn_int		id_replace;
} t_d2game_switchbeltitem;

#define D2GAME_USEBELTITEM 0x26
typedef struct
{
	bn_int		id;
	bn_int		unk1;
	bn_int		unk2;
} t_d2game_usebeltitem;

#define D2GAME_IDENTIFYITEM 0x27
typedef struct
{
	bn_int		id_1;
	bn_int		id_2;
} t_d2game_identifyitem;

#define D2GAME_INSERTSOCKETITEM 0x28
typedef struct
{
	bn_int		id_item;
	bn_int		id_socketed;
} t_d2game_insertsocketitem;

#define D2GAME_SCROLLTOTOME 0x29
typedef struct
{
	bn_int		id_scroll;
	bn_int		id_tome;
} t_d2game_scrolltotome;

#define D2GAME_ITEMTOCUBE 0x2A
typedef struct
{
	bn_int		id_item;
	bn_int		id_cube;
} t_d2game_itemtocube;

#define D2GAME_UNSELECTOBJ 0x2D

#define D2GAME_CHATUNK1 0x2E
typedef struct
{
	bn_short	unk1;
} t_d2game_chatunk1;

#define D2GAME_NPCINIT 0x2F
typedef struct
{
	bn_int		type;
	bn_int		id;
} t_d2game_npcinit;

#define D2GAME_NPCCANCEL 0x30
typedef struct
{
	bn_int		type;
	bn_int		id;
} t_d2game_npccancel;

#define D2GAME_QUESTMESSAGE 0x31
typedef struct
{
	bn_int		unk1;
	bn_int		unk2;
} t_d2game_questmessage;

#define D2GAME_NPCBUY 0x32
typedef struct
{
	bn_int		id_npc;
	bn_int		id_item;
	bn_int		type;
	bn_int		cost;
} t_d2game_npcbuy;

#define D2GAME_NPCSELL 0x33
typedef struct
{
	bn_int		id_npc;
	bn_int		id_item;
	bn_int		type;
	bn_int		cost;
} t_d2game_npcsell;

#define D2GAME_CAINIDENTIFY 0x34
typedef struct
{
	bn_int		unk1;
} t_d2game_cainidentify;

#define D2GAME_REPAIR 0x35
typedef struct
{
	bn_int		id1;
	bn_int		id2;
	bn_int		id3;
	bn_int		id4;
} t_d2game_repair;

#define D2GAME_HIRE 0x36
typedef struct
{
	bn_int		id1;
	bn_int		id2;
} t_d2game_hire;

#define D2GAME_GAMBLED 0x37
typedef struct
{
	bn_int		id;
} t_d2game_gambled;

#define D2GAME_NPCTRADE 0x38
typedef struct
{
	bn_int		type;
	bn_int		id_npc;
	bn_int		unk1;
} t_d2game_npctrade;

#define D2GAME_BUYHEALTH 0x39
typedef struct
{
	bn_int		type;
} t_d2game_buyhealth;

#define D2GAME_STATPOINT 0x3A
typedef struct
{
	bn_short		type;
} t_d2game_statpoint;

#define D2GAME_SKILLPOINT 0x3B
typedef struct
{
	bn_short		type;
} t_d2game_skillpoint;

#define D2GAME_SWITCHSKILL 0x3C
typedef struct
{
	bn_int		unk1;
	bn_int		unk2;
} t_d2game_switchskill;

#define D2GAME_CLOSEDOOR 0x3D
typedef struct
{
	bn_int		unk1;
} t_d2game_closedoor;

#define D2GAME_UPDATEITEMSTAT 0x3E
typedef struct
{
	bn_int		id;
} t_d2game_updateitemstat;

#define D2GAME_CHARACTERPHRASE 0x3F
typedef struct
{
	bn_short	id;
} t_d2game_characterphrase;

#define D2GAME_QUESTLOG 0x40

#define D2GAME_RESPAWN 0x41

#define D2GAME_UNK2 0x48

#define D2GAME_PUTSLOT 0x44
typedef struct
{
	bn_int		id1;
	bn_int		id2;
	bn_int		id3;
	bn_int		id4;
} t_d2game_putslot;

#define D2GAME_CHANGETP 0x45
typedef struct
{
	bn_int		id1;
	bn_short	id2;
	bn_short	id3;
} t_d2game_changetp;

#define D2GAME_MERCINTER 0x46
typedef struct
{
	bn_int		mercid;
	bn_int		unitid;
	bn_int		type;
} t_d2game_mercinter;

#define D2GAME_MOVEMERC 0x47
typedef struct
{
	bn_int		mercid;
	bn_int		x;
	bn_int		y;
} t_d2game_movemerc;

#define D2GAME_WAYPOINT 0x49
typedef struct
{
	bn_byte		id;
	bn_byte		unk1;
	bn_short	unk2;
	bn_byte		level;
	bn_short	unk3;
	bn_byte		unk4;
} t_d2game_waypoint;

#define D2GAME_REASSIGN 0x4B
typedef struct
{
	bn_int		id1;
	bn_int		id2;
} t_d2game_reassign;

#define D2GAME_DISAPPEARITEM 0x4C
typedef struct
{
	bn_int		id1;
} t_d2game_disappearitem;

#define D2GAME_UNK1 0x4D
typedef struct
{
	bn_short	unk1;
} t_d2game_unk1;

#define D2GAME_TRADE 0x4F
typedef struct
{
	bn_int		type;
	bn_short	gold;
} t_d2game_trade;

#define D2GAME_DROPGOLD 0x50
typedef struct
{
	bn_int		id;
	bn_int		gold;
} t_d2game_dropgold;

#define D2GAME_ASSIGNMENT 0x51
typedef struct
{
	bn_int		unk1;
	bn_int		unk2;
} t_d2game_assignment;

#define D2GAME_STAON 0x53
#define D2GAME_STAOFF 0x54

#define D2GAME_CLOSEQUEST 0x58
typedef struct
{
	bn_short	unk1;
} t_d2game_closequest;

#define D2GAME_TOWNFOLK 0x59
typedef struct
{
	bn_int		unk1;
	bn_int		unk2;
	bn_int		unk3;
	bn_int		unk4;
} t_d2game_townfolk;

#define D2GAME_RELATION 0x5d
typedef struct
{
	bn_byte		id;
	bn_byte		type;
	bn_int		player;
} t_d2game_relation;

#define D2GAME_PARTY 0x5E
typedef struct
{
	bn_byte		action;
	bn_int		player;
} t_d2game_party;

#define D2GAME_UPDATEPOSITION 0x5F
typedef struct
{
	bn_int		unk1;
} t_d2game_updateposition;

#define D2GAME_SWITCHEQUIP 0x60

#define D2GAME_POTIONTOMERCENARY 0x61
typedef struct
{
	bn_short	unk1;
} t_d2game_potiontomercenary;

#define D2GAME_RESURRECTMERC 0x62
typedef struct
{
	bn_int		id;
} t_d2game_resurrectmerc;

#define D2GAME_INVENTORYTOBELT 0x63
typedef struct
{
	bn_int		id;
} t_d2game_inventorytobelt;

#define D2GAME_GAMELOGON 0x68
typedef struct
{
	bn_int		hash;
	bn_short 	token;
	bn_byte		charid;
	bn_int		ver;
	bn_int		unk1;
	bn_int		unk2;
	bn_byte		unk3;
	bn_byte		charname[16];
} t_d2game_gamelogon;

#define D2GAME_GAMEEXIT 0x69

#define D2GAME_ENTERGAMEENVIRONMENT 0x6B

#define D2GAME_PING 0x6D
typedef struct
{
	bn_int		tick;
	bn_int		unk1;
	bn_int		unk2;
} t_d2game_ping;

#pragma pack(pop, pack01)

#endif