#include "stdafx.h"
#include "./GLChar.h"

#include "./GLGaeaServer.h"
#include "./GLSchoolFreePK.h"
#include "GLClubDeathMatch.h"

/*pvp tyranny, Juver, 2017/08/24 */
#include "GLPVPTyrannyField.h"  

/*school wars, Juver, 2018/01/19 */
#include "GLPVPSchoolWarsField.h"  

/*pvp capture the flag, Juver, 2018/01/31 */
#include "GLPVPCaptureTheFlagField.h"
#include "../Lib_ClientUI/Interface/GameTextControl.h"

#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//LearnSkill
HRESULT GLChar::MsgReqLearnSkillNonInven ( NET_MSG_GENERIC* nmg )
{
	if ( !IsValidBody() )	return E_FAIL;

	GLMSG::SNETPC_REQ_LEARNSKILL_NONINVEN *pNetMsg = (GLMSG::SNETPC_REQ_LEARNSKILL_NONINVEN *) nmg;

	SNATIVEID sSKILL_ID = pNetMsg->sSKILLID;

		if ( ISLEARNED_SKILL(sSKILL_ID) )
	
		{
			GLMSG::SNETPC_REQ_LEARNSKILL_NONINVEN_FB NetMsgFB;
			NetMsgFB.skill_id = sSKILL_ID;
			NetMsgFB.emCHECK = EMSKILL_LEARN_ALREADY;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
	
			return E_FAIL;
		}
	
		EMSKILL_LEARNCHECK emSKILL_LEARNCHECK = CHECKLEARNABLE_SKILL(sSKILL_ID);
		if ( emSKILL_LEARNCHECK!=EMSKILL_LEARN_OK )
		{
			GLMSG::SNETPC_REQ_LEARNSKILL_NONINVEN_FB	NetMsgFB;
			NetMsgFB.skill_id = sSKILL_ID;
			NetMsgFB.emCHECK = emSKILL_LEARNCHECK;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
			return E_FAIL;
		}
		LEARN_SKILL_NONSCROLL(sSKILL_ID);

		GLMSG::SNETPC_REQ_LEARNSKILL_NONINVEN_FB	NetMsgFB;
		NetMsgFB.skill_id = sSKILL_ID;
		NetMsgFB.emCHECK = EMSKILL_LEARN_OK;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
	
		GLMSG::SNETPC_UPDATE_MONEY NetMsgMoney;
		NetMsgMoney.lnMoney = m_lnMoney;
		/*dmk14 ingame web*/
		NetMsgMoney.dwPremium = m_dwPremiumPoints;
		NetMsgMoney.dwCombat = m_dwCombatPoints;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgMoney );
	
		GLMSG::SNETPC_UPDATE_SKP NetMsgSkp;
		NetMsgSkp.dwSkillPoint = m_dwSkillPoint;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgSkp );
	
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sSKILL_ID );
		if ( !pSkill ) return E_FAIL;
	
		if ( pSkill->m_sBASIC.emROLE == SKILL::EMROLE_PASSIVE )
		{
			GLMSG::SNETPC_UPDATE_PASSIVE_BRD NetMsgBrd;
			NetMsgBrd.dwGaeaID = m_dwGaeaID;
			NetMsgBrd.sSKILL_DATA = m_sSUM_PASSIVE;
			SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
		}
	
		return S_OK;
}
HRESULT GLChar::MsgReqLearnSkill ( NET_MSG_GENERIC* nmg )
{
	if ( !IsValidBody() )	return E_FAIL;

	GLMSG::SNETPC_REQ_LEARNSKILL *pNetMsg = (GLMSG::SNETPC_REQ_LEARNSKILL *) nmg;

	SINVENITEM* pInvenItem = m_cInventory.GetItem ( pNetMsg->wPosX, pNetMsg->wPosY );
	if ( !pInvenItem )	return E_FAIL;

	if ( CheckCoolTime( pInvenItem->sItemCustom.sNativeID ) )	return S_FALSE;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem || pItem->sBasicOp.emItemType!=ITEM_SKILL )	return E_FAIL;

	SNATIVEID sSKILL_ID = pItem->sSkillBookOp.sSkill_ID;

	if ( ISLEARNED_SKILL(sSKILL_ID) )
	{
		//	이미 습득한 스킬.
		GLMSG::SNETPC_REQ_LEARNSKILL_FB	NetMsgFB;
		NetMsgFB.skill_id = sSKILL_ID;
		NetMsgFB.emCHECK = EMSKILL_LEARN_ALREADY;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

		return E_FAIL;
	}

	EMSKILL_LEARNCHECK emSKILL_LEARNCHECK = CHECKLEARNABLE_SKILL(sSKILL_ID);
	if ( emSKILL_LEARNCHECK!=EMSKILL_LEARN_OK )
	{
		//	스킬 습득 요구 조건을 충족하지 못합니다.
		GLMSG::SNETPC_REQ_LEARNSKILL_FB	NetMsgFB;
		NetMsgFB.skill_id = sSKILL_ID;
		NetMsgFB.emCHECK = emSKILL_LEARNCHECK;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

		return E_FAIL;
	}

	//	Note : 스킬 습득함.
	//
	LEARN_SKILL(sSKILL_ID);

	//	Note : 스킬 습득으로 인한 퀘스트 시작 점검.
	//
	QuestStartFromGetSKILL ( sSKILL_ID );

	//	Note : 소모성 아이템 제거.
	//
	DoDrugInvenItem ( pNetMsg->wPosX, pNetMsg->wPosY, true );

	//	스킬 배움 성공.
	GLMSG::SNETPC_REQ_LEARNSKILL_FB	NetMsgFB;
	NetMsgFB.skill_id = sSKILL_ID;
	NetMsgFB.emCHECK = EMSKILL_LEARN_OK;
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

	//	스킬 배울때 소모된 포인트 업데이트.
	GLMSG::SNETPC_UPDATE_SKP NetMsgSkp;
	NetMsgSkp.dwSkillPoint = m_dwSkillPoint;
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgSkp );

	//	Note : passive skill 의 경우 케릭에 적용되는 속성값을 클라이언트들에 동기화.
	//
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sSKILL_ID );
	if ( !pSkill ) return E_FAIL;

	if ( pSkill->m_sBASIC.emROLE == SKILL::EMROLE_PASSIVE )
	{
		GLMSG::SNETPC_UPDATE_PASSIVE_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID = m_dwGaeaID;
		NetMsgBrd.sSKILL_DATA = m_sSUM_PASSIVE;
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
	}

	return S_OK;
}

HRESULT GLChar::MsgReqSkillUp ( NET_MSG_GENERIC* nmg )
{
	if ( !IsValidBody() )	return E_FAIL;

	GLMSG::SNETPC_REQ_SKILLUP *pNetMsg = (GLMSG::SNETPC_REQ_SKILLUP *) nmg;

	EMSKILL_LEARNCHECK emSKILL_LVLUPCHECK = EMSKILL_LEARN_UNKNOWN;
	SCHARSKILL* pCHARSKILL = GETLEARNED_SKILL(pNetMsg->skill_id);
	if ( !pCHARSKILL )
	{
		//	정상적으로 발생 할 수 없는 상황.
		return E_FAIL;
	}

	emSKILL_LVLUPCHECK = CHECKLEARNABLE_SKILL(pNetMsg->skill_id);
	if ( emSKILL_LVLUPCHECK!=EMSKILL_LEARN_OK )
	{
		//	랩업 조건이 부족합니다. FB 메시지.
		GLMSG::SNETPC_REQ_SKILLUP_FB	NetMsgFB;
		NetMsgFB.sSkill.sNativeID = pNetMsg->skill_id;
		NetMsgFB.emCHECK = emSKILL_LVLUPCHECK;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
		return E_FAIL;
	}

	//	Note : 스킬 LEVEL UP.
	//
	WORD wToLevel = pCHARSKILL->wLevel + 1;
	LVLUP_SKILL ( pNetMsg->skill_id, wToLevel );

	//	랩업. FB 메시지.
	GLMSG::SNETPC_REQ_SKILLUP_FB	NetMsgFB;
	
	NetMsgFB.sSkill.sNativeID = pNetMsg->skill_id;
	NetMsgFB.sSkill.wLevel = wToLevel;
	NetMsgFB.emCHECK = EMSKILL_LEARN_OK;

	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

	//	스킬 배울때 소모된 포인트 업데이트.
	GLMSG::SNETPC_UPDATE_SKP NetMsgSkp;
	NetMsgSkp.dwSkillPoint = m_dwSkillPoint;
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgSkp );

	//	Note : passive skill 의 경우 케릭에 적용되는 속성값을 클라이언트들에 동기화.
	//
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( pNetMsg->skill_id );
	if ( !pSkill ) return E_FAIL;

	if ( pSkill->m_sBASIC.emROLE == SKILL::EMROLE_PASSIVE )
	{
		GLMSG::SNETPC_UPDATE_PASSIVE_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID = m_dwGaeaID;
		NetMsgBrd.sSKILL_DATA = m_sSUM_PASSIVE;
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
	}

	return S_OK;
}

HRESULT GLChar::MsgReqSkill ( NET_MSG_GENERIC* nmg )
{
	static bool bFirst = true;
	//static int nCnt = 0;

	//CONSOLEMSG_WRITE( "MsgReqSkill Start" );

	if( GLGaeaServer::GetInstance().m_bEmptyMsg )						return S_OK;
	if ( !IsValidBody() )												return E_FAIL;
	if ( m_pLandMan && m_pLandMan->IsPeaceZone() )						return E_FAIL;

#if !defined(KR_PARAM) && !defined(KRT_PARAM)

	// 딜래이 체크
	if ( m_fSkillDelay < 0.3f )											return E_FAIL;  // 05.11.30  0.5f -> 0.3f

#endif

	bool bFreePKMap		= m_pLandMan->IsFreePK();
	bool bGuidBattleMap = m_pLandMan->m_bGuidBattleMap;
	bool bBRIGHTEVENT   = GLGaeaServer::GetInstance().IsBRIGHTEVENT();

	GLMSG::SNETPC_REQ_SKILL *pNetMsg = (GLMSG::SNETPC_REQ_SKILL *) nmg;
	SNATIVEID skill_id = pNetMsg->skill_id;

	// 발동 스킬일경우 현재 발동 스킬인지 검사
	if ( pNetMsg->bDefenseSkill )
	{
		if ( pNetMsg->skill_id != m_sDefenseSkill.m_dwSkillID ) return E_FAIL;
		if ( !m_bDefenseSkill ) return E_FAIL;
	}

	//	스킬 정보 가져옴.
	const PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( skill_id.wMainID, skill_id.wSubID );
	if ( !pSkill )	return E_FAIL;
	const SKILL::SEXT_DATA &sEXT_DATA = pSkill->m_sEXT_DATA;
	bool benermy = ( SIDE_ENEMY==pSkill->m_sBASIC.emIMPACT_SIDE );

	//anti aspeed hack
	//bring front animation skill check
	GLCONST_CHARCLASS &ConstCharClass = GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX];
	VECANIATTACK &vecAniAttack = ConstCharClass.m_ANIMATION[sEXT_DATA.emANIMTYPE][sEXT_DATA.emANISTYPE];
	if ( vecAniAttack.empty() )
	{
		// 캐릭터의 스킬에니메이션이 없습니다.
		DEBUGMSG_WRITE ( _T("Can't find skill animation. char[%d] animation[%d][%d]"),
			             m_CHARINDEX,
						 sEXT_DATA.emANIMTYPE,
						 sEXT_DATA.emANISTYPE );
		return E_FAIL;
	}

	const SANIATTACK &sAniAttack = vecAniAttack[0];
	WORD wStrikeNum = sAniAttack.m_wDivCount;
	if ( wStrikeNum==0 )
	{
		// 스킬 에니메이션에 타격 지점이 지정되지 않았습니다
		DEBUGMSG_WRITE ( _T("Can't find target position of skill animation. char[%d] animation[%d][%d]"),
			             m_CHARINDEX,
						 sEXT_DATA.emANIMTYPE,
						 sEXT_DATA.emANISTYPE );
		return E_FAIL;
	}

	if( pNetMsg->fAttVelo != GETATTVELO() || pNetMsg->fAttVeloItem != GETATT_ITEM() || pNetMsg->dwAnimationKey != sAniAttack.m_dwSTime ||
		pNetMsg->emMType != sEXT_DATA.emANIMTYPE || pNetMsg->emSType != sEXT_DATA.emANISTYPE )
		return E_FAIL;
	//end

	//	Note : 타겟 지정.
	//
	m_TargetID.vPos = pNetMsg->vTARPOS;
	m_TargetID.emCrow = pNetMsg->sTARGET_IDS[0].GETCROW();
	m_TargetID.dwID = pNetMsg->sTARGET_IDS[0].GETID();

	/*dmk14 anti shit*/
	SCHARSKILL sSkill;
    if ( pNetMsg->bDefenseSkill ) 
	{
		if ( pNetMsg->skill_id != m_sDefenseSkill.m_dwSkillID ) return E_FAIL;
		sSkill.sNativeID = skill_id;
		sSkill.wLevel = m_sDefenseSkill.m_wLevel;
	}
	else
	{
		SKILL_MAP_ITER learniter = m_ExpSkills.find ( skill_id.dwID );
		if ( learniter==m_ExpSkills.end() )						return E_FAIL;
		sSkill = (*learniter).second;
	}

	if( pNetMsg->fSum_TarRange != GETSUM_TARRANGE() || pNetMsg->wBodyRadius != GETBODYRADIUS () || 
		pNetMsg->wAttRange != GETATTACKRANGE() || pNetMsg->wSkillRangeApply != GETSKILLRANGE_APPLY(*pSkill,sSkill.wLevel) ||
		pNetMsg->wSkillRangeTar != GETSKILLRANGE_TAR( *pSkill ) )
			return E_FAIL; 	

	WORD wSkillRange = GETSKILLRANGE_TAR(*pSkill);	

	// 대련 종료후 무적타임이면 공격 무시
	if ( m_sCONFTING.IsPOWERFULTIME () ) return E_FAIL;
	
	//	Note : 타겟의 유효성 점검.
	//
	
	const SKILL::CDATA_LVL &sSKILL_DATA = pSkill->m_sAPPLY.sDATA_LVL[sSkill.wLevel];

	bool bFound = false;
	bool bFirstTarLR = false;
	bool bFirstTerrainHack = false;

	D3DXVECTOR3 vTarOrigin(0.f,0.f,0.f);
	float fTarOriginTOLinkPos = 0.0f;

	D3DXVECTOR3 vTarPos(0.f,0.f,0.f);
	D3DXVECTOR3 vTarOrgDir(0.f,0.f,0.f);

	GLSkillTargetArgv SkillTargetArgv(this, pNetMsg, pSkill, m_TargetID, sSkill.wLevel);
	switch ( pSkill->m_sBASIC.emIMPACT_TAR )
	{
	case TAR_SELF:
		if ( setTarget_Self(SkillTargetArgv) == false )	return E_FAIL;
		break;
	case TAR_SPEC:
		if ( setTarget_Spec(SkillTargetArgv) == false )	return E_FAIL;
		break;
	case TAR_SELF_TOSPEC:
		if ( setTarget_SelfToSpec(SkillTargetArgv) == false )	return E_FAIL;
		break;
	case TAR_ZONE:
		if ( setTarget_Zone(SkillTargetArgv) == false )	return E_FAIL;
		break;
	case TAR_SPECIFIC:
		if ( setTarget_Land(SkillTargetArgv) == false )	return E_FAIL;
		break;
	}

	EMSKILLCHECK emCHECK = GLCHARLOGIC::CHECHSKILL ( skill_id, 1, pNetMsg->bDefenseSkill );
	if ( emCHECK != EMSKILL_OK /*&& emCHECK != EMSKILL_NOTSP*/ )
	{
		//	Note : 스킬 구동 실패 FB 메시지.
		GLMSG::SNETPC_REQ_SKILL_FB NetMsgFB;
		NetMsgFB.emSKILL_FB = emCHECK;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
		return E_FAIL;
	}

	/*dmk14 teleport update*/
	bool bUseTeleport = false;
	/*if( pSkill->m_sEXT_DATA.sADDSKILL != NATIVEID_NULL() )
	{
		const PGLSKILL pSLink = GLSkillMan::GetInstance().GetData ( pSkill->m_sEXT_DATA.sADDSKILL );
		if( pSLink && pSLink->m_sAPPLY.IsSpec( EMSPECA_RANDOM_EXP_RATE ) )		bUseTeleport = true;
	}*/

	/*dmk14 visual bug fix*/
	if ( m_actorMove.PathIsActive() )	m_actorMove.Stop();

	if ( pSkill->m_sAPPLY.IsSpec( EMSPECA_RANDOM_EXP_RATE ) )		bUseTeleport = true;
	if ( bUseTeleport )										
	{
		WORD wAblteToTeleportDis = 0;
		float fTeleportDistance = 0.0f;
		STARGETID sTARID(pNetMsg->sTARGET_IDS[0].GETCROW(),pNetMsg->sTARGET_IDS[0].GETID());
		GLACTOR* pTARGET = GLGaeaServer::GetInstance().GetTarget ( m_pLandMan, sTARID );
		if ( pTARGET )	
		{
			wAblteToTeleportDis = pTARGET->GetBodyRadius() + GETBODYRADIUS() + GETSKILLRANGE_TAR(*pSkill) + 2;
			fTeleportDistance = D3DXVec3Length ( &D3DXVECTOR3( GetPosition() - pNetMsg->vTARPOS ) );
		}
		else
		{
			wAblteToTeleportDis = GETBODYRADIUS() + GETSKILLRANGE_TAR(*pSkill) + 2;
			fTeleportDistance = D3DXVec3Length ( &D3DXVECTOR3( GetPosition() - pNetMsg->vTARPOS ) );
		}

		if( fTeleportDistance <= wAblteToTeleportDis )	ReceiveTeleport( pNetMsg->vTARPOS );
	}
	/*dmk14 teleport code end*/

	WORD wAdditionalRange = 4;
	float fDetectRange = 0.0f;
	float fDistance = 0.0f;

	std::vector<STARID> vectargets;
	vectargets.reserve( EMTARGET_NET );
	for ( WORD i=0; i<pNetMsg->wTARGET_NUM; ++i )
	{
		fTarOriginTOLinkPos = 0.0f;
		//	Note : 유효성 검사.
		//
		if ( i >= EMTARGET_NET )	continue;

		STARGETID sTARID(pNetMsg->sTARGET_IDS[i].GETCROW(),pNetMsg->sTARGET_IDS[i].GETID());
		GLACTOR* pTARGET = GLGaeaServer::GetInstance().GetTarget ( m_pLandMan, sTARID );
		if ( !pTARGET )	continue;


		for( int x=i+1; x<pNetMsg->wTARGET_NUM;++x )
		{
			if( pNetMsg->sTARGET_IDS[i].wID == pNetMsg->sTARGET_IDS[x].wID )	
			{
				bFound = true;
				break;
			}
		}
		if( bFound )	return E_FAIL;
		
		
		/* anti ba code war
		if ( skill_id == SNATIVEID(14,14) && m_pLandMan->m_bPVPTyrannyMap && pTARGET->GetSchool() == m_wSchool )
		{
			if( m_dwPartyID == PARTY_NULL && pTARGET->GetCharID() != m_dwCharID  )			continue;
			if( m_dwPartyID != PARTY_NULL && m_dwPartyID != pTARGET->GetPartyID() )			continue;
		}
		else if ( skill_id == SNATIVEID(14,14) && m_pLandMan->m_bPVPTyrannyMap )
		{
			if( m_bSafeZone || pTARGET->IsSafeZone() )	 continue;
		}
		*/

		/*dmk14 anti shit start*/
		if ( bFirstTarLR /*|| bFirstTerrainHack*/ )		continue;

		if( i == 0 ) 
		{
			vTarOrigin = pTARGET->GetPosition();
			vTarPos = pTARGET->GetPosition();
			vTarOrgDir = vTarPos - GetPosition();
		}

		D3DXVECTOR3 vTarGetPos = pTARGET->GetPosition();
		const float fDist = D3DXVec3Length ( &D3DXVECTOR3(m_vPos-vTarGetPos) );
		const float fApplyAngle = D3DXToRadian(sSKILL_DATA.wAPPLYANGLE) / 2.0f;
		
		fTarOriginTOLinkPos = D3DXVec3Length ( &D3DXVECTOR3(vTarOrigin-vTarGetPos) );

		WORD wAttackRange = 0;
		WORD wAttackAbleDis = 0;

		if ( ISLONGRANGE_ARMS() )	 wAttackRange = static_cast<WORD> ( GETSUM_TARRANGE());

		bool bValidAngle = true;
		bool bTarSpecRealmZone = false;

		D3DXVECTOR3 vTarDir = pTARGET->GetPosition() - GetPosition();
		float fDir = DXGetThetaYFromDirection ( vTarDir, vTarOrgDir );

		DWORD dwTarSeq = 0;
		switch ( pSkill->m_sBASIC.emIMPACT_TAR )	//TARGET
		{
			case TAR_SELF:
				{
					switch ( pSkill->m_sBASIC.emIMPACT_REALM )
					{
						case REALM_ZONE:
							{
								switch ( pSkill->m_sBASIC.emIMPACT_SIDE )
								{
									case SIDE_OUR:
									case SIDE_ENEMY:
									case SIDE_ANYBODY:
										{
											wAttackRange += GETBODYRADIUS() + sSKILL_DATA.wAPPLYRANGE;
											wAttackAbleDis = wAttackRange+1;
										}
										break;
								}
							}
							break;
					}
				}
				break;
			case TAR_SPEC:	//target
				{
					switch ( pSkill->m_sBASIC.emIMPACT_REALM )	//AREA
					{
						case REALM_KEEP_ZONE:
							{
								
							}
							break;
						case REALM_SELF:
							{
								switch ( pSkill->m_sBASIC.emIMPACT_SIDE )	//SIDE
								{
									case SIDE_OUR:
									case SIDE_ENEMY:
									case SIDE_ANYBODY:
										{
											wAttackRange += pTARGET->GetBodyRadius() + GETBODYRADIUS() + wSkillRange;
											wAttackAbleDis = wAttackRange+1;
										}
										break;
								}
							}
							break;
						case REALM_ZONE:
							{
								switch ( pSkill->m_sBASIC.emIMPACT_SIDE )
								{
									case SIDE_OUR:
									case SIDE_ENEMY:
									case SIDE_ANYBODY:
										{
											if( i == 0 )	wAttackRange += pTARGET->GetBodyRadius() + wSkillRange;
											else			wAttackRange = GETBODYRADIUS() + sSKILL_DATA.wAPPLYRANGE;

											wAttackAbleDis = wAttackRange+1;

											bTarSpecRealmZone = true;
										}
										break;
								}
							}
							break;
						case REALM_FANWIZE:
							{
								switch ( pSkill->m_sBASIC.emIMPACT_SIDE )
								{
									case SIDE_ENEMY:
										{
											if ( fabs(fDir) > fApplyAngle )
											{
												if( sSKILL_DATA.wAPPLYANGLE!= 0 && sSKILL_DATA.wAPPLYANGLE!= 360)
												bValidAngle = false;
											}

											const WORD wSkillAppRange = ( GETSKILLRANGE_APPLY(*pSkill,sSkill.wLevel) );
											wAttackRange +=  pTARGET->GetBodyRadius() + GETBODYRADIUS() + wSkillAppRange;
											wAttackAbleDis = wAttackRange+1;
										}
										break;
								}
							}
							break;
					}
				}
				break;
			case TAR_SELF_TOSPEC:	//from self to target
				{
					switch ( pSkill->m_sBASIC.emIMPACT_SIDE )
					{
						case SIDE_ENEMY:
							{
								const WORD wSkillAppRange = ( GETSKILLRANGE_APPLY(*pSkill,sSkill.wLevel) );
								wAttackRange +=  pTARGET->GetBodyRadius() + GETBODYRADIUS() + wSkillAppRange;
								wAttackAbleDis = wAttackRange+1;
							}
							break;
					}
				}
				break;
			case TAR_ZONE:	//specific loc
				{
					switch ( pSkill->m_sBASIC.emIMPACT_SIDE )
					{
						case SIDE_OUR:
						case SIDE_ANYBODY:
							{
								wAttackRange += GETBODYRADIUS() + sSKILL_DATA.wAPPLYRANGE;
								wAttackAbleDis = wAttackRange+1;
							}
							break;
						case SIDE_ENEMY:
							{
								wAttackRange += sSKILL_DATA.wAPPLYRANGE;
								wAttackAbleDis = wAttackRange+1;
							}
							break;
					}
				}
				break;
				//teleport cheat fix
			case TAR_SPECIFIC:
				{
					switch ( pSkill->m_sBASIC.emIMPACT_SIDE )
					{
						case SIDE_OUR:
						case SIDE_ANYBODY:
							{
								wAttackRange += GETBODYRADIUS() + GETSKILLRANGE_TAR(*pSkill) + 2;
								wAttackAbleDis = wAttackRange+2;
							}
							break;
						case SIDE_ENEMY:
							{
								wAttackRange +=  GETSKILLRANGE_TAR(*pSkill) + 2;
								wAttackAbleDis = wAttackRange+2;
							}
							break;
					}
				}
				break;
		}

		//terrain hack fix
		if( i == 0 )
		{
			D3DXVECTOR3 vREACTOR_POS = vTarPos;
			D3DXVECTOR3 vDir = ( vREACTOR_POS - GetPosition() );
			
			D3DXVECTOR3 vACTOR_POS = GetPosition();
			D3DXVECTOR3 vDirZ = ( vACTOR_POS - vTarPos );
			vDir.y = 0.0f;
			D3DXVec3Normalize (  &vDir, &vDir );

			// Check terrain collision
			NavigationMesh* pNavi = m_pLandMan->GetNavi();
			if ( pNavi )
			{
				D3DXVECTOR3 vPosA(0,0,0);
				D3DXVECTOR3 vPosB(0,0,0);

				vPosA = vREACTOR_POS;
				vPosB = GetPosition();
				
				BOOL bCoA(FALSE);
				DWORD dwCoA(0);
				D3DXVECTOR3 vCoA(0,0,0);

				pNavi->IsCollision ( D3DXVECTOR3(vPosA.x,vPosA.y+5.0f,vPosA.z), D3DXVECTOR3(vPosA.x,vPosA.y-5.0f,vPosA.z), vCoA, dwCoA, bCoA );
				if ( bCoA )
				{
					BOOL bCoB(FALSE);
					DWORD dwCoB(0);
					D3DXVECTOR3 vCoB(0,0,0);

					pNavi->IsCollision ( D3DXVECTOR3(vPosB.x,vPosB.y+5.0f,vPosB.z), D3DXVECTOR3(vPosB.x,vPosB.y-5.0f,vPosB.z), vCoB, dwCoB, bCoB );
					if ( !bCoB /*&& m_pLandMan->m_bPVPTyrannyMap*/ )	bFirstTerrainHack = true;
					if ( bCoB )
					{
						if ( !pNavi->LineOfSightTest ( dwCoA, vCoA, dwCoB, vCoB ) )		
						{
							/*if( m_pLandMan->m_bPVPTyrannyMap )*/	bFirstTerrainHack = true;
						}
					}
				}
			}
		}
		//end terrain fix

		if( bTarSpecRealmZone )
		{
			if( i == 0 && ( fDist > wAttackAbleDis ) )
			{
				bFirstTarLR = true;
				continue;
			}
			else if ( i > 0 && fTarOriginTOLinkPos > wAttackAbleDis )	continue;				
		}
		else if ( fDist > wAttackAbleDis )	continue;

		if ( !bValidAngle || bFirstTarLR /*|| bFirstTerrainHack*/ )			continue;

		fDistance = D3DXVec3Length ( &D3DXVECTOR3(GetPosition()-pTARGET->GetPosition()) );
		fDetectRange = GETBODYRADIUS() + pTARGET->GetBodyRadius() + GETSKILLRANGE_TAR(*pSkill) + GETSKILLRANGE_APPLY(*pSkill,sSkill.wLevel) + wAdditionalRange;
		if( fDistance > fDetectRange )			 continue;
		/*dmk14 anti shit end*/


		bool bREACTIONALBE  = IsReActionable( pTARGET, benermy );			
		if ( !bREACTIONALBE )												continue;

		bool bSCHOOL_FREEPK = GLSchoolFreePK::GetInstance().IsON();
		if ( GetSchool() == pTARGET->GetSchool() )	bSCHOOL_FREEPK = false;

		//	Note : 학원간 자유 pk일 때는 다른 학원생을 부활 시킬수 없음.
		if ( pSkill->m_sAPPLY.IsSpec( EMSPECA_REBIRTH ) && bSCHOOL_FREEPK ) continue;
		
		if ( pTARGET->GetCrow() == CROW_PC )
		{
			PGLCHAR pCHAR = GLGaeaServer::GetInstance().GetChar ( sTARID.dwID );
			if ( pCHAR )
			{
				// 부활일 경우에 상대방이 부활을 가능할 경우에만 실행 된다.
				if( pSkill->m_sBASIC.sNATIVEID.wMainID == 14 && pSkill->m_sBASIC.sNATIVEID.wSubID == 15 && pCHAR->m_bNon_Rebirth == TRUE )
				{
					//	Note : 스킬 구동 실패 FB 메시지.
					{
						GLMSG::SNETPC_REQ_SKILL_FB NetMsgFB;
						strcpy( NetMsgFB.szName, pCHAR->m_szName );
						NetMsgFB.emSKILL_FB = EMSKILL_NOTREBIRTH;
						GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
					}
					{
						GLMSG::SNET_MSG_REQ_SKILL_REVIVEL_FAILED NetMsgFB;
						strcpy( NetMsgFB.szName, m_szName );
						GLGaeaServer::GetInstance().SENDTOCLIENT ( pCHAR->m_dwClientID, &NetMsgFB );
					}
					return E_FAIL;
				}
				// 대련 종료후 무적타임이면 공격 무시
				if ( pCHAR->m_sCONFTING.IsPOWERFULTIME () )					continue;

				bool bClubBattle = false;
				bool bClubDeathMatch = false; 
				bool bPVPTyranny = false;			/*pvp tyranny, Juver, 2017/08/24 */
				bool bPVPSchoolWars = false;		/*school wars, Juver, 2018/01/19 */
				bool bPVPCaptureTheFlag = false;	/*pvp capture the flag, Juver, 2018/01/31 */

				if ( m_pLandMan->IsClubBattleZone() )
				{
					GLClubMan &cClubMan = m_pGLGaeaServer->GetClubMan();
					GLCLUB *pMyClub = cClubMan.GetClub ( m_dwGuild );
					GLCLUB *pTarClub = cClubMan.GetClub ( pCHAR->m_dwGuild );

					if ( pMyClub && pTarClub )
					{
						bool bClub = pMyClub->IsBattle( pCHAR->m_dwGuild );
						bool bAlliance = pMyClub->IsBattleAlliance ( pTarClub->m_dwAlliance );

						bClubBattle = ( bClub || bAlliance );
					}
				}

				if ( m_pLandMan->m_bClubDeathMatchMap )
				{
					GLClubDeathMatch* pCDM = GLClubDeathMatchFieldMan::GetInstance().Find( m_pLandMan->m_dwClubMapID );
					if ( pCDM && pCDM->IsBattle() ) bClubDeathMatch = true;	
				}

				/*pvp tyranny, Juver, 2017/08/24 */
				if ( m_pLandMan->m_bPVPTyrannyMap && GLPVPTyrannyField::GetInstance().IsBattle() )
					bPVPTyranny = true;	

				/*school wars, Juver, 2018/01/19 */
				if ( m_pLandMan->m_bPVPSchoolWarsMap && GLPVPSchoolWarsField::GetInstance().IsBattle() )
					bPVPSchoolWars = true;	

				/*pvp capture the flag, Juver, 2018/01/31 */
				if ( m_pLandMan->m_bPVPCaptureTheFlagMap && GLPVPCaptureTheFlagField::GetInstance().IsBattle() )
					bPVPCaptureTheFlag = true;	

				/*pvp tyranny, Juver, 2017/08/24 */
				/*school wars, Juver, 2018/01/19 */
				/*pvp capture the flag, Juver, 2018/01/31 */
				if ( !(bGuidBattleMap|| bClubDeathMatch ||bSCHOOL_FREEPK||bFreePKMap||bBRIGHTEVENT||IsConflictTarget(pTARGET) || bClubBattle ||
					bPVPTyranny||bPVPSchoolWars || bPVPCaptureTheFlag  ) )
				{
					//	Note : 상대방이 범죄자가 아니고 정당방위 시간이 지정되지 않았을때는 
					//		자신이 적대행위를 시 작함.
					if ( !IS_PLAYHOSTILE(pCHAR->m_dwCharID) && !pCHAR->ISOFFENDER() && benermy )
					{
						int nBRIGHT(GLCONST_CHAR::nPK_TRY_BRIGHT_POINT), nLIFE(GLCONST_CHAR::nPK_TRY_LIVING_POINT);

						//	Note : 속성수치 변화.
						m_nBright += nBRIGHT;

						GLMSG::SNETPC_UPDATE_BRIGHT NetMsg;
						NetMsg.nBright = m_nBright;
						GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg);

						GLMSG::SNETPC_UPDATE_BRIGHT_BRD NetMsgBrd;
						NetMsgBrd.dwGaeaID = m_dwGaeaID;
						NetMsgBrd.nBright = m_nBright;
						SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

						//	Note : 생활점수 변화.
						m_nLiving += nLIFE;

						GLMSG::SNETPC_UPDATE_LP NetMsgLp;
						NetMsgLp.nLP = m_nLiving;
						GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgLp);

						//	Note : 적대행위 로그 기록.
						GLITEMLMT::GetInstance().ReqAction
						(
							m_dwCharID,					//	당사자.
							EMLOGACT_HOSTILE,			//	행위.
							ID_CHAR, pCHAR->m_dwCharID,	//	상대방.
							0,							//	exp
							nBRIGHT,					//	bright
							nLIFE,						//	life
							0							//	money
						);
					}
				}

				if ( benermy && !IsConflictTarget(pTARGET) )
				{
					//	Note : 적대 행위자에게 피해자 등록. ( pk )
					AddPlayHostile ( pCHAR->m_dwCharID, TRUE, bClubBattle );

					//	Note : 적대 피해자에게 적대자 등록. ( PK )
					pCHAR->AddPlayHostile ( m_dwCharID, FALSE, bClubBattle );

					STARGETID sTargetID = STARGETID ( CROW_PC, m_dwGaeaID, m_vPos );
					
					
					// 공격자와 공격대상자 모두 팻을 공격모드로 변경
					PGLPETFIELD pEnemyPet = GLGaeaServer::GetInstance().GetPET ( pCHAR->m_dwPetGUID );
					if ( pEnemyPet && pEnemyPet->IsValid () && !pEnemyPet->IsSTATE ( EM_PETACT_ATTACK ) )
					{
						D3DXVECTOR3 vOwnerPos, vDist;
						float fDist;
						vOwnerPos = pCHAR->GetPosition ();
						vDist = pEnemyPet->m_vPos - vOwnerPos;
						fDist = D3DXVec3Length(&vDist);

						// 일정거리 안에 있으면 
						if ( fDist <= GLCONST_PET::fWalkArea )
						{
							pEnemyPet->ReSetAllSTATE ();
							pEnemyPet->SetSTATE ( EM_PETACT_ATTACK );

							GLMSG::SNETPET_ATTACK NetMsg;
							NetMsg.sTarID = sTargetID;
							GLGaeaServer::GetInstance().SENDTOCLIENT ( pCHAR->m_dwClientID, &NetMsg );
							
							GLMSG::SNETPET_ATTACK_BRD NetMsgBRD;
							NetMsgBRD.dwGUID = pEnemyPet->m_dwGUID;
							NetMsgBRD.sTarID = sTargetID;
							pCHAR->SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBRD );
						}

					}

					// 맞는자만 Summon Attack 처리
					/*skill summon, Juver, 2017/10/09 */
					for ( int ii=0; ii<SKILL_SUMMON_MAX_CLIENT_NUM; ++ii )
					{
						PGLSUMMONFIELD pEnemySummon = GLGaeaServer::GetInstance().GetSummon ( pCHAR->m_dwSummonGUID_FLD[ii] );
						if ( pEnemySummon && pEnemySummon->IsValid () && !pEnemySummon->IsSTATE ( EM_SUMMONACT_ATTACK ) && 
							pEnemySummon->GetAttackTarget( sTargetID ) && pEnemySummon->IsTargetReady() )
						{
							/*D3DXVECTOR3 vOwnerPos, vDist;
							float fDist;
							vOwnerPos = pCHAR->GetPosition ();
							vDist = pEnemySummon->m_vPos - vOwnerPos;
							fDist = D3DXVec3Length(&vDist);

							if ( fDist <= pEnemySummon->m_fWalkArea )*/
							{
								pEnemySummon->SetAttackTarget( sTargetID );
							}
						}
					}
					

					PGLPETFIELD pMyPet = GLGaeaServer::GetInstance().GetPET ( m_dwPetGUID );
					if ( pMyPet && pMyPet->IsValid () && !pMyPet->IsSTATE ( EM_PETACT_ATTACK ) )
					{
						D3DXVECTOR3 vDist;
						float fDist;
						vDist = pMyPet->m_vPos - m_vPos;
						fDist = D3DXVec3Length(&vDist);

						// 일정거리 안에 있으면 
						if ( fDist <= GLCONST_PET::fWalkArea )
						{
							pMyPet->ReSetAllSTATE ();
							pMyPet->SetSTATE ( EM_PETACT_ATTACK );

							GLMSG::SNETPET_ATTACK NetMsg;
							NetMsg.sTarID = m_TargetID;
							GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );
							
							GLMSG::SNETPET_ATTACK_BRD NetMsgBRD;
							NetMsgBRD.dwGUID = pMyPet->m_dwGUID;
							NetMsgBRD.sTarID = m_TargetID;
							SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBRD );
						}
					}
				}
			}
		}

		SKILL::CDATA_LVL &sSKILL_DATA = pSkill->m_sAPPLY.sDATA_LVL[0];

		BOOL bexist = FALSE;
		for( int n=0; n<(int)vectargets.size(); ++n )
		{
			if( pSkill->m_sAPPLY.emBASIC_TYPE == SKILL::EMFOR_HP && sSKILL_DATA.fBASIC_VAR < 0.0f ){
				if(  pNetMsg->sTARGET_IDS[i].wCrow == static_cast<WORD>(CROW_PC) && pNetMsg->sTARGET_IDS[i].wID == m_dwGaeaID )
				{
					//CDebugSet::ToFileWithTime( "_skill_check.txt", "[%d]%s [%d~%d]%s self-targeting attempt.", m_dwCharID, m_szName, 
					//	pSkill->m_sBASIC.sNATIVEID.wMainID, pSkill->m_sBASIC.sNATIVEID.wSubID, pSkill->GetName() );
					continue;
				}
			}

			if ( pNetMsg->sTARGET_IDS[i].wCrow == vectargets[n].wCrow &&  
				pNetMsg->sTARGET_IDS[i].wID == vectargets[n].wID )
			{
				bexist = TRUE;
				//CDebugSet::ToFileWithTime( "_skill_check.txt", "[%d]%s [%d~%d]%s multi-hit attempt", m_dwCharID, m_szName, 
				//	pSkill->m_sBASIC.sNATIVEID.wMainID, pSkill->m_sBASIC.sNATIVEID.wSubID, pSkill->GetName() );
				break;
			}
		}

		if ( !bexist )	vectargets.push_back( pNetMsg->sTARGET_IDS[i] );
	}


	/*skill validity check, Juver, 2017/11/26 */
	m_wTARNUM = (WORD)vectargets.size();
	for( int n=0; n<EMTARGET_NET; ++n )
	{
		m_sTARIDS[n].wCrow = 0;
		m_sTARIDS[n].wID = USHRT_MAX;
	}

	for( int n=0; n<(int)vectargets.size(); ++n )
	{
		if ( n >= EMTARGET_NET )	continue;
		m_sTARIDS[n] = vectargets[n];
	}


	if ( m_wTARNUM == 0 )
	{
		//	Note : 자신에게.
		GLMSG::SNETPC_ATTACK_DAMAGE NetMsg;
		NetMsg.emTarCrow		= m_TargetID.emCrow;
		NetMsg.dwTarID			= m_TargetID.dwID;
		NetMsg.nDamage			= 0;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsg );

		return E_FAIL;
	}

	//	Note : 스킬 구동 조건 검사.
	//
	/* disable bring front
	GLCONST_CHARCLASS &ConstCharClass = GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX];
	VECANIATTACK &vecAniAttack = ConstCharClass.m_ANIMATION[sEXT_DATA.emANIMTYPE][sEXT_DATA.emANISTYPE];
	if ( vecAniAttack.empty() )
	{
		// 캐릭터의 스킬에니메이션이 없습니다.
		DEBUGMSG_WRITE ( _T("Can't find skill animation. char[%d] animation[%d][%d]"),
			             m_CHARINDEX,
						 sEXT_DATA.emANIMTYPE,
						 sEXT_DATA.emANISTYPE );
		return E_FAIL;
	}

	const SANIATTACK &sAniAttack = vecAniAttack[0];
	WORD wStrikeNum = sAniAttack.m_wDivCount;
	if ( wStrikeNum==0 )
	{
		// 스킬 에니메이션에 타격 지점이 지정되지 않았습니다
		DEBUGMSG_WRITE ( _T("Can't find target position of skill animation. char[%d] animation[%d][%d]"),
			             m_CHARINDEX,
						 sEXT_DATA.emANIMTYPE,
						 sEXT_DATA.emANISTYPE );
		return E_FAIL;
	}
	*/

	//	Note : SKILL 구동.
	//
	SETACTIVESKILL ( skill_id );
	SetDefenseSkill( pNetMsg->bDefenseSkill );
	if ( pNetMsg->bDefenseSkill ) m_bDefenseSkill = false;

	m_SKILLMTYPE = sEXT_DATA.emANIMTYPE;
	m_SKILLSTYPE = sEXT_DATA.emANISTYPE;

	BOOL bLowSP =  (emCHECK==EMSKILL_NOTSP) ? TRUE : FALSE;
	PreStrikeProc ( TRUE, bLowSP );

	

	if ( IsACTION ( GLAT_IDLE ) || !(pSkill->m_sBASIC.dwFlags & FLAG_SB_RUNNING_CAST ) ) TurnAction ( GLAT_SKILL );
	else
	{
		SkillProc ( m_idACTIVESKILL, false );
	}
	//TurnAction ( GLAT_SKILL );

	// 스킬이 정상적으로 발동되면 딜래이를 초기화
	m_fSkillDelay = 0.0f;
	
	//	Note : 스킬 사용시 소모값들 소모시킴.
	//
	GLCHARLOGIC::ACCOUNTSKILL ( skill_id, 1, true );


	//	Note : SKILL 구동 성공시 - 자신의 주변 Char에게 자신의 Msg를 전달.
	//
	WORD wLevel;
	if ( pNetMsg->bDefenseSkill )	wLevel = m_sDefenseSkill.m_wLevel;
	else wLevel = GETLEARNED_SKILL(pNetMsg->skill_id)->wLevel;

	GLMSG::SNETPC_REQ_SKILL_BRD NetMsgBRD;
	NetMsgBRD.emCrow = GETCROW();
	NetMsgBRD.dwID = m_dwGaeaID; //- 시전자 정보. 
	NetMsgBRD.skill_id = pNetMsg->skill_id;
	NetMsgBRD.wLEVEL = wLevel; //- 시전 스킬 종류.
	NetMsgBRD.vTARPOS = pNetMsg->vTARPOS;
	for ( WORD i=0; i<m_wTARNUM; ++i )		NetMsgBRD.ADDTARGET ( m_sTARIDS[i] );

	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBRD );



	//	Note : 스킬 구동 성공 FB 메시지.
	GLMSG::SNETPC_REQ_SKILL_FB NetMsgFB;
	NetMsgFB.emSKILL_FB = EMSKILL_OK;
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

	//CONSOLEMSG_WRITE( "MsgReqSkill End [%u]", skill_id.dwID );

	// 몹을 공격하면 팻이 공격모션을 취함
	if ( m_TargetID.emCrow==CROW_MOB )
	{
		PGLPETFIELD pMyPet = GLGaeaServer::GetInstance().GetPET ( m_dwPetGUID );
		if ( pMyPet && pMyPet->IsValid () && !pMyPet->IsSTATE ( EM_PETACT_ATTACK ) )
		{
			D3DXVECTOR3 vDist;
			float fDist;
			vDist = pMyPet->m_vPos - m_vPos;
			fDist = D3DXVec3Length(&vDist);

			// 일정거리 안에 있으면 
			if ( fDist <= GLCONST_PET::fWalkArea )
			{
				GLMSG::SNETPET_ATTACK NetMsg;
				NetMsg.sTarID = m_TargetID;
				GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );
				
				GLMSG::SNETPET_ATTACK_BRD NetMsgBRD;
				NetMsgBRD.dwGUID = m_dwPetGUID;
				NetMsgBRD.sTarID = m_TargetID;
				SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBRD );
			}
		}
	}

	/*skill summon, Juver, 2017/10/11 */
	ReTargetSummon( STARID(m_TargetID.emCrow,m_TargetID.dwID) );

	return S_OK;
}

HRESULT GLChar::MsgReqSkillCancel ( NET_MSG_GENERIC* nmg )
{
	//CONSOLEMSG_WRITE( "MsgReqSkillCancel" );



	//	Note : 액션 취소.
	if ( IsACTION(GLAT_SKILL) )		TurnAction ( GLAT_IDLE );

	//	Note : 자신의 주변 Char에게 자신의 Msg를 전달.
	//
	GLMSG::SNETPC_SKILL_CANCEL_BRD NetMsgBRD;
	NetMsgBRD.dwGaeaID = m_dwGaeaID;
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBRD );

	return S_OK;
}

HRESULT GLChar::MsgReqSkillQSet ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_SKILLQUICK_SET *pNetMsg = (GLMSG::SNETPC_REQ_SKILLQUICK_SET *) nmg;

	if ( EMSKILLQUICK_SIZE <= pNetMsg->wSLOT )	return E_FAIL;

	//	Note : 배운 스킬이 아닐 경우 취소됨.
	if ( !ISLEARNED_SKILL(pNetMsg->skill_id) )	return E_FAIL;

	//	Note : 슬롯에 넣어줌.
	m_sSKILLQUICK[pNetMsg->wSLOT] = pNetMsg->skill_id;

	//	Note : 클라이언트에 통보.
	GLMSG::SNETPC_REQ_SKILLQUICK_FB NetMsgFB;
	NetMsgFB.wSLOT = pNetMsg->wSLOT;
	NetMsgFB.skill_id = m_sSKILLQUICK[pNetMsg->wSLOT];
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

	return S_OK;
}

HRESULT GLChar::MsgReqSkillQReSet ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_SKILLQUICK_RESET *pNetMsg = (GLMSG::SNETPC_REQ_SKILLQUICK_RESET *) nmg;

	if ( EMSKILLQUICK_SIZE <= pNetMsg->wSLOT )	return E_FAIL;

	//	Note : 슬롯에 넣어줌.
	m_sSKILLQUICK[pNetMsg->wSLOT] = NATIVEID_NULL();

	//	Note : 클라이언트에 통보.
	GLMSG::SNETPC_REQ_SKILLQUICK_FB NetMsgFB;
	NetMsgFB.wSLOT = pNetMsg->wSLOT;
	NetMsgFB.skill_id = m_sSKILLQUICK[pNetMsg->wSLOT];
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

	return S_OK;
}

HRESULT GLChar::MsgReqSkillQSetActive ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_SKILLQUICK_ACTIVE *pNetMsg = (GLMSG::SNETPC_REQ_SKILLQUICK_ACTIVE *) nmg;

	if ( EMSKILLQUICK_SIZE <= pNetMsg->wSLOT )	return E_FAIL;

	m_wSKILLQUICK_ACT = pNetMsg->wSLOT;

	/*dmk14 Auto-switch weap Extreme*/
	if( m_emClass&GLCC_EXTREME_M || m_emClass&GLCC_EXTREME_W )
	{		
		bool bSwitch = false;
		if( !IsACTION(GLAT_SKILL) )
		{
			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_sSKILLQUICK[m_wSKILLQUICK_ACT] );
			if ( pSkill )
			{
				EMSLOT emRHand = GetCurRHand();
				SITEM* pRHandItem = GET_SLOT_ITEMDATA(emRHand);
				
				const bool	bHiddenWeapon	= pSkill->m_sLEARN.bHiddenWeapon;
				if( pRHandItem )
				{
					if( !CHECHSKILL_ITEM( pSkill->m_sBASIC.emUSE_RITEM, pRHandItem->sSuitOp.emAttack,bHiddenWeapon ) )
					{
						switch( emRHand )
						{
						case SLOT_RHAND:	
							{
								SITEM* pSubItem = GET_SLOT_ITEMDATA(SLOT_RHAND_S);
								if( pSubItem && CHECHSKILL_ITEM( pSkill->m_sBASIC.emUSE_RITEM, pSubItem->sSuitOp.emAttack,bHiddenWeapon ) )	
								{
									bSwitch = true;
								}
							}break;
						case SLOT_RHAND_S:	
							{
								SITEM* pSubItem = GET_SLOT_ITEMDATA(SLOT_RHAND);
								if( pSubItem && CHECHSKILL_ITEM( pSkill->m_sBASIC.emUSE_RITEM, pSubItem->sSuitOp.emAttack,bHiddenWeapon ) )	
								{
									bSwitch = true;
								}
							}break;
						}
					}
				}
			}
		}

		if( bSwitch)
		{
			if( IsUseArmSub() ) SetUseArmSub( FALSE ); // 주무기
			else				SetUseArmSub( TRUE );  // 보조무기

			INIT_DATA ( FALSE, FALSE, m_sCONFTING.sOption.fHP_RATE );

			GLMSG::SNETPC_PUTON_CHANGE NetMsg_PutOn;
			NetMsg_PutOn.bUseArmSub = IsUseArmSub();
			NetMsg_PutOn.fCONFT_HP_RATE = m_sCONFTING.sOption.fHP_RATE;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_PutOn);

			GLMSG::SNETPC_PUTON_CHANGE_BRD NetMsg_PutOn_Brd;
			NetMsg_PutOn_Brd.dwGaeaID = m_dwGaeaID;
			NetMsg_PutOn_Brd.bUseArmSub = IsUseArmSub();
			SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&NetMsg_PutOn_Brd) );

			GLMSG::SNETPC_PUTON_CHANGE_AG NetMsg_PutOn_AG;
			NetMsg_PutOn_AG.bUseArmSub = IsUseArmSub();
			GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsg_PutOn_AG );

			DisableSkillFact();
		}	
	}



	return S_OK;
}