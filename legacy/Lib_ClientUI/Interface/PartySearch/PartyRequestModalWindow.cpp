#include "StdAfx.h"
#include "PartyRequestModalWindow.h"
#include "GLCharacter.h"
#include "GLGaeaClient.h"
#include "GLCharDefine.h"

#include "DxGlobalStage.h"
#include "GLPartyClient.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicTextButton.h"
#include "InnerInterface.h"
#include "ModalCallerID.h"

#include "UITextControl.h"
#include "GameTextControl.h"
#include "GLItemMan.h"
#include "GLItem.h"

#include "../Lib_Engine/DxCommon/d3dfont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPartyRequestModalWindow::CPartyRequestModalWindow()
	: m_pName(NULL)
	, m_pLevel(NULL)
	, m_pInfo(NULL)
	, m_pOK(NULL)
	, m_pCANCEL(NULL)
	, m_dwRequestorGaeaID(GAEAID_NULL)
	, m_dwRequestorLevel(0)
	, m_dwRequestorSchool(0)
	, m_emRequestorClass(GLCC_BRAWLER_M)
{
	memset ( m_pSchoolMark, 0, sizeof ( CUIControl* ) * nSCHOOL_MARK );
	memset ( m_pClassImg, 0, sizeof ( CUIControl* ) * GLCI_NUM_7CLASS );
}

CPartyRequestModalWindow::~CPartyRequestModalWindow ()
{
}
void CPartyRequestModalWindow::CreateSubControl ()
{
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "PARTYREQUEST_MODAL_NODE_BACK" );
	pBasicLineBox->SetVisibleSingle(TRUE);
	RegisterControl ( pBasicLineBox );
	
	CString strText;

	m_pText = CreateStaticControl("PARTYREQUEST_MODAL_TEXT",pFont8,NS_UITEXTCOLOR::WHITE,TEXT_ALIGN_CENTER_X);
	strText.Format("%s",ID2GAMEWORD( "PARTYREQUEST_MODAL", 2 ));
	m_pText->SetText( strText, NS_UITEXTCOLOR::WHITE );

	m_pName = CreateStaticControl("PARTYREQUEST_MODAL_NAME",pFont8,NS_UITEXTCOLOR::WHITE,TEXT_ALIGN_LEFT);
	m_pLevel = CreateStaticControl("PARTYREQUEST_MODAL_LEVEL",pFont8,NS_UITEXTCOLOR::WHITE,TEXT_ALIGN_LEFT);
	m_pInfo = CreateStaticControl("PARTYREQUEST_MODAL_INFO",pFont8,NS_UITEXTCOLOR::WHITE,TEXT_ALIGN_LEFT);
	
	{
		CUIControl	TempControl;
		TempControl.Create ( 1, "PARTYREQUEST_MODAL_INFORMATION_SCHOOL" );

		const UIRECT& rcLocalPos = TempControl.GetLocalPos ();

		CString strSchoolMark[] = 
		{
			"PARTYREQUEST_SCHOOL_LOGO_0",
			"PARTYREQUEST_SCHOOL_LOGO_1",
			"PARTYREQUEST_SCHOOL_LOGO_2"
		};

		for ( int i = 0; i < nSCHOOL_MARK; ++i )
		{
			m_pSchoolMark[i] = CreateControl ( strSchoolMark[i].GetString() );
			m_pSchoolMark[i]->SetLocalPos ( rcLocalPos );
			m_pSchoolMark[i]->SetVisibleSingle ( FALSE );
		}

		CString strTemp;
		for ( int i = 0 ; i < GLCI_NUM_7CLASS; ++i ) 
		{
			strTemp.Format ( "PARTYREQUEST_CLASS_IMAGE%d",i); 	
			m_pClassImg[i] = new CUIControl;
			//m_pClassImg[i]->CreateSub ( this, strTemp, UI_FLAG_DEFAULT, ICON_CLASS_IMAGE0 + i );	
			m_pClassImg[i] = CreateControl(strTemp);
			m_pClassImg[i]->SetVisibleSingle ( FALSE );
			m_pClassImg[i]->SetTransparentOption( TRUE );
			RegisterControl ( m_pClassImg[i] );
		}
	}
	
	m_pOK = new CBasicTextButton;
	m_pOK->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, PARTYREQUEST_MODAL_OK );
	m_pOK->CreateBaseButton ( "PARTYREQUEST_MODAL_OK", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, ID2GAMEWORD("PARTYFINDER_SLOT", 6 ) );
	RegisterControl ( m_pOK );

	m_pCANCEL = new CBasicTextButton;
	m_pCANCEL->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, PARTYREQUEST_MODAL_CANCEL );
	m_pCANCEL->CreateBaseButton ( "PARTYREQUEST_MODAL_CANCEL", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, ID2GAMEWORD("PARTYFINDER_SLOT", 7 ) );
	RegisterControl ( m_pCANCEL );
}

CUIControl*	CPartyRequestModalWindow::CreateControl ( const char* szControl, const UIGUID& cID )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl, UI_FLAG_DEFAULT, cID );
	RegisterControl ( pControl );
	return pControl;
}
CBasicTextBox* CPartyRequestModalWindow::CreateStaticControl ( char* szControlKeyword, CD3DFontPar* pFont, D3DCOLOR dwColor, int nAlign )
{
	CBasicTextBox* pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, szControlKeyword );
	pStaticText->SetFont ( pFont );
	pStaticText->SetTextAlign ( nAlign );
	RegisterControl ( pStaticText );
	return pStaticText;
}
void CPartyRequestModalWindow::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	switch ( ControlID )
	{
	case PARTYREQUEST_MODAL_OK:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			{
				GLMSG::SNET_PARTY_SEARCH_LURE_TAR_ANS NetMsg;
				NetMsg.dwRequestorID = m_dwRequestorGaeaID;
				NetMsg.dwPartyID = PARTY_NULL;
				NetMsg.emAnswer = EMPARTY_LURE_OK;
				NetMsg.sOption = GLPartyClient::GetInstance().GetOption();
				NETSEND ( &NetMsg );

				CInnerInterface::GetInstance().SetPartyRequestModalClose();
			}
		}break;
	case ET_CONTROL_BUTTON:
	case PARTYREQUEST_MODAL_CANCEL:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			{
				GLMSG::SNET_PARTY_SEARCH_LURE_TAR_ANS NetMsg;
				NetMsg.dwRequestorID = m_dwRequestorGaeaID;
				NetMsg.dwPartyID = PARTY_NULL;
				NetMsg.emAnswer = EMPARTY_LURE_REFUSE;
				NETSEND ( &NetMsg );

				CInnerInterface::GetInstance().SetPartyRequestModalClose();
			}
		}break;
	};
	CUIWindowEx::TranslateUIMessage ( ControlID, dwMsg );
}
void CPartyRequestModalWindow::SetVisibleSingle ( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );	
}
void CPartyRequestModalWindow::PartyRequestSet( const CString& strName, DWORD dwGaeaID, DWORD dwLevel, DWORD dwSchool, EMCHARCLASS emRequestorClass  )
{
	PartyRequestReSet();
	CString strText;
	m_strRequestorName = strName;
	m_dwRequestorGaeaID = dwGaeaID;
	m_dwRequestorLevel = dwLevel;
	m_dwRequestorSchool = dwSchool;
	m_emRequestorClass = emRequestorClass;

	if ( m_pName )
	{
		CBasicTextBox::STEXTMULTIWORDVEC vectext;
		CString strData, strName;

		strData.Format("%s: ",ID2GAMEWORD( "PARTYREQUEST_MODAL", 0 ));
		CBasicTextBox::STEXTMULTIWORD word_item;
		word_item.strWord = strData.GetString();
		word_item.dwColor = NS_UITEXTCOLOR::WHITE;
		vectext.push_back( word_item );

		strName.Format( "%s", m_strRequestorName );
		CBasicTextBox::STEXTMULTIWORD word_itemdata;
		word_itemdata.strWord = strName.GetString();
		word_itemdata.dwColor = NS_UITEXTCOLOR::GREENYELLOW;
		vectext.push_back( word_itemdata );

		m_pName->AddMultiTextNoSplit( vectext );
	}

	if ( m_pLevel )
	{
		CBasicTextBox::STEXTMULTIWORDVEC vectext;
		CString strData, strName;

		strData.Format("%s: ",ID2GAMEWORD( "PARTYREQUEST_MODAL", 1 ));
		CBasicTextBox::STEXTMULTIWORD word_item;
		word_item.strWord = strData.GetString();
		word_item.dwColor = NS_UITEXTCOLOR::WHITE;
		vectext.push_back( word_item );

		strName.Format( "%d", m_dwRequestorLevel );
		CBasicTextBox::STEXTMULTIWORD word_itemdata;
		word_itemdata.strWord = strName.GetString();
		word_itemdata.dwColor = NS_UITEXTCOLOR::PRIVATE;
		vectext.push_back( word_itemdata );

		m_pLevel->AddMultiTextNoSplit( vectext );
	}
	
	if ( m_pInfo )
	{
		CString strClass = COMMENT::CHARCLASS[CharClassToIndex( m_emRequestorClass )].c_str();
		CString strSchool = ID2GAMEWORD("ACADEMY_NAME",m_dwRequestorSchool );

		strText.Format("%s / %s", strClass, strSchool);
		m_pInfo->SetText( strText, NS_UITEXTCOLOR::WHITE );
		
		m_pSchoolMark[m_dwRequestorSchool]->SetVisibleSingle ( TRUE );
		m_pClassImg[CharClassToIndex(m_emRequestorClass)]->SetVisibleSingle ( TRUE );
	}
}
void CPartyRequestModalWindow::PartyRequestReSet()
{
	if ( m_pName )	m_pName->ClearText();
	if ( m_pLevel )	m_pLevel->ClearText();
	if ( m_pInfo )	m_pInfo->ClearText();

	for ( int i = 0; i < nSCHOOL_MARK; ++i ) m_pSchoolMark[i]->SetVisibleSingle ( FALSE );
	for ( int i = 0 ; i < GLCI_NUM_7CLASS; ++i ) m_pClassImg[i]->SetVisibleSingle(FALSE);
}