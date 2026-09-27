#include "StdAfx.h"
#include "MobPreviewWindow.h"
#include "MobPreviewWindowRender.h"
#include "MobDBWindowItem.h"
#include "GLItemMan.h"

#include "InnerInterface.h"
#include "BasicLineBox.h"
#include "ItemImage.h"
#include "UITextControl.h"
#include "GameTextControl.h"
#include "../Lib_Engine/DxCommon/d3dfont.h"
#include "BasicTextBox.h"
#include "BasicButton.h"
#include "GLItemMan.h"
#include "GLGaeaClient.h"
#include "DxGlobalStage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// [MobPreviewWindow][Construction] by edwin 20190804 ***** [Edition 1]

CMobPreviewWindow::CMobPreviewWindow () 
	: m_pRender( NULL )
	, m_pButtonLeft( NULL )
	, m_pButtonRight( NULL )
	, m_pMobNameText( NULL )
{
    memset( m_pMobName, 0, sizeof(m_pMobName) );
}

CMobPreviewWindow::~CMobPreviewWindow ()
{
}

void CMobPreviewWindow::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	m_pBackGround = new CUIControl;
	m_pBackGround->CreateSub ( this, "MOBPREVIEW_WINDOW_MAIN_REGION" );	
	m_pBackGround->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pBackGround );

	{
        CUIControl* pControl;

        pControl = new CUIControl();
        pControl->CreateSub ( this, "MOBPREVIEW_WINDOW_MOB_MAIN_TITLE_HEAD" );
        RegisterControl( pControl );
        m_pMobName[TITLE_HEAD] = pControl;

        pControl = new CUIControl();
        pControl->CreateSub ( this, "MOBPREVIEW_WINDOW_MOB_MAIN_TITLE_BODY" );
        RegisterControl( pControl );
        m_pMobName[TITLE_BODY] = pControl;

        pControl = new CUIControl();
        pControl->CreateSub ( this, "MOBPREVIEW_WINDOW_MOB_MAIN_TITLE_TAIL" );
        RegisterControl( pControl );
        m_pMobName[TITLE_TAIL] = pControl;
	}


	m_pMobNameText = new CBasicTextBox;
	m_pMobNameText->CreateSub ( this, "MOBPREVIEW_WINDOW_MOB_NAME_TEXT", TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y  );
	m_pMobNameText->SetFont ( pFont9 );
	m_pMobNameText->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pMobNameText );
	
	{
		CMobDBWindowItem* pMobDBItemList = new CMobDBWindowItem;
		pMobDBItemList->CreateSub ( this, "MOB_PREVIEW_ITEM_LIST", UI_FLAG_DEFAULT, ITEM_LIST );
		pMobDBItemList->CreateSubControl ();
		RegisterControl ( pMobDBItemList );
		m_pMobDBItemList = pMobDBItemList;
	}
}

void CMobPreviewWindow::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CMobPreviewWindow::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	switch( ControlID )
	{
	};

	CUIWindowEx::TranslateUIMessage ( ControlID, dwMsg );
}

void CMobPreviewWindow::SetVisibleSingle ( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );

	if( bVisible )
	{
	}
	else
	{
		ClearRender();
	}
}

void CMobPreviewWindow::ClearRender()
{
	if ( m_pRender )
		m_pRender->ClearRender();
}

void CMobPreviewWindow::StartPreviewMob ( SNATIVEID sMobID )
{
	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pCharacter )		return;

	//////////////////////////////////////////////////////////////////////////////
	//--load & insert drop item list
	m_pMobDBItemList->Reset();
	
	/////////////////////////////////////////////////
	//--version 1, each map code to each crow in every map
	SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData ( sMobID );
	
	if ( pCROW )
	{
		MOB_DROP_SPECID_MAP_ITER iter_MDSID = pCROW->m_mapMobDropSpecID.begin ();
		MOB_DROP_SPECID_MAP_ITER iter_MDSID_end = pCROW->m_mapMobDropSpecID.end ();
		
		for( ; iter_MDSID != iter_MDSID_end; ++iter_MDSID )
		{
			ITEMDBDROP_RANGE sRANGE;
			sRANGE = GLItemMan::GetInstance().m_mapItemDBDropKey.equal_range ( iter_MDSID->second );
			
			ITEMDBDROP_KEYMAP_ITER iter_RANGE = sRANGE.first;
			ITEMDBDROP_KEYMAP_ITER iter_RANGE_end = sRANGE.second;
			DWORD dwID;
			
			for ( ; iter_RANGE != iter_RANGE_end; ++iter_RANGE )
			{
				dwID = iter_RANGE->second.sItemID.dwID;
				SITEM* pItem = GLItemMan::GetInstance().GetItem( iter_RANGE->second.sItemID );
				
				//--insert drop info to local temp map list
				if( pItem )
					m_pMobDBItemList->Insert( dwID, iter_RANGE->second.sItemID );
			}
		}
		
		//--this gen item is not included in .genitem file
		SNATIVEID sGenID =  pCROW->m_sGenerate.m_sGenItemID;
		if ( sGenID != NATIVEID_NULL() )
		{
			SITEM* pItem = GLItemMan::GetInstance().GetItem( sGenID );
			if ( pItem ) m_pMobDBItemList->Insert( sGenID.dwID, sGenID );
		}
		
		m_pMobDBItemList->LoadItemList( true );
		m_pMobDBItemList->SetVisibleSingle ( true );
		
		CString strTmp;
		strTmp.Format( "< %s >", pCROW->GetName() );
		m_pMobNameText->SetOneLineText( strTmp, NS_UITEXTCOLOR::CHARTREUSE );
	}
}