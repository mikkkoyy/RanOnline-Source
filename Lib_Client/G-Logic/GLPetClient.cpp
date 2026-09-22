#include "StdAfx.h"
#include "GLPetClient.h"
#include "GLGaeaClient.h"
#include "GLContrlMsg.h"
#include "DxGlobalStage.h"
#include "GLItemMan.h"
#include "../Lib_ClientUI/Interface/InnerInterface.h"
#include "../Lib_ClientUI/Interface/UITextcontrol.h"
#include "../Lib_ClientUI/Interface/GameTextcontrol.h"
#include "DxEffGroupPlayer.h"
#include "DxShadowMap.h"
#include "DxEnvironment.h"
#include "GLStrikeM.h"

/*ABL system, Juver, 2017/06/02 */
#include "GLStrikeSelector.h"

GLPetClient::GLPetClient(void) :
	m_pd3dDevice(NULL),
	m_pSkinChar(NULL),

	m_vDir(D3DXVECTOR3(0,0,-1)),
	m_vDirOrig(D3DXVECTOR3(0,0,-1)),
	m_vPos(D3DXVECTOR3(0,0,0)),

	m_vMax(6,20,6),
	m_vMin(-6,0,-6),
	m_fHeight(20.f),

	m_vMaxOrg(6,20,6),
	m_vMinOrg(-6,0,-6),

	m_pOwner(NULL),

	m_bValid(false),

	/*dual pet skill, Juver, 2017/12/27 */
	m_bSkillProcessing_A(false),
	m_bSkillProcessing_B(false),

	m_dwActionFlag(0),
	m_fIdleTimer(0.0f),
	m_fAttackTimer(0.0f),
	m_sPetCrowID(NATIVEID_NULL()),
	m_wColorTEMP(0),
	m_wStyleTEMP(0),
	m_bEnableSytle(FALSE),
	m_bEnableColor(FALSE),
	m_bCannotPickUpItem(false),
	m_bCannotPickUpPileItem(false),
	m_hCheckStrDLL( NULL ),
	m_pCheckString( NULL ),
	m_vRandPos(0.0f,0.0f,0.0f)

{
	SecureZeroMemory ( m_wAniSub,sizeof(WORD)*3 );

	/*dual pet skill, Juver, 2017/12/28 */
	m_vecDroppedItems_A.reserve(50);
	m_vecDroppedItems_B.reserve(50);

	m_sSkillTARGET.RESET ();

	m_SKILLDELAY.clear();

	m_sPetSkinPackState.Init();

#ifdef TH_PARAM
	HMODULE m_hCheckStrDLL = LoadLibrary("ThaiCheck.dll");

	if ( m_hCheckStrDLL )
	{
		m_pCheckString = ( BOOL (_stdcall*)(CString)) GetProcAddress(m_hCheckStrDLL, "IsCompleteThaiChar");
	}
    
#endif
}

GLPetClient::~GLPetClient(void)
{
	SAFE_DELETE ( m_pSkinChar );
#ifdef TH_PARAM
    if ( m_hCheckStrDLL ) FreeLibrary( m_hCheckStrDLL );
#endif
}

HRESULT GLPetClient::Create ( const PGLPET pPetData, D3DXVECTOR3 vPos, D3DXVECTOR3 vDir, NavigationMesh* pNavi, LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( !pNavi || !pd3dDevice ) return E_FAIL;

	m_pd3dDevice = pd3dDevice;

	ASSIGN ( *pPetData );

	m_vPos = vPos;
	m_vDir = vDir;
	m_pOwner = GLGaeaClient::GetInstance().GetCharacter ();

	m_actorMove.Create ( pNavi, m_vPos, -1 );

	if ( FAILED ( SkinLoad ( pd3dDevice ) ) )
		return E_FAIL;

	UpdateSuit ( TRUE );

	// Note : 1.AABB Box를 가져온다. 2.높이를 계산해 놓는다.
	m_pSkinChar->GetAABBBox( m_vMaxOrg, m_vMinOrg );
	m_fHeight = m_vMaxOrg.y - m_vMinOrg.y;
	
	m_bValid = true;

	ReSetAllSTATE ();
	SetSTATE ( EM_PETACT_STOP );

	return S_OK;
}

HRESULT GLPetClient::SkinLoad ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( m_sPetID == NATIVEID_NULL() )
	{
		GASSERT ( 0&&"m_sPetID==NATIVEID_NULL()" );
		return E_FAIL;
	}

	SNATIVEID loadPetID = m_sPetID;

	if( IsUsePetSkinPack() )
	{
		loadPetID = m_sPetSkinPackState.petSkinMobID;
	}
	PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( loadPetID );
	if ( !pCrowData )
	{
		GASSERT ( pCrowData );
		return E_FAIL;
	}

	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().LoadData( 
		pCrowData->GetSkinObjFile(),
		pd3dDevice, 
		TRUE );

	if ( !pSkinChar )
	{
		GASSERT ( pSkinChar );
		return E_FAIL;
	}

	SAFE_DELETE(m_pSkinChar);
	m_pSkinChar = new DxSkinChar;
	m_pSkinChar->SetCharData ( pSkinChar, pd3dDevice );

	

	// 현재 펫 에니메이션중에 플레이가 불가능한 애니메이션을 찾는다.
	if( IsUsePetSkinPack() )
	{
		if( !m_pSkinChar->GETANIAMOUNT( AN_GUARD_N, AN_SUB_NONE )  ) m_sPetSkinPackState.bNotFoundGuard = TRUE;
		if( !m_pSkinChar->GETANIAMOUNT( AN_RUN, AN_SUB_NONE ) )		 m_sPetSkinPackState.bNotFoundRun = TRUE;
		if( !m_pSkinChar->GETANIAMOUNT( AN_WALK, AN_SUB_NONE ) )	 m_sPetSkinPackState.bNotFoundWalk = TRUE;	
//		m_pSkinChar->GETANIAMOUNT( AN_GESTURE, AN_SUB_NONE ) )
		if( !m_pSkinChar->GETANIAMOUNT( AN_ATTACK, AN_SUB_NONE ) )	 m_sPetSkinPackState.bNotFoundAttack = TRUE;
		if( !m_pSkinChar->GETANIAMOUNT( AN_GESTURE, AN_SUB_04 ) )	 m_sPetSkinPackState.bNotFoundSad	 = TRUE;
	}

	
	m_pSkinChar->SELECTANI ( AN_GUARD_N, AN_SUB_NONE );

	return S_OK;
}

HRESULT GLPetClient::UpdateAnimation ( float fTime, float fElapsedTime )
{
	return S_OK;
}

HRESULT GLPetClient::UpdateSuit ( BOOL bChangeStyle )
{

	if( IsUsePetSkinPack() ) return S_OK;

	PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( m_sPetID );
	if ( !pCrowData )					   return E_FAIL;

	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().LoadData( 
		pCrowData->GetSkinObjFile(),
		m_pd3dDevice, 
		TRUE );

	if ( !pSkinChar )					  return E_FAIL;

	const PETSTYLE &sSTYLE = GLCONST_PET::GetPetStyle ( m_emTYPE );

	// 스타일 갱신
	if ( bChangeStyle )
	{
		if ( sSTYLE.wSTYLENum > m_wStyle )
		{
			std::string strSTYLE_CPS = sSTYLE.strSTYLE_CPS[m_wStyle];
		
			PDXCHARPART pCharPart = NULL;
			pCharPart = m_pSkinChar->GetPiece(PIECE_HAIR);		//	현재 장착 스킨.

			if ( pCharPart && strcmp(strSTYLE_CPS.c_str(),pCharPart->m_szFileName) )
			{
				m_pSkinChar->SetPiece ( strSTYLE_CPS.c_str(), m_pd3dDevice );
			}
		}
	}

	// 컬러 갱신 (컬러변경카드 사용중이면 갱신하지 않는다)
	if ( !m_bEnableColor ) m_pSkinChar->SetHairColor( m_wColor );

	// 악세서리 모양 갱신
	for ( WORD i = 0; i < PET_ACCETYPE_SIZE; ++i )
	{
		SITEM* pSlotItem = GLItemMan::GetInstance().GetItem ( m_PutOnItems[i].sNativeID );
		if ( pSlotItem ) m_pSkinChar->SetPiece ( pSlotItem->GetPetWearingFile (), m_pd3dDevice, FG_MUSTNEWLOAD );
		else
		{
			PDXSKINPIECE pSkinPiece = NULL;
			PDXCHARPART pCharPart = NULL;

			EMPIECECHAR emPiece = GetPieceFromSlot ( (PET_ACCESSORY_TYPE)i );
			if ( emPiece != PIECE_SIZE ) 
			{
				pSkinPiece = pSkinChar->GetPiece(emPiece);
				pCharPart = m_pSkinChar->GetPiece(emPiece);

				if ( pSkinPiece )
				{
					if ( pCharPart && strcmp(pSkinPiece->m_szFileName,pCharPart->m_szFileName) )
					{
						m_pSkinChar->SetPiece ( pSkinPiece->m_szFileName, m_pd3dDevice, 0X0, 0, TRUE );
					}
				}else{
					m_pSkinChar->ResetPiece ( emPiece );
				}
			}
		}
	}

	return S_OK;
}

void GLPetClient::SetMoveState ( BOOL bRun = TRUE )
{
	if ( bRun ) 
	{
		m_actorMove.SetMaxSpeed ( m_fRunSpeed );
		SetSTATE ( EM_PETACT_RUN );
		if ( IsSTATE ( EM_PETACT_MOVE ) )
		{
			if( IsUsePetSkinPack() && m_sPetSkinPackState.bNotFoundRun )
			{
				m_pSkinChar->SELECTANI ( AN_WALK, AN_SUB_NONE );				
			}else{
				m_pSkinChar->SELECTANI ( AN_RUN, AN_SUB_NONE );
			}
		}
	}
	else
	{
		m_actorMove.SetMaxSpeed ( m_fWalkSpeed );
		ReSetSTATE ( EM_PETACT_RUN );
		if ( IsSTATE ( EM_PETACT_MOVE ) )
		{
			m_pSkinChar->SELECTANI ( AN_WALK, AN_SUB_NONE );
		}
	}

	// 이동상태를 변경하는 메시지 발송요망
	GLMSG::SNETPET_REQ_UPDATE_MOVESTATE NetMsg;
	NetMsg.dwFlag = m_dwActionFlag;
	NETSENDTOFIELD ( &NetMsg );
}

HRESULT	GLPetClient::UpdatePetState ( float fTime, float fElapsedTime )
{
	D3DXVECTOR3 vOwnerPos, vRandPos, vDist;
	float fDist;

	static float fTimer(0.0f);
	static WORD wSubAni(0);
	static float fIdleTimer(0.0f);

	// 팻이 공격모션 중이면
	if ( IsSTATE ( EM_PETACT_ATTACK ) )
	{
		vOwnerPos = m_pOwner->GetPosition ();
		vDist = m_vPos - vOwnerPos;
		fDist = D3DXVec3Length(&vDist);
		if ( fDist > GLCONST_PET::fWalkArea )
		{
			ReqGoto ( GetRandomOwnerTarget (), false );
		}
		else
		{
			// 공격모션은 아무일 없을때 2초이상 지속하지 않는다.
			m_fAttackTimer += fElapsedTime;
			if ( m_fAttackTimer > 2.0f )
			{
				ReqStop ( true );
			}
		}
	}

	// 슬퍼할때
	if ( IsSTATE ( EM_PETACT_SAD ) ) return S_OK;

	// 멈춰있을때
	if ( IsSTATE ( EM_PETACT_STOP ) )
	{
		vOwnerPos = m_pOwner->GetPosition ();
		vDist = m_vPos - vOwnerPos;
		fDist = D3DXVec3Length(&vDist);
		if ( fDist > GLCONST_PET::fWalkArea )
		{
			ReqGoto ( GetRandomOwnerTarget (), false );
		}
		else
		{
			m_vRandPos = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

			m_fIdleTimer += fElapsedTime;
			if ( m_fIdleTimer > 2.0f )
			{
				ReSetSTATE ( EM_PETACT_STOP );
				SetSTATE ( EM_PETACT_FUNNY );
				MakeAniSubKey ( fTime );

				// Funny Action
				m_pSkinChar->SELECTANI ( AN_GESTURE, (EMANI_SUBTYPE) m_wAniSub[wSubAni] );

				GLMSG::SNETPET_REQ_FUNNY NetMsg;
				NetMsg.wFunny = m_wAniSub[wSubAni];
				NETSENDTOFIELD ( &NetMsg );

				m_fIdleTimer = 0.0f;
			}
		}

		return S_OK;
	}

	// FUNNY 액션중이면
	if ( IsSTATE ( EM_PETACT_FUNNY ) )
	{
		vOwnerPos = m_pOwner->GetPosition ();
		vDist = m_vPos - vOwnerPos;
		fDist = D3DXVec3Length(&vDist);
		if ( fDist > GLCONST_PET::fWalkArea )
		{
			ReqGoto ( GetRandomOwnerTarget (), false );
			wSubAni = 0;
		}
		else
		{
			PSANIMCONTAINER pANIMCON = m_pSkinChar->GETCURANIM ();

			// Funny 애니메이션이 끝났으면 Idle 애니메이션 설정
			if ( pANIMCON->m_MainType == AN_GESTURE && pANIMCON->m_SubType == m_wAniSub[wSubAni] )
			{
				if ( m_pSkinChar->ISENDANIM () )
				{
					m_pSkinChar->SELECTANI ( AN_GUARD_N, AN_SUB_NONE );

					GLMSG::SNETPET_REQ_STOP NetMsg;
					NetMsg.dwGUID      = m_dwGUID;
					NetMsg.dwFlag      = EM_PETACT_STOP;
					NetMsg.vPos	       = m_vPos;
					NetMsg.bStopAttack = true; // 제자리에 그냥 서있게 하기 위해
					NETSENDTOFIELD ( &NetMsg );

					if ( ++wSubAni > 2 ) 
					{
						// Funny 애니메이션 순서 재설정
						wSubAni = 0;
						MakeAniSubKey ( fTime );
					}

					fIdleTimer = 0.0f;
				}
			}
			// Idle 애니메이션이 끝나면 Funny 애니메이션 설정
			else if ( pANIMCON->m_MainType == AN_GUARD_N && pANIMCON->m_SubType == AN_SUB_NONE )
			{
				fIdleTimer += fElapsedTime;
				// 애니메이션 설정에 루프가 설정되어 있으므로 끝남 유무를 알수 없으므로 시간으로 제어함.
				if ( fIdleTimer > 3.0f )
				{
					// Funny Action
					m_pSkinChar->SELECTANI ( AN_GESTURE, (EMANI_SUBTYPE) m_wAniSub[wSubAni] );

					GLMSG::SNETPET_REQ_FUNNY NetMsg;
					NetMsg.wFunny = m_wAniSub[wSubAni];
					NETSENDTOFIELD ( &NetMsg );

					fIdleTimer = 0.0f;
				}
			}
		}
	}

	// 주인이 공격/스킬 중일때
	if ( m_pOwner->IsACTION ( GLAT_ATTACK ) || m_pOwner->IsACTION ( GLAT_SKILL ) )
	{
		vOwnerPos = m_pOwner->GetPosition ();
		vDist = m_vPos - vOwnerPos;
		fDist = D3DXVec3Length(&vDist);
		if ( fDist > GLCONST_PET::fWalkArea )
		{
			D3DXVECTOR3 vRandPos = GLPETDEFINE::GetRandomPostision ();
			vOwnerPos += vRandPos;
			ReqGoto ( vOwnerPos, false );
		}
	}

	// 이동중 주인을 기다릴때
	if ( IsSTATE ( EM_PETACT_WAIT ) )
	{
		vOwnerPos = m_pOwner->GetPosition ();

		if ( m_pOwner->IsACTION ( GLAT_IDLE ) )
		{
			D3DXVECTOR3 vRandPos = GLPETDEFINE::GetRandomPostision ();
			vOwnerPos = m_pOwner->GetPosition ();
			vOwnerPos += vRandPos;
			ReqGoto ( vOwnerPos, false );
			return S_OK;
		}

		vDist = vOwnerPos - m_vPos;
		float fDist = D3DXVec3Length(&vDist);
		if ( fDist > GLCONST_PET::fWalkArea )
		{
			D3DXVECTOR3 vOwnerBack = m_vPos - vOwnerPos;
			D3DXVec3Normalize ( &vOwnerBack, &vOwnerBack );
			vOwnerPos += (vOwnerBack*GLCONST_PET::fOwnerDistance);

			ReqGoto ( vOwnerPos, false );
		}

		m_fIdleTimer = 0.0f;
	}

	if ( IsSTATE ( EM_PETACT_MOVE ) )
	{
		// 아이템을 주우러 달릴때
		if ( IsSTATE ( EM_PETACT_RUN_PICKUP_ITEM ) )
		{
			vDist = m_vPos - m_sSkillTARGET.vPos;
			float fDist = D3DXVec3Length(&vDist);
			if ( !m_actorMove.PathIsActive() )
			{
				// 아이템을 줍는다.
				m_pOwner->ReqFieldTo ( m_sSkillTARGET, true );
				ReqStop ();
				return S_OK;
			}
		}
		else
		{
			vOwnerPos = m_pOwner->GetPosition ();
			vDist = m_vPos - vOwnerPos;
			fDist = D3DXVec3Length(&vDist);

			STARGETID sTarget = m_pOwner->GetTargetID ();

			if ( !m_actorMove.PathIsActive() )
			{
				ReqStop ();
				return S_OK;
			}

			// 1초에 한번씩 이동메시지를 전송한다. (이부분은 수정해봐야 할듯하다)
			fTimer += fElapsedTime;
			if ( fTimer > 1.0f && m_pOwner->IsACTION ( GLAT_MOVE ) )
			{
				// D3DXVECTOR3 vRandPos = GLPETDEFINE::GetRandomPostision ();
				vOwnerPos = m_pOwner->GetPosition ();

				D3DXVECTOR3 vOwnerBack = m_vPos - vOwnerPos;
				D3DXVec3Normalize ( &vOwnerBack, &vOwnerBack );
				vOwnerPos += (vOwnerBack*GLCONST_PET::fOwnerDistance);

				ReqGoto ( vOwnerPos, false );
				fTimer = 0.0f;
			}
			
			// 주인이 걸으면 거리에 따라 뛰거나 걷는다
			// [주의] 위의 코드에서 이미 팻의 움직임 상태를 변경했으므로 
			//		   주인의 걷는 상태에 대한 처리만 해주면 된다.
			if ( !m_pOwner->IsSTATE ( EM_ACT_RUN ) )
			{
				vDist = m_vPos - vOwnerPos;
				float fDist = D3DXVec3Length(&vDist);

				// 주인과 거리가 너무 멀어지면 일정거리를 유지하기 위해 달린다
				if ( fDist > GLCONST_PET::fRunArea && !IsSTATE ( EM_PETACT_RUN_CLOSELY ) )
				{
					m_actorMove.SetMaxSpeed ( m_fRunSpeed );
					SetSTATE ( EM_PETACT_RUN_CLOSELY );
					SetMoveState ();
				}
				else if ( IsSTATE ( EM_PETACT_RUN_CLOSELY ) )
				{
					if ( fDist <= GLCONST_PET::fWalkArea )
					{
						m_actorMove.SetMaxSpeed ( m_fWalkSpeed );
						ReSetSTATE ( EM_PETACT_RUN_CLOSELY );
						SetMoveState ( FALSE );
					}
				}

				// 주인은 멈춰있는데 걷고 있다면
				if ( m_pOwner->IsACTION ( GLAT_IDLE ) )
				{
					SetMoveState ();
				}
			}
		}
		m_fIdleTimer = 0.0f;
	}

	return S_OK;
}

EM_FIELDITEM_STATE GLPetClient::IsInsertableInven ( STARGETID sTar )
{
	PLANDMANCLIENT pLAND = GLGaeaClient::GetInstance().GetActiveMap();
	if ( !pLAND ) return EM_FIELDITEM_STATE_GENERALFAIL;

	if ( m_sSkillTARGET.emCrow==CROW_ITEM )
	{
		PITEMCLIENTDROP pItemDrop = pLAND->GetItem ( m_sSkillTARGET.dwID );
		if ( !pItemDrop ) return EM_FIELDITEM_STATE_REMOVED;

		SITEM *pItem = GLItemMan::GetInstance().GetItem(pItemDrop->sItemClient.sNativeID);
		if ( !pItem )		return EM_FIELDITEM_STATE_GENERALFAIL;

		BOOL bOk = m_pOwner->IsInsertToInven ( pItemDrop );
		if ( !bOk ) return EM_FIELDITEM_STATE_INSERTFAIL;
	}
	else if ( m_sSkillTARGET.emCrow==CROW_MONEY )
	{
		PMONEYCLIENTDROP pMoney = pLAND->GetMoney ( m_sSkillTARGET.dwID );
		if ( !pMoney )	return EM_FIELDITEM_STATE_REMOVED;
	}

	return EM_FIELDITEM_STATE_INSERTOK;
}

bool GLPetClient::CheckSkill ( SNATIVEID sSkillID )
{
	PETSKILL_MAP_CITER learniter = m_ExpSkills.find ( sSkillID.dwID );
	if ( learniter==m_ExpSkills.end() )
	{
		// 배우지 않은 스킬
		return false;
	}

	PETDELAY_MAP_ITER delayiter = m_SKILLDELAY.find ( sSkillID.dwID );
	if ( delayiter!=m_SKILLDELAY.end() )
	{
		// 스킬딜레이
		return false;
	}
	return true;
}

float GLPetClient::GetSkillDelayPercent ( SNATIVEID sSkillID )
{
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sSkillID );
	if ( !pSkill )					     return 0.0f;

	PETDELAY_MAP_ITER delayiter = m_SKILLDELAY.find ( sSkillID.dwID );
	if ( delayiter==m_SKILLDELAY.end() ) return 0.0f;

	SKILL::CDATA_LVL &sSKILL_DATA = pSkill->m_sAPPLY.sDATA_LVL[0];
	float fSkillDelay = sSKILL_DATA.fDELAYTIME;
	float fCurDelay = (*delayiter).second;

	if ( fSkillDelay == 0.0f ) fSkillDelay = 1.0f;

	return fCurDelay / fSkillDelay;
}

void GLPetClient::AccountSkill ( SNATIVEID sSkillID )
{
	PETSKILL_MAP_ITER iter = m_ExpSkills.find ( sSkillID.dwID );
	if ( iter == m_ExpSkills.end() ) return;
	PETSKILL sPetSkill = (*iter).second;

	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sSkillID );
	if ( !pSkill )					 return;
	SKILL::CDATA_LVL &sSKILL_DATA = pSkill->m_sAPPLY.sDATA_LVL[sPetSkill.wLevel];

	m_SKILLDELAY.insert ( std::make_pair(sSkillID.dwID, sSKILL_DATA.fDELAYTIME) );
}

void GLPetClient::UpdateSkillDelay ( float fElapsedTime )
{
	DELAY_MAP_ITER iter_del;

	PETDELAY_MAP_ITER iter = m_SKILLDELAY.begin ();
	PETDELAY_MAP_ITER iter_end = m_SKILLDELAY.end ();

	for ( ; iter!=iter_end; )
	{
		float &fDelay = (*iter).second;
		iter_del = iter++;

		fDelay -= fElapsedTime;
		if ( fDelay < 0.0f )	m_SKILLDELAY.erase ( iter_del );
	}
}

HRESULT GLPetClient::FrameMove( float fTime, float fElapsedTime )
{
	if ( !m_bValid ) return S_OK;

	HRESULT hr(S_OK);
	
	UpdatePetState ( fTime, fElapsedTime );

	/*dual pet skill, Juver, 2017/12/28 */
	ProceedSkill_A ();
	ProceedSkill_B ();

	// UpdateSkillDelay ( fElapsedTime ); 기획팀 김병욱씨 요청으로 제거 [06.10.09]

	// ============== Update about Actor (Begin) ============== /

	hr = m_actorMove.Update ( fElapsedTime );
	if ( FAILED(hr) )	return E_FAIL;

	// ============== Update about Actor (end)   ============== /

	// ============== Update about Skin (Begin) ============== /

	D3DXMATRIX matTrans, matYRot;
	D3DXMatrixTranslation ( &matTrans, m_vPos.x, m_vPos.y, m_vPos.z );
	float fThetaY = DXGetThetaYFromDirection ( m_vDir, m_vDirOrig );
	D3DXMatrixRotationY ( &matYRot, fThetaY );
	m_matTrans = matYRot * matTrans;
	
	m_pSkinChar->SetPosition ( m_vPos );

	if( IsUsePetSkinPack() && IsSTATE ( EM_PETACT_MOVE ) )
	{
		float fSpeedGap1, fSpeedGap2, fTempTime, fTempElapsedTime;

		fSpeedGap1 = 1.0f / m_sPetSkinPackState.fPetScale;
		if( IsSTATE( EM_PETACT_RUN ) && m_sPetSkinPackState.bNotFoundRun ) fSpeedGap2 = m_fRunSpeed / m_fWalkSpeed;
		else															  fSpeedGap2 = 1.0f;

		fTempTime		 = fTime * fSpeedGap1 * fSpeedGap2;
		fTempElapsedTime = fElapsedTime * fSpeedGap1 * fSpeedGap2;

		m_pSkinChar->FrameMove ( fTempTime, fTempElapsedTime );
   	}else{
		m_pSkinChar->FrameMove ( fTime, fElapsedTime );
	}

	// ============== Update about Skin (End)   ============== /

	// 현재 위치 업데이트
	m_vPos = m_actorMove.Position();

	// 방향 업데이트
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

	//	Note : AABB 계산.
	//
	m_vMax = m_vPos + m_vMaxOrg;
	m_vMin = m_vPos + m_vMinOrg;

	return S_OK;
}

HRESULT GLPetClient::Render( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( !m_bValid )   return S_OK;
	if ( !pd3dDevice ) return E_FAIL;

	// 펫은 피스 순서를 거꾸로 그린다.
	if ( m_pSkinChar ) 
	{
		if( IsUsePetSkinPack() )
		{
			D3DXMATRIX matScale, matRender;
			D3DXMatrixScaling( &matScale, m_sPetSkinPackState.fPetScale, m_sPetSkinPackState.fPetScale, m_sPetSkinPackState.fPetScale );
			matRender = matScale * m_matTrans;
			m_pSkinChar->Render( pd3dDevice, matRender, FALSE, TRUE, TRUE );
		}else{
			m_pSkinChar->Render( pd3dDevice, m_matTrans, FALSE, TRUE, TRUE );
		}
	}
	
	return S_OK;
}

HRESULT GLPetClient::RenderShadow ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( !m_bValid )   return S_OK;
	if ( !pd3dDevice ) return E_FAIL;

	//	Note : 그림자 랜더링.
	//
	if ( m_pSkinChar )
	{
		DxShadowMap::GetInstance().RenderShadowCharMob ( m_pSkinChar, m_matTrans, pd3dDevice );
	}

	return S_OK;
}

HRESULT GLPetClient::RenderReflect( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( !m_bValid )   return S_OK;
	if ( !pd3dDevice ) return E_FAIL;

	//	Note : 반사 랜더링.
	//
	if ( m_pSkinChar )
	{
		DxEnvironment::GetInstance().RenderRefelctChar( m_pSkinChar, m_matTrans, pd3dDevice );
	}

	return S_OK;
}

HRESULT GLPetClient::RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	//if ( !m_bValid ) return S_OK;

	if ( m_pSkinChar )
	{
		m_pSkinChar->RestoreDeviceObjects ( pd3dDevice );
	}
	
	return S_OK;
}

HRESULT GLPetClient::InvalidateDeviceObjects ()
{
	//if ( !m_bValid ) return S_OK;

	if ( m_pSkinChar )
	{
		m_pSkinChar->InvalidateDeviceObjects ();
	}
	
	return S_OK;
}

HRESULT GLPetClient::DeleteDeviceObjects ()
{
	//if ( !m_bValid ) return S_OK;

	if ( m_pSkinChar )
	{
		m_pSkinChar->DeleteDeviceObjects ();
		SAFE_DELETE( m_pSkinChar );
	}

	m_actorMove.Stop ();
	m_actorMove.Release ();

	m_pd3dDevice = NULL;
	m_pOwner	 = NULL;
	m_vDir	     = D3DXVECTOR3 (0,0,-1);
	m_vPos	     = D3DXVECTOR3(0,0,0);
	m_vDirOrig   = D3DXVECTOR3(0,0,-1);
	m_bValid	 = false;

	/*dual pet skill, Juver, 2017/12/28 */
	m_bSkillProcessing_A = false;
	m_bSkillProcessing_B = false;

	ReSetAllSTATE ();
	SetSTATE ( EM_PETACT_STOP );

	m_sSkillTARGET.RESET ();
	m_fIdleTimer = 0.0f;
	m_fAttackTimer = 0.0f;
	SecureZeroMemory ( m_wAniSub,sizeof(WORD)*3 );

	m_sPetCrowID = NATIVEID_NULL();

	m_SKILLDELAY.clear();

	m_bCannotPickUpPileItem = false;
	m_bCannotPickUpItem = false;

	/*dual pet skill, Juver, 2017/12/28 */
	m_vecDroppedItems_A.clear();
	m_vecDroppedItems_B.clear();

	RESET ();

	return S_OK;
}

/*skill activated effect, Juver, 2017/12/29 */
DxEffSingleGroup* GLPetClient::SK_EFF_SELFZONE ( const SANIMSTRIKE &sStrikeEff, const SKILL::EMEFFECTPOS emPOS, const char* const szZONE_EFF, STARGETID *pTarget )
{
	D3DXMATRIX matTrans;

	//	스트라이크 지점에서.
	BOOL bTRANS(FALSE);
	if ( emPOS == SKILL::EMPOS_STRIKE )
	{
		//	타겟의 위치.
		D3DXVECTOR3 vTARPOS = m_vPos + m_vDir * 60.0f;

		/*ABL system, Juver, 2017/06/02 */
		D3DXVECTOR3 vOUT(0.0f,0.0f,0.0f);
		BOOL bOk = STRIKE_SELECT::SelectStrike( m_pSkinChar, vTARPOS, vOUT, sStrikeEff );
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

//----------------------------------------------------------------------------------------------------------------------------------
//								H	a	i	r				S	t	y	l	e	 &&	 C	o	l	o	r
//----------------------------------------------------------------------------------------------------------------------------------
void GLPetClient::StyleInitData()
{
	m_bEnableSytle = TRUE;
	
	m_wStyleTEMP = m_wStyle;

	// 컬러 변화 중일 경우 건들지 않는다.
	if( !m_bEnableColor )
	{
		m_wColorTEMP = m_wColor;
	}
}

void GLPetClient::ColorInitData()
{
	m_bEnableColor = TRUE;

	m_wColorTEMP = m_wColor;

	// 스타일 변화 중일 경우 건들지 않는다.
	if( !m_bEnableSytle )
	{
		m_wStyleTEMP = m_wStyle;
	}
}

void GLPetClient::StyleChange( WORD wStyle )
{
	m_wStyleTEMP = wStyle; 

	if ( m_wColor == GLCONST_PET::sPETSTYLE[m_emTYPE].wSTYLE_COLOR[m_wStyle] && !m_bEnableColor )
	{
		m_wColorTEMP = GLCONST_PET::sPETSTYLE[m_emTYPE].wSTYLE_COLOR[m_wStyleTEMP];
	}

	StyleUpdate();
	ColorUpdate();
}

void GLPetClient::ColorChange( WORD wColor )
{ 
	m_wColorTEMP = wColor; 

	ColorUpdate();
}

void GLPetClient::StyleUpdate()
{
	PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( m_sPetID );
	if ( !pCrowData )					   return;

	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().LoadData( 
		pCrowData->GetSkinObjFile(),
		m_pd3dDevice, 
		TRUE );

	if ( !pSkinChar )					  return;

	const PETSTYLE &sSTYLE = GLCONST_PET::GetPetStyle ( m_emTYPE );

	// 헤어스타일.
	if ( sSTYLE.wSTYLENum > m_wStyleTEMP )
	{
		std::string strHAIR_CPS = sSTYLE.strSTYLE_CPS[m_wStyleTEMP];
	
		PDXCHARPART pCharPart = NULL;
		pCharPart = m_pSkinChar->GetPiece(PIECE_HAIR);		//	현재 장착 스킨.

		if( pCharPart && strcmp(strHAIR_CPS.c_str(),pCharPart->m_szFileName) )
		{
			m_pSkinChar->SetPiece( strHAIR_CPS.c_str(), m_pd3dDevice, NULL, NULL );
		}
	}

	// Note : 이것을 호출하면서.. 머리형이 원상태로 바뀌어 버린다.
	//		그래서 FALSE 호출로 머리모양을 안바뀌게 한다.
	//UpdateSuit( FALSE );
}

void GLPetClient::ColorUpdate()
{
	PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( m_sPetID );
	if ( !pCrowData )					   return;

	DxSkinCharData* pSkinChar = DxSkinCharDataContainer::GetInstance().LoadData( 
		pCrowData->GetSkinObjFile(),
		m_pd3dDevice, 
		TRUE );

	if ( !pSkinChar )					  return;

	m_pSkinChar->SetHairColor( m_wColorTEMP );
}

void GLPetClient::MakeAniSubKey ( float fTime )
{
	static bool bPlus(true);

	srand ( unsigned int(fTime) );
	m_wAniSub[0] = rand()%3+1;
	m_wAniSub[1] = rand()%3+1;
	if ( m_wAniSub[0] == m_wAniSub[1] )
	{
		if ( bPlus )
		{
			m_wAniSub[1] += 1;
			if ( m_wAniSub[1] > 3 ) m_wAniSub[1] = 1;
		}
		else
		{
			m_wAniSub[1] -= 1;
			if ( m_wAniSub[1] < 1 ) m_wAniSub[1] = 3;
		}
		bPlus = !bPlus;
	}
	for ( int i = 1; i < 4; ++i )
	{
		if ( m_wAniSub[0] != i && m_wAniSub[1] != i )
		{
			m_wAniSub[2] = i;
			break;
		}
	}
}

void GLPetClient::GetSkinAABB( D3DXVECTOR3& vMin, D3DXVECTOR3& vMax )
{ 
	if( m_pSkinChar )
		m_pSkinChar->GetAABBBox( vMax, vMin );
}

float GLPetClient::GetSkinScale()
{ 
	if( !m_pSkinChar )	return 1.0f;
	return m_pSkinChar->GetScale();
}

float GLPetClient::GetSkinHeight()
{ 
	if( !m_pSkinChar )	return 20.0f;
	return m_pSkinChar->m_fHeight;
}
