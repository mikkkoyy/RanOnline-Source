#include "stdAfx.h"
#include "GMToolOdbcBase.h"
#include "GMToolOdbcConn.h"
#include "GMToolLogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CGMToolOdbcBase::GMLogin( std::string strUSER, std::string strPASS, SGMTOOL_USER_GM* pDATA )
{
	if ( strUSER.size() <= 0 ){
		return DB_ERROR;
	}

	if ( strPASS.size() <= 0 ){
		return DB_ERROR;
	}

	if ( !pDATA ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "SELECT UserNum, UserID, UserType FROM UserInfo WITH (NOLOCK) WHERE UserID = '%s' AND UserPass = '%s' ", strUSER.c_str(), strPASS.c_str() );

	CGMToolOdbcConn* pDatabase = m_pUserDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

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
	SQLINTEGER nUserType = 0, cbUserType=SQL_NTS;

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
			::SQLGetData(pConn->hStmt,  3, SQL_C_LONG,	&nUserType,          0, &cbUserType);
		
			if (cbUserID != 0 && cbUserID != -1)
				::StringCchCopy(pDATA->szUserID, USR_ID_LENGTH+1, (const TCHAR*) szUserID);

			pDATA->dwUserNum = (DWORD)nUserNum;
			pDATA->wUserType = (WORD)nUserType;
			pDATA->bValid = TRUE;
			
		}else{
			break;
		}

		Sleep(0);
	}

	pDatabase->FreeConnection(pConn);

	return DB_OK;
}