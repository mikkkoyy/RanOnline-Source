/*!
 * \file CListXML.h
 *
 * \author Juver
 * \date May 2017
 *
 * 
 */
#pragma once

#include <string>
#include <list>

struct SXMLFILE
{
	std::string		strFileName;
	BOOL			bEncrypted;

	SXMLFILE () : 
		bEncrypted(FALSE)
	{
	}
};

class CListXML
{
public:
	typedef std::list<SXMLFILE>		LISTXMLFILE;
	typedef LISTXMLFILE::iterator	LISTXMLFILE_ITER;

protected:
	LISTXMLFILE		m_ListXML;

public:
	BOOL	LOAD ( TCHAR* szPath );
	LISTXMLFILE& GetXmlList ()	{ return m_ListXML; }

protected:
	CListXML(void);

public:
	~CListXML(void);

public:
	static CListXML& GetInstance ();
};