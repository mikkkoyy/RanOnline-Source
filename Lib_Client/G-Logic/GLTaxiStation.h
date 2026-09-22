#ifndef GLTAXI_STATION_H_
#define GLTAXI_STATION_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "./GLTaxiData.h"

class GLTaxiStation 
{
public:
	enum
	{
		VERSION				= 0x0200,	
		VERSION_ENCODE		= 0x0200,
		VERSION_ENCODE_OLD	= 0x0100,
		TAXI_MAXID			= 1000,
	};

	typedef std::vector<STAXI_STATION>	TAXISTATION_VEC;

public:
	static const char*	_FILEHEAD;
	char				m_szFileName[MAX_PATH];

public:
	virtual ~GLTaxiStation(void);

protected:
	std::string			m_strPATH;
	DWORD				m_dwBasicCharge;

public:
	void SetPath ( std::string strPATH )	{ m_strPATH = strPATH; }
	std::string GetPath ()					{ return m_strPATH.c_str(); }
	DWORD GetBasicCharge() { return m_dwBasicCharge; }
	void SetBasicCharge( DWORD dwValue )	{ m_dwBasicCharge = dwValue; }

public:
	TAXISTATION_VEC			m_vecTaxiData;

protected:
	std::vector<STAXI_MAP>	m_vecTaxiMap;

protected:
	GLTaxiStation(void);
	void insert( STAXI_STATION &sSTATION );

public:
	virtual bool IMPORT( std::string strFILE );

	HRESULT LoadFile ( const char* szFile );
	HRESULT SaveFile ( const char* szFile );
	
public:
	DWORD GetTaxiMapNum();
	STAXI_MAP* GetTaxiMap ( int nIndex );

	virtual STAXI_STATION* GetStation ( int nMapIndex, int nStationIndex );

public:
	static GLTaxiStation& GetInstance ();
};

#endif // GLTAXI_STATION_H_