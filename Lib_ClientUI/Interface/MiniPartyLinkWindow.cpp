#include "StdAfx.h"
#include "MiniPartyLinkWindow.h"
#include "BasicTextBox.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "GameTextControl.h"
#include "UITextControl.h"
#include "MiniPartyLinkSlot.h"
#include "InnerInterface.h"
#include "PartyBuffInfo.h"
#include "BasicTextButton.h"
#include "./Util/RnButton.h"

#include "GLGaeaClient.h"
#include "GLPartyClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMiniPartyLinkWindow::CMiniPartyLinkWindow ()
		:	m_pButtonRequest ( NULL )
		,	m_dwMasterID(0)
		,	m_dwPartyID(0)
{
	memset( m_pPartySlot, 0, sizeof(m_pPartySlot) );
}

CMiniPartyLinkWindow::~CMiniPartyLinkWindow ()
{
}

void CMiniPartyLinkWindow::CreateSubControl ()
{	
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

	int nTextAlign = TEXT_ALIGN_LEFT;
	D3DCOLOR dwFontColor = NS_UITEXTCOLOR::DEFAULT;

	CBasicTextBox* pTextBox = NULL;

	CString strMemberKeyword[MAXPARTY] = 
	{
		"MINIPARTYLINK_SLOT_MEMBER0",
		"MINIPARTYLINK_SLOT_MEMBER1",
		"MINIPARTYLINK_SLOT_MEMBER2",
		"MINIPARTYLINK_SLOT_MEMBER3",
		"MINIPARTYLINK_SLOT_MEMBER4",
		"MINIPARTYLINK_SLOT_MEMBER5",
		"MINIPARTYLINK_SLOT_MEMBER6",
		"MINIPARTYLINK_SLOT_MEMBER7",
	};

	for ( int i = 0; i < MAXPARTY; i++ )
	{
		CMiniPartyLinkSlot* pPartySlot = new CMiniPartyLinkSlot;
		pPartySlot->CreateSub ( this, strMemberKeyword[i].GetString (), UI_FLAG_DEFAULT, MINIPARTYLINK_SLOT_MEMBER0 + i );
		pPartySlot->CreateSubControl ();
		RegisterControl ( pPartySlot );
		m_pPartySlot[i] = pPartySlot;
	}

	RnButton::CreateArg arg;
	arg.defaultTextureKey = "RNBUTTON_DEFAULT_TEXTURE";
	arg.mouseOverTextureKey = "RNBUTTON_OVER_TEXTURE";
	arg.mouseClickTextureKey = "RNBUTTON_CLICK_TEXTURE";
	arg.pFont = pFont;
	arg.dwColor = NS_UITEXTCOLOR::WHITE;
	//////////////////////////////////////////////////////////////////////////
    m_pButtonRequest = new RnButton();
    m_pButtonRequest->CreateSub( this, "MINIPARTYLINK_BUTTON_REQ", UI_FLAG_DEFAULT, MINIPARTYLINK_BUTTON_REQ );
	arg.text = (ID2GAMEWORD("MINIPARTYLINK_BUTTON_REQ",0));
    m_pButtonRequest->CreateSubControl(arg);
    RegisterControl(m_pButtonRequest);
}

void CMiniPartyLinkWindow::SetSlotMember ( CMiniPartyLinkSlot* pPartySlot, GLPARTY_CLIENT *pMember, const int nPartySlotID )
{
	if ( !pPartySlot || !pMember )
	{
		GASSERT ( 0 && "CMiniPartyLinkWindow::SetSlotMember()" );
		return ;
	}

	pPartySlot->SetPlayerData ( nPartySlotID, pMember->m_szName, (int)pMember->m_wLevel, (int)pMember->m_emClass );	
	pPartySlot->SetVisibleSingle ( TRUE );
}

void CMiniPartyLinkWindow::SetSlotData ( GLPARTY_CLIENT *pMember, const int nPartySlotID )
{
	if ( nPartySlotID >= MAXPARTY ) return;
	if ( m_pPartySlot[nPartySlotID] ) SetSlotMember ( m_pPartySlot[nPartySlotID], pMember, nPartySlotID );
}

void CMiniPartyLinkWindow::ReSetSlotData ()
{
	m_dwMasterID = 0;
	m_dwPartyID = 0;
	for ( int i = 0; i < MAXPARTY; i++ ) 
	{
		m_pPartySlot[i]->SetVisibleSingle ( FALSE );
	}
}

void CMiniPartyLinkWindow::SetData ( DWORD dwMasterID, DWORD dwPartyID )
{
	m_dwMasterID = dwMasterID;
	m_dwPartyID = dwPartyID;
}

void CMiniPartyLinkWindow::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if( !IsVisible() )	return ;

	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	//	파티 리스트 갱신
	//	
	//GLPARTY_CLIENT *pMaster = GLPartyClient::GetInstance().GetMaster();	//	마스터
	//if( pMaster )
	//{
	//	GLPARTY_CLIENT *pSelf = FindSelfClient ();
	//	//	NOTE
	//	//		만약, 자신을 찾지 못하면
	//	//		이 아래로의 작업은 아무것도 진행하지 않습니다.
	//	if ( !pSelf ) return ;

	//	SetSlotMember ( m_pPartySlot[0], pMaster, IsSameMap ( pSelf, pMaster ), 0 );

	//	DWORD nMEMBER_NUM = GLPartyClient::GetInstance().GetMemberNum();
	//	if( nMEMBER_NUM > 1 )
	//	{
	//		nMEMBER_NUM -= 1; // Note : 마스터는 제외
	//		for ( DWORD i = 0; i < nMEMBER_NUM; i++ )
	//		{
	//			GLPARTY_CLIENT *pMember = GLPartyClient::GetInstance().GetMember ( i );			
	//			if ( pMember )
	//			{				
	//				const BOOL bONESELF = pMember->ISONESELF ();

	//				SetSlotMember ( m_pPartySlot[i+1], pMember, IsSameMap ( pSelf, pMember ), i + 1 );
	//			}
	//		}
	//	}
	//}
}

void CMiniPartyLinkWindow::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	switch ( ControlID )
	{
	case ET_CONTROL_TITLE:
	case ET_CONTROL_TITLE_F:
		{
			if ( (dwMsg & UIMSG_LB_DUP) && CHECK_MOUSE_IN ( dwMsg ) )
			{
				CInnerInterface::GetInstance().SetDefaultPosInterface( MINIPARTYLINK_WINDOW );
			}
		}
		break;

	case ET_CONTROL_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				CInnerInterface::GetInstance().HideGroup ( GetWndID () );
			}
		}
		break;
	case MINIPARTYLINK_BUTTON_REQ:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				GLGaeaClient::GetInstance().GetCharacter()->ReqPartyLure ( m_dwPartyID, m_dwMasterID );
			}
		}
		break;
	}

	CUIWindowEx::TranslateUIMessage ( ControlID, dwMsg );
}