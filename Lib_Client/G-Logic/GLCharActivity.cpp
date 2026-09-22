#include "stdafx.h"
#include "./GLChar.h"
#include "./GLGaeaServer.h"
#include "./GLItemMan.h"
#include "./GLActivity.h"

#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*activity system, Juver, 2017/10/30 */
void GLChar::ActivityComplete( SACTIVITY_CHAR_DATA& sactivity_char_data )
{
	SACTIVITY_FILE_DATA* pactivity_file_data = GLActivity::GetInstance().GetActivity( sactivity_char_data.dwActivityID );
	if ( !pactivity_file_data )	return;

	m_mapActivityDone.insert( std::make_pair( sactivity_char_data.dwActivityID, sactivity_char_data ) );

	GLMSG::SNETPC_ACTIVITY_COMPLETE net_msg_client;
	net_msg_client.sData = sactivity_char_data;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client );

	GLMSG::SNETPC_ACTIVITY_COMPLETE_BRD net_msg_client_brd;
	net_msg_client_brd.dwGaeaID = m_dwGaeaID;
	SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&net_msg_client_brd) );

	switch( pactivity_file_data->emNotify )
	{
	case EMACTIVITY_NOTIFY_NONE:
		{
			GLMSG::SNETPC_ACTIVITY_NOTIFY_CLIENT net_msg_client;
			StringCchCopy( net_msg_client.szCharacterName, CHAR_SZNAME+1, m_szName );
			StringCchCopy( net_msg_client.szActivityTitle, ACTIVITY_TITLE_MSG_SIZE+1, pactivity_file_data->strActivityTitle.c_str() );
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client );

		}break;

	case EMACTIVITY_NOTIFY_AROUND:
		{
			GLMSG::SNETPC_ACTIVITY_NOTIFY_CLIENT net_msg_client;
			StringCchCopy( net_msg_client.szCharacterName, CHAR_SZNAME+1, m_szName );
			StringCchCopy( net_msg_client.szActivityTitle, ACTIVITY_TITLE_MSG_SIZE+1, pactivity_file_data->strActivityTitle.c_str() );
			GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMapID.dwID, &net_msg_client );
		}break;

	case EMACTIVITY_NOTIFY_ALLSERVER:
		{
			GLMSG::SNETPC_ACTIVITY_NOTIFY_AGENT net_msg_brd;
			StringCchCopy( net_msg_brd.szCharacterName, CHAR_SZNAME+1, m_szName );
			StringCchCopy( net_msg_brd.szActivityTitle, ACTIVITY_TITLE_MSG_SIZE+1, pactivity_file_data->strActivityTitle.c_str() );
			GLGaeaServer::GetInstance().SENDTOAGENT( &net_msg_brd );;
		}break;
	};

	m_dwActivityPoint += pactivity_file_data->dwRewardPoint;

	GLMSG::SNETPC_UPDATE_ACTIVITY_POINT net_msg_client_update;
	net_msg_client_update.dwPoint = m_dwActivityPoint;
	net_msg_client_update.bNotice = TRUE;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client_update );
}

/*activity system, Juver, 2017/10/30 */
void GLChar::DoActivityLevel ()
{
	if ( !RANPARAM::bFeatureActivity )	return;

	std::vector<DWORD> vecComplete;

	for( SACTIVITY_CHAR_DATA_MAP_ITER iter_activity_prog = m_mapActivityProg.begin();
		iter_activity_prog != m_mapActivityProg.end(); ++ iter_activity_prog )
	{
		SACTIVITY_CHAR_DATA& sactivity_char_data = (*iter_activity_prog).second;
		if ( sactivity_char_data.emType != EMACTIVITY_TYPE_REACH_LEVEL )	continue;

		sactivity_char_data.dwProgressNow = m_wLevel;

		if ( sactivity_char_data.dwProgressNow >= sactivity_char_data.dwProgressMax )
		{
			 sactivity_char_data.dwProgressNow = sactivity_char_data.dwProgressMax;
			 vecComplete.push_back( sactivity_char_data.dwActivityID );
		}
		else
		{
			GLMSG::SNETPC_ACTIVITY_UPDATE net_msg_client;
			net_msg_client.sData = sactivity_char_data;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client );
		}
	}

	for( int i=0; i<(int)vecComplete.size(); ++i )
	{
		DWORD dwID = vecComplete[i];

		SACTIVITY_CHAR_DATA_MAP_ITER pos = m_mapActivityProg.find( dwID );
		if ( pos != m_mapActivityProg.end() )
		{
			SACTIVITY_CHAR_DATA sactivity_char_data = (*pos).second;
			
			ActivityComplete( sactivity_char_data );
			
			m_mapActivityProg.erase ( pos );
		}
	}

	if ( vecComplete.size() )
	{
		DoActivityReachPoint();
	}
}

/*activity system, Juver, 2017/10/30 */
void GLChar::DoActivityMobKill( SNATIVEID sidMob )
{
	if ( !RANPARAM::bFeatureActivity )	return;

	std::vector<DWORD> vecComplete;

	for( SACTIVITY_CHAR_DATA_MAP_ITER iter_activity_prog = m_mapActivityProg.begin();
		iter_activity_prog != m_mapActivityProg.end(); ++ iter_activity_prog )
	{
		SACTIVITY_CHAR_DATA& sactivity_char_data = (*iter_activity_prog).second;
		if ( sactivity_char_data.emType != EMACTIVITY_TYPE_KILL_MOB )	continue;
		if ( sactivity_char_data.sidProgress != sidMob )				continue;

		sactivity_char_data.dwProgressNow++;

		if ( sactivity_char_data.dwProgressNow >= sactivity_char_data.dwProgressMax )
		{
			sactivity_char_data.dwProgressNow = sactivity_char_data.dwProgressMax;
			vecComplete.push_back( sactivity_char_data.dwActivityID );
		}
		else
		{
			GLMSG::SNETPC_ACTIVITY_UPDATE net_msg_client;
			net_msg_client.sData = sactivity_char_data;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client );
		}
	}

	for( int i=0; i<(int)vecComplete.size(); ++i )
	{
		DWORD dwID = vecComplete[i];

		SACTIVITY_CHAR_DATA_MAP_ITER pos = m_mapActivityProg.find( dwID );
		if ( pos != m_mapActivityProg.end() )
		{
			SACTIVITY_CHAR_DATA sactivity_char_data = (*pos).second;

			ActivityComplete( sactivity_char_data );

			m_mapActivityProg.erase ( pos );
		}
	}

	if ( vecComplete.size() )
	{
		DoActivityReachPoint();
	}
}

void GLChar::DoActivityMapKill( SNATIVEID sidMap )
{
	if ( !RANPARAM::bFeatureActivity )	return;

	std::vector<DWORD> vecComplete;

	for( SACTIVITY_CHAR_DATA_MAP_ITER iter_activity_prog = m_mapActivityProg.begin();
		iter_activity_prog != m_mapActivityProg.end(); ++ iter_activity_prog )
	{
		SACTIVITY_CHAR_DATA& sactivity_char_data = (*iter_activity_prog).second;
		if ( sactivity_char_data.emType != EMACTIVITY_TYPE_KILL_PLAYER )	continue;
		if ( sactivity_char_data.sidProgress != sidMap )					continue;

		sactivity_char_data.dwProgressNow++;

		if ( sactivity_char_data.dwProgressNow >= sactivity_char_data.dwProgressMax )
		{
			sactivity_char_data.dwProgressNow = sactivity_char_data.dwProgressMax;
			vecComplete.push_back( sactivity_char_data.dwActivityID );
		}
		else
		{
			GLMSG::SNETPC_ACTIVITY_UPDATE net_msg_client;
			net_msg_client.sData = sactivity_char_data;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client );
		}
	}

	for( int i=0; i<(int)vecComplete.size(); ++i )
	{
		DWORD dwID = vecComplete[i];

		SACTIVITY_CHAR_DATA_MAP_ITER pos = m_mapActivityProg.find( dwID );
		if ( pos != m_mapActivityProg.end() )
		{
			SACTIVITY_CHAR_DATA sactivity_char_data = (*pos).second;

			ActivityComplete( sactivity_char_data );

			m_mapActivityProg.erase ( pos );
		}
	}

	if ( vecComplete.size() )
	{
		DoActivityReachPoint();
	}
}

void GLChar::DoActivityMapReach( SNATIVEID sidMap )
{
	if ( !RANPARAM::bFeatureActivity )	return;

	std::vector<DWORD> vecComplete;

	for( SACTIVITY_CHAR_DATA_MAP_ITER iter_activity_prog = m_mapActivityProg.begin();
		iter_activity_prog != m_mapActivityProg.end(); ++ iter_activity_prog )
	{
		SACTIVITY_CHAR_DATA& sactivity_char_data = (*iter_activity_prog).second;
		if ( sactivity_char_data.emType != EMACTIVITY_TYPE_REACH_MAP )	continue;
		if ( sactivity_char_data.sidProgress != sidMap )				continue;

		sactivity_char_data.dwProgressNow++;

		if ( sactivity_char_data.dwProgressNow >= sactivity_char_data.dwProgressMax )
		{
			sactivity_char_data.dwProgressNow = sactivity_char_data.dwProgressMax;
			vecComplete.push_back( sactivity_char_data.dwActivityID );
		}
		else
		{
			GLMSG::SNETPC_ACTIVITY_UPDATE net_msg_client;
			net_msg_client.sData = sactivity_char_data;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client );
		}
	}

	for( int i=0; i<(int)vecComplete.size(); ++i )
	{
		DWORD dwID = vecComplete[i];

		SACTIVITY_CHAR_DATA_MAP_ITER pos = m_mapActivityProg.find( dwID );
		if ( pos != m_mapActivityProg.end() )
		{
			SACTIVITY_CHAR_DATA sactivity_char_data = (*pos).second;

			ActivityComplete( sactivity_char_data );

			m_mapActivityProg.erase ( pos );
		}
	}

	if ( vecComplete.size() )
	{
		DoActivityReachPoint();
	}
}

void GLChar::DoActivityTakeItem( SNATIVEID sidItem, WORD wTurnNum )
{
	if ( !RANPARAM::bFeatureActivity )	return;

	if ( wTurnNum == 0 )	wTurnNum = 1;

	std::vector<DWORD> vecComplete;

	for( SACTIVITY_CHAR_DATA_MAP_ITER iter_activity_prog = m_mapActivityProg.begin();
		iter_activity_prog != m_mapActivityProg.end(); ++ iter_activity_prog )
	{
		SACTIVITY_CHAR_DATA& sactivity_char_data = (*iter_activity_prog).second;
		if ( sactivity_char_data.emType != EMACTIVITY_TYPE_TAKE_ITEM )	continue;
		if ( sactivity_char_data.sidProgress != sidItem )				continue;

		sactivity_char_data.dwProgressNow += wTurnNum;

		if ( sactivity_char_data.dwProgressNow >= sactivity_char_data.dwProgressMax )
		{
			sactivity_char_data.dwProgressNow = sactivity_char_data.dwProgressMax;
			vecComplete.push_back( sactivity_char_data.dwActivityID );
		}
		else
		{
			GLMSG::SNETPC_ACTIVITY_UPDATE net_msg_client;
			net_msg_client.sData = sactivity_char_data;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client );
		}
	}

	for( int i=0; i<(int)vecComplete.size(); ++i )
	{
		DWORD dwID = vecComplete[i];

		SACTIVITY_CHAR_DATA_MAP_ITER pos = m_mapActivityProg.find( dwID );
		if ( pos != m_mapActivityProg.end() )
		{
			SACTIVITY_CHAR_DATA sactivity_char_data = (*pos).second;

			ActivityComplete( sactivity_char_data );

			m_mapActivityProg.erase ( pos );
		}
	}

	if ( vecComplete.size() )
	{
		DoActivityReachPoint();
	}
}

void GLChar::DoActivityUseItem( SNATIVEID sidItem, WORD wUseNum )
{
	if ( !RANPARAM::bFeatureActivity )	return;

	if ( wUseNum == 0 )	wUseNum = 1;

	std::vector<DWORD> vecComplete;

	for( SACTIVITY_CHAR_DATA_MAP_ITER iter_activity_prog = m_mapActivityProg.begin();
		iter_activity_prog != m_mapActivityProg.end(); ++ iter_activity_prog )
	{
		SACTIVITY_CHAR_DATA& sactivity_char_data = (*iter_activity_prog).second;
		if ( sactivity_char_data.emType != EMACTIVITY_TYPE_USE_ITEM )	continue;
		if ( sactivity_char_data.sidProgress != sidItem )				continue;

		sactivity_char_data.dwProgressNow += wUseNum;

		if ( sactivity_char_data.dwProgressNow >= sactivity_char_data.dwProgressMax )
		{
			sactivity_char_data.dwProgressNow = sactivity_char_data.dwProgressMax;
			vecComplete.push_back( sactivity_char_data.dwActivityID );
		}
		else
		{
			GLMSG::SNETPC_ACTIVITY_UPDATE net_msg_client;
			net_msg_client.sData = sactivity_char_data;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client );
		}
	}

	for( int i=0; i<(int)vecComplete.size(); ++i )
	{
		DWORD dwID = vecComplete[i];

		SACTIVITY_CHAR_DATA_MAP_ITER pos = m_mapActivityProg.find( dwID );
		if ( pos != m_mapActivityProg.end() )
		{
			SACTIVITY_CHAR_DATA sactivity_char_data = (*pos).second;

			ActivityComplete( sactivity_char_data );

			m_mapActivityProg.erase ( pos );
		}
	}

	if ( vecComplete.size() )
	{
		DoActivityReachPoint();
	}
}

void GLChar::DoActivityObtainQBox( EMITEM_QUESTION emQuestion )
{
	if ( !RANPARAM::bFeatureActivity )	return;

	std::vector<DWORD> vecComplete;

	for( SACTIVITY_CHAR_DATA_MAP_ITER iter_activity_prog = m_mapActivityProg.begin();
		iter_activity_prog != m_mapActivityProg.end(); ++ iter_activity_prog )
	{
		SACTIVITY_CHAR_DATA& sactivity_char_data = (*iter_activity_prog).second;
		if ( sactivity_char_data.emType != EMACTIVITY_TYPE_QUESTION_BOX )	 continue;
		if ( sactivity_char_data.sidProgress.dwID != (DWORD)emQuestion ) continue;

		sactivity_char_data.dwProgressNow ++;

		if ( sactivity_char_data.dwProgressNow >= sactivity_char_data.dwProgressMax )
		{
			sactivity_char_data.dwProgressNow = sactivity_char_data.dwProgressMax;
			vecComplete.push_back( sactivity_char_data.dwActivityID );
		}
		else
		{
			GLMSG::SNETPC_ACTIVITY_UPDATE net_msg_client;
			net_msg_client.sData = sactivity_char_data;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client );
		}
	}

	for( int i=0; i<(int)vecComplete.size(); ++i )
	{
		DWORD dwID = vecComplete[i];

		SACTIVITY_CHAR_DATA_MAP_ITER pos = m_mapActivityProg.find( dwID );
		if ( pos != m_mapActivityProg.end() )
		{
			SACTIVITY_CHAR_DATA sactivity_char_data = (*pos).second;

			ActivityComplete( sactivity_char_data );

			m_mapActivityProg.erase ( pos );
		}
	}

	if ( vecComplete.size() )
	{
		DoActivityReachPoint();
	}
}

void GLChar::DoActivityCompleteQuest( DWORD dwQuestID )
{
	if ( !RANPARAM::bFeatureActivity )	return;

	std::vector<DWORD> vecComplete;

	for( SACTIVITY_CHAR_DATA_MAP_ITER iter_activity_prog = m_mapActivityProg.begin();
		iter_activity_prog != m_mapActivityProg.end(); ++ iter_activity_prog )
	{
		SACTIVITY_CHAR_DATA& sactivity_char_data = (*iter_activity_prog).second;
		if ( sactivity_char_data.emType != EMACTIVITY_TYPE_COMPLETE_QUEST )	 continue;
		if ( sactivity_char_data.sidProgress.dwID != dwQuestID ) continue;

		sactivity_char_data.dwProgressNow ++;

		if ( sactivity_char_data.dwProgressNow >= sactivity_char_data.dwProgressMax )
		{
			sactivity_char_data.dwProgressNow = sactivity_char_data.dwProgressMax;
			vecComplete.push_back( sactivity_char_data.dwActivityID );
		}
		else
		{
			GLMSG::SNETPC_ACTIVITY_UPDATE net_msg_client;
			net_msg_client.sData = sactivity_char_data;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client );
		}
	}

	for( int i=0; i<(int)vecComplete.size(); ++i )
	{
		DWORD dwID = vecComplete[i];

		SACTIVITY_CHAR_DATA_MAP_ITER pos = m_mapActivityProg.find( dwID );
		if ( pos != m_mapActivityProg.end() )
		{
			SACTIVITY_CHAR_DATA sactivity_char_data = (*pos).second;

			ActivityComplete( sactivity_char_data );

			m_mapActivityProg.erase ( pos );
		}
	}

	if ( vecComplete.size() )
	{
		DoActivityReachPoint();
	}
}

void GLChar::DoActivityReachPoint()
{
	if ( !RANPARAM::bFeatureActivity )	return;

	std::vector<DWORD> vecComplete;

	for( SACTIVITY_CHAR_DATA_MAP_ITER iter_activity_prog = m_mapActivityProg.begin();
		iter_activity_prog != m_mapActivityProg.end(); ++ iter_activity_prog )
	{
		SACTIVITY_CHAR_DATA& sactivity_char_data = (*iter_activity_prog).second;
		if ( sactivity_char_data.emType != EMACTIVITY_TYPE_ACTIVITY_POINT )	continue;

		sactivity_char_data.dwProgressNow = m_dwActivityPoint;

		if ( sactivity_char_data.dwProgressNow >= sactivity_char_data.dwProgressMax )
		{
			sactivity_char_data.dwProgressNow = sactivity_char_data.dwProgressMax;
			vecComplete.push_back( sactivity_char_data.dwActivityID );
		}
		else
		{
			GLMSG::SNETPC_ACTIVITY_UPDATE net_msg_client;
			net_msg_client.sData = sactivity_char_data;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client );
		}
	}

	for( int i=0; i<(int)vecComplete.size(); ++i )
	{
		DWORD dwID = vecComplete[i];

		SACTIVITY_CHAR_DATA_MAP_ITER pos = m_mapActivityProg.find( dwID );
		if ( pos != m_mapActivityProg.end() )
		{
			SACTIVITY_CHAR_DATA sactivity_char_data = (*pos).second;

			ActivityComplete( sactivity_char_data );

			m_mapActivityProg.erase ( pos );
		}
	}

	if ( vecComplete.size() )
	{
		DoActivityReachPoint();
	}
}

/*activity system, Juver, 2017/11/05 */
HRESULT GLChar::ReqCharacterBadgeChange( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_CHARACTER_BADGE_CHANGE* pnet_msg_client = (GLMSG::SNETPC_REQ_CHARACTER_BADGE_CHANGE*)nmg;
	GLMSG::SNETPC_REQ_CHARACTER_BADGE_CHANGE_FB	net_msg_fb;

	SACTIVITY_FILE_DATA* pactivity_file_data = GLActivity::GetInstance().GetActivity( pnet_msg_client->dwBadgeID );
	if ( pactivity_file_data )
	{
		if ( !pactivity_file_data->bRewardBadge )
		{
			net_msg_fb.emFB = EMFB_CHAR_TITLE_FAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
			return E_FAIL;
		}

		if ( strcmp(m_szBadge, pactivity_file_data->strBadgeString.c_str() ) == 0 )
		{
			net_msg_fb.emFB = EMFB_CHAR_TITLE_SAME;
			GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
			return E_FAIL;
		}

		StringCchCopy ( m_szBadge, CHAR_SZNAME, pactivity_file_data->strBadgeString.c_str() );
	}
	else
	{
		StringCchCopy ( m_szBadge, CHAR_SZNAME, "" );
	}
	

	net_msg_fb.emFB = EMFB_CHAR_TITLE_DONE;
	StringCchCopy ( net_msg_fb.szBadge, CHAR_SZNAME, m_szBadge );
	GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );

	GLMSG::SNETPC_REQ_CHARACTER_BADGE_CHANGE_BRD net_msg_client_brd;
	net_msg_client_brd.dwGaeaID = m_dwGaeaID;
	StringCchCopy ( net_msg_client_brd.szBadge, CHAR_SZNAME, m_szBadge );
	SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&net_msg_client_brd) );

	return S_OK;
}