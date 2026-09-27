#include "stdAfx.h"
#include "GMToolOdbcBase.h"
#include "GMToolOdbcConn.h"
#include "GMToolLogs.h"

#include "GLCharDefine.h"
#include "GLCharData.h"

#include "HLibTimeFunctions.h"
#include "HLibDataConvert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


int CGMToolOdbcBase::EquipmentLock_Find( const char* szSQL, SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE_MAP &mapResult )
{
	mapResult.clear();

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "%s", szSQL );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	SQLRETURN sReturn = 0;
	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS );

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{	
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}

	SQLINTEGER nNum = 0, cbNum=SQL_NTS;
	SQLINTEGER nChaNum = 0, cbChaNum=SQL_NTS;
	SQLCHAR    szChaName[CHR_ID_LENGTH+1] = {0}; SQLINTEGER cbChaName = SQL_NTS;
	SQLCHAR    szChaEPin[CHAR_EQUIPMENT_LOCK_PASS_SIZE+1] = {0}; SQLINTEGER cbChaEPin = SQL_NTS;
	TIMESTAMP_STRUCT sDate; SQLINTEGER cbDate = SQL_NTS;
	
	while (true)
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
			::SQLGetData(pConn->hStmt,  1, SQL_C_LONG,	&nNum,		0, &cbNum);
			::SQLGetData(pConn->hStmt,  2, SQL_C_LONG,	&nChaNum,     0, &cbChaNum);
			::SQLGetData(pConn->hStmt,  3, SQL_C_CHAR,  szChaName,    CHR_ID_LENGTH+1, &cbChaName);
			::SQLGetData(pConn->hStmt,  4, SQL_C_CHAR,  szChaEPin,    CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, &cbChaEPin);
			::SQLGetData(pConn->hStmt,  5, SQL_C_TYPE_TIMESTAMP, &sDate, 0, &cbDate);

			if ( !_HLIB::timestamp_correction( sDate ) ){
				GMTOOL_LOGS::LogError( "Variable sDate value corrected" );
			}

			SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE sDATA;

			if (cbChaName != 0 && cbChaName != -1)
				::StringCchCopy(sDATA.szChaName, USR_ID_LENGTH+1, (const TCHAR*) szChaName);

			if (cbChaEPin != 0 && cbChaEPin != -1)
				::StringCchCopy(sDATA.szEPin, CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, (const TCHAR*) szChaEPin);

			sDATA.dwNum = (DWORD)nNum;
			sDATA.dwChaNum = (DWORD)nChaNum;
			sDATA.tDate = _HLIB::timestamp_timet64( sDate );

			mapResult.insert( std::make_pair( sDATA.dwNum, sDATA ) );
		}else{
			break;
		}

		Sleep(0);
	}

	pDatabase->FreeConnection(pConn);

	return DB_OK;
}

int CGMToolOdbcBase::EquipmentLock_Find_ChaNum( int nChaNum, SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nChaNum < 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT Num, ChaNum, ChaName, ChaEPass, Date FROM ChaEquipmentPass WITH (NOLOCK) WHERE ChaNum = %d ", nChaNum );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d Num, ChaNum, ChaName, ChaEPass, Date FROM ChaEquipmentPass WITH (NOLOCK) WHERE ChaNum = %d ", nLIMITNUM, nChaNum );
	}

	return EquipmentLock_Find( szTemp, mapResult );
}

int CGMToolOdbcBase::EquipmentLock_Find_ChaName( std::string strChaName, SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( strChaName.size() <= 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	std::string strSign = "%";
	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT Num, ChaNum, ChaName, ChaEPass, Date FROM ChaEquipmentPass WITH (NOLOCK) WHERE ChaName LIKE '%s%s%s' "
			"ORDER BY ChaNum", strSign.c_str(), strChaName.c_str(), strSign.c_str() );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d Num, ChaNum, ChaName, ChaEPass, Date FROM ChaEquipmentPass WITH (NOLOCK) WHERE ChaName LIKE '%s%s%s' "
			"ORDER BY ChaNum", nLIMITNUM, strSign.c_str(), strChaName.c_str(), strSign.c_str() );
	}


	return EquipmentLock_Find( szTemp, mapResult );
}

int CGMToolOdbcBase::EquipmentLock_Find_EPin( std::string strEPin, SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	std::string strSign = "%";

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT Num, ChaNum, ChaName, ChaEPass, Date FROM ChaEquipmentPass WITH (NOLOCK) WHERE ChaEPass LIKE '%s%s%s' "
			"ORDER BY ChaNum", strSign.c_str(), strEPin.c_str(), strSign.c_str() );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d Num, ChaNum, ChaName, ChaEPass, Date FROM ChaEquipmentPass WITH (NOLOCK) WHERE ChaEPass LIKE '%s%s%s' "
			"ORDER BY ChaNum", nLIMITNUM, strSign.c_str(), strEPin.c_str(), strSign.c_str() );
	}

	return EquipmentLock_Find( szTemp, mapResult );
}

int CGMToolOdbcBase::EquipmentLock_Delete( int nNUM )
{
	if ( nNUM <= 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[512] = {0};
	_snprintf( szTemp, 512, "Delete From ChaEquipmentPass WHERE Num = %d ", nNUM );

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	return pDatabase->ExecuteSQL(szTemp);
}

int CGMToolOdbcBase::EquipmentLock_Delete_To_Char( int nNUM )
{
	if ( nNUM <= 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[512] = {0};
	_snprintf( szTemp, 512, "Update ChaInfo Set ChaEquipmentLockEnable=0, ChaEquipmentLockStatus=0 WHERE ChaNum = %d ", nNUM );

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	return pDatabase->ExecuteSQL(szTemp);
}

int CGMToolOdbcBase::EquipmentLock_Get( int nNUM, SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE* pDATA )
{
	if ( nNUM <= 0 ){
		return DB_ERROR;
	}

	if ( !pDATA ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "SELECT Num, ChaNum, ChaName, ChaEPass, Date FROM ChaEquipmentPass WITH (NOLOCK) WHERE Num = %d",  nNUM );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	int nRowCount = 0;
	SQLRETURN	sReturn = 0;
	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS);

	SQLINTEGER nNum = 0, cbNum=SQL_NTS;
	SQLINTEGER nChaNum = 0, cbChaNum=SQL_NTS;
	SQLCHAR    szChaName[CHR_ID_LENGTH+1] = {0}; SQLINTEGER cbChaName = SQL_NTS;
	SQLCHAR    szChaEPin[CHAR_EQUIPMENT_LOCK_PASS_SIZE+1] = {0}; SQLINTEGER cbChaEPin = SQL_NTS;
	TIMESTAMP_STRUCT sDate; SQLINTEGER cbDate = SQL_NTS;

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
	}
	else
	{
		while(true)
		{
			sReturn = ::SQLFetch(pConn->hStmt);
			if (sReturn == SQL_ERROR || sReturn == SQL_SUCCESS_WITH_INFO){				
				break;
			}

			if (sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO)
			{	
				::SQLGetData(pConn->hStmt,  1, SQL_C_LONG,	&nNum,		0, &cbNum);
				::SQLGetData(pConn->hStmt,  2, SQL_C_LONG,	&nChaNum,     0, &cbChaNum);
				::SQLGetData(pConn->hStmt,  3, SQL_C_CHAR,  szChaName,    CHR_ID_LENGTH+1, &cbChaName);
				::SQLGetData(pConn->hStmt,  4, SQL_C_CHAR,  szChaEPin,    CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, &cbChaEPin);
				::SQLGetData(pConn->hStmt,  5, SQL_C_TYPE_TIMESTAMP, &sDate, 0, &cbDate);

				if ( !_HLIB::timestamp_correction( sDate ) ){
					GMTOOL_LOGS::LogError( "Variable sDate value corrected" );
				}

				if (cbChaName != 0 && cbChaName != -1)
					::StringCchCopy(pDATA->szChaName, USR_ID_LENGTH+1, (const TCHAR*) szChaName);

				if (cbChaEPin != 0 && cbChaEPin != -1)
					::StringCchCopy(pDATA->szEPin, CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, (const TCHAR*) szChaEPin);

				pDATA->dwNum = (DWORD)nNum;
				pDATA->dwChaNum = (DWORD)nChaNum;
				pDATA->tDate = _HLIB::timestamp_timet64( sDate );

				nRowCount++;
			}else{
				break;		
			}

			Sleep(0);
		}
	}

	pDatabase->FreeConnection(pConn);

	if (nRowCount != 1){ 
		return DB_ERROR;
	}

	return DB_OK;
}

int CGMToolOdbcBase::EquipmentLock_Save( SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE* pDATA )
{
	if ( !pDATA ){
		return DB_ERROR;
	}

	if ( pDATA->dwNum <= 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "UPDATE ChaEquipmentPass SET ChaNum =%u, ChaName ='%s', ChaEPass ='%s' WHERE Num = %u ", 
			pDATA->dwChaNum, pDATA->szChaName, pDATA->szEPin, pDATA->dwNum );

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	if ( pDatabase->ExecuteSQL(szTemp) != DB_OK){
		return DB_ERROR;
	}

	return DB_OK;
}

int CGMToolOdbcBase::EquipmentLock_New( SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE* pDATA )
{
	if ( !pDATA ){
		return DB_ERROR;
	}

	CString strName (pDATA->szChaName);
	strName.Trim(_T(" ")); 
	strName.Replace(_T("'"), _T("''"));

	if (strName.GetLength() >= (CHR_ID_LENGTH+1))
	{
		return DB_ERROR;
	}

	CString strEPin (pDATA->szEPin);
	strEPin.Trim(_T(" ")); 
	strEPin.Replace(_T("'"), _T("''"));

	if (strEPin.GetLength() >= (CHAR_EQUIPMENT_LOCK_PASS_SIZE+1))
	{
		return DB_ERROR;
	}

	TCHAR szTemp[128] = {0};
	_snprintf( szTemp, 128, "{call sp_InsertEquipmentPass(%d,'%s','%s',?)}", pDATA->dwChaNum, strName.GetString(), strEPin.GetString() ); 

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	int nReturn = pDatabase->ExecuteSpInt(szTemp);
	return nReturn;
}