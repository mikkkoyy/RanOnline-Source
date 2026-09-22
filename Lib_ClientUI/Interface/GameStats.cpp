#include "StdAfx.h"
#include "GameStats.h"
#include "BasicTextBox.h"
#include "GLGaeaClient.h"
#include "d3dfont.h"
#include "UITextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CGameStats::CGameStats()
	: m_pPingText(NULL)
	, m_pFpsText(NULL)
	, m_fPing(0.0f)
	, m_fFPS(0.0f)
	, m_fPingBack(0.0f)
	, m_fFPSBack(0.0f)
{
}

CGameStats::~CGameStats(void)
{
}

void CGameStats::CreateSubControl ()
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	m_pPingText = new CBasicTextBox;
	m_pPingText->CreateSub ( this, "GAMESTATS_PING" );
	m_pPingText->SetFont ( pFont );
	m_pPingText->SetTextAlign ( TEXT_ALIGN_CENTER_X );
	RegisterControl ( m_pPingText );

	m_pFpsText = new CBasicTextBox;
	m_pFpsText->CreateSub ( this, "GAMESTATS_FPS" );
	m_pFpsText->SetFont ( pFont );
	m_pFpsText->SetTextAlign ( TEXT_ALIGN_CENTER_X );
	RegisterControl ( m_pFpsText );

	/*m_pPingText->SetVisibleSingle(FALSE);
	m_pFpsText->SetVisibleSingle(FALSE);*/
}

void CGameStats::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{	
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
	
	if ( m_fPing != m_fPingBack || m_fFPS != m_fFPSBack )
	{
		UpdateInfo();

		m_fPingBack = m_fPing;
		m_fFPSBack = m_fFPS;
	}
}

void CGameStats::UpdateInfo()
{
	if ( m_pPingText )
	{
		float fPing = m_fPing;
		fPing = fPing * 0.5f;
		if ( fPing >= 1000.0f )	fPing = 1000.0f;
		if ( fPing <= 100.0f  ) fPing = 100.0f;

		DWORD dwTEXTCOLOR = NS_UITEXTCOLOR::AQUAMARINE;

		if ( fPing >= 300.0f )	dwTEXTCOLOR = NS_UITEXTCOLOR::YELLOW;
		if ( fPing >= 500.0f )	dwTEXTCOLOR = NS_UITEXTCOLOR::RED;

		CString strPing("");
		strPing.Format( "%2.0f ms", fPing );
		m_pPingText->ClearText();
		m_pPingText->AddText( strPing.GetString(), dwTEXTCOLOR );
	}

	if ( m_pFpsText )
	{
		float fFPS = m_fFPS;

		DWORD dwTEXTCOLOR = NS_UITEXTCOLOR::RED;

		if ( fFPS >= 20.0f )	dwTEXTCOLOR = NS_UITEXTCOLOR::YELLOW;
		if ( fFPS >= 50.0f )	dwTEXTCOLOR = NS_UITEXTCOLOR::AQUAMARINE;

		CString strFPS("");
		strFPS.Format( "%2.0f fps", fFPS );
		m_pFpsText->ClearText();
		m_pFpsText->AddText( strFPS.GetString(), dwTEXTCOLOR );
	}
}