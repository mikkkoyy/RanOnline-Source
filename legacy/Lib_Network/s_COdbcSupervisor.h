#ifndef _CODBCSUPERVISOR_H_
#define _CODBCSUPERVISOR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// To use the installer DLL functions, you must: 
// Use #include < ODBCINST.H>.
// Link with Odbcinst.lib for 16-bit applications.
// Link with Odbccp32.lib for 32-bit applications. 
// #pragma comment (lib,"Odbccp32.lib")
#pragma comment (lib,"odbc32.lib")
#include <odbcinst.h>
#include <sqlext.h> // ODBC library

#include <Windows.h>
#include <strstream>
#include <stdio.h>

#include "s_NetGlobal.h"
#include "s_DbBase.h"
#include "s_CLock.h"
#include "s_CConsoleMessage.h"
#include "s_CMemPooler.h"
#include "s_COdbcEnv.h"

// #include "../Lib_Engine/G-Logic/GLDBMan.h"
#include "../Lib_Client/G-Logic/GLCharData.h"
#include "../Lib_Client/G-Logic/GLContrlMsg.h"

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
/**
 * \ingroup NetServerLib
 * 
 *
 * \par requirements
 * win2k or later\n
 * MFC\n
 *
 * \version 1.0
 * first version
 *
 * \date 2003-07-12
 *
 * \author jgkim
 *
 * \par license
 * 
 * \todo 
 *
 * \bug 
 *
 */

// ---------------------------------------------------------------------------
// PHASE2 : descriptor for one INPUT parameter of a parameterized stored
// procedure call. Provide them in the same order as their ? placeholders.
// ---------------------------------------------------------------------------
struct SP_PARAM
{
	enum SP_PARAM_TYPE { TYPE_STRING, TYPE_INT };

	SP_PARAM_TYPE	eType;
	const TCHAR*	szValue;	// valid when eType == TYPE_STRING
	int			nValue;		// valid when eType == TYPE_INT

	SP_PARAM() : eType( TYPE_STRING ), szValue( NULL ), nValue( 0 ) {}

	void SetString( const TCHAR* sz )	{ eType = TYPE_STRING; szValue = sz; nValue = 0; }
	void SetInt( int n )			{ eType = TYPE_INT;    szValue = NULL; nValue = n; }
};

// Maximum number of bound INPUT parameters accepted by
// COdbcSupervisor::ExecuteSpInt( szSP, pParams, nParamCount ).
#define MAX_SP_INPUT_PARAM	8
class COdbcSupervisor
{
public:
	COdbcSupervisor();
	COdbcSupervisor(
		const TCHAR* szOdbcName,
		const TCHAR* szUsrID,
		const TCHAR* szUsrPass,
		const TCHAR* szDBName,
		int nPoolSize = DB_POOL_SIZE,
		int nDBTimeOut = DB_RESPONSE_TIME );
	~COdbcSupervisor();

protected:
	CRITICAL_SECTION	m_CriticalSection;
	void LockOn();
	void LockOff();

	TCHAR m_szOdbcName[DB_SVR_NAME_LENGTH+1];
	TCHAR m_szUsrID[USR_ID_LENGTH+1];	
	TCHAR m_szUsrPass[USR_PASS_LENGTH+1];
	TCHAR m_szDBName[DB_NAME_LENGTH+1];		
	int	m_nDBTimeOut;
	int	m_nPoolSize;

	MEM_POOLER::TCList<ODBC_STMT> m_Use;
	MEM_POOLER::TCList<ODBC_STMT> m_UnUse;

	ODBC_STMT*	CreateNewConnection();

public:	
	void SetDB(
			const TCHAR* szOdbcName,
			const TCHAR* szUsrID,
			const TCHAR* szUsrPass,
			const TCHAR* szDBName,
			int nPoolSize = DB_POOL_SIZE,
			int nDBTimeOut = DB_RESPONSE_TIME );
	void SetOdbcName( const TCHAR* szName );
	void SetUserID( const TCHAR* szUsrID );
	void SetUserPass( const TCHAR* szUsrPasswd );
	void SetDBName( const TCHAR* szDBName );
	void SetPoolSize( int nSize = DB_POOL_SIZE );
	void SetResponseTime( int nTime = DB_RESPONSE_TIME );
	
	int	OpenDB();
	void CloseDB();

	ODBC_STMT* GetConnection();	
    void FreeConnection( ODBC_STMT* hConn );

	int	ExecuteSQL( CString strSQL );
	int	ExecuteSQL( std::strstream& strSQL );
	int ExecuteSp( std::strstream& strSP );
	int ExecuteSpInt( std::strstream& strSP );

	int	ExecuteSQL( const TCHAR * szSQL );
	int ExecuteSp( const TCHAR * szSP );
	int ExecuteSpInt( const TCHAR * szSP );

	// PHASE2 : parameterized overload. The inputs in pParams are bound as
	// parameters 1..nParamCount following the order of the ? placeholders
	// in szSP, and the trailing ? of szSP is bound as the SQL_PARAM_OUTPUT
	// result (parameter nParamCount+1).
	// NOTE : never pass ?-bearing text to the single argument ExecuteSpInt
	// above, it hard-codes parameter 1 as SQL_PARAM_OUTPUT.
	int ExecuteSpInt( const TCHAR * szSP, const SP_PARAM * pParams, int nParamCount );

	int ReadImage(
			std::strstream& strTemp, 
			int nChaNum, 
			CByteStream &ByteStream );
	int	ReadImage(
			const TCHAR* objName, 
			int nChaNum, 
			CByteStream &ByteStream );
	int	ReadImagePet(
			const TCHAR* objName,
			int nChaNum,
			int nPetNum, 
			CByteStream &ByteStream );
	int	ReadImageVehicle(
		const TCHAR* objName, 
		int nVehicleNum,
		int nCharNum,
		CByteStream &ByteStream );
    int WriteImage(
			std::strstream& strTemp,
			int nChaNum, 
			BYTE* pData, 
			int nSize );
	int WriteImage(
			const TCHAR* strTemp,
			int nChaNum, 
			BYTE* pData, 
			int nSize );
	CString	GetErrorString( SQLHSTMT hStmt );
	void Print( CString strMsg );
	void Print( const TCHAR * szMsg );
};

#endif // _CODBCSUPERVISOR_H_