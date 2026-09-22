#ifndef GLCROWDATA_H_
#define GLCROWDATA_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <string>
#include "./GLCharData.h"
#include "./GLCharDefine.h"
#include "./GLInventory.h"
#include "../NpcTalk/NpcDialogueSet.h"

#include "../../Lib_Engine/G-Logic/GLDefine.h"
#include "../../Lib_Engine/Meshs/SAnimation.h"
#include "../../Lib_Engine/NaviMesh/NavigationPath.h"
 
#include "./GLCrowDataBasic.h"
#include "./GLCrowDataAction.h"
#include "./GLCrowDataGen.h"
#include "./GLCrowDataAttack.h"

/*npc shop, Juver, 2017/07/25 */
#include "./GLCrowDataNPCShop.h" 

/*pvp tyranny, Juver, 2017/08/25 */
#include "./GLPVPTyrannyDefine.h" 

/*item exchange, Juver, 2017/10/11 */
#include "./GLCrowDataItemExchange.h"

//	Note : 다수 아이템 발생시 처리 구조.
//
struct SGENITEM
{
	enum
	{
		EMGNUM		= 5,
		EMGSPEC		= 30,
		EMSPEC_NULL	= -1
	};

	struct SGNUM	//	발생 갯수별 확율.
	{
		WORD	wNum;
		float	fRateL;
		float	fRateH;

		SGNUM () :
			wNum(0),
			fRateL(100.0f),
			fRateH(100.0f)
		{
		}
	};

	struct SGITEMSPEC	//	발생 아이템군별 확율.
	{
		DWORD	dwSPECID;
		float	fRateL;
		float	fRateH;
		bool	bSPECID;

		SGITEMSPEC () :
			dwSPECID(EMSPEC_NULL),
			fRateL(100.0f),
			fRateH(100.0f),
			bSPECID( false )
		{
		}
	};

	float		m_fGenRate;				//	발생 확율.
	SGNUM		m_sGenNum[EMGNUM];		//	발생 갯수별 확율 분포.
	SGITEMSPEC	m_sItemSpec[EMGSPEC];	//	발생 아이템 군.

	SGENITEM() :
		m_fGenRate(0)
	{
	}

	BOOL LOADFILE ( const char* szFileName );

	BOOL ISGEN ( float fGenRate = 1.0f );							//	발생 확율.
	WORD SELECTNUM ();						//	발생 갯수.
	DWORD SELECTSPECID ( float fGEN_RATE, bool& bSPECID );	//	발생 군.
};

//	Note : Question 아이템 발생시 처리 구조 by 경대
//
struct SQTGENITEM
{
	enum { EMQTMAX = 20 };

	struct ITEM
	{
		SNATIVEID	m_sGenItemID;
		float		m_fGenRate;

		ITEM() : m_sGenItemID(false), m_fGenRate(0)
		{
		}
	};

	ITEM	m_sItem[EMQTMAX];
	int		m_nItemNum;

	SQTGENITEM() : m_nItemNum(0)
	{
	}

	BOOL LOADFILE ( const char * szFileName );

	SNATIVEID SELECTITEM(); // 아이템 발생
};

typedef std::map<DWORD,LONGLONG>	NPCSELL_PRICE_MAP;
typedef NPCSELL_PRICE_MAP::iterator NPCSELL_PRICE_MAP_ITER;

typedef std::map<DWORD,DWORD>	MOB_DROP_SPECID_MAP;
typedef MOB_DROP_SPECID_MAP::iterator MOB_DROP_SPECID_MAP_ITER;

struct SCROWDATA
{
	enum
	{
		VERSION = 0x0101,
		EMMAXATTACK = SCROWACTION::ACTION_MAXATTACK,

		FILE_SBASIC			= 1,
		FILE_SACTION		= 2,
		FILE_SGEN			= 3,
		FILE_SATTACK		= 4,
		FILE_END_DATA		= 0xEDEDEDED,
	};

	union
	{
		struct
		{
			SCROWBASIC			m_sBasic;
		};

		//changes to SCROWBASIC structure or variable type/arrangement must be also applied here
		struct
		{
			SNATIVEID			sNativeID;					//	고유 ID.
			EMCROW				m_emCrow;					//	Npc, Mob.
			EMTRIBE				m_emTribe;					//	종족.
			EMBRIGHT			m_emBright;					//	속성 (광/암)
			char				m_szName[CHAR_SZNAME];		//	이름.
			WORD				m_wLevel;					//	레벨.
			DWORD				m_dwHP;						//	HP
			WORD				m_wMP;						//	MP
			WORD				m_wSP;						//	SP
			float				m_fIncHP;					//	HP 회복율.
			float				m_fIncMP;					//	MP 회복율.
			float				m_fIncSP;					//	SP 회복율.
			WORD				m_wDefense;					//	방어력.
			WORD				m_wViewRange;				//	시야 거리.
			WORD				m_wAvoidRate;				//	회피율.
			WORD				m_wHitRate;					//	명중율.
			DWORD				m_dwBonusExp;				//	추가경험치.
			WORD				m_wBonusCP;
			SRESIST				m_sResist;					//	저항값.
			bool				m_bOverlapAttack;			//  겹쳤을때 공격 가능 여부
			DWORD				m_dwGenTime;
			WORD				m_wGatherTimeLow;			// 채집시간(Low)
			WORD				m_wGatherTimeHigh;			// 채집시간(High)
			float				m_fGatherRate;				// 채집 확률
			WORD				m_wGatherAnimation;			// 애니메이션
			PETTYPE				m_emPetType;
			SUMMON_TYPE			m_emSummonType;
			EMNPC_TYPE			m_emNPCType;
			bool				m_bHideName;				/*hide crow name, Juver, 2017/09/01 */
			DWORD				m_dwNotice;					/*crow notice, Juver, 2017/11/25 */
			DWORD				dummy_var_0;
			bool				dummy_var_1;
			bool				dummy_var_2;
			EMNPC_CTF_FLAG_TYPE	m_emCTFType;				/*pvp capture the flag, Juver, 2018/02/03 */
		};
	};

	SCROWACTION				m_sAction;
	SCROWGEN				m_sGenerate;

	//	npc talk 정보.
	CNpcDialogueSet			m_sNpcTalkDlg;

	//	공격, Skill 설정.
	SCROWATTACK				m_sCrowAttack[EMMAXATTACK];

	//	생성 아이템군 설정 [server 에서만 필요한 정보]
	SGENITEM				m_sGenItem[10];
	SQTGENITEM				m_sQtGenItem; // Question Item 생성 by 경대

	//	판매 목록.
	std::string				m_strSaleType[SCROWACTION::SALENUM];
	GLInventory				m_sSaleItems[SCROWACTION::SALENUM];
	//  판매 NPC에게 입력된 가격
	NPCSELL_PRICE_MAP		m_mapNpcSellPrice;
	MOB_DROP_SPECID_MAP 	m_mapMobDropSpecID;			// [MobDB][Construction] by edwin 20190804 ***** [Edition 1] 

	//	에니메이션 정보. ( 시간/스탭 ) [server 에서만 필요한 정보]
	//	
	VECANIATTACK*			m_pANIMATION;

	VECANIATTACK&			GetAnimation ( EMANI_MAINTYPE emMType, EMANI_SUBTYPE emSType );

	/*npc shop, Juver, 2017/07/25 */
	SNPC_SHOP				m_sNPCShop;

	/*item exchange, Juver, 2017/10/11 */
	SNPC_ITEM_EXCHANGE		m_sNPCItemExchange;

	BOOL IsCdCertify ()		{ return m_sAction.m_dwActFlag&EMCROWACT_CDCERTIFY; }
	BOOL IsBusUnit ()		{ return m_sAction.m_dwActFlag&EMCROWACT_BUSUNIT; }
	BOOL IsTarShort ()		{ return m_sAction.m_dwActFlag&EMCROWACT_TARSHORT; }
	BOOL IsIgnoreShock()	{ return m_sAction.m_dwActFlag&EMCROWACT_IGNORE_SHOCK; }

	BOOL IsInVisible ()		{ return m_sAction.m_dwActFlag&EMCROWACT_INVISIBLE; }
	BOOL IsRecVisible ()	{ return m_sAction.m_dwActFlag&EMCROWACT_RECVISIBLE; }

	BOOL IsBarrier ()		{ return m_sAction.m_dwActFlag&EMCROWACT_BARRIER; }
	BOOL IsPosHold ()		{ return m_sAction.m_dwActFlag&EMCROWACT_POSHOLD; }
	BOOL IsDirHold ()		{ return m_sAction.m_dwActFlag&EMCROWACT_DIRHOLD; }

	DWORD GetSaleNum () { return SCROWACTION::SALENUM; }
	GLInventory* GetSaleInven ( DWORD dwIndex )
	{
		if ( GetSaleNum() <= dwIndex )	return NULL;
		return &m_sSaleItems[dwIndex];
	}
	std::string GetSaleType ( DWORD dwIndex )
	{
		if ( GetSaleNum() <= dwIndex )	return "";
		return m_strSaleType[dwIndex];
	}
	LONGLONG GetNpcSellPrice ( DWORD dwID )
	{
		NPCSELL_PRICE_MAP_ITER it = m_mapNpcSellPrice.find(dwID);
		if( it == m_mapNpcSellPrice.end() )
		{
			return 0;
		}
		return it->second;
	}

	const char* GetName();
	const char* GetTalkFile ()		{ return m_sAction.m_strTalkFile.c_str(); }
	const char* GetSkinObjFile ()	{ return m_sAction.m_strSkinObj.c_str(); }

	void LoadAniSet ( const char* szSkinObj );
	SCROWDATA& Assign ( SCROWDATA &Data );

	HRESULT LoadFile ( basestream &SFile, BOOL bServer, bool bPastLoad );
	HRESULT SaveFile ( CSerialFile &SFile );

	static VOID SaveCsvHead ( std::fstream &SFile );
	VOID SaveCsv ( std::fstream &SFile );
	VOID LoadCsv ( CStringArray &StrArray );

	SCROWDATA () :
		m_pANIMATION(NULL)
	{
		for ( int i=0; i<SCROWACTION::SALENUM; ++i )
		{
			m_sSaleItems[i].SetState ( SALE_INVEN_X, SALE_INVEN_Y );
			m_mapNpcSellPrice.clear();
			m_mapMobDropSpecID.clear();
		}
	}

	~SCROWDATA ()
	{
		SAFE_DELETE_ARRAY(m_pANIMATION);
	}
};
typedef SCROWDATA*		PCROWDATA;

BOOL SaleInvenLoadFile ( const char* szFileName, GLInventory &_sINVENTORY, std::string &_strSaleType, NPCSELL_PRICE_MAP &_mapNpcSellPrice );

class GLCrowDataMan
{
public:
	enum
	{
		ENCODE_VER_OLD	= 0x0102,
		ENCODE_VER		= 0x0200,
		VERSION			= 0x0200,
		MAX_CROW_MID	= 1024,
		MAX_CROW_SID	= 1024
	};

public:
	static const char*	_FILEHEAD;
	static const char*	_LOGFILE;
	static const char*	_STRINGTABLE;

protected:
	char				m_szFileName[MAX_PATH];

	PCROWDATA**			m_ppPCROWDATA;
	bool				m_bModify;

	LPDIRECT3DDEVICEQ	m_pd3dDevice;

public:
	PCROWDATA GetMainCrowData ( WORD wMainID );

	PCROWDATA GetCrowData ( WORD wMID, WORD wSID );
	PCROWDATA GetCrowData ( SNATIVEID sNativeID );

	bool	ValidData ();
	bool	IsModify ()								{ return m_bModify; }

public:
	WORD FindFreeCrowDataMID ();
	WORD FindFreeCrowDataSID ( WORD wMainID );

public:
	BOOL InsertCrowData ( WORD wMID, WORD wSID, PCROWDATA pCrowData, bool binner=false );
	BOOL InsertCrowData ( SNATIVEID sNativeID, PCROWDATA pCrowData, bool binner=false )
	{
		return InsertCrowData ( sNativeID.wMainID, sNativeID.wSubID, pCrowData, binner );
	}

public:
	BOOL DeleteCrowData ( WORD wMID, WORD wSID );
	BOOL DeleteCrowData ( SNATIVEID sNativeID );

public:
	HRESULT OneTimeSceneInit ();
	HRESULT FinalCleanup ();

public:
	HRESULT	SyncStringTable();
	HRESULT SyncUpdateData ();
	HRESULT SaveCsvFile( CWnd* pWnd );
	HRESULT LoadCsvFile( CWnd* pWnd );

public:
	HRESULT LoadFile ( const char* szFile, BOOL bServer, bool bPastLoad );
	HRESULT SaveFile ( const char* szFile );

public:
	GLCrowDataMan ();

public:
	~GLCrowDataMan ();

public:
	static GLCrowDataMan& GetInstance();
};

struct SDROP_CROW
{
	enum { CROW_GEM=0x0001, };

	SNATIVEID			sNativeID;	//	Crow 고유 ID.

	SNATIVEID			sMapID;		//	맵 ID.
	DWORD				dwCeID;		//	셀 ID.
	DWORD				dwGlobID;	//	생성 메모리 인덱스용.
	D3DXVECTOR3			vPos;		//	위치.
	D3DXVECTOR3			vDir;		//	방향.
	FLOAT				fScale;		//  크기.

	// 서버 클라이언트간 몹의 위치 동기화를 위해
	D3DXVECTOR3			vStartPos;  // 이동시 시작위치. 추가된거
	NavigationPath::WAYPOINT sNextWaypoint; // 다음 웨이포인트. 추가된거

	DWORD				dwNowHP;	//	생명량.
	WORD				wNowMP;		//	정신량.

	DWORD				dwFLAGS;	//	기타 속성.
	EMACTIONTYPE		emAction;	//	행동.
	DWORD				dwActState;	//	행동 상태.

	STARGETID			TargetID;	//	목표 유닛, 위치.

	SDROP_SKILLFACT		sSKILLFACT[SKILLFACT_SIZE];
	SDROP_STATEBLOW		sSTATEBLOWS[EMBLOW_MULTI];

	/*pvp tyranny, Juver, 2017/08/25 */
	WORD				m_wTyrannyOwner;
	float				m_fTyrannyDamage[TYRANNY_SCHOOL_SIZE];

	/*pvp capture the flag, Juver, 2018/02/06 */
	bool				m_bCaptureTheFlagCrowHide;

	SDROP_CROW () 
		: sMapID(NATIVEID_NULL())
		, dwCeID(0)
		, dwGlobID(0)
		, vPos(0,0,0)
		, vDir(1,0,0)
		, dwNowHP(0)
		, wNowMP(0)
		, dwFLAGS(NULL)
		, emAction(GLAT_IDLE)
		, dwActState(0)
		, m_wTyrannyOwner(TYRANNY_SCHOOL_SIZE)	/*pvp tyranny, Juver, 2017/08/25 */
		, m_bCaptureTheFlagCrowHide(false)		/*pvp capture the flag, Juver, 2018/02/06 */
	{
		/*pvp tyranny, Juver, 2017/08/25 */
		for ( int i=0; i<TYRANNY_SCHOOL_SIZE; ++ i )
			m_fTyrannyDamage[i] = 0.0f;
	}
};

struct SDROP_MATERIAL
{
	enum { CROW_GEM=0x0001, };

	SNATIVEID			sNativeID;	//	Crow 고유 ID.

	SNATIVEID			sMapID;		//	맵 ID.
	DWORD				dwCeID;		//	셀 ID.
	DWORD				dwGlobID;	//	생성 메모리 인덱스용.
	D3DXVECTOR3			vPos;		//	위치.
	D3DXVECTOR3			vDir;		//	방향.

	DWORD				dwFLAGS;	//	기타 속성.
	EMACTIONTYPE		emAction;	//	행동.
	DWORD				dwActState;	//	행동 상태.

	SDROP_MATERIAL () 
		: sMapID(NATIVEID_NULL())
		, dwCeID(0)
		, dwGlobID(0)
		, vPos(0,0,0)
		, vDir(1,0,0)
		, dwFLAGS(NULL)
		, emAction(GLAT_IDLE)
		, dwActState(0)
	{
	}
};


#endif // GLCROWDATA_H_

