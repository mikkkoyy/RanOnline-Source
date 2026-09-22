#include "stdAfx.h"
#include "GMToolOdbcBase.h"
#include "GMToolOdbcConn.h"
#include "GMToolLogs.h"
#include "GMToolGlobal.h"
#include "HLibTimeFunctions.h"
#include "HLibDataConvert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CGMToolOdbcBase::UserFind( const char* szSQL, SGMTOOL_USER_SIMPLE_MAP &mapResult )
{
	mapResult.clear();

	CGMToolOdbcConn* pDatabase = m_pUserDB;
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
	SQLCHAR    szUserID[USR_ID_LENGTH+1] = {0}; SQLINTEGER cbUserID = SQL_NTS;
	SQLCHAR    szUserEmail[USR_EMAIL_LENGTH+1] = {0}; SQLINTEGER cbUserEmail = SQL_NTS;
	SQLINTEGER nUserType = 0, cbUserType=SQL_NTS;
	SQLINTEGER nUserLoginState = 0, cbUserLoginState=SQL_NTS;
	SQLINTEGER nUserAvailable = 0, cbUserAvailable=SQL_NTS;
	SQLINTEGER nUserBlock = 0, cbUserBlock=SQL_NTS;
	TIMESTAMP_STRUCT sUserBlockDate; SQLINTEGER cbUserBlockDate = SQL_NTS;
	TIMESTAMP_STRUCT sChatBlockDate; SQLINTEGER cbChatBlockDate = SQL_NTS;
	SQLINTEGER nUserPoint = 0, cbUserPoint=SQL_NTS;

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
			::SQLGetData(pConn->hStmt,  1, SQL_C_LONG,	&nUserNum,           0, &cbUserNum);
			::SQLGetData(pConn->hStmt,  2, SQL_C_CHAR,  szUserID, USR_ID_LENGTH, &cbUserID);
			::SQLGetData(pConn->hStmt,  3, SQL_C_CHAR,  szUserEmail, USR_EMAIL_LENGTH, &cbUserEmail);
			::SQLGetData(pConn->hStmt,  4, SQL_C_LONG,	&nUserType,          0, &cbUserType);
			::SQLGetData(pConn->hStmt,  5, SQL_C_LONG,	&nUserLoginState,    0, &cbUserLoginState);
			::SQLGetData(pConn->hStmt,  6, SQL_C_LONG,	&nUserAvailable,     0, &cbUserAvailable);
			::SQLGetData(pConn->hStmt,  7, SQL_C_LONG,	&nUserBlock,         0, &cbUserBlock);
			::SQLGetData(pConn->hStmt,  8, SQL_C_TYPE_TIMESTAMP, &sUserBlockDate, 0, &cbUserBlockDate);
			::SQLGetData(pConn->hStmt,  9, SQL_C_TYPE_TIMESTAMP, &sChatBlockDate, 0, &cbChatBlockDate);
			::SQLGetData(pConn->hStmt,  10, SQL_C_LONG,	&nUserPoint,         0, &cbUserPoint);

			if ( !_HLIB::timestamp_correction( sUserBlockDate ) ){
				GMTOOL_LOGS::LogError( "Variable sUserBlockDate value corrected" );
			}

			if ( !_HLIB::timestamp_correction( sChatBlockDate ) ){
				GMTOOL_LOGS::LogError( "Variable sChatBlockDate value corrected" );
			}

			SGMTOOL_USER_SIMPLE sDATA;
			if (cbUserID != 0 && cbUserID != -1)
				::StringCchCopy(sDATA.szUserID, USR_ID_LENGTH+1, (const TCHAR*) szUserID);

			if (cbUserEmail != 0 && cbUserEmail != -1)
				::StringCchCopy(sDATA.szUserEmail, USR_EMAIL_LENGTH+1, (const TCHAR*) szUserEmail);

			sDATA.dwUserNum = (DWORD)nUserNum;
			sDATA.wUserType = (WORD)nUserType;
			sDATA.bUserLoginState = (BOOL)nUserLoginState;
			sDATA.bUserAvailable = (BOOL)nUserAvailable;
			sDATA.bUserBlock = (BOOL)nUserBlock;
			sDATA.tUserBlockDate = _HLIB::timestamp_timet64( sUserBlockDate );
			sDATA.tChatBlockDate = _HLIB::timestamp_timet64( sChatBlockDate );
			sDATA.dwUserPoint = (DWORD)nUserPoint;

			mapResult.insert( std::make_pair( sDATA.dwUserNum, sDATA ) );
		}
		else
		{
			break;
		}

		Sleep(0);
	}	  
	pDatabase->FreeConnection(pConn);

	return DB_OK;
}

int CGMToolOdbcBase::UserFind_UserNumber( int nUSERNUM, SGMTOOL_USER_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nUSERNUM <= 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT UserNum, UserID, UserEmail, UserType, UserLoginState, UserAvailable, UserBlock, UserBlockDate, ChatBlockDate, UserPoint FROM UserInfo WITH (NOLOCK) WHERE UserNum = '%d'", nUSERNUM );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d UserNum, UserID, UserEmail, UserType, UserLoginState, UserAvailable, UserBlock, UserBlockDate, ChatBlockDate, UserPoint FROM UserInfo WITH (NOLOCK) WHERE UserNum = '%d'", nLIMITNUM, nUSERNUM );
	}

	return UserFind( szTemp, mapResult );
}

int CGMToolOdbcBase::UserFind_UserName( std::string strUSERID, SGMTOOL_USER_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( strUSERID.size() <= 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	std::string strSign = "%";
	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT UserNum, UserID, UserEmail, UserType, UserLoginState, UserAvailable, UserBlock, UserBlockDate, ChatBlockDate, UserPoint FROM UserInfo WITH (NOLOCK) WHERE UserID LIKE '%s%s%s' "
			"ORDER BY UserNum", strSign.c_str(), strUSERID.c_str(), strSign.c_str() );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d UserNum, UserID, UserEmail, UserType, UserLoginState, UserAvailable, UserBlock, UserBlockDate, ChatBlockDate, UserPoint FROM UserInfo WITH (NOLOCK) WHERE UserID LIKE '%s%s%s' "
			"ORDER BY UserNum", nLIMITNUM, strSign.c_str(), strUSERID.c_str(), strSign.c_str() );
	}

	return UserFind( szTemp, mapResult );
}

int CGMToolOdbcBase::UserFind_UserEmail( std::string strUSEREMAIL, SGMTOOL_USER_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( strUSEREMAIL.size() <= 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	std::string strSign = "%";
	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT UserNum, UserID, UserEmail, UserType, UserLoginState, UserAvailable, UserBlock, UserBlockDate, ChatBlockDate, UserPoint FROM UserInfo WITH (NOLOCK) WHERE UserEmail LIKE '%s%s%s' "
			"ORDER BY UserNum", strSign.c_str(), strUSEREMAIL.c_str(), strSign.c_str() );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d UserNum, UserID, UserEmail, UserType, UserLoginState, UserAvailable, UserBlock, UserBlockDate, ChatBlockDate, UserPoint FROM UserInfo WITH (NOLOCK) WHERE UserEmail LIKE '%s%s%s' "
			"ORDER BY UserNum", nLIMITNUM, strSign.c_str(), strUSEREMAIL.c_str(), strSign.c_str() );
	}

	return UserFind( szTemp, mapResult );
}

int CGMToolOdbcBase::UserFind_UserGM( SGMTOOL_USER_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT UserNum, UserID, UserEmail, UserType, UserLoginState, UserAvailable, UserBlock, UserBlockDate, ChatBlockDate, UserPoint FROM UserInfo WITH (NOLOCK) WHERE UserType >= '%d' "
			"ORDER BY UserNum", USER_GM3 );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d UserNum, UserID, UserEmail, UserType, UserLoginState, UserAvailable, UserBlock, UserBlockDate, ChatBlockDate, UserPoint FROM UserInfo WITH (NOLOCK) WHERE UserType >= '%d' "
			"ORDER BY UserNum", nLIMITNUM, USER_GM3 );
	}

	return UserFind( szTemp, mapResult );
}

int CGMToolOdbcBase::UserFind_Online( SGMTOOL_USER_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT UserNum, UserID, UserEmail, UserType, UserLoginState, UserAvailable, UserBlock, UserBlockDate, ChatBlockDate, UserPoint FROM UserInfo WITH (NOLOCK) WHERE UserLoginState >= '%d' "
			"ORDER BY UserNum", 1 );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d UserNum, UserID, UserEmail, UserType, UserLoginState, UserAvailable, UserBlock, UserBlockDate, ChatBlockDate, UserPoint FROM UserInfo WITH (NOLOCK) WHERE UserLoginState >= '%d' "
			"ORDER BY UserNum", nLIMITNUM, 1 );
	}

	return UserFind( szTemp, mapResult );
}

int CGMToolOdbcBase::UserIDCheck( std::string strUSERID, BOOL &bEXIST, int &nUSERNUM )
{
	bEXIST = FALSE;
	nUSERNUM = 0;

	if ( strUSERID.size() <= 0 ){
		return DB_ERROR;
	}
	
	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "SELECT UserNum FROM UserInfo WITH (NOLOCK) WHERE UserID = '%s' ",  strUSERID.c_str() );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	CGMToolOdbcConn* pDatabase = m_pUserDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	SQLRETURN	sReturn = 0;
	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS);

	SQLINTEGER nUserNum = 0, cbUserNum=SQL_NTS;

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

			if ( sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO ){	
				::SQLGetData(pConn->hStmt,  1, SQL_C_LONG,	&nUserNum, 0, &cbUserNum);

				nUSERNUM = nUserNum;
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

int CGMToolOdbcBase::UserNew( SGMTOOL_USER_NEW* pData )
{
	if ( pData == NULL){ 
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	_snprintf( szTemp, 1024, 
		"INSERT Into UserInfo ( UserName, UserID, UserPass, UserPass2, UserEmail, UserType, ChaRemain, ChaTestRemain, UserPoint, UserAvailable, CreateDate )"
		" Values ( '%s', '%s', '%s', '%s', '%s', %u, %u, %u, %u, %u, '%s' )"
		, pData->szUserID, pData->szUserID, pData->szUserPass, pData->szUserPass2, pData->szUserEmail
		, pData->wUserType, pData->wChaRemain, pData->wChaRemain, pData->dwUserPoint, 1, 
		_HLIB::cstring_timet24( CTime::GetCurrentTime().GetTime() ).GetString() 
		);

	CGMToolOdbcConn* pDatabase = m_pUserDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	if ( pDatabase->ExecuteSQL(szTemp) != DB_OK){
		return DB_ERROR;
	}

	return DB_OK;
}

int CGMToolOdbcBase::UserOnlineCheck( int nUSERNUM, BOOL& bONLINE )
{
	if ( nUSERNUM <= 0 ){
		return DB_ERROR;
	}

	bONLINE = FALSE;

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "SELECT UserLoginState FROM UserInfo WITH (NOLOCK) WHERE UserNum = %d ",  nUSERNUM );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	CGMToolOdbcConn* pDatabase = m_pUserDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	SQLRETURN	sReturn = 0;
	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS);

	SQLINTEGER nUserLoginState = 0, cbUserLoginState=SQL_NTS;

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
				::SQLGetData(pConn->hStmt,  1, SQL_C_LONG,	&nUserLoginState,    0, &cbUserLoginState);
				bONLINE = ( BOOL ) nUserLoginState;
			}else{
				break;		
			}

			Sleep(0);
		}
	}
	pDatabase->FreeConnection(pConn);

	return DB_OK;
}

int CGMToolOdbcBase::UserLogOut( int nUSERNUM )
{
	if ( nUSERNUM <= 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "UPDATE UserInfo SET UserLoginState = %u WHERE UserNum = %u ", 
		0, nUSERNUM );

	CGMToolOdbcConn* pDatabase = m_pUserDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	if ( pDatabase->ExecuteSQL(szTemp) != DB_OK){
		return DB_ERROR;
	}

	return DB_OK;
}

int CGMToolOdbcBase::UserBlockSet( int nUSERNUM, BOOL bBlock, __time64_t tTime )
{
	if ( nUSERNUM <= 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "UPDATE UserInfo SET UserBlock = %u, UserBlockDate = '%s' WHERE UserNum = %u ", 
		bBlock, _HLIB::cstring_timet24( tTime ).GetString(), nUSERNUM );

	CGMToolOdbcConn* pDatabase = m_pUserDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	if ( pDatabase->ExecuteSQL(szTemp) != DB_OK){
		return DB_ERROR;
	}

	return DB_OK;
}

int CGMToolOdbcBase::UserChatBlockSet( int nUSERNUM, __time64_t tTime )
{
	if ( nUSERNUM <= 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "UPDATE UserInfo SET ChatBlockDate = '%s' WHERE UserNum = %u ", 
		_HLIB::cstring_timet24( tTime ).GetString(), nUSERNUM );

	CGMToolOdbcConn* pDatabase = m_pUserDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	if ( pDatabase->ExecuteSQL(szTemp) != DB_OK){
		return DB_ERROR;
	}

	return DB_OK;
}

int CGMToolOdbcBase::UserDataGet( int nUSERNUM, SGMTOOL_DATA_USER* pDATA )
{
	if ( nUSERNUM <= 0 ){
		return DB_ERROR;
	}

	if ( !pDATA ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "SELECT UserNum, UserID, UserPass, UserPass2, UserEmail, UserType, UserType2, UserLoginState, UserAvailable, ChaRemain, UserPoint, UserBlock, "
		"UserBlockDate, ChatBlockDate, PremiumDate, CreateDate, LastLoginDate, SGNum, SvrNum FROM UserInfo WITH (NOLOCK) WHERE UserNum = %d ",  nUSERNUM );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	CGMToolOdbcConn* pDatabase = m_pUserDB;
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

	SQLINTEGER nUserNum = 0, cbUserNum=SQL_NTS;
	SQLCHAR    szUserID[USR_ID_LENGTH+1] = {0}; SQLINTEGER cbUserID = SQL_NTS;
	SQLCHAR    szUserPass[USR_PASS_LENGTH+1] = {0}; SQLINTEGER cbUserPass = SQL_NTS;
	SQLCHAR    szUserPass2[USR_PASS_LENGTH+1] = {0}; SQLINTEGER cbUserPass2 = SQL_NTS;
	SQLCHAR    szUserEmail[USR_EMAIL_LENGTH+1] = {0}; SQLINTEGER cbUserEmail = SQL_NTS;
	SQLINTEGER nUserType = 0, cbUserType=SQL_NTS;
	SQLINTEGER nUserType2 = 0, cbUserType2=SQL_NTS;
	SQLINTEGER nUserLoginState = 0, cbUserLoginState=SQL_NTS;
	SQLINTEGER nUserAvailable = 0, cbUserAvailable=SQL_NTS;
	SQLINTEGER nChaRemain = 0, cbChaRemain=SQL_NTS;
	SQLINTEGER nUserPoint = 0, cbUserPoint=SQL_NTS;
	SQLINTEGER nUserBlock = 0, cbUserBlock=SQL_NTS;
	TIMESTAMP_STRUCT sUserBlockDate; SQLINTEGER cbUserBlockDate = SQL_NTS;
	TIMESTAMP_STRUCT sChatBlockDate; SQLINTEGER cbChatBlockDate = SQL_NTS;
	TIMESTAMP_STRUCT sPremiumDate; SQLINTEGER cbPremiumDate = SQL_NTS;
	TIMESTAMP_STRUCT sCreateDate; SQLINTEGER cbCreateDate = SQL_NTS;
	TIMESTAMP_STRUCT sLastLoginDate; SQLINTEGER cbLastLoginDate = SQL_NTS;
	SQLINTEGER nSGNum = 0, cbSGNum=SQL_NTS;
	SQLINTEGER nSVRNum = 0, cbSVRNum=SQL_NTS;

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
				::SQLGetData(pConn->hStmt,  1, SQL_C_LONG,	&nUserNum, 0, &cbUserNum);
				::SQLGetData(pConn->hStmt,  2, SQL_C_CHAR,  szUserID, USR_ID_LENGTH, &cbUserID);
				::SQLGetData(pConn->hStmt,  3, SQL_C_CHAR,  szUserPass, USR_PASS_LENGTH, &cbUserPass);
				::SQLGetData(pConn->hStmt,  4, SQL_C_CHAR,  szUserPass2, USR_PASS_LENGTH, &cbUserPass2);
				::SQLGetData(pConn->hStmt,  5, SQL_C_CHAR,  szUserEmail, USR_EMAIL_LENGTH, &cbUserEmail);
				::SQLGetData(pConn->hStmt,  6, SQL_C_LONG,	&nUserType,          0, &cbUserType);
				::SQLGetData(pConn->hStmt,  7, SQL_C_LONG,	&nUserType2,         0, &cbUserType2);
				::SQLGetData(pConn->hStmt,  8, SQL_C_LONG,	&nUserLoginState,    0, &cbUserLoginState);
				::SQLGetData(pConn->hStmt,  9, SQL_C_LONG,	&nUserAvailable,     0, &cbUserAvailable);
				::SQLGetData(pConn->hStmt,  10, SQL_C_LONG,	&nChaRemain,		 0, &cbChaRemain);
				::SQLGetData(pConn->hStmt,  11, SQL_C_LONG,	&nUserPoint,         0, &cbUserPoint);
				::SQLGetData(pConn->hStmt,  12, SQL_C_LONG,	&nUserBlock,         0, &cbUserBlock);
				::SQLGetData(pConn->hStmt,  13, SQL_C_TYPE_TIMESTAMP, &sUserBlockDate, 0, &cbUserBlockDate);
				::SQLGetData(pConn->hStmt,  14, SQL_C_TYPE_TIMESTAMP, &sChatBlockDate, 0, &cbChatBlockDate);
				::SQLGetData(pConn->hStmt,  15, SQL_C_TYPE_TIMESTAMP, &sPremiumDate, 0, &cbPremiumDate);
				::SQLGetData(pConn->hStmt,  16, SQL_C_TYPE_TIMESTAMP, &sCreateDate, 0, &cbCreateDate);
				::SQLGetData(pConn->hStmt,  17, SQL_C_TYPE_TIMESTAMP, &sLastLoginDate, 0, &cbLastLoginDate);
				::SQLGetData(pConn->hStmt,  18, SQL_C_LONG,	&nSGNum,  0, &cbSGNum);
				::SQLGetData(pConn->hStmt,  19, SQL_C_LONG,	&nSVRNum, 0, &cbSVRNum);

				pDATA->dwUserNum = ( DWORD ) nUserNum;

				if (cbUserID != 0 && cbUserID != -1)
					::StringCchCopy( pDATA->szUserID, USR_ID_LENGTH+1, (const char*) szUserID);

				if (cbUserPass != 0 && cbUserPass != -1)
					::StringCchCopy( pDATA->szUserPass, USR_PASS_LENGTH+1, (const char*) szUserPass);

				if (cbUserPass2 != 0 && cbUserPass2 != -1)
					::StringCchCopy( pDATA->szUserPass2, USR_PASS_LENGTH+1, (const char*) szUserPass2);

				if (cbUserEmail != 0 && cbUserEmail != -1)
					::StringCchCopy( pDATA->szUserEmail, USR_EMAIL_LENGTH+1, (const char*) szUserEmail);

				if ( !_HLIB::timestamp_correction( sUserBlockDate ) ){
					GMTOOL_LOGS::LogError( "Variable sUserBlockDate value corrected" );
				}

				if ( !_HLIB::timestamp_correction( sChatBlockDate ) ){
					GMTOOL_LOGS::LogError( "Variable sChatBlockDate value corrected" );
				}

				if ( !_HLIB::timestamp_correction( sPremiumDate ) ){
					GMTOOL_LOGS::LogError( "Variable sPremiumDate value corrected" );
				}

				if ( !_HLIB::timestamp_correction( sCreateDate ) ){
					GMTOOL_LOGS::LogError( "Variable sCreateDate value corrected" );
				}

				if ( !_HLIB::timestamp_correction( sLastLoginDate ) ){
					GMTOOL_LOGS::LogError( "Variable sLastLoginDate value corrected" );
				}

				pDATA->wUserType = (WORD) nUserType;
				pDATA->wUserType2 = (WORD) nUserType2;
				pDATA->bUserLoginState = (BOOL) nUserLoginState;
				pDATA->bUserAvailable = (BOOL) nUserAvailable;
				pDATA->wChaRemain = (WORD) nChaRemain;
				pDATA->dwUserPoint = (DWORD) nUserPoint;
				pDATA->bUserBlock = (BOOL) nUserBlock;
				pDATA->tUserBlockDate = _HLIB::timestamp_timet64( sUserBlockDate );
				pDATA->tChatBlockDate = _HLIB::timestamp_timet64( sChatBlockDate );
				pDATA->tPremiumDate = _HLIB::timestamp_timet64( sPremiumDate );
				pDATA->tCreateDate = _HLIB::timestamp_timet64( sCreateDate );
				pDATA->tLastLoginDate = _HLIB::timestamp_timet64( sLastLoginDate );
				pDATA->wServerGroup = (WORD) nSGNum;
				pDATA->wServerNumber = (WORD) nSVRNum;

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

int CGMToolOdbcBase::UserDataSave( SGMTOOL_DATA_USER* pDATA )
{
	if ( !pDATA ){
		return DB_ERROR;
	}

	if ( pDATA->dwUserNum <= 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "UPDATE UserInfo SET UserPass ='%s', UserPass2 ='%s', UserEmail ='%s', "
		"UserType = %u, UserType2 = %u,UserLoginState = %u, UserAvailable = %u, ChaRemain = %u, UserPoint = %u, "
		"UserBlock = %u, UserBlockDate = '%s', ChatBlockDate = '%s', PremiumDate = '%s' "
		"WHERE UserNum = %u ", 
		pDATA->szUserPass, pDATA->szUserPass2, pDATA->szUserEmail
		, pDATA->wUserType, pDATA->wUserType2, pDATA->bUserLoginState, pDATA->bUserAvailable, pDATA->wChaRemain, pDATA->dwUserPoint, pDATA->bUserBlock
		, _HLIB::cstring_timet24( pDATA->tUserBlockDate ).GetString()
		, _HLIB::cstring_timet24( pDATA->tChatBlockDate ).GetString()
		, _HLIB::cstring_timet24( pDATA->tPremiumDate ).GetString()
		, pDATA->dwUserNum
		);

	CGMToolOdbcConn* pDatabase = m_pUserDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	if ( pDatabase->ExecuteSQL(szTemp) != DB_OK){
		return DB_ERROR;
	}

	return DB_OK;
}

int CGMToolOdbcBase::UserIPLOGRead( int nUSERNUM, SGMTOOL_DATA_USER_IPLOG_VEC &vecResult )
{
	vecResult.clear();

	if ( nUSERNUM <= 0 ){
		return DB_ERROR;
	}

	CGMToolOdbcConn* pDatabase = m_pUserDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "SELECT LogDate, LogIpAddress FROM LogLogin WITH (NOLOCK) WHERE UserNum = %d AND LogInOut = %d ORDER BY LogDate desc", nUSERNUM, 1 );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	SQLRETURN sReturn = 0;
	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS );

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{	
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}

	TIMESTAMP_STRUCT sLogDate; SQLINTEGER cbLogDate = SQL_NTS;
	SQLCHAR    szLogIpAddress[KOR_USERIP+1] = {0}; SQLINTEGER cbLogIpAddress = SQL_NTS;

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
			::SQLGetData(pConn->hStmt,  1, SQL_C_TYPE_TIMESTAMP, &sLogDate, 0, &cbLogDate);
			::SQLGetData(pConn->hStmt,  2, SQL_C_CHAR,  szLogIpAddress, KOR_USERIP, &cbLogIpAddress );

			if ( !_HLIB::timestamp_correction( sLogDate ) ){
				GMTOOL_LOGS::LogError( "Variable sLogDate value corrected" );
			}

			SGMTOOL_DATA_USER_IPLOG sDATA;

			if (cbLogIpAddress != 0 && cbLogIpAddress != -1)
				::StringCchCopy(sDATA.szLogIpAddress, KOR_USERIP+1, (const TCHAR*) szLogIpAddress );

			sDATA.tLogDate = _HLIB::timestamp_timet64( sLogDate );

			vecResult.push_back ( sDATA );
		}
		else
		{
			break;
		}

		Sleep(0);
	}	

	pDatabase->FreeConnection(pConn);

	return DB_OK;
}

int CGMToolOdbcBase::UserIPLOGRead( int nUSERNUM, SGMTOOL_DATA_USER_IPLOG_MAP &mapResult )
{
	SGMTOOL_DATA_USER_IPLOG_VEC vecLog;

	UserIPLOGRead( nUSERNUM, vecLog );

	mapResult.clear();

	for( size_t i=0; i<vecLog.size(); ++i ){
		SGMTOOL_DATA_USER_IPLOG sLog = vecLog[i];

		std::string strIP = sLog.szLogIpAddress;
		SGMTOOL_DATA_USER_IPLOG_MAP_ITER iter = mapResult.find( strIP );
		if ( iter != mapResult.end() ){
			SGMTOOL_DATA_USER_IPLOG &sLogMap = (*iter).second;
			if ( sLog.tLogDate > sLogMap.tLogDate ){
				sLogMap.tLogDate = sLog.tLogDate;
			}
		}else{
			mapResult.insert( std::make_pair( strIP, sLog ) );
		}
	}

	return DB_OK;
}

int CGMToolOdbcBase::UserIPLOGErase( int nUSERNUM )
{
	if ( nUSERNUM <= 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[512] = {0};
	_snprintf( szTemp, 512, "Delete From LogLogin WHERE UserNum = %d ", nUSERNUM );

	CGMToolOdbcConn* pDatabase = m_pUserDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	return pDatabase->ExecuteSQL(szTemp);
}

