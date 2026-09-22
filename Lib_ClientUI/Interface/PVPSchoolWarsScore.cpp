#include "StdAfx.h"
#include "PVPSchoolWarsScore.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicProgressBar.h"
#include "d3dfont.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "GLPVPSchoolWarsClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPSchoolWarsScore::CPVPSchoolWarsScore ()
	: m_bCHECK_MOUSE_STATE(FALSE)
	, m_bFirstGap(FALSE)
	, m_nPosX(0)
	, m_nPosY(0)
	, m_fUpdateTime(0.0f)
	, m_fImageTime(0.0f)
	, m_pProgressTimer(NULL)
	, m_pProgress0(NULL)
	, m_pProgress1(NULL)
	, m_pProgress2(NULL)
	, m_pProgressDP(NULL)
	, m_pTextTimer(NULL)
	, m_pTextStatic0(NULL)
	, m_pTextStatic1(NULL)
	, m_pTextStatic2(NULL)
	, m_pTextStaticDP(NULL)
	, m_pTextScore0(NULL)
	, m_pTextScore1(NULL)
	, m_pTextScore2(NULL)	
	, m_pTextScoreDP(NULL)
	, m_pImage(NULL)
{
}

CPVPSchoolWarsScore::~CPVPSchoolWarsScore ()
{
}

void CPVPSchoolWarsScore::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG );

	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, "PVP_SCHOOLWARS_SCORE_BG", UI_FLAG_DEFAULT, SCHOOWARS_STATUS_BOX1 );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "PVP_SCHOOLWARS_DP_BG2", UI_FLAG_DEFAULT, SCHOOWARS_STATUS_BOX2 );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "PVP_SCHOOLWARS_SCORE_IMAGE_SCHOOL0", UI_FLAG_DEFAULT );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "PVP_SCHOOLWARS_SCORE_IMAGE_SCHOOL1", UI_FLAG_DEFAULT );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "PVP_SCHOOLWARS_SCORE_IMAGE_SCHOOL2", UI_FLAG_DEFAULT );
	RegisterControl ( pControl );

	m_pImage = new CUIControl;
	m_pImage->CreateSub ( this, "PVP_SCHOOLWARS_SCORE_NOTIFY", UI_FLAG_DEFAULT );
	m_pImage->SetVisibleSingle( FALSE );
	RegisterControl ( m_pImage );

	m_pProgressTimer = new CBasicProgressBar;
	m_pProgressTimer->CreateSub ( this, "PVP_SCHOOLWARS_TIMER_BAR" );
	m_pProgressTimer->CreateOverImage ( "PVP_SCHOOLWARS_TIMER_OVERIMAGE" );
	m_pProgressTimer->SetPercent( 0.0f );
	RegisterControl ( m_pProgressTimer );

	m_pProgress0 = new CBasicProgressBar;
	m_pProgress0->CreateSub ( this, "PVP_SCHOOLWARS_SCORE_BAR1" );
	m_pProgress0->CreateOverImage ( "PVP_SCHOOLWARS_SCORE_OVERIMAGE" );
	m_pProgress0->SetPercent( 0.0f );
	RegisterControl ( m_pProgress0 );

	m_pProgress1 = new CBasicProgressBar;
	m_pProgress1->CreateSub ( this, "PVP_SCHOOLWARS_SCORE_BAR2" );
	m_pProgress1->CreateOverImage ( "PVP_SCHOOLWARS_SCORE_OVERIMAGE" );
	m_pProgress1->SetPercent( 0.0f );
	RegisterControl ( m_pProgress1 );

	m_pProgress2 = new CBasicProgressBar;
	m_pProgress2->CreateSub ( this, "PVP_SCHOOLWARS_SCORE_BAR3" );
	m_pProgress2->CreateOverImage ( "PVP_SCHOOLWARS_SCORE_OVERIMAGE" );
	m_pProgress2->SetPercent( 0.0f );
	RegisterControl ( m_pProgress2 );

	m_pProgressDP = new CBasicProgressBar;
	m_pProgressDP->CreateSub ( this, "PVP_SCHOOLWARS_DP_BAR" );
	m_pProgressDP->CreateOverImage ( "PVP_SCHOOLWARS_DP_OVERIMAGE" );
	m_pProgressDP->SetPercent( 0.0f );
	RegisterControl ( m_pProgressDP );

	m_pTextTimer = new CBasicTextBox;
	m_pTextTimer->CreateSub ( this, "PVP_SCHOOLWARS_SCORE_TIMER" );
	m_pTextTimer->SetFont ( pFont9 );
	m_pTextTimer->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextTimer->AddText( "FF:FF", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextTimer );

	m_pTextStatic0 = new CBasicTextBox;
	m_pTextStatic0->CreateSub ( this, "PVP_SCHOOLWARS_SCORE_TEXT1" );
	m_pTextStatic0->SetFont ( pFont8 );
	m_pTextStatic0->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pTextStatic0->AddText( ID2GAMEWORD("SCHOOLWARS_SCORE_TEXT", 0 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextStatic0 );

	m_pTextStatic1 = new CBasicTextBox;
	m_pTextStatic1->CreateSub ( this, "PVP_SCHOOLWARS_SCORE_TEXT2" );
	m_pTextStatic1->SetFont ( pFont8 );
	m_pTextStatic1->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pTextStatic1->AddText( ID2GAMEWORD("SCHOOLWARS_SCORE_TEXT", 1 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextStatic1 );

	m_pTextStatic2 = new CBasicTextBox;
	m_pTextStatic2->CreateSub ( this, "PVP_SCHOOLWARS_SCORE_TEXT3" );
	m_pTextStatic2->SetFont ( pFont8 );
	m_pTextStatic2->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pTextStatic2->AddText( ID2GAMEWORD("SCHOOLWARS_SCORE_TEXT", 2 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextStatic2 );

	m_pTextStaticDP = new CBasicTextBox;
	m_pTextStaticDP->CreateSub ( this, "PVP_SCHOOLWARS_DP_TEXT_STATIC" );
	m_pTextStaticDP->SetFont ( pFont8 );
	m_pTextStaticDP->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pTextStaticDP->AddText( ID2GAMEWORD("SCHOOLWARS_SCORE_TEXT", 3 ), NS_UITEXTCOLOR::RED );
	RegisterControl ( m_pTextStaticDP );

	m_pTextScore0 = new CBasicTextBox;
	m_pTextScore0->CreateSub ( this, "PVP_SCHOOLWARS_SCORE_GRADE_TEXT1" );
	m_pTextScore0->SetFont ( pFont8 );
	m_pTextScore0->SetTextAlign ( TEXT_ALIGN_CENTER_X );
	m_pTextScore0->AddText( "0", NS_UITEXTCOLOR::ORNAGERED );
	RegisterControl ( m_pTextScore0 );

	m_pTextScore1 = new CBasicTextBox;
	m_pTextScore1->CreateSub ( this, "PVP_SCHOOLWARS_SCORE_GRADE_TEXT2" );
	m_pTextScore1->SetFont ( pFont8 );
	m_pTextScore1->SetTextAlign ( TEXT_ALIGN_CENTER_X );
	m_pTextScore1->AddText( "0", NS_UITEXTCOLOR::ORNAGERED );
	RegisterControl ( m_pTextScore1 );

	m_pTextScore2 = new CBasicTextBox;
	m_pTextScore2->CreateSub ( this, "PVP_SCHOOLWARS_SCORE_GRADE_TEXT3" );
	m_pTextScore2->SetFont ( pFont8 );
	m_pTextScore2->SetTextAlign ( TEXT_ALIGN_CENTER_X );
	m_pTextScore2->AddText( "0", NS_UITEXTCOLOR::ORNAGERED );
	RegisterControl ( m_pTextScore2 );

	m_pTextScoreDP = new CBasicTextBox;
	m_pTextScoreDP->CreateSub ( this, "PVP_SCHOOLWARS_DP_TEXT" );
	m_pTextScoreDP->SetFont ( pFont8 );
	m_pTextScoreDP->SetTextAlign ( TEXT_ALIGN_CENTER_X );
	m_pTextScoreDP->AddText( "0", NS_UITEXTCOLOR::ORNAGERED );
	RegisterControl ( m_pTextScoreDP );
}

void CPVPSchoolWarsScore::CheckMouseState ()
{
	const DWORD dwMsg = GetMessageEx ();
	if ( CHECK_MOUSE_IN ( dwMsg ) )
	{
		if( UIMSG_LB_DUP & dwMsg )
		{
			AddMessageEx( UIMSG_SCHOOLWARS_SCORE_WINDOW_LBDUP );
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

void CPVPSchoolWarsScore::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
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

	if ( m_pImage && m_pImage->IsVisible() )
	{
		m_fImageTime += fElapsedTime;
		if ( m_fImageTime >= 0.5f )
		{
			m_fImageTime = 0.0f;
			m_pImage->SetVisibleSingle( FALSE );
		}
	}
}

void CPVPSchoolWarsScore::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	if ( !m_bCHECK_MOUSE_STATE )
	{
		CheckMouseState ();
		m_bCHECK_MOUSE_STATE = true;
	}
}

void CPVPSchoolWarsScore::UpdateInfo()
{
	GLPVPSchoolWarsClient* pClient = &GLPVPSchoolWarsClient::GetInstance();
	if ( !pClient )	return;

	if ( m_pTextTimer )
		m_pTextTimer->ClearText();

	if ( pClient->m_emState == SCHOOLWARS_STATE_BATTLE )
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

		float ftime = pClient->m_fRemain / float(pClient->m_wBattleTime) * 100.0f;

		if ( m_pProgressTimer && ftime > 0.0f )
			m_pProgressTimer->SetPercent( ftime / 100.0f );
	}
	else
	{
		if ( m_pTextTimer && pClient->m_wLastWinner < SCHOOLWARS_SCHOOL_SIZE )
			m_pTextTimer->AddText( COMMENT::EMSCHOOL[pClient->m_wLastWinner].c_str(), NS_UITEXTCOLOR::WHITE );

		if ( m_pProgressTimer )
			m_pProgressTimer->SetPercent( 0.0f );
	}

	float fScore0 = float(pClient->m_sScore[SCHOOLWARS_SCHOOL_SG].wKills) / float(pClient->m_wMaxScore) * 100.0f;
	float fScore1 = float(pClient->m_sScore[SCHOOLWARS_SCHOOL_MP].wKills) / float(pClient->m_wMaxScore) * 100.0f;
	float fScore2 = float(pClient->m_sScore[SCHOOLWARS_SCHOOL_PHX].wKills) / float(pClient->m_wMaxScore) * 100.0f;
	float fDoublePont = float(pClient->m_fDoublePoint / SCHOOLWARS_DOUBLE_POINT_TIME_DURATION) * 100.0f;

	if ( m_pProgress0 )	m_pProgress0->SetPercent( 0.0f );
	if ( m_pProgress1 )	m_pProgress1->SetPercent( 0.0f );
	if ( m_pProgress2 )	m_pProgress2->SetPercent( 0.0f );
	if ( m_pProgressDP )	m_pProgressDP->SetPercent( 0.0f );

	if ( m_pProgress0 && fScore0 > 0.0f )
		m_pProgress0->SetPercent( fScore0 / 100.0f );
	
	if ( m_pProgress1 && fScore1 > 0.0f )
		m_pProgress1->SetPercent( fScore1 / 100.0f );

	if ( m_pProgress2 && fScore2 > 0.0f )
		m_pProgress2->SetPercent( fScore2 / 100.0f );

	if ( m_pProgressDP && fDoublePont > 0.0f )
		m_pProgressDP->SetPercent( fDoublePont / 100.0f );

	CString strTemp;
	if ( m_pTextScore0 )
	{
		m_pTextScore0->ClearText();
		strTemp.Format( "%d", pClient->m_sScore[SCHOOLWARS_SCHOOL_SG].wKills );
		m_pTextScore0->AddText( strTemp.GetString(), NS_UITEXTCOLOR::GREENYELLOW );
	}

	if ( m_pTextScore1 )
	{
		m_pTextScore1->ClearText();
		strTemp.Format( "%d", pClient->m_sScore[SCHOOLWARS_SCHOOL_MP].wKills );
		m_pTextScore1->AddText( strTemp.GetString(), NS_UITEXTCOLOR::GREENYELLOW );
	}

	if ( m_pTextScore2 )
	{
		m_pTextScore2->ClearText();
		strTemp.Format( "%d", pClient->m_sScore[SCHOOLWARS_SCHOOL_PHX].wKills );
		m_pTextScore2->AddText( strTemp.GetString(), NS_UITEXTCOLOR::GREENYELLOW );
	}

	if ( m_pTextScoreDP )
	{
		m_pTextScoreDP->ClearText();
		if ( pClient->m_fDoublePoint > 0.0f )
		{
			CString strCombine;
			strCombine.Format ( "%02d:%02d", (DWORD)pClient->m_fDoublePoint/60, (DWORD)pClient->m_fDoublePoint%60 );
			m_pTextScoreDP->AddText( strCombine.GetString(), NS_UITEXTCOLOR::RED );
		}
	}

	/*CString strTemp;
	if ( m_pTextScore0 )
	{
		m_pTextScore0->ClearText();
		strTemp.Format( "K:%d", pClient->m_sScore[SCHOOLWARS_SCHOOL_SG].wKills );
		int nIndex = m_pTextScore0->AddText( strTemp.GetString(), NS_UITEXTCOLOR::GREENYELLOW );
		m_pTextScore0->AddString( nIndex, " / ", NS_UITEXTCOLOR::WHITE );
		strTemp.Format( "D:%d", pClient->m_sScore[SCHOOLWARS_SCHOOL_SG].wDeaths );
		m_pTextScore0->AddString( nIndex, strTemp.GetString(), NS_UITEXTCOLOR::ORNAGERED );
	}

	if ( m_pTextScore1 )
	{
		m_pTextScore1->ClearText();
		strTemp.Format( "K:%d", pClient->m_sScore[SCHOOLWARS_SCHOOL_MP].wKills );
		int nIndex = m_pTextScore1->AddText( strTemp.GetString(), NS_UITEXTCOLOR::GREENYELLOW );
		m_pTextScore1->AddString( nIndex, " / ", NS_UITEXTCOLOR::WHITE );
		strTemp.Format( "D:%d", pClient->m_sScore[SCHOOLWARS_SCHOOL_MP].wDeaths );
		m_pTextScore1->AddString( nIndex, strTemp.GetString(), NS_UITEXTCOLOR::ORNAGERED );
	}

	if ( m_pTextScore2 )
	{
		m_pTextScore2->ClearText();
		strTemp.Format( "K:%d", pClient->m_sScore[SCHOOLWARS_SCHOOL_PHX].wKills );
		int nIndex = m_pTextScore2->AddText( strTemp.GetString(), NS_UITEXTCOLOR::GREENYELLOW );
		m_pTextScore2->AddString( nIndex, " / ", NS_UITEXTCOLOR::WHITE );
		strTemp.Format( "D:%d", pClient->m_sScore[SCHOOLWARS_SCHOOL_PHX].wDeaths );
		m_pTextScore2->AddString( nIndex, strTemp.GetString(), NS_UITEXTCOLOR::ORNAGERED );
	}*/
}

void CPVPSchoolWarsScore::ToNotify()
{
	if ( m_pImage )
	{
		m_fImageTime = 0.0f;
		m_pImage->SetVisibleSingle( TRUE );
	}
}