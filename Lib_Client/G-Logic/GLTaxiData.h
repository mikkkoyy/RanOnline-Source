#ifndef GLTAXI_DATA_H_
#define GLTAXI_DATA_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "../../Lib_Engine/Common/ByteStream.h"
#include "../../Lib_Engine/Common/SerialFile.h"

struct STAXI_STATION
{
	enum { VERSION = 0x0100 };

	DWORD		dwLINKID;
	bool		bUsed;
	DWORD		dwMAPID;
	WORD		wPosX;
	WORD		wPosY;
	DWORD		dwNPCID;
	DWORD		dwMapCharge;
	std::string	strMAP;
	std::string	strSTATION;

	STAXI_STATION ()
		: dwLINKID(0)
		, bUsed( false )
		, dwMAPID(0)
		, wPosX(0)
		, wPosY(0)
		, dwNPCID(0)
		, dwMapCharge(0)
	{
	}

	STAXI_STATION& operator= ( const STAXI_STATION& value )
	{
		dwLINKID = value.dwLINKID;
		bUsed = value.bUsed;
		dwMAPID = value.dwMAPID;
		wPosX = value.wPosX;
		wPosY = value.wPosY;
		dwNPCID = value.dwNPCID;
		dwMapCharge = value.dwMapCharge;
		strMAP = value.strMAP;
		strSTATION = value.strSTATION;

		return *this;
	}

	BOOL LOAD ( basestream &SFile );
	BOOL SAVE ( CSerialFile &SFile );
};

struct STAXI_MAP
{
	DWORD						dwMAPID;
	std::string					strMAP;
	std::vector<STAXI_STATION>	vecTaxiStation;

	STAXI_MAP () :
	dwMAPID(0)
	{
	}

	STAXI_MAP& operator= ( const STAXI_MAP& value )
	{
		dwMAPID = value.dwMAPID;
		strMAP = value.strMAP;
		vecTaxiStation = value.vecTaxiStation;
		return *this;
	}

	DWORD GetStationNum ()
	{
		return (DWORD) vecTaxiStation.size();
	}

	STAXI_STATION* GetStation ( DWORD dwIndex )
	{

		if ( vecTaxiStation.size() <= dwIndex )
			return NULL;

		return &vecTaxiStation[dwIndex];
	}
};


class TAXISTATION_CMP
{
public:
	bool operator() ( const STAXI_STATION &lvalue, const STAXI_STATION &rvalue )
	{
		if ( lvalue.dwLINKID < rvalue.dwLINKID ) return true;
		else if ( lvalue.dwLINKID == rvalue.dwLINKID ) 
		{	
			if ( lvalue.dwMAPID < rvalue.dwMAPID ) return true;
		}
		else if ( lvalue.dwLINKID == rvalue.dwLINKID && lvalue.dwMAPID == rvalue.dwMAPID ) 
		{	
			if ( lvalue.dwNPCID < rvalue.dwNPCID ) return true;
		}

		return false;
	}
};

#endif // GLTAXI_DATA_H_