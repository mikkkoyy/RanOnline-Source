#include "StdAfx.h"
#include "GLPVPTyrannyAgent.h"
#include "GLAgentServer.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void GLPVPTyrannyAgent::RequestTowerInfo( DWORD dwCharID )
{
	PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar )	return;


	GLMSG::SNETPC_TYRANNY_A2C_TOWER_OWNER_INFO netmsgclient;
	netmsgclient.wLastWinner = m_wLastWinner;
	for( int i=0; i<TYRANNY_TOWER_SIZE; ++i )
		netmsgclient.wTowerSchool[i] = m_sTowerData[i].wOwner;

	GLAgentServer::GetInstance().SENDTOCLIENT( pchar->m_dwClientID, &netmsgclient );
}

void GLPVPTyrannyAgent::TowerCapture( SNATIVEID sidMob, WORD wSchool )
{
	if ( wSchool >= TYRANNY_SCHOOL_SIZE )		return;
	if ( m_emState != TYRANNY_STATE_BATTLE )	return;
	if ( m_wLastWinner != TYRANNY_SCHOOL_SIZE )	return;

	for( int i=0; i<TYRANNY_TOWER_SIZE; ++i )
	{
		if ( sidMob == m_sTowerData[i].sidCrow && m_sTowerData[i].wOwner != wSchool )
		{
			if ( m_sTowerData[i].wOwner != TYRANNY_SCHOOL_SIZE )
			{
				if ( m_sTowerScore[ m_sTowerData[i].wOwner ].wCapture > 0 )
				{
					m_sTowerScore[ m_sTowerData[i].wOwner ].wCapture --;
				}
			}

			m_sTowerData[i].wOwner = wSchool;

			m_sTowerScore[ m_sTowerData[i].wOwner ].wCapture ++;

			GLMSG::SNETPC_TYRANNY_A2F_TOWER_CAPTURE netmsgfield;
			netmsgfield.wTowerIndex = i;
			netmsgfield.wTowerSchool = wSchool;
			netmsgfield.wTowerScore = m_sTowerScore[ m_sTowerData[i].wOwner ].wCapture ;
			GLAgentServer::GetInstance().SENDTOCHANNEL( &netmsgfield, 0 );

			if ( m_sTowerScore[ m_sTowerData[i].wOwner ].wCapture  >= TYRANNY_TOWER_SIZE )
			{
				m_wLastWinner = m_sTowerData[i].wOwner;

				//full capture, end
				SetEventState( TYRANNY_STATE_REWARD );
			}

			break;
		}
	}
}

void GLPVPTyrannyAgent::CheckWinner()
{
	//already have winner from capturing all the towers
	if( m_wLastWinner != TYRANNY_SCHOOL_SIZE )
	{
		GLMSG::SNETPC_TYRANNY_A2F_TOWER_WINNER netmsgwinner;
		netmsgwinner.wLastWinner = m_wLastWinner;
		netmsgwinner.bFullCapture = TRUE;
		GLAgentServer::GetInstance().SENDTOCHANNEL( &netmsgwinner, 0 );
		return;
	}

	TYRANNY_SCORE_DATA_VEC vecscore;
	for( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i )
	{
		if( m_sTowerScore[i].wCapture == 0 )
			continue;

		vecscore.push_back( m_sTowerScore[i] );
	}

	if ( vecscore.empty() )
	{
		//no winner
		GLMSG::SNETPC_TYRANNY_A2F_TOWER_WINNER netmsgwinner;
		netmsgwinner.wLastWinner = TYRANNY_SCHOOL_SIZE;
		netmsgwinner.bFullCapture = FALSE;
		GLAgentServer::GetInstance().SENDTOCHANNEL( &netmsgwinner, 0 );
		return;
	}

	std::sort( vecscore.begin(), vecscore.end() );

	m_wLastWinner = vecscore[0].wSchool;

	GLMSG::SNETPC_TYRANNY_A2F_TOWER_WINNER netmsgwinner;
	netmsgwinner.wLastWinner = m_wLastWinner;
	netmsgwinner.bFullCapture = FALSE;
	GLAgentServer::GetInstance().SENDTOCHANNEL( &netmsgwinner, 0 );
}