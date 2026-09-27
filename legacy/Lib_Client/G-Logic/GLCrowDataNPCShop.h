/*!
 * \file GLCrowDataNPCShop.h
 *
 * \author Juver
 * \date July 2017
 *
 * 
 */

#ifndef GLCROWDATA_NPC_SHOP_H_
#define GLCROWDATA_NPC_SHOP_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "./GLCharDefine.h"

struct SNPC_SHOP_TYPE
{
	WORD		wID;
	std::string strName;
	WORD		wItemNum;

	SNPC_SHOP_TYPE()
		: wID(0)
		, strName("")
		, wItemNum(0)
	{
	};
};

struct SNPC_SHOP_ITEM
{
	SNATIVEID	sidItem;
	WORD		wType;
	
	BYTE	cDAMAGE;
	BYTE	cDEFENSE;
	BYTE	cRESIST_FIRE;
	BYTE	cRESIST_ICE;
	BYTE	cRESIST_ELEC;
	BYTE	cRESIST_POISON;	
	BYTE	cRESIST_SPIRIT;
	BOOL	bGenerateRandomValue;

	SNPC_SHOP_ITEM()
		: sidItem(false)
		, wType(0)
		, cDAMAGE(0)
		, cDEFENSE(0)
		, cRESIST_FIRE(0)
		, cRESIST_ICE(0)
		, cRESIST_ELEC(0)
		, cRESIST_POISON(0)
		, cRESIST_SPIRIT(0)
		, bGenerateRandomValue(FALSE)
	{
	};

	bool operator < ( const SNPC_SHOP_ITEM& sData )
	{			
		if ( sidItem.wMainID < sData.sidItem.wMainID ) 
			return true;

		else if ( sidItem.wMainID == sData.sidItem.wMainID && 
			sidItem.wSubID < sData.sidItem.wSubID ) 
			return true;

		return  false;
	}
};

typedef std::map<WORD,SNPC_SHOP_TYPE>		SNPC_SHOP_TYPE_MAP;
typedef SNPC_SHOP_TYPE_MAP::iterator		SNPC_SHOP_TYPE_MAP_ITER;

typedef std::map<DWORD,SNPC_SHOP_ITEM>		SNPC_SHOP_ITEM_MAP;
typedef SNPC_SHOP_ITEM_MAP::iterator		SNPC_SHOP_ITEM_MAP_ITER;
typedef std::vector<SNPC_SHOP_ITEM>			SNPC_SHOP_ITEM_VEC;

struct SNPC_SHOP
{
	std::string strTitle;	//shop window title
	WORD		wShopType;	//shop type default 0 for gold

	SNPC_SHOP_TYPE_MAP		mapType;
	SNPC_SHOP_ITEM_MAP		mapItem;

	SNPC_SHOP()
		: strTitle("Shop")
		, wShopType(0)
	{

	}

	SNPC_SHOP ( const SNPC_SHOP &value )
	{
		operator=(value);
	}

	SNPC_SHOP& operator = ( const SNPC_SHOP& rvalue )
	{
		strTitle	= rvalue.strTitle;
		wShopType	= rvalue.wShopType;
		mapType		= rvalue.mapType;
		mapItem		= rvalue.mapItem;
		
		return *this;
	}

	BOOL LoadData( std::string strShop );
	SNPC_SHOP_TYPE*	findShopType( WORD wType );
	SNPC_SHOP_ITEM*	findShopItem( DWORD dwItemID );
};

#endif // GLCROWDATA_NPC_SHOP_H_