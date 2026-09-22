#pragma once

#include <string>

#include "./GLCharData.h"
#include "./GLParty.h"
#include "./GLInventory.h"
#include "./GLPet.h"
#include "./GLVEHICLE.h"
#include "./GLSUMMON.h"
#include "./GLSummonDef.h"

#include "../Lib_Engine/Common/SFileSystem.h"
#include "../Lib_Engine/Meshs/DxSkinAniMan.h"
#include "../Lib_Network/s_NetGlobal.h"

/*pvp tyranny, Juver, 2017/08/25 */
#include "./GLPVPTyrannyDefine.h" 

/*pvp capture the flag, Juver, 2018/02/07 */
#include "./GLPVPCaptureTheFlagDefine.h"

namespace GLOGIC
{
	extern BOOL			bGLOGIC_ZIPFILE;
	extern std::string	strGLOGIC_ZIPFILE;
	extern std::string	strQUEST_ZIPFILE;
	extern std::string	strLEVEL_ZIPFILE;
	extern std::string	strNPCTALK_ZIPFILE;

	void SetFullPath( BOOL bPack );
};

enum GLCONST
{
	MAX_SERVERCHAR		= MAX_ONESERVERCHAR_NUM,
	MAX_SELECTCHAR		= 3, // ĳ���� ���� ȭ�鿡�� ���̴� �ִ� ĳ������ ��, ���� 4��
	//MAX_VIEWRANGE		= 300, // �ִ� ���� (��Ƽ�� ���� �����Ѱ�?...)
	MAX_VIEWRANGE		= 250,
	MAX_SUMMON_VIEWRANGE= 100,

	MAX_HEAD			= 20,
	MAX_HAIR			= 20,

	MAX_SCHOOL			= 10,
	MAX_LEVEL			= 300,

	MAX_CLUBSTORAGE		= 5,
	MAX_CLUBRANK		= 10,

	GRADE_NORMAL		= 4,
	GRADE_HIGH			= 9,
	GRADE_LIMIT_MAX		= 15,
};

//	Note : ĳ���� ������ �⺻ ���.
//
struct GLCONST_CHARCLASS
{
	float				fWALKVELO;		//	ĳ���� �ȱ� �ӵ�.
	float				fRUNVELO;		//	ĳ���� �ٱ� �ӵ�.

	float				fHP_STR;		//	str(ü��) -> HP ȯ�� ����.
	float				fMP_SPI;		//	spi(����) -> MP ȯ�� ����.
	float				fSP_STA;		//	sta(�ٷ�) -> SP ȯ�� ����.
	float				fHIT_DEX;		//	dex(��ø) -> HIT rate ȯ�� ����.
	float				fAVOID_DEX;		//	dex(��ø) -> AVOID rate ȯ�� ����.
	float				fDEFENSE_DEX;	//	dex(��ø) -> DEFENSE ȯ�� ����.

	float				fPA_POW;		//	pow(��) -> PA(����ġ) ȯ�� ����.
	float				fPA_DEX;		//	dex(��ø) -> PA(����ġ) ȯ�� ����.

	float				fSA_POW;		//	pow(��) -> SA(���ġ) ȯ�� ����.
	float				fSA_DEX;		//	dex(��ø) -> SA(���ġ) ȯ�� ����.

	float				fMA_DEX;		//	dex(��ø) -> MA(����ġ) ȯ�� ����.
	float				fMA_SPI;		//	spi(����) -> MA(����ġ) ȯ�� ����.
	float				fMA_INT;		//	int(����) -> MA(����ġ) ȯ�� ����.

	float				fCONV_AP;		//	ȯ�� �⺻ ���ݷ�.
	float				fCONV_DP;		//	ȯ�� �⺻ ����.
	float				fCONV_PA;		//	ȯ�� �⺻ ����ġ.
	float				fCONV_SA;		//	ȯ�� �⺻ ���ġ.

	SCHARSTATS			sBEGIN_STATS;	//	STATS �ʱ� ��ġ.
	FCHARSTATS			sLVLUP_STATS;	//	������ STATS �⺻ ���� ��ġ.

	WORD				wBEGIN_AP;		//	�ʱ� ���ݷ�.
	WORD				wBEGIN_DP;		//	�ʱ� ����.
	WORD				wBEGIN_PA;		//	�ʱ� ����ġ.
	WORD				wBEGIN_SA;		//	�ʱ� ���ġ.

	float				fLVLUP_AP;		//	������ ���ݷ� ����.
	float				fLVLUP_DP;		//	������ ���� ����.
	float				fLVLUP_PA;		//	������ ����ġ ����.
	float				fLVLUP_SA;		//	������ ���ġ ����.

	//	���ϸ��̼� ����. ( �ð�/���� )
	//	server ������ �ʿ��� ����.
	VECANIATTACK		m_ANIMATION[AN_TYPE_SIZE][AN_SUB_00_SIZE];

	DWORD				dwHEADNUM;
	DWORD				dwHEADNUM_SELECT;
	std::string			strHEAD_CPS[MAX_HEAD];

	DWORD				dwHAIRNUM;
	DWORD				dwHAIRNUM_SELECT;
	std::string			strHAIR_CPS[MAX_HAIR];

	std::string			strCLASS_EFFECT;

	void LoadAniSet ( char* szSkinObj );
	void ClearAniSet ();

	BOOL LOADFILE ( const char* szFileName );

	GLCONST_CHARCLASS () :
		fWALKVELO(12),
		fRUNVELO(34),

		fHP_STR(0),
		fMP_SPI(0),
		fSP_STA(0),
		fHIT_DEX(0),
		fAVOID_DEX(0),
		fPA_POW(0),
		fSA_DEX(0),
		
		fMA_DEX(0),
		fMA_SPI(0),
		fMA_INT(0),

		fCONV_AP(0),
		fCONV_DP(0),
		fCONV_PA(0),
		fCONV_SA(0),

		wBEGIN_AP(0),
		wBEGIN_DP(0),
		wBEGIN_PA(0),
		wBEGIN_SA(0),

		fLVLUP_AP(0),
		fLVLUP_DP(0),
		fLVLUP_PA(0),
		fLVLUP_SA(0),

		dwHEADNUM(0),
		dwHEADNUM_SELECT(0),

		dwHAIRNUM(0),
		dwHAIRNUM_SELECT(0)
	{
	}
	
	GLCONST_CHARCLASS ( float _fHP_STR, float _fMP_SPI, float _fSP_STA, float _fHIT_DEX,
		float _fAVOID_DEX, float _fDEFENSE_DEX, float _fPA_POW, float _fSA_DEX,
		SCHARSTATS _fBEGIN_STATS, FCHARSTATS _fLVLUP_STATS,
		WORD _wBEGIN_AP, WORD _wBEGIN_DP, WORD _wBEGIN_PA, WORD _wBEGIN_SA,
		float _fLVLUP_AP, float _fLVLUP_DP, float _fLVLUP_PA, float _fLVLUP_SA ) :
		fWALKVELO(12),
		fRUNVELO(34),

		fHP_STR(_fHP_STR),
		fMP_SPI(_fMP_SPI),
		fSP_STA(_fSP_STA),
		fHIT_DEX(_fHIT_DEX),
		fAVOID_DEX(_fAVOID_DEX),
		fDEFENSE_DEX(_fDEFENSE_DEX),
		fPA_POW(_fPA_POW),
		fSA_DEX(_fSA_DEX),

		sBEGIN_STATS(_fBEGIN_STATS),
		sLVLUP_STATS(_fLVLUP_STATS),
		wBEGIN_AP(_wBEGIN_AP),
		wBEGIN_DP(_wBEGIN_DP),
		wBEGIN_PA(_wBEGIN_PA),
		wBEGIN_SA(_wBEGIN_SA),

		fLVLUP_AP(_fLVLUP_AP),
		fLVLUP_DP(_fLVLUP_DP),
		fLVLUP_PA(_fLVLUP_PA),
		fLVLUP_SA(_fLVLUP_SA)
	{
	}

	~GLCONST_CHARCLASS()
	{
	}

private:
	//	���� ������ ���Ͼ�� ��õ������ ��������.
	GLCONST_CHARCLASS& operator= ( GLCONST_CHARCLASS &Input )	{ GASSERT(0); };	
};

struct GLCLUBRANK
{
	DWORD	m_dwMasterLvl;
	DWORD	m_dwLivingPoint;
	DWORD	m_dwPay;
	DWORD	m_dwMember;

	GLCLUBRANK () :
		m_dwMasterLvl(0),
		m_dwLivingPoint(0),
		m_dwPay(0),
		m_dwMember(0)
	{
	}

	GLCLUBRANK ( DWORD dwMLvl, DWORD dwLP, DWORD dwPY, DWORD dwMem ) :
		m_dwMasterLvl(dwMLvl),
		m_dwLivingPoint(dwLP),
		m_dwPay(dwPY),
		m_dwMember(dwMem)
	{
	}
};

struct SPLAYERKILL
{
	DWORD		dwLEVEL;
	DWORD		dwNAME_COLOR;
	int			nPKPOINT;
	std::string	strNAME;
	float		fPK_EXP_RATE;

	DWORD		dwITEM_DROP_NUM;
	float		fITEM_DROP_RATE;
	
	float		fSHOP_2BUY_RATE;
	float		fSHOP_2SALE_RATE;

	SPLAYERKILL () :
		dwLEVEL(0),
		dwNAME_COLOR(0),
		nPKPOINT(0),
		fPK_EXP_RATE(0),

		dwITEM_DROP_NUM(0),
		fITEM_DROP_RATE(0),
		
		fSHOP_2BUY_RATE(0),
		fSHOP_2SALE_RATE(0)
	{
	}

	SPLAYERKILL ( DWORD _dwLEVEL, DWORD _dwNAME_COLOR, int _nPKPOINT, std::string _strNAME, float _fPK_EXP_RATE,
		DWORD _dwITEM_DROP_NUM, float _fITEM_DROP_RATE, float _fSHOP_2BUY_RATE, float _fSHOP_2SALE_RATE ) :
		dwLEVEL(_dwLEVEL),
		dwNAME_COLOR(_dwNAME_COLOR),
		nPKPOINT(_nPKPOINT),
		strNAME(_strNAME),
		fPK_EXP_RATE(_fPK_EXP_RATE),

		dwITEM_DROP_NUM(_dwITEM_DROP_NUM),
		fITEM_DROP_RATE(_fITEM_DROP_RATE),
		
		fSHOP_2BUY_RATE(_fSHOP_2BUY_RATE),
		fSHOP_2SALE_RATE(_fSHOP_2SALE_RATE)
	{
	}

	SPLAYERKILL ( const SPLAYERKILL &value )
	{
		operator= ( value );
	}

	SPLAYERKILL& operator= ( const SPLAYERKILL &value )
	{
		dwLEVEL = value.dwLEVEL;
		dwNAME_COLOR = value.dwNAME_COLOR;
		nPKPOINT = value.nPKPOINT;
		strNAME = value.strNAME;
		fPK_EXP_RATE = value.fPK_EXP_RATE;

		dwITEM_DROP_NUM = value.dwITEM_DROP_NUM;
		fITEM_DROP_RATE = value.fITEM_DROP_RATE;
		
		fSHOP_2BUY_RATE = value.fSHOP_2BUY_RATE;
		fSHOP_2SALE_RATE = value.fSHOP_2SALE_RATE;

		return *this;
	}
};

//Weapon Skill - SqueeeAK
struct GLWEAPON_LINKSKILL
{
	SNATIVEID			sNativeID;

	float				fPossibility;
	float				fRate;

	GLWEAPON_LINKSKILL () 
		: sNativeID( NATIVEID_NULL() )
		, fPossibility(0.0f)
		, fRate(0.0f)
	{
	}
};

//	Note : ĳ���� ���� �⺻ ���.
//
namespace GLCONST_CHAR
{
	enum
	{
		DIE_DECEXP_NUM	= 30,
	};

	//PANDORA
    extern VECPANDORABOX            vecPandoraBoxes;
	//PANDORA
    extern GLPANDORA_BOX_SETTING      sPandoraSetting;


	extern BOOL			bTESTSERVER;			// �׽�Ʈ �������� ����.
	extern INT			nUI_KEYBOARD;			// �������̽� Ű���� ����
	extern BOOL			bBATTLEROYAL;			// ��Ʋ�ξ�
	extern INT			nMAX_FRIEND_NUMBER;		// �ִ� ģ��, ���� ��� ����
	extern WORD			wLEVEL_FOR_EXTREME;		// �ذ��θ� �����ϱ� ���� �����Ǿ�� �ϴ� ����(lv.192)

	//	Note : �Ϲ� ���.
	//
	extern float		fFIELD_NEXTPICK;		//	�㿡 �ִ� ������(��) ������ ó�� �޽��� ������ ���� ���� �ð�.
	extern WORD			wSERVER_NORMAL;
	extern WORD			wSERVER_CONGEST;
	extern WORD			wSERVER_NOVACANCY;

	//---------------------------------------------------------------------------------------[�ɸ��� �⺻]
	extern WORD			wSCHOOLNUM;
	extern std::string	strSCHOOLNAME[MAX_SCHOOL];
	extern SNATIVEID	nidSTARTMAP[MAX_SCHOOL];
	extern DWORD		dwSTARTGATE[MAX_SCHOOL];
	extern float		fCHAR_HEIGHT[GLCI_NUM_8CLASS];

	//---------------------------------------------------------------------------------------[]
	extern BOOL			bPARTY_2OTHERSCHOOL;
	extern BOOL			bCLUB_2OTHERSCHOOL;

    //---------------------------------------------------------------------------------------[��Ȱ â ��õ ������ ���]
    extern SNATIVEID   nidREBIRTH_RECOMMEND_ITEM;    // ��Ȱ â���� ��õ�Ǵ� ������ �̹����� ID.
    extern std::string strREBIRTH_RECOMMEND_ITEM;    // ��Ȱ â���� ��õ�Ǿ� ����Ʈ �������� �˻��� �������� �̸�.
    extern WORD wAUTO_REBIRTH_TIMER; // ��Ȱ â���� �ڵ� ��Ȱ �ð�.

    //---------------------------------------------------------------------------------------[NPC ��ȭ â ��õ ������ ���]
    extern SNATIVEID nidDIALOGUE_RECOMMEND_ITEM;    // NPC ��ȭ â���� ��õ�Ǵ� ������ �̹����� ID.
    extern std::string strDIALOGUE_RECOMMEND_ITEM;    // NPC ��ȭ â���� ��õ�Ǿ� ����Ʈ �������� �˻��� �������� �̸�.

//	extern float		fDISPRICE;				//	������ ���Ǹ� ����.
    extern WORD			wMAXITEM_AGE;			//	ITEM, �ִ� ���ӽð�.
	extern WORD			wMAXITEM_HOLD;			//	ITEM, �ӽ� ������ ��ȣ �ð�.
	extern WORD			wMAXMOB_TRACKING;		//	�ִ� �� ���� ��.

	extern WORD			wMAX_LEVEL;				//	���� �ִ� ����.
	extern WORD			wMAX_EXTREME_LEVEL;		//	�ذ��� ���� �ִ� ����.
	extern WORD			wLVL_STATS_P;			//	������ ���� ����Ʈ �ο���.
	extern WORD			wLVL_2ndSTATS_P;			//	������ ���� ����Ʈ �ο���.
	extern float		fLVL_EXP_G;				//	���� ����ġ ���⿡ �ʿ��� ȯ�� ����.
	extern float		fLVL_EXP_S;				//	���� ����ġ ���⿡ �ʿ��� ȯ�� ����.
	extern float		fKILL_EXP_RATE;			//	"��ȹ�� ���� ����ġ" * ���϶� ����ġ.

	/*dmk14 level required gametime*/
	extern WORD			wLEVEL_REQ_GAMETIME;


	extern float		fDIE_DECEXP[DIE_DECEXP_NUM];	//	����� ����ġ ���� ����.

	extern float		fDIE_RECOVERYEXP[DIE_DECEXP_NUM];	// ����ġ ���� ����
	extern float		fEXP_RATE_MONEY[DIE_DECEXP_NUM];	// ����ġ ���� ���

	extern float		fREC_EXP_RATE;			//	������ ȸ������ �ٶ� ȹ���ϴ� ����ġ�� ����.

	//---------------------------------------------------------------------------------------[����]

	extern WORD			wGRADE_MAX;				// ���� �Ҽ� �ִ� ��� ����ġ 
	extern WORD			wGRADE_MAX_REGI;		// ���� �Ҽ� �ִ� ��� ����ġ(����) 
	
	extern WORD			wDAMAGE_GRADE;			//	�� ��޴� ���� ���ݷ� ��ġ.
	extern WORD			wDEFENSE_GRADE;			//	�� ��޴� ���� ���� ��ġ.

	extern float		fDAMAGE_GRADE;			//	�� ��޴� ������ ���ݷ� ��ġ.
	extern float		fDEFENSE_GRADE;			//	�� ��޴� ������ ���� ��ġ.

	extern float		fDAMAGE_GRADE_TOP[GRADE_LIMIT_MAX-GRADE_HIGH];		// �ֻ��� ��� ���ݷ� ������
	extern float		fDEFENSE_GRADE_TOP[GRADE_LIMIT_MAX-GRADE_HIGH];	// �ֻ��� ��� ���� ������
	extern	WORD		wUSE_GRADE_NUM[GRADE_LIMIT_MAX-GRADE_HIGH];			// �ֻ��� ��� �ʿ��� ������ ����

	extern WORD			wRESIST_FIRE_GRADE;		//	�� ��޴� ���� ����(ȭ) ��ġ.
	extern WORD			wRESIST_ICE_GRADE;		//	�� ��޴� ���� ����(��) ��ġ.
	extern WORD			wRESIST_ELEC_GRADE;		//	�� ��޴� ���� ����(��) ��ġ.
	extern WORD			wRESIST_POISON_GRADE;	//	�� ��޴� ���� ����(��) ��ġ.
	extern WORD			wRESIST_SPIRIT_GRADE;	//	�� ��޴� ���� ����(��) ��ġ.

	//---------------------------------------------------------------------------------------
	extern WORD			wLVL_SKILL_P;			//	������ ��ų ����Ʈ �ο���.
	extern WORD			wLVL_2ndSKILL_P;			//	������ ��ų ����Ʈ �ο���.

	extern WORD			wMAXATRANGE_SHORT;		//	���� ���ݰŸ�.
	extern float		fPUSHPULL_VELO;			//	�аų� ��涧 �̵� �ӵ�.

	extern float		fMOB_TRACING;			//	�� ���� �Ÿ�.
	extern float		fLOW_SEED_DAMAGE;		//	damage ���� ���� ���� damage.
	extern float		fCONFT_SPTY_EXP;		//	�б� ��� �¸��ÿ� ��ü ����ġ �й�.

	extern WORD			wBODYRADIUS;			//	ĳ���� ��ü �ݰ�.

	extern float		fREACT_VALID_SCALE;		//	�׼��� ��ȿ �Ÿ� ����.

	extern float		fUNIT_TIME;				//	ȸ�� ���� �ð�.
	extern float		fHP_INC_PER;			//	�����ð��� HP ȸ����(%)
	extern float		fMP_INC_PER;			//	�����ð��� MP ȸ����(%)
	extern float		fSP_INC_PER;			//	�����ð��� SP ȸ����(%)

	extern float		fHP_INC;				//	�����ð��� HP ȸ����.
	extern float		fMP_INC;				//	�����ð��� MP ȸ����.
	extern float		fSP_INC;				//	�����ð��� SP ȸ����.

	extern float		fRESIST_PHYSIC_G;		//	���� ���ݽ� �Ӽ� ����ġ ���� ����.
	extern float		fRESIST_G;				//	�Ӽ� ����ġ ���� ����.
	extern float		fMAX_RESIST;

	extern WORD			wBASIC_DIS_SP;			//	�⺻ �Ҹ� SP.

	extern float		fLOWSP_MOTION;			//	���¹̳� (%) ���� ������ ��� ó��.
	extern float		fLOWSP_DAMAGE;			//	���¹̳� ������ Damage ����.
	extern float		fLOWSP_HIT_DROP;		//	���¹̳� ������ ������ ����.
	extern float		fLOWSP_AVOID_DROP;		//	���¹̳� ������ ȸ���� ����.
	extern DWORD		dwACCEPT_LOWERSTATS;	//	stats �������� ���¹̳��� Ŀ���� �� �ִ� ���� �ѵ�. ( ���� ���⿡ ����. )

	extern float		fDAMAGE_DEC_RATE;		//	����� ������.
	extern float		fDAMAGE_GRADE_K;		//	(�⺻����+�������) ����� �ݿ���.

	extern DWORD		dwCRITICAL_DAMAGE;		//	ũ��Ƽ�� ������ ����
	extern DWORD		dwCRITICAL_MAX;			//	ũ��Ƽ�� Ȯ�� MAX
	extern DWORD		dwCRUSHING_BLOW_DAMAGE;	//  ����Ÿ�� ������ ����
	extern DWORD		dwCRUSHING_BLOW_MAX;	//  ����Ÿ�� Ȯ�� max
	extern float		fCRUSH_BLOW_RANGE;		//  ����Ÿ�ݽ� �з����� �Ÿ� ( ������ ) 

	extern float		fPKCOMBO_RANGE_TIME;
		

	//	Note : ----------------------------------------------------------------���
	extern float		fCONFRONT_TIME;			//	��� ���� �ð�.
	extern float		fCONFRONT_ELAP;			//	���� ��� ���� ���� �ð�.
	extern DWORD		dwCONFRONT_MAX;			//	���� ��� �� ���� Ƚ��.
	extern float		fCONFRONT_RESET;		//	���� ��� ���� ���� �ð�.
	extern float		fCONFRONT_ONE_DIST;		//	��� �Ÿ� ����. ( 0�� ��� ���Ѵ�. )
	extern float		fCONFRONT_PY_DIST;		//	��� �Ÿ� ����. ( 0�� ��� ���Ѵ�. )
	extern float		fCONFRONT_CLB_DIST;		//	��� �Ÿ� ����. ( 0�� ��� ���Ѵ�. )

	extern int			nCONFRONT_WIN_LP;		//	��� �¸��� living point ( ��Ȱ ���� ).
	extern int			nCONFRONT_LOSS_LP;		//	��� �й�� living point ( ��Ȱ ���� ).

	extern int			nCONFRONT_CLB_WIN_LP;	//	Ŭ�� ��� �¸��� living point
	extern int			nCONFRONT_CLB_LOSS_LP;	//	Ŭ�� ��� �й�� living point

	//	Note : ----------------------------------------------------------------���
	enum { EMCONFT_RC_TYPENUM = 10, EMCONFT_STATE_TYPENUM = 8 };
	extern WORD			wCONFT_RC_TYPE[EMCONFT_RC_TYPENUM];			//	��� ȸ���� ��밡�� Ƚ��.
	extern float		fCONFT_STATE_TYPE[EMCONFT_STATE_TYPENUM];	//	��� ü�� Ȯ�� ����.

	extern WORD			wCONFRONT_SCHOOL_LIMIT_NUM;					//	�б��� ��� �ּ� �ο�.
	extern WORD			wCONFRONT_SCHOOL_LIMIT_LEVEL;				//	�б��� ��� �ּ� ����.
	extern float		fCONFRONT_SCHOOL_EXP_SCALE;					//	�б��� ��ý� ���� ����ġ ������.

	extern WORD			wCONFRONT_SCHOOL_WIN_NUM;
	extern WORD			wCONFRONT_SCHOOL_WIN_RATE;
	extern float		fCONFRONT_SCHOOL_GENITEM;

	//	Note : ----------------------------------------------------------------pk
	enum { EMPK_STATE_LEVEL = 5, };
	extern BOOL			bPK_MODE;				//	pk on/off.
	extern BOOL			bPKLESS;				//  �������� PK on/off.
	extern int			nPK_LIMIT_LEVEL;		//	pk ���� ����.
	extern int			nPK_LIMIT_DX_LEVEL;		//	pk ������ ���� ����.
	extern int			nPK_TRY_BRIGHT_POINT;	//	pk �õ��� ����Ʈ.
	extern int			nPK_KILL_BRIGHT_POINT;	//	pk �׿��� ��� ����Ʈ.
	extern int			nPK_TRY_LIVING_POINT;	//	pk �õ��� ����Ʈ.
	extern int			nPK_KILL_LIVING_POINT;	//	pk �׿��� ��� ����Ʈ.
	extern float		fNONPK_ITEM_DROP;		//	non pk �� ������ �����.
	extern float		fPK_ITEM_DROP;			//	pk �� ������ �����.

	extern DWORD		wItemPass;

	//	 ( pk level : 0 ~ 4 )
	//	0 - �Ϲ��л�
	//	1 - �ҷ��л�
	//	2 - ������
	//	3 - ���θ�
	//	4 - ���α�
	extern DWORD		dwPK_RECALL_ENABLE_LEVEL;	//	��ȯ ��� ���� ���� ����.
	extern DWORD		dwPK_DRUG_ENABLE_LEVEL;		//	ȸ���� ��� ���� ���� ����.


	extern float		fPK_JUSTNESS_TIME;		//	�ݰ� ������ ������� �ð�.
	extern float		fPK_SAFE_TIME;			//	pk ���� �ð�.

	extern DWORD		dwPK_NORMAL_NAME_COLOR;	//	pk ��ġ�� ������ �̸� ����.
	extern float		fPK_POINT_DEC_RATE;		//	pk ��ġ ������. ( �ð���. )
	extern float		fPK_POINT_DEC_PHY;		//  pk ������ ������ ( ���� ).
	extern float		fPK_POINT_DEC_MAGIC;	//  pk ������ ������ ( ���� ).
	
	extern SPLAYERKILL	sPK_STATE[EMPK_STATE_LEVEL];

	//	Note : ----------------------------------------------------------------�����̻�
	enum { EMSTATEBLOW_LEVEL_SIZE = 10, EMSTATEBLOW_LEVEL_BASE = 1, };
	extern int			nSTATEBLOW_LEVEL[EMSTATEBLOW_LEVEL_SIZE];

	//	Note : ----------------------------------------------------------------����ġ Table
	//
	enum { EXPTABLE_SIZE = 52, EXPTABLE_LVL = 19, EXPBASEINDEX = 10, EXP_LVL_STEP = 5,
	EXPTABLE_RANGE = 61,  EXPTABLE_RANGE_BASE = 30 };
	extern WORD			wEXPTABLE[EXPTABLE_SIZE][EXPTABLE_LVL];
	extern float		fEXP_RATE_TABLE[EXPTABLE_RANGE];

	extern LONGLONG		lnEXP_MAX_TABLE[MAX_LEVEL];
	extern LONGLONG		lnEXP_MAX_TABLE_2nd[MAX_LEVEL];
	extern DWORD		dwEXP_PC_TABLE[MAX_LEVEL];

	extern LONGLONG		lnSKILL_PREREQ[EMSKILLCLASS_NSIZE][GLSkillMan::MAX_CLASSSKILL];
	//	Note : ----------------------------------------------------------------��Ƽ ����ġ.
	extern float		fPARTYEXPRANGE;				//	����ġ ������ ������ �ִ� �Ÿ�.
	extern float		fPARTYEXP_S;				//	��Ƽ ����ġ �й�� �������̿� ���� ���Һ���.
	extern WORD			aPARTYEXP_ONE[MAXPARTY];
	extern WORD			aPARTYKILLEXP_ONE[MAXPARTY];

	//	Note : ----------------------------------------------------------------Ŭ��
	extern DWORD		dwCLUB_PARTYNUM;			//	Ŭ�� â���� �ּ� ��Ƽ(�ʱ�) �ο�.
	extern DWORD		dwCLUB_DISSOLUTION_DAY;		//	Ŭ�� ��ü ���� ��.
	extern DWORD		dwCLUB_JOINBLOCK_DAY;		//	Ŭ�� Ż��� ���� ���� ��.
	extern DWORD		dwMAX_CLUBRANK;
	extern GLCLUBRANK	sCLUBRANK[MAX_CLUBRANK];	//	Ŭ�� ��ũ�� ���.
	extern DWORD		dwCLUB_AUTHORITY;			//	Ŭ�������� ���� ���� �ð�			
	extern DWORD		dwCLUB_ALLIANCE_SEC;		//	Ŭ�� ���� Ż��, ������ �簡�� ���ѽð�.
	extern DWORD		dwCLUB_ALLIANCE_DIS;		//	Ŭ�� ���� ��ü�� �Ἲ ���ѽð�.
	extern DWORD		dwCLUB_ALLIANCE_NUM;		//	Ŭ�� ���� �ִ� ����.

	extern DWORD		bCLUB_BATTLE;				//	Ŭ�� ��Ʋ ���� ����
	extern DWORD		dwCLUB_BATTLE_MAX;			//  Ŭ�� ��Ʋ ���� ���� ����
	extern DWORD		dwCLUB_BATTLE_RANK;			//  Ŭ�� ��Ʋ ��ũ ����
	extern DWORD		dwCLUB_BATTLE_TIME;			//  Ŭ�� ��Ʋ �ð�(��)
	extern DWORD		dwCLUB_BATTLE_TIMEMIN;		//	Ŭ�� ��Ʋ �ּҽð�(��)
	extern DWORD		dwCLUB_BATTLE_TIMEMAX;		//	Ŭ�� ��Ʋ �ִ�ð�(��)
	extern DWORD		dwCLUB_BATTLE_GUID_TIME;	//  Ŭ�� ��Ʋ ������ ���� �ð�(��)
	extern DWORD		dwCLUB_BATTLE_DIS_TIME;		//  Ŭ�� ��Ʋ ���� �� �׺� ������ �ð�(��)
	extern BOOL			bCLUB_BATTLE_ALLIANCE;		//  Ŭ�� ��Ʋ�� ���� �ڵ� ���� ���

	extern BOOL			bCLUB_DEATHMATCH;			//	Ŭ�� ������ġ ���� ����
	extern DWORD		dwCLUB_DEATHMATCH_MEM;		//	Ŭ�� ������ġ ���� �����
	

	//	Note : ----------------------------------------------------------------������
	extern float		fMAX_COMMISSION;			//	�ִ� ���� ���� ������ ����.
	extern float		fDEFAULT_COMMISSION;		//	�� ���� ������ ������.
	extern float		fEARNING_RATE;				//	������ ������.
	extern float		fEARNING_RATE_NPC;			//	NPC ��ȯ ������ ������.
	extern float		fCDCERTIFY_DIST;			//	���� ���� �Ÿ�.
	extern float		fCDCERTIFY_DIST2;			//	���� ���� �Ÿ�.
	extern float		fCDCERTIFY_TIME;			//	���� �ҿ� �ð�.

	//	Note : ----------------------------------------------------------------���� ����
	extern float		fEXP_SCALE;					//	Note : ����ġ ����.
	extern float		fITEM_DROP_SCALE;			//	Note : ������ �����.
	extern float		fMONEY_DROP_SCALE;			//	Note : �� �����.
	extern float		fITEM_DROP_LOWER_SCALE;		//	Note : ��� ������ �����.

	extern float		fEXP_SCALE_BACKUP;			//	Note : ����ġ ����.
	extern float		fITEM_DROP_SCALE_BACKUP;	//	Note : ������ �����.
	extern float		fMONEY_DROP_SCALE_BACKUP;	//	Note : �� �����.

	//	Note : ----------------------------------------------------------------

	//	Note : ----------------------------------------------------------------�п� ���� pk
	extern bool			bSCHOOL_FREE_PK_ALWAYS;		//	�п��� ���� pk �׻� ���� ����.
	extern bool			bSCHOOL_FREE_PK;			//	�п��� ���� pk on/off.
	extern bool			bSCHOOL_FREE_PK_TIME_REAL;	//	�п��� ���� pk ���� �ð� ( on : ���� �ð�, off : ���ӽð� )
	extern bool			bSCHOOL_FREE_PK_Z_FORCED;	//	�п��� ���� pk ���� ����.
	extern DWORD		dwSCHOOL_FREE_PK_TIME_START;//	�п��� ���� pk ���� �ð�.
	extern DWORD		dwSCHOOL_FREE_PK_TIME_TERM;	//	�п��� ���� pk ���� �ð�.
	extern float		fSCHOOL_FREE_PK_ITEM_DROP;	//	�п��� ���� pk ������ ����� ���갪. ( �⺻ ����� + ���갪. )
	//	Note : ----------------------------------------------------------------

	//	Note : ----------------------------------------------------------------��� ����.
	extern bool			bENCHANT_TERMINATE_ITEM;	//	��þƮ�� ������ �ļ� ����.
	extern bool			bMONEY_DROP2FIELD;			//	�� �ٴڿ� ������ ���� ����.

	//	Note : ----------------------------------------------------------------���� Ȯ��.
	extern float		fGRADE_RATE[GRADE_LIMIT_MAX];
	extern float		fGRADE_RESET_RATE[GRADE_LIMIT_MAX];
	extern float		fGRADE_TERMINATE_RATE[GRADE_LIMIT_MAX];

	//add stats limit
	extern DWORD		dwPowMax;
	extern DWORD		dwStrMax;
	extern DWORD		dwSpiMax;
	extern DWORD		dwDexMax;
	extern DWORD		dwIntMax;
	extern DWORD		dwStaMax;
	//	Note : ----------------------------------------------------------------����
	extern std::string	strGRINDING_SUCCEED;
	extern std::string	strGRINDING_FAIL;
	extern std::string	strGRINDING_RESET;
	extern std::string	strGRINDING_BROKEN;

	extern std::string	strGAMBLING_SHUFFLE;	// Monster7j
	extern std::string	strGAMBLING_WIN;		
	extern std::string	strGAMBLING_LOSE;	

	extern std::string	strITEMDROP_SUIT;
	extern std::string	strITEMDROP_WAPON;
	extern std::string	strITEMDROP_SHOES;
	extern std::string	strITEMDROP_RING;
	extern std::string	strITEMDROP_QBOX;

	extern std::string	strITEMDROP_SCROLL;
	extern std::string	strITEMDROP_COIN;
	extern std::string	strITEMDROP_DRUGS;

	extern std::string	strPICKUP_ITEM;
	extern std::string	strQITEM_FACT;
	extern std::string	strQITEM_BGM[QUESTION_SIZE];

	//	Note : ----------------------------------------------------------------ȿ��
	extern std::string	strSELECT_CHAR;			//	�κ񿡼� �ɸ��� ���ý�.

	extern std::string	strREBIRTH_EFFECT;		//	ĳ�� ��Ȱ ����Ʈ.
	extern std::string	strLEVELUP_EFFECT;		//	���� �� ����Ʈ.
	
	extern std::string	strSRIKE_ELMT_EFFECT[EMELEMENT_MAXNUM]; //	Ÿ�ݽ� ����Ʈ.

	extern std::string	strAMBIENT_EFFECT;		//	Ÿ�ݽ� Ambient ����Ʈ
	extern std::string	strERASE_EFFECT;		//	�� ������� ����Ʈ.

	extern std::string	strSKILL_LEARN_EFFECT;	//	��ų �����.
	extern std::string	strSKILL_UP_EFFECT;		//	��ų ��.

	extern std::string strQUEST_START;			//	����Ʈ ����.
	extern std::string strQUEST_END_SUCCEED;	//	����Ʈ ����.
	extern std::string strQUEST_END_FAIL;		//	����Ʈ ����.

	extern std::string	strMOB_GEM_EFFECT;		//	�� ������ ����Ʈ.
	extern std::string	strMOB_DIE_BLOOD;		//	���� �׾�����.

	// PET
	extern std::string	strPET_GEN_EFFECT;		// �� ������ ����Ʈ

	// Vehicle
	extern std::string  strVEHICLE_GEN_EFFECT;	// ž�� ����Ʈ

	extern std::string	strCONFRONT_BOUND;		//	��� ��輱 ǥ�� ȿ��.

	extern std::string	strHALFALPHA_EFFECT;	//	���� �ɸ���.

	extern std::string	strBLOW_NUMB_EFFECT;
	extern std::string	strBLOW_STUN_EFFECT;
	extern std::string	strBLOW_STONE_EFFECT;
	extern std::string	strBLOW_BURN_EFFECT;
	extern std::string	strBLOW_FROZEN_EFFECT;

	extern std::string	strBLOW_MAD_EFFECT;
	extern std::string	strBLOW_POISON_EFFECT;
	extern std::string	strBLOW_CURSE_EFFECT;

	extern std::string	strBLOW_BODY_NUMB_EFFECT;
	extern std::string	strBLOW_BODY_STUN_EFFECT;
	extern std::string	strBLOW_BODY_STONE_EFFECT;
	extern std::string	strBLOW_BODY_BURN_EFFECT;
	extern std::string	strBLOW_BODY_FROZEN_EFFECT;

	extern std::string	strBLOW_BODY_MAD_EFFECT;
	extern std::string	strBLOW_BODY_POISON_EFFECT;
	extern std::string	strBLOW_BODY_CURSE_EFFECT;

	extern std::string strBLOW_EFFECTS[EMBLOW_SIZE];
	extern std::string strBLOW_BODY_EFFECTS[EMBLOW_SIZE];

	extern std::string strCRUSHING_BLOW_EFFECT;		// ����Ÿ�� ����Ʈ

	extern std::string	strPKCOMBO_DOUBLE;
	extern std::string	strPKCOMBO_TRIPLE;
	extern std::string	strPKCOMBO_ULTRA;
	extern std::string	strPKCOMBO_RAMPAGE;

	extern float fIllusionInterval;

	extern GLCONST_CHARCLASS	cCONSTCLASS[GLCI_NUM_8CLASS];	//	ĳ���� Ŭ������ ���.

	extern std::string strCLASS_INIT[MAX_SCHOOL][GLCI_NUM_8CLASS];

	extern std::vector<std::string>	vecGUIDANCE_FILE;
	extern std::vector<std::string>	vecClubDM_FILE;

	extern std::vector<GLWEAPON_LINKSKILL>	vecWeaponLinkSkill; //Weapon Skill - SqueeeAK

	extern std::string	strCLICK_EFFECT;
	extern std::string	strTARGET_EFFECT;
	extern std::string	strMINE_EFFECT;
	extern std::string	strCOMPLETE_NPC_EFFECT;

	/*combatpoint logic, Juver, 2017/05/27 */
	extern WORD		wCombatPoint_MAX; //max cp
	extern WORD		wCombatPoint_PK; //cp reward per pk

	/*vehicle booster system, Juver, 2017/08/11 */
	extern float	fVehicleBoosterChargeTime;
	extern float	fVehicleBoosterBoostTime;
	extern float	fVehicleBoosterAddSpeed;

	/*Money Luck, Juver, 2017/08/24 */
	extern float		fMONEY_LUCK_RATE;

	/*pvp tyranny, Juver, 2017/08/25 */
	extern std::string	strTyrannyFile;
	extern std::string	strSchoolEffect[TYRANNY_SCHOOL_SIZE];

	/*quest rate, Juver, 2017/09/27 */
	extern float		fQuestExpRate;
	extern float		fQuestMoneyRate;

	/*int-def logic, Juver, 2017/09/30 */
	extern float		fIntDefRate;

	/*character simple, Juver, 2017/10/01 */
	extern	std::string strSimpleWearM[SLOT_WEAR_SIMPLE];
	extern	std::string strSimpleWearF[SLOT_WEAR_SIMPLE];

	/*contribution/activity bonus, Juver, 2017/10/14 */
	extern BOOL		bContributionPointBonusTimer;
	extern float	fContributionPointBonusTime;
	extern WORD		wContributionPointBonusAdd;

	extern BOOL		bActivityPointBonusTimer;
	extern float	fActivityPointBonusTime;
	extern WORD		wActivityPointBonusAdd;

	/*activity system, Juver, 2017/10/30 */
	extern std::string strCOMPLETE_ACTIVITY_EFFECT;

	/*school wars, Juver, 2018/01/19 */
	extern std::string	strSchoolWarsFile;

	/*pvp capture the flag, Juver, 2018/01/23 */
	extern std::string	strCaptureTheFlagFile;
	extern std::string	strCaptureTheFlagEffect[CAPTURE_THE_FLAG_TEAM_SIZE];
	extern std::string	strCaptureTheFlagDirectionEffect[CAPTURE_THE_FLAG_TEAM_SIZE];

	//	Note : ĳ���� Ŭ������ �ʱ� ������.
	//
	extern char					szCharSkin[GLCI_NUM_8CLASS][MAX_PATH];	//	ĳ���� �ʱ� ����.
	
	//random tyranny reward buff
	extern SRANDOM_SYSTEMBUFF_SET	sRandomSystemBuff[SYSTEM_BUFF_SETTING_SIZE];

	BOOL	LOADFILE ( char* szFileName, BOOL bServer=TRUE );
	BOOL	EXP_LOADFILE ( char* szFileName );

	BOOL	EXP_PC_LOADFILE ( char* szFileName );
	BOOL	EXP_MAX_LOADFILE ( char* szFileName );
	BOOL	EXP_MAX_2nd_LOADFILE ( char* szFileName );

	//PANDORA
	BOOL LOADFILE_PANDORA ( char* szFileName );

	BOOL	LOAD_SKILLPREREQ ( char* szFileName ); //add Skill Prerequisites 
	BOOL 	LOAD_WEAPON_LINKSKILL( char* szFileName ); //Weapon Skill - SqueeeAK

	BOOL VAID_CHAR_DATA2 ( WORD wSchool, EMCHARINDEX emIndex );
	SCHARDATA2& GET_CHAR_DATA2 ( WORD wSchool, EMCHARINDEX emIndex );

	inline const char* GETSTRIKE_EFFECT ( EMELEMENT emELEMENT=EMELEMENT_SPIRIT )
	{
		GASSERT(emELEMENT>=0);
		GASSERT(emELEMENT<EMELEMENT_MAXNUM);

		return strSRIKE_ELMT_EFFECT[emELEMENT].c_str();
	}

	const char* GETSCHOOLNAME ( WORD wSCHOOL );
	void SETEXP_SCALE ( float fScale );
	float GETEXP_SCALE ();
	DWORD GETEXP ( int nAttackerLev, int nDefenserLev );
	float GETEXP_RATE ( int nAttackerLev, int nDefenserLev );

	DWORD GETEXP_PC ( WORD wLev );
};

struct PETSTYLE
{
	std::string	strSTYLE_CPS[MAX_HAIR];
	WORD		wSTYLE_COLOR[MAX_HAIR];
	WORD		wSTYLENum;


	PETSTYLE () :
	wSTYLENum(0)
	{
		memset( wSTYLE_COLOR,0,sizeof(wSTYLE_COLOR));
	}
};

namespace GLCONST_PET
{
	extern PGLPET		pGLPET[PET_TYPE_SIZE];
	extern char			szPetInit[PET_TYPE_SIZE][MAX_PATH];
	extern PETSTYLE		sPETSTYLE[PET_TYPE_SIZE];
	extern BOOL			bCHANGEABLECOLOR[PET_TYPE_SIZE];

	extern float		fRunArea;
	extern float		fWalkArea;
	extern float		fOwnerDistance;
	extern int			nMAXVIEWRANGE;
	extern int			nFullDecrement[PET_TYPE_SIZE];
	extern float		fMaginotLineHP;
	extern float		fMaginotLineMP;
	extern float		fMaginotLineSP;

	PGLPET   GetPetData ( PETTYPE emTYPE );
	PETSTYLE GetPetStyle ( PETTYPE emTYPE );
	BOOL     IsChangeablePetColor ( PETTYPE emTYPE );
};

namespace GLCONST_VEHICLE
{
	extern PGLVEHICLE		pGLVEHICLE[VEHICLE_TYPE_SIZE];
	extern char				szVehicleInit[VEHICLE_TYPE_SIZE][MAX_PATH];

	extern int				nFullDecrVehicle[VEHICLE_TYPE_SIZE]; // ������ ����ġ
	
	PGLVEHICLE		GetVehicleData ( VEHICLE_TYPE emTYPE );

};

namespace GLCONST_SUMMON
{
	extern PGLSUMMON		pGLSUMMON[SUMMON_TYPE_SIZE];
	extern char				szSummonInit[SUMMON_TYPE_SIZE][MAX_PATH];
	extern DWORD			m_dwMaxSummon[SUMMON_TYPE_SIZE];

	PGLSUMMON GetSummonData ( SUMMON_TYPE emTYPE );
	DWORD GetMaxSummon( SUMMON_TYPE emTYPE );

};

namespace GLCONST_ATTENDANCE
{
	const int MAX_REWARD = 10;

	struct	ATTEND_REWARD
	{
		int				nComboDay;
		SNATIVEID		idReward;

		ATTEND_REWARD()
			: nComboDay ( 0 )
			, idReward( NATIVEID_NULL() )
		{
		}
	};


	extern ATTEND_REWARD	sATEEND_REWARD[MAX_REWARD];
	extern DWORD			dwMAXREWARD;
	extern DWORD			dwAttendTime;

	BOOL LoadFile( std::string strFileName, bool bServer );
};

namespace MINIGAME_ODDEVEN // �̴� ����, Ȧ¦ ����
{
	const unsigned int MAX_ROUND = 5;

	extern float	fReturnRate[MAX_ROUND];		// ���� ���� �μ�(�������� = ����*1ȸ���μ�*2ȸ���μ�...)
	extern float	fSuccessRate[MAX_ROUND];	// ���� Ȱ��
	extern float	fTimeLimit;					// ���� �ð�(��)
	extern float	fShuffleTime;				// ���� �ð�(��)
	extern UINT		uiMaxBattingMoney;			// �ִ� ���� ���� �ݾ�
};

namespace HAIRCOLOR
{
	extern WORD wHairColor[GLCI_NUM_8CLASS][MAX_HAIR];

	extern WORD GetHairColor ( WORD wClass, WORD wStyle ); 
};


namespace MONEYCOLOR
{
	enum 
	{
		MAX_COLOR = 6,
	};

    extern DWORD dwTEXT_MONEY_COLOR_BEGIN;
    extern DWORD dwMONEY_COLOR_DEFAULT;
	extern DWORD dwMONEY_COLOR[MAX_COLOR];

    extern const DWORD GetMoneyColor (const LONGLONG lnMoney);
}

namespace ITEM_INFOR_COLOR
{
    extern DWORD dwDefault;					// �⺻ �÷�.
    extern DWORD dwItemRank[LEVEL_NSIZE];	// ���� ��޿� ���� �÷�.
    extern DWORD dwReqWrong; 				// ���� ���� �÷�.
    extern DWORD dwInchant1; 				// �⺻ ���ݷ� + ���� �ջ� �÷�.
    extern DWORD dwInchant2; 				// ���� ���� �÷�.
    extern DWORD dwEquipCostum;				// �ڽ�Ƭ �� ���� ���� ���� �÷�.
    extern DWORD dwRankUp1; 				// �⺻ �ɷ�ġ + ���� �ջ� �÷�.
    extern DWORD dwRankUp2; 				// ���� ���� �÷�.
    extern DWORD dwSPReduce;				// SP �߰� �Ҹ� �÷�.
    extern DWORD dwAddAbility;				// ���� ȿ�� ��� �÷�.
    extern DWORD dwRandomAbility;			// ���� �ɼ� ��� �÷�.
    extern DWORD dwElementalAbility;		// �����̻� ��� �÷�.
    extern DWORD dwTrade;					// �ŷ� �� ��ȯ �Ұ� ��� �÷�.
    extern DWORD dwLimitTime;				// ������ ��ȿ�Ⱓ ��� �÷�.
	extern DWORD dwAddDisguise;				// �ڽ�Ƭ ������ ���� ���� �ɼ� �÷�.
	extern DWORD dwMoney;					// ��� �ؽ�Ʈ �÷�.
	extern DWORD dwSetItem;					// ��Ʈ ������ �÷�.

	//////////////////////////////////////////////////////////////////////////

	// ������ ���� ����
	struct ITEM_GRIND
	{
		ITEM_GRIND() { RangeStart=0; RangeEnd = 0; GrindColor = 0; }
		static const int TOTAL_COUNT = 5;
		int RangeStart;
		int RangeEnd;
		DWORD GrindColor;
	};

	extern ITEM_GRIND dwItemGrind[ITEM_GRIND::TOTAL_COUNT];
}

namespace SKILL_INFOR_COLOR
{
    extern DWORD dwDefault; // �⺻ �÷�.
    extern DWORD dwSkillName; // ��ų �̸� ��� �÷�.
    extern DWORD dwSkillGrade; // ��ų ��� ��� �÷�.
    extern DWORD dwSkillType; // ��ų Ÿ�� ��� �÷�.(�Ϲ���, �нú��� ���)
    extern DWORD dwSkillElemental; // ��ų �Ӽ� ��� �÷�.
    extern DWORD dwReqWrong; // ��ų ���� ���� ���� ��� �÷�.
    extern DWORD dwSkillRange; // ��ų ���� ���� ��� �÷�.
    extern DWORD dwAddAbility; // ��ų ȿ�� ���� ��� �÷�.
    extern DWORD dwElementalAbility; // ��ų �����̻� ��� �÷�.
}


class GLogicData
{
public:
	HRESULT LoadData ( BOOL bServer, bool bPastLoad = FALSE );
	HRESULT ClearData ();

	HRESULT InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );
	HRESULT DeleteDeviceObjects ();

protected:
	GLogicData(void);

public:
	~GLogicData(void);

public:
	static GLogicData& GetInstance();
};
