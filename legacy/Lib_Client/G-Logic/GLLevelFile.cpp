#include "stdafx.h"
#include "./GLLevelFile.h"
#include "./GLLandMark.h"

#include "../../Lib_Engine/DxOctree/DxLandEff.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const char*		GLLevelFile::FILE_EXT		= "glmap";
char			GLLevelFile::szPATH[MAX_PATH] = "";
char			GLLevelFile::szLEVELPATH[MAX_PATH] = "";

GLLevelFile::GLLevelFile(void):
	m_sMapID(0,0),
	m_dwFileVer(0),
	m_bPeaceZone(false),
	m_bPKZone(false),
	m_bFreePK(false),
	m_bItemDrop(false),
	m_bPetActivity(false),
	m_bDECEXP(false),
	m_bVehicleActivity(false),
	m_bClubBattleZone(false),
	m_bQBoxEnable(false),
	m_bLunchBoxForbid(false),
	m_bCPReset(false), /*combatpoint logic, Juver, 2017/05/29 */
	m_bPKMap(false),
	m_bUIMapSelect(false),
	m_bUIMapInfo(false),
	m_bClubPKRecord(false),
	m_bOpenPrivateMarket(false),
	m_bPartySparring(false),

	/*map move settings, Juver, 2017/11/24 */
	m_bBlockTaxi(false),
	m_bBlockFriendCard(false),
	m_bBlockRecall(false),
	m_bBlockTeleport(false),

	/*global buffs map setting, Juver, 2018/01/23 */
	m_bDisableSystemBuffs(false),

	/*hp potion map setting, Juver, 2018/01/23 */
	m_bBlockHPPotion(false),

	m_dwNumLandEff(0),
	m_pLandEffList(NULL),
	m_pLandEffTree(NULL),
	m_dwNumLandMark(NULL),
	m_pLandMarkList(NULL),
	m_pLandMarkTree(NULL),
	m_bInstantMap(FALSE),
	m_bPartyInstantMap(FALSE),
	m_dwInstantHostID(GAEAID_NULL)
{
}

GLLevelFile::~GLLevelFile(void)
{
}

GLLevelFile& GLLevelFile::operator = ( GLLevelFile &rvalue )
{
	m_strFileName		= rvalue.m_strFileName;

	m_dwFileVer			= rvalue.m_dwFileVer;
//	m_sMapID			= rvalue.m_sMapID;
//	m_bPeaceZone		= rvalue.m_bPeaceZone;
//	m_bPKZone			= rvalue.m_bPKZone;
	m_bFreePK			= rvalue.m_bFreePK;   
	m_bItemDrop			= rvalue.m_bItemDrop; 
	m_bPetActivity		= rvalue.m_bPetActivity;
	m_bDECEXP			= rvalue.m_bDECEXP;
	m_bVehicleActivity	= rvalue.m_bVehicleActivity;
	m_bClubBattleZone	= rvalue.m_bClubBattleZone;
	m_bQBoxEnable		= rvalue.m_bQBoxEnable;

	m_bLunchBoxForbid	= rvalue.m_bLunchBoxForbid;
	m_bCPReset			= rvalue.m_bCPReset; /*combatpoint logic, Juver, 2017/05/29 */
	m_bPKMap			= rvalue.m_bPKMap;

	m_bUIMapSelect			= rvalue.m_bUIMapSelect;
	m_bUIMapInfo			= rvalue.m_bUIMapInfo;
	m_bClubPKRecord			= rvalue.m_bClubPKRecord;
	m_bOpenPrivateMarket	= rvalue.m_bOpenPrivateMarket;
	m_bPartySparring		= rvalue.m_bPartySparring;

	/*map move settings, Juver, 2017/11/24 */
	m_bBlockTaxi			= rvalue.m_bBlockTaxi;
	m_bBlockFriendCard		= rvalue.m_bBlockFriendCard;
	m_bBlockRecall			= rvalue.m_bBlockRecall;
	m_bBlockTeleport		= rvalue.m_bBlockTeleport;

	/*global buffs map setting, Juver, 2018/01/23 */
	m_bDisableSystemBuffs	= rvalue.m_bDisableSystemBuffs;

	/*hp potion map setting, Juver, 2018/01/23 */
	m_bBlockHPPotion		= rvalue.m_bBlockHPPotion;

	m_sLevelHead		= rvalue.m_sLevelHead;
	m_sLevelRequire		= rvalue.m_sLevelRequire;
	m_sLevelAxisInfo	= rvalue.m_sLevelAxisInfo;
	m_sLevelEtcFunc		= rvalue.m_sLevelEtcFunc;

	m_cLandGateMan		= rvalue.m_cLandGateMan;
	m_MobSchMan			= rvalue.m_MobSchMan;
	/*dmk14 large map window ep9*/
	m_MobSchManClient	= rvalue.m_MobSchManClient;

	m_dwNumLandEff		= rvalue.m_dwNumLandEff;
	m_pLandEffList		= rvalue.m_pLandEffList;
	m_pLandEffTree		= rvalue.m_pLandEffTree;

	m_vecLANDMARK_DISP	= rvalue.m_vecLANDMARK_DISP;

	m_sLevelEtcFunc		= rvalue.m_sLevelEtcFunc;

//	m_bInstantMap		= rvalue.m_bInstantMap;

	return *this;
}

void GLLevelFile::SetFileName ( const char* szFile )
{
	m_strFileName = szFile;
}
const DWORD GLLevelFile::GetFileVer () const
{
	return m_dwFileVer;
}

const char* GLLevelFile::GetFileName () const
{
	return m_strFileName.c_str();
}

void GLLevelFile::SetMapID(const SNATIVEID &sMapID, bool bPeaceZone, bool bPKZone/* =true */ )
{
	m_sMapID = sMapID;
	m_bPeaceZone = bPeaceZone;
	m_bPKZone = bPKZone;
}

void GLLevelFile::SetInstantMap( bool bInstantMap, DWORD dwHostID, DWORD dwPartyID /*= PARTY_NULL*/ )
{
	m_bInstantMap	   = bInstantMap;
	if( dwPartyID == PARTY_NULL )
	{
		m_dwInstantHostID  = dwHostID;
		m_bPartyInstantMap = FALSE;
	}else{
		m_dwInstantHostID  = dwPartyID;
		m_bPartyInstantMap = TRUE;
	}
}

const SNATIVEID& GLLevelFile::GetMapID () const
{
	return m_sMapID;
}

void GLLevelFile::SetWldFileName ( const char* szFile )
{
	m_sLevelHead.m_strWldFile = szFile;
}

const char* GLLevelFile::GetWldFileName () const
{
	return m_sLevelHead.m_strWldFile.c_str();
}

const EMBRIGHT GLLevelFile::GETBRIGHT () const
{
	return m_sLevelHead.m_emBright;
}

void GLLevelFile::SetLandTreeDiv ( EMDIVISION eDivision )
{
	m_sLevelHead.m_eDivision = eDivision;
}

EMDIVISION GLLevelFile::GetLandTreeDiv () const
{ 
	return m_sLevelHead.m_eDivision;
}

void GLLevelFile::BuildSingleEffTree ()
{
	PLANDEFF pCur = m_pLandEffList;
	while( pCur )
	{
		pCur->ReSetAABBBox();
		pCur = pCur->m_pNext;
	}

	COLLISION::MakeAABBTree ( m_pLandEffTree, m_pLandEffList );
}

PLANDEFF GLLevelFile::FindLandEff ( char* szName )
{
	PLANDEFF pCur = m_pLandEffList;
	while ( pCur )
	{
		if ( !strcmp(pCur->m_szName,szName) )	return pCur;

		pCur = pCur->m_pNext;
	};

	return NULL;
}

void GLLevelFile::AddLandEff ( PLANDEFF pLandEff )
{
	m_dwNumLandEff++;
	pLandEff->m_pNext = m_pLandEffList;
	m_pLandEffList = pLandEff;
}

void GLLevelFile::DelLandEff ( PLANDEFF pLandEff )
{
	GASSERT(pLandEff);
	PLANDEFF pCur, pBack;

	if ( m_pLandEffList==pLandEff )
	{
		pCur = m_pLandEffList;
		m_pLandEffList = m_pLandEffList->m_pNext;

		pCur->m_pNext = NULL;
		delete pCur;

		m_dwNumLandEff--;
		return;
	}

	pCur = m_pLandEffList;
	while ( pCur )
	{
		if ( pCur==pLandEff )
		{
			pBack->m_pNext = pCur->m_pNext;

			pCur->m_pNext = NULL;
			delete pCur;

			m_dwNumLandEff--;
			return;
		}

		pBack = pCur;
		pCur = pCur->m_pNext;
	}
}

void GLLevelFile::AddLandMark ( PLANDMARK pLandMark )
{
	m_dwNumLandMark++;
	pLandMark->m_pNext = m_pLandMarkList;
	m_pLandMarkList = pLandMark;
}

void GLLevelFile::DelLandMark ( PLANDMARK pLandMark )
{
	GASSERT(pLandMark);
	PLANDMARK pCur, pBack;

	if ( m_pLandMarkList==pLandMark )
	{
		pCur = m_pLandMarkList;
		m_pLandMarkList = m_pLandMarkList->m_pNext;

		pCur->m_pNext = NULL;
		delete pCur;

		m_dwNumLandMark--;
		return;
	}

	pCur = m_pLandMarkList;
	while ( pCur )
	{
		if ( pCur==pLandMark )
		{
			pBack->m_pNext = pCur->m_pNext;

			pCur->m_pNext = NULL;
			delete pCur;

			m_dwNumLandMark--;
			return;
		}

		pBack = pCur;
		pCur = pCur->m_pNext;
	}
}

PLANDMARK GLLevelFile::FindLandMark ( char* szName )
{
	PLANDMARK pCur = m_pLandMarkList;
	while ( pCur )
	{
		if ( pCur->m_strMARK==szName )	return pCur;

		pCur = pCur->m_pNext;
	};

	return NULL;
}

void GLLevelFile::BuildLandMarkTree ()
{
	COLLISION::MakeAABBTree ( m_pLandMarkTree, m_pLandMarkList );
}

void GLLevelFile::SetObjRotate90()
{
	D3DXMATRIX matRotate;
	D3DXMatrixRotationY( &matRotate, D3DX_PI*0.5f );

	// Note : 이펙트
	PLANDEFF pCur = m_pLandEffList;
	while ( pCur )
	{
		D3DXMatrixMultiply( &pCur->m_matWorld, &pCur->m_matWorld, &matRotate );
		pCur = pCur->m_pNext;
	}
	BuildSingleEffTree();

	// Note : Mark
	PLANDMARK pMark = m_pLandMarkList;
	while( pMark )
	{
		//D3DXMatrixMultiply( &pMark->, &pMark->m_matWorld, &matRotate );	// 위치
		pMark = pMark->m_pNext;
	}
	BuildLandMarkTree();

	// Note : 게이트
	m_cLandGateMan.SetObjRotate90();

	// Note : 몹
	m_MobSchMan.SetObjRotate90();
	/*dmk14 large map window ep9*/
	m_MobSchManClient.SetObjRotate90();
}
/*
	D3DXMATRIX matRotate;
	D3DXMatrixRotationY( &matRotate, D3DX_PI*0.5f );

	// Note : 이펙트
	PLANDEFF pCur = m_pLandEffList;
	while ( pCur )
	{
		D3DXMatrixMultiply( &pCur->m_matWorld, &pCur->m_matWorld, &matRotate );
		pCur = pCur->m_pNext;
	}
	BuildSingleEffTree();

	// Note : 스킨 오브젝브
	PLANDSKINOBJ pSkin = m_pLandSkinObjList;
	while( pSkin )
	{
		D3DXMatrixMultiply( &pSkin->m_matWorld, &pSkin->m_matWorld, &matRotate );
		pSkin = pSkin->m_pNext;
	}
	BuildSkinObjTree();

	// Note : 라이트
	DXLIGHT* pLight = DxLightMan::GetInstance()->GetLightHead();
	while( pLight )
	{
		D3DXMatrixMultiply( &pLight->m_matWorld, &pLight->m_matWorld, &matRotate );
		pLight = pLight->pNext;
	}
	DxLightMan::GetInstance()->MakeAABBTree();

	// Note : 사운드
	m_StaticSoundMan.SetObjRotate90();

	// Note : 게이트의 변환
	m_LandGateMan.SetObjRotate90();
*/

HRESULT GLLevelFile::InitDeviceObjects(LPDIRECT3DDEVICEQ pd3dDevice)
{
	PLANDEFF pCur = m_pLandEffList;
	while ( pCur )
	{
		pCur->InitDeviceObjects ( pd3dDevice );

		pCur = pCur->m_pNext;
	}

	return S_OK;
}

HRESULT GLLevelFile::RestoreDeviceObjects(LPDIRECT3DDEVICEQ pd3dDevice)
{
	PLANDEFF pCur = m_pLandEffList;
	while ( pCur )
	{
		pCur->RestoreDeviceObjects ( pd3dDevice );

		pCur = pCur->m_pNext;
	}

	return S_OK;
}

HRESULT GLLevelFile::InvalidateDeviceObjects()
{
	PLANDEFF pCur = m_pLandEffList;
	while ( pCur )
	{
		pCur->InvalidateDeviceObjects ();

		pCur = pCur->m_pNext;
	}

	return S_OK;
}

HRESULT GLLevelFile::DeleteDeviceObjects()
{
	PLANDEFF pCur = m_pLandEffList;
	while ( pCur )
	{
		pCur->DeleteDeviceObjects ();
	
		pCur = pCur->m_pNext;
	}

	m_dwNumLandEff = 0;
	SAFE_DELETE(m_pLandEffList);
	SAFE_DELETE(m_pLandEffTree);

	m_dwNumLandMark = 0;
	SAFE_DELETE(m_pLandMarkList);
	SAFE_DELETE(m_pLandMarkTree);

	return S_OK;
}

HRESULT GLLevelFile::FrameMove ( float fTime, float fElapsedTime )
{
	DXLANDEFF::FrameMove ( fTime, fElapsedTime );

	return S_OK;
}

HRESULT GLLevelFile::Render ( LPDIRECT3DDEVICEQ pd3dDevice, CLIPVOLUME &cv )
{
	HRESULT hr = S_OK;

	COLLISION::RenderAABBTree ( pd3dDevice, &cv, m_pLandMarkTree, FALSE );

	return S_OK;
}

HRESULT GLLevelFile::Render_EFF ( LPDIRECT3DDEVICEQ pd3dDevice, CLIPVOLUME &cv )
{
	//	Note : 단독 이펙트 ( single effect ) 랜더링.
	//
	COLLISION::dwRendAABB = 0;
	COLLISION::RenderAABBTree ( pd3dDevice, &cv, m_pLandEffTree, FALSE );

	return S_OK;
}
