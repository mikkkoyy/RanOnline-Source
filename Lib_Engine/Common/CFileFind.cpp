#include "stdafx.h"
#include "CFileFind.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFileFindTree::CFileFindTree (void)
{
}

CFileFindTree::~CFileFindTree (void)
{
}

void CFileFindTree::Add ( std::string &strFile, std::string &strPath )
{
	m_mapFile[strFile] = strPath;
}

std::string* CFileFindTree::FindPathName ( std::string &str )
{
	FILEMAP_ITER iter = m_mapFile.find ( str );
	if ( iter != m_mapFile.end() )	return &(iter->second);

	return NULL;
}

void CFileFindTree::PathRecurse ( std::string &strDir )
{
	CFileFind finder;

	//	build a string with wildcards
	std::string strWildcard ( strDir );
	strWildcard += _T("\\*.*");

	//	start working for files
	BOOL bWorking = finder.FindFile ( strWildcard.c_str() );

	while ( bWorking )
	{
		bWorking = finder.FindNextFile ();

		//	skip . and .. files; otherwise, we'd
		//	recur infinitely!
		if ( finder.IsDots() ) continue;

		//	if it's a directory, recursively search it
		if ( finder.IsDirectory() )
		{
			std::string strPath = finder.GetFilePath();
			PathRecurse ( strPath );
		}
		else
		{
			std::string strName = finder.GetFileName().GetString();
			std::transform ( strName.begin(), strName.end(), strName.begin(), std::tolower );
			
			std::string strPath = finder.GetFilePath().GetString();

			m_mapFile.insert ( std::make_pair(strName,strPath) );
		}
	}

	finder.Close();
}

void CFileFindTree::CreateTree ( std::string &strPath )
{
	m_strPath = strPath;
	
	CleanUp ();

	//	Note : 해당 폴더를 모두 탐색하며 파일을 검색하여 트리에 삽입.
	//
	PathRecurse ( m_strPath );
}

void CFileFindTree::CleanUp ()
{
	m_mapFile.clear ();
}

std::string* CFileFindTree::FindPathNameNoExtension ( std::string &str )
{
	//remove any extension
	std::string strReplace = str.c_str();
	std::size_t nfound = strReplace.rfind( '.' );
	if ( nfound != std::string::npos )
	{
		strReplace.erase( nfound );

		for ( FILEMAP_ITER pos = m_mapFile.begin(); 
			pos != m_mapFile.end(); pos++ )
		{
			std::string strFileCheck = pos->first;					
			nfound = strFileCheck.rfind( strReplace.c_str() );

			if ( nfound != std::string::npos )
			{
				return &(pos->second);
			}
		}
	}

	return NULL;
}

void CFileFindTree::RebuildTree ()
{
	std::string strPath = m_strPath;
	CreateTree( strPath );

}