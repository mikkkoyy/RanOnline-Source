#pragma once
#include <string>
#include "../Common/CList.h"
#include "DxSkinCharData.h"

class DxSkinDataDummy
{
	typedef std::vector<SANIMCONINFO*>	VECANIMCONINFO;

public:
	std::string				m_strFileName;

	float					m_fScale;
	std::string				m_strPIECE[PIECE_SIZE];
	std::string				m_strSkeleton;

	VECANIMCONINFO			m_vecANIMINFO;
	D3DXVECTOR3				m_vMax, m_vMin;

protected:
	BOOL LOAD_0100 ( basestream &SFile ); //0100
	BOOL LOAD_0101 ( basestream &SFile ); //0101,0102,0103
	BOOL LOAD_0104 ( basestream &SFile ); //0104,0106,0107,0108
	BOOL LOAD_0200 ( basestream &SFile ); 

public:
	BOOL LoadFile ( const char* szFile );
	void CleanUp ();

public:
	SANIMCONINFO* FindAniInfo ( const char* szName );

public:
	DxSkinDataDummy(void);
	~DxSkinDataDummy(void);
};
