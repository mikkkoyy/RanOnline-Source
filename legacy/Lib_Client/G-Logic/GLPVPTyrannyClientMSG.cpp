#include "StdAfx.h"
#include "GLPVPTyrannyClient.h"
#include "GLGaeaClient.h"
#include "InnerInterface.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HRESULT GLPVPTyrannyClient::MsgProcess ( NET_MSG_GENERIC* nmg )
{
	if ( !nmg ){
		return E_FAIL;
	}

	switch ( nmg->nType )
	{
	case NET_MSG_GCTRL_TYRANNY_A2FC_STATE_REGISTER:
		{
			GLMSG::SNET_TYRANNY_A2FC_STATE_REGISTER* pnetmsg = ( GLMSG::SNET_TYRANNY_A2FC_STATE_REGISTER* ) nmg;
			SetEventState( TYRANNY_STATE_REGISTER );
			m_fRemain = pnetmsg->fTime;

			CInnerInterface::GetInstance().SetCompetitionButtonAlarm( TRUE );

		}break;

	case NET_MSG_GCTRL_TYRANNY_A2FC_STATE_BATTLE:
		{
			GLMSG::SNET_TYRANNY_A2FC_STATE_BATTLE* pnetmsg = ( GLMSG::SNET_TYRANNY_A2FC_STATE_BATTLE* ) nmg;
			SetEventState( TYRANNY_STATE_BATTLE );
			m_fRemain = pnetmsg->fTime;
		}break;

	case NET_MSG_GCTRL_TYRANNY_A2FC_STATE_REWARD:
		{
			GLMSG::SNET_TYRANNY_A2FC_STATE_REWARD* pnetmsg = ( GLMSG::SNET_TYRANNY_A2FC_STATE_REWARD* ) nmg;
			SetEventState( TYRANNY_STATE_REWARD );
			m_fRemain = pnetmsg->fTime;
		}break;

	case NET_MSG_GCTRL_TYRANNY_A2FC_STATE_ENDED:
		{
			GLMSG::SNET_TYRANNY_A2FC_STATE_ENDED* pnetmsg = ( GLMSG::SNET_TYRANNY_A2FC_STATE_ENDED* ) nmg;
			SetEventState( TYRANNY_STATE_ENDED );
		}break;

	case NET_MSG_GCTRL_TYRANNY_A2C_TOBATTLE_TIME:
		{
			GLMSG::SNET_TYRANNY_A2C_TOBATTLE_TIME* pnetmsg = ( GLMSG::SNET_TYRANNY_A2C_TOBATTLE_TIME* ) nmg;
			NotifyTimeToStart( pnetmsg->wTime );
		}break;

	case NET_MSG_GCTRL_TYRANNY_A2FC_NEXTSCHED:
		{
			GLMSG::SNET_TYRANNY_A2FC_NEXTSCHED* pnetmsg = ( GLMSG::SNET_TYRANNY_A2FC_NEXTSCHED* ) nmg;
			m_sScheduleNext = pnetmsg->sScheduleNext;
		}break;

	case NET_MSG_GCTRL_TYRANNY_A2C_BATTLEINFO_PC:
		{
			GLMSG::SNET_TYRANNY_A2C_BATTLEINFO_PC* pnetmsg = ( GLMSG::SNET_TYRANNY_A2C_BATTLEINFO_PC* ) nmg;
			m_emState = pnetmsg->emState;
			m_fRemain = pnetmsg->fRemain;
			m_sScheduleNext = pnetmsg->sScheduleNext;
			m_wLevelReq = pnetmsg->wLevelReq;
			m_wPlayerLimit = pnetmsg->wPlayerLimit;

			if ( m_emState == TYRANNY_STATE_REGISTER ){
				CInnerInterface::GetInstance().SetCompetitionButtonAlarm( TRUE );
			}

			if ( m_emState != TYRANNY_STATE_REWARD ){
				m_vecRankingAll.clear();
				m_vecRankingSG.clear();
				m_vecRankingMP.clear();
				m_vecRankingPHX.clear();
			}

			m_bRegistered = pnetmsg->bRegistered;
			m_bQueued = pnetmsg->bQueued;
			m_wQueueNum = pnetmsg->wQueueNum;
		}break;

	case NET_MSG_GCTRL_TYRANNY_A2C_REGISTER_FB:
		{
			GLMSG::SNETPC_TYRANNY_A2C_REGISTER_FB* pnetmsg = ( GLMSG::SNETPC_TYRANNY_A2C_REGISTER_FB* ) nmg;

			switch( pnetmsg->emFB )
			{
			case TYRANNY_REGISTER_FB_NOTPOSSIBLE:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("TYRANNY_REGISTER_FB_NOTPOSSIBLE") );
				}break;

			case TYRANNY_REGISTER_FB_INVALID_CHANNEL:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("TYRANNY_REGISTER_FB_INVALID_CHANNEL") );
				}break;

			case TYRANNY_REGISTER_FB_INVALID_CHARID:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("TYRANNY_REGISTER_FB_INVALID_CHARID") );
				}break;

			case TYRANNY_REGISTER_FB_INVALID_SCHOOL:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("TYRANNY_REGISTER_FB_INVALID_SCHOOL") );
				}break;

			case TYRANNY_REGISTER_FB_LEVEL_LOW:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("TYRANNY_REGISTER_FB_LEVEL_LOW") );
				}break;

			case TYRANNY_REGISTER_FB_ALREADY_REG:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("TYRANNY_REGISTER_FB_ALREADY_REG") );
				}break;

			case TYRANNY_REGISTER_FB_ALREADY_QUEUE:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("TYRANNY_REGISTER_FB_ALREADY_QUEUE") );
				}break;
			
			case TYRANNY_REGISTER_FB_QUEUED:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("TYRANNY_REGISTER_FB_QUEUED") );
					m_bRegistered = TRUE;
					if ( pnetmsg->wQueueNum != 0 ){
						m_bQueued = TRUE;
						m_wQueueNum = pnetmsg->wQueueNum;
						CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("TYRANNY_REGISTER_FB_QUEUED_NUM"), m_wQueueNum );
					}
				}break;

			case TYRANNY_REGISTER_FB_REGISTERED:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("TYRANNY_REGISTER_FB_REGISTERED") );
					m_bRegistered = TRUE;
					m_bQueued = FALSE;
					m_wQueueNum = 0;
				}break;
			
			case TYRANNY_REGISTER_FB_CANCEL_REG:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("TYRANNY_REGISTER_FB_CANCEL_REG") );
					m_bRegistered = FALSE;
					m_bQueued = FALSE;
					m_wQueueNum = 0;
				}break;

			case TYRANNY_REGISTER_FB_CANCEL_FAIL:
				{
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("TYRANNY_REGISTER_FB_CANCEL_FAIL") );
				}break;

			};
		}break;

	case NET_MSG_GCTRL_TYRANNY_A2C_REJOIN_FB:
		{
			GLMSG::SNETPC_TYRANNY_A2C_REJOIN_FB* pnetmsg = ( GLMSG::SNETPC_TYRANNY_A2C_REJOIN_FB* ) nmg;
			switch( pnetmsg->emFB )
			{
			case TYRANNY_REJOIN_FB_FAILED:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("TYRANNY_REJOIN_FB_FAILED") );
				break;

			case TYRANNY_REJOIN_FB_NOTPOSSIBLE:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("TYRANNY_REJOIN_FB_NOTPOSSIBLE") );
				break;

			case TYRANNY_REJOIN_FB_NOT_REGISTERED:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("TYRANNY_REJOIN_FB_NOT_REGISTERED") );
				break;

			case TYRANNY_REJOIN_FB_INVALID_CHARID:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("TYRANNY_REJOIN_FB_INVALID_CHARID") );
				break;

			case TYRANNY_REJOIN_FB_INBATTLE:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("TYRANNY_REJOIN_FB_INBATTLE") );
				break;
			};
		}break;

	case NET_MSG_GCTRL_TYRANNY_A2C_QUEUE_MOVED:
		{
			CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("TYRANNY_REGISTER_FB_REGISTERED") );
			m_bRegistered = TRUE;
			m_bQueued = FALSE;
			m_wQueueNum = 0;
		}break;

	case NET_MSG_GCTRL_TYRANNY_A2C_QUEUE_UPDATE:
		{
			GLMSG::SNETPC_TYRANNY_A2C_QUEUE_UPDATE* pnetmsg = ( GLMSG::SNETPC_TYRANNY_A2C_QUEUE_UPDATE* ) nmg;
			if ( pnetmsg->wQueueNum != 0 && m_bQueued && m_bRegistered ){
				m_wQueueNum = pnetmsg->wQueueNum;
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("TYRANNY_REGISTER_FB_QUEUED_NUM"), m_wQueueNum );
			}
		}break;


	case NET_MSG_GCTRL_TYRANNY_F2C_TOWER_CAPTURE:
		{
			GLMSG::SNETPC_TYRANNY_F2C_TOWER_CAPTURE* pnetmsg = ( GLMSG::SNETPC_TYRANNY_F2C_TOWER_CAPTURE* ) nmg;
			if ( pnetmsg->wTowerIndex < TYRANNY_TOWER_SIZE )
			{
				m_wTowerOwner[pnetmsg->wTowerIndex] = pnetmsg->wTowerSchool;
				CInnerInterface::GetInstance().PVPTyrannyShowCaptureNotice( m_wTowerOwner[pnetmsg->wTowerIndex] );

				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("TYRANNY_CAPTURE_INFO"), 
					ID2GAMEWORD( "TYRANNY_TOWER_TEXT", pnetmsg->wTowerIndex),  GLCONST_CHAR::strSCHOOLNAME[pnetmsg->wTowerSchool].c_str() );

				if ( pnetmsg->wTowerScore == ( TYRANNY_TOWER_SIZE -1 ) )
				{
					if ( GLGaeaClient::GetInstance().GetCharacter()->m_wSchool == pnetmsg->wTowerSchool ) 
						CInnerInterface::GetInstance().PVPTyrannyShow2CaptureOwnNotice();
					else
						CInnerInterface::GetInstance().PVPTyrannyShow2CaptureNotice( pnetmsg->wTowerSchool );	
				}
			}
		}break;

	case NET_MSG_GCTRL_TYRANNY_A2C_TOWER_INFO:
		{
			GLMSG::SNETPC_TYRANNY_A2C_TOWER_OWNER_INFO* pnetmsg = ( GLMSG::SNETPC_TYRANNY_A2C_TOWER_OWNER_INFO* ) nmg;
			m_wLastWinner = pnetmsg->wLastWinner;

			for( int i=0; i<TYRANNY_TOWER_SIZE; ++i )
				m_wTowerOwner[i] = pnetmsg->wTowerSchool[i];
		}break;

	case NET_MSG_GCTRL_TYRANNY_F2C_TOWER_WINNER:
		{
			GLMSG::SNETPC_TYRANNY_F2C_TOWER_WINNER* pnetmsg = ( GLMSG::SNETPC_TYRANNY_F2C_TOWER_WINNER* ) nmg;
			m_wLastWinner = pnetmsg->wLastWinner;

			if ( m_wLastWinner < TYRANNY_SCHOOL_SIZE )
			{
				if ( pnetmsg->bFullCapture )
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("TYRANNY_WINNER_INFO_FULL_CAPTURE"), GLCONST_CHAR::strSCHOOLNAME[m_wLastWinner].c_str() );
				else
					CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("TYRANNY_WINNER_INFO"), GLCONST_CHAR::strSCHOOLNAME[m_wLastWinner].c_str() );
			}
			else
			{
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("TYRANNY_WINNER_INFO_FAILED") );
			}
			
		}break;

	case NET_MSG_GCTRL_TYRANNY_F2C_RANKING_UPDATE:
		{
			GLMSG::SNETPC_TYRANNY_F2C_RANKING_UPDATE* pnetmsg = ( GLMSG::SNETPC_TYRANNY_F2C_RANKING_UPDATE* ) nmg;
			for( int i=0; i<pnetmsg->wRankNum; ++i )
			{
				m_vecRankingAll.push_back( pnetmsg->sPlayerData[i] );

				if ( pnetmsg->sPlayerData[i].wInfoSchool == TYRANNY_SCHOOL_SG )
					m_vecRankingSG.push_back( pnetmsg->sPlayerData[i] );
				else if ( pnetmsg->sPlayerData[i].wInfoSchool == TYRANNY_SCHOOL_MP )
					m_vecRankingMP.push_back( pnetmsg->sPlayerData[i] );
				else if ( pnetmsg->sPlayerData[i].wInfoSchool == TYRANNY_SCHOOL_PHX )
					m_vecRankingPHX.push_back( pnetmsg->sPlayerData[i] );
			}
		}break;

	case NET_MSG_GCTRL_TYRANNY_F2C_RANKING_END:
		{
			GLMSG::SNETPC_TYRANNY_F2C_RANKING_END* pnetmsg = ( GLMSG::SNETPC_TYRANNY_F2C_RANKING_END* ) nmg;
			std::sort( m_vecRankingAll.begin(), m_vecRankingAll.end() );
			std::sort( m_vecRankingSG.begin(), m_vecRankingSG.end() );
			std::sort( m_vecRankingMP.begin(), m_vecRankingMP.end() );
			std::sort( m_vecRankingPHX.begin(), m_vecRankingPHX.end() );

			CInnerInterface::GetInstance().PVPTyrannyShowRanking();
			CInnerInterface::GetInstance().PVPTyrannyShowEndNotice();

			if( pnetmsg->wPlayerNum[TYRANNY_SCHOOL_SG] != (WORD)m_vecRankingSG.size() ||
				pnetmsg->wPlayerNum[TYRANNY_SCHOOL_MP] != (WORD)m_vecRankingMP.size() ||
				pnetmsg->wPlayerNum[TYRANNY_SCHOOL_PHX] != (WORD)m_vecRankingPHX.size() )
			{
				CDebugSet::ToFileWithTime( "_pvptyranny.txt", "GLPVPTyrannyClient Receive Player Rank size error Field:%d~%d~%d Client:%d~%d~%d", 
					pnetmsg->wPlayerNum[TYRANNY_SCHOOL_SG], pnetmsg->wPlayerNum[TYRANNY_SCHOOL_MP], pnetmsg->wPlayerNum[TYRANNY_SCHOOL_PHX],
					(WORD)m_vecRankingSG.size(), (WORD)m_vecRankingMP.size(), (WORD)m_vecRankingPHX.size() );
			}
		}break;

	case NET_MSG_GCTRL_TYRANNY_A2C_RANKINFO_PC:
		{
			GLMSG::SNETPC_TYRANNY_A2C_RANKINFO_PC* pnetmsg = ( GLMSG::SNETPC_TYRANNY_A2C_RANKINFO_PC* ) nmg;
			m_wLastWinner = pnetmsg->wLastWinner;

			for( int i=0; i<TYRANNY_MINI_RANKING_NUM; ++i )
				m_sLastTop[i] = pnetmsg->sPlayerData[i];

			for( int i=0; i<TYRANNY_REWARD_BUFF_NUM; ++i )
				m_sRewardBuff[i] = pnetmsg->sRewardBuff[i];
		}break;
	};

	return S_OK;
}