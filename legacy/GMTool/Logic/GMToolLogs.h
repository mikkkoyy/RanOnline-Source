#ifndef _GMTOOL_LOGS_H_
#define _GMTOOL_LOGS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace GMTOOL_LOGS
{
	char* LogsPathGet();
	void LogsPathSet( const char* szPath );

	void ConsoleSet( HWND hControl );
	void ConsoleClear();

	std::string GetCurTime();
	void LogPrint( const char *szLOG );
	void LogWrite( const char *szLOG );

	void LogSql( const char *szFormat, ... );
	void LogSqlError( const char *szFormat, ... );
	void LogError( const char *szFormat, ... );
	void LogInfo( const char *szFormat, ... );
};

#endif // _GMTOOL_LOGS_H_