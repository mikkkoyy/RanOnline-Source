#include "StdAfx.h"
#include "SelectCharacterListItem.h"

#include "d3dfont.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"

#include "DxGlobalStage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSelectCharacterListItem::CSelectCharacterListItem ()
	: m_pClassText(NULL)
	, m_pLevelText(NULL)
	, m_pNameText(NULL)
	, m_pItemSelect(NULL)
	, m_pItemSelectMouseIn(NULL)
	, m_dwCharID(GAEAID_NULL)
	, m_dwCharIDSelected(GAEAID_NULL)
	, m_nIndex(-1)
{
	for( int i=0; i<GLCI_NUM_8CLASS; ++ i )
	{
		m_pClassImageOn[i] = NULL;
		m_pClassImageOff[i] = NULL;
	}
}

CSelectCharacterListItem::~CSelectCharacterListItem ()
{
}

void CSelectCharacterListItem::CreateSubControl ()
{
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "SELECT_CHAR_WINDOW_CHAR_LIST_ITEM_FOCUS" );
	RegisterControl ( pBasicLineBox );	

	m_pClassText = new CBasicTextBox;
	m_pClassText->CreateSub ( this, "SELECT_CHAR_WINDOW_CHAR_LIST_CLASS" );
	m_pClassText->SetFont ( pFont8 );
	m_pClassText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pClassText );

	m_pLevelText = new CBasicTextBox;
	m_pLevelText->CreateSub ( this, "SELECT_CHAR_WINDOW_CHAR_LIST_LEVEL" );
	m_pLevelText->SetFont ( pFont8 );
	m_pLevelText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pLevelText );

	m_pNameText = new CBasicTextBox;
	m_pNameText->CreateSub ( this, "SELECT_CHAR_WINDOW_CHAR_LIST_NAME" );
	m_pNameText->SetFont ( pFont8 );
	m_pNameText->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pNameText );

	std::string strClassOn[GLCI_NUM_8CLASS] = 
	{
		"SELECT_CHAR_WINDOW_BRAWLER_MALE_ON",
		"SELECT_CHAR_WINDOW_SWORDSMAN_MALE_ON",
		"SELECT_CHAR_WINDOW_ARCHER_FEMALE_ON",
		"SELECT_CHAR_WINDOW_SHAMAN_FEMALE_ON",
		"SELECT_CHAR_WINDOW_EXTREME_MALE_ON",
		"SELECT_CHAR_WINDOW_EXTREME_FEMALE_ON",
		"SELECT_CHAR_WINDOW_BRAWLER_FEMALE_ON",
		"SELECT_CHAR_WINDOW_SWORDSMAN_FEMALE_ON",
		"SELECT_CHAR_WINDOW_ARCHER_MALE_ON",
		"SELECT_CHAR_WINDOW_SHAMAN_MALE_ON",
		"SELECT_CHAR_WINDOW_SCIENTIST_MALE_ON",
		"SELECT_CHAR_WINDOW_SCIENTIST_FEMALE_ON",
		"SELECT_CHAR_WINDOW_ASSASSIN_MALE_ON",
		"SELECT_CHAR_WINDOW_ASSASSIN_FEMALE_ON",
		"SELECT_CHAR_WINDOW_TRICKER_MALE_ON",
		"SELECT_CHAR_WINDOW_TRICKER_FEMALE_ON",
	};

	std::string strClassOff[GLCI_NUM_8CLASS] = 
	{
		"SELECT_CHAR_WINDOW_BRAWLER_MALE_OFF",
		"SELECT_CHAR_WINDOW_SWORDSMAN_MALE_OFF",
		"SELECT_CHAR_WINDOW_ARCHER_FEMALE_OFF",
		"SELECT_CHAR_WINDOW_SHAMAN_FEMALE_OFF",
		"SELECT_CHAR_WINDOW_EXTREME_MALE_OFF",
		"SELECT_CHAR_WINDOW_EXTREME_FEMALE_OFF",
		"SELECT_CHAR_WINDOW_BRAWLER_FEMALE_OFF",
		"SELECT_CHAR_WINDOW_SWORDSMAN_FEMALE_OFF",
		"SELECT_CHAR_WINDOW_ARCHER_MALE_OFF",
		"SELECT_CHAR_WINDOW_SHAMAN_MALE_OFF",
		"SELECT_CHAR_WINDOW_SCIENTIST_MALE_OFF",
		"SELECT_CHAR_WINDOW_SCIENTIST_FEMALE_OFF",
		"SELECT_CHAR_WINDOW_ASSASSIN_MALE_OFF",
		"SELECT_CHAR_WINDOW_ASSASSIN_FEMALE_OFF",
		"SELECT_CHAR_WINDOW_TRICKER_MALE_OFF",
		"SELECT_CHAR_WINDOW_TRICKER_FEMALE_OFF",
	};

	for( int i=0; i<GLCI_NUM_8CLASS; ++ i )
	{;
		m_pClassImageOn[i] = new CUIControl;
		m_pClassImageOn[i]->CreateSub ( this, strClassOn[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		m_pClassImageOn[i]->SetVisibleSingle( FALSE );
		RegisterControl ( m_pClassImageOn[i] );

		m_pClassImageOff[i] = new CUIControl;
		m_pClassImageOff[i]->CreateSub ( this, strClassOff[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		m_pClassImageOff[i]->SetVisibleSingle( FALSE );
		RegisterControl ( m_pClassImageOff[i] );
	}

	m_pItemSelect = new CUIControl;
	m_pItemSelect->CreateSub ( this, "SELECT_CHAR_WINDOW_SELECT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pItemSelect->SetVisibleSingle( FALSE );
	RegisterControl ( m_pItemSelect );

	m_pItemSelectMouseIn = new CUIControl;
	m_pItemSelectMouseIn->CreateSub ( this, "SELECT_CHAR_WINDOW_SELECT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pItemSelectMouseIn->SetVisibleSingle( FALSE );
	RegisterControl ( m_pItemSelectMouseIn );
}

void CSelectCharacterListItem::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	SCHARINFO_LOBBY* pInfo = DxGlobalStage::GetInstance().GetLobyStage()->GetCharInfo( m_nIndex );
	SCHARINFO_LOBBY* pInfoSelect = DxGlobalStage::GetInstance().GetLobyStage()->GetSelectCharInfo();

	if ( pInfo && pInfo->m_dwCharID != 0 )
	{
		if ( m_dwCharID != pInfo->m_dwCharID )
		{
			m_dwCharID = pInfo->m_dwCharID;
			UpdateData();
		}

		if ( pInfoSelect && pInfoSelect->m_dwCharID != 0 )
		{
			if ( m_dwCharIDSelected != pInfoSelect->m_dwCharID )
			{
				m_dwCharIDSelected = pInfoSelect->m_dwCharID;
				UpdateData();
			}
		}
	}else{
		ResetData ();
	}

	SetMouseIn( FALSE );
}

void CSelectCharacterListItem::SetVisibleSingle ( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle( bVisible );

	if ( bVisible )
	{	
		ResetData();
	}
}

void CSelectCharacterListItem::ResetData ()
{
	for ( int i=0 ; i<GLCI_NUM_8CLASS; ++i ) 
	{
		m_pClassImageOn[i]->SetVisibleSingle( FALSE );
		m_pClassImageOff[i]->SetVisibleSingle( FALSE );
	}

	m_pClassText->ClearText();
	m_pLevelText->ClearText();
	m_pNameText->ClearText();

	m_pItemSelect->SetVisibleSingle( FALSE );
	m_pItemSelectMouseIn->SetVisibleSingle( FALSE );

	m_dwCharID = GAEAID_NULL;
	m_dwCharIDSelected = GAEAID_NULL;
	m_nIndex = -1;
}

void CSelectCharacterListItem::UpdateData()
{
	SCHARINFO_LOBBY* pInfo = DxGlobalStage::GetInstance().GetLobyStage()->GetCharInfo( m_nIndex );
	SCHARINFO_LOBBY* pInfoSelect = DxGlobalStage::GetInstance().GetLobyStage()->GetSelectCharInfo();

	if ( pInfo && pInfo->m_dwCharID != 0 )
	{
		bool bSelected = false;
		if ( pInfoSelect && pInfoSelect->m_dwCharID != 0 )
		{
			if ( pInfoSelect->m_dwCharID == pInfo->m_dwCharID )
				bSelected = true;
		}

		for ( int i=0 ; i<GLCI_NUM_8CLASS; ++i ) 
		{
			m_pClassImageOn[i]->SetVisibleSingle( FALSE );
			m_pClassImageOff[i]->SetVisibleSingle( FALSE );
		}

		m_pClassImageOn[ CharClassToIndex(pInfo->m_emClass) ]->SetVisibleSingle( bSelected? TRUE:FALSE );
		m_pClassImageOff[ CharClassToIndex(pInfo->m_emClass) ]->SetVisibleSingle( bSelected? FALSE:TRUE );
		m_pItemSelect->SetVisibleSingle( bSelected? TRUE:FALSE );

		CString strTemp;
		strTemp.Format("%s", COMMENT::CHARCLASS[CharClassToIndex( pInfo->m_emClass )].c_str() );
		m_pClassText->SetOneLineText ( strTemp.GetString(), NS_UITEXTCOLOR::WHITE );
		strTemp.Format("Lv. %d",pInfo->m_wLevel);
		m_pLevelText->SetOneLineText ( strTemp.GetString(), NS_UITEXTCOLOR::WHITE );
		strTemp.Format("%s",pInfo->m_szName);
		m_pNameText->SetOneLineText ( strTemp.GetString(), NS_UITEXTCOLOR::WHITE );		
	}
}

void CSelectCharacterListItem::SetMouseIn( BOOL bEnable )
{
	m_pItemSelectMouseIn->SetVisibleSingle( bEnable );
}