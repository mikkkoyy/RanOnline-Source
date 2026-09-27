#include "StdAfx.h"
#include "CharacterCreateInfoSex.h"

#include "d3dfont.h"
#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCharacterCreateInfoSex::CCharacterCreateInfoSex ()
	: m_pInfoTitle( NULL )
{
	for( int i=0; i<2; ++i ){
		m_pLogo[i] = NULL;
	}
}

CCharacterCreateInfoSex::~CCharacterCreateInfoSex ()
{
}

void CCharacterCreateInfoSex::CreateSubControl ()
{
	CD3DFontPar* pFont12 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 12, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER_FAT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuterFat ( "CREATE_CHAR_SEX_WINDOW_BACK" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "CREATE_CHAR_SEX_NAME_TEXT" );
	RegisterControl ( pBasicLineBox );

	m_pInfoTitle = new CBasicTextBox;
	m_pInfoTitle->CreateSub ( this, "CREATE_CHAR_SEX_NAME_TEXT", UI_FLAG_DEFAULT );
	m_pInfoTitle->SetFont ( pFont12 );
	m_pInfoTitle->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );		
	RegisterControl ( m_pInfoTitle );


	std::string strLogo[2] = 
	{
		"CREATE_CHAR_SEX_WINDOW_MALE",
		"CREATE_CHAR_SEX_WINDOW_FEMALE",
	};

	for( int i=0; i<2; ++i )
	{
		m_pLogo[i] = new CUIControl;
		m_pLogo[i]->CreateSub ( this, strLogo[i].c_str() );	
		m_pLogo[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pLogo[i] );
	}
}

void CCharacterCreateInfoSex::Reset()
{
	m_pInfoTitle->ClearText();

	for( int i=0; i<2; ++i ){
		m_pLogo[i]->SetVisibleSingle ( FALSE );
	}
}

void CCharacterCreateInfoSex::SetData( int nSex )
{
	Reset();

	if ( nSex < 0 )	return;
	if ( nSex >= 2 )	return;

	m_pLogo[nSex]->SetVisibleSingle( TRUE );
	m_pInfoTitle->SetText( ID2GAMEEXTEXT("NEW_CHAR_SELECT_SEX_NAME", nSex ), NS_UITEXTCOLOR::WHITE );
}
