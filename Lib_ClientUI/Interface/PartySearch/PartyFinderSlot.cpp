#include "StdAfx.h"
#include "PartyFinderSlot.h"
#include "PartyFinderWindow.h"
#include "GLCharacter.h"
#include "GLGaeaClient.h"
#include "GLCharDefine.h"

#include "GLPartyClient.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicTextButton.h"
#include "InnerInterface.h"

#include "UITextControl.h"
#include "GameTextControl.h"
#include "GLItemMan.h"
#include "GLItem.h"

#include "../Lib_Engine/DxCommon/d3dfont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPartyFinderSlot::CPartyFinderSlot()
	: m_pLeaderName(NULL)
	, m_pLeaderInfo(NULL)
	, m_pPartyMap(NULL)
	, m_pPartyInfo(NULL)
	, m_pPartyCount(NULL)
	, m_pSelectParty(NULL)
	, m_pButtonJoin(NULL)
	// need to
	, m_dwGaeaID(GAEAID_NULL)
	, m_sPartyData (PARTYLINKDATA())
	, m_fCooldown(0.0f)
	, m_bCanJoin (true)
	, m_bRequesting (FALSE)
{
	memset ( m_pSchoolMark, 0, sizeof ( CUIControl* ) * nSCHOOL_MARK );
	memset ( m_pClassImg, 0, sizeof ( CUIControl* ) * GLCI_NUM_7CLASS );
}

CPartyFinderSlot::~CPartyFinderSlot ()
{
}

void CPartyFinderSlot::CreateSubControl ()
{
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "PARTYFINDER_SLOT_NODE_BACK" );
	pBasicLineBox->SetVisibleSingle(TRUE);
	RegisterControl ( pBasicLineBox );

	m_pSelectParty = CreateUILineBoxWhiteNoBody("PARTYFINDER_SLOT_LIST_NODE_SELECT");

	m_pLeaderName = CreateStaticControl("PARTYFINDER_LEADER_NAME",pFont8,NS_UITEXTCOLOR::WHITE,TEXT_ALIGN_LEFT);
	m_pLeaderInfo = CreateStaticControl("PARTYFINDER_LEADER_INFO",pFont8,NS_UITEXTCOLOR::WHITE,TEXT_ALIGN_LEFT);
	m_pPartyMap = CreateStaticControl("PARTYFINDER_PARTY_MAP",pFont8,NS_UITEXTCOLOR::WHITE,TEXT_ALIGN_LEFT);
	m_pPartyInfo = CreateStaticControl("PARTYFINDER_PARTY_INFO",pFont8,NS_UITEXTCOLOR::WHITE,TEXT_ALIGN_LEFT);
	m_pPartyCount = CreateStaticControl("PARTYFINDER_PARTY_COUNT",pFont8,NS_UITEXTCOLOR::WHITE,TEXT_ALIGN_LEFT);

	{
		CUIControl	TempControl;
		TempControl.Create ( 1, "PARTYFINDER_WINDOW_INFORMATION_SCHOOL" );

		const UIRECT& rcLocalPos = TempControl.GetLocalPos ();

		CString strSchoolMark[] = 
		{
			"PARTYFINDER_SCHOOL_LOGO_0",
			"PARTYFINDER_SCHOOL_LOGO_1",
			"PARTYFINDER_SCHOOL_LOGO_2"
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
			strTemp.Format ( "PARTYFINDER_CLASS_IMAGE%d",i); 	
			m_pClassImg[i] = new CUIControl;
			m_pClassImg[i]->CreateSub ( this, strTemp, UI_FLAG_DEFAULT, ICON_CLASS_IMAGE0 + i );	
			m_pClassImg[i]->SetVisibleSingle ( FALSE );
			m_pClassImg[i]->SetTransparentOption( TRUE );
			RegisterControl ( m_pClassImg[i] );
		}
	}

	m_pButtonJoin = new CBasicTextButton;
	m_pButtonJoin->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, PARTYFINDER_JOIN );
	m_pButtonJoin->CreateBaseButton ( "PARTYFINDER_JOIN", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, ID2GAMEWORD("PARTYFINDER_SLOT", 5 ) );
	RegisterControl ( m_pButtonJoin );

	DWORD dwPARTYID = GLPartyClient::GetInstance().GetPartyID();

	if ( dwPARTYID != PARTY_NULL )
		m_pButtonJoin->SetVisibleSingle(FALSE);

}

CUIControl*	CPartyFinderSlot::CreateControl ( const char* szControl, const UIGUID& cID )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl, UI_FLAG_DEFAULT, cID );
	RegisterControl ( pControl );
	return pControl;
}
CBasicTextBox* CPartyFinderSlot::CreateStaticControl ( char* szControlKeyword, CD3DFontPar* pFont, D3DCOLOR dwColor, int nAlign )
{
	CBasicTextBox* pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, szControlKeyword );
	pStaticText->SetFont ( pFont );
	pStaticText->SetTextAlign ( nAlign );
	RegisterControl ( pStaticText );
	return pStaticText;
}
CBasicLineBox* CPartyFinderSlot::CreateUILineBoxWhiteNoBody( char* szBaseControl )
{
	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( szBaseControl );
	RegisterControl ( pLineBox );

	return pLineBox;
}

void CPartyFinderSlot::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case PARTYFINDER_JOIN:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			{
				if ( !IsJoinAvailable() ) return;
				SetRequesting(TRUE);

				// cooldown based pa din master sa partysearch button
				GLGaeaClient::GetInstance().GetCharacter()->ReqPartySearchJoin( m_dwGaeaID, m_sPartyData.dwPartyID  );
				/*
				CString str;
				str.Format("Selected: m_dwGaeaID: %d PartyID: %d", m_sPartyData.m_dwGaeaID , m_sPartyData.dwPartyID );
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::ENABLE, str );
				*/
				CInnerInterface::GetInstance().SetPartyRequestModalClose();
			}
		}break;
	};
}
void CPartyFinderSlot::Update ( int x ,int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{	
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( IsRequesting() )
	{
		if (m_pButtonJoin)
			m_pButtonJoin->SetVisibleSingle ( FALSE );

		m_fCooldown += fElapsedTime;

		if ( m_fCooldown >= 10.0f )
		{
			m_fCooldown = 0.0f;
			SetRequesting(FALSE);
			if (m_pButtonJoin)
			{
				DWORD dwPARTYID = GLPartyClient::GetInstance().GetPartyID();

				if (dwPARTYID == PARTY_NULL)
					m_pButtonJoin->SetVisibleSingle (TRUE);
			}
		}	
	}


	
}
BOOL CPartyFinderSlot::IsJoinAvailable()
{
	DWORD dwPARTYID = GLPartyClient::GetInstance().GetPartyID();
	if ( dwPARTYID != PARTY_NULL ) return FALSE; 
	if( IsRequesting() ) return FALSE;

	return TRUE;
}
void CPartyFinderSlot::SlotSet( PARTYLINKDATA sPartyData )
{
	SlotReset();
	m_sPartyData = sPartyData;
	m_dwGaeaID = sPartyData.m_dwGaeaID;

	CString strText;

	if ( m_pLeaderName )
	{
		CBasicTextBox::STEXTMULTIWORDVEC vectext;
		CString strData, strName;

		strData.Format("%s: ",ID2GAMEWORD( "PARTYFINDER_SLOT", 0 ));
		CBasicTextBox::STEXTMULTIWORD word_item;
		word_item.strWord = strData.GetString();
		word_item.dwColor = NS_UITEXTCOLOR::WHITE;
		vectext.push_back( word_item );

		strName.Format( "%s", sPartyData.m_szName );
		CBasicTextBox::STEXTMULTIWORD word_itemdata;
		word_itemdata.strWord = strName.GetString();
		word_itemdata.dwColor = NS_UITEXTCOLOR::GREENYELLOW;
		vectext.push_back( word_itemdata );

		m_pLeaderName->AddMultiTextNoSplit( vectext );
	}
	
	if ( m_pLeaderInfo )
	{
		strText.Format ( "%s / %s",COMMENT::CHARCLASS[CharClassToIndex( sPartyData.m_emClass)].c_str(),ID2GAMEWORD("ACADEMY_NAME",sPartyData.m_wSchool) );
		m_pLeaderInfo->SetText( strText, NS_UITEXTCOLOR::WHITE );
	}
	
	if ( m_pPartyMap )
	{
		const CString strMapName = GLGaeaClient::GetInstance().GetMapName ( sPartyData.m_sMapID );
		CBasicTextBox::STEXTMULTIWORDVEC vectext;
		CString strData, strName;

		strData.Format("%s: ",ID2GAMEWORD( "PARTYFINDER_SLOT", 1 ));
		CBasicTextBox::STEXTMULTIWORD word_item;
		word_item.strWord = strData.GetString();
		word_item.dwColor = NS_UITEXTCOLOR::WHITE;
		vectext.push_back( word_item );

		strName.Format( "%s", strMapName );
		CBasicTextBox::STEXTMULTIWORD word_itemdata;
		word_itemdata.strWord = strName.GetString();
		word_itemdata.dwColor = NS_UITEXTCOLOR::ORNAGERED;
		vectext.push_back( word_itemdata );

		m_pPartyMap->AddMultiTextNoSplit( vectext );
	}
	
	if ( m_pPartyInfo )
	{
		CBasicTextBox::STEXTMULTIWORDVEC vectext;
		CString strItem, strItemData, strGold, strGoldData;
		
		strItem.Format("%s:",ID2GAMEWORD( "PARTYFINDER_SLOT", 2 ));
		CBasicTextBox::STEXTMULTIWORD word_item;
		word_item.strWord = strItem.GetString();
		word_item.dwColor = NS_UITEXTCOLOR::WHITE;
		vectext.push_back( word_item );

		strItemData = ID2GAMEWORD("PARTY_MODAL_ITEM_OPTION", sPartyData.sOption.emGET_ITEM );
		CBasicTextBox::STEXTMULTIWORD word_itemdata;
		word_itemdata.strWord = strItemData.GetString();
		word_itemdata.dwColor = NS_UITEXTCOLOR::PRIVATE;
		vectext.push_back( word_itemdata );

		strGold.Format("/%s:",ID2GAMEWORD( "PARTYFINDER_SLOT", 3 ));
		CBasicTextBox::STEXTMULTIWORD word_gold;
		word_gold.strWord = strGold.GetString();
		word_gold.dwColor = NS_UITEXTCOLOR::WHITE;
		vectext.push_back( word_gold );

		strGoldData =  ID2GAMEWORD("PARTY_MODAL_MONEY_OPTION", sPartyData.sOption.emGET_MONEY );
		CBasicTextBox::STEXTMULTIWORD word_golddata;
		word_golddata.strWord = strGoldData.GetString();
		word_golddata.dwColor = NS_UITEXTCOLOR::PRIVATE;
		vectext.push_back( word_golddata );

		m_pPartyInfo->AddMultiTextNoSplit( vectext );
	}

	if ( m_pPartyCount )
	{
		//strText.Format("%s: %d/%",ID2GAMEWORD( "PARTYFINDER_SLOT", 4 ), sPartyData.m_wMembers );
		strText.Format("%s: %d/%d",ID2GAMEWORD( "PARTYFINDER_SLOT", 4 ), sPartyData.m_wMembers, MAXPARTY );		
		m_pPartyCount->SetText( strText, NS_UITEXTCOLOR::WHITE );
	}

	m_pSchoolMark[sPartyData.m_wSchool]->SetVisibleSingle ( TRUE );
	m_pClassImg[CharClassToIndex(sPartyData.m_emClass)]->SetVisibleSingle ( TRUE );
}
void CPartyFinderSlot::SlotReset()
{
	m_sPartyData = PARTYLINKDATA();
	m_dwGaeaID = GAEAID_NULL;
	
	if ( m_pLeaderName )	m_pLeaderName->ClearText();
	if ( m_pLeaderInfo )	m_pLeaderInfo->ClearText();
	if ( m_pPartyMap )	m_pPartyMap->ClearText();
	if ( m_pPartyInfo )	m_pPartyInfo->ClearText();
	if ( m_pPartyCount )	m_pPartyCount->ClearText();

	if ( m_pSelectParty )	m_pSelectParty->SetVisibleSingle( FALSE );
	for ( int i = 0; i < nSCHOOL_MARK; ++i ) m_pSchoolMark[i]->SetVisibleSingle ( FALSE );
	for ( int i = 0 ; i < GLCI_NUM_6CLASS; ++i ) m_pClassImg[i]->SetVisibleSingle(FALSE);
}
void CPartyFinderSlot::SlotSelect( BOOL bSelect )
{
	if ( m_pSelectParty )
		m_pSelectParty->SetVisibleSingle( bSelect );
}