/*dmk14 ingame web*/
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

int CNetClient::SndForgotPass(	const char* szUserID,
								const char* szUserPassword )
{
	NET_FORGOTPASS_DATA nld;
	
	//CHINA_MD5::MD5 md5_first;

	size_t nLength = 0;
	HRESULT sRet = StringCchLength( szUserPassword, USR_PASS_LENGTH, &nLength );

	if (sRet == STRSAFE_E_INVALID_PARAMETER )
		return NET_ERROR;
	
	//md5_first.update( reinterpret_cast<const unsigned char*> (szUserPassword), nLength );	

	//std::string strPass = md5_first.asString();

	StringCchCopy( nld.szUserid,   USR_ID_LENGTH+1, szUserID );
	StringCchCopy( nld.szPassword, USR_PASS_LENGTH, szUserPassword );
	
	m_Tea.encrypt( nld.szUserid, USR_ID_LENGTH+1 );
	m_Tea.encrypt( nld.szPassword, USR_PASS_LENGTH );

	return Send((char *) &nld);
}

int CNetClient::SndForgotChangePass( const char* szUserPassword,
									 const char* szUserPassword2 )
{
	NET_CHANGEPASS_DATA nld;
	
	CHINA_MD5::MD5 md5_first;
	CHINA_MD5::MD5 md5_second;

	size_t nLength = 0;
	HRESULT sRet = StringCchLength( szUserPassword, USR_PASS_LENGTH, &nLength );

	size_t nLength2 = 0;
	HRESULT sRet2 = StringCchLength( szUserPassword2, USR_PASS_LENGTH, &nLength2 );

	if (sRet == STRSAFE_E_INVALID_PARAMETER || sRet2 == STRSAFE_E_INVALID_PARAMETER )
		return NET_ERROR;

	
	/*
	md5_first.update( reinterpret_cast<const unsigned char*> (szUserPassword), nLength );	
	md5_second.update( reinterpret_cast<const unsigned char*> (szUserPassword2), nLength2 );

	std::string strPass = md5_first.asString();
	std::string strPass2 = md5_second.asString();
	*/

	StringCchCopy( nld.szPassword, USR_PASS_LENGTH, szUserPassword );
	StringCchCopy( nld.szPassword2, USR_PASS_LENGTH, szUserPassword2 );
	
	m_Tea.encrypt( nld.szPassword, USR_PASS_LENGTH );
	m_Tea.encrypt( nld.szPassword2, USR_PASS_LENGTH );

	return Send((char *) &nld);
}

int CNetClient::SndAccountChangePass( const char* szUserPassword,
									  const char* szUserPincode,
									  const char* szUserNewPass )
{
	NET_SELCHAR_CHANGEPASS nld;
	
	CHINA_MD5::MD5 md5_Pass;
	CHINA_MD5::MD5 md5_Pin;
	CHINA_MD5::MD5 md5_NewPass;

	size_t nLength_Pass = 0;
	HRESULT sRet_Pass = StringCchLength( szUserPassword, USR_PASS_LENGTH, &nLength_Pass );

	size_t nLength_Pin = 0;
	HRESULT sRet_Pin = StringCchLength( szUserPincode, USR_PASS_LENGTH, &nLength_Pin );

	size_t nLength_NewPass = 0;
	HRESULT sRet_NewPass = StringCchLength( szUserNewPass, USR_PASS_LENGTH, &nLength_NewPass );

	if (sRet_Pass == STRSAFE_E_INVALID_PARAMETER || sRet_Pin == STRSAFE_E_INVALID_PARAMETER ||
		sRet_NewPass == STRSAFE_E_INVALID_PARAMETER )
		return NET_ERROR;

	/*
	md5_Pass.update( reinterpret_cast<const unsigned char*> (szUserPassword), nLength_Pass );
	md5_Pin.update( reinterpret_cast<const unsigned char*> (szUserPincode), nLength_Pin );
	md5_NewPass.update( reinterpret_cast<const unsigned char*> (szUserNewPass), nLength_NewPass );

	std::string strPass = md5_Pass.asString();
	std::string strPin = md5_Pin.asString();
	std::string strNewPass = md5_NewPass.asString();
	*/

	StringCchCopy( nld.szPassword, USR_PASS_LENGTH, szUserPassword );
	StringCchCopy( nld.szPincode, USR_PASS_LENGTH, szUserPincode );
	StringCchCopy( nld.szNewPassword, USR_PASS_LENGTH, szUserNewPass );
	
	m_Tea.encrypt( nld.szPassword, USR_PASS_LENGTH );
	m_Tea.encrypt( nld.szPincode, USR_PASS_LENGTH );
	m_Tea.encrypt( nld.szNewPassword, USR_PASS_LENGTH );

	return Send((char *) &nld);
}

int CNetClient::SndAccountChangePin( const char* szUserPassword,
									 const char* szUserPincode,
									 const char* szUserNewPin )
{
	NET_SELCHAR_CHANGEPIN nld;
	
	CHINA_MD5::MD5 md5_Pass;
	CHINA_MD5::MD5 md5_Pin;
	CHINA_MD5::MD5 md5_NewPin;

	size_t nLength_Pass = 0;
	HRESULT sRet_Pass = StringCchLength( szUserPassword, USR_PASS_LENGTH, &nLength_Pass );

	size_t nLength_Pin = 0;
	HRESULT sRet_Pin = StringCchLength( szUserPincode, USR_PASS_LENGTH, &nLength_Pin );

	size_t nLength_NewPin = 0;
	HRESULT sRet_NewPin = StringCchLength( szUserNewPin, USR_PASS_LENGTH, &nLength_NewPin );

	if (sRet_Pass == STRSAFE_E_INVALID_PARAMETER || sRet_Pin == STRSAFE_E_INVALID_PARAMETER ||
		sRet_NewPin == STRSAFE_E_INVALID_PARAMETER )
		return NET_ERROR;

	
	/*
	md5_Pass.update( reinterpret_cast<const unsigned char*> (szUserPassword), nLength_Pass );
	md5_Pin.update( reinterpret_cast<const unsigned char*> (szUserPincode), nLength_Pin );
	md5_NewPin.update( reinterpret_cast<const unsigned char*> (szUserNewPin), nLength_NewPin );
	*/

	//std::string strPass = md5_Pass.asString();
	//std::string strPin = md5_Pin.asString();
	//std::string strNewPin = md5_NewPin.asString();

	StringCchCopy( nld.szPassword, USR_PASS_LENGTH, szUserPassword );
	StringCchCopy( nld.szPincode, USR_PASS_LENGTH, szUserPincode );
	StringCchCopy( nld.szNewPincode, USR_PASS_LENGTH, szUserNewPin );
	
	m_Tea.encrypt( nld.szPassword, USR_PASS_LENGTH );
	m_Tea.encrypt( nld.szPincode, USR_PASS_LENGTH );
	m_Tea.encrypt( nld.szNewPincode, USR_PASS_LENGTH );

	return Send((char *) &nld);
}

int CNetClient::ReqGameTimeConvert()
{
	NET_REQ_GAMETIME_CONVERT nld;
	return Send((char *) &nld);
}

int CNetClient::SndAccountResetPin( const char* szUserPassword, const char* szUserMail )
{
	NET_SELCHAR_RESETPIN nld;
	
	CHINA_MD5::MD5 md5_Pass;

	size_t nLength_Pass = 0;
	HRESULT sRet_Pass = StringCchLength( szUserPassword, USR_PASS_LENGTH, &nLength_Pass );

	if (sRet_Pass == STRSAFE_E_INVALID_PARAMETER )				return NET_ERROR;
	
	//md5_Pass.update( reinterpret_cast<const unsigned char*> (szUserPassword), nLength_Pass );

	//std::string strPass = md5_Pass.asString();

	StringCchCopy( nld.szPassword, USR_PASS_LENGTH, szUserPassword );
	StringCchCopy( nld.szEmail, USR_INFOMAIL_LENGTH, szUserMail );

	m_Tea.encrypt( nld.szPassword, USR_PASS_LENGTH );
	m_Tea.encrypt( nld.szEmail, USR_INFOMAIL_LENGTH );

	return Send((char *) &nld);
}

int CNetClient::SndAccountChangeMail( const char* szUserPassword, const char* szUserPin, const char* szUserMailOld, const char* szUserMailNew )
{
	NET_SELCHAR_CHANGEMAIL nld;
	
	CHINA_MD5::MD5 md5_Pass;
	CHINA_MD5::MD5 md5_Pin;

	size_t nLength_Pass = 0;
	HRESULT sRet_Pass = StringCchLength( szUserPassword, USR_PASS_LENGTH, &nLength_Pass );

	size_t nLength_Pin = 0;
	HRESULT sRet_Pin = StringCchLength( szUserPin, USR_PASS_LENGTH, &nLength_Pin );

	if (sRet_Pass == STRSAFE_E_INVALID_PARAMETER || sRet_Pin == STRSAFE_E_INVALID_PARAMETER )				return NET_ERROR;
	
	/*
	md5_Pass.update( reinterpret_cast<const unsigned char*> (szUserPassword), nLength_Pass );
	md5_Pin.update( reinterpret_cast<const unsigned char*> (szUserPin), nLength_Pin );

	std::string strPass = md5_Pass.asString();
	std::string strPin = md5_Pin.asString();
	*/

	StringCchCopy( nld.szPassword, USR_PASS_LENGTH, szUserPassword );
	StringCchCopy( nld.szPincode, USR_PASS_LENGTH, szUserPin );
	StringCchCopy( nld.szEmailOld, USR_INFOMAIL_LENGTH, szUserMailOld );
	StringCchCopy( nld.szEmailNew, USR_INFOMAIL_LENGTH, szUserMailNew );

	m_Tea.encrypt( nld.szPassword, USR_PASS_LENGTH );
	m_Tea.encrypt( nld.szPincode, USR_PASS_LENGTH );
	m_Tea.encrypt( nld.szEmailOld, USR_INFOMAIL_LENGTH );
	m_Tea.encrypt( nld.szEmailNew, USR_INFOMAIL_LENGTH );

	return Send((char *) &nld);
}

int CNetClient::SndAccountTopUp( const char* szCode,const char* szPin )
{
	NET_SELCHAR_TOPUP nld;

	StringCchCopy( nld.szCode, TOPUP_CODE_LENGTH, szCode );
	StringCchCopy( nld.szPin, TOPUP_PIN_LENGTH, szPin );

	m_Tea.encrypt( nld.szCode, TOPUP_CODE_LENGTH );
	m_Tea.encrypt( nld.szPin, TOPUP_PIN_LENGTH );

	return Send((char *) &nld);
}