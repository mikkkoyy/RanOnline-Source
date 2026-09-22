#include "stdafx.h"
#include "./DxEffectParticleSys.h"
#include "./DxEffectGround.h"
#include "./DxEffectMesh.h"
#include "./DxEffectSequence.h"
#include "./DxEffectDecal.h"
#include "./DxEffectCamera.h"
#include "./DxEffectBlurSys.h"
#include "./DxEffectLighting.h"
#include "./DxEffectMoveRotate.h"
#include "./DxEffectSkinMesh.h"
#include "./DxEffectWave.h"
#include "./DxEffectPointLight.h"
#include "./DxEffectMoveTarget.h"
#include "./DxEffSinglePropGMan.h"

#include "../Common/SerialFile.h"
#include "../DxCommon/DxMethods.h"
#include "../DxSound/DxSoundMan.h"
#include "../G-Logic/GLogic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//-------------------------------------------------------------------------------------[EFF_PROPERTY]

EFF_PROPERTY* EFF_PROPERTY::NEW_PROP ( DWORD TypeID )
{
	EFF_PROPERTY* pNew = NULL;

	if ( TypeID == PARTICLESYS_PROPERTY::TYPEID		)	pNew = new PARTICLESYS_PROPERTY;
	else if ( TypeID == GROUND_PROPERTY::TYPEID		)	pNew = new GROUND_PROPERTY;
	else if ( TypeID == MESH_PROPERTY::TYPEID		)   pNew = new MESH_PROPERTY;
	else if ( TypeID == SEQUENCE_PROPERTY::TYPEID	)   pNew = new SEQUENCE_PROPERTY;
	else if ( TypeID == DECAL_PROPERTY::TYPEID		)   pNew = new DECAL_PROPERTY;
	else if ( TypeID == CAMERA_PROPERTY::TYPEID		)   pNew = new CAMERA_PROPERTY;
	else if ( TypeID == BLURSYS_PROPERTY::TYPEID	)   pNew = new BLURSYS_PROPERTY;
	else if ( TypeID == LIGHTNING_PROPERTY::TYPEID	)   pNew = new LIGHTNING_PROPERTY;
	else if ( TypeID == MOVEROTATE_PROPERTY::TYPEID	)   pNew = new MOVEROTATE_PROPERTY;
	else if ( TypeID == SKINMESH_PROPERTY::TYPEID	)   pNew = new SKINMESH_PROPERTY;
	else if ( TypeID == WAVE_PROPERTY::TYPEID		)	pNew = new WAVE_PROPERTY;
	else if ( TypeID == POINTLIGHT_PROPERTY::TYPEID	)	pNew = new POINTLIGHT_PROPERTY;
	else if ( TypeID == MOVETARGET_PROPERTY::TYPEID	)	pNew = new MOVETARGET_PROPERTY;

	return pNew;
}

void	EFF_PROPERTY::CloneEffProp ( LPDIRECT3DDEVICEQ pd3dDevice, EFF_PROPERTY* pSrcProp )
{
	this->m_fGBeginTime		= pSrcProp->m_fGBeginTime;
	this->m_fGLifeTime		= pSrcProp->m_fGLifeTime;
	this->m_MovSound		= pSrcProp->m_MovSound;
	this->m_matLocal		= pSrcProp->m_matLocal;

	DWORD dwType = pSrcProp->GetTypeID ();
	switch ( dwType )
	{
	case PARTICLESYS_PROPERTY::TYPEID:
		((PARTICLESYS_PROPERTY*)this)->m_Property = ((PARTICLESYS_PROPERTY*) pSrcProp)->m_Property;
		break;
	case GROUND_PROPERTY::TYPEID:
		((GROUND_PROPERTY*) this)->m_Property = ((GROUND_PROPERTY*) pSrcProp)->m_Property;
		break;
	case MESH_PROPERTY::TYPEID:
		((MESH_PROPERTY*) this)->m_Property = ((MESH_PROPERTY*) pSrcProp)->m_Property;
		break;
	case SEQUENCE_PROPERTY::TYPEID:
		((SEQUENCE_PROPERTY*) this)->m_Property = ((SEQUENCE_PROPERTY*) pSrcProp)->m_Property;
		break;
	case DECAL_PROPERTY::TYPEID:
		((DECAL_PROPERTY*) this)->m_Property = ((DECAL_PROPERTY*) pSrcProp)->m_Property;
		break;
	case CAMERA_PROPERTY::TYPEID:
		((CAMERA_PROPERTY*) this)->m_Property = ((CAMERA_PROPERTY*) pSrcProp)->m_Property;
		break;
	case BLURSYS_PROPERTY::TYPEID:
		((BLURSYS_PROPERTY*) this)->m_Property = ((BLURSYS_PROPERTY*) pSrcProp)->m_Property;
		break;
	case LIGHTNING_PROPERTY::TYPEID:
		((LIGHTNING_PROPERTY*) this)->m_Property = ((LIGHTNING_PROPERTY*) pSrcProp)->m_Property;
		break;
	case MOVEROTATE_PROPERTY::TYPEID:
		((MOVEROTATE_PROPERTY*) this)->m_Property = ((MOVEROTATE_PROPERTY*) pSrcProp)->m_Property;
		break;
	case SKINMESH_PROPERTY::TYPEID:
		((SKINMESH_PROPERTY*) this)->m_Property = ((SKINMESH_PROPERTY*) pSrcProp)->m_Property;
		break;
	case WAVE_PROPERTY::TYPEID:
		((WAVE_PROPERTY*) this)->m_Property = ((WAVE_PROPERTY*) pSrcProp)->m_Property;
		break;
	case POINTLIGHT_PROPERTY::TYPEID:
		((POINTLIGHT_PROPERTY*) this)->m_Property = ((POINTLIGHT_PROPERTY*) pSrcProp)->m_Property;
		break;
	case MOVETARGET_PROPERTY::TYPEID:
		((MOVETARGET_PROPERTY*) this)->m_Property = ((MOVETARGET_PROPERTY*) pSrcProp)->m_Property;
		break;
	};

	this->DeleteDeviceObjects ();
	this->InitDeviceObjects ( pd3dDevice );
}

HRESULT EFF_PROPERTY::InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( m_pChild )		m_pChild->InitDeviceObjects ( pd3dDevice );
	if ( m_pSibling )	m_pSibling->InitDeviceObjects ( pd3dDevice );

	return S_OK;
}

HRESULT EFF_PROPERTY::RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( m_pChild )		m_pChild->RestoreDeviceObjects ( pd3dDevice );
	if ( m_pSibling )	m_pSibling->RestoreDeviceObjects ( pd3dDevice );

	return S_OK;
}

HRESULT EFF_PROPERTY::InvalidateDeviceObjects ()
{
	if ( m_pChild )		m_pChild->InvalidateDeviceObjects ();
	if ( m_pSibling )	m_pSibling->InvalidateDeviceObjects ();

	return S_OK;
}

HRESULT EFF_PROPERTY::DeleteDeviceObjects ()
{
	if ( m_pChild )		m_pChild->DeleteDeviceObjects ();
	if ( m_pSibling )	m_pSibling->DeleteDeviceObjects ();

	return S_OK;
}
	
DWORD EFF_PROPERTY::GetSizeBase ()
{
	return ( sizeof(float)+sizeof(float)+sizeof(BOOL)+sizeof(D3DXMATRIX) );
}

//-----------------------------------------------------------------------------------------[EFF_PROPGROUP]

BOOL EFF_PROPGROUP::RESET_MOVEOBJ_PROP ( PEFF_PROPERTY &pPropHead, EFF_PROPERTY &pCurEffProp )
{
	PEFF_PROPERTY pPropCur = pPropHead;
	while ( pPropCur )
	{
		pPropCur->m_bMoveObj = FALSE;

		//	Note : 하위 자식 노드 탐색.
		//
		if ( pPropCur->m_pChild )
		{
			RESET_MOVEOBJ_PROP ( pPropCur->m_pChild, pCurEffProp );
		}

		pPropCur = pPropCur->m_pSibling;
	}

	pCurEffProp.m_bMoveObj = TRUE;

	return TRUE;
}

BOOL EFF_PROPGROUP::NEWEFFSINGLE ( PEFF_PROPERTY &pCurEffProp, PDXEFFSINGLE &pCurEffSingle, DxEffSingleGroup* pThisGroup )
{
	BOOL bValid;
	GASSERT(pCurEffProp);

	//	Note : 현제 노드 생성.
	//
	pCurEffSingle = pCurEffProp->NEWOBJ();
	if ( !pCurEffSingle )	return FALSE;

	//	Note : 생성될 그룹의 포인터 지정.
	//
	pCurEffSingle->m_pThisGroup = pThisGroup;

	//	Note : 형제, 자식 노드 순회.
	//
	if ( pCurEffProp->m_pSibling )
	{
		bValid = NEWEFFSINGLE ( pCurEffProp->m_pSibling, pCurEffSingle->m_pSibling, pThisGroup );
		if ( !bValid )	return bValid;
	}

	if ( pCurEffProp->m_pChild )
	{
		bValid = NEWEFFSINGLE ( pCurEffProp->m_pChild, pCurEffSingle->m_pChild, pThisGroup );
		if ( !bValid )	return bValid;
	}

	return TRUE;
}

BOOL EFF_PROPGROUP::NEWEFFSINGLE ( PEFF_PROPERTY &pCurEffProp, PDXEFFSINGLE &pCurEffSingle, DxEffSingleGroup* pThisGroup, EFF_PROPERTY* pTargetProp )
{
	BOOL bValid;
	GASSERT(pCurEffProp);

	//	Note : 현제 노드 생성.
	//
	pCurEffSingle = pCurEffProp->NEWOBJ();
	if ( !pCurEffSingle )	return FALSE;

	//	Note : 그려지지 않는 이펙트 표시.
	//
	if ( pTargetProp != pCurEffProp )
		pCurEffSingle->m_dwRunFlag |= EFF_PLY_RENDPASS;

	//	Note : 생성될 그룹의 포인터 지정.
	//
	pCurEffSingle->m_pThisGroup = pThisGroup;

	//	Note : 형제, 자식 노드 순회.
	//
	if ( pCurEffProp->m_pSibling )
	{
		bValid = NEWEFFSINGLE ( pCurEffProp->m_pSibling, pCurEffSingle->m_pSibling, pThisGroup, pTargetProp );
		if ( !bValid )	return bValid;
	}

	if ( pCurEffProp->m_pChild )
	{
		bValid = NEWEFFSINGLE ( pCurEffProp->m_pChild, pCurEffSingle->m_pChild, pThisGroup, pTargetProp );
		if ( !bValid )	return bValid;
	}

	return TRUE;
}

DxEffSingleGroup* EFF_PROPGROUP::NEWEFFGROUP ()
{
	if ( !m_pPropRoot )	return NULL;

	DxEffSingleGroup* pNewEffGroup = new DxEffSingleGroup;
	pNewEffGroup->m_strFileName = m_strFileName;

	//	Note : Group 설정.
	//
	pNewEffGroup->m_dwFlag = m_dwFlag;
	pNewEffGroup->m_pMax = &m_vMax;
	pNewEffGroup->m_pMin = &m_vMin;
	pNewEffGroup->m_vLocal = m_vLocal;

	//	Note : Single 개체 생성.
	//
	BOOL bValid = NEWEFFSINGLE ( m_pPropRoot, pNewEffGroup->m_pEffSingleRoot, pNewEffGroup );

	if ( !bValid )
	{
		SAFE_DELETE(pNewEffGroup);
		return NULL;
	}

	return pNewEffGroup;
}

DxEffSingleGroup* EFF_PROPGROUP::NEWEFFGROUP ( EFF_PROPERTY* pTargetProp )
{
	if ( !m_pPropRoot )	return NULL;

	DxEffSingleGroup* pNewEffGroup = new DxEffSingleGroup;
	pNewEffGroup->m_strFileName = m_strFileName;

	//	Note : 설정.
	pNewEffGroup->m_pMax = &m_vMax;
	pNewEffGroup->m_pMin = &m_vMin;

	BOOL bValid = NEWEFFSINGLE ( m_pPropRoot, pNewEffGroup->m_pEffSingleRoot, pNewEffGroup, pTargetProp );

	if ( !bValid )
	{
		SAFE_DELETE(pNewEffGroup);
		return NULL;
	}

	return pNewEffGroup;
}

DxEffSingleGroup* EFF_PROPGROUP::NEWEFFSINGLE ( EFF_PROPERTY* pTargetProp )
{
	if ( !pTargetProp ) return NULL;

	DxEffSingleGroup* pNewEffGroup = new DxEffSingleGroup;
	pNewEffGroup->m_strFileName = "_singletest_";

	//	Note : 설정.
	pNewEffGroup->m_pMax = &m_vMax;
	pNewEffGroup->m_pMin = &m_vMin;

	//	Note : 현제 노드 생성.
	pNewEffGroup->m_pEffSingleRoot = pTargetProp->NEWOBJ();
	if ( !pNewEffGroup->m_pEffSingleRoot )
	{
		SAFE_DELETE(pNewEffGroup);
		return NULL;
	}

	//	Note : 생성될 그룹의 포인터 지정.
	//
	pNewEffGroup->m_pEffSingleRoot->m_pThisGroup = pNewEffGroup;

	return pNewEffGroup;
}

HRESULT EFF_PROPGROUP::DeleteProp ( PEFF_PROPERTY &pPropHead, PEFF_PROPERTY pProp )
{
	HRESULT hr = S_OK;

	PEFF_PROPERTY pPropDel = NULL;
	if ( pPropHead == pProp )
	{
		pPropDel = pPropHead;
		pPropHead = pPropHead->m_pSibling;

		pPropDel->m_pSibling = NULL;
		SAFE_DELETE(pPropDel);

		return S_OK;
	}

	PEFF_PROPERTY pPropCur = pPropHead, pPropPrev = pPropHead;
	while ( pPropCur )
	{
		if ( pPropCur == pProp )
		{
			pPropPrev->m_pSibling = pPropCur->m_pSibling;

			pPropCur->m_pSibling = NULL;
			SAFE_DELETE(pPropCur);
			return S_OK;
		}

		//	Note : 하위 자식 노드 탐색.
		//
		if ( pPropCur->m_pChild )
		{
			hr = DeleteProp ( pPropCur->m_pChild, pProp );
			if ( hr == S_OK )	return hr;
		}

		pPropPrev = pPropCur;
		pPropCur = pPropCur->m_pSibling;
	}

	return S_FALSE;
}

HRESULT EFF_PROPGROUP::GetMatCombParent ( PEFF_PROPERTY &pPropHead, PEFF_PROPERTY pProp, D3DXMATRIX &matParent, D3DXMATRIX &matComb )
{
	HRESULT hr = S_OK;

	PEFF_PROPERTY pPropCur = pPropHead;
	while ( pPropCur )
	{
		if ( pPropCur == pProp )
		{
			matComb = matParent;
			return S_OK;
		}

		//	Note : 하위 자식 노드 탐색.
		//
		if ( pPropCur->m_pChild )
		{
			//	Note : 메트릭스를 계산. matCombined = matRot * matParent
			D3DXMATRIX matThisComb;
			D3DXMatrixMultiply ( &matThisComb, &pPropCur->m_matLocal, &matParent );
			hr = GetMatCombParent ( pPropCur->m_pChild, pProp, matThisComb, matComb );
			if ( hr == S_OK )	return hr;
		}

		pPropCur = pPropCur->m_pSibling;
	}

	return S_FALSE;
}

HRESULT EFF_PROPGROUP::InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( m_pPropRoot )	m_pPropRoot->InitDeviceObjects ( pd3dDevice );

	return S_OK;
}

HRESULT EFF_PROPGROUP::RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( m_pPropRoot )	m_pPropRoot->RestoreDeviceObjects ( pd3dDevice );

	return S_OK;
}

HRESULT EFF_PROPGROUP::InvalidateDeviceObjects ()
{
	if ( m_pPropRoot )	m_pPropRoot->InvalidateDeviceObjects ();

	return S_OK;
}

HRESULT EFF_PROPGROUP::DeleteDeviceObjects ()
{
	if ( m_pPropRoot )	m_pPropRoot->DeleteDeviceObjects ();

	SAFE_DELETE(m_pPropRoot);

	return S_OK;
}

//-------------------------------------------------------------------------------------[DxEffSinglePropGMan]

DxEffSinglePropGMan& DxEffSinglePropGMan::GetInstance()
{
	static DxEffSinglePropGMan Instance;
	return Instance;
}

DxEffSinglePropGMan::DxEffSinglePropGMan(void) :
	m_pd3dDevice(NULL)
{
	memset( m_szPath, 0, sizeof(char)*MAX_PATH );
}

DxEffSinglePropGMan::~DxEffSinglePropGMan(void)
{
}

EFF_PROPGROUP* DxEffSinglePropGMan::FindEffGProp ( const char* szFileName )
{
	GASSERT(szFileName);

	std::string strFindFile = szFileName;
	std::transform ( strFindFile.begin(), strFindFile.end(), strFindFile.begin(), tolower );

	MAPPROPGROUP_ITER pos = m_mapPROPGROUP.find ( strFindFile );
	if ( pos == m_mapPROPGROUP.end() )	return NULL;

	return (*pos).second;
}

HRESULT DxEffSinglePropGMan::DeleteEffectGProp ( const char* szFileName )
{
	GASSERT(szFileName);

	std::string strFindFile = szFileName;
	std::transform ( strFindFile.begin(), strFindFile.end(), strFindFile.begin(), tolower );

	MAPPROPGROUP_ITER pos = m_mapPROPGROUP.find ( strFindFile );
	if ( pos == m_mapPROPGROUP.end() )	return NULL;

	SAFE_DELETE((*pos).second);
	m_mapPROPGROUP.erase ( pos );

	return E_FAIL;
}

EFF_PROPGROUP* DxEffSinglePropGMan::LoadEffectGProp ( const char* szFileName )
{
	HRESULT hr;

	GASSERT(szFileName);
	if( !m_pd3dDevice )
	{
		CDebugSet::ToLogFile( _T("LoadEffectGProp - m_pd3dDevice == NULL") );
		return NULL;
	}

	std::string strFindFile = szFileName;
	std::transform ( strFindFile.begin(), strFindFile.end(), strFindFile.begin(), tolower );

	EFF_PROPGROUP* pPropGroup;

	pPropGroup = FindEffGProp ( strFindFile.c_str() );
	if ( pPropGroup )	return pPropGroup;


	pPropGroup = new EFF_PROPGROUP;
	hr = pPropGroup->LoadFile ( strFindFile.c_str(), m_pd3dDevice );
	if ( FAILED(hr) )
	{
		SAFE_DELETE(pPropGroup);
		return NULL;
	}

	//	Note : 로드한 이팩트 Prop Group을 등록.
	//
	m_mapPROPGROUP.insert ( std::make_pair(strFindFile,pPropGroup) );

	return pPropGroup;
}

HRESULT DxEffSinglePropGMan::OneTimeSceneInit ( const char *szPath )
{
	SetPath ( szPath );

	return S_OK;
}

HRESULT DxEffSinglePropGMan::InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	m_pd3dDevice = pd3dDevice;

	MAPPROPGROUP_ITER pos = m_mapPROPGROUP.begin();
	MAPPROPGROUP_ITER end = m_mapPROPGROUP.end();
	for ( ; pos!=end; ++pos )
	{
		(*pos).second->InitDeviceObjects ( m_pd3dDevice );
	}

	return S_OK;
}

HRESULT DxEffSinglePropGMan::RestoreDeviceObjects ()
{
	MAPPROPGROUP_ITER pos = m_mapPROPGROUP.begin();
	MAPPROPGROUP_ITER end = m_mapPROPGROUP.end();
	for ( ; pos!=end; ++pos )
	{
		(*pos).second->RestoreDeviceObjects ( m_pd3dDevice );
	}

	return S_OK;
}

HRESULT DxEffSinglePropGMan::InvalidateDeviceObjects ()
{
	MAPPROPGROUP_ITER pos = m_mapPROPGROUP.begin();
	MAPPROPGROUP_ITER end = m_mapPROPGROUP.end();
	for ( ; pos!=end; ++pos )
	{
		(*pos).second->InvalidateDeviceObjects ();
	}

	return S_OK;
}

HRESULT DxEffSinglePropGMan::DeleteDeviceObjects ()
{
	MAPPROPGROUP_ITER pos = m_mapPROPGROUP.begin();
	MAPPROPGROUP_ITER end = m_mapPROPGROUP.end();
	for ( ; pos!=end; ++pos )
	{
		(*pos).second->DeleteDeviceObjects ();
		SAFE_DELETE ( (*pos).second );
	}

	m_mapPROPGROUP.clear ();

	return S_OK;
}
