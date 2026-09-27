#include "StdAfx.h"
#include "PVPTyrannyTowerCapture.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "d3dfont.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "GLPVPTyrannyClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPTyrannyTowerCapture::CPVPTyrannyTowerCapture ()
	: m_bCHECK_MOUSE_STATE(FALSE)
	, m_bFirstGap(FALSE)
	, m_nPosX(0)
	, m_nPosY(0)
	, m_fUpdateTime(0.0f)
	, m_pTextTower0(NULL)
	, m_pTextTower1(NULL)
	, m_pTextTower2(NULL)
	, m_pTextStatus(NULL)
	, m_pTextTimer(NULL)
{
	for( int i=0; i<TOWER_STATUS_TYPES; ++i )
	{
		m_pStatusTower0[i] = NULL;
		m_pStatusTower1[i] = NULL;
		m_pStatusTower2[i] = NULL;
		m_pStatusWinner[i] = NULL;
	}
}

CPVPTyrannyTowerCapture::~CPVPTyrannyTowerCapture ()
{
}

void CPVPTyrannyTowerCapture::CreateSubControl ()
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, "TYRANNY_TOWER_CAPTURE_BG1", UI_FLAG_DEFAULT, TOWER_STATUS_BOX1 );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "TYRANNY_TOWER_CAPTURE_BG2", UI_FLAG_DEFAULT, TOWER_STATUS_BOX2 );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "TYRANNY_TOWER_CAPTURE_DEVIDER_1" );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "TYRANNY_TOWER_CAPTURE_DEVIDER_2" );
	RegisterControl ( pControl );

	m_pTextTower0 = new CBasicTextBox;
	m_pTextTower0->CreateSub ( this, "TYRANNY_TOWER_CAPTURE_TEXT_0" );
	m_pTextTower0->SetFont ( pFont );
	m_pTextTower0->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextTower0->AddText( ID2GAMEWORD("TYRANNY_TOWER_TEXT", 0 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextTower0 );

	m_pTextTower1 = new CBasicTextBox;
	m_pTextTower1->CreateSub ( this, "TYRANNY_TOWER_CAPTURE_TEXT_1" );
	m_pTextTower1->SetFont ( pFont );
	m_pTextTower1->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextTower1->AddText( ID2GAMEWORD("TYRANNY_TOWER_TEXT", 1 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextTower1 );

	m_pTextTower2 = new CBasicTextBox;
	m_pTextTower2->CreateSub ( this, "TYRANNY_TOWER_CAPTURE_TEXT_2" );
	m_pTextTower2->SetFont ( pFont );
	m_pTextTower2->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextTower2->AddText( ID2GAMEWORD("TYRANNY_TOWER_TEXT", 2 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextTower2 );

	m_pTextStatus = new CBasicTextBox;
	m_pTextStatus->CreateSub ( this, "TYRANNY_TOWER_CAPTURE_STATUS_TEXT" );
	m_pTextStatus->SetFont ( pFont );
	m_pTextStatus->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextStatus->AddText( ID2GAMEINTEXT("TYRANNY_TOWER_STATUS_BATTLE"), NS_UITEXTCOLOR::ORANGE );
	RegisterControl ( m_pTextStatus );

	m_pTextTimer = new CBasicTextBox;
	m_pTextTimer->CreateSub ( this, "TYRANNY_TOWER_CAPTURE_TIME_TEXT" );
	m_pTextTimer->SetFont ( pFont );
	m_pTextTimer->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextTimer->AddText( "FF:FF", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextTimer );

	std::string strtowerinfo0[TOWER_STATUS_TYPES] = 
	{
		"TYRANNY_TOWER_CAPTURE_IMAGE_SG_0",
		"TYRANNY_TOWER_CAPTURE_IMAGE_MP_0",
		"TYRANNY_TOWER_CAPTURE_IMAGE_PHX_0",
		"TYRANNY_TOWER_CAPTURE_IMAGE_NONE_0",
	};

	std::string strtowerinfo1[TOWER_STATUS_TYPES] = 
	{
		"TYRANNY_TOWER_CAPTURE_IMAGE_SG_1",
		"TYRANNY_TOWER_CAPTURE_IMAGE_MP_1",
		"TYRANNY_TOWER_CAPTURE_IMAGE_PHX_1",
		"TYRANNY_TOWER_CAPTURE_IMAGE_NONE_1",
	};

	std::string strtowerinfo2[TOWER_STATUS_TYPES] = 
	{
		"TYRANNY_TOWER_CAPTURE_IMAGE_SG_2",
		"TYRANNY_TOWER_CAPTURE_IMAGE_MP_2",
		"TYRANNY_TOWER_CAPTURE_IMAGE_PHX_2",
		"TYRANNY_TOWER_CAPTURE_IMAGE_NONE_2",
	};

	std::string strstatuswinner[TOWER_STATUS_TYPES] = 
	{
		"TYRANNY_TOWER_WINNER_IMAGE_SG",
		"TYRANNY_TOWER_WINNER_IMAGE_MP",
		"TYRANNY_TOWER_WINNER_IMAGE_PHX",
		"TYRANNY_TOWER_WINNER_IMAGE_NONE",
	};

	for( int i=0; i<TOWER_STATUS_TYPES; ++i )
	{
		m_pStatusTower0[i] = new CUIControl;
		m_pStatusTower0[i]->CreateSub ( this, strtowerinfo0[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		m_pStatusTower0[i]->SetVisibleSingle( i == TYRANNY_TOWER_SIZE );
		RegisterControl ( m_pStatusTower0[i] );

		m_pStatusTower1[i] = new CUIControl;
		m_pStatusTower1[i]->CreateSub ( this, strtowerinfo1[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		m_pStatusTower1[i]->SetVisibleSingle( i == TYRANNY_TOWER_SIZE );
		RegisterControl ( m_pStatusTower1[i] );

		m_pStatusTower2[i] = new CUIControl;
		m_pStatusTower2[i]->CreateSub ( this, strtowerinfo2[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		m_pStatusTower2[i]->SetVisibleSingle( i == TYRANNY_TOWER_SIZE );
		RegisterControl ( m_pStatusTower2[i] );

		m_pStatusWinner[i] = new CUIControl;
		m_pStatusWinner[i]->CreateSub ( this, strstatuswinner[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		m_pStatusWinner[i]->SetVisibleSingle( FALSE );
		RegisterControl ( m_pStatusWinner[i] );
	}
}

void CPVPTyrannyTowerCapture::CheckMouseState ()
{
	const DWORD dwMsg = GetMessageEx ();
	if ( CHECK_MOUSE_IN ( dwMsg ) )
	{
		if( UIMSG_LB_DUP & dwMsg )
		{
			AddMessageEx( UIMSG_PVPTYRANNY_TOWER_CAPTURE_LBDUP );
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

void CPVPTyrannyTowerCapture::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
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
}

void CPVPTyrannyTowerCapture::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	if ( !m_bCHECK_MOUSE_STATE )
	{
		CheckMouseState ();
		m_bCHECK_MOUSE_STATE = true;
	}
}

void CPVPTyrannyTowerCapture::UpdateInfo()
{
	TYRANNY_STATE emstate = GLPVPTyrannyClient::GetInstance().m_emState;
	float fbattle_remain_time = GLPVPTyrannyClient::GetInstance().m_fRemain;
	WORD wwinner = GLPVPTyrannyClient::GetInstance().m_wLastWinner;
	WORD wowner_tower0 = GLPVPTyrannyClient::GetInstance().m_wTowerOwner[TYRANNY_TOWER_0];
	WORD wowner_tower1 = GLPVPTyrannyClient::GetInstance().m_wTowerOwner[TYRANNY_TOWER_1];
	WORD wowner_tower2 = GLPVPTyrannyClient::GetInstance().m_wTowerOwner[TYRANNY_TOWER_2];

	if ( m_pTextStatus )
		m_pTextStatus->ClearText();

	if ( m_pTextTimer )
		m_pTextTimer->ClearText();

	for( int i=0; i<TOWER_STATUS_TYPES; ++i )
	{
		if ( m_pStatusTower0[i] )	
			m_pStatusTower0[i]->SetVisibleSingle( FALSE );

		if ( m_pStatusTower1[i] )
			m_pStatusTower1[i]->SetVisibleSingle( FALSE );

		if ( m_pStatusTower2[i] )
			m_pStatusTower2[i]->SetVisibleSingle( FALSE );

		if ( m_pStatusWinner[i] )
			m_pStatusWinner[i]->SetVisibleSingle( FALSE );
	}
	
	if ( m_pTextStatus )
	{
		if ( emstate == TYRANNY_STATE_BATTLE )
			m_pTextStatus->AddText( ID2GAMEINTEXT("TYRANNY_TOWER_STATUS_BATTLE" ), NS_UITEXTCOLOR::ORANGE );
		else
			m_pTextStatus->AddText( ID2GAMEINTEXT("TYRANNY_TOWER_STATUS_COMPLETE" ), NS_UITEXTCOLOR::ORANGE );
	}
	
	if ( emstate == TYRANNY_STATE_BATTLE )
	{
		if ( m_pTextTimer )
		{
			if ( fbattle_remain_time > 0.0f )
			{
				DWORD dwTextColor = NS_UITEXTCOLOR::ORANGE;
				if ( fbattle_remain_time < 300.0f ){
					dwTextColor = NS_UITEXTCOLOR::RED;
				}
				CString strCombine;
				strCombine.Format ( "%02d:%02d", (DWORD)fbattle_remain_time/60, (DWORD)fbattle_remain_time%60 );
				m_pTextTimer->AddText( strCombine.GetString(), dwTextColor );
			}
		}
	}
	else
	{
		if ( wwinner < TOWER_STATUS_TYPES && m_pStatusWinner[wwinner] )
			m_pStatusWinner[wwinner]->SetVisibleSingle( TRUE );
	}

	if ( wowner_tower0 < TOWER_STATUS_TYPES && m_pStatusTower0[wowner_tower0] )
		m_pStatusTower0[wowner_tower0]->SetVisibleSingle( TRUE );

	if ( wowner_tower1 < TOWER_STATUS_TYPES && m_pStatusTower1[wowner_tower1] )
		m_pStatusTower1[wowner_tower1]->SetVisibleSingle( TRUE );

	if ( wowner_tower2 < TOWER_STATUS_TYPES && m_pStatusTower2[wowner_tower2] )
		m_pStatusTower2[wowner_tower2]->SetVisibleSingle( TRUE );
}