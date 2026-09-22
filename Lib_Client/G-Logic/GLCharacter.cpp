#include "stdafx.h"
#include "shlobj.h"
#include "SUBPATH.h"

#include "../Lib_Engine/GUInterface/Cursor.h"
//#include "DxCursor.h"
#include "DxInputDevice.h"
#include "editmeshs.h"
#include "DxMethods.h"
#include "DxViewPort.h"
#include "DxEffectMan.h"
#include "DxEnvironment.h"
#include "DxShadowMap.h"
#include "EditMeshs.h"
#include "GLogicData.h"
#include "GLItemMan.h"
#include "DxEffcharData.h"
#include "DxEffProj.h"

#include "tlhelp32.h"
#include "winbase.h"

#include "Psapi.h"
#pragma comment( lib, "Psapi.lib" )

#include "../../Lib_ClientUI/Interface/GameTextControl.h"
#include "../../Lib_ClientUI/Interface/InnerInterface.h"
#include "../../Lib_ClientUI/Interface/UITextControl.h"
#include "DxGlobalStage.h"
#include "GLGaeaClient.h"
#include "stl_Func.h"
#include "DxEffGroupPlayer.h"
#include "GLStrikeM.h"
#include "GLCrowData.h"
#include "GLPartyClient.h"
#include "GLQUEST.h"
#include "GLQUESTMAN.h"
#include "GLFriendClient.h"
#include "GLFactEffect.h"

#include "RANPARAM.h"
#include "DxRenderStates.h"
#include "GLCharacter.h"
#include "DXInputString.h"
#include "../Lib_Engine/DxSound/BgmSound.h"
#include "GLTaxiStation.h"
#include "GLItemMixMan.h"


#include "../Lib_Engine/DxEffect/Single/DxEffExtra.h"

#include "../NpcTalk/NpcDialogue.h"
#include "../NpcTalk/NpcTalkControl.h"
#include "../NpcTalk/NpcTalk.h"

/*ABL system, Juver, 2017/06/02 */
#include "GLStrikeSelector.h"

/*game stats, Juver, 2017/06/21 */
#include "../../Lib_ClientUI/Interface/GameStats.h"

/*pvp tyranny, Juver, 2017/08/24 */
#include "GLPVPTyrannyClient.h" 

/*bike color , Juver, 2017/11/15 */
#include "GLBikeColorDef.h"

/*school wars, Juver, 2018/01/19 */
#include "GLPVPSchoolWarsClient.h" 

/*pvp capture the flag, Juver, 2018/01/31 */
#include "GLPVPCaptureTheFlagClient.h"
#include "GLPVPCaptureTheFlagDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace COMMENT
{
	std::string szEMREACTION[REACT_SIZE+1] =
	{
		"REACT_MOB",
		"REACT_NPC",
		"REACT_P2P",
		"REACT_PVP",
		"REACT_ITEM",
		"REACT_SKILL",
		"REACT_GATHERING",
		"REACT_TOGGLE_VEHICLE",
		"REACT_SWAP_ARMS",
		"REACT_NULL",
	};
};

namespace
{
	class CFINDER
	{
	public:
		enum { BUFFER = 36, };
	protected:
		std::vector<STARGETID*>		m_vecTAR;

	public:
		CFINDER()
		{
			m_vecTAR.reserve(BUFFER);
		}

	public:
		void CLEAR ()
		{
			if ( !m_vecTAR.empty() )
			{
				m_vecTAR.erase ( m_vecTAR.begin(), m_vecTAR.end() );
			}
		}

	public:
		STARGETID& FindClosedCrow ( const DETECTMAP_RANGE &mapRange, const D3DXVECTOR3 vFromPt )
		{
			GASSERT(mapRange.first!=mapRange.second);

			CLEAR();
			std_afunc::CCompareTargetDist sCOMP(vFromPt);

			for ( DETECTMAP_ITER pos=mapRange.first; pos!=mapRange.second; ++pos )
			{
				m_vecTAR.push_back ( &(*pos) );
			}

			std::vector<STARGETID*>::iterator found = std::min_element ( m_vecTAR.begin(), m_vecTAR.end(), sCOMP );

			return *(*found);
		}

		static CFINDER& GetInstance()
		{
			static CFINDER Instance;
			return Instance;
		}
	};

	STARGETID* FindCrow ( const DETECTMAP_RANGE &mapRange, const STARGETID &_starid )
	{
		static STARGETID sTARID;

		GASSERT(mapRange.first!=mapRange.second);

		DETECTMAP_ITER iter = mapRange.first;
		for ( ; iter!=mapRange.second; iter++ )
		{
			sTARID = (*iter);

			/*skill summon, Juver, 2017/10/11 */
			if ( sTARID.emCrow == CROW_SUMMON )
			{
				PGLANYSUMMON psummon_any = GLGaeaClient::GetInstance().GetSummon ( sTARID.dwID );
				if ( psummon_any )
				{
					PGLCHARCLIENT pchar_client = GLGaeaClient::GetInstance().GetChar ( psummon_any->m_dwOwner );
					if ( pchar_client )
					{
						if ( pchar_client->GetCrow()==_starid.emCrow && pchar_client->GetCtrlID()==_starid.dwID )	
							return &sTARID;
					}
				}
			}
			else
			{
				if ( sTARID.emCrow==_starid.emCrow && sTARID.dwID==_starid.dwID )	return &sTARID;
			}
			
		}

		return NULL;
	}

	STARGETID* FindCrow ( const DETECTMAP_RANGE &mapRange, SCONFTING_CLT::CONFT_MEM &setconftMember )
	{
		static STARGETID sTARID;

		GASSERT(mapRange.first!=mapRange.second);

		DETECTMAP_ITER iter = mapRange.first;
		for ( ; iter!=mapRange.second; iter++ )
		{
			sTARID = (*iter);

			/*skill summon, Juver, 2017/10/11 */
			if ( sTARID.emCrow == CROW_SUMMON )
			{
				PGLANYSUMMON psummon_any = GLGaeaClient::GetInstance().GetSummon ( sTARID.dwID );
				if ( psummon_any )
				{
					PGLCHARCLIENT pchar_client = GLGaeaClient::GetInstance().GetChar ( psummon_any->m_dwOwner );
					if ( pchar_client )
					{
						SCONFTING_CLT::CONFT_MEM_ITER iter = setconftMember.find ( pchar_client->GetCtrlID() );
						if ( iter!=setconftMember.end() )	return &sTARID;
					}
				}
			}
			else if ( sTARID.emCrow==CROW_PC )
			{
				SCONFTING_CLT::CONFT_MEM_ITER iter = setconftMember.find ( sTARID.dwID );
				if ( iter!=setconftMember.end() )	return &sTARID;
			}
		}

		return NULL;
	}

	STARGETID* FindCrowCID ( const DETECTMAP_RANGE &mapRange, SCONFTING_CLT::CONFT_MEM &setconftMember )
	{
		static STARGETID sTARID;

		GASSERT(mapRange.first!=mapRange.second);

		DETECTMAP_ITER iter = mapRange.first;
		for ( ; iter!=mapRange.second; iter++ )
		{
			sTARID = (*iter);

			/*skill summon, Juver, 2017/10/11 */
			if ( sTARID.emCrow == CROW_SUMMON )
			{
				PGLANYSUMMON psummon_any = GLGaeaClient::GetInstance().GetSummon ( sTARID.dwID );
				if ( psummon_any )
				{
					PGLCHARCLIENT pchar_client = GLGaeaClient::GetInstance().GetChar ( psummon_any->m_dwOwner );
					if ( pchar_client )
					{
						SCONFTING_CLT::CONFT_MEM_ITER iter = setconftMember.find ( pchar_client->GetCharData().dwCharID );
						if ( iter!=setconftMember.end() )	return &sTARID;
					}
				}
			}
			else if ( sTARID.emCrow==CROW_PC )
			{
				PGLCHARCLIENT pCHAR = GLGaeaClient::GetInstance().GetChar ( sTARID.dwID );

				if ( pCHAR )
				{
					SCONFTING_CLT::CONFT_MEM_ITER iter = setconftMember.find ( pCHAR->GetCharData().dwCharID );
					if ( iter!=setconftMember.end() )	return &sTARID;
				}
			}
		}

		return NULL;
	}
};

const float GLCharacter::m_fELAPS_MOVE = 0.2f;

GLCharacter::GLCharacter () :
	m_pd3dDevice(NULL),
	m_pSkinChar(NULL),

	m_vDir(D3DXVECTOR3(0,0,-1)),
	m_vDirOrig(D3DXVECTOR3(0,0,-1)),
	m_vPos(D3DXVECTOR3(0,0,0)),

	m_vSAVEPOSGM(D3DXVECTOR3(0,0,0)),
	m_vSAVETARPOS(D3DXVECTOR3(0,0,0)),

	m_vServerPos(0,0,0),

	m_bPASSIVITY_ACTION(FALSE),
	m_fPASSIVITY_ACTION_ELAPS(0.0f),
	m_fPASSIVITY_ACTION_TOTAL(0.0f),

	m_bPASSIVITY_ACTION_CHECK(FALSE),
	m_fPASSIVITY_ACTION_CHECK(0.0f),

	m_fKeyDownCheckTime(0.0f),
	m_bOneHourNotInputKey(FALSE),

	m_Action(GLAT_IDLE),
	m_dwActState(NULL),

	m_wTARNUM(0),
	m_vTARPOS(0,0,0),

	m_fLastMsgMoveSend(0.0f),

	m_fIdleTime(0.0f),
	m_fattTIMER(0.0f),
	m_nattSTEP(0),

	m_dwWAIT(0),

	m_sRunSkill(NATIVEID_NULL()),
	m_sActiveSkill(NATIVEID_NULL()),
	m_dwANISUBCOUNT(0),
	m_dwANISUBSELECT(0),
	m_dwANISUBGESTURE(0),

	m_emOldQuestionType(QUESTION_NONE),

	m_dwGaeaID(0),

	m_wInvenPosX1(0),
	m_wInvenPosY1(0),
	m_wInvenPosX2(0),
	m_wInvenPosY2(0),

	/*item wrapper, Juver, 2018/01/12 */
	m_wInvenPosX3(0),
	m_wInvenPosY3(0),
	//locker
	m_wInvenPosX4(0),
	m_wInvenPosY4(0),

	m_bCLUB_CERTIFY(FALSE),
	m_fCLUB_CERTIFY_TIMER(0),

	m_wPMPosX(USHRT_MAX),
	m_wPMPosY(USHRT_MAX),

	m_vMaxOrg(6,20,6),
	m_vMinOrg(-6,0,-6),

	m_vMax(6,20,6),
	m_vMin(-6,0,-6),
	m_fHeight(20.f),
	m_dwNPCID(0),

	//dmk14 | 11-1-16 | AI - auto pilot
	m_vBotPos(D3DXVECTOR3(0,0,0)),
	m_bRunBot( false ),
	m_bBotSupport( false ),
	m_bAutoLoot( false ),
	m_bBotPK( false ),
	m_bResuPtLeader( false ),
	m_bBotFLeader( false ),
	m_bBotLockSpot( true ),

	m_bRunAutoPots( false ),
	m_vBotTargetPt(0,0,0),

	//m_dwSummonGUID(GAEAID_NULL),

	m_bEnableHairSytle(FALSE),
	m_bEnableHairColor(FALSE),
	m_bEnableFaceStyle(FALSE),

	m_bAttackable(TRUE),
	m_llPetCardGenNum(0),
	m_sPetCardNativeID(NATIVEID_NULL()),
	m_cPetCardGenType(-1),
	m_bRecievedPetCardInfo(FALSE),
	m_bIsPetActive(FALSE),
	m_bMiniGameOpen(FALSE),
//	m_bWeaponSwitching(FALSE),	
	m_hCheckStrDLL( NULL ),
	m_pCheckString( NULL ),
	m_bReqVehicle ( FALSE ),
	m_bReqDoneVehi( FALSE ),
	m_bIsVehicleActive ( FALSE ),
	m_bRecivedVehicleItemInfo ( FALSE ),
	m_bGarbageOpen ( false ),
	m_bItemShopOpen ( false ),
	m_bReqAttendList( false ),
	m_dwComboAttend(0),
	m_tAttendLogin(0),
	m_dwAttendTime(0),

	m_bRecordChat(FALSE),
	m_strRecordChat(""),

	m_wActionAnim(AN_SUB_00_SIZE), /*dash skill logic, Juver, 2017/06/17 */

	m_fHPCheckTimer( 0.0f ), //hpcheck
	m_wLastHPCheck( 0 ),//hpcheck
	m_wLastHPCounter( 0 ),//hpcheck
	m_dwMismatchCount( 0 ),

	/*game stats, Juver, 2017/06/21 */
	m_fPacketInterValTimer(0.0f), 
	m_fFPSInterValTimer(0.0f),
	m_uNumFramesSinceLastCalc(0),

	/*rightclick wear/unwear, Juver, 2017/06/23 */
	m_bRingSlotFirst(false), 
	m_bEarringSlotFirst(false),
	m_bAccessorySlotFirst(false),
	m_fWearDelay(1.0f),

	/* pet fix add call delay Juver 2017/06/27 */
	m_fPetUseDelay(PET_USE_DELAY),

	/*npc shop, Juver, 2017/07/27 */
	m_sNPCShopCrow(NATIVEID_NULL()),
	m_sNPCShopItem(NATIVEID_NULL()),
	m_wNPCShopType(0),
	m_fNPCShopDelay(NPC_SHOP_DELAY),

	/*item exchange, Juver, 2017/10/13 */
	m_fNPCItemExchangeDelay(NPC_ITEM_EXCHANGE_DELAY),

	/*product item, Juver, 2017/10/18 */
	m_dwProductID(UINT_MAX),
	m_nProductCount(1),
	m_bItemCompoundTask(FALSE),
	m_dwItemCompoundTaskID(UINT_MAX),
	m_fItemCompoundTaskTime(0.0f),
	m_fItemCompoundTaskTimer(0.0f),

	/*activity system, Juver, 2017/11/05 */
	m_fCharTitleDelay(0.0f),

	/*charinfoview , Juver, 2017/11/11 */
	m_fReqCharInfoTimer( 0.0f ),

	/*bike color , Juver, 2017/11/16 */
	m_bEnableBikeColor(FALSE),

	/*change scale card, Juver, 2018/01/03 */
	m_fActiveCharScale(1.0f),
	m_bChangeCharScale(FALSE),

	m_wItemColor1(ITEMCOLOR_WHITE),
	m_wItemColor2(ITEMCOLOR_WHITE),
	m_bEnableItemColor(FALSE),
	m_emSlotItemColor(SLOT_TSIZE),

	/*pvp capture the flag, Juver, 2018/02/07 */
	m_fCaptureTheFlagHoldFlagTimer(0.0f),
	m_bCaptureTheFlagLocatorOn(FALSE),

	/* car, cart color, Juver, 2018/02/14 */
	m_bEnableCarColor(FALSE),
	m_wCarColorA(ITEMCOLOR_WHITE),
	m_wCarColorB(ITEMCOLOR_WHITE),
	m_bDisTradeOLD (FALSE),
	m_bDisTrade (FALSE),
	m_bAutoPots( FALSE ), // 9/13/2016 - Anti  Auto Pots System - Eduj
	m_bUsePots( FALSE ), // 9/13/2016 - Anti  Auto Pots System - Eduj
	m_bDisablePots ( FALSE ), // 9/13/2016 - Anti  Auto Pots System - Eduj
	m_bDetectOnce ( FALSE ), // 9/13/2016 - Anti  Auto Pots System - Eduj
	m_fTickDelay (0),
	m_dwTickCount(0),
	m_bReqSwapArms ( FALSE ),
	m_fBotPotsDelay ( 0.0f )
{	
	D3DXMatrixIdentity ( &m_matTrans );
	for ( int i=0; i<EMSTORAGE_CHANNEL; ++i )		m_dwNumStorageItem[i] = UINT_MAX;

	for( int i = 0; i < 8; i++ )
	{
		m_bOldVisibleTracingUI[i] = FALSE;
		m_bNewVisibleTracingUI[i] = FALSE;
	}

	m_fPrintProcessInfoMsgGap = -10.0f;

	m_vecTAR_TEMP.reserve(220);

	m_bReqSwapArms			  = FALSE;

	SetSTATE(EM_ACT_RUN);

	m_mapPETCardInfo.clear();
	m_mapPETCardInfoTemp.clear();

	m_mapPETReviveInfo.clear();

	m_mapVEHICLEItemInfo.clear();
	m_mapVEHICLEItemInfoTemp.clear();

	m_vecAttend.clear();

	m_vecMarketClick.clear();

	m_fHPCheckTimer = 0.0f; //hpcheck
	m_wLastHPCheck = 0;//hpcheck
	m_wLastHPCounter = 0;//hpcheck


#ifdef TH_PARAM
	HMODULE m_hCheckStrDLL = LoadLibrary("ThaiCheck.dll");

	if ( m_hCheckStrDLL )
	{
		m_pCheckString = ( BOOL (_stdcall*)(CString)) GetProcAddress(m_hCheckStrDLL, "IsCompleteThaiChar");
	}
#endif 

	/*skill summon, Juver, 2017/10/08 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		m_dwSummonGUID[i] = GAEAID_NULL;

	/*bike color , Juver, 2017/11/13 */
	for( int i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
		m_wBikeColor[i] = 0;

}

GLCharacter::~GLCharacter ()
{

	SAFE_DELETE(m_pSkinChar);

#ifdef TH_PARAM
	if( m_hCheckStrDLL ) FreeLibrary( m_hCheckStrDLL );
#endif
}

float GLCharacter::GetBuyRate ()
{
	float fBUY_RATE = GET_PK_SHOP2BUY();

	PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();
	if ( pLand )
	{
		fBUY_RATE += pLand->m_fCommission;
	}

	return fBUY_RATE;
}

float GLCharacter::GetSaleRate ()
{
	float fSALE_RATE = GET_PK_SHOP2SALE ();
	
	PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();
	if ( pLand )
	{
		fSALE_RATE -= pLand->m_fCommission;
	}

	return fSALE_RATE;
}

void GLCharacter::ResetData ()
{
	int i = 0;

	GLCHARLOGIC::RESET_DATA();

	m_vDir = D3DXVECTOR3(0,0,-1);
	m_vDirOrig = D3DXVECTOR3(0,0,-1);
	m_vPos = D3DXVECTOR3(0,0,0);
	D3DXMatrixIdentity ( &m_matTrans );

	m_Action = GLAT_IDLE;
	m_dwActState = NULL;
	SetSTATE(EM_ACT_RUN);

	m_sTargetID = STARGETID(CROW_MOB,EMTARGET_NULL,D3DXVECTOR3( 0, 0, 0 ));

	m_wTARNUM = 0;
	m_vTARPOS = D3DXVECTOR3(0,0,0);
	for ( i=0; i<EMTARGET_NET; ++i )
		m_sTARIDS[i] = STARID(CROW_MOB,EMTARGET_NULL);

	m_sRunSkill = SNATIVEID(false);
	m_sActiveSkill = SNATIVEID(false);
	m_dwANISUBCOUNT = 0;
	m_dwANISUBSELECT = 0;
	m_dwANISUBGESTURE = 0;

	m_fLastMsgMoveSend = 0.0f;
	m_sLastMsgMove = GLMSG::SNETPC_GOTO();
	m_sLastMsgMoveSend = GLMSG::SNETPC_GOTO();

	m_sREACTION.RESET();

	m_fIdleTime = 0.0f;
	m_fattTIMER = 0.0f;
	m_nattSTEP = 0;
	m_dwWAIT = 0;

	m_bRecordChat = FALSE;

	m_strRecordChat = "";

	m_dwGaeaID = 0;

	for ( i=0; i<EMSTORAGE_CHANNEL; ++i )		m_dwNumStorageItem[i] = UINT_MAX;
	m_sCONFTING.RESET();

	m_sPMarket.DoMarketClose();

	m_bCLUB_CERTIFY = FALSE;
	m_fCLUB_CERTIFY_TIMER = 0;

	m_sCLUB.RESET();

	m_vMaxOrg = D3DXVECTOR3(6,20,6);
	m_vMinOrg = D3DXVECTOR3(-6,0,-6);

	m_vMax = D3DXVECTOR3(6,20,6);
	m_vMin = D3DXVECTOR3(-6,0,-6);

	m_dwNPCID = 0;

	m_bAttackable = TRUE;

	m_mapPETCardInfo.clear();
	m_mapPETCardInfoTemp.clear();
	m_bRecievedPetCardInfo = FALSE;
	m_bIsPetActive		   = FALSE;

	m_mapPETReviveInfo.clear();

	m_mapVEHICLEItemInfo.clear();
	m_mapVEHICLEItemInfoTemp.clear();

	m_bReqVehicle			  = FALSE;
	m_bReqDoneVehi			  = FALSE;
	m_bIsVehicleActive		  = FALSE;
	m_bRecivedVehicleItemInfo = FALSE;

	for( i = 0; i < 8; i++ )
	{
		m_bOldVisibleTracingUI[i] = FALSE;
		m_bNewVisibleTracingUI[i] = FALSE;
	}

	m_fPrintProcessInfoMsgGap = -10.0f;

	m_sVehicle.RESET();

	//dmk14 | 11-1-16 | AI - auto pilot
	m_vBotPos = (D3DXVECTOR3(0,0,0));
	m_bRunBot = false;
	m_bBotSupport = false;
	m_bBotPK = false;
	m_bResuPtLeader = false;
	m_bBotFLeader = false;
	m_bBotLockSpot = true;

	m_bRunAutoPots = false;
	m_fBotPotsDelay = 0.0f;

	m_bAutoLoot = false;
	m_vBotTargetPt = D3DXVECTOR3(0,0,0);

	m_bGarbageOpen = false;
	m_bItemShopOpen = false;
	m_bReqAttendList = false;
	m_vecAttend.clear();
	m_dwComboAttend = 0;
	m_tAttendLogin = 0;
	m_dwAttendTime = 0;

	m_dwTransfromSkill = SNATIVEID::ID_NULL;

	m_vecMarketClick.clear();

	/*charinfoview , Juver, 2017/11/11 */
	m_fReqCharInfoTimer = 0.0f;

	/*bike color , Juver, 2017/11/13 */
	for( int i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
		m_wBikeColor[i] = 0;

	m_bEnableBikeColor = FALSE;

	/*change scale card, Juver, 2018/01/03 */
	m_fActiveCharScale = 1.0f;
	m_bChangeCharScale = FALSE;

	/*item color, Juver, 2018/01/10 */
	m_wItemColor1 = ITEMCOLOR_WHITE;
	m_wItemColor2 = ITEMCOLOR_WHITE;
	m_bEnableItemColor = FALSE;
	m_emSlotItemColor = SLOT_TSIZE;

	/*pvp capture the flag, Juver, 2018/02/07 */
	m_fCaptureTheFlagHoldFlagTimer = 0.0f;
	m_bCaptureTheFlagLocatorOn = FALSE;

	/* car, cart color, Juver, 2018/02/14 */
	m_bEnableCarColor = FALSE;
	m_wCarColorA = ITEMCOLOR_WHITE;
	m_wCarColorB = ITEMCOLOR_WHITE;

	m_bAutoPots = FALSE;
	m_bUsePots = FALSE;
	m_bDisablePots = FALSE;
	m_bDetectOnce = FALSE; 
	m_fTickDelay = 0;
	m_dwTickCount = 0;
}

HRESULT GLCharacter::SkinLoad ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	EMCHARINDEX emIndex = CharClassToIndex(m_emClass);
	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().LoadData( GLCONST_CHAR::szCharSkin[emIndex], pd3dDevice, TRUE );
	if ( !pSkinChar )	return E_FAIL;


	SAFE_DELETE(m_pSkinChar);
	m_pSkinChar = new DxSkinChar;

	m_pSkinChar->SetCharData ( pSkinChar, pd3dDevice, TRUE );

	
	UpdateSuit( TRUE );





	// Note : 1.AABB Box�� �����´�. 2.���̸� ����� ���´�.
	m_pSkinChar->GetAABBBox( m_vMaxOrg, m_vMinOrg );
	m_fHeight = m_vMaxOrg.y - m_vMinOrg.y;

	//	Note : ���ϸ��̼� �ʱ�ȭ.
	//
	if ( m_pSkinChar )
	{
		m_pSkinChar->SELECTANI ( AN_GUARD_N, GLCHARLOGIC::m_emANISUBTYPE );

		if ( !m_pSkinChar->GETCURANIMNODE() )
		{
			CDebugSet::ToLogFile ( "ERORR : current animation node null point error [ %s ] [ M %d S %d ]", m_pSkinChar->m_szFileName, AN_GUARD_N, GLCHARLOGIC::m_emANISUBTYPE );
			m_pSkinChar->DEFAULTANI ();
		}
	}

	//	Note : ��ų ����, �����̻� ȿ�� ����.
	FACTEFF::ReNewEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT, m_sSTATEBLOWS, m_matTrans, m_vDir );

	if( GLCONST_CHAR::cCONSTCLASS[emIndex].strCLASS_EFFECT.size() != 0 )
	{
		DxEffcharDataMan::GetInstance().PutPassiveEffect ( m_pSkinChar, GLCONST_CHAR::cCONSTCLASS[emIndex].strCLASS_EFFECT.c_str(), &m_vDir );
	}

	return S_OK;
}

HRESULT GLCharacter::SkinLoadForTool( LPDIRECT3DDEVICEQ pd3dDevice, const char* szFile )
{
	EMCHARINDEX emIndex = CharClassToIndex(m_emClass);
	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().LoadData( szFile, pd3dDevice, TRUE );
	if ( !pSkinChar )	return E_FAIL;


	SAFE_DELETE(m_pSkinChar);
	m_pSkinChar = new DxSkinChar;

	m_pSkinChar->SetCharData ( pSkinChar, pd3dDevice, TRUE );


//	UpdateSuit( TRUE );





	// Note : 1.AABB Box�� �����´�. 2.���̸� ����� ���´�.
	m_pSkinChar->GetAABBBox( m_vMaxOrg, m_vMinOrg );
	m_fHeight = m_vMaxOrg.y - m_vMinOrg.y;

	//	Note : ���ϸ��̼� �ʱ�ȭ.
	//
	if ( m_pSkinChar )
	{
		m_pSkinChar->SELECTANI ( AN_GUARD_N, GLCHARLOGIC::m_emANISUBTYPE );

		if ( !m_pSkinChar->GETCURANIMNODE() )
		{
			CDebugSet::ToLogFile ( "ERORR : current animation node null point error [ %s ] [ M %d S %d ]", m_pSkinChar->m_szFileName, AN_GUARD_N, GLCHARLOGIC::m_emANISUBTYPE );
			m_pSkinChar->DEFAULTANI ();
		}
	}

	//	Note : ��ų ����, �����̻� ȿ�� ����.
	FACTEFF::ReNewEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT, m_sSTATEBLOWS, m_matTrans, m_vDir );

	if( GLCONST_CHAR::cCONSTCLASS[emIndex].strCLASS_EFFECT.size() != 0 )
	{
		DxEffcharDataMan::GetInstance().PutPassiveEffect ( m_pSkinChar, GLCONST_CHAR::cCONSTCLASS[emIndex].strCLASS_EFFECT.c_str(), &m_vDir );
	}

	return S_OK;
}


HRESULT GLCharacter::Create ( NavigationMesh* pNavi, SCHARDATA2 *pCharData2, D3DXVECTOR3* pvPos, DWORD dwGaeaID, LPDIRECT3DDEVICEQ pd3dDevice )
{
	GASSERT(pd3dDevice);
	HRESULT hr;

	m_pd3dDevice = pd3dDevice;
	m_dwGaeaID = dwGaeaID;

	m_dwTransfromSkill = SNATIVEID::ID_NULL;

	//	Note : �ɸ����� ������ ����.
	if ( pCharData2 )										SCHARDATA2::Assign ( *pCharData2 );
	else if ( GLCONST_CHAR::VAID_CHAR_DATA2(0,GLCI_SWORDSMAN_M) )	SCHARDATA2::Assign ( GLCONST_CHAR::GET_CHAR_DATA2(0,GLCI_SWORDSMAN_M) );

    m_cInventory.SetAddLine ( GetOnINVENLINE(), true );
	CInnerInterface::GetInstance().SetInventorySlotViewSize ( EM_INVEN_DEF_SIZE_Y + GetOnINVENLINE() );

	// ��Ʈ�� �κ��丮�� ���ΰ� 10ĭ�̹Ƿ� 6ĭ�� �� �߰��Ѵ�.
#if defined(VN_PARAM) //vietnamtest%%%
	m_cVietnamInventory.SetAddLine ( 6, true );
#endif	

	GLCHARLOGIC::INIT_DATA ( FALSE, TRUE );
	m_sCONFTING.RESET();

	//	�ʱ� �� ��ų ����.
	if ( m_wSKILLQUICK_ACT < EMSKILLQUICK_SIZE )
	{
		m_sRunSkill = m_sSKILLQUICK[m_wSKILLQUICK_ACT];
	}

	hr = SkinLoad ( pd3dDevice );
	if ( FAILED(hr) )	return hr;


	//	Note : �̵� ���� �ʱ�ȭ.
	if ( pvPos )	m_vPos = *pvPos;
	else			m_vPos = m_vStartPos;
	
	//	Note : ���� ��ġ �ʱ�ȭ.
	D3DXMatrixTranslation ( &m_matTrans, m_vPos.x, m_vPos.y, m_vPos.z );

	m_actorMove.SetMaxSpeed ( GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX].fWALKVELO );
	if ( pNavi )
	{
		m_actorMove.Create ( pNavi, m_vPos, -1 );
	}

	DoActWait ();

	GLQuestPlay::MAPQUEST& mapQuest = m_cQuestPlay.GetQuestProc ();
	{
		bool bKNOCK(false);

		GLQuestPlay::MAPQUEST_ITER iter = mapQuest.begin();
		GLQuestPlay::MAPQUEST_ITER iter_end = mapQuest.end();
		for ( ; iter!=iter_end; ++iter )
		{
			GLQUESTPROG *pPROG = (*iter).second;
			if ( !pPROG->IsReqREADING () )	continue;

			bKNOCK = true;

			GLQUEST *pQUEST = GLQuestMan::GetInstance().Find ( pPROG->m_sNID.dwID );
			if ( pQUEST )
				CInnerInterface::GetInstance().PrintConsoleText ( ID2GAMEINTEXT("QUEST_KNOCK"), pQUEST->GetTITLE() );
			else
				CInnerInterface::GetInstance().PrintConsoleText ( ID2GAMEINTEXT("QUEST_NOINFO"), pPROG->m_sNID.dwID );
		}

		if ( bKNOCK )
		{
			CInnerInterface::GetInstance().PrintConsoleText ( ID2GAMEINTEXT("QUEST_KNOCK_END") );
		}
	}

	GLFriendClient::GetInstance().Clear();


	GLMSG::SNET_GM_LIMIT_EVENT_TIME_REQ NetMsg;
	NetMsg.dwGaeaID	= m_dwGaeaID;
	NETSEND ( &NetMsg );

#if defined(VN_PARAM) //vietnamtest%%%
	// �̵� �� ������ ��Ʈ�� Ž�� �ý��� �ð��� ��û�Ѵ�.
	GLMSG::SNETPC_VIETNAM_TIME_REQ NetVietnamMsg;
	NetMsg.dwGaeaID	= m_dwGaeaID;	
	NETSENDTOFIELD ( &NetVietnamMsg );
#endif


	PLANDMANCLIENT pLandMClient = GLGaeaClient::GetInstance().GetActiveMap();
	if ( !pLandMClient )								return E_FAIL;

	SNATIVEID mapID = pLandMClient->GetMapID();
	if( pLandMClient->IsInstantMap() ) mapID.wSubID = 0;

	SMAPNODE *pMapNode = GLGaeaClient::GetInstance().FindMapNode ( mapID );
	if ( pMapNode )
	{
		GLLevelFile cLevelFile;
		if( cLevelFile.LoadFile ( pMapNode->strFile.c_str(), TRUE, NULL ) )
		{
			SLEVEL_ETC_FUNC* pEtcFunc = cLevelFile.GetLevelEtcFunc();
			if( !pEtcFunc )	return E_FAIL;
			if( pEtcFunc->m_bUseFunction[EMETCFUNC_CONTROLCAM] )
			{
				D3DXVECTOR3 vFromPt, vLookAtPt, vUpPt;
				DxViewPort::GetInstance().GetViewTrans( vFromPt, vLookAtPt, vUpPt );
				vFromPt = pEtcFunc->m_sControlCam.vCamPos + vLookAtPt;

				DxViewPort::GetInstance().SetViewTrans( vFromPt, vLookAtPt, vUpPt );
				DxViewPort::GetInstance().SetCameraControl( TRUE, pEtcFunc->m_sControlCam.vCamPos, pEtcFunc->m_sControlCam.fUp, 
																	pEtcFunc->m_sControlCam.fDown, pEtcFunc->m_sControlCam.fLeft, 
																	pEtcFunc->m_sControlCam.fRight, pEtcFunc->m_sControlCam.fFar, 
																	pEtcFunc->m_sControlCam.fNear );
			}else{
				DxViewPort::GetInstance().SetCameraControl( FALSE );
			}
		}

	}

	/*Codex System, Jhoniex 2/14/2024 */
	CODEX_STATS(1,1);

	return S_OK;
}

BOOL GLCharacter::IsVALID_STORAGE ( DWORD dwChannel )
{
	if ( dwChannel >= EMSTORAGE_CHANNEL )	return FALSE;
	return m_bStorage[dwChannel];
}

const SITEMCUSTOM& GLCharacter::GET_PRETRADE_ITEM ()
{
	static SITEMCUSTOM sItemCustom;
	sItemCustom.sNativeID = NATIVEID_NULL();

	SINVEN_POS sPreTradeItem = GLTradeClient::GetInstance().GetPreItem();

	if ( !sPreTradeItem.VALID() )		return sItemCustom;

	SINVENITEM* pResistItem = m_cInventory.GetItem ( sPreTradeItem.wPosX, sPreTradeItem.wPosY );
	if ( !pResistItem )					return sItemCustom;

	sItemCustom = pResistItem->sItemCustom;

	return sItemCustom;
}

void GLCharacter::ReSelectAnimation ()
{
	// ����ó���̸� �ִϸ��̼� ���� ����.
	if ( !IsACTION ( GLAT_TALK ) && !IsACTION( GLAT_GATHERING ) && !IsACTION( GLAT_SKILL ) )
	{
		//	Note : ���ϸ��̼� �ʱ�ȭ.
		//

		/*vehicle system, Juver, 2017/08/07 */
		if ( m_bVehicle )
		{
			int emType = m_sVehicle.m_emTYPE;
			if ( emType == VEHICLE_TYPE_BOARD )
			{
				m_emANISUBTYPE = (EMANI_SUBTYPE) ( AN_SUB_HOVERBOARD );
				if ( m_pSkinChar->GETCURSTYPE() !=GLCHARLOGIC::m_emANISUBTYPE )
					m_pSkinChar->SELECTANI ( m_pSkinChar->GETCURMTYPE(), GLCHARLOGIC::m_emANISUBTYPE );
			}else{
				m_emANISUBTYPE = CHECK_VEHICLE_SUB( m_pSkinChar->GETCURMTYPE() );
				if ( m_pSkinChar->GETCURSTYPE() !=GLCHARLOGIC::m_emANISUBTYPE )
					m_pSkinChar->SELECTANI ( CHECK_VEHICLE_MAIN( m_sVehicle.m_sVehicleID ), GLCHARLOGIC::m_emANISUBTYPE );
			}
		}else{
			if ( m_pSkinChar->GETCURSTYPE() !=GLCHARLOGIC::m_emANISUBTYPE )
				m_pSkinChar->SELECTANI ( m_pSkinChar->GETCURMTYPE(), GLCHARLOGIC::m_emANISUBTYPE );
		}
	}
}

HRESULT GLCharacter::DoActWait ()
{
	m_dwWAIT = 0;
	SetSTATE(EM_ACT_WAITING);

	return S_OK;
}

HRESULT GLCharacter::ReBirth ( DWORD dwHP, DWORD dwMP, DWORD dwSP, D3DXVECTOR3 vPos, bool bWait )
{
	FACTEFF::DeleteEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT, m_sSTATEBLOWS );

	//	��Ȱ�� ä��ġ ȸ����.
	INIT_RECOVER ();

	m_sHP.dwNow = dwHP;
	m_sMP.dwNow = dwMP;
	m_sSP.dwNow = dwSP;

	//	�̵��� �㿡 ����.
	MoveActiveMap ( GLGaeaClient::GetInstance().GetActiveMap()->GetNaviMesh(), vPos, bWait );

	TurnAction ( GLAT_IDLE );

	D3DXMATRIX matEffect;
	D3DXMatrixTranslation ( &matEffect, m_vPos.x, m_vPos.y, m_vPos.z );

	STARGETID sTargetID(CROW_PC,m_dwGaeaID,m_vPos);
	DxEffGroupPlayer::GetInstance().NewEffGroup
	(
		GLCONST_CHAR::strREBIRTH_EFFECT.c_str(),
		matEffect,
		&sTargetID
	);

	ReSetSTATE(EM_ACT_DIE);

	return S_OK;
}

HRESULT GLCharacter::MoveActiveMap ( NavigationMesh* pNavi, const D3DXVECTOR3 &vPos, bool bWait )
{
	if ( bWait ) DoActWait ();

	//	Note : �̵� ���� �ʱ�ȭ.
	m_vPos = vPos;

	m_actorMove.Stop ();
	if ( pNavi )
	{
		m_actorMove.Create ( pNavi, m_vPos, -1 );
		SetPosition ( m_vPos );
	}

	ReSetSTATE(EM_REQ_GATEOUT);

	// �̵� �� ������ �̺�Ʈ �ð��� ��û�Ѵ�.
	GLMSG::SNET_GM_LIMIT_EVENT_TIME_REQ NetMsg;
	NetMsg.dwGaeaID	= m_dwGaeaID;
	NETSEND ( &NetMsg );

#if defined(VN_PARAM) //vietnamtest%%%
	// �̵� �� ������ ��Ʈ�� Ž�� �ý��� �ð��� ��û�Ѵ�.
	GLMSG::SNETPC_VIETNAM_TIME_REQ NetVietnamMsg;
	NetMsg.dwGaeaID	= m_dwGaeaID;	
	NETSENDTOFIELD ( &NetVietnamMsg );
#endif

	PLANDMANCLIENT pLandMClient = GLGaeaClient::GetInstance().GetActiveMap();
	if ( !pLandMClient )								return E_FAIL;

	SNATIVEID mapID = pLandMClient->GetMapID();
	if( pLandMClient->IsInstantMap() ) mapID.wSubID = 0;

	SMAPNODE *pMapNode = GLGaeaClient::GetInstance().FindMapNode ( mapID );
	if ( pMapNode )
	{
		GLLevelFile cLevelFile;
		if( cLevelFile.LoadFile ( pMapNode->strFile.c_str(), TRUE, NULL ) )

		{
			SLEVEL_ETC_FUNC* pEtcFunc = cLevelFile.GetLevelEtcFunc();
			if( !pEtcFunc )	return E_FAIL;
			if( pEtcFunc->m_bUseFunction[EMETCFUNC_CONTROLCAM] )
			{
				D3DXVECTOR3 vFromPt, vLookAtPt, vUpPt;
				DxViewPort::GetInstance().GetViewTrans( vFromPt, vLookAtPt, vUpPt );
				vFromPt = pEtcFunc->m_sControlCam.vCamPos + vLookAtPt;

				DxViewPort::GetInstance().SetViewTrans( vFromPt, vLookAtPt, vUpPt );
				DxViewPort::GetInstance().SetCameraControl( TRUE, pEtcFunc->m_sControlCam.vCamPos, pEtcFunc->m_sControlCam.fUp, 
																	pEtcFunc->m_sControlCam.fDown, pEtcFunc->m_sControlCam.fLeft, 
																	pEtcFunc->m_sControlCam.fRight, pEtcFunc->m_sControlCam.fFar, 
																	pEtcFunc->m_sControlCam.fNear );
			}else{
				DxViewPort::GetInstance().SetCameraControl( FALSE );
			}
		}

	}


	return S_OK;
}

// *****************************************************
// Desc: ������ ������ ������Ʈ
// *****************************************************
HRESULT GLCharacter::UpdateSuit( BOOL bChangeHair, BOOL bChangeFace )
{
	GASSERT(m_pd3dDevice);

	EMCHARINDEX emIndex = CharClassToIndex(m_emClass);
	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().FindData ( GLCONST_CHAR::szCharSkin[emIndex] );
	if ( !pSkinChar )	return E_FAIL;

	const GLCONST_CHARCLASS &sCONST = GLCONST_CHAR::cCONSTCLASS[emIndex];

	if ( bChangeFace )
	{
		// ��
		if ( sCONST.dwHEADNUM > m_wFace )
		{
			std::string strHEAD_CPS = sCONST.strHEAD_CPS[m_wFace];
		
			PDXCHARPART pCharPart = NULL;
			pCharPart = m_pSkinChar->GetPiece(PIECE_HEAD);		//	���� ���� ��Ų.

			if ( pCharPart && strcmp(strHEAD_CPS.c_str(),pCharPart->m_szFileName) )
			{
				m_pSkinChar->SetPiece ( strHEAD_CPS.c_str(), m_pd3dDevice, 0x0, 0, TRUE );
			}
		}
	}
	// ��Ÿ��
	if( bChangeHair )
	{
		if ( sCONST.dwHAIRNUM > m_wHair )
		{
			std::string strHAIR_CPS = sCONST.strHAIR_CPS[m_wHair];
		
			PDXCHARPART pCharPart = NULL;
			pCharPart = m_pSkinChar->GetPiece(PIECE_HAIR);		//	���� ���� ��Ų.

			if ( pCharPart && strcmp(strHAIR_CPS.c_str(),pCharPart->m_szFileName) )
			{
				m_pSkinChar->SetPiece ( strHAIR_CPS.c_str(), m_pd3dDevice, 0x0, 0, TRUE );
			}
		}
	}

	m_pSkinChar->SetHairColor( m_wHairColor );

	// ������ ������ ������Ʈ
	for ( int i=0; i<SLOT_NSIZE_S_2; i++ )
	{	
		// ���� �������� ���Ⱑ �ƴϸ� �Ѿ��.
		if( !IsCurUseArm( EMSLOT(i) ) ) continue;

		SITEMCUSTOM ItemCustom = m_PutOnItems[i];

		/*ABL system, Juver, 2017/06/02 */
		if ( i == SLOT_LHAND  )	ItemCustom = m_PutOnItems[SLOT_RHAND];
		if ( i == SLOT_LHAND_S  )	ItemCustom = m_PutOnItems[SLOT_RHAND_S];

		// ���� ������
		SNATIVEID nidITEM = ItemCustom.nidDISGUISE;
		if ( nidITEM==SNATIVEID(false) )	nidITEM = ItemCustom.sNativeID;
		
		if ( !m_bVehicle && i == SLOT_VEHICLE  ) nidITEM = SNATIVEID(false);
		else if (  m_bVehicle && i == SLOT_VEHICLE )
		{
			nidITEM = m_sVehicle.GetSkinID();
			if ( nidITEM == SNATIVEID(false) )
			{
				m_sVehicle.m_sVehicleID = ItemCustom.sNativeID;
				nidITEM = ItemCustom.sNativeID;
			}
		}

		// ��ȿ���� ������
		if ( nidITEM == SNATIVEID(false) )
		{
			//	Note : �⺻ ��Ų�� ���� ������ ��Ų�� Ʋ�� ���.
			//	SLOT->PIECE.
			PDXSKINPIECE pSkinPiece = NULL;	//	�⺻ ��Ų.
			PDXCHARPART pCharPart = NULL;	//	���� ���� ��Ų.

			/*ABL system, Juver, 2017/06/02 */
			DxAttBoneLink* pBoneLink = NULL;
			DxAttBone* pAttBone = NULL;

			EMPIECECHAR emPiece = SLOT_2_PIECE(EMSLOT(i));
			if ( emPiece!=PIECE_SIZE )
			{
				pSkinPiece = pSkinChar->GetPiece(emPiece);	//	�⺻ ��Ų.
				pCharPart = m_pSkinChar->GetPiece(emPiece);	//	���� ���� ��Ų.

				/*ABL system, Juver, 2017/06/02 */
				pBoneLink = pSkinChar->GetAttBone(emPiece);
				pAttBone = m_pSkinChar->GetAttBone(emPiece);
			}

			/*ABL system, Juver, 2017/06/02 */
			if ( pBoneLink )
			{
				if ( pAttBone && strcmp( pBoneLink->GetFileName(), pAttBone->GetFileName() ) )
				{
					m_pSkinChar->SetAttBone( pBoneLink, m_pd3dDevice, 0X0, 0, TRUE );
				}
				else if ( !pAttBone )
				{
					m_pSkinChar->SetAttBone( pBoneLink, m_pd3dDevice, 0X0, 0, TRUE );
				}
			}
			else if ( pSkinPiece )
			{
				if ( pCharPart && strcmp(pSkinPiece->m_szFileName,pCharPart->m_szFileName) )
					m_pSkinChar->SetPiece ( pSkinPiece->m_szFileName, m_pd3dDevice, 0X0, 0, TRUE );
			}
			else
			{
				if ( pCharPart )	m_pSkinChar->ResetPiece(emPiece);
			}

			/*upgrade effect, Juver, 2017/09/01 */
			EMPIECECHAR piece_reset = SLOT_2_UPGRADEPIECE( EMSLOT(i) );
			if ( piece_reset != PIECE_SIZE )	
				m_pSkinChar->ResetPiece( piece_reset );
		}
		// ��ȿ�ϸ�
		else
		{
			SITEM* pItem = GLItemMan::GetInstance().GetItem ( nidITEM );
			if ( pItem ) 
			{
				/*Item Self Effect, Juver, 2017/06/09 */
				std::string strSelfEffect = pItem->GetSelfBodyEffect();

				/*ABL system, Juver, 2017/06/02 */
				if ( pItem->sBasicOp.emItemType != ITEM_VEHICLE )
				{
					std::string strFileName = pItem->GetWearingFileR( emIndex );
					if ( i == SLOT_LHAND || i == SLOT_LHAND_S )	strFileName = pItem->GetWearingFileL( emIndex );

					DxAttBoneLink* pBoneLink = DxAttBoneLinkContainer::GetInstance().LoadData( strFileName.c_str(), m_pd3dDevice, TRUE );
					if ( pBoneLink )	m_pSkinChar->SetAttBone( pBoneLink, m_pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE, strSelfEffect.c_str() );
					else	m_pSkinChar->SetPiece ( strFileName.c_str(), m_pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE, strSelfEffect.c_str() );
				
					/*item color, Juver, 2018/01/10 */
					EMPIECECHAR piece_color = SLOT_2_PIECE(EMSLOT(i));
					if ( piece_color != PIECE_SIZE )
					{
						m_pSkinChar->SetColor1( piece_color, ItemCustom.wColor1 );
						m_pSkinChar->SetColor2( piece_color, ItemCustom.wColor2 );
					}
					
					/*upgrade effect, Juver, 2017/09/01 */
					BOOL bResetUpgradeEffect = TRUE;
					if( ItemCustom.GETGRADE_EFFECT() >= 6 && ( i == SLOT_UPPER || i == SLOT_LOWER || i == SLOT_HAND || i == SLOT_FOOT ) )
					{
						EMCHARGENDER emGender = CharClassToGender( m_emClass );
						std::string strPartName[4] = { "body", "leg", "hand", "foot" };
						std::string strGender[2] = { "w", "m" };

						CString strUpgradeFileName;
						strUpgradeFileName.Format("%s_bs_%s_enchent+%d.cps", strGender[emGender].c_str(), strPartName[i-1].c_str(), ItemCustom.GETGRADE_EFFECT() );
						HRESULT hrSetUpgrade = m_pSkinChar->SetPiece ( strUpgradeFileName.GetString(), m_pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE );
						if ( hrSetUpgrade == S_OK )	bResetUpgradeEffect = FALSE;
					}

					if ( bResetUpgradeEffect )
					{
						EMPIECECHAR piece_reset = SLOT_2_UPGRADEPIECE( EMSLOT(i) );
						if ( piece_reset != PIECE_SIZE )	
							m_pSkinChar->ResetPiece( piece_reset );
					}
				}
				else
				{
					/*vehicle system, Juver, 2017/08/06 */
					DxVehicleData*	pVehicleData = DxVehicleDataContainer::GetInstance().LoadData( pItem->GetWearingFileR(emIndex), m_pd3dDevice, TRUE  );
					if ( pVehicleData )
					{
						m_pSkinChar->SetVehicle ( pVehicleData, m_pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE, strSelfEffect.c_str() );

						if ( m_pSkinChar->m_pVehicle )
						{
							/* vehicle no accessory, Juver, 2018/02/14 */
							if ( !pItem->sVehicle.bNoAcc )
							{	
								for( int j=0; j<VEHICLE_ACCETYPE_SIZE; j++ )
								{
									SITEM* pVehicleItem = GLItemMan::GetInstance().GetItem ( m_sVehicle.m_PutOnItems[j].sNativeID );
									if ( pVehicleItem )
										m_pSkinChar->m_pVehicle->SetPiece( pVehicleItem->GetVehicleWearingFile( pItem->sVehicle.emVehicleSubType ), m_pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE );
								}
							}
							
							/*bike color , Juver, 2017/11/15 */
							if ( pItem->sVehicle.emVehicleType == VEHICLE_TYPE_BIKE )
							{
								for ( int ncolor=0; ncolor<BIKE_COLOR_SLOT_PART_SIZE; ++ncolor )
								{
									EMPIECECHAR piece_color = BIKE_COLOR_SLOT_TO_PIECE( (BIKE_COLOR_SLOT_PART) ncolor );
									BOOL bColor1 = IS_BIKECOLOR_1( (BIKE_COLOR_SLOT_PART) ncolor );
									if ( bColor1 )
										m_pSkinChar->m_pVehicle->SetColor1( piece_color, m_sVehicle.m_wColor[ncolor] );
									else
										m_pSkinChar->m_pVehicle->SetColor2( piece_color, m_sVehicle.m_wColor[ncolor] );
								}
							}

							/* car, cart color, Juver, 2018/02/14 */
							if ( pItem->sVehicle.emVehicleType == VEHICLE_TYPE_CAR || pItem->sVehicle.emVehicleType == VEHICLE_TYPE_CART )
							{
								m_pSkinChar->m_pVehicle->SetColor1( PIECE_SIZE, m_sVehicle.m_wColor[BIKE_COLOR_SLOT_PART_A1] );
								m_pSkinChar->m_pVehicle->SetColor2( PIECE_SIZE, m_sVehicle.m_wColor[BIKE_COLOR_SLOT_PART_A2] );
							}
						}	
					}
					else
						m_pSkinChar->SetPiece ( pItem->GetWearingFileR(emIndex), m_pd3dDevice, NULL, ItemCustom.GETGRADE_EFFECT(), TRUE, strSelfEffect.c_str() );
				}
			}
		}
	}

	//	Note : ��ų ����, �����̻� ȿ�� ����.
	FACTEFF::ReNewEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT, m_sSTATEBLOWS, m_matTrans, m_vDir );

	if( GLCONST_CHAR::cCONSTCLASS[emIndex].strCLASS_EFFECT.size() != 0 )
	{
		DxEffcharDataMan::GetInstance().PutPassiveEffect ( m_pSkinChar, GLCONST_CHAR::cCONSTCLASS[emIndex].strCLASS_EFFECT.c_str(), &m_vDir );
	}

	if( m_dwTransfromSkill != SNATIVEID::ID_NULL )
	{
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_sSKILLFACT[m_dwTransfromSkill].sNATIVEID.wMainID, m_sSKILLFACT[m_dwTransfromSkill].sNATIVEID.wSubID );
		if( pSkill ) 
		{
			for( BYTE i = 0; i < SKILL::EMTRANSFORM_NSIZE; i++ )
			{
				if( m_wSex == 1 )
				{				
					m_pSkinChar->SetPiece ( pSkill->m_sSPECIAL_SKILL.strTransform_Man[i].c_str(), m_pd3dDevice, 0X0, 0, TRUE );
				}else{
					m_pSkinChar->SetPiece ( pSkill->m_sSPECIAL_SKILL.strTransform_Woman[i].c_str(), m_pd3dDevice, 0X0, 0, TRUE );
				}
			}
		}
	}

	return S_OK;
}

WORD GLCharacter::GetBodyRadius ()
{
	return GETBODYRADIUS();
}

HRESULT GLCharacter::SetPosition ( D3DXVECTOR3 vPos )
{
	m_vPos = vPos;
	m_actorMove.SetPosition ( m_vPos, -1 );
	if ( m_actorMove.PathIsActive() )		m_actorMove.Stop();

	return S_OK;
}

EMCONFT_TYPE GLCharacter::GetConftType ( DWORD dwID )
{
	PGLCHARCLIENT pChar = GLGaeaClient::GetInstance().GetChar ( dwID );
	if ( !pChar )		return EMCONFT_NONE;

	if ( IsClubMaster() && pChar->IsClubMaster() )			return EMCONFT_GUILD;
	else if ( IsPartyMaster() && pChar->IsPartyMaster() )	return EMCONFT_PARTY;
	else													return EMCONFT_ONE;

	return EMCONFT_NONE;
}

void GLCharacter::DoPASSIVITY ( BOOL bPASSIVITY )
{
	if ( m_bPASSIVITY_ACTION_CHECK )	return;

	m_bPASSIVITY_ACTION = bPASSIVITY;
	m_fPASSIVITY_ACTION_ELAPS = 0.0f;

	if ( !bPASSIVITY )
	{
		ResetPASSIVITY ();
	}
}

enum
{
	EMPASSIVITY_ACTION_TOTAL		= 3600,
	EMPASSIVITY_ACTION_ELAPS		= 60,
	EMPASSIVITY_ACTION_CHECK		= 30,
	EMPASSIVITY_ACTION_CHECK_RESET	= 360,
};

bool GLCharacter::CheckPASSIVITY ( float fElaps )
{
	if ( m_bPASSIVITY_ACTION_CHECK )
	{
		m_fPASSIVITY_ACTION_CHECK += fElaps;
		return true;
	}

	m_fPASSIVITY_ACTION_ELAPS += fElaps;
	m_fPASSIVITY_ACTION_TOTAL += fElaps;

	if ( m_fPASSIVITY_ACTION_ELAPS > EMPASSIVITY_ACTION_CHECK_RESET )
	{
		ResetPASSIVITY ();
	}

//	CDebugSet::ToView ( 4, 1, "action total : %5.1f", m_fPASSIVITY_ACTION_TOTAL );
//	CDebugSet::ToView ( 4, 2, "action elaps : %5.1f", m_fPASSIVITY_ACTION_ELAPS );

	if ( m_fPASSIVITY_ACTION_TOTAL > EMPASSIVITY_ACTION_TOTAL &&
		m_fPASSIVITY_ACTION_ELAPS < EMPASSIVITY_ACTION_ELAPS )
	{
		m_bPASSIVITY_ACTION_CHECK = TRUE;
		CDebugSet::ToListView ( "[___CheckPASSIVITY___]" );
		return true;
	}

	return false;
}

bool GLCharacter::IsBlockPASSIVITY ()
{
	return ( m_bPASSIVITY_ACTION_CHECK && m_fPASSIVITY_ACTION_CHECK>EMPASSIVITY_ACTION_CHECK );
}

void GLCharacter::ResetPASSIVITY ()
{
	m_bPASSIVITY_ACTION = FALSE;
	m_fPASSIVITY_ACTION_ELAPS = 0.0f;
	m_fPASSIVITY_ACTION_TOTAL = 0.0f;
	m_bPASSIVITY_ACTION_CHECK = FALSE;
	m_fPASSIVITY_ACTION_CHECK = 0.0f;
}

BOOL GLCharacter::ActionMoveTo ( float fTime, D3DXVECTOR3 vFromPt, D3DXVECTOR3 vTargetPt, BOOL bContinue, BOOL bREACT )
{
	// ������ ������ ���¿��� �̵��� �� ����.
	if ( m_sPMarket.IsOpen() ) return FALSE;

	if ( m_bSTATE_STUN )	   return FALSE;

	BOOL bColl=FALSE;
	DWORD dwCollID;
	D3DXVECTOR3 vCollPos;
	if ( m_actorMove.GetParentMesh() )
	{
		m_actorMove.GetParentMesh()->IsCollision ( vFromPt, vTargetPt, vCollPos, dwCollID, bColl );
	}
	if ( !bColl )	return FALSE;
	
	//	�̵��ϰ��� �ϴ� ������ ���� �ִ� ������ ��ĥ ��� �̵� �Ұ�.
	bColl = GLGaeaClient::GetInstance().IsCollisionMobToPoint ( vCollPos, GETBODYRADIUS() );
	if ( bColl )	return FALSE;

	BOOL bGoto = m_actorMove.GotoLocation
	(
		vCollPos+D3DXVECTOR3(0,+10,0),
		vCollPos+D3DXVECTOR3(0,-10,0)
	);

	if ( bGoto )
	{
		TurnAction ( GLAT_MOVE );
		m_sTargetID.vPos = vCollPos;

		//	Note : �̵� �޽��� ����.
		GLMSG::SNETPC_GOTO NetMsg;
		NetMsg.dwActState = m_dwActState;
		NetMsg.vCurPos = m_vPos;
		NetMsg.vTarPos = m_sTargetID.vPos;

		m_sLastMsgMove = NetMsg;

		if ( RANPARAM::bClickEffect )
		{
			STARGETID sTargetID( CROW_PC, m_dwGaeaID, vCollPos );
			D3DXMATRIX matTrans;
			D3DXMatrixTranslation ( &matTrans, vCollPos.x, vCollPos.y, vCollPos.z );
			DxEffExtra::GetInstance().NewClickEff ( GLCONST_CHAR::strCLICK_EFFECT.c_str(), matTrans, &sTargetID );
		}

		//	Note : �޼��� �߻�. ( ���������� �߻��ϴ� �̵� ��Ʈ���� �Ϻθ� �޽��� �߻�. )
		//
		if ( bContinue && (m_fLastMsgMoveSend+m_fELAPS_MOVE) > fTime )		return TRUE;

		DoPASSIVITY ( bREACT );

		if ( m_actorMove.PathIsActive() )
		{
			m_fLastMsgMoveSend = fTime;
			m_sLastMsgMoveSend = NetMsg;
			NETSENDTOFIELD ( &NetMsg );
		}
	}

	return TRUE;
}

BOOL GLCharacter::LargeMapMoveTo( D3DXVECTOR3* pPos )
{
	if ( m_dwUserLvl >= USER_GM3 )
	{
		int nPosX(0), nPosY(0);
		PLANDMANCLIENT pLandClient = GLGaeaClient::GetInstance().GetActiveMap();
		if ( pLandClient )
		{
			const GLMapAxisInfo &sMapAxisInfo = pLandClient->GetMapAxisInfo();
			sMapAxisInfo.Convert2MapPos ( pPos->x, pPos->z, nPosX, nPosY );

			if (DxGlobalStage::GetInstance().IsEmulator())
			{
				GLMSG::SNET_GM_MOVE2MAPPOS_FLD NetMsgFld;
				NetMsgFld.nidMAP = pLandClient->GetMapID();
				NetMsgFld.dwPOSX = nPosX;
				NetMsgFld.dwPOSY = nPosY;
				NetMsgFld.dwGaeaID = 0;
				NETSENDTOFIELD(&NetMsgFld);
			}
			else
			{
				GLMSG::SNET_GM_MOVE2MAPPOS NetMsg;
				NetMsg.nidMAP = pLandClient->GetMapID();
				NetMsg.dwPOSX = nPosX;
				NetMsg.dwPOSY = nPosY;
				NETSEND(&NetMsg);
			}
		}
		return FALSE;
	}

	D3DXVECTOR3 vFrom = *pPos;
	vFrom.y += 1000.0f;

	D3DXVECTOR3 vTarget = *pPos;
	vTarget.y -= 1000.0f;

	return ActionMoveTo( 0.0f, vFrom, vTarget, FALSE, TRUE );
}

void GLCharacter::ResetAction() 
{ 
	TurnAction( GLAT_IDLE ); 
}

void GLCharacter::TurnAction ( EMACTIONTYPE toAction )
{
	//	Note : �� �׼� ����.
	//
	switch ( m_Action )
	{
	case GLAT_ATTACK:
		if ( toAction<=GLAT_TALK )
		{
			//	Note : ���� ���� �޼��� �߻�.
			GLMSG::SNETPC_ATTACK_CANCEL NetMsg;
			NETSENDTOFIELD ( &NetMsg );
		}
		break;

	case GLAT_SKILL:
		if ( toAction == GLAT_MOVE || toAction == GLAT_ATTACK || toAction == GLAT_SHOCK )
		{
			GLMSG::SNETPC_SKILL_CANCEL NetMsg;
			NETSENDTOFIELD ( &NetMsg );
		}

		m_idACTIVESKILL = NATIVEID_NULL();
		SetDefenseSkill( false );
		m_wTARNUM = 0;
		ResetHiddenWeaponPiece();
		break;

	case GLAT_FALLING:
		if ( toAction==GLAT_FALLING )	return;
		break;
		
	case GLAT_DIE:
		if ( toAction==GLAT_FALLING || toAction==GLAT_DIE )	return;
		break;

	case GLAT_CONFT_END:
		ReSetSTATE(EM_ACT_CONFT_WIN);
		break;
	case GLAT_GATHERING:
		ReqCancelGathering();
		break;
	};

	//	Note : �׼� �ʱ�ȭ.
	//

	m_Action = toAction;



	//	Note : �׼� �ٲ�.
	//
	switch ( m_Action )
	{
	case GLAT_IDLE:
		{
			m_fIdleTime = 0.0f;

			/*dash skill logic, Juver, 2017/06/17 */
			m_wActionAnim = AN_SUB_00_SIZE;

			/*dmk14 visual bug fix*/
			if ( m_actorMove.PathIsActive() )	m_actorMove.Stop();
		}
		break;

	case GLAT_ATTACK:
		{
			StartAttackProc ();

			/*dmk14 visual bug fix*/
			if ( m_actorMove.PathIsActive() )	m_actorMove.Stop();
		}
		break;

	case GLAT_SKILL:
		{
			m_pSkinChar->m_bResetSkillAni = TRUE;
			StartSkillProc ();
			SetHiddenWeaponPiece();

			/*dmk14 visual bug fix*/
			if ( m_actorMove.PathIsActive() )	m_actorMove.Stop();
		}
		break;

	case GLAT_TALK:
		{
		}
		break;

	case GLAT_FALLING:
		m_sHP.dwNow = 0;
	
		FACTEFF::DeleteEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT, m_sSTATEBLOWS );
		break;

	case GLAT_DIE:
		//	ä���� 0�� �ʱ�ȭ.
		//	( Ŭ���̾�Ʈ���� ȸ���� �������� �������� ä���� 0 �϶� 0�� �ƴҼ� ���� )
		m_sHP.dwNow = 0;
		
		FACTEFF::DeleteEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT, m_sSTATEBLOWS );
		break;

	case GLAT_CONFT_END:
		break;

	case GLAT_GATHERING:		
		break;

	default:
		GASSERT("GLCharacter::TurnAction() �غ���� ���� ACTION �� ���Խ��ϴ�.");
		break;
	};

	if ( m_actorMove.PathIsActive() )
	{
		/*push pull skill logic, Juver, 2017/06/05 */
		/*dash skill logic, Juver, 2017/06/17 */
		if ( !IsACTION(GLAT_MOVE) && !IsACTION(GLAT_PUSHPULL) && !IsACTION(GLAT_SKILLMOVE) && !IsACTION(GLAT_SKILLDASH) ) m_actorMove.Stop();
	}
}

BOOL GLCharacter::IsCtrlBlockBody ()
{
	if ( IsACTION(GLAT_SHOCK) )		return TRUE;
	if ( IsACTION(GLAT_PUSHPULL) )	return TRUE;
	if ( IsACTION(GLAT_SKILLMOVE) )	return TRUE; /*push pull skill logic, Juver, 2017/06/05 */
	if ( IsACTION(GLAT_SKILLDASH) )	return TRUE; /*dash skill logic, Juver, 2017/06/17 */
	if ( IsACTION(GLAT_CONFT_END) )	return TRUE;
	if ( IsACTION(GLAT_GATHERING) && ( m_dwANISUBGESTURE == 0 ||  m_dwANISUBGESTURE == 1 ) ) 
		return TRUE;

	return FALSE;
}

BOOL GLCharacter::IsValidBody ()
{
	if ( IsSTATE(EM_ACT_WAITING) )		return FALSE;
	if ( IsSTATE(EM_REQ_GATEOUT) )		return FALSE;
	if ( IsSTATE(EM_ACT_DIE) )			return FALSE;

	return ( m_Action < GLAT_FALLING );
}

BOOL GLCharacter::IsDie ()
{
	if ( !IsSTATE(EM_ACT_DIE) )
	{
		if ( IsACTION(GLAT_DIE) )		return TRUE;
	}

	return FALSE;
}

BOOL GLCharacter::IsPartyMaster ()
{
	GLPARTY_CLIENT* pParty = GLPartyClient::GetInstance().GetMaster();
	if ( pParty==NULL )		return FALSE;
	
	return ( pParty->m_dwGaeaID==m_dwGaeaID );
}

BOOL GLCharacter::IsPartyMem ()
{
	return GLPartyClient::GetInstance().GetMaster()!=NULL;
}

BOOL GLCharacter::IsClubMaster ()
{
	return m_sCLUB.m_dwMasterID==m_dwCharID;
}

BOOL GLCharacter::IsCDCertify ()
{
	return m_sCLUB.IsMemberFlgCDCertify(m_dwCharID);
}

void GLCharacter::P2PReaction ( const STARGETID &sTargetID, const DWORD dwML, const bool bcontinue, BOOL &bMove, D3DXVECTOR3 &vMoveTo )
{
	if ( m_bSTATE_STUN )				return;
	
	if ( dwML & (DXKEY_UP|DXKEY_DUP) )
	{
		//	Note : ������ ���.
		SREACTION sREC_TEMP = ( m_sREACTION.sTARID == sTargetID ) ? m_sREACTION : SREACTION();
		m_sREACTION.RESET();

		PLANDMANCLIENT pLandMan = GLGaeaClient::GetInstance().GetActiveMap();
		
		PGLCHARCLIENT pCharClient = pLandMan->GetChar ( sTargetID.dwID );
		if ( !pCharClient )		return;


		const D3DXVECTOR3 &vTarPos = pCharClient->GetPosition();

		float fMoveDist = FLT_MAX;
		float fDist = D3DXVec3Length ( &D3DXVECTOR3(m_vPos-vTarPos) );
		if ( IsACTION(GLAT_MOVE) )	fMoveDist = D3DXVec3Length ( &D3DXVECTOR3(m_sTargetID.vPos-vTarPos) );

		WORD wP2PRange = pCharClient->GetBodyRadius() + GETBODYRADIUS() + 25;
		WORD wP2PAbleDis = wP2PRange + 2;

		if ( fDist>wP2PAbleDis )
		{
			if ( fMoveDist > wP2PAbleDis )
			{
				//	Note : �̵����� ����.
				//
				D3DXVECTOR3 vDir = vTarPos - m_vPos;
				D3DXVec3Normalize ( &vDir, &vDir );

				bMove = TRUE;
				vMoveTo = m_vPos + vDir*wP2PRange;
				vMoveTo.y = vTarPos.y;

				//	Note : ������ ���.
				//
				m_sREACTION.emREACTION = REACT_P2P;
				m_sREACTION.sTARID = sTargetID;
				m_sREACTION.vMARK_POS = vMoveTo;
			}
			else
			{
				m_sREACTION = sREC_TEMP;
			}

			return;
		}

		//	Note : PtoP �޴�.
		//
		//bool bVALID_CLUB = m_sCLUB.m_dwID!=CLUB_NULL;
		//bool bMASTER = m_sCLUB.m_dwMasterID==m_dwCharID;

		if( !GLCONST_CHAR::bBATTLEROYAL )
		{
			CInnerInterface::GetInstance().SetPtoPWindowOpen( pCharClient, sTargetID.dwID, m_sCLUB );
		}
	}
}

void GLCharacter::PvPReaction ( const STARGETID &sTargetID, const DWORD dwML, const bool bcontinue, BOOL &bMove, D3DXVECTOR3 &vMoveTo )
{
	if ( m_bSTATE_STUN )	return;	
	if ( m_bReqVehicle ) return;
	if ( m_bVehicle ) return;

	// ���ݾִϸ��̼��� ��������
	if ( !m_bAttackable ) return;

	PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();
	if ( pLand && pLand->IsPeaceZone() )	return;

	//	Note : ������ ���.
	SREACTION sREC_TEMP = ( m_sREACTION.sTARID == sTargetID ) ? m_sREACTION : SREACTION();
	m_sREACTION.RESET();

	const D3DXVECTOR3 &vTarPos = sTargetID.vPos;

	float fMoveDist = FLT_MAX;
	float fDist = D3DXVec3Length ( &D3DXVECTOR3(m_vPos-vTarPos) );
	if ( IsACTION(GLAT_MOVE) )	fMoveDist = D3DXVec3Length ( &D3DXVECTOR3(m_sTargetID.vPos-vTarPos) );

	GLCOPY* pTarget = GLGaeaClient::GetInstance().GetCopyActor ( sTargetID );
	if ( !pTarget )		return;

	//	Note : ��Ÿ� ������ ��츸 ���ݰ��ɰŸ��� �����Ÿ� �ɼ� ���� ������.
	//
	WORD wAttackRange = pTarget->GetBodyRadius() + GETBODYRADIUS() + GETATTACKRANGE() + 2;
	if ( ISLONGRANGE_ARMS() )	 wAttackRange += (WORD) GETSUM_TARRANGE();
	WORD wAttackAbleDis = wAttackRange + 2;

	if ( fDist>wAttackAbleDis )
	{
		if ( fMoveDist > wAttackAbleDis )
		{
			//	Note : �̵����� ����.
			//
			D3DXVECTOR3 vPos = vTarPos - m_vPos;
			D3DXVec3Normalize ( &vPos, &vPos );

			bMove = TRUE;
			vMoveTo = vTarPos - vPos*wAttackRange;
			vMoveTo.y = vTarPos.y;

			//	Note : ������ ���.
			//
			m_sREACTION.emREACTION = REACT_PVP;
			m_sREACTION.bCONTINUE = bcontinue;
			m_sREACTION.sTARID = sTargetID;
			m_sREACTION.vMARK_POS = vMoveTo;
		}
		else
		{
			m_sREACTION = sREC_TEMP;
		}

		return;
	}

	//	������ �浹�ϴ��� �˻�.
	//	
	D3DXVECTOR3 vPos1 = GetPosition(); vPos1.y += 15.0f;
	D3DXVECTOR3 vPos2 = vTarPos; vPos2.y += 15.0f;
	
	BOOL bCollision;
	D3DXVECTOR3 vCollision;
	LPDXFRAME pDxFrame;

	DxLandMan *pLandMan = GLGaeaClient::GetInstance().GetActiveMap()->GetLandMan();
	pLandMan->IsCollision ( vPos1, vPos2, vCollision, bCollision, pDxFrame, FALSE );
	if ( bCollision )	return;

	PANIMCONTNODE pAnicont = m_pSkinChar->GETANI ( AN_ATTACK, GLCHARLOGIC::m_emANISUBTYPE );
	if ( !pAnicont )	return;

	WORD wStrikeNum = pAnicont->pAnimCont->m_wStrikeCount;

	//	Note : ���ݼ��� ���� �˻�.
	//    
	EMBEGINATTACK_FB emBeginFB = BEGIN_ATTACK(wStrikeNum);
	if ( emBeginFB==EMBEGINA_OK || emBeginFB==EMBEGINA_SP )
	{
		m_sTargetID.emCrow = sTargetID.emCrow;
		m_sTargetID.dwID = sTargetID.dwID;
		TurnAction ( GLAT_ATTACK );
	}

	if ( bcontinue )
	{
		m_sREACTION.emREACTION = REACT_PVP;
		m_sREACTION.bCONTINUE = true;
		m_sREACTION.sTARID = sTargetID;
		m_sREACTION.vMARK_POS = m_vPos;
	}

	return;
}

void GLCharacter::MobReaction ( const STARGETID &sTargetID, const DWORD dwML, const bool bcontinue, BOOL &bMove, D3DXVECTOR3 &vMoveTo )
{
	if ( m_bSTATE_STUN )	return;
	if ( m_bReqVehicle ) return;
	if ( m_bVehicle ) return;

	// ���ݾִϸ��̼��� ��������
	if ( !m_bAttackable ) return;

	m_sOLD_TARMOB = sTargetID;

	//	Note : ������ ���.
	SREACTION sREC_TEMP = ( m_sREACTION.sTARID == sTargetID ) ? m_sREACTION : SREACTION();
	m_sREACTION.RESET();

	const D3DXVECTOR3 &vTarPos = sTargetID.vPos;

	float fMoveDist = FLT_MAX;
	float fDist = D3DXVec3Length ( &D3DXVECTOR3(m_vPos-vTarPos) );
	if ( IsACTION(GLAT_MOVE) )	fMoveDist = D3DXVec3Length ( &D3DXVECTOR3(m_sTargetID.vPos-vTarPos) );

	GLCOPY* pTarget = GLGaeaClient::GetInstance().GetCopyActor ( sTargetID );
	if ( !pTarget )		return;

	//	Note : ��Ÿ� ������ ��츸 ���ݰ��ɰŸ��� �����Ÿ� �ɼ� ���� ������.
	//
	WORD wAttackRange = pTarget->GetBodyRadius() + GETBODYRADIUS() + GETATTACKRANGE() + 2;
	if ( ISLONGRANGE_ARMS() )	 wAttackRange += (WORD) GETSUM_TARRANGE();
	WORD wAttackAbleDis = wAttackRange + 2;

	if ( fDist>wAttackAbleDis )
	{
		if ( fMoveDist > wAttackAbleDis )
		{
			//	Note : �̵����� ����.
			//
			D3DXVECTOR3 vPos = vTarPos - m_vPos;
			D3DXVec3Normalize ( &vPos, &vPos );

			bMove = TRUE;
			vMoveTo = vTarPos - vPos*wAttackRange;
			vMoveTo.y = vTarPos.y;

			//	Note : ������ ���.
			//
			m_sREACTION.emREACTION = REACT_MOB;
			m_sREACTION.bCONTINUE = bcontinue;
			m_sREACTION.sTARID = sTargetID;
			m_sREACTION.vMARK_POS = vMoveTo;
		}
		else
		{
			m_sREACTION = sREC_TEMP;
		}

		return;
	}

	//	������ �浹�ϴ��� �˻�.
	//	
	D3DXVECTOR3 vPos1 = GetPosition(); vPos1.y += 15.0f;
	D3DXVECTOR3 vPos2 = vTarPos; vPos2.y += 15.0f;
	
	BOOL bCollision;
	D3DXVECTOR3 vCollision;
	LPDXFRAME pDxFrame;

	DxLandMan *pLandMan = GLGaeaClient::GetInstance().GetActiveMap()->GetLandMan();
	pLandMan->IsCollision ( vPos1, vPos2, vCollision, bCollision, pDxFrame, FALSE );
	if ( bCollision )	return;

	PANIMCONTNODE pAnicont = m_pSkinChar->GETANI ( AN_ATTACK, GLCHARLOGIC::m_emANISUBTYPE );
	if ( !pAnicont )	return;

	WORD wStrikeNum = pAnicont->pAnimCont->m_wStrikeCount;

	//	Note : ���ݼ��� ���� �˻�.
	//
	EMBEGINATTACK_FB emBeginFB = BEGIN_ATTACK(wStrikeNum);
	if ( emBeginFB==EMBEGINA_OK || emBeginFB==EMBEGINA_SP )
	{
		m_sTargetID.emCrow = sTargetID.emCrow;
		m_sTargetID.dwID = sTargetID.dwID;
		TurnAction ( GLAT_ATTACK );
	}

	if ( bcontinue )
	{
		m_sREACTION.bCONTINUE = true;
		m_sREACTION.emREACTION = REACT_MOB;
		m_sREACTION.sTARID = sTargetID;
		m_sREACTION.vMARK_POS = m_vPos;
	}

	return;
}

// *****************************************************
// Desc: NPC�� ���õ� ����� �׼��� ó���Ѵ�.
// *****************************************************
void GLCharacter::NpcReaction ( const STARGETID &sTargetID, const DWORD dwML, const bool bcontinue, BOOL &bMove, D3DXVECTOR3 &vMoveTo )
{
	if ( m_bSTATE_STUN )	return;
	if ( m_bReqVehicle ) return;
	if ( m_bVehicle ) return;

	//	Note : ������ ���.
	SREACTION sREC_TEMP = ( m_sREACTION.sTARID == sTargetID ) ? m_sREACTION : SREACTION();
	m_sREACTION.RESET();

	// ���õ� NPC �ʱ�ȭ
	m_dwNPCID = 0;

	const D3DXVECTOR3 &vTarPos = sTargetID.vPos;
	// �ʿ���� �ڵ�
	D3DXVECTOR3 vDistance = m_vPos - vTarPos;
	float fDistance = D3DXVec3Length ( &vDistance );

	// NPC���� �Ÿ�
	float fMoveDist = FLT_MAX;
	float fDist = D3DXVec3Length ( &D3DXVECTOR3(m_vPos-vTarPos) );
	if ( IsACTION(GLAT_MOVE) )	fMoveDist = D3DXVec3Length ( &D3DXVECTOR3(m_sTargetID.vPos-vTarPos) );

	GLCOPY* pTarget = GLGaeaClient::GetInstance().GetCopyActor ( sTargetID );
	if ( !pTarget )	return;

	// ��ȭ�� ������ ����
	WORD wTakeRange = pTarget->GetBodyRadius() + GETBODYRADIUS() + 30;
	WORD wTakeAbleDis = wTakeRange + 2;

	// ���� ��ġ�� ��ȭ�� �Ұ��� �ϸ�
	if ( fDist>wTakeAbleDis )
	{
		// �̵��� �������� ��ȭ�� �Ұ��� �ϸ� ������ ��ġ�� �̵��ϴ� ����� �׼��� ���
		if ( fMoveDist > wTakeAbleDis )
		{
			//	Note : �̵����� ����.
			//
			D3DXVECTOR3 vPos = vTarPos - m_vPos;
			D3DXVec3Normalize ( &vPos, &vPos );

			bMove = TRUE; 
			vMoveTo = vTarPos - vPos*wTakeRange;
			vMoveTo.y = vTarPos.y;

			//	Note : ������ ���.
			//
			m_sREACTION.emREACTION = REACT_NPC;
			m_sREACTION.sTARID = sTargetID;
			m_sREACTION.vMARK_POS = vMoveTo;
		}
		// �̵��� �������� ��ȭ�� �����ϸ� �׳� ���� ����׼��� �׷��� ���
		else
		{
			m_sREACTION = sREC_TEMP;
		}

		return;
	}

	// ���� ����� NPC�� ������� NPC�� ��ġ Ȯ��
	if ( dwML&(DXKEY_UP|DXKEY_DUP) )
	{
		PGLCROWCLIENT pCrow = GLGaeaClient::GetInstance().GetActiveMap()->GetCrow ( sTargetID.dwID );
		if ( !pCrow )		return;
		PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( pCrow->m_sNativeID );
		if ( !pCrowData )	return;

		m_dwNPCID   = sTargetID.dwID;

		/*pvp capture the flag, Juver, 2018/02/03 */
		if ( pCrowData->m_emCTFType != EMNPC_CTF_FLAG_TYPE_NONE )
			GLPVPCaptureTheFlagClient::GetInstance().FlagActionCapture( m_dwCharID, pCrowData->m_emCTFType, pCrow->m_dwGlobID );
		else
			CInnerInterface::GetInstance().SetDialogueWindowOpen ( pCrow->m_sNativeID,m_dwNPCID, this );
	}
}

// *****************************************************
// Desc: ä��NPC�� ���õ� ����� �׼��� ó���Ѵ�.
// *****************************************************
void GLCharacter::GatheringReaction ( const STARGETID &sTargetID, const DWORD dwML, const bool bcontinue, BOOL &bMove, D3DXVECTOR3 &vMoveTo )
{
	if ( m_bSTATE_STUN )	return;
	if ( m_bReqVehicle ) return;
	if ( m_bVehicle ) return;

	//	Note : ������ ���.
	SREACTION sREC_TEMP = ( m_sREACTION.sTARID == sTargetID ) ? m_sREACTION : SREACTION();
	m_sREACTION.RESET();

	// ���õ� NPC �ʱ�ȭ
	m_dwNPCID = 0;

	const D3DXVECTOR3 &vTarPos = sTargetID.vPos;
	// �ʿ���� �ڵ�
	D3DXVECTOR3 vDistance = m_vPos - vTarPos;
	float fDistance = D3DXVec3Length ( &vDistance );

	// NPC���� �Ÿ�
	float fMoveDist = FLT_MAX;
	float fDist = D3DXVec3Length ( &D3DXVECTOR3(m_vPos-vTarPos) );
	if ( IsACTION(GLAT_MOVE) )	fMoveDist = D3DXVec3Length ( &D3DXVECTOR3(m_sTargetID.vPos-vTarPos) );

	GLCOPY* pTarget = GLGaeaClient::GetInstance().GetCopyActor ( sTargetID );
	if ( !pTarget )	return;

	// ��ȭ�� ������ ����
	WORD wCollectionRange = pTarget->GetBodyRadius() + GETBODYRADIUS() + 2;
	WORD wCollectionAbleDis = wCollectionRange + 10;

	// ���� ��ġ�� ��ȭ�� �Ұ��� �ϸ�
	if ( fDist>wCollectionAbleDis )
	{
		// �̵��� �������� ��ȭ�� �Ұ��� �ϸ� ������ ��ġ�� �̵��ϴ� ����� �׼��� ���
		if ( fMoveDist > wCollectionAbleDis )
		{
			//	Note : �̵����� ����.
			//
			D3DXVECTOR3 vPos = vTarPos - m_vPos;
			D3DXVec3Normalize ( &vPos, &vPos );

			bMove = TRUE; 
			vMoveTo = vTarPos - vPos*wCollectionRange;
			vMoveTo.y = vTarPos.y;

			//	Note : ������ ���.
			//
			m_sREACTION.emREACTION = REACT_GATHERING;
			m_sREACTION.sTARID = sTargetID;
			m_sREACTION.vMARK_POS = vMoveTo;
		}
		// �̵��� �������� ��ȭ�� �����ϸ� �׳� ���� ����׼��� �׷��� ���
		else
		{
			m_sREACTION = sREC_TEMP;
		}

		return;
	}

	// ���� ����� NPC�� ������� NPC�� ��ġ Ȯ��
	if ( dwML&(DXKEY_UP|DXKEY_DUP) )
	{
		PGLMATERIALCLIENT pMaterial = GLGaeaClient::GetInstance().GetActiveMap()->GetMaterial ( sTargetID.dwID );
		if ( !pMaterial )		return;
		PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( pMaterial->m_sNativeID );
		if ( !pCrowData )	return;
		if ( pCrowData->m_emCrow != CROW_MATERIAL ) return;

		// ä�� ����
		ReqGathering( sTargetID );
	}
}

void GLCharacter::ItemReaction ( const STARGETID &sTargetID, const DWORD dwML, const bool bcontinue, BOOL &bMove, D3DXVECTOR3 &vMoveTo )
{
	if ( m_bSTATE_STUN )	return;
	if ( m_bReqVehicle ) return;
	if ( m_bVehicle ) return;

	//	Note : ������ ���.
	SREACTION sREC_TEMP = ( m_sREACTION.sTARID == sTargetID ) ? m_sREACTION : SREACTION();
	m_sREACTION.RESET();

	const D3DXVECTOR3 &vTarPos = sTargetID.vPos;

	D3DXVECTOR3 vDistance = m_vPos - vTarPos;
	float fDistance = D3DXVec3Length ( &vDistance );

	WORD wTarBodyRadius = 4;
	WORD wTakeRange = wTarBodyRadius + GETBODYRADIUS() + 2;
	WORD wTakeAbleDis = wTakeRange + 12;

	if ( fDistance>wTakeAbleDis )
	{
		//	Note : �̵����� ����.
		//
		D3DXVECTOR3 vPos = vTarPos - m_vPos;
		D3DXVec3Normalize ( &vPos, &vPos );

		bMove = TRUE; 
		vMoveTo = vTarPos - vPos*wTakeRange;

		//	Note : ������ ���.
		//
		m_sREACTION.emREACTION = REACT_ITEM;
		m_sREACTION.sTARID = sTargetID;
		m_sREACTION.vMARK_POS = vMoveTo;

		return;
	}
	
	if ( dwML&(DXKEY_UP|DXKEY_DUP) )
	{
		//	�ݱ� �õ�.
		ReqFieldTo ( sTargetID );
	}
}

void GLCharacter::ReservedAction ( float fTime )
{
    if ( !m_sREACTION.ISVALID() )									return;
	if ( m_actorMove.PathIsActive() || !IsACTION(GLAT_IDLE) )		return;
	if ( !GLGaeaClient::GetInstance().GetActiveMap() )				return;
	if ( m_bVehicle )												return;
	
	if ( m_bSTATE_STUN )
	{
		/*dmk14 visual bug fix*/
		m_actorMove.Stop ();

		m_sREACTION.RESET();
		return;
	}


	PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();

	//	Note : ��ǥ ��ġ�� ��Ȯ���� �˻�.
	//
	D3DXVECTOR3 vCURPOS = m_actorMove.Position();
	D3DXVECTOR3 vDIR =  m_sREACTION.vMARK_POS - vCURPOS;
	float fLength = D3DXVec3Length ( &vDIR );
	if ( fLength > 10.0f )
	{
		m_sREACTION.RESET();
		return;
	}

	//	Note : �������ڰ� ��Ȯ���� �˻�.
	//
	BOOL bTARID(TRUE);
	STARGETID sTARID = m_sREACTION.sTARID;
	switch ( m_sREACTION.emREACTION )
	{
	case REACT_MOB:
	case REACT_NPC:
	case REACT_P2P:
	case REACT_PVP:
	case REACT_GATHERING:
		{
			GLCOPY* pTarget = GLGaeaClient::GetInstance().GetCopyActor ( sTARID );
 			if ( !pTarget || pTarget->IsACTION(GLAT_FALLING) || pTarget->IsACTION(GLAT_DIE) )	bTARID = FALSE;
		}
		break;

	case REACT_ITEM:
		switch ( sTARID.emCrow )
		{
		case CROW_ITEM:
			if ( !pLand->GetItem(sTARID.dwID) )										bTARID = FALSE;
			break;
		case CROW_MONEY:
			if ( !pLand->GetMoney(sTARID.dwID) )									bTARID = FALSE;
			break;
		//case CROW_PET:	break;	// PetData
		default:
			bTARID = FALSE;
			break;
		};
		break;
	};

	if ( !bTARID )
	{
		m_sREACTION.RESET();
		return;
	}

	//	���������� ���� ��ġ.
	m_sREACTION.sTARID.vPos = GLGaeaClient::GetInstance().GetTargetPos ( sTARID );

	BOOL bMove(FALSE);
	D3DXVECTOR3 vMoveTo(0,0,0);
	bool bcontinue = m_sREACTION.bCONTINUE;
	switch ( m_sREACTION.emREACTION )
	{
	case REACT_P2P:			P2PReaction(m_sREACTION.sTARID,DXKEY_UP,bcontinue,bMove,vMoveTo);		break;
	case REACT_PVP:			PvPReaction(m_sREACTION.sTARID,DXKEY_UP,bcontinue,bMove,vMoveTo);		break;

	case REACT_MOB:			MobReaction(m_sREACTION.sTARID,DXKEY_UP,bcontinue,bMove,vMoveTo);		break;
	case REACT_NPC:			NpcReaction(m_sREACTION.sTARID,DXKEY_UP,bcontinue,bMove,vMoveTo);		break;
	case REACT_ITEM:		ItemReaction(m_sREACTION.sTARID,DXKEY_UP,bcontinue,bMove,vMoveTo);		break;
	case REACT_SKILL:		SkillReaction(m_sREACTION.sTARID,DXKEY_UP,bcontinue,bMove,vMoveTo);		break;
	case REACT_GATHERING:	GatheringReaction(m_sREACTION.sTARID,DXKEY_UP,bcontinue,bMove,vMoveTo);	break;
	};

	//	Note : Reaction ���� �̵��� ��û�� ���.
	//
	if ( bMove )
	{
		ActionMoveTo ( fTime, vMoveTo+D3DXVECTOR3(0,+5,0), vMoveTo+D3DXVECTOR3(0,-5,0), FALSE, TRUE );
	}
}

STARGETID* GLCharacter::GetCONFT_TAR ( DETECTMAP_RANGE &pair )
{
	if ( !m_sCONFTING.IsFIGHTING() )	return NULL;

	switch ( m_sCONFTING.emTYPE )
	{
	case EMCONFT_ONE:
		{
			//	Note : ����ڰ� �ɷ��� ��� �̸� �켱��.
			STARGETID sCONFT_TARID(CROW_PC,m_sCONFTING.dwTAR_ID);
			STARGETID *pTARID = FindCrow ( pair, sCONFT_TARID );
			if ( pTARID )	return pTARID;
		}
		break;

	case EMCONFT_PARTY:
		{
			STARGETID *pTARID = FindCrow ( pair, m_sCONFTING.setConftMember );
			if ( pTARID )	return pTARID;
		}
		break;

	case EMCONFT_GUILD:
		{
			STARGETID *pTARID = FindCrowCID ( pair, m_sCONFTING.setConftMember );
			if ( pTARID )	return pTARID;
		}
		break;
	};

	return NULL;
}

bool GLCharacter::IsPK_TAR ( PGLCHARCLIENT pCHAR, bool bFORCED )
{
	//	Note : PK �� ���.
	PLANDMANCLIENT pLAND = GLGaeaClient::GetInstance().GetActiveMap ();
	if ( !pLAND )				return false;
	if ( !pLAND->IsPKZone() )	return false;

	if ( m_dwUserLvl >= USER_GM3 ) return false;

	if ( pCHAR->IsSafeZone() )	
	{
		return false;
	}
	if ( pCHAR->IsSafeTime() )	
	{
		return false;
	}
	if ( IsSafeTime () )		
	{
		return false;
	}
	if ( IsSafeZone () )		
	{
		return false;
	}

	SDROP_CHAR &sCHARDATA_TAR = pCHAR->GetCharData();

	//	��Ƽ�� ���� � ���� ���������� ���� ����.
	GLPARTY_CLIENT* pPARTY_CLT = GLPartyClient::GetInstance().FindMember(sCHARDATA_TAR.dwGaeaID);
	if ( pPARTY_CLT )			return false;

	//	����������.
	bool bHOSTILE = IS_PLAYHOSTILE ( sCHARDATA_TAR.dwCharID );
	if ( bHOSTILE )																		return true;

	//	����Ŭ���� Ÿ��.
	if ( pLAND->m_bClubBattle )
	{
		if ( m_sCLUB.IsAllianceGuild(sCHARDATA_TAR.dwGuild) ) return false;
		else												  return true;
	}

	if ( pLAND->m_bClubDeathMatch && GLGaeaClient::GetInstance().IsClubDeathMatch() )
	{
		if ( m_sCLUB.m_dwID != sCHARDATA_TAR.dwGuild ) 
			return true;
	}

	/*pvp tyranny, Juver, 2017/08/24 */
	if ( pLAND->m_bPVPTyrannyMap && GLPVPTyrannyClient::GetInstance().IsBattle() )
	{
		if ( m_wSchool!=sCHARDATA_TAR.wSchool )	return true;
		else return false;
	}

	/*school wars, Juver, 2018/01/19 */
	if ( pLAND->m_bPVPSchoolWarsMap && GLPVPSchoolWarsClient::GetInstance().IsBattle() )
	{
		if ( m_wSchool!=sCHARDATA_TAR.wSchool )	return true;
		else return false;
	}

	/*pvp capture the flag, Juver, 2018/01/31 */
	if ( pLAND->m_bPVPCaptureTheFlagMap && GLPVPCaptureTheFlagClient::GetInstance().IsBattle() )
	{
		if ( m_wCaptureTheFlagTeam!=sCHARDATA_TAR.m_wCaptureTheFlagTeam )	return true;
		else return false;
	}

	if ( pLAND->IsPKMap() && pLAND->IsPKZone() ) return true;

	//	���� PK.
	if ( bFORCED )																	
	{
		if ( sCHARDATA_TAR.wLevel <= GLCONST_CHAR::nPK_LIMIT_LEVEL )
			return false;
		else
			return true;
	}

	//	�б��� ���� PK.
	if ( !GLCONST_CHAR::bSCHOOL_FREE_PK_Z_FORCED ) 
	{
		bool bSCHOOL_FREEPK = GLGaeaClient::GetInstance().IsSchoolFreePk ();
		if ( bSCHOOL_FREEPK && m_wSchool!=sCHARDATA_TAR.wSchool )					return true;
	}

	// ���� �̺�Ʈ�ϰ��.
	if ( GLGaeaClient::GetInstance().IsBRIGHTEVENT() )								return true;

	// Ŭ�� ��Ʋ���ϰ��
	if ( ( m_sCLUB.IsBattle(pCHAR->GETCLUBID()) || m_sCLUB.IsBattleAlliance(pCHAR->GETALLIANCEID()) ) 
		&& pLAND->IsClubBattleZone() )		
		return true;

	return false;
}

bool GLCharacter::IsClubBattle_TAR ( GLCharClient* pCHAR )
{
	//	Note : PK �� ���.
	PLANDMANCLIENT pLAND = GLGaeaClient::GetInstance().GetActiveMap ();
	if ( !pLAND )				return false;

	// Ŭ�� ��Ʋ���ϰ��
	if ( ( m_sCLUB.IsBattle(pCHAR->GETCLUBID()) || m_sCLUB.IsBattleAlliance( pCHAR->GETALLIANCEID()) )
		&& pLAND->IsClubBattleZone() )
		return true;

	return false;
}

int GLCharacter::GetClubColorIndex( GLCharClient* pCHAR )
{
	int nIndex = 0;
	
	if ( !pCHAR ) return nIndex;

	CLUB_BATTLE_ITER pos = m_sCLUB.m_mapBattle.begin();
	CLUB_BATTLE_ITER pos_end = m_sCLUB.m_mapBattle.end();

	for ( ; pos != pos_end; ++pos )
	{
		if ( pCHAR->GETCLUBID() == (*pos).first ) break;
		if ( pCHAR->GETALLIANCEID() == (*pos).first ) break;
		nIndex++;
	}

	nIndex = nIndex % NS_UITEXTCOLOR::MAX_CLUB_BATTLE_COLOR;

	return nIndex;
}

SACTIONTAR GLCharacter::FindActionTarget ( const D3DXVECTOR3 &vTargetPt, const D3DXVECTOR3 &vFromPt, const EMFIND_TAR emFINDTAR )
{
	SACTIONTAR sACTAR;
	if ( IsSTATE(EM_ACT_CONTINUEMOVE) )		return sACTAR;

	//	Note : ����� �Է°�.
	//
	DxInputDevice &dxInputDev = DxInputDevice::GetInstance();
	
	DWORD dwKeyR = dxInputDev.GetKeyState ( RANPARAM::MenuShotcut[SHOTCUT_RUN] );
	DWORD dwKeyLAlt = dxInputDev.GetKeyState ( DIK_LMENU );
	DWORD dwKeyLCtrl = dxInputDev.GetKeyState ( DIK_LCONTROL );
	DWORD dwKeyLShift = dxInputDev.GetKeyState ( DIK_LSHIFT );
	DWORD dwKeySPACE = dxInputDev.GetKeyState ( DIK_SPACE );

	DWORD dwKeyZ = dxInputDev.GetKeyState ( RANPARAM::MenuShotcut[SHOTCUT_PKMODE] );

	bool bONLY_ITEM = (dwKeyLAlt&DXKEY_DOWNED)!=NULL;
	bool bODER_P2P = (dwKeyLShift&DXKEY_DOWNED)!=NULL;

	bool bANY(false), bENEMY(false), bOUR(false), bDIE(false);
	switch ( emFINDTAR )
	{
	case ENFIND_TAR_NULL:
		bANY = true;
		bENEMY = true;
		bOUR = true;
		break;
	case EMFIND_TAR_ENEMY:
		bENEMY = true;
		break;
	case EMFIND_TAR_ANY:
		bANY = true;
		break;
	case EMFIND_TAR_OUR:
		bOUR = true;
		break;

	case EMFIND_TAR_ANY_DIE:
		bANY = true;
		bDIE = true;
		break;
	case EMFIND_TAR_OUR_DIE:
		bOUR = true;
		bDIE = true;
		break;
	};

	STARGETID sTID_FOUND;
	DETECTMAP_RANGE pairRange;

	//	Crow �˻� ( ��, ���� �̵��� ���� �˻����� �ʴ´�. )
	DETECTMAP *pDetectMap = NULL;

	if ( bDIE )
	{
		pDetectMap = GLGaeaClient::GetInstance().DetectCrowDie ( vFromPt, vTargetPt );
		if ( pDetectMap )
		{
			//	Note : ALT Ű�� ���� ���¿����� ���� ���� ==> ������, ���� �켱���� ��.
			//
			if ( !bONLY_ITEM )
			{
				if ( bENEMY )
				{
					//	MOB.
					pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_MOB, STAR_ORDER() );
					if ( pairRange.first != pairRange.second )
					{
						sACTAR.emACTAR = EMACTAR_MOB;
						sACTAR.sTARID = CFINDER::GetInstance().FindClosedCrow ( pairRange, vFromPt );
						return sACTAR;
					}
				}
				else
				{
					//	PC ANY.
					pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_PC, STAR_ORDER() );
					if ( pairRange.first != pairRange.second )
					{
						sTID_FOUND = CFINDER::GetInstance().FindClosedCrow ( pairRange, vFromPt );

						sACTAR.emACTAR = EMACTAR_PC_ANY;
						sACTAR.sTARID = sTID_FOUND;
						return sACTAR;
					}
				}
			}
		}

		return sACTAR;
	}

	DWORD emCrow = ( CROW_EX_PC | CROW_EX_NPC | CROW_EX_MOB | CROW_EX_ITEM | 
					 CROW_EX_MONEY | CROW_EX_SUMMON | CROW_EX_MATERIAL );
	pDetectMap = GLGaeaClient::GetInstance().DetectCrow ( emCrow, vFromPt, vTargetPt );

	if ( !pDetectMap )
	{
		//	Note : ��ų�� '��ο���' Ȥ�� '�츮������' �Ӽ��� ��� ���õ� �ɸ��� ������,
		//			�ڽ��� ���õǴ��� ����.
		if ( emFINDTAR==EMFIND_TAR_ANY || emFINDTAR==EMFIND_TAR_ENEMY )
		{
			BOOL bCol = COLLISION::IsCollisionLineToAABB ( vFromPt, vTargetPt, m_vMax, m_vMin );
			if ( bCol )
			{
				//	�츮���̸� OUR �ƴϸ� ANY
				EMACTIONTAR		emACTAR = EMACTAR_PC_ANY;
				if ( bOUR )		emACTAR = EMACTAR_PC_OUR;

				sACTAR.emACTAR = emACTAR;
				sACTAR.sTARID = STARGETID(CROW_PC,m_dwGaeaID,m_vPos);
				return sACTAR;
			}
		}

		return sACTAR;
	}

	//	Note : ALT Ű�� ���� ���¿����� ���� ���� ==> ������, ���� �켱���� ��.
	//
	if ( !bONLY_ITEM )
	{
		//	PVP.
		pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_PC, STAR_ORDER() );
		if ( pairRange.first != pairRange.second )
		{
			//	Note : P2P �뵵.
			//
			sTID_FOUND = CFINDER::GetInstance().FindClosedCrow ( pairRange, vFromPt );
			if ( bODER_P2P )
			{
				sACTAR.emACTAR = EMACTAR_PC_P2P;
				sACTAR.sTARID = sTID_FOUND;
				return sACTAR;
			}
			else if ( bENEMY )
			{
				STARGETID *pPVP(NULL);

				//	Note : ����ڰ� �ִ��� �˻�.	( ����� �켱 )
				//
				pPVP = GetCONFT_TAR(pairRange);
				if ( pPVP )
				{
					sACTAR.emACTAR = EMACTAR_PC_PVP;
					sACTAR.sTARID = *pPVP;
					return sACTAR;
				}

				STARGETID &sTID_FOUND = CFINDER::GetInstance().FindClosedCrow ( pairRange, vFromPt );
				PGLCHARCLIENT pCHAR = GLGaeaClient::GetInstance().GetChar ( sTID_FOUND.dwID );
				if ( pCHAR )
				{
					// ���� ����Ű�� ������ �ʾƵ� �Ǵ� ���( ���� �̺�Ʈ ���ϰ�� )
					bool bFORCED_PK = (dwKeyZ&DXKEY_DOWNED)!=NULL;
					bFORCED_PK = bFORCED_PK && !CInnerInterface::GetInstance().IsCHAT_BEGIN();
					bool bBRIGHTEVENT = GLGaeaClient::GetInstance().IsBRIGHTEVENT();

					bFORCED_PK = bFORCED_PK || bBRIGHTEVENT;
					if ( IsPK_TAR(pCHAR,bFORCED_PK) )
					{
						sACTAR.emACTAR = EMACTAR_PC_PVP;
						sACTAR.sTARID = sTID_FOUND;
						return sACTAR;
					}
				}
				
			}
		}

		

		if ( bENEMY )
		{
			//	MOB.
			pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_MOB, STAR_ORDER() );
			if ( pairRange.first != pairRange.second )
			{
				STARGETID *pTARID = FindCrow ( pairRange, m_sOLD_TARMOB );
				if ( pTARID )
				{
					sACTAR.emACTAR = EMACTAR_MOB;
					sACTAR.sTARID = *pTARID;
					return sACTAR;
				}

				sACTAR.emACTAR = EMACTAR_MOB;
				sACTAR.sTARID = CFINDER::GetInstance().FindClosedCrow ( pairRange, vFromPt );
				return sACTAR;
			}
		}

		/*skill summon, Juver, 2017/10/08 */
		if ( bENEMY )
		{
			pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_SUMMON, STAR_ORDER() );
			if ( pairRange.first != pairRange.second )
			{
				STARGETID *pPVP(NULL);
				pPVP = GetCONFT_TAR(pairRange);
				if ( pPVP )
				{
					sACTAR.emACTAR = EMACTAR_SUMMON_ATTACK;
					sACTAR.sTARID = *pPVP;
					return sACTAR;
				}

				sTID_FOUND = CFINDER::GetInstance().FindClosedCrow ( pairRange, vFromPt );

				PGLANYSUMMON pSummon = GLGaeaClient::GetInstance().GetSummon ( sTID_FOUND.dwID );
				if ( pSummon && pSummon->IsTargetReady() )
				{
					PGLCHARCLIENT pCHAR = GLGaeaClient::GetInstance().GetChar ( pSummon->m_dwOwner );
					if ( pCHAR )
					{
						// ���� ����Ű�� ������ �ʾƵ� �Ǵ� ���( ���� �̺�Ʈ ���ϰ�� )
						bool bFORCED_PK = (dwKeyZ&DXKEY_DOWNED)!=NULL;
						bFORCED_PK = bFORCED_PK && !CInnerInterface::GetInstance().IsCHAT_BEGIN();
						bool bBRIGHTEVENT = GLGaeaClient::GetInstance().IsBRIGHTEVENT();

						bFORCED_PK = bFORCED_PK || bBRIGHTEVENT;
						if ( IsPK_TAR(pCHAR,bFORCED_PK) )
						{
							sACTAR.emACTAR = EMACTAR_SUMMON_ATTACK;
							sACTAR.sTARID = sTID_FOUND;
							return sACTAR;
						}
					}
				}
			}
		}

		/*skill summon, Juver, 2017/10/08 */
		if ( bOUR || bANY )
		{
			pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_SUMMON, STAR_ORDER() );
			if ( pairRange.first != pairRange.second )
			{
				sTID_FOUND = CFINDER::GetInstance().FindClosedCrow ( pairRange, vFromPt );

				PGLANYSUMMON pSummon = GLGaeaClient::GetInstance().GetSummon ( sTID_FOUND.dwID );
				if ( pSummon && pSummon->IsTargetReady() )
				{
					GLPARTY_CLIENT* pPARTY_CLT = GLPartyClient::GetInstance().FindMember(pSummon->m_dwOwner);

					if ( pPARTY_CLT && bOUR )
					{
						sACTAR.emACTAR = EMACTAR_PC_OUR;
						sACTAR.sTARID = sTID_FOUND;
						return sACTAR;
					}
					else
					{
						sACTAR.emACTAR = EMACTAR_PC_ANY;
						sACTAR.sTARID = sTID_FOUND;
						return sACTAR;
					}
				}	
			}
		}

		//	NPC.
		pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_NPC, STAR_ORDER() );
		if ( pairRange.first != pairRange.second )
		{
			sACTAR.emACTAR = EMACTAR_NPC;
			sACTAR.sTARID = CFINDER::GetInstance().FindClosedCrow ( pairRange, vFromPt );
			return sACTAR;
		}

		//	METERIAL
		pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_MATERIAL, STAR_ORDER() );
		if ( pairRange.first != pairRange.second )
		{
			sACTAR.emACTAR = EMACTAR_MATERIAL;
			sACTAR.sTARID = CFINDER::GetInstance().FindClosedCrow ( pairRange, vFromPt );
			return sACTAR;
		}

		//	PC, OUR, ANY.
		pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_PC, STAR_ORDER() );
		if ( pairRange.first != pairRange.second )
		{
			sTID_FOUND = CFINDER::GetInstance().FindClosedCrow ( pairRange, vFromPt );
			GLPARTY_CLIENT* pPARTY_CLT = GLPartyClient::GetInstance().FindMember(sTID_FOUND.dwID);

			//	�츮���̸� OUR �ƴϸ� ANY
			if ( pPARTY_CLT && bOUR )
			{
				sACTAR.emACTAR = EMACTAR_PC_OUR;
				sACTAR.sTARID = sTID_FOUND;
				return sACTAR;
			}
			else
			{
				sACTAR.emACTAR = EMACTAR_PC_ANY;
				sACTAR.sTARID = sTID_FOUND;
				return sACTAR;
			}
		}

		//	Note : ��ų�� '��ο���' Ȥ�� '�츮������' �Ӽ��� ��� ���õ� �ɸ��� ������,
		//			�ڽ��� ���õǴ��� ����.
		if ( emFINDTAR==EMFIND_TAR_ANY || emFINDTAR==EMFIND_TAR_ENEMY )
		{
			BOOL bCol = COLLISION::IsCollisionLineToAABB ( vFromPt, vTargetPt, m_vMax, m_vMin );
			if ( bCol )
			{
				//	�츮���̸� OUR �ƴϸ� ANY
				EMACTIONTAR		emACTAR = EMACTAR_PC_ANY;
				if ( bOUR )		emACTAR = EMACTAR_PC_OUR;

				sACTAR.emACTAR = emACTAR;
				sACTAR.sTARID = STARGETID(CROW_PC,m_dwGaeaID,m_vPos);
				return sACTAR;
			}
		}
	}

	// SUMMON ATTACK
	bool bFORCED_PK = (dwKeyZ&DXKEY_DOWNED)!=NULL;
	if ( bFORCED_PK )
	{
		pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_SUMMON, STAR_ORDER() );
		if ( pairRange.first != pairRange.second )
		{
			sTID_FOUND = CFINDER::GetInstance().FindClosedCrow ( pairRange, vFromPt );
			if ( bENEMY )
			{
				PGLANYSUMMON pSummon = GLGaeaClient::GetInstance().GetSummon ( sTID_FOUND.dwID );
				if ( pSummon && pSummon->IsTargetReady() )
				{
					// ���� ����Ű�� ������ �ʾƵ� �Ǵ� ���( ���� �̺�Ʈ ���ϰ�� )
					bool bFORCED_PK = (dwKeyZ&DXKEY_DOWNED)!=NULL;
					if ( bFORCED_PK )
					{
						GLPARTY_CLIENT* pPARTY_CLT = GLPartyClient::GetInstance().FindMember(pSummon->m_dwOwner);
						if ( !pPARTY_CLT || !bOUR ) 
						{
							sACTAR.emACTAR = EMACTAR_SUMMON_ATTACK;
							sACTAR.sTARID = sTID_FOUND;
						}
						return sACTAR;
					}
				}
			}
		}
	}

	//	ITEM.
	pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_ITEM, STAR_ORDER() );
	if ( pairRange.first != pairRange.second )
	{
		sACTAR.emACTAR = EMACTAR_ITEM;
		sACTAR.sTARID = CFINDER::GetInstance().FindClosedCrow ( pairRange, vFromPt );
		return sACTAR;
	}

	//	MONEY.
	pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_MONEY, STAR_ORDER() );
	if ( pairRange.first != pairRange.second )
	{
		sACTAR.emACTAR = EMACTAR_ITEM;
		sACTAR.sTARID = CFINDER::GetInstance().FindClosedCrow ( pairRange, vFromPt );
		return sACTAR;
	}

	return sACTAR;
}

SACTIONTAR GLCharacter::FindNearItem ()
{
	SACTIONTAR sACTAR_ITEM;
	SACTIONTAR sACTAR_MONEY;

	//	Crow �˻� ( ��, ���� �̵��� ���� �˻����� �ʴ´�. )
	DETECTMAP *pDetectMap = NULL;
	DETECTMAP_RANGE pairRange;
	STARGETID sTID_FOUND;

	DWORD emCrow = ( CROW_EX_ITEM | CROW_EX_MONEY );
	pDetectMap = GLGaeaClient::GetInstance().DetectCrow ( emCrow, m_vPos, MAX_VIEWRANGE/3.0f );
	if ( pDetectMap )
	{
		//	Item.
		pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_ITEM, STAR_ORDER() );
		if ( pairRange.first != pairRange.second )
		{
			sTID_FOUND = CFINDER::GetInstance().FindClosedCrow ( pairRange, m_vPos );

			sACTAR_ITEM.emACTAR = EMACTAR_ITEM;
			sACTAR_ITEM.sTARID = sTID_FOUND;
		}

		//	Money.
		pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_MONEY, STAR_ORDER() );
		if ( pairRange.first != pairRange.second )
		{
			sTID_FOUND = CFINDER::GetInstance().FindClosedCrow ( pairRange, m_vPos );

			sACTAR_MONEY.emACTAR = EMACTAR_ITEM;
			sACTAR_MONEY.sTARID = sTID_FOUND;
		}
	}

	if ( sACTAR_MONEY.emACTAR!=EMACTAR_NULL && sACTAR_ITEM.emACTAR!=EMACTAR_NULL )
	{
		std_afunc::CCompareTargetDist sDist(m_vPos);

		bool bMore = sDist.operator () ( sACTAR_MONEY.sTARID, sACTAR_ITEM.sTARID );

		if ( bMore )	return sACTAR_MONEY;
		else			return sACTAR_ITEM;
	}
	else if ( sACTAR_MONEY.emACTAR!=EMACTAR_NULL )
	{
		return sACTAR_MONEY;
	}
	else if ( sACTAR_ITEM.emACTAR!=EMACTAR_NULL )
	{
		return sACTAR_ITEM;
	}

	SACTIONTAR sACTAR;
	return sACTAR;
}

HRESULT GLCharacter::PlayerUpdate ( float fTime, float fElapsedTime )
{
	if ( m_bRunBot )	return S_FALSE;
	//	Note : ����� �Է°� ����.
	//
	DxInputDevice &dxInputDev = DxInputDevice::GetInstance();

	DWORD dwKeyR(NULL);
	DWORD dwKeyX(NULL);
	DWORD dwKeyLAlt(NULL);
	DWORD dwKeyLCtrl(NULL);
	DWORD dwKeyLShift(NULL);
	DWORD dwkeySPACE(NULL);

	DWORD dwML(NULL), dwMM(NULL), dwMR(NULL);


	//	Note : ä�������� Ȱ��ȭ�� Ű���� ���� ���� ����.
	//
	if ( !DXInputString::GetInstance().IsOn() )
	{
		dwKeyR = dxInputDev.GetKeyState ( RANPARAM::MenuShotcut[SHOTCUT_RUN] );
		dwKeyX = dxInputDev.GetKeyState ( RANPARAM::MenuShotcut[SHOTCUT_ATTACKMODE] );
		dwkeySPACE = dxInputDev.GetKeyState ( DIK_SPACE );
	}

#if defined( TW_PARAM ) || defined( HK_PARAM ) 
	if ( !m_sPMarket.IsOpen() ) 
	{
		if( m_bOneHourNotInputKey == FALSE )
		{
			if( dxInputDev.IsUpdatInputState() )
			{
				m_fKeyDownCheckTime = 0.0f;
			}else{
				m_fKeyDownCheckTime += fElapsedTime;

				if( m_fKeyDownCheckTime >= 3600 )
				{
					m_bOneHourNotInputKey = TRUE;
					CInnerInterface::GetInstance().SetBlockProgramFound(true);
				}
			}
		}
	}else{
		m_fKeyDownCheckTime = 0.0f;
	}

	CDebugSet::ToView ( 1, "KeyDownCheckTime %f", m_fKeyDownCheckTime );
#endif

	dwKeyLAlt = dxInputDev.GetKeyState ( DIK_LMENU );
	dwKeyLCtrl = dxInputDev.GetKeyState ( DIK_LCONTROL );
	dwKeyLShift = dxInputDev.GetKeyState ( DIK_LSHIFT );

	dwML = dxInputDev.GetMouseState ( DXMOUSE_LEFT );
	dwMM = dxInputDev.GetMouseState ( DXMOUSE_MIDDLE );
	dwMR = dxInputDev.GetMouseState ( DXMOUSE_RIGHT );

#ifdef _RELEASED
	// �޽��� ��ŷ �׽�Ʈ
	DWORD dwKeyA = dxInputDev.GetKeyState ( DIK_A );
	DWORD dwKeyQ = dxInputDev.GetKeyState ( DIK_Q );

	if ( (dwKeyA&DXKEY_DOWNED) && (dwKeyLShift&DXKEY_DOWNED) )
	{
        SendHackingMSG ();
	}

	if ( (dwKeyQ&DXKEY_DOWNED) && (dwKeyLShift&DXKEY_DOWNED) )
	{
		SendHackingMSG1 ();
	}
#endif

	//	Note : Ʈ���̵� ��� ���� ������ ����.
	//
	if ( dwMR & DXKEY_DOWN )
	{
		GLTradeClient::GetInstance().ReSetPreItem();
	}

	//	Note : �ͱ� �ٱ� ����.
	if ( dwKeyR&DXKEY_UP )
	{
		ReqToggleRun ();
	}

	//	Note : ��ȭ ��� ��� ����.
	if ( dwKeyX&DXKEY_UP )
	{
		ReqTogglePeaceMode ();
	}

	//	Note : ����, â��, �ŷ�â, ��޹ڽ��� Ȱ��ȭ�� �ɸ��� ���� ���.
	//
	BOOL bCharMoveBlock = CInnerInterface::GetInstance().IsCharMoveBlock ();
	if ( bCharMoveBlock )
	{
		//DxCursor::GetInstance().SetCursorType(EMCS_NORMAL);
		CCursor::GetInstance().SetCursorType(CURSOR_NORMAL);
		DxViewPort::GetInstance().SetHandling ( false );
		return S_FALSE;
	}

	//	Note : ī�޶� Ű����� ���� on/off
	//
	{
		DxViewPort::GetInstance().SetHandling ( true );

		bool bKeyHandling(true);
		if ( DXInputString::GetInstance().IsOn() )	bKeyHandling = false;
		DxViewPort::GetInstance().SetKeyboard ( bKeyHandling );
	}

	D3DXVECTOR3 vTargetPt, vFromPt;
	vFromPt = DxViewPort::GetInstance().GetFromPt ();
	BOOL bOk = DxViewPort::GetInstance().GetMouseTargetPosWnd ( vTargetPt );
	if ( !bOk )	return S_FALSE;

	//	�տ� �� �������� ������. �ٴ��� ��´ٸ�.
	bool bCLICK_LEFT = NULL != (dwML&(DXKEY_DOWNED|DXKEY_UP|DXKEY_DUP));
	bool bCLICK_RIGHT = NULL != (dwMR&(DXKEY_DOWNED|DXKEY_UP|DXKEY_DUP));

	bool bCLICK_LEFT_EFFECT = NULL != (dwML&(DXKEY_DOWN));

	if ( (dwML&(DXKEY_DOWN)) || (dwMR&(DXKEY_DOWN)) )	m_sACTARTargetCamera = SACTIONTAR();

	if ( bCLICK_LEFT_EFFECT )
	{
		if ( RANPARAM::bClickEffect )
			DxEffExtra::GetInstance().RestartClickEffect();
	}

	if ( VALID_HOLD_ITEM() && (bCLICK_LEFT||bCLICK_RIGHT) )
	{
		if ( dwML&DXKEY_UP || dwMR&DXKEY_UP )
		{
			D3DXVECTOR3 vCollisionPos;
			DWORD dwCollisionID;
			BOOL bCollision;
			GLGaeaClient::GetInstance().GetActiveMap()->GetNaviMesh()->IsCollision
			(
				vFromPt,
				vTargetPt,
				vCollisionPos,
				dwCollisionID,
				bCollision
			);

			if ( bCollision )
			{
				D3DXVECTOR3 vDir = vCollisionPos - GetPosition();
				vDir.y = 0.0f;

				D3DXVec3Normalize ( &vDir, &vDir );
				D3DXVECTOR3 vDropPos = GetPosition() + vDir*float(GLCONST_CHAR::wBODYRADIUS+12);

				GLGaeaClient::GetInstance().GetActiveMap()->GetNaviMesh()->IsCollision
				(
					vDropPos + D3DXVECTOR3(0,+5,0),
					vDropPos + D3DXVECTOR3(0,-5,0),
					vCollisionPos,
					dwCollisionID,
					bCollision
				);

				vDropPos = GetPosition();
				if ( bCollision )		vDropPos = vCollisionPos;

				if ( bCLICK_LEFT )		ReqHoldToField ( vDropPos );
				else					ReqHoldToFieldFireCracker ( vDropPos );

				return S_FALSE;
			}
		}
		else	return S_FALSE;
	}

	//	Note : ��ų �ൿ �켱�� ��� ��ų ���� ������.
	//
	PGLSKILL pRunSkill(NULL);
	if ( dwMR&(DXKEY_DOWNED|DXKEY_UP|DXKEY_DUP) )
	{
		m_sActiveSkill = m_sRunSkill;

		SKILL_MAP_ITER learniter = m_ExpSkills.find ( m_sActiveSkill.dwID );
		if ( learniter!=m_ExpSkills.end() )
		{
			SCHARSKILL &sSkill = (*learniter).second;
			pRunSkill = GLSkillMan::GetInstance().GetData ( m_sActiveSkill.wMainID, m_sActiveSkill.wSubID );
		}
	}

	//	Note : �������� ã�� �Ӽ�.
	EMFIND_TAR emFINDTAR(ENFIND_TAR_NULL);
	if ( pRunSkill )
	{
		if ( pRunSkill->m_sAPPLY.IsSpec( EMSPECA_REBIRTH ) )
		{
			switch ( pRunSkill->m_sBASIC.emIMPACT_SIDE )
			{
			default:				break;
			case SIDE_ENEMY:		break;
			case SIDE_OUR:			emFINDTAR = EMFIND_TAR_OUR_DIE;		break;
			case SIDE_ANYBODY:		emFINDTAR = EMFIND_TAR_ANY_DIE;		break;
			};
		}
		else
		{
			switch ( pRunSkill->m_sBASIC.emIMPACT_SIDE )
			{
			default:				break;
			case SIDE_ENEMY:		emFINDTAR = EMFIND_TAR_ENEMY;	break;
			case SIDE_OUR:			emFINDTAR = EMFIND_TAR_OUR;		break;
			case SIDE_ANYBODY:		emFINDTAR = EMFIND_TAR_ANY;		break;
			};
		}
	}

	//	Note : �ൿ�� ���õ� �������� ã��.
	//
	GLCOPY* pACTTAR(NULL);
	m_sACTAR = FindActionTarget ( vTargetPt, vFromPt, emFINDTAR );

	EMCROW emCROW = m_sACTAR.sTARID.emCrow;

	//EMACTAR_PC_ANY;

	if( RANPARAM::bDIS_TRADE != m_bDisTradeOLD )
	{
		m_bDisTradeOLD = RANPARAM::bDIS_TRADE;
		ReqDisTrade();
	}

	if ( RANPARAM::bTargetEffect )
	{
		if ( bCLICK_LEFT || bCLICK_RIGHT )
		{
			if ( m_sACTARTarget.sTARID != m_sACTAR.sTARID ) 
			{
				if ( m_sACTAR.emACTAR != EMACTAR_NULL )
				{
					EMCROW emTargetCrowType = m_sACTAR.sTARID.emCrow;
					if ( emTargetCrowType == CROW_PC || 
						emTargetCrowType == CROW_NPC || 
						emTargetCrowType == CROW_MOB || 
						emTargetCrowType == CROW_SUMMON ||
						emTargetCrowType == CROW_MATERIAL )
					{
						m_sACTARTarget = SACTIONTAR();
						m_sACTARTarget = m_sACTAR;
						m_sACTARTargetCamera = m_sACTARTarget;
					}
				}
			}
		}
	}

	if ( DxViewPort::GetInstance().IsDefault() && GLGaeaClient::GetInstance().GetCopyActor(m_sACTARTargetCamera.sTARID) )
	{
		D3DXVECTOR3 vCamera = GLGaeaClient::GetInstance().GetCopyActor(m_sACTARTargetCamera.sTARID)->GetPosition();
		vCamera.y += 10.0f;
		DxViewPort::GetInstance().CameraJump ( vCamera );
	}

	bool bBRIGHTEVENT = GLGaeaClient::GetInstance().IsBRIGHTEVENT();

	if ( (emCROW==CROW_PC&&m_sACTAR.emACTAR==EMACTAR_PC_PVP) || m_sACTAR.emACTAR==EMACTAR_MOB || ( bBRIGHTEVENT&&m_sACTAR.emACTAR!=EMACTAR_NPC ) ||
		(emCROW==CROW_SUMMON&&m_sACTAR.emACTAR==EMACTAR_SUMMON_ATTACK) )
	{
		pACTTAR = GLGaeaClient::GetInstance().GetCopyActor ( m_sACTAR.sTARID );
	}

	if ( pACTTAR )
	{
		D3DXVECTOR3 vCenterTarget = m_sACTAR.sTARID.vPos;
		vCenterTarget.y += 10.f;

		D3DXVECTOR3 vFromTEMP = vFromPt;
		if( !GLGaeaClient::GetInstance().IsMapCollsion( vCenterTarget, vFromTEMP ) )
		{
			CInnerInterface::GetInstance().SetTargetInfo ( m_sACTAR.sTARID );
		}
	}
	else
	{
//#ifdef CH_PARAM
		// �߱� : ������ ��ư���� Ÿ�� ������ ī�޶� ���� Ǯ��.
		// DxViewPort::GetInstance().SetHandling( true );
//#endif
	}

	if ( m_sCONFTING.bFIGHT && m_sCONFTING.sOption.bSCHOOL )
	{
		//	Note : �б��� ��� ��ũ ǥ�� Update.
		CInnerInterface::GetInstance().SetAcademyUpdateNumber
		(
			GLPartyClient::GetInstance().GetConfrontNum (),
			(int) m_sCONFTING.setConftMember.size()
		);
	}

	////	Note : ���콺 ��� ����.
	////
	//switch ( m_sACTAR.emACTAR )
	//{
	//case EMACTAR_NULL:		DxCursor::GetInstance().SetCursorType(EMCS_NORMAL);		break;
	//case EMACTAR_PC_PVP:	DxCursor::GetInstance().SetCursorType(EMCS_ATTACK);		break;
	//case EMACTAR_MOB:		DxCursor::GetInstance().SetCursorType(EMCS_ATTACK);		break;
	//case EMACTAR_NPC:
	//	{
	//		bool bTALK(false);
	//		PLANDMANCLIENT pLAND = GLGaeaClient::GetInstance().GetActiveMap();
	//		if ( pLAND )
	//		{
	//			PGLCROWCLIENT pCROW = pLAND->GetCrow(m_sACTAR.sTARID.dwID);
	//			if ( pCROW )
	//			{
	//				if ( strlen(pCROW->m_pCrowData->GetTalkFile()) )		bTALK = true;
	//			}
	//		}

	//		if ( bTALK )	DxCursor::GetInstance().SetCursorType(EMCS_TALK);
	//		else			DxCursor::GetInstance().SetCursorType(EMCS_NORMAL);
	//	}
	//	break;

	//case EMACTAR_PC_OUR:	DxCursor::GetInstance().SetCursorType(EMCS_NORMAL);		break;
	//case EMACTAR_PC_P2P:	DxCursor::GetInstance().SetCursorType(EMCS_NORMAL);		break;
	//case EMACTAR_PC_ANY:	DxCursor::GetInstance().SetCursorType(EMCS_NORMAL);		break;
	//case EMACTAR_ITEM:		DxCursor::GetInstance().SetCursorType(EMCS_SELECT);		break;
	//};
	//	Note : ���콺 ��� ����.
	//
	switch ( m_sACTAR.emACTAR )
	{
	case EMACTAR_NULL:			CCursor::GetInstance().SetCursorType(CURSOR_NORMAL);		break;
	case EMACTAR_SUMMON_ATTACK: CCursor::GetInstance().SetCursorType(CURSOR_ATTACK);		break;
	case EMACTAR_PC_PVP:		
		{
			PLANDMANCLIENT pLAND = GLGaeaClient::GetInstance().GetActiveMap();
			if ( pLAND )
			{
				PGLCHARCLIENT pCHAR = pLAND->GetChar(m_sACTAR.sTARID.dwID);
				if ( pCHAR )
				{
					CInnerInterface::GetInstance().SetTargetInfo ( m_sACTAR.sTARID);
					CCursor::GetInstance().SetCursorType(CURSOR_ATTACK);
				}
			}
		}
		break;
	case EMACTAR_MOB:			
		{
			PLANDMANCLIENT pLAND = GLGaeaClient::GetInstance().GetActiveMap();
			if ( pLAND )
			{
				PGLCROWCLIENT pCROW = pLAND->GetCrow(m_sACTAR.sTARID.dwID);
				if ( pCROW && !pCROW->m_pCrowData->m_emNPCType == EMNPC_TYPE_EVENT_TOWER )
				{
					CInnerInterface::GetInstance().SetTargetInfo ( m_sACTAR.sTARID );
					CCursor::GetInstance().SetCursorType(CURSOR_ATTACK);
				}
			}
		}
		break;
	case EMACTAR_NPC:
		{
			bool bTALK(false);
			PLANDMANCLIENT pLAND = GLGaeaClient::GetInstance().GetActiveMap();
			if ( pLAND )
			{
				PGLCROWCLIENT pCROW = pLAND->GetCrow(m_sACTAR.sTARID.dwID);
				if ( pCROW )
				{
					CInnerInterface::GetInstance().SetTargetInfoNpc ( m_sACTAR.sTARID );
					if ( _tcslen(pCROW->m_pCrowData->GetTalkFile()) )		bTALK = true;
				}

				if ( bTALK )	CCursor::GetInstance().SetCursorType(CURSOR_TALK);
				else			CCursor::GetInstance().SetCursorType(CURSOR_NORMAL);
			}
		}
		break;

	case EMACTAR_PC_OUR:	
	case EMACTAR_PC_P2P:	
	case EMACTAR_PC_ANY:	
		{
			PLANDMANCLIENT pLAND = GLGaeaClient::GetInstance().GetActiveMap();
			if ( pLAND )
			{
				PGLCHARCLIENT pCHAR = pLAND->GetChar(m_sACTAR.sTARID.dwID);
				if ( pCHAR )
				{
					CInnerInterface::GetInstance().SetTargetInfoPlayer ( m_sACTAR.sTARID );
					CCursor::GetInstance().SetCursorType(CURSOR_NORMAL);
				}
			}
		}
		break;
	case EMACTAR_ITEM:		CCursor::GetInstance().SetCursorType(CURSOR_HAND);			break;
	case EMACTAR_MATERIAL:	CCursor::GetInstance().SetCursorType(CURSOR_NORMAL);		break;
	};

	//	Note : ���� ���� �ൿ ��� ���� üũ.
	bool bcontinue = (dwKeyLCtrl&(DXKEY_DOWNED|DXKEY_UP|DXKEY_DUP)) != NULL;
	if ( !bcontinue ) bcontinue = RANPARAM::bFORCED_ATTACK!=FALSE;

	BOOL bMove(FALSE);
	D3DXVECTOR3	vMoveTo(0,0,0);
	if ( !IsACTION(GLAT_ATTACK) && !IsACTION(GLAT_SKILL) && !m_bVehicle && !m_bReqVehicle )
	{
		STARGETID sTARID = m_sACTAR.sTARID;

		if ( pRunSkill )
		{			
			SetDefenseSkill( false );

			//	Note : ��ų �ߵ� �õ�.
			//
			EMIMPACT_TAR emTAR = pRunSkill->m_sBASIC.emIMPACT_TAR;
			bool bSKILL_ACT = ( emTAR==TAR_SELF || emTAR==TAR_ZONE || emTAR==TAR_SPECIFIC ); //Teleport Skill Effect 

			//	Note : ��ų �Ӽ� �˻�.11
			bool bSK2ENERMY(false), bSK2OUR(false), bSK2ANYBODY(false);
			switch ( pRunSkill->m_sBASIC.emIMPACT_SIDE )
			{
			default:				break;
			case SIDE_ENEMY:
				bSK2ENERMY = true;
				bcontinue = false;	// ��ų �������� �ɼ� off
				break;
			
			case SIDE_OUR:
				bSK2OUR = true;
				bcontinue = false;	//	���� ���� �ൿ off ( ������ �����ϴ� �͸� �����ϰ�. )
				break;
			
			case SIDE_ANYBODY:
				bSK2ANYBODY = true;
				bcontinue = false;	//	���� ���� �ൿ off ( ������ �����ϴ� �͸� �����ϰ�. )
				break;
			};

			if ( bSKILL_ACT && m_sACTAR.emACTAR==EMACTAR_NULL )
			{
				D3DXVECTOR3 vCollisionPos;
				DWORD dwCollisionID;
				BOOL bCollision;
				GLGaeaClient::GetInstance().GetActiveMap()->GetNaviMesh()->IsCollision
				(
					vFromPt, vTargetPt,
					vCollisionPos, dwCollisionID, bCollision
				);

				if ( bCollision )
				{
					D3DXVECTOR3 vDir = vCollisionPos - GetPosition();
					sTARID.vPos = vCollisionPos;
				}
				else
				{
					if ( emTAR==TAR_ZONE )	bSKILL_ACT = false;
				}
			}

			switch ( m_sACTAR.emACTAR )
			{
			case EMACTAR_NULL:			if(bSKILL_ACT)	SkillReaction(sTARID,dwMR,bcontinue,bMove,vMoveTo);		break;
			case EMACTAR_SUMMON_ATTACK: if(bSK2ENERMY)	SkillReaction(sTARID,dwMR,bcontinue,bMove,vMoveTo);		break;
			case EMACTAR_PC_PVP:		if(bSK2ENERMY)	SkillReaction(sTARID,dwMR,bcontinue,bMove,vMoveTo);		break;
			case EMACTAR_MOB:			if(bSK2ENERMY)	SkillReaction(sTARID,dwMR,bcontinue,bMove,vMoveTo);		break;
			case EMACTAR_NPC:			break;
			case EMACTAR_PC_OUR:		if(bSK2OUR)		SkillReaction(sTARID,dwMR,bcontinue,bMove,vMoveTo);		break;
			case EMACTAR_PC_P2P:		break;
			case EMACTAR_PC_ANY:		if(bSK2ANYBODY)	SkillReaction(sTARID,dwMR,bcontinue,bMove,vMoveTo);		break;
			case EMACTAR_ITEM:			break;
			case EMACTAR_MATERIAL:		break;
			};
		}
		else if ( dwML&(DXKEY_DOWNED|DXKEY_UP|DXKEY_DUP) )
		{
			//	Note : ��뿡 ���� ����.
			//
			switch ( m_sACTAR.emACTAR )
			{
			case EMACTAR_NULL:			break;
			case EMACTAR_SUMMON_ATTACK: PvPReaction(sTARID,dwML,bcontinue,bMove,vMoveTo);	break;
			case EMACTAR_PC_PVP:		PvPReaction(sTARID,dwML,bcontinue,bMove,vMoveTo);	break;
			case EMACTAR_MOB:			MobReaction(sTARID,dwML,bcontinue,bMove,vMoveTo);	break;
			case EMACTAR_NPC:			NpcReaction(sTARID,dwML,bcontinue,bMove,vMoveTo);	break;
			case EMACTAR_PC_OUR:		break;
			case EMACTAR_PC_P2P:		P2PReaction(sTARID,dwML,bcontinue,bMove,vMoveTo);	break;
			case EMACTAR_PC_ANY:		break;
			case EMACTAR_ITEM:			ItemReaction(sTARID,dwML,bcontinue,bMove,vMoveTo);	break;
			case EMACTAR_MATERIAL:		GatheringReaction(sTARID,dwML,bcontinue,bMove,vMoveTo);	break;
			};
		}
		else if ( dwkeySPACE & DXKEY_DOWN )
		{
			m_sACTAR = FindNearItem ();
			STARGETID sTARID = m_sACTAR.sTARID;

			if ( m_sACTAR.emACTAR==EMACTAR_ITEM )
				ItemReaction(sTARID,DXKEY_DOWNED|DXKEY_UP,bcontinue,bMove,vMoveTo);
		}
	}

	//	Note : Reaction ���� �̵��� ��û�� ���.
	//
	if ( bMove )
	{
		ActionMoveTo ( fTime, vMoveTo+D3DXVECTOR3(0,+5,0), vMoveTo+D3DXVECTOR3(0,-5,0), FALSE, TRUE );
		return S_OK;
	}

	if ( DxViewPort::GetInstance().IsDefault() && m_vPos != m_vSAVEPOSGM && !m_actorMove.PathIsActive() )
	{
		ActionMoveTo ( fTime, m_vSAVEPOSGM+D3DXVECTOR3(0,+5,0), m_vSAVEPOSGM+D3DXVECTOR3(0,-5,0), FALSE, TRUE );
		return S_OK;
	}
	else
	{
		if ( DxViewPort::GetInstance().IsDefault() && m_vPos == m_vSAVEPOSGM )
		{
			ActionMoveTo ( fTime, m_vSAVEPOSGM+D3DXVECTOR3(10,+5,0), m_vSAVEPOSGM+D3DXVECTOR3(10,-5,0), FALSE, TRUE );
			return S_OK;
		}
	}

	//	Note : �Ϲ� �̵� ó��.
	//
	BOOL bMOVEABLE = (m_sACTAR.emACTAR==EMACTAR_NULL);
	//				&& !IsACTION(GLAT_ATTACK) && !IsACTION(GLAT_SKILL);
	//				������ ������ ���Ӱ��� ��Ҹ� ��Ȱ�ϰ� �ϱ� ���ؼ� �׼� �߰����� ��Ұ� �����ϰ� ��ħ.
	if ( bMOVEABLE )
	{
		if ( dwML&DXKEY_UP )
		{
			CInnerInterface::GetInstance().DisableMinimapTarget();

			m_sREACTION.RESET();

			ReSetSTATE(EM_ACT_CONTINUEMOVE);
			ActionMoveTo ( fTime, vFromPt, vTargetPt );
		}
		else if ( dwML&DXKEY_DOWNED )
		{
			CInnerInterface::GetInstance().DisableMinimapTarget();

 			m_sREACTION.RESET();

			SetSTATE(EM_ACT_CONTINUEMOVE);
			ActionMoveTo ( fTime, vFromPt, vTargetPt, IsSTATE(EM_ACT_CONTINUEMOVE) );
		}
	}

	if ( !(dwML&DXKEY_DOWNED) && IsSTATE(EM_ACT_CONTINUEMOVE) )		ReSetSTATE(EM_ACT_CONTINUEMOVE);

	return S_OK;
}

HRESULT GLCharacter::UpateAnimation ( float fTime, float fElapsedTime )
{
	HRESULT hr=S_OK;

	if( !m_pSkinChar )	return E_FAIL;
	
	BOOL bLowSP = ( float(m_sSP.dwNow) / float(m_sSP.dwMax) ) <= GLCONST_CHAR::fLOWSP_MOTION;
	PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();

	BOOL bPeaceZone = pLand ? pLand->IsPeaceZone() : FALSE;
	if ( !bPeaceZone && IsSTATE(EM_ACT_PEACEMODE) )		bPeaceZone = TRUE;

	/*vehicle booster system, Juver, 2017/08/12 */
	BOOL bVehicleBooster = IsSTATE(EM_ACT_VEHICLE_BOOSTER);

	BOOL bFreeze = FALSE;

	EMANI_MAINTYPE emMType;
	EMANI_SUBTYPE emSType;

	switch ( m_Action )
	{
	case GLAT_IDLE:
		{
			m_fIdleTime += fElapsedTime;

			if ( bPeaceZone && m_pSkinChar->GETANI ( AN_PLACID, AN_SUB_NONE ) )
			{
				emMType = AN_PLACID;
				emSType = AN_SUB_NONE;
				if ( m_bVehicle )
				{
					/*vehicle system, Juver, 2017/08/07 */
					int emType = m_sVehicle.m_emTYPE ;
					if ( emType == VEHICLE_TYPE_BOARD )
					{
						emSType = (EMANI_SUBTYPE) ( AN_SUB_HOVERBOARD );
					}else{
						emMType = CHECK_VEHICLE_MAIN( m_sVehicle.m_sVehicleID );
						/*vehicle booster system, Juver, 2017/08/12 */
						emSType = CHECK_VEHICLE_SUB( bVehicleBooster? AN_ATTACK : AN_PLACID );
					}
				}
				else  emSType = AN_SUB_NONE;
			}
			else
			{
				BOOL bPANT(FALSE); // �涱�Ÿ�.

				if ( m_pSkinChar->GETANI ( AN_GUARD_L, m_emANISUBTYPE ) )
				{
					bPANT = bLowSP || m_bSTATE_PANT;
					for ( int i=0; i<EMBLOW_MULTI; ++i )
					{
						if ( m_sSTATEBLOWS[i].emBLOW == EMBLOW_STUN || m_sSTATEBLOWS[i].emBLOW == EMBLOW_MAD )		bPANT = TRUE;
					}
				}

				if ( bPANT )
				{
					emMType = AN_GUARD_L;
					emSType = m_emANISUBTYPE;
				}
				else
				{
					emMType = AN_GUARD_N;
					emSType = m_emANISUBTYPE;
				}
			}

			m_pSkinChar->SELECTANI ( emMType, emSType );
			m_bAttackable = TRUE;

			/*ABL system, Juver, 2017/06/02 */
			for ( int i=0; i<PIECE_SIZE; ++i )
			{
				if ( m_pSkinChar->m_pAttBone[i] )	
					m_pSkinChar->m_pAttBone[i]->SELECTANI( AN_ABL_STAY, AN_SUB_NONE );
			}

			/*vehicle system, Juver, 2017/08/07 */
			/*vehicle booster system, Juver, 2017/08/12 */
			if ( m_pSkinChar->m_pVehicle )	
			{
				m_pSkinChar->m_pVehicle->SELECTANI( bVehicleBooster? AN_VEHICLE_BOOSTER : AN_VEHICLE_STAY, AN_SUB_NONE );

				if ( bVehicleBooster && ( m_pSkinChar->ISENDANIM () || m_pSkinChar->m_pVehicle->ISENDANIM () ) )
					VehicleBoosterStateOff();
			}

		}break;

	case GLAT_MOVE:
		{
			emMType = IsSTATE(EM_ACT_RUN) ? AN_RUN : AN_WALK;
			emSType = bPeaceZone ? AN_SUB_NONE : m_emANISUBTYPE;
			if ( m_bVehicle )
			{
				/*vehicle system, Juver, 2017/08/07 */
				int emType = m_sVehicle.m_emTYPE;
				if ( emType == VEHICLE_TYPE_BOARD )
				{
					emSType = (EMANI_SUBTYPE) ( AN_SUB_HOVERBOARD );
				}else{
					emMType = CHECK_VEHICLE_MAIN( m_sVehicle.m_sVehicleID );
					/*vehicle booster system, Juver, 2017/08/12 */
					emSType = CHECK_VEHICLE_SUB( bVehicleBooster? AN_ATTACK : IsSTATE(EM_ACT_RUN) ? AN_RUN : AN_WALK );
				}
			}
			m_pSkinChar->SELECTANI ( emMType, emSType );

			/*ABL system, Juver, 2017/06/02 */
			for ( int i=0; i<PIECE_SIZE; ++i )
			{
				if ( m_pSkinChar->m_pAttBone[i] )	
					m_pSkinChar->m_pAttBone[i]->SELECTANI( AN_ABL_STAY, AN_SUB_NONE );
			}

			/*vehicle system, Juver, 2017/08/07 */
			/*vehicle booster system, Juver, 2017/08/12 */
			if ( m_pSkinChar->m_pVehicle )	
			{
				m_pSkinChar->m_pVehicle->SELECTANI( bVehicleBooster? AN_VEHICLE_BOOSTER : IsSTATE(EM_ACT_RUN) ? AN_VEHICLE_RUN : AN_VEHICLE_WALK, AN_SUB_NONE );

				if ( bVehicleBooster && ( m_pSkinChar->ISENDANIM () || m_pSkinChar->m_pVehicle->ISENDANIM () ) )
					VehicleBoosterStateOff();
			}
		}break;

	case GLAT_ATTACK:
		{
			EMSLOT emRHand = GetCurRHand();
			EMSLOT emLHand = GetCurLHand();

			EMANI_SUBTYPE emANISUBTYPE = CHECK_ATTACK_ANISUB ( m_pITEMS[emRHand], m_pITEMS[emLHand] );
			m_pSkinChar->SELECTANI ( AN_ATTACK, emANISUBTYPE, NULL, m_dwANISUBSELECT );
			if ( m_pSkinChar->ISENDANIM () || m_bVehicle )
			{
				TurnAction ( GLAT_IDLE );

				/*ABL system, Juver, 2017/06/02 */
				for ( int i=0; i<PIECE_SIZE; ++i )
				{
					if ( m_pSkinChar->m_pAttBone[i] )	
						m_pSkinChar->m_pAttBone[i]->SELECTANI( AN_ABL_STAY, AN_SUB_NONE );
				}
			}
		}
		break;

	case GLAT_SKILL:
		{
			m_pSkinChar->SELECTSKILLANI ( GLCHARLOGIC::m_emANIMAINSKILL, GLCHARLOGIC::m_emANISUBSKILL );

			PSANIMCONTAINER pANIMCON = m_pSkinChar->GETCURANIM();
			if ( ( !(pANIMCON->m_dwFlag&ACF_LOOP) && m_pSkinChar->ISENDANIM () ) || m_bVehicle)
			{
				TurnAction ( GLAT_IDLE );
//				m_pSkinChar->m_bResetSkillAni = TRUE;

				/*ABL system, Juver, 2017/06/02 */
				for ( int i=0; i<PIECE_SIZE; ++i )
				{
					if ( m_pSkinChar->m_pAttBone[i] )	
						m_pSkinChar->m_pAttBone[i]->SELECTANI( AN_ABL_STAY, AN_SUB_NONE );
				}
			}	
		}
		break;

	case GLAT_TALK:
		{
			BOOL bOK = m_pSkinChar->SELECTANI ( AN_GESTURE, EMANI_SUBTYPE(m_dwANISUBGESTURE) );
			if ( !bOK )
			{
				TurnAction ( GLAT_IDLE );
				break;
			}

			PSANIMCONTAINER pANIMCON = m_pSkinChar->GETCURANIM();
			if ( !(pANIMCON->m_dwFlag&ACF_LOOP) && m_pSkinChar->ISENDANIM () )
			{
				TurnAction ( GLAT_IDLE );
			}
		}
		break;

	case GLAT_SHOCK:
		{
			emMType = AN_SHOCK;	
			emSType = m_emANISUBTYPE;

			if ( m_bVehicle )
			{
				/*vehicle system, Juver, 2017/08/07 */
				int emType = m_sVehicle.m_emTYPE;
				if ( emType == VEHICLE_TYPE_BOARD )
				{
					emSType = (EMANI_SUBTYPE) ( AN_SUB_HOVERBOARD );
				}else{
					emMType = CHECK_VEHICLE_MAIN( m_sVehicle.m_sVehicleID );
					emSType = CHECK_VEHICLE_SUB( AN_SHOCK );
				}
			}

			m_pSkinChar->SELECTANI ( emMType, emSType );

			if ( m_pSkinChar->ISENDANIM () )	TurnAction ( GLAT_IDLE );

			/*vehicle system, Juver, 2017/08/07 */
			if ( m_pSkinChar->m_pVehicle )	
				m_pSkinChar->m_pVehicle->SELECTANI( AN_VEHICLE_SHOCK, AN_SUB_NONE );

		}break;
		
	case GLAT_PUSHPULL:
		m_pSkinChar->SELECTANI ( AN_SHOCK, GLCHARLOGIC::m_emANISUBTYPE );
		break;

		/*dash skill logic, Juver, 2017/06/17 */
	case GLAT_SKILLDASH:
		{
			if ( m_wActionAnim != AN_SUB_00_SIZE )
				m_pSkinChar->SELECTANI ( AN_SPEC, (EMANI_SUBTYPE)m_wActionAnim );
		}break;

	case GLAT_FALLING:
		{
			emMType = AN_DIE;
			emSType = AN_SUB_NONE;

			if ( m_bVehicle )
			{
				/*vehicle system, Juver, 2017/08/07 */
				int emType = m_sVehicle.m_emTYPE;
				if ( emType == VEHICLE_TYPE_BOARD )
				{
					emSType = (EMANI_SUBTYPE) ( AN_SUB_HOVERBOARD );
				}else{
					emMType = CHECK_VEHICLE_MAIN( m_sVehicle.m_sVehicleID );
					emSType = CHECK_VEHICLE_SUB( AN_DIE );
				}
			}

			/*ABL system, Juver, 2017/06/02 */
			for ( int i=0; i<PIECE_SIZE; ++i )
			{
				if ( m_pSkinChar->m_pAttBone[i] )	
					m_pSkinChar->m_pAttBone[i]->SELECTANI( AN_ABL_FREE, AN_SUB_NONE );
			}

			/*vehicle system, Juver, 2017/08/07 */
			if ( m_pSkinChar->m_pVehicle )	
				m_pSkinChar->m_pVehicle->SELECTANI( AN_VEHICLE_DOWN, AN_SUB_NONE, EMANI_ENDFREEZE );

			m_pSkinChar->SELECTANI ( emMType, emSType, EMANI_ENDFREEZE );

			if ( m_pSkinChar->ISENDANIM () )
			{
				TurnAction ( GLAT_DIE );
			}
		}break;

	case GLAT_DIE:
		{
			bFreeze = TRUE;

			/*ABL system, Juver, 2017/06/02 */
			for ( int i=0; i<PIECE_SIZE; ++i )
			{
				if ( m_pSkinChar->m_pAttBone[i] )
					m_pSkinChar->m_pAttBone[i]->SELECTANI( AN_ABL_FREE, AN_SUB_NONE );
			}

			/*vehicle system, Juver, 2017/08/07 */
			if ( m_pSkinChar->m_pVehicle )	
			{
				m_pSkinChar->m_pVehicle->SELECTANI( AN_VEHICLE_DOWN, AN_SUB_NONE, EMANI_ENDFREEZE );
				m_pSkinChar->m_pVehicle->TOENDTIME();
			}


			m_pSkinChar->TOENDTIME();
		}break;

	case GLAT_CONFT_END:
		{
			EMANI_MAINTYPE emMTYPE(AN_CONFT_LOSS);
			if ( IsSTATE(EM_ACT_CONFT_WIN) )	emMTYPE = AN_CONFT_WIN;

			m_pSkinChar->SELECTANI ( emMTYPE, AN_SUB_NONE );
			if ( m_pSkinChar->ISENDANIM () )	TurnAction ( GLAT_IDLE );
		}
		break;

	case GLAT_GATHERING:
		{
			BOOL bOK = m_pSkinChar->SELECTANI ( AN_GATHERING, EMANI_SUBTYPE(m_dwANISUBGESTURE) );
			if ( !bOK )
			{
				TurnAction ( GLAT_IDLE );
				break;
			}

			PSANIMCONTAINER pANIMCON = m_pSkinChar->GETCURANIM();
			if ( !(pANIMCON->m_dwFlag&ACF_LOOP) && m_pSkinChar->ISENDANIM () )
			{
				TurnAction ( GLAT_IDLE );
			}
		}
		break;
	};

	//	Note : ��Ų ������Ʈ.
	//
	m_pSkinChar->SetPosition ( m_vPos );

	BOOL bContinue = IsACTION(GLAT_ATTACK) || IsACTION(GLAT_SKILL);

	//	Note : ���� �̻� ���� ��� �ӵ��� �����Ѵ�.
	//
	float fSkinAniElap = fElapsedTime;
	switch ( m_Action )
	{
	case GLAT_MOVE:
		fSkinAniElap *= ( GLCHARLOGIC::GETMOVE_ITEM() + GLCHARLOGIC::GETMOVEVELO() );
		break;

	case GLAT_ATTACK:
	case GLAT_SKILL:
		fSkinAniElap *= GLCHARLOGIC::GETATTVELO();
		fSkinAniElap += GLCHARLOGIC::GETATT_ITEM(); // ����ġ
		break;
	};

	if ( !IsSTATE(EM_REQ_VISIBLENONE) )	m_pSkinChar->FrameMove ( fTime, fSkinAniElap, bContinue, bFreeze, !bPeaceZone );
	
	//	Note : �� ���� �����϶� ǥ�� �ȵǾ� �ϴ� ������(�ܰ�,��ô) ���� �˻��� ���� ����.
	//
	if ( bPeaceZone )
	{
		EMSLOT emRHand = GetCurRHand();
		EMSLOT emLHand = GetCurLHand();

		SITEM *pItemR = GET_SLOT_ITEMDATA ( emRHand );
		SITEM *pItemL = GET_SLOT_ITEMDATA ( emLHand );

		if ( pItemR )
		{
			BOOL bRend = !( pItemR->sSuitOp.emAttack==ITEMATT_DAGGER || pItemR->sSuitOp.emAttack==ITEMATT_THROW );
			m_pSkinChar->SetPartRend ( PIECE_RHAND, bRend );
		}
		if ( pItemL )
		{
			BOOL bRend = !( pItemL->sSuitOp.emAttack==ITEMATT_DAGGER || pItemL->sSuitOp.emAttack==ITEMATT_THROW );
			m_pSkinChar->SetPartRend ( PIECE_LHAND, bRend );
		}
	}
	else
	{
		m_pSkinChar->SetPartRend ( PIECE_RHAND, TRUE );
		m_pSkinChar->SetPartRend ( PIECE_LHAND, TRUE );
	}

	return S_OK;
}

// *****************************************************
// Desc: ����Ʈ ����
// *****************************************************
DWORD GLCharacter::DetectGate ()
{
	if ( IsSTATE(EM_REQ_GATEOUT) )		return UINT_MAX;

	PLANDMANCLIENT pLandMClient = GLGaeaClient::GetInstance().GetActiveMap();
	if ( !pLandMClient )								return UINT_MAX;

	DxLandGateMan *pLandGateMan = &pLandMClient->GetLandGateMan();
	if ( !pLandGateMan )								return UINT_MAX;
		
	if ( pLandMClient->m_fAge<3.0f )					return UINT_MAX;

	PDXLANDGATE pLandGate = pLandGateMan->DetectGate ( GetPosition() );
	if ( !pLandGate )									return UINT_MAX;
		
	if ( pLandGate->GetFlags()&DxLandGate::GATE_OUT )	return pLandGate->GetGateID();

	return UINT_MAX;
}

// *****************************************************
// Desc: ����Ʈ�� �̸� ����
// *****************************************************
CString GLCharacter::DetectGateToMapName ()
{
	if ( IsSTATE(EM_REQ_GATEOUT) )						return "";

	PLANDMANCLIENT pLandMClient = GLGaeaClient::GetInstance().GetActiveMap();
	if ( !pLandMClient )								return "";

	DxLandGateMan *pLandGateMan = &pLandMClient->GetLandGateMan();
	if ( !pLandGateMan )								return "";
		
	if ( pLandMClient->m_fAge<3.0f )					return "";

	PDXLANDGATE pLandGate = pLandGateMan->DetectGate ( GetPosition() );
	if ( !pLandGate )									return "";
		
	if ( pLandGate->GetFlags()&DxLandGate::GATE_OUT )
	{
		SNATIVEID sToMapID = pLandGate->GetToMapID();

		SMAPNODE *pMapNode = GLGaeaClient::GetInstance().FindMapNode ( sToMapID );
		if ( pMapNode )		return pMapNode->strMapName.c_str();
	}

	return "";
}

float GLCharacter::GetMoveVelo ()
{
	float fDefaultVelo = IsSTATE(EM_ACT_RUN) ? GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX].fRUNVELO : GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX].fWALKVELO;
	float fMoveVelo = fDefaultVelo * ( GLCHARLOGIC::GETMOVEVELO() + GLCHARLOGIC::GETMOVE_ITEM() );
	return fMoveVelo;
}

void GLCharacter::VietnamCalculate()
{
	const int nMax1 = 180;
	const int nMax2 = 300;
	/*const int nMax1 = 30;
	const int nMax2 = 50;*/
	CInnerInterface::GetInstance().SET_VNGAINTYPE_GAUGE( 0, nMax2 );
   	if( m_sVietnamSystem.loginTime == 0 ) return;
	if( m_dwVietnamGainType == GAINTYPE_EMPTY ) 
	{
		CInnerInterface::GetInstance().SET_VNGAINTYPE_GAUGE( nMax2, nMax2 );
		return;
	}

	CTimeSpan gameTime( 0, (int)m_sVietnamSystem.gameTime / 60, (int)m_sVietnamSystem.gameTime % 60, 0 );


	CTime loginTime   = m_sVietnamSystem.loginTime;
	CTime crtTime     = GLGaeaClient::GetInstance().GetCurrentTime();
	CTimeSpan crtGameSpan;
	crtGameSpan			  = gameTime + ( crtTime - loginTime );

	int totalMinute  = (int)crtGameSpan.GetTotalMinutes();
	int totalSecond  = (int)crtGameSpan.GetTotalSeconds();

	CDebugSet::ToView ( 1, "Vietnam total Time M %d S %d", totalMinute, totalSecond );
	CDebugSet::ToView ( 2, "GameTime H %d M %d S %d Value %d", gameTime.GetTotalHours(), gameTime.GetTotalMinutes(), gameTime.GetTotalSeconds(),
						m_sVietnamSystem.gameTime );
	CDebugSet::ToView ( 3, "CrtTime Year %d Mon %d D %d H %d M %d S %d LoginTime Year %d Mon %d D %d H %d M %d S %d", 
							crtTime.GetYear(), crtTime.GetMonth(), crtTime.GetDay(), crtTime.GetHour(), crtTime.GetMinute(), crtTime.GetSecond(), 
							loginTime.GetYear(), loginTime.GetMonth(), loginTime.GetDay(), loginTime.GetHour(), loginTime.GetMinute(), loginTime.GetSecond() );


	if( m_dwVietnamGainType == GAINTYPE_MAX )
	{
		if( totalMinute > nMax1 )
			totalMinute = nMax1;
	}else if( m_dwVietnamGainType == GAINTYPE_HALF )
	{
		if( totalMinute < nMax1 )
			totalMinute = nMax1;
		if( totalMinute > nMax2 )
			totalMinute = nMax2;
	}

	if( totalMinute < 0 )  totalMinute = 0;
	
	CInnerInterface::GetInstance().SET_VNGAINTYPE_GAUGE( totalMinute, nMax2 );
	
	
	//	m_sVietnamSystem.currentGameTime = totalHours;
	//	m_sVietnamSystem.currentGameTime = totalMinutes;
	// �׽�Ʈ�� �ʷ� ����..
}

void GLCharacter::EventCalculate()
{
	if( m_bEventStart == FALSE )
	{
//		CDebugSet::ToView ( 1, "Event Start FALSE" );	
		return;
	}
	if( m_bEventApply == FALSE )
	{
//		CDebugSet::ToView ( 1, "Event Apply FALSE" );	
		return;
	}

	CTime crtTime   = GLGaeaClient::GetInstance().GetCurrentTime();
	CTime startTime = m_tLoginTime;
	CTimeSpan crtGameSpan;
	crtGameSpan					 = ( crtTime - startTime );

	if( m_bEventBuster == FALSE )
	{
		m_RemainEventTime  = m_EventStartTime - (int)crtGameSpan.GetTotalSeconds();
		m_RemainBusterTime = m_EventBusterTime;

		if( crtGameSpan.GetTotalSeconds() >= m_EventStartTime )
		{
//			m_bEventBuster = TRUE;	
			m_RemainEventTime = 0;
		}
	}

	if( m_bEventBuster == TRUE )
	{
		m_RemainEventTime  = 0;
		m_RemainBusterTime = (m_EventStartTime + m_EventBusterTime) - (int)crtGameSpan.GetTotalSeconds();
		if( m_RemainBusterTime > m_EventBusterTime )
			m_RemainBusterTime = m_EventBusterTime;

		if( crtGameSpan.GetTotalSeconds() >= m_EventStartTime + m_EventBusterTime )
		{
//			m_bEventBuster = FALSE;
//			m_tLoginTime   = crtTime.GetTime();

			m_RemainEventTime  = m_EventStartTime;
			m_RemainBusterTime = m_EventBusterTime;
		}
	}

	

	
	/*CDebugSet::ToView ( 1, "gameSpan: %d ;; bEventBuster: %d ;; startTime: %d ;; busterTime: %d", 
					    (int)crtGameSpan.GetTotalSeconds(), m_bEventBuster, m_EventStartTime, m_EventBusterTime );

	CDebugSet::ToView ( 2, "RemainEventTime: %d RemainBusterTime: %d", m_RemainEventTime, m_RemainBusterTime );*/

	
}

void GLCharacter::UpdateSpecialSkill()
{
	// ���� ��ų üũ
	if( m_dwTransfromSkill != SNATIVEID::ID_NULL )
	{
		if( m_sSKILLFACT[m_dwTransfromSkill].IsSpecialSkill( SKILL::EMSSTYPE_TRANSFORM ) )
		{
			// �������� ����̸� ��������
			STARGETID sTargetID(CROW_PC,m_dwGaeaID,m_vPos);
			if( IsACTION( GLAT_FALLING ) )
			{
				FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT[m_dwTransfromSkill].sNATIVEID );
			}

			if( m_sSKILLFACT[m_dwTransfromSkill].sNATIVEID != NATIVEID_NULL() ) 
			{			
				PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_sSKILLFACT[m_dwTransfromSkill].sNATIVEID.wMainID, m_sSKILLFACT[m_dwTransfromSkill].sNATIVEID.wSubID );
				if( pSkill && pSkill->m_sSPECIAL_SKILL.emSSTYPE == SKILL::EMSSTYPE_TRANSFORM )
				{
					if( pSkill->m_sSPECIAL_SKILL.strEffectName.size() != 0 )
					{
						if( m_sSKILLFACT[m_dwTransfromSkill].fAGE <= pSkill->m_sSPECIAL_SKILL.dwRemainSecond && 
							!m_sSKILLFACT[m_dwTransfromSkill].bRanderSpecialEffect )
						{
							D3DXMATRIX matTrans;							
							D3DXMatrixTranslation ( &matTrans, m_vPos.x, m_vPos.y, m_vPos.z );
							DxEffGroupPlayer::GetInstance().NewEffGroup ( pSkill->m_sSPECIAL_SKILL.strEffectName.c_str(), matTrans, &sTargetID );
							m_sSKILLFACT[m_dwTransfromSkill].bRanderSpecialEffect = TRUE;
						}
					}
				}
			}else{
				DISABLESKEFF(m_dwTransfromSkill);
				UpdateSuit(TRUE);				
				m_dwTransfromSkill = SNATIVEID::ID_NULL;
			}

			
		}else{
			DISABLESKEFF(m_dwTransfromSkill);
			UpdateSuit(TRUE);
			m_dwTransfromSkill = SNATIVEID::ID_NULL;
		}
	}
}

void GLCharacter::SET_RECORD_CHAT()
{
	if( m_bRecordChat ) return;
	m_bRecordChat = TRUE;
	m_strRecordChat		= CInnerInterface::GetInstance().GET_RECORD_CHAT();
	m_recordStartTime	= GLGaeaClient::GetInstance().GetCurrentTime();

	CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("RECORD_CHAT_START") );
}

void GLCharacter::UPDATE_RECORD_CHAT( bool bCloseClient /* = FALSE */ )
{
	if( !m_bRecordChat ) return;
	CTime curTime = GLGaeaClient::GetInstance().GetCurrentTime();
	CTimeSpan timeSpan = curTime - m_recordStartTime;
	if( timeSpan.GetMinutes() >= 2 || bCloseClient ) 
	{
		m_strRecordChat += CInnerInterface::GetInstance().GET_RECORD_CHAT();

		{
			TCHAR szPROFILE[MAX_PATH] = {0};
			TCHAR szFullPathFileName[MAX_PATH] = {0};
			SHGetSpecialFolderPath( NULL, szPROFILE, CSIDL_PERSONAL, FALSE );

			StringCchCopy( szFullPathFileName, MAX_PATH, szPROFILE );
			StringCchCat( szFullPathFileName, MAX_PATH, SUBPATH::SAVE_ROOT );
			CreateDirectory( szFullPathFileName, NULL );

			CHAR  szFileName[MAX_PATH] = {0};
			sprintf( szFileName, "chat_[%d%d%d%d%d].txt", curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(), 
													      curTime.GetHour(), curTime.GetMinute() );

			StringCchCat ( szFullPathFileName, MAX_PATH, szFileName );

			m_bRecordChat   = FALSE;

			CFile file;
			if ( ! file.Open( _T(szFullPathFileName), CFile::modeCreate|CFile::modeWrite|CFile::typeBinary ) )// ���� ����
			{

				CDebugSet::ToListView ( "RECORD_CHAT_FAILED" );
				return;
			}

			CString strSaveChat;			
			for( int i = 0; i < m_strRecordChat.GetLength(); i++ )
			{
				char szTempChar = m_strRecordChat.GetAt(i) ^ 0x2139;
				strSaveChat += szTempChar;
			}

			int nLength = strSaveChat.GetLength();
			file.Write(&nLength, sizeof(int));
			file.Write(strSaveChat.GetString(), nLength);
			file.Close();


			m_strRecordChat = "";
		}

		if( !bCloseClient )
		{
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("RECORD_CHAT_END") );
		}

	}
}

void GLCharacter::UpdateLandEffect()
{
	PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();
	if( !pLand ) return;

	SLEVEL_ETC_FUNC *pLevelEtcFunc = pLand->GetLevelEtcFunc();
	if( !pLevelEtcFunc ) return;
	if( !pLevelEtcFunc->m_bUseFunction[EMETCFUNC_LANDEFFECT] ) return;

	// ��� ������ ����ؾ��� ����� ó��
	for( int i = 0; i < EMLANDEFFECT_MULTI; i++ )
	{
		SLANDEFFECT landEffect = m_sLandEffect[i];
		if( !landEffect.IsUse() ) continue;
		if( landEffect.emLandEffectType == EMLANDEFFECT_CANCEL_ALLBUFF )
		{
			if( m_dwTransfromSkill != SNATIVEID::ID_NULL )
			{
				m_dwTransfromSkill = SNATIVEID::ID_NULL;
				UpdateSuit(TRUE);
			}
			for ( int i=0; i<SKILLFACT_SIZE; ++i )
			{
				FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT[i].sNATIVEID );
				DISABLESKEFF(i);
			}			
			return;
		}
	}
}

HRESULT GLCharacter::FrameMove ( float fTime, float fElapsedTime )
{
	HRESULT hr=S_OK;

	if( !m_bReqDoneVehi && m_bVehicle ){
		m_bVehicle = FALSE;
	}

	/*This Anti AP code is from Eduj. Boss Eli, for Effectiveness of anti AP 
	  I have added some recoding for effectiveness		*/
	// 9/13/2016 - Anti  Auto Pots System - Eduj
	// time delay when potion is use.
	/*
	if ( m_bUsePots || m_bAutoPots ) m_fTickDelay += fElapsedTime;
	// remain true
	if ( m_bUsePots ) m_bUsePots = TRUE;
	else m_bUsePots = FALSE;
	// remain true
	if ( m_bAutoPots ) m_bAutoPots = TRUE;
	else m_bAutoPots = FALSE;
	// remain true
	if ( m_bDisablePots ) m_bDisablePots = TRUE;
	else m_bDisablePots = FALSE;
	// remain true
	if ( m_bDetectOnce ) m_bDetectOnce = TRUE;
	else m_bDetectOnce = FALSE;
     // function and detection is fucking here! HAHAHA
     if ( !m_bAutoPots && !m_bDisablePots )
        {
            if ( m_bUsePots )
	          {
	             if ( m_fTickDelay > 1.0f )
	               {
				   // tickcount this is count of applying pots persecond
				   // you can change the amount if the detection is too sensitive
				   // default 25
	                 if ( m_dwTickCount >= 35 )
	                   {
		                 m_fTickDelay = 0.0f;
		                 m_bAutoPots = TRUE;
						 //ReqDetectAP ( m_bAutoPots, m_dwCharID );
	                    }else{
	                     m_dwTickCount = 0;
	                     m_fTickDelay = 0.0f;
                         m_bUsePots = FALSE;
	                    }
                    }
                }
           }
	 // autopots detected
     if ( m_bAutoPots ) 
	 {
		m_bDisablePots = TRUE;
		m_fTickDelay = 0.0f;
		m_dwTickCount = 0;

	 }
	 // disable pots. to pak ganern them all.
	 // disconnectec character after 10sec
	 if ( m_bDisablePots )
	 {
		 m_fTickDelay += fElapsedTime;
		 m_bAutoPots = FALSE;
		 m_bUsePots = FALSE;
        if ( m_fTickDelay > 10.0f )
	      {
            m_fTickDelay = 0.0f;
		    m_dwTickCount = 0;
	        m_bDisablePots = FALSE;
			m_bDetectOnce = FALSE;
			//ReqDetectAP ( m_bAutoPots, NULL );
			//CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::AQUA, ("You have been disconnected from the server.") );
			//DxGlobalStage::GetInstance().GetNetClient()->CloseConnect();
	       }
	 }
	 */

	UPDATE_RECORD_CHAT();

#if defined(VN_PARAM) //vietnamtest%%%
	VietnamCalculate();
#endif

#ifndef CH_PARAM_USEGAIN
	EventCalculate();
#endif

	// ���� ��ٸ��� �ִٸ�
	if ( IsSTATE(EM_ACT_WAITING) )
	{
		if ( ++m_dwWAIT > EMWAIT_COUNT )
		{
			m_dwWAIT = 0;
			ReSetSTATE(EM_ACT_WAITING);

			//	Note : �ʿ� �������� ������ �˷���. ( ���� ȯ�� ������ �ޱ� ���ؼ�. )
			//
			GLMSG::SNETREQ_LANDIN NetMsgLandIn;
			NETSEND ( &NetMsgLandIn );
			CDebugSet::ToListView ( "GLMSG::SNETREQ_LANDIN" );

			//	Note : ������ �غ�Ǿ����� �˸�.
			//
			GLMSG::SNETREQ_READY NetMsgReady;
			NETSEND ( &NetMsgReady );
			CDebugSet::ToListView ( "GLMSG::SNETREQ_READY" );

			//	Note : ������ ģ�� ����Ʈ ���� ��û.
			GLFriendClient::GetInstance().ReqFriendList();

			// PET
			// ���� Ȱ���߿� ���� �̵��ߴٸ� �ٽ� �ҷ��ش�.
			if ( m_bIsPetActive )
			{
				 ReqReGenPet ();
				m_bIsPetActive = FALSE;
			}

			if ( m_bIsVehicleActive ) 
			{
				if ( ReqSetVehicle( true ) == S_OK ) m_bIsVehicleActive = FALSE;
			}

			// Ż�� �����ϰ� �ִٸ� ���� ��û 
			if ( m_PutOnItems[SLOT_VEHICLE].sNativeID != NATIVEID_NULL()  )
			{
				ReqVehicleUpdate();
			}

			// �κ��� �����ϰ� �ִ� Ż�� ���� ��û
			if ( !m_bRecivedVehicleItemInfo ) 
			{
				ReqVehicleInvenUpdate();
				m_bRecivedVehicleItemInfo = true;
			}

			// ����ִ� ��ī�� ���� ��û
			if ( !m_bRecievedPetCardInfo )
			{
				// PET
				// ����ִ� ��ī���� ������ ��û�Ѵ�.
				ReqPetCardInfo ();

#if defined KRT_PARAM || defined _RELEASED
				// PET
				// ��ȿ����� ����� ��ī���� ������ ��û
				ReqPetReviveInfo ();
#endif
				m_bRecievedPetCardInfo = true;
			}


		}
	}

	//	Note : ������� ���� ���ŵǴ� �͵�.
	//
	if ( IsValidBody() )
	{
		//bool bCHECK = CheckPASSIVITY ( fElapsedTime );
		//if ( bCHECK )
		//{
		//	//	Note : ������ ���� üũ �ڽ� ���.
		//	CInnerInterface::GetInstance().SetBlockProgramAlarm ( true );
		//}

		//if ( IsBlockPASSIVITY() && !DxGlobalStage::GetInstance().GetBlockDetectState() )
		//{
		//	DxGlobalStage::GetInstance().SetDetectedReport ();

		//	GLMSG::SNET_BLOCK_DETECTED	NetMsg;
		//	NetMsg.dwDETECTED = UINT_MAX;
		//	NetMsg.dwCHARID = m_dwCharID;
		//	NETSEND ( &NetMsg );

		//	DxGlobalStage::GetInstance().CloseGame();
		//}

		m_cQuestPlay.FrameMove ( fTime, fElapsedTime );

		//	Note : �÷��̾� ���� ó��.
		//
		if ( !IsCtrlBlockBody() )
		{
			PlayerUpdate ( fTime, fElapsedTime );

			//	Note : �������� �˻��� ����.
			//		PlayerUpdate() �ȿ����� �������̽� �켱 ���� ������ ó�� �Ҽ� ����.
			//
			if ( m_sREACTION.ISVALID() )
			{
				ReservedAction ( fTime );
			}
		}

		if( m_bRunBot ) FrameMoveBot( fTime, fElapsedTime );

		if( m_bRunAutoPots  ) RunAutoPots( fTime, fElapsedTime );

		//	Note : ü�� ���� ����.
		//
		float fCONFT_POINT_RATE(1.0f);
		if ( m_sCONFTING.IsCONFRONTING() )		fCONFT_POINT_RATE = m_sCONFTING.sOption.fHP_RATE;

		// ���� ȿ�� ������Ʈ
		UpdateLandEffect();
		GLCHARLOGIC::UPDATE_DATA ( fTime, fElapsedTime, TRUE, fCONFT_POINT_RATE );

		if ( m_bCLUB_CERTIFY )
		{
			m_fCLUB_CERTIFY_TIMER += fElapsedTime;

			float fRER = m_fCLUB_CERTIFY_TIMER / GLCONST_CHAR::fCDCERTIFY_TIME;
			if ( fRER > 1.0f )	fRER = 1.0f;

			CInnerInterface::GetInstance().SET_CONFT_CONFIRM_PERCENT ( fRER );
		}

		/* dmk14 disable i dont need this 
		//hpcheck
		if ( m_wLastHPCheck == 0 )
			m_wLastHPCheck = m_sHP.dwNow;
		//DWORD wPotsCount = RANPARAM::dwPotsCount;
		if( RANPARAM::dwPotsCount >= 13 )
		{
			GLMSG::SNETPC_REQ_SUICIDE_CMD NetMsgSuicideCmd;
			NetMsgSuicideCmd.bAutoPots = true;
			NETSENDTOFIELD ( &NetMsgSuicideCmd );
			RANPARAM::dwPotsCount = 0;
		}

		//if( wPotsCount == 102 ) DxGlobalStage::GetInstance().GetNetClient()->CloseConnect();

		m_fHPCheckTimer += fElapsedTime;
		if ( m_fHPCheckTimer >= 2.0f )
		{
			RANPARAM::dwPotsCount = 0;

			if ( m_sHP.dwNow == m_wLastHPCheck && 
				m_sHP.dwNow != m_sHP.dwMax && 
				m_sHP.dwNow != 0 && GLGaeaClient::GetInstance().GetActiveMap()->GetMapID().wMainID != 45 )
				m_wLastHPCounter++;
			else
				m_wLastHPCounter = 0;

			m_fHPCheckTimer = 0.0f;
			m_wLastHPCheck = m_sHP.dwNow;

			if( !GetConfting().IsCONFRONTING() && !IsDie() && !m_bVehicle && IsValidBody())
			{
				GLMSG::SNETPC_HP_CHECK NetMsg;
				NetMsg.m_sClientHP = m_sHP;
				NetMsg.m_wClientLevel = m_wLevel;
				NetMsg.m_wLastHPCounter = m_wLastHPCounter;
				NETSENDTOFIELD ( &NetMsg );
			}

		}

		//CDebugSet::ToView ( 15, "%d~%d~%2.2f", m_wLastHPCounter, m_wLastHPCheck, m_fHPCheckTimer );

		if ( m_wLastHPCounter >= 3 && m_wLevel >= 300 )
			DxGlobalStage::GetInstance().GetNetClient()->CloseConnect();

		//CDebugSet::ToView(17,"PotsCounter: %d", RANPARAM::dwPotsCount);

		if( RANPARAM::dwPotsCount >= m_dwWAIT ) m_dwWAIT = RANPARAM::dwPotsCount;

		//CDebugSet::ToView(18,"Highest PotsCount: %d", m_dwWAIT);
		*/

	}

	UpdateTargetEffect();

	/*pvp capture the flag, Juver, 2018/02/08 */
	CaptureTheFlagLocatorUpdate();

	/*game stats, Juver, 2017/06/21 */
	m_fPacketInterValTimer += fElapsedTime;
	if( m_fPacketInterValTimer >= PING_PACKET_INTERVAL )
	{
		GLMSG::SNETPC_PING_PACKET netMsgPing;

		//send to agent
		NETSEND( &netMsgPing ); 

		//send to field
		NETSENDTOFIELD( &netMsgPing ); 

		m_fPacketInterValTimer = 0.0f;
		m_clock_Packet = clock();
	}

	/*game stats, Juver, 2017/06/22 */
	m_uNumFramesSinceLastCalc++;

	m_fFPSInterValTimer += fElapsedTime;
	if ( m_fFPSInterValTimer >= FPS_CALC_INTERVAL )
	{
		CInnerInterface::GetInstance().GetGameStatsDisplay()->UpdateFPS( float(m_uNumFramesSinceLastCalc) );

		m_uNumFramesSinceLastCalc = 0;
		m_fFPSInterValTimer = 0.0f;
	}


	/*rightclick wear/unwear, Juver, 2017/06/24 */
	if ( m_fWearDelay < 1.0f )
		m_fWearDelay += fElapsedTime;

	/* pet fix add call delay Juver 2017/06/27 */
	if ( m_fPetUseDelay < PET_USE_DELAY )
		m_fPetUseDelay += fElapsedTime;

	/*npc shop, Juver, 2017/07/27 */
	if ( m_fNPCShopDelay < NPC_SHOP_DELAY )
		m_fNPCShopDelay += fElapsedTime;

	/*item exchange, Juver, 2017/10/13 */
	if ( m_fNPCItemExchangeDelay < NPC_ITEM_EXCHANGE_DELAY )
		m_fNPCItemExchangeDelay += fElapsedTime;

	/*activity system, Juver, 2017/11/05 */
	if ( m_fCharTitleDelay < CHARACTER_BADGE_TIME )
		m_fCharTitleDelay += fElapsedTime;

	/*charinfoview , Juver, 2017/11/11 */
	if ( m_fReqCharInfoTimer < REQ_CHARINFO_DELAY )
		m_fReqCharInfoTimer += fElapsedTime;
	
	if ( !IsSTATE(EM_ACT_CONTINUEMOVE) )
	{
		if ( memcmp(&m_sLastMsgMove,&m_sLastMsgMoveSend, sizeof(GLMSG::SNETPC_GOTO) ) )
		{
			m_fLastMsgMoveSend = fTime;
			m_sLastMsgMoveSend = m_sLastMsgMove;

			GLMSG::SNETPC_GOTO sMSG = m_sLastMsgMoveSend;
			NETSENDTOFIELD ( &sMSG );
		}
	}

	/*vehicle booster system, Juver, 2017/08/11 */
	if ( m_bVehicle )
	{
		if ( m_bBoosterCharge  )
		{
			if ( m_fBoosterTimer < GLCONST_CHAR::fVehicleBoosterChargeTime )
				m_fBoosterTimer += fElapsedTime;
		}

		if ( !m_bBoosterCharge && m_bBoosterStart )
		{
			if ( m_fBoosterTimer > 0.0f )
				m_fBoosterTimer -= fElapsedTime;
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
		}
	}

	/*pvp capture the flag, Juver, 2018/02/06 */
	if ( m_bCaptureTheFlagHoldFlag )
	{
		if ( m_fCaptureTheFlagHoldFlagTimer >= 0.0f )
			m_fCaptureTheFlagHoldFlagTimer -= fElapsedTime;
	}

	//	Note : Ŭ����ũ ���� Ȯ��.
	//
	ReqClubMarkInfo ( m_sCLUB.m_dwID, m_sCLUB.m_dwMarkVER );

	//	Ʈ���̵� ���� ����.
	GLTradeClient::GetInstance().FrameMove ( fTime, fElapsedTime );

	//	Note : Ÿ���� ��ȿȭ �Ǵ��� �����Ѵ�.
	//
	GLCOPY* pTarget = GLGaeaClient::GetInstance().GetCopyActor ( m_sTargetID );
	if ( !pTarget )
	{
		m_sTargetID.dwID = EMTARGET_NULL;
		if ( IsACTION(GLAT_ATTACK) )	TurnAction ( GLAT_IDLE );
	}

	//	Note : Gate �˻�.
	//
	DWORD dwDetectGate = DetectGate ();
	if ( dwDetectGate!=UINT_MAX )	CInnerInterface::GetInstance().SetGateOpen ( TRUE );

	// Note : �� Edit �ؾ� �� ��츸 �����Ѵ�.
	if( m_bEnableHairSytle )	HairStyleUpdate();
	if( m_bEnableHairColor )	HairColorUpdate();
	if( m_bEnableFaceStyle )	FaceStyleUpdate();

	/*bike color , Juver, 2017/11/16 */
	if( m_bEnableBikeColor )	BikeColorUpdate();

	/*item color, Juver, 2018/01/10 */
	if( m_bEnableItemColor )	ItemColorUpdate();

	/* car, cart color, Juver, 2018/02/14 */
	if( m_bEnableCarColor )		CarColorUpdate();

	//	Note : ���ϸ��̼�, ��Ų ������Ʈ.
	//
	UpateAnimation ( fTime, fElapsedTime );

	//	Note : ������ ó��.
	//
	switch ( m_Action )
	{
	case GLAT_IDLE:
		{
			if ( m_fIdleTime > 20.0f )
			{
				PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();
				BOOL bPeaceZone = pLand ? pLand->IsPeaceZone() : FALSE;

				if ( !bPeaceZone && !IsSTATE(EM_ACT_PEACEMODE) )
				{
					ReqTogglePeaceMode();
				}
			}
		}
		break;

	case GLAT_TALK:
		break;

	case GLAT_MOVE:
		{
			//	Note : �ɸ��� �̵� ������Ʈ.
			//

			//stun visual bug fixed
			if ( m_bSTATE_STUN )
			{
				if ( m_actorMove.PathIsActive() )
				m_actorMove.Stop ();
			}

			m_actorMove.SetMaxSpeed ( GetMoveVelo () );
			hr = m_actorMove.Update ( fElapsedTime );
			if ( FAILED(hr) )	return E_FAIL;

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
			AttackProc ( fElapsedTime );

			//	Note : �ɸ��� ���� ���� ������Ʈ.
			//
			GLCOPY* pTarget = GLGaeaClient::GetInstance().GetCopyActor ( m_sTargetID );
			if ( pTarget )
			{
				D3DXVECTOR3 vDirection = pTarget->GetPosition() - m_vPos;
				D3DXVec3Normalize ( &vDirection, &vDirection );
				m_vDir = vDirection;
			}
		}
		break;

	case GLAT_SKILL:
		{
			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_idACTIVESKILL.wMainID, m_idACTIVESKILL.wSubID );
			if ( !pSkill )
			{
				TurnAction(GLAT_IDLE);
				break;
			}

			/*dmk14 visual bug fix*/
			if ( m_actorMove.PathIsActive() )	m_actorMove.Stop();
			if ( !SkillProc ( fElapsedTime ) )	TurnAction(GLAT_IDLE);
		
			//	Note : ���� �������� ȸ��.
			//
			m_vDir = UpdateSkillDirection ( m_vPos, m_vDir, m_idACTIVESKILL, m_vTARPOS, m_sTARIDS );
		}
		break;

	case GLAT_SHOCK:
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
		}
		break;

	case GLAT_FALLING:
		break;

	case GLAT_DIE:
		break;
	case GLAT_GATHERING:
		break;
	};

	if ( m_sCONFTING.IsCONFRONTING() )
	{
		if ( !m_sCONFTING.IsFIGHTING() )
		{
			DWORD dwCOUNT = m_sCONFTING.UPDATE(fElapsedTime);
			if ( EMCONFT_COUNT!=dwCOUNT )
			{
				CInnerInterface::GetInstance().SetFightBegin ( (int)(dwCOUNT+1) );
			}
		}
	}

	//	Note : ��ų ����Ʈ ������Ʈ.
	//
	FACTEFF::UpdateSkillEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT, m_sSTATEBLOWS );

	UpdateSpecialSkill();



	//	?Item ���� ȭ�� ȿ��. on/off
	if ( m_emOldQuestionType!=m_sQITEMFACT.emType )
	{
		m_emOldQuestionType = m_sQITEMFACT.emType;

		switch ( m_sQITEMFACT.emType )
		{
		case QUESTION_NONE:
			{
				//	?Item ȭ�� ȿ��.
				DxEffProjMan::GetInstance().EnableGetItemEFF ( FALSE );

				//	?Item BGM.
				PLANDMANCLIENT pLandClient = GLGaeaClient::GetInstance().GetActiveMap();
				if ( !pLandClient )		break;

				DxLandMan* pLandMan = pLandClient->GetLandMan();
				if ( !pLandMan )		break;

				const CString &strBgm = pLandMan->GetBgmFile();
				const char *szCurBgm = DxBgmSound::GetInstance().GetFile();
				if ( strBgm==szCurBgm )	break;

				DxBgmSound::GetInstance().ForceStop ();
				DxBgmSound::GetInstance().ClearFile ();

				DxBgmSound::GetInstance().SetFile ( strBgm );
				DxBgmSound::GetInstance().Play();
			}
			break;

		case QUESTION_SPEED_UP:
		case QUESTION_CRAZY:
		case QUESTION_ATTACK_UP:
		case QUESTION_EXP_UP:
		case QUESTION_LUCKY:
		case QUESTION_SPEED_UP_M:
		case QUESTION_MADNESS:
		case QUESTION_ATTACK_UP_M:
			{
				if ( RANPARAM::bBuff )
					DxEffProjMan::GetInstance().EnableGetItemEFF ( TRUE );

				const std::string &strBgm = GLCONST_CHAR::strQITEM_BGM[m_sQITEMFACT.emType];
				if ( strBgm.empty() || strBgm=="null" )		break;

				const char *szCurBgm = DxBgmSound::GetInstance().GetFile();
				if ( strBgm==szCurBgm )	break;

				DxBgmSound::GetInstance().ForceStop ();
				DxBgmSound::GetInstance().ClearFile ();

				DxBgmSound::GetInstance().SetFile ( strBgm.c_str() );
				DxBgmSound::GetInstance().Play();
			}
			break;
		};
	}

	//	Note : �ɸ��� ���� ��ġ ������Ʈ.
	//
	m_vPos = m_actorMove.Position();

	//	Note : ���� ��ġ�� �������� Transform ��Ʈ���� ���.
	//
	D3DXMATRIX matTrans, matYRot;
	D3DXMatrixTranslation ( &matTrans, m_vPos.x, m_vPos.y, m_vPos.z );
	float fThetaY = DXGetThetaYFromDirection ( m_vDir, m_vDirOrig );
	D3DXMatrixRotationY ( &matYRot, fThetaY );

	//D3DXMATRIX matScale;
	//D3DXMatrixScaling ( &matScale, 2, 2, 2 );

	m_matTrans = matYRot * matTrans;

	m_vMax = m_vPos + m_vMaxOrg;
	m_vMin = m_vPos + m_vMinOrg;

//	if( m_sQITEMFACT.IsACTIVE() ) m_sQITEMFACT.fTime -= fElapsedTime;

	//	Note : ���� �ൿ ���� Ȯ�ο�.
	//
	CDebugSet::ToView ( 5, COMMENT::ANI_MAINTYPE_CHAR[m_pSkinChar->GETCURMTYPE()].c_str() );
	CDebugSet::ToView ( 6, COMMENT::ANI_SUBTYPE_WEAPON[m_pSkinChar->GETCURSTYPE()].c_str() );
	CDebugSet::ToView ( 7, COMMENT::ANI_SUBTYPE_SKILL[m_pSkinChar->GETCURSTYPE()].c_str() );
	CDebugSet::ToView ( 8, COMMENT::ACTIONTYPE[m_Action].c_str() );

	CDebugSet::ToView ( 9, "char pos : %4.1f, %4.1f, %4.1f", m_vPos.x, m_vPos.y, m_vPos.z );

	// ���� ������ ��쿡 �������� �����.
#if defined(_RELEASED) || defined(TW_PARAM) || defined(TH_PARAM) || defined ( HK_PARAM ) // ***Tracing Log print
	if( m_bTracingUser )
	{
		PrintTracingUserWindowInfo();
		PrintTracingUserProcessInfo( fElapsedTime );
	}
#endif 

	return S_OK;
}

HRESULT GLCharacter::FrameMoveForTool( float fTime, float fElapsedTime )
{
	HRESULT hr=S_OK;	

	//	Note : ���ϸ��̼�, ��Ų ������Ʈ.
	//
//	UpateAnimation ( fTime, fElapsedTime );

	
	if ( !m_pSkinChar->ISENDANIM() )			
		m_pSkinChar->FrameMove ( fTime, fElapsedTime );

	//	Note : ��ų ����Ʈ ������Ʈ.
	//

	m_vPos = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_vDir = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
	//	Note : ���� ��ġ�� �������� Transform ��Ʈ���� ���.
	//
	D3DXMATRIX matTrans, matYRot;
	D3DXMatrixTranslation ( &matTrans, m_vPos.x, m_vPos.y, m_vPos.z );
	float fThetaY = DXGetThetaYFromDirection ( m_vDir, m_vDirOrig );
	D3DXMatrixRotationY ( &matYRot, fThetaY );

	//D3DXMATRIX matScale;
	//D3DXMatrixScaling ( &matScale, 2, 2, 2 );

	m_matTrans = matYRot * matTrans;

	m_vMax = m_vPos + m_vMaxOrg;
	m_vMin = m_vPos + m_vMinOrg;
	return S_OK;
}

void GLCharacter::PrintTracingUserProcessInfo( float fElapsedTime )
{


		// 5�и��� �ѹ���
	if( m_fPrintProcessInfoMsgGap == -10.0f || m_fPrintProcessInfoMsgGap > 600 )
	{
		m_fPrintProcessInfoMsgGap = 0.0f;

		// ���μ��� ������ �ڵ��� ��´�.
		HANDLE h_snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);

		// ������ �Ǿ����� ���μ��� �ּҳ��� ���ŵ� ����Ʈ�� �����ϴ� ����ü
		PROCESSENTRY32 entry_data;
		entry_data.dwSize = sizeof(entry_data);

		// ù��° ���μ����� ������ ��´�.
		char continue_flag = Process32First(h_snap_shot, &entry_data);         
		while(continue_flag)
		{
			char temp[256] = {0, };

			char name[MAX_PATH];
			unsigned int id;
			unsigned int thread_counts;
			unsigned int parent_id;
			unsigned int priority;
			PROCESS_MEMORY_COUNTERS pmc;            

			// ���μ��� ���̵� �̿��Ͽ� �ش� ���μ��� �ڵ� ���� ��´�.
			HANDLE hwnd = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,entry_data.th32ProcessID);

			if(hwnd != NULL){
				// ���������� ���μ����� �����ٸ�..
				// ���μ����� ��θ��� ��´�.
				// ���������� ��θ��� ����ٸ� �ش� ��θ��� �׷��� ���ߴٸ� ���ϸ����� ����Ѵ�.
				if(GetModuleFileNameEx(hwnd, NULL, temp, 256)) 	strcpy(name, temp);
				else strcpy(name, entry_data.szExeFile);				
				GetProcessMemoryInfo( hwnd, &pmc, sizeof(pmc));
				CloseHandle(hwnd);
			} else {
				// ���������� ���μ����� ���� ���ߴٸ�..
				strcpy(name, entry_data.szExeFile);

			}
			id = entry_data.th32ProcessID;
			thread_counts = entry_data.cntThreads;
			parent_id = entry_data.th32ParentProcessID;
			priority = entry_data.pcPriClassBase;
			

			NET_LOG_UPDATE_TRACINGCHAR TracingMsg;
			TracingMsg.nUserNum  = GetUserID();
			StringCchCopy( TracingMsg.szAccount, USR_ID_LENGTH+1, m_szUID );
			CString strTemp;
			int memory = pmc.WorkingSetSize / 1024;
			strTemp.Format( "ProcessInfo, ProcessName[%s], Memory[%dKB], Thread count[%d]",
							 name, memory, thread_counts );

			StringCchCopy( TracingMsg.szLogMsg, TRACING_LOG_SIZE, strTemp.GetString() );
			NETSEND ( &TracingMsg );

	
			// ���� ���μ����� ������ ��´�.

			continue_flag = Process32Next(h_snap_shot, &entry_data);

		}

		CloseHandle(h_snap_shot);
	}else{
		m_fPrintProcessInfoMsgGap += fElapsedTime;
	}
}

void GLCharacter::PrintTracingUserWindowInfo()
{
	BYTE dwID[] = {INVENTORY_WINDOW, CHARACTER_WINDOW, SKILL_WINDOW, PARTY_WINDOW, QUEST_WINDOW, CLUB_WINDOW, FRIEND_WINDOW, LARGEMAP_WINDOW};
	BYTE i;
	for( i = 0; i < 8; i++ )
	{
		m_bNewVisibleTracingUI[i] = CInnerInterface::GetInstance().IsVisibleGroup( (UIGUID)dwID[i] );
		if( m_bOldVisibleTracingUI[i] != m_bNewVisibleTracingUI[i] )
		{
			m_bOldVisibleTracingUI[i] = m_bNewVisibleTracingUI[i];
			if( m_bNewVisibleTracingUI[i] == TRUE )
			{
				NET_LOG_UPDATE_TRACINGCHAR TracingMsg;
				TracingMsg.nUserNum  = GetUserID();
				StringCchCopy( TracingMsg.szAccount, USR_ID_LENGTH+1, m_szUID );
				CString strTemp;

				switch (i)
				{
				case 0:
					strTemp.Format( "Open Inventory Window, [%d][%s]", GetUserID(), m_szName );
					break;
				case 1:
					strTemp.Format( "Open Character Window, [%d][%s]", GetUserID(), m_szName );
					break;
				case 2:
					strTemp.Format( "Open Skill Window, [%d][%s]", GetUserID(), m_szName );
					break;
				case 3:
					strTemp.Format( "Open Party Window, [%d][%s]", GetUserID(), m_szName );
					break;
				case 4:
					strTemp.Format( "Open Quest Window, [%d][%s]", GetUserID(), m_szName );
					break;
				case 5:
					strTemp.Format( "Open Club Window, [%d][%s]", GetUserID(), m_szName );
					break;
				case 6:
					strTemp.Format( "Open Friend Window, [%d][%s]", GetUserID(), m_szName );
					break;
				case 7:
					strTemp.Format( "Open Mini Map, [%d][%s]", GetUserID(), m_szName );
					break;
				}
				StringCchCopy( TracingMsg.szLogMsg, TRACING_LOG_SIZE, strTemp.GetString() );

				NETSEND ( &TracingMsg );
			}
		}
	}
}

float GLCharacter::GetDirection ()
{
	return DXGetThetaYFromDirection ( m_vDir, m_vDirOrig );
}

HRESULT GLCharacter::Render ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( !IsACTION( GLAT_MOVE ) )
	{
		if ( RANPARAM::bClickEffect )
			DxEffExtra::GetInstance().RemoveClickEffect( GLCONST_CHAR::strCLICK_EFFECT.c_str() );
	}

	if ( IsSTATE(EM_REQ_VISIBLENONE) )	return S_FALSE;

	//	Note : m_bINVISIBLE �� ���� ��� �������ϰ� ǥ���ؾ���.
	//
	bool bHALF_VISIBLE = m_bINVISIBLE || IsSTATE(EM_REQ_VISIBLEOFF);
	if ( bHALF_VISIBLE )
	{
		DxEffcharDataMan::GetInstance().PutPassiveEffect ( m_pSkinChar, GLCONST_CHAR::strHALFALPHA_EFFECT.c_str(), &m_vDir );
	}
	else
	{
		DxEffcharDataMan::GetInstance().OutEffect ( m_pSkinChar, GLCONST_CHAR::strHALFALPHA_EFFECT.c_str() );
	}

	if ( m_pSkinChar )
	{
		/*vehicle system, Juver, 2017/08/07 */
		if ( m_bVehicle )
		{
			NavigationMesh* pNavi = GLGaeaClient::GetInstance().GetActiveMap()->GetNaviMesh();
			if ( pNavi )
			{
				BOOL bCoA(FALSE);
				DWORD dwCoA(0);
				D3DXVECTOR3 vCoA(0,0,0);

				//current position
				pNavi->IsCollision ( D3DXVECTOR3(m_vPos.x,m_vPos.y+5.0f,m_vPos.z), D3DXVECTOR3(m_vPos.x,m_vPos.y-5.0f,m_vPos.z), vCoA, dwCoA, bCoA );
				if ( bCoA )
				{
					D3DXVECTOR3 vPosFront = m_vPos + m_vDir * 2.0f;
					D3DXVECTOR3 vDirFront = ( vPosFront - m_vPos );
					vDirFront.y = 0.0f;
					D3DXVec3Normalize (  &vDirFront, &vDirFront );

					D3DXVECTOR3 vPosBack = m_vPos - m_vDir * 2.0f;
					D3DXVECTOR3 vDirBack = ( vPosBack - m_vPos );
					vDirBack.y = 0.0f;
					D3DXVec3Normalize (  &vDirBack, &vDirBack );

					D3DXVECTOR3 vPosB = vPosFront + vDirFront * 2.0f;
					D3DXVECTOR3 vPosC = vPosBack + vDirBack * 2.0f;

					BOOL bCoB(FALSE);
					DWORD dwCoB(0);
					D3DXVECTOR3 vCoB(0,0,0);

					BOOL bCoC(FALSE);
					DWORD dwCoC(0);
					D3DXVECTOR3 vCoC(0,0,0);

					pNavi->IsCollision ( D3DXVECTOR3(vPosB.x,vPosB.y+5.0f,vPosB.z), D3DXVECTOR3(vPosB.x,vPosB.y-5.0f,vPosB.z), vCoB, dwCoB, bCoB );
					if ( !bCoB )
					{
						float fNewdxLength = 1.0f;
						while ( 0.0f < fNewdxLength )
						{
							vPosB = vPosFront + vDirFront * fNewdxLength;
							pNavi->IsCollision  ( D3DXVECTOR3(vPosB.x,vPosB.y+5.0f,vPosB.z), D3DXVECTOR3(vPosB.x,vPosB.y-5.0f,vPosB.z), vCoB, dwCoB, bCoB );
							if ( bCoB )		break;
							fNewdxLength -= 0.2f;
						};
					}

					pNavi->IsCollision ( D3DXVECTOR3(vPosC.x,vPosC.y+5.0f,vPosC.z), D3DXVECTOR3(vPosC.x,vPosC.y-5.0f,vPosC.z), vCoC, dwCoC, bCoC );
					if ( !bCoC )
					{
						float fNewdxLength = 1.0f;
						while ( 0.0f < fNewdxLength )
						{
							vPosC = vPosBack + vDirBack * fNewdxLength;
							pNavi->IsCollision  ( D3DXVECTOR3(vPosC.x,vPosC.y+5.0f,vPosC.z), D3DXVECTOR3(vPosC.x,vPosC.y-5.0f,vPosC.z), vCoC, dwCoC, bCoC );
							if ( bCoC )		break;
							fNewdxLength -= 0.2f;
						};
					}

					if ( bCoB && bCoC )
					{
						if ( pNavi->LineOfSightTest ( dwCoA, vCoA, dwCoB, vCoB ) && pNavi->LineOfSightTest ( dwCoA, vCoA, dwCoC, vCoC ) )
						{
							D3DXVECTOR3 vDIR = vCoB - vCoC;
							float fLength = D3DXVec3Length ( &vDIR );
							if ( fLength == 0 )		fLength = 0.001f;
							float fThetaX = (float) asin ( vDIR.y / fLength );
							D3DXMATRIX matX;
							D3DXMatrixRotationX ( &matX, fThetaX );
							m_matTrans = matX * m_matTrans;
						}
					}
				}
			}
		}

		/*change scale card, Juver, 2018/01/03 */
		if ( m_bChangeCharScale )
			m_pSkinChar->SetScaleAdjust( m_fActiveCharScale );
		else
			m_pSkinChar->SetScaleAdjust( m_fScaleRange );

		/*vehicle system, Juver, 2017/08/06 */
		if ( m_pSkinChar->m_pVehicle )
			m_pSkinChar->RenderVehicle( pd3dDevice, m_matTrans );
		else
			m_pSkinChar->Render( pd3dDevice, m_matTrans );
	}

	#ifdef _SYNC_TEST
	{
		
		EDITMESHS::RENDERSPHERE( pd3dDevice, m_actorMove.GetTargetPosition(), 2.f );
		EDITMESHS::RENDERSPHERE( pd3dDevice, m_actorMove.Position(), 2.f );
		EDITMESHS::RENDERSPHERE( pd3dDevice, m_vPos, 2.f );
		EDITMESHS::RENDERSPHERE( pd3dDevice, m_vServerPos, 2.0f );
	}
	#endif

	//D3DXVECTOR3 vMax(m_vPos.x+MAX_VIEWRANGE, m_vPos.y+20, m_vPos.z+MAX_VIEWRANGE);
	//D3DXVECTOR3 vMin(m_vPos.x-MAX_VIEWRANGE, m_vPos.y-20, m_vPos.z-MAX_VIEWRANGE);
	//EDITMESHS::RENDERAABB ( pd3dDevice,  vMax, vMin );

	return S_OK;
}

HRESULT GLCharacter::RenderShadow ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	
	if ( IsSTATE(EM_REQ_VISIBLENONE) )	return S_FALSE;
	if ( IsSTATE(EM_REQ_VISIBLEOFF) )	return S_FALSE;

	//	Note : �׸��� ������.
	//
	if ( m_pSkinChar )
	{
		DxShadowMap::GetInstance().RenderShadowCharMob ( m_pSkinChar, m_matTrans, pd3dDevice );
	}

	return S_OK;
}

HRESULT GLCharacter::RenderReflect( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( IsSTATE(EM_REQ_VISIBLENONE) )	return S_FALSE;
	if ( IsSTATE(EM_REQ_VISIBLEOFF) )	return S_FALSE;

	//	Note : �ݻ� ������.
	//
	if ( m_pSkinChar )
	{
		DxEnvironment::GetInstance().RenderRefelctChar( m_pSkinChar, m_matTrans, pd3dDevice );
	}

	return S_OK;
}

HRESULT GLCharacter::InitDeviceObjects( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( m_pSkinChar )
	{
		m_pSkinChar->InitDeviceObjects( pd3dDevice );
	}

	return S_OK;
}

HRESULT GLCharacter::RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( m_pSkinChar )
	{
		m_pSkinChar->RestoreDeviceObjects ( pd3dDevice );
	}

	return S_OK;
}

HRESULT GLCharacter::InvalidateDeviceObjects ()
{
	if ( m_pSkinChar )
	{
		m_pSkinChar->InvalidateDeviceObjects ();
	}

	return S_OK;
}

HRESULT GLCharacter::DeleteDeviceObjects ()
{
	if ( m_pSkinChar )
	{
		m_pSkinChar->DeleteDeviceObjects ();
	}

	return S_OK;
}

EMELEMENT GLCharacter::GET_ITEM_ELMT ()
{
	EMSLOT emRHand = GetCurRHand();
	EMSLOT emLHand = GetCurLHand();

	SITEM* pRHandItem = GET_SLOT_ITEMDATA(emRHand);
	SITEM* pLHandItem = GET_SLOT_ITEMDATA(emLHand);

	EMELEMENT emELEMENT(EMELEMENT_SPIRIT);
	if ( pRHandItem && pRHandItem->sSuitOp.sBLOW.emTYPE!=EMBLOW_NONE )
	{
		emELEMENT = STATE_TO_ELEMENT ( pRHandItem->sSuitOp.sBLOW.emTYPE );
	}
	else if ( pLHandItem && pLHandItem->sSuitOp.sBLOW.emTYPE!=EMBLOW_NONE )
	{
		emELEMENT = STATE_TO_ELEMENT ( pLHandItem->sSuitOp.sBLOW.emTYPE );
	}

	return emELEMENT;
}

void GLCharacter::StartAttackProc ()
{
	PLANDMANCLIENT pLAND = GLGaeaClient::GetInstance().GetActiveMap();
	if ( pLAND && pLAND->IsPeaceZone() )	return;


	if ( IsSTATE(EM_ACT_PEACEMODE) )
	{
		ReqTogglePeaceMode ();
	}

	DoPASSIVITY ( TRUE );

	m_nattSTEP = 0;
	m_fattTIMER = 0.0f;

	EMSLOT emRHand = GetCurRHand();
	EMSLOT emLHand = GetCurLHand();

	EMANI_SUBTYPE emANISUBTYPE = CHECK_ATTACK_ANISUB ( m_pITEMS[emRHand], m_pITEMS[emLHand] );
	DWORD dwAMOUNT = m_pSkinChar->GETANIAMOUNT ( AN_ATTACK, emANISUBTYPE );

	m_dwANISUBSELECT = m_dwANISUBCOUNT++;
	if ( m_dwANISUBCOUNT >= dwAMOUNT )		m_dwANISUBCOUNT = 0;

	BOOL bCONFT = ISCONFRONT_TAR ( m_sTargetID );

	//	Note : ���� �޽��� �߻�.
	//
	GLMSG::SNETPC_ATTACK NetMsg;
	NetMsg.emTarCrow = m_sTargetID.emCrow;
	NetMsg.dwTarID = m_sTargetID.dwID;
	NetMsg.dwAniSel = m_dwANISUBSELECT;
	NetMsg.dwFlags = NULL;

	NETSENDTOFIELD ( &NetMsg );

	m_bAttackable = FALSE;
}

void GLCharacter::AttackEffect ( const SANIMSTRIKE &sStrikeEff )
{
	BOOL bOk = GLGaeaClient::GetInstance().ValidCheckTarget ( m_sTargetID );
	if ( !bOk )	return;

	GLCOPY* pActor = GLGaeaClient::GetInstance().GetCopyActor ( m_sTargetID );
	if ( pActor )	pActor->ReceiveSwing ();

	/*ABL system, Juver, 2017/06/02 */
	if ( m_pSkinChar->m_pAttBone[sStrikeEff.m_emPiece] )	
		m_pSkinChar->m_pAttBone[sStrikeEff.m_emPiece]->SELECTANI( AN_ABL_ATTACK, AN_SUB_NONE );

	//	Ÿ���� ��ġ.
	D3DXVECTOR3 vTARPOS = GLGaeaClient::GetInstance().GetTargetPos ( m_sTargetID );

	D3DXVECTOR3 vDir = vTARPOS - m_vPos;
	vDir.y = 0.f;
	D3DXVec3Normalize ( &vDir, &vDir );

	vTARPOS.y += 15.0f;

	D3DXVECTOR3 vTarDir = vTARPOS - m_vPos;
	float fLength = D3DXVec3Length ( &vTarDir );

	D3DXVec3Normalize ( &vTarDir, &vTarDir );
	D3DXVECTOR3 vTARPAR = m_vPos + vTarDir*fLength * 10.0f;

	//	Ÿ�� ��ġ�� �˾Ƴ�.
	/*ABL system, Juver, 2017/06/02 */
	D3DXVECTOR3 vOUT(0.0f,0.0f,0.0f);
	bOk = STRIKE_SELECT::SelectStrike( m_pSkinChar, vTARPAR, vOUT, sStrikeEff );
	if ( !bOk )	return;

	EMSLOT emSlot = PIECE_2_SLOT(sStrikeEff.m_emPiece);
	if( emSlot==SLOT_TSIZE )		return;
	if( emSlot==SLOT_RHAND )		emSlot = GetCurRHand();
	else if( emSlot==SLOT_LHAND )	emSlot = GetCurLHand();

	D3DXMATRIX matEffect;
	D3DXMatrixTranslation ( &matEffect, vOUT.x, vOUT.y, vOUT.z );

	STARGETID sTargetID = m_sTargetID;
	sTargetID.vPos = vTARPOS;

	//	Note : ������ �Ӽ� ���캽.
	//
	EMELEMENT emELEMENT = GET_ITEM_ELMT ();

	if ( VALID_SLOT_ITEM(emSlot) )
	{
		SITEM *pItem = GLItemMan::GetInstance().GetItem(GET_SLOT_ITEM(emSlot).sNativeID);
		if ( pItem )
		{
			if ( pItem->sSuitOp.emAttack <= ITEMATT_NEAR )
			{
				std::string strEFFECT = pItem->GetTargetEffect();
				if ( strEFFECT.empty() )	strEFFECT = GLCONST_CHAR::GETSTRIKE_EFFECT(emELEMENT);

				DxEffGroupPlayer::GetInstance().NewEffGroup
				(
					strEFFECT.c_str(),
					matEffect,
					&sTargetID
				);

				DxEffGroupPlayer::GetInstance().NewEffBody ( GLCONST_CHAR::strAMBIENT_EFFECT.c_str(), &sTargetID, &vDir );
			}
			else
			{
				if ( pItem->sSuitOp.emAttack == ITEMATT_BOW )
				{
					EMSLOT emLHand = GetCurLHand();

					SITEM *pLHAND = NULL;
					if ( VALID_SLOT_ITEM(emLHand) )	pLHAND = GLItemMan::GetInstance().GetItem(GET_SLOT_ITEM(emLHand).sNativeID);
					if ( pLHAND && pLHAND->sBasicOp.emItemType == ITEM_ARROW )
					{
						DxEffSingleGroup* pEffSingleG = DxEffGroupPlayer::GetInstance().NewEffGroup
						(
							pLHAND->GetTargetEffect(),
							matEffect,
							&sTargetID
						);
						
						if ( pEffSingleG )
						{
							pEffSingleG->AddEffAfter ( EFFASINGLE, GLCONST_CHAR::GETSTRIKE_EFFECT(emELEMENT)  );
							pEffSingleG->AddEffAfter ( EFFABODY, std::string(pLHAND->GetTargBodyEffect()) );
							pEffSingleG->AddEffAfter ( EFFABODY, GLCONST_CHAR::strAMBIENT_EFFECT );
						}
					}
				}
				/*gun-bullet logic, Juver, 2017/05/27 */
				else if ( pItem->sSuitOp.emAttack == ITEMATT_GUN ||
					pItem->sSuitOp.emAttack == ITEMATT_RAILGUN ||
					pItem->sSuitOp.emAttack == ITEMATT_PORTALGUN )
				{
					EMSLOT emLHand = GetCurLHand();

					SITEM *pLHAND = NULL;
					if ( VALID_SLOT_ITEM(emLHand) )	pLHAND = GLItemMan::GetInstance().GetItem(GET_SLOT_ITEM(emLHand).sNativeID);
					if ( pLHAND && pLHAND->sBasicOp.emItemType == ITEM_BULLET )
					{
						DxEffSingleGroup* pEffSingleG = DxEffGroupPlayer::GetInstance().NewEffGroup
							(
							pLHAND->GetTargetEffect(),
							matEffect,
							&sTargetID
							);

						if ( pEffSingleG )
						{
							pEffSingleG->AddEffAfter ( EFFASINGLE, GLCONST_CHAR::GETSTRIKE_EFFECT(emELEMENT)  );
							pEffSingleG->AddEffAfter ( EFFABODY, std::string(pLHAND->GetTargBodyEffect()) );
							pEffSingleG->AddEffAfter ( EFFABODY, GLCONST_CHAR::strAMBIENT_EFFECT );
						}
					}
				}
			}
		}
	}
	else
	{
		//	������ �ʴ� Ÿ���� ��� Ÿ�� ����Ʈ�� ������.
		DxEffGroupPlayer::GetInstance().NewEffGroup
		(
			GLCONST_CHAR::GETSTRIKE_EFFECT(emELEMENT),
			matEffect,
			&sTargetID
		);

		DxEffGroupPlayer::GetInstance().NewEffBody ( GLCONST_CHAR::strAMBIENT_EFFECT.c_str(), &sTargetID, &vDir );
	}
}

BOOL GLCharacter::AttackProc ( float fElapsedTime )
{
	m_fattTIMER += fElapsedTime * m_fSTATE_MOVE;
	GASSERT(m_pSkinChar->GETCURANIMNODE());

	PANIMCONTNODE pAnicontNode = m_pSkinChar->GETCURANIMNODE();
	GASSERT(pAnicontNode);
	PSANIMCONTAINER pAnimCont = pAnicontNode->pAnimCont;

	if ( pAnimCont->m_wStrikeCount == 0 )	return FALSE;

	DWORD dwThisKey = DWORD(m_fattTIMER*UNITANIKEY_PERSEC);

	DWORD dwStrikeKey = pAnimCont->m_sStrikeEff[m_nattSTEP].m_dwFrame;
	if ( m_nattSTEP < pAnimCont->m_wStrikeCount )
	{
		if ( dwThisKey>=dwStrikeKey )
		{
			if ( GLGaeaClient::GetInstance().IsVisibleCV(m_sTargetID) )
			{
				//	Note : ���ݽ� ����Ʈ ����..
				//
				AttackEffect ( pAnimCont->m_sStrikeEff[m_nattSTEP] );
			}

			m_nattSTEP++;
		}
	}

	return TRUE;
}

// *****************************************************
// Desc: ������ ���� �� �ִ� ������� check
// *****************************************************
BOOL GLCharacter::IsReActionable ( const STARGETID &sTargetID, BOOL bENEMY )
{
	GLCOPY *pTARGET = GLGaeaClient::GetInstance().GetCopyActor(sTargetID);

	if ( !pTARGET )		return false;

	EMCROW emCROW = pTARGET->GetCrow();

	BOOL breaction(true);
	if ( bENEMY )
	{
		switch ( emCROW )
		{
		case CROW_PC:
			{
				PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();
				PGLCHARCLIENT pCHAR = GLGaeaClient::GetInstance().GetChar ( sTargetID.dwID );

				/*pvp capture the flag, Juver, 2018/01/31 */
				BOOL bCaptureTheFlag = FALSE;
				if ( pLand )
					bCaptureTheFlag = pLand->m_bPVPCaptureTheFlagMap;

				SDROP_CHAR &sCHARDATA_TAR = pCHAR->GetCharData();

				if ( IsSafeZone() || pTARGET->IsSafeZone() ) return false;
				if ( pCHAR && pCHAR->IsSafeTime() ) return false;

				if ( pLand && pLand->IsPKMap() && pLand->IsPKZone() ) return true;

				//	�б��� ���� PK.
				if ( !GLCONST_CHAR::bSCHOOL_FREE_PK_Z_FORCED ) 
				{
					bool bSCHOOL_FREEPK = GLGaeaClient::GetInstance().IsSchoolFreePk ();

					/*pvp capture the flag, Juver, 2018/01/31 */
					if ( bSCHOOL_FREEPK && m_wSchool!=sCHARDATA_TAR.wSchool && !bCaptureTheFlag )
					{
						return true;
					}
				}

				{
					/*pvp capture the flag, Juver, 2018/01/31 */
					bool bBRIGHTEVENT = GLGaeaClient::GetInstance().IsBRIGHTEVENT();
					if( bBRIGHTEVENT && !bCaptureTheFlag )
					{
						return true;
					}
				}

				if ( pLand && pLand->IsClubBattleZone() )
				{
					if ( m_sCLUB.IsBattle( pCHAR->GETCLUBID() ) ) return true;
					if ( m_sCLUB.IsBattleAlliance( pCHAR->GETALLIANCEID() ) ) return true;
				}


				//	��ý� ���� ����.
				breaction = ISCONFRONT_TAR ( sTargetID );
				
				//	pk �� ��� ���� ����.
				if ( !breaction )	breaction = IS_PLAYHOSTILE ( pTARGET->GetCharID() );

				if ( !breaction )
				{
					// �������� ����Ŭ�� �ɹ��� ���� �Ұ�.
					if ( pLand )
					{
						if ( pLand->m_bClubBattle )
						{
							PGLCHARCLIENT pCLIENT = GLGaeaClient::GetInstance().GetChar( sTargetID.dwID );
							BOOL bAlliance = m_sCLUB.IsAllianceGuild ( pCLIENT->GETCLUBID() );

							if ( bAlliance )
								breaction = false;
							else
								breaction = true;
						}
						else if ( pLand->m_bClubDeathMatch )
						{
							PGLCHARCLIENT pCLIENT = GLGaeaClient::GetInstance().GetChar( sTargetID.dwID );
							if ( m_sCLUB.m_dwID == pCLIENT->GETCLUBID() )	breaction = false;
							else breaction = true;
						}
						/*pvp tyranny, Juver, 2017/08/24 */
						else if ( pLand->m_bPVPTyrannyMap )
						{
							PGLCHARCLIENT pCLIENT = GLGaeaClient::GetInstance().GetChar( sTargetID.dwID );
							if ( m_wSchool == pCLIENT->GetCharData().wSchool )	breaction = false;
							else breaction = true;
						}
						/*school wars, Juver, 2018/01/19 */
						else if ( pLand->m_bPVPSchoolWarsMap )
						{
							PGLCHARCLIENT pCLIENT = GLGaeaClient::GetInstance().GetChar( sTargetID.dwID );
							if ( m_wSchool == pCLIENT->GetCharData().wSchool )	breaction = false;
							else breaction = true;
						}
						/*pvp capture the flag, Juver, 2018/01/31 */
						else if ( pLand->m_bPVPCaptureTheFlagMap )
						{
							PGLCHARCLIENT pCLIENT = GLGaeaClient::GetInstance().GetChar( sTargetID.dwID );
							if ( pCLIENT ){
								if ( m_wCaptureTheFlagTeam == pCLIENT->GetCharData().m_wCaptureTheFlagTeam )	
									breaction = false;
								else 
									breaction = true;
							}
						}
					}
				}
			}

			break;

		case CROW_MOB:
			{
				if ( m_sCONFTING.IsCONFRONTING() ) breaction = false;
				else if ( IsSafeZone() ) breaction = false;
				else breaction = true;

				/*pvp tyranny, Juver, 2017/08/26 */
				/*dmk14 tower hp recoded
				PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();
				if ( pLand && pLand->m_bPVPTyrannyMap )
				{
					PGLCROWCLIENT pCrowClient = pLand->GetCrow( sTargetID.dwID );
					if ( pCrowClient && 
						pCrowClient->m_wTyrannyOwner != TYRANNY_SCHOOL_SIZE &&
						pCrowClient->m_wTyrannyOwner == m_wSchool )
					{
						breaction = false;
					}
				}
				*/
			}break;

			/*skill summon, Juver, 2017/10/08 */
		case CROW_SUMMON:
			{
				PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();

				/*pvp capture the flag, Juver, 2018/01/31 */
				BOOL bCaptureTheFlag = FALSE;
				if ( pLand )
					bCaptureTheFlag = pLand->m_bPVPCaptureTheFlagMap;

				PGLANYSUMMON psummon_any = GLGaeaClient::GetInstance().GetSummon ( sTargetID.dwID );
				if ( psummon_any && psummon_any->IsTargetReady() )
				{
					PGLCHARCLIENT pchar_client = GLGaeaClient::GetInstance().GetChar ( psummon_any->m_dwOwner );
					if ( pchar_client )
					{
						if ( IsSafeZone() || pchar_client->IsSafeZone() ) return false;
	
						if ( !GLCONST_CHAR::bSCHOOL_FREE_PK_Z_FORCED ) 
						{
							/*pvp capture the flag, Juver, 2018/01/31 */
							if ( GLGaeaClient::GetInstance().IsSchoolFreePk () && m_wSchool!=pchar_client->GetCharData().wSchool && !bCaptureTheFlag )
								return true;
						}

						{
							/*pvp capture the flag, Juver, 2018/01/31 */
							bool bBRIGHTEVENT = GLGaeaClient::GetInstance().IsBRIGHTEVENT();
							if( bBRIGHTEVENT && !bCaptureTheFlag )	return true;
						}

						PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();

						if ( pLand && pLand->IsClubBattleZone() )
						{
							if ( m_sCLUB.IsBattle( pchar_client->GETCLUBID() ) )				return true;
							if ( m_sCLUB.IsBattleAlliance( pchar_client->GETALLIANCEID() ) )	return true;
						}


						//	��ý� ���� ����.
						STARGETID starget_id_owner;
						starget_id_owner.dwID = pchar_client->GetCtrlID();
						starget_id_owner.emCrow = pchar_client->GetCrow();

						breaction = ISCONFRONT_TAR ( starget_id_owner );

						//	pk �� ��� ���� ����.
						if ( !breaction )	breaction = IS_PLAYHOSTILE ( pchar_client->GetCharID() );

						if ( !breaction )
						{
							// �������� ����Ŭ�� �ɹ��� ���� �Ұ�.
							if ( pLand )
							{
								if ( pLand->m_bClubBattle )
								{
									BOOL bAlliance = m_sCLUB.IsAllianceGuild ( pchar_client->GETCLUBID() );

									if ( bAlliance )
										breaction = false;
									else
										breaction = true;
								}
								else if ( pLand->m_bClubDeathMatch )
								{
									if ( m_sCLUB.m_dwID == pchar_client->GETCLUBID() )	breaction = false;
									else breaction = true;
								}
								/*pvp tyranny, Juver, 2017/08/24 */
								else if ( pLand->m_bPVPTyrannyMap )
								{
									if ( m_wSchool == pchar_client->GetCharData().wSchool )	breaction = false;
									else breaction = true;
								}
								/*school wars, Juver, 2018/01/19 */
								else if ( pLand->m_bPVPSchoolWarsMap )
								{
									if ( m_wSchool == pchar_client->GetCharData().wSchool )	breaction = false;
									else breaction = true;
								}
								/*pvp capture the flag, Juver, 2018/01/31 */
								else if ( pLand->m_bPVPCaptureTheFlagMap )
								{
									if ( m_wCaptureTheFlagTeam == pchar_client->GetCharData().m_wCaptureTheFlagTeam )	
										breaction = false;
									else 
										breaction = true;
								}
							}
						}
					}
				}
			}break;

		case CROW_NPC:
			breaction = false;
			break;

		case CROW_MATERIAL:
			breaction = false;
			break;

		//case CROW_PET:	break;	// PetData

		default:
			breaction = false;
			break;
		};
	}
	else
	{
		switch ( emCROW )
		{
		case CROW_PC:
			{
				//	pc ���� ����� ���� ���� ���� ����ڰ� �ƴ� ���� ����. ( �ϴ� Ŭ���̾�Ʈ�� �˻� ����. )
				breaction = !IS_PLAYHOSTILE ( pTARGET->GetCharID() );

				PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();
				if ( pLand && ( pLand->m_bClubBattle || pLand->m_bClubDeathMatch || pLand->IsFreePK() ) )
				{
					PGLCHARCLIENT pCLIENT = GLGaeaClient::GetInstance().GetChar(sTargetID.dwID);
					breaction = (pCLIENT->GETCLUBID()==m_dwGuild);
				}

				/*pvp tyranny, Juver, 2017/08/24 */
				if ( pLand && pLand->m_bPVPTyrannyMap )
				{
					PGLCHARCLIENT pCLIENT = GLGaeaClient::GetInstance().GetChar( sTargetID.dwID );
					breaction = ( pCLIENT && m_wSchool == pCLIENT->GetCharData().wSchool );
				}

				/*school wars, Juver, 2018/01/19 */
				if ( pLand && pLand->m_bPVPSchoolWarsMap )
				{
					PGLCHARCLIENT pCLIENT = GLGaeaClient::GetInstance().GetChar( sTargetID.dwID );
					breaction = ( pCLIENT && m_wSchool == pCLIENT->GetCharData().wSchool );
				}

				/*pvp capture the flag, Juver, 2018/01/31 */
				if ( pLand && pLand->m_bPVPCaptureTheFlagMap )
				{
					PGLCHARCLIENT pCLIENT = GLGaeaClient::GetInstance().GetChar( sTargetID.dwID );
					breaction = ( pCLIENT && m_wCaptureTheFlagTeam == pCLIENT->GetCharData().m_wCaptureTheFlagTeam );
				}
			}
			break;
		
		case CROW_MOB:
			breaction = false;
			break;

		case CROW_NPC:
			breaction = m_sCONFTING.IsCONFRONTING() ? false : true;
			break;

		case CROW_MATERIAL:
			breaction = true;
			break;

		//case CROW_PET:	break;	// PetData

		default:
			breaction = false;
			break;
		};
	}

	return breaction;
}

BOOL GLCharacter::ISCONFRONT_TAR ( const STARGETID &sTargetID )
{
	if ( sTargetID.emCrow!=CROW_PC )	return FALSE;

	if ( m_sCONFTING.IsFIGHTING() )
	{
		switch ( m_sCONFTING.emTYPE )
		{
		case EMCONFT_ONE:
			return ( sTargetID.dwID==m_sCONFTING.dwTAR_ID );
			break;

		case EMCONFT_PARTY:
			{
				SCONFTING_CLT::CONFT_MEM_ITER iter = m_sCONFTING.setConftMember.find(sTargetID.dwID);
				return ( iter!= m_sCONFTING.setConftMember.end() );
			}
			break;

		case EMCONFT_GUILD:
			{
				PGLCHARCLIENT pCHAR = GLGaeaClient::GetInstance().GetChar ( sTargetID.dwID );
				if ( !pCHAR )						return FALSE;

				SCONFTING_CLT::CONFT_MEM_ITER iter = m_sCONFTING.setConftMember.find(pCHAR->GetCharData().dwCharID);
				return ( iter!= m_sCONFTING.setConftMember.end() );
			}
			break;
		};
	}

	return FALSE;
}

void GLCharacter::DELCONFRONT_MEMBER ( DWORD dwID )
{
	switch ( m_sCONFTING.emTYPE )
	{
	case EMCONFT_ONE:
		break;

	case EMCONFT_PARTY:
		{
			//	Note : �ڱ� ��Ƽ���� ���.
			GLPartyClient::GetInstance().ResetConfrontMember ( dwID );

			//	Note : ���� ��Ƽ���� ���.
			m_sCONFTING.DEL_CONFT_MEMBER ( dwID );
		}
		break;

	case EMCONFT_GUILD:
		{
			//	Note : �ڱ� Ŭ������ ���.
			m_sCLUB.DELCONFT_MEMBER ( dwID );

			//	Note : ���� Ŭ������ ���.
			m_sCONFTING.DEL_CONFT_MEMBER ( dwID );
		}
		break;
	};

	return;
}

void GLCharacter::NewConftBoundEffect ( const EMCONFT_TYPE emCONFT )
{
	//	Note : ����Ʈ ����.
	//
	DxEffGroupPlayer::GetInstance().DeleteAllEff ( GLCONST_CHAR::strCONFRONT_BOUND.c_str() );

	D3DXMATRIX matTrans, matRotY;
	D3DXVECTOR3 vDist;
	D3DXMatrixIdentity ( &matTrans );

	for ( float fRotY=0.0f; fRotY<(D3DX_PI*2.0f); fRotY += (D3DX_PI*2.0f/24.0f) )
	{
		vDist = D3DXVECTOR3(1,0,0);
		D3DXMatrixRotationY(&matRotY,fRotY);
		D3DXVec3TransformCoord(&vDist,&vDist,&matRotY);

		float fCONFT_DIST(GLCONST_CHAR::fCONFRONT_ONE_DIST);
		switch ( emCONFT )
		{
		case EMCONFT_ONE:
			fCONFT_DIST = GLCONST_CHAR::fCONFRONT_ONE_DIST;
			break;
		case EMCONFT_PARTY:
			fCONFT_DIST = GLCONST_CHAR::fCONFRONT_PY_DIST;
			break;

		case EMCONFT_GUILD:
			fCONFT_DIST = GLCONST_CHAR::fCONFRONT_CLB_DIST;
			break;
		};

		vDist = m_sCONFTING.vPosition + vDist * fCONFT_DIST;

		BOOL bCollision(FALSE);
		D3DXVECTOR3 vCollision;
		DWORD dwCollisionID;

		NavigationMesh* pNaviMesh = GLGaeaClient::GetInstance().GetActiveMap()->GetNaviMesh();
		pNaviMesh->IsCollision(vDist+D3DXVECTOR3(0,10000.0f,0),vDist+D3DXVECTOR3(0,-10000.0f,0),vCollision,dwCollisionID,bCollision);

		matTrans._41 = vDist.x;
		matTrans._42 = vCollision.y;
		matTrans._43 = vDist.z;

		DxEffGroupPlayer::GetInstance().NewEffGroup ( GLCONST_CHAR::strCONFRONT_BOUND.c_str(), matTrans );
	}
}

void GLCharacter::DelConftBoundEffect ()
{
	DxEffGroupPlayer::GetInstance().DeleteAllEff ( GLCONST_CHAR::strCONFRONT_BOUND.c_str() );
}

BOOL GLCharacter::IsVaildTradeInvenSpace ()
{
	GLInventory &sTradeBoxTar = GLTradeClient::GetInstance().GetTarTradeBox();

	GLInventory sInvenTemp;
	sInvenTemp.SetAddLine ( m_cInventory.GETAddLine(), true );
	sInvenTemp.Assign ( m_cInventory );

	//	Note : �ڽ��� ��ȯ ��Ͽ� �ö� �������� �ι꿡�� ����.
	//
	GLInventory &sTradeBoxMy = GLTradeClient::GetInstance().GetMyTradeBox();
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

//----------------------------------------------------------------------------------------------------------------------------------
//								H	a	i	r				S	t	y	l	e	 &&	 C	o	l	o	r
//----------------------------------------------------------------------------------------------------------------------------------
void GLCharacter::HairStyleInitData()
{
	m_bEnableHairSytle = TRUE;
	
	m_wHairStyleTEMP = m_wHair;

	// �÷� ��ȭ ���� ��� �ǵ��� �ʴ´�.
	if( !m_bEnableHairColor )
	{
		m_wHairColorTEMP = m_wHairColor;
	}
}

void GLCharacter::HairColorInitData()
{
	m_bEnableHairColor = TRUE;

	m_wHairColorTEMP = m_wHairColor;

	// ��Ÿ�� ��ȭ ���� ��� �ǵ��� �ʴ´�.
	if( !m_bEnableHairSytle )
	{
		m_wHairStyleTEMP = m_wHair;
	}
}

void GLCharacter::HairStyleChange( WORD wStyle )
{
	m_wHairStyleTEMP = wStyle; 

	HairStyleUpdate();
}

void GLCharacter::HairColorChange( WORD wColor )
{ 
	m_wHairColorTEMP = wColor; 

	HairColorUpdate();
}

void GLCharacter::HairStyleUpdate()
{
	EMCHARINDEX emIndex = CharClassToIndex(m_emClass);
	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().FindData ( GLCONST_CHAR::szCharSkin[emIndex] );
	if ( !pSkinChar )	return;

	const GLCONST_CHARCLASS &sCONST = GLCONST_CHAR::cCONSTCLASS[emIndex];

	// ��Ÿ��.
	if ( sCONST.dwHAIRNUM > m_wHairStyleTEMP )
	{
		std::string strHAIR_CPS = sCONST.strHAIR_CPS[m_wHairStyleTEMP];
	
		PDXCHARPART pCharPart = NULL;
		pCharPart = m_pSkinChar->GetPiece(PIECE_HAIR);		//	���� ���� ��Ų.

		if( pCharPart && strcmp(strHAIR_CPS.c_str(),pCharPart->m_szFileName) )
		{
			m_pSkinChar->SetPiece( strHAIR_CPS.c_str(), m_pd3dDevice, NULL, NULL, TRUE );
		}
	}

	// Note : �̰��� ȣ���ϸ鼭.. �Ӹ����� �����·� �ٲ�� ������.
	//		�׷��� FALSE ȣ��� �Ӹ������ �ȹٲ�� �Ѵ�.
	UpdateSuit( FALSE, TRUE );
}

void GLCharacter::HairColorUpdate()
{
	EMCHARINDEX emIndex = CharClassToIndex(m_emClass);
	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().FindData ( GLCONST_CHAR::szCharSkin[emIndex] );
	if ( !pSkinChar )	return;

	m_pSkinChar->SetHairColor( m_wHairColorTEMP );
}

void GLCharacter::FaceStyleInitData()
{
	m_bEnableFaceStyle = TRUE;
	
	m_wFaceStyleTEMP = m_wFace;
}

void GLCharacter::FaceStyleChange( WORD wStyle )
{
	m_wFaceStyleTEMP = wStyle; 

	FaceStyleUpdate();
}

void GLCharacter::FaceStyleUpdate()
{
	EMCHARINDEX emIndex = CharClassToIndex(m_emClass);
	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().FindData ( GLCONST_CHAR::szCharSkin[emIndex] );
	if ( !pSkinChar )	return;

	const GLCONST_CHARCLASS &sCONST = GLCONST_CHAR::cCONSTCLASS[emIndex];

	// ��
	if ( sCONST.dwHEADNUM > m_wFaceStyleTEMP )
	{
		std::string strHEAD_CPS = sCONST.strHEAD_CPS[m_wFaceStyleTEMP];
	
		PDXCHARPART pCharPart = NULL;
		pCharPart = m_pSkinChar->GetPiece(PIECE_HEAD);		//	���� ���� ��Ų.

		if ( pCharPart && strcmp(strHEAD_CPS.c_str(),pCharPart->m_szFileName) )
		{
			m_pSkinChar->SetPiece ( strHEAD_CPS.c_str(), m_pd3dDevice, 0x0, 0, TRUE );
		}
	}

	// Note : �̰��� ȣ���ϸ鼭.. �Ӹ����� �����·� �ٲ�� ������.
	//		�׷��� FALSE ȣ��� �Ӹ������ �ȹٲ�� �Ѵ�.
	UpdateSuit( TRUE, FALSE );
}

BOOL GLCharacter::IsMarketClick( DWORD dwGaeaID )
{
	for( int i = 0; i < ( int ) m_vecMarketClick.size(); i++ )
	{
		if( m_vecMarketClick[ i ] == dwGaeaID )
		{
			return TRUE;
		}
	}
	return FALSE;
}

void GLCharacter::InsertMarketClick( DWORD dwGaeaID )
{
	m_vecMarketClick.push_back( dwGaeaID );
}

void GLCharacter::InitAllSkillFact ()
{
	for ( int i=0; i<EMBLOW_MULTI; ++i )		DISABLEBLOW ( i );
	for ( int i=0; i<SKILLFACT_SIZE; ++i )		DISABLESKEFF ( i );

	m_dwTransfromSkill = SNATIVEID::ID_NULL;

	FACTEFF::DeleteEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT, m_sSTATEBLOWS );
}

D3DXVECTOR3 GLCharacter::GetPosBodyHeight ()
{ 
	float fHeight = m_fHeight * m_fScaleRange;

	/*change scale card, Juver, 2018/01/03 */
	if ( m_bChangeCharScale )
		fHeight = m_fHeight * m_fActiveCharScale;

	/*vehicle system, Juver, 2017/08/09 */
	if ( m_bVehicle )
	{
		int emType = m_sVehicle.m_emTYPE;
		if ( emType == VEHICLE_TYPE_BOARD )	fHeight += 10.0f;
	}

	return D3DXVECTOR3( m_vPos.x, m_vPos.y+fHeight, m_vPos.z ); 
}

void GLCharacter::GetSkinAABB( D3DXVECTOR3& vMin, D3DXVECTOR3& vMax )
{ 
	if( m_pSkinChar )
		m_pSkinChar->GetAABBBox( vMax, vMin );
}

float GLCharacter::GetSkinScale()
{ 
	if( !m_pSkinChar )	return 1.0f;
	return m_pSkinChar->GetScale();
}

float GLCharacter::GetSkinHeight()
{ 
	if( !m_pSkinChar )	return 20.0f;
	return m_pSkinChar->m_fHeight;
}

LONGLONG GLCharacter::GetCalcTaxiCharge( int nSelectMap, int nSelectStop )
{
	GLTaxiStation& sTaxiStation = GLTaxiStation::GetInstance();

	STAXI_MAP* pTaxiMap = sTaxiStation.GetTaxiMap( nSelectMap );
	if ( !pTaxiMap )
	{
		GASSERT ( 0 && "pTaxiMap�� ã���� �����ϴ�." );
		return 0;
	}

	STAXI_STATION* pSTATION = pTaxiMap->GetStation( nSelectStop );
	if ( !pSTATION )
	{
		GASSERT ( 0 && "STATION�� ã�� �� �����ϴ�." );
		return 0;
	}

	DWORD dwCurMapID = GLGaeaClient::GetInstance().GetActiveMapID().dwID;
	LONGLONG dwCharge = sTaxiStation.GetBasicCharge();

	if ( pSTATION->dwMAPID != dwCurMapID ) dwCharge += pSTATION->dwMapCharge;

	PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();
	if ( pLand )
	{
		volatile float fSHOP_RATE = GetBuyRate();
		volatile float fSHOP_RATE_C = fSHOP_RATE * 0.01f;
		dwCharge = LONGLONG ( (float)dwCharge * fSHOP_RATE_C );
	}

	return dwCharge;
}

bool GLCharacter::IsCoolTime( SNATIVEID sNativeID )
{
	__time64_t tCurTime = GLGaeaClient::GetInstance().GetCurrentTime().GetTime();
	__time64_t tCoolTime = GetMaxCoolTime( sNativeID );

	if ( tCurTime < tCoolTime ) return true;

	return false;
}

bool GLCharacter::CheckCoolTime( SNATIVEID sNativeID )
{

	if ( IsCoolTime ( sNativeID ) )
	{
		SITEM* pItem = GLItemMan::GetInstance().GetItem ( sNativeID );
		if ( !pItem )	return true;

		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("ITEM_COOLTIME"), pItem->GetName() );
		return true;
	}

	return false;
}

__time64_t	GLCharacter::GetMaxCoolTime ( SNATIVEID sNativeID )
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

ITEM_COOLTIME*	GLCharacter::GetCoolTime ( SNATIVEID sNativeID )
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

	return (tCoolType > tCoolID) ? pCoolTimeType : pCoolTimeID;
}



ITEM_COOLTIME*	GLCharacter::GetCoolTime( DWORD dwCoolID, EMCOOL_TYPE emCoolType )
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

bool GLCharacter::SetCoolTime ( ITEM_COOLTIME& sCoolTime, EMCOOL_TYPE emCoolType )
{
	if ( emCoolType == EMCOOL_ITEMID )
	{
		m_mapCoolTimeID[sCoolTime.dwCoolID] = sCoolTime;
	}
	else if( emCoolType == EMCOOL_ITEMTYPE )
	{
		m_mapCoolTimeType[sCoolTime.dwCoolID] = sCoolTime;
	}

	return true;
}

const SITEMCUSTOM GLCharacter::GET_ITEM_MIX( int nIndex )
{
	SITEMCUSTOM sItemCustom;
	sItemCustom.sNativeID = NATIVEID_NULL();

	if( !m_sItemMixPos[nIndex].VALID() )
		return sItemCustom;

	SINVENITEM* pResistItem = m_cInventory.GetItem( m_sItemMixPos[nIndex].wPosX, m_sItemMixPos[nIndex].wPosY );
	if( !pResistItem )
		return sItemCustom;

	sItemCustom = pResistItem->sItemCustom;

	return sItemCustom;
}

void GLCharacter::SetItemMixMoveItem( int nIndex )
{
	if ( nIndex >= ITEMMIX_ITEMNUM )
	{
		m_sPreInventoryItem.RESET();
		return;
	}

	if( m_sPreInventoryItem.VALID() )
	{
		for ( int i = 0; i < ITEMMIX_ITEMNUM; ++i )
		{
			if ( m_sItemMixPos[i].wPosX == m_sPreInventoryItem.wPosX &&
				m_sItemMixPos[i].wPosY == m_sPreInventoryItem.wPosY )
			{
				m_sPreInventoryItem.RESET();
				return;
			}

		}

		m_sItemMixPos[nIndex].SET( m_sPreInventoryItem.wPosX, m_sPreInventoryItem.wPosY );
		
		m_sPreInventoryItem.RESET();
	}
	
	return;
}

void GLCharacter::ReSetItemMixItem( int nIndex )
{
	if ( nIndex >= ITEMMIX_ITEMNUM || m_sPreInventoryItem.VALID() )
	{
		m_sPreInventoryItem.RESET();
		return;
	}

	m_sItemMixPos[nIndex].RESET();
}

void GLCharacter::ResetItemMix()
{
	for ( int i = 0; i < ITEMMIX_ITEMNUM; ++i ) 
	{
		m_sItemMixPos[i].RESET();
	}
	m_sPreInventoryItem.RESET();
}

bool GLCharacter::ValidItemMixOpen()
{
	if ( CInnerInterface::GetInstance().IsVisibleGroup( ITEM_MIX_INVEN_WINDOW ) 
		|| CInnerInterface::GetInstance().IsVisibleGroup( ITEM_MIX_WINDOW ) )
		return true;
	return false;
}

DWORD GLCharacter::GetItemMixMoney()
{
	ITEM_MIX sItemMix;

	for( int i = 0; i < ITEMMIX_ITEMNUM; ++i ) 
	{
		sItemMix.sMeterialItem[i].sNID = GET_ITEM_MIX( i ).sNativeID;

		if( sItemMix.sMeterialItem[i].sNID != NATIVEID_NULL() )
			sItemMix.sMeterialItem[i].nNum = GET_ITEM_MIX( i ).wTurnNum;
	}

	GLItemMixMan::GetInstance().SortMeterialItem( sItemMix );

	const ITEM_MIX* pItemMix = GLItemMixMan::GetInstance().GetItemMix( sItemMix );
	if ( !pItemMix ) return UINT_MAX;

	return pItemMix->dwPrice;
}

bool GLCharacter::ValidWindowOpen()
{
	if ( ValidRebuildOpen() )					return true;	// ITEMREBUILD_MARK
	if ( ValidGarbageOpen() )					return true;	// ������
	if ( ValidItemMixOpen() )					return true;	// ������ ����
	if ( IsOpenMiniGame() )						return true;	// �̴ϰ���
	if ( GLTradeClient::GetInstance().Valid() )	return true;
	if ( ValidItemCompoundOpen() )				return true;	/*product item, Juver, 2017/10/17 */
	if ( ValidItemTransferOpen() )				return true;	/*item transfer card, Juver, 2018/01/18 */

	return false;
}

void GLCharacter::UpdateTargetEffect()
{
	bool bCancel = true;

	if ( RANPARAM::bTargetEffect )
	{
		if ( m_sACTARTarget.sTARID != STARGETID() )
		{
			GLCOPY* pActorTarget = GLGaeaClient::GetInstance().GetCopyActor ( m_sACTARTarget.sTARID );
			if ( pActorTarget )
			{
				STARGETID sTargetID ( m_sACTARTarget.sTARID.emCrow, m_sACTARTarget.sTARID.dwID, m_sACTARTarget.sTARID.vPos );
				D3DXMATRIX  matTrans, matScale, matWorld;
				D3DXVECTOR3 vMin, vMax;
				pActorTarget->GetSkinAABB( vMin, vMax );
				D3DXMatrixTranslation ( &matTrans, m_sACTARTarget.sTARID.vPos.x, m_sACTARTarget.sTARID.vPos.y, m_sACTARTarget.sTARID.vPos.z );
				D3DXMatrixScaling( &matScale, vMax.x/4.f, vMax.y/4.f, vMax.z/4.f );
				D3DXMatrixMultiply( &matWorld, &matScale, &matTrans );
				DxEffExtra::GetInstance().NewTargetEff ( GLCONST_CHAR::strTARGET_EFFECT.c_str(), matWorld, &sTargetID );

				D3DXVECTOR3	vTarPos = GLGaeaClient::GetInstance().GetTargetPos( m_sACTARTarget.sTARID );
				if ( vTarPos != VERROR() )
					m_sACTARTarget.sTARID.vPos = vTarPos;

				bCancel = false;
			}
		}
	}

	if ( bCancel )
	{
		DxEffExtra::GetInstance().RemoveTargetEffect( GLCONST_CHAR::strTARGET_EFFECT.c_str() );
		m_sACTARTarget = SACTIONTAR();
	}
}

BOOL GLCharacter::IsNpcQuest( DWORD dwGlobID )
{
	PGLCROWCLIENT pCrow = GLGaeaClient::GetInstance().GetActiveMap()->GetCrow ( dwGlobID );
	if ( !pCrow )		return FALSE;
	if ( !pCrow->IsValidBody() )		return FALSE;
	if ( !pCrow->IsHaveVisibleBody() )	return FALSE;
	if ( pCrow->m_pCrowData->m_emCrow != CROW_NPC ) return FALSE;

	SNATIVEID sIDCROW = pCrow->m_pCrowData->sNativeID;

	BOOL bFIND = FALSE;
	GLQuestPlay::MAPQUEST QuestProcMap = m_cQuestPlay.GetQuestProc();
	GLQuestPlay::MAPQUEST_ITER iterQuestProg = QuestProcMap.begin();
	for ( ; iterQuestProg != QuestProcMap.end (); ++iterQuestProg )
	{
		GLQUESTPROG* pQuestProg = (*iterQuestProg).second;
		if ( !pQuestProg ) continue;
		DWORD dwQuestID = pQuestProg->m_sNID.dwID;
		GLQUEST* pQuest = GLQuestMan::GetInstance().Find ( dwQuestID );
		if ( !pQuest ) continue;
		DWORD dwCurrentStep = pQuestProg->m_dwSTEP;
		GLQUEST_STEP* pQuestStep = pQuest->GetSTEP ( dwCurrentStep );
		if ( !pQuestStep )	continue;
		if ( !pQuestStep->IsNEED_NPCTALK() ) continue;
		if ( pQuestStep->m_dwNID_NPCTALK == sIDCROW.dwID )
		{
			//verify if able to talk to this npc
			//verify if quest id and quest step exist on this npc
			CNpcDialogueSet		m_DialogueSet;
			//client dont have dialogset data in m_pCrowdata so we need to load it
			BOOL bLoad = m_DialogueSet.Load ( pCrow->m_pCrowData->GetTalkFile() );
			if ( !bLoad )	continue;
			CNpcDialogueSet::DIALOGMAP* pDialogueMap = m_DialogueSet.GetDialogueMap();
			CNpcDialogueSet::DIALOGMAP_IT iterDialogMap = pDialogueMap->begin();
			for( ; iterDialogMap != pDialogueMap->end(); ++iterDialogMap )
			{
				CNpcDialogue* pNpcDialogue = (*iterDialogMap).second;
				if ( !pNpcDialogue )	continue;
				DWORD dwIndex = pNpcDialogue->FindConditionSatisfied (this);
				if ( dwIndex==UINT_MAX )	continue;
				CNpcDialogueCase* pCase = pNpcDialogue->GetDlgCase(dwIndex);	
				if ( !pCase )	continue;
				CNpcTalkControl* pNpcTalkControl = pCase->GetTalkControl ();
				if ( !pNpcTalkControl )	continue;
	
				CNpcTalkControl::NPCMAP* pMap = pNpcTalkControl->GetTalkMap ();
				CNpcTalkControl::NPCMAP_IT iterTalkControl = pMap->begin();
				for ( ; iterTalkControl != pMap->end(); ++iterTalkControl )
				{
					SNpcTalk* pNpcTalk = (*iterTalkControl).second;
					if ( !pNpcTalk )	continue;
					if ( !pNpcTalk->DoTEST(this) )	continue;
					if ( pNpcTalk->m_nACTION != SNpcTalk::EM_QUEST_STEP )	continue;
					if ( pNpcTalk->m_dwACTION_PARAM1 != dwQuestID )	continue;
					if ( pNpcTalk->m_dwACTION_PARAM2 != dwCurrentStep )	continue;

					bFIND = true;	
					break;		
				}		

				if ( bFIND )	
					break;
			}
		}

		if ( bFIND )	
			break;
	}

	return bFIND;
}

void GLCharacter::UpdateNPCQuestEffect()
{
	GLCROWCLIENTLIST *pCrowList = GLGaeaClient::GetInstance().GetActiveMap()->GetCrowList ();
	if ( !pCrowList )	return;
	GLCROWCLIENTNODE *pCrowCur = pCrowList->m_pHead;
	for ( ; pCrowCur; pCrowCur = pCrowCur->pNext )
	{
		PGLCROWCLIENT pCrow = pCrowCur->Data;
		if ( pCrow )
		{
			if ( !pCrow )		continue;
			if ( !pCrow->IsValidBody() )		continue;
			if ( !pCrow->IsHaveVisibleBody() )	continue;
			if ( pCrow->m_pCrowData->m_emCrow != CROW_NPC ) continue;

			BOOL bHaveQuest = IsNpcQuest( pCrow->m_dwGlobID );
			if ( bHaveQuest )
			{
				STARGETID sTARID ( CROW_NPC, pCrow->m_dwGlobID, pCrow->GetPosition() );
				BOOL bExisting = DxEffExtra::GetInstance().FindPassiveEffect( GLCONST_CHAR::strCOMPLETE_NPC_EFFECT.c_str(), sTARID );
				if ( !bExisting ) 
				{
					D3DXMATRIX  matTrans;
					D3DXMatrixTranslation ( &matTrans, pCrow->GetPosition().x, pCrow->GetPosition().y, pCrow->GetPosition().z );
					matTrans._42 += pCrow->GetSkinHeight();
					DxEffExtra::GetInstance().PassiveEffect ( GLCONST_CHAR::strCOMPLETE_NPC_EFFECT.c_str(), matTrans, sTARID );
				}
			}
			else
			{
				STARGETID sTARID ( CROW_NPC, pCrow->m_dwGlobID, pCrow->GetPosition() );
				DxEffExtra::GetInstance().DeletePassiveEffect ( GLCONST_CHAR::strCOMPLETE_NPC_EFFECT.c_str(),sTARID );
			}
		}
	}
}

/*itemfood system, Juver, 2017/05/25 */
BOOL GLCharacter::ItemFoodAdd ( SNATIVEID skill_id, WORD wLevel, WORD wSLOT )
{
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( skill_id.wMainID, skill_id.wSubID );
	if ( !pSkill )				return FALSE;
	if ( wLevel >= SKILL::MAX_LEVEL )	return FALSE;
	if ( wSLOT >= FITEMFACT_SIZE )	return FALSE;
	SKILL::CDATA_LVL &sSKILL_DATA = pSkill->m_sAPPLY.sDATA_LVL[wLevel];

	BOOL bHOLD = FALSE;
	SFITEMFACT sSKILLEF;
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
			sSKILLEF.sImpacts[nImpact].fADDON_VAR = pSkill->m_sAPPLY.sImpacts[nImpact].fADDON_VAR[wLevel];
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
		case EMSPECA_CHANGE_ATTACK_RANGE:	/*skill range spec logic, Juver, 2017/06/06 */
		case EMSPECA_CHANGE_APPLY_RANGE:	/*skill range spec logic, Juver, 2017/06/06 */
		case EMSPECA_TALK_TO_NPC:			/*npc talk buff, Juver, 2017/10/03 */
			bHOLD = TRUE;
			sSKILLEF.sSpecs[nSpec].emSPEC = pSkill->m_sAPPLY.sSpecs[nSpec].emSPEC;
			sSKILLEF.sSpecs[nSpec].fSPECVAR1 = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].fVAR1;
			sSKILLEF.sSpecs[nSpec].fSPECVAR2 = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].fVAR2;
			sSKILLEF.sSpecs[nSpec].dwSPECFLAG = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].dwFLAG;
			sSKILLEF.sSpecs[nSpec].dwNativeID = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].dwNativeID;
			break;
		};
	}

	if ( bHOLD )
	{
		sSKILLEF.sNATIVEID	= skill_id;
		sSKILLEF.wLEVEL		= wLevel;
		sSKILLEF.fAGE		= sSKILL_DATA.fLIFE;

		m_sFITEMFACT[wSLOT] = sSKILLEF;

		return TRUE;
	}

	GASSERT ( 0 && "���� ��ų������ ��ȿ���� ����." );
	return FALSE;
}

/*vehicle booster system, Juver, 2017/08/11 */
void GLCharacter::VehicleBoosterStart()
{
	if ( GLTradeClient::GetInstance().Valid() ) return;

	if ( !m_bVehicle )				return;
	if ( !m_sVehicle.m_bBooster )	return;
	if ( m_bBoosterStart )	return;

	if ( m_bBoosterCharge )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("BIKE_BOOSTER_NOT_READY") );
		return;
	}

	GLMSG::SNETPC_REQ_VEHICLE_BOOSTER netmsgfield;
	netmsgfield.bStart = true;
	NETSENDTOFIELD ( &netmsgfield );
}

/*vehicle booster system, Juver, 2017/08/11 */
void GLCharacter::VehicleBoosterStateOn()
{
	if ( IsSTATE( EM_ACT_VEHICLE_BOOSTER ))	return;

	SetSTATE(EM_ACT_VEHICLE_BOOSTER);

	GLMSG::SNETPC_ACTSTATE NetMsg;
	NetMsg.dwActState = m_dwActState;
	NETSEND ( (NET_MSG_GENERIC*) &NetMsg );
}

/*vehicle booster system, Juver, 2017/08/12 */
void GLCharacter::VehicleBoosterStateOff()
{
	if ( !IsSTATE( EM_ACT_VEHICLE_BOOSTER ))	return;

	ReSetSTATE(EM_ACT_VEHICLE_BOOSTER);

	GLMSG::SNETPC_ACTSTATE NetMsg;
	NetMsg.dwActState = m_dwActState;
	NETSEND ( (NET_MSG_GENERIC*) &NetMsg );
}

/*system buffs, Juver, 2017/09/04 */
BOOL GLCharacter::SystemBuffAdd ( SNATIVEID skill_id, WORD wLevel, WORD wSLOT )
{
	if ( wSLOT >= SYSTEM_BUFF_SIZE )		return FALSE;
	m_sSYSTEM_BUFF[wSLOT].RESET();

	if ( skill_id == NATIVEID_NULL() )	return FALSE;

	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( skill_id.wMainID, skill_id.wSubID );
	if ( !pSkill )						return FALSE;
	if ( wLevel >= SKILL::MAX_LEVEL )	return FALSE;

	SKILL::CDATA_LVL &sSKILL_DATA = pSkill->m_sAPPLY.sDATA_LVL[wLevel];

	BOOL bHOLD = FALSE;
	SSYSTEM_BUFF ssystem_buff;
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
		ssystem_buff.emTYPE = pSkill->m_sAPPLY.emBASIC_TYPE;
		ssystem_buff.fMVAR = sSKILL_DATA.fBASIC_VAR;
		break;
	};

	for( int nImpact=0; nImpact<SKILL::MAX_IMPACT; ++nImpact )
	{
		if ( pSkill->m_sAPPLY.sImpacts[nImpact].emADDON != EMIMPACTA_NONE )
		{
			bHOLD = TRUE;
			ssystem_buff.sImpacts[nImpact].emADDON = pSkill->m_sAPPLY.sImpacts[nImpact].emADDON;
			ssystem_buff.sImpacts[nImpact].fADDON_VAR = pSkill->m_sAPPLY.sImpacts[nImpact].fADDON_VAR[wLevel];
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
		case EMSPECA_CHANGE_ATTACK_RANGE:	/*skill range spec logic, Juver, 2017/06/06 */
		case EMSPECA_CHANGE_APPLY_RANGE:	/*skill range spec logic, Juver, 2017/06/06 */
		case EMSPECA_TALK_TO_NPC:			/*npc talk buff, Juver, 2017/10/03 */
			bHOLD = TRUE;
			ssystem_buff.sSpecs[nSpec].emSPEC = pSkill->m_sAPPLY.sSpecs[nSpec].emSPEC;
			ssystem_buff.sSpecs[nSpec].fSPECVAR1 = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].fVAR1;
			ssystem_buff.sSpecs[nSpec].fSPECVAR2 = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].fVAR2;
			ssystem_buff.sSpecs[nSpec].dwSPECFLAG = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].dwFLAG;
			ssystem_buff.sSpecs[nSpec].dwNativeID = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].dwNativeID;
			break;
		};
	}

	if ( bHOLD )
	{
		ssystem_buff.sNATIVEID	= skill_id;
		ssystem_buff.wLEVEL		= wLevel;

		m_sSYSTEM_BUFF[wSLOT] = ssystem_buff;

		return TRUE;
	}

	return FALSE;
}

/*bike color , Juver, 2017/11/16 */
void GLCharacter::BikeColorUpdate()
{
	if ( m_bEnableBikeColor && m_pSkinChar && m_pSkinChar->m_pVehicle && m_sVehicle.m_emTYPE == VEHICLE_TYPE_BIKE )
	{
		for ( int ncolor=0; ncolor<BIKE_COLOR_SLOT_PART_SIZE; ++ncolor )
		{
			EMPIECECHAR piece_color = BIKE_COLOR_SLOT_TO_PIECE( (BIKE_COLOR_SLOT_PART) ncolor );
			BOOL bColor1 = IS_BIKECOLOR_1( (BIKE_COLOR_SLOT_PART) ncolor );
			if ( bColor1 )
				m_pSkinChar->m_pVehicle->SetColor1( piece_color, m_wBikeColor[ncolor] );
			else
				m_pSkinChar->m_pVehicle->SetColor2( piece_color, m_wBikeColor[ncolor] );
		}
	}
}

/*bike color , Juver, 2017/11/16 */
void GLCharacter::BikeColorEnd()
{ 
	m_bEnableBikeColor = FALSE;
	
	for( int i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
		m_wBikeColor[i] = 0;

	if ( m_pd3dDevice )
		UpdateSuit();
}

/*pvp capture the flag, Juver, 2018/02/08 */
void GLCharacter::CaptureTheFlagLocatorUpdate()
{
	if ( !m_bCaptureTheFlagLocatorOn || m_bCaptureTheFlagHoldFlag )
	{
		for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		{
			DxEffExtra::GetInstance().RemoveLocatorAEffect( GLCONST_CHAR::strCaptureTheFlagDirectionEffect[i].c_str() );
			DxEffExtra::GetInstance().RemoveLocatorBEffect( GLCONST_CHAR::strCaptureTheFlagDirectionEffect[i].c_str() );
		}

		return;
	}

	STARGETID starget_id ( CROW_PC, m_dwGaeaID, m_vPos );

	if ( m_wCaptureTheFlagTeam == CAPTURE_THE_FLAG_TEAM_A )
	{
		D3DXVECTOR3 vtargetpos = GLPVPCaptureTheFlagClient::GetInstance().m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_B].vPos;
		D3DXVECTOR3 vfindpos = GLGaeaClient::GetInstance().GetTargetPos( GLPVPCaptureTheFlagClient::GetInstance().m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_B] );
		if ( vfindpos != VERROR() )	vtargetpos = vfindpos;

		D3DXVECTOR3 vdirection =  m_vPos - vtargetpos;
		D3DXVec3Normalize ( &vdirection, &vdirection );
		float ftheta = DXGetThetaYFromDirection ( vdirection, m_vDirOrig );
		D3DXMATRIX  matworld, mattrans, matrotate;
		D3DXMatrixTranslation ( &mattrans, m_vPos.x, m_vPos.y, m_vPos.z );
		D3DXMatrixRotationY ( &matrotate, ftheta );
		matworld = matrotate * mattrans;
		DxEffExtra::GetInstance().NewLocatorAEff ( GLCONST_CHAR::strCaptureTheFlagDirectionEffect[CAPTURE_THE_FLAG_TEAM_B].c_str(), matworld, &starget_id );
		return;
	}

	if ( m_wCaptureTheFlagTeam == CAPTURE_THE_FLAG_TEAM_B )
	{
		D3DXVECTOR3 vtargetpos = GLPVPCaptureTheFlagClient::GetInstance().m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_A].vPos;
		D3DXVECTOR3 vfindpos = GLGaeaClient::GetInstance().GetTargetPos( GLPVPCaptureTheFlagClient::GetInstance().m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_A] );
		if ( vfindpos != VERROR() )	vtargetpos = vfindpos;

		D3DXVECTOR3 vdirection =  m_vPos - vtargetpos;
		D3DXVec3Normalize ( &vdirection, &vdirection );
		float ftheta = DXGetThetaYFromDirection ( vdirection, m_vDirOrig );
		D3DXMATRIX  matworld, mattrans, matrotate;
		D3DXMatrixTranslation ( &mattrans, m_vPos.x, m_vPos.y, m_vPos.z );
		D3DXMatrixRotationY ( &matrotate, ftheta );
		matworld = matrotate * mattrans;
		DxEffExtra::GetInstance().NewLocatorAEff ( GLCONST_CHAR::strCaptureTheFlagDirectionEffect[CAPTURE_THE_FLAG_TEAM_A].c_str(), matworld, &starget_id );
		return;
	}
}

void GLCharacter::SetHiddenWeaponPiece()
{
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( m_sActiveSkill.wMainID, m_sActiveSkill.wSubID );
	if( !pSkill )
		return;

	// ��ų�� ���Ǵ� ���⸦ �����Ѵ�;
	const bool bHiddenWeapon = pSkill->m_sLEARN.bHiddenWeapon;
	if( !bHiddenWeapon )
		return;

	SNATIVEID sHiddenWeapon = pSkill->m_sLEARN.sHiddenWeapon;
	SITEM* pItem = GLItemMan::GetInstance().GetItem ( sHiddenWeapon );
	if( pItem )
	{
		// [shhan][2014.12.18] rm #571 - �ذ��� ��ݽ�ų(�Ƿ�/�ذ��۷����) �� �޼� ���� �ܻ���.
		//						����� ���� ResetHiddenWeaponPiece ���� UpdateSuit �� �ߴ��� 
		//						��ձ��� ���� ��� �� �Ѽչ��� ��ų�� ����ϸ� �ٸ��Ѽտ� ������ ���̴� ���װ� �־���.
		//						�׷��� Ȱ��ȭ �� ��ġ�� ������ �ǵ��� �����۾���.
		/*EMPIECECHAR emType(PIECE_SHIFT_RWEAPON);
		if ( m_pSkinChar->SetPieceHiddenWeapon ( pItem->GetWearingFile(m_CHARINDEX),	m_pd3dDevice, 0, EMSCD_ALL_THREAD, emType, pItem->GetSelfBodyEffect() ) == S_OK )
		{
			if ( emType == PIECE_SHIFT_RWEAPON || emType == PIECE_SHIFT_LWEAPON )
			{
				m_pSkinChar->SetPartRend( emType, TRUE );
			}
		}
		if ( m_pSkinChar->SetPieceHiddenWeapon ( pItem->GetWearingFileEx(m_CHARINDEX),	m_pd3dDevice, 0, EMSCD_ALL_THREAD,  emType, pItem->GetSelfBodyEffect() ) == S_OK )
		{
			if ( emType == PIECE_SHIFT_RWEAPON || emType == PIECE_SHIFT_LWEAPON )
			{
				m_pSkinChar->SetPartRend( emType, TRUE );
			}
		}*/
		/*Item Self Effect, Juver, 2017/06/09 */
		std::string strSelfEffect = pItem->GetSelfBodyEffect();

		/*ABL system, Juver, 2017/06/02 */
		{
			std::string strFileName = pItem->GetWearingFileR( m_CHARINDEX );

			DxAttBoneLink* pBoneLink = DxAttBoneLinkContainer::GetInstance().LoadData( strFileName.c_str(), m_pd3dDevice, TRUE );
			if ( pBoneLink )	
			{
				m_pSkinChar->SetAttBone( pBoneLink, m_pd3dDevice, NULL, 0, TRUE, strSelfEffect.c_str() );
			}
			else
			{
				m_pSkinChar->SetPiece ( strFileName.c_str(), m_pd3dDevice, NULL, 0, TRUE, strSelfEffect.c_str() );
			}
			m_pSkinChar->SetPartRend ( PIECE_SHIFT_RWEAPON, TRUE );
		}

		{
			std::string strFileName = pItem->GetWearingFileL( m_CHARINDEX );

			DxAttBoneLink* pBoneLink = DxAttBoneLinkContainer::GetInstance().LoadData( strFileName.c_str(), m_pd3dDevice, TRUE );
			if ( pBoneLink )	
			{
				m_pSkinChar->SetAttBone( pBoneLink, m_pd3dDevice, NULL, 0, TRUE, strSelfEffect.c_str() );
			}
			else
			{
				m_pSkinChar->SetPiece ( strFileName.c_str(), m_pd3dDevice, NULL, 0, TRUE, strSelfEffect.c_str() );
			}
			m_pSkinChar->SetPartRend ( PIECE_SHIFT_LWEAPON, TRUE );
		}
	}
}

void GLCharacter::ResetHiddenWeaponPiece()
{
	PDXCHARPART pRHAND_HIDDEN = m_pSkinChar->GetPiece( PIECE_SHIFT_RWEAPON );
	PDXCHARPART pLHAND_HIDDEN = m_pSkinChar->GetPiece( PIECE_SHIFT_LWEAPON );
	if( pRHAND_HIDDEN || pLHAND_HIDDEN )
	{
		m_pSkinChar->SetPartRend ( PIECE_SHIFT_RWEAPON, FALSE );
		m_pSkinChar->SetPartRend ( PIECE_SHIFT_LWEAPON, FALSE );

		m_pSkinChar->ResetPiece( PIECE_SHIFT_RWEAPON );
		m_pSkinChar->ResetPiece( PIECE_SHIFT_LWEAPON );

		// [shhan][2014.11.20] �����ο��� UpdateSuit �� �ϸ� ���䰡 ���µǸ鼭 ��鸮�� ������ �־ ���ƺ�.
		//						�ϴ� �ּ��ص� �� �� �ѵ�, ������ �Ǹ� ���䵵 �����ϸ鼭 �����۾� �ʿ���.
		//UpdateSuit();
	}
} 

void GLCharacter::AutoArmSwap()
{
	if( true == IsReqSwapArms() )
		return;

	if( GLCC_EXTREME_M != m_emClass && GLCC_EXTREME_W != m_emClass )
		return;

	SNATIVEID sSkillID = GetskillRunSlot();
	if( sSkillID == NATIVEID_NULL() )
		return;

	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( sSkillID.wMainID, sSkillID.wSubID );
	if( NULL == pSkill )
		return;
	if( SKILL::EMROLE_PASSIVE == pSkill->m_sBASIC.emROLE )
		return;

	//	Note : ������ ���� ���� �˻�.
	EMSLOT emRHand, emRHand_S;
	if( TRUE == IsUseArmSub() )
	{
		emRHand = SLOT_RHAND_S;
		emRHand_S = SLOT_RHAND;
	}
	else
	{
		emRHand = SLOT_RHAND;
		emRHand_S = SLOT_RHAND_S;
	}

	const bool bHiddenWeapon = pSkill->m_sLEARN.bHiddenWeapon;
	GLITEM_ATT emITEM_RIGHT = ITEMATT_NOTHING;
	GLSKILL_ATT emSKILL_RIGHT = pSkill->m_sBASIC.emUSE_RITEM;

	if( SKILLATT_NOCARE != emSKILL_RIGHT )
	{
		SITEM* pItem = GET_SLOT_ITEMDATA( emRHand );

		// �������� ���ų� ��ų ���� ���� �������� �ƴҶ� 
		if( NULL == pItem || 
			false == CHECHSKILL_ITEM( emSKILL_RIGHT, pItem->sSuitOp.emAttack, bHiddenWeapon ) )
		{
			// Sub���⸦ üũ�ؼ� ������� �����Ѵ�.
			SITEM* pItem_S = GET_SLOT_ITEMDATA( emRHand_S );
			if( NULL != pItem_S && 
				true == CHECHSKILL_ITEM( emSKILL_RIGHT, pItem_S->sSuitOp.emAttack, bHiddenWeapon ) )
				ReqSlotChange();
		}
	}
}

//dmk14 | 11-1-16 | AI - auto pilot
SACTIONTAR GLCharacter::FindNearTarget ( D3DXVECTOR3 &vTargetPt, const EMFIND_TAR emFINDTAR, SNATIVEID sSKILLID )
{
	SACTIONTAR sACTAR;
	if ( IsSTATE(EM_ACT_CONTINUEMOVE) )		return sACTAR;

	PGLSKILL pRunSkill = GLSkillMan::GetInstance().GetData ( sSKILLID.wMainID , sSKILLID.wSubID );
	if( !pRunSkill )						return sACTAR;

	bool bONLY_ITEM = false;
	bool bODER_P2P = false;

	bool bANY(false), bENEMY(false), bOUR(false), bDIE(false);
	switch ( emFINDTAR )
	{
	case ENFIND_TAR_NULL:
		bANY = true;
		bENEMY = true;
		bOUR = true;
		break;
	case EMFIND_TAR_ENEMY:
		bENEMY = true;
		break;
	case EMFIND_TAR_ANY:
		bANY = true;
		break;
	case EMFIND_TAR_OUR:
		bOUR = true;
		break;

	case EMFIND_TAR_ANY_DIE:
		bANY = true;
		bDIE = true;
		break;
	case EMFIND_TAR_OUR_DIE:
		bOUR = true;
		bDIE = true;
		break;
	};

	STARGETID sTID_FOUND;
	DETECTMAP_RANGE pairRange;

	DETECTMAP *pDetectMap = NULL;

	PLANDMANCLIENT pLAND = GLGaeaClient::GetInstance().GetActiveMap();
	if( !pLAND )					return sACTAR;

	DWORD emCrow = CROW_EX_PC;
	if( m_bBotLockSpot )	
		pDetectMap = GLGaeaClient::GetInstance().DetectCrowBot ( emCrow, m_vBotPos, MAX_VIEWRANGE/3.0f );
	else
		pDetectMap = GLGaeaClient::GetInstance().DetectCrowBot ( emCrow, m_vPos, MAX_VIEWRANGE/3.0f );

	if ( bOUR )
	{
		if( sSKILLID == SNATIVEID(14,0) || sSKILLID == SNATIVEID(14,20) )
		{
			float fHpPercent =  ( float(m_sHP.dwNow) / float(m_sHP.dwMax) ) * 100;

			if( fHpPercent < 60.0f )
			{
				sACTAR.emACTAR = EMACTAR_PC_OUR;
				sACTAR.sTARID = STARGETID(CROW_PC,m_dwGaeaID,m_vPos);
						
				vTargetPt = GLGaeaClient::GetInstance().GetTargetPos( sACTAR.sTARID );

				return sACTAR;
			}

			if ( pDetectMap && m_bBotSupport )
			{
				pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_PC, STAR_ORDER() );
				if ( pairRange.first != pairRange.second )
				{
					STARGETID sTargetLowHP;
					float fHpPercent = 100.0f;

					pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_PC, STAR_ORDER() );
					DETECTMAP_ITER iter = pairRange.first;
					for ( ; (iter!=pairRange.second); ++iter )
					{
						const STARGETID &sFindID = (*iter);
						GLPARTY_CLIENT* pPARTY_CLT = GLPartyClient::GetInstance().FindMember( sFindID.dwID );
						PGLCHARCLIENT pCHAR = GLGaeaClient::GetInstance().GetChar ( sFindID.dwID );
						if( !pCHAR )				continue;
						if( pCHAR->IsDie() )		continue;
						if( !pPARTY_CLT )			continue;
						
						GLDWDATA sHP = pPARTY_CLT->m_sHP;
						float fPercent = ( float(sHP.dwNow) / float(sHP.dwMax) ) * 100.0f;
										 
						if( fHpPercent > fPercent )
						{
							sTargetLowHP = sFindID;
							fHpPercent = fPercent;
						}
					}
								
					if( fHpPercent != 100.0f )
					{
						sACTAR.emACTAR = EMACTAR_PC_OUR;
						sACTAR.sTARID = sTargetLowHP;

						vTargetPt = GLGaeaClient::GetInstance().GetTargetPos( sACTAR.sTARID );
						return sACTAR;
					}
				}
			}
		}
		else
		{
			sACTAR.emACTAR = EMACTAR_PC_OUR;
			sACTAR.sTARID = STARGETID(CROW_PC,m_dwGaeaID,m_vPos);
					
			vTargetPt = GLGaeaClient::GetInstance().GetTargetPos( sACTAR.sTARID );

			return sACTAR;
		}
	}
	else if( bANY )
	{
		if( sSKILLID == SNATIVEID(14,2) )
		{
			float fHpPercent =  ( float(m_sHP.dwNow) / float(m_sHP.dwMax) ) * 100;

			if( fHpPercent < 70.0f )
			{
				sACTAR.emACTAR = EMACTAR_PC_ANY;
				sACTAR.sTARID = STARGETID(CROW_PC,m_dwGaeaID,m_vPos);
						
				vTargetPt = GLGaeaClient::GetInstance().GetTargetPos( sACTAR.sTARID );

				return sACTAR;
			}
			else
			{
				if ( pDetectMap && m_bBotSupport )
				{
					pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_PC, STAR_ORDER() );
					if ( pairRange.first != pairRange.second )
					{
						STARGETID sTargetLowHP;
						float fHpPercent = 100.0f;

						pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_PC, STAR_ORDER() );
						DETECTMAP_ITER iter = pairRange.first;
						for ( ; (iter!=pairRange.second); ++iter )
						{
							const STARGETID &sFindID = (*iter);
							GLPARTY_CLIENT* pPARTY_CLT = GLPartyClient::GetInstance().FindMember( sFindID.dwID );
							PGLCHARCLIENT pCHAR = GLGaeaClient::GetInstance().GetChar ( sFindID.dwID );
							if( !pCHAR )				continue;
							if( pCHAR->IsDie() )		continue;
							if( !pPARTY_CLT )			continue;
						
							GLDWDATA sHP = pPARTY_CLT->m_sHP;
							float fPercent = ( float(sHP.dwNow) / float(sHP.dwMax) ) * 100.0f;
										 
							if( fHpPercent > fPercent )
							{
								sTargetLowHP = sFindID;
								fHpPercent = fPercent;
							}
						}
								
						if( fHpPercent != 100.0f )
						{
							sACTAR.emACTAR = EMACTAR_PC_ANY;
							sACTAR.sTARID = sTargetLowHP;

							vTargetPt = GLGaeaClient::GetInstance().GetTargetPos( sACTAR.sTARID );
							return sACTAR;
						}
					}
				}
			}
		}
		else
		{
			if ( pDetectMap )
			{
				//	PVP.
				pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_PC, STAR_ORDER() );
				if ( pairRange.first != pairRange.second )
				{
					if( bDIE )
					{
						pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_PC, STAR_ORDER() );
						DETECTMAP_ITER iter = pairRange.first;
						for ( ; (iter!=pairRange.second); ++iter )
						{
							const STARGETID &sFindID = (*iter);
							GLPARTY_CLIENT* pPARTY_CLT = GLPartyClient::GetInstance().FindMember( sFindID.dwID );
							if( !pPARTY_CLT )			continue;
							if( m_bResuPtLeader )
							{
								GLPARTY_CLIENT *pMaster = GLPartyClient::GetInstance().GetMaster();
								if( pPARTY_CLT->m_dwGaeaID != pMaster->m_dwGaeaID )		continue;
							}
								
							PGLCHARCLIENT pCHAR = GLGaeaClient::GetInstance().GetChar ( sFindID.dwID );
							if( !pCHAR )				continue;
							if( !pCHAR->IsDie() )		continue;
					
							sACTAR.emACTAR = EMACTAR_PC_ANY;
							sACTAR.sTARID = sFindID;

							vTargetPt = GLGaeaClient::GetInstance().GetTargetPos( sACTAR.sTARID );
							return sACTAR;
						}					
					}
					else
					{
						pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_PC, STAR_ORDER() );
						DETECTMAP_ITER iter = pairRange.first;
						for ( ; (iter!=pairRange.second); ++iter )
						{
							const STARGETID &sFindID = (*iter);
							PGLCHARCLIENT pCHAR = GLGaeaClient::GetInstance().GetChar ( sFindID.dwID );
							if( !pCHAR )				continue;
							if( pCHAR->IsDie() )		continue;

							bool	bContinue = false;
							GLPARTY_CLIENT* pPARTY_CLT = GLPartyClient::GetInstance().FindMember( sFindID.dwID );
							if( pPARTY_CLT )
							{
								if( sSKILLID == SNATIVEID(14,14) && 
								  ( pCHAR->GetCharData().emClass == GLCC_ARCHER_W || pCHAR->GetCharData().emClass == GLCC_ARCHER_M ) )
									bContinue = true;
							}
							else
							{
								if( !pLAND->m_bClubBattle &&
									!pLAND->m_bClubBattleHall &&
									!pLAND->m_bClubDeathMatch &&
									!pLAND->m_bClubDeathMatchHall && 
									!m_bBotPK ) 
										bContinue = true;

								if( pLAND->m_bClubBattleHall || pLAND->m_bClubBattle || m_bBotPK )
								{
									if( sSKILLID == SNATIVEID(14,14) && 
										pCHAR->GetCharData().emClass != GLCC_ARCHER_W && 
										pCHAR->GetCharData().emClass != GLCC_ARCHER_M )				bContinue = true;
									if( pCHAR->GetSchool() == m_wSchool )	bContinue = true;
									if( pLAND->m_bClubBattleHall || pLAND->m_bClubBattle )
									{
										if( m_sCLUB.IsAllianceGuild ( pCHAR->GETCLUBID() ) )		bContinue = true;
									}

									if( !bContinue )
									{
										bool bFindChrono = false;
										for ( int i = 0; i < SKILLFACT_SIZE; i++ )
										{
											const SSKILLFACT* pSKILLFACT = pCHAR->GET_SKILLFACT(i);
											if( pSKILLFACT->sNATIVEID == NATIVEID_NULL() )	continue;

											if( pSKILLFACT->sNATIVEID == SNATIVEID(11,6) )
											{
												bFindChrono = true;
												break;
											}
										}
										if( bFindChrono )	bContinue = false;
										else				bContinue = true;
									}
								}
							}

							for ( int i = 0; i < SKILLFACT_SIZE; i++ )
							{
								const SSKILLFACT* pSKILLFACT = pCHAR->GET_SKILLFACT(i);
								if( pSKILLFACT->sNATIVEID == NATIVEID_NULL() )	continue;

								if( pSKILLFACT->sNATIVEID == sSKILLID )
								{
									bContinue = true;
									break;
								}
							}

							if( bContinue )		continue;
														
							sACTAR.emACTAR = EMACTAR_PC_ANY;
							sACTAR.sTARID = sFindID;

							vTargetPt = GLGaeaClient::GetInstance().GetTargetPos( sACTAR.sTARID );
							return sACTAR;
						}
					}
				}
			}
		}
	}
	else if( bENEMY )
	{
		if( m_bBotLockSpot )	
			pDetectMap = GLGaeaClient::GetInstance().DetectCrowBot ( emCrow, m_vBotPos, MAX_VIEWRANGE/3.0f );
		else
			pDetectMap = GLGaeaClient::GetInstance().DetectCrowBot ( emCrow, m_vPos, MAX_VIEWRANGE/3.0f );
		
		if( pDetectMap )
		{
			if( pLAND->m_bPVPTyrannyMap || pLAND->m_bClubBattle || pLAND->m_bClubBattleHall || m_bBotPK )
			{
				STARGETID *pPVP(NULL);

				pPVP = GetCONFT_TAR(pairRange);
				if ( pPVP )
				{
					sACTAR.emACTAR = EMACTAR_PC_PVP;
					sACTAR.sTARID = *pPVP;

					vTargetPt = GLGaeaClient::GetInstance().GetTargetPos( sACTAR.sTARID );

					return sACTAR;
				}

				pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_PC, STAR_ORDER() );
				DETECTMAP_ITER iter = pairRange.first;
				for ( ; (iter!=pairRange.second); ++iter )
				{
					const STARGETID &sFindID = (*iter);
					PGLCHARCLIENT pCHAR = GLGaeaClient::GetInstance().GetChar ( sFindID.dwID );
					if( !pCHAR )			continue;
					if( pCHAR->IsDie() )	continue;

					bool bContinue = false;
					for ( int i = 0; i < SKILLFACT_SIZE; i++ )
					{
						const SSKILLFACT* pSKILLFACT = pCHAR->GET_SKILLFACT(i);
						if( pSKILLFACT->sNATIVEID == NATIVEID_NULL() )	continue;

						if( pSKILLFACT->sNATIVEID == sSKILLID )
						{
							bContinue = true;
							break;
						}
					}
					if( bContinue )		continue;

					bool bBRIGHTEVENT = GLGaeaClient::GetInstance().IsBRIGHTEVENT();

					if ( IsPK_TAR(pCHAR,bBRIGHTEVENT) )
					{
						sACTAR.emACTAR = EMACTAR_PC_PVP;
						sACTAR.sTARID = sFindID;

						vTargetPt = GLGaeaClient::GetInstance().GetTargetPos( sACTAR.sTARID );

						if( pRunSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF )
						{
							vTargetPt.y -= 2.0f;
						}

						return sACTAR;
					}
				}
			}
		}

		//	crow.
		emCrow = CROW_EX_MOB; 
		if( m_bBotLockSpot )	
			pDetectMap = GLGaeaClient::GetInstance().DetectCrowBot ( emCrow, m_vBotPos, MAX_VIEWRANGE/2.0f );
		else
			pDetectMap = GLGaeaClient::GetInstance().DetectCrow ( emCrow, m_vPos, MAX_VIEWRANGE/2.0f );

		if ( pDetectMap )
		{
			pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_MOB, STAR_ORDER() );
			if ( pairRange.first != pairRange.second )
			{
				sTID_FOUND = CFINDER::GetInstance().FindClosedCrow ( pairRange, m_vPos );

				sACTAR.emACTAR = EMACTAR_MOB;
				sACTAR.sTARID = sTID_FOUND;
				vTargetPt = GLGaeaClient::GetInstance().GetTargetPos( sTID_FOUND );

				if( pRunSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF )
				{
					vTargetPt.y -= 2.0f;
				}

				return sACTAR;
			}
		}
	}   

	SACTIONTAR sACTAR_NULL;
	vTargetPt = m_vBotPos;

	return sACTAR_NULL;
}

SACTIONTAR GLCharacter::FindNearTarget ( D3DXVECTOR3 &vTargetPt, const EMFIND_TAR emFINDTAR, bool bUseSkill )
{
	SACTIONTAR sACTAR;
	if ( IsSTATE(EM_ACT_CONTINUEMOVE) )		return sACTAR;

	bool bONLY_ITEM = false;
	bool bODER_P2P = false;

	bool bANY(false), bENEMY(false), bOUR(false), bDIE(false);
	switch ( emFINDTAR )
	{
	case ENFIND_TAR_NULL:
		bANY = true;
		bENEMY = true;
		bOUR = true;
		break;
	case EMFIND_TAR_ENEMY:
		bENEMY = true;
		break;
	case EMFIND_TAR_ANY:
		bANY = true;
		break;
	case EMFIND_TAR_OUR:
		bOUR = true;
		break;

	case EMFIND_TAR_ANY_DIE:
		bANY = true;
		bDIE = true;
		break;
	case EMFIND_TAR_OUR_DIE:
		bOUR = true;
		bDIE = true;
		break;
	};

	STARGETID sTID_FOUND;
	DETECTMAP_RANGE pairRange;

	DETECTMAP *pDetectMap = NULL;

	DWORD emCrow = CROW_EX_PC;
	pDetectMap = GLGaeaClient::GetInstance().DetectCrow ( emCrow, m_vPos, MAX_VIEWRANGE/2.0f );
	if ( bOUR && bUseSkill )
	{
		sACTAR.emACTAR = EMACTAR_PC_OUR;
		sACTAR.sTARID = STARGETID(CROW_PC,m_dwGaeaID,m_vPos);
				
		vTargetPt = GLGaeaClient::GetInstance().GetTargetPos( sACTAR.sTARID );

		return sACTAR;
	}

    emCrow = CROW_EX_MOB; 
	pDetectMap = GLGaeaClient::GetInstance().DetectCrow ( emCrow, m_vBotPos, MAX_VIEWRANGE/2.0f );
	if ( pDetectMap )
	{
		//	crow.
		pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_MOB, STAR_ORDER() );
		if ( pairRange.first != pairRange.second )
		{
			sTID_FOUND = CFINDER::GetInstance().FindClosedCrow ( pairRange, m_vBotPos );

			sACTAR.emACTAR = EMACTAR_MOB;
			sACTAR.sTARID = sTID_FOUND;
			vTargetPt = GLGaeaClient::GetInstance().GetTargetPos( sTID_FOUND );

			return sACTAR;
		}
	}

	SACTIONTAR sACTAR_NULL;
	vTargetPt = m_vBotPos;

	return sACTAR_NULL;
}

SACTIONTAR GLCharacter::FindNearItem ( bool &bLoot )
{
	SACTIONTAR sACTAR_ITEM;
	SACTIONTAR sACTAR_MONEY;
	bLoot = false;

	if ( m_fBotPotsDelay < 10.0f ) return sACTAR_ITEM;

	PLANDMANCLIENT pLAND = GLGaeaClient::GetInstance().GetActiveMap();
	if ( !pLAND )			return	sACTAR_ITEM;
	if ( !m_bAutoLoot )		return	sACTAR_ITEM;

	DETECTMAP *pDetectMap = NULL;
	DETECTMAP_RANGE pairRange;
	STARGETID sTID_FOUND;

	//DWORD emCrow = ( CROW_EX_ITEM | CROW_EX_MONEY );
	//pDetectMap = GLGaeaClient::GetInstance().DetectCrow ( emCrow, m_vPos, 60.0f );
	//if ( pDetectMap )
	//{
	//	//	item
	//	/*
	//	pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_ITEM, STAR_ORDER() );
	//	if ( pairRange.first != pairRange.second )
	//	{
	//		DETECTMAP_ITER iter = pairRange.first;
	//		for ( ; iter != pairRange.second; ++iter )
	//		{
	//			const STARGETID &sTarID = (*iter);
	//			PITEMCLIENTDROP pDropItem = pLAND->GetItem ( sTarID.dwID );
	//			if ( !pDropItem ) continue;
	//			SITEM *pItem = GLItemMan::GetInstance().GetItem(pDropItem->sItemClient.sNativeID);
	//			if ( !pItem ) continue;

	//			
	//			if ( pItem->sBasicOp.emItemType == ITEM_GRINDING || 
	//				 pItem->sBasicOp.emItemType == ITEM_MATERIALS || 
	//				 pItem->sBasicOp.emItemType == ITEM_SKILL )
	//			{
	//				if( pItem->sBasicOp.emItemType == ITEM_SKILL )
	//				{
	//					PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( pItem->sSkillBookOp.sSkill_ID );
	//					if( pSkill && pSkill->m_sLEARN.sLVL_STEP[0].dwLEVEL <= 127 ) continue;
	//				}

	//				if( pItem->sBasicOp.emItemType == ITEM_GRINDING )
	//				{
	//					if( pItem->sGrindingOp.emTYPE != EMGRINDING_DEFENSE &&
	//						pItem->sGrindingOp.emTYPE != EMGRINDING_DAMAGE  )

	//						continue;
	//				}
	//				if( pItem->sBasicOp.emItemType == ITEM_MATERIALS &&
	//					pDropItem->sItemClient.sNativeID != SNATIVEID(303,1) && 
	//					pDropItem->sItemClient.sNativeID != SNATIVEID(303,2) &&
	//					pDropItem->sItemClient.sNativeID != SNATIVEID(303,5) &&
	//					pDropItem->sItemClient.sNativeID != SNATIVEID(303,6) && 
	//					pDropItem->sItemClient.sNativeID != SNATIVEID(303,7) && 
	//					pDropItem->sItemClient.sNativeID != SNATIVEID(303,8) && 
	//					pDropItem->sItemClient.sNativeID != SNATIVEID(303,9) )	continue;
	//				bLoot = true;

	//				sTID_FOUND = sTarID;

	//				sACTAR_ITEM.emACTAR = EMACTAR_ITEM;
	//				sACTAR_ITEM.sTARID = sTID_FOUND;

	//				return sACTAR_ITEM;
	//			}
	//		}			
	//	}
	//	*/
	//	//	Money.
	//	
	//	pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_MONEY, STAR_ORDER() );
	//	if ( pairRange.first != pairRange.second )
	//	{
	//		sTID_FOUND = CFINDER::GetInstance().FindClosedCrow ( pairRange, m_vPos );

	//		sACTAR_ITEM.emACTAR = EMACTAR_ITEM;
	//		sACTAR_ITEM.sTARID = sTID_FOUND;

	//		bLoot = true;

	//		return sACTAR_ITEM;
	//	}
	//
	//}

	DWORD emCrow = ( CROW_EX_ITEM | CROW_EX_MONEY );
	pDetectMap = GLGaeaClient::GetInstance().DetectCrow ( emCrow, m_vPos, MAX_VIEWRANGE/3.0f );
	if ( pDetectMap )
	{
		//	Item.
		pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_ITEM, STAR_ORDER() );
		if ( pairRange.first != pairRange.second )
		{
			sTID_FOUND = CFINDER::GetInstance().FindClosedCrow ( pairRange, m_vPos );

			sACTAR_ITEM.emACTAR = EMACTAR_ITEM;
			sACTAR_ITEM.sTARID = sTID_FOUND;
			bLoot = true;
		}

		//	Money.
		pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_MONEY, STAR_ORDER() );
		if ( pairRange.first != pairRange.second )
		{
			sTID_FOUND = CFINDER::GetInstance().FindClosedCrow ( pairRange, m_vPos );

			sACTAR_MONEY.emACTAR = EMACTAR_ITEM;
			sACTAR_MONEY.sTARID = sTID_FOUND;
			bLoot = true;
		}
	}
	else m_fBotPotsDelay = 0.0f;

	if ( sACTAR_MONEY.emACTAR!=EMACTAR_NULL && sACTAR_ITEM.emACTAR!=EMACTAR_NULL )
	{
		std_afunc::CCompareTargetDist sDist(m_vPos);

		bool bMore = sDist.operator () ( sACTAR_MONEY.sTARID, sACTAR_ITEM.sTARID );

		if ( bMore )	return sACTAR_MONEY;
		else			return sACTAR_ITEM;
	}
	else if ( sACTAR_MONEY.emACTAR!=EMACTAR_NULL )
	{
		return sACTAR_MONEY;
	}
	else if ( sACTAR_ITEM.emACTAR!=EMACTAR_NULL )
	{
		return sACTAR_ITEM;
	}

	SACTIONTAR sACTAR;

	return sACTAR;
}

void GLCharacter::BotSkillReaction ( const STARGETID &sTargetID, const DWORD dwMR, const bool bcontinue, BOOL &bMove, D3DXVECTOR3 &vMoveTo )
{
	if ( m_bSTATE_STUN )	return;

	PLANDMANCLIENT pLAND = GLGaeaClient::GetInstance().GetActiveMap();
	if ( pLAND && pLAND->IsPeaceZone() )	return;

	SREACTION sREC_TEMP = ( m_sREACTION.sTARID == sTargetID ) ? m_sREACTION : SREACTION();
	m_sREACTION.RESET();

	m_idACTIVESKILL = NATIVEID_NULL();

	m_sOLD_TARMOB = sTargetID;

	if ( !ISLEARNED_SKILL ( m_sActiveSkill ) && !IsDefenseSkill() )	return;

	const D3DXVECTOR3 &vTarPos = sTargetID.vPos;

	float fMoveDist = FLT_MAX;
	float fDist = D3DXVec3Length ( &D3DXVECTOR3(m_vPos-vTarPos) );	// ���� �̵��� �Ÿ�
	if ( IsACTION(GLAT_MOVE) )	fMoveDist = D3DXVec3Length ( &D3DXVECTOR3(m_sTargetID.vPos-vTarPos) ); 

	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_sActiveSkill.wMainID, m_sActiveSkill.wSubID );
	if ( !pSkill )								return;

	// add safe time safezone logic
	{
		if ( IsSafeTime() || IsSafeZone() )
		{
			if ( pSkill->m_sBASIC.emIMPACT_SIDE == SIDE_ENEMY )
			{
				GLCOPY* pTarget = GLGaeaClient::GetInstance().GetCopyActor ( sTargetID );
				if ( pTarget && pTarget->GetCrow() == CROW_PC )
				{
					if ( PGLCHARCLIENT(pTarget)->IsSafeTime() || PGLCHARCLIENT(pTarget)->IsSafeZone() ) return;
				}
			}
		}
	}

	EMIMPACT_TAR emTARGET = pSkill->m_sBASIC.emIMPACT_TAR;
	//disable longrage fix
	//EMIMPACT_SIDE emSIDE = pSkill->m_sBASIC.emIMPACT_SIDE;

	if ( emTARGET==TAR_SPEC || emTARGET==TAR_SELF_TOSPEC )
	{
		GLCOPY* pTarget = GLGaeaClient::GetInstance().GetCopyActor ( sTargetID.emCrow, sTargetID.dwID );
		if ( pTarget )
		{
			WORD wAttackRange = pTarget->GetBodyRadius() + GETBODYRADIUS() + GETSKILLRANGE_TAR(*pSkill) + 2;
			WORD wAttackAbleDis = wAttackRange + 2;

			if ( fDist>wAttackAbleDis )
			{
				if ( fMoveDist > wAttackAbleDis )
				{
					D3DXVECTOR3 vPos = vTarPos - m_vPos;
					D3DXVec3Normalize ( &vPos, &vPos );

					bMove = TRUE;
					vMoveTo = vTarPos - vPos*wAttackRange;
					vMoveTo.y = vTarPos.y;

					m_sREACTION.emREACTION = REACT_SKILL;
					m_sREACTION.bCONTINUE = bcontinue;
					m_sREACTION.sTARID = sTargetID;
					m_sREACTION.vMARK_POS = vMoveTo;
				}
				else
				{
					m_sREACTION = sREC_TEMP;
				}

				return;
			}
		}
	}
	//disable longrage fix
	/*
	else if ( emTARGET == TAR_SELF || emTARGET==TAR_ZONE )
	{
		if( emSIDE != SIDE_OUR )
		{
			WORD wAttackRange = GETBODYRADIUS() + GETSKILLRANGE_TAR(*pSkill) + 2;
			WORD wAttackAbleDis = wAttackRange + 2;

			if ( fDist>wAttackAbleDis )
			{
				if ( fMoveDist > wAttackAbleDis )
				{
					D3DXVECTOR3 vPos = vTarPos - m_vPos;
					D3DXVec3Normalize ( &vPos, &vPos );

					bMove = TRUE;
					vMoveTo = vTarPos - vPos*wAttackRange;
					vMoveTo.y = vTarPos.y;

					m_sREACTION.emREACTION = REACT_SKILL;
					m_sREACTION.bCONTINUE = bcontinue;
					m_sREACTION.sTARID = sTargetID;
					m_sREACTION.vMARK_POS = vMoveTo;
				}
				else
				{
					m_sREACTION = sREC_TEMP;
				}

				return;
			}
		}
	}
	*/

	if ( emTARGET!=TAR_SELF && sTargetID.dwID!=EMTARGET_NULL )
	{
		D3DXVECTOR3 vPos1 = GetPosition();	vPos1.y += 15.0f;
		D3DXVECTOR3 vPos2 = vTarPos;		vPos2.y += 15.0f;
		
		BOOL bCollision = TRUE;
		D3DXVECTOR3 vCollision;
		LPDXFRAME pDxFrame = NULL;

		DxLandMan *pLandMan = GLGaeaClient::GetInstance().GetActiveMap()->GetLandMan();
		pLandMan->IsCollision ( vPos1, vPos2, vCollision, bCollision, pDxFrame, FALSE );
		if ( bCollision )						
		{
			//CInnerInterface::GetInstance().PrintConsoleText("Bot Can't reach target");

			return;
		}
	}

	PANIMCONTNODE pAnicont = m_pSkinChar->GETANI ( pSkill->m_sEXT_DATA.emANIMTYPE, pSkill->m_sEXT_DATA.emANISTYPE );
	if ( !pAnicont )
	{
		CDebugSet::ToLogFile ( "��Ų ���ϸ��̼��� �������� �ʽ��ϴ�. ANIM %d, ANIS %d", pSkill->m_sEXT_DATA.emANIMTYPE, pSkill->m_sEXT_DATA.emANISTYPE );
		return;
	}

	WORD wStrikeNum = pAnicont->pAnimCont->m_wStrikeCount;

	EMSKILLCHECK emSkillCheck = GLCHARLOGIC::CHECHSKILL ( m_sActiveSkill, 1, IsDefenseSkill() );
	if ( emSkillCheck!=EMSKILL_OK && emSkillCheck!=EMSKILL_NOTSP)
	{
		if ( dwMR&(DXKEY_UP) )
		{
			switch ( emSkillCheck )
			{
			case EMSKILL_DELAYTIME:
				CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SKILL_DELAYTIME") );
				break;

			case EMSKILL_NOTITEM:
				{	

					if ( pSkill->m_sBASIC.emUSE_LITEM!=ITEMATT_NOCARE && pSkill->m_sBASIC.emUSE_RITEM!=ITEMATT_NOCARE )
					{
						CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SKILL_NOTITEM_2"),
							COMMENT::ITEMATTACK[pSkill->m_sBASIC.emUSE_LITEM].c_str(), COMMENT::ITEMATTACK[pSkill->m_sBASIC.emUSE_RITEM].c_str() );
					}
					else if ( pSkill->m_sBASIC.emUSE_LITEM!=ITEMATT_NOCARE || pSkill->m_sBASIC.emUSE_RITEM!=ITEMATT_NOCARE )
					{
						std::string strITEM;
						if ( pSkill->m_sBASIC.emUSE_LITEM!=ITEMATT_NOCARE )			strITEM = COMMENT::ITEMATTACK[pSkill->m_sBASIC.emUSE_LITEM].c_str();
						if ( pSkill->m_sBASIC.emUSE_RITEM!=ITEMATT_NOCARE )			strITEM = COMMENT::ITEMATTACK[pSkill->m_sBASIC.emUSE_RITEM].c_str();

						if ( strITEM.empty() )		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SKILL_NOTITEM") );
						CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SKILL_NOTITEM_1"), strITEM.c_str() );
					}
				}
				break;

			case EMSKILL_NOTARROW:
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SKILL_NOTITEM_ARROW") );
				break;

			case EMSKILL_NOTCHARM:
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SKILL_NOTITEM_CHARM") );
				break;

			case EMSKILL_NOTHP:
				CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SKILL_NOTHP") );
				break;

			case EMSKILL_NOTMP:
				CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SKILL_NOTMP") );
				break;

			case EMSKILL_NOTSP:
				GASSERT(0&&"SP�� �����ϴ��� ��ų�� �ߵ��Ǿ� �մϴ�.");
				break;

			case EMSKILL_NOTEXP:
				CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SKILL_NOTEXP") );
				break;

			//case EMSKILL_NOTSKILL: //add antiskill
			//	CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, "Unable to use skills." );
			//	break;

			default:
				CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SKILL_DOFAIL") );
				break;
			};
		}
		return;
	}

	//	Note : Ŭ������ ã��.
	//
	D3DXVECTOR3 *pvTarPos = NULL;
	D3DXVECTOR3 vFromPt;
	vFromPt = DxViewPort::GetInstance().GetFromPt ();

	{
		D3DXVECTOR3 vCollisionPos;
		DWORD dwCollisionID;
		BOOL bCollision;
		
		GLGaeaClient::GetInstance().GetActiveMap()->GetNaviMesh()->IsCollision
		(
			vFromPt,
			m_vBotTargetPt,
			vCollisionPos,
			dwCollisionID,
			bCollision
		);			

		if ( bCollision )	pvTarPos = &vCollisionPos;
	}	

	//	Note : Skill ��ǥ�� ������.
	//
	BOOL bOK = SelectSkillTarget ( sTargetID, pvTarPos );
	if ( !bOK )
	{
		//	��ǥ���� ���ų� ��ǥ ������ ��ȿ���� ������.
		if ( dwMR&(DXKEY_UP) )		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("NOTARGET_SKILL") );
		return;
	}


	//	Note : ���� ȸ���� �ʿ��� ��� ������ ư��.
	//
	if ( emTARGET==TAR_SPEC || emTARGET==TAR_SELF_TOSPEC )
	{
		if ( m_sTARIDS[0].wID != USHRT_MAX )
		{
			STARGETID sRotateTarID ( static_cast<EMCROW>(m_sTARIDS[0].wCrow), static_cast<DWORD>(m_sTARIDS[0].wID) );
			GLCOPY* pTarget = GLGaeaClient::GetInstance().GetCopyActor ( sRotateTarID );
			if ( pTarget )
			{
				D3DXVECTOR3 vDirection = pTarget->GetPosition() - m_vPos;
				D3DXVec3Normalize ( &vDirection, &vDirection );
				m_vDir = vDirection;
			}
		}

		//	Note : ���� ��ġ�� �������� Transform ��Ʈ���� ���.
		//
		D3DXMATRIX matTrans, matYRot;
		D3DXMatrixTranslation ( &matTrans, m_vPos.x, m_vPos.y, m_vPos.z );
		float fThetaY = DXGetThetaYFromDirection ( m_vDir, m_vDirOrig );
		D3DXMatrixRotationY ( &matYRot, fThetaY );

		m_matTrans = matYRot * matTrans;
	}
	else if ( emTARGET==TAR_ZONE )
	{
		D3DXVECTOR3 vDirection = m_vTARPOS - m_vPos;
		D3DXVec3Normalize ( &vDirection, &vDirection );
		m_vDir = vDirection;
	}
	else if ( emTARGET==TAR_SELF )
	{
		//	Note : ���� ȸ�� �ʿ� ����.
	}

	//	Note : ��ų�� �ߵ���.
	//

	//CInnerInterface::GetInstance().PrintConsoleText("Bot Use skill");

	if ( IsACTION ( GLAT_IDLE ) || !(pSkill->m_sBASIC.dwFlags & FLAG_SB_RUNNING_CAST ) ) TurnAction ( GLAT_SKILL );
	else
	{
		m_pSkinChar->m_bResetSkillAni = TRUE;
		StartSkillProc ();
		SetHiddenWeaponPiece();
	}
	
	//TurnAction ( GLAT_SKILL );

	if ( bcontinue )
	{
		m_sREACTION.emREACTION = REACT_SKILL;
		m_sREACTION.bCONTINUE = true;
		m_sREACTION.sTARID = sTargetID;
		m_sREACTION.vMARK_POS = m_vPos;
	}

	return;
}



//dmk14 | 11-1-16 | AI - auto pilot
HRESULT GLCharacter::SetSelectSkillTarget( const WORD wSLOT, D3DXVECTOR3 &vTargetPt, SACTIONTAR &sACTAR, bool &bForceBuff )
{
	if ( EMSKILLQUICK_SIZE <= wSLOT )	return E_FAIL;

	const SNATIVEID &skill_id = m_sSKILLQUICK[wSLOT];
	if ( skill_id==NATIVEID_NULL() )	return E_FAIL;

	if ( !ISLEARNED_SKILL(skill_id) )	return E_FAIL;

	EMSKILLCHECK emSkillCheck = GLCHARLOGIC::CHECHSKILL ( skill_id, 1, IsDefenseSkill() );
	if ( emSkillCheck!=EMSKILL_OK && emSkillCheck!=EMSKILL_NOTSP )
	{
		switch ( emSkillCheck )
		{
			case EMSKILL_DELAYTIME:
			case EMSKILL_NOTITEM:
			case EMSKILL_NOTARROW:
			case EMSKILL_NOTCHARM:
			case EMSKILL_NOTHP:
			case EMSKILL_NOTMP:
			case EMSKILL_NOTSP:
			case EMSKILL_NOTEXP:
				return E_FAIL;
		};
	}

	PGLSKILL pRunSkill = GLSkillMan::GetInstance().GetData ( skill_id );
	EMFIND_TAR emFINDTAR(ENFIND_TAR_NULL);
	if ( pRunSkill )
	{
		if ( pRunSkill->m_sAPPLY.IsSpec( EMSPECA_REBIRTH ) )
		{
			switch ( pRunSkill->m_sBASIC.emIMPACT_SIDE )
			{
			default:				break;
			case SIDE_ENEMY:		break;
			case SIDE_OUR:			emFINDTAR = EMFIND_TAR_OUR_DIE;		break;
			case SIDE_ANYBODY:		emFINDTAR = EMFIND_TAR_ANY_DIE;		break;
			};
		}
		else
		{
			switch ( pRunSkill->m_sBASIC.emIMPACT_SIDE )
			{
			default:				break;
			case SIDE_ENEMY:		emFINDTAR = EMFIND_TAR_ENEMY;	break;
			case SIDE_OUR:			emFINDTAR = EMFIND_TAR_OUR;		break;
			case SIDE_ANYBODY:		emFINDTAR = EMFIND_TAR_ANY;		break;
			};
		}
	}
	m_sACTAR = FindNearTarget( m_vBotTargetPt, emFINDTAR, skill_id );
	if( m_sACTAR.emACTAR == EMACTAR_NULL )		return E_FAIL;

	for( int i=0;i<SKILLFACT_SIZE;++i )
	{
		if ( m_sSKILLFACT[i].sNATIVEID == skill_id ) 
		{
			if ( m_sSKILLFACT[i].fAGE > 10.0f )		return E_FAIL;
		}
	}

	if ( pRunSkill->m_sBASIC.sNATIVEID.wMainID ==  44 &&
		pRunSkill->m_sBASIC.sNATIVEID.wSubID == 15 ) return E_FAIL;

	if( m_sRunSkill == skill_id )	return S_OK;

	m_sRunSkill = skill_id;
	bForceBuff = true;

	GLMSG::SNETPC_REQ_SKILLQUICK_ACTIVE NetMsg;
	NetMsg.wSLOT = wSLOT;
	NETSENDTOFIELD ( &NetMsg );

	return S_OK;
}

HRESULT GLCharacter::SetSkillRunSet ( const WORD wSLOT )
{
	if ( EMSKILLQUICK_SIZE <= wSLOT )	return E_FAIL;

	const SNATIVEID &skill_id = m_sSKILLQUICK[wSLOT];
	if ( skill_id==NATIVEID_NULL() )	return E_FAIL;

	if ( !ISLEARNED_SKILL(skill_id) )	return E_FAIL;

	bool bSkillFact = false;
	for( int i=0;i<SKILLFACT_SIZE;++i )
	{
		if ( m_sSKILLFACT[i].sNATIVEID == m_sRunSkill ) bSkillFact = true;
	}

	EMSKILLCHECK emSkillCheck = GLCHARLOGIC::CHECHSKILL ( m_sRunSkill, 1, IsDefenseSkill() );
	if ( emSkillCheck==EMSKILL_OK && !bSkillFact )	return S_OK;

	emSkillCheck = GLCHARLOGIC::CHECHSKILL ( skill_id, 1, IsDefenseSkill() );
	if ( emSkillCheck!=EMSKILL_OK && emSkillCheck!=EMSKILL_NOTSP )
	{
		switch ( emSkillCheck )
		{
			case EMSKILL_DELAYTIME:
			case EMSKILL_NOTITEM:
			case EMSKILL_NOTARROW:
			case EMSKILL_NOTCHARM:
			case EMSKILL_NOTHP:
			case EMSKILL_NOTMP:
			case EMSKILL_NOTSP:
			case EMSKILL_NOTEXP:
				return E_FAIL;
		};
	}

	for( int i=0;i<SKILLFACT_SIZE;++i )
	{
		if ( m_sSKILLFACT[i].sNATIVEID == skill_id )	return E_FAIL;
	}

	if( m_sRunSkill == skill_id )	return S_OK;

	m_sRunSkill = skill_id;

	GLMSG::SNETPC_REQ_SKILLQUICK_ACTIVE NetMsg;
	NetMsg.wSLOT = wSLOT;
	NETSENDTOFIELD ( &NetMsg );

	return S_OK;
}

HRESULT GLCharacter::RunAutoPots( float fTime, float fElapsedTime )
{
	if( IsDie() )	{	return E_FAIL;	}

	m_fBotPotsDelay += fElapsedTime;
	if( m_fBotPotsDelay <= 0.01f )	return E_FAIL;
	m_fBotPotsDelay = 0.0f;

	int wHpPercent =  ( float(m_sHP.dwNow) / float(m_sHP.dwMax) ) * 100;
	int wMpPercent =  ( float(m_sMP.dwNow) / float(m_sMP.dwMax) ) * 100;
	int wSpPercent =  ( float(m_sSP.dwNow) / float(m_sSP.dwMax) ) * 100;

	for( int i=0;i<EMACTIONQUICK_SIZE;++i )
	{
		SITEM* pItem = GLItemMan::GetInstance().GetItem ( m_sACTIONQUICK[i].sNID );
		if ( pItem ) 
		{
			//if( m_bBotReturn && pItem->sBasicOp.emItemType == ITEM_RECALL && pItem->sDrugOp.emDrug == ITEM_DRUG_CALL_LASTCALL )
			//{
			//	m_bBotReturn = false;
			//	ReqActionQ( (WORD) i );
			//}
			//else
			{
				srand(time(0));
				int nRand = rand() % 50 + 1;
				PLANDMANCLIENT pLAND = GLGaeaClient::GetInstance().GetActiveMap();
				if( pLAND && !pLAND->m_bPVPTyrannyMap )
				{
					if( pItem->sDrugOp.emDrug == ITEM_DRUG_HP ||
					pItem->sDrugOp.emDrug == ITEM_DRUG_HP_MP ||
					pItem->sDrugOp.emDrug == ITEM_DRUG_HP_MP_SP )
					{
						if( wHpPercent <= (80+nRand) )	ReqActionQ( (WORD) i );
					}
				}
				
				if( pItem->sDrugOp.emDrug == ITEM_DRUG_MP ||
					pItem->sDrugOp.emDrug == ITEM_DRUG_HP_MP ||
					pItem->sDrugOp.emDrug == ITEM_DRUG_HP_MP_SP )
				{
					if( wMpPercent <= nRand )	ReqActionQ( (WORD) i );
				}

				if( pItem->sDrugOp.emDrug == ITEM_DRUG_SP ||
					pItem->sDrugOp.emDrug == ITEM_DRUG_HP_MP_SP )
				{
					if( wSpPercent <= nRand )	ReqActionQ( (WORD) i );
				}
			}
		}
	}

	return S_OK;
}


//dmk14 | 11-1-16 | AI - auto pilot
HRESULT GLCharacter::ReqToggleRunBot()
{
	HRESULT hr = S_OK;
	

	PLANDMANCLIENT pLandManClient = GLGaeaClient::GetInstance().GetActiveMap ();
	if ( pLandManClient && !pLandManClient->IsInstantMap() )
	{
		if ( pLandManClient->m_bClubBattle || pLandManClient->m_bClubDeathMatch ||
			pLandManClient->m_bPVPCaptureTheFlagMap || pLandManClient->m_bPVPSchoolWarsMap ||
			pLandManClient->m_bPVPTyrannyMap)
		{
			return E_FAIL;
		}
	}
	
	if( !m_bRunBot )	
	{
		m_bRunBot = true;
		m_vBotPos = m_vPos;
		CInnerInterface::GetInstance().PrintMsgText (NS_UITEXTCOLOR::GREEN_WOW, ID2GAMEINTEXT("AUTOSKILL_SYSTEM_START") );
	}
	else				
	{
		m_bRunBot = false;
		CInnerInterface::GetInstance().PrintMsgText (NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("AUTOSKILL_SYSTEM_STOP") );
	}

	return S_OK;
}

HRESULT GLCharacter::SetBotSettings( bool bSupport, bool bLoot, bool bPk, bool bResuPtLeader, bool bFollowLead, bool bLockSpot )
{
	m_bBotSupport = bSupport;
	m_bAutoLoot = bLoot;
	m_bBotPK = bPk;
	m_bResuPtLeader = bResuPtLeader;
	m_bBotFLeader = bFollowLead;
	m_bBotLockSpot = bLockSpot;

	return S_OK;
}

//dmk14 | 11-1-16 | AI - auto pilot
HRESULT GLCharacter::FrameMoveBot( float fTime, float fElapsedTime )
{
	if( IsDie() )	return E_FAIL;

	//tw bot disable
	PLANDMANCLIENT pLander = GLGaeaClient::GetInstance().GetActiveMap();
	if( pLander )
	{
		if( pLander->m_bPVPTyrannyMap || pLander->m_bPVPSchoolWarsMap ) return E_FAIL;
	}

	if( IsACTION( GLAT_SKILL ) || IsACTION( GLAT_PUSHPULL ) || IsACTION( GLAT_GATHERING ) )		return E_FAIL;

	D3DXVECTOR3 vFromPt;
	vFromPt = DxViewPort::GetInstance().GetFromPt ();
	DWORD dwML( DXKEY_DOWNED ), dwMR( DXKEY_DOWNED );

	bool bLoot = false;
	bool bBuff = false;
	SACTIONTAR sActar = FindNearItem( bLoot );
	if( bLoot && m_fIdleTime >= 2.0f )	bLoot = false;
	if( !bLoot )
	{
		for( int i = 0; i < 10; ++i)
		{
			INT nIndex = 2 * 10 + i;
			if( SetSelectSkillTarget( nIndex, m_vBotTargetPt, m_sACTAR, bBuff ) == S_OK )	
			{	
				break;	
			}
		}

		PLANDMANCLIENT pLAND = GLGaeaClient::GetInstance().GetActiveMap();
		if( pLAND && bBuff )
		{
			/*if( pLAND->m_bTowerWars )
			{
				int wHpPercent =  ( float(m_sHP.dwNow) / float(m_sHP.dwMax) ) * 100;
				if( wHpPercent <= 60 )	bBuff = false;
				//
				DWORD dwPARTYID = GLPartyClient::GetInstance().GetPartyID();
				if ( dwPARTYID!=PARTY_NULL )
				{
					DETECTMAP_RANGE pairRange;
					DETECTMAP *pDetectMap = GLGaeaClient::GetInstance().DetectCrowBot ( CROW_EX_PC, m_vPos, MAX_VIEWRANGE/3.0f );

					if ( pDetectMap )
					{
						pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_PC, STAR_ORDER() );
						if ( pairRange.first != pairRange.second )
						{
							STARGETID sTargetLowHP;

							pairRange = std::equal_range ( pDetectMap->begin(), pDetectMap->end(), CROW_PC, STAR_ORDER() );
							DETECTMAP_ITER iter = pairRange.first;
							for ( ; (iter!=pairRange.second); ++iter )
							{
								const STARGETID &sFindID = (*iter);
								GLPARTY_CLIENT* pPARTY_CLT = GLPartyClient::GetInstance().FindMember( sFindID.dwID );
								PGLCHARCLIENT pCHAR = GLGaeaClient::GetInstance().GetChar ( sFindID.dwID );
								if( !pCHAR )				continue;
								if( pCHAR->IsDie() )		continue;
								if( !pPARTY_CLT )			continue;
								
								GLPADATA sHP = pPARTY_CLT->m_sHP;
								float fPercent = ( float(sHP.dwNow) / float(sHP.dwMax) ) * 100.0f;
								float fDist = D3DXVec3Length ( &D3DXVECTOR3(m_vPos-pPARTY_CLT->m_vPos) );

								if( fPercent <= 70.0f && fDist < 60.0f )
								{
									bBuff = false;
									break;
								}
							}
						}
					}
				}
			}*/
		}
		

		if( !bBuff )
		{
			for( int j = 0; j < 2 ; ++j)
			{
				bool bUse(false);
				for( int i = 0; i < 10; ++i)
				{
					INT nIndex = j * 10 + i;
					if( SetSelectSkillTarget( nIndex, m_vBotTargetPt, m_sACTAR, bBuff ) == S_OK )	
					{	
						bUse = true; 
						break;	
					}
				}
				if( bUse ) break;
			}
		}
	}
	else 
	{
		m_sACTAR = sActar;
	}

	if( m_sACTAR.emACTAR == EMACTAR_NULL )
	{
		if( sActar.emACTAR == EMACTAR_ITEM && !bLoot )
		{
			m_sACTAR = sActar;
		}
		else
		{
			for( int i = 0; i < 10; ++i)
			{
				INT nIndex = 2 * 10 + i;
				if( SetSelectSkillTarget( nIndex, m_vBotTargetPt, m_sACTAR, bBuff ) == S_OK )	
				{	
					break;	
				}
			}
		}
	}	
	
	PGLSKILL pRunSkill(NULL);
	if ( dwMR&(DXKEY_DOWNED|DXKEY_UP|DXKEY_DUP) )
	{
		m_sActiveSkill = m_sRunSkill;

		SKILL_MAP_ITER learniter = m_ExpSkills.find ( m_sActiveSkill.dwID );
		if ( learniter!=m_ExpSkills.end() )
		{
			SCHARSKILL &sSkill = (*learniter).second;
			pRunSkill = GLSkillMan::GetInstance().GetData ( m_sActiveSkill.wMainID, m_sActiveSkill.wSubID );
		}
	}

	PLANDMANCLIENT pLAND = GLGaeaClient::GetInstance().GetActiveMap();
	if( pLAND && m_bBotFLeader )
	{
		DWORD dwPARTYID = GLPartyClient::GetInstance().GetPartyID();
		if ( dwPARTYID!=PARTY_NULL )
		{
			GLPARTY_CLIENT* pPARTY_CLT = GLPartyClient::GetInstance().GetMaster();
			if( pPARTY_CLT )
			{
				srand(unsigned int(time(NULL)));
				float idx = 0.0f;

				float fDist = D3DXVec3Length ( &D3DXVECTOR3(m_vPos-pPARTY_CLT->m_vPos) );
				if ( fDist>45.0f )
				{
					m_vBotPos = pPARTY_CLT->m_vPos;
					idx = (float)(rand() % 29 + (-19)); 
					m_vBotPos.y -= idx;
					m_sACTAR.emACTAR = EMACTAR_NULL;
					pRunSkill = NULL;
				}
			}
		}
	}

	GLCOPY* pACTTAR(NULL);

	EMCROW emCROW = m_sACTAR.sTARID.emCrow;

	bool bBRIGHTEVENT = GLGaeaClient::GetInstance().IsBRIGHTEVENT();

	if ( (emCROW==CROW_PC&&m_sACTAR.emACTAR==EMACTAR_PC_PVP) || m_sACTAR.emACTAR==EMACTAR_MOB || ( bBRIGHTEVENT&&m_sACTAR.emACTAR!=EMACTAR_NPC ) ||
		(emCROW==CROW_SUMMON&&m_sACTAR.emACTAR==EMACTAR_SUMMON_ATTACK) )
	{
		pACTTAR = GLGaeaClient::GetInstance().GetCopyActor ( m_sACTAR.sTARID );
	}

	if ( pACTTAR )
	{
		D3DXVECTOR3 vCenterTarget = m_sACTAR.sTARID.vPos;
		vCenterTarget.y += 10.f;

		D3DXVECTOR3 vFromTEMP = vFromPt;
		if( !GLGaeaClient::GetInstance().IsMapCollsion( vCenterTarget, vFromTEMP ) )
		{
			CInnerInterface::GetInstance().SetTargetInfo ( m_sACTAR.sTARID );
		}
	}

	bool bcontinue = ( m_sACTAR.emACTAR != EMACTAR_NULL );

	BOOL bMove(FALSE);
	D3DXVECTOR3	vMoveTo(0,0,0);
	if ( !IsACTION(GLAT_ATTACK) && !IsACTION(GLAT_SKILL) && !m_bVehicle && !m_bReqVehicle )
	{
		STARGETID sTARID = m_sACTAR.sTARID;

		if ( bLoot )
		{
			if ( m_sACTAR.emACTAR==EMACTAR_ITEM )
				ItemReaction(sTARID,DXKEY_DOWNED|DXKEY_UP,bcontinue,bMove,vMoveTo);
		}
		else if ( pRunSkill )
		{			
				SetDefenseSkill( false );

				EMIMPACT_TAR emTAR = pRunSkill->m_sBASIC.emIMPACT_TAR;
				bool bSKILL_ACT = ( emTAR==TAR_SELF || emTAR==TAR_ZONE );

				bool bSK2ENERMY(false), bSK2OUR(false), bSK2ANYBODY(false);
				switch ( pRunSkill->m_sBASIC.emIMPACT_SIDE )
				{
				default:				break;
				case SIDE_ENEMY:
					bSK2ENERMY = true;
					bcontinue = false;	
					break;
				
				case SIDE_OUR:
					bSK2OUR = true;
					bcontinue = false;	
					break;
				
				case SIDE_ANYBODY:
					bSK2ANYBODY = true;
					bcontinue = false;	
					break;
				};

				if ( bSKILL_ACT && m_sACTAR.emACTAR==EMACTAR_NULL )
				{
					D3DXVECTOR3 vCollisionPos;
					DWORD dwCollisionID;
					BOOL bCollision;
					GLGaeaClient::GetInstance().GetActiveMap()->GetNaviMesh()->IsCollision
					(
						vFromPt, m_vBotTargetPt,
						vCollisionPos, dwCollisionID, bCollision
					);

					if ( bCollision )
					{
						D3DXVECTOR3 vDir = vCollisionPos - GetPosition();
						sTARID.vPos = vCollisionPos;
					}
				}

				//if( m_sACTAR.emACTAR==EMACTAR_NULL && bSKILL_ACT && !bSK2OUR )	bSKILL_ACT = false;

				switch ( m_sACTAR.emACTAR )
				{
				case EMACTAR_NULL:			if(bSKILL_ACT)	BotSkillReaction(sTARID,dwMR,bcontinue,bMove,vMoveTo);		break;
				case EMACTAR_SUMMON_ATTACK: if(bSK2ENERMY)	BotSkillReaction(sTARID,dwMR,bcontinue,bMove,vMoveTo);		break;
				case EMACTAR_PC_PVP:		if(bSK2ENERMY)	BotSkillReaction(sTARID,dwMR,bcontinue,bMove,vMoveTo);		break;
				case EMACTAR_MOB:			if(bSK2ENERMY)	BotSkillReaction(sTARID,dwMR,bcontinue,bMove,vMoveTo);		break;
				case EMACTAR_NPC:			break;
				case EMACTAR_PC_OUR:		if(bSK2OUR)		BotSkillReaction(sTARID,dwMR,bcontinue,bMove,vMoveTo);		break;
				case EMACTAR_PC_P2P:		break;
				case EMACTAR_PC_ANY:		if(bSK2ANYBODY)	BotSkillReaction(sTARID,dwMR,bcontinue,bMove,vMoveTo);		break;
				case EMACTAR_ITEM:			break;
				case EMACTAR_MATERIAL:		break;
				};
		}
		else if ( dwML&(DXKEY_DOWNED|DXKEY_UP|DXKEY_DUP) )
		{
			if( m_sACTAR.emACTAR==EMACTAR_NULL )	dwML = NULL;

			switch ( m_sACTAR.emACTAR )
			{
			case EMACTAR_NULL:			break;
			case EMACTAR_SUMMON_ATTACK: PvPReaction(sTARID,dwML,bcontinue,bMove,vMoveTo);	break;
			case EMACTAR_PC_PVP:		PvPReaction(sTARID,dwML,bcontinue,bMove,vMoveTo);	break;
			case EMACTAR_MOB:			MobReaction(sTARID,dwML,bcontinue,bMove,vMoveTo);	break;
			case EMACTAR_NPC:			NpcReaction(sTARID,dwML,bcontinue,bMove,vMoveTo);	break;
			case EMACTAR_PC_OUR:		break;
			case EMACTAR_PC_P2P:		P2PReaction(sTARID,dwML,bcontinue,bMove,vMoveTo);	break;
			case EMACTAR_PC_ANY:		break;
			case EMACTAR_ITEM:			ItemReaction(sTARID,dwML,bcontinue,bMove,vMoveTo);	break;
			case EMACTAR_MATERIAL:		GatheringReaction(sTARID,dwML,bcontinue,bMove,vMoveTo);	break;
			};
		}
	}

	if ( bMove )
	{
		ActionMoveTo ( fTime, vMoveTo+D3DXVECTOR3(0,+5,0), vMoveTo+D3DXVECTOR3(0,-5,0), FALSE, TRUE );
		return S_OK;
	}
	else if ( m_sACTAR.emACTAR==EMACTAR_NULL )
	{
		D3DXVECTOR3 vDist = m_vPos - m_vBotPos;
		float fDist = D3DXVec3Length(&vDist);
		if ( 5.0f < fDist )
		{
			ActionMoveTo ( fTime, m_vBotPos+D3DXVECTOR3(0,+5,0), m_vBotPos+D3DXVECTOR3(0,-5,0), FALSE, TRUE );
			ReSetSTATE(EM_ACT_CONTINUEMOVE);
			return S_OK;
		}		
	}
	//else
	//{
	//	ActionMoveTo ( fTime, m_vBotTargetPt+D3DXVECTOR3(0,+5,0), m_sACTAR.sTARID.vPos+D3DXVECTOR3(0,-5,0), FALSE, TRUE );
	//	return S_OK;
	//}

	return S_OK;
}

/*PartySearch - Added By Bubu*/
HRESULT GLCharacter::ReqPartySearchJoin(DWORD dwGaeaID, DWORD dwPartyID )
{
	DWORD dwPARTYID = GLPartyClient::GetInstance().GetPartyID();

	if ( m_dwGaeaID == dwGaeaID ) 
		return E_FAIL;

	if ( dwPARTYID != PARTY_NULL ) 
		return E_FAIL;

	GLMSG::SNET_PARTY_SEARCH_LURE NetMsg;
	NetMsg.dwGaeaID = dwGaeaID;
	NetMsg.dwPartyID = dwPartyID;
	NETSEND ( (NET_MSG_GENERIC *) &NetMsg );
	

	return S_OK;
}