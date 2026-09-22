/*!
 * \file GLCrowDataItemExchange.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */

#ifndef GLCROWDATA_ITEM_EXCHANGE_H_
#define GLCROWDATA_ITEM_EXCHANGE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "./GLCharDefine.h"

#define NPC_ITEM_EXCHANGE_MAX_REQ	5
#define NPC_ITEM_EXCHANGE_NULL		512

struct SNPC_ITEM_EXCHANGE_ITEM
{
	SNATIVEID	sidItem;
	WORD		wItemNum;

	SNPC_ITEM_EXCHANGE_ITEM()
		: sidItem(false)
		, wItemNum(1)
	{
	};
};

struct SNPC_ITEM_EXCHANGE_DATA
{
	WORD	wID;
	SNPC_ITEM_EXCHANGE_ITEM	sidItemResult;
	SNPC_ITEM_EXCHANGE_ITEM	sidItemRequire[NPC_ITEM_EXCHANGE_MAX_REQ];
	
	BYTE	cDAMAGE;
	BYTE	cDEFENSE;
	BYTE	cRESIST_FIRE;
	BYTE	cRESIST_ICE;
	BYTE	cRESIST_ELEC;
	BYTE	cRESIST_POISON;	
	BYTE	cRESIST_SPIRIT;
	BOOL	bGenerateRandomValue;

	SNPC_ITEM_EXCHANGE_DATA()
		: wID( NPC_ITEM_EXCHANGE_NULL )
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

	bool operator < ( const SNPC_ITEM_EXCHANGE_DATA& sData )
	{			
		if ( wID < sData.wID ) 
			return true;

		return  false;
	}
};


typedef std::map<WORD,SNPC_ITEM_EXCHANGE_DATA>		SNPC_ITEM_EXCHANGE_DATA_MAP;
typedef SNPC_ITEM_EXCHANGE_DATA_MAP::iterator		SNPC_ITEM_EXCHANGE_DATA_MAP_ITER;
typedef std::vector<SNPC_ITEM_EXCHANGE_DATA>		SNPC_ITEM_EXCHANGE_DATA_VEC;

struct SNPC_ITEM_EXCHANGE
{
	std::string strTitle;			//window title
	SNPC_ITEM_EXCHANGE_DATA_MAP		mapItem;

	SNPC_ITEM_EXCHANGE()
		: strTitle("Shop")
	{

	}

	SNPC_ITEM_EXCHANGE ( const SNPC_ITEM_EXCHANGE &value )
	{
		operator=(value);
	}

	SNPC_ITEM_EXCHANGE& operator = ( const SNPC_ITEM_EXCHANGE& rvalue )
	{
		strTitle	= rvalue.strTitle;
		mapItem		= rvalue.mapItem;

		return *this;
	}

	BOOL LoadData( std::string strItemExchange );
	SNPC_ITEM_EXCHANGE_DATA*	findData( WORD wExchangeID );
};

#endif // GLCROWDATA_ITEM_EXCHANGE_H_