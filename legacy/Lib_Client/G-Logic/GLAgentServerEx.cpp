#include "stdafx.h"
#include "./GLAgentServer.h"

/*pvp tyranny, Juver, 2017/08/25 */
#include "GLPVPTyrannyAgent.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*event map move, Juver, 2017/08/25 */
BOOL GLAgentServer::MsgGMMove2_MapPos( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID )
{
	GLMSG::SNETPC_GM_MOVE2_MAPPOS* pNetMsg = (GLMSG::SNETPC_GM_MOVE2_MAPPOS*)nmg;

	GLMSG::SNETPC_REQ_EVENT_MOVEMAP_FB NetMsgFB;

	PGLCHARAG pMyChar = GetChar ( dwGaeaID );
	if ( !pMyChar )							return FALSE;

	if ( pMyChar->m_dwUserLvl < USER_GM3 )
	{
		NetMsgFB.emFB = EMEVENT_MOVEMAP_FB_FAIL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	GLMSG::SNETPC_REQ_EVENT_MOVEMAP_FLD net_msg_field;
	net_msg_field.nidMAP = pNetMsg->nidMAP;
	net_msg_field.wPosX = pNetMsg->wPosX;
	net_msg_field.wPosY = pNetMsg->wPosY;
	net_msg_field.bCurMapCheck = FALSE;
	net_msg_field.bCalculatePos = TRUE;
	net_msg_field.dwCharID = pMyChar->m_dwCharID;
	SENDTOALLCHANNEL ( &net_msg_field );

	return TRUE;
}

/*pvp tyranny, Juver, 2017/08/25 */
void GLAgentServer::TyrannyResetBuffs()
{
	int nCnt = 0;
	AGCHARNODE *pCharNode = m_PCList.m_pHead;
	for ( ; pCharNode; pCharNode=pCharNode->pNext )
	{
		PGLCHARAG pCharAG = pCharNode->Data;
		if( !pCharAG ) continue;

		GLMSG::SNETPC_TYRANNY_A2F_BUFF_RESET netmsgbuff;
		netmsgbuff.dwCharID = pCharAG->m_dwCharID;

		SENDTOFIELDSVR ( pCharAG->m_nChannel, pCharAG->m_dwCurFieldSvr, &netmsgbuff );

		nCnt ++;
	}

	CONSOLEMSG_WRITE( "Tyranny reset buffs to :%d players", nCnt );
}

/*pvp tyranny, Juver, 2017/08/25 */
void GLAgentServer::TyrannySetRewardBuffs()
{
	int nCnt = 0;

	GLMSG::SNETPC_TYRANNY_A2F_BUFF_REWARD netmsgbuff;
	netmsgbuff.sRewardBuffMain = GLPVPTyrannyAgent::GetInstance().m_sRewardBuffMain;

	for ( int i=0; i<TYRANNY_REWARD_BUFF_NUM; ++i )
		netmsgbuff.sRewardBuff[i] = GLPVPTyrannyAgent::GetInstance().m_sRewardBuff[i];

	WORD wLastWinner = GLPVPTyrannyAgent::GetInstance().m_wLastWinner;

	if ( wLastWinner != TYRANNY_SCHOOL_SIZE )
	{
		AGCHARNODE *pCharNode = m_PCList.m_pHead;
		for ( ; pCharNode; pCharNode=pCharNode->pNext )
		{
			PGLCHARAG pCharAG = pCharNode->Data;
			if( !pCharAG )							continue;
			if( pCharAG->m_wSchool != wLastWinner )	continue;

			netmsgbuff.dwCharID = pCharAG->m_dwCharID;
			GLAgentServer::GetInstance().SENDTOFIELDSVR ( pCharAG->m_nChannel, pCharAG->m_dwCurFieldSvr, &netmsgbuff );

			nCnt ++;
		}
	}
	
	CONSOLEMSG_WRITE( "Tyranny set reward buffs to :%d players", nCnt );
}

/*pvp capture the flag, Juver, 2018/01/30 */
void GLAgentServer::CaptureTheFlagRemoveParty( DWORD dwCharID )
{
	PGLCHARAG pchar = GetCharID( dwCharID );
	if ( !pchar )	return;
	
	GLPARTY* pparty = m_cPartyMan.GetParty ( pchar->m_dwPartyID );
	if ( !pparty )	return;
	if ( !pparty->ISMEMBER( pchar->m_dwGaeaID) )	return;

	if ( pparty->ISMASTER( pchar->m_dwGaeaID ) )
	{
		m_cPartyMan.DelParty ( pchar->m_dwPartyID );
	}
	else
	{
		GLMSG::SNET_PARTY_DEL NetMsgDel;
		NetMsgDel.dwPartyID = pchar->m_dwPartyID;
		NetMsgDel.dwDelMember = pchar->m_dwGaeaID;
		SENDTOPARTY ( pchar->m_dwPartyID, (NET_MSG_GENERIC*) &NetMsgDel );
		SENDTOALLCHANNEL ( &NetMsgDel );

		pchar->ReSetPartyID ();
		pparty->DELMEMBER ( pchar->m_dwGaeaID );
	}
}
