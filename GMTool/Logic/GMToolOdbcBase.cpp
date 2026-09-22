#include "stdAfx.h"
#include "GMToolOdbcBase.h"

#include "GMToolOdbcHenv.h"
#include "GMToolOdbcConn.h"
#include "GMToolConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CGMToolOdbcBase* CGMToolOdbcBase::SelfInstance = NULL;

CGMToolOdbcBase::CGMToolOdbcBase(void) 
	: m_pGameDB(NULL)
	, m_pUserDB(NULL)
	, m_pLogDB(NULL)
	, m_pShopDB(NULL)
{
	CCGMToolOdbcHenv::GetInstance();
}

CGMToolOdbcBase::~CGMToolOdbcBase(void)
{
	SAFE_DELETE(m_pGameDB);
	SAFE_DELETE(m_pUserDB);
	SAFE_DELETE(m_pLogDB);
	SAFE_DELETE(m_pShopDB);
	CCGMToolOdbcHenv::GetInstance()->ReleaseInstance();
}

CGMToolOdbcBase* CGMToolOdbcBase::GetInstance()
{
	if (SelfInstance == NULL)
		SelfInstance = new CGMToolOdbcBase();
	return SelfInstance;
}

void CGMToolOdbcBase::ReleaseInstance()
{
	if (SelfInstance != NULL)
	{
		delete SelfInstance;
		SelfInstance = NULL;
	}
}

BOOL CGMToolOdbcBase::DatabaseOpen()
{
	//open user DB
	int nRetCode = OpenUserDB();
	if (nRetCode == DB_ERROR){
		CDebugSet::MsgBoxAfx( "Error Open User Database" );
		return FALSE;
	}

	//open game db
	nRetCode = OpenGameDB();
	if (nRetCode == DB_ERROR){
		CDebugSet::MsgBoxAfx( "Error Open Game Database" );
		return FALSE;
	}

	//open log db
	nRetCode = OpenLogDB();
	if (nRetCode == DB_ERROR){
		CDebugSet::MsgBoxAfx( "Error Open Log Database" );
		return FALSE;
	}

	//open shop db
	nRetCode = OpenShopDB();
	if (nRetCode == DB_ERROR){
		CDebugSet::MsgBoxAfx( "Error Open Shop Database" );
		return FALSE;
	}
	
	return TRUE;
}

BOOL CGMToolOdbcBase::DatabaseClose()
{
	//close user DB
	CloseUserDB();

	//close game db
	CloseGameDB();

	//close log db
	CloseLogDB();

	//close shop db
	CloseShopDB();

	return TRUE;
}

int	CGMToolOdbcBase::OpenGameDB()
{
	SAFE_DELETE(m_pGameDB);
    m_pGameDB = new CGMToolOdbcConn( 
		GMTOOL_CONFIG::m_szGameDBServer, 
		GMTOOL_CONFIG::m_szGameDBDatabase,
		GMTOOL_CONFIG::m_szGameDBUser, 
		GMTOOL_CONFIG::m_szGameDBPass,
		GMTOOL_CONFIG::m_nGameDBPort );
	return m_pGameDB->OpenDB();
}

int	CGMToolOdbcBase::OpenUserDB()
{
	SAFE_DELETE(m_pUserDB);
	m_pUserDB = new CGMToolOdbcConn( 
		GMTOOL_CONFIG::m_szUserDBServer, 
		GMTOOL_CONFIG::m_szUserDBDatabase, 
		GMTOOL_CONFIG::m_szUserDBUser, 
		GMTOOL_CONFIG::m_szUserDBPass,
		GMTOOL_CONFIG::m_nUserDBPort );
	return m_pUserDB->OpenDB();
}

int CGMToolOdbcBase::OpenLogDB()
{
	SAFE_DELETE(m_pLogDB);
	m_pLogDB = new CGMToolOdbcConn( 
		GMTOOL_CONFIG::m_szLogDBServer, 
		GMTOOL_CONFIG::m_szLogDBDatabase, 
		GMTOOL_CONFIG::m_szLogDBUser, 
		GMTOOL_CONFIG::m_szLogDBPass,
		GMTOOL_CONFIG::m_nLogDBPort );
	return m_pLogDB->OpenDB();
}

int CGMToolOdbcBase::OpenShopDB()
{
	SAFE_DELETE(m_pShopDB);
	m_pShopDB = new CGMToolOdbcConn( 
		GMTOOL_CONFIG::m_szShopDBServer, 
		GMTOOL_CONFIG::m_szShopDBDatabase, 
		GMTOOL_CONFIG::m_szShopDBUser, 
		GMTOOL_CONFIG::m_szShopDBPass,
		GMTOOL_CONFIG::m_nShopDBPort);
	return m_pShopDB->OpenDB();
}

void CGMToolOdbcBase::CloseGameDB()
{
	SAFE_DELETE(m_pGameDB);
}

void CGMToolOdbcBase::CloseUserDB()
{
	SAFE_DELETE(m_pUserDB);
}

void CGMToolOdbcBase::CloseLogDB()
{
	SAFE_DELETE(m_pLogDB);
}

void CGMToolOdbcBase::CloseShopDB()
{
	SAFE_DELETE(m_pShopDB);
}

CString	CGMToolOdbcBase::GetErrorString(SQLHSTMT hStmt)
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
			strTemp.Format(_T("%s, NativeError:%d, %s"), (LPCTSTR) SqlState, nError,(LPCTSTR) Msg);
	}

	return strTemp;
}


