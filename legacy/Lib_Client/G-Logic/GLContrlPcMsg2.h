#pragma once

#include "GLContrlBaseMsg.h"
#include "GLContrlPcMsgEnum.h"


namespace GLMSG
{
	#pragma pack(1)

	/*pk info, Juver, 2017/11/17 */
	struct SNETPC_UPDATE_PK_SCORE
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwVal;
		short				uPkCombo;
		char				szName[CHAR_SZNAME];
		
		SNETPC_UPDATE_PK_SCORE ()
			: dwVal(0),
			 uPkCombo(0)
		{
			memset (szName, 0, sizeof(char) * CHAR_SZNAME);

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_UPDATE_PK_SCORE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*pk info, Juver, 2017/11/17 */
	struct SNETPC_UPDATE_PK_DEATH
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwVal;
		char				szName[CHAR_SZNAME];

		SNETPC_UPDATE_PK_DEATH ()
			: dwVal(0)
		{
			memset (szName, 0, sizeof(char) * CHAR_SZNAME);

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_UPDATE_PK_DEATH;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};
	

	/*change scale card, Juver, 2018/01/04 */
	struct SNETPC_INVEN_SCALE_CHANGE
	{
		NET_MSG_GENERIC		nmg;
		float				fScale;

		SNETPC_INVEN_SCALE_CHANGE () 
			: fScale(1.0f)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_SCALE_CHANGE;
		}
	};

	/*change scale card, Juver, 2018/01/04 */
	struct SNETPC_INVEN_SCALE_CHANGE_FB
	{
		NET_MSG_GENERIC		nmg;

		EMFB_CHANGE_SCALE	emFB;
		float				fScale;

		SNETPC_INVEN_SCALE_CHANGE_FB ()
			: emFB(EMFB_CHANGE_SCALE_FAIL)
			, fScale(1.0f)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_SCALE_CHANGE_FB;
		}
	};

	/*change scale card, Juver, 2018/01/04 */
	struct SNETPC_INVENSCALE_CHANGE_BRD : public SNETPC_BROAD
	{
		float		fScale;

		SNETPC_INVENSCALE_CHANGE_BRD ()
			: fScale(1.0f)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_SCALE_CHANGE_BRD;
		}
	};

	/*item color, Juver, 2018/01/10 */
	struct SNETPC_INVEN_ITEMCOLOR_CHANGE
	{
		NET_MSG_GENERIC			nmg;

		EMSLOT	emSlot;
		WORD	wColor1;
		WORD	wColor2;

		SNETPC_INVEN_ITEMCOLOR_CHANGE () 
			: emSlot(SLOT_TSIZE)
			, wColor1(0)
			, wColor2(0)
		{
			nmg.dwSize = (DWORD) sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_ITEMCOLOR_CHANGE;
		}
	};

	/*item color, Juver, 2018/01/10 */
	struct SNETPC_INVEN_ITEMCOLOR_CHANGE_FB
	{
		NET_MSG_GENERIC			nmg;
		EMFB_ITEM_COLOR_CHANGE	emFB;
		EMSLOT	emSlot;
		WORD	wColor1;
		WORD	wColor2;

		SNETPC_INVEN_ITEMCOLOR_CHANGE_FB ()
			: emFB(EMFB_ITEM_COLOR_CHANGE_FAILED)
			, emSlot(SLOT_TSIZE)
			, wColor1(0)
			, wColor2(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_ITEMCOLOR_CHANGE_FB;
		}
	};

	/*item color, Juver, 2018/01/10 */
	struct SNETPC_INVEN_ITEMCOLOR_CHANGE_BRD : public SNETPC_BROAD
	{
		EMSLOT	emSlot;
		WORD	wColor1;
		WORD	wColor2;

		SNETPC_INVEN_ITEMCOLOR_CHANGE_BRD ()
			: emSlot(SLOT_TSIZE)
			, wColor1(0)
			, wColor2(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_ITEMCOLOR_CHANGE_BRD;
		}
	};

	/*item wrapper, Juver, 2018/01/12 */
	struct SNETPC_INVEN_WRAP
	{
		NET_MSG_GENERIC	nmg;
		WORD			wPosX;
		WORD			wPosY;

		SNETPC_INVEN_WRAP () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = (DWORD) sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_WRAP;
		}
	};

	/*item wrapper, Juver, 2018/01/12 */
	struct SNETPC_INVEN_WRAP_FB
	{
		NET_MSG_GENERIC		nmg;
		EMFB_ITEM_WRAP		emFB;
		
		SNETPC_INVEN_WRAP_FB ()
			: emFB(EMFB_ITEM_WRAP_FAILED)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_WRAP_FB;
		}
	};

	/*item wrapper, Juver, 2018/01/12 */
	struct SNETPC_INVEN_UNWRAP
	{
		NET_MSG_GENERIC	nmg;
		WORD			wPosX;
		WORD			wPosY;

		SNETPC_INVEN_UNWRAP () 
			: wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = (DWORD) sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_UNWRAP;
		}
	};

	/*item wrapper, Juver, 2018/01/12 */
	struct SNETPC_INVEN_UNWRAP_FB
	{
		NET_MSG_GENERIC		nmg;
		EMFB_ITEM_UNWRAP	emFB;

		SNETPC_INVEN_UNWRAP_FB ()
			: emFB(EMFB_ITEM_UNWRAP_FAILED)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_UNWRAP_FB;
		}
	};

	/*change school card, Juver, 2018/01/12 */
	struct SNETPC_INVEN_CHANGE_SCHOOL
	{
		NET_MSG_GENERIC	nmg;
		WORD			wSchool;

		SNETPC_INVEN_CHANGE_SCHOOL () 
			: wSchool(0)
		{
			nmg.dwSize = (DWORD) sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_CHANGE_SCHOOL;
		}
	};

	/*change school card, Juver, 2018/01/12 */
	struct SNETPC_INVEN_CHANGE_SCHOOL_FB
	{
		NET_MSG_GENERIC		nmg;
		EMFB_CHANGE_SCHOOL	emFB;

		SNETPC_INVEN_CHANGE_SCHOOL_FB ()
			: emFB(EMFB_CHANGE_SCHOOL_FAILED)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_CHANGE_SCHOOL_FB;
		}
	};

	/*equipment lock, Juver, 2018/01/13 */
	struct SNETPC_INVEN_EQUIPMENT_LOCK_ENABLE
	{
		NET_MSG_GENERIC		nmg;
		char				szPin1[CHAR_EQUIPMENT_LOCK_PASS_SIZE+1];
		char				szPin2[CHAR_EQUIPMENT_LOCK_PASS_SIZE+1];

		SNETPC_INVEN_EQUIPMENT_LOCK_ENABLE ()
		{
			memset (szPin1, 0, sizeof(char) * CHAR_EQUIPMENT_LOCK_PASS_SIZE+1);
			memset (szPin2, 0, sizeof(char) * CHAR_EQUIPMENT_LOCK_PASS_SIZE+1);

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_ENABLE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*equipment lock, Juver, 2018/01/14 */
	struct SNETPC_INVEN_EQUIPMENT_LOCK_ENABLE_FB
	{
		NET_MSG_GENERIC		nmg;
		EMFB_EQUIPMENT_LOCK_ENABLE emFB;
		BOOL		bEquipmentLockEnable;

		SNETPC_INVEN_EQUIPMENT_LOCK_ENABLE_FB ()
			: emFB(EMFB_EQUIPMENT_LOCK_ENABLE_FAILED)
			, bEquipmentLockEnable(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_ENABLE_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*equipment lock, Juver, 2018/01/14 */
	struct SNETPC_INVEN_EQUIPMENT_LOCK_ENABLE_FROM_DB
	{
		NET_MSG_GENERIC				nmg;
		EMFB_EQUIPMENT_LOCK_ENABLE	emFB;
		char						szName[CHAR_SZNAME+1];
		char						szPin[CHAR_EQUIPMENT_LOCK_PASS_SIZE+1];
		
		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+CHAR_SZNAME+1+CHAR_EQUIPMENT_LOCK_PASS_SIZE+1};
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPC_INVEN_EQUIPMENT_LOCK_ENABLE_FROM_DB () :
			emFB(EMFB_EQUIPMENT_LOCK_ENABLE_FAILED)
		{
			nmg.dwSize = sizeof(*this);
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			memset ( szPin, 0, sizeof(char)*CHAR_EQUIPMENT_LOCK_PASS_SIZE+1 );
			nmg.nType = NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_ENABLE_FROM_DB;
		}
	};

	/*equipment lock, Juver, 2018/01/16 */
	struct SNETPC_INVEN_EQUIPMENT_LOCK_INPUT
	{
		NET_MSG_GENERIC		nmg;
		char				szPin[CHAR_EQUIPMENT_LOCK_PASS_SIZE+1];

		SNETPC_INVEN_EQUIPMENT_LOCK_INPUT ()
		{
			memset (szPin, 0, sizeof(char) * CHAR_EQUIPMENT_LOCK_PASS_SIZE+1);

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_INPUT;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*equipment lock, Juver, 2018/01/16 */
	struct SNETPC_INVEN_EQUIPMENT_LOCK_INPUT_FB
	{
		NET_MSG_GENERIC				nmg;
		EMFB_EQUIPMENT_LOCK_INPUT	emFB;
		BOOL						bEquipmentLockStatus;

		SNETPC_INVEN_EQUIPMENT_LOCK_INPUT_FB ()
			: emFB(EMFB_EQUIPMENT_LOCK_INPUT_FAILED)
			, bEquipmentLockStatus(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_INPUT_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};
	
	/*equipment lock, Juver, 2018/01/16 */
	struct SNETPC_INVEN_EQUIPMENT_LOCK_INPUT_FROM_DB
	{
		NET_MSG_GENERIC				nmg;
		EMFB_EQUIPMENT_LOCK_INPUT	emFB;
		char						szName[CHAR_SZNAME+1];
		char						szPin[CHAR_EQUIPMENT_LOCK_PASS_SIZE+1];

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+CHAR_SZNAME+1+CHAR_EQUIPMENT_LOCK_PASS_SIZE+1};
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPC_INVEN_EQUIPMENT_LOCK_INPUT_FROM_DB () :
			emFB(EMFB_EQUIPMENT_LOCK_INPUT_FAILED)
		{
			nmg.dwSize = sizeof(*this);
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			memset ( szPin, 0, sizeof(char)*CHAR_EQUIPMENT_LOCK_PASS_SIZE+1 );
			nmg.nType = NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_INPUT_FROM_DB;
		}
	};

	/*equipment lock, Juver, 2018/01/16 */
	struct SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER
	{
		NET_MSG_GENERIC		nmg;
		SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*equipment lock, Juver, 2018/01/16 */
	struct SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_FB
	{
		NET_MSG_GENERIC				nmg;
		EMFB_EQUIPMENT_LOCK_RECOVER	emFB;
		char						szName[CHAR_SZNAME+1];
		char						szPin[CHAR_EQUIPMENT_LOCK_PASS_SIZE+1];

		SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_FB ()
			: emFB(EMFB_EQUIPMENT_LOCK_RECOVER_FAILED)
		{
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			memset ( szPin, 0, sizeof(char)*CHAR_EQUIPMENT_LOCK_PASS_SIZE+1 );

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*equipment lock, Juver, 2018/01/16 */
	struct SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_FROM_DB
	{
		NET_MSG_GENERIC				nmg;
		EMFB_EQUIPMENT_LOCK_RECOVER	emFB;
		char						szName[CHAR_SZNAME+1];
		char						szPin[CHAR_EQUIPMENT_LOCK_PASS_SIZE+1];

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+CHAR_SZNAME+1+CHAR_EQUIPMENT_LOCK_PASS_SIZE+1};
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_FROM_DB () :
			emFB(EMFB_EQUIPMENT_LOCK_RECOVER_FAILED)
		{
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			memset ( szPin, 0, sizeof(char)*CHAR_EQUIPMENT_LOCK_PASS_SIZE+1 );
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_FROM_DB;
		}
	};

	/*equipment lock, Juver, 2018/01/17 */
	struct SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE
	{
		NET_MSG_GENERIC		nmg;
		char				szPin1[CHAR_EQUIPMENT_LOCK_PASS_SIZE+1];
		char				szPin2[CHAR_EQUIPMENT_LOCK_PASS_SIZE+1];

		SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE ()
		{
			memset (szPin1, 0, sizeof(char) * CHAR_EQUIPMENT_LOCK_PASS_SIZE+1);
			memset (szPin2, 0, sizeof(char) * CHAR_EQUIPMENT_LOCK_PASS_SIZE+1);

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*equipment lock, Juver, 2018/01/17 */
	struct SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE_FB
	{
		NET_MSG_GENERIC						nmg;
		EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE	emFB;

		SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE_FB ()
			: emFB(EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_FAILED)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*equipment lock, Juver, 2018/01/18 */
	struct SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE_FROM_DB
	{
		NET_MSG_GENERIC						nmg;
		EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE	emFB;
		char								szName[CHAR_SZNAME+1];
		char								szPin[CHAR_EQUIPMENT_LOCK_PASS_SIZE+1];

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+CHAR_SZNAME+1+CHAR_EQUIPMENT_LOCK_PASS_SIZE+1};
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE_FROM_DB () :
			emFB(EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_FAILED)
		{
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			memset ( szPin, 0, sizeof(char)*CHAR_EQUIPMENT_LOCK_PASS_SIZE+1 );
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE_FROM_DB;
		}
	};

	/*equipment lock, Juver, 2018/01/18 */
	struct SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE
	{
		NET_MSG_GENERIC		nmg;

		SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*equipment lock, Juver, 2018/01/18 */
	struct SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE_FB
	{
		NET_MSG_GENERIC						nmg;
		EMFB_EQUIPMENT_LOCK_RECOVER_DELETE	emFB;
		BOOL		bEquipmentLockEnable;
		BOOL		bEquipmentLockStatus;
		
		SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE_FB ()
			: emFB(EMFB_EQUIPMENT_LOCK_RECOVER_DELETE_FAILED)
			, bEquipmentLockEnable(FALSE)
			, bEquipmentLockStatus(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*equipment lock, Juver, 2018/01/18 */
	struct SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE_FROM_DB
	{
		NET_MSG_GENERIC						nmg;
		EMFB_EQUIPMENT_LOCK_RECOVER_DELETE	emFB;
		char								szName[CHAR_SZNAME+1];

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+CHAR_SZNAME+1};
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE_FROM_DB () :
			emFB(EMFB_EQUIPMENT_LOCK_RECOVER_DELETE_FAILED)
		{
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE_FROM_DB;
		}
	};


	/*item transfer card, Juver, 2018/01/18 */
	struct SNETPC_INVEN_TRANSFER_STATS
	{
		NET_MSG_GENERIC		nmg;
		SINVEN_POS			sInvenPosOLD;
		SINVEN_POS			sInvenPosNEW;

		SNETPC_INVEN_TRANSFER_STATS ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_ITEM_TRANSFER;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*equipment lock, Juver, 2018/01/17 */
	struct SNETPC_INVEN_TRANSFER_STATS_FB
	{
		NET_MSG_GENERIC		nmg;
		EMFB_ITEM_TRANSFER	emFB;

		SNETPC_INVEN_TRANSFER_STATS_FB ()
			: emFB(EMFB_ITEM_TRANSFER_FAILED)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVEN_ITEM_TRANSFER_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*dmk14 remove active buff selection*/
	struct SNETPC_REQ_DISABLESKILLEFF
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwSKILL;

		SNETPC_REQ_DISABLESKILLEFF () :
			dwSKILL(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_REQ_DISABLESKILLEFF;
		}
	};

	struct SNETPC_REQ_DISABLESKILLEFF_FB
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwSKILL;

		SNETPC_REQ_DISABLESKILLEFF_FB () :
			dwSKILL(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_REQ_DISABLESKILLEFF_FB;
		}
	};

	struct SNETPC_REQ_DISABLESKILLEFF_BRD : public SNETPC_BROAD
	{
		DWORD dwSKILL;

		SNETPC_REQ_DISABLESKILLEFF_BRD ():
			dwSKILL(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_REQ_DISABLESKILLEFF_BRD;
		}
	};

	struct SNETPC_REQ_DISABLEITEMFACT
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwSKILL;

		SNETPC_REQ_DISABLEITEMFACT () :
			dwSKILL(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_REQ_DISABLEITEMFACT;
		}
	};

	struct SNETPC_REQ_DISABLEITEMFACT_FB
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwSKILL;

		SNETPC_REQ_DISABLEITEMFACT_FB () :
			dwSKILL(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_REQ_DISABLEITEMFACT_FB;
		}
	};

	struct SNETPC_REQ_DISABLEITEMFACT_BRD : public SNETPC_BROAD
	{
		DWORD dwSKILL;

		SNETPC_REQ_DISABLEITEMFACT_BRD ():
			dwSKILL(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_REQ_DISABLEITEMFACT_BRD;
		}
	};

	/*storage lock, Juver, 2018/01/13 */
	struct SNETPC_STORAGE_LOCK_ENABLE
	{
		NET_MSG_GENERIC		nmg;
		char				szPin1[USER_STORAGE_LOCK_PASS_SIZE+1];
		char				szPin2[USER_STORAGE_LOCK_PASS_SIZE+1];

		SNETPC_STORAGE_LOCK_ENABLE ()
		{
			memset (szPin1, 0, sizeof(char) * USER_STORAGE_LOCK_PASS_SIZE+1);
			memset (szPin2, 0, sizeof(char) * USER_STORAGE_LOCK_PASS_SIZE+1);

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_STORAGE_LOCK_ENABLE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*storage lock, Juver, 2018/01/14 */
	struct SNETPC_STORAGE_LOCK_ENABLE_FB
	{
		NET_MSG_GENERIC		nmg;
		EMFB_STORAGE_LOCK_ENABLE emFB;
		BOOL		bStorageLockEnable;

		SNETPC_STORAGE_LOCK_ENABLE_FB ()
			: emFB(EMFB_STORAGE_LOCK_ENABLE_FAILED)
			, bStorageLockEnable(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_STORAGE_LOCK_ENABLE_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*storage lock, Juver, 2018/01/14 */
	struct SNETPC_STORAGE_LOCK_ENABLE_FROM_DB
	{
		NET_MSG_GENERIC				nmg;
		EMFB_STORAGE_LOCK_ENABLE	emFB;
		char						szName[CHAR_SZNAME+1];
		char						szPin[USER_STORAGE_LOCK_PASS_SIZE+1];
		
		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+CHAR_SZNAME+1+USER_STORAGE_LOCK_PASS_SIZE+1};
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPC_STORAGE_LOCK_ENABLE_FROM_DB () :
			emFB(EMFB_STORAGE_LOCK_ENABLE_FAILED)
		{
			nmg.dwSize = sizeof(*this);
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			memset ( szPin, 0, sizeof(char)*USER_STORAGE_LOCK_PASS_SIZE+1 );
			nmg.nType = NET_MSG_GCTRL_STORAGE_LOCK_ENABLE_FROM_DB;
		}
	};

	/*storage lock, Juver, 2018/01/16 */
	struct SNETPC_STORAGE_LOCK_INPUT
	{
		NET_MSG_GENERIC		nmg;
		char				szPin[USER_STORAGE_LOCK_PASS_SIZE+1];

		SNETPC_STORAGE_LOCK_INPUT ()
		{
			memset (szPin, 0, sizeof(char) * USER_STORAGE_LOCK_PASS_SIZE+1);

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_STORAGE_LOCK_INPUT;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*storage lock, Juver, 2018/01/16 */
	struct SNETPC_STORAGE_LOCK_INPUT_FB
	{
		NET_MSG_GENERIC				nmg;
		EMFB_STORAGE_LOCK_INPUT		emFB;
		BOOL						bStorageLockStatus;

		SNETPC_STORAGE_LOCK_INPUT_FB ()
			: emFB(EMFB_STORAGE_LOCK_INPUT_FAILED)
			, bStorageLockStatus(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_STORAGE_LOCK_INPUT_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};
	
	/*storage lock, Juver, 2018/01/16 */
	struct SNETPC_STORAGE_LOCK_INPUT_FROM_DB
	{
		NET_MSG_GENERIC				nmg;
		EMFB_STORAGE_LOCK_INPUT		emFB;
		char						szName[CHAR_SZNAME+1];
		char						szPin[USER_STORAGE_LOCK_PASS_SIZE+1];

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+CHAR_SZNAME+1+USER_STORAGE_LOCK_PASS_SIZE+1};
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPC_STORAGE_LOCK_INPUT_FROM_DB () :
			emFB(EMFB_STORAGE_LOCK_INPUT_FAILED)
		{
			nmg.dwSize = sizeof(*this);
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			memset ( szPin, 0, sizeof(char)*USER_STORAGE_LOCK_PASS_SIZE+1 );
			nmg.nType = NET_MSG_GCTRL_STORAGE_LOCK_INPUT_FROM_DB;
		}
	};

	/*storage lock, Juver, 2018/01/16 */
	struct SNETPC_STORAGE_LOCK_RECOVER
	{
		NET_MSG_GENERIC		nmg;
		SNETPC_STORAGE_LOCK_RECOVER ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_STORAGE_LOCK_RECOVER;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*storage lock, Juver, 2018/01/16 */
	struct SNETPC_STORAGE_LOCK_RECOVER_FB
	{
		NET_MSG_GENERIC				nmg;
		EMFB_STORAGE_LOCK_RECOVER	emFB;
		char						szName[CHAR_SZNAME+1];
		char						szPin[USER_STORAGE_LOCK_PASS_SIZE+1];

		SNETPC_STORAGE_LOCK_RECOVER_FB ()
			: emFB(EMFB_STORAGE_LOCK_RECOVER_FAILED)
		{
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			memset ( szPin, 0, sizeof(char)*USER_STORAGE_LOCK_PASS_SIZE+1 );

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*storage lock, Juver, 2018/01/16 */
	struct SNETPC_STORAGE_LOCK_RECOVER_FROM_DB
	{
		NET_MSG_GENERIC				nmg;
		EMFB_STORAGE_LOCK_RECOVER	emFB;
		char						szName[CHAR_SZNAME+1];
		char						szPin[USER_STORAGE_LOCK_PASS_SIZE+1];

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+CHAR_SZNAME+1+USER_STORAGE_LOCK_PASS_SIZE+1};
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPC_STORAGE_LOCK_RECOVER_FROM_DB () :
			emFB(EMFB_STORAGE_LOCK_RECOVER_FAILED)
		{
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			memset ( szPin, 0, sizeof(char)*USER_STORAGE_LOCK_PASS_SIZE+1 );
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_FROM_DB;
		}
	};

	/*storage lock, Juver, 2018/01/17 */
	struct SNETPC_STORAGE_LOCK_RECOVER_CHANGE
	{
		NET_MSG_GENERIC		nmg;
		char				szPin1[USER_STORAGE_LOCK_PASS_SIZE+1];
		char				szPin2[USER_STORAGE_LOCK_PASS_SIZE+1];

		SNETPC_STORAGE_LOCK_RECOVER_CHANGE ()
		{
			memset (szPin1, 0, sizeof(char) * USER_STORAGE_LOCK_PASS_SIZE+1);
			memset (szPin2, 0, sizeof(char) * USER_STORAGE_LOCK_PASS_SIZE+1);

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_CHANGE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*storage lock, Juver, 2018/01/17 */
	struct SNETPC_STORAGE_LOCK_RECOVER_CHANGE_FB
	{
		NET_MSG_GENERIC						nmg;
		EMFB_STORAGE_LOCK_RECOVER_CHANGE	emFB;

		SNETPC_STORAGE_LOCK_RECOVER_CHANGE_FB ()
			: emFB(EMFB_STORAGE_LOCK_RECOVER_CHANGE_FAILED)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_CHANGE_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*storage lock, Juver, 2018/01/18 */
	struct SNETPC_STORAGE_LOCK_RECOVER_CHANGE_FROM_DB
	{
		NET_MSG_GENERIC						nmg;
		EMFB_STORAGE_LOCK_RECOVER_CHANGE	emFB;
		char								szName[CHAR_SZNAME+1];
		char								szPin[USER_STORAGE_LOCK_PASS_SIZE+1];

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+CHAR_SZNAME+1+USER_STORAGE_LOCK_PASS_SIZE+1};
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPC_STORAGE_LOCK_RECOVER_CHANGE_FROM_DB () :
			emFB(EMFB_STORAGE_LOCK_RECOVER_CHANGE_FAILED)
		{
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			memset ( szPin, 0, sizeof(char)*USER_STORAGE_LOCK_PASS_SIZE+1 );
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_CHANGE_FROM_DB;
		}
	};

	/*storage lock, Juver, 2018/01/18 */
	struct SNETPC_STORAGE_LOCK_RECOVER_DELETE
	{
		NET_MSG_GENERIC		nmg;

		SNETPC_STORAGE_LOCK_RECOVER_DELETE ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_DELETE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*storage lock, Juver, 2018/01/18 */
	struct SNETPC_STORAGE_LOCK_RECOVER_DELETE_FB
	{
		NET_MSG_GENERIC						nmg;
		EMFB_STORAGE_LOCK_RECOVER_DELETE	emFB;
		BOOL		bStorageLockEnable;
		BOOL		bStorageLockStatus;
		
		SNETPC_STORAGE_LOCK_RECOVER_DELETE_FB ()
			: emFB(EMFB_STORAGE_LOCK_RECOVER_DELETE_FAILED)
			, bStorageLockEnable(FALSE)
			, bStorageLockStatus(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_DELETE_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*storage lock, Juver, 2018/01/18 */
	struct SNETPC_STORAGE_LOCK_RECOVER_DELETE_FROM_DB
	{
		NET_MSG_GENERIC						nmg;
		EMFB_STORAGE_LOCK_RECOVER_DELETE	emFB;
		char								szName[CHAR_SZNAME+1];

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+CHAR_SZNAME+1};
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPC_STORAGE_LOCK_RECOVER_DELETE_FROM_DB () :
			emFB(EMFB_STORAGE_LOCK_RECOVER_DELETE_FAILED)
		{
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_DELETE_FROM_DB;
		}
	};

	/*inventory lock, Juver, 2018/01/14 */
	struct SNETPC_INVENTORY_LOCK_ENABLE
	{
		NET_MSG_GENERIC		nmg;
		char				szPin1[CHAR_INVENTORY_LOCK_PASS_SIZE+1];
		char				szPin2[CHAR_INVENTORY_LOCK_PASS_SIZE+1];

		SNETPC_INVENTORY_LOCK_ENABLE ()
		{
			memset (szPin1, 0, sizeof(char) * CHAR_INVENTORY_LOCK_PASS_SIZE+1);
			memset (szPin2, 0, sizeof(char) * CHAR_INVENTORY_LOCK_PASS_SIZE+1);

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVENTORY_LOCK_ENABLE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*inventory lock, Juver, 2018/01/14 */
	struct SNETPC_INVENTORY_LOCK_ENABLE_FB
	{
		NET_MSG_GENERIC		nmg;
		EMFB_INVENTORY_LOCK_ENABLE emFB;
		BOOL		bInventoryLockEnable;

		SNETPC_INVENTORY_LOCK_ENABLE_FB ()
			: emFB(EMFB_INVENTORY_LOCK_ENABLE_FAILED)
			, bInventoryLockEnable(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVENTORY_LOCK_ENABLE_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*inventory lock, Juver, 2018/01/14 */
	struct SNETPC_INVENTORY_LOCK_ENABLE_FROM_DB
	{
		NET_MSG_GENERIC				nmg;
		EMFB_INVENTORY_LOCK_ENABLE	emFB;
		char						szName[CHAR_SZNAME+1];
		char						szPin[CHAR_INVENTORY_LOCK_PASS_SIZE+1];
		
		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+CHAR_SZNAME+1+CHAR_INVENTORY_LOCK_PASS_SIZE+1};
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPC_INVENTORY_LOCK_ENABLE_FROM_DB () :
			emFB(EMFB_INVENTORY_LOCK_ENABLE_FAILED)
		{
			nmg.dwSize = sizeof(*this);
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			memset ( szPin, 0, sizeof(char)*CHAR_INVENTORY_LOCK_PASS_SIZE+1 );
			nmg.nType = NET_MSG_GCTRL_INVENTORY_LOCK_ENABLE_FROM_DB;
		}
	};

	/*inventory lock, Juver, 2018/01/16 */
	struct SNETPC_INVENTORY_LOCK_INPUT
	{
		NET_MSG_GENERIC		nmg;
		char				szPin[CHAR_INVENTORY_LOCK_PASS_SIZE+1];

		SNETPC_INVENTORY_LOCK_INPUT ()
		{
			memset (szPin, 0, sizeof(char) * CHAR_INVENTORY_LOCK_PASS_SIZE+1);

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVENTORY_LOCK_INPUT;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*inventory lock, Juver, 2018/01/16 */
	struct SNETPC_INVENTORY_LOCK_INPUT_FB
	{
		NET_MSG_GENERIC				nmg;
		EMFB_INVENTORY_LOCK_INPUT		emFB;
		BOOL						bInventoryLockStatus;

		SNETPC_INVENTORY_LOCK_INPUT_FB ()
			: emFB(EMFB_INVENTORY_LOCK_INPUT_FAILED)
			, bInventoryLockStatus(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVENTORY_LOCK_INPUT_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};
	
	/*inventory lock, Juver, 2018/01/16 */
	struct SNETPC_INVENTORY_LOCK_INPUT_FROM_DB
	{
		NET_MSG_GENERIC				nmg;
		EMFB_INVENTORY_LOCK_INPUT		emFB;
		char						szName[CHAR_SZNAME+1];
		char						szPin[CHAR_INVENTORY_LOCK_PASS_SIZE+1];

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+CHAR_SZNAME+1+CHAR_INVENTORY_LOCK_PASS_SIZE+1};
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPC_INVENTORY_LOCK_INPUT_FROM_DB () :
			emFB(EMFB_INVENTORY_LOCK_INPUT_FAILED)
		{
			nmg.dwSize = sizeof(*this);
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			memset ( szPin, 0, sizeof(char)*CHAR_INVENTORY_LOCK_PASS_SIZE+1 );
			nmg.nType = NET_MSG_GCTRL_INVENTORY_LOCK_INPUT_FROM_DB;
		}
	};

	/*inventory lock, Juver, 2018/01/16 */
	struct SNETPC_INVENTORY_LOCK_RECOVER
	{
		NET_MSG_GENERIC		nmg;
		SNETPC_INVENTORY_LOCK_RECOVER ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*inventory lock, Juver, 2018/01/16 */
	struct SNETPC_INVENTORY_LOCK_RECOVER_FB
	{
		NET_MSG_GENERIC				nmg;
		EMFB_INVENTORY_LOCK_RECOVER	emFB;
		char						szName[CHAR_SZNAME+1];
		char						szPin[CHAR_INVENTORY_LOCK_PASS_SIZE+1];

		SNETPC_INVENTORY_LOCK_RECOVER_FB ()
			: emFB(EMFB_INVENTORY_LOCK_RECOVER_FAILED)
		{
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			memset ( szPin, 0, sizeof(char)*CHAR_INVENTORY_LOCK_PASS_SIZE+1 );

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*inventory lock, Juver, 2018/01/16 */
	struct SNETPC_INVENTORY_LOCK_RECOVER_FROM_DB
	{
		NET_MSG_GENERIC				nmg;
		EMFB_INVENTORY_LOCK_RECOVER	emFB;
		char						szName[CHAR_SZNAME+1];
		char						szPin[CHAR_INVENTORY_LOCK_PASS_SIZE+1];

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+CHAR_SZNAME+1+CHAR_INVENTORY_LOCK_PASS_SIZE+1};
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPC_INVENTORY_LOCK_RECOVER_FROM_DB () :
			emFB(EMFB_INVENTORY_LOCK_RECOVER_FAILED)
		{
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			memset ( szPin, 0, sizeof(char)*CHAR_INVENTORY_LOCK_PASS_SIZE+1 );
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_FROM_DB;
		}
	};

	/*inventory lock, Juver, 2018/01/17 */
	struct SNETPC_INVENTORY_LOCK_RECOVER_CHANGE
	{
		NET_MSG_GENERIC		nmg;
		char				szPin1[CHAR_INVENTORY_LOCK_PASS_SIZE+1];
		char				szPin2[CHAR_INVENTORY_LOCK_PASS_SIZE+1];

		SNETPC_INVENTORY_LOCK_RECOVER_CHANGE ()
		{
			memset (szPin1, 0, sizeof(char) * CHAR_INVENTORY_LOCK_PASS_SIZE+1);
			memset (szPin2, 0, sizeof(char) * CHAR_INVENTORY_LOCK_PASS_SIZE+1);

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_CHANGE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*inventory lock, Juver, 2018/01/17 */
	struct SNETPC_INVENTORY_LOCK_RECOVER_CHANGE_FB
	{
		NET_MSG_GENERIC						nmg;
		EMFB_INVENTORY_LOCK_RECOVER_CHANGE	emFB;

		SNETPC_INVENTORY_LOCK_RECOVER_CHANGE_FB ()
			: emFB(EMFB_INVENTORY_LOCK_RECOVER_CHANGE_FAILED)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_CHANGE_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*inventory lock, Juver, 2018/01/18 */
	struct SNETPC_INVENTORY_LOCK_RECOVER_CHANGE_FROM_DB
	{
		NET_MSG_GENERIC						nmg;
		EMFB_INVENTORY_LOCK_RECOVER_CHANGE	emFB;
		char								szName[CHAR_SZNAME+1];
		char								szPin[CHAR_INVENTORY_LOCK_PASS_SIZE+1];

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+CHAR_SZNAME+1+CHAR_INVENTORY_LOCK_PASS_SIZE+1};
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPC_INVENTORY_LOCK_RECOVER_CHANGE_FROM_DB () :
			emFB(EMFB_INVENTORY_LOCK_RECOVER_CHANGE_FAILED)
		{
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			memset ( szPin, 0, sizeof(char)*CHAR_INVENTORY_LOCK_PASS_SIZE+1 );
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_CHANGE_FROM_DB;
		}
	};

	/*inventory lock, Juver, 2018/01/18 */
	struct SNETPC_INVENTORY_LOCK_RECOVER_DELETE
	{
		NET_MSG_GENERIC		nmg;

		SNETPC_INVENTORY_LOCK_RECOVER_DELETE ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_DELETE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*inventory lock, Juver, 2018/01/18 */
	struct SNETPC_INVENTORY_LOCK_RECOVER_DELETE_FB
	{
		NET_MSG_GENERIC						nmg;
		EMFB_INVENTORY_LOCK_RECOVER_DELETE	emFB;
		BOOL		bInventoryLockEnable;
		BOOL		bInventoryLockStatus;
		
		SNETPC_INVENTORY_LOCK_RECOVER_DELETE_FB ()
			: emFB(EMFB_INVENTORY_LOCK_RECOVER_DELETE_FAILED)
			, bInventoryLockEnable(FALSE)
			, bInventoryLockStatus(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_DELETE_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*inventory lock, Juver, 2018/01/18 */
	struct SNETPC_INVENTORY_LOCK_RECOVER_DELETE_FROM_DB
	{
		NET_MSG_GENERIC						nmg;
		EMFB_INVENTORY_LOCK_RECOVER_DELETE	emFB;
		char								szName[CHAR_SZNAME+1];

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+CHAR_SZNAME+1};
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPC_INVENTORY_LOCK_RECOVER_DELETE_FROM_DB () :
			emFB(EMFB_INVENTORY_LOCK_RECOVER_DELETE_FAILED)
		{
			memset ( szName, 0, sizeof(char)*CHAR_SZNAME+1 );
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_DELETE_FROM_DB;
		}
	};

	// Revert to default structure packing
	#pragma pack()
};
