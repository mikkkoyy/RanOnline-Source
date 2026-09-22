#include "StdAfx.h"
#include "AdminMessageDisplay.h"
#include "BasicChat.h"
#include "BasicInfoView.h"
#include "BasicQuickSkillSlot.h"
#include "BasicVarTextBox.h"
#include "BlockProgramAlarm.h"
#include "CommonWeb.h"
#include "DxGlobalStage.h"
#include "DxInputString.h"
#include "GameTextControl.h"
#include "GLGaeaClient.h"
#include "GLPartyClient.h"
#include "InnerInterface.h"
//#include "InventoryWindow.h"
#include "InventoryUI.h"
#include "ItemShopWindowWeb.h"
#include "MapMoveDisplay.h"
#include "ModalCallerID.h"
#include "ModalWindow.h"
#include "NameDisplayMan.h"
#include "PrivateMarketWindow.h"
#include "QuestAlarm.h"
#include "QuestWindow.h"
#include "RANPARAM.h"
#include "RebirthDialogue.h"
#include "RebirthWindow.h"
#include "ShopItemSearchWindow.h"
#include "ItemSearchResultWindow.h"
#include "MapRequireCheck.h"
#include "SkillTrayTab.h"
#include "DxResponseMan.h"
#include "SystemMessageWindow.h"
#include "UILeftTopGroup.h"
#include "UITextControl.h"
#include "s_CHttpPatch.h"
#include "ItemMixWindow.h"
#include "GatherGauge.h"
#include "CdmRankingDisplay.h"
#include "DialogueWindowRenewal.h"

#include "CharacterWindow/CharacterWindow.h"
#include "../../Lib_Engine/GUInterface/NSGUI.h"

/*game stats, Juver, 2017/06/22 */
#include "GameStats.h" 

/*npc shop, Juver, 2017/07/25 */
#include "NPCShopWindow.h" 

 /*item preview, Juver, 2017/07/27 */
#include "ItemPreviewWindow.h"

 /*pet status, Juver, 2017/07/30 */
#include "PetStatus.h"

/*vehicle status, Juver, 2017/07/30 */
#include "VehicleStatus.h" 

/*item link, Juver, 2017/07/31 */
#include "BasicVarTextBoxItemLink.h" 

/*vehicle booster system, Juver, 2017/08/11 */
#include "VehicleBoosterDisplay.h" 

/*box contents, Juver, 2017/08/30 */
#include "InfoBoxContents.h"		

/* Competition UI, Juver, 2017/08/24 */
#include "CompetitionNotifyButton.h"
#include "CompetitionWindow.h"

/* extra notice, Juver, 2017/08/24 */
#include "ExtraNotice.h" 

/*pvp tyranny, Juver, 2017/08/25 */
#include "PVPTyrannyTowerCapture.h"
#include "PVPTyrannyTowerProgress.h"
#include "PVPTyrannyRebirthDialogue.h"
#include "PVPTyrannyCaptureNotice.h"
#include "PVPTyranny2CaptureNotice.h"
#include "PVPTyrannyRankings.h"
#include "PVPTyrannyTopRankings.h"
#include "PVPTyrannyBattleNotice.h"

/*crow target info*/
#include "CrowTargetInfo.h"
#include "CrowTargetInfoNpc.h"
#include "CrowTargetInfoPlayer.h"

/*item exchange, Juver, 2017/10/12 */
#include "NPCItemExchangeWindow.h"

/*product item, Juver, 2017/10/15 */
#include "ProductButton.h"
#include "ProductWindow.h"

/*student record ui, Juver, 2017/10/30 */
#include "StudentRecordButton.h"
#include "StudentRecordWindow.h"

/*activity system, Juver, 2017/11/05 */
#include "ActivityItemWindow.h"

/*quest ui, Juver, 2017/11/06 */
#include "ModernQuestWindow.h"

/*charinfoview , Juver, 2017/11/11 */
#include "CharacterInfoViewWindow.h"

/*bike color , Juver, 2017/11/12 */
#include "BikeColorWindow.h"

/*pet skill info display, Juver, 2017/12/01 */
/*dual pet skill, Juver, 2017/12/29 */
#include "PetSkillInfoDisplay.h"
#include "PetSkillInfoDisplay2.h"

/*change scale card, Juver, 2018/01/03 */
#include "ChangeScaleWindow.h"

/*item color, Juver, 2018/01/10 */
#include "ItemColorWindow.h"

/*change school card, Juver, 2018/01/12 */
#include "ChangeSchoolWindow.h"

/*equipment lock, Juver, 2018/01/13 */
#include "EquipmentLockEnableWindow.h"
#include "EquipmentLockInputWindow.h"
#include "EquipmentLockRecoverWindow.h"

/*storage lock, Juver, 2018/01/13 */
#include "StorageLockEnableWindow.h"
#include "StorageLockInputWindow.h"
#include "StorageLockRecoverWindow.h"

/*inventory lock, Juver, 2018/01/13 */
#include "InventoryLockEnableWindow.h"
#include "InventoryLockInputWindow.h"
#include "InventoryLockRecoverWindow.h"

/*item transfer card, Juver, 2018/01/18 */
#include "ItemTransferInventoryWindow.h"
#include "ItemTransferWindow.h"

/*school wars, Juver, 2018/01/19 */
#include "PVPSchoolWarsScore.h"
#include "PVPSchoolWarsRebirthDialogue.h"
#include "PVPSchoolWarsTopRankings.h"
#include "PVPSchoolWarsBattleNotice.h"
#include "PVPSchoolWarsRankings.h"

/*pvp capture the flag, Juver, 2018/02/01 */
#include "PVPCaptureTheFlagTopRankings.h"
#include "PVPCaptureTheFlagRebirthDialogue.h"
#include "PVPCaptureTheFlagBattleNotice.h"
#include "PVPCaptureTheFlagScore.h"
#include "PVPCaptureTheFlagRankings.h"
#include "PVPCaptureTheFlagHoldIcon.h"

/* car, cart color, Juver, 2018/02/14 */
#include "CarColorWindow.h"

#include "../Lib_Engine/GUInterface/UIKeyCheck.h"
#include "PKRankNotification.h"

#include "PartyDisplay.h"

#include "StaticUIManager.h"
#include "ItemInfoToolTip.h"
#include "SkillInforToolTip.h"
#include "SkillLinkInfoToolTip.h"

#include "LargeMapWindow.h"
#include "LargeMapWindowImage.h"
#include "MobPreviewWindow.h"

#include "SecurityWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define CHAT_MACRO_SIZE	10

CInnerInterface& CInnerInterface::GetInstance()
{
	static CInnerInterface Instance;
	return Instance;
}

const float	CInnerInterface::fMENU_LIFTUP = 90.0f;
const int CInnerInterface::nOUTOFRANGE = -1;

CInnerInterface::CInnerInterface () :
	m_emConflictReqType ( EMCONFT_ONE ),
	m_ConflictReqID ( GAEAID_NULL ),
	m_bPartyStateBack ( FALSE ),
	m_bBlockProgramFound ( false ),
	m_dwEventQuestID ( NATIVEID_NULL().dwID ),
	m_dwEventQuestStep ( UINT_MAX ),
	m_bBlockProgramAlarm ( false ),
	m_bITEM_INFO_EX_DISPLAY_MODE ( false ),
	m_bITEM_INFO_EX_EX_DISPLAY_MODE ( false ),
	m_bITEM_INFO_ITEM_LINK_DISPLAY_MODE ( false ), /*item link, Juver, 2017/07/31 */
	m_bITEM_INFO_ITEM_LINK_EX_DISPLAY_MODE ( false ),
	m_bCHANNEL ( false ),
	m_bFirstVNGainSysCall ( true ),
	m_bTabReserve ( false ),
	m_bItemShopLoad ( false ),
	m_bPVPTyrannyShowRanking(false),		/*pvp tyranny, Juver, 2017/08/25 */
	m_bPVPSchoolWarsShowRanking(false),		/*school wars, Juver, 2018/01/20 */
	m_bPVPCaptureTheFlagShowRanking(false),	/*pvp capture the flag, Juver, 2018/02/02 */
	m_bFBuff(false),
	m_nINFO_DISPLAY_TYPE(EM_INFO_DISPLAY_BASIC),
	/*PartySearch - Added By Bubu*/
	m_dwRequestorGaeaID(GAEAID_NULL),
	m_dwRequestorLevel(0),
	m_dwRequestorSchool(0),
	emRequestorClass(GLCC_BRAWLER_M)
{
	ResetControl();

	
}

CInnerInterface::~CInnerInterface ()
{
}

void CInnerInterface::ResetControl()
{
	//PANDORA
	// m_pPandoraBoxButton = NULL;
	/////////////////////////////////////////////////////////////////////////////
	// LG-7 GlobalRanking
	m_pGlobalRankingWindow	= NULL;
	/////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////
	// LG-7 CWRanking
	m_pCWRankingDisplay		= NULL;
	/////////////////////////////////////////////////////////////////////////////

	m_pSimpleHP = NULL;
	m_pBusWindow = NULL;
	m_pMiniMap = NULL;
	m_pChat = NULL;
	m_pQuestWindow = NULL;
	m_pQuestAlarm = NULL;	
	m_pFriendWindow = NULL;	
	m_pSkillWindowToTray = NULL;	
	//m_pInventoryWindow = NULL;
	m_pInventoryUI = NULL;
	m_pPartyWindow = NULL;
	m_pTradeWindow = NULL;	
	m_pPtoPWindow = NULL;
	m_pDamageDisplayMan = NULL;
	m_pTargetInfoDisplay = NULL;
	m_pTargetInfoDisplayNpc = NULL;
	m_pTargetInfoDisplayPlayer = NULL;
	m_pNameDisplayMan = NULL;
	m_pDialogueWindow = NULL;
	m_pMarketWindow = NULL;
	m_pStorageWindow = NULL;
	m_pItemMove = NULL;
	m_pBlockProgramAlarm = NULL;
	//m_pItemBankWindow = NULL;
	m_pVNGainSysInventory = NULL;
	//m_pTradeInventoryWindow = NULL;
	m_pInventoryUI_Trade = NULL;
	m_pHeadChatDisplayMan = NULL;	
	m_pAdminMessageDisplay = NULL;	
	m_pConftModalWindow = NULL;	
	m_pPartyModalWindow = NULL;
	m_pConftDisplayMan = NULL;
	m_pInfoDisplay = NULL;
	m_pMapMoveDisplay = NULL;
	m_pWaitServerDisplay = NULL;
	m_pAcademyConftDisplay = NULL;
	m_pLargeMapWindow = NULL;
	m_pLargeMapWindowImage = NULL;
	m_pMobPreviewWindow = NULL;
	m_pStorageChargeCard = NULL;
	m_pRebirthDialogue = NULL;
	m_pRebirthWindow = NULL;
	m_pPrivateMarketWindow = NULL;
	m_pPrivateMarketMake = NULL;
	m_pPrivateMarketSellWindow = NULL;
	m_pPrivateMarketShowMan = NULL;
	m_pClubWindow = NULL;
	m_pSubMasterSet = NULL;
	m_pClubMake = NULL;
	m_pSystemMessageWindow = NULL;
	m_pQuestionItemDisplay = NULL;
	m_pInfoDisplayEx = NULL;
	m_pClubStorageWindow = NULL;
	m_pConftConfirm = NULL;
	m_pBasicInfoView = NULL;
	m_pUILeftTopGroup = NULL;
	m_pSkillTrayTab = NULL;
	m_pBasicQuickSkillSlot = NULL;
	m_pCountMsgWindow = NULL;
	m_pGameMenu = NULL;
	m_pQBoxButton = NULL;
	m_pItemRebuildWindow = NULL;	// ITEMREBUILD_MARK
	//m_pRebuildInventoryWindow = NULL;
	m_pInventoryUI_Rebuild = NULL;
	m_pPetRebirthDialogue = NULL; // monster7j
	m_pGambleBox = NULL;
	m_pGambleSelectBox = NULL;
	m_pGambleAgainBox = NULL;
	m_pGambleResultOddBox = NULL;
	m_pGambleResultEvenBox = NULL;
	m_pPetWindow = NULL;
	m_pCharacterWindow = NULL;

	m_pModalWindow = NULL;

	m_pSystemMessageWindowDummy = NULL;
	m_pMapMoveDisplayDummy = NULL;
	m_pBlockProgramAlarmDummy = NULL;
	m_pLeftTopGroupDummy = NULL;
	m_pBasicInfoViewDummy = NULL;
	m_pBasicPotionTrayDummy = NULL;
	m_pQuestAlarmDummy = NULL;

	m_pKeySettingWindow = NULL;
	m_pChatMacroWindow = NULL;

	m_pBonusTimeGauge = NULL;
	m_pBonusTimeDisplay = NULL;
	m_pQuestHelper = NULL;
	m_pVehicleWindow = NULL;
	m_pThaiCCafeMark = NULL;

	m_pItemGarbageWindow = NULL;
	//m_pGarbageInventoryWindow = NULL;
	m_pInventoryUI_Trash = NULL;

	m_pItemShopIconMan = NULL;
	m_pNameDisplayMan = NULL;

	m_pShopItemSearchWindow   = NULL;
	m_pItemSearchResultWindow = NULL;

	m_pAttendanceBookWindow = NULL;
	m_pClubBattleModalWindow = NULL;

	m_pMapRequireCheckWindow = NULL;
	m_pTaxiWindow = NULL;

	m_bFirstVNGainSysCall = true;	

	m_fItemBankDelay = 5.0f;
	m_fItemShopDelay = 5.0f;

	m_pItemMixWindow = NULL;
	//m_pItemMixInvenWindow = NULL;
	m_pInventoryUI_ItemMix = NULL;
	m_pGatherGauge = NULL;
	m_pCdmRankingDisplay = NULL;
	/*crow target info*/
	m_pCrowTargetInfo = NULL; 
	m_pCrowTargetInfoNpc = NULL; 
	m_pCrowTargetInfoPlayer = NULL;
	

	/*game stats, Juver, 2017/06/22 */
	m_pGameStats = NULL; 
	m_pPKRankNotification = NULL;

	 /*npc shop, Juver, 2017/07/25 */
	m_pNPCShopWindow = NULL;

	/*item preview, Juver, 2017/07/27 */
	m_pItemPreviewWindow = NULL; 

	/*pet status, Juver, 2017/07/30 */
	m_pPetStatus = NULL; 

	/*vehicle status, Juver, 2017/07/30 */
	m_pVehicleStatus = NULL; 

	/*item link, Juver, 2017/07/31 */
	m_pInfoDisplayItemLink = NULL; 

	/*vehicle booster system, Juver, 2017/08/11 */
	m_pVehicleBoosterDisplay = NULL; 

	/*box contents, Juver, 2017/08/30 */
	m_pInfoBoxContents = NULL;

	/* Competition UI, Juver, 2017/08/24 */
	m_pCompetitionNotifyButton = NULL;
	m_pCompetitionWindow = NULL;

	/* extra notice, Juver, 2017/08/24 */
	m_pExtraNotice = NULL;

	/*pvp tyranny, Juver, 2017/08/25 */
	m_pPVPTyrannyTowerCapture = NULL;
	m_pPVPTyrannyTowerCaptureDummy = NULL;
	m_pPVPTyrannyTowerProgress = NULL;
	m_pRebirthDialogueTyranny = NULL;
	m_pPVPTyranny2CaptureOwn = NULL;
	m_pPVPTyrannyRankings = NULL;
	m_pPVPTyrannyTopRankings = NULL;
	m_pPVPTyrannyBattleNoticeStart = NULL;
	m_pPVPTyrannyBattleNoticeEnd = NULL;

	m_pSecurityWindow = NULL;

    m_pItemInforTooltip = NULL;
    m_pItemInforTooltipWear = NULL;
	m_pItemInforTooltipLink = NULL;

    m_pSkillTooltip = NULL;
	m_pSkillTooltipSubSkill = NULL;
	m_pSkillTooltipLinkSkill = NULL;

	for( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i )
	{
		m_pPVPTyrannyNoticeCapture[i] = NULL;
		m_pPVPTyranny2Capture[i] = NULL;
	}

	/*item exchange, Juver, 2017/10/12 */
	m_pNPCItemExchangeWindow = NULL;

	/*product item, Juver, 2017/10/15 */
	m_pProductButton = NULL;
	m_pProductWindow = NULL;

	/*student record ui, Juver, 2017/10/30 */
	m_pStudentRecordButton = NULL;
	m_pStudentRecordWindow = NULL;

	/*activity system, Juver, 2017/11/05 */
	m_pActivityItemWindow = NULL;

	/*quest ui, Juver, 2017/11/06 */
	m_pModernQuestWindow = NULL;

	/*charinfoview , Juver, 2017/11/11 */
	m_pCharacterInfoViewWindow = NULL;

	/*bike color , Juver, 2017/11/12 */
	m_pBikeColorWindow = NULL;

	/*pet skill info display, Juver, 2017/12/01 */
	m_pPetSkillInfoDisplay = NULL;
	m_pPetSkillInfoDisplay2 = NULL;

	/*change scale card, Juver, 2018/01/03 */
	m_pChangeScaleWindow = NULL;

	/*item color, Juver, 2018/01/10 */
	m_pItemColorWindow = NULL;

	/*change school card, Juver, 2018/01/12 */
	m_pChangeSchoolWindow = NULL;

	/*equipment lock, Juver, 2018/01/13 */
	m_pEquipmentLockEnableWindow = NULL;
	m_pEquipmentLockInputWindow = NULL;
	m_pEquipmentLockRecoverWindow = NULL;

	/*storage lock, Juver, 2018/01/13 */
	m_pStorageLockEnableWindow = NULL;
	m_pStorageLockInputWindow = NULL;
	m_pStorageLockRecoverWindow = NULL;

	/*inventory lock, Juver, 2018/01/13 */
	m_pInventoryLockEnableWindow = NULL;
	m_pInventoryLockInputWindow = NULL;
	m_pInventoryLockRecoverWindow = NULL;

	/*item transfer card, Juver, 2018/01/18 */
	m_pItemTransferInvenWindow = NULL;
	m_pItemTransferWindow = NULL;

	/*school wars, Juver, 2018/01/19 */
	m_pPVPSchoolWarsScore = NULL;
	m_pPVPSchoolWarsScoreDummy = NULL;
	m_pRebirthDialogueSchoolWars = NULL;
	m_pPVPSchoolWarsTopRankings = NULL;
	m_pPVPSchoolWarsBattleNoticeStart = NULL;
	m_pPVPSchoolWarsBattleNoticeEnd = NULL;
	m_pPVPSchoolWarsRankings = NULL;

	/*pvp capture the flag, Juver, 2018/02/01 */
	m_pRebirthDialogueCaptureTheFlag = NULL;
	m_pPVPCaptureTheFlagTopRankings = NULL;
	m_pPVPCaptureTheFlagBattleNoticeStart = NULL;
	m_pPVPCaptureTheFlagBattleNoticeEnd = NULL;
	m_pPVPCaptureTheFlagScore = NULL;
	m_pPVPCaptureTheFlagRankings = NULL;
	m_pPVPCaptureTheFlagHoldIcon = NULL;

	/* car, cart color, Juver, 2018/02/14 */
	m_pCarColorWindow = NULL;
	m_pPartyDisplay = NULL;

	/*dmk14 ingame web*/
	m_pItemMallWindow = NULL;
	m_fItemMallDelay = 5.0f;

	/*dmk14 remove active buff selection*/
	m_bFBuff = false;

	/*dmk14 itemshop new ui*/
	m_pItemShopWindow = NULL;

	/*PartySearch - Added By Bubu*/
	m_pPartyFinderWindow = NULL;
	m_pPartyReqModalWindow = NULL;
}

HRESULT CInnerInterface::OneTimeSceneInit ()
{
	switch ( RANPARAM::emSERVICE_TYPE )
	{
	case EMSERVICE_DEFAULT:
	case EMSERVICE_KOREA:
	case EMSERVICE_FEYA:
	case EMSERVICE_MALAYSIA_CN:
	case EMSERVICE_MALAYSIA_EN:
	case EMSERVICE_JAPAN:
	case EMSERVICE_THAILAND:
	case EMSERVICE_CHINA:
	case EMSERVICE_INDONESIA:
	case EMSERVICE_PHILIPPINES:
	case EMSERVICE_VIETNAM:
	case EMSERVICE_GLOBAL:
		m_bCHANNEL = true;
		break;

	default:
		m_bCHANNEL = false;
		break;
	};

	return S_OK;
}

HRESULT CInnerInterface::FrameMove ( LPDIRECT3DDEVICEQ pd3dDevice, float fTime, float fElapsedTime )
{
	GASSERT( pd3dDevice );

	// Ż�� ������ ����
	m_fVehicleDelay += fElapsedTime;

	//	������ �� ���̰� ����.
	//	���ÿ� Update()���� �Ѱ� ����.
	
    HideGroup ( ITEM_INFOR_TOOLTIP );
    HideGroup ( ITEM_INFOR_TOOLTIP_WEAR );
    HideGroup ( ITEM_INFOR_TOOLTIP_LINK );
    HideGroup ( SKILL_INFOR_TOOLTIP );
    HideGroup ( SKILL_INFOR_TOOLTIP_MULTI );
    HideGroup ( SKILL_INFOR_TOOLTIP_LINK );
	HideGroup ( INFO_DISPLAY );
	HideGroup ( INFO_DISPLAY_EX );
	HideGroup ( INFO_DISPLAY_EX_EX );
	HideGroup ( INFO_DISPLAY_ITEM_LINK ); /*item link, Juver, 2017/07/31 */
	HideGroup ( INFO_DISPLAY_ITEM_LINK_EX );

	/*pvp tyranny, Juver, 2017/08/25 */
	HideGroup ( PVP_TYRANNY_TOWER_PROGRESS ); 

	ResetTargetInfo ();
	ResetTargetInfoNpc ();			//add target info
	ResetTargetInfoPlayer ();		//add target info

	ResetTargetInfoCrow ();			//add target info
	ResetTargetInfoCrowNpc ();		//add target info
	ResetTargetInfoCrowPlayer ();	//add target info

	ResetTargetInfo ();		//	Ÿ�� ����â ����
	ResetCharMoveBlock ();	//	ĳ���� ������ ��� Ǯ��	
	ResetSnapItem ();		//	������ ���� ����
	ResetFirstItemSlot ();	//	�����۽��� �ٱ� ����

	m_bUSING_INFO_DISPLAY = false;
	m_bITEM_INFO_ITEM_LINK_DISPLAY_MODE = false; /*item link, Juver, 2017/07/31 */
	m_bITEM_INFO_ITEM_LINK_EX_DISPLAY_MODE = false;

	// ������ ��ũâ ������ ����
	/*
	if( !IsVisibleGroup( ITEMBANK_WINDOW ) && m_fItemBankDelay < 5.0f )
	{
		m_fItemBankDelay += fElapsedTime;
	}
	*/

	if( ( !IsVisibleGroup( INVENTORY_WINDOW ) && m_fItemBankDelay < 5.0f ) ||
		( m_pInventoryUI && m_pInventoryUI->GetActivePage() != ITEMBANK_PAGE && m_fItemBankDelay < 5.0f )	)
	{
		m_fItemBankDelay += fElapsedTime;
	}

	/*dmk14 ingame web*/
	if( !IsVisibleGroup( ITEMMALL_WINDOW ) && m_fItemMallDelay < 5.0f )
	{
		m_fItemMallDelay += fElapsedTime;
	}


	if ( RANPARAM::bPartyDisplay )
	{
		if ( m_pPartyDisplay && !m_pPartyDisplay->IsVisible() ) ShowGroupBottom ( PARTY_DISPLAY );
	}
	else if ( !RANPARAM::bPartyDisplay )
	{
		if ( m_pPartyDisplay && m_pPartyDisplay->IsVisible() ) HideGroup( PARTY_DISPLAY );
	}

	if ( RANPARAM::bPartyBuffDisplay )
	{
		if ( m_pPartyDisplay && m_pPartyDisplay->IsVisible() && !m_pPartyDisplay->IsBuffOK() ) m_pPartyDisplay->SetBuff();
	}
	else if ( !RANPARAM::bPartyBuffDisplay )
	{
		if ( m_pPartyDisplay && m_pPartyDisplay->IsVisible() && m_pPartyDisplay->IsBuffOK() ) m_pPartyDisplay->HideBuff();
	}


	bool bKEYBOARD_BLOCK = false;
	if( IsVisibleGroup ( MODAL_WINDOW ) ||
		IsVisibleGroup ( REBIRTH_WINDOW ) ||
		IsVisibleGroup ( REBIRTH_DIALOGUE_TYRANNY ) ||			/*pvp tyranny, Juver, 2017/08/25 */
		IsVisibleGroup ( REBIRTH_DIALOGUE_SCHOOLWARS ) ||		/*school wars, Juver, 2018/01/20 */
		IsVisibleGroup ( REBIRTH_DIALOGUE_CAPTURE_THE_FLAG ) || /*pvp capture the flag, Juver, 2018/02/01 */
		//IsVisibleGroup ( HELP_WINDOW ) ||	// �� ��������, ���� ��� ���� ( ���� )
		IsVisibleGroup ( ITEMSHOP_WINDOW ) ||
		DXInputString::GetInstance().IsOn () )
	{
		bKEYBOARD_BLOCK = true;
	}

	// WebBrowser�� �������� �������� �����Ƿ�
	// �ʿ��� ��Ȳ������ HELP_WINDOW�� �ݴ´� ( ���� )
	if( CCommonWeb::Get()->GetCreate() )
	{
		if( IsVisibleGroup ( MODAL_WINDOW ) ||
			IsVisibleGroup ( REBIRTH_WINDOW ) ||
			IsVisibleGroup ( REBIRTH_DIALOGUE_TYRANNY ) ||		/*pvp tyranny, Juver, 2017/08/25 */
			IsVisibleGroup ( REBIRTH_DIALOGUE_SCHOOLWARS ) ||	/*school wars, Juver, 2018/01/20 */
			IsVisibleGroup ( REBIRTH_DIALOGUE_CAPTURE_THE_FLAG ) || /*pvp capture the flag, Juver, 2018/02/01 */
			IsVisibleGroup( ESC_MENU ) )
		{
			HideGroup( HELP_WINDOW );
			HideGroup( ITEMSHOP_WINDOW );
			HideGroup( ITEMMALL_WINDOW );	/*dmk14 ingame web*/
		}
	}

	if ( !bKEYBOARD_BLOCK ) UpdateShortcutBefore ();

	/*item link, Juver, 2017/07/31 */
	if ( m_sItemInfoLinkTemp.sNativeID != NATIVEID_NULL() )
	{
		SHOW_INFO_ITEM_LINK( m_sItemInfoLinkTemp );
	}

	HRESULT hr = S_OK;
	hr = CUIMan::FrameMove ( pd3dDevice, fTime, fElapsedTime );
	if ( FAILED ( hr ) ) return hr;

	
			if ( IsVisibleGroup( SKILL_INFOR_TOOLTIP_MULTI ) ) 
			{
				if ( IsVisibleGroup( ITEM_INFOR_TOOLTIP ) ) 
				{
					UIRECT rcGlobalPos = m_pItemInforTooltip->GetGlobalPos();
					UIRECT rcGlobalPosEx = m_pSkillTooltipSubSkill->GetGlobalPos();

					float x = rcGlobalPos.right + 2.0f;
					float y = min(rcGlobalPos.top,rcGlobalPosEx.top);

					const long lResolution = uiman::GetInnerUIMan().GetResolution();
					WORD X_RES = HIWORD ( lResolution );
					WORD Y_RES = LOWORD ( lResolution );

					if ( X_RES < x + rcGlobalPosEx.sizeX )
					{
						x = X_RES - rcGlobalPosEx.sizeX;

						m_pItemInforTooltip->SetGlobalPos(D3DXVECTOR2(x-2.0f-rcGlobalPos.sizeX,y));
						m_pSkillTooltipSubSkill->SetGlobalPos(D3DXVECTOR2(x,y));
					}
					else
					{
						m_pItemInforTooltip->SetGlobalPos(D3DXVECTOR2(rcGlobalPos.left,y));
						m_pSkillTooltipSubSkill->SetGlobalPos(D3DXVECTOR2(x,y));
					}
				}
			}

			/*{
				CUIGroup* pTooltip = (CUIGroup*)uiman::GetInnerUIMan().FindControl(ITEM_INFOR_TOOLTIP);
				if ( pTooltip->IsVisible() == true )
				{
					UIRECT rcAllTooltip = pTooltip->GetGlobalPos();
					const long lResolution = uiman::GetInnerUIMan().GetResolution();
					INT nReleativePosX(rcAllTooltip.left)
						, X_RES(HIWORD ( lResolution ))
						, Y_RES(LOWORD ( lResolution ));

					INT uiidItemTooltip[3] = {
						// ITEM_INFOR_TOOLTIP,
						SKILL_INFOR_TOOLTIP_MULTI,				// ITEM_INFOR_TOOLTIP_LINK,
						ITEM_INFOR_TOOLTIP_DURABILITY,
						ITEM_INFOR_TOOLTIP_WEAR
					};

					D3DXVECTOR2 vArrTooltipPos[3];
					INT nPosY(rcAllTooltip.top);
					for ( INT i = 0; i < 3; ++i )
					{
						pTooltip = (CUIGroup*)uiman::GetInnerUIMan().FindControl(uiidItemTooltip[i]);
						if ( pTooltip->IsVisible() == false )
						{
							vArrTooltipPos[i] = D3DXVECTOR2(0.0f, 0.0f);
							continue;
						}

						UIRECT rcTooltip = pTooltip->GetGlobalPos();

						INT nGap(0), nAddWidth(rcAllTooltip.sizeX + rcTooltip.sizeX + 2);

						vArrTooltipPos[i] = D3DXVECTOR2(nReleativePosX + rcAllTooltip.sizeX + 2, rcTooltip.top);
						nPosY = min(nPosY, rcTooltip.top);
						if ( nReleativePosX + nAddWidth > X_RES )
						{
							nGap = X_RES - (nReleativePosX + nAddWidth);
						}

						if ( nReleativePosX + nGap < 0 )
						{
							nGap = -nReleativePosX;
						}

						nReleativePosX += nGap;

						for ( INT j = i; j >= 0; --j )
						{
							vArrTooltipPos[j].x += nGap;
						}

						rcAllTooltip.SetPosition(nReleativePosX, rcAllTooltip.top
							, nAddWidth
							, rcAllTooltip.sizeY);
					}

					UIRECT rcNormalInfo = m_pItemInforTooltip->GetGlobalPos();
					m_pItemInforTooltip->SetGlobalPos( D3DXVECTOR2(nReleativePosX, nPosY) );
					for ( INT i = 0; i < 3; ++i )
					{
						pTooltip = (CUIGroup*)uiman::GetInnerUIMan().FindControl(uiidItemTooltip[i]);
						if ( pTooltip->IsVisible() )
						{
							UIRECT rcTooltip = pTooltip->GetGlobalPos();
							pTooltip->SetGlobalPos( D3DXVECTOR2(vArrTooltipPos[i].x, nPosY) );
						}
					}
				}
			}*/

	if ( IsVisibleGroup( ITEM_INFOR_TOOLTIP_WEAR ) ) 
            {
                UIRECT rcGlobalPos = m_pItemInforTooltip->GetGlobalPos();
                UIRECT rcGlobalPosEx = m_pItemInforTooltipWear->GetGlobalPos();

                float x = rcGlobalPos.right + 2.0f;
                float y = min(rcGlobalPos.top,rcGlobalPosEx.top);

                const long lResolution = GetResolution();
                WORD X_RES = HIWORD ( lResolution );
                WORD Y_RES = LOWORD ( lResolution );

                if ( X_RES < x + rcGlobalPosEx.sizeX )
                {
                    x = X_RES - rcGlobalPosEx.sizeX;

                    m_pItemInforTooltip->SetGlobalPos(D3DXVECTOR2(x-2.0f-rcGlobalPos.sizeX,y));
                    m_pItemInforTooltipWear->SetGlobalPos(D3DXVECTOR2(x,y));
                }
                else
                {
                    m_pItemInforTooltip->SetGlobalPos(D3DXVECTOR2(rcGlobalPos.left,y));
                    m_pItemInforTooltipWear->SetGlobalPos(D3DXVECTOR2(x,y));
                }
            }

	/*item link, Juver, 2017/07/31 */
	if ( IsVisibleGroup ( INFO_DISPLAY_ITEM_LINK ) )
	{
		UIRECT rcGlobalPos1 = GetChat()->GetGlobalPos();
		UIRECT rcGlobalPos2 = m_pInfoDisplayItemLink->GetGlobalPos();

		float x = rcGlobalPos1.right + 60.0f;
		float y = rcGlobalPos1.bottom - rcGlobalPos2.sizeY;

		m_pInfoDisplayItemLink->SetGlobalPos(D3DXVECTOR2(x,y));

		if ( IsVisibleGroup ( INFO_DISPLAY_ITEM_LINK_EX ) )
		{
			UIRECT rcGlobalPos = m_pInfoDisplayItemLink->GetGlobalPos();
			UIRECT rcGlobalPosEx = m_pInfoDisplayItemLinkEx->GetGlobalPos();
			float x = rcGlobalPos.right + 2.0f;
			//float y = min(rcGlobalPos.top,rcGlobalPosEx.top);
			float y = rcGlobalPos.top;

			//const long lResolution = GetResolution ();
			//WORD X_RES = HIWORD ( lResolution );
			//WORD Y_RES = LOWORD ( lResolution );

			m_pInfoDisplayItemLinkEx->SetGlobalPos(D3DXVECTOR2(x,y));


			/*if ( X_RES < x + rcGlobalPosEx.sizeX )
			{
				x = X_RES - rcGlobalPosEx.sizeX;

				//m_pInfoDisplayLink->SetGlobalPos(D3DXVECTOR2(x-2.0f-rcGlobalPos.sizeX,y));
				m_pInfoDisplayLinkEx->SetGlobalPos(D3DXVECTOR2(x,y));
			}
			else
			{
				//m_pInfoDisplayLink->SetGlobalPos(D3DXVECTOR2(rcGlobalPos.left,y));
			}*/
		}
	}

	if( IsOpenWindowToMoveBlock() )
	{
		SetCharMoveBlock ();
	}

	UpdateStatus ();

	if ( !m_bUSING_INFO_DISPLAY ) RESET_INFO ();
	if ( !bKEYBOARD_BLOCK )	UpdateShortcutAfter ();

	if ( IsExclusiveControl() ) SetCharMoveBlock ();

	int LB = (int)DxInputDevice::GetInstance().GetMouseState(DXMOUSE_LEFT);
	if ( (LB & DXKEY_UP) || (LB & DXKEY_DUP) )
	{
		ResetExclusiveControl();
	}

	MoveBasicInfoWindow ();

	CUIFocusContainer::UICONTROL_FOCUSLIST focus_list = GetFocusList()->GetFocusList();
	CUIFocusContainer::UICONTROL_FOCUSLIST_RITER riter = focus_list.rbegin ();
	CUIFocusContainer::UICONTROL_FOCUSLIST_RITER riter_end = focus_list.rend ();

	bool bBasicChatBoxShow = false;
	for ( ; riter != riter_end; ++riter )
	{
		UIGUID cID = riter->cID;
		if ( BASIC_INFO_VIEW == cID )
		{
			if ( bBasicChatBoxShow )
			{
				if ( !GetChat()->IsCHAT_BEGIN() )
				{
					GetFocusList()->InsertAfter ( BASIC_CHAT_BOX, BASIC_INFO_VIEW, riter->pUIControl );
				}
			}
			break;
		}
		else if ( BASIC_CHAT_BOX == cID )
		{
			bBasicChatBoxShow = true;
		}
	}

	{
		GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter ();
		Actor& actor = pCharacter->GetActorMove ();
		if ( actor.PathIsActive () ) m_vCharDir = actor.Movement ();
	}

	{
		if ( IsVisibleGroup ( INFO_DISPLAY_EX ) )
		{
			UIRECT rcGlobalPos = m_pInfoDisplay->GetGlobalPos();
			UIRECT rcGlobalPosEx = m_pInfoDisplayEx->GetGlobalPos();
			float x = rcGlobalPos.right + 2.0f;
			float y = min(rcGlobalPos.top,rcGlobalPosEx.top);

			const long lResolution = GetResolution ();
			WORD X_RES = HIWORD ( lResolution );
			WORD Y_RES = LOWORD ( lResolution );

			if ( X_RES < x + rcGlobalPosEx.sizeX )
			{
				x = X_RES - rcGlobalPosEx.sizeX;

				m_pInfoDisplay->SetGlobalPos(D3DXVECTOR2(x-2.0f-rcGlobalPos.sizeX,y));
				m_pInfoDisplayEx->SetGlobalPos(D3DXVECTOR2(x,y));
			}
			else
			{
				m_pInfoDisplay->SetGlobalPos(D3DXVECTOR2(rcGlobalPos.left,y));
				m_pInfoDisplayEx->SetGlobalPos(D3DXVECTOR2(x,y));
			}

			if ( IsVisibleGroup ( INFO_DISPLAY_EX_EX ) )
			{
				UIRECT rcGlobalPos1 = m_pInfoDisplayEx->GetGlobalPos();
				UIRECT rcGlobalPos1Ex = m_pInfoDisplayExEx->GetGlobalPos();
				float x = rcGlobalPos1.left;
				//float y = rcGlobalPos1.bottom - rcGlobalPos1Ex.sizeY;
				float y = rcGlobalPos1.bottom + 5.0f;
				//float y = min(rcGlobalPos1.bottom,rcGlobalPos1Ex.bottom);

				m_pInfoDisplayExEx->SetGlobalPos(D3DXVECTOR2(x,y));

			}
		}
		else
		{
			if ( IsVisibleGroup ( INFO_DISPLAY_EX_EX ) )
			{
				UIRECT rcGlobalPos = m_pInfoDisplay->GetGlobalPos();
				UIRECT rcGlobalPosEx = m_pInfoDisplayExEx->GetGlobalPos();
				float x = rcGlobalPos.right + 2.0f;
				float y = min(rcGlobalPos.top,rcGlobalPosEx.top);

				const long lResolution = GetResolution ();
				WORD X_RES = HIWORD ( lResolution );
				WORD Y_RES = LOWORD ( lResolution );

				if ( X_RES < x + rcGlobalPosEx.sizeX )
				{
					x = X_RES - rcGlobalPosEx.sizeX;

					m_pInfoDisplay->SetGlobalPos(D3DXVECTOR2(x-2.0f-rcGlobalPos.sizeX,y));
					m_pInfoDisplayExEx->SetGlobalPos(D3DXVECTOR2(x,y));
				}
				else
				{
					m_pInfoDisplay->SetGlobalPos(D3DXVECTOR2(rcGlobalPos.left,y));
					m_pInfoDisplayExEx->SetGlobalPos(D3DXVECTOR2(x,y));
				}
			}
		}
	}
	{
		if( IsVisibleGroup( LARGEMAP_WINDOW ) )
		{
			UIRECT rcGlobalPos = GetMobPreviewWindow()->GetGlobalPos();
			UIRECT rcGlobalPosEx = GetLargeMapWindow()->GetGlobalPos();

			const long lResolution = GetResolution ();
			WORD X_RES = HIWORD ( lResolution );
			WORD Y_RES = LOWORD ( lResolution );

			float xPosHalf = rcGlobalPos.sizeX / 2;
			float xPosExHalf = rcGlobalPosEx.sizeX / 2;

			float xHalf = xPosHalf - xPosExHalf;

			float x = rcGlobalPosEx.left + rcGlobalPosEx.sizeX; //X_RES - rcGlobalPos.sizeX - 5.0f;
			float y = rcGlobalPosEx.top;

			m_pMobPreviewWindow->SetGlobalPos(D3DXVECTOR2(x,y));
		}
	}

	return hr;
}

void CInnerInterface::UpdateShortcutBefore ()
{	
	{
		if ( !m_pNameDisplayMan ) return ;		

		if ( UIKeyCheck::GetInstance()->CheckSimple ( DIK_LMENU, DXKEY_PRESSED ) )
		{
			for (int i= 0; i < CHAT_MACRO_SIZE; ++i){
				if ( UIKeyCheck::GetInstance()->CheckSimple ( DIK_1+i, DXKEY_DOWN ) )
				{			
					AddChatMacro(i);
					break;
				}
			}
		}



		if( RANPARAM::bNAME_DISPLAY )
		{
			if( IsVisibleGroup( NAME_DISPLAY_MAN ) )
			{
				if( m_pNameDisplayMan->GetNameType() != NAME_DISPLAY_MOBCHAR )
				{
					HideGroup( NAME_DISPLAY_MAN );
					m_pNameDisplayMan->SetNameType( NAME_DISPLAY_MOBCHAR );
					ShowGroupBottom( NAME_DISPLAY_MAN );
				}
			}
			else
			{
				m_pNameDisplayMan->SetNameType( NAME_DISPLAY_MOBCHAR );
				ShowGroupBottom( NAME_DISPLAY_MAN );
			}

			return ;
		}

		HideGroup ( NAME_DISPLAY_MAN );		
		if ( UIKeyCheck::GetInstance()->Check ( DIK_LMENU, DXKEY_PRESSED ) )
		{
			m_pNameDisplayMan->SetNameType ( NAME_DISPLAY_ITEMMONEY );
			ShowGroupBottom ( NAME_DISPLAY_MAN );
		}
		else if ( UIKeyCheck::GetInstance()->Check ( DIK_LCONTROL, DXKEY_PRESSED ) )
		{
			m_pNameDisplayMan->SetNameType ( NAME_DISPLAY_MOBCHAR );
			ShowGroupBottom ( NAME_DISPLAY_MAN );
		}
		else if ( UIKeyCheck::GetInstance()->Check ( DIK_LSHIFT, DXKEY_PRESSED ) )
		{
			m_pNameDisplayMan->SetNameType ( NAME_DISPLAY_ALL );
			ShowGroupBottom ( NAME_DISPLAY_MAN );
		}
		else
		{
			//	BEGIN : CLUB CONFT SETTING
			const EMCONFT_TYPE emTYPE = GLGaeaClient::GetInstance().GetCharacter()->GetConfting().emTYPE;
			if ( emTYPE == EMCONFT_GUILD )
			{
				m_pNameDisplayMan->SetNameType ( NAME_DISPLAY_CLUB_CONFT );
				ShowGroupBottom ( NAME_DISPLAY_MAN );                
				return ;
			}

			//	��Ƽ
			GLPARTY_CLIENT *pMaster = GLPartyClient::GetInstance().GetMaster();	//	������
			if ( pMaster )
			{
				GLCharacter *pMyCharacter = GLGaeaClient::GetInstance().GetCharacter();			

				if ( pMyCharacter->GetConfting().IsCONFRONTING() )	//	��Ƽ ��� ���ΰ�?
				{
					m_pNameDisplayMan->SetNameType ( NAME_DISPLAY_PARTY_CONFT );			
				}
				else	//	��Ƽ ���� �� ���
				{
					m_pNameDisplayMan->SetNameType ( NAME_DISPLAY_PARTY );		
				}
				ShowGroupBottom ( NAME_DISPLAY_MAN );
			}
			else
			{
				//	������ �Ѹ�
				m_pNameDisplayMan->SetNameType ( NAME_DISPLAY_DEFAULT );
				ShowGroupBottom ( NAME_DISPLAY_MAN );
			}
		}
	}	
}

void CInnerInterface::UpdateShortcutAfter ()
{
	if ( UIKeyCheck::GetInstance()->Check ( DIK_ESCAPE, DXKEY_DOWN ) )
	{
		bool bALLHIDE = true;

		//	NOTE
		//		���ÿ� �� ������ FocusList��
		//		������ ���ɼ��� ���ٴ� ������ �־�� �Ѵ�.
		//		�����ϱ� ��.��
		CUIFocusContainer::UICONTROL_FOCUSLIST list = GetFocusList()->GetFocusList ();
		CUIFocusContainer::UICONTROL_FOCUSLIST_RITER riter = list.rbegin ();
		CUIFocusContainer::UICONTROL_FOCUSLIST_RITER riter_end = list.rend ();
		for ( ; riter != riter_end; ++riter )
		{
			bool bAnotherProcess = false;
			UIGUID cID = riter->cID;

			/*pvp tyranny, Juver, 2017/08/25 */
			/*school wars, Juver, 2018/01/20 */
			/*pvp capture the flag, Juver, 2018/02/02 */
			if ( IsVisibleGroup ( cID ) && 
				cID != BASIC_CHAT_BOX && 
				cID != BASIC_INFO_VIEW &&
				cID != PRIVATE_MARKET_SHOW_MAN && 
				cID != MINIMAP &&
				cID != PVP_TYRANNY_RANKINGS &&
				cID != PVP_SCHOOLWARS_RANKINGS &&
				cID != PVP_CAPTURE_THE_FLAG_RANKINGS &&
				/////////////////////////////////////////////////////////////////////////////
				// LG-7 CWRanking
				cID != CW_RANKING_DISPLAY )
				/////////////////////////////////////////////////////////////////////////////
			{
				if ( cID == INVENTORY_WINDOW || cID == MARKET_WINDOW ||
					cID == STORAGE_WINDOW || cID == CLUB_STORAGE_WINDOW || cID == ITEMMALL_WINDOW )
				{
					SetMarketWindowClose ();
					SetStorageWindowClose ();
					SetClubStorageWindowClose ();
					/*dmk14 ingame web*/
					SetItemMallWindowClose();
				}
				else if ( cID == TRADEINVENTORY_WINDOW )
				{
					if ( IsVisibleGroup ( TRADE_WINDOW ) )
					{
						SetTradeWindowCloseReq ();
					}
					else if ( IsVisibleGroup ( PRIVATE_MARKET_WINDOW ) )
					{
						if ( m_pPrivateMarketWindow->IsOPENER () )
						{
							DoModal ( ID2GAMEINTEXT("PRIVATE_MARKET_QUIT"), MODAL_QUESTION, YESNO, MODAL_PRIVATE_MARKET_QUIT );
						}
						else
						{
							SetPrivateMarketClose ();
						}
						bAnotherProcess = true;
					}
				}
				else if ( cID == TRADE_WINDOW )
				{							
					SetTradeWindowCloseReq ();
				}
				else if ( cID == MINIPARTY_WINDOW )
				{
					ShowGroupBottom ( MINIPARTY_OPEN );
// #ifdef CH_PARAM // �߱� �������̽� ����
//					UpdatePotionTrayPosition();
// #endif
				}
				else if ( cID == PRIVATE_MARKET_WINDOW )
				{
					if ( m_pPrivateMarketWindow->IsOPENER () )
					{
						DoModal ( ID2GAMEINTEXT("PRIVATE_MARKET_QUIT"), MODAL_QUESTION, YESNO, MODAL_PRIVATE_MARKET_QUIT );
					}
					else
					{
						SetPrivateMarketClose ();
					}
					bAnotherProcess = true;					
				}
				else if( cID == ITEM_REBUILD_WINDOW || cID == REBUILDINVENTORY_WINDOW )	// ITEMREBUILD_MARK
				{
					CloseItemRebuildWindow();
				}
				else if( cID == ITEM_GARBAGE_WINDOW || cID == GARBAGEINVENTORY_WINDOW )	// ������
				{
					CloseItemGarbageWindow();
				}
				else if( cID == ITEM_MIX_WINDOW || cID == ITEM_MIX_INVEN_WINDOW )	// ������ ����
				{
					CloseItemMixWindow();
				}
				else if( cID == GAMBLE_DIALOGUE || cID == GAMBLE_SELECT_DIALOGUE || 
						cID == GAMBLE_AGAIN_DIALOGUE || cID == GAMBLE_ANIMATION_DIALOGUE || 
						cID == GAMBLE_RESULT_EVEN_DIALOGUE || cID == GAMBLE_RESULT_ODD_DIALOGUE  )
				{
					HideGroup( INVENTORY_WINDOW );
					GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
					if( pCharacter )
					{
						pCharacter->ReqMGameOddEvenCancel();
					}
				}
				else if ( cID == QUEST_WINDOW )
				{
					int nPage = m_pQuestWindow->GET_SHOW_PAGE ();
					switch ( nPage )
					{
					case CQuestWindow::QUEST_STEP:
						{							
							m_pQuestWindow->SET_SHOW_PAGE ( CQuestWindow::QUEST_MAIN );
							bAnotherProcess = true;
						}
						break;

					case CQuestWindow::QUEST_MAIN:
						{												
							m_pQuestWindow->SET_SHOW_PAGE ( CQuestWindow::QUEST_LIST );
							bAnotherProcess = true;
						}
						break;
					}					
				}
				/*pvp tyranny, Juver, 2017/08/25 */
				/*school wars, Juver, 2018/01/20 */
				/*pvp capture the flag, Juver, 2018/02/01 */
				else if ( cID == WAITSERVER_DISPLAY || cID == REBIRTH_WINDOW || cID == MAP_REQUIRE_CHECK ||
					cID == REBIRTH_DIALOGUE_TYRANNY || cID == REBIRTH_DIALOGUE_SCHOOLWARS || cID == REBIRTH_DIALOGUE_CAPTURE_THE_FLAG )
				{
					bAnotherProcess = true;
				}
				/*item transfer card, Juver, 2018/01/18 */
				else if( cID == ITEM_TRANSFER_WINDOW || cID == ITEM_TRANSFER_INVEN_WINDOW )
				{
					CloseItemTransferWindow();
				}
				/////////////////////////////////////////////////////////////////////////////
				// LG-7 GlobalRanking
				else if ( cID == GLOBAL_RANKING_WINDOW )	SetGlobalRankingWindowOpen(FALSE);
				/////////////////////////////////////////////////////////////////////////////
				else if ( (MODAL_WINDOW + 1) <= cID )
				{
					//					CLOSE_MODAL ( cID, false );
					bAnotherProcess = true;
				}
				

				if ( !bAnotherProcess )	
				{
					HideGroup ( cID );
				}

				bALLHIDE = false;
				break;
			}
		}

		/*item link, Juver, 2017/07/31 */
		if ( bALLHIDE )
		{
			if( IsVisibleGroup( ITEM_INFOR_TOOLTIP_LINK ) )
			{
				CLEAR_INFO_ITEM_LINK ();
				bALLHIDE = FALSE;
			}
		}

		if ( bALLHIDE )
		{
			if ( !IsVisibleGroup ( ESC_MENU ) )
				ShowGroupFocus ( ESC_MENU );
		}
	}

	// Tab Key �������̶��
	if ( m_bTabReserve ) 
	{
		if ( !GLGaeaClient::GetInstance().GetCharacter()->IsACTION(GLAT_SKILL) )
		{
			GLGaeaClient::GetInstance().GetCharacter()->ReqSlotChange();
			m_bTabReserve = false;
		}
	}	
	else 
	{
		if ( UIKeyCheck::GetInstance()->Check( DIK_TAB, DXKEY_DOWN ) )
		{
			EMCHARCLASS emClass = GLGaeaClient::GetInstance().GetCharacter()->m_emClass;

			if( emClass&GLCC_EXTREME_M || emClass&GLCC_EXTREME_W )
			{
				// ��ų ��������� �����ϸ� Tab Key ����
				if ( E_FAIL == GLGaeaClient::GetInstance().GetCharacter()->ReqSlotChange() )
					m_bTabReserve = true;
			}
				
		}
	}



	if ( IsVisibleGroup ( ESC_MENU ) )				return ;
	if ( IsVisibleGroup ( HELP_WINDOW ) )			return ;
	if ( IsVisibleGroup ( ITEMSHOP_WINDOW ) )		return ;
	if ( IsVisibleGroup ( ITEMMALL_WINDOW ) )		return ;	/*dmk14 ingame web*/
	if ( IsVisibleGroup ( KEY_SETTING_WINDOW ) )	return ;

	if ( UIKeyCheck::GetInstance()->CheckSimple ( DIK_V, DXKEY_DOWN ) )
	{
		if ( GLTradeClient::GetInstance().Valid() ) return;
		GLCharacter* const pCharacter = GLGaeaClient::GetInstance().GetCharacter ();

		if ( pCharacter->m_sVehicle.IsActiveValue() && m_fVehicleDelay < 1.0f )	
		{
			PrintMsgText ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("VEHICLE_SET_FB_DELAY") );
			return;
		}

		pCharacter->ReqSetVehicle( !pCharacter->m_bVehicle ); // Ȱ�� ��Ȱ��
		m_fVehicleDelay = 0.0f;
	}

	/*vehicle booster system, Juver, 2017/08/11 */
	if ( UIKeyCheck::GetInstance()->CheckSimple ( DIK_Z, DXKEY_DOWN ) )
	{
		GLGaeaClient::GetInstance().GetCharacter ()->VehicleBoosterStart();
	}

	if ( UIKeyCheck::GetInstance()->CheckSimple ( DIK_LMENU, DXKEY_PRESSED ) )
	{
		for (int i= 0; i < CHAT_MACRO_SIZE; ++i){
			if ( UIKeyCheck::GetInstance()->CheckSimple ( DIK_1+i, DXKEY_DOWN ) )
			{			
				return ;
			}
		}
	}

	GLCharacter* const pCharacter = GLGaeaClient::GetInstance().GetCharacter ();
	if ( pCharacter )
	{
		//	�� ��ų
		for( int i = 0; i < QUICK_SKILL_SLOT_MAX; ++i)
		{
            if ( UIKeyCheck::GetInstance()->Check ( RANPARAM::SkillSlot[i], DXKEY_DOWN ) )
			{
				INT nIndex = m_pSkillTrayTab->GetTabIndex() * QUICK_SKILL_SLOT_MAX + i;
				pCharacter->ReqSkillRunSet( nIndex );
			}
		}
	}
	
	if ( UIKeyCheck::GetInstance()->Check ( RANPARAM::MenuShotcut[SHOTCUT_INVEN], DXKEY_DOWN ) )
	{		
		/*
		if ( !IsVisibleGroup ( INVENTORY_WINDOW ) && !IsVisibleGroup ( TRADEINVENTORY_WINDOW ) )
		{
			ShowGroupFocus( INVENTORY_WINDOW );
		}
		else
		{
			HideGroup ( INVENTORY_WINDOW );
		}
		*/

		OpenInventory_Inven();
	}

	if ( UIKeyCheck::GetInstance()->Check ( RANPARAM::MenuShotcut[SHOTCUT_SKILL], DXKEY_DOWN ) )
	{		
		if ( !IsVisibleGroup ( SKILL_WINDOW ) )
			ShowGroupFocus ( SKILL_WINDOW );
		else
			HideGroup ( SKILL_WINDOW );
	}

	if ( UIKeyCheck::GetInstance()->Check ( RANPARAM::MenuShotcut[SHOTCUT_PARTY], DXKEY_DOWN ) )
	{		
		if ( !IsVisibleGroup ( PARTY_WINDOW ) )
			ShowGroupFocus ( PARTY_WINDOW );
		else
			HideGroup ( PARTY_WINDOW );
	}

	if ( UIKeyCheck::GetInstance()->Check ( RANPARAM::MenuShotcut[SHOTCUT_CHAR], DXKEY_DOWN ) )
	{		
		if ( !IsVisibleGroup ( CHARACTER_WINDOW ) )
			ShowGroupFocus ( CHARACTER_WINDOW );
		else
			HideGroup ( CHARACTER_WINDOW );
	}

	if ( UIKeyCheck::GetInstance()->CheckSimple ( RANPARAM::MenuShotcut[SHOTCUT_MAP], DXKEY_DOWN ) )
	{
		if ( !IsVisibleGroup ( LARGEMAP_WINDOW ) )
		{
			ShowGroupFocus ( LARGEMAP_WINDOW );
			/*dmk14 large map window ep9*/
			CInnerInterface::GetInstance().GetLargeMapWindow()->SetLargeMap( GLGaeaClient::GetInstance().GetActiveMapID(), pCharacter );
		}
		else
		{
			HideGroup ( LARGEMAP_WINDOW );
			HideGroup ( MOBPREVIEW_WINDOW );
		}
			
	}

	if ( UIKeyCheck::GetInstance()->CheckSimple ( RANPARAM::MenuShotcut[SHOTCUT_CLUB], DXKEY_DOWN ) )
	{
		if ( !IsVisibleGroup ( CLUB_WINDOW ) )
			ShowGroupFocus ( CLUB_WINDOW );
		else
			HideGroup ( CLUB_WINDOW );
	}

	if ( UIKeyCheck::GetInstance()->Check ( RANPARAM::MenuShotcut[SHOTCUT_QUEST], DXKEY_DOWN ) )
	{
		/*quest ui, Juver, 2017/11/06 */
		UIGUID questui_id = QUEST_WINDOW;
		if ( RANPARAM::bFeatureModernQuestWindow )
			questui_id = MODERN_QUEST_WINDOW;

		if ( !IsVisibleGroup ( questui_id ) )
		{			
			if ( NATIVEID_NULL().dwID == m_dwEventQuestID )
			{
				ShowGroupFocus ( questui_id );
				REFRESH_QUEST_WINDOW ();				
			}
			else
			{
				SetQuestWindowOpen ( m_dwEventQuestID );
			}
		}
		else
		{
			HideGroup ( questui_id );
		}	
	}

	if ( UIKeyCheck::GetInstance()->Check ( RANPARAM::MenuShotcut[SHOTCUT_HELP], DXKEY_DOWN) )
	{
		if ( !IsVisibleGroup ( HELP_WINDOW ) )
		{
			HideGroup ( ITEMSHOP_WINDOW );
			ShowGroupFocus ( HELP_WINDOW );
		}
		else
		{
			HideGroup ( HELP_WINDOW );
		}
	}

#if defined ( CH_PARAM ) || defined ( TH_PARAM ) || defined( PH_PARAM ) || defined ( JP_PARAM ) || defined( _RELEASED ) // �ʸ��� �����ۼ�
	if ( UIKeyCheck::GetInstance()->Check ( RANPARAM::MenuShotcut[SHOTCUT_ITEMSHOP], DXKEY_DOWN) )
	{
		if ( !IsVisibleGroup ( ITEMSHOP_WINDOW ) )
		{
			//	ItemShopAuth
			if ( ItemShopAuth() )
			{
				ShowGroupFocus ( ITEMSHOP_WINDOW );
			}

			HideGroup ( HELP_WINDOW );
		}
		else
		{
			HideGroup ( ITEMSHOP_WINDOW );
		}
	}
	
	//	ItemShopAuth	
	ItemShopVisible();

#endif	

	if ( UIKeyCheck::GetInstance()->Check ( RANPARAM::MenuShotcut[SHOTCUT_FRIEND], DXKEY_DOWN ) )
	{
		if ( IsVisibleGroup ( FRIEND_WINDOW ) )		HideGroup ( FRIEND_WINDOW );			
		else										ShowGroupFocus ( FRIEND_WINDOW );
	}

	if ( UIKeyCheck::GetInstance()->Check ( RANPARAM::MenuShotcut[SHOTCUT_ITEMBANK], DXKEY_DOWN ) )
	{
		//if ( IsVisibleGroup ( ITEMBANK_WINDOW ) )	HideGroup ( ITEMBANK_WINDOW );
		//else										SetItemBankWindowOpen ();

		/*if ( !IsVisibleGroup ( ITEMBANK_WINDOW ) )	
		{
			SetItemBankWindowOpen();			
		}*/

		OpenInventory_Bank();
	}

	/*if ( UIKeyCheck::GetInstance()->Check ( RANPARAM::MenuShotcut[SHOTCUT_VIETNAMGAINSYS], DXKEY_DOWN ) )
	{
		if ( IsVisibleGroup ( VNGAINSYS_WINDOW ) )	HideGroup ( VNGAINSYS_WINDOW );
		else										SetVNGainSysWindowOpen();
	}*/

	if ( UIKeyCheck::GetInstance()->Check ( RANPARAM::MenuShotcut[SHOTCUT_CHATMACRO], DXKEY_DOWN ) )
	{
		if ( IsVisibleGroup ( CHATMACRO_WINDOW ) )
		{
			HideGroup ( CHATMACRO_WINDOW );
		}
		else
		{
			GetChatMacro();
			ShowGroupFocus ( CHATMACRO_WINDOW );
		}
	}



	if ( UIKeyCheck::GetInstance()->CheckSimple ( DIK_LMENU, DXKEY_PRESSED ) )
	{
		if ( UIKeyCheck::GetInstance()->CheckSimple ( DIK_F4, DXKEY_DOWN ) )
		{			
			DoModal ( ID2GAMEINTEXT("CLOSE_GAME"), MODAL_QUESTION, OKCANCEL, MODAL_CLOSEGAME );
		}
	}

//#ifndef CH_PARAM // �߱� �������̽� ����
	if( UIKeyCheck::GetInstance()->Check ( DIK_GRAVE, DXKEY_DOWN ) )
	{
		if ( IsVisibleGroup ( QUICK_SKILL_TRAY_OPEN_BUTTON ) )
		{
			HideGroup ( QUICK_SKILL_TRAY_OPEN_BUTTON );
			ShowGroupBottom ( QUICK_SKILL_TRAY_TAB_WINDOW );
		}
		else
		{
			HideGroup ( QUICK_SKILL_TRAY_TAB_WINDOW );
			ShowGroupBottom ( QUICK_SKILL_TRAY_OPEN_BUTTON );
		}
	}

	/*Codex System, Jhoniex 2/14/2024 */
	if	( UIKeyCheck::GetInstance()->Check ( DIK_O, DXKEY_DOWN ) )
	{
		if ( !IsVisibleGroup( STUDENT_CODEX_WINDOW ))
		{
		ShowGroupFocus( STUDENT_CODEX_WINDOW );
		//m_pStudentRecordWindow->LoadData();
		}
		else
		{
		HideGroup ( STUDENT_CODEX_WINDOW );
		}
	}
//#endif

	if( UIKeyCheck::GetInstance()->Check( DIK_F1, DXKEY_DOWN ) )	
		m_pSkillTrayTab->SetTabIndex( 0 );
	if( UIKeyCheck::GetInstance()->Check( DIK_F2, DXKEY_DOWN ) )	
		m_pSkillTrayTab->SetTabIndex( 1 );
	if( UIKeyCheck::GetInstance()->Check( DIK_F3, DXKEY_DOWN ) )	
		m_pSkillTrayTab->SetTabIndex( 2 );

	if( UIKeyCheck::GetInstance()->Check( RANPARAM::MenuShotcut[SHOTCUT_PET], DXKEY_DOWN ) )
	{
		if( GLGaeaClient::GetInstance().GetPetClient()->IsVALID() )
		{
			if ( GLTradeClient::GetInstance().Valid() ) return;
			
			if ( IsVisibleGroup( PET_WINDOW ) )		HideGroup ( PET_WINDOW );
			else									ShowGroupFocus( PET_WINDOW );

			
		}
	}


	if( UIKeyCheck::GetInstance()->Check( DIK_N, DXKEY_DOWN ) )
	{
		if( GLGaeaClient::GetInstance().GetCharacter()->m_sVehicle.IsActiveValue() )
		{
			if ( GLTradeClient::GetInstance().Valid() ) return;
			
			if ( IsVisibleGroup( VEHICLE_WINDOW ) )	HideGroup ( VEHICLE_WINDOW );
			else									ShowGroupFocus( VEHICLE_WINDOW );

			
		}
	}

	if ( UIKeyCheck::GetInstance()->Check ( DIK_Y, DXKEY_DOWN ) )
	{
		if ( RANPARAM::bFeatureStudentRecord )
		{
			if ( IsVisibleGroup(STUDENT_RECORD_WINDOW) )
				HideGroup( STUDENT_RECORD_WINDOW );
			else	ShowGroupFocus( STUDENT_RECORD_WINDOW );
		}
	}

	/*character simple, Juver, 2017/10/01 */
	if( UIKeyCheck::GetInstance()->Check( DIK_F9, DXKEY_DOWN ) )	
	{
		RANPARAM::bCharacterSimple = !RANPARAM::bCharacterSimple;
		RANPARAM::bCharacterMisc = !RANPARAM::bCharacterMisc;
		GLGaeaClient::GetInstance().SetCharacterSimple( RANPARAM::bCharacterSimple, RANPARAM::bCharacterMisc );

		if ( RANPARAM::bCharacterSimple && RANPARAM::bCharacterMisc )
			PrintMsgText ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CHARACTER_SIMPLE_ON") );
		else
			PrintMsgText ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CHARACTER_SIMPLE_OFF") );
	}
	
	if( UIKeyCheck::GetInstance()->Check( DIK_F7, DXKEY_DOWN ) )	
	{
		RANPARAM::bSpecialName = !RANPARAM::bSpecialName;
		if ( RANPARAM::bSpecialName )
			PrintMsgText ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("SPECIAL_NAME_ON") );
		else
			PrintMsgText ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("SPECIAL_NAME_OFF") );
	}



	/*skill effect setting, Juver, 2017/10/01 */
	if( UIKeyCheck::GetInstance()->Check( DIK_F10, DXKEY_DOWN ) )	
	{
		RANPARAM::bHideSkillEffect = !RANPARAM::bHideSkillEffect;

		if ( RANPARAM::bHideSkillEffect )
			PrintMsgText ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("HIDE_SKILL_EFFECT_ON") );
		else
			PrintMsgText ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("HIDE_SKILL_EFFECT_OFF") );
	}
}

void CInnerInterface::UpdateStatus ()
{
	GLCharacter* const pCharacter = GLGaeaClient::GetInstance().GetCharacter();

	if ( IsVisibleGroup ( SYSTEM_MESSAGE_WINDOW ) )
	{
		UIRECT rcGlobalPos = m_pSystemMessageWindowDummy->GetGlobalPos ();
		//rcGlobalPos.top += fMENU_LIFTUP;
		m_pSystemMessageWindow->SetGlobalPos ( rcGlobalPos );
	}

	if ( IsBlockProgramAlarm () )
	{		
		if ( !IsVisibleGroup ( BLOCK_PROGRAM_ALARM ) )
		{
			UIRECT rcGlobalPos = m_pBlockProgramAlarmDummy->GetGlobalPos ();
			rcGlobalPos.top += float((rand()%200) - 100);
			rcGlobalPos.left += float((rand()%200) - 100);
			m_pBlockProgramAlarm->SetGlobalPos ( rcGlobalPos );

			GetPositionKeeper()->SetPosition ( BLOCK_PROGRAM_ALARM, rcGlobalPos );

			ShowGroupFocus ( BLOCK_PROGRAM_ALARM );
		}
	}
	else
	{
		HideGroup ( BLOCK_PROGRAM_ALARM );
	}
	
	
	//	ĳ���� ���� ���� Ȯ��
	if ( pCharacter->IsDie () )
	{
		bool bPVPTyranny = false;			/*pvp tyranny, Juver, 2017/08/25 */
		bool bPVPSchoolWars = false;		/*school wars, Juver, 2018/01/19 */
		bool bPVCaptureTheFlag = false;		/*pvp capture the flag, Juver, 2018/01/31 */

		PLANDMANCLIENT plandmanclient = GLGaeaClient::GetInstance().GetActiveMap();
		if ( plandmanclient )
		{
			bPVPTyranny = plandmanclient->m_bPVPTyrannyMap;
			bPVPSchoolWars = plandmanclient->m_bPVPSchoolWarsMap;
			bPVCaptureTheFlag = plandmanclient->m_bPVPCaptureTheFlagMap;
		}

		/*pvp tyranny, Juver, 2017/08/25 */
		if ( bPVPTyranny )
		{
			if ( !IsVisibleGroup( REBIRTH_DIALOGUE_TYRANNY ) )
			{
				if ( m_pRebirthDialogueTyranny )
					m_pRebirthDialogueTyranny->timeStart = CTime::GetCurrentTime().GetTime();

				ShowGroupFocus ( REBIRTH_DIALOGUE_TYRANNY, true );
				ShowGroupFocus ( BASIC_CHAT_BOX, true );

				if ( IsVisibleGroup( PVP_TYRANNY_RANKINGS ) )
				{
					HideGroup( PVP_TYRANNY_RANKINGS );
					m_bPVPTyrannyShowRanking = TRUE;
				}
			}
		}
		/*school wars, Juver, 2018/01/19 */
		else if ( bPVPSchoolWars )
		{
			if ( !IsVisibleGroup( REBIRTH_DIALOGUE_SCHOOLWARS ) )
			{
				if ( m_pRebirthDialogueSchoolWars )
					m_pRebirthDialogueSchoolWars->timeStart = CTime::GetCurrentTime().GetTime();

				ShowGroupFocus ( REBIRTH_DIALOGUE_SCHOOLWARS, true );
				ShowGroupFocus ( BASIC_CHAT_BOX, true );

				if ( IsVisibleGroup( PVP_SCHOOLWARS_RANKINGS ) )
				{
					HideGroup( PVP_SCHOOLWARS_RANKINGS );
					m_bPVPSchoolWarsShowRanking = TRUE;
				}
			}
		}
		/*pvp capture the flag, Juver, 2018/02/01 */
		else if ( bPVCaptureTheFlag )
		{
			if ( !IsVisibleGroup( REBIRTH_DIALOGUE_CAPTURE_THE_FLAG ) )
			{
				if ( m_pRebirthDialogueCaptureTheFlag )
					m_pRebirthDialogueCaptureTheFlag->timeStart = CTime::GetCurrentTime().GetTime();

				ShowGroupFocus ( REBIRTH_DIALOGUE_CAPTURE_THE_FLAG, true );
				ShowGroupFocus ( BASIC_CHAT_BOX, true );

				if ( IsVisibleGroup( PVP_CAPTURE_THE_FLAG_RANKINGS ) )
				{
					HideGroup( PVP_CAPTURE_THE_FLAG_RANKINGS );
					m_bPVPCaptureTheFlagShowRanking = TRUE;
				}
			}
		}
		else if ( !IsVisibleGroup ( REBIRTH_WINDOW ) )
		{
			bool bItemRebirth = (pCharacter->ISREVIVE ())?true:false;
			bool bAutoRebirth = false;
			bool bCantUseRebirth = FALSE;

			//	��ȥ�� �� Ÿ�� üũ
			if ( bItemRebirth ) 
			{
				SITEM* pITEM = pCharacter->GET_SLOT_ITEMDATA(SLOT_ORNAMENT);
				if ( pITEM && pCharacter->IsCoolTime( pITEM->sBasicOp.sNativeID ) )
				{
					bItemRebirth = FALSE;
				}
			}

			if( pCharacter->IsRestartPossible() == FALSE )
			{
				bItemRebirth = FALSE;
				bCantUseRebirth = TRUE;
			}

			if ( bItemRebirth )
			{
				// �Ҹ� ��ȥ���̸� �ڵ� ��ȥ�� ���
				SITEM* pITEM = pCharacter->GET_SLOT_ITEMDATA(SLOT_ORNAMENT);
				if ( pITEM && !pITEM->ISINSTANCE() ) bAutoRebirth = true;
			}
			
			m_pRebirthWindow->SetItemRebirth ( bItemRebirth, bAutoRebirth, bCantUseRebirth );
			//m_pRebirthWindow->SetCallerID ( MODAL_REBIRTH );
			ShowGroupFocus ( REBIRTH_WINDOW, true );
			ShowGroupFocus ( BASIC_CHAT_BOX, true );
		}
	}
	else
	{
		HideGroup ( REBIRTH_WINDOW, true );
		HideGroup ( REBIRTH_DIALOGUE_TYRANNY, true );			/*pvp tyranny, Juver, 2017/08/25 */
		HideGroup ( REBIRTH_DIALOGUE_SCHOOLWARS, true );		/*school wars, Juver, 2018/01/20 */
		HideGroup ( REBIRTH_DIALOGUE_CAPTURE_THE_FLAG, true );	/*pvp capture the flag, Juver, 2018/02/01 */

		if ( IsGateOpen () )
		{
			//	���� �� �̸��� ���ٸ�, �̵��� �� ����.
			if ( m_pMapMoveDisplay )
			{
				CString strMapName = pCharacter->DetectGateToMapName();
				m_pMapMoveDisplay->SetMapName ( strMapName + ID2GAMEINTEXT ( "MAPMOVE_MESSAGE" ) );

				UIRECT rcGlobalPos = m_pMapMoveDisplayDummy->GetGlobalPos ();
				rcGlobalPos.top -= fMENU_LIFTUP;
				m_pMapMoveDisplay->SetGlobalPos ( rcGlobalPos );

				if ( strMapName.GetLength () )	ShowGroupFocus ( MAPMOVE_DISPLAY );
				else							HideGroup ( MAPMOVE_DISPLAY );
			}		
		}
		else
		{
			HideGroup ( MAPMOVE_DISPLAY );
		}
	}

	//	����, ���� ���μ��� üũ
	if ( IsBlockProgramFound () )
	{
		if ( !IsVisibleGroup ( WAITSERVER_DISPLAY ) )
			DoModal ( ID2GAMEINTEXT("BLOCK_PROGRAM_FOUND"), MODAL_INFOMATION, OK, MODAL_CLOSEGAME );
	}

	if ( !DxGlobalStage::GetInstance().IsEmulator() )		//��Ʈ��ũ ������ ������ ���
	{
		if ( !DxGlobalStage::GetInstance().IsSTATE( DxGlobalStage::EM_CHANGE ) )
		{
			if ( !DxGlobalStage::GetInstance().GetNetClient()->IsOnline() )
			{
				if ( !IsVisibleGroup ( WAITSERVER_DISPLAY ) )
					DoModal ( ID2GAMEINTEXT("PLAYINTERFACE_5"), MODAL_INFOMATION, OK, MODAL_CLOSEGAME );				
			}
		}
	}

	if ( m_pAdminMessageDisplay )
	{
		if ( IsVisibleGroup ( ADMIN_MESSAGE_DISPLAY ) )
		{
			if ( m_pAdminMessageDisplay->GetLifeTime () < 0.0f )
			{
				HideGroup ( ADMIN_MESSAGE_DISPLAY );			
			}
		}
	}

	//if ( m_pSimpleMessageMan )
	//{
	//if ( IsVisibleGroup ( SIMPLE_MESSAGE_MAN ) )
	//{
	//	if ( m_pSimpleMessageMan->GetLifeTime () < 0.0f )
	//	{
	//		HideGroup ( SIMPLE_MESSAGE_MAN );			
	//	}
	//}
	//	}

	BOOL bPartyState = FALSE;
	GLPARTY_CLIENT *pMaster = GLPartyClient::GetInstance().GetMaster();	//	������
	if ( pMaster )
	{
		bPartyState = TRUE;
	}

	if ( bPartyState != m_bPartyStateBack )
	{
		if ( GLPARTY_CLIENT *pMaster = GLPartyClient::GetInstance().GetMaster() )
		{
			ShowGroupFocus ( MINIPARTY_WINDOW );
			HideGroup ( MINIPARTY_OPEN );
		}
		else
		{
			ShowGroupBottom ( MINIPARTY_OPEN );
			HideGroup ( MINIPARTY_WINDOW );
		}
		m_bPartyStateBack = bPartyState;

// #ifdef CH_PARAM // �߱� �������̽� ����
//		UpdatePotionTrayPosition();
// #endif
	}

	UpdateStateQuestAlarm ();
	UpdateStateSimpleHP ();	

	if ( IsVisibleGroup ( ACADEMY_CONFT_DISPLAY ) || IsVisibleGroup ( SYSTEM_MESSAGE_WINDOW ) )
	{
		HideGroup ( SIMPLE_MESSAGE_MAN );
	}
	else
	{
		if ( RANPARAM::bSHOW_TIP )
		{
			ShowGroupBottom ( SIMPLE_MESSAGE_MAN, true );
		}
		else
		{
			HideGroup ( SIMPLE_MESSAGE_MAN, true );
		}
	}
}

void CInnerInterface::MoveBasicInfoWindow ()
{
	CUIControl * pControl(NULL);

//#ifdef CH_PARAM // �߱� �������̽� ����
//	pControl = m_pBasicQuickSkillSlot;
//#else
	pControl = m_pUILeftTopGroup;
//#endif

	if ( !pControl ) return;

	const UIRECT& rcBasicInfoViewDummy = m_pBasicInfoViewDummy->GetGlobalPos ();
	const UIRECT& rcLeftTopGroupDummy = m_pLeftTopGroupDummy->GetGlobalPos ();
	const UIRECT& rcBasicInfoView = m_pBasicInfoView->GetGlobalPos ();
	const UIRECT& rcLeftTopGroup = pControl->GetGlobalPos ();

	const D3DXVECTOR2 vLeftTopGroup ( rcLeftTopGroup.left, rcLeftTopGroup.top );
	const D3DXVECTOR2 vLeftTopGroupDummy ( rcLeftTopGroupDummy.left, rcLeftTopGroupDummy.top );

	const D3DXVECTOR2 vBasicInfoViewDummy (rcBasicInfoViewDummy.left,rcBasicInfoViewDummy.top);
	const D3DXVECTOR2 vBasicInfoView (rcBasicInfoView.left,rcBasicInfoView.top);
	const D3DXVECTOR2 vDist = vBasicInfoViewDummy - vBasicInfoView;
	const float fDist = D3DXVec2LengthSq ( &vDist );

	if ( fDist > 900.0f )	//	��Ʈ���� ��������?
	{	
		if ( vBasicInfoViewDummy != vLeftTopGroup ) pControl->SetGlobalPos ( vBasicInfoViewDummy );
	}
	else
	{
		if ( vLeftTopGroupDummy != vLeftTopGroup )		pControl->SetGlobalPos ( vLeftTopGroupDummy );
		if ( vBasicInfoViewDummy != vBasicInfoView )	m_pBasicInfoView->SetGlobalPos( vBasicInfoViewDummy );
	}
}

void CInnerInterface::BasicInfoViewDoubleClick()
{
	CUIControl * pControl(NULL);

//#ifdef CH_PARAM // �߱� �������̽� ����
//	pControl = m_pBasicQuickSkillSlot;
//#else
	pControl = m_pUILeftTopGroup;
//#endif

	const UIRECT& rcBasicInfoViewDummy = m_pBasicInfoViewDummy->GetGlobalPos ();
	const UIRECT& rcLeftTopGroupDummy = m_pLeftTopGroupDummy->GetGlobalPos ();

	const D3DXVECTOR2 vLeftTopGroupDummy ( rcLeftTopGroupDummy.left, rcLeftTopGroupDummy.top );
	const D3DXVECTOR2 vBasicInfoViewDummy (rcBasicInfoViewDummy.left,rcBasicInfoViewDummy.top);

	pControl->SetGlobalPos ( vLeftTopGroupDummy );
	m_pBasicInfoView->SetGlobalPos( vBasicInfoViewDummy );
}

//************************************
// Method:    IsOpenWindowToMoveBlock
// FullName:  CInnerInterface::IsOpenWindowToMoveBlock
// Access:    public 
// Returns:   BOOL
// Qualifier: 
//************************************
BOOL CInnerInterface::IsOpenWindowToMoveBlock()
{
	// ���⿡ �߰��Ǵ� ��Ʈ���� �ݵ��� InitDeviceObjects���� ������ SetVisibleSingle( FALSE )�� ����� �Ѵ�.
	if( IsVisibleGroup( TRADE_WINDOW ) || 
		IsVisibleGroup( TRADEINVENTORY_WINDOW ) || 
		IsVisibleGroup( MARKET_WINDOW ) || 
		IsVisibleGroup( STORAGE_WINDOW ) || 
		//IsVisibleGroup( ITEMBANK_WINDOW ) || 
		( IsVisibleGroup( INVENTORY_WINDOW ) && m_pInventoryUI && m_pInventoryUI->GetActivePage() == ITEMBANK_PAGE ) || 
		IsVisibleGroup( MAP_REQUIRE_CHECK ) ||
		IsVisibleGroup( REBIRTH_WINDOW ) ||
		IsVisibleGroup( REBIRTH_DIALOGUE_TYRANNY ) ||			/*pvp tyranny, Juver, 2017/08/25 */
		IsVisibleGroup( REBIRTH_DIALOGUE_SCHOOLWARS ) ||		/*school wars, Juver, 2018/01/20 */
		IsVisibleGroup( REBIRTH_DIALOGUE_CAPTURE_THE_FLAG ) ||	/*pvp capture the flag, Juver, 2018/02/01 */
		IsVisibleGroup( DIALOGUE_WINDOW_RENEWAL ) || 
		IsVisibleGroup( DIALOGUE_WINDOW ) || 
		IsVisibleGroup( CLUB_STORAGE_WINDOW ) || 
		IsVisibleGroup( PRIVATE_MARKET_MAKE ) || 
		IsVisibleGroup( GAMBLE_DIALOGUE ) || 
		IsVisibleGroup( GAMBLE_SELECT_DIALOGUE ) || 
		IsVisibleGroup( GAMBLE_AGAIN_DIALOGUE ) ||
		IsVisibleGroup( GAMBLE_ANIMATION_DIALOGUE ) || 
		IsVisibleGroup( GAMBLE_RESULT_EVEN_DIALOGUE ) ||
		IsVisibleGroup( GAMBLE_RESULT_ODD_DIALOGUE ) ||
		IsVisibleGroup( ITEM_REBUILD_WINDOW ) || 
		IsVisibleGroup( REBUILDINVENTORY_WINDOW ) ||	// ITEMREBUILD_MARK
		IsVisibleGroup( ITEM_GARBAGE_WINDOW ) || 
		IsVisibleGroup( GARBAGEINVENTORY_WINDOW ) ||	// ������
		IsVisibleGroup( ITEMSHOP_WINDOW ) ||
		IsVisibleGroup( ITEM_SHOP_SEARCH_WINDOW ) ||
		IsVisibleGroup( ITEM_SEARCH_RESULT_WINDOW ) || // ������ �˻� â
		IsVisibleGroup( PETSKIN_MIX_IMAGE_WINDOW ) ||
		IsVisibleGroup( ITEM_MIX_WINDOW ) ||
		IsVisibleGroup( ITEM_MIX_INVEN_WINDOW ) ||
		IsVisibleGroup( PVP_TYRANNY_RANKINGS ) ||			/*pvp tyranny, Juver, 2017/08/25 */
		IsVisibleGroup( PRODUCT_WINDOW ) ||					/*product item, Juver, 2017/10/17 */
		IsVisibleGroup( BIKE_COLOR_WINDOW )	||				/*bike color , Juver, 2017/11/16 */
		IsVisibleGroup( CHANGE_SCALE_WINDOW ) ||			/*change scale card, Juver, 2018/01/03 */
		IsVisibleGroup( ITEM_COLOR_WINDOW )	||				/*item color, Juver, 2018/01/10 */
		IsVisibleGroup( ITEM_TRANSFER_WINDOW ) ||			/*item transfer card, Juver, 2018/01/18 */
		IsVisibleGroup( ITEM_TRANSFER_INVEN_WINDOW ) ||		/*item transfer card, Juver, 2018/01/18 */
		IsVisibleGroup( PVP_SCHOOLWARS_RANKINGS )  ||		/*school wars, Juver, 2018/01/20 */
		IsVisibleGroup( PVP_CAPTURE_THE_FLAG_RANKINGS )	||	/*pvp capture the flag, Juver, 2018/02/02 */
		IsVisibleGroup( CAR_COLOR_WINDOW ) ||				/* car, cart color, Juver, 2018/02/14 */
		IsVisibleGroup( ITEMMALL_WINDOW )					/*dmk14 ingame web*/
		)

	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*
#ifdef CH_PARAM // �߱� �������̽� ����
void CInnerInterface::UpdatePotionTrayPosition()
{
	m_pUILeftTopGroup->SetGlobalPos( m_pBasicPotionTrayDummy->GetGlobalPos() );
	m_pQuestAlarm->SetGlobalPos( m_pQuestAlarmDummy->GetGlobalPos() );

	BOOL bVisibleQuest = IsVisibleGroup( QUEST_ALARM);
	BOOL bVisibleMini = IsVisibleGroup( MINIPARTY_OPEN );

	if( bVisibleQuest && bVisibleMini ) return;

	UIRECT rcDummy = m_pUILeftTopGroup->GetGlobalPos();
	rcDummy.left += 41; // ����Ʈ, �̴���Ƽ ��ư�� ���� ũ�Ⱑ 41
	rcDummy.right = rcDummy.left + rcDummy.sizeX;
	m_pUILeftTopGroup->SetGlobalPos( rcDummy );

	CUIControl * pUIControl(NULL);

	if( !bVisibleQuest && !bVisibleMini )
	{
		pUIControl = m_pUILeftTopGroup;
	}
	else if( !bVisibleMini )
	{
		pUIControl = m_pQuestAlarm;
	}

	if( pUIControl != NULL )
	{
		rcDummy = pUIControl->GetGlobalPos();
		rcDummy.left += 41;
		rcDummy.right = rcDummy.left + rcDummy.sizeX;
		pUIControl->SetGlobalPos( rcDummy );
	}
}
#endif
*/
void CInnerInterface::SetArmSwap( BOOL bSwap )
{
	m_pCharacterWindow->SetArmSwapTabButton( bSwap );
}

bool CInnerInterface::ItemShopAuth ()
{

//	ItemShopAuth
#if defined ( JP_PARAM ) 
	if ( m_bItemShopLoad )	return false;

	GLCharacter* pChar = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pChar ) return false;

	CString strData = "";
	strData.Format( "%s?id=%s& uuid=%s", RANPARAM::ItemShopAddress, pChar->m_szUID, RANPARAM::GETJAPANUUID() );

	CCommonWeb::Get()->Navigate( CCommonWeb::ITEMSHOP_ID, strData, true );
	CCommonWeb::Get()->ReSetCompleteLoad( CCommonWeb::ITEMSHOP_ID );

	m_bItemShopLoad = true;

	// �ε� �Ϸ� �޼����� �ö����� ��ٸ���.
	return false;
#endif	
	return true;
}

void CInnerInterface::ItemShopVisible()
{

//	ItemShopAuth
#if defined ( JP_PARAM ) 
	if ( m_bItemShopLoad )
	{
		if ( CCommonWeb::Get()->IsCompleteLoad( CCommonWeb::ITEMSHOP_ID ) )
		{
			ShowGroupFocus ( ITEMSHOP_WINDOW );			
			HideGroup ( HELP_WINDOW );
			m_bItemShopLoad = false;
		}
	}
#endif
	return;
}

void CInnerInterface::SetItemMixResult( CString strMsg, bool bSuccess, bool bFail )
{
	m_pItemMixWindow->SetItemMixResult( strMsg, bSuccess, bFail );
}

void CInnerInterface::SET_GATHER_GAUGE( DWORD dwGaeaID, float fTime )
{
	m_pGatherGauge->SetTime( fTime );
	m_pGatherGauge->SetMaterial ( dwGaeaID );
}

void CInnerInterface::VisibleCDMRanking( bool bVisible )
{
	if ( bVisible )
	{
		HideGroup( QUEST_HELPER_DISPLAY );
		ShowGroupBottom ( CDM_RANKING_DISPLAY );
	}
	else
	{
		HideGroup( CDM_RANKING_DISPLAY );
		ShowGroupBottom ( QUEST_HELPER_DISPLAY );
	}
}

void CInnerInterface::RefreshCDMRanking()
{
	if ( m_pCdmRankingDisplay && m_pCdmRankingDisplay->IsVisible() )
		m_pCdmRankingDisplay->RefreshCdmRanking();
}

bool CInnerInterface::IsInventoryBlocked()
{
	//detect windows that must prevent the inventory window to show up
	if ( IsVisibleGroup( TRADEINVENTORY_WINDOW ) ||
		IsVisibleGroup( GARBAGEINVENTORY_WINDOW ) ||
		IsVisibleGroup( ITEM_MIX_INVEN_WINDOW ) ||
		IsVisibleGroup( REBUILDINVENTORY_WINDOW )  
		)	return TRUE;
	return FALSE;
}

/*item preview, Juver, 2017/07/27 */
bool CInnerInterface::PreviewItemCheckSimple( SNATIVEID sID )
{
	SITEM* pItem = GLItemMan::GetInstance().GetItem( sID );
	if ( !pItem )	return false;

	if ( pItem->sBasicOp.emItemType == ITEM_SUIT )
	{
		return PreviewItemCheckWear( sID );
	}
	else if ( pItem->sBasicOp.emItemType == ITEM_BOX )
	{
		for ( int i=0; i<ITEM::SBOX::ITEM_SIZE; ++i )
		{
			SNATIVEID sNativeID = pItem->sBox.sITEMS[i].nidITEM;
			SITEM* pItemBox = GLItemMan::GetInstance().GetItem( sNativeID );
			if( pItemBox )
			{
				bool bCan = PreviewItemCheckWear( sNativeID );
				if ( bCan )	return true;
			}
		}
	}

	return false;
}

/*item preview, Juver, 2017/07/27 */
bool CInnerInterface::PreviewItemCheckWear( SNATIVEID sID )
{
	SITEM* pItem = GLItemMan::GetInstance().GetItem( sID );
	if ( !pItem )	return false;

	EMCHARCLASS	emClass = GLGaeaClient::GetInstance().GetCharacter()->m_emClass;
	WORD wSchool = GLGaeaClient::GetInstance().GetCharacter()->m_wSchool;
	EMCHARINDEX emIndex = CharClassToIndex( emClass );

	if ( !(pItem->sBasicOp.dwReqCharClass&emClass) )			return false;
	if ( !(pItem->sBasicOp.dwReqSchool&index2school(wSchool)) ) return false;

	//new item preview
	if ( pItem->sBasicOp.emItemType == ITEM_SUIT )
	{
		if ( pItem->sSuitOp.emSuit > SUIT_WRIST && 
			pItem->sSuitOp.emSuit != SUIT_BELT &&
			pItem->sSuitOp.emSuit != SUIT_FACE &&
			pItem->sSuitOp.emSuit != SUIT_MISC )	return false;

		std::string strR = pItem->GetWearingFileR( emIndex );
		std::string strL = pItem->GetWearingFileL( emIndex );

		if ( strR.size() > 0 || strL.size() > 0 )	return true;
	}

	return false;
}

/*item preview, Juver, 2017/07/27 */
void CInnerInterface::PreviewItem( SITEMCUSTOM sItem )
{
	if ( !m_pItemPreviewWindow )	return;

	/*item wrapper, Juver, 2018/01/12 */
	SITEM* pitem_data_check = GLItemMan::GetInstance().GetItem( sItem.sNativeID );
	if ( pitem_data_check && pitem_data_check->sBasicOp.emItemType == ITEM_WRAPPER_BOX && sItem.nidDISGUISE != NATIVEID_NULL() )
	{
		SITEMCUSTOM sitem_new = sItem;
		sitem_new.sNativeID = sitem_new.nidDISGUISE;
		sitem_new.nidDISGUISE = NATIVEID_NULL();

		if ( !PreviewItemCheckSimple( sitem_new.sNativeID ) )	return;
		m_pItemPreviewWindow->StartPreview( sitem_new );
	}
	else
	{
		if ( !PreviewItemCheckSimple( sItem.sNativeID ) )	return;
		m_pItemPreviewWindow->StartPreview( sItem );
	}
	
	ShowGroupFocus( ITEMPREVIEW_WINDOW );
}

/*pet status, Juver, 2017/07/30 */
void CInnerInterface::ShowPetStatus( bool bVisible )
{
	if ( m_pPetStatus )	
	{
		if ( bVisible )	ShowGroupBottom ( PET_STATUS_INFO );
		else	HideGroup( PET_STATUS_INFO );
	}

	/*pet skill info display, Juver, 2017/12/01 */
	if ( m_pPetSkillInfoDisplay )
	{
		if ( bVisible )	ShowGroupBottom ( PET_SKILL_INFO_DISPLAY );
		else	HideGroup( PET_SKILL_INFO_DISPLAY );
	}

	/*dual pet skill, Juver, 2017/12/29 */
	if ( m_pPetSkillInfoDisplay2 )
	{
		if ( bVisible )	ShowGroupBottom ( PET_SKILL_INFO_DISPLAY2 );
		else	HideGroup( PET_SKILL_INFO_DISPLAY2 );
	}
}

/*vehicle status, Juver, 2017/07/30 */
void CInnerInterface::ShowVehicleStatus( bool bVisible )
{
	if ( !m_pVehicleStatus )	return;

	if ( bVisible )	ShowGroupBottom ( VEHICLE_STATUS_INFO );
	else	HideGroup( VEHICLE_STATUS_INFO );
}

/*item link, Juver, 2017/07/31 */
void CInnerInterface::LinkItem( SITEMCUSTOM* pItem )
{
	if ( GetChat() && pItem )
	{
		GetChat()->AddItemLink( pItem );
	}
}

/*vehicle booster system, Juver, 2017/08/11 */
void CInnerInterface::VehicleBoosterVisible( bool bVisible )
{
	if ( !m_pVehicleBoosterDisplay )	return;

	if (bVisible)	ShowGroupBottom ( VEHICLE_BOOSTER_DISPLAY );
	else	HideGroup( VEHICLE_BOOSTER_DISPLAY );
}

/*box contents, Juver, 2017/08/30 */
void CInnerInterface::ShowBoxContents( SNATIVEID sidItem, BOOL bBoxSelect, WORD wPosX, WORD wPosY )
{
	if ( IsVisibleGroup( INFO_BOX_CONTENTS ))
		HideGroup( INFO_BOX_CONTENTS );

	if ( !m_pInfoBoxContents )			return;
	if ( sidItem == NATIVEID_NULL() )	return;

	SITEM* pitem = GLItemMan::GetInstance().GetItem( sidItem );
	if ( !pitem )	return;

	if ( !( pitem->sBasicOp.emItemType == ITEM_BOX || pitem->sBasicOp.emItemType == ITEM_PREMIUMSET ) )	return;

	ShowGroupFocus( INFO_BOX_CONTENTS );

	m_pInfoBoxContents->DataShow( pitem, bBoxSelect, wPosX, wPosY );

}

/* Competition UI, Juver, 2017/08/24 */
void CInnerInterface::SetCompetitionButtonAlarm( BOOL bAlarm )
{
	if ( m_pCompetitionNotifyButton ){
		m_pCompetitionNotifyButton->SetButtonAlarm( bAlarm );
	}
}

/* Competition UI, Juver, 2017/08/24 */
void CInnerInterface::ShowCompetitionWindow()
{
	if ( !IsVisibleGroup( COMPETITION_WINDOW )){
		SetDefaultPosInterface( COMPETITION_WINDOW );
		ShowGroupFocus( COMPETITION_WINDOW );
	}

	SetCompetitionButtonAlarm( FALSE );
}

/* extra notice, Juver, 2017/08/24 */
void CInnerInterface::ShowExtraNotice( DWORD dwTextColor, const char* szFormat, ... )
{
	const int MAX_STRLENGTH = 1024;

	va_list argList;
	char szBuffer[MAX_STRLENGTH];

	va_start(argList, szFormat);
	StringCbVPrintf(szBuffer, MAX_STRLENGTH, szFormat, argList);
	va_end(argList);

	ShowGroupTop ( EXTRA_NOTICE );

	if ( m_pExtraNotice ){
		m_pExtraNotice->AddText( szBuffer, dwTextColor );
	}
}

/*pvp tyranny, Juver, 2017/08/25 */
void CInnerInterface::PVPTyrannyTowerCaptureResetPos()
{
	if ( !m_pPVPTyrannyTowerCapture )		return;
	if ( !m_pPVPTyrannyTowerCaptureDummy )	return;

	m_pPVPTyrannyTowerCapture->SetGlobalPos( 
		D3DXVECTOR2( m_pPVPTyrannyTowerCaptureDummy->GetGlobalPos ().left, m_pPVPTyrannyTowerCaptureDummy->GetGlobalPos ().top ) );
}

/*pvp tyranny, Juver, 2017/08/25 */
void CInnerInterface::PVPTyrannyShowCaptureNotice( WORD wSchool )
{
	for( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i )
	{
		if ( m_pPVPTyrannyNoticeCapture[i] )
			m_pPVPTyrannyNoticeCapture[i]->SetVisibleSingle( FALSE );
	}

	if ( wSchool < TYRANNY_SCHOOL_SIZE )
	{
		if ( m_pPVPTyrannyNoticeCapture[wSchool] )
			m_pPVPTyrannyNoticeCapture[wSchool]->SetVisibleSingle( TRUE );
	}
}

/*pvp tyranny, Juver, 2017/08/25 */
void CInnerInterface::PVPTyrannyShow2CaptureNotice( WORD wSchool )
{
	for( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i )
	{
		if ( m_pPVPTyranny2Capture[i] )
			m_pPVPTyranny2Capture[i]->SetVisibleSingle( FALSE );
	}

	if ( m_pPVPTyranny2CaptureOwn )
		m_pPVPTyranny2CaptureOwn->SetVisibleSingle( FALSE );

	if ( wSchool < TYRANNY_SCHOOL_SIZE )
	{
		if ( m_pPVPTyranny2Capture[wSchool] )
			m_pPVPTyranny2Capture[wSchool]->SetVisibleSingle( TRUE );
	}
}

/*pvp tyranny, Juver, 2017/08/25 */
void CInnerInterface::PVPTyrannyShow2CaptureOwnNotice()
{
	for( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i )
	{
		if ( m_pPVPTyranny2Capture[i] )
			m_pPVPTyranny2Capture[i]->SetVisibleSingle( FALSE );
	}

	if ( m_pPVPTyranny2CaptureOwn )
		m_pPVPTyranny2CaptureOwn->SetVisibleSingle( FALSE );

	if ( m_pPVPTyranny2CaptureOwn )
		m_pPVPTyranny2CaptureOwn->SetVisibleSingle( TRUE );
}

/*pvp tyranny, Juver, 2017/08/25 */
void CInnerInterface::PVPTyrannyShowRanking()
{
	if ( IsVisibleGroup( REBIRTH_DIALOGUE_TYRANNY ))
	{
		//character is dead show rank window later when revived!
		m_bPVPTyrannyShowRanking = true;
		return;
	}

	if ( !IsVisibleGroup( PVP_TYRANNY_RANKINGS )){
		SetDefaultPosInterface( PVP_TYRANNY_RANKINGS );
		ShowGroupFocus( PVP_TYRANNY_RANKINGS );
	}

	if ( m_pPVPTyrannyRankings ){
		m_pPVPTyrannyRankings->UpdateRankings();
	}

	m_bPVPTyrannyShowRanking = false;
}

/*pvp tyranny, Juver, 2017/08/25 */
void CInnerInterface::PVPTyrannyShowTopRanking()
{
	if ( !IsVisibleGroup( COMPETITION_WINDOW ))	return;
	if ( !m_pCompetitionWindow )	return;

	ShowGroupFocus( PVP_TYRANNY_TOPRANKINGS );

	if ( m_pPVPTyrannyTopRankings )
	{
		UIRECT rect = m_pCompetitionWindow->GetGlobalPos();
		rect.left = rect.left + rect.sizeX;

		m_pPVPTyrannyTopRankings->SetGlobalPos( D3DXVECTOR2(rect.left, rect.top) );
	}
}

/*pvp tyranny, Juver, 2017/08/25 */
void CInnerInterface::PVPTyrannyShowStartNotice()
{
	ShowGroupFocus( PVP_TYRANNY_BATTLE_START_NOTICE );
}

/*pvp tyranny, Juver, 2017/08/25 */
void CInnerInterface::PVPTyrannyShowEndNotice()
{
	ShowGroupFocus( PVP_TYRANNY_BATTLE_END_NOTICE );
}

/*item exchange, Juver, 2017/10/12 */
void CInnerInterface::SetNPCItemExchangeWindowOpen( SNATIVEID sid_npc )
{
	if ( m_pNPCItemExchangeWindow )
	{
		m_pNPCItemExchangeWindow->LoadList ( sid_npc );
		ShowGroupFocus ( NPC_ITEM_EXCHANGE_WINDOW );
	}
}

/*product item, Juver, 2017/10/15 */
void CInnerInterface::OpenProductWindow()
{
	if ( IsVisibleGroup( PRODUCT_WINDOW ) )	return;

	CloseAllWindow();

	ShowGroupFocus( PRODUCT_WINDOW );

	if ( m_pProductWindow )
		m_pProductWindow->LoadData();
}

/*student record ui, Juver, 2017/10/30 */
void CInnerInterface::OpenStudentRecord()
{
	if ( !RANPARAM::bFeatureStudentRecord )	return;

	if ( IsVisibleGroup( STUDENT_RECORD_WINDOW ) )	return;

	ShowGroupFocus( STUDENT_RECORD_WINDOW );
}

/*activity system, Juver, 2017/11/05 */
void CInnerInterface::OpenActivityItemWindow()
{
	if ( !RANPARAM::bFeatureActivity )	return;

	if ( IsVisibleGroup( ACTIVITY_ITEM_WINDOW ) )	return;

	ShowGroupFocus( ACTIVITY_ITEM_WINDOW );
}

/*Codex System, Jhoniex 2/14/2024 */
void CInnerInterface::OpenStudentCodex()
{

	if ( !IsVisibleGroup( STUDENT_CODEX_WINDOW ))	
	{
		ShowGroupFocus( STUDENT_CODEX_WINDOW );
		//m_pStudentRecordWindow->LoadData();
	}
	else
	{
		HideGroup ( STUDENT_CODEX_WINDOW );
	}
}

/*equipment lock, Juver, 2018/01/16 */
void CInnerInterface::OpenEquipmentLockInput()
{
	if ( IsVisibleGroup( EQUIPMENT_LOCK_INPUT_WINDOW ) )	return;
	ShowGroupFocus( EQUIPMENT_LOCK_INPUT_WINDOW );
}

/*equipment lock, Juver, 2018/01/16 */
void CInnerInterface::OpenEquipmentLockRecover( std::string strName, std::string strEPass )
{
	if ( strName.size() <= 0 )	return;
	if ( strEPass.size() <= 0 )	return;
	if ( !m_pEquipmentLockRecoverWindow )	return;

	if ( IsVisibleGroup( EQUIPMENT_LOCK_RECOVER_WINDOW ) )	return;
	ShowGroupFocus( EQUIPMENT_LOCK_RECOVER_WINDOW );

	m_pEquipmentLockRecoverWindow->setCurrentKey( strEPass.c_str() ); 
}

/*storage lock, Juver, 2018/01/16 */
void CInnerInterface::OpenStorageLockInput()
{
	if ( IsVisibleGroup( STORAGE_LOCK_INPUT_WINDOW ) )	return;
	ShowGroupFocus( STORAGE_LOCK_INPUT_WINDOW );
}

/*storage lock, Juver, 2018/01/16 */
void CInnerInterface::OpenStorageLockRecover( std::string strName, std::string strSPass )
{
	if ( strName.size() <= 0 )	return;
	if ( strSPass.size() <= 0 )	return;
	if ( !m_pStorageLockRecoverWindow )	return;

	if ( IsVisibleGroup( STORAGE_LOCK_RECOVER_WINDOW ) )	return;
	ShowGroupFocus( STORAGE_LOCK_RECOVER_WINDOW );

	m_pStorageLockRecoverWindow->setCurrentKey( strSPass.c_str() ); 
}

/*inventory lock, Juver, 2018/01/16 */
void CInnerInterface::OpenInventoryLockInput()
{
	if ( IsVisibleGroup( INVENTORY_LOCK_INPUT_WINDOW ) )	return;
	ShowGroupFocus( INVENTORY_LOCK_INPUT_WINDOW );
}

/*inventory lock, Juver, 2018/01/16 */
void CInnerInterface::OpenInventoryLockRecover( std::string strName, std::string strIPass )
{
	if ( strName.size() <= 0 )	return;
	if ( strIPass.size() <= 0 )	return;
	if ( !m_pInventoryLockRecoverWindow )	return;

	if ( IsVisibleGroup( INVENTORY_LOCK_RECOVER_WINDOW ) )	return;
	ShowGroupFocus( INVENTORY_LOCK_RECOVER_WINDOW );

	m_pInventoryLockRecoverWindow->setCurrentKey( strIPass.c_str() ); 
}
/*school wars, Juver, 2018/01/20 */
void CInnerInterface::PVPSchoolWarsScoreResetPos()
{
	if ( !m_pPVPSchoolWarsScore )		return;
	if ( !m_pPVPSchoolWarsScoreDummy )	return;

	m_pPVPSchoolWarsScore->SetGlobalPos( 
		D3DXVECTOR2( m_pPVPSchoolWarsScoreDummy->GetGlobalPos ().left, m_pPVPSchoolWarsScoreDummy->GetGlobalPos ().top ) );
}

/*school wars, Juver, 2018/01/20 */
void CInnerInterface::PVPSchoolWarsShowTopRanking()
{
	if ( !IsVisibleGroup( COMPETITION_WINDOW ))	return;
	if ( !m_pCompetitionWindow )	return;

	ShowGroupFocus( PVP_SCHOOLWARS_TOPRANKINGS );

	if ( m_pPVPSchoolWarsTopRankings )
	{
		UIRECT rect = m_pCompetitionWindow->GetGlobalPos();
		rect.left = rect.left + rect.sizeX;
		m_pPVPSchoolWarsTopRankings->SetGlobalPos( D3DXVECTOR2(rect.left, rect.top) );
	}
}

/*school wars, Juver, 2018/01/20 */
void CInnerInterface::PVPSchoolWarsShowStartNotice()
{
	ShowGroupFocus( PVP_SCHOOLWARS_BATTLE_START_NOTICE );
}

/*school wars, Juver, 2018/01/20 */
void CInnerInterface::PVPSchoolWarsShowEndNotice()
{
	ShowGroupFocus( PVP_SCHOOLWARS_BATTLE_END_NOTICE );
}

/*school wars, Juver, 2018/01/20 */
void CInnerInterface::PVPSchoolWarsShowRanking()
{
	if ( IsVisibleGroup( REBIRTH_DIALOGUE_SCHOOLWARS ))
	{
		//character is dead show rank window later when revived!
		m_bPVPSchoolWarsShowRanking = true;
		return;
	}

	if ( !IsVisibleGroup( PVP_SCHOOLWARS_RANKINGS )){
		SetDefaultPosInterface( PVP_SCHOOLWARS_RANKINGS );
		ShowGroupFocus( PVP_SCHOOLWARS_RANKINGS );
	}

	if ( m_pPVPSchoolWarsRankings ){
		m_pPVPSchoolWarsRankings->UpdateRankings();
	}

	m_bPVPSchoolWarsShowRanking = false;
}

/*pvp capture the flag, Juver, 2018/02/01 */
void CInnerInterface::PVPCaptureTheFlagShowTopRanking()
{
	if ( !IsVisibleGroup( COMPETITION_WINDOW ))	return;
	if ( !m_pCompetitionWindow )	return;

	ShowGroupFocus( PVP_CAPTURE_THE_FLAG_TOPRANKINGS );

	if ( m_pPVPCaptureTheFlagTopRankings )
	{
		UIRECT rect = m_pCompetitionWindow->GetGlobalPos();
		rect.left = rect.left + rect.sizeX;

		m_pPVPCaptureTheFlagTopRankings->SetGlobalPos( D3DXVECTOR2(rect.left, rect.top) );
	}
}

/*pvp capture the flag, Juver, 2018/02/02 */
void CInnerInterface::PVPCaptureTheFlagShowStartNotice()
{
	ShowGroupFocus( PVP_CAPTURE_THE_FLAG_BATTLE_START_NOTICE );
}

/*pvp capture the flag, Juver, 2018/02/02 */
void CInnerInterface::PVPCaptureTheFlagShowEndNotice()
{
	ShowGroupFocus( PVP_CAPTURE_THE_FLAG_BATTLE_END_NOTICE );
}

/*pvp capture the flag, Juver, 2018/02/02 */
void CInnerInterface::PVPCaptureTheFlagScoreResetPos()
{
	if ( !m_pPVPCaptureTheFlagScore )		return;
	if ( !m_pPVPCaptureTheFlagScoreDummy )	return;

	m_pPVPCaptureTheFlagScore->SetGlobalPos( D3DXVECTOR2( m_pPVPCaptureTheFlagScoreDummy->GetGlobalPos ().left, m_pPVPCaptureTheFlagScoreDummy->GetGlobalPos ().top ) );
}

/*pvp capture the flag, Juver, 2018/02/02 */
void CInnerInterface::PVPCaptureTheFlagShowRanking()
{
	if ( IsVisibleGroup( REBIRTH_DIALOGUE_CAPTURE_THE_FLAG ))
	{
		//character is dead show rank window later when revived!
		m_bPVPCaptureTheFlagShowRanking = true;
		return;
	}

	if ( !IsVisibleGroup( PVP_CAPTURE_THE_FLAG_RANKINGS )){
		SetDefaultPosInterface( PVP_CAPTURE_THE_FLAG_RANKINGS );
		ShowGroupFocus( PVP_CAPTURE_THE_FLAG_RANKINGS );
	}

	if ( m_pPVPCaptureTheFlagRankings ){
		m_pPVPCaptureTheFlagRankings->UpdateRankings();
	}

	m_bPVPCaptureTheFlagShowRanking = false;
}

void CInnerInterface::SecurityWindowResetPos()
{
	if ( !m_pSecurityWindow )		return;
	if ( !m_pSecurityWindowDummy )	return;

	m_pSecurityWindow->SetGlobalPos( 
		D3DXVECTOR2( m_pSecurityWindowDummy->GetGlobalPos ().left, m_pSecurityWindowDummy->GetGlobalPos ().top ) );
}

void CInnerInterface::ResetCartListInDialogueWindowMarketPage( int nPageIndex, int nPosX, int nPosY )
{
    if ( m_pDialogueWindowRenewal )
        m_pDialogueWindowRenewal->ResetCartListInDialogueWindowMarketPage( nPageIndex, nPosX, nPosY );
}

void CInnerInterface::GetDialogueWindowMarketItemIndex( int* pnPosX, int* pnPosY )
{
    if ( m_pDialogueWindowRenewal )
        m_pDialogueWindowRenewal->GetItemIndex( pnPosX, pnPosY );
}

int CInnerInterface::GetDialogueWindowPageIndex()
{
    if ( m_pDialogueWindowRenewal )
        return m_pDialogueWindowRenewal->GetPageIndex();

    return nOUTOFRANGE;
}

bool CInnerInterface::IsOpenDialogueWindowMarketPage()
{
    if ( !IsVisibleGroup( DIALOGUE_WINDOW_RENEWAL ) )
        return false;

    if ( m_pDialogueWindowRenewal )
        return m_pDialogueWindowRenewal->IsOpenMarketPage();

    return false;
}

void CInnerInterface::SetEnableDialogueWindowMarketPage(bool bEnable)
{
    if (m_pDialogueWindowRenewal)
        m_pDialogueWindowRenewal->SetEnableMarketPage(bEnable);
}