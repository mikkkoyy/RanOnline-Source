#pragma once

#include "GLContrlBaseMsg.h"

enum EMUSE_SUMMON_FB
{
	EMUSE_SUMMON_FB_OK				  = 0, // 성공
	EMUSE_SUMMON_FB_FAIL			  = 1, // 실패
	EMUSE_SUMMON_FB_FAIL_INVALIDZONE  = 2, // 소환할 수 없는 지역
	EMUSE_SUMMON_FB_FAIL_VEHICLE	  = 3, // 탈것을 타고 있어서 실패
	EMUSE_SUMMON_FB_FAIL_CONFRONT	  = 4, // 대련중 실패
	EMUSE_SUMMON_FB_FAIL_NODATA		  = 5, // 데이터가 없을경우

	/*skill summon, Juver, 2017/10/09 */
	EMUSE_SUMMON_FB_FAIL_MAX			= 6,
	EMUSE_SUMMON_FB_FAIL_MAX_SKILL		= 7,
};

enum EMSUMMON_REQ_SLOT_EX_HOLD_FB
{
	EMSUMMON_REQ_SLOT_EX_HOLD_FB_FAIL		  = 0,		// 일반오류.
	EMSUMMON_REQ_SLOT_EX_HOLD_FB_OK			  = 1,		// 아이템 교체 성공
	EMSUMMON_REQ_SLOT_EX_HOLD_FB_NOMATCH	  = 2,		// 타입이 맞지 않는 아이템
	EMSUMMON_REQ_SLOT_EX_HOLD_FB_INVALIDITEM  = 3,		// 탈것용이 아니다.
};


namespace GLMSG
{
	#pragma pack(1)

	/*struct SNETPC_REQ_USE_SUMMON
	{
		NET_MSG_GENERIC		nmg;
		SNATIVEID			sSummonID;
		SUMMON_TYPE			emTYPE;

		SNETPC_REQ_USE_SUMMON () :
		sSummonID(NATIVEID_NULL()),
			emTYPE(SUMMON_TYPE_NONE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_REQ_USE_SUMMON;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};*/

	struct SNETPC_REQ_USE_SUMMON_FB
	{
		NET_MSG_GENERIC		nmg;

		SUMMON_TYPE			emTYPE;
		DWORD				dwGUID;
		SNATIVEID			sSummonID;
		DWORD				dwOwner;					// 주인 GaeaID		
		SNATIVEID			sMapID;
		DWORD				dwCellID;
		EMUSE_SUMMON_FB		emFB;
		D3DXVECTOR3			vPos;
		D3DXVECTOR3			vDir;

		DWORD				dwNowHP;	//	생명량.
		WORD				wNowMP;		//	정신량.

		/*skill summon, Juver, 2017/10/09 */
		WORD				wIndex;
		SUMMON_DATA_SKILL	sSummon;

		SNETPC_REQ_USE_SUMMON_FB ()
			: emTYPE(SUMMON_TYPE_NONE)
			, dwGUID(UINT_MAX)
			, sSummonID(NATIVEID_NULL())
			, dwOwner(0)
			, sMapID(NATIVEID_NULL())
			, dwCellID(0)
			, vPos(0,0,0)
			, vDir(0,0,0)
			, emFB(EMUSE_SUMMON_FB_FAIL)
			, dwNowHP(0)
			, wNowMP(0)
			, wIndex(0) /*skill summon, Juver, 2017/10/09 */
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_REQ_USE_SUMMON_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_REQ_USE_SUMMON_DEL
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwGUID;

		SNETPC_REQ_USE_SUMMON_DEL () :
		dwGUID(UINT_MAX)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_REQ_USE_SUMMON_DEL;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_SUMMON_REQ_GOTO
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwGUID;
		D3DXVECTOR3			vCurPos;
		D3DXVECTOR3			vTarPos;
		DWORD				dwFlag;

		SNET_SUMMON_REQ_GOTO () :
		dwGUID(UINT_MAX),
			vCurPos(0,0,0),
			vTarPos(0,0,0),
			dwFlag(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_SUMMON_REQ_GOTO;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	struct SNET_SUMMON_BROAD
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwGUID;

		SNET_SUMMON_BROAD () :
		dwGUID(UINT_MAX)
		{
		}
	};

	struct SNET_SUMMON_REQ_STOP
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwGUID;
		DWORD				dwFlag;
		D3DXVECTOR3			vPos;
		bool				bStopAttack;

		SNET_SUMMON_REQ_STOP () :
			dwGUID(UINT_MAX),
			dwFlag(0),
			vPos(0,0,0),
			bStopAttack(false)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_SUMMON_REQ_STOP;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_SUMMON_CREATE_ANYSUMMON
	{
		NET_MSG_GENERIC		nmg;

		SDROPSUMMON			Data;

		SNET_SUMMON_CREATE_ANYSUMMON ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_CREATE_ANYSUMMON;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_SUMMON_DROP_SUMMON
	{
		NET_MSG_GENERIC		nmg;

		SDROPSUMMON			Data;

		SNET_SUMMON_DROP_SUMMON ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_DROP_ANYSUMMON;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*skill summon, Juver, 2017/10/09 */
	struct SNET_SUMMON_ATTACK
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwGUID;
		STARGETID			sTarID;
		DWORD				dwAType;

		SNATIVEID			sidSkill;
		WORD				wSkillLevel;
		D3DXVECTOR3			vTARPOS;
		WORD				wTARNUM;
		STARID				sTARIDS[EMTARGET_NET];
		
		SNET_SUMMON_ATTACK () :
			dwGUID(UINT_MAX),
			dwAType(0),
			sidSkill(NATIVEID_NULL()),
			wSkillLevel(0),
			vTARPOS(0,0,0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_SUMMON_ATTACK;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_SUMMON_ATTACK_BRD : public SNET_SUMMON_BROAD
	{
		STARGETID			sTarID;
		DWORD				dwAType;

		SNATIVEID			sidSkill;
		WORD				wSkillLevel;
		D3DXVECTOR3			vTARPOS;
		WORD				wTARNUM;
		STARID				sTARIDS[EMTARGET_NET];

		SNET_SUMMON_ATTACK_BRD () :
			dwAType(0),
			sidSkill(NATIVEID_NULL()),
			wSkillLevel(0),
			vTARPOS(0,0,0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_SUMMON_ATTACK_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	struct SNET_SUMMON_GOTO_BRD : public SNET_SUMMON_BROAD
	{
		D3DXVECTOR3			vCurPos;
		D3DXVECTOR3			vTarPos;
		DWORD				dwFlag;

		SNET_SUMMON_GOTO_BRD () :
		vCurPos(0,0,0),
			vTarPos(0,0,0),
			dwFlag(false)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_SUMMON_GOTO_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_SUMMON_STOP_BRD : public SNET_SUMMON_BROAD
	{
		D3DXVECTOR3			vPos;
		DWORD				dwFlag;
		bool				bStopAttack;

		SNET_SUMMON_STOP_BRD () :
		dwFlag(0),
			vPos(0,0,0),
			bStopAttack(false)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_SUMMON_REQ_STOP_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_SUMMON_REQ_UPDATE_MOVESTATE_BRD : public SNET_SUMMON_BROAD
	{
		DWORD				dwFlag;

		SNET_SUMMON_REQ_UPDATE_MOVESTATE_BRD () :
		dwFlag(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_SUMMON_REQ_UPDATE_MOVE_STATE_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_SUMMON_REQ_UPDATE_MOVESTATE
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwGUID;
		DWORD				dwFlag;

		SNET_SUMMON_REQ_UPDATE_MOVESTATE ()
			: dwGUID(UINT_MAX)
			, dwFlag(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_SUMMON_REQ_UPDATE_MOVE_STATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNET_SUMMON_ATTACK_AVOID
	{
		NET_MSG_GENERIC		nmg;

		EMCROW				emTarCrow;
		DWORD				dwTarID;

		SNET_SUMMON_ATTACK_AVOID () :
		emTarCrow(CROW_MOB),
			dwTarID(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SUMMON_ATTACK_AVOID;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNET_SUMMON_ATTACK_DAMAGE
	{
		NET_MSG_GENERIC		nmg;

		EMCROW				emTarCrow;
		DWORD				dwTarID;
		int					nDamage;
		DWORD				dwDamageFlag;

		SNET_SUMMON_ATTACK_DAMAGE () :
		emTarCrow(CROW_MOB),
			dwTarID(0),
			nDamage(0),
			dwDamageFlag( DAMAGE_TYPE_NONE )
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SUMMON_ATTACK_DAMAGE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	/*skill summon, Juver, 2017/10/09 */
	struct SNET_SUMMON_END_REST
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwGUID;

		SNET_SUMMON_END_REST () :
			dwGUID(UINT_MAX)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_SUMMON_END_REST;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*skill summon, Juver, 2017/10/09 */
	struct SNET_SUMMON_END_REST_BRD : public SNET_SUMMON_BROAD
	{
		STARGETID			sTarID;

		SNET_SUMMON_END_REST_BRD () 
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_SUMMON_END_REST_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*skill summon, Juver, 2017/10/09 */
	struct SNET_SUMMON_END_LIFE
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwGUID;

		SNET_SUMMON_END_LIFE () :
		dwGUID(UINT_MAX)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_SUMMON_END_LIFE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*skill summon, Juver, 2017/10/09 */
	struct SNET_SUMMON_END_LIFE_BRD : public SNET_SUMMON_BROAD
	{
		STARGETID			sTarID;

		SNET_SUMMON_END_LIFE_BRD () 
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_SUMMON_END_LIFE_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*skill summon, Juver, 2017/10/09 */
	struct SNET_SUMMON_RESET_TARGET
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwGUID;

		SNET_SUMMON_RESET_TARGET () :
			dwGUID(UINT_MAX)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_SUMMON_RESET_TARGET;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*skill summon, Juver, 2017/10/09 */
	struct SNET_SUMMON_RESET_TARGET_BRD : public SNET_SUMMON_BROAD
	{
		STARGETID			sTarID;

		SNET_SUMMON_RESET_TARGET_BRD () 
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_SUMMON_RESET_TARGET_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	// Revert to default structure packing
	#pragma pack()
};