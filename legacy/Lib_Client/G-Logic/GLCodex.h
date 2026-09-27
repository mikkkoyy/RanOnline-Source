/*!
 * \file GLCodex.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */

#ifndef GLCODEX_H_
#define GLCODEX_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "GLCodexData.h"

#include "../../Lib_Engine/Common/ByteStream.h"
#include "../../Lib_Engine/Common/SerialFile.h"

class GLCodex
{
public:
	enum
	{
		VERSION				= 0x0001,	
		VERSION_ENCODE		= 0x0001,
		CODEX_MAXID		= 1024,
	};

public:
	GLCodex();
	virtual ~GLCodex();

public:
	static const char*	_FILEHEAD;
	char				m_szFileName[MAX_PATH];

protected:
	std::string			m_strPATH;

public:
	SCODEX_FILE_DATA_MAP		m_mapCodex;

public:
	void SetPath ( std::string strPATH )	{ m_strPATH = strPATH; }
	std::string GetPath ()					{ return m_strPATH.c_str(); }
	void	CleanUp();

	HRESULT LoadFile ( const char* szFile, BOOL bServer );
	HRESULT SaveFile ( const char* szFile );
	
	HRESULT	Import ( const char* szFile );
	bool	SaveCsvs( std::fstream &SFile );
	bool	LoadCsv( std::string& strFileName );
public:
	SCODEX_FILE_DATA*	GetCodex( DWORD dwID );

public:
	static GLCodex& GetInstance ();
};

#endif // GLCODEX_H_