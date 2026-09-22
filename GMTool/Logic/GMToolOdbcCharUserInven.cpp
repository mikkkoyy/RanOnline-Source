#include "stdAfx.h"
#include "GMToolOdbcBase.h"
#include "GMToolOdbcConn.h"
#include "GMToolLogs.h"


#include "GLCharDefine.h"
#include "GLCharData.h"
#include "GMToolLogs.h"
#include "GMToolGlobal.h"
#include "HLibTimeFunctions.h"
#include "HLibDataConvert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CGMToolOdbcBase::UserInvenValidate( SCHARDATA2* pCharData2 )
{
	if ( !pCharData2 ){
		return DB_ERROR;
	}

	if ( pCharData2->GetUserID() == 0 ){
		return DB_ERROR;
	}

	TCHAR szCheck[128] = {0};
	_snprintf( szCheck, 128, "{call GetInvenCount(%u,?)}", pCharData2->GetUserID() );

	TCHAR szInsert[128] = {0};
	_snprintf( szInsert, 128, "{call MakeUserInven(%d,%u,?)}", 0, pCharData2->GetUserID() );

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	int nReturn = pDatabase->ExecuteSpInt(szCheck);
	if ( nReturn <= 0 ){
		pDatabase->ExecuteSpInt(szInsert);
	}

	return DB_OK;
}

int CGMToolOdbcBase::UserInvenGetData( SCHARDATA2* pCharData2 )
{
	if ( !pCharData2 ){
		return DB_ERROR;
	}

	if ( pCharData2->GetUserID() == 0 ){
		return DB_ERROR;
	}

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "SELECT UserMoney, ChaStorage2, ChaStorage3, ChaStorage4 FROM UserInven WHERE UserNum=%u", pCharData2->GetUserID() );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	SQLRETURN sReturn = 0;
	sReturn = ::SQLExecDirect( pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS );

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}

	LONGLONG   llUserMoney = 0;
	SQLINTEGER cbUserMoney = SQL_NTS;
	TIMESTAMP_STRUCT sStorageDate2; SQLINTEGER cbStorageDate2 = SQL_NTS;
	TIMESTAMP_STRUCT sStorageDate3; SQLINTEGER cbStorageDate3 = SQL_NTS;
	TIMESTAMP_STRUCT sStorageDate4; SQLINTEGER cbStorageDate4 = SQL_NTS;

	while(true)
	{
		sReturn = ::SQLFetch(pConn->hStmt);
		if (sReturn == SQL_ERROR || sReturn == SQL_SUCCESS_WITH_INFO)
		{
			GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
			pDatabase->FreeConnection(pConn);
			return DB_ERROR;
		}

		if (sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO)
		{	
			::SQLGetData(pConn->hStmt, 1, SQL_C_SBIGINT,        &llUserMoney,   0, &cbUserMoney);
			::SQLGetData(pConn->hStmt, 2, SQL_C_TYPE_TIMESTAMP, &sStorageDate2, 0, &cbStorageDate2);
			::SQLGetData(pConn->hStmt, 3, SQL_C_TYPE_TIMESTAMP, &sStorageDate3, 0, &cbStorageDate3);
			::SQLGetData(pConn->hStmt, 4, SQL_C_TYPE_TIMESTAMP, &sStorageDate4, 0, &cbStorageDate4);

			pCharData2->m_lnStorageMoney = llUserMoney;

			if ( !_HLIB::timestamp_correction( sStorageDate2 ) ){
				GMTOOL_LOGS::LogError( "Variable sStorageDate2 value corrected" );
			}

			if ( !_HLIB::timestamp_correction( sStorageDate3 ) ){
				GMTOOL_LOGS::LogError( "Variable sStorageDate3 value corrected" );
			}

			if ( !_HLIB::timestamp_correction( sStorageDate4 ) ){
				GMTOOL_LOGS::LogError( "Variable sStorageDate4 value corrected" );
			}

			pCharData2->m_tSTORAGE[0] = _HLIB::timestamp_timet64( sStorageDate2 );
			pCharData2->m_tSTORAGE[1] = _HLIB::timestamp_timet64( sStorageDate3 );
			pCharData2->m_tSTORAGE[2] = _HLIB::timestamp_timet64( sStorageDate4 );
		}else{
			break;		
		}

		Sleep(0);
	}

	pDatabase->FreeConnection(pConn);

	return DB_OK;
}

int	CGMToolOdbcBase::UserInvenGetBinary( DWORD dwUserNum, CByteStream &ByteStream )
{
	if ( dwUserNum == 0 ){
		return DB_ERROR;
	}

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	ByteStream.ClearBuffer();

	TCHAR szTemp[256] = {0};
	_snprintf( szTemp, 256, "SELECT UserInven.UserInven FROM UserInven WHERE UserNum=%u", dwUserNum );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	SQLRETURN sReturn = 0;
	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS);
	if (sReturn != SQL_SUCCESS && sReturn != SQL_SUCCESS_WITH_INFO)
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );		
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}	

	BYTE pBuffer[DB_IMAGE_BUF_SIZE] = {0};
	SQLINTEGER  lSize=0, lTotalSize=0;

	while ((sReturn = ::SQLFetch(pConn->hStmt)) != SQL_NO_DATA) 
	{			
		while (1) 
		{
			::ZeroMemory(pBuffer, DB_IMAGE_BUF_SIZE);
			lSize = 0;
			sReturn = ::SQLGetData(pConn->hStmt, 1, SQL_C_BINARY, pBuffer, DB_IMAGE_BUF_SIZE, &lSize);
			if (lSize > 0)
			{
				if (lSize > DB_IMAGE_BUF_SIZE) lSize = DB_IMAGE_BUF_SIZE;
				lTotalSize += lSize;
				ByteStream.WriteBuffer((LPBYTE) pBuffer, lSize);
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

	pDatabase->FreeConnection(pConn);

	return DB_OK;
}

int	CGMToolOdbcBase::UserInvenSaveData( SCHARDATA2* pCharData2 )
{
	if ( !pCharData2 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	_snprintf( szTemp, 1024, "Update UserInven Set "
		"ChaStorage2='%s', ChaStorage3='%s', ChaStorage4='%s', UserMoney=%I64d "
		"WHERE UserNum=%u", 
		_HLIB::cstring_timet24(pCharData2->m_tSTORAGE[0]).GetString(),
		_HLIB::cstring_timet24(pCharData2->m_tSTORAGE[1]).GetString(),
		_HLIB::cstring_timet24(pCharData2->m_tSTORAGE[2]).GetString(),
		pCharData2->m_lnStorageMoney,
		pCharData2->GetUserID() );

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	return pDatabase->ExecuteSQL(szTemp);
}

int	CGMToolOdbcBase::UserInvenSaveBinary( DWORD dwUserNum, BYTE* pData, int nSize )
{
	if ( dwUserNum == 0 ){
		return DB_ERROR;
	}

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	TCHAR szTemp[256] = {0};
	_snprintf( szTemp, 256, "UPDATE UserInven SET UserInven = ? WHERE UserNum=%u", dwUserNum );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	SQLRETURN	sReturn = 0;

	sReturn = ::SQLPrepare(pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS);
	if (sReturn != SQL_SUCCESS && sReturn != SQL_SUCCESS_WITH_INFO) 
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );		
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}	

	SQLINTEGER cbBinaryParam = SQL_LEN_DATA_AT_EXEC(0);

	sReturn = ::SQLBindParameter(pConn->hStmt, 
		1, 
		SQL_PARAM_INPUT,
		SQL_C_BINARY, 
		SQL_LONGVARBINARY,
		nSize, 
		0, 
		(SQLPOINTER) 2, 
		0, 
		&cbBinaryParam);
	if (sReturn != SQL_SUCCESS && sReturn != SQL_SUCCESS_WITH_INFO) 
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );		
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}

	sReturn = ::SQLExecute(pConn->hStmt);
	if (sReturn == SQL_ERROR || sReturn == SQL_SUCCESS_WITH_INFO)
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );		
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}

	SQLPOINTER pToken = NULL;
	while (sReturn == SQL_NEED_DATA) 
	{
		sReturn = ::SQLParamData(pConn->hStmt, &pToken);
		if (sReturn == SQL_NEED_DATA) {				
			::SQLPutData(pConn->hStmt, pData, (SQLINTEGER) nSize);
		}

		Sleep(0);
	}

	pDatabase->FreeConnection(pConn);

	return DB_OK;
}