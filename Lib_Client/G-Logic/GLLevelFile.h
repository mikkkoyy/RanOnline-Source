#ifndef GLLEVEL_FILE_H
#define GLLEVEL_FILE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <string>

#include "./GLMapAxisInfo.h"
#include "./GLMobSchedule.h"
#include "./GLogicEx.h"
#include "./GLCharDefine.h"

#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/DxCommon/DxCustomTypes.h"
#include "../../Lib_Engine/DxOctree/DxLandDef.h"
#include "../../Lib_Engine/DxOctree/DxLandGateMan.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"
#include "../../Lib_Engine/G-Logic/GLQuadTree.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"

#include "./GLLevelHead.h"
#include "./GLLevelRequire.h"
#include "./GLLevelEtcFunction.h"

struct DXLANDEFF;
typedef DXLANDEFF* PLANDEFF;

struct GLLANDMARK;
typedef GLLANDMARK* PLANDMARK;
typedef std::vector<PLANDMARK>	VECLANDMARK;

const int MAX_CREATE_INSTANT_MAP_NUM	= 200;


class GLLevelFile
{
public:
	enum
	{
		VERSION					= 0x0200,
		VERSION_NEW_ENCODE		= 0x0200,
		VERSION_BEFORE_ENCODE	= 0x0100,
		VERSION_BEFORE_GATE		= 0x0101,
	};

public:
	const static char*		FILE_EXT;
	static char				szPATH[MAX_PATH];
	static char				szLEVELPATH[MAX_PATH];

public:
	static void SetPath ( const char*szPath )	{ StringCchCopy(szPATH,MAX_PATH,szPath); }
	static char* GetPath ()						{ return szPATH; }

	static void SetLevelPath ( const char*szPath )	{ StringCchCopy(szLEVELPATH,MAX_PATH,szPath); }
	static char* GetLevelPath ()					{ return szLEVELPATH; }

protected:
	std::string		m_strFileName;
	DWORD			m_dwFileVer;
	SNATIVEID		m_sMapID;
	bool			m_bPeaceZone; // true:무조건 PK 안됨
	bool			m_bPKZone;    // true:pk 할수 있다 없다를 나타냄/적대관계 설정됨
	bool			m_bFreePK;    // true:성향치가 감소되지 않는다/적대관계 설정됨
	bool			m_bItemDrop;  // true:아이템 드랍이 가능
	bool			m_bPetActivity; // true:팻활동 가능
	bool			m_bDECEXP; // 경험치 드랍 유무
	bool			m_bVehicleActivity;	// true:탈것 활성화 가능
	bool			m_bClubBattleZone;
	bool			m_bQBoxEnable;
	bool			m_bLunchBoxForbid;
	bool			m_bCPReset; /*combatpoint logic, Juver, 2017/05/29 */
	bool			m_bPKMap;

	bool			m_bUIMapSelect;
	bool			m_bUIMapInfo;
	bool			m_bClubPKRecord;
	bool			m_bOpenPrivateMarket;
	bool			m_bPartySparring;

	/*map move settings, Juver, 2017/11/24 */
	bool			m_bBlockTaxi;
	bool			m_bBlockFriendCard;
	bool			m_bBlockRecall;
	bool			m_bBlockTeleport;

	/*global buffs map setting, Juver, 2018/01/23 */
	bool			m_bDisableSystemBuffs;

	/*hp potion map setting, Juver, 2018/01/23 */
	bool			m_bBlockHPPotion;

private:
	bool					m_bInstantMap;
	DWORD					m_dwInstantHostID;  // 인던주인의 GAEA ID
	bool					m_bPartyInstantMap; // 파티 인던인지 아닌지. 

protected:
	SLEVEL_HEAD				m_sLevelHead;
	SLEVEL_REQUIRE			m_sLevelRequire;
	GLMapAxisInfo			m_sLevelAxisInfo;
	SLEVEL_ETC_FUNC			m_sLevelEtcFunc;

protected:
	DxLandGateMan			m_cLandGateMan;
	GLMobScheduleMan		m_MobSchMan;

	/*dmk14 large map window ep9*/
	GLMobScheduleMan		m_MobSchManClient;

protected:
	//	Note : Single Effect
	//
	DWORD			m_dwNumLandEff;
	PLANDEFF		m_pLandEffList;
	OBJAABBNode*	m_pLandEffTree;

protected:
	DWORD			m_dwNumLandMark;
	PLANDMARK		m_pLandMarkList;
	OBJAABBNode*	m_pLandMarkTree;

protected:
	VECLANDMARK		m_vecLANDMARK_DISP;

public:
	void SetObjRotate90();

public:
	DWORD GetNumLandEff ()				{ return m_dwNumLandEff; }
	PLANDEFF GetLandEffList ()			{ return m_pLandEffList; }

	void AddLandEff ( PLANDEFF pLandEff );
	void DelLandEff ( PLANDEFF pLandEff );
	PLANDEFF FindLandEff ( char* szName );

	void BuildSingleEffTree ();

public:
	DWORD GetNumLandMark ()					{ return m_dwNumLandMark; }
	PLANDMARK GetLandMarkList ()			{ return m_pLandMarkList; }

	void AddLandMark ( PLANDMARK pLandMark );
	void DelLandMark ( PLANDMARK pLandMark );
	PLANDMARK FindLandMark ( char* szName );

	void BuildLandMarkTree ();

public:
	void SetFileName ( const char* szFile );
	const char* GetFileName () const;
	const DWORD GetFileVer () const;
	const DWORD GetInstantMapHostID () const { return m_dwInstantHostID; }

public:
	void SetMapID ( const SNATIVEID &sMapID, bool bPeaceZone, bool bPKZone=true );
	void SetInstantMap( bool bInstantMap, DWORD dwHostID, DWORD dwPartyID );
	const SNATIVEID& GetMapID () const;

	void SetWldFileName ( const char* szFile );
	const char* GetWldFileName () const;
	const EMBRIGHT GETBRIGHT () const;

	bool IsPeaceZone () const						{ return m_bPeaceZone; }
	bool IsPKZone () const							{ return m_bPKZone; }
	bool IsFreePK () const							{ return m_bFreePK; }
	bool IsItemDrop () const						{ return m_bItemDrop; }
	bool IsPetActivity () const						{ return m_bPetActivity; }
	bool IsDecreaseExpMap () const					{ return m_bDECEXP; }
	bool IsVehicleActivity () const					{ return m_bVehicleActivity; }
	bool IsClubBattleZone()	const					{ return m_bClubBattleZone; }
	bool IsInstantMap() const						{ return m_bInstantMap; }
	bool IsPartyInstantMap() const					{ return m_bPartyInstantMap; }
	bool IsQBoxEnabled() const						{ return m_bQBoxEnable;	}

	bool IsLunchBoxForbid() const					{ return m_bLunchBoxForbid;	}
	bool IsCPReset() const							{ return m_bCPReset;	} /*combatpoint logic, Juver, 2017/05/29 */
	bool IsPKMap() const							{ return m_bPKMap;	}

	bool IsUIMapSelect() const						{ return m_bUIMapSelect;	}
	bool IsUIMapInfo() const						{ return m_bUIMapInfo;	}
	bool IsClubPKRecord() const						{ return m_bClubPKRecord;	}
	bool IsOpenPrivateMarket() const				{ return m_bOpenPrivateMarket;	} /* map private market setting, Juver, 2017/10/02 */
	bool IsPartySparring() const					{ return m_bPartySparring;	}

	/*map move settings, Juver, 2017/11/25 */
	bool IsBlockTaxi() const						{ return m_bBlockTaxi; }
	bool IsBlockFriendCard() const					{ return m_bBlockFriendCard; }
	bool IsBlockRecall() const						{ return m_bBlockRecall; }
	bool IsBlockTeleport()	const					{ return m_bBlockTeleport; }

	/*global buffs map setting, Juver, 2018/01/23 */
	bool IsDisableSystemBuffs() const				{ return m_bDisableSystemBuffs; }

	/*hp potion map setting, Juver, 2018/01/23 */
	bool IsBlockHPPotion() const					{ return m_bBlockHPPotion; }

	void SetInstantMap( const bool bInstantMap )	{ m_bInstantMap = bInstantMap; }

	void SetPKZone ( bool bPK )						{ m_bPKZone = true; }
	void SetClubBattleZone ( bool bClubBattleZone )	{ m_bClubBattleZone = bClubBattleZone; }

public:
	EMDIVISION GetLandTreeDiv () const;
	void SetLandTreeDiv ( EMDIVISION eDivision );

public:
	SLEVEL_ETC_FUNC* GetLevelEtcFunc ()				{ return &m_sLevelEtcFunc; }
	SLEVEL_REQUIRE* GetLevelRequire ()				{ return &m_sLevelRequire; }
	GLMobScheduleMan* GetMobSchMan ()				{ return &m_MobSchMan; }
	GLMapAxisInfo& GetMapAxisInfo ()				{ return m_sLevelAxisInfo; }
	DxLandGateMan& GetLandGateMan ()				{ return m_cLandGateMan; }
	
	/*dmk14 large map window ep9*/
	GLMobScheduleMan* GetMobSchManClient()			{ return &m_MobSchManClient; }

public:
	BOOL LOAD_000 ( basestream &SFile, BOOL bCLIENT );
	BOOL LOAD_101 ( basestream &SFile, BOOL bCLIENT );
	BOOL LOAD_102 ( basestream &SFile, BOOL bCLIENT, LPDIRECT3DDEVICEQ pd3dDevice );
	BOOL LOAD_103 ( basestream &SFile, BOOL bCLIENT, LPDIRECT3DDEVICEQ pd3dDevice );
	BOOL LOAD_104 ( basestream &SFile, BOOL bCLIENT, LPDIRECT3DDEVICEQ pd3dDevice );
	BOOL LOAD_105 ( basestream &SFile, BOOL bCLIENT, LPDIRECT3DDEVICEQ pd3dDevice );
	BOOL LOAD_106 ( basestream &SFile, BOOL bCLIENT, LPDIRECT3DDEVICEQ pd3dDevice );
	BOOL LOAD_107 ( basestream &SFile, BOOL bCLIENT, LPDIRECT3DDEVICEQ pd3dDevice );
	BOOL LOAD_200 ( basestream &SFile, BOOL bCLIENT, LPDIRECT3DDEVICEQ pd3dDevice );

public:
	BOOL SaveFile ( const char *szFile );
	BOOL LoadFile ( const char *szFile, BOOL bCLIENT, LPDIRECT3DDEVICEQ pd3dDevice );

	BOOL SaveCsvFile ( std::fstream &SFile, bool bAllSave );
	BOOL LoadCsvFile ( const char *szFile );
	BOOL LoadAllCsvFile( LPDIRECT3DDEVICEQ pd3dDevice );

public:
	HRESULT FrameMove ( float fTime, float fElapsedTime );
	HRESULT Render ( LPDIRECT3DDEVICEQ pd3dDevice, CLIPVOLUME &cv );
	HRESULT Render_EFF ( LPDIRECT3DDEVICEQ pd3dDevice, CLIPVOLUME &cv );

public:
	HRESULT InitDeviceObjects(LPDIRECT3DDEVICEQ pd3dDevice);
	HRESULT RestoreDeviceObjects(LPDIRECT3DDEVICEQ pd3dDevice);

	HRESULT InvalidateDeviceObjects();
	HRESULT DeleteDeviceObjects();	

public:
	GLLevelFile(void);
	~GLLevelFile(void);

public:
	GLLevelFile& operator= ( GLLevelFile &rvalue );
};


#endif // GLLEVEL_FILE_H