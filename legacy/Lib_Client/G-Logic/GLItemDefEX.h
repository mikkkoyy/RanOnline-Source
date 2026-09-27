#ifndef GLITEMDEF_EX_H_
#define GLITEMDEF_EX_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//official item type used for loading official item
enum EMEXITEM_TYPE
{
	EXITEM_SUIT					= 0,
	EXITEM_ARROW				= 1,	//	화살.
	EXITEM_CURE					= 2,	//	약품.
	EXITEM_SKILL				= 3,	//	스킬 습득 서적.
	EXITEM_RECALL				= 4,	//	귀환서.
	EXITEM_KEY					= 5,	//	인증서 ( 열쇠 ).
	EXITEM_GRINDING				= 6,	//	연마제
	EXITEM_CHARM				= 7,	//	부적.
	EXITEM_TICKET				= 8,	//	승차권.
	EXITEM_SKP_RESET			= 9,	//	스킬 포인트 리셋.
	EXITEM_STAT_RESET			= 10,	//	스텟 포인트 리셋.
	EXITEM_SKP_STAT_RESET		= 11,	//	스킬, 스텟 포인트 리셋.
	EXITEM_BOX					= 12,	//	선물 상자.
	EXITEM_CLEANSER				= 13,	//	세탁제.
	EXITEM_LOUDSPEAKER			= 14,	//	확성기.
	EXITEM_FIRECRACKER			= 15,	//	폭죽.
	EXITEM_CHARACTER_CARD		= 16,	//	캐릭터 추가 카드.
	EXITEM_INVEN_CARD			= 17,	//	인벤 확장 카드.
	EXITEM_STORAGE_CARD			= 18,	//	창고 확장 카드.
	EXITEM_STORAGE_CONNECT		= 19,	//	창고 연결 카드.
	EXITEM_PREMIUMSET			= 20,	//	프리미엄 세트.
	EXITEM_PRIVATEMARKET		= 21,	//	개인상점 개설 권리.
	EXITEM_RANDOMITEM			= 22,	//	랜덤 아이탬.
	EXITEM_DISJUNCTION			= 23,	//	코스툼 분리.
	EXITEM_HAIR					= 24,	//	헤어스타일 변경.
	EXITEM_FACE					= 25,	//	얼굴 변경.
	EXITEM_QITEM				= 26,	//	? 아이템.
	EXITEM_CD					= 27,	//	CD.
	EXITEM_2FRIEND				= 28,	//	친구에게.
	EXITEM_CLUBCALL				= 29,	//	클럽호출.
	EXITEM_HAIRSHOP				= 30,	//	헤어샾 이용권.
	EXITEM_RENAME				= 31,   //  이름변경 카드.
	EXITEM_HAIR_STYLE			= 32,	//  헤어스타일
	EXITEM_HAIR_COLOR			= 33,   //  헤어컬러
	EXITEM_REVIVE				= 34,	//  귀혼주
	EXITEM_PET_CARD				= 35,	//	펫 카드	// PetData
	EXITEM_PET_FOOD				= 36,	//	펫 먹이
	EXITEM_PET_RENAME			= 37,	//	팻 이름변경
	EXITEM_PET_COLOR			= 38,	//	팻 컬러변경
	EXITEM_PET_STYLE			= 39,	//	팻 스타일변경
	EXITEM_PET_SKILL			= 40,	//	팻 스킬
	EXITEM_SMS					= 41,	//	SMS문자 발송
	EXITEM_PET_REVIVE			= 42,	//	팻 부활카드
	EXITEM_ANTI_DISAPPEAR 		= 43,	//	소방주(소멸방지)
	EXITEM_REMODEL				= 44,	//	개조 기능
	EXITEM_VEHICLE				= 45,	//	탈것
	EXITEM_VEHICLE_OIL			= 46,	//  탈것 기름
	EXITEM_VIETNAM_ITEMGET		= 47,	//  베트남 탐닉 방지 아이템 획득
	EXITEM_VIETNAM_EXPGET		= 48,	//  베트남 탐닉 방지 경험치 획득	
	EXITEM_GENDER_CHANGE		= 49,	//  성별 변경 카드 
	EXITEM_GARBAGE_CARD			= 50,	//	휴지통 카드	
	EXITEM_TELEPORT_CARD		= 51,	//	소환서	
	EXITEM_PET_SKIN_PACK		= 52,	//  펫 스킨 팩
	EXITEM_FACE_STYLE			= 53,	//	얼굴스타일
	EXITEM_TAXI_CARD			= 54,	//	택시카드
	EXITEM_MATERIALS			= 55,	//	재료아이템
	EXITEM_NPC_RECALL			= 56,	//	NPC소환 카드
	EXITEM_BULLET				= 57,
	EXITEM_FOOD					= 58,
	EXITEM_CARD_DUALPETSKILL	= 59,
	EXITEM_BEADS_STRENGTHEN		= 60,
	EXITEM_BEADS_RELAXANT		= 61,
	EXITEM_CARD_BIKECOLOR		= 62,
	EXITEM_CARD_BIKEBOOST		= 63,
	EXITEM_OBLIVION_NEW			= 64,
	EXITEM_COSTUME_COLOR		= 65,
	EXITEM_CARD_MAILBOX			= 66,
	EXITEM_POINTCARD_A			= 67,
	EXITEM_POINTCARD_B			= 68,
	EXITEM_CRAFTING_BOOK		= 69,
	EXITEM_RESET_CRAFTING			= 70,
	EXITEM_PERSONAL_STORE_SEARCH	= 71,

	EXITEM_NSIZE				= 72,	//	종류갯수.
};

#endif // GLITEMDEF_EX_H_
