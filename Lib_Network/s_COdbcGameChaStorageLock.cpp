#include "stdafx.h"
#include "s_COdbcManager.h"
#include "s_CDbAction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*storage lock, Juver, 2018/01/14 */
int COdbcManager::StorageLockEnable (DWORD dwChaNum, char* szChaName, char* szSPin )
{
	SQLRETURN	sReturn = 0;	

	CString strName (szChaName);
	strName.Trim(_T(" ")); 
	strName.Replace(_T("'"), _T("''"));

    if (strName.GetLength() >= (CHR_ID_LENGTH+1))
	{
		return DB_ERROR;
	}

	CString strSPin (szSPin);
	strSPin.Trim(_T(" ")); 
	strSPin.Replace(_T("'"), _T("''"));

	if (strSPin.GetLength() >= (USER_STORAGE_LOCK_PASS_SIZE+1))
	{
		return DB_ERROR;
	}


	TCHAR szTemp[128] = {0};
	_snprintf( szTemp, 128, "{call sp_InsertStoragePass(%d,'%s','%s',?)}", dwChaNum, strName.GetString(), strSPin.GetString() ); 

	int nReturn = m_pGameDB->ExecuteSpInt(szTemp);
	return nReturn;
}

/*storage lock, Juver, 2018/01/16 */
int COdbcManager::StorageLockInput (DWORD dwChaNum, char* szChaName, char* szSPin )
{
	SQLRETURN	sReturn = 0;	

	CString strName (szChaName);
	strName.Trim(_T(" ")); 
	strName.Replace(_T("'"), _T("''"));

	if (strName.GetLength() >= (CHR_ID_LENGTH+1))
	{
		return DB_ERROR;
	}

	CString strSPin (szSPin);
	strSPin.Trim(_T(" ")); 
	strSPin.Replace(_T("'"), _T("''"));

	if (strSPin.GetLength() >= (USER_STORAGE_LOCK_PASS_SIZE+1))
	{
		return DB_ERROR;
	}

	TCHAR szTemp[128] = {0};
	_snprintf( szTemp, 128, "{call sp_CheckStoragePass(%d,'%s','%s',?)}", dwChaNum, strName.GetString(), strSPin.GetString() ); 

	int nReturn = m_pGameDB->ExecuteSpInt(szTemp);
	return nReturn;
}

/*storage lock, Juver, 2018/01/16 */
int COdbcManager::StorageLockRecover (DWORD dwChaNum, char* szChaName, std::string& strSPin )
{
	SQLRETURN	sReturn = 0;	
	int		  nRowCount = 0;

	strSPin = "";

	CString strName (szChaName);
	strName.Trim(_T(" ")); 
	strName.Replace(_T("'"), _T("''"));

	if (strName.GetLength() >= (CHR_ID_LENGTH+1))
	{
		return DB_ERROR;
	}

	ODBC_STMT* pConn = m_pGameDB->GetConnection();
	if (!pConn) return DB_ERROR;

	TCHAR szTemp[256] = {0};
	_snprintf( szTemp, 256, "SELECT ChaSPass FROM ChaStoragePass WHERE ChaNum=%u And ChaName='%s'", dwChaNum, strName.GetString() );

	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*) szTemp,  SQL_NTS);

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{
		Print(szTemp);		
		Print(GetErrorString(pConn->hStmt));
		m_pGameDB->FreeConnection(pConn);
		return DB_ERROR;
	}

	SQLCHAR    szChaSPin[USER_STORAGE_LOCK_PASS_SIZE+1] = {0}; SQLINTEGER cbChaSPin = SQL_NTS;

	while (true)
	{
		sReturn = ::SQLFetch(pConn->hStmt);
		if (sReturn == SQL_ERROR || sReturn == SQL_SUCCESS_WITH_INFO)
		{
			Print(szTemp);		
			Print(GetErrorString(pConn->hStmt));
			m_pGameDB->FreeConnection(pConn);
			return DB_ERROR;
		}

		if (sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO)
		{
			::SQLGetData(pConn->hStmt,  1, SQL_C_CHAR,  szChaSPin,    USER_STORAGE_LOCK_PASS_SIZE+1, &cbChaSPin);

			if (cbChaSPin != 0 && cbChaSPin != -1)
				strSPin = (const TCHAR*) szChaSPin;

			nRowCount++;
		}
		else
		{
			break;
		}
		Sleep( 0 );
	}

	m_pGameDB->FreeConnection(pConn);

	if (nRowCount == 1)	{
		nRowCount = 0;
	}else{
		CConsoleMessage::GetInstance()->WriteDatabase(_T("ERROR:StorageLockRecover RowCount = 0, ChaNum:%d ChaName:%s"), dwChaNum, szChaName );
		return DB_ROWCNT_ERROR;
	}

	if ( strSPin.size() <= 0 )
		return DB_ERROR;

	return DB_OK;
}

/*storage lock, Juver, 2018/01/18 */
int COdbcManager::StorageLockRecoverChange (DWORD dwChaNum, char* szChaName, char* szSPin )
{
	SQLRETURN	sReturn = 0;	

	CString strName (szChaName);
	strName.Trim(_T(" ")); 
	strName.Replace(_T("'"), _T("''"));

	if (strName.GetLength() >= (CHR_ID_LENGTH+1))
	{
		return DB_ERROR;
	}

	CString strSPin (szSPin);
	strSPin.Trim(_T(" ")); 
	strSPin.Replace(_T("'"), _T("''"));

	if (strSPin.GetLength() >= (USER_STORAGE_LOCK_PASS_SIZE+1))
	{
		return DB_ERROR;
	}


	TCHAR szTemp[128] = {0};
	_snprintf( szTemp, 128, "{call sp_ChangeStoragePass(%d,'%s','%s',?)}", dwChaNum, strName.GetString(), strSPin.GetString() ); 

	int nReturn = m_pGameDB->ExecuteSpInt(szTemp);
	return nReturn;
}

/*storage lock, Juver, 2018/01/18 */
int COdbcManager::StorageLockRecoverDelete (DWORD dwChaNum, char* szChaName )
{
	SQLRETURN	sReturn = 0;	

	CString strName (szChaName);
	strName.Trim(_T(" ")); 
	strName.Replace(_T("'"), _T("''"));

	if (strName.GetLength() >= (CHR_ID_LENGTH+1))
	{
		return DB_ERROR;
	}

	TCHAR szTemp[128] = {0};
	_snprintf( szTemp, 128, "{call sp_DeleteStoragePass(%d,'%s',?)}", dwChaNum, strName.GetString() ); 

	int nReturn = m_pGameDB->ExecuteSpInt(szTemp);
	return nReturn;

	return 0;
}