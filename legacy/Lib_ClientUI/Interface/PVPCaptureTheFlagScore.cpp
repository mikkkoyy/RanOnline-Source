#include "StdAfx.h"
#include "PVPCaptureTheFlagScore.h"
#include "BasicTextBox.h"
#include "d3dfont.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "GLPVPCaptureTheFlagClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPCaptureTheFlagScore::CPVPCaptureTheFlagScore ()
	: m_bCHECK_MOUSE_STATE(FALSE)
	, m_bFirstGap(FALSE)
	, m_nPosX(0)
	, m_nPosY(0)
	, m_fUpdateTime(0.0f)
	, m_fImageTime(0.0f)
	, m_fBLINK_TIME_A(0.0f)
	, m_fBLINK_TIME_B(0.0f)

	, m_pTextTeamLeft(NULL)
	, m_pTextTeamRight(NULL)
	, m_pTextScoreLeft(NULL)
	, m_pTextScoreRight(NULL)
	, m_pTextMaxScore(NULL)
	, m_pTextTimer(NULL)

	, m_pImageWinLeft(NULL)
	, m_pImageLoseLeft(NULL)
	, m_pImageWinRight(NULL)
	, m_pImageLoseRight(NULL)

	, m_pImageFlagLeft(NULL)
	, m_pImageFlagRight(NULL)
{
}

CPVPCaptureTheFlagScore::~CPVPCaptureTheFlagScore ()
{
}

void CPVPCaptureTheFlagScore::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );
	CD3DFontPar* pFont16 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 16, _DEFAULT_FONT_SHADOW_FLAG );

	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, "PVP_CAPTURE_THE_FLAG_SCORE_BG_LEFT", UI_FLAG_DEFAULT, CAPTURE_THE_FLAG_STATUS_BOX1 );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "PVP_CAPTURE_THE_FLAG_SCORE_BG_RIGHT", UI_FLAG_DEFAULT, CAPTURE_THE_FLAG_STATUS_BOX2 );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "PVP_CAPTURE_THE_FLAG_SCORE_ICON_RED", UI_FLAG_DEFAULT );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "PVP_CAPTURE_THE_FLAG_SCORE_ICON_BLUE", UI_FLAG_DEFAULT );
	RegisterControl ( pControl );

	m_pImageFlagLeft = new CUIControl;
	m_pImageFlagLeft->CreateSub ( this, "PVP_CAPTURE_THE_FLAG_SCORE_FLAG_RED", UI_FLAG_DEFAULT, CAPTURE_THE_FLAG_STATUS_BOX3 );
	RegisterControl ( m_pImageFlagLeft );

	m_pImageFlagRight = new CUIControl;
	m_pImageFlagRight->CreateSub ( this, "PVP_CAPTURE_THE_FLAG_SCORE_FLAG_BLUE", UI_FLAG_DEFAULT, CAPTURE_THE_FLAG_STATUS_BOX4 );
	RegisterControl ( m_pImageFlagRight );

	m_pTextTeamLeft = new CBasicTextBox;
	m_pTextTeamLeft->CreateSub ( this, "PVP_CAPTURE_THE_FLAG_SCORE_TEXT_TEAM_LEFT" );
	m_pTextTeamLeft->SetFont ( pFont9 );
	m_pTextTeamLeft->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	RegisterControl ( m_pTextTeamLeft );

	m_pTextTeamRight = new CBasicTextBox;
	m_pTextTeamRight->CreateSub ( this, "PVP_CAPTURE_THE_FLAG_SCORE_TEXT_TEAM_RIGHT" );
	m_pTextTeamRight->SetFont ( pFont9 );
	m_pTextTeamRight->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	RegisterControl ( m_pTextTeamRight );

	m_pTextScoreLeft = new CBasicTextBox;
	m_pTextScoreLeft->CreateSub ( this, "PVP_CAPTURE_THE_FLAG_SCORE_TEXT_SCORE_LEFT" );
	m_pTextScoreLeft->SetFont ( pFont9 );
	m_pTextScoreLeft->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	RegisterControl ( m_pTextScoreLeft );

	m_pTextScoreRight = new CBasicTextBox;
	m_pTextScoreRight->CreateSub ( this, "PVP_CAPTURE_THE_FLAG_SCORE_TEXT_SCORE_RIGHT" );
	m_pTextScoreRight->SetFont ( pFont9 );
	m_pTextScoreRight->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	RegisterControl ( m_pTextScoreRight );

	m_pTextMaxScore = new CBasicTextBox;
	m_pTextMaxScore->CreateSub ( this, "PVP_CAPTURE_THE_FLAG_SCORE_TEXT_MAXSCORE" );
	m_pTextMaxScore->SetFont ( pFont9 );
	m_pTextMaxScore->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	RegisterControl ( m_pTextMaxScore );

	m_pTextTimer = new CBasicTextBox;
	m_pTextTimer->CreateSub ( this, "PVP_CAPTURE_THE_FLAG_SCORE_TEXT_TIMER" );
	m_pTextTimer->SetFont ( pFont16 );
	m_pTextTimer->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	RegisterControl ( m_pTextTimer );

	m_pImageWinLeft = new CUIControl;
	m_pImageWinLeft->CreateSub ( this, "PVP_CAPTURE_THE_FLAG_SCORE_ICON_WIN_LEFT", UI_FLAG_DEFAULT );
	RegisterControl ( m_pImageWinLeft );
	m_pImageWinLeft->SetVisibleSingle( FALSE );

	m_pImageLoseLeft = new CUIControl;
	m_pImageLoseLeft->CreateSub ( this, "PVP_CAPTURE_THE_FLAG_SCORE_ICON_LOSE_LEFT", UI_FLAG_DEFAULT );
	RegisterControl ( m_pImageLoseLeft );
	m_pImageLoseLeft->SetVisibleSingle( FALSE );

	m_pImageWinRight = new CUIControl;
	m_pImageWinRight->CreateSub ( this, "PVP_CAPTURE_THE_FLAG_SCORE_ICON_WIN_RIGHT", UI_FLAG_DEFAULT );
	RegisterControl ( m_pImageWinRight );
	m_pImageWinRight->SetVisibleSingle( FALSE );

	m_pImageLoseRight = new CUIControl;
	m_pImageLoseRight->CreateSub ( this, "PVP_CAPTURE_THE_FLAG_SCORE_ICON_LOSE_RIGHT", UI_FLAG_DEFAULT );
	RegisterControl ( m_pImageLoseRight );
	m_pImageLoseRight->SetVisibleSingle( FALSE );
}

void CPVPCaptureTheFlagScore::CheckMouseState ()
{
	const DWORD dwMsg = GetMessageEx ();
	if ( CHECK_MOUSE_IN ( dwMsg ) )
	{
		if( UIMSG_LB_DUP & dwMsg )
		{
			AddMessageEx( UIMSG_CAPTURE_THE_FLAG_SCORE_WINDOW_LBDUP );
			SetDiffuse ( D3DCOLOR_ARGB ( 0xFF, 0xFF, 0xFF, 0xFF ) );
			return ;
		}

		if ( dwMsg & UIMSG_LB_DOWN )
		{
			SetExclusiveControl();	

			if ( !m_bFirstGap )
			{
				UIRECT rcPos = GetGlobalPos ();
				m_vGap.x = m_nPosX - rcPos.left;
				m_vGap.y = m_nPosY - rcPos.top;
				m_bFirstGap = TRUE;

				SetDiffuse ( D3DCOLOR_ARGB ( 0xB4, 0xFF, 0xFF, 0xFF ) );
			}
		}
		else if ( CHECK_LB_UP_LIKE ( dwMsg ) )
		{
			ResetExclusiveControl();
			m_bFirstGap = FALSE;				

			SetDiffuse ( D3DCOLOR_ARGB ( 0xFF, 0xFF, 0xFF, 0xFF ) );
		}
	}
	else if ( CHECK_LB_UP_LIKE ( dwMsg ) )		
	{								
		ResetExclusiveControl();
		m_bFirstGap = FALSE;					

		SetDiffuse ( D3DCOLOR_ARGB ( 0xFF, 0xFF, 0xFF, 0xFF ) );
	}
}

void CPVPCaptureTheFlagScore::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{	
	m_bCHECK_MOUSE_STATE = false;
	m_nPosX = x;
	m_nPosY = y;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( IsExclusiveSelfControl() ){
		SetGlobalPos ( D3DXVECTOR2 ( x - m_vGap.x, y - m_vGap.y ) );
	}

	m_fUpdateTime += fElapsedTime;
	if ( m_fUpdateTime >= 0.5f ){
		m_fUpdateTime = 0.0f;

		UpdateInfo();
	}

	GLPVPCaptureTheFlagClient* pClient = &GLPVPCaptureTheFlagClient::GetInstance();
	if ( pClient )
	{
		if ( m_pImageFlagLeft )
		{
			if ( pClient->m_bFlagHold[CAPTURE_THE_FLAG_TEAM_B] )
			{
				m_fBLINK_TIME_A += fElapsedTime;
				if ( 0.5 <= m_fBLINK_TIME_A )
				{
					BOOL bVisible = m_pImageFlagLeft->IsVisible ();
					m_pImageFlagLeft->SetVisibleSingle ( !bVisible );
					m_fBLINK_TIME_A = 0.0f;
				}
			}else{
				m_fBLINK_TIME_A = 0.0f;
				m_pImageFlagLeft->SetVisibleSingle( TRUE );
			}
		}
		
		if ( m_pImageFlagRight )
		{
			if ( pClient->m_bFlagHold[CAPTURE_THE_FLAG_TEAM_A] )
			{
				m_fBLINK_TIME_B += fElapsedTime;
				if ( 0.5 <= m_fBLINK_TIME_B )
				{
					BOOL bVisible = m_pImageFlagRight->IsVisible ();
					m_pImageFlagRight->SetVisibleSingle ( !bVisible );
					m_fBLINK_TIME_B = 0.0f;
				}
			}else{
				m_fBLINK_TIME_B = 0.0f;
				m_pImageFlagRight->SetVisibleSingle( TRUE );
			}
		}
	}
}

void CPVPCaptureTheFlagScore::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	if ( !m_bCHECK_MOUSE_STATE )
	{
		CheckMouseState ();
		m_bCHECK_MOUSE_STATE = true;
	}
}

void CPVPCaptureTheFlagScore::UpdateInfo()
{
	GLPVPCaptureTheFlagClient* pClient = &GLPVPCaptureTheFlagClient::GetInstance();
	if ( !pClient )	return;

	if ( m_pTextTimer )
		m_pTextTimer->ClearText();

	if ( pClient->m_emState == CAPTURE_THE_FLAG_STATE_BATTLE )
	{
		if ( m_pTextTimer )
		{
			if ( pClient->m_fRemain > 0.0f )
			{
				DWORD dwTextColor = NS_UITEXTCOLOR::WHITE;
				if ( pClient->m_fRemain < 300.0f ){
					dwTextColor = NS_UITEXTCOLOR::RED;
				}
				CString strCombine;
				strCombine.Format ( "%02d:%02d", (DWORD)pClient->m_fRemain/60, (DWORD)pClient->m_fRemain%60 );
				m_pTextTimer->AddText( strCombine.GetString(), dwTextColor );
			}
		}
	}

	CString strTemp;

	if ( m_pTextMaxScore )
	{
		m_pTextMaxScore->ClearText();
		strTemp.Format ( ID2GAMEWORD( "CAPTURE_THE_FLAG_SCORE_TEXT", 0 ), pClient->m_wMaxScore );
		m_pTextMaxScore->AddText( strTemp.GetString(), NS_UITEXTCOLOR::WHITE );
	}

	if ( m_pTextTeamLeft )
	{
		m_pTextTeamLeft->ClearText();
		if ( pClient->m_wTeam == CAPTURE_THE_FLAG_TEAM_A )
			m_pTextTeamLeft->AddText( ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_TEAM_TEXT", 0 ), NS_UITEXTCOLOR::GREENYELLOW );
		else
			m_pTextTeamLeft->AddText( ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_TEAM_TEXT", 0 ), NS_UITEXTCOLOR::WHITE );
	}

	if ( m_pTextTeamRight )
	{
		m_pTextTeamRight->ClearText();
		if ( pClient->m_wTeam == CAPTURE_THE_FLAG_TEAM_B )
			m_pTextTeamRight->AddText( ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_TEAM_TEXT", 1 ), NS_UITEXTCOLOR::GREENYELLOW );
		else
			m_pTextTeamRight->AddText( ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_TEAM_TEXT", 1 ), NS_UITEXTCOLOR::WHITE );
	}

	if ( m_pImageWinLeft )	
		m_pImageWinLeft->SetVisibleSingle( pClient->m_wLastWinner == CAPTURE_THE_FLAG_TEAM_A && pClient->m_wLastWinner < CAPTURE_THE_FLAG_TEAM_SIZE );

	if ( m_pImageLoseLeft )	
		m_pImageLoseLeft->SetVisibleSingle( pClient->m_wLastWinner == CAPTURE_THE_FLAG_TEAM_B && pClient->m_wLastWinner < CAPTURE_THE_FLAG_TEAM_SIZE );

	if ( m_pImageWinRight )	
		m_pImageWinRight->SetVisibleSingle( pClient->m_wLastWinner == CAPTURE_THE_FLAG_TEAM_B && pClient->m_wLastWinner < CAPTURE_THE_FLAG_TEAM_SIZE );

	if ( m_pImageLoseRight )	
		m_pImageLoseRight->SetVisibleSingle( pClient->m_wLastWinner == CAPTURE_THE_FLAG_TEAM_A && pClient->m_wLastWinner < CAPTURE_THE_FLAG_TEAM_SIZE );


	if ( m_pTextScoreLeft )
	{
		m_pTextScoreLeft->ClearText();
		strTemp.Format( ID2GAMEWORD( "CAPTURE_THE_FLAG_SCORE_TEXT", 1 ), pClient->m_sScore[CAPTURE_THE_FLAG_TEAM_A].wCapture );
		m_pTextScoreLeft->AddText( strTemp.GetString(), NS_UITEXTCOLOR::WHITE );
	}

	if ( m_pTextScoreRight )
	{
		m_pTextScoreRight->ClearText();
		strTemp.Format( ID2GAMEWORD( "CAPTURE_THE_FLAG_SCORE_TEXT", 1 ), pClient->m_sScore[CAPTURE_THE_FLAG_TEAM_B].wCapture );
		m_pTextScoreRight->AddText( strTemp.GetString(), NS_UITEXTCOLOR::WHITE );
	}
}

