/*dmk14 ingame web*/
/*!
 * \file GLItemShopData.h
 *
 * \author dmk14
 * \date Dec 2020
 *
 * 
 */

#ifndef GLITEMSHOPDATA_H_
#define GLITEMSHOPDATA_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GLDefine.h"
#include "GLItem.h"

struct ITEMMALLDATA
{
	unsigned		dwIndex;
	unsigned		dwBuyCount;
	unsigned		dwProductNum;
	unsigned		dwItemPrice;
	unsigned		dwItemStocks;
	unsigned	 	wItemCtg;
	//item section - cndev
	unsigned		wItemSec;
	SNATIVEID		sNativeID;

	ITEMMALLDATA()
		: dwIndex(0)
		, dwBuyCount(0)
		, dwProductNum(0)
		, dwItemPrice(0)
		, dwItemStocks(0)
		, wItemCtg(0)
		//item section - cndev
		, wItemSec(0)
		, sNativeID( NATIVEID_NULL() )
	{
	}

	ITEMMALLDATA( const ITEMMALLDATA &value)
	{
		operator=(value);
	}

	ITEMMALLDATA& operator=( const ITEMMALLDATA &value )
	{
		dwIndex = value.dwIndex;
		dwBuyCount = value.dwBuyCount;
		dwProductNum = value.dwProductNum;
		dwItemPrice = value.dwItemPrice;
		dwItemStocks = value.dwItemStocks;
		wItemCtg = value.wItemCtg;
		//item section - cndev
		wItemSec = value.wItemSec;
		sNativeID = value.sNativeID;

		return *this;
	}

	bool operator < ( const ITEMMALLDATA& sItemMallRank )
	{		
		if( dwBuyCount >= sItemMallRank.dwBuyCount ) return true;
		return  false;
	}

	bool operator == ( const ITEMMALLDATA& sItemMallRank ) 
	{
		if( dwBuyCount == sItemMallRank.dwBuyCount ) return true;
		return false;
	}
};

typedef std::vector<ITEMMALLDATA>			VECITEMMALL;
typedef VECITEMMALL::iterator				VECITEMMALL_ITER;

typedef std::map<DWORD,ITEMMALLDATA>		MAPITEMMALL;
typedef MAPITEMMALL::iterator				MAPITEMMALL_ITER;

#endif // GLITEMSHOPDATA_H_