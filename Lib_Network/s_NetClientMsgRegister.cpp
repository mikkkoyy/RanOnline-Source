///////////////////////////////////////////////////////////////////////////////
// s_NetClientMsgRegister.cpp
//
// class CNetClient
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "s_NetClient.h"
#include "s_CClientConsoleMsg.h"
#include <stdlib.h>
#include "GLContrlMsg.h"
#include "china_md5.h"
#include <string.h>
#include <wchar.h>

#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CNetClient::RegisterInit( bool bInit, int nChannel )
{
	if (nChannel < 0 || nChannel >= MAX_CHANNEL_NUMBER) nChannel = 0;

	NET_REGISTER_INIT nmsg;

	nmsg.nChannel  = nChannel;
	nmsg.bInit = bInit;

	return Send((char *) &nmsg);
}

int CNetClient::RegisterAction( const TCHAR* szUserID, const TCHAR* szPass, const TCHAR* szPass2, const TCHAR* szMail, const TCHAR* szSA, int nChannel/*=0*/ )
{
	if ( !RANPARAM::bFeatureRegister )	return 0;

	if (nChannel < 0 || nChannel >= MAX_CHANNEL_NUMBER) nChannel = 0;

	NET_REGISTER_ACTION nmsg;

	nmsg.nChannel  = nChannel;

	if ( RANPARAM::bFeatureRegisterUseMD5 )
	{
		CHINA_MD5::MD5 md5;
		size_t nLength = 0;
		HRESULT sRet = StringCchLength( szPass, USR_PASS_LENGTH, &nLength );

		if (sRet == STRSAFE_E_INVALID_PARAMETER)
			return NET_ERROR;

		md5.update( reinterpret_cast<const unsigned char*> (szPass), nLength );	
		std::string strPass = md5.asString();
		StringCchCopy( nmsg.szPass,   USR_PASS_LENGTH, strPass.c_str() );
	}
	else
	{
		StringCchCopy( nmsg.szPass, USR_PASS_LENGTH, szPass );
	}

	if ( RANPARAM::bFeatureRegisterUseMD5 )
	{
		CHINA_MD5::MD5 md5;
		size_t nLength = 0;
		HRESULT sRet = StringCchLength( szPass2, USR_PASS_LENGTH, &nLength );

		if (sRet == STRSAFE_E_INVALID_PARAMETER)
			return NET_ERROR;

		md5.update( reinterpret_cast<const unsigned char*> (szPass2), nLength );	
		std::string strPass = md5.asString();
		StringCchCopy( nmsg.szPass2,   USR_PASS_LENGTH, strPass.c_str() );
	}
	else
	{
		StringCchCopy( nmsg.szPass2, USR_PASS_LENGTH, szPass2 );
	}
	

	StringCchCopy( nmsg.szUser, USR_ID_LENGTH, szUserID ); 
	StringCchCopy( nmsg.szMail, USR_INFOMAIL_LENGTH, szMail ); 
	StringCchCopy( nmsg.szSA, USR_PASS_LENGTH, szSA ); 
	
	m_Tea.encrypt( nmsg.szUser, USR_ID_LENGTH ); 
	m_Tea.encrypt( nmsg.szMail, USR_INFOMAIL_LENGTH ); 
	m_Tea.encrypt( nmsg.szSA, USR_PASS_LENGTH ); 

	return Send((char *) &nmsg);
}