#include "stdafx.h"
#include "s_COdbcManager.h"
#include "s_CDbAction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*equipment lock, Juver, 2018/01/14 */
int COdbcManager::EquipmentLockEnable (DWORD dwChaNum, char* szChaName, char* szEPin )
{
	SQLRETURN	sReturn = 0;	

	CString strName (szChaName);
	strName.Trim(_T(" ")); 
	strName.Replace(_T("'"), _T("''"));

    if (strName.GetLength() >= (CHR_ID_LENGTH+1))
	{
		return DB_ERROR;
	}

	CString strEPin (szEPin);
	strEPin.Trim(_T(" ")); 
	strEPin.Replace(_T("'"), _T("''"));

	if (strEPin.GetLength() >= (CHAR_EQUIPMENT_LOCK_PASS_SIZE+1))
	{
		return DB_ERROR;
	}


	TCHAR szTemp[128] = {0};
	_snprintf( szTemp, 128, "{call sp_InsertEquipmentPass(%d,'%s','%s',?)}", dwChaNum, strName.GetString(), strEPin.GetString() ); 

	int nReturn = m_pGameDB->ExecuteSpInt(szTemp);
	return nReturn;
}

/*equipment lock, Juver, 2018/01/16 */
int COdbcManager::EquipmentLockInput (DWORD dwChaNum, char* szChaName, char* szEPin )
{
	SQLRETURN	sReturn = 0;	

	CString strName (szChaName);
	strName.Trim(_T(" ")); 
	strName.Replace(_T("'"), _T("''"));

	if (strName.GetLength() >= (CHR_ID_LENGTH+1))
	{
		return DB_ERROR;
	}

	CString strEPin (szEPin);
	strEPin.Trim(_T(" ")); 
	strEPin.Replace(_T("'"), _T("''"));

	if (strEPin.GetLength() >= (CHAR_EQUIPMENT_LOCK_PASS_SIZE+1))
	{
		return DB_ERROR;
	}

	TCHAR szTemp[128] = {0};
	_snprintf( szTemp, 128, "{call sp_CheckEquipmentPass(%d,'%s','%s',?)}", dwChaNum, strName.GetString(), strEPin.GetString() ); 

	int nReturn = m_pGameDB->ExecuteSpInt(szTemp);
	return nReturn;
}

/*equipment lock, Juver, 2018/01/16 */
int COdbcManager::EquipmentLockRecover (DWORD dwChaNum, char* szChaName, std::string& strEPin )
{
	SQLRETURN	sReturn = 0;	
	int		  nRowCount = 0;

	strEPin = "";

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
	_snprintf( szTemp, 256, "SELECT ChaEPass FROM ChaEquipmentPass WHERE ChaNum=%u And ChaName='%s'", dwChaNum, strName.GetString() );

	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*) szTemp,  SQL_NTS);

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{
		Print(szTemp);		
		Print(GetErrorString(pConn->hStmt));
		m_pGameDB->FreeConnection(pConn);
		return DB_ERROR;
	}

	SQLCHAR    szChaEPin[CHAR_EQUIPMENT_LOCK_PASS_SIZE+1] = {0}; SQLINTEGER cbChaEPin = SQL_NTS;

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
			::SQLGetData(pConn->hStmt,  1, SQL_C_CHAR,  szChaEPin,    CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, &cbChaEPin);

			if (cbChaEPin != 0 && cbChaEPin != -1)
				strEPin = (const TCHAR*) szChaEPin;

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
		CConsoleMessage::GetInstance()->WriteDatabase(_T("ERROR:EquipmentLockRecover RowCount = 0, ChaNum:%d ChaName:%s"), dwChaNum, szChaName );
		return DB_ROWCNT_ERROR;
	}

	if ( strEPin.size() <= 0 )
		return DB_ERROR;

	return DB_OK;
}

/*equipment lock, Juver, 2018/01/18 */
int COdbcManager::EquipmentLockRecoverChange (DWORD dwChaNum, char* szChaName, char* szEPin )
{
	SQLRETURN	sReturn = 0;	

	CString strName (szChaName);
	strName.Trim(_T(" ")); 
	strName.Replace(_T("'"), _T("''"));

	if (strName.GetLength() >= (CHR_ID_LENGTH+1))
	{
		return DB_ERROR;
	}

	CString strEPin (szEPin);
	strEPin.Trim(_T(" ")); 
	strEPin.Replace(_T("'"), _T("''"));

	if (strEPin.GetLength() >= (CHAR_EQUIPMENT_LOCK_PASS_SIZE+1))
	{
		return DB_ERROR;
	}


	TCHAR szTemp[128] = {0};
	_snprintf( szTemp, 128, "{call sp_ChangeEquipmentPass(%d,'%s','%s',?)}", dwChaNum, strName.GetString(), strEPin.GetString() ); 

	int nReturn = m_pGameDB->ExecuteSpInt(szTemp);
	return nReturn;
}

/*equipment lock, Juver, 2018/01/18 */
int COdbcManager::EquipmentLockRecoverDelete (DWORD dwChaNum, char* szChaName )
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
	_snprintf( szTemp, 128, "{call sp_DeleteEquipmentPass(%d,'%s',?)}", dwChaNum, strName.GetString() ); 

	int nReturn = m_pGameDB->ExecuteSpInt(szTemp);
	return nReturn;

	return 0;
}