#include "StdAfx.h"
#include "GLPVPSchoolWarsClient.h"
#include "GLGaeaClient.h"
#include "InnerInterface.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "../Lib_ClientUI/Interface/PVPSchoolWarsScore.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HRESULT GLPVPSchoolWarsClient::MsgProcess ( NET_MSG_GENERIC* nmg )
{
	if ( !nmg ){
		return E_FAIL;
	}

	switch ( nmg->nType )
	{
	case NET_MSG_GCTRL_SCHOOLWARS_A2FC_STATE_REGISTER:
		{
			GLMSG::SNET_SCHOOLWARS_A2FC_STATE_REGISTER* pnetmsg = ( GLMSG::SNET_SCHOOLWARS_A2FC_STATE_REGISTER* ) nmg;
			SetEventState( SCHOOLWARS_STATE_REGISTER );
			m_fRemain = pnetmsg->fTime;

			CInnerInterface::GetInstance().SetCompetitionButtonAlarm( TRUE );

		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2FC_STATE_BATTLE:
		{
			GLMSG::SNET_SCHOOLWARS_A2FC_STATE_BATTLE* pnetmsg = ( GLMSG::SNET_SCHOOLWARS_A2FC_STATE_BATTLE* ) nmg;
			SetEventState( SCHOOLWARS_STATE_BATTLE );
			m_fRemain = pnetmsg->fTime;
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2FC_STATE_REWARD:
		{
			GLMSG::SNET_SCHOOLWARS_A2FC_STATE_REWARD* pnetmsg = ( GLMSG::SNET_SCHOOLWARS_A2FC_STATE_REWARD* ) nmg;
			SetEventState( SCHOOLWARS_STATE_REWARD );
			m_fRemain = pnetmsg->fTime;
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2FC_STATE_ENDED:
		{
			GLMSG::SNET_SCHOOLWARS_A2FC_STATE_ENDED* pnetmsg = ( GLMSG::SNET_SCHOOLWARS_A2FC_STATE_ENDED* ) nmg;
			SetEventState( SCHOOLWARS_STATE_ENDED );
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2C_TOBATTLE_TIME:
		{
			GLMSG::SNET_SCHOOLWARS_A2C_TOBATTLE_TIME* pnetmsg = ( GLMSG::SNET_SCHOOLWARS_A2C_TOBATTLE_TIME* ) nmg;
			NotifyTimeToStart( pnetmsg->wTime );
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2FC_NEXTSCHED:
		{
			GLMSG::SNET_SCHOOLWARS_A2FC_NEXTSCHED* pnetmsg = ( GLMSG::SNET_SCHOOLWARS_A2FC_NEXTSCHED* ) nmg;
			m_sScheduleNext = pnetmsg->sScheduleNext;
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2C_BATTLEINFO_PC:
		{
			GLMSG::SNET_SCHOOLWARS_A2C_BATTLEINFO_PC* pnetmsg = ( GLMSG::SNET_SCHOOLWARS_A2C_BATTLEINFO_PC* ) nmg;
			m_emState = pnetmsg->emState;
			m_fRemain = pnetmsg->fRemain;
			m_sScheduleNext = pnetmsg->sScheduleNext;
			m_wLevelReq = pnetmsg->wLevelReq;
			m_llContriReq = pnetmsg->llContriReq;
			m_wPlayerLimit = pnetmsg->wPlayerLimit;
			m_wMaxScore = pnetmsg->wMaxScore;
			m_wBattleTime = pnetmsg->wBattleTime;

			if ( m_emState == SCHOOLWARS_STATE_REGISTER ){
				CInnerInterface::GetInstance().SetCompetitionButtonAlarm( TRUE );
			}

			if ( m_emState != SCHOOLWARS_STATE_REWARD ){
				m_vecRankingAll.clear();
				m_vecRankingSG.clear();
				m_vecRankingMP.clear();
				m_vecRankingPHX.clear();
			}

			m_bRegistered = pnetmsg->bRegistered;
			m_bQueued = pnetmsg->bQueued;
			m_wQueueNum = pnetmsg->wQueueNum;
		}break;
	
	case NET_MSG_GCTRL_SCHOOLWARS_A2C_REGISTER_FB:
		{
			GLMSG::SNETPC_SCHOOLWARS_A2C_REGISTER_FB* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_A2C_REGISTER_FB* ) nmg;

			switch( pnetmsg->emFB )
			{
			case SCHOOLWARS_REGISTER_FB_NOTPOSSIBLE:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_NOTPOSSIBLE") );
				}break;

			case SCHOOLWARS_REGISTER_FB_INVALID_CHANNEL:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_INVALID_CHANNEL") );
				}break;

			case SCHOOLWARS_REGISTER_FB_INVALID_CHARID:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_INVALID_CHARID") );
				}break;

			case SCHOOLWARS_REGISTER_FB_INVALID_SCHOOL:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_INVALID_SCHOOL") );
				}break;

			case SCHOOLWARS_REGISTER_FB_LEVEL_LOW:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_LEVEL_LOW") );
				}break;

			case SCHOOLWARS_REGISTER_FB_CONTRI_LOW:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_CONTRI_LOW") );
				}break;

			case SCHOOLWARS_REGISTER_FB_ALREADY_REG:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_ALREADY_REG") );
				}break;

			case SCHOOLWARS_REGISTER_FB_ALREADY_QUEUE:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_ALREADY_QUEUE") );
				}break;

			case SCHOOLWARS_REGISTER_FB_QUEUED:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_QUEUED") );
					m_bRegistered = TRUE;
					if ( pnetmsg->wQueueNum != 0 ){
						m_bQueued = TRUE;
						m_wQueueNum = pnetmsg->wQueueNum;
						CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_QUEUED_NUM"), m_wQueueNum );
					}
				}break;

			case SCHOOLWARS_REGISTER_FB_REGISTERED:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_REGISTERED") );
					m_bRegistered = TRUE;
					m_bQueued = FALSE;
					m_wQueueNum = 0;
				}break;

			case SCHOOLWARS_REGISTER_FB_CANCEL_REG:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_CANCEL_REG") );
					m_bRegistered = FALSE;
					m_bQueued = FALSE;
					m_wQueueNum = 0;
				}break;

			case SCHOOLWARS_REGISTER_FB_CANCEL_FAIL:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_CANCEL_FAIL") );
				}break;

			};
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2C_REJOIN_FB:
		{
			GLMSG::SNETPC_SCHOOLWARS_A2C_REJOIN_FB* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_A2C_REJOIN_FB* ) nmg;
			switch( pnetmsg->emFB )
			{
			case SCHOOLWARS_REJOIN_FB_FAILED:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SCHOOLWARS_REJOIN_FB_FAILED") );
				break;

			case SCHOOLWARS_REJOIN_FB_NOTPOSSIBLE:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SCHOOLWARS_REJOIN_FB_NOTPOSSIBLE") );
				break;

			case SCHOOLWARS_REJOIN_FB_NOT_REGISTERED:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SCHOOLWARS_REJOIN_FB_NOT_REGISTERED") );
				break;

			case SCHOOLWARS_REJOIN_FB_INVALID_CHARID:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("SCHOOLWARS_REJOIN_FB_INVALID_CHARID") );
				break;

			case SCHOOLWARS_REJOIN_FB_INBATTLE:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("SCHOOLWARS_REJOIN_FB_INBATTLE") );
				break;
			};
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2C_QUEUE_MOVED:
		{
			CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_REGISTERED") );
			m_bRegistered = TRUE;
			m_bQueued = FALSE;
			m_wQueueNum = 0;
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2C_QUEUE_UPDATE:
		{
			GLMSG::SNETPC_SCHOOLWARS_A2C_QUEUE_UPDATE* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_A2C_QUEUE_UPDATE* ) nmg;
			if ( pnetmsg->wQueueNum != 0 && m_bQueued && m_bRegistered ){
				m_wQueueNum = pnetmsg->wQueueNum;
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("SCHOOLWARS_REGISTER_FB_QUEUED_NUM"), m_wQueueNum );
			}
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2C_SCORE_INFO:
		{
			GLMSG::SNETPC_SCHOOLWARS_A2C_SCORE_INFO* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_A2C_SCORE_INFO* ) nmg;
			m_wLastWinner = pnetmsg->wLastWinner;

			for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++i )
				m_sScore[i] = pnetmsg->sScore[i];

			m_fDoublePoint = pnetmsg->fDoublePoint;
			m_bDoublePoint = pnetmsg->bDoublePoint;
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_F2C_SCORE_UPDATE:
		{
			GLMSG::SNETPC_SCHOOLWARS_F2C_SCORE_UPDATE* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_F2C_SCORE_UPDATE* ) nmg;
			
			for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++i )
				m_sScore[i] = pnetmsg->sScore[i];

			CPVPSchoolWarsScore* pscore = CInnerInterface::GetInstance().GetSchoolWarsScoreWindow();
			if ( pscore )	pscore->ToNotify();
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2C_RANKINFO_PC:
		{
			GLMSG::SNETPC_SCHOOLWARS_A2C_RANKINFO_PC* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_A2C_RANKINFO_PC* ) nmg;
			m_wLastWinner = pnetmsg->wLastWinner;

			for( int i=0; i<SCHOOLWARS_MINI_RANKING_NUM; ++i )
				m_sLastTop[i] = pnetmsg->sPlayerData[i];

			CPVPSchoolWarsScore* pscore= CInnerInterface::GetInstance().GetSchoolWarsScoreWindow();
			if ( pscore )	pscore->ToNotify();
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_F2C_SCORE_WINNER:
		{
			GLMSG::SNETPC_SCHOOLWARS_F2C_SCORE_WINNER* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_F2C_SCORE_WINNER* ) nmg;
			m_wLastWinner = pnetmsg->wLastWinner;

			if ( m_wLastWinner < SCHOOLWARS_SCHOOL_SIZE )
			{
				if ( pnetmsg->bFullScore )
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("SCHOOLWARS_WINNER_INFO_FULL_CAPTURE"), GLCONST_CHAR::strSCHOOLNAME[m_wLastWinner].c_str() );
				else
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("SCHOOLWARS_WINNER_INFO"), GLCONST_CHAR::strSCHOOLNAME[m_wLastWinner].c_str() );
			}
			else
			{
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("SCHOOLWARS_WINNER_INFO_FAILED") );
			}

		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_F2C_RANKING_UPDATE:
		{
			GLMSG::SNETPC_SCHOOLWARS_F2C_RANKING_UPDATE* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_F2C_RANKING_UPDATE* ) nmg;
			for( int i=0; i<pnetmsg->wRankNum; ++i )
			{
				m_vecRankingAll.push_back( pnetmsg->sPlayerData[i] );

				if ( pnetmsg->sPlayerData[i].wInfoSchool == SCHOOLWARS_SCHOOL_SG )
					m_vecRankingSG.push_back( pnetmsg->sPlayerData[i] );
				else if ( pnetmsg->sPlayerData[i].wInfoSchool == SCHOOLWARS_SCHOOL_MP )
					m_vecRankingMP.push_back( pnetmsg->sPlayerData[i] );
				else if ( pnetmsg->sPlayerData[i].wInfoSchool == SCHOOLWARS_SCHOOL_PHX )
					m_vecRankingPHX.push_back( pnetmsg->sPlayerData[i] );
			}
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_F2C_RANKING_END:
		{
			GLMSG::SNETPC_SCHOOLWARS_F2C_RANKING_END* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_F2C_RANKING_END* ) nmg;
			std::sort( m_vecRankingAll.begin(), m_vecRankingAll.end() );
			std::sort( m_vecRankingSG.begin(), m_vecRankingSG.end() );
			std::sort( m_vecRankingMP.begin(), m_vecRankingMP.end() );
			std::sort( m_vecRankingPHX.begin(), m_vecRankingPHX.end() );

			CInnerInterface::GetInstance().PVPSchoolWarsShowRanking();
			CInnerInterface::GetInstance().PVPSchoolWarsShowEndNotice();

			if( pnetmsg->wPlayerNum[SCHOOLWARS_SCHOOL_SG] != (WORD)m_vecRankingSG.size() ||
				pnetmsg->wPlayerNum[SCHOOLWARS_SCHOOL_MP] != (WORD)m_vecRankingMP.size() ||
				pnetmsg->wPlayerNum[SCHOOLWARS_SCHOOL_PHX] != (WORD)m_vecRankingPHX.size() )
			{
				CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "GLPVPSchoolWarsClient Receive Player Rank size error Field:%d~%d~%d Client:%d~%d~%d", 
					pnetmsg->wPlayerNum[SCHOOLWARS_SCHOOL_SG], pnetmsg->wPlayerNum[SCHOOLWARS_SCHOOL_MP], pnetmsg->wPlayerNum[SCHOOLWARS_SCHOOL_PHX],
					(WORD)m_vecRankingSG.size(), (WORD)m_vecRankingMP.size(), (WORD)m_vecRankingPHX.size() );
			}
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2C_SCORE_UPDATE:
		{
			GLMSG::SNETPC_SCHOOLWARS_A2C_SCORE_UPDATE* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_A2C_SCORE_UPDATE* ) nmg;

			for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++i )
				m_sScore[i] = pnetmsg->sScore[i];

			CPVPSchoolWarsScore* pscore= CInnerInterface::GetInstance().GetSchoolWarsScoreWindow();
			if ( pscore )	pscore->ToNotify();
		}break;
	};

	return S_OK;
}