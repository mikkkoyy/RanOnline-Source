#include "StdAfx.h"
#include "GLPVPCaptureTheFlagClient.h"

#include "GLGaeaClient.h"
#include "DxGlobalStage.h"

#include "InnerInterface.h"
#include "UITextControl.h"
#include "GameTextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GLPVPCaptureTheFlagClient& GLPVPCaptureTheFlagClient::GetInstance()
{
	static GLPVPCaptureTheFlagClient cInstance;
	return cInstance;
}

bool GLPVPCaptureTheFlagClient::FrameMove ( float fTime, float fElapsedTime  )
{
	if( m_emState == CAPTURE_THE_FLAG_STATE_REGISTER ||
		m_emState == CAPTURE_THE_FLAG_STATE_BATTLE ||
		m_emState == CAPTURE_THE_FLAG_STATE_REWARD ){
		if ( m_fRemain > 0.0f ){
			m_fRemain -= fElapsedTime;
		}
	}

	if ( m_emState == CAPTURE_THE_FLAG_STATE_BATTLE )
	{
	}

	return true;
}

void GLPVPCaptureTheFlagClient::SetEventState( CAPTURE_THE_FLAG_STATE emState )
{
	switch( emState )
	{
	case CAPTURE_THE_FLAG_STATE_ENDED:
		{
			DoStateEnd();
		}break;

	case CAPTURE_THE_FLAG_STATE_REGISTER:
		{
			DoStateRegister();
		}break;

	case CAPTURE_THE_FLAG_STATE_BATTLE:
		{
			DoStateBattle();
		}break;

	case CAPTURE_THE_FLAG_STATE_REWARD:
		{
			DoStateReward();
		}break;
	};

	m_emState = emState;
}

void GLPVPCaptureTheFlagClient::DoStateRegister()
{
	CInnerInterface::GetInstance().PrintConsoleText( ID2GAMEINTEXT("PVP_CAPTURE_THE_FLAG_REG") );
	CInnerInterface::GetInstance().ShowExtraNotice( NS_UITEXTCOLOR::WHITE, ID2GAMEINTEXT("PVP_CAPTURE_THE_FLAG_REG") );
}

void GLPVPCaptureTheFlagClient::DoStateBattle()
{
	CInnerInterface::GetInstance().PrintConsoleText( ID2GAMEINTEXT("PVP_CAPTURE_THE_FLAG_BATTLE") );
	CInnerInterface::GetInstance().ShowExtraNotice( NS_UITEXTCOLOR::WHITE, ID2GAMEINTEXT("PVP_CAPTURE_THE_FLAG_BATTLE") );

	ResetScore();

	m_wLastWinner = CAPTURE_THE_FLAG_TEAM_SIZE;

	for( int i=0; i<CAPTURE_THE_FLAG_MINI_RANKING_NUM; ++i )
		m_sLastTop[i] = CAPTURE_THE_FLAG_PLAYER_DATA();
}

void GLPVPCaptureTheFlagClient::DoStateReward()
{
	CInnerInterface::GetInstance().PrintConsoleText( ID2GAMEINTEXT("PVP_CAPTURE_THE_FLAG_REWARD") );
	CInnerInterface::GetInstance().ShowExtraNotice( NS_UITEXTCOLOR::WHITE, ID2GAMEINTEXT("PVP_CAPTURE_THE_FLAG_REWARD") );
}

void GLPVPCaptureTheFlagClient::DoStateEnd()
{
	CInnerInterface::GetInstance().PrintConsoleText( ID2GAMEINTEXT("PVP_CAPTURE_THE_FLAG_ENDED") );
	CInnerInterface::GetInstance().ShowExtraNotice( NS_UITEXTCOLOR::WHITE, ID2GAMEINTEXT("PVP_CAPTURE_THE_FLAG_ENDED") );

	m_fRemain = 0.0f;
	m_bRegistered = false;
	m_wTeam = CAPTURE_THE_FLAG_TEAM_SIZE;
	m_bShowStartNotice = FALSE;

	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		m_wPlayerNum[i] = 0;

	ResetScore();
}

void GLPVPCaptureTheFlagClient::NotifyTimeToStart( WORD wTime )
{
	if ( wTime > 0 ){
		if ( wTime <= 1 )
		{
			CInnerInterface::GetInstance().PrintConsoleText( ID2GAMEINTEXT("PVP_CAPTURE_THE_FLAG_TOBATTLE2"), wTime );
			CInnerInterface::GetInstance().ShowExtraNotice( NS_UITEXTCOLOR::WHITE, ID2GAMEINTEXT("PVP_CAPTURE_THE_FLAG_TOBATTLE2"), wTime );
		}
		else
		{
			CInnerInterface::GetInstance().PrintConsoleText( ID2GAMEINTEXT("PVP_CAPTURE_THE_FLAG_TOBATTLE"), wTime );
			CInnerInterface::GetInstance().ShowExtraNotice( NS_UITEXTCOLOR::WHITE, ID2GAMEINTEXT("PVP_CAPTURE_THE_FLAG_TOBATTLE"), wTime );
		}
	
	}
}
void GLPVPCaptureTheFlagClient::DoRegister( WORD wTeam )
{
	if ( m_emState != CAPTURE_THE_FLAG_STATE_REGISTER && m_emState != CAPTURE_THE_FLAG_STATE_BATTLE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_NOTPOSSIBLE") );
		return;
	}

	if( DxGlobalStage::GetInstance().GetChannel() != 0 ){
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_INVALID_CHANNEL") );
		return;
	}

	if ( m_emState == CAPTURE_THE_FLAG_STATE_BATTLE && !m_bRegistered )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_NOTPOSSIBLE") );
		return;
	}

	if ( wTeam >= CAPTURE_THE_FLAG_TEAM_SIZE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_INVALID_TEAM") );
		return;
	}

	if ( m_emState == CAPTURE_THE_FLAG_STATE_REGISTER )
	{
		if ( m_bRegistered && m_wTeam != wTeam )
		{
			CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_OTHER_TEAM") );
			return;
		}

		GLMSG::SNETPC_CAPTURE_THE_FLAG_C2A_REGISTER_REQ netmsgregister;
		netmsgregister.bRegister = !m_bRegistered;
		netmsgregister.wLevel = GLGaeaClient::GetInstance().GetCharacter()->m_wLevel;
		netmsgregister.llContributionReq = GLGaeaClient::GetInstance().GetCharacter()->m_llContributionPoint;
		netmsgregister.dwCharID = GLGaeaClient::GetInstance().GetCharacter()->m_dwCharID;
		netmsgregister.wTeam = wTeam;
		NETSEND ( &netmsgregister );
	}

	if ( m_emState == CAPTURE_THE_FLAG_STATE_BATTLE && m_bRegistered )
	{
		if ( m_wTeam != wTeam )
		{
			CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REJOIN_FB_OTHER_TEAM") );
			return;
		}

		if ( GLGaeaClient::GetInstance().GetActiveMap()->m_bPVPCaptureTheFlagMap )
		{
			CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REJOIN_FB_INBATTLE") );
			return;
		}

		GLMSG::SNETPC_CAPTURE_THE_FLAG_C2A_REJOIN_REQ netmsgrejoin;
		netmsgrejoin.dwCharID = GLGaeaClient::GetInstance().GetCharacter()->m_dwCharID;
		NETSEND ( &netmsgrejoin );
	}
}

void GLPVPCaptureTheFlagClient::DoRevive()
{
	if ( !GLGaeaClient::GetInstance().GetCharacter()->IsDie () )
	{
		return;
	}

	if ( m_emState != CAPTURE_THE_FLAG_STATE_BATTLE && m_emState != CAPTURE_THE_FLAG_STATE_REWARD )
	{
		GLGaeaClient::GetInstance().GetCharacter()->ReqReBirth();
		CInnerInterface::GetInstance().CloseAllWindow();
		return;
	}

	CInnerInterface::GetInstance().CloseAllWindow();
	GLGaeaClient::GetInstance().GetCharacter()->SetSTATE(EM_ACT_DIE);

	GLMSG::SNETPC_CAPTURE_THE_FLAG_C2F_REVIVE_REQ netmsgrevive;
	netmsgrevive.dwCharID = GLGaeaClient::GetInstance().GetCharacter()->m_dwCharID;
	NETSENDTOFIELD ( &netmsgrevive );
}

void GLPVPCaptureTheFlagClient::RequestScoreInfo()
{
	PLANDMANCLIENT plandclient = GLGaeaClient::GetInstance().GetActiveMap();
	if ( !plandclient )	return;

	if ( !plandclient->m_bPVPCaptureTheFlagMap )	return;

	GLMSG::SNETPC_CAPTURE_THE_FLAG_C2A_SCORE_INFO_REQ netmsginforeq;
	netmsginforeq.dwCharID = GLGaeaClient::GetInstance().GetCharacter()->m_dwCharID;
	NETSEND ( &netmsginforeq );
}

void GLPVPCaptureTheFlagClient::ResetScore()
{
	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
	{
		m_sScore[i].wTeam = i;
		m_sScore[i].Reset();
	}

	m_vecRankingAll.clear();
	m_vecRankingA.clear();
	m_vecRankingB.clear();

	for ( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
	{
		m_bFlagHold[i] = FALSE;
		m_sFlagTarget[i].RESET();
	}
}

CAPTURE_THE_FLAG_PLAYER_DATA* GLPVPCaptureTheFlagClient::GetOwnRanking()
{
	if ( m_vecRankingAll.empty() )	return NULL;

	DWORD dwcharid = GLGaeaClient::GetInstance().GetCharacter()->m_dwCharID;

	for( size_t i=0; i<m_vecRankingAll.size(); ++i )
	{
		CAPTURE_THE_FLAG_PLAYER_DATA& sranking = m_vecRankingAll[i];
		if ( dwcharid == sranking.dwInfoCharID )
		{
			return &sranking;
			break;
		}
	}

	return NULL;
}

int GLPVPCaptureTheFlagClient::GetRankingSize( int nType )
{
	switch( nType )
	{
	case CAPTURE_THE_FLAG_TEAM_A:
		return (int)m_vecRankingA.size();
		break;

	case CAPTURE_THE_FLAG_TEAM_B:
		return (int)m_vecRankingB.size();
		break;
	};

	return (int)m_vecRankingAll.size();
}

CAPTURE_THE_FLAG_PLAYER_DATA_VEC& GLPVPCaptureTheFlagClient::GetRankingVector( int nType )
{
	switch( nType )
	{
	case CAPTURE_THE_FLAG_TEAM_A:
		return m_vecRankingA;
		break;

	case CAPTURE_THE_FLAG_TEAM_B:
		return m_vecRankingB;
		break;
	};

	return m_vecRankingAll;
}

void GLPVPCaptureTheFlagClient::RequestMapExit()
{
	GLMSG::SNETPC_CAPTURE_THE_FLAG_C2A_MAPEXIT_REQ netmsgexit;
	netmsgexit.dwCharID = GLGaeaClient::GetInstance().GetCharacter()->m_dwCharID;
	NETSEND ( &netmsgexit );
}



