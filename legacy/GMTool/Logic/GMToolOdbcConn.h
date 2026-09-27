#ifndef _GMTOOL_ODBC_CONNECTION_H_
#define _GMTOOL_ODBC_CONNECTION_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma comment (lib,"odbc32.lib")
#include <odbcinst.h>
#include <sqlext.h> 

#include <Windows.h>
#include <strstream>
#include <stdio.h>

#include "s_NetGlobal.h"
#include "s_DbBase.h"
#include "s_CMemPooler.h"

#include "ByteStream.h"

struct ODBC_STMT
{	
	SQLHSTMT hStmt;
	SQLHDBC hOdbc;

	ODBC_STMT()
	{
		hStmt = NULL;
		hOdbc = NULL;
	};
};

class CGMToolOdbcConn
{
public:
	CGMToolOdbcConn(void);
	CGMToolOdbcConn(
		const TCHAR* szServer,
		const TCHAR* szDatabase,
		const TCHAR* szUsrID,
		const TCHAR* szUsrPass,
		int nServerPort );
	~CGMToolOdbcConn(void);

protected:
	CRITICAL_SECTION	m_CriticalSection;
	void LockOn();
	void LockOff();

	TCHAR m_szServer[DB_SVR_NAME_LENGTH+1];
	TCHAR m_szDatabase[DB_SVR_NAME_LENGTH+1];
	TCHAR m_szUsrID[USR_ID_LENGTH+1];	
	TCHAR m_szUsrPass[USR_PASS_LENGTH+1];	
	int m_nServerPort;

	MEM_POOLER::TCList<ODBC_STMT> m_Use;
	MEM_POOLER::TCList<ODBC_STMT> m_UnUse;

	ODBC_STMT*	CreateNewConnection();

public:	
	void SetDB( const TCHAR* szServer, const TCHAR* szDatabase, const TCHAR* szUsrID, const TCHAR* szUsrPass, int nPort );

	int	OpenDB();

	ODBC_STMT* GetConnection();	
    void FreeConnection( ODBC_STMT* hConn );

	int	ExecuteSQL( CString strSQL );
	int	ExecuteSQL( std::strstream& strSQL );
	int ExecuteSp( std::strstream& strSP );
	int ExecuteSpInt( std::strstream& strSP );

	int	ExecuteSQL( const TCHAR * szSQL );
	int ExecuteSp( const TCHAR * szSP );
	int ExecuteSpInt( const TCHAR * szSP );

	int ReadImage( std::strstream& strTemp, int nChaNum, CByteStream &ByteStream );
	int	ReadImage( const TCHAR* objName, int nChaNum, CByteStream &ByteStream );
    int WriteImage( std::strstream& strTemp, int nChaNum, BYTE* pData, int nSize );
	int WriteImage( const TCHAR* strTemp, int nChaNum, BYTE* pData, int nSize );

	int	ReadImagePet( const TCHAR* objName, int nChaNum, int nPetNum, CByteStream &ByteStream );
	int	ReadImageVehicle( const TCHAR* objName, int nVehicleNum, int nCharNum, CByteStream &ByteStream );

	CString	GetErrorString( SQLHSTMT hStmt );
};

#endif // _GMTOOL_ODBC_CONNECTION_H_