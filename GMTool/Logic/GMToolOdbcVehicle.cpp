#include "stdAfx.h"
#include "GMToolOdbcBase.h"
#include "GMToolOdbcConn.h"
#include "GMToolLogs.h"

#include "GLCharDefine.h"
#include "GLCharData.h"
#include "GLVEHICLE.h"
#include "HLibDataConvert.h"
#include "HLibTimeFunctions.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CGMToolOdbcBase::VehicleFind( const char* szSQL, SGMTOOL_VEHICLEDATA_SIMPLE_MAP &mapResult )
{
	mapResult.clear();

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "%s", szSQL );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	SQLRETURN sReturn = 0;
	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*)szTemp, SQL_NTS );

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{	
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}

	SQLINTEGER nVehicleNum = 0, cbVehicleNum=SQL_NTS;
	SQLINTEGER nVehicleChaNum = 0, cbVehicleChaNum=SQL_NTS;
	SQLCHAR    szVehicleName[VEHICLE_NAME_LENGTH+1] = {0}; SQLINTEGER cbVehicleName = SQL_NTS;
	SQLINTEGER nVehicleCardMID = 0, cbVehicleCardMID=SQL_NTS;
	SQLINTEGER nVehicleCardSID = 0, cbVehicleCardSID=SQL_NTS;
	SQLINTEGER nVehicleType = 0, cbVehicleType=SQL_NTS;

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
			::SQLGetData(pConn->hStmt,  1, SQL_C_LONG,	&nVehicleNum,    0, &cbVehicleNum);
			::SQLGetData(pConn->hStmt,  2, SQL_C_LONG,	&nVehicleChaNum,     0, &cbVehicleChaNum);
			::SQLGetData(pConn->hStmt,  3, SQL_C_CHAR,  szVehicleName,    VEHICLE_NAME_LENGTH, &cbVehicleName);
			::SQLGetData(pConn->hStmt,  4, SQL_C_LONG,	&nVehicleCardMID,   0, &cbVehicleCardMID);
			::SQLGetData(pConn->hStmt,  5, SQL_C_LONG,	&nVehicleCardSID,   0, &cbVehicleCardSID);
			::SQLGetData(pConn->hStmt,  6, SQL_C_LONG,	&nVehicleType,  0, &cbVehicleType);

			if ( nVehicleType >= VEHICLE_TYPE_SIZE ){
				GMTOOL_LOGS::LogError( "VehicleNum :%d  Vehicle type(%d) not supported, reading skipped", nVehicleNum, nVehicleType );
				continue;
			}

			SGMTOOL_VEHICLEDATA_SIMPLE sDATA;

			if (cbVehicleName != 0 && cbVehicleName != -1)
				::StringCchCopy(sDATA.szVehicleName, VEHICLE_NAME_LENGTH+1, (const TCHAR*) szVehicleName);

			sDATA.dwVehicleNum = (DWORD)nVehicleNum;
			sDATA.dwVehicleChaNum = (DWORD)nVehicleChaNum;
			sDATA.sIDVEHICLE.wMainID = (WORD)nVehicleCardMID;
			sDATA.sIDVEHICLE.wSubID = (WORD)nVehicleCardSID;
			sDATA.wVehicleType = (WORD)nVehicleType;

			mapResult.insert( std::make_pair( sDATA.dwVehicleNum, sDATA ) );
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

int CGMToolOdbcBase::VehicleFind_Name( std::string strVehicleName, SGMTOOL_VEHICLEDATA_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( strVehicleName.size() <=0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	std::string strSign = "%";
	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT VehicleNum, VehicleChaNum, VehicleName, VehicleCardMID, VehicleCardSID, VehicleType FROM VehicleInfo WITH (NOLOCK) WHERE VehicleName LIKE '%s%s%s' AND VehicleDeleted = 0"
			"ORDER BY VehicleNum", strSign.c_str(), strVehicleName.c_str(), strSign.c_str() );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d VehicleNum, VehicleChaNum, VehicleName, VehicleCardMID, VehicleCardSID, VehicleType FROM VehicleInfo WITH (NOLOCK) WHERE VehicleName LIKE '%s%s%s' AND VehicleDeleted = 0"
			"ORDER BY VehicleNum", nLIMITNUM, strSign.c_str(), strVehicleName.c_str(), strSign.c_str() );
	}

	return VehicleFind( szTemp, mapResult );
}

int CGMToolOdbcBase::VehicleFind_VehicleID( int nVehicleID, SGMTOOL_VEHICLEDATA_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nVehicleID < 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT VehicleNum, VehicleChaNum, VehicleName, VehicleCardMID, VehicleCardSID, VehicleType FROM VehicleInfo WITH (NOLOCK) WHERE VehicleNum = %d AND VehicleDeleted = 0 ORDER BY VehicleNum", nVehicleID );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d VehicleNum, VehicleChaNum, VehicleName, VehicleCardMID, VehicleCardSID, VehicleType FROM VehicleInfo WITH (NOLOCK) WHERE VehicleNum = %d AND VehicleDeleted = 0 ORDER BY VehicleNum", nLIMITNUM, nVehicleID );
	}

	return VehicleFind( szTemp, mapResult );
}

int CGMToolOdbcBase::VehicleFind_CharID( int nCharID, SGMTOOL_VEHICLEDATA_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nCharID < 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT VehicleNum, VehicleChaNum, VehicleName, VehicleCardMID, VehicleCardSID, VehicleType FROM VehicleInfo WITH (NOLOCK) WHERE VehicleChaNum = %d AND VehicleDeleted = 0 ORDER BY VehicleNum", nCharID );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d VehicleNum, VehicleChaNum, VehicleName, VehicleCardMID, VehicleCardSID, VehicleType FROM VehicleInfo WITH (NOLOCK) WHERE VehicleChaNum = %d AND VehicleDeleted = 0 ORDER BY VehicleNum", nLIMITNUM, nCharID );
	}

	return VehicleFind( szTemp, mapResult );
}

int CGMToolOdbcBase::VehicleFind_MID( int nMID, SGMTOOL_VEHICLEDATA_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nMID < 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT VehicleNum, VehicleChaNum, VehicleName, VehicleCardMID, VehicleCardSID, VehicleType FROM VehicleInfo WITH (NOLOCK) WHERE VehicleCardMID = %d AND VehicleDeleted = 0 ORDER BY VehicleNum", nMID );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d VehicleNum, VehicleChaNum, VehicleName, VehicleCardMID, VehicleCardSID, VehicleType FROM VehicleInfo WITH (NOLOCK) WHERE VehicleCardMID = %d AND VehicleDeleted = 0 ORDER BY VehicleNum", nLIMITNUM, nMID );
	}

	return VehicleFind( szTemp, mapResult );
}

int CGMToolOdbcBase::VehicleFind_SID( int nSID, SGMTOOL_VEHICLEDATA_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nSID < 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT VehicleNum, VehicleChaNum, VehicleName, VehicleCardMID, VehicleCardSID, VehicleType FROM VehicleInfo WITH (NOLOCK) WHERE VehicleCardSID = %d AND VehicleDeleted = 0 ORDER BY VehicleNum", nSID );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d VehicleNum, VehicleChaNum, VehicleName, VehicleCardMID, VehicleCardSID, VehicleType FROM VehicleInfo WITH (NOLOCK) WHERE VehicleCardSID = %d AND VehicleDeleted = 0 ORDER BY VehicleNum", nLIMITNUM, nSID );
	}

	return VehicleFind( szTemp, mapResult );
}

int CGMToolOdbcBase::VehicleFind_Type( int nTYPE, SGMTOOL_VEHICLEDATA_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nTYPE < 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT VehicleNum, VehicleChaNum, VehicleName, VehicleCardMID, VehicleCardSID, VehicleType FROM VehicleInfo WITH (NOLOCK) WHERE VehicleType = %d AND VehicleDeleted = 0 ORDER BY VehicleNum", nTYPE);
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d VehicleNum, VehicleChaNum, VehicleName, VehicleCardMID, VehicleCardSID, VehicleType FROM VehicleInfo WITH (NOLOCK) WHERE VehicleType = %d AND VehicleDeleted = 0 ORDER BY VehicleNum", nLIMITNUM, nTYPE);
	}

	return VehicleFind( szTemp, mapResult );
}


int CGMToolOdbcBase::VehicleDataGet( DWORD dwVEHICLENUM, DWORD dwVEHICLEOWNERNUM, GLVEHICLE* pDATA )
{
	if (pDATA == NULL) {
		return DB_ERROR;
	}

	if ( dwVEHICLENUM <= 0 ){
		return DB_ERROR;
	}

	if ( dwVEHICLEOWNERNUM <= 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 256, "SELECT VehicleName, VehicleChaNum, VehicleType, VehicleCardMID, VehicleCardSID, VehicleBattery, VehicleBooster "
		"FROM VehicleInfo WHERE VehicleNum=%u And VehicleChaNum=%u", dwVEHICLENUM, dwVEHICLEOWNERNUM );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	SQLRETURN sReturn = 0;
	int		  nRowCount = 0;
	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*) szTemp, SQL_NTS);

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
		pDatabase->FreeConnection(pConn);

		return DB_ERROR;
	}

	SQLCHAR    szVehicleName[VEHICLE_NAME_LENGTH+1] = {0}; SQLINTEGER cbVehicleName = SQL_NTS;
	SQLINTEGER nOwnerNum = 0, cbOwnerNum = SQL_NTS;	
	SQLINTEGER nType = 0, cbType = SQL_NTS;
	SQLINTEGER nVehicleCardMID = 0, cbVehicleCardMID = SQL_NTS;
	SQLINTEGER nVehicleCardSID = 0, cbVehicleCardSID = SQL_NTS;
	SQLINTEGER nBattery = 0, cbBattery = SQL_NTS;
	SQLINTEGER nBooster = 0, cbBooster = SQL_NTS; /*vehicle booster system, Juver, 2017/08/10 */

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
			::SQLGetData(pConn->hStmt, 1, SQL_C_CHAR, szVehicleName, VEHICLE_NAME_LENGTH+1, &cbVehicleName);
			::SQLGetData(pConn->hStmt, 2, SQL_C_LONG, &nOwnerNum, 0, &cbOwnerNum);
			::SQLGetData(pConn->hStmt, 3, SQL_C_LONG, &nType, 0, &cbType);
			::SQLGetData(pConn->hStmt, 4, SQL_C_LONG, &nVehicleCardMID, 0, &cbVehicleCardMID);
			::SQLGetData(pConn->hStmt, 5, SQL_C_LONG, &nVehicleCardSID, 0, &cbVehicleCardSID);
			::SQLGetData(pConn->hStmt, 6, SQL_C_LONG, &nBattery, 0, &cbBattery);
			::SQLGetData(pConn->hStmt, 7, SQL_C_LONG, &nBooster, 0, &cbBooster); /*vehicle booster system, Juver, 2017/08/10 */

			pDATA->m_dwOwner				= (DWORD) nOwnerNum;
			pDATA->m_sVehicleID.wMainID		= (WORD) nVehicleCardMID;
			pDATA->m_sVehicleID.wSubID		= (WORD) nVehicleCardSID;
			pDATA->m_emTYPE					= VEHICLE_TYPE(nType);			
			pDATA->m_nFull					= nBattery;
			pDATA->m_bBooster				= (nBooster == 1)? true:false; /*vehicle booster system, Juver, 2017/08/10 */
			nRowCount++;
		}
		else
		{
			break;
		}
		Sleep( 0 );
	}

	pDatabase->FreeConnection(pConn);	

	if (nRowCount != 1){ 
		return DB_ERROR;
	}

	pDATA->m_dwGUID = dwVEHICLENUM;

	if ( VehicleDataGetInven( dwVEHICLENUM, dwVEHICLEOWNERNUM, pDATA ) == DB_ERROR){
		return DB_ERROR;	
	}

	/*bike color , Juver, 2017/11/13 */
	if ( VehicleDataGetColor( dwVEHICLENUM, dwVEHICLEOWNERNUM, pDATA ) == DB_ERROR){
		return DB_ERROR;	
	}

	return DB_OK;
}

int CGMToolOdbcBase::VehicleDataGetInven( DWORD dwVEHICLENUM, DWORD dwVEHICLEOWNERNUM, GLVEHICLE* pDATA )
{
	if ( pDATA == NULL ){
		return DB_ERROR;
	}

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase )	return DB_ERROR;

	CByteStream ByteStream;
	int nReturn = 0;

	nReturn = pDatabase->ReadImageVehicle("VehicleInfo.VehiclePutOnItems", dwVEHICLENUM, dwVEHICLEOWNERNUM, ByteStream);

	if (nReturn == DB_ERROR)
	{
		return DB_ERROR;
	}else {
		SETPUTONITEMS_BYBUF( pDATA->m_PutOnItems, ByteStream );
		return DB_OK;
	}
}

int CGMToolOdbcBase::VehicleDataSave( GLVEHICLE* pDATA )
{
	if ( !pDATA ){
		return DB_ERROR;
	}

	DWORD dwVEHICLENUM = pDATA->m_dwGUID;
	DWORD dwVEHICLECHANUM = pDATA->m_dwOwner;

	if ( dwVEHICLENUM <= 0 ){
		return DB_ERROR;
	}

	if ( dwVEHICLECHANUM <= 0 ){
		return DB_ERROR;
	}

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "UPDATE VehicleInfo SET VehicleType=%d, VehicleBattery=%d, VehicleBooster=%d "
		"WHERE VehicleNum=%u And VehicleChaNum=%u", 
		(int)pDATA->m_emTYPE, 
		pDATA->m_nFull,
		pDATA->m_bBooster, /*vehicle booster system, Juver, 2017/08/10 */
		dwVEHICLENUM, 
		dwVEHICLECHANUM );		

	if (pDatabase->ExecuteSQL(szTemp) != DB_OK)	{
		return DB_ERROR;
	}

	//vehicle inven
	CByteStream ByteStream;
	pDATA->GETPUTONITEMS_BYBUF( ByteStream );
	int nReturn = VehicleDataSaveInven( dwVEHICLENUM, dwVEHICLECHANUM, ByteStream );
	if( nReturn != DB_OK )	return nReturn;

	/*bike color , Juver, 2017/11/13 */
	ByteStream.ClearBuffer();
	pDATA->GETCOLORS_BYBUF( ByteStream );
	nReturn = VehicleDataSaveColor( dwVEHICLENUM, dwVEHICLECHANUM, ByteStream );
	if( nReturn != DB_OK )	return nReturn;

	return DB_OK;
}

int CGMToolOdbcBase::VehicleDataSaveInven( int nVEHICLENUM, int nVEHICLECHANUM, CByteStream &ByteStream )
{
	if ( nVEHICLENUM <= 0 )
		return DB_ERROR;

	if ( nVEHICLECHANUM <= 0 )
		return DB_ERROR;

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	LPBYTE pBuffer = NULL;
	DWORD dwSize = 0;
	int nResult = 0;
	ByteStream.GetBuffer( pBuffer, dwSize );
	if (pBuffer != NULL)
	{
		std::strstream strnVehicleInven;
		strnVehicleInven << _T("UPDATE VehicleInfo SET VehicleInfo.VehiclePutOnItems=?");
		strnVehicleInven <<" WHERE (VehicleNum=" <<  nVEHICLENUM << ") And( VehicleChaNum="<< nVEHICLECHANUM << ")";
		strnVehicleInven << std::ends;
		nResult = pDatabase->WriteImage( strnVehicleInven, (DWORD) nVEHICLENUM, (BYTE *) pBuffer, dwSize );

		strnVehicleInven.freeze( false );
		pBuffer = NULL;
		return nResult;
	}
	else
	{
		return DB_ERROR;
	}
}

/*bike color , Juver, 2017/11/13 */
int CGMToolOdbcBase::VehicleDataGetColor( DWORD dwVEHICLENUM, DWORD dwVEHICLEOWNERNUM, GLVEHICLE* pDATA )
{
	if ( pDATA == NULL ){
		return DB_ERROR;
	}

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase )	return DB_ERROR;

	CByteStream ByteStream;
	int nReturn = 0;

	nReturn = pDatabase->ReadImageVehicle("VehicleInfo.VehicleColor", dwVEHICLENUM, dwVEHICLEOWNERNUM, ByteStream);

	if (nReturn == DB_ERROR)
	{
		return DB_ERROR;
	}else {
		pDATA->SETCOLORS_BYBUF( ByteStream );
		return DB_OK;
	}
}

/*bike color , Juver, 2017/11/13 */
int CGMToolOdbcBase::VehicleDataSaveColor( int nVEHICLENUM, int nVEHICLECHANUM, CByteStream &ByteStream )
{
	if ( nVEHICLENUM <= 0 )
		return DB_ERROR;

	if ( nVEHICLECHANUM <= 0 )
		return DB_ERROR;

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	LPBYTE pBuffer = NULL;
	DWORD dwSize = 0;
	int nResult = 0;
	ByteStream.GetBuffer( pBuffer, dwSize );
	if (pBuffer != NULL)
	{
		std::strstream strnVehicleColor;
		strnVehicleColor << _T("UPDATE VehicleInfo SET VehicleInfo.VehicleColor=?");
		strnVehicleColor <<" WHERE (VehicleNum=" <<  nVEHICLENUM << ") And( VehicleChaNum="<< nVEHICLECHANUM << ")";
		strnVehicleColor << std::ends;
		nResult = pDatabase->WriteImage( strnVehicleColor, (DWORD) nVEHICLENUM, (BYTE *) pBuffer, dwSize );

		strnVehicleColor.freeze( false );
		pBuffer = NULL;
		return nResult;
	}
	else
	{
		return DB_ERROR;
	}
}