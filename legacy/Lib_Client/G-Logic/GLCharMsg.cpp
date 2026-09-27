#include "stdafx.h"
#include "./GLChar.h"
#include "./GLGaeaServer.h"
#include "./GLSchoolFreePK.h"

#include "../NpcTalk/NpcTalk.h"
#include "GLClubDeathMatch.h"

/*pvp tyranny, Juver, 2017/08/24 */
#include "GLPVPTyrannyField.h" 

/*school wars, Juver, 2018/01/19 */
#include "GLPVPSchoolWarsField.h" 

/*pvp capture the flag, Juver, 2018/01/31 */
#include "GLPVPCaptureTheFlagField.h"

#include "RANPARAM.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define dwMoneyReset 1000000

void GLChar::MsgSendPeriod ()
{
	m_fPERIOD_TIMER = 0.0f;

	//	Note : Period Sync
	//
	GLMSG::SNET_PERIOD NetMsgPeriod;
	NetMsgPeriod.sPRERIODTIME = GLPeriod::GetInstance().GetPeriod();

	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, (LPVOID) &NetMsgPeriod );
}

void GLChar::MsgSendUpdateState ( bool bparty, bool bconfront, bool bview )
{
	//	[�ڽſ���] ȸ������ ����.
	GLMSG::SNETPC_UPDATE_STATE NetMsg;
	NetMsg.sHP = GLCHARLOGIC::m_sHP;
	NetMsg.sMP = GLCHARLOGIC::m_sMP;
	NetMsg.sSP = GLCHARLOGIC::m_sSP;
	NetMsg.bSafeTime = IsSafeTime();
	NetMsg.sCP = GLCHARLOGIC::m_sCombatPoint; /*combatpoint logic, Juver, 2017/05/28 */

	NetMsg.dwCharGaeaID = m_dwGaeaID;
	NetMsg.dwCharID	= m_dwCharID;
	StringCchCopy( NetMsg.szCharName, CHAR_SZNAME+1, m_szName );

	//NetMsg.ENCODE ( m_dwGaeaID );
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );

	//	[��Ƽ���鿡��]
	if ( m_dwPartyID!=PARTY_NULL && bparty )
	{
		GLMSG::SNET_PARTY_MBR_POINT NetMsg;
		NetMsg.dwGaeaID = m_dwGaeaID;
		NetMsg.sHP = m_sHP;
		NetMsg.sMP = m_sMP;
		//party search
		NetMsg.m_wLevel = m_wLevel;
		GLGaeaServer::GetInstance().SENDTOPARTYCLIENT ( m_dwPartyID, (NET_MSG_GENERIC*) &NetMsg );
	}

	//	����ڿ��� ä�� ��ȭ �˸�.
	GLMSG::SNETPC_UPDATE_STATE_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	NetMsgBrd.sHP = GLCHARLOGIC::m_sHP;
	NetMsgBrd.bSafeTime = IsSafeTime();

	if ( m_sCONFTING.IsCONFRONTING() && bconfront )
	{
		switch ( m_sCONFTING.emTYPE )
		{
		case EMCONFT_ONE:
			{
				PGLCHAR pCHAR = GLGaeaServer::GetInstance().GetChar ( m_sCONFTING.dwTAR_ID );
				if ( pCHAR )
				{
					GLGaeaServer::GetInstance().SENDTOCLIENT ( pCHAR->m_dwClientID, (NET_MSG_GENERIC*) &NetMsgBrd );
				}
			}
			break;
		case EMCONFT_PARTY:
			{
				GLPARTY_FIELD *pConftParty = GLGaeaServer::GetInstance().GetParty(m_sCONFTING.dwTAR_ID);
				if ( pConftParty )
				{
					GLPARTY_FIELD::MEMBER_ITER iter = pConftParty->m_cMEMBER.begin();
					GLPARTY_FIELD::MEMBER_ITER iter_end = pConftParty->m_cMEMBER.end();
					for ( ; iter!=iter_end; ++iter )
					{
						const GLPARTY_FNET &sMEMBER = (*iter).second;
						if ( !sMEMBER.m_bConfront )		continue;

						PGLCHAR pTAR = GLGaeaServer::GetInstance().GetChar(sMEMBER.m_dwGaeaID);
						if ( !pTAR )					continue;

						GLGaeaServer::GetInstance().SENDTOCLIENT ( pTAR->m_dwClientID, &NetMsgBrd );
					}
				}
			}
			break;
		case EMCONFT_GUILD:
			{
				GLClubMan &sClubMan = GLGaeaServer::GetInstance().GetClubMan();

				GLCLUB *pMY_CLUB = sClubMan.GetClub ( m_dwGuild );
				if ( pMY_CLUB )
				{
					CLUBMEMBERS_ITER pos = pMY_CLUB->m_mapMembers.begin();
					CLUBMEMBERS_ITER end = pMY_CLUB->m_mapMembers.end();
					for ( ; pos!=end; ++pos )
					{
						PGLCHAR pTAR = GLGaeaServer::GetInstance().GetCharID((*pos).first);
						if ( !pTAR )								continue;
						if ( !pTAR->m_sCONFTING.IsCONFRONTING() )	continue;

						GLGaeaServer::GetInstance().SENDTOCLIENT ( pTAR->m_dwClientID, &NetMsgBrd );
					}
				}

				GLCLUB *pTAR_CLUB = sClubMan.GetClub ( m_sCONFTING.dwTAR_ID );
				if ( pTAR_CLUB )
				{
					CLUBMEMBERS_ITER pos = pTAR_CLUB->m_mapMembers.begin();
					CLUBMEMBERS_ITER end = pTAR_CLUB->m_mapMembers.end();
					for ( ; pos!=end; ++pos )
					{
						PGLCHAR pTAR = GLGaeaServer::GetInstance().GetCharID((*pos).first);
						if ( !pTAR )								continue;
						if ( !pTAR->m_sCONFTING.IsCONFRONTING() )	continue;

						GLGaeaServer::GetInstance().SENDTOCLIENT ( pTAR->m_dwClientID, &NetMsgBrd );
					}
				}
			}
			break;
		};
	}

	//	Note : ���� �������� Ÿ�̸� ���� �� ����.
	{
		PGLCHAR pTAR = NULL;
		for ( MAPPLAYHOSTILE_ITER pos = m_mapPlayHostile.begin(); pos != m_mapPlayHostile.end(); ++pos )
		{
			pTAR = GLGaeaServer::GetInstance().GetCharID( pos->first );
			if ( pTAR )
				GLGaeaServer::GetInstance().SENDTOCLIENT ( pTAR->m_dwClientID, &NetMsgBrd );

			if( !pos->second )
				CDebugSet::ToLogFile( "GLChar::MsgSendUpdateState, pos->second = NULL" );
		}
	}

	bool bGuidBattleMap = m_pLandMan->m_bGuidBattleMap;
	bool bClubDeathMatch = m_pLandMan->m_bClubDeathMatchMap;
	bool bSCHOOL_FREEPK = GLSchoolFreePK::GetInstance().IsON();
	bool bBRIGHTEVENT   = GLGaeaServer::GetInstance().IsBRIGHTEVENT();


	// �����̺�Ʈ �̸� �ֺ����� 
	/*pvp tyranny, Juver, 2017/08/24 */
	/*school wars, Juver, 2018/01/19 */
	/*pvp capture the flag, Juver, 2018/01/31 */
	if ( bview || bGuidBattleMap || bClubDeathMatch || bSCHOOL_FREEPK || bBRIGHTEVENT || 
		m_pLandMan->m_bPVPTyrannyMap || m_pLandMan->m_bPVPSchoolWarsMap || m_pLandMan->m_bPVPCaptureTheFlagMap )
	{
		SendMsgViewAround((NET_MSG_GENERIC*)&NetMsgBrd);
	}
}

inline HRESULT GLChar::MsgReady ( NET_MSG_GENERIC* nmg )
{
	//	Note : Ȱ��ȭ ��Ŵ.
	//
	ReSetSTATE(EM_ACT_WAITING);
	return S_OK;
}

inline HRESULT GLChar::MsgMoveState ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_MOVESTATE *pNetMsg = reinterpret_cast<GLMSG::SNETPC_MOVESTATE*> ( nmg );

	DWORD dwOldActState = m_dwActState;

	if ( m_dwUserLvl >= USER_GM3 )
	{
		if ( pNetMsg->dwActState & EM_REQ_VISIBLENONE )	SetSTATE ( EM_REQ_VISIBLENONE );
		else											ReSetSTATE ( EM_REQ_VISIBLENONE );
		
		if ( pNetMsg->dwActState & EM_REQ_VISIBLEOFF )	SetSTATE ( EM_REQ_VISIBLEOFF );
		else											ReSetSTATE ( EM_REQ_VISIBLEOFF );
	}

	if ( pNetMsg->dwActState&EM_ACT_RUN )			SetSTATE(EM_ACT_RUN);
	else											ReSetSTATE(EM_ACT_RUN);

	if ( pNetMsg->dwActState & EM_ACT_PEACEMODE )	SetSTATE ( EM_ACT_PEACEMODE );
	else											ReSetSTATE ( EM_ACT_PEACEMODE );

	if ( dwOldActState != m_dwActState )
	{
		//	Note : �����̸� �ݿ��� �̵� �ӵ� ����.
		float fVelo = GetMoveVelo ();

		m_actorMove.SetMaxSpeed ( fVelo );

		//	Note : �ڽ��� �ֺ� Char���� �ڽ��� Msg�� ����.
		//
		GLMSG::SNETPC_MOVESTATE_BRD NetMsgFB;
		NetMsgFB.dwGaeaID = m_dwGaeaID;
		NetMsgFB.dwActState = m_dwActState;
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgFB );
	}

	return S_OK;
}

// *****************************************************
// Desc: �̵� ó��
// *****************************************************
inline HRESULT GLChar::MsgGoto ( NET_MSG_GENERIC* nmg )
{
	if( GLGaeaServer::GetInstance().m_bEmptyMsg ) return S_OK;
	if ( m_sPMarket.IsOpen() ) return E_FAIL;
	
	//stun visual bug fixed
	if ( m_bSTATE_STUN )  
	{
		m_actorMove.Stop ();

		return E_FAIL;
	}

	// �����°� DEAD �̸� DEAD Animation ó��
	if ( IsSTATE(EM_ACT_DIE) )
	{
		//	Note : �������� Ŭ���̾�Ʈ �鿡�� �˸�.
		GLMSG::SNET_ACTION_BRD NetMsgBrd;
		NetMsgBrd.emCrow	= CROW_PC;
		NetMsgBrd.dwID		= m_dwGaeaID;
		NetMsgBrd.emAction	= GLAT_FALLING;

		//	Note : �ڽ��� Ŭ���̾�Ʈ��.
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsgBrd );

		return E_FAIL;
	}

	GLMSG::SNETPC_GOTO *pNetMsg = reinterpret_cast<GLMSG::SNETPC_GOTO*> ( nmg );

	BOOL bRun = IsSTATE ( EM_ACT_RUN );				// �޸��� ��������
	BOOL bToRun = pNetMsg->dwActState&EM_ACT_RUN;   // �޷��� �ϴ���

	// ���� ����
	if ( bRun != bToRun )
	{
		if ( bToRun )		SetSTATE(EM_ACT_RUN);
		else				ReSetSTATE(EM_ACT_RUN);
	}

	D3DXVECTOR3 vDist = m_vPos - pNetMsg->vCurPos;
	float fDist = D3DXVec3Length(&vDist);
	if ( fDist > 60.0f )
	{
		// ������ ���̸� ������ ������ ( ������ �����̵� ���� )
		if ( m_Action == GLAT_TALK || m_Action == GLAT_GATHERING )
			return E_FAIL;
			
		TurnAction ( GLAT_IDLE );

		//	Note : �ڽ��� Ŭ���̾�Ʈ�� ��ġ ���� ����.
		//
		GLMSG::SNET_GM_MOVE2GATE_FB NetMsgFB;
		NetMsgFB.vPOS = m_vPos;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

		//	Note : Ŭ���̾�Ʈ�鿡�� ��ġ ���� ����.
		//
		GLMSG::SNETPC_JUMP_POS_BRD NetMsgJump;
		NetMsgJump.dwGaeaID = m_dwGaeaID;
		NetMsgJump.vPOS = m_vPos;
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgJump );

		return S_OK;
	}

	m_TargetID.vPos = pNetMsg->vTarPos;
	TurnAction ( GLAT_MOVE );

	BOOL bSucceed = m_actorMove.GotoLocation
	(
		m_TargetID.vPos+D3DXVECTOR3(0,+10,0),
		m_TargetID.vPos+D3DXVECTOR3(0,-10,0)
	);

	if ( !bSucceed )
	{
		//CDebugSet::ToLogFile ( "m_actorMove.GotoLocation error : %f, %f, %f", m_TargetID.vPos.x, m_TargetID.vPos.y, m_TargetID.vPos.z );
	}

	if ( bSucceed )
	{
		float fVelo = GetMoveVelo ();
		m_actorMove.SetMaxSpeed ( fVelo );
	
		//	Note : �ڽ��� �ֺ� Char���� �ڽ��� Msg�� ����.
		//
		GLMSG::SNETPC_GOTO_BRD NetMsgFB;
		NetMsgFB.dwGaeaID = m_dwGaeaID;
		NetMsgFB.dwActState = m_dwActState;
		NetMsgFB.vCurPos = m_vPos;
		NetMsgFB.vTarPos = m_TargetID.vPos;
		NetMsgFB.fDelay = 0.0f;

		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgFB );
	}

	MsgSendUpdateState(false,false,true);

	return S_OK;
}

inline HRESULT GLChar::MsgAttack ( NET_MSG_GENERIC* nmg )
{
	if( GLGaeaServer::GetInstance().m_bEmptyMsg )													return S_OK;
	if ( !IsValidBody() )																			return E_FAIL;
	if ( m_pLandMan && m_pLandMan->IsPeaceZone() )													return E_FAIL;

	GLMSG::SNETPC_ATTACK *pNetMsg = reinterpret_cast<GLMSG::SNETPC_ATTACK*> ( nmg );
	m_dwANISUBSELECT = pNetMsg->dwAniSel;
	m_TargetID.emCrow = pNetMsg->emTarCrow;
	m_TargetID.dwID = pNetMsg->dwTarID;

	//	Note : Ÿ�� �˻�.
	//
	GLACTOR* pTARGET = GLGaeaServer::GetInstance().GetTarget ( m_pLandMan, m_TargetID );
	if ( !pTARGET )																					return E_FAIL;

	// Ÿ�ٰ��� �Ÿ� �˻�
	D3DXVECTOR3 vTarPos = pTARGET->GetPosition();
	float fDist = D3DXVec3Length ( &D3DXVECTOR3(m_vPos-vTarPos) );
	WORD wAttackRange = pTARGET->GetBodyRadius() + GETBODYRADIUS() + GETATTACKRANGE() + 2;
	if ( ISLONGRANGE_ARMS() )	 wAttackRange += (WORD) GETSUM_TARRANGE();
	WORD wAttackAbleDis = wAttackRange + 7;

	if ( fDist > wAttackAbleDis )
	{
		//	Note : �ڽſ���.
		GLMSG::SNETPC_ATTACK_AVOID NetMsg;
		NetMsg.emTarCrow	= m_TargetID.emCrow;
		NetMsg.dwTarID		= m_TargetID.dwID;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsg );

		//	Note : �ֺ� Ŭ���̾�Ʈ�鿡�� �޼��� ����.
		//
		GLMSG::SNETPC_ATTACK_AVOID_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID	= m_dwGaeaID;
		NetMsgBrd.emTarCrow	= m_TargetID.emCrow;
		NetMsgBrd.dwTarID	= m_TargetID.dwID;
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
			
		return E_FAIL;
	}

	bool bFreePKMap		= m_pLandMan->IsFreePK();
	bool bGuidBattleMap = m_pLandMan->m_bGuidBattleMap;
	bool bBRIGHTEVENT   = GLGaeaServer::GetInstance().IsBRIGHTEVENT();
	bool bSCHOOL_FREEPK = GLSchoolFreePK::GetInstance().IsON();
	if ( GetSchool() == pTARGET->GetSchool() )	bSCHOOL_FREEPK = false;

	bool bREACTIONALBE  = IsReActionable(pTARGET);
	if ( !bREACTIONALBE )																			return E_FAIL;

	//	Note : pc �̰� pk ����� ��� ���� ����.
	if ( pTARGET->GetCrow()==CROW_PC )
	{
		PGLCHAR pCHAR = GLGaeaServer::GetInstance().GetChar ( m_TargetID.dwID );
		if ( pCHAR )
		{

			// ������ ��� ������ ����Ÿ���̸� ���� ����
			if ( pCHAR->m_sCONFTING.IsPOWERFULTIME () )	
				return E_FAIL;
			// ���� ��� ������ ����Ÿ���̸� ���� ����
			if ( m_sCONFTING.IsPOWERFULTIME() )			
				return E_FAIL;

			bool bClubBattle = false;
			bool bClubDeathMatch = false;
			
			if ( m_pLandMan->IsClubBattleZone() )
			{	
				GLClubMan &cClubMan = m_pGLGaeaServer->GetClubMan();
				GLCLUB *pMyClub = cClubMan.GetClub ( m_dwGuild );
				GLCLUB *pTarClub = cClubMan.GetClub ( pCHAR->m_dwGuild );

				if ( pMyClub && pTarClub )
				{
					bool bClub = pMyClub->IsBattle( pCHAR->m_dwGuild );
					bool bAlliance = pMyClub->IsBattleAlliance ( pTarClub->m_dwAlliance );
					bClubBattle = (bClub || bAlliance);
				}				
			}

			if ( m_pLandMan->m_bClubDeathMatchMap )
			{
				GLClubDeathMatch* pCDM = GLClubDeathMatchFieldMan::GetInstance().Find( m_pLandMan->m_dwClubMapID );
				if ( pCDM && pCDM->IsBattle() ) bClubDeathMatch = true;
			}

			/*pvp tyranny, Juver, 2017/08/24 */
			bool bPVPTyranny = false;
			if ( m_pLandMan->m_bPVPTyrannyMap && GLPVPTyrannyField::GetInstance().IsBattle() )
				bPVPTyranny = true;

			/*school wars, Juver, 2018/01/19 */
			bool bPVPSchoolWars = false;
			if ( m_pLandMan->m_bPVPSchoolWarsMap && GLPVPSchoolWarsField::GetInstance().IsBattle() )
				bPVPSchoolWars = true;

			/*pvp capture the flag, Juver, 2018/01/31 */
			bool bPVPCaptureTheFlag = false;
			if ( m_pLandMan->m_bPVPCaptureTheFlagMap && GLPVPCaptureTheFlagField::GetInstance().IsBattle() )
				bPVPCaptureTheFlag = true;

			/*pvp tyranny, Juver, 2017/08/24 */
			/*school wars, Juver, 2018/01/19 */
			/*pvp capture the flag, Juver, 2018/01/31 */
			if ( !(bGuidBattleMap || bClubDeathMatch || bSCHOOL_FREEPK || bFreePKMap || bBRIGHTEVENT || IsConflictTarget(pTARGET) || bClubBattle || 
				bPVPTyranny || bPVPSchoolWars || bPVPCaptureTheFlag ) )
			{
				//	Note : ������ �����ڰ� �ƴϰ� ������� �ð��� �������� �ʾ������� 
				//		�ڽ��� ���������� ������.
				if ( !IS_PLAYHOSTILE(pCHAR->m_dwCharID) && !pCHAR->ISOFFENDER() )
				{
					int nBRIGHT(GLCONST_CHAR::nPK_TRY_BRIGHT_POINT), nLIFE(GLCONST_CHAR::nPK_TRY_LIVING_POINT);

					//	Note : �Ӽ���ġ ��ȭ.
					m_nBright += nBRIGHT;

					GLMSG::SNETPC_UPDATE_BRIGHT NetMsg;
					NetMsg.nBright = m_nBright;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg);

					GLMSG::SNETPC_UPDATE_BRIGHT_BRD NetMsgBrd;
					NetMsgBrd.dwGaeaID = m_dwGaeaID;
					NetMsgBrd.nBright = m_nBright;
					SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

					//	Note : ��Ȱ���� ��ȭ.
					m_nLiving += nLIFE;

					GLMSG::SNETPC_UPDATE_LP NetMsgLp;
					NetMsgLp.nLP = m_nLiving;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgLp);

					//	Note : �������� �α� ���.
					GLITEMLMT::GetInstance().ReqAction
					(
						m_dwCharID,					//	�����.
						EMLOGACT_HOSTILE,			//	����.
						ID_CHAR, pCHAR->m_dwCharID,	//	����.
						0,							//	exp
						nBRIGHT,					//	bright
						nLIFE,						//	life
						0							//	money
					);
				}
			}

			if ( !IsConflictTarget(pTARGET) )
			{
				//	Note : ���� �����ڿ��� ������ ���. ( pk )
				AddPlayHostile ( pCHAR->m_dwCharID, TRUE, bClubBattle );

				//	Note : ���� �����ڿ��� ������ ���. ( PK )
				pCHAR->AddPlayHostile ( m_dwCharID, FALSE, bClubBattle);
			}

			STARGETID sTargetID = STARGETID ( CROW_PC, m_dwGaeaID, m_vPos );
			
			// �����ڿ� ���ݴ���� ��� ���� ���ݸ��� ����
			// �´���
			PGLPETFIELD pEnemyPet = GLGaeaServer::GetInstance().GetPET ( pCHAR->m_dwPetGUID );
			if ( pEnemyPet && pEnemyPet->IsValid () && !pEnemyPet->IsSTATE ( EM_PETACT_ATTACK ) )
			{
				D3DXVECTOR3 vOwnerPos, vDist;
				float fDist;
				vOwnerPos = pCHAR->GetPosition ();
				vDist = pEnemyPet->m_vPos - vOwnerPos;
				fDist = D3DXVec3Length(&vDist);

				// �����Ÿ� �ȿ� ������ 
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
			// �´��ڸ� Summon Attack ó��

			/*skill summon, Juver, 2017/10/09 */
			for ( int ii=0; ii<SKILL_SUMMON_MAX_CLIENT_NUM; ++ii )
			{
				PGLSUMMONFIELD pEnemySummon = GLGaeaServer::GetInstance().GetSummon ( pCHAR->m_dwSummonGUID_FLD[ii] );
				if ( pEnemySummon && pEnemySummon->IsValid () && !pEnemySummon->IsSTATE ( EM_SUMMONACT_ATTACK ) && 
					pEnemySummon->GetAttackTarget( sTargetID ) )
				{
					D3DXVECTOR3 vOwnerPos, vDist;
					float fDist;
					vOwnerPos = pCHAR->GetPosition ();
					vDist = pEnemySummon->m_vPos - vOwnerPos;
					fDist = D3DXVec3Length(&vDist);

					// �����Ÿ� �ȿ� ������ 
					//if ( fDist <= pEnemySummon->m_fWalkArea )
					{
						pEnemySummon->SetAttackTarget( sTargetID );
					}
				}
			}
			

			// ������
			PGLPETFIELD pMyPet = GLGaeaServer::GetInstance().GetPET ( m_dwPetGUID );
			if ( pMyPet && pMyPet->IsValid () && !pMyPet->IsSTATE ( EM_PETACT_ATTACK ) )
			{
				D3DXVECTOR3 vDist;
				float fDist;
				vDist = pMyPet->m_vPos - m_vPos;
				fDist = D3DXVec3Length(&vDist);

				// �����Ÿ� �ȿ� ������ 
				if ( fDist <= GLCONST_PET::fWalkArea )
				{
					pMyPet->ReSetAllSTATE ();
					pMyPet->SetSTATE ( EM_PETACT_ATTACK );

					m_TargetID.vPos = pCHAR->GetPosition ();

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
	}else if ( pTARGET->GetCrow()==CROW_SUMMON )
	{
		STARGETID sTargetID = STARGETID ( CROW_PC, m_dwGaeaID, m_vPos );

		// Summon Attack ó��
		PGLSUMMONFIELD pEnemySummon = GLGaeaServer::GetInstance().GetSummon ( m_TargetID.dwID );
		if ( pEnemySummon && pEnemySummon->IsValid () && !pEnemySummon->IsSTATE ( EM_SUMMONACT_ATTACK ) && 
			pEnemySummon->GetAttackTarget( sTargetID ) )
		{
			/*D3DXVECTOR3 vOwnerPos, vDist;
			float fDist;
			vOwnerPos = pEnemySummon->m_pOwner->GetPosition ();
			vDist = pEnemySummon->m_vPos - vOwnerPos;
			fDist = D3DXVec3Length(&vDist);

			if ( fDist <= pEnemySummon->m_fWalkArea )*/
			{
				pEnemySummon->SetAttackTarget( sTargetID );
			}
		}
	}

	EMSLOT emRHand = GetCurRHand();
	EMSLOT emLHand = GetCurLHand();
    
	EMANI_SUBTYPE emANISUBTYPE = CHECK_ATTACK_ANISUB ( m_pITEMS[emRHand], m_pITEMS[emLHand] );

	GLCONST_CHARCLASS &ConstCharClass = GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX];
	VECANIATTACK &vecAniAttack = ConstCharClass.m_ANIMATION[AN_ATTACK][emANISUBTYPE];
	if ( vecAniAttack.empty() )		return E_FAIL;
	if ( vecAniAttack.size() <= m_dwANISUBSELECT )	m_dwANISUBSELECT = 0;

	const SANIATTACK &sAniAttack = vecAniAttack[m_dwANISUBSELECT];
	WORD wStrikeNum = sAniAttack.m_wDivCount;

	m_TargetID.vPos = pTARGET->GetPosition();

	//	Note : ������ �������� �˻�.
	//
	EMBEGINATTACK_FB emBeginFB = BEGIN_ATTACK(wStrikeNum);
	if ( emBeginFB!=EMBEGINA_OK && emBeginFB!=EMBEGINA_SP )		return E_FAIL;

	//	Note : �Ҹ� ������ ��ȿ�� �˻���, ������� �޽��� �߻�.
	//
	CheckInstanceItem ();

	//	Note : ���� ����ġ ���.
	//
	BOOL bLowSP = (emBeginFB==EMBEGINA_SP) ? TRUE: FALSE;
	PreStrikeProc ( FALSE, bLowSP );

	//	Note : ���� ����.
	//
	TurnAction ( GLAT_ATTACK );

	//	Note : ���� ������ - �ڽ��� �ֺ� Char���� �ڽ��� Msg�� ����.
	//
	GLMSG::SNETPC_ATTACK_BRD NetMsgFB;
	NetMsgFB.dwGaeaID = m_dwGaeaID;
	NetMsgFB.emTarCrow = pNetMsg->emTarCrow;
	NetMsgFB.dwTarID = pNetMsg->dwTarID;
	NetMsgFB.dwAniSel = m_dwANISUBSELECT;
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgFB );

	// ���� �����ϸ� ���� ���ݸ���� ����
	if ( pTARGET->GetCrow()==CROW_MOB )
	{
		PGLPETFIELD pMyPet = GLGaeaServer::GetInstance().GetPET ( m_dwPetGUID );
		if ( pMyPet && pMyPet->IsValid () && !pMyPet->IsSTATE ( EM_PETACT_ATTACK ) )
		{
			D3DXVECTOR3 vDist;
			float fDist;
			vDist = pMyPet->m_vPos - m_vPos;
			fDist = D3DXVec3Length(&vDist);

			// �����Ÿ� �ȿ� ������ 
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

inline HRESULT GLChar::MsgAttackCancel ( NET_MSG_GENERIC* nmg )
{
	//	Note : �׼� ���.
	if ( IsACTION(GLAT_ATTACK) )	TurnAction ( GLAT_IDLE );

	//	Note : �ڽ��� �ֺ� Char���� �ڽ��� Msg�� ����.
	//
	GLMSG::SNETPC_ATTACK_CANCEL_BRD NetMsgFB;
	NetMsgFB.dwGaeaID = m_dwGaeaID;
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgFB );

	return S_OK;
}

inline HRESULT GLChar::MsgGatheringCancel ( NET_MSG_GENERIC* nmg )
{
	//	Note : �׼� ���.
	if ( IsACTION(GLAT_GATHERING) )	TurnAction ( GLAT_IDLE );
	return S_OK;
}

void GLChar::ReSetStartMap()
{
	// ����Ʈ�� �� ã���� ���� ���� ��ġ�� �̵��Ѵ�.
	m_sStartMapID = GLCONST_CHAR::nidSTARTMAP[m_wSchool];
	m_dwStartGate = GLCONST_CHAR::dwSTARTGATE[m_wSchool];
	m_vStartPos   = D3DXVECTOR3(0.0f,0.0f,0.0f);

	// ������ġ ���� (���п��� ���۸�����)
	GLMSG::SNETPC_UPDATE_STARTCALL NetMsg;
	NetMsg.sStartMapID   = m_sStartMapID;
	NetMsg.dwStartGateID = m_dwStartGate;
	NetMsg.vStartPos	 = m_vStartPos;

	// Ŭ���̾�Ʈ�� �˸�
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );

	// Agent�� �˸�
	GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsg );		

	return;
}

BOOL GLChar::CheckStartMap()
{
	GLLandMan *pLandMan=NULL;
	DWORD dwGenGate = 0;
	pLandMan = GLGaeaServer::GetInstance().GetByMapID ( m_sStartMapID );

	if ( pLandMan && pLandMan->m_bClubDeathMatchMap )
	{
		if ( GLCONST_CHAR::bPKLESS ) return FALSE;	

	    GLClubDeathMatch *pCDM = GLClubDeathMatchFieldMan::GetInstance().Find ( pLandMan->m_dwClubMapID );
		if ( pCDM && !pCDM->IsEnterMap( m_dwGuild ) )	return FALSE;
		
		GLCLUB* pClub = GLGaeaServer::GetInstance().GetClubMan().GetClub ( m_dwGuild );
		if ( !pClub ) return FALSE;
		if ( pClub->GetAllBattleNum() > 0 ) return FALSE;
		if ( !pClub->IsMemberFlgCDM(m_dwCharID) )	return FALSE;
	}

	/*pvp tyranny, Juver, 2017/08/24 */
	/*school wars, Juver, 2018/01/19 */
	/*pvp capture the flag, Juver, 2018/01/31 */
	if ( pLandMan && ( pLandMan->m_bPVPTyrannyMap || pLandMan->m_bPVPSchoolWarsMap || pLandMan->m_bPVPCaptureTheFlagMap ) )
	{
		return FALSE;
	}

	if ( pLandMan && pLandMan->GetLandGateMan().FindLandGate(m_dwStartGate) ) 		
	{
		return TRUE;
	}
	
	return FALSE;

}


inline void GLChar::DoReBirth2StartMap ()
{
	SNATIVEID nidSTARTMAP = m_sStartMapID;
	DWORD dwSTARTGATE = m_dwStartGate;

	//	��Ȱ.
	GLLandMan *pLandMan=NULL;
	DWORD dwGenGate = 0;


	//	�������� ��Ȳ 
	if ( CheckStartMap() )
	{
		pLandMan = GLGaeaServer::GetInstance().GetByMapID ( nidSTARTMAP );
		if ( pLandMan )
		{
			BOOL bOk = GLGaeaServer::GetInstance().RequestReBirth ( m_dwGaeaID, pLandMan->GetMapID(), dwSTARTGATE, D3DXVECTOR3(0,0,0) );
			if ( !bOk )	DEBUGMSG_WRITE ( "GLGaeaServer::GetInstance().RequestReBirth () bOk false" );
			return;
		}
		else
		{
			//	���ü� ���� ��Ȳ
			DEBUGMSG_WRITE ( "GLGaeaServer::GetInstance().RequestReBirth () pLandMan NULL" );
			return;
		}
	}
	else	//	������ġ�� �̵�
	{
		// ����Ʈ�� �� ã���� ���� ���� ��ġ�� �̵��Ѵ�.
		ReSetStartMap();

		pLandMan = GLGaeaServer::GetInstance().GetByMapID ( m_sStartMapID );
		if ( pLandMan )
		{
			DxLandGateMan* pLandGateMan = &pLandMan->GetLandGateMan();
			if ( !pLandGateMan->FindLandGate ( m_dwStartGate ) )
			{
				DEBUGMSG_WRITE ( "(Revive) - Default gate is a failure. school %d", m_wSchool );
				return;
			}
			else
			{
//				m_bEntryFailed = TRUE;
				BOOL bOk = GLGaeaServer::GetInstance().RequestReBirth ( m_dwGaeaID, pLandMan->GetMapID(), m_dwStartGate, D3DXVECTOR3(0,0,0) );	
				if ( !bOk )	DEBUGMSG_WRITE ( "GLGaeaServer::GetInstance().RequestReBirth () bOk false" );
				return;
			}
		}
		else
		{
			DEBUGMSG_WRITE ( "(Revive) - Default map is a failure. school %d", m_wSchool );
			return;
		}
	}

	return;

/*


	//	��Ȱ�� Map�� Gate ��ȿ�� �˻�.
	if ( nidSTARTMAP!=NATIVEID_NULL() )
	{
		pLandMan = GLGaeaServer::GetInstance().GetByMapID ( nidSTARTMAP );

		if ( pLandMan )
		{
			DxLandGateMan* pLandGateMan = &pLandMan->GetLandGateMan();
			if ( pLandGateMan->FindLandGate(dwSTARTGATE)==NULL )
			{
				//PDXLANDGATE pLandGate = pLandGateMan->GetListLandGate();
				//if ( !pLandGate )	pLandMan = NULL;	//	Gate�� �ϳ��� ���� ���� ��Ȱ�� �� ����.
				//else
				//{
				//	dwGenGate = pLandGate->GetGateID();	//	ù��° Gate ���� ����.
				//}
				// ����Ʈ�� �� ã���� ���� ���� ��ġ�� �̵��Ѵ�.
				m_sStartMapID = GLCONST_CHAR::nidSTARTMAP[m_wSchool];
				m_dwStartGate = GLCONST_CHAR::dwSTARTGATE[m_wSchool];
				m_vStartPos   = D3DXVECTOR3(0.0f,0.0f,0.0f);

				// ������ġ ���� (���п��� ���۸�����)
				GLMSG::SNETPC_UPDATE_STARTCALL NetMsg;
				NetMsg.sStartMapID   = m_sStartMapID;
				NetMsg.dwStartGateID = m_dwStartGate;
				NetMsg.vStartPos	 = m_vStartPos;

				// Ŭ���̾�Ʈ�� �˸�
				GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );

				// Agent�� �˸�
				GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsg );

				pLandMan = GLGaeaServer::GetInstance().GetByMapID ( m_sStartMapID );
				DxLandGateMan* pLandGateMan = &pLandMan->GetLandGateMan();
				if ( !pLandGateMan->FindLandGate ( m_dwStartGate ) )
				{
					DEBUGMSG_WRITE ( "Gate == NULL (Revive) - Default gate is a failure. school %d", m_wSchool );
					return;
				}else{
					dwGenGate = m_dwStartGate;
				}

				m_bEntryFailed = TRUE;
			}
			else	dwGenGate = dwSTARTGATE;
		}
	}

	if ( !pLandMan )
	{
		SNATIVEID nidSTARTMAP = GLCONST_CHAR::nidSTARTMAP[m_wSchool];
		DWORD dwSTARTGATE = GLCONST_CHAR::dwSTARTGATE[m_wSchool];

		pLandMan = GLGaeaServer::GetInstance().GetByMapID ( nidSTARTMAP );
		if ( pLandMan )
		{
			DxLandGateMan* pLandGateMan = &pLandMan->GetLandGateMan();
			if ( !pLandGateMan->FindLandGate ( dwSTARTGATE ) )
			{
				DEBUGMSG_WRITE ( "(Revive) - Default gate is a failure. school %d", m_wSchool );
				return;
			}
			else
			{
				dwGenGate = dwSTARTGATE;
			}
		}
	}

	if ( !pLandMan )
	{
		DEBUGMSG_WRITE ( "(Revive) - Default map is a failure. school %d", m_wSchool );
		return;
	}

	//	Note : ��Ȱ map�� ��Ȱ gate ����, ��ġ�� �ǹ� ���� ��.
	BOOL bOk = GLGaeaServer::GetInstance().RequestReBirth ( m_dwGaeaID, pLandMan->GetMapID(), dwGenGate, D3DXVECTOR3(0,0,0) );
	if ( !bOk )
	{
		DEBUGMSG_WRITE ( "GLGaeaServer::GetInstance().RequestReBirth () bOk false" );
		return;
	}
*/
}

// *****************************************************
// Desc: ��Ȱ��û ó�� ( Ageng->Field )
// *****************************************************
inline HRESULT GLChar::MsgReqReBirth ( NET_MSG_GENERIC* nmg )
{
	if ( !IsSTATE(EM_ACT_DIE) )
	{
		DEBUGMSG_WRITE ( "GLChar::MsgReqReBirth IsSTATE(EM_ACT_DIE) false" );
		return S_OK;
	}

	if( m_bDeathTimer && GLGaeaServer::GetInstance().IsEmulatorMode() )
	{
		//emulator dont send packets to agent
		//fastrevive fix
		GLMSG::SNETPC_REQ_REVIVE_DEATHTIMER_FB NetMsgFB;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return FALSE;
	}

	//	��Ȱ �Ϸ� üũ.
	ReSetSTATE(EM_ACT_DIE);

	//  ��Ȱ �������� ���� �Ұ��� ��Ȱ ������ �ʱ�ȭ�Ѵ�.
	GLMSG::SNETPC_REQ_REBIRTH *pNetMsg = (GLMSG::SNETPC_REQ_REBIRTH *) nmg;
	if( pNetMsg->bRegenEntryFailed )
	{
		m_sStartMapID = GLCONST_CHAR::nidSTARTMAP[m_wSchool];
		m_dwStartGate = GLCONST_CHAR::dwSTARTGATE[m_wSchool];
		m_vStartPos   = D3DXVECTOR3(0.0f,0.0f,0.0f);
	}

	//	Note : ������ ��Ȱ�������� ��Ȱ�ǵ��� ��ġ �̵�.
	//
	DoReBirth2StartMap ();

	//	��Ȱ�� ä�� ȸ��.
	m_fGenAge = 0.0f;
	GLCHARLOGIC::INIT_RECOVER();
	TurnAction ( GLAT_IDLE );

	//	Note : ��Ȱ�� ����ġ ����.
	ReBirthDecExp ();

	// PET
	// ���̵��� Pet ����
	GLGaeaServer::GetInstance().ReserveDropOutPet ( SDROPOUTPETINFO(m_dwPetGUID,true,true) );
	//GLGaeaServer::GetInstance().ReserveDropOutSummon ( SDROPOUTSUMMONINFO(m_dwSummonGUID,true) );
	GLGaeaServer::GetInstance().SaveVehicle( m_dwClientID, m_dwGaeaID, true );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		GLGaeaServer::GetInstance().ReserveDropOutSummon ( m_dwSummonGUID_FLD[i] );

	//	��Ȱ Ȯ�� �޽���.
	GLMSG::SNETPC_REQ_REBIRTH_FB NetMsg;
	NetMsg.sMapID = m_pLandMan->GetMapID();
	NetMsg.vPos = GetPosition();
	NetMsg.dwNowHP = m_sHP.dwNow;
	NetMsg.dwNowMP = m_sMP.dwNow;
	NetMsg.dwNowSP = m_sSP.dwNow;
	NetMsg.bRegenEntryFailed = pNetMsg->bRegenEntryFailed;

	//	Note : ������ �α� ���.
	// ��Ȱ�ϱ� ��ư�� ������ �ʾ� �α׸� ������� ��찡 �־ ������ ������ �����.
	//{
	//	EMIDTYPE emKILL = ID_MOB;
	//	if ( m_sAssault.emCrow==CROW_PC )	emKILL = ID_CHAR;

	//	GLITEMLMT::GetInstance().ReqAction
	//		(
	//		m_dwCharID,					//	�����.
	//		EMLOGACT_DIE,				//	����.
	//		emKILL, 0,					//	����.
	//		m_nDECEXP,					// exp
	//		0,							// bright
	//		0,							// life
	//		0							// money
	//		);
	//}


	GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID,&NetMsg);

	return S_OK;
}

// *****************************************************
// Desc: ��Ȱ��û ó�� (��ȥ�� ���)
// *****************************************************
inline HRESULT GLChar::MsgReqRevive ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_REVIVE *pNetMsg = (GLMSG::SNETPC_REQ_REVIVE *)nmg;
	GLMSG::SNETPC_REQ_REVIVE_FB MsgFB;

	if ( m_bDeathTimer )
	{
		//fastrevive fix
		GLMSG::SNETPC_REQ_REVIVE_DEATHTIMER_FB NetMsgFB;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return FALSE;
	}

	// �ƾ����� �ִ���
	SITEM* pITEM = GET_SLOT_ITEMDATA(SLOT_ORNAMENT);
	if ( !pITEM )
	{
		MsgFB.emFB = EMREQ_REVIVE_FB_NOITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	if ( CheckCoolTime ( pITEM->sBasicOp.sNativeID ) )
	{
		MsgFB.emFB = EMREQ_REVIVE_FB_COOLTIME;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

/*
	// ��ȥ�� ����
	if ( pITEM->sBasicOp.emItemType != ITEM_REVIVE )
	{
		MsgFB.emFB = EMREQ_REVIVE_FB_NOITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}
*/
	// ������ �Ӽ��� ��Ȱ�������
	if ( pITEM->sDrugOp.emDrug!=ITEM_DRUG_CALL_REVIVE )
	{
		MsgFB.emFB = EMREQ_REVIVE_FB_NOITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	// ����������
	if ( m_pLandMan && m_pLandMan->m_bGuidBattleMap )
	{
		MsgFB.emFB = EMREQ_REVIVE_FB_NOTUSE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	// Ŭ��������ġ ��
	if ( m_pLandMan->m_bClubDeathMatchMap )
	{
		MsgFB.emFB = EMREQ_REVIVE_FB_NOTUSE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	/*pvp tyranny, Juver, 2017/08/24 */
	if ( m_pLandMan->m_bPVPTyrannyMap )
	{
		MsgFB.emFB = EMREQ_REVIVE_FB_NOTUSE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	/*school wars, Juver, 2018/01/19 */
	if ( m_pLandMan->m_bPVPSchoolWarsMap )
	{
		MsgFB.emFB = EMREQ_REVIVE_FB_NOTUSE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	/*pvp capture the flag, Juver, 2018/01/31 */
	if ( m_pLandMan->m_bPVPCaptureTheFlagMap )
	{
		MsgFB.emFB = EMREQ_REVIVE_FB_NOTUSE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	//	Note : ��ȥ�� ���.
	//SITEMCUSTOM &sCUSTOM = m_PutOnItems[SLOT_ORNAMENT];

	//	Note : �Ҹ�� ��ȥ�� ó��.
	//
	DoDrugSlotItem(SLOT_ORNAMENT);

	//	Note : ��Ȱ map�� ��Ȱ gate ����, ��ġ�� �ǹ� ���� ��.
	BOOL bOk = GLGaeaServer::GetInstance().RequestReBirth ( m_dwGaeaID, m_pLandMan->GetMapID(), UINT_MAX, m_vPos );
	if ( !bOk )
	{
		DEBUGMSG_WRITE ( "ĳ���� ��Ȱ �õ��߿� ������ �߻�! GLGaeaServer::GetInstance().RequestReBirth ()" );
		return E_FAIL;
	}

	//	��Ȱ �Ϸ� üũ.
	ReSetSTATE(EM_ACT_DIE);

	//	��Ȱ�� ä�� ȸ��.
	m_fGenAge = 0.0f;
	GLCHARLOGIC::INIT_RECOVER(100);
	TurnAction ( GLAT_IDLE );	

	// PET
	// ���̵��� Pet ����
	GLGaeaServer::GetInstance().ReserveDropOutPet ( SDROPOUTPETINFO(m_dwPetGUID,true,true) );
	//GLGaeaServer::GetInstance().ReserveDropOutSummon ( SDROPOUTSUMMONINFO(m_dwSummonGUID,true) );
	GLGaeaServer::GetInstance().SaveVehicle( m_dwClientID, m_dwGaeaID, true );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		GLGaeaServer::GetInstance().ReserveDropOutSummon ( m_dwSummonGUID_FLD[i] );

	
	m_sHP.TO_FULL();
	m_sMP.TO_FULL();
	m_sSP.TO_FULL();

	//	��Ȱ Ȯ�� �޽���.
	GLMSG::SNETPC_REQ_REBIRTH_FB NetMsg;
	NetMsg.sMapID = m_pLandMan->GetMapID();
	NetMsg.vPos = GetPosition();
	NetMsg.dwNowHP = m_sHP.dwNow;
	NetMsg.dwNowMP = m_sMP.dwNow;
	NetMsg.dwNowSP = m_sSP.dwNow;

	GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID,&NetMsg);

	// ��ȥ�� ��� ��Ȱ ������ �˸�
	MsgFB.emFB = EMREQ_REVIVE_FB_OK;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	////	Note : ������ �α� ���.
	// ��Ȱ�ϱ� ��ư�� ������ �ʾ� �α׸� ������� ��찡 �־ ��ȥ�� ���� �α׸� �����.
	{

		GLITEMLMT::GetInstance().ReqAction
			(
			m_dwCharID,					//	�����.
			EMLOGACT_RESTORE,			//	����.
			ID_CHAR, 0,					//	����.
			m_nDECEXP,					// exp
			0,							// bright
			0,							// life
			0							// money
			);
	}

	return S_OK;
}

HRESULT GLChar::MsgReqRecovery ( NET_MSG_GENERIC* nmg )
{
	/*recovery exp setting, Juver, 2017/11/18 */
	if ( !RANPARAM::bFeatureUseRecoveryEXP )	return E_FAIL;
	
	if ( m_bDeathTimer )
	{
		//fastrevive fix
		GLMSG::SNETPC_REQ_REVIVE_DEATHTIMER_FB NetMsgFB;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return FALSE;
	}

	GLMSG::SNETPC_REQ_RECOVERY *pNetMsg = (GLMSG::SNETPC_REQ_RECOVERY *)nmg;
	GLMSG::SNETPC_REQ_RECOVERY_FB MsgFB;

	LONGLONG nReExp = 0;
	LONGLONG nDecMoney = 0;

	nReExp = GetReExp();
	nDecMoney = GetReExpMoney ( nReExp );

	if ( nDecMoney > m_lnMoney )
	{
		MsgFB.emFB = EMREQ_RECOVERY_FB_NOMONEY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	if ( nReExp <= 0  )
	{
		MsgFB.emFB = EMREQ_RECOVERY_FB_NOREEXP;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}


	// ����������
	if ( m_pLandMan && m_pLandMan->m_bGuidBattleMap )
	{
		MsgFB.emFB = EMREQ_RECOVERY_FB_NOTUSE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	// Ŭ��������ġ��
	if ( m_pLandMan->m_bClubDeathMatchMap )
	{
		MsgFB.emFB = EMREQ_RECOVERY_FB_NOTUSE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	/*pvp tyranny, Juver, 2017/08/24 */
	if ( m_pLandMan->m_bPVPTyrannyMap )
	{
		MsgFB.emFB = EMREQ_RECOVERY_FB_NOTUSE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	/*school wars, Juver, 2018/01/19 */
	if ( m_pLandMan->m_bPVPSchoolWarsMap )
	{
		MsgFB.emFB = EMREQ_RECOVERY_FB_NOTUSE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	/*pvp capture the flag, Juver, 2018/01/31 */
	if ( m_pLandMan->m_bPVPCaptureTheFlagMap )
	{
		MsgFB.emFB = EMREQ_RECOVERY_FB_NOTUSE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	//	Note : ��Ȱ map�� ��Ȱ gate ����, ��ġ�� �ǹ� ���� ��.
	BOOL bOk = GLGaeaServer::GetInstance().RequestReBirth ( m_dwGaeaID, m_pLandMan->GetMapID(), UINT_MAX, m_vPos );
	if ( !bOk )
	{
		DEBUGMSG_WRITE ( "ĳ���� ��Ȱ �õ��߿� ������ �߻�! GLGaeaServer::GetInstance().RequestReBirth ()" );
		return E_FAIL;
	}

	//	��Ȱ �Ϸ� üũ.
	ReSetSTATE(EM_ACT_DIE);

	//	��Ȱ�� ä�� ȸ��.
	m_fGenAge = 0.0f;
	GLCHARLOGIC::INIT_RECOVER(100);
	TurnAction ( GLAT_IDLE );
	
	if ( m_nDECEXP > m_sExperience.lnNow ) m_nDECEXP = m_sExperience.lnNow;

	if ( m_nDECEXP >= nReExp ) m_nDECEXP -= nReExp;

	CheckMoneyUpdate( m_lnMoney, nDecMoney, FALSE, "Recovery Exp" );
	m_bMoneyUpdate = TRUE;

	m_lnMoney -= nDecMoney;

	ReBirthDecExp ();

	m_lnReExp = 0;	// ����ġ ȸ�� ���ɷ� 0���� �ʱ�ȭ

	// PET
	// ���̵��� Pet ����
	GLGaeaServer::GetInstance().ReserveDropOutPet ( SDROPOUTPETINFO(m_dwPetGUID,true,true) );
	//GLGaeaServer::GetInstance().ReserveDropOutSummon ( SDROPOUTSUMMONINFO(m_dwSummonGUID,true) );
	GLGaeaServer::GetInstance().SaveVehicle( m_dwClientID, m_dwGaeaID, true );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		GLGaeaServer::GetInstance().ReserveDropOutSummon ( m_dwSummonGUID_FLD[i] );

	//	��Ȱ Ȯ�� �޽���.
	GLMSG::SNETPC_REQ_REBIRTH_FB NetMsg;
	NetMsg.sMapID = m_pLandMan->GetMapID();
	NetMsg.vPos = GetPosition();
	NetMsg.dwNowHP = m_sHP.dwNow;
	NetMsg.dwNowMP = m_sMP.dwNow;
	NetMsg.dwNowSP = m_sSP.dwNow;

	GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID,&NetMsg);

	// ����ġ ȸ�� ���� �˸�
	MsgFB.emFB = EMREQ_RECOVERY_FB_OK;
	MsgFB.nReExp = nReExp;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	GLMSG::SNETPC_UPDATE_MONEY NetMsgMoney;
	NetMsgMoney.lnMoney = m_lnMoney;
	/*dmk14 ingame web*/
	NetMsgMoney.dwPremium = m_dwPremiumPoints;
	NetMsgMoney.dwCombat = m_dwCombatPoints;
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgMoney );

	
	//	����ġ ��Ȱ �α�
	{

		GLITEMLMT::GetInstance().ReqAction
			(
			m_dwCharID,					//	�����.
			EMLOGACT_RECOVERY,			//	����.
			ID_CHAR, 0,					//	����.
			nReExp,						// exp
			0,							// bright
			0,							// life
			(int)nDecMoney				// money
			);
	}

	return S_OK;
}


// ������ ����ġ���� ���´�.
HRESULT GLChar::MsgReqGetExpRecovery ( NET_MSG_GENERIC* nmg )
{
	//fastrevive fix
	if ( m_bDeathTimer )
	{
		GLMSG::SNETPC_REQ_REVIVE_DEATHTIMER_FB NetMsgFB;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return FALSE;
	}
	GLMSG::SNETPC_REQ_GETEXP_RECOVERY *pNetMsg = (GLMSG::SNETPC_REQ_GETEXP_RECOVERY *)nmg;
	GLMSG::SNETPC_REQ_GETEXP_RECOVERY_FB MsgFB;

	LONGLONG nReExp = 0;
	LONGLONG nDecMoney = 0;
	LONGLONG nDecExp = 0;

	nDecExp = m_nDECEXP;
	if ( nDecExp > m_sExperience.lnNow ) nDecExp = m_sExperience.lnNow;

	nReExp = GetReExp();
	nDecMoney = GetReExpMoney ( nReExp );

	
	MsgFB.nDecExp = nDecExp;
	MsgFB.nReExp = nReExp;
	MsgFB.nDecMoney = nDecMoney;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);	

	return S_OK;
}

//	ȸ���� ����ġ���� �˷��ش�. NPC
HRESULT GLChar::MsgReqGetExpRecoveryNpc ( NET_MSG_GENERIC* nmg )
{
// ����ġȸ��_����_Npc
#if defined( _RELEASED ) || defined ( KRT_PARAM ) || defined ( KR_PARAM ) || defined ( TH_PARAM ) || defined ( MYE_PARAM ) || defined ( MY_PARAM ) || defined ( PH_PARAM ) || defined ( CH_PARAM ) || defined ( TW_PARAM ) || defined ( HK_PARAM ) || defined ( GS_PARAM )
	GLMSG::SNETPC_REQ_GETEXP_RECOVERY_NPC *pNetMsg = (GLMSG::SNETPC_REQ_GETEXP_RECOVERY_NPC *)nmg;
	GLMSG::SNETPC_REQ_GETEXP_RECOVERY_NPC_FB MsgFB;

	LONGLONG nReExp = 0;
	LONGLONG nDecMoney = 0;

	nReExp = m_lnReExp;
	nDecMoney = GetReExpMoney ( nReExp );	

	MsgFB.nReExp = nReExp;
	MsgFB.nDecMoney = nDecMoney;
	MsgFB.dwNPCID = pNetMsg->dwNPCID;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);	
#endif

	return S_OK;
}

//	����ġ�� ȸ���Ѵ�.
HRESULT GLChar::MsgReqRecoveryNpc ( NET_MSG_GENERIC* nmg )
{
// ����ġȸ��_����_Npc
#if defined( _RELEASED ) || defined ( KRT_PARAM ) || defined ( KR_PARAM ) || defined ( TH_PARAM ) || defined ( MYE_PARAM ) || defined ( MY_PARAM ) || defined ( PH_PARAM ) || defined ( CH_PARAM ) || defined ( TW_PARAM ) || defined ( HK_PARAM ) || defined ( GS_PARAM )
	GLMSG::SNETPC_REQ_RECOVERY_NPC *pNetMsg = (GLMSG::SNETPC_REQ_RECOVERY_NPC *)nmg;
	GLMSG::SNETPC_REQ_RECOVERY_NPC_FB MsgFB;

	LONGLONG nReExp = 0;
	LONGLONG nDecMoney = 0;

	nReExp = m_lnReExp;
	nDecMoney = GetReExpMoney ( nReExp );

	if ( nDecMoney > m_lnMoney )
	{
		MsgFB.emFB = EMREQ_RECOVERY_NPC_FB_NOMONEY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	if ( nReExp <= 0  )
	{
		MsgFB.emFB = EMREQ_RECOVERY_NPC_FB_NOREEXP;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	// NPC üũ 
	PGLCROW pCrow = m_pLandMan->GetCrow ( pNetMsg->dwNPCID );
	if ( !pCrow )
	{
		MsgFB.emFB = EMREQ_RECOVERY_NPC_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	float fDist = D3DXVec3Length ( &D3DXVECTOR3(m_vPos-pCrow->GetPosition()) );
	float fTalkRange = (float) (pCrow->GetBodyRadius() + GETBODYRADIUS() + 30);
	float fTalkableDis = fTalkRange + 20;

	if ( fDist>fTalkableDis )
	{
		MsgFB.emFB = EMREQ_RECOVERY_NPC_FB_NOTUSE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	CheckMoneyUpdate( m_lnMoney, nDecMoney, FALSE, "Recovery Exp To Npc" );
	m_bMoneyUpdate = TRUE;

	m_sExperience.lnNow += m_lnReExp;
	m_lnMoney -= nDecMoney;
	m_lnReExp = 0;	// ����ġ ȸ�� ���ɷ� 0���� �ʱ�ȭ		
	
	// ����ġ ȸ�� ���� �˸�
	MsgFB.emFB = EMREQ_RECOVERY_NPC_FB_OK;
	MsgFB.nReExp = nReExp;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	GLMSG::SNETPC_UPDATE_MONEY NetMsgMoney;
	NetMsgMoney.lnMoney = m_lnMoney;
	/*dmk14 ingame web*/
	NetMsgMoney.dwPremium = m_dwPremiumPoints;
	NetMsgMoney.dwCombat = m_dwCombatPoints;
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgMoney );

	
	//	����ġ ��Ȱ �α�
	{

		GLITEMLMT::GetInstance().ReqAction
			(
			m_dwCharID,					//	�����.
			EMLOGACT_RECOVERY_NPC,		//	����.
			ID_CHAR, 0,					//	����.
			nReExp,						// exp
			0,							// bright
			0,							// life
			(int)nDecMoney				// money
			);
	}

#endif
	
	return S_OK;


}

// *****************************************************
// Desc: ģ���̵� ó�� (Agent->Field)
// *****************************************************
inline HRESULT GLChar::MsgReqFriendFld ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_2_FRIEND_FLD *pNetMsg = (GLMSG::SNETPC_2_FRIEND_FLD *)nmg;
	GLMSG::SNETPC_2_FRIEND_FB MsgFB;

	//	Note : ������ ���� ���� �Ǵ�.
	SINVENITEM *pINVENITEM = m_cInventory.GetItem ( pNetMsg->wItemPosX, pNetMsg->wItemPosY );
	if ( !pINVENITEM )
	{
		MsgFB.emFB = EM2FRIEND_FB_NO_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return S_FALSE;
	}

	if ( CheckCoolTime( pINVENITEM->sItemCustom.sNativeID ) )	return S_FALSE;

	SITEM *pITEM_DATA = GLItemMan::GetInstance().GetItem ( pINVENITEM->sItemCustom.sNativeID );
	if ( !pITEM_DATA )
	{
		MsgFB.emFB = EM2FRIEND_FB_NO_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return S_FALSE;
	}

	if ( pITEM_DATA->sBasicOp.emItemType!=ITEM_2FRIEND )
	{
		MsgFB.emFB = EM2FRIEND_FB_NO_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return S_FALSE;
	}

	//	�ɸ��� ���� ���°� �ƴ� ���.
	if ( !IsValidBody() )
	{
		MsgFB.emFB = EM2FRIEND_FB_MY_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	//	��� ������ ���.
	if ( m_sCONFTING.IsCONFRONTING() )
	{
		MsgFB.emFB = EM2FRIEND_FB_MY_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	//	�ŷ����� ���.
	if ( m_sTrade.Valid() )
	{
		MsgFB.emFB = EM2FRIEND_FB_MY_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	SMAPNODE *pMapNode = GLGaeaServer::GetInstance().FindMapNode ( pNetMsg->sFriendMapID );
	if ( !pMapNode )
	{
		MsgFB.emFB = EM2FRIEND_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	if ( pMapNode->bBattleZone )
	{
		MsgFB.emFB = EM2FRIEND_FB_MAP_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	if ( pMapNode->bCDMZone )
	{
		MsgFB.emFB = EM2FRIEND_FB_MAP_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	/*pvp tyranny, Juver, 2017/08/24 */
	if ( pMapNode->bPVPTyrannyZone )
	{
		MsgFB.emFB = EM2FRIEND_FB_MAP_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	/*school wars, Juver, 2018/01/19 */
	if ( pMapNode->bPVPSchoolWarsZone )
	{
		MsgFB.emFB = EM2FRIEND_FB_MAP_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	/*pvp capture the flag, Juver, 2018/01/31 */
	if ( pMapNode->bPVPCaptureTheFlagZone )
	{
		MsgFB.emFB = EM2FRIEND_FB_MAP_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	//	Note : �ش� ������ ���� ���� �˻�.
	const SLEVEL_REQUIRE* pLEVEL_REQUIRE = GLGaeaServer::GetInstance().GetLevelRequire(pNetMsg->sFriendMapID);
	if ( !pLEVEL_REQUIRE )
	{
		MsgFB.emFB = EM2FRIEND_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	if ( m_dwUserLvl < USER_GM3 )
	{
		if ( pLEVEL_REQUIRE->ISCOMPLETE(this)!=EMREQUIRE_COMPLETE )
		{
			MsgFB.emFB = EM2FRIEND_FB_MAP_CONDITION;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
			return FALSE;
		}
	}

	//	Note : pk ����� ������ ��� �̻��� ��� ��ȯ ī���� ����� ���´�.
	//
	DWORD dwPK_LEVEL = GET_PK_LEVEL();
	if ( dwPK_LEVEL != UINT_MAX && dwPK_LEVEL>GLCONST_CHAR::dwPK_RECALL_ENABLE_LEVEL )
	{
		MsgFB.emFB = EM2FRIEND_FB_PK_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return E_FAIL;
	}

	//	Note : ������ �Ҹ�.
	DoDrugInvenItem ( pNetMsg->wItemPosX, pNetMsg->wItemPosY, true );

	SNATIVEID sMAPID = pNetMsg->sFriendMapID;
	DWORD dwGATEID(UINT_MAX);
	D3DXVECTOR3 vPOS = pNetMsg->vFriendPos;

	// PET
	// ���̵��� Pet ����
	GLGaeaServer::GetInstance().ReserveDropOutPet ( SDROPOUTPETINFO(m_dwPetGUID,true,true) );
	//GLGaeaServer::GetInstance().ReserveDropOutSummon ( SDROPOUTSUMMONINFO(m_dwSummonGUID,true) );
	GLGaeaServer::GetInstance().SaveVehicle( m_dwClientID, m_dwGaeaID, true );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		GLGaeaServer::GetInstance().ReserveDropOutSummon ( m_dwSummonGUID_FLD[i] );

	//	Note : �ٸ� �ʵ� ������ ���.
	if ( pMapNode->dwFieldSID!=GLGaeaServer::GetInstance().GetFieldSvrID() )
	{
		GLMSG::SNETPC_REQ_RECALL_AG NetMsgAg;
		NetMsgAg.sMAPID = sMAPID;
		NetMsgAg.dwGATEID = dwGATEID;
		NetMsgAg.vPOS = vPOS;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgAg );
	}
	//	Note : ���� �ʵ� ������ ���.
	else
	{
		GLMSG::SNETPC_REQ_RECALL_FB	NetMsgFB;
		SNATIVEID sCurMapID = m_sMapID;

		BOOL bOK = GLGaeaServer::GetInstance().RequestInvenRecallThisSvr ( this, sMAPID, dwGATEID, vPOS );
		if ( !bOK )
		{
			NetMsgFB.emFB = EMREQ_RECALL_FB_FAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
			return FALSE;
		}

		// ������ �����̻� ����
		if ( sCurMapID != sMAPID )
		{
			for ( int i=0; i<EMBLOW_MULTI; ++i )		DISABLEBLOW ( i );
			for ( int i=0; i<SKILLFACT_SIZE; ++i )		DISABLESKEFF ( i );
			DISABLEALLLANDEFF();
		}

		ResetAction();

		//	Note : �� �̵� ������ �˸�.
		//
		NetMsgFB.emFB = EMREQ_RECALL_FB_OK;
		NetMsgFB.sMAPID = sMAPID;
		NetMsgFB.vPOS = m_vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgFB );
	}

	return S_OK;
}

inline HRESULT GLChar::MsgReqMove2CharFld ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_GM_MOVE2CHAR_FLD *pNetMsg = (GLMSG::SNETPC_GM_MOVE2CHAR_FLD *)nmg;
	GLMSG::SNETPC_GM_MOVE2CHAR_FB MsgFB;

	//	�ɸ��� ���� ���°� �ƴ� ���.
	if ( !IsValidBody() )
	{
		MsgFB.emFB = EMGM_MOVE2CHAR_FB_MY_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	//	��� ������ ���.
	if ( m_sCONFTING.IsCONFRONTING() )
	{
		MsgFB.emFB = EMGM_MOVE2CHAR_FB_MY_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	//	�ŷ����� ���.
	if ( m_sTrade.Valid() )
	{
		MsgFB.emFB = EMGM_MOVE2CHAR_FB_MY_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	SMAPNODE *pMapNode = GLGaeaServer::GetInstance().FindMapNode ( pNetMsg->sToMapID );
	if ( !pMapNode )
	{
		MsgFB.emFB = EMGM_MOVE2CHAR_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	SNATIVEID sMAPID = pNetMsg->sToMapID;
	DWORD dwGATEID(UINT_MAX);
	D3DXVECTOR3 vPOS = pNetMsg->vToPos;

	// PET
	// ���̵��� Pet ����
	GLGaeaServer::GetInstance().ReserveDropOutPet ( SDROPOUTPETINFO(m_dwPetGUID,false,true) );
	//GLGaeaServer::GetInstance().ReserveDropOutSummon ( SDROPOUTSUMMONINFO(m_dwSummonGUID,true) );
	GLGaeaServer::GetInstance().SetActiveVehicle( m_dwClientID, m_dwGaeaID, false );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		GLGaeaServer::GetInstance().ReserveDropOutSummon ( m_dwSummonGUID_FLD[i] );

	//	Note : �ٸ� �ʵ� ������ ���.
	if ( pMapNode->dwFieldSID!=GLGaeaServer::GetInstance().GetFieldSvrID() )
	{
		GLMSG::SNETPC_REQ_RECALL_AG NetMsgAg;
		NetMsgAg.sMAPID = sMAPID;
		NetMsgAg.dwGATEID = dwGATEID;
		NetMsgAg.vPOS = vPOS;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgAg );
	}
	//	Note : ���� �ʵ� ������ ���.
	else
	{
		GLMSG::SNETPC_REQ_RECALL_FB	NetMsgFB;
		SNATIVEID sCurMapID = m_sMapID;

		BOOL bOK = GLGaeaServer::GetInstance().RequestInvenRecallThisSvr ( this, sMAPID, dwGATEID, vPOS );
		if ( !bOK )
		{
			NetMsgFB.emFB = EMREQ_RECALL_FB_FAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
			return FALSE;
		}

		// ������ �����̻� ����
		if ( sCurMapID != sMAPID )
		{
			for ( int i=0; i<EMBLOW_MULTI; ++i )		DISABLEBLOW ( i );
			for ( int i=0; i<SKILLFACT_SIZE; ++i )		DISABLESKEFF ( i );
			DISABLEALLLANDEFF();
		}

		ResetAction();

		//	Note : �� �̵� ������ �˸�.
		//
		NetMsgFB.emFB = EMREQ_RECALL_FB_OK;
		NetMsgFB.sMAPID = sMAPID;
		NetMsgFB.vPOS = m_vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgFB );
	}

	return S_OK;
}

inline HRESULT GLChar::MsgReqLevelUp ( NET_MSG_GENERIC* nmg )
{
	if( GLGaeaServer::GetInstance().m_bEmptyMsg ) return S_OK;
	if ( VALID_LEVELUP() )
	{

		//	����� ó��.
		LEVLEUP ( !m_bVietnamLevelUp );

		//	Note : ������ ���� ����Ʈ �ڵ� ����.
		//
		QuestStartFromGetLEVEL ( m_wLevel );

		//	Note : ����Ʈ ���� ���� ���� ����.
		//
		DoQuestLevel ();

		/*activity system, Juver, 2017/10/30 */
		DoActivityLevel();

		//	[�ڽſ���]
		GLMSG::SNETPC_REQ_LEVELUP_FB NetMsg;
		NetMsg.wLevel		 = m_wLevel;
		NetMsg.wStatsPoint	 = m_wStatsPoint;
		NetMsg.dwSkillPoint  = m_dwSkillPoint;
		NetMsg.bInitNowLevel = !m_bVietnamLevelUp;
		NetMsg.sMapID		 = m_sMapID;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg);

		//	[��ο���]
		GLMSG::SNETPC_REQ_LEVELUP_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID = m_dwGaeaID;
		SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&NetMsgBrd) );
	
		//	[��Ƽ,�����] hp ��ȭ �˸�.
		MsgSendUpdateState();


		//	Note : ������ �α� ���.(���� 20�̻󿡼�)
		if( m_wLevel > 20 )
		{
			GLITEMLMT::GetInstance().ReqAction( m_dwCharID,				//	�����.
												EMLOGACT_LEV_UP,		//	����.
												ID_CHAR, 0,				//	����.
												m_sExperience.lnNow,	//	exp
												0,						//	bright
												0,						//	life
												0 );					//	money
		}

		if( m_CHARINDEX == GLCI_EXTREME_M || m_CHARINDEX == GLCI_EXTREME_W )
		{
			if ( m_wLevel == GLCONST_CHAR::wMAX_EXTREME_LEVEL )
			{
				std::string strTIME = CDebugSet::GetCurTime();
				CDebugSet::ToFile("MaxLevelLogsExtreme.txt", "%s	userid= %s (%d), charid= %s (%d), Level: %d",
					strTIME.c_str(), m_szUID, GetUserID(), m_szName, GetCharID(), m_wLevel);
				strTIME.clear();
			}
		}
		else
		{
			if ( m_wLevel == GLCONST_CHAR::wMAX_LEVEL)
			{
				std::string strTIME = CDebugSet::GetCurTime();
				CDebugSet::ToFile("MaxLevelLogs.txt", "%s	userid= %s (%d), charid= %s (%d), Level: %d",
					strTIME.c_str(), m_szUID, GetUserID(), m_szName, GetCharID(), m_wLevel);
				strTIME.clear();
			}
		}

#if defined(_RELEASED) || defined(TW_PARAM) || defined(TH_PARAM) || defined(HK_PARAM) // ***Tracing Log print
		if( m_bTracingUser )
		{
			NET_LOG_UPDATE_TRACINGCHAR TracingMsg;
			TracingMsg.nUserNum  = GetUserID();
			StringCchCopy( TracingMsg.szAccount, USR_ID_LENGTH+1, m_szUID );

			CString strTemp;
			strTemp.Format( "LevelUp!!, [%s][%s], Level[%d]", m_szUID, m_szName, m_wLevel );

			StringCchCopy( TracingMsg.szLogMsg, TRACING_LOG_SIZE, strTemp.GetString() );

			m_pGLGaeaServer->SENDTOAGENT( m_dwClientID, &TracingMsg );
		}
#endif

		//** Add EventTime

		SEventState sEventState = GLGaeaServer::GetInstance().m_sEventState;
		if( sEventState.bEventStart )
		{
			if( m_bEventApply == TRUE )
			{
				if( sEventState.MinEventLevel > GETLEVEL() || sEventState.MaxEventLevel < GETLEVEL() )
				{
					m_bEventApply = FALSE;
				}
			}else{
				if( sEventState.MinEventLevel <= GETLEVEL() && sEventState.MaxEventLevel >= GETLEVEL() )
				{
					CTime	  crtTime     = CTime::GetCurrentTime();
					m_sEventTime.loginTime = crtTime.GetTime();

					GLMSG::SNET_GM_LIMIT_EVENT_RESTART NetMsg;
					NetMsg.restartTime = crtTime.GetTime();
					GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID,&NetMsg);
					m_bEventApply = TRUE;
				}
			}
		}

		if( m_bVietnamLevelUp )
		{
			if ( m_sExperience.ISOVER() )
			{
				//	Note : ���� ����. ( ���� �޽��� �߻�. )
				//
				GLMSG::SNETPC_REQ_LEVELUP NetMsg;
				MsgReqLevelUp ( (NET_MSG_GENERIC*) &NetMsg );

				m_lnLastSendExp = m_sExperience.lnNow;
			}
		}

		
	}else{
		if( m_CHARINDEX == GLCI_EXTREME_M || m_CHARINDEX == GLCI_EXTREME_W )
		{
			if ( m_wLevel > GLCONST_CHAR::wMAX_EXTREME_LEVEL )
			{
				HACKINGLOG_WRITE( "Level up Failed!!, Account[%s], ID[%s], Level %d, NowExp %d, MaxExp %d", 
					m_szUID, m_szName, m_wLevel, m_sExperience.lnNow, m_sExperience.lnMax );	

				m_sExperience.lnMax = GLOGICEX::GLNEEDEXP2(GLCONST_CHAR::wMAX_EXTREME_LEVEL);
			}
		}else{
			if ( m_wLevel > GLCONST_CHAR::wMAX_LEVEL )	
			{
				HACKINGLOG_WRITE( "Level up Failed!!, Account[%s], ID[%s], Level %d, NowExp %d, MaxExp %d", 
					m_szUID, m_szName, m_wLevel, m_sExperience.lnNow, m_sExperience.lnMax );

				m_sExperience.lnMax = GLOGICEX::GLNEEDEXP(GLCONST_CHAR::wMAX_LEVEL);
			}
		}
		return E_FAIL;
	}

	m_bVietnamLevelUp = FALSE;

	return S_OK;
}

inline HRESULT GLChar::MsgReqStatsUp ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_STATSUP *pNetMsg = (GLMSG::SNETPC_REQ_STATSUP *) nmg;
	if ( m_wStatsPoint == 0 )	return S_FALSE;
	if ( pNetMsg->sStats.GetTotal() >  m_wStatsPoint )	return S_FALSE;

	//	������ STATE�� ������Ŵ.
	STATSUP(pNetMsg->sStats);

	//	ó�� ��� ����.
	GLMSG::SNETPC_REQ_STATSUP_FB NetMsg;
	NetMsg.sStats = pNetMsg->sStats;
	NetMsg.wStatsPoint = m_wStatsPoint;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg);

	//	[��Ƽ,�����] hp ��ȭ �˸�.
	MsgSendUpdateState();

	return S_OK;
}

/*dmk14 remove active buff selection*/
inline HRESULT GLChar::MsgReqDisableSkillEff ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_DISABLESKILLEFF *pNetMsg = (GLMSG::SNETPC_REQ_DISABLESKILLEFF *) nmg;
	if ( pNetMsg->dwSKILL > SKILLFACT_SIZE ) return S_FALSE;

	DISABLESKEFF(pNetMsg->dwSKILL);

	GLMSG::SNETPC_REQ_DISABLESKILLEFF_FB NetMsg;
	NetMsg.dwSKILL = pNetMsg->dwSKILL;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg);

	GLMSG::SNETPC_REQ_DISABLESKILLEFF_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	NetMsgBrd.dwSKILL = pNetMsg->dwSKILL;
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

	MsgSendUpdateState();

	return S_OK;
}

inline HRESULT GLChar::MsgReqDisableItemFact ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_DISABLEITEMFACT *pNetMsg = (GLMSG::SNETPC_REQ_DISABLEITEMFACT *) nmg;
	if ( pNetMsg->dwSKILL > FITEMFACT_SIZE ) return S_FALSE;


	m_sFITEMFACT[pNetMsg->dwSKILL].RESET();

	GLMSG::SNETPC_REQ_DISABLEITEMFACT_FB NetMsg;
	NetMsg.dwSKILL = pNetMsg->dwSKILL;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg);

	GLMSG::SNETPC_REQ_DISABLEITEMFACT_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	NetMsgBrd.dwSKILL = pNetMsg->dwSKILL;
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

	MsgSendUpdateState();

	return S_OK;
}

inline HRESULT GLChar::MsgReqActionQSet ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_ACTIONQUICK_SET *pNetMsg = (GLMSG::SNETPC_REQ_ACTIONQUICK_SET *) nmg;

	WORD wSLOT = pNetMsg->wSLOT;
	if ( EMACTIONQUICK_SIZE <= wSLOT )		return E_FAIL;
	if ( !VALID_HOLD_ITEM() )				return S_FALSE;

	const SITEMCUSTOM& sITEM = GET_HOLD_ITEM ();

#if defined(VN_PARAM) //vietnamtest%%%
	if ( sITEM.bVietnamGainItem ) return E_FAIL;
#endif

	SITEM* pITEM = GLItemMan::GetInstance().GetItem ( sITEM.sNativeID );
	if ( !pITEM )							return S_FALSE;

	if ( pITEM->sBasicOp.emItemType != ITEM_CURE && 
		 pITEM->sBasicOp.emItemType != ITEM_RECALL &&
		 pITEM->sBasicOp.emItemType != ITEM_TELEPORT_CARD &&
		 pITEM->sBasicOp.emItemType != ITEM_PET_CARD )
		return S_FALSE;

	//	Note : ���� �ൿ ���� ����.
	//
	m_sACTIONQUICK[wSLOT].wACT = pNetMsg->wACT;
	m_sACTIONQUICK[wSLOT].sNID = sITEM.sNativeID;

	//	Note : Ŭ���̾�Ʈ�� ����.
	//
	GLMSG::SNETPC_REQ_ACTIONQUICK_FB NetMsgFB;
	NetMsgFB.wSLOT = wSLOT;
	NetMsgFB.sACT = m_sACTIONQUICK[wSLOT];
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

	//	Note : hold ������ �κ��� �ǵ���.
	WORD wPosX, wPosY;
	BOOL bOk = m_cInventory.FindInsrtable ( pITEM->sBasicOp.wInvenSizeX, pITEM->sBasicOp.wInvenSizeY, wPosX, wPosY );
	if ( !bOk )		return S_OK;	//	�κ� ���� �������� �ǵ��� ����.

	//	����ִ� ������ �ι꿡 ����.
	m_cInventory.InsertItem ( GET_HOLD_ITEM(), wPosX, wPosY );

	//	�տ��� ������ ����.
	RELEASE_HOLD_ITEM ();

	SINVENITEM *pInvenItem = m_cInventory.GetItem ( wPosX, wPosY );
	if ( !pInvenItem ) return E_FAIL;
	
	//	[�ڽſ���] �޽��� �߻�.
	GLMSG::SNETPC_INVEN_INSERT NetMsg_Inven;
	NetMsg_Inven.Data = *pInvenItem;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_Inven);

	//	[�ڽſ���] �� �־��� ������ ����.
	GLMSG::SNETPC_PUTON_RELEASE NetMsg_ReleaseHold(SLOT_HOLD);
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_ReleaseHold);

	return S_OK;
}

inline HRESULT GLChar::MsgReqActionQReSet ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_ACTIONQUICK_RESET *pNetMsg = (GLMSG::SNETPC_REQ_ACTIONQUICK_RESET *) nmg;

	WORD wSLOT = pNetMsg->wSLOT;
	if ( EMACTIONQUICK_SIZE <= wSLOT )		return E_FAIL;

	m_sACTIONQUICK[wSLOT].RESET ();

	//	Note : Ŭ���̾�Ʈ�� ����.
	//
	GLMSG::SNETPC_REQ_ACTIONQUICK_FB NetMsgFB;
	NetMsgFB.wSLOT = wSLOT;
	NetMsgFB.sACT = m_sACTIONQUICK[wSLOT];
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

	return S_OK;
}

// *****************************************************
// Desc: ��Ȱ��ġ��û ó��
// *****************************************************
HRESULT GLChar::MsgReqReGenGate ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_REGEN_GATE *pNetMsg = (GLMSG::SNETPC_REQ_REGEN_GATE *)nmg;

	GLMSG::SNETPC_REQ_REGEN_GATE_FB	MsgFB;
	MsgFB.emFB = EMREGEN_GATE_FAIL;

	PGLCROW pCrow = m_pLandMan->GetCrow ( pNetMsg->dwNpcID );
	if ( !pCrow )										goto _REQ_FAIL;
	if ( pCrow->GETCROW() != CROW_NPC )					goto _REQ_FAIL;

	GLMobSchedule* pMOBSCH = pCrow->GetMobSchedule();
	if ( !pMOBSCH )										goto _REQ_FAIL;

	if ( pMOBSCH->m_dwPC_REGEN_GATEID == UINT_MAX )		goto _REQ_FAIL;

	DxLandGateMan* pLandGateMan = &m_pLandMan->GetLandGateMan();
	PDXLANDGATE pLandGate = pLandGateMan->FindLandGate ( pMOBSCH->m_dwPC_REGEN_GATEID );
	if ( !pLandGate )									goto _REQ_FAIL;

	if ( !(pLandGate->GetFlags()&DxLandGate::GATE_IN) )	goto _REQ_FAIL;

	//	Note : ��Ȱ ��ġ ����.
	//
	m_sStartMapID = m_pLandMan->GetMapID();
	m_dwStartGate = pLandGate->GetGateID();

	//	Note : ��Ȱ ��ġ ������ �����Ͽ����� �˸�.
	//
	MsgFB.emFB = EMREGEN_GATE_SUCCEED;
	MsgFB.sMapID = m_sStartMapID;
	MsgFB.dwGateID = m_dwStartGate;
	GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID,&MsgFB);

	return S_OK;

_REQ_FAIL:
	//	Note : ��Ȱ ��ġ ������ �����Ͽ����� �˸�.
	//
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
	return E_FAIL;
}

HRESULT GLChar::MsgReqCure ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_CURE *pNetMsg = (GLMSG::SNETPC_REQ_CURE *)nmg;

	GLMSG::SNETPC_REQ_CURE_FB	MsgFB;
	MsgFB.emFB = EMREGEN_CURE_FAIL;

	PGLCROW pCrow = m_pLandMan->GetCrow ( pNetMsg->dwNpcID );
	
	if ( !pCrow )										goto _REQ_FAIL;
	if ( pCrow->GETCROW() != CROW_NPC )					goto _REQ_FAIL;

	//	Note : ġ�ᰡ���� NPC���� �˻� �ʿ�.
	//

	bool bCURE = pCrow->m_pCrowData->m_sNpcTalkDlg.IsCURE();
	if ( !bCURE )										goto _REQ_FAIL;

	// ġ�ᰡ���� �������� üũ
	SNpcTalk* pTalk = pCrow->m_pCrowData->m_sNpcTalkDlg.GetTalk( pNetMsg->dwGlobalID );
	if ( !pTalk )										goto _REQ_FAIL;

    GLCHARLOGIC* pCharLogic = dynamic_cast<GLCHARLOGIC*>(this);
	if ( pCharLogic )
	{
		if ( !(pTalk->DoTEST ( pCharLogic )) )
		{
			goto _REQ_FAIL;
		}
	}

	// �Ÿ� ���� �׽�Ʈ
	float fDist = D3DXVec3Length ( &D3DXVECTOR3(m_vPos-pCrow->GetPosition()) );
	float fTalkRange = (float) (pCrow->GetBodyRadius() + GETBODYRADIUS() + 30);
	float fTalkableDis = fTalkRange + 20;

	if ( fDist>fTalkableDis )							goto _REQ_FAIL;

	//	Note : ġ���Ŵ.
	//
	m_sHP.TO_FULL();
	m_sMP.TO_FULL();
	m_sSP.TO_FULL();
	CURE_STATEBLOW ( DIS_ALL );

	//	Note : ��� ����.
	//
	MsgFB.emFB = EMREGEN_CURE_SUCCEED;
	MsgFB.sHP = m_sHP;
	MsgFB.sMP = m_sMP;
	MsgFB.sSP = m_sSP;
	MsgFB.dwCUREFLAG = DIS_ALL;

	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	return S_OK;

_REQ_FAIL:
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	return S_OK;
}

HRESULT GLChar::MsgReqCharReset ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_CHARRESET *pNetMsg = (GLMSG::SNETPC_REQ_CHARRESET *)nmg;

	GLMSG::SNETPC_REQ_CHARRESET_FB	MsgFB;
	MsgFB.emFB = EMREGEN_CHARRESET_FAIL;

	GLMSG::SNETPC_INVEN_DELETE NetMsg_Inven_Delete;


	PGLCROW pCrow = m_pLandMan->GetCrow ( pNetMsg->dwNpcID );

	if ( !pCrow )										goto _REQ_FAIL;
	if ( pCrow->GETCROW() != CROW_NPC )					goto _REQ_FAIL;

	//	Note : ġ�ᰡ���� NPC���� �˻� �ʿ�.
	//
	bool bRESET = pCrow->m_pCrowData->m_sNpcTalkDlg.IsCHARRESET();
	if ( !bRESET )										goto _REQ_FAIL;

	//	Note : char reset ������ ��.
	//
	WORD wPosX(0), wPosY(0);
	bool bITEM = m_cInventory.GetCharResetItem ( wPosX, wPosY );
	if ( !bITEM )
	{
		MsgFB.emFB = EMREGEN_CHARRESET_ITEM_FAIL;
		goto _REQ_FAIL;
	}

	m_cInventory.DeleteItem ( wPosX, wPosY );

	//	[�ڽſ���] �ι꿡 ������ ����.
	NetMsg_Inven_Delete.wPosX = wPosX;
	NetMsg_Inven_Delete.wPosY = wPosY;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_Inven_Delete);


	//	Note : ���� ����. ( stats, skill )
	//
	RESET_STATS_SKILL();

	//	Note : ���� SKILL ������̶�� ��� ����.
	//
	if ( IsACTION(GLAT_SKILL) )
	{
		GLMSG::SNETPC_SKILL_CANCEL_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID = m_dwGaeaID;

		SendMsgViewAround ( (NET_MSG_GENERIC *)&NetMsgBrd );

		TurnAction(GLAT_IDLE);
	}

	//	Note : ��� ����.
	//
	MsgFB.emFB = EMREGEN_CHARRESET_SUCCEED;
	MsgFB.dwSKILL_P = m_dwSkillPoint;
	MsgFB.dwSTATS_P = m_wStatsPoint;

	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	return S_OK;

_REQ_FAIL:

	//	Note : ���� ���� �����Ͽ����� �˸�.
	//
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	return S_OK;
}

void GLChar::DoConftStateBackup ()
{
	//	Note : ü�� ������ ��ȭ.
	//
	m_sCONFTING.sBACKUP_HP = m_sHP;
	m_sCONFTING.sBACKUP_MP = m_sMP;
	m_sCONFTING.sBACKUP_SP = m_sSP;

	if ( !m_mapPlayHostile.empty() )
	{
		m_sCONFTING.sOption.fHP_RATE = 1.0f;
	}

	//	Note : ü���� �ִ� ��ġ ���.
	UPDATE_MAX_POINT ( m_sCONFTING.sOption.fHP_RATE );

	//	Note : ��ÿ� ���� ���� ����.
	m_sHP.TO_FULL();
	m_sMP.TO_FULL();
	m_sSP.TO_FULL();

	//	Note : ü�� ��ȭ�� �˸�. [�ڽ�,��Ƽ,�����]
	//
	MsgSendUpdateState();
}

void GLChar::DoConftStateRestore ()
{
	if ( !m_sCONFTING.IsCONFRONTING() )		return;

	//	Note : ü�� ȯ��.
	//
	m_sHP = m_sCONFTING.sBACKUP_HP;
	m_sMP = m_sCONFTING.sBACKUP_MP;
	m_sSP = m_sCONFTING.sBACKUP_SP;

	//	Note : ü���� �ִ� ��ġ ���.
	UPDATE_MAX_POINT ( 1.0f );

	m_sHP.LIMIT();
	m_sMP.LIMIT();
	m_sSP.LIMIT();

	//	Note : ü�� ��ȭ�� �˸�. [�ڽ�,��Ƽ,�����]
	//
	MsgSendUpdateState();

	//	Note : �ൿ�� ����Ѵ�.
	//
	TurnAction(GLAT_IDLE);
}

HRESULT GLChar::MsgReqConFrontStart ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_CONFRONT_START2_FLD *pNetMsg = (GLMSG::SNETPC_CONFRONT_START2_FLD *)nmg;
	
	m_sCONFTING.RESET();

	m_cDamageLog.clear ();

	PGLCHAR pTAR = GLGaeaServer::GetInstance().GetChar(pNetMsg->dwTARID);
	if ( !pTAR )
	{
		GLMSG::SNETPC_REQ_CONFRONT_FB NetMsgFB;
		NetMsgFB.emTYPE = EMCONFT_ONE;
		NetMsgFB.dwID = m_dwGaeaID;
		NetMsgFB.emFB = EMCONFRONT_FAIL;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgFB );

		return E_FAIL;
	}

	// ������ ���� ����
	if( pTAR->IsDie() )
	{
		GLMSG::SNETPC_REQ_CONFRONT_FB NetMsgFB;
		NetMsgFB.emTYPE = EMCONFT_ONE;
		NetMsgFB.dwID = m_dwGaeaID;
		NetMsgFB.emFB = EMCONFRONT_DIE_YOU;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgFB );

		return E_FAIL;
	}

	// ���� ���� ����
	if( IsDie() )
	{
		GLMSG::SNETPC_REQ_CONFRONT_FB NetMsgFB;
		NetMsgFB.emTYPE = EMCONFT_ONE;
		NetMsgFB.dwID = m_dwGaeaID;
		NetMsgFB.emFB = EMCONFRONT_DIE_ME;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgFB );

		return E_FAIL;
	}

	D3DXVECTOR3 vPosition = ( m_vPos + pTAR->m_vPos ) / 2.0f;

	//	Note : ���������� ����.
	//
	DelPlayHostile();

	//	Note : ��� ���� ����.
	//
	m_sCONFTING.emTYPE = pNetMsg->emTYPE;
	m_sCONFTING.dwTAR_ID = pNetMsg->dwTARID;
	m_sCONFTING.vPosition = vPosition;
	m_sCONFTING.sOption = pNetMsg->sOption;

	//	Note : ��� ���� Ŭ���̾�Ʈ�� �˸�.
	GLMSG::SNETPC_CONFRONT_START2_CLT NetMsgClt;
	NetMsgClt.emTYPE = pNetMsg->emTYPE;
	NetMsgClt.dwTARID = pNetMsg->dwTARID;
	NetMsgClt.vPosition = vPosition;
	NetMsgClt.sOption = pNetMsg->sOption;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgClt);

	// ��� ���۽� ��ȯ�� ����
	//GLGaeaServer::GetInstance().DropOutSummon ( m_dwSummonGUID,true );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		GLGaeaServer::GetInstance().DropOutSummon ( m_dwSummonGUID_FLD[i] );

	//	Note : ü�� ���.
	//
	DoConftStateBackup();

	return S_OK;
}

HRESULT GLChar::MsgReqConFrontEnd ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_CONFRONT_END2_FLD *pNetMsg = (GLMSG::SNETPC_CONFRONT_END2_FLD *)nmg;

	EMCONFRONT_END emEND = pNetMsg->emEND;

	//	Note : ü�� ȯ��.
	//
	DoConftStateRestore();

	//	Note : Ŭ���̾�Ʈ�� ��� ��Ҹ� �뺸.
	GLMSG::SNETPC_CONFRONT_END2_CLT NetMsgClt;
	NetMsgClt.emEND = emEND;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgClt);

	//	Note : ��� ���� �޽���.
	GLMSG::SNETPC_CONFRONT_END2_CLT_BRD NetMsgTarBrd;
	NetMsgTarBrd.dwGaeaID = m_dwGaeaID;
	NetMsgTarBrd.emEND = emEND;

	switch ( m_sCONFTING.emTYPE )
	{
	case EMCONFT_PARTY:
		{
			//	��Ƽ�� �ִ� '���' ���������� ����.
			GLPARTY_FIELD *pParty = GLGaeaServer::GetInstance().GetParty(m_dwPartyID);
			if ( pParty )
			{
				//	��� ����Ʈ���� ����.
				pParty->MBR_CONFRONT_LEAVE(m_dwGaeaID);

				//	��Ƽ������ ��ÿ��� ���� ������ �˸�.
				GLGaeaServer::GetInstance().SENDTOPARTYCLIENT ( m_dwPartyID, &NetMsgTarBrd );
			}
		}
		break;

	case EMCONFT_GUILD:
		{
			GLClubMan &cClubMan = GLGaeaServer::GetInstance().GetClubMan();
			GLCLUB *pCLUB = cClubMan.GetClub(m_dwGuild);
			if ( pCLUB )
			{
				pCLUB->DELCONFT_MEMBER ( m_dwCharID );

				//	Ŭ�������� ��ÿ��� ���������� �˸�.
				GLGaeaServer::GetInstance().SENDTOCLUBCLIENT ( m_dwGuild, &NetMsgTarBrd );
			}
		}
		break;
	};

	//	Note : �ֺ� ���ֵ鿡 ������� �뺸.
	if ( emEND==EMCONFRONT_END_WIN || emEND==EMCONFRONT_END_LOSS )
	{
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgTarBrd );
	}


	// ������� ����

	m_sCONFTING.RESET();

	//	Note : ��� ����, ���� Ÿ�� ���� (�ڽ� & ��)
	m_sCONFTING.SETPOWERFULTIME ( 10.0f );

	//	Note : �ൿ�� ����Ѵ�.
	TurnAction ( GLAT_IDLE );

	return S_OK;
}

HRESULT GLChar::MsgActState ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_ACTSTATE *pNetMsg = (GLMSG::SNETPC_ACTSTATE *)nmg;

	const DWORD dwOldActState = m_dwActState;

	// ��Ʋ�ξ� �������� ���������� Visible ��� ���������ϵ��� üũ�׸� �߰�.

	if ( m_dwUserLvl >= USER_GM3 || GLCONST_CHAR::bBATTLEROYAL )
	{
		if ( pNetMsg->dwActState & EM_REQ_VISIBLENONE )	SetSTATE ( EM_REQ_VISIBLENONE );
		else											ReSetSTATE ( EM_REQ_VISIBLENONE );
		
		if ( pNetMsg->dwActState & EM_REQ_VISIBLEOFF )	SetSTATE ( EM_REQ_VISIBLEOFF );
		else											ReSetSTATE ( EM_REQ_VISIBLEOFF );
	}

	if ( pNetMsg->dwActState & EM_ACT_PEACEMODE )	SetSTATE ( EM_ACT_PEACEMODE );
	else											ReSetSTATE ( EM_ACT_PEACEMODE );

	/*vehicle booster system, Juver, 2017/08/12 */
	if ( pNetMsg->dwActState & EM_ACT_VEHICLE_BOOSTER )	
		SetSTATE ( EM_ACT_VEHICLE_BOOSTER );
	else										
		ReSetSTATE ( EM_ACT_VEHICLE_BOOSTER );

	if ( dwOldActState != m_dwActState || GLCONST_CHAR::bBATTLEROYAL )
	{
		GLMSG::SNETPC_MOVESTATE_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID = m_dwGaeaID;
		NetMsgBrd.dwActState = m_dwActState;
		
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
	}

	return S_OK;
}

HRESULT GLChar::MsgReqGesture ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_GESTURE *pNetMsg = (GLMSG::SNETPC_REQ_GESTURE *)nmg;

	// ������ �׼�
	TurnAction ( GLAT_TALK );
	m_dwANISUBSELECT = pNetMsg->dwID;

	//	Note : ������ ������ ��� �ֺ� ����鿡�� ����.
	GLMSG::SNETPC_REQ_GESTURE_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	NetMsgBrd.dwID = pNetMsg->dwID;

	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////
// ��������
// 2005-12-27 Jgkim
// �߱�, ���������ƿ��� NpcTalk ������ �����Ͽ� �ҹ����������� ��ȯ����	
//
// pNetMsg->dwA_NID; // A:npc���� �� ������
// pNetMsg->dwB_NID; // b:npc���� ���� ������
// pNetMsg->dwNpcID; // ��û�� �޴� NPC ID
//
// 1. ��û�� NPC �� �ִ��� Ȯ��
// 2. A �������� ĳ���Ͱ� ������ �ִ��� Ȯ��
// 3. NPC �� B �������� �� �� �ִ��� Ȯ��
// 4. NPC �� B �������� �� �� �ִٸ�, ĳ������ A �������� �Ҹ��� B �������� ����
///////////////////////////////////////////////////////////////////////////
// PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData(pNetMsg->dwNpcID);
// pCrowData->m_sNpcTalkDlg
///////////////////////////////////////////////////////////////////////////
HRESULT GLChar::MsgReqNpcItemTrade ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_NPC_ITEM_TRADE *pNetMsg = (GLMSG::SNETPC_REQ_NPC_ITEM_TRADE *) nmg;

	GLMSG::SNETPC_INVEN_DELETE NetMsgDelete;
	GLMSG::SNETPC_INVEN_INSERT NetMsgInsert;

	GLMSG::SNETPC_REQ_NPC_ITEM_TRADE_FB	MsgFB;
	MsgFB.emFB = EMNPC_ITEM_TRADE_FAIL;


	PGLCROW pCrow = m_pLandMan->GetCrow ( pNetMsg->dwNpcID );

	WORD wPosX[] = { 0, 0, 0, 0, 0 };
	WORD wPosY[] = { 0, 0, 0, 0, 0 };
	WORD wInsertX(0), wInsertY(0);

	BOOL bITEM[] = { FALSE, FALSE, FALSE, FALSE, FALSE };
	SINVENITEM *pDEL_ITEM(NULL);
	SITEMCUSTOM sDEL_CUSTOM;

	BOOL bOK(FALSE);
	SITEMCUSTOM sITEM_NEW;
	CTime cTIME;

	SITEM *pITEM = GLItemMan::GetInstance().GetItem ( SNATIVEID(pNetMsg->dwB_NID) );
	if ( !pITEM )										goto _REQ_FAIL;

	if ( !pCrow )										goto _REQ_FAIL;
	if ( pCrow->GETCROW() != CROW_NPC )					goto _REQ_FAIL;

	SNpcTalk* pTalk = pCrow->m_pCrowData->m_sNpcTalkDlg.GetTalk( pNetMsg->dwGlobalID );

	if ( !pTalk )										goto _REQ_FAIL;

	// Need Add New Item Con.

	// NPC�� �̿��� ������ ��ȯ�� ��ȿ�� üũ
//	if ( pTalk->m_dwACTION_PARAM1 != pNetMsg->dwA_NID 
//		|| pTalk->m_dwACTION_PARAM2 != pNetMsg->dwB_NID ) goto _REQ_FAIL;

	//jdevtodo
	//pending num logic and vector logic
	if ( pTalk->m_sNeedItem[0].dwItemID != pNetMsg->dwA_NID[0] ||
		 pTalk->m_sNeedItem[1].dwItemID != pNetMsg->dwA_NID[1] ||
		 pTalk->m_sNeedItem[2].dwItemID != pNetMsg->dwA_NID[2] ||
		 pTalk->m_sNeedItem[3].dwItemID != pNetMsg->dwA_NID[3] ||
		 pTalk->m_sNeedItem[4].dwItemID != pNetMsg->dwA_NID[4] ||
		 pTalk->m_dwACTION_PARAM2 != pNetMsg->dwB_NID ) goto _REQ_FAIL;

	//	Note : char reset ������ ��.
	//
	bITEM[0] = m_cInventory.GetPileItem ( SNATIVEID(pNetMsg->dwA_NID[0]), wPosX[0], wPosY[0] );
	BYTE i;
	// MAX_NEEDITEM_COUNT 5
	for( i = 1; i < MAX_NEEDITEM_COUNT; i++ )
	{
		if( pNetMsg->dwA_NID[i] == UINT_MAX )
			bITEM[i] = TRUE;
		else
			bITEM[i] = m_cInventory.GetPileItem ( SNATIVEID(pNetMsg->dwA_NID[i]), wPosX[i], wPosY[i] );
	}
	if( bITEM[0] == FALSE || bITEM[1] == FALSE || bITEM[2] == FALSE || bITEM[3] == FALSE || bITEM[4] == FALSE )
	{
		MsgFB.emFB = EMNPC_ITEM_TRADE_ITEM_FAIL;
		goto _REQ_FAIL;
	}

	//	������ ������ ���.
	// MAX_NEEDITEM_COUNT 5
	for( i = 0; i < MAX_NEEDITEM_COUNT; i++ )
	{
		if( pNetMsg->dwA_NID[i] == UINT_MAX ) continue;

		pDEL_ITEM = m_cInventory.GetItem ( wPosX[i], wPosY[i] );
		if ( !pDEL_ITEM )									goto _REQ_FAIL;
		sDEL_CUSTOM = pDEL_ITEM->sItemCustom;

		//	Note : ������ ������ ����.
		m_cInventory.DeleteItem ( wPosX[i], wPosY[i] );

		//	Note : ��ȯ�� ������ �־��� �����ִ��� �˻�.
		bOK = m_cInventory.FindInsrtable ( pITEM->sBasicOp.wInvenSizeX, pITEM->sBasicOp.wInvenSizeY, wInsertX, wInsertY );
		if ( !bOK )
		{
			//	Note : ��ȯ�� ������ �־��� ������ ������, ���ŵ� ������ �ٽ� ��ȯ.
			m_cInventory.InsertItem ( sDEL_CUSTOM, wPosX[i], wPosY[i] );

			MsgFB.emFB = EMNPC_ITEM_TRADE_INSERT_FAIL;
			goto _REQ_FAIL;
		}
	}

	//	Note :�������� ���� ���� ��� ���.
	//
	GLITEMLMT::GetInstance().ReqItemRoute ( sDEL_CUSTOM, ID_CHAR, m_dwCharID, ID_CHAR, 0, EMITEM_ROUTE_DELETE, sDEL_CUSTOM.wTurnNum );

	//	Note : ���ο� ������ ����.
	//
	sITEM_NEW.sNativeID = SNATIVEID(pNetMsg->dwB_NID);
	cTIME = CTime::GetCurrentTime();
	sITEM_NEW.tBORNTIME = cTIME.GetTime();

	sITEM_NEW.wTurnNum = 1;
	sITEM_NEW.cGenType = EMGEN_NPC;
	sITEM_NEW.cChnID = (BYTE)GLGaeaServer::GetInstance().GetServerChannel();
	sITEM_NEW.cFieldID = (BYTE)GLGaeaServer::GetInstance().GetFieldSvrID();
	sITEM_NEW.lnGenNum = GLITEMLMT::GetInstance().RegItemGen ( sITEM_NEW.sNativeID, EMGEN_NPC );

	/*item color, Juver, 2018/01/08 */
	SITEM* pitem_data_check = GLItemMan::GetInstance().GetItem( sITEM_NEW.sNativeID );
	if ( pitem_data_check && pitem_data_check->sBasicOp.bItemColor )
	{
		sITEM_NEW.wColor1 = pitem_data_check->sBasicOp.wItemColor1;
		sITEM_NEW.wColor2 = pitem_data_check->sBasicOp.wItemColor2;
	}

	// ������ ����Ʈ���� �Է��� ���� ��� ���� ( ���� : NpcItem, ������ ���� �ƴѵ� )
	//sITEM_NEW.cDAMAGE = pITEM->sBasicOp.wGradeAttack;
	//sITEM_NEW.cDEFENSE = pITEM->sBasicOp.wGradeDefense;

	//	���� �ɼ� ����.
	if( sITEM_NEW.GENERATE_RANDOM_OPT() )
	{
		GLITEMLMT::GetInstance().ReqRandomItem( sITEM_NEW );
	}

	m_cInventory.InsertItem ( sITEM_NEW, wInsertX, wInsertY );
	SINVENITEM *pINVENITEM = m_cInventory.GetItem ( wInsertX, wInsertY );
	if ( !pINVENITEM ) return E_FAIL;

	//	Note :�������� ���� ���� ��� ���.
	//
	GLITEMLMT::GetInstance().ReqItemRoute ( sITEM_NEW, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, sITEM_NEW.wTurnNum );

	//	Note : ������ ����� �ڵ� ����Ʈ ���� ����.
	//
	QuestStartFromGetITEM ( sITEM_NEW.sNativeID );



	//	[�ڽſ���] �ι꿡 ������ ����, ��ȯ�� ������ �־���.
	for( i = 0; i < 5; i++ )
	{
		if( pNetMsg->dwA_NID[i] == UINT_MAX ) 
		{
			continue;
		}
		NetMsgDelete.wPosX = wPosX[i];
		NetMsgDelete.wPosY = wPosY[i];
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgDelete);
	}

	//	�ι꿡 ������ �־��ִ� �޽���.
	
	NetMsgInsert.Data = *pINVENITEM;
	NetMsgInsert.bAllLine = true;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgInsert);


	//	Note : ��� ����.
	//
	MsgFB.emFB = EMNPC_ITEM_TRADE_SUCCEED;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	return S_OK;

_REQ_FAIL:

	//	Note : ���� ���� �����Ͽ����� �˸�.
	//
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	return S_OK;
}

HRESULT GLChar::MsgReqPhoneNumber( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_PHONE_NUMBER *pNetMsg = (GLMSG::SNETPC_PHONE_NUMBER*) nmg;

	// DB�� �����û
	CSetChaPhoneNumber* pDBAction = new CSetChaPhoneNumber( m_dwClientID, m_dwCharID, pNetMsg->szPhoneNumber );

	if( GLGaeaServer::GetInstance().GetDBMan() )
	{
		GLGaeaServer::GetInstance().GetDBMan()->AddJob( pDBAction );
	}

	return S_OK;
}

// *****************************************************
// Desc: ����ȣ ���� ��� ó��
// *****************************************************
HRESULT GLChar::MsgPhoneNumber( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_PHONE_NUMBER_FROM_DB *pNetMsg = (GLMSG::SNETPC_PHONE_NUMBER_FROM_DB *)nmg;

	// Ŭ���̾�Ʈ�� ó�� ��� ����.
	GLMSG::SNETPC_PHONE_NUMBER_FB MsgFB;

	if ( pNetMsg->emFB == EMSMS_PHONE_NUMBER_FROM_DB_FAIL )
	{
		MsgFB.emFB = EMSMS_PHONE_NUMBER_FROM_DB_FAIL;
		StringCchCopy ( MsgFB.szPhoneNumber, SMS_RECEIVER, pNetMsg->szPhoneNumber );
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return S_FALSE;
	}

	StringCchCopy ( MsgFB.szPhoneNumber, SMS_RECEIVER, pNetMsg->szPhoneNumber );
	MsgFB.emFB = EMSMS_PHONE_NUMBER_FB_OK;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	// ����ȣ ������ ģ���ɹ��鿡�� �˸�
	GLMSG::SNETPC_PHONE_NUMBER_AGTBRD NetMsgAgt;
	NetMsgAgt.dwID = m_dwGaeaID;
	StringCchCopy ( NetMsgAgt.szName, CHAR_SZNAME, m_szName );
	StringCchCopy ( NetMsgAgt.szNewPhoneNumber, SMS_RECEIVER, pNetMsg->szPhoneNumber );
	GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgAgt );

	GLGaeaServer::GetInstance().ChangeNameMap( this, pNetMsg->szPhoneNumber );

	return S_OK;
}

HRESULT	GLChar::MsgPetReviveInfo ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_REQ_PETREVIVEINFO* pNetMsg = ( GLMSG::SNETPET_REQ_PETREVIVEINFO* ) nmg;
	CGetRestorePetList *pDbAction = new CGetRestorePetList ( m_dwCharID, m_dwClientID );
	GLDBMan* pDBMan = GLGaeaServer::GetInstance().GetDBMan ();
	if ( pDBMan ) pDBMan->AddJob ( pDbAction );

	return S_OK;
}

HRESULT GLChar::ActiveVehicle ( bool bActive, bool bMoveMap )
{
	/////////////////////////////////////////////////////////////////////////////
	//Fix Vehicle Card
	SITEM* pItem = GLItemMan::GetInstance().GetItem(m_PutOnItems[SLOT_VEHICLE].sNativeID);
	if (!pItem) return E_FAIL;

	if (IsCoolTime(m_PutOnItems[SLOT_VEHICLE].sNativeID))
		return E_FAIL;

	if (pItem->sBasicOp.IsCoolTime())
		SetCoolTime(m_PutOnItems[SLOT_VEHICLE].sNativeID, pItem->sBasicOp.emCoolType);
	/////////////////////////////////////////////////////////////////////////////
    // Ȱ��ȭ
	if ( bActive )
	{
		GLMSG::SNETPC_ACTIVE_VEHICLE_FB NetMsgFB;

		if ( m_bVehicle ) 
		{
			NetMsgFB.bActive = bActive;
			NetMsgFB.emFB = EMVEHICLE_SET_FB_RESET;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
			return E_FAIL;
		}

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

		/*vehicle booster system, Juver, 2017/08/11 */
		VehicleBoosterStateReset();

		if ( m_sVehicle.m_bBooster )
			VehicleBoosterStateCharge( true );

		/*
		// ������ų ����
		for ( int i = 0; i < SKILLFACT_SIZE; ++i )
		{
			if ( m_sSKILLFACT[i].sNATIVEID == NATIVEID_NULL() ) continue;
			
			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( m_sSKILLFACT[i].sNATIVEID );

			if ( pSkill && pSkill->m_sBASIC.emIMPACT_SIDE != SIDE_ENEMY )
			{
				DISABLESKEFF( i );
			}	
		}

		// ���ǹ��� ���� 
		m_sQITEMFACT.RESET();
		*/

		m_sPKCOMBOCOUNT.RESET();

		// Ż�� Ż�� ��ȯ�� ����
		//GLGaeaServer::GetInstance().DropOutSummon ( m_dwSummonGUID,true );

		/*skill summon, Juver, 2017/10/09 */
		for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
			GLGaeaServer::GetInstance().DropOutSummon ( m_dwSummonGUID_FLD[i] );

		GLCHARLOGIC::INIT_DATA ( FALSE, FALSE );
		ReSelectAnimation();

		NetMsgFB.bActive = true;
		NetMsgFB.emFB = EMVEHICLE_SET_FB_OK;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
		
		// �ֺ��� �˸�
		GLMSG::SNETPC_ACTIVE_VEHICLE_BRD NetMsgBRD;

		NetMsgBRD.bActive = true;
		NetMsgBRD.dwGaeaID = m_dwGaeaID;

		SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&NetMsgBRD) );

		
	}
	// ��Ȱ��ȭ
	else 
	{
		GLMSG::SNETPC_ACTIVE_VEHICLE_FB NetMsgFB;

		if ( !m_bVehicle ) 
		{
//			CDebugSet::ToLogFile ( "Current Active : %d, bActive : %d",m_bVehicle, bActive );
			NetMsgFB.bActive = bActive;
			NetMsgFB.emFB = EMVEHICLE_SET_FB_RESET;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
			return E_FAIL;
		}

		m_bVehicle = FALSE;		
		EMSLOT emRHand = GetCurRHand();
		EMSLOT emLHand = GetCurLHand();
		m_emANISUBTYPE = CHECK_ANISUB ( m_pITEMS[emRHand], m_pITEMS[emLHand]  );

		/*vehicle booster system, Juver, 2017/08/11 */
		VehicleBoosterStateReset();

		/*
        // ������ų ����
		for ( int i = 0; i < SKILLFACT_SIZE; ++i )
		{
			if ( m_sSKILLFACT[i].sNATIVEID == NATIVEID_NULL() ) continue;
			
			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( m_sSKILLFACT[i].sNATIVEID );

			if ( pSkill && pSkill->m_sBASIC.emIMPACT_SIDE != SIDE_ENEMY )
			{
				DISABLESKEFF( i );
			}	
		}

		// ���ǹ��� ���� 
		m_sQITEMFACT.RESET();
		*/

		m_sPKCOMBOCOUNT.RESET();
		
		GLCHARLOGIC::INIT_DATA ( FALSE, FALSE );
		ReSelectAnimation();

		NetMsgFB.bActive = false;
		NetMsgFB.emFB = EMVEHICLE_SET_FB_OK;
		NetMsgFB.bLeaveFieldServer = bMoveMap;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
		
		// �ֺ��� �˸�
		GLMSG::SNETPC_ACTIVE_VEHICLE_BRD NetMsgBRD;

		NetMsgBRD.bActive = false;
		NetMsgBRD.dwGaeaID = m_dwGaeaID;

		SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&NetMsgBRD) );

	}

	return S_OK;
}

HRESULT GLChar::MsgVehicleSlotExHold( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_VEHICLE_REQ_SLOT_EX_HOLD* pNetMsg = ( GLMSG::SNET_VEHICLE_REQ_SLOT_EX_HOLD* )nmg;
	GLMSG::SNET_VEHICLE_REQ_SLOT_EX_HOLD_FB NetMsgFB;

	/* vehicle no accessory, Juver, 2018/02/14 */
	SITEM* pitem_vehicle = GLItemMan::GetInstance().GetItem( m_sVehicle.m_sVehicleID );
	if ( pitem_vehicle && pitem_vehicle->sVehicle.bNoAcc )	return E_FAIL;

	//	Note : �������̳� ��ų ���� �߿� ���� ������ ���� �� �� ���ٰ� ��.

	SITEMCUSTOM sHoldItemCustom = GET_HOLD_ITEM ();
	SITEMCUSTOM	sSlotItemCustom = m_sVehicle.GetSlotitembySuittype ( pNetMsg->emSuit );

	SITEM* pHoldItem = GLItemMan::GetInstance().GetItem ( sHoldItemCustom.sNativeID );
	SITEM* pSlotItem = GLItemMan::GetInstance().GetItem ( sSlotItemCustom.sNativeID );

	if ( !pHoldItem || !pSlotItem ) 
	{
		// �Ϲݿ���
		NetMsgFB.emFB = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( pHoldItem->sBasicOp.emItemType != ITEM_SUIT )						return E_FAIL;

	if ( pHoldItem->sSuitOp.emSuit < SUIT_VEHICLE_SKIN && pHoldItem->sSuitOp.emSuit > SUIT_VEHICLE_PARTS_C )
	{
		// Ż�Ϳ� �������� �ƴҶ�
		NetMsgFB.emFB = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_INVALIDITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	// ���� �ٸ� Ÿ���϶�
	if ( pHoldItem->sSuitOp.emSuit != pSlotItem->sSuitOp.emSuit )
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_NOMATCH;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL; 
	}

	if ( !m_sVehicle.CheckSlotItem( pHoldItem->sBasicOp.sNativeID, pNetMsg->emSuit ) ) return E_FAIL;

	// �տ��� �������� Ż�Ϳ� ������Ű��
	m_sVehicle.SetSlotItem ( pNetMsg->emSuit, sHoldItemCustom );
	m_fVehicleSpeedRate = m_sVehicle.GetSpeedRate();
	m_fVehicleSpeedVol = m_sVehicle.GetSpeedVol();
	INIT_DATA( FALSE, FALSE );

	// �����ƴ� �������� �տ� ���
	HOLD_ITEM ( sSlotItemCustom );

	// �ڽſ��� �˸�
	NetMsgFB.emFB		 = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_OK;
	NetMsgFB.emSuit		 = pNetMsg->emSuit;
	NetMsgFB.sItemCustom = m_sVehicle.m_PutOnItems[pNetMsg->emSuit];
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);


	// �ٸ� Ŭ���̾�Ʈ ȭ�鿡 �������� �ٲ� ���尡 ��µǾ�� �Ѵ�
	GLMSG::SNET_VEHICLE_REQ_SLOT_EX_HOLD_BRD NetMsgBRD;
	NetMsgBRD.dwGaeaID	  = m_dwGaeaID;
	NetMsgBRD.emSuit	  = pNetMsg->emSuit;
	NetMsgBRD.sItemCustom = m_sVehicle.GetSlotitembySuittype ( pNetMsg->emSuit );
	SendMsgViewAround ( ( NET_MSG_GENERIC* )&NetMsgBRD );

	// Require DB����
	
	return S_OK;
}

HRESULT GLChar::MsgVehicleHoldToSlot( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_VEHICLE_REQ_HOLD_TO_SLOT* pNetMsg = ( GLMSG::SNET_VEHICLE_REQ_HOLD_TO_SLOT* )nmg;
	GLMSG::SNET_VEHICLE_REQ_HOLD_TO_SLOT_FB NetMsgFB;

	/* vehicle no accessory, Juver, 2018/02/14 */
	SITEM* pitem_vehicle = GLItemMan::GetInstance().GetItem( m_sVehicle.m_sVehicleID );
	if ( pitem_vehicle && pitem_vehicle->sVehicle.bNoAcc )	return E_FAIL;

	SITEMCUSTOM sHoldItemCustom = GET_HOLD_ITEM ();

	SITEM* pHoldItem = GLItemMan::GetInstance().GetItem ( sHoldItemCustom.sNativeID );

	if ( !pHoldItem ) 
	{
		// �Ϲݿ���
		NetMsgFB.emFB = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( pHoldItem->sBasicOp.emItemType != ITEM_SUIT )						return E_FAIL;

	if ( pHoldItem->sSuitOp.emSuit < SUIT_VEHICLE_SKIN && pHoldItem->sSuitOp.emSuit > SUIT_VEHICLE_PARTS_C )
	{
		// Ż�Ϳ� �������� �ƴҶ�
		NetMsgFB.emFB = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_INVALIDITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	// ���� �ٸ� Ÿ���϶�
	if ( pHoldItem->sSuitOp.emSuit != pNetMsg->emSuit )
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_NOMATCH;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL; 
	}

	if ( !m_sVehicle.CheckSlotItem( pHoldItem->sBasicOp.sNativeID, pNetMsg->emSuit ) ) return E_FAIL;

	// �տ��� �������� �ֿ��� ������Ű��
	m_sVehicle.SetSlotItem ( pNetMsg->emSuit, sHoldItemCustom );
	m_fVehicleSpeedRate = m_sVehicle.GetSpeedRate();
	m_fVehicleSpeedVol = m_sVehicle.GetSpeedVol();
	INIT_DATA( FALSE, FALSE );

	// �տ��� �������� �����ϰ�
	RELEASE_HOLD_ITEM ();

	// �ڽſ��� �˸�
	NetMsgFB.emFB		 = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_OK;
	NetMsgFB.emSuit		 = pNetMsg->emSuit;
	NetMsgFB.sItemCustom = m_sVehicle.GetSlotitembySuittype ( pNetMsg->emSuit );
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);

	// �ٸ� Ŭ���̾�Ʈ ȭ�鿡 �������� �ٲ� ���� ��µǾ�� �Ѵ�
	GLMSG::SNET_VEHICLE_REQ_SLOT_EX_HOLD_BRD NetMsgBRD;
	NetMsgBRD.dwGaeaID	  = m_dwGaeaID;
	NetMsgBRD.emSuit	  = pNetMsg->emSuit;
	NetMsgBRD.sItemCustom = m_sVehicle.GetSlotitembySuittype ( pNetMsg->emSuit );
	SendMsgViewAround ( ( NET_MSG_GENERIC* )&NetMsgBRD );

	// Require DB����
	
	return S_OK;
}

HRESULT GLChar::MsgVehicleSlotToHold( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_VEHICLE_REQ_SLOT_TO_HOLD* pNetMsg = ( GLMSG::SNET_VEHICLE_REQ_SLOT_TO_HOLD* )nmg;
	GLMSG::SNET_VEHICLE_REQ_SLOT_TO_HOLD_FB NetMsgFB;

	/* vehicle no accessory, Juver, 2018/02/14 */
	SITEM* pitem_vehicle = GLItemMan::GetInstance().GetItem( m_sVehicle.m_sVehicleID );
	if ( pitem_vehicle && pitem_vehicle->sVehicle.bNoAcc )	return E_FAIL;

	SITEMCUSTOM	sSlotItemCustom = m_sVehicle.GetSlotitembySuittype ( pNetMsg->emSuit );
	SITEM* pSlotItem = GLItemMan::GetInstance().GetItem ( sSlotItemCustom.sNativeID );

	if ( !pSlotItem ) 
	{
		// �Ϲݿ���
		NetMsgFB.emFB = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	// ������ �������� �տ� ���
	HOLD_ITEM ( sSlotItemCustom );

	// ������ �������� �����Ѵ�.
	m_sVehicle.ReSetSlotItem ( pNetMsg->emSuit );
	m_fVehicleSpeedRate = m_sVehicle.GetSpeedRate();
	m_fVehicleSpeedVol = m_sVehicle.GetSpeedVol();
	INIT_DATA( FALSE, FALSE );

	// �ڽſ��� �˸�
	NetMsgFB.emFB		 = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_OK;
	NetMsgFB.emSuit		 = pNetMsg->emSuit;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);

	// �ٸ� Ŭ���̾�Ʈ ȭ�鿡 �������� �ٲ� ���� ��µǾ�� �Ѵ�
	GLMSG::SNET_VEHICLE_REQ_SLOT_EX_HOLD_BRD NetMsgBRD;
	NetMsgBRD.dwGaeaID	  = m_dwGaeaID;
	NetMsgBRD.emSuit	  = pNetMsg->emSuit;
	NetMsgBRD.sItemCustom = SITEMCUSTOM ( NATIVEID_NULL() );
	SendMsgViewAround ( ( NET_MSG_GENERIC* )&NetMsgBRD );

	// Require DB����
	
	return S_OK;
}


HRESULT GLChar::MsgVehicleRemoveSlot( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_VEHICLE_REQ_REMOVE_SLOTITEM* pNetMsg = ( GLMSG::SNET_VEHICLE_REQ_REMOVE_SLOTITEM* )nmg;
	
	/* vehicle no accessory, Juver, 2018/02/14 */
	SITEM* pitem_vehicle = GLItemMan::GetInstance().GetItem( m_sVehicle.m_sVehicleID );
	if ( pitem_vehicle && pitem_vehicle->sVehicle.bNoAcc )	return E_FAIL;

	WORD wPosX(0), wPosY(0);
	SITEMCUSTOM	sSlotItemCustom = m_sVehicle.GetSlotitembySuittype ( pNetMsg->emSuit );

	SITEM* pSlotItem = GLItemMan::GetInstance().GetItem ( sSlotItemCustom.sNativeID );
	if ( !pSlotItem ) 
	{
		// �Ϲ� ����
		return E_FAIL;
	}

	BOOL bOk = m_cInventory.FindInsrtable ( pSlotItem->sBasicOp.wInvenSizeX, pSlotItem->sBasicOp.wInvenSizeY, wPosX, wPosY );
	if ( !bOk )
	{
		//	�ι��� ��������.
		return E_FAIL;
	}

	// �κ��� �ֱ�
	m_cInventory.InsertItem ( sSlotItemCustom, wPosX, wPosY );

	// ���Ծ����� ����
	m_sVehicle.ReSetSlotItem ( pNetMsg->emSuit );
	m_fVehicleSpeedRate = m_sVehicle.GetSpeedRate();
	m_fVehicleSpeedVol = m_sVehicle.GetSpeedVol();
	INIT_DATA( FALSE, FALSE );

	GLMSG::SNET_VEHICLE_REQ_REMOVE_SLOTITEM_FB NetMsgFB;
	NetMsgFB.emSuit = pNetMsg->emSuit;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);

	// ���Ծ����� ���Ÿ� �˸�
	GLMSG::SNET_VEHICLE_REQ_REMOVE_SLOTITEM_BRD NetMsgBRD;
	NetMsgBRD.dwGaeaID = m_dwGaeaID;
	NetMsgBRD.emSuit = pNetMsg->emSuit;
	SendMsgViewAround ( ( NET_MSG_GENERIC* )&NetMsgBRD );

	return S_OK;
}

HRESULT GLChar::MsgVehicleGiveBattery( NET_MSG_GENERIC* nmg )
{

	GLMSG::SNET_VEHICLE_REQ_GIVE_BATTERY* pNetMsg = ( GLMSG::SNET_VEHICLE_REQ_GIVE_BATTERY* ) nmg;
	
	GLMSG::SNET_VEHICLE_REQ_GIVE_BATTERY_FB NetMsgFB;
	SNATIVEID sNativeID;

	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( pNetMsg->wPosX, pNetMsg->wPosY );
	if ( !pInvenItem ) 
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_GIVE_BATTERY_FB_INVALIDITEM;
		return E_FAIL;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_GIVE_BATTERY_FB_INVALIDITEM;
		return E_FAIL;
	}

	// Ż�� ���� üũ
	if ( pItem->sBasicOp.emItemType != ITEM_VEHICLE )	
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_GIVE_BATTERY_FB_INVALIDITEM;
		return E_FAIL;
	}

	SITEM* pHold = GET_SLOT_ITEMDATA ( SLOT_HOLD );
	if ( !pHold ) 
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_GIVE_BATTERY_FB_INVALIDBATTERY;
		return E_FAIL;
	}

	// ���͸� ���� üũ
	if ( pHold->sBasicOp.emItemType != ITEM_VEHICLE_OIL )	
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_GIVE_BATTERY_FB_INVALIDBATTERY;
		return E_FAIL;
	}

	// ������ ������ ���� Ʋ���� �������� �ʴ´�.
	SITEM* pPutOnItem = GLItemMan::GetInstance().GetItem ( m_PutOnItems[SLOT_HOLD].sNativeID );
	if( pHold != pPutOnItem )
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_GIVE_BATTERY_FB_INVALIDBATTERY;
		return E_FAIL;
	}

	sNativeID = pHold->sBasicOp.sNativeID;

	switch ( pHold->sDrugOp.emDrug )
	{
	case ITEM_DRUG_HP:
		{
			if ( pInvenItem->sItemCustom.dwVehicleID == 0 ) return E_FAIL;

			CGetVehicleBattery* pDbAction = new CGetVehicleBattery ( m_dwClientID,
														m_dwCharID,
														pInvenItem->sItemCustom.dwVehicleID,
														pInvenItem->sItemCustom.sNativeID,
														pHold->sDrugOp.wCureVolume, 
														pHold->sDrugOp.bRatio,
														sNativeID,
														pHold->sVehicle.emVehicleType );													   ;

			GLDBMan* pDBMan = GLGaeaServer::GetInstance().GetDBMan ();
			if ( pDBMan ) pDBMan->AddJob ( pDbAction );
		}
		break;

	default:
		break;
	}

	return S_OK;

}

HRESULT	GLChar::MsgGetVehicleFullFromDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_VEHICLE_REQ_GET_BATTERY_FROMDB_FB* pIntMsg = ( GLMSG::SNET_VEHICLE_REQ_GET_BATTERY_FROMDB_FB* ) nmg;
	switch ( pIntMsg->emFB )
	{
	case EMVEHICLE_REQ_GET_BATTERY_FROMDB_FAIL:
		{
			GLMSG::SNET_VEHICLE_REQ_GIVE_BATTERY_FB NetMsgFB;
			NetMsgFB.emFB = EMVEHICLE_REQ_GET_BATTERY_FROMDB_FAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
		}
		break;

	case EMVEHICLE_REQ_GET_BATTERY_FROMDB_OK:
		{
			// �� ������ ���� �α�
			GLITEMLMT::GetInstance().ReqVehicleAction( pIntMsg->dwVehicleID, 
													pIntMsg->sBatteryID,
													EMVEHICLE_ACTION_BATTERY_BEFORE, 
													pIntMsg->nFull );

			/*dmk14 vehicle fix*/
			if ( pIntMsg->emType >= VEHICLE_TYPE_SIZE )	break;

			int nFull = pIntMsg->nFull;
			int nMaxFull = GLCONST_VEHICLE::pGLVEHICLE[pIntMsg->emType]->m_nFull;
			if ( nFull >= nMaxFull ) break;

			if ( pIntMsg->bRatio )
			{
				nFull += ( nMaxFull*pIntMsg->wCureVolume )/100;
				if ( nFull > nMaxFull ) nFull = nMaxFull;
			}
			else
			{
				nFull += pIntMsg->wCureVolume;
				if ( nFull > nMaxFull ) nFull = nMaxFull;
			}

			CSetVehicleBattery* pDbAction = new CSetVehicleBattery( m_dwClientID, m_dwCharID, pIntMsg->dwVehicleID, nFull );
			GLDBMan* pDbMan = GLGaeaServer::GetInstance().GetDBMan ();
			if ( pDbMan ) pDbMan->AddJob ( pDbAction );
			DoDrugSlotItem ( SLOT_HOLD );

			// �� ������ ���� �α�
			GLITEMLMT::GetInstance().ReqVehicleAction(	pIntMsg->dwVehicleID,
													pIntMsg->sBatteryID,
													EMVEHICLE_ACTION_BATTERY_AFTER,
													nFull );

			// ���� ������ ���� Ż�Ͱ� �����ѰŶ�� ������ �������ش�.
			// ������ ��Ȱ��ȭ �Ǿ������� ���߿� �ٷ� ����ϱ� ���� ����
			if ( m_sVehicle.m_dwGUID == pIntMsg->dwVehicleID )	
				m_sVehicle.IncreaseFull ( pIntMsg->wCureVolume, pIntMsg->bRatio );


			GLMSG::SNET_VEHICLE_REQ_GIVE_BATTERY_FB NetMsgFB;
			NetMsgFB.emFB	   = EMVEHICLE_REQ_GIVE_BATTERY_FB_OK;
			NetMsgFB.dwVehicleID = pIntMsg->dwVehicleID;
			NetMsgFB.sItemID	= pIntMsg->sItemID;
			NetMsgFB.sBatteryID = pIntMsg->sBatteryID;
			NetMsgFB.nFull	   = nFull;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
		}
	}

	return S_OK;
}

/*vehicle booster system, Juver, 2017/08/10 */
HRESULT GLChar::MsgVehicleEnableBooster( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_VEHICLE_REQ_ENABLE_BOOSTER* pNetMsg = ( GLMSG::SNET_VEHICLE_REQ_ENABLE_BOOSTER* ) nmg;

	GLMSG::SNET_VEHICLE_REQ_ENABLE_BOOSTER_FB NetMsgFB;
	SNATIVEID sNativeID;

	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( pNetMsg->wPosX, pNetMsg->wPosY );
	if ( !pInvenItem ) 
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_VEHICLE )	
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SITEM* pHold = GET_SLOT_ITEMDATA ( SLOT_HOLD );
	if ( !pHold ) 
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( pHold->sBasicOp.emItemType != ITEM_CARD_BIKEBOOST )	
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SITEM* pPutOnItem = GLItemMan::GetInstance().GetItem ( m_PutOnItems[SLOT_HOLD].sNativeID );
	if( pHold != pPutOnItem )
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( pHold->sVehicle.emVehicleType != pItem->sVehicle.emVehicleType )	
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_TYPE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( pHold->sVehicle.emVehicleSubType != pItem->sVehicle.emVehicleSubType )	
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_TYPE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	DWORD dwVehicleID = pInvenItem->sItemCustom.dwVehicleID;
	if (  dwVehicleID == 0 )
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( m_sVehicle.m_dwGUID == dwVehicleID && m_sVehicle.IsBooster() )	
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_ENABLE_BOOSTER_FB_ALREADY_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	CSetVehicleBooster* pDbAction = new CSetVehicleBooster( m_dwClientID, m_dwCharID, dwVehicleID, TRUE );
	GLDBMan* pDbMan = GLGaeaServer::GetInstance().GetDBMan ();
	if ( pDbMan ) pDbMan->AddJob ( pDbAction );

	DoDrugSlotItem ( SLOT_HOLD );

	if ( m_sVehicle.m_dwGUID == dwVehicleID )	
	{
		m_sVehicle.SetBooster( TRUE );
	}

	GLITEMLMT::GetInstance().ReqVehicleAction( dwVehicleID, pHold->sBasicOp.sNativeID, EMVEHICLE_ACTION_BOOSTER_ENABLE, m_sVehicle.m_bBooster );


	NetMsgFB.emFB = EMVEHICLE_REQ_ENABLE_BOOSTER_FB_OK;
	NetMsgFB.dwVehicleID = dwVehicleID;
	NetMsgFB.sCardID = pHold->sBasicOp.sNativeID;
	NetMsgFB.sItemID = pItem->sBasicOp.sNativeID;
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

	return S_OK;

}

HRESULT GLChar::MsgReqVietnamGainType ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_VIETNAM_TIME_REQ_FB NetMsgFB;
	NetMsgFB.gameTime  = m_sVietnamSystem.gameTime;
	NetMsgFB.loginTime = m_sVietnamSystem.loginTime;
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

	GLMSG::SNETPC_VIETNAM_GAINTYPE NetMsg;
	NetMsg.dwGainType = (BYTE)m_dwVietnamGainType;
	GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsg );
	return S_OK;
}

HRESULT GLChar::MsgItemShopOpen( NET_MSG_GENERIC* nmg ) // ItemShopOpen
{
#if defined ( JP_PARAM ) || defined ( _RELEASED)	// JAPAN Item Shop
	
	GLMSG::SNETPC_OPEN_ITEMSHOP* NetMsg = ( GLMSG::SNETPC_OPEN_ITEMSHOP* ) nmg;

	if ( m_bItemShopOpen == NetMsg->bOpen ) return E_FAIL;

	m_bItemShopOpen = NetMsg->bOpen;

	GLMSG::SNETPC_OPEN_ITEMSHOP_BRD NetMsgBrd;
	NetMsgBrd.bOpen = m_bItemShopOpen;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&NetMsgBrd) );

#endif
	
	return S_OK;
}

HRESULT GLChar::MsgAttendReward ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_ATTEND_REWARD_FLD* NetMsg = ( GLMSG::SNETPC_REQ_ATTEND_REWARD_FLD* ) nmg;

	GLMSG::SNETPC_REQ_ATTEND_REWARD_CLT	NetMsgFb;

	if ( NetMsg->idAttendReward == NATIVEID_NULL() )
	{
		NetMsgFb.idAttendReward = NetMsg->idAttendReward;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsgFb );
		return S_OK;
	}

	NetMsgFb.idAttendReward = NetMsg->idAttendReward;
	GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsgFb );	

	CString strPurKey;
	CString strUserID = m_szUID;
	CTime cCurTime ( CTime::GetCurrentTime() );
	strPurKey.Format( "%04d%02d%02d%s", cCurTime.GetYear(), cCurTime.GetMonth(), cCurTime.GetDay(), strUserID.GetString() );

	CInsertAttendItem* pDbAction = new CInsertAttendItem( strPurKey, strUserID, NetMsg->idAttendReward.wMainID, NetMsg->idAttendReward.wSubID );
	GLDBMan* pDbMan = GLGaeaServer::GetInstance().GetDBMan ();
	if ( pDbMan ) pDbMan->AddJob ( pDbAction );

	return S_OK;
}

HRESULT	GLChar::MsgGathering( NET_MSG_GENERIC* nmg )
{
	if ( !m_pLandMan )	return E_FAIL;

	GLMSG::SNETPC_REQ_GATHERING *pNetMsg = reinterpret_cast<GLMSG::SNETPC_REQ_GATHERING*> ( nmg );

	

	PGLMATERIAL pMaterial = m_pLandMan->GetMaterial ( pNetMsg->dwTargetID );

	if ( !pMaterial )
	{
		//	�������� ��������ϴ�.
		GLMSG::SNETPC_REQ_GATHERING_FB NetMsg;
		NetMsg.emFB = EMREQ_GATHER_FB_NOCROW;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );
		return E_FAIL;
	}

	if ( pMaterial->GetCrow() != CROW_MATERIAL )
	{
		//	�������� ��������ϴ�.
		GLMSG::SNETPC_REQ_GATHERING_FB NetMsg;
		NetMsg.emFB = EMREQ_GATHER_FB_NOTTYPE;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );
		return E_FAIL;
	}

	if ( !pMaterial->IsValidBody()  )
	{
		//	�������� ��������ϴ�.
		GLMSG::SNETPC_REQ_GATHERING_FB NetMsg;
		NetMsg.emFB = EMREQ_GATHER_FB_NOCROW;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );
		return E_FAIL;
	}

	// ���Ȯ��
	if ( !IsValidBody() )	return E_FAIL;
	if ( IsACTION( GLAT_GATHERING ) )
	{
		GLMSG::SNETPC_REQ_GATHERING_FB NetMsg;
		NetMsg.emFB = EMREQ_GATHER_FB_GATHERING;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );
		return E_FAIL;
	}

	//	�Ÿ� üũ
	D3DXVECTOR3 vPos;
	vPos = m_vPos;

	const D3DXVECTOR3 &vTarPos = pMaterial->GetPosition();

	D3DXVECTOR3 vDistance = vPos - vTarPos;
	float fDistance = D3DXVec3Length ( &vDistance );

	WORD wTarBodyRadius = 4;
	WORD wGatherRange = wTarBodyRadius + GETBODYRADIUS() + 2;
	WORD wGatherAbleDis = wGatherRange + 15;

	if ( fDistance>wGatherAbleDis )
	{
		//	�Ÿ��� �ٴϴ�.
		GLMSG::SNETPC_REQ_GATHERING_FB NetMsg;
		NetMsg.emFB = EMREQ_GATHER_FB_DISTANCE;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );

		return E_FAIL;
	}


	// �̹� �ٸ������ ������̶��???

	if ( pMaterial->m_dwGatherCharID != GAEAID_NULL )
	{
		//	�Ÿ��� �ٴϴ�.
		GLMSG::SNETPC_REQ_GATHERING_FB NetMsg;
		NetMsg.emFB = EMREQ_GATHER_FB_USE;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );
		return E_FAIL;
	}

	int nTime = RandomNumber( pMaterial->m_pCrowData->m_wGatherTimeLow, pMaterial->m_pCrowData->m_wGatherTimeHigh );
	// crow�� ����
	pMaterial->m_dwGatherCharID = m_dwGaeaID;
	pMaterial->m_fGatherTime = (float)nTime;

	// random Ÿ�� �����Ŀ� �ð� üũ

	//	User�� ����
	TurnAction ( GLAT_GATHERING );

	m_dwANISUBSELECT = pMaterial->m_pCrowData->m_wGatherAnimation;

	//	�Ÿ��� �ٴϴ�.
	GLMSG::SNETPC_REQ_GATHERING_FB NetMsg;
	NetMsg.emFB = EMREQ_GATHER_FB_OK;
	NetMsg.dwTime = nTime; // pCrow->dwTime;
	NetMsg.dwAniSel = m_dwANISUBSELECT;
	NetMsg.dwGaeaID = pMaterial->m_dwGlobID;
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );

	GLMSG::SNETPC_REQ_GATHERING_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	NetMsgBrd.dwAniSel = m_dwANISUBSELECT;
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

	return S_OK;
}
HRESULT GLChar::MsgHPCheck( NET_MSG_GENERIC* nmg)
{
	if ( !IsValidBody() )					return S_OK;
	if ( m_wLevel < 350 )	return S_OK;
	if ( m_sCONFTING.IsCONFRONTING() )		return S_OK;
	if ( m_bVehicle )						return S_OK;
	if ( m_sHP.dwNow == m_sHP.dwMax )			return S_OK;
	if ( m_sHP.dwNow == 0 )					return S_OK;
	if ( m_sMapID.wMainID == 222 )			return S_OK;
	
	GLMSG::SNETPC_HP_CHECK *pNetMsg = (GLMSG::SNETPC_HP_CHECK *) nmg;
	
	int wMaxHPDiff = abs((int)pNetMsg->m_sClientHP.dwMax - (int)m_sHP.dwMax);
	int wNowHPDiff = abs((int)pNetMsg->m_sClientHP.dwNow - (int)m_sHP.dwNow);
	
	bool bObviousCheaterHP(false);
	bool bObviousCheaterNowHP(false);
	if ( wMaxHPDiff > 1750 ) bObviousCheaterHP = true;
	if ( wNowHPDiff > 1750 ) m_wObviousCount++;
	
	if( m_sFreezeCheckHP.dwNow == pNetMsg->m_sClientHP.dwNow 
		&& bObviousCheaterHP ) m_wFreezeCount++; 

	if( m_wLevel >= 350 )
	{
		if( m_wObviousCount > 10 && bObviousCheaterNowHP ){
			CDebugSet::ToTracingFile(  "GerberSec-HP_ObviousDiff",
				"[wMaxC > wMaxS], [%d][%s], wNowC[%d], wMaxC[%d], wNowS[%d], wMaxS[%d], Frozen Value [%d]", 
				m_dwCharID, m_szName, pNetMsg->m_sClientHP.dwNow, pNetMsg->m_sClientHP.dwMax, m_sHP.dwNow, m_sHP.dwMax, m_sFreezeCheckHP.dwNow  );
			GLMSG::SNETPC_DROPCHAR_TOAGENT DropMsg;
			DropMsg.dwCHARID  = GetCharID();
			m_pGLGaeaServer->SENDTOAGENT( m_dwClientID, &DropMsg );
		}

		if( m_wFreezeCount > 10 && !bObviousCheaterNowHP && ( m_sMapID.wMainID == 23 || /*m_sMapID.wMainID == 222 ||*/ m_sMapID.wMainID == 231 || m_sMapID.wMainID == 214 ) )
		{
			bool bDontKick(false);
			if( pNetMsg->m_sClientHP.dwNow == m_sHP.dwNow ) bDontKick = true;
			if( pNetMsg->m_sClientHP.dwMax == m_sHP.dwMax ) bDontKick = true;
			if( !bDontKick )
			{
				CDebugSet::ToTracingFile(  "GerberSec-HP_MonitorFreeze",
					"[wMaxC > wMaxS], [%d][%s], wNowC[%d], wMaxC[%d], wNowS[%d], wMaxS[%d], Frozen Value [%d]", 
					m_dwCharID, m_szName, pNetMsg->m_sClientHP.dwNow, pNetMsg->m_sClientHP.dwMax, m_sHP.dwNow, m_sHP.dwMax, m_sFreezeCheckHP.dwNow  );
				
				GLMSG::SNETPC_DROPCHAR_TOAGENT DropMsg;
				DropMsg.dwCHARID  = GetCharID();
				m_pGLGaeaServer->SENDTOAGENT( m_dwClientID, &DropMsg );
			}
		}
	
		if( pNetMsg->m_sClientHP.dwNow > 200000 )
		{
			CDebugSet::ToTracingFile(  "GerberSec-HP_MonitorDC",
				"[wMaxC > wMaxS], [%d][%s], wNowC[%d], wMaxC[%d], wNowS[%d], wMaxS[%d]", 
				m_dwCharID, m_szName, pNetMsg->m_sClientHP.dwNow, pNetMsg->m_sClientHP.dwMax, m_sHP.dwNow, m_sHP.dwMax  );
			GLMSG::SNETPC_DROPCHAR_TOAGENT DropMsg;
			DropMsg.dwCHARID  = GetCharID();
			m_pGLGaeaServer->SENDTOAGENT( m_dwClientID, &DropMsg );
		}

		if( pNetMsg->m_sClientHP.dwMax > 200000 )
		{
			CDebugSet::ToTracingFile(  "GerberSec-HP_MonitorDC",
				"[wMaxC > wMaxS], [%d][%s], wNowC[%d], wMaxC[%d], wNowS[%d], wMaxS[%d]", 
				m_dwCharID, m_szName, pNetMsg->m_sClientHP.dwNow, pNetMsg->m_sClientHP.dwMax, m_sHP.dwNow, m_sHP.dwMax  );

			GLMSG::SNETPC_DROPCHAR_TOAGENT DropMsg;
			DropMsg.dwCHARID  = GetCharID();
			m_pGLGaeaServer->SENDTOAGENT( m_dwClientID, &DropMsg );
		}

		if ( (m_sHP.dwMax == pNetMsg->m_sClientHP.dwMax) && (pNetMsg->m_sClientHP.dwNow > m_sHP.dwMax) )
		{
			CDebugSet::ToTracingFile(  "GerberSec-HP_MonitorNM",
				"[wNowC > wMaxS], [%d][%s], wNowC[%d], wMaxC[%d], wNowS[%d], wMaxS[%d]", 
				m_dwCharID, m_szName, pNetMsg->m_sClientHP.dwNow, pNetMsg->m_sClientHP.dwMax, m_sHP.dwNow, m_sHP.dwMax  );

			GLMSG::SNETPC_DROPCHAR_TOAGENT DropMsg;
			DropMsg.dwCHARID  = GetCharID();
			m_pGLGaeaServer->SENDTOAGENT( m_dwClientID, &DropMsg );
		}
	}

	m_sFreezeCheckHP = pNetMsg->m_sClientHP;

	return S_OK;
}

//add statsup cmd
inline HRESULT GLChar::MsgReqStatsUpCmd ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_STATSUPCMD *pNetMsg = (GLMSG::SNETPC_REQ_STATSUPCMD *) nmg;
	if ( m_wStatsPoint < pNetMsg->dwValue )	return S_FALSE;

	//	������ STATE�� ������Ŵ.
	STATSUP_CMD(pNetMsg->emStats,pNetMsg->dwValue);

	//	ó�� ��� ����.
	GLMSG::SNETPC_REQ_STATSUPCMD_FB NetMsg;
	NetMsg.emStats = pNetMsg->emStats;
	NetMsg.dwValue = pNetMsg->dwValue;
	NetMsg.wStatsPoint = m_wStatsPoint;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg);

	//	[��Ƽ,�����] hp ��ȭ �˸�.
	MsgSendUpdateState();

	return S_OK;
}

HRESULT GLChar::MsgProcess ( NET_MSG_GENERIC* nmg )
{
	if (nmg == NULL) return E_FAIL;
	switch ( nmg->nType )
	{
	//PANDORA
	case NET_MSG_PANDORA_BOX_REFRESH_RESULT:
	{
		// C++17 hardening: validate size before static_cast (OOB-read prevention).
		if (nmg->dwSize < sizeof(GLMSG::SNET_PANDORA_BOX_REFRESH_RESULT)) break;
		GLMSG::SNET_PANDORA_BOX_REFRESH_RESULT* pNetMsg =
			static_cast<GLMSG::SNET_PANDORA_BOX_REFRESH_RESULT*>(nmg);

		float fNowRate = seqrandom::getpercent();
		float fLOW = 0;
		for (DWORD i = 0; i < GLCONST_CHAR::vecPandoraBoxes.size(); ++i)
		{
			GLPANDORA_BOX sITEM = GLCONST_CHAR::vecPandoraBoxes[i];

			if (fLOW <= fNowRate && fNowRate < (fLOW + sITEM.fRate))
			{
				const SITEM* pItem = GLItemMan::GetInstance().GetItem(sITEM.sItemID);
				if (pItem)
				{
					sITEM.nIndex = m_mapPandoraResults.size() + 1;
					m_mapPandoraResults.insert(std::make_pair(sITEM.nIndex, sITEM));

					m_sPandoraStatus.uCount++;
					m_sPandoraStatus.fReqDelay = GLCONST_CHAR::sPandoraSetting.fReqDelay;

					// Remove the last inserted item
					auto it = m_mapPandoraResults.find(sITEM.nIndex);
					if (it != m_mapPandoraResults.end())
					{
						m_mapPandoraResults.erase(it);  // Remove the item you just inserted

						// Decrement the count in PandoraStatus
						m_sPandoraStatus.uCount--;
					}

					break;
				}
			}
			fLOW += sITEM.fRate;
		}

		GLMSG::SNET_PANDORA_BOX_RESULT NetMsgRes;
		int i = 0;
		for (PANDORABOXMAP_ITER iter = m_mapPandoraResults.begin();
			iter != m_mapPandoraResults.end(); ++iter)
			NetMsgRes.sBOX[i++] = iter->second;

		switch (m_sPandoraStatus.uCount)
		{
		case 10:
			m_sPandoraStatus.emStatus = EMPANDORA_STATUS_RARE;
			break;
		case 20:
			m_sPandoraStatus.emStatus = EMPANDORA_STATUS_FREE;
			break;
		default:
			m_sPandoraStatus.emStatus = EMPANDORA_STATUS_OPEN;
			if (m_sPandoraStatus.uCount > 20)
				m_sPandoraStatus.uCount = 0;
			break;
		}
		NetMsgRes.sStatus = m_sPandoraStatus;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &NetMsgRes);
	}
	break;
	case NET_MSG_PANDORA_BOX_OPEN_REQ:
		{
			// C++17 hardening: validate size before static_cast (OOB-read prevention).
			if (nmg->dwSize < sizeof(GLMSG::SNET_PANDORA_BOX_OPEN_REQ)) break;
			GLMSG::SNET_PANDORA_BOX_OPEN_REQ* pNetMsg =
				static_cast< GLMSG::SNET_PANDORA_BOX_OPEN_REQ* >( nmg );

			GLMSG::SNET_PANDORA_BOX_OPEN_FB NetMsgFB;

			if ( m_mapPandoraResults.size() >= GLPANDORA_BOX::PANDORA_RESULT_SIZE )
			{
				std::string strText( "Remove 1 or more Pandora Item to Continue..." );

				GLMSG::SNET_SERVER_GENERALCHAT SendData;
				SendData.SETTEXT(strText.c_str());
				SendData.bNotify = true;
				SendData.iColor = 0;
				GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&SendData);
				return S_FALSE;
			}

			if ( m_sPandoraStatus.fReqDelay > 0.0f )
			{
				CString strText;
				strText.Format("Please wait for %.2fsecond(s)...", m_sPandoraStatus.fReqDelay);

				GLMSG::SNET_SERVER_GENERALCHAT SendData;
				SendData.SETTEXT(strText.GetString());
				SendData.bNotify = true;
				SendData.iColor = 0;
				GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&SendData);
				return S_FALSE;
			}

			switch ( pNetMsg->emReq )
			{
			case EMPANDORA_OPEN_BOX_GOLD:
				{
					if ( GetInvenMoney() < GLCONST_CHAR::sPandoraSetting.llGold )
					{
						NetMsgFB.emFB = EMPANDORA_OPEN_BOX_FB_NOGOLD;
						GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
						return S_FALSE;
					}
					NetMsgFB.emFB = EMPANDORA_OPEN_BOX_GOLD_FB_OK;
					if ( m_sPandoraStatus.emStatus != EMPANDORA_STATUS_FREE )
					{
						m_lnMoney -= GLCONST_CHAR::sPandoraSetting.llGold;

						GLMSG::SNETPC_UPDATE_MONEY NetMsgMoney;
						NetMsgMoney.lnMoney = m_lnMoney;
						NetMsgMoney.dwPremium = m_dwPremiumPoints;
						NetMsgMoney.dwCombat = m_dwCombatPoints;
						GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgMoney );
					}
				}break;
			case EMPANDORA_OPEN_BOX_PREM:
				{
					if ( m_dwPremiumPoints < GLCONST_CHAR::sPandoraSetting.wEP )
					{
						NetMsgFB.emFB = EMPANDORA_OPEN_BOX_FB_NOEP;
						GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
						return S_FALSE;
					}
					NetMsgFB.emFB = EMPANDORA_OPEN_BOX_PREM_FB_OK;
					if ( m_sPandoraStatus.emStatus != EMPANDORA_STATUS_FREE )
					{
						CDeductMoneyPoints* pDbAction = new CDeductMoneyPoints ( ((int)GLCONST_CHAR::sPandoraSetting.wEP), m_szUID );
						GLDBMan* pDbMan = GLGaeaServer::GetInstance().GetDBMan ();
						if ( pDbMan ) pDbMan->AddJob ( pDbAction );	

						/*CGetPoints* pDbAction2 = new CGetPoints ( m_dwCharID, m_dwUserID , m_dwClientID );
						if ( pDbMan ) pDbMan->AddJob ( pDbAction2 );*/

						//PANDORA - UPDATED BY ARWEKAJ09 - NOT USING THE CGetPoints
						m_dwPremiumPoints -= GLCONST_CHAR::sPandoraSetting.wEP;

						GLMSG::SNETPC_UPDATE_MONEY NetMsgMoney;
						NetMsgMoney.lnMoney = m_lnMoney;
						NetMsgMoney.dwPremium = m_dwPremiumPoints;
						NetMsgMoney.dwCombat = m_dwCombatPoints;
						GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgMoney);
					}
				}break;
			}

			float fNowRate = seqrandom::getpercent();
			float fLOW = 0;
			for ( DWORD i=0; i<GLCONST_CHAR::vecPandoraBoxes.size(); ++i )
			{
				GLPANDORA_BOX sITEM = GLCONST_CHAR::vecPandoraBoxes[i];

				if ( fLOW <= fNowRate && fNowRate < ( fLOW + sITEM.fRate ) )
				{
					const SITEM* pItem = GLItemMan::GetInstance().GetItem(sITEM.sItemID);
					if ( pItem )
					{
						sITEM.nIndex = m_mapPandoraResults.size() + 1;
						m_mapPandoraResults.insert ( std::make_pair( sITEM.nIndex, sITEM ) );

						m_sPandoraStatus.uCount ++;
						m_sPandoraStatus.fReqDelay = GLCONST_CHAR::sPandoraSetting.fReqDelay;

						break;
					}
				}
				fLOW += sITEM.fRate;
			}

			GLMSG::SNET_PANDORA_BOX_RESULT NetMsgRes;
			int i=0;
			for ( PANDORABOXMAP_ITER iter = m_mapPandoraResults.begin();
				iter!= m_mapPandoraResults.end(); ++iter )
				NetMsgRes.sBOX[i++] = iter->second;

			switch ( m_sPandoraStatus.uCount )
			{
			case 10:
				m_sPandoraStatus.emStatus = EMPANDORA_STATUS_RARE;
				break;
			case 20:
				m_sPandoraStatus.emStatus = EMPANDORA_STATUS_FREE;
				break;
			default:
				m_sPandoraStatus.emStatus = EMPANDORA_STATUS_OPEN;
				if ( m_sPandoraStatus.uCount > 20 )
					m_sPandoraStatus.uCount = 0;
				break;
			}
			NetMsgRes.sStatus = m_sPandoraStatus;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgRes);
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		}
		break;
	case NET_MSG_PANDORA_BOX_BUY_ITEM:
		{
			GLMSG::SNET_PANDORA_BOX_BUY_ITEM* pNetMsg =
				static_cast< GLMSG::SNET_PANDORA_BOX_BUY_ITEM* >( nmg );

			GLMSG::SNET_PANDORA_BOX_BUY_ITEM_FB NetMsgFB;

			PANDORABOXMAP_ITER iter_find = m_mapPandoraResults.find(pNetMsg->nIndex);
			if ( iter_find == m_mapPandoraResults.end() )
			{
				NetMsgFB.emFB = EMPANDORA_ITEM_BUY_FAILED;
				GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
				return S_FALSE;
			}

			switch ( pNetMsg->emReq )
			{
			case EMPANDORA_ITEM_REMOVE_REQ:
				{
					m_mapPandoraResults.erase(iter_find);
					NetMsgFB.emFB = EMPANDORA_ITEM_REMOVE_OK;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
				}
				break;
			case EMPANDORA_ITEM_BUY_GOLD_REQ:
				{
					if ( m_lnMoney < iter_find->second.llGold )
					{
						NetMsgFB.emFB = EMPANDORA_ITEM_BUY_NOGOLD;
						GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
						return S_FALSE;
					}

					// Â¾Ã†Ã€ÃŒÃ…Ã› Â»Ã½Â¼Âº
					SITEMCUSTOM sITEM_NEW;
					sITEM_NEW.sNativeID = iter_find->second.sItemID;
					if ( sITEM_NEW.sNativeID==SNATIVEID(false) )				return E_FAIL;

					SITEM *pITEM = GLItemMan::GetInstance().GetItem ( sITEM_NEW.sNativeID );
					if ( !pITEM )											return E_FAIL;

					CTime cTIME = CTime::GetCurrentTime();
					sITEM_NEW.tBORNTIME = cTIME.GetTime();

					sITEM_NEW.cGenType = EMGEN_SYSTEM;
					sITEM_NEW.cChnID = (BYTE)GLGaeaServer::GetInstance().GetServerChannel();
					sITEM_NEW.cFieldID = (BYTE)GLGaeaServer::GetInstance().GetFieldSvrID();
					sITEM_NEW.lnGenNum = GLITEMLMT::GetInstance().RegItemGen ( sITEM_NEW.sNativeID, (EMITEMGEN)sITEM_NEW.cGenType );

					sITEM_NEW.cDAMAGE = (BYTE)pITEM->sBasicOp.wGradeAttack; //item is +
					sITEM_NEW.cDEFENSE = (BYTE)pITEM->sBasicOp.wGradeDefense;//item is +

					if( sITEM_NEW.GENERATE_RANDOM_OPT() )
					{
						GLITEMLMT::GetInstance().ReqRandomItem( sITEM_NEW );
					}

					WORD wInsertPosX, wInsertPosY;
					BOOL bOk = m_cInventory.FindInsrtable ( pITEM->sBasicOp.wInvenSizeX, pITEM->sBasicOp.wInvenSizeY, wInsertPosX, wInsertPosY );
					if ( !bOk )		return S_OK;	

					m_cInventory.InsertItem ( sITEM_NEW, wInsertPosX, wInsertPosY );
					SINVENITEM *pInsertItem = m_cInventory.GetItem ( wInsertPosX, wInsertPosY );

					if ( pInsertItem )
					{
						GLITEMLMT::GetInstance().ReqItemRoute ( pInsertItem->sItemCustom, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, pInsertItem->sItemCustom.wTurnNum );

						GLMSG::SNETPC_INVEN_INSERT NetMsg_Inven;
						NetMsg_Inven.Data = *pInsertItem;
						GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_Inven);


						m_lnMoney -= iter_find->second.llGold;

						GLMSG::SNETPC_UPDATE_MONEY NetMsgMoney;
						NetMsgMoney.lnMoney = m_lnMoney;
						NetMsgMoney.dwPremium = m_dwPremiumPoints;
						NetMsgMoney.dwCombat = m_dwCombatPoints;
						GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgMoney );

						m_mapPandoraResults.erase(iter_find);
						NetMsgFB.emFB = EMPANDORA_ITEM_BUY_GOLD_OK;
						GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
					}
				}break;
			case EMPANDORA_ITEM_BUY_PREM_REQ:
				{
					if ( m_dwPremiumPoints < GLCONST_CHAR::sPandoraSetting.wEP )
					{
						NetMsgFB.emFB = EMPANDORA_ITEM_BUY_NOEP;
						GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
						return S_FALSE;
					}
					// Â¾Ã†Ã€ÃŒÃ…Ã› Â»Ã½Â¼Âº
					SITEMCUSTOM sITEM_NEW;
					sITEM_NEW.sNativeID = iter_find->second.sItemID;
					if ( sITEM_NEW.sNativeID==SNATIVEID(false) )				return E_FAIL;

					SITEM *pITEM = GLItemMan::GetInstance().GetItem ( sITEM_NEW.sNativeID );
					if ( !pITEM )											return E_FAIL;

					CTime cTIME = CTime::GetCurrentTime();
					sITEM_NEW.tBORNTIME = cTIME.GetTime();

					sITEM_NEW.cGenType = EMGEN_SYSTEM;
					sITEM_NEW.cChnID = (BYTE)GLGaeaServer::GetInstance().GetServerChannel();
					sITEM_NEW.cFieldID = (BYTE)GLGaeaServer::GetInstance().GetFieldSvrID();
					sITEM_NEW.lnGenNum = GLITEMLMT::GetInstance().RegItemGen ( sITEM_NEW.sNativeID, (EMITEMGEN)sITEM_NEW.cGenType );

					sITEM_NEW.cDAMAGE = (BYTE)pITEM->sBasicOp.wGradeAttack; //item is +
					sITEM_NEW.cDEFENSE = (BYTE)pITEM->sBasicOp.wGradeDefense;//item is +

					if( sITEM_NEW.GENERATE_RANDOM_OPT() )
					{
						GLITEMLMT::GetInstance().ReqRandomItem( sITEM_NEW );
					}

					WORD wInsertPosX, wInsertPosY;
					BOOL bOk = m_cInventory.FindInsrtable ( pITEM->sBasicOp.wInvenSizeX, pITEM->sBasicOp.wInvenSizeY, wInsertPosX, wInsertPosY );
					if ( !bOk )		return S_OK;	

					m_cInventory.InsertItem ( sITEM_NEW, wInsertPosX, wInsertPosY );
					SINVENITEM *pInsertItem = m_cInventory.GetItem ( wInsertPosX, wInsertPosY );

					if ( pInsertItem )
					{
						GLITEMLMT::GetInstance().ReqItemRoute ( pInsertItem->sItemCustom, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, pInsertItem->sItemCustom.wTurnNum );

						GLMSG::SNETPC_INVEN_INSERT NetMsg_Inven;
						NetMsg_Inven.Data = *pInsertItem;
						GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_Inven);

						CDeductMoneyPoints* pDbAction = new CDeductMoneyPoints ( ((int)GLCONST_CHAR::sPandoraSetting.wEP), m_szUID );
						GLDBMan* pDbMan = GLGaeaServer::GetInstance().GetDBMan ();
						if ( pDbMan ) pDbMan->AddJob ( pDbAction );	

						/*CGetPoints* pDbAction2 = new CGetPoints ( m_dwCharID, m_dwUserID , m_dwClientID );
						if ( pDbMan ) pDbMan->AddJob ( pDbAction2 );*/

						//PANDORA - UPDATED BY ARWEKAJ09 - NOT USING THE CGetPoints
						m_dwPremiumPoints -= GLCONST_CHAR::sPandoraSetting.wEP;

						GLMSG::SNETPC_UPDATE_MONEY NetMsgMoney;
						NetMsgMoney.lnMoney = m_lnMoney;
						NetMsgMoney.dwCombat = m_dwCombatPoints;
						NetMsgMoney.dwPremium = m_dwPremiumPoints;
						GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgMoney);

						m_mapPandoraResults.erase(iter_find);
						NetMsgFB.emFB = EMPANDORA_ITEM_BUY_PREM_OK;
						GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
					}

				}break;
			};

			VECPANDORABOX vecTemp;
			vecTemp.reserve(m_mapPandoraResults.size());

			for ( PANDORABOXMAP_ITER iter = m_mapPandoraResults.begin(); 
				iter!= m_mapPandoraResults.end(); ++iter )
			{
				GLPANDORA_BOX sPandora = iter->second;
				vecTemp.push_back(sPandora);
			}

			m_mapPandoraResults.clear();

			std::sort(vecTemp.begin(),vecTemp.end());

			for ( int i = 0; i < vecTemp.size(); ++i )
				vecTemp[i].nIndex = i + 1;

			for ( int i = 0; i < vecTemp.size(); ++i )
				m_mapPandoraResults.insert( std::make_pair( vecTemp[i].nIndex, vecTemp[i] ) );

			GLMSG::SNET_PANDORA_BOX_RESULT NetMsgRes;
			int i=0;
			for ( PANDORABOXMAP_ITER iter = m_mapPandoraResults.begin(); 
				iter!= m_mapPandoraResults.end(); ++iter )
				NetMsgRes.sBOX[i++] = iter->second;

			NetMsgRes.sStatus = m_sPandoraStatus;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgRes);
		}
		break;
	case NET_MSG_PANDORA_BOX_CLEAR_RESULTS_REQ:
		{
			m_mapPandoraResults.clear();
			GLMSG::SNET_PANDORA_BOX_RESULT NetMsgRes;
			NetMsgRes.sStatus = m_sPandoraStatus;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgRes);
		}
		break;

	case NET_MSG_GCTRL_REQ_READY:			MsgReady(nmg);					break;
	case NET_MSG_GCTRL_ACTSTATE:			MsgActState(nmg);				break;
	case NET_MSG_GCTRL_MOVESTATE:			MsgMoveState(nmg);				break;

	case NET_MSG_GCTRL_GOTO:				MsgGoto(nmg);					break;

	case NET_MSG_GCTRL_REQ_STATSUPCMD:		MsgReqStatsUpCmd(nmg);			break; // add addstats cmd

	case NET_MSG_GCTRL_ATTACK:				MsgAttack(nmg);					break;

	case NET_MSG_GCTRL_ATTACK_CANCEL:		MsgAttackCancel(nmg);			break;

	case NET_MSG_GCTRL_REQ_FIELD_TO_INVEN:	MsgReqFieldToInven(nmg);		break;
	case NET_MSG_GCTRL_REQ_FIELD_TO_HOLD:	MsgReqFieldToHold(nmg);			break;

	case NET_MSG_GCTRL_REQ_INVEN_TO_HOLD:	MsgReqInvenToHold(nmg); 		break;
	case NET_MSG_GCTRL_REQ_HOLD_ARRAY:		MsgReqHoldArray(nmg);			break;
	case NET_MSG_GCTRL_REQ_INVEN_EX_HOLD:	MsgReqInvenExHold(nmg); 		break;
	case NET_MSG_GCTRL_REQ_INVEN_TO_SLOT:	MsgReqInvenToSlot(nmg);			break;

	case NET_MSG_GCTRL_REQ_VNGAIN_TO_HOLD:  MsgReqVNGainToHold(nmg);		break;
	case NET_MSG_GCTRL_REQ_VNGAIN_EX_HOLD:  MsgReqVNGainExHold(nmg);		break;
	case NET_MSG_GCTRL_REQ_HOLD_TO_VNGAIN:	MsgReqHoldToVNGain(nmg);		break;
	case NET_MSG_GCTRL_REQ_VNINVEN_TO_INVEN: MsgReqVNInvenToInven(nmg);		break;

	case NET_MSG_GCTRL_REQ_VNGAIN_INVEN_RESET: MsgReqVNGainInvenReset(nmg);		break;

	case NET_MSG_GCTRL_REQ_SLOT_TO_HOLD:	MsgReqSlotToHold(nmg);			break;
	case NET_MSG_GCTRL_REQ_SLOT_EX_HOLD:	MsgReqSlotExHold(nmg);			break;


	case NET_MSG_GCTRL_REQ_INVEN_SPLIT:		MsgReqInvenSplit(nmg);			break;


	case NET_MSG_GCTRL_REQ_HOLD_TO_FIELD:	MsgReqHoldToField(nmg); 		break;
	case NET_MSG_GCTRL_REQ_HOLD_TO_INVEN:	MsgReqHoldToInven(nmg); 		break;
	case NET_MSG_GCTRL_REQ_HOLD_TO_SLOT:	MsgReqHoldToSlot(nmg);			break;
	case NET_MSG_GCTRL_REQ_SLOT_CHANGE:		MsgReqSlotChange(nmg);			break;

	case NET_MSG_GCTRL_REQ_BUY_FROM_NPC:	MsgReqBuyFromNpc(nmg);			break;
	case NET_MSG_GCTRL_REQ_SALE_TO_NPC:		MsgReqSaleToNpc(nmg);			break;

	case NET_MSG_GCTRL_REQ_REBIRTH:			MsgReqReBirth(nmg);				break;
	case NET_MSG_GCTRL_REQ_LEVELUP:			MsgReqLevelUp(nmg);				break;
	case NET_MSG_GCTRL_REQ_STATSUP:			MsgReqStatsUp(nmg);				break;

	case NET_MSG_GCTRL_REQ_LEARNSKILL:		MsgReqLearnSkill(nmg);			break;
	case NET_MSG_GCTRL_REQ_SKILLUP:			MsgReqSkillUp(nmg);				break;

	case NET_MSG_GCTRL_FIRECRACKER:			MsgReqFireCracker(nmg);			break;
	case NET_MSG_GCTRL_REQ_INVENDRUG:		MsgReqInvenDrug(nmg);			break;
	case NET_MSG_GCTRL_REQ_DAMAGE_OVERLAP:	MsgReqDamageOverlap(nmg);		break;
	case NET_MSG_GCTRL_REQ_SUICIDE_CMD:		MsgReqSuicideCmd(nmg);			break;
	case NET_MSG_GCTRL_INVEN_BOXOPEN:		MsgReqInvenBoxOpen(nmg);		break;
	/*Codex System, Jhoniex 2/14/2024 */
	case NET_MSG_GCTRL_REGISTER_CODEX:		MsgReqRegisterCodex(nmg);		break;
	case NET_MSG_GCTRL_INVEN_GMITEM:		MsgReqInvenGMItem(nmg);			break; //add itemcmd
	case NET_MSG_GCTRL_REQ_LEARNSKILL_NONINVEN:		MsgReqLearnSkillNonInven(nmg); break;
	case NET_MSG_GCTRL_GET_CHARGEDITEM_FROMDB: MsgReqInvenBoxInfo(nmg);		break;
	case NET_MSG_GCTRL_INVEN_DISGUISE:		MsgReqInvenDisguise(nmg);		break;
	case NET_MSG_GCTRL_INVEN_CLEANSER:		MsgReqInvenCleanser(nmg);		break;
	case NET_MSG_GCTRL_CHARGED_ITEM_GET:	MsgReqInvenChargedItem(nmg);	break;
	case NET_MSG_GCTRL_INVEN_RANDOMBOXOPEN:	MsgReqInvenRandomBoxOpen(nmg);	break;
	case NET_MSG_GCTRL_INVEN_DISJUNCTION:	MsgReqInvenDisJunction(nmg);	break;

	case NET_MSG_GCTRL_REQ_SKILL:			MsgReqSkill(nmg);				break;
	case NET_MSG_GCTRL_SKILL_CANCEL:		MsgReqSkillCancel(nmg);			break;

	case NET_MSG_GCTRL_REQ_SKILLQ_ACTIVE:	MsgReqSkillQSetActive(nmg);		break;
	case NET_MSG_GCTRL_REQ_SKILLQ_SET:		MsgReqSkillQSet(nmg);			break;
	case NET_MSG_GCTRL_REQ_SKILLQ_RESET:	MsgReqSkillQReSet(nmg);			break;

	case NET_MSG_GCTRL_REQ_ACTIONQ_SET:		MsgReqActionQSet(nmg);			break;
	case NET_MSG_GCTRL_REQ_ACTIONQ_RESET:	MsgReqActionQReSet(nmg);		break;

	case NET_MSG_GCTRL_REQ_GETSTORAGE:		MsgReqGetStorage(nmg);			break;
	case NET_MSG_GCTRL_REQ_STORAGEDRUG:		MsgReqStorageDrug(nmg);			break;
	case NET_MSG_GCTRL_REQ_STORAGESKILL:	MsgReqStorageSkill(nmg);		break;

	case NET_MSG_GCTRL_REQ_STORAGE_TO_HOLD:	MsgReqStorageToHold(nmg);		break;
	case NET_MSG_GCTRL_REQ_STORAGE_EX_HOLD:	MsgReqStorageExHold(nmg);		break;
	case NET_MSG_GCTRL_REQ_HOLD_TO_STORAGE:	MsgReqHoldToStorage(nmg);		break;

	case NET_MSG_GCTRL_STORAGE_SAVE_MONEY:	MsgReqStorageSaveMoney(nmg);	break;
	case NET_MSG_GCTRL_STORAGE_DRAW_MONEY:	MsgReqStorageDrawMoney(nmg);	break;


	case NET_MSG_GCTRL_REQ_STORAGE_SPLIT:	MsgReqStorageSplit(nmg);		break;

	case NET_MSG_GCTRL_REQ_MONEY_TO_FIELD:	MsgReqMoneyToField(nmg);		break;
	case NET_MSG_GCTRL_INVEN_GRINDING:		MsgReqInvenGrinding(nmg);		break;
	case NET_MSG_GCTRL_INVEN_RESET_SKST:	MsgReqInvenResetSkSt(nmg);		break;

	case NET_MSG_GCTRL_REGEN_GATE:			MsgReqReGenGate(nmg);			break;
	case NET_MSG_GCTRL_CURE:				MsgReqCure(nmg);				break;
	case NET_MSG_GCTRL_REVIVE:				MsgReqRevive(nmg);				break;
	case NET_MSG_GCTRL_2_FRIEND_FLD:		MsgReqFriendFld(nmg);			break;
	case NET_MSG_GM_MOVE2CHAR_FLD:			MsgReqMove2CharFld(nmg);		break;

	case NET_MSG_GCTRL_GETEXP_RECOVERY:		MsgReqGetExpRecovery(nmg);		break;
	case NET_MSG_GCTRL_RECOVERY:			MsgReqRecovery(nmg);			break;
	case NET_MSG_GCTRL_GETEXP_RECOVERY_NPC:	MsgReqGetExpRecoveryNpc(nmg);	break;
	case NET_MSG_GCTRL_RECOVERY_NPC:		MsgReqRecoveryNpc(nmg);			break;

	case NET_MSG_GCTRL_CHARRESET:			MsgReqCharReset(nmg);			break;
	case NET_MSG_GCTRL_INVEN_CHARCARD:		MsgReqCharCard(nmg);			break;
	case NET_MSG_GCTRL_INVEN_STORAGECARD:	MsgReqStorageCard(nmg);			break;
	case NET_MSG_GCTRL_INVEN_INVENLINE:		MsgReqInvenLine(nmg);			break;
	case NET_MSG_GCTRL_INVEN_STORAGEOPEN:	MsgReqInvenStorageOpen(nmg);	break;
	case NET_MSG_GCTRL_INVEN_REMODELOPEN:	MsgReqInvenRemodelOpen(nmg);	break;
	case NET_MSG_GCTRL_INVEN_GARBAGEOPEN:	MsgReqInvenGarbageOpen(nmg);	break;
	case NET_MSG_GCTRL_INVEN_STORAGECLOSE:	MsgReqInvenStorageClose(nmg);	break;
	case NET_MSG_GCTRL_INVEN_PREMIUMSET:	MsgReqInvenPremiumSet(nmg);		break;

	case NET_MSG_GCTRL_INVEN_HAIR_CHANGE:	MsgReqInvenHairChange(nmg);		break;
	case NET_MSG_GCTRL_INVEN_FACE_CHANGE:	MsgReqInvenFaceChange(nmg);		break;
	case NET_MSG_GCTRL_INVEN_HAIRSTYLE_CHANGE: MsgReqInvenHairStyleChange(nmg);		break;
	case NET_MSG_GCTRL_INVEN_HAIRCOLOR_CHANGE: MsgReqInvenHairColorChange(nmg);		break;
	case NET_MSG_GCTRL_INVEN_FACESTYLE_CHANGE: MsgReqInvenFaceStyleChange(nmg);		break;
	case NET_MSG_GCTRL_INVEN_GENDER_CHANGE:	MsgReqInvenGenderChange(nmg);	break;
	case NET_MSG_GCTRL_INVEN_RENAME:		MsgReqInvenRename(nmg);			break;
	case NET_MSG_GCTRL_INVEN_RENAME_FROM_DB: MsgInvenRename(nmg);			break;
		
	case NET_MSG_GCTRL_CARD_BRIGHT:			MsgReqResetBrightPoint(nmg);	break;
	//CNDev | 2-7-2020 | e-point card system
	case NET_MSG_GCTRL_CARD_EPOINT:			MsgReqCardEPoint(nmg);	break;
	case NET_MSG_GCTRL_CARD_CONTRIBUTION:	MsgReqCardContribution(nmg);	break;


	case NET_MSG_GCTRL_INVEN_VIETNAM_INVENGET: MsgInvenVietnamGet(nmg);		break;

	case NET_MSG_GCTRL_CONFRONT_START2_FLD:	MsgReqConFrontStart(nmg);		break;
	case NET_MSG_GCTRL_CONFRONT_END2_FLD:	MsgReqConFrontEnd(nmg);			break;

	case NET_MSG_GCTRL_NPC_ITEM_TRADE:		MsgReqNpcItemTrade(nmg);		break;

	case NET_MSG_GCTRL_REQ_QUEST_START:		MsgReqNpcQuestStart(nmg);		break;
	case NET_MSG_GCTRL_QUEST_PROG_NPCTALK:	MsgReqNpcQuestTalk(nmg);		break;

	case NET_MSG_GCTRL_QUEST_PROG_GIVEUP:	MsgReqQuestGiveUp(nmg);			break;
	case NET_MSG_GCTRL_QUEST_PROG_READ:		MsgReqQuestReadReset(nmg);		break;
	case NET_MSG_GCTRL_REQ_QUEST_COMPLETE:	MsgReqQuestComplete(nmg);		break;

	case NET_MSG_GCTRL_REQ_GESTURE:			MsgReqGesture ( nmg );			break;

	case NET_MSG_CHAT_LOUDSPEAKER:			MsgLoudSpeaker(nmg);			break;

	case NET_MSG_GCTRL_PMARKET_TITLE:		MsgPMarketTitle(nmg);			break;
	case NET_MSG_GCTRL_PMARKET_REGITEM:		MsgPMarketReqItem(nmg);			break;
	case NET_MSG_GCTRL_PMARKET_DISITEM:		MsgPMarketDisItem(nmg);			break;
	case NET_MSG_GCTRL_PMARKET_OPEN:		MsgPMarketOpen(nmg);			break;
	case NET_MSG_GCTRL_PMARKET_CLOSE:		MsgPMarketClose(nmg);			break;
	case NET_MSG_GCTRL_PMARKET_ITEM_INFO:	MsgPMarketItemInfo(nmg);		break;
	case NET_MSG_GCTRL_PMARKET_BUY:			MsgPMarketBuy(nmg);				break;

	case NET_MSG_GCTRL_CLUB_NEW:			MsgClubNew(nmg);				break;
	case NET_MSG_GCTRL_CLUB_RANK:			MsgClubRank(nmg);				break;
	case NET_MSG_GCTRL_CLUB_NEW_2FLD:		MsgClubNew2FLD(nmg);			break;
	case NET_MSG_GCTRL_CLUB_MEMBER_REQ:		MsgClubMemberReq(nmg);			break;
	case NET_MSG_GCTRL_CLUB_MEMBER_REQ_ANS:	MsgClubMemberAns(nmg);			break;
	case NET_MSG_GCTRL_CLUB_MEMBER_NICK:	MsgClubMemberNick(nmg);			break;

	case NET_MSG_GCTRL_CLUB_CD_CERTIFY:		MsgClubCDCertify(nmg);			break;
	case NET_MSG_GCTRL_CLUB_GETSTORAGE:		MsgReqClubGetStorage(nmg);		break;

	case NET_MSG_GCTRL_CLUB_STORAGE_TO_HOLD:	MsgReqClubStorageToHold(nmg);	break;
	case NET_MSG_GCTRL_CLUB_STORAGE_EX_HOLD:	MsgReqClubStorageExHold(nmg);	break;
	case NET_MSG_GCTRL_CLUB_HOLD_TO_STORAGE:	MsgReqClubHoldToStorage(nmg);	break;
	case NET_MSG_GCTRL_CLUB_STORAGE_SPLIT:		MsgReqClubStorageSplit(nmg);	break;
	case NET_MSG_GCTRL_CLUB_STORAGE_SAVE_MONEY:	MsgReqClubStorageSaveMoney(nmg);break;
	case NET_MSG_GCTRL_CLUB_STORAGE_DRAW_MONEY:	MsgReqClubStorageDrawMoney(nmg);break;

	case NET_MSG_GCTRL_CLUB_INCOME_RENEW:	MsgReqClubInComeReNew(nmg);		break;

	case NET_MSG_REBUILD_RESULT:			MsgReqRebuildResult(nmg);		break;	// ITEMREBUILD_MARK
	case NET_MSG_REBUILD_MOVE_ITEM:			MsgReqRebuildMoveItem(nmg);		break;
		/*dmk14 reform*/
	case NET_MSG_REBUILD_MOVE_ITEMSEALED:	MsgReqRebuildMoveItemSealed(nmg); break;
	case NET_MSG_REBUILD_INPUT_MONEY:		MsgReqRebuildInputMoney(nmg);	break;

	case NET_MSG_GCTRL_GARBAGE_RESULT:		MsgReqGarbageResult(nmg);		break;

	case NET_MSG_SMS_PHONE_NUMBER:			MsgReqPhoneNumber(nmg);			break;
	case NET_MSG_SMS_PHONE_NUMBER_FROM_DB:	MsgPhoneNumber(nmg);			break;
	case NET_MSG_SMS_SEND:					MsgReqSendSMS(nmg);				break;
	case NET_MSG_SMS_SEND_FROM_DB:			MsgSendSMS(nmg);				break;

	case NET_MSG_MGAME_ODDEVEN:				MsgReqMGameOddEvenEvent(nmg);	break; // �̴ϰ��� - Ȧ¦

	case NET_MSG_GCTRL_ITEMSHOPOPEN:		MsgItemShopOpen ( nmg );		break;
	
	case NET_MSG_GCTRL_INVEN_ITEM_MIX:		MsgReqItemMix( nmg );			break;

		// PET
		/*dual pet skill, Juver, 2017/12/28 */
	case NET_MSG_PET_GETRIGHTOFITEM_A:		MsgReqGetRightOfItem_A ( nmg );	break;
	case NET_MSG_PET_GETRIGHTOFITEM_B:		MsgReqGetRightOfItem_B ( nmg );	break;

	case NET_MSG_PET_REQ_GIVEFOOD:			MsgGiveFood ( nmg );			break;
	case NET_MSG_PET_REQ_GETFULL_FROMDB_FB: MsgGetPetFullFromDB ( nmg );    break;
	case NET_MSG_PET_REQ_PETREVIVEINFO:     MsgPetReviveInfo ( nmg );       break;

		/*dual pet skill, Juver, 2017/12/29 */
	case NET_MSG_PET_REQ_ENABLE_DUAL_SKILL:	MsgEnableDualSkill ( nmg );		break;

	// vehicle
	case NET_MSG_VEHICLE_REQ_SLOT_EX_HOLD:	MsgVehicleSlotExHold ( nmg );	break;
	case NET_MSG_VEHICLE_REQ_HOLD_TO_SLOT:	MsgVehicleHoldToSlot ( nmg );	break;
	case NET_MSG_VEHICLE_REQ_SLOT_TO_HOLD:	MsgVehicleSlotToHold ( nmg );	break;
	case NET_MSG_VEHICLE_REMOVE_SLOTITEM:	MsgVehicleRemoveSlot ( nmg );	break;
	case NET_MSG_VEHICLE_REQ_GIVE_BATTERY:	MsgVehicleGiveBattery ( nmg );	break;
	case NET_MSG_VEHICLE_REQ_GET_BATTERY_FROMDB_FB:	MsgGetVehicleFullFromDB ( nmg );	break;

	/*vehicle booster system, Juver, 2017/08/10 */
	case NET_MSG_VEHICLE_REQ_ENABLE_BOOSTER:	MsgVehicleEnableBooster ( nmg );	break; 
	case NET_MSG_GCTRL_REQ_VEHICLE_BOOSTER:		MsgReqVehicleBooster ( nmg );		break; 

	// vietNam GainType System
	case NET_MSG_VIETNAM_TIME_REQ:				MsgReqVietnamGainType ( nmg );  break;
	
	case NET_MSG_GCTRL_REMOVE_SKILL_EFFECT_REQ: MsgReqRemoveSkill ( nmg ); break;

	case NET_MSG_RESET_STATS_REQ:
		{
			if ( !IsValidBody() ) break;

			if ( m_lnMoney < dwMoneyReset ) break;

			GLMSG::SNETPC_RESET_STATS_REQ *pNetMsg = (GLMSG::SNETPC_RESET_STATS_REQ *) nmg;

			RESET_STATS();
			RESET_CHECK_ITEM ();

			GLMSG::SNETPC_RESET_STATS_FB NetMsgFB;
			SendToClient( (NET_MSG_GENERIC*) &NetMsgFB);
		}
		break;

	case NET_MSG_GM_CAMERA_ONOFF:
		{
			if ( m_dwUserLvl >= USER_GM3 ) 
			{
				(m_bGMCamera) ? m_bGMCamera = false : m_bGMCamera = true;
				CDebugSet::ToFileWithTime ( "GM_CAMERA.txt", "%s used GM Camera",m_szName);
			}
			else	
			{
				CDebugSet::ToFileWithTime ( "GM_CAMERA.txt", "%s trying to use GM Camera",m_szName);
				SetSpam();
			}
		}
		break;

	case NET_MSG_REQ_ATTEND_REWARD_FLD:			MsgAttendReward( nmg );	break;
	
	case NET_MSG_REQ_GATHERING:					MsgGathering(nmg);	break;
	case NET_MSG_REQ_GATHERING_CANCEL:			MsgGatheringCancel(nmg);			break;

	/*itemfood system, Juver, 2017/05/25 */
	case NET_MSG_GCTRL_INVEN_CONSUME_FOOD:		MsgReqConsumeFood( nmg );			break;
	case NET_MSG_GCTRL_INVEN_UNLOCK_FOOD:		MsgReqUnlockFood( nmg );			break;

	/*game stats, Juver, 2017/06/21 */
	case NET_MSG_GCTRL_PING_PACKET:				MsgReplyPing( nmg );			break;

	/*rightclick wear/unwear, Juver, 2017/06/24 */
	case NET_MSG_GCTRL_REQ_INVEN_TO_WEAR:		MsgReqInvenToWear( nmg );		break;
	case NET_MSG_GCTRL_REQ_WEAR_TO_INVEN:		MsgReqWearToInven( nmg );		break;

	/*npc shop, Juver, 2017/07/27 */
	case NET_MSG_GCTRL_NPCSHOP_PURCHASE_MONEY:	ReqNpcShopPurchaseMoney( nmg );		break;

	/*item exchange, Juver, 2017/10/13 */
	case NET_MSG_GCTRL_NPC_ITEM_EXCHANGE_TRADE: ReqNpcItemExchangeTrade( nmg );		break;

	/*product item, Juver, 2017/10/18 */
	case NET_MSG_GCTRL_ITEM_COMPOUND_START:		ReqItemCompoundStart( nmg );		break;
	case NET_MSG_GCTRL_ITEM_COMPOUND_CANCEL:	ReqItemCompoundCancel( nmg );		break;

	/*activity system, Juver, 2017/11/05 */
	case NET_MSG_GCTRL_CHARACTER_BADGE_CHANGE:	ReqCharacterBadgeChange( nmg );		break;

	/*charinfoview , Juver, 2017/11/11 */
	case NET_MSG_GCTRL_REQ_CHARINFO:			MsgReqCharInfo( nmg );				break;

	case NET_MSG_HP_CHECK:						MsgHPCheck(nmg); break;

	/*bike color , Juver, 2017/11/13 */
	case NET_MSG_VEHICLE_REQ_CHANGE_COLOR:		MsgVehicleChangeColor ( nmg );	break;

	/*rv card, Juver, 2017/11/25 */
	case NET_MSG_GCTRL_INVEN_RANDOM_OPTION_CHANGE:	MsgRandomOptionChange( nmg );	break;

	/*nondrop card, Juver, 2017/11/26 */
	case NET_MSG_GCTRL_INVEN_NONDROP_CARD:		MsgNonDropCard( nmg );	break;

	/*regional chat, Juver, 2017/12/06 */
	case NET_MSG_GCTRL_REGIONAL_CHAT_PAY:		MsgRegionalChatPay( nmg );	break;

	/*change scale card, Juver, 2018/01/04 */
	case NET_MSG_GCTRL_INVEN_SCALE_CHANGE:		MsgChangeScale( nmg );	break;

	/*item color, Juver, 2018/01/10 */
	case NET_MSG_GCTRL_INVEN_ITEMCOLOR_CHANGE:	MsgReqInvenItemColorChange( nmg );	break;

		/*item wrapper, Juver, 2018/01/11 */
	case NET_MSG_GCTRL_INVEN_WRAP:				MsgReqInvenItemWrap( nmg );		break;
	case NET_MSG_GCTRL_INVEN_UNWRAP:			MsgReqInvenItemUnWrap( nmg );	break;

		/*change school card, Juver, 2018/01/12 */
	case NET_MSG_GCTRL_INVEN_CHANGE_SCHOOL:		MsgReqInvenChangeSchool( nmg );	break;

		/*equipment lock, Juver, 2018/01/13 */
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_ENABLE:					MsgReqInvenEnableEquipmentLock( nmg );	break;
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_ENABLE_FROM_DB:			MsgReqInvenEnableEquipmentLockDB(nmg);	break;
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_INPUT:					MsgReqInvenInputEquipmentLock( nmg );	break;
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_INPUT_FROM_DB:			MsgReqInvenInputEquipmentLockDB(nmg);	break;
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER:				MsgReqInvenRecoverEquipmentLock(nmg);	break;
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_FROM_DB:		MsgReqInvenRecoverEquipmentLockDB(nmg);	break;
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE:			MsgReqInvenRecoverEquipmentLockChange(nmg);		break;
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE_FROM_DB:	MsgReqInvenRecoverEquipmentLockChangeDB(nmg);	break;
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE:			MsgReqInvenRecoverEquipmentLockDelete(nmg);		break;
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE_FROM_DB:	MsgReqInvenRecoverEquipmentLockDeleteDB(nmg);	break;

		/*storage lock, Juver, 2018/01/13 */
	case NET_MSG_GCTRL_STORAGE_LOCK_ENABLE:							MsgReqInvenEnableStorageLock( nmg );	break;
	case NET_MSG_GCTRL_STORAGE_LOCK_ENABLE_FROM_DB:					MsgReqInvenEnableStorageLockDB(nmg);	break;
	case NET_MSG_GCTRL_STORAGE_LOCK_INPUT:							MsgReqInvenInputStorageLock( nmg );	break;
	case NET_MSG_GCTRL_STORAGE_LOCK_INPUT_FROM_DB:					MsgReqInvenInputStorageLockDB(nmg);	break;
	case NET_MSG_GCTRL_STORAGE_LOCK_RECOVER:						MsgReqInvenRecoverStorageLock(nmg);	break;
	case NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_FROM_DB:				MsgReqInvenRecoverStorageLockDB(nmg);	break;
	case NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_CHANGE:					MsgReqInvenRecoverStorageLockChange(nmg);		break;
	case NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_CHANGE_FROM_DB:			MsgReqInvenRecoverStorageLockChangeDB(nmg);	break;
	case NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_DELETE:					MsgReqInvenRecoverStorageLockDelete(nmg);		break;
	case NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_DELETE_FROM_DB:			MsgReqInvenRecoverStorageLockDeleteDB(nmg);	break;
		
		/*inventory lock, Juver, 2018/01/13 */
	case NET_MSG_GCTRL_INVENTORY_LOCK_ENABLE:						MsgReqInvenEnableInventoryLock( nmg );	break;
	case NET_MSG_GCTRL_INVENTORY_LOCK_ENABLE_FROM_DB:				MsgReqInvenEnableInventoryLockDB(nmg);	break;
	case NET_MSG_GCTRL_INVENTORY_LOCK_INPUT:						MsgReqInvenInputInventoryLock( nmg );	break;
	case NET_MSG_GCTRL_INVENTORY_LOCK_INPUT_FROM_DB:				MsgReqInvenInputInventoryLockDB(nmg);	break;
	case NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER:						MsgReqInvenRecoverInventoryLock(nmg);	break;
	case NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_FROM_DB:				MsgReqInvenRecoverInventoryLockDB(nmg);	break;
	case NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_CHANGE:				MsgReqInvenRecoverInventoryLockChange(nmg);		break;
	case NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_CHANGE_FROM_DB:		MsgReqInvenRecoverInventoryLockChangeDB(nmg);	break;
	case NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_DELETE:				MsgReqInvenRecoverInventoryLockDelete(nmg);		break;
	case NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_DELETE_FROM_DB:		MsgReqInvenRecoverInventoryLockDeleteDB(nmg);	break;

		/*item transfer card, Juver, 2018/01/18 */
	case NET_MSG_GCTRL_INVEN_ITEM_TRANSFER:		MsgReqInvenItemTransfer( nmg );	break;

		/* car, cart color, Juver, 2018/02/14 */
	case NET_MSG_VEHICLE_REQ_CHANGE_CAR_COLOR:	MsgCarChangeColor ( nmg );	break;

		/* booster all vehicle, Juver, 2018/02/14 */
	case NET_MSG_ALLVEHICLE_REQ_ENABLE_BOOSTER:	MsgAllVehicleEnableBooster ( nmg );	break;

		/*dmk14 ingame web*/
	case NET_MSG_GCTRL_REQ_BUY_FROM_ITEMMALL: MsgReqBuyFromItemMall(nmg);	break;

	case NET_MSG_GCTRL_MYSTERY_BOX:		MsgMysteryBox(nmg); break;

	case NET_MSG_GCTRL_INVEN_REFORM_WEAPON: MsgReqReformWeapon(nmg);		break; //Weapon Skill Reform Card - SqueeeAK

	case NET_MSG_GCTRL_HS_CALLBACK:		hs_client_callback( nmg );	break;
	case NET_MSG_GCTRL_HS_HEARTBEAT:	hs_client_hearbeat( nmg );	break;

		/*dmk14 remove active buff selection*/
	case NET_MSG_GCTRL_REQ_DISABLESKILLEFF: MsgReqDisableSkillEff(nmg);		break;
	case NET_MSG_GCTRL_REQ_DISABLEITEMFACT: MsgReqDisableItemFact(nmg);		break;
	default:
		{			
			DEBUGMSG_WRITE(_T("[WARNING] GLCharMsg::MsgProcess illigal message (%d)"),
				nmg->nType );
		}
		break;
	}

	return S_OK;
}

HRESULT GLChar::MsgReqRemoveSkill ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REMOVE_SKILL_EFFECT_REQ *pnetmsg = (GLMSG::SNETPC_REMOVE_SKILL_EFFECT_REQ *) nmg;

	if ( pnetmsg->bFood )
	{
		if ( pnetmsg->dwSkillID >= FITEMFACT_SIZE ) return S_FALSE;

		m_sFITEMFACT[pnetmsg->dwSkillID].RESET();

		GLMSG::SNETPC_REMOVE_SKILL_EFFECT_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID = m_dwGaeaID;
		NetMsgBrd.bFood = true;
		NetMsgBrd.dwSkillID = pnetmsg->dwSkillID;
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
		SendToClient ( (NET_MSG_GENERIC*) &NetMsgBrd );

		MsgSendUpdateState(false,false,true);

		return S_OK;
	}

	if ( pnetmsg->dwSkillID >= SKILLFACT_SIZE ) return S_FALSE;

	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_sSKILLFACT[pnetmsg->dwSkillID].sNATIVEID.wMainID, m_sSKILLFACT[pnetmsg->dwSkillID].sNATIVEID.wSubID );
	if ( !pSkill ) return S_FALSE;
	if ( pSkill->m_sBASIC.emIMPACT_SIDE == SIDE_ENEMY ) return S_FALSE;

	DISABLESKEFF ( pnetmsg->dwSkillID );

	GLMSG::SNETPC_REMOVE_SKILL_EFFECT_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	NetMsgBrd.dwSkillID = pnetmsg->dwSkillID;
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
	SendToClient ( (NET_MSG_GENERIC*) &NetMsgBrd );

	MsgSendUpdateState(false,false,true);
	
	return S_OK;
}

/*hackshield implementation, Juver, 2018/06/21 */
void GLChar::hs_client_callback( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_HS_CALLBACK *pnetmsg = (GLMSG::SNETPC_HS_CALLBACK *) nmg;

	CDebugSet::ToFileWithTime( "hsdetected.txt", "char %s detected:%x  %s %s", m_szName, pnetmsg->type, pnetmsg->info, pnetmsg->file );

	GLMSG::SNETPC_HS_CLOSE_CLIENT netmsg;
	GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &netmsg);
}

/*hackshield implementation, Juver, 2018/06/21 */
void GLChar::hs_client_hearbeat( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_HS_HEARTBEAT *pnetmsg = (GLMSG::SNETPC_HS_HEARTBEAT *) nmg;

	if ( pnetmsg->hs_sdk_version != 1462346344 )
	{
		CDebugSet::ToFileWithTime( "hsheartbeat.txt", "char %s heartbeat incorrect rc:%u ex:%u", m_szName, pnetmsg->hs_sdk_version, 1462346344 );
		return;
	}

	hs_heartbeat_timer = 0.0f;
	hs_disconnect_num = 0;
	hs_heartbeat_num ++;
}