#include "stdafx.h"
#include "./GLCharClient.h"
#include "./GLFactEffect.h"
#include "./GLGaeaClient.h"
#include "./GLItemMan.h"
#include "./GLStrikeM.h"

#include "../Lib_ClientUI/Interface/InnerInterface.h"
#include "../Lib_ClientUI/Interface/DamageDisplay.h"

#include "../../Lib_Engine/DxCommon/DxShadowMap.h"
#include "../../Lib_Engine/DxEffect/Char/DxEffCharData.h"
#include "../../Lib_Engine/DxEffect/Single/DxEffGroupPlayer.h"
#include "../Lib_ClientUI/Interface/ItemShopIconMan.h"

/*ABL system, Juver, 2017/06/02 */
#include "GLStrikeSelector.h"

/*bike color , Juver, 2017/11/15 */
#include "GLBikeColorDef.h"

#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GLCharClient::GLCharClient(void) 
	: m_pd3dDevice(NULL)
	, m_pSkinChar(NULL)

	, m_vDir(0,0,-1)
	, m_vDirOrig(0,0,-1)
	, m_vPos(0,0,0)
	, m_vServerPos(0,0,0)
	, m_fAge(0.0f)
	, m_Action(GLAT_IDLE)
	, m_dwActState(NULL)
	, m_vTarPos(0,0,0)
	, m_fIdleTime(0.0f)
	, m_fMoveDelay(0.0f)
	
	, m_dwANISUBSELECT(0)
	, m_dwANISUBGESTURE(0)

	, m_vMaxOrg(6,20,6)
	, m_vMinOrg(-6,0,-6)
	
	, m_vMax(6,20,6)
	, m_vMin(-6,0,-6)
	, m_fHeight(20.f)
	
	, m_CHARINDEX(GLCI_BRAWLER_M)
	, m_dwGaeaID(0)
	, m_dwCeID(0)
	
	, m_pLandManClient(NULL)
	, m_pGlobNode(NULL)
	, m_pQuadNode(NULL)
	, m_pCellNode(NULL)
	
	, m_emANIMAINSKILL(AN_SKILL_A)
	, m_emANISUBSKILL(AN_SUB_NONE)
	
	, m_emANISUBTYPE(AN_SUB_NONE)
	, m_dwRevData(NULL)
	
	, m_fSTATE_MOVE(0.0f)
	, m_fSTATE_DELAY(0.0f)
	
	, m_bSTATE_PANT(false)
	, m_bSTATE_STUN(false)
	
	, m_fSKILL_MOVE(0.0f)
	, m_fITEM_MOVE_R(0.0f)
	, m_fITEM_MOVE (0.0f)
	, m_fATTVELO( 0.0f )
	, m_fITEMATTVELO_R ( 0.0f ) 
	, m_fITEMATTVELO ( 0.0f ) 
	//, m_dwSummonGUID ( GAEAID_NULL )
	
	, m_wTARNUM(0)
	, m_vTARPOS(0,0,0)
	, m_wACTIVESKILL_LEVEL(0)
	
	, m_fattTIMER(0.0f)
	, m_nattSTEP(0)
	, m_bVehicle ( FALSE )
	, m_bItemShopOpen ( false )
	, m_dwTransfromSkill(SNATIVEID::ID_NULL)
	, m_bSafeZone(false)
	, m_wActionAnim(AN_SUB_00_SIZE) /*dash skill logic, Juver, 2017/06/17 */
{
	D3DXMatrixIdentity( &m_matTrans );
	DISABLEALLLANDEFF();

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		m_dwSummonGUID[i] = GAEAID_NULL;

	memset ( m_bActionType, false, sizeof( bool ) * SKILL::EMACTION_NSIZE );
}

GLCharClient::~GLCharClient(void)
{
	FACTEFF::DeleteAllEffect( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT, m_sSTATEBLOWS );
	SAFE_DELETE(m_pSkinChar);

	if ( m_bItemShopOpen )
	{
		CInnerInterface::GetInstance().GetItemShopIconMan()->DEL_SHOP_ICON( m_dwGaeaID );
	}

}

BOOL GLCharClient::IsValidBody ()
{
	return ( m_Action < GLAT_FALLING );
}

BOOL GLCharClient::IsDie ()
{
	if ( IsACTION(GLAT_DIE) )		return TRUE;

	return FALSE;
}

BOOL GLCharClient::IsVisibleDetect ()
{
	if ( IsSTATE(EM_REQ_VISIBLENONE) || IsSTATE(EM_REQ_VISIBLEOFF) )	return FALSE;

	BOOL bRECVISIBLE = GLGaeaClient::GetInstance().GetCharacter()->m_bRECVISIBLE;
	if ( m_bINVISIBLE && !bRECVISIBLE )									return FALSE;

	return TRUE;
}

HRESULT GLCharClient::Create ( GLLandManClient* pLandManClient, SDROP_CHAR* pCharData, LPDIRECT3DDEVICEQ pd3dDevice )
{
	m_pd3dDevice = pd3dDevice;
	m_pLandManClient = pLandManClient;
	if ( pCharData )	m_CharData = *pCharData;

	m_bVehicle = pCharData->m_bVehicle;
	m_sVehicle.m_dwGUID = pCharData->m_sVehicle.m_dwGUID;
	m_sVehicle.m_emTYPE = pCharData->m_sVehicle.m_emTYPE;
	m_sVehicle.m_sVehicleID = pCharData->m_sVehicle.m_sVehicleID;

	m_bItemShopOpen = pCharData->m_bItemShopOpen;

	for ( int i = 0; i < VEHICLE_ACCETYPE_SIZE; ++i )
	{
		m_sVehicle.m_PutOnItems[i].Assign( pCharData->m_sVehicle.m_PutOnItems[i] );
	}

	/*bike color , Juver, 2017/11/13 */
	for( WORD i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
		m_sVehicle.m_wColor[i] = pCharData->m_sVehicle.m_wColor[i];

	m_sVehicle.ITEM_UPDATE();

	//	Note : 현제 스킬 FACT 업데이트.
	//
	for ( int i=0; i<SKILLFACT_SIZE; ++i )
	{
		SDROP_SKILLFACT &sDropFact = pCharData->sSKILLFACT[i];
		if ( sDropFact.sNATIVEID==NATIVEID_NULL() )			continue;

		RECEIVE_SKILLFACT ( sDropFact.sNATIVEID, sDropFact.wLEVEL, sDropFact.wSLOT, sDropFact._wCasterCrow, sDropFact._dwCasterID, sDropFact.fAGE );
	}

	/*itemfood system, Juver, 2017/05/25 */
	for ( int i=0; i<FITEMFACT_SIZE; ++i )
	{
		SDROP_FITEMFACT &sDropFact = pCharData->sFITEMFACT[i];
		if ( sDropFact.sNATIVEID==NATIVEID_NULL() )			continue;
		ItemFoodAdd ( sDropFact.sNATIVEID, sDropFact.wLEVEL, sDropFact.wSLOT, sDropFact.fAGE );
	}
	
	/*system buffs, Juver, 2017/09/04 */
	for ( int i=0; i<SYSTEM_BUFF_SIZE; ++i )
	{
		SDROP_SYSTEM_BUFF &sDropFact = pCharData->sSYSTEM_BUFF[i];
		SystemBuffAdd ( sDropFact.sNATIVEID, sDropFact.wLEVEL, sDropFact.wSLOT );
	}

	//	Note : 현제 상태이상 업데이트.
	//
	for ( int i=0; i<EMBLOW_MULTI; ++i )
	{
		SDROP_STATEBLOW &sDropBlow = pCharData->sSTATEBLOWS[i];
		if ( sDropBlow.emBLOW==EMBLOW_NONE )				continue;

		SSTATEBLOW *pSTATEBLOW = NULL;
		if ( sDropBlow.emBLOW <= EMBLOW_SINGLE )		pSTATEBLOW = &m_sSTATEBLOWS[0];
		else											pSTATEBLOW = &m_sSTATEBLOWS[sDropBlow.emBLOW-EMBLOW_SINGLE];

		sDropBlow.emBLOW = sDropBlow.emBLOW;
		sDropBlow.fAGE = sDropBlow.fAGE;
		sDropBlow.fSTATE_VAR1 = sDropBlow.fSTATE_VAR1;
		sDropBlow.fSTATE_VAR2 = sDropBlow.fSTATE_VAR2;
	}

	DISABLEALLLANDEFF();
	SLEVEL_ETC_FUNC *pEtcFunc = m_pLandManClient->GetLevelEtcFunc();
	if( pEtcFunc && pEtcFunc->m_bUseFunction[EMETCFUNC_LANDEFFECT] )
	{
		int iNum = 0;
		for( int i=0; i<EMLANDEFFECT_MULTI; i++ )
		{
			if( pCharData->nLandEffect[i] == -1 ) continue;

			SLANDEFFECT landEffect = pEtcFunc->FindLandEffect(pCharData->nLandEffect[i]);
			ADDLANDEFF( landEffect, iNum );
			iNum++;
		}
	}

	//	Note : 케릭터의 외형을 설정.
	EMCHARINDEX emIndex = CharClassToIndex(m_CharData.emClass);
	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().LoadData( GLCONST_CHAR::szCharSkin[emIndex], pd3dDevice, TRUE );
	if ( !pSkinChar )	return E_FAIL;

	SAFE_DELETE(m_pSkinChar);
	m_pSkinChar = new DxSkinChar;
	m_pSkinChar->SetCharData ( pSkinChar, pd3dDevice, TRUE );
	UpdateSuit ();
	UPDATE_ITEM();

	// Note : 1.AABB Box를 가져온다. 2.높이를 계산해 놓는다.
	m_pSkinChar->GetAABBBox( m_vMaxOrg, m_vMinOrg );
	m_fHeight = m_vMaxOrg.y - m_vMinOrg.y;

	//	Note : 에니메이션 초기화.
	//
	ReSelectAnimation ();

	//	Note : 이동 제어 초기화.
	m_strName = m_CharData.szName;
	m_dwGaeaID = m_CharData.dwGaeaID;
	m_dwCeID = m_CharData.dwCeID;
	m_vPos = m_CharData.vPos;
	m_vDir = m_CharData.vDir;

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		m_dwSummonGUID[i] = m_CharData.dwSummonGUID[i];

	m_CHARINDEX = CharClassToIndex(m_CharData.emClass);

	m_actorMove.SetMaxSpeed ( GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX].fWALKVELO );
	if ( pLandManClient )
	{
		m_actorMove.Create ( pLandManClient->GetNaviMesh(), m_vPos, -1 );
	}

	if ( pCharData->dwActState & EM_ACT_RUN )			SetSTATE ( EM_ACT_RUN );
	if ( pCharData->dwActState & EM_REQ_VISIBLENONE )	SetSTATE ( EM_REQ_VISIBLENONE );
	if ( pCharData->dwActState & EM_REQ_VISIBLEOFF )	SetSTATE ( EM_REQ_VISIBLEOFF );
	if ( pCharData->dwActState & EM_ACT_PEACEMODE )		SetSTATE ( EM_ACT_PEACEMODE );

	/*vehicle booster system, Juver, 2017/08/12 */
	if ( pCharData->dwActState & EM_ACT_VEHICLE_BOOSTER )		
		SetSTATE ( EM_ACT_VEHICLE_BOOSTER );

	switch ( pCharData->Action )
	{
	case GLAT_MOVE:
		{
			//	Note : 이동 시키기 위해 내부 전달 메시지 발생.
			//
			GLMSG::SNETPC_GOTO_BRD NetMsgGoto;
			NetMsgGoto.dwActState = pCharData->dwActState;
			NetMsgGoto.fDelay = 0.0f;
			NetMsgGoto.vCurPos = pCharData->vPos;
			NetMsgGoto.vTarPos = pCharData->vTarPos;
			MsgGoto ( (NET_MSG_GENERIC*) &NetMsgGoto );
		}
		break;

	case GLAT_FALLING:
		TurnAction ( GLAT_FALLING );
		break;

	case GLAT_DIE:
		TurnAction ( GLAT_DIE );
		break;
	case GLAT_GATHERING:
		m_dwANISUBGESTURE = m_CharData.m_dwANISUBTYPE;
		TurnAction( GLAT_GATHERING );
		break;
	case GLAT_TALK:
		m_dwANISUBGESTURE = m_CharData.m_dwANISUBTYPE;
		TurnAction ( GLAT_TALK );
		break;
	};

	if ( pCharData->dwFLAGS & SDROP_CHAR::CHAR_GEN )
	{
		if ( !IsSTATE(EM_REQ_VISIBLENONE) && !IsSTATE(EM_REQ_VISIBLEOFF) )
		{
			D3DXMATRIX matEffect;
			D3DXMatrixTranslation ( &matEffect, m_vPos.x, m_vPos.y, m_vPos.z );

			STARGETID sTargetID(CROW_PC,pCharData->dwGaeaID,m_vPos);
			DxEffGroupPlayer::GetInstance().NewEffGroup
			(
			GLCONST_CHAR::strREBIRTH_EFFECT.c_str(),
				matEffect,
				&sTargetID
			);
		}
	}

	if ( !m_pSkinChar->GETCURANIMNODE() )
	{
		CDebugSet::ToLogFile ( "ERORR : current animation node null point error [ %s ] [ M %d S %d ]", m_pSkinChar->GetFileName(), m_pSkinChar->GETCURMTYPE(), m_pSkinChar->GETCURSTYPE() );
		m_pSkinChar->DEFAULTANI ();
	}

	//	Note : 클럽마크 버전 확인.
	//
	GLGaeaClient::GetInstance().GetCharacter()->ReqClubMarkInfo ( m_CharData.dwGuild, m_CharData.dwGuildMarkVer );

	/*if ( m_bItemShopOpen )
	{
		CInnerInterface::GetInstance().GetItemShopIconMan()->ADD_SHOP_ICON( m_dwGaeaID );
	}*/

	/*pvp capture the flag, Juver, 2018/02/07 */
	D3DXMATRIX matEffect;
	D3DXMatrixTranslation ( &matEffect, m_vPos.x, m_vPos.y, m_vPos.z );

	for ( int i=0; i< CAPTURE_THE_FLAG_TEAM_SIZE; ++ i )
		DxEffGroupPlayer::GetInstance().DeletePassiveEffect( GLCONST_CHAR::strCaptureTheFlagEffect[i].c_str(), STARGETID( CROW_PC, m_dwGaeaID, m_vPos ) );

	if ( m_CharData.m_wCaptureTheFlagTeam < CAPTURE_THE_FLAG_TEAM_SIZE && m_CharData.m_bCaptureTheFlagHoldFlag )
	{
		if ( m_CharData.m_wCaptureTheFlagTeam == CAPTURE_THE_FLAG_TEAM_A )
			DxEffGroupPlayer::GetInstance().PassiveEffect( GLCONST_CHAR::strCaptureTheFlagEffect[CAPTURE_THE_FLAG_TEAM_B].c_str(), matEffect, STARGETID( CROW_PC, m_dwGaeaID, m_vPos ) );
		else if ( m_CharData.m_wCaptureTheFlagTeam == CAPTURE_THE_FLAG_TEAM_B )
			DxEffGroupPlayer::GetInstance().PassiveEffect( GLCONST_CHAR::strCaptureTheFlagEffect[CAPTURE_THE_FLAG_TEAM_A].c_str(), matEffect, STARGETID( CROW_PC, m_dwGaeaID, m_vPos ) );
	}

	return S_OK;
}

DWORD GLCharClient::GET_PK_COLOR ()
{
	if ( m_CharData.nBright >= 0 )	return GLCONST_CHAR::dwPK_NORMAL_NAME_COLOR;

	int nLEVEL = 0;
	for ( nLEVEL=0; nLEVEL<GLCONST_CHAR::EMPK_STATE_LEVEL; ++nLEVEL )
	{
		if ( GLCONST_CHAR::sPK_STATE[nLEVEL].nPKPOINT <= m_CharData.nBright )	break;
	}

	if ( nLEVEL>=GLCONST_CHAR::EMPK_STATE_LEVEL )	nLEVEL = GLCONST_CHAR::EMPK_STATE_LEVEL-1;

	return GLCONST_CHAR::sPK_STATE[nLEVEL].dwNAME_COLOR;
}

SITEM* GLCharClient::GET_SLOT_ITEMDATA ( EMSLOT _slot )
{
	if ( VALID_SLOT_ITEM(_slot) )
	{
		const SITEMCLIENT &sITEMCLIENT = GET_SLOT_ITEM ( _slot );

		SITEM* pITEM = GLItemMan::GetInstance().GetItem ( sITEMCLIENT.sNativeID );
		return pITEM;
	}

	return NULL;
}

HRESULT GLCharClient::UpdateSuit ()
{
	GASSERT(m_pd3dDevice);

	EMCHARINDEX emIndex = CharClassToIndex(m_CharData.emClass);
	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().FindData ( GLCONST_CHAR::szCharSkin[emIndex] );
	if ( !pSkinChar )	return E_FAIL;

	const GLCONST_CHARCLASS &sCONST = GLCONST_CHAR::cCONSTCLASS[emIndex];

	if ( sCONST.dwHEADNUM > m_CharData.wFace )
	{
		std::string strHEAD_CPS = sCONST.strHEAD_CPS[m_CharData.wFace];
	
		PDXCHARPART pCharPart = NULL;
		pCharPart = m_pSkinChar->GetPiece(PIECE_HEAD);		//	현제 장착 스킨.

		if ( pCharPart && strcmp(strHEAD_CPS.c_str(),pCharPart->GetFileName()) )
		{
			m_pSkinChar->SetPiece ( strHEAD_CPS.c_str(), m_pd3dDevice, 0x0, 0, TRUE );
		}
	}

	if ( sCONST.dwHAIRNUM > m_CharData.wHair )
	{
		std::string strHAIR_CPS = sCONST.strHAIR_CPS[m_CharData.wHair];
	
		PDXCHARPART pCharPart = NULL;
		pCharPart = m_pSkinChar->GetPiece(PIECE_HAIR);		//	현제 장착 스킨.

		if ( pCharPart && strcmp(strHAIR_CPS.c_str(),pCharPart->GetFileName()) )
		{
			m_pSkinChar->SetPiece ( strHAIR_CPS.c_str(), m_pd3dDevice, 0x0, 0, TRUE );
		}
	}

	m_pSkinChar->SetHairColor( m_CharData.wHairColor );

	for ( int i=0; i<SLOT_NSIZE_S_2; i++ )
	{
		// 현재 장착중인 무기가 아니면 넘어간다.
		if( !IsCurUseArm( EMSLOT(i) ) ) continue;

		SITEMCLIENT ItemClient = m_CharData.m_PutOnItems[i];

		/*ABL system, Juver, 2017/06/02 */
		if ( i == SLOT_LHAND  )	ItemClient = m_CharData.m_PutOnItems[SLOT_RHAND];
		if ( i == SLOT_LHAND_S  )	ItemClient = m_CharData.m_PutOnItems[SLOT_RHAND_S];

		SNATIVEID nidITEM = ItemClient.nidDISGUISE;
		if ( nidITEM==SNATIVEID(false) )	nidITEM = ItemClient.sNativeID;

		if ( !m_bVehicle && i == SLOT_VEHICLE  ) nidITEM = SNATIVEID(false);
		else if ( i == SLOT_VEHICLE )
		{
			nidITEM = m_sVehicle.GetSkinID();
			if ( nidITEM == SNATIVEID(false) )
			{
				m_sVehicle.m_sVehicleID = ItemClient.sNativeID;
				nidITEM = ItemClient.sNativeID;
			}
		}

		if ( nidITEM == SNATIVEID(false) )
		{
			//	Note : 기본 스킨과 지금 장착된 스킨이 틀릴 경우.
			//	SLOT->PIECE.
			PDXSKINPIECE pSkinPiece = NULL;	//	기본 스킨.
			PDXCHARPART pCharPart = NULL;	//	현제 장착 스킨.

			/*ABL system, Juver, 2017/06/02 */
			DxAttBoneLink* pBoneLink = NULL;
			DxAttBone* pAttBone = NULL;

			EMPIECECHAR emPiece = SLOT_2_PIECE(EMSLOT(i));
			if ( emPiece!=PIECE_SIZE )
			{
				pSkinPiece = pSkinChar->GetPiece(emPiece);	//	기본 스킨.
				pCharPart = m_pSkinChar->GetPiece(emPiece);	//	현제 장착 스킨.

				/*ABL system, Juver, 2017/06/02 */
				pBoneLink = pSkinChar->GetAttBone(emPiece);
				pAttBone = m_pSkinChar->GetAttBone(emPiece);
			}

			/*ABL system, Juver, 2017/06/02 */
			if ( pBoneLink )
			{
				if ( pAttBone && strcmp( pBoneLink->GetFileName(), pAttBone->GetFileName() ) )
				{
					m_pSkinChar->SetAttBone( pBoneLink, m_pd3dDevice, 0X0, 0, TRUE );
				}
				else if ( !pAttBone )
				{
					m_pSkinChar->SetAttBone( pBoneLink, m_pd3dDevice, 0X0, 0, TRUE );
				}
			}
			else if ( pSkinPiece )
			{
				if ( pCharPart && strcmp(pSkinPiece->m_szFileName,pCharPart->GetFileName()) )
					m_pSkinChar->SetPiece ( pSkinPiece->m_szFileName, m_pd3dDevice, 0x0, 0, TRUE );
			}
			else
			{
				if ( pCharPart )	m_pSkinChar->ResetPiece(emPiece);
			}

			/*upgrade effect, Juver, 2017/09/01 */
			EMPIECECHAR piece_reset = SLOT_2_UPGRADEPIECE( EMSLOT(i) );
			if ( piece_reset != PIECE_SIZE )	
				m_pSkinChar->ResetPiece( piece_reset );
		}
		else
		{
			SITEM* pItem = GLItemMan::GetInstance().GetItem ( nidITEM );
			if ( pItem )
			{
				/*character simple, Juver, 2017/10/01 */
				BOOL bCharacterSimple = RANPARAM::bCharacterSimple;
				BOOL bCharacterMisc = RANPARAM::bCharacterMisc;

				/*Item Self Effect, Juver, 2017/06/09 */
				std::string strSelfEffect = pItem->GetSelfBodyEffect();

				if ( pItem->sBasicOp.emItemType != ITEM_VEHICLE )
				{
					/*ABL system, Juver, 2017/06/02 */
					std::string strFileName = pItem->GetWearingFileR( emIndex );
					if ( i == SLOT_LHAND || i == SLOT_LHAND_S )	strFileName = pItem->GetWearingFileL( emIndex );

					if ( bCharacterMisc )
					{
						if ( i == SLOT_NECK || i == SLOT_WRIST || i == SLOT_LEARRING || i == SLOT_WAIST || i == SLOT_ORNAMENT ) 
						{
							EMPIECECHAR emPiece = SLOT_2_PIECE(EMSLOT(i));
							PDXCHARPART pCharPart = NULL;	//	현제 장착 스킨.
							if ( emPiece!=PIECE_SIZE )
							{
								pCharPart = m_pSkinChar->GetPiece(emPiece);	//	현제 장착 스킨.
							}
							if ( pCharPart )	m_pSkinChar->ResetPiece(emPiece);
							continue;
						}
					}

					/*character simple, Juver, 2017/10/01 */
					if ( bCharacterSimple  )
					{
						if ( i < SLOT_WEAR_SIMPLE )
						{
							EMCHARGENDER char_gender = CharIndexToGender( emIndex );
							if ( char_gender == GLGENDER_W )
								strFileName = GLCONST_CHAR::strSimpleWearF[i];
							else if ( char_gender == GLGENDER_M )
								strFileName = GLCONST_CHAR::strSimpleWearM[i];
						}

						if ( i == SLOT_FACE )
						{
							EMPIECECHAR emPiece = SLOT_2_PIECE(EMSLOT(i));
							PDXCHARPART pCharPart = NULL;	//	현제 장착 스킨.
							if ( emPiece!=PIECE_SIZE )
							{
								pCharPart = m_pSkinChar->GetPiece(emPiece);	//	현제 장착 스킨.
							}
							if ( pCharPart )	m_pSkinChar->ResetPiece(emPiece);
							continue;
						}
						

						if ( i == SLOT_RHAND || i ==  SLOT_RHAND_S || i == SLOT_LHAND || i ==  SLOT_LHAND_S ) 
						{
							EMPIECECHAR emPiece = SLOT_2_PIECE(EMSLOT(i));
							PDXCHARPART pCharPart = NULL;	//	현제 장착 스킨.
							if ( emPiece!=PIECE_SIZE )
							{
								pCharPart = m_pSkinChar->GetPiece(emPiece);	//	현제 장착 스킨.
							}
							if ( pCharPart )	m_pSkinChar->ResetPiece(emPiece);
							continue;
						}


						if ( i == SLOT_MISC ) 
						{
							EMPIECECHAR emPiece = SLOT_2_PIECE(EMSLOT(i));
							PDXCHARPART pCharPart = NULL;	//	현제 장착 스킨.
							if ( emPiece!=PIECE_SIZE )
							{
								pCharPart = m_pSkinChar->GetPiece(emPiece);	//	현제 장착 스킨.
							}
							if ( pCharPart )	m_pSkinChar->ResetPiece(emPiece);
							continue;
						}
					}
					
					DxAttBoneLink* pBoneLink = DxAttBoneLinkContainer::GetInstance().LoadData( strFileName.c_str(), m_pd3dDevice, TRUE );
					if ( pBoneLink )	m_pSkinChar->SetAttBone( pBoneLink, m_pd3dDevice, NULL, ItemClient.GETGRADE_EFFECT(), TRUE, strSelfEffect.c_str() );
					else	m_pSkinChar->SetPiece ( strFileName.c_str(), m_pd3dDevice, NULL, ItemClient.GETGRADE_EFFECT(), TRUE, strSelfEffect.c_str() );
				
					/*item color, Juver, 2018/01/10 */
					EMPIECECHAR piece_color = SLOT_2_PIECE(EMSLOT(i));
					if ( piece_color != PIECE_SIZE )
					{
						m_pSkinChar->SetColor1( piece_color, ItemClient.wColor1 );
						m_pSkinChar->SetColor2( piece_color, ItemClient.wColor2 );
					}
					
					/*upgrade effect, Juver, 2017/09/01 */
					/*character simple, Juver, 2017/10/01 */
					BOOL bResetUpgradeEffect = TRUE;
					if( !bCharacterSimple && ItemClient.GETGRADE_EFFECT() >= 6 && ( i == SLOT_UPPER || i == SLOT_LOWER || i == SLOT_HAND || i == SLOT_FOOT ) )
					{
						EMCHARGENDER emGender = CharClassToGender( m_CharData.emClass );
						std::string strPartName[4] = { "body", "leg", "hand", "foot" };
						std::string strGender[2] = { "w", "m" };

						CString strUpgradeFileName;
						strUpgradeFileName.Format("%s_bs_%s_enchent+%d.cps", strGender[emGender].c_str(), strPartName[i-1].c_str(), ItemClient.GETGRADE_EFFECT() );
						HRESULT hrSetUpgrade = m_pSkinChar->SetPiece ( strUpgradeFileName.GetString(), m_pd3dDevice, NULL, ItemClient.GETGRADE_EFFECT(), TRUE );
						if ( hrSetUpgrade == S_OK )	bResetUpgradeEffect = FALSE;
					}

					if ( bResetUpgradeEffect )
					{
						EMPIECECHAR piece_reset = SLOT_2_UPGRADEPIECE( EMSLOT(i) );
						if ( piece_reset != PIECE_SIZE )	
							m_pSkinChar->ResetPiece( piece_reset );
					}
				}
				else
				{
					/*vehicle system, Juver, 2017/08/07 */
					DxVehicleData*	pVehicleData = DxVehicleDataContainer::GetInstance().LoadData( pItem->GetWearingFileR(emIndex), m_pd3dDevice, TRUE );
					if ( pVehicleData )	
					{
						m_pSkinChar->SetVehicle( pVehicleData, m_pd3dDevice, NULL, ItemClient.GETGRADE_EFFECT(), TRUE, strSelfEffect.c_str() );

						/*character simple, Juver, 2017/10/01 */
						if ( !bCharacterSimple && m_pSkinChar->m_pVehicle )
						{
							/* vehicle no accessory, Juver, 2018/02/14 */
							if ( !pItem->sVehicle.bNoAcc )
							{	
								for( int j=0; j<VEHICLE_ACCETYPE_SIZE; j++ )
								{
									SITEM* pVehicleItem = GLItemMan::GetInstance().GetItem ( m_sVehicle.m_PutOnItems[j].sNativeID );
									if ( pVehicleItem )
										m_pSkinChar->m_pVehicle->SetPiece( pVehicleItem->GetVehicleWearingFile( pItem->sVehicle.emVehicleSubType), m_pd3dDevice, NULL, ItemClient.GETGRADE_EFFECT(), TRUE );
								}
							}

							/*bike color , Juver, 2017/11/15 */
							if ( pItem->sVehicle.emVehicleType == VEHICLE_TYPE_BIKE )
							{
								for ( int ncolor=0; ncolor<BIKE_COLOR_SLOT_PART_SIZE; ++ncolor )
								{
									EMPIECECHAR piece_color = BIKE_COLOR_SLOT_TO_PIECE( (BIKE_COLOR_SLOT_PART) ncolor );
									BOOL bColor1 = IS_BIKECOLOR_1( (BIKE_COLOR_SLOT_PART) ncolor );
									if ( bColor1 )
										m_pSkinChar->m_pVehicle->SetColor1( piece_color, m_sVehicle.m_wColor[ncolor] );
									else
										m_pSkinChar->m_pVehicle->SetColor2( piece_color, m_sVehicle.m_wColor[ncolor] );
								}
							}

							/* car, cart color, Juver, 2018/02/14 */
							if ( pItem->sVehicle.emVehicleType == VEHICLE_TYPE_CAR || pItem->sVehicle.emVehicleType == VEHICLE_TYPE_CART )
							{
								m_pSkinChar->m_pVehicle->SetColor1( PIECE_SIZE, m_sVehicle.m_wColor[BIKE_COLOR_SLOT_PART_A1] );
								m_pSkinChar->m_pVehicle->SetColor2( PIECE_SIZE, m_sVehicle.m_wColor[BIKE_COLOR_SLOT_PART_A2] );
							}
						}
					}
					else
						m_pSkinChar->SetPiece ( pItem->GetWearingFileR(emIndex), m_pd3dDevice, NULL, ItemClient.GETGRADE_EFFECT(), TRUE, strSelfEffect.c_str() );
				}
			}
		}
	}

	if( m_dwTransfromSkill != SNATIVEID::ID_NULL )
	{
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_sSKILLFACT[m_dwTransfromSkill].sNATIVEID.wMainID, m_sSKILLFACT[m_dwTransfromSkill].sNATIVEID.wSubID );
		if( pSkill ) 
		{
			for( BYTE i = 0; i < SKILL::EMTRANSFORM_NSIZE; i++ )
			{
				if( m_CharData.wSex == 1 )
				{				
					m_pSkinChar->SetPiece ( pSkill->m_sSPECIAL_SKILL.strTransform_Man[i].c_str(), m_pd3dDevice, 0X0, 0, TRUE );
				}else{
					m_pSkinChar->SetPiece ( pSkill->m_sSPECIAL_SKILL.strTransform_Woman[i].c_str(), m_pd3dDevice, 0X0, 0, TRUE );
				}
			}
		}
	}


	//	Note : 스킬 버프, 상태이상 효과 생성.
	FACTEFF::ReNewEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT, m_sSTATEBLOWS, m_matTrans, m_vDir );

	if( GLCONST_CHAR::cCONSTCLASS[emIndex].strCLASS_EFFECT.size() != 0 )
	{
		DxEffcharDataMan::GetInstance().PutPassiveEffect ( m_pSkinChar, GLCONST_CHAR::cCONSTCLASS[emIndex].strCLASS_EFFECT.c_str(), &m_vDir );
	}

	return S_OK;
}

WORD GLCharClient::GetBodyRadius ()
{
	return GETBODYRADIUS();
}

void GLCharClient::DisableSkillFact()
{
	EMSLOT emRHand = GetCurRHand();	
	SITEM* pRightItem = GET_SLOT_ITEMDATA(emRHand);	
	
	for ( int i=0; i<SKILLFACT_SIZE; ++i )
	{
		if ( m_sSKILLFACT[i].sNATIVEID == NATIVEID_NULL() ) continue;
	
        PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_sSKILLFACT[i].sNATIVEID );
		if ( !pSkill ) continue;

		// 스킬 자신 버프
		if ( pSkill->m_sBASIC.emIMPACT_TAR != TAR_SELF || pSkill->m_sBASIC.emIMPACT_REALM != REALM_SELF ) continue;
				
		const bool bExtremeWeapon	= pSkill->m_sLEARN.bHiddenWeapon;
		GLSKILL_ATT emSKILL_RITEM = pSkill->m_sBASIC.emUSE_RITEM;

		// 스킬 도구 종속 없음
		if ( emSKILL_RITEM == SKILLATT_NOCARE )	continue;

		// 스킬 과 무기가 불일치
		if( !pRightItem || !CHECHSKILL_ITEM(emSKILL_RITEM,pRightItem->sSuitOp.emAttack,bExtremeWeapon) )
		{
			FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT[i].sNATIVEID );
			DISABLESKEFF( i );
		}
	}
}


void GLCharClient::ReSelectAnimation ()
{
	EMSLOT emRHand = GetCurRHand();
	EMSLOT emLHand = GetCurLHand();

	SITEM* pRHAND = NULL;
	SITEM* pLHAND = NULL;

	if ( m_CharData.m_PutOnItems[emRHand].sNativeID!=NATIVEID_NULL() )
		pRHAND = GLItemMan::GetInstance().GetItem ( m_CharData.m_PutOnItems[emRHand].sNativeID );
	
	if ( m_CharData.m_PutOnItems[emLHand].sNativeID!=NATIVEID_NULL() )
		pLHAND = GLItemMan::GetInstance().GetItem ( m_CharData.m_PutOnItems[emLHand].sNativeID );

	m_emANISUBTYPE = CHECK_ANISUB ( pRHAND, pLHAND );

	// 제스처중이면 애니메이션 리셋 안함.
	if ( !IsACTION ( GLAT_TALK ) && !IsACTION( GLAT_GATHERING ) && !IsACTION( GLAT_SKILL ) )
	{
		//	Note : 에니메이션 초기화.
		//
		m_pSkinChar->SELECTANI ( m_pSkinChar->GETCURMTYPE(), m_emANISUBTYPE );
	}
}

void GLCharClient::SetPosition ( const D3DXVECTOR3 &vPos )
{
	m_vPos = vPos;
	m_actorMove.SetPosition ( m_vPos, -1 );
	if ( m_actorMove.PathIsActive() )		m_actorMove.Stop();
}

void GLCharClient::TurnAction ( EMACTIONTYPE toAction )
{
	//	Note : 이전 액션 취소.
	//
	switch ( m_Action )
	{
	case GLAT_IDLE:
		break;

	case GLAT_SKILL:
		ResetHiddenWeaponPiece();
		break;

	case GLAT_FALLING:
		if ( toAction==GLAT_FALLING )	return;
		break;
		
	case GLAT_DIE:
		if ( toAction==GLAT_FALLING || toAction==GLAT_DIE )	return;
		break;

	case GLAT_CONFT_END:
		ReSetSTATE(EM_ACT_CONFT_WIN);
		break;

	default:
		break;
	};

	//	Note : 액션 초기화.
	//
	m_Action = toAction;

	switch ( m_Action )
	{
	case GLAT_IDLE:
		{
			m_fIdleTime = 0.0f;

			/*dash skill logic, Juver, 2017/06/17 */
			m_wActionAnim = AN_SUB_00_SIZE;

			/*dmk14 visual bug fix*/
			if ( m_actorMove.PathIsActive() )	m_actorMove.Stop();
		}break;

	case GLAT_ATTACK:
		{
			StartAttackProc ();
			/*dmk14 visual bug fix*/
			if ( m_actorMove.PathIsActive() )	m_actorMove.Stop();
		}
		break;

	case GLAT_SKILL:
		{
			//	Note : 스킬 정보 가져옴.
			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_idACTIVESKILL.wMainID, m_idACTIVESKILL.wSubID );
			if ( !pSkill )								return;

			m_emANIMAINSKILL = pSkill->m_sEXT_DATA.emANIMTYPE;
			m_emANISUBSKILL = pSkill->m_sEXT_DATA.emANISTYPE;

			StartSkillProc ();
			SetHiddenWeaponPiece();

			/*dmk14 visual bug fix*/
			if ( m_actorMove.PathIsActive() )	m_actorMove.Stop();
		}
		break;

	case GLAT_FALLING:
		FACTEFF::DeleteEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT, m_sSTATEBLOWS );
		break;

	case GLAT_CONFT_END:
		break;

	case GLAT_DIE:
		FACTEFF::DeleteEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT, m_sSTATEBLOWS );
		break;
	case GLAT_GATHERING:		
		break;

	default:
		GASSERT("GLCharacter::TurnAction() 준비되지 않은 ACTION 이 들어왔습니다.");
		break;
	};

	if ( m_actorMove.PathIsActive() )
	{
		/*push pull skill logic, Juver, 2017/06/05 */
		/*dash skill logic, Juver, 2017/06/17 */
		if ( !IsACTION(GLAT_MOVE) && !IsACTION(GLAT_PUSHPULL) && !IsACTION(GLAT_SKILLMOVE) && !IsACTION(GLAT_SKILLDASH) ) m_actorMove.Stop();
	}
}

HRESULT GLCharClient::UpateAnimation ( float fTime, float fElapsedTime )
{
	HRESULT hr=S_OK;

	BOOL bPeaceZone = GLGaeaClient::GetInstance().GetActiveMap()->IsPeaceZone();
	if ( !bPeaceZone && IsSTATE(EM_ACT_PEACEMODE) )		bPeaceZone = TRUE;

	/*vehicle booster system, Juver, 2017/08/12 */
	BOOL bVehicleBooster = IsSTATE(EM_ACT_VEHICLE_BOOSTER);

	BOOL bLowSP = FALSE;
	BOOL bFreeze = FALSE;

	EMANI_MAINTYPE emMType;
	EMANI_SUBTYPE emSType;

	switch ( m_Action )
	{
	case GLAT_IDLE:
		{
			m_fIdleTime += fElapsedTime;

			if ( bPeaceZone && m_pSkinChar->GETANI ( AN_PLACID, AN_SUB_NONE ) )
			{
				emMType = AN_PLACID;
				emSType = AN_SUB_NONE;
				if ( m_bVehicle )
				{
					/*vehicle system, Juver, 2017/08/07 */
					int emType = m_sVehicle.m_emTYPE;
					if ( emType == VEHICLE_TYPE_BOARD )
					{
						emSType = (EMANI_SUBTYPE) ( AN_SUB_HOVERBOARD );
					}else{
						emMType = CHECK_VEHICLE_MAIN( m_sVehicle.m_sVehicleID );
						/*vehicle booster system, Juver, 2017/08/12 */
						emSType = CHECK_VEHICLE_SUB( bVehicleBooster? AN_ATTACK : AN_PLACID );
					}
				}
				else  emSType = AN_SUB_NONE;
			}
			else
			{
				BOOL bPANT(FALSE); // 헐떡거림.

				if ( m_pSkinChar->GETANI ( AN_GUARD_L, m_emANISUBTYPE ) )
				{
					bPANT = bLowSP || m_bSTATE_PANT;
				}

				if ( bPANT )
				{
					emMType = AN_GUARD_L;
					emSType = m_emANISUBTYPE;
				}
				else
				{
					emMType = AN_GUARD_N;
					emSType = m_emANISUBTYPE;
				}
			}

			m_pSkinChar->SELECTANI ( emMType, emSType );

			/*ABL system, Juver, 2017/06/02 */
			for ( int i=0; i<PIECE_SIZE; ++i )
			{
				if ( m_pSkinChar->m_pAttBone[i] )	
					m_pSkinChar->m_pAttBone[i]->SELECTANI( AN_ABL_STAY, AN_SUB_NONE );
			}

			/*vehicle system, Juver, 2017/08/07 */
			/*vehicle booster system, Juver, 2017/08/12 */
			if ( m_pSkinChar->m_pVehicle )	
				m_pSkinChar->m_pVehicle->SELECTANI( bVehicleBooster? AN_VEHICLE_BOOSTER : AN_VEHICLE_STAY , AN_SUB_NONE );
		}break;

	case GLAT_MOVE:
		{
			emMType = IsSTATE(EM_ACT_RUN) ? AN_RUN : AN_WALK;
			emSType = bPeaceZone ? AN_SUB_NONE : m_emANISUBTYPE;
			if ( m_bVehicle )
			{
				/*vehicle system, Juver, 2017/08/07 */
				int emType = m_sVehicle.m_emTYPE;
				if ( emType == VEHICLE_TYPE_BOARD )
				{
					emSType = (EMANI_SUBTYPE) ( AN_SUB_HOVERBOARD );
				}else{
					emMType = CHECK_VEHICLE_MAIN( m_sVehicle.m_sVehicleID );
					/*vehicle booster system, Juver, 2017/08/12 */
					emSType = CHECK_VEHICLE_SUB( bVehicleBooster? AN_ATTACK : IsSTATE(EM_ACT_RUN) ? AN_RUN : AN_WALK );
				}
			}
			m_pSkinChar->SELECTANI ( emMType, emSType );

			/*ABL system, Juver, 2017/06/02 */
			for ( int i=0; i<PIECE_SIZE; ++i )
			{
				if ( m_pSkinChar->m_pAttBone[i] )	
					m_pSkinChar->m_pAttBone[i]->SELECTANI( AN_ABL_STAY, AN_SUB_NONE );
			}

			/*vehicle system, Juver, 2017/08/07 */
			/*vehicle booster system, Juver, 2017/08/12 */
			if ( m_pSkinChar->m_pVehicle )	
				m_pSkinChar->m_pVehicle->SELECTANI( bVehicleBooster? AN_VEHICLE_BOOSTER : IsSTATE(EM_ACT_RUN) ? AN_VEHICLE_RUN : AN_VEHICLE_WALK, AN_SUB_NONE );
		}break;

	case GLAT_ATTACK:
		{
			EMSLOT emRHand = GetCurRHand();
			EMSLOT emLHand = GetCurLHand();

			SITEM* pRHAND = NULL;
			SITEM* pLHAND = NULL;

			if ( m_CharData.m_PutOnItems[emRHand].sNativeID!=NATIVEID_NULL() )
				pRHAND = GLItemMan::GetInstance().GetItem ( m_CharData.m_PutOnItems[emRHand].sNativeID );
			
			if ( m_CharData.m_PutOnItems[emLHand].sNativeID!=NATIVEID_NULL() )
				pLHAND = GLItemMan::GetInstance().GetItem ( m_CharData.m_PutOnItems[emLHand].sNativeID );

			EMANI_SUBTYPE emANISUBTYPE = CHECK_ATTACK_ANISUB ( pRHAND, pLHAND );
			m_pSkinChar->SELECTANI ( AN_ATTACK, emANISUBTYPE, NULL, m_dwANISUBSELECT );
			if ( m_pSkinChar->ISENDANIM () )
			{
				TurnAction ( GLAT_IDLE );

				/*ABL system, Juver, 2017/06/02 */
				for ( int i=0; i<PIECE_SIZE; ++i )
				{
					if ( m_pSkinChar->m_pAttBone[i] )	
						m_pSkinChar->m_pAttBone[i]->SELECTANI( AN_ABL_STAY, AN_SUB_NONE );
				}
			}
		}break;

	case GLAT_SKILL:
		{
			m_pSkinChar->SELECTANI ( m_emANIMAINSKILL, m_emANISUBSKILL );
			if ( m_pSkinChar->ISENDANIM () )
			{
				TurnAction ( GLAT_IDLE );

				/*ABL system, Juver, 2017/06/02 */
				for ( int i=0; i<PIECE_SIZE; ++i )
				{
					if ( m_pSkinChar->m_pAttBone[i] )	
						m_pSkinChar->m_pAttBone[i]->SELECTANI( AN_ABL_STAY, AN_SUB_NONE );
				}
			}
		}break;

	case GLAT_TALK:
		{
			BOOL bOK = m_pSkinChar->SELECTANI ( AN_GESTURE, EMANI_SUBTYPE(m_dwANISUBGESTURE) );
			if ( !bOK )
			{
				TurnAction ( GLAT_IDLE );
				break;
			}

			PSANIMCONTAINER pANIMCON = m_pSkinChar->GETCURANIM();
			if ( !(pANIMCON->m_dwFlag&ACF_LOOP) && m_pSkinChar->ISENDANIM () )
			{
				TurnAction ( GLAT_IDLE );
			}
		}
		break;

	case GLAT_SHOCK:
		{
			emMType = AN_SHOCK;	
			emSType = m_emANISUBTYPE;

			if ( m_bVehicle )
			{
				/*vehicle system, Juver, 2017/08/07 */
				int emType = m_sVehicle.m_emTYPE;
				if ( emType == VEHICLE_TYPE_BOARD )
				{
					emSType = (EMANI_SUBTYPE) ( AN_SUB_HOVERBOARD );
				}else{
					emMType = CHECK_VEHICLE_MAIN( m_sVehicle.m_sVehicleID );
					emSType = CHECK_VEHICLE_SUB( AN_SHOCK );
				}
			}

			m_pSkinChar->SELECTANI ( emMType, emSType );

			if ( m_pSkinChar->ISENDANIM () )	TurnAction ( GLAT_IDLE );

			/*vehicle system, Juver, 2017/08/07 */
			if ( m_pSkinChar->m_pVehicle )	
				m_pSkinChar->m_pVehicle->SELECTANI( AN_VEHICLE_SHOCK, AN_SUB_NONE );

		}break;

	case GLAT_PUSHPULL:
		m_pSkinChar->SELECTANI ( AN_SHOCK, m_emANISUBTYPE );
		break;

		/*dash skill logic, Juver, 2017/06/17 */
	case GLAT_SKILLDASH:
		{
			if ( m_wActionAnim != AN_SUB_00_SIZE )
				m_pSkinChar->SELECTANI ( AN_SPEC, (EMANI_SUBTYPE)m_wActionAnim );
		}break;

	case GLAT_FALLING:
		{
			emMType = AN_DIE;	
			emSType = AN_SUB_NONE;

			if ( m_bVehicle )
			{
				/*vehicle system, Juver, 2017/08/07 */
				int emType = m_sVehicle.m_emTYPE;
				if ( emType == VEHICLE_TYPE_BOARD )
				{
					emMType = AN_DIE;//fix board die
					emSType = (EMANI_SUBTYPE) ( AN_SUB_HOVERBOARD );
				}else{
					emMType = CHECK_VEHICLE_MAIN( m_sVehicle.m_sVehicleID );
					emSType = CHECK_VEHICLE_SUB( AN_DIE );
				}
			}

			/*ABL system, Juver, 2017/06/02 */
			for ( int i=0; i<PIECE_SIZE; ++i )
			{
				if ( m_pSkinChar->m_pAttBone[i] )	
					m_pSkinChar->m_pAttBone[i]->SELECTANI( AN_ABL_FREE, AN_SUB_NONE );
			}

			/*vehicle system, Juver, 2017/08/07 */
			if ( m_pSkinChar->m_pVehicle )	
				m_pSkinChar->m_pVehicle->SELECTANI( AN_VEHICLE_DOWN, AN_SUB_NONE, EMANI_ENDFREEZE );

			m_pSkinChar->SELECTANI ( emMType, emSType, EMANI_ENDFREEZE );

			if ( m_pSkinChar->ISENDANIM () )
			{
				TurnAction ( GLAT_DIE );
			}
		}break;

	case GLAT_DIE:
		{
			bFreeze = TRUE;

			emMType = AN_DIE;	
			emSType = AN_SUB_NONE;

			if ( m_bVehicle )
			{
				/*vehicle system, Juver, 2017/08/07 */
				int emType = m_sVehicle.m_emTYPE;
				if ( emType == VEHICLE_TYPE_BOARD )
				{
					emSType = (EMANI_SUBTYPE) ( AN_SUB_HOVERBOARD );
				}else{
					emMType = CHECK_VEHICLE_MAIN( m_sVehicle.m_sVehicleID );
					emSType = CHECK_VEHICLE_SUB( AN_DIE );
				}
			}

			/*ABL system, Juver, 2017/06/02 */
			for ( int i=0; i<PIECE_SIZE; ++i )
			{
				if ( m_pSkinChar->m_pAttBone[i] )	
					m_pSkinChar->m_pAttBone[i]->SELECTANI( AN_ABL_FREE, AN_SUB_NONE );
			}

			/*vehicle system, Juver, 2017/08/07 */
			if ( m_pSkinChar->m_pVehicle )	
			{
				m_pSkinChar->m_pVehicle->SELECTANI( AN_VEHICLE_DOWN, AN_SUB_NONE, EMANI_ENDFREEZE );
				m_pSkinChar->m_pVehicle->TOENDTIME();
			}

			/*vehicle system, Juver, 2017/08/09 */
			m_pSkinChar->SELECTANI ( emMType, emSType, EMANI_ENDFREEZE );
			m_pSkinChar->TOENDTIME();
		}break;

	case GLAT_CONFT_END:
		{
			EMANI_MAINTYPE emMTYPE(AN_CONFT_LOSS);
			if ( IsSTATE(EM_ACT_CONFT_WIN) )	emMTYPE = AN_CONFT_WIN;

			m_pSkinChar->SELECTANI ( emMTYPE, AN_SUB_NONE );
			if ( m_pSkinChar->ISENDANIM () )	TurnAction ( GLAT_IDLE );
		}
		break;

	case GLAT_GATHERING:
		{
			BOOL bOK = m_pSkinChar->SELECTANI ( AN_GATHERING, EMANI_SUBTYPE(m_dwANISUBGESTURE) );
			if ( !bOK )
			{
				TurnAction ( GLAT_IDLE );
				break;
			}

			PSANIMCONTAINER pANIMCON = m_pSkinChar->GETCURANIM();
			if ( !(pANIMCON->m_dwFlag&ACF_LOOP) && m_pSkinChar->ISENDANIM () )
			{
				TurnAction ( GLAT_IDLE );
			}
		}
		break;
	};

	//	Note : 스킨 업데이트.
	//
	m_pSkinChar->SetPosition ( m_vPos );


	BOOL bContinue = IsACTION(GLAT_ATTACK) || IsACTION(GLAT_SKILL);

	//	Note : 상태 이상에 따라 모션 속도를 조정한다.
	//
	float fSkinAniElap = fElapsedTime;
	switch ( m_Action )
	{
	case GLAT_MOVE:
		fSkinAniElap *= ( GETMOVE_ITEM() + GETMOVEVELO() );
		break;

	case GLAT_ATTACK:
	case GLAT_SKILL:
		fSkinAniElap *= GETATTVELO();;
		fSkinAniElap += GETATT_ITEM();;
		break;
	};

	m_pSkinChar->FrameMove ( fTime, fSkinAniElap, bContinue, bFreeze, !bPeaceZone );

	//	Note : 비 전투 지역일때 표시 안되야 하는 아이템(단검,투척) 인지 검사후 랜더 끄기.
	//
	if ( bPeaceZone )
	{
		EMSLOT emRHand = GetCurRHand();
		EMSLOT emLHand = GetCurLHand();

		SITEM *pItemR(NULL);
		SITEM *pItemL(NULL);

		if ( VALID_SLOT_ITEM(emRHand) )
		{
			pItemR = GLItemMan::GetInstance().GetItem ( GET_SLOT_ITEM(emRHand).sNativeID );
		}

		if ( VALID_SLOT_ITEM(emLHand) )
		{
			pItemL = GLItemMan::GetInstance().GetItem ( GET_SLOT_ITEM(emLHand).sNativeID );
		}

		if ( pItemR )
		{
			BOOL bRend = !( pItemR->sSuitOp.emAttack==ITEMATT_DAGGER || pItemR->sSuitOp.emAttack==ITEMATT_THROW );
			m_pSkinChar->SetPartRend ( PIECE_RHAND, bRend );
		}

		if ( pItemL )
		{
			BOOL bRend = !( pItemL->sSuitOp.emAttack==ITEMATT_DAGGER || pItemL->sSuitOp.emAttack==ITEMATT_THROW );
			m_pSkinChar->SetPartRend ( PIECE_LHAND, bRend );
		}
	}
	else
	{
		m_pSkinChar->SetPartRend ( PIECE_RHAND, TRUE );
		m_pSkinChar->SetPartRend ( PIECE_LHAND, TRUE );
	}

	return S_OK;
}

BOOL GLCharClient::IsCollisionVolume ()
{
	const CLIPVOLUME &CV = DxViewPort::GetInstance().GetClipVolume();
	if ( COLLISION::IsCollisionVolume ( CV, m_vMax, m_vMin ) )	return TRUE;

	return FALSE;
}

float GLCharClient::GETATTVELO ()
{
//	return m_fATTVELO<0.0f?0.0f:m_fATTVELO;
	float fATTVELO = m_fATTVELO + m_fITEMATTVELO_R;
	return fATTVELO<0.0f?0.0f:fATTVELO;
}

float GLCharClient::GETMOVEVELO ()
{
	float fMOVE = m_fSTATE_MOVE + m_fSKILL_MOVE + m_fITEM_MOVE_R; 
//	float fMOVE = m_fSTATE_MOVE + m_fSKILL_MOVE;
	return fMOVE<0.0f?0.0f:fMOVE;
}

float GLCharClient::GETATT_ITEM ()
{
	float fATTVELO = m_fITEMATTVELO / 100;
	return fATTVELO;
}
float GLCharClient::GETMOVE_ITEM ()
{
	float fMOVE = m_fITEM_MOVE / GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX].fRUNVELO;
	return fMOVE<0.0f?0.0f:fMOVE;
}

float GLCharClient::GetMoveVelo ()
{
	float fDefaultVelo = IsSTATE(EM_ACT_RUN) ? GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX].fRUNVELO : GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX].fWALKVELO;
	float fMoveVelo = fDefaultVelo * ( GETMOVEVELO() + GETMOVE_ITEM() );
	return fMoveVelo;
}

void GLCharClient::UpdateSpecialSkill()
{
	// 변신 스킬 체크
	if( m_dwTransfromSkill != SNATIVEID::ID_NULL )
	{
		if( m_sSKILLFACT[m_dwTransfromSkill].IsSpecialSkill( SKILL::EMSSTYPE_TRANSFORM ) )
		{
			// 쓰러지는 모션이면 변신해제
			STARGETID sTargetID(CROW_PC,m_dwGaeaID,m_vPos);
			if( IsACTION( GLAT_FALLING ) )
			{
				FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT[m_dwTransfromSkill].sNATIVEID );
			}

			if( m_sSKILLFACT[m_dwTransfromSkill].sNATIVEID != NATIVEID_NULL() ) 
			{			
				PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_sSKILLFACT[m_dwTransfromSkill].sNATIVEID.wMainID, m_sSKILLFACT[m_dwTransfromSkill].sNATIVEID.wSubID );
				if( pSkill && pSkill->m_sSPECIAL_SKILL.emSSTYPE == SKILL::EMSSTYPE_TRANSFORM )
				{
					if( pSkill->m_sSPECIAL_SKILL.strEffectName.size() != 0 )
					{
						if( m_sSKILLFACT[m_dwTransfromSkill].fAGE <= pSkill->m_sSPECIAL_SKILL.dwRemainSecond && 
							!m_sSKILLFACT[m_dwTransfromSkill].bRanderSpecialEffect )
						{
							D3DXMATRIX matTrans;							
							D3DXMatrixTranslation ( &matTrans, m_vPos.x, m_vPos.y, m_vPos.z );
							DxEffGroupPlayer::GetInstance().NewEffGroup ( pSkill->m_sSPECIAL_SKILL.strEffectName.c_str(), matTrans, &sTargetID );
							m_sSKILLFACT[m_dwTransfromSkill].bRanderSpecialEffect = TRUE;
						}
					}
				}
			}else{
				DISABLESKEFF(m_dwTransfromSkill);
				UpdateSuit();				
				m_dwTransfromSkill = SNATIVEID::ID_NULL;
			}


		}else{
			DISABLESKEFF(m_dwTransfromSkill);
			UpdateSuit();
			m_dwTransfromSkill = SNATIVEID::ID_NULL;
		}
	}
}

HRESULT GLCharClient::FrameMove ( float fTime, float fElapsedTime )
{
	HRESULT hr=S_OK;


	// 지형 효과 업데이트
	UpdateLandEffect();
	//	Note : 상태 갱신.

	//
	UPDATE_DATA ( fTime, fElapsedTime, TRUE );

	//	Note : 케릭 현제 위치 업대이트.
	//
	m_vPos = m_actorMove.Position ();

	//	Note : 공격 목표의 유효성 검사.
	//		(주의) 타갯정보 사용전 반드시 호출하여 유효성 검사 필요.
	//
	GLCOPY* pTarget = GLGaeaClient::GetInstance().GetCopyActor ( m_sTargetID );
	if ( !pTarget )
	{
		m_sTargetID.dwID = EMTARGET_NULL;
		if ( IsACTION(GLAT_ATTACK) )	TurnAction ( GLAT_IDLE );
	}

	//	Note : 에니메이션, 스킨 업데이트.
	//
	UpateAnimation ( fTime, fElapsedTime );

	switch ( m_Action )
	{
	case GLAT_MOVE:
		{
			//	Note : 케릭의 이동 업데이트.
			//
			//need stun getter here cause m_bSTATE_STUN is not being recognized
			//must put the stun here
			//cndev
			if ( m_bSTATE_STUN )
			{
				if ( m_actorMove.PathIsActive() )	m_actorMove.Stop();
			}

			m_actorMove.SetMaxSpeed ( GetMoveVelo () );
			m_actorMove.Update ( fElapsedTime );
			if ( !m_actorMove.PathIsActive() )
			{
				m_actorMove.Stop ();
				TurnAction ( GLAT_IDLE );
			}

			//	Note : 케릭의 현제 위치 업데이트.
			//
			m_vPos = m_actorMove.Position();

			//	Note : 현제 방향 업데이트.
			//
			D3DXVECTOR3 vMovement = m_actorMove.NextPosition();
			if ( vMovement.x != FLT_MAX && vMovement.y != FLT_MAX && vMovement.z != FLT_MAX )
			{
				D3DXVECTOR3 vDirection = vMovement - m_vPos;
				if ( !DxIsMinVector(vDirection,0.2f) )
				{
					D3DXVec3Normalize ( &vDirection, &vDirection );
					m_vDir = vDirection;
				}
			}
		}
		break;

	case GLAT_ATTACK:
		{
			AttackProc ( fElapsedTime );

			//	Note : 공격 방향으로 회전.
			//
			GLCOPY* pTarget = GLGaeaClient::GetInstance().GetCopyActor ( m_sTargetID );
			if ( pTarget )
			{
				D3DXVECTOR3 vDirection = pTarget->GetPosition() - m_vPos;
				D3DXVec3Normalize ( &vDirection, &vDirection );
				m_vDir = vDirection;
			}
		}
		break;

	case GLAT_SKILL:
		{
			SkillProc ( fElapsedTime );
			/*dmk14 visual bug fix*/
			if ( m_actorMove.PathIsActive() )	m_actorMove.Stop();

			//	Note : 공격 방향으로 회전.
			//
			m_vDir = UpdateSkillDirection ( m_vPos, m_vDir, m_idACTIVESKILL, m_vTARPOS, m_sTARIDS );
		}
		break;

	case GLAT_PUSHPULL:
	case GLAT_SKILLMOVE: /*push pull skill logic, Juver, 2017/06/05 */
	case GLAT_SKILLDASH: /*dash skill logic, Juver, 2017/06/17 */
		{
			m_actorMove.Update ( fElapsedTime );
			if ( !m_actorMove.PathIsActive() )
			{
				m_actorMove.Stop ();
				TurnAction ( GLAT_IDLE );
			}
		}
		break;
	};

	//	Note : 스킬 이펙트 업데이트.
	//
	FACTEFF::UpdateSkillEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT, m_sSTATEBLOWS );

	UpdateSpecialSkill();

	//	Note : 현제 위치 업데이트.
	//
	m_vPos = m_actorMove.Position();

	//	Note : 현제 위치와 방향으로 Transform 메트릭스 계산.
	//
	D3DXMATRIX matTrans, matYRot;
	D3DXMatrixTranslation ( &matTrans, m_vPos.x, m_vPos.y, m_vPos.z );
	float fThetaY = DXGetThetaYFromDirection ( m_vDir, m_vDirOrig );
	D3DXMatrixRotationY ( &matYRot, fThetaY );

	m_matTrans = matYRot * matTrans;

	//	Note : AABB 계산.
	//
	m_vMax = m_vPos + m_vMaxOrg;
	m_vMin = m_vPos + m_vMinOrg;

	return S_OK;
}

float GLCharClient::GetDirection ()
{
	return DXGetThetaYFromDirection ( m_vDir, m_vDirOrig );
}

HRESULT GLCharClient::Render ( LPDIRECT3DDEVICEQ pd3dDevice, CLIPVOLUME &cv, BOOL bRendAABB )
{
	HRESULT hr;
	if ( !IsVisibleDetect() )			return S_FALSE;

	//	Note : m_bINVISIBLE 가 참일 경우 반투명하게 표현해야함.
	//
	if ( m_bINVISIBLE )
	{
		DxEffcharDataMan::GetInstance().PutPassiveEffect ( m_pSkinChar, GLCONST_CHAR::strHALFALPHA_EFFECT.c_str(), &m_vDir );
	}
	else
	{
		DxEffcharDataMan::GetInstance().OutEffect ( m_pSkinChar, GLCONST_CHAR::strHALFALPHA_EFFECT.c_str() );
	}

	//	TODO : m_bINVISIBLE 가 참일 경우 반투명하게 표현해야함.

	if ( !COLLISION::IsCollisionVolume ( cv, m_vMax, m_vMin ) )	return S_OK;

	if ( m_pSkinChar )
	{
		/*vehicle system, Juver, 2017/08/07 */
		if ( m_bVehicle )
		{
			NavigationMesh* pNavi = GLGaeaClient::GetInstance().GetActiveMap()->GetNaviMesh();
			if ( pNavi )
			{
				BOOL bCoA(FALSE);
				DWORD dwCoA(0);
				D3DXVECTOR3 vCoA(0,0,0);

				//current position
				pNavi->IsCollision ( D3DXVECTOR3(m_vPos.x,m_vPos.y+5.0f,m_vPos.z), D3DXVECTOR3(m_vPos.x,m_vPos.y-5.0f,m_vPos.z), vCoA, dwCoA, bCoA );
				if ( bCoA )
				{
					D3DXVECTOR3 vPosFront = m_vPos + m_vDir * 2.0f;
					D3DXVECTOR3 vDirFront = ( vPosFront - m_vPos );
					vDirFront.y = 0.0f;
					D3DXVec3Normalize (  &vDirFront, &vDirFront );

					D3DXVECTOR3 vPosBack = m_vPos - m_vDir * 2.0f;
					D3DXVECTOR3 vDirBack = ( vPosBack - m_vPos );
					vDirBack.y = 0.0f;
					D3DXVec3Normalize (  &vDirBack, &vDirBack );

					D3DXVECTOR3 vPosB = vPosFront + vDirFront * 2.0f;
					D3DXVECTOR3 vPosC = vPosBack + vDirBack * 2.0f;

					BOOL bCoB(FALSE);
					DWORD dwCoB(0);
					D3DXVECTOR3 vCoB(0,0,0);

					BOOL bCoC(FALSE);
					DWORD dwCoC(0);
					D3DXVECTOR3 vCoC(0,0,0);

					pNavi->IsCollision ( D3DXVECTOR3(vPosB.x,vPosB.y+5.0f,vPosB.z), D3DXVECTOR3(vPosB.x,vPosB.y-5.0f,vPosB.z), vCoB, dwCoB, bCoB );
					if ( !bCoB )
					{
						float fNewdxLength = 1.0f;
						while ( 0.0f < fNewdxLength )
						{
							vPosB = vPosFront + vDirFront * fNewdxLength;
							pNavi->IsCollision  ( D3DXVECTOR3(vPosB.x,vPosB.y+5.0f,vPosB.z), D3DXVECTOR3(vPosB.x,vPosB.y-5.0f,vPosB.z), vCoB, dwCoB, bCoB );
							if ( bCoB )		break;
							fNewdxLength -= 0.2f;
						};
					}

					pNavi->IsCollision ( D3DXVECTOR3(vPosC.x,vPosC.y+5.0f,vPosC.z), D3DXVECTOR3(vPosC.x,vPosC.y-5.0f,vPosC.z), vCoC, dwCoC, bCoC );
					if ( !bCoC )
					{
						float fNewdxLength = 1.0f;
						while ( 0.0f < fNewdxLength )
						{
							vPosC = vPosBack + vDirBack * fNewdxLength;
							pNavi->IsCollision  ( D3DXVECTOR3(vPosC.x,vPosC.y+5.0f,vPosC.z), D3DXVECTOR3(vPosC.x,vPosC.y-5.0f,vPosC.z), vCoC, dwCoC, bCoC );
							if ( bCoC )		break;
							fNewdxLength -= 0.2f;
						};
					}

					if ( bCoB && bCoC )
					{
						if ( pNavi->LineOfSightTest ( dwCoA, vCoA, dwCoB, vCoB ) && pNavi->LineOfSightTest ( dwCoA, vCoA, dwCoC, vCoC ) )
						{
							D3DXVECTOR3 vDIR = vCoB - vCoC;
							float fLength = D3DXVec3Length ( &vDIR );
							if ( fLength == 0 )		fLength = 0.001f;
							float fThetaX = (float) asin ( vDIR.y / fLength );
							D3DXMATRIX matX;
							D3DXMatrixRotationX ( &matX, fThetaX );
							m_matTrans = matX * m_matTrans;
						}
					}
				}
			}
		}

		m_pSkinChar->SetScaleAdjust( m_CharData.m_fScaleRange );

		/*vehicle system, Juver, 2017/08/07 */
		if ( m_pSkinChar->m_pVehicle )
			hr = m_pSkinChar->RenderVehicle( pd3dDevice, m_matTrans );
		else
			hr = m_pSkinChar->Render ( pd3dDevice, m_matTrans );

		if ( FAILED(hr) )	return hr;
	}
	
	#ifdef _SYNC_TEST
	{
		EDITMESHS::RENDERSPHERE ( pd3dDevice, m_vServerPos, 2.0f );
	}
	#endif

	return S_OK;
}

HRESULT GLCharClient::RenderShadow ( LPDIRECT3DDEVICEQ pd3dDevice, CLIPVOLUME &cv )
{
	HRESULT hr;
	if ( !IsVisibleDetect() )									return S_FALSE;
	if ( IsACTION(GLAT_DIE) )									return S_FALSE;
	if ( !COLLISION::IsCollisionVolume ( cv, m_vMax, m_vMin ) )	return S_OK;

	//	Note : 그림자 랜더링.
	//
	hr = DxShadowMap::GetInstance().RenderShadowCharMob ( m_pSkinChar, m_matTrans, pd3dDevice );
	if ( FAILED(hr) )	return hr;

	return S_OK;
}

HRESULT GLCharClient::RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	m_pSkinChar->RestoreDeviceObjects ( pd3dDevice );
	return S_OK;
}

HRESULT GLCharClient::InvalidateDeviceObjects ()
{
	m_pSkinChar->InvalidateDeviceObjects ();
	return S_OK;
}

EMELEMENT GLCharClient::GET_ITEM_ELMT ()
{
	EMSLOT emRHand = GetCurRHand();
	EMSLOT emLHand = GetCurLHand();

	SITEM* pRHandItem = GET_SLOT_ITEMDATA(emRHand);
	SITEM* pLHandItem = GET_SLOT_ITEMDATA(emLHand);

	EMELEMENT emELEMENT(EMELEMENT_SPIRIT);
	if ( pRHandItem && pRHandItem->sSuitOp.sBLOW.emTYPE!=EMBLOW_NONE )
	{
		emELEMENT = STATE_TO_ELEMENT ( pRHandItem->sSuitOp.sBLOW.emTYPE );
	}
	else if ( pLHandItem && pLHandItem->sSuitOp.sBLOW.emTYPE!=EMBLOW_NONE )
	{
		emELEMENT = STATE_TO_ELEMENT ( pLHandItem->sSuitOp.sBLOW.emTYPE );
	}

	return emELEMENT;
}

void GLCharClient::ReceiveDamage ( DWORD dwDamage, DWORD dwDamageFlag, STARGETID sACTOR )
{
	if ( dwDamageFlag & DAMAGE_TYPE_SHOCK )	TurnAction ( GLAT_SHOCK );

	D3DXVECTOR3 vPos = GetPosBodyHeight();
	CInnerInterface::GetInstance().SetDamage( vPos, dwDamage, dwDamageFlag, UI_ATTACK );

	if ( dwDamageFlag & DAMAGE_TYPE_CRUSHING_BLOW )
	{
		// 강한타격 이펙트
		D3DXVECTOR3 vDIR = sACTOR.vPos - m_vPos;

		D3DXVECTOR3 vDIR_ORG(1,0,0);
		float fdir_y = DXGetThetaYFromDirection ( vDIR, vDIR_ORG );

		D3DXMATRIX matTrans;
		D3DXMatrixRotationY ( &matTrans, fdir_y );
		matTrans._41 = m_vPos.x;
		matTrans._42 = m_vPos.y + 10.0f;
		matTrans._43 = m_vPos.z;

		//	Note : 자기 위치 이펙트 발생시킴.
		DxEffGroupPlayer::GetInstance().NewEffGroup ( GLCONST_CHAR::strCRUSHING_BLOW_EFFECT.c_str(), matTrans, &sACTOR );
	}

	m_CharData.sHP.DECREASE(dwDamage);
	m_CharData.sHP.dwNow;

	//	Note : 방어스킬의 이펙트가 있을때 발동시킴.
	SKT_EFF_HOLDOUT ( sACTOR, dwDamageFlag );
}

void GLCharClient::ReceiveAVoid ()
{
	if ( IsCollisionVolume() )
	{
		D3DXVECTOR3 vPos = GetPosBodyHeight();
		CInnerInterface::GetInstance().SetDamage( vPos, 0, DAMAGE_TYPE_NONE, UI_ATTACK );
	}
}

void GLCharClient::ReceiveSwing ()
{
	m_pSkinChar->DOSHOCKMIX();
}

HRESULT GLCharClient::MsgMoveState ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_MOVESTATE_BRD *pNetMsg = reinterpret_cast<GLMSG::SNETPC_MOVESTATE_BRD*> ( nmg );

	BOOL bRun = IsSTATE ( EM_ACT_RUN );
	BOOL bToRun = pNetMsg->dwActState&EM_ACT_RUN;

	if ( bRun != bToRun )
	{
		//	Note : 이동 상태 변경.
		if ( bToRun )		SetSTATE(EM_ACT_RUN);
		else				ReSetSTATE(EM_ACT_RUN);

		m_actorMove.SetMaxSpeed ( GetMoveVelo () );
	}

	if ( pNetMsg->dwActState & EM_REQ_VISIBLENONE )	SetSTATE ( EM_REQ_VISIBLENONE );
	else											ReSetSTATE ( EM_REQ_VISIBLENONE );

	if ( pNetMsg->dwActState & EM_REQ_VISIBLEOFF )	SetSTATE ( EM_REQ_VISIBLEOFF );
	else											ReSetSTATE ( EM_REQ_VISIBLEOFF );

	if ( pNetMsg->dwActState & EM_ACT_PEACEMODE )	SetSTATE ( EM_ACT_PEACEMODE );
	else											ReSetSTATE ( EM_ACT_PEACEMODE );

	/*vehicle booster system, Juver, 2017/08/12 */
	if ( pNetMsg->dwActState & EM_ACT_VEHICLE_BOOSTER )	
		SetSTATE ( EM_ACT_VEHICLE_BOOSTER );
	else										
		ReSetSTATE ( EM_ACT_VEHICLE_BOOSTER );

	return S_OK;
}

HRESULT GLCharClient::MsgGoto ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_GOTO_BRD *pNetMsg = reinterpret_cast<GLMSG::SNETPC_GOTO_BRD*> ( nmg );

	if ( pNetMsg->dwActState&EM_ACT_RUN )	SetSTATE ( EM_ACT_RUN );
	else									ReSetSTATE ( EM_ACT_RUN );

	m_sTargetID.vPos = pNetMsg->vTarPos;
	TurnAction ( GLAT_MOVE );

	BOOL bSucceed = m_actorMove.GotoLocation
	(
		m_sTargetID.vPos+D3DXVECTOR3(0,+10,0),
		m_sTargetID.vPos+D3DXVECTOR3(0,-10,0)
	);

	if ( bSucceed )
	{
		m_actorMove.SetMaxSpeed ( GetMoveVelo() );
	}

	return S_OK;
}

void GLCharClient::MsgProcess ( NET_MSG_GENERIC* nmg )
{
	switch ( nmg->nType )
	{
	case NET_MSG_GCTRL_MOVESTATE_BRD:				MsgMoveState ( nmg ); break;
	case NET_MSG_GCTRL_GOTO_BRD:					MsgGoto ( nmg ); break;
		
	case NET_MSG_GCTRL_REMOVE_SKILL_EFFECT_BRD:
		{
			GLMSG::SNETPC_REMOVE_SKILL_EFFECT_BRD *pNetMsg = (GLMSG::SNETPC_REMOVE_SKILL_EFFECT_BRD *)nmg;
			
			FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT[pNetMsg->dwSkillID].sNATIVEID );

			DISABLESKEFF(pNetMsg->dwSkillID);
		}
		break;

	case NET_MSG_GCTRL_JUMP_POS_BRD:
		{
			GLMSG::SNETPC_JUMP_POS_BRD *pNetMsg = (GLMSG::SNETPC_JUMP_POS_BRD *) nmg;
			m_vPos = pNetMsg->vPOS;
			SetPosition ( m_vPos );
		}
		break;

	case NET_MSG_GCTRL_ATTACK_BRD:
		{
			GLMSG::SNETPC_ATTACK_BRD *pNetMsg = (GLMSG::SNETPC_ATTACK_BRD *) nmg;

			//	Note : 공격 수행.
			m_sTargetID.emCrow = pNetMsg->emTarCrow;
			m_sTargetID.dwID = pNetMsg->dwTarID;
			m_dwANISUBSELECT = pNetMsg->dwAniSel;
			TurnAction ( GLAT_ATTACK );
		}
		break;

	case NET_MSG_GCTRL_ATTACK_CANCEL_BRD:
		{
			if ( IsACTION(GLAT_ATTACK) )	TurnAction ( GLAT_IDLE );
		}
		break;

	case NET_MSG_GCTRL_ATTACK_AVOID_BRD:
		{
			GLMSG::SNETPC_ATTACK_AVOID_BRD *pNetMsg = (GLMSG::SNETPC_ATTACK_AVOID_BRD *) nmg;

			GLCOPY* pActor = GLGaeaClient::GetInstance().GetCopyActor ( pNetMsg->emTarCrow, pNetMsg->dwTarID );
			if ( pActor )	pActor->ReceiveAVoid ();
		}
		break;

	case NET_MSG_GCTRL_ATTACK_DAMAGE_BRD:
		{
			GLMSG::SNETPC_ATTACK_DAMAGE_BRD *pNetMsg = (GLMSG::SNETPC_ATTACK_DAMAGE_BRD *) nmg;

			GLCOPY* pActor = GLGaeaClient::GetInstance().GetCopyActor ( pNetMsg->emTarCrow, pNetMsg->dwTarID );
			if ( pActor )
			{
				STARGETID sACTOR(GetCrow(),GetCtrlID(),GetPosition());
				pActor->ReceiveDamage ( pNetMsg->nDamage, pNetMsg->dwDamageFlag, sACTOR );
			}
		}
		break;

	case NET_MSG_GCTRL_ACTION_BRD:
		{
			GLMSG::SNET_ACTION_BRD *pNetMsg = (GLMSG::SNET_ACTION_BRD *)nmg;
			TurnAction ( pNetMsg->emAction );
		}
		break;

	case NET_MSG_GCTRL_PUTON_RELEASE_BRD:
		{
			GLMSG::SNETPC_PUTON_RELEASE_BRD *pNetMsg = (GLMSG::SNETPC_PUTON_RELEASE_BRD *)nmg;

			//	제거되는 아이탬 반영.
			m_CharData.m_PutOnItems[pNetMsg->emSlot] = SITEMCLIENT ( SNATIVEID(false) );
			UpdateSuit ();
			UPDATE_ITEM();

			ReSelectAnimation ();

			// 무기에 따른 버프를 초기화 한다.
			DisableSkillFact();
		}
		break;

	case NET_MSG_GCTRL_PUTON_UPDATE_BRD:
		{
			GLMSG::SNETPC_PUTON_UPDATE_BRD *pNetMsg = (GLMSG::SNETPC_PUTON_UPDATE_BRD *)nmg;

			//	제거되는 아이탬이 있을 경우.
			if ( pNetMsg->emSlotRelease != SLOT_NSIZE_S_2 )
				m_CharData.m_PutOnItems[pNetMsg->emSlotRelease] = SITEMCLIENT ( SNATIVEID(false) );

			//	변경되는 아이탬 반영.
			m_CharData.m_PutOnItems[pNetMsg->emSlot] = pNetMsg->sItemClient;
			UpdateSuit ();
			UPDATE_ITEM();

			ReSelectAnimation ();

			// 무기에 따른 버프를 초기화 한다.
			DisableSkillFact();
		}
		break;

	case NET_MSG_GCTRL_PUTON_CHANGE_BRD:
		{
			GLMSG::SNETPC_PUTON_CHANGE_BRD *pNetMsg = (GLMSG::SNETPC_PUTON_CHANGE_BRD *)nmg;

			SetUseArmSub( pNetMsg->bUseArmSub );

			UpdateSuit ();
			UPDATE_ITEM();

			ReSelectAnimation ();

			// 무기에 따른 버프를 초기화 한다.
			DisableSkillFact();
		}
		break;

	case NET_MSG_GCTRL_REQ_LEVELUP_BRD:
		{
			D3DXMATRIX matEffect;
			D3DXMatrixTranslation ( &matEffect, m_vPos.x, m_vPos.y, m_vPos.z );

			STARGETID sTargetID(CROW_PC,m_dwGaeaID,m_vPos);
			DxEffGroupPlayer::GetInstance().NewEffGroup
			(
				GLCONST_CHAR::strLEVELUP_EFFECT.c_str(),
				matEffect,
				&sTargetID
			);
		}
		break;

	case NET_MSG_GCTRL_REQ_SKILL_BRD:
		{
			GLMSG::SNETPC_REQ_SKILL_BRD *pNetMsg = (GLMSG::SNETPC_REQ_SKILL_BRD *)nmg;
			
			m_idACTIVESKILL = pNetMsg->skill_id;
			m_wACTIVESKILL_LEVEL = pNetMsg->wLEVEL;
			m_vTARPOS = pNetMsg->vTARPOS;

			m_wTARNUM = pNetMsg->wTARNUM;
			if ( m_wTARNUM > EMTARGET_NET )
			{
				break;
			}
			else
			{
				memcpy ( m_sTARIDS, pNetMsg->sTARIDS, sizeof(STARID)*m_wTARNUM );
				//	Note : 스킬이 발동됨.
				PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( pNetMsg->skill_id );
				if ( pSkill )
				{
					if ( IsACTION ( GLAT_IDLE ) || !(pSkill->m_sBASIC.dwFlags & FLAG_SB_RUNNING_CAST ) ) TurnAction ( GLAT_SKILL );
					//TurnAction ( GLAT_SKILL );
				}
			}
		}
		break;

	case NET_MSG_GCTRL_SKILL_CANCEL_BRD:
		{
			if ( IsACTION(GLAT_SKILL) )	TurnAction ( GLAT_IDLE );
		}
		break;

	case NET_MSG_GCTRL_SKILLFACT_BRD:
		{
			GLMSG::SNETPC_SKILLFACT_BRD *pNetMsg = (GLMSG::SNETPC_SKILLFACT_BRD *)nmg;

			m_CharData.sHP.VARIATION ( pNetMsg->nVAR_HP );

			//	데미지 메시지.
			if ( pNetMsg->nVAR_HP < 0 )
			{
				if ( pNetMsg->dwDamageFlag & DAMAGE_TYPE_SHOCK )	TurnAction ( GLAT_SHOCK );
				else					ReceiveSwing ();

				D3DXVECTOR3 vPos = GetPosBodyHeight();
				CInnerInterface::GetInstance().SetDamage( vPos, static_cast<DWORD>(-pNetMsg->nVAR_HP), pNetMsg->dwDamageFlag, UI_UNDERATTACK );
			
				//	Note : 방어스킬의 이펙트가 있을때 발동시킴.
				STARGETID sACTOR(pNetMsg->sACTOR.GETCROW(),pNetMsg->sACTOR.GETID());
				sACTOR.vPos = GLGaeaClient::GetInstance().GetTargetPos ( sACTOR );
				
				SKT_EFF_HOLDOUT ( sACTOR, pNetMsg->dwDamageFlag );

				if ( pNetMsg->dwDamageFlag & DAMAGE_TYPE_CRUSHING_BLOW )
				{
					// 강한타격 이펙트

					D3DXVECTOR3 vDIR = sACTOR.vPos - m_vPos;

					D3DXVECTOR3 vDIR_ORG(1,0,0);
					float fdir_y = DXGetThetaYFromDirection ( vDIR, vDIR_ORG );

					D3DXMATRIX matTrans;
					D3DXMatrixRotationY ( &matTrans, fdir_y );
					matTrans._41 = m_vPos.x;
					matTrans._42 = m_vPos.y + 10.0f;
					matTrans._43 = m_vPos.z;

					//	Note : 자기 위치 이펙트 발생시킴.
					DxEffGroupPlayer::GetInstance().NewEffGroup ( GLCONST_CHAR::strCRUSHING_BLOW_EFFECT.c_str(), matTrans, &sACTOR );
				}
			}
			else
			{
				if ( pNetMsg->dwDamageFlag & DAMAGE_TYPE_DAMAGE_ABSORBED || pNetMsg->dwDamageFlag & DAMAGE_TYPE_IMMUNE )
				{
					D3DXVECTOR3 vPos = GetPosBodyHeight();
					CInnerInterface::GetInstance().SetDamage( vPos, 0, pNetMsg->dwDamageFlag, UI_UNDERATTACK );
				}
			}

			//	힐링 메시지.
			//if ( pNetMsg->nVAR_HP > 0 )
			//{
			//	CPlayInterface::GetInstance().InsertText ( GetPosition(), static_cast<WORD>(pNetMsg->nVAR_HP), pNetMsg->bCRITICAL, 1 );
			//}
		}
		break;

	case NET_MSG_GCTRL_SKILLHOLD_BRD:
		{
			GLMSG::SNETPC_SKILLHOLD_BRD *pNetMsg = (GLMSG::SNETPC_SKILLHOLD_BRD *)nmg;

			if ( pNetMsg->fPARAM )
				m_sLINKHP.fRef = pNetMsg->fPARAM; //MaxHP 비율을 서버로 부터 받음

			//	지속형 스킬의 경우 스킬 팩터 추가됨.
			if ( pNetMsg->skill_id != SNATIVEID(false) )
			{
				RECEIVE_SKILLFACT ( pNetMsg->skill_id, pNetMsg->wLEVEL, pNetMsg->wSELSLOT, pNetMsg->wCasterCrow, pNetMsg->dwCasterID );
				FACTEFF::NewSkillFactEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, pNetMsg->skill_id, m_matTrans, m_vDir );
			}
		}
		break;

	case NET_MSG_GCTRL_SKILLHOLD_RS_BRD:
		{
			GLMSG::SNETPC_SKILLHOLD_RS_BRD *pNetMsg = (GLMSG::SNETPC_SKILLHOLD_RS_BRD *)nmg;

			//	Note : 스킬 fact 들을 종료.
			//		바로 리샛 하지 않고 여기서 시간 조종하여 정상 종료되게 함. ( 이팩트 종료 때문. )
			for ( int i=0; i<SKILLFACT_SIZE; ++i )
			{
				if ( pNetMsg->bRESET[i] )
				{
					FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT[i].sNATIVEID );

					DISABLESKEFF(i);
				}
			}
		}
		break;

	case NET_MSG_GCTRL_STATEBLOW_BRD:
		{
			GLMSG::SNETPC_STATEBLOW_BRD *pNetMsg = (GLMSG::SNETPC_STATEBLOW_BRD *)nmg;

			if ( pNetMsg->emBLOW <= EMBLOW_SINGLE )
				FACTEFF::DeleteBlowSingleEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSTATEBLOWS );

			SSTATEBLOW *pSTATEBLOW = NULL;
			if ( pNetMsg->emBLOW <= EMBLOW_SINGLE )		pSTATEBLOW = &m_sSTATEBLOWS[0];
			else										pSTATEBLOW = &m_sSTATEBLOWS[pNetMsg->emBLOW-EMBLOW_SINGLE];

			pSTATEBLOW->emBLOW = pNetMsg->emBLOW;
			pSTATEBLOW->fAGE = pNetMsg->fAGE;
			pSTATEBLOW->fSTATE_VAR1 = pNetMsg->fSTATE_VAR1;
			pSTATEBLOW->fSTATE_VAR2 = pNetMsg->fSTATE_VAR2;

			//	Note : 효과 생성.
			FACTEFF::NewBlowEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, pSTATEBLOW->emBLOW, m_matTrans, m_vDir );
		}
		break;

	case NET_MSG_GCTRL_CURESTATEBLOW_BRD:
		{
			GLMSG::SNETPC_CURESTATEBLOW_BRD *pNetMsg = (GLMSG::SNETPC_CURESTATEBLOW_BRD *)nmg;

			for ( int i=0; i<EMBLOW_MULTI; ++i )
			{
				EMSTATE_BLOW emBLOW = m_sSTATEBLOWS[i].emBLOW;

				if ( emBLOW==EMBLOW_NONE )						continue;

				EMDISORDER emDIS = STATE_TO_DISORDER(emBLOW);
				if ( !(pNetMsg->dwCUREFLAG&emDIS) )				continue;

				DISABLEBLOW(i);
				FACTEFF::DeleteBlowEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, emBLOW );
			}
		}
		break;

	case NET_MSG_GCTRL_PUSHPULL_BRD:
		{
			GLMSG::SNET_PUSHPULL_BRD *pNetMsg = (GLMSG::SNET_PUSHPULL_BRD *)nmg;
			const D3DXVECTOR3 &vMovePos = pNetMsg->vMovePos;

			//	Note : 밀려날 위치로 이동 시도.
			//
			BOOL bSucceed = m_actorMove.GotoLocation
			(
				D3DXVECTOR3(vMovePos.x,vMovePos.y+5,vMovePos.z),
				D3DXVECTOR3(vMovePos.x,vMovePos.y-5,vMovePos.z)
			);

			if ( bSucceed )
			{
				//	Note : 밀려나는 엑션 시작.
				//
				m_sTargetID.vPos = vMovePos;
				
				/*dash skill logic, Juver, 2017/06/17 */
				m_wActionAnim = pNetMsg->wActionAnim;

				if ( pNetMsg->bSkillDash )
				{
					/*dash skill logic, Juver, 2017/06/17 */
					TurnAction ( GLAT_SKILLDASH );
				}
				else if ( pNetMsg->bSkillMove )
				{
					/*push pull skill logic, Juver, 2017/06/05 */
					TurnAction ( GLAT_SKILLMOVE );
				}else{
					TurnAction ( GLAT_PUSHPULL );
				}

				//	Note : 밀리는 속도 설정.
				//
				/*push pull skill logic, Juver, 2017/06/04 */
				m_actorMove.SetMaxSpeed ( pNetMsg->fSpeed );

				/*push pull skill logic, Juver, 2017/06/05 */
				if ( pNetMsg->bSkillMove && vMovePos.x != FLT_MAX && vMovePos.y != FLT_MAX && vMovePos.z != FLT_MAX )
				{
					D3DXVECTOR3 vNewDirection = m_vPos - vMovePos;
					if ( pNetMsg->bReverseDir )	vNewDirection = vMovePos - m_vPos;
					if ( !DxIsMinVector(vNewDirection,0.2f) )
					{
						D3DXVec3Normalize ( &vNewDirection, &vNewDirection );
						m_vDir = vNewDirection;
					}
				}
			}
		}
		break;

	case NET_MSG_GCTRL_LANDEFFECT:
		{
			GLMSG::SNETPC_LANDEFFECT* pNetMsg = ( GLMSG::SNETPC_LANDEFFECT* ) nmg;
			memcpy( m_sLandEffect, pNetMsg->sLandEffect, sizeof(m_sLandEffect) );
		}
		break;


	case NET_MSG_GCTRL_CONFRONT_END2_CLT_BRD:
		{
			GLMSG::SNETPC_CONFRONT_END2_CLT_BRD *pNetMsg = (GLMSG::SNETPC_CONFRONT_END2_CLT_BRD *)nmg;

			switch ( pNetMsg->emEND )
			{
			case EMCONFRONT_END_WIN:
				SetSTATE(EM_ACT_CONFT_WIN);
				TurnAction ( GLAT_CONFT_END );
				break;

			case EMCONFRONT_END_LOSS:
				ReSetSTATE(EM_ACT_CONFT_WIN);
				TurnAction ( GLAT_CONFT_END );
				break;

			case EMCONFRONT_END_NOTWIN:
				ReSetSTATE(EM_ACT_CONFT_WIN);
				TurnAction ( GLAT_CONFT_END );
				break;

			case EMCONFRONT_END_TIME:
				ReSetSTATE(EM_ACT_CONFT_WIN);
				TurnAction ( GLAT_CONFT_END );
				break;

			case EMCONFRONT_END_DISWIN:
				SetSTATE(EM_ACT_CONFT_WIN);
				TurnAction ( GLAT_CONFT_END );
				break;

			case EMCONFRONT_END_DISLOSS:
				ReSetSTATE(EM_ACT_CONFT_WIN);
				TurnAction ( GLAT_CONFT_END );
				break;

			case EMCONFRONT_END_PWIN:
				SetSTATE(EM_ACT_CONFT_WIN);
				TurnAction ( GLAT_CONFT_END );
				break;

			case EMCONFRONT_END_PLOSS:
				ReSetSTATE(EM_ACT_CONFT_WIN);
				TurnAction ( GLAT_CONFT_END );
				break;
			};
		}
		break;

	case NET_MSG_GCTRL_UPDATE_BRIGHT_BRD:
		{
			GLMSG::SNETPC_UPDATE_BRIGHT_BRD *pNetMsg = (GLMSG::SNETPC_UPDATE_BRIGHT_BRD *)nmg;
			m_CharData.nBright = pNetMsg->nBright;
		}
		break;

	case NET_MSG_GCTRL_UPDATE_STATE_BRD:
		{
			GLMSG::SNETPC_UPDATE_STATE_BRD *pNetMsg = (GLMSG::SNETPC_UPDATE_STATE_BRD *)nmg;
			m_CharData.sHP = pNetMsg->sHP;
			m_CharData.bSafeTime = pNetMsg->bSafeTime;

			//CDebugSet::ToListView ( "NET_MSG_GCTRL_UPDATE_STATE_BRD %s,  %d/%d", m_CharData.szName, m_CharData.sHP.wNow, m_CharData.sHP.wMax );
		}
		break;

	case NET_MSG_GCTRL_UPDATE_FLAGS:
		{
			GLMSG::SNETPC_UPDATE_FLAGS_BRD *pNetMsg = (GLMSG::SNETPC_UPDATE_FLAGS_BRD *)nmg;
			m_CharData.dwFLAGS = pNetMsg->dwFLAGS;
		}
		break;

	case NET_MSG_GCTRL_PARTY_BRD:
		{
			GLMSG::SNET_PARTY_BRD *pNetMsg = (GLMSG::SNET_PARTY_BRD *)nmg;
			m_CharData.dwParty = pNetMsg->dwPartyID;
			m_CharData.dwPMasterID = pNetMsg->dwPMasterID;
		}
		break;

	case NET_MSG_GCTRL_UPDATE_PASSIVE_BRD:
		{
			GLMSG::SNETPC_UPDATE_PASSIVE_BRD *pNetMsg = (GLMSG::SNETPC_UPDATE_PASSIVE_BRD *)nmg;
			m_CharData.sPASSIVE_SKILL = pNetMsg->sSKILL_DATA;
		}
		break;

	case NET_MSG_GCTRL_POSITIONCHK_BRD:
		{
			GLMSG::SNET_POSITIONCHK_BRD *pNetNsg = (GLMSG::SNET_POSITIONCHK_BRD *)nmg;
			m_vServerPos = pNetNsg->vPOS;
		}
		break;

	case NET_MSG_GCTRL_PMARKET_OPEN_BRD:
		{
			GLMSG::SNETPC_PMARKET_OPEN_BRD *pNetNsg = (GLMSG::SNETPC_PMARKET_OPEN_BRD *)nmg;

			m_sPMarket.SetTITLE ( pNetNsg->szPMarketTitle );
			m_sPMarket.DoMarketOpen();
		}
		break;

	case NET_MSG_GCTRL_PMARKET_CLOSE_BRD:
		{
			GLMSG::SNETPC_PMARKET_CLOSE_BRD *pNetNsg = (GLMSG::SNETPC_PMARKET_CLOSE_BRD *)nmg;

			m_sPMarket.DoMarketClose();
		}
		break;

	case NET_MSG_GCTRL_PMARKET_ITEM_UPDATE_BRD:
		{
			GLMSG::SNETPC_PMARKET_ITEM_UPDATE_BRD *pNetMsg = (GLMSG::SNETPC_PMARKET_ITEM_UPDATE_BRD *) nmg;

			SSALEITEM *pSALEITEM = m_sPMarket.GetItem ( pNetMsg->sSALEPOS );
			if ( pSALEITEM )
			{
				pSALEITEM->bSOLD = pNetMsg->bSOLD;
				pSALEITEM->dwNUMBER = pNetMsg->dwNUMBER;
				pSALEITEM->sITEMCUSTOM.wTurnNum = (WORD) pNetMsg->dwNUMBER;

				SINVENITEM *pINVENITEM = m_sPMarket.GetInven().GetItem(pNetMsg->sSALEPOS.wMainID,pNetMsg->sSALEPOS.wSubID);
				if ( pINVENITEM )
				{
					pINVENITEM->sItemCustom.wTurnNum = (WORD) pNetMsg->dwNUMBER;
				}
			}
		}
		break;

	case NET_MSG_GCTRL_PMARKET_ITEM_INFO_BRD:
		{
			GLMSG::SNETPC_PMARKET_ITEM_INFO_BRD *pNetNsg = (GLMSG::SNETPC_PMARKET_ITEM_INFO_BRD *)nmg;

			SINVENITEM sINVENITEM;
			sINVENITEM.sItemCustom = pNetNsg->sITEMCUSTOM;
			sINVENITEM.wPosX = pNetNsg->sINVENPOS.wMainID;
			sINVENITEM.wPosY = pNetNsg->sINVENPOS.wSubID;
			m_sPMarket.RegItem ( sINVENITEM, pNetNsg->llPRICE, pNetNsg->dwNUMBER, pNetNsg->sSALEPOS );
			m_sPMarket.SetSaleState ( pNetNsg->sSALEPOS, pNetNsg->dwNUMBER, pNetNsg->bSOLD );
		}
		break;

	case NET_MSG_GCTRL_CLUB_INFO_BRD:
		{
			GLMSG::SNET_CLUB_INFO_BRD *pNetMsg = (GLMSG::SNET_CLUB_INFO_BRD *)nmg;

			m_CharData.dwGuild = pNetMsg->dwClubID;
			m_CharData.dwGuildMarkVer = pNetMsg->dwMarkVer;

			StringCchCopy ( m_CharData.szNick, CHAR_SZNAME, pNetMsg->szNickName );

			//	Note : 클럽마크 버전 확인.
			//
			GLGaeaClient::GetInstance().GetCharacter()->ReqClubMarkInfo ( m_CharData.dwGuild, m_CharData.dwGuildMarkVer );
		}
		break;

	case NET_MSG_GCTRL_CLUB_INFO_MARK_BRD:
		{
			GLMSG::SNET_CLUB_INFO_MARK_BRD *pNetMsg = (GLMSG::SNET_CLUB_INFO_MARK_BRD *)nmg;

			m_CharData.dwGuildMarkVer = pNetMsg->dwMarkVer;
			//	Note : 클럽마크 버전 확인.
			//
			GLGaeaClient::GetInstance().GetCharacter()->ReqClubMarkInfo ( m_CharData.dwGuild, m_CharData.dwGuildMarkVer );
		}
		break;

	case NET_MSG_GCTRL_CLUB_INFO_NICK_BRD:
		{
			GLMSG::SNET_CLUB_INFO_NICK_BRD *pNetMsg = (GLMSG::SNET_CLUB_INFO_NICK_BRD *)nmg;
			StringCchCopy ( m_CharData.szNick, CHAR_SZNAME, pNetMsg->szNickName );
		}
		break;

	case NET_MSG_GCTRL_CLUB_DEL_BRD:
		{
			m_CharData.dwGuild = CLUB_NULL;
			m_CharData.dwGuildMarkVer = 0;
			StringCchCopy ( m_CharData.szNick, CHAR_SZNAME, "" );
		}
		break;
	case NET_MSG_GCTRL_CLUB_AUTHORITY_BRD:
		{
			GLMSG::SNET_CLUB_AUTHORITY_BRD *pNetMsg = (GLMSG::SNET_CLUB_AUTHORITY_BRD *)nmg;
			m_CharData.dwGuildMaster = pNetMsg->dwCharID;
		}
		break;
		/*dmk14 remove active buff selection*/
	case NET_MSG_GCTRL_REQ_DISABLESKILLEFF_BRD:
		{
			GLMSG::SNETPC_REQ_DISABLESKILLEFF_BRD *pNetMsg = (GLMSG::SNETPC_REQ_DISABLESKILLEFF_BRD *)nmg;
			FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT[pNetMsg->dwSKILL].sNATIVEID );
			DISABLESKEFF(pNetMsg->dwSKILL);
		}
		break;
	case NET_MSG_GCTRL_REQ_DISABLEITEMFACT_BRD:
		{
			GLMSG::SNETPC_REQ_DISABLEITEMFACT_BRD *pNetMsg = (GLMSG::SNETPC_REQ_DISABLEITEMFACT_BRD *)nmg;
			m_sFITEMFACT[pNetMsg->dwSKILL].RESET();
		}
		break;
	case NET_MSG_GCTRL_INVEN_HAIR_CHANGE_BRD:
		{
			GLMSG::SNETPC_INVEN_HAIR_CHANGE_BRD *pNetMsg = (GLMSG::SNETPC_INVEN_HAIR_CHANGE_BRD *)nmg;
			
			m_CharData.wHair = (WORD) pNetMsg->dwID;
			UpdateSuit();
		}
		break;

	case NET_MSG_GCTRL_INVEN_HAIRCOLOR_CHANGE_BRD:
		{
			GLMSG::SNETPC_INVEN_HAIRCOLOR_CHANGE_BRD *pNetMsg = (GLMSG::SNETPC_INVEN_HAIRCOLOR_CHANGE_BRD*)nmg;

			m_CharData.wHairColor = pNetMsg->wHairColor;
			UpdateSuit();
		}
		break;

	case NET_MSG_GCTRL_INVEN_FACE_CHANGE_BRD:
		{
			GLMSG::SNETPC_INVEN_FACE_CHANGE_BRD *pNetMsg = (GLMSG::SNETPC_INVEN_FACE_CHANGE_BRD *)nmg;
			
			m_CharData.wFace = (WORD) pNetMsg->dwID;
			UpdateSuit();
		}
		break;

	case NET_MSG_GCTRL_INVEN_RENAME_BRD:
		{
			GLMSG::SNETPC_INVEN_RENAME_BRD *pNetMsg = (GLMSG::SNETPC_INVEN_RENAME_BRD *)nmg;

			StringCchCopy ( m_CharData.szName, CHAR_SZNAME, pNetMsg->szName );
		}
		break;

	case NET_MSG_GCTRL_REQ_GESTURE_BRD:
		{
			GLMSG::SNETPC_REQ_GESTURE_BRD *pNetMsg = (GLMSG::SNETPC_REQ_GESTURE_BRD *)nmg;
	
			//	Note : 모션 종류가 평화 모드일 경우만 제스쳐를 할 수 있다.
			PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();
			BOOL bPeaceZone = pLand ? pLand->IsPeaceZone() : FALSE;
			if ( !bPeaceZone && IsSTATE(EM_ACT_PEACEMODE) )		bPeaceZone = TRUE;

			if ( !bPeaceZone )		return;

			//	Note : 해당 제스쳐 에니메이션이 존제시 구동.
			PANIMCONTNODE pNODE = m_pSkinChar->GETANI ( AN_GESTURE, EMANI_SUBTYPE(pNetMsg->dwID) );
			if ( !pNODE )			return;

			//	Note : 신체가 정상적일때 구동.
			if ( !IsValidBody() )	return;

			//	Note : 제스쳐를 행함.
			m_dwANISUBGESTURE = (DWORD) pNetMsg->dwID;
			TurnAction(GLAT_TALK);
		}
		break;

	case NET_MSG_GCTRL_QITEMFACT_BRD:
		{
			GLMSG::SNETPC_QITEMFACT_BRD *pNetMsg = (GLMSG::SNETPC_QITEMFACT_BRD *)nmg;
			if ( m_bVehicle ) break;
		
			m_CharData.sQITEMFACT.emType = pNetMsg->sFACT.emType;
			m_CharData.sQITEMFACT.fTime = pNetMsg->sFACT.fTime;
			m_CharData.sQITEMFACT.wParam1 = pNetMsg->sFACT.wParam1;
			m_CharData.sQITEMFACT.wParam2 = pNetMsg->sFACT.wParam2;
		}
		break;

	case NET_MSG_GCTRL_QITEMFACT_END_BRD:
		{
			GLMSG::SNETPC_QITEMFACT_END_BRD *pNetMsg = (GLMSG::SNETPC_QITEMFACT_END_BRD *)nmg;

			m_CharData.sQITEMFACT.RESET ();
		}
		break;

	case NET_MSG_GCTRL_PKCOMBO_BRD:
		{
			GLMSG::SNETPC_PKCOMBO_BRD *pNetMsg = (GLMSG::SNETPC_PKCOMBO_BRD *)nmg;

			if ( m_bVehicle ) break;

			m_CharData.sPKCOMBOCOUNT.bShow = pNetMsg->sCOMBO.bShow;
			m_CharData.sPKCOMBOCOUNT.fTime = pNetMsg->sCOMBO.fTime;
			m_CharData.sPKCOMBOCOUNT.nCount = pNetMsg->sCOMBO.nCount;
		}
		break;

	case NET_MSG_GCTRL_PKCOMBO_END_BRD:
		{
			GLMSG::SNETPC_PKCOMBO_END_BRD *pNetMsg = (GLMSG::SNETPC_PKCOMBO_END_BRD *)nmg;

			m_CharData.sPKCOMBOCOUNT.RESET();
		}
		break;

	case NET_MSG_GCTRL_EVENTFACT_BRD:
		{
			GLMSG::SNETPC_EVENTFACT_BRD *pNetMsg = (GLMSG::SNETPC_EVENTFACT_BRD *)nmg;

			m_CharData.sEVENTFACT.SetEVENT( pNetMsg->emType, pNetMsg->wValue );
		}
		break;

	case NET_MSG_GCTRL_EVENTFACT_END_BRD:
		{
			GLMSG::SNETPC_EVENTFACT_END_BRD *pNetMsg = (GLMSG::SNETPC_EVENTFACT_END_BRD *)nmg;

			m_CharData.sEVENTFACT.ResetEVENT( pNetMsg->emType );
		}
		break;
	case NET_MSG_GCTRL_ACTIVE_VEHICLE_BRD:
		{
			GLMSG::SNETPC_ACTIVE_VEHICLE_BRD* pNetMsg = (GLMSG::SNETPC_ACTIVE_VEHICLE_BRD*) nmg;

			if ( pNetMsg->bActive )
			{
				if ( m_bVehicle )	return;
				m_bVehicle = TRUE;

				/*vehicle system, Juver, 2017/08/07 */
				int emType = m_sVehicle.m_emTYPE ;
				if ( emType == VEHICLE_TYPE_BOARD )
				{
					m_emANISUBTYPE = (EMANI_SUBTYPE) ( AN_SUB_HOVERBOARD );
				}else{
					m_emANISUBTYPE = (EMANI_SUBTYPE) ( AN_SUB_NONE );
				}

				SetSTATE(EM_ACT_PEACEMODE);
				/*
				// 버프 스킬 제거
				for ( int i = 0; i < SKILLFACT_SIZE; ++i )
				{
					if ( m_sSKILLFACT[i].sNATIVEID == NATIVEID_NULL() ) continue;
					
					PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( m_sSKILLFACT[i].sNATIVEID );

					if ( pSkill && pSkill->m_sBASIC.emIMPACT_SIDE != SIDE_ENEMY )
					{
						FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT[i].sNATIVEID );
						DISABLESKEFF( i );
					}	
				}*/

				// 탑승 이팩트 추가
				STARGETID sTargetID(CROW_PC, m_dwGaeaID, m_vPos);
				DxEffGroupPlayer::GetInstance().NewEffGroup
				(
					GLCONST_CHAR::strVEHICLE_GEN_EFFECT.c_str(),
					m_matTrans,
					&sTargetID
				);

				// 퀘션 아이템 제거
				//m_CharData.sQITEMFACT.RESET ();				
				
				UpdateSuit( );				
				ReSelectAnimation();				
				UPDATE_ITEM ();

			}
			else
			{
				if ( !m_bVehicle )	return;
				m_bVehicle = FALSE;		
				EMSLOT emRHand = GetCurRHand();
				EMSLOT emLHand = GetCurLHand();

				SITEM* pRHAND = NULL;
				SITEM* pLHAND = NULL;

				if ( m_CharData.m_PutOnItems[emRHand].sNativeID!=NATIVEID_NULL() )
				pRHAND = GLItemMan::GetInstance().GetItem ( m_CharData.m_PutOnItems[emRHand].sNativeID );
	
				if ( m_CharData.m_PutOnItems[emLHand].sNativeID!=NATIVEID_NULL() )
				pLHAND = GLItemMan::GetInstance().GetItem ( m_CharData.m_PutOnItems[emLHand].sNativeID );

				m_emANISUBTYPE = CHECK_ANISUB ( pRHAND, pLHAND  );

				/*
				// 버프스킬 제거
				for ( int i = 0; i < SKILLFACT_SIZE; ++i )
				{
					if ( m_sSKILLFACT[i].sNATIVEID == NATIVEID_NULL() ) continue;
					
					PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( m_sSKILLFACT[i].sNATIVEID );

					if ( pSkill && pSkill->m_sBASIC.emIMPACT_SIDE != SIDE_ENEMY )
					{
						DISABLESKEFF( i );
						FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT[i].sNATIVEID );
					}	
				}
				*/

/*
				// 하차 이펙트 추가
				STARGETID sTargetID(CROW_PC, m_dwGaeaID, m_vPos);
				DxEffGroupPlayer::GetInstance().NewEffGroup
				(
					GLCONST_CHAR::strVEHICLE_GEN_EFFECT.c_str(),
					m_matTrans,
					&sTargetID
				);
*/
				// 퀘션 아이템 제거
				//m_CharData.sQITEMFACT.RESET ();		

				UpdateSuit();				
				ReSelectAnimation();				
				UPDATE_ITEM();
			}

		}
		break;

	case NET_MSG_GCTRL_GET_VEHICLE_BRD:
		{
			GLMSG::SNETPC_GET_VEHICLE_BRD* pNetMsg = (GLMSG::SNETPC_GET_VEHICLE_BRD*) nmg;
			
			m_sVehicle.m_dwGUID = pNetMsg->dwGUID;
			m_sVehicle.m_emTYPE = pNetMsg->emTYPE;
			m_sVehicle.m_sVehicleID = pNetMsg->sVehicleID;

			for ( int i = 0; i < VEHICLE_ACCETYPE_SIZE; ++i )
			{
				m_sVehicle.m_PutOnItems[i] = pNetMsg->PutOnItems[i];
			}
			
			/*bike color , Juver, 2017/11/13 */
			for( WORD i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
				m_sVehicle.m_wColor[i] = pNetMsg->wColor[i];

			m_sVehicle.SetActiveValue( true );
			
			m_sVehicle.ITEM_UPDATE();

			UpdateSuit();				
			UPDATE_ITEM();
		}
		break;
	case NET_MSG_GCTRL_UNGET_VEHICLE_BRD:
		{
			GLMSG::SNETPC_UNGET_VEHICLE_BRD* pNetMsg = (GLMSG::SNETPC_UNGET_VEHICLE_BRD*) nmg;			
			m_sVehicle.SetActiveValue( false );	
			m_sVehicle.RESET();
		}
		break;
	case NET_MSG_VEHICLE_ACCESSORY_DELETE_BRD:
		{
			GLMSG::SNET_VEHICLE_ACCESSORY_DELETE_BRD* pNetMsg = (GLMSG::SNET_VEHICLE_ACCESSORY_DELETE_BRD*) nmg;
			
			if ( m_sVehicle.m_dwGUID == pNetMsg->dwVehicleNum )
			{
                m_sVehicle.m_PutOnItems[pNetMsg->accetype] = SITEMCUSTOM ();			
				m_sVehicle.ITEM_UPDATE();
				UpdateSuit();
				UPDATE_ITEM();
			}
		}
		break;
	case NET_MSG_VEHICLE_REQ_SLOT_EX_HOLD_BRD:
		{
			GLMSG::SNET_VEHICLE_REQ_SLOT_EX_HOLD_BRD* pNetMsg = (GLMSG::SNET_VEHICLE_REQ_SLOT_EX_HOLD_BRD*) nmg;

			// 손에든 아이템을
			m_sVehicle.SetSlotItem ( pNetMsg->emSuit, pNetMsg->sItemCustom );

			UpdateSuit();
			UPDATE_ITEM();
		}
		break;
	case NET_MSG_VEHICLE_REMOVE_SLOTITEM_BRD:
		{
			GLMSG::SNET_VEHICLE_REQ_REMOVE_SLOTITEM_BRD* pNetMsg = ( GLMSG::SNET_VEHICLE_REQ_REMOVE_SLOTITEM_BRD* ) nmg;
	
			// 슬롯아이템 제거
			m_sVehicle.ReSetSlotItem ( pNetMsg->emSuit );

			UpdateSuit();
			UPDATE_ITEM();
		}
		break;
	case NET_MSG_GCTRL_ITEMSHOPOPEN_BRD:
		{
#if defined ( JP_PARAM ) || defined ( _RELEASED)	// JAPAN Item Shop
			
			GLMSG::SNETPC_OPEN_ITEMSHOP_BRD* pNetMsg = ( GLMSG::SNETPC_OPEN_ITEMSHOP_BRD* ) nmg;
			m_bItemShopOpen = pNetMsg->bOpen;

			if ( m_bItemShopOpen )	CInnerInterface::GetInstance().GetItemShopIconMan()->ADD_SHOP_ICON( m_dwGaeaID );
			else	CInnerInterface::GetInstance().GetItemShopIconMan()->DEL_SHOP_ICON( m_dwGaeaID );	
#endif
		}
		break;
	case NET_MSG_REQ_GATHERING_BRD:
		{
			GLMSG::SNETPC_REQ_GATHERING_BRD *pNetMsg = (GLMSG::SNETPC_REQ_GATHERING_BRD *) nmg;

			//	Note : 공격 수행.
			m_dwANISUBGESTURE = pNetMsg->dwAniSel;
			TurnAction ( GLAT_GATHERING );
		}
		break;
	case NET_MSG_REQ_GATHERING_CANCEL_BRD:
		{
			if ( IsACTION(GLAT_GATHERING) )	TurnAction ( GLAT_IDLE );
		}
		break;
	case NET_MSG_REQ_GATHERING_RESULT_BRD:
		{
			GLMSG::SNETPC_REQ_GATHERING_RESULT_BRD* pNetMsg = ( GLMSG::SNETPC_REQ_GATHERING_RESULT_BRD* ) nmg;
			
			switch ( pNetMsg->emFB )
			{
			case EMREQ_GATHER_RESULT_SUCCESS:
			case EMREQ_GATHER_RESULT_SUCCESS_EX:
				{
					m_dwANISUBGESTURE = 0;
				}
				break;
			case EMREQ_GATHER_RESULT_FAIL:
			case EMREQ_GATHER_RESULT_ITEMFAIL:
				{
					m_dwANISUBGESTURE = 1;
				}
				break;
			}
		}
		break;

		/*itemfood system, Juver, 2017/05/25 */
	case NET_MSG_GCTRL_FITEMFACT_BRD:
		{
			GLMSG::SNETPC_FITEMFACT_BRD *pNetMsg = (GLMSG::SNETPC_FITEMFACT_BRD *)nmg;
			if ( pNetMsg->sidSkill != NATIVEID_NULL() && pNetMsg->wSLOT != FITEMFACT_SIZE )
			{
				m_CharData.sFITEMFACT[pNetMsg->wSLOT].Assign( pNetMsg->sFACT, pNetMsg->wSLOT );
				SDROP_FITEMFACT sFITEMFACT = m_CharData.sFITEMFACT[pNetMsg->wSLOT];
				ItemFoodAdd ( sFITEMFACT.sNATIVEID, sFITEMFACT.wLEVEL, sFITEMFACT.wSLOT, sFITEMFACT.fAGE );
			}
		}break;

		/*vehicle booster system, Juver, 2017/08/11 */
	case NET_MSG_GCTRL_VEHICLE_BOOSTER_STATE_START_BRD:
		{
			GLMSG::SNETPC_REQ_VEHICLE_BOOSTER_STATE_START_BRD *pNetMsg = (GLMSG::SNETPC_REQ_VEHICLE_BOOSTER_STATE_START_BRD *)nmg;
			m_CharData.m_bBoosterStart = pNetMsg->bStart;

			if ( !m_CharData.m_bBoosterStart && IsSTATE( EM_ACT_VEHICLE_BOOSTER ) )	ReSetSTATE( EM_ACT_VEHICLE_BOOSTER );

			//CInnerInterface::GetInstance().PrintMsgTextDlg ( 0xffff0000, "client set booster %d %d", m_CharData.m_bBoosterStart, m_dwGaeaID );
		}break;

		/*vehicle booster system, Juver, 2017/08/11 */
	case NET_MSG_GCTRL_VEHICLE_BOOSTER_STATE_RESET_BRD:
		{
			GLMSG::SNETPC_REQ_VEHICLE_BOOSTER_STATE_RESET_BRD *pNetMsg = (GLMSG::SNETPC_REQ_VEHICLE_BOOSTER_STATE_RESET_BRD *)nmg;
			m_CharData.m_bBoosterStart = false;

			if ( IsSTATE( EM_ACT_VEHICLE_BOOSTER ) )	ReSetSTATE( EM_ACT_VEHICLE_BOOSTER );

			//CInnerInterface::GetInstance().PrintMsgTextDlg ( 0xffff0000, "client reset booster %d", m_dwGaeaID );
		}break;

		/*system buffs, Juver, 2017/09/05 */
	case NET_MSG_GCTRL_SYSTEM_BUFF_BRD:
		{
			GLMSG::SNETPC_SYSTEMBUFF_BRD *pNetMsg = (GLMSG::SNETPC_SYSTEMBUFF_BRD *)nmg;
			for( int i=0; i<SYSTEM_BUFF_SIZE; ++i )
			{
				SystemBuffAdd( pNetMsg->sSYSTEM_BUFF[i].sNATIVEID, pNetMsg->sSYSTEM_BUFF[i].wLEVEL, pNetMsg->sSYSTEM_BUFF[i].wSLOT );
			}
		}break;

		/*activity system, Juver, 2017/11/01 */
	case NET_MSG_GCTRL_ACTIVITY_COMPLETE_BRD:
		{
			D3DXMATRIX matEffect;
			D3DXMatrixTranslation ( &matEffect, m_vPos.x, m_vPos.y, m_vPos.z );

			STARGETID sTargetID(CROW_PC,m_dwGaeaID,m_vPos);
			DxEffGroupPlayer::GetInstance().NewEffGroup( GLCONST_CHAR::strCOMPLETE_ACTIVITY_EFFECT.c_str(), matEffect, &sTargetID );
		}break;

		/*activity system, Juver, 2017/11/05 */
	case NET_MSG_GCTRL_CHARACTER_BADGE_CHANGE_BRD:
		{
			GLMSG::SNETPC_REQ_CHARACTER_BADGE_CHANGE_BRD *pNetMsg = (GLMSG::SNETPC_REQ_CHARACTER_BADGE_CHANGE_BRD *)nmg;
			StringCchCopy ( m_CharData.szBadge, CHAR_SZNAME, pNetMsg->szBadge );
		}break;

		/*bike color , Juver, 2017/11/13 */
	case NET_MSG_VEHICLE_REQ_CHANGE_COLOR_BRD:
		{
			GLMSG::SNET_VEHICLE_REQ_CHANGE_COLOR_BRD *pNetMsg = (GLMSG::SNET_VEHICLE_REQ_CHANGE_COLOR_BRD*)nmg;

			if ( m_bVehicle && ( m_sVehicle.m_dwGUID == pNetMsg->dwVehicleID || m_CharData.m_sVehicle.m_dwGUID == pNetMsg->dwVehicleID ))
			{
				for( WORD i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
				{
					m_sVehicle.m_wColor[i] = pNetMsg->wColor[i];
				}

				UpdateSuit();
			}
		}break;

		/*change scale card, Juver, 2018/01/04 */
	case NET_MSG_GCTRL_INVEN_SCALE_CHANGE_BRD:
		{
			GLMSG::SNETPC_INVENSCALE_CHANGE_BRD *pNetMsg = (GLMSG::SNETPC_INVENSCALE_CHANGE_BRD*)nmg;

			m_CharData.m_fScaleRange = pNetMsg->fScale;
		}break;

		/*item color, Juver, 2018/01/10 */
	case NET_MSG_GCTRL_INVEN_ITEMCOLOR_CHANGE_BRD:
		{
			GLMSG::SNETPC_INVEN_ITEMCOLOR_CHANGE_BRD *pNetMsg = (GLMSG::SNETPC_INVEN_ITEMCOLOR_CHANGE_BRD*)nmg;
			m_CharData.m_PutOnItems[pNetMsg->emSlot].wColor1 = pNetMsg->wColor1;
			m_CharData.m_PutOnItems[pNetMsg->emSlot].wColor2 = pNetMsg->wColor2;
			UpdateSuit();
		}break;

		/*pvp capture the flag, Juver, 2018/01/30 */
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_PLAYER_TEAM_BRD:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_PLAYER_TEAM_BRD* pNetMsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_PLAYER_TEAM_BRD* ) nmg;
			m_CharData.m_wCaptureTheFlagTeam = pNetMsg->wTeam;
		}break;

		/*pvp capture the flag, Juver, 2018/02/07 */
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_FLAG_HOLD_BRD:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_PLAYER_FLAG_HOLD* pNetMsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_PLAYER_FLAG_HOLD* ) nmg;
			m_CharData.m_bCaptureTheFlagHoldFlag = pNetMsg->bHold;

			D3DXMATRIX matEffect;
			D3DXMatrixTranslation ( &matEffect, m_vPos.x, m_vPos.y, m_vPos.z );

			for ( int i=0; i< CAPTURE_THE_FLAG_TEAM_SIZE; ++ i )
				DxEffGroupPlayer::GetInstance().DeletePassiveEffect( GLCONST_CHAR::strCaptureTheFlagEffect[i].c_str(), STARGETID( CROW_PC, m_dwGaeaID, m_vPos ) );

			if ( m_CharData.m_wCaptureTheFlagTeam < CAPTURE_THE_FLAG_TEAM_SIZE && m_CharData.m_bCaptureTheFlagHoldFlag )
			{
				if ( m_CharData.m_wCaptureTheFlagTeam == CAPTURE_THE_FLAG_TEAM_A )
					DxEffGroupPlayer::GetInstance().PassiveEffect( GLCONST_CHAR::strCaptureTheFlagEffect[CAPTURE_THE_FLAG_TEAM_B].c_str(), matEffect, STARGETID( CROW_PC, m_dwGaeaID, m_vPos ) );
				else if ( m_CharData.m_wCaptureTheFlagTeam == CAPTURE_THE_FLAG_TEAM_B )
					DxEffGroupPlayer::GetInstance().PassiveEffect( GLCONST_CHAR::strCaptureTheFlagEffect[CAPTURE_THE_FLAG_TEAM_A].c_str(), matEffect, STARGETID( CROW_PC, m_dwGaeaID, m_vPos ) );
			}
		}break;

		/* car, cart color, Juver, 2018/02/14 */
	case NET_MSG_VEHICLE_REQ_CHANGE_CAR_COLOR_BRD:
		{
			GLMSG::SNET_VEHICLE_REQ_CHANGE_CAR_COLOR_BRD *pNetMsg = (GLMSG::SNET_VEHICLE_REQ_CHANGE_CAR_COLOR_BRD*)nmg;

			if ( m_bVehicle && ( m_sVehicle.m_dwGUID == pNetMsg->dwVehicleID || m_CharData.m_sVehicle.m_dwGUID == pNetMsg->dwVehicleID ))
			{
				m_sVehicle.m_wColor[BIKE_COLOR_SLOT_PART_A1] = pNetMsg->wColorA;
				m_sVehicle.m_wColor[BIKE_COLOR_SLOT_PART_A2] = pNetMsg->wColorB;
				UpdateSuit();
			}
		}break;

	//Teleport Skill Effect
	case NET_MSG_GCTRL_TELEPORT_BRD:
		{
			GLMSG::SNET_TELEPORT_BRD *pNetMsg = (GLMSG::SNET_TELEPORT_BRD *)nmg;
			const D3DXVECTOR3 &vMovePos = pNetMsg->vMovePos;

			SetPosition(pNetMsg->vMovePos);
		}
		break;
	};
}

void GLCharClient::StartAttackProc ()
{
	m_nattSTEP = 0;
	m_fattTIMER = 0.0f;
}

void GLCharClient::AttackEffect ( const SANIMSTRIKE &sStrikeEff )
{
	BOOL bOk = GLGaeaClient::GetInstance().ValidCheckTarget ( m_sTargetID );
	if ( !bOk )	return;

	GLCOPY* pActor = GLGaeaClient::GetInstance().GetCopyActor ( m_sTargetID );
	if ( pActor )	pActor->ReceiveSwing ();

	/*ABL system, Juver, 2017/06/02 */
	if ( m_pSkinChar->m_pAttBone[sStrikeEff.m_emPiece] )	
		m_pSkinChar->m_pAttBone[sStrikeEff.m_emPiece]->SELECTANI( AN_ABL_ATTACK, AN_SUB_NONE );

	//	타겟의 위치.
	D3DXVECTOR3 vTARPOS = GLGaeaClient::GetInstance().GetTargetPos ( m_sTargetID );

	D3DXVECTOR3 vDir = vTARPOS - m_vPos;
	vDir.y = 0.f;
	D3DXVec3Normalize ( &vDir, &vDir );

	vTARPOS.y += 15.0f;

	D3DXVECTOR3 vTarDir = vTARPOS - m_vPos;
	float fLength = D3DXVec3Length ( &vTarDir );

	D3DXVec3Normalize ( &vTarDir, &vTarDir );
	D3DXVECTOR3 vTARPAR = m_vPos + vTarDir*fLength * 10.0f;

	/*ABL system, Juver, 2017/06/02 */
	D3DXVECTOR3 vOUT(0.0f,0.0f,0.0f);
	bOk = STRIKE_SELECT::SelectStrike( m_pSkinChar, vTARPAR, vOUT, sStrikeEff );
	if ( !bOk )	return;

	EMSLOT emSlot = PIECE_2_SLOT(sStrikeEff.m_emPiece);
	if( emSlot==SLOT_TSIZE )		return;
	if( emSlot==SLOT_RHAND )		emSlot = GetCurRHand();
	else if( emSlot==SLOT_LHAND )	emSlot = GetCurLHand();

	D3DXMATRIX matEffect;
	D3DXMatrixTranslation ( &matEffect, vOUT.x, vOUT.y, vOUT.z );

	STARGETID sTargetID = m_sTargetID;
	sTargetID.vPos = vTARPOS;

	//	Note : 무기의 속성 살펴봄.
	//
	EMELEMENT emELEMENT = GET_ITEM_ELMT();

	if ( VALID_SLOT_ITEM(emSlot) )
	{
		SITEM *pItem = GLItemMan::GetInstance().GetItem(GET_SLOT_ITEM(emSlot).sNativeID);
		if ( pItem )
		{
			if ( pItem->sSuitOp.emAttack <= ITEMATT_NEAR )
			{
				std::string strEFFECT = pItem->GetTargetEffect();
				if ( strEFFECT.empty() )	strEFFECT = GLCONST_CHAR::GETSTRIKE_EFFECT(emELEMENT);

				DxEffGroupPlayer::GetInstance().NewEffGroup
				(
					strEFFECT.c_str(),
					matEffect,
					&sTargetID
				);

				DxEffGroupPlayer::GetInstance().NewEffBody ( GLCONST_CHAR::strAMBIENT_EFFECT.c_str(), &sTargetID, &vDir );
			}
			else
			{
				if ( pItem->sSuitOp.emAttack == ITEMATT_BOW )
				{
					SITEM *pLHAND = NULL;
					if ( VALID_SLOT_ITEM(SLOT_LHAND) )	pLHAND = GLItemMan::GetInstance().GetItem(GET_SLOT_ITEM(SLOT_LHAND).sNativeID);
					if ( pLHAND && pLHAND->sBasicOp.emItemType == ITEM_ARROW )
					{
						DxEffSingleGroup* pEffSingleG = DxEffGroupPlayer::GetInstance().NewEffGroup
						(
							pLHAND->GetTargetEffect(),
							matEffect,
							&sTargetID
						);
						
						if ( pEffSingleG )
						{
							pEffSingleG->AddEffAfter ( EFFASINGLE, GLCONST_CHAR::GETSTRIKE_EFFECT(emELEMENT)  );
							pEffSingleG->AddEffAfter ( EFFABODY, std::string(pLHAND->GetTargBodyEffect()) );
							pEffSingleG->AddEffAfter ( EFFABODY, GLCONST_CHAR::strAMBIENT_EFFECT );
						}
					}
				}
				/*gun-bullet logic, Juver, 2017/05/27 */
				else if ( pItem->sSuitOp.emAttack == ITEMATT_GUN ||
					pItem->sSuitOp.emAttack == ITEMATT_RAILGUN ||
					pItem->sSuitOp.emAttack == ITEMATT_PORTALGUN )
				{
					SITEM *pLHAND = NULL;
					if ( VALID_SLOT_ITEM(SLOT_LHAND) )	pLHAND = GLItemMan::GetInstance().GetItem(GET_SLOT_ITEM(SLOT_LHAND).sNativeID);
					if ( pLHAND && pLHAND->sBasicOp.emItemType == ITEM_BULLET )
					{
						DxEffSingleGroup* pEffSingleG = DxEffGroupPlayer::GetInstance().NewEffGroup
							(
							pLHAND->GetTargetEffect(),
							matEffect,
							&sTargetID
							);

						if ( pEffSingleG )
						{
							pEffSingleG->AddEffAfter ( EFFASINGLE, GLCONST_CHAR::GETSTRIKE_EFFECT(emELEMENT)  );
							pEffSingleG->AddEffAfter ( EFFABODY, std::string(pLHAND->GetTargBodyEffect()) );
							pEffSingleG->AddEffAfter ( EFFABODY, GLCONST_CHAR::strAMBIENT_EFFECT );
						}
					}
				}
			}
		}
	}
	else
	{
		DxEffGroupPlayer::GetInstance().NewEffGroup
		(
			GLCONST_CHAR::GETSTRIKE_EFFECT(emELEMENT),
			matEffect,
			&sTargetID
		);

		DxEffGroupPlayer::GetInstance().NewEffBody ( GLCONST_CHAR::strAMBIENT_EFFECT.c_str(), &sTargetID, &vDir );
	}
}

BOOL GLCharClient::AttackProc ( float fElapsedTime )
{
	m_fattTIMER += fElapsedTime * GETATTVELO();
	m_fattTIMER += GETATT_ITEM();

	GASSERT(m_pSkinChar->GETCURANIM());

	PSANIMCONTAINER pAnimCont = m_pSkinChar->GETCURANIM();

	if ( pAnimCont->m_wStrikeCount == 0 )	return FALSE;

	DWORD dwThisKey = DWORD(m_fattTIMER*UNITANIKEY_PERSEC);

	DWORD dwStrikeKey = pAnimCont->m_sStrikeEff[m_nattSTEP].m_dwFrame;
	if ( m_nattSTEP < pAnimCont->m_wStrikeCount )
	{
		if ( dwThisKey>=dwStrikeKey )
		{
			if ( GLGaeaClient::GetInstance().IsVisibleCV(m_sTargetID) )
			{
				//	Note : 공격시 이팩트 발휘..
				//
				AttackEffect ( pAnimCont->m_sStrikeEff[m_nattSTEP] );
			}

			m_nattSTEP++;
		}
	}

	return TRUE;
}

void GLCharClient::StartSkillProc ()
{
	m_nattSTEP = 0;
	m_fattTIMER = 0.0f;

	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_idACTIVESKILL.wMainID, m_idACTIVESKILL.wSubID );
	if ( !pSkill )							return;

	//	Note : 공격 방향으로 회전.
	//
	m_vDir = UpdateSkillDirection ( m_vPos, m_vDir, m_idACTIVESKILL, m_vTARPOS, m_sTARIDS );

	STARGETID sTARG(GETCROW(),m_dwGaeaID);

	/*skill effect setting, Juver, 2017/10/01 */
	if ( RANPARAM::bHideSkillEffect )	return;

	//	Note : 스킬 시작과 동시에 나타나는 이팩트. 발동.
	//
	EMELEMENT emELMT = ( pSkill->m_sAPPLY.emELEMENT==EMELEMENT_ARM ) ? GET_ITEM_ELMT() : (EMELEMENT_SPIRIT);
	if ( pSkill->m_sEXT_DATA.emSELFBODY==SKILL::EMTIME_FIRST )
		DxEffGroupPlayer::GetInstance().NewEffBody ( pSkill->m_sEXT_DATA.GETSELFBODY(emELMT), &sTARG, &m_vDir);

	EMSLOT emRHand = GetCurRHand();

	SANIMSTRIKE sStrike;
	sStrike.m_emPiece = VALID_SLOT_ITEM(emRHand) ? PIECE_RHAND : PIECE_GLOVE;
	sStrike.m_emEffect = EMSF_TARGET;
	sStrike.m_dwFrame = 0;

	if ( pSkill->m_sEXT_DATA.emSELFZONE01==SKILL::EMTIME_FIRST )
		SK_EFF_SELFZONE ( sStrike, pSkill->m_sEXT_DATA.emSELFZONE01_POS, pSkill->m_sEXT_DATA.GETSELFZONE01(emELMT), &sTARG );

	if ( pSkill->m_sEXT_DATA.emSELFZONE02==SKILL::EMTIME_FIRST )
		SK_EFF_SELFZONE ( sStrike, pSkill->m_sEXT_DATA.emSELFZONE02_POS, pSkill->m_sEXT_DATA.GETSELFZONE02(emELMT), &sTARG );

	if ( pSkill->m_sEXT_DATA.emSELFZONE03==SKILL::EMTIME_FIRST )
		SK_EFF_SELFZONE ( sStrike, pSkill->m_sEXT_DATA.emSELFZONE03_POS, pSkill->m_sEXT_DATA.GETSELFZONE03(emELMT), &sTARG );
}

void GLCharClient::SKT_EFF_HOLDOUT ( STARGETID sTarget, DWORD dwDamageFlag )
{
	for ( int i=0; i<SKILLFACT_SIZE; ++i )
	{
		SSKILLFACT &sSKEFF = m_sSKILLFACT[i];
		if ( sSKEFF.sNATIVEID==NATIVEID_NULL() )	continue;

		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sSKEFF.sNATIVEID.wMainID, sSKEFF.sNATIVEID.wSubID );
		if ( !pSkill )							continue;		

		if ( !pSkill->m_sEXT_DATA.strHOLDOUT.empty() )
		{

			if ( (pSkill->m_sAPPLY.IsSpec( EMSPECA_PSY_DAMAGE_REDUCE ) && 
				!( dwDamageFlag & DAMAGE_TYPE_PSY_REDUCE) ) )
			continue;
		
			if ( (pSkill->m_sAPPLY.IsSpec( EMSPECA_MAGIC_DAMAGE_REDUCE ) && 
				!( dwDamageFlag & DAMAGE_TYPE_MAGIC_REDUCE) ) )
				continue;

			if ( (pSkill->m_sAPPLY.IsSpec( EMSPECA_PSY_DAMAGE_REFLECTION ) && 
				!( dwDamageFlag & DAMAGE_TYPE_PSY_REFLECTION) ) )
				continue;

			if ( (pSkill->m_sAPPLY.IsSpec( EMSPECA_MAGIC_DAMAGE_REFLECTION ) && 
				!( dwDamageFlag & DAMAGE_TYPE_MAGIC_REFLECTION) ) )
				continue;



			const char* szZONE_EFF = pSkill->m_sEXT_DATA.strHOLDOUT.c_str();

			D3DXVECTOR3 vDIR = sTarget.vPos - m_vPos;

			D3DXVECTOR3 vDIR_ORG(1,0,0);
			float fdir_y = DXGetThetaYFromDirection ( vDIR, vDIR_ORG );

			D3DXMATRIX matTrans;
			D3DXMatrixRotationY ( &matTrans, fdir_y );
			matTrans._41 = m_vPos.x;
			matTrans._42 = m_vPos.y;
			matTrans._43 = m_vPos.z;

			//	Note : 자기 위치 이펙트 발생시킴.
			DxEffGroupPlayer::GetInstance().NewEffGroup ( szZONE_EFF, matTrans, &sTarget );
		}
	}
}

/*skill activated effect, Juver, 2017/12/29 */
DxEffSingleGroup* GLCharClient::SK_EFF_SELFZONE ( const SANIMSTRIKE &sStrikeEff, const SKILL::EMEFFECTPOS emPOS, const char* const szZONE_EFF, STARGETID *pTarget )
{
	D3DXMATRIX matTrans;

	//	스트라이크 지점에서.
	BOOL bTRANS(FALSE);
	if ( emPOS == SKILL::EMPOS_STRIKE )
	{
		//	타겟의 위치.
		D3DXVECTOR3 vTARPOS = m_vPos + m_vDir * 60.0f;

		/*ABL system, Juver, 2017/06/02 */
		BOOL bOk = FALSE;
		D3DXVECTOR3 vOUT(0.0f,0.0f,0.0f);
		bOk = STRIKE_SELECT::SelectStrike( m_pSkinChar, vTARPOS, vOUT, sStrikeEff );
		if ( bOk )
		{
			bTRANS = TRUE;

			D3DXMatrixTranslation ( &matTrans, vOUT.x, vOUT.y, vOUT.z );
		}
	}

	//	지면기준 목표점에 회전 속성 넣어서.
	if ( !bTRANS )
	{
		bTRANS = TRUE;

		D3DXMATRIX matYRot;
		D3DXMatrixRotationY ( &matYRot, D3DX_PI/2.0f );
		matTrans = matYRot * m_matTrans;
	}

	//	Note : 자기 위치 이펙트 발생시킴.
	return DxEffGroupPlayer::GetInstance().NewEffGroup ( szZONE_EFF, matTrans, pTarget );
}

/*skill activated effect, Juver, 2017/12/29 */
DxEffSingleGroup* GLCharClient::SK_EFF_TARZONE ( const STARGETID &_sTarget, const SKILL::EMEFFECTPOS emPOS, const char* const szZONE_EFF )
{
	D3DXMATRIX matTrans;
	STARGETID sTarget = _sTarget;

	//	스트라이크 지점에서.
	BOOL bTRANS(FALSE);
	if ( emPOS == SKILL::EMPOS_STRIKE )
	{
		DxSkinChar* pSkin = GLGaeaClient::GetInstance().GetSkinChar ( sTarget );

		//	타겟의 위치.
		D3DXVECTOR3 vTARPOS ( m_vPos.x, m_vPos.y+15, m_vPos.z );;

		/*ABL system, Juver, 2017/06/02 */
		D3DXVECTOR3 vOUT(0.0f,0.0f,0.0f);
		BOOL bOk = STRIKE_SELECT::SelectArrow( pSkin, vTARPOS, vOUT );
		if ( bOk )
		{
			bTRANS = TRUE;

			D3DXMatrixTranslation ( &matTrans, vOUT.x, vOUT.y, vOUT.z );
			sTarget.vPos = vTARPOS;
		}
	}

	//	지면기준 목표점.
	if ( !bTRANS )
	{
		bTRANS = TRUE;
		D3DXMatrixTranslation ( &matTrans, sTarget.vPos.x, sTarget.vPos.y, sTarget.vPos.z );
	}

	//	Note : 목표 위치 이펙트 발생시킴.
	return DxEffGroupPlayer::GetInstance().NewEffGroup ( szZONE_EFF, matTrans, &sTarget );
}

void GLCharClient::SK_EFF_TARG ( const PGLSKILL pSkill, const SANIMSTRIKE &sStrikeEff, const STARGETID &_sTarget )
{
	D3DXMATRIX matTrans;
	STARGETID sTarget = _sTarget;

	//	스트라이크 지점에서.
	BOOL bTRANS(FALSE);
	if ( pSkill->m_sEXT_DATA.emTARG_POSA == SKILL::EMPOS_STRIKE )
	{
		//	타겟의 위치.
		D3DXVECTOR3 vSTRPOS ( sTarget.vPos.x, sTarget.vPos.y+15, sTarget.vPos.z );;

		D3DXVECTOR3 vDir = vSTRPOS - m_vPos;
		float fLength = D3DXVec3Length ( &vDir );

		D3DXVec3Normalize ( &vDir, &vDir );
		D3DXVECTOR3 vSTRPAR = m_vPos + vDir*fLength * 10.0f;

		/*ABL system, Juver, 2017/06/02 */
		BOOL bOk = FALSE;
		D3DXVECTOR3 vOUT(0.0f,0.0f,0.0f);
		bOk = STRIKE_SELECT::SelectStrike( m_pSkinChar, vSTRPAR, vOUT, sStrikeEff );
		if ( bOk )
		{
			bTRANS = TRUE;
			D3DXMatrixTranslation ( &matTrans, vOUT.x, vOUT.y, vOUT.z );
		}
	}
	
	//	지면기준 회전 속성 넣어서.
	if ( !bTRANS )
	{
		bTRANS = TRUE;

		D3DXMATRIX matYRot;
		D3DXMatrixRotationY ( &matYRot, D3DX_PI/2.0f );
		matTrans = matYRot * m_matTrans;
	}

	//	목표 지점 선택
	BOOL bIMPACT(FALSE);
	if ( pSkill->m_sEXT_DATA.emTARG_POSB == SKILL::EMPOS_STRIKE )
	{
		DxSkinChar* pSkin = GLGaeaClient::GetInstance().GetSkinChar ( sTarget );
		if ( !pSkin )	return;

		//	타겟의 위치.
		D3DXVECTOR3 vTARPOS ( m_vPos.x, m_vPos.y+15, m_vPos.z );;

		//	타격 위치를 알아냄.
		D3DXVECTOR3 vOUT(0.0f,0.0f,0.0f);
		BOOL bOk = STRIKE_SELECT::SelectArrow( pSkin, vTARPOS, vOUT );
		if ( bOk )
		{
			bIMPACT = TRUE;
			sTarget.vPos = vOUT;
		}
	}

	if ( !bIMPACT )
	{
		sTarget.vPos;
	}

	EMELEMENT emELMT = ( pSkill->m_sAPPLY.emELEMENT==EMELEMENT_ARM ) ? GET_ITEM_ELMT() : (EMELEMENT_SPIRIT);
	DxEffSingleGroup* pEffSingleG = DxEffGroupPlayer::GetInstance().NewEffGroup ( pSkill->m_sEXT_DATA.GETTARG(emELMT), matTrans, &sTarget );
	if ( !pEffSingleG )		return;
	if( RANPARAM::bHideSkillEffect )	return;

	//	Note : 목표 위치 이펙트.
	if ( pSkill->m_sEXT_DATA.emTARGZONE01==SKILL::EMTIME_TARG_OVR )
		pEffSingleG->AddEffAfter ( EFFASINGLE, pSkill->m_sEXT_DATA.GETTARGZONE01(emELMT) );

	if ( pSkill->m_sEXT_DATA.emTARGZONE02==SKILL::EMTIME_TARG_OVR )
		pEffSingleG->AddEffAfter ( EFFASINGLE, pSkill->m_sEXT_DATA.GETTARGZONE02(emELMT) );

	if ( pSkill->m_sEXT_DATA.emTARGZONE03==SKILL::EMTIME_TARG_OVR )
		pEffSingleG->AddEffAfter ( EFFASINGLE, pSkill->m_sEXT_DATA.GETTARGZONE03(emELMT) );

	//	Note : 목표 몸 이펙트.
	if ( pSkill->m_sEXT_DATA.emTARGBODY01==SKILL::EMTIME_TARG_OVR )
		pEffSingleG->AddEffAfter ( EFFABODY, pSkill->m_sEXT_DATA.GETTARGBODY01(emELMT) );

	if ( pSkill->m_sEXT_DATA.emTARGBODY02==SKILL::EMTIME_TARG_OVR )
		pEffSingleG->AddEffAfter ( EFFABODY, pSkill->m_sEXT_DATA.GETTARGBODY02(emELMT) );
}

void GLCharClient::SkillEffect ( const SANIMSTRIKE &sStrikeEff )
{
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_idACTIVESKILL.wMainID, m_idACTIVESKILL.wSubID );
	if ( !pSkill )							return;

	/*skill effect setting, Juver, 2017/10/01 */
	if ( RANPARAM::bHideSkillEffect )	return;

	/*ABL system, Juver, 2017/06/02 */
	if ( sStrikeEff.m_emPiece == PIECE_RHAND || sStrikeEff.m_emPiece == PIECE_LHAND )
	{
		if ( m_pSkinChar->m_pAttBone[PIECE_RHAND] )	
			m_pSkinChar->m_pAttBone[PIECE_RHAND]->SELECTANI( AN_ABL_ATTACK, AN_SUB_NONE );

		if ( m_pSkinChar->m_pAttBone[PIECE_LHAND] )	
			m_pSkinChar->m_pAttBone[PIECE_LHAND]->SELECTANI( AN_ABL_ATTACK, AN_SUB_NONE );
	}
	else
	{
		if ( m_pSkinChar->m_pAttBone[sStrikeEff.m_emPiece] )	
			m_pSkinChar->m_pAttBone[sStrikeEff.m_emPiece]->SELECTANI( AN_ABL_ATTACK, AN_SUB_NONE );
	}

	D3DXMATRIX matTrans;
	using namespace SKILL;

	//	Note : 타겟 이펙트가 지면을 향해 발사되는 경우.
	//
	EMELEMENT emELMT = ( pSkill->m_sAPPLY.emELEMENT==EMELEMENT_ARM ) ? GET_ITEM_ELMT() : (EMELEMENT_SPIRIT);
	if ( pSkill->m_sBASIC.emIMPACT_TAR == TAR_ZONE )
	{
		if ( pSkill->m_sEXT_DATA.emTARG == EMTIME_IMPACT )
		{
			D3DXMatrixTranslation ( &matTrans, m_vTARPOS.x, m_vTARPOS.y, m_vTARPOS.z );
			DxEffSingleGroup* pEffSingleG = DxEffGroupPlayer::GetInstance().NewEffGroup ( pSkill->m_sEXT_DATA.GETSELFZONE01(emELMT), matTrans );
			if ( pEffSingleG )
			{
				if ( pSkill->m_sEXT_DATA.emTARGZONE01==EMTIME_TARG_OVR )
					pEffSingleG->AddEffAfter ( EFFASINGLE, pSkill->m_sEXT_DATA.GETTARGZONE01(emELMT) );

				if ( pSkill->m_sEXT_DATA.emTARGZONE02==EMTIME_TARG_OVR )
					pEffSingleG->AddEffAfter ( EFFASINGLE, pSkill->m_sEXT_DATA.GETTARGZONE02(emELMT) );

				if ( pSkill->m_sEXT_DATA.emTARGZONE03==EMTIME_TARG_OVR )
					pEffSingleG->AddEffAfter ( EFFASINGLE, pSkill->m_sEXT_DATA.GETTARGZONE03(emELMT) );
			}
		}

		if ( pSkill->m_sEXT_DATA.emTARGZONE01==EMTIME_IMPACT )
		{
			/*skill activated effect, Juver, 2017/12/29 */
			DxEffSingleGroup* pnew_group_effect = DxEffGroupPlayer::GetInstance().NewEffGroup ( pSkill->m_sEXT_DATA.GETTARGZONE01(emELMT), matTrans );
			if ( pnew_group_effect )
			{
				if ( pSkill->m_sEXT_DATA.emTARGZONE01==SKILL::EMTIME_EFFECT_ACTIVATED )
					DxEffGroupPlayer::GetInstance().NewEffGroup ( pSkill->m_sEXT_DATA.GETTARGZONE01(emELMT), matTrans );

				if ( pSkill->m_sEXT_DATA.emTARGZONE02==SKILL::EMTIME_EFFECT_ACTIVATED )
					DxEffGroupPlayer::GetInstance().NewEffGroup ( pSkill->m_sEXT_DATA.GETTARGZONE02(emELMT), matTrans );

				if ( pSkill->m_sEXT_DATA.emTARGZONE03==SKILL::EMTIME_EFFECT_ACTIVATED )
					DxEffGroupPlayer::GetInstance().NewEffGroup ( pSkill->m_sEXT_DATA.GETTARGZONE03(emELMT), matTrans );
			}
		}

		if ( pSkill->m_sEXT_DATA.emTARGZONE02==EMTIME_IMPACT )
		{
			/*skill activated effect, Juver, 2017/12/29 */
			DxEffSingleGroup* pnew_group_effect = DxEffGroupPlayer::GetInstance().NewEffGroup ( pSkill->m_sEXT_DATA.GETTARGZONE02(emELMT), matTrans );
			if ( pnew_group_effect )
			{
				if ( pSkill->m_sEXT_DATA.emTARGZONE01==SKILL::EMTIME_EFFECT_ACTIVATED )
					DxEffGroupPlayer::GetInstance().NewEffGroup ( pSkill->m_sEXT_DATA.GETTARGZONE01(emELMT), matTrans );

				if ( pSkill->m_sEXT_DATA.emTARGZONE02==SKILL::EMTIME_EFFECT_ACTIVATED )
					DxEffGroupPlayer::GetInstance().NewEffGroup ( pSkill->m_sEXT_DATA.GETTARGZONE02(emELMT), matTrans );

				if ( pSkill->m_sEXT_DATA.emTARGZONE03==SKILL::EMTIME_EFFECT_ACTIVATED )
					DxEffGroupPlayer::GetInstance().NewEffGroup ( pSkill->m_sEXT_DATA.GETTARGZONE03(emELMT), matTrans );
			}
		}

		if ( pSkill->m_sEXT_DATA.emTARGZONE03==EMTIME_IMPACT )
		{
			/*skill activated effect, Juver, 2017/12/29 */
			DxEffSingleGroup* pnew_group_effect = DxEffGroupPlayer::GetInstance().NewEffGroup ( pSkill->m_sEXT_DATA.GETTARGZONE03(emELMT), matTrans );
			if ( pnew_group_effect )
			{
				if ( pSkill->m_sEXT_DATA.emTARGZONE01==SKILL::EMTIME_EFFECT_ACTIVATED )
					DxEffGroupPlayer::GetInstance().NewEffGroup ( pSkill->m_sEXT_DATA.GETTARGZONE01(emELMT), matTrans );

				if ( pSkill->m_sEXT_DATA.emTARGZONE02==SKILL::EMTIME_EFFECT_ACTIVATED )
					DxEffGroupPlayer::GetInstance().NewEffGroup ( pSkill->m_sEXT_DATA.GETTARGZONE02(emELMT), matTrans );

				if ( pSkill->m_sEXT_DATA.emTARGZONE03==SKILL::EMTIME_EFFECT_ACTIVATED )
					DxEffGroupPlayer::GetInstance().NewEffGroup ( pSkill->m_sEXT_DATA.GETTARGZONE03(emELMT), matTrans );
			}
		}
	}
	//	Note : 타겟 이펙트가 지면을 향해 발사되는 경우.
	//		자기 위치에서 대상 위치까지. ( 화살 관통 )
	else if ( pSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF_TOSPEC )
	{
		if ( m_wTARNUM == 0 )	return;

		STARGETID sTarget(static_cast<EMCROW>(m_sTARIDS[m_wTARNUM-1].wCrow),static_cast<DWORD>(m_sTARIDS[m_wTARNUM-1].wID));
		if ( !GLGaeaClient::GetInstance().IsVisibleCV(sTarget) )		return;
		sTarget.vPos = GLGaeaClient::GetInstance().GetTargetPos(sTarget);

		D3DXVECTOR3 vDir = sTarget.vPos - m_vPos;
		vDir.y = 0.f;
		D3DXVec3Normalize ( &vDir, &vDir );

		//	Note : 목표 지향 이펙트에 실어 보내는 이펙트.
		//		( 실어 보낼때는 목표 위치 이펙트의 위치는 자동으로 지향 이팩트의 종료 지점으로 고정. )
		if ( pSkill->m_sEXT_DATA.VALIDTARG(emELMT) )
		{
			SK_EFF_TARG ( pSkill, sStrikeEff, sTarget );
		}
		//	단독으로 목표 지점에 발생.
		else
		{
			//	Note : 목표 위치 이펙트.
			if ( pSkill->m_sEXT_DATA.emTARGZONE01==EMTIME_IMPACT )
			{
				/*skill activated effect, Juver, 2017/12/29 */
				DxEffSingleGroup* pnew_group_effect = SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE01_POS, pSkill->m_sEXT_DATA.GETTARGZONE01(emELMT) );
				if ( pnew_group_effect )
				{
					if ( pSkill->m_sEXT_DATA.emTARGZONE01==SKILL::EMTIME_EFFECT_ACTIVATED )
						SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE01_POS, pSkill->m_sEXT_DATA.GETTARGZONE01(emELMT) );

					if ( pSkill->m_sEXT_DATA.emTARGZONE02==SKILL::EMTIME_EFFECT_ACTIVATED )
						SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE02_POS, pSkill->m_sEXT_DATA.GETTARGZONE02(emELMT) );

					if ( pSkill->m_sEXT_DATA.emTARGZONE03==SKILL::EMTIME_EFFECT_ACTIVATED )
						SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE03_POS, pSkill->m_sEXT_DATA.GETTARGZONE03(emELMT) );
				}
			}

			if ( pSkill->m_sEXT_DATA.emTARGZONE02==EMTIME_IMPACT )
			{
				/*skill activated effect, Juver, 2017/12/29 */
				DxEffSingleGroup* pnew_group_effect = SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE02_POS, pSkill->m_sEXT_DATA.GETTARGZONE02(emELMT) );
				if ( pnew_group_effect )
				{
					if ( pSkill->m_sEXT_DATA.emTARGZONE01==SKILL::EMTIME_EFFECT_ACTIVATED )
						SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE01_POS, pSkill->m_sEXT_DATA.GETTARGZONE01(emELMT) );

					if ( pSkill->m_sEXT_DATA.emTARGZONE02==SKILL::EMTIME_EFFECT_ACTIVATED )
						SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE02_POS, pSkill->m_sEXT_DATA.GETTARGZONE02(emELMT) );

					if ( pSkill->m_sEXT_DATA.emTARGZONE03==SKILL::EMTIME_EFFECT_ACTIVATED )
						SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE03_POS, pSkill->m_sEXT_DATA.GETTARGZONE03(emELMT) );
				}
			}

			if ( pSkill->m_sEXT_DATA.emTARGZONE03==EMTIME_IMPACT )
			{
				/*skill activated effect, Juver, 2017/12/29 */
				DxEffSingleGroup* pnew_group_effect = SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE03_POS, pSkill->m_sEXT_DATA.GETTARGZONE03(emELMT) );
				if ( pnew_group_effect )
				{
					if ( pSkill->m_sEXT_DATA.emTARGZONE01==SKILL::EMTIME_EFFECT_ACTIVATED )
						SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE01_POS, pSkill->m_sEXT_DATA.GETTARGZONE01(emELMT) );

					if ( pSkill->m_sEXT_DATA.emTARGZONE02==SKILL::EMTIME_EFFECT_ACTIVATED )
						SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE02_POS, pSkill->m_sEXT_DATA.GETTARGZONE02(emELMT) );

					if ( pSkill->m_sEXT_DATA.emTARGZONE03==SKILL::EMTIME_EFFECT_ACTIVATED )
						SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE03_POS, pSkill->m_sEXT_DATA.GETTARGZONE03(emELMT) );
				}
			}

			//	Note : 목표 몸 이펙트.
			if ( pSkill->m_sEXT_DATA.emTARGBODY01==EMTIME_IMPACT )
				DxEffGroupPlayer::GetInstance().NewEffBody ( pSkill->m_sEXT_DATA.GETTARGBODY01(emELMT), &sTarget, &vDir );
			if ( pSkill->m_sEXT_DATA.emTARGBODY02==EMTIME_IMPACT )
				DxEffGroupPlayer::GetInstance().NewEffBody ( pSkill->m_sEXT_DATA.GETTARGBODY02(emELMT), &sTarget, &vDir );
		}
	}
	//	Note : 타겟 이펙트가 유닛을 향해 발사되는 경우.
	//
	else
	{
		//	Note : 목표 유닛이 있는 경우.
		//
		if ( m_wTARNUM )
		{
			for ( WORD i=0; i<m_wTARNUM; ++i )
			{
				STARGETID sTarget(static_cast<EMCROW>(m_sTARIDS[i].wCrow),static_cast<DWORD>(m_sTARIDS[i].wID));
				if ( !GLGaeaClient::GetInstance().IsVisibleCV(sTarget) )	continue;
				sTarget.vPos = GLGaeaClient::GetInstance().GetTargetPos(sTarget);

				D3DXVECTOR3 vDir = sTarget.vPos - m_vPos;
				vDir.y = 0.f;
				D3DXVec3Normalize ( &vDir, &vDir );

				//	Note : 목표 지향 이펙트에 실어 보내는 이펙트.
				//		( 실어 보낼때는 목표 위치 이펙트의 위치는 자동으로 지향 이팩트의 종료 지점으로 고정. )
				if ( pSkill->m_sEXT_DATA.VALIDTARG(emELMT) )
				{
					SK_EFF_TARG ( pSkill, sStrikeEff, sTarget );
				}
				//	단독으로 목표 지점에 발생.
				else
				{
					//	Note : 목표 위치 이펙트.
					if ( pSkill->m_sEXT_DATA.emTARGZONE01==EMTIME_IMPACT )
					{
						/*skill activated effect, Juver, 2017/12/29 */
						DxEffSingleGroup* pnew_group_effect = SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE01_POS, pSkill->m_sEXT_DATA.GETTARGZONE01(emELMT) );
						if ( pnew_group_effect )
						{
							if ( pSkill->m_sEXT_DATA.emTARGZONE01==SKILL::EMTIME_EFFECT_ACTIVATED )
								SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE01_POS, pSkill->m_sEXT_DATA.GETTARGZONE01(emELMT) );

							if ( pSkill->m_sEXT_DATA.emTARGZONE02==SKILL::EMTIME_EFFECT_ACTIVATED )
								SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE02_POS, pSkill->m_sEXT_DATA.GETTARGZONE02(emELMT) );

							if ( pSkill->m_sEXT_DATA.emTARGZONE03==SKILL::EMTIME_EFFECT_ACTIVATED )
								SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE03_POS, pSkill->m_sEXT_DATA.GETTARGZONE03(emELMT) );
						}
					}

					if ( pSkill->m_sEXT_DATA.emTARGZONE02==EMTIME_IMPACT )
					{
						/*skill activated effect, Juver, 2017/12/29 */
						DxEffSingleGroup* pnew_group_effect = SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE02_POS, pSkill->m_sEXT_DATA.GETTARGZONE02(emELMT) );
						if ( pnew_group_effect )
						{
							if ( pSkill->m_sEXT_DATA.emTARGZONE01==SKILL::EMTIME_EFFECT_ACTIVATED )
								SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE01_POS, pSkill->m_sEXT_DATA.GETTARGZONE01(emELMT) );

							if ( pSkill->m_sEXT_DATA.emTARGZONE02==SKILL::EMTIME_EFFECT_ACTIVATED )
								SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE02_POS, pSkill->m_sEXT_DATA.GETTARGZONE02(emELMT) );

							if ( pSkill->m_sEXT_DATA.emTARGZONE03==SKILL::EMTIME_EFFECT_ACTIVATED )
								SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE03_POS, pSkill->m_sEXT_DATA.GETTARGZONE03(emELMT) );
						}
					}

					if ( pSkill->m_sEXT_DATA.emTARGZONE03==EMTIME_IMPACT )
					{
						/*skill activated effect, Juver, 2017/12/29 */
						DxEffSingleGroup* pnew_group_effect = SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE03_POS, pSkill->m_sEXT_DATA.GETTARGZONE03(emELMT) );
						if ( pnew_group_effect )
						{
							if ( pSkill->m_sEXT_DATA.emTARGZONE01==SKILL::EMTIME_EFFECT_ACTIVATED )
								SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE01_POS, pSkill->m_sEXT_DATA.GETTARGZONE01(emELMT) );

							if ( pSkill->m_sEXT_DATA.emTARGZONE02==SKILL::EMTIME_EFFECT_ACTIVATED )
								SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE02_POS, pSkill->m_sEXT_DATA.GETTARGZONE02(emELMT) );

							if ( pSkill->m_sEXT_DATA.emTARGZONE03==SKILL::EMTIME_EFFECT_ACTIVATED )
								SK_EFF_TARZONE ( sTarget, pSkill->m_sEXT_DATA.emTARGZONE03_POS, pSkill->m_sEXT_DATA.GETTARGZONE03(emELMT) );
						}
					}

					//	Note : 목표 몸 이펙트.
					if ( pSkill->m_sEXT_DATA.emTARGBODY01==EMTIME_IMPACT )
						DxEffGroupPlayer::GetInstance().NewEffBody ( pSkill->m_sEXT_DATA.GETTARGBODY01(emELMT), &sTarget, &vDir );
					if ( pSkill->m_sEXT_DATA.emTARGBODY02==EMTIME_IMPACT )
						DxEffGroupPlayer::GetInstance().NewEffBody ( pSkill->m_sEXT_DATA.GETTARGBODY02(emELMT), &sTarget, &vDir );
				}

				if ( pSkill->m_sEXT_DATA.bTARG_ONE )	break;
			}
		}
	}


	STARGETID sTARG(GETCROW(),m_dwGaeaID);

	if ( pSkill->m_sEXT_DATA.emSELFZONE01 == EMTIME_IMPACT )
	{
		/*skill activated effect, Juver, 2017/12/29 */
		DxEffSingleGroup* pnew_group_effect = SK_EFF_SELFZONE ( sStrikeEff, pSkill->m_sEXT_DATA.emSELFZONE01_POS, pSkill->m_sEXT_DATA.GETSELFZONE01(emELMT), &sTARG );
		if ( pnew_group_effect )
		{
			if ( pSkill->m_sEXT_DATA.emSELFZONE01==SKILL::EMTIME_EFFECT_ACTIVATED )
				SK_EFF_SELFZONE ( sStrikeEff, pSkill->m_sEXT_DATA.emSELFZONE01_POS, pSkill->m_sEXT_DATA.GETSELFZONE01(emELMT), &sTARG );

			if ( pSkill->m_sEXT_DATA.emSELFZONE02==SKILL::EMTIME_EFFECT_ACTIVATED )
				SK_EFF_SELFZONE ( sStrikeEff, pSkill->m_sEXT_DATA.emSELFZONE02_POS, pSkill->m_sEXT_DATA.GETSELFZONE02(emELMT), &sTARG );

			if ( pSkill->m_sEXT_DATA.emSELFZONE03==SKILL::EMTIME_EFFECT_ACTIVATED )
				SK_EFF_SELFZONE ( sStrikeEff, pSkill->m_sEXT_DATA.emSELFZONE03_POS, pSkill->m_sEXT_DATA.GETSELFZONE03(emELMT), &sTARG );
		}
	}

	if ( pSkill->m_sEXT_DATA.emSELFZONE02 == EMTIME_IMPACT )
	{
		/*skill activated effect, Juver, 2017/12/29 */
		DxEffSingleGroup* pnew_group_effect = SK_EFF_SELFZONE ( sStrikeEff, pSkill->m_sEXT_DATA.emSELFZONE02_POS, pSkill->m_sEXT_DATA.GETSELFZONE02(emELMT), &sTARG );
		if ( pnew_group_effect )
		{
			if ( pSkill->m_sEXT_DATA.emSELFZONE01==SKILL::EMTIME_EFFECT_ACTIVATED )
				SK_EFF_SELFZONE ( sStrikeEff, pSkill->m_sEXT_DATA.emSELFZONE01_POS, pSkill->m_sEXT_DATA.GETSELFZONE01(emELMT), &sTARG );

			if ( pSkill->m_sEXT_DATA.emSELFZONE02==SKILL::EMTIME_EFFECT_ACTIVATED )
				SK_EFF_SELFZONE ( sStrikeEff, pSkill->m_sEXT_DATA.emSELFZONE02_POS, pSkill->m_sEXT_DATA.GETSELFZONE02(emELMT), &sTARG );

			if ( pSkill->m_sEXT_DATA.emSELFZONE03==SKILL::EMTIME_EFFECT_ACTIVATED )
				SK_EFF_SELFZONE ( sStrikeEff, pSkill->m_sEXT_DATA.emSELFZONE03_POS, pSkill->m_sEXT_DATA.GETSELFZONE03(emELMT), &sTARG );
		}
	}

	if ( pSkill->m_sEXT_DATA.emSELFZONE03 == EMTIME_IMPACT )
	{
		/*skill activated effect, Juver, 2017/12/29 */
		DxEffSingleGroup* pnew_group_effect = SK_EFF_SELFZONE ( sStrikeEff, pSkill->m_sEXT_DATA.emSELFZONE03_POS, pSkill->m_sEXT_DATA.GETSELFZONE03(emELMT), &sTARG );
		if ( pnew_group_effect )
		{
			if ( pSkill->m_sEXT_DATA.emSELFZONE01==SKILL::EMTIME_EFFECT_ACTIVATED )
				SK_EFF_SELFZONE ( sStrikeEff, pSkill->m_sEXT_DATA.emSELFZONE01_POS, pSkill->m_sEXT_DATA.GETSELFZONE01(emELMT), &sTARG );

			if ( pSkill->m_sEXT_DATA.emSELFZONE02==SKILL::EMTIME_EFFECT_ACTIVATED )
				SK_EFF_SELFZONE ( sStrikeEff, pSkill->m_sEXT_DATA.emSELFZONE02_POS, pSkill->m_sEXT_DATA.GETSELFZONE02(emELMT), &sTARG );

			if ( pSkill->m_sEXT_DATA.emSELFZONE03==SKILL::EMTIME_EFFECT_ACTIVATED )
				SK_EFF_SELFZONE ( sStrikeEff, pSkill->m_sEXT_DATA.emSELFZONE03_POS, pSkill->m_sEXT_DATA.GETSELFZONE03(emELMT), &sTARG );
		}
	}

	if ( pSkill->m_sEXT_DATA.emSELFBODY == EMTIME_IMPACT )
		DxEffGroupPlayer::GetInstance().NewEffBody ( pSkill->m_sEXT_DATA.GETSELFBODY(emELMT), &sTARG, &m_vDir );
}

BOOL GLCharClient::SkillProc ( float fElapsedTime )
{
	m_fattTIMER += fElapsedTime * GETATTVELO();
	m_fattTIMER += GETATT_ITEM();
	GASSERT(m_pSkinChar->GETCURANIMNODE());

	PANIMCONTNODE pAnicontNode = m_pSkinChar->GETCURANIMNODE();
	GASSERT(pAnicontNode);
	PSANIMCONTAINER pAnimCont = pAnicontNode->pAnimCont;

	if ( pAnimCont->m_wStrikeCount == 0 )	return FALSE;

	DWORD dwThisKey = pAnimCont->m_dwSTime + DWORD(m_fattTIMER*UNITANIKEY_PERSEC);

	DWORD dwStrikeKey = pAnimCont->m_sStrikeEff[m_nattSTEP].m_dwFrame;
	if ( m_nattSTEP < pAnimCont->m_wStrikeCount )
	{
		if ( dwThisKey>=dwStrikeKey )
		{
			//	Note : 스킬 이펙트 발동.
			//
			SkillEffect ( pAnimCont->m_sStrikeEff[m_nattSTEP] );
		
			m_nattSTEP++;
		}
	}

	return TRUE;
}

void GLCharClient::UPDATE_ITEM ( )
{
	m_fITEM_MOVE_R = 0.0f;
	m_fITEMATTVELO_R = 0.0f;
	m_fITEM_MOVE = 0.0f;
	m_fITEMATTVELO = 0.0f;

	for ( int i=0; i<SLOT_NSIZE_S_2; i++ )
	{
		EMSLOT emSLOT = static_cast<EMSLOT>(i);
		if ( !VALID_SLOT_ITEM(emSLOT) )					continue;

		SITEMCUSTOM sItemCustom;
		const SITEMCLIENT& sItemClient = m_CharData.m_PutOnItems[i];
		sItemCustom.Assign( sItemClient );
		
		SITEM* sItem = GET_SLOT_ITEMDATA( emSLOT );
		if ( !sItem ) continue;

		//	Note : 변화율 효과.
		for ( DWORD svaron=0; svaron<ITEM::SSUIT::VARIATION_SIZE; ++svaron )
		{
			switch ( sItem->sSuitOp.sVARIATE[svaron].emTYPE )
			{
			case EMVAR_HP:
			case EMVAR_MP:
			case EMVAR_SP:
			case EMVAR_AP:
				break;
			case EMVAR_MOVE_SPEED:
				if ( emSLOT != SLOT_VEHICLE )	m_fITEM_MOVE_R += sItemCustom.GETMOVESPEEDR();
				else if ( m_bVehicle )	m_fITEM_MOVE_R += m_sVehicle.GetSpeedRate() + sItemCustom.GETMOVESPEEDR();;
				break;
			case EMVAR_ATTACK_SPEED:
				m_fITEMATTVELO_R	+= sItem->sSuitOp.sVARIATE[svaron].fVariate;
				break;
			case EMVAR_CRITICAL_RATE:
				break;
			case EMVAR_CRUSHING_BLOW:
				break;
			case EMVAR_DECR_DAMAGE_MELEESKILL:
				break;
			case EMVAR_DECR_DAMAGE_RANGESKILL:
				break;
			case EMVAR_DECR_DAMAGE_MAGICSKILL:
				break;
			};
		}

		// 변화량 효과
		switch ( sItem->sSuitOp.sVOLUME.emTYPE )
		{
		case EMVAR_HP:
		case EMVAR_MP:
		case EMVAR_SP:
		case EMVAR_AP:
			break;
		case EMVAR_MOVE_SPEED:
			if ( emSLOT != SLOT_VEHICLE )	m_fITEM_MOVE += sItemCustom.GETMOVESPEED();
			else if ( m_bVehicle ) m_fITEM_MOVE += m_sVehicle.GetSpeedVol() + sItemCustom.GETMOVESPEED();
			break;
		case EMVAR_ATTACK_SPEED:
			m_fITEMATTVELO += sItem->sSuitOp.sVOLUME.fVolume;
			break;
		case EMVAR_CRITICAL_RATE:
			break;
		case EMVAR_CRUSHING_BLOW:
			break;
		case EMVAR_DECR_DAMAGE_MELEESKILL:
			break;
		case EMVAR_DECR_DAMAGE_RANGESKILL:
			break;
		case EMVAR_DECR_DAMAGE_MAGICSKILL:
			break;
		};


		// 이동속도 랜덤옵션 적용( 파츠는 위에서 적용 m_sVehicle.GetSpeedRate(); ) 
//		if ( emSLOT != SLOT_VEHICLE )	m_fITEM_MOVE += sItemCustom.GETOptVALUE( EMR_OPT_MOVE_SPEED );
//		else if ( m_bVehicle ) m_fITEM_MOVE += sItemCustom.GETOptVALUE( EMR_OPT_MOVE_SPEED );

	}
}

void GLCharClient::UpdateLandEffect()
{
	PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();
	if( !pLand ) return;

	SLEVEL_ETC_FUNC *pLevelEtcFunc = pLand->GetLevelEtcFunc();
	if( !pLevelEtcFunc ) return;
	if( !pLevelEtcFunc->m_bUseFunction[EMETCFUNC_LANDEFFECT] ) return;

	// 모든 버프를 취소해야할 경우의 처리
	for( int i = 0; i < EMLANDEFFECT_MULTI; i++ )
	{
		SLANDEFFECT landEffect = m_sLandEffect[i];
		if( !landEffect.IsUse() ) continue;
		if( landEffect.emLandEffectType == EMLANDEFFECT_CANCEL_ALLBUFF )
		{
			if( m_dwTransfromSkill != SNATIVEID::ID_NULL )
			{
				m_dwTransfromSkill = SNATIVEID::ID_NULL;
				UpdateSuit();
			}
			for ( int i=0; i<SKILLFACT_SIZE; ++i )
			{
				FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_dwGaeaID,m_vPos), m_pSkinChar, m_sSKILLFACT[i].sNATIVEID );
				DISABLESKEFF(i);
			}			
			return;
		}
	}
}

void GLCharClient::UPDATE_DATA ( float fTime, float fElapsedTime, BOOL bClient )
{
	m_fSTATE_MOVE = 1.0f;
	m_fSTATE_DELAY = 1.0f;

	m_fSKILL_MOVE = 0.0f;
	m_fATTVELO = 0.0f;

	m_bSTATE_PANT = false;
	m_bSTATE_STUN = false;

	m_bINVISIBLE = false;
	m_bSafeZone = false;

	m_sLINKHP.RESET();

	m_sLINKHP.fTake = 1.0f;
	m_sLINKHP.fRef = 0.0f;

	memset ( m_bActionType, false, sizeof( bool ) * SKILL::EMACTION_NSIZE );

	for ( int i=0; i<SKILLFACT_SIZE; ++i )
	{
		SSKILLFACT &sSKEFF = m_sSKILLFACT[i];
		if ( sSKEFF.sNATIVEID==NATIVEID_NULL() )	continue;

		sSKEFF.fAGE -= fElapsedTime;

		for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
		{
			switch ( sSKEFF.sSpecs[nSpec].emSPEC )
			{
			case EMSPECA_MOVEVELO:
				m_fSKILL_MOVE += sSKEFF.sSpecs[nSpec].fSPECVAR1;
				break;

			//stun visual bug fixed
			case EMSPECA_STUN:
				m_bSTATE_STUN = true;

			case EMSPECA_INVISIBLE:
				m_bINVISIBLE = true;
				break;

			case EMSPECA_ATTACKVELO:
				//	공격 속도 계념이므로 ( 공격 시간을 -0.1 (-10%) 감소 시키기 위해서는 부호 반전을 하여야함. )
				m_fATTVELO -= sSKEFF.sSpecs[nSpec].fSPECVAR1;
				break;

			case EMSPECA_SKILLDELAY:
				break;
			case EMSPECA_DAMAGE_LOOP:
				if ( static_cast<int>(sSKEFF.sSpecs[nSpec].fSPECVAR2) == 1 )
				{
					if ( sSKEFF.sSpecs[nSpec].fSPECVAR1 < m_sLINKHP.fTake )
						m_sLINKHP.fTake = sSKEFF.sSpecs[nSpec].fSPECVAR1;
				}
				break;
			};
		}

        if ( bClient ) 
        {
            PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( sSKEFF.sNATIVEID );
            if ( pSkill ) 
            {
                m_bActionType[pSkill->m_sBASIC.emACTION] = true;
            }
        }
		

		//	클라이언트가 아닐 경우 여기서 스킬 이펙트를 비활성화, 클라이언트는 UpdateSkillEffect()에서 함.
		if ( !bClient && sSKEFF.fAGE <= 0.0f )		DISABLESKEFF(i);
	}

	for ( int i=0; i<EMBLOW_MULTI; ++i )
	{
		SSTATEBLOW &sSTATEBLOW = m_sSTATEBLOWS[i];
		if ( sSTATEBLOW.emBLOW == EMBLOW_NONE )		continue;

		sSTATEBLOW.fAGE -= fElapsedTime;

		switch ( sSTATEBLOW.emBLOW )
		{
		case EMBLOW_NUMB:
			m_fSTATE_MOVE += sSTATEBLOW.fSTATE_VAR1;			//	이동속도 감소.
			m_fSTATE_DELAY = 1.0f + sSTATEBLOW.fSTATE_VAR2;		//	딜래이 증가.
			break;

		case EMBLOW_STUN:
			m_fSTATE_MOVE = 0.0f;								//	기절.
			m_bSTATE_PANT = true;
			m_bSTATE_STUN = true;
			break;

		case EMBLOW_STONE:
			m_fSTATE_MOVE += sSTATEBLOW.fSTATE_VAR1;			//	이동속도 감소.
			break;

		case EMBLOW_BURN:
			break;

		case EMBLOW_FROZEN:
			m_fSTATE_MOVE += sSTATEBLOW.fSTATE_VAR1;			//	이동속도 감소.
			break;

		case EMBLOW_MAD:
			m_bSTATE_PANT = true;
			break;

		case EMBLOW_POISON:
			break;

		case EMBLOW_CURSE:
			break;
		};
	}
	
	switch ( m_CharData.sQITEMFACT.emType )
	{
	case QUESTION_SPEED_UP:
		m_fSKILL_MOVE += (m_CharData.sQITEMFACT.wParam1/100.0f);
		break;

	case QUESTION_CRAZY:
		m_fSKILL_MOVE += (m_CharData.sQITEMFACT.wParam1/100.0f);
		m_fATTVELO += (m_CharData.sQITEMFACT.wParam2/100.0f);
		break;

	case QUESTION_ATTACK_UP:
		break;

	case QUESTION_EXP_UP:
		break;

	case QUESTION_LUCKY:
		break;

	case QUESTION_SPEED_UP_M:
		m_fSKILL_MOVE += (m_CharData.sQITEMFACT.wParam1/100.0f);
		break;

	case QUESTION_MADNESS:
		m_fSKILL_MOVE += (m_CharData.sQITEMFACT.wParam1/100.0f);
		m_fATTVELO += (m_CharData.sQITEMFACT.wParam2/100.0f);
		break;

	case QUESTION_ATTACK_UP_M:
		break;
	};


	// 지형 효과 처리
	for( int i = 0; i < EMLANDEFFECT_MULTI; i++ )
	{
		SLANDEFFECT landEffect = m_sLandEffect[i];
		if( !landEffect.IsUse() ) continue;

		switch( landEffect.emLandEffectType ) 
		{
		case EMLANDEFFECT_ATK_SPEED: // 공격속도
			m_fATTVELO	  += landEffect.fValue;
			break;
		case EMLANDEFFECT_MOVE_SPEED: // 이동속도
			m_fSKILL_MOVE += landEffect.fValue;
			break;
		case EMLANDEFFECT_MP_RATE:	  // HP 변화율
		case EMLANDEFFECT_HP_RATE:    // MP 변화율
		case EMLANDEFFECT_RECOVER_RATE: // 회복율
		case EMLANDEFFECT_DAMAGE_RATE:	// 공격치 변화율
		case EMLANDEFFECT_DEFENSE_RATE:	// 방어치 변화율
		case EMLANDEFFECT_RESIST_RATE:	// 저항 수치			
		case EMLANDEFFECT_CANCEL_ALLBUFF: // 모든 버프 취소
			break;
		case EMLANDEFFECT_SAFE_ZONE:
			int fValue = landEffect.fValue;
			switch (fValue)
			{

			case 1:
				{
					if(m_CharData.wSchool == 0) m_bSafeZone = true;
				}
				break;

			case 2:
				{
					if(m_CharData.wSchool == 1) m_bSafeZone = true;
				}
				break;

			case 3:
				{
					if(m_CharData.wSchool == 2) m_bSafeZone = true;
				}
				break;

			case 4:
				{
					if(m_CharData.m_wCaptureTheFlagTeam == 0) m_bSafeZone = true;
				}
				break;

			case 5:
				{
					if(m_CharData.m_wCaptureTheFlagTeam == 1) m_bSafeZone = true;
				}
				break;

			default:
				m_bSafeZone = true;
				break;
			}
			//m_bSafeZone = true;
			break;
		}
	}

	/*itemfood system, Juver, 2017/05/25 */
	for ( int i=0; i<FITEMFACT_SIZE; ++i )
	{
		SFITEMFACT &sSKEFF = m_sFITEMFACT[i];
		if ( sSKEFF.sNATIVEID==NATIVEID_NULL() )	continue;

		sSKEFF.fAGE -= fElapsedTime;

		for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
		{
			switch ( sSKEFF.sSpecs[nSpec].emSPEC )
			{
			case EMSPECA_MOVEVELO:
				m_fSKILL_MOVE += sSKEFF.sSpecs[nSpec].fSPECVAR1;
				break;

			case EMSPECA_STUN:
				m_bSTATE_STUN = true;

			case EMSPECA_INVISIBLE:
				m_bINVISIBLE = true;
				break;

			case EMSPECA_ATTACKVELO:
				m_fATTVELO -= sSKEFF.sSpecs[nSpec].fSPECVAR1;
				break;

			case EMSPECA_SKILLDELAY:
				break;
			};
		}

		if ( sSKEFF.fAGE <= 0.0f )		
		{
			m_sFITEMFACT[i].sNATIVEID = NATIVEID_NULL();
		}
	}

	/*system buffs, Juver, 2017/09/04 */
	for ( int i=0; i<SYSTEM_BUFF_SIZE; ++i )
	{
		SSYSTEM_BUFF &ssystembuff = m_sSYSTEM_BUFF[i];
		if ( ssystembuff.sNATIVEID==NATIVEID_NULL() )	continue;

		for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
		{
			switch ( ssystembuff.sSpecs[nSpec].emSPEC )
			{
			case EMSPECA_MOVEVELO:
				m_fSKILL_MOVE += ssystembuff.sSpecs[nSpec].fSPECVAR1;
				break;

			case EMSPECA_STUN:
				m_bSTATE_STUN = true;

			case EMSPECA_INVISIBLE:
				m_bINVISIBLE = true;
				break;

			case EMSPECA_ATTACKVELO:
				m_fATTVELO -= ssystembuff.sSpecs[nSpec].fSPECVAR1;
				break;

			case EMSPECA_SKILLDELAY:
				break;
			};
		}
	}

	//	Memo :	? 아이템 이벤트 적용
	if( m_CharData.sEVENTFACT.IsACTIVE( EMGM_EVENT_SPEED ) )	m_fSKILL_MOVE += (m_CharData.sEVENTFACT.wSpeed/100.0f);
	if( m_CharData.sEVENTFACT.IsACTIVE( EMGM_EVENT_ASPEED) )	m_fATTVELO += (m_CharData.sEVENTFACT.wASpeed/100.0f);

	//	공격/이동 속도. ( 지속 skill 속성 가산된 부분에 state, passive skill 속성 가산. )
	m_fATTVELO += m_fSTATE_MOVE + m_CharData.sPASSIVE_SKILL.m_fATTVELO;
	m_fSKILL_MOVE += m_CharData.sPASSIVE_SKILL.m_fMOVEVELO;

	/*vehicle booster system, Juver, 2017/08/11 */
	if ( m_CharData.m_bBoosterStart )	m_fSKILL_MOVE += ( GLCONST_CHAR::fVehicleBoosterAddSpeed /100.0f );
}

BOOL GLCharClient::RECEIVE_SKILLFACT ( const SNATIVEID skill_id, const WORD wlevel, const DWORD dwSELECT, WORD _wCasterCrow, DWORD _dwCasterID, float fAge )
{
	//	스킬 정보 가져옴.
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( skill_id.wMainID, skill_id.wSubID );
	if ( !pSkill )				return FALSE;
	if ( wlevel >= SKILL::MAX_LEVEL )	return FALSE;
	SKILL::CDATA_LVL &sSKILL_DATA = pSkill->m_sAPPLY.sDATA_LVL[wlevel];

	BOOL bHOLD = FALSE;
	SSKILLFACT sSKILLEF;
	sSKILLEF._wCasterCrow = _wCasterCrow;
	sSKILLEF._dwCasterID = _dwCasterID;

	switch ( pSkill->m_sAPPLY.emBASIC_TYPE )
	{
	case SKILL::EMFOR_VARHP:
	case SKILL::EMFOR_VARMP:
	case SKILL::EMFOR_VARSP:
	case SKILL::EMFOR_DEFENSE:
	case SKILL::EMFOR_HITRATE:
	case SKILL::EMFOR_AVOIDRATE:
	case SKILL::EMFOR_VARAP:
	case SKILL::EMFOR_VARDAMAGE:
	case SKILL::EMFOR_VARDEFENSE:
	case SKILL::EMFOR_PA:
	case SKILL::EMFOR_SA:
	case SKILL::EMFOR_MA:
	case SKILL::EMFOR_RESIST:
	case SKILL::EMFOR_SUMMONTIME: /*summon time, Juver, 2017/12/12 */
		bHOLD = TRUE;
		sSKILLEF.emTYPE = pSkill->m_sAPPLY.emBASIC_TYPE;
		sSKILLEF.fMVAR = sSKILL_DATA.fBASIC_VAR;
		break;
	};

	for( int nImpact=0; nImpact<SKILL::MAX_IMPACT; ++nImpact )
	{
		if ( pSkill->m_sAPPLY.sImpacts[nImpact].emADDON != EMIMPACTA_NONE )
		{
			bHOLD = TRUE;
			sSKILLEF.sImpacts[nImpact].emADDON = pSkill->m_sAPPLY.sImpacts[nImpact].emADDON;
			sSKILLEF.sImpacts[nImpact].fADDON_VAR = pSkill->m_sAPPLY.sImpacts[nImpact].fADDON_VAR[wlevel];
		}
	}

	for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
	{
		switch ( pSkill->m_sAPPLY.sSpecs[nSpec].emSPEC )
		{
		case EMSPECA_REFDAMAGE:
		case EMSPECA_NONBLOW:
		case EMSPECA_PIERCE:
		case EMSPECA_TARRANGE:
		case EMSPECA_MOVEVELO:
		case EMSPECA_ATTACKVELO:
		case EMSPECA_SKILLDELAY:
		case EMSPECA_PSY_DAMAGE_REDUCE:
		case EMSPECA_MAGIC_DAMAGE_REDUCE:
		case EMSPECA_PSY_DAMAGE_REFLECTION:
		case EMSPECA_MAGIC_DAMAGE_REFLECTION:
		case EMSPECA_DEFENSE_SKILL_ACTIVE:
		case EMSPECA_IMMUNE:
		case EMSPECA_PROHIBIT_POTION:		/*prohibit potion skill logic, Juver, 2017/06/06 */
		case EMSPECA_PROHIBIT_SKILL:		/*prohibit skill logic, Juver, 2017/06/06 */
		case EMSPECA_CHANGE_ATTACK_RANGE:	/*skill range spec logic, Juver, 2017/06/06 */
		case EMSPECA_CHANGE_APPLY_RANGE:	/*skill range spec logic, Juver, 2017/06/06 */
		case EMSPECA_STUN:					/*skill stun logic, Juver, 2017/06/06 */
		case EMSPECA_CONTINUOUS_DAMAGE:		/*continuous damage skill logic, Juver, 2017/06/10 */
		case EMSPECA_CURSE:					/*curse skill logic, Juver, 2017/06/10 */
		case EMSPECA_TALK_TO_NPC:			/*npc talk buff, Juver, 2017/10/03 */
		case EMSPECA_IGNORE_DAMAGE:			/*ignore damage, Juver, 2017/12/12 */
		case EMSPECA_STIGMA:
		case EMSPECA_INVISIBLE:
		case EMSPECA_RECVISIBLE:
		case EMSPECA_DAMAGE_LOOP:
		case EMSPECA_ENHANCEMENT:
			bHOLD = TRUE;
			sSKILLEF.sSpecs[nSpec].emSPEC = pSkill->m_sAPPLY.sSpecs[nSpec].emSPEC;
			sSKILLEF.sSpecs[nSpec].fSPECVAR1 = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wlevel].fVAR1;
			sSKILLEF.sSpecs[nSpec].fSPECVAR2 = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wlevel].fVAR2;
			sSKILLEF.sSpecs[nSpec].dwSPECFLAG = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wlevel].dwFLAG;
			sSKILLEF.sSpecs[nSpec].dwNativeID = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wlevel].dwNativeID;
			break;
		};
	}

	//	Note : 지속형 스킬일 경우 최적 슬롯을 찾아서 스킬 효과를 넣어줌.
	//
	if ( bHOLD )
	{
		sSKILLEF.sNATIVEID	= skill_id;
		sSKILLEF.wLEVEL		= wlevel;

		if ( fAge == _SKILLFACT_DEFAULTTIME )	sSKILLEF.fAGE = sSKILL_DATA.fLIFE;
		else									sSKILLEF.fAGE = fAge;

		m_sSKILLFACT[dwSELECT] = sSKILLEF;

		if( pSkill->m_sSPECIAL_SKILL.emSSTYPE == SKILL::EMSSTYPE_TRANSFORM )
		{
			m_dwTransfromSkill = dwSELECT;
			m_sSKILLFACT[dwSELECT].SetSpecialSkill( SKILL::EMSSTYPE_TRANSFORM );
			m_sSKILLFACT[dwSELECT].bRanderSpecialEffect = FALSE;

			if( m_pSkinChar )
			{
				for( BYTE i = 0; i < SKILL::EMTRANSFORM_NSIZE; i++ )
				{
					if( m_CharData.wSex == 1 ) {
						if( pSkill->m_sSPECIAL_SKILL.strTransform_Man[i].empty() ) continue;
						m_pSkinChar->SetPiece ( pSkill->m_sSPECIAL_SKILL.strTransform_Man[i].c_str(), m_pd3dDevice, 0X0, 0, TRUE );
					}else{
						if( pSkill->m_sSPECIAL_SKILL.strTransform_Woman[i].empty() ) continue;
						m_pSkinChar->SetPiece ( pSkill->m_sSPECIAL_SKILL.strTransform_Woman[i].c_str(), m_pd3dDevice, 0X0, 0, TRUE );
					}
				}
			}
		}

		return TRUE;
	}

	GASSERT ( 0 && "지속 스킬정보가 유효하지 않음." );
	return FALSE;
}

EMSLOT GLCharClient::GetCurRHand()
{
	if( IsUseArmSub() ) return SLOT_RHAND_S;
	else				return SLOT_RHAND;
}

EMSLOT GLCharClient::GetCurLHand()
{
	if( IsUseArmSub() ) return SLOT_LHAND_S;
	else				return SLOT_LHAND;
}

BOOL GLCharClient::VALID_SLOT_ITEM ( EMSLOT _slot )							
{ 
	if( m_CharData.m_PutOnItems[_slot].sNativeID==NATIVEID_NULL() ) return FALSE;

	if( IsUseArmSub() )
	{
		if( _slot == SLOT_RHAND || _slot == SLOT_LHAND ) return FALSE;
	}
	else
	{
		if( _slot == SLOT_RHAND_S || _slot == SLOT_LHAND_S ) return FALSE;
	}

	return TRUE;
}

BOOL GLCharClient::IsCurUseArm( EMSLOT emSlot )
{
	if( IsUseArmSub() )
	{
		if( emSlot == SLOT_RHAND || emSlot == SLOT_LHAND ) return FALSE;
	}
	else
	{
		if( emSlot == SLOT_RHAND_S || emSlot == SLOT_LHAND_S ) return FALSE;
	}

	return TRUE;
}

D3DXVECTOR3 GLCharClient::GetPosBodyHeight ()
{ 
	float fHeight = m_fHeight * m_CharData.m_fScaleRange;

	/*vehicle system, Juver, 2017/08/09 */
	if ( m_bVehicle )
	{
		int emType = m_sVehicle.m_emTYPE;
		if ( emType == VEHICLE_TYPE_BOARD )	fHeight += 10.0f;
	}

	return D3DXVECTOR3( m_vPos.x, m_vPos.y+fHeight, m_vPos.z ); 
}


void GLCharClient::DISABLEALLLANDEFF()
{
	for( int i = 0; i < EMLANDEFFECT_MULTI; i++ )
	{
		m_sLandEffect[i].Init();
	}
}

void GLCharClient::ADDLANDEFF( SLANDEFFECT landEffect, int iNum )
{
	if( iNum >= EMLANDEFFECT_MULTI ) return;
	m_sLandEffect[iNum] = landEffect;
}

void GLCharClient::GetSkinAABB( D3DXVECTOR3& vMin, D3DXVECTOR3& vMax )
{ 
	if( m_pSkinChar )
		m_pSkinChar->GetAABBBox( vMax, vMin );
}

float GLCharClient::GetSkinScale()
{ 
	if( !m_pSkinChar )	return 1.0f;
	return m_pSkinChar->GetScale();
}

float GLCharClient::GetSkinHeight()
{ 
	if( !m_pSkinChar )	return 20.0f;
	return m_pSkinChar->m_fHeight;
}

/*itemfood system, Juver, 2017/05/25 */
BOOL GLCharClient::ItemFoodAdd ( SNATIVEID skill_id, WORD wLevel, WORD wSLOT, float fAge /*= _SKILLFACT_DEFAULTTIME*/ )
{
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( skill_id.wMainID, skill_id.wSubID );
	if ( !pSkill )	return FALSE;
	if ( wLevel >= SKILL::MAX_LEVEL )	return FALSE;
	if ( wSLOT >= FITEMFACT_SIZE )	return FALSE;

	SKILL::CDATA_LVL &sSKILL_DATA = pSkill->m_sAPPLY.sDATA_LVL[wLevel];

	BOOL bHOLD = FALSE;
	SFITEMFACT sSKILLEF;
	switch ( pSkill->m_sAPPLY.emBASIC_TYPE )
	{
	case SKILL::EMFOR_VARHP:
	case SKILL::EMFOR_VARMP:
	case SKILL::EMFOR_VARSP:
	case SKILL::EMFOR_DEFENSE:
	case SKILL::EMFOR_HITRATE:
	case SKILL::EMFOR_AVOIDRATE:
	case SKILL::EMFOR_VARAP:
	case SKILL::EMFOR_VARDAMAGE:
	case SKILL::EMFOR_VARDEFENSE:
	case SKILL::EMFOR_PA:
	case SKILL::EMFOR_SA:
	case SKILL::EMFOR_MA:
	case SKILL::EMFOR_RESIST:
	case SKILL::EMFOR_SUMMONTIME: /*summon time, Juver, 2017/12/12 */
		bHOLD = TRUE;
		sSKILLEF.emTYPE = pSkill->m_sAPPLY.emBASIC_TYPE;
		sSKILLEF.fMVAR = sSKILL_DATA.fBASIC_VAR;
		break;
	};

	for( int nImpact=0; nImpact<SKILL::MAX_IMPACT; ++nImpact )
	{
		if ( pSkill->m_sAPPLY.sImpacts[nImpact].emADDON != EMIMPACTA_NONE )
		{
			bHOLD = TRUE;
			sSKILLEF.sImpacts[nImpact].emADDON = pSkill->m_sAPPLY.sImpacts[nImpact].emADDON;
			sSKILLEF.sImpacts[nImpact].fADDON_VAR = pSkill->m_sAPPLY.sImpacts[nImpact].fADDON_VAR[wLevel];
		}
	}

	for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
	{
		switch ( pSkill->m_sAPPLY.sSpecs[nSpec].emSPEC )
		{
		case EMSPECA_REFDAMAGE:
		case EMSPECA_NONBLOW:
		case EMSPECA_PIERCE:
		case EMSPECA_TARRANGE:
		case EMSPECA_MOVEVELO:
		case EMSPECA_ATTACKVELO:
		case EMSPECA_SKILLDELAY:
		case EMSPECA_PSY_DAMAGE_REDUCE:
		case EMSPECA_MAGIC_DAMAGE_REDUCE:
		case EMSPECA_PSY_DAMAGE_REFLECTION:
		case EMSPECA_MAGIC_DAMAGE_REFLECTION:
		case EMSPECA_DEFENSE_SKILL_ACTIVE:
		case EMSPECA_CHANGE_ATTACK_RANGE:		/*skill range spec logic, Juver, 2017/06/06 */
		case EMSPECA_CHANGE_APPLY_RANGE:		/*skill range spec logic, Juver, 2017/06/06 */
		case EMSPECA_TALK_TO_NPC:				/*npc talk buff, Juver, 2017/10/03 */
			bHOLD = TRUE;
			sSKILLEF.sSpecs[nSpec].emSPEC = pSkill->m_sAPPLY.sSpecs[nSpec].emSPEC;
			sSKILLEF.sSpecs[nSpec].fSPECVAR1 = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].fVAR1;
			sSKILLEF.sSpecs[nSpec].fSPECVAR2 = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].fVAR2;
			sSKILLEF.sSpecs[nSpec].dwSPECFLAG = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].dwFLAG;
			sSKILLEF.sSpecs[nSpec].dwNativeID = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].dwNativeID;
			break;
		};
	}

	if ( bHOLD )
	{
		sSKILLEF.sNATIVEID	= skill_id;
		sSKILLEF.wLEVEL		= wLevel;

		if ( fAge == _SKILLFACT_DEFAULTTIME )	sSKILLEF.fAGE = sSKILL_DATA.fLIFE;
		else									sSKILLEF.fAGE = fAge;

		m_sFITEMFACT[wSLOT] = sSKILLEF;

		return TRUE;
	}

	GASSERT ( 0 && "지속 스킬정보가 유효하지 않음." );
	return FALSE;
}

/*system buffs, Juver, 2017/09/04 */
BOOL GLCharClient::SystemBuffAdd ( SNATIVEID skill_id, WORD wLevel, WORD wSLOT )
{
	if ( wSLOT >= SYSTEM_BUFF_SIZE )		return FALSE;
	m_sSYSTEM_BUFF[wSLOT].RESET();

	if ( skill_id == NATIVEID_NULL() )	return FALSE;

	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( skill_id.wMainID, skill_id.wSubID );
	if ( !pSkill )						return FALSE;
	if ( wLevel >= SKILL::MAX_LEVEL )	return FALSE;


	SKILL::CDATA_LVL &sSKILL_DATA = pSkill->m_sAPPLY.sDATA_LVL[wLevel];

	BOOL bHOLD = FALSE;
	SSYSTEM_BUFF ssystem_buff;
	switch ( pSkill->m_sAPPLY.emBASIC_TYPE )
	{
	case SKILL::EMFOR_VARHP:
	case SKILL::EMFOR_VARMP:
	case SKILL::EMFOR_VARSP:
	case SKILL::EMFOR_DEFENSE:
	case SKILL::EMFOR_HITRATE:
	case SKILL::EMFOR_AVOIDRATE:
	case SKILL::EMFOR_VARAP:
	case SKILL::EMFOR_VARDAMAGE:
	case SKILL::EMFOR_VARDEFENSE:
	case SKILL::EMFOR_PA:
	case SKILL::EMFOR_SA:
	case SKILL::EMFOR_MA:
	case SKILL::EMFOR_RESIST:
	case SKILL::EMFOR_SUMMONTIME: /*summon time, Juver, 2017/12/12 */
		bHOLD = TRUE;
		ssystem_buff.emTYPE = pSkill->m_sAPPLY.emBASIC_TYPE;
		ssystem_buff.fMVAR = sSKILL_DATA.fBASIC_VAR;
		break;
	};

	for( int nImpact=0; nImpact<SKILL::MAX_IMPACT; ++nImpact )
	{
		if ( pSkill->m_sAPPLY.sImpacts[nImpact].emADDON != EMIMPACTA_NONE )
		{
			bHOLD = TRUE;
			ssystem_buff.sImpacts[nImpact].emADDON = pSkill->m_sAPPLY.sImpacts[nImpact].emADDON;
			ssystem_buff.sImpacts[nImpact].fADDON_VAR = pSkill->m_sAPPLY.sImpacts[nImpact].fADDON_VAR[wLevel];
		}
	}

	for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
	{
		switch ( pSkill->m_sAPPLY.sSpecs[nSpec].emSPEC )
		{
		case EMSPECA_REFDAMAGE:
		case EMSPECA_NONBLOW:
		case EMSPECA_PIERCE:
		case EMSPECA_TARRANGE:
		case EMSPECA_MOVEVELO:
		case EMSPECA_ATTACKVELO:
		case EMSPECA_SKILLDELAY:
		case EMSPECA_PSY_DAMAGE_REDUCE:
		case EMSPECA_MAGIC_DAMAGE_REDUCE:
		case EMSPECA_PSY_DAMAGE_REFLECTION:
		case EMSPECA_MAGIC_DAMAGE_REFLECTION:
		case EMSPECA_DEFENSE_SKILL_ACTIVE:
		case EMSPECA_CHANGE_ATTACK_RANGE:		/*skill range spec logic, Juver, 2017/06/06 */
		case EMSPECA_CHANGE_APPLY_RANGE:		/*skill range spec logic, Juver, 2017/06/06 */
		case EMSPECA_TALK_TO_NPC:				/*npc talk buff, Juver, 2017/10/03 */
			bHOLD = TRUE;
			ssystem_buff.sSpecs[nSpec].emSPEC = pSkill->m_sAPPLY.sSpecs[nSpec].emSPEC;
			ssystem_buff.sSpecs[nSpec].fSPECVAR1 = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].fVAR1;
			ssystem_buff.sSpecs[nSpec].fSPECVAR2 = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].fVAR2;
			ssystem_buff.sSpecs[nSpec].dwSPECFLAG = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].dwFLAG;
			ssystem_buff.sSpecs[nSpec].dwNativeID = pSkill->m_sAPPLY.sSpecs[nSpec].sSPEC[wLevel].dwNativeID;
			break;
		};
	}

	if ( bHOLD )
	{
		ssystem_buff.sNATIVEID	= skill_id;
		ssystem_buff.wLEVEL		= wLevel;

		m_sSYSTEM_BUFF[wSLOT]	= ssystem_buff;

		return TRUE;
	}


	return FALSE;
}

void GLCharClient::SetHiddenWeaponPiece()
{
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( m_idACTIVESKILL.wMainID, m_idACTIVESKILL.wSubID );
	if( !pSkill )
		return;

	// 스킬에 사용되는 무기를 세팅한다;
	const bool bHiddenWeapon = pSkill->m_sLEARN.bHiddenWeapon;
	if( !bHiddenWeapon )
		return;

	SNATIVEID sHiddenWeapon = pSkill->m_sLEARN.sHiddenWeapon;
	SITEM* pItem = GLItemMan::GetInstance().GetItem ( sHiddenWeapon );
	if( pItem )
	{
		// [shhan][2014.12.18] rm #571 - 극강부 사격스킬(건류/극강글러브류) 후 왼손 권총 잔상문제.
		//						망토로 인해 ResetHiddenWeaponPiece 에서 UpdateSuit 안 했더니 
		//						양손권총 무기 사용 후 한손무기 스킬을 사용하면 다른한손에 권총이 보이는 버그가 있었다.
		//						그래서 활성화 된 위치만 렌더링 되도록 수정작업함.
		/*EMPIECECHAR emType(PIECE_SHIFT_RWEAPON);
		if ( m_pSkinChar->SetPieceHiddenWeapon ( pItem->GetWearingFile(m_CHARINDEX),	m_pd3dDevice, 0, EMSCD_ALL_THREAD, emType, pItem->GetSelfBodyEffect() ) == S_OK )
		{
			if ( emType == PIECE_SHIFT_RWEAPON || emType == PIECE_SHIFT_LWEAPON )
			{
				m_pSkinChar->SetPartRend( emType, TRUE );
			}
		}
		if ( m_pSkinChar->SetPieceHiddenWeapon ( pItem->GetWearingFileEx(m_CHARINDEX),	m_pd3dDevice, 0, EMSCD_ALL_THREAD,  emType, pItem->GetSelfBodyEffect() ) == S_OK )
		{
			if ( emType == PIECE_SHIFT_RWEAPON || emType == PIECE_SHIFT_LWEAPON )
			{
				m_pSkinChar->SetPartRend( emType, TRUE );
			}
		}*/
		/*Item Self Effect, Juver, 2017/06/09 */
		std::string strSelfEffect = pItem->GetSelfBodyEffect();

		/*ABL system, Juver, 2017/06/02 */
		{
			std::string strFileName = pItem->GetWearingFileR( m_CHARINDEX );

			DxAttBoneLink* pBoneLink = DxAttBoneLinkContainer::GetInstance().LoadData( strFileName.c_str(), m_pd3dDevice, TRUE );
			if ( pBoneLink )	
			{
				m_pSkinChar->SetAttBone( pBoneLink, m_pd3dDevice, NULL, 0, TRUE, strSelfEffect.c_str() );
			}
			else
			{
				m_pSkinChar->SetPiece ( strFileName.c_str(), m_pd3dDevice, NULL, 0, TRUE, strSelfEffect.c_str() );
			}
			m_pSkinChar->SetPartRend ( PIECE_SHIFT_RWEAPON, TRUE );
		}

		{
			std::string strFileName = pItem->GetWearingFileL( m_CHARINDEX );

			DxAttBoneLink* pBoneLink = DxAttBoneLinkContainer::GetInstance().LoadData( strFileName.c_str(), m_pd3dDevice, TRUE );
			if ( pBoneLink )	
			{
				m_pSkinChar->SetAttBone( pBoneLink, m_pd3dDevice, NULL, 0, TRUE, strSelfEffect.c_str() );
			}
			else
			{
				m_pSkinChar->SetPiece ( strFileName.c_str(), m_pd3dDevice, NULL, 0, TRUE, strSelfEffect.c_str() );
			}
			m_pSkinChar->SetPartRend ( PIECE_SHIFT_LWEAPON, TRUE );
		}
	}
}

void GLCharClient::ResetHiddenWeaponPiece()
{
	PDXCHARPART pRHAND_HIDDEN = m_pSkinChar->GetPiece( PIECE_SHIFT_RWEAPON );
	PDXCHARPART pLHAND_HIDDEN = m_pSkinChar->GetPiece( PIECE_SHIFT_LWEAPON );
	if( pRHAND_HIDDEN || pLHAND_HIDDEN )
	{
		m_pSkinChar->SetPartRend ( PIECE_SHIFT_RWEAPON, FALSE );
		m_pSkinChar->SetPartRend ( PIECE_SHIFT_LWEAPON, FALSE );

		m_pSkinChar->ResetPiece( PIECE_SHIFT_RWEAPON );
		m_pSkinChar->ResetPiece( PIECE_SHIFT_LWEAPON );

		// [shhan][2014.11.20] 마술부에서 UpdateSuit 를 하면 망토가 리셋되면서 흔들리는 현상이 있어서 막아봄.
		//						일단 주석해도 될 듯 한데, 문제가 되면 망토도 생각하면서 수정작업 필요함.
		//UpdateSuit();
	}
} 