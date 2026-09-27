#ifndef _GMTOOL_CONFIG_H_
#define _GMTOOL_CONFIG_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "s_NetGlobal.h"

#define GMC_DB_PASS_LENGTH	34

namespace GMTOOL_CONFIG
{
	//database user
	extern TCHAR	m_szUserDBServer  [DB_SVR_NAME_LENGTH];
	extern TCHAR	m_szUserDBDatabase  [DB_SVR_NAME_LENGTH];
	extern TCHAR	m_szUserDBUser    [USR_ID_LENGTH];
	extern TCHAR	m_szUserDBPass    [GMC_DB_PASS_LENGTH];
	extern int		m_nUserDBPort;

	//database game
	extern TCHAR	m_szGameDBServer  [DB_SVR_NAME_LENGTH];
	extern TCHAR	m_szGameDBDatabase  [DB_SVR_NAME_LENGTH];
	extern TCHAR	m_szGameDBUser    [USR_ID_LENGTH];
	extern TCHAR	m_szGameDBPass    [GMC_DB_PASS_LENGTH];
	extern int		m_nGameDBPort;

	//database log
	extern TCHAR	m_szLogDBServer  [DB_SVR_NAME_LENGTH];
	extern TCHAR	m_szLogDBDatabase  [DB_SVR_NAME_LENGTH];
	extern TCHAR	m_szLogDBUser    [USR_ID_LENGTH];
	extern TCHAR	m_szLogDBPass    [GMC_DB_PASS_LENGTH];
	extern int		m_nLogDBPort;

	//database shop
	extern TCHAR	m_szShopDBServer  [DB_SVR_NAME_LENGTH];
	extern TCHAR	m_szShopDBDatabase  [DB_SVR_NAME_LENGTH];
	extern TCHAR	m_szShopDBUser    [USR_ID_LENGTH];
	extern TCHAR	m_szShopDBPass    [GMC_DB_PASS_LENGTH];
	extern int		m_nShopDBPort;

	//text log setting
	extern BOOL		m_bLogSQLCODE;
	extern BOOL		m_bLogSQLERROR;
	extern BOOL		m_bLogDATAERROR;

	//console window setting
	extern BOOL		m_bPrintSQLCODE;
	extern BOOL		m_bPrintSQLERROR;
	extern BOOL		m_bPrintDATAERROR;

	//info saved in login
	extern TCHAR	m_szLoginUser[USR_ID_LENGTH];
	extern WORD		m_wLoginType;
	extern DWORD	m_dwLoginID;

	char* ConfigPathGet();
	void ConfigPathSet( const char* szPath );

	BOOL SetLoginInfo( const char* szLoginUser, WORD wLoginType, DWORD dwLoginID );

	BOOL SettingsLoad();
	BOOL SettingsSave();	
};

#endif // _GMTOOL_CONFIG_H_