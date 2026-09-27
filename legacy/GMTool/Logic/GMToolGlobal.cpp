
#include "stdafx.h"
#include "GMToolGlobal.h"
#include "china_md5.h"
#include "StringUtils.h"

USERLEVEL_INDEX UserTypeToIndex( EMUSERTYPE emTYPE )
{
	if ( emTYPE >= USER_MASTER ){
		return USERLEVEL_INDEX_ADMINISTRATOR;
	}else if ( emTYPE >= USER_GM3 && emTYPE < USER_MASTER ){
		return USERLEVEL_INDEX_GAMEMASTER;
	}else /*if ( emTYPE >= USER_COMMON && emTYPE < USER_GM3 )*/{
		return USERLEVEL_INDEX_NORMAL;
	}
};

EMUSERTYPE UserIndexToType( USERLEVEL_INDEX emINDEX )
{
	if ( emINDEX == USERLEVEL_INDEX_ADMINISTRATOR ){
		return USER_MASTER;
	}else if ( emINDEX == USERLEVEL_INDEX_GAMEMASTER ){
		return USER_GM3;
	}else {
		return USER_COMMON;	
	}
};

DWORD GetSearchLimitNum( EMSEARCH_LIMIT emLIMIT )
{
	switch( emLIMIT )
	{
	case EMSEARCH_LIMIT_10:
		return 10;
		break;

	case EMSEARCH_LIMIT_50:
		return 50;
		break;

	case EMSEARCH_LIMIT_100:
		return 100;
		break;

	case EMSEARCH_LIMIT_500:
		return 500;
		break;

	case EMSEARCH_LIMIT_999:
		return 999;
		break;
	};

	return 0; //EMSEARCH_LIMIT_NONE
};

namespace GMTOOL_GLOBAL
{
	std::string GMToolStringConvertToMD5( const TCHAR* szText, int nMAXLEN )
	{
		if ( nMAXLEN <= 0 ){
			return "";
		}

		CHINA_MD5::MD5 md5; 
		size_t nLength = 0;
		StringCchLength( szText, nMAXLEN, &nLength );
		md5.update( reinterpret_cast<const unsigned char*> (szText), nLength );	

		std::string strTemp = md5.asString();
		strTemp.resize( nMAXLEN-1 );
		return strTemp;
	};

	BOOL IsBadString( std::string strText )
	{
		CString strCheck = strText.c_str();
		if( strCheck.FindOneOf("~!@#$%^&*+|:?><,.;{}()\n\t\v\b\r\a\\\?\'\" ") != -1 )
		{
			return TRUE;
		}

		return FALSE;
	};

	BOOL IsBadString2( std::string strText )
	{
		CString strCheck = strText.c_str();
		if( strCheck.FindOneOf("~!#$%^&*+|:?><,;{}()\n\t\v\b\r\a\\\?\'\" ") != -1 ){
			return TRUE;
		} 

		return FALSE;
	};

	BOOL IsBadString3( std::string strText )
	{
		CString strCheck = strText.c_str();
		if( strCheck.FindOneOf("~!#$%^&*+|:?><,;{}()\n\t\v\b\r\a\\\?\'\"") != -1 ){
			return TRUE;
		} 

		return FALSE;
	};

	BOOL IsBadNumber( std::string strText )
	{
		if ( std::string::npos != strText.find_first_not_of("0123456789")){
			return TRUE;
		}

		return FALSE;
	};
};


