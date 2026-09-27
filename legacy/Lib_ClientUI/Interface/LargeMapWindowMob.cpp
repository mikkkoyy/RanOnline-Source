#include "StdAfx.h"
#include "LargeMapWindow.h"
#include "LargeMapWindowMob.h"
#include "LargeMapWindowCheckBox.h"
#include "LargeMapWindowMobDrop.h"

#include "../Lib_Client/G-Logic/GLContrlServerMsg.h"
#include "../Lib_Client/G-Logic/GLCrowData.h"
#include "../Lib_Client/G-Logic/GLItemMan.h"


#include "../Lib_Client/G-Logic/GLGaeaClient.h"
#include "../Lib_Client/DxGlobalStage.h"

#include "GLCrowData.h"
#include "GLItemMan.h"

#include "InnerInterface.h"
#include "BasicLineBox.h"
#include "BasicLineBoxEx.h"
#include "BasicTextBoxEx.h"
#include "BasicScrollBarEx.h"
#include "UITextControl.h"
#include "GameTextControl.h"
#include "ItemImage.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"

#include "BasicScrollThumbFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int CLargeMapWindowMob::nSTARTLINE = 0;
const int CLargeMapWindowMob::nOUTOFRANGE = -1;

CLargeMapWindowMob::CLargeMapWindowMob ():
	m_nONE_VIEW_SLOT ( 10 ),
	m_nCurPos ( -1 ),
	m_pScrollBar(NULL),
	m_pRewardScrollBar(NULL)
{
	SecureZeroMemory ( m_pMob, sizeof( m_pMob ) );
	SecureZeroMemory ( m_pRewardItems, sizeof( m_pRewardItems ) );
	SecureZeroMemory ( m_pMobArrayDummy, sizeof( m_pMobArrayDummy ) );
    memset( m_pMobMainBG, 0, sizeof(m_pMobMainBG) );
    memset( m_pMobRewardBG, 0, sizeof(m_pMobRewardBG) );
}

CLargeMapWindowMob::~CLargeMapWindowMob ()
{
	m_RewardItemMap.clear();
	m_UniqueRewardItemSet.clear();
}

void CLargeMapWindowMob::CreateSubControl()
{
	
	{
        CUIControl* pControl;

        pControl = new CUIControl();
        pControl->CreateSub ( this, "MAP_WINDOW_MOB_MAIN_TITLE_HEAD" );
        RegisterControl( pControl );
        m_pMobMainBG[TITLE_HEAD] = pControl;

        pControl = new CUIControl();
        pControl->CreateSub ( this, "MAP_WINDOW_MOB_MAIN_TITLE_BODY" );
        RegisterControl( pControl );
        m_pMobMainBG[TITLE_BODY] = pControl;

        pControl = new CUIControl();
        pControl->CreateSub ( this, "MAP_WINDOW_MOB_MAIN_TITLE_TAIL" );
        RegisterControl( pControl );
        m_pMobMainBG[TITLE_TAIL] = pControl;
	}

	{
        CUIControl* pControlMotion;

        pControlMotion = new CUIControl();
        pControlMotion->CreateSub ( this, "MAP_WINDOW_MOB_REWARD_TITLE_HEAD" );
        RegisterControl( pControlMotion );
        m_pMobRewardBG[TITLE_HEAD] = pControlMotion;

        pControlMotion = new CUIControl();
        pControlMotion->CreateSub ( this, "MAP_WINDOW_MOB_REWARD_TITLE_BODY" );
        RegisterControl( pControlMotion );
        m_pMobRewardBG[TITLE_BODY] = pControlMotion;

        pControlMotion = new CUIControl();
        pControlMotion->CreateSub ( this, "MAP_WINDOW_MOB_REWARD_TITLE_TAIL" );
        RegisterControl( pControlMotion );
        m_pMobRewardBG[TITLE_TAIL] = pControlMotion;
	}

	m_pMobMain = CreateMobSlot( "MAP_WINDOW_INFO_TAP_MONSTER0_CHECKBOX", CROW_MAIN );
	m_pMobMain->SetTextTitle( ID2GAMEWORD("MAP_MONSTER_INFO",0), NS_UITEXTCOLOR::WHITE );

	CString strSlotKeyword[nMAX_ONE_VIEW_SLOT] =
	{
		"MAP_WINDOW_INFO_TAP_MONSTER1_CHECKBOX",
		"MAP_WINDOW_INFO_TAP_MONSTER2_CHECKBOX",
		"MAP_WINDOW_INFO_TAP_MONSTER3_CHECKBOX",
		"MAP_WINDOW_INFO_TAP_MONSTER4_CHECKBOX",
		"MAP_WINDOW_INFO_TAP_MONSTER5_CHECKBOX",
		"MAP_WINDOW_INFO_TAP_MONSTER6_CHECKBOX",
		"MAP_WINDOW_INFO_TAP_MONSTER7_CHECKBOX",
		"MAP_WINDOW_INFO_TAP_MONSTER8_CHECKBOX",
		"MAP_WINDOW_INFO_TAP_MONSTER9_CHECKBOX",
		"MAP_WINDOW_INFO_TAP_MONSTER10_CHECKBOX"
	};

	for ( int i = 0; i < nMAX_ONE_VIEW_SLOT; i++ )	m_pMobArrayDummy[i] = CreateControl ( strSlotKeyword[i].GetString () );
	for ( int i = 0; i < MAXCROW; i++ )				m_pMob[i] = CreateMobSlot ( strSlotKeyword[0], CROW_SLOT0 + i );

	{
		CUIControl* pMonsterScrollBarSpace = new CUIControl;
		pMonsterScrollBarSpace->CreateSub(this, "MAP_WINDOW_INFO_TAP_MONSTER_LIST_SCROLL_SPACE" , UI_FLAG_RIGHT | UI_FLAG_YSIZE, MONSTER_LIST_SCROLL_BAR_SPACE );
		RegisterControl ( pMonsterScrollBarSpace );
		m_pMonsterScrollBarSpace = pMonsterScrollBarSpace;

		CBasicScrollBarEx* pScrollBar = new CBasicScrollBarEx;
		pScrollBar->CreateSub( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, MONSTER_LIST_SCROLL_BAR );
		pScrollBar->CreateBaseScrollBar( "MAP_WINDOW_INFO_TAP_MONSTER_LIST_SCROLL" );
		RegisterControl ( pScrollBar );
		m_pScrollBar = pScrollBar;
		m_pScrollBar->GetThumbFrame ()->SetScrollParent(pMonsterScrollBarSpace);
	}

	for (int v=0; v < REWARD_V_LINE_COUNT; ++v)
	{
		for (int h=0; h < REWARD_H_LINE_COUNT; ++h)
		{
			int index = v*REWARD_H_LINE_COUNT + h;  
			CString strFormat;
			strFormat.Format("MAP_WINDOW_INFO_MONSTER_LINEBOX%d", index + 1);
			//m_pRewardItems[v][h] = CreateItemImage(strFormat.GetString(),REWARD_ITEM1 + index );
			m_pBasicLineBox = new CBasicLineBox;
			m_pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
			m_pBasicLineBox->CreateBaseBoxQuestList( (char*)strFormat.GetString() );
			RegisterControl ( m_pBasicLineBox );
		}
	}


	for (int v=0; v < REWARD_V_LINE_COUNT; ++v)
	{
		for (int h=0; h < REWARD_H_LINE_COUNT; ++h)
		{
			int index = v*REWARD_H_LINE_COUNT + h;  
			CString strFormat;
			strFormat.Format("MAP_WINDOW_INFO_TAP_MONSTER_REWARD%d", index + 1);
			m_pRewardItems[v][h] = CreateItemImage(strFormat.GetString(),REWARD_ITEM1 + index );
		}
	}
	CUIControl* pRewardScrollBarSpace = new CUIControl;
	pRewardScrollBarSpace->CreateSub(this, "MAP_WINDOW_INFO_TAP_MONSTER_REWARD_SCROLL_SPACE" , UI_FLAG_RIGHT | UI_FLAG_YSIZE, REWARD_SCROLL_BAR_SPACE );
	RegisterControl ( pRewardScrollBarSpace );
	m_pRewardScrollBarSpace = pRewardScrollBarSpace;

	CBasicScrollBarEx* pRewardScrollBar = new CBasicScrollBarEx;
	pRewardScrollBar->CreateSub( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, REWARD_SCROLL_BAR );
	pRewardScrollBar->CreateBaseScrollBar( "MAP_WINDOW_INFO_TAP_MONSTER_REWARD_SCROLL" );
	RegisterControl ( pRewardScrollBar );
	m_pRewardScrollBar = pRewardScrollBar;
	m_pRewardScrollBar->GetThumbFrame ()->SetScrollParent(m_pRewardScrollBarSpace);
}

void CLargeMapWindowMob::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;
	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CLargeMapWindowMob::TranslateUIMessage ( UIGUID cID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( cID, dwMsg );

	if ( CROW_SLOT0 <= cID && cID < CROW_SLOT_END )
	{
		if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
		{
			const int nIndex = cID - CROW_SLOT0;
			m_pMob[nIndex]->SetCheck();
			CInnerInterface::GetInstance().GetLargeMapWindow()->m_bMOB[nIndex] = m_pMob[nIndex]->IsChecked();

			SetMonsterInfo();
		}
	}

	switch(cID)
	{
	case CROW_MAIN:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg))
			{
				m_pMobMain->SetCheck();
				for(int i = 0; i < m_nIndex; i++ )
				{
					if ( m_pMobMain->IsChecked()) m_pMob[i]->ForceCheck();
					else m_pMob[i]->ForceUnCheck();

					CInnerInterface::GetInstance().GetLargeMapWindow()->m_bMOB[i] = m_pMobMain->IsChecked();
				}

				SetMonsterInfo();
			}
		}
		break;
	case MONSTER_LIST_SCROLL_BAR:
	case MONSTER_LIST_SCROLL_BAR_SPACE:
		{
			if( CHECK_MOUSE_IN(dwMsg) )
			{
				UpdateMonsterList();
			}
		}
		break;
	case REWARD_SCROLL_BAR:
	case REWARD_SCROLL_BAR_SPACE:
		{
			if( CHECK_MOUSE_IN(dwMsg) )
			{
				UpdateMonsterInfo();
			}
		}
		break;
	}

	for (int v=0; v < REWARD_V_LINE_COUNT; ++v)
	{
		for (int h=0; h < REWARD_H_LINE_COUNT; ++h)
		{
			if( m_pRewardItems[v][h]->GetWndID() == cID )
			{
				if( CHECK_MOUSE_IN(dwMsg) )
				{
					SITEM* pItem = GLItemMan::GetInstance().GetItem( m_pRewardItems[v][h]->GetItemID() );
					if ( pItem )
					{
						SITEMCUSTOM sItemCustom;
						sItemCustom.sNativeID = pItem->sBasicOp.sNativeID;
						sItemCustom.cDAMAGE = pItem->sBasicOp.wGradeAttack;
						sItemCustom.cDEFENSE = pItem->sBasicOp.wGradeDefense;
						sItemCustom.wTurnNum = pItem->sDrugOp.wPileNum;
						//CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW( sItemCustom, FALSE, FALSE, FALSE, 0, 0 );
						CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW( sItemCustom, FALSE, FALSE, FALSE, FALSE, FALSE, 0, 0 );
					}
				}
			}
		}
	}


}

void CLargeMapWindowMob::LoadMob ( PCROWDATA pCrow )
{
	if ( !pCrow ) return;

	CString strName;
	strName.Format("%s", pCrow->GetName());
	m_pMob[m_nIndex]->SetText(strName, NS_UITEXTCOLOR::WHITE, pCrow->sNativeID );
	m_pMob[m_nIndex]->SetVisibleSingle(TRUE);
	m_sNativeID[m_nIndex] = pCrow->sNativeID;
	m_nIndex++;

	ResetAllCrowRender (m_nIndex);
	SetCrowRender( 0, 10 );

	m_pScrollBar->GetThumbFrame()->SetState( m_nIndex, 10 );
}

void CLargeMapWindowMob::SetMonsterInfo()
{
	m_RewardItemMap.clear();
	m_UniqueRewardItemSet.clear();

	for (int v=0; v < REWARD_V_LINE_COUNT; ++v)
	{
		for (int h=0; h < REWARD_H_LINE_COUNT; ++h)
		{
			m_pRewardItems[v][h]->ResetItem();
			m_pRewardItems[v][h]->SetVisibleSingle(FALSE);
		}
	}

	for( int i = 0; i < MAXCROW; i++ ) 
	{
		if( !m_pMob[i]->IsChecked() ) continue;

		SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData ( m_sNativeID[i] );
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
				
				for ( ; iter_RANGE != iter_RANGE_end; ++iter_RANGE )
				{
					SITEM* pItem = GLItemMan::GetInstance().GetItem( iter_RANGE->second.sItemID );
					if( !pItem )						continue;

					REWARDITEM_MAP_ITER it = m_RewardItemMap.find( iter_RANGE->second.sItemID.dwID );
					if( it != m_RewardItemMap.end() )	continue;
					
					m_RewardItemMap.insert( std::make_pair( pItem->sBasicOp.sNativeID.dwID, pItem->sBasicOp.sNativeID ) );
				}
			}			
		}
	}

	for( REWARDITEM_MAP_ITER iter = m_RewardItemMap.begin();
		iter != m_RewardItemMap.end(); ++iter )
	{
		const SNATIVEID& sID = (*iter).second;
		m_UniqueRewardItemSet.insert( sID.dwID );
	}	

	UpdateMonsterInfo();
}

void CLargeMapWindowMob::UpdateMonsterList()
{
	if ( m_pScrollBar )
	{
		CBasicScrollThumbFrame* const pThumbFrame = m_pScrollBar->GetThumbFrame ();
		int nTotal = pThumbFrame->GetTotal ();

		if ( nTotal < nMAX_ONE_VIEW_SLOT ) return ;
	
		const int nViewPerPage = pThumbFrame->GetViewPerPage ();
		if ( nViewPerPage <= nTotal )
		{
			int nCurPos = nSTARTLINE;
			const int nMovableLine = nTotal - nViewPerPage;
			float fPercent = pThumbFrame->GetPercent ();

			nCurPos = (int)floor(fPercent * nMovableLine);
			if ( nCurPos < nSTARTLINE ) nCurPos = nSTARTLINE;

			if ( m_nCurPos == nCurPos ) return;

			m_nCurPos = nCurPos;

			ResetAllCrowRender ( nTotal );
			SetCrowRender ( nCurPos, nCurPos + nViewPerPage );
		}
	}

	if ( m_nIndex == 0 ) m_pScrollBar->SetVisibleSingle(FALSE);
	else m_pScrollBar->SetVisibleSingle(TRUE);
}

void CLargeMapWindowMob::UpdateMonsterInfo()
{
	int lineCount = (int)m_UniqueRewardItemSet.size()/REWARD_H_LINE_COUNT;
	
	if( m_UniqueRewardItemSet.size() % REWARD_H_LINE_COUNT != 0 )
		lineCount++;

	m_pRewardScrollBar->GetThumbFrame()->SetState( lineCount, REWARD_V_LINE_COUNT );

	int rewardPos = m_pRewardScrollBar->GetThumbFrame()->GetPosition();
	for (int v=0; v < REWARD_V_LINE_COUNT; ++v)
	{
		for (int h=0; h < REWARD_H_LINE_COUNT; ++h)
		{
			size_t index = (v+rewardPos)*REWARD_H_LINE_COUNT + h;  
			if( index >= m_UniqueRewardItemSet.size())
			{
				m_pRewardItems[v][h]->SetVisibleSingle(FALSE);
				continue;
			}
			
			std::set<DWORD>::iterator it = m_UniqueRewardItemSet.begin();
			std::advance(it,index); 

			SITEM* pItem = GLItemMan::GetInstance().GetItem( SNATIVEID(*it) );
			if( !pItem )	continue;

			m_pRewardItems[v][h]->SetItem( SNATIVEID(*it) );		
			m_pRewardItems[v][h]->SetVisibleSingle(TRUE);
		}
	}
}

void CLargeMapWindowMob::ClearMobList()
{
	m_nIndex = 0;
	for( int i = 0; i < MAXCROW; i++ ) 
	{
		m_pMob[i]->ClearText();
		m_pMob[i]->SetVisibleSingle(FALSE);
		m_sNativeID[i] = NATIVEID_NULL();
	}
}

CUIControl*	CLargeMapWindowMob::CreateControl ( const char* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl );
	RegisterControl ( pControl );

	return pControl;
}

CLargeMapWindowCheckBox* CLargeMapWindowMob::CreateMobSlot ( CString strKeyword, UIGUID ControlID )
{
	CLargeMapWindowCheckBox* pCrowSlot = new CLargeMapWindowCheckBox;
	pCrowSlot->CreateSub ( this, strKeyword.GetString (), UI_FLAG_DEFAULT, ControlID );
	pCrowSlot->CreateSubControl();
	pCrowSlot->SetVisibleSingle(TRUE);
	RegisterControl ( pCrowSlot );

	return pCrowSlot;
}

void CLargeMapWindowMob::SetCrowRender ( int nStartIndex, int nTotal )
{
	if ( nTotal < 0 ) return ;

	const UIRECT& rcParentPos = GetGlobalPos ();

	for ( int i = nStartIndex; i < nTotal; i++ )
	{
		CLargeMapWindowCheckBox* pMob = m_pMob[i];

		if ( pMob )
		{
			int nAbsoluteIndex = i - nStartIndex;

			CUIControl* pDummyControl = m_pMobArrayDummy[nAbsoluteIndex];			
			const UIRECT& rcSlotPos = pDummyControl->GetGlobalPos ();
			const UIRECT& rcSlotLocalPos = pDummyControl->GetLocalPos ();

			pMob->SetLocalPos ( rcSlotLocalPos );
			pMob->SetGlobalPos ( rcSlotPos );

			pMob->SetVisibleSingle ( TRUE );
		}
	}
}

void CLargeMapWindowMob::ResetAllCrowRender ( int nTotal )
{
	if ( nTotal < 0 ) return ;

	for ( int i = 0; i < nTotal; i++ )
	{
		CLargeMapWindowCheckBox* pMob = m_pMob[i];
		if ( pMob ) pMob->SetVisibleSingle ( FALSE );
	}
}

CItemImage*	CLargeMapWindowMob::CreateItemImage ( const char* szControl, UIGUID ControlID )
{
	CItemImage* pItemImage = new CItemImage;
	pItemImage->CreateSub ( this, szControl, UI_FLAG_DEFAULT, ControlID );
	pItemImage->CreateSubControl ();
	RegisterControl ( pItemImage );

	return pItemImage;
}