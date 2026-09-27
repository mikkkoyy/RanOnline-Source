#include "stdafx.h"
#include "./GLItemDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

EMITEM_TYPE EXITEM_TO_ITEM( EMEXITEM_TYPE _emTYPE )
{
	switch (_emTYPE)
	{
	case EXITEM_SUIT:			return ITEM_SUIT;			break;
	case EXITEM_ARROW:			return ITEM_ARROW;			break;
	case EXITEM_CURE:			return ITEM_CURE;			break;
	case EXITEM_SKILL:			return ITEM_SKILL;			break;
	case EXITEM_RECALL:			return ITEM_RECALL;			break;
	case EXITEM_KEY:			return ITEM_KEY;			break;
	case EXITEM_GRINDING:		return ITEM_GRINDING;		break;
	case EXITEM_CHARM:			return ITEM_CHARM;			break;
	case EXITEM_TICKET:			return ITEM_TICKET;			break;
	case EXITEM_SKP_RESET:		return ITEM_SKP_RESET;		break;
	case EXITEM_STAT_RESET:		return ITEM_STAT_RESET;		break;
	case EXITEM_SKP_STAT_RESET:	return ITEM_SKP_STAT_RESET;	break;
	case EXITEM_BOX:			return ITEM_BOX;			break;
	case EXITEM_CLEANSER:		return ITEM_CLEANSER;		break;
	case EXITEM_LOUDSPEAKER:	return ITEM_LOUDSPEAKER;	break;
	case EXITEM_FIRECRACKER:	return ITEM_FIRECRACKER;	break;
	case EXITEM_CHARACTER_CARD:	return ITEM_CHARACTER_CARD;	break;
	case EXITEM_INVEN_CARD:		return ITEM_INVEN_CARD;		break;
	case EXITEM_STORAGE_CARD:	return ITEM_STORAGE_CARD;	break;
	case EXITEM_STORAGE_CONNECT:	return ITEM_STORAGE_CONNECT;	break;
	case EXITEM_PREMIUMSET:		return ITEM_PREMIUMSET;		break;
	case EXITEM_PRIVATEMARKET:	return ITEM_PRIVATEMARKET;	break;
	case EXITEM_RANDOMITEM:		return ITEM_RANDOMITEM;		break;
	case EXITEM_DISJUNCTION:	return ITEM_DISJUNCTION;	break;
	case EXITEM_HAIR:			return ITEM_HAIR;			break;
	case EXITEM_FACE:			return ITEM_FACE;			break;
	case EXITEM_QITEM:			return ITEM_QITEM;			break;
	case EXITEM_CD:				return ITEM_CD;				break;
	case EXITEM_2FRIEND:		return ITEM_2FRIEND;		break;
	case EXITEM_CLUBCALL:		return ITEM_CLUBCALL;		break;
	case EXITEM_HAIRSHOP:		return ITEM_HAIRSHOP;		break;
	case EXITEM_RENAME:			return ITEM_RENAME;			break;
	case EXITEM_HAIR_STYLE:		return ITEM_HAIR_STYLE;		break;
	case EXITEM_HAIR_COLOR:		return ITEM_HAIR_COLOR;		break;
	case EXITEM_REVIVE:			return ITEM_REVIVE;			break;
	case EXITEM_PET_CARD:		return ITEM_PET_CARD;		break;
	case EXITEM_PET_FOOD:		return ITEM_PET_FOOD;		break;
	case EXITEM_PET_RENAME:		return ITEM_PET_RENAME;		break;
	case EXITEM_PET_COLOR:		return ITEM_PET_COLOR;		break;
	case EXITEM_PET_STYLE:		return ITEM_PET_STYLE;		break;
	case EXITEM_PET_SKILL:		return ITEM_PET_SKILL;		break;
	case EXITEM_SMS:			return ITEM_SMS;			break;
	case EXITEM_PET_REVIVE:		return ITEM_PET_REVIVE;		break;
	case EXITEM_ANTI_DISAPPEAR:	return ITEM_ANTI_DISAPPEAR;		break;
	case EXITEM_REMODEL:		return ITEM_REMODEL;	break;
	case EXITEM_VEHICLE:		return ITEM_VEHICLE;	break;
	case EXITEM_VEHICLE_OIL:	return ITEM_VEHICLE_OIL;		break;
	case EXITEM_VIETNAM_ITEMGET:	return ITEM_VIETNAM_ITEMGET;	break;
	case EXITEM_VIETNAM_EXPGET:	return ITEM_VIETNAM_EXPGET;		break;
	case EXITEM_GENDER_CHANGE:	return ITEM_GENDER_CHANGE;		break;
	case EXITEM_GARBAGE_CARD:	return ITEM_GARBAGE_CARD;		break;
	case EXITEM_TELEPORT_CARD:	return ITEM_TELEPORT_CARD;		break;
	case EXITEM_PET_SKIN_PACK:	return ITEM_PET_SKIN_PACK;		break;
	case EXITEM_FACE_STYLE:		return ITEM_FACE_STYLE;			break;
	case EXITEM_TAXI_CARD:		return ITEM_TAXI_CARD;			break;
	case EXITEM_MATERIALS:		return ITEM_MATERIALS;			break;
	case EXITEM_NPC_RECALL:		return ITEM_NPC_RECALL;			break;
	case EXITEM_BULLET:			return ITEM_BULLET;				break;
	case EXITEM_FOOD:			return ITEM_FOOD;				break;
	case EXITEM_CARD_DUALPETSKILL:	return ITEM_CARD_DUALPETSKILL;	break;
	case EXITEM_BEADS_STRENGTHEN:	return ITEM_BEADS_STRENGTHEN;	break;
	case EXITEM_BEADS_RELAXANT:	return ITEM_BEADS_RELAXANT;			break;
	case EXITEM_CARD_BIKECOLOR:	return ITEM_CARD_BIKECOLOR;			break;
	case EXITEM_CARD_BIKEBOOST:	return ITEM_CARD_BIKEBOOST;			break;
	case EXITEM_OBLIVION_NEW:	return ITEM_OBLIVION_NEW;			break;
	case EXITEM_COSTUME_COLOR:	return ITEM_COSTUME_COLOR;			break;
	case EXITEM_CARD_MAILBOX:	return ITEM_CARD_MAILBOX;			break;
	case EXITEM_POINTCARD_A:	return ITEM_POINTCARD_A;			break;
	case EXITEM_POINTCARD_B:	return ITEM_POINTCARD_B;			break;
	case EXITEM_CRAFTING_BOOK:	return ITEM_CRAFTING_BOOK;			break;
	case EXITEM_RESET_CRAFTING:	return ITEM_RESET_CRAFTING;					break;
	case EXITEM_PERSONAL_STORE_SEARCH:	return ITEM_PERSONAL_STORE_SEARCH;	break;
	};


	return ITEM_NSIZE;
}

