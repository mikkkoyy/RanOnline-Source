#include "stdafx.h"
#include "GMToolOdbcHenv.h"
#include "GMToolLogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCGMToolOdbcHenv* CCGMToolOdbcHenv::SelfInstance = NULL;

CCGMToolOdbcHenv::CCGMToolOdbcHenv(void) 
	: m_hEnv(NULL)
{
	CloseHandle();
	CreateHandle();	
}

CCGMToolOdbcHenv::~CCGMToolOdbcHenv(void)
{
	CloseHandle();
}

CCGMToolOdbcHenv* CCGMToolOdbcHenv::GetInstance()
{
	if (SelfInstance == NULL)
		SelfInstance = new CCGMToolOdbcHenv();
	return SelfInstance;
}

void CCGMToolOdbcHenv::ReleaseInstance()
{
	if (SelfInstance != NULL)
	{
		delete SelfInstance;
		SelfInstance = NULL;
	}
}

int CCGMToolOdbcHenv::CreateHandle()
{
	CloseHandle();

	SQLRETURN sReturn;

	//pooling disabled
	//sReturn = ::SQLSetEnvAttr( NULL, SQL_ATTR_CONNECTION_POOLING, (void*) SQL_CP_ONE_PER_HENV, 0 );
	sReturn = ::SQLSetEnvAttr( NULL, SQL_ATTR_CONNECTION_POOLING, (void*) SQL_CP_OFF, 0 );

	if (sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO)
	{
		GMTOOL_LOGS::LogInfo( "INFO:SQL_ATTR_CONNECTION_POOLING OK" );
	}
	else
	{
		GMTOOL_LOGS::LogInfo( "ERROR:SQL_ATTR_CONNECTION_POOLING Failed" );
		return DB_ERROR;
	}

	sReturn = ::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnv);
	if (sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO)
	{
		GMTOOL_LOGS::LogInfo( "INFO:SQLAllocHandle OK" );
	}
	else
	{
		GMTOOL_LOGS::LogInfo( "ERROR:SQLAllocHandle Failed" );
		return DB_ERROR;
	}

	sReturn = ::SQLSetEnvAttr(m_hEnv, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0);
	if (sReturn != SQL_SUCCESS && sReturn != SQL_SUCCESS_WITH_INFO)
	{		
		::SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
		m_hEnv = NULL;
		return DB_ERROR;
	}

	return DB_OK;
}

void CCGMToolOdbcHenv::CloseHandle()
{
	if (m_hEnv != NULL)
	{
		::SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
		m_hEnv = NULL;
	}
}

SQLHENV CCGMToolOdbcHenv::GetHandle()
{
	return m_hEnv;
}