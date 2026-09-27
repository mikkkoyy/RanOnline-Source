#include "StdAfx.h"
#include "GLGaeaClient.h"
#include "GLContrlMsg.h"
#include "DxShadowMap.h"
#include "GLItemMan.h"
#include "GLCharDefine.h"
#include "GLFactEffect.h"
#include "GLStrikeM.h"
#include "DxEffGroupPlayer.h"

#include "../Lib_ClientUI/Interface/DamageDisplay.h"
#include "../Lib_ClientUI/Interface/InnerInterface.h"
#include "../Lib_ClientUI/Interface/UITextcontrol.h"
#include "../Lib_ClientUI/Interface/GameTextcontrol.h"
#include "../../Lib_Engine/DxEffect/Char/DxEffCharData.h"


#include ".\glanysummon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void GLAnySummon::MsgUpdateMoveState ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_SUMMON_REQ_UPDATE_MOVESTATE_BRD* pNetMsg = 
		( GLMSG::SNET_SUMMON_REQ_UPDATE_MOVESTATE_BRD* ) nmg;

	m_dwActionFlag = pNetMsg->dwFlag;

	switch ( m_dwActionFlag )
	{
	case EM_SUMMONACT_ATTACK:
		TurnAction(GLAT_ATTACK);
		break;
	case EM_SUMMONACT_TRACING:
	case EM_SUMMONACT_MOVE:
	case EM_SUMMONACT_RUN:
	case EM_SUMMONACT_RUN_CLOSELY:
		TurnAction(GLAT_MOVE);
		break;
	case EM_SUMMONACT_STOP:
		TurnAction(GLAT_IDLE);
		break;

	}

	if ( IsSTATE ( EM_SUMMONACT_RUN ) )	m_actorMove.SetMaxSpeed ( m_pSummonCrowData->m_sAction.m_fRunVelo  );
	else								m_actorMove.SetMaxSpeed ( m_pSummonCrowData->m_sAction.m_fWalkVelo );
}

void GLAnySummon::MsgGoto ( NET_MSG_GENERIC* nmg )
{
	if ( !m_pSummonCrowData ) return;

	GLMSG::SNET_SUMMON_GOTO_BRD* pNetMsg = ( GLMSG::SNET_SUMMON_GOTO_BRD* ) nmg;

	m_dwActionFlag = pNetMsg->dwFlag;

	BOOL bSucceed = m_actorMove.GotoLocation
		(
		pNetMsg->vTarPos+D3DXVECTOR3(0,+10,0),
		pNetMsg->vTarPos+D3DXVECTOR3(0,-10,0)
		);

	BOOL bReqRun = pNetMsg->dwFlag&EM_SUMMONACT_RUN;

	if ( IsSTATE ( EM_SUMMONACT_RUN ) )
	{
		//		m_pSkinChar->SELECTANI ( AN_RUN, AN_SUB_NONE );
		m_actorMove.SetMaxSpeed ( m_pSummonCrowData->m_sAction.m_fRunVelo );		
	}
	else
	{
		//		m_pSkinChar->SELECTANI ( AN_WALK, AN_SUB_NONE );
		m_actorMove.SetMaxSpeed ( m_pSummonCrowData->m_sAction.m_fWalkVelo );
	}

	TurnAction( GLAT_MOVE );
}

void GLAnySummon::MsgStop ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_SUMMON_STOP_BRD* pNetMsg = ( GLMSG::SNET_SUMMON_STOP_BRD* ) nmg;

	// 멈춰야 한다면 현재 네비게이션을 끝까지 가게 해줘야 한다.
	BOOL bReqStop = pNetMsg->dwFlag&EM_SUMMONACT_STOP;
	if ( bReqStop && !pNetMsg->bStopAttack )	return;

	m_dwActionFlag = pNetMsg->dwFlag;

	TurnAction( GLAT_IDLE );

	// 기다려야 하거나 어텍모션을 멈춰야 한다면
	if ( IsSTATE ( EM_SUMMONACT_WAIT ) || pNetMsg->bStopAttack )
	{
		m_actorMove.Stop ();
		m_pSkinChar->SELECTANI ( AN_GUARD_N, AN_SUB_NONE );
	}
}

void GLAnySummon::MsgAttack ( NET_MSG_GENERIC* nmg )
{
	// 이동중이면 이동을 마친후 공격 모션을 취한다.
	if ( m_actorMove.PathIsActive () ) 
	{
		m_actorMove.Stop();
		///*if( IsSTATE( EM_SUMMONACT_TRACING ) )
		//{
		//m_actorMove.Stop();
		//}else*/{
		//return;
		//}
	}

	GLMSG::SNET_SUMMON_ATTACK_BRD* pNetMsg = ( GLMSG::SNET_SUMMON_ATTACK_BRD* ) nmg;

	m_idACTIVESKILL = pNetMsg->sidSkill;
	m_wACTIVESKILL_LVL = pNetMsg->wSkillLevel;
	m_wTARNUM = pNetMsg->wTARNUM;
	for( int i=0; i<EMTARGET_NET; ++i )
		m_sTARIDS[i] = pNetMsg->sTARIDS[i];

	m_dwActionFlag = EM_SUMMONACT_ATTACK;
	TurnAction( GLAT_ATTACK );

	m_sTargetID	   = pNetMsg->sTarID;
	m_dwAType	   = pNetMsg->dwAType;

	if ( pNetMsg->sTarID.emCrow == CROW_PC )
	{
		//GLLandManClient* pLandMan = GLGaeaClient::GetInstance().GetActiveMap ();
		//PGLCHARCLIENT pChar = pLandMan->GetChar ( pNetMsg->sTarID.dwID );
		GLCOPY* pCopy =  GLGaeaClient::GetInstance().GetCopyActor( pNetMsg->sTarID );
		if ( pCopy )
		{
			// 방향 전환
			D3DXVECTOR3 vDirection = pCopy->GetPosition () - m_vPos;
			D3DXVec3Normalize ( &m_vDir, &vDirection );
			m_vDir = vDirection;

			m_pSkinChar->SELECTANI ( AN_ATTACK, AN_SUB_NONE );
		}
		// 다른 PC 가 아니면 플레이어 케릭터이다
		else
		{
			// 방향 전환
			D3DXVECTOR3 vDirection = GLGaeaClient::GetInstance().GetCharacter()->GetPosition () - m_vPos;
			D3DXVec3Normalize ( &m_vDir, &vDirection );
			m_vDir = vDirection;

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

			m_pSkinChar->SELECTANI ( AN_ATTACK, AN_SUB_NONE );
		}
	}

}

void GLAnySummon::MsgSkillFact( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_SKILLFACT_BRD *pNetMsg = (GLMSG::SNETPC_SKILLFACT_BRD *)nmg;

	if( pNetMsg->emCrow != CROW_SUMMON ) return;

	GLOGICEX::VARIATION ( m_dwNowHP, m_pSummonCrowData->m_dwHP, pNetMsg->nVAR_HP );
	GLOGICEX::VARIATION ( m_wNowMP, m_pSummonCrowData->m_wMP, pNetMsg->nVAR_MP );
	GLOGICEX::VARIATION ( m_wNowSP, m_pSummonCrowData->m_wSP, pNetMsg->nVAR_SP );

	//	데미지 메시지.
	if ( pNetMsg->nVAR_HP < 0 )
	{
		if ( pNetMsg->dwDamageFlag & DAMAGE_TYPE_SHOCK )	
		{
			//					SetSTATE ( EM_SUMMONACT_SHOCK );
			TurnAction ( GLAT_SHOCK );
		}
		else					ReceiveSwing ();

		D3DXVECTOR3 vPos = GetPosBodyHeight();
		CInnerInterface::GetInstance().SetDamage( vPos, static_cast<DWORD>(-pNetMsg->nVAR_HP), pNetMsg->dwDamageFlag, UI_ATTACK );

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
}

void GLAnySummon::MsgProcess ( NET_MSG_GENERIC* nmg )
{
	switch ( nmg->nType )
	{
	case NET_MSG_SUMMON_GOTO_BRD:						MsgGoto ( nmg );			break;
	case NET_MSG_SUMMON_REQ_UPDATE_MOVE_STATE_BRD:		MsgUpdateMoveState ( nmg ); break;
	case NET_MSG_SUMMON_REQ_STOP_BRD:					MsgStop ( nmg );			break;
	case NET_MSG_SUMMON_ATTACK_BRD:						MsgAttack ( nmg );			break;

	case NET_MSG_GCTRL_SKILLFACT_BRD:					MsgSkillFact ( nmg );		break;
	case NET_MSG_GCTRL_SKILLHOLD_BRD:
		{
			GLMSG::SNETPC_SKILLHOLD_BRD *pNetMsg = (GLMSG::SNETPC_SKILLHOLD_BRD *)nmg;

			//	지속형 스킬의 경우 스킬 팩터 추가됨.
			if ( pNetMsg->skill_id != SNATIVEID(false) )
			{
				RECEIVE_SKILLFACT ( pNetMsg->skill_id, pNetMsg->wLEVEL, pNetMsg->wSELSLOT, pNetMsg->wCasterCrow, pNetMsg->dwCasterID );
				FACTEFF::NewSkillFactEffect ( STARGETID(CROW_SUMMON,m_dwGUID,m_vPos), m_pSkinChar, pNetMsg->skill_id, m_matTrans, m_vDir );
			}
		}
		break;
	case NET_MSG_GCTRL_REMOVE_SKILL_EFFECT_BRD:
		{
			GLMSG::SNETPC_REMOVE_SKILL_EFFECT_BRD *pNetMsg = (GLMSG::SNETPC_REMOVE_SKILL_EFFECT_BRD *)nmg;
			
			FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_SUMMON,m_dwGUID,m_vPos), m_pSkinChar, m_sSKILLFACT[pNetMsg->dwSkillID].sNATIVEID );

			DISABLESKEFF(pNetMsg->dwSkillID);
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
					FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_SUMMON,m_dwGUID,m_vPos), m_pSkinChar, m_sSKILLFACT[i].sNATIVEID );

					DISABLESKEFF(i);
				}
			}
		}
		break;

	case NET_MSG_GCTRL_STATEBLOW_BRD:
		{
			GLMSG::SNETPC_STATEBLOW_BRD *pNetMsg = (GLMSG::SNETPC_STATEBLOW_BRD *)nmg;

			//	Note : "단독상태이상"일 경우 단독 효과 모두 끄기.
			if ( pNetMsg->emBLOW <= EMBLOW_SINGLE )
				FACTEFF::DeleteBlowSingleEffect ( STARGETID(CROW_SUMMON,m_dwGUID,m_vPos), m_pSkinChar, m_sSTATEBLOWS );

			//	Note : 상태 이상 추가.
			SSTATEBLOW *pSTATEBLOW = NULL;
			if ( pNetMsg->emBLOW <= EMBLOW_SINGLE )		pSTATEBLOW = &m_sSTATEBLOWS[0];
			else										pSTATEBLOW = &m_sSTATEBLOWS[pNetMsg->emBLOW-EMBLOW_SINGLE];
			pSTATEBLOW->emBLOW = pNetMsg->emBLOW;
			pSTATEBLOW->fAGE = pNetMsg->fAGE;
			pSTATEBLOW->fSTATE_VAR1 = pNetMsg->fSTATE_VAR1;
			pSTATEBLOW->fSTATE_VAR2 = pNetMsg->fSTATE_VAR2;

			//	Note : 효과 생성.
			FACTEFF::NewBlowEffect ( STARGETID(CROW_SUMMON,m_dwGUID,m_vPos), m_pSkinChar, pSTATEBLOW->emBLOW, m_matTrans, m_vDir );
		}
		break;

	case NET_MSG_GCTRL_CURESTATEBLOW_BRD:
		{
			GLMSG::SNETPC_CURESTATEBLOW_BRD *pNetMsg = (GLMSG::SNETPC_CURESTATEBLOW_BRD *)nmg;

			for ( int i=0; i<EMBLOW_MULTI; ++i )
			{
				EMSTATE_BLOW emBLOW = m_sSTATEBLOWS[i].emBLOW;
				if ( emBLOW==EMBLOW_NONE )		continue;

				EMDISORDER emDIS = STATE_TO_DISORDER(emBLOW);
				if ( !(pNetMsg->dwCUREFLAG&emDIS) )				continue;

				DISABLEBLOW(i);
				FACTEFF::DeleteBlowEffect ( STARGETID(CROW_SUMMON,m_dwGUID,m_vPos), m_pSkinChar, emBLOW );
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

	case NET_MSG_GCTRL_POSITIONCHK_BRD:
		{
			GLMSG::SNET_POSITIONCHK_BRD *pNetNsg = (GLMSG::SNET_POSITIONCHK_BRD *)nmg;
			m_vServerPos = pNetNsg->vPOS;
		}
		break;

		/*skill summon, Juver, 2017/10/09 */
	case NET_MSG_SUMMON_END_REST_BRD:
		{
			m_dwActionFlag = 0;
			SetSTATE( EM_SUMMONACT_STOP );
			TurnAction( GLAT_IDLE );

			if ( m_strRestEffect.size() )
			{
				STARGETID sTargetID(CROW_SUMMON, m_dwGUID, GetPosition());
				DxEffGroupPlayer::GetInstance().DeletePassiveEffect ( m_strRestEffect.c_str(), sTargetID );
				m_fRestTime = 0.0f;
			}
		}break;

		/*skill summon, Juver, 2017/10/10 */
	case NET_MSG_SUMMON_END_LIFE_BRD:
		{
			m_dwNowHP = 0;
			TurnAction ( GLAT_FALLING );
		}break;

		/*skill summon, Juver, 2017/10/10 */
	case NET_MSG_SUMMON_RESET_TARGET_BRD:
		{
			m_sTargetID.RESET();
			TurnAction( GLAT_IDLE );
		}break;

	//Teleport Skill Effect
	case NET_MSG_GCTRL_TELEPORT_BRD:
		{
			GLMSG::SNET_TELEPORT_BRD *pNetMsg = (GLMSG::SNET_TELEPORT_BRD *)nmg;
			const D3DXVECTOR3 &vMovePos = pNetMsg->vMovePos;

			m_vPos = pNetMsg->vMovePos;
			m_actorMove.SetPosition ( m_vPos, -1 );
			if ( m_actorMove.PathIsActive() ) m_actorMove.Stop();
		}
		break;

	default:
		CDebugSet::ToListView ( "[WARNING]GLAnySummon::MsgProcess Illigal Message(%d)", nmg->nType );
		break;
	};
}

