#include "StdAfx.h"
#include "LobbyTitle.h"

#include "d3dfont.h"
#include "GameTextControl.h"
#include "UITextControl.h"
#include "BasicTextBox.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CLobbyTitle::CLobbyTitle ()
	: m_pTitle( NULL )
{
}

CLobbyTitle::~CLobbyTitle ()
{
}

void CLobbyTitle::CreateSubControl ()
{
	CD3DFontPar* pFont12 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 12, _DEFAULT_FONT_FLAG );

	m_pTitle = new CBasicTextBox;
	m_pTitle->CreateSub ( this, "OUTER_LOBBY_TITLE_TEXT" );
	m_pTitle->SetFont ( pFont12 );
	m_pTitle->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pTitle );
}

void CLobbyTitle::SetLobbyText( CString strText )
{
	ClearLobbyText();

	if ( strText.GetLength() <= 0 )	{
		return;
	}

	CString strTitleText;
	strTitleText.Format( "< %s >", strText.GetString() );
	m_pTitle->SetText( strTitleText.GetString(), NS_UITEXTCOLOR::WHITE );
}

void CLobbyTitle::ClearLobbyText()
{
	m_pTitle->ClearText();
}