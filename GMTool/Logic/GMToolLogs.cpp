
#include "stdafx.h"
#include "GMToolLogs.h"
#include "GMToolConfig.h"

namespace GMTOOL_LOGS
{
	static char g_szLogPath[MAX_PATH] = "";
	HWND	m_pConsole = NULL;

	char* LogsPathGet (){
		return g_szLogPath;
	}

	void LogsPathSet ( const char* szPath ){
		if ( !szPath )		
			szPath = "";

		StringCchCopy( g_szLogPath, MAX_PATH, szPath );
		StringCchCat( g_szLogPath, MAX_PATH, "\\GMTool" );
		CreateDirectory ( g_szLogPath, NULL );
	}

	void ConsoleSet( HWND hControl  ){
		m_pConsole = hControl;
		
		if ( m_pConsole )
			SendMessage( m_pConsole, EM_LIMITTEXT, (WPARAM) (64 * 2048), (LPARAM) 0 );
	}

	void ConsoleClear(){
		if ( m_pConsole ){
			DWORD dwRetCode = 0;
			TCHAR TempBuffer[3] = {0};
			::StringCchCopy( TempBuffer, 3, "\0" );	
			SendMessage( m_pConsole, WM_SETFOCUS, NULL, 0 );
			SendMessage( m_pConsole, WM_SETTEXT, 0, (LPARAM) TempBuffer );
		}
	}

	std::string GetCurTime (){
		CTime Time = CTime::GetCurrentTime();

		CString strTime;
		strTime.Format ( "%d-%02d-%02d %02d:%02d:%02d", Time.GetYear(), Time.GetMonth(), Time.GetDay(), Time.GetHour(), Time.GetMinute(), Time.GetSecond() );
		return strTime.GetString();
	}

	void LogPrint( const char *szLOG ){
		if ( !szLOG ) return;
		if ( !m_pConsole ) return;

		//add timestamp
		std::string strTIME = GetCurTime ();
		CString strTEXT;
		strTEXT.Format( "[%s]~~%s", strTIME.c_str(), szLOG );

		DWORD dwRetCode = 0;
		
		TCHAR TempBuffer[2048+3] = {0};
		::StringCchCopy( TempBuffer, 2048, strTEXT.GetString() );	
		::StringCchCat ( TempBuffer, 2048+3, _T("\r\n") );	

		::SendMessageCallback( m_pConsole, EM_SETSEL,      (WPARAM) UINT_MAX-1,  (LPARAM) UINT_MAX, NULL, (ULONG_PTR) &dwRetCode );
		::SendMessageCallback( m_pConsole, EM_SCROLL,      (WPARAM) SB_PAGEDOWN, (LPARAM) 0,	    NULL, (ULONG_PTR) &dwRetCode );	
		::SendMessageCallback( m_pConsole, EM_SCROLLCARET, (WPARAM) 0,			 (LPARAM) 0,		NULL, (ULONG_PTR) &dwRetCode );	
		::SendMessageTimeout(  m_pConsole, EM_REPLACESEL,  (WPARAM) FALSE,		 (LPARAM) TempBuffer, SMTO_BLOCK, 15, &dwRetCode );
		::SendMessageCallback( m_pConsole, EM_LINESCROLL,  (WPARAM) 0,			 (LPARAM) 1,		NULL, (ULONG_PTR) &dwRetCode );
		::SendMessageCallback( m_pConsole, EM_SETSEL,	   (WPARAM) -1,		     (LPARAM) UINT_MAX, NULL, (ULONG_PTR) &dwRetCode );
	}

	void LogWrite( const char *szLOG ){
		CTime cCurTime = CTime::GetCurrentTime();
		CString strFileName;
		strFileName.Format( "log_%04d_%02d_%02d.txt", cCurTime.GetYear(), cCurTime.GetMonth(), cCurTime.GetDay() );

		CString strLogName;
		strLogName.Format ( "%s\\%s", g_szLogPath, strFileName );

		FILE *fp;
		fp = fopen ( strLogName, "at" );
		if ( fp ){
			//add timestamp
			std::string strTIME = GetCurTime ();
			fprintf ( fp, "[%s]~~%s\n", strTIME.c_str(), szLOG );
			fclose(fp);
		}
	}

	//sql query
	void LogSql( const char *szFormat, ... ){
		va_list argList;
		char szBuffer[2048] = "";

		va_start(argList, szFormat);
		StringCbVPrintf(szBuffer, 2048, szFormat, argList);
		va_end(argList);

		if( GMTOOL_CONFIG::m_bLogSQLCODE )
			LogWrite( szBuffer );

		if( GMTOOL_CONFIG::m_bPrintSQLCODE )
			LogPrint( szBuffer );
	}

	//sql error
	void LogSqlError( const char *szFormat, ... ){
		va_list argList;
		char szBuffer[2048] = "";

		va_start(argList, szFormat);
		StringCbVPrintf(szBuffer, 2048, szFormat, argList);
		va_end(argList);

		if( GMTOOL_CONFIG::m_bLogSQLERROR )
			LogWrite( szBuffer );

		if( GMTOOL_CONFIG::m_bPrintSQLERROR )
			LogPrint( szBuffer );
	}

	//etch error
	void LogError( const char *szFormat, ... ){
		va_list argList;
		char szBuffer[2048] = "";

		va_start(argList, szFormat);
		StringCbVPrintf(szBuffer, 2048, szFormat, argList);
		va_end(argList);

		if( GMTOOL_CONFIG::m_bLogDATAERROR )
			LogWrite( szBuffer );

		if( GMTOOL_CONFIG::m_bPrintDATAERROR )
			LogPrint( szBuffer );
	}

	//print-log info
	void LogInfo( const char *szFormat, ... ){
		va_list argList;
		char szBuffer[2048] = "";

		va_start(argList, szFormat);
		StringCbVPrintf(szBuffer, 2048, szFormat, argList);
		va_end(argList);

		LogWrite( szBuffer );
		LogPrint( szBuffer );
	}
};