#include "StdAfx.h"
#include "GLPetClient.h"
#include "GLGaeaClient.h"
#include "GLContrlMsg.h"
#include "../Lib_ClientUI/Interface/InnerInterface.h"
#include "../Lib_ClientUI/Interface/PetWindow.h"
#include "../Lib_ClientUI/Interface/UItextControl.h"
#include "../Lib_ClientUI/Interface/GameTextControl.h"
#include "GLItemMan.h"
#include "DxEffGroupPlayer.h"
#include "GLFactEffect.h"

/*dual pet skill, Juver, 2017/12/28 */
void GLPetClient::MsgGetRightOfItem_A ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_REQ_GETRIGHTOFITEM_A_FB* pNetMsg = ( GLMSG::SNETPET_REQ_GETRIGHTOFITEM_A_FB* ) nmg;

	switch ( pNetMsg->emFB )
	{
	case EMPET_REQ_GETRIGHTOFITEM_FB_OK:
		{
			m_vecDroppedItems_A.clear();
			for ( WORD i = 0; i < pNetMsg->wSum; ++i )
			{
				m_vecDroppedItems_A.push_back ( pNetMsg->DroppedItems[i] );
			}
		}
		m_bSkillProcessing_A = true;
		break;
	};
}

/*dual pet skill, Juver, 2017/12/28 */
void GLPetClient::MsgGetRightOfItem_B ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_REQ_GETRIGHTOFITEM_B_FB* pNetMsg = ( GLMSG::SNETPET_REQ_GETRIGHTOFITEM_B_FB* ) nmg;

	switch ( pNetMsg->emFB )
	{
	case EMPET_REQ_GETRIGHTOFITEM_FB_OK:
		{
			m_vecDroppedItems_B.clear();
			for ( WORD i = 0; i < pNetMsg->wSum; ++i )
			{
				m_vecDroppedItems_B.push_back ( pNetMsg->DroppedItems[i] );
			}
		}
		m_bSkillProcessing_B = true;
		break;
	};
}


/*dual pet skill, Juver, 2017/12/27 */
void GLPetClient::MsgSkillChange_A ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_ADD_SKILLFACT_A* pNetMsg = ( GLMSG::SNETPET_ADD_SKILLFACT_A* ) nmg;

	m_pOwner->m_sPETSKILLFACT_A.sNATIVEID = pNetMsg->sNATIVEID;
	m_pOwner->m_sPETSKILLFACT_A.emTYPE	= pNetMsg->emTYPE;
	m_pOwner->m_sPETSKILLFACT_A.fMVAR		= pNetMsg->fMVAR;
	m_pOwner->m_sPETSKILLFACT_A.fAGE		= pNetMsg->fAge;

	// 이전 이펙트 제거
	FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_pOwner->m_dwGaeaID,m_pOwner->GetPosition() ), m_pOwner->GetSkinChar (), m_sActiveSkillID_A );
	
	m_sActiveSkillID_A = pNetMsg->sNATIVEID;

	//	지속형 스킬의 경우 스킬 팩터 추가됨.
	if ( m_sActiveSkillID_A != SNATIVEID(false) )
	{
		// 주인이 타겟이된다.
		STARGETID sTargetID( CROW_PC, m_pOwner->m_dwGaeaID, m_pOwner->GetPosition () );
		FACTEFF::NewSkillFactEffect ( sTargetID, m_pOwner->GetSkinChar (), m_sActiveSkillID_A, m_matTrans, sTargetID.vPos );
	}

	// 스킬구동
	StartSkillProc_A ();

	/*enable pet skill at pet creation, Juver, 2017/12/13 */
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_sActiveSkillID_A );
	if ( pSkill )
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("PETSKILL_ACTIVATE"), pSkill->GetName() );
}


void GLPetClient::MsgSkillChange_B ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_ADD_SKILLFACT_B* pNetMsg = ( GLMSG::SNETPET_ADD_SKILLFACT_B* ) nmg;

	m_pOwner->m_sPETSKILLFACT_B.sNATIVEID = pNetMsg->sNATIVEID;
	m_pOwner->m_sPETSKILLFACT_B.emTYPE	= pNetMsg->emTYPE;
	m_pOwner->m_sPETSKILLFACT_B.fMVAR		= pNetMsg->fMVAR;
	m_pOwner->m_sPETSKILLFACT_B.fAGE		= pNetMsg->fAge;

	// 이전 이펙트 제거
	FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_pOwner->m_dwGaeaID,m_pOwner->GetPosition() ), m_pOwner->GetSkinChar (), m_sActiveSkillID_B );

	m_sActiveSkillID_B = pNetMsg->sNATIVEID;

	//	지속형 스킬의 경우 스킬 팩터 추가됨.
	if ( m_sActiveSkillID_B != SNATIVEID(false) )
	{
		// 주인이 타겟이된다.
		STARGETID sTargetID( CROW_PC, m_pOwner->m_dwGaeaID, m_pOwner->GetPosition () );
		FACTEFF::NewSkillFactEffect ( sTargetID, m_pOwner->GetSkinChar (), m_sActiveSkillID_B, m_matTrans, sTargetID.vPos );
	}

	// 스킬구동
	StartSkillProc_B ();

	/*enable pet skill at pet creation, Juver, 2017/12/13 */
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_sActiveSkillID_B );
	if ( pSkill )
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("PETSKILL_ACTIVATE"), pSkill->GetName() );
}

void GLPetClient::MsgLearnSkill ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_REQ_LEARNSKILL_FB* pNetMsg = ( GLMSG::SNETPET_REQ_LEARNSKILL_FB* ) nmg;

	switch ( pNetMsg->emFB )
	{
	case EMPET_REQ_LEARNSKILL_FB_OK:
		{
			SINVENITEM* pInvenItem = m_pOwner->m_cInventory.FindPosItem ( pNetMsg->wPosX, pNetMsg->wPosY );
			if ( !pInvenItem )	return;

			SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
			if ( !pItem )		return;

			// 스킬습득
			LEARN_SKILL ( pItem->sSkillBookOp.sSkill_ID );

			// 팻카드의 정보도 갱신해준다.
			PETCARDINFO_MAP_ITER iter = m_pOwner->m_mapPETCardInfo.find ( m_dwPetID );
			if ( iter!=m_pOwner->m_mapPETCardInfo.end() )
			{
				SPETCARDINFO& sPet = (*iter).second;
				PETSKILL sPetSkill( pItem->sSkillBookOp.sSkill_ID, 0 );
				sPet.m_ExpSkills.insert ( std::make_pair(sPetSkill.sNativeID.dwID,sPetSkill) );
			}

			// 스킬 배움 이팩트 추가
			STARGETID sTargetID(CROW_PET, m_dwGUID, m_vPos);
			DxEffGroupPlayer::GetInstance().NewEffGroup
			(
				GLCONST_CHAR::strSKILL_LEARN_EFFECT.c_str(),
				m_matTrans,
				&sTargetID
			);

			// 스킬콤보박스 리프레쉬
			CInnerInterface::GetInstance().GetPetWindow()->SetSkillCombo();

			CInnerInterface::GetInstance().PrintMsgText ( 
				NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("EMPET_REQ_LEARNSKILL_FB_OK"), m_szName );
		}
		break;
	case EMPET_REQ_LEARNSKILL_FB_FAIL:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_LEARNSKILL_FB_FAIL") );
		break;
	case EMPET_REQ_LEARNSKILL_FB_NOITEM:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_LEARNSKILL_FB_NOITEM") );
		break;
	case EMPET_REQ_LEARNSKILL_FB_BADITEM:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_LEARNSKILL_FB_BADITEM") );
		break;
	case EMPET_REQ_LEARNSKILL_FB_ALREADY:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_LEARNSKILL_FB_ALREADY") );
		break;
	};
}

void GLPetClient::MsgUpdateFull ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_UPDATE_CLIENT_PETFULL* pNetMsg = ( GLMSG::SNETPET_UPDATE_CLIENT_PETFULL* ) nmg;
	m_nFull = pNetMsg->nFull;

	// 팻카드의 정보도 갱신해준다.
	PETCARDINFO_MAP_ITER iter = m_pOwner->m_mapPETCardInfo.find ( m_dwPetID );
	if ( iter!=m_pOwner->m_mapPETCardInfo.end() )
	{
		SPETCARDINFO& sPet = (*iter).second;
		sPet.m_nFull = pNetMsg->nFull;
	}
}


void GLPetClient::MsgRename ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_REQ_RENAME_FB* pNetMsg = ( GLMSG::SNETPET_REQ_RENAME_FB* ) nmg;

	switch ( pNetMsg->emFB )
	{
	case EMPET_REQ_RENAME_FB_FB_OK:
		{
			CInnerInterface::GetInstance().PrintMsgText ( 
				NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("EMPET_REQ_RENAME_FB_FB_OK"), m_szName, pNetMsg->szName );
			StringCchCopy ( m_szName, PETNAMESIZE+1, pNetMsg->szName );

			// 팻카드의 정보도 갱신해준다.
			PETCARDINFO_MAP_ITER iter = m_pOwner->m_mapPETCardInfo.find ( m_dwPetID );
			if ( iter!=m_pOwner->m_mapPETCardInfo.end() )
			{
				SPETCARDINFO& sPet = (*iter).second;
				StringCchCopy ( sPet.m_szName, PETNAMESIZE+1, pNetMsg->szName );
			}
		}
		break;

	case EMPET_REQ_RENAME_FB_FB_FAIL:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMINVEN_RENAME_FROM_DB_FAIL") );
		break;

	case EMPET_REQ_RENAME_FB_FB_NOITEM:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMINVEN_RENAME_FB_NOITEM") );
		break;

	case EMPET_REQ_RENAME_FB_FB_BADITEM:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMINVEN_RENAME_FB_BADITEM") );
		break;

	case EMPET_REQ_RENAME_FB_FB_LENGTH:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEEXTEXT("NEWCHAR_NAME_TOO_SHORT") );	
		break;

	case EMPET_REQ_RENAME_FB_BAD_NAME:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEEXTEXT("CHARACTER_BADNAME") );
		break;

	case EMPET_REQ_RENAME_FB_FROM_DB_FAIL:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMINVEN_RENAME_FROM_DB_FAIL"), pNetMsg->szName );
		break;
	case EMPET_REQ_RENAME_FB_FB_THAICHAR_ERROR:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMINVEN_RENAME_FB_THAICHAR_ERROR"));
		break;
	case EMPET_REQ_RENAME_FB_FB_VNCHAR_ERROR:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMINVEN_RENAME_FB_VNCHAR_ERROR"));
		break;
	};
}

void GLPetClient::MsgChangeColor ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_REQ_CHANGE_COLOR_FB* pNetMsg = ( GLMSG::SNETPET_REQ_CHANGE_COLOR_FB* ) nmg;

	switch ( pNetMsg->emFB )
	{
	case EMPET_REQ_COLOR_CHANGE_FB_OK:
		{
			m_wColor = pNetMsg->wColor;
			UpdateSuit ( FALSE );

			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("EMPET_REQ_COLOR_CHANGE_FB_OK") );
		}
		break;

	case EMPET_REQ_STYLECOLOR_CHANGE_FB_FAIL:
		ColorChange( m_wColor );
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_STYLECOLOR_CHANGE_FB_FAIL") );
		break;

	case EMPET_REQ_STYLECOLOR_CHANGE_FB_NOITEM:
		ColorChange( m_wColor );
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_STYLECOLOR_CHANGE_FB_NOITEM") );
		break;

	case EMPET_REQ_STYLECOLOR_CHANGE_FB_BADITEM:
		ColorChange( m_wColor );
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_STYLECOLOR_CHANGE_FB_BADITEM") );
		break;
	case EMPET_REQ_STYLECOLOR_CHANGE_FB_UNCHANGEABLE:
		ColorChange( m_wColor );
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_STYLECOLOR_CHANGE_FB_UNCHANGEABLE") );
		break;
	case EMPET_REQ_STYLECOLOR_CHANGE_FB_COOLTIME:		
		ColorChange( m_wColor );
		break;
	case EMPET_REQ_STYLECOLOR_CHANGE_FB_SKINPACK:
		ColorChange( m_wColor );
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_STYLECOLOR_CHANGE_FB_SKINPACK") );
		break;
	};
}

void GLPetClient::MsgChangeStyle ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_REQ_CHANGE_STYLE_FB* pNetMsg = ( GLMSG::SNETPET_REQ_CHANGE_STYLE_FB* ) nmg;

	switch ( pNetMsg->emFB )
	{
	case EMPET_REQ_STYLE_CHANGE_FB_OK:
		{
			m_wStyle = pNetMsg->wStyle;
			m_wColor = pNetMsg->wColor;
			UpdateSuit ( TRUE );
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("EMPET_REQ_STYLE_CHANGE_FB_OK") );
		}
		break;

	case EMPET_REQ_STYLECOLOR_CHANGE_FB_FAIL:
		StyleChange( m_wStyle );
		ColorChange( m_wColor );
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_STYLECOLOR_CHANGE_FB_FAIL") );
		break;

	case EMPET_REQ_STYLECOLOR_CHANGE_FB_NOITEM:
		StyleChange( m_wStyle );
		ColorChange( m_wColor );
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_STYLECOLOR_CHANGE_FB_NOITEM") );
		break;

	case EMPET_REQ_STYLECOLOR_CHANGE_FB_BADITEM:
		StyleChange( m_wStyle );
		ColorChange( m_wColor );
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_STYLECOLOR_CHANGE_FB_BADITEM") );
		break;
	case EMPET_REQ_STYLECOLOR_CHANGE_FB_COOLTIME:
		StyleChange( m_wStyle );
		ColorChange( m_wColor );
		break;
	case EMPET_REQ_STYLECOLOR_CHANGE_FB_SKINPACK:
		StyleChange( m_wStyle );
		ColorChange( m_wColor );
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_STYLECOLOR_CHANGE_FB_SKINPACK") );
		break;
	};
}

void GLPetClient::MsgSlotExHold ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_REQ_SLOT_EX_HOLD_FB* pNetMsg = ( GLMSG::SNETPET_REQ_SLOT_EX_HOLD_FB* ) nmg;

	switch ( pNetMsg->emFB )
	{
	case EMPET_REQ_SLOT_EX_HOLD_FB_FAIL:

		break;

	case EMPET_REQ_SLOT_EX_HOLD_FB_OK:
		{
			// 장착된 아이템 BackUp
			SITEMCUSTOM sSlotItemCustom = GetSlotitembySuittype ( pNetMsg->emSuit );

			// 손에든 아이템을 팻에게 장착시키고
			SetSlotItem ( pNetMsg->emSuit, m_pOwner->GET_HOLD_ITEM () );

			// 팻카드의 정보도 갱신해준다.
			PETCARDINFO_MAP_ITER iter = m_pOwner->m_mapPETCardInfo.find ( m_dwPetID );
			if ( iter!=m_pOwner->m_mapPETCardInfo.end() )
			{
				SPETCARDINFO& sPet = (*iter).second;
				sPet.m_PutOnItems[(WORD)pNetMsg->emSuit-(WORD)SUIT_PET_A] = m_pOwner->GET_HOLD_ITEM ();
			}

			// 장착됐던 아이템을 손에 든다
			m_pOwner->HOLD_ITEM ( sSlotItemCustom );

			UpdateSuit ( FALSE );
		}
		break;

	case EMPET_REQ_SLOT_EX_HOLD_FB_NOMATCH:
		break;

	case EMPET_REQ_SLOT_EX_HOLD_FB_INVALIDITEM:
		break;
	};
}

void GLPetClient::MsgHoldToSlot ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_REQ_HOLD_TO_SLOT_FB* pNetMsg = ( GLMSG::SNETPET_REQ_HOLD_TO_SLOT_FB* ) nmg;

	switch ( pNetMsg->emFB )
	{
	case EMPET_REQ_SLOT_EX_HOLD_FB_FAIL:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_SLOT_EX_HOLD_FB_FAIL") );
		break;

	case EMPET_REQ_SLOT_EX_HOLD_FB_OK:
		{
			// 손에든 아이템을 팻에게 장착시키고
			SetSlotItem ( pNetMsg->emSuit, m_pOwner->GET_HOLD_ITEM () );

			// 팻카드의 정보도 갱신해준다.
			PETCARDINFO_MAP_ITER iter = m_pOwner->m_mapPETCardInfo.find ( m_dwPetID );
			if ( iter!=m_pOwner->m_mapPETCardInfo.end() )
			{
				SPETCARDINFO& sPet = (*iter).second;
				sPet.m_PutOnItems[(WORD)pNetMsg->emSuit-(WORD)SUIT_PET_A] = m_pOwner->GET_HOLD_ITEM ();
			}

			// 손에든 아이템을 제거하고
			m_pOwner->RELEASE_HOLD_ITEM ();

			UpdateSuit ( FALSE );
		}
		break;

	case EMPET_REQ_SLOT_EX_HOLD_FB_NOMATCH:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_SLOT_EX_HOLD_FB_NOMATCH") );
		break;

	case EMPET_REQ_SLOT_EX_HOLD_FB_INVALIDITEM:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_SLOT_EX_HOLD_FB_INVALIDITEM") );
		break;
	};
}

void GLPetClient::MsgSlotToHold ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_REQ_SLOT_TO_HOLD_FB* pNetMsg = ( GLMSG::SNETPET_REQ_SLOT_TO_HOLD_FB* ) nmg;

	switch ( pNetMsg->emFB )
	{
	case EMPET_REQ_SLOT_EX_HOLD_FB_FAIL:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_SLOT_EX_HOLD_FB_FAIL") );
		break;

	case EMPET_REQ_SLOT_EX_HOLD_FB_OK:
		{
			// 장착된 아이템 BackUp
			SITEMCUSTOM sSlotItemCustom = GetSlotitembySuittype ( pNetMsg->emSuit );

			// 장착된 아이템 제거
			ReSetSlotItem ( pNetMsg->emSuit );

			// 팻카드의 정보도 갱신해준다.
			PETCARDINFO_MAP_ITER iter = m_pOwner->m_mapPETCardInfo.find ( m_dwPetID );
			if ( iter!=m_pOwner->m_mapPETCardInfo.end() )
			{
				SPETCARDINFO& sPet = (*iter).second;
				WORD i = (WORD)pNetMsg->emSuit-(WORD)SUIT_PET_A;
				if ( i < PET_ACCETYPE_SIZE ) sPet.m_PutOnItems[i] = SITEMCUSTOM ( NATIVEID_NULL() );
			}

			// 장착됐던 아이템을 손에 든다
			m_pOwner->HOLD_ITEM ( sSlotItemCustom );

			UpdateSuit ( FALSE );
		}
		break;

	case EMPET_REQ_SLOT_EX_HOLD_FB_NOMATCH:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_SLOT_EX_HOLD_FB_NOMATCH") );
		break;

	case EMPET_REQ_SLOT_EX_HOLD_FB_INVALIDITEM:
		CInnerInterface::GetInstance().PrintMsgText ( 
			NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_SLOT_EX_HOLD_FB_INVALIDITEM") );
		break;
	};
}

void GLPetClient::MsgRemoveSlotItem ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_REQ_REMOVE_SLOTITEM_FB* pNetMsg = ( GLMSG::SNETPET_REQ_REMOVE_SLOTITEM_FB* ) nmg;
	
	WORD wPosX(0), wPosY(0);

	SITEMCUSTOM	sSlotItemCustom = GetSlotitembySuittype ( pNetMsg->emSuit );

	SITEM* pSlotItem = GLItemMan::GetInstance().GetItem ( sSlotItemCustom.sNativeID );
	if ( !pSlotItem ) 
	{
		// 일반 오류
		return;
	}

	BOOL bOk = m_pOwner->m_cInventory.FindInsrtable ( pSlotItem->sBasicOp.wInvenSizeX, pSlotItem->sBasicOp.wInvenSizeY, wPosX, wPosY );
	if ( !bOk )
	{
		//	인밴이 가득찻음.
		return;
	}

	// 인벤에 넣기
	m_pOwner->m_cInventory.InsertItem ( sSlotItemCustom, wPosX, wPosY );

	// 슬롯아이템 제거
	ReSetSlotItem ( pNetMsg->emSuit );

	UpdateSuit ( FALSE );
}

void GLPetClient::MsgAttack ( NET_MSG_GENERIC* nmg )
{
	// 이동중이면 이동을 마친후 공격 모션을 취한다.
	if ( m_actorMove.PathIsActive () ) return;

	GLMSG::SNETPET_ATTACK* pNetMsg = ( GLMSG::SNETPET_ATTACK* ) nmg;

	if ( pNetMsg->sTarID.emCrow == CROW_PC )
	{
		GLLandManClient* pLandMan = GLGaeaClient::GetInstance().GetActiveMap ();
		PGLCHARCLIENT pChar = pLandMan->GetChar ( pNetMsg->sTarID.dwID );
		if ( pChar )
		{
			// 방향 전환
			D3DXVECTOR3 vDirection = pChar->GetPosition () - m_vPos;
			D3DXVec3Normalize ( &m_vDir, &vDirection );
			m_vDir = vDirection;

			ReSetAllSTATE ();
			SetSTATE ( EM_PETACT_ATTACK );
			m_pSkinChar->SELECTANI ( AN_ATTACK, AN_SUB_NONE );
		}
	}
	else if ( pNetMsg->sTarID.emCrow == CROW_MOB )
	{
		GLLandManClient* pLandMan = GLGaeaClient::GetInstance().GetActiveMap ();
		PGLCROWCLIENT pCrow = pLandMan->GetCrow ( pNetMsg->sTarID.dwID );
		if ( pCrow )
		{
			// 방향 전환
			D3DXVECTOR3 vDirection = pCrow->GetPosition () - m_vPos;
			D3DXVec3Normalize ( &m_vDir, &vDirection );
			m_vDir = vDirection;

			ReSetAllSTATE ();
			SetSTATE ( EM_PETACT_ATTACK );
			m_pSkinChar->SELECTANI ( AN_ATTACK, AN_SUB_NONE );
		}
	}
	else if ( pNetMsg->sTarID.emCrow == CROW_SUMMON )
	{
		GLLandManClient* pLandMan = GLGaeaClient::GetInstance().GetActiveMap ();
		PGLANYSUMMON pSummon = pLandMan->GetSummon ( pNetMsg->sTarID.dwID );
		if ( pSummon )
		{
			// 방향 전환
			D3DXVECTOR3 vDirection = pSummon->GetPosition () - m_vPos;
			D3DXVec3Normalize ( &m_vDir, &vDirection );
			m_vDir = vDirection;

			ReSetAllSTATE ();
			SetSTATE ( EM_PETACT_ATTACK );
			m_pSkinChar->SELECTANI ( AN_ATTACK, AN_SUB_NONE );
		}
	}

	// 타이머 초기화
	m_fAttackTimer = 0.0f;
}

void GLPetClient::MsgSad ( NET_MSG_GENERIC* nmg )
{
	ReSetAllSTATE ();
	SetSTATE ( EM_PETACT_SAD );
	
	// SAD 애니메이션
	if( IsUsePetSkinPack() && m_sPetSkinPackState.bNotFoundSad )
	{
		m_pSkinChar->SELECTANI ( AN_GUARD_N, AN_SUB_NONE );
	}else{
		m_pSkinChar->SELECTANI ( AN_GESTURE, AN_SUB_04 );
	}
	
	// 방향 전환
	D3DXVECTOR3 vDirection = m_pOwner->GetPosition () - m_vPos;
	D3DXVec3Normalize ( &m_vDir, &vDirection );
	m_vDir = vDirection;
}

void GLPetClient::MsgAccessoryDelete ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_ACCESSORY_DELETE* pNetMsg = (GLMSG::SNETPET_ACCESSORY_DELETE*) nmg;
	m_PutOnItems[pNetMsg->accetype] = SITEMCUSTOM ();
	UpdateSuit ( FALSE );
}

/*dual pet skill, Juver, 2017/12/27 */
void GLPetClient::MsgRemoveSkillFact_A ( NET_MSG_GENERIC* nmg )
{
	// 이펙트 제거
	FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_pOwner->m_dwGaeaID,m_pOwner->GetPosition() ), m_pOwner->GetSkinChar (), m_sActiveSkillID_A );

	m_pOwner->m_sPETSKILLFACT_A.RESET ();
	m_sActiveSkillID_A = NATIVEID_NULL();
	m_bSkillProcessing_A = false;
}

/*dual pet skill, Juver, 2017/12/27 */
void GLPetClient::MsgRemoveSkillFact_B ( NET_MSG_GENERIC* nmg )
{
	// 이펙트 제거
	FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_pOwner->m_dwGaeaID,m_pOwner->GetPosition() ), m_pOwner->GetSkinChar (), m_sActiveSkillID_B );

	m_pOwner->m_sPETSKILLFACT_B.RESET ();
	m_sActiveSkillID_B = NATIVEID_NULL();
	m_bSkillProcessing_B = false;
}

void GLPetClient::MsgProcess ( NET_MSG_GENERIC* nmg )
{
	switch ( nmg->nType )
	{
		/*dual pet skill, Juver, 2017/12/28 */
	case NET_MSG_PET_GETRIGHTOFITEM_A_FB:						MsgGetRightOfItem_A ( nmg );
		break;

		/*dual pet skill, Juver, 2017/12/28 */
	case NET_MSG_PET_GETRIGHTOFITEM_B_FB:						MsgGetRightOfItem_B ( nmg );
		break;

		/*dual pet skill, Juver, 2017/12/27 */
	case NET_MSG_PET_REQ_SKILLCHANGE_A_FB:						MsgSkillChange_A ( nmg );
		break;

		/*dual pet skill, Juver, 2017/12/27 */
	case NET_MSG_PET_REQ_SKILLCHANGE_B_FB:						MsgSkillChange_B ( nmg );
		break;

	case NET_MSG_PET_REQ_RENAME_FB:								MsgRename ( nmg );
		break;

	case NET_MSG_PET_UPDATECLIENT_FULL:							MsgUpdateFull ( nmg );
		break;

	case NET_MSG_PET_REQ_CHANGE_COLOR_FB:						MsgChangeColor ( nmg );
		break;

	case NET_MSG_PET_REQ_CHANGE_STYLE_FB:						MsgChangeStyle ( nmg );
		break;

	case NET_MSG_PET_REQ_SLOT_EX_HOLD_FB:						MsgSlotExHold ( nmg );
		break;

	case NET_MSG_PET_REQ_HOLD_TO_SLOT_FB:						MsgHoldToSlot ( nmg );
		break;

	case NET_MSG_PET_REQ_SLOT_TO_HOLD_FB:						MsgSlotToHold ( nmg );
		break;

		/*dual pet skill, Juver, 2017/12/27 */
	case NET_MSG_PET_ADD_SKILLFACT_A:							MsgSkillChange_A ( nmg );
		break;

		/*dual pet skill, Juver, 2017/12/27 */
	case NET_MSG_PET_ADD_SKILLFACT_B:							MsgSkillChange_B ( nmg );
		break;

		/*dual pet skill, Juver, 2017/12/27 */
	case NET_MSG_PET_REMOVE_SKILLFACT_A:						MsgRemoveSkillFact_A ( nmg );
		break;

		/*dual pet skill, Juver, 2017/12/27 */
	case NET_MSG_PET_REMOVE_SKILLFACT_B:						MsgRemoveSkillFact_B ( nmg );
		break;

	case NET_MSG_PET_REQ_LEARNSKILL_FB:							MsgLearnSkill ( nmg );
		break;

	case NET_MSG_PET_REMOVE_SLOTITEM_FB:						MsgRemoveSlotItem ( nmg );
		break;

	case NET_MSG_PET_ATTACK:									MsgAttack ( nmg );
		break;

	case NET_MSG_PET_SAD:										MsgSad ( nmg );
		break;

	case NET_MSG_PET_ACCESSORY_DELETE:							MsgAccessoryDelete ( nmg );
		break;

	default:
		CDebugSet::ToListView ( "[WARNING]GLPetField::MsgProcess Illigal Message(%d)", nmg->nType );
	  	break;
	};
}