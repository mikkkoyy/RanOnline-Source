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

int CGMToolOdbcBase::CharDataSave( SCHARDATA2* pCharData2 )
{
	if ( !pCharData2 ){
		return DB_ERROR;
	}

	DWORD dwChaNum = pCharData2->m_dwCharID;
	DWORD dwUserNum = pCharData2->GetUserID();

	if ( dwChaNum == 0 ){
		return DB_ERROR;
	}

	if ( dwUserNum == 0 ){
		return DB_ERROR;
	}

	if ( pCharData2->m_wLevel == 0 ){
		return DB_ERROR;
	}

	CGMToolOdbcConn* pDatabase = m_pGameDB;
	if ( !pDatabase ){
		return DB_ERROR;
	}

	CString strName (pCharData2->m_szName);
	strName.Trim(_T(" "));
	strName.Replace(_T("'"), _T("''"));

	CString strNick (pCharData2->m_szNick);
	strNick.Trim(_T(" "));
	strNick.Replace(_T("'"), _T("''"));

	if (strName.GetLength() >= (CHR_ID_LENGTH-1)){
		return DB_ERROR;
	}

	if (strNick.GetLength() >= (CHR_ID_LENGTH-1)){
		return DB_ERROR;
	}

	TCHAR szTemp[1024] = {0};
	_snprintf( szTemp, 1024, "UPDATE ChaInfo SET ChaBright=%d, ChaLevel=%u, ChaMoney=%I64d, "
		"ChaDex=%u, ChaIntel=%u, ChaSchool=%u, ChaHair=%u, ChaFace=%u, ChaLiving=%d, "
		"ChaStrong=%u, ChaPower=%u, ChaSpirit=%u, ChaStrength=%u, ChaStRemain=%u, "
		"ChaExp=%I64d, ChaSkillPoint=%u, ChaPK=%u, ChaStartMap=%u, ChaStartGate=%u, "
		"ChaPosX=%f, ChaPosY=%f, ChaPosZ=%f, ChaSaveMap=%d, ChaSavePosX=%f, ChaSavePosY=%f, "
		"ChaSavePosZ=%f, ChaReturnMap=%d, ChaReturnPosX=%f, ChaReturnPosY=%f, ChaReturnPosZ=%f, "
		"ChaGuName='%s', ChaReExp=%I64d, ChaClass=%u, ChaHairColor=%u, ChaInvenLine=%d, "
		"ChaName='%s', ChaSex=%d, ChaScaleRange=%f, ChaCP=%u, ChaContributionPoint=%I64d, "
		"ChaActivityPoint=%u, ChaBadge='%s', ChaPKScore=%u, ChaPKDeath=%u, "
		"ChaEquipmentLockEnable=%u, ChaEquipmentLockStatus=%u "
		"WHERE ChaNum=%u AND UserNum=%u", 
		pCharData2->m_nBright, 
		pCharData2->m_wLevel, 
		pCharData2->m_lnMoney, 
		pCharData2->m_sStats.wDex, 
		pCharData2->m_sStats.wInt, 
		pCharData2->m_wSchool, 
		pCharData2->m_wHair, 
		pCharData2->m_wFace, 
		pCharData2->m_nLiving, 
		pCharData2->m_sStats.wStr, 
		pCharData2->m_sStats.wPow, 
		pCharData2->m_sStats.wSpi, 
		pCharData2->m_sStats.wSta, 
		pCharData2->m_wStatsPoint, 
		pCharData2->m_sExperience.lnNow, 
		pCharData2->m_dwSkillPoint, 
		pCharData2->m_wPK, 
		(int)pCharData2->m_sStartMapID.dwID,
		pCharData2->m_dwStartGate, 
		pCharData2->m_vStartPos.x, 
		pCharData2->m_vStartPos.y, 
		pCharData2->m_vStartPos.z,
		(int)pCharData2->m_sSaveMapID.dwID, 
		pCharData2->m_vSavePos.x, 
		pCharData2->m_vSavePos.y, 
		pCharData2->m_vSavePos.z, 
		(int)pCharData2->m_sLastCallMapID.dwID, 
		pCharData2->m_vLastCallPos.x,
		pCharData2->m_vLastCallPos.y, 
		pCharData2->m_vLastCallPos.z, 
		strNick.GetString(),
		pCharData2->m_lnReExp,
		(int)pCharData2->m_emClass,
		pCharData2->m_wHairColor,
		pCharData2->m_wINVENLINE,
		strName.GetString(),
		(int)CharClassToGender( pCharData2->m_emClass ),
		pCharData2->m_fScaleRange,
		pCharData2->m_sCombatPoint.dwNow, /*combatpoint logic, Juver, 2017/05/27 */

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
		pCharData2->m_bEquipmentLockStatus,

		dwChaNum, 
		dwUserNum );		

	if (pDatabase->ExecuteSQL(szTemp) != DB_OK)	{
		return DB_ERROR;
	}

	CByteStream ByteStream;
	LPBYTE pBuffer = NULL;
	DWORD dwSize = 0;

	// Skill 
	pCharData2->GETEXPSKILLS_BYBUF(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaSkills=? WHERE (ChaNum=%u)", dwChaNum );

		pDatabase->WriteImage( szTemp, dwChaNum, (BYTE *) pBuffer, dwSize );
		pBuffer = NULL;
	}

	// Skill Quick Slot	
	pCharData2->GETSKILL_QUICKSLOT(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaSkillSlot=? WHERE (ChaNum=%u)", dwChaNum );

		pDatabase->WriteImage( szTemp, dwChaNum, (BYTE *) pBuffer, dwSize );
		pBuffer = NULL;
	}

	// action slot
	pCharData2->GETACTION_QUICKSLOT(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);	
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaActionSlot=? WHERE (ChaNum=%u)", dwChaNum );

		pDatabase->WriteImage( szTemp, dwChaNum, (BYTE *) pBuffer, dwSize );
		pBuffer = NULL;
	}

	//puton
	pCharData2->GETPUTONITEMS_BYBUF(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaPutOnItems=? WHERE (ChaNum=%u)", dwChaNum );

		pDatabase->WriteImage( szTemp, dwChaNum, (BYTE *) pBuffer, dwSize );		
		pBuffer = NULL;
	}	

	//inven
	ByteStream.ClearBuffer();
	pCharData2->GETINVENTORYE_BYBUF(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);	
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaInven=? WHERE (ChaNum=%u)", dwChaNum );

		pDatabase->WriteImage( szTemp, dwChaNum, (BYTE *) pBuffer, dwSize );		
		pBuffer = NULL;
	}	

	//quest
	pCharData2->GETQUESTPLAY(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaQuest=? WHERE (ChaNum=%u)", dwChaNum );

		pDatabase->WriteImage( szTemp, dwChaNum, (BYTE *) pBuffer, dwSize );
		pBuffer = NULL;
	}

	//cooltime
	pCharData2->GETITEMCOOLTIME_BYBUF(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaCoolTime=? WHERE (ChaNum=%u)", dwChaNum );

		pDatabase->WriteImage( szTemp, dwChaNum, (BYTE *) pBuffer, dwSize );
		pBuffer = NULL;
	}	

	/*itemfood system, Juver, 2017/05/26 */
	pCharData2->GETITEMFOOD_BYBUF(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaItemFood=? WHERE (ChaNum=%u)", dwChaNum );
		m_pGameDB->WriteImage( szTemp, dwChaNum, (BYTE *) pBuffer, dwSize );
		pBuffer = NULL;
	}	

	/*activity system, Juver, 2017/10/23 */
	pCharData2->GETACTIVITY_BYBUF(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaActivity=? WHERE (ChaNum=%u)", dwChaNum );
		m_pGameDB->WriteImage( szTemp, dwChaNum, (BYTE *) pBuffer, dwSize );
		pBuffer = NULL;
	}	
	
	/*Codex System, Jhoniex 2/14/2024 */
	pCharData2->GETCODEX_BYBUF(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);
	if (pBuffer != NULL)
	{
		TCHAR szTemp[128] = {0};
		_snprintf( szTemp, 128, "UPDATE ChaInfo SET ChaInfo.ChaCodex=? WHERE (ChaNum=%u)", dwChaNum );
		m_pGameDB->WriteImage( szTemp, dwChaNum, (BYTE *) pBuffer, dwSize );
		pBuffer = NULL;
	}	

	//locker binary
	pCharData2->GETSTORAGE_BYBUF(ByteStream);
	ByteStream.GetBuffer(pBuffer, dwSize);		
	if (pBuffer != NULL)
	{
		 UserInvenSaveBinary( pCharData2->GetUserID(), (BYTE *) pBuffer, dwSize);
	}

	//locker basic data
	UserInvenSaveData( pCharData2 );

	return DB_OK;
}
