#include "stdafx.h"
#include "GMToolOdbcConn.h"
#include "GMToolOdbcHenv.h"
#include "GMToolLogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CGMToolOdbcConn::CGMToolOdbcConn(void) 
	: m_nServerPort(0)
{	
	InitializeCriticalSection( &m_CriticalSection );
	memset( m_szServer, 0, (DB_SVR_NAME_LENGTH+1) * sizeof(TCHAR) );
	memset( m_szDatabase, 0, (DB_SVR_NAME_LENGTH+1) * sizeof(TCHAR) );
	memset( m_szUsrID,    0, (USR_ID_LENGTH+1) * sizeof(TCHAR) );
	memset( m_szUsrPass,  0, (USR_PASS_LENGTH+1) * sizeof(TCHAR) );
}

CGMToolOdbcConn::CGMToolOdbcConn(
	const TCHAR* szServer,
	const TCHAR* szDatabase,
	const TCHAR* szUsrID,
	const TCHAR* szUsrPass,
	int nServerPort ) 
	: m_nServerPort(0)
{
	InitializeCriticalSection( &m_CriticalSection );
	memset( m_szServer, 0, (DB_SVR_NAME_LENGTH+1) * sizeof(TCHAR) );
	memset( m_szDatabase, 0, (DB_SVR_NAME_LENGTH+1) * sizeof(TCHAR) );
	memset( m_szUsrID,    0, (USR_ID_LENGTH+1) * sizeof(TCHAR) );
	memset( m_szUsrPass,  0, (USR_PASS_LENGTH+1) * sizeof(TCHAR) );

	SetDB( szServer, szDatabase, szUsrID, szUsrPass, nServerPort );
}

CGMToolOdbcConn::~CGMToolOdbcConn(void)
{
	DeleteCriticalSection( &m_CriticalSection );
}

void CGMToolOdbcConn::LockOn()
{
	EnterCriticalSection( &m_CriticalSection );
}

void CGMToolOdbcConn::LockOff() 
{
	LeaveCriticalSection( &m_CriticalSection );
}

void CGMToolOdbcConn::SetDB( const TCHAR* szServer, const TCHAR* szDatabase, const TCHAR* szUsrID, const TCHAR* szUsrPass, int nPort )
{
	::StringCchCopy( m_szServer, DB_SVR_NAME_LENGTH+1, szServer );
	::StringCchCopy( m_szDatabase, DB_SVR_NAME_LENGTH+1, szDatabase );
	::StringCchCopy( m_szUsrID, USR_ID_LENGTH+1, szUsrID );
	::StringCchCopy( m_szUsrPass, USR_PASS_LENGTH+1, szUsrPass );
	m_nServerPort = nPort;
}
	
int CGMToolOdbcConn::OpenDB()
{
	ODBC_STMT* pTemp = CreateNewConnection();
	if (pTemp == NULL)
	{
		return DB_ERROR;
	}else{
		FreeConnection( pTemp );
		return DB_OK;
	}
}

ODBC_STMT* CGMToolOdbcConn::CreateNewConnection()
{
	SQLRETURN	sReturn = 0;
	SQLHSTMT	hStmt = NULL;

	ODBC_STMT* pOdbc = new ODBC_STMT;

	if (pOdbc == NULL) return NULL;

	sReturn = ::SQLAllocHandle( SQL_HANDLE_DBC, CCGMToolOdbcHenv::GetInstance()->GetHandle(), (SQLHANDLE *) &pOdbc->hOdbc );
	if ( (sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO) ){		
		GMTOOL_LOGS::LogInfo( "ERROR:SQLAllocHandle SQL_HANDLE_DBC Failed" );
		SAFE_DELETE( pOdbc );
		CCGMToolOdbcHenv::GetInstance()->CreateHandle();
		return NULL;
	}
	
	sReturn = ::SQLSetConnectAttr( pOdbc->hOdbc, 5, (void*) SQL_LOGIN_TIMEOUT, 0 );
	if ( (sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO) ){
		GMTOOL_LOGS::LogInfo( "ERROR:SQLSetConnectAttr SQL_LOGIN_TIMEOUT Failed" );
		::SQLFreeHandle( SQL_HANDLE_DBC, pOdbc->hOdbc );
		SAFE_DELETE( pOdbc );
		return NULL;
	}

	//sReturn = ::SQLConnect( pOdbc->hOdbc, (SQLCHAR*) m_szOdbcName, SQL_NTS, (SQLCHAR*) m_szUsrID, SQL_NTS, (SQLCHAR*) m_szUsrPass, SQL_NTS );

	char szConnectString[256];
	sprintf( szConnectString, "DRIVER={SQL Server};SERVER=%s,%d;DATABASE=%s;UID=%s;PWD=%s;", m_szServer, m_nServerPort, m_szDatabase, m_szUsrID, m_szUsrPass );

	SQLCHAR szReturnString[1024];
	sReturn = ::SQLDriverConnect ( pOdbc->hOdbc, NULL, (SQLCHAR*)szConnectString, SQL_NTS, szReturnString, 1024, NULL, SQL_DRIVER_NOPROMPT );

	if ( (sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO) ){
		GMTOOL_LOGS::LogInfo( "ERROR:SQLDriverConnect Failed" );
		::SQLFreeHandle( SQL_HANDLE_DBC, pOdbc->hOdbc );
		SAFE_DELETE( pOdbc );
		return NULL;
	} 

	sReturn = ::SQLAllocHandle( SQL_HANDLE_STMT, pOdbc->hOdbc, (SQLHANDLE *) &pOdbc->hStmt );
	if ( (sReturn != SQL_SUCCESS) && (sReturn == SQL_SUCCESS_WITH_INFO) ){
		GMTOOL_LOGS::LogInfo( "ERROR:SQLAllocHandle SQL_HANDLE_STMT Failed" );
		::SQLFreeHandle( SQL_HANDLE_DBC, pOdbc->hOdbc );
		SAFE_DELETE( pOdbc );
		return NULL;
	}

	return pOdbc;
}

ODBC_STMT* CGMToolOdbcConn::GetConnection()
{	
	ODBC_STMT* pTemp = NULL;
	LockOn();
	pTemp = CreateNewConnection();
	LockOff();
	return pTemp;
}

void CGMToolOdbcConn::FreeConnection( ODBC_STMT* hConn )
{	
	if ( hConn == NULL ) return;

	LockOn();
	::SQLCloseCursor( hConn->hStmt );
	::SQLFreeHandle( SQL_HANDLE_STMT, hConn->hStmt );
	::SQLDisconnect( hConn->hOdbc );
	::SQLFreeHandle( SQL_HANDLE_DBC, hConn->hOdbc );
	SAFE_DELETE( hConn );
	LockOff();
}

int	CGMToolOdbcConn::ExecuteSQL( std::strstream& strSQL )
{
	ODBC_STMT*	pConn = NULL;
    SQLRETURN	sReturn = 0;
	
	pConn = GetConnection();
    if (pConn == NULL) return DB_ERROR;
	
	GMTOOL_LOGS::LogSql( "%s", strSQL.str() );
	sReturn = ::SQLExecDirect( pConn->hStmt, (SQLCHAR*) strSQL.str(), SQL_NTS );
	if ( sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO || sReturn == SQL_NO_DATA )
	{		
		FreeConnection(pConn);
		return DB_OK;
	}
	else
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString( pConn->hStmt ) );
		FreeConnection( pConn );
		return DB_ERROR;
	}
}

int	CGMToolOdbcConn::ExecuteSQL( CString strSQL )
{
	ODBC_STMT*	pConn = NULL;
    SQLRETURN	sReturn = 0;
	
	pConn = GetConnection();
    if (!pConn) return DB_ERROR;
	
	GMTOOL_LOGS::LogSql( "%s", strSQL.GetString() );
	sReturn = ::SQLExecDirect( pConn->hStmt, (SQLCHAR*) strSQL.GetString(), SQL_NTS );
	if ( sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO || sReturn == SQL_NO_DATA )
	{
		FreeConnection(pConn);
		return DB_OK;
	}
	else
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString( pConn->hStmt ) );
		FreeConnection( pConn );
		return DB_ERROR;
	}
}

int CGMToolOdbcConn::ExecuteSp( std::strstream& strSP )
{
	ODBC_STMT* pConn = NULL;
	SQLRETURN sReturn = 0;
	
	pConn = GetConnection();
	if (!pConn)	return DB_ERROR;
	
	GMTOOL_LOGS::LogSql( "%s", strSP.str() );
	::SQLPrepare( pConn->hStmt,(SQLCHAR*) strSP.str(), SQL_NTS );
	sReturn = ::SQLExecute( pConn->hStmt );
	if ( (sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO) )
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString( pConn->hStmt ) );
		FreeConnection( pConn );
		return DB_ERROR;
	}
	else
	{
		FreeConnection(pConn);
		return DB_OK;
	}
}

int CGMToolOdbcConn::ExecuteSpInt( std::strstream& strSP )
{
	ODBC_STMT* pConn = GetConnection();
	if (!pConn)	return DB_ERROR;
		
	SQLRETURN  sReturn=0;
	SQLINTEGER nOutput=0;
	SQLINTEGER nLenBuffer=SQL_NTS;

	GMTOOL_LOGS::LogSql( "%s", strSP.str() );
	::SQLPrepare( pConn->hStmt, (SQLCHAR*) strSP.str(), SQL_NTS );
	
	sReturn = ::SQLBindParameter( pConn->hStmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &nOutput, 0, &nLenBuffer );
	
	if ( sReturn == SQL_ERROR || sReturn == SQL_INVALID_HANDLE )
	{		
		GMTOOL_LOGS::LogSqlError( "ERROR:SQLBindParameter" );
		FreeConnection( pConn );
		return DB_ERROR;
	}

	sReturn = ::SQLExecute( pConn->hStmt );
	if ( (sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO) )
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString( pConn->hStmt ) );
		FreeConnection( pConn );
		return DB_ERROR;
	} 
	else
	{
		FreeConnection( pConn );
	}

    return nOutput;
}

int	CGMToolOdbcConn::ExecuteSQL( const TCHAR * szSQL )
{
	ODBC_STMT*	pConn = NULL;
	SQLRETURN	sReturn = 0;

	pConn = GetConnection();
	if (!pConn) return DB_ERROR;

	GMTOOL_LOGS::LogSql( "%s", szSQL );
	sReturn = ::SQLExecDirect( pConn->hStmt, (SQLCHAR*)szSQL, SQL_NTS );
	if ( sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO || sReturn == SQL_NO_DATA )
	{
		FreeConnection(pConn);
		return DB_OK;
	}
	else
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString( pConn->hStmt ) );
		FreeConnection( pConn );
		return DB_ERROR;
	}
}

int CGMToolOdbcConn::ExecuteSp( const TCHAR * szSP )
{
	ODBC_STMT* pConn = NULL;
	SQLRETURN sReturn = 0;

	pConn = GetConnection();
	if (!pConn)	return DB_ERROR;

	GMTOOL_LOGS::LogSql( "%s", szSP );
	::SQLPrepare( pConn->hStmt,(SQLCHAR*)szSP, SQL_NTS );
	sReturn = ::SQLExecute( pConn->hStmt );
	if ( (sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO) )
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString( pConn->hStmt ) );
		FreeConnection( pConn );
		return DB_ERROR;
	}
	else
	{
		FreeConnection(pConn);
		return DB_OK;
	}
}

int CGMToolOdbcConn::ExecuteSpInt( const TCHAR * szSP )
{
	ODBC_STMT* pConn = GetConnection();
	if (!pConn)	return DB_ERROR;

	SQLRETURN  sReturn=0;
	SQLINTEGER nOutput=0;
	SQLINTEGER nLenBuffer=SQL_NTS;

	GMTOOL_LOGS::LogSql( "%s", szSP );
	::SQLPrepare( pConn->hStmt, (SQLCHAR*)szSP, SQL_NTS );

	sReturn = ::SQLBindParameter( pConn->hStmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &nOutput, 0, &nLenBuffer );

	if ( sReturn == SQL_ERROR || sReturn == SQL_INVALID_HANDLE )
	{		
		GMTOOL_LOGS::LogSqlError( "ERROR:SQLBindParameter" );
		FreeConnection( pConn );
		return DB_ERROR;
	}

	sReturn = ::SQLExecute( pConn->hStmt );
	if ( (sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO) )
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString( pConn->hStmt ) );
		FreeConnection( pConn );
		return DB_ERROR;
	} 
	else
	{
		FreeConnection( pConn );
	}
	return nOutput;
}

int	CGMToolOdbcConn::ReadImage( const TCHAR* objName, int nChaNum, CByteStream &ByteStream )
{
	if (objName == NULL) return DB_ERROR;

	SQLRETURN	sReturn = 0;
	ODBC_STMT*	pConn = NULL;
	SQLCHAR		pBinary[DB_IMAGE_BUF_SIZE] = {0};
	SQLINTEGER  lSize=0, lTotalSize=0;	
	
	pConn = GetConnection();
    if (!pConn) return DB_ERROR;	
	
	TCHAR szTemp[128] = {0};
	_snprintf( szTemp, 128, "SELECT %s FROM ChaInfo where (ChaNum=%d)", objName, nChaNum );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	sReturn = ::SQLExecDirect( pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS );
	if ( sReturn != SQL_SUCCESS && sReturn != SQL_SUCCESS_WITH_INFO )
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString( pConn->hStmt ) );
		FreeConnection( pConn );
		return DB_ERROR;
	}
	
	ByteStream.ClearBuffer();

	while ( (sReturn = ::SQLFetch(pConn->hStmt) ) != SQL_NO_DATA )
	{			
		while (1) 
		{
			::memset( pBinary, 0, DB_IMAGE_BUF_SIZE );
			lSize = 0;
			sReturn = ::SQLGetData( pConn->hStmt, 1, SQL_C_BINARY, pBinary, DB_IMAGE_BUF_SIZE, &lSize );
			if (lSize > 0)
			{
				if (lSize > DB_IMAGE_BUF_SIZE) lSize = DB_IMAGE_BUF_SIZE;
				lTotalSize += lSize;
				ByteStream.WriteBuffer( (LPBYTE) pBinary, lSize );
			}

			if (sReturn == SQL_NO_DATA || lSize == 0){
				break;
			}

			Sleep(0);
		}

		if ( lTotalSize < DB_IMAGE_MIN_SIZE ){		
			ByteStream.ClearBuffer ();
		}

		Sleep(0);
	}

	FreeConnection(pConn);
	return DB_OK;
}


int	CGMToolOdbcConn::ReadImage( std::strstream& strTemp, int nChaNum, CByteStream &ByteStream )
{
	SQLRETURN	sReturn = 0;
	ODBC_STMT*	pConn = NULL;
	SQLCHAR		pBinary[DB_IMAGE_BUF_SIZE] = {0};
	SQLINTEGER  lSize=0, lTotalSize=0;
	
	pConn = GetConnection();
    if (!pConn) return DB_ERROR;
	
	GMTOOL_LOGS::LogSql( "%s", strTemp.str() );

	sReturn = ::SQLExecDirect( pConn->hStmt, (SQLCHAR*) strTemp.str(), SQL_NTS);
	if (sReturn != SQL_SUCCESS && sReturn != SQL_SUCCESS_WITH_INFO)
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString( pConn->hStmt ) );
		FreeConnection(pConn);
		return DB_ERROR;
	}
	
	ByteStream.ClearBuffer();

	while ((sReturn = ::SQLFetch(pConn->hStmt)) != SQL_NO_DATA) 
	{			
		while (1) 
		{
			::memset(pBinary, 0, DB_IMAGE_BUF_SIZE);
			lSize = 0;
			sReturn = ::SQLGetData(pConn->hStmt, 1, SQL_C_BINARY, pBinary, DB_IMAGE_BUF_SIZE, &lSize);
			if (lSize > 0)
			{
				if (lSize > DB_IMAGE_BUF_SIZE) lSize = DB_IMAGE_BUF_SIZE;
				lTotalSize += lSize;
				ByteStream.WriteBuffer((LPBYTE) pBinary, lSize);
			}
			if (sReturn == SQL_NO_DATA || lSize == 0){
				break;
			}

			Sleep(0);
		}

		if ( lTotalSize < DB_IMAGE_MIN_SIZE ){		
			ByteStream.ClearBuffer ();
		}

		Sleep(0);
	}

	FreeConnection(pConn);
	return DB_OK;
}

int CGMToolOdbcConn::WriteImage( const TCHAR* strTemp, int nChaNum, BYTE* pData,  int nSize )
{
	SQLRETURN	sReturn = 0;
	ODBC_STMT*	pConn = NULL;
	SQLINTEGER  lSize=0, lTotalSize=0;

	pConn = GetConnection();
	if (!pConn) return DB_ERROR;

	GMTOOL_LOGS::LogSql( "%s", strTemp );

	sReturn = ::SQLPrepare(pConn->hStmt, (SQLCHAR*)strTemp, SQL_NTS);
	if (sReturn != SQL_SUCCESS && sReturn != SQL_SUCCESS_WITH_INFO) 
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString( pConn->hStmt ) );
		FreeConnection(pConn);
		return DB_ERROR;
	}

	SQLINTEGER cbBinaryParam = SQL_LEN_DATA_AT_EXEC(0);

	sReturn = ::SQLBindParameter(pConn->hStmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_LONGVARBINARY, nSize, 0, (SQLPOINTER) 2, 0, &cbBinaryParam);
	if (sReturn != SQL_SUCCESS && sReturn != SQL_SUCCESS_WITH_INFO) 
	{
		FreeConnection(pConn);
		return DB_ERROR;
	}

	sReturn = ::SQLExecute(pConn->hStmt);
	if (sReturn == SQL_ERROR || sReturn == SQL_SUCCESS_WITH_INFO)
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString( pConn->hStmt ) );
		FreeConnection(pConn);
		return DB_ERROR;
	}

	SQLPOINTER pToken;
	while (sReturn == SQL_NEED_DATA) 
	{
		sReturn = ::SQLParamData(pConn->hStmt, &pToken);
		if (sReturn == SQL_NEED_DATA) {				
			::SQLPutData(pConn->hStmt, pData, (SQLINTEGER) nSize);
		}

		Sleep(0);
	}

	FreeConnection(pConn);
	return DB_OK;
}

int CGMToolOdbcConn::WriteImage( std::strstream& strTemp, int nChaNum, BYTE* pData, int nSize )
{
	SQLRETURN	sReturn = 0;
	ODBC_STMT*	pConn = NULL;
	SQLINTEGER  lSize=0, lTotalSize=0;

	pConn = GetConnection();
    if (!pConn) return DB_ERROR;
	
	GMTOOL_LOGS::LogSql( "%s", strTemp.str() );

	sReturn = ::SQLPrepare(pConn->hStmt, (SQLCHAR*) strTemp.str(), SQL_NTS);
	if (sReturn != SQL_SUCCESS && sReturn != SQL_SUCCESS_WITH_INFO) 
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString( pConn->hStmt ) );
		FreeConnection(pConn);
		return DB_ERROR;
	}

	SQLINTEGER cbBinaryParam = SQL_LEN_DATA_AT_EXEC(0);

	sReturn = ::SQLBindParameter(pConn->hStmt, 1, SQL_PARAM_INPUT,SQL_C_BINARY, SQL_LONGVARBINARY, nSize, 0, (SQLPOINTER) 2, 0, &cbBinaryParam);
	if (sReturn != SQL_SUCCESS && sReturn != SQL_SUCCESS_WITH_INFO) 
	{
		FreeConnection(pConn);
		return DB_ERROR;
	}

	sReturn = ::SQLExecute(pConn->hStmt);
	if (sReturn == SQL_ERROR || sReturn == SQL_SUCCESS_WITH_INFO)
    {
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString( pConn->hStmt ) );
        FreeConnection(pConn);
        return DB_ERROR;
	}
	
	SQLPOINTER pToken;
	while (sReturn == SQL_NEED_DATA) 
	{
		sReturn = ::SQLParamData(pConn->hStmt, &pToken);
		if (sReturn == SQL_NEED_DATA) {				
			::SQLPutData(pConn->hStmt, pData, (SQLINTEGER) nSize);
		}

		Sleep(0);
	}
 
	FreeConnection(pConn);
	return DB_OK;
}

CString	CGMToolOdbcConn::GetErrorString(SQLHSTMT hStmt)
{
	SQLRETURN sReturn = 0;
	SQLCHAR SqlState[6] = {0};
	SQLCHAR Msg[ODBC_ERROR_MESSAGE_LENGTH+1] = {0};
	SQLSMALLINT MsgLen = 0;
	SQLINTEGER nError = 0;
	int nDiag = 0;

	CString strTemp;

	::SQLGetDiagField(SQL_HANDLE_STMT, hStmt, 0, SQL_DIAG_NUMBER, &nDiag, 0, &MsgLen);

	for (nDiag=1;;nDiag++)
	{
		sReturn = ::SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, nDiag, SqlState, &nError, Msg, ODBC_ERROR_MESSAGE_LENGTH,  &MsgLen);
		if (sReturn == SQL_NO_DATA)
			break;
		else
			strTemp.Format(_T("%s, NativeError:%d, %s"),(LPCTSTR) SqlState,  nError, (LPCTSTR) Msg);
	}
	return strTemp;
}

int	CGMToolOdbcConn::ReadImagePet( const TCHAR* objName, int nChaNum, int nPetNum, CByteStream &ByteStream )
{
	if (objName == NULL) return DB_ERROR;

	SQLRETURN	sReturn = 0;
	ODBC_STMT*	pConn = NULL;
	SQLCHAR		pBinary[DB_IMAGE_BUF_SIZE] = {0};
	SQLINTEGER  lSize=0, lTotalSize=0;	

	pConn = GetConnection();
	if (!pConn) return DB_ERROR;	

	TCHAR szTemp[128] = {0};
	_snprintf( szTemp, 128, "SELECT %s FROM PetInfo where (PetNum=%d And PetChaNum=%d)", objName, nPetNum, nChaNum );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	sReturn = ::SQLExecDirect( pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS );

	if ( sReturn != SQL_SUCCESS && sReturn != SQL_SUCCESS_WITH_INFO )
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString( pConn->hStmt ) );
		FreeConnection( pConn );
		return DB_ERROR;
	}

	ByteStream.ClearBuffer();

	while ( (sReturn = ::SQLFetch(pConn->hStmt) ) != SQL_NO_DATA )
	{			
		while (1) 
		{
			::memset( pBinary, 0, DB_IMAGE_BUF_SIZE );
			lSize = 0;
			sReturn = ::SQLGetData( pConn->hStmt, 1, SQL_C_BINARY, pBinary, DB_IMAGE_BUF_SIZE, &lSize );
			if (lSize > 0)
			{
				if (lSize > DB_IMAGE_BUF_SIZE) lSize = DB_IMAGE_BUF_SIZE;
				lTotalSize += lSize;
				ByteStream.WriteBuffer( (LPBYTE) pBinary, lSize );
			}
			if (sReturn == SQL_NO_DATA || lSize == 0){
				break;
			}

			Sleep(0);
		}

		if ( lTotalSize < DB_IMAGE_MIN_SIZE ){		
			ByteStream.ClearBuffer ();
		}

		Sleep(0);
	}

	FreeConnection(pConn);
	return DB_OK;
}

int	CGMToolOdbcConn::ReadImageVehicle( const TCHAR* objName, int nVehicleNum, int nCharNum, CByteStream &ByteStream )
{
	if (objName == NULL) return DB_ERROR;

	SQLRETURN	sReturn = 0;
	ODBC_STMT*	pConn = NULL; 
	SQLCHAR		pBinary[DB_IMAGE_BUF_SIZE] = {0};
	SQLINTEGER  lSize=0, lTotalSize=0;	

	pConn = GetConnection();
	if (!pConn) return DB_ERROR;	

	TCHAR szTemp[128] = {0};
	_snprintf( szTemp, 128, "SELECT %s FROM VehicleInfo where (VehicleNum=%d) And (VehicleChaNum=%d)", objName, nVehicleNum, nCharNum );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	sReturn = ::SQLExecDirect( pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS );

	if ( sReturn != SQL_SUCCESS && sReturn != SQL_SUCCESS_WITH_INFO )
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString( pConn->hStmt ) );
		FreeConnection( pConn );
		return DB_ERROR;
	}

	ByteStream.ClearBuffer();

	while ( (sReturn = ::SQLFetch(pConn->hStmt) ) != SQL_NO_DATA )
	{			
		while (1) 
		{
			::memset( pBinary, 0, DB_IMAGE_BUF_SIZE );
			lSize = 0;
			sReturn = ::SQLGetData( pConn->hStmt, 1, SQL_C_BINARY, pBinary, DB_IMAGE_BUF_SIZE, &lSize );

			if (lSize > 0)
			{
				if (lSize > DB_IMAGE_BUF_SIZE) lSize = DB_IMAGE_BUF_SIZE;
				lTotalSize += lSize;
				ByteStream.WriteBuffer( (LPBYTE) pBinary, lSize );
			}
			if (sReturn == SQL_NO_DATA || lSize == 0){
				break;
			}

			Sleep(0);
		}

		if ( lTotalSize < DB_IMAGE_MIN_SIZE ){		
			ByteStream.ClearBuffer ();
		}

		Sleep(0);
	}

	FreeConnection(pConn);

	return DB_OK;
}