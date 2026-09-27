#ifndef _GMTOOL_DATA_STRUCT_
#define _GMTOOL_DATA_STRUCT_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <vector>

#include "s_NetGlobal.h"
#include "GLCharDefine.h"

#define MAX_PRESET_ITEM_ID	254	//max number of item presets
#define MAX_PRESET_SKILL_ID 254	//max number of skill presets

#define MIN_STRING_SEARCH 3		//minimum string size for search

#define MAX_USER_ID 12
#define MIN_USER_ID 3
#define MAX_USER_PASS  USR_PASS_LENGTH		//max userpass size
#define MIN_USER_PASS 3

#define USR_EMAIL_LENGTH	50	//max size for useremail

#define MAX_CHAR_ID 16
#define MIN_CHAR_ID 4

#define SHOP_ITEMNAME_LENGTH	100 //size of itemname text in shopitemmap
#define SHOP_DURATION_LENGTH	50 //size of duration text in shopitemmap
#define SHOP_ITEMIMAGE_LENGTH	300 //size of itemimage text in shopitemmap
#define SHOP_ITEMCOMMENT_LENGTH	50 //size of itemcomment text in shopitemmap

#define PET_NAME_LENGTH		20 //size of Pet Name text in PetInfo
#define VEHICLE_NAME_LENGTH		20 //size of Vehicle Name text in VehicleInfo

//item preset
struct SGMTOOL_PRESET_ITEM
{
	WORD wID;
	std::string strNAME;
	BYTE cDAMAGE;
	BYTE cDEFENSE;
	BYTE cRESFIRE;
	BYTE cRESICE;
	BYTE cRESELECT;
	BYTE cRESPOISON;
	BYTE cRESSPIRIT;
	BYTE cOPTIONTYPE1;
	BYTE cOPTIONTYPE2;
	BYTE cOPTIONTYPE3;
	BYTE cOPTIONTYPE4;
	short nOPTIONVAL1;
	short nOPTIONVAL2;
	short nOPTIONVAL3;
	short nOPTIONVAL4;
	bool bNONDROP;

	SGMTOOL_PRESET_ITEM()
		: wID(0)
		, cDAMAGE(0)
		, cDEFENSE(0)
		, cRESFIRE(0)
		, cRESICE(0)
		, cRESELECT(0)
		, cRESPOISON(0)
		, cRESSPIRIT(0)
		, cOPTIONTYPE1(0)
		, cOPTIONTYPE2(0)
		, cOPTIONTYPE3(0)
		, cOPTIONTYPE4(0)
		, nOPTIONVAL1(0)
		, nOPTIONVAL2(0)
		, nOPTIONVAL3(0)
		, nOPTIONVAL4(0)
		, bNONDROP( false )
	{
		strNAME = "";
	};
};

typedef std::map<WORD, SGMTOOL_PRESET_ITEM>			SGMTOOL_PRESET_ITEM_MAP;
typedef SGMTOOL_PRESET_ITEM_MAP::iterator			SGMTOOL_PRESET_ITEM_MAP_ITER;

struct SGMTOOL_PRESET_SKILL_DATA
{
	SNATIVEID	sSKILLID;
	WORD		wLEVEL;

	SGMTOOL_PRESET_SKILL_DATA()
		: sSKILLID(NATIVEID_NULL())
		, wLEVEL(0)
	{
	};
};


typedef std::map<DWORD, SGMTOOL_PRESET_SKILL_DATA>				SGMTOOL_PRESET_SKILL_DATA_MAP;
typedef SGMTOOL_PRESET_SKILL_DATA_MAP::iterator					SGMTOOL_PRESET_SKILL_DATA_MAP_ITER;

struct SGMTOOL_PRESET_SKILL
{
	WORD		wID;
	std::string strNAME;
	SGMTOOL_PRESET_SKILL_DATA_MAP	mapData;

	SGMTOOL_PRESET_SKILL()
		: wID(0)
	{
		strNAME = "";
	};
};


typedef std::map<WORD, SGMTOOL_PRESET_SKILL>			SGMTOOL_PRESET_SKILL_MAP;
typedef SGMTOOL_PRESET_SKILL_MAP::iterator				SGMTOOL_PRESET_SKILL_MAP_ITER;


struct SGMTOOL_USER_GM
{
	DWORD		dwUserNum;
	TCHAR		szUserID[USR_ID_LENGTH];
	WORD		wUserType;
	BOOL		bValid;

	SGMTOOL_USER_GM()
		: dwUserNum(0)
		, wUserType(0)
		, bValid(FALSE)
	{
		SecureZeroMemory( szUserID, sizeof(char)*USR_ID_LENGTH );
	};
};

//simple user information
struct SGMTOOL_USER_SIMPLE
{
	DWORD		dwUserNum;
	TCHAR		szUserID[USR_ID_LENGTH];
	TCHAR		szUserEmail[USR_EMAIL_LENGTH];
	WORD		wUserType;
	BOOL		bUserLoginState;
	BOOL		bUserAvailable;
	BOOL		bUserBlock;
	__time64_t	tUserBlockDate;
	__time64_t	tChatBlockDate;
	DWORD		dwUserPoint;

	SGMTOOL_USER_SIMPLE()
		: dwUserNum(0)
		, wUserType(0)
		, bUserLoginState(FALSE)
		, bUserAvailable(FALSE)
		, bUserBlock(FALSE)
		, tUserBlockDate(0)
		, tChatBlockDate(0)
		, dwUserPoint(0)	
	{
		SecureZeroMemory( szUserID, sizeof(char)*USR_ID_LENGTH );
		SecureZeroMemory( szUserEmail, sizeof(char)*USR_EMAIL_LENGTH );
	};
};

typedef std::map<DWORD, SGMTOOL_USER_SIMPLE>			SGMTOOL_USER_SIMPLE_MAP;
typedef SGMTOOL_USER_SIMPLE_MAP::iterator				SGMTOOL_USER_SIMPLE_MAP_ITER;

//user new
struct SGMTOOL_USER_NEW
{
	TCHAR		szUserID[USR_ID_LENGTH];
	TCHAR		szUserPass[USR_PASS_LENGTH];
	TCHAR		szUserPass2[USR_PASS_LENGTH];
	TCHAR		szUserEmail[USR_EMAIL_LENGTH];
	WORD		wUserType;
	WORD		wChaRemain;
	DWORD		dwUserPoint;

	SGMTOOL_USER_NEW()
		: wUserType(0)
		, wChaRemain(0)
		, dwUserPoint(0)
	{
		memset( szUserID, 0, sizeof(char)*USR_ID_LENGTH );
		memset( szUserPass, 0, sizeof(char)*USR_PASS_LENGTH );
		memset( szUserPass2, 0, sizeof(char)*USR_PASS_LENGTH );
		memset( szUserEmail, 0, sizeof(char)*USR_EMAIL_LENGTH );
	};
};

//read and save user
struct SGMTOOL_DATA_USER
{
	DWORD		dwUserNum;
	TCHAR		szUserID[USR_ID_LENGTH];
	TCHAR		szUserPass[USR_PASS_LENGTH];
	TCHAR		szUserPass2[USR_PASS_LENGTH];
	TCHAR		szUserEmail[USR_EMAIL_LENGTH];
	WORD		wUserType;
	WORD		wUserType2;
	BOOL		bUserLoginState;
	BOOL		bUserAvailable;
	WORD		wChaRemain;
	DWORD		dwUserPoint;
	BOOL		bUserBlock;
	__time64_t	tUserBlockDate;
	__time64_t	tChatBlockDate;
	__time64_t	tPremiumDate;

	//dont save back to db just for info only
	__time64_t	tCreateDate;
	__time64_t	tLastLoginDate;
	WORD		wServerGroup;
	WORD		wServerNumber;

	SGMTOOL_DATA_USER()
		: dwUserNum(0)
		, wUserType(0)
		, wUserType2(0)
		, bUserLoginState(FALSE)
		, bUserAvailable(FALSE)
		, wChaRemain(0)
		, dwUserPoint(0)
		, bUserBlock(FALSE)
		, tUserBlockDate(0)
		, tChatBlockDate(0)
		, tPremiumDate(0)
		, wServerGroup(0)
		, wServerNumber(0)
	{
		memset( szUserID, 0, sizeof(char)*USR_ID_LENGTH );
		memset( szUserPass, 0, sizeof(char)*USR_PASS_LENGTH );
		memset( szUserPass2, 0, sizeof(char)*USR_PASS_LENGTH );
		memset( szUserEmail, 0, sizeof(char)*USR_EMAIL_LENGTH );
	};
};

//user loglogin data
struct SGMTOOL_DATA_USER_IPLOG
{
	TCHAR		szLogIpAddress[KOR_USERIP];
	__time64_t	tLogDate;
	
	SGMTOOL_DATA_USER_IPLOG()
		: tLogDate(0)
	{
		memset( szLogIpAddress, 0, sizeof(char)*KOR_USERIP );
	};
};

typedef std::vector< SGMTOOL_DATA_USER_IPLOG>			SGMTOOL_DATA_USER_IPLOG_VEC;
typedef SGMTOOL_DATA_USER_IPLOG_VEC::iterator			SGMTOOL_DATA_USER_IPLOG_VEC_ITER;

typedef std::map<std::string, SGMTOOL_DATA_USER_IPLOG>				SGMTOOL_DATA_USER_IPLOG_MAP;
typedef SGMTOOL_DATA_USER_IPLOG_MAP::iterator						SGMTOOL_DATA_USER_IPLOG_MAP_ITER;

//user character list
struct SGMTOOL_DATA_USER_CHARACTERS
{	
	DWORD		dwChaNum;
	TCHAR		szChaName[CHR_ID_LENGTH];
	WORD		wChaClass;
	WORD		wChaSchool;
	WORD		wChaLevel;
	BOOL		bChaDeleted;

	SGMTOOL_DATA_USER_CHARACTERS()
		: dwChaNum(0)
		, wChaClass(0)
		, wChaSchool(0)
		, wChaLevel(0)
		, bChaDeleted(FALSE)
	{
		memset( szChaName, 0, sizeof(char)*CHR_ID_LENGTH );
	};
};

typedef std::vector< SGMTOOL_DATA_USER_CHARACTERS>			SGMTOOL_DATA_USER_CHARACTERS_VEC;
typedef SGMTOOL_DATA_USER_CHARACTERS_VEC::iterator			SGMTOOL_DATA_USER_CHARACTERS_VEC_ITER;

//simple char
struct SGMTOOL_DATA_CHAR_SIMPLE
{	
	DWORD		dwUserNum;
	DWORD		dwChaNum;
	TCHAR		szChaName[CHR_ID_LENGTH];
	WORD		wChaClass;
	WORD		wChaSchool;
	WORD		wChaLevel;
	BOOL		bChaOnline;
	BOOL		bChaDeleted;

	SGMTOOL_DATA_CHAR_SIMPLE()
		: dwUserNum(0)
		, dwChaNum(0)
		, wChaClass(0)
		, wChaSchool(0)
		, wChaLevel(0)
		, bChaOnline(FALSE)
		, bChaDeleted(FALSE)
	{
		memset( szChaName, 0, sizeof(char)*CHR_ID_LENGTH );
	};
};

typedef std::map<DWORD, SGMTOOL_DATA_CHAR_SIMPLE>				SGMTOOL_DATA_CHAR_SIMPLE_MAP;
typedef SGMTOOL_DATA_CHAR_SIMPLE_MAP::iterator					SGMTOOL_DATA_CHAR_SIMPLE_MAP_ITER;

//skill insert
struct SGMTOOL_SKILL_DATA_SELECTED
{	
	SNATIVEID	sID;
	std::string strNAME;
	WORD		wLEVEL;

	SGMTOOL_SKILL_DATA_SELECTED()
		: sID(NATIVEID_NULL())
		, strNAME("")
		, wLEVEL(0)
	{
	};
};

typedef std::vector< SGMTOOL_SKILL_DATA_SELECTED>				SGMTOOL_SKILL_DATA_SELECTED_VEC;
typedef SGMTOOL_SKILL_DATA_SELECTED_VEC::iterator				SGMTOOL_SKILL_DATA_SELECTED_VEC_ITER;

typedef std::map<DWORD, SGMTOOL_SKILL_DATA_SELECTED>			SGMTOOL_SKILL_DATA_SELECTED_MAP;
typedef SGMTOOL_SKILL_DATA_SELECTED_MAP::iterator				SGMTOOL_SKILL_DATA_SELECTED_MAP_ITER;

struct SGMTOOL_SKILL_DATA_SELECTION
{	
	SNATIVEID	sID;
	std::string strNAME;

	SGMTOOL_SKILL_DATA_SELECTION()
		: sID(NATIVEID_NULL())
		, strNAME("")
	{
	};
};

typedef std::vector< SGMTOOL_SKILL_DATA_SELECTION>				SGMTOOL_SKILL_DATA_SELECTION_VEC;
typedef SGMTOOL_SKILL_DATA_SELECTION_VEC::iterator				SGMTOOL_SKILL_DATA_SELECTION_VEC_ITER;



struct SGMTOOL_USER_BANK_ITEM
{
	DWORD		dwPurKey;
	DWORD		dwProductNum;
	DWORD		dwPrice;
	__time64_t	tPurchaseDate;

	//for item info
	TCHAR		szItemName[SHOP_ITEMNAME_LENGTH];
	WORD		wItemMain;
	WORD		wItemSub;

	SGMTOOL_USER_BANK_ITEM()
		: dwPurKey( 0 )
		, dwProductNum( 0 )
		, dwPrice( 0 )
		, tPurchaseDate( 0 )
		, wItemMain( SNATIVEID::ID_NULL )
		, wItemSub( SNATIVEID::ID_NULL )
	{
		memset( szItemName, 0, sizeof(char)*SHOP_ITEMNAME_LENGTH );
	};
};

typedef std::map<DWORD, SGMTOOL_USER_BANK_ITEM>			SGMTOOL_USER_BANK_ITEM_MAP;
typedef SGMTOOL_USER_BANK_ITEM_MAP::iterator			SGMTOOL_USER_BANK_ITEM_MAP_ITER;

struct SGMTOOL_SHOP_ITEM
{
	DWORD		dwProductNum;
	WORD		wItemMain;
	WORD		wItemSub;
	TCHAR		szItemName[SHOP_ITEMNAME_LENGTH];
	WORD		wItemList;
	TCHAR		szDuration[SHOP_DURATION_LENGTH];
	DWORD		dwCategory;
	DWORD		dwItemStock;
	TCHAR		szItemImage[SHOP_ITEMIMAGE_LENGTH];
	DWORD		dwItemMoney;
	TCHAR		szItemComment[SHOP_ITEMCOMMENT_LENGTH];
	WORD		wItemSec;

	SGMTOOL_SHOP_ITEM()
		: dwProductNum( SNATIVEID::ID_NULL )
		, wItemMain( SNATIVEID::ID_NULL )
		, wItemSub( SNATIVEID::ID_NULL )
		, wItemList( 0 )
		, dwCategory( 0 )
		, dwItemStock( 0 )
		, dwItemMoney( 0 )
		, wItemSec( 0 )
	{
		memset( szItemName, 0, sizeof(char)*SHOP_ITEMNAME_LENGTH );
		memset( szDuration, 0, sizeof(char)*SHOP_DURATION_LENGTH );
		memset( szItemImage, 0, sizeof(char)*SHOP_ITEMIMAGE_LENGTH );
		memset( szItemComment, 0, sizeof(char)*SHOP_ITEMCOMMENT_LENGTH );
	};
};

typedef std::map<DWORD, SGMTOOL_SHOP_ITEM>				SGMTOOL_SHOP_ITEM_MAP;
typedef SGMTOOL_SHOP_ITEM_MAP::iterator					SGMTOOL_SHOP_ITEM_MAP_ITER;

typedef std::vector< SGMTOOL_SHOP_ITEM>					SGMTOOL_SHOP_ITEM_VEC;
typedef SGMTOOL_SHOP_ITEM_VEC::iterator					SGMTOOL_SHOP_ITEM_VEC_ITER;


struct SGMTOOL_PETDATA_SIMPLE
{
	DWORD	dwPetNum;
	DWORD	dwPetChaNum; //owner
	TCHAR	szPetName[PET_NAME_LENGTH];
	SNATIVEID sIDPET;
	WORD	wPetType;

	SGMTOOL_PETDATA_SIMPLE ()
		: dwPetNum(0)
		, dwPetChaNum(0)
		, wPetType( 0 )
	{
		memset( szPetName, 0, sizeof(char)*PET_NAME_LENGTH );
	};
};

typedef std::map<DWORD, SGMTOOL_PETDATA_SIMPLE>				SGMTOOL_PETDATA_SIMPLE_MAP;
typedef SGMTOOL_PETDATA_SIMPLE_MAP::iterator				SGMTOOL_PETDATA_SIMPLE_MAP_ITER;

struct SGMTOOL_VEHICLEDATA_SIMPLE
{
	DWORD	dwVehicleNum;
	DWORD	dwVehicleChaNum; //owner
	TCHAR	szVehicleName[VEHICLE_NAME_LENGTH];
	SNATIVEID sIDVEHICLE;
	WORD	wVehicleType;

	SGMTOOL_VEHICLEDATA_SIMPLE ()
		: dwVehicleNum(0)
		, dwVehicleChaNum(0)
		, wVehicleType( 0 )
	{
		memset( szVehicleName, 0, sizeof(char)*VEHICLE_NAME_LENGTH );
	};
};

typedef std::map<DWORD, SGMTOOL_VEHICLEDATA_SIMPLE>			SGMTOOL_VEHICLEDATA_SIMPLE_MAP;
typedef SGMTOOL_VEHICLEDATA_SIMPLE_MAP::iterator			SGMTOOL_VEHICLEDATA_SIMPLE_MAP_ITER;

/*equipment lock, Juver, 2018/01/15 */
struct SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE
{
	DWORD	dwNum;
	DWORD	dwChaNum; 
	TCHAR	szChaName[CHR_ID_LENGTH+1];
	TCHAR	szEPin[CHAR_EQUIPMENT_LOCK_PASS_SIZE+1];
	__time64_t	tDate;

	SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE ()
		: dwNum(0)
		, dwChaNum(0)
		, tDate(0)
	{
		memset( szChaName, 0, sizeof(char)*CHR_ID_LENGTH+1 );
		memset( szEPin, 0, sizeof(char)*CHAR_EQUIPMENT_LOCK_PASS_SIZE+1 );
	};
};

typedef std::map<DWORD, SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE>	SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE_MAP;
typedef SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE_MAP::iterator	SGMTOOL_EQUIPMENT_LOCK_DATA_SIMPLE_MAP_ITER;

#endif // _GMTOOL_DATA_STRUCT_