#include "stdafx.h"
#include "s_COdbcManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*register page, Juver, 2017/11/18 */ 
int COdbcManager::UserRegister( const TCHAR* szUser, const TCHAR* szPass, const TCHAR* szPass2, const TCHAR* szSA, const TCHAR* szMail, int nSvrGrp, int nSvrNum )
{
	CString strUser(szUser);
	strUser.Trim(_T(" "));
	strUser.Replace(_T("'"), _T("''"));

	CString strPass(szPass);
	strPass.Trim(_T(" "));
	strPass.Replace(_T("'"), _T("''"));

	CString strPass2(szPass2);
	strPass2.Trim(_T(" "));
	strPass2.Replace(_T("'"), _T("''"));

	CString strSA(szSA);
	strSA.Trim(_T(" "));
	strSA.Replace(_T("'"), _T("''"));

	CString strMail(szMail);
	strMail.Trim(_T(" "));
	strMail.Replace(_T("'"), _T("''"));

	TCHAR szTemp[1024] = {0};

	_snprintf( szTemp, 1024, "{call user_register('%s','%s','%s','%s','%s', %d, %d,?)}", 
		strUser.GetString(),
		strPass.GetString(),
		strPass2.GetString(),
		strSA.GetString(),
		strMail.GetString(),
		nSvrGrp,
		nSvrNum );

	int nReturn = m_pUserDB->ExecuteSpInt(szTemp);

	return nReturn;
}