
#include "stdafx.h"
#include "GMToolConfig.h"
#include "IniLoader.h"
#include "Rijndael.h"
#include <shlwapi.h>
#pragma comment( lib, "shlwapi.lib")

namespace GMTOOL_CONFIG
{
	static char g_szPath[MAX_PATH] = "";
	
	TCHAR	m_szUserDBServer[DB_SVR_NAME_LENGTH] = _T("localhost");
	TCHAR	m_szUserDBDatabase[DB_SVR_NAME_LENGTH] = _T("RanUser");
	TCHAR	m_szUserDBUser[USR_ID_LENGTH] = _T("sa");
	TCHAR	m_szUserDBPass[GMC_DB_PASS_LENGTH] = _T("1234");
	int		m_nUserDBPort = 1433;

	TCHAR	m_szGameDBServer[DB_SVR_NAME_LENGTH] = _T("localhost");
	TCHAR	m_szGameDBDatabase[DB_SVR_NAME_LENGTH] = _T("RanGame1");
	TCHAR	m_szGameDBUser[USR_ID_LENGTH] = _T("sa");
	TCHAR	m_szGameDBPass[GMC_DB_PASS_LENGTH] = _T("1234");
	int		m_nGameDBPort = 1433;

	TCHAR	m_szLogDBServer[DB_SVR_NAME_LENGTH] = _T("localhost");
	TCHAR	m_szLogDBDatabase[DB_SVR_NAME_LENGTH] = _T("RanLog");
	TCHAR	m_szLogDBUser[USR_ID_LENGTH] = _T("sa");
	TCHAR	m_szLogDBPass[GMC_DB_PASS_LENGTH] = _T("1234");
	int		m_nLogDBPort = 1433;

	TCHAR	m_szShopDBServer[DB_SVR_NAME_LENGTH] = _T("localhost");
	TCHAR	m_szShopDBDatabase[DB_SVR_NAME_LENGTH] = _T("RanShop");
	TCHAR	m_szShopDBUser[USR_ID_LENGTH] = _T("sa");
	TCHAR	m_szShopDBPass[GMC_DB_PASS_LENGTH] = _T("1234");
	int		m_nShopDBPort = 1433;

	BOOL	m_bLogSQLCODE = FALSE;
	BOOL	m_bLogSQLERROR = TRUE;
	BOOL	m_bLogDATAERROR = TRUE;

	BOOL	m_bPrintSQLCODE = FALSE;
	BOOL	m_bPrintSQLERROR = TRUE;
	BOOL	m_bPrintDATAERROR = TRUE;

	TCHAR	m_szLoginUser[USR_ID_LENGTH] = _T("");
	WORD	m_wLoginType = 0;
	DWORD	m_dwLoginID = 0;
	
	char* ConfigPathGet (){
		return g_szPath;
	}

	void ConfigPathSet ( const char* szPath ){
		if ( !szPath )		
			szPath = "";

		StringCchCopy( g_szPath, MAX_PATH, szPath );
	}

	BOOL SettingsLoad(){
		std::string strFILE(g_szPath);
		strFILE += "\\GMToolConfig.ini";

		if ( !PathFileExists( strFILE.c_str() ) )
			return FALSE;

		CIniLoader cFILE;

		if( !cFILE.open( strFILE, true) )
			return FALSE;

		cFILE.SetUseErrorMsgBox( FALSE );

		cFILE.getflag_sz( "ODBC", "user_db_server", 0, 1, m_szUserDBServer, DB_SVR_NAME_LENGTH );
		cFILE.getflag_sz( "ODBC", "user_db_database", 0, 1, m_szUserDBDatabase, DB_SVR_NAME_LENGTH );
		cFILE.getflag_sz( "ODBC", "user_db_user", 0, 1, m_szUserDBUser, USR_ID_LENGTH );
		cFILE.getflag_sz( "ODBC", "user_db_pass", 0, 1, m_szUserDBPass, GMC_DB_PASS_LENGTH );
		cFILE.getflag ( "ODBC", "user_db_port", 0, 1, m_nUserDBPort );

		cFILE.getflag_sz( "ODBC", "game_db_server", 0, 1, m_szGameDBServer, DB_SVR_NAME_LENGTH );
		cFILE.getflag_sz( "ODBC", "game_db_database", 0, 1, m_szGameDBDatabase, DB_SVR_NAME_LENGTH );
		cFILE.getflag_sz( "ODBC", "game_db_user", 0, 1, m_szGameDBUser, USR_ID_LENGTH );
		cFILE.getflag_sz( "ODBC", "game_db_pass", 0, 1, m_szGameDBPass, GMC_DB_PASS_LENGTH );
		cFILE.getflag ( "ODBC", "game_db_port", 0, 1, m_nGameDBPort );

		cFILE.getflag_sz( "ODBC", "log_db_server", 0, 1, m_szLogDBServer, DB_SVR_NAME_LENGTH );
		cFILE.getflag_sz( "ODBC", "log_db_database", 0, 1, m_szLogDBDatabase, DB_SVR_NAME_LENGTH );
		cFILE.getflag_sz( "ODBC", "log_db_user", 0, 1, m_szLogDBUser, USR_ID_LENGTH );
		cFILE.getflag_sz( "ODBC", "log_db_pass", 0, 1, m_szLogDBPass, GMC_DB_PASS_LENGTH );
		cFILE.getflag ( "ODBC", "log_db_port", 0, 1, m_nLogDBPort );

		cFILE.getflag_sz( "ODBC", "shop_db_server", 0, 1, m_szShopDBServer, DB_SVR_NAME_LENGTH );
		cFILE.getflag_sz( "ODBC", "shop_db_database", 0, 1, m_szShopDBDatabase, DB_SVR_NAME_LENGTH );
		cFILE.getflag_sz( "ODBC", "shop_db_user", 0, 1, m_szShopDBUser, USR_ID_LENGTH );
		cFILE.getflag_sz( "ODBC", "shop_db_pass", 0, 1, m_szShopDBPass, GMC_DB_PASS_LENGTH );
		cFILE.getflag ( "ODBC", "shop_db_port", 0, 1, m_nShopDBPort );

		cFILE.getflag ( "LOG", "log_sql", 0, 1, m_bLogSQLCODE );
		cFILE.getflag ( "LOG", "log_sqlerror", 0, 1, m_bLogSQLERROR );
		cFILE.getflag ( "LOG", "log_error", 0, 1, m_bLogDATAERROR );

		cFILE.getflag ( "PRINT", "print_sql", 0, 1, m_bPrintSQLCODE );
		cFILE.getflag ( "PRINT", "print_sqlerror", 0, 1, m_bPrintSQLERROR );
		cFILE.getflag ( "PRINT", "print_error", 0, 1, m_bPrintDATAERROR );

		return TRUE;
	}

	BOOL SettingsSave(){
		CString strBuffer;

		strBuffer += _T(";GMTool Settings File\n");
		strBuffer += _T(";\n");
		strBuffer += _T("[ODBC]\n");
		strBuffer.AppendFormat( "user_db_server	= %s\n", m_szUserDBServer );
		strBuffer.AppendFormat( "user_db_database	= %s\n", m_szUserDBDatabase );
		strBuffer.AppendFormat( "user_db_user	= %s\n", m_szUserDBUser );
		strBuffer.AppendFormat( "user_db_pass = %s\n", m_szUserDBPass );
		strBuffer.AppendFormat( "user_db_port = %d\n", m_nUserDBPort );

		strBuffer.AppendFormat( "game_db_server	= %s\n", m_szGameDBServer );
		strBuffer.AppendFormat( "game_db_database	= %s\n", m_szGameDBDatabase );
		strBuffer.AppendFormat( "game_db_user	= %s\n", m_szGameDBUser );
		strBuffer.AppendFormat( "game_db_pass = %s\n", m_szGameDBPass );
		strBuffer.AppendFormat( "game_db_port = %d\n", m_nGameDBPort );

		strBuffer.AppendFormat( "log_db_server	= %s\n", m_szLogDBServer );
		strBuffer.AppendFormat( "log_db_database	= %s\n", m_szLogDBDatabase );
		strBuffer.AppendFormat( "log_db_user	= %s\n", m_szLogDBUser );
		strBuffer.AppendFormat( "log_db_pass = %s\n", m_szLogDBPass );
		strBuffer.AppendFormat( "log_db_port = %d\n", m_nLogDBPort );

		strBuffer.AppendFormat( "shop_db_server	= %s\n", m_szShopDBServer );
		strBuffer.AppendFormat( "shop_db_database	= %s\n", m_szShopDBDatabase );
		strBuffer.AppendFormat( "shop_db_user	= %s\n", m_szShopDBUser );
		strBuffer.AppendFormat( "shop_db_pass = %s\n", m_szShopDBPass );
		strBuffer.AppendFormat( "shop_db_port = %d\n", m_nShopDBPort );
		
		strBuffer += _T(";\n");
		strBuffer += _T("[LOG]\n");
		strBuffer.AppendFormat( "log_sql = %d\n", m_bLogSQLCODE );
		strBuffer.AppendFormat( "log_sqlerror = %d\n", m_bLogSQLERROR );
		strBuffer.AppendFormat( "log_error = %d\n", m_bLogDATAERROR );

		strBuffer += _T(";\n");
		strBuffer += _T("[PRINT]\n");
		strBuffer.AppendFormat( "print_sql = %d\n", m_bPrintSQLCODE );
		strBuffer.AppendFormat( "print_sqlerror = %d\n", m_bPrintSQLERROR );
		strBuffer.AppendFormat( "print_error = %d\n", m_bPrintDATAERROR );

		CRijndael oRijndael;
		oRijndael.Initialize( CRijndael::VERSION, CRijndael::sm_Version[CRijndael::VERSION-1].c_str(), CRijndael::sm_KeyLength[CRijndael::VERSION-1] );

		int nLen = oRijndael.GetEncryptStringLength( strBuffer );
		char * szBuffer = new char[nLen];
		memset( szBuffer, 0, nLen );

		oRijndael.EncryptEx( strBuffer.GetString(), szBuffer, strBuffer.GetLength() );
		int nVersion = CRijndael::VERSION;

		if ( nVersion <= 0 && strlen( szBuffer ) <= 0 )	return FALSE;

		std::string strFILE(g_szPath);
		strFILE += "\\GMToolConfig.ini";

		CFile file;
		if ( ! file.Open( strFILE.c_str(), CFile::modeCreate|CFile::modeWrite ) ){
			delete [] szBuffer;
			return FALSE;
		}

		file.Write( &nVersion, sizeof(int) );
		file.Write( szBuffer, nLen );
		file.Close();

		delete [] szBuffer;

		return TRUE;
	}

	BOOL SetLoginInfo( const char* szLoginUser, WORD wLoginType, DWORD dwLoginID ){

		if ( wLoginType < USER_GM3 ){
			CDebugSet::MsgBoxAfx( "Login Info Error!, Invalid Login Type");
			return FALSE;
		}

		if ( dwLoginID <= 0 ){
			CDebugSet::MsgBoxAfx( "Login Info Error!, Invalid Login ID");
			return FALSE;
		}

		StringCchCopy( m_szLoginUser, USR_ID_LENGTH, szLoginUser );
		m_wLoginType = wLoginType;
		m_dwLoginID = dwLoginID;

		return TRUE;
	}
};