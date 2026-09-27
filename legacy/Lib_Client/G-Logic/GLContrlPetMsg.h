#pragma once

#include "GLContrlBaseMsg.h"
#include "GLPet.h"
//#include "GLSkill.h"

enum EMPET_USECARD_FB
{
	EMPET_USECARD_FB_OK			   = 0,
	EMPET_USECARD_FB_FAIL		   = 1,
	EMPET_USECARD_FB_INVALIDCARD   = 2,
	EMPET_USECARD_FB_NOITEM		   = 3,
	EMPET_USECARD_FB_NOTENOUGHFULL = 4,
	EMPET_USECARD_FB_INVALIDZONE   = 5,
	EMPET_USECARD_FB_NODATA		   = 6,
};

enum EMPET_REQ_GETRIGHTOFITEM_FB
{
	EMPET_REQ_GETRIGHTOFITEM_FB_OK   = 0,
	EMPET_REQ_GETRIGHTOFITEM_FB_FAIL = 1,
};

enum EMPET_PETSKINPACKOPEN_FB
{
	EMPET_PETSKINPACKOPEN_FB_FAIL			= 0,	//	일반오류.
	EMPET_PETSKINPACKOPEN_FB_PETCARD_FAIL	= 1,	//	펫카드오류.
	EMPET_PETSKINPACKOPEN_FB_OK			= 2,	//	성공.
	EMPET_PETSKINPACKOPEN_FB_END			= 3,	//	시간 종료
};

enum EMPET_REQ_GIVEFOOD_FB
{
	EMPET_REQ_GIVEFOOD_FB_OK		  = 0,
	EMPET_REQ_GIVEFOOD_FB_FAIL		  = 1,
	EMPET_REQ_GIVEFOOD_FB_INVALIDFOOD = 2,
	EMPET_REQ_GIVEFOOD_FB_INVALIDCARD = 3,
	EMPET_REQ_GETFULL_FROMDB_OK		  = 4,
	EMPET_REQ_GETFULL_FROMDB_FAIL	  = 5,
};

enum EMPET_REQ_RENAME_FB
{
	EMPET_REQ_RENAME_FB_FB_FAIL		= 0,		//	일반오류.
	EMPET_REQ_RENAME_FB_FB_OK		= 1,		//	성공.
	EMPET_REQ_RENAME_FB_FB_NOITEM	= 2,		//	아이템이 없음.
	EMPET_REQ_RENAME_FB_FB_BADITEM	= 3,		//	사용 불가능 아이템.
	EMPET_REQ_RENAME_FB_FB_LENGTH	= 4,		//	이름의 길이가 4자 미만
	EMPET_REQ_RENAME_FB_FROM_DB_OK	= 5,		//	성공.
	EMPET_REQ_RENAME_FB_FROM_DB_FAIL	= 6,	//	실패.(이름 중복)
	EMPET_REQ_RENAME_FB_FB_THAICHAR_ERROR = 7,  //  태국어 문자 조합 에러
	EMPET_REQ_RENAME_FB_FB_VNCHAR_ERROR = 8,	//  베트남 문자 조합 에러
	EMPET_REQ_RENAME_FB_BAD_NAME	= 9,
};

enum EMPET_REQ_STYLECOLOR_CHANGE_FB
{
	EMPET_REQ_STYLECOLOR_CHANGE_FB_FAIL		= 0,	//	일반오류.
	EMPET_REQ_STYLE_CHANGE_FB_OK			= 1,	//	성공.
	EMPET_REQ_COLOR_CHANGE_FB_OK			= 2,	//	성공.
	EMPET_REQ_STYLECOLOR_CHANGE_FB_NOITEM	= 3,	//	아이템이 없음.
	EMPET_REQ_STYLECOLOR_CHANGE_FB_BADITEM	= 4,	//	사용 불가능 아이템.
	EMPET_REQ_STYLECOLOR_CHANGE_FB_UNCHANGEABLE	= 5,	//	컬러변경이 불가능한 팻.
	EMPET_REQ_STYLECOLOR_CHANGE_FB_COOLTIME	= 6,	//	컬러변경이 불가능한 팻.
	EMPET_REQ_STYLECOLOR_CHANGE_FB_SKINPACK	= 7,	//	스킨팩 사용중
};

enum EMPET_REQ_SLOT_EX_HOLD_FB
{
	EMPET_REQ_SLOT_EX_HOLD_FB_FAIL		  = 0,		// 일반오류.
	EMPET_REQ_SLOT_EX_HOLD_FB_OK		  = 1,		// 아이템 교체 성공
	EMPET_REQ_SLOT_EX_HOLD_FB_NOMATCH	  = 2,		// 타입이 맞지 않는 아이템
	EMPET_REQ_SLOT_EX_HOLD_FB_INVALIDITEM = 3,		// 팻용 아이템이 아니다
};

enum EMPET_REQ_CHANGESKILL_FB
{
	EMPET_REQ_CHANGESKILL_FB_FAIL		= 0,		// 일반오류
	EMPET_REQ_CHANGESKILL_FB_OK			= 1,		// 성공
	EMPET_REQ_CHANGESKILL_FB_COOLTIME	= 2,		// 쿨타임 
};

enum EMPET_REQ_LEARNSKILL_FB
{
	EMPET_REQ_LEARNSKILL_FB_FAIL		= 0,		// 일반오류
	EMPET_REQ_LEARNSKILL_FB_OK			= 1,		// 성공
	EMPET_REQ_LEARNSKILL_FB_NOITEM		= 2,		// 아이템 없음
	EMPET_REQ_LEARNSKILL_FB_BADITEM		= 3,		// 사용 불가능 아이템
	EMPET_REQ_LEARNSKILL_FB_ALREADY		= 4,		// 이미 배운 스킬
	EMPET_REQ_LEARNSKILL_FB_INVALIDSTORAGE = 5,		// 유료하지 않은 창고
};

enum EMPET_REQ_REVIVE_FB
{
	EMPET_REQ_REVIVE_FB_FAIL			= 0,		// 일반오류
	EMPET_REQ_REVIVE_FB_OK				= 1,		// 성공
	EMPET_REQ_REVIVE_FB_DBFAIL			= 2,		// DB쿼리실패(부활실패)
};

/*dual pet skill, Juver, 2017/12/29 */
enum EMPET_REQ_ENABLE_DUAL_SKILL_FB
{
	EMPET_REQ_ENABLE_DUAL_SKILL_FB_OK			= 0,
	EMPET_REQ_ENABLE_DUAL_SKILL_FB_FAIL			= 1,
	EMPET_REQ_ENABLE_DUAL_SKILL_INVALID_ITEM	= 2,
	EMPET_REQ_ENABLE_DUAL_SKILL_INVALID_CARD	= 3,
	EMPET_REQ_ENABLE_DUAL_SKILL_ALREADY_ENABLED	= 4,
	EMPET_REQ_ENABLE_DUAL_SKILL_NOT_ACTIVE		= 5,
};

namespace GLMSG
{
	#pragma pack(1)

	struct SNETPET_CREATEPET_FROMDB_FB
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwPetID;
		WORD				wPosX;
		WORD				wPosY;

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)*2 };
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPET_CREATEPET_FROMDB_FB () :
			dwPetID(-1),
			wPosX(0),
			wPosY(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_CREATE_PET_FROMDB_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_GETPET_FROMDB_FB
	{
		NET_MSG_GENERIC		nmg;

		PGLPET				pPet;
		DWORD				dwPetID;
		bool				bCardInfo;
		bool				bTrade;
		bool				bLMTItemCheck;

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+sizeof(PGLPET)+sizeof(bool)*3 };
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPET_GETPET_FROMDB_FB () :
			pPet(NULL),
			dwPetID(0),
			bCardInfo(false),
			bTrade(false),
			bLMTItemCheck(false)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GET_PET_FROMDB_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

    struct SNETPET_GETPET_FROMDB_ERROR
	{
		NET_MSG_GENERIC		nmg;

		bool				bCardInfo;
		bool				bTrade;
		bool				bLMTItemCheck;
		WORD				wPosX;
		WORD				wPosY;

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(bool)*3+sizeof(WORD)+sizeof(WORD)};
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPET_GETPET_FROMDB_ERROR () :
			bCardInfo(false),
			bTrade(false),
			bLMTItemCheck(false)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GET_PET_FROMDB_ERROR;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	struct SNETPET_REQ_USEPETCARD
	{
		NET_MSG_GENERIC		nmg;
		WORD				wPosX;
		WORD				wPosY;

		SNETPET_REQ_USEPETCARD () :
			wPosX(0),
			wPosY(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_USECARD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_USEPETCARD_FB
	{
		NET_MSG_GENERIC		nmg;

		char			m_szName[PETNAMESIZE+1];
		PETTYPE			m_emTYPE;
		DWORD			m_dwGUID;
		SNATIVEID		m_sPetID;

		/*dual pet skill, Juver, 2017/12/27 */
		SNATIVEID		m_sActiveSkillID_A;
		SNATIVEID		m_sActiveSkillID_B;
		BOOL			m_bDualSkill;

		PETSKILL		m_Skills[MAX_PETSKILL];
		DWORD			m_dwOwner;
		WORD			m_wStyle;
		WORD			m_wColor;
		float			m_fWalkSpeed;
		float			m_fRunSpeed;
		int				m_nFull;
		SITEMCUSTOM		m_PutOnItems[PET_ACCETYPE_SIZE];
		SNATIVEID		m_sMapID;
		DWORD			m_dwCellID;
		WORD			m_wSkillNum;
		D3DXVECTOR3		m_vPos;
		D3DXVECTOR3		m_vDir;

		SPETSKINPACKDATA m_sPetSkinPackData;

		EMPET_USECARD_FB	emFB;

		DWORD			m_dwPetID;

		SNETPET_REQ_USEPETCARD_FB () :
			m_emTYPE(PET_TYPE_NONE),
			m_dwGUID(UINT_MAX),
			m_sPetID(NATIVEID_NULL()),

			/*dual pet skill, Juver, 2017/12/27 */
			m_sActiveSkillID_A(NATIVEID_NULL()),
			m_sActiveSkillID_B(NATIVEID_NULL()),
			m_bDualSkill(FALSE),

			m_dwOwner(0),
			m_wStyle(0),
			m_wColor(0),
			m_fWalkSpeed(0.0f),
			m_fRunSpeed(0.0f),
			m_nFull(0),
			m_wSkillNum(0),
			m_sMapID(NATIVEID_NULL()),
			m_dwCellID(0),
			m_vPos(0,0,0),
			m_vDir(0,0,0),
			emFB(EMPET_USECARD_FB_FAIL),
			m_dwPetID(0)
		{
			m_sPetSkinPackData.Init();
			memset ( m_szName, 0, PETNAMESIZE+1 );
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_USECARD_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_UNUSEPETCARD
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwGUID;
		DWORD				dwPetID;

		SNETPET_REQ_UNUSEPETCARD () :
			dwGUID(UINT_MAX),
			dwPetID(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_UNUSECARD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_UNUSEPETCARD_FB
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwGUID;
		bool				bMoveMap;

		SNETPET_REQ_UNUSEPETCARD_FB () :
			dwGUID(UINT_MAX),
		    bMoveMap(false)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_UNUSECARD_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_PETREVIVEINFO
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwCharID;

		SNETPET_REQ_PETREVIVEINFO () :
			dwCharID(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType  = NET_MSG_PET_REQ_PETREVIVEINFO;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_PETREVIVEINFO_FB
	{
		NET_MSG_GENERIC		nmg;

		PETREVIVEINFO		arryPetReviveInfo[MAXPETREVINFO];
		WORD				wPetReviveNum;

		SNETPET_REQ_PETREVIVEINFO_FB () :
			wPetReviveNum(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType  = NET_MSG_PET_REQ_PETREVIVEINFO_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_PETCARDINFO
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwPetID;
		bool				bTrade;

		SNETPET_REQ_PETCARDINFO () :
			dwPetID(0),
			bTrade(false)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_PETCARDINFO;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_PETCARDINFO_FB
	{
		NET_MSG_GENERIC		nmg;

		char				szName[PETNAMESIZE+1];
		PETTYPE				emTYPE;
		int					nFull;

		/*dual pet skill, Juver, 2017/12/27 */
		SNATIVEID			sActiveSkillID_A;
		SNATIVEID			sActiveSkillID_B;
		BOOL				bDualSkill;

		SITEMCUSTOM			PutOnItems[PET_ACCETYPE_SIZE];
		PETSKILL			Skills[MAX_PETSKILL];
		WORD				wSkillNum;
		DWORD				dwPetID;

		bool				bTrade;

		SNETPET_REQ_PETCARDINFO_FB () :
			emTYPE(PET_TYPE_NONE),
			nFull(0),

			/*dual pet skill, Juver, 2017/12/27 */
			sActiveSkillID_A(NATIVEID_NULL()),
			sActiveSkillID_B(NATIVEID_NULL()),
			bDualSkill(FALSE),

			wSkillNum(0),
			dwPetID(0),
			bTrade(false)
		{
			memset ( szName, 0, PETNAMESIZE+1 );
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_PETCARDINFO_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_DISAPPEAR
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwGUID;

		SNETPET_REQ_DISAPPEAR () :
			dwGUID(UINT_MAX)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_DISAPPEAR;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_CREATE_ANYPET
	{
		NET_MSG_GENERIC		nmg;

		SDROPPET			Data;

		SNETPET_CREATE_ANYPET ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_CREATE_ANYPET;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_DROP_PET
	{
		NET_MSG_GENERIC		nmg;

		SDROPPET			Data;

		SNETPET_DROP_PET ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_DROPPET;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_ATTACK
	{
		NET_MSG_GENERIC		nmg;

		STARGETID			sTarID;

		SNETPET_ATTACK ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_ATTACK;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_SAD
	{
		NET_MSG_GENERIC		nmg;

		SNETPET_SAD ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_SAD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_UPDATE_MOVESTATE
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwFlag;

		SNETPET_REQ_UPDATE_MOVESTATE () :
			dwFlag(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_UPDATE_MOVE_STATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_UPDATE_MOVESTATE_FB
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwFlag;

		SNETPET_REQ_UPDATE_MOVESTATE_FB () :
			dwFlag(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_UPDATE_MOVE_STATE_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_UPDATE_CLIENT_PETFULL
	{
		NET_MSG_GENERIC		nmg;

		int					nFull;

		SNETPET_UPDATE_CLIENT_PETFULL () :
			nFull(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_UPDATECLIENT_FULL;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*dual pet skill, Juver, 2017/12/27 */
	struct SNETPET_REQ_SKILLCHANGE_A
	{
		NET_MSG_GENERIC		nmg;

		SNATIVEID			sSkillID;
		BOOL				bFromUI; /* pet skill bug Juver 2017/06/27 */

		SNETPET_REQ_SKILLCHANGE_A ()
			: sSkillID(NATIVEID_NULL())
			, bFromUI(FALSE) /* pet skill bug Juver 2017/06/27 */
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_SKILLCHANGE_A;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*dual pet skill, Juver, 2017/12/27 */
	struct SNETPET_REQ_SKILLCHANGE_A_FB
	{
		NET_MSG_GENERIC		nmg;

		SNATIVEID					sSkillID;
		EMPET_REQ_CHANGESKILL_FB	emFB;

		SNETPET_REQ_SKILLCHANGE_A_FB () :
			emFB(EMPET_REQ_CHANGESKILL_FB_FAIL),
			sSkillID(NATIVEID_NULL())
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_SKILLCHANGE_A_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*dual pet skill, Juver, 2017/12/27 */
	struct SNETPET_ADD_SKILLFACT_A
	{
		NET_MSG_GENERIC		nmg;

		SNATIVEID			sNATIVEID;
		SKILL::EMTYPES		emTYPE;
		float				fMVAR;
		float				fAge;

		SNETPET_ADD_SKILLFACT_A () :
			sNATIVEID(NATIVEID_NULL()),
			emTYPE(SKILL::FOR_TYPE_SIZE),
			fMVAR(0.0f),
			fAge(0.0f)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_ADD_SKILLFACT_A;
		}
	};

	/*dual pet skill, Juver, 2017/12/27 */
	struct SNETPET_REMOVE_SKILLFACT_A
	{
		NET_MSG_GENERIC		nmg;

		SNETPET_REMOVE_SKILLFACT_A ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REMOVE_SKILLFACT_A;
		}
	};

	/*dual pet skill, Juver, 2017/12/27 */
	struct SNETPET_REQ_SKILLCHANGE_B
	{
		NET_MSG_GENERIC		nmg;

		SNATIVEID			sSkillID;
		BOOL				bFromUI; /* pet skill bug Juver 2017/06/27 */

		SNETPET_REQ_SKILLCHANGE_B ()
			: sSkillID(NATIVEID_NULL())
			, bFromUI(FALSE) /* pet skill bug Juver 2017/06/27 */
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_SKILLCHANGE_B;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*dual pet skill, Juver, 2017/12/27 */
	struct SNETPET_REQ_SKILLCHANGE_B_FB
	{
		NET_MSG_GENERIC		nmg;

		SNATIVEID					sSkillID;
		EMPET_REQ_CHANGESKILL_FB	emFB;

		SNETPET_REQ_SKILLCHANGE_B_FB () :
			emFB(EMPET_REQ_CHANGESKILL_FB_FAIL),
			sSkillID(NATIVEID_NULL())
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_SKILLCHANGE_B_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*dual pet skill, Juver, 2017/12/27 */
	struct SNETPET_ADD_SKILLFACT_B
	{
		NET_MSG_GENERIC		nmg;

		SNATIVEID			sNATIVEID;
		SKILL::EMTYPES		emTYPE;
		float				fMVAR;
		float				fAge;

		SNETPET_ADD_SKILLFACT_B () :
			sNATIVEID(NATIVEID_NULL()),
			emTYPE(SKILL::FOR_TYPE_SIZE),
			fMVAR(0.0f),
			fAge(0.0f)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_ADD_SKILLFACT_B;
		}
	};

	/*dual pet skill, Juver, 2017/12/27 */
	struct SNETPET_REMOVE_SKILLFACT_B
	{
		NET_MSG_GENERIC		nmg;

		SNETPET_REMOVE_SKILLFACT_B ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REMOVE_SKILLFACT_B;
		}
	};


	struct SNETPET_REQ_GOTO
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwGUID;
		D3DXVECTOR3			vCurPos;
		D3DXVECTOR3			vTarPos;
		DWORD				dwFlag;

		SNETPET_REQ_GOTO () :
			dwGUID(UINT_MAX),
			vCurPos(0,0,0),
			vTarPos(0,0,0),
			dwFlag(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_GOTO;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_GOTO_FB
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwGUID;
		D3DXVECTOR3			vPos;

		SNETPET_REQ_GOTO_FB () :
			dwGUID(UINT_MAX),
			vPos(0,0,0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_GOTO_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_STOP
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwGUID;
		DWORD				dwFlag;
		D3DXVECTOR3			vPos;
		bool				bStopAttack;

		SNETPET_REQ_STOP () :
			dwGUID(UINT_MAX),
			dwFlag(0),
			vPos(0,0,0),
			bStopAttack(false)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_STOP;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*dual pet skill, Juver, 2017/12/27 */
	struct SNETPET_REQ_GETRIGHTOFITEM_A
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwGUID;
		EMPETGETITEM_SKILL	emSkill;

		SNETPET_REQ_GETRIGHTOFITEM_A () :
			dwGUID(UINT_MAX),
			emSkill(EMPETSKILL_NONE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_GETRIGHTOFITEM_A;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*dual pet skill, Juver, 2017/12/27 */
	struct SNETPET_REQ_GETRIGHTOFITEM_A_FB
	{
		NET_MSG_GENERIC				nmg;

		WORD						wSum;
		STARGETID					DroppedItems[MAXRIGHTOFITEM];
		EMPET_REQ_GETRIGHTOFITEM_FB emFB;

		SNETPET_REQ_GETRIGHTOFITEM_A_FB () :
			wSum(0),
			emFB(EMPET_REQ_GETRIGHTOFITEM_FB_OK)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_GETRIGHTOFITEM_A_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*dual pet skill, Juver, 2017/12/27 */
	struct SNETPET_REQ_GETRIGHTOFITEM_B
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwGUID;
		EMPETGETITEM_SKILL	emSkill;

		SNETPET_REQ_GETRIGHTOFITEM_B () :
			dwGUID(UINT_MAX),
			emSkill(EMPETSKILL_NONE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_GETRIGHTOFITEM_B;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*dual pet skill, Juver, 2017/12/27 */
	struct SNETPET_REQ_GETRIGHTOFITEM_B_FB
	{
		NET_MSG_GENERIC				nmg;

		WORD						wSum;
		STARGETID					DroppedItems[MAXRIGHTOFITEM];
		EMPET_REQ_GETRIGHTOFITEM_FB emFB;

		SNETPET_REQ_GETRIGHTOFITEM_B_FB () :
		wSum(0),
			emFB(EMPET_REQ_GETRIGHTOFITEM_FB_OK)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_GETRIGHTOFITEM_B_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	struct SNETPET_MSG_NOTENOUGHINVEN
	{
		NET_MSG_GENERIC				nmg;

		SNETPET_MSG_NOTENOUGHINVEN ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_NOTENOUGHINVEN;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_GIVEFOOD
	{
		NET_MSG_GENERIC				nmg;

		WORD						wPosX;
		WORD						wPosY;
		PETTYPE						emPetTYPE;

		SNETPET_REQ_GIVEFOOD () :
			wPosX(0),
			wPosY(0),
			emPetTYPE(PET_TYPE_NONE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_GIVEFOOD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_GIVEFOOD_FB
	{
		NET_MSG_GENERIC				nmg;

		DWORD						dwPetID;
		SNATIVEID					sNativeID;
		int							nFull;

		EMPET_REQ_GIVEFOOD_FB		emFB;

		SNETPET_REQ_GIVEFOOD_FB () :
			dwPetID(0),
			sNativeID(NATIVEID_NULL()),
			nFull(0),
			emFB(EMPET_REQ_GIVEFOOD_FB_FAIL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_GIVEFOOD_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_GETFULL_FROMDB_FB
	{
		NET_MSG_GENERIC				nmg;

		DWORD						dwPetID;
		int							nFull;
		WORD						wPosX;
		WORD						wPosY;
		WORD						wCureVolume;
		BOOL						bRatio;
		SNATIVEID					sNativeID;
		EMPET_REQ_GIVEFOOD_FB		emFB;
		PETTYPE						emType;


		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(DWORD)+sizeof(int)+sizeof(WORD)*3+sizeof(BOOL)+sizeof(SNATIVEID)+sizeof(EMPET_REQ_GIVEFOOD_FB)+sizeof(PETTYPE) };
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPET_REQ_GETFULL_FROMDB_FB () :
			dwPetID(0),
			nFull(0),
			wPosX(0),
			wPosY(0),
			wCureVolume(0),
			bRatio(false),
			sNativeID(NATIVEID_NULL()),
			emFB(EMPET_REQ_GETFULL_FROMDB_FAIL),
			emType(PET_TYPE_NONE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_GETFULL_FROMDB_FB;
		}
	};

	struct SNETPET_REQ_RENAME
	{
		NET_MSG_GENERIC			nmg;

		char					szName[PETNAMESIZE+1];
		WORD					wPosX;
		WORD					wPosY;

		SNETPET_REQ_RENAME () :
			wPosX(0),
			wPosY(0)
		{
			nmg.dwSize = sizeof(*this);
			memset ( szName, 0, sizeof(char)*(PETNAMESIZE+1) );
			nmg.nType  = NET_MSG_PET_REQ_RENAME;
		}
	};

	struct SNETPET_REQ_RENAME_FB
	{
		NET_MSG_GENERIC			nmg;

		EMPET_REQ_RENAME_FB		emFB;
		char					szName[PETNAMESIZE+1];

		SNETPET_REQ_RENAME_FB () :
			emFB(EMPET_REQ_RENAME_FB_FB_FAIL)
		{
			nmg.dwSize = sizeof(*this);
			memset ( szName, 0, sizeof(char)*(PETNAMESIZE+1) );
			nmg.nType = NET_MSG_PET_REQ_RENAME_FB;
		}
	};

	struct SNETPET_REQ_RENAME_FROMDB_FB
	{
		NET_MSG_GENERIC				nmg;

		EMPET_REQ_RENAME_FB			emFB;
		WORD						wPosX;
		WORD						wPosY;
		char						szName[PETNAMESIZE+1];

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC)+sizeof(EMPET_REQ_RENAME_FB)+sizeof(WORD)+sizeof(WORD)+sizeof(char)*(PETNAMESIZE+1) };
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPET_REQ_RENAME_FROMDB_FB () :
			emFB(EMPET_REQ_RENAME_FB_FB_FAIL),
			wPosX(0),
			wPosY(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_RENAME_FROMDB_FB;

			memset( szName, 0, sizeof(char)*(PETNAMESIZE+1) );
			memset( m_cBUFFER, 0, NET_DATA_BUFSIZE-EMMSG_SIZE );			
		}
	};

	struct SNETPET_REQ_CHANGE_COLOR
	{
		NET_MSG_GENERIC			nmg;

		WORD					wColor;
		WORD					wPosX;
		WORD					wPosY;

		SNETPET_REQ_CHANGE_COLOR () :
			wPosX(0),
			wPosY(0),
			wColor(-1)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType  = NET_MSG_PET_REQ_CHANGE_COLOR;
		}
	};

	struct SNETPET_REQ_CHANGE_COLOR_FB
	{
		NET_MSG_GENERIC				    nmg;

		WORD						    wColor;
		EMPET_REQ_STYLECOLOR_CHANGE_FB  emFB;


		SNETPET_REQ_CHANGE_COLOR_FB () :
			wColor(-1),
			emFB(EMPET_REQ_STYLECOLOR_CHANGE_FB_FAIL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType  = NET_MSG_PET_REQ_CHANGE_COLOR_FB;
		}
	};

	struct SNETPET_REQ_CHANGE_STYLE
	{
		NET_MSG_GENERIC			nmg;

		WORD					wStyle;
		WORD					wPosX;
		WORD					wPosY;

		SNETPET_REQ_CHANGE_STYLE () :
			wPosX(0),
			wPosY(0),
			wStyle(-1)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType  = NET_MSG_PET_REQ_CHANGE_STYLE;
		}
	};

	struct SNETPET_REQ_CHANGE_STYLE_FB
	{
		NET_MSG_GENERIC				    nmg;

		WORD						    wStyle;
		WORD							wColor;
		EMPET_REQ_STYLECOLOR_CHANGE_FB  emFB;


		SNETPET_REQ_CHANGE_STYLE_FB () :
			wStyle(-1),
			wColor(0),
			emFB(EMPET_REQ_STYLECOLOR_CHANGE_FB_FAIL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType  = NET_MSG_PET_REQ_CHANGE_STYLE_FB;
		}
	};

	struct SNETPET_REQ_SLOT_EX_HOLD
	{
		NET_MSG_GENERIC		nmg;

		EMSUIT				emSuit;

		SNETPET_REQ_SLOT_EX_HOLD () 
			: emSuit(SUIT_NSIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_SLOT_EX_HOLD;
		}
	};

	struct SNETPET_REQ_SLOT_EX_HOLD_FB
	{
		NET_MSG_GENERIC			  nmg;

		EMPET_REQ_SLOT_EX_HOLD_FB emFB;
		SITEMCUSTOM				  sItemCustom;
		EMSUIT					  emSuit;

		SNETPET_REQ_SLOT_EX_HOLD_FB () 
			: emSuit(SUIT_NSIZE)
			, emFB(EMPET_REQ_SLOT_EX_HOLD_FB_FAIL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_SLOT_EX_HOLD_FB;
		}
	};

	struct SNETPET_REQ_HOLD_TO_SLOT
	{
		NET_MSG_GENERIC		nmg;

		EMSUIT				emSuit;

		SNETPET_REQ_HOLD_TO_SLOT () 
			: emSuit(SUIT_NSIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_HOLD_TO_SLOT;
		}
	};

	struct SNETPET_REQ_HOLD_TO_SLOT_FB
	{
		NET_MSG_GENERIC		nmg;

		EMPET_REQ_SLOT_EX_HOLD_FB emFB;
		SITEMCUSTOM				  sItemCustom;
		EMSUIT					  emSuit;

		SNETPET_REQ_HOLD_TO_SLOT_FB () 
			: emSuit(SUIT_NSIZE)
			, emFB(EMPET_REQ_SLOT_EX_HOLD_FB_FAIL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_HOLD_TO_SLOT_FB;
		}
	};

	struct SNETPET_REQ_SLOT_TO_HOLD
	{
		NET_MSG_GENERIC		nmg;

		EMSUIT				emSuit;

		SNETPET_REQ_SLOT_TO_HOLD () 
			: emSuit(SUIT_NSIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_SLOT_TO_HOLD;
		}
	};

	struct SNETPET_REQ_SLOT_TO_HOLD_FB
	{
		NET_MSG_GENERIC		nmg;

		EMPET_REQ_SLOT_EX_HOLD_FB emFB;
		SITEMCUSTOM				  sItemCustom;
		EMSUIT					  emSuit;

		SNETPET_REQ_SLOT_TO_HOLD_FB () 
			: emSuit(SUIT_NSIZE)
			, emFB(EMPET_REQ_SLOT_EX_HOLD_FB_FAIL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_SLOT_TO_HOLD_FB;
		}
	};

	struct SNETPET_REQ_REMOVE_SLOTITEM
	{
		NET_MSG_GENERIC		nmg;

		EMSUIT				emSuit;

		SNETPET_REQ_REMOVE_SLOTITEM () 
			: emSuit(SUIT_NSIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REMOVE_SLOTITEM;
		}
	};

	struct SNETPET_REQ_REMOVE_SLOTITEM_FB
	{
		NET_MSG_GENERIC		nmg;

		EMSUIT				emSuit;

		SNETPET_REQ_REMOVE_SLOTITEM_FB () 
			: emSuit(SUIT_NSIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REMOVE_SLOTITEM_FB;
		}
	};

	struct SNETPET_REQ_LEARNSKILL
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;
		bool				bStorage; // 창고여부
		DWORD				dwChannel;

		SNETPET_REQ_LEARNSKILL ()
			: wPosX(0)
			, wPosY(0)
			, bStorage(false)
			, dwChannel(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_LEARNSKILL;
		}
	};

	struct SNETPET_REQ_LEARNSKILL_FB
	{
		NET_MSG_GENERIC		nmg;

		EMPET_REQ_LEARNSKILL_FB		emFB;
		WORD						wPosX;
		WORD						wPosY;

		SNETPET_REQ_LEARNSKILL_FB ()
			: emFB(EMPET_REQ_LEARNSKILL_FB_FAIL)
			, wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_LEARNSKILL_FB;
		}
	};

	struct SNETPET_REQ_FUNNY
	{
		NET_MSG_GENERIC		nmg;

		WORD				wFunny;

		SNETPET_REQ_FUNNY ()
			: wFunny(1)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_FUNNY;
		}
	};

	struct SNETPET_REQ_REVIVE
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwPetID;
		WORD				wPosX;
		WORD				wPosY;

		SNETPET_REQ_REVIVE ()
			: dwPetID(0)
			, wPosX(0)
			, wPosY(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_REVIVE;
		}
	};

	struct SNETPET_REQ_REVIVE_FB
	{
		NET_MSG_GENERIC		nmg;

		DWORD				dwPetID;
		EMPET_REQ_REVIVE_FB emFB;

		SNETPET_REQ_REVIVE_FB ()
			: dwPetID(0),
			  emFB(EMPET_REQ_REVIVE_FB_FAIL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_REVIVE_FB;
		}
	};

	struct SNETPET_REQ_REVIVE_FROMDB_FB
	{
		NET_MSG_GENERIC		nmg;

		WORD				wPosX;
		WORD				wPosY;
		DWORD				dwPetID;

		EMPET_REQ_REVIVE_FB emFB;

		enum { EMMSG_SIZE = sizeof(NET_MSG_GENERIC) + sizeof(DWORD)*2+ sizeof(EMPET_REQ_REVIVE_FB) };
		char				m_cBUFFER[NET_DATA_BUFSIZE-EMMSG_SIZE];

		SNETPET_REQ_REVIVE_FROMDB_FB ()
			: wPosX(0)
			, wPosY(0)
			, dwPetID(0)
			, emFB(EMPET_REQ_REVIVE_FB_FAIL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType  = NET_MSG_PET_REQ_REVIVE_FROMDB_FB;
		}
	};

	struct SNETPET_ACCESSORY_DELETE
	{
		NET_MSG_GENERIC		nmg;

		PET_ACCESSORY_TYPE		accetype;

		SNETPET_ACCESSORY_DELETE ()
			: accetype(PET_ACCETYPE_SIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType  = NET_MSG_PET_ACCESSORY_DELETE;
		}
	};

	struct SNETPET_SKINPACKOPEN
	{
		NET_MSG_GENERIC			nmg;

		WORD					wPosX;
		WORD					wPosY;
		SNATIVEID				sItemId;

		SNETPET_SKINPACKOPEN () 
			: wPosX(0)
			, wPosY(0)
			, sItemId ( NATIVEID_NULL () )
		{
			nmg.dwSize = sizeof(SNETPET_SKINPACKOPEN);
			nmg.nType = NET_MSG_PET_PETSKINPACKOPEN;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_SKINPACKOPEN_FB
	{
		NET_MSG_GENERIC			nmg;

		EMPET_PETSKINPACKOPEN_FB	emFB;
		SPETSKINPACKDATA			sPetSkinPackData;

		SNETPET_SKINPACKOPEN_FB () 
			: emFB(EMPET_PETSKINPACKOPEN_FB_FAIL)
		{
			sPetSkinPackData.Init();

			nmg.dwSize = sizeof(SNETPET_SKINPACKOPEN_FB);
			nmg.nType = NET_MSG_PET_PETSKINPACKOPEN_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_SKINPACKOPEN_BRD : public SNETPET_BROAD
	{
		SPETSKINPACKDATA		sPetSkinPackData;

		SNETPET_SKINPACKOPEN_BRD () 
		{
			sPetSkinPackData.Init();

			nmg.dwSize = sizeof(SNETPET_SKINPACKOPEN_BRD);
			nmg.nType = NET_MSG_PET_PETSKINPACKOPEN_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


//-----------------------------------------------------------------------------------------------------
	struct SNETPET_GOTO_BRD : public SNETPET_BROAD
	{
		D3DXVECTOR3			vCurPos;
		D3DXVECTOR3			vTarPos;
		DWORD				dwFlag;

		SNETPET_GOTO_BRD () :
			vCurPos(0,0,0),
			vTarPos(0,0,0),
			dwFlag(false)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_GOTO_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_STOP_BRD : public SNETPET_BROAD
	{
		D3DXVECTOR3			vPos;
		DWORD				dwFlag;
		bool				bStopAttack;

		SNETPET_STOP_BRD () :
			dwFlag(0),
			vPos(0,0,0),
			bStopAttack(false)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_STOP_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*dual pet skill, Juver, 2017/12/27 */
	struct SNETPET_REQ_SKILLCHANGE_A_BRD : public SNETPET_BROAD
	{
		SNATIVEID sSkillID;
		DWORD	  dwTarID;

		SNETPET_REQ_SKILLCHANGE_A_BRD () :
			sSkillID ( NATIVEID_NULL () ),
			dwTarID(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_SKILLCHANGE_A_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*dual pet skill, Juver, 2017/12/27 */
	struct SNETPET_REQ_SKILLCHANGE_B_BRD : public SNETPET_BROAD
	{
		SNATIVEID sSkillID;
		DWORD	  dwTarID;

		SNETPET_REQ_SKILLCHANGE_B_BRD () :
			sSkillID ( NATIVEID_NULL () ),
			dwTarID(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_SKILLCHANGE_B_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_UPDATE_MOVESTATE_BRD : public SNETPET_BROAD
	{
		DWORD				dwFlag;

		SNETPET_REQ_UPDATE_MOVESTATE_BRD () :
			dwFlag(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_UPDATE_MOVE_STATE_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_ATTACK_BRD : public SNETPET_BROAD
	{
		STARGETID				sTarID;

		SNETPET_ATTACK_BRD ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_ATTACK_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_SAD_BRD : public SNETPET_BROAD
	{
		DWORD					dwOwner;

		SNETPET_SAD_BRD () :
			dwOwner(UINT_MAX)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_SAD_BRD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPET_REQ_RENAME_BRD : public SNETPET_BROAD
	{
		char 		szName[PETNAMESIZE+1];

		SNETPET_REQ_RENAME_BRD ()
		{
			memset ( szName, 0, sizeof(char)*PETNAMESIZE+1 );
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_RENAME_BRD;
		}
	};

	struct SNETPET_REQ_CHANGE_COLOR_BRD  : public SNETPET_BROAD
	{
		WORD				    wColor;

		SNETPET_REQ_CHANGE_COLOR_BRD () :
			wColor(-1)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType  = NET_MSG_PET_REQ_CHANGE_COLOR_BRD;
		}
	};

	struct SNETPET_REQ_CHANGE_STYLE_BRD  : public SNETPET_BROAD
	{
		WORD				    wStyle;
		WORD					wColor;

		SNETPET_REQ_CHANGE_STYLE_BRD () :
			wStyle(-1),
			wColor(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType  = NET_MSG_PET_REQ_CHANGE_STYLE_BRD;
		}
	};

	struct SNETPET_REQ_SLOT_EX_HOLD_BRD : public SNETPET_BROAD
	{
		SITEMCUSTOM			sItemCustom;
		EMSUIT				emSuit;

		SNETPET_REQ_SLOT_EX_HOLD_BRD () 
			: emSuit(SUIT_NSIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_SLOT_EX_HOLD_BRD;
		}
	};

	struct SNETPET_REQ_LEARNSKILL_BRD : public SNETPET_BROAD
	{
		SNETPET_REQ_LEARNSKILL_BRD ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_LEARNSKILL_BRD;
		}
	};

	struct SNETPET_REQ_FUNNY_BRD : public SNETPET_BROAD
	{
		WORD				wFunny;

		SNETPET_REQ_FUNNY_BRD ()
			: wFunny(1)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_FUNNY_BRD;
		}
	};

	struct SNETPET_REQ_REMOVE_SLOTITEM_BRD : public SNETPET_BROAD
	{
		EMSUIT			emSuit;

		SNETPET_REQ_REMOVE_SLOTITEM_BRD ()
			: emSuit(SUIT_NSIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REMOVE_SLOTITEM_BRD;
		}
	};
    
	struct SNETPET_ACCESSORY_DELETE_BRD : public SNETPET_BROAD
	{
		PET_ACCESSORY_TYPE		accetype;

		SNETPET_ACCESSORY_DELETE_BRD ()
			: accetype(PET_ACCETYPE_SIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType  = NET_MSG_PET_ACCESSORY_DELETE_BRD;
		}
	};


	/*dual pet skill, Juver, 2017/12/29 */
	struct SNETPET_REQ_ENABLE_DUAL_SKILL
	{
		NET_MSG_GENERIC				nmg;

		WORD						wPosX;
		WORD						wPosY;
		PETTYPE						emPetTYPE;

		SNETPET_REQ_ENABLE_DUAL_SKILL () :
			wPosX(0),
			wPosY(0),
			emPetTYPE(PET_TYPE_NONE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_ENABLE_DUAL_SKILL;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	/*dual pet skill, Juver, 2017/12/29 */
	struct SNETPET_REQ_ENABLE_DUAL_SKILL_FB
	{
		NET_MSG_GENERIC				nmg;

		DWORD						dwPetID;
		SNATIVEID					sNativeID;
		BOOL						bDualSkill;

		EMPET_REQ_ENABLE_DUAL_SKILL_FB		emFB;

		SNETPET_REQ_ENABLE_DUAL_SKILL_FB () :
			dwPetID(0),
			sNativeID(NATIVEID_NULL()),
			bDualSkill(FALSE),
			emFB(EMPET_REQ_ENABLE_DUAL_SKILL_FB_FAIL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_PET_REQ_ENABLE_DUAL_SKILL_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	// Revert to default structure packing
	#pragma pack()
};