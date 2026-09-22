#include "stdafx.h"
#include "s_COdbcManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////
// Shop 에서 구입한 아이템을 가져온다.
// 가져온 아이템은 실제로 가져 갈 수 있는 아이템이 아니다.
// 아이템을 가져갈때는 실제로 가져갈 수 있는지 다시 확인해야 한다. (SetPurchaseItem)
// 웹을 통한 구매와 취소 및 게임내 캐릭터의 동기화 문제.
int COdbcManager::GetPurchaseItem(CString strUID, std::vector<SHOPPURCHASE> &v)
{
    ODBC_STMT* pConn = m_pShopDB->GetConnection();
	if (!pConn)	return DB_ERROR;    

	//std::strstream strTemp;
 //   strTemp << "SELECT Purkey, ItemMain, ItemSub FROM viewShopPurchase WHERE ";
 //   strTemp << "useruid='" << strUID.GetString() << "' AND ";
 //   strTemp << "PurFlag=0";
 //   strTemp << std::ends;

	TCHAR szTemp[128] = {0};
	_snprintf( szTemp, 128, "SELECT Purkey, ItemMain, ItemSub FROM viewShopPurchase WHERE "
							"useruid='%s' AND PurFlag=0", strUID.GetString() );

	SQLCHAR    szPurKey[PURKEY_LENGTH+1] = {0}; SQLINTEGER cbPurKey = SQL_NTS;
    SQLINTEGER nItemMain = 0, cbItemMain =SQL_NTS;
    SQLINTEGER nItemSub  = 0, cbItemSub  =SQL_NTS;
    SQLRETURN  sReturn=0;

    sReturn = ::SQLExecDirect(pConn->hStmt,
							(SQLCHAR*)szTemp, 
							SQL_NTS);

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{
        Print(szTemp);		
		Print(GetErrorString(pConn->hStmt));
		m_pShopDB->FreeConnection(pConn);

		//strTemp.freeze( false );	// Note : std::strstream의 freeze. 안 하면 Leak 발생.

        return DB_ERROR;
	}

    while(true)
	{
		sReturn = ::SQLFetch(pConn->hStmt);
		if (sReturn == SQL_ERROR || sReturn == SQL_SUCCESS_WITH_INFO)
        {
            Print(szTemp);		
			Print(GetErrorString(pConn->hStmt));
            m_pShopDB->FreeConnection(pConn);

			//strTemp.freeze( false );	// Note : std::strstream의 freeze. 안 하면 Leak 발생.

            return DB_ERROR;
		}

		if (sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO)
		{   
            ::SQLGetData(pConn->hStmt, 1, SQL_C_CHAR, szPurKey,   PURKEY_LENGTH+1, &cbPurKey);
            ::SQLGetData(pConn->hStmt, 2, SQL_C_LONG, &nItemMain, 0,             &cbItemMain);
            ::SQLGetData(pConn->hStmt, 3, SQL_C_LONG, &nItemSub,  0,             &cbItemSub);

            SHOPPURCHASE sTemp;

            if (cbPurKey != 0 && cbPurKey != -1)
                sTemp.strPurKey = szPurKey;
            sTemp.wItemMain = static_cast<WORD> (nItemMain);
            sTemp.wItemSub  = static_cast<WORD> (nItemSub);

            v.push_back(sTemp);
		}
		else
		{
			break;
		}
		Sleep( 0 );
	}
	//strTemp.freeze( false );	// Note : std::strstream의 freeze. 안 하면 Leak 발생.

	m_pShopDB->FreeConnection(pConn);
    return DB_OK;
}

///////////////////////////////////////////////////////////////////////////////
// 실제로 가져갈 수 있는 상품인지 확인한다.
// 입력값
// nFlag  : (0 : 구매, 1 : 구매완료, 2 : 구매취소신청, 3 : 구매취소처리)
// 출력값
// 1 : 가져갈수 있음
// 이외의값 : 가져갈수 없음
int COdbcManager::SetPurchaseItem(CString strPurKey, int nFlag)
{
	//std::strstream strTemp;
	//strTemp << "{call sp_purchase_change_state(";	
	//strTemp << "'" << strPurKey.GetString() << "',";		
	//strTemp << nFlag << ",?)}";
	//strTemp << std::ends;

	TCHAR szTemp[256] = {0};
	_snprintf( szTemp, 256, "{call sp_purchase_change_state('%s',%d,?)}", strPurKey.GetString(), nFlag );

	int nReturn = m_pShopDB->ExecuteSpInt(szTemp);
	//strTemp.freeze( false );	// Note : std::strstream의 freeze. 안 하면 Leak 발생.

	return nReturn;
}

/*dmk14 ingame web*/
int COdbcManager::GetItemMall(std::vector<ITEMMALLDATA> &v)
{
	
    ODBC_STMT* pConn = m_pShopDB->GetConnection();
	if (!pConn)	return DB_ERROR;    

	TCHAR szTemp[256] = {0};
	_snprintf( szTemp, 256, "SELECT ProductNum, ItemMain, ItemSub, ItemMoney, ItemStock, Category, ItemSec FROM ShopItemMap WHERE "
							"Itemstock <> 0 and Category <= 1 and ItemVisible = 1 ORDER BY Category ASC, ItemOrder ASC" );

	SQLINTEGER nProductNum  = 0, cbProductNum	= SQL_NTS;
    SQLINTEGER nItemMain	= 0, cbItemMain		=SQL_NTS;
    SQLINTEGER nItemSub		= 0, cbItemSub		=SQL_NTS;
	SQLINTEGER nItemMoney	= 0, cbItemMoney	=SQL_NTS;
	SQLINTEGER nItemStocks  = 0, cbItemStocks	=SQL_NTS;
	SQLINTEGER nItemCtg		= 0, cbItemCtg		=SQL_NTS;
	SQLINTEGER nItemSec		= 0, cbItemSec		=SQL_NTS;

    SQLRETURN  sReturn=0;

    sReturn = ::SQLExecDirect(pConn->hStmt,
							(SQLCHAR*)szTemp, 
							SQL_NTS);

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{
        Print(szTemp);		
		Print(GetErrorString(pConn->hStmt));
		m_pShopDB->FreeConnection(pConn);

        return DB_ERROR;
	}

	int nCounter = 0;
    while(true)
	{
		sReturn = ::SQLFetch(pConn->hStmt);
		if (sReturn == SQL_ERROR || sReturn == SQL_SUCCESS_WITH_INFO)
        {
            Print(szTemp);		
			Print(GetErrorString(pConn->hStmt));
            m_pShopDB->FreeConnection(pConn);

            return DB_ERROR;
		}

		if (sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO)
		{   
			::SQLGetData(pConn->hStmt, 1, SQL_C_LONG, &nProductNum, 0,				&cbProductNum);
            ::SQLGetData(pConn->hStmt, 2, SQL_C_LONG, &nItemMain, 0,				&cbItemMain);
            ::SQLGetData(pConn->hStmt, 3, SQL_C_LONG, &nItemSub,  0,				&cbItemSub);
			::SQLGetData(pConn->hStmt, 4, SQL_C_LONG, &nItemMoney,  0,				&cbItemMoney);
			::SQLGetData(pConn->hStmt, 5, SQL_C_LONG, &nItemStocks,  0,				&cbItemStocks);
			::SQLGetData(pConn->hStmt, 6, SQL_C_LONG, &nItemCtg,  0,				&cbItemCtg);
			::SQLGetData(pConn->hStmt, 7, SQL_C_LONG, &nItemSec,  0,				&cbItemSec);

            ITEMMALLDATA sTemp;

            sTemp.dwProductNum = static_cast<DWORD> (nProductNum);
			sTemp.dwItemPrice = static_cast<DWORD> (nItemMoney);
			sTemp.dwItemStocks = static_cast<DWORD> (nItemStocks);
			sTemp.wItemCtg = static_cast<WORD> (nItemCtg);
			//item section - cndev
			sTemp.wItemSec = static_cast<WORD> (nItemSec);
			sTemp.dwIndex = nCounter;
			WORD wMain = static_cast<WORD> (nItemMain);
			WORD wSub = static_cast<WORD> (nItemSub);
            sTemp.sNativeID = SNATIVEID( wMain, wSub );

            v.push_back(sTemp);
			nCounter++;
		}
		else
		{
			break;
		}
		Sleep( 0 );
	}

	m_pShopDB->FreeConnection(pConn);
    return DB_OK;
}

int COdbcManager::GetItemMall( ITEMMALLDATA &sItemData, DWORD dwProductNum )
{
	
    ODBC_STMT* pConn = m_pShopDB->GetConnection();
	if (!pConn)	return DB_ERROR;    

	TCHAR szTemp[256] = {0};
	_snprintf( szTemp, 256, "SELECT ProductNum, ItemMain, ItemSub, ItemMoney, Itemstock, ItemCtg, ItemSec FROM ShopItemMap WHERE "
							"ProductNum=%d", dwProductNum );

	SQLINTEGER nProductNum  = 0, cbProductNum	= SQL_NTS;
    SQLINTEGER nItemMain	= 0, cbItemMain		=SQL_NTS;
    SQLINTEGER nItemSub		= 0, cbItemSub		=SQL_NTS;
	SQLINTEGER nItemMoney	= 0, cbItemMoney	=SQL_NTS;
	SQLINTEGER nItemStocks  = 0, cbItemStocks	=SQL_NTS;
	SQLINTEGER nItemCtg		= 0, cbItemCtg		=SQL_NTS;
	SQLINTEGER nItemSec		= 0, cbItemSec		=SQL_NTS;

    SQLRETURN  sReturn=0;

    sReturn = ::SQLExecDirect(pConn->hStmt,
							(SQLCHAR*)szTemp, 
							SQL_NTS);

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{
        Print(szTemp);		
		Print(GetErrorString(pConn->hStmt));
		m_pShopDB->FreeConnection(pConn);

        return DB_ERROR;
	}

    while(true)
	{
		sReturn = ::SQLFetch(pConn->hStmt);
		if (sReturn == SQL_ERROR || sReturn == SQL_SUCCESS_WITH_INFO)
        {
            Print(szTemp);		
			Print(GetErrorString(pConn->hStmt));
            m_pShopDB->FreeConnection(pConn);

            return DB_ERROR;
		}

		if (sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO)
		{   
			::SQLGetData(pConn->hStmt, 1, SQL_C_LONG, &nProductNum, 0,				&cbProductNum);
            ::SQLGetData(pConn->hStmt, 2, SQL_C_LONG, &nItemMain, 0,				&cbItemMain);
            ::SQLGetData(pConn->hStmt, 3, SQL_C_LONG, &nItemSub,  0,				&cbItemSub);
			::SQLGetData(pConn->hStmt, 4, SQL_C_LONG, &nItemMoney,  0,				&cbItemMoney);
			::SQLGetData(pConn->hStmt, 5, SQL_C_LONG, &nItemStocks,  0,				&cbItemStocks);
			::SQLGetData(pConn->hStmt, 6, SQL_C_LONG, &nItemCtg,  0,				&cbItemCtg);
			::SQLGetData(pConn->hStmt, 7, SQL_C_LONG, &nItemSec,  0,				&cbItemSec);

            sItemData.dwProductNum = static_cast<DWORD> (nProductNum);
			sItemData.dwItemPrice = static_cast<DWORD> (nItemMoney);
			sItemData.dwItemStocks = static_cast<DWORD> (nItemStocks);
			sItemData.wItemCtg = static_cast<WORD> (nItemCtg);
			//item section - cndev
			sItemData.wItemSec = static_cast<WORD> (nItemSec);

			WORD wMain = static_cast<WORD> (nItemMain);
			WORD wSub = static_cast<WORD> (nItemSub);
            sItemData.sNativeID = SNATIVEID( wMain, wSub );
		}
		else
		{
			break;
		}
		Sleep( 0 );
	}

	m_pShopDB->FreeConnection(pConn);
    return DB_OK;
}

int COdbcManager::InsertBuyItemMall( DWORD dwProductNum, DWORD dwPrice, int wType, CString strUserID )
{
	TCHAR szTemp[128] = {0};
	_snprintf( szTemp, 128, "{call sp_ItemMallBuy(%d,%d,%d,'%s',?)}",
		dwProductNum,
		dwPrice,
		wType,
		strUserID.GetString() );

	int nReturn = m_pShopDB->ExecuteSpInt(szTemp);
    
	return nReturn;
}

/*dmk14 topup generator*/
int COdbcManager::GenerateTopUpCode( DWORD dwEpValue, DWORD dwCpValue, DWORD dwPrice, DWORD dwUserID, DWORD dwClientID )
{
	TCHAR szTemp[128] = {0};
	_snprintf( szTemp, 128, "{call sp_GenerateTopUpCard(%d,%d,%d,%d,?)}",
		dwEpValue,
		dwCpValue,
		dwPrice,
		dwUserID );

	int nReturn = m_pUserDB->ExecuteSpInt(szTemp);

	return nReturn;
}

int COdbcManager::GetTopUpCode( CString &strCode, CString &strPin, DWORD dwID )
{
	ODBC_STMT* pConn = m_pUserDB->GetConnection();
	if (!pConn)	return DB_ERROR;    

	TCHAR szTemp[256] = {0};
	_snprintf( szTemp, 256, "SELECT CardID, CardCODE FROM TopUp WHERE ID=%d", dwID );

    SQLRETURN  sReturn=0;

    sReturn = ::SQLExecDirect(pConn->hStmt,
							(SQLCHAR*)szTemp, 
							SQL_NTS);

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{
        Print(szTemp);		
		Print(GetErrorString(pConn->hStmt));
		m_pUserDB->FreeConnection(pConn);

        return DB_ERROR;
	}

	SQLCHAR		szCode[TOPUP_CODE_LENGTH+1] = {0}; SQLINTEGER cbCode = SQL_NTS;
	SQLCHAR		szPin[TOPUP_PIN_LENGTH+1] = {0}; SQLINTEGER cbPin = SQL_NTS;

    while(true)
	{
		sReturn = ::SQLFetch(pConn->hStmt);
		if (sReturn == SQL_ERROR || sReturn == SQL_SUCCESS_WITH_INFO)
        {
            Print(szTemp);		
			Print(GetErrorString(pConn->hStmt));
            m_pUserDB->FreeConnection(pConn);

            return DB_ERROR;
		}

		if (sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO)
		{   
			::SQLGetData(pConn->hStmt, 1, SQL_C_CHAR, szCode, TOPUP_CODE_LENGTH, &cbCode );
			::SQLGetData(pConn->hStmt, 2, SQL_C_CHAR, szPin, TOPUP_PIN_LENGTH, &cbPin );

			if (cbCode != 0 )	strCode = szCode;
			if (cbPin != 0 )	strPin = szPin;
		}
		else
		{
			break;
		}
		Sleep( 0 );
	}

	m_pUserDB->FreeConnection(pConn);
    return DB_OK;
}

int COdbcManager::ViewTopUpCode( WORD &wStatus, WORD &wValue, CString &strUserTopUp, CString strCode )
{
	ODBC_STMT* pConn = m_pUserDB->GetConnection();
	if (!pConn)	return DB_ERROR;    

	TCHAR szTemp[256] = {0};
	_snprintf( szTemp, 256, "SELECT CardSTATUS, CardVALUE, UserTopUp FROM TopUp WHERE CardID='%s'", strCode.GetString() );

    SQLRETURN  sReturn=0;

    sReturn = ::SQLExecDirect(pConn->hStmt,
							(SQLCHAR*)szTemp, 
							SQL_NTS);

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{
        Print(szTemp);		
		Print(GetErrorString(pConn->hStmt));
		m_pUserDB->FreeConnection(pConn);

        return DB_ERROR;
	}

	SQLINTEGER	nCardStatus=0, cbCardStatus=SQL_NTS;
	SQLINTEGER	nCardValue=0, cbCardValue=SQL_NTS;
	SQLCHAR		szUser[USR_ID_LENGTH+1] = {0}; SQLINTEGER cbUser = SQL_NTS;

    while(true)
	{
		sReturn = ::SQLFetch(pConn->hStmt);
		if (sReturn == SQL_ERROR || sReturn == SQL_SUCCESS_WITH_INFO)
        {
            Print(szTemp);		
			Print(GetErrorString(pConn->hStmt));
            m_pUserDB->FreeConnection(pConn);

            return DB_ERROR;
		}

		if (sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO)
		{   
			::SQLGetData(pConn->hStmt, 1, SQL_C_LONG, &nCardStatus, 0, &cbCardStatus);	
			::SQLGetData(pConn->hStmt, 2, SQL_C_LONG, &nCardValue, 0, &cbCardValue);	
			::SQLGetData(pConn->hStmt, 3, SQL_C_CHAR, szUser, USR_ID_LENGTH, &cbUser );

			wStatus = nCardStatus;
			wValue = nCardValue;
			if ( cbUser != 0 )	strUserTopUp = szUser;
		}
		else
		{
			break;
		}
		Sleep( 0 );
	}

	m_pUserDB->FreeConnection(pConn);
    return DB_OK;
}