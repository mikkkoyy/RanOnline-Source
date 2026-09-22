/////////////////////////////////////////////////////////////////////////////
//	FileName	: PartyFinderWindow.cpp
//	Project		: RanClientUILib
//	Coder		: KHALEL
#include "stdafx.h"

#include "PartyFinderWindow.h"
#include "PartyFinderSlot.h"

#include "BasicButton.h"
#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicTextButton.h"
#include "GameTextControl.h"
#include "InnerInterface.h"
#include "UITextControl.h"

#include "BasicTextBoxEx.h"
#include "BasicScrollBarEx.h"
#include "BasicScrollThumbFrame.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"

#include "GLCrowData.h"
#include "GLGaeaClient.h"
#include "GLItemMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPartyFinderWindow::CPartyFinderWindow()
	: m_pListTextBox(NULL)
	, m_pListScrollBar(NULL)
	, m_pPartySearch(NULL)
	, m_pPartyResult(NULL)
	, m_fUpdateTime (0.0f)
	, m_pButtonSearch(NULL)
	, m_pButtonPrev(NULL)
	, m_pButtonNext(NULL)
	, m_pTextPage(NULL)
	, m_nCurrentPage(0)
	, m_nMaxPage(0)
	, m_fCooldown(0.0f)
	, m_nSelectIndexType(0)
	, m_bCanJoin(false)
{
	for( int i=0; i<PARTYFINDER_MAX_SLOT; ++i )
		m_pPartySlot[i] = NULL;
}
CPartyFinderWindow::~CPartyFinderWindow ()
{
}
void CPartyFinderWindow::CreateSubControl ()
{	
	CreateUILineBoxQuestList("PARTYFINDER_SEARCH_PARTY_LBHEAD");
	CreateUILineBoxQuestList("PARTYFINDER_SEARCH_PARTY_LB_BODY");
	
	CreateUILineBoxQuestList("PARTYFINDER_SEARCH_RESULT_LBHEAD");
	CreateUILineBoxQuestList("PARTYFINDER_SEARCH_RESULT_LB_BODY");
	CreateUILineBoxQuestList("PARTYFINDER_SEARCH_RESULT_LBBOTTOM");

	CD3DFontPar* pFont9	= DxFontMan::GetInstance().LoadDxFont(_DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG);
	m_pPartySearch = CreateStaticControl("PARTYFINDER_SEARCH_PARTY_LBHEAD",pFont9,NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y);
	m_pPartyResult = CreateStaticControl("PARTYFINDER_SEARCH_RESULT_LBHEAD",pFont9,NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y);

	m_pListTextBox = new CBasicTextBoxEx;
	m_pListTextBox->CreateSub ( this, "PARTYFINDER_SEARCH_PARTY_LOCATION", UI_FLAG_DEFAULT, PARTYFINDER_SEARCH_PARTY_LOCATION );
	m_pListTextBox->SetFont ( pFont9 );		
	m_pListTextBox->SetLineInterval ( 3.0f );
	m_pListTextBox->SetSensitive ( true );
	m_pListTextBox->SetLimitLine ( 10000 );
	RegisterControl ( m_pListTextBox );

	int nTotalLine = m_pListTextBox->GetVisibleLine ();
	
	m_pListScrollBar = new CBasicScrollBarEx;
	m_pListScrollBar->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, PARTYFINDER_SEARCH_PARTY_LOCATION_SCROLLBAR );
	m_pListScrollBar->CreateBaseScrollBar ( "PARTYFINDER_SEARCH_PARTY_LOCATION_SCROLL" );
	m_pListScrollBar->GetThumbFrame()->SetState ( 1, nTotalLine );
	RegisterControl ( m_pListScrollBar );


	// Slot 
	std::string strSlot[PARTYFINDER_MAX_SLOT] =
	{
		"PARTYFINDER_SLOT_NODE_00",
		"PARTYFINDER_SLOT_NODE_01",
		"PARTYFINDER_SLOT_NODE_02",
		"PARTYFINDER_SLOT_NODE_03",
		"PARTYFINDER_SLOT_NODE_04",
		"PARTYFINDER_SLOT_NODE_05",
		"PARTYFINDER_SLOT_NODE_06",
		"PARTYFINDER_SLOT_NODE_07",
	};

	for( int i=0; i<PARTYFINDER_MAX_SLOT; ++i )
	{
		m_pPartySlot[i] = new CPartyFinderSlot;
		m_pPartySlot[i]->CreateSub ( this, strSlot[i].c_str(), UI_FLAG_DEFAULT, PARTYFINDER_SLOT00 + i );
		m_pPartySlot[i]->CreateSubControl ();
		m_pPartySlot[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pPartySlot[i] );
	}
	
	m_pTextPage = new CBasicTextBox;
	m_pTextPage->CreateSub ( this, "PARTYFINDER_CUR_PAGE_TEXT" );
	m_pTextPage->SetFont ( pFont9 );
	m_pTextPage->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextPage->SetText( "0" );
	RegisterControl ( m_pTextPage );

	m_pButtonPrev = new CBasicButton;
	m_pButtonPrev->CreateSub ( this, "PARTYFINDER_PRE_PAGE_BUTTON", UI_FLAG_DEFAULT, PARTYFINDER_PREV_BUTTON );
	m_pButtonPrev->CreateFlip ( "PARTYFINDER_PRE_PAGE_BUTTON_F", CBasicButton::CLICK_FLIP );
	m_pButtonPrev->CreateMouseOver ( "PARTYFINDER_PRE_PAGE_BUTTON_OVER" );
	RegisterControl ( m_pButtonPrev );

	m_pButtonNext = new CBasicButton;
	m_pButtonNext->CreateSub ( this, "PARTYFINDER_NEXT_PAGE_BUTTON", UI_FLAG_DEFAULT, PARTYFINDER_NEXT_BUTTON );
	m_pButtonNext->CreateFlip ( "PARTYFINDER_NEXT_PAGE_BUTTON_F", CBasicButton::CLICK_FLIP );
	m_pButtonNext->CreateMouseOver ( "PARTYFINDER_NEXT_PAGE_BUTTON_OVER" );
	RegisterControl ( m_pButtonNext );

	m_pButtonSearch = new CBasicTextButton;
	m_pButtonSearch->CreateSub ( this, "BASIC_TEXT_BUTTON40", UI_FLAG_XSIZE, PARTYFINDER_SEARCH_PARTY_BUTTON );
	m_pButtonSearch->CreateBaseButton ( "PARTYFINDER_SEARCH_PARTY_BUTTON", CBasicTextButton::SIZE40, CBasicButton::CLICK_FLIP, "Search" );
	RegisterControl ( m_pButtonSearch );
}
CBasicLineBox* CPartyFinderWindow::CreateUILineBoxQuestList( char* szBaseControl )
{
	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( szBaseControl );
	RegisterControl ( pLineBox );
	return pLineBox;
}
CBasicTextBox* CPartyFinderWindow::CreateStaticControl ( char* szControlKeyword, CD3DFontPar* pFont, D3DCOLOR dwColor, int nAlign )
{
	CBasicTextBox* pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, szControlKeyword );
	pStaticText->SetFont ( pFont );
	pStaticText->SetTextAlign ( nAlign );
	RegisterControl ( pStaticText );
	return pStaticText;
}
void CPartyFinderWindow::SetVisibleSingle ( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );	
	if( bVisible )
	{	
	}
	else
	{
	}
}
void CPartyFinderWindow::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	switch( ControlID )
	{
	case PARTYFINDER_SLOT00:
	case PARTYFINDER_SLOT01:
	case PARTYFINDER_SLOT02:
	case PARTYFINDER_SLOT03:
	case PARTYFINDER_SLOT04:
	case PARTYFINDER_SLOT05:
	case PARTYFINDER_SLOT06:
	case PARTYFINDER_SLOT07:
		{			
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				int nSelect = ControlID - PARTYFINDER_SLOT00;
				if ( m_pPartySlot[nSelect] )
				{
					if ( CHECK_LB_UP_LIKE ( dwMsg ) )	
					{
						//CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, "Slot Party Select: %u", nSelect );
						SelectParty(nSelect);
					}
				}
			}
		}break;
	case PARTYFINDER_PREV_BUTTON:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( CHECK_LB_UP_LIKE ( dwMsg ) )	
				{
					m_nCurrentPage --;
					if ( m_nCurrentPage < 1 )
						m_nCurrentPage = 1;
					ShowParty( m_nCurrentPage );
				}
			}
		}break;
	case PARTYFINDER_NEXT_BUTTON:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( CHECK_LB_UP_LIKE ( dwMsg ) )	
				{
					if ( m_nCurrentPage >= m_nMaxPage )	return;

					m_nCurrentPage ++;
					ShowParty( m_nCurrentPage );
				}
			}
		}break;
	case PARTYFINDER_SEARCH_PARTY_LOCATION:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				int nIndex = m_pListTextBox->GetSelectPos ();
				if ( m_pListTextBox->GetCount () <= nIndex || nIndex < 0  ) return ;

				if ( UIMSG_LB_UP & dwMsg )
				{
					DWORD dwType = m_pListTextBox->GetTextData ( nIndex );
					m_nSelectIndexType = nIndex;
//					SelectType( dwType ); // no need button based kasi tayo,bali
				}

				m_pListTextBox->SetUseOverColor ( TRUE );
				m_pListTextBox->SetOverColor ( nIndex, NS_UITEXTCOLOR::ORANGE );
			}
		}
		break;
	case PARTYFINDER_SEARCH_PARTY_BUTTON:
		{
			if ( CHECK_MOUSE_IN (dwMsg) )
			{
				if ( CHECK_LB_UP_LIKE ( dwMsg )  )
				{					
					SelectPartyArea (m_nSelectIndexType); // once ma call, page 1
				}
			}

		}break;
	};
	CUIWindowEx::TranslateUIMessage ( ControlID, dwMsg );
}
void CPartyFinderWindow::Update ( int x ,int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	m_pListTextBox->SetUseOverColor ( FALSE );
	if ( m_pListTextBox->GetCount () && 0 <= m_nSelectIndexType )
		m_pListTextBox->SetUseTextColor ( m_nSelectIndexType, FALSE );

	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
	
	if ( m_pListTextBox->GetCount () && 0 <= m_nSelectIndexType )
	{
		m_pListTextBox->SetUseTextColor ( m_nSelectIndexType, TRUE );
		m_pListTextBox->SetTextColor ( m_nSelectIndexType, NS_UITEXTCOLOR::ORANGE );
	}

	CBasicScrollThumbFrame* const pThumbFrame = m_pListScrollBar->GetThumbFrame ();
	const int nTotalLine = m_pListTextBox->GetTotalLine ();
	const int nLinePerOneView = m_pListTextBox->GetVisibleLine ();
	pThumbFrame->SetState ( nTotalLine, nLinePerOneView );
	if ( nLinePerOneView < nTotalLine )
	{
		const int nMovableLine = nTotalLine - nLinePerOneView;
		float fPercent = pThumbFrame->GetPercent ();
		int nPos = (int)floor(fPercent * nMovableLine);
		m_pListTextBox->SetCurLine ( nPos );			
	}
	
	m_fUpdateTime += fElapsedTime;
	if ( m_fUpdateTime >= 0.5f )
	{
		SetTextInfo();
		m_fUpdateTime = 0.0f;
	}

	m_fCooldown += fElapsedTime;
	if ( m_fCooldown >= 10.0f )
	{
		if ( m_pButtonSearch )
		m_pButtonSearch->SetVisibleSingle(TRUE);
	}

}
void CPartyFinderWindow::SetTextInfo()
{
	if ( m_pPartySearch )
	{
		m_pPartySearch->ClearText();
		m_pPartySearch->AddText( ID2GAMEWORD( "PARTYFINDER_WINDOW", 0 ), NS_UITEXTCOLOR::WHITE );
	}
	
	if ( m_pPartyResult )
	{
		m_pPartyResult->ClearText();
		m_pPartyResult->AddText( ID2GAMEWORD( "PARTYFINDER_WINDOW", 1 ), NS_UITEXTCOLOR::WHITE );
	}
	//CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}
void CPartyFinderWindow::InitPartySearch()
{
	PartyResetInfo();

	for ( int k = 0; k < 18; k++ ) // add k < to number map
	{	
		CString strTemp;

		strTemp.Format( "%s", ID2GAMEWORD( "PARTYFINDER_AREAS", k) );
		int nIndex = m_pListTextBox->AddText ( strTemp.GetString(), NS_UITEXTCOLOR::WHITE );
		m_pListTextBox->SetTextData ( nIndex, k );
	}

	//select first line 
	if ( m_pListTextBox->GetCount () > 0 )
	{
		m_pListTextBox->SetSelectPos( 0 );
		int nIndex = m_pListTextBox->GetSelectPos ();
		if ( m_pListTextBox->GetCount () <= nIndex || nIndex < 0  ) return;
		DWORD dwType = m_pListTextBox->GetTextData ( nIndex );
		m_nSelectIndexType = nIndex;
		//SelectPartyArea( dwType ); // ETO DI TO NEED, KASE MAGBABASED TAYO SA  PARTY SEARCH BUTTON
		m_pListTextBox->SetUseOverColor ( TRUE );
		m_pListTextBox->SetOverColor ( nIndex, NS_UITEXTCOLOR::ORANGE );
	}
	
	// FOR TEST PURPOSE LANG TO, THIS FUNCTION WILL BE CALLED ONLY WHEN THE BUTTON IS PRESSED
	/*SelectPartyArea (m_nSelectIndexType);*/
}
void CPartyFinderWindow::PartyResetInfo()
{
	/**MAP CLEAR **/
	m_nCurrentPage = 1;
	m_nMaxPage = 1;

	m_nSelectIndexType = -1;

	m_vecPartySearch.clear();
	m_vecPartySearchView.clear();
	m_mapPartySearch.clear();
	m_pListTextBox->ClearText();

	for( int i=0; i<PARTYFINDER_MAX_SLOT; ++i )
	{
		m_pPartySlot[i]->SlotReset();
		m_pPartySlot[i]->SetVisibleSingle(FALSE);
	}
}
void CPartyFinderWindow::SelectParty( int nSelect )
{
	for ( int i=0; i< PARTYFINDER_MAX_SLOT; ++ i )
	{
		if ( m_pPartySlot[i] )
			m_pPartySlot[i]->SlotSelect(FALSE);
	}

	if ( nSelect >= 0 && nSelect < PARTYFINDER_MAX_SLOT )
	{
		if ( m_pPartySlot[nSelect] )
		{
			if ( m_pPartySlot[nSelect]->IsVisible() )
				m_pPartySlot[nSelect]->SlotSelect(TRUE);
		}
	}
}

void CPartyFinderWindow::ShowParty( int nPage )
{
	for( int i=0; i<PARTYFINDER_MAX_SLOT; ++i )
	{
		m_pPartySlot[i]->SlotReset();
		m_pPartySlot[i]->SetVisibleSingle(FALSE);
	}
	SelectParty( PARTYFINDER_MAX_SLOT );

	if ( nPage < 1 )	return;

	int nEnd = nPage * PARTYFINDER_MAX_SLOT;
	int nStart = nEnd - PARTYFINDER_MAX_SLOT;
	int nSlot = 0;

	for ( int i=nStart; i<nEnd; ++i )
	{
		if ( i >= m_vecPartySearchView.size() )	continue;

		if ( m_pPartySlot[nSlot] )
		{
			m_pPartySlot[nSlot]->SetVisibleSingle( TRUE );
			m_pPartySlot[nSlot]->SlotSet( m_vecPartySearchView[i] );
			nSlot ++;
		}
	}

	if ( m_pTextPage )
	{
		CString strTemp;
		strTemp.Format( "%d", nPage );
		m_pTextPage->ClearText();
		m_pTextPage->SetText( strTemp.GetString() );
	}
}
void CPartyFinderWindow::SelectPartyArea ( int nIndex )
{
	m_vecPartySearchView.clear();
	m_nCurrentPage = 1;

	m_vecPartySearch = GLGaeaClient::GetInstance().GetCharacter()->m_vecPartySearch; 

	/** TEST PUSH BACK **/
	DWORD dwSIZE = static_cast<DWORD>(m_vecPartySearch.size());
	int nCounter = 0;

	for ( DWORD i=0; i<dwSIZE; ++i )
	{
		const PARTYLINKDATA &sParty = m_vecPartySearch[i];
		m_vecPartySearch[i].dwIndex = nCounter;
		
		m_mapPartySearch[sParty.dwPartyID] = sParty;
		nCounter++;
	}

	for( MAPPARTYSEARCH_ITER iter = m_mapPartySearch.begin();
		iter != m_mapPartySearch.end(); ++iter )
	{
		PARTYLINKDATA& sPartyData = (*iter).second;
		if ( nIndex == 1 && sPartyData.m_sMapID.wMainID != 22 ) continue; // Market
		else if ( nIndex == 2 && sPartyData.m_sMapID.wMainID != 2 ) continue; // SG
		else if ( nIndex == 3 && sPartyData.m_sMapID.wMainID != 5 ) continue; // MP
		else if ( nIndex == 4 && sPartyData.m_sMapID.wMainID != 8 ) continue; // PH
		else if ( nIndex == 5 && sPartyData.m_sMapID.wMainID != 16 ) continue; // ศูนย์การค้า
		else if ( nIndex == 6 && sPartyData.m_sMapID.wMainID != 23 ) continue; // ลานประลอง
		else if ( nIndex == 7 && sPartyData.m_sMapID.wMainID != 27 ) continue; // คุกเอกชน
		else if ( nIndex == 8 && sPartyData.m_sMapID.wMainID != 28 ) continue; // เขตวินาศภัย
		else if ( nIndex == 9 && sPartyData.m_sMapID.wMainID != 29 ) continue; // เขตธรณีพิบัติ
		else if ( nIndex == 10 && sPartyData.m_sMapID.wMainID != 32 ) continue; // เขตทดสอบเรือนจำ
		else if ( nIndex == 11 && sPartyData.m_sMapID.wMainID != 34 ) continue; // โตเกียว
		else if ( nIndex == 12 && sPartyData.m_sMapID.wMainID != 35 ) continue; // โตเกียว2
		else if ( nIndex == 13 && sPartyData.m_sMapID.wMainID != 36 ) continue; // โตเกียว3
		else if ( nIndex == 14 && sPartyData.m_sMapID.wMainID != 37 ) continue; // โตเกียว4
		else if ( nIndex == 15 && sPartyData.m_sMapID.wMainID != 51 ) continue; // โตเกียว4
		else if ( nIndex == 16 && sPartyData.m_sMapID.wMainID != 52 ) continue; // โตเกียว4
		else if ( nIndex == 18 && sPartyData.m_sMapID.wMainID != 214 ) continue; // ห้องคอม Trading
		else if ( nIndex == 19 && sPartyData.m_sMapID.wMainID != 222 ) continue; // Tyranny
		if ( !sPartyData.m_wMembers ) continue;
		//if ( GLGaeaClient::GetInstance().GetCharacter()->m_wSchool != sPartyData.m_wSchool) continue; // by.l3est Party School Only
		////////////////////////////////
		if ( !GLCONST_CHAR::bPARTY_2OTHERSCHOOL )  // by.l3est Party All School
		{
			if ( GLGaeaClient::GetInstance().GetCharacter()->m_wSchool != sPartyData.m_wSchool) continue;
		}
		////////////////////////////////

		m_vecPartySearchView.push_back( sPartyData );
	}

	std::sort( m_vecPartySearchView.begin(), m_vecPartySearchView.end() );
	
	m_nMaxPage = 1 + ( (int)m_vecPartySearchView.size() / PARTYFINDER_MAX_SLOT ); // dito manggagaling ung count ng total page natin
	ShowParty(m_nCurrentPage);	
}
WORD CPartyFinderWindow::GetSchool()
{
	GLCHARLOGIC& ref_CharData = GLGaeaClient::GetInstance().GetCharacterLogic ();
	return ref_CharData.m_wSchool;	
}