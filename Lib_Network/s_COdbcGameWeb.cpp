/*dmk14 ingame web*/
#include "stdafx.h"
#include "s_COdbcManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int COdbcManager::UserForgotPass(
	char* szUsrID, 
	char* szPasswd )
{
	CString strUserID(szUsrID);
	strUserID.Trim(_T(" "));
	strUserID.Replace(_T("'"), _T("''"));

	CString strPasswd(szPasswd);
	strPasswd.Trim(_T(" "));
	strPasswd.Replace(_T("'"), _T("''"));

	TCHAR szTemp[512] = {0};

	_snprintf( szTemp, 512, "{call user_forgotpass('%s','%s',?)}", 
												strUserID.GetString(),
												strPasswd.GetString() );

	int nReturn = m_pUserDB->ExecuteSpInt(szTemp);

	return nReturn;
}

int COdbcManager::UserChangePass(
	char* szUsrID, 
	char* szPasswd, 
	char* szPasswd2 )
{
	CString strUserID(szUsrID);
	strUserID.Trim(_T(" "));
	strUserID.Replace(_T("'"), _T("''"));

	CString strPasswd(szPasswd);
	strPasswd.Trim(_T(" "));
	strPasswd.Replace(_T("'"), _T("''"));

	CString strPasswd2(szPasswd2);
	strPasswd2.Trim(_T(" "));
	strPasswd2.Replace(_T("'"), _T("''"));

	TCHAR szTemp[512] = {0};

	_snprintf( szTemp, 512, "{call user_changepass('%s','%s','%s',?)}", 
												strUserID.GetString(),
												strPasswd.GetString(),
												strPasswd2.GetString() );

	int nReturn = m_pUserDB->ExecuteSpInt(szTemp);

	return nReturn;
}

int COdbcManager::SelcharChangePass(
	char* szUsrID, 
	char* szPasswd, 
	char* szPincode, 
	char* szNewPasswd )
{
	CString strUserID(szUsrID);
	strUserID.Trim(_T(" "));
	strUserID.Replace(_T("'"), _T("''"));

	CString strPasswd(szPasswd);
	strPasswd.Trim(_T(" "));
	strPasswd.Replace(_T("'"), _T("''"));

	CString strPincode(szPincode);
	strPincode.Trim(_T(" "));
	strPincode.Replace(_T("'"), _T("''"));

	CString strNewPasswd(szNewPasswd);
	strNewPasswd.Trim(_T(" "));
	strNewPasswd.Replace(_T("'"), _T("''"));

	TCHAR szTemp[512] = {0};

	_snprintf( szTemp, 512, "{call selchar_changepass('%s','%s','%s','%s',?)}", 
												strUserID.GetString(),
												strPasswd.GetString(),
												strPincode.GetString(),
												strNewPasswd.GetString() );

	int nReturn = m_pUserDB->ExecuteSpInt(szTemp);

	return nReturn;
}

int COdbcManager::SelcharChangePin(
	char* szUsrID, 
	char* szPasswd, 
	char* szPincode, 
	char* szNewPincode )
{
	CString strUserID(szUsrID);
	strUserID.Trim(_T(" "));
	strUserID.Replace(_T("'"), _T("''"));

	CString strPasswd(szPasswd);
	strPasswd.Trim(_T(" "));
	strPasswd.Replace(_T("'"), _T("''"));

	CString strPincode(szPincode);
	strPincode.Trim(_T(" "));
	strPincode.Replace(_T("'"), _T("''"));

	CString strNewPincode(szNewPincode);
	strNewPincode.Trim(_T(" "));
	strNewPincode.Replace(_T("'"), _T("''"));

	TCHAR szTemp[512] = {0};

	_snprintf( szTemp, 512, "{call selchar_changepin('%s','%s','%s','%s',?)}", 
												strUserID.GetString(),
												strPasswd.GetString(),
												strPincode.GetString(),
												strNewPincode.GetString() );

	int nReturn = m_pUserDB->ExecuteSpInt(szTemp);

	return nReturn;
}

int COdbcManager::ReqGameTimeConvert( char* szUsrID )
{
	CString strUserID(szUsrID);
	strUserID.Trim(_T(" "));
	strUserID.Replace(_T("'"), _T("''"));

	TCHAR szTemp[512] = {0};
	_snprintf( szTemp, 512, "{call sp_ReqGameTimeConvert('%s',?)}", strUserID.GetString() );

	int nReturn = m_pUserDB->ExecuteSpInt(szTemp);

	return nReturn;
}

int COdbcManager::SelcharResetPin(
	char* szUsrID, 
	char* szPasswd, 
	char* szMail, 
	char* szNewPincode )
{
	CString strUserID(szUsrID);
	strUserID.Trim(_T(" "));
	strUserID.Replace(_T("'"), _T("''"));

	CString strPasswd(szPasswd);
	strPasswd.Trim(_T(" "));
	strPasswd.Replace(_T("'"), _T("''"));

	CString strMail(szMail);
	strMail.Trim(_T(" "));
	strMail.Replace(_T("'"), _T("''"));

	CString strNewPincode(szNewPincode);
	strNewPincode.Trim(_T(" "));
	strNewPincode.Replace(_T("'"), _T("''"));

	TCHAR szTemp[512] = {0};

	_snprintf( szTemp, 512, "{call selchar_resetpin('%s','%s','%s','%s',?)}", 
												strUserID.GetString(),
												strPasswd.GetString(),
												strMail.GetString(),
												strNewPincode.GetString() );

	int nReturn = m_pUserDB->ExecuteSpInt(szTemp);

	return nReturn;
}

int COdbcManager::SelcharTopUp(
	char* szUsrID, 
	char* szCode, 
	char* szPin )
{
	CString strUserID(szUsrID);
	strUserID.Trim(_T(" "));
	strUserID.Replace(_T("'"), _T("''"));

	CString strCode(szCode);
	strCode.Trim(_T(" "));
	strCode.Replace(_T("'"), _T("''"));

	CString strPin(szPin);
	strPin.Trim(_T(" "));
	strPin.Replace(_T("'"), _T("''"));

	TCHAR szTemp[512] = {0};

	_snprintf( szTemp, 512, "{call selchar_topup('%s','%s','%s',?)}", 
												strUserID.GetString(),
												strCode.GetString(),
												strPin.GetString() );

	int nReturn = m_pUserDB->ExecuteSpInt(szTemp);

	return nReturn;
}

int COdbcManager::SelcharChangeMail(
	char* szUsrID, 
	char* szPasswd, 
	char* szPincode, 
	char* szMailOld, 
	char* szMailNew )
{
	CString strUserID(szUsrID);
	strUserID.Trim(_T(" "));
	strUserID.Replace(_T("'"), _T("''"));

	CString strPasswd(szPasswd);
	strPasswd.Trim(_T(" "));
	strPasswd.Replace(_T("'"), _T("''"));

	CString strPincode(szPincode);
	strPincode.Trim(_T(" "));
	strPincode.Replace(_T("'"), _T("''"));

	CString strMailOld(szMailOld);
	strMailOld.Trim(_T(" "));
	strMailOld.Replace(_T("'"), _T("''"));

	CString strMailNew(szMailNew);
	strMailNew.Trim(_T(" "));
	strMailNew.Replace(_T("'"), _T("''"));

	TCHAR szTemp[512] = {0};

	_snprintf( szTemp, 512, "{call selchar_changemail('%s','%s','%s','%s','%s',?)}", 
												strUserID.GetString(),
												strPasswd.GetString(),
												strPincode.GetString(),
												strMailOld.GetString(),
												strMailNew.GetString() );

	int nReturn = m_pUserDB->ExecuteSpInt(szTemp);

	return nReturn;
}