#include "StdAfx.h"
#include "CharacterCreateInfoSchool.h"

#include "d3dfont.h"
#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCharacterCreateInfoSchool::CCharacterCreateInfoSchool ()
	: m_pInfoTitle( NULL )
	, m_pInfoDesc( NULL )
{
	for( int i=0; i<GLSCHOOL_NUM; ++i ){
		m_pLogo[i] = NULL;
	}
}

CCharacterCreateInfoSchool::~CCharacterCreateInfoSchool ()
{
}

void CCharacterCreateInfoSchool::CreateSubControl ()
{
	CD3DFontPar* pFont12 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 12, _DEFAULT_FONT_SHADOW_FLAG );
	CD3DFontPar* pFont08 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER_FAT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuterFat ( "CREATE_CHAR_SCHOOL_WINDOW_BACK" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "CREATE_CHAR_SCHOOL_NAME_TEXT" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "CREATE_CHAR_SCHOOL_INFO_TEXT" );
	RegisterControl ( pBasicLineBox );

	m_pInfoTitle = new CBasicTextBox;
	m_pInfoTitle->CreateSub ( this, "CREATE_CHAR_SCHOOL_NAME_TEXT", UI_FLAG_DEFAULT );
	m_pInfoTitle->SetFont ( pFont12 );
	m_pInfoTitle->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );		
	RegisterControl ( m_pInfoTitle );

	m_pInfoDesc = new CBasicTextBox;
	m_pInfoDesc->CreateSub ( this, "CREATE_CHAR_SCHOOL_INFO_TEXT", UI_FLAG_DEFAULT );
	m_pInfoDesc->SetFont ( pFont08 );
	m_pInfoDesc->SetTextAlign ( TEXT_ALIGN_LEFT );		
	RegisterControl ( m_pInfoDesc );	

	std::string strLogo[GLSCHOOL_NUM] = 
	{
		"CREATE_CHAR_SCHOOL_WINDOW_BH",
		"CREATE_CHAR_SCHOOL_WINDOW_HA",
		"CREATE_CHAR_SCHOOL_WINDOW_SM",
	};

	for( int i=0; i<GLSCHOOL_NUM; ++i )
	{
		m_pLogo[i] = new CUIControl;
		m_pLogo[i]->CreateSub ( this, strLogo[i].c_str() );	
		m_pLogo[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pLogo[i] );
	}
}

void CCharacterCreateInfoSchool::Reset()
{
	m_pInfoTitle->ClearText();
	m_pInfoDesc->ClearText();

	for( int i=0; i<GLSCHOOL_NUM; ++i ){
		m_pLogo[i]->SetVisibleSingle ( FALSE );
	}
}

void CCharacterCreateInfoSchool::SetData( int nSchool )
{
	Reset();

	if ( nSchool < 0 )	return;
	if ( nSchool >= GLSCHOOL_NUM )	return;

	m_pLogo[nSchool]->SetVisibleSingle( TRUE );
	m_pInfoTitle->SetText( ID2GAMEEXTEXT("NEW_CHAR_SELECT_SCHOOL_NAME", nSchool ), NS_UITEXTCOLOR::WHITE );	

	CString strText;
	strText.Format ( "NEW_CHAR_SELECT_SCHOOL_DEC%d", nSchool );
	m_pInfoDesc->SetText ( ID2GAMEEXTEXT(strText), NS_UITEXTCOLOR::WHITE );
}
