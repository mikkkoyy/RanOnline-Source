#include "stdafx.h"
#include "./GLChar.h"
#include "./GLCharData.h"
#include "./GLGaeaServer.h"
#include "./GLSchoolFreePK.h"
#include "GLClubDeathMatch.h"

 /*pvp tyranny, Juver, 2017/08/24 */
#include "GLPVPTyrannyField.h"

/*skill summon, Juver, 2017/10/09 */
#include "GLSummonData.h"

/*school wars, Juver, 2018/01/19 */
#include "GLPVPSchoolWarsField.h"

/*pvp capture the flag, Juver, 2018/01/31 */
#include "GLPVPCaptureTheFlagField.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BOX_R	(4)

enum { IDX_HP = 0, IDX_MP, IDX_SP,NUM_HP_MP_SP };

GLChar::GLChar () :
	m_wTARNUM(0),

	m_dwANISUBSELECT(0),

	m_dwActState(NULL),
	m_Action(GLAT_IDLE),

	m_fIdleTime(0.0f),
	m_fMBR_TIMER(0.0f),
	m_fSTATE_TIMER(0.0f),
	m_fPERIOD_TIMER(0.0f),
	m_fQUEST_TIMER(60.0f),
	m_fITEM_TIMER(0.0f),
	m_fSAVEDB_TIMER(0.0f),
	m_fattTIMER(0.0f),
	m_fMoveDelay(0.0f),
	m_fBRIGHT_TIMER(0.0f),

	m_sAssault(CROW_PC,GAEAID_NULL),
	m_lnLastSendExp(0),

	m_lnLastSendVNExp(0),

	m_dwViewFrame(0),
	m_fAge(0.0f),
	m_fGenAge(0.0f),
	m_nDECEXP(0),
	m_vPos(0,0,0),
	
	m_dwClientID(0),
	m_dwGaeaID(0),
	m_dwPartyID(PARTY_NULL),

	m_dwCeID(0),

	m_wHealingNum(0),

	m_pLandMan(NULL),
	
	m_pGaeaNode(NULL),
	m_pLandNode(NULL),

	m_pQuadNode(NULL),
	m_pCellNode(NULL),

	m_SKILLMTYPE(AN_SKILL_A),
	m_SKILLSTYPE(AN_SUB_00),

	m_bUsedStorageCard(false),

	m_bNon_Rebirth(false),

	/*charinfoview , Juver, 2017/11/12 */
	m_bPrivateStats(false),

	m_bDisTrade(FALSE),

	m_fSkillDelay(0.0f),

	m_dwPetGUID(UINT_MAX),
	m_dwPetID(0),

	//m_dwSummonGUID(UINT_MAX),
	//m_dwSummonID(0),

	m_bEntryFailed(FALSE),


	m_bProtectPutOnItem(false),
	m_bGetPetFromDB(false),
	m_bGetVehicleFromDB ( false ),
	m_fVehicleTimer( 0.0f ),

	m_ItemRemodelPosX(EM_INVENSIZE_X),
	m_ItemRemodelPosY(EM_INVENSIZE_Y),
	m_ItemGarbagePosX(EM_INVENSIZE_X),
	m_ItemGarbagePosY(EM_INVENSIZE_Y),

	m_ChinaGainType(255),
	m_dwVietnamGainType(GAINTYPE_MAX),

	m_bEventStart(FALSE),
	m_bEventApply(FALSE),
	m_hCheckStrDLL( NULL ),
	m_pCheckString( NULL ),

	m_bUseRebuildCard( false ),
	m_sidLastMapCheck(false),
	m_pGLGaeaServer( NULL ),
	m_bItemShopOpen ( false ),
	m_bDefenseSkill ( false ),
	m_bDeathTimer( FALSE ),
	m_bHoverConfirm ( FALSE ),
	m_fDeathTimer( 0.0f ),
	m_fContinuousDamageCheck(0.0f), /*continuous damage skill logic, Juver, 2017/06/10 */
	m_fSummonCheckTime(0.0f),		/*skill summon, Juver, 2017/10/10 */

	/*contribution/activity bonus, Juver, 2017/10/14 */
	m_bUseContributionPointTimer(FALSE),
	m_bUseActivityPointTimer(FALSE),
	m_fContributionPointBonusTimer(0.0f),
	m_fActivityPointBonusTimer(0.0f),

	m_bInstanceDropOut(FALSE),
	m_fInstanceDropTimer(0.0f),

	/*product item, Juver, 2017/10/18 */
	m_bItemCompoundTask(FALSE),
	m_dwItemCompoundTaskID(UINT_MAX),
	m_fItemCompoundTaskTime(0.0f),
	m_fItemCompoundTaskTimer(0.0f),
	m_nProductMake(1),
	m_nProductCounter(1),

	/*activity system, Juver, 2017/11/02 */
	m_sActivityCheckMapID(NATIVEID_NULL()),

	/*charinfoview , Juver, 2017/11/11 */
	m_fReqCharInfoTimer( 0.0f ),

	/*pet skill check, Juver, 2017/12/01 */
	m_fPetSkillCheckTimer(0.0f),
	m_sFreezeCheckHP(GLDWDATA()),
	m_wFreezeCount(0),
	m_wObviousCount(0),
	/*clubwar verify non push or pull, Juver, 2017/12/13 */
	m_bClubWarVerify(FALSE),
	m_fClubWarVerifyTimer(0.0f),

	/*equipment lock, Juver, 2018/01/16 */
	m_fEquipmentLockTimer(0.0f),

	/*storage lock, Juver, 2018/01/16 */
	m_fStorageLockTimer(0.0f),

	/*inventory lock, Juver, 2018/01/16 */
	m_fInventoryLockTimer(0.0f),

	m_fItemRebuildTimer(0.0f),

	m_fInvenSortTimer(0.0f),

	/* tyranny anti-afk, SeiferXIII 2020/05/17 */
	m_fAntiSafeZoneAFK(0.0f),
	m_fTWAntiAFK(0.0f),
	m_fDeadAFK(0.0f),

	/*pvp capture the flag, Juver, 2018/02/06 */
	m_fCaptureTheFlagDetectGateTimer(0.0f),
	m_fCaptureTheFlagHoldFlagTimer(0.0f),
	/*hackshield implementation, Juver, 2018/06/21 */
	hs_heartbeat_timer(0.0f),
	hs_disconnect_num(0),
	hs_heartbeat_num(0),
	m_bSpam(FALSE),

	/*dmk14 chat limit*/
	m_bChatProc(false),
	m_fChatTimerProc(0.0f),
	m_wChatCount(0),

	m_bGMCamera ( false ),
	m_sItemCustomRebuild(SITEMCUSTOM())
{


	m_sSaveMapID = NATIVEID_NULL();
	m_ARoundSlot.SetChar ( this );


	m_China15Time = CTime::GetCurrentTime();
	DWORD nMaxClient = GLGaeaServer::GetInstance().GetMaxClient();

	m_sFreezeCheckHP = GLDWDATA();
	m_wFreezeCount = 0;
	m_wObviousCount = 0;

	GASSERT(nMaxClient);
	m_arrayFieldPC = new PSFIELDCROW[nMaxClient];

	memset ( m_arrayFieldPC, 0, sizeof(PSFIELDCROW)*nMaxClient );
	memset ( m_arrayFieldCROW, 0, sizeof(PSFIELDCROW)*MAXCROW );
	memset ( m_arrayFieldMATERIAL, 0, sizeof(PSFIELDCROW)*MAXCROW );
	memset ( m_arrayFieldITEM, 0, sizeof(PSFIELDCROW)*MAXITEM );
	memset ( m_arrayFieldMONEY, 0, sizeof(PSFIELDCROW)*MAXMONEY );

	m_setLandEffectNum.clear();


	// PET
	m_arrayFieldPET = new PSFIELDCROW[nMaxClient];
	memset ( m_arrayFieldPET, 0, sizeof(PSFIELDCROW)*nMaxClient );

	m_arrayFieldSummon = new PSFIELDCROW[nMaxClient];
	memset ( m_arrayFieldSummon, 0, sizeof(PSFIELDCROW)*nMaxClient );

	MGOddEvenInit();

#ifdef TH_PARAM
	HMODULE m_hCheckStrDLL = LoadLibrary("ThaiCheck.dll");

	if ( m_hCheckStrDLL )
	{
		m_pCheckString = ( BOOL (_stdcall*)(CString)) GetProcAddress(m_hCheckStrDLL, "IsCompleteThaiChar");
	}
#endif

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
	{
		m_dwSummonGUID_FLD[i] = GAEAID_NULL;
	}

	/*dmk14 reform*/
	m_sItemRemodel = NATIVEID_NULL();
	m_sItemSealed = NATIVEID_NULL();
	m_bIsReformProcess = false;
	m_fReformProcessTimer = 0.0f;
}

GLChar::~GLChar ()
{
	delete[] m_arrayFieldPC;

	// PET
	delete[] m_arrayFieldPET;


	// Summon
	delete[] m_arrayFieldSummon;

#ifdef TH_PARAM
	if( m_hCheckStrDLL ) FreeLibrary( m_hCheckStrDLL );
#endif
}

DWORD GLChar::GetBonusExp () const
{
	return GLCONST_CHAR::GETEXP_PC(m_wLevel);
}

void GLChar::RESET_DATA ()
{
	int i = 0;

	GLCHARLOGIC::RESET_DATA();

	m_actorMove.ResetMovedData();
	m_actorMove.Stop();
	m_actorMove.Release();

	m_vDir = D3DXVECTOR3(1,0,0);

	m_TargetID = STARGETID(CROW_PC,GAEAID_NULL);
	m_sNPCID   = STARGETID(CROW_PC,GAEAID_NULL);
	m_wTARNUM = 0;

	for ( i=0; i<EMTARGET_NET; ++i )
		m_sTARIDS[i] = STARID(CROW_PC,GAEAID_NULL);

	m_dwANISUBSELECT = 0;

	m_Action = GLAT_IDLE;
	m_dwActState = NULL;
	m_fIdleTime = 0.0f;
	m_fMBR_TIMER = 0.0f;
	m_fSTATE_TIMER = 0.0f;
	m_fPERIOD_TIMER = 0.0f;
	m_fQUEST_TIMER = 60.0f;	// ó�� �ѹ� ������ ����ǰ� �ϱ� ���Ͽ�

	m_fattTIMER = 0.0f;
	m_fBRIGHT_TIMER = 0.0f;
	m_bHoverConfirm = FALSE;

	m_fMoveDelay = 0.0f;
	m_sAssault = STARGETID(CROW_PC,GAEAID_NULL);

	m_ARoundSlot.Reset();

	m_cDamageLog.clear();

	m_dwThaiCCafeClass	= 0;
	// �����̽þ� PC�� �̺�Ʈ
	m_nMyCCafeClass		= 0;

	m_bVietnamLevelUp = false;

	m_ChinaGainType    = 255;

	m_bTracingUser     = FALSE;

	m_dwVietnamGainType = GAINTYPE_MAX;

	m_bEventStart	   = FALSE;

	m_bEventApply	   = FALSE;

	m_sChinaTime.Init();
	m_sEventTime.Init();
	m_sVietnamSystem.Init();

	

	m_fAge = 0.0f;
	m_fGenAge = 0.0f;
	m_vPos = D3DXVECTOR3(0,0,0);

	m_dwClientID = 0;
	m_dwGaeaID = GAEAID_NULL;
	m_dwPartyID = PARTY_NULL;

	m_sMapID    = SNATIVEID(false);
	m_sOldMapID = SNATIVEID(false);
	m_dwCeID = 0;
	
	m_pLandMan = NULL;

	m_pGaeaNode = NULL;
	m_pLandNode = NULL;

	m_pQuadNode = NULL;
	m_pCellNode = NULL;

	m_sTrade.Reset();
	m_sPMarket.DoMarketClose();
	m_sCONFTING.RESET();

	m_sHITARRAY.clear();
	m_SKILLMTYPE = AN_SKILL_A;
	m_SKILLSTYPE = AN_SUB_00;

	m_bUsedStorageCard = false;

	m_bNon_Rebirth     = false;

	/*charinfoview , Juver, 2017/11/12 */
	m_bPrivateStats = false;

	m_bDisTrade = FALSE;

	m_bGetPetFromDB    = false;

	m_dwPetGUID = UINT_MAX;
	m_dwPetID	= 0;

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
	{
		m_dwSummonGUID_FLD[i] = GAEAID_NULL;
	}

	m_bEntryFailed = FALSE;

	m_setLandEffectNum.clear();

	m_bProtectPutOnItem = false;

	m_China15Time = CTime::GetCurrentTime();

	MGOddEvenInit();

	m_sVehicle.RESET();
	m_bGetVehicleFromDB = false;
	m_fVehicleTimer = 0.0f;

	m_bItemShopOpen = false;
	m_bDefenseSkill = false;

	DWORD nMaxClient = GLGaeaServer::GetInstance().GetMaxClient();


	memset ( m_arrayFieldPC, 0, sizeof(PSFIELDCROW)*nMaxClient );
	memset ( m_arrayFieldCROW, 0, sizeof(PSFIELDCROW)*MAXCROW );
	memset ( m_arrayFieldMATERIAL, 0, sizeof(PSFIELDCROW)*MAXCROW );
	memset ( m_arrayFieldITEM, 0, sizeof(PSFIELDCROW)*MAXITEM );
	memset ( m_arrayFieldMONEY, 0, sizeof(PSFIELDCROW)*MAXMONEY );

	memset ( m_arrayFieldPET, 0, sizeof(PSFIELDCROW)*nMaxClient );
	memset ( m_arrayFieldSummon, 0, sizeof(PSFIELDCROW)*nMaxClient );

	m_bSpam			= FALSE;
	
	/*skill summon, Juver, 2017/10/10 */
	m_fSummonCheckTime = 0.0f; 

	m_bInstanceDropOut = FALSE;
	m_fInstanceDropTimer = 0.0f;

	/*product item, Juver, 2017/10/18 */
	m_bItemCompoundTask = FALSE;
	m_dwItemCompoundTaskID = UINT_MAX;
	m_fItemCompoundTaskTime = 0.0f;
	m_fItemCompoundTaskTimer = 0.0f;
	m_nProductMake = 1;
	m_nProductCounter = 1;

	/*activity system, Juver, 2017/11/02 */
	m_sActivityCheckMapID = NATIVEID_NULL();

	/*charinfoview , Juver, 2017/11/11 */
	m_fReqCharInfoTimer = 0.0f;

	/*clubwar verify non push or pull, Juver, 2017/12/13 */
	m_bClubWarVerify = FALSE;
	m_fClubWarVerifyTimer = 0.0f;

	/*equipment lock, Juver, 2018/01/16 */
	m_fEquipmentLockTimer = 0.0f;

	m_fInvenSortTimer = 0.0f;

	/* tyranny anti-afk, SeiferXIII 2020/05/17 */
	m_fAntiSafeZoneAFK = 0.0f;
	m_fDeadAFK = 0.0f;
	m_fTWAntiAFK = 0.0f;

	/*pvp capture the flag, Juver, 2018/02/06 */
	m_fCaptureTheFlagDetectGateTimer = 0.0f;
	m_fCaptureTheFlagHoldFlagTimer = 0.0f;

	/*hackshield implementation, Juver, 2018/06/21 */
	hs_heartbeat_timer = 0.0f;
	hs_disconnect_num = 0;
	hs_heartbeat_num = 0;

	m_bGMCamera = false;
}

void GLChar::SendToClient(NET_MSG_GENERIC* pMsg)
{
    if (m_pGLGaeaServer)
        m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID, pMsg);
}

void GLChar::SendToClient(const DWORD dwClientSlot, NET_MSG_GENERIC* pMsg)
{
	if (m_pGLGaeaServer)
		m_pGLGaeaServer->SENDTOCLIENT(dwClientSlot, pMsg);
}

HRESULT GLChar::CreateChar( GLLandMan* pLandMan, D3DXVECTOR3 &vPos, PCHARDATA2 pCharData, LPDIRECT3DDEVICEQ pd3dDevice, BOOL bNEW )
{
	HRESULT hr;
	//*/*/*/*
	m_vPos = vPos;

	

	m_dwThaiCCafeClass = pCharData->m_dwThaiCCafeClass;
	// �����̽þ� PC�� �̺�Ʈ
	m_nMyCCafeClass = pCharData->m_nMyCCafeClass;

	m_sChinaTime	   = pCharData->m_sChinaTime;
	m_sEventTime	   = pCharData->m_sEventTime;
	m_sVietnamSystem   = pCharData->m_sVietnamSystem;

	m_bTracingUser	   = pCharData->m_bTracingUser;

//	m_sEventState	   = m_pGLGaeaServer->m_sEventState;

	m_bEventStart = FALSE;
	m_bEventApply = FALSE;
	if( m_pGLGaeaServer->m_sEventState.bEventStart == TRUE )
	{
		// �̺�Ʈ�� ���� ���� ��
		CTime	  crtTime     = CTime::GetCurrentTime();
		CTimeSpan crtGameSpan;
		CTime	  loginTime			 = m_sEventTime.loginTime;
		crtGameSpan					 = ( crtTime - loginTime );
		m_sEventTime.currentGameTime = crtGameSpan.GetTimeSpan();

		if( crtGameSpan.GetTotalSeconds() >= m_pGLGaeaServer->m_sEventState.EventPlayTime )
		{
			m_bEventStart = TRUE;
		}

		if( !(m_pGLGaeaServer->m_sEventState.MinEventLevel > pCharData->m_wLevel || 
			  m_pGLGaeaServer->m_sEventState.MaxEventLevel < pCharData->m_wLevel ) )
		{
			m_bEventApply = TRUE;
		}
	}


	hr = SetData ( pCharData, bNEW );
	if ( FAILED(hr) )	return E_FAIL;

	m_bEntryFailed = FALSE;
	// �ּ�����
	// �������� �˻�
	if ( m_dwUserLvl < USER_GM3 )
	{
		if( pLandMan->GetMapID() != NATIVEID_NULL() )
		{
			const SLEVEL_REQUIRE* pLEVEL_REQUIRE = GLGaeaServer::GetInstance().GetLevelRequire(pLandMan->GetMapID().dwID);
			EMREQFAIL emReqFail(EMREQUIRE_COMPLETE);
			if( pLEVEL_REQUIRE ) 
			{
				emReqFail = pLEVEL_REQUIRE->ISCOMPLETE ( this );
			}else{
				TEXTCONSOLEMSG_WRITE( "[ERROR]Level Data Load failed! charID[%s] mapID[%d][%d] EMFail[%d]", m_szName, pLandMan->GetMapID().wMainID, pLandMan->GetMapID().wSubID, emReqFail );
			}
			if ( emReqFail != EMREQUIRE_COMPLETE )
			{
				TEXTCONSOLEMSG_WRITE( "[INFO]Entry failed! charID[%s] mapID[%d][%d] EMFail[%d]", m_szName, pLandMan->GetMapID().wMainID, pLandMan->GetMapID().wSubID, emReqFail );
				m_bEntryFailed = TRUE;
			}
		}else{
			TEXTCONSOLEMSG_WRITE( "[ERROR]Land ID is NATIVEID NULL! charID[%s]", m_szName );
		}
	}


	m_setLandEffectNum.clear();
	


	hr = SetNavi ( pLandMan->GetNavi(), m_vPos );
	if ( FAILED(hr) )	return E_FAIL;

	m_lnLastSendExp = m_sExperience.lnNow;

	m_lnLastSendVNExp = m_lVNGainSysExp;

	m_bServerStorage = TRUE;	

	DWORD Num1 = m_cInventory.GetNumItems();
	DWORD Num2 = m_cVietnamInventory.GetNumItems ();

	m_fMoveDelay = 0.0f;
	CTime cTIME_CUR = CTime::GetCurrentTime();

	//	Note : â���� ��ȿ�� �˻�.
	//
	for ( WORD i=0; i<EMSTORAGE_CHANNEL_SPAN_SIZE; ++i )
	{
		CTime cSTORAGE(m_tSTORAGE[i]);

		bool bVALID(true);
		if ( m_tSTORAGE[i]==0 || cSTORAGE<cTIME_CUR )	bVALID = false;
		
		m_bSTORAGE[i] = bVALID;
	}

	//	Note : �����̾� ���� �˻�.
	//
	bool bPREMIUM(true);
	CTime tPREMIUM(m_tPREMIUM);
	if ( m_tPREMIUM==0 || tPREMIUM<cTIME_CUR )	bPREMIUM = false;
	m_bPREMIUM = bPREMIUM;

	m_cInventory.SetAddLine ( GetOnINVENLINE(), true );
	// ��Ʈ�� �κ��丮�� ���ΰ� 10ĭ�̹Ƿ� 6ĭ�� �� �߰��Ѵ�.
#if defined(VN_PARAM) //vietnamtest%%%
	m_cVietnamInventory.SetAddLine ( 6, true );
#endif


	//DoQuestCheckLeaveMap();


	SetSTATE(EM_ACT_WAITING);

	/*contribution/activity bonus, Juver, 2017/10/14 */
	m_bUseContributionPointTimer = GLCONST_CHAR::bContributionPointBonusTimer;
	m_fContributionPointBonusTimer = 0.0f;
	m_fContributionPointBonusTimeMax = GLCONST_CHAR::fContributionPointBonusTime;

	m_bUseActivityPointTimer = GLCONST_CHAR::bActivityPointBonusTimer;
	m_fActivityPointBonusTimer = 0.0f;
	m_fActivityPointBonusTimeMax = GLCONST_CHAR::fActivityPointBonusTime;
	/*Codex System, Jhoniex 2/14/2024 */
	CODEX_STATS(1,1);

	return S_OK;
}

HRESULT GLChar::SetNavi ( NavigationMesh* pNavi, D3DXVECTOR3 &vPos )
{
	m_vPos = vPos;
	if ( m_actorMove.PathIsActive() )		m_actorMove.Stop();

	m_actorMove.Create ( pNavi, vPos, -1 );
	m_actorMove.SetMaxSpeed ( GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX].fWALKVELO );

	return S_OK;
}

HRESULT GLChar::SetData ( PCHARDATA2 pCharData, BOOL bNEW )
{
	SCHARDATA2::Assign ( *pCharData );
	
	if ( !GLCHARLOGIC::INIT_DATA(bNEW,TRUE) )	return E_FAIL;

	return S_OK;
}

HRESULT GLChar::SavePosition ()
{
	//	Note : ���������� ���� ���� �������� �������� �ʴ´�.
	//
	if ( IsDie() )
	{
		m_sSaveMapID = NATIVEID_NULL();
	}
	//	Note : ���� ��ġ ����.
	//
	else
	{
		m_sSaveMapID = m_sMapID;
		m_vSavePos = m_vPos;
	}

	return S_OK;
}

//	Note : ���� ��ġ�� ����.
//
HRESULT GLChar::SaveLastCall ()
{
	if ( !m_pLandMan )	return S_FALSE;

	/*pvp tyranny, Juver, 2017/08/24 */
	/*school wars, Juver, 2018/01/19 */
	/*pvp capture the flag, Juver, 2018/01/31 */
	//	Note : ���� ���� Ŭ������ ���� ��� ���� ��ġ�� �������� ����.
	if ( !m_pLandMan->m_bGuidBattleMap && !m_pLandMan->m_bClubDeathMatchMap && 
		!m_pLandMan->m_bPVPTyrannyMap && !m_pLandMan->m_bPVPSchoolWarsMap && !m_pLandMan->m_bPVPCaptureTheFlagMap )
	{
		m_sLastCallMapID = m_sMapID;
		m_vLastCallPos = m_vPos;

		GLMSG::SNETPC_UPDATE_LASTCALL NetMsg;
		NetMsg.sLastCallMapID = m_sLastCallMapID;
		NetMsg.vLastCallPos = m_vLastCallPos;
		m_pGLGaeaServer->SENDTOCLIENT( m_dwClientID, &NetMsg );
	}

	return S_OK;
}

HRESULT GLChar::LoadStorageFromDB ()
{
	if ( m_bServerStorage )	return S_OK;

	GLDBMan* pDBMan = m_pGLGaeaServer->GetDBMan();
	if ( pDBMan )	pDBMan->ReadUserInven ( &GetCharData2() );
	m_bServerStorage = TRUE;

	return S_OK;
}

HRESULT GLChar::SetPosition ( D3DXVECTOR3 &vPos )
{
	m_actorMove.SetPosition ( vPos, -1 );
	if ( m_actorMove.PathIsActive() )		m_actorMove.Stop();

	m_vPos = m_actorMove.Position();

	return S_OK;
}

BOOL GLChar::IsValidBody () const
{
	if ( IsSTATE(EM_ACT_DIE) )			return FALSE;
	if ( IsSTATE(EM_ACT_WAITING) )		return FALSE;
	if ( IsSTATE(EM_REQ_LOGOUT) )		return FALSE;

	return ( m_Action < GLAT_FALLING );
}

BOOL GLChar::IsDie () const
{
	if ( !IsSTATE(EM_ACT_WAITING) )
	{
		if ( m_Action==GLAT_DIE )		return TRUE;
		if ( IsSTATE(EM_ACT_DIE) )		return TRUE;
	}

	return FALSE;
}

BOOL GLChar::IsVisibleDetect ( const BOOL bRECVISIBLE )
{
	if ( IsSTATE(EM_REQ_VISIBLENONE) || IsSTATE(EM_REQ_VISIBLEOFF) )	return FALSE;

	if ( m_bINVISIBLE && !bRECVISIBLE )									return FALSE;

	return TRUE;
}

BOOL GLChar::IsVaildTradeData ()
{
	//	�ݾ� ��ȿ�� �˻�.
	if ( m_lnMoney < m_sTrade.GetMoney() )		return FALSE;

	GLInventory &sTradeBox = m_sTrade.GetTradeBox();
	GLInventory::CELL_MAP* pItemList = sTradeBox.GetItemList();

	GLInventory::CELL_MAP_ITER iter = pItemList->begin();
	GLInventory::CELL_MAP_ITER iter_end = pItemList->end();
	for ( ; iter!=iter_end; ++iter )
	{
		SINVENITEM* pTradeItem = (*iter).second;
		SINVENITEM* pInvenItem = m_cInventory.GetItem ( pTradeItem->wBackX, pTradeItem->wBackY );
		if ( !pInvenItem )														return FALSE;

		if ( memcmp ( &pTradeItem->sItemCustom, &pInvenItem->sItemCustom, sizeof(SITEMCUSTOM) ) )	return FALSE;
	}

	return TRUE;
}

BOOL GLChar::IsVaildTradeInvenSpace ( GLInventory &sTradeBoxTar )
{
	GLInventory sInvenTemp;
	sInvenTemp.SetAddLine ( m_cInventory.GETAddLine(), true );
	sInvenTemp.Assign ( m_cInventory );

	//	Note : �ڽ��� ��ȯ ��Ͽ� �ö� �������� �ι꿡�� ����.
	//
	GLInventory &sTradeBoxMy = m_sTrade.GetTradeBox();
	GLInventory::CELL_MAP* pItemListMy = sTradeBoxMy.GetItemList();

	GLInventory::CELL_MAP_ITER iter = pItemListMy->begin();
	GLInventory::CELL_MAP_ITER iter_end = pItemListMy->end();
	for ( ; iter!=iter_end; ++iter )
	{
		SINVENITEM* pTradeItem = (*iter).second;

		BOOL bOk = sInvenTemp.DeleteItem ( pTradeItem->wBackX, pTradeItem->wBackY );
		if ( !bOk )		return FALSE;
	}

	//	Note : ������ ��ȯ ��ǰ�� ��� ���� �ִ��� �˻�.
	//
	GLInventory::CELL_MAP* pItemListTar = sTradeBoxTar.GetItemList();

	iter = pItemListTar->begin();
	iter_end = pItemListTar->end();
	for ( ; iter!=iter_end; ++iter )
	{
		SINVENITEM* pTradeItem = (*iter).second;

		SITEM *pItem = GLItemMan::GetInstance().GetItem ( pTradeItem->sItemCustom.sNativeID );
		if ( !pItem )	return FALSE;

		WORD wPosX = 0, wPosY = 0;
		BOOL bOk = sInvenTemp.FindInsrtable ( pItem->sBasicOp.wInvenSizeX, pItem->sBasicOp.wInvenSizeY, wPosX, wPosY );
		if ( !bOk )		return FALSE;

		bOk = sInvenTemp.InsertItem ( pTradeItem->sItemCustom, wPosX, wPosY );
		if ( !bOk )		return FALSE;
	}
	
	return TRUE;
}

BOOL GLChar::DoTrade ( GLTrade &sTrade, DWORD dwFromCharID )
{
	//	Note : �� �ݾ�.
	//
	LONGLONG lnMoneyOut = m_sTrade.GetMoney();
	if ( m_lnMoney < lnMoneyOut ) return FALSE;
	CheckMoneyUpdate( m_lnMoney, lnMoneyOut, FALSE, "Do Trade Out Money." );
	m_lnMoney -= lnMoneyOut;	

	//	Note : ���� �ݾ�.
	//
	LONGLONG lnMoneyIn = sTrade.GetMoney();
	CheckMoneyUpdate( m_lnMoney, lnMoneyIn, TRUE, "Do Trade In Money." );
	m_lnMoney += lnMoneyIn;

	m_bMoneyUpdate = TRUE;

	//	Note : �ݾ� �α�.
	//
	LONGLONG lnMoneyDx = (lnMoneyIn-lnMoneyOut);
	if ( lnMoneyDx!=0 )
	{
		GLITEMLMT::GetInstance().ReqMoneyExc ( ID_CHAR, dwFromCharID, ID_CHAR, m_dwCharID, lnMoneyDx, EMITEM_ROUTE_CHAR );
		GLITEMLMT::GetInstance().ReqMoneyExc ( ID_CHAR, m_dwCharID, ID_CHAR, 0, m_lnMoney, EMITEM_ROUTE_CHAR );
	}

#if defined(_RELEASED) || defined(TW_PARAM) || defined(TH_PARAM) || defined(HK_PARAM) // ***Tracing Log print
	if ( m_bTracingUser )
	{
		NET_LOG_UPDATE_TRACINGCHAR TracingMsg;
		TracingMsg.nUserNum  = GetUserID();
		StringCchCopy( TracingMsg.szAccount, USR_ID_LENGTH+1, m_szUID );

		CString strTemp;
		PGLCHAR pFromChar = m_pGLGaeaServer->GetCharID( dwFromCharID );
		if ( pFromChar != NULL )
		{
			strTemp.Format( "Do Trade! From, [%s][%s], Target[%s][%s], RecvMoney:[%I64d], SendMoney:[%I64d], HaveMoney:[%I64d], Target HaveMoney:[%I64d]",
					 m_szUID, m_szName, pFromChar->m_szUID, pFromChar->m_szName, lnMoneyIn, lnMoneyOut, m_lnMoney, pFromChar->m_lnMoney );
		}else{
			strTemp.Format( "Do Trade! From, [%s][%s], Target Not Found, RecvMoney:[%I64d], SendMoney:[%I64d], HaveMoney:[%I64d]",
					 m_szUID, m_szName, lnMoneyIn, lnMoneyOut, m_lnMoney );
		}

		StringCchCopy( TracingMsg.szLogMsg, TRACING_LOG_SIZE, strTemp.GetString() );

		m_pGLGaeaServer->SENDTOAGENT( m_dwClientID, &TracingMsg );
	}
#endif

	PGLCHAR pFromChar = m_pGLGaeaServer->GetCharID( dwFromCharID );
	CString strTemp;
	if ( pFromChar != NULL )
	{
		strTemp.Format( ",Do Trade! From, [%s][%s], Target[%s][%s], RecvMoney:[%I64d], SendMoney:[%I64d], HaveMoney:[%I64d], Target HaveMoney:[%I64d]",
				 m_szUID, m_szName, pFromChar->m_szUID, pFromChar->m_szName, lnMoneyIn, lnMoneyOut, m_lnMoney, pFromChar->m_lnMoney );


		CString strName;
		strName.Format ( "%d.csv",GetUserID() );
		CDebugSet::ToFileWithTime ( strName.GetString(),strTemp );

	}else{
		strTemp.Format( ",Do Trade! From, [%s][%s], Target Not Found, RecvMoney:[%I64d], SendMoney:[%I64d], HaveMoney:[%I64d]",
				 m_szUID, m_szName, lnMoneyIn, lnMoneyOut, m_lnMoney );

		CString strName;
		strName.Format ( "%d.csv",GetUserID() );
		CDebugSet::ToFileWithTime ( strName.GetString(),strTemp );
	}

	//	Note : �ݾ� ��ȭ Ŭ���̾�Ʈ�� �˸�.
	//
	GLMSG::SNETPC_UPDATE_MONEY NetMsgMoney;
	NetMsgMoney.lnMoney = m_lnMoney;
	/*dmk14 ingame web*/
	NetMsgMoney.dwPremium = m_dwPremiumPoints;
	NetMsgMoney.dwCombat = m_dwCombatPoints;
	m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsgMoney );


	//	Note : �ڽ��� ��ȯ ��Ͽ� �ö� �������� �ι꿡�� ����.
	//
	GLInventory &sTradeBoxMy = m_sTrade.GetTradeBox();
	GLInventory::CELL_MAP* pItemListMy = sTradeBoxMy.GetItemList();
	
	SINVENITEM* pTradeItem = NULL;
	BOOL bOk = FALSE;
	GLMSG::SNETPC_INVEN_DELETE NetMsgDelete;
	GLInventory::CELL_MAP_ITER iter = pItemListMy->begin();
	GLInventory::CELL_MAP_ITER iter_end = pItemListMy->end();
	for ( ; iter!=iter_end; ++iter )
	{
		pTradeItem = (*iter).second;

		bOk = m_cInventory.DeleteItem ( pTradeItem->wBackX, pTradeItem->wBackY );
		if ( !bOk )
		{
			return FALSE;
		}
		else
		{
			//	Note : ������ ��ȭ�� Ŭ���̾�Ʈ�� �˸�.
			//			
			NetMsgDelete.wPosX = pTradeItem->wBackX;
			NetMsgDelete.wPosY = pTradeItem->wBackY;
			m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsgDelete );
		}
	}

	//	Note : ������ �ִ� ��ǰ�� �ι꿡 ����.
	//
	GLInventory::CELL_MAP* pItemListTar = sTrade.GetTradeBox().GetItemList();

	pTradeItem = NULL;
	SITEM* pItem = NULL;
	WORD wPosX = 0, wPosY = 0;

	SINVENITEM* pInvenItem = NULL;
	GLMSG::SNETPC_INVEN_INSERT NetMsgInert;

	if ( !pItemListTar->empty() && pFromChar != NULL )
	{
		strTemp.Format( ",Do Trade Item! From, [%s][%s], Target[%s][%s]",m_szUID, m_szName, pFromChar->m_szUID, pFromChar->m_szName);
		
		CString strName;
		strName.Format ( "%d.csv",GetUserID() );
		CDebugSet::ToFileWithTime ( strName.GetString(),strTemp );
	}

	iter = pItemListTar->begin();
	iter_end = pItemListTar->end();	
	for ( ; iter!=iter_end; ++iter )
	{
		pTradeItem = (*iter).second;
		pItem = GLItemMan::GetInstance().GetItem ( pTradeItem->sItemCustom.sNativeID );
		if ( !pItem )	return FALSE;

		wPosX = 0;
		wPosY = 0;
		bOk = m_cInventory.FindInsrtable ( pItem->sBasicOp.wInvenSizeX, pItem->sBasicOp.wInvenSizeY, wPosX, wPosY );
		if ( !bOk )		return FALSE;

		bOk = m_cInventory.InsertItem ( pTradeItem->sItemCustom, wPosX, wPosY );
		if ( !bOk )		return FALSE;


		strTemp.Format( ",%02d_%02d,%s x (%d/%d), GenNum(%d)",
						pTradeItem->sItemCustom.sNativeID.wMainID,
						pTradeItem->sItemCustom.sNativeID.wSubID,
						pItem->GetName(),
						pTradeItem->sItemCustom.wTurnNum,
						pItem->sDrugOp.wPileNum,
						pTradeItem->sItemCustom.lnGenNum);
		
		CString strName;
		strName.Format ( "%d.csv",GetUserID() );
		CDebugSet::ToFileWithTime ( strName.GetString(),strTemp );

/*
		// ȹ���� �������� ��ī�� && ������ ���̶�� �������� �����Ѵ�.
		// �־��̵� 0 �� �ƴϸ� ��ī���̸鼭 DB�� ���� ������ ���̴�.
		if ( pItem->sBasicOp.emItemType == ITEM_PET_CARD && pTradeItem->sItemCustom.dwPetID != 0 )
		{
			CExchangePet* pDbAction = new CExchangePet ( m_dwCharID, pTradeItem->sItemCustom.dwPetID );
			GLDBMan* pDBMan = m_pGLGaeaServer->GetDBMan ();
			if ( pDBMan ) pDBMan->AddJob ( pDbAction );
		}
*/

		pInvenItem = m_cInventory.GetItem ( wPosX, wPosY );

		//	Note :�������� ���� ���� ��� ���.
		//
		GLITEMLMT::GetInstance().ReqItemRoute ( pInvenItem->sItemCustom, ID_CHAR, dwFromCharID, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, pInvenItem->sItemCustom.wTurnNum );

		//	Note : ������ ��ȭ�� Ŭ���̾�Ʈ�� �˸�.
		//		
		NetMsgInert.Data = *pInvenItem;
		m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsgInert );
	}

	return TRUE;
}

// *****************************************************
// Desc: �ð����� ������ ������Ʈ
// *****************************************************
void GLChar::RESET_TIMELMT_ITEM ()
{
	std::vector<GLInventory::CELL_KEY>	vecDELETE;

	const CTime cTIME_CUR = CTime::GetCurrentTime();

	SINVENITEM* pInvenItem = NULL;
	SITEM* pITEM = NULL;
	GLMSG::SNET_INVEN_DEL_ITEM_TIMELMT NetMsgInvenDelTimeLmt;
	GLMSG::SNET_INVEN_ITEM_UPDATE NetItemUpdate;

	// �� ������ ����
	PGLPETFIELD pMyPet = m_pGLGaeaServer->GetPET ( m_dwPetGUID );
	if ( pMyPet && pMyPet->IsValid () ) pMyPet->UpdateTimeLmtItem ( this );

	// Ż�� ������ ���� 
	if ( m_sVehicle.IsActiveValue() ) m_sVehicle.UpdateTimeLmtItem( this );

//	GLGaeaServer & glGaeaServer = m_pGLGaeaServer;
	GLITEMLMT & glItemmt = GLITEMLMT::GetInstance();
	GLItemMan & glItemMan = GLItemMan::GetInstance();
	
	// �κ��丮 ������
	GLInventory::CELL_MAP* pMapItem = m_cInventory.GetItemList();
	GLInventory::CELL_MAP_CITER iter = pMapItem->begin();
	GLInventory::CELL_MAP_CITER iter_end = pMapItem->end();
	for ( ; iter!=iter_end; ++iter )
	{
		pInvenItem = (*iter).second;
		pITEM = glItemMan.GetItem ( pInvenItem->sItemCustom.sNativeID );
		if ( !pITEM )	continue;

		bool bDELETE(false);

		// ���Ѻ� ������
		if ( pITEM->IsTIMELMT() )
		{
			CTimeSpan cSPAN(pITEM->sDrugOp.tTIME_LMT);
			CTime cTIME_LMT(pInvenItem->sItemCustom.tBORNTIME);
			cTIME_LMT += cSPAN;

			if ( cTIME_CUR > cTIME_LMT )
			{
				vecDELETE.push_back ( (*iter).first );
				bDELETE = true;

				if ( pITEM->sBasicOp.emItemType == ITEM_VEHICLE && pInvenItem->sItemCustom.dwVehicleID != 0 )
				{
					// �Ǽ��縮�� üũ�� ���� DB���� �������ش�.
					GLVEHICLE* pNewVehicle = new GLVEHICLE();
					CGetVehicle* pGetVehicle = new CGetVehicle ( pNewVehicle, 
													 pInvenItem->sItemCustom.dwVehicleID, 
													 m_dwClientID, 
													 m_dwCharID, 
													 true );
					GLDBMan* pDBMan = m_pGLGaeaServer->GetDBMan ();
					if ( pDBMan ) pDBMan->AddJob ( pGetVehicle );				

				}

				// ��ī���� ���
				if ( pITEM->sBasicOp.emItemType == ITEM_PET_CARD && pInvenItem->sItemCustom.dwPetID != 0 )
				{
					// Ȱ�����̸� ������� ���ش�.
					if ( pMyPet && pInvenItem->sItemCustom.dwPetID == pMyPet->m_dwPetID ) 
					{
						m_pGLGaeaServer->ReserveDropOutPet ( SDROPOUTPETINFO(pMyPet->m_dwGUID,true,false) );

						// ���� �Ǽ��縮�� �˻��Ѵ�.
						pMyPet->UpdateTimeLmtItem ( this );

						// �־��ٲ� �־��ش�.
						for ( WORD i = 0; i < PET_ACCETYPE_SIZE; ++i )
						{
							CItemDrop cDropItem;
							cDropItem.sItemCustom = pMyPet->m_PutOnItems[i];
							if ( IsInsertToInven ( &cDropItem ) ) InsertToInven ( &cDropItem );
							else
							{
								if ( m_pLandMan )
								{
									m_pLandMan->DropItem ( m_vPos, 
														   &(cDropItem.sItemCustom), 
														   EMGROUP_ONE, 
														   m_dwGaeaID );
								}
							}
						}

						CDeletePet* pDeletePet = new CDeletePet ( m_dwCharID, pInvenItem->sItemCustom.dwPetID );
						GLDBMan* pDBMan = m_pGLGaeaServer->GetDBMan ();
						if ( pDBMan ) pDBMan->AddJob ( pDeletePet );
						
						// ���� �����Ǹ� ��Ȱ������ Ŭ���̾�Ʈ�� �˷���.
						CGetRestorePetList *pGetRestorePetList = new CGetRestorePetList ( m_dwCharID, m_dwClientID );
						if ( pDBMan ) pDBMan->AddJob ( pGetRestorePetList );
					}
					else
					{
						// �Ǽ��縮�� üũ�� ���� DB���� �������ش�.
						GLPET* pNewPet = new GLPET ();
						CGetPet* pGetPet = new CGetPet ( pNewPet, 
														 pInvenItem->sItemCustom.dwPetID, 
														 m_dwClientID, 
														 m_dwCharID,
														 pInvenItem->wPosX,
														 pInvenItem->wPosY,
														 true );
						GLDBMan* pDBMan = m_pGLGaeaServer->GetDBMan ();
						if ( pDBMan ) pDBMan->AddJob ( pGetPet );
					}
				}
			}
		}

		// �ڽ�Ƭ
		if ( !bDELETE && pInvenItem->sItemCustom.nidDISGUISE!=SNATIVEID(false) )
		{
			if ( pInvenItem->sItemCustom.tDISGUISE != 0 )
			{
				CTime cTIME_LMT(pInvenItem->sItemCustom.tDISGUISE);
				if ( cTIME_LMT.GetYear()!=1970 )
				{
					if ( cTIME_CUR > cTIME_LMT )
					{
						//	�ð� �������� ������ ���� �˸�.						
						NetMsgInvenDelTimeLmt.nidITEM = pInvenItem->sItemCustom.nidDISGUISE;
						m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgInvenDelTimeLmt);

						//	Note : �ڽ��� ���� ����.
						pInvenItem->sItemCustom.tDISGUISE = 0;
						pInvenItem->sItemCustom.nidDISGUISE = SNATIVEID(false);

						/*item color, Juver, 2018/01/08 */
						SITEM* pitem_data_check = GLItemMan::GetInstance().GetItem( pInvenItem->sItemCustom.sNativeID );
						if ( pitem_data_check && pitem_data_check->sBasicOp.bItemColor )
						{
							pInvenItem->sItemCustom.wColor1 = pitem_data_check->sBasicOp.wItemColor1;
							pInvenItem->sItemCustom.wColor2 = pitem_data_check->sBasicOp.wItemColor2;
						}
						else
						{
							pInvenItem->sItemCustom.wColor1 = ITEMCOLOR_WHITE;
							pInvenItem->sItemCustom.wColor2 = ITEMCOLOR_WHITE;
						}

						//	Note : Ŭ���̾�Ʈ�� �˸�.						
						NetItemUpdate.wPosX = pInvenItem->wPosX;
						NetItemUpdate.wPosY = pInvenItem->wPosY;
						NetItemUpdate.sItemCustom = pInvenItem->sItemCustom;
						m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetItemUpdate);

						//	Note : �α� ����.
						glItemmt.ReqItemConversion ( pInvenItem->sItemCustom, ID_CHAR, m_dwCharID );
					}
				}
			}
		}
	}

	if ( !vecDELETE.empty() )
	{
		for ( DWORD i=0; i<vecDELETE.size(); ++i )
		{
			const GLInventory::CELL_KEY &cKEY = vecDELETE[i];
			SINVENITEM sINVENITEM = *m_cInventory.GetItem(cKEY.first,cKEY.second);

			SNATIVEID nidITEM = sINVENITEM.sItemCustom.sNativeID;

			//	�ð� �������� ������ ���� �α� ����.
			glItemmt.ReqItemRoute ( sINVENITEM.sItemCustom, ID_CHAR, m_dwCharID, ID_CHAR, 0, EMITEM_ROUTE_DELETE, sINVENITEM.sItemCustom.wTurnNum );

			//	Note : ������ ����.
			//
			m_cInventory.DeleteItem ( cKEY.first, cKEY.second );

			//	[�ڽſ���] �ش� �������� �ι꿡�� ����.
			GLMSG::SNETPC_INVEN_DELETE NetMsgInvenDel;
			NetMsgInvenDel.wPosX = cKEY.first;
			NetMsgInvenDel.wPosY = cKEY.second;
			m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgInvenDel);

			//	�ð� �������� ������ ���� �˸�.
			GLMSG::SNET_INVEN_DEL_ITEM_TIMELMT NetMsgInvenDelTimeLmt;
			NetMsgInvenDelTimeLmt.nidITEM = nidITEM;
			m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgInvenDelTimeLmt);

			//	Note : �ڽ��� ����.
			if ( sINVENITEM.sItemCustom.nidDISGUISE!=SNATIVEID(false) )
			{
				SITEM *pONE = glItemMan.GetItem ( sINVENITEM.sItemCustom.nidDISGUISE );
				if ( !pONE )		continue;

				//	�κ� ��ġ.
				WORD wInsertPosX = cKEY.first, wInsertPosY = cKEY.second;

				SITEMCUSTOM sITEM_NEW;
				sITEM_NEW.sNativeID = sINVENITEM.sItemCustom.nidDISGUISE;
				CTime cTIME = CTime::GetCurrentTime();
				if ( sINVENITEM.sItemCustom.tDISGUISE!=0 && pONE->sDrugOp.tTIME_LMT!= 0 )
				{
					cTIME = CTime(sINVENITEM.sItemCustom.tDISGUISE);

					CTimeSpan tLMT(pONE->sDrugOp.tTIME_LMT);
					cTIME -= tLMT;
				}

				//	Note : ������ ����.
				//
				sITEM_NEW.tBORNTIME = cTIME.GetTime();
				sITEM_NEW.wTurnNum = 1;
				sITEM_NEW.cGenType = EMGEN_BILLING;
				sITEM_NEW.cChnID = (BYTE)m_pGLGaeaServer->GetServerChannel();
				sITEM_NEW.cFieldID = (BYTE)m_pGLGaeaServer->GetFieldSvrID();
				sITEM_NEW.lnGenNum = glItemmt.RegItemGen ( sITEM_NEW.sNativeID, (EMITEMGEN)sITEM_NEW.cGenType );

				/*item color, Juver, 2018/01/08 */
				sITEM_NEW.wColor1 = sINVENITEM.sItemCustom.wColor1;
				sITEM_NEW.wColor2 = sINVENITEM.sItemCustom.wColor2;

				//	Note : �κ��� �� ���� �˻�.
				BOOL bOk = m_cInventory.IsInsertable ( pONE->sBasicOp.wInvenSizeX, pONE->sBasicOp.wInvenSizeY, wInsertPosX, wInsertPosY );
				if ( !bOk )		continue;

				//	Note : �κ��� ����.
				m_cInventory.InsertItem ( sITEM_NEW, wInsertPosX, wInsertPosY );
				SINVENITEM *pINSERT_ITEM = m_cInventory.GetItem ( wInsertPosX, wInsertPosY );

				GLMSG::SNETPC_INVEN_INSERT NetItemInsert;
				NetItemInsert.Data = *pINSERT_ITEM;
				m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetItemInsert);

				//	Note : �и��� �ڽ��� �α�.
				glItemmt.ReqItemRoute ( pINSERT_ITEM->sItemCustom, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, pINSERT_ITEM->sItemCustom.wTurnNum );
			}
		}
	}

	// ������ ���� ���� ����
	for ( int i=0; i<SLOT_TSIZE; ++i )
	{
		EMSLOT emSLOT = (EMSLOT) i;
		if( m_PutOnItems[emSLOT].sNativeID==NATIVEID_NULL() ) continue;

		SITEMCUSTOM sCUSTOM = m_PutOnItems[emSLOT];

		SITEM *pITEM = glItemMan.GetItem ( sCUSTOM.sNativeID );
		if ( !pITEM )								continue;

		bool bDELETE(false);

		if ( pITEM->IsTIMELMT() )
		{
			CTimeSpan cSPAN(pITEM->sDrugOp.tTIME_LMT);
			CTime cTIME_LMT(sCUSTOM.tBORNTIME);
			cTIME_LMT += cSPAN;

			if ( cTIME_CUR > cTIME_LMT )
			{
				//	�ð� �������� ������ ���� �α� ����.
				glItemmt.ReqItemRoute ( sCUSTOM, ID_CHAR, m_dwCharID, ID_CHAR, 0, EMITEM_ROUTE_DELETE, sCUSTOM.wTurnNum );

				//	�ð� �������� ������ ���� �˸�.
				GLMSG::SNET_INVEN_DEL_ITEM_TIMELMT NetMsgInvenDelTimeLmt;
				NetMsgInvenDelTimeLmt.nidITEM = sCUSTOM.sNativeID;
				m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgInvenDelTimeLmt);

				if ( pITEM->sBasicOp.emItemType == ITEM_VEHICLE && sCUSTOM.dwVehicleID != 0 )
				{
					m_pGLGaeaServer->SaveVehicle( m_dwClientID, m_dwGaeaID, false );

                	// �Ǽ��縮�� üũ�� ���� DB���� �������ش�.
					GLVEHICLE* pNewVehicle = new GLVEHICLE();
					CGetVehicle* pGetVehicle = new CGetVehicle ( pNewVehicle, 
													 sCUSTOM.dwVehicleID, 
													 m_dwClientID, 
													 m_dwCharID, 
													 true );

					GLDBMan* pDBMan = m_pGLGaeaServer->GetDBMan ();
					if ( pDBMan ) pDBMan->AddJob ( pGetVehicle );
				}

				//	������ ����.
				RELEASE_SLOT_ITEM ( emSLOT );

				//	[�ڽſ���]  ������ ����.
				GLMSG::SNETPC_PUTON_RELEASE NetMsg_Release(emSLOT);
				m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsg_Release);

				//	Note : �ֺ� ����鿡�� ������ ����.
				GLMSG::SNETPC_PUTON_RELEASE_BRD NetMsgReleaseBrd;
				NetMsgReleaseBrd.dwGaeaID = m_dwGaeaID;
				NetMsgReleaseBrd.emSlot = emSLOT;
				SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&NetMsgReleaseBrd) );

				bDELETE = true;

				//	Note : �ڽ��� ����.
				if ( sCUSTOM.nidDISGUISE!=SNATIVEID(false) )
				{
					SITEM *pONE = glItemMan.GetItem ( sCUSTOM.nidDISGUISE );
					if ( !pONE )		continue;

					SITEMCUSTOM sITEM_NEW;
					sITEM_NEW.sNativeID = sCUSTOM.nidDISGUISE;
					CTime cTIME = CTime::GetCurrentTime();
					if ( sCUSTOM.tDISGUISE!=0 && pONE->sDrugOp.tTIME_LMT!= 0 )
					{
						cTIME = CTime(sCUSTOM.tDISGUISE);

						CTimeSpan tLMT(pONE->sDrugOp.tTIME_LMT);
						cTIME -= tLMT;
					}

					//	Note : ������ ����.
					//
					sITEM_NEW.tBORNTIME = cTIME.GetTime();
					sITEM_NEW.wTurnNum = 1;
					sITEM_NEW.cGenType = EMGEN_BILLING;
					sITEM_NEW.cChnID = (BYTE)m_pGLGaeaServer->GetServerChannel();
					sITEM_NEW.cFieldID = (BYTE)m_pGLGaeaServer->GetFieldSvrID();
					sITEM_NEW.lnGenNum = glItemmt.RegItemGen ( sITEM_NEW.sNativeID, (EMITEMGEN)sITEM_NEW.cGenType );

					/*item color, Juver, 2018/01/08 */
					sITEM_NEW.wColor1 = sCUSTOM.wColor1;
					sITEM_NEW.wColor2 = sCUSTOM.wColor2;

					//	Note : ���� ����.
					SLOT_ITEM ( sITEM_NEW, emSLOT );

					//	Note : �ڽſ��� ��������� ����.
					GLMSG::SNETPC_PUTON_UPDATE NetMsgUpdate;
					NetMsgUpdate.emSlot = emSLOT;
					NetMsgUpdate.sItemCustom = sITEM_NEW;
					m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgUpdate);

					//	Note : �ֺ� ����鿡�� ��������� ����.
					GLMSG::SNETPC_PUTON_UPDATE_BRD NetMsgUpdateBrd;
					NetMsgUpdateBrd.dwGaeaID = m_dwGaeaID;
					NetMsgUpdateBrd.emSlot = emSLOT;
					NetMsgUpdateBrd.sItemClient.Assign ( sITEM_NEW );
					SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&NetMsgUpdateBrd) );

					//	Note : �и��� �ڽ��� �α�.
					glItemmt.ReqItemRoute ( sITEM_NEW, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, sITEM_NEW.wTurnNum );
				}
			}
		}
		
		if ( !bDELETE && VALID_SLOT_ITEM(emSLOT) && sCUSTOM.nidDISGUISE!=SNATIVEID(false) )
		{
			if ( sCUSTOM.tDISGUISE != 0 )
			{
				CTime cTIME_LMT(sCUSTOM.tDISGUISE);
				if ( cTIME_LMT.GetYear()!=1970 )
				{
					if ( cTIME_CUR > cTIME_LMT )
					{
						//	�ð� �������� ������ ���� �˸�.
						GLMSG::SNET_INVEN_DEL_ITEM_TIMELMT NetMsgInvenDelTimeLmt;
						NetMsgInvenDelTimeLmt.nidITEM = sCUSTOM.nidDISGUISE;
						m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgInvenDelTimeLmt);

						//	Note : �ڽ��� ���� ����.
						sCUSTOM.tDISGUISE = 0;
						sCUSTOM.nidDISGUISE = SNATIVEID(false);
						SLOT_ITEM ( sCUSTOM, emSLOT );			//	������ ����.

						//	Note : �ڽſ��� ��������� ����.
						GLMSG::SNETPC_PUTON_UPDATE NetMsgUpdate;
						NetMsgUpdate.emSlot = emSLOT;
						NetMsgUpdate.sItemCustom = sCUSTOM;
						m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgUpdate);

						//	Note : �ֺ� ����鿡�� ��������� ����.
						GLMSG::SNETPC_PUTON_UPDATE_BRD NetMsgUpdateBrd;
						NetMsgUpdateBrd.dwGaeaID = m_dwGaeaID;
						NetMsgUpdateBrd.emSlot = emSLOT;
						NetMsgUpdateBrd.sItemClient.Assign ( sCUSTOM );
						SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&NetMsgUpdateBrd) );
					}
				}
			}
		}
	}

	//	Note : �κ��丮 �߰����� ��ȿ�� �˻�.
	//
	GLMSG::SNETPC_STORAGE_STATE NetMsgStorageState;
	for ( WORD i=0; i<EMSTORAGE_CHANNEL_SPAN_SIZE; ++i )
	{
		bool bVALID(true);
		CTime cSTORAGE(m_tSTORAGE[i]);

		if ( m_tSTORAGE[i]==0 )		bVALID = false;
		if ( cSTORAGE<cTIME_CUR )
		{
			bVALID = false;
		}

		m_bSTORAGE[i] = bVALID;
		NetMsgStorageState.bVALID[i] = bVALID;
	}
	m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgStorageState);

	bool bPREMIUM(true);
	CTime cPREMIUM(m_tPREMIUM);
	if ( m_tPREMIUM==0 )			bPREMIUM = false;
	if ( cPREMIUM<cTIME_CUR )
	{
		bPREMIUM = false;
	}

	m_bPREMIUM = bPREMIUM;

	GLMSG::SNETPC_PREMIUM_STATE NetMsgPremium;
	NetMsgPremium.bPREMIUM = bPREMIUM;
	m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgPremium);

	//	Note : ���� Ȱ��ȭ�� �κ� ���� ����.
	//
	m_cInventory.SetAddLine ( GetOnINVENLINE(), true );

	// â�� ������ ��ȿ�� ����
	for ( WORD idx = 0; idx < EMSTORAGE_CHANNEL; ++idx )
	{
		GLInventory::CELL_MAP* pMapItem = m_cStorage[idx].GetItemList();
		GLInventory::CELL_MAP_CITER iter = pMapItem->begin();
		GLInventory::CELL_MAP_CITER iter_end = pMapItem->end();

		vecDELETE.clear();

		for ( ; iter!=iter_end; ++iter )
		{
			SINVENITEM *pInvenItem = (*iter).second;

			SITEM *pITEM = glItemMan.GetItem ( pInvenItem->sItemCustom.sNativeID );
			if ( !pITEM )								continue;

			bool bDELETE(false);

			// ���Ѻ� ������
			if ( pITEM->IsTIMELMT() )
			{
				CTimeSpan cSPAN(pITEM->sDrugOp.tTIME_LMT);
				CTime cTIME_LMT(pInvenItem->sItemCustom.tBORNTIME);
				cTIME_LMT += cSPAN;

				if ( cTIME_CUR > cTIME_LMT )
				{
					vecDELETE.push_back ( (*iter).first );
					bDELETE = true;

					// ��ī���� ���
					if ( pITEM->sBasicOp.emItemType == ITEM_PET_CARD && pInvenItem->sItemCustom.dwPetID != 0 )
					{
						// �Ǽ��縮�� üũ�� ���� DB���� �������ش�.
						GLPET* pNewPet = new GLPET ();
						CGetPet* pGetPet = new CGetPet ( pNewPet, 
														 pInvenItem->sItemCustom.dwPetID, 
														 m_dwClientID, 
														 m_dwCharID,
														 pInvenItem->wPosX,
														 pInvenItem->wPosY,
														 true );
						GLDBMan* pDBMan = m_pGLGaeaServer->GetDBMan ();
						if ( pDBMan ) pDBMan->AddJob ( pGetPet );
					}

					// Ż���� ���
					if ( pITEM->sBasicOp.emItemType == ITEM_VEHICLE && pInvenItem->sItemCustom.dwVehicleID != 0 )
					{
						// �Ǽ��縮�� üũ�� Ż���� DB���� �������ش�.
						GLVEHICLE* pNewVehicle = new GLVEHICLE();
						CGetVehicle* pGetVehicle = new CGetVehicle ( pNewVehicle, 
													 pInvenItem->sItemCustom.dwVehicleID, 
													 m_dwClientID,
													 m_dwCharID, 
													 true );
						GLDBMan* pDBMan = m_pGLGaeaServer->GetDBMan ();
						if ( pDBMan ) pDBMan->AddJob ( pGetVehicle );		
					}
				}
			}
			
			// �ڽ�Ƭ
			if ( !bDELETE && pInvenItem->sItemCustom.nidDISGUISE!=SNATIVEID(false) )
			{
				if ( pInvenItem->sItemCustom.tDISGUISE != 0 )
				{
					CTime cTIME_LMT(pInvenItem->sItemCustom.tDISGUISE);
					if ( cTIME_LMT.GetYear()!=1970 )
					{
						if ( cTIME_CUR > cTIME_LMT )
						{
							//	�ð� �������� ������ ���� �˸�.
							GLMSG::SNET_INVEN_DEL_ITEM_TIMELMT NetMsgInvenDelTimeLmt;
							NetMsgInvenDelTimeLmt.nidITEM = pInvenItem->sItemCustom.nidDISGUISE;
							m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgInvenDelTimeLmt);

							//	Note : �ڽ��� ���� ����.
							pInvenItem->sItemCustom.tDISGUISE = 0;
							pInvenItem->sItemCustom.nidDISGUISE = SNATIVEID(false);

							/*item color, Juver, 2018/01/08 */
							SITEM* pitem_data_check = GLItemMan::GetInstance().GetItem( pInvenItem->sItemCustom.sNativeID );
							if ( pitem_data_check && pitem_data_check->sBasicOp.bItemColor )
							{
								pInvenItem->sItemCustom.wColor1 = pitem_data_check->sBasicOp.wItemColor1;
								pInvenItem->sItemCustom.wColor2 = pitem_data_check->sBasicOp.wItemColor2;
							}
							else
							{
								pInvenItem->sItemCustom.wColor1 = ITEMCOLOR_WHITE;
								pInvenItem->sItemCustom.wColor2 = ITEMCOLOR_WHITE;
							}

							//	Note : Ŭ���̾�Ʈ�� �˸�.
							GLMSG::SNETPC_STORAGE_ITEM_UPDATE NetItemUpdate;
							NetItemUpdate.dwChannel = idx;
							NetItemUpdate.wPosX = pInvenItem->wPosX;
							NetItemUpdate.wPosY = pInvenItem->wPosY;
							NetItemUpdate.sItemCustom = pInvenItem->sItemCustom;
							m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetItemUpdate);

							//	Note : �α� ����.
							glItemmt.ReqItemConversion ( pInvenItem->sItemCustom, ID_CHAR, m_dwCharID );
						}
					}
				}
			}
		}

		if ( !vecDELETE.empty() )
		{
			for ( DWORD i=0; i<vecDELETE.size(); ++i )
			{
				const GLInventory::CELL_KEY &cKEY = vecDELETE[i];
				SINVENITEM sINVENITEM = *m_cStorage[idx].GetItem(cKEY.first,cKEY.second);

				SNATIVEID nidITEM = sINVENITEM.sItemCustom.sNativeID;

				//	�ð� �������� ������ ���� �α� ����.
				glItemmt.ReqItemRoute ( sINVENITEM.sItemCustom, ID_CHAR, m_dwCharID, ID_CHAR, 0, EMITEM_ROUTE_DELETE, sINVENITEM.sItemCustom.wTurnNum );

				//	Note : ������ ����.
				//
				m_cStorage[idx].DeleteItem ( cKEY.first, cKEY.second );

				//	[�ڽſ���] �ش� �������� �ι꿡�� ����.
				GLMSG::SNETPC_STORAGE_DELETE NetMsgStorageDel;
				NetMsgStorageDel.dwChannel = idx;
				NetMsgStorageDel.wPosX = cKEY.first;
				NetMsgStorageDel.wPosY = cKEY.second;
				m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgStorageDel);

				//	�ð� �������� ������ ���� �˸�.
				GLMSG::SNET_INVEN_DEL_ITEM_TIMELMT NetMsgInvenDelTimeLmt;
				NetMsgInvenDelTimeLmt.nidITEM = nidITEM;
				m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgInvenDelTimeLmt);

				//	Note : �ڽ��� ����.
				if ( sINVENITEM.sItemCustom.nidDISGUISE!=SNATIVEID(false) )
				{
					SITEM *pONE = glItemMan.GetItem ( sINVENITEM.sItemCustom.nidDISGUISE );
					if ( !pONE )		continue;

					//	�κ� ��ġ.
					WORD wInsertPosX = cKEY.first, wInsertPosY = cKEY.second;

					SITEMCUSTOM sITEM_NEW;
					sITEM_NEW.sNativeID = sINVENITEM.sItemCustom.nidDISGUISE;
					CTime cTIME = CTime::GetCurrentTime();
					if ( sINVENITEM.sItemCustom.tDISGUISE!=0 && pONE->sDrugOp.tTIME_LMT!= 0 )
					{
						cTIME = CTime(sINVENITEM.sItemCustom.tDISGUISE);

						CTimeSpan tLMT(pONE->sDrugOp.tTIME_LMT);
						cTIME -= tLMT;
					}

					//	Note : ������ ����.
					//
					sITEM_NEW.tBORNTIME = cTIME.GetTime();
					sITEM_NEW.wTurnNum = 1;
					sITEM_NEW.cGenType = EMGEN_BILLING;
					sITEM_NEW.cChnID = (BYTE)m_pGLGaeaServer->GetServerChannel();
					sITEM_NEW.cFieldID = (BYTE)m_pGLGaeaServer->GetFieldSvrID();
					sITEM_NEW.lnGenNum = glItemmt.RegItemGen ( sITEM_NEW.sNativeID, (EMITEMGEN)sITEM_NEW.cGenType );

					/*item color, Juver, 2018/01/08 */
					sITEM_NEW.wColor1 = sINVENITEM.sItemCustom.wColor1;
					sITEM_NEW.wColor2 = sINVENITEM.sItemCustom.wColor2;

					//	Note : �κ��� �� ���� �˻�.
					BOOL bOk = m_cStorage[idx].IsInsertable ( pONE->sBasicOp.wInvenSizeX, pONE->sBasicOp.wInvenSizeY, wInsertPosX, wInsertPosY );
					if ( !bOk )		continue;

					//	Note : �κ��� ����.
					m_cStorage[idx].InsertItem ( sITEM_NEW, wInsertPosX, wInsertPosY );
					SINVENITEM *pINSERT_ITEM = m_cStorage[idx].GetItem ( wInsertPosX, wInsertPosY );

					GLMSG::SNETPC_STORAGE_INSERT NetItemInsert;
					NetItemInsert.dwChannel = idx;
					NetItemInsert.Data = *pINSERT_ITEM;
					m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetItemInsert);

					//	Note : �и��� �ڽ��� �α�.
					glItemmt.ReqItemRoute ( pINSERT_ITEM->sItemCustom, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, pINSERT_ITEM->sItemCustom.wTurnNum );
				}
			}
		}
	}
}




// *****************************************************
// Desc: ������ ���� üũ ( �ð����Ѱ� ���� �۵� ) 
// *****************************************************
void GLChar::RESET_CHECK_ITEM ()
{

	// ������ ���� ���� ������ üũ�Ѵ�.
	for ( int i=0; i<SLOT_HOLD; ++i )
	{
		EMSLOT emSLOT = (EMSLOT) i;
		
		if ( m_PutOnItems[emSLOT].sNativeID == NATIVEID_NULL() ) continue;

		SITEMCUSTOM sCUSTOM = m_PutOnItems[emSLOT];

		SITEM *pITEM = GLItemMan::GetInstance().GetItem ( sCUSTOM.sNativeID );
		if ( !pITEM )	continue;

		bool bRelease(false);

		// ���ڽ�Ƭ
		if ( sCUSTOM.nidDISGUISE == NATIVEID_NULL() )
		{
			if ( !SIMPLE_CHECK_ITEM( sCUSTOM.sNativeID ) )	bRelease = true;
		}
		// �ڽ�Ƭ
		else 
		{
			if ( !SIMPLE_CHECK_ITEM ( sCUSTOM.sNativeID, sCUSTOM.nidDISGUISE ) ) bRelease = true;
		}

		if ( pITEM->sSuitOp.emSuit != SLOT_2_SUIT(emSLOT) ) bRelease = true;

		if ( bRelease ) 
		{
			EMSLOT emRHand = GetCurRHand();
			EMSLOT emLHand = GetCurLHand();

			if ( emSLOT == emRHand ) 
			{
				SITEM *pITEM_LEFT=NULL, *pITEM_RIGHT=NULL;
				if ( VALID_SLOT_ITEM(emLHand) )	pITEM_LEFT = GLItemMan::GetInstance().GetItem(GET_SLOT_ITEM(emLHand).sNativeID);
				if ( VALID_SLOT_ITEM(emRHand) )	pITEM_RIGHT = GLItemMan::GetInstance().GetItem(GET_SLOT_ITEM(emRHand).sNativeID);

				if ( (pITEM_RIGHT->sSuitOp.emAttack==ITEMATT_BOW) || 
					(pITEM_RIGHT->sSuitOp.emAttack==ITEMATT_SPEAR) ||

					/*gun-bullet logic, Juver, 2017/05/27 */
					(pITEM_RIGHT->sSuitOp.emAttack==ITEMATT_GUN) || 
					(pITEM_RIGHT->sSuitOp.emAttack==ITEMATT_RAILGUN) ||
					(pITEM_RIGHT->sSuitOp.emAttack==ITEMATT_PORTALGUN) )
				{
					if ( pITEM_LEFT )	//	ȭ��, ���� ������ ��.
					{
						SITEMCUSTOM sITEM_LHAND = GET_SLOT_ITEM(emLHand);

						CItemDrop cDropItem;
						cDropItem.sItemCustom = m_PutOnItems[emLHand];

						if ( IsInsertToInvenEx ( &cDropItem ) )
						{
							InsertToInvenEx( &cDropItem );
						}
						else
						{
							if ( m_pLandMan )
							{
								m_pLandMan->DropItem ( m_vPos, 
														&(cDropItem.sItemCustom), 
														EMGROUP_ONE, 
														m_dwGaeaID );
							}
						}

						//	���� ���� ������.
						RELEASE_SLOT_ITEM(emLHand);

						//	[�ڽſ���] SLOT�� �־��� ������ ����.
						GLMSG::SNETPC_PUTON_RELEASE NetMsg_PutOn_Release(emLHand);
						m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsg_PutOn_Release);				
					}

				}
			}
			
			// Ż�� �����ϰ�� ������ ������ ����
			if ( emSLOT == SLOT_VEHICLE &&  m_sVehicle.IsActiveValue() )
			{
				for ( int i = 0; i < VEHICLE_ACCETYPE_SIZE; ++i )
				{
					if ( m_sVehicle.m_PutOnItems[i].sNativeID == NATIVEID_NULL() ) continue;
					
					// �κ� ����
					CItemDrop cDropItem;
					cDropItem.sItemCustom = m_sVehicle.m_PutOnItems[i];
					
					if ( IsInsertToInvenEx ( &cDropItem ) )
					{
						InsertToInvenEx ( &cDropItem );
					}
					else
					{
						if ( m_pLandMan )
						{
							m_pLandMan->DropItem ( m_vPos, 
													&(cDropItem.sItemCustom), 
													EMGROUP_ONE, 
													m_dwGaeaID );
						}
					}

					m_sVehicle.m_PutOnItems[i].sNativeID = NATIVEID_NULL();
					
					// Ż�� ������ 
					//	[�ڽſ���] �ش� �������� �ι꿡�� ����.
					GLMSG::SNET_VEHICLE_ACCESSORY_DELETE NetMsg;
					NetMsg.dwVehicleNum	= m_sVehicle.m_dwGUID;
					NetMsg.accetype = (VEHICLE_ACCESSORY_TYPE)i;
					m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsg);

					// �ֺ��� �˸�.
					GLMSG::SNET_VEHICLE_ACCESSORY_DELETE_BRD NetMsgBrd;
					NetMsgBrd.dwGaeaID = m_dwGaeaID;
					NetMsgBrd.dwVehicleNum = m_sVehicle.m_dwGUID;
					NetMsgBrd.accetype = (VEHICLE_ACCESSORY_TYPE)i;
					SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&NetMsgBrd) );				
				}

				m_pGLGaeaServer->SaveVehicle( m_dwClientID, m_dwGaeaID, false );
			}
			// Ż�� ������ Ȱ��ȭ ���� �ʾ�����쿡�� ���� ���� �ʴ´�.
			else if ( emSLOT == SLOT_VEHICLE &&  !m_sVehicle.IsActiveValue() )
			{
				continue;
			}

			//	[�ڽſ���] SLOT�� �־��� ������ ����.
			GLMSG::SNETPC_PUTON_RELEASE NetMsg_PutOn_Release(emSLOT);
			m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsg_PutOn_Release);

			//	[��ο���] �ڽ��� ���� ������ �ٲ��� �˸�.
			GLMSG::SNETPC_PUTON_RELEASE_BRD NetMsg_PutOn_Release_Brd;
			NetMsg_PutOn_Release_Brd.dwGaeaID = m_dwGaeaID;
			NetMsg_PutOn_Release_Brd.emSlot = emSLOT;
			SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&NetMsg_PutOn_Release_Brd) );	


			// �κ��� ����
			CItemDrop cDropItem;
			cDropItem.sItemCustom = m_PutOnItems[i];

			if ( IsInsertToInvenEx ( &cDropItem ) )
			{
				InsertToInvenEx ( &cDropItem );
			}
			else
			{
				if ( m_pLandMan )
				{
					m_pLandMan->DropItem ( m_vPos, 
											&(cDropItem.sItemCustom), 
											EMGROUP_ONE, 
											m_dwGaeaID );
				}
			}

			RELEASE_SLOT_ITEM ( emSLOT );			
		}
	}

	// Ż�� �Ǽ����� ���� üũ
	if ( m_sVehicle.IsActiveValue() )
	{
		for ( int i = 0; i < VEHICLE_ACCETYPE_SIZE; ++i )
		{
			if ( m_sVehicle.m_PutOnItems[i].sNativeID == NATIVEID_NULL() ) continue;
			
            if ( !SIMPLE_CHECK_ITEM( m_sVehicle.m_PutOnItems[i].sNativeID ) )
			{
				
				// �κ� ����
				CItemDrop cDropItem;
				cDropItem.sItemCustom = m_sVehicle.m_PutOnItems[i];
				
				if ( IsInsertToInvenEx ( &cDropItem ) )
				{
					InsertToInvenEx ( &cDropItem );
				}
				else
				{
					if ( m_pLandMan )
					{
						m_pLandMan->DropItem ( m_vPos, 
												&(cDropItem.sItemCustom), 
												EMGROUP_ONE, 
												m_dwGaeaID );
					}
				}

				m_sVehicle.m_PutOnItems[i].sNativeID = NATIVEID_NULL();
				
				// Ż�� ������ 
				//	[�ڽſ���] �ش� �������� �ι꿡�� ����.
				GLMSG::SNET_VEHICLE_ACCESSORY_DELETE NetMsg;
				NetMsg.dwVehicleNum	= m_sVehicle.m_dwGUID;
				NetMsg.accetype = (VEHICLE_ACCESSORY_TYPE)i;
				m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsg);

				// �ֺ��� �˸�.
				GLMSG::SNET_VEHICLE_ACCESSORY_DELETE_BRD NetMsgBrd;
				NetMsgBrd.dwGaeaID = m_dwGaeaID;
				NetMsgBrd.dwVehicleNum = m_sVehicle.m_dwGUID;
				NetMsgBrd.accetype = (VEHICLE_ACCESSORY_TYPE)i;
				SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&NetMsgBrd) );				
			}
		}
	}
}

void GLChar::TurnAction ( EMACTIONTYPE toAction )
{
	//	Note : ���� �׼� ���.
	//

	switch ( m_Action )
	{
	case GLAT_IDLE:
		break;

	case GLAT_ATTACK:
		m_fIdleTime = 0.0f;
		break;

	case GLAT_SKILL:
		{
			/*dmk14 skill cut move fix*/
			m_actorMove.Stop ();
			m_fIdleTime = 0.0f;
		}
		break;
	case GLAT_GATHERING:
		break;

		/*push pull skill logic, Juver, 2017/06/05 */
	case GLAT_SKILLMOVE:
	case GLAT_SKILLDASH: /*dash skill logic, Juver, 2017/06/17 */
		{
			if ( toAction == GLAT_DIE || toAction == GLAT_FALLING ){
				if ( m_actorMove.PathIsActive() )		
					m_actorMove.Stop();
			}
		}break;

	default:
		break;
	};

	//CONSOLEMSG_WRITE( "TurnAction [%d]", toAction );

	//	Note : �׼� �ʱ�ȭ.
	//
	m_fIdleTime = 0.0f;
	m_Action = toAction;

    switch ( m_Action )
	{
	case GLAT_IDLE:
		/*dmk14 visual bug fix*/
		if ( m_actorMove.PathIsActive() )	m_actorMove.Stop();
		break;

	case GLAT_MOVE:
		m_wHealingNum = 0;
		break;

	case GLAT_ATTACK:
		m_fattTIMER = 0.0f;
		/*dmk14 visual bug fix*/
		if ( m_actorMove.PathIsActive() )	m_actorMove.Stop();
		break;

	case GLAT_SKILL:
		m_fattTIMER = 0.0f;
		/*dmk14 visual bug fix*/
		if ( m_actorMove.PathIsActive() )	m_actorMove.Stop();
		break;

	case GLAT_SHOCK:
		break;

	case GLAT_PUSHPULL:
	case GLAT_SKILLMOVE: /*push pull skill logic, Juver, 2017/06/05 */
	case GLAT_SKILLDASH: /*dash skill logic, Juver, 2017/06/17 */
		break;

	case GLAT_FALLING:
		TurnAction ( GLAT_DIE );
		break;

	case GLAT_DIE:
		break;
	
	case GLAT_GATHERING:
		break;

	default:
		break;
	};
}

void GLChar::AddPlayHostile ( DWORD dwCHARID, BOOL bBAD, BOOL bClubBattle )
{
	bool bBRIGHTEVENT   = m_pGLGaeaServer->IsBRIGHTEVENT();
	bool bSCHOOL_FREEPK = GLSchoolFreePK::GetInstance().IsON();
	bool bGuidBattleMap = m_pLandMan->m_bGuidBattleMap;
	bool bClubDeathMatch = m_pLandMan->m_bClubDeathMatchMap;

	PGLCHAR pCHAR = m_pGLGaeaServer->GetCharID ( dwCHARID );
	if ( !pCHAR )		return;

	if ( pCHAR->IsSTATE( EM_ACT_DIE ) ) return;
	if ( IsSTATE( EM_ACT_DIE ) ) return;
	
	if ( GetSchool() == pCHAR->GetSchool() )	bSCHOOL_FREEPK = false;

	/*pvp tyranny, Juver, 2017/08/24 */
	/*school wars, Juver, 2018/01/19 */
	/*pvp capture the flag, Juver, 2018/01/31 */
	// DESC : ������ ���� �븸������ �п��� ����PK�� ��� �����ڰ� ���ϱ޼������� �þ��
	//			�Ҿ��� ������ ���� HASH_MAP�� ����ϴ� �� ����Ÿ �����μ��� ����ϱ� �Ұ����ϴ�.			            
	if ( !(bBRIGHTEVENT||bSCHOOL_FREEPK||bGuidBattleMap||bClubDeathMatch||
		m_pLandMan->m_bPVPTyrannyMap||m_pLandMan->m_bPVPSchoolWarsMap||m_pLandMan->m_bPVPCaptureTheFlagMap ) )
	{
		//	Note : ���� ������ ���. ( pk )
		ADD_PLAYHOSTILE ( dwCHARID, bBAD );
	}
	// �п��� ���������� �������̸� ��밡 ���� �б��ϰ�� �������� ����ڷ� �����Ͽ� ������.
	if ( GLSchoolFreePK::GetInstance().IsON() && !bSCHOOL_FREEPK )
	{
		ADD_PLAYHOSTILE ( dwCHARID, bBAD );
	}

	// �п��� ���������� ������ ��밡 ���� 20���� ������ �������� ����ڷ� �����Ͽ� ������.
	if ( bSCHOOL_FREEPK )
	{
		if ( pCHAR->GetLevel() < GLCONST_CHAR::nPK_LIMIT_LEVEL || GetLevel() < GLCONST_CHAR::nPK_LIMIT_LEVEL )
		{
			ADD_PLAYHOSTILE ( dwCHARID, bBAD );
		}
	}

	//	Note : ������ ��� �˸�.
	GLMSG::SNETPC_PLAYERKILLING_ADD NetMsgAdd;
	NetMsgAdd.dwCharID = dwCHARID;
	NetMsgAdd.wSchoolID = pCHAR->m_wSchool;
	NetMsgAdd.bBAD = bBAD;
	NetMsgAdd.bClubBattle = bClubBattle;
	NetMsgAdd.fTime = 0.0f;

	StringCchCopy ( NetMsgAdd.szName, CHAR_SZNAME, pCHAR->m_szName );

	m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgAdd);
}

// *****************************************************
// Desc: ������� ������� Check
// *****************************************************
bool GLChar::IsConflictTarget ( GLACTOR* pTARGET )
{
	//GASSERT(pTARGET&&"GLChar::IsConflictTarget()");
	if ( !pTARGET )				return false;

	EMCROW emCROW = pTARGET->GetCrow();
	if ( emCROW!=CROW_PC )		return false;

	SCONFTING* pCONFTING = pTARGET->GETCONFTING();
	if ( !pCONFTING )			return false;

	if ( m_sCONFTING.IsFIGHTING() && pCONFTING->IsFIGHTING() )
	{
		if ( m_sCONFTING.emTYPE!=pCONFTING->emTYPE )	return false;

		switch ( pCONFTING->emTYPE )
		{
		case EMCONFT_ONE:
			return ( m_dwGaeaID==pCONFTING->dwTAR_ID );

		case EMCONFT_PARTY:
			return ( m_dwPartyID==pCONFTING->dwTAR_ID );

		case EMCONFT_GUILD:
			return ( m_dwGuild==pCONFTING->dwTAR_ID );
		};
	}

	return false;
}

// *****************************************************
// Desc: �׼��� ���� �� �ִ� ������� check
// *****************************************************
bool GLChar::IsReActionable ( GLACTOR* pTARGET, bool benermy )
{
	GASSERT(pTARGET&&"GLChar::IsReActionable()");
	if ( !pTARGET )					return false;
	if ( !m_pLandMan )				return false;

	// �����̺�Ʈ ������
	bool bBRIGHTEVENT = m_pGLGaeaServer->IsBRIGHTEVENT();

	EMCROW emCROW = pTARGET->GetCrow();
	bool bPKZone = m_pLandMan->IsPKZone();
	bool bGuidBattleMap = m_pLandMan->m_bGuidBattleMap;
	bool bClubDeathMatch = m_pLandMan->m_bClubDeathMatchMap;
	bool bSCHOOL_FREEPK = GLSchoolFreePK::GetInstance().IsON();
	bool bClubBattleZone = m_pLandMan->IsClubBattleZone();
	if ( GetSchool() == pTARGET->GetSchool() )	bSCHOOL_FREEPK = false;

	bool bREACTION(true);

	/*pvp capture the flag, Juver, 2018/01/31 */
	BOOL bCaptureTheFlag = m_pLandMan->m_bPVPCaptureTheFlagMap;

	// ��밡 ���� ���
	if ( benermy )
	{
		switch ( emCROW )
		{
		case CROW_PC:
			// Absolute Non-PK State Check.
			if ( GLCONST_CHAR::bPKLESS ) return false;

			// Desc : ���� ������ �ð� (10��)
			if ( !pTARGET->IsSafeTime() && !IsSafeTime() && !pTARGET->IsSafeZone() && !IsSafeZone() )
			{
				// ���� ��Ƽ�� �������
				GLPartyFieldMan& sPartyFieldMan = m_pGLGaeaServer->GetPartyMan();
				GLPARTY_FIELD* sMyParty = sPartyFieldMan.GetParty ( m_dwPartyID );
				if ( sMyParty )
				{
					if ( sMyParty->ISMEMBER ( pTARGET->GetCtrlID() ) )
					{
						return false;
					}
				}

				// ���� ���������
				bREACTION = IsConflictTarget ( pTARGET );
				if ( !bREACTION )
				{
					SCONFTING* pCONFTING = pTARGET->GETCONFTING();
					if ( m_sCONFTING.IsCONFRONTING() || pCONFTING->IsCONFRONTING() )
					{
						return false;
					}
				}
				else
				{
					return true;
				}

				//anticheat red all enemy
				if ( m_pLandMan && m_pLandMan->IsPKMap() && m_pLandMan->IsPKZone() ) return true;

				// ����������� üũ
				DWORD dwCHARID = pTARGET->GetCharID();
				bREACTION = IS_PLAYHOSTILE ( dwCHARID );
				if ( bREACTION ) return true;
	
				// 20 Lev �̻��϶�
				if ( pTARGET->GetLevel() >= GLCONST_CHAR::nPK_LIMIT_LEVEL && GLCONST_CHAR::bPK_MODE && bPKZone)
				{
					// PK ������ ���� ( �б��� Free PK )
					/*pvp capture the flag, Juver, 2018/01/31 */
					if ( bSCHOOL_FREEPK && !bCaptureTheFlag )						return true;
					
					// ���� �̺�Ʈ ���̸�
					/*pvp capture the flag, Juver, 2018/01/31 */
					if ( bBRIGHTEVENT && ( m_dwPartyID != pTARGET->GetPartyID() ) && !bCaptureTheFlag )	return true;
											
					// �׳� ��������
					bREACTION = true;		
				}

				PGLCHAR pChar = m_pGLGaeaServer->GetChar( pTARGET->GetCtrlID() );
				GLClubMan &cClubMan = m_pGLGaeaServer->GetClubMan();
				GLCLUB *pMyClub = cClubMan.GetClub ( m_dwGuild );
				GLCLUB *pTarClub = NULL;
				if ( pChar ) pTarClub = cClubMan.GetClub ( pChar->m_dwGuild );

				// ���������� ���� ���� Ŭ���̸� ���� �Ұ���
				if ( GLCONST_CHAR::bPK_MODE )
				{
					if ( bGuidBattleMap )
					{
						bREACTION = true;

						if ( pMyClub && pTarClub )
						{
							BOOL bAlliance = pMyClub->IsAllianceGuild ( pTarClub->m_dwID );
							if ( bAlliance )	bREACTION = false;
						} // end of if
					}
					else if ( bClubDeathMatch )
					{
						if ( m_dwGuild == pChar->m_dwGuild ) bREACTION = false;
						else bREACTION = true;
					}
					/*pvp tyranny, Juver, 2017/08/24 */
					else if ( m_pLandMan->m_bPVPTyrannyMap )
					{
						if ( m_wSchool == pChar->m_wSchool ) bREACTION = false;
						else bREACTION = true;
					}
					/*school wars, Juver, 2018/01/19 */
					else if ( m_pLandMan->m_bPVPSchoolWarsMap )
					{
						if ( m_wSchool == pChar->m_wSchool ) bREACTION = false;
						else bREACTION = true;
					}
					/*pvp capture the flag, Juver, 2018/01/31 */
					else if ( m_pLandMan->m_bPVPCaptureTheFlagMap )
					{
						if ( m_wCaptureTheFlagTeam == pChar->m_wCaptureTheFlagTeam ) 
							bREACTION = false;
						else 
							bREACTION = true;
					}

					if ( bPKZone && bClubBattleZone )
					{
						if ( pMyClub && pTarClub )
						{
							if ( pMyClub->IsBattle( pTarClub->m_dwID ) ) return true;
							if ( pMyClub->IsBattleAlliance( pTarClub->m_dwAlliance ) ) return true;
						}
					}
				}

			} // end of if
			else
				bREACTION = false;
			break;
		
			/*skill summon, Juver, 2017/10/11 */
		case CROW_SUMMON:
			{
				// Absolute Non-PK State Check.
				if ( GLCONST_CHAR::bPKLESS ) return false;

				PGLSUMMONFIELD psummon_field = GLGaeaServer::GetInstance().GetSummon ( pTARGET->GetCtrlID() );
				if ( psummon_field && psummon_field->m_pOwner )
				{
					//if( !psummon_field->IsTargetReady() )	return false;

					// Desc : ���� ������ �ð� (10��)
					if ( !psummon_field->m_pOwner->IsSafeTime() && !IsSafeTime() && !psummon_field->m_pOwner->IsSafeZone() && !IsSafeZone() )
					{
						// ���� ��Ƽ�� �������
						GLPartyFieldMan& sPartyFieldMan = m_pGLGaeaServer->GetPartyMan();
						GLPARTY_FIELD* sMyParty = sPartyFieldMan.GetParty ( m_dwPartyID );
						if ( sMyParty )
						{
							if ( sMyParty->ISMEMBER ( psummon_field->m_pOwner->GetCtrlID() ) )
							{
								return false;
							}
						}

						// ���� ���������
						bREACTION = IsConflictTarget ( psummon_field->m_pOwner );
						if ( !bREACTION )
						{
							SCONFTING* pCONFTING = psummon_field->m_pOwner->GETCONFTING();
							if ( m_sCONFTING.IsCONFRONTING() || pCONFTING->IsCONFRONTING() )
							{
								return false;
							}
						}
						else
						{
							return true;
						}

						// ����������� üũ
						DWORD dwCHARID = psummon_field->m_pOwner->GetCharID();
						bREACTION = IS_PLAYHOSTILE ( dwCHARID );
						if ( bREACTION ) return true;

						// 20 Lev �̻��϶�
						if ( psummon_field->m_pOwner->GetLevel() >= GLCONST_CHAR::nPK_LIMIT_LEVEL && GLCONST_CHAR::bPK_MODE && bPKZone)
						{
							// PK ������ ���� ( �б��� Free PK )
							/*pvp capture the flag, Juver, 2018/01/31 */
							if ( bSCHOOL_FREEPK && !bCaptureTheFlag )											return true;

							// ���� �̺�Ʈ ���̸�
							/*pvp capture the flag, Juver, 2018/01/31 */
							if ( bBRIGHTEVENT && ( m_dwPartyID != psummon_field->m_pOwner->GetPartyID() ) && !bCaptureTheFlag )	return true;

							// �׳� ��������
							bREACTION = true;		
						}

						//PGLCHAR pChar = m_pGLGaeaServer->GetChar( pTARGET->GetCtrlID() );
						GLClubMan &cClubMan = m_pGLGaeaServer->GetClubMan();
						GLCLUB *pMyClub = cClubMan.GetClub ( m_dwGuild );
						GLCLUB *pTarClub = NULL;
						if ( psummon_field->m_pOwner ) pTarClub = cClubMan.GetClub ( psummon_field->m_pOwner->m_dwGuild );

						// ���������� ���� ���� Ŭ���̸� ���� �Ұ���
						if ( GLCONST_CHAR::bPK_MODE )
						{
							if ( bGuidBattleMap )
							{
								bREACTION = true;

								if ( pMyClub && pTarClub )
								{
									BOOL bAlliance = pMyClub->IsAllianceGuild ( pTarClub->m_dwID );
									if ( bAlliance )	bREACTION = false;
								} // end of if
							}
							else if ( bClubDeathMatch )
							{
								if ( m_dwGuild == psummon_field->m_pOwner->m_dwGuild ) bREACTION = false;
								else bREACTION = true;
							}
							/*pvp tyranny, Juver, 2017/08/24 */
							else if ( m_pLandMan->m_bPVPTyrannyMap )
							{
								if ( m_wSchool == psummon_field->m_pOwner->m_wSchool ) bREACTION = false;
								else bREACTION = true;
							}
							/*school wars, Juver, 2018/01/19 */
							else if ( m_pLandMan->m_bPVPSchoolWarsMap )
							{
								if ( m_wSchool == psummon_field->m_pOwner->m_wSchool ) bREACTION = false;
								else bREACTION = true;
							}
							/*pvp capture the flag, Juver, 2018/01/31 */
							else if ( m_pLandMan->m_bPVPCaptureTheFlagMap )
							{
								if ( m_wCaptureTheFlagTeam == psummon_field->m_pOwner->m_wCaptureTheFlagTeam ) 
									bREACTION = false;
								else 
									bREACTION = true;
							}

							if ( bPKZone && bClubBattleZone )
							{
								if ( pMyClub && pTarClub )
								{
									if ( pMyClub->IsBattle( pTarClub->m_dwID ) ) return true;
									if ( pMyClub->IsBattleAlliance( pTarClub->m_dwAlliance ) ) return true;
								}
							}
						}

					} // end of if
					else
						bREACTION = false;
				}
			}break;

		case CROW_MOB:
			{
				if ( m_sCONFTING.IsCONFRONTING() )	bREACTION = false;
				else if ( IsSafeZone() ) 	bREACTION = false;
				else bREACTION = true;

				/*pvp tyranny, Juver, 2017/08/26 */
				/*dmk14 tower hp recoded
				if ( emCROW == CROW_MOB && m_pLandMan && m_pLandMan->m_bPVPTyrannyMap  )
				{
					PGLCROW pCROW = m_pLandMan->GetCrow( pTARGET->GetCtrlID() );
					if ( pCROW && 
						pCROW->m_wTyrannyOwner != TYRANNY_SCHOOL_SIZE &&
						pCROW->m_wTyrannyOwner == m_wSchool )
					{
						bREACTION = false;
					}
				}
				*/
			}break;

		case CROW_NPC:
			bREACTION = false;
			break;

		case CROW_MATERIAL:
			bREACTION = false;
			break;

		//case CROW_PET:	break;	// PetData

		default:
			bREACTION = false;
			break;
		};
	}
	// ��밡 ���� �ƴ� ���
	else
	{
		switch ( emCROW )
		{
		case CROW_PC:
			{
				//	pc ���� ����� ���� ���� ���� ����ڰ� �ƴ� ���� ����.
				bREACTION = !IsConflictTarget ( pTARGET );

				//	������ ������� ���� �ڱⰡ ������̰� ���� ���϶� ����.
				SCONFTING* pTAR_CONFT = pTARGET->GETCONFTING();
				if ( pTAR_CONFT->IsCONFRONTING() )
				{
					bREACTION = m_sCONFTING.IsSAMENESS(pTAR_CONFT->emTYPE,pTAR_CONFT->dwTAR_ID);
				}

				bool bHOSTILE = IS_PLAYHOSTILE ( pTARGET->GetCharID() );
				if ( bHOSTILE )	bREACTION = false;
			}
			break;
		
		case CROW_SUMMON:
		case CROW_MOB:
			bREACTION = false;
			break;

		case CROW_NPC:
			bREACTION = m_sCONFTING.IsCONFRONTING() ? false : true;
			break;

		case CROW_MATERIAL:
			bREACTION = true;
			break;

		//case CROW_PET:	break;	// PetData

		default:
			bREACTION = false;
			break;
		};
	}

	return bREACTION;
}


inline DWORD GLChar::ToDamage ( const STARGETID &sTargetID, const int nDAMAGE, const BOOL bShock )
{
	GLACTOR* pACTOR_TAR = m_pGLGaeaServer->GetTarget ( m_pLandMan, sTargetID );
	if ( !pACTOR_TAR )	return 0;

	//	Note : PC���� damage�� �ַ��� ����ڿ��� ����.
	//		��� ���� ���� damage ���� ������ ���ؼ�.
	//
	if ( !IsReActionable(pACTOR_TAR) )	return 0;

	return pACTOR_TAR->ReceiveDamage ( GETCROW(), m_dwGaeaID, nDAMAGE, bShock );
}

void GLChar::PreStrikeProc ( BOOL bSkill, BOOL bLowSP )
{
	SANIATTACK sAniAttack;
	STARGETID sTargetID;

	BOOL bCheckHit = FALSE;

	if ( !bSkill )
	{
		EMSLOT emRHand = GetCurRHand();
		EMSLOT emLHand = GetCurLHand();

		EMANI_SUBTYPE emANISUBTYPE = CHECK_ATTACK_ANISUB ( m_pITEMS[emRHand], m_pITEMS[emLHand] );

		GLCONST_CHARCLASS &ConstCharClass = GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX];
		VECANIATTACK &vecAniAttack = ConstCharClass.m_ANIMATION[AN_ATTACK][emANISUBTYPE];
		if ( vecAniAttack.empty() )	return; // FALSE;

		//	Note : Ư�� ���� ���ϸ��̼� ���� ����.
		//
		sAniAttack = vecAniAttack[0];

		sTargetID = m_TargetID;
		bCheckHit = TRUE;
	}
	else
	{
		//	Note : ��ų ���� ������.
		//
		PGLSKILL pSkill = NULL;
		pSkill = GLSkillMan::GetInstance().GetData ( m_idACTIVESKILL.wMainID, m_idACTIVESKILL.wSubID );

		GLCONST_CHARCLASS &ConstCharClass = GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX];
		VECANIATTACK &vecAniAttack = ConstCharClass.m_ANIMATION[pSkill->m_sEXT_DATA.emANIMTYPE][pSkill->m_sEXT_DATA.emANISTYPE];
		if ( vecAniAttack.empty() )	return; // FALSE;

		//	Note : Ư�� ���� ���ϸ��̼� ���� ����.
		//
		sAniAttack = vecAniAttack[0];

		if ( pSkill->m_sBASIC.emAPPLY != SKILL::EMAPPLY_MAGIC )
		{
			//	Note : ��ų Ÿ���� �ϳ��� ��ȿ�ϰ� Ȯ���� �ϰ� ����.
			//	NEED : ���� Ÿ���� ��� �ٽ� Ȯ���ؾ���.
			sTargetID = STARGETID(static_cast<EMCROW>(m_sTARIDS[0].wCrow),static_cast<DWORD>(m_sTARIDS[0].wID));
			bCheckHit = TRUE;
		}
		else
		{
			sTargetID.dwID = EMTARGET_NULL;
		}
	}

	//	ť �ʱ�ȭ
	m_sHITARRAY.clear ();

	for ( int i = 0; i < sAniAttack.m_wDivCount; i++ )
	{
		bool bhit = true;
		if ( bCheckHit && (sTargetID.dwID!=EMTARGET_NULL) )
		{
			if ( !CHECKHIT(sTargetID,m_pLandMan,bLowSP) )		bhit = false;
		}

		m_sHITARRAY.push_back ( SSTRIKE(sAniAttack.m_wDivFrame[i],bhit,cast_bool(bLowSP)) );
	}
}

void GLChar::AvoidProc ( const SNATIVEID skill_id, const BOOL bLowSP )
{
	//CONSOLEMSG_WRITE( "AvoidProc Start [%u]", skill_id.dwID );

	if ( skill_id==NATIVEID_NULL() )
	{
		if ( !bLowSP )
		{
			WORD wDisSP = m_wSUM_DisSP + GLCONST_CHAR::wBASIC_DIS_SP;
			m_sSP.DECREASE ( wDisSP );
		}

		GLACTOR *pACTOR = m_pGLGaeaServer->GetTarget ( m_pLandMan, m_TargetID );
		if ( pACTOR )	pACTOR->ReceiveDamage ( CROW_PC, m_dwGaeaID, 0, FALSE);
	}
	else
	{
		//	Note : SP�� �������� ���������� SP �Ҹ��Ŵ.
		//
		if ( !bLowSP )
		{
			//	Note : ĳ���Ͱ� ��� ��ų ���� ������.
			SKILL_MAP_ITER learniter = m_ExpSkills.find ( skill_id.dwID );
			if( learniter==m_ExpSkills.end() )								return;	// ��� ��ų�� �ƴ� ���.
			SCHARSKILL &sSkill = (*learniter).second;

			//	��ų ���� ������.
			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( skill_id.wMainID, skill_id.wSubID );
			if ( !pSkill )													return;
			SKILL::CDATA_LVL &sSKILL_DATA = pSkill->m_sAPPLY.sDATA_LVL[sSkill.wLevel];

			WORD wDisSP = m_wSUM_DisSP + sSKILL_DATA.wUSE_SP;
			m_sSP.DECREASE ( wDisSP );
		}

		for ( WORD i=0; i<m_wTARNUM; ++i )
		{
			STARGETID sTargetID ( (EMCROW)m_sTARIDS[i].wCrow, (WORD)m_sTARIDS[i].wID );

			GLACTOR *pACTOR = m_pGLGaeaServer->GetTarget ( m_pLandMan, sTargetID );
			if ( pACTOR )	pACTOR->ReceiveDamage ( CROW_PC, m_dwGaeaID, 0, FALSE);
		}
	}

	//	Note : �ڽſ���.
	GLMSG::SNETPC_ATTACK_AVOID NetMsg;
	NetMsg.emTarCrow	= m_TargetID.emCrow;
	NetMsg.dwTarID		= m_TargetID.dwID;

	m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsg );

	//	Note : �ֺ� Ŭ���̾�Ʈ�鿡�� �޼��� ����.
	//
	GLMSG::SNETPC_ATTACK_AVOID_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID	= m_dwGaeaID;
	NetMsgBrd.emTarCrow	= m_TargetID.emCrow;
	NetMsgBrd.dwTarID	= m_TargetID.dwID;

	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
}

void GLChar::DamageProc ( int nDAMAGE, const DWORD dwDamageFlag, const BOOL bLowSP )
{
	//	Note : ������ ����.
	//				( sp ������ ������ ����. )
	float	fDAMAGE_RATE(1.0f);
	if ( bLowSP )						fDAMAGE_RATE *= (1-GLCONST_CHAR::fLOWSP_DAMAGE);

	nDAMAGE = int(nDAMAGE*fDAMAGE_RATE);
	if ( nDAMAGE < 1 )	nDAMAGE = 1;

	// ��� ������ ����Ÿ���̸� ���� ����
	PGLCHAR pCHAR = m_pGLGaeaServer->GetChar ( m_TargetID.dwID );
	if ( pCHAR )
	{
		if ( pCHAR->m_sCONFTING.IsPOWERFULTIME () )
			return;
	}

	//	Note : SP �Ҹ�.
	//
	if ( !bLowSP )
	{
		WORD wDisSP = m_wSUM_DisSP + GLCONST_CHAR::wBASIC_DIS_SP;
		m_sSP.DECREASE ( wDisSP );

		GLMSG::SNETPC_UPDATE_SP NetMsg;
		NetMsg.wNowSP = m_sSP.dwNow;
		m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsg );
	}

	if ( m_TargetID.emCrow==CROW_PC )
	{
		nDAMAGE = int(nDAMAGE*GLCONST_CHAR::fPK_POINT_DEC_PHY);
		if ( nDAMAGE==0 )	nDAMAGE = 1;
	}

	//	Note : ����ڿ��� ������� ����.
	//
	bool bShock = ( dwDamageFlag & DAMAGE_TYPE_SHOCK );
	ToDamage ( m_TargetID, nDAMAGE, bShock );

	//	Note : �ڽſ���.
	GLMSG::SNETPC_ATTACK_DAMAGE NetMsg;
	NetMsg.emTarCrow		= m_TargetID.emCrow;
	NetMsg.dwTarID			= m_TargetID.dwID;
	NetMsg.nDamage			= nDAMAGE;
	NetMsg.dwDamageFlag		= dwDamageFlag;
	m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsg );

	//	Note : �ֺ� Ŭ���̾�Ʈ�鿡�� �޼��� ����.
	//
	GLMSG::SNETPC_ATTACK_DAMAGE_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID		= m_dwGaeaID;
	NetMsgBrd.emTarCrow		= m_TargetID.emCrow;
	NetMsgBrd.dwTarID		= m_TargetID.dwID;
	NetMsgBrd.nDamage		= nDAMAGE;
	NetMsgBrd.dwDamageFlag	= dwDamageFlag;
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

	GLACTOR* pACTOR = m_pGLGaeaServer->GetTarget ( m_pLandMan, m_TargetID );
	if ( !pACTOR )	return;

	EMSLOT emRHand = GetCurRHand();
	EMSLOT emLHand = GetCurLHand();

	//	Note : ���� �̻� ����.
	//
	SITEM* pRHandItem = GET_SLOT_ITEMDATA(emRHand);
	SITEM* pLHandItem = GET_SLOT_ITEMDATA(emLHand);

	if ( pRHandItem && pRHandItem->sSuitOp.sBLOW.emTYPE!=EMBLOW_NONE )
	{
		ITEM::SSATE_BLOW &sBlow = pRHandItem->sSuitOp.sBLOW;

		//	Note : �߻� Ȯ�� ���.
		//
		short nBLOWRESIST = pACTOR->GETRESIST().GetElement ( STATE_TO_ELEMENT(sBlow.emTYPE) );
		if ( nBLOWRESIST>GLCONST_CHAR::fRESIST_G )	nBLOWRESIST = GLCONST_CHAR::fRESIST_G;

//		float fPOWER = GLOGICEX::WEATHER_BLOW_POW ( sBlow.emTYPE, GLPeriod::GetInstance().GetWeather(), m_pLandMan->IsWeatherActive() );
		DWORD dwWeather = GLPeriod::GetInstance().GetMapWeather( m_pLandMan->GetMapID().wMainID, m_pLandMan->GetMapID().wSubID );
		float fPOWER = GLOGICEX::WEATHER_BLOW_POW ( sBlow.emTYPE, dwWeather, m_pLandMan->IsWeatherActive() );

		BOOL bBLOW = FALSE;
		if ( !(pACTOR->GETHOLDBLOW()&STATE_TO_DISORDER(sBlow.emTYPE)) )
			bBLOW = GLOGICEX::CHECKSTATEBLOW ( sBlow.fRATE*fPOWER, GETLEVEL(), pACTOR->GetLevel(), nBLOWRESIST );

		if ( bBLOW )
		{
			//	Note : �����̻� �߻�.
			//	
			SSTATEBLOW sSTATEBLOW;
			float fLIFE = sBlow.fLIFE * fPOWER;
			fLIFE = ( fLIFE - (fLIFE*nBLOWRESIST/100.0f*GLCONST_CHAR::fRESIST_G ) );

			sSTATEBLOW.emBLOW = sBlow.emTYPE;
			sSTATEBLOW.fAGE = fLIFE;
			sSTATEBLOW.fSTATE_VAR1 = sBlow.fVAR1;
			sSTATEBLOW.fSTATE_VAR2 = sBlow.fVAR2;

			pACTOR->STATEBLOW ( sSTATEBLOW );

			//	Note : �����̻� �߻� Msg.
			//
			GLMSG::SNETPC_STATEBLOW_BRD NetMsgState;
			NetMsgState.emCrow = m_TargetID.emCrow;
			NetMsgState.dwID = m_TargetID.dwID;
			NetMsgState.emBLOW = sBlow.emTYPE;
			NetMsgState.fAGE = fLIFE;
			NetMsgState.fSTATE_VAR1 = sBlow.fVAR1;
			NetMsgState.fSTATE_VAR2 = sBlow.fVAR2;

			//	Note : '��ų���'�� �ֺ� Ŭ���̾�Ʈ�鿡�� �޼��� ����.
			//
			pACTOR->SNDMSGAROUND ( (NET_MSG_GENERIC*) &NetMsgState );

			//	Note : ��ų ��󿡰� �޼��� ����.
			if ( m_TargetID.emCrow==CROW_PC )	m_pGLGaeaServer->SENDTOCLIENT ( pACTOR->GETCLIENTID (), &NetMsgState );
		}
	}
	else if ( pLHandItem && pLHandItem->sSuitOp.sBLOW.emTYPE!=EMBLOW_NONE )
	{
		ITEM::SSATE_BLOW &sBlow = pLHandItem->sSuitOp.sBLOW;

		//	Note : �߻� Ȯ�� ���.
		//
		short nBLOWRESIST = pACTOR->GETRESIST().GetElement ( STATE_TO_ELEMENT(sBlow.emTYPE) );
		if ( nBLOWRESIST>GLCONST_CHAR::fRESIST_G )	nBLOWRESIST = GLCONST_CHAR::fRESIST_G;

//		float fPOWER = GLOGICEX::WEATHER_BLOW_POW ( sBlow.emTYPE, GLPeriod::GetInstance().GetWeather(), m_pLandMan->IsWeatherActive() );
		DWORD dwWeather = GLPeriod::GetInstance().GetMapWeather( m_pLandMan->GetMapID().wMainID, m_pLandMan->GetMapID().wSubID );
		float fPOWER = GLOGICEX::WEATHER_BLOW_POW ( sBlow.emTYPE, dwWeather, m_pLandMan->IsWeatherActive() );

		BOOL bBLOW = FALSE;
		if ( !(pACTOR->GETHOLDBLOW()&STATE_TO_DISORDER(sBlow.emTYPE)) )
			bBLOW = GLOGICEX::CHECKSTATEBLOW ( sBlow.fRATE*fPOWER, GETLEVEL(), pACTOR->GetLevel(), nBLOWRESIST );

		if ( bBLOW )
		{
			//	Note : �����̻� �߻�.
			//	 
			SSTATEBLOW sSTATEBLOW;
			float fLIFE = sBlow.fLIFE * fPOWER;
			fLIFE = ( fLIFE - (fLIFE*nBLOWRESIST/100.0f*GLCONST_CHAR::fRESIST_G ) );

			sSTATEBLOW.emBLOW = sBlow.emTYPE;
			sSTATEBLOW.fAGE = fLIFE;
			sSTATEBLOW.fSTATE_VAR1 = sBlow.fVAR1;
			sSTATEBLOW.fSTATE_VAR2 = sBlow.fVAR2;

			pACTOR->STATEBLOW ( sSTATEBLOW );

			//	Note : �����̻� �߻� Msg.
			//
			GLMSG::SNETPC_STATEBLOW_BRD NetMsgState;
			NetMsgState.emCrow = m_TargetID.emCrow;
			NetMsgState.dwID = m_TargetID.dwID;
			NetMsgState.emBLOW = sBlow.emTYPE;
			NetMsgState.fAGE = fLIFE;
			NetMsgState.fSTATE_VAR1 = sBlow.fVAR1;
			NetMsgState.fSTATE_VAR2 = sBlow.fVAR2;

			//	Note : '��ų���'�� �ֺ� Ŭ���̾�Ʈ�鿡�� �޼��� ����.
			//
			pACTOR->SNDMSGAROUND ( (NET_MSG_GENERIC*) &NetMsgState );

			//	Note : ��ų ��󿡰� �޼��� ����.
			if ( m_TargetID.emCrow==CROW_PC )	m_pGLGaeaServer->SENDTOCLIENT ( pACTOR->GETCLIENTID (), &NetMsgState );
		}
	}

	// ���� Ÿ��
	if ( dwDamageFlag & DAMAGE_TYPE_CRUSHING_BLOW ) 
	{
		STARGETID dwActorID = STARGETID(CROW_PC,m_dwGaeaID);
		m_pLandMan->PushPullAct( dwActorID, m_TargetID, GLCONST_CHAR::fCRUSH_BLOW_RANGE );
	}


	/*curse skill logic, Juver, 2017/06/11 */
	if ( m_pLandMan && m_bCurse && m_fCurseDamage != 0.0f )
	{
		int _nCurseDamage = int(nDAMAGE*m_fCurseDamage);

		SSKILLACT sSKILLACTCURSE;
		sSKILLACTCURSE.sID = m_TargetID;
		sSKILLACTCURSE.sID_TAR = STARGETID(CROW_PC,m_dwGaeaID);
		sSKILLACTCURSE.fDELAY = 0.2f;
		sSKILLACTCURSE.emAPPLY = SKILL::EMAPPLY_PHY_LONG;
		sSKILLACTCURSE.nVAR_HP = -_nCurseDamage;
		sSKILLACTCURSE.dwDamageFlag = DAMAGE_TYPE_NONE;		

		if ( sSKILLACTCURSE.VALID() )
		{
			m_pLandMan->RegSkillAct ( sSKILLACTCURSE );
		}
	}

}

void GLChar::DamageReflectionProc ( int nDAMAGE, STARGETID sACTOR )
{
	/*block back damage, Juver, 2017/12/12 */
	if ( RANPARAM::bFeatureBlockDamageBack && m_pLandMan )
	{
		GLACTOR* pactor_target = GLGaeaServer::GetInstance().GetTarget( m_pLandMan, sACTOR );
		if ( pactor_target )
		{
			if ( pactor_target->IsBlockDamageBack() )	return;
			pactor_target->SetBlockDamageBack( true );
		}
	}

	if ( sACTOR.emCrow==CROW_PC )
	{
		nDAMAGE = int(nDAMAGE*GLCONST_CHAR::fPK_POINT_DEC_PHY);
		if ( nDAMAGE==0 )	nDAMAGE = 1;
	}

	ToDamage ( sACTOR, nDAMAGE, FALSE );

	//	Note : �ڽſ���.
	GLMSG::SNETPC_ATTACK_DAMAGE NetMsg;
	NetMsg.emTarCrow		= sACTOR.emCrow;
	NetMsg.dwTarID			= sACTOR.dwID;
	NetMsg.nDamage			= nDAMAGE;
	NetMsg.dwDamageFlag		= DAMAGE_TYPE_NONE;
	m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsg );

	//	Note : �ֺ� Ŭ���̾�Ʈ�鿡�� �޼��� ����.
	//
	GLMSG::SNETPC_ATTACK_DAMAGE_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID		= m_dwGaeaID;
	NetMsgBrd.emTarCrow		= sACTOR.emCrow;
	NetMsgBrd.dwTarID		= sACTOR.dwID;
	NetMsgBrd.nDamage		= nDAMAGE;
	NetMsgBrd.dwDamageFlag	= DAMAGE_TYPE_NONE;
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
}

void GLChar::DefenseSkill( SNATIVEID sNativeID, WORD wLevel, STARGETID sACTOR )
{
	if ( sNativeID == NATIVEID_NULL() ) return;

	GLMSG::SNETPC_DEFENSE_SKILL_ACTIVE NetMsg;
	NetMsg.emTarCrow = sACTOR.emCrow;
	NetMsg.dwTarID = sACTOR.dwID;
	NetMsg.sNativeID = sNativeID;
	NetMsg.wLevel = wLevel;
	m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsg );

	m_bDefenseSkill	= true;

    return;
}

BOOL GLChar::AttackProcess ( float fElapsedTime )
{
	m_fattTIMER += fElapsedTime*GLCHARLOGIC::GETATTVELO();
	m_fattTIMER += GLCHARLOGIC::GETATT_ITEM();

	GLCONST_CHARCLASS &ConstCharClass = GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX];

	EMSLOT emRHand = GetCurRHand();
	EMSLOT emLHand = GetCurLHand();

	EMANI_SUBTYPE emANISUBTYPE = CHECK_ATTACK_ANISUB ( m_pITEMS[emRHand], m_pITEMS[emLHand] );
	VECANIATTACK &vecAniAttack = ConstCharClass.m_ANIMATION[AN_ATTACK][emANISUBTYPE];
	if ( vecAniAttack.empty() )	return FALSE;

	//	Note : ���� ��� ������ ����� ������ ����. ���� ���. ( ��� ��Ź. )
	//
	if ( vecAniAttack.size() <= m_dwANISUBSELECT )
	{
		TurnAction ( GLAT_IDLE );

		//GLMSG::SNET_ACTION_BRD NetMsgBrd;
		//NetMsgBrd.emCrow = CROW_PC;
		//NetMsgBrd.dwID = m_dwGaeaID;
		//NetMsgBrd.emAction = GLAT_IDLE;
		//m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsgBrd );

		//SendMsgViewAround ( (NET_MSG_GENERIC *) &NetMsgBrd );

		return FALSE;
	}

	if ( !ISLONGRANGE_ARMS() )
	{
		GLACTOR *pTARGET = m_pGLGaeaServer->GetTarget ( m_pLandMan, m_TargetID );
		if ( !pTARGET )
		{
			TurnAction ( GLAT_IDLE );

			//	Note : �ڽſ���.
			GLMSG::SNETPC_ATTACK_DAMAGE NetMsg;
			NetMsg.emTarCrow		= m_TargetID.emCrow;
			NetMsg.dwTarID			= m_TargetID.dwID;
			NetMsg.nDamage			= 0;
			m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsg );
			SendMsgViewAround ( (NET_MSG_GENERIC *) &NetMsg );

			////	Note : ��� ����.
			//GLMSG::SNET_ACTION_BRD NetMsgBrd;
			//NetMsgBrd.emCrow = CROW_PC;
			//NetMsgBrd.dwID = m_dwGaeaID;
			//NetMsgBrd.emAction = GLAT_IDLE;
			//m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsgBrd );

			////	Note : ��� ���� (��ο���).
			//SendMsgViewAround ( (NET_MSG_GENERIC *) &NetMsgBrd );

			return FALSE;
		}

		D3DXVECTOR3 vDist = m_vPos - pTARGET->GetPosition();
		float fTarLength = D3DXVec3Length(&vDist);

		WORD wAttackRange = pTARGET->GetBodyRadius() + GETBODYRADIUS() + GETATTACKRANGE() + 2;
		if ( wAttackRange*GLCONST_CHAR::fREACT_VALID_SCALE < (WORD)(fTarLength) )
		{
			TurnAction ( GLAT_IDLE );

			//	Note : �ڽſ���.
			GLMSG::SNETPC_ATTACK_DAMAGE NetMsg;
			NetMsg.emTarCrow		= m_TargetID.emCrow;
			NetMsg.dwTarID			= m_TargetID.dwID;
			NetMsg.nDamage			= 0;
			m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsg );
			SendMsgViewAround ( (NET_MSG_GENERIC *) &NetMsg );

			////	Note : ��� ����.
			//GLMSG::SNET_ACTION_BRD NetMsgBrd;
			//NetMsgBrd.emCrow = CROW_PC;
			//NetMsgBrd.dwID = m_dwGaeaID;
			//NetMsgBrd.emAction = GLAT_IDLE;
			//m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsgBrd );

			////	Note : ��� ���� (��ο���).
			//SendMsgViewAround ( (NET_MSG_GENERIC *) &NetMsgBrd );

			return FALSE;
		}
	}


	//	Note : Ư�� ���� ���ϸ��̼� ���� ����.
	//
	const SANIATTACK &sAniAttack = vecAniAttack[m_dwANISUBSELECT];

	DWORD dwThisKey = sAniAttack.m_dwSTime + DWORD(m_fattTIMER*UNITANIKEY_PERSEC);
	
	if ( !m_sHITARRAY.empty() )
	{
		SSTRIKE sStrike = *m_sHITARRAY.begin();

		if ( dwThisKey >= sStrike.wDivKey )
		{
			if ( sStrike.bHit )
			{
				int nDamage = 0;
				DWORD dwDamageFlag = DAMAGE_TYPE_NONE;
				dwDamageFlag = CALCDAMAGE ( nDamage, m_dwGaeaID, m_TargetID, m_pLandMan, NULL, 0, NULL, sAniAttack.m_wDivCount );

				DamageProc ( nDamage, dwDamageFlag, sStrike.bLowSP );
			}
			else
			{
				AvoidProc ( NATIVEID_NULL(), sStrike.bLowSP );
			}

			//	ť ����Ÿ�� ó���� ��� �����Ѵ�.
			m_sHITARRAY.pop_front ();
		}	
	}

	//	�̻����� ���� ����Ÿ�� ���� ó������ �������,
	//	�������� �Ѳ����� ó���ϰ�, FALSE�� �����Ѵ�.
	if ( dwThisKey >= sAniAttack.m_dwETime )
	{
		if ( !m_sHITARRAY.empty() )
		{
			while ( !m_sHITARRAY.empty() )
			{
				SSTRIKE sStrike = *m_sHITARRAY.begin();
				
				if ( sStrike.bHit )
				{
					int nDamage = 0;
					DWORD dwDamageFlag = DAMAGE_TYPE_NONE;
					dwDamageFlag = CALCDAMAGE ( nDamage, m_dwGaeaID, m_TargetID, m_pLandMan, NULL, 0, NULL, sAniAttack.m_wDivCount );
					DamageProc ( nDamage, dwDamageFlag, sStrike.bLowSP );
				}
				else
				{
					AvoidProc ( NATIVEID_NULL(), sStrike.bLowSP );
				}

				//	ť ����Ÿ�� ó���� ��� �����Ѵ�.
				m_sHITARRAY.pop_front ();
			}
		}

		return FALSE;
	}

	//	Note : ���� ������ ���� ���� �ʾ����� hit�� �Ϸ�̰� ����Ű�� ���� ���� ������ ���
	//		������ ����Ȱɷ� ����. ( Ŭ���̾�Ʈ�� �������� ������ ����. )
	if ( m_sHITARRAY.empty() )
	{
		//DWORD dwDISKEY = sAniAttack.m_dwETime - dwThisKey;
		//if ( dwDISKEY <= 5 )		return FALSE;

		return FALSE;
	}

	return TRUE;
}

// *****************************************************
// Desc: ��Ȱ��ų �õ�
// *****************************************************
WORD GLChar::DoRevive2Skill ( SNATIVEID skill_id, WORD wSKILL_LVL, WORD wSPEC )
{
	if ( IsValidBody() )					return 0;
	if ( SKILL::MAX_LEVEL <= wSKILL_LVL )	return 0;
	if ( !IsSTATE(EM_ACT_DIE) )				return 0;
	if ( wSPEC >= SKILL::MAX_SPEC )			return 0;

	//	��ų ���� ������.
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( skill_id );
	const SKILL::SSPEC &sSPEC = pSkill->m_sAPPLY.sSpecs[wSPEC].sSPEC[wSKILL_LVL];

    //	Note : ��Ȱ map�� ��Ȱ gate ����, ��ġ�� �ǹ� ���� ��.
	BOOL bOk = m_pGLGaeaServer->RequestReBirth ( m_dwGaeaID, m_pLandMan->GetMapID(), UINT_MAX, m_vPos );
	if ( !bOk )
	{
		DEBUGMSG_WRITE ( "ĳ���� ��Ȱ �õ��߿� ������ �߻�! m_pGLGaeaServer->RequestReBirth ()" );
		return 0;
	}

	//	��Ȱ �Ϸ� üũ.
	ReSetSTATE(EM_ACT_DIE);

	//	��Ȱ�� ä�� ȸ��.
	m_fGenAge = 0.0f;
	GLCHARLOGIC::INIT_RECOVER(int(sSPEC.fVAR1));
	TurnAction ( GLAT_IDLE );

	//	Note : ��Ȱ�� ����ġ ����.
	ReBirthDecExp ();

	// PET
	// ���̵��� Pet �� Vehicle ����
	m_pGLGaeaServer->ReserveDropOutPet ( SDROPOUTPETINFO(m_dwPetGUID,true,true) );
	//m_pGLGaeaServer->ReserveDropOutSummon ( SDROPOUTSUMMONINFO(m_dwSummonGUID,true) );
	m_pGLGaeaServer->SaveVehicle( m_dwClientID, m_dwGaeaID, true );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		m_pGLGaeaServer->ReserveDropOutSummon ( m_dwSummonGUID_FLD[i] );

	m_sHP.TO_FULL();
	m_sMP.TO_FULL();
	m_sSP.TO_FULL();

	//	��Ȱ Ȯ�� �޽���.
	GLMSG::SNETPC_REQ_REBIRTH_FB NetMsg;
	NetMsg.sMapID = m_pLandMan->GetMapID();
	NetMsg.vPos = GetPosition();
	NetMsg.dwNowHP = m_sHP.dwNow;
	NetMsg.dwNowMP = m_sMP.dwNow;
	NetMsg.dwNowSP = m_sSP.dwNow;

	m_pGLGaeaServer->SENDTOAGENT(m_dwClientID,&NetMsg);
	return m_sHP.dwNow;
}

// *****************************************************
// Desc: ��ų ������ �ɸ��Ϳ� �����Ų��.
// *****************************************************
void GLChar::SkillProc ( SNATIVEID skill_id, BOOL bLowSP )
{
	//CONSOLEMSG_WRITE( "SkillProc Start [%u]", skill_id.dwID );

	//	Note : ĳ���Ͱ� ��� ��ų ���� ������.
	//
	SCHARSKILL sSkill;
	WORD wSKILL_LVL = 0;

	if ( IsDefenseSkill() )
	{
		if ( skill_id != m_sDefenseSkill.m_dwSkillID ) return;
		sSkill.sNativeID = skill_id;
		sSkill.wLevel = m_sDefenseSkill.m_wLevel;
		wSKILL_LVL = m_sDefenseSkill.m_wLevel;;

	}
	else
	{
		SKILL_MAP_ITER learniter = m_ExpSkills.find ( skill_id.dwID );
		if ( learniter==m_ExpSkills.end() )	return;
		sSkill = (*learniter).second;
		wSKILL_LVL = sSkill.wLevel;
	}

	//	Note : ���ϸ��̼� ����.
	GLCONST_CHARCLASS &ConstCharClass = GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX];
	VECANIATTACK &vecAniAttack = ConstCharClass.m_ANIMATION[m_SKILLMTYPE][m_SKILLSTYPE];
	if ( vecAniAttack.empty() )	return;

	//	Note : Ư�� ���� ���ϸ��̼� ���� ����. (������)
	//
	const SANIATTACK &sAniAttack = vecAniAttack[0];

	//	��ų ���� ������.
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( skill_id );
	SKILL::CDATA_LVL &sSKILL_DATA = pSkill->m_sAPPLY.sDATA_LVL[wSKILL_LVL];

	//	Note : SP�� �������� ���������� SP �Ҹ��Ŵ.
	//
	if ( !bLowSP )
	{
		WORD wDisSP = m_wSUM_DisSP + sSKILL_DATA.wUSE_SP;
		m_sSP.DECREASE ( wDisSP );
	}

	int nEXP_NUM = 0;
	int nALL_LVL = 0;
	int nALL_VARHP = 0, nALL_FULLHP = 0;
	int nALL_VARMP = 0, nALL_FULLMP = 0;
	int nALL_VARSP = 0, nALL_FULLSP = 0;

	//	Note : ���� ������ �߻�.
	//
	DWORD dwDamageFlag = DAMAGE_TYPE_NONE;

	int nVAR_HP(0), nVAR_MP(0), nVAR_SP(0);

	//	Note : ��ų Ư�� ���.
	//
	int nGATHER_HP(0), nGATHER_MP(0), nGATHER_SP(0);

	WORD wMAX_TAR = 0;
	if ( pSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF_TOSPEC )			wMAX_TAR = sSKILL_DATA.wPIERCENUM + GETSUM_PIERCE() + 1;
	else if ( pSkill->m_sBASIC.emIMPACT_REALM == REALM_FANWIZE )	wMAX_TAR = sSKILL_DATA.wTARNUM * ( sSKILL_DATA.wPIERCENUM + GETSUM_PIERCE() + 1 );
	else															wMAX_TAR = sSKILL_DATA.wTARNUM;

	if( pSkill->m_sBASIC.emIMPACT_SIDE == SIDE_ENEMY )	WeaponSkillProc();

	if ( m_wTARNUM > wMAX_TAR )		m_wTARNUM = wMAX_TAR;

	/*dash skill logic, Juver, 2017/06/17 */
	BOOL bACTDASH = FALSE;

	//	Note : ��ų�� ����Ǵ� Ÿ�� �˻�.
	//
	BOOL bAsignStigma = false;
	BOOL bStigmaEff = FALSE;
	STARGETID sTARID;
	for ( WORD i=0; i<m_wTARNUM; ++i )
	{
		const STARID &_sTARID = m_sTARIDS[i];

		sTARID.emCrow = _sTARID.GETCROW();
		sTARID.dwID = _sTARID.GETID();
		GLACTOR* pACTOR = m_pGLGaeaServer->GetTarget ( m_pLandMan, sTARID );
		if ( !pACTOR )									continue;

		//DAMAGE_SPEC*	pDamageSpec = pACTOR->GetDamageSpec();
		DAMAGE_SPEC	pDamageSpec = pACTOR->GetDamageSpec();
		if ( CHECK_FLAG_SKILL(pDamageSpec.m_emImmuneActionType, pSkill->m_sBASIC.emACTION) ||
			CHECK_FLAG_SKILL(pDamageSpec.m_emImmuneApplyType, pSkill->m_sBASIC.emAPPLY))
		{
			AvoidProc();
			return;
		}

		bool bCrushBlowOne = false;


		for ( WORD j=0; j<sSKILL_DATA.wAPPLYNUM; ++j )
		{
			if ( pACTOR->GetNowHP()==0 )				continue;

			//	����ġ.
			short nRESIST = pACTOR->GETRESIST().GetElement(pSkill->m_sAPPLY.emELEMENT);
			if ( nRESIST>GLCONST_CHAR::fMAX_RESIST )	nRESIST = GLCONST_CHAR::fMAX_RESIST;

			//	SKILL �⺻ ����.
			//
			switch ( pSkill->m_sAPPLY.emBASIC_TYPE )
			{
			case SKILL::EMFOR_HP:
				//	��󿡰� ���ظ� �ִ� ��ų.
				if ( sSKILL_DATA.fBASIC_VAR < 0.0f )
				{
//					bCRITICAL = CALCDAMAGE ( nVAR_HP, bShock, sTARID, m_pLandMan, pSkill, wSKILL_LVL, GLPeriod::GetInstance().GetWeather(), sAniAttack.m_wDivCount );
					DWORD dwWeather = GLPeriod::GetInstance().GetMapWeather( m_pLandMan->GetMapID().wMainID, m_pLandMan->GetMapID().wSubID );
					dwDamageFlag = CALCDAMAGE ( nVAR_HP, m_dwGaeaID, sTARID, m_pLandMan, pSkill, wSKILL_LVL, dwWeather, sAniAttack.m_wDivCount );
					nVAR_HP = - nVAR_HP;
				}
				//	����� ȸ������ �ִ� ��ų.
				else
				{
					int nDX = pACTOR->GetMaxHP() - pACTOR->GetNowHP();
					nVAR_HP += (int) min(nDX,sSKILL_DATA.fBASIC_VAR);
				}
				break;

			case SKILL::EMFOR_MP:
				//	��󿡰� ���ظ� �ִ� ��ų.
				if ( sSKILL_DATA.fBASIC_VAR < 0.0f )
				{
					int nVAR = int(-sSKILL_DATA.fBASIC_VAR);
					nVAR_MP -= (int) ( nVAR - (nVAR*nRESIST/100.0f*GLCONST_CHAR::fRESIST_G ) );
				}
				//	����� ȸ������ �ִ� ��ų.
				else
				{
					int nDX = pACTOR->GetMaxMP() - pACTOR->GetNowMP();
					nVAR_MP += (int) min(nDX,sSKILL_DATA.fBASIC_VAR);
				}
				break;

			case SKILL::EMFOR_SP:
				//	��󿡰� ���ظ� �ִ� ��ų.
				if ( sSKILL_DATA.fBASIC_VAR < 0.0f )
				{
					int nVAR = int(-sSKILL_DATA.fBASIC_VAR);
					nVAR_SP -= (int) ( nVAR - (nVAR*nRESIST/100.0f*GLCONST_CHAR::fRESIST_G ) );
				}
				//	����� ȸ������ �ִ� ��ų.
				else
				{
					int nDX = pACTOR->GetMaxSP() - pACTOR->GetNowSP();
					nVAR_SP += (int) min(nDX,sSKILL_DATA.fBASIC_VAR);
				}
				break;
			};

			if ( dwDamageFlag & DAMAGE_TYPE_CRUSHING_BLOW ) bCrushBlowOne = true;

			//	Note : SP �����ÿ��� ��ȭ ���� ������ ����.
			//
			if ( bLowSP )
			{
				nVAR_HP /= 2;
				nVAR_MP /= 2;
				nVAR_SP /= 2;
			}

			for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
			{
				const SKILL::SSPEC &sSKILL_SPEC = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wSKILL_LVL];

				switch ( pSkill->m_sAPPLY.sSpecs[nSpec].emSPEC )
				{
				case EMSPECA_HP_GATHER:
					nGATHER_HP += int ( abs(nVAR_HP) * sSKILL_SPEC.fVAR1 );
					break;

				case EMSPECA_MP_GATHER:
					nGATHER_MP += int ( abs(nVAR_MP) * sSKILL_SPEC.fVAR1 );
					break;

				case EMSPECA_SP_GATHER:
					nGATHER_SP += int ( abs(nVAR_SP) * sSKILL_SPEC.fVAR1 );
					break;
				};
			}


			//	Note : �ڱ� �ڽſ��� �� ��ų�� �����ϰ� EXP �ڷḦ ����.
			//
			if ( sTARID!=STARGETID(CROW_PC,m_dwGaeaID) )
			{
				++nEXP_NUM;
				nALL_LVL += pACTOR->GetLevel ();

				nALL_VARHP += nVAR_HP;
				nALL_VARMP += nVAR_MP;
				nALL_VARSP += nVAR_SP;

				nALL_FULLHP += pACTOR->GetMaxHP();
				nALL_FULLMP += pACTOR->GetMaxMP();
				nALL_FULLSP += pACTOR->GetMaxSP();
			}

			if ( pACTOR->GetCrow() == CROW_PC && sSKILL_DATA.fBASIC_VAR < 0.0f )
			{
				PGLCHAR pCHARACTOR = (PGLCHAR)pACTOR;
				WORD wAbsorb = (WORD)pCHARACTOR->ISHAVE_BUFF( (EMSPEC_ADDON) EMSPECA_IGNORE_DAMAGE );
				if ( wAbsorb != SKILLFACT_SIZE ) 
				{
					pCHARACTOR->DISABLESKEFF(wAbsorb);

					GLMSG::SNETPC_REMOVE_SKILL_EFFECT_BRD NetMsgBrd;
					NetMsgBrd.dwGaeaID = pCHARACTOR->m_dwGaeaID;
					NetMsgBrd.dwSkillID = wAbsorb;
					pCHARACTOR->SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
					pCHARACTOR->SendToClient ( (NET_MSG_GENERIC*) &NetMsgBrd );

					SSKILLACT sSKILLACT;
					sSKILLACT.sID = STARGETID(CROW_PC,m_dwGaeaID);
					sSKILLACT.sID_TAR = STARGETID(_sTARID.GETCROW(),_sTARID.GETID());
					sSKILLACT.dwDamageFlag = DAMAGE_TYPE_NONE;
					m_pLandMan->RegSkillAct ( sSKILLACT );

					pCHARACTOR->MsgSendUpdateState(false,false,true);
				}
				else
				{
					SSKILLACT sSKILLACT;
					sSKILLACT.sID = STARGETID(CROW_PC,m_dwGaeaID);
					sSKILLACT.sID_TAR = STARGETID(_sTARID.GETCROW(),_sTARID.GETID());
					sSKILLACT.fDELAY = pSkill->m_sEXT_DATA.fDELAY4DAMAGE;
					sSKILLACT.emAPPLY = pSkill->m_sBASIC.emAPPLY;
					sSKILLACT.dwDamageFlag = dwDamageFlag;			

					//	Note : ü�� ��ȭ ����.
					//
					sSKILLACT.nVAR_HP = nVAR_HP;
					sSKILLACT.nVAR_MP = nVAR_MP;
					sSKILLACT.nVAR_SP = nVAR_SP;

					//	Note : ü�� ���� ����.
					//
					sSKILLACT.nGATHER_HP = nGATHER_HP;
					sSKILLACT.nGATHER_MP = nGATHER_MP;
					sSKILLACT.nGATHER_SP = nGATHER_SP;
					if ( sSKILLACT.VALID() )
					{
						m_pLandMan->RegSkillAct ( sSKILLACT );
						bAsignStigma = true;
					}
				}
			}
			else if ( pACTOR->GetCrow() == CROW_MOB && sSKILL_DATA.fBASIC_VAR < 0.0f )
			{
				PGLCROW pCROWACTOR = (PGLCROW)pACTOR;
				WORD wAbsorb = (WORD)pCROWACTOR->ISHAVE_BUFF( (EMSPEC_ADDON) EMSPECA_IGNORE_DAMAGE );
				if ( wAbsorb != SKILLFACT_SIZE ) 
				{
					pCROWACTOR->DISABLESKEFF(wAbsorb);

					GLMSG::SNETPC_SKILLHOLD_RS_BRD NetMsgBrd;
					NetMsgBrd.dwID = pCROWACTOR->GetCtrlID();
					NetMsgBrd.emCrow = pACTOR->GetCrow();
					NetMsgBrd.bRESET[wAbsorb] = true;
					pCROWACTOR->SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

					SSKILLACT sSKILLACT;
					sSKILLACT.sID = STARGETID(CROW_PC,m_dwGaeaID);
					sSKILLACT.sID_TAR = STARGETID(_sTARID.GETCROW(),_sTARID.GETID());
					sSKILLACT.dwDamageFlag = DAMAGE_TYPE_NONE;
					m_pLandMan->RegSkillAct ( sSKILLACT );
				}
				else
				{
					//	Note : ��ų �⺻ ����.
					//
					SSKILLACT sSKILLACT;
					sSKILLACT.sID = STARGETID(CROW_PC,m_dwGaeaID);
					sSKILLACT.sID_TAR = STARGETID(_sTARID.GETCROW(),_sTARID.GETID());
					sSKILLACT.fDELAY = pSkill->m_sEXT_DATA.fDELAY4DAMAGE;
					sSKILLACT.emAPPLY = pSkill->m_sBASIC.emAPPLY;
					sSKILLACT.dwDamageFlag = dwDamageFlag;

					//	Note : ü�� ��ȭ ����.
					//
					sSKILLACT.nVAR_HP = nVAR_HP;
					sSKILLACT.nVAR_MP = nVAR_MP;
					sSKILLACT.nVAR_SP = nVAR_SP;

					//	Note : ü�� ���� ����.
					//
					sSKILLACT.nGATHER_HP = nGATHER_HP;
					sSKILLACT.nGATHER_MP = nGATHER_MP;
					sSKILLACT.nGATHER_SP = nGATHER_SP;

					if ( sSKILLACT.VALID() )
					{
						m_pLandMan->RegSkillAct ( sSKILLACT );
						bAsignStigma = true;
					}
				}
			}
			else
			{
				SSKILLACT sSKILLACT;
				sSKILLACT.sID = STARGETID(CROW_PC,m_dwGaeaID);
				sSKILLACT.sID_TAR = STARGETID(_sTARID.GETCROW(),_sTARID.GETID());
				sSKILLACT.fDELAY = pSkill->m_sEXT_DATA.fDELAY4DAMAGE;
				sSKILLACT.emAPPLY = pSkill->m_sBASIC.emAPPLY;
				sSKILLACT.dwDamageFlag = dwDamageFlag;			

				//	Note : ü�� ��ȭ ����.
				//
				sSKILLACT.nVAR_HP = nVAR_HP;
				sSKILLACT.nVAR_MP = nVAR_MP;
				sSKILLACT.nVAR_SP = nVAR_SP;

				//	Note : ü�� ���� ����.
				//
				sSKILLACT.nGATHER_HP = nGATHER_HP;
				sSKILLACT.nGATHER_MP = nGATHER_MP;
				sSKILLACT.nGATHER_SP = nGATHER_SP;
				if ( sSKILLACT.VALID() )
				{
					m_pLandMan->RegSkillAct ( sSKILLACT );
					bAsignStigma = true;
				}
			}

			//	Note : ��ų �⺻ ����.
			//
			//SSKILLACT sSKILLACT;
			//sSKILLACT.sID = STARGETID(CROW_PC,m_dwGaeaID);
			//sSKILLACT.sID_TAR = STARGETID(_sTARID.GETCROW(),_sTARID.GETID());
			//sSKILLACT.fDELAY = pSkill->m_sEXT_DATA.fDELAY4DAMAGE;
			//sSKILLACT.emAPPLY = pSkill->m_sBASIC.emAPPLY;
			//sSKILLACT.dwDamageFlag = dwDamageFlag;			

			////	Note : ü�� ��ȭ ����.
			////
			//sSKILLACT.nVAR_HP = nVAR_HP;
			//sSKILLACT.nVAR_MP = nVAR_MP;
			//sSKILLACT.nVAR_SP = nVAR_SP;

			////	Note : ü�� ���� ����.
			////
			//sSKILLACT.nGATHER_HP = nGATHER_HP;
			//sSKILLACT.nGATHER_MP = nGATHER_MP;
			//sSKILLACT.nGATHER_SP = nGATHER_SP;

			//if ( sSKILLACT.VALID() )
			//{
			//	m_pLandMan->RegSkillAct ( sSKILLACT );
			//}
		}

		//	Note : ��ų ���� ����.
		//
		SSKILLACTEX sSKILLACTEX;
		sSKILLACTEX.sID = STARGETID(CROW_PC,m_dwGaeaID);
		sSKILLACTEX.sID_TAR = STARGETID(_sTARID.GETCROW(),_sTARID.GETID());
		sSKILLACTEX.fDELAY = pSkill->m_sEXT_DATA.fDELAY4DAMAGE;

		sSKILLACTEX.idSKILL = skill_id;
		sSKILLACTEX.wSKILL_LVL = wSKILL_LVL;

		//  ���� Ÿ�� Ȯ��
		if ( bCrushBlowOne )
		{
			sSKILLACTEX.bCrushBlow = bCrushBlowOne;
			sSKILLACTEX.fPUSH_PULL = GLCONST_CHAR::fCRUSH_BLOW_RANGE;
		}

		//	Note : ���� �̻� ����.
		//
		ITEM::SSATE_BLOW sBLOW;
		sBLOW.emTYPE = EMBLOW_NONE;

		if ( pSkill->m_sAPPLY.emELEMENT==EMELEMENT_ARM )
		{
			SITEM *pITEM = GET_ELMT_ITEM ();
			if ( pITEM )	sBLOW = pITEM->sSuitOp.sBLOW;
		}
		else
		{
			const SKILL::SSTATE_BLOW &sSKILL_BLOW = pSkill->m_sAPPLY.sSTATE_BLOW[wSKILL_LVL];

			sBLOW.emTYPE = pSkill->m_sAPPLY.emSTATE_BLOW;
			sBLOW.fRATE = sSKILL_BLOW.fRATE;
			sBLOW.fLIFE = pSkill->m_sAPPLY.sDATA_LVL[wSKILL_LVL].fLIFE;
			sBLOW.fVAR1 = sSKILL_BLOW.fVAR1;
			sBLOW.fVAR2 = sSKILL_BLOW.fVAR2;
		}
        
		if ( sBLOW.emTYPE!=EMBLOW_NONE )
		{
			//	Note : �߻� Ȯ�� ���.
			//
			short nBLOWRESIST = pACTOR->GETRESIST().GetElement ( STATE_TO_ELEMENT(sBLOW.emTYPE) );
			if ( nBLOWRESIST>GLCONST_CHAR::fRESIST_G )	nBLOWRESIST = GLCONST_CHAR::fRESIST_G;

//			float fPOWER = GLOGICEX::WEATHER_BLOW_POW ( sBLOW.emTYPE, GLPeriod::GetInstance().GetWeather(), m_pLandMan->IsWeatherActive() );
			DWORD dwWeather = GLPeriod::GetInstance().GetMapWeather( m_pLandMan->GetMapID().wMainID, m_pLandMan->GetMapID().wSubID );
			float fPOWER = GLOGICEX::WEATHER_BLOW_POW ( sBLOW.emTYPE, dwWeather, m_pLandMan->IsWeatherActive() );

			BOOL bBLOW(FALSE);
			if ( !(pACTOR->GETHOLDBLOW()&STATE_TO_DISORDER(sBLOW.emTYPE)) )
			{
				bBLOW = GLOGICEX::CHECKSTATEBLOW ( sBLOW.fRATE * fPOWER, GETLEVEL(), pACTOR->GetLevel(), nBLOWRESIST );
			}

			if ( bBLOW )
			{
				//	Note : �����̻� �߻�.
				//	
				SSTATEBLOW sSTATEBLOW;
				float fLIFE = sBLOW.fLIFE * fPOWER;
				fLIFE = ( fLIFE - (fLIFE*nBLOWRESIST/100.0f*GLCONST_CHAR::fRESIST_G ) );

				sSTATEBLOW.emBLOW = sBLOW.emTYPE;
				sSTATEBLOW.fAGE = fLIFE;
				sSTATEBLOW.fSTATE_VAR1 = sBLOW.fVAR1;
				sSTATEBLOW.fSTATE_VAR2 = sBLOW.fVAR2;

				//	Note : ���� �̻� �߻� ����.
				//
				sSKILLACTEX.sSTATEBLOW = sSTATEBLOW;
			}
		}

		//	Note : ��ų Ư�� ���.
		//
		BOOL bActionMovement = FALSE;
		BOOL bCUREFLAG = FALSE;
		BOOL bREMOVEFLAG = FALSE;
		BOOL bREBIRTH = FALSE;

		if ( bCrushBlowOne )	bActionMovement = TRUE;
		if ( bACTDASH )			bActionMovement = TRUE;

		for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
		{
			const SKILL::SSPEC &sSKILL_SPEC = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wSKILL_LVL];
			switch ( pSkill->m_sAPPLY.sSpecs[nSpec].emSPEC )
			{
			case EMSPECA_PUSHPULL:
				{
					if ( bActionMovement )	break;
					if ( !pACTOR->IsValidBody() )				break;
					if ( RANDOM_POS > sSKILL_SPEC.fVAR2 )		break;	//	Note : �߻�Ȯ�� ����.
					sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR1;
					bActionMovement = TRUE;
				}break;

			case EMSPECA_CRUSHING_BLOW:
				{
					/*dmk14 crushing blow with pull push effect fix*/
					if ( pSkill->m_sAPPLY.IsSpec( EMSPECA_PULL ) || pSkill->m_sAPPLY.IsSpec( EMSPECA_PUSH ) ||
						 pSkill->m_sAPPLY.IsSpec( EMSPECA_PUSHPULL ))	continue;
					if ( bActionMovement )	break;
					sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR1;
					bActionMovement = TRUE;
				}break;

			case EMSPECA_STIGMA:
				{
					ApplySkillTargetStigma(sTARID, sSKILL_SPEC, pSkill);	
					bStigmaEff = TRUE;
				}
				break;

				/*push pull skill logic, Juver, 2017/06/04 */
			case EMSPECA_PUSH:
				{
					if ( bActionMovement )			break;
					if ( !pACTOR->IsValidBody() )	break;
					if ( RANDOM_POS > sSKILL_SPEC.fVAR1 )		break;
					sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR2; //distance
					sSKILLACTEX.bPush = true;
					bActionMovement = TRUE;
				}break;

			case EMSPECA_INVISIBLE:
				{
					//if ( IsSTATE(EM_ACT_RUN) )
					{
						ReSetSTATE(EM_ACT_RUN);
						GLMSG::SNETPC_MOVESTATE_BRD NetMsgFB;
						NetMsgFB.dwGaeaID = m_dwGaeaID;
						NetMsgFB.dwActState = m_dwActState;
						SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgFB );
						SendToClient ( (NET_MSG_GENERIC*) &NetMsgFB );
					}
				}
				break;

			case EMSPECA_TELEPORT:
				ApplySkillSelfTeleport ( GetGLGaeaServer(), this, pSkill, wSKILL_LVL, pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wSKILL_LVL], sTARID, m_wTARNUM, m_sTARIDS );
				break;

			case EMSPECA_STUN:
				ApplySkillTargetStun( pACTOR );
				break;

			case EMSPECA_POSITION_SHIFT:
				//ApplySkillSelfSwapPos ( this, pSkill, wSKILL_LVL, pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wSKILL_LVL], m_wTARNUM, m_sTARIDS, GetLandMan() );
				break;

			//Teleport Skill Effect
			/*case EMSPECA_RANDOM_EXP_RATE: 	
				if (!pACTOR->IsValidBody()) break;
		
				sSKILLACTEX.bTeleport = true;
				sSKILLACTEX.vTeleport_Pos = m_TargetID.vPos;
				break;*/

				/*push pull skill logic, Juver, 2017/06/04 */
			case EMSPECA_PULL:
				{
					if ( bActionMovement )			break;
					if ( !pACTOR->IsValidBody() )	break;
					if ( RANDOM_POS > sSKILL_SPEC.fVAR1 )		break;
					sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR2; //pull speed
					sSKILLACTEX.bPull = true;
					bActionMovement = TRUE;
				}break;

				/*dash skill logic, Juver, 2017/06/17 */
			case EMSPECA_ONWARD:
				{
					if ( bActionMovement )			break;
					if ( bACTDASH )					break;
					if ( !pACTOR->IsValidBody() )	break;
					if ( RANDOM_POS > sSKILL_SPEC.fVAR1 )		break;
					sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR2; //dash speed
					sSKILLACTEX.wActionAnim = (WORD)sSKILL_SPEC.dwFLAG; //animation
					sSKILLACTEX.bDash = true;
					bACTDASH = TRUE;
					bActionMovement = TRUE;
				}break;

			case EMSPECA_DAMAGE_LOOP: //����
				{
					//PGLCROW pCrow = m_pLandMan->GetCrow ( pACTOR->GetCtrlID() );
					if( !pACTOR->GetCrow() == CROW_PC ) break;
					//if( pCrow->m_pCrowData->m_emNPCType == EMNPC_TYPE_EVENT_TOWER ) break;
					if( !pACTOR->IsValidBody() )		 break;
					const float fMaxHP = static_cast<float>(GetMaxHP());
					const float fNoWHP = static_cast<float>(GetNowHP());
					const float fActorOldHP = static_cast<float>(pACTOR->GetNowHP());
					const float fActorMaxHP = static_cast<float>(pACTOR->GetMaxHP());

					const int fDamage = static_cast<int>(fActorOldHP - ( fActorMaxHP * fNoWHP / fMaxHP ));
					if( fDamage > 0 )
					{
						nVAR_HP -= fDamage;  //�ִ� HP�� �����ϱ����� ������ ȿ���� �����ν� ������ ǥ�ø� �� �� �ְ���(������ ���� ���̴� HP���� ����)
					}
					pACTOR->SetMaxHpRate( fNoWHP / fMaxHP );
				}
				break;

			case EMSPECA_SKILLLINK:
				ApplySkillTargetSkillLink(m_TargetID, sTARID, pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wSKILL_LVL], sSkill);
				break;

			case EMSPECA_HP_DIV:
			case EMSPECA_MP_DIV:
			case EMSPECA_SP_DIV:
				break;

			case EMSPECA_RECBLOW:
				{
					//	Note : ���� �̻� ġ��.
					//
					if ( bCUREFLAG )	break;
					sSKILLACTEX.dwCUREFLAG |= pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wSKILL_LVL].dwFLAG;
					bCUREFLAG = TRUE;
				}break;

				/*debuff skill logic, Juver, 2017/06/10 */
			case EMSPECA_BUFF_REMOVE:
				{
					if ( bREMOVEFLAG )	break;
					if ( sSKILLACTEX.bDebuff )	break;
					sSKILLACTEX.dwRemoveFlag = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wSKILL_LVL].dwFLAG;
					sSKILLACTEX.bDebuff = true;
					bREMOVEFLAG = TRUE;
				}break;

			case EMSPECA_REBIRTH:
				{
					if ( bREBIRTH )	break;

					if ( sTARID.emCrow==CROW_PC )
					{
						PGLCHAR pPC = m_pGLGaeaServer->GetChar(sTARID.dwID);
						if ( pPC )
						{
							/*dmk14 disable resu on other schoool pvp wars*/
							bool bTyrannyWars = false;
							bool bSchoolWars = false;
							bool bCaptureTheFlag = false;
							bool bClubWars = false;

							if( m_pLandMan )
							{
								bTyrannyWars = m_pLandMan->m_bPVPTyrannyMap;
								bSchoolWars = m_pLandMan->m_bPVPSchoolWarsMap;
								bCaptureTheFlag = m_pLandMan->m_bPVPCaptureTheFlagMap;
								bClubWars = m_pLandMan->m_bGuidBattleMap;
							}

							if ( m_pLandMan )
							{
								if ( m_pLandMan->m_bClubDeathMatchMap )
								{
									if ( pPC->m_dwGuild != m_dwGuild )	break;
								}
								else
								{
									if ( pPC->m_wSchool != m_wSchool )	break;
								}
							}

							WORD wRECOVE = pPC->DoRevive2Skill(skill_id,wSKILL_LVL, (WORD)nSpec );
							if ( 0<wRECOVE)
							{
								++nEXP_NUM;
								nALL_LVL += pPC->GetLevel ();
								nALL_VARHP += wRECOVE;
								nALL_FULLHP += pPC->GetMaxHP();

								/*pvp tyranny, Juver, 2017/08/24 */
								if ( m_pLandMan && m_pLandMan->m_bPVPTyrannyMap )
									GLPVPTyrannyField::GetInstance().ScoreResu( m_dwCharID );

								/*school wars, Juver, 2018/01/20 */
								if ( m_pLandMan && m_pLandMan->m_bPVPSchoolWarsMap )
									GLPVPSchoolWarsField::GetInstance().ScoreResu( m_dwCharID );

								/*pvp capture the flag, Juver, 2018/02/01 */
								if ( m_pLandMan && m_pLandMan->m_bPVPCaptureTheFlagMap )
									GLPVPCaptureTheFlagField::GetInstance().ScoreResu( m_dwCharID );
							}
						}
					}

					bREBIRTH = TRUE;
				}break;
			};
		}
		

		switch (pSkill->m_sAPPLY.emBASIC_TYPE)
		{
		case SKILL::EMFOR_CURE:
			//	Note : ���� �̻� ġ��.
			//
			sSKILLACTEX.dwCUREFLAG |= pSkill->m_sAPPLY.dwCUREFLAG;
			break;
		};

		if ( sSKILLACTEX.VALID() )
		{
			m_pLandMan->RegSkillActEx ( sSKILLACTEX );
			nALL_VARHP;
		}

		//	Note : ȸ���� �⿩���� ���� ����ġ �߻�.	( ���ݽô� ���� )
		//
		int nSUM_EXP = 0;
		bool bRECOVE = (nALL_VARHP>=0&&nALL_VARMP>=0&&nALL_VARSP&&0) && (nALL_VARHP+nALL_VARMP+nALL_VARSP)>0;

		if ( bRECOVE )
		{
			//	���� ��ȭ���� ���� ���� ����ġ�� ����.
			if ( nALL_VARHP>0 )
				nSUM_EXP += GLOGICEX::GLATTACKEXP(GETLEVEL(),nALL_LVL,nALL_VARHP,nALL_FULLHP,pACTOR->GetBonusExp());

			if ( nALL_VARMP>0 )
				nSUM_EXP += GLOGICEX::GLATTACKEXP(GETLEVEL(),nALL_LVL,nALL_VARMP,nALL_FULLMP,pACTOR->GetBonusExp());

			if ( nALL_VARSP>0 )
				nSUM_EXP += GLOGICEX::GLATTACKEXP(GETLEVEL(),nALL_LVL,nALL_VARSP,nALL_FULLSP,pACTOR->GetBonusExp());

			nSUM_EXP += int (nSUM_EXP*GLCONST_CHAR::fREC_EXP_RATE);
		}

		//	Note : ����ġ ȹ�淮 ����.
		//
		if ( nSUM_EXP )
		{
			// ������ ����Ʈ���� �Է��� ����ġ ���� ���� (����)
			nSUM_EXP = int( (float)nSUM_EXP * GetExpMultipleRateFromSlotItem() );

			m_sExperience.lnNow += nSUM_EXP;
			m_sExperience.LIMIT();
		}
	}

	/*skill summon, Juver, 2017/10/09 */
	if ( m_pGLGaeaServer && pSkill->m_sSPECIAL_SKILL.emSSTYPE == SKILL::EMSSTYPE_SUMMON )
	{
		SKILL::SSKILL_SUMMON_DATA &summon_data = pSkill->m_sSPECIAL_SKILL.sSummon[wSKILL_LVL];

		SUMMON_DATA_SKILL sdata;
		sdata.bSummon = TRUE;
		sdata.sTarget = m_sTARIDS[0];
		sdata.sidSkill = pSkill->m_sBASIC.sNATIVEID;
		
		sdata.Assign( summon_data );
		m_pGLGaeaServer->CreateSummon( sdata, m_dwGaeaID );
	}

	if ( bAsignStigma )
	{		
		SSKILLACT sSKILLACT_STIGMA;
		sSKILLACT_STIGMA.fDELAY		= pSkill->m_sEXT_DATA.fDELAY4DAMAGE;
		sSKILLACT_STIGMA.emAPPLY		= pSkill->m_sBASIC.emAPPLY;
		//sSKILLACT_STIGMA.sSkillID		= sSkill.sNativeID;
		//sSKILLACT_STIGMA.wSkillLevel	= sSkill.wLevel;
		sSKILLACT_STIGMA.dwDamageFlag	= dwDamageFlag | DAMAGE_TYPE_NONE;
		sSKILLACT_STIGMA.nVAR_HP = nVAR_HP;
		sSKILLACT_STIGMA.nVAR_MP = nVAR_MP;
		sSKILLACT_STIGMA.nVAR_SP = nVAR_SP;
		sSKILLACT_STIGMA.nGATHER_HP = nGATHER_HP;
		sSKILLACT_STIGMA.nGATHER_MP = nGATHER_MP;
		sSKILLACT_STIGMA.nGATHER_SP = nGATHER_SP;
		StigmaProc_Skill(&sSKILLACT_STIGMA, bStigmaEff);
	}

	//	Note : ��ų ��뿡 ���� �Ҹ� FB MSG.
	//
	GLMSG::SNETPC_SKILLCONSUME_FB NetMsgFB;
	EMSLOT emLHand = GetCurLHand();
	SITEM* pItem = GET_SLOT_ITEMDATA(emLHand);
	if ( pItem && (pItem->sDrugOp.bInstance) )		NetMsgFB.wTurnNum = m_PutOnItems[emLHand].wTurnNum;
	NetMsgFB.dwNowHP = m_sHP.dwNow;
	NetMsgFB.dwNowMP = m_sMP.dwNow;
	NetMsgFB.dwNowSP = m_sSP.dwNow;
	NetMsgFB.dwNowCP = m_sCombatPoint.dwNow; /*combatpoint logic, Juver, 2017/05/28 */

	m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsgFB );

	//	Note : �Ҹ� ������ ��ȿ�� �˻���, ������� �޽��� �߻�.
	//
	CheckInstanceItem ();

	PGLSKILL pSkillL = GLSkillMan::GetInstance().GetData ( pSkill->m_sEXT_DATA.sADDSKILL );
	if ( !pSkillL || pSkillL && pSkillL->m_sBASIC.emROLE == SKILL::EMROLE_PASSIVE ) return;
	{
		SKILL::CDATA_LVL &sSKILL_DATA = pSkillL->m_sAPPLY.sDATA_LVL[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL];

		if ( pSkillL->m_sAPPLY.IsSpec(EMSPECA_INVISIBLE) )
		{
			//if ( IsSTATE(EM_ACT_RUN) )
			{
				ReSetSTATE(EM_ACT_RUN);
				GLMSG::SNETPC_MOVESTATE_BRD NetMsgFB;
				NetMsgFB.dwGaeaID = m_dwGaeaID;
				NetMsgFB.dwActState = m_dwActState;
				SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgFB );
				SendToClient ( (NET_MSG_GENERIC*) &NetMsgFB );
			}
		}
		/*dash skill logic, Juver, 2017/06/17 */
		BOOL bACTDASH = FALSE;

		//	Note : ��ų�� ����Ǵ� Ÿ�� �˻�.
		//
		BOOL bAsignStigma = false;
		BOOL	bStigmaEff = FALSE;
		STARGETID sTARID;

		if ( pSkillL->m_sBASIC.emIMPACT_SIDE != SIDE_ENEMY )
		{
			const STARID &_sTARID = STARID(CROW_PC,m_dwGaeaID);

			sTARID.emCrow = _sTARID.GETCROW();
			sTARID.dwID = _sTARID.GETID();
			GLACTOR* pACTOR = m_pGLGaeaServer->GetTarget ( m_pLandMan, sTARID );
			if ( !pACTOR )									return;

			//	Note : ��ų ���� ����.
			//
			SSKILLACTEX sSKILLACTEX;
			//	Note : ?? ?? ??.
			//
			sSKILLACTEX.sID = STARGETID(CROW_PC,m_dwGaeaID);
			sSKILLACTEX.sID_TAR = STARGETID(_sTARID.GETCROW(),_sTARID.GETID());
			sSKILLACTEX.fDELAY = pSkillL->m_sEXT_DATA.fDELAY4DAMAGE;

			sSKILLACTEX.idSKILL = pSkill->m_sEXT_DATA.sADDSKILL;
			sSKILLACTEX.wSKILL_LVL = pSkill->m_sEXT_DATA.dwADDSKILLLEVEL;
		
			//	Note : ��ų Ư�� ���.
			//
			BOOL bActionMovement = FALSE;
			BOOL bCUREFLAG = FALSE;
			BOOL bREMOVEFLAG = FALSE;
			BOOL bREBIRTH = FALSE;

			if ( bACTDASH )			bActionMovement = TRUE;

			for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
			{
				const SKILL::SSPEC &sSKILL_SPEC = pSkillL->m_sAPPLY.sSpecs[nSpec].sSPEC[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL];
				switch ( pSkillL->m_sAPPLY.sSpecs[nSpec].emSPEC )
				{
				/*push pull skill logic, Juver, 2017/06/04 */
				case EMSPECA_PUSH:
					{
						if ( bActionMovement )			break;
						if ( !pACTOR->IsValidBody() )	break;
						if ( RANDOM_POS > sSKILL_SPEC.fVAR1 )		break;
						sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR2; //distance
						sSKILLACTEX.bPush = true;
						bActionMovement = TRUE;
					}break;

				case EMSPECA_INVISIBLE:
					{
						//if ( IsSTATE(EM_ACT_RUN) )
						{
							ReSetSTATE(EM_ACT_RUN);
							GLMSG::SNETPC_MOVESTATE_BRD NetMsgFB;
							NetMsgFB.dwGaeaID = m_dwGaeaID;
							NetMsgFB.dwActState = m_dwActState;
							SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgFB );
							SendToClient ( (NET_MSG_GENERIC*) &NetMsgFB );
						}
					}
					break;

				case EMSPECA_TELEPORT:
					ApplySkillSelfTeleport ( GetGLGaeaServer(), this, pSkillL, pSkill->m_sEXT_DATA.dwADDSKILLLEVEL, pSkillL->m_sAPPLY.sSpecs[nSpec].sSPEC[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL], m_TargetID, m_wTARNUM, m_sTARIDS );
					break;

				case EMSPECA_POSITION_SHIFT:
					//ApplySkillSelfSwapPos ( this, pSkillL, pSkill->m_sEXT_DATA.dwADDSKILLLEVEL, pSkillL->m_sAPPLY.sSpecs[nSpec].sSPEC[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL], m_wTARNUM, m_sTARIDS, GetLandMan() );
					break;

					/*push pull skill logic, Juver, 2017/06/04 */
				case EMSPECA_PULL:
					{
						if ( bActionMovement )			break;
						if ( !pACTOR->IsValidBody() )	break;
						if ( RANDOM_POS > sSKILL_SPEC.fVAR1 )		break;
						sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR2; //pull speed
						sSKILLACTEX.bPull = true;
						bActionMovement = TRUE;
					}break;

					/*dash skill logic, Juver, 2017/06/17 */
				case EMSPECA_ONWARD:
					{
						if ( bActionMovement )			break;
						if ( bACTDASH )					break;
						if ( !pACTOR->IsValidBody() )	break;
						if ( RANDOM_POS > sSKILL_SPEC.fVAR1 )		break;
						sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR2; //dash speed
						sSKILLACTEX.wActionAnim = (WORD)sSKILL_SPEC.dwFLAG; //animation
						sSKILLACTEX.bDash = true;
						bACTDASH = TRUE;
						bActionMovement = TRUE;
					}break;

				case EMSPECA_SKILLLINK:
					ApplySkillTargetSkillLink(m_TargetID, sTARID, pSkillL->m_sAPPLY.sSpecs[nSpec].sSPEC[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL], sSkill);
					break;

				case EMSPECA_HP_DIV:
				case EMSPECA_MP_DIV:
				case EMSPECA_SP_DIV:
					break;

				case EMSPECA_RECBLOW:
					{
						//	Note : ���� �̻� ġ��.
						//
						if ( bCUREFLAG )	break;
						sSKILLACTEX.dwCUREFLAG |= pSkillL->m_sAPPLY.sSpecs[nSpec].sSPEC[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL].dwFLAG;
						bCUREFLAG = TRUE;
					}break;

					/*debuff skill logic, Juver, 2017/06/10 */
				case EMSPECA_BUFF_REMOVE:
					{
						if ( bREMOVEFLAG )	break;
						if ( sSKILLACTEX.bDebuff )	break;
						sSKILLACTEX.dwRemoveFlag = pSkillL->m_sAPPLY.sSpecs[nSpec].sSPEC[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL].dwFLAG;
						sSKILLACTEX.bDebuff = true;
						bREMOVEFLAG = TRUE;
					}break;

				case EMSPECA_REBIRTH:
					{
						if ( bREBIRTH )	break;

						if ( sTARID.emCrow==CROW_PC )
						{
							PGLCHAR pPC = m_pGLGaeaServer->GetChar(sTARID.dwID);
							if ( pPC )
							{
								WORD wRECOVE = pPC->DoRevive2Skill(pSkill->m_sEXT_DATA.sADDSKILL,pSkill->m_sEXT_DATA.dwADDSKILLLEVEL, (WORD)nSpec );
								if ( 0<wRECOVE)
								{
									++nEXP_NUM;
									nALL_LVL += pPC->GetLevel ();
									nALL_VARHP += wRECOVE;
									nALL_FULLHP += pPC->GetMaxHP();

									/*pvp tyranny, Juver, 2017/08/24 */
									if ( m_pLandMan && m_pLandMan->m_bPVPTyrannyMap )
										GLPVPTyrannyField::GetInstance().ScoreResu( m_dwCharID );

									/*school wars, Juver, 2018/01/20 */
									if ( m_pLandMan && m_pLandMan->m_bPVPSchoolWarsMap )
										GLPVPSchoolWarsField::GetInstance().ScoreResu( m_dwCharID );

									/*pvp capture the flag, Juver, 2018/02/01 */
									if ( m_pLandMan && m_pLandMan->m_bPVPCaptureTheFlagMap )
										GLPVPCaptureTheFlagField::GetInstance().ScoreResu( m_dwCharID );
								}
							}
						}

						bREBIRTH = TRUE;
					}break;
				};
			}


			switch (pSkillL->m_sAPPLY.emBASIC_TYPE)
			{
			case SKILL::EMFOR_CURE:
				//	Note : ���� �̻� ġ��.
				//
				sSKILLACTEX.dwCUREFLAG |= pSkillL->m_sAPPLY.dwCUREFLAG;
				break;
			};

			if ( sSKILLACTEX.VALID() )
			{
				m_pLandMan->RegSkillActEx ( sSKILLACTEX );
				nALL_VARHP;
			}

			//	Note : ȸ���� �⿩���� ���� ����ġ �߻�.	( ���ݽô� ���� )
			//
			int nSUM_EXP = 0;
			bool bRECOVE = (nALL_VARHP>=0&&nALL_VARMP>=0&&nALL_VARSP&&0) && (nALL_VARHP+nALL_VARMP+nALL_VARSP)>0;

			if ( bRECOVE )
			{
				//	���� ��ȭ���� ���� ���� ����ġ�� ����.
				if ( nALL_VARHP>0 )
					nSUM_EXP += GLOGICEX::GLATTACKEXP(GETLEVEL(),nALL_LVL,nALL_VARHP,nALL_FULLHP,pACTOR->GetBonusExp());

				if ( nALL_VARMP>0 )
					nSUM_EXP += GLOGICEX::GLATTACKEXP(GETLEVEL(),nALL_LVL,nALL_VARMP,nALL_FULLMP,pACTOR->GetBonusExp());

				if ( nALL_VARSP>0 )
					nSUM_EXP += GLOGICEX::GLATTACKEXP(GETLEVEL(),nALL_LVL,nALL_VARSP,nALL_FULLSP,pACTOR->GetBonusExp());

				nSUM_EXP += int (nSUM_EXP*GLCONST_CHAR::fREC_EXP_RATE);
			}

			//	Note : ����ġ ȹ�淮 ����.
			//
			if ( nSUM_EXP )
			{
				// ������ ����Ʈ���� �Է��� ����ġ ���� ���� (����)
				nSUM_EXP = int( (float)nSUM_EXP * GetExpMultipleRateFromSlotItem() );

				m_sExperience.lnNow += nSUM_EXP;
				m_sExperience.LIMIT();
			}

			/*skill summon, Juver, 2017/10/09 */
			if ( m_pGLGaeaServer && pSkillL->m_sSPECIAL_SKILL.emSSTYPE == SKILL::EMSSTYPE_SUMMON )
			{
				SKILL::SSKILL_SUMMON_DATA &summon_data = pSkillL->m_sSPECIAL_SKILL.sSummon[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL];

				SUMMON_DATA_SKILL sdata;
				sdata.bSummon = TRUE;
				sdata.sTarget = m_sTARIDS[0];
				sdata.sidSkill = pSkillL->m_sBASIC.sNATIVEID;

				sdata.Assign( summon_data );
				m_pGLGaeaServer->CreateSummon( sdata, m_dwGaeaID );
			}
		}
		else
		{
			for ( WORD i=0; i<m_wTARNUM; ++i )
			{
				const STARID &_sTARID = m_sTARIDS[i];

				sTARID.emCrow = _sTARID.GETCROW();
				sTARID.dwID = _sTARID.GETID();
				GLACTOR* pACTOR = m_pGLGaeaServer->GetTarget ( m_pLandMan, sTARID );
				if ( !pACTOR )									continue;

				//	Note : ��ų ���� ����.
				//
				SSKILLACTEX sSKILLACTEX;
				//sSKILLACTEX.sID = STARGETID(CROW_PC,m_dwGaeaID);
				//sSKILLACTEX.sID_TAR = STARGETID(_sTARID.GETCROW(),_sTARID.GETID());

				//	Note : ?? ?? ??.
				//
				
				sSKILLACTEX.sID = STARGETID(CROW_PC,m_dwGaeaID);
				sSKILLACTEX.sID_TAR = STARGETID(_sTARID.GETCROW(),_sTARID.GETID());
				

				sSKILLACTEX.fDELAY = pSkillL->m_sEXT_DATA.fDELAY4DAMAGE;

				sSKILLACTEX.idSKILL = pSkill->m_sEXT_DATA.sADDSKILL;
				sSKILLACTEX.wSKILL_LVL = pSkill->m_sEXT_DATA.dwADDSKILLLEVEL;

				//	Note : ���� �̻� ����.
				//
				ITEM::SSATE_BLOW sBLOW;
				sBLOW.emTYPE = EMBLOW_NONE;

				if ( pSkillL->m_sAPPLY.emELEMENT==EMELEMENT_ARM )
				{
					SITEM *pITEM = GET_ELMT_ITEM ();
					if ( pITEM )	sBLOW = pITEM->sSuitOp.sBLOW;
				}
				else
				{
					const SKILL::SSTATE_BLOW &sSKILL_BLOW = pSkillL->m_sAPPLY.sSTATE_BLOW[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL];

					sBLOW.emTYPE = pSkillL->m_sAPPLY.emSTATE_BLOW;
					sBLOW.fRATE = sSKILL_BLOW.fRATE;
					sBLOW.fLIFE = pSkillL->m_sAPPLY.sDATA_LVL[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL].fLIFE;
					sBLOW.fVAR1 = sSKILL_BLOW.fVAR1;
					sBLOW.fVAR2 = sSKILL_BLOW.fVAR2;
				}

				if ( sBLOW.emTYPE!=EMBLOW_NONE )
				{
					//	Note : �߻� Ȯ�� ���.
					//
					short nBLOWRESIST = pACTOR->GETRESIST().GetElement ( STATE_TO_ELEMENT(sBLOW.emTYPE) );
					if ( nBLOWRESIST>GLCONST_CHAR::fRESIST_G )	nBLOWRESIST = GLCONST_CHAR::fRESIST_G;

					//			float fPOWER = GLOGICEX::WEATHER_BLOW_POW ( sBLOW.emTYPE, GLPeriod::GetInstance().GetWeather(), m_pLandMan->IsWeatherActive() );
					DWORD dwWeather = GLPeriod::GetInstance().GetMapWeather( m_pLandMan->GetMapID().wMainID, m_pLandMan->GetMapID().wSubID );
					float fPOWER = GLOGICEX::WEATHER_BLOW_POW ( sBLOW.emTYPE, dwWeather, m_pLandMan->IsWeatherActive() );

					BOOL bBLOW(FALSE);
					if ( !(pACTOR->GETHOLDBLOW()&STATE_TO_DISORDER(sBLOW.emTYPE)) )
					{
						bBLOW = GLOGICEX::CHECKSTATEBLOW ( sBLOW.fRATE * fPOWER, GETLEVEL(), pACTOR->GetLevel(), nBLOWRESIST );
					}

					if ( bBLOW )
					{
						//	Note : �����̻� �߻�.
						//	
						SSTATEBLOW sSTATEBLOW;
						float fLIFE = sBLOW.fLIFE * fPOWER;
						fLIFE = ( fLIFE - (fLIFE*nBLOWRESIST/100.0f*GLCONST_CHAR::fRESIST_G ) );

						sSTATEBLOW.emBLOW = sBLOW.emTYPE;
						sSTATEBLOW.fAGE = fLIFE;
						sSTATEBLOW.fSTATE_VAR1 = sBLOW.fVAR1;
						sSTATEBLOW.fSTATE_VAR2 = sBLOW.fVAR2;

						//	Note : ���� �̻� �߻� ����.
						//
						sSKILLACTEX.sSTATEBLOW = sSTATEBLOW;
					}
				}

				//	Note : ��ų Ư�� ���.
				//
				BOOL bActionMovement = FALSE;
				BOOL bCUREFLAG = FALSE;
				BOOL bREMOVEFLAG = FALSE;
				BOOL bREBIRTH = FALSE;

				if ( bACTDASH )			bActionMovement = TRUE;

				for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
				{
					const SKILL::SSPEC &sSKILL_SPEC = pSkillL->m_sAPPLY.sSpecs[nSpec].sSPEC[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL];
					switch ( pSkillL->m_sAPPLY.sSpecs[nSpec].emSPEC )
					{
					case EMSPECA_PUSHPULL:
						{
							if ( bActionMovement )	break;
							if ( !pACTOR->IsValidBody() )				break;
							if ( RANDOM_POS > sSKILL_SPEC.fVAR2 )		break;	//	Note : �߻�Ȯ�� ����.
							sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR1;
							bActionMovement = TRUE;
						}break;

					case EMSPECA_STIGMA:
						{
							ApplySkillTargetStigma(sTARID, sSKILL_SPEC, pSkillL);	
						}
						break;

						/*push pull skill logic, Juver, 2017/06/04 */
					case EMSPECA_PUSH:
						{
							if ( bActionMovement )			break;
							if ( !pACTOR->IsValidBody() )	break;
							if ( RANDOM_POS > sSKILL_SPEC.fVAR1 )		break;
							sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR2; //distance
							sSKILLACTEX.bPush = true;
							bActionMovement = TRUE;
						}break;

					case EMSPECA_TELEPORT:
						ApplySkillSelfTeleport ( GetGLGaeaServer(), this, pSkillL, pSkill->m_sEXT_DATA.dwADDSKILLLEVEL, pSkillL->m_sAPPLY.sSpecs[nSpec].sSPEC[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL], m_TargetID, m_wTARNUM, m_sTARIDS );
						break;

					case EMSPECA_POSITION_SHIFT:
						//ApplySkillSelfSwapPos ( this, pSkillL, pSkill->m_sEXT_DATA.dwADDSKILLLEVEL, pSkillL->m_sAPPLY.sSpecs[nSpec].sSPEC[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL], m_wTARNUM, m_sTARIDS, GetLandMan() );
						break;

						/*push pull skill logic, Juver, 2017/06/04 */
					case EMSPECA_PULL:
						{
							if ( bActionMovement )			break;
							if ( !pACTOR->IsValidBody() )	break;
							if ( RANDOM_POS > sSKILL_SPEC.fVAR1 )		break;
							sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR2; //pull speed
							sSKILLACTEX.bPull = true;
							bActionMovement = TRUE;
						}break;

						/*dash skill logic, Juver, 2017/06/17 */
					case EMSPECA_ONWARD:
						{
							if ( bActionMovement )			break;
							if ( bACTDASH )					break;
							if ( !pACTOR->IsValidBody() )	break;
							if ( RANDOM_POS > sSKILL_SPEC.fVAR1 )		break;
							sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR2; //dash speed
							sSKILLACTEX.wActionAnim = (WORD)sSKILL_SPEC.dwFLAG; //animation
							sSKILLACTEX.bDash = true;
							bACTDASH = TRUE;
							bActionMovement = TRUE;
						}break;

					case EMSPECA_SKILLLINK:
						ApplySkillTargetSkillLink(m_TargetID, sTARID, pSkillL->m_sAPPLY.sSpecs[nSpec].sSPEC[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL], sSkill);
						break;

					case EMSPECA_HP_DIV:
					case EMSPECA_MP_DIV:
					case EMSPECA_SP_DIV:
						break;

					case EMSPECA_RECBLOW:
						{
							//	Note : ���� �̻� ġ��.
							//
							if ( bCUREFLAG )	break;
							sSKILLACTEX.dwCUREFLAG |= pSkillL->m_sAPPLY.sSpecs[nSpec].sSPEC[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL].dwFLAG;
							bCUREFLAG = TRUE;
						}break;

						/*debuff skill logic, Juver, 2017/06/10 */
					case EMSPECA_BUFF_REMOVE:
						{
							if ( bREMOVEFLAG )	break;
							if ( sSKILLACTEX.bDebuff )	break;
							sSKILLACTEX.dwRemoveFlag = pSkillL->m_sAPPLY.sSpecs[nSpec].sSPEC[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL].dwFLAG;
							sSKILLACTEX.bDebuff = true;
							bREMOVEFLAG = TRUE;
						}break;

					case EMSPECA_REBIRTH:
						{
							if ( bREBIRTH )	break;

							if ( sTARID.emCrow==CROW_PC )
							{
								PGLCHAR pPC = m_pGLGaeaServer->GetChar(sTARID.dwID);
								if ( pPC )
								{
									WORD wRECOVE = pPC->DoRevive2Skill(skill_id,pSkill->m_sEXT_DATA.dwADDSKILLLEVEL, (WORD)nSpec );
									if ( 0<wRECOVE)
									{
										++nEXP_NUM;
										nALL_LVL += pPC->GetLevel ();
										nALL_VARHP += wRECOVE;
										nALL_FULLHP += pPC->GetMaxHP();

										/*pvp tyranny, Juver, 2017/08/24 */
										if ( m_pLandMan && m_pLandMan->m_bPVPTyrannyMap )
											GLPVPTyrannyField::GetInstance().ScoreResu( m_dwCharID );

										/*school wars, Juver, 2018/01/20 */
										if ( m_pLandMan && m_pLandMan->m_bPVPSchoolWarsMap )
											GLPVPSchoolWarsField::GetInstance().ScoreResu( m_dwCharID );

										/*pvp capture the flag, Juver, 2018/02/01 */
										if ( m_pLandMan && m_pLandMan->m_bPVPCaptureTheFlagMap )
											GLPVPCaptureTheFlagField::GetInstance().ScoreResu( m_dwCharID );
									}
								}
							}

							bREBIRTH = TRUE;
						}break;
					};
				}


				switch (pSkillL->m_sAPPLY.emBASIC_TYPE)
				{
				case SKILL::EMFOR_CURE:
					//	Note : ���� �̻� ġ��.
					//
					sSKILLACTEX.dwCUREFLAG |= pSkillL->m_sAPPLY.dwCUREFLAG;
					break;
				};

				if ( sSKILLACTEX.VALID() )
				{
					m_pLandMan->RegSkillActEx ( sSKILLACTEX );
					nALL_VARHP;
				}

				//	Note : ȸ���� �⿩���� ���� ����ġ �߻�.	( ���ݽô� ���� )
				//
				int nSUM_EXP = 0;
				bool bRECOVE = (nALL_VARHP>=0&&nALL_VARMP>=0&&nALL_VARSP&&0) && (nALL_VARHP+nALL_VARMP+nALL_VARSP)>0;

				if ( bRECOVE )
				{
					//	���� ��ȭ���� ���� ���� ����ġ�� ����.
					if ( nALL_VARHP>0 )
						nSUM_EXP += GLOGICEX::GLATTACKEXP(GETLEVEL(),nALL_LVL,nALL_VARHP,nALL_FULLHP,pACTOR->GetBonusExp());

					if ( nALL_VARMP>0 )
						nSUM_EXP += GLOGICEX::GLATTACKEXP(GETLEVEL(),nALL_LVL,nALL_VARMP,nALL_FULLMP,pACTOR->GetBonusExp());

					if ( nALL_VARSP>0 )
						nSUM_EXP += GLOGICEX::GLATTACKEXP(GETLEVEL(),nALL_LVL,nALL_VARSP,nALL_FULLSP,pACTOR->GetBonusExp());

					nSUM_EXP += int (nSUM_EXP*GLCONST_CHAR::fREC_EXP_RATE);
				}

				//	Note : ����ġ ȹ�淮 ����.
				//
				if ( nSUM_EXP )
				{
					// ������ ����Ʈ���� �Է��� ����ġ ���� ���� (����)
					nSUM_EXP = int( (float)nSUM_EXP * GetExpMultipleRateFromSlotItem() );

					m_sExperience.lnNow += nSUM_EXP;
					m_sExperience.LIMIT();
				}
			}

			/*skill summon, Juver, 2017/10/09 */
			if ( m_pGLGaeaServer && pSkillL->m_sSPECIAL_SKILL.emSSTYPE == SKILL::EMSSTYPE_SUMMON )
			{
				SKILL::SSKILL_SUMMON_DATA &summon_data = pSkillL->m_sSPECIAL_SKILL.sSummon[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL];

				SUMMON_DATA_SKILL sdata;
				sdata.bSummon = TRUE;
				sdata.sTarget = m_sTARIDS[0];
				sdata.sidSkill = pSkillL->m_sBASIC.sNATIVEID;

				sdata.Assign( summon_data );
				m_pGLGaeaServer->CreateSummon( sdata, m_dwGaeaID );
			}
		}
	}
}

void GLChar::WeaponSkillProc ()
{
	SITEMCUSTOM& sItemCustom = m_PutOnItems[GetCurRHand()];
	SNATIVEID sSkillID = sItemCustom.sSkillLinkID;
	if ( !sSkillID.IsValidNativeID() )	return;

	if ( sSkillID.IsValidNativeID() )
	{
		float fNowRate = seqrandom::getpercent();
		float fRate = 0.0f;
		std::vector<GLWEAPON_LINKSKILL>::size_type i = 0, j = GLCONST_CHAR::vecWeaponLinkSkill.size();
		for( ; i < j; ++i )
		{
			GLWEAPON_LINKSKILL glSkill = GLCONST_CHAR::vecWeaponLinkSkill[i];
			if (glSkill.sNativeID != sSkillID)
				continue;

			fRate = glSkill.fRate;
			break;
		}

		if(fRate < fNowRate)	return;

		PGLSKILL pSkillW = GLSkillMan::GetInstance().GetData ( sSkillID.wMainID, sSkillID.wSubID );
		if ( pSkillW )
		{
			if ( pSkillW->m_sAPPLY.IsSpec(EMSPECA_INVISIBLE) )
			{
				{
					ReSetSTATE(EM_ACT_RUN);
					GLMSG::SNETPC_MOVESTATE_BRD NetMsgFB;
					NetMsgFB.dwGaeaID = m_dwGaeaID;
					NetMsgFB.dwActState = m_dwActState;
					SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgFB );
					SendToClient ( (NET_MSG_GENERIC*) &NetMsgFB );
				}
			}

			/*dash skill logic, Juver, 2017/06/17 */
			BOOL bACTDASH = FALSE;

			//	Note : ��ų�� ����Ǵ� Ÿ�� �˻�.
			//
			BOOL bAsignStigma = false;
			BOOL bStigmaEff = FALSE;
			STARGETID sTARID;

			if ( pSkillW->m_sBASIC.emIMPACT_SIDE != SIDE_ENEMY )
			{
				const STARID &_sTARID = STARID(CROW_PC,m_dwGaeaID);

				sTARID.emCrow = _sTARID.GETCROW();
				sTARID.dwID = _sTARID.GETID();
				GLACTOR* pACTOR = m_pGLGaeaServer->GetTarget ( m_pLandMan, sTARID );
				if ( !pACTOR )									return;

				//	Note : ��ų ���� ����.
				//
				SSKILLACTEX sSKILLACTEX;
				//	Note : ?? ?? ??.
				//
				sSKILLACTEX.sID = STARGETID(CROW_PC,m_dwGaeaID);
				sSKILLACTEX.sID_TAR = STARGETID(_sTARID.GETCROW(),_sTARID.GETID());
				sSKILLACTEX.fDELAY = pSkillW->m_sEXT_DATA.fDELAY4DAMAGE;

				sSKILLACTEX.idSKILL = sSkillID;
				sSKILLACTEX.wSKILL_LVL = 0;
				
				//	Note : ��ų Ư�� ���.
				//
				BOOL bActionMovement = FALSE;
				BOOL bCUREFLAG = FALSE;
				BOOL bREMOVEFLAG = FALSE;
				BOOL bREBIRTH = FALSE;

				if ( bACTDASH )			bActionMovement = TRUE;

				for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
				{
					const SKILL::SSPEC &sSKILL_SPEC = pSkillW->m_sAPPLY.sSpecs[nSpec].sSPEC[0];
					switch ( pSkillW->m_sAPPLY.sSpecs[nSpec].emSPEC )
					{
					/*push pull skill logic, Juver, 2017/06/04 */
					case EMSPECA_PUSH:
						{
							if ( bActionMovement )			break;
							if ( !pACTOR->IsValidBody() )	break;
							if ( RANDOM_POS > sSKILL_SPEC.fVAR1 )		break;
							sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR2; //distance
							sSKILLACTEX.bPush = true;
							bActionMovement = TRUE;
						}break;

					case EMSPECA_INVISIBLE:
						{
							//if ( IsSTATE(EM_ACT_RUN) )
							{
								ReSetSTATE(EM_ACT_RUN);
								GLMSG::SNETPC_MOVESTATE_BRD NetMsgFB;
								NetMsgFB.dwGaeaID = m_dwGaeaID;
								NetMsgFB.dwActState = m_dwActState;
								SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgFB );
									SendToClient ( (NET_MSG_GENERIC*) &NetMsgFB );
							}
						}
						break;

					case EMSPECA_TELEPORT:
						ApplySkillSelfTeleport ( GetGLGaeaServer(), this, pSkillW, 0, pSkillW->m_sAPPLY.sSpecs[nSpec].sSPEC[0], m_TargetID, m_wTARNUM, m_sTARIDS );
						break;

					case EMSPECA_POSITION_SHIFT:
						//ApplySkillSelfSwapPos ( this, pSkillL, pSkill->m_sEXT_DATA.dwADDSKILLLEVEL, pSkillL->m_sAPPLY.sSpecs[nSpec].sSPEC[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL], m_wTARNUM, m_sTARIDS, GetLandMan() );
						break;

							/*push pull skill logic, Juver, 2017/06/04 */
					case EMSPECA_PULL:
						{
							if ( bActionMovement )			break;
							if ( !pACTOR->IsValidBody() )	break;
							if ( RANDOM_POS > sSKILL_SPEC.fVAR1 )		break;
							sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR2; //pull speed
							sSKILLACTEX.bPull = true;
							bActionMovement = TRUE;
						}break;

						/*dash skill logic, Juver, 2017/06/17 */
					case EMSPECA_ONWARD:
						{
							if ( bActionMovement )			break;
							if ( bACTDASH )					break;
							if ( !pACTOR->IsValidBody() )	break;
							if ( RANDOM_POS > sSKILL_SPEC.fVAR1 )		break;
							sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR2; //dash speed
							sSKILLACTEX.wActionAnim = (WORD)sSKILL_SPEC.dwFLAG; //animation
							sSKILLACTEX.bDash = true;
							bACTDASH = TRUE;
							bActionMovement = TRUE;
						}break;

					case EMSPECA_HP_DIV:
					case EMSPECA_MP_DIV:
					case EMSPECA_SP_DIV:
						break;

					case EMSPECA_RECBLOW:
						{
							//	Note : ���� �̻� ġ��.
							//
							if ( bCUREFLAG )	break;
							sSKILLACTEX.dwCUREFLAG |= pSkillW->m_sAPPLY.sSpecs[nSpec].sSPEC[0].dwFLAG;
							bCUREFLAG = TRUE;
						}break;

						/*debuff skill logic, Juver, 2017/06/10 */
					case EMSPECA_BUFF_REMOVE:
						{
							if ( bREMOVEFLAG )	break;
							if ( sSKILLACTEX.bDebuff )	break;
							sSKILLACTEX.dwRemoveFlag = pSkillW->m_sAPPLY.sSpecs[nSpec].sSPEC[0].dwFLAG;
							sSKILLACTEX.bDebuff = true;
							bREMOVEFLAG = TRUE;
						}break;
					};
				}


				switch (pSkillW->m_sAPPLY.emBASIC_TYPE)
				{
				case SKILL::EMFOR_CURE:
					//	Note : ���� �̻� ġ��.
					//
					sSKILLACTEX.dwCUREFLAG |= pSkillW->m_sAPPLY.dwCUREFLAG;
					break;
				};

				if ( sSKILLACTEX.VALID() )
				{
					m_pLandMan->RegSkillActEx ( sSKILLACTEX );
				}
			}

			else
			{
				for ( WORD i=0; i<m_wTARNUM; ++i )
				{
					const STARID &_sTARID = m_sTARIDS[i];

					sTARID.emCrow = _sTARID.GETCROW();
					sTARID.dwID = _sTARID.GETID();
					GLACTOR* pACTOR = m_pGLGaeaServer->GetTarget ( m_pLandMan, sTARID );
					if ( !pACTOR )									continue;

					//	Note : ��ų ���� ����.
					//
					SSKILLACTEX sSKILLACTEX;
					//sSKILLACTEX.sID = STARGETID(CROW_PC,m_dwGaeaID);
					//sSKILLACTEX.sID_TAR = STARGETID(_sTARID.GETCROW(),_sTARID.GETID());

					//	Note : ?? ?? ??.
					//
						
					sSKILLACTEX.sID = STARGETID(CROW_PC,m_dwGaeaID);
					sSKILLACTEX.sID_TAR = STARGETID(_sTARID.GETCROW(),_sTARID.GETID());
					
					sSKILLACTEX.fDELAY = pSkillW->m_sEXT_DATA.fDELAY4DAMAGE;

					sSKILLACTEX.idSKILL = sSkillID;
					sSKILLACTEX.wSKILL_LVL = 0;

					//	Note : ���� �̻� ����.
					//
					ITEM::SSATE_BLOW sBLOW;
					sBLOW.emTYPE = EMBLOW_NONE;

					if ( pSkillW->m_sAPPLY.emELEMENT==EMELEMENT_ARM )
					{
						SITEM *pITEM = GET_ELMT_ITEM ();
						if ( pITEM )	sBLOW = pITEM->sSuitOp.sBLOW;
					}
					else
					{
						const SKILL::SSTATE_BLOW &sSKILL_BLOW = pSkillW->m_sAPPLY.sSTATE_BLOW[0];

						sBLOW.emTYPE = pSkillW->m_sAPPLY.emSTATE_BLOW;
						sBLOW.fRATE = sSKILL_BLOW.fRATE;
						sBLOW.fLIFE = pSkillW->m_sAPPLY.sDATA_LVL[0].fLIFE;
						sBLOW.fVAR1 = sSKILL_BLOW.fVAR1;
						sBLOW.fVAR2 = sSKILL_BLOW.fVAR2;
					}

					if ( sBLOW.emTYPE!=EMBLOW_NONE )
					{
						//	Note : �߻� Ȯ�� ���.
						//
						short nBLOWRESIST = pACTOR->GETRESIST().GetElement ( STATE_TO_ELEMENT(sBLOW.emTYPE) );
						if ( nBLOWRESIST>GLCONST_CHAR::fRESIST_G )	nBLOWRESIST = GLCONST_CHAR::fRESIST_G;

						//			float fPOWER = GLOGICEX::WEATHER_BLOW_POW ( sBLOW.emTYPE, GLPeriod::GetInstance().GetWeather(), m_pLandMan->IsWeatherActive() );
						DWORD dwWeather = GLPeriod::GetInstance().GetMapWeather( m_pLandMan->GetMapID().wMainID, m_pLandMan->GetMapID().wSubID );
						float fPOWER = GLOGICEX::WEATHER_BLOW_POW ( sBLOW.emTYPE, dwWeather, m_pLandMan->IsWeatherActive() );

						BOOL bBLOW(FALSE);
						if ( !(pACTOR->GETHOLDBLOW()&STATE_TO_DISORDER(sBLOW.emTYPE)) )
						{
							bBLOW = GLOGICEX::CHECKSTATEBLOW ( sBLOW.fRATE * fPOWER, GETLEVEL(), pACTOR->GetLevel(), nBLOWRESIST );
						}

						if ( bBLOW )
						{
							//	Note : �����̻� �߻�.
							//	
							SSTATEBLOW sSTATEBLOW;
							float fLIFE = sBLOW.fLIFE * fPOWER;
							fLIFE = ( fLIFE - (fLIFE*nBLOWRESIST/100.0f*GLCONST_CHAR::fRESIST_G ) );

							sSTATEBLOW.emBLOW = sBLOW.emTYPE;
							sSTATEBLOW.fAGE = fLIFE;
							sSTATEBLOW.fSTATE_VAR1 = sBLOW.fVAR1;
							sSTATEBLOW.fSTATE_VAR2 = sBLOW.fVAR2;

							//	Note : ���� �̻� �߻� ����.
							//
							sSKILLACTEX.sSTATEBLOW = sSTATEBLOW;
						}
					}

					//	Note : ��ų Ư�� ���.
					//
					BOOL bActionMovement = FALSE;
					BOOL bCUREFLAG = FALSE;
					BOOL bREMOVEFLAG = FALSE;
					BOOL bREBIRTH = FALSE;

					if ( bACTDASH )			bActionMovement = TRUE;

					for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
					{
						const SKILL::SSPEC &sSKILL_SPEC = pSkillW->m_sAPPLY.sSpecs[nSpec].sSPEC[0];
						switch ( pSkillW->m_sAPPLY.sSpecs[nSpec].emSPEC )
						{
						case EMSPECA_PUSHPULL:
							{
								if ( bActionMovement )	break;
								if ( !pACTOR->IsValidBody() )				break;
								if ( RANDOM_POS > sSKILL_SPEC.fVAR2 )		break;	//	Note : �߻�Ȯ�� ����.
								sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR1;
								bActionMovement = TRUE;
							}break;

						case EMSPECA_STIGMA:
							{
								ApplySkillTargetStigma(sTARID, sSKILL_SPEC, pSkillW);	
							}
							break;

							/*push pull skill logic, Juver, 2017/06/04 */
						case EMSPECA_PUSH:
							{
								if ( bActionMovement )			break;
								if ( !pACTOR->IsValidBody() )	break;
								if ( RANDOM_POS > sSKILL_SPEC.fVAR1 )		break;
								sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR2; //distance
								sSKILLACTEX.bPush = true;
								bActionMovement = TRUE;
							}break;

						case EMSPECA_TELEPORT:
							ApplySkillSelfTeleport ( GetGLGaeaServer(), this, pSkillW, 0, pSkillW->m_sAPPLY.sSpecs[nSpec].sSPEC[0], m_TargetID, m_wTARNUM, m_sTARIDS );
							break;

						case EMSPECA_POSITION_SHIFT:
							//ApplySkillSelfSwapPos ( this, pSkillL, pSkill->m_sEXT_DATA.dwADDSKILLLEVEL, pSkillL->m_sAPPLY.sSpecs[nSpec].sSPEC[pSkill->m_sEXT_DATA.dwADDSKILLLEVEL], m_wTARNUM, m_sTARIDS, GetLandMan() );
							break;

							/*push pull skill logic, Juver, 2017/06/04 */
						case EMSPECA_PULL:
							{
								if ( bActionMovement )			break;
								if ( !pACTOR->IsValidBody() )	break;
								if ( RANDOM_POS > sSKILL_SPEC.fVAR1 )		break;
								sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR2; //pull speed
								sSKILLACTEX.bPull = true;
								bActionMovement = TRUE;
							}break;

							/*dash skill logic, Juver, 2017/06/17 */
						case EMSPECA_ONWARD:
							{
								if ( bActionMovement )			break;
								if ( bACTDASH )					break;
								if ( !pACTOR->IsValidBody() )	break;
								if ( RANDOM_POS > sSKILL_SPEC.fVAR1 )		break;
								sSKILLACTEX.fPUSH_PULL = sSKILL_SPEC.fVAR2; //dash speed
								sSKILLACTEX.wActionAnim = (WORD)sSKILL_SPEC.dwFLAG; //animation
								sSKILLACTEX.bDash = true;
								bACTDASH = TRUE;
								bActionMovement = TRUE;
							}break;

						case EMSPECA_HP_DIV:
						case EMSPECA_MP_DIV:
						case EMSPECA_SP_DIV:
							break;

						case EMSPECA_RECBLOW:
							{
								//	Note : ���� �̻� ġ��.
								//
								if ( bCUREFLAG )	break;
								sSKILLACTEX.dwCUREFLAG |= pSkillW->m_sAPPLY.sSpecs[nSpec].sSPEC[0].dwFLAG;
								bCUREFLAG = TRUE;
							}break;

							/*debuff skill logic, Juver, 2017/06/10 */
						case EMSPECA_BUFF_REMOVE:
							{
								if ( bREMOVEFLAG )	break;
								if ( sSKILLACTEX.bDebuff )	break;
								sSKILLACTEX.dwRemoveFlag = pSkillW->m_sAPPLY.sSpecs[nSpec].sSPEC[0].dwFLAG;
								sSKILLACTEX.bDebuff = true;
								bREMOVEFLAG = TRUE;
							}break;
						};
					}


					switch (pSkillW->m_sAPPLY.emBASIC_TYPE)
					{
					case SKILL::EMFOR_CURE:
						//	Note : ���� �̻� ġ��.
						//
						sSKILLACTEX.dwCUREFLAG |= pSkillW->m_sAPPLY.dwCUREFLAG;
						break;
					};
					
					if ( sSKILLACTEX.VALID() )
					{
						m_pLandMan->RegSkillActEx ( sSKILLACTEX );
					}
				}
			}
		}
	}
}

// *****************************************************
// Desc: �õ����� ��ų���� ���� ����
// *****************************************************
BOOL GLChar::SkillProcess ( float fElapsedTime )
{
	//CONSOLEMSG_WRITE( "SkillProcess Start [%u]", m_idACTIVESKILL.dwID );

	//	Note : ĳ���Ͱ� ��� ��ų ���� ������.
	//
	WORD wSKILL_LVL;
	SCHARSKILL sSkill;

	if ( IsDefenseSkill() )
	{
		if ( m_idACTIVESKILL != m_sDefenseSkill.m_dwSkillID ) return FALSE;
		sSkill.sNativeID = m_idACTIVESKILL;
		sSkill.wLevel = m_sDefenseSkill.m_wLevel;
		wSKILL_LVL = sSkill.wLevel;
	}
	else
	{
        SKILL_MAP_ITER learniter = m_ExpSkills.find ( m_idACTIVESKILL.dwID );
		if ( learniter==m_ExpSkills.end() )		return FALSE;
		sSkill = (*learniter).second;
		wSKILL_LVL = sSkill.wLevel;
	}

	//	��ų ���� ������.
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_idACTIVESKILL );

	//	�ڽ��� ��ü�ݰ� + ��ųŸ�ٿ��� + ��ų���뿵�� + 20(������)
	float fSkillRange = (float) ( GETBODYRADIUS() + GETSKILLRANGE_TAR(*pSkill) +
		GETSKILLRANGE_APPLY(*pSkill,wSKILL_LVL) + 20 );

	// ��� ������ ����Ÿ���̸� ���� ����
	if ( m_sCONFTING.IsPOWERFULTIME () ) return FALSE;

	//	Note : ��ų�� ����Ǵ� Ÿ�� �˻�.
	//
	DWORD dwVALIDNUM(0); // ��ȿ Ÿ��
	STARGETID sTARID;
	for ( WORD i=0; i<m_wTARNUM; ++i )
	{
		sTARID.emCrow = m_sTARIDS[i].GETCROW();
		sTARID.dwID = m_sTARIDS[i].GETID();
		GLACTOR* pACTOR = m_pGLGaeaServer->GetTarget ( m_pLandMan, sTARID );
		if ( !pACTOR )					continue;

		// ��� ������ ����Ÿ���̸� ���� ����
		PGLCHAR pChar = m_pGLGaeaServer->GetChar ( sTARID.dwID );
		if ( pChar )
		{
			if ( pChar->m_sCONFTING.IsPOWERFULTIME () )
				continue;
		}

		// Ÿ�ٹݰ� + ��ų ��ȿ�ݰ�
		float fReActionRange = (float) ( pACTOR->GetBodyRadius() + fSkillRange );

		// ��ų �ߵ��ڿ� Ÿ�ٰ��� �Ÿ�
		D3DXVECTOR3 vDist = m_vPos - pACTOR->GetPosition();  
		float fDist = D3DXVec3Length(&vDist);

		if ( fReActionRange*GLCONST_CHAR::fREACT_VALID_SCALE < fDist )
		{
			m_sTARIDS[i].wID = USHRT_MAX;
		}
		else
		{
			dwVALIDNUM++;
		}
	}

	// ��ų ��ȿ�ݰ� ���� Ÿ���� ���� ���
	if ( dwVALIDNUM==0 )
	{
		TurnAction ( GLAT_IDLE );

		//	Note : �ڽſ���.
		GLMSG::SNETPC_ATTACK_DAMAGE NetMsg;
		NetMsg.emTarCrow		= m_TargetID.emCrow;
		NetMsg.dwTarID			= m_TargetID.dwID;
		NetMsg.nDamage			= 0;
		m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsg );
	}

	m_fattTIMER += fElapsedTime*GLCHARLOGIC::GETATTVELO();
	m_fattTIMER += GLCHARLOGIC::GETATT_ITEM();

	GLCONST_CHARCLASS &ConstCharClass = GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX];
	VECANIATTACK &vecAniAttack = ConstCharClass.m_ANIMATION[m_SKILLMTYPE][m_SKILLSTYPE];
	if ( vecAniAttack.empty() )	return FALSE;

	//	Note : Ư�� ���� ���ϸ��̼� ���� ����. (������)
	//
	const SANIATTACK &sAniAttack = vecAniAttack[0];

	//
	//
	int nTotalKeys = int(sAniAttack.m_dwETime) - int(sAniAttack.m_dwSTime);
	DWORD dwThisKey = sAniAttack.m_dwSTime + DWORD(m_fattTIMER*UNITANIKEY_PERSEC);
	
	if ( !m_sHITARRAY.empty() )
	{
		SSTRIKE sStrike = *m_sHITARRAY.begin();

		if ( dwThisKey >= sStrike.wDivKey )
		{
			if ( sStrike.bHit )
			{
				//CONSOLEMSG_WRITE( "SkillProcess m_sHITARRAY1 SkillProc[%u]", m_idACTIVESKILL.dwID );
				SkillProc ( m_idACTIVESKILL, sStrike.bLowSP );
				//TurnAction ( GLAT_IDLE );
			}
			else
			{
				//CONSOLEMSG_WRITE( "SkillProcess m_sHITARRAY1 AvoidProc[%u]", m_idACTIVESKILL.dwID );
				AvoidProc ( m_idACTIVESKILL, sStrike.bLowSP );
				//TurnAction ( GLAT_IDLE );
			}

			//	ť ����Ÿ�� ó���� ��� �����Ѵ�.
			m_sHITARRAY.pop_front ();
		}	
	}

	//	�̻����� ���� ����Ÿ�� ���� ó������ �������,
	//	�������� �Ѳ����� ó���ϰ�, FALSE�� �����Ѵ�.
	//
	if ( dwThisKey >= sAniAttack.m_dwETime )
	{
		if ( !m_sHITARRAY.empty() )
		{
			while ( !m_sHITARRAY.empty() )
			{
				SSTRIKE sStrike = *m_sHITARRAY.begin();

				if ( sStrike.bHit )		
				{
					//CONSOLEMSG_WRITE( "SkillProcess m_sHITARRAY2 SkillProc[%u]", m_idACTIVESKILL.dwID );
					SkillProc ( m_idACTIVESKILL, sStrike.bLowSP );
					//TurnAction ( GLAT_IDLE );
				}
				else
				{
					//CONSOLEMSG_WRITE( "SkillProcess m_sHITARRAY2 AvoidProc[%u]", m_idACTIVESKILL.dwID );
					AvoidProc ( m_idACTIVESKILL, sStrike.bLowSP );
					//TurnAction ( GLAT_IDLE );
				}

				//	ť ����Ÿ�� ó���� ��� �����Ѵ�.
				m_sHITARRAY.pop_front ();
			}
		}

		if ( !sAniAttack.IsLOOP() )	return FALSE;

		//	Note : ��ų �ݺ�.
		//
		EMSKILLCHECK emCHECK = GLCHARLOGIC::CHECHSKILL ( m_idACTIVESKILL, 1, IsDefenseSkill() );
		if ( emCHECK != EMSKILL_OK && emCHECK != EMSKILL_NOTSP )
		{
			//	Note : ��ų ���� ���� FB �޽���.
			GLMSG::SNETPC_REQ_SKILL_FB NetMsgFB;
			NetMsgFB.emSKILL_FB = emCHECK;
			m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
			return FALSE;
		}

		//	Note : SKILL ����.
		//
		SETACTIVESKILL ( m_idACTIVESKILL );

		BOOL bLowSP =  (emCHECK==EMSKILL_NOTSP) ? TRUE : FALSE;
		PreStrikeProc ( TRUE, bLowSP );

		//	Note : ��ų ���� �Ҹ𰪵� �Ҹ��Ŵ.
		//
		GLCHARLOGIC::ACCOUNTSKILL ( m_idACTIVESKILL, 1, true );

		m_fattTIMER = 0.0f;
	}

	return TRUE;
}

inline HRESULT GLChar::UpdateClientState ( float fElapsedTime )
{
	//	Note : ������ġ�� Ŭ���̾�Ʈ�� ������ �ϰ� �Ǹ�? �޼��� �߻��� ���ص� �ȴ�.
	//		�� �ణ�� ������ �߻��� ������ ����.
	//
	m_fSTATE_TIMER += fElapsedTime;
	if ( m_fSTATE_TIMER>1.6f )
	{
		m_fSTATE_TIMER = 0.0f;

		//	Note : �ڽ�, ����ڿ��� hp,mp �� ����.
		//
		//	��Ƽ�����Դ� hp,mp �������� �ʴ´�.
		MsgSendUpdateState ( false );
	}

	m_fMBR_TIMER += fElapsedTime;

	if ( m_fMBR_TIMER > 1.6f )
	{
		m_fMBR_TIMER = 0.0f;

		GLPartyFieldMan& sPartyFieldMan = m_pGLGaeaServer->GetPartyMan();
		if ( sPartyFieldMan.GetParty ( m_dwPartyID ) )
		{
			GLMSG::SNET_PARTY_MBR_POS NetMsg;
			NetMsg.dwGaeaID = m_dwGaeaID;
			NetMsg.vPos = m_actorMove.Position();
			NetMsg.sHP = m_sHP;
			NetMsg.sMP = m_sMP;

			sPartyFieldMan.SendMsgToMember ( m_dwPartyID, (NET_MSG_GENERIC*) &NetMsg );
		}
	}

	return S_OK;
}

VOID GLChar::UpdateLandEffect()
{
	std::set<int> setOldLandEffectNum = m_setLandEffectNum;
	std::set<int>::iterator iter_EffNum;
	m_setLandEffectNum.clear();

	SLEVEL_ETC_FUNC *pLevelEtcFunc = m_pLandMan->GetLevelEtcFunc();
	if( !pLevelEtcFunc ) return;
	if( !pLevelEtcFunc->m_bUseFunction[EMETCFUNC_LANDEFFECT] ) return;

	int nActiveNum = 0;
	for( int i=0; i<EMLANDEFFECT_MULTI; ++i )
	{
		if ( m_sLandEffect[i].IsUse() )
			nActiveNum ++;
	}

	for( int i = 0; i < (int)pLevelEtcFunc->m_vecLandEffect.size(); i++ )
	{
		SLANDEFFECT landEffect = pLevelEtcFunc->m_vecLandEffect[i];
		if( m_vPos.x > landEffect.vMinPos.x &&
			m_vPos.z > landEffect.vMinPos.y &&
			m_vPos.x < landEffect.vMaxPos.x &&
			m_vPos.z < landEffect.vMaxPos.y )
		{
			m_setLandEffectNum.insert(i);
		}
	}

	BOOL bUpdateLandEff = FALSE;
	if( setOldLandEffectNum.size() != m_setLandEffectNum.size() )
	{
		bUpdateLandEff = TRUE;
	}else{
		iter_EffNum = m_setLandEffectNum.begin();
		for( ; iter_EffNum != m_setLandEffectNum.end(); ++iter_EffNum )
		{
			if( setOldLandEffectNum.find( *iter_EffNum ) == setOldLandEffectNum.end() )
			{
				bUpdateLandEff = TRUE; 
				break;
			}
		}
	}

	if ( nActiveNum != (int)m_setLandEffectNum.size() )
	{
		//CONSOLEMSG_WRITE( "%d %s land effect update", m_dwCharID, m_szName );
		bUpdateLandEff = TRUE;
	}

	if( bUpdateLandEff )
	{
		DISABLEALLLANDEFF();

		GLMSG::SNETPC_LANDEFFECT NetMsg;
		int iNum = 0;
		iter_EffNum = m_setLandEffectNum.begin();
		for( ; iter_EffNum != m_setLandEffectNum.end(); ++iter_EffNum )
		{
			SLANDEFFECT landEffect = pLevelEtcFunc->FindLandEffect(*iter_EffNum);
			if( !landEffect.IsUse() ) continue;
			ADDLANDEFF( landEffect, iNum );
			NetMsg.sLandEffect[iNum] = landEffect;
			iNum++;
		}

		NetMsg.dwGaeaID		= m_dwGaeaID;	
		m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsg );
		SendMsgViewAround( (NET_MSG_GENERIC*) &NetMsg ); 	
	}


	// ��� ������ ����ؾ��� ����� ó��
	for( int i = 0; i < EMLANDEFFECT_MULTI; i++ )
	{
		SLANDEFFECT landEffect = m_sLandEffect[i];
		if( !landEffect.IsUse() ) continue;
		if( landEffect.emLandEffectType == EMLANDEFFECT_CANCEL_ALLBUFF )
		{
			DisableSkillFact();
			break;
		}
	}
}

BOOL GLChar::ShockProcess ( float fElapsedTime )
{
	GLCONST_CHARCLASS &ConstCharClass = GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX];
	VECANIATTACK &vecShock = ConstCharClass.m_ANIMATION[AN_SHOCK][AN_SUB_NONE];
	if ( vecShock.empty() )	return FALSE;

	m_fIdleTime += fElapsedTime * ( GETMOVEVELO() + GETMOVE_ITEM() );
			
	SANIATTACK &sAniShock = vecShock[0];

	int wTotalKeys = int(sAniShock.m_dwETime) - int(sAniShock.m_dwSTime);
	int wThisKey = int(m_fIdleTime*UNITANIKEY_PERSEC);
	
	if ( wThisKey >= wTotalKeys )	return FALSE;

	return TRUE;
}

float GLChar::GetMoveVelo ()
{
	float fDefaultVelo = IsSTATE(EM_ACT_RUN) ? GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX].fRUNVELO : GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX].fWALKVELO;
	float fMoveVelo = fDefaultVelo * ( GLCHARLOGIC::GETMOVEVELO() + GLCHARLOGIC::GETMOVE_ITEM() ) ;
	return fMoveVelo;
}

void GLChar::ResetConfront ( EMCONFRONT_END emEND )
{
	//	Note : ü�� ȯ��.
	//
	DoConftStateRestore();

	//	Note : ��� ���Ḧ (�ڽ�) ������Ʈ�� �˸�.
	GLMSG::SNETPC_CONFRONT_END2_AGT NetMsgMyAgt;
	NetMsgMyAgt.emEND = emEND;
	m_pGLGaeaServer->SENDTOAGENT ( m_dwClientID, &NetMsgMyAgt );

	//	Note : (�ڽ�) Ŭ���̾�Ʈ��.
	GLMSG::SNETPC_CONFRONT_END2_CLT NetMsgMyClt;
	NetMsgMyClt.emEND = emEND;
	m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsgMyClt );

	//	��Ƽ�� �ִ� '���' ���������� ����.
	GLMSG::SNETPC_CONFRONT_END2_CLT_MBR NetMsgTarMbr;

	switch ( m_sCONFTING.emTYPE )
	{
	case EMCONFT_PARTY:
		{
			GLPARTY_FIELD *pParty = m_pGLGaeaServer->GetParty(m_dwPartyID);
			if ( pParty )
			{
				NetMsgTarMbr.dwID = m_dwGaeaID;

				//	��� ����Ʈ���� ����.
				pParty->MBR_CONFRONT_LEAVE(m_dwGaeaID);

				//	��Ƽ������ ��ÿ��� ���� ������ �˸�.
				m_pGLGaeaServer->SENDTOPARTYCLIENT ( m_dwPartyID, &NetMsgTarMbr );

				//	��� ���� ��Ƽ������.
				GLPARTY_FIELD *pPartyConft = m_pGLGaeaServer->GetParty ( pParty->m_dwconftPARTYID );
				if ( pPartyConft )
				{
					//	��Ƽ������ ��ÿ��� ���� ������ �˸�.
					m_pGLGaeaServer->SENDTOPARTYCLIENT ( pPartyConft->m_dwPARTYID, &NetMsgTarMbr );
				}
			}
		}
		break;

	case EMCONFT_GUILD:
		{
			GLClubMan &cClubMan = m_pGLGaeaServer->GetClubMan();
			GLCLUB *pCLUB = cClubMan.GetClub ( m_dwGuild );
			if ( pCLUB )
			{
				NetMsgTarMbr.dwID = m_dwCharID;

				//	Note : �ڽ��� Ŭ�� �������� ������� ������� ����.
				pCLUB->DELCONFT_MEMBER ( m_dwCharID );

				//	Note : �ڽ��� Ŭ��������.
				m_pGLGaeaServer->SENDTOCLUBCLIENT ( m_dwGuild, &NetMsgTarMbr );

				//	Note : ���� Ŭ��������.
				GLCLUB *pTAR_CLUB = cClubMan.GetClub ( m_sCONFTING.dwTAR_ID );
				if ( pTAR_CLUB )
				{
					m_pGLGaeaServer->SENDTOCLUBCLIENT ( m_sCONFTING.dwTAR_ID, &NetMsgTarMbr );
				}
			}
		}
		break;
	};

	//	Note : �ֺ� ���ֵ鿡 ������� �뺸.
	//
	GLMSG::SNETPC_CONFRONT_END2_CLT_BRD NetMsgTarBrd;
	NetMsgTarBrd.dwGaeaID = m_dwGaeaID;
	NetMsgTarBrd.emEND = emEND;
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgTarBrd );

	m_sCONFTING.RESET ();
	m_sCONFTING.SETPOWERFULTIME ( 10.0f );

	/*skill summon, Juver, 2017/10/10 */
	if ( m_pGLGaeaServer )
	{
		for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		{
			PGLSUMMONFIELD pMySummon = m_pGLGaeaServer->GetSummon( m_dwSummonGUID_FLD[i] );
			if ( pMySummon && pMySummon->IsValid() )
			{
				m_pGLGaeaServer->DropOutSummon ( m_dwSummonGUID_FLD[i] );
			}
		}
	}	
}

void GLChar::VietnamGainCalculate()
{

	if ( IsSTATE(EM_ACT_WAITING) )		return; 
	if ( m_dwVietnamGainType == GAINTYPE_EMPTY )		return;
	// �߱� ���� �ð� �� ���� ����
	CTimeSpan gameTime( 0, (int)m_sVietnamSystem.gameTime / 60, (int)m_sVietnamSystem.gameTime % 60, 0 );
	CTime	  crtTime     = CTime::GetCurrentTime();
	CTimeSpan crtGameSpan, crtGame15Span;
	CTime	  loginTime   = m_sVietnamSystem.loginTime;
	crtGameSpan			  = gameTime + ( crtTime - loginTime );

//	GLGaeaServer & glGaeaServer = m_pGLGaeaServer;


	LONGLONG totalHours	  = crtGameSpan.GetTotalHours();


	// 5�ð� �̻��̸� ������� �ʰ� �Ѿ��.
	// ���� ����� �ٲ�� �޽����� ����
	if( totalHours >= 5 && m_dwVietnamGainType != GAINTYPE_EMPTY )
	{
		m_dwVietnamGainType = GAINTYPE_EMPTY;	

		GLMSG::SNETPC_VIETNAM_GAINTYPE NetMsg;
		NetMsg.dwGainType = (BYTE)GAINTYPE_EMPTY;
		m_pGLGaeaServer->SENDTOCLIENT( m_dwClientID, &NetMsg );
		// ���� 50%
	}else if( totalHours >= 3 && m_dwVietnamGainType != GAINTYPE_HALF )
	{	
		m_dwVietnamGainType = GAINTYPE_HALF;	

		GLMSG::SNETPC_VIETNAM_GAINTYPE NetMsg;
		NetMsg.dwGainType = (BYTE)GAINTYPE_HALF;
		m_pGLGaeaServer->SENDTOCLIENT( m_dwClientID, &NetMsg );
	}
	m_sVietnamSystem.currentGameTime = totalHours;
}

void GLChar::ChinaGainCalculate()
{
	if ( IsSTATE(EM_ACT_WAITING) )		return; 
	// 18�� �̻��̸� �����Ѵ�.
	if( m_sChinaTime.userAge == 1 ) return;

//	GLGaeaServer & glGaeaServer = m_pGLGaeaServer;

	// �߱� ���� �ð� �� ���� ����
	CTimeSpan gameTime( 0, (int)m_sChinaTime.gameTime / 60, (int)m_sChinaTime.gameTime % 60, 0 );
	CTime	  crtTime     = CTime::GetCurrentTime();
	CTimeSpan crtGameSpan, crtGame15Span;
	CTime	  loginTime   = m_sChinaTime.loginTime;
	crtGameSpan			  = gameTime + ( crtTime - loginTime );

	// ���� 0%
	//if( m_ChinaGainType == 2 )
	//{
	//	crtGame15Span = crtTime - m_China15Time;
	//	// 15���� ������ �޽����� ����
	//	if( crtGame15Span.GetTotalMinutes() > 15 )
	//	{
	//		m_China15Time	= CTime::GetCurrentTime();

	//		GLMSG::SNETPC_CHINA_GAINTYPE NetMsg;
	//		NetMsg.dwGainType = 3;
	//		m_pGLGaeaServer->SENDTOCLIENT( m_dwClientID, &NetMsg );
	//	}
	//}else
	if( m_ChinaGainType != 2 )
	{
		LONGLONG totalHours	  = crtGameSpan.GetTotalHours();
//		LONGLONG totalMinutes = crtGameSpan.GetTotalMinutes();
//		LONGLONG totalSecond  = crtGameSpan.GetTotalSeconds();

		// ���� ����� �ٲ�� �޽����� ����
		if( totalHours >= 5 && m_ChinaGainType != GAINTYPE_EMPTY )
			//if( totalSecond >= 60 && m_ChinaGainType != 2 )
		{
			m_China15Time	= CTime::GetCurrentTime();
			m_ChinaGainType = GAINTYPE_EMPTY;	

			GLMSG::SNETPC_CHINA_GAINTYPE NetMsg;
			NetMsg.dwGainType = GAINTYPE_EMPTY;
			m_pGLGaeaServer->SENDTOCLIENT( m_dwClientID, &NetMsg );
			// ���� 50%
		}else if( totalHours >= 3 && m_ChinaGainType != GAINTYPE_HALF  )
			//}else if( totalSecond >= 50 && m_ChinaGainType != 2 )
		{	
			m_ChinaGainType = GAINTYPE_HALF;

			GLMSG::SNETPC_CHINA_GAINTYPE NetMsg;
			NetMsg.dwGainType = GAINTYPE_HALF;
			m_pGLGaeaServer->SENDTOCLIENT( m_dwClientID, &NetMsg );
		}else if( totalHours >= 1 && m_ChinaGainType == 255 )
			//}else if( totalSecond >= 40 && m_ChinaGainType != 2 )
		{
			m_ChinaGainType = 0;

			GLMSG::SNETPC_CHINA_GAINTYPE NetMsg;
			NetMsg.dwGainType = 4;
			m_pGLGaeaServer->SENDTOCLIENT( m_dwClientID, &NetMsg );
		}
	}
	m_sChinaTime.currentGameTime = crtGameSpan.GetTotalMinutes();
}

void GLChar::EventCalculate()
{
	if ( IsSTATE(EM_ACT_WAITING) )		return; 

	SEventState sEventState = m_pGLGaeaServer->m_sEventState;

	if( sEventState.bEventStart == FALSE ) return;

	// �̺�Ʈ ���� ������ �ƴϸ� ����
	if( m_bEventApply == FALSE ) return;

	CTime	  crtTime     = CTime::GetCurrentTime();
	CTimeSpan crtGameSpan;
	CTime	  loginTime			 = m_sEventTime.loginTime;
	crtGameSpan					 = ( crtTime - loginTime );
	m_sEventTime.currentGameTime = crtGameSpan.GetTimeSpan();

	// �̺�Ʈ�� ���� ���� ��
	if( m_bEventStart == FALSE )
	{
		// �׽�Ʈ �ÿ� Second�� �Ѵ�.
		if( crtGameSpan.GetTotalSeconds() >= sEventState.EventPlayTime )
		{
			m_bEventStart = TRUE;

			//	Note : �ڽ��� Ŭ���̾�Ʈ��.
			GLMSG::SNET_GM_LIMIT_EVENT_APPLY_START NetMsg;
			NetMsg.loginTime = m_sEventTime.loginTime;
			//dmk14 btg info
			NetMsg.expGain_Rate = sEventState.fExpGainRate;
			NetMsg.itemGain_Rate = sEventState.fItemGainRate;

			m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*)&NetMsg );

		}
	}

	if( m_bEventStart == TRUE )
	{
		if( crtGameSpan.GetTotalSeconds() >= sEventState.EventPlayTime + sEventState.EventBusterTime )
		{
			m_bEventStart = FALSE;
			m_sEventTime.loginTime = crtTime.GetTime();

			// ���� ���ŵ� �̺�Ʈ �ð�
			GLMSG::SNET_GM_LIMIT_EVENT_RESTART NetMsg;
			NetMsg.restartTime = crtTime.GetTime();

			m_pGLGaeaServer->SENDTOAGENT(m_dwClientID,&NetMsg);

			//	Note : �ڽ��� Ŭ���̾�Ʈ��.
			GLMSG::SNET_GM_LIMIT_EVENT_APPLY_END NetMsg2;
			NetMsg2.loginTime = m_sEventTime.loginTime;
			m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*)&NetMsg2 );
		}
	}




}

HRESULT GLChar::FrameMove ( float fTime, float fElapsedTime )
{
	GASSERT(m_pLandMan);

//	GLGaeaServer & glGaeaServer = m_pGLGaeaServer;

#if defined(VN_PARAM) //vietnamtest%%%
	// ��Ʈ�� Ž�� ���� ���
	VietnamGainCalculate();
#endif

#ifdef CH_PARAM_USEGAIN
	// �߱� Ž�� ���� ���
	ChinaGainCalculate();
#else
	// ����Ʈ �̺�Ʈ ���
	EventCalculate();
#endif

	//PANDORA
	if( m_sPandoraStatus.fReqDelay > 0.0f )
        m_sPandoraStatus.fReqDelay -= fElapsedTime;

	//fastrevive fix
	if ( m_bDeathTimer )
	{
		float fDeathTime = 4.0f;
		if( m_pLandMan )
		{
			if( m_pLandMan->m_bPVPTyrannyMap )			fDeathTime = TYRANNY_REVIVE_TIME;
			if( m_pLandMan->m_bPVPSchoolWarsMap )		fDeathTime = SCHOOLWARS_REVIVE_TIME;
			if( m_pLandMan->m_bPVPCaptureTheFlagMap )	fDeathTime = CAPTURE_THE_FLAG_REVIVE_TIME;
		}
		m_fDeathTimer += fElapsedTime;
		if ( m_fDeathTimer >= fDeathTime )
		{
			m_bDeathTimer = FALSE;
			m_fDeathTimer = 0.0f;
		}
	}

	/*dmk14 reform auto reset the reform process when stock*/
	if ( m_bIsReformProcess )
	{
		m_fReformProcessTimer += fElapsedTime;
		if( m_fReformProcessTimer >= 1.0f )
		{
			m_bIsReformProcess = false;
			m_fReformProcessTimer = 0.0f;
		}
	}

	if ( !IsSTATE(EM_ACT_WAITING) && m_bEntryFailed )
	{
		//  ���������� �����ʾ� �ٸ� ������ �̵� �ؾ��ϴ� ��� �޽����� ������.
		GLMSG::SNETENTRY_FAILED NetMsg;
		m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsg );

		m_bEntryFailed = FALSE;
	}

	

	// Ż�� ���͸� üũ
	UpdateVehicle( fElapsedTime );


	if ( !IsSTATE(EM_GETVA_AFTER) && !(m_pGLGaeaServer->IsReserveServerStop ()) )	return S_FALSE;

	

	m_fAge += fElapsedTime;
	m_fGenAge += fElapsedTime;

	m_fSkillDelay += fElapsedTime;


	/*continuous damage skill logic, Juver, 2017/06/10 */
	if ( m_pLandMan )
	{
		for( CONTINUOUS_DAMAGE_DATA_MAP_ITER iterdamage = m_mapContinuousDamage.begin();
			iterdamage != m_mapContinuousDamage.end(); ++iterdamage )
		{
			SCONTINUOUS_DAMAGE_DATA& sContinuousDamageData = (*iterdamage).second;
			sContinuousDamageData.fAge += fElapsedTime;

			if ( sContinuousDamageData.fAge >= sContinuousDamageData.fInterval )
			{
				SSKILLACT sSKILLACTCD;
				sSKILLACTCD.sID.dwID = sContinuousDamageData.dwCrowID;
				sSKILLACTCD.sID.emCrow = (EMCROW)sContinuousDamageData.wCrow;
				sSKILLACTCD.sID.vPos = m_vPos;
				sSKILLACTCD.sID_TAR.dwID = m_dwGaeaID;
				sSKILLACTCD.sID_TAR.emCrow = GETCROW();
				sSKILLACTCD.sID_TAR.vPos = m_vPos;

				sSKILLACTCD.fDELAY = 0.0f;
				sSKILLACTCD.emAPPLY = SKILL::EMAPPLY_PHY_LONG;
				sSKILLACTCD.nVAR_HP = (int)-sContinuousDamageData.fVar;
				sSKILLACTCD.dwDamageFlag = DAMAGE_TYPE_NONE;		

				if ( sSKILLACTCD.VALID() )
				{
					m_pLandMan->RegSkillAct ( sSKILLACTCD );
				}

				sContinuousDamageData.fAge = 0.0f;
			}
		}

		m_fContinuousDamageCheck += fElapsedTime;
		if ( m_fContinuousDamageCheck >= 1.0f )
		{
			for( CONTINUOUS_DAMAGE_DATA_MAP_ITER iterdamage = m_mapContinuousDamage.begin();
				iterdamage != m_mapContinuousDamage.end(); ++iterdamage )
			{
				SCONTINUOUS_DAMAGE_DATA sContinuousDamageData = (*iterdamage).second;

				BOOL _bFIND = FALSE;
				for( int _sf=0; _sf<SKILLFACT_SIZE; ++_sf )
				{
					if ( m_sSKILLFACT[_sf].sNATIVEID != NATIVEID_NULL() &&
						m_sSKILLFACT[_sf].sNATIVEID == sContinuousDamageData.sidSkill )
						_bFIND  = TRUE;
				}

				if ( !_bFIND )
					CONTINUOUSDAMAGE_DELETE( sContinuousDamageData.sidSkill );
			}

			m_fContinuousDamageCheck = 0.0f;
		}
	}

	/*vehicle booster system, Juver, 2017/08/11 */
	if ( m_bVehicle )
	{
		if ( m_bBoosterCharge )
		{
			m_fBoosterTimer += fElapsedTime;
			if ( m_fBoosterTimer >= GLCONST_CHAR::fVehicleBoosterChargeTime )
			{
				VehicleBoosterStateCharge(false);
			}
		}

		if ( !m_bBoosterCharge && m_bBoosterStart )
		{
			m_fBoosterTimer -= fElapsedTime;
			if ( m_fBoosterTimer <= 0.0f )
			{
				VehicleBoosterStateStart(false);
				VehicleBoosterStateCharge(true);
			}
		}
	}
	
	/*block back damage, Juver, 2017/12/12 */
	if ( m_bBlockBackDamage && RANPARAM::bFeatureBlockDamageBack )
	{
		m_fBlockBackDamageTime += fElapsedTime;
		if ( m_fBlockBackDamageTime >= RANPARAM::fFeatureBlockDamageBackTimer )
		{
			m_bBlockBackDamage = false;
			m_fBlockBackDamageTime = 0.0f;
		}
	}

	/*clubwar verify non push or pull, Juver, 2017/12/13 */
	if ( m_bClubWarVerify )
	{
		m_fClubWarVerifyTimer += fElapsedTime;
		if ( m_fClubWarVerifyTimer >= float( GLCONST_CHAR::fCDCERTIFY_TIME + 10.0f ) )
		{
			m_bClubWarVerify = FALSE;
			m_fClubWarVerifyTimer = 0.0f;
		}
	}

	/*hackshield implementation, Juver, 2018/06/21 */
	//hs_heartbeat_timer += fElapsedTime;
	//CString strTextGen = "";
	//if ( hs_heartbeat_timer >= RANPARAM::hs_monitor_check_time )
	//{

	//	/*GLMSG::SNET_SERVER_GENERALCHAT NetMsgGen;
	//	strTextGen.Format( "Barugo ka %d",RANPARAM::hs_monitor_check_time );
	//	NetMsgGen.SETTEXT ( strTextGen.GetString() );
	//	NetMsgGen.bNotify = true;
	//	NetMsgGen.iColor = 7;
	//	GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsgGen );*/

	//	hs_heartbeat_timer = 0.0f;
	//	hs_client_disconnect();
	//}

	//	Ʈ���̵� ���� ����.
	if ( m_sTrade.Valid() )
	{
		m_sTrade.FrameMove ( fTime, fElapsedTime );

		PGLCHAR pChar = m_pGLGaeaServer->GetChar ( m_sTrade.GetTarget() );
		if ( !pChar )
		{
			GLMSG::SNET_TRADE_CANCEL_TAR NetMsg;
			NetMsg.emCancel = EMTRADE_CANCEL_NORMAL;

			m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsg );

			m_sTrade.Reset ();
		}
	}

	//	DB�� ���� ������ �ð��� �Ǿ����� �˻��Ŀ� ���� ����.
	//
	m_fSAVEDB_TIMER += fElapsedTime;
	if ( m_fSAVEDB_TIMER > 1800.0f )
	{
		m_fSAVEDB_TIMER = 0.0f;
		m_pGLGaeaServer->SaveCharDB ( m_dwGaeaID );
	}

	if ( m_sCONFTING.IsCONFRONTING() )
	{
		//	Note : ��� ��ȿ�� �˻�.
		//
		bool bvalid(false);
		PGLCHAR pTAR_ONE(NULL);
		switch ( m_sCONFTING.emTYPE )
		{
		case EMCONFT_ONE:
			{
				pTAR_ONE = m_pGLGaeaServer->GetChar ( m_sCONFTING.dwTAR_ID );
				if ( pTAR_ONE )		bvalid = true;
			}
			break;

		case EMCONFT_PARTY:
			{
				bool bpartyA(false), bpartyB(false);
				if ( PARTY_NULL!=m_dwPartyID && m_pGLGaeaServer->GetParty(m_dwPartyID) )						bpartyA = true;
				if ( PARTY_NULL!=m_sCONFTING.dwTAR_ID && m_pGLGaeaServer->GetParty(m_sCONFTING.dwTAR_ID) )	bpartyB = true;

				if ( bpartyA && bpartyB )	bvalid = true;
			}
			break;

		case EMCONFT_GUILD:
			{
				GLClubMan &sClubMan = m_pGLGaeaServer->GetClubMan();

				bool bclubA(false), bclubB(false);
				if ( CLUB_NULL!=m_dwGuild && sClubMan.GetClub(m_dwGuild) )							bclubA = true;
				if ( CLUB_NULL!=m_sCONFTING.dwTAR_ID && sClubMan.GetClub(m_sCONFTING.dwTAR_ID) )	bclubB = true;

				if ( bclubA && bclubB )	bvalid = true;
			}
			break;
		};
		

		if ( !bvalid )
		{
			ResetConfront ( EMCONFRONT_END_FAIL );
		}
		else
		{
			bool bFIGHT = m_sCONFTING.UPDATE(fElapsedTime);
			if ( bFIGHT )
			{
				//	Note : ��� FIGHT �˸�.
				//
				GLMSG::SNETPC_CONFRONT_FIGHT2_CLT NetMsg;
				m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsg );

				/* pet duel use bug Juver 2017/06/27 */
				PGLPETFIELD pMyPet = m_pGLGaeaServer->GetPET ( m_dwPetGUID );
				if ( pMyPet && pMyPet->IsValid () ) 
				{
					m_pGLGaeaServer->DropOutPET ( pMyPet->m_dwGUID, false, false );

					/*pet skill check, Juver, 2017/12/01 */
					CDebugSet::ToFileWithTime( "_petcheck.txt", "[%u]%s pet activated in duel start", m_dwCharID, m_szName );
				}
			}

			switch ( m_sCONFTING.emTYPE )
			{
			case EMCONFT_ONE:
				{
					if ( m_sCONFTING.sOption.bBOUND )
					{
						D3DXVECTOR3 vDistance = m_vPos - m_sCONFTING.vPosition;
						float fDistance = D3DXVec3Length ( &vDistance );

						if ( GLCONST_CHAR::fCONFRONT_ONE_DIST < fDistance )
						{
							ReceiveLivingPoint ( GLCONST_CHAR::nCONFRONT_LOSS_LP );

							ResetConfront ( EMCONFRONT_END_DISLOSS );
							pTAR_ONE->ResetConfront ( EMCONFRONT_END_DISWIN );
						}
					}

					//	����� '����'�� ���ؼ� ��ҵ��� ���.
					if ( !m_sCONFTING.IsCONFRONTING() )		break;

					//	Note : ���� ���� ��Ƽ���� �˻�.
					//
					if ( GetPartyID()!=PARTY_NULL && pTAR_ONE->GetPartyID()==GetPartyID() )
					{
						ResetConfront ( EMCONFRONT_END_PARTY );
					}
				}
				break;

			case EMCONFT_PARTY:
				{
					if ( m_sCONFTING.sOption.bBOUND )
					{
						D3DXVECTOR3 vDistance = m_vPos - m_sCONFTING.vPosition;
						float fDistance = D3DXVec3Length ( &vDistance );

						if ( GLCONST_CHAR::fCONFRONT_PY_DIST < fDistance )
						{
							ReceiveLivingPoint ( GLCONST_CHAR::nCONFRONT_LOSS_LP );

							ResetConfront ( EMCONFRONT_END_DISLOSS );
						}
					}
				}
				break;

			case EMCONFT_GUILD:
				{
					if ( m_sCONFTING.sOption.bBOUND )
					{
						D3DXVECTOR3 vDistance = m_vPos - m_sCONFTING.vPosition;
						float fDistance = D3DXVec3Length ( &vDistance );

						if ( GLCONST_CHAR::fCONFRONT_CLB_DIST < fDistance )
						{
							ReceiveLivingPoint ( GLCONST_CHAR::nCONFRONT_CLB_LOSS_LP );

							ResetConfront ( EMCONFRONT_END_DISLOSS );
						}
					}
				}
				break;
			};
		}
	}
	// ��������� ������ �ð� ����
	else if ( m_sCONFTING.IsPOWERFULTIME () )
	{
		m_sCONFTING.UPDATEPWRFULTIME ( fElapsedTime );
	}

	//	Note : ���� �������� Ÿ�̸� ���� �� ����.
	{
		GLMSG::SNETPC_PLAYERKILLING_DEL NetMsgDel;		
		SPLAYHOSTILE* pHOSTILE = NULL;
		for ( MAPPLAYHOSTILE_ITER pos = m_mapPlayHostile.begin() ; pos!=m_mapPlayHostile.end() ; )
		{
			pHOSTILE = pos->second;
			if( !pHOSTILE )
			{
				//	Note : ������ ���� �˸�.				
				NetMsgDel.dwCharID = pos->first;
				m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID (), &NetMsgDel );

				CDebugSet::ToLogFile( "GLChar::FrameMove, pos->second = NULL" );
				m_mapPlayHostile.erase( pos++ );
				continue;
			}

			pHOSTILE->fTIME -= fElapsedTime;
			if ( pHOSTILE->fTIME < 0 )
			{
				//	Note : ������ ���� �˸�.				
				NetMsgDel.dwCharID = pos->first;
				m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID (), &NetMsgDel );

				SAFE_DELETE( pHOSTILE );
				m_mapPlayHostile.erase( pos++ );
				continue;
			}
			++pos;
		}

		if ( IsValidBody() && m_nBright < 0 )
		{
			m_fBRIGHT_TIMER += fElapsedTime;
			if ( m_fBRIGHT_TIMER >= GLCONST_CHAR::fPK_POINT_DEC_RATE )
			{
				m_fBRIGHT_TIMER = 0.0f;

				//	Note : �Ӽ���ġ ��ȭ.
				m_nBright += 1;

				GLMSG::SNETPC_UPDATE_BRIGHT NetMsg;
				NetMsg.nBright = m_nBright;
				m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsg);

				GLMSG::SNETPC_UPDATE_BRIGHT_BRD NetMsgBrd;
				NetMsgBrd.dwGaeaID = m_dwGaeaID;
				NetMsgBrd.nBright = m_nBright;
				SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
			}
		}
	}

	//	�ð� ����.
	//
	m_fPERIOD_TIMER += fElapsedTime;
	if ( m_fPERIOD_TIMER > 360.0f )
	{
		m_fPERIOD_TIMER = 0.0f;
		MsgSendPeriod ();
	}

	m_fITEM_TIMER += fElapsedTime;
	if ( m_fITEM_TIMER > 1800.0f )
	{
		m_fITEM_TIMER = 0.0f;
		RESET_TIMELMT_ITEM ();
		RESET_CHECK_ITEM ();
	}

	/*pet skill check, Juver, 2017/12/01 */
	m_fPetSkillCheckTimer += fElapsedTime;
	if ( m_fPetSkillCheckTimer >= PET_SKILL_CHECK_TIMER )
	{
		m_fPetSkillCheckTimer = 0.0f;

		BOOL bValidPet = FALSE;
		PGLPETFIELD pMyPet = m_pGLGaeaServer->GetPET ( m_dwPetGUID );
		if ( pMyPet && pMyPet->IsValid ())	bValidPet = TRUE;

		/*dual pet skill, Juver, 2017/12/27 */
		if ( !bValidPet && ( m_bProtectPutOnItem || m_sPETSKILLFACT_A.sNATIVEID.dwID != NATIVEID_NULL().dwID ) )
		{
			CDebugSet::ToFileWithTime( "_petcheck.txt", "[%u]%s pet skill activated on pet not valid m_bProtectPutOnItem:%d m_sPETSKILLFACT_A:%u~%u", 
				m_dwCharID, m_szName, m_bProtectPutOnItem, m_sPETSKILLFACT_A.sNATIVEID.wMainID, m_sPETSKILLFACT_A.sNATIVEID.wSubID );

			m_sPETSKILLFACT_A.RESET ();
			m_bProtectPutOnItem = false;
		}

		/*dual pet skill, Juver, 2017/12/27 */
		if ( !bValidPet && ( m_bProtectPutOnItem || m_sPETSKILLFACT_B.sNATIVEID.dwID != NATIVEID_NULL().dwID ) )
		{
			CDebugSet::ToFileWithTime( "_petcheck.txt", "[%u]%s pet skill activated on pet not valid m_bProtectPutOnItem:%d m_sPETSKILLFACT_B:%u~%u", 
				m_dwCharID, m_szName, m_bProtectPutOnItem, m_sPETSKILLFACT_B.sNATIVEID.wMainID, m_sPETSKILLFACT_B.sNATIVEID.wSubID );

			m_sPETSKILLFACT_B.RESET ();
			m_bProtectPutOnItem = false;
		}
	}

	/*qbox check, Juver, 2017/12/05 */
	m_fQBoxCheckTimer += fElapsedTime;
	if ( m_fQBoxCheckTimer >= QUESTION_BOX_CHECK_TIMER )
	{
		m_fQBoxCheckTimer = 0.0f;

		if ( m_sQITEMFACT.IsACTIVE() && m_pLandMan )
		{
			if ( !m_sQITEMFACT.bMobGen && !m_pLandMan->IsQBoxEnabled() )
			{
				CDebugSet::ToFileWithTime( "_qbox_check.txt", "[%d]%s qbox running in non qbox map", m_dwCharID, m_szName );

				m_sQITEMFACT.RESET();

				GLMSG::SNETPC_QITEMFACT_END_BRD	NetMsgBrd;
				NetMsgBrd.dwGaeaID = m_dwGaeaID;
				m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID (), &NetMsgBrd );
				SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
			}
		}
	}

	/*pvp capture the flag, Juver, 2018/02/06 */
	if ( m_bCaptureTheFlagHoldFlag )
	{
		m_fCaptureTheFlagDetectGateTimer += fElapsedTime;
		if ( m_fCaptureTheFlagDetectGateTimer >= CAPTURE_THE_FLAG_DETECT_GATE_TIMER )
		{
			PVPCaptureTheFlagDetectGate();
			m_fCaptureTheFlagDetectGateTimer = 0.0f;
		}

		m_fCaptureTheFlagHoldFlagTimer += fElapsedTime;
		if ( m_fCaptureTheFlagHoldFlagTimer >= CAPTURE_THE_FLAG_HOLD_MAX_TIME )
		{
			PVPCaptureTheFlagHoldFlag( false );
			m_fCaptureTheFlagHoldFlagTimer = 0.0f;
		}
	}

	m_fMoveDelay += fElapsedTime;

	if ( m_sQITEMFACT.IsACTIVE() )
	{
		bool bOK = UPDATE_QITEMFACT(fElapsedTime);
		if ( !bOK )
		{
			GLMSG::SNETPC_QITEMFACT_END_BRD	NetMsgBrd;
			NetMsgBrd.dwGaeaID = m_dwGaeaID;

			//	Note : ����Ǿ��� ��� �ڽſ��� �˸�.
			m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID (), &NetMsgBrd );

			//	Note : ����Ǿ��� ��� �ֺ� ������� �˸�.
			SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
		}
	}

	if ( m_sPKCOMBOCOUNT.IsACTIVE() )
	{
		bool bOK = UPDATE_PKCOMBO ( fElapsedTime );
		if ( !bOK )
		{
			GLMSG::SNETPC_PKCOMBO_END_BRD	NetMsgBrd;
			NetMsgBrd.dwGaeaID = m_dwGaeaID;

			m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID (), &NetMsgBrd );
			SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
		}
	}

	if ( m_lnLastSendExp != m_sExperience.lnNow )
	{
		m_lnLastSendExp = m_sExperience.lnNow;

		//	Note : ����ġ ��ȭ Ŭ���̾�Ʈ�� �˷���.
		//
		GLMSG::SNETPC_UPDATE_EXP NetMsgExp;
		NetMsgExp.lnNowExp = m_sExperience.lnNow;
		m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID (), &NetMsgExp );

		if ( m_sExperience.ISOVER() )
		{
			//	Note : ���� ����. ( ���� �޽��� �߻�. )
			//
			GLMSG::SNETPC_REQ_LEVELUP NetMsg;
			MsgReqLevelUp ( (NET_MSG_GENERIC*) &NetMsg );

			m_lnLastSendExp = m_sExperience.lnNow;


		}else{
			m_bVietnamLevelUp = FALSE; 
		}
	}

	if ( m_lnLastSendVNExp != m_lVNGainSysExp )
	{
		m_lnLastSendVNExp = m_lVNGainSysExp;

		//	Note : ����ġ ��ȭ Ŭ���̾�Ʈ�� �˷���.
		//
		GLMSG::SNETPC_VIETNAM_GAIN_EXP NetMsgExp;
		NetMsgExp.gainExp = m_lVNGainSysExp;
		m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID (), &NetMsgExp );

	}

#if defined(_RELEASED) || defined(TW_PARAM) || defined(HK_PARAM)
	if( m_lnTempMoney != m_lnMoney )
	{
		if( m_bMoneyUpdate )
		{
			m_bMoneyUpdate = FALSE;
		}else{
			// ��ŷ���� �α׸� ����
			HACKINGLOG_WRITE( "Different Update Money!!, Account[%s], ID[%s], Money %I64d, TempMoney %I64d, Money Gap %I64d", 
								m_szUID, m_szName, m_lnMoney, m_lnTempMoney, m_lnMoney - m_lnTempMoney );	
		}
		m_lnTempMoney = m_lnMoney;
	}else{
		m_bMoneyUpdate = FALSE;
	}

	if( m_lnTempStorageMoney != m_lnStorageMoney )
	{
		if( m_bStorageMoneyUpdate )
		{
			m_bStorageMoneyUpdate = FALSE;
		}else{
			// ��ŷ���� �α׸� ����
			HACKINGLOG_WRITE( "Different Update Storage Money!!, Account[%s], ID[%s], Storage Money %I64d, TempStorage Money %I64d, Storage Money Gap %I64d", 
							   m_szUID, m_szName, m_lnStorageMoney, m_lnTempStorageMoney, m_lnStorageMoney - m_lnTempStorageMoney );	
		}
		m_lnTempStorageMoney = m_lnStorageMoney;
	}else{
		m_bStorageMoneyUpdate = FALSE;
	}
#endif

	//	Note : ���� ��ǥ�� ��ȿ�� �˻�.
	//		(����) Ÿ������ ����� �ݵ�� ȣ���Ͽ� ��ȿ�� �˻� �ʿ�.
	//
	if ( m_TargetID.dwID != EMTARGET_NULL )
	{
		if ( !m_pGLGaeaServer->ValidCheckTarget(m_pLandMan,m_TargetID) )
			m_TargetID.dwID = EMTARGET_NULL;
	}
	
	if ( m_TargetID.dwID == EMTARGET_NULL && IsACTION(GLAT_ATTACK) )	TurnAction ( GLAT_IDLE );

	if ( IsValidBody () )
	{
		DoQuestCheckLimitTime ( fTime, fElapsedTime );

		if ( m_sHP.dwNow>0 )
		{
			float fCONFT_POINT_RATE(1.0f);
			if ( m_sCONFTING.IsCONFRONTING() )		fCONFT_POINT_RATE = m_sCONFTING.sOption.fHP_RATE;

			// ���� �̻�ȿ�� ������Ʈ
			UpdateLandEffect();

			
			//	Note : ���� LOGIC ������ ������Ʈ.
			//
			GLCHARLOGIC::UPDATE_DATA ( fTime, fElapsedTime, FALSE, fCONFT_POINT_RATE );

			UpdateClientState ( fElapsedTime );

			m_ARoundSlot.Update ( m_pLandMan );

			DoQuestReachZone();

			/*activity system, Juver, 2017/11/02 */
			if ( m_sActivityCheckMapID != m_sMapID )
			{
				m_sActivityCheckMapID = m_sMapID;

				DoActivityMapReach( m_sMapID );
			}
		}
		else
		{
			//	Note : �������� ���� �߻�. ( ����ġ + ������ + �ݾ� )
			DoFalling ();

#if defined(_RELEASED) || defined(TW_PARAM) || defined(TH_PARAM) || defined(HK_PARAM) // ***Tracing Log print
			if ( m_bTracingUser )
			{
				NET_LOG_UPDATE_TRACINGCHAR TracingMsg;
				TracingMsg.nUserNum  = GetUserID();
				StringCchCopy( TracingMsg.szAccount, USR_ID_LENGTH+1, m_szUID );

				CString strTemp;
				strTemp.Format( "Dead!!, [%s][%s], Money:[%I64d]", m_szUID, m_szName, m_lnMoney );
				StringCchCopy( TracingMsg.szLogMsg, TRACING_LOG_SIZE, strTemp.GetString() );

				m_pGLGaeaServer->SENDTOAGENT( m_dwClientID, &TracingMsg );
			}
#endif
		}
	}

	// ���� ��ġ�� �ٲ����� ����Ʈ ���� ���θ� �Ǵ��Ѵ�.
	if( m_sMapID != m_sOldMapID )
	{
		DoQuestCheckLeaveMap();

		m_sOldMapID = m_sMapID;

		/*pvp capture the flag, Juver, 2018/02/07 */
		if ( m_bCaptureTheFlagHoldFlag )
		{
			GLPVPCaptureTheFlagField::GetInstance().FlagResetBase( m_dwCharID, CAPTURE_THE_FLAG_FLAG_HOLD_STATUS_LEFTBATTLE );
		}
	}

	/*skill summon, Juver, 2017/10/10 */
	m_fSummonCheckTime += fElapsedTime;
	if ( m_fSummonCheckTime >= SUMMON_VALID_CHECK_TIME && m_pGLGaeaServer )
	{
		for( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		{
			if ( m_dwSummonGUID_FLD[i] != GAEAID_NULL )
			{
				PGLSUMMONFIELD psummon_field = m_pGLGaeaServer->GetSummon( m_dwSummonGUID_FLD[i] );
				if ( !psummon_field )
				{
					CDebugSet::ToFileWithTime( "_summon.txt", "summon id stuck no longer connected to summon id:%d", m_dwSummonGUID_FLD[i]);
					m_dwSummonGUID_FLD[i] = GAEAID_NULL;
				}
			}
		}
	}	

	/*contribution/activity bonus, Juver, 2017/10/14 */
	if( m_bUseContributionPointTimer )
	{
		m_fContributionPointBonusTimer += fElapsedTime;
		if ( m_fContributionPointBonusTimer >= m_fContributionPointBonusTimeMax )
		{
			m_fContributionPointBonusTimer = 0.0f;
			m_llContributionPoint += GLCONST_CHAR::wContributionPointBonusAdd;
			
			GLMSG::SNETPC_UPDATE_CONTRIBUTION_POINT netmsgclient;
			netmsgclient.bNotice = true;
			netmsgclient.llPoint = m_llContributionPoint;
			GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &netmsgclient );
		}
	}

	/*contribution/activity bonus, Juver, 2017/10/14 */
	if ( m_bUseActivityPointTimer )
	{
		m_fActivityPointBonusTimer += fElapsedTime;
		if ( m_fActivityPointBonusTimer >= m_fActivityPointBonusTimeMax )
		{
			m_fActivityPointBonusTimer = 0.0f;
			m_dwActivityPoint += GLCONST_CHAR::wActivityPointBonusAdd;

			GLMSG::SNETPC_UPDATE_ACTIVITY_POINT netmsgclient;
			netmsgclient.bNotice = true;
			netmsgclient.dwPoint = m_dwActivityPoint;
			GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &netmsgclient );
		}
	}

	/*product item, Juver, 2017/10/18 */
	if ( m_bItemCompoundTask )
	{
		m_fItemCompoundTaskTimer += fElapsedTime;

		if ( m_fItemCompoundTaskTimer >= m_fItemCompoundTaskTime )
		{
			m_bItemCompoundTask = FALSE;
			m_fItemCompoundTaskTimer = 0.0f;
			m_fItemCompoundTaskTime = 0.0f;

			DoItemCompoundTask();
		}
	}

	/*charinfoview , Juver, 2017/11/11 */
	if ( m_fReqCharInfoTimer < REQ_CHARINFO_DELAY )
		m_fReqCharInfoTimer += fElapsedTime;

	/*equipment lock, Juver, 2018/01/16 */
	if ( m_fEquipmentLockTimer < EQUIPMENT_LOCK_TIMER )
		m_fEquipmentLockTimer += fElapsedTime;

	/*storage lock, Juver, 2018/01/16 */
	if ( m_fStorageLockTimer < STORAGE_LOCK_TIMER )
		m_fStorageLockTimer += fElapsedTime;

	/*inventory lock, Juver, 2018/01/16 */
	if ( m_fInventoryLockTimer < INVENTORY_LOCK_TIMER )
		m_fInventoryLockTimer += fElapsedTime;

	//cndev
	if ( m_fItemRebuildTimer < ITEM_REBUILD_TIMER )
		m_fItemRebuildTimer += fElapsedTime;

	if ( m_fInvenSortTimer < INVEN_SORT_TIMER )
		m_fInvenSortTimer += fElapsedTime;

	/* tyranny anti-afk, SeiferXIII 2020/05/17 */
	if( IsSafeZone() ) m_fAntiSafeZoneAFK += fElapsedTime;
	else m_fAntiSafeZoneAFK = 0.0f;

	/* tyranny anti-afk, SeiferXIII 2020/05/17 */
	if(m_sMapID.wMainID == 222){
		if( IsDie() ) m_fDeadAFK += fElapsedTime;
		else m_fDeadAFK = 0.0f;

		if(m_fDeadAFK >= 180.0f&& m_dwUserLvl < USER_GM3){
			GLMSG::SNET_SERVER_GENERALCHAT NetMsgGen;
			CString strTextGen;
			strTextGen.Format( "You have been disconnected for Dead-AFKing at the Tyranny Wars." );
			NetMsgGen.SETTEXT ( strTextGen.GetString() );
			NetMsgGen.bNotify = true;
			NetMsgGen.iColor = 7;
			GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsgGen );

			GLMSG::SNETPC_DROPCHAR_TOAGENT DropMsg;
			DropMsg.dwCHARID  = GetCharID();
			GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID, &DropMsg);
		}

		if(m_fTWAntiAFK >= 180.0f&& m_dwUserLvl < USER_GM3){
			GLMSG::SNET_SERVER_GENERALCHAT NetMsgGen;
			CString strTextGen;
			strTextGen.Format( "You have been disconnected for AFKing at the Tyranny Wars." );
			NetMsgGen.SETTEXT ( strTextGen.GetString() );
			NetMsgGen.bNotify = true;
			NetMsgGen.iColor = 7;
			GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsgGen );

			GLMSG::SNETPC_DROPCHAR_TOAGENT DropMsg;
			DropMsg.dwCHARID  = GetCharID();
			GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID, &DropMsg);
		}
	}

	/* tyranny anti-afk, SeiferXIII 2020/05/17 */
	if( m_fAntiSafeZoneAFK >= 180.0f && m_dwUserLvl < USER_GM3 ){
		GLMSG::SNET_SERVER_GENERALCHAT NetMsgGen;
		CString strTextGen;
		strTextGen.Format( "You have been disconnected for idling at the SafeZone Area." );
		NetMsgGen.SETTEXT ( strTextGen.GetString() );
		NetMsgGen.bNotify = true;
		NetMsgGen.iColor = 7;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsgGen );

		GLMSG::SNETPC_DROPCHAR_TOAGENT DropMsg;
		DropMsg.dwCHARID  = GetCharID();
		GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID, &DropMsg);
	}

	switch ( m_Action )
	{
	case GLAT_IDLE:
		m_fIdleTime += fElapsedTime;

		/* tyranny anti-afk, SeiferXIII 2020/05/17 */
		if(m_sMapID.wMainID == 222)
			m_fTWAntiAFK += fElapsedTime;

		break;

	case GLAT_TALK:
		{
		/* tyranny anti-afk, SeiferXIII 2020/05/17 */
		if(m_sMapID.wMainID == 222)
			m_fTWAntiAFK += fElapsedTime;
		}
		break;

	case GLAT_MOVE:
		{
			/* tyranny anti-afk, SeiferXIII 2020/05/17 */
			m_fTWAntiAFK = 0.0f;
			//	Note : �ɸ��� �̵� ������Ʈ.
			//

			//stun visual bug fixed
			if ( m_bSTATE_STUN )
			{
				if ( m_actorMove.PathIsActive() )	m_actorMove.Stop ();
			}
			
			m_actorMove.SetMaxSpeed ( GetMoveVelo () );
			m_actorMove.Update ( fElapsedTime );
			if ( !m_actorMove.PathIsActive() )
			{
				m_actorMove.Stop ();
				TurnAction ( GLAT_IDLE );
			}

			//	Note : �ɸ��� ���� ��ġ ������Ʈ.
			//
			m_vPos = m_actorMove.Position();

			//	Note : �ɸ��� ���� ���� ������Ʈ.
			//
			D3DXVECTOR3 vMovement = m_actorMove.NextPosition();
			if ( vMovement.x != FLT_MAX && vMovement.y != FLT_MAX && vMovement.z != FLT_MAX )
			{
				D3DXVECTOR3 vDirection = vMovement - m_vPos;
				if ( !DxIsMinVector(vDirection,0.2f) )
				{
					D3DXVec3Normalize ( &vDirection, &vDirection );
					m_vDir = vDirection;
				}
			}
		}
		break;

	case GLAT_ATTACK:
		{
			/* tyranny anti-afk, SeiferXIII 2020/05/17 */
			m_fTWAntiAFK = 0.0f;

			if ( !AttackProcess ( fElapsedTime ) )
			{
				TurnAction ( GLAT_IDLE );
			}

			GLACTOR* pACTOR = m_pGLGaeaServer->GetTarget ( m_pLandMan, m_TargetID );
			if ( pACTOR )
			{
				D3DXVECTOR3 vDirection = pACTOR->GetPosition() - m_vPos;
				D3DXVec3Normalize ( &vDirection, &vDirection );
				m_vDir = vDirection;
			}
		}
		break;

	case GLAT_SKILL:
		{
			/* tyranny anti-afk, SeiferXIII 2020/05/17 */
			m_fTWAntiAFK = 0.0f;

			/*dmk14 visual bug fix*/
			if ( m_actorMove.PathIsActive() )	m_actorMove.Stop();
			if( !SkillProcess ( fElapsedTime ) )		
				TurnAction ( GLAT_IDLE );
		}
		break;

	case GLAT_SHOCK:
		{
			if( !ShockProcess ( fElapsedTime ) )		TurnAction ( GLAT_IDLE );
		}
		break;

	case GLAT_PUSHPULL:
	case GLAT_SKILLMOVE: /*push pull skill logic, Juver, 2017/06/05 */
	case GLAT_SKILLDASH: /*dash skill logic, Juver, 2017/06/17 */
		{
			m_actorMove.Update ( fElapsedTime );
			if ( !m_actorMove.PathIsActive() )
			{
				m_actorMove.Stop ();
				TurnAction ( GLAT_IDLE );
			}

			//	Note : Mob�� ���� ��ġ ������Ʈ.
			//
			m_vPos = m_actorMove.Position();
		}
		break;

	case GLAT_FALLING:
		break;

	case GLAT_DIE:
		break;
	case GLAT_GATHERING:
		break;
	};

	//	Note : Mob�� ���� ��ġ ������Ʈ.
	//
	m_vPos = m_actorMove.Position();


	/*dmk14 chat limit*/
	if( m_bChatProc )
	{
		m_fChatTimerProc += fElapsedTime;
		if( m_fChatTimerProc >= 5.0f )
		{
			m_bChatProc = false;
			m_wChatCount = 0;
			m_fChatTimerProc = 0.0f;
		}
	}

	return S_OK;
}

void GLChar::STATEBLOW ( const SSTATEBLOW &sStateBlow )
{
	int nIndex = 0;

	if ( sStateBlow.emBLOW <= EMBLOW_SINGLE )	nIndex = 0;
	else										nIndex = sStateBlow.emBLOW-EMBLOW_SINGLE;

	m_sSTATEBLOWS[nIndex] = sStateBlow;

	//	�����̻� ( ���� ) �� ������ ���� ���� �ߴ�.
	if ( sStateBlow.emBLOW == EMBLOW_STUN )
	{
		//	Note : �������� Ŭ���̾�Ʈ �鿡�� �˸�.
		GLMSG::SNET_ACTION_BRD NetMsgBrd;
		NetMsgBrd.emCrow	= CROW_PC;
		NetMsgBrd.dwID		= m_dwGaeaID;
		NetMsgBrd.emAction	= GLAT_IDLE;

		//	Note : �ֺ��� Char ����.
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

		//	Note : �ڽ��� Ŭ���̾�Ʈ��.
		m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsgBrd );

		TurnAction ( GLAT_IDLE );
	}
}

void GLChar::CURE_STATEBLOW ( DWORD dwCUREFLAG )
{
	BOOL bChanged = FALSE;

	for ( int i=0; i<EMBLOW_MULTI; ++i )
	{
		if ( m_sSTATEBLOWS[i].emBLOW==EMBLOW_NONE )		continue;

		bChanged = TRUE;
		if ( STATE_TO_DISORDER(m_sSTATEBLOWS[i].emBLOW)&dwCUREFLAG )
		{
			m_sSTATEBLOWS[i].emBLOW = EMBLOW_NONE;
		}
	}

	if ( !bChanged )	return;

	//	Note : ���� �̻� ��ȭ MsgFB.
	//
	GLMSG::SNETPC_CURESTATEBLOW_BRD NetMsgBRD;
	NetMsgBRD.dwID = m_dwGaeaID;
	NetMsgBRD.emCrow = CROW_PC;
	NetMsgBRD.dwCUREFLAG = dwCUREFLAG;

	//	Note : �ֺ� Ŭ���̾�Ʈ�鿡�� �޼��� ����.
	//
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBRD );

	//	Note : ��󿡰� �޼��� ����.
	m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsgBRD );
}

/*debuff skill logic, Juver, 2017/06/10 */
void GLChar::BUFF_REMOVE( DWORD dwBuffFlag )
{
	GLMSG::SNETPC_SKILLHOLD_RS_BRD NetMsgSkillBrd;
	for ( int i=0; i<SKILLFACT_SIZE; ++i )
	{
		if ( m_sSKILLFACT[i].sNATIVEID == NATIVEID_NULL() ) continue;
	
        PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_sSKILLFACT[i].sNATIVEID );
		if ( !pSkill ) continue;
		if ( pSkill->m_sBASIC.emACTION != (SKILL::EMACTION)dwBuffFlag ) continue;
		
		DISABLESKEFF( i );
		NetMsgSkillBrd.bRESET[i] = true;
	}
	
	NetMsgSkillBrd.dwID = m_dwGaeaID;
	NetMsgSkillBrd.emCrow = CROW_PC;

	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgSkillBrd );
	m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsgSkillBrd );
}

void GLChar::DO_STUN_ACTION ()
{
	// �������� Ŭ���̾�Ʈ �鿡�� �˸�.
	GLMSG::SNET_ACTION_BRD NetMsgBRD;
	NetMsgBRD.emCrow	= CROW_PC;
	NetMsgBRD.dwID		= m_dwGaeaID;
	NetMsgBRD.emAction	= GLAT_IDLE;

	// �ֺ��� Char ����.
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBRD );

	// �ڽ��� Ŭ���̾�Ʈ��.
	m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsgBRD );

	TurnAction ( GLAT_IDLE );
}

void GLChar::VAR_BODY_POINT ( const EMCROW emACrow, const DWORD dwAID, const BOOL bPartySkill, int nvar_hp, int nvar_mp, int nvar_sp )
{
	if ( m_sHP.dwNow == 0 )	return;

	DWORD dwDxHP = m_sHP.VARIATION ( nvar_hp );
	m_sMP.VARIATION ( nvar_mp );
	m_sSP.VARIATION ( nvar_sp );

	m_sAssault.emCrow = emACrow;
	m_sAssault.dwID = dwAID;

	if ( emACrow == CROW_PC && nvar_hp<0 )
	{
		PGLCHAR pChar = m_pGLGaeaServer->GetChar ( dwAID );
		if ( pChar )
		{
			AddDamageLog ( m_cDamageLog, dwAID, pChar->m_dwUserID, dwDxHP );
			//if ( m_sHP.dwNow == 0 || abs(nvar_hp) >= m_sHP.dwMax /*|| abs(nvar_hp) > m_sHP.dwNow*/ ) DoFalling();	//add new damage overlap fixed
		}
	}

	if ( emACrow == CROW_PC && nvar_hp>0 )
	{
		PGLCHAR pChar = m_pGLGaeaServer->GetChar ( dwAID );
		if ( pChar )
		{
			/*pvp tyranny, Juver, 2017/08/24 */
			if ( m_pLandMan && m_pLandMan->m_bPVPTyrannyMap )
				GLPVPTyrannyField::GetInstance().ScoreHeal( pChar->m_dwCharID, dwDxHP );

			/*school wars, Juver, 2018/01/20 */
			if ( m_pLandMan && m_pLandMan->m_bPVPSchoolWarsMap )
				GLPVPSchoolWarsField::GetInstance().ScoreHeal( pChar->m_dwCharID, dwDxHP );

			/*pvp capture the flag, Juver, 2018/02/01 */
			if ( m_pLandMan && m_pLandMan->m_bPVPCaptureTheFlagMap )
				GLPVPCaptureTheFlagField::GetInstance().ScoreHeal( pChar->m_dwCharID, dwDxHP );
		}
	}

	// ��ȯ���� ������ ��� ��ȯ�� ������ �����ɷ� ���
	if ( emACrow == CROW_SUMMON && nvar_hp<0 )
	{
		PGLSUMMONFIELD pSummon = GLGaeaServer::GetInstance().GetSummon ( dwAID );
		if( pSummon )
		{
			AddDamageLog ( m_cDamageLog, dwAID, pSummon->m_pOwner->m_dwUserID, dwDxHP );
		}
	}

	//	[��Ƽ���鿡��]
	if ( m_dwPartyID!=PARTY_NULL )
	{
		GLMSG::SNET_PARTY_MBR_POINT NetMsg;
		NetMsg.dwGaeaID = m_dwGaeaID;
		NetMsg.sHP = m_sHP;
		NetMsg.sMP = m_sMP;
		//party search
		NetMsg.m_wLevel = m_wLevel;
		m_pGLGaeaServer->SENDTOPARTYCLIENT ( m_dwPartyID, (NET_MSG_GENERIC*) &NetMsg );
	}

	if ( nvar_hp<0 && IsACTION(GLAT_GATHERING) )
	{
		GLMSG::SNET_ACTION_BRD NetMsgBrd;
		NetMsgBrd.emCrow	= CROW_PC;
		NetMsgBrd.dwID		= m_dwGaeaID;
		NetMsgBrd.emAction	= GLAT_IDLE;

		//	Note : �ֺ��� Char ����.
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

		m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsgBrd );		
		TurnAction ( GLAT_IDLE );
	}
}

DWORD GLChar::SELECT_SKILLSLOT ( SNATIVEID skill_id )
{
	SKILLREALFACT_SIZE;
	DWORD dwSELECT = UINT_MAX;

	//	Note : ��ų�� �� �⺻ ���� ����.
	//
	for ( DWORD i=0; i<SKILLREALFACT_SIZE; ++i )
	{
		if ( m_sSKILLFACT[i].sNATIVEID==skill_id )
		{
			return dwSELECT = i;
		}
	}

	float fAGE = FLT_MAX;
	for ( DWORD i=0; i<SKILLREALFACT_SIZE; ++i )
	{
		if ( m_sSKILLFACT[i].sNATIVEID==SNATIVEID(false) )
		{
			return dwSELECT = i;
		}

		if ( m_sSKILLFACT[i].fAGE < fAGE )
		{
			fAGE = m_sSKILLFACT[i].fAGE;
			dwSELECT = i;
		}
	}

	return dwSELECT;
}

BOOL GLChar::RECEIVE_SKILLFACT ( const SNATIVEID skill_id, const WORD wlevel, DWORD &dwSELECT, WORD _wCasterCrow, DWORD _dwCasterID )
{
	dwSELECT = SKILLFACT_SIZE;

	//	��ų ���� ������.
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( skill_id.wMainID, skill_id.wSubID );
	if ( !pSkill )				return FALSE;
	if ( wlevel >= SKILL::MAX_LEVEL )	return FALSE;

	
	if ( pSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF && 
		pSkill->m_sBASIC.emIMPACT_REALM == REALM_SELF && 
		pSkill->m_sBASIC.emIMPACT_SIDE == SIDE_OUR )
	{
		switch ( m_emClass )
		{
		case GLCC_BRAWLER_M:
		case GLCC_BRAWLER_W:
			switch( pSkill->m_sBASIC.sNATIVEID.wMainID )
			{
				case EMSKILL_SWORDSMAN_01:
				case EMSKILL_SWORDSMAN_02:
				case EMSKILL_SWORDSMAN_03:
				case EMSKILL_SWORDSMAN_04:
				case EMSKILL_ARCHER_01:
				case EMSKILL_ARCHER_02:
				case EMSKILL_ARCHER_03:
				case EMSKILL_ARCHER_04:
				case EMSKILL_SHAMAN_01:
				case EMSKILL_SHAMAN_02:
				case EMSKILL_SHAMAN_03:
				case EMSKILL_SHAMAN_04:
					return FALSE;
					break;
			}
			break;
		case GLCC_SWORDSMAN_M:
		case GLCC_SWORDSMAN_W:
			switch( pSkill->m_sBASIC.sNATIVEID.wMainID )
			{
				case EMSKILL_BRAWLER_01:
				case EMSKILL_BRAWLER_02:
				case EMSKILL_BRAWLER_03:
				case EMSKILL_BRAWLER_04:
				case EMSKILL_ARCHER_01:
				case EMSKILL_ARCHER_02:
				case EMSKILL_ARCHER_03:
				case EMSKILL_ARCHER_04:
				case EMSKILL_SHAMAN_01:
				case EMSKILL_SHAMAN_02:
				case EMSKILL_SHAMAN_03:
				case EMSKILL_SHAMAN_04:
					return FALSE;
					break;					
			}
			break;
		case GLCC_ARCHER_M:
		case GLCC_ARCHER_W:
			switch( pSkill->m_sBASIC.sNATIVEID.wMainID )
			{
				case EMSKILL_BRAWLER_01:
				case EMSKILL_BRAWLER_02:
				case EMSKILL_BRAWLER_03:
				case EMSKILL_BRAWLER_04:
				case EMSKILL_SWORDSMAN_01:
				case EMSKILL_SWORDSMAN_02:
				case EMSKILL_SWORDSMAN_03:
				case EMSKILL_SWORDSMAN_04:
				case EMSKILL_SHAMAN_01:
				case EMSKILL_SHAMAN_02:
				case EMSKILL_SHAMAN_03:
				case EMSKILL_SHAMAN_04:
					return FALSE;
					break;					
			}
			break;
		case GLCC_SHAMAN_M:
		case GLCC_SHAMAN_W:
			switch( pSkill->m_sBASIC.sNATIVEID.wMainID )
			{
				case EMSKILL_BRAWLER_01:
				case EMSKILL_BRAWLER_02:
				case EMSKILL_BRAWLER_03:
				case EMSKILL_BRAWLER_04:
				case EMSKILL_SWORDSMAN_01:
				case EMSKILL_SWORDSMAN_02:
				case EMSKILL_SWORDSMAN_03:
				case EMSKILL_SWORDSMAN_04:
				case EMSKILL_ARCHER_01:
				case EMSKILL_ARCHER_02:
				case EMSKILL_ARCHER_03:
				case EMSKILL_ARCHER_04:
					return FALSE;
					break;					
			}
			break;
		}
	}

	SKILL::CDATA_LVL &sSKILL_DATA = pSkill->m_sAPPLY.sDATA_LVL[wlevel];

	/*pvp capture the flag, Juver, 2018/02/08 */
	if ( m_bCaptureTheFlagHoldFlag && pSkill->m_sBASIC.emACTION == SKILL::EMACTION_BUFF )	return FALSE;

	BOOL bHOLD = FALSE;
	SSKILLFACT sSKILLEF;
	sSKILLEF._wCasterCrow = _wCasterCrow;
	sSKILLEF._dwCasterID = _dwCasterID;

	switch ( pSkill->m_sAPPLY.emBASIC_TYPE )
	{
	case SKILL::EMFOR_VARHP:
	case SKILL::EMFOR_VARMP:
	case SKILL::EMFOR_VARSP:
	case SKILL::EMFOR_DEFENSE:
	case SKILL::EMFOR_HITRATE:
	case SKILL::EMFOR_AVOIDRATE:
	case SKILL::EMFOR_VARAP:
	case SKILL::EMFOR_VARDAMAGE:
	case SKILL::EMFOR_VARDEFENSE:
	case SKILL::EMFOR_PA:
	case SKILL::EMFOR_SA:
	case SKILL::EMFOR_MA:
	case SKILL::EMFOR_RESIST:
	case SKILL::EMFOR_SUMMONTIME: /*summon time, Juver, 2017/12/12 */
		bHOLD = TRUE;
		sSKILLEF.emTYPE = pSkill->m_sAPPLY.emBASIC_TYPE;
		sSKILLEF.fMVAR = sSKILL_DATA.fBASIC_VAR;
		break;
	};

	for( int nImpact=0; nImpact<SKILL::MAX_IMPACT; ++nImpact )
	{
		if ( pSkill->m_sAPPLY.sImpacts[nImpact].emADDON != EMIMPACTA_NONE )
		{
			bHOLD = TRUE;
			sSKILLEF.sImpacts[nImpact].emADDON = pSkill->m_sAPPLY.sImpacts[nImpact].emADDON;
			sSKILLEF.sImpacts[nImpact].fADDON_VAR = pSkill->m_sAPPLY.sImpacts[nImpact].fADDON_VAR[wlevel];
		}
	}
	
	for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
	{
		switch ( pSkill->m_sAPPLY.sSpecs[nSpec].emSPEC )
		{
		case EMSPECA_REFDAMAGE:
		case EMSPECA_NONBLOW:
		case EMSPECA_PIERCE:
		case EMSPECA_TARRANGE:
		case EMSPECA_MOVEVELO:
		case EMSPECA_ATTACKVELO:
		case EMSPECA_SKILLDELAY:
		case EMSPECA_PSY_DAMAGE_REDUCE:
		case EMSPECA_MAGIC_DAMAGE_REDUCE:
		case EMSPECA_PSY_DAMAGE_REFLECTION:
		case EMSPECA_MAGIC_DAMAGE_REFLECTION:
		case EMSPECA_DEFENSE_SKILL_ACTIVE:
		case EMSPECA_IMMUNE:
		case EMSPECA_PROHIBIT_POTION:		/*prohibit potion skill logic, Juver, 2017/06/06 */
		case EMSPECA_PROHIBIT_SKILL:		/*prohibit skill logic, Juver, 2017/06/06 */
		case EMSPECA_CHANGE_ATTACK_RANGE:	/*skill range spec logic, Juver, 2017/06/06 */
		case EMSPECA_CHANGE_APPLY_RANGE:	/*skill range spec logic, Juver, 2017/06/06 */
		case EMSPECA_STUN:					/*skill stun logic, Juver, 2017/06/06 */
		case EMSPECA_CONTINUOUS_DAMAGE:		/*continuous damage skill logic, Juver, 2017/06/10 */
		case EMSPECA_CURSE:					/*curse skill logic, Juver, 2017/06/10 */
		case EMSPECA_TALK_TO_NPC:			/*npc talk buff, Juver, 2017/10/03 */
		case EMSPECA_IGNORE_DAMAGE:			/*ignore damage, Juver, 2017/12/12 */
		case EMSPECA_STIGMA:
		case EMSPECA_INVISIBLE:
		case EMSPECA_ENHANCEMENT:
		case EMSPECA_RECVISIBLE:
			bHOLD = TRUE;
			sSKILLEF.sSpecs[nSpec].emSPEC = pSkill->m_sAPPLY.sSpecs[nSpec].emSPEC;
			sSKILLEF.sSpecs[nSpec].fSPECVAR1 = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wlevel].fVAR1;
			sSKILLEF.sSpecs[nSpec].fSPECVAR2 = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wlevel].fVAR2;
			sSKILLEF.sSpecs[nSpec].dwSPECFLAG = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wlevel].dwFLAG;
			sSKILLEF.sSpecs[nSpec].dwNativeID = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wlevel].dwNativeID;
			break;
		case EMSPECA_DAMAGE_LOOP:
			{
				bHOLD = true;
				sSKILLEF.sSpecs[nSpec].emSPEC = pSkill->m_sAPPLY.sSpecs[nSpec].emSPEC;
				sSKILLEF.sSpecs[nSpec].fSPECVAR1 = m_sLINKHP.fRef;
				sSKILLEF.sSpecs[nSpec].fSPECVAR2 = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wlevel].fVAR3;
				sSKILLEF.sSpecs[nSpec].dwSPECFLAG = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wlevel].dwFLAG;
				sSKILLEF.sSpecs[nSpec].dwNativeID = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wlevel].dwNativeID;
				GLOGICEX::VARIATION( m_sHP.dwMax, INT_MAX, static_cast<int>(static_cast<float>(m_sHP.dwMax) * m_sLINKHP.fRef) );
				m_sHP.LIMIT();
			}
			break;
		};
	}

	//	Note : ������ ��ų�� ��� ���� ������ ã�Ƽ� ��ų ȿ���� �־���.
	//
	if ( bHOLD )
	{
		sSKILLEF.sNATIVEID	= skill_id;
		sSKILLEF.wLEVEL		= wlevel;
		sSKILLEF.fAGE		= sSKILL_DATA.fLIFE;

		dwSELECT = SELECT_SKILLSLOT ( skill_id );
		m_sSKILLFACT[dwSELECT] = sSKILLEF;
	}

	return TRUE;
}

BOOL GLChar::RECEIVE_DRUGFACT ( EMITEM_DRUG emDRUG, WORD wCURE, BOOL bRATIO )
{
	//SKILL::EMTYPES emTYPE[3] = { SKILL::EMFOR_VARHP, SKILL::EMFOR_VARMP, SKILL::EMFOR_VARSP };
	//SNATIVEID nidNO[3] = { SNATIVEID(35,USHRT_MAX), SNATIVEID(35,USHRT_MAX), SNATIVEID(35,USHRT_MAX) };

	//if ( bRATIO )
	//{
	//	wCURE = m_sHP.wMax * wCURE / 100;
	//}

	//switch ( emDRUG )
	//{
	//case ITEM_DRUG_HP:
	//	nidNO[0].wSubID = 0;
	//	break;

	//case ITEM_DRUG_MP:
	//	nidNO[1].wSubID = 1;
	//	break;

	//case ITEM_DRUG_SP:
	//	nidNO[2].wSubID = 2;
	//	break;

	//case ITEM_DRUG_HP_MP:
	//	nidNO[0].wSubID = 0;
	//	nidNO[1].wSubID = 1;
	//	break;

	//case ITEM_DRUG_MP_SP:
	//	nidNO[1].wSubID = 1;
	//	nidNO[2].wSubID = 2;
	//	break;

	//case ITEM_DRUG_HP_MP_SP:
	//	nidNO[0].wSubID = 0;
	//	nidNO[1].wSubID = 1;
	//	nidNO[2].wSubID = 2;
	//	break;
	//};

	//for ( int i=0; i<3; ++i )
	//{
	//	const SNATIVEID &skill = nidNO[i];

	//	if ( skill.wSubID == USHRT_MAX )		continue;

	//	SSKILLFACT sSKILLEF;
	//	sSKILLEF.emTYPE = emTYPE[i];
	//	
	//	//	��ų ���� ������.
	//	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( skill );
	//	if ( !pSkill )				return FALSE;
	//	SKILL::CDATA_LVL &sSKILL_DATA = pSkill->m_sAPPLY.sDATA_LVL[0];

	//	float fRATE = wCURE/float(m_sHP.wMax);
	//	if ( fRATE>1.0f )	fRATE = 1.0f;

	//	float fLIFE = sSKILL_DATA.fLIFE;
	//	if ( fLIFE==0.0f )	fLIFE = 0.01f;

	//	sSKILLEF.sNATIVEID	= skill;
	//	sSKILLEF.wLEVEL		= 0;
	//	sSKILLEF.fAGE		= fLIFE;
	//	sSKILLEF.fMVAR		= fRATE/fLIFE;

	//	WORD wSELECT = SKILLREALFACT_SIZE + i;
	//	m_sSKILLFACT[wSELECT] = sSKILLEF;

	//	GLMSG::SNETPC_REQ_SKILLHOLDEX_BRD NetMsgBRD;
	//	NetMsgBRD.emCrow = GetCrow();
	//	NetMsgBRD.dwID = GetCtrlID();
	//	NetMsgBRD.wSLOT = wSELECT;
	//	NetMsgBRD.sSKILLEF = sSKILLEF;

	//	//	Note : '��ų���'�� �ֺ� Ŭ���̾�Ʈ�鿡�� �޼��� ����.
	//	//
	//	SNDMSGAROUND ( (NET_MSG_GENERIC*) &NetMsgBRD );

	//	//	Note : ��ų ��󿡰� �޼��� ����.
	//	m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID(), &NetMsgBRD );
	//}

	return TRUE;
}

void GLChar::ReBirthDecExp ()
{
	if ( m_nDECEXP==0 )		return;

	m_lnReExp = GetReExp();

	if ( (m_sExperience.lnNow-m_nDECEXP) < 0 )	m_sExperience.lnNow = 0;
	else	m_sExperience.lnNow -= m_nDECEXP;

	//	Note : ����ġ ��ȭ Ŭ���̾�Ʈ�� �˷���.
	//
	GLMSG::SNETPC_UPDATE_EXP NetMsgExp;
	NetMsgExp.lnNowExp = m_sExperience.lnNow;
	m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID (), &NetMsgExp );
}

LONGLONG GLChar::GetReExp ()
{
	LONGLONG nDecExp = 0;
	LONGLONG nReExp = 0;

	nDecExp = m_nDECEXP;

	if ( nDecExp > m_sExperience.lnNow ) nDecExp = m_sExperience.lnNow;
	if ( nDecExp <= 0 )	return 0;


	float fRExpRecovery = GLOGICEX::GLDIE_RECOVERYEXP( m_wLevel );
	nReExp = (LONGLONG)(nDecExp * fRExpRecovery);

	if ( nReExp < 0  ) nReExp = 0;	
	if ( nDecExp < nReExp ) nReExp = nDecExp;

	return nReExp;

}
LONGLONG GLChar::GetReExpMoney ( LONGLONG nReExp )
{
	LONGLONG nDecMoney = 0;

	if ( nReExp <= 0 ) return 0;

	float fRMoney = GLOGICEX::GLDIE_EXPMONEY( m_wLevel );
	nDecMoney = (LONGLONG)(nReExp * fRMoney);

	return nDecMoney;
}

void GLChar::DROP_PUTONITEM ( DWORD _dwNUM, float _fRATE )
{
//	GLGaeaServer & glGaeaServer = m_pGLGaeaServer;
	GLITEMLMT & glItemmt = GLITEMLMT::GetInstance();

	for ( DWORD i=0; i<_dwNUM; ++i )
	{
		if ( RANDOM_GEN(_fRATE) )
		{
			DWORD dwSLOT_NUM(0);

			DWORD dwSLOT_LIST[SLOT_NSIZE_S_2];
			for ( int j=0; j<SLOT_NSIZE_S_2; ++j )
			{
				EMSLOT emSLOT = EMSLOT(j);

				if ( VALID_SLOT_ITEM(emSLOT) && !GET_SLOT_ITEM(emSLOT).IsGM_GENITEM() )
				{
					SITEM* pITEM = GET_SLOT_ITEMDATA(emSLOT);
					
					//	�ŷ��ɼ�
					if ( pITEM && pITEM->sBasicOp.IsTHROW() )
						dwSLOT_LIST[dwSLOT_NUM++] = j;
				}
			}

			if ( dwSLOT_NUM==0 )					break;

			DWORD dwSLOT = (DWORD) ( rand()%dwSLOT_NUM );
			
			if ( dwSLOT >= SLOT_NSIZE_S_2 )				break;

			dwSLOT = dwSLOT_LIST[dwSLOT];
			if ( !VALID_SLOT_ITEM(EMSLOT(dwSLOT)) )	break;

			// ����� ������� �ʴ´�.
			if ( dwSLOT == SLOT_VEHICLE )			break;

			SITEMCUSTOM sITEM_CUSTOM = m_PutOnItems[dwSLOT];
			
			D3DXVECTOR3 vGenPos = D3DXVECTOR3(m_vPos) + D3DXVECTOR3(RANDOM_NUM*4.0f,0.0f,RANDOM_NUM*4.0f);
			BOOL bCollision;
			D3DXVECTOR3 vCollision;
			bCollision = m_pLandMan->IsCollisionNavi
			(
				D3DXVECTOR3(vGenPos)+D3DXVECTOR3(0,+5,0),
				D3DXVECTOR3(vGenPos)+D3DXVECTOR3(0,-5,0),
				vCollision
			);

			if ( bCollision )
			{
				//	Note :�������� ���� ���� ��� ���.
				//
				glItemmt.ReqItemRoute ( sITEM_CUSTOM, ID_CHAR, m_dwCharID, ID_CHAR, 0, EMITEM_ROUTE_GROUND, sITEM_CUSTOM.wTurnNum );


				PGLPETFIELD pMyPet = m_pGLGaeaServer->GetPET ( m_dwPetGUID );
				if ( pMyPet && pMyPet->IsValid () ) 
				{
					/*dual pet skill, Juver, 2017/12/27 */
					int nPetMID_A = pMyPet->m_sActiveSkillID_A.wMainID;
					int nPetSID_A = pMyPet->m_sActiveSkillID_A.wSubID;
                    // Note : ���� �� ��ų Ȯ�� �α� 
					glItemmt.ReqAction ( m_dwCharID,		// �����.
										EMLOGACT_PET_SKILL, // ����.
										ID_CHAR, 0,  0,				
										nPetMID_A,			// Pet Skill MID
										nPetSID_A, 			// Pet Skill SID
										0 );		

					/*dual pet skill, Juver, 2017/12/27 */
					int nPetMID_B = pMyPet->m_sActiveSkillID_B.wMainID;
					int nPetSID_B = pMyPet->m_sActiveSkillID_B.wSubID;
					// Note : ���� �� ��ų Ȯ�� �α� 
					glItemmt.ReqAction ( m_dwCharID,		// �����.
										EMLOGACT_PET_SKILL, // ����.
										ID_CHAR, 0,  0,				
										nPetMID_B,			// Pet Skill MID
										nPetSID_B, 			// Pet Skill SID
										0 );		
				}


				//	������ ����.
				GLCHARLOGIC::RELEASE_SLOT_ITEM ( (EMSLOT) dwSLOT );

				//	[�ڽſ���] SLOT�� �־��� ������ ����.
				GLMSG::SNETPC_PUTON_RELEASE NetMsg_PutOn_Release((EMSLOT)dwSLOT);
				m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsg_PutOn_Release);

				//	������ �߻�.
				m_pLandMan->DropItem ( vGenPos, &sITEM_CUSTOM );

				if ( pMyPet && pMyPet->IsValid () ) 
				{
					/*dual pet skill, Juver, 2017/12/27 */
					CDebugSet::ToFileWithTime( "_drop_puton.txt", "[%d]%s item:[%d~%d] gen:%d pet-skill:%d~%d %d~%d", m_dwCharID, m_szName, 
						sITEM_CUSTOM.sNativeID.wMainID, sITEM_CUSTOM.sNativeID.wSubID, sITEM_CUSTOM.cGenType,
						pMyPet->m_sActiveSkillID_A.wMainID, pMyPet->m_sActiveSkillID_A.wSubID,
						pMyPet->m_sActiveSkillID_B.wMainID, pMyPet->m_sActiveSkillID_B.wSubID );
				}
				else
				{
					CDebugSet::ToFileWithTime( "_drop_puton.txt", "[%d]%s item:[%d~%d] gen:%d", m_dwCharID, m_szName,
						sITEM_CUSTOM.sNativeID.wMainID, sITEM_CUSTOM.sNativeID.wSubID, sITEM_CUSTOM.cGenType );
				}
			}
		}
	}
}

void GLChar::GenerateReward ( bool bCONFT /*=false*/, bool bCONFT_SCHOOL /*=false*/ )
{
	/*combatpoint logic, Juver, 2017/05/29 */
	if ( !bCONFT && !bCONFT_SCHOOL && m_pGLGaeaServer )
	{
		//reset cp when dead
		m_sCombatPoint.dwNow = 0;
		GLMSG::SNETPC_UPDATE_CP NetMsgUpdateCP;
		NetMsgUpdateCP.wNowCP = m_sCombatPoint.dwNow;
		NetMsgUpdateCP.bDie = TRUE;
		m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID (), &NetMsgUpdateCP );

		if ( m_sAssault.emCrow==CROW_PC )
		{
			GLChar* pChar = m_pGLGaeaServer->GetChar( m_sAssault.dwID );
			if ( pChar ){
				pChar->ReceiveCP( (int)GLCONST_CHAR::wCombatPoint_PK, false );
			}
		}
		else if ( m_sAssault.emCrow==CROW_SUMMON )
		{
			PGLSUMMONFIELD pSummon = m_pGLGaeaServer->GetSummon ( m_sAssault.dwID );
			if( pSummon ) {
				if ( pSummon->m_pOwner ){
					pSummon->m_pOwner->ReceiveCP( (int)GLCONST_CHAR::wCombatPoint_PK, true );
				}
			}
		}
	}
	
	//	Note : ����ġ �ջ� ó��.
	//
	bool bBRIGHTEVENT = m_pGLGaeaServer->IsBRIGHTEVENT();

	m_nDECEXP = 0;
	// ������ �̰ų� ������ �׾������
	/*skill summon, Juver, 2017/10/10 */
	if ( ISOFFENDER() || ( m_sAssault.emCrow!=CROW_PC && m_sAssault.emCrow!=CROW_SUMMON ) )
	{
		m_nDECEXP = __int64 ( GLOGICEX::GLDIE_DECEXP(GETLEVEL()) * 0.01f * GET_LEVELUP_EXP() );
		
		if ( bCONFT )
		{
			//m_nDECEXP /= 10;	//	��ý� ����ġ �ջ��� �ݰ���Ų��.
			m_nDECEXP = 0;		//	��ý� ����ġ �ջ��� ����.
		}
		else
		{
			m_nDECEXP = __int64 ( m_nDECEXP * GET_PK_DECEXP_RATE()/100.0f );
		}
	}

	//	Note : ����ġ �߻�.
	//
	if ( m_nDECEXP > 0 )
	{
		//STARGETID mytarid(GETCROW(),m_dwGaeaID);

		//if ( m_sAssault.emCrow == CROW_PC && m_sAssault.dwID!=GAEAID_NULL )
		//{
		//	PGLCHAR pChar = m_pGLGaeaServer->GetChar ( m_sAssault.dwID );
		//	if ( pChar )
		//	{
		//		pChar->ReceiveKillExp ( mytarid, bCONFT_SCHOOL );
		//	}
		//}

		//if ( !m_cDamageLog.empty() )
		//{
		//	DAMAGELOG_ITER iter = m_cDamageLog.begin ();
		//	DAMAGELOG_ITER iter_end = m_cDamageLog.end ();
		//	for ( ; iter!=iter_end; ++iter )
		//	{
		//		SDAMAGELOG &sDamageLog = (*iter).second;

		//		PGLCHAR pChar = m_pGLGaeaServer->GetChar ( sDamageLog.dwGaeaID );
		//		if ( !pChar )	continue;
		//		if ( sDamageLog.dwUserID != pChar->m_dwUserID )		continue;

		//		pChar->ReceiveAttackExp ( mytarid, sDamageLog.dwDamage, FALSE, bCONFT_SCHOOL );
		//	}
		//}
	}

	m_cDamageLog.clear ();

	/*pvp capture the flag, Juver, 2018/02/07 */
	if ( m_pLandMan && m_pLandMan->m_bPVPCaptureTheFlagMap && m_bCaptureTheFlagHoldFlag )
	{
		GLPVPCaptureTheFlagField::GetInstance().FlagResetBase( m_dwCharID, CAPTURE_THE_FLAG_FLAG_HOLD_STATUS_DEAD );
	}

	// ���� �̺�Ʈ �߿��� ����ġ �������� �ʰ� �����۵� ������� ����
	if ( bBRIGHTEVENT ) 
	{
		m_nDECEXP = 0; 
		return;
	}

	// Ŭ�� ��Ʋ���̸� ����ġ �ս� �� ������ ��� ����
	if ( m_sAssault.emCrow==CROW_PC && m_pLandMan->IsClubBattleZone() )
	{
		GLClubMan &cClubMan = GLGaeaServer::GetInstance().GetClubMan();
		GLCLUB *pCLUB = cClubMan.GetClub ( m_dwGuild );
		if ( pCLUB )
		{
			GLChar* pChar = GLGaeaServer::GetInstance().GetChar( m_sAssault.dwID );
			if ( pChar )
			{
				if ( pCLUB->IsBattle( pChar->m_dwGuild ) )
				{
					m_nDECEXP = 0;
					return;
				}
				
				GLCLUB *pCLUB_Tar = cClubMan.GetClub ( pChar->m_dwGuild );
				if ( pCLUB_Tar && pCLUB->IsBattleAlliance( pCLUB_Tar->m_dwAlliance ) )
				{
					m_nDECEXP = 0;
					return;
				}
			}
		}
	}

	/*skill summon, Juver, 2017/10/10 */
	if ( m_sAssault.emCrow==CROW_SUMMON && m_pLandMan->IsClubBattleZone() )
	{
		GLClubMan &cClubMan = GLGaeaServer::GetInstance().GetClubMan();
		GLCLUB *pCLUB = cClubMan.GetClub ( m_dwGuild );
		if ( pCLUB )
		{
			PGLSUMMONFIELD psummon_field = GLGaeaServer::GetInstance().GetSummon( m_sAssault.dwID );
			if ( psummon_field && psummon_field->m_pOwner )
			{
				if ( pCLUB->IsBattle( psummon_field->m_pOwner->m_dwGuild ) )
				{
					m_nDECEXP = 0;
					return;
				}

				GLCLUB *pCLUB_Tar = cClubMan.GetClub ( psummon_field->m_pOwner->m_dwGuild );
				if ( pCLUB_Tar && pCLUB->IsBattleAlliance( pCLUB_Tar->m_dwAlliance ) )
				{
					m_nDECEXP = 0;
					return;
				}
			}
		}
	}

	// ����ġ NonDrop ���ϰ��
	if ( !m_pLandMan->IsDecreaseExpMap () ) m_nDECEXP = 0;

	//	Note : ����� �ƴϰ� ��Ӹ��� ��� ������ ���. && m_bProtectPutOnItem(���� ������ ��ȣ��ų)
	//
	if ( m_pLandMan && m_pLandMan->IsItemDrop() && !bCONFT && !m_bProtectPutOnItem )
	{
		//	�������� ���
		if ( ISOFFENDER() )
		{
			DWORD dwDROP_NUM = GET_PK_ITEMDROP_NUM();
			float fDROP_RATE = GET_PK_ITEMDROP_RATE();
			DROP_PUTONITEM ( dwDROP_NUM, fDROP_RATE );
		}
		//	�Ϲ������� �������.
		else if ( GLCONST_CHAR::fNONPK_ITEM_DROP!=0 )
		{
			DROP_PUTONITEM ( 1, GLCONST_CHAR::fNONPK_ITEM_DROP );
		}
	}
}

void GLChar::DelPlayHostile ()
{
//	GLGaeaServer & glGaeaServer = m_pGLGaeaServer;

	GLMSG::SNETPC_PLAYERKILLING_DEL NetMsgDel;
	PGLCHAR pCHAR = NULL;
	for ( MAPPLAYHOSTILE_ITER pos = m_mapPlayHostile.begin(); pos != m_mapPlayHostile.end(); ++pos )
	{
		//	Note : ������ ���� �˸�.		
		NetMsgDel.dwCharID = pos->first;
		m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID (), &NetMsgDel );

		pCHAR = m_pGLGaeaServer->GetCharID ( pos->first );
		if( pCHAR )
			pCHAR->DelPlayHostile ( m_dwCharID );

		if( !pos->second )
			CDebugSet::ToLogFile( "GLChar::DelPlayHostile, pos->second = NULL" );
	}

	DEL_PLAYHOSTILE_ALL();
}

void GLChar::DelPlayHostile ( DWORD dwCharID )
{
	bool bDEL = DEL_PLAYHOSTILE ( dwCharID );
	if ( bDEL )
	{
		//	Note : ������ ���� �˸�.
		GLMSG::SNETPC_PLAYERKILLING_DEL NetMsgDel;
		NetMsgDel.dwCharID = dwCharID;
		m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID (), &NetMsgDel );

		/*skill summon, Juver, 2017/10/10 */
		if ( m_pLandMan )
		{
			for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
			{
				PGLSUMMONFIELD psummon_field = m_pGLGaeaServer->GetSummon( m_dwSummonGUID_FLD[i] );
				if ( psummon_field )
				{
					if ( psummon_field->GetAttackTar().emCrow == CROW_PC )
					{
						PGLCHAR pchar_field = m_pGLGaeaServer->GetChar( psummon_field->GetAttackTar().dwID );
						if ( pchar_field && pchar_field->m_dwCharID == dwCharID )
						{
							psummon_field->ResetAttackTarget();
						}
					}
				}
			}
		}
		
	}
}

//	Ư�� Ŭ���� �����ڸ� ��� �����.
void GLChar::DelPlayHostileClub( DWORD dwClubID )
{
	PGLCHAR pCHAR = NULL;

	GLMSG::SNETPC_PLAYERKILLING_DEL NetMsgDel;
	
	MAPPLAYHOSTILE_ITER pos = m_mapPlayHostile.begin();
	MAPPLAYHOSTILE_ITER pos_end = m_mapPlayHostile.end();
	MAPPLAYHOSTILE_ITER del;

	GLClubMan &sClubMan = m_pGLGaeaServer->GetClubMan();
	GLCLUB *pClub = sClubMan.GetClub(m_dwGuild);
	if ( !pClub )	return;
	
	for ( ; pos != pos_end;  )
	{
		del = pos++;
		//	Note : ������ ���� �˸�.	
		pCHAR = m_pGLGaeaServer->GetCharID ( del->first );
		if ( !pCHAR ) continue;

		GLCLUB *pClub_S = sClubMan.GetClub(pCHAR->m_dwGuild);
		if ( !pClub_S )	return;

		if ( pClub->IsBattle( pCHAR->m_dwGuild ) || pClub->IsBattleAlliance( pClub_S->m_dwAlliance ) )
		{
			NetMsgDel.dwCharID = del->first;
			m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID (), &NetMsgDel );
			
			pCHAR->DelPlayHostile ( m_dwCharID );
		}

		m_mapPlayHostile.erase( del );
	}
}


void GLChar::DoFalling ()
{
//	GLGaeaServer & glGaeaServer = m_pGLGaeaServer;

	if ( m_sCONFTING.IsCONFRONTING() )
	{
		switch ( m_sCONFTING.emTYPE )
		{
		case EMCONFT_ONE:
			{
				/*skill summon, Juver, 2017/10/10 */
				BOOL bDie = FALSE;
				if ( m_pGLGaeaServer && m_sAssault.emCrow == CROW_SUMMON )
				{
					PGLSUMMONFIELD psummon_field = m_pGLGaeaServer->GetSummon( m_sAssault.dwID );
					if ( psummon_field && psummon_field->m_pOwner )
					{
						if ( psummon_field->m_pOwner->m_dwGaeaID != m_sCONFTING.dwTAR_ID )
							bDie = TRUE;
					}
				}
				else
				{
					STARGETID sCONFTID(CROW_PC,m_sCONFTING.dwTAR_ID);
					if( m_sAssault != sCONFTID )	bDie = TRUE;
				}

				//	Note : ��õ��� ����ڰ� �ƴ� ĳ���ͳ� ������ ���� ���.
				//
				if( bDie )
				{
					//	Note : ���� ����� ID ���. Reset �ÿ� ��ȿȭ�Ǳ� ������ �̸� ���.
					//
					const DWORD dwTAR_ID = m_sCONFTING.dwTAR_ID;

					//	Note : (�ڽ�) ��� ����.
					//
					ResetConfront(EMCONFRONT_END_FAIL);

					//	Note : ��� ���� ã��.
					//
					PGLCHAR pCONFT = m_pGLGaeaServer->GetChar(dwTAR_ID);
					if ( pCONFT )
					{
						//	Note : ��� ������ ������ �� ����ġ �α� ���Ž�Ŵ.
						//
						DAMAGELOG_ITER iter;
						iter = m_cDamageLog.find ( pCONFT->m_dwUserID );
						if ( iter!=m_cDamageLog.end() )		m_cDamageLog.erase ( iter );

						//	Note : (����) ��� ����.
						//
						pCONFT->ResetConfront ( EMCONFRONT_END_FAIL );
					}

					goto _DIE_ACTION;
				}

				//	Note : �������� ���� �߻�. ( ����ġ + ������ + �ݾ� �� �� )
				//
				GenerateReward ( true, m_sCONFTING.sOption.bSCHOOL );

				//	Note : ���� ����� ID ���. Reset �ÿ� ��ȿȭ�Ǳ� ������ �̸� ���.
				//
				const DWORD dwTAR_ID = m_sCONFTING.dwTAR_ID;
				const SCONFT_OPTION sOption = m_sCONFTING.sOption;

				//	Note : (�ڽ�) ��� ����.
				//
				ResetConfront(EMCONFRONT_END_LOSS);

				//	Note : ��� ���� ã��.
				//
				PGLCHAR pCONFT = m_pGLGaeaServer->GetChar(dwTAR_ID);
				if ( pCONFT==NULL )			return;

				//	Note : (����) ��� ����.
				//
				EMCONFRONT_END emCONFT_END = (pCONFT->GETHP()>0) ? EMCONFRONT_END_WIN : EMCONFRONT_END_LOSS;
				pCONFT->ResetConfront ( emCONFT_END );

				//	Note : �ڽ� ��Ȱ���� ��ȭ.
				//
				if ( GLCONST_CHAR::nCONFRONT_LOSS_LP!=0 && GLCONST_CHAR::nCONFRONT_WIN_LP!=0 )
				{
					float fLIFE_P(0);

					fLIFE_P = GLCONST_CHAR::nCONFRONT_LOSS_LP - (GETLEVEL()-pCONFT->GETLEVEL())/2.0f
						+ (sOption.fHP_RATE-sOption.fTAR_HP_RATE);
					LIMIT ( fLIFE_P, 0.0f, -20.0f );

					ReceiveLivingPoint ( (int)fLIFE_P );

					//	Note : ���� ��Ȱ���� ��ȭ.
					//
					fLIFE_P = GLCONST_CHAR::nCONFRONT_WIN_LP + (GETLEVEL()-pCONFT->GETLEVEL())/2.0f
						+ (sOption.fHP_RATE-sOption.fTAR_HP_RATE);
					LIMIT ( fLIFE_P, 20.0f, 0.0f );

					pCONFT->ReceiveLivingPoint ( (int)fLIFE_P );
				}
			}
			break;

		case EMCONFT_PARTY:
			{
				bool breward(false);
				if ( m_sAssault.emCrow==CROW_PC )
				{
					GLPARTY_FIELD *pConftParty = m_pGLGaeaServer->GetParty(m_sCONFTING.dwTAR_ID);
					if ( pConftParty )
					{
						const GLPARTY_FNET *pPARTY = pConftParty->GETMEMBER(m_sAssault.dwID);
						if ( pPARTY )	breward = pPARTY->m_bConfront;
					}
				}

				/*skill summon, Juver, 2017/10/10 */
				if ( m_pGLGaeaServer && m_sAssault.emCrow==CROW_SUMMON )
				{
					PGLSUMMONFIELD psummon_field = m_pGLGaeaServer->GetSummon( m_sAssault.dwID );
					if ( psummon_field && psummon_field->m_pOwner )
					{
						GLPARTY_FIELD *pConftParty = m_pGLGaeaServer->GetParty(m_sCONFTING.dwTAR_ID);
						if ( pConftParty )
						{
							const GLPARTY_FNET *pPARTY = pConftParty->GETMEMBER(psummon_field->m_pOwner->m_dwGaeaID);
							if ( pPARTY )	breward = pPARTY->m_bConfront;
						}
					}
				}

				if ( !breward )
				{
					m_cDamageLog.clear();

					//	Note : (�ڽ�) ��� ����.
					//
					ResetConfront(EMCONFRONT_END_NOTWIN);

					goto _DIE_ACTION;
				}

				//	Note : �������� ���� �߻�. ( ����ġ + ������ + �ݾ� �� �� )
				//
				GenerateReward ( true, m_sCONFTING.sOption.bSCHOOL );

				//	Note : (�ڽ�) ��� ����.
				//
				ResetConfront(EMCONFRONT_END_NOTWIN);
			}
			break;

		case EMCONFT_GUILD:
			{
				bool breward(false);
				if ( m_sAssault.emCrow==CROW_PC )
				{
					GLClubMan &sClubMan = m_pGLGaeaServer->GetClubMan();
					PGLCHAR pCHAR = m_pGLGaeaServer->GetChar(m_sAssault.dwID);

					GLCLUB *pConftClub = sClubMan.GetClub(m_sCONFTING.dwTAR_ID);
					if ( pConftClub && pCHAR )
					{
						breward = pConftClub->ISCONFT_MEMBER ( pCHAR->m_dwCharID );
					}
				}

				/*skill summon, Juver, 2017/10/10 */
				if ( m_pGLGaeaServer && m_sAssault.emCrow==CROW_SUMMON )
				{
					PGLSUMMONFIELD psummon_field = m_pGLGaeaServer->GetSummon( m_sAssault.dwID );
					if ( psummon_field && psummon_field->m_pOwner )
					{
						GLClubMan &sClubMan = m_pGLGaeaServer->GetClubMan();
						PGLCHAR pCHAR = m_pGLGaeaServer->GetChar(psummon_field->m_pOwner->m_dwGaeaID);

						GLCLUB *pConftClub = sClubMan.GetClub(m_sCONFTING.dwTAR_ID);
						if ( pConftClub && pCHAR )
						{
							breward = pConftClub->ISCONFT_MEMBER ( pCHAR->m_dwCharID );
						}
					}
				}

				if ( !breward )
				{
					m_cDamageLog.clear();

					//	Note : (�ڽ�) ��� ����.
					//
					ResetConfront(EMCONFRONT_END_NOTWIN);

					goto _DIE_ACTION;
				}

				//	Note : �������� ���� �߻�. ( ����ġ + ������ + �ݾ� �� �� )
				//
				GenerateReward ( true );

				//	Note : (�ڽ�) ��� ����.
				//
				ResetConfront(EMCONFRONT_END_NOTWIN);
			}
			break;
		}

		//	Memo :	������̶�� �Ʒ��ڵ�� �������� �ʴ´�.
		return;
	}

	if ( m_sQITEMFACT.IsACTIVE() )
	{
		m_sQITEMFACT.RESET();

		GLMSG::SNETPC_QITEMFACT_END_BRD	NetMsgBrd;
		NetMsgBrd.dwGaeaID = m_dwGaeaID;

		//	Note : ����Ǿ��� ��� �ڽſ��� �˸�.
		m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID (), &NetMsgBrd );

		//	Note : ����Ǿ��� ��� �ֺ� ������� �˸�.
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
	}

	if ( m_sPKCOMBOCOUNT.IsACTIVE() )
	{
		m_sPKCOMBOCOUNT.RESET();

		GLMSG::SNETPC_PKCOMBO_END_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID = m_dwGaeaID;

		m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID(), &NetMsgBrd );
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
	}

_DIE_ACTION:

	//	Note : player �� �׿��� ���.
	DWORD dwKILL_ID(0);

	PGLCHAR pCHAR  =  NULL;

	/*skill summon, Juver, 2017/10/10 */
	if ( m_sAssault.emCrow==CROW_PC )
		pCHAR = m_pGLGaeaServer->GetChar(m_sAssault.dwID);
	else if (  m_sAssault.emCrow==CROW_SUMMON )
	{
		PGLSUMMONFIELD psummon_field = m_pGLGaeaServer->GetSummon( m_sAssault.dwID );
		if ( psummon_field && psummon_field->m_pOwner )
			pCHAR = psummon_field->m_pOwner;
	}

	if ( pCHAR )
	{
		dwKILL_ID = pCHAR->m_dwCharID;
		int nBRIGHT(0), nLIFE(0);

		bool bClubBattle = false;
		bool bAllianceBattle = false;
		GLClubMan &cClubMan = m_pGLGaeaServer->GetClubMan();
		GLCLUB *pMyClub = cClubMan.GetClub ( m_dwGuild );
		GLCLUB *pTarClub = cClubMan.GetClub ( pCHAR->m_dwGuild );
		
		if ( pMyClub && pTarClub )
		{
			bClubBattle =  pMyClub->IsBattle( pCHAR->m_dwGuild );
			bAllianceBattle = pMyClub->IsBattleAlliance ( pTarClub->m_dwAlliance );
		}

		/////////////////////////////////////////////////////////////////////////////
		// LG-7 CWRanking
		if (m_pLandMan->m_bGuidBattleMap)
		{
			GLGuidance* pGuidance = GLGuidanceFieldMan::GetInstance().Find(m_pLandMan->m_dwClubMapID);
			if (pGuidance && pGuidance->IsBattle())
				pGuidance->AddCWRanking(pCHAR->m_dwCharID, m_dwCharID);
		}
		/////////////////////////////////////////////////////////////////////////////

		bool bClubDeathMatch = false;

		if ( m_pLandMan->m_bClubDeathMatchMap )
		{
			GLClubDeathMatch* pCDM = GLClubDeathMatchFieldMan::GetInstance().Find( m_pLandMan->m_dwClubMapID );
			if( pCDM && pCDM->IsBattle() ) bClubDeathMatch = true;
		}

		/*pvp tyranny, Juver, 2017/08/24 */
		bool bPVPTyranny = false;
		if ( m_pLandMan->m_bPVPTyrannyMap && GLPVPTyrannyField::GetInstance().IsBattle() )
			bPVPTyranny = true;

		/*school wars, Juver, 2018/01/19 */
		bool bPVPSchoolWars = false;
		if ( m_pLandMan->m_bPVPSchoolWarsMap && GLPVPSchoolWarsField::GetInstance().IsBattle() )
			bPVPSchoolWars = true;

		/*pvp capture the flag, Juver, 2018/01/31 */
		bool bPVPCaptureTheFlag = false;
		if ( m_pLandMan->m_bPVPCaptureTheFlagMap && GLPVPCaptureTheFlagField::GetInstance().IsBattle() )
			bPVPCaptureTheFlag = true;

		// Ŭ����Ʋ�� �׾��� ��� ó��
		if ( ( bClubBattle || bAllianceBattle ) && m_pLandMan->IsClubBattleZone() )
		{
			ClubBattleSetPoint( m_dwGuild, pCHAR->m_dwGuild, bAllianceBattle );

			DWORD dwClubID_P, dwClubID_S;
			if ( bAllianceBattle ) 
			{
				dwClubID_P = pMyClub->m_dwAlliance;
				dwClubID_S = pTarClub->m_dwAlliance;
			}
			else
			{
				dwClubID_P = m_dwGuild;
				dwClubID_S = pCHAR->m_dwGuild;
			}

			GLMSG::SNET_CLUB_BATTLE_POINT_UPDATE NetMsg;

			NetMsg.dwBattleClubID = dwClubID_S;
			NetMsg.bKillPoint = false;
			
			m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsg );

			NetMsg.dwBattleClubID = dwClubID_P;
			NetMsg.bKillPoint = true;
												
			m_pGLGaeaServer->SENDTOCLIENT ( pCHAR->m_dwClientID, &NetMsg );
		
		}
		//	CDM ��Ȳ
		else if ( bClubDeathMatch )
		{
			GLClubDeathMatch* pCDM = GLClubDeathMatchFieldMan::GetInstance().Find( m_pLandMan->m_dwClubMapID );	
			if ( pCDM )
			{
				pCDM->AddCDMScore( pCHAR->m_dwGuild, m_dwGuild );

				//	POINT UPDATE ����, ����
				GLMSG::SNET_CLUB_DEATHMATCH_POINT_UPDATE NetMsg;
				NetMsg.bKillPoint = false;					
				m_pGLGaeaServer->SENDTOCLUBCLIENT_ONMAP ( m_pLandMan->GetMapID().dwID, m_dwGuild, &NetMsg );

				NetMsg.bKillPoint = true;														
				m_pGLGaeaServer->SENDTOCLUBCLIENT_ONMAP ( pCHAR->m_pLandMan->GetMapID().dwID, pCHAR->m_dwGuild, &NetMsg );

			}

		}
		/*pvp tyranny, Juver, 2017/08/24 */
		else if ( bPVPTyranny )
		{
			GLPVPTyrannyField::GetInstance().ScoreKill( m_dwCharID, pCHAR->m_dwCharID );
		}
		/*school wars, Juver, 2018/01/19 */
		else if ( bPVPSchoolWars )
		{
			GLPVPSchoolWarsField::GetInstance().ScoreKill( m_dwCharID, pCHAR->m_dwCharID );
		}
		/*pvp capture the flag, Juver, 2018/02/01 */
		else if ( bPVPCaptureTheFlag )
		{
			GLPVPCaptureTheFlagField::GetInstance().ScoreKill( m_dwCharID, pCHAR->m_dwCharID );
			/*pk info, Juver, 2017/11/17 */
			//pCHAR->PKIncreaseScore( m_dwCharID );
			//PKIncreaseDeath( pCHAR->m_dwCharID );
		}
		else if ( (m_sMapID.wMainID == 214||m_sMapID.wMainID == 213||m_sMapID.wMainID == 212||m_sMapID.wMainID == 211) && m_pGLGaeaServer->IsClubBattleStarted() )
		{
			/*pk info, Juver, 2017/11/17 */
			//pCHAR->PKIncreaseScore( m_dwCharID );
			//PKIncreaseDeath( pCHAR->m_dwCharID );
		}
		else if ( m_sMapID.wMainID == 251 && m_pGLGaeaServer->IsClubDMStarted() )
		{
			//pCHAR->PKIncreaseScore( m_dwCharID );
			//PKIncreaseDeath( pCHAR->m_dwCharID );
		}
		//	Note : ���� ���� �������� ���.
		else if ( !IS_HOSTILE_ACTOR(pCHAR->m_dwCharID) && pCHAR->IS_HOSTILE_ACTOR(m_dwCharID) )
		{
			//	Note : ������ ����� ��� ���濡�� ��Ȱ���� ��.
			//

			if ( ISOFFENDER() )
			{
				DWORD dwMY_PK_LEVEL = GET_PK_LEVEL();
				if ( dwMY_PK_LEVEL != UINT_MAX && dwMY_PK_LEVEL>0 )
				{
					//	Note : ��Ȱ���� ��ȭ.
					nLIFE = 7-(GLCONST_CHAR::EMPK_STATE_LEVEL-dwMY_PK_LEVEL);
					pCHAR->ReceiveLivingPoint ( nLIFE );
				}
			}
			//	�ڽ��� ����� �Ϲ��л��� ���, ���� �Ӽ�, ���� ����.
			else
			{
				bool bGuidBattleMap = m_pLandMan->m_bGuidBattleMap;
				bool bFreePKMap = m_pLandMan->IsFreePK();
				bool bSCHOOL_FREEPK = GLSchoolFreePK::GetInstance().IsON();
				bool bBRIGHTEVENT = m_pGLGaeaServer->IsBRIGHTEVENT(); // ���� �̺�Ʈ��				
				
				if ( GetSchool() == pCHAR->GetSchool() )	bSCHOOL_FREEPK = false;

				if ( !(bGuidBattleMap||bSCHOOL_FREEPK||bFreePKMap) && !bBRIGHTEVENT )
				{
					nBRIGHT = GLCONST_CHAR::nPK_KILL_BRIGHT_POINT;
					nLIFE = GLCONST_CHAR::nPK_KILL_LIVING_POINT;

					//	Note : �Ӽ���ġ ��ȭ.
					pCHAR->m_nBright += nBRIGHT;

					GLMSG::SNETPC_UPDATE_BRIGHT NetMsg;
					NetMsg.nBright = pCHAR->m_nBright;
					m_pGLGaeaServer->SENDTOCLIENT(pCHAR->m_dwClientID,&NetMsg);

					GLMSG::SNETPC_UPDATE_BRIGHT_BRD NetMsgBrd;
					NetMsgBrd.dwGaeaID = pCHAR->m_dwGaeaID;
					NetMsgBrd.nBright = pCHAR->m_nBright;
					pCHAR->SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

					//	Note : ��Ȱ���� ��ȭ.
					pCHAR->m_nLiving += nLIFE;

					GLMSG::SNETPC_UPDATE_LP NetMsgLp;
					NetMsgLp.nLP = pCHAR->m_nLiving;
					m_pGLGaeaServer->SENDTOCLIENT(pCHAR->m_dwClientID,&NetMsgLp);
				}
			}
		}

		//	Note : �׿��� ��� �α� ���.
		GLITEMLMT::GetInstance().ReqAction
		(
			dwKILL_ID,					//	�����.
			EMLOGACT_KILL,				//	����.
			ID_CHAR, m_dwCharID,		//	����.
			0,							//	exp
			nBRIGHT,					//	bright
			nLIFE,						//	life
			0							//	money
		);

		/*activity system, Juver, 2017/11/01 */
		if ( m_pLandMan )
		{
			pCHAR->DoActivityMapKill( m_pLandMan->GetMapID() );
		}

		pCHAR->ReceivePKCombo ( 1 );

		/*pk info, Juver, 2017/11/17 */
		//pCHAR->PKIncreaseScore( m_dwCharID );
		//PKIncreaseDeath( pCHAR->m_dwCharID );

		if( bPVPTyranny || bPVPSchoolWars || bPVPCaptureTheFlag || bClubBattle || bClubDeathMatch )
		{
			pCHAR->PKIncreaseScore( m_dwCharID );
			PKIncreaseDeath( pCHAR->m_dwCharID );
		}

		if( m_wLevel == GLCONST_CHAR::wMAX_LEVEL && pCHAR->m_wLevel == m_wLevel ) {
			if ( m_pGLGaeaServer )
				m_pGLGaeaServer->PkRankAdd( m_dwCharID, pCHAR->m_dwCharID );
		}

		if( m_sMapID.wMainID == 211 || m_sMapID.wMainID == 212 || m_sMapID.wMainID == 213 || m_sMapID.wMainID == 214 ){
			if ( m_pGLGaeaServer->IsClubBattleStarted() && pCHAR->m_wLevel == m_wLevel && m_wLevel == GLCONST_CHAR::wMAX_LEVEL ){
				pCHAR->m_dwCWKill += 1;
				m_dwCWDeath += 1;
			}
		}
	}

	//	Note : ���� ������ ����.
	DelPlayHostile();

	//	Note : �������� ���� �߻�. ( ����ġ + ������ + �ݾ� )
	GenerateReward ();

	m_wObviousCount = 0;

	// �ð��� ��ȥ�� ����� ������ ����ġ�� 0���� �����.
	if ( m_nDECEXP > 0 )
	{
		bool bItemRebirth = ISREVIVE() ? true : false;
		if ( bItemRebirth )
		{
			// �Ҹ� ��ȥ���̸� �ڵ� ��ȥ�� ���
			SITEM* pITEM = GET_SLOT_ITEMDATA(SLOT_ORNAMENT);
			if ( !pITEM->ISINSTANCE() ) m_nDECEXP = 0;
		}
	}


	m_cDamageLog.clear();

	////	Note : ������ �α� ���.
	// ��Ȱ�ϱ� ��ư�� ������ �ʾ� �α׸� ������� ��찡 �־ ������ ������ �����.
	{
		EMIDTYPE emKILL = ID_MOB;
		if ( m_sAssault.emCrow==CROW_PC )	emKILL = ID_CHAR;

		GLITEMLMT::GetInstance().ReqAction
		(
			m_dwCharID,					//	�����.
			EMLOGACT_DIE,				//	����.
			emKILL, dwKILL_ID,			//	����.
			m_nDECEXP,					// exp
			0,							// bright
			0,							// life
			0							// money
		);
	}

	//	Note : ����Ʈ ���� ����� ������ ���� �ɼ� �˻��� ó��.
	DoQuestCheckLimitDie ();

	//	������ ����.
	SetSTATE(EM_ACT_DIE);

	//	Note : �������� Ŭ���̾�Ʈ�鿡�� �˸�.
	GLMSG::SNET_ACTION_BRD NetMsgBrd;
	NetMsgBrd.emCrow	= CROW_PC;
	NetMsgBrd.dwID		= m_dwGaeaID;
	NetMsgBrd.emAction	= GLAT_FALLING;

	//	Note : �ֺ��� Char ����.
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

	//	Note : �ڽ��� Ŭ���̾�Ʈ��.
	m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsgBrd );

	//	���� ��ȭ.
	TurnAction ( GLAT_FALLING );

	// ��Ʋ�ξ� �̺�Ʈ�϶� �������� �������� ��ȯ�Ѵ�.
	if ( GLCONST_CHAR::bBATTLEROYAL )
	{
		SetSTATE(EM_REQ_VISIBLEOFF);

		//	Note : (������Ʈ����) �޼��� �߻�.
		//
		GLMSG::SNETPC_ACTSTATE NetMsgFld;
		NetMsgFld.dwActState = m_dwActState;
		m_pGLGaeaServer->SENDTOAGENT ( m_dwClientID, &NetMsgFld );
	}

	// �� Sad �׼�
	PGLPETFIELD pMyPet = m_pGLGaeaServer->GetPET ( m_dwPetGUID );
	if ( pMyPet && pMyPet->IsValid () )
	{
		pMyPet->ReSetAllSTATE ();
		pMyPet->SetSTATE ( EM_PETACT_SAD );
		
		// Ŭ���̾�Ʈ�� �˸�
		GLMSG::SNETPET_SAD PetNetMsg;
		m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &PetNetMsg );

		GLMSG::SNETPET_SAD_BRD PetNetMsgBrd;
		PetNetMsgBrd.dwGUID   = pMyPet->m_dwGUID;
		PetNetMsgBrd.dwOwner  = m_dwGaeaID;

		SendMsgViewAround ( (NET_MSG_GENERIC*) &PetNetMsgBrd );
	}

	// �׾��� ��� ��ȯ���� ����

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
	{
		PGLSUMMONFIELD pMySummon = m_pGLGaeaServer->GetSummon( m_dwSummonGUID_FLD[i] );
		if ( pMySummon && pMySummon->IsValid() )
		{
			m_pGLGaeaServer->DropOutSummon ( m_dwSummonGUID_FLD[i] );
		}
	}

	//fastrevive fix
	m_bDeathTimer = TRUE;
	m_fDeathTimer = 0.0f;

	GLMSG::SNETPC_REQ_DEATHTIMER_AG NetMsgAg;
	GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgAg );
		
}

HRESULT GLChar::TourchQItem ( DWORD dwGlobID )
{
	PITEMDROP pItemDrop = m_pLandMan->GetItem ( dwGlobID );
	if ( !pItemDrop )	return S_FALSE;

	/*qbox check, Juver, 2017/12/05 */
	if ( !pItemDrop->bMobGen && !m_pLandMan->IsQBoxEnabled() )
	{
		CDebugSet::ToFileWithTime( "_qbox_check.txt", "[%d]%s attempt to loot qbox in non qbox map", m_dwCharID, m_szName );
		return S_FALSE;
	}

//	GLGaeaServer & glGaeaServer = m_pGLGaeaServer;
	GLItemMan & glItemMan = GLItemMan::GetInstance();
	
	const SITEM *pItemData = glItemMan.GetItem(pItemDrop->sItemCustom.sNativeID);
	if ( !pItemData )	return S_FALSE;

	if ( !pItemDrop->IsTakeItem(m_dwPartyID,m_dwGaeaID) )	return S_FALSE;

	if ( pItemData->sBasicOp.emItemType!=ITEM_QITEM )		return S_FALSE;

	const ITEM::SQUESTIONITEM &sQUESTIONITEM = pItemData->sQuestionItem;

	//	Note : ?�������� ����. ��Ƽ�� ��� ������ ��Ƽ������ ��ģ��.
	//
	switch ( sQUESTIONITEM.emType )
	{
	case QUESTION_SPEED_UP:
	case QUESTION_CRAZY:
	case QUESTION_ATTACK_UP:
	case QUESTION_EXP_UP:
	case QUESTION_LUCKY:
	case QUESTION_SPEED_UP_M:
	case QUESTION_MADNESS:
	case QUESTION_ATTACK_UP_M:
		{
			SNATIVEID sNativeID;

			// ���� ���ι��̴��� �ؿ��� ���
			if ( m_pGLGaeaServer->m_nServiceProvider != SP_KOREA )
			{
                if ( sQUESTIONITEM.emType == QUESTION_SPEED_UP_M ||
				     sQUESTIONITEM.emType == QUESTION_MADNESS    ||
				     sQUESTIONITEM.emType == QUESTION_ATTACK_UP_M )
				{
                    sNativeID = pItemData->sBasicOp.sNativeID;
				}
				else
				{
                    sNativeID = glItemMan.GetRandomQItemID ( pItemDrop->sItemCustom.sNativeID );
				}
			}
			else
			{
				sNativeID = glItemMan.GetRandomQItemID ( pItemDrop->sItemCustom.sNativeID );
			}
				
			GLPARTY_FIELD* pParty = m_pGLGaeaServer->GetParty(m_dwPartyID);

			// ���Ӱ� �߰��� ?�������� ��� ������ ���� ������� ó��
			if ( sNativeID == NATIVEID_NULL() )
			{
				if ( pParty )
				{
					GLPARTY_FIELD::MEMBER_ITER pos = pParty->m_cMEMBER.begin();
					GLPARTY_FIELD::MEMBER_ITER end = pParty->m_cMEMBER.end();
					for ( ; pos!=end; ++pos )
					{
						PGLCHAR pCHAR = m_pGLGaeaServer->GetChar ( (*pos).first );
						if ( !pCHAR )	continue;
						if ( pCHAR->m_sMapID!=m_sMapID )	continue;

						D3DXVECTOR3 vDist = pCHAR->m_vPos - m_vPos;
						float fDist = D3DXVec3Length ( &vDist );
						if ( MAX_VIEWRANGE < fDist )		continue;

						/*qbox check, Juver, 2017/12/05 */
						pCHAR->RecieveQItemFact ( pItemDrop->sItemCustom.sNativeID, pItemDrop->bMobGen );
					}
				}
				else
				{
					/*qbox check, Juver, 2017/12/05 */
					RecieveQItemFact ( pItemDrop->sItemCustom.sNativeID, pItemDrop->bMobGen );
				}	
			}
			else
			{
				const SITEM *pItem = glItemMan.GetItem( sNativeID );
				if ( !pItem ) return S_FALSE;

				if ( pParty )
				{
					GLPARTY_FIELD::MEMBER_ITER pos = pParty->m_cMEMBER.begin();
					GLPARTY_FIELD::MEMBER_ITER end = pParty->m_cMEMBER.end();
					for ( ; pos!=end; ++pos )
					{
						PGLCHAR pCHAR = m_pGLGaeaServer->GetChar ( (*pos).first );
						if ( !pCHAR )	continue;
						if ( pCHAR->m_sMapID!=m_sMapID )	continue;

						D3DXVECTOR3 vDist = pCHAR->m_vPos - m_vPos;
						float fDist = D3DXVec3Length ( &vDist );
						if ( MAX_VIEWRANGE < fDist )		continue;

						/*qbox check, Juver, 2017/12/05 */
						pCHAR->RecieveQItemFact ( sNativeID , pItemDrop->bMobGen);
					}
				}
				else
				{
					/*qbox check, Juver, 2017/12/05 */
					RecieveQItemFact ( sNativeID, pItemDrop->bMobGen );
				}
			}

			/*activity system, Juver, 2017/11/04 */
			if ( m_sQITEMFACT.emType != QUESTION_NONE && m_sQITEMFACT.emType != QUESTION_SIZE )
			{
				DoActivityObtainQBox( m_sQITEMFACT.emType );
			}
		}
		break;

	case QUESTION_EXP_GET:
		{
			SQITEM_FACT sFACT;
			sFACT.emType = sQUESTIONITEM.emType;

			GLMSG::SNETPC_QITEMFACT_BRD NetMsgBrd;
			NetMsgBrd.dwGaeaID = m_dwGaeaID;
			NetMsgBrd.sFACT = sFACT;

			GLPARTY_FIELD* pParty = m_pGLGaeaServer->GetParty(m_dwPartyID);
			if ( pParty )
			{
				GLPARTY_FIELD::MEMBER_ITER pos = pParty->m_cMEMBER.begin();
				GLPARTY_FIELD::MEMBER_ITER end = pParty->m_cMEMBER.end();
				for ( ; pos!=end; ++pos )
				{
					PGLCHAR pCHAR = m_pGLGaeaServer->GetChar ( (*pos).first );
					if ( !pCHAR )						continue;
					if ( pCHAR->m_sMapID!=m_sMapID )	continue;

					D3DXVECTOR3 vDist = pCHAR->m_vPos - m_vPos;
					float fDist = D3DXVec3Length ( &vDist );
					if ( MAX_VIEWRANGE < fDist )		continue;

					int nRecExp = int ( pCHAR->m_sExperience.lnMax * sQUESTIONITEM.fExp/100.0f );
					pCHAR->ReceiveExp ( nRecExp, false );
					m_pGLGaeaServer->SENDTOCLIENT(pCHAR->m_dwClientID,&NetMsgBrd);
				}
			}
			else
			{
				int nRecExp = int ( m_sExperience.lnMax * sQUESTIONITEM.fExp/100.0f );
				ReceiveExp ( nRecExp, false );
				m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgBrd);
			}

			/*activity system, Juver, 2017/11/04 */
			DoActivityObtainQBox( sQUESTIONITEM.emType );
		}
		break;

	case QUESTION_BOMB:
		{
			SQITEM_FACT sFACT;
			sFACT.emType = sQUESTIONITEM.emType;

			GLMSG::SNETPC_QITEMFACT_BRD NetMsgBrd;
			NetMsgBrd.dwGaeaID = m_dwGaeaID;
			NetMsgBrd.sFACT = sFACT;

			GLPARTY_FIELD* pParty = m_pGLGaeaServer->GetParty(m_dwPartyID);
			if ( pParty )
			{
				GLPARTY_FIELD::MEMBER_ITER pos = pParty->m_cMEMBER.begin();
				GLPARTY_FIELD::MEMBER_ITER end = pParty->m_cMEMBER.end();
				for ( ; pos!=end; ++pos )
				{
					PGLCHAR pCHAR = m_pGLGaeaServer->GetChar ( (*pos).first );
					if ( !pCHAR )	continue;
					if ( pCHAR->m_sMapID!=m_sMapID )	continue;

					D3DXVECTOR3 vDist = pCHAR->m_vPos - m_vPos;
					float fDist = D3DXVec3Length ( &vDist );
					if ( MAX_VIEWRANGE < fDist )		continue;

					pCHAR->ReceiveDamage ( CROW_ITEM, 0, sQUESTIONITEM.wParam1, FALSE );
					m_pGLGaeaServer->SENDTOCLIENT(pCHAR->m_dwClientID,&NetMsgBrd);
				}
			}
			else
			{
				ReceiveDamage ( CROW_ITEM, 0, sQUESTIONITEM.wParam1, FALSE );
				m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgBrd);
			}
		}
		break;

	case QUESTION_MOBGEN:
		{
			SQITEM_FACT sFACT;
			sFACT.emType = sQUESTIONITEM.emType;

			GLMSG::SNETPC_QITEMFACT_BRD NetMsgBrd;
			NetMsgBrd.dwGaeaID = m_dwGaeaID;
			NetMsgBrd.sFACT = sFACT;

			GLPARTY_FIELD* pParty = m_pGLGaeaServer->GetParty(m_dwPartyID);
			if ( pParty )
			{
				GLPARTY_FIELD::MEMBER_ITER pos = pParty->m_cMEMBER.begin();
				GLPARTY_FIELD::MEMBER_ITER end = pParty->m_cMEMBER.end();
				for ( ; pos!=end; ++pos )
				{
					PGLCHAR pCHAR = m_pGLGaeaServer->GetChar ( (*pos).first );
					if ( !pCHAR )	continue;
					if ( pCHAR->m_sMapID!=m_sMapID )	continue;

					D3DXVECTOR3 vDist = pCHAR->m_vPos - m_vPos;
					float fDist = D3DXVec3Length ( &vDist );
					if ( MAX_VIEWRANGE < fDist )		continue;

					m_pGLGaeaServer->SENDTOCLIENT(pCHAR->m_dwClientID,&NetMsgBrd);
				}
			}
			else
			{
				m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgBrd);
			}

			m_pLandMan->DropCrow ( SNATIVEID(sQUESTIONITEM.wParam1,sQUESTIONITEM.wParam2), pItemDrop->vPos.x, pItemDrop->vPos.z );

			/*activity system, Juver, 2017/11/04 */
			DoActivityObtainQBox( sQUESTIONITEM.emType );
		}
		break;

	case QUESTION_HEAL:
		{
			SQITEM_FACT sFACT;
			sFACT.emType = sQUESTIONITEM.emType;

			GLMSG::SNETPC_QITEMFACT_BRD NetMsgBrd;
			NetMsgBrd.dwGaeaID = m_dwGaeaID;
			NetMsgBrd.sFACT = sFACT;

			GLPARTY_FIELD* pParty = m_pGLGaeaServer->GetParty(m_dwPartyID);
			if ( pParty )
			{
				GLPARTY_FIELD::MEMBER_ITER pos = pParty->m_cMEMBER.begin();
				GLPARTY_FIELD::MEMBER_ITER end = pParty->m_cMEMBER.end();
				for ( ; pos!=end; ++pos )
				{
					PGLCHAR pCHAR = m_pGLGaeaServer->GetChar ( (*pos).first );
					if ( !pCHAR )	continue;
					if ( pCHAR->m_sMapID!=m_sMapID )	continue;

					D3DXVECTOR3 vDist = pCHAR->m_vPos - m_vPos;
					float fDist = D3DXVec3Length ( &vDist );
					if ( MAX_VIEWRANGE < fDist )		continue;

					m_pGLGaeaServer->SENDTOCLIENT(pCHAR->m_dwClientID,&NetMsgBrd);

					pCHAR->VAR_BODY_POINT ( CROW_PC, m_dwGaeaID, FALSE, USHRT_MAX, USHRT_MAX, USHRT_MAX );
				}
			}
			else
			{
				VAR_BODY_POINT ( CROW_PC, m_dwGaeaID, FALSE, USHRT_MAX, USHRT_MAX, USHRT_MAX );
				m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID,&NetMsgBrd);
			}

			/*activity system, Juver, 2017/11/04 */
			DoActivityObtainQBox( sQUESTIONITEM.emType );
		}
		break;
	};

	//	Note : ������ Land���� ����.
	m_pLandMan->DropOutItem ( pItemDrop->dwGlobID );

	return S_OK;
}

// �������� ����ġ ���� ���ϱ�...�����ۿ� ���� ����ġ ������ ��� ���Ѵ�.
float GLChar::GetExpMultipleRateFromSlotItem()
{
	float fExpMultipleRateMax = 1.0f;

	GLItemMan & glItemMan = GLItemMan::GetInstance();

	for( int i=0; i<SLOT_TSIZE; ++i )
	{
		const SITEMCUSTOM &sItemCustoms = m_PutOnItems[i];

		if( sItemCustoms.sNativeID == SNATIVEID( false ) )
			continue;

		SITEM* pItem = glItemMan.GetItem( sItemCustoms.sNativeID );
		if( !pItem )
			continue;

		float fExpMultipleRate = pItem->GetExpMultiple();
		if(  1.0f < fExpMultipleRate )
			fExpMultipleRateMax += fExpMultipleRate - 1.0f;

		//add exp combile stats by DarkEagle
		if ( sItemCustoms.nidDISGUISE != SNATIVEID( false ) )
		{
			SITEM* pJDisguiseData = GLItemMan::GetInstance().GetItem ( sItemCustoms.nidDISGUISE );
			if ( pJDisguiseData )
			{
				float fExpDisguise = pJDisguiseData->GetExpMultiple();
				if ( 1.0f < fExpDisguise )
					fExpMultipleRateMax += fExpDisguise - 1.0f;
			}
		}
		//
	}

	/*dmk14 system buff fix*/	
	float fSystemRate = 0.0f;
	for ( int i=0; i<SYSTEM_BUFF_SIZE; ++i )
	{
		SSYSTEM_BUFF &ssystem_buff = m_sSYSTEM_BUFF[i];
		if ( ssystem_buff.sNATIVEID==NATIVEID_NULL() )	continue;

		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( ssystem_buff.sNATIVEID );
		if ( !pSkill )													continue;
		if ( !pSkill->m_sAPPLY.IsSpec( EMSPECA_GETEXP_RATE ) )			continue;

		for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
		{
			if( pSkill->m_sAPPLY.sSpecs[i].emSPEC != EMSPECA_GETEXP_RATE )	continue;
					
			fSystemRate += pSkill->m_sAPPLY.sSpecs[i].sSPEC[ ssystem_buff.wLEVEL ].fVAR1 ;
			break;
		}
	}

	fExpMultipleRateMax += fSystemRate;

	return fExpMultipleRateMax < 1.0f ? 1.0f : fExpMultipleRateMax;

}

void GLChar::SENDBOXITEMINFO ()
{
	GLMSG::SNET_GET_CHARGEDITEM_FROMDB_FB NetMsgFB;

	NetMsgFB.emFB = EMREQ_CHARGEDITEM_FROMDB_FB_OK;

	//	������ �޿��� ������ ��ǰ ��� Ŭ���̾�Ʈ�� ����

//	GLGaeaServer & glGaeaServer = m_pGLGaeaServer;

	MAPSHOP::reverse_iterator pos = m_mapCharged.rbegin();
	MAPSHOP::reverse_iterator end = m_mapCharged.rend();

	for ( ; pos!=end; ++pos )
	{
		SHOPPURCHASE &sPURCHASE = (*pos).second;

		StringCchCopy ( NetMsgFB.szPurKey, PURKEY_LENGTH+1, sPURCHASE.strPurKey.GetString() );
		NetMsgFB.nidITEM = SNATIVEID(sPURCHASE.wItemMain,sPURCHASE.wItemSub);
		m_pGLGaeaServer->SENDTOCLIENT( m_dwClientID, &NetMsgFB );
	}

	NetMsgFB.emFB = EMREQ_CHARGEDITEM_FROMDB_FB_END;
	m_pGLGaeaServer->SENDTOCLIENT( m_dwClientID, &NetMsgFB );
	return;
}

HRESULT GLChar::MGOddEvenOK( GLMSG::SNETPC_MGAME_ODDEVEN* pNetMsg )
{
	GLMSG::SNETPC_MGAME_ODDEVEN_FB NetMsgFB;

	// ���� �ݾ� üũ
	if( pNetMsg->uiBattingMoney > m_lnMoney )
	{
		NetMsgFB.emResult = EMMGAME_ODDEVEN_FB_MONEY_FAIL;
		m_pGLGaeaServer->SENDTOCLIENT( m_dwClientID, &NetMsgFB );

		return S_FALSE;
	}

	// �ִ� ���� ���� �ݾ� üũ
	if( pNetMsg->uiBattingMoney > MINIGAME_ODDEVEN::uiMaxBattingMoney )
	{
		NetMsgFB.emResult = EMMGAME_ODDEVEN_FB_MAXBATTING;
		m_pGLGaeaServer->SENDTOCLIENT( m_dwClientID, &NetMsgFB );

		return S_FALSE;
	}

	MGOddEvenInit();

	m_ui64BattingMoney = pNetMsg->uiBattingMoney;

	// ���� �Ϸ� ���� ����
	NetMsgFB.emResult = EMMGAME_ODDEVEN_FB_OK;
	m_pGLGaeaServer->SENDTOCLIENT( m_dwClientID, &NetMsgFB );

	return S_OK;
}

HRESULT GLChar::MGOddEvenCANCEL( GLMSG::SNETPC_MGAME_ODDEVEN* pNetMsg )
{
	GLMSG::SNETPC_MGAME_ODDEVEN_FB NetMsgFB;
	NetMsgFB.emResult = EMMGAME_ODDEVEN_FB_FAIL;
	m_pGLGaeaServer->SENDTOCLIENT( m_dwClientID, &NetMsgFB );

	return S_OK;
}

HRESULT GLChar::MGOddEvenSelect( GLMSG::SNETPC_MGAME_ODDEVEN* pNetMsg )
{
	m_nOddEven = pNetMsg->emCase;

	return S_OK;
}

HRESULT GLChar::MGOddEvenAGAIN( GLMSG::SNETPC_MGAME_ODDEVEN* pNetMsg )
{
	GLMSG::SNETPC_MGAME_ODDEVEN_FB NetMsgFB;

	if( pNetMsg->uiBattingMoney > m_lnMoney ) // ���� �ݾ� üũ
	{
		NetMsgFB.emResult = EMMGAME_ODDEVEN_FB_FAIL;
	}
	else
	{
		NetMsgFB.emResult = EMMGAME_ODDEVEN_FB_AGAIN_OK;
		NetMsgFB.wRound = m_wMGOddEvenCurRnd+1;

		m_ui64BattingMoney = m_ui64DividendMoney;
	}

	m_pGLGaeaServer->SENDTOCLIENT( m_dwClientID, &NetMsgFB );

	return S_OK;
}

HRESULT GLChar::MGOddEvenShuffle( GLMSG::SNETPC_MGAME_ODDEVEN* pNetMsg )
{
	std::vector<int> vecShuffleBuffer;
	vecShuffleBuffer.reserve( 100 );

	{ // Ȱ���� ���� ���� �迭 ����
		int nSuccessCnt = (int)(MINIGAME_ODDEVEN::fSuccessRate[m_wMGOddEvenCurRnd]*100.0); // ���� Ȯ��
		int i = 0;

		for( ; i<nSuccessCnt; ++i )
			vecShuffleBuffer.push_back( m_nOddEven );

		for( ; i<100; ++i)
			vecShuffleBuffer.push_back( EMMGAME_ODDEVEN_ODD - m_nOddEven );

		std::random_shuffle( vecShuffleBuffer.begin(), vecShuffleBuffer.end() );
	}

	srand ( (UINT)time( NULL ) );

	int nDiceNumber = rand() % 100;						// 0~99���� ���ڸ� �����ϰ� �����Ѵ�.
	int nHabbyNumber = vecShuffleBuffer[nDiceNumber];	// �迭���� ����

	GLITEMLMT::GetInstance().ReqMoneyExc(	ID_CHAR, m_dwCharID, 
											ID_CHAR, 0, 
											m_ui64BattingMoney, EMITEM_ROUTE_ODDEVEN );

	GLMSG::SNETPC_MGAME_ODDEVEN_FB NetMsgFB;

	if( nHabbyNumber == m_nOddEven ) // �´ٸ� ���� ����
	{
		// ���� ���
		m_ui64DividendMoney = (LONGLONG)(m_ui64BattingMoney * MINIGAME_ODDEVEN::fReturnRate[m_wMGOddEvenCurRnd]);

		NetMsgFB.emResult = EMMGAME_ODDEVEN_FB_WIN;
		NetMsgFB.ui64DividendMoney = m_ui64DividendMoney;
		NetMsgFB.ui64ActualMoney = m_ui64DividendMoney - m_ui64BattingMoney; // �Ǽ��� �ݾ� = ���� - ���ñ�
		
		CheckMoneyUpdate( m_lnMoney, NetMsgFB.ui64ActualMoney, TRUE, "Odd Event Shuffle." );
		m_bMoneyUpdate = TRUE;

		m_lnMoney += NetMsgFB.ui64ActualMoney;		
		m_wMGOddEvenCurRnd++;

		GLITEMLMT::GetInstance().ReqMoneyExc(	ID_CHAR, m_dwCharID, 
												ID_CHAR, m_wMGOddEvenCurRnd, 
												NetMsgFB.ui64ActualMoney, EMITEM_ROUTE_ODDEVEN );
	}
	else
	{
		NetMsgFB.emResult = EMMGAME_ODDEVEN_FB_LOSE;
		NetMsgFB.ui64DividendMoney = m_ui64BattingMoney;

		CheckMoneyUpdate( m_lnMoney, NetMsgFB.ui64ActualMoney, FALSE, "Odd Event Shuffle." );
		m_bMoneyUpdate = TRUE;

		m_lnMoney -= m_ui64BattingMoney;
		

		GLITEMLMT::GetInstance().ReqMoneyExc(	ID_CHAR, m_dwCharID, 
												ID_MOB, m_wMGOddEvenCurRnd, 
												m_ui64BattingMoney, EMITEM_ROUTE_ODDEVEN );
	}

	GLITEMLMT::GetInstance().ReqMoneyExc(	ID_CHAR, m_dwCharID, 
											ID_CHAR, 0, 
											m_lnMoney, EMITEM_ROUTE_ODDEVEN );

	if( nHabbyNumber )	NetMsgFB.emCase = EMMGAME_ODDEVEN_ODD;
	else				NetMsgFB.emCase = EMMGAME_ODDEVEN_EVEN;

	m_pGLGaeaServer->SENDTOCLIENT( m_dwClientID, &NetMsgFB );

	return S_OK;
}

HRESULT GLChar::MGOddEvenFinish( GLMSG::SNETPC_MGAME_ODDEVEN* pNetMsg )
{
	GLMSG::SNETPC_MGAME_ODDEVEN_FB NetMsgFB;

	// �ݺ� ȸ���� üũ
	if( m_wMGOddEvenCurRnd >= MINIGAME_ODDEVEN::MAX_ROUND )
	{
		NetMsgFB.emResult = EMMGAME_ODDEVEN_FB_MAXROUND;
	}
	else
	{
		NetMsgFB.emResult = EMMGAME_ODDEVEN_FB_FINISH_OK;
		NetMsgFB.wRound = m_wMGOddEvenCurRnd;
		NetMsgFB.ui64DividendMoney = m_ui64DividendMoney;
	}

	m_pGLGaeaServer->SENDTOCLIENT( m_dwClientID, &NetMsgFB );

	return S_OK;
}

HRESULT GLChar::UpdateVehicle( float fElapsedTime )
{
	if ( !m_bVehicle || !m_sVehicle.IsActiveValue() ) return S_OK;
	m_fVehicleTimer += fElapsedTime;
	// ���� ������ ����
	if ( m_fVehicleTimer > 3600.0f/GLPeriod::REALTIME_TO_VBR )
	{
		m_fVehicleTimer = 0;
		m_sVehicle.m_nFull -= GLCONST_VEHICLE::nFullDecrVehicle[m_sVehicle.m_emTYPE];
		if ( m_sVehicle.m_nFull < 0 ) m_sVehicle.m_nFull = 0;

		// Ŭ���̾�Ʈ ������ ����
		GLMSG::SNET_VEHICLE_UPDATE_CLIENT_BATTERY NetMsg;
		NetMsg.nFull = m_sVehicle.m_nFull;

		m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsg );

		if ( m_sVehicle.IsNotEnoughFull() )	ActiveVehicle( false, false );
	}
	
	return S_OK;
}

BOOL GLChar::CheckMoneyUpdate( const LONGLONG lnCurrentMoney, const LONGLONG lnUpdateMoney, const bool bPlus, const char* msg )
{
	if( lnUpdateMoney < 100000000 && lnUpdateMoney >= 0 )
	{
		return FALSE;
	}

	char szTempChar[256] = {0,};
	string str;
	sprintf( szTempChar, "[%s][%s] %s", m_szUID, m_szName, msg );
	str = szTempChar;
	
	if( lnUpdateMoney < 0 )
	{
		str += " Not Correct Update Money Value!!";
	}

	if( bPlus )	sprintf( szTempChar, " Current Money %I64d Update Money %I64d Plus!!", lnCurrentMoney, lnUpdateMoney );
	else		sprintf( szTempChar, " Current Money %I64d Update Money %I64d Minus!!", lnCurrentMoney, lnUpdateMoney );
	str += szTempChar;

	BILLIONUPDATE_WRITE( str.c_str() );

	return TRUE;
}

void GLChar::ClubBattleSetPoint( DWORD dwClub_P, DWORD dwClub_S, bool bAlliancBattle )
{

	GLClubMan& cClubMan = GLGaeaServer::GetInstance().GetClubMan();
	GLCLUB* pClub_P = cClubMan.GetClub( dwClub_P );
	GLCLUB* pClub_S = cClubMan.GetClub( dwClub_S );

	if ( !pClub_P || !pClub_S ) return;

	if ( bAlliancBattle ) 
	{
		GLCLUB* pClub_M = cClubMan.GetClub( pClub_P->m_dwAlliance );
		if ( !pClub_M ) return;

		CLUB_ALLIANCE_ITER pos = pClub_M->m_setAlliance.begin();
		CLUB_ALLIANCE_ITER end = pClub_M->m_setAlliance.end();
		for ( ; pos!=end; ++pos )
		{
			const GLCLUBALLIANCE &sALLIANCE = *pos;

			GLCLUB *pCLUB = cClubMan.GetClub ( sALLIANCE.m_dwID );
			if ( !pCLUB ) continue;
			
			GLCLUBBATTLE* pClubBattle = pCLUB->GetClubBattle( pClub_S->m_dwAlliance );
			if ( !pClubBattle ) continue;

			pClubBattle->m_wDeathPointTemp++;
			pClubBattle->m_bKillUpdate = true;
		}

		pClub_M = cClubMan.GetClub( pClub_S->m_dwAlliance );
		if ( !pClub_M ) return;

		pos = pClub_M->m_setAlliance.begin();
		end = pClub_M->m_setAlliance.end();
		for ( ; pos!=end; ++pos )
		{
			const GLCLUBALLIANCE &sALLIANCE = *pos;

			GLCLUB *pCLUB = cClubMan.GetClub ( sALLIANCE.m_dwID );
			if ( !pCLUB ) continue;
			
			GLCLUBBATTLE* pClubBattle = pCLUB->GetClubBattle( pClub_P->m_dwAlliance );
			if ( !pClubBattle ) continue;

			pClubBattle->m_wKillPointTemp++;
			pClubBattle->m_bKillUpdate = true;
		}
	}
	else
	{
		GLCLUBBATTLE* pClubBattle = pClub_P->GetClubBattle( dwClub_S );
		if ( !pClubBattle ) return;

		pClubBattle->m_wDeathPointTemp++;
		pClubBattle->m_bKillUpdate = true;
		
		pClubBattle = pClub_S->GetClubBattle( dwClub_P );
		if ( !pClubBattle ) return;
				
		pClubBattle->m_wKillPointTemp++;
		pClubBattle->m_bKillUpdate = true;
	}
	
	return;
}

ITEM_COOLTIME*	GLChar::GetCoolTime( DWORD dwCoolID, EMCOOL_TYPE emCoolType )
{
	if ( emCoolType == EMCOOL_ITEMID )
	{
		COOLTIME_MAP_ITER pos = m_mapCoolTimeID.find(dwCoolID);
		if ( pos==m_mapCoolTimeID.end() )		return NULL;
		
		return &(*pos).second;
	}
	else if ( emCoolType == EMCOOL_ITEMTYPE )
	{
		COOLTIME_MAP_ITER pos = m_mapCoolTimeType.find(dwCoolID);
		if ( pos==m_mapCoolTimeType.end() )		return NULL;
		
		return &(*pos).second;
	}
	else	
		return NULL;
}

bool GLChar::IsCoolTime( SNATIVEID sNativeID )
{
	__time64_t tCurTime = CTime::GetCurrentTime().GetTime();
	__time64_t tCoolTime = GetMaxCoolTime( sNativeID );

	if ( tCurTime < tCoolTime ) return true;

	return false;
}

bool GLChar::CheckCoolTime( SNATIVEID sNativeID )
{
	if ( IsCoolTime ( sNativeID ) )
	{
		GLMSG::SNET_ITEM_COOLTIME_ERROR NetMsg;
		NetMsg.sNativeID = sNativeID;

		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg);
		return true;

	}
	return false;
}

__time64_t	GLChar::GetMaxCoolTime ( SNATIVEID sNativeID )
{
	//	������ ���� ������.
	SITEM* pItem = GLItemMan::GetInstance().GetItem ( sNativeID );
	if ( !pItem )			return 0;

	ITEM_COOLTIME* pCoolTimeType = GetCoolTime( (DWORD) pItem->sBasicOp.emItemType, EMCOOL_ITEMTYPE );
	ITEM_COOLTIME* pCoolTimeID = GetCoolTime( pItem->sBasicOp.sNativeID.dwID, EMCOOL_ITEMID );

	__time64_t tCoolType = 0;
	__time64_t tCoolID = 0;

	if ( pCoolTimeType )	tCoolType = pCoolTimeType->tCoolTime;
	if ( pCoolTimeID )		tCoolID = pCoolTimeID->tCoolTime;

	return (tCoolType > tCoolID) ? tCoolType : tCoolID;
}


bool GLChar::SetCoolTime( SNATIVEID sNativeID , EMCOOL_TYPE emCoolType )
{
	//	������ ���� ������.
	SITEM* pItem = GLItemMan::GetInstance().GetItem ( sNativeID );
	if ( !pItem )			return false;

	if ( !pItem->sBasicOp.IsCoolTime() ) return false;

	// ��Ÿ�� ����
	__time64_t  tCurTime = CTime::GetCurrentTime().GetTime();
	__time64_t  tCoolTime = tCurTime + pItem->sBasicOp.dwCoolTime;

	ITEM_COOLTIME* pCoolTime = GetCoolTime( sNativeID.dwID, emCoolType );
	if ( pCoolTime && tCoolTime < pCoolTime->tCoolTime ) return false;

	ITEM_COOLTIME sCoolTime;
	sCoolTime.dwID = pItem->sBasicOp.sNativeID.dwID;
	sCoolTime.tUseTime = tCurTime;
	sCoolTime.tCoolTime = tCoolTime;	

	if ( emCoolType == EMCOOL_ITEMID )
	{
		sCoolTime.dwCoolID = pItem->sBasicOp.sNativeID.dwID;
		m_mapCoolTimeID[sCoolTime.dwCoolID] = sCoolTime;
	}
	else if( emCoolType == EMCOOL_ITEMTYPE )
	{
		sCoolTime.dwCoolID = (DWORD)pItem->sBasicOp.emItemType;
		m_mapCoolTimeType[sCoolTime.dwCoolID] = sCoolTime;
	}

	GLMSG::SNETPC_ITEM_COOLTIME_UPDATE NetMsg;			
	NetMsg.dwID = sCoolTime.dwID;
	NetMsg.dwCoolID = sCoolTime.dwCoolID;
	NetMsg.tCoolTime = sCoolTime.tCoolTime;
	NetMsg.tUseTime = sCoolTime.tUseTime;
	NetMsg.emCoolType = emCoolType;

	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg);

	return true;
}

bool GLChar::UpdateNpcCommission( LONGLONG lnCommission )
{
	if ( lnCommission < 0 ) return false;
	
	CheckMoneyUpdate( m_lnStorageMoney, lnCommission, TRUE, "Add Npc Commission");
	m_lnStorageMoney += lnCommission;
	m_bStorageMoneyUpdate = TRUE;
			
	GLITEMLMT::GetInstance().ReqMoneyExc ( ID_CHAR, m_dwCharID, ID_USER, m_dwUserID, lnCommission, EMITEM_ROUTE_NPCCOME );

	GLMSG::SNETPC_REQ_STORAGE_UPDATE_MONEY NetMsgStorage;
	NetMsgStorage.lnMoney = m_lnStorageMoney;
	GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID,&NetMsgStorage);

	return true;
}


/*itemfood system, Juver, 2017/05/25 */
WORD GLChar::ItemFoodFindSlot ( SNATIVEID skill_id )
{
	WORD wSELECT = FITEMFACT_SIZE;

	for ( WORD i=0; i<FITEMFACT_SIZE; ++i )
	{
		//if exist just use the slot
		if ( m_sFITEMFACT[i].sNATIVEID==skill_id 
			&& m_sFITEMFACT[i].bENABLE )
			return wSELECT = i;
	}

	for ( WORD i=0; i<FITEMFACT_SIZE; ++i )
	{
		if ( m_sFITEMFACT[i].sNATIVEID==SNATIVEID(false) &&
			m_sFITEMFACT[i].bENABLE )
			return wSELECT = i;
	}

	return wSELECT;
}

WORD GLChar::ItemFoodAdd ( SNATIVEID skill_id, WORD wLevel, WORD wSLOT )
{
	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( skill_id );
	if ( !pSKILL )	
		return FITEMFACT_SIZE;
	
	if ( wLevel >= SKILL::MAX_LEVEL )
		return FITEMFACT_SIZE;

	if ( wSLOT == FITEMFACT_SIZE )	
		return FITEMFACT_SIZE;

	SNATIVEID _sidSKILL = pSKILL->m_sBASIC.sNATIVEID;

	BOOL bHOLD = FALSE;
	SFITEMFACT sSKILLEF;
	sSKILLEF.bENABLE = TRUE;

	SKILL::CDATA_LVL &sSKILL_DATA = pSKILL->m_sAPPLY.sDATA_LVL[wLevel]; 
	switch ( pSKILL->m_sAPPLY.emBASIC_TYPE )
	{
	case SKILL::EMFOR_VARHP:
	case SKILL::EMFOR_VARMP:
	case SKILL::EMFOR_VARSP:
	case SKILL::EMFOR_DEFENSE:
	case SKILL::EMFOR_HITRATE:
	case SKILL::EMFOR_AVOIDRATE:
	case SKILL::EMFOR_VARAP:
	case SKILL::EMFOR_VARDAMAGE:
	case SKILL::EMFOR_VARDEFENSE:
	case SKILL::EMFOR_PA:
	case SKILL::EMFOR_SA:
	case SKILL::EMFOR_MA:
	case SKILL::EMFOR_RESIST:
	case SKILL::EMFOR_SUMMONTIME: /*summon time, Juver, 2017/12/12 */
		bHOLD = TRUE;
		sSKILLEF.emTYPE = pSKILL->m_sAPPLY.emBASIC_TYPE;
		sSKILLEF.fMVAR = sSKILL_DATA.fBASIC_VAR;
		break;
	};

	for( int nImpact=0; nImpact<SKILL::MAX_IMPACT; ++nImpact )
	{
		if ( pSKILL->m_sAPPLY.sImpacts[nImpact].emADDON != EMIMPACTA_NONE )
		{
			bHOLD = TRUE;
			sSKILLEF.sImpacts[nImpact].emADDON = pSKILL->m_sAPPLY.sImpacts[nImpact].emADDON;
			sSKILLEF.sImpacts[nImpact].fADDON_VAR = pSKILL->m_sAPPLY.sImpacts[nImpact].fADDON_VAR[wLevel];
		}
	}

	for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
	{
		switch ( pSKILL->m_sAPPLY.sSpecs[nSpec].emSPEC )
		{
		case EMSPECA_REFDAMAGE:
		case EMSPECA_NONBLOW:
		case EMSPECA_PIERCE:
		case EMSPECA_TARRANGE:
		case EMSPECA_MOVEVELO:
		case EMSPECA_ATTACKVELO:
		case EMSPECA_SKILLDELAY:
		case EMSPECA_PSY_DAMAGE_REDUCE:
		case EMSPECA_MAGIC_DAMAGE_REDUCE:
		case EMSPECA_PSY_DAMAGE_REFLECTION:
		case EMSPECA_MAGIC_DAMAGE_REFLECTION:
		case EMSPECA_DEFENSE_SKILL_ACTIVE:
		case EMSPECA_CHANGE_ATTACK_RANGE:	/*skill range spec logic, Juver, 2017/06/06 */
		case EMSPECA_CHANGE_APPLY_RANGE:	/*skill range spec logic, Juver, 2017/06/06 */
		case EMSPECA_TALK_TO_NPC:			/*npc talk buff, Juver, 2017/10/03 */
			bHOLD = TRUE;
			sSKILLEF.sSpecs[nSpec].emSPEC = pSKILL->m_sAPPLY.sSpecs[nSpec].emSPEC;
			sSKILLEF.sSpecs[nSpec].fSPECVAR1 = pSKILL->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].fVAR1;
			sSKILLEF.sSpecs[nSpec].fSPECVAR2 = pSKILL->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].fVAR2;
			sSKILLEF.sSpecs[nSpec].dwSPECFLAG = pSKILL->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].dwFLAG;
			sSKILLEF.sSpecs[nSpec].dwNativeID = pSKILL->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].dwNativeID;
			break;
		};
	}

	if ( !bHOLD )
		return FITEMFACT_SIZE;

	sSKILLEF.sNATIVEID	= pSKILL->m_sBASIC.sNATIVEID;
	sSKILLEF.wLEVEL		= wLevel;
	sSKILLEF.fAGE		= sSKILL_DATA.fLIFE;

	m_sFITEMFACT[wSLOT] = sSKILLEF;

	return wSLOT;
}

//check change of map id
void GLChar::CheckChangeMap()
{
	if( !m_pLandMan )	return;

	if ( m_sidLastMapCheck != m_pLandMan->GetMapID() )
	{
		m_sidLastMapCheck = m_pLandMan->GetMapID();

		/*combatpoint logic, Juver, 2017/05/29 */
		if ( m_pLandMan->IsCPReset() )
		{
			m_sSP.dwNow = 0;
			GLMSG::SNETPC_UPDATE_CP NetMsgUpdateCP;
			NetMsgUpdateCP.wNowCP = m_sSP.dwNow;
			NetMsgUpdateCP.bReset = TRUE;
			m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID (), &NetMsgUpdateCP );
		}
	}
}

/*combatpoint logic, Juver, 2017/05/29 */
void GLChar::ReceiveCP ( int nGen, bool bSummonReceive/*=false */)
{
	int nGenCP = nGen;
	if ( bSummonReceive ) nGenCP /= 3; 
	if ( nGenCP > 0 )
	{
		m_sCombatPoint.dwNow += (WORD)nGenCP;
		m_sCombatPoint.LIMIT();

		GLMSG::SNETPC_UPDATE_CP NetMsgUpdateCP;
		NetMsgUpdateCP.wNowCP = m_sCombatPoint.dwNow;
		m_pGLGaeaServer->SENDTOCLIENT ( GETCLIENTID (), &NetMsgUpdateCP );
	}
}

/*vehicle booster system, Juver, 2017/08/10 */
HRESULT GLChar::MsgReqVehicleBooster( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_VEHICLE_BOOSTER* pNetMsg = ( GLMSG::SNETPC_REQ_VEHICLE_BOOSTER* ) nmg;

	if ( !m_bVehicle )					return E_FAIL;
	if ( !m_sVehicle.m_bBooster )		return E_FAIL;
	if ( m_bBoosterStart )				return E_FAIL;
	if ( m_bBoosterCharge )				return E_FAIL;
	
	VehicleBoosterStateStart( true );

	return S_OK;
}
HRESULT GLChar::MsgReqDamageOverlap ( NET_MSG_GENERIC* nmg )
{
	if( !IsValidBody() ) return S_OK;
	if ( m_sCONFTING.IsCONFRONTING() ) return S_OK;
	if( m_wLevel < 1 ) return S_OK;
	GLMSG::SNETPC_REQ_DAMAGE_OVERLAP *pNetMsg = (GLMSG::SNETPC_REQ_DAMAGE_OVERLAP *) nmg;
	DEBUGMSG_WRITE ( _T("[%d][%s], Damage Overlap Client Side, [%d >= %d -- %d]"),
		m_dwCharID, m_szName, pNetMsg->wDamageHistory, pNetMsg->wNow, m_sHP.dwNow );
	if( pNetMsg->wDamageHistory > pNetMsg->wNow ) CDebugSet::ToFileWithTime( "DamageOverlap.txt", "[%d][%s], Damage Overlap Client Side, [%d > %d -- %d]", m_dwCharID, m_szName, pNetMsg->wDamageHistory, pNetMsg->wNow, m_sHP.dwNow );
	if( pNetMsg->wDamageHistory == pNetMsg->wNow )  CDebugSet::ToFileWithTime( "DamageOverlap.txt", "[%d][%s], Client Side 0 HP Detection, [%d == %d -- %d]", m_dwCharID, m_szName, pNetMsg->wDamageHistory, pNetMsg->wNow, m_sHP.dwNow );
	return S_OK;
}
HRESULT GLChar::MsgReqSuicideCmd ( NET_MSG_GENERIC* nmg )
{
	/* dmk14 disable i dont need this */
	return S_OK;

	if( !IsValidBody() ) return S_OK;
	if ( m_sCONFTING.IsCONFRONTING() ) return S_OK;

	/*GLMSG::SNETPC_REQ_SUICIDE_CMD *pNetMsg = (GLMSG::SNETPC_REQ_SUICIDE_CMD *) nmg;
	if( m_sHP.dwNow == m_sHP.dwMax && !pNetMsg->bAutoPots ) return S_OK;
	if(pNetMsg->bAutoPots)
	{
		if( m_wLevel < 200 ) return S_OK;
		GLCHARLOGIC::m_wAutoPotsAttemptCount++;
	}else{
		if( m_wLevel < 300 ) return S_OK;
		DEBUGMSG_WRITE ( _T("[%d][%s], Suicide Command Received"),
			m_dwCharID, m_szName );
		CDebugSet::ToFileWithTime( "SuicideCmd.txt", "[%d][%s], Suicide Command Received", m_dwCharID, m_szName );
		DoFalling();
	}

	if( GLCHARLOGIC::m_wAutoPotsAttemptCount >= 2 && GLCHARLOGIC::m_wAutoPotsAttemptCount <= 5  )
	{
		CString strTextGen;
		GLMSG::SNET_SERVER_GENERALCHAT NetMsgGen;
		//strTextGen.Format( "%s, consider this as your warning. Your action has been logged. Stop using auto potion software now. If you persist, you will be kicked out of the game and won't be able to login for awhile.", m_szName );
		strTextGen.Format( "%s, consider this as your warning. Stop using auto potion software now. If you persist, you will be kicked out of the game.", m_szName );
		NetMsgGen.SETTEXT ( strTextGen.GetString() );
		NetMsgGen.bNotify = true;
		NetMsgGen.iColor = 5;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsgGen );
		strTextGen = "";
	}

	if( GLCHARLOGIC::m_wAutoPotsAttemptCount == 7 )
	{
		DEBUGMSG_WRITE ( _T("[%d][%s], AP Detected"),
			m_dwCharID, m_szName );
		CDebugSet::ToFileWithTime( "DetList.txt", "[%d][%s], AP Count : %d", m_dwCharID, m_szName, GLCHARLOGIC::m_wAutoPotsAttemptCount*27 );

		GLMSG::SNETPC_DROPCHAR_TOAGENT DropMsg;
		DropMsg.dwCHARID  = m_dwCharID;
		DropMsg.bCheat = true;
		DropMsg.nCheatType = 4;
		GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID, &DropMsg);

		//GLITEMLMT::GetInstance().ReqAction(m_dwCharID,EMLOGACT_CHEATER_BAN,ID_CHAR, 2,0,0,0,0);
	}*/

	return S_OK;
}

/*system buffs, Juver, 2017/09/05 */
WORD GLChar::SystemBuffFindSlot ( SNATIVEID skill_id )
{
	WORD wSELECT = SYSTEM_BUFF_SIZE;

	for ( WORD i=0; i<SYSTEM_BUFF_SIZE; ++i )
	{
		//if exist just use the slot
		if ( m_sSYSTEM_BUFF[i].sNATIVEID==skill_id )
			return wSELECT = i;
	}

	for ( WORD i=0; i<SYSTEM_BUFF_SIZE; ++i )
	{
		if ( m_sSYSTEM_BUFF[i].sNATIVEID==SNATIVEID(false) )
			return wSELECT = i;
	}

	return wSELECT;
}

/*system buffs, Juver, 2017/09/05 */
void GLChar::SystemBuffAdd ( SNATIVEID skill_id, WORD wLevel, WORD wSLOT )
{
	if ( wSLOT >= SYSTEM_BUFF_SIZE )	return;

	m_sSYSTEM_BUFF[wSLOT].RESET();

	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( skill_id );
	if ( !pSKILL )						return;

	if ( wLevel >= SKILL::MAX_LEVEL )	return;

	SNATIVEID _sidSKILL = pSKILL->m_sBASIC.sNATIVEID;

	BOOL bHOLD = FALSE;
	SSYSTEM_BUFF ssystem_buff;

	SKILL::CDATA_LVL &sSKILL_DATA = pSKILL->m_sAPPLY.sDATA_LVL[wLevel]; 
	switch ( pSKILL->m_sAPPLY.emBASIC_TYPE )
	{
	case SKILL::EMFOR_VARHP:
	case SKILL::EMFOR_VARMP:
	case SKILL::EMFOR_VARSP:
	case SKILL::EMFOR_DEFENSE:
	case SKILL::EMFOR_HITRATE:
	case SKILL::EMFOR_AVOIDRATE:
	case SKILL::EMFOR_VARAP:
	case SKILL::EMFOR_VARDAMAGE:
	case SKILL::EMFOR_VARDEFENSE:
	case SKILL::EMFOR_PA:
	case SKILL::EMFOR_SA:
	case SKILL::EMFOR_MA:
	case SKILL::EMFOR_RESIST:
	case SKILL::EMFOR_SUMMONTIME: /*summon time, Juver, 2017/12/12 */
		bHOLD = TRUE;
		ssystem_buff.emTYPE = pSKILL->m_sAPPLY.emBASIC_TYPE;
		ssystem_buff.fMVAR = sSKILL_DATA.fBASIC_VAR;
		break;
	};

	for( int nImpact=0; nImpact<SKILL::MAX_IMPACT; ++nImpact )
	{
		if ( pSKILL->m_sAPPLY.sImpacts[nImpact].emADDON != EMIMPACTA_NONE )
		{
			bHOLD = TRUE;
			ssystem_buff.sImpacts[nImpact].emADDON = pSKILL->m_sAPPLY.sImpacts[nImpact].emADDON;
			ssystem_buff.sImpacts[nImpact].fADDON_VAR = pSKILL->m_sAPPLY.sImpacts[nImpact].fADDON_VAR[wLevel];
		}
	}

	for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
	{
		switch ( pSKILL->m_sAPPLY.sSpecs[nSpec].emSPEC )
		{
		case EMSPECA_REFDAMAGE:
		case EMSPECA_NONBLOW:
		case EMSPECA_PIERCE:
		case EMSPECA_TARRANGE:
		case EMSPECA_MOVEVELO:
		case EMSPECA_ATTACKVELO:
		case EMSPECA_SKILLDELAY:
		case EMSPECA_PSY_DAMAGE_REDUCE:
		case EMSPECA_MAGIC_DAMAGE_REDUCE:
		case EMSPECA_PSY_DAMAGE_REFLECTION:
		case EMSPECA_MAGIC_DAMAGE_REFLECTION:
		case EMSPECA_DEFENSE_SKILL_ACTIVE:
		case EMSPECA_CHANGE_ATTACK_RANGE:	/*skill range spec logic, Juver, 2017/06/06 */
		case EMSPECA_CHANGE_APPLY_RANGE:	/*skill range spec logic, Juver, 2017/06/06 */
		case EMSPECA_TALK_TO_NPC:			/*npc talk buff, Juver, 2017/10/03 */
			bHOLD = TRUE;
			ssystem_buff.sSpecs[nSpec].emSPEC = pSKILL->m_sAPPLY.sSpecs[nSpec].emSPEC;
			ssystem_buff.sSpecs[nSpec].fSPECVAR1 = pSKILL->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].fVAR1;
			ssystem_buff.sSpecs[nSpec].fSPECVAR2 = pSKILL->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].fVAR2;
			ssystem_buff.sSpecs[nSpec].dwSPECFLAG = pSKILL->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].dwFLAG;
			ssystem_buff.sSpecs[nSpec].dwNativeID = pSKILL->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].dwNativeID;
			break;
		};
	}

	if ( !bHOLD )	return;

	ssystem_buff.sNATIVEID	= pSKILL->m_sBASIC.sNATIVEID;
	ssystem_buff.wLEVEL		= wLevel;

	m_sSYSTEM_BUFF[wSLOT] = ssystem_buff;

}

/*system buffs, Juver, 2017/09/05 */
void GLChar::SystemBuffReset ( WORD wSLOT )
{
	if ( wSLOT >= SYSTEM_BUFF_SIZE )
	{
		for( int i=0; i<SYSTEM_BUFF_SIZE; ++i )
			m_sSYSTEM_BUFF[i].RESET();

		return;
	}

	m_sSYSTEM_BUFF[wSLOT].RESET();
}

/*pvp tyranny, Juver, 2017/08/24 */
BOOL GLChar::PVPTyrannyRevive( DWORD dwGateID )
{
	if ( !m_pLandMan )	return FALSE;

	/*dmk14 fastrevive fix*/
	if( m_bDeathTimer )
	{
		GLMSG::SNETPC_REQ_REVIVE_DEATHTIMER_FB NetMsgFB;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return FALSE;
	}

	DxLandGateMan* pgateman = &m_pLandMan->GetLandGateMan ();
	if ( !pgateman )	return FALSE;

	DxLandGate* pgate = pgateman->FindLandGate ( dwGateID );
	if ( !pgate )		return FALSE;

	BOOL brebirth = GLGaeaServer::GetInstance().RequestReBirth ( m_dwGaeaID, m_pLandMan->GetMapID(), dwGateID, m_vPos );
	if ( !brebirth )	return FALSE;

	ReSetSTATE(EM_ACT_DIE);

	m_fGenAge = 0.0f;
	GLCHARLOGIC::INIT_RECOVER(100);
	TurnAction ( GLAT_IDLE );	

	GLGaeaServer::GetInstance().ReserveDropOutPet ( SDROPOUTPETINFO(m_dwPetGUID,true,true) );
	//GLGaeaServer::GetInstance().ReserveDropOutSummon ( SDROPOUTSUMMONINFO(m_dwSummonGUID,true) );
	GLGaeaServer::GetInstance().SaveVehicle( m_dwClientID, m_dwGaeaID, true );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		GLGaeaServer::GetInstance().ReserveDropOutSummon ( m_dwSummonGUID_FLD[i] );

	GLMSG::SNETPC_REQ_REBIRTH_FB netmsgagent;
	netmsgagent.sMapID = m_pLandMan->GetMapID();
	netmsgagent.vPos = GetPosition();
	netmsgagent.dwNowHP = m_sHP.dwNow;
	netmsgagent.dwNowMP = m_sMP.dwNow;
	netmsgagent.dwNowSP = m_sSP.dwNow;

	GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID,&netmsgagent);

	GLMSG::SNETPC_REQ_REVIVE_FB netmsgclient;
	netmsgclient.bEventRevive = true;
	netmsgclient.emFB = EMREQ_REVIVE_FB_OK;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgclient);

	GLITEMLMT::GetInstance().ReqAction( m_dwCharID,	EMLOGACT_RESTORE, ID_CHAR, 0, m_nDECEXP, 0, 0, 0 );

	return TRUE;
}

/*skill summon, Juver, 2017/10/09 */
WORD GLChar::GetFreeSummon()
{
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
	{
		if ( m_dwSummonGUID_FLD[i] == GAEAID_NULL )
		{
			return i;
			break;
		}
	}

	return SKILL_SUMMON_MAX_CLIENT_NUM;
}

/*skill summon, Juver, 2017/10/09 */
BOOL GLChar::isMySummon( DWORD dwID )
{
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
	{
		if ( m_dwSummonGUID_FLD[i] != GAEAID_NULL && m_dwSummonGUID_FLD[i] == dwID )
		{
			return TRUE;
			break;
		}
	}

	return FALSE;
}

/*skill summon, Juver, 2017/10/11 */
void GLChar::ReTargetSummon( STARID sid_target )
{
	if ( !m_pLandMan )		return;
	if ( !m_pGLGaeaServer )	return;

	STARGETID starget_id;
	starget_id.dwID = sid_target.GETID();
	starget_id.emCrow = sid_target.GETCROW();

	GLACTOR* pactor_field = m_pGLGaeaServer->GetTarget( m_pLandMan, starget_id );
	if ( !pactor_field )	return;

	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
	{
		PGLSUMMONFIELD psummon_field = m_pGLGaeaServer->GetSummon( m_dwSummonGUID_FLD[i] );
		if ( !psummon_field )					continue;
		if ( !psummon_field->IsValid() )		continue;
		if ( !psummon_field->IsTargetReady() )	continue;
		
		//if ( psummon_field->GetAttackTar().dwID != EMTARGET_NULL )	continue;
		if ( psummon_field->IsSTATE ( EM_SUMMONACT_ATTACK ) )	continue;
		if ( !psummon_field->GetAttackTarget( starget_id ) )	continue;

		psummon_field->SetAttackTarget( starget_id );
	}
}
BOOL GLChar::ReceivePKCombo ( int nCount )
{
	if ( m_bVehicle ) return FALSE;

	RECEIVE_PKCOMBO ( nCount );

	GLMSG::SNETPC_PKCOMBO_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	NetMsgBrd.sCOMBO = m_sPKCOMBOCOUNT;
	m_pGLGaeaServer->SENDTOCLIENT(m_dwClientID, &NetMsgBrd);
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

	return TRUE;
}
/*charinfoview , Juver, 2017/11/11 */
HRESULT GLChar::MsgReqCharInfo ( NET_MSG_GENERIC* nmg )
{
	if ( !RANPARAM::bFeatureViewCharInfo )	return S_OK;

	GLMSG::SNETPC_REQ_CHARINFO *pNetMsg = reinterpret_cast<GLMSG::SNETPC_REQ_CHARINFO*> ( nmg );

	PGLCHAR pChar = GLGaeaServer::GetInstance().GetChar( pNetMsg->dwGaeaID );
	if ( !pChar )
	{
		GLMSG::SNETPC_REQ_CHARINFO_FB NetMsgFB;
		NetMsgFB.emFB = EMREQ_CHARINFO_FB_INVALID_TARGET;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	/*hide gm info, Juver, 2017/11/26 */
	if ( RANPARAM::bFeatureHideGMInfo && pChar->m_dwUserLvl >= USER_GM3 && m_dwUserLvl < USER_GM3 )
	{
		GLMSG::SNETPC_REQ_CHARINFO_FB NetMsgFB;
		NetMsgFB.emFB = EMREQ_CHARINFO_FB_INVALID_TARGET;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if  ( m_fReqCharInfoTimer < REQ_CHARINFO_DELAY )
	{
		GLMSG::SNETPC_REQ_CHARINFO_FB NetMsgFB;
		NetMsgFB.emFB = EMREQ_CHARINFO_FB_TIMEDELAY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	{	//send charinfo
		SINFO_CHAR	sCharInfo;
		sCharInfo.dwGaeaID		= pNetMsg->dwGaeaID;
		sCharInfo.dwCharID		= pChar->m_dwCharID;
		StringCchCopy ( sCharInfo.szName, CHAR_SZNAME, pChar->m_szName );
		sCharInfo.emClass		= pChar->m_emClass;
		sCharInfo.wSchool		= pChar->m_wSchool;
		sCharInfo.wHair			= pChar->m_wHair;
		sCharInfo.wHairColor	= pChar->m_wHairColor;
		sCharInfo.wFace			= pChar->m_wFace;
		sCharInfo.wLevel		= pChar->m_wLevel;
		sCharInfo.sdDAMAGE.dwLow		= pChar->GETFORCE_LOW();
		sCharInfo.sdDAMAGE.dwHigh	= pChar->GETFORCE_HIGH();
		sCharInfo.wHP			= pChar->m_sHP.dwMax;
		sCharInfo.wMP			= pChar->m_sMP.dwMax;
		sCharInfo.wSP			= pChar->m_sSP.dwMax;
		sCharInfo.sSUMSTATS		= pChar->m_sSUMSTATS;
		sCharInfo.wSUM_PA		= pChar->m_wSUM_PA;
		sCharInfo.wSUM_SA		= pChar->m_wSUM_SA;
		sCharInfo.wSUM_MA		= pChar->m_wSUM_MA;
		sCharInfo.sSUMRESIST	= pChar->m_sSUMRESIST_SKILL;
		sCharInfo.nSUM_DEF		= pChar->GETDEFENSE();
		sCharInfo.nSUM_HIT		= pChar->GETHIT();
		sCharInfo.nSUM_AVOID	= pChar->GETAVOID();
		sCharInfo.bUseArmSub	= pChar->IsUseArmSub();
		sCharInfo.fScaleRange	= pChar->m_fScaleRange;
		StringCchCopy ( sCharInfo.szBadge, CHAR_SZNAME, pChar->m_szBadge );

		if ( m_dwUserLvl < USER_GM3 )
		{
			sCharInfo.bPrivate	= pChar->m_bPrivateStats;

			if ( sCharInfo.bPrivate )
				sCharInfo.sSUMSTATS.RESET();
		}	

		for ( int i=0; i<SLOT_NSIZE_S_2; i++ )		
			sCharInfo.sPutOnItems[i] = pChar->m_PutOnItems[i];

		if ( pChar->m_dwGuild!=CLUB_NULL )
		{
			GLClubMan &cClubMan = m_pGLGaeaServer->GetClubMan();
			GLCLUB *pCLUB = cClubMan.GetClub ( pChar->m_dwGuild );
			if ( pCLUB )
				StringCchCopy ( sCharInfo.szClubName, CHAR_SZNAME, pCLUB->m_szName );
		}

		GLMSG::SNETPC_REQ_CHARINFO_FB NetMsgFB;
		NetMsgFB.emFB = EMREQ_CHARINFO_FB_WEARINFO;
		NetMsgFB.sCharInfo = sCharInfo;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
	}

	m_fReqCharInfoTimer = 0.0f;

	return S_OK;
}

/*pk info, Juver, 2017/11/17 */
void GLChar::PKIncreaseScore( DWORD dwCharID )
{
	PGLCHAR pchar_target = GLGaeaServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar_target )	return;

	DoPkScore();

	STOP_RANK_KILL sPKWIN;
	sPKWIN.dwChaNum		= m_dwCharID;
	StringCchCopy(sPKWIN.szChaName, CHAR_SZNAME, m_szName);
	sPKWIN.wChaLevel	= m_wLevel;
	sPKWIN.nChaClass	= (int)m_emClass;
	sPKWIN.wChaSchool	= m_wSchool;
	{
		BOOL bGuild = m_dwGuild ? TRUE : FALSE;
		if (bGuild)
		{
			sPKWIN.wGuNum		= (WORD)m_dwGuild;
			GLClubMan& cClubMan	= GLGaeaServer::GetInstance().GetClubMan();
			GLCLUB* pCLUB		= cClubMan.GetClub(m_dwGuild);
			if (pCLUB)
			{
				sPKWIN.wGuMarkVer = (WORD)pCLUB->m_dwMarkVER;
				StringCchCopy(sPKWIN.szGuName, CHAR_SZNAME, pCLUB->m_szName);
			}
		}
	}
	sPKWIN.dwChaPKWin	= m_dwPKScore;
	sPKWIN.dwChaPKLoss	= m_dwPKDeath;
	GLGaeaServer::GetInstance().ReqGlobalRankingKillUpdate(sPKWIN, TRUE);


	CString PKStreak[14] =
	{
		"",
		"",
		"",
		"ULTRAKILL",
		"RAMPAGE",
		"KILLING SPREE",
		"DOMINATING",
		"MEGAKILL",
		"UNSTOPPABLE",
		"WICKED SICK",
		"MONSTER KILL",
		"GODLIKE",
		"HOLY SHIT",
		"OWNAGE"
	};

	if ( m_sPKCOMBOCOUNT.IsACTIVE() &&  m_sPKCOMBOCOUNT.nCount > 3 )
	{
		GLMSG::SNET_SERVER_GENERALCHAT NetMsgLvlUP;
		CString strTEXT;
		int nIndex = m_sPKCOMBOCOUNT.nCount;
		if ( m_sPKCOMBOCOUNT.nCount >= 14 ) nIndex = 13;
		strTEXT.Format("'%s' is now %s",m_szName,PKStreak[nIndex].GetString());
		NetMsgLvlUP.SETTEXT ( strTEXT.GetString() );
		NetMsgLvlUP.bNotify = true;
		NetMsgLvlUP.iColor = 0;
		GLGaeaServer::GetInstance().SENDTOAGENT ( &NetMsgLvlUP );
	}

	GLMSG::SNETPC_UPDATE_PK_SCORE net_msg_client;
	net_msg_client.dwVal = m_dwPKScore;
	StringCchCopy( net_msg_client.szName, CHAR_SZNAME+1, pchar_target->m_szName );
	GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_client );
}

/*pk info, Juver, 2017/11/17 */
void GLChar::PKIncreaseDeath( DWORD dwCharID )
{
	PGLCHAR pchar_target = GLGaeaServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar_target )	return;

	DoPkDeath();

	/*LG-7 Ranking*/
	STOP_RANK_KILL sPKLOSS;
	sPKLOSS.dwChaNum	= m_dwCharID;
	StringCchCopy(sPKLOSS.szChaName, CHAR_SZNAME, m_szName);
	sPKLOSS.wChaLevel	= m_wLevel;
	sPKLOSS.nChaClass	= (int)m_emClass;
	sPKLOSS.wChaSchool	= m_wSchool;
	{
		BOOL bGuild = m_dwGuild ? TRUE : FALSE;
		if (bGuild)
		{
			sPKLOSS.wGuNum		= (WORD)m_dwGuild;
			GLClubMan& cClubMan	= GLGaeaServer::GetInstance().GetClubMan();
			GLCLUB* pCLUB		= cClubMan.GetClub(m_dwGuild);
			if (pCLUB)
			{
				sPKLOSS.wGuMarkVer = (WORD)pCLUB->m_dwMarkVER;
				StringCchCopy(sPKLOSS.szGuName, CHAR_SZNAME, pCLUB->m_szName);
			}
		}
	}
	sPKLOSS.dwChaPKWin	= m_dwPKScore;
	sPKLOSS.dwChaPKLoss = m_dwPKDeath;
	GLGaeaServer::GetInstance().ReqGlobalRankingKillUpdate(sPKLOSS, FALSE);

	GLMSG::SNETPC_UPDATE_PK_DEATH net_msg_client;
	net_msg_client.dwVal = m_dwPKDeath;
	StringCchCopy( net_msg_client.szName, CHAR_SZNAME+1, pchar_target->m_szName );
	GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_client );
}

/*block back damage, Juver, 2017/12/12 */
void GLChar::SetBlockDamageBack( bool bSet )
{
	m_bBlockBackDamage = bSet;
	m_fBlockBackDamageTime = 0.0f;
}

/*skill summon, Juver, 2017/10/10 */
void GLChar::RemoveSummonBuff( DWORD dwSummonID )
{
	GLMSG::SNETPC_SKILLHOLD_RS_BRD NetMsgSkillBrd;
	BOOL bValidErase = FALSE;
	for ( int i=0; i<SKILLFACT_SIZE; ++i )
	{
		if ( m_sSKILLFACT[i].sNATIVEID == NATIVEID_NULL() ) continue;

		if ( m_sSKILLFACT[i]._wCasterCrow != CROW_SUMMON )	continue;
		if ( m_sSKILLFACT[i]._dwCasterID != dwSummonID )	continue;

		DISABLESKEFF( i );
		NetMsgSkillBrd.bRESET[i] = true;
		bValidErase = TRUE;
	}

	if ( bValidErase )
	{
		NetMsgSkillBrd.dwID = m_dwGaeaID;
		NetMsgSkillBrd.emCrow = CROW_PC;

		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgSkillBrd );
		m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsgSkillBrd );
	}
}

/*clubwar verify non push or pull, Juver, 2017/12/13 */
void GLChar::SetClubWarVerify( BOOL bSET )
{
	if ( RANPARAM::bFeatureClubWarVerifyDisablePushPull )
	{
		m_bClubWarVerify = bSET;
		m_fClubWarVerifyTimer = 0.0f;
	}
}

/*school wars, Juver, 2018/01/20 */
BOOL GLChar::PVPSchoolWarsRevive( DWORD dwGateID )
{
	if ( !m_pLandMan )	return FALSE;

	DxLandGateMan* pgateman = &m_pLandMan->GetLandGateMan ();
	if ( !pgateman )	return FALSE;

	DxLandGate* pgate = pgateman->FindLandGate ( dwGateID );
	if ( !pgate )		return FALSE;

	BOOL brebirth = GLGaeaServer::GetInstance().RequestReBirth ( m_dwGaeaID, m_pLandMan->GetMapID(), dwGateID, m_vPos );
	if ( !brebirth )	return FALSE;

	ReSetSTATE(EM_ACT_DIE);

	m_fGenAge = 0.0f;
	GLCHARLOGIC::INIT_RECOVER(100);
	TurnAction ( GLAT_IDLE );	

	GLGaeaServer::GetInstance().ReserveDropOutPet ( SDROPOUTPETINFO(m_dwPetGUID,true,true) );
	//GLGaeaServer::GetInstance().ReserveDropOutSummon ( SDROPOUTSUMMONINFO(m_dwSummonGUID,true) );
	GLGaeaServer::GetInstance().SaveVehicle( m_dwClientID, m_dwGaeaID, true );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		GLGaeaServer::GetInstance().ReserveDropOutSummon ( m_dwSummonGUID_FLD[i] );

	GLMSG::SNETPC_REQ_REBIRTH_FB netmsgagent;
	netmsgagent.sMapID = m_pLandMan->GetMapID();
	netmsgagent.vPos = GetPosition();
	netmsgagent.dwNowHP = m_sHP.dwNow;
	netmsgagent.dwNowMP = m_sMP.dwNow;
	netmsgagent.dwNowSP = m_sSP.dwNow;

	GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID,&netmsgagent);

	GLMSG::SNETPC_REQ_REVIVE_FB netmsgclient;
	netmsgclient.bEventRevive = true;
	netmsgclient.emFB = EMREQ_REVIVE_FB_OK;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgclient);

	GLITEMLMT::GetInstance().ReqAction( m_dwCharID,	EMLOGACT_RESTORE, ID_CHAR, 0, m_nDECEXP, 0, 0, 0 );

	return TRUE;
}

/*pvp capture the flag, Juver, 2018/02/01 */
BOOL GLChar::PVPCaptureTheFlagRevive( DWORD dwGateID )
{
	if ( !m_pLandMan )	return FALSE;

	DxLandGateMan* pgateman = &m_pLandMan->GetLandGateMan ();
	if ( !pgateman )	return FALSE;

	DxLandGate* pgate = pgateman->FindLandGate ( dwGateID );
	if ( !pgate )		return FALSE;

	BOOL brebirth = GLGaeaServer::GetInstance().RequestReBirth ( m_dwGaeaID, m_pLandMan->GetMapID(), dwGateID, m_vPos );
	if ( !brebirth )	return FALSE;

	ReSetSTATE(EM_ACT_DIE);

	m_fGenAge = 0.0f;
	GLCHARLOGIC::INIT_RECOVER(100);
	TurnAction ( GLAT_IDLE );	

	GLGaeaServer::GetInstance().ReserveDropOutPet ( SDROPOUTPETINFO(m_dwPetGUID,true,true) );
	//GLGaeaServer::GetInstance().ReserveDropOutSummon ( SDROPOUTSUMMONINFO(m_dwSummonGUID,true) );
	GLGaeaServer::GetInstance().SaveVehicle( m_dwClientID, m_dwGaeaID, true );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		GLGaeaServer::GetInstance().ReserveDropOutSummon ( m_dwSummonGUID_FLD[i] );

	GLMSG::SNETPC_REQ_REBIRTH_FB netmsgagent;
	netmsgagent.sMapID = m_pLandMan->GetMapID();
	netmsgagent.vPos = GetPosition();
	netmsgagent.dwNowHP = m_sHP.dwNow;
	netmsgagent.dwNowMP = m_sMP.dwNow;
	netmsgagent.dwNowSP = m_sSP.dwNow;

	GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID,&netmsgagent);

	GLMSG::SNETPC_REQ_REVIVE_FB netmsgclient;
	netmsgclient.bEventRevive = true;
	netmsgclient.emFB = EMREQ_REVIVE_FB_OK;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgclient);

	GLITEMLMT::GetInstance().ReqAction( m_dwCharID,	EMLOGACT_RESTORE, ID_CHAR, 0, m_nDECEXP, 0, 0, 0 );

	return TRUE;
}

/*pvp capture the flag, Juver, 2018/02/06 */
void GLChar::PVPCaptureTheFlagDetectGate()
{
	if ( !m_pLandMan )					return;
	if ( !m_bCaptureTheFlagHoldFlag )	return;
	if ( !m_pLandMan->m_bPVPCaptureTheFlagMap )	return;

	DxLandGateMan *plandgateman = &m_pLandMan->GetLandGateMan();
	if ( !plandgateman )		return;

	PDXLANDGATE pdetectedgate = plandgateman->DetectGate ( GetPosition() );
	if ( !pdetectedgate )		return;

	GLPVPCaptureTheFlagField::GetInstance().FlagToBase( m_dwCharID, pdetectedgate->GetGateID() ); 
}

/*pvp capture the flag, Juver, 2018/02/07 */
void GLChar::PVPCaptureTheFlagHoldFlag( bool bHold )
{
	m_bCaptureTheFlagHoldFlag = bHold;

	m_fCaptureTheFlagHoldFlagTimer = 0.0f;
	m_fCaptureTheFlagDetectGateTimer = 0.0f;

	GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_PLAYER_FLAG_HOLD netmsgbrd;
	netmsgbrd.dwGaeaID = m_dwGaeaID;
	netmsgbrd.bHold = m_bCaptureTheFlagHoldFlag;
	GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &netmsgbrd);
	SendMsgViewAround ( (NET_MSG_GENERIC*) &netmsgbrd );

	if ( m_bCaptureTheFlagHoldFlag )
	{
		//remove buffs
		GLMSG::SNETPC_SKILLHOLD_RS_BRD NetMsgSkillBrd;
		BOOL bUpdateBuffs = FALSE;
		for ( int i=0; i<SKILLFACT_SIZE; ++i )
		{
			if ( m_sSKILLFACT[i].sNATIVEID == NATIVEID_NULL() ) continue;
			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_sSKILLFACT[i].sNATIVEID );
			if ( !pSkill ) continue;
			DISABLESKEFF( i );
			NetMsgSkillBrd.bRESET[i] = true;
			bUpdateBuffs = TRUE;
		}

		if ( bUpdateBuffs )
		{
			NetMsgSkillBrd.dwID = m_dwGaeaID;
			NetMsgSkillBrd.emCrow = CROW_PC;

			SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgSkillBrd );
			m_pGLGaeaServer->SENDTOCLIENT ( m_dwClientID, &NetMsgSkillBrd );
		}
	}
}
/*hackshield implementation, Juver, 2018/06/21 */
void GLChar::hs_client_disconnect()
{
	if( GLGaeaServer::GetInstance().IsEmulatorMode() )	return;

	hs_disconnect_num ++;
	CDebugSet::ToFileWithTime( "hsheartbeat.txt", "disconnect char %s heartbeat time limit dc:%I64d hb:%I64d", m_szName, hs_disconnect_num, hs_heartbeat_num );


	GLMSG::SNETPC_HS_CLOSE_CLIENT netmsg;
	GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &netmsg);
}

void GLChar::StigmaProc_SendMessage(const STARGETID& sTargetID, const float fRate, const bool bBroad)
{	
	GLLandMan* const pLand( GetLandMan() );
	if ( pLand == NULL )
		return;

	int nDAMAGE = 0;
	if ( fRate < 0.0f )
	{
		/*const DWORD dwHealFlag = CALCHEAL( nDAMAGE, m_dwGaeaID, sTargetID, pLand);
		const float fResultHeal = -fRate * float(nDAMAGE);
		Heal_SendMessage(DWORD(fResultHeal), dwHealFlag, sTargetID, bBroad);*/
	}
	else
	{
		const DWORD dwDamageFlag = CALCDAMAGE(nDAMAGE, m_dwGaeaID, sTargetID, pLand);
		const float fResultDamage = fRate * float(nDAMAGE);
		Damage_SendMessage(DWORD(fResultDamage), dwDamageFlag, sTargetID, bBroad);
	}
}


void GLChar::StigmaProc_Normal(const bool bBroad)
{	
	for ( DWORD _i = SKILLFACT_SIZE; _i; --_i )
	{
		SSTIGMA::SSTIGMA_PACK& sStigmaPack = m_sSTIGMA_USER.sSTIGMA_PACK[_i - 1];		
		if ( !sStigmaPack.bUse )
			continue;

		if ( !(sStigmaPack.dwFLAG & SKILL::EMSPEC_STIGMA_TYPE_DWORD_DAMAGE_NORMAL) )
			continue;
		GLACTOR* pACTOR = GLGaeaServer::GetInstance().GetTarget(GetLandMan(),sStigmaPack.sTakerID);
		if ( !pACTOR )
		{
			m_sSTIGMA_USER.DeleteStigmaPack(_i - 1);
			continue;
		}

		if ( !pACTOR->IsValidBody() )
			m_sSTIGMA_USER.DeleteStigmaPack(_i - 1);

		switch ( pACTOR->GetCrow() )
		{
		case CROW_PC:
			if ( ((GLChar*)pACTOR)->m_sSTIGMA_TAKER.IsExistStigmaPack(STARGETID(CROW_PC, m_dwGaeaID), sStigmaPack.sSkillID) == SKILLFACT_SIZE )
			{
				m_sSTIGMA_USER.DeleteStigmaPack(_i - 1);
				continue;
			}
			break;
		case CROW_NPC:					
		case CROW_MOB:
			if ( ((GLCrow*)pACTOR)->m_sSTIGMA_TAKER.IsExistStigmaPack(STARGETID(CROW_PC, m_dwGaeaID), sStigmaPack.sSkillID) == SKILLFACT_SIZE )
			{
				m_sSTIGMA_USER.DeleteStigmaPack(_i - 1);
				continue;
			}
			break;
		case CROW_SUMMON:
			if ( ((GLSummonField*)pACTOR)->m_sSTIGMA_TAKER.IsExistStigmaPack(STARGETID(CROW_PC, m_dwGaeaID), sStigmaPack.sSkillID) == SKILLFACT_SIZE )
			{
				m_sSTIGMA_USER.DeleteStigmaPack(_i - 1);
				continue;
			}
			break;
		}

		//if ( sStigmaPack.sTakerID != m_TargetID )
			StigmaProc_SendMessage(sStigmaPack.sTakerID, sStigmaPack.fRate, bBroad);
	}	
}

void GLChar::StigmaProc_Skill(const SSKILLACT* pSkillAct, BOOL bStigmaEffect)
{	
	GLLandMan* const pLand( GetLandMan() );
	if ( pLand == NULL )
		return;

	for ( DWORD _i = SKILLFACT_SIZE; _i; --_i )
	{
		SSTIGMA::SSTIGMA_PACK& sStigmaPack = m_sSTIGMA_USER.sSTIGMA_PACK[_i - 1];		
		if ( !sStigmaPack.bUse )
			continue;

		if ( !(sStigmaPack.dwFLAG & SKILL::EMSPEC_STIGMA_TYPE_DWORD_DAMAGE_SKILL) )
			continue;

		GLACTOR* pACTOR = GLGaeaServer::GetInstance().GetTarget(GetLandMan(),sStigmaPack.sTakerID);
		if ( !pACTOR )
		{
			m_sSTIGMA_USER.DeleteStigmaPack(_i - 1);
			continue;
		}
		
		if ( !pACTOR->IsValidBody() )
			m_sSTIGMA_USER.DeleteStigmaPack(_i - 1);

		switch ( pACTOR->GetCrow() )
		{
		case CROW_PC:
			if ( ((GLChar*)pACTOR)->m_sSTIGMA_TAKER.IsExistStigmaPack(STARGETID(CROW_PC, m_dwGaeaID), sStigmaPack.sSkillID) == SKILLFACT_SIZE )
			{
				m_sSTIGMA_USER.DeleteStigmaPack(_i - 1);	
				continue;
			}
			break;
		case CROW_NPC:					
		case CROW_MOB:
			if ( ((GLCrow*)pACTOR)->m_sSTIGMA_TAKER.IsExistStigmaPack(STARGETID(CROW_PC, m_dwGaeaID), sStigmaPack.sSkillID) == SKILLFACT_SIZE )
			{
				m_sSTIGMA_USER.DeleteStigmaPack(_i - 1);
				continue;
			}
			break;
		case CROW_SUMMON:
			if ( ((GLSummonField*)pACTOR)->m_sSTIGMA_TAKER.IsExistStigmaPack(STARGETID(CROW_PC, m_dwGaeaID), sStigmaPack.sSkillID) == SKILLFACT_SIZE )
			{
				m_sSTIGMA_USER.DeleteStigmaPack(_i - 1);
				continue;
			}
			break;
		}

		//if ( sStigmaPack.sTakerID != m_TargetID )
		if( !bStigmaEffect || sStigmaPack.sTakerID != m_TargetID)
		{					
			SSKILLACT sSKILLACT_STIGMA;
			sSKILLACT_STIGMA.sID			= sStigmaPack.sUserID;
			sSKILLACT_STIGMA.sID_TAR	= sStigmaPack.sTakerID;
			sSKILLACT_STIGMA.fDELAY			= pSkillAct->fDELAY;
			sSKILLACT_STIGMA.emAPPLY		= pSkillAct->emAPPLY;
			//sSKILLACT_STIGMA.sSkillID		= pSkillAct->sSkillID;
			//sSKILLACT_STIGMA.wSkillLevel	= pSkillAct->wSkillLevel;
			sSKILLACT_STIGMA.dwDamageFlag	= pSkillAct->dwDamageFlag | DAMAGE_TYPE_NONE;	//DAMAGE_TYPE_STIGMA		
			sSKILLACT_STIGMA.nVAR_HP 		= int(float(pSkillAct->nVAR_HP) * sStigmaPack.fRate);
			sSKILLACT_STIGMA.nVAR_MP		= int(float(pSkillAct->nVAR_MP) * sStigmaPack.fRate);
			sSKILLACT_STIGMA.nVAR_SP 		= int(float(pSkillAct->nVAR_SP) * sStigmaPack.fRate);
			sSKILLACT_STIGMA.nGATHER_HP 	= int(float(pSkillAct->nGATHER_HP) * sStigmaPack.fRate);
			sSKILLACT_STIGMA.nGATHER_MP 	= int(float(pSkillAct->nGATHER_MP) * sStigmaPack.fRate);
			sSKILLACT_STIGMA.nGATHER_SP 	= int(float(pSkillAct->nGATHER_SP) * sStigmaPack.fRate);			
			pLand->RegSkillAct( sSKILLACT_STIGMA );
		}			
	}	
}

void GLChar::Damage_SendMessage(const int nDAMAGE, const DWORD dwDamageFlag, const STARGETID& sTargetID, const bool bBroad)
{
	// ����ڿ��� ������� ����.
	//
	const int nActualDamage = (int)ToDamage( sTargetID, nDAMAGE, dwDamageFlag );

	// �ڽſ���.
	GLMSG::SNETPC_ATTACK_DAMAGE NetMsg;
	NetMsg.emTarCrow		= sTargetID.emCrow;
	NetMsg.dwTarID			= sTargetID.dwID;
	NetMsg.nDamage			= nActualDamage;
	NetMsg.dwDamageFlag	= dwDamageFlag | DAMAGE_TYPE_NONE; //DAMAGE_TYPE_STIGMA
	//NetMsg.wSchool          = GetSchool();
	SendToClient( (NET_MSG_GENERIC*) &NetMsg );

	if ( bBroad )
	{
		// �ֺ� Ŭ���̾�Ʈ�鿡�� �޼��� ����.
		//
		GLMSG::SNETPC_ATTACK_DAMAGE_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID		= m_dwGaeaID;
		NetMsgBrd.emTarCrow		= sTargetID.emCrow;
		NetMsgBrd.dwTarID			= sTargetID.dwID;
		NetMsgBrd.nDamage			= nActualDamage;
		NetMsgBrd.dwDamageFlag	= dwDamageFlag | DAMAGE_TYPE_NONE; //DAMAGE_TYPE_STIGMA
		//NetMsgBrd.wSchool			= GetSchool();
		SendMsgViewAround((NET_MSG_GENERIC*)&NetMsgBrd);
	}	
}


void GLChar::ApplySkillTargetStigma(const STARGETID& sTARID, const SKILL::SSPEC& sSPEC, const PGLSKILL pSkill)
{
	STARGETID sSelfID(CROW_PC, m_dwGaeaID);	
	GLACTOR* pACTOR = GLGaeaServer::GetInstance().GetTarget(GetLandMan(),sTARID);
	if ( pACTOR )
	{
        if( !pACTOR->IsValidBody() )
            return;

		switch ( pACTOR->GetCrow() )
		{ // ���� ��� ���� �̸� �ѹ� ��� ���ִ� ������ ApplySkill ��ü ���ּ� ���� �˻縦 ��;
		case CROW_MOB:
			{
				GLCrow* const pCrow = (GLCrow*)pACTOR;
				/*if ( pCrow->IsNoAllPenalty() )
					break;*/

				m_sSTIGMA_USER.PushStigmaPack(sSelfID, sTARID, pSkill->m_sBASIC.sNATIVEID, sSPEC);
				pCrow->m_sSTIGMA_TAKER.PushStigmaPack(sSelfID, sTARID, pSkill->m_sBASIC.sNATIVEID, sSPEC);
			}
			break;
		//disabled for now
		//case CROW_SUMMON:
		//	{
		//		GLSummonField* const pSummon = (GLSummonField*)pACTOR;
		//		/*if ( pSummon->IsNoAllPenalty() )
		//			break;*/

		//		m_sSTIGMA_USER.PushStigmaPack(sSelfID, sTARID, pSkill->m_sBASIC.sNATIVEID, sSPEC);
		//		pSummon->m_sSTIGMA_TAKER.PushStigmaPack(sSelfID, sTARID, pSkill->m_sBASIC.sNATIVEID, sSPEC);
		//	}
		//	break;
		case CROW_PC:
			m_sSTIGMA_USER.PushStigmaPack(sSelfID, sTARID, pSkill->m_sBASIC.sNATIVEID, sSPEC);
			((GLChar*)pACTOR)->m_sSTIGMA_TAKER.PushStigmaPack(sSelfID, sTARID, pSkill->m_sBASIC.sNATIVEID, sSPEC);
			break;
		}
	}
}


void GLChar::ApplySkillTargetSkillLink(const STARGETID& sSelectTARID, const STARGETID& sTARGETID, const SKILL::SSPEC& sSPEC, const SCHARSKILL& sSkill)
{
	GLLandMan* const pLand( GetLandMan() );
	if ( pLand == NULL )
		return;

	GLACTOR* pACTOR = GLGaeaServer::GetInstance().GetTarget(GetLandMan(),sTARGETID);
	if( !pACTOR )
		return;

	if( !pACTOR->IsValidBody() )
		return;

	switch ( pACTOR->GetCrow() )
	{
	case CROW_MOB:
		{
			GLCrow* const pCrow = (GLCrow*)pACTOR;
			/*if ( pCrow->IsNoAllPenalty() )
				return;*/

			if ( !pCrow )
				return;

			if( pCrow->ISHAVE_BUFF( sSPEC.dwLinkID ) == false )
				return;
		}
		break;
	case CROW_PC:
		{
			GLChar* const pTarget = (GLChar*)pACTOR;
			if ( !pTarget )
				return;

			if( pTarget->ISHAVE_BUFF( sSPEC.dwLinkID ) == false )
				return;
		}
		break;
	default:
		return;
	}

	if( RANDOM_POS > sSPEC.fVAR1 )
		return;

	PGLSKILL			pLinkSkill	= GLSkillMan::GetInstance().GetData( sSPEC.dwNativeID );
	EMIMPACT_TAR		emTarget	= pLinkSkill->m_sBASIC.emIMPACT_TAR;
	SKILL::SAPPLY&		sSAPPLY		= pLinkSkill->m_sAPPLY;
	SKILL::CDATA_LVL&	sSKILL_DATA	= sSAPPLY.sDATA_LVL[sSkill.wLevel];
	SSKILLACT			sSKILLACT;
	SSKILLACTEX			sSKILLACTEX;
	STARID				sTARID;	// �߰� �ߵ���ų Ÿ��;

	// �ڱ� �ڽ� �Ǵ� Ÿ�� �̿ܿ��� ������� �ʴ´�;
	if( emTarget == TAR_SELF )
	{
		sTARID.SET( STARGETID( CROW_PC, m_dwGaeaID ) );
		pACTOR = GLGaeaServer::GetInstance().GetTarget(GetLandMan(),STARGETID((EMCROW)sTARID.wCrow,sTARID.wID));
	}
	else
	{
		sTARID.SET( sTARGETID );
	}

	int		nGATHER[NUM_HP_MP_SP]		= { 0 };
	int		nVarsArray[NUM_HP_MP_SP]	= { 0 };
	BOOL	bAsignStigma = false;
	BOOL	bCrushBlowOne = false;
	DWORD	dwDamageFlag = DAMAGE_TYPE_NONE;
	nVarsArray[IDX_HP] = 0;
	nVarsArray[IDX_MP] = 0;
	nVarsArray[IDX_SP] = 0;

	BOOL	bStigmaEff = FALSE;

	if ( pACTOR->GetNowHP() != 0 )
	{
		switch( sSAPPLY.emBASIC_TYPE )
		{
		case SKILL::EMFOR_HP:
			ApplySkillTarget_HP( pLinkSkill, sSkill.wLevel, sSKILL_DATA, pACTOR, 1, STARGETID((EMCROW)sTARID.wCrow,sTARID.wID), nVarsArray[IDX_HP], dwDamageFlag );
			break;

		case SKILL::EMFOR_MP:
			ApplySkillTarget_MP( pLinkSkill, sSKILL_DATA, pACTOR, nVarsArray[IDX_MP] );
			break;

		case SKILL::EMFOR_SP:
			ApplySkillTarget_SP( pLinkSkill, sSKILL_DATA, pACTOR, nVarsArray[IDX_SP] );
			break;
		}

		//if ( CalcBlocking(pACTOR, nVarsArray[IDX_HP], pLinkSkill, sSkill.wLevel) == true ) // Note : ����ŷ �˻�
		//{
		//	BlockProc(sTARID, sSPEC.dwNativeID, sSkill.wLevel);
		//	return;
		//}
		//else
		{
			if( dwDamageFlag & DAMAGE_TYPE_CRUSHING_BLOW )
				bCrushBlowOne = true;

			ApplySkillTarget_Spec( pLinkSkill, sSkill.wLevel, pACTOR, dwDamageFlag, nGATHER, nVarsArray );

			// SSKILLACT
			sSKILLACT.sID			= STARGETID(CROW_PC,m_dwGaeaID);
			sSKILLACT.sID_TAR		= STARGETID(sTARGETID.emCrow,sTARGETID.dwID);
			sSKILLACT.fDELAY		= pLinkSkill->m_sEXT_DATA.fDELAY4DAMAGE;
			sSKILLACT.emAPPLY		= pLinkSkill->m_sBASIC.emAPPLY;
			//sSKILLACT.sSkillID		= sSPEC.dwNativeID;
			//sSKILLACT.wSkillLevel	= sSkill.wLevel;
			sSKILLACT.dwDamageFlag	= dwDamageFlag;
			sSKILLACT.nVAR_HP 		= nVarsArray[IDX_HP];
			sSKILLACT.nVAR_MP 		= nVarsArray[IDX_MP];
			sSKILLACT.nVAR_SP 		= nVarsArray[IDX_SP];
			sSKILLACT.nGATHER_HP 	= nGATHER[IDX_HP];
			sSKILLACT.nGATHER_MP 	= nGATHER[IDX_MP];
			sSKILLACT.nGATHER_SP 	= nGATHER[IDX_SP];

			if( sSKILLACT.VALID() )
			{
				pLand->RegSkillAct( sSKILLACT );
				bAsignStigma = true;
			}
		}


		// SSKILLACTEX
		sSKILLACTEX.sID			= STARGETID(CROW_PC,m_dwGaeaID);
		sSKILLACTEX.fDELAY		= pLinkSkill->m_sEXT_DATA.fDELAY4DAMAGE;
		sSKILLACTEX.idSKILL		= sSPEC.dwNativeID;
		sSKILLACTEX.wSKILL_LVL	= sSkill.wLevel;
		sSKILLACTEX.sID_TAR		= STARGETID((EMCROW)sTARID.wCrow,sTARID.wID);

		if ( bCrushBlowOne )
		{
			sSKILLACTEX.fPUSH_PULL	= GLCONST_CHAR::fCRUSH_BLOW_RANGE;
			//sSKILLACTEX.fPUSH_SPEED = GLCONST_CHAR::fPUSHPULL_VELO;
		}

		if( sSAPPLY.emBASIC_TYPE == SKILL::EMFOR_CURE )
			sSKILLACTEX.dwCUREFLAG |= sSAPPLY.dwCUREFLAG;

		if ( sSKILLACTEX.VALID() )
		{
			pLand->RegSkillActEx ( sSKILLACTEX );
		}

		/*if( pLinkSkill->m_sSPECIAL_SKILL.emSSTYPE == SKILL::EMSSTYPE_SUMMONS )
			ApplySkillSummons( pLinkSkill, sSkill, sTARID, pACTOR );*/

		/*skill summon, Juver, 2017/10/09 */
		if ( m_pGLGaeaServer && pLinkSkill->m_sSPECIAL_SKILL.emSSTYPE == SKILL::EMSSTYPE_SUMMON )
		{
			SKILL::SSKILL_SUMMON_DATA &summon_data = pLinkSkill->m_sSPECIAL_SKILL.sSummon[sSkill.wLevel];

			SUMMON_DATA_SKILL sdata;
			sdata.bSummon = TRUE;
			sdata.sTarget = m_sTARIDS[0];
			sdata.sidSkill = pLinkSkill->m_sBASIC.sNATIVEID;
			
			sdata.Assign( summon_data );
			m_pGLGaeaServer->CreateSummon( sdata, m_dwGaeaID );
		}

		if ( bAsignStigma )
		{
			SSKILLACT sSKILLACT_STIGMA;
			sSKILLACT_STIGMA.fDELAY			= pLinkSkill->m_sEXT_DATA.fDELAY4DAMAGE;
			sSKILLACT_STIGMA.emAPPLY		= pLinkSkill->m_sBASIC.emAPPLY;
			//sSKILLACT_STIGMA.sSkillID		= sSkill.sNativeID;
			//sSKILLACT_STIGMA.wSkillLevel	= sSkill.wLevel;
			sSKILLACT_STIGMA.dwDamageFlag	= dwDamageFlag | DAMAGE_TYPE_NONE;
			sSKILLACT_STIGMA.nVAR_HP 		= nVarsArray[IDX_HP];
			sSKILLACT_STIGMA.nVAR_MP 		= nVarsArray[IDX_MP];
			sSKILLACT_STIGMA.nVAR_SP	 	= nVarsArray[IDX_SP];
			sSKILLACT_STIGMA.nGATHER_HP 	= nGATHER[IDX_HP];
			sSKILLACT_STIGMA.nGATHER_MP 	= nGATHER[IDX_MP];
			sSKILLACT_STIGMA.nGATHER_SP 	= nGATHER[IDX_SP];

			StigmaProc_Skill(&sSKILLACT_STIGMA, bStigmaEff);
		}
	}
}


void GLChar::ApplySkillTarget_SpecToMob( const EMSPEC_ADDON&	emSpec, 
										 const SKILL::SSPEC&	sSKILL_SPEC, 
										 GLACTOR*			pACTOR, 
										 DWORD&					dwDamageFlag, 
										 int*					nGather, 
										 int*					Vars )
{
	GLCrow* pCrow = m_pLandMan->GetCrow(pACTOR->GetCtrlID());
	// ���� ��� ��ų ȿ�� ���� ����. by luxes.
	switch( emSpec )
	{
	case EMSPECA_PUSHPULL:
		if (!pACTOR->IsValidBody())
            break;

		if (RANDOM_POS > sSKILL_SPEC.fVAR2 * sSKILL_SPEC.fVAR4)
            break;	// �߻�Ȯ�� ����.
		dwDamageFlag |= DAMAGE_TYPE_SHOCK;
		break;

	case EMSPECA_HP_GATHER:
		nGather[IDX_HP] += int ( abs(Vars[IDX_HP]) * sSKILL_SPEC.fVAR1 * sSKILL_SPEC.fVAR3 );
		break;

	case EMSPECA_MP_GATHER:
		nGather[IDX_MP] += int ( abs(Vars[IDX_HP]) * sSKILL_SPEC.fVAR1 * sSKILL_SPEC.fVAR3 );
		break;

	case EMSPECA_SP_GATHER:
		nGather[IDX_SP] += int ( abs(Vars[IDX_HP]) * sSKILL_SPEC.fVAR1 * sSKILL_SPEC.fVAR3 );
		break;
    case EMSPECA_DAMAGE_LOOP:
        if( !pACTOR->IsValidBody() )
            break;
		
		{			
			//// if ( pCrow->m_pCrowData->m_bNoAllPenalty )
			//if ( pCrow->IsNoAllPenalty() )
			//{
			//	pACTOR->SetMaxHpRate(1.0f);  //�Ҹ��� ȿ�� ������ ���� ����ȿ�� ����
			//	break;
			//}
   //         const float fMaxHP = static_cast<float>(GetMaxHP());
			//const float fNoWHP = static_cast<float>(GetNowHP());
   //         const float fActorOldHP = static_cast<float>(pACTOR->GetNowHP());
   //         const float fActorMaxHP = static_cast<float>(pACTOR->GetMaxHP());

   //         const int fDamage = static_cast<int>(fActorOldHP - ( fActorMaxHP * fNoWHP / fMaxHP ));
   //         if( fDamage > 0 )
   //         {
   //             Vars[IDX_HP] -= fDamage;  //�ִ� HP�� �����ϱ����� ������ ȿ���� �����ν� ������ ǥ�ø� �� �� �ְ���(������ ���� ���̴� HP���� ����)
   //         }
			//pACTOR->SetMaxHpRate( fNoWHP / fMaxHP );
		}
        break;	
	};
} //GLChar::ApplySkillTarget_SpecToMob

void GLChar::ApplySkillTarget_SpecToMobElse( const EMSPEC_ADDON&	emSpec, 
											 const SKILL::SSPEC&	sSKILL_SPEC, 
											 GLACTOR*			pACTOR, 
											 DWORD&					dwDamageFlag, 
											 int*					nGather, 
											 int*					Vars )
{
	switch( emSpec )
	{
	case EMSPECA_HP_GATHER:
		nGather[IDX_HP] += int ( abs(Vars[IDX_HP]) * sSKILL_SPEC.fVAR1 );
		break;

	case EMSPECA_MP_GATHER:
		nGather[IDX_MP] += int ( abs(Vars[IDX_HP]) * sSKILL_SPEC.fVAR1 );
		break;

	case EMSPECA_SP_GATHER:
		nGather[IDX_SP] += int ( abs(Vars[IDX_HP]) * sSKILL_SPEC.fVAR1 );
		break;

	case EMSPECA_PUSHPULL:
		if( !pACTOR->IsValidBody() )
            break;

		if( RANDOM_POS > sSKILL_SPEC.fVAR2 )
            break;	// �߻�Ȯ�� ����.
		
		dwDamageFlag |= DAMAGE_TYPE_SHOCK;
		break;

    case EMSPECA_VEHICLE_OFF: //����
		if( !pACTOR->IsValidBody() )
            break;

        if( RANDOM_POS < sSKILL_SPEC.fVAR1 )   // ȿ�� ���� ������
        {
			GLChar *pTargetChar = m_pGLGaeaServer->GetChar( pACTOR->GetCtrlID() );
			if (pTargetChar)
				pTargetChar->ActiveVehicle(false,false);
        }
        break;

	case EMSPECA_HOSTILE_ON:
		if( RANDOM_POS > sSKILL_SPEC.fVAR1 ||
			pACTOR->GetCrow() != CROW_PC )
		{
			break;
		} //if

		if ( IsConflictTarget((GLChar*)pACTOR))
			break;

		
			//	Note : ���� �����ڿ��� ������ ���. ( pk )
			//AddPlayHostile ( pCHAR->m_dwCharID, TRUE, bClubBattle );

			//	Note : ���� �����ڿ��� ������ ���. ( PK )
			//pCHAR->AddPlayHostile ( m_dwCharID, FALSE, bClubBattle );

		//AddPlayHostile ( pCHAR->m_dwCharID, TRUE, FALSE );
		//((GLChar*)pACTOR)->AddPlayHostile( m_dwCharID, FALSE/*, sSKILL_SPEC.fVAR2*/ );
		break;

	case EMSPECA_HOSTILE_OFF:
		if( pACTOR->GetCrow() != CROW_PC )
			break;

		//((GLChar*)pACTOR)->RemovePlayHostileEachOther( m_CharDbNum );
		break;	

    case EMSPECA_DAMAGE_LOOP:
		if( !pACTOR->IsValidBody() )
			break;

		{
   //         const float fMaxHP = static_cast<float>(GetMaxHP());
   //         const float fNoWHP = static_cast<float>(GetNowHP());
   //         const float fActorOldHP = static_cast<float>(pACTOR->GetNowHP());
   //         const float fActorMaxHP = static_cast<float>(pACTOR->GetMaxHP());

   //         const int fDamage = static_cast<int>(fActorOldHP - ( fActorMaxHP * fNoWHP / fMaxHP ));
   //         if( fDamage > 0 )
   //         {
   //             Vars[IDX_HP] -= fDamage;  //�ִ� HP�� �����ϱ����� ������ ȿ���� �����ν� ������ ǥ�ø� �� �� �ְ���(������ ���� ���̴� HP���� ����)
   //         }
			//pACTOR->SetMaxHpRate( fNoWHP / fMaxHP );
		}
        break;
	/*case EMSPECA_EXP_RATE:
		m_fEXP_RATE += sSKILL_SPEC.fVAR1;
		break;*/
	};
} //GLChar::ApplySkillTarget_SpecToMobElse

void GLChar::ApplySkillTarget_Spec( PGLSKILL pSkill, WORD wSKILL_LVL, GLACTOR* pACTOR, DWORD& dwDamageFlag, int* nGather, int* Vars )
{
	// ��ų Ư�� ���.
	for ( DWORD k = 0; k < SKILL::MAX_SPEC; ++k )
	{
		const SKILL::SSPEC&		sSKILL_SPEC = pSkill->m_sAPPLY.sSpecs[k].sSPEC[wSKILL_LVL];
		if ( pACTOR->GetCrow() == CROW_MOB )
			ApplySkillTarget_SpecToMob( pSkill->m_sAPPLY.sSpecs[k].emSPEC, sSKILL_SPEC, pACTOR, dwDamageFlag, nGather, Vars );
		else
			ApplySkillTarget_SpecToMobElse( pSkill->m_sAPPLY.sSpecs[k].emSPEC, sSKILL_SPEC, pACTOR, dwDamageFlag, nGather, Vars );
	} //for
}


void GLChar::ApplySkillTarget_HP( PGLSKILL			pSkill, 
								  WORD				wSKILL_LVL, 
								  SKILL::CDATA_LVL& sSKILL_DATA, 
								  GLACTOR*		pACTOR, 
								  const WORD		wAttackDivCount,
								  const STARGETID&	sTARID,
								  int&				nHP, 
								  DWORD&			dwDamageFlag )
{
	GLLandMan* const pLand( GetLandMan() );
	if ( pLand == NULL )
		return;

	//	��󿡰� ���ظ� �ִ� ��ų.
	if ( sSKILL_DATA.fBASIC_VAR < 0.0f )
	{
		const SNATIVEID baseMapID(pLand->GetMapID());
		DWORD		dwWeather = GLPeriod::GetInstance().GetMapWeather( baseMapID.wMainID, baseMapID.wSubID);
		dwDamageFlag = CALCDAMAGE( nHP, m_dwGaeaID, sTARID, pLand, pSkill, wSKILL_LVL, dwWeather, wAttackDivCount );
		//pACTOR->ReactDamage( this, dwDamageFlag );

		nHP = -nHP;
	}
	else //	����� ȸ������ �ִ� ��ų.
	{
		int nDX = pACTOR->GetMaxHP() - pACTOR->GetNowHP();
		nHP += (int) min(nDX,sSKILL_DATA.fBASIC_VAR);// + m_nHP_ADD;
	} //if..else
} //GLChar::ApplySkillTarget_HP

void GLChar::ApplySkillTarget_MP( PGLSKILL pSkill, SKILL::CDATA_LVL& sSKILL_DATA, GLACTOR* pACTOR, int& nMP )
{
	if ( sSKILL_DATA.fBASIC_VAR < 0.0f )
	{
		short		nRESIST = min( 99, pACTOR->GETRESIST().GetElement(pSkill->m_sAPPLY.emELEMENT) );		//	����ġ.
		int			nVAR	= int( -sSKILL_DATA.fBASIC_VAR );
		nMP -= (int)( nVAR - ( nVAR * nRESIST / 100.0f * GLCONST_CHAR::fRESIST_G ) );
	}
	else //	����� ȸ������ �ִ� ��ų.
	{
		int			nDX = pACTOR->GetMaxMP() - pACTOR->GetNowMP();
		nMP += (int)min( nDX, sSKILL_DATA.fBASIC_VAR );// + m_nMP_ADD;
	} //if..else
} //GLChar::ApplySkillTarget_MP

void GLChar::ApplySkillTarget_SP( PGLSKILL pSkill, SKILL::CDATA_LVL& sSKILL_DATA, GLACTOR* pACTOR, int& nSP )
{
	if ( sSKILL_DATA.fBASIC_VAR < 0.0f )
	{
		short		nRESIST = min( 99, pACTOR->GETRESIST().GetElement(pSkill->m_sAPPLY.emELEMENT) );
		int			nVAR	= int( -sSKILL_DATA.fBASIC_VAR );
		nSP -= (int)( nVAR - ( nVAR * nRESIST / 100.0f * GLCONST_CHAR::fRESIST_G ) );
	}
	else //	����� ȸ������ �ִ� ��ų.
	{
		int			nDX = pACTOR->GetMaxSP() - pACTOR->GetNowSP();
		nSP += (int)min( nDX, sSKILL_DATA.fBASIC_VAR );// + m_nSP_ADD;
	} //if..else
} //GLChar::ApplySkillTarget_SP


bool CheckPullIn( GLACTOR* pActor, GLACTOR* pTarget, float fRate, float& fDist )
{
	WORD	wLevel		=  pActor->GetLevel();
	WORD	wTarLevel	= pTarget->GetLevel();
	int		wTarAvoid	= pTarget->GetAvoid();

	float	fBasicRate	= ( (float)( wLevel - wTarLevel ) / (float) GLCONST_CHAR::wMAX_LEVEL * (float)wTarAvoid ) * 0.01f;
	fBasicRate += fRate;

	if (fBasicRate > 1.0f)
		fBasicRate = 1.0f;
	else if (fBasicRate < 0.0f)
		fBasicRate = 0.0f;

	if (fBasicRate > RANDOM_POS) 
	{
		fDist = D3DXVec3Length(&D3DXVECTOR3(pActor->GetPosition() - pTarget->GetPosition()));
		fDist -= GLCONST_CHAR::wBODYRADIUS;
		fDist -= pTarget->GetBodyRadius();
		fDist = -fDist;

		return true;
	}

	return false;
} //CheckPullIn

bool GetPullSpeed( GLACTOR* pActor, GLACTOR* pTarget, const SKILL::SSPEC& sSKILL_SPEC, float& fSpeed, float& fDist )
{
	if( !pTarget )
		return false;

	if( pTarget->GetCrow() == CROW_MOB )
	{	// ���� ��� ��ų ȿ�� ���� ����. by luxes.
		if( !CheckPullIn( pActor, pTarget, sSKILL_SPEC.fVAR1 * sSKILL_SPEC.fVAR2, fDist ) )
			return false;

		fSpeed = sSKILL_SPEC.fVAR2 * sSKILL_SPEC.fVAR4;
	}
	else
	{
		if( !CheckPullIn( pActor, pTarget, sSKILL_SPEC.fVAR1, fDist ) ) 
			return false;

		fSpeed = sSKILL_SPEC.fVAR2;
	} //if..else

	return true;
} //GetPullSpeed


bool GLChar::ApplySkillSelfTeleport( 
							GLGaeaServer* const pGaeaServer, 
							GLACTOR*			pActor,
							PGLSKILL				pSkill, 
							const WORD				wSKILL_LVL, 
							const SKILL::SSPEC&	sSKILL_SPEC,
							const STARGETID&		TargetID, 
							const WORD&			wTARNUM, 
							const STARID*			sTARIDS )
{
	float		fRatio = sSKILL_SPEC.fVAR1;
	if( fRatio < 1.0f )
	{
		fRatio = min( 1.0f, fRatio );
		fRatio = max( 0.0f, fRatio );
		if( fRatio <= RANDOM_POS ) 
			return false;
	} //if

	// where
	D3DXVECTOR3				vDestPos = TargetID.vPos;				// specific destination from a client. that needs checking about it

	D3DXVECTOR3		vDestPosF;
	D3DXVECTOR3		vDist = vDestPos - pActor->GetPosition();

	pActor->MoveTo( &vDestPos, true, MOVE_OPTION_NOT_TURN_IDLE );
	GLLandMan* pLandMan = pActor->GetLandMan();
	for ( DWORD _i = wTARNUM; _i; --_i )
	{		
		GLACTOR* const pReActor = pGaeaServer->GetTarget(pLandMan, STARGETID(sTARIDS[_i - 1].GETCROW(),sTARIDS[_i - 1].GETID()) );
		if ( pReActor == NULL )
			continue;

		if( pReActor->GetNowHP() == 0 )
			continue;

		if ( IsReActionable(pReActor) )
			continue;

		vDestPosF = pReActor->GetPosition() + vDist;
		vDestPosF.y = vDestPos.y;
		pReActor->MoveTo( &vDestPosF, true, MOVE_OPTION_NOT_TURN_IDLE );
	}	

	return true;
} //ApplySkillSelfTeleport

/**
 * 
 */
bool GLChar::ApplySkillSelfSwapPos( GLACTOR*			pActor,
							PGLSKILL				pSkill, 
							const WORD				wSKILL_LVL, 
							const SKILL::SSPEC&		sSKILL_SPEC,
							const WORD&				wTARNUM, 
							const STARID*			sTARIDS,
							GLLandMan*				pLandMan )
{
	return false;

	if( wTARNUM == 0 )
		return false;

	STARGETID			sActId( pActor->GetCrow(), pActor->GetCtrlID() );
	STARGETID			sTarId( sTARIDS[0].GETCROW(), sTARIDS[0].GETID() );

	GLACTOR*	pTarget = GLGaeaServer::GetInstance().GetTarget( pLandMan, sTarId );
	if( !pTarget )
		return false;

	if( !pTarget->IsValidBody() )
		return false;

	SSKILLACTEX			sSKILLACTEX;
	sSKILLACTEX.idSKILL		= pSkill->m_sBASIC.sNATIVEID;
	sSKILLACTEX.wSKILL_LVL	= wSKILL_LVL;

	DWORD		dwSwapFlag = sSKILL_SPEC.dwFLAG;
	if( pTarget->GetCrow() == CROW_MOB )
	{
		PGLCROW	pMob = m_pLandMan->GetCrow( pTarget->GetCtrlID() );
		if( pMob->m_pCrowData->IsPosHold() || pMob->m_pCrowData->IsDirHold() || pMob->IsIgnoreShock()  ) 
		{
			if( dwSwapFlag == 0 )
				dwSwapFlag = 1;
			else if( dwSwapFlag == 2 )
				return false;
		} //if
	} //if

	float		fDist = 0.0f;
	float		fSpeed = 0.0f;
	if( !GetPullSpeed( pActor, pTarget, sSKILL_SPEC, fSpeed, fDist ) )
		return false;

	sSKILLACTEX.fPUSH_PULL	= fDist;

	pLandMan->SwapPosAct( sActId, sTarId, fSpeed, dwSwapFlag );

	return true;
} //ApplySkillSelfSwapPos

bool GLChar::ApplySkillTargetOnward( GLACTOR* pActor, GLACTOR* pTarget, const STARGETID& sSelectTARID, const SKILL::SSPEC& sSKILL_SPEC, SSKILLACTEX& sSKILLACTEX )
{
	if( !pTarget )
		return false;

	if( !pTarget->IsValidBody() )
		return false;

	float		fDist = 0.0f, fSpeed = 0.0f;
	if( !GetPullSpeed( pActor, pTarget, sSKILL_SPEC, fSpeed, fDist ) )
		return false;

	if ( sSelectTARID.dwID == pTarget->GetCtrlID() )
	{
		sSKILLACTEX.fPUSH_PULL	= fDist; 
		sSKILLACTEX.wActionAnim	= (WORD)sSKILL_SPEC.dwFLAG;
		sSKILLACTEX.bOnward		= true;
	}
	return true;
} //ApplySkillTargetOnward
void GLChar::RECEIVE_SKILLFACT_ReleasePet(void)
{
	PGLPETFIELD pPet = m_pGLGaeaServer->GetPET ( m_dwPetGUID );
	if ( pPet && pPet->IsValid () )
	{
		m_pGLGaeaServer->DropOutPET ( pPet->m_dwGUID, false, false );
	}
} // GLChar::RECEIVE_SKILLFACT_ReleasePet

void GLChar::ApplySkillTargetStun( GLACTOR* pActor )
{
	if( !pActor->IsValidBody() )
		return;

	pActor->DO_STUN_ACTION();
} //GLChar::ApplySkillTargetStun