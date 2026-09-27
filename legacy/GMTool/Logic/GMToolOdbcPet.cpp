#include "stdAfx.h"
#include "GMToolOdbcBase.h"
#include "GMToolOdbcConn.h"
#include "GMToolLogs.h"

#include "GLCharDefine.h"
#include "GLCharData.h"
#include "GLPet.h"
#include "HLibDataConvert.h"
#include "HLibTimeFunctions.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CGMToolOdbcBase::PetFind( const char* szSQL, SGMTOOL_PETDATA_SIMPLE_MAP &mapResult )
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

	SQLINTEGER nPetNum = 0, cbPetNum=SQL_NTS;
	SQLINTEGER nPetChaNum = 0, cbPetChaNum=SQL_NTS;
	SQLCHAR    szPetName[PET_NAME_LENGTH+1] = {0}; SQLINTEGER cbPetName = SQL_NTS;
	SQLINTEGER nPetCardMID = 0, cbPetCardMID=SQL_NTS;
	SQLINTEGER nPetCardSID = 0, cbPetCardSID=SQL_NTS;
	SQLINTEGER nPetType = 0, cbPetType=SQL_NTS;

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
			::SQLGetData(pConn->hStmt,  1, SQL_C_LONG,	&nPetNum,    0, &cbPetNum);
			::SQLGetData(pConn->hStmt,  2, SQL_C_LONG,	&nPetChaNum, 0, &cbPetChaNum);
			::SQLGetData(pConn->hStmt,  3, SQL_C_CHAR,  szPetName,    PET_NAME_LENGTH, &cbPetName);
			::SQLGetData(pConn->hStmt,  4, SQL_C_LONG,	&nPetCardMID, 0, &cbPetCardMID);
			::SQLGetData(pConn->hStmt,  5, SQL_C_LONG,	&nPetCardSID, 0, &cbPetCardSID);
			::SQLGetData(pConn->hStmt,  6, SQL_C_LONG,	&nPetType,  0, &cbPetType);

			if ( nPetType >= PET_TYPE_SIZE ){
				GMTOOL_LOGS::LogError( "PetNum :%d Pet type(%d) not supported, reading skipped", nPetNum, nPetType );
				continue;
			}

			SGMTOOL_PETDATA_SIMPLE sDATA;

			if (cbPetName != 0 && cbPetName != -1)
				::StringCchCopy(sDATA.szPetName, PET_NAME_LENGTH+1, (const TCHAR*) szPetName);

			sDATA.dwPetNum = (DWORD)nPetNum;
			sDATA.dwPetChaNum = (DWORD)nPetChaNum;
			sDATA.sIDPET.wMainID = (WORD)nPetCardMID;
			sDATA.sIDPET.wSubID = (WORD)nPetCardSID;
			sDATA.wPetType = (WORD)nPetType;

			mapResult.insert( std::make_pair( sDATA.dwPetNum, sDATA ) );
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

int CGMToolOdbcBase::PetFind_Name( std::string strPETNAME, SGMTOOL_PETDATA_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( strPETNAME.size() <=0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	std::string strSign = "%";
	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT PetNum, PetChaNum, PetName, PetMID, PetSID, PetType FROM PetInfo WITH (NOLOCK) WHERE PetName LIKE '%s%s%s' AND PetDeleted = 0 ORDER BY PetNum", 
			strSign.c_str(), strPETNAME.c_str(), strSign.c_str() );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d PetNum, PetChaNum, PetName, PetMID, PetSID, PetType FROM PetInfo WITH (NOLOCK) WHERE PetName LIKE '%s%s%s' AND PetDeleted = 0 ORDER BY PetNum", 
			nLIMITNUM, strSign.c_str(), strPETNAME.c_str(), strSign.c_str() );
	}
	
	return PetFind( szTemp, mapResult );
}

int CGMToolOdbcBase::PetFind_PetID( int nPETID, SGMTOOL_PETDATA_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nPETID < 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT PetNum, PetChaNum, PetName, PetMID, PetSID, PetType FROM PetInfo WITH (NOLOCK) WHERE PetNum = %d AND PetDeleted = 0 ORDER BY PetNum", nPETID );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d PetNum, PetChaNum, PetName, PetMID, PetSID, PetType FROM PetInfo WITH (NOLOCK) WHERE PetNum = %d AND PetDeleted = 0 ORDER BY PetNum", nLIMITNUM, nPETID );
	}

	return PetFind( szTemp, mapResult );
}

int CGMToolOdbcBase::PetFind_CharID( int nCHARID, SGMTOOL_PETDATA_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nCHARID < 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT PetNum, PetChaNum, PetName, PetMID, PetSID, PetType FROM PetInfo WITH (NOLOCK) WHERE PetChaNum = %d AND PetDeleted = 0 ORDER BY PetNum", nCHARID );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d PetNum, PetChaNum, PetName, PetMID, PetSID, PetType FROM PetInfo WITH (NOLOCK) WHERE PetChaNum = %d AND PetDeleted = 0 ORDER BY PetNum", nLIMITNUM, nCHARID );
	}

	return PetFind( szTemp, mapResult );
}

int CGMToolOdbcBase::PetFind_MID( int nMID, SGMTOOL_PETDATA_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nMID < 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT PetNum, PetChaNum, PetName, PetMID, PetSID, PetType FROM PetInfo WITH (NOLOCK) WHERE PetMID = %d AND PetDeleted = 0 ORDER BY PetNum", nMID );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d PetNum, PetChaNum, PetName, PetMID, PetSID, PetType FROM PetInfo WITH (NOLOCK) WHERE PetMID = %d AND PetDeleted = 0 ORDER BY PetNum", nLIMITNUM, nMID );
	}

	return PetFind( szTemp, mapResult );
}

int CGMToolOdbcBase::PetFind_SID( int nSID, SGMTOOL_PETDATA_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nSID < 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT PetNum, PetChaNum, PetName, PetMID, PetSID, PetType FROM PetInfo WITH (NOLOCK) WHERE PetSID = %d AND PetDeleted = 0 ORDER BY PetNum", nSID );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d PetNum, PetChaNum, PetName, PetMID, PetSID, PetType FROM PetInfo WITH (NOLOCK) WHERE PetSID = %d AND PetDeleted = 0 ORDER BY PetNum", nLIMITNUM, nSID );
	}

	return PetFind( szTemp, mapResult );
}

int CGMToolOdbcBase::PetFind_Type( int nTYPE, SGMTOOL_PETDATA_SIMPLE_MAP &mapResult, int nLIMITNUM )
{
	if ( nTYPE < 0 ){
		return DB_ERROR;
	}

	if ( nLIMITNUM < 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};

	if ( nLIMITNUM == 0 ){
		_snprintf( szTemp, 1024, "SELECT PetNum, PetChaNum, PetName, PetMID, PetSID, PetType FROM PetInfo WITH (NOLOCK) WHERE PetType = %d AND PetDeleted = 0 ORDER BY PetNum", nTYPE );
	}
	else{
		_snprintf( szTemp, 1024, "SELECT TOP %d PetNum, PetChaNum, PetName, PetMID, PetSID, PetType FROM PetInfo WITH (NOLOCK) WHERE PetType = %d AND PetDeleted = 0 ORDER BY PetNum", nLIMITNUM, nTYPE );
	}

	return PetFind( szTemp, mapResult );
}

int CGMToolOdbcBase::PetDataGet( DWORD dwPETID, DWORD dwPETOWNERID, GLPET* pDATA )
{
	if ( pDATA == NULL){
		return DB_ERROR;
	}

	if ( dwPETID <= 0 ){
		return DB_ERROR;
	}

	if ( dwPETOWNERID <= 0 ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 256, "SELECT PetName, PetChaNum, PetType, PetMID, PetSID, PetCardMID, PetCardSID, PetStyle, PetColor, PetFull, PetSkinMID, PetSkinSID, PetSkinScale, PetSkinTime, PetSkinStartDate, PetDualSkill "
		"FROM PetInfo WHERE PetNum=%u And PetChaNum=%u", dwPETID, dwPETOWNERID );

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

	SQLCHAR szPetName[PETNAMESIZE+1] = {0}; SQLINTEGER cbPetName = SQL_NTS;
	SQLINTEGER nOwnerNum = 0, cbOwnerNum = SQL_NTS;
	SQLINTEGER nType = 0, cbType = SQL_NTS;
	SQLINTEGER nMainID = 0, cbMainID = SQL_NTS;
	SQLINTEGER nSubID = 0, cbSubID = SQL_NTS;
	SQLINTEGER nPetCardMID = 0, cbPetCardMID = SQL_NTS;
	SQLINTEGER nPetCardSID = 0, cbPetCardSID = SQL_NTS;
	SQLINTEGER nStyle = 0, cbStyle = SQL_NTS;
	SQLINTEGER nColor = 0, cbColor = SQL_NTS;
	SQLINTEGER nFull = 0, cbFull = SQL_NTS;

	SQLINTEGER nPetSkinMID = 0, cbPetSkinMID = SQL_NTS;
	SQLINTEGER nPetSkinSID = 0, cbPetSkinSID = SQL_NTS;
	SQLINTEGER nPetSkinScale = 0, cbPetSkinScale = SQL_NTS;
	SQLINTEGER nPetSkinTime = 0, cbPetSkinTime = SQL_NTS;
	TIMESTAMP_STRUCT sPetSkinStartDate; SQLINTEGER cbPetSkinStartDate = SQL_NTS;

	/*dual pet skill, Juver, 2017/12/29 */
	SQLINTEGER nDualSkill = 0, cbDualSkill = SQL_NTS;

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
			::SQLGetData(pConn->hStmt, 1, SQL_C_CHAR, szPetName, PETNAMESIZE+1, &cbPetName);
			::SQLGetData(pConn->hStmt, 2, SQL_C_LONG, &nOwnerNum, 0, &cbOwnerNum);
			::SQLGetData(pConn->hStmt, 3, SQL_C_LONG, &nType, 0, &cbType);	
			::SQLGetData(pConn->hStmt, 4, SQL_C_LONG, &nMainID, 0, &cbMainID);
			::SQLGetData(pConn->hStmt, 5, SQL_C_LONG, &nSubID, 0, &cbSubID);
			::SQLGetData(pConn->hStmt, 6, SQL_C_LONG, &nPetCardMID, 0, &cbPetCardMID);
			::SQLGetData(pConn->hStmt, 7, SQL_C_LONG, &nPetCardSID, 0, &cbPetCardSID);
			::SQLGetData(pConn->hStmt, 8, SQL_C_LONG, &nStyle, 0, &cbStyle);
			::SQLGetData(pConn->hStmt, 9, SQL_C_LONG, &nColor, 0, &cbColor);
			::SQLGetData(pConn->hStmt, 10, SQL_C_LONG, &nFull, 0, &cbFull);
			::SQLGetData(pConn->hStmt, 11, SQL_C_LONG, &nPetSkinMID, 0, &cbPetSkinMID);
			::SQLGetData(pConn->hStmt, 12, SQL_C_LONG, &nPetSkinSID, 0, &cbPetSkinSID);
			::SQLGetData(pConn->hStmt, 13, SQL_C_LONG, &nPetSkinScale, 0, &cbPetSkinScale);
			::SQLGetData(pConn->hStmt, 14, SQL_C_LONG, &nPetSkinTime, 0, &cbPetSkinTime);
			::SQLGetData(pConn->hStmt, 15, SQL_C_TYPE_TIMESTAMP, &sPetSkinStartDate, 0, &cbPetSkinStartDate);

			/*dual pet skill, Juver, 2017/12/29 */
			::SQLGetData(pConn->hStmt, 16, SQL_C_LONG, &nDualSkill, 0, &cbDualSkill);

			if (cbPetName != 0 && cbPetName != -1)
				::StringCchCopy(pDATA->m_szName, PETNAMESIZE+1, (const char*) szPetName);

			pDATA->m_dwOwner							= (DWORD) nOwnerNum;
			pDATA->m_emTYPE								= PETTYPE(nType);
			pDATA->m_sPetID.wMainID						= (WORD) nMainID;
			pDATA->m_sPetID.wSubID						= (WORD) nSubID;
			pDATA->m_sPetCardID.wMainID					= (WORD) nPetCardMID;
			pDATA->m_sPetCardID.wSubID					= (WORD) nPetCardSID;
			pDATA->m_wStyle								= (WORD) nStyle;
			pDATA->m_wColor								= (WORD) nColor;
			pDATA->m_nFull								= nFull;
			pDATA->m_sPetSkinPackData.dwPetSkinTime		= (DWORD) nPetSkinTime;
			pDATA->m_sPetSkinPackData.sMobID.wMainID	= (WORD) nPetSkinMID;
			pDATA->m_sPetSkinPackData.sMobID.wSubID		= (WORD) nPetSkinSID;
			pDATA->m_sPetSkinPackData.fScale			= (float)nPetSkinScale / 100.0f;		

			/*dual pet skill, Juver, 2017/12/29 */
			pDATA->m_bDualSkill							= (BOOL)nDualSkill;

			if ( !_HLIB::timestamp_correction( sPetSkinStartDate ) ){
				GMTOOL_LOGS::LogError( "Variable sPetSkinStartDate value corrected" );
			}

			CTime cTemp(sPetSkinStartDate.year, sPetSkinStartDate.month, sPetSkinStartDate.day, 
				sPetSkinStartDate.hour, sPetSkinStartDate.minute, sPetSkinStartDate.second);

			pDATA->m_sPetSkinPackData.startTime = cTemp.GetTime();

			if( pDATA->m_sPetSkinPackData.dwPetSkinTime == 0 )
			{
				pDATA->m_sPetSkinPackData.Init();
			}else {
				CTime currentTime = CTime::GetCurrentTime();
				CTimeSpan timeSpan = currentTime - pDATA->m_sPetSkinPackData.startTime;
				if( timeSpan.GetTotalSeconds() >= pDATA->m_sPetSkinPackData.dwPetSkinTime )
				{
					pDATA->m_sPetSkinPackData.Init();
				}else{
					pDATA->m_sPetSkinPackData.bUsePetSkinPack = TRUE;
				}
			}

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

	pDATA->m_dwPetID = dwPETID;

	if ( PetDataGetInven( dwPETID, dwPETOWNERID, pDATA ) == DB_ERROR){
		return DB_ERROR;
	}

	if ( PetDataGetSkill( dwPETID, dwPETOWNERID, pDATA ) == DB_ERROR){
		return DB_ERROR;	
	}

	return DB_OK;
}

int CGMToolOdbcBase::PetDataGetInven( DWORD dwPETID, DWORD dwPETOWNERID, GLPET* pDATA )
{
	if ( pDATA == NULL){
		return DB_ERROR;
	}

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	CByteStream ByteStream;
	int nReturn = 0;

	nReturn = pDatabase->ReadImagePet("PetInfo.PetPutOnItems", dwPETOWNERID, dwPETID, ByteStream);

	if (nReturn == DB_ERROR)
	{
		return DB_ERROR;
	}
	else 
	{
		SETPUTONITEMS_BYBUF( pDATA->m_PutOnItems, ByteStream );
		return DB_OK;
	}
}

int CGMToolOdbcBase::PetDataGetSkill( DWORD dwPETID, DWORD dwPETOWNERID, GLPET* pDATA )
{
	if ( pDATA == NULL){ 
		return DB_ERROR;
	}

	TCHAR szTemp[256] = {0};
	_snprintf( szTemp, 256, "SELECT PetInvenMID, PetInvenSID, PetInvenAvailable "
		" FROM PetInven WHERE PetNum=%u AND PetInvenType=%d AND PetChaNum=%d ORDER BY PetInvenNum",
		dwPETID,
		DB_PET_INVEN_TYPE::SKILL,
		dwPETOWNERID);

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
	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*) szTemp, SQL_NTS);

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}

	SQLINTEGER nAvailable = 0, cbAvailable = SQL_NTS;
	SQLINTEGER nMainID = 0, cbMainID = SQL_NTS;
	SQLINTEGER nSubID = 0, cbSubID = SQL_NTS;

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
			::SQLGetData(pConn->hStmt, 1, SQL_C_SHORT, &nMainID, 0, &cbMainID);
			::SQLGetData(pConn->hStmt, 2, SQL_C_SHORT, &nSubID, 0, &cbSubID);
			::SQLGetData(pConn->hStmt, 3, SQL_C_TINYINT, &nAvailable, 0, &cbAvailable);

			PETSKILL PetSkill;
			PetSkill.sNativeID.wMainID = (WORD) nMainID;
			PetSkill.sNativeID.wSubID  = (WORD) nSubID;

			pDATA->m_ExpSkills.insert( std::make_pair( PetSkill.sNativeID.dwID, PetSkill ) );

			/*dual pet skill, Juver, 2017/12/29 */
			if (nAvailable == 1)
				pDATA->m_sActiveSkillID_A = PetSkill.sNativeID;

			/*dual pet skill, Juver, 2017/12/29 */
			if (nAvailable == 2)
				pDATA->m_sActiveSkillID_B = PetSkill.sNativeID;
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

int CGMToolOdbcBase::PetDataSave( GLPET* pDATA )
{
	if ( !pDATA ){
		return DB_ERROR;
	}

	DWORD dwPETNUM = pDATA->m_dwPetID;
	DWORD dwPETCHANUM = pDATA->m_dwOwner;

	if ( dwPETNUM == 0 ){
		return DB_ERROR;
	}

	if ( dwPETCHANUM == 0 ){
		return DB_ERROR;
	}

	CString strName (pDATA->m_szName);
	strName.Trim(_T(" "));
	strName.Replace(_T("'"), _T("''"));

	if (strName.GetLength() > (PETNAMESIZE)){
		return DB_ERROR;
	}
	
	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "UPDATE PetInfo SET PetName='%s', PetType=%d, PetStyle=%u, PetColor=%u, PetFull=%d, "
		"PetSkinMID=%u, PetSkinSID=%u, PetSkinScale=%f, PetSkinTime=%u, PetSkinStartDate='%s', PetDualSkill=%d "
		"WHERE PetNum=%u And PetChaNum=%u", 
		strName.GetString(), 
		(int)pDATA->m_emTYPE, 
		pDATA->m_wStyle,
		pDATA->m_wColor, 
		pDATA->m_nFull,
		pDATA->m_sPetSkinPackData.sMobID.wMainID,
		pDATA->m_sPetSkinPackData.sMobID.wSubID,
		pDATA->m_sPetSkinPackData.fScale,
		pDATA->m_sPetSkinPackData.dwPetSkinTime,
		_HLIB::cstring_ctime24(pDATA->m_sPetSkinPackData.startTime).GetString(),
		pDATA->m_bDualSkill, /*dual pet skill, Juver, 2017/12/29 */
		dwPETNUM, 
		dwPETCHANUM );		

	if (pDatabase->ExecuteSQL(szTemp) != DB_OK)	{
		return DB_ERROR;
	}

	//pet inven
	CByteStream ByteStream;
	pDATA->GETPUTONITEMS_BYBUF( ByteStream );
	int nRESULT = PetDataSaveInven( dwPETNUM, dwPETCHANUM, ByteStream );
	if ( nRESULT != DB_OK )
		return nRESULT;

	//clear the skills
	nRESULT = PetDataClearSkills( dwPETNUM, dwPETCHANUM );
	if ( nRESULT != DB_OK )
		return nRESULT;

	//pet skill
	for( PETSKILL_MAP_ITER iter_start = pDATA->m_ExpSkills.begin(); 
		iter_start != pDATA->m_ExpSkills.end(); 
		++ iter_start )
	{
		const PETSKILL sPetSkill = (*iter_start).second;
		const SNATIVEID sidSKILL = sPetSkill.sNativeID;

		int nRETURN = PetDataSaveSkill( dwPETNUM, dwPETCHANUM, DB_PET_INVEN_TYPE::SKILL, sidSKILL.wMainID, sidSKILL.wSubID, 0, 0, 0 );
		if ( nRETURN != DB_OK ){
			break;
			return nRETURN;
		}
	}

	//pet active skill
	/*dual pet skill, Juver, 2017/12/29 */
	const SNATIVEID sACTIVESKILL_A = pDATA->m_sActiveSkillID_A;
	if ( sACTIVESKILL_A != NATIVEID_NULL() && pDATA->ISLEARNED_SKILL( sACTIVESKILL_A ) )
	{
		nRESULT = PetDataSaveSkill( dwPETNUM, dwPETCHANUM, DB_PET_INVEN_TYPE::SKILL, sACTIVESKILL_A.wMainID, sACTIVESKILL_A.wSubID, 0, 0, 1 );
		if ( nRESULT != DB_OK )
			return nRESULT;
	}

	//pet active skill
	/*dual pet skill, Juver, 2017/12/29 */
	const SNATIVEID sACTIVESKILL_B = pDATA->m_sActiveSkillID_B;
	if ( sACTIVESKILL_B != NATIVEID_NULL() && pDATA->ISLEARNED_SKILL( sACTIVESKILL_B ) )
	{
		nRESULT = PetDataSaveSkill( dwPETNUM, dwPETCHANUM, DB_PET_INVEN_TYPE::SKILL, sACTIVESKILL_B.wMainID, sACTIVESKILL_B.wSubID, 0, 0, 2 );
		if ( nRESULT != DB_OK )
			return nRESULT;
	}

	return DB_OK;
}

int CGMToolOdbcBase::PetDataSaveInven( int nPETNUM, int nPETCHANUM, CByteStream &ByteStream )
{
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
		std::strstream strPetInven;
		strPetInven << _T("UPDATE PetInfo SET PetInfo.PetPutOnItems=? WHERE (PetNum=" <<  nPETNUM << "And PetChaNum=" << nPETCHANUM << ")");
		strPetInven << std::ends;
		nResult = pDatabase->WriteImage( strPetInven, (DWORD) nPETNUM, (BYTE *) pBuffer, dwSize );

		strPetInven.freeze( false );
		pBuffer = NULL;
		return nResult;
	}
	else
	{
		return DB_ERROR;
	}
}

int CGMToolOdbcBase::PetDataClearSkills( int nPETNUM, int nPETCHANUM )
{
	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	TCHAR szTemp[256] = {0};
	_snprintf( szTemp, 256, "Delete FROM PetInven WHERE PetNum=%d AND PetChaNum=%d AND PetInvenType=%d", nPETNUM, nPETCHANUM, DB_PET_INVEN_TYPE::SKILL );

	return pDatabase->ExecuteSQL(szTemp);;
}

int CGMToolOdbcBase::PetDataSaveSkill( int nPETNUM, int nPETCHANUM, int nPetInvenType, int nPetInvenMID, int nPetInvenSID, int nPetInvenCMID, int nPetInvenCSID, int nPetInvenAvailable )
{
	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	TCHAR szTemp[256] = {0};
	_snprintf( szTemp, 256, "{call sp_UpdatePetInven(%d,%d,%d,%d,%d,%d,%d,%d,?)}", 
		nPETCHANUM,
		nPETNUM,
		nPetInvenType,
		nPetInvenMID,
		nPetInvenSID,
		nPetInvenCMID,
		nPetInvenCSID,
		nPetInvenAvailable );

	return pDatabase->ExecuteSpInt(szTemp);;	
}