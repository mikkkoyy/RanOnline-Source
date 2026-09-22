#include "stdafx.h"
#include "GLMapNode.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


SMAPNODE& SMAPNODE::operator = ( const SMAPNODE& value )
{
	if( this == &value )
		return *this;

	strFile					= value.strFile;
	sNativeID				= value.sNativeID;
	dwFieldSID				= value.dwFieldSID;

	bPeaceZone				= value.bPeaceZone;
	bCommission				= value.bCommission;
	bPKZone					= value.bPKZone;
	bFreePK					= value.bFreePK;
	bItemDrop				= value.bItemDrop;
	bMove					= value.bMove;
	bRestart				= value.bRestart;
	bPetActivity			= value.bPetActivity;
	bDECEXP					= value.bDECEXP;
	bVehicleActivity		= value.bVehicleActivity;
	bClubBattleZone			= value.bClubBattleZone;
	
	bInstantMap				= value.bInstantMap;
	bQBoxEnable				= value.bQBoxEnable;

	bLunchBoxForbid			= value.bLunchBoxForbid;
	bCPReset				= value.bCPReset; /*combatpoint logic, Juver, 2017/05/29 */
	bPKMap					= value.bPKMap;

	bUIMapSelect			= value.bUIMapSelect;
	bUIMapInfo				= value.bUIMapInfo;
	bClubPKRecord			= value.bClubPKRecord;
	bOpenPrivateMarket		= value.bOpenPrivateMarket;		/* map private market setting, Juver, 2017/10/02 */
	bPartySparring			= value.bPartySparring;

	/*map move settings, Juver, 2017/11/24 */
	bBlockTaxi				= value.bBlockTaxi;
	bBlockFriendCard		= value.bBlockFriendCard;
	bBlockRecall			= value.bBlockRecall;
	bBlockTeleport			= value.bBlockTeleport;

	/*global buffs map setting, Juver, 2018/01/23 */
	bDisableSystemBuffs		= value.bDisableSystemBuffs;

	/*hp potion map setting, Juver, 2018/01/23 */
	bBlockHPPotion			= value.bBlockHPPotion;

	strMapName				= value.strMapName;
	strBGM					= value.strBGM;
	strLoadingImageName		= value.strLoadingImageName;

	bBattleZone				= value.bBattleZone;
	bCDMZone				= value.bCDMZone;
	bPVPTyrannyZone			= value.bPVPTyrannyZone;		/*pvp tyranny, Juver, 2017/08/25 */
	bPVPSchoolWarsZone		= value.bPVPSchoolWarsZone;		/*school wars, Juver, 2018/01/19 */
	bPVPCaptureTheFlagZone	= value.bPVPCaptureTheFlagZone;	/*pvp capture the flag, Juver, 2018/01/24 */


	sLEVEL_REQUIRE			= value.sLEVEL_REQUIRE;
	sLEVEL_ETC_FUNC			= value.sLEVEL_ETC_FUNC;
	
	return *this;
}

SMAPNODE& SMAPNODE::operator = ( const SMAPNODE_DATA& value )
{
	strFile					= value.strFile;
	sNativeID				= value.sNativeID;
	dwFieldSID				= value.dwFieldSID;

	bPeaceZone				= value.bPeaceZone;
	bCommission				= !value.bCommission;
	bPKZone					= value.bPKZone;
	bFreePK					= value.bFreePK;
	bItemDrop				= value.bItemDrop;
	bMove					= value.bMove;
	bRestart				= value.bRestart;
	bPetActivity			= value.bPetActivity;
	bDECEXP					= value.bDECEXP;
	bVehicleActivity		= value.bVehicleActivity;
	bClubBattleZone			= value.bClubBattleZone;

	bInstantMap				= value.bInstantMap;
	bQBoxEnable				= value.bQBoxEnable;

	bLunchBoxForbid			= value.bLunchBoxForbid;
	bCPReset				= value.bCPReset; /*combatpoint logic, Juver, 2017/05/29 */
	bPKMap					= value.bPKMap;

	bUIMapSelect			= value.bUIMapSelect;
	bUIMapInfo				= value.bUIMapInfo;
	bClubPKRecord			= value.bClubPKRecord;
	bOpenPrivateMarket		= value.bOpenPrivateMarket; /* map private market setting, Juver, 2017/10/02 */
	bPartySparring			= value.bPartySparring;

	/*map move settings, Juver, 2017/11/24 */
	bBlockTaxi				= value.bBlockTaxi;
	bBlockFriendCard		= value.bBlockFriendCard;
	bBlockRecall			= value.bBlockRecall;
	bBlockTeleport			= value.bBlockTeleport;

	/*global buffs map setting, Juver, 2018/01/23 */
	bDisableSystemBuffs		= value.bDisableSystemBuffs;

	/*hp potion map setting, Juver, 2018/01/23 */
	bBlockHPPotion			= value.bBlockHPPotion;

	strMapName				= value.strMapName;
	strBGM					= value.strBGM;
	strLoadingImageName		= value.strLoadingImageName;


	bBattleZone				= false;
	bCDMZone				= false;
	bPVPTyrannyZone			= false;	/*pvp tyranny, Juver, 2017/08/25 */
	bPVPSchoolWarsZone		= false;	/*school wars, Juver, 2018/01/19 */
	bPVPCaptureTheFlagZone	= false;	/*pvp capture the flag, Juver, 2018/01/24 */

	sLEVEL_REQUIRE  = SLEVEL_REQUIRE();
	sLEVEL_ETC_FUNC = SLEVEL_ETC_FUNC();

	return *this;
}

BOOL SMAPNODE_DATA::LOAD ( basestream &SFile )
{
	DWORD dwVer(0);

	SFile >> dwVer;

	if ( dwVer == VERSION )
	{
		SFile >> bUsed;
		SFile >> strFile;
		SFile >> sNativeID.wMainID;
		SFile >> sNativeID.wSubID;
		SFile >> dwFieldSID;

		SFile >> bPeaceZone;
		SFile >> bCommission;
		SFile >> bPKZone;
		SFile >> bFreePK;
		SFile >> bItemDrop;
		SFile >> bMove;
		SFile >> bRestart;
		SFile >> bPetActivity;
		SFile >> bDECEXP;
		SFile >> bVehicleActivity;
		SFile >> bClubBattleZone;				

		SFile >> strMapName;
		SFile >> strBGM;
		SFile >> strLoadingImageName;

		SFile >> bInstantMap;
		SFile >> bQBoxEnable;

		SFile >> bLunchBoxForbid;
		SFile >> bCPReset; /*combatpoint logic, Juver, 2017/05/29 */
		SFile >> bPKMap;

		SFile >> bUIMapSelect;
		SFile >> bUIMapInfo;
		SFile >> bClubPKRecord;
		SFile >> bOpenPrivateMarket; /* map private market setting, Juver, 2017/10/02 */
		SFile >> bPartySparring;

		/*map move settings, Juver, 2017/11/25 */
		SFile >> bBlockTaxi;
		SFile >> bBlockFriendCard;
		SFile >> bBlockRecall;
		SFile >> bBlockTeleport;

		/*global buffs map setting, Juver, 2018/01/23 */
		SFile >> bDisableSystemBuffs;

		/*hp potion map setting, Juver, 2018/01/23 */
		SFile >> bBlockHPPotion;
	}
	else if ( dwVer == 0x0202 )
	{
		SFile >> bUsed;
		SFile >> strFile;
		SFile >> sNativeID.wMainID;
		SFile >> sNativeID.wSubID;
		SFile >> dwFieldSID;

		SFile >> bPeaceZone;
		SFile >> bCommission;
		SFile >> bPKZone;
		SFile >> bFreePK;
		SFile >> bItemDrop;
		SFile >> bMove;
		SFile >> bRestart;
		SFile >> bPetActivity;
		SFile >> bDECEXP;
		SFile >> bVehicleActivity;
		SFile >> bClubBattleZone;				

		SFile >> strMapName;
		SFile >> strBGM;
		SFile >> strLoadingImageName;

		SFile >> bInstantMap;
		SFile >> bQBoxEnable;

		SFile >> bLunchBoxForbid;
		SFile >> bCPReset; /*combatpoint logic, Juver, 2017/05/29 */
		SFile >> bPKMap;

		SFile >> bUIMapSelect;
		SFile >> bUIMapInfo;
		SFile >> bClubPKRecord;
		SFile >> bOpenPrivateMarket; /* map private market setting, Juver, 2017/10/02 */
		SFile >> bPartySparring;

		/*map move settings, Juver, 2017/11/25 */
		SFile >> bBlockTaxi;
		SFile >> bBlockFriendCard;
		SFile >> bBlockRecall;
		SFile >> bBlockTeleport;

		/*global buffs map setting, Juver, 2018/01/23 */
		SFile >> bDisableSystemBuffs;
	}
	else if ( dwVer == 0x0201 )
	{
		SFile >> bUsed;
		SFile >> strFile;
		SFile >> sNativeID.wMainID;
		SFile >> sNativeID.wSubID;
		SFile >> dwFieldSID;

		SFile >> bPeaceZone;
		SFile >> bCommission;
		SFile >> bPKZone;
		SFile >> bFreePK;
		SFile >> bItemDrop;
		SFile >> bMove;
		SFile >> bRestart;
		SFile >> bPetActivity;
		SFile >> bDECEXP;
		SFile >> bVehicleActivity;
		SFile >> bClubBattleZone;				

		SFile >> strMapName;
		SFile >> strBGM;
		SFile >> strLoadingImageName;

		SFile >> bInstantMap;
		SFile >> bQBoxEnable;

		SFile >> bLunchBoxForbid;
		SFile >> bCPReset; /*combatpoint logic, Juver, 2017/05/29 */
		SFile >> bPKMap;

		SFile >> bUIMapSelect;
		SFile >> bUIMapInfo;
		SFile >> bClubPKRecord;
		SFile >> bOpenPrivateMarket; /* map private market setting, Juver, 2017/10/02 */
		SFile >> bPartySparring;

		/*map move settings, Juver, 2017/11/25 */
		SFile >> bBlockTaxi;
		SFile >> bBlockFriendCard;
		SFile >> bBlockRecall;
		SFile >> bBlockTeleport;
	}
	else if ( dwVer == 0x0200 )
	{
		SFile >> bUsed;
		SFile >> strFile;
		SFile >> sNativeID.wMainID;
		SFile >> sNativeID.wSubID;
		SFile >> dwFieldSID;

		SFile >> bPeaceZone;
		SFile >> bCommission;
		SFile >> bPKZone;
		SFile >> bFreePK;
		SFile >> bItemDrop;
		SFile >> bMove;
		SFile >> bRestart;
		SFile >> bPetActivity;
		SFile >> bDECEXP;
		SFile >> bVehicleActivity;
		SFile >> bClubBattleZone;				

		SFile >> strMapName;
		SFile >> strBGM;
		SFile >> strLoadingImageName;

		SFile >> bInstantMap;
		SFile >> bQBoxEnable;

		SFile >> bLunchBoxForbid;
		SFile >> bCPReset; /*combatpoint logic, Juver, 2017/05/29 */
		SFile >> bPKMap;

		SFile >> bUIMapSelect;
		SFile >> bUIMapInfo;
		SFile >> bClubPKRecord;
		SFile >> bOpenPrivateMarket; /* map private market setting, Juver, 2017/10/02 */
		SFile >> bPartySparring;
	}
	else if ( dwVer == 0x0103 )
	{
		SFile >> bUsed;
		SFile >> strFile;
		SFile >> sNativeID.wMainID;
		SFile >> sNativeID.wSubID;
		SFile >> dwFieldSID;

		SFile >> bPeaceZone;
		SFile >> bCommission;
		SFile >> bPKZone;
		SFile >> bFreePK;
		SFile >> bItemDrop;
		SFile >> bMove;
		SFile >> bRestart;
		SFile >> bPetActivity;
		SFile >> bDECEXP;
		SFile >> bVehicleActivity;
		SFile >> bClubBattleZone;				

		SFile >> strMapName;
		SFile >> strBGM;
		SFile >> strLoadingImageName;

		SFile >> bInstantMap;
		SFile >> bQBoxEnable;

		SFile >> bLunchBoxForbid;
		SFile >> bCPReset; /*combatpoint logic, Juver, 2017/05/29 */
		SFile >> bPKMap;
	}
	else if ( dwVer == 0x0102 )
	{
		SFile >> bUsed;
		SFile >> strFile;
		SFile >> sNativeID.wMainID;
		SFile >> sNativeID.wSubID;
		SFile >> dwFieldSID;

		SFile >> bPeaceZone;
		SFile >> bCommission;
		SFile >> bPKZone;
		SFile >> bFreePK;
		SFile >> bItemDrop;
		SFile >> bMove;
		SFile >> bRestart;
		SFile >> bPetActivity;
		SFile >> bDECEXP;
		SFile >> bVehicleActivity;
		SFile >> bClubBattleZone;				

		SFile >> strMapName;
		SFile >> strBGM;
		SFile >> strLoadingImageName;

		SFile >> bInstantMap;
		SFile >> bQBoxEnable;

		SFile >> bLunchBoxForbid;
		SFile >> bCPReset; /*combatpoint logic, Juver, 2017/05/29 */
	}
	else if ( dwVer == 0x0101 )
	{
		SFile >> bUsed;
		SFile >> strFile;
		SFile >> sNativeID.wMainID;
		SFile >> sNativeID.wSubID;
		SFile >> dwFieldSID;

		SFile >> bPeaceZone;
		SFile >> bCommission;
		SFile >> bPKZone;
		SFile >> bFreePK;
		SFile >> bItemDrop;
		SFile >> bMove;
		SFile >> bRestart;
		SFile >> bPetActivity;
		SFile >> bDECEXP;
		SFile >> bVehicleActivity;
		SFile >> bClubBattleZone;				

		SFile >> strMapName;
		SFile >> strBGM;
		SFile >> strLoadingImageName;

		SFile >> bInstantMap;
		SFile >> bQBoxEnable;

		SFile >> bLunchBoxForbid;
	}
	else if ( dwVer == 0x0100 )
	{
		SFile >> bUsed;
		SFile >> strFile;
		SFile >> sNativeID.wMainID;
		SFile >> sNativeID.wSubID;
		SFile >> dwFieldSID;

		SFile >> bPeaceZone;
		SFile >> bCommission;
		SFile >> bPKZone;
		SFile >> bFreePK;
		SFile >> bItemDrop;
		SFile >> bMove;
		SFile >> bRestart;
		SFile >> bPetActivity;
		SFile >> bDECEXP;
		SFile >> bVehicleActivity;
		SFile >> bClubBattleZone;				

		SFile >> strMapName;
		SFile >> strBGM;
		SFile >> strLoadingImageName;

		SFile >> bInstantMap;
		SFile >> bQBoxEnable;
	}
	else
	{
		CDebugSet::ErrorVersion( "SMAPNODE_DATA::LOAD", dwVer );
		return FALSE;
	}

	return TRUE;
}

BOOL SMAPNODE_DATA::SAVE ( CSerialFile &SFile )
{
	SFile << (DWORD)VERSION;

	SFile << bUsed;
	SFile << strFile;
	SFile << sNativeID.wMainID;
	SFile << sNativeID.wSubID;
	SFile << dwFieldSID;

	SFile << bPeaceZone;
	SFile << bCommission;
	SFile << bPKZone;
	SFile << bFreePK;
	SFile << bItemDrop;
	SFile << bMove;
	SFile << bRestart;
	SFile << bPetActivity;
	SFile << bDECEXP;
	SFile << bVehicleActivity;
	SFile << bClubBattleZone;				

	SFile << strMapName;
	SFile << strBGM;
	SFile << strLoadingImageName;

	SFile << bInstantMap;
	SFile << bQBoxEnable;

	SFile << bLunchBoxForbid;
	SFile << bCPReset; /*combatpoint logic, Juver, 2017/05/29 */
	SFile << bPKMap;

	SFile << bUIMapSelect;
	SFile << bUIMapInfo;
	SFile << bClubPKRecord;
	SFile << bOpenPrivateMarket; /* map private market setting, Juver, 2017/10/02 */
	SFile << bPartySparring;

	/*map move settings, Juver, 2017/11/25 */
	SFile << bBlockTaxi;
	SFile << bBlockFriendCard;
	SFile << bBlockRecall;
	SFile << bBlockTeleport;

	/*global buffs map setting, Juver, 2018/01/23 */
	SFile << bDisableSystemBuffs;

	/*hp potion map setting, Juver, 2018/01/23 */
	SFile << bBlockHPPotion;

	return TRUE;
}
