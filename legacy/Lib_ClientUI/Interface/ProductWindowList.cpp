#include "StdAfx.h"
#include "ProductWindowList.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "UITextControl.h"
#include "GameTextControl.h"

#include "BasicTextBoxEx.h"
#include "BasicScrollBarEx.h"
#include "BasicScrollThumbFrame.h"

#include "BasicLineBoxSmart.h"

#include "GLogicData.h"

#include "GLItemMixMan.h"
#include "GLItemMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	//현재 사이즈에 최적화된 수치로 윈도우 사이즈가 변경된다면 조절해야만한다.
    const float fTEXT_PART_INTERVAL = 4.5f;       // 줄간격
    const float fSELBOX_PART_INTERVAL = 2.5f;     // 선택박스 간격 조절
}

CProductWindowList::CProductWindowList ()
	: m_pTextBox(NULL)
    , m_pScrollBar(NULL)
    , m_pSeleted(NULL)
    , m_nSelectedPos(-1)
	, m_nSelectIndex(-1)
	, m_dwSelectedData(0)
	, m_strSelectedText("")
{
}

CProductWindowList::~CProductWindowList ()
{
}

void CProductWindowList::CreateSubControl ()
{
    CBasicLineBoxSmart::CreateSubControl( "PRODUCT_WINDOW_BASE_SMARTLINEBOX_DARKGRAY" );
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

    { // 선택영역
        CBasicLineBoxSmart* pLinBoxSmartTypeTitle = new CBasicLineBoxSmart(  );
        pLinBoxSmartTypeTitle->CreateSub( this, "PRODUCT_WINDOW_LIST_SELECTED", UI_FLAG_XSIZE | UI_FLAG_YSIZE);
        pLinBoxSmartTypeTitle->CreateSubControl( "PRODUCT_WINDOW_BASE_SMARTLINEBOX_YELLOWLINE" );
        RegisterControl( pLinBoxSmartTypeTitle );
        m_pSeleted = pLinBoxSmartTypeTitle;
        m_rcSeleted = m_pSeleted->GetLocalPos();
    }
	

    { // 텍스트 박스
        CBasicTextBoxEx* pTextBox = new CBasicTextBoxEx();
        pTextBox->CreateSub( this, "PRODUCT_WINDOW_LIST_TEXTBOX", UI_FLAG_DEFAULT, ID_PRODUCT_TYPE_LIST_TEXTLIST );
        pTextBox->SetFont( pFont9 );		
        pTextBox->SetSensitive( true );
        //pTextBox->SetLineInterval( 4.5f );
        pTextBox->SetPartInterval( fTEXT_PART_INTERVAL );
        RegisterControl( pTextBox );
        m_pTextBox = pTextBox;
    }
	{ // 스크롤바
        CBasicScrollBarEx* pScrollBar = new CBasicScrollBarEx();
        pScrollBar->CreateSub( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, ID_PRODUCT_TYPE_LIST_SCROOL );
        pScrollBar->CreateBaseScrollBar( "PRODUCT_WINDOW_LIST_SCROOL" );
        pScrollBar->GetThumbFrame()->SetScrollParent( m_pTextBox );
        pScrollBar->GetThumbFrame()->SetScrollMoveLine( 1 );
        RegisterControl ( pScrollBar );
        m_pScrollBar = pScrollBar;
    }
}

void CProductWindowList::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	m_pTextBox->SetUseOverColor ( FALSE );
	if ( m_pTextBox->GetCount () && 0 <= m_nSelectIndex )
		m_pTextBox->SetUseTextColor ( m_nSelectIndex, FALSE );

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	CBasicScrollThumbFrame* const pThumbFrame = m_pScrollBar->GetThumbFrame();
    const int nTotal       = m_pTextBox->GetTotalLine();
    const int nVisibleLine = m_pTextBox->GetVisibleLine();
    m_pScrollBar->GetThumbFrame()->SetState( nTotal, nVisibleLine );

	int nCurPos = 0;
    const int nMovableLine = nTotal - nVisibleLine;
    float fPercent = pThumbFrame->GetPercent();
    nCurPos = (int)floor(fPercent * nMovableLine);
    if ( nCurPos < 0 ) nCurPos = 0;
    m_pTextBox->SetCurLine(nCurPos);

    int nScrollRelativePos = m_nSelectedPos-nCurPos;
    if( 0 <= nScrollRelativePos && nScrollRelativePos <= nVisibleLine)
    {
        m_pSeleted->SetVisibleSingle( TRUE );

		UIRECT rcPart;
		if( !m_pTextBox->GetPartInfo(0, rcPart) )
			return;

        float fHeight = rcPart.sizeY + m_pTextBox->GetPartInterval();
		float fSelectSizeY = fHeight + fSELBOX_PART_INTERVAL;

        D3DXVECTOR2 vLocalPos, vGlobalPos;
        vLocalPos.x = m_rcSeleted.left;
        vLocalPos.y = m_rcSeleted.top + fHeight * (nScrollRelativePos);
        vGlobalPos = vLocalPos;

        vGlobalPos.x += this->GetGlobalPos().left;
        vGlobalPos.y += this->GetGlobalPos().top;

		m_pSeleted->SetControlSize( (int)m_rcSeleted.sizeX,  (int)fSelectSizeY );
        m_pSeleted->SetLocalPos( vLocalPos );
        m_pSeleted->SetGlobalPos( vGlobalPos );
    }
    else
    {
        m_pSeleted->SetVisibleSingle( FALSE );
    }
}


void CProductWindowList::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch( ControlID )
	{
	case ID_PRODUCT_TYPE_LIST_TEXTLIST:
        if( CHECK_MOUSE_IN( dwMsg ) )
        {
            //int i = 0;
			int nIndex = m_pTextBox->GetSelectPos ();
			if ( m_pTextBox->GetCount () <= nIndex || nIndex < 0  ) return ;
			if ( UIMSG_LB_UP & dwMsg )
			{
                m_pSeleted->SetVisibleSingle( TRUE );
				
				m_nSelectedPos = m_pTextBox->GetSelectPos();

				m_dwSelectedData = m_pTextBox->GetTextData ( nIndex );
				m_strSelectedText = m_pTextBox->GetText( nIndex );
				m_nSelectIndex = nIndex;

				AddMessageEx ( UIMSG_PRODUCT_WINDOW_LIST_SELECT );
            }

			m_pTextBox->SetUseOverColor ( TRUE );
			m_pTextBox->SetOverColor ( nIndex, NS_UITEXTCOLOR::ORANGE );

        }
        break;
	}
}


void CProductWindowList::LoadData( WORD wCategory, WORD wFilter )
{
	ClearData();

	ITEMMIX_KEYMAP &mapkeys = GLItemMixMan::GetInstance().m_mapKeyProduct;

	ITEMMIX_VEC vecitemmix;
	vecitemmix.reserve( mapkeys.size() );

	for( ITEMMIX_KEYMAP_ITER iter = mapkeys.begin();
		iter != mapkeys.end(); ++iter )
	{
		const ITEM_MIX& item_mix_data = (*iter).second;

		if ( wCategory > 0 )
		{
			WORD wcategory_check = wCategory - 1;

			if ( item_mix_data.wCategory != wcategory_check )
				continue;
		}

		vecitemmix.push_back( item_mix_data );
	}

	std::sort( vecitemmix.begin(), vecitemmix.end() );

	for ( int i=0; i<(int)vecitemmix.size(); ++i )
	{
		const ITEM_MIX& item_mix_data = vecitemmix[i];
		
		SITEM* pitem_data = GLItemMan::GetInstance().GetItem( item_mix_data.sResultItem.sNID );
		if ( !pitem_data )	continue;

		int nIndex = m_pTextBox->AddOneLineText ( pitem_data->GetName(),FALSE, ITEM_INFOR_COLOR::dwItemRank[pitem_data->sBasicOp.emLevel] );
		m_pTextBox->SetTextImage( nIndex, "RAN_PRODUCT_WINDOW_TYPE_STATUS_NOTHING" );
		m_pTextBox->SetTextData ( nIndex, item_mix_data.dwKey );
	}

	//select first line 
	if ( m_pTextBox->GetCount () > 0 )
	{
		m_pTextBox->SetSelectPos( 0 );
		int nIndex = m_pTextBox->GetSelectPos ();
		if ( m_pTextBox->GetCount () <= nIndex || nIndex < 0  ) return;

		m_pSeleted->SetVisibleSingle( TRUE );
				
		m_nSelectedPos = m_pTextBox->GetSelectPos();

		m_dwSelectedData = m_pTextBox->GetTextData ( nIndex );
		m_strSelectedText = m_pTextBox->GetText( nIndex );
		m_nSelectIndex = nIndex;
		
		m_pTextBox->SetUseOverColor ( TRUE );
		m_pTextBox->SetOverColor ( nIndex, NS_UITEXTCOLOR::ORANGE );
	}
}

void CProductWindowList::ClearData()
{
	m_dwSelectedData = UINT_MAX;
	m_strSelectedText = "";
	m_nSelectIndex  = -1;
	m_nSelectedPos = -1;
	m_pTextBox->ClearText();
}