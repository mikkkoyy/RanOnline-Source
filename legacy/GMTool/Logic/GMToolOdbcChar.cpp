#include "stdAfx.h"
#include "GMToolOdbcBase.h"
#include "GMToolOdbcConn.h"
#include "GMToolLogs.h"

#include "GLCharDefine.h"
#include "GLCharData.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CGMToolOdbcBase::UserCharactersRead( int nUSERNUM, SGMTOOL_DATA_USER_CHARACTERS_VEC &vecResult )
{
	if ( nUSERNUM <= 0 ){
		return DB_ERROR;
	}

	vecResult.clear();

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "SELECT ChaNum, ChaName, ChaClass, ChaSchool, ChaLevel, ChaDeleted FROM ChaInfo WITH (NOLOCK) WHERE UserNum = %d ORDER BY ChaNum", nUSERNUM );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	SQLRETURN sReturn = 0;
	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS );

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{	
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}

	SQLINTEGER nChaNum = 0, cbChaNum=SQL_NTS;
	SQLCHAR    szChaName[CHR_ID_LENGTH+1] = {0}; SQLINTEGER cbChaName = SQL_NTS;
	SQLINTEGER nChaClass = 0, cbChaClass=SQL_NTS;
	SQLINTEGER nChaSchool = 0, cbChaSchool=SQL_NTS;
	SQLINTEGER nChaLevel = 0, cbChaLevel=SQL_NTS;
	SQLINTEGER nChaDeleted = 0, cbChaDeleted=SQL_NTS;

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
			::SQLGetData(pConn->hStmt,  1, SQL_C_LONG,	&nChaNum, 0, &cbChaNum );
			::SQLGetData(pConn->hStmt,  2, SQL_C_CHAR,  szChaName, CHR_ID_LENGTH, &cbChaName );
			::SQLGetData(pConn->hStmt,  3, SQL_C_LONG,	&nChaClass, 0, &cbChaClass );
			::SQLGetData(pConn->hStmt,  4, SQL_C_LONG,	&nChaSchool, 0, &cbChaSchool );
			::SQLGetData(pConn->hStmt,  5, SQL_C_LONG,	&nChaLevel, 0, &cbChaLevel );
			::SQLGetData(pConn->hStmt,  6, SQL_C_LONG,	&nChaDeleted, 0, &cbChaDeleted );

			SGMTOOL_DATA_USER_CHARACTERS sDATA;

			if (cbChaName != 0 && cbChaName != -1)
				::StringCchCopy(sDATA.szChaName, CHR_ID_LENGTH+1, (const TCHAR*) szChaName );

			sDATA.dwChaNum = (DWORD)nChaNum;
			sDATA.wChaClass = (WORD)CharClassToIndex( (EMCHARCLASS)nChaClass );
			sDATA.wChaSchool = (WORD)nChaSchool;
			sDATA.wChaLevel = (WORD)nChaLevel;
			sDATA.bChaDeleted = (BOOL)nChaDeleted;
			vecResult.push_back ( sDATA );
		}else{
			break;
		}

		Sleep(0);
	}	  
	pDatabase->FreeConnection(pConn);

	return DB_OK;
}


int CGMToolOdbcBase::CharFind( const char* szSQL, SGMTOOL_DATA_CHAR_SIMPLE_MAP &mapResult )
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

	SQLINTEGER nUserNum = 0, cbUserNum=SQL_NTS;
	SQLINTEGER nChaNum = 0, cbChaNum=SQL_NTS;
	SQLCHAR    szChaName[CHR_ID_LENGTH+1] = {0}; SQLINTEGER cbChaName = SQL_NTS;
	SQLINTEGER nChaClass = 0, cbChaClass=SQL_NTS;
	SQLINTEGER nChaSchool = 0, cbChaSchool=SQL_NTS;
	SQLINTEGER nChaLevel = 0, cbChaLevel=SQL_NTS;
	SQLINTEGER nChaOnline = 0, cbChaOnline=SQL_NTS;
	SQLINTEGER nChaDeleted = 0, cbChaDeleted=SQL_NTS;

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
			::SQLGetData(pConn->hStmt,  1, SQL_C_LONG,	&nUserNum,    0, &cbUserNum);
			::SQLGetData(pConn->hStmt,  2, SQL_C_LONG,	&nChaNum,     0, &cbChaNum);
			::SQLGetData(pConn->hStmt,  3, SQL_C_CHAR,  szChaName,    CHR_ID_LENGTH, &cbChaName);
			::SQLGetData(pConn->hStmt,  4, SQL_C_LONG,	&nChaClass,   0, &cbChaClass);
			::SQLGetData(pConn->hStmt,  5, SQL_C_LONG,	&nChaSchool,   0, &cbChaSchool);
			::SQLGetData(pConn->hStmt,  6, SQL_C_LONG,	&nChaLevel,   0, &cbChaLevel);
			::SQLGetData(pConn->hStmt,  7, SQL_C_LONG,	&nChaOnline,  0, &cbChaOnline);
			::SQLGetData(pConn->hStmt,  8, SQL_C_LONG,	&nChaDeleted, 0, &cbChaDeleted);

			SGMTOOL_DATA_CHAR_SIMPLE sDATA;

			if (cbChaName != 0 && cbChaName != -1)
				::StringCchCopy(sDATA.szChaName, USR_ID_LENGTH+1, (const TCHAR*) szChaName);

			sDATA.dwUserNum = (DWORD)nUserNum;
			sDATA.dwChaNum = (DWORD)nChaNum;
			sDATA.wChaClass = (WORD)CharClassToIndex( (EMCHARCLASS)nChaClass );
			sDATA.wChaLevel = (WORD)nChaLevel;
			sDATA.wChaSchool = (WORD)nChaSchool;
			sDATA.bChaOnline = (BOOL)nChaOnline;
			sDATA.bChaDeleted = (BOOL)nChaDeleted;

			mapResult.insert( std::make_pair( sDATA.dwChaNum, sDATA ) );
		}else{
			break;
		}

		Sleep(0);
	}

	pDatabase->FreeConnection(pConn);

	return DB_OK;
}

int CGMToolOdbcBase::CharFind_UserNumber( int nUserNum, SGMTOOL_DATA_CHAR_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nUserNum < 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT UserNum, ChaNum, ChaName, ChaClass, ChaSchool, ChaLevel, ChaOnline, ChaDeleted FROM ChaInfo WITH (NOLOCK) WHERE UserNum = %d ", nUserNum );
	}
	else{

		_snprintf( szTemp, 1024, "SELECT TOP %d UserNum, ChaNum, ChaName, ChaClass, ChaSchool, ChaLevel, ChaOnline, ChaDeleted FROM ChaInfo WITH (NOLOCK) WHERE UserNum = %d ", nLIMITNUM, nUserNum );
	}

	return CharFind( szTemp, mapResult );
}

int CGMToolOdbcBase::CharFind_ChaNumber( int nChaNum, SGMTOOL_DATA_CHAR_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nChaNum < 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT UserNum, ChaNum, ChaName, ChaClass, ChaSchool, ChaLevel, ChaOnline, ChaDeleted FROM ChaInfo WITH (NOLOCK) WHERE ChaNum = %d ", nChaNum );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d UserNum, ChaNum, ChaName, ChaClass, ChaSchool, ChaLevel, ChaOnline, ChaDeleted FROM ChaInfo WITH (NOLOCK) WHERE ChaNum = %d ", nLIMITNUM, nChaNum );
	}

	return CharFind( szTemp, mapResult );
}

int CGMToolOdbcBase::CharFind_ChaName( std::string strChaName, SGMTOOL_DATA_CHAR_SIMPLE_MAP &mapResult, int nLIMITNUM )
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
		_snprintf( szTemp, 1024, "SELECT UserNum, ChaNum, ChaName, ChaClass, ChaSchool, ChaLevel, ChaOnline, ChaDeleted FROM ChaInfo WITH (NOLOCK) WHERE ChaName LIKE '%s%s%s' "
			"ORDER BY ChaNum", strSign.c_str(), strChaName.c_str(), strSign.c_str() );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d UserNum, ChaNum, ChaName, ChaClass, ChaSchool, ChaLevel, ChaOnline, ChaDeleted FROM ChaInfo WITH (NOLOCK) WHERE ChaName LIKE '%s%s%s' "
			"ORDER BY ChaNum", nLIMITNUM, strSign.c_str(), strChaName.c_str(), strSign.c_str() );
	}


	return CharFind( szTemp, mapResult );
}

int CGMToolOdbcBase::CharFind_Online( SGMTOOL_DATA_CHAR_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT UserNum, ChaNum, ChaName, ChaClass, ChaSchool, ChaLevel, ChaOnline, ChaDeleted FROM ChaInfo WITH (NOLOCK) WHERE ChaOnline = 1 ");
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d UserNum, ChaNum, ChaName, ChaClass, ChaSchool, ChaLevel, ChaOnline, ChaDeleted FROM ChaInfo WITH (NOLOCK) WHERE ChaOnline = 1 ", nLIMITNUM );
	}

	return CharFind( szTemp, mapResult );
}

int CGMToolOdbcBase::CharFind_ALL( SGMTOOL_DATA_CHAR_SIMPLE_MAP &mapResult )
{
	TCHAR szTemp[1024] = {0};

	_snprintf( szTemp, 1024, "SELECT UserNum, ChaNum, ChaName, ChaClass, ChaSchool, ChaLevel, ChaOnline, ChaDeleted FROM ChaInfo WITH (NOLOCK) ");

	return CharFind( szTemp, mapResult );
}

int CGMToolOdbcBase::CharOnlineCheck( int nCHANUM, BOOL& bONLINE )
{
	if ( nCHANUM <= 0 ){
		return DB_ERROR;
	}

	bONLINE = FALSE;

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "SELECT ChaOnline FROM ChaInfo WITH (NOLOCK) WHERE ChaNum = %d ",  nCHANUM );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	SQLRETURN	sReturn = 0;
	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS);

	SQLINTEGER nChaOnline = 0, cbChaOnline=SQL_NTS;

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

			if (sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO){	
				::SQLGetData(pConn->hStmt,  1, SQL_C_LONG,	&nChaOnline,    0, &cbChaOnline);
				bONLINE = ( BOOL ) nChaOnline;
			}else{
				break;		
			}

			Sleep(0);
		}
	}
	pDatabase->FreeConnection(pConn);

	return DB_OK;
}

int CGMToolOdbcBase::CharSetDeleted( int nCHANUM, BOOL bDeleted )
{
	if ( nCHANUM <= 0 )	{
		return DB_ERROR;
	}

	TCHAR szTemp[512] = {0};
	_snprintf( szTemp, 512, "UPDATE ChaInfo SET ChaDeleted = %d WHERE ChaNum= %u", bDeleted, nCHANUM );

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	return pDatabase->ExecuteSQL(szTemp);
}

int CGMToolOdbcBase::CharSetOnline( int nCHANUM, BOOL bOnline )
{
	if ( nCHANUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[128] = {0};
	_snprintf( szTemp, 512, "UPDATE ChaInfo SET ChaOnline = %d WHERE ChaNum= %u", bOnline, nCHANUM );

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	return pDatabase->ExecuteSp(szTemp);
}

int CGMToolOdbcBase::CharNameCheck( std::string strCHANAME, BOOL &bEXIST, int &nCHANUM )
{
	bEXIST = FALSE;
	nCHANUM = 0;

	if ( strCHANAME.size() <= 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "SELECT ChaNum FROM ChaInfo WITH (NOLOCK) WHERE ChaName = '%s' ",  strCHANAME.c_str() );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	SQLRETURN	sReturn = 0;
	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS);

	SQLINTEGER nChaNum = 0, cbChaNum =SQL_NTS;

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}
	else
	{
		while(true)
		{
			sReturn = ::SQLFetch(pConn->hStmt);
			if ( sReturn == SQL_ERROR || sReturn == SQL_SUCCESS_WITH_INFO ){				
				break;
			}

			if ( sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO )
			{	
				::SQLGetData(pConn->hStmt,  1, SQL_C_LONG,	&nChaNum, 0, &cbChaNum);

				nCHANUM = nChaNum;
				bEXIST = TRUE;
			}else{
				break;		
			}

			Sleep(0);
		}
	}

	pDatabase->FreeConnection(pConn);

	return DB_OK;
}