#pragma once

#include "GLContrlBaseMsg.h"

/*pvp tyranny, Juver, 2017/08/25 */
#include "GLPVPTyrannyDefine.h"  

//namespace GLMSG
//{
//	#pragma pack(1)
//
//
//	// Revert to default structure packing
//	#pragma pack()
//};

namespace GLMSG
{
	#pragma pack(1)

	//---------------------------------------------------------------------------NET
	struct SNETCROW
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwGlobID;

		SNETCROW () 
			: dwGlobID(0)
		{
		}
	};


	//---------------------------------------------------------------------------NET
	struct SNETCROW_MOVETO : public SNETCROW
	{
		DWORD				dwActState;
		D3DXVECTOR3			vCurPos;
		D3DXVECTOR3			vTarPos;
		DWORD				dwCellID;
		BOOL				bTrace;	  // 추격을 위한 이동인지 유무

		SNETCROW_MOVETO () 
			: dwActState(NULL)
			, vCurPos(0,0,0)
			, vTarPos(0,0,0)
			, dwCellID(0)
			, bTrace(FALSE)
		{
			nmg.dwSize = sizeof(SNETCROW_MOVETO);
			nmg.nType = NET_MSG_GCTRL_CROW_MOVETO;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETCROW_ATTACK : public SNETCROW
	{
		EMCROW				emTarCrow;
		DWORD				dwTarID;
		DWORD				dwAType;

		SNETCROW_ATTACK () 
			: emTarCrow(CROW_PC)
			, dwTarID(0)
			, dwAType(0)
		{
			nmg.dwSize = sizeof(SNETCROW_ATTACK);
			nmg.nType = NET_MSG_GCTRL_CROW_ATTACK;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETCROW_AVOID : public SNETCROW
	{
		EMCROW				emTarCrow;
		DWORD				dwTarID;

		SNETCROW_AVOID () 
			: emTarCrow(CROW_PC)
			, dwTarID(0)
		{
			nmg.dwSize = sizeof(SNETCROW_AVOID);
			nmg.nType = NET_MSG_GCTRL_CROW_AVOID;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	//---------------------------------------------------------------------------NET
	struct SNETCROW_DAMAGE : public SNETCROW
	{
		EMCROW				emTarCrow;
		DWORD				dwTarID;
		int					nDamage;
		DWORD				dwNowHP;
		DWORD				dwDamageFlag;

		SNETCROW_DAMAGE () 
			: emTarCrow(CROW_PC)
			, dwTarID(0)
			, nDamage(0)
			, dwNowHP(0)
			, dwDamageFlag(DAMAGE_TYPE_NONE)
		{
			nmg.dwSize = sizeof(SNETCROW_DAMAGE);
			nmg.nType = NET_MSG_GCTRL_CROW_DAMAGE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETCROW_SKILL : public SNETCROW
	{
		DWORD				dwAType;
		SNATIVEID			skill_id;
		WORD				skill_lev;
	
		D3DXVECTOR3			vTARPOS;
		WORD				wTARNUM;
		STARID				sTARIDS[EMTARGET_NET];

		SNETCROW_SKILL () 
			: SNETCROW()
			, dwAType(0)
			, skill_id(0,0)
			, skill_lev(0)
			, vTARPOS(0,0,0)
			, wTARNUM(0)
		{
			nmg.dwSize = sizeof(SNETCROW_SKILL);
			nmg.nType = NET_MSG_GCTRL_CROW_SKILL;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}

		void CALCSIZE ()
		{
			nmg.dwSize = sizeof(SNETCROW_SKILL) - ( sizeof(STARID) * (EMTARGET_NET - wTARNUM) );
		}

		BOOL ADDTARGET ( EMCROW emCrow, DWORD dwID )
		{
			if ( wTARNUM>=EMTARGET_NET )
			{
				return FALSE;
			}
			else
			{
				sTARIDS[wTARNUM].wCrow = static_cast<EMCROW>(emCrow);
				sTARIDS[wTARNUM].wID = static_cast<WORD>(dwID);			
				wTARNUM++;
				CALCSIZE ();
				return TRUE;
			}
		}

		BOOL ADDTARGET ( const STARID &starget )
		{
			return ADDTARGET(starget.GETCROW(),starget.GETID());
		}
	};

	/*pvp tyranny, Juver, 2017/08/25 */
	struct SNETCROW_TYRANNY_OWNER : public SNETCROW
	{
		WORD	wTyrannyOwner;

		SNETCROW_TYRANNY_OWNER () 
			: wTyrannyOwner( TYRANNY_SCHOOL_SIZE )
		{
			nmg.dwSize = sizeof(SNETCROW_TYRANNY_OWNER);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_CROW_OWNER;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*pvp tyranny, Juver, 2017/08/25 */
	struct SNETCROW_TYRANNY_DAMAGE : public SNETCROW
	{
		float fTyrannyDamage[TYRANNY_SCHOOL_SIZE];

		SNETCROW_TYRANNY_DAMAGE () 
		{
			for ( int i=0; i< TYRANNY_SCHOOL_SIZE; ++ i )
				fTyrannyDamage[i] = 0.0f;

			nmg.dwSize = sizeof(SNETCROW_TYRANNY_DAMAGE);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_CROW_DAMAGE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	/*pvp capture the flag, Juver, 2018/02/06 */
	struct SNETCROW_CAPTURE_THE_FLAG_SET_HIDE : public SNETCROW
	{
		bool bHide;

		SNETCROW_CAPTURE_THE_FLAG_SET_HIDE () 
			: bHide( false )
		{
			nmg.dwSize = sizeof(SNETCROW_CAPTURE_THE_FLAG_SET_HIDE);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_CROW_HIDE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	// Revert to default structure packing
	#pragma pack()
};

