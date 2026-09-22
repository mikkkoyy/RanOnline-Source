#include "stdAfx.h"
#include "GMToolOdbcBase.h"
#include "GMToolOdbcConn.h"
#include "GMToolLogs.h"
#include "GMToolGlobal.h"
#include "HLibTimeFunctions.h"
#include "HLibDataConvert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CGMToolOdbcBase::UserBankGet( std::string strUSER, SGMTOOL_USER_BANK_ITEM_MAP &mapResult )
{
	mapResult.clear();

	if ( strUSER.size() <= 0 ){
		return DB_ERROR;
	}

	CGMToolOdbcConn* pDatabase = m_pShopDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "SELECT PurKey, ProductNum, PurPrice, PurDate, ItemName, ItemMain, ItemSub FROM viewShopPurchase WHERE UserUID = '%s' AND PurFlag=0 ORDER BY PurDate DESC", strUSER.c_str() );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	SQLRETURN sReturn = 0;
	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS );

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{	
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}

	SQLINTEGER nPurKey = 0, cbPurKey=SQL_NTS;
	SQLINTEGER nProductNum = 0, cbProductNum=SQL_NTS;
	SQLINTEGER nPurPrice = 0, cbPurPrice=SQL_NTS;
	TIMESTAMP_STRUCT sPurDate; SQLINTEGER cbPurDate = SQL_NTS;
	SQLCHAR    szItemName[SHOP_ITEMNAME_LENGTH+1] = {0}; SQLINTEGER cbItemName = SQL_NTS;
	SQLINTEGER nItemMain = 0, cbItemMain=SQL_NTS;
	SQLINTEGER nItemSub = 0, cbItemSub=SQL_NTS;

	while (true)
	{
		sReturn = ::SQLFetch(pConn->hStmt);
		if (sReturn == SQL_ERROR || sReturn == SQL_SUCCESS_WITH_INFO)
		{		
			GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
			pDatabase->FreeConnection(pConn);
			return DB_ERROR;
		}

		if (sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO)
		{			
			::SQLGetData(pConn->hStmt,  1, SQL_C_LONG,	&nPurKey,           0, &cbPurKey );
			::SQLGetData(pConn->hStmt,  2, SQL_C_LONG,	&nProductNum,        0, &cbProductNum );
			::SQLGetData(pConn->hStmt,  3, SQL_C_LONG,	&nPurPrice,        0, &cbPurPrice );
			::SQLGetData(pConn->hStmt,  4, SQL_C_TYPE_TIMESTAMP, &sPurDate, 0, &cbPurDate);
			::SQLGetData(pConn->hStmt,  5, SQL_C_CHAR,  szItemName, SHOP_ITEMNAME_LENGTH, &cbItemName);
			::SQLGetData(pConn->hStmt,  6, SQL_C_LONG,	&nItemMain,     0, &cbItemMain );
			::SQLGetData(pConn->hStmt,  7, SQL_C_LONG,	&nItemSub,      0, &cbItemSub );

			_HLIB::timestamp_correction( sPurDate );

			SGMTOOL_USER_BANK_ITEM sDATA;
			sDATA.dwPurKey = (DWORD)nPurKey;
			sDATA.dwProductNum = (DWORD)nProductNum;
			sDATA.dwPrice = (DWORD)nPurPrice;
			sDATA.tPurchaseDate = _HLIB::timestamp_timet64(sPurDate);
			sDATA.wItemMain = (WORD) nItemMain;
			sDATA.wItemSub = (WORD) nItemSub;

			if (cbItemName != 0 && cbItemName != -1){
				::StringCchCopy( sDATA.szItemName, SHOP_ITEMNAME_LENGTH+1, (const char*) szItemName);
			}

			mapResult.insert( std::make_pair( sDATA.dwPurKey, sDATA ) );
		}
		else
		{
			break;
		}
		Sleep( 0 );
	}	  
	pDatabase->FreeConnection(pConn);

	return DB_OK;
}

int CGMToolOdbcBase::UserBankMarkTaken( std::string strUSER, DWORD dwPurKey )
{
	if ( strUSER.size() <= 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[512] = {0};
	_snprintf( szTemp, 512, "UPDATE ShopPurchase SET PurFlag = 1, PurChgDate = getdate() WHERE UserUID = '%s' AND PurKey=%d", strUSER.c_str(), dwPurKey );

	CGMToolOdbcConn* pDatabase = m_pShopDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	return pDatabase->ExecuteSQL(szTemp);
}

int CGMToolOdbcBase::UserBankWipe( std::string strUSER )
{
	if ( strUSER.size() <= 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[512] = {0};
	_snprintf( szTemp, 512, "DELETE FROM ShopPurchase WHERE UserUID = '%s'", strUSER.c_str() );
	
	CGMToolOdbcConn* pDatabase = m_pShopDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	return pDatabase->ExecuteSQL(szTemp);
}

int CGMToolOdbcBase::ShopGetData( SGMTOOL_SHOP_ITEM_MAP &mapResult )
{
	mapResult.clear();

	CGMToolOdbcConn* pDatabase = m_pShopDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "SELECT ProductNum, ItemMain, ItemSub, ItemName, ItemList, Duration, Category, ItemStock, ItemImage, ItemMoney, ItemComment, ItemSec FROM ShopItemMap WITH (NOLOCK)" );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	SQLRETURN	sReturn = 0;
	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS);

	SQLINTEGER nProductNum = 0, cbProductNum=SQL_NTS;
	SQLINTEGER nItemMain = 0, cbItemMain=SQL_NTS;
	SQLINTEGER nItemSub = 0, cbItemSub=SQL_NTS;
	SQLCHAR    szItemName[SHOP_ITEMNAME_LENGTH+1] = {0}; SQLINTEGER cbItemName = SQL_NTS;
	SQLINTEGER nItemList = 0, cbItemList =SQL_NTS;
	SQLCHAR    szDuration[SHOP_DURATION_LENGTH+1] = {0}; SQLINTEGER cbDuration = SQL_NTS;
	SQLINTEGER nCategory = 0, cbCategory =SQL_NTS;
	SQLINTEGER nItemStock = 0, cbItemStock =SQL_NTS;
	SQLCHAR    szItemImage[SHOP_ITEMIMAGE_LENGTH+1] = {0}; SQLINTEGER cbItemImage = SQL_NTS;
	SQLINTEGER nItemMoney = 0, cbItemMoney =SQL_NTS;
	SQLCHAR    szItemComment[SHOP_ITEMCOMMENT_LENGTH+1] = {0}; SQLINTEGER cbItemComment = SQL_NTS;
	SQLINTEGER nItemSec = 0, cbItemSec =SQL_NTS;

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}
	else
	{
		while(true)
		{
			sReturn = ::SQLFetch(pConn->hStmt);
			if ( sReturn == SQL_ERROR || sReturn == SQL_SUCCESS_WITH_INFO )
			{				
				break;
			}

			if ( sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO )
			{	
				::SQLGetData(pConn->hStmt,  1, SQL_C_LONG,	&nProductNum,   0, &cbProductNum );
				::SQLGetData(pConn->hStmt,  2, SQL_C_LONG,	&nItemMain,     0, &cbItemMain );
				::SQLGetData(pConn->hStmt,  3, SQL_C_LONG,	&nItemSub,      0, &cbItemSub );
				::SQLGetData(pConn->hStmt,  4, SQL_C_CHAR,  szItemName, SHOP_ITEMNAME_LENGTH, &cbItemName);
				::SQLGetData(pConn->hStmt,  5, SQL_C_LONG,	&nItemList,      0, &cbItemList );
				::SQLGetData(pConn->hStmt,  6, SQL_C_CHAR,  szDuration, SHOP_DURATION_LENGTH, &cbDuration);
				::SQLGetData(pConn->hStmt,  7, SQL_C_LONG,	&nCategory,      0, &cbCategory );
				::SQLGetData(pConn->hStmt,  8, SQL_C_LONG,	&nItemStock,     0, &cbItemStock );
				::SQLGetData(pConn->hStmt,  9, SQL_C_CHAR,  szItemImage, SHOP_ITEMIMAGE_LENGTH, &cbItemImage);
				::SQLGetData(pConn->hStmt,  10, SQL_C_LONG,	&nItemMoney,     0, &cbItemMoney );
				::SQLGetData(pConn->hStmt,  11, SQL_C_CHAR,  szItemComment, SHOP_ITEMCOMMENT_LENGTH, &cbItemComment);
				::SQLGetData(pConn->hStmt,  12, SQL_C_LONG,	&nItemSec,     0, &cbItemSec );

				SGMTOOL_SHOP_ITEM sDATA;
				sDATA.dwProductNum = (DWORD)nProductNum;
				sDATA.wItemMain = (WORD)nItemMain;
				sDATA.wItemSub = (WORD)nItemSub;
				sDATA.wItemList = (WORD)nItemList;
				sDATA.dwCategory = (DWORD)nCategory;
				sDATA.dwItemStock = (DWORD)nItemStock;
				sDATA.dwItemMoney = (DWORD)nItemMoney;
				sDATA.wItemSec = (WORD)nItemSec;

				if (cbItemName != 0 && cbItemName != -1)
					::StringCchCopy( sDATA.szItemName, SHOP_ITEMNAME_LENGTH+1, (const char*) szItemName);

				if (cbDuration != 0 && cbDuration != -1)
					::StringCchCopy( sDATA.szDuration, SHOP_DURATION_LENGTH+1, (const char*) szDuration);

				if (cbItemImage != 0 && cbItemImage != -1)
					::StringCchCopy( sDATA.szItemImage, SHOP_ITEMIMAGE_LENGTH+1, (const char*) szItemImage);

				if (cbItemComment != 0 && cbItemComment != -1)
					::StringCchCopy( sDATA.szItemComment, SHOP_ITEMCOMMENT_LENGTH+1, (const char*) szItemComment);

				mapResult.insert( std::make_pair( sDATA.dwProductNum, sDATA ) );
			}else{
				break;		
			}

			Sleep( 0 );
		}
	}

	pDatabase->FreeConnection(pConn);

	return DB_OK;
}

int CGMToolOdbcBase::UserBankInsert( std::string strUSER, DWORD dwPRODUCTNUM )
{
	if ( strUSER.size() <= 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	_snprintf( szTemp, 1024, 
		"INSERT Into ShopPurchase ( UserUID, ProductNum, PurFlag, PurDate, PurChgDate )"
		" Values ( '%s', %u, %u, '%s', '%s' )"
		, strUSER.c_str()
		, dwPRODUCTNUM
		, 0
		, _HLIB::cstring_timet24( CTime::GetCurrentTime().GetTime() ).GetString() 
		, _HLIB::cstring_timet24( CTime::GetCurrentTime().GetTime() ).GetString() 
		);

	CGMToolOdbcConn* pDatabase = m_pShopDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	if ( pDatabase->ExecuteSQL(szTemp) != DB_OK){
		return DB_ERROR;
	}

	return DB_OK;
}


int CGMToolOdbcBase::UserBankInsertAll( DWORD dwPRODUCTNUM )
{
	TCHAR szTemp[1024] = {0};

	_snprintf( szTemp, 1024, 
		"INSERT Into ShopPurchase ( UserUID, ProductNum, PurFlag, PurDate, PurChgDate )"
		" SELECT UserID, %u, %u, '%s', '%s' FROM RanUser.dbo.UserInfo "
		, dwPRODUCTNUM
		, 0
		, _HLIB::cstring_timet24( CTime::GetCurrentTime().GetTime() ).GetString() 
		, _HLIB::cstring_timet24( CTime::GetCurrentTime().GetTime() ).GetString() 
		);

	CGMToolOdbcConn* pDatabase = m_pShopDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	if ( pDatabase->ExecuteSQL(szTemp) != DB_OK){
		return DB_ERROR;
	}

	return DB_OK;
}

DWORD CGMToolOdbcBase::ShopItemNew( SGMTOOL_SHOP_ITEM* pSHOPITEM )
{
	if ( !pSHOPITEM )	return DB_ERROR;

	TCHAR szTemp[2048] = {0};

	_snprintf( szTemp, 2048, "INSERT Into ShopItemMap ( ItemMain, ItemSub, ItemName, ItemList, Duration, Category, ItemStock, ItemImage, ItemMoney, ItemComment )"
		" Values ( %u, %u, '%s', %u, '%s', %u, %u, '%s', %u, '%s' )"
		, pSHOPITEM->wItemMain
		, pSHOPITEM->wItemSub
		, pSHOPITEM->szItemName
		, pSHOPITEM->wItemList
		, pSHOPITEM->szDuration
		, pSHOPITEM->dwCategory
		, pSHOPITEM->dwItemStock
		, pSHOPITEM->szItemImage
		, pSHOPITEM->dwItemMoney
		, pSHOPITEM->szItemComment
		);

	CGMToolOdbcConn* pDatabase = m_pShopDB;
	if ( !pDatabase )	return DB_ERROR;

	SQLRETURN	sReturn = 0;

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn) return DB_ERROR;

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS);

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{		
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}

	if (sReturn == SQL_ERROR) 
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}
	
	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*) "SELECT cast(SCOPE_IDENTITY() as int)", SQL_NTS);

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}

	SQLINTEGER nNewNum=0, cbNewNum=SQL_NTS;

	while(true)
	{
		sReturn = ::SQLFetch(pConn->hStmt);
		if (sReturn == SQL_ERROR || sReturn == SQL_SUCCESS_WITH_INFO)
		{
			GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
			pDatabase->FreeConnection(pConn);

			return DB_ERROR;
		}
		if (sReturn == SQL_SUCCESS || sReturn == SQL_SUCCESS_WITH_INFO)
		{				
			::SQLGetData(pConn->hStmt, 1, SQL_C_LONG, &nNewNum, 0, &cbNewNum);
			pSHOPITEM->dwProductNum = nNewNum;			
		}
		else
		{
			break;
		}
		Sleep( 0 );
	}

	pDatabase->FreeConnection(pConn);

	return DB_OK;
}

int CGMToolOdbcBase::ShopItemUpdate( SGMTOOL_SHOP_ITEM* pSHOPITEM )
{
	if ( !pSHOPITEM )	return DB_ERROR;

	if ( pSHOPITEM->dwProductNum == SNATIVEID::ID_NULL )
		return DB_ERROR;

	TCHAR szTemp[2048] = {0};

	_snprintf( szTemp, 2048, 
		"UPDATE ShopItemMap Set ItemMain=%u, ItemSub=%u, ItemName='%s', "
		"ItemList=%u, Duration='%s', Category=%u, ItemStock=%u, "
		"ItemImage='%s', ItemMoney=%u, ItemComment='%s', ItemSec=%u "
		"WHERE ProductNum=%d", 
		pSHOPITEM->wItemMain, pSHOPITEM->wItemSub, pSHOPITEM->szItemName, 
		pSHOPITEM->wItemList, pSHOPITEM->szDuration, pSHOPITEM->dwCategory, pSHOPITEM->dwItemStock, 
		pSHOPITEM->szItemImage, pSHOPITEM->dwItemMoney, pSHOPITEM->szItemComment, pSHOPITEM->wItemSec,
		pSHOPITEM->dwProductNum );

	CGMToolOdbcConn* pDatabase = m_pShopDB;
	if ( !pDatabase )	return DB_ERROR;

	if ( pDatabase->ExecuteSQL(szTemp) != DB_OK){
		return DB_ERROR;
	}

	return DB_OK;
}

int CGMToolOdbcBase::ShopItemDelete( SGMTOOL_SHOP_ITEM* pSHOPITEM )
{
	if ( !pSHOPITEM )	return DB_ERROR;

	if ( pSHOPITEM->dwProductNum == SNATIVEID::ID_NULL )
		return DB_ERROR;

	TCHAR szTemp1[1024] = {0};
	_snprintf( szTemp1, 1024, "DELETE FROM ShopItemMap WHERE ProductNum=%d AND ItemMain=%d AND ItemSub=%d", 
		pSHOPITEM->dwProductNum, pSHOPITEM->wItemMain, pSHOPITEM->wItemSub );

	TCHAR szTemp2[1024] = {0};
	_snprintf( szTemp2, 1024, "DELETE FROM ShopPurchase WHERE ProductNum=%d", 
		pSHOPITEM->dwProductNum );

	CGMToolOdbcConn* pDatabase = m_pShopDB;
	if ( !pDatabase )	return DB_ERROR;

	if ( pDatabase->ExecuteSQL(szTemp1) != DB_OK){
		return DB_ERROR;
	}

	if ( pDatabase->ExecuteSQL(szTemp2) != DB_OK){
		return DB_ERROR;
	}

	return DB_OK;
}