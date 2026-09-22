/*!
 * \file GLActivity.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */

#ifndef GLACTIVITY_H_
#define GLACTIVITY_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "GLActivityData.h"

#include "../../Lib_Engine/Common/ByteStream.h"
#include "../../Lib_Engine/Common/SerialFile.h"

class GLActivity
{
public:
	enum
	{
		VERSION				= 0x0001,	
		VERSION_ENCODE		= 0x0001,
		ACTIVITY_MAXID		= 1024,
	};

public:
	GLActivity();
	virtual ~GLActivity();

public:
	static const char*	_FILEHEAD;
	char				m_szFileName[MAX_PATH];

protected:
	std::string			m_strPATH;

public:
	SACTIVITY_FILE_DATA_MAP		m_mapActivity;

public:
	void SetPath ( std::string strPATH )	{ m_strPATH = strPATH; }
	std::string GetPath ()					{ return m_strPATH.c_str(); }
	void	CleanUp();

	HRESULT LoadFile ( const char* szFile, BOOL bServer );
	HRESULT SaveFile ( const char* szFile );

public:
	SACTIVITY_FILE_DATA*	GetActivity( DWORD dwID );

public:
	static GLActivity& GetInstance ();
};

#endif // GLACTIVITY_H_