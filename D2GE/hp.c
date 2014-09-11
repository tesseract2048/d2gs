#include "hp.h"
#include "eventlog.h"
#include "d2game_protocol.h"

#define HP_MAX_SLOT 65536
#define HP_RATE_LIMIT 8

#define HP_STATE_HEAD 0
#define HP_STATE_MSG 1
#define HP_STATE_STRING 2
#define HP_STATE_MSG_STRING 3

//#define NFD_LEVEL_INACTIVE
#define NFD_LEVEL_ACTIVE
#define NFD_LEVEL_ENFORCE
#define NFD_LEVEL_STRICT

typedef struct {
	char 	head;
	int		size;
	int		next_state;
	char	rate;
} PACKET_TABLE;

static PACKET_TABLE packets[255];

static unsigned int* ratetbl;
static unsigned int* statetbl;
static unsigned int* tailingtbl;

DWORD WINAPI hp_watch(LPVOID p);

void hp_set_packet(char head, int size, int next_state, char rate) {
	packets[head].head = head;
	packets[head].size = size;
	packets[head].next_state = next_state;
	packets[head].rate = rate;
}

void hp_init_packets() {
	memset(packets, 0, sizeof(PACKET_TABLE) * 255);
	hp_set_packet(D2GAME_WALKTOLOCATION, sizeof(t_d2game_walktolocation) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_WALKTOENTITY, sizeof(t_d2game_walktoentity) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_RUNTOLOCATION, sizeof(t_d2game_runtolocation) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_RUNTOENTITY, sizeof(t_d2game_runtoentity) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_LEFTSKILLONLOCATION, sizeof(t_d2game_leftskillonlocation) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_LEFTSKILLONENTITY, sizeof(t_d2game_leftskillonentity) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_LEFTSKILLONENTITYEX, sizeof(t_d2game_leftskillonentityex) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_LEFTSKILLONLOCATIONEX, sizeof(t_d2game_leftskillonlocationex) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_LEFTSKILLONENTITYEX2, sizeof(t_d2game_leftskillonentityex2) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_LEFTSKILLONENTITYEX3, sizeof(t_d2game_leftskillonentityex3) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_SKILLUNK1, 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_RIGHTSKILLONLOCATION, sizeof(t_d2game_rightskillonlocation) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_RIGHTSKILLONENTITY, sizeof(t_d2game_rightskillonentity) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_RIGHTSKILLONENTITYEX, sizeof(t_d2game_rightskillonentityex) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_RIGHTSKILLONLOCATIONEX, sizeof(t_d2game_rightskillonlocationex) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_RIGHTSKILLONENTITYEX2, sizeof(t_d2game_rightskillonentityex2) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_RIGHTSKILLONENTITYEX3, sizeof(t_d2game_rightskillonentityex3) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_SKILLUNK2, 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_INTERACTWITHENTITY, sizeof(t_d2game_interactwithentity) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_OVERHEADMESSAGE, sizeof(t_d2game_overheadmessage) + 1, HP_STATE_MSG_STRING, 0);
	hp_set_packet(D2GAME_CHATMESSAGE, sizeof(t_d2game_chatmessage) + 1, HP_STATE_MSG_STRING, 0);
	hp_set_packet(D2GAME_PICKUPITEM, sizeof(t_d2game_pickupitem) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_DROPITEM, sizeof(t_d2game_dropitem) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_ITEMTOBUFFER, sizeof(t_d2game_itemtobuffer) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_PICKUPBUFFERITEM, sizeof(t_d2game_pickupbufferitem) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_ITEMTOBODY, sizeof(t_d2game_itemtobody) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_SWAP2HANDEDITEM, sizeof(t_d2game_swap2handeditem) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_PICKUPBODYITEM, sizeof(t_d2game_pickupbodyitem) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_SWITCHBODYITEM, sizeof(t_d2game_switchbodyitem) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_SWITCHBODYITEM2, sizeof(t_d2game_switchbodyitem2) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_SWITCHINVENTORYITEM, sizeof(t_d2game_switchinventoryitem) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_USEITEM, sizeof(t_d2game_useitem) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_STACKITEM, sizeof(t_d2game_stackitem) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_REMOVESTACKITEM, sizeof(t_d2game_removestackitem) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_ITEMTOBELT, sizeof(t_d2game_itemtobelt) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_REMOVEBELTITEM, sizeof(t_d2game_removebeltitem) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_SWITCHBELTITEM, sizeof(t_d2game_switchbeltitem) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_USEBELTITEM, sizeof(t_d2game_usebeltitem) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_IDENTIFYITEM, sizeof(t_d2game_identifyitem) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_INSERTSOCKETITEM, sizeof(t_d2game_insertsocketitem) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_SCROLLTOTOME, sizeof(t_d2game_scrolltotome) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_ITEMTOCUBE, sizeof(t_d2game_itemtocube) + 1, HP_STATE_MSG, 1);
	hp_set_packet(D2GAME_UNSELECTOBJ, 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_CHATUNK1, sizeof(t_d2game_chatunk1) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_NPCINIT, sizeof(t_d2game_npcinit) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_NPCCANCEL, sizeof(t_d2game_npccancel) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_QUESTMESSAGE, sizeof(t_d2game_questmessage) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_NPCBUY, sizeof(t_d2game_npcbuy) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_NPCSELL, sizeof(t_d2game_npcsell) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_CAINIDENTIFY, sizeof(t_d2game_cainidentify) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_REPAIR, sizeof(t_d2game_repair) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_HIRE, sizeof(t_d2game_hire) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_GAMBLED, sizeof(t_d2game_gambled) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_NPCTRADE, sizeof(t_d2game_npctrade) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_STATPOINT, sizeof(t_d2game_statpoint) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_SKILLPOINT, sizeof(t_d2game_skillpoint) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_SWITCHSKILL, sizeof(t_d2game_switchskill) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_CLOSEDOOR, sizeof(t_d2game_closedoor) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_UPDATEITEMSTAT, sizeof(t_d2game_updateitemstat) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_CHARACTERPHRASE, sizeof(t_d2game_characterphrase) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_QUESTLOG, 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_RESPAWN, 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_UNK2, 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_PUTSLOT, sizeof(t_d2game_putslot) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_CHANGETP, sizeof(t_d2game_changetp) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_MERCINTER, sizeof(t_d2game_mercinter) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_MOVEMERC, sizeof(t_d2game_movemerc) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_WAYPOINT, sizeof(t_d2game_waypoint) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_REASSIGN, sizeof(t_d2game_reassign) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_UNK1, sizeof(t_d2game_unk1) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_TRADE, sizeof(t_d2game_trade) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_DROPGOLD, sizeof(t_d2game_dropgold) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_ASSIGNMENT, sizeof(t_d2game_assignment) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_STAON, 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_STAOFF, 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_CLOSEQUEST, sizeof(t_d2game_closequest) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_TOWNFOLK, sizeof(t_d2game_townfolk) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_RELATION, sizeof(t_d2game_relation) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_PARTY, sizeof(t_d2game_party) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_UPDATEPOSITION, sizeof(t_d2game_updateposition) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_SWITCHEQUIP, 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_POTIONTOMERCENARY, sizeof(t_d2game_potiontomercenary) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_RESURRECTMERC, sizeof(t_d2game_resurrectmerc) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_INVENTORYTOBELT, sizeof(t_d2game_inventorytobelt) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_GAMEEXIT, 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_GAMELOGON, sizeof(t_d2game_gamelogon) + 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_ENTERGAMEENVIRONMENT, 1, HP_STATE_MSG, 0);
	hp_set_packet(D2GAME_PING, sizeof(t_d2game_ping) + 1, HP_STATE_MSG, 0);
}

void hp_init() {
	hp_init_packets();
	ratetbl = (unsigned int*)malloc(HP_MAX_SLOT * sizeof(unsigned int));
	statetbl = (unsigned int*)malloc(HP_MAX_SLOT * sizeof(unsigned int));
	tailingtbl = (unsigned int*)malloc(HP_MAX_SLOT * sizeof(unsigned int));
	memset(ratetbl, 0, HP_MAX_SLOT * sizeof(unsigned int));
	memset(statetbl, 0, HP_MAX_SLOT * sizeof(unsigned int));
	memset(tailingtbl, 0, HP_MAX_SLOT * sizeof(unsigned int));
	CreateThread(NULL, NULL, hp_watch, NULL, NULL, NULL);
	D2GEEventLog(__FUNCTION__, "Hack prevent initialized with %d slots", HP_MAX_SLOT);
}

void hp_setup(SOCKET s) {
	if (s >= HP_MAX_SLOT) {
		D2GEEventLog(__FUNCTION__, "Handle out of range (%d/%d)", s, HP_MAX_SLOT);
		return;
	}
	InterlockedExchange(&statetbl[s], HP_STATE_HEAD);
	InterlockedExchange(&ratetbl[s], 0);
	InterlockedExchange(&tailingtbl[s], 0);
}

/**
	[Nomanland Firewall of D2GS]
 */

int nfd_filter(char const* data, int start, int end)
{
#ifdef NFD_LEVEL_INACTIVE
	return 0;
#endif
	int i;
	if (start < 0) start = 0;
	//FIXEEEEEEEEEEEEED: How about truncate?
	for (i = start; i <= end; i ++)
	{
		int dt = *((int*)&data[i]);
#ifdef NFD_LEVEL_ACTIVE
		if (dt == 0x00104D4A)
		{
			return -1;
		}
		if (dt == 0x20104D4A)
		{
			return -1;
		}
		if (dt == 0x00074D4A)
		{
			return -1;
		}
		if (dt == 0x08104D4A)
		{
			return -1;
		}
		if (dt == 0xAA55AA55)
		{
			return -1;
		}
#endif
#ifdef NFD_LEVEL_ENFORCE
		if (dt == 0x02033327 || dt == 0x02133327 || dt == 0x02233327 || dt == 0x02333327 || dt == 0x02432327 || dt == 0x02532327 || dt == 0x02632327 || dt == 0x02732327 || dt == 0x02832327 || dt == 0x02932327)
		{
			return -1;
		}
#endif
#ifdef NFD_LEVEL_STRICT
		if (dt == 0x00343272 || dt == 0x00353272 || dt == 0x00363272 || dt == 0x00373272 || dt == 0x00383272 || dt == 0x00393272 || dt == 0x00303372 || dt == 0x00313372 || dt == 0x00323372 || dt == 0x00333372)
		{
			return -1;
		}
		if (dt == 0x72323400 || dt == 0x72323500 || dt == 0x72323600 || dt == 0x72323700 || dt == 0x72323800 || dt == 0x72323900 || dt == 0x72333000 || dt == 0x72333100 || dt == 0x72333200 || dt == 0x72333300)
		{
			return -1;
		}
#endif
	}
	return 0;
}

void hp_handle_game_packet(int s, char* buf, int len) {
	int i, j, tailing;
	if (nfd_filter(buf, 0, len) < 0) {
		D2GEEventLog(__FUNCTION__, "Client 0x%x: Killed by nfd", s);
		closesocket((SOCKET)s);
		return;
	}
	for (i = 0; i < len; i ++) {
		switch (statetbl[s]) {
		case HP_STATE_HEAD:
			if (packets[buf[i]].head != buf[i]) {
				D2GEEventLog(__FUNCTION__, "Client 0x%x: Unknown packet 0x%x", s, buf[i]);
				continue;
			}
			if (packets[buf[i]].size > 1) {
				InterlockedExchange(&statetbl[s], packets[buf[i]].next_state);
				InterlockedExchange(&tailingtbl[s], packets[buf[i]].size - 1);
			}
			for (j = 0; j < packets[buf[i]].rate; j ++) {
				InterlockedIncrement(&ratetbl[s]);
			}
			//D2GEEventLog(__FUNCTION__, "Client 0x%x: Message 0x%x, Tailing: %d", s, buf[i], tailingtbl[s]);
			break;
		case HP_STATE_MSG_STRING:
		case HP_STATE_MSG:
			tailing = InterlockedDecrement(&tailingtbl[s]);
			if (tailing == 0) {
				if (statetbl[s] == HP_STATE_MSG_STRING) {
					InterlockedExchange(&statetbl[s], HP_STATE_STRING);
					InterlockedExchange(&tailingtbl[s], 3);
				} else {
					InterlockedExchange(&statetbl[s], HP_STATE_HEAD);
				}
			}
			break;
		case HP_STATE_STRING:
			if (buf[i] == 0) {
				tailing = InterlockedDecrement(&tailingtbl[s]);
				if (tailing == 0) InterlockedExchange(&statetbl[s], HP_STATE_HEAD);
			}
			break;
		}
	}
}

DWORD WINAPI hp_watch(LPVOID p) {
	int i;
	while (1) {
		for (i = 0; i < HP_MAX_SLOT; i ++) {
			if (ratetbl[i] > HP_RATE_LIMIT) {
				D2GEEventLog(__FUNCTION__, "Client 0x%x killed for rate limit (rate: %d)", i, ratetbl[i]);
				closesocket((SOCKET)i);
			}
			InterlockedExchange(&ratetbl[i], 0);
		}
		Sleep(1000);
	}
}