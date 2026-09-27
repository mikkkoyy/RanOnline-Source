#include "StdAfx.h"
#include "PVPCaptureTheFlagRankingPageSlot.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"

#include "BasicTextBox.h"
#include "GameTextControl.h"
#include "UITextControl.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPCaptureTheFlagRankingPageSlot::CPVPCaptureTheFlagRankingPageSlot ()
	: m_pTextRank(NULL)
	, m_pTextName(NULL)
	, m_pTextFlag(NULL)
	, m_pTextKill(NULL)
	, m_pTextDeath(NULL)
	, m_pTextResu(NULL)
	, m_pTextHeal(NULL)
	, m_pTextScore(NULL)
	, m_pTextPoint(NULL)
	, m_pImageWin(NULL)
	, m_pImageLose(NULL)
	, m_bSelf(FALSE)
{
	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		m_pTeamImage[i] = NULL;

	for( int i=0; i<CAPTURE_THE_FLAG_INFO_SCHOOL_SIZE; ++i )
		m_pSchoolImage[i] = NULL;

	for( int i=0; i<GLCI_NUM_8CLASS; ++i )
		m_pClassImage[i] = NULL;
}

CPVPCaptureTheFlagRankingPageSlot::~CPVPCaptureTheFlagRankingPageSlot ()
{
}

void CPVPCaptureTheFlagRankingPageSlot::CreateSubControl( BOOL bSelf )
{
	m_bSelf = bSelf;

	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG );

	DWORD dwTextColor = NS_UITEXTCOLOR::WHITE;
	if ( m_bSelf )	dwTextColor = NS_UITEXTCOLOR::GREENYELLOW;

	m_pTextRank = new CBasicTextBox;
	m_pTextRank->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_RANK" );
	m_pTextRank->SetFont ( pFont8 );
	m_pTextRank->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextRank->SetText( "--", dwTextColor );
	RegisterControl ( m_pTextRank );

	m_pTextName = new CBasicTextBox;
	m_pTextName->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_NAME" );
	m_pTextName->SetFont ( pFont8 );
	m_pTextName->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextName->SetText( "--", dwTextColor );
	RegisterControl ( m_pTextName );

	m_pTextFlag = new CBasicTextBox;
	m_pTextFlag->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_FLAG" );
	m_pTextFlag->SetFont ( pFont8 );
	m_pTextFlag->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextFlag->SetText( "--", dwTextColor );
	RegisterControl ( m_pTextFlag );

	m_pTextKill = new CBasicTextBox;
	m_pTextKill->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_KILL" );
	m_pTextKill->SetFont ( pFont8 );
	m_pTextKill->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextKill->SetText( "--", dwTextColor );
	RegisterControl ( m_pTextKill );

	m_pTextDeath = new CBasicTextBox;
	m_pTextDeath->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_DEATH" );
	m_pTextDeath->SetFont ( pFont8 );
	m_pTextDeath->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextDeath->SetText( "--", dwTextColor );
	RegisterControl ( m_pTextDeath );

	m_pTextResu = new CBasicTextBox;
	m_pTextResu->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_REBIRTH" );
	m_pTextResu->SetFont ( pFont8 );
	m_pTextResu->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextResu->SetText( "--", dwTextColor );
	RegisterControl ( m_pTextResu );

	m_pTextHeal = new CBasicTextBox;
	m_pTextHeal->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_HEAL" );
	m_pTextHeal->SetFont ( pFont8 );
	m_pTextHeal->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextHeal->SetText( "--", dwTextColor );
	RegisterControl ( m_pTextHeal );

	m_pTextScore = new CBasicTextBox;
	m_pTextScore->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_SCORE" );
	m_pTextScore->SetFont ( pFont8 );
	m_pTextScore->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextScore->SetText( "--", dwTextColor );
	RegisterControl ( m_pTextScore );

	m_pTextPoint = new CBasicTextBox;
	m_pTextPoint->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_CONTRIBUTION" );
	m_pTextPoint->SetFont ( pFont8 );
	m_pTextPoint->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextPoint->SetText( "--", dwTextColor );
	RegisterControl ( m_pTextPoint );

	std::string strteam_image[CAPTURE_THE_FLAG_TEAM_SIZE] = 
	{
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_TEAM0",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_TEAM1",
	};

	std::string strschool_image[CAPTURE_THE_FLAG_INFO_SCHOOL_SIZE] = 
	{
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_SCHOOL0",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_SCHOOL1",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_SCHOOL2",
	};

	std::string strclass_image[GLCI_NUM_8CLASS] = 
	{
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_BRAWLER_MALE",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_SWORDMAN_MALE",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_ARCHER_FEMALE",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_SHAMAN_FEMALE",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_EXTREME_MALE",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_EXTREME_FEMALE",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_BRAWLER_FEMALE",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_SWORDMAN_FEMALE",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_ARCHER_MALE",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_SHAMAN_MALE",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_SCIENTIST_MALE",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_SCIENTIST_FEMALE",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_ASSASSIN_MALE",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_ASSASSIN_FEMALE",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_TRICKER_MALE",
		"CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_TRICKER_FEMALE",
	};

	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
	{
		m_pTeamImage[i] = new CUIControl;
		m_pTeamImage[i]->CreateSub ( this, strteam_image[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		m_pTeamImage[i]->SetVisibleSingle( FALSE );
		RegisterControl ( m_pTeamImage[i] );
	}

	for( int i=0; i<CAPTURE_THE_FLAG_INFO_SCHOOL_SIZE; ++i )
	{
		m_pSchoolImage[i] = new CUIControl;
		m_pSchoolImage[i]->CreateSub ( this, strschool_image[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		m_pSchoolImage[i]->SetVisibleSingle( FALSE );
		RegisterControl ( m_pSchoolImage[i] );
	}

	for( int i=0; i<GLCI_NUM_8CLASS; ++i )
	{
		m_pClassImage[i] = new CUIControl;
		m_pClassImage[i]->CreateSub ( this, strclass_image[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		m_pClassImage[i]->SetVisibleSingle( FALSE );
		RegisterControl ( m_pClassImage[i] );
	}

	m_pImageWin = new CUIControl;
	m_pImageWin->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_WIN", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pImageWin->SetVisibleSingle( FALSE );
	RegisterControl( m_pImageWin );

	m_pImageLose = new CUIControl;
	m_pImageLose->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_LOSE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pImageLose->SetVisibleSingle( FALSE );
	RegisterControl( m_pImageLose );

}

void CPVPCaptureTheFlagRankingPageSlot::ResetData()
{
	if ( m_pTextRank )		m_pTextRank->ClearText();
	if ( m_pTextName )		m_pTextName->ClearText();
	if ( m_pTextFlag )		m_pTextFlag->ClearText();
	if ( m_pTextKill )		m_pTextKill->ClearText();
	if ( m_pTextDeath )		m_pTextDeath->ClearText();
	if ( m_pTextResu )		m_pTextResu->ClearText();
	if ( m_pTextHeal )		m_pTextHeal->ClearText();
	if ( m_pTextScore )		m_pTextScore->ClearText();
	if ( m_pTextPoint )		m_pTextPoint->ClearText();

	if ( m_pImageWin )	m_pImageWin->SetVisibleSingle( FALSE );
	if ( m_pImageLose )	m_pImageLose->SetVisibleSingle( FALSE );

	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
	{
		if ( m_pTeamImage[i] )
			m_pTeamImage[i]->SetVisibleSingle( FALSE );
	}

	for( int i=0; i<CAPTURE_THE_FLAG_INFO_SCHOOL_SIZE; ++i )
	{
		if ( m_pSchoolImage[i] )
			m_pSchoolImage[i]->SetVisibleSingle( FALSE );
	}

	for( int i=0; i<GLCI_NUM_8CLASS; ++i )
	{
		if ( m_pClassImage[i] )
			m_pClassImage[i]->SetVisibleSingle( FALSE );
	}
}

void CPVPCaptureTheFlagRankingPageSlot::SetData( CAPTURE_THE_FLAG_PLAYER_DATA* pData, BOOL bTotal, WORD wTeamWinner  )
{
	ResetData();

	if ( !pData )	return;
	CString strText;

	DWORD dwTextColor = NS_UITEXTCOLOR::WHITE;
	if ( m_bSelf )	dwTextColor = NS_UITEXTCOLOR::GREENYELLOW;

	if ( m_pTextRank )
	{
		WORD wRankNum = (bTotal)? pData->wRankAll:pData->wRankTeam;
		strText.Format( "%u", wRankNum );
		m_pTextRank->AddText( strText.GetString(), dwTextColor );
	}

	if ( m_pTextName )
	{
		strText.Format( "%s", pData->szInfoCharName );
		m_pTextName->AddText( strText.GetString(), dwTextColor );
	}

	if ( m_pTextHeal )
	{
		strText.Format( "%u", pData->dwScoreHeal );
		m_pTextHeal->AddText( strText.GetString(), dwTextColor );
	}

	if ( m_pTextFlag )
	{
		strText.Format( "%u", pData->wScoreFlag );
		m_pTextFlag->AddText( strText.GetString(), dwTextColor );
	}

	if ( m_pTextKill )
	{
		strText.Format( "%u", pData->wScoreKill );
		m_pTextKill->AddText( strText.GetString(), dwTextColor );
	}

	if ( m_pTextDeath )
	{
		strText.Format( "%u", pData->wScoreDeath );
		m_pTextDeath->AddText( strText.GetString(), dwTextColor );
	}

	if ( m_pTextResu )
	{
		strText.Format( "%u", pData->wScoreResu );
		m_pTextResu->AddText( strText.GetString(), dwTextColor );
	}

	if ( m_pTextScore )
	{
		strText.Format( "%u", pData->dwScoreTotal );
		m_pTextScore->AddText( strText.GetString(), dwTextColor );
	}

	if ( m_pTextPoint )
	{
		strText.Format( "+%u", pData->wRewardPoint );
		m_pTextPoint->AddText( strText.GetString(), dwTextColor );
	}

	if ( pData->wInfoTeam < CAPTURE_THE_FLAG_TEAM_SIZE )
	{
		if ( m_pTeamImage[pData->wInfoTeam] )
			m_pTeamImage[pData->wInfoTeam]->SetVisibleSingle( TRUE );
	}

	if ( pData->wInfoSchool < CAPTURE_THE_FLAG_INFO_SCHOOL_SIZE )
	{
		if ( m_pSchoolImage[pData->wInfoSchool] )
			m_pSchoolImage[pData->wInfoSchool]->SetVisibleSingle( TRUE );
	}
	
	if ( pData->wInfoClass < GLCI_NUM_8CLASS )
	{
		if ( m_pClassImage[pData->wInfoClass] )
			m_pClassImage[pData->wInfoClass]->SetVisibleSingle( TRUE );
	}

	if ( m_pImageWin )	
		m_pImageWin->SetVisibleSingle( wTeamWinner == pData->wInfoTeam && wTeamWinner < CAPTURE_THE_FLAG_TEAM_SIZE );

	if ( m_pImageLose )	
		m_pImageLose->SetVisibleSingle( wTeamWinner != pData->wInfoTeam && wTeamWinner < CAPTURE_THE_FLAG_TEAM_SIZE );
}