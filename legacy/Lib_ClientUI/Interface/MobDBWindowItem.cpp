#include "StdAfx.h"
#include "MobDBWindowItem.h"
#include "GLItemMan.h"
#include "GLGaeaClient.h"

#include "BasicAnimationBox.h"
#include "BasicButton.h"
#include "../Lib_Engine/GUInterface/BasicButtonText.h"
#include "BasicScrollThumbFrame.h"
#include "BasicTextBox.h"
#include "d3dfont.h"

#include "InnerInterface.h"
#include "BasicLineBox.h"
#include "InnerInterfaceGuid.h"
#include "BasicTextBoxEx.h"
#include "BasicScrollBarEx.h"
#include "GameTextControl.h"
#include "BasicTextButton.h"
#include "UITextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//--------------------------------------------------------------------
// CItemGarbage
//--------------------------------------------------------------------
CMobDBWindowItem::CMobDBWindowItem()	// ???
	: m_pListText( NULL )
	, m_pListScrollBar( NULL )
	, m_nSelectIndex( 0 )
	, m_bRefresh ( false )
{
}

CBasicTextBox* CMobDBWindowItem::CreateStaticControl ( char* szControlKeyword, CD3DFontPar* pFont, int nAlign )
{
	CBasicTextBox* pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, szControlKeyword );
	pStaticText->SetFont ( pFont );
	pStaticText->SetTextAlign ( nAlign );	
	RegisterControl ( pStaticText );

	return pStaticText;
}

CMobDBWindowItem::~CMobDBWindowItem()
{
}

CBasicTextBox* CMobDBWindowItem::CreateTextBox ( const char* szTextBox )
{
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, D3DFONT_SHADOW | D3DFONT_ASCII );

	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, szTextBox );
    pTextBox->SetFont ( pFont8 );	
	RegisterControl ( pTextBox );
	return pTextBox;
}

VOID CMobDBWindowItem::CreateSubControl()
{
	const int nAlignCenterBoth = TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y;
	CD3DFontPar* pFont12 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 12,	D3DFONT_SHADOW | D3DFONT_ASCII );
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, D3DFONT_SHADOW | D3DFONT_ASCII );

	const int nAlignLeftCenter = TEXT_ALIGN_CENTER_Y | TEXT_ALIGN_LEFT;
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, D3DFONT_SHADOW | D3DFONT_ASCII );

	{
		CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 10, D3DFONT_SHADOW | D3DFONT_ASCII );

		CBasicTextBoxEx* pTextBox = new CBasicTextBoxEx;
		pTextBox->CreateSub ( this, "MOB_DB_ITEMLIST_TEXTBOX", UI_FLAG_DEFAULT, ITEM_LIST_TEXTBOX );
		pTextBox->SetFont ( pFont8 );		
		pTextBox->SetLineInterval ( 3.0f );
		pTextBox->SetSensitive ( true );
		pTextBox->SetLimitLine ( 22000000 );
		RegisterControl ( pTextBox );
		m_pListText = pTextBox;
		
		int nTotalLine = pTextBox->GetVisibleLine ();

		CBasicScrollBarEx* pScrollBar = new CBasicScrollBarEx;
		pScrollBar->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, ITEM_LIST_SCROLLBAR );
		pScrollBar->CreateBaseScrollBar ( "MOB_DB_ITEMLIST_SCROLLBAR" );
		pScrollBar->GetThumbFrame()->SetState ( 1, nTotalLine );
		RegisterControl ( pScrollBar );
		m_pListScrollBar = pScrollBar;
	}
}

void CMobDBWindowItem::SetVisibleSingle( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle( bVisible );

	Reset();
}

VOID CMobDBWindowItem::Update( INT x, INT y, BYTE LB, BYTE MB, BYTE RB, INT nScroll, FLOAT fElapsedTime, BOOL bFirstControl )
{
		m_pListText->SetUseOverColor ( FALSE );
	if ( 0 <= m_nSelectIndex && m_nSelectIndex < m_pListText->GetCount () )
		m_pListText->SetUseTextColor ( m_nSelectIndex, FALSE );


	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( 0 <= m_nSelectIndex && m_nSelectIndex < m_pListText->GetCount () )
	{
		m_pListText->SetUseTextColor ( m_nSelectIndex, TRUE );
		m_pListText->SetTextColor ( m_nSelectIndex, NS_UITEXTCOLOR::ORANGE );
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////
	
	{
		CBasicScrollThumbFrame* const pThumbFrame = m_pListScrollBar->GetThumbFrame ();

		const int nTotalLine = m_pListText->GetTotalLine ();
		const int nLinePerOneView = m_pListText->GetVisibleLine ();
		CDebugSet::ToView ( 1, 10, "InitFucker %d", nLinePerOneView );
		pThumbFrame->SetState ( nTotalLine, nLinePerOneView );
		if ( nLinePerOneView < nTotalLine )
		{
			const int nMovableLine = nTotalLine - nLinePerOneView;
			float fPercent = pThumbFrame->GetPercent ();
			int nPos = (int)floor(fPercent * nMovableLine);
			m_pListText->SetCurLine ( nPos );	
			
			m_pListScrollBar->SetVisibleSingle(true);
		}
		else
		{
			m_pListScrollBar->SetVisibleSingle(false);
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	m_bRefresh = true;
}

VOID CMobDBWindowItem::TranslateUIMessage( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage( ControlID, dwMsg );
	
	switch( ControlID )
	{
	case ITEM_LIST_TEXTBOX:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				const int nIndex = m_pListText->GetSelectPos ();
				if ( nIndex < 0 || m_pListText->GetCount () <= nIndex ) return ;

				m_pListText->SetUseOverColor ( TRUE );
				m_pListText->SetOverColor ( nIndex, NS_UITEXTCOLOR::ORANGE );
				
				SNATIVEID sNativeID = m_pListText->GetTextData ( nIndex );
				if ( sNativeID!= NATIVEID_NULL () )
				{
					SITEMCUSTOM sItemCustom(sNativeID);
					CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW ( sItemCustom, FALSE, FALSE, FALSE,FALSE,FALSE, USHRT_MAX, USHRT_MAX, NATIVEID_NULL(), FALSE, 0 );
				}

				if( UIMSG_LB_UP & dwMsg )
				{
					m_nSelectIndex = nIndex;
					m_bRefresh = true;
				}
			}
		}
		break;
	}
}

void CMobDBWindowItem::LoadItemList( bool bRefresh )
{
	m_pListText->ClearText ();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//-- original 
	MOB_DROPITEM_MAP_ITER iter 			= m_mapMobDropItemList_tmp.begin ();
	MOB_DROPITEM_MAP_ITER iter_end 		= m_mapMobDropItemList_tmp.end ();
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/////////////////////////////////
	std::vector< SNATIVEID >	vecSort;
	for ( ; iter != iter_end; ++iter )
	{
		vecSort.push_back( (*iter).second );
	}
	
	std::sort( vecSort.begin(), vecSort.end(), MOB_DROPITEM_MAP_CMP() );
	std::vector< SNATIVEID >::iterator pos = vecSort.begin();
	/////////////////////////////////
	
	for ( ; pos != vecSort.end(); pos++ )
	{
		SITEM* pItem = GLItemMan::GetInstance().GetItem ( (SNATIVEID)*pos );	

		CString strTemp;
		D3DCOLOR dwColor;
		
		strTemp.Format( "%s", pItem->GetName() );
		dwColor = NS_UITEXTCOLOR::WHITE;

		int nIndex = m_pListText->AddText ( strTemp, dwColor );	

		m_pListText->SetTextData ( nIndex, pos->dwID );

		m_pListText->SetCurLine ( 0 );

		if( bRefresh )
		{
			const int nTotal = m_pListText->GetCount ();
			const int nViewPerPage = m_pListText->GetVisibleLine ();	
			m_pListScrollBar->GetThumbFrame()->SetState ( nTotal, nViewPerPage );
			m_pListScrollBar->GetThumbFrame()->SetPercent ( 0.0f );
		}
	}
	
	vecSort.clear();
}

void CMobDBWindowItem::Reset()
{
	m_mapMobDropItemList_tmp.clear();
}

void CMobDBWindowItem::Insert( DWORD dwID, SNATIVEID sID )
{
	if( m_mapMobDropItemList_tmp.count( dwID ) == 0 )
	{
		m_mapMobDropItemList_tmp.insert( std::make_pair( dwID, sID ));
	}
}