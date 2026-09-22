/*!
 * \file HLibDataConvert.h
 *
 * \author Juver
 * \date March 2017
 *
 * 
 */

#pragma once

#include <string>
#include "tchar.h"
#include "afxstr.h"
#include "sqltypes.h"

namespace _HLIB
{
	CString cstring_dword( const DWORD _dwValue );
	CString cstring_int( const int _nValue );
	CString cstring_uint( const unsigned int _nValue );
	CString cstring_floatf( const float _fValue );
	CString cstring_floatg( const float _fValue );
	CString cstring_longlong( const LONGLONG _llValue );
	CString cstring_ulonglong( const ULONGLONG _llValue );
	CString cstring_timet12( const __time64_t _tValue );
	CString cstring_timet24( const __time64_t _tValue );
	CString cstring_timestamp12( const TIMESTAMP_STRUCT _tsValue );
	CString cstring_timestamp24( const TIMESTAMP_STRUCT _tsValue );
	CString cstring_ctime12( const CTime cTime );
	CString cstring_ctime24( const CTime cTime );

	int		int_cstring( const CString _strNum );
	unsigned int uint_cstring( const CString _strNum );
	float	float_cstring( const CString _strNum );
	LONGLONG longlong_cstring( const CString _strNum );
	ULONGLONG ulonglong_cstring( const CString _strNum );

#ifdef UNICODE
	CString cstringformat( const wchar_t * _szFormat, ... );
#else
	CString cstringformat( const char *_szFormat, ... );
#endif

	
};