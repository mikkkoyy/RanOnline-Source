//
//	[class CFileFindTree], (2002.12.07), JDH.
//
//	
//
#pragma once


#include <map>
#include <string>

class CFileFindTree
{
public:
	typedef std::map<std::string,std::string>				FILEMAP;
	typedef FILEMAP::iterator								FILEMAP_ITER;

protected:
	std::string		m_strPath;
	FILEMAP			m_mapFile;

public:
	void Add ( std::string &strFile, std::string &strPath );

	std::string* FindPathName ( std::string &str );
	std::string* FindPathNameNoExtension ( std::string &str );

	FILEMAP&	GetFileMap ()		{ return m_mapFile; }
	std::string	GetPathName ()		{ return m_strPath.c_str(); }
	BOOL	IsEmpty()	{ return m_mapFile.empty(); }

protected:
	void PathRecurse ( std::string &strDir );
	void CleanUp ();

public:
	void CreateTree ( std::string &strPath );
	void RebuildTree();

public:
	CFileFindTree (void);
	~CFileFindTree (void);
};
