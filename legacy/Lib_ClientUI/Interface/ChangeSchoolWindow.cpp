#include "StdAfx.h"
#include "ChangeSchoolWindow.h"
#include "InnerInterface.h"

#include "d3dfont.h"
#include "BasicTextButton.h"
#include "BasicLineBox.h"
#include "BasicTextBox.h"

#include "GLGaeaClient.h"
#include "UITextControl.h"
#include "GameTextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CChangeSchoolWindow::CChangeSchoolWindow(void)
	: m_pButtonClose(NULL)
	, m_pButtonSave(NULL)
	, m_pInfoTitle(NULL)
	, m_pInfoDesc(NULL)
	, m_nSelectedSchool( -1 )
	, m_nSelectedLastSchool( -1 )
{
	for( int i=0; i<GLSCHOOL_NUM; ++i )
	{
		m_pTypeSchool_Off[i] = NULL;
		m_pTypeSchool_On[i] = NULL;
		m_pTypeSchool_Box[i] = NULL;
		m_pTypeSchool_Focus[i] = NULL;
	}
}

CChangeSchoolWindow::~CChangeSchoolWindow(void)
{
}

void CChangeSchoolWindow::CreateSubControl()
{
	CD3DFontPar* pFont12 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 12, _DEFAULT_FONT_SHADOW_FLAG );
	CD3DFontPar* pFont08 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_CHANGE_SCHOOL_WINDOW_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_CHANGE_SCHOOL_TITLE_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_CHANGE_SCHOOL_DESC_LINE" );
	RegisterControl ( pBasicLineBox );


	std::string strSchoolOff[GLSCHOOL_NUM] = 
	{
		"RAN_CHANGE_SCHOOL_MARK_SM_OFF",
		"RAN_CHANGE_SCHOOL_MARK_HA_OFF",
		"RAN_CHANGE_SCHOOL_MARK_BH_OFF",
	};

	std::string strSchoolOn[GLSCHOOL_NUM] = 
	{
		"RAN_CHANGE_SCHOOL_MARK_SM_ON",
		"RAN_CHANGE_SCHOOL_MARK_HA_ON",
		"RAN_CHANGE_SCHOOL_MARK_BH_ON",
	};

	std::string strSchoolBox[GLSCHOOL_NUM] = 
	{
		"RAN_CHANGE_SCHOOL_MARK_SM_BOX",
		"RAN_CHANGE_SCHOOL_MARK_HA_BOX",
		"RAN_CHANGE_SCHOOL_MARK_BH_BOX",
	};

	std::string strSchoolFocus[GLSCHOOL_NUM] = 
	{
		"RAN_CHANGE_SCHOOL_MARK_SM_FOCUS",
		"RAN_CHANGE_SCHOOL_MARK_HA_FOCUS",
		"RAN_CHANGE_SCHOOL_MARK_BH_FOCUS",
	};

	for( int i=0; i<GLSCHOOL_NUM; ++i )
	{

		m_pTypeSchool_Off[i] = new CUIControl;
		m_pTypeSchool_Off[i]->CreateSub ( this, strSchoolOff[i].c_str() );	
		m_pTypeSchool_Off[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pTypeSchool_Off[i] );

		m_pTypeSchool_On[i] = new CUIControl;
		m_pTypeSchool_On[i]->CreateSub ( this, strSchoolOn[i].c_str() );	
		m_pTypeSchool_On[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pTypeSchool_On[i] );

		m_pTypeSchool_Box[i] = new CUIControl;
		m_pTypeSchool_Box[i]->CreateSub ( this, strSchoolBox[i].c_str() );	
		m_pTypeSchool_Box[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pTypeSchool_Box[i] );

		m_pTypeSchool_Focus[i] = new CUIControl;
		m_pTypeSchool_Focus[i]->CreateSub ( this, strSchoolFocus[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE, CHANGE_SCHOOL_WINDOW_SELECT_SG + i );	
		m_pTypeSchool_Focus[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pTypeSchool_Focus[i] );
	}
	

	m_pInfoTitle = new CBasicTextBox;
	m_pInfoTitle->CreateSub ( this, "RAN_CHANGE_SCHOOL_TITLE_TEXT", UI_FLAG_DEFAULT );
	m_pInfoTitle->SetFont ( pFont12 );
	m_pInfoTitle->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );		
	RegisterControl ( m_pInfoTitle );

	m_pInfoDesc = new CBasicTextBox;
	m_pInfoDesc->CreateSub ( this, "RAN_CHANGE_SCHOOL_DESC_TEXT", UI_FLAG_DEFAULT );
	m_pInfoDesc->SetFont ( pFont08 );
	m_pInfoDesc->SetTextAlign ( TEXT_ALIGN_LEFT );		
	RegisterControl ( m_pInfoDesc );	


	m_pButtonSave = new CBasicTextButton;
	m_pButtonSave->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, CHANGE_SCHOOL_WINDOW_BUTTON_SAVE );
	m_pButtonSave->CreateBaseButton ( "RAN_CHANGE_SCHOOL_OK", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_CHANGE_SCHOOL_WINDOW", 2 ) );
	RegisterControl ( m_pButtonSave );

	m_pButtonClose = new CBasicTextButton;
	m_pButtonClose->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, CHANGE_SCHOOL_WINDOW_BUTTON_CANCEL );
	m_pButtonClose->CreateBaseButton ( "RAN_CHANGE_SCHOOL_CANCEL", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_CHANGE_SCHOOL_WINDOW", 1 ) );
	RegisterControl ( m_pButtonClose );

	ResetAll();
}

void CChangeSchoolWindow::TranslateUIMessage( UIGUID cID, DWORD dwMsg )
{
	CUIWindowEx::TranslateUIMessage( cID, dwMsg );

	switch ( cID )
	{
	case ET_CONTROL_TITLE:
	case ET_CONTROL_TITLE_F:
		{
			if ( (dwMsg & UIMSG_LB_DUP) && CHECK_MOUSE_IN ( dwMsg ) ){
				CInnerInterface::GetInstance().SetDefaultPosInterface( GetWndID() );
			}
		}break;

	case ET_CONTROL_BUTTON:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			{
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
			}
		}break;

	
	case CHANGE_SCHOOL_WINDOW_SELECT_SG:
	case CHANGE_SCHOOL_WINDOW_SELECT_MP:
	case CHANGE_SCHOOL_WINDOW_SELECT_PHX:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) ) {
				int nSelect = cID - CHANGE_SCHOOL_WINDOW_SELECT_SG;
				SchoolMouseIn( nSelect );

				if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
					SchoolSelect( nSelect );
				}
			}

		}break;

	case CHANGE_SCHOOL_WINDOW_BUTTON_SAVE:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				if ( m_nSelectedSchool >= 0 && m_nSelectedSchool < GLSCHOOL_NUM )
				{
					if ( GLGaeaClient::GetInstance().GetCharacter()->m_wSchool == (WORD)m_nSelectedSchool ){
						CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_SAME_SCHOOL") );
					}else{
						GLGaeaClient::GetInstance().GetCharacter()->ReqChangeSchoolMsg( (WORD)m_nSelectedSchool );
						CInnerInterface::GetInstance().HideGroup( GetWndID() );
					}
				}else{
					CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_NOT_SELECTED") );
				}
			}
		}break;

	case CHANGE_SCHOOL_WINDOW_BUTTON_CANCEL:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
			}
		}break;

	};

}

void CChangeSchoolWindow::Update ( int x ,int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	for( int i=0; i<GLSCHOOL_NUM; ++i )
	{
		m_pTypeSchool_Box[i]->SetVisibleSingle( FALSE );
	}

	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( m_nSelectedSchool != m_nSelectedLastSchool )
	{
		UpdateUI();
		m_nSelectedLastSchool = m_nSelectedSchool;
	}
}

void CChangeSchoolWindow::SetVisibleSingle( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		ResetAll();
	}
	else
	{
		
	}
}

void CChangeSchoolWindow::ResetAll()
{
	m_nSelectedSchool = -1;

	for( int i=0; i<GLSCHOOL_NUM; ++i )
	{
		m_pTypeSchool_Off[i]->SetVisibleSingle( TRUE );
		m_pTypeSchool_On[i]->SetVisibleSingle( FALSE );
		m_pTypeSchool_Box[i]->SetVisibleSingle( FALSE );
		m_pTypeSchool_Focus[i]->SetVisibleSingle( TRUE );
	}

	m_pInfoTitle->ClearText();
	m_pInfoDesc->ClearText();
}

void CChangeSchoolWindow::UpdateUI()
{
	for( int i=0; i<GLSCHOOL_NUM; ++i )
	{
		m_pTypeSchool_Off[i]->SetVisibleSingle( TRUE );
		m_pTypeSchool_On[i]->SetVisibleSingle( FALSE );
	}

	m_pInfoTitle->ClearText();
	m_pInfoDesc->ClearText();

	if ( m_nSelectedSchool >= 0 && m_nSelectedSchool < GLSCHOOL_NUM )
	{
		m_pTypeSchool_On[m_nSelectedSchool]->SetVisibleSingle( TRUE );

		m_pInfoTitle->SetText( ID2GAMEEXTEXT("NEW_CHAR_SELECT_SCHOOL_NAME", m_nSelectedSchool ), NS_UITEXTCOLOR::WHITE );	

		CString strText;
		strText.Format ( "NEW_CHAR_SELECT_SCHOOL_DEC%d", m_nSelectedSchool );
		m_pInfoDesc->SetText ( ID2GAMEEXTEXT(strText), NS_UITEXTCOLOR::WHITE );
	}
}

void CChangeSchoolWindow::SchoolSelect( int nSelect )
{
	if ( nSelect < 0 )	return;
	if ( nSelect >= GLSCHOOL_NUM )	return;
	if ( nSelect == m_nSelectedSchool )	return;

	m_nSelectedSchool = nSelect;
}

void CChangeSchoolWindow::SchoolMouseIn( int nSelect )
{
	if ( nSelect < 0 )	return;
	if ( nSelect >= GLSCHOOL_NUM )	return;
	m_pTypeSchool_Box[nSelect]->SetVisibleSingle( TRUE );
}