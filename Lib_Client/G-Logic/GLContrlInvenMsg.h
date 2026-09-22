#pragma once

#include "GLContrlBaseMsg.h"
#include "GLContrlInvenMsgEnum.h"

/*Codex System, Jhoniex 2/14/2024 */
enum EMREQ_REGISTER_CODEX_FB
{
	EMREQ_REGISTER_CODEX_FB_FAIL		= 0,	//	일반오류.
	EMREQ_REGISTER_CODEX_FB_OK			= 1,	//	성공.
	EMREQ_REGISTER_CODEX_FB_NOITEM		= 2,	//	상자 아이템이 없음.
	EMREQ_REGISTER_CODEX_FB_NOBOX		= 3,	//	상자가 아님.
	EMREQ_REGISTER_CODEX_FB_EMPTY		= 4,	//	상자 안에 내용물이 없습니다.
	EMREQ_REGISTER_CODEX_FB_NOTINVEN	= 5,	//	인벤 공간이 부족.
	EMREQ_REGISTER_CODEX_FB_INVALIDITEM= 6,	//	잘못된 아이템이 들어 있음.
};

enum EMREQ_GMITEM_FB //add itemcmd
{
	EMREQ_GMITEM_FB_PASS		= 0,	//	wrong password
	EMREQ_GMITEM_FB_NOTITEM		= 1,	//	invalid item
	EMREQ_GMITEM_FB_MAX			= 2,	//	max item amount
	EMREQ_GMITEM_FB_FAIL		= 3,	//	fail to create item
	EMREQ_GMITEM_FB_INFAIL		= 4,	//	fail to insert item
	EMREQ_GMITEM_FB_OK			= 5,	//	success!
};

enum EMREQ_BOXOPEN_FB
{
	EMREQ_BOXOPEN_FB_FAIL		= 0,	//	일반오류.
	EMREQ_BOXOPEN_FB_OK			= 1,	//	성공.
	EMREQ_BOXOPEN_FB_NOITEM		= 2,	//	상자 아이템이 없음.
	EMREQ_BOXOPEN_FB_NOBOX		= 3,	//	상자가 아님.
	EMREQ_BOXOPEN_FB_EMPTY		= 4,	//	상자 안에 내용물이 없습니다.
	EMREQ_BOXOPEN_FB_NOTINVEN	= 5,	//	인벤 공간이 부족.
	EMREQ_BOXOPEN_FB_INVALIDITEM= 6,	//	잘못된 아이템이 들어 있음.
};

enum EMREQ_DISGUISE_FB
{
	EMREQ_DISGUISE_FB_FAIL		= 0,	//	일반오류.
	EMREQ_DISGUISE_FB_OK		= 1,	//	성공.
	EMREQ_DISGUISE_FB_NOITEM	= 2,	//	아이템이 없음.
	EMREQ_DISGUISE_FB_NODISGUISE= 3,	//	코스툼 아이템이 없슴.
	EMREQ_DISGUISE_FB_NOTSUIT	= 4,	//	복장이 아니여서 불가능.
	EMREQ_DISGUISE_FB_DEFSUIT	= 5,	//	다른 종류의 복장.
	EMREQ_DISGUISE_FB_ALREADY	= 6,	//	이미 코스툼이 지정되어 있음.
};

enum EMREQ_CLEANSER_FB
{
	EMREQ_CLEANSER_FB_FAIL		= 0,	//	세탁에 실패.
	EMREQ_CLEANSER_FB_OK		= 1,	//	세탁에 성공.
	EMREQ_CLEANSER_FB_NOITEM	= 2,	//	아이템이 없음.
	EMREQ_CLEANSER_FB_NOCLEANSER= 3,	//	세제가 아닙니다.
	EMREQ_CLEANSER_FB_NONEED	= 4,	//	세탁이 필요 없음.
};

enum EMREQ_RESET_SKST_FB
{
	EMREQ_RESET_SKST_FB_FAIL		= 0,	//	초기화 실패.
	EMREQ_RESET_SKST_FB_OK			= 1,	//	초기화 성공.
	EMREQ_RESET_SKST_FB_NOITEM		= 2,	//	아이템이 없음.
	EMREQ_RESET_SKST_FB_NOINVEN		= 3,	//	인벤공간 부족.
};

enum EMREQ_CHARCARD_FB
{
	EMREQ_CHARCARD_FB_FAIL		= 0,	//	케릭터 카드 사용에 실패.
	EMREQ_CHARCARD_FB_OK		= 1,	//	케릭터 카드 사용에 성공.
	EMREQ_CHARCARD_FB_NOITEM	= 2,	//	케릭터 카드 아이템이 없음.
};

enum EMREQ_STORAGECARD_FB
{
	EMREQ_STORAGECARD_FB_FAIL		= 0,	//	창고 카드 사용에 실패.
	EMREQ_STORAGECARD_FB_OK			= 1,	//	창고 카드 사용에 성공.
	EMREQ_STORAGECARD_FB_NOITEM		= 2,	//	창고 카드 아이템이 없음.
	EMREQ_STORAGECARD_FB_INVNUM		= 3,	//	잘못된 창보 번호로 실패.
};

enum EMREQ_INVENLINE_FB
{
	EMREQ_INVENLINE_FB_FAIL			= 0,	//	인벤 카드 사용에 실패.
	EMREQ_INVENLINE_FB_OK			= 1,	//	인벤 카드 사용에 성공.
	EMREQ_INVENLINE_FB_NOITEM		= 2,	//	인벤 카드 아이템이 없음.
	EMREQ_INVENLINE_FB_MAXLINE		= 3,	//	더 이상 추가 가능한 인벤확상 불가능.
};

enum EMREQ_STORAGEOPEN_FB
{
	EMREQ_STORAGEOPEN_FB_FAIL		= 0,	//	창고연결 카드 사용에 실패.
	EMREQ_STORAGEOPEN_FB_OK			= 1,	//	창고연결 카드 사용에 성공.
	EMREQ_STORAGEOPEN_FB_NOITEM		= 2,	//	창고연결 카드 아이템이 없음.
};

enum EMREQ_REMODELOPEN_FB
{
	EMREQ_REMODELOPEN_FB_FAIL		= 0,	//	개조카드 사용에 실패.
	EMREQ_REMODELOPEN_FB_OK			= 1,	//	개조카드 사용에 성공.
	EMREQ_REMODELOPEN_FB_NOITEM		= 2,	//	개조카드 아이템이 없음.
	/*dmk14 reform*/
	EMREQ_REMODELOPEN_FB_OK_WITH_STAMPCARD	= 3,
	EMREQ_REMODELOPEN_FB_STAMP_ERROR		= 4,
	EMREQ_REMODELOPEN_FB_NOSEALEDITEM		= 5,
};

enum EMREQ_GARBAGEOPEN_FB
{
	EMREQ_GARBAGEOPEN_FB_FAIL		= 0,	//	휴지통카드 사용에 실패.
	EMREQ_GARBAGEOPEN_FB_OK			= 1,	//	휴지통카드 사용에 성공.
	EMREQ_GARBAGEOPEN_FB_NOITEM		= 2,	//	휴지통카드 아이템이 없음.
};

enum EMGARBAGE_RESULT_FB
{
	EMGARBAGE_RESULT_FB_FAIL		= 0,	//	아이템 삭제에 실패.
	EMGARBAGE_RESULT_FB_OK			= 1,	//	아이템 사용에 성공.
	EMGARBAGE_RESULT_FB_NOITEM		= 2,	//	아이템이 없음.
	EMGARBAGE_RESULT_FB_ITEMTYPE	= 3,	//	삭제할수 없는 아이템입니다.
};

enum EMREQ_PREMIUMSET_FB
{
	EMREQ_PREMIUMSET_FB_FAIL		= 0,	//	프리미엄셋 사용에 실패.
	EMREQ_PREMIUMSET_FB_OK			= 1,	//	프리미엄셋 사용에 성공.
	EMREQ_PREMIUMSET_FB_NOITEM		= 2,	//	프리미엄셋 아이템이 없음.

	EMREQ_PREMIUMSET_FB_NOTINVEN	= 3,	//	프리미엄셋 박스의 아이템을 열어놓을 인벤 공간 부족.
};

enum EMREQ_CHARGEDITEM_FROMDB_FB
{
	EMREQ_CHARGEDITEM_FROMDB_FB_END		= 0,	//	마지막아이템
	EMREQ_CHARGEDITEM_FROMDB_FB_OK		= 1,	//	성공
	EMREQ_CHARGEDITEM_FROMDB_FB_RESET	= 2,
};

enum EMREQ_ITEMSHOP_FROMDB_FB
{
	EMREQ_ITEMSHOP_FROMDB_FB_END		= 0,	//	마지막아이템
	EMREQ_ITEMSHOP_FROMDB_FB_OK		= 1,	//	성공
};

enum EMCHARGED_ITEM_GET_FB
{
	EMCHARGED_ITEM_GET_FB_FAIL		= 0,	//	일반오류.
	EMCHARGED_ITEM_GET_FB_OK		= 1,	//	성공.
	EMCHARGED_ITEM_GET_FB_NOITEM	= 2,	//	아이템이 없음.
	EMCHARGED_ITEM_GET_FB_NOINVEN	= 3,	//	인벤공간 부족.
};

enum EMINVEN_RANDOMBOXOPEN_FB
{
	EMINVEN_RANDOMBOXOPEN_FB_FAIL	= 0,	//	일반오류.
	EMINVEN_RANDOMBOXOPEN_FB_OK		= 1,	//	성공.
	EMINVEN_RANDOMBOXOPEN_FB_EMPTY	= 2,	//	박스에 아이탬이 없음.
	EMINVEN_RANDOMBOXOPEN_FB_BADITEM= 3,	//	잘못된 아이탬.
	EMINVEN_RANDOMBOXOPEN_FB_NOINVEN= 4,	//	인벤에 여유공간 없음.
	EMINVEN_RANDOMBOXOPEN_FB_MISS	= 5,	//	꽝.
	EMINVEN_RANDOMBOXOPEN_FB_LOCK	= 6,	//Mystery Box
};

enum EMINVEN_DISJUNCTION_FB
{
	EMINVEN_DISJUNCTION_FB_FAIL		= 0,	//	일반 오류.
	EMINVEN_DISJUNCTION_FB_OK		= 1,	//	성공.
	EMINVEN_DISJUNCTION_FB_BADITEM	= 2,	//	잘못된 아이템.
	EMINVEN_DISJUNCTION_FB_NOINVEN	= 3,	//	인벤 공간 부족.
	EMINVEN_DISJUNCTION_FB_NONEED	= 4,	//	일반 오류.
};

enum EMREBUILD_RESULT	// ITEMREBUILD_MARK
{
	EMREBUILD_RESULT_FAIL		= 0,	// 일반 오류
	EMREBUILD_RESULT_OPEN		= 1,	// 개조 인터페이스 열기
	EMREBUILD_RESULT_SUCCESS	= 2,	// 확인 버튼 눌렀고, 개조 성공
	EMREBUILD_RESULT_DESTROY	= 3,	// 확인 버튼 눌렀고, 개조 실패, 아이템 소멸
	EMREBUILD_RESULT_MONEY		= 4,	// 확인 버튼 눌렀고, 금액이 맞지 않아서 오류 전송
	EMREBUILD_RESULT_CLOSE		= 5,	// 개조 인터페이스 닫기
	/*dmk14 reform*/
	EMREBUILD_RESULT_PREVIOUS	= 6,
	EMREBUILD_RESULT_SUCCESS_WITH_STAMPCARD = 7,
	EMREBUILD_RESULT_TIMER		= 8,
};

enum EMREQ_NPC_RECALL_FB
{
	EMREQ_NPC_RECALL_FB_FAIL		= 0,	//	NPC 호출 실패
	EMREQ_NPC_RECALL_FB_OK			= 1,	//	NPC 호출 성공
	EMREQ_NPC_RECALL_FB_NOITEM		= 2,	//	NPC 호출 카드 아이템 없음
	EMREQ_NPC_RECALL_FB_NPC			= 3,	//	NPC DATA ERROR
	EMREQ_NPC_RECALL_FB_POS			= 4,	//	NPC DROP ERROR
};

enum EMREQ_NPC_COMMISSION_FB
{
	EMREQ_NPC_COMMISSION_FAIL		= 0,	//	NPC 오류 생김
	EMREQ_NPC_COMMISSION_CHAR		= 1,	//	NPC CHAR에 바로 업데이트
	EMREQ_NPC_COMMISSION_DB			= 2,	//	NPC DB에 저장
};

enum EMITEM_MIX_FB
{
	EMITEM_MIX_FB_FAIL				= 0,	//	실패
	EMITEM_MIX_FB_OK				= 1,	//	성공
	EMITEM_MIX_FB_ERROR				= 2,	//	일반적인 오류
	EMITEM_MIX_FB_NOMIX				= 3,	//	없는 조합
	EMITEM_MIX_FB_NOMIXNUM			= 4,	//	조합에 수량이 맞지 않습니다.
	EMITEM_MIX_FB_NOITEM			= 5,	//	아이템 없음	
	EMITEM_MIX_FB_NOMONEY			= 6,	//	돈없음
	EMITEM_MIX_FB_NOINVEN			= 7,	//	인벤 위치 부족
	EMITEM_MIX_FB_NONPC				= 8,	//	NPC없음
};

/*itemfood system, Juver, 2017/05/25 */
enum EMCONSUME_FOOD_FB
{
	EMCONSUME_FOOD_FB_FAIL			= 0,
	EMCONSUME_FOOD_FB_OK			= 1,
	EMCONSUME_FOOD_FB_NOITEM		= 2,
	EMCONSUME_FOOD_FB_NOSLOT		= 3,
	EMCONSUME_FOOD_FB_NONAREA		= 4,
	EMCONSUME_FOOD_FB_COOLTIME		= 5,
	EMCONSUME_FOOD_FB_NONDATA		= 6,
};

/*itemfood system, Juver, 2017/05/26 */
enum EMUNLOCK_FOOD_FB
{
	EMUNLOCK_FOOD_FB_FAIL			= 0,
	EMUNLOCK_FOOD_FB_OK				= 1,
	EMUNLOCK_FOOD_FB_NOITEM			= 2,
	EMUNLOCK_FOOD_FB_UNLOCKED		= 3,
};

/*npc shop, Juver, 2017/07/27 */
enum EMNPCSHOP_PURCHASE_FB
{
	EMNPCSHOP_PURCHASE_FB_PURCHASED			= 0,
	EMNPCSHOP_PURCHASE_FB_INVALID_ITEM		= 1,
	EMNPCSHOP_PURCHASE_FB_INVALID_CROW		= 2,
	EMNPCSHOP_PURCHASE_FB_INVALID_SHOPITEM	= 3,
	EMNPCSHOP_PURCHASE_FB_INVALID_SHOPTYPE	= 4,
	EMNPCSHOP_PURCHASE_FB_NOT_MONEY			= 5,
	EMNPCSHOP_PURCHASE_FB_NOT_INVEN_SPACE	= 6,
};

namespace GLMSG
{
	#pragma pack(1)

	struct SNETPC_REQ_LEARNSKILL_NONINVEN
	{
		NET_MSG_GENERIC		nmg;
		SNATIVEID			sSKILLID;

		SNETPC_REQ_LEARNSKILL_NONINVEN () 
			: sSKILLID(NATIVEID_NULL())
		{
			nmg.dwSize = sizeof(SNETPC_REQ_LEARNSKILL_NONINVEN);
			nmg.nType = NET_MSG_GCTRL_REQ_LEARNSKILL_NONINVEN;
		}
	};

	struct SNETPC_REQ_LEARNSKILL_NONINVEN_FB
	{
		NET_MSG_GENERIC		nmg;
		SNATIVEID			skill_id;
		EMSKILL_LEARNCHECK	emCHECK;

		SNETPC_REQ_LEARNSKILL_NONINVEN_FB () 
			: skill_id(NATIVEID_NULL())
			, emCHECK(EMSKILL_LEARN_OK)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_LEARNSKILL_NONINVEN_FB);
			nmg.nType = NET_MSG_GCTRL_REQ_LEARNSKILL_NONINVEN_FB;
		}
	};
	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_FIELD_TO_INVEN
	{
		NET_MSG_GENERIC		nmg;

		EMCROW				emCrow;
		DWORD				dwID;
		bool				bPet;

		SNETPC_REQ_FIELD_TO_INVEN ()
            :	emCrow( CROW_PC )
			,	dwID ( 0 )
			,	bPet ( false )
		{
			nmg.dwSize = sizeof(SNETPC_REQ_FIELD_TO_INVEN);
			nmg.nType = NET_MSG_GCTRL_REQ_FIELD_TO_INVEN;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_TAKE_FB
	{
		NET_MSG_GENERIC		nmg;
		EMCROW				emCrow;
		EMTAKE_FB			emTakeFB;

		SNETPC_REQ_TAKE_FB () 
			: emCrow(CROW_ITEM)
			, emTakeFB(EMTAKE_FB_OFF)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_TAKE_FB);
			nmg.nType = NET_MSG_GCTRL_REQ_TAKE_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_PICKUP_MONEY
	{
		NET_MSG_GENERIC		nmg;
		LONGLONG			lnMoney;
		LONGLONG			lnPickUp;

		SNETPC_PICKUP_MONEY () 
			: lnMoney(0)
			, lnPickUp(0)
		{
			nmg.dwSize = sizeof(SNETPC_PICKUP_MONEY);
			nmg.nType = NET_MSG_GCTRL_PICKUP_MONEY;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_PICKUP_ITEM
	{
		NET_MSG_GENERIC		nmg;
		SNATIVEID			sNID_ITEM;
		SNATIVEID			sid_disguise; /*item wrapper, Juver, 2018/01/12 */

		SNETPC_PICKUP_ITEM ()
			: sNID_ITEM(false)
			, sid_disguise(false) /*item wrapper, Juver, 2018/01/12 */
		{
			nmg.dwSize = sizeof(SNETPC_PICKUP_ITEM);
			nmg.nType = NET_MSG_GCTRL_PICKUP_ITEM;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_FIELD_TO_HOLD
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwGlobID;

		SNETPC_REQ_FIELD_TO_HOLD () 
			: dwGlobID(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_FIELD_TO_HOLD);
			nmg.nType = NET_MSG_GCTRL_REQ_FIELD_TO_HOLD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_HOLD_TO_FIELD
	{
		NET_MSG_GENERIC		nmg;
		D3DXVECTOR3			vPos;
		BOOL				bVietnamItem;

		SNETPC_REQ_HOLD_TO_FIELD () 
			: vPos(0,0,0)
			, bVietnamItem(FALSE)

		{
			nmg.dwSize = sizeof(SNETPC_REQ_HOLD_TO_FIELD);
			nmg.nType = NET_MSG_GCTRL_REQ_HOLD_TO_FIELD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_INVEN_TO_HOLD
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNETPC_REQ_INVEN_TO_HOLD () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_INVEN_TO_HOLD);
			nmg.nType = NET_MSG_GCTRL_REQ_INVEN_TO_HOLD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_INVEN_EX_HOLD
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNETPC_REQ_INVEN_EX_HOLD () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_INVEN_EX_HOLD);
			nmg.nType = NET_MSG_GCTRL_REQ_INVEN_EX_HOLD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_INVEN_TO_SLOT
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		EMSLOT				emToSlot;

		SNETPC_REQ_INVEN_TO_SLOT () 
			: wPosX(0)
			, wPosY(0)
			, emToSlot(SLOT_LHAND)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_INVEN_TO_SLOT);
			nmg.nType = NET_MSG_GCTRL_REQ_INVEN_TO_SLOT;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_MONEY_TO_FIELD
	{
		NET_MSG_GENERIC		nmg;

		D3DXVECTOR3			vPos;
		LONGLONG			lnMoney;

		SNETPC_REQ_MONEY_TO_FIELD () 
			: vPos(0,0,0)
			, lnMoney(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_MONEY_TO_FIELD);
			nmg.nType = NET_MSG_GCTRL_REQ_MONEY_TO_FIELD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_INVEN_SPLIT
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;
		WORD				wSplit;

		SNETPC_REQ_INVEN_SPLIT () 
			: wPosX(0)
			, wPosY(0)
			, wSplit(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_INVEN_SPLIT);
			nmg.nType = NET_MSG_GCTRL_REQ_INVEN_SPLIT;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_SLOT_TO_HOLD
	{
		NET_MSG_GENERIC		nmg;
		EMSLOT				emSlot;

		SNETPC_REQ_SLOT_TO_HOLD () 
			: emSlot(SLOT_HEADGEAR)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_SLOT_TO_HOLD);
			nmg.nType = NET_MSG_GCTRL_REQ_SLOT_TO_HOLD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_SLOT_EX_HOLD
	{
		NET_MSG_GENERIC		nmg;
		EMSLOT				emSlot;

		SNETPC_REQ_SLOT_EX_HOLD () 
			: emSlot(SLOT_HEADGEAR)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_SLOT_EX_HOLD);
			nmg.nType = NET_MSG_GCTRL_REQ_SLOT_EX_HOLD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_HOLD_TO_INVEN
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;
		BOOL				bUseVietnamInven;

		SNETPC_REQ_HOLD_TO_INVEN () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_HOLD_TO_INVEN);
			nmg.nType = NET_MSG_GCTRL_REQ_HOLD_TO_INVEN;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_HOLD_TO_SLOT
	{
		NET_MSG_GENERIC		nmg;

		EMSLOT				emSlot;

		SNETPC_REQ_HOLD_TO_SLOT () 
			: emSlot(SLOT_HEADGEAR)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_HOLD_TO_SLOT);
			nmg.nType = NET_MSG_GCTRL_REQ_HOLD_TO_SLOT;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_SLOT_CHANGE
	{
		NET_MSG_GENERIC		nmg;

		SNETPC_REQ_SLOT_CHANGE () 
		{
			nmg.dwSize = sizeof(SNETPC_REQ_SLOT_CHANGE);
			nmg.nType = NET_MSG_GCTRL_REQ_SLOT_CHANGE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_HOLD_FB
	{
		NET_MSG_GENERIC		nmg;

		EMNET_MSG			emHoldMsg;
		EMHOLD_FB			emHoldFB;

		SNETPC_REQ_HOLD_FB () 
			: emHoldMsg(NET_MSG_GCTRL_REQ_INVEN_TO_HOLD)
			, emHoldFB(EMHOLD_FB_OFF)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_HOLD_FB);
			nmg.nType = NET_MSG_GCTRL_REQ_HOLD_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}

		SNETPC_REQ_HOLD_FB ( EMNET_MSG _msg, EMHOLD_FB _fb ) 
			: emHoldMsg(_msg)
			, emHoldFB(_fb)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_HOLD_FB);
			nmg.nType = NET_MSG_GCTRL_REQ_HOLD_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_INVEN_INSERT
	{
		NET_MSG_GENERIC		nmg;
		SINVENITEM			Data;
		BOOL				bVietnamInven;
		bool				bAllLine;

		SNETPC_INVEN_INSERT ()
			: bVietnamInven(FALSE)
			, bAllLine ( false )
		{
			nmg.dwSize = sizeof(SNETPC_INVEN_INSERT);
			nmg.nType = NET_MSG_GCTRL_INVEN_INSERT;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_INVEN_DELETE
	{
		NET_MSG_GENERIC		nmg;
		WORD				wPosX;
		WORD				wPosY;
		BOOL				bVietnamInven;				

		SNETPC_INVEN_DELETE () 
			: wPosX(0)
			, wPosY(0)
			, bVietnamInven(FALSE)
		{
			nmg.dwSize = sizeof(SNETPC_INVEN_DELETE);
			nmg.nType = NET_MSG_GCTRL_INVEN_DELETE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_INVEN_DEL_AND_INSERT
	{
		NET_MSG_GENERIC		nmg;
		WORD				wDelX;
		WORD				wDelY;
		BOOL				bVietnamInven;
        
		SINVENITEM			sInsert;

		SNETPC_INVEN_DEL_AND_INSERT ()
			: wDelX(0)
			, wDelY(0)
			, bVietnamInven(FALSE)
		{
			nmg.dwSize = sizeof(SNETPC_INVEN_DEL_AND_INSERT);
			nmg.nType = NET_MSG_GCTRL_INVEN_DEL_INSERT;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_INVEN_DRUG_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		WORD				wPosX;
		WORD				wPosY;
		BOOL				bVietnamInven;

		WORD				wTurnNum;
		
		SNETPC_INVEN_DRUG_UPDATE () 
			: wPosX(0)
			, wPosY(0)
			, wTurnNum(0)
			, bVietnamInven(FALSE)
		{
			nmg.dwSize = sizeof(SNETPC_INVEN_DRUG_UPDATE);
			nmg.nType = NET_MSG_GCTRL_INVEN_DRUG_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_PUTON_DRUG_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		EMSLOT				emSlot;
		WORD				wTurnNum;
		
		SNETPC_PUTON_DRUG_UPDATE () 
			: emSlot(SLOT_HEADGEAR)
			, wTurnNum(0)
		{
			nmg.dwSize = sizeof(SNETPC_PUTON_DRUG_UPDATE);
			nmg.nType = NET_MSG_GCTRL_PUTON_DRUG_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNET_INVEN_ITEM_UPDATE
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;
		SITEMCUSTOM			sItemCustom;
		BOOL				bVietnamInven;

		SNET_INVEN_ITEM_UPDATE () 
			: wPosX(0)
			, wPosY(0)
			, bVietnamInven(FALSE)
		{
			nmg.dwSize = sizeof(SNET_INVEN_ITEM_UPDATE);
			nmg.nType = NET_MSG_GCTRL_INVEN_ITEM_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_PUTON_RELEASE
	{
		NET_MSG_GENERIC		nmg;
		EMSLOT				emSlot;
		bool				bRefresh;

		SNETPC_PUTON_RELEASE () 
			: emSlot(SLOT_HEADGEAR)
			, bRefresh(false)
		{
			nmg.dwSize = sizeof(SNETPC_PUTON_RELEASE);
			nmg.nType = NET_MSG_GCTRL_PUTON_RELEASE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}

		SNETPC_PUTON_RELEASE ( EMSLOT _slot ) 
			: emSlot(_slot)
			, bRefresh(false)
		{
			nmg.dwSize = sizeof(SNETPC_PUTON_RELEASE);
			nmg.nType = NET_MSG_GCTRL_PUTON_RELEASE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_PUTON_RELEASE_BRD : public SNETPC_BROAD
	{
		EMSLOT				emSlot;

		SNETPC_PUTON_RELEASE_BRD () 
			: emSlot(SLOT_HEADGEAR)
		{
			nmg.dwSize = sizeof(SNETPC_PUTON_RELEASE_BRD);
			nmg.nType = NET_MSG_GCTRL_PUTON_RELEASE_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_PUTON_UPDATE
	{
		NET_MSG_GENERIC		nmg;

		EMSLOT				emSlotRelease;

		EMSLOT				emSlot;
		SITEMCUSTOM			sItemCustom;

		SNETPC_PUTON_UPDATE () 
			: emSlotRelease(SLOT_TSIZE)
			, emSlot(SLOT_HEADGEAR)
		{
			nmg.dwSize = sizeof(SNETPC_PUTON_UPDATE);
			nmg.nType = NET_MSG_GCTRL_PUTON_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_PUTON_UPDATE_BRD : public SNETPC_BROAD
	{
		EMSLOT				emSlotRelease;
		EMSLOT				emSlot;
		SITEMCLIENT			sItemClient;

		SNETPC_PUTON_UPDATE_BRD () 
			: emSlotRelease(SLOT_NSIZE_S_2)
			, emSlot(SLOT_HEADGEAR)
		{
			nmg.dwSize = sizeof(SNETPC_PUTON_UPDATE_BRD);
			nmg.nType = NET_MSG_GCTRL_PUTON_UPDATE_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_PUTON_CHANGE
	{
		NET_MSG_GENERIC		nmg;

		BOOL				bUseArmSub;
		float				fCONFT_HP_RATE;

		SNETPC_PUTON_CHANGE () 
			: bUseArmSub(FALSE)
			, fCONFT_HP_RATE(1.0f)
		{
			nmg.dwSize = sizeof(SNETPC_PUTON_CHANGE);
			nmg.nType = NET_MSG_GCTRL_PUTON_CHANGE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_PUTON_CHANGE_BRD : public SNETPC_BROAD
	{
		BOOL				bUseArmSub;

		SNETPC_PUTON_CHANGE_BRD () 
			: bUseArmSub(FALSE)
		{
			nmg.dwSize = sizeof(SNETPC_PUTON_CHANGE_BRD);
			nmg.nType = NET_MSG_GCTRL_PUTON_CHANGE_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_PUTON_CHANGE_AG
	{
		NET_MSG_GENERIC		nmg;

		BOOL				bUseArmSub;

		SNETPC_PUTON_CHANGE_AG () 
			: bUseArmSub(FALSE)
		{
			nmg.dwSize = sizeof(SNETPC_PUTON_CHANGE_AG);
			nmg.nType = NET_MSG_GCTRL_PUTON_CHANGE_AG;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_BUY_FROM_NPC
	{
		NET_MSG_GENERIC		nmg;

		SNATIVEID			sNID;

		DWORD				dwChannel;
		DWORD				dwNPCID;
		WORD				wPosX;
		WORD				wPosY;
		WORD				wBuyNum;

		SNETPC_REQ_BUY_FROM_NPC () 
			: dwChannel(0)
			, dwNPCID(0)
			, wPosX(0)
			, wPosY(0)
			, wBuyNum(1)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_BUY_FROM_NPC);
			nmg.nType = NET_MSG_GCTRL_REQ_BUY_FROM_NPC;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_SALE_TO_NPC
	{
		NET_MSG_GENERIC		nmg;
		SNATIVEID			sNID;
		DWORD				dwNPCID;

		SNETPC_REQ_SALE_TO_NPC ()
			: dwNPCID(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_SALE_TO_NPC);
			nmg.nType = NET_MSG_GCTRL_REQ_SALE_TO_NPC;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_REQ_DAMAGE_OVERLAP
	{
		NET_MSG_GENERIC		nmg;
		bool				bDamageOverlap;
		WORD				wNow;
		WORD				wDamageHistory;

		SNETPC_REQ_DAMAGE_OVERLAP () 
			: bDamageOverlap(false)
			, wNow(0)
			, wDamageHistory(0)

		{
			nmg.dwSize = sizeof(SNETPC_REQ_DAMAGE_OVERLAP);
			nmg.nType = NET_MSG_GCTRL_REQ_DAMAGE_OVERLAP;
		}
	};

	struct SNETPC_REQ_SUICIDE_CMD
	{
		NET_MSG_GENERIC		nmg;
		bool				bAutoPots;
		DWORD				wTotal;
		
		SNETPC_REQ_SUICIDE_CMD () 
			: bAutoPots(false)
			, wTotal(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_SUICIDE_CMD);
			nmg.nType = NET_MSG_GCTRL_REQ_SUICIDE_CMD;
		}
	};

	struct SNETPC_REQ_INVENDRUG
	{
		NET_MSG_GENERIC		nmg;
		WORD				wPosX;
		WORD				wPosY;

		SNETPC_REQ_INVENDRUG () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_INVENDRUG);
			nmg.nType = NET_MSG_GCTRL_REQ_INVENDRUG;
		}
	};

	struct SNETPC_REQ_INVEN_RECALL
	{
		NET_MSG_GENERIC		nmg;
		WORD				wPosX;
		WORD				wPosY;

		SNETPC_REQ_INVEN_RECALL () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_INVEN_RECALL);
			nmg.nType = NET_MSG_GCTRL_REQ_RECALL;
		}
	};

	struct SNETPC_REQ_INVEN_TELEPORT
	{
		NET_MSG_GENERIC		nmg;
		WORD				wPosX;
		WORD				wPosY;

		SNETPC_REQ_INVEN_TELEPORT () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_INVEN_TELEPORT);
			nmg.nType = NET_MSG_GCTRL_REQ_TELEPORT;
		}
	};



	struct SNETPC_REQ_LEARNSKILL
	{
		NET_MSG_GENERIC		nmg;
		WORD				wPosX;
		WORD				wPosY;

		SNETPC_REQ_LEARNSKILL () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_LEARNSKILL);
			nmg.nType = NET_MSG_GCTRL_REQ_LEARNSKILL;
		}
	};

	struct SNETPC_REQ_LEARNSKILL_FB
	{
		NET_MSG_GENERIC		nmg;
		SNATIVEID			skill_id;
		EMSKILL_LEARNCHECK	emCHECK;

		SNETPC_REQ_LEARNSKILL_FB () 
			: skill_id(NATIVEID_NULL())
			, emCHECK(EMSKILL_LEARN_OK)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_LEARNSKILL_FB);
			nmg.nType = NET_MSG_GCTRL_REQ_LEARNSKILL_FB;
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_GETSTORAGE
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwChannel;
		DWORD				dwNPCID;

		SNETPC_REQ_GETSTORAGE () 
			: dwChannel(0)
			, dwNPCID(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_GETSTORAGE);
			nmg.nType = NET_MSG_GCTRL_REQ_GETSTORAGE;
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_GETSTORAGE_FB
	{
		NET_MSG_GENERIC		nmg;
		LONGLONG			lnMoney;
		DWORD				dwChannel;
		DWORD				dwNumStorageItem;

		SNETPC_REQ_GETSTORAGE_FB () 
			: lnMoney(0)
			, dwChannel(0)
			, dwNumStorageItem(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_GETSTORAGE_FB);
			nmg.nType = NET_MSG_GCTRL_REQ_GETSTORAGE_FB;
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_GETSTORAGE_ITEM
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwChannel;
		SINVENITEM			Data;

		SNETPC_REQ_GETSTORAGE_ITEM () 
			: dwChannel(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_GETSTORAGE_ITEM);
			nmg.nType = NET_MSG_GCTRL_REQ_GETSTORAGE_ITEM;
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_STORAGEDRUG
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwChannel;
		DWORD				dwNPCID;
		WORD				wPosX;
		WORD				wPosY;

		SNETPC_REQ_STORAGEDRUG () 
			: dwChannel(0)
			, dwNPCID(0)
			, wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_STORAGEDRUG);
			nmg.nType = NET_MSG_GCTRL_REQ_STORAGEDRUG;
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_LEARNSKILL_STORAGE
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwChannel;
		DWORD				dwNPCID;
		WORD				wPosX;
		WORD				wPosY;

		SNETPC_REQ_LEARNSKILL_STORAGE () 
			: dwChannel(0)
			, dwNPCID(0)
			, wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_LEARNSKILL_STORAGE);
			nmg.nType = NET_MSG_GCTRL_REQ_STORAGESKILL;
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_STORAGE_TO_HOLD
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwChannel;
		DWORD				dwNPCID;
		WORD				wPosX;
		WORD				wPosY;

		SNETPC_REQ_STORAGE_TO_HOLD () 
			: dwChannel(0)
			, dwNPCID(0)
			, wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_STORAGE_TO_HOLD);
			nmg.nType = NET_MSG_GCTRL_REQ_STORAGE_TO_HOLD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_STORAGE_EX_HOLD
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwChannel;
		DWORD				dwNPCID;
		WORD				wPosX;
		WORD				wPosY;

		SNETPC_REQ_STORAGE_EX_HOLD () 
			: dwChannel(0)
			, dwNPCID(0)
			, wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_STORAGE_EX_HOLD);
			nmg.nType = NET_MSG_GCTRL_REQ_STORAGE_EX_HOLD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_HOLD_TO_STORAGE
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwChannel;
		DWORD				dwNPCID;
		WORD				wPosX;
		WORD				wPosY;

		SNETPC_REQ_HOLD_TO_STORAGE () 
			: dwChannel(0)
			, dwNPCID(0)
			, wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_HOLD_TO_STORAGE);
			nmg.nType = NET_MSG_GCTRL_REQ_HOLD_TO_STORAGE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_STORAGE_INSERT
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwChannel;
		SINVENITEM			Data;

		SNETPC_STORAGE_INSERT () 
			: dwChannel(0)
		{
			nmg.dwSize = sizeof(SNETPC_STORAGE_INSERT);
			nmg.nType = NET_MSG_GCTRL_STORAGE_INSERT;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_STORAGE_DELETE
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwChannel;
		WORD				wPosX;
		WORD				wPosY;

		SNETPC_STORAGE_DELETE () 
			: dwChannel(0)
			, wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_STORAGE_DELETE);
			nmg.nType = NET_MSG_GCTRL_STORAGE_DELETE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_STORAGE_ITEM_UPDATE
	{
		NET_MSG_GENERIC		nmg;

        DWORD				dwChannel;
		WORD				wPosX;
		WORD				wPosY;
		SITEMCUSTOM			sItemCustom;

		SNETPC_STORAGE_ITEM_UPDATE () 
			: dwChannel(0)
			, wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_STORAGE_ITEM_UPDATE);
			nmg.nType = NET_MSG_GCTRL_STORAGE_ITEM_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_STORAGE_DEL_AND_INSERT
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwChannel;

		WORD				wDelX;
		WORD				wDelY;
        
		SINVENITEM			sInsert;

		SNETPC_STORAGE_DEL_AND_INSERT () 
			: dwChannel(0)
			, wDelX(0)
			, wDelY(0)
		{
			nmg.dwSize = sizeof(SNETPC_STORAGE_DEL_AND_INSERT);
			nmg.nType = NET_MSG_GCTRL_STORAGE_DEL_INSERT;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_STORAGE_DRUG_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwChannel;

		WORD				wPosX;
		WORD				wPosY;

		WORD				wTurnNum;
		
		SNETPC_STORAGE_DRUG_UPDATE () 
			: dwChannel(0)
			, wPosX(0)
			, wPosY(0)
			, wTurnNum(0)
		{
			nmg.dwSize = sizeof(SNETPC_STORAGE_DRUG_UPDATE);
			nmg.nType = NET_MSG_GCTRL_STORAGE_DRUG_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_STORAGE_SAVE_MONEY
	{
		NET_MSG_GENERIC		nmg;
		LONGLONG			lnMoney;
		DWORD				dwNPCID;

		SNETPC_REQ_STORAGE_SAVE_MONEY () 
			: lnMoney(0)
			, dwNPCID(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_STORAGE_SAVE_MONEY);
			nmg.nType = NET_MSG_GCTRL_STORAGE_SAVE_MONEY;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_STORAGE_DRAW_MONEY
	{
		NET_MSG_GENERIC		nmg;
		LONGLONG			lnMoney;
		DWORD				dwNPCID;

		SNETPC_REQ_STORAGE_DRAW_MONEY () 
			: lnMoney(0)
			, dwNPCID(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_STORAGE_DRAW_MONEY);
			nmg.nType = NET_MSG_GCTRL_STORAGE_DRAW_MONEY;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_STORAGE_UPDATE_MONEY
	{
		NET_MSG_GENERIC		nmg;
		LONGLONG			lnMoney;

		SNETPC_REQ_STORAGE_UPDATE_MONEY ()
			: lnMoney(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_STORAGE_UPDATE_MONEY);
			nmg.nType = NET_MSG_GCTRL_STORAGE_UPDATE_MONEY;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_STORAGE_SPLIT
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwChannel;
		DWORD				dwNPCID;
		WORD				wPosX;
		WORD				wPosY;
		WORD				wSplit;

		SNETPC_REQ_STORAGE_SPLIT () 
			: dwChannel(0)
			, dwNPCID(0)
			, wPosX(0)
			, wPosY(0)
			, wSplit(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_STORAGE_SPLIT);
			nmg.nType = NET_MSG_GCTRL_REQ_STORAGE_SPLIT;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNET_INVEN_GRINDING
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNET_INVEN_GRINDING () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_GRINDING);
			nmg.nType = NET_MSG_GCTRL_INVEN_GRINDING;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};
	struct SNETPC_REQ_HOLD_ARRAY
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX[300];
		WORD				wPosY[300];

		SNETPC_REQ_HOLD_ARRAY () 
		{
			nmg.dwSize = sizeof(SNETPC_REQ_HOLD_ARRAY);
			nmg.nType = NET_MSG_GCTRL_REQ_HOLD_ARRAY;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};
	
	struct SNETPC_REQ_HOLDARRAY_TO_INVEN
	{
		NET_MSG_GENERIC		nmg;

		SNETPC_REQ_HOLDARRAY_TO_INVEN () 
		{
			nmg.dwSize = sizeof(SNETPC_REQ_HOLDARRAY_TO_INVEN);
			nmg.nType = NET_MSG_GCTRL_REQ_HOLDARRAY_TO_INVEN;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNET_INVEN_GRINDING_FB
	{
		NET_MSG_GENERIC		nmg;

		bool				bRESET;
		bool				bTERMINATE;
		EMANTIDISAPPEAR		emANTIDISAPPEAR;
		EMGRINDING_FB		emGrindFB;

		SNET_INVEN_GRINDING_FB () 
			: bRESET(false)
			, bTERMINATE(false)
			, emANTIDISAPPEAR(EMANTIDISAPPEAR_OFF)
			, emGrindFB(EMGRINDING_FAIL)
		{
			nmg.dwSize = sizeof(SNET_INVEN_GRINDING_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_GRINDING_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_BOXOPEN
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;
		//Select Box
		BOOL				bSelect[ ITEM::SBOX::ITEM_SIZE ];		

		SNET_INVEN_BOXOPEN () 
			: wPosX(0)
			, wPosY(0)
		{
			//Select Box
			for( int i=0;i<ITEM::SBOX::ITEM_SIZE;++i)	bSelect[i] = FALSE;

			nmg.dwSize = sizeof(SNET_INVEN_BOXOPEN);
			nmg.nType = NET_MSG_GCTRL_INVEN_BOXOPEN;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_BOXOPEN_FB
	{
		NET_MSG_GENERIC		nmg;

		EMREQ_BOXOPEN_FB	emFB;

		SNET_INVEN_BOXOPEN_FB () 
			: emFB(EMREQ_BOXOPEN_FB_FAIL)
		{
			nmg.dwSize = sizeof(SNET_INVEN_BOXOPEN_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_BOXOPEN_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_DISGUISE
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNET_INVEN_DISGUISE () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_DISGUISE);
			nmg.nType = NET_MSG_GCTRL_INVEN_DISGUISE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_DISGUISE_FB
	{
		NET_MSG_GENERIC		nmg;

		EMREQ_DISGUISE_FB	emFB;

		SNET_INVEN_DISGUISE_FB () 
			: emFB(EMREQ_DISGUISE_FB_FAIL)
		{
			nmg.dwSize = sizeof(SNET_INVEN_DISGUISE_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_DISGUISE_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_CLEANSER
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNET_INVEN_CLEANSER () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_CLEANSER);
			nmg.nType = NET_MSG_GCTRL_INVEN_CLEANSER;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_CLEANSER_FB
	{
		NET_MSG_GENERIC		nmg;

		EMREQ_CLEANSER_FB	emFB;

		SNET_INVEN_CLEANSER_FB () 
			: emFB(EMREQ_CLEANSER_FB_FAIL)
		{
			nmg.dwSize = sizeof(SNET_INVEN_CLEANSER_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_CLEANSER_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_RESET_SKST
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNET_INVEN_RESET_SKST () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_RESET_SKST);
			nmg.nType = NET_MSG_GCTRL_INVEN_RESET_SKST;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_RESET_SKST_FB
	{
		NET_MSG_GENERIC		nmg;

		WORD				wITEM_NUM;
		WORD				wSTATS_POINT;

		EMREQ_RESET_SKST_FB	emFB;

		SNET_INVEN_RESET_SKST_FB () 
			: wITEM_NUM(0)
			, wSTATS_POINT(0)
			, emFB(EMREQ_RESET_SKST_FB_FAIL)
		{
			nmg.dwSize = sizeof(SNET_INVEN_RESET_SKST_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_RESET_SKST_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_DEL_ITEM_TIMELMT
	{
		NET_MSG_GENERIC		nmg;

		SNATIVEID			nidITEM;

		SNET_INVEN_DEL_ITEM_TIMELMT () 
			: nidITEM(false)
		{
			nmg.dwSize = sizeof(SNET_INVEN_DEL_ITEM_TIMELMT);
			nmg.nType = NET_MSG_GCTRL_INVEN_DEL_ITEM_TIMELMT;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	struct SNET_INVEN_CHARCARD
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNET_INVEN_CHARCARD () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_CHARCARD);
			nmg.nType = NET_MSG_GCTRL_INVEN_CHARCARD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_CHARCARD_FB
	{
		NET_MSG_GENERIC			nmg;

		EMREQ_CHARCARD_FB		emFB;

		SNET_INVEN_CHARCARD_FB () 
			: emFB(EMREQ_CHARCARD_FB_FAIL)
		{
			nmg.dwSize = sizeof(SNET_INVEN_CHARCARD_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_CHARCARD_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_STORAGECARD
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		WORD				wSTORAGE;

		SNET_INVEN_STORAGECARD () 
			: wPosX(0)
			, wPosY(0)
			, wSTORAGE(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_STORAGECARD);
			nmg.nType = NET_MSG_GCTRL_INVEN_STORAGECARD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_STORAGECARD_FB
	{
		NET_MSG_GENERIC			nmg;

		EMREQ_STORAGECARD_FB	emFB;

		WORD					wSTORAGE;
		__time64_t				tSTORAGE_LIMIT;
		__time64_t				tSPAN;

		SNET_INVEN_STORAGECARD_FB () 
			: emFB(EMREQ_STORAGECARD_FB_FAIL)
			, wSTORAGE(0)
			, tSTORAGE_LIMIT(0)
			, tSPAN(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_STORAGECARD_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_STORAGECARD_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_INVENLINE
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNET_INVEN_INVENLINE () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_INVENLINE);
			nmg.nType = NET_MSG_GCTRL_INVEN_INVENLINE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_INVENLINE_FB
	{
		NET_MSG_GENERIC		nmg;

		WORD				wINVENLINE;
		EMREQ_INVENLINE_FB	emFB;

		SNET_INVEN_INVENLINE_FB () 
			: emFB(EMREQ_INVENLINE_FB_FAIL)
			, wINVENLINE(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_INVENLINE_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_INVENLINE_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_REMODELOPEN
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNET_INVEN_REMODELOPEN () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_REMODELOPEN);
			nmg.nType = NET_MSG_GCTRL_INVEN_REMODELOPEN;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};	

	struct SNET_INVEN_REMODELOPEN_FB
	{
		NET_MSG_GENERIC			nmg;

		EMREQ_REMODELOPEN_FB	emFB;
		WORD					wPosX;
		WORD					wPosY;

		SNET_INVEN_REMODELOPEN_FB () 
			: emFB(EMREQ_REMODELOPEN_FB_FAIL)
			, wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_REMODELOPEN_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_REMODELOPEN_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_GARBAGEOPEN
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		bool				bInven;

		SNET_INVEN_GARBAGEOPEN () 
			: wPosX(0)
			, wPosY(0)
			, bInven(false)
		{
			nmg.dwSize = sizeof(SNET_INVEN_GARBAGEOPEN);
			nmg.nType = NET_MSG_GCTRL_INVEN_GARBAGEOPEN;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_GARBAGEOPEN_FB
	{
		NET_MSG_GENERIC			nmg;

		EMREQ_GARBAGEOPEN_FB	emFB;

		SNET_INVEN_GARBAGEOPEN_FB () 
			: emFB(EMREQ_GARBAGEOPEN_FB_FAIL)
		{
			nmg.dwSize = sizeof(SNET_INVEN_GARBAGEOPEN_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_GARBAGEOPEN_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_STORAGEOPEN
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNET_INVEN_STORAGEOPEN () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_STORAGEOPEN);
			nmg.nType = NET_MSG_GCTRL_INVEN_STORAGEOPEN;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_STORAGEOPEN_FB
	{
		NET_MSG_GENERIC			nmg;

		EMREQ_STORAGEOPEN_FB	emFB;
		WORD				wPosX;
		WORD				wPosY;
		
		SNET_INVEN_STORAGEOPEN_FB () 
			: emFB(EMREQ_STORAGEOPEN_FB_FAIL)
			, wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_STORAGEOPEN_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_STORAGEOPEN_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_STORAGECLOSE
	{
		NET_MSG_GENERIC		nmg;

		SNET_INVEN_STORAGECLOSE () 
		{
			nmg.dwSize = sizeof(SNET_INVEN_STORAGECLOSE);
			nmg.nType = NET_MSG_GCTRL_INVEN_STORAGECLOSE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_PREMIUMSET
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNET_INVEN_PREMIUMSET () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_PREMIUMSET);
			nmg.nType = NET_MSG_GCTRL_INVEN_PREMIUMSET;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_PREMIUMSET_FB
	{
		NET_MSG_GENERIC			nmg;

		__time64_t				tLMT;
		__time64_t				tSPAN;

		EMREQ_PREMIUMSET_FB		emFB;

		SNET_INVEN_PREMIUMSET_FB () 
			: emFB(EMREQ_PREMIUMSET_FB_FAIL)
			, tLMT(0)
			, tSPAN(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_PREMIUMSET_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_PREMIUMSET_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_GET_CHARGEDITEM_FROMDB
	{
		NET_MSG_GENERIC			nmg;

		DWORD					dwCharID;
		char					szUID[USR_ID_LENGTH+1];

		SNET_GET_CHARGEDITEM_FROMDB ()
			: dwCharID(0)
		{
			nmg.dwSize = sizeof ( SNET_GET_CHARGEDITEM_FROMDB );
			nmg.nType  = NET_MSG_GCTRL_GET_CHARGEDITEM_FROMDB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
			memset ( szUID, 0, sizeof(char)*(USR_ID_LENGTH+1) );
		}
	};

	struct SNET_GET_CHARGEDITEM_FROMDB_FB
	{
		NET_MSG_GENERIC				nmg;

		EMREQ_CHARGEDITEM_FROMDB_FB emFB;
		char						szPurKey[PURKEY_LENGTH+1];
		SNATIVEID					nidITEM;

		SNET_GET_CHARGEDITEM_FROMDB_FB ()
			: emFB(EMREQ_CHARGEDITEM_FROMDB_FB_END)
			, nidITEM(false)
		{
			nmg.dwSize = sizeof ( SNET_GET_CHARGEDITEM_FROMDB_FB );
			nmg.nType  = NET_MSG_GCTRL_GET_CHARGEDITEM_FROMDB_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
			memset ( szPurKey, 0, sizeof(char)*(PURKEY_LENGTH+1) );
		}
	};

	struct SNET_CHARGED_ITEM_GET
	{
		NET_MSG_GENERIC			nmg;
		
		DWORD					dwID;
		char					szPurKey[PURKEY_LENGTH+1];

		SNET_CHARGED_ITEM_GET () 
			: dwID(UINT_MAX)
		{
			nmg.dwSize = sizeof(SNET_CHARGED_ITEM_GET);
			nmg.nType = NET_MSG_GCTRL_CHARGED_ITEM_GET;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);

			memset(szPurKey, 0, sizeof(char) * (PURKEY_LENGTH+1));
		}
	};

	struct SNET_CHARGED_ITEM_GET_FB
	{
		NET_MSG_GENERIC			nmg;

		EMCHARGED_ITEM_GET_FB	emFB;
		SNATIVEID				nidITEM;

		SNET_CHARGED_ITEM_GET_FB () 
			: nidITEM(false)
			, emFB(EMCHARGED_ITEM_GET_FB_FAIL)
		{
			nmg.dwSize = sizeof(SNET_CHARGED_ITEM_GET_FB);
			nmg.nType = NET_MSG_GCTRL_CHARGED_ITEM_GET_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_CHARGED_ITEM_DEL
	{
		NET_MSG_GENERIC			nmg;

		DWORD					dwID;

		SNET_CHARGED_ITEM_DEL () 
			: dwID(UINT_MAX)
		{
			nmg.dwSize = sizeof(SNET_CHARGED_ITEM_DEL);
			nmg.nType = NET_MSG_GCTRL_CHARGED_ITEM_DEL;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_CHARGED_ITEM2_INVEN
	{
		NET_MSG_GENERIC			nmg;

		DWORD					dwUserID;
		SNATIVEID				nidITEM;
		char					szPurKey[PURKEY_LENGTH+1];
		DWORD					dwInvenPos;


		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+sizeof(SNATIVEID)+sizeof(char)*(PURKEY_LENGTH+1)+sizeof(DWORD) };
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNET_CHARGED_ITEM2_INVEN () 
			: dwUserID(0)
			, nidITEM(false)
			, dwInvenPos(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CHARGED_ITEM2_INVEN;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
			memset(szPurKey, 0, sizeof(char) * (PURKEY_LENGTH+1));
		}
	};

	struct SNET_INVEN_RANDOMBOXOPEN
	{
		NET_MSG_GENERIC			nmg;

		WORD					wPosX;
		WORD					wPosY;

		SNET_INVEN_RANDOMBOXOPEN () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_RANDOMBOXOPEN);
			nmg.nType = NET_MSG_GCTRL_INVEN_RANDOMBOXOPEN;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_RANDOMBOXOPEN_FB
	{
		NET_MSG_GENERIC				nmg;

		EMINVEN_RANDOMBOXOPEN_FB	emFB;

		SNET_INVEN_RANDOMBOXOPEN_FB () 
			: emFB(EMINVEN_RANDOMBOXOPEN_FB_FAIL)
		{
			nmg.dwSize = sizeof(SNET_INVEN_RANDOMBOXOPEN_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_RANDOMBOXOPEN_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	

	




	struct SNET_INVEN_DISJUNCTION
	{
		NET_MSG_GENERIC			nmg;

		WORD					wPosX;
		WORD					wPosY;

		SNET_INVEN_DISJUNCTION () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_DISJUNCTION);
			nmg.nType = NET_MSG_GCTRL_INVEN_DISJUNCTION;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_DISJUNCTION_FB
	{
		NET_MSG_GENERIC				nmg;

		EMINVEN_DISJUNCTION_FB		emFB;

		SNET_INVEN_DISJUNCTION_FB () 
			: emFB(EMINVEN_DISJUNCTION_FB_FAIL)
		{
			nmg.dwSize = sizeof(SNET_INVEN_DISJUNCTION_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_DISJUNCTION_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*dmk14 reform*/
	struct SNET_REBUILD_RESULT
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwNPCID;
		EMREBUILD_RESULT	emResult;
		EMRANDOM_OPT		emLockOpt[ SRANDOM_GEN::MAX_OPT ];
		SITEMCUSTOM			sItemCopy;

		SNET_REBUILD_RESULT()
			: emResult( EMREBUILD_RESULT_FAIL )
			, dwNPCID(0)
		{
			for( int i=0;i<SRANDOM_GEN::MAX_OPT;i++) emLockOpt[i] = EMR_OPT_NULL;

			nmg.dwSize = sizeof( SNET_REBUILD_RESULT );
			nmg.nType = NET_MSG_REBUILD_RESULT;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};

	// 휴지통
	struct SNET_GARBAGE_RESULT
	{
		NET_MSG_GENERIC		nmg;
		WORD				wPosX;	// 삭제할 아이템 좌표
		WORD				wPosY;	// 삭제할 아이템 좌표

		SNET_GARBAGE_RESULT()
			: wPosX(EM_INVENSIZE_X)
			, wPosY(EM_INVENSIZE_Y)
		{
			nmg.dwSize = sizeof( SNET_GARBAGE_RESULT );
			nmg.nType = NET_MSG_GCTRL_GARBAGE_RESULT;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};

	struct SNET_GARBAGE_RESULT_FB
	{
		NET_MSG_GENERIC		nmg;
		EMGARBAGE_RESULT_FB	emResult;		

		SNET_GARBAGE_RESULT_FB()
			: emResult ( EMGARBAGE_RESULT_FB_FAIL )
		{
			nmg.dwSize = sizeof( SNET_GARBAGE_RESULT_FB );
			nmg.nType = NET_MSG_GCTRL_GARBAGE_RESULT_FB;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};

	struct SNET_REBUILD_MOVE_ITEM
	{
		NET_MSG_GENERIC	nmg;
		WORD			wPosX;
		WORD			wPosY;

		SNET_REBUILD_MOVE_ITEM()
			: wPosX( USHRT_MAX )
			, wPosY( USHRT_MAX )
		{
			nmg.dwSize = sizeof( SNET_REBUILD_MOVE_ITEM );
			nmg.nType = NET_MSG_REBUILD_MOVE_ITEM;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};

	/*dmk14 reform*/
	struct SNET_REBUILD_MOVE_ITEMSEALED
	{
		NET_MSG_GENERIC	nmg;
		SNATIVEID		sItemSeal;

		SNET_REBUILD_MOVE_ITEMSEALED()
			: sItemSeal ( NATIVEID_NULL() )
		{
			nmg.dwSize = sizeof( SNET_REBUILD_MOVE_ITEMSEALED );
			nmg.nType = NET_MSG_REBUILD_MOVE_ITEMSEALED;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};
	
	struct SNET_REBUILD_COST_MONEY
	{
		NET_MSG_GENERIC	nmg;
		LONGLONG		i64CostMoney;

		SNET_REBUILD_COST_MONEY()
			: i64CostMoney( 0 )
		{
			nmg.dwSize = sizeof( SNET_REBUILD_COST_MONEY );
			nmg.nType = NET_MSG_REBUILD_COST_MONEY;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};

	struct SNET_REBUILD_INPUT_MONEY
	{
		NET_MSG_GENERIC	nmg;
		LONGLONG		i64InputMoney;

		SNET_REBUILD_INPUT_MONEY()
			: i64InputMoney( 0 )
		{
			nmg.dwSize = sizeof( SNET_REBUILD_INPUT_MONEY );
			nmg.nType = NET_MSG_REBUILD_INPUT_MONEY;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_VNGAIN_TO_HOLD
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNETPC_REQ_VNGAIN_TO_HOLD () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_VNGAIN_TO_HOLD);
			nmg.nType = NET_MSG_GCTRL_REQ_VNGAIN_TO_HOLD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_VNGAIN_EX_HOLD
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNETPC_REQ_VNGAIN_EX_HOLD () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_VNGAIN_EX_HOLD);
			nmg.nType = NET_MSG_GCTRL_REQ_VNGAIN_EX_HOLD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_HOLD_TO_VNGAIN
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNETPC_REQ_HOLD_TO_VNGAIN () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_HOLD_TO_VNGAIN);
			nmg.nType = NET_MSG_GCTRL_REQ_HOLD_TO_VNGAIN;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_VNINVEN_TO_INVEN
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNETPC_REQ_VNINVEN_TO_INVEN () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_VNINVEN_TO_INVEN);
			nmg.nType = NET_MSG_GCTRL_REQ_VNINVEN_TO_INVEN;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_VNINVEN_TO_INVEN_FB
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;
		WORD				wNewPosX;
		WORD				wNewPosY;
		SINVENITEM			Data;

		SNETPC_REQ_VNINVEN_TO_INVEN_FB () 
			: wPosX(0)
			, wPosY(0)
			, wNewPosX(0)
			, wNewPosY(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_VNINVEN_TO_INVEN_FB);
			nmg.nType = NET_MSG_GCTRL_REQ_VNINVEN_TO_INVEN_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETPC_REQ_VNGAIN_INVEN_RESET
	{
		NET_MSG_GENERIC		nmg;

	
		SNETPC_REQ_VNGAIN_INVEN_RESET () 
		{
			nmg.dwSize = sizeof(SNETPC_REQ_VNGAIN_INVEN_RESET);
			nmg.nType = NET_MSG_GCTRL_REQ_VNGAIN_INVEN_RESET;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	struct SNETPC_ITEM_COOLTIME_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		
		DWORD				dwID;
		DWORD				dwCoolID;
		__time64_t			tUseTime;
		__time64_t			tCoolTime;
		EMCOOL_TYPE			emCoolType;
		
		SNETPC_ITEM_COOLTIME_UPDATE () 
			: dwID(0)
			, dwCoolID( 0 )
			, tUseTime(0)
			, tCoolTime(0)
			, emCoolType(EMCOOL_ITEMID)
		{
			nmg.dwSize = sizeof(SNETPC_ITEM_COOLTIME_UPDATE);
			nmg.nType = NET_MSG_GCTRL_ITEM_COOLTIME_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_ITEM_COOLTIME_ERROR
	{
		NET_MSG_GENERIC		nmg;
		SNATIVEID			sNativeID;

		SNET_ITEM_COOLTIME_ERROR () 
			: sNativeID(false)
		{
			nmg.dwSize = sizeof(SNET_ITEM_COOLTIME_ERROR);
			nmg.nType = NET_MSG_GCTRL_ITEM_COOLTIME_ERROR;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	struct SNET_INVEN_NPC_RECALL
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;


		SNET_INVEN_NPC_RECALL () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_NPC_RECALL);
			nmg.nType = NET_MSG_GCTRL_NPC_RECALL;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_NPC_RECALL_FB
	{
		NET_MSG_GENERIC			nmg;

		EMREQ_NPC_RECALL_FB	emFB;

		SNET_INVEN_NPC_RECALL_FB () 
			: emFB(EMREQ_NPC_RECALL_FB_FAIL)		
		{
			nmg.dwSize = sizeof(SNET_INVEN_NPC_RECALL_FB);
			nmg.nType = NET_MSG_GCTRL_NPC_RECALL_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_NPC_COMMISSION
	{
		NET_MSG_GENERIC			nmg;

        int			nCHANNEL;
		DWORD		dwFieldID;
		DWORD		dwCharID;
		DWORD		dwUserID;
		LONGLONG	lnCommission;

		SNET_INVEN_NPC_COMMISSION () 
			: nCHANNEL(0)
			, dwFieldID(0)
			, dwCharID(0)
			, dwUserID(0)
			, lnCommission(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_NPC_COMMISSION);
			nmg.nType = NET_MSG_GCTRL_NPC_COMMISSION;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_NPC_COMMISSION_FB
	{
		NET_MSG_GENERIC			nmg;

		EMREQ_NPC_COMMISSION_FB	emFB;

		DWORD		dwCharID;
		DWORD		dwUserID;
		LONGLONG	lnCommission;

		SNET_INVEN_NPC_COMMISSION_FB () 
			: emFB( EMREQ_NPC_COMMISSION_FAIL )
			, dwCharID(0)
			, dwUserID(0)
			, lnCommission(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_NPC_COMMISSION_FB);
			nmg.nType = NET_MSG_GCTRL_NPC_COMMISSION_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	// 아이템 조합
	struct SNET_INVEN_ITEM_MIX
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwKey;
		SINVEN_POS			sInvenPos[ITEMMIX_ITEMNUM];
		DWORD				dwNpcID;

		SNET_INVEN_ITEM_MIX()
			: dwKey( UINT_MAX )
			, dwNpcID( UINT_MAX )
		{
			nmg.dwSize = sizeof( SNET_INVEN_ITEM_MIX );
			nmg.nType = NET_MSG_GCTRL_INVEN_ITEM_MIX;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};

	// 아이템 조합
	struct SNET_INVEN_ITEM_MIX_FB
	{
		NET_MSG_GENERIC		nmg;

		EMITEM_MIX_FB		emFB;
		SNATIVEID			sNativeID;
		WORD				wTurnNum;


		SNET_INVEN_ITEM_MIX_FB()
			: emFB( EMITEM_MIX_FB_FAIL )
			, sNativeID(false)
			, wTurnNum( 0 ) 
		{
			nmg.dwSize = sizeof( SNET_INVEN_ITEM_MIX );
			nmg.nType = NET_MSG_GCTRL_INVEN_ITEM_MIX_FB;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};


	/*itemfood system, Juver, 2017/05/25 */
	struct SNET_INVEN_CONSUME_FOOD
	{
		NET_MSG_GENERIC		nmg;
		WORD	wPosX;
		WORD	wPosY;

		SNET_INVEN_CONSUME_FOOD()
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof( SNET_INVEN_CONSUME_FOOD );
			nmg.nType = NET_MSG_GCTRL_INVEN_CONSUME_FOOD;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};

	struct SNET_INVEN_CONSUME_FOOD_FB
	{
		NET_MSG_GENERIC		nmg;
		EMCONSUME_FOOD_FB	emFB;

		SNET_INVEN_CONSUME_FOOD_FB()
			: emFB( EMCONSUME_FOOD_FB_FAIL )
		{
			nmg.dwSize = sizeof( SNET_INVEN_CONSUME_FOOD_FB );
			nmg.nType = NET_MSG_GCTRL_INVEN_CONSUME_FOOD_FB;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};

	/*itemfood system, Juver, 2017/05/26 */
	struct SNET_INVEN_UNLOCK_FOOD
	{
		NET_MSG_GENERIC		nmg;
		WORD	wPosX;
		WORD	wPosY;

		SNET_INVEN_UNLOCK_FOOD()
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof( SNET_INVEN_UNLOCK_FOOD );
			nmg.nType = NET_MSG_GCTRL_INVEN_UNLOCK_FOOD;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};

	struct SNET_INVEN_UNLOCK_FOOD_FB
	{
		NET_MSG_GENERIC		nmg;
		EMUNLOCK_FOOD_FB	emFB;

		SNET_INVEN_UNLOCK_FOOD_FB()
			: emFB( EMUNLOCK_FOOD_FB_FAIL )
		{
			nmg.dwSize = sizeof( SNET_INVEN_UNLOCK_FOOD_FB );
			nmg.nType = NET_MSG_GCTRL_INVEN_UNLOCK_FOOD_FB;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};

	/*rightclick wear/unwear, Juver, 2017/06/23 */
	struct SNETPC_REQ_INVEN_TO_WEAR
	{
		NET_MSG_GENERIC		nmg;

		WORD		wPosX;
		WORD		wPosY;
		EMSLOT		emSlot;

		SNETPC_REQ_INVEN_TO_WEAR () 
			: wPosX(0)
			, wPosY(0)
			, emSlot(SLOT_TSIZE)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_INVEN_TO_WEAR);
			nmg.nType = NET_MSG_GCTRL_REQ_INVEN_TO_WEAR;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_REQ_WEAR_TO_INVEN
	{
		NET_MSG_GENERIC		nmg;

		EMSLOT		emSlot;

		SNETPC_REQ_WEAR_TO_INVEN () 
			: emSlot(SLOT_TSIZE)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_WEAR_TO_INVEN);
			nmg.nType = NET_MSG_GCTRL_REQ_WEAR_TO_INVEN;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*npc shop, Juver, 2017/07/27 */
	struct SNETPC_REQ_NPCSHOP_PURCHASE_MONEY
	{
		NET_MSG_GENERIC		nmg;

		SNATIVEID	sidCrow;
		SNATIVEID	sidItem;
		WORD		wShopType;

		SNETPC_REQ_NPCSHOP_PURCHASE_MONEY () 
			: sidCrow(NATIVEID_NULL())
			, sidItem(NATIVEID_NULL())
			, wShopType(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_NPCSHOP_PURCHASE_MONEY);
			nmg.nType = NET_MSG_GCTRL_NPCSHOP_PURCHASE_MONEY;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*npc shop, Juver, 2017/07/27 */
	struct SNETPC_REQ_NPCSHOP_PURCHASE_MONEY_FB
	{
		NET_MSG_GENERIC			nmg;
		EMNPCSHOP_PURCHASE_FB	emFB;

		SNETPC_REQ_NPCSHOP_PURCHASE_MONEY_FB()
			: emFB( EMNPCSHOP_PURCHASE_FB_PURCHASED )
		{
			nmg.dwSize = sizeof(SNETPC_REQ_NPCSHOP_PURCHASE_MONEY_FB);
			nmg.nType = NET_MSG_GCTRL_NPCSHOP_PURCHASE_MONEY_FB;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};


	/*item exchange, Juver, 2017/10/13 */
	struct SNETPC_REQ_NPC_ITEM_EXCHANGE_TRADE
	{
		NET_MSG_GENERIC		nmg;

		SNATIVEID	sid_NPC;
		WORD		wTradeID;

		SNETPC_REQ_NPC_ITEM_EXCHANGE_TRADE () 
			: sid_NPC(NATIVEID_NULL())
			, wTradeID(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_NPC_ITEM_EXCHANGE_TRADE);
			nmg.nType = NET_MSG_GCTRL_NPC_ITEM_EXCHANGE_TRADE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*item exchange, Juver, 2017/10/13 */
	struct SNETPC_REQ_NPC_ITEM_EXCHANGE_TRADE_FB
	{
		NET_MSG_GENERIC			nmg;
		EMNPC_ITEM_EXCHANGE_FB	emFB;

		SNETPC_REQ_NPC_ITEM_EXCHANGE_TRADE_FB()
			: emFB( EMNPC_ITEM_EXCHANGE_FB_OK )
		{
			nmg.dwSize = sizeof(SNETPC_REQ_NPC_ITEM_EXCHANGE_TRADE_FB);
			nmg.nType = NET_MSG_GCTRL_NPC_ITEM_EXCHANGE_TRADE_FB;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};

	/*product item, Juver, 2017/10/18 */
	struct SNETPC_REQ_ITEM_COMPOUND_START
	{
		NET_MSG_GENERIC		nmg;

		DWORD		dwProductID;
		int			nProductCount;

		SNETPC_REQ_ITEM_COMPOUND_START () 
			: dwProductID(UINT_MAX)
			, nProductCount(1)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_ITEM_COMPOUND_START);
			nmg.nType = NET_MSG_GCTRL_ITEM_COMPOUND_START;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_REQ_ITEM_COMPOUND_CANCEL
	{
		NET_MSG_GENERIC		nmg;

		SNETPC_REQ_ITEM_COMPOUND_CANCEL () 
		{
			nmg.dwSize = sizeof(SNETPC_REQ_ITEM_COMPOUND_CANCEL);
			nmg.nType = NET_MSG_GCTRL_ITEM_COMPOUND_CANCEL;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*product item, Juver, 2017/10/18 */
	struct SNETPC_REQ_ITEM_COMPOUND_START_FB
	{
		NET_MSG_GENERIC			nmg;
		RAN_PRODUCT_FB			emFB;

		BOOL	bItemCompoundTask;
		DWORD	dwItemCompoundTaskID;
		float	fItemCompoundTaskTime;
		float	fItemCompoundTaskTimer;

		SNETPC_REQ_ITEM_COMPOUND_START_FB()
			: emFB( RAN_PRODUCT_FB_STARTED )
			, bItemCompoundTask(FALSE)
			, dwItemCompoundTaskID(UINT_MAX)
			, fItemCompoundTaskTime(0.0f)
			, fItemCompoundTaskTimer(0.0f)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_ITEM_COMPOUND_START_FB);
			nmg.nType = NET_MSG_GCTRL_ITEM_COMPOUND_START_FB;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};

	/*product item, Juver, 2017/10/18 */
	struct SNETPC_REQ_ITEM_COMPOUND_PROCESS_FB
	{
		NET_MSG_GENERIC			nmg;
		RAN_PRODUCT_PROCESS_FB	emFB;

		SNETPC_REQ_ITEM_COMPOUND_PROCESS_FB()
			: emFB( RAN_PRODUCT_PROCESS_FB_OK )
		{
			nmg.dwSize = sizeof(SNETPC_REQ_ITEM_COMPOUND_PROCESS_FB);
			nmg.nType = NET_MSG_GCTRL_ITEM_COMPOUND_PROCESS_FB;
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};

	/*rv card, Juver, 2017/11/25 */
	struct SNET_INVEN_RANDOM_OPTION_CHANGE
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNET_INVEN_RANDOM_OPTION_CHANGE () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_RANDOM_OPTION_CHANGE);
			nmg.nType = NET_MSG_GCTRL_INVEN_RANDOM_OPTION_CHANGE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*rv card, Juver, 2017/11/25 */
	struct SNET_INVEN_RANDOM_OPTION_CHANGE_FB
	{
		NET_MSG_GENERIC		nmg;

		EM_RANDOM_OPTION_CHANGE_FB	emFB;

		SNET_INVEN_RANDOM_OPTION_CHANGE_FB () 
			: emFB(EM_RANDOM_OPTION_CHANGE_DONE_CHANGE)
		{
			nmg.dwSize = sizeof(SNET_INVEN_RANDOM_OPTION_CHANGE_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_RANDOM_OPTION_CHANGE_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	struct SNET_MYSTERY_BOX
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNET_MYSTERY_BOX()
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_MYSTERY_BOX);
			nmg.nType = NET_MSG_GCTRL_MYSTERY_BOX;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//Mystery Box
	struct SNET_MYSTERY_BOX_FB
	{
		NET_MSG_GENERIC		nmg;

		SNET_MYSTERY_BOX_FB()
		{
			nmg.dwSize = sizeof(SNET_INVEN_GRINDING_FB);
			nmg.nType = NET_MSG_GCTRL_MYSTERY_BOX_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*nondrop card, Juver, 2017/11/26 */
	struct SNET_INVEN_NONDROP_CARD
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNET_INVEN_NONDROP_CARD () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_NONDROP_CARD);
			nmg.nType = NET_MSG_GCTRL_INVEN_NONDROP_CARD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*nondrop card, Juver, 2017/11/26 */
	struct SNET_INVEN_NONDROP_CARD_FB
	{
		NET_MSG_GENERIC		nmg;

		EM_NONDROP_CARD_FB	emFB;

		SNET_INVEN_NONDROP_CARD_FB () 
			: emFB(EM_NONDROP_CARD_DONE_CHANGE)
		{
			nmg.dwSize = sizeof(SNET_INVEN_NONDROP_CARD_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_NONDROP_CARD_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	/*regional chat, Juver, 2017/12/06 */
	struct SNET_REGIONAL_CHAT_PAY
	{
		NET_MSG_GENERIC		nmg;
		DWORD dwCost;

		SNET_REGIONAL_CHAT_PAY () 
			: dwCost(0)
		{
			nmg.dwSize = sizeof(SNET_REGIONAL_CHAT_PAY);
			nmg.nType = NET_MSG_GCTRL_REGIONAL_CHAT_PAY;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_GMITEM //add itemcmd
	{
		NET_MSG_GENERIC		nmg;

		SNATIVEID			sItemID;
		WORD				wNum;
		WORD				wPass;
		bool				bDrop;

		SNET_INVEN_GMITEM () 
			: wNum(0)
			, wPass(0)
			, bDrop(false)
		{
			nmg.dwSize = sizeof(SNET_INVEN_GMITEM);
			nmg.nType = NET_MSG_GCTRL_INVEN_GMITEM;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_INVEN_GMITEM_FB //add itemcmd
	{
		NET_MSG_GENERIC		nmg;

		EMREQ_GMITEM_FB	emFB;

		SNET_INVEN_GMITEM_FB () 
			: emFB(EMREQ_GMITEM_FB_FAIL)
		{
			nmg.dwSize = sizeof(SNET_INVEN_GMITEM_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_GMITEM_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*dmk14 ingame web*/
	struct SNET_GET_ITEMMALL_FROMDB
	{
		NET_MSG_GENERIC			nmg;
		bool					bRequest;

		SNET_GET_ITEMMALL_FROMDB()
			: bRequest ( false )
		{
			nmg.dwSize = sizeof ( SNET_GET_ITEMMALL_FROMDB );
			nmg.nType  = NET_MSG_GCTRL_GET_ITEMMALL_FROMDB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_GET_ITEMMALL_FROMDB_FB
	{
		NET_MSG_GENERIC				nmg;

		EMREQ_ITEMMALL_FROMDB_FB	emFB;
		ITEMMALLDATA				sItemMall;

		SNET_GET_ITEMMALL_FROMDB_FB ()
			: emFB(EMREQ_ITEMMALL_FROMDB_FB_END)
		{
			nmg.dwSize = sizeof ( SNET_GET_ITEMMALL_FROMDB_FB );
			nmg.nType  = NET_MSG_GCTRL_GET_ITEMMALL_FROMDB_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_REQ_BUY_FROM_ITEMMALL
	{
		NET_MSG_GENERIC		nmg;
		
		DWORD				dwProductNum;
		DWORD				dwIndex;

		SNETPC_REQ_BUY_FROM_ITEMMALL () 
			: dwProductNum(0)
			, dwIndex(0)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_BUY_FROM_ITEMMALL);
			nmg.nType = NET_MSG_GCTRL_REQ_BUY_FROM_ITEMMALL;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};
	/*dmk14 ingame web end*/

	//Weapon Skill Reform Card - SqueeeAK
	struct SNET_INVEN_REFORM_WEAPON
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;

		SNET_INVEN_REFORM_WEAPON () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(SNET_INVEN_REFORM_WEAPON);
			nmg.nType = NET_MSG_GCTRL_INVEN_REFORM_WEAPON;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//Weapon Skill Reform Card - SqueeeAK
	struct SNET_INVEN_REFORM_WEAPON_FB
	{
		NET_MSG_GENERIC		nmg;
		bool				bSuccess;

		SNET_INVEN_REFORM_WEAPON_FB () 
			: bSuccess(false)
		{
			nmg.dwSize = sizeof(SNET_INVEN_REFORM_WEAPON_FB);
			nmg.nType = NET_MSG_GCTRL_INVEN_REFORM_WEAPON_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	// Revert to default structure packing
	#pragma pack()
};
