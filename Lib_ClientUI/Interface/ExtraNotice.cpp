#include "StdAfx.h"
#include "ExtraNotice.h"
#include "InnerInterface.h"
#include "./BasicTextBox.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define EVENTNOTICE_RENDERTIME (float)20.0f

CExtraNotice::CExtraNotice ()	
	: m_pTextBox(NULL)
{
}

CExtraNotice::~CExtraNotice ()
{
}

void CExtraNotice::CreateSubControl ()
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 16, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, "EXTRA_NOTICE_TEXT" );
	pTextBox->SetFont ( pFont );
	pTextBox->SetTextAlign ( TEXT_ALIGN_CENTER_X );
	RegisterControl ( pTextBox );
	m_pTextBox = pTextBox;
}

void CExtraNotice::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ){
		return;
	}

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_fLifeTime -= fElapsedTime;

	if ( m_fLifeTime < 0.0f ){
		m_pTextBox->ClearText();
		CInnerInterface::GetInstance().HideGroup ( GetWndID () );
		return;
	}
}

void CExtraNotice::AddText ( CString strMessage, D3DCOLOR dwMessageColor )
{
	if ( m_pTextBox ){
		m_pTextBox->ClearText();
		m_pTextBox->AddText ( strMessage, dwMessageColor );
		m_fLifeTime = EVENTNOTICE_RENDERTIME;
	}
}
