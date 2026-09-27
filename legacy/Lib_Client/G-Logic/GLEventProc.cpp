#include "stdafx.h"
#include "./GLEventProc.h"
#include "./GLContrlMsg.h"
#include "./GLogicData.h"
#include "./GLGaeaServer.h"
#include "./GLChar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GLEventProc::GLEventProc(void)
{
}

GLEventProc::~GLEventProc(void)
{
}

void GLEventProc::MsgEventExp ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID )
{
	GLMSG::SNET_GM_EVENT_EXP *pNetMsg = (GLMSG::SNET_GM_EVENT_EXP *) nmg;

	DWORD dwAgentSlot = GLGaeaServer::GetInstance().GetAgentSlot();
	if ( dwAgentSlot != dwClientID )		return;

	PGLCHAR pGMChar = GLGaeaServer::GetInstance().GetCharID( pNetMsg->dwCHARID );
	if ( !pGMChar )	return;

	GLMSG::SNET_GM_EVENT_EXP_FB NetMsgFB;

	if ( pNetMsg->fRATE > 10.0f || pNetMsg->fRATE < 0.0f )
	{
		NetMsgFB.bFAIL = TRUE;
		GLGaeaServer::GetInstance().SENDTOCLIENT( pGMChar->m_dwClientID, &NetMsgFB );
		return;
	}

	GLCONST_CHAR::fEXP_SCALE = pNetMsg->fRATE;

	NetMsgFB.bFAIL = FALSE;
	NetMsgFB.fRATE = GLCONST_CHAR::fEXP_SCALE;
	GLGaeaServer::GetInstance().SENDTOCLIENT( pGMChar->m_dwClientID, &NetMsgFB );
}

void GLEventProc::MsgEventExpEnd ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID )
{
	GLMSG::SNET_GM_EVENT_EXP_END *pNetMsg = (GLMSG::SNET_GM_EVENT_EXP_END *) nmg;

	DWORD dwAgentSlot = GLGaeaServer::GetInstance().GetAgentSlot();
	if ( dwAgentSlot != dwClientID )		return;

	PGLCHAR pGMChar = GLGaeaServer::GetInstance().GetCharID( pNetMsg->dwCHARID );
	if ( !pGMChar )	return;

	GLCONST_CHAR::fEXP_SCALE = GLCONST_CHAR::fEXP_SCALE_BACKUP;

	GLMSG::SNET_GM_EVENT_EXP_END_FB NetMsgFB;
	NetMsgFB.fRATE = GLCONST_CHAR::fEXP_SCALE;
	GLGaeaServer::GetInstance().SENDTOCLIENT( pGMChar->m_dwClientID, &NetMsgFB );
}

void GLEventProc::MsgEventItemGen ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID )
{
	GLMSG::SNET_GM_EVENT_ITEM_GEN *pNetMsg = (GLMSG::SNET_GM_EVENT_ITEM_GEN *) nmg;

	DWORD dwAgentSlot = GLGaeaServer::GetInstance().GetAgentSlot();
	if ( dwAgentSlot != dwClientID )		return;

	PGLCHAR pGMChar = GLGaeaServer::GetInstance().GetCharID( pNetMsg->dwCHARID );
	if ( !pGMChar )	return;

	GLMSG::SNET_GM_EVENT_ITEM_GEN_FB NetMsgFB;

	if ( pNetMsg->fRATE > 10.0f || pNetMsg->fRATE < 0.0f )
	{
		NetMsgFB.bFAIL = TRUE;
		GLGaeaServer::GetInstance().SENDTOCLIENT( pGMChar->m_dwClientID, &NetMsgFB );
		return;
	}

	GLCONST_CHAR::fITEM_DROP_SCALE = pNetMsg->fRATE;

	NetMsgFB.bFAIL = FALSE;
	NetMsgFB.fRATE = GLCONST_CHAR::fITEM_DROP_SCALE;
	GLGaeaServer::GetInstance().SENDTOCLIENT( pGMChar->m_dwClientID, &NetMsgFB );
}

void GLEventProc::MsgEventItemGenEnd ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID )
{
	GLMSG::SNET_GM_EVENT_ITEM_GEN_END *pNetMsg = (GLMSG::SNET_GM_EVENT_ITEM_GEN_END *) nmg;

	DWORD dwAgentSlot = GLGaeaServer::GetInstance().GetAgentSlot();
	if ( dwAgentSlot != dwClientID )		return;

	PGLCHAR pGMChar = GLGaeaServer::GetInstance().GetCharID( pNetMsg->dwCHARID );
	if ( !pGMChar )	return;

	GLCONST_CHAR::fITEM_DROP_SCALE = GLCONST_CHAR::fITEM_DROP_SCALE_BACKUP;

	GLMSG::SNET_GM_EVENT_ITEM_GEN_END_FB NetMsgFB;
	NetMsgFB.fRATE = GLCONST_CHAR::fITEM_DROP_SCALE;
	GLGaeaServer::GetInstance().SENDTOCLIENT( pGMChar->m_dwClientID, &NetMsgFB );
}

void GLEventProc::MsgEventMoneyGen ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID )
{
	GLMSG::SNET_GM_EVENT_MONEY_GEN *pNetMsg = (GLMSG::SNET_GM_EVENT_MONEY_GEN *) nmg;

	DWORD dwAgentSlot = GLGaeaServer::GetInstance().GetAgentSlot();
	if ( dwAgentSlot != dwClientID )		return;

	PGLCHAR pGMChar = GLGaeaServer::GetInstance().GetCharID( pNetMsg->dwCHARID );
	if ( !pGMChar )	return;

	GLMSG::SNET_GM_EVENT_MONEY_GEN_FB NetMsgFB;

	if ( pNetMsg->fRATE > 10.0f || pNetMsg->fRATE < 0.0f )
	{
		NetMsgFB.bFAIL = TRUE;
		GLGaeaServer::GetInstance().SENDTOCLIENT( pGMChar->m_dwClientID, &NetMsgFB );
		return;
	}

	GLCONST_CHAR::fMONEY_DROP_SCALE = pNetMsg->fRATE;

	NetMsgFB.bFAIL = FALSE;
	NetMsgFB.fRATE = GLCONST_CHAR::fMONEY_DROP_SCALE;
	GLGaeaServer::GetInstance().SENDTOCLIENT( pGMChar->m_dwClientID, &NetMsgFB );
}

void GLEventProc::MsgEventMoneyGenEnd ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID )
{
	GLMSG::SNET_GM_EVENT_MONEY_GEN_END *pNetMsg = (GLMSG::SNET_GM_EVENT_MONEY_GEN_END *) nmg;

	DWORD dwAgentSlot = GLGaeaServer::GetInstance().GetAgentSlot();
	if ( dwAgentSlot != dwClientID )		return;

	PGLCHAR pGMChar = GLGaeaServer::GetInstance().GetCharID( pNetMsg->dwCHARID );
	if ( !pGMChar )	return;

	GLCONST_CHAR::fMONEY_DROP_SCALE = GLCONST_CHAR::fMONEY_DROP_SCALE_BACKUP;

	GLMSG::SNET_GM_EVENT_MONEY_GEN_END_FB NetMsgFB;
	NetMsgFB.fRATE = GLCONST_CHAR::fMONEY_DROP_SCALE;
	GLGaeaServer::GetInstance().SENDTOCLIENT( pGMChar->m_dwClientID, &NetMsgFB );
}

void GLEventProc::MsgProcess ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID )
{
	switch ( nmg->nType )
	{
	case NET_MSG_GM_EVENT_EXP:				MsgEventExp ( nmg, dwClientID, dwGaeaID );			break;
	case NET_MSG_GM_EVENT_EXP_END:			MsgEventExpEnd ( nmg, dwClientID, dwGaeaID  );		break;
	case NET_MSG_GM_EVENT_ITEM_GEN:			MsgEventItemGen ( nmg, dwClientID, dwGaeaID  );		break;
	case NET_MSG_GM_EVENT_ITEM_GEN_END:		MsgEventItemGenEnd ( nmg, dwClientID, dwGaeaID  );	break;
	case NET_MSG_GM_EVENT_MONEY_GEN:		MsgEventMoneyGen ( nmg, dwClientID, dwGaeaID  );	break;
	case NET_MSG_GM_EVENT_MONEY_GEN_END:	MsgEventMoneyGenEnd ( nmg, dwClientID, dwGaeaID  );	break;
	default: break;
	};
}
