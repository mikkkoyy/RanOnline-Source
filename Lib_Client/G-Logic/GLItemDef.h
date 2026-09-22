#ifndef GLITEMDEF_H_
#define GLITEMDEF_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include "../../Lib_Engine/Meshs/DxPieceDefine.h"

#include "GLItemDefEX.h"

/*item color, Juver, 2018/01/04 */
#define ITEMCOLOR_WHITE 32767

/*dmk14 ingame web*/
#define MAX_ITEM_MALL		300

namespace glold_102
{
	enum GLITEM_ATT_102
	{
		ITEMATT_NOTHING		= 0,	//	착용무기없음.

		ITEMATT_SWORD		= 1,	//	검류.
		ITEMATT_REV00		= 2,	//	REV00
		ITEMATT_DAGGER		= 3,	//	단검류.
		ITEMATT_SPEAR		= 4,	//	창류.
		ITEMATT_BOW			= 5,	//	활류.
		ITEMATT_THROW		= 6,	//	투척류.

		ITEMATT_NOCARE		= 7,	//	타입 상관 안함.	( "GLSKILL" 에서 Hand 무기 제한 속성에서 사용됨. )
		ITEMATT_NSIZE		= 7,	//	종류갯수.

		ITEMATT_NEAR		= 4,	//	근접 공격.
	};
};

namespace glold_103
{
	enum GLITEM_ATT_103
	{
		ITEMATT_NOTHING		= 0,	//	착용무기없음.

		ITEMATT_SWORD		= 1,	//	검.
		ITEMATT_BLADE		= 2,	//	도.
		ITEMATT_DAGGER		= 3,	//	단검.
		ITEMATT_SPEAR		= 4,	//	창.
		ITEMATT_STICK		= 5,	//	몽둥이.
		ITEMATT_BOW			= 6,	//	활.
		ITEMATT_THROW		= 7,	//	투척.

		ITEMATT_NOCARE		= 8,	//	타입 상관 안함.	( "GLSKILL" 에서 Hand 무기 제한 속성에서 사용됨. )
		ITEMATT_NSIZE		= 8,	//	종류갯수.

		ITEMATT_NEAR		= 5,	//	근접 공격.
	};
};

namespace glold_108
{
	enum GLITEM_ATT_108
	{
		ITEMATT_NOTHING		= 0,	//	착용무기없음.

		ITEMATT_SWORD		= 1,	//	검.
		ITEMATT_BLADE		= 2,	//	도.
		ITEMATT_SWORDBLADE	= 3,	//	검/도. "검/도" 는 스킬에서 사용무기 확인시에만 사용됨.

		ITEMATT_DAGGER		= 4,	//	단검.
		ITEMATT_SPEAR		= 5,	//	창.
		ITEMATT_STICK		= 6,	//	몽둥이.
		ITEMATT_GAUNT		= 7,	//	권.
		ITEMATT_BOW			= 8,	//	활.
		ITEMATT_THROW		= 9,	//	투척.

		ITEMATT_NOCARE		= 10,	//	타입 상관 안함.	( "GLSKILL" 에서 Hand 무기 제한 속성에서 사용됨. )
		ITEMATT_NSIZE		= 10,	//	종류갯수.

		ITEMATT_NEAR		= 7,	//	근접 공격.
	};
};

enum EMITEMLEVEL
{
	LEVEL_NORMAL	= 0,	//	일반
	LEVEL_RARE		= 1,	//	진귀한
	LEVEL_UNIQUE	= 2,	//	유일한
	LEVEL_INFINITY	= 3,	//	무한
	LEVEL_FERVOR	= 4,	//	백열
	LEVEL_LEGENDARY	= 5,	// 전설;
	
	LEVEL_NSIZE		= 6		//	종류갯수.
};

enum EMITEM_FLAG
{
	TRADE_SALE			= 0x01,	//	상점에 판매 가능.
	TRADE_EXCHANGE		= 0x02,	//	개인간 교환 가능.
	TRADE_THROW			= 0x04,	//	필드에 버리기 가능.

	TRADE_EVENT_SGL		= 0x08,	//	여러 이벤트 아이템중 한개 이상 못 줍게 하기 위한.

	ITEM_DISGUISE		= 0x10,	//	코스툼.
	ITEM_TIMELMT		= 0x20,	//	시효성.
	
	ITEM_CHANNEL_ALL	= 0x40, //	모든 체널.

	TRADE_GARBAGE		= 0x80, //	파쇄 가능

	ITEM_FLAG_EX_1		= 0x100,
	ITEM_FLAG_EX_2		= 0x200,
	ITEM_FLAG_EX_3		= 0x400,
	ITEM_FLAG_EX_4		= 0x800,
	ITEM_FLAG_EX_5		= 0x1000,

	TRADE_ALL		= TRADE_SALE|TRADE_EXCHANGE|TRADE_THROW,
};

enum EMITEM_HAND
{
	HAND_RIGHT		= 0x0001,	//	오른손 용.
	HAND_LEFT		= 0x0002,	//	왼손 용.
	HAND_BOTH		= 0x0003,	//	왼손, 오른손 모두 가능.
};

//	Note : 캐릭터 에니메이션 제어와 관련이 있음.
//		캐릭터 에니메이션 선택할때 착용 아이탬에 따라 선택을 함.
//
enum GLITEM_ATT
{
	ITEMATT_NOTHING		= 0,	//	착용무기없음.
	ITEMATT_SWORD		= 1,	//	검.
	ITEMATT_BLADE		= 2,	//	도.
	ITEMATT_DAGGER		= 3,	//	단검.
	ITEMATT_SPEAR		= 4,	//	창.
	ITEMATT_STICK		= 5,	//	몽둥이.
	ITEMATT_GAUNT		= 6,	//	권.
	ITEMATT_BOW			= 7,	//	활.
	ITEMATT_THROW		= 8,	//	투척.
	ITEMATT_GUN			= 9,
	ITEMATT_RAILGUN		= 10,
	ITEMATT_PORTALGUN	= 11,
	ITEMATT_SCYTHE		= 12,
	ITEMATT_DUALSPEAR	= 13,
	ITEMATT_SHURIKEN	= 14,
	ITEMATT_FIST		= 15,
	ITEMATT_WAND		= 16,
	ITEMATT_CUBE		= 17,
	ITEMATT_WHIP		= 18,
	ITEMATT_SHIELD		= 19,
	ITEMATT_HAMMER		= 20,
	ITEMATT_UMBRELLA	= 21,
	ITEMATT_NOCARE		= 22,
	ITEMATT_SHOTGUN		= 23,
	ITEMATT_GUNS		= 24,
	ITEMATT_SWORDSABER	= 25,

	ITEMATT_NEAR		= 6,	//	근접 공격.
	ITEMATT_NSIZE		= 26,	//	종류갯수.
};

enum GLITEM_CONST
{
	ITEM_SZNAME		= 65,	//	item 이름 길이 제한. (NULL) 문자 포함 길이.
	ITEM_SZCOMMENT	= 256,	//	item "간략설명" 길이 제한. (NULL) 문자 포함 길이.
	ITEM_SKILLS		= 4,	//	"스킬 포인트 증가치" 가 붙을수 있는 갯수.
};

enum EMSUIT
{
	SUIT_HEADGEAR		= 0,
	SUIT_UPPER			= 1,
	SUIT_LOWER			= 2,
	SUIT_HAND			= 3,
	SUIT_FOOT			= 4,

	SUIT_HANDHELD		= 5,

	SUIT_NECK			= 6,
	SUIT_WRIST			= 7,

	SUIT_FINGER			= 8,

	SUIT_PET_A			= 9,	// PetData
	SUIT_PET_B			= 10,
	SUIT_VEHICLE		= 11,	// 탈것
	
	SUIT_VEHICLE_SKIN		= 12,	// 탈것 중 보드 스킨
	SUIT_VEHICLE_PARTS_A	= 13,	// 탈것 중 보드 파츠
	SUIT_VEHICLE_PARTS_B	= 14,
	SUIT_VEHICLE_PARTS_C	= 15,
	SUIT_VEHICLE_PARTS_D	= 16,	// 탈것 중 보드 파츠
	SUIT_VEHICLE_PARTS_E	= 17,
	SUIT_VEHICLE_PARTS_F	= 18,
	SUIT_BELT				= 19,//add wing
	SUIT_EARRING			= 20,//add aura
	SUIT_ACCESSORY			= 21,//add newitem
	SUIT_ORNAMENT			= 22,//add newitem
	SUIT_FACE				= 23,
	SUIT_MISC				= 24,

	SUIT_NSIZE				= 25,
};

//	Note : 착용위치.
enum EMSLOT
{
	SLOT_HEADGEAR	= 0,	//	머리 쓸것
	SLOT_UPPER		= 1,	//	상체
	SLOT_LOWER		= 2,	//	하체
	SLOT_HAND		= 3,	//	손
	SLOT_FOOT		= 4,	//	발

	SLOT_RHAND		= 5,	//	오른손 도구
	SLOT_LHAND		= 6,	//	왼손 도구
	
	SLOT_NECK		= 7,	//	목걸이
	SLOT_WRIST		= 8,	//	손?E
	SLOT_RFINGER	= 9,	//	오른손 손가?E
	SLOT_LFINGER	= 10,	//	왼손 손가?E
	SLOT_RHAND_S	= 11,	//	오른손 도구, 극강부
	SLOT_LHAND_S	= 12,	//	왼손 도구, 극강부

	SLOT_VEHICLE	= 13,	//  탈것 
	SLOT_LEARRING		= 14,	//SLOT_LACCESSORY		= 14,	
	SLOT_LACCESSORY		= 15,	//SLOT_LEARRING		= 15,	
	SLOT_RACCESSORY		= 16,	//SLOT_WAIST			= 16,	
	SLOT_ORNAMENT		= 17,   //SLOT_ORNAMENT
	SLOT_WAIST			= 18,	//SLOT_RACCESSORY		= 18,

	//
	SLOT_FACE		= 19,
	SLOT_MISC		= 20,

	SLOT_NSIZE		= 13,   
	SLOT_NSIZE_S	= 15,	
	SLOT_NSIZE_2	= 19,   // Normal Class
	SLOT_NSIZE_S_2	= 21,	// Extreme Class

	SLOT_HOLD		= 21,	// I dont know what the fuck is this 
	SLOT_TSIZE		= 22,   //Max Slot Size
	
	SLOT_WEAR_SIMPLE = 5,	/*character simple, Juver, 2017/10/01 */
};

inline EMSUIT SLOT_2_SUIT ( EMSLOT emSlot )
{
	switch( emSlot )
	{
	case SLOT_HEADGEAR: return SUIT_HEADGEAR;
	case SLOT_UPPER:	return SUIT_UPPER;
	case SLOT_LOWER:	return SUIT_LOWER;
	case SLOT_HAND:		return SUIT_HAND;
	case SLOT_FOOT:		return SUIT_FOOT;

	case SLOT_RHAND:	
	case SLOT_RHAND_S:
	case SLOT_LHAND:	
	case SLOT_LHAND_S:	return SUIT_HANDHELD;

	case SLOT_NECK:		return SUIT_NECK;
	case SLOT_WRIST:	return SUIT_WRIST;
	case SLOT_RFINGER:
	case SLOT_LFINGER:	return SUIT_FINGER;
	case SLOT_VEHICLE:	return SUIT_VEHICLE;

	case SLOT_WAIST:			return SUIT_BELT;

	case SLOT_LEARRING:			return SUIT_EARRING;

	case SLOT_ORNAMENT:			return SUIT_ORNAMENT;
		
	case SLOT_FACE:				return SUIT_FACE;
	case SLOT_MISC:				return SUIT_MISC;

	case SLOT_LACCESSORY:		
	case SLOT_RACCESSORY:		return SUIT_ACCESSORY;

	

	default:			return SUIT_NSIZE;
	}
}

inline EMPIECECHAR SLOT_2_PIECE ( EMSLOT emSlot )
{
	switch( emSlot )
	{
	case SLOT_HEADGEAR: return PIECE_HEADGEAR;
	case SLOT_UPPER:	return PIECE_UPBODY;
	case SLOT_LOWER:	return PIECE_LOBODY;
	case SLOT_HAND:		return PIECE_GLOVE;
	case SLOT_FOOT:		return PIECE_FOOT;

	case SLOT_RHAND:	
	case SLOT_RHAND_S:	return PIECE_RHAND;
	case SLOT_LHAND:	
	case SLOT_LHAND_S:	return PIECE_LHAND;
	case SLOT_VEHICLE:	return PIECE_VEHICLE;

	case SLOT_NECK:		return PIECE_NECK;
	case SLOT_WRIST:	return PIECE_WRIST;
	case SLOT_RFINGER:
	case SLOT_LFINGER:	return PIECE_RING;

	case SLOT_WAIST:	return PIECE_BELT; 

	case SLOT_LEARRING:	return PIECE_EAR_RING; 

	case SLOT_FACE:		return PIECE_REV02;
	case SLOT_MISC:		return PIECE_REV01;

	case SLOT_RACCESSORY:		
	case SLOT_LACCESSORY: return PIECE_ACCESSORY;
	default:					return PIECE_SIZE;
	}
}

/*upgrade effect, Juver, 2017/09/01 */
inline EMPIECECHAR SLOT_2_UPGRADEPIECE ( EMSLOT emSlot )
{
	switch( emSlot )
	{
	case SLOT_UPPER:	return PIECE_ENCHANT_UPBODY;
	case SLOT_LOWER:	return PIECE_ENCHANT_LOBODY;
	case SLOT_HAND:		return PIECE_ENCHANT_GLOVE;
	case SLOT_FOOT:		return PIECE_ENCHANT_FOOT;
	default:			return PIECE_SIZE;
	};
}

inline EMSLOT PIECE_2_SLOT ( EMPIECECHAR emPiece )
{
	switch( emPiece )
	{
	case PIECE_HEAD:		return SLOT_HEADGEAR;
	case PIECE_UPBODY:		return SLOT_UPPER;
	case PIECE_LOBODY:		return SLOT_LOWER;
	case PIECE_GLOVE:		return SLOT_HAND;
	case PIECE_RHAND:		return SLOT_RHAND;
	case PIECE_LHAND:		return SLOT_LHAND;
	case PIECE_FOOT:		return SLOT_FOOT;
	case PIECE_VEHICLE:		return SLOT_VEHICLE;
	case PIECE_HAIR:
	case PIECE_HEADGEAR:	return SLOT_TSIZE;

	case PIECE_REV01:
	case PIECE_REV02:
	case PIECE_REV03:
	case PIECE_SIZE:		return SLOT_TSIZE;

	case PIECE_NECK:		return SLOT_NECK;
	case PIECE_WRIST:		return SLOT_WRIST;
	case PIECE_RING:		return SLOT_RFINGER;

	case PIECE_BELT:			return SLOT_WAIST;
	case PIECE_EAR_RING:		return SLOT_LEARRING;
	case PIECE_ACCESSORY:		return SLOT_RACCESSORY;
	default:				return SLOT_TSIZE;
	}
}

enum EMITEM_TYPE
{
	ITEM_SUIT				= 0,

	ITEM_ARROW				= 1,	//	화살.

	ITEM_CURE				= 2,	//	약품.
	ITEM_SKILL				= 3,	//	스킬 습득 서적.
	ITEM_RECALL				= 4,	//	귀환서.
	ITEM_KEY				= 5,	//	인증서 ( 열쇠 ).
	ITEM_GRINDING			= 6,	//	연마제
	ITEM_CHARM				= 7,	//	부적.
	ITEM_TICKET				= 8,	//	승차권.
	ITEM_SKP_RESET			= 9,	//	스킬 포인트 리셋.
	ITEM_STAT_RESET			= 10,	//	스텟 포인트 리셋.
	ITEM_SKP_STAT_RESET		= 11,	//	스킬, 스텟 포인트 리셋.

	ITEM_BOX				= 12,	//	선물 상자.
	ITEM_CLEANSER			= 13,	//	세탁제.
	ITEM_LOUDSPEAKER		= 14,	//	확성기.
	ITEM_FIRECRACKER		= 15,	//	폭죽.
	ITEM_CHARACTER_CARD		= 16,	//	캐릭터 추가 카드.
	ITEM_INVEN_CARD			= 17,	//	인벤 확장 카드.
	ITEM_STORAGE_CARD		= 18,	//	창고 확장 카드.
	ITEM_STORAGE_CONNECT	= 19,	//	창고 연결 카드.
	ITEM_PREMIUMSET			= 20,	//	프리미엄 세트.
	ITEM_PRIVATEMARKET		= 21,	//	개인상점 개설 권리.
	ITEM_RANDOMITEM			= 22,	//	랜덤 아이탬.
	ITEM_DISJUNCTION		= 23,	//	코스툼 분리.
	ITEM_HAIR				= 24,	//	헤어스타일 변경.
	ITEM_FACE				= 25,	//	얼굴 변경.
	ITEM_QITEM				= 26,	//	? 아이템.
	ITEM_CD					= 27,	//	CD.
	ITEM_2FRIEND			= 28,	//	친구에게.
	ITEM_CLUBCALL			= 29,	//	클럽호출.
	ITEM_HAIRSHOP			= 30,	//	헤어샾 이용권.
	ITEM_RENAME				= 31,   //  이름변경 카드.
	ITEM_HAIR_STYLE			= 32,	//  헤어스타일
	ITEM_HAIR_COLOR			= 33,   //  헤어컬러
	ITEM_REVIVE				= 34,	//  귀혼주
	ITEM_PET_CARD			= 35,	//	펫 카드	// PetData
	ITEM_PET_FOOD			= 36,	//	펫 먹이
	ITEM_PET_RENAME			= 37,	//	팻 이름변경
	ITEM_PET_COLOR			= 38,	//	팻 컬러변경
	ITEM_PET_STYLE			= 39,	//	팻 스타일변경
	ITEM_PET_SKILL			= 40,	//	팻 스킬
	ITEM_SMS				= 41,	//	SMS문자 발송
	ITEM_PET_REVIVE			= 42,	//	팻 부활카드
	ITEM_ANTI_DISAPPEAR 	= 43,	//	소방주(소멸방지)
	ITEM_REMODEL			= 44,	//	개조 기능
	ITEM_VEHICLE			= 45,	//	탈것
	ITEM_VEHICLE_OIL		= 46,	//  탈것 기름
	ITEM_VIETNAM_ITEMGET	= 47,	//  베트남 탐닉 방지 아이템 획득
	ITEM_VIETNAM_EXPGET		= 48,	//  베트남 탐닉 방지 경험치 획득	
	ITEM_GENDER_CHANGE		= 49,	//  성별 변경 카드 
	ITEM_GARBAGE_CARD		= 50,	//	휴지통 카드	
	ITEM_TELEPORT_CARD		= 51,	//	소환서	
	ITEM_PET_SKIN_PACK  	= 52,	//  펫 스킨 팩
	ITEM_FACE_STYLE			= 53,	//	얼굴스타일
	ITEM_TAXI_CARD			= 54,	//	택시카드
	ITEM_MATERIALS			= 55,	//	재료아이템
	ITEM_NPC_RECALL			= 56,	//	NPC소환 카드
	ITEM_BULLET				= 57,	/*gun-bullet logic, Juver, 2017/05/27 */
	ITEM_FOOD				= 58,	/*itemfood system, Juver, 2017/05/26 */
	ITEM_CARD_DUALPETSKILL	= 59,	/*dual pet skill, Juver, 2017/12/29 */
	ITEM_BEADS_STRENGTHEN	= 60,
	ITEM_BEADS_RELAXANT		= 61,
	ITEM_CARD_BIKECOLOR		= 62,	/*bike color , Juver, 2017/11/12 */
	ITEM_CARD_BIKEBOOST		= 63,	/*vehicle booster system, Juver, 2017/08/10 */
	ITEM_OBLIVION_NEW		= 64,
	ITEM_COSTUME_COLOR		= 65,	/*item color, Juver, 2018/01/10 */
	ITEM_CARD_MAILBOX		= 66,
	ITEM_CARD_FOODUNLOCK	= 67,	/*itemfood system, Juver, 2017/05/26 */
	ITEM_POINTCARD_A		= 68,
	ITEM_POINTCARD_B		= 69,
	ITEM_CRAFTING_BOOK		= 70,
	ITEM_RANDOM_OPTION_CARD	= 71,	/*rv card, Juver, 2017/11/25 */
	ITEM_NONDROP_CARD		= 72,	/*nondrop card, Juver, 2017/11/26 */
	ITEM_RESET_CRAFTING			= 73,
	ITEM_PERSONAL_STORE_SEARCH	= 74,
	ITEM_CHANGE_SCALE_CARD	= 75,	/*change scale card, Juver, 2018/01/03 */
	ITEM_WRAPPER			= 76,	/*item wrapper, Juver, 2018/01/11 */
	ITEM_WRAPPER_BOX		= 77,	/*item wrapper, Juver, 2018/01/11 */
	ITEM_CHANGE_SCHOOL		= 78,	/*change school card, Juver, 2018/01/12 */
	ITEM_EQUIPMENT_LOCK_ENABLE	= 79,	/*equipment lock, Juver, 2018/01/13 */
	ITEM_EQUIPMENT_LOCK_RECOVER	= 80,	/*equipment lock, Juver, 2018/01/13 */
	ITEM_TRANSFER_CARD		= 81,	/*item transfer card, Juver, 2018/01/18 */
	ITEM_CARD_CAR_COLOR		= 82,	/* car, cart color, Juver, 2018/02/14 */
	ITEM_CARD_VEHICLE_BOOST	= 83,	/* booster all vehicle, Juver, 2018/02/14 */
	
	ITEM_SEALED_CARD		= 84,		// add sealed card
	ITEM_COSTUME_BLESSED_PAPER					= 85,

	ITEM_MYSTERY_BOX		= 86,
	ITEM_MYSTERY_KEY		= 87,

	ITEM_RESET_BRIGHT_CARD	= 88,
	ITEM_EPOINT_CARD		= 89,

	ITEM_SKILL_REFORMCARD		= 90,

	ITEM_STORAGE_LOCK_ENABLE	= 91,	/*storage lock, Juver, 2018/01/13 */
	ITEM_STORAGE_LOCK_RECOVER	= 92,	/*storage lock, Juver, 2018/01/13 */

	ITEM_INVENTORY_LOCK_ENABLE	= 93,	/*inventory lock, Juver, 2018/01/13 */
	ITEM_INVENTORY_LOCK_RECOVER	= 94,	/*inventory lock, Juver, 2018/01/13 */

	ITEM_CONTRIBUTION_CARD		= 95,

	ITEM_NSIZE				= 96,	//	종류갯수.
};

EMITEM_TYPE EXITEM_TO_ITEM( EMEXITEM_TYPE _emTYPE );

enum EMITEM_DRUG
{
	ITEM_DRUG_NONE			= 0,
	ITEM_DRUG_HP			= 1,	//	채력
	ITEM_DRUG_MP			= 2,	//	기력
	ITEM_DRUG_SP			= 3,	//	활력

	ITEM_DRUG_HP_MP			= 4,	//	체력+기력
	ITEM_DRUG_MP_SP			= 5,	//	체력+활력
	ITEM_DRUG_HP_MP_SP		= 6,	//	체력+기력+활력
	
	ITEM_DRUG_CURE			= 7,	//	상태 이상 치료.

	ITEM_DRUG_CALL_SCHOOL	= 8,	//	학교귀환.
	ITEM_DRUG_CALL_REGEN	= 9,	//	시작귀환.
	ITEM_DRUG_CALL_LASTCALL	= 10,	//	직전귀환.
	ITEM_DRUG_CALL_REVIVE	= 11,	//	부활기능.

	ITEM_DRUG_HP_CURE		= 12,	// 체력+이상치료
	ITEM_DRUG_HP_MP_SP_CURE	= 13,	// 체력+기력+활력+이상치료
	ITEM_DRUG_CALL_TELEPORT	= 14,	// 특정공간이동

	ITEM_DRUG_CP			= 15,	/*combatpoint logic, Juver, 2017/05/27 */

	ITEM_DRUG_SIZE			= 16
};

enum EMITEM_ADDON
{
	EMADD_NONE		= 0,
	EMADD_HITRATE	= 1,	//	명중율
	EMADD_AVOIDRATE	= 2,	//	회피율.
	
	EMADD_DAMAGE	= 3,	//	공격력.
	EMADD_DEFENSE	= 4,	//	방어력.

	EMADD_HP		= 5,	//	채력 증가.
	EMADD_MP		= 6,	//	마나 증가.
	EMADD_SP		= 7,	//	스테미나 증가.

	EMADD_STATS_POW	= 8,	//	힘.
	EMADD_STATS_STR	= 9,	//	체력.
	EMADD_STATS_SPI	= 10,	//	정신.
	EMADD_STATS_DEX	= 11,	//	민첩.
	EMADD_STATS_INT	= 12,	//	지성.
	EMADD_STATS_STA	= 13,	//	근력.

	EMADD_PA		= 14,	//	격투치.
	EMADD_SA		= 15,	//	사격치.
	EMADD_MA		= 16,	//	마력치.
	EMADD_POWINTDEX		= 17,	//	마력치.
	EMADD_STMVIT	= 18,	//	마력치.
	EMADD_PASA		= 19,	//	마력치.
	EMADD_PASAMA	= 20,	//	마력치.

	EMADD_SIZE		= 21,
};

enum EMITEM_VAR
{
	EMVAR_NONE			= 0,
	EMVAR_HP			= 1,
	EMVAR_MP			= 2,
	EMVAR_SP			= 3,
	EMVAR_AP			= 4,
	EMVAR_MOVE_SPEED	= 5,
	EMVAR_ATTACK_SPEED	= 6,
	EMVAR_CRITICAL_RATE	= 7,
	EMVAR_CRUSHING_BLOW	= 8, 

	EMVAR_DECR_DAMAGE_MELEESKILL = 9,	// 물리 근접 스킬 피격시 경우 데미지 감소;
	EMVAR_DECR_DAMAGE_RANGESKILL = 10,	// 사격 스킬 피격시 경우 데미지 감소;
	EMVAR_DECR_DAMAGE_MAGICSKILL = 11,	// 마법 스킬 피격시 경우 데미지 감소;

	EMVAR_SIZE			= 12,
};

enum EMITEM_QUESTION
{
	QUESTION_NONE			= 0,
	QUESTION_SPEED_UP		= 1,
	QUESTION_CRAZY			= 2,
	QUESTION_ATTACK_UP		= 3,
	QUESTION_EXP_UP			= 4,
	QUESTION_EXP_GET		= 5,
	QUESTION_LUCKY			= 6,
	QUESTION_BOMB			= 7,
	QUESTION_MOBGEN			= 8,

	QUESTION_SPEED_UP_M		= 9,
	QUESTION_MADNESS		= 10,
	QUESTION_ATTACK_UP_M	= 11,
	QUESTION_HEAL			= 12,

	QUESTION_SIZE			= 13
};

enum EMRANDOM_OPT
{
	EMR_OPT_NULL			= 0,
	EMR_OPT_DAMAGE			= 1,	//	공격력.
	EMR_OPT_DEFENSE			= 2,	//	방엉澹.

	EMR_OPT_HITRATE			= 3,	//	뫄鵬율.
	EMR_OPT_AVOIDRATE		= 4,	//	회피율.

	EMR_OPT_HP				= 5,	//	hp 증가.
	EMR_OPT_MP				= 6,	//	mp 증가.
	EMR_OPT_SP				= 7,	//	sp 증가.

	EMR_OPT_HP_INC			= 8,	//	hp 증가율.
	EMR_OPT_MP_INC			= 9,	//	mp 증가율.
	EMR_OPT_SP_INC			= 10,	//	sp 증가율.
	EMR_OPT_HMS_INC			= 11,	//	hp, mp, sp 증가율.

	EMR_OPT_GRIND_DAMAGE	= 12,	//	공격력 연마.
	EMR_OPT_GRIND_DEFENSE	= 13,	//	방엉澹 연마.

	EMR_OPT_RANGE			= 14,	//	공격 영역.
	EMR_OPT_DIS_SP			= 15,	//	sp 소모량.
	EMR_OPT_RESIST			= 16,	//	저항.

	EMR_OPT_MOVE_SPEED		= 17,	// 이동속도 증가
	
	// Add latest Random Values
	// Latest RVs'
	EMR_OPT_ATTACK_VOL		= 18,	// Attack
	EMR_OPT_DEFENSE_VOL		= 19,	// Defense
	EMR_OPT_HIT_VOL			= 20,	// Hit Rate
	EMR_OPT_AVOID_VOL		= 21,	// Avoid Rate
	EMR_OPT_POWER			= 22,	// Power
	EMR_OPT_VITALITY		= 23,	// Vitality
	EMR_OPT_SPIRIT			= 24,	// Spirit
	EMR_OPT_DEXTERITY		= 25,	// Dexterity
	EMR_OPT_STAMINA			= 26,	// Stamina
	EMR_OPT_MELEE			= 27,	// Melee
	EMR_OPT_SHOOTING		= 28,	// Missile
	EMR_OPT_ENERGY			= 29,	// Energy
	EMR_OPT_HP_REC			= 30,	// HP Potion Recover Increase Amount
	EMR_OPT_MP_REC			= 31,	// MP Potion Recover Increase Amount
	EMR_OPT_SP_REC			= 32,	// SP Potion Recover Increase Amount
	EMR_OPT_CP				= 33,	// MP Potion Recover Increase Amount

	EMR_OPT_BLOW_RATE		= 34,	// MP Potion Recover Increase Amount
	EMR_OPT_STRIKE_RATE		= 35,	// SP Potion Recover Increase Amount

	EMR_OPT_SIZE			= 36
};

enum EMCOOL_TYPE
{
	EMCOOL_ITEMID	= 0,	// MID/SID
	EMCOOL_ITEMTYPE	= 1,	// 아이템 타입
	EMCOOL_SIZE		= 2,	// 사이즈
};

struct SRANDOM_OPT
{
	WORD	wTYPE;
	short	nVALUE;

	SRANDOM_OPT()
		: wTYPE( EMR_OPT_NULL )
		, nVALUE( 0 )
	{
	}

	float get_float()	{ return nVALUE * 0.01f; }
	float get_int()		{ return nVALUE; }
};

struct SRANDOM_OPT_SECTION
{
	float	fRATE;	// 구간별 발생 확률
	float	fHIGH;	// 값 최대
	float	fLOW;	// 값 최소

	SRANDOM_OPT_SECTION()
		: fRATE( 0.0f )
		, fHIGH( 0.0f )
		, fLOW( 0.0f )
	{
	}
};

struct SRANDOM_OPT_SET
{
	float					fRATE;	// 설정된 옵션당 발생 확률
	WORD					wTYPE;	// 옵션 종류
	WORD					wSECT;	// 구간 수
	SRANDOM_OPT_SECTION*	pSECT;	// 구간

	SRANDOM_OPT_SET()
		: fRATE( 0.0f )
		, wTYPE( EMR_OPT_NULL )
		, wSECT( 0 )
		, pSECT( NULL )
	{
	}

	~SRANDOM_OPT_SET()
	{
		SAFE_DELETE_ARRAY( pSECT );
	}
};

struct SRANDOM_GEN
{
	enum { MAX_SET = 20, MAX_OPT = 5 };

	float			fRATE;				// 랜덤 옵션 발생 확률
	DWORD			dwSET_NUM;			// 설정된 랜덤 옵션 갯수
	float			fD_point;			// 개조 시 소멸할 수 있는 확률
	float			fS_value;			// 개조 시 필요한 금액 배율 ( 아이템 가격에 곱함 )
	SRANDOM_OPT_SET	sSET[ MAX_SET ];	// 설정된 옵션 ( 처음 생성 시 사용 )
	SRANDOM_OPT_SET	sSETR[ MAX_SET ];	// 설정된 옵션 ( 이후 개조 시 사용 )

	SRANDOM_GEN()
		: fRATE( 0.0f )
		, dwSET_NUM( 0 )
		, fD_point( 0.0f )
		, fS_value( 1.0f )
	{
	}
};
	
struct SRANDOM_DATA
{
	enum { NAME_LEN = 64 };
	char			szNAME[ NAME_LEN ];
	SRANDOM_GEN*	pSET;

	SRANDOM_DATA()
		: pSET( NULL )
	{
		memset( szNAME, 0, sizeof(char) * NAME_LEN );
	}
};

struct SRANDOM_DATA_OPER
{
	bool operator() ( const SRANDOM_DATA &lvalue, const SRANDOM_DATA &rvalue )
	{
		return strcmp(lvalue.szNAME,rvalue.szNAME)<0;
	}

	bool operator() ( const SRANDOM_DATA &lvalue, const char *szNAME )
	{
		return strcmp(lvalue.szNAME,szNAME)<0;
	}

	bool operator() ( const char *szNAME, const SRANDOM_DATA &rvalue )
	{
		return strcmp(szNAME,rvalue.szNAME)<0;
	}
};

enum EMGRINDING_CLASS
{
	EMGRINDING_CLASS_ARM	= 0,
	EMGRINDING_CLASS_CLOTH	= 1,

	EMGRINDING_CLASS_SIZE	= 2,
};

enum EMGRINDING_TYPE
{
	EMGRINDING_NONE				= 0,

	EMGRINDING_DAMAGE			= 1,
	EMGRINDING_DEFENSE			= 2,

	EMGRINDING_RESIST_FIRE		= 3,
	EMGRINDING_RESIST_ICE		= 4,
	EMGRINDING_RESIST_ELEC		= 5,
	EMGRINDING_RESIST_POISON	= 6,
	EMGRINDING_RESIST_SPIRIT	= 7,
	
	EMGRINDING_NSIZE			= 8
};

enum EMGRINDING_LEVEL
{
	EMGRINDING_LEVEL_NORMAL		= 0,
	EMGRINDING_LEVEL_HIGH		= 1,
	EMGRINDING_LEVEL_TOP		= 2,

	EMGRINDING_LEVEL_SIZE		= 3,
};

enum EMGRINDING_RS_FLAGS
{
	EMGRINDING_RS_FIRE		= 0x01,
	EMGRINDING_RS_ICE		= 0x02,
	EMGRINDING_RS_ELECTRIC	= 0x04,
	EMGRINDING_RS_POISON	= 0x08,
	EMGRINDING_RS_SPIRIT	= 0x10,

	EMGRINDING_RS_NSIZE		= 5,
	EMGRINDING_NO			= 2,
	EMGRINDING_RS_ALL		= (EMGRINDING_RS_FIRE|EMGRINDING_RS_ICE|EMGRINDING_RS_ELECTRIC|EMGRINDING_RS_POISON|EMGRINDING_RS_SPIRIT)
};

enum EMGRINDING_RS_INDEX
{
	EMGRINDING_RS_FIRE_INDEX		= 0,
	EMGRINDING_RS_ICE_INDEX			= 1,
	EMGRINDING_RS_ELECTRIC_INDEX	= 2,
	EMGRINDING_RS_POISON_INDEX		= 3,
	EMGRINDING_RS_SPIRIT_INDEX		= 4,

	EMGRINDING_RS_INDEX_NSIZE		= 5
};

EMGRINDING_RS_INDEX	GrindRsClassToIndex ( const EMGRINDING_RS_FLAGS emClass );
EMGRINDING_RS_FLAGS	GrindIndexToRsClass ( const EMGRINDING_RS_INDEX	emIndex );

struct SGRIND
{
	EMGRINDING_TYPE	emTYPE;

	DWORD			dwRESIST;
	int				nMin;
	int				nMax;

	SGRIND () :
		emTYPE(EMGRINDING_NONE),

		dwRESIST(NULL),
		nMin(0),
		nMax(0)
	{
	}
};

struct ITEM_COOLTIME
{
	enum { VERSION = 0x0001 };

	DWORD		dwID;		//	사용한 아이템 ID
	DWORD		dwCoolID;	//	쿨타임 ID ( 타입에 따라 다름 ) 
	__time64_t	tUseTime;	//	사용시간
	__time64_t	tCoolTime;	//	사용가능한 시간

	ITEM_COOLTIME()
		: dwID( 0 )
		, dwCoolID ( 0 )
		, tUseTime ( 0 )
		, tCoolTime ( 0 )
	{
	}
};


namespace COMMENT
{
	extern std::string ITEMLEVEL[LEVEL_NSIZE];
	extern std::string ITEMTYPE[ITEM_NSIZE];
	std::string ItemTypeName(EMITEM_TYPE Type);

	extern std::string ITEMSUIT[SUIT_NSIZE];
	extern std::string ITEMATTACK[ITEMATT_NSIZE];

	extern std::string ITEMDRUG[ITEM_DRUG_SIZE];

	extern std::string ITEMADDON[EMADD_SIZE];
	
	extern std::string ITEMVAR[EMVAR_SIZE];

	extern std::string ITEMVOL[EMVAR_SIZE]; // by 경대
	extern std::string ITEM_QUE_TYPE[QUESTION_SIZE]; // Question Item
	extern std::string ITEM_QUE_VAR1[QUESTION_SIZE];
	extern std::string ITEM_QUE_VAR2[QUESTION_SIZE];

	extern float ITEMVAR_SCALE[EMVAR_SIZE];
	bool IsITEMVAR_SCALE ( EMITEM_VAR emITEM_VAR );

	extern std::string GRINDING_TYPE[EMGRINDING_NSIZE];
	extern std::string GRINDING_RESIST[EMGRINDING_RS_NSIZE];
	extern std::string GRINDING_LEVEL[EMGRINDING_LEVEL_SIZE];
	extern std::string GRINDING_CLASS[EMGRINDING_CLASS_SIZE];

	extern std::string ITEM_RANDOM_OPT[EMR_OPT_SIZE];
	extern float ITEM_RANDOM_OPT_SCALE[EMR_OPT_SIZE];

	extern DWORD	ITEMCOLOR[LEVEL_NSIZE];

	extern std::string COOLTYPE[EMCOOL_SIZE];

	extern std::string ITEMSLOT_CHAR[SLOT_NSIZE_S_2];
};

#endif // GLITEMDEF_H_
