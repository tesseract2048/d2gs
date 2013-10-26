#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "d2gs.h"
#include "d2ge.h"
#include "eventlog.h"
#include "vars.h"
#include "net.h"
#include "handle_s2s.h"
#include "handle_d2game.h"
#include "debug.h"
#include "d2gamelist.h"
#include "d2game_protocol.h"
#include "nfd.h"

int handle_d2game(char* buf, int len, IOCPClient *Client)
{
	unsigned char type = buf[0];
	int i;
	char eof;
	char repeat;
	char msgtype;
	switch (type)
	{
		case D2GAME_WALKTOLOCATION:
			return sizeof(t_d2game_walktolocation) + 1;
			break;
		case D2GAME_WALKTOENTITY:
			return sizeof(t_d2game_walktoentity) + 1;
			break;
		case D2GAME_RUNTOLOCATION:
			return sizeof(t_d2game_runtolocation) + 1;
			break;
		case D2GAME_RUNTOENTITY:
			return sizeof(t_d2game_runtoentity) + 1;
			break;
		case D2GAME_LEFTSKILLONLOCATION:
			return sizeof(t_d2game_leftskillonlocation) + 1;
			break;
		case D2GAME_LEFTSKILLONENTITY:
			return sizeof(t_d2game_leftskillonentity) + 1;
			break;
		case D2GAME_LEFTSKILLONENTITYEX:
			return sizeof(t_d2game_leftskillonentityex) + 1;
			break;
		case D2GAME_LEFTSKILLONLOCATIONEX:
			return sizeof(t_d2game_leftskillonlocationex) + 1;
			break;
		case D2GAME_LEFTSKILLONENTITYEX2:
			return sizeof(t_d2game_leftskillonentityex2) + 1;
			break;
		case D2GAME_LEFTSKILLONENTITYEX3:
			return sizeof(t_d2game_leftskillonentityex3) + 1;
			break;
		case D2GAME_SKILLUNK1:
			return 1;
			break;
		case D2GAME_RIGHTSKILLONLOCATION:
			return sizeof(t_d2game_rightskillonlocation) + 1;
			break;
		case D2GAME_RIGHTSKILLONENTITY:
			return sizeof(t_d2game_rightskillonentity) + 1;
			break;
		case D2GAME_RIGHTSKILLONENTITYEX:
			return sizeof(t_d2game_rightskillonentityex) + 1;
			break;
		case D2GAME_RIGHTSKILLONLOCATIONEX:
			return sizeof(t_d2game_rightskillonlocationex) + 1;
			break;
		case D2GAME_RIGHTSKILLONENTITYEX2:
			return sizeof(t_d2game_rightskillonentityex2) + 1;
			break;
		case D2GAME_RIGHTSKILLONENTITYEX3:
			return sizeof(t_d2game_rightskillonentityex3) + 1;
			break;
		case D2GAME_SKILLUNK2:
			return 1;
			break;
		case D2GAME_INTERACTWITHENTITY:
			return sizeof(t_d2game_interactwithentity) + 1;
			break;
		case D2GAME_OVERHEADMESSAGE:
			eof = 0;
			repeat = 0;
			for(i = 3; i < len; i ++)
			{
				if (buf[i] == 0)
				{
					repeat ++;
					if (repeat == 3)
					{
						eof = 1;
						break;
					}
				}
			}
			if (strcmpi(&buf[3], "bye") == 0) return -1;
			if (eof == 0) return 0;
			return (++i);
			break;
		case D2GAME_CHATMESSAGE:
			eof = 0;
			msgtype = buf[1];
			repeat = 0;
			for(i = 3; i < len; i ++)
			{
				if (buf[i] == 0)
				{
					break;
				}
			}
			for(i++; i < len; i ++)
			{
				if (buf[i] == 0)
				{
					repeat ++;
					if (repeat == 2)
					{
						eof = 1;
						break;
					}
				}
			}
			if (eof == 0) return 0;
			return (++i);
			break;
		case D2GAME_PICKUPITEM:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_pickupitem) + 1;
			break;
		case D2GAME_DROPITEM:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_dropitem) + 1;
			break;
		case D2GAME_ITEMTOBUFFER:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_itemtobuffer) + 1;
			break;
		case D2GAME_PICKUPBUFFERITEM:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_pickupbufferitem) + 1;
			break;
		case D2GAME_ITEMTOBODY:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_itemtobody) + 1;
			break;
		case D2GAME_SWAP2HANDEDITEM:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_swap2handeditem) + 1;
			break;
		case D2GAME_PICKUPBODYITEM:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_pickupbodyitem) + 1;
			break;
		case D2GAME_SWITCHBODYITEM:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_switchbodyitem) + 1;
			break;
		case D2GAME_SWITCHBODYITEM2:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_switchbodyitem2) + 1;
			break;
		case D2GAME_SWITCHINVENTORYITEM:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_switchinventoryitem) + 1;
			break;
		case D2GAME_USEITEM:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_useitem) + 1;
			break;
		case D2GAME_STACKITEM:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_stackitem) + 1;
			break;
		case D2GAME_REMOVESTACKITEM:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_removestackitem) + 1;
			break;
		case D2GAME_ITEMTOBELT:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_itemtobelt) + 1;
			break;
		case D2GAME_REMOVEBELTITEM:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_removebeltitem) + 1;
			break;
		case D2GAME_SWITCHBELTITEM:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_switchbeltitem) + 1;
			break;
		case D2GAME_USEBELTITEM:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_usebeltitem) + 1;
			break;
		case D2GAME_IDENTIFYITEM:
			return sizeof(t_d2game_identifyitem) + 1;
			break;
		case D2GAME_INSERTSOCKETITEM:
			return sizeof(t_d2game_insertsocketitem) + 1;
			break;
		case D2GAME_SCROLLTOTOME:
			return sizeof(t_d2game_scrolltotome) + 1;
			break;
		case D2GAME_ITEMTOCUBE:
			InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_itemtocube) + 1;
			break;
		case D2GAME_UNSELECTOBJ:
			return 1;
			break;
		case D2GAME_CHATUNK1:
			return sizeof(t_d2game_chatunk1) + 1;
			break;
		case D2GAME_NPCINIT:
			return sizeof(t_d2game_npcinit) + 1;
			break;
		case D2GAME_NPCCANCEL:
			return sizeof(t_d2game_npccancel) + 1;
			break;
		case D2GAME_QUESTMESSAGE:
			return sizeof(t_d2game_questmessage) + 1;
			break;
		case D2GAME_NPCBUY:
			return sizeof(t_d2game_npcbuy) + 1;
			break;
		case D2GAME_NPCSELL:
			return sizeof(t_d2game_npcsell) + 1;
			break;
		case D2GAME_CAINIDENTIFY:
			return sizeof(t_d2game_cainidentify) + 1;
			break;
		case D2GAME_REPAIR:
			return sizeof(t_d2game_repair) + 1;
			break;
		case D2GAME_HIRE:
			return sizeof(t_d2game_hire) + 1;
			break;
		case D2GAME_GAMBLED:
			return sizeof(t_d2game_gambled) + 1;
			break;
		case D2GAME_NPCTRADE:
			return sizeof(t_d2game_npctrade) + 1;
			break;
		case D2GAME_STATPOINT:
			return sizeof(t_d2game_statpoint) + 1;
			break;
		case D2GAME_SKILLPOINT:
			return sizeof(t_d2game_skillpoint) + 1;
			break;
		case D2GAME_SWITCHSKILL:
			return sizeof(t_d2game_switchskill) + 1;
			break;
		case D2GAME_CLOSEDOOR:
			return sizeof(t_d2game_closedoor) + 1;
			break;
		case D2GAME_UPDATEITEMSTAT:
			return sizeof(t_d2game_updateitemstat) + 1;
			break;
		case D2GAME_CHARACTERPHRASE:
			return sizeof(t_d2game_characterphrase) + 1;
			break;
		case D2GAME_QUESTLOG:
			return 1;
			break;
		case D2GAME_RESPAWN:
			return 1;
			break;
		case D2GAME_UNK2:
			return 1;
			break;
		case D2GAME_PUTSLOT:
			return sizeof(t_d2game_putslot) + 1;
			break;
		case D2GAME_CHANGETP:
			return sizeof(t_d2game_changetp) + 1;
			break;
		case D2GAME_MERCINTER:
			return sizeof(t_d2game_mercinter) + 1;
			break;
		case D2GAME_MOVEMERC:
			return sizeof(t_d2game_movemerc) + 1;
			break;
		case D2GAME_WAYPOINT:
			return sizeof(t_d2game_waypoint) + 1;
			break;
		case D2GAME_REASSIGN:
			return sizeof(t_d2game_reassign) + 1;
			break;
		case D2GAME_UNK1:
			return sizeof(t_d2game_unk1) + 1;
			break;
		case D2GAME_TRADE:
			//InterlockedIncrement(&Client->RequestRate);
			return sizeof(t_d2game_trade) + 1;
			break;
		case D2GAME_DROPGOLD:
			return sizeof(t_d2game_dropgold) + 1;
			break;
		case D2GAME_ASSIGNMENT:
			return sizeof(t_d2game_assignment) + 1;
			break;
		case D2GAME_STAON:
			return 1;
			break;
		case D2GAME_STAOFF:
			return 1;
			break;
		case D2GAME_CLOSEQUEST:
			return sizeof(t_d2game_closequest) + 1;
			break;
		case D2GAME_TOWNFOLK:
			return sizeof(t_d2game_townfolk) + 1;
			break;
		case D2GAME_RELATION:
			return sizeof(t_d2game_relation) + 1;
			break;
		case D2GAME_PARTY:
			return sizeof(t_d2game_party) + 1;
			break;
		case D2GAME_UPDATEPOSITION:
			return sizeof(t_d2game_updateposition) + 1;
			break;
		case D2GAME_SWITCHEQUIP:
			return 1;
			break;
		case D2GAME_POTIONTOMERCENARY:
			return sizeof(t_d2game_potiontomercenary) + 1;
			break;
		case D2GAME_RESURRECTMERC:
			return sizeof(t_d2game_resurrectmerc) + 1;
			break;
		case D2GAME_INVENTORYTOBELT:
			return sizeof(t_d2game_inventorytobelt) + 1;
			break;
		case D2GAME_GAMEEXIT:
			return 1;
			break;
		case D2GAME_ENTERGAMEENVIRONMENT:
			return 1;
			break;
		case D2GAME_PING:
			//Client->PingTimer = GetTickCount();
			return sizeof(t_d2game_ping) + 1;
			break;
		default:
			D2GSEventLog("HandleGame", "UNKNOWN PACKET %x", type);
			return -1;
	}
}