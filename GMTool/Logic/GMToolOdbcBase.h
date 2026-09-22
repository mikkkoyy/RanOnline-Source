#ifndef _GMTOOL_ODBC_BASE_H_
#define _GMTOOL_ODBC_BASE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Windows.h>
#include <stdio.h>
#include <odbcinst.h>
#include <sqlext.h>

#include "s_NetGlobal.h"
#include "s_CLock.h"
#include "s_DbBase.h"

#include "GMToolOdbcConn.h"
#include "GMToolData.h"

struct SCHARDATA2;
struct GLPET;
struct GLVEHICLE;

class CGMToolOdbcBase : public CLock
{
public:	
	static CGMToolOdbcBase* GetInstance();
	static void ReleaseInstance();

private:
	CGMToolOdbcBase(void);
	~CGMToolOdbcBase(void);
	static CGMToolOdbcBase* SelfInstance;

protected:
   	CGMToolOdbcConn* m_pGameDB;
	CGMToolOdbcConn* m_pUserDB;
	CGMToolOdbcConn* m_pLogDB;
	CGMToolOdbcConn* m_pShopDB;

public:
	enum DB_PET_INVEN_TYPE
	{
		TYPEA = 1,
		TYPEB = 2,
		SKILL = 3,
	};

public:
	BOOL DatabaseOpen();
	BOOL DatabaseClose();

	int	OpenGameDB();
	int	OpenUserDB();
	int	OpenLogDB();
	int OpenShopDB();

	void CloseGameDB();
	void CloseUserDB();
	void CloseLogDB();
	void CloseShopDB();

	CString	GetErrorString(SQLHSTMT hStmt);

	
public:
	int GMLogin( std::string strUSER, std::string strPASS, SGMTOOL_USER_GM* pDATA );

private:
	int UserFind( const char* szSQL, SGMTOOL_USER_SIMPLE_MAP &mapResult );

public:
	int UserFind_UserNumber( int nUSERNUM, SGMTOOL_USER_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int UserFind_UserName( std::string strUSERID, SGMTOOL_USER_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int UserFind_UserEmail( std::string strUSEREMAIL, SGMTOOL_USER_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int UserFind_UserGM( SGMTOOL_USER_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int UserFind_Online( SGMTOOL_USER_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int UserIDCheck( std::string strUSERID, BOOL &bEXIST, int &nUSERNUM );
	int UserNew( SGMTOOL_USER_NEW* pData );
	int UserOnlineCheck( int nUSERNUM, BOOL& bONLINE );
	int UserLogOut( int nUSERNUM );
	int UserBlockSet( int nUSERNUM, BOOL bBlock, __time64_t tTime );
	int UserChatBlockSet( int nUSERNUM, __time64_t tTime );
	int UserDataGet( int nUSERNUM, SGMTOOL_DATA_USER* pDATA );
	int UserDataSave( SGMTOOL_DATA_USER* pDATA );
	int UserIPLOGRead( int nUSERNUM, SGMTOOL_DATA_USER_IPLOG_VEC &vecResult );
	int UserIPLOGRead( int nUSERNUM, SGMTOOL_DATA_USER_IPLOG_MAP &mapResult );
	int UserIPLOGErase( int nUSERNUM );
	int UserCharactersRead( int nUSERNUM, SGMTOOL_DATA_USER_CHARACTERS_VEC &vecResult );


private:
	int CharFind( const char* szSQL, SGMTOOL_DATA_CHAR_SIMPLE_MAP &mapResult );

public:
	int CharFind_UserNumber( int nUserNum, SGMTOOL_DATA_CHAR_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int CharFind_ChaNumber( int nChaNum, SGMTOOL_DATA_CHAR_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int CharFind_ChaName( std::string strChaName, SGMTOOL_DATA_CHAR_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int CharFind_Online( SGMTOOL_DATA_CHAR_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int CharOnlineCheck( int nCHANUM, BOOL& bONLINE );
	int CharSetDeleted( int nCHANUM, BOOL bDeleted );
	int CharSetOnline( int nCHANUM, BOOL bOnline );
	int CharDataGet( int nUserNumber, int nChaNumber, SCHARDATA2* pCharData2 );
	int CharNameCheck( std::string strCHANAME, BOOL &bEXIST, int &nCHANUM );
	int CharDataSave( SCHARDATA2* pCharData2 );
	int CharDataCopy( int nUserNum, SCHARDATA2* pCharData2 );
	int CharFind_ALL( SGMTOOL_DATA_CHAR_SIMPLE_MAP &mapResult );

public:
	int UserInvenValidate( SCHARDATA2* pCharData2 );
	int UserInvenGetData( SCHARDATA2* pCharData2 );
	int UserInvenGetBinary( DWORD dwUserNum, CByteStream &ByteStream );

	int UserInvenSaveData( SCHARDATA2* pCharData2 );
	int UserInvenSaveBinary( DWORD dwUserNum, BYTE* pData, int nSize );

public:
	int UserBankGet( std::string strUSER, SGMTOOL_USER_BANK_ITEM_MAP &mapResult );
	int UserBankMarkTaken( std::string strUSER, DWORD dwPurKey );
	int UserBankWipe( std::string strUSER );
	int UserBankInsert( std::string strUSER, DWORD dwPRODUCTNUM );
	int UserBankInsertAll( DWORD dwPRODUCTNUM );

public:
	int ShopGetData( SGMTOOL_SHOP_ITEM_MAP &mapResult );
	DWORD ShopItemNew( SGMTOOL_SHOP_ITEM* pSHOPITEM );
	int ShopItemUpdate( SGMTOOL_SHOP_ITEM* pSHOPITEM );
	int ShopItemDelete( SGMTOOL_SHOP_ITEM* sSHOPITEM );

public:
	int PetFind( const char* szSQL, SGMTOOL_PETDATA_SIMPLE_MAP &mapResult );
	int PetFind_Name( std::string strPETNAME, SGMTOOL_PETDATA_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int PetFind_PetID( int nPETID, SGMTOOL_PETDATA_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int PetFind_CharID( int nCHARID, SGMTOOL_PETDATA_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int PetFind_MID( int nMID, SGMTOOL_PETDATA_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int PetFind_SID( int nSID, SGMTOOL_PETDATA_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int PetFind_Type( int nTYPE, SGMTOOL_PETDATA_SIMPLE_MAP &mapResult, int nLIMITNUM );

	int PetDataGet( DWORD dwPETID, DWORD dwPETOWNERID, GLPET* pDATA );
	int PetDataGetInven( DWORD dwPETID, DWORD dwPETOWNERID, GLPET* pDATA );
	int PetDataGetSkill( DWORD dwPETID, DWORD dwPETOWNERID, GLPET* pDATA );

	int PetDataSave( GLPET* pDATA );
	int PetDataSaveInven( int nPETNUM, int nPETCHANUM, CByteStream &ByteStream );
	int PetDataClearSkills( int nPETNUM, int nPETCHANUM );
	int PetDataSaveSkill( int nPETNUM, int nPETCHANUM, int nPetInvenType,  int nPetInvenMID, int nPetInvenSID, int nPetInvenCMID, int nPetInvenCSID, int nPetInvenAvailable);

public:
	int VehicleFind( const char* szSQL, SGMTOOL_VEHICLEDATA_SIMPLE_MAP &mapResult );
	int VehicleFind_Name( std::string strVehicleName, SGMTOOL_VEHICLEDATA_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int VehicleFind_VehicleID( int nVehicleID, SGMTOOL_VEHICLEDATA_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int VehicleFind_CharID( int nCharID, SGMTOOL_VEHICLEDATA_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int VehicleFind_MID( int nMID, SGMTOOL_VEHICLEDATA_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int VehicleFind_SID( int nMID, SGMTOOL_VEHICLEDATA_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int VehicleFind_Type( int nTYPE, SGMTOOL_VEHICLEDATA_SIMPLE_MAP &mapResult, int nLIMITNUM );

	int VehicleDataGet( DWORD dwVEHICLENUM, DWORD dwVEHICLEOWNERNUM, GLVEHICLE* pDATA );
	int VehicleDataGetInven( DWORD dwVEHICLENUM, DWORD dwVEHICLEOWNERNUM, GLVEHICLE* pDATA );

	int VehicleDataSave( GLVEHICLE* pDATA );
	int VehicleDataSaveInven( int nVEHICLENUM, int nVEHICLECHANUM, CByteStream &ByteStream );

	/*bike color , Juver, 2017/11/13 */
	int VehicleDataGetColor( DWORD dwVEHICLENUM, DWORD dwVEHICLEOWNERNUM, GLVEHICLE* pDATA );
	int VehicleDataSaveColor( int nVEHICLENUM, int nVEHICLECHANUM, CByteStream &ByteStream );

public:
	/*equipment lock, Juver, 2018/01/15 */
	int EquipmentLock_Find( const char* szSQL, SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE_MAP &mapResult );
	int EquipmentLock_Find_ChaNum( int nChaNum, SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int EquipmentLock_Find_ChaName( std::string strChaName, SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int EquipmentLock_Find_EPin( std::string strEPin, SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE_MAP &mapResult, int nLIMITNUM );
	int EquipmentLock_Delete( int nNum );
	int EquipmentLock_Delete_To_Char( int nNum );
	int EquipmentLock_Get( int nNUM, SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE* pDATA );
	int EquipmentLock_Save( SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE* pDATA );
	int EquipmentLock_New( SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE* pDATA );
};

#endif // _GMTOOL_ODBC_BASE_H_