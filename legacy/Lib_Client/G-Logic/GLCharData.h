#ifndef GLCHARDATA_H_
#define GLCHARDATA_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <map>


/*dmk14 ingame web*/
#include "./GLItemShopData.h"
#include "./GLFactData.h"

#include "./GLInventory.h"
#include "./GLItem.h"
#include "./GLQuestPlay.h"
#include "./GLSkill.h"
#include "./GLSkillDefine.h"
#include "./GLVEHICLE.h"

#include "../../Lib_Engine/Common/ByteStream.h"

/*skill summon, Juver, 2017/10/09 */
#include "GLSummonData.h"

/*activity system, Juver, 2017/10/23 */
#include "GLActivityData.h"

/*Codex System, Jhoniex 2/14/2024 */
#include "GLCodexData.h"

/*pvp capture the flag, Juver, 2018/01/29 */
#include "GLPVPCaptureTheFlagDefine.h"

#define SCALERANGE_MIN	(0.88f) //(0.75f)
#define SCALERANGE_MAX	(1.12f) //(1.20f)

/*charinfoview , Juver, 2017/11/11 */
#define REQ_CHARINFO_DELAY 5.0f

//PANDORA
enum EMPANDORA_STATUS
{
    EMPANDORA_STATUS_OPEN = 0,
    EMPANDORA_STATUS_RARE = 1,
    EMPANDORA_STATUS_FREE = 2,

};

//PANDORA
struct GLPANDORA_BOX_STATUS
{
    BYTE uCount;
    float fReqDelay;
    EMPANDORA_STATUS emStatus;

    GLPANDORA_BOX_STATUS()
        : uCount ( 0 )
        , fReqDelay (0.0f)
        , emStatus(EMPANDORA_STATUS_OPEN)

    {
    }

    void RESET ()
    {
        uCount = 0;
        fReqDelay = 0.0f;
        emStatus = EMPANDORA_STATUS_OPEN;
    }

};

//PANDORA
struct GLPANDORA_BOX_SETTING
{

    LONGLONG  llGold;
    WORD      wEP;
    float      fPremChance;
    float      fReqDelay;

    GLPANDORA_BOX_SETTING()
        : llGold ( 0 )
        , wEP ( 0 )
        , fPremChance( 0.0f )
        , fReqDelay (0.0f)
    {
    }

};
//PANDORA
struct GLPANDORA_BOX
{
    enum { PANDORA_RESULT_SIZE = 10 };
    int          nIndex;
    SNATIVEID sItemID;
    LONGLONG  llGold;
    WORD      wEP;
    float      fRate;

    GLPANDORA_BOX()
        : nIndex ( -1 )
        , sItemID(NATIVEID_NULL())
        , llGold ( 0 )
        , wEP ( 0 )
        , fRate ( 0.0f )

    {
    }

    bool operator < (const GLPANDORA_BOX& rhs) const
    {
        if ( nIndex != -1 )
            return (nIndex < rhs.nIndex);
        else
        {
            return (sItemID.dwID < rhs.sItemID.dwID);
        }
    }

};


/*PartySearch - Added By Bubu*/
struct PARTYLINKDATA
{
	unsigned		dwIndex;
	DWORD			dwPartyID;
	DWORD			m_dwGaeaID;
	char			m_szName[CHAR_SZNAME];		//	�̸�. (����)
	EMCHARCLASS		m_emClass;
	bool			m_bPrivate;
	SNATIVEID		m_sMapID;
	WORD			m_wLevel;
	WORD			m_wSchool;
	WORD			m_wMembers;
	SPARTY_OPT		sOption;

	PARTYLINKDATA()
		: dwIndex(0)
		, dwPartyID(0)
		, m_dwGaeaID(GAEAID_NULL)
		, m_emClass(GLCC_BRAWLER_M)
		, m_sMapID(MAXLANDMID,MAXLANDSID)
		, m_wLevel(0)
		, m_wSchool(0)
		, m_wMembers(0)
		, m_bPrivate(FALSE)
	{
		memset(m_szName, 0, sizeof(char) * CHAR_SZNAME);
	}

	PARTYLINKDATA( const PARTYLINKDATA &value)
	{
		operator=(value);
	}

	PARTYLINKDATA& operator=( const PARTYLINKDATA &value )
	{
		dwIndex = value.dwIndex;
		dwPartyID = value.dwPartyID;
		m_dwGaeaID = value.m_dwGaeaID;
		m_emClass = value.m_emClass;
		m_sMapID = value.m_sMapID;
		m_wLevel = value.m_wLevel;
		m_wSchool = value.m_wSchool;
		m_bPrivate = value.m_bPrivate;
		sOption = value.sOption;
		m_wMembers = value.m_wMembers;

		StringCchCopy( m_szName, CHAR_SZNAME, value.m_szName );

		return *this;
	}

	bool operator < ( const PARTYLINKDATA& sPartyLink )
	{		
		if( dwPartyID >= sPartyLink.dwPartyID ) return true;
		return  false;
	}

	bool operator == ( const PARTYLINKDATA& sPartyLink ) 
	{
		if( dwPartyID == sPartyLink.dwPartyID ) return true;
		return false;
	}
};
///////////////////////////////////////////////////////////
enum EMCHARDATA
{
	EMSKILLQUICK_VERSION	= 0x0102,
	EMSKILLQUICK_SIZE_101	= 60,
	EMSKILLQUICK_SIZE		= 60,

	EMACTIONQUICK_SIZE		= 6,

	SKILLREALFACT_SIZE		= 14,
	SKILLFACT_SIZE			= 14,

	EMMAX_CLUB_NUM			= 100,

	EMSTORAGE_CHANNEL				= 5,
	
	EMSTORAGE_CHANNEL_DEF			= 0,
	EMSTORAGE_CHANNEL_DEF_SIZE		= 1,

	EMSTORAGE_CHANNEL_SPAN			= 1,
	EMSTORAGE_CHANNEL_SPAN_SIZE		= 3,

	EMSTORAGE_CHANNEL_PREMIUM		= 4,
	EMSTORAGE_CHANNEL_PREMIUM_SIZE	= 1,

	 /*itemfood system, Juver, 2017/05/24 */
	FITEMFACT_SIZE			= 4,
	FITEMFACT_DEFAULT_SIZE	= 2,

	/*system buffs, Juver, 2017/09/04 */
	SYSTEM_BUFF_SIZE			= 5,
	SYSTEM_BUFF_SETTING_SIZE	=	15,
};

enum EMGM_EVENT_TYPE
{
	EMGM_EVENT_NONE		= 0x00,	//	����.
	EMGM_EVENT_SPEED	= 0x01,	//	�̵��ӵ�.
	EMGM_EVENT_ASPEED	= 0x02,	//	���ݼӵ�.
	EMGM_EVENT_ATTACK	= 0x04,	//	���ݷ�.
};

struct SCHARSKILL
{
	static DWORD VERSION;
	static DWORD SIZE;

	SNATIVEID	sNativeID;
	WORD		wLevel;

	SCHARSKILL () :
		sNativeID(SNATIVEID::ID_NULL,SNATIVEID::ID_NULL),
		wLevel(0)
	{
	}

	SCHARSKILL ( const SNATIVEID &_sNID, const WORD _wLvl ) :
		sNativeID(_sNID),
		wLevel(_wLvl)
	{
	}
};

struct DAMAGE_SPEC
{
	enum 
	{
		FLAG_IGNORED_DAMAGE		= 0x00000001,
		FLAG_ILLUSION					= 0X00000002,
	}; //enum

	float		m_fPsyDamageReduce;				// ���� ������ ������
    float		m_fPsyDamageReduceEFFRate;			// ���� ������ ������ ����. by luxes.
	float		m_fMagicDamageReduce;			// ���� ������ ������
    float		m_fMagicDamageReduceEffRate;				// ���� ������ ������ ����. by luxes.    
	float		m_fPsyDamageReflection;			// ���� ������ �ݻ���
	float		m_fPsyDamageReflectionEffRate;			// ���� ������ �ݻ��� ����. by luxes.
	float		m_fPsyDamageReflectionRateEffRate;	// ���� ������ �ݻ�Ȯ�� ����. by luxes.
	float		m_fPsyDamageReflectionRate;		// ���� ������ �ݻ�Ȯ��
	float		m_fMagicDamageReflection;		// ���� ������ �ݻ���
    float		m_fMagicDamageReflectionEffRate;			// ���� ������ �ݻ��� ����. by luxes.
	float		m_fMagicDamageReflectionRate;	// ���� ������ �ݻ�Ȯ��
    float		m_fMagicDamageReflectionRateEffRate;	// ���� ������ �ݻ�Ȯ�� ����. by luxes.
    float		m_fDamageCurse;				// ���� ������ �ݻ���
    float		m_fDamageCurseEffRate;	// ���� ������ �ݻ��� ����. by luxes.
	float		m_fIllusionJumpBound;
	float		m_fIllusionLastTime;
	WORD		m_wIllusionRemain;
	STARGETID	m_sCurseTargetID;		// ���ָ� �� Ÿ�� ( ��ų ����� ) 
    DWORD		m_dwFlags;					// ������ ���� Ȱ��ȭ ����
	DWORD		m_emImmuneApplyType;	// �鿪 ���� Ÿ��;
	DWORD		m_emImmuneActionType;	// �鿪 �ൿ Ÿ��;	
	
    DAMAGE_SPEC()				 { AllReset(); }	

    void RESET()
    {
		m_fPsyDamageReduce					= 0.0f;
        m_fPsyDamageReduceEFFRate			= 1.0f;
        m_fMagicDamageReduce				= 0.0f;
        m_fMagicDamageReduceEffRate			= 1.0f;
        m_fPsyDamageReflection				= 0.0f;
        m_fPsyDamageReflectionEffRate		= 1.0f;
        m_fPsyDamageReflectionRate			= 0.0f;
        m_fPsyDamageReflectionRateEffRate	= 1.0f;
        m_fMagicDamageReflection			= 0.0f;
        m_fMagicDamageReflectionEffRate		= 1.0f;
        m_fMagicDamageReflectionRate		= 0.0f;
        m_fMagicDamageReflectionRateEffRate	= 1.0f;
        m_fDamageCurse						= 0.0f;
        m_fDamageCurseEffRate				= 1.0f;
        m_dwFlags							= 0;
		m_emImmuneApplyType					= 0;
		m_emImmuneActionType				= 0;
    }

	/**
	 * reset all of data include saved data.
	 */
	void AllReset()
	{
		RESET();
		m_wIllusionRemain		= 0;
		m_fIllusionJumpBound	= 0.0f;
		m_fIllusionLastTime		= 0.0f;
	} //AllReset

	DAMAGE_SPEC& operator = ( const DAMAGE_SPEC& rh )
	{
		m_fPsyDamageReduce					= rh.m_fPsyDamageReduce;
        m_fPsyDamageReduceEFFRate			= rh.m_fPsyDamageReduceEFFRate;
        m_fMagicDamageReduce				= rh.m_fMagicDamageReduce;
        m_fMagicDamageReduceEffRate			= rh.m_fMagicDamageReduceEffRate;
        m_fPsyDamageReflection				= rh.m_fPsyDamageReflection;
        m_fPsyDamageReflectionEffRate		= rh.m_fPsyDamageReflectionEffRate;
        m_fPsyDamageReflectionRate			= rh.m_fPsyDamageReflectionRate;
        m_fPsyDamageReflectionRateEffRate	= rh.m_fPsyDamageReflectionRateEffRate;
        m_fMagicDamageReflection			= rh.m_fMagicDamageReflection;
        m_fMagicDamageReflectionEffRate		= rh.m_fMagicDamageReflectionEffRate;
        m_fMagicDamageReflectionRate		= rh.m_fMagicDamageReflectionRate;
        m_fMagicDamageReflectionRateEffRate	= rh.m_fMagicDamageReflectionRateEffRate;
        m_fDamageCurse						= rh.m_fDamageCurse;
        m_fDamageCurseEffRate				= rh.m_fDamageCurseEffRate;
        m_dwFlags							= rh.m_dwFlags;							
		m_emImmuneApplyType					= rh.m_emImmuneApplyType;
		m_emImmuneActionType				= rh.m_emImmuneActionType;

		return *this;
	} 
};

struct SSTIGMA
{	
	struct SSTIGMA_PACK
	{		
		STARGETID sUserID;		// ���� ȿ�� ����� ID
		STARGETID sTakerID;	// ���� ȿ�� �ǰ��� ID
		SNATIVEID sSkillID;		
		DWORD dwFLAG;
		float fRate;
		DWORD dwAsignDistance;		
		bool bUse;
		SSTIGMA_PACK(void) : bUse(false){}
	};	

	DWORD dwStigmaIndex;

	SSTIGMA_PACK sSTIGMA_PACK[SKILLFACT_SIZE];
	SSTIGMA(void) : dwStigmaIndex(0) 
	{
		for ( unsigned int _i = SKILLFACT_SIZE; _i; --_i )
			this->sSTIGMA_PACK[_i - 1].bUse = false;
	}
	void RESET(void)
	{
		*this = SSTIGMA();
	}
	const DWORD PushStigmaPack(const STARGETID& sUserID, const STARGETID& sTakerID, const SNATIVEID sSkillID, const SKILL::SSPEC& sSPEC);	
	const DWORD IsExistStigmaPack(const STARGETID& sUserID, const STARGETID& sTakerID, const SNATIVEID sSkillID);	
	const DWORD IsExistStigmaPack(const STARGETID& sUserID, const SNATIVEID sSkillID);	
	const SSTIGMA_PACK* GetStigmaPack(const DWORD dwIndex);
	const SSTIGMA_PACK* GetStigmaPack(void);
	void DeleteStigmaPack(const DWORD dwIndex);
};

struct SINCREASEEFF // ��ȭ ȿ��;
{
	DWORD dwApplyFlag; // dwFLAG;
	int nIncreaseCrushingBlowRate[SKILL::EMAPPLY_NSIZE]; // VAR1;
	int nIncreaseDamageRate[SKILL::EMAPPLY_NSIZE]; // VAR2;

	SINCREASEEFF() : dwApplyFlag(SKILL::EMAPPLY_DWORD_NULL)
	{
		for ( unsigned int _i = SKILL::EMAPPLY_NSIZE; _i; --_i )
		{
			this->nIncreaseCrushingBlowRate[_i - 1] = 0;
			this->nIncreaseDamageRate[_i - 1] = 0;
		}
	}

	const int GetIncreaseCrushingBlowRate(const DWORD dwApplyType)
	{
		switch ( dwApplyType )
		{
		case SKILL::EMAPPLY_DWORD_MELEE:
			return nIncreaseCrushingBlowRate[SKILL::EMAPPLY_PHY_SHORT];
			break;
		case SKILL::EMAPPLY_DWORD_RANGE:
			return nIncreaseCrushingBlowRate[SKILL::EMAPPLY_PHY_LONG];
			break;
		case SKILL::EMAPPLY_DWORD_MAGIC:
			return nIncreaseCrushingBlowRate[SKILL::EMAPPLY_MAGIC];
			break;
		}
		return 0;
	}
	const int GetIncreaseDamageRate(const DWORD dwApplyType)
	{
		switch ( dwApplyType )
		{
		case SKILL::EMAPPLY_DWORD_MELEE:
			return nIncreaseDamageRate[SKILL::EMAPPLY_PHY_SHORT];
			break;
		case SKILL::EMAPPLY_DWORD_RANGE:
			return nIncreaseDamageRate[SKILL::EMAPPLY_PHY_LONG];
			break;
		case SKILL::EMAPPLY_DWORD_MAGIC:
			return nIncreaseDamageRate[SKILL::EMAPPLY_MAGIC];
			break;
		}
		return 0;
	}
	void SET(const SKILL::SSPEC& sSPEC)
	{
		this->dwApplyFlag |=  sSPEC.dwFLAG;

		if ( sSPEC.dwFLAG & SKILL::EMAPPLY_DWORD_MELEE )
		{
			const int _nIncreseCrushingBlowRate = int(sSPEC.fVAR1 * 100.0f);
			const int _nIncreaseRate = int(sSPEC.fVAR2 * 100.0f);

			if ( _nIncreseCrushingBlowRate > this->nIncreaseCrushingBlowRate[SKILL::EMAPPLY_PHY_SHORT] )
				this->nIncreaseCrushingBlowRate[SKILL::EMAPPLY_PHY_SHORT] = _nIncreseCrushingBlowRate;
			if ( _nIncreaseRate > this->nIncreaseDamageRate[SKILL::EMAPPLY_PHY_SHORT] )
				this->nIncreaseDamageRate[SKILL::EMAPPLY_PHY_SHORT] = _nIncreaseRate;
		}

		if ( sSPEC.dwFLAG & SKILL::EMAPPLY_DWORD_RANGE )
		{
			const int _nIncreseCrushingBlowRate = int(sSPEC.fVAR1 * 100.0f);
			const int _nIncreaseRate = int(sSPEC.fVAR2 * 100.0f);

			if ( _nIncreseCrushingBlowRate > this->nIncreaseCrushingBlowRate[SKILL::EMAPPLY_PHY_LONG] )
				this->nIncreaseCrushingBlowRate[SKILL::EMAPPLY_PHY_LONG] = _nIncreseCrushingBlowRate;
			if ( _nIncreaseRate > this->nIncreaseDamageRate[SKILL::EMAPPLY_PHY_LONG] )
				this->nIncreaseDamageRate[SKILL::EMAPPLY_PHY_LONG] = _nIncreaseRate;
		}

		if ( sSPEC.dwFLAG & SKILL::EMAPPLY_DWORD_MAGIC )
		{
			const int _nIncreseCrushingBlowRate = int(sSPEC.fVAR1 * 100.0f);
			const int _nIncreaseRate = int(sSPEC.fVAR2 * 100.0f);

			if ( _nIncreseCrushingBlowRate > this->nIncreaseCrushingBlowRate[SKILL::EMAPPLY_MAGIC] )
				this->nIncreaseCrushingBlowRate[SKILL::EMAPPLY_MAGIC] = _nIncreseCrushingBlowRate;
			if ( _nIncreaseRate > this->nIncreaseDamageRate[SKILL::EMAPPLY_MAGIC] )
				this->nIncreaseDamageRate[SKILL::EMAPPLY_MAGIC] = _nIncreaseRate;
		}
	}
	void RESET(void)
	{
		*this = SINCREASEEFF();
	}
};

struct SLINKHP
{
	float fRef;
	float fTake;

	SLINKHP(void) :
	fRef(0.0f) ,fTake(1.0f)
	{
	}

	void SET(const float fVAR1, const float fVAR2)
	{
		if ( static_cast<int>(fVAR2) == TRUE )
		{
			if ( fVAR1 < this->fTake)
				this->fTake = fVAR1;
		}
	}
	void RESET(void)
	{
		*this = SLINKHP();
	}
};

struct DEFENSE_SKILL
{

	SNATIVEID	m_dwSkillID;					//	�ߵ��� ��ų MID/SID
	WORD		m_wLevel;						//	�ߵ��� ��ų ����
	float		m_fRate;						//	�ߵ��� ��ų MID/SID Ȯ��
	bool		m_bActive;						//	�ߵ��� ��ų ��������

	DEFENSE_SKILL() 
		: m_dwSkillID ( NATIVEID_NULL() )
		, m_wLevel ( 0 )
		, m_fRate ( 0.0f )
		, m_bActive(false)
	{
	}

	void RESET()
	{
		m_dwSkillID = NATIVEID_NULL();
		m_wLevel = 0;
		m_fRate = 0.0f;
	}
};

enum EMACTION_SLOT
{
	EMACT_SLOT_NONE	= 0,
	EMACT_SLOT_DRUG	= 1,
};

struct SACTION_SLOT
{
	enum { VERSION = 0x0100, };

	WORD		wACT;
	SNATIVEID	sNID;

	SACTION_SLOT () :
		wACT(EMACT_SLOT_NONE),
		sNID(false)
	{
	}

	bool VALID () const
	{
		return wACT!=EMACT_SLOT_NONE && sNID!=SNATIVEID(false);
	}

	void RESET ()
	{
		wACT = EMACT_SLOT_NONE;
		sNID = SNATIVEID(false);
	}
};

//systembuff
struct SRANDOM_SYSTEMBUFF_SET
{
	SNATIVEID		sSystemID;		
	float			fRATE;	

	SRANDOM_SYSTEMBUFF_SET()
		: sSystemID(NATIVEID_NULL())
		, fRATE( 0.0f )
	{
	}

	SRANDOM_SYSTEMBUFF_SET ( SNATIVEID sID, float fValue ) 
		:	sSystemID(sID)
        ,	fRATE(fValue)
	{
	}
};

//	Note : �ɸ��� 1�� �׸�.
//
struct SCHARDATA
{
protected:
	DWORD				m_dwUserID;
	WORD				m_wSlotNumber;

public:
	void SetUserID( DWORD dwUserID) { m_dwUserID = dwUserID; }
	DWORD GetUserID() { return m_dwUserID; }

	void SetSlotNumber( WORD wNumber ) { m_wSlotNumber = wNumber; }
	WORD GetSlotNumber() { return m_wSlotNumber; }

public:
	DWORD				m_dwUserLvl;				//	����� ���� Level.
	__time64_t			m_tPREMIUM;					//	�����̾� ����.
	bool				m_bPREMIUM;					//	���� �����̾� ����.
	__time64_t			m_tCHATBLOCK;				//	�Ӹ� ����.

	__time64_t			m_tSTORAGE[EMSTORAGE_CHANNEL_SPAN_SIZE];	//	â�� ����.
	bool				m_bSTORAGE[EMSTORAGE_CHANNEL_SPAN_SIZE];	//	â�� 2 ���.

	WORD				m_wINVENLINE;				//	�߰��� �κ��丮 �ټ�.

	DWORD				m_dwServerID;			
	DWORD				m_dwCharID;

	char				m_szName[CHAR_SZNAME];		//	�̸�. (����)

	EMTRIBE				m_emTribe;					//	����. (����)
	EMCHARCLASS			m_emClass;					//	����. (����)
	WORD				m_wSchool;					//	�п�.
	WORD				m_wSex;						//	����.
	WORD				m_wHair;					//	�Ӹ���Ÿ��.
	WORD				m_wHairColor;				//	�Ӹ�����
	WORD				m_wFace;					//	�󱼸��.

	int					m_nBright;					//	�Ӽ�.
	int					m_nLiving;					//	��Ȱ.
	
	WORD				m_wLevel;					//	����.
	LONGLONG			m_lnMoney;					//	���� �ݾ�.

	bool				m_bMoneyUpdate;				// ���� ������Ʈ �Ǿ����� �ƴ���
	bool				m_bStorageMoneyUpdate;		// â�� ���� ������Ʈ �Ǿ����� �ƴ���
	WORD				m_wTempLevel;				// �񱳿� ���� �ӽ� ����
	LONGLONG			m_lnTempMoney;				// �񱳿� ���� �ݾ� �ӽ� ����
	LONGLONG			m_lnTempStorageMoney;		// �񱳿� â��  �ݾ� �ӽ� ����

	LONGLONG			m_lVNGainSysMoney;			//  ��Ʈ�� Ž�� ���� ���� �ݾ�

	DWORD				m_dwGuild;					//	��� ��ȣ.
	char				m_szNick[CHAR_SZNAME];		//	����.
	__time64_t			m_tSECEDE;					//	Ż��ð�.

	SCHARSTATS			m_sStats;					//	Stats.
	WORD				m_wStatsPoint;				//	���� stats ����Ʈ.

	WORD				m_wAP;						//	�⺻ ���ݷ�.
	WORD				m_wDP;						//	�⺻ ����.

	WORD				m_wPA;						//	����ġ.
	WORD				m_wSA;						//	���ġ.
	WORD				m_wMA;						//	����ġ.

	GLLLDATA			m_sExperience;				//	����ġ. ( ����/�������뵵�ް� )
	LONGLONG			m_lnReExp;					//  ȸ���Ҽ� �ִ� ����ġ

	DWORD				m_dwSkillPoint;				//	Skill Point.

	LONGLONG			m_lVNGainSysExp;			// ��Ʈ�� Ž�� ���� ���� ����ġ

	GLDWDATA			m_sHP;						//	������. ( ����/�ִ뷮 )
	GLDWDATA			m_sMP;						//	���ŷ�. ( ����/�ִ뷮 )
	GLDWDATA			m_sSP;						//	�ٷ·�.	( ����/�ִ뷮 )
	GLDWDATA			m_sCombatPoint;				/*combatpoint logic logic, Juver, 2017/05/27 */
	bool				m_bSafeTime;

	WORD				m_wPK;						//	�� PK Ƚ��.

	bool				m_bEventBuster;				//	�̺�Ʈ ������ �ƴ���

	__time64_t			m_tLoginTime;				//  �α� �ð��̳� �̺�Ʈ ���۽ð�

	int					m_EventStartLv;				//  �̺�Ʈ �ּ� ����
	int					m_EventEndLv;				//  �̺�Ʈ �ִ� ����

	int					m_RemainEventTime;			//  �̺�Ʈ ������� ���� �ð�
	int					m_RemainBusterTime;			//  �ν��� �����ð�

	bool				m_bEventApply;				//  �̺�Ʈ ������ ����
	bool				m_bEventStart;				//  �̺�Ʈ �����ߴ��� �ƴ���, �̺�Ʈ �������

	int					m_EventStartTime;			//  �̺�Ʈ ���� ���� �ð�
	int					m_EventBusterTime;			//  �̺�Ʈ �ν��� Ÿ��

	//dmk14 btg info
	float				m_fBtgExpRate;
	float				m_fBtgItemRate;


	/// ��Ʈ�� Ž�� ���� �ý��� ����
	LONGLONG			m_VietnamGameTime;			//	����� ���� �ð�
	BYTE				m_dwVietnamGainType;		//  ��Ʈ�� Ž�� ���� �ý��� 
	DWORD				m_dwVietnamInvenCount;		// �ű�� �ִ� �������� ����

    TCHAR				m_szPhoneNumber[SMS_RECEIVER]; // ĳ���� �� ��ȣ

	float				m_fScaleRange;

	/*contribution point, Juver, 2017/08/23 */
	LONGLONG			m_llContributionPoint;	

	/*activity point, Juver, 2017/08/23 */
	DWORD				m_dwActivityPoint;

	/*activity system, Juver, 2017/11/04 */
	char				m_szBadge[CHAR_SZNAME];

	/*pk info, Juver, 2017/11/16 */
	DWORD				m_dwPKScore;
	DWORD				m_dwPKDeath;

	DWORD				m_dwCWKill;
	DWORD				m_dwCWDeath;

	/*equipment lock, Juver, 2018/01/13 */
	BOOL				m_bEnableEquipmentLock;
	BOOL				m_bEquipmentLockStatus;

	/*storage lock, Juver, 2018/01/13 */
	BOOL				m_bEnableStorageLock;
	BOOL				m_bStorageLockStatus;

	/*inventory lock, Juver, 2018/01/13 */
	BOOL				m_bEnableInventoryLock;
	BOOL				m_bInventoryLockStatus;

	/*pvp capture the flag, Juver, 2018/01/29 */
	WORD				m_wCaptureTheFlagTeam;
	bool				m_bCaptureTheFlagHoldFlag;

	//add directval
	int				m_nHP_Potion_Rate;
	int				m_nMP_Potion_Rate;
	int				m_nSP_Potion_Rate;

	/*dmk14 ingame web*/
	DWORD				m_dwPremiumPoints;
	DWORD				m_dwCombatPoints;

	EMACCTYPE			m_emUserType;

	/*Codex System, Jhoniex 2/14/2024 */
	DWORD			m_dwActivityDoneSize;
	DWORD			m_dwHPIncrease;
	DWORD			m_dwMPIncrease;
	DWORD			m_dwSPIncrease;
	DWORD			m_dwAttackIncrease;
	DWORD			m_dwDefenseIncrease;
	DWORD			m_dwShootingIncrease;
	DWORD			m_dwMeleeIncrease;
	DWORD			m_dwEnergyIncrease;
	DWORD			m_dwResistanceIncrease;
	DWORD			m_dwHitrateIncrease;
	DWORD			m_dwAvoidrateIncrease;
	
	SCHARDATA () 
		: m_dwUserID(0)
		, m_dwUserLvl(0)
		, m_tPREMIUM(0)
		, m_bPREMIUM(false)
		, m_tCHATBLOCK(0)
		, m_wINVENLINE(0)
		, m_dwServerID(0)
		, m_dwCharID(0)
		, m_emTribe(TRIBE_HUMAN)
		, m_emClass(GLCC_BRAWLER_M)
		, m_wSchool(0)
		, m_wSex(0)
		, m_wHair(0)
		, m_wHairColor(0)
		, m_wFace(0)
		, m_nBright(1)
		, m_nLiving(0)
		, m_wLevel(0)
		, m_lnMoney(0)
		, m_lVNGainSysMoney(0)
		, m_bMoneyUpdate(FALSE)
		, m_bStorageMoneyUpdate(FALSE)
		, m_wTempLevel(0)
		, m_lnTempMoney(0)
		, m_lnTempStorageMoney(0)
		, m_dwGuild(CLUB_NULL)
		, m_tSECEDE(0)
		, m_wStatsPoint(0)
		, m_wAP(0)
		, m_wDP(0)
		, m_wPA(0)
		, m_wSA(0)
		, m_wMA(0)
		, m_dwSkillPoint(0)
		, m_wPK(0)
		, m_EventStartTime(0)
		, m_EventBusterTime(0)
		, m_bEventBuster(FALSE)
		, m_bEventStart(FALSE)
		, m_tLoginTime(0)
		, m_EventStartLv(0)
		, m_EventEndLv(0)
		, m_bEventApply(FALSE)
		, m_RemainEventTime(0)
		, m_RemainBusterTime(0)
		, m_VietnamGameTime(0)
		, m_dwVietnamGainType(0)
		, m_lVNGainSysExp(0)
		, m_dwVietnamInvenCount(0)
		, m_lnReExp ( 0 )
		, m_fScaleRange( 1.0f )

		, m_nHP_Potion_Rate(0) //add directval
		, m_nMP_Potion_Rate(0)
		, m_nSP_Potion_Rate(0)
		, m_wSlotNumber(0)
		
		/*contribution point, Juver, 2017/08/23 */
		, m_llContributionPoint(0) 

		/*activity point, Juver, 2017/08/23 */
		, m_dwActivityPoint(0) 

		/*pk info, Juver, 2017/11/16 */
		, m_dwPKScore(0)
		, m_dwPKDeath(0)

		, m_dwCWKill(0)
		, m_dwCWDeath(0)

		/*equipment lock, Juver, 2018/01/13 */
		, m_bEnableEquipmentLock(FALSE)
		, m_bEquipmentLockStatus(FALSE)

		/*storage lock, Juver, 2018/01/13 */
		, m_bEnableStorageLock(FALSE)
		, m_bStorageLockStatus(FALSE)

		/*inventory lock, Juver, 2018/01/13 */
		, m_bEnableInventoryLock(FALSE)
		, m_bInventoryLockStatus(FALSE)

		/*pvp capture the flag, Juver, 2018/01/29 */
		, m_wCaptureTheFlagTeam(CAPTURE_THE_FLAG_TEAM_SIZE)
		, m_bCaptureTheFlagHoldFlag(false)

		/*dmk14 ingame web*/
		, m_dwPremiumPoints(0)
		, m_dwCombatPoints(0)
		//dmk14 btg info
		, m_fBtgExpRate(0.0f)
		, m_fBtgItemRate(0.0f)
		, m_bSafeTime(false)
		, m_emUserType(EM_TYPE_NORMAL)
		/*Codex System, Jhoniex 2/14/2024 */
		, m_dwActivityDoneSize(0)
		, m_dwHPIncrease(0)
		, m_dwMPIncrease(0)
		, m_dwSPIncrease(0)
		, m_dwAttackIncrease(0)
		, m_dwDefenseIncrease(0)
		, m_dwShootingIncrease(0)
		, m_dwMeleeIncrease(0)
		, m_dwEnergyIncrease(0)
		, m_dwResistanceIncrease(0)
		, m_dwHitrateIncrease(0)
		, m_dwAvoidrateIncrease(0)
	{
		memset(m_szName, 0, sizeof(char) * CHAR_SZNAME);
		memset(m_szNick, 0, sizeof(char) * CHAR_SZNAME);
		memset(m_szPhoneNumber, 0, sizeof(TCHAR) * SMS_RECEIVER);

		/*activity system, Juver, 2017/11/05 */
		memset(m_szBadge, 0, sizeof(char) * CHAR_SZNAME);

		for ( int i=0; i<EMSTORAGE_CHANNEL_SPAN_SIZE; ++i )	m_tSTORAGE[i] = 0;
		for ( int i=0; i<EMSTORAGE_CHANNEL_SPAN_SIZE; ++i )	m_bSTORAGE[i] = false;
	}

	SCHARDATA &GETCHARDATA ()			{ return *this; }
	EMCHARINDEX GETCHARINDEX () const	{ return CharClassToIndex ( m_emClass ); }

	bool IsKEEP_STORAGE ( DWORD dwCHANNEL );	//	�ش� ��ȣ�� ��Ŀ�� ������ �ñ�� �ִ��� �˻�.
	CTime GetStorageTime (  DWORD dwCHANNEL );
	WORD GetOnINVENLINE ();
	void CalcPREMIUM ();
    //! �κ��丮 ���� �ݾ�
    inline LONGLONG GetInvenMoney() const { return m_lnMoney; }
};
typedef SCHARDATA* PCHARDATA;

//PANDORA
typedef std::vector<GLPANDORA_BOX>            VECPANDORABOX;
typedef std::map<int,GLPANDORA_BOX>            PANDORABOXMAP;
typedef PANDORABOXMAP::iterator                PANDORABOXMAP_ITER;
typedef PANDORABOXMAP::const_iterator        PANDORABOXMAP_CITER;


typedef std::vector<SHOPPURCHASE>			VECSHOP;

typedef std::map<std::string,SHOPPURCHASE>	MAPSHOP;
typedef MAPSHOP::iterator					MAPSHOP_ITER;

typedef std::map<DWORD,std::string>			MAPSHOP_KEY;
typedef MAPSHOP_KEY::iterator				MAPSHOP_KEY_ITER;

/*PartySearch - Added By Bubu*/
typedef std::vector<PARTYLINKDATA>			VECPARTYSEARCH;
typedef VECPARTYSEARCH::iterator			VECPARTYSEARCH_ITER;

typedef std::map<DWORD,PARTYLINKDATA>		MAPPARTYSEARCH;
typedef MAPPARTYSEARCH::iterator			MAPPARTYSEARCH_ITER;
////////////////////////////////////
struct SCHARDATA2 : public SCHARDATA
{
	typedef std::map<DWORD,SCHARSKILL>		SKILL_MAP;
	typedef SKILL_MAP::iterator						SKILL_MAP_ITER;
	typedef SKILL_MAP::const_iterator				SKILL_MAP_CITER;

	char					m_szUID[USR_ID_LENGTH+1];

	SKILL_MAP				m_ExpSkills;						//	���� ��ų �Ӽ�.
	SITEMCUSTOM				m_PutOnItems[SLOT_TSIZE];			//	���� Item.

	WORD					m_wSKILLQUICK_ACT;					//	��ų �������� ��Ƽ��� ��ų.
	SNATIVEID				m_sSKILLQUICK[EMSKILLQUICK_SIZE];	//	��ų ������.
	SACTION_SLOT			m_sACTIONQUICK[EMACTIONQUICK_SIZE];	//	�׼� ������.

	GLInventory				m_cInventory;						//	�κ��丮.

	//PANDORA
	PANDORABOXMAP            m_mapPandoraResults;
	GLPANDORA_BOX_STATUS    m_sPandoraStatus;

	BOOL					m_bServerStorage;					//	â�� ��ȿ��. ( ������. ) ( ���� �����Ǵ� ĳ���ʹ� ������ ��ȿ�ϰ� ������ üũ��. - GLCHARLOGIC::INIT_DATA() )
	LONGLONG				m_lnStorageMoney;					//	â�� ���� �ݾ�.
	BOOL					m_bStorage[EMSTORAGE_CHANNEL];		//	â�� ��ȿ��. ( Ŭ���̾�Ʈ��. )
	GLInventory				m_cStorage[EMSTORAGE_CHANNEL];		//	â��.

	GLQuestPlay				m_cQuestPlay;						//	����Ʈ.

	//	���� ����.
	MAPSHOP					m_mapCharged;						//	������ ������ ���.
	
	//	Ŭ���̾�Ʈ ����.
	GLInventory				m_cInvenCharged;					//	������ ������ �κ�.
	MAPSHOP_KEY				m_mapChargedKey;					//	������ ������ �κ��� �ش� ��ġ purkey ã��.

	//	Note : �ʱ� ���۵� ��, �� ��ġ��.
	//
	SNATIVEID			m_sStartMapID;				//	�ʱ� ���� ��.
	DWORD				m_dwStartGate;				//	�ʱ� ���� ����Ʈ.
	D3DXVECTOR3			m_vStartPos;				//	�ʱ� ���� ��ġ.

	SNATIVEID			m_sSaveMapID;				//	���� ��.
	D3DXVECTOR3			m_vSavePos;					//	���� ��ġ.

	SNATIVEID			m_sLastCallMapID;			//	������ȯ ��.
	D3DXVECTOR3			m_vLastCallPos;				//	������ȯ ��ġ.

	INT					m_dwThaiCCafeClass;			// �±� ���̹� ī�� 
	INT					m_nMyCCafeClass;			// �����̽þ� PC�� �̺�Ʈ 
	SChinaTime			m_sChinaTime;				// �߱� �ð��� ����
	SEventTime			m_sEventTime;				// �̺�Ʈ �ð�

	SVietnamGainSystem  m_sVietnamSystem;			// ��Ʈ�� Ž�й��� �ý���
	GLInventory			m_cVietnamInventory;		// ��Ʈ�� ������ Ž�� �κ��丮
	bool				m_bVietnamLevelUp;			// ��Ʈ�� ����ġ ȹ�� ������ ���� �����ܰ��� ������ ������ų �� �ִ�.

	// ������ ���� ����	// ITEMREBUILD_MARK
	BOOL				m_bRebuildOpen;
	SINVEN_POS			m_sRebuildCardPos;
	SINVEN_POS			m_sRebuildItem;
	SINVEN_POS			m_sPreInventoryItem;
	LONGLONG			m_i64RebuildCost;
	LONGLONG			m_i64RebuildInput;

	/*dmk14 reform*/
	SNATIVEID			m_sRebuildItemSealed;
	SITEMCUSTOM			m_sItemPrevCopy;

	bool				m_bTracingUser;			// ���� �������� �������� �ƴ���

	typedef std::map<DWORD,ITEM_COOLTIME>	COOLTIME_MAP;
	typedef COOLTIME_MAP::iterator			COOLTIME_MAP_ITER;
	typedef COOLTIME_MAP::const_iterator	COOLTIME_MAP_CITER;

	COOLTIME_MAP		m_mapCoolTimeType;		//	������ Ÿ�Ժ� ��Ÿ��
    COOLTIME_MAP		m_mapCoolTimeID;		//	������ MID/SID�� ��Ÿ��

	/*itemfood system, Juver, 2017/05/24 */
	SFITEMFACT			m_sFITEMFACT[FITEMFACT_SIZE];	

	/*system buffs, Juver, 2017/09/04 */
	SSYSTEM_BUFF		m_sSYSTEM_BUFF[SYSTEM_BUFF_SIZE]; 

	/*activity system, Juver, 2017/10/23 */
	SACTIVITY_CHAR_DATA_MAP		m_mapActivityProg;
	SACTIVITY_CHAR_DATA_MAP		m_mapActivityDone;

	/*Codex System, Jhoniex 2/14/2024 */
	SCODEX_CHAR_DATA_MAP		m_mapCodexProg;
	SCODEX_CHAR_DATA_MAP		m_mapCodexDone;

	SCHARDATA2();
	void Assign ( SCHARDATA2 &CharData );

	EMCHARINDEX GETCHARINDEX () const { return CharClassToIndex ( m_emClass ); }

	BOOL SETEXPSKILLS_BYBUF ( CByteStream &ByteStream );
	BOOL GETEXPSKILLS_BYBUF ( CByteStream &ByteStream ) const;

	BOOL GETPUTONITEMS_BYBUF ( CByteStream &ByteStream ) const;
	
	BOOL SETSKILL_QUICKSLOT ( CByteStream &ByteStream );
	BOOL GETSKILL_QUICKSLOT ( CByteStream &ByteStream ) const;

	BOOL SETACTION_QUICKSLOT ( CByteStream &ByteStream );
	BOOL GETACTION_QUICKSLOT ( CByteStream &ByteStream ) const;


	BOOL SETINVENTORY_BYBUF ( CByteStream &ByteStream );
	BOOL GETINVENTORYE_BYBUF ( CByteStream &ByteStream ) const;

	BOOL SETSTORAGE_BYBUF ( CByteStream &ByteStream );
	BOOL GETSTORAGE_BYBUF ( CByteStream &ByteStream ) const;

	BOOL SETQUESTPLAY ( CByteStream &ByteStream );
	BOOL GETQUESTPLAY ( CByteStream &ByteStream ) const;

	BOOL SETSHOPPURCHASE ( VECSHOP &vecSHOP );

	// ��Ʈ�� Ž�й��� �ý��� �߰��� ���� ĳ���� �߰� �κ��丮 Ȯ��
	BOOL SETVTADDINVENTORY_BYBUF ( CByteStream &ByteStream );
	BOOL GETVTADDINVENTORYE_BYBUF ( CByteStream &ByteStream ) const;

	BOOL SETITEMCOOLTIME_BYBUF( CByteStream &ByteStream ); 
	BOOL GETITEMCOOLTIME_BYBUF( CByteStream &ByteStream ) const; 

	/*itemfood system, Juver, 2017/05/25 */
	BOOL SETITEMFOOD_BYBUF( CByteStream &ByteStream ); 
	BOOL GETITEMFOOD_BYBUF( CByteStream &ByteStream ) const; 

	/*activity system, Juver, 2017/10/23 */
	BOOL SETACTIVITY_BYBUF( CByteStream &ByteStream ); 
	BOOL GETACTIVITY_BYBUF( CByteStream &ByteStream ) const; 

	/*Codex System, Jhoniex 2/14/2024 */
	BOOL SETCODEX_BYBUF( CByteStream &ByteStream ); 
	BOOL GETCODEX_BYBUF( CByteStream &ByteStream ) const; 

	/* PANDORA DB BASED SYSTEM, ARWEKAJ09 9/16/2024 */
	BOOL SETPANDORA_BYBUF(CByteStream& ByteStream);
	BOOL GETPANDORA_BYBUF(CByteStream& ByteStream) const;

public:
	BOOL LOADFILE ( const char* szFileName );

public:
	//	Ŭ���̾�Ʈ ����.
	BOOL ADDSHOPPURCHASE ( const char* szPurKey, SNATIVEID nidITEM );
	BOOL DELSHOPPURCHASE ( const DWORD dwID );

public:	// ITEMREBUILD_MARK
	const SITEMCUSTOM& GET_REBUILD_ITEM();
	const SITEMCUSTOM& GET_PREHOLD_ITEM();	// ??? ???? ???

	/*dmk14 reform*/
	const SITEMCUSTOM& GET_SEALED_ITEM();
	
public:	// ITEMREBUILD_MARK
	VOID InitRebuildData();
	VOID OpenRebuild()							{ m_bRebuildOpen = TRUE; }
	VOID CloseRebuild()							{ m_bRebuildOpen = FALSE; }
	const BOOL ValidRebuildOpen()				{ return m_bRebuildOpen; }
	const LONGLONG GetRebuildCost()				{ return m_i64RebuildCost; }
	const LONGLONG GetRebuildInput()			{ return m_i64RebuildInput; }

private:
	SCHARDATA2(const SCHARDATA2 &Inven )		{ GASSERT(0&&"������ ���� ����!"); }
	SCHARDATA2& operator= ( SCHARDATA2 &Inven )	{ GASSERT(0&&"������ ���� ����!"); return *this; }
};
typedef SCHARDATA2* PCHARDATA2;


struct SSTATEBLOW
{
	EMSTATE_BLOW	emBLOW;			//	���� �̻� ����.
	float			fAGE;			//	���� �ð�.
	float			fSTATE_VAR1;	//	���� �� 1.
	float			fSTATE_VAR2;	//	���� �� 2.

	SSTATEBLOW () :
		emBLOW(EMBLOW_NONE),
		fAGE(0),
		fSTATE_VAR1(0),
		fSTATE_VAR2(0)
	{

	}
};

struct SLANDEFFECT
{
	D3DXVECTOR2			vMinPos;
	D3DXVECTOR2			vMaxPos;
	EMLANDEFFECT_TYPE	emLandEffectType;
	float				fValue;

	SLANDEFFECT() :
		vMinPos( 0.0f, 0.0f ),
		vMaxPos( 0.0f, 0.0f ),
		emLandEffectType(EMLANDEFFECT_ATK_SPEED),
		fValue( 0.0f )
	{
	}

	void Init()
	{
		vMinPos			 = D3DXVECTOR2( 0.0f, 0.0f );
		vMaxPos			 = D3DXVECTOR2( 0.0f, 0.0f );
		emLandEffectType = EMLANDEFFECT_ATK_SPEED;
		fValue			 = 0.0f;
	}

	bool IsUse()
	{
		if( vMinPos		 	 == D3DXVECTOR2( 0.0f, 0.0f ) &&
			vMaxPos			 == D3DXVECTOR2( 0.0f, 0.0f ) &&
			emLandEffectType == EMLANDEFFECT_ATK_SPEED &&
			fValue			 == 0.0f ) return FALSE;
		return TRUE;
	}

	bool operator == ( const SLANDEFFECT &value )
	{
		if( vMaxPos			 != value.vMaxPos )			 return FALSE;
		if( vMinPos			 != value.vMinPos )			 return FALSE;
		if( emLandEffectType != value.emLandEffectType ) return FALSE;
		if( fValue		     != value.fValue )			 return FALSE;

		return TRUE;
	}

};

typedef std::vector<SLANDEFFECT> VEC_LANDEFF;
typedef VEC_LANDEFF::iterator	 VEC_LANDEFF_ITER;

struct SPASSIVE_SKILL_DATA
{
	short	m_nHP;
	short	m_nMP;
	short	m_nSP;

	short	m_nDAMAGE;
	short	m_nDEFENSE;

	short	m_nHIT;
	short	m_nAVOID;

	short	m_nPIERCE;
	float	m_fTARRANGE;

	float	m_fMOVEVELO;
	float	m_fATTVELO;
	float	m_fSKILLDELAY;

	float	m_fINCR_HP;
	float	m_fINCR_MP;
	float	m_fINCR_SP;

	float	m_fDAMAGE_RATE;
	float	m_fDEFENSE_RATE;

	DAMAGE_SPEC m_sDamageSpec;

	short	m_nPA;
	short	m_nSA;
	short	m_nMA;

	float	m_fHP_RATE;
	float	m_fMP_RATE;
	float	m_fSP_RATE;

	SRESIST	m_sSUMRESIST;					//	���װ�.

	/*skill range spec logic, Juver, 2017/06/06 */
	float	m_fSUM_SKILL_ATTACKRANGE;
	float	m_fSUM_SKILL_APPLYRANGE;

	/*summon time, Juver, 2017/12/12 */
	int		m_nSummonTime;

	SPASSIVE_SKILL_DATA () :
		m_nHP(0),
		m_nMP(0),
		m_nSP(0),

		m_fINCR_HP(0),
		m_fINCR_MP(0),
		m_fINCR_SP(0),

		m_nDAMAGE(0),
		m_nDEFENSE(0),

		m_nHIT(0),
		m_nAVOID(0),

		m_nPIERCE(0),
		m_fTARRANGE(0),

		m_fMOVEVELO(0),
		m_fATTVELO(0),
		m_fSKILLDELAY(0),

		m_fDAMAGE_RATE(0),
		m_fDEFENSE_RATE(0),

		m_nPA(0),
		m_nSA(0),
		m_nMA(0),

		m_fHP_RATE(0),
		m_fMP_RATE(0),
		m_fSP_RATE(0),

		/*skill range spec logic, Juver, 2017/06/06 */
		m_fSUM_SKILL_ATTACKRANGE(0.0f),
		m_fSUM_SKILL_APPLYRANGE(0.0f),

		/*summon time, Juver, 2017/12/12 */
		m_nSummonTime(0)
	{
	}
};

struct SPKCOMBO_COUNT
{
	bool		bShow;
	int			nCount;
	float		fTime;

	SPKCOMBO_COUNT() :
	bShow(FALSE),
		nCount(0),
		fTime(0)
	{
	}

	bool IsACTIVE()
	{
		return bShow != FALSE;
	}

	void RESET()
	{
		bShow = FALSE;
		nCount = 0;
		fTime = 0;
	};
};
struct SEventState
{
	float				fItemGainRate;		// �̺�Ʈ �� ��ԵǴ� ������ �����
	float				fExpGainRate;		// �̺�Ʈ �� ��ԵǴ� ����ġ ����
	int					MinEventLevel;		// �̺�Ʈ �ּ� ����
	int					MaxEventLevel;		// �̺�Ʈ �ִ� ����
	bool				bEventStart;		// �̺�Ʈ�� ���۵ƴ��� �ƴ���
	int					EventPlayTime;		// �̺�Ʈ�� ���� �Ǵ� �÷��� �ð�
	int					EventBusterTime;	// �̺�Ʈ�� ���ӵǴ� �ð�
	CTime				EventStartTime;		// �̺�Ʈ�� ���� ������ �ð�
	DWORD				dwEventEndMinute;	// �̺�Ʈ ���� �ð�


	SEventState()
	{
		Init();
	}
	void Init()
	{
		fItemGainRate    = 1.0f;
		fExpGainRate     = 1.0f;
		MinEventLevel    = 0;		// �̺�Ʈ �ּ� ����
		MaxEventLevel    = 0;		// �̺�Ʈ �ִ� ����
		bEventStart      = FALSE;			// �̺�Ʈ�� ���۵ƴ��� �ƴ���
		EventPlayTime    = 0;		// �̺�Ʈ�� ���� �ð�
		EventBusterTime  = 0;		// �̺�Ʈ�� ���ӵǴ� �ð�
		EventStartTime   = 0;
		dwEventEndMinute = 0;

	}
};

struct SEVENT_FACT
{
	INT				nType;
	WORD			wSpeed;
	WORD			wASpeed;
	WORD			wAttack;

	SEVENT_FACT () 
		: nType(EMGM_EVENT_NONE)
		, wSpeed(0)
		, wASpeed(0)
		, wAttack(0)
	{
	}

	bool IsACTIVE( EMGM_EVENT_TYPE emType )
	{
		return (nType&emType)!=EMGM_EVENT_NONE;
	}

	void SetEVENT( EMGM_EVENT_TYPE emType, WORD wValue )
	{
		nType |= emType;

		switch( emType )
		{
		case EMGM_EVENT_SPEED:	wSpeed=wValue;	break;
		case EMGM_EVENT_ASPEED:	wASpeed=wValue;	break;
		case EMGM_EVENT_ATTACK:	wAttack=wValue;	break;
		}
	}

	void ResetEVENT( EMGM_EVENT_TYPE emType )
	{
		nType &= ~emType;
	}
};

struct SDROP_STATEBLOW
{
	EMSTATE_BLOW	emBLOW;			//	���� �̻� ����.
	float			fAGE;			//	���� �ð�.
	float			fSTATE_VAR1;	//	���� �� 1.
	float			fSTATE_VAR2;	//	���� �� 2.

	SDROP_STATEBLOW () 
		: emBLOW(EMBLOW_NONE)
		, fAGE(0)
		, fSTATE_VAR1(0)
		, fSTATE_VAR2(0)
	{
	}

	SDROP_STATEBLOW& operator= ( SSTATEBLOW &sblow )
	{
		emBLOW = sblow.emBLOW;
		fAGE = sblow.fAGE;
		fSTATE_VAR1 = sblow.fSTATE_VAR1;
		fSTATE_VAR2 = sblow.fSTATE_VAR2;

		return *this;
	}
};


struct SDROP_CHAR
{
	enum
	{
		CHAR_GEN	= 0x001,
		CLUB_CD		= 0x002
	};

	char			szName[CHAR_SZNAME];		//	�̸�.
	EMTRIBE			emTribe;					//	����.
	EMCHARCLASS		emClass;					//	����.
	WORD			wSchool;					//	�п�.
	WORD			wHair;						//	�Ӹ�ī��.
	WORD			wHairColor;					//  �Ӹ�ī�� �÷�
	WORD			wFace;						//	�󱼸��.
	WORD			wSex;						//  ����

	int				nBright;					//	�Ӽ�.

	DWORD			dwCharID;					//	�ɸ���ID.
	WORD			wLevel;						//	����.
	DWORD			dwGuild;					//	��� ��ȣ.
	DWORD			dwAlliance;					//	���� ��ȣ
	char			szClubName[CHAR_SZNAME];	//	Ŭ�� �̸�.
	DWORD			dwGuildMarkVer;				//	��� ��ũ ��ȣ.
	DWORD			dwGuildMaster;				//	��� ������.
	char			szNick[CHAR_SZNAME];		//	����.
	
	DWORD			dwParty;					//	��Ƽ ��ȣ.
	DWORD			dwPMasterID;				//	��Ƽ ������ ID.

	GLDWDATA		sHP;						//	������. ( ����/�ִ뷮 )

	DWORD			dwGaeaID;					//	���� �޸� �ε�����.
	SNATIVEID		sMapID;						//	���� �� ID.
	DWORD			dwCeID;						//	�� ID.
	D3DXVECTOR3		vPos;						//	��ġ.
	D3DXVECTOR3		vDir;						//	��ġ.

	EMACTIONTYPE	Action;						//	���� �׼�.
	DWORD			dwActState;					//	���� �׼� �÷���.
	D3DXVECTOR3		vTarPos;					//	���� ��ǥ ��ġ.

	/*skill summon, Juver, 2017/10/09 */
	DWORD			dwSummonGUID[SKILL_SUMMON_MAX_CLIENT_NUM];				//  ��ȯ�� ID

	SDROP_SKILLFACT	sSKILLFACT[SKILLFACT_SIZE];

	 /*itemfood system, Juver, 2017/05/24 */
	SDROP_FITEMFACT sFITEMFACT[FITEMFACT_SIZE];

	/*system buffs, Juver, 2017/09/04 */
	SDROP_SYSTEM_BUFF sSYSTEM_BUFF[SYSTEM_BUFF_SIZE]; 

	SDROP_STATEBLOW	sSTATEBLOWS[EMBLOW_MULTI];
	int				nLandEffect[EMLANDEFFECT_MULTI];	//  ���� �̻� ȿ����

	SQITEM_FACT		sQITEMFACT;
	SEVENT_FACT		sEVENTFACT;
	SEventState		sEventState;
	SPKCOMBO_COUNT	sPKCOMBOCOUNT;
	
	SITEMCLIENT		m_PutOnItems[SLOT_NSIZE_S_2];	//	���� Item.
	BOOL			m_bVehicle;
	CLIENT_VEHICLE	m_sVehicle;

	bool			m_bItemShopOpen;		// ItemShopOpen

	BOOL			m_bUseArmSub;				// �ذ��θ� ���� ���� ���� ��뿩��

	DWORD			dwFLAGS;					//	��Ÿ �Ӽ�.

	SPASSIVE_SKILL_DATA	sPASSIVE_SKILL;			//	passive skill data.
	
	DWORD			m_dwANISUBTYPE;				//	�ִϸ��̼� ����Ÿ��

	float			m_fScaleRange;

	bool			m_bBoosterStart; /*vehicle booster system, Juver, 2017/08/12 */

	/*activity system, Juver, 2017/11/04 */
	char			szBadge[CHAR_SZNAME];

	/*hide gm info, Juver, 2017/11/26 */
	DWORD			dwUserID;
	DWORD			dwUserLevel;

	/*pvp capture the flag, Juver, 2018/01/29 */
	WORD			m_wCaptureTheFlagTeam;
	bool			m_bCaptureTheFlagHoldFlag;

	bool			bSafeTime;

	EMACCTYPE		emUserType;

	/*Codex System, Jhoniex 2/14/2024 */
	DWORD			m_dwActivityDoneSize;
	DWORD			m_dwHPIncrease;
	DWORD			m_dwMPIncrease;
	DWORD			m_dwSPIncrease;
	DWORD			m_dwAttackIncrease;
	DWORD			m_dwDefenseIncrease;
	DWORD			m_dwShootingIncrease;
	DWORD			m_dwMeleeIncrease;
	DWORD			m_dwEnergyIncrease;
	DWORD			m_dwResistanceIncrease;
	DWORD			m_dwHitrateIncrease;
	DWORD			m_dwAvoidrateIncrease;

	SDROP_CHAR () 
		: emTribe(TRIBE_HUMAN)
		, emClass(GLCC_BRAWLER_M)
		, wSchool(0)
		, wHairColor(0)
		, wSex(0)
		, wHair(0)
		, wFace(0)
		, nBright(0)
		, dwCharID(0)
		, wLevel(1)
		, dwGuild(CLUB_NULL)
		, dwGuildMarkVer(0)
		, dwGuildMaster(0)
		, dwAlliance(0)
		, dwParty(PARTY_NULL)
		, dwPMasterID(GAEAID_NULL)
		, dwGaeaID(0)
		, dwCeID(0)
		, vPos(0,0,0)
		, vDir(0,0,-1)
		, Action(GLAT_IDLE)
		, dwActState(NULL)
		, vTarPos(0,0,0)
		, dwFLAGS(NULL)
		, m_bUseArmSub(FALSE)
		, m_bVehicle ( FALSE )
		, m_bItemShopOpen( false )
		//, dwSummonGUID(GAEAID_NULL)
		, m_dwANISUBTYPE( 0 )
		, m_fScaleRange( 1.0f )
		, m_bBoosterStart(false) /*vehicle booster system, Juver, 2017/08/12 */

		/*hide gm info, Juver, 2017/11/26 */
		, dwUserID(0)
		, dwUserLevel(0)

		/*pvp capture the flag, Juver, 2018/01/29 */
		, m_wCaptureTheFlagTeam(CAPTURE_THE_FLAG_TEAM_SIZE)
		, m_bCaptureTheFlagHoldFlag(false)
		, bSafeTime ( false )
		, emUserType( EM_TYPE_NORMAL )
		/*Codex System, Jhoniex 2/14/2024 */
		, m_dwHPIncrease(0)
		, m_dwMPIncrease(0)
		, m_dwSPIncrease(0)
		, m_dwAttackIncrease(0)
		, m_dwDefenseIncrease(0)
		, m_dwShootingIncrease(0)
		, m_dwMeleeIncrease(0)
		, m_dwEnergyIncrease(0)
		, m_dwResistanceIncrease(0)
		, m_dwHitrateIncrease(0)
		, m_dwAvoidrateIncrease(0)
	{
		for( int i=0; i < EMLANDEFFECT_MULTI; i++)
		{
			nLandEffect[i] = -1;
		}
		memset(szName, 0, sizeof(char) * CHAR_SZNAME);
		memset(szNick, 0, sizeof(char) * CHAR_SZNAME);
		memset(szClubName, 0, sizeof(char) * CHAR_SZNAME);

		/*activity system, Juver, 2017/11/05 */
		memset(szBadge, 0, sizeof(char) * CHAR_SZNAME);

		/*skill summon, Juver, 2017/10/09 */
		for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
			dwSummonGUID[i] = GAEAID_NULL;
	}
};


struct SINFO_CHAR
{
	DWORD			dwCharID;
	DWORD			dwGaeaID;
	char			szName[CHAR_SZNAME];
	EMCHARCLASS		emClass;
	WORD			wSchool;
	WORD			wHair;
	WORD			wHairColor;
	WORD			wFace;
	WORD			wLevel;

	char			szClubName[CHAR_SZNAME];

	GLDWDATA		sdDAMAGE;
	WORD			wHP;
	WORD			wMP;
	WORD			wSP;

	SITEMCUSTOM		sPutOnItems[SLOT_NSIZE_S_2];

	SCHARSTATS		sSUMSTATS;

	WORD		wSUM_PA;
	WORD		wSUM_SA;
	WORD		wSUM_MA;

	SRESIST		sSUMRESIST;

	int			nSUM_DEF;
	int			nSUM_HIT;
	int			nSUM_AVOID;
	bool		bUseArmSub;
	bool		bPrivate;

	/*activity system, Juver, 2017/11/04 */
	char			szBadge[CHAR_SZNAME];

	float		fScaleRange;

	SINFO_CHAR () 
		: emClass(GLCC_BRAWLER_M)
		, wSchool(0)
		, wHairColor(0)
		, wHair(0)
		, wFace(0)
		, dwGaeaID(GAEAID_NULL)
		, dwCharID(GAEAID_NULL)
		, wLevel(1)
		, wSUM_PA( 0 )
		, wSUM_SA( 0 )
		, wSUM_MA( 0 )
		, nSUM_DEF( 0 )
		, nSUM_HIT( 0 )
		, nSUM_AVOID( 0 )
		, bUseArmSub( FALSE )
		, bPrivate(false)
		, fScaleRange(1.0f)
	{
		memset(szName, 0, sizeof(char) * CHAR_SZNAME);
		memset(szClubName, 0, sizeof(char) * CHAR_SZNAME);
		StringCchCopy ( szClubName, CHAR_SZNAME, "No Guild" );

		/*activity system, Juver, 2017/11/05 */
		memset(szBadge, 0, sizeof(char) * CHAR_SZNAME);
		StringCchCopy ( szBadge, CHAR_SZNAME, "No Badge" );
	}
};

#endif // GLCHARDATA_H_