#include "StdAfx.h"
#include "PartySlotDisplay.h"
#include "./BasicTextBox.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "GameTextControl.h"
#include "UITextControl.h"
#include "PartyDisplay.h"
#include "InnerInterface.h"
#include "PartyDisplayBuffInfo.h"

#include "GLGaeaClient.h"
#include "GLPartyClient.h"
#include "BasicLineBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPartyDisplay::CPartyDisplay ()
{
	memset( m_pPartySlot, 0, sizeof(m_pPartySlot) );
}

CPartyDisplay::~CPartyDisplay ()
{
}

void CPartyDisplay::CreateSubControl ()
{	
	int nTextAlign = TEXT_ALIGN_LEFT;
	D3DCOLOR dwFontColor = NS_UITEXTCOLOR::DEFAULT;

	CBasicTextBox* pTextBox = NULL;
	
	CString strMemberKeyword[MAXPARTY] = 
	{
		"PARTYDISPLAY_SLOT_MEMBER0",
		"PARTYDISPLAY_SLOT_MEMBER1",
		"PARTYDISPLAY_SLOT_MEMBER2",
		"PARTYDISPLAY_SLOT_MEMBER3",
		"PARTYDISPLAY_SLOT_MEMBER4",
		"PARTYDISPLAY_SLOT_MEMBER5",
		"PARTYDISPLAY_SLOT_MEMBER6",
		"PARTYDISPLAY_SLOT_MEMBER7",
	};

	for ( int i = 0; i < MAXPARTY; i++ )
	{
		CPartySlotDisplay* pPartySlot = new CPartySlotDisplay;
		pPartySlot->CreateSub ( this, strMemberKeyword[i].GetString (), UI_FLAG_DEFAULT, PARTYDISPLAY_SLOT_MEMBER0 + i );
		pPartySlot->CreateSubControl ();
		RegisterControl ( pPartySlot );
		m_pPartySlot[i] = pPartySlot;
	}

	{
		CPartyDisplayBuffInfo* pPartyDisplayBuffInfo = new CPartyDisplayBuffInfo;
		pPartyDisplayBuffInfo->CreateSub ( this, "PARTYDISPLAY_BUFF_INFO", UI_FLAG_DEFAULT, PARTYDISPLAY_BUFF_INFO0 );
		pPartyDisplayBuffInfo->CreateSubControl ();
		RegisterControl ( pPartyDisplayBuffInfo );
		m_pPartyDisplayBuffInfo = pPartyDisplayBuffInfo;
	}
	{
		CPartyDisplayBuffInfo* pPartyDisplayBuffInfo1 = new CPartyDisplayBuffInfo;
		pPartyDisplayBuffInfo1->CreateSub ( this, "PARTYDISPLAY_BUFF_INFO1", UI_FLAG_DEFAULT, PARTYDISPLAY_BUFF_INFO1 );
		pPartyDisplayBuffInfo1->CreateSubControl ();
		RegisterControl ( pPartyDisplayBuffInfo1 );
		m_pPartyDisplayBuffInfo1 = pPartyDisplayBuffInfo1;
	}
	{
		CPartyDisplayBuffInfo* pPartyDisplayBuffInfo2 = new CPartyDisplayBuffInfo;
		pPartyDisplayBuffInfo2->CreateSub ( this, "PARTYDISPLAY_BUFF_INFO2", UI_FLAG_DEFAULT, PARTYDISPLAY_BUFF_INFO2 );
		pPartyDisplayBuffInfo2->CreateSubControl ();
		RegisterControl ( pPartyDisplayBuffInfo2 );
		m_pPartyDisplayBuffInfo2 = pPartyDisplayBuffInfo2;
	}
	{
		CPartyDisplayBuffInfo* pPartyDisplayBuffInfo3 = new CPartyDisplayBuffInfo;
		pPartyDisplayBuffInfo3->CreateSub ( this, "PARTYDISPLAY_BUFF_INFO3", UI_FLAG_DEFAULT, PARTYDISPLAY_BUFF_INFO3 );
		pPartyDisplayBuffInfo3->CreateSubControl ();
		RegisterControl ( pPartyDisplayBuffInfo3 );
		m_pPartyDisplayBuffInfo3 = pPartyDisplayBuffInfo3;
	}
	{
		CPartyDisplayBuffInfo* pPartyDisplayBuffInfo4 = new CPartyDisplayBuffInfo;
		pPartyDisplayBuffInfo4->CreateSub ( this, "PARTYDISPLAY_BUFF_INFO4", UI_FLAG_DEFAULT, PARTYDISPLAY_BUFF_INFO4 );
		pPartyDisplayBuffInfo4->CreateSubControl ();
		RegisterControl ( pPartyDisplayBuffInfo4 );
		m_pPartyDisplayBuffInfo4 = pPartyDisplayBuffInfo4;
	}
	{
		CPartyDisplayBuffInfo* pPartyDisplayBuffInfo5 = new CPartyDisplayBuffInfo;
		pPartyDisplayBuffInfo5->CreateSub ( this, "PARTYDISPLAY_BUFF_INFO5", UI_FLAG_DEFAULT, PARTYDISPLAY_BUFF_INFO5 );
		pPartyDisplayBuffInfo5->CreateSubControl ();
		RegisterControl ( pPartyDisplayBuffInfo5 );
		m_pPartyDisplayBuffInfo5 = pPartyDisplayBuffInfo5;
	}
	{
		CPartyDisplayBuffInfo* pPartyDisplayBuffInfo6 = new CPartyDisplayBuffInfo;
		pPartyDisplayBuffInfo6->CreateSub ( this, "PARTYDISPLAY_BUFF_INFO6", UI_FLAG_DEFAULT, PARTYDISPLAY_BUFF_INFO6 );
		pPartyDisplayBuffInfo6->CreateSubControl ();
		RegisterControl ( pPartyDisplayBuffInfo6 );
		m_pPartyDisplayBuffInfo6 = pPartyDisplayBuffInfo6;
	}
	{
		CPartyDisplayBuffInfo* pPartyDisplayBuffInfo7 = new CPartyDisplayBuffInfo;
		pPartyDisplayBuffInfo7->CreateSub ( this, "PARTYDISPLAY_BUFF_INFO7", UI_FLAG_DEFAULT, PARTYDISPLAY_BUFF_INFO7 );
		pPartyDisplayBuffInfo7->CreateSubControl ();
		RegisterControl ( pPartyDisplayBuffInfo7 );
		m_pPartyDisplayBuffInfo7 = pPartyDisplayBuffInfo7;
	}
}

void CPartyDisplay::SetSlotMember ( CPartySlotDisplay* pPartySlot, GLPARTY_CLIENT *pMember, BOOL bSameMap, const int nPartySlotID )
{
	if ( !pPartySlot || !pMember )
	{
		return ;
	}
	
	const int nClassType = CharClassToIndex ( pMember->m_emClass );
	
	pPartySlot->SetNumber( nPartySlotID ); 
	pPartySlot->SetClass ( nClassType /*, bSameMap*/ );
	pPartySlot->SetPlayerHealth ( pMember->m_sHP.dwNow, pMember->m_sHP.dwMax, pMember->m_szName/*, bSameMap*/ );
	pPartySlot->SetSchool (  pMember->m_wSchool/*, bSameMap*/ );
	
	float fHPPercent = 0.0f;
		
	if( bSameMap )
	{
		if( pMember->m_sHP.dwMax != 0 )
		{
			fHPPercent = float(pMember->m_sHP.dwNow) / float(pMember->m_sHP.dwMax);
		}
		pPartySlot->SetHP( fHPPercent );
	}
	else
	{
		fHPPercent = 100.0f;
		pPartySlot->SetHP( fHPPercent );
	}
	
	pPartySlot->SetVisibleSingle ( TRUE );
}

void CPartyDisplay::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if( !IsVisible() )
	{
		m_pPartyDisplayBuffInfo->SetVisibleSingle ( FALSE );
		m_pPartyDisplayBuffInfo1->SetVisibleSingle ( FALSE );
		m_pPartyDisplayBuffInfo2->SetVisibleSingle ( FALSE );
		m_pPartyDisplayBuffInfo3->SetVisibleSingle ( FALSE );
		m_pPartyDisplayBuffInfo4->SetVisibleSingle ( FALSE );
		m_pPartyDisplayBuffInfo5->SetVisibleSingle ( FALSE );
		m_pPartyDisplayBuffInfo6->SetVisibleSingle ( FALSE );
		m_pPartyDisplayBuffInfo7->SetVisibleSingle ( FALSE );
	}

	if ( m_pPartySlot[0]->IsVisible() )
	{
		if ( m_bBuffOK )
		{
			m_pPartyDisplayBuffInfo->SetVisibleSingle ( TRUE );
		}
		else
		{
			m_pPartyDisplayBuffInfo->SetVisibleSingle ( FALSE );
		}
	}
	else
	{
		m_pPartyDisplayBuffInfo->SetVisibleSingle ( FALSE );
	}
	
	if ( m_pPartySlot[1]->IsVisible() )
	{
		if ( m_bBuffOK )
		{
			m_pPartyDisplayBuffInfo1->SetVisibleSingle ( TRUE );
		}
		else
		{
			m_pPartyDisplayBuffInfo1->SetVisibleSingle ( FALSE );
		}
	}
	else
	{
		m_pPartyDisplayBuffInfo1->SetVisibleSingle ( FALSE );
	}
	
	if ( m_pPartySlot[2]->IsVisible() )
	{
		if ( m_bBuffOK )
		{
			m_pPartyDisplayBuffInfo2->SetVisibleSingle ( TRUE );
		}
		else
		{
			m_pPartyDisplayBuffInfo2->SetVisibleSingle ( FALSE );
		}
	}
	else
	{
		m_pPartyDisplayBuffInfo2->SetVisibleSingle ( FALSE );
	}
	
	if ( m_pPartySlot[3]->IsVisible() )
	{
		if ( m_bBuffOK )
		{
			m_pPartyDisplayBuffInfo3->SetVisibleSingle ( TRUE );
		}
		else
		{
			m_pPartyDisplayBuffInfo3->SetVisibleSingle ( FALSE );
		}
	}
	else
	{
		m_pPartyDisplayBuffInfo3->SetVisibleSingle ( FALSE );
	}
	
	if ( m_pPartySlot[4]->IsVisible() )
	{
		if ( m_bBuffOK )
		{
			m_pPartyDisplayBuffInfo4->SetVisibleSingle ( TRUE );
		}
		else
		{
			m_pPartyDisplayBuffInfo4->SetVisibleSingle ( FALSE );
		}
	}
	else
	{
		m_pPartyDisplayBuffInfo4->SetVisibleSingle ( FALSE );
	}
	
	if ( m_pPartySlot[5]->IsVisible() )
	{
		if ( m_bBuffOK )
		{
			m_pPartyDisplayBuffInfo5->SetVisibleSingle ( TRUE );
		}
		else
		{
			m_pPartyDisplayBuffInfo5->SetVisibleSingle ( FALSE );
		}
	}
	else
	{
		m_pPartyDisplayBuffInfo5->SetVisibleSingle ( FALSE );
	}
	
	if ( m_pPartySlot[6]->IsVisible() )
	{
		if ( m_bBuffOK )
		{
			m_pPartyDisplayBuffInfo6->SetVisibleSingle ( TRUE );
		}
		else
		{
			m_pPartyDisplayBuffInfo6->SetVisibleSingle ( FALSE );
		}
	}
	else
	{
		m_pPartyDisplayBuffInfo6->SetVisibleSingle ( FALSE );
	}
	
	if ( m_pPartySlot[7]->IsVisible() )
	{
		if ( m_bBuffOK )
		{
			m_pPartyDisplayBuffInfo7->SetVisibleSingle ( TRUE );
		}
		else
		{
			m_pPartyDisplayBuffInfo7->SetVisibleSingle ( FALSE );
		}
	}
	else
	{
		m_pPartyDisplayBuffInfo7->SetVisibleSingle ( FALSE );
	}

	/*m_bCHECK_MOUSE_STATE = false; 

	m_PosX = x;
	m_PosY = y;

	const UIRECT& rcGlobalPos = GetGlobalPos ();

	m_bMOUSEIN = false; 

	m_rcOriginalPos = GetGlobalPos();*/
	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	/*if ( IsExclusiveSelfControl() )
	{
		SetLocalPos ( D3DXVECTOR2 ( x - m_vGap.x, y - m_vGap.y ) );
	}*/

	for ( int i = 0; i < MAXPARTY; i++ ) 
	{
		m_pPartySlot[i]->SetVisibleSingle ( FALSE );
	}

	GLPARTY_CLIENT *pMaster = GLPartyClient::GetInstance().GetMaster();	

	if ( !pMaster )
	{
		const UIRECT& rcOriginPos = GetLocalPos ();
		UIRECT rcLocalNewPos = UIRECT ( 50, 110, rcOriginPos.sizeX, rcOriginPos.sizeY );

		SetGlobalPos ( rcLocalNewPos );
	}

	if( pMaster )
	{
		GLPARTY_CLIENT *pSelf = FindSelfClient ();
		if ( !pSelf ) return ;

		SetSlotMember ( m_pPartySlot[0], pMaster, IsSameMap ( pSelf, pMaster ), 0 );

		DWORD nMEMBER_NUM = GLPartyClient::GetInstance().GetMemberNum();
		if( nMEMBER_NUM > 1 )
		{
			nMEMBER_NUM -= 1; 
			for ( DWORD i = 0; i < nMEMBER_NUM; i++ )
			{
				GLPARTY_CLIENT *pMember = GLPartyClient::GetInstance().GetMember ( i );			
				if ( pMember )
				{				
					const BOOL bONESELF = pMember->ISONESELF ();

					SetSlotMember ( m_pPartySlot[i+1], pMember, IsSameMap ( pSelf, pMember ), i + 1 );
				}
			}
		}
	}
}


BOOL CPartyDisplay::IsSameMap ( GLPARTY_CLIENT *pSelf, GLPARTY_CLIENT *pMember )
{
	if ( !pSelf || !pMember )
	{
		GASSERT ( 0 && "??? ??????." );
		return FALSE;
	}

	return pSelf->m_sMapID.dwID == pMember->m_sMapID.dwID;
}

GLPARTY_CLIENT*	CPartyDisplay::FindSelfClient ()
{
	GLPARTY_CLIENT *pMaster = GLPartyClient::GetInstance().GetMaster();
	if ( pMaster && pMaster->ISONESELF() )
	{
		return pMaster;
	}
	else
	{
		DWORD nMEMBER_NUM = GLPartyClient::GetInstance().GetMemberNum();
		if( nMEMBER_NUM > 1 )
		{
			nMEMBER_NUM -= 1;
			for ( DWORD i = 0; i < nMEMBER_NUM; i++ )
			{
				GLPARTY_CLIENT *pMember = GLPartyClient::GetInstance().GetMember( i );
				if ( pMember && pMember->ISONESELF () )
				{
					return pMember;				
				}
			}
		}
	}

	return NULL;
}

void CPartyDisplay::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{

	/*if ( !m_bCHECK_MOUSE_STATE )
	{
		CheckMouseState ();
		m_bCHECK_MOUSE_STATE = true;
	}*/

	switch ( ControlID )
	{
	case PARTYDISPLAY_SLOT_MEMBER0:
		{
			if ( m_bBuffOK )
			{
				m_pPartyDisplayBuffInfo->SetPartyID ( 0 );
				m_pPartyDisplayBuffInfo->SetVisibleSingle ( TRUE );
			}
			else
			{
				m_pPartyDisplayBuffInfo->SetPartyID ( 0 );
				m_pPartyDisplayBuffInfo->SetVisibleSingle ( FALSE );
			}
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( dwMsg & UIMSG_MOUSEIN_RB_EVENT2 )
				{
					GLPARTY_CLIENT *pMaster = GLPartyClient::GetInstance().GetMaster();
					if ( pMaster )
					{
						STARGETID sTARID(CROW_PC,pMaster->m_dwGaeaID);
						GLGaeaClient::GetInstance().GetCharacter ()->ReqSkillReaction(sTARID);
					}
				}
			}
		}
		break;

	case PARTYDISPLAY_SLOT_MEMBER1:
	case PARTYDISPLAY_SLOT_MEMBER2:
	case PARTYDISPLAY_SLOT_MEMBER3:
	case PARTYDISPLAY_SLOT_MEMBER4:
	case PARTYDISPLAY_SLOT_MEMBER5:
	case PARTYDISPLAY_SLOT_MEMBER6:
	case PARTYDISPLAY_SLOT_MEMBER7:
		{
			int nIndex = ControlID - PARTYDISPLAY_SLOT_MEMBER0;

			if ( m_bBuffOK )
			{
				if ( nIndex == 1 ){	m_pPartyDisplayBuffInfo1->SetPartyID ( 1 );
				m_pPartyDisplayBuffInfo1->SetVisibleSingle ( TRUE ); }
				else if ( nIndex == 2 ){	m_pPartyDisplayBuffInfo2->SetPartyID ( 2 );
				m_pPartyDisplayBuffInfo2->SetVisibleSingle ( TRUE ); }
				else if ( nIndex == 3 ){	m_pPartyDisplayBuffInfo3->SetPartyID ( 3 );
				m_pPartyDisplayBuffInfo3->SetVisibleSingle ( TRUE ); }
				else if ( nIndex == 4 ){	m_pPartyDisplayBuffInfo4->SetPartyID ( 4 );
				m_pPartyDisplayBuffInfo4->SetVisibleSingle ( TRUE ); }
				else if ( nIndex == 5 ){	m_pPartyDisplayBuffInfo5->SetPartyID ( 5 );
				m_pPartyDisplayBuffInfo5->SetVisibleSingle ( TRUE ); }
				else if ( nIndex == 6 ){	m_pPartyDisplayBuffInfo6->SetPartyID ( 6 );
				m_pPartyDisplayBuffInfo6->SetVisibleSingle ( TRUE ); }
				else if ( nIndex == 7 ){	m_pPartyDisplayBuffInfo7->SetPartyID ( 7 );
				m_pPartyDisplayBuffInfo7->SetVisibleSingle ( TRUE ); }
			}
			else
			{
				if ( nIndex == 1 ){	m_pPartyDisplayBuffInfo1->SetPartyID ( 1 );
				m_pPartyDisplayBuffInfo1->SetVisibleSingle ( FALSE ); }
				else if ( nIndex == 2 ){	m_pPartyDisplayBuffInfo2->SetPartyID ( 2 );
				m_pPartyDisplayBuffInfo2->SetVisibleSingle ( FALSE ); }
				else if ( nIndex == 3 ){	m_pPartyDisplayBuffInfo3->SetPartyID ( 3 );
				m_pPartyDisplayBuffInfo3->SetVisibleSingle ( FALSE ); }
				else if ( nIndex == 4 ){	m_pPartyDisplayBuffInfo4->SetPartyID ( 4 );
				m_pPartyDisplayBuffInfo4->SetVisibleSingle ( FALSE ); }
				else if ( nIndex == 5 ){	m_pPartyDisplayBuffInfo5->SetPartyID ( 5 );
				m_pPartyDisplayBuffInfo5->SetVisibleSingle ( FALSE ); }
				else if ( nIndex == 6 ){	m_pPartyDisplayBuffInfo6->SetPartyID ( 6 );
				m_pPartyDisplayBuffInfo6->SetVisibleSingle ( FALSE ); }
				else if ( nIndex == 7 ){	m_pPartyDisplayBuffInfo7->SetPartyID ( 7 );
				m_pPartyDisplayBuffInfo7->SetVisibleSingle ( FALSE ); }
			}

			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( dwMsg & UIMSG_MOUSEIN_RB_EVENT2 )
				{
					GLPARTY_CLIENT *pMaster = GLPartyClient::GetInstance().GetMaster();
					if ( pMaster )
					{
						GLPARTY_CLIENT *pMember = GLPartyClient::GetInstance().GetMember( nIndex );
						if ( pMember )
						{
							STARGETID sTARID(CROW_PC,pMember->m_dwGaeaID);
							GLGaeaClient::GetInstance().GetCharacter ()->ReqSkillReaction(sTARID);
						}
					}
				}
			}
		}
		break;

	case ET_CONTROL_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				CInnerInterface::GetInstance().HideGroup ( GetWndID () );
				CInnerInterface::GetInstance().ShowGroupBottom ( PARTY_DISPLAY );
			}
		}
		break;
	}

	CUIWindowEx::TranslateUIMessage ( ControlID, dwMsg );
}

void CPartyDisplay::SetBuff ()
{
	m_pPartyDisplayBuffInfo->SetVisibleSingle ( TRUE ); 
	m_pPartyDisplayBuffInfo1->SetVisibleSingle ( TRUE ); 
	m_pPartyDisplayBuffInfo2->SetVisibleSingle ( TRUE ); 
	m_pPartyDisplayBuffInfo3->SetVisibleSingle ( TRUE ); 
	m_pPartyDisplayBuffInfo4->SetVisibleSingle ( TRUE ); 
	m_pPartyDisplayBuffInfo5->SetVisibleSingle ( TRUE ); 
	m_pPartyDisplayBuffInfo6->SetVisibleSingle ( TRUE ); 
	m_pPartyDisplayBuffInfo7->SetVisibleSingle ( TRUE ); 
	m_bBuffOK = true;
}

void CPartyDisplay::HideBuff ()
{
	m_pPartyDisplayBuffInfo->SetVisibleSingle ( FALSE ); 
	m_pPartyDisplayBuffInfo1->SetVisibleSingle ( FALSE ); 
	m_pPartyDisplayBuffInfo2->SetVisibleSingle ( FALSE ); 
	m_pPartyDisplayBuffInfo3->SetVisibleSingle ( FALSE ); 
	m_pPartyDisplayBuffInfo4->SetVisibleSingle ( FALSE ); 
	m_pPartyDisplayBuffInfo5->SetVisibleSingle ( FALSE ); 
	m_pPartyDisplayBuffInfo6->SetVisibleSingle ( FALSE ); 
	m_pPartyDisplayBuffInfo7->SetVisibleSingle ( FALSE ); 
	m_bBuffOK = false;
}

void CPartyDisplay::CheckMouseState ()
{
	const DWORD dwMsg = GetMessageEx ();
	if ( CHECK_MOUSE_IN ( dwMsg ) )
	{	
		if ( dwMsg & UIMSG_LB_DOWN )
		{
			SetExclusiveControl();	

			UIRECT rcPos = GetGlobalPos ();
			m_vGap.x = m_PosX - rcPos.left;
			m_vGap.y = m_PosY - rcPos.top;
		}
		else if ( CHECK_LB_UP_LIKE ( dwMsg ) )
		{
			ResetExclusiveControl();			
		}
	}
	else if ( CHECK_LB_UP_LIKE ( dwMsg ) )		
	{								
		ResetExclusiveControl();				
	}
}