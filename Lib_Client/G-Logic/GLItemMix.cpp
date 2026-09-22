#include "stdafx.h"
#include "GLItemMix.h"
#include "StringUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL ITEM_MIX::LOAD ( basestream &SFile )
{
	DWORD dwVer(0);

	SFile >> dwVer;

	if ( dwVer == VERSION )
	{
		SFile >> dwKey;

		for ( int i=0; i<ITEMMIX_ITEMNUM; ++i )
		{
			SFile >> sMeterialItem[i].sNID.wMainID;
			SFile >> sMeterialItem[i].sNID.wSubID;		
			SFile >> sMeterialItem[i].nNum;
		}

		SFile >> sResultItem.sNID.wMainID;
		SFile >> sResultItem.sNID.wSubID;		
		SFile >> sResultItem.nNum;

		SFile >> dwRate;
		SFile >> dwPrice;

		/*product item, Juver, 2017/10/15 */
		SFile >> fTime;
		SFile >> dwData1;
		SFile >> dwData2;
		SFile >> nData1;
		SFile >> nData2;
		SFile >> wLevelReq;
		SFile >> wCategory;
		SFile >> wData1;
		SFile >> wData2;
		SFile >> wData3;

		SFile >> cDAMAGE;
		SFile >> cDEFENSE;
		SFile >> cRESIST_FIRE;
		SFile >> cRESIST_ICE;
		SFile >> cRESIST_ELEC;
		SFile >> cRESIST_POISON;
		SFile >> cRESIST_SPIRIT;
		SFile >> bGenerateRandomValue;
	}
	else if ( dwVer == 0x0102 )
	{
		SFile >> dwKey;

		for ( int i=0; i<ITEMMIX_ITEMNUM; ++i )
		{
			SFile >> sMeterialItem[i].sNID.wMainID;
			SFile >> sMeterialItem[i].sNID.wSubID;		
			SFile >> sMeterialItem[i].nNum;
		}

		SFile >> sResultItem.sNID.wMainID;
		SFile >> sResultItem.sNID.wSubID;		
		SFile >> sResultItem.nNum;

		SFile >> dwRate;
		SFile >> dwPrice;

		/*product item, Juver, 2017/10/15 */
		SFile >> fTime;
		SFile >> dwData1;
		SFile >> dwData2;
		SFile >> nData1;
		SFile >> nData2;
		SFile >> wLevelReq;
		SFile >> wCategory;
		SFile >> wData1;
		SFile >> wData2;
		SFile >> wData3;
	}
	else if ( dwVer == 0x0101 )
	{
		SFile >> dwKey;

		for ( int i=0; i<ITEMMIX_ITEMNUM; ++i )
		{
			SFile >> sMeterialItem[i].sNID.wMainID;
			SFile >> sMeterialItem[i].sNID.wSubID;		
			SFile >> sMeterialItem[i].nNum;
		}

		SFile >> sResultItem.sNID.wMainID;
		SFile >> sResultItem.sNID.wSubID;		
		SFile >> sResultItem.nNum;

		SFile >> dwRate;
		SFile >> dwPrice;

		/*product item, Juver, 2017/10/15 */
		SFile >> fTime;
		SFile >> dwData1;
		SFile >> dwData2;
		SFile >> nData1;
		SFile >> nData2;
	}
	else if ( dwVer == 0x0100 )
	{
		SFile >> dwKey;

		for ( int i=0; i<ITEMMIX_ITEMNUM; ++i )
		{
			SFile >> sMeterialItem[i].sNID.wMainID;
			SFile >> sMeterialItem[i].sNID.wSubID;		
			SFile >> sMeterialItem[i].nNum;
		}

		SFile >> sResultItem.sNID.wMainID;
		SFile >> sResultItem.sNID.wSubID;		
		SFile >> sResultItem.nNum;

		SFile >> dwRate;
		SFile >> dwPrice;
	}
	else
	{
		CDebugSet::ErrorVersion( "ITEM_MIX::LOAD", dwVer );
	}

	return TRUE;
}

BOOL ITEM_MIX::SAVE ( CSerialFile &SFile )
{
	SFile << (DWORD)VERSION;
	
	SFile << dwKey;

	for ( int i=0; i<ITEMMIX_ITEMNUM; ++i )
	{
		SFile << sMeterialItem[i].sNID.wMainID;
		SFile << sMeterialItem[i].sNID.wSubID;		
		SFile << sMeterialItem[i].nNum;
	}

	SFile << sResultItem.sNID.wMainID;                    
	SFile << sResultItem.sNID.wSubID;		
	SFile << sResultItem.nNum;

	SFile << dwRate;
	SFile << dwPrice;

	/*product item, Juver, 2017/10/15 */
	SFile << fTime;
	SFile << dwData1;
	SFile << dwData2;
	SFile << nData1;
	SFile << nData2;
	SFile << wLevelReq;
	SFile << wCategory;
	SFile << wData1;
	SFile << wData2;
	SFile << wData3;

	SFile << cDAMAGE;
	SFile << cDEFENSE;
	SFile << cRESIST_FIRE;
	SFile << cRESIST_ICE;
	SFile << cRESIST_ELEC;
	SFile << cRESIST_POISON;
	SFile << cRESIST_SPIRIT;
	SFile << bGenerateRandomValue;

	return TRUE;
}
