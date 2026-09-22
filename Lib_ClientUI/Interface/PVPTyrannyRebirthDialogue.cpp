#include "StdAfx.h"
#include "PVPTyrannyRebirthDialogue.h"
#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicProgressBar.h"
#include "InnerInterface.h"

#include "GLPVPTyrannyData.h"
#include "GLPVPTyrannyClient.h"
#include "BasicLineBox.h"
#include "d3dfont.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include <time.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CPVPTyrannyRebirthDialogue::CPVPTyrannyRebirthDialogue ()
	: m_pTextDesc(NULL)
	, m_pTimer(NULL)
{
	timeStart = CTime::GetCurrentTime().GetTime();
}

CPVPTyrannyRebirthDialogue::~CPVPTyrannyRebirthDialogue ()
{
}

void CPVPTyrannyRebirthDialogue::CreateSubControl ()
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 12, _DEFAULT_FONT_SHADOW_FLAG );

	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, "TYRANNY_FORCE_REBIRTH_WATCH_ICON" );
	RegisterControl ( pControl );

	m_pTimer = new CBasicProgressBar;
	m_pTimer->CreateSub ( this, "TYRANNY_FORCE_REBIRTH_BLANK_LINE" );
	m_pTimer->CreateOverImage ( "TYRANNY_FORCE_REBIRTH_GAUGE_LINE" );
	RegisterControl ( m_pTimer );

	m_pTextDesc = new CBasicTextBox;
	m_pTextDesc->CreateSub ( this, "TYRANNY_FORCE_REBIRTH_TEXT" );
	m_pTextDesc->SetFont ( pFont );
	m_pTextDesc->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y  );
	RegisterControl ( m_pTextDesc );
}

void CPVPTyrannyRebirthDialogue::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIWindow::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	float fseconds = (float)difftime( CTime::GetCurrentTime().GetTime(), timeStart );

	if ( fseconds >= TYRANNY_REVIVE_TIME )
	{
		GLPVPTyrannyClient::GetInstance().DoRevive();
		timeStart = CTime::GetCurrentTime().GetTime();
	}

	float fremain = TYRANNY_REVIVE_TIME - fseconds;
	float fpercentage = fremain / TYRANNY_REVIVE_TIME;

	if ( m_pTimer )
		m_pTimer->SetPercent( fpercentage );

	if ( m_pTextDesc )
	{
		CString strtime = CInnerInterface::GetInstance().MakeString ( ID2GAMEINTEXT("TYRANNY_REBIRTH_TIMER"), (int)fremain );
		
		m_pTextDesc->ClearText();
		m_pTextDesc->AddText( strtime, NS_UITEXTCOLOR::WHITE );
	}
}
