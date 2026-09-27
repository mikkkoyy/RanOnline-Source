#include "stdafx.h"
#include "HLibDataConvert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace _HLIB
{
	CString cstring_dword( const DWORD _dwValue )
	{
		CString strText(_T(""));
		strText.Format(_T("%d"), _dwValue );
		return strText;
	};

	CString cstring_int( const int _nValue )
	{
		CString strText(_T(""));
		strText.Format(_T("%d"), _nValue );
		return strText;
	};

	CString cstring_uint( const unsigned int _nValue )
	{
		CString strText(_T(""));
		strText.Format(_T("%u"), _nValue );
		return strText;
	};

	CString cstring_floatf( const float _fValue )
	{
		CString strText(_T(""));
		strText.Format(_T("%f"), _fValue );
		return strText;
	};

	CString cstring_floatg( const float _fValue )
	{
		CString strText(_T(""));
		strText.Format(_T("%g"), _fValue );
		return strText;
	};

	CString cstring_longlong( const LONGLONG _llValue )
	{
		CString strText(_T(""));
		strText.Format(_T("%I64d"), _llValue );
		return strText;
	};

	CString cstring_ulonglong( const ULONGLONG _llValue )
	{
		CString strText(_T(""));
		strText.Format(_T("%I64u"), _llValue );
		return strText;
	};

	CString cstring_timet12( const __time64_t _tValue )
	{
		CTime cTime ( _tValue );
		return cTime.Format( _T("%Y-%m-%d %I:%M:%S:%p") );
	};

	CString cstring_timet24( const __time64_t _tValue )
	{
		CTime cTime ( _tValue );
		return cTime.Format( _T("%Y-%m-%d %H:%M:%S") );
	};

	CString cstring_timestamp12( const TIMESTAMP_STRUCT _tsValue )
	{
		CTime cTime( _tsValue.year, _tsValue.month, _tsValue.day, 
			_tsValue.hour, _tsValue.minute, _tsValue.second );
		return cTime.Format( _T("%Y-%m-%d %I:%M:%S:%p") );
	};

	CString cstring_timestamp24( const TIMESTAMP_STRUCT _tsValue )
	{
		CTime cTime( _tsValue.year, _tsValue.month, _tsValue.day, 
			_tsValue.hour, _tsValue.minute, _tsValue.second );
		return cTime.Format( _T("%Y-%m-%d %H:%M:%S") );
	};

	CString cstring_ctime12( const CTime cTime )
	{
		return cTime.Format( _T("%Y-%m-%d %I:%M:%S:%p") );
	};

	CString cstring_ctime24( const CTime cTime )
	{
		return cTime.Format( _T("%Y-%m-%d %H:%M:%S") );
	};

	int int_cstring( const CString _strNum )
	{
#ifdef UNICODE
		int nVal = _wtoi( _strNum.GetString() );
		return nVal;
#else
		int nVal = atoi( _strNum.GetString() );
		return nVal;;
#endif 	
	};

	unsigned int uint_cstring( const CString _strNum )
	{
#ifdef UNICODE
		unsigned int nVal = _wtol( _strNum.GetString() );
		return nVal;
#else
		unsigned int nVal = atol( _strNum.GetString() );
		return nVal;;
#endif 
	};

	float float_cstring( const CString _strNum )
	{
#ifdef UNICODE
		float fVal = (float)_wtof( _strNum.GetString() );
		return fVal;
#else
		float fVal = (float)atof( _strNum.GetString() );
		return fVal;
#endif 
	};

	LONGLONG longlong_cstring( const CString _strNum )
	{
		TCHAR szBuffer[32];
		_tcscpy(szBuffer, _strNum );

#ifdef UNICODE
		LONGLONG llVal = _wcstoi64( szBuffer, NULL, 10 );
		return llVal;
#else
		LONGLONG llVal = _strtoi64( szBuffer, NULL, 10 );
		return llVal;
#endif 	
	};

	ULONGLONG ulonglong_cstring( const CString _strNum )
	{
		TCHAR szBuffer[32];
		_tcscpy (szBuffer, _strNum );

#ifdef UNICODE
		ULONGLONG llVal = _wcstoui64( szBuffer, NULL, 10 );
		return llVal;
#else
		ULONGLONG llVal = _strtoui64( szBuffer, NULL, 10 );
		return llVal;
#endif 	
	};

	//c++ 11 only
#ifdef UNICODE
	CString cstringformat( const wchar_t * _szFormat, ... )
	{
		wchar_t szbuffer[2048];
		va_list argList;
		va_start( argList, _szFormat );
		vswprintf( szbuffer, 2048, _szFormat, argList );
		va_end( argList );

		return CString( szbuffer, 2048 );
	};
#else
	CString cstringformat( const char *_szFormat, ... )
	{
		char szbuffer[2048];
		va_list argList;
		va_start( argList, _szFormat);
		_vsnprintf( szbuffer, 2048, _szFormat, argList );
		va_end(argList);

		return CString( szbuffer, 2048 );
	};
#endif



};