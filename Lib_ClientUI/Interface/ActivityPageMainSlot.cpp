#include "StdAfx.h"
#include "ActivityPageMainSlot.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "UITextControl.h"
#include "GameTextControl.h"

#include "GLActivity.h"
#include "GLGaeaClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CActivityPageMainSlot::CActivityPageMainSlot ()	
	: m_pStatusComplete(NULL)
	, m_pProgressComplete(NULL)
	, m_pTextComplete(NULL)
	, m_pTextTitle(NULL)
	, m_pTextDesc(NULL)
	, m_pTextProgress(NULL)
	, m_pImageBadge(NULL)
	, m_pImagePoint(NULL)
	, m_pTextBadge(NULL)
	, m_pTextPoint(NULL)
	, m_fUpdateTime(0.0f)
	, m_dwActivityID(UINT_MAX)
{
}

CActivityPageMainSlot::~CActivityPageMainSlot ()
{
}

void CActivityPageMainSlot::CreateSubControl ()
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_ACT_SLOT_ARRIVAL_BACK" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_ACT_SLOT_PRESENT_BACK" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_ACT_SLOT_REWARD_BACK" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_ACT_SLOT_REWARD_BACK_INNER1" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_ACT_SLOT_REWARD_BACK_INNER2" );
	RegisterControl ( pLineBox );

	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_SPEC_ACT_SLOT_PRESENT_BACK_TOP",  UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_SPEC_ACT_SLOT_REWARD_BACK_TOP",  UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_SPEC_ACT_SLOT_ALL",  UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	RegisterControl ( pControl );

	m_pStatusComplete = new CUIControl;
	m_pStatusComplete->CreateSub ( this, "RAN_SPEC_ACT_SLOT_ARRIVAL",  UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pStatusComplete->SetVisibleSingle( FALSE );
	RegisterControl ( m_pStatusComplete );

	m_pProgressComplete = new CUIControl;
	m_pProgressComplete->CreateSub ( this, "RAN_SPEC_ACT_SLOT_COMP_IMAGE",  UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pProgressComplete->SetVisibleSingle( FALSE );
	RegisterControl ( m_pProgressComplete );

	m_pImageBadge = new CUIControl;
	m_pImageBadge->CreateSub ( this, "RAN_SPEC_ACT_SLOT_REWARD_TITLE_IMAGE",  UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pImageBadge->SetVisibleSingle( TRUE );
	RegisterControl ( m_pImageBadge );

	m_pImagePoint = new CUIControl;
	m_pImagePoint->CreateSub ( this, "RAN_SPEC_ACT_SLOT_REWARD_POINT_IMAGE",  UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pImagePoint->SetVisibleSingle( TRUE );
	RegisterControl ( m_pImagePoint );

	m_pTextComplete = new CBasicTextBox;
	m_pTextComplete->CreateSub ( this, "RAN_SPEC_ACT_SLOT_COMP_TEXT" );
	m_pTextComplete->SetFont ( pFont );
	m_pTextComplete->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextComplete->SetText( "--" , NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextComplete );

	m_pTextTitle = new CBasicTextBox;
	m_pTextTitle->CreateSub ( this, "RAN_SPEC_ACT_SLOT_TITLE_TEXT" );
	m_pTextTitle->SetFont ( pFont );
	m_pTextTitle->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextTitle->SetText( "--" , NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextTitle );

	m_pTextDesc = new CBasicTextBox;
	m_pTextDesc->CreateSub ( this, "RAN_SPEC_ACT_SLOT_DESC_TEXT" );
	m_pTextDesc->SetFont ( pFont );
	m_pTextDesc->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextDesc->SetText( "--" , NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextDesc );

	m_pTextProgress = new CBasicTextBox;
	m_pTextProgress->CreateSub ( this, "RAN_SPEC_ACT_SLOT_PROGRESS_TEXT" );
	m_pTextProgress->SetFont ( pFont );
	m_pTextProgress->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextProgress->SetText( "--" , NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextProgress );

	m_pTextBadge = new CBasicTextBox;
	m_pTextBadge->CreateSub ( this, "RAN_SPEC_ACT_SLOT_REWARD_TITLE_TEXT" );
	m_pTextBadge->SetFont ( pFont );
	m_pTextBadge->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextBadge->SetText( "--" , NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextBadge );

	m_pTextPoint = new CBasicTextBox;
	m_pTextPoint->CreateSub ( this, "RAN_SPEC_ACT_SLOT_REWARD_POINT_TEXT" );
	m_pTextPoint->SetFont ( pFont );
	m_pTextPoint->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextPoint->SetText( "--" , NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextPoint );
}

void CActivityPageMainSlot::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );
}

void CActivityPageMainSlot::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_fUpdateTime += fElapsedTime;
	if ( m_fUpdateTime >= 0.2f )
	{
		UpdateInfo();
	}
}

void CActivityPageMainSlot::ResetData()
{
	m_fUpdateTime = 0.0f;
	m_dwActivityID = UINT_MAX;

	if ( m_pStatusComplete )	m_pStatusComplete->SetVisibleSingle( FALSE );
	if ( m_pProgressComplete )	m_pProgressComplete->SetVisibleSingle( FALSE );
	if ( m_pImageBadge )		m_pImageBadge->SetVisibleSingle( FALSE );
	if ( m_pImagePoint )		m_pImagePoint->SetVisibleSingle( FALSE );

	if ( m_pTextComplete )		m_pTextComplete->ClearText();
	if ( m_pTextTitle )			m_pTextTitle->ClearText();
	if ( m_pTextDesc )			m_pTextDesc->ClearText();
	if ( m_pTextProgress )		m_pTextProgress->ClearText();
	if ( m_pTextBadge )			m_pTextBadge->ClearText();
	if ( m_pTextPoint )			m_pTextPoint->ClearText();
}

void CActivityPageMainSlot::SetData( DWORD dwID )
{
	ResetData();

	m_dwActivityID = dwID;

	UpdateInfo();
}

void CActivityPageMainSlot::UpdateInfo()
{
	if ( m_pStatusComplete )	m_pStatusComplete->SetVisibleSingle( FALSE );
	if ( m_pProgressComplete )	m_pProgressComplete->SetVisibleSingle( FALSE );
	if ( m_pImageBadge )		m_pImageBadge->SetVisibleSingle( FALSE );
	if ( m_pImagePoint )		m_pImagePoint->SetVisibleSingle( FALSE );

	if ( m_pTextComplete )		m_pTextComplete->ClearText();
	if ( m_pTextTitle )			m_pTextTitle->ClearText();
	if ( m_pTextDesc )			m_pTextDesc->ClearText();
	if ( m_pTextProgress )		m_pTextProgress->ClearText();
	if ( m_pTextBadge )			m_pTextBadge->ClearText();
	if ( m_pTextPoint )			m_pTextPoint->ClearText();

	CString strTemp;

	SACTIVITY_FILE_DATA* pactivity_data = GLActivity::GetInstance().GetActivity( m_dwActivityID );
	if ( pactivity_data )
	{
		if ( m_pTextTitle )
		{
			strTemp.Format( "< %s >", pactivity_data->strActivityTitle.c_str() );
			m_pTextTitle->AddText( strTemp.GetString(), NS_UITEXTCOLOR::ORANGE );
		}

		if ( m_pTextDesc )			m_pTextDesc->AddText( pactivity_data->strDescription.c_str(), NS_UITEXTCOLOR::AQUA );

		if ( pactivity_data->bRewardBadge && pactivity_data->strBadgeString.size() )
		{
			if ( m_pImageBadge )		m_pImageBadge->SetVisibleSingle( TRUE );
			if ( m_pTextBadge )			m_pTextBadge->AddText( pactivity_data->strBadgeString.c_str(), NS_UITEXTCOLOR::WHITE );
		}

		if ( pactivity_data->dwRewardPoint )
		{
			strTemp.Format( "%u", pactivity_data->dwRewardPoint );
			if ( m_pImagePoint )		m_pImagePoint->SetVisibleSingle( TRUE );
			if ( m_pTextPoint )			m_pTextPoint->AddText( strTemp.GetString(), NS_UITEXTCOLOR::GOLD );
		}


		SACTIVITY_CHAR_DATA* pchar_activity_data_done = GLGaeaClient::GetInstance().GetCharacter()->GetActivityDone( m_dwActivityID );
		if ( pchar_activity_data_done )
		{
			if ( m_pStatusComplete )	m_pStatusComplete->SetVisibleSingle( TRUE );
			if ( m_pProgressComplete )	m_pProgressComplete->SetVisibleSingle( TRUE );

			if ( m_pTextProgress )		m_pTextProgress->AddText( ID2GAMEWORD( "RAN_SPEC_ACT_PROGRESS_TEXT", 1 ), NS_UITEXTCOLOR::WHITE );

			return;
		}

		SACTIVITY_CHAR_DATA* pchar_activity_data_prog = GLGaeaClient::GetInstance().GetCharacter()->GetActivityProg( m_dwActivityID );
		if ( pchar_activity_data_prog )
		{
			strTemp.Format( "%s %u/%u", ID2GAMEWORD( "RAN_SPEC_ACT_PROGRESS_TEXT", 0 ), pchar_activity_data_prog->dwProgressNow, pchar_activity_data_prog->dwProgressMax );

			if ( m_pTextProgress )		m_pTextProgress->AddText( strTemp.GetString(), NS_UITEXTCOLOR::WHITE );
			if ( m_pTextComplete )		m_pTextComplete->AddText( ID2GAMEWORD( "RAN_SPEC_ACT_PROGRESS_TEXT", 2 ), NS_UITEXTCOLOR::AQUA );
		}
	}
}