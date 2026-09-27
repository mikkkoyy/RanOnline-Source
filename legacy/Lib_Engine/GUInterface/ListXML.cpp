#include "stdafx.h"
#include "./ListXML.h"
#include "../Common/IniLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CListXML::CListXML(void)
{
}

CListXML::~CListXML(void)
{
}

CListXML& CListXML::GetInstance ()
{
	static CListXML Instance;
	return Instance;
}

BOOL CListXML::LOAD ( TCHAR* szPath )
{
	if ( !szPath ){
		return FALSE;
	}
	std::string strFilePath(szPath);
	strFilePath += "XMLList.ini";

	CIniLoader cFILE;
	if( !cFILE.open( strFilePath, false) )
	{
		CDebugSet::ErrorFile( "CListXML::LOAD", strFilePath.c_str());
		return FALSE;
	}

	std::string strSep( ",[]\t" );
	cFILE.reg_sep( strSep );

	m_ListXML.clear();

	//not encrypted
	DWORD dwNUM = cFILE.GetKeySize( "XML_LIST", "NOR" );
	for ( DWORD i=0; i<dwNUM; ++i )
	{
		SXMLFILE sXMLFILE;
		sXMLFILE.bEncrypted = FALSE;

		cFILE.getflag( i, "XML_LIST", "NOR", 0, 1, sXMLFILE.strFileName );

		m_ListXML.push_back( sXMLFILE );
	}

	//encrypted
	dwNUM = cFILE.GetKeySize( "XML_LIST", "ENC" );
	for ( DWORD i=0; i<dwNUM; ++i )
	{
		SXMLFILE sXMLFILE;
		sXMLFILE.bEncrypted = TRUE;

		cFILE.getflag( i, "XML_LIST", "ENC", 0, 1, sXMLFILE.strFileName );

		m_ListXML.push_back( sXMLFILE );
	}

	return TRUE;
}
