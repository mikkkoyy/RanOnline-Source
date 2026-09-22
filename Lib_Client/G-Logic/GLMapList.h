#ifndef GLMAPLIST_H_
#define GLMAPLIST_H_

#include <map>
#include <string>

#include "./GLLevelFile.h"
#include "./GLogicData.h"

#include "../../Lib_Engine/Common/CList.h"
#include "../../Lib_Network/s_NetGlobal.h"

#include "./GLMapNode.h"


class GLMapList
{
public:
	enum
	{
		VERSION				= 0x0200,		
		VERSION_ENCODE		= 0x0200,
		VERSION_ENCODE_OLD	= 0x0100,
		MAPLIST_MAX			= 1000,
	};

public:
	static const char*	_FILEHEAD;
	char				m_szFileName[MAX_PATH];

public:
	typedef std::map<DWORD,SMAPNODE_DATA>		FIELDMAP_DATA;
	typedef FIELDMAP_DATA::iterator				FIELDMAP_DATA_ITER;

	typedef std::map<DWORD,SMAPNODE>	FIELDMAP;
	typedef FIELDMAP::iterator					FIELDMAP_ITER;

	typedef std::vector<DWORD>				VEC_INSTANT_MAPID;
	typedef VEC_INSTANT_MAPID::iterator		VEC_INSTANT_MAPID_ITER;

public:
	FIELDMAP_DATA	m_mapListData;

protected:
	FIELDMAP	m_MapList;

public:
	BOOL IsRegMapNode ( SMAPNODE &MapNode );
	HRESULT LoadMapList_OLD ( const TCHAR* szMapListFile );
	HRESULT CleanUp ();

	HRESULT LoadMapsListFile ( const char* szFile, LPF_SERVER_INFO pServerInfo=NULL, int nServerNum=0, BOOL bTOOL = FALSE );
	HRESULT SaveMapsListFile ( const char* szFile );

public:
	SMAPNODE* FindMapNode ( const SNATIVEID &sNativeID );
	const char* GetMapName ( const SNATIVEID &sNativeID );
	const SLEVEL_REQUIRE*  GetLevelRequire ( const SNATIVEID &sNativeID );
	const SLEVEL_ETC_FUNC* GetLevelEtcFunc ( const SNATIVEID &sNativeID );

	FIELDMAP& GetMapList ()						{ return m_MapList; }

	BOOL InsertMapList( SMAPNODE &mapnew );
	BOOL EraseMapList( SNATIVEID sidNew );

public:
	GLMapList(void);
	~GLMapList(void);
};


#endif // GLMAPLIST_H_