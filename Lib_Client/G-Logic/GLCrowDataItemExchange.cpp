#include "stdafx.h"
#include "./GLCrowDataItemExchange.h"
#include "./GLItemMan.h"
#include "./GLogicData.h"

#include "../../Lib_Engine/Common/IniLoader.h"
#include "../../Lib_Engine/G-Logic/GLOGIC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL SNPC_ITEM_EXCHANGE::LoadData( std::string strShop )
{
	if( strShop.empty() )		return FALSE;


	std::string strPath;
	strPath = GLOGIC::GetPath();
	strPath += strShop;

	CIniLoader cFILE;

	if( GLOGIC::bGLOGIC_ZIPFILE )
		cFILE.SetZipFile( GLOGIC::strGLOGIC_ZIPFILE );

	std::string strSep( ",[]\t" );
	cFILE.reg_sep( strSep );

	if( !cFILE.open( strPath, true, GLOGIC::bGLOGIC_PACKFILE ) )
	{
		CDebugSet::ErrorFile( "SNPC_ITEM_EXCHANGE::LoadData", strPath.c_str() );
		return false;
	}

	mapItem.clear();

	cFILE.getflag( "NPC_EXCHANGE_BASIC", "NPC_EXCHANGE_TITLE", 0, 1, strTitle );

	DWORD dwitem = cFILE.GetKeySize( "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM" );
	for ( DWORD i=0; i<dwitem; ++i )
	{
		SNPC_ITEM_EXCHANGE_DATA npc_item_exchange_data;
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 0, 27, npc_item_exchange_data.wID );

		//result item
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 1, 27, npc_item_exchange_data.sidItemResult.sidItem.wMainID );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 2, 27, npc_item_exchange_data.sidItemResult.sidItem.wSubID );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 3, 27, npc_item_exchange_data.sidItemResult.wItemNum );

		//result item stats
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 4, 27, npc_item_exchange_data.cDAMAGE );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 5, 27, npc_item_exchange_data.cDEFENSE );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 6, 27, npc_item_exchange_data.cRESIST_FIRE );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 7, 27, npc_item_exchange_data.cRESIST_ICE );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 8, 27, npc_item_exchange_data.cRESIST_ELEC );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 9, 27, npc_item_exchange_data.cRESIST_POISON );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 10, 27, npc_item_exchange_data.cRESIST_SPIRIT );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 11, 27, npc_item_exchange_data.bGenerateRandomValue );

		//require item
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 12, 27, npc_item_exchange_data.sidItemRequire[0].sidItem.wMainID );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 13, 27, npc_item_exchange_data.sidItemRequire[0].sidItem.wSubID );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 14, 27, npc_item_exchange_data.sidItemRequire[0].wItemNum );

		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 15, 27, npc_item_exchange_data.sidItemRequire[1].sidItem.wMainID );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 16, 27, npc_item_exchange_data.sidItemRequire[1].sidItem.wSubID );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 17, 27, npc_item_exchange_data.sidItemRequire[1].wItemNum );

		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 18, 27, npc_item_exchange_data.sidItemRequire[2].sidItem.wMainID );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 19, 27, npc_item_exchange_data.sidItemRequire[2].sidItem.wSubID );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 20, 27, npc_item_exchange_data.sidItemRequire[2].wItemNum );

		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 21, 27, npc_item_exchange_data.sidItemRequire[3].sidItem.wMainID );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 22, 27, npc_item_exchange_data.sidItemRequire[3].sidItem.wSubID );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 23, 27, npc_item_exchange_data.sidItemRequire[3].wItemNum );

		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 24, 27, npc_item_exchange_data.sidItemRequire[4].sidItem.wMainID );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 25, 27, npc_item_exchange_data.sidItemRequire[4].sidItem.wSubID );
		cFILE.getflag( i, "NPC_EXCHANGE_LIST", "NPC_EXCHANGE_ITEM", 26, 27, npc_item_exchange_data.sidItemRequire[4].wItemNum );

		
		if ( npc_item_exchange_data.sidItemResult.wItemNum < 1 )		npc_item_exchange_data.sidItemResult.wItemNum = 1;
		if ( npc_item_exchange_data.sidItemRequire[0].wItemNum < 1 )	npc_item_exchange_data.sidItemRequire[0].wItemNum = 1;
		if ( npc_item_exchange_data.sidItemRequire[1].wItemNum < 1 )	npc_item_exchange_data.sidItemRequire[1].wItemNum = 1;
		if ( npc_item_exchange_data.sidItemRequire[2].wItemNum < 1 )	npc_item_exchange_data.sidItemRequire[2].wItemNum = 1;
		if ( npc_item_exchange_data.sidItemRequire[3].wItemNum < 1 )	npc_item_exchange_data.sidItemRequire[3].wItemNum = 1;
		if ( npc_item_exchange_data.sidItemRequire[4].wItemNum < 1 )	npc_item_exchange_data.sidItemRequire[4].wItemNum = 1;

		SNPC_ITEM_EXCHANGE_DATA* pType = findData( npc_item_exchange_data.wID );
		if ( pType )	continue;

		mapItem.insert( std::make_pair( npc_item_exchange_data.wID, npc_item_exchange_data ) );
	}

	return TRUE;
}

SNPC_ITEM_EXCHANGE_DATA* SNPC_ITEM_EXCHANGE::findData( WORD wExchangeID )
{
	SNPC_ITEM_EXCHANGE_DATA_MAP_ITER it = mapItem.find( wExchangeID );
	if( it != mapItem.end() )
		return &(*it).second;

	return NULL;
}
