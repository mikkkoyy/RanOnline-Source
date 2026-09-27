#include "stdAfx.h"
#include "GMToolOdbcBase.h"
#include "GMToolOdbcConn.h"
#include "GMToolLogs.h"

#include "GLCharDefine.h"
#include "GLCharData.h"
#include "DxLobyStage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CGMToolOdbcBase::CharDataCopy( int nUserNum, SCHARDATA2* pCharData2 )
{
	if ( !pCharData2 ){
		return DB_ERROR;
	}

	if ( nUserNum <= 0 ){
		return DB_ERROR;
	}

	pCharData2->SetUserID( (DWORD)nUserNum );

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	ODBC_STMT* pConn = pDatabase->GetConnection();
	if (!pConn){
		return DB_ERROR;
	}

	SQLRETURN	sReturn = 0;

	DWORD dwUserNum = pCharData2->GetUserID();
	DWORD dwSvrNum  = pCharData2->m_dwServerID;

	TCHAR szTemp[2048] = {0};
	_snprintf( szTemp, 2048, "INSERT INTO ChaInfo(UserNum, SGNum, ChaName, ChaTribe, ChaClass, "
		"ChaSchool, ChaHair, ChaFace, ChaLiving, "
		"ChaBright, ChaLevel, ChaMoney, ChaDex, ChaIntel, "
		"ChaStrong, ChaPower, ChaSpirit, ChaStrength, ChaStRemain, "
		"ChaAttackP, ChaDefenseP, ChaFightA, ChaShootA, ChaExp, "
		"ChaSkillPoint, ChaHP, ChaMP, ChaSP, ChaPK, "
		"ChaStartMap, ChaStartGate, ChaPosX, ChaPosY, ChaPosZ, "
		"ChaSaveMap, ChaSavePosX, ChaSavePosY, ChaSavePosZ, "
		"ChaReturnMap, ChaReturnPosX, ChaReturnPosY, ChaReturnPosZ, "
		"ChaHairColor, ChaSex, ChaReExp, ChaGuName, ChaInvenLine, "
		"ChaScaleRange, ChaCP, ChaContributionPoint, "
		"ChaActivityPoint, ChaBadge, ChaPKScore, ChaPKDeath, "
		"ChaEquipmentLockEnable, ChaEquipmentLockStatus, "
		"ChaSkills, ChaSkillSlot, ChaActionSlot, ChaPutOnItems, ChaInven, ChaQuest, ChaCoolTime,"
		"ChaActivity, ChaCodex ) Values ( "
		"%u,%u,'%s',%d,%d,%u,%u,%u,%d,"
		"%d,%u,%I64d,%u,%u,%u,%u,%u,%u,%u,"
		"%u,%u,%u,%u,%I64d,%u,%u,%u,%u,%u,"
		"%u,%u,%f,%f,%f,%u,%f,%f,%f,%u,%f,%f,%f,"
		"%u,%u,%I64d,'%s',%u,%f,%u,"
		"%I64d,%u,'%s',%u,%u,"
		"%u,%u,"
		"'','','','','','','',"
		"'','')",
		dwUserNum,
		dwSvrNum,
		pCharData2->m_szName,
		pCharData2->m_emTribe,
		pCharData2->m_emClass,

		pCharData2->m_wSchool,
		pCharData2->m_wHair,
		pCharData2->m_wFace,
		pCharData2->m_nLiving,

		pCharData2->m_nBright,
		pCharData2->m_wLevel,
		pCharData2->m_lnMoney,
		pCharData2->m_sStats.wDex,
		pCharData2->m_sStats.wInt,

		pCharData2->m_sStats.wStr,
		pCharData2->m_sStats.wPow,
		pCharData2->m_sStats.wSpi,
		pCharData2->m_sStats.wSta,
		pCharData2->m_wStatsPoint,

		pCharData2->m_wAP,
		pCharData2->m_wDP,
		pCharData2->m_wPA,
		pCharData2->m_wSA,
		pCharData2->m_sExperience.lnNow,

		pCharData2->m_dwSkillPoint,
		pCharData2->m_sHP.dwData1,
		pCharData2->m_sMP.dwData1,
		pCharData2->m_sSP.dwData1,
		pCharData2->m_wPK,

		pCharData2->m_sStartMapID.dwID,
		pCharData2->m_dwStartGate,
		pCharData2->m_vStartPos.x,
		pCharData2->m_vStartPos.y,
		pCharData2->m_vStartPos.z,

		pCharData2->m_sSaveMapID.dwID,
		pCharData2->m_vSavePos.x,
		pCharData2->m_vSavePos.y,
		pCharData2->m_vSavePos.z,

		pCharData2->m_sLastCallMapID.dwID,
		pCharData2->m_vLastCallPos.x,
		pCharData2->m_vLastCallPos.y,
		pCharData2->m_vLastCallPos.z,

		pCharData2->m_wHairColor,
		pCharData2->m_wSex,
		pCharData2->m_lnReExp,
		pCharData2->m_szNick,
		pCharData2->m_wINVENLINE,
		pCharData2->m_fScaleRange,
		pCharData2->m_sCombatPoint.dwNow,  /*combatpoint logic, Juver, 2017/05/27 */

		/*contribution point, Juver, 2017/08/23 */
		pCharData2->m_llContributionPoint,

		/*activity point, Juver, 2017/08/23 */
		pCharData2->m_dwActivityPoint,

		/*activity system, Juver, 2017/11/04 */
		pCharData2->m_szBadge,

		/*pk info, Juver, 2017/11/17 */
		pCharData2->m_dwPKScore,
		pCharData2->m_dwPKDeath,

		/*equipment lock, Juver, 2018/01/13 */
		pCharData2->m_bEnableEquipmentLock,
		pCharData2->m_bEquipmentLockStatus

		);

	
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
	
	sReturn = ::SQLExecDirect(pConn->hStmt, (SQLCHAR*) "SELECT cast(SCOPE_IDENTITY() as int)",  SQL_NTS);

	if ((sReturn != SQL_SUCCESS) && (sReturn != SQL_SUCCESS_WITH_INFO)) 
	{
		GMTOOL_LOGS::LogSqlError( "%s", GetErrorString(pConn->hStmt) );
		pDatabase->FreeConnection(pConn);
		return DB_ERROR;
	}

	SQLINTEGER nChaNewNum=0, cbChaNewNum=SQL_NTS;

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
			::SQLGetData(pConn->hStmt, 1, SQL_C_LONG, &nChaNewNum, 0, &cbChaNewNum);
			pCharData2->m_dwCharID = nChaNewNum;			
		}
		else
		{
			break;
		}
		Sleep( 0 );
	}
	
	pDatabase->FreeConnection(pConn);

	CByteStream ByteStream;
	LPBYTE pBuffer = NULL;
	DWORD dwSize = 0;

	// Character Skill 
	pCharData2->GETEXPSKILLS_BYBUF(ByteStream);		
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{	
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaSkills=? WHERE (ChaNum=%d)", nChaNewNum );
		pDatabase->WriteImage(szTemp, nChaNewNum, (BYTE *) pBuffer, dwSize);
		pBuffer = NULL;
	}	

	// Character Skill Quick Slot
	pCharData2->GETSKILL_QUICKSLOT(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaSkillSlot=? WHERE (ChaNum=%d)", nChaNewNum );
		pDatabase->WriteImage(szTemp, nChaNewNum, (BYTE *) pBuffer, dwSize);
		pBuffer = NULL;
	}

	// Character Action Quick Slot
	pCharData2->GETACTION_QUICKSLOT(ByteStream);		
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaActionSlot=? WHERE (ChaNum=%d)", nChaNewNum );
		pDatabase->WriteImage(szTemp, nChaNewNum, (BYTE *) pBuffer, dwSize);
		pBuffer = NULL;
	}

	// Character Put on item
	pCharData2->GETPUTONITEMS_BYBUF(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaPutOnItems=? WHERE (ChaNum=%d)", nChaNewNum );
		pDatabase->WriteImage(szTemp, nChaNewNum, (BYTE *) pBuffer, dwSize);
		pBuffer = NULL;
	}

	// Character Inventory
	ByteStream.ClearBuffer();
	pCharData2->GETINVENTORYE_BYBUF(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);	
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaInven=? WHERE (ChaNum=%d)", nChaNewNum );
		pDatabase->WriteImage(szTemp, nChaNewNum, (BYTE *) pBuffer, dwSize);
		pBuffer = NULL;
	}

	// Character Quest information
	pCharData2->GETQUESTPLAY(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaQuest=? WHERE (ChaNum=%d)", nChaNewNum );
		pDatabase->WriteImage(szTemp, nChaNewNum, (BYTE *) pBuffer, dwSize);
		pBuffer = NULL;
	}

	//cooltime
	pCharData2->GETITEMCOOLTIME_BYBUF(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaCoolTime=? WHERE (ChaNum=%d)", nChaNewNum );
		pDatabase->WriteImage( szTemp, nChaNewNum, (BYTE *) pBuffer, dwSize );
		pBuffer = NULL;
	}	

	/*itemfood system, Juver, 2017/05/26 */
	pCharData2->GETITEMFOOD_BYBUF(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaItemFood=? WHERE (ChaNum=%d)", nChaNewNum );
		m_pGameDB->WriteImage( szTemp, nChaNewNum, (BYTE *) pBuffer, dwSize );
		pBuffer = NULL;
	}	

	/*activity system, Juver, 2017/10/23 */
	pCharData2->GETACTIVITY_BYBUF(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaActivity=? WHERE (ChaNum=%d)", nChaNewNum );
		m_pGameDB->WriteImage( szTemp, nChaNewNum, (BYTE *) pBuffer, dwSize );
		pBuffer = NULL;
	}	

	/*Codex System, Jhoniex 2/14/2024 */
	pCharData2->GETCODEX_BYBUF(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaCodex=? WHERE (ChaNum=%d)", nChaNewNum );
		m_pGameDB->WriteImage( szTemp, nChaNewNum, (BYTE *) pBuffer, dwSize );
		pBuffer = NULL;
	}	

	return DB_OK;
}
