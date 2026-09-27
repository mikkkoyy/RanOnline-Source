#include "StdAfx.h"
#include "GLPVPCaptureTheFlagClient.h"
#include "GLGaeaClient.h"
#include "InnerInterface.h"
#include "GameTextControl.h"
#include "UITextControl.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HRESULT GLPVPCaptureTheFlagClient::MsgProcess ( NET_MSG_GENERIC* nmg )
{
	if ( !nmg ){
		return E_FAIL;
	}

	switch ( nmg->nType )
	{
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_STATE_REGISTER:
		{
			GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_REGISTER* pnetmsg = ( GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_REGISTER* ) nmg;
			SetEventState( CAPTURE_THE_FLAG_STATE_REGISTER );
			m_fRemain = pnetmsg->fTime;

			CInnerInterface::GetInstance().SetCompetitionButtonAlarm( TRUE );

		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_STATE_BATTLE:
		{
			GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_BATTLE* pnetmsg = ( GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_BATTLE* ) nmg;
			SetEventState( CAPTURE_THE_FLAG_STATE_BATTLE );
			m_fRemain = pnetmsg->fTime;
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_STATE_REWARD:
		{
			GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_REWARD* pnetmsg = ( GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_REWARD* ) nmg;
			SetEventState( CAPTURE_THE_FLAG_STATE_REWARD );
			m_fRemain = pnetmsg->fTime;
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_STATE_ENDED:
		{
			GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_ENDED* pnetmsg = ( GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_ENDED* ) nmg;
			SetEventState( CAPTURE_THE_FLAG_STATE_ENDED );
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2C_TOBATTLE_TIME:
		{
			GLMSG::SNET_CAPTURE_THE_FLAG_A2C_TOBATTLE_TIME* pnetmsg = ( GLMSG::SNET_CAPTURE_THE_FLAG_A2C_TOBATTLE_TIME* ) nmg;
			NotifyTimeToStart( pnetmsg->wTime );
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_NEXTSCHED:
		{
			GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_NEXTSCHED* pnetmsg = ( GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_NEXTSCHED* ) nmg;
			m_sScheduleNext = pnetmsg->sScheduleNext;
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2C_BATTLEINFO_PC:
		{
			GLMSG::SNET_CAPTURE_THE_FLAG_A2C_BATTLEINFO_PC* pnetmsg = ( GLMSG::SNET_CAPTURE_THE_FLAG_A2C_BATTLEINFO_PC* ) nmg;
			m_emState = pnetmsg->emState;
			m_fRemain = pnetmsg->fRemain;
			m_sScheduleNext = pnetmsg->sScheduleNext;
			m_wLevelReq = pnetmsg->wLevelReq;
			m_llContributionReq = pnetmsg->llContributionReq;
			m_wPlayerLimit = pnetmsg->wPlayerLimit;
			m_wMaxScore = pnetmsg->wMaxScore;
			m_wBattleTime = pnetmsg->wBattleTime;

			if ( m_emState == CAPTURE_THE_FLAG_STATE_REGISTER ){
				CInnerInterface::GetInstance().SetCompetitionButtonAlarm( TRUE );
			}

			if ( m_emState != CAPTURE_THE_FLAG_STATE_REWARD ){
				m_vecRankingAll.clear();
				m_vecRankingA.clear();
				m_vecRankingB.clear();
			}

			m_bRegistered = pnetmsg->bRegistered;
			m_wTeam = pnetmsg->wTeam;

			for ( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
			{
				m_wPlayerNum[i] = pnetmsg->wPlayerNum[i];
			}
		}break;


	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2C_REGISTER_FB:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_A2C_REGISTER_FB* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_A2C_REGISTER_FB* ) nmg;

			for ( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
				m_wPlayerNum[i] = pnetmsg->wPlayerNum[i];

			switch( pnetmsg->emFB )
			{
			case CAPTURE_THE_FLAG_REGISTER_FB_NOTPOSSIBLE:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_NOTPOSSIBLE") );
				break;

			case CAPTURE_THE_FLAG_REGISTER_FB_INVALID_CHANNEL:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_INVALID_CHANNEL") );
				break;

			case CAPTURE_THE_FLAG_REGISTER_FB_INVALID_CHARID:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_INVALID_CHARID") );
				break;

			case CAPTURE_THE_FLAG_REGISTER_FB_INVALID_SCHOOL:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_INVALID_SCHOOL") );
				break;

			case CAPTURE_THE_FLAG_REGISTER_FB_INVALID_TEAM:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_INVALID_TEAM") );
				break;

			case CAPTURE_THE_FLAG_REGISTER_FB_LEVEL_LOW:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_LEVEL_LOW") );
				break;

			case CAPTURE_THE_FLAG_REGISTER_FB_ALREADY_REG:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_ALREADY_REG") );
				break;

			case CAPTURE_THE_FLAG_REGISTER_FB_OTHER_TEAM:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_OTHER_TEAM") );
				break;

			case CAPTURE_THE_FLAG_REGISTER_FB_TEAM_A_FULL:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_TEAM_A_FULL") );
				break;

			case CAPTURE_THE_FLAG_REGISTER_FB_TEAM_B_FULL:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_TEAM_B_FULL") );
				break;

			case CAPTURE_THE_FLAG_REGISTER_FB_REGISTERED_A:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_REGISTERED_A") );
					m_bRegistered = TRUE;
					m_wTeam = pnetmsg->wTeam;
				}break;

			case CAPTURE_THE_FLAG_REGISTER_FB_REGISTERED_B:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_REGISTERED_B") );
					m_bRegistered = TRUE;
					m_wTeam = pnetmsg->wTeam;
				}break;

			case CAPTURE_THE_FLAG_REGISTER_FB_CANCEL_REG:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_CANCEL_REG") );
					m_bRegistered = FALSE;
					m_wTeam = CAPTURE_THE_FLAG_TEAM_SIZE;
				}break;

			case CAPTURE_THE_FLAG_REGISTER_FB_CANCEL_FAIL:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_CANCEL_FAIL") );
				break;

			case CAPTURE_THE_FLAG_REGISTER_FB_TEAM_A_BALANCE:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_TEAM_A_BALANCE") );
				break;

			case CAPTURE_THE_FLAG_REGISTER_FB_TEAM_B_BALANCE:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_TEAM_B_BALANCE") );
				break;

			case CAPTURE_THE_FLAG_REGISTER_FB_CONTRI_LOW:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REGISTER_FB_CONTRI_LOW") );
				break;

			};
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2C_REJOIN_FB:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_A2C_REJOIN_FB* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_A2C_REJOIN_FB* ) nmg;
			switch( pnetmsg->emFB )
			{
			case CAPTURE_THE_FLAG_REJOIN_FB_FAILED:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REJOIN_FB_FAILED") );
				break;

			case CAPTURE_THE_FLAG_REJOIN_FB_NOTPOSSIBLE:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REJOIN_FB_NOTPOSSIBLE") );
				break;

			case CAPTURE_THE_FLAG_REJOIN_FB_NOT_REGISTERED:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REJOIN_FB_NOT_REGISTERED") );
				break;

			case CAPTURE_THE_FLAG_REJOIN_FB_INVALID_CHARID:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REJOIN_FB_INVALID_CHARID") );
				break;

			case CAPTURE_THE_FLAG_REJOIN_FB_INBATTLE:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_REJOIN_FB_INBATTLE") );
				break;
			};
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2C_PLAYER_NUM:
		{
			GLMSG::SNET_CAPTURE_THE_FLAG_A2C_PLAYER_NUM* pnetmsg = ( GLMSG::SNET_CAPTURE_THE_FLAG_A2C_PLAYER_NUM* ) nmg;

			for ( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
				m_wPlayerNum[i] = pnetmsg->wPlayerNum[i];
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2C_SCORE_INFO:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_A2C_SCORE_INFO* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_A2C_SCORE_INFO* ) nmg;
			m_wLastWinner = pnetmsg->wLastWinner;

			for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
			{
				m_sScore[i] = pnetmsg->sScore[i];
				m_bFlagHold[i] = pnetmsg->bFlagHold[i];
			}
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_CAPTURE_UPDATE:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_CAPTURE_UPDATE* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_CAPTURE_UPDATE* ) nmg;

			for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i ){
				m_sScore[i] = pnetmsg->sScore[i];
			}
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2C_RANKINFO_PC:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_A2C_RANKINFO_PC* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_A2C_RANKINFO_PC* ) nmg;
			m_wLastWinner = pnetmsg->wLastWinner;

			for( int i=0; i<CAPTURE_THE_FLAG_MINI_RANKING_NUM; ++i )
				m_sLastTop[i] = pnetmsg->sPlayerData[i];
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_WINNER:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_WINNER* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_WINNER* ) nmg;
			m_wLastWinner = pnetmsg->wLastWinner;

			if ( m_wLastWinner < CAPTURE_THE_FLAG_TEAM_SIZE )
			{
				if ( pnetmsg->bFullCapture )
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_WINNER_INFO_FULL_CAPTURE"), ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_TEAM_TEXT", m_wLastWinner ) );
				else
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_WINNER_INFO"), ID2GAMEWORD( "PVP_CAPTURE_THE_FLAG_TEAM_TEXT", m_wLastWinner ) );
			}
			else
			{
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_WINNER_INFO_FAILED") );
			}
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_RANKING_UPDATE:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_RANKING_UPDATE* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_RANKING_UPDATE* ) nmg;
			for( int i=0; i<pnetmsg->wRankNum; ++i )
			{
				m_vecRankingAll.push_back( pnetmsg->sPlayerData[i] );

				if ( pnetmsg->sPlayerData[i].wInfoTeam == CAPTURE_THE_FLAG_TEAM_A )
					m_vecRankingA.push_back( pnetmsg->sPlayerData[i] );
				else if ( pnetmsg->sPlayerData[i].wInfoTeam == CAPTURE_THE_FLAG_TEAM_B )
					m_vecRankingB.push_back( pnetmsg->sPlayerData[i] );
			}
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_RANKING_END:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_RANKING_END* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_RANKING_END* ) nmg;
			std::sort( m_vecRankingAll.begin(), m_vecRankingAll.end() );
			std::sort( m_vecRankingA.begin(), m_vecRankingA.end() );
			std::sort( m_vecRankingB.begin(), m_vecRankingB.end() );

			CInnerInterface::GetInstance().PVPCaptureTheFlagShowRanking();
			CInnerInterface::GetInstance().PVPCaptureTheFlagShowEndNotice();

			if( pnetmsg->wPlayerNum[CAPTURE_THE_FLAG_TEAM_A] != (WORD)m_vecRankingA.size() ||
				pnetmsg->wPlayerNum[CAPTURE_THE_FLAG_TEAM_B] != (WORD)m_vecRankingB.size()  )
			{
				CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagClient Receive Player Rank size error Field:%d~%d Client:%d~%d", 
					pnetmsg->wPlayerNum[CAPTURE_THE_FLAG_TEAM_A], pnetmsg->wPlayerNum[CAPTURE_THE_FLAG_TEAM_B],
					(WORD)m_vecRankingA.size(), (WORD)m_vecRankingB.size() );
			}
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_FLAG_HOLD:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_FLAG_HOLD* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_FLAG_HOLD* ) nmg;

			for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
				m_bFlagHold[i] = pnetmsg->bFlagHold[i];

			if ( pnetmsg->emStatus == CAPTURE_THE_FLAG_FLAG_HOLD_STATUS_TIMEOUT && pnetmsg->wTeam < CAPTURE_THE_FLAG_TEAM_SIZE )
			{
				if ( pnetmsg->wTeam ==  CAPTURE_THE_FLAG_TEAM_A )
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_FLAG_TIMEOUT_A") );
				else if ( pnetmsg->wTeam == CAPTURE_THE_FLAG_TEAM_B )
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_FLAG_TIMEOUT_B") );
			}

			if ( pnetmsg->emStatus == CAPTURE_THE_FLAG_FLAG_HOLD_STATUS_LEFTBATTLE && pnetmsg->wTeam < CAPTURE_THE_FLAG_TEAM_SIZE )
			{
				if ( pnetmsg->wTeam ==  CAPTURE_THE_FLAG_TEAM_A )
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_FLAG_PLAYER_LEFT_A"), pnetmsg->szName );
				else if ( pnetmsg->wTeam == CAPTURE_THE_FLAG_TEAM_B )
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_FLAG_PLAYER_LEFT_B"), pnetmsg->szName );
			}

			if ( pnetmsg->emStatus == CAPTURE_THE_FLAG_FLAG_HOLD_STATUS_DEAD && pnetmsg->wTeam < CAPTURE_THE_FLAG_TEAM_SIZE )
			{
				if ( pnetmsg->wTeam ==  CAPTURE_THE_FLAG_TEAM_A )
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_FLAG_PLAYER_DEAD_A"), pnetmsg->szName );
				else if ( pnetmsg->wTeam == CAPTURE_THE_FLAG_TEAM_B )
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_FLAG_PLAYER_DEAD_B"), pnetmsg->szName );
			}

			if ( pnetmsg->emStatus == CAPTURE_THE_FLAG_FLAG_HOLD_STATUS_CAPTURE )
			{
				if ( pnetmsg->wTeam ==  CAPTURE_THE_FLAG_TEAM_A )
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_FLAG_PLAYER_CAPTURE_A"), pnetmsg->szName );
				else if ( pnetmsg->wTeam == CAPTURE_THE_FLAG_TEAM_B )
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_FLAG_PLAYER_CAPTURE_B"), pnetmsg->szName );
			}

			if ( pnetmsg->emStatus == CAPTURE_THE_FLAG_FLAG_HOLD_STATUS_SCORE )
			{
				if ( pnetmsg->wTeam ==  CAPTURE_THE_FLAG_TEAM_A )
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_FLAG_PLAYER_SCORE_A"), pnetmsg->szName );
				else if ( pnetmsg->wTeam == CAPTURE_THE_FLAG_TEAM_B )
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("CAPTURE_THE_FLAG_FLAG_PLAYER_SCORE_B"), pnetmsg->szName );
			}
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_FLAG_POSITION:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_FLAG_POSITION* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_FLAG_POSITION* ) nmg;

			for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
				m_sFlagTarget[i] = pnetmsg->sFlagTarget[i];
		}break;
	};
	

	return S_OK;
}