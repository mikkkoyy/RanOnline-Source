#ifndef GLMAPNODE_H_
#define GLMAPNODE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "../../Lib_Engine/Common/ByteStream.h"
#include "../../Lib_Engine/Common/SerialFile.h"

#include "./GLLevelFile.h"

enum
{
	MAP_NAME_MAX	= 16,
};

struct SMAPNODE_DATA
{
	enum { VERSION = 0x0203 };

	bool			bUsed;
	std::string		strFile;
	SNATIVEID		sNativeID;
	DWORD			dwFieldSID;

	bool			bPeaceZone;
	bool			bCommission;
	bool			bPKZone;
	bool			bFreePK;
	bool			bItemDrop;
	bool			bMove;
	bool			bRestart;
	bool			bPetActivity;
	bool			bDECEXP;
	bool			bVehicleActivity;
	bool			bClubBattleZone;
	bool			bInstantMap;
	bool			bQBoxEnable;
	bool			bLunchBoxForbid;
	bool			bCPReset; /*combatpoint logic, Juver, 2017/05/29 */
	bool			bPKMap;

	bool			bUIMapSelect;
	bool			bUIMapInfo;
	bool			bClubPKRecord;
	bool			bOpenPrivateMarket; /* map private market setting, Juver, 2017/10/02 */
	bool			bPartySparring;

	/*map move settings, Juver, 2017/11/24 */
	bool			bBlockTaxi;
	bool			bBlockFriendCard;
	bool			bBlockRecall;
	bool			bBlockTeleport;

	/*global buffs map setting, Juver, 2018/01/23 */
	bool			bDisableSystemBuffs;

	/*hp potion map setting, Juver, 2018/01/23 */
	bool			bBlockHPPotion;

	std::string		strMapName;
	std::string		strBGM;
	std::string		strLoadingImageName;

	SMAPNODE_DATA () :
		bUsed(false),
		dwFieldSID(0),

		bPeaceZone(false),
		bCommission(false),
		bPKZone(false),
		bFreePK(false),
		bItemDrop(false),
		bMove(false),
		bRestart(false),
		bDECEXP(true),
		bPetActivity(false),
		bVehicleActivity(false),
		bClubBattleZone(false),
		bInstantMap(false),
		bQBoxEnable(false),
		bLunchBoxForbid(false),
		bCPReset(false), /*combatpoint logic, Juver, 2017/05/29 */
		bPKMap(false),
		bUIMapSelect(false),
		bUIMapInfo(false),
		bClubPKRecord(false),
		bOpenPrivateMarket(false), /* map private market setting, Juver, 2017/10/02 */
		bPartySparring(false),

		/*map move settings, Juver, 2017/11/24 */
		bBlockTaxi(false),
		bBlockFriendCard(false),
		bBlockRecall(false),
		bBlockTeleport(false),

		/*global buffs map setting, Juver, 2018/01/23 */
		bDisableSystemBuffs(false),

		/*hp potion map setting, Juver, 2018/01/23 */
		bBlockHPPotion(false)

	{
	}

	BOOL LOAD ( basestream &SFile );
	BOOL SAVE ( CSerialFile &SFile );
};

struct SMAPNODE
{
	std::string		strFile;
	SNATIVEID		sNativeID;
	DWORD			dwFieldSID;

	bool			bPeaceZone;
	bool			bCommission;
	bool			bPKZone;
	bool			bFreePK;
	bool			bItemDrop;
	bool			bMove;
	bool			bRestart;
	bool			bPetActivity;
	bool			bDECEXP;
	bool			bVehicleActivity;
	bool			bClubBattleZone;
	bool			bInstantMap;
	bool			bQBoxEnable;
	bool			bLunchBoxForbid;
	bool			bCPReset; /*combatpoint logic, Juver, 2017/05/29 */
	bool			bPKMap;

	bool			bUIMapSelect;
	bool			bUIMapInfo;
	bool			bClubPKRecord;
	bool			bOpenPrivateMarket; /* map private market setting, Juver, 2017/10/02 */
	bool			bPartySparring;

	/*map move settings, Juver, 2017/11/24 */
	bool			bBlockTaxi;
	bool			bBlockFriendCard;
	bool			bBlockRecall;
	bool			bBlockTeleport;

	std::string		strMapName;
	std::string		strBGM;
	std::string		strLoadingImageName;

	bool			bBattleZone;	//club war
	bool			bCDMZone;		//club death match
	bool			bPVPTyrannyZone;		/*pvp tyranny, Juver, 2017/08/25 */
	bool			bPVPSchoolWarsZone;		/*school wars, Juver, 2018/01/19 */
	bool			bPVPCaptureTheFlagZone;	/*pvp capture the flag, Juver, 2018/01/24 */

	/*global buffs map setting, Juver, 2018/01/23 */
	bool			bDisableSystemBuffs;

	/*hp potion map setting, Juver, 2018/01/23 */
	bool			bBlockHPPotion;

	SLEVEL_REQUIRE	sLEVEL_REQUIRE;
	SLEVEL_ETC_FUNC sLEVEL_ETC_FUNC;

	SMAPNODE () :
		dwFieldSID(0),
		bPeaceZone(false),
		bCommission(false),
		bPKZone(false),
		bFreePK(false),
		bItemDrop(false),
		bMove(false),
		bRestart(false),
		bDECEXP(true),
		bPetActivity(false),
		bVehicleActivity(false),
		bClubBattleZone(false),
		bInstantMap(false),
		bQBoxEnable(false),
		bLunchBoxForbid(false),
		bCPReset(false), /*combatpoint logic, Juver, 2017/05/29 */
		bPKMap(false),
		bUIMapSelect(false),
		bUIMapInfo(false),
		bClubPKRecord(false),
		bOpenPrivateMarket(false), /* map private market setting, Juver, 2017/10/02 */
		bPartySparring(false),

		/*map move settings, Juver, 2017/11/24 */
		bBlockTaxi(false),
		bBlockFriendCard(false),
		bBlockRecall(false),
		bBlockTeleport(false),

		/*global buffs map setting, Juver, 2018/01/23 */
		bDisableSystemBuffs(false),

		/*hp potion map setting, Juver, 2018/01/23 */
		bBlockHPPotion(false),

		bBattleZone(false),
		bCDMZone(false),
		bPVPTyrannyZone(false),			/*pvp tyranny, Juver, 2017/08/25 */
		bPVPSchoolWarsZone(false),		/*school wars, Juver, 2018/01/19 */
		bPVPCaptureTheFlagZone(false)	/*pvp capture the flag, Juver, 2018/01/24 */
	{
	}

	SMAPNODE ( const SMAPNODE& value )
	{
		operator = ( value );
	}

	SMAPNODE& operator = ( const SMAPNODE& value );


	SMAPNODE ( const SMAPNODE_DATA& value )
	{
		operator = ( value );
	}

	SMAPNODE& operator = ( const SMAPNODE_DATA& value );
};

#endif // GLMAPNODE_H_