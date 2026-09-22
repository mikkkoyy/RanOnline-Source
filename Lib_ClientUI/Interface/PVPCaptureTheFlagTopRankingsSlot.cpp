#include "StdAfx.h"
#include "PVPCaptureTheFlagTopRankingsSlot.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "GameTextControl.h"
#include "UITextControl.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPCaptureTheFlagTopRankingsSlot::CPVPCaptureTheFlagTopRankingsSlot ()
	: m_pTextRank(NULL)
	, m_pTextName(NULL)
	, m_pImageWin(NULL)
	, m_pImageLose(NULL)
{
	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		m_pTeamImage[i] = NULL;

	for( int i=0; i<CAPTURE_THE_FLAG_INFO_SCHOOL_SIZE; ++i )
		m_pSchoolImage[i] = NULL;

	for( int i=0; i<GLCI_NUM_8CLASS; ++i )
		m_pClassImage[i] = NULL;
}

CPVPCaptureTheFlagTopRankingsSlot::~CPVPCaptureTheFlagTopRankingsSlot ()
{
}

void CPVPCaptureTheFlagTopRankingsSlot::CreateSubControl ()
{
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_FLAG );

	m_pTextRank = new CBasicTextBox;
	m_pTextRank->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_RANK" );
	m_pTextRank->SetFont ( pFont8 );
	m_pTextRank->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextRank->SetText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextRank );

	m_pTextName = new CBasicTextBox;
	m_pTextName->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_TEXT_BASE_NAME" );
	m_pTextName->SetFont ( pFont8 );
	m_pTextName->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextName->SetText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextName );

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

	m_pImageWin = new CUIControl;
	m_pImageWin->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_WIN", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pImageWin->SetVisibleSingle( FALSE );
	RegisterControl( m_pImageWin );

	m_pImageLose = new CUIControl;
	m_pImageLose->CreateSub ( this, "CAPTURE_THE_FLAG_RESULT_RANKSLOT_IMAGE_LOSE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pImageLose->SetVisibleSingle( FALSE );
	RegisterControl( m_pImageLose );


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
}

void CPVPCaptureTheFlagTopRankingsSlot::ResetData()
{
	if ( m_pTextRank )		m_pTextRank->ClearText();
	if ( m_pTextName )		m_pTextName->ClearText();

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

void CPVPCaptureTheFlagTopRankingsSlot::SetData( CAPTURE_THE_FLAG_PLAYER_DATA* pData, WORD wTeamWinner )
{
	ResetData();

	if ( !pData )	return;
	CString strText;

	if ( m_pTextRank )
	{
		strText.Format( "%u", pData->wRankAll );
		m_pTextRank->AddText( strText.GetString(), NS_UITEXTCOLOR::WHITE );
	}

	if ( m_pTextName )
	{
		strText.Format( "%s", pData->szInfoCharName );
		m_pTextName->AddText( strText.GetString(), NS_UITEXTCOLOR::WHITE );
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