#include "stdafx.h"
#include "./GLCrowDataNPCShop.h"
#include "./GLItemMan.h"
#include "./GLogicData.h"

#include "../../Lib_Engine/Common/IniLoader.h"
#include "../../Lib_Engine/G-Logic/GLOGIC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL SNPC_SHOP::LoadData( std::string strShop )
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
		CDebugSet::ErrorFile( "SNPC_SHOP::LoadData", strPath.c_str() );
		return false;
	}

	mapType.clear();
	mapItem.clear();

	cFILE.getflag( "SHOP_BASIC", "SHOP_TITLE", 0, 1, strTitle );
	cFILE.getflag( "SHOP_BASIC", "SHOP_OPTION", 0, 1, wShopType );

	DWORD dwtype = cFILE.GetKeySize( "SHOP_TYPE_LIST", "SHOP_TYPE" );
	for ( DWORD i=0; i<dwtype; ++i )
	{
		SNPC_SHOP_TYPE npc_shop_type;
		cFILE.getflag( i, "SHOP_TYPE_LIST", "SHOP_TYPE", 0, 2, npc_shop_type.wID );
		cFILE.getflag( i, "SHOP_TYPE_LIST", "SHOP_TYPE", 1, 2, npc_shop_type.strName );
	
		mapType.insert( std::make_pair( npc_shop_type.wID, npc_shop_type ) );
	}

	DWORD dwitem = cFILE.GetKeySize( "SHOP_ITEM_LIST", "SHOP_ITEM" );
	for ( DWORD i=0; i<dwitem; ++i )
	{
		SNPC_SHOP_ITEM npc_shop_item;
		cFILE.getflag( i, "SHOP_ITEM_LIST", "SHOP_ITEM", 0, 11, npc_shop_item.wType );
		cFILE.getflag( i, "SHOP_ITEM_LIST", "SHOP_ITEM", 1, 11, npc_shop_item.sidItem.wMainID );
		cFILE.getflag( i, "SHOP_ITEM_LIST", "SHOP_ITEM", 2, 11, npc_shop_item.sidItem.wSubID );

		cFILE.getflag( i, "SHOP_ITEM_LIST", "SHOP_ITEM", 3, 11, npc_shop_item.cDAMAGE );
		cFILE.getflag( i, "SHOP_ITEM_LIST", "SHOP_ITEM", 4, 11, npc_shop_item.cDEFENSE );
		cFILE.getflag( i, "SHOP_ITEM_LIST", "SHOP_ITEM", 5, 11, npc_shop_item.cRESIST_FIRE );
		cFILE.getflag( i, "SHOP_ITEM_LIST", "SHOP_ITEM", 6, 11, npc_shop_item.cRESIST_ICE );
		cFILE.getflag( i, "SHOP_ITEM_LIST", "SHOP_ITEM", 7, 11, npc_shop_item.cRESIST_ELEC );
		cFILE.getflag( i, "SHOP_ITEM_LIST", "SHOP_ITEM", 8, 11, npc_shop_item.cRESIST_POISON );
		cFILE.getflag( i, "SHOP_ITEM_LIST", "SHOP_ITEM", 9, 11, npc_shop_item.cRESIST_SPIRIT );
		cFILE.getflag( i, "SHOP_ITEM_LIST", "SHOP_ITEM", 10, 11, npc_shop_item.bGenerateRandomValue );

		SNPC_SHOP_TYPE* pType = findShopType( npc_shop_item.wType );
		if ( !pType )	continue;
		pType->wItemNum ++;

		mapItem.insert( std::make_pair( npc_shop_item.sidItem.dwID, npc_shop_item ) );
	}

	return TRUE;
}

SNPC_SHOP_TYPE* SNPC_SHOP::findShopType( WORD wType )
{
	SNPC_SHOP_TYPE_MAP_ITER it = mapType.find( wType );
	if( it != mapType.end() )
		return &(*it).second;

	return NULL;
}

SNPC_SHOP_ITEM* SNPC_SHOP::findShopItem( DWORD dwItemID )
{
	SNPC_SHOP_ITEM_MAP_ITER it = mapItem.find( dwItemID );
	if( it != mapItem.end() )
		return &(*it).second;

	return NULL;
}
