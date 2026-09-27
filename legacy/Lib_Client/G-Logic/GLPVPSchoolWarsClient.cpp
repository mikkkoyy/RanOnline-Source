#include "StdAfx.h"
#include "GLPVPSchoolWarsClient.h"

#include "GLGaeaClient.h"
#include "DxGlobalStage.h"

#include "InnerInterface.h"
#include "UITextControl.h"
#include "GameTextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GLPVPSchoolWarsClient& GLPVPSchoolWarsClient::GetInstance()
{
	static GLPVPSchoolWarsClient cInstance;
	return cInstance;
}

bool GLPVPSchoolWarsClient::FrameMove ( float fTime, float fElapsedTime  )
{
	if( m_emState == SCHOOLWARS_STATE_REGISTER ||
		m_emState == SCHOOLWARS_STATE_BATTLE ||
		m_emState == SCHOOLWARS_STATE_REWARD ){
		if ( m_fRemain > 0.0f ){
			m_fRemain -= fElapsedTime;
		}
	}

	if ( IsBattle() )
	{
		if ( m_bDoublePoint )
		{
			m_fDoublePointInterval -= fElapsedTime;
			if ( m_fDoublePointInterval <= 0.0f )
			{
				m_bDoublePoint = false;
				m_fDoublePoint = 0.0f;

				if ( GLGaeaClient::GetInstance().GetActiveMap()->m_bPVPSchoolWarsMap )
				{
					CInnerInterface::GetInstance().PrintConsoleText( ID2GAMEINTEXT("PVP_SCHOOLWARS_DOUBLE_POINT_OFF") );
					CInnerInterface::GetInstance().ShowExtraNotice( NS_UITEXTCOLOR::WHITE, ID2GAMEINTEXT("PVP_SCHOOLWARS_DOUBLE_POINT_OFF") );
				}
			}
		}
		else
		{
			m_fDoublePoint += fElapsedTime;
			if ( m_fDoublePoint >= SCHOOLWARS_DOUBLE_POINT_TIME_INTERVAL )
			{
				m_bDoublePoint = true;
				m_fDoublePointInterval = SCHOOLWARS_DOUBLE_POINT_TIME_DURATION;

				if ( GLGaeaClient::GetInstance().GetActiveMap()->m_bPVPSchoolWarsMap )
				{
					CInnerInterface::GetInstance().PrintConsoleText( ID2GAMEINTEXT("PVP_SCHOOLWARS_DOUBLE_POINT_ON") );
					CInnerInterface::GetInstance().ShowExtraNotice( NS_UITEXTCOLOR::WHITE, ID2GAMEINTEXT("PVP_SCHOOLWARS_DOUBLE_POINT_ON") );
				}
			}
		}
	}

	return true;
}

void GLPVPSchoolWarsClient::SetEventState( SCHOOLWARS_STATE emState )
{
	switch( emState )
	{
	case SCHOOLWARS_STATE_ENDED:
		{
			DoStateEnd();
		}break;

	case SCHOOLWARS_STATE_REGISTER:
		{
			DoStateRegister();
		}break;

	case SCHOOLWARS_STATE_BATTLE:
		{
			DoStateBattle();
		}break;

	case SCHOOLWARS_STATE_REWARD:
		{
			DoStateReward();
		}break;
	};

	m_emState = emState;
}

void GLPVPSchoolWarsClient::DoStateRegister()
{
	CInnerInterface::GetInstance().PrintConsoleText( ID2GAMEINTEXT("PVP_SCHOOLWARS_REG") );
	CInnerInterface::GetInstance().ShowExtraNotice( NS_UITEXTCOLOR::WHITE, ID2GAMEINTEXT("PVP_SCHOOLWARS_REG") );
}

void GLPVPSchoolWarsClient::DoStateBattle()
{
	CInnerInterface::GetInstance().PrintConsoleText( ID2GAMEINTEXT("PVP_SCHOOLWARS_BATTLE") );
	CInnerInterface::GetInstance().ShowExtraNotice( NS_UITEXTCOLOR::WHITE, ID2GAMEINTEXT("PVP_SCHOOLWARS_BATTLE") );

	ResetScore();

	m_wLastWinner = SCHOOLWARS_SCHOOL_SIZE;

	for( int i=0; i<SCHOOLWARS_MINI_RANKING_NUM; ++i )
		m_sLastTop[i] = SCHOOLWARS_PLAYER_DATA();
}

void GLPVPSchoolWarsClient::DoStateReward()
{
	CInnerInterface::GetInstance().PrintConsoleText( ID2GAMEINTEXT("PVP_SCHOOLWARS_REWARD") );
	CInnerInterface::GetInstance().ShowExtraNotice( NS_UITEXTCOLOR::WHITE, ID2GAMEINTEXT("PVP_SCHOOLWARS_REWARD") );
}

void GLPVPSchoolWarsClient::DoStateEnd()
{
	CInnerInterface::GetInstance().PrintConsoleText( ID2GAMEINTEXT("PVP_SCHOOLWARS_ENDED") );
	CInnerInterface::GetInstance().ShowExtraNotice( NS_UITEXTCOLOR::WHITE, ID2GAMEINTEXT("PVP_SCHOOLWARS_ENDED") );

	m_fRemain = 0.0f;
	m_bRegistered = false;
	m_bQueued = false;
	m_wQueueNum = 0;
	m_bShowStartNotice = FALSE;

	ResetScore();
}

void GLPVPSchoolWarsClient::NotifyTimeToStart( WORD wTime )
{
	if ( wTime > 0 ){
		CInnerInterface::GetInstance().PrintConsoleText( ID2GAMEINTEXT("PVP_SCHOOLWARS_TOBATTLE"), wTime );
		CInnerInterface::GetInstance().ShowExtraNotice( NS_UITEXTCOLOR::WHITE, ID2GAMEINTEXT("PVP_SCHOOLWARS_TOBATTLE"), wTime );
	}
}

void GLPVPSchoolWarsClient::DoRegister()
{
	if ( m_emState != SCHOOLWARS_STATE_REGISTER && m_emState != SCHOOLWARS_STATE_BATTLE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_NOTPOSSIBLE") );
		return;
	}

	if( DxGlobalStage::GetInstance().GetChannel() != 0 ){
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_INVALID_CHANNEL") );
		return;
	}

	/*if ( m_emState == SCHOOLWARS_STATE_BATTLE && !m_bRegistered )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_NOTPOSSIBLE") );
		return;
	}*/

	if ( /*m_emState == SCHOOLWARS_STATE_REGISTER */!m_bRegistered)
	{
		GLMSG::SNETPC_SCHOOLWARS_C2A_REGISTER_REQ netmsgregister;
		netmsgregister.bRegister = !m_bRegistered;
		netmsgregister.wLevel = GLGaeaClient::GetInstance().GetCharacter()->m_wLevel;
		netmsgregister.llContri = GLGaeaClient::GetInstance().GetCharacter()->m_llContributionPoint;
		netmsgregister.dwCharID = GLGaeaClient::GetInstance().GetCharacter()->m_dwCharID;
		NETSEND ( &netmsgregister );
	}

	if ( m_emState == SCHOOLWARS_STATE_BATTLE && m_bRegistered )
	{
		if ( GLGaeaClient::GetInstance().GetActiveMap()->m_bPVPSchoolWarsMap )
		{
			CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("SCHOOLWARS_REJOIN_FB_INBATTLE") );
			return;
		}

		GLMSG::SNETPC_SCHOOLWARS_C2A_REJOIN_REQ netmsgrejoin;
		netmsgrejoin.dwCharID = GLGaeaClient::GetInstance().GetCharacter()->m_dwCharID;
		NETSEND ( &netmsgrejoin );
	}
}

void GLPVPSchoolWarsClient::DoRevive()
{
	if ( !GLGaeaClient::GetInstance().GetCharacter()->IsDie () )
	{
		return;
	}

	if ( m_emState != SCHOOLWARS_STATE_BATTLE && m_emState != SCHOOLWARS_STATE_REWARD )
	{
		GLGaeaClient::GetInstance().GetCharacter()->ReqReBirth();
		CInnerInterface::GetInstance().CloseAllWindow();
		return;
	}

	CInnerInterface::GetInstance().CloseAllWindow();
	GLGaeaClient::GetInstance().GetCharacter()->SetSTATE(EM_ACT_DIE);

	GLMSG::SNETPC_SCHOOLWARS_C2F_REVIVE_REQ netmsgrevive;
	netmsgrevive.dwCharID = GLGaeaClient::GetInstance().GetCharacter()->m_dwCharID;
	NETSENDTOFIELD ( &netmsgrevive );
}

void GLPVPSchoolWarsClient::RequestScoreInfo()
{
	PLANDMANCLIENT plandclient = GLGaeaClient::GetInstance().GetActiveMap();
	if ( !plandclient )	return;

	if ( !plandclient->m_bPVPSchoolWarsMap )	return;

	GLMSG::SNETPC_SCHOOLWARS_C2A_SCORE_INFO_REQ netmsginforeq;
	netmsginforeq.dwCharID = GLGaeaClient::GetInstance().GetCharacter()->m_dwCharID;
	NETSEND ( &netmsginforeq );
}

void GLPVPSchoolWarsClient::ResetScore()
{
	for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++i )
	{
		m_sScore[i].wSchool = i;
		m_sScore[i].Reset();
	}

	m_vecRankingAll.clear();
	m_vecRankingSG.clear();
	m_vecRankingMP.clear();
	m_vecRankingPHX.clear();

	m_fDoublePoint = 0.0f;
	m_fDoublePointInterval  = 0.0f;
	m_bDoublePoint = false;
}

SCHOOLWARS_PLAYER_DATA* GLPVPSchoolWarsClient::GetOwnRanking()
{
	if ( m_vecRankingAll.empty() )	return NULL;

	DWORD dwcharid = GLGaeaClient::GetInstance().GetCharacter()->m_dwCharID;

	for( size_t i=0; i<m_vecRankingAll.size(); ++i )
	{
		SCHOOLWARS_PLAYER_DATA& sranking = m_vecRankingAll[i];
		if ( dwcharid == sranking.dwInfoCharID )
		{
			return &sranking;
			break;
		}
	}

	return NULL;
}

int GLPVPSchoolWarsClient::GetRankingSize( int nType )
{
	switch( nType )
	{
	case SCHOOLWARS_SCHOOL_SG:
		return (int)m_vecRankingSG.size();
		break;

	case SCHOOLWARS_SCHOOL_MP:
		return (int)m_vecRankingMP.size();
		break;

	case SCHOOLWARS_SCHOOL_PHX:
		return (int)m_vecRankingPHX.size();
		break;
	};

	return (int)m_vecRankingAll.size();
}

SCHOOLWARS_PLAYER_DATA_VEC& GLPVPSchoolWarsClient::GetRankingVector( int nType )
{
	switch( nType )
	{
	case SCHOOLWARS_SCHOOL_SG:
		return m_vecRankingSG;
		break;

	case SCHOOLWARS_SCHOOL_MP:
		return m_vecRankingMP;
		break;

	case SCHOOLWARS_SCHOOL_PHX:
		return m_vecRankingPHX;
		break;
	};

	return m_vecRankingAll;
}

void GLPVPSchoolWarsClient::RequestMapExit()
{
	GLMSG::SNETPC_SCHOOLWARS_C2A_MAPEXIT_REQ netmsgexit;
	netmsgexit.dwCharID = GLGaeaClient::GetInstance().GetCharacter()->m_dwCharID;
	NETSEND ( &netmsgexit );
}




