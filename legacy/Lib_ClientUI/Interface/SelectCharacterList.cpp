#include "StdAfx.h"
#include "SelectCharacterList.h"
#include "SelectCharacterListItem.h"
#include "BasicLineBox.h"
#include "BasicScrollBarEx.h"
#include "BasicScrollThumbFrame.h"

#include "DxLobyStage.h"
#include "DxGlobalStage.h"
#include "OuterInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSelectCharacterList::CSelectCharacterList ()
	: m_pScrollBar( NULL )
{
	for( int i=0; i<SELECT_CHARACTER_LIST_MAX; ++i )
		m_pListItem[i] = NULL;

	for( int i=0; i<SELECT_CHARACTER_LIST_MAX; ++i )
		m_pListItem[i] = NULL;
}

CSelectCharacterList::~CSelectCharacterList ()
{
}

void CSelectCharacterList::CreateSubControl ()
{
	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "SELECT_CHAR_WINDOW_CHAR_LIST_BACK" );
	RegisterControl ( pBasicLineBox );	

	/*std::string strLine[7] = 
	{
		"SELECT_CHAR_WINDOW_LINE_0",
		"SELECT_CHAR_WINDOW_LINE_1",
		"SELECT_CHAR_WINDOW_LINE_2",
		"SELECT_CHAR_WINDOW_LINE_3",
		"SELECT_CHAR_WINDOW_LINE_4",
		"SELECT_CHAR_WINDOW_LINE_5",
		"SELECT_CHAR_WINDOW_LINE_6",
	};

	for( int i=0; i<7; ++i )
	{
		CUIControl* pControl = new CUIControl;
		pControl->CreateSub ( this, strLine[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		RegisterControl ( pControl );
	}*/

	std::string strList[SELECT_CHARACTER_LIST_MAX] = 
	{
		"SELECT_CHAR_WINDOW_CHAR_LIST_ITEM_0",
		"SELECT_CHAR_WINDOW_CHAR_LIST_ITEM_1",
		"SELECT_CHAR_WINDOW_CHAR_LIST_ITEM_2",
		"SELECT_CHAR_WINDOW_CHAR_LIST_ITEM_3",
		//"SELECT_CHAR_WINDOW_CHAR_LIST_ITEM_4",
		//"SELECT_CHAR_WINDOW_CHAR_LIST_ITEM_5",
	};

	for( int i=0; i<SELECT_CHARACTER_LIST_MAX; ++i )
	{
		m_pListItem[i] = new CSelectCharacterListItem;
		m_pListItem[i]->CreateSub( this, strList[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE, SELECT_CHARACTER_LIST_0 +i );
		m_pListItem[i]->CreateSubControl ();
		RegisterControl ( m_pListItem[i] );
	}

	m_pScrollBar = new CBasicScrollBarEx;
	m_pScrollBar->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE );
	m_pScrollBar->CreateBaseScrollBar ( "SELECT_CHAR_WINDOW_SCROLLBAR" );
	m_pScrollBar->GetThumbFrame()->SetState ( MAX_SERVERCHAR, SELECT_CHARACTER_LIST_MAX );
	RegisterControl ( m_pScrollBar );
}

void CSelectCharacterList::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case SELECT_CHARACTER_LIST_0:
	case SELECT_CHARACTER_LIST_1:
	case SELECT_CHARACTER_LIST_2:
	case SELECT_CHARACTER_LIST_3:
	//case SELECT_CHARACTER_LIST_4:
	//case SELECT_CHARACTER_LIST_5:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				int nSelected = ControlID - SELECT_CHARACTER_LIST_0;
				if ( nSelected >= SELECT_CHARACTER_LIST_MAX ) return;

				if ( m_pListItem[nSelected]->m_dwCharID != GAEAID_NULL )
				{
					m_pListItem[nSelected]->SetMouseIn( TRUE );

					if ( UIMSG_LB_UP & dwMsg )
					{
						DxGlobalStage::GetInstance().GetLobyStage()->SelectChar( m_pListItem[nSelected]->m_dwCharID );
					}

					/*if ( UIMSG_LB_DUP & dwMsg )
					{
						BOOL bGameJoin = DxGlobalStage::GetInstance().GetLobyStage()->m_bGameJoin;
						if ( !bGameJoin ){
							GLMSG::SNETLOBBY_REQ_GAME_JOIN NetMsgReq;
							COuterInterface::GetInstance().MsgProcess ( &NetMsgReq );
						}
					}*/
				}
			}
		}break;
	};
}

void CSelectCharacterList::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	int nSNum = DxGlobalStage::GetInstance().GetLobyStage()->GetChaSNum ();
	int nTotal = m_pScrollBar->GetThumbFrame()->GetTotal ();
	float fPercent = m_pScrollBar->GetThumbFrame()->GetPercent ();

	if ( nSNum != nTotal )
	{
		m_pScrollBar->GetThumbFrame()->SetState ( nSNum, SELECT_CHARACTER_LIST_MAX );
		nTotal = m_pScrollBar->GetThumbFrame()->GetTotal ();
		fPercent = m_pScrollBar->GetThumbFrame()->GetPercent ();
	}

	BOOL bEnableScrollBar = ( nSNum >= SELECT_CHARACTER_LIST_MAX );
	m_pScrollBar->SetVisibleSingle( bEnableScrollBar );

	int nIncrease = 0;
	if ( SELECT_CHARACTER_LIST_MAX < nTotal )
	{
		nIncrease = (int)floor(fPercent * nTotal);
		if ( nIncrease + SELECT_CHARACTER_LIST_MAX >= nTotal )
		{
			nIncrease = nTotal - SELECT_CHARACTER_LIST_MAX;
		}
	}

	for( int i=0; i<SELECT_CHARACTER_LIST_MAX; ++i )
		m_pListItem[i]->SetIndex( i + nIncrease );
}

void CSelectCharacterList::SetVisibleSingle ( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle( bVisible );
	if ( bVisible )
	{
		ResetData();
	}
}

void CSelectCharacterList::ResetData()
{
	for( int i=0; i<SELECT_CHARACTER_LIST_MAX; ++i )
		m_pListItem[i]->ResetData();
}

