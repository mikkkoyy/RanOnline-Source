#include "stdAfx.h"
#include "GMToolOdbcBase.h"
#include "GMToolOdbcConn.h"
#include "GMToolLogs.h"

#include "GLCharDefine.h"
#include "GLCharData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CGMToolOdbcBase::CharDataGet( int nUserNumber, int nChaNumber, SCHARDATA2* pCharData2 )
{
	if ( !pCharData2 ){
		return DB_ERROR;
	}

	if ( nUserNumber <= 0 ){
		return DB_ERROR;
	}

	if ( nChaNumber <= 0 ){
		return DB_ERROR;
	}

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, 
		"SELECT UserNum,ChaName,ChaClass,ChaBright,ChaLevel,ChaDex,"
		"ChaIntel,ChaPower,ChaStrong,ChaSpirit,ChaStrength,ChaStRemain,"
		"ChaSkillPoint,ChaPK,ChaStartMap,ChaStartGate,ChaPosX,ChaPosY,"
		"ChaPosZ,ChaMoney,ChaExp,ChaSaveMap,ChaSavePosX,ChaSavePosY,"
		"ChaSavePosZ,ChaSchool,ChaHair,ChaFace,ChaLiving,ChaInvenLine,"
		"ChaReturnMap,ChaReturnPosX,ChaReturnPosY,ChaReturnPosZ,GuNum,"
		"ChaGuName,ChaSex,ChaHairColor,ChaReExp,ChaScaleRange,ChaCP, "
		"ChaContributionPoint, ChaActivityPoint, ChaBadge, ChaPKScore, "
		"ChaPKDeath, ChaEquipmentLockEnable, ChaEquipmentLockStatus "
		"FROM ChaInfo WHERE ChaNum=%d AND UserNum=%d", nChaNumber, nUserNumber );

	GMTOOL_LOGS::LogSql( "%s", szTemp );

	int nRowCount = 0;
	SQLRETURN sReturn = 0;

	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*) szTemp, SQL_NTS);

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}

	SQLINTEGER nUserNum = 0, cbUserNum=SQL_NTS;
	SQLCHAR    szChaName[CHR_ID_LENGTH+1] = {0}; SQLINTEGER cbChaName = SQL_NTS;
	SQLINTEGER nChaClass = 0, cbChaClass=SQL_NTS;
	SQLINTEGER nChaBright = 0, cbChaBright=SQL_NTS;
	SQLINTEGER nChaLevel = 0, cbChaLevel=SQL_NTS;
	SQLINTEGER nChaDex = 0, cbChaDex=SQL_NTS;
	SQLINTEGER nChaIntel = 0, cbChaIntel=SQL_NTS;
	SQLINTEGER nChaPower = 0, cbChaPower=SQL_NTS; 
	SQLINTEGER nChaStrong = 0, cbChaStrong=SQL_NTS;   
	SQLINTEGER nChaSpirit = 0, cbChaSpirit=SQL_NTS;
	SQLINTEGER nChaStrength = 0, cbChaStrength=SQL_NTS; 
	SQLINTEGER nChaStRemain = 0, cbChaStRemain=SQL_NTS; 
	SQLINTEGER nChaSkillPoint = 0, cbChaSkillPoint=SQL_NTS;
	SQLINTEGER nChaPK = 0, cbChaPK=SQL_NTS;
	SQLINTEGER nChaStartMap = 0, cbChaStartMap=SQL_NTS;
	SQLINTEGER nChaStartGate = 0, cbChaStartGate=SQL_NTS;
	SQLFLOAT fChaPosX = 0; SQLINTEGER cbChaPosX=SQL_NTS;
	SQLFLOAT fChaPosY = 0; SQLINTEGER cbChaPosY=SQL_NTS;
	SQLFLOAT fChaPosZ = 0; SQLINTEGER cbChaPosZ=SQL_NTS;
	LONGLONG   llChaExp = 0, llChaMoney = 0;
	SQLINTEGER cbChaExp=SQL_NTS, cbChaMoney=SQL_NTS; 
	SQLINTEGER nChaSaveMap = 0, cbChaSaveMap=SQL_NTS;
	SQLFLOAT fChaSavePosX = 0; SQLINTEGER cbChaSavePosX=SQL_NTS;
	SQLFLOAT fChaSavePosY = 0; SQLINTEGER cbChaSavePosY=SQL_NTS;
	SQLFLOAT fChaSavePosZ = 0; SQLINTEGER cbChaSavePosZ=SQL_NTS;
	SQLINTEGER nChaSchool = 0, cbChaSchool = SQL_NTS;
	SQLINTEGER nChaHair = 0, cbChaHair = SQL_NTS;
	SQLINTEGER nChaFace = 0, cbChaFace = SQL_NTS;
	SQLINTEGER nChaLiving = 0, cbChaLiving = SQL_NTS;
	SQLINTEGER nChaInvenLine = 0; SQLINTEGER cbChaInvenLine = SQL_NTS;
	SQLINTEGER nChaReturnMap  = 0; SQLINTEGER cbChaReturnMap  = SQL_NTS;
	SQLFLOAT   fChaReturnPosX = 0; SQLINTEGER cbChaReturnPosX = SQL_NTS;
	SQLFLOAT   fChaReturnPosY = 0; SQLINTEGER cbChaReturnPosY = SQL_NTS;
	SQLFLOAT   fChaReturnPosZ = 0; SQLINTEGER cbChaReturnPosZ = SQL_NTS;
	SQLINTEGER nGuNum = 0; SQLINTEGER cbGuNum = SQL_NTS;
	SQLCHAR    szChaGuName[CHAR_SZNAME+1] = {0}; SQLINTEGER cbChaGuName = SQL_NTS;
	SQLSMALLINT nChaSex = 0; SQLINTEGER cbChaSex = SQL_NTS;
	SQLINTEGER nChaHairColor =0, cbChaHairColor = SQL_NTS;
	LONGLONG   llChaReExp = 0;
	SQLINTEGER cbChaReExp=SQL_NTS;
	SQLFLOAT fChaScaleRange = 0; SQLINTEGER cbChaScaleRange=SQL_NTS;
	
	/*combatpoint logic, Juver, 2017/05/27 */
	SQLINTEGER nChaCP = 0, cbChaCP=SQL_NTS;

	/*contribution point, Juver, 2017/08/23 */
	LONGLONG   llChaContributionPoint = 0;
	SQLINTEGER cbChaContributionPoint = SQL_NTS;

	/*activity point, Juver, 2017/08/23 */
	SQLINTEGER  nChaActivityPoint, cbChaActivityPoint = SQL_NTS;

	/*activity system, Juver, 2017/11/04 */
	SQLCHAR    szChaBadge[CHAR_SZNAME+1] = {0}; SQLINTEGER cbChaBadge = SQL_NTS;

	/*pk info, Juver, 2017/11/17 */
	SQLINTEGER  nChaPKScore, cbChaPKScore = SQL_NTS;
	SQLINTEGER  nChaPKDeath, cbChaPKDeath = SQL_NTS;

	/*equipment lock, Juver, 2018/01/13 */
	SQLINTEGER  nChaEquipmentLockEnable, cbChaEquipmentLockEnable = SQL_NTS;
	SQLINTEGER  nChaEquipmentLockStatus, cbChaEquipmentLockStatus = SQL_NTS;

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
			::SQLGetData(pConn->hStmt, 1, SQL_C_LONG, &nUserNum,       0, &cbUserNum);
			::SQLGetData(pConn->hStmt, 2, SQL_C_CHAR, szChaName,  CHR_ID_LENGTH+1, &cbChaName); 
			::SQLGetData(pConn->hStmt, 3, SQL_C_LONG, &nChaClass,      0, &cbChaClass);
			::SQLGetData(pConn->hStmt, 4, SQL_C_LONG, &nChaBright,     0, &cbChaBright);
			::SQLGetData(pConn->hStmt, 5, SQL_C_LONG, &nChaLevel,      0, &cbChaLevel);
			::SQLGetData(pConn->hStmt, 6, SQL_C_LONG, &nChaDex,        0, &cbChaDex);
			::SQLGetData(pConn->hStmt, 7, SQL_C_LONG, &nChaIntel,      0, &cbChaIntel);
			::SQLGetData(pConn->hStmt, 8, SQL_C_LONG, &nChaPower,      0, &cbChaPower);
			::SQLGetData(pConn->hStmt, 9, SQL_C_LONG, &nChaStrong,     0, &cbChaStrong);
			::SQLGetData(pConn->hStmt,10, SQL_C_LONG, &nChaSpirit,     0, &cbChaSpirit);
			::SQLGetData(pConn->hStmt,11, SQL_C_LONG, &nChaStrength,   0, &cbChaStrength);
			::SQLGetData(pConn->hStmt,12, SQL_C_LONG, &nChaStRemain,   0, &cbChaStRemain);
			::SQLGetData(pConn->hStmt,13, SQL_C_LONG, &nChaSkillPoint, 0, &cbChaSkillPoint);
			::SQLGetData(pConn->hStmt,14, SQL_C_LONG, &nChaPK,			0, &cbChaPK);
			::SQLGetData(pConn->hStmt,15, SQL_C_LONG, &nChaStartMap,	0, &cbChaStartMap);
			::SQLGetData(pConn->hStmt,16, SQL_C_LONG, &nChaStartGate,  0, &cbChaStartGate);
			::SQLGetData(pConn->hStmt,17, SQL_C_DOUBLE, &fChaPosX,		0, &cbChaPosX);
			::SQLGetData(pConn->hStmt,18, SQL_C_DOUBLE, &fChaPosY,		0, &cbChaPosY);
			::SQLGetData(pConn->hStmt,19, SQL_C_DOUBLE, &fChaPosZ,		0, &cbChaPosZ);			
			::SQLGetData(pConn->hStmt,20, SQL_C_SBIGINT, &llChaMoney,  0, &cbChaMoney);
			::SQLGetData(pConn->hStmt,21, SQL_C_SBIGINT, &llChaExp,    0, &cbChaExp);
			::SQLGetData(pConn->hStmt,22, SQL_C_LONG, &nChaSaveMap,    0, &cbChaSaveMap);
			::SQLGetData(pConn->hStmt,23, SQL_C_DOUBLE, &fChaSavePosX,   0, &cbChaSavePosX);
			::SQLGetData(pConn->hStmt,24, SQL_C_DOUBLE, &fChaSavePosY,   0, &cbChaSavePosY);
			::SQLGetData(pConn->hStmt,25, SQL_C_DOUBLE, &fChaSavePosZ,   0, &cbChaSavePosZ);
			::SQLGetData(pConn->hStmt,26, SQL_C_LONG, &nChaSchool,		0, &cbChaSchool);
			::SQLGetData(pConn->hStmt,27, SQL_C_LONG, &nChaHair,		0, &cbChaHair);
			::SQLGetData(pConn->hStmt,28, SQL_C_LONG, &nChaFace,		0, &cbChaFace);
			::SQLGetData(pConn->hStmt,29, SQL_C_LONG, &nChaLiving,		0, &cbChaLiving);
			::SQLGetData(pConn->hStmt,30, SQL_C_LONG, &nChaInvenLine, 0, &cbChaInvenLine);
			::SQLGetData(pConn->hStmt,31, SQL_C_LONG,   &nChaReturnMap,  0, &cbChaReturnMap);
			::SQLGetData(pConn->hStmt,32, SQL_C_DOUBLE, &fChaReturnPosX, 0, &cbChaReturnPosX);
			::SQLGetData(pConn->hStmt,33, SQL_C_DOUBLE, &fChaReturnPosY, 0, &cbChaReturnPosY);
			::SQLGetData(pConn->hStmt,34, SQL_C_DOUBLE, &fChaReturnPosZ, 0, &cbChaReturnPosZ);
			::SQLGetData(pConn->hStmt,35, SQL_C_LONG, &nGuNum, 0, &cbGuNum);
			::SQLGetData(pConn->hStmt,36, SQL_C_CHAR, szChaGuName, CHAR_SZNAME, &cbChaGuName); 
			::SQLGetData(pConn->hStmt,37, SQL_SMALLINT, &nChaSex,       0, &cbChaSex);
			::SQLGetData(pConn->hStmt,38, SQL_C_LONG,   &nChaHairColor, 0, &cbChaHairColor);
			::SQLGetData(pConn->hStmt,39, SQL_C_SBIGINT, &llChaReExp, 0, &cbChaReExp);	
			::SQLGetData(pConn->hStmt,40, SQL_C_DOUBLE, &fChaScaleRange, 0, &cbChaScaleRange);

			/*combatpoint logic, Juver, 2017/05/27 */
			::SQLGetData(pConn->hStmt,41, SQL_C_LONG, &nChaCP,			0, &cbChaCP);

			/*contribution point, Juver, 2017/08/23 */
			::SQLGetData(pConn->hStmt,42, SQL_C_SBIGINT, &llChaContributionPoint, 0, &cbChaContributionPoint);

			/*activity point, Juver, 2017/08/23 */
			::SQLGetData(pConn->hStmt,43, SQL_C_LONG, &nChaActivityPoint, 0, &cbChaActivityPoint);	

			/*activity system, Juver, 2017/11/04 */
			::SQLGetData(pConn->hStmt,44, SQL_C_CHAR, szChaBadge, CHAR_SZNAME, &cbChaBadge); 

			/*pk info, Juver, 2017/11/17 */
			::SQLGetData(pConn->hStmt,45, SQL_C_LONG, &nChaPKScore, 0, &cbChaPKScore );	
			::SQLGetData(pConn->hStmt,46, SQL_C_LONG, &nChaPKDeath, 0, &cbChaPKDeath );	

			/*equipment lock, Juver, 2018/01/13 */
			::SQLGetData(pConn->hStmt,47, SQL_C_LONG, &nChaEquipmentLockEnable, 0, &cbChaEquipmentLockEnable );	
			::SQLGetData(pConn->hStmt,48, SQL_C_LONG, &nChaEquipmentLockStatus, 0, &cbChaEquipmentLockStatus );	


			pCharData2->m_dwCharID	= (DWORD) nChaNumber; 
			pCharData2->SetUserID( (DWORD) nUserNum );
			pCharData2->m_dwGuild    = (DWORD) nGuNum;

			if (cbChaName != 0 && cbChaName != -1) 
				::StringCchCopy(pCharData2->m_szName, CHR_ID_LENGTH, (const char*) szChaName);

			if (cbChaGuName != 0 && cbChaGuName != -1) 
				::StringCchCopy(pCharData2->m_szNick, CHAR_SZNAME, (const char*) szChaGuName);

			pCharData2->m_emClass			= EMCHARCLASS(nChaClass);
			pCharData2->m_wLevel			= (WORD) nChaLevel;           
			pCharData2->m_sStats.wPow		= (WORD) nChaPower;
			pCharData2->m_sStats.wStr		= (WORD) nChaStrong;
			pCharData2->m_sStats.wSta		= (WORD) nChaStrength;
			pCharData2->m_sStats.wSpi		= (WORD) nChaSpirit;
			pCharData2->m_sStats.wDex		= (WORD) nChaDex;
			pCharData2->m_sStats.wInt		= (WORD) nChaIntel;
			pCharData2->m_wStatsPoint		= (WORD) nChaStRemain;
			pCharData2->m_lnMoney            = llChaMoney;
			pCharData2->m_sExperience.lnNow  = llChaExp;

			pCharData2->m_sStartMapID.dwID	= (DWORD) nChaStartMap;
			pCharData2->m_dwStartGate		= (DWORD) nChaStartGate;
			pCharData2->m_vStartPos.x		= (float) fChaPosX;
			pCharData2->m_vStartPos.y		= (float) fChaPosY;
			pCharData2->m_vStartPos.z		= (float) fChaPosZ,	

			pCharData2->m_nBright			= nChaBright;
			pCharData2->m_dwSkillPoint		= (DWORD) nChaSkillPoint;
			pCharData2->m_wPK				= (WORD) nChaPK;

			pCharData2->m_sSaveMapID.dwID	= (DWORD) nChaSaveMap;
			pCharData2->m_vSavePos.x		= (float) fChaSavePosX; 
			pCharData2->m_vSavePos.y		= (float) fChaSavePosY; 
			pCharData2->m_vSavePos.z		= (float) fChaSavePosZ;

			pCharData2->m_sLastCallMapID.dwID= (DWORD) nChaReturnMap;
			pCharData2->m_vLastCallPos.x	= (float) fChaReturnPosX; 
			pCharData2->m_vLastCallPos.y	= (float) fChaReturnPosY; 
			pCharData2->m_vLastCallPos.z	= (float) fChaReturnPosZ;

			pCharData2->m_wSchool			= (WORD) nChaSchool;
			pCharData2->m_wFace				= (WORD) nChaFace;
			pCharData2->m_wHair				= (WORD) nChaHair;
			pCharData2->m_nLiving			= (int) nChaLiving;
			pCharData2->m_wHairColor        = (WORD) nChaHairColor;
			pCharData2->m_wSex              = (WORD) nChaSex;

			pCharData2->m_wINVENLINE = (WORD) nChaInvenLine; 
			pCharData2->m_lnReExp = llChaReExp;

			pCharData2->m_fScaleRange	= (float) fChaScaleRange;

			/*combatpoint logic, Juver, 2017/05/27 */
			pCharData2->m_sCombatPoint.dwNow = (DWORD) nChaCP;

			/*contribution point, Juver, 2017/08/23 */
			pCharData2->m_llContributionPoint = llChaContributionPoint;

			/*activity point, Juver, 2017/08/23 */
			pCharData2->m_dwActivityPoint = (DWORD)nChaActivityPoint;

			/*activity system, Juver, 2017/11/04 */
			if (cbChaBadge != 0 && cbChaBadge != -1) 
				::StringCchCopy(pCharData2->m_szBadge, CHAR_SZNAME, (const char*) szChaBadge);

			/*pk info, Juver, 2017/11/17 */
			pCharData2->m_dwPKScore = (DWORD)nChaPKScore;
			pCharData2->m_dwPKDeath = (DWORD)nChaPKDeath;

			/*equipment lock, Juver, 2018/01/13 */
			pCharData2->m_bEnableEquipmentLock = (BOOL)nChaEquipmentLockEnable;
			pCharData2->m_bEquipmentLockStatus = (BOOL)nChaEquipmentLockStatus;

			nRowCount++;
		}else{
			break;
		}

		Sleep(0);
	}

	pDatabase->FreeConnection(pConn);

	if (nRowCount != 1){ 
		return DB_ERROR;
	}

	CByteStream ByteStream;

	// Skill	
	sReturn = pDatabase->ReadImage("ChaInfo.ChaSkills", nChaNumber, ByteStream);
	if (sReturn == DB_ERROR) 
		return DB_ERROR;
	else 
		pCharData2->SETEXPSKILLS_BYBUF(ByteStream);

	//Skill Quick Slot		
	sReturn = pDatabase->ReadImage("ChaInfo.ChaSkillSlot", nChaNumber, ByteStream);
	if (sReturn == DB_ERROR)
		return DB_ERROR;
	else
		pCharData2->SETSKILL_QUICKSLOT(ByteStream);

	//Action Quick Slot	
	sReturn = pDatabase->ReadImage("ChaInfo.ChaActionSlot", nChaNumber, ByteStream);
	if (sReturn == DB_ERROR)
		return DB_ERROR;
	else
		pCharData2->SETACTION_QUICKSLOT(ByteStream);

	//Quest
	sReturn = pDatabase->ReadImage("ChaInfo.ChaQuest", nChaNumber, ByteStream);
	if (sReturn == DB_ERROR)
		return DB_ERROR;
	else
		pCharData2->SETQUESTPLAY(ByteStream);

	//puton
	sReturn = pDatabase->ReadImage("ChaInfo.ChaPutOnItems", nChaNumber, ByteStream);
	if (sReturn == DB_ERROR)
		return DB_ERROR;
	else
		SETPUTONITEMS_BYBUF(pCharData2->m_PutOnItems,ByteStream);

	//inventory
	sReturn = pDatabase->ReadImage("ChaInfo.ChaInven", nChaNumber, ByteStream);
	if (sReturn == DB_ERROR)
		return DB_ERROR;
	else
		pCharData2->SETINVENTORY_BYBUF(ByteStream);

	//cooltime
	sReturn = pDatabase->ReadImage("ChaInfo.ChaCoolTime", nChaNumber, ByteStream);
	if (sReturn == DB_ERROR)
		return DB_ERROR;
	else
		pCharData2->SETITEMCOOLTIME_BYBUF(ByteStream);

	/*itemfood system, Juver, 2017/05/26 */
	sReturn = m_pGameDB->ReadImage("ChaInfo.ChaItemFood", nChaNumber, ByteStream);
	if (sReturn == DB_ERROR)
		return DB_ERROR;
	else
		pCharData2->SETITEMFOOD_BYBUF(ByteStream);


	/*activity system, Juver, 2017/10/23 */
	sReturn = m_pGameDB->ReadImage("ChaInfo.ChaActivity", nChaNumber, ByteStream);
	if (sReturn == DB_ERROR)
		return DB_ERROR;
	else
		pCharData2->SETACTIVITY_BYBUF(ByteStream);
	
	/*Codex System, Jhoniex 2/14/2024 */
	sReturn = m_pGameDB->ReadImage("ChaInfo.ChaCodex", nChaNumber, ByteStream);
	if (sReturn == DB_ERROR)
		return DB_ERROR;
	else
		pCharData2->SETCODEX_BYBUF(ByteStream);

	//validate locker if exist
	UserInvenValidate( pCharData2 );

	//get basic locker data
	UserInvenGetData( pCharData2 );

	//get locker item data
	sReturn = UserInvenGetBinary( pCharData2->GetUserID(), ByteStream );
	if (sReturn == DB_ERROR)
		return DB_ERROR;
	else
		pCharData2->SETSTORAGE_BYBUF(ByteStream);



	return DB_OK;
}
