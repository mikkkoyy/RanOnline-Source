#include "stdafx.h"
#include "./GLCharData.h"
#include "./GLItemMan.h"
#include "./GLActivity.h"

#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*activity system, Juver, 2017/10/23 */
BOOL SCHARDATA2::GETACTIVITY_BYBUF( CByteStream &ByteStream ) const
{
	ByteStream.ClearBuffer ();

	ByteStream << (DWORD)SACTIVITY_CHAR_DATA::VERSION;

	ByteStream << (DWORD) m_mapActivityProg.size();
	for ( SACTIVITY_CHAR_DATA_MAP_CITER pos = m_mapActivityProg.begin(); 
		pos != m_mapActivityProg.end(); ++pos )
	{
		const SACTIVITY_CHAR_DATA* pActivity = &pos->second;
		ByteStream.WriteBuffer ( (LPBYTE)pActivity, sizeof(SACTIVITY_CHAR_DATA) );
	}

	ByteStream << (DWORD) m_mapActivityDone.size();
	for ( SACTIVITY_CHAR_DATA_MAP_CITER pos = m_mapActivityDone.begin(); 
		pos != m_mapActivityDone.end(); ++pos )
	{
		const SACTIVITY_CHAR_DATA* pActivity = &pos->second;
		ByteStream.WriteBuffer ( (LPBYTE)pActivity, sizeof(SACTIVITY_CHAR_DATA) );
	}


	return TRUE;
}

/*activity system, Juver, 2017/10/23 */
BOOL SCHARDATA2::SETACTIVITY_BYBUF( CByteStream &ByteStream )
{
	if ( ByteStream.IsEmpty() )	return TRUE;

	DWORD dwVersion, dwNum;
	ByteStream >> dwVersion;

	if ( dwVersion == SACTIVITY_CHAR_DATA::VERSION )
	{
		ByteStream >> dwNum;
		for ( DWORD i=0; i<dwNum; i++ )
		{
			SACTIVITY_CHAR_DATA sActivity;
			ByteStream.ReadBuffer ( (LPBYTE)&sActivity, sizeof(SACTIVITY_CHAR_DATA) );
			m_mapActivityProg.insert( std::make_pair( sActivity.dwActivityID, sActivity ) );
		}

		ByteStream >> dwNum;
		for ( DWORD i=0; i<dwNum; i++ )
		{
			SACTIVITY_CHAR_DATA sActivity;
			ByteStream.ReadBuffer ( (LPBYTE)&sActivity, sizeof(SACTIVITY_CHAR_DATA) );
			m_mapActivityDone.insert( std::make_pair( sActivity.dwActivityID, sActivity ) );
		}
	}
	else
	{
		CDebugSet::ErrorVersion( "SCHARDATA2::SETACTIVITY_BYBUF", dwVersion );
	}

	if ( !RANPARAM::bFeatureActivity )	return TRUE;

	//insert
	SACTIVITY_FILE_DATA_MAP &map_activity_file_data = GLActivity::GetInstance().m_mapActivity;
	for( SACTIVITY_FILE_DATA_MAP_ITER pos = map_activity_file_data.begin();
		pos != map_activity_file_data.end(); ++pos )
	{
		SACTIVITY_FILE_DATA &sactivity_data = (*pos).second; 

		SACTIVITY_CHAR_DATA_MAP_ITER iter_done = m_mapActivityDone.find( sactivity_data.dwActivityID );
		if ( iter_done != m_mapActivityDone.end() )	continue;

		SACTIVITY_CHAR_DATA_MAP_ITER iter_prog = m_mapActivityProg.find( sactivity_data.dwActivityID );
		if ( iter_prog != m_mapActivityProg.end() )
		{
			SACTIVITY_CHAR_DATA& sactivity_cur = (*iter_prog).second;
			sactivity_cur.Correction( sactivity_data );
			continue;
		}

		SACTIVITY_CHAR_DATA sactivity_char_data;
		sactivity_char_data.Assign( sactivity_data );
		m_mapActivityProg.insert( std::make_pair( sactivity_char_data.dwActivityID, sactivity_char_data ) );
	}

	std::vector<DWORD> vecDelete;

	for( SACTIVITY_CHAR_DATA_MAP_ITER pos_prog_find = m_mapActivityProg.begin();
		pos_prog_find != m_mapActivityProg.end(); ++pos_prog_find )
	{
		SACTIVITY_CHAR_DATA &sactivity_data = (*pos_prog_find).second; 
 
		SACTIVITY_FILE_DATA* pactivity_char = GLActivity::GetInstance().GetActivity( sactivity_data.dwActivityID );
		if ( !pactivity_char )
		{
			vecDelete.push_back( sactivity_data.dwActivityID );
		}
	}

	for( SACTIVITY_CHAR_DATA_MAP_ITER pos_done_find = m_mapActivityDone.begin();
		pos_done_find != m_mapActivityDone.end(); ++pos_done_find )
	{
		SACTIVITY_CHAR_DATA &sactivity_data = (*pos_done_find).second; 

		SACTIVITY_FILE_DATA* pactivity_char = GLActivity::GetInstance().GetActivity( sactivity_data.dwActivityID );
		if ( !pactivity_char )
		{
			vecDelete.push_back( sactivity_data.dwActivityID );
		}
	}

	for( int i=0; i<(int)vecDelete.size(); ++i )
	{
		DWORD dwID = vecDelete[i];

		SACTIVITY_CHAR_DATA_MAP_ITER pos_prog = m_mapActivityProg.find( dwID );
		if ( pos_prog != m_mapActivityProg.end() )
			m_mapActivityProg.erase( pos_prog );

		SACTIVITY_CHAR_DATA_MAP_ITER pos_done = m_mapActivityDone.find( dwID );
		if ( pos_done != m_mapActivityDone.end() )
			m_mapActivityDone.erase( pos_done );
	}

	return TRUE;
}