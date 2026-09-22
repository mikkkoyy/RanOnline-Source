#include "stdafx.h"
#include "./GLChar.h"
#include "./GLGaeaServer.h"
#include "./GLItemMan.h"
#include "./GLCodex.h"

#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*Codex System, Jhoniex 2/14/2024 */
void GLChar::CodexComplete( SCODEX_CHAR_DATA& scodex_char_data )
{
	SCODEX_FILE_DATA* pcodex_file_data = GLCodex::GetInstance().GetCodex( scodex_char_data.dwCodexID );
	if ( !pcodex_file_data )	return;

	m_mapCodexDone.insert( std::make_pair( scodex_char_data.dwCodexID, scodex_char_data ) );

	GLMSG::SNETPC_CODEX_COMPLETE net_msg_client;
	net_msg_client.sData = scodex_char_data;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client );

	GLMSG::SNETPC_CODEX_COMPLETE_BRD net_msg_client_brd;
	net_msg_client_brd.dwGaeaID = m_dwGaeaID;
	SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&net_msg_client_brd) );

	switch( pcodex_file_data->emNotify )
	{
	case EMCODEX_NOTIFY_NONE:
		{
			GLMSG::SNETPC_CODEX_NOTIFY_CLIENT net_msg_client;
			StringCchCopy( net_msg_client.szCharacterName, CHAR_SZNAME+1, m_szName );
			StringCchCopy( net_msg_client.szCodexTitle, CODEX_TITLE_MSG_SIZE+1, pcodex_file_data->strCodexTitle.c_str() );
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client );

		}break;

	case EMCODEX_NOTIFY_AROUND:
		{
			GLMSG::SNETPC_CODEX_NOTIFY_CLIENT net_msg_client;
			StringCchCopy( net_msg_client.szCharacterName, CHAR_SZNAME+1, m_szName );
			StringCchCopy( net_msg_client.szCodexTitle, CODEX_TITLE_MSG_SIZE+1, pcodex_file_data->strCodexTitle.c_str() );
			GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMapID.dwID, &net_msg_client );
		}break;

	case EMCODEX_NOTIFY_ALLSERVER:
		{
			GLMSG::SNETPC_CODEX_NOTIFY_AGENT net_msg_brd;
			StringCchCopy( net_msg_brd.szCharacterName, CHAR_SZNAME+1, m_szName );
			StringCchCopy( net_msg_brd.szCodexTitle, CODEX_TITLE_MSG_SIZE+1, pcodex_file_data->strCodexTitle.c_str() );
			GLGaeaServer::GetInstance().SENDTOAGENT( &net_msg_brd );;
		}break;
	};
	
	CODEX_STATS(1,1);//send to client-serverside
}

void GLChar::DoCodexRegisterItem( SNATIVEID sidItem, DWORD dwCodexID, WORD wTurnNum, WORD wGrade )
{
	if ( wTurnNum == 0 )	wTurnNum = 1;
	
	int wReturn = 1;

	std::vector<DWORD> vecComplete;
	for( SCODEX_CHAR_DATA_MAP_ITER iter_codex_prog = m_mapCodexProg.begin();
		iter_codex_prog != m_mapCodexProg.end(); ++ iter_codex_prog )
	{
		SCODEX_CHAR_DATA& scodex_char_data = (*iter_codex_prog).second;

		//if ( scodex_char_data.emType != EMCODEX_TYPE_TAKE_ITEM )	continue;
		if ( scodex_char_data.dwCodexID != dwCodexID ) continue;
		
		if ( scodex_char_data.sidProgressItem1 == sidItem )
		{
			if ( scodex_char_data.dwProgressItemDone1 != 1 )
			{
				if ( scodex_char_data.wItemGrade1 == wGrade )
				{
					if (scodex_char_data.wQuantity1 == wTurnNum)
					{
						scodex_char_data.dwProgressItemDone1 = 1;
						scodex_char_data.dwProgressNow++;
					}
				}
			}
		}
		if ( scodex_char_data.sidProgressItem2 == sidItem )
		{
			if ( scodex_char_data.dwProgressItemDone2 != 1 )
			{
				if ( scodex_char_data.wItemGrade2 == wGrade )
				{
					if (scodex_char_data.wQuantity2 == wTurnNum)
					{
						scodex_char_data.dwProgressItemDone2 = 1;
						scodex_char_data.dwProgressNow++;
					}
				}
			}
		}
		if ( scodex_char_data.sidProgressItem3 == sidItem )
		{
			if ( scodex_char_data.dwProgressItemDone3 != 1 )
			{
				if ( scodex_char_data.wItemGrade3 == wGrade )
				{
					if (scodex_char_data.wQuantity3 == wTurnNum)
					{
						scodex_char_data.dwProgressItemDone3 = 1;
						scodex_char_data.dwProgressNow++;
					}
				}
			}
		}
		if ( scodex_char_data.sidProgressItem4 == sidItem )
		{
			if ( scodex_char_data.dwProgressItemDone4 != 1 )
			{
				if ( scodex_char_data.wItemGrade4 == wGrade )
				{
					if (scodex_char_data.wQuantity4 == wTurnNum)
					{
						scodex_char_data.dwProgressItemDone4 = 1;
						scodex_char_data.dwProgressNow++;
					}
				}
			}
		}
		if ( scodex_char_data.sidProgressItem5 == sidItem )
		{
			if ( scodex_char_data.dwProgressItemDone5 != 1 )
			{
				if ( scodex_char_data.wItemGrade5 == wGrade )
				{
					if (scodex_char_data.wQuantity5 == wTurnNum)
					{
						scodex_char_data.dwProgressItemDone5 = 1;
						scodex_char_data.dwProgressNow++;
					}
				}
			}
		}

		
		if ( scodex_char_data.dwProgressNow >= scodex_char_data.dwProgressMax )
		{
			scodex_char_data.dwProgressNow = scodex_char_data.dwProgressMax;
			vecComplete.push_back( scodex_char_data.dwCodexID );
		}
		else
		{
			GLMSG::SNETPC_CODEX_UPDATE net_msg_client;
			net_msg_client.sData = scodex_char_data;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client );
		}
	}


	for( int i=0; i<(int)vecComplete.size(); ++i )
	{
		DWORD dwID = vecComplete[i];

		SCODEX_CHAR_DATA_MAP_ITER pos = m_mapCodexProg.find( dwID );
		if ( pos != m_mapCodexProg.end() )
		{
			SCODEX_CHAR_DATA scodex_char_data = (*pos).second;

			CodexComplete( scodex_char_data );
			m_mapCodexProg.erase ( pos );
		}
	}

	if ( vecComplete.size() )
	{
		//DoCodexReachPoint();
	}
}