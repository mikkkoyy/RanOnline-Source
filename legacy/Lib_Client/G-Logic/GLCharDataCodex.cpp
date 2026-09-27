#include "stdafx.h"
#include "./GLCharData.h"
#include "./GLItemMan.h"
#include "./GLCodex.h"

#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*Codex System, Jhoniex 2/14/2024 */
BOOL SCHARDATA2::GETCODEX_BYBUF( CByteStream &ByteStream ) const
{
	ByteStream.ClearBuffer ();

	ByteStream << (DWORD)SCODEX_CHAR_DATA::VERSION;

	ByteStream << (DWORD) m_mapCodexProg.size();
	for ( SCODEX_CHAR_DATA_MAP_CITER pos = m_mapCodexProg.begin(); 
		pos != m_mapCodexProg.end(); ++pos )
	{
		const SCODEX_CHAR_DATA* pCodex = &pos->second;
		ByteStream.WriteBuffer ( (LPBYTE)pCodex, sizeof(SCODEX_CHAR_DATA) );
	}

	ByteStream << (DWORD) m_mapCodexDone.size();
	for ( SCODEX_CHAR_DATA_MAP_CITER pos = m_mapCodexDone.begin(); 
		pos != m_mapCodexDone.end(); ++pos )
	{
		const SCODEX_CHAR_DATA* pCodex = &pos->second;
		ByteStream.WriteBuffer ( (LPBYTE)pCodex, sizeof(SCODEX_CHAR_DATA) );
	}


	return TRUE;
}

/*Codex System, Jhoniex 2/14/2024 */
BOOL SCHARDATA2::SETCODEX_BYBUF( CByteStream &ByteStream )
{
	if ( ByteStream.IsEmpty() )	return TRUE;

	DWORD dwVersion, dwNum;
	ByteStream >> dwVersion;

	if ( dwVersion == SCODEX_CHAR_DATA::VERSION )
	{
		ByteStream >> dwNum;
		for ( DWORD i=0; i<dwNum; i++ )
		{
			SCODEX_CHAR_DATA sCodex;
			ByteStream.ReadBuffer ( (LPBYTE)&sCodex, sizeof(SCODEX_CHAR_DATA) );
			m_mapCodexProg.insert( std::make_pair( sCodex.dwCodexID, sCodex ) );
		}

		ByteStream >> dwNum;
		for ( DWORD i=0; i<dwNum; i++ )
		{
			SCODEX_CHAR_DATA sCodex;
			ByteStream.ReadBuffer ( (LPBYTE)&sCodex, sizeof(SCODEX_CHAR_DATA) );
			m_mapCodexDone.insert( std::make_pair( sCodex.dwCodexID, sCodex ) );
		}
	}
	else
	{
		CDebugSet::ErrorVersion( "SCHARDATA2::SETCODEX_BYBUF", dwVersion );
	}

	//if ( !RANPARAM::bFeatureCodex )	return TRUE;

	//insert
	SCODEX_FILE_DATA_MAP &map_activity_file_data = GLCodex::GetInstance().m_mapCodex;
	for( SCODEX_FILE_DATA_MAP_ITER pos = map_activity_file_data.begin();
		pos != map_activity_file_data.end(); ++pos )
	{
		SCODEX_FILE_DATA &sactivity_data = (*pos).second; 

		SCODEX_CHAR_DATA_MAP_ITER iter_done = m_mapCodexDone.find( sactivity_data.dwCodexID );
		if ( iter_done != m_mapCodexDone.end() )	continue;

		SCODEX_CHAR_DATA_MAP_ITER iter_prog = m_mapCodexProg.find( sactivity_data.dwCodexID );
		if ( iter_prog != m_mapCodexProg.end() )
		{
			SCODEX_CHAR_DATA& sactivity_cur = (*iter_prog).second;
			sactivity_cur.Correction( sactivity_data );
			continue;
		}

		SCODEX_CHAR_DATA sactivity_char_data;
		sactivity_char_data.Assign( sactivity_data );
		m_mapCodexProg.insert( std::make_pair( sactivity_char_data.dwCodexID, sactivity_char_data ) );
	}

	std::vector<DWORD> vecDelete;

	for( SCODEX_CHAR_DATA_MAP_ITER pos_prog_find = m_mapCodexProg.begin();
		pos_prog_find != m_mapCodexProg.end(); ++pos_prog_find )
	{
		SCODEX_CHAR_DATA &sactivity_data = (*pos_prog_find).second; 
 
		SCODEX_FILE_DATA* pactivity_char = GLCodex::GetInstance().GetCodex( sactivity_data.dwCodexID );
		if ( !pactivity_char )
		{
			vecDelete.push_back( sactivity_data.dwCodexID );
		}
	}

	for( SCODEX_CHAR_DATA_MAP_ITER pos_done_find = m_mapCodexDone.begin();
		pos_done_find != m_mapCodexDone.end(); ++pos_done_find )
	{
		SCODEX_CHAR_DATA &sactivity_data = (*pos_done_find).second; 

		SCODEX_FILE_DATA* pactivity_char = GLCodex::GetInstance().GetCodex( sactivity_data.dwCodexID );
		if ( !pactivity_char )
		{
			vecDelete.push_back( sactivity_data.dwCodexID );
		}
	}

	for( int i=0; i<(int)vecDelete.size(); ++i )
	{
		DWORD dwID = vecDelete[i];

		SCODEX_CHAR_DATA_MAP_ITER pos_prog = m_mapCodexProg.find( dwID );
		if ( pos_prog != m_mapCodexProg.end() )
			m_mapCodexProg.erase( pos_prog );

		SCODEX_CHAR_DATA_MAP_ITER pos_done = m_mapCodexDone.find( dwID );
		if ( pos_done != m_mapCodexDone.end() )
			m_mapCodexDone.erase( pos_done );
	}

	return TRUE;
}