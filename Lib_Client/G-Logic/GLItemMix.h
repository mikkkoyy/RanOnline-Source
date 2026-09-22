#ifndef GLITEMMIX_H_
#define GLITEMMIX_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "../../Lib_Engine/Common/ByteStream.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "./GLDefine.h"

/*product item, Juver, 2017/10/15 */
#include "GLProductDefine.h"

enum EM_ITEM_MIX
{
	ITEMMIX_ITEMNUM = 5,
};

struct ITEMMIX_DATA
{
	SNATIVEID	sNID;
	BYTE		nNum;

	ITEMMIX_DATA()
		: sNID ( false )
		, nNum ( 0 ) 
	{
	}

	void RESET() { sNID = false; nNum = 0; }
};


struct ITEM_MIX
{
	enum { VERSION = 0x0200 };

	DWORD			dwKey;

	ITEMMIX_DATA	sMeterialItem[ITEMMIX_ITEMNUM];
	ITEMMIX_DATA	sResultItem;

	DWORD		dwRate;
	DWORD		dwPrice;

	/*product item, Juver, 2017/10/15 */
	float		fTime; //time req
	DWORD		dwData1; //point req
	DWORD		dwData2; //point reward
	BYTE		nData1; //
	BYTE		nData2; //
	WORD		wLevelReq; //level req
	WORD		wCategory; //category
	WORD		wData1; //
	WORD		wData2; //
	WORD		wData3; //
	
	BYTE	cDAMAGE;
	BYTE	cDEFENSE;
	BYTE	cRESIST_FIRE;
	BYTE	cRESIST_ICE;
	BYTE	cRESIST_ELEC;
	BYTE	cRESIST_POISON;	
	BYTE	cRESIST_SPIRIT;
	BOOL	bGenerateRandomValue;

	ITEM_MIX()
		: dwKey(UINT_MAX)
		, dwRate(0)
		, dwPrice(0)

		/*product item, Juver, 2017/10/15 */
		, fTime(1.0f)
		, dwData1(0)
		, dwData2(0)
		, nData1(0)
		, nData2(0)
		, wLevelReq(0)
		, wCategory(PRODUCT_TYPE_ETC)
		, wData1(0)
		, wData2(0)
		, wData3(0)

		, cDAMAGE(0)
		, cDEFENSE(0)
		, cRESIST_FIRE(0)
		, cRESIST_ICE(0)
		, cRESIST_ELEC(0)
		, cRESIST_POISON(0)
		, cRESIST_SPIRIT(0)
		, bGenerateRandomValue(FALSE)
	{
	}

	/*product item, Juver, 2017/10/15 */
	bool operator < ( const ITEM_MIX& sData )
	{			
		if ( sResultItem.sNID.wMainID < sData.sResultItem.sNID.wMainID ) 
			return true;

		else if ( sResultItem.sNID.wMainID == sData.sResultItem.sNID.wMainID && 
			sResultItem.sNID.wSubID < sData.sResultItem.sNID.wSubID ) 
			return true;

		return  false;
	}

	BOOL LOAD ( basestream &SFile );
	BOOL SAVE ( CSerialFile &SFile );
};


#endif // GLITEMMIX_H_