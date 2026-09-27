#include "StdAfx.h"
#include "CharacterCreateInfoClass.h"

#include "d3dfont.h"
#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCharacterCreateInfoClass::CCharacterCreateInfoClass ()
	: m_pInfoTitle( NULL )
	, m_pInfoDesc( NULL )
{
	for( int i=0; i<GLCI_NUM_8CLASS; ++i ){
		m_pLogo[i] = NULL;
	}
}

CCharacterCreateInfoClass::~CCharacterCreateInfoClass ()
{
}

void CCharacterCreateInfoClass::CreateSubControl ()
{
	CD3DFontPar* pFont11 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 11, _DEFAULT_FONT_FLAG );
	CD3DFontPar* pFont09 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER_FAT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuterFat ( "CREATE_CHAR_CLASS_WINDOW_BACK" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "CREATE_CHAR_CLASS_NAME_TEXT" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "CREATE_CHAR_CLASS_INFO_TEXT" );
	RegisterControl ( pBasicLineBox );

	m_pInfoTitle = new CBasicTextBox;
	m_pInfoTitle->CreateSub ( this, "CREATE_CHAR_CLASS_NAME_TEXT", UI_FLAG_DEFAULT );
	m_pInfoTitle->SetFont ( pFont11 );
	m_pInfoTitle->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pInfoTitle );

	m_pInfoDesc = new CBasicTextBox;
	m_pInfoDesc->CreateSub ( this, "CREATE_CHAR_CLASS_INFO_TEXT", UI_FLAG_DEFAULT );
	m_pInfoDesc->SetFont ( pFont09 );
	m_pInfoDesc->SetTextAlign ( TEXT_ALIGN_LEFT );
	RegisterControl ( m_pInfoDesc );	

	std::string strLogo[GLCI_NUM_8CLASS] = 
	{
		"CREATE_CHAR_CLASS_BRAWLER_MALE",
		"CREATE_CHAR_CLASS_SWORDMAN_MALE",
		"CREATE_CHAR_CLASS_ARCHER_FEMALE",
		"CREATE_CHAR_CLASS_SHAMAN_FEMALE",
		"CREATE_CHAR_CLASS_EXTREME_MALE",
		"CREATE_CHAR_CLASS_EXTREME_FEMALE",
		"CREATE_CHAR_CLASS_BRAWLER_FEMALE",
		"CREATE_CHAR_CLASS_SWORDMAN_FEMALE",
		"CREATE_CHAR_CLASS_ARCHER_MALE",
		"CREATE_CHAR_CLASS_SHAMAN_MALE",
		"CREATE_CHAR_CLASS_SCIENTIST_MALE",
		"CREATE_CHAR_CLASS_SCIENTIST_FEMALE",
		"CREATE_CHAR_CLASS_ASSASSIN_MALE",
		"CREATE_CHAR_CLASS_ASSASSIN_FEMALE",
		"CREATE_CHAR_CLASS_TRICKER_MALE",
		"CREATE_CHAR_CLASS_TRICKER_FEMALE",
	};

	for( int i=0; i<GLCI_NUM_8CLASS; ++i )
	{
		m_pLogo[i] = new CUIControl;
		m_pLogo[i]->CreateSub ( this, strLogo[i].c_str() );	
		m_pLogo[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pLogo[i] );
	}
}

void CCharacterCreateInfoClass::Reset()
{
	m_pInfoTitle->ClearText();
	m_pInfoDesc->ClearText();

	for( int i=0; i<GLCI_NUM_8CLASS; ++i ){
		m_pLogo[i]->SetVisibleSingle ( FALSE );
	}
}

void CCharacterCreateInfoClass::SetData( int nClass )
{
	Reset();

	if ( nClass < 0 )	return;
	if ( nClass >= GLCI_NUM_8CLASS )	return;

	m_pLogo[nClass]->SetVisibleSingle( TRUE );

	int nIndex = 0;
	switch( nClass )
	{
	case GLCI_EXTREME_M:
	case GLCI_EXTREME_W:
		nIndex = 0;
		break;

	case GLCI_BRAWLER_M:
	case GLCI_BRAWLER_W:
		nIndex = 1;
		break;

	case GLCI_SWORDSMAN_M:
	case GLCI_SWORDSMAN_W:
		nIndex = 2;
		break;

	case GLCI_ARCHER_M:
	case GLCI_ARCHER_W:
		nIndex = 3;
		break;

	case GLCI_SHAMAN_M:
	case GLCI_SHAMAN_W:
		nIndex = 4;
		break;

	case GLCI_GUNNER_M:
	case GLCI_GUNNER_W:
		nIndex = 5;
		break;

	case GLCI_ASSASSIN_M:
	case GLCI_ASSASSIN_W:
		nIndex = 6;
		break;

	case GLCI_TRICKER_M:
	case GLCI_TRICKER_W:
		nIndex = 7;
		break;

	};

	m_pInfoTitle->SetText( ID2GAMEEXTEXT("NEW_CHAR_SELECT_CLASS_NAME", nIndex ), NS_UITEXTCOLOR::WHITE );	

	CString strText;
	strText.Format ( "NEW_CHAR_SELECT_CLASS_DEC%d", nIndex );
	m_pInfoDesc->SetText ( ID2GAMEEXTEXT(strText), NS_UITEXTCOLOR::WHITE );
}
