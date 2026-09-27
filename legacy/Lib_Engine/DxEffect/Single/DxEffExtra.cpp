#include "stdafx.h"

#include "../DxEffect/Single/DxEffSingle.h"
#include "./DxEffSinglePropGMan.h"
#include "../Common/StlFunctions.h"
#include "../DxEffect/DxEffectMan.h"
#include "./DxEffectParticleSys.h"
#include "./DxEffcharData.h"
#include "./DxEffExtra.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DxEffExtra& DxEffExtra::GetInstance()
{
	static DxEffExtra Instance;
	return Instance;
}

DxEffExtra::DxEffExtra (void)
	: m_fTimer(0.0f)
	, m_pd3dDevice(NULL)
	, m_pEffClick(NULL)
	, bEffClickCreated(false)
	, m_pEffTarget(NULL)
	, bEffTargetCreated(false)

	, m_pEffLocatorA(NULL)
	, m_pEffLocatorB(NULL)
	, bEffLocatorACreated(false)
	, bEffLocatorBCreated(false)
{
}

DxEffExtra::~DxEffExtra (void)
{
}

DxEffSingleGroup* DxEffExtra::NewEffGroup ( EFF_PROPGROUP* pPropGroup, const D3DXMATRIX &matTrans, const STARGETID* pTargetID )
{
	GASSERT(pPropGroup);
	DxEffSingleGroup* pEffSingleGroup = pPropGroup->NEWEFFGROUP ();
	if ( !pEffSingleGroup )	return NULL;
	pEffSingleGroup->SetTargetID ( pTargetID );
	pEffSingleGroup->m_matWorld = matTrans;
	pEffSingleGroup->Create ( m_pd3dDevice );
	STARGETID sTargetID;
	if ( pTargetID )	sTargetID = *pTargetID;
	m_mapESG.insert ( std::make_pair(sTargetID,pEffSingleGroup) );
	return pEffSingleGroup;
}

DxEffSingleGroup* DxEffExtra::NewEff( const char* szFileName, const D3DXMATRIX &matTrans, const STARGETID* pTargetID )
{
	GASSERT(m_pd3dDevice);
	EFF_PROPGROUP* pPropGroup;

	pPropGroup = DxEffSinglePropGMan::GetInstance().LoadEffectGProp ( szFileName );
	if ( !pPropGroup )	return NULL;

	DxEffSingleGroup* pEffSingleG = NewEffGroup ( pPropGroup, matTrans, pTargetID );

	return pEffSingleG;
}

HRESULT DxEffExtra::InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	GASSERT(pd3dDevice);
	m_pd3dDevice = pd3dDevice;

	EFFSGMAP_ITER iter = m_mapESG.begin();
	EFFSGMAP_ITER iter_end = m_mapESG.end();
	for ( ; iter!=iter_end; ++iter )
	{
		(*iter).second->InitDeviceObjects ( pd3dDevice );
	}
	return S_OK;
}

HRESULT DxEffExtra::RestoreDeviceObjects ()
{
	EFFSGMAP_ITER iter = m_mapESG.begin();
	EFFSGMAP_ITER iter_end = m_mapESG.end();
	for ( ; iter!=iter_end; ++iter )
	{
		(*iter).second->RestoreDeviceObjects ( m_pd3dDevice );
	}
	return S_OK;
}

HRESULT DxEffExtra::InvalidateDeviceObjects ()
{
	EFFSGMAP_ITER iter = m_mapESG.begin();
	EFFSGMAP_ITER iter_end = m_mapESG.end();
	for ( ; iter!=iter_end; ++iter )
	{
		(*iter).second->InvalidateDeviceObjects ();
	}
	return S_OK;
}

HRESULT DxEffExtra::DeleteDeviceObjects ()
{
	RemoveAllEff ();
	return S_OK;
}

HRESULT DxEffExtra::RemoveAllEff ()
{
	m_pEffClick = NULL;
	bEffClickCreated = false;

	m_pEffTarget = NULL;
	bEffTargetCreated = false;

	m_pEffLocatorA = NULL;
	m_pEffLocatorB = NULL;
	bEffLocatorACreated = false;
	bEffLocatorBCreated = false;

	std::for_each ( m_mapESG.begin(), m_mapESG.end(), std_afunc::DeleteMapObject() );
	m_mapESG.clear();
	return S_OK;
}

HRESULT DxEffExtra::DeleteEff ( const char* szFile )
{
	if ( !szFile )						return S_FALSE;
	if ( m_mapESG.empty() )				return S_FALSE;

	std::string strFileName = szFile;
	std::transform ( strFileName.begin(), strFileName.end(), strFileName.begin(), tolower );

	EFFSGMAP_ITER iter = m_mapESG.begin();
	EFFSGMAP_ITER iter_end = m_mapESG.end();
	for ( ; iter!=iter_end; ++iter )
	{
		if ( (*iter).second->m_strFileName==strFileName )
		{
			SAFE_DELETE ( (*iter).second );
			m_mapESG.erase ( iter );
			return S_OK;
		}
	}

	return S_OK;
}

HRESULT DxEffExtra::DeleteAllEff ( const char* szFile )
{
	if ( !szFile )						return S_FALSE;
	if ( m_mapESG.empty() )				return S_FALSE;

	std::string strFileName = szFile;
	std::transform ( strFileName.begin(), strFileName.end(), strFileName.begin(), tolower );

	EFFSGMAP_ITER iter = m_mapESG.begin();
	EFFSGMAP_ITER iter_end = m_mapESG.end();
	for ( ; iter!=iter_end; )
	{
		EFFSGMAP_ITER iter_cur = iter++;
		if ( (*iter_cur).second->m_strFileName==strFileName )
		{
			SAFE_DELETE ( (*iter_cur).second );
			m_mapESG.erase ( iter_cur );
		}
	}

	return S_OK;
}

void DxEffExtra::NewAfterEff ( DxEffSingleGroup* pEffSG )
{
	if ( pEffSG->m_listEffSAfter.empty() )	return;
	STARGETID sTargetID = pEffSG->m_TargetID;
	D3DXVECTOR3 vTARPOS = DxEffectMan::GetInstance().GetCrowPos ( sTargetID );
	vTARPOS.y += 15.0f;
	float fTARDIR = DxEffectMan::GetInstance().GetCrowDir ( sTargetID );
	DxEffSingleGroup::AFTERLIST_ITER iter = pEffSG->m_listEffSAfter.begin();
	DxEffSingleGroup::AFTERLIST_ITER iter_end = pEffSG->m_listEffSAfter.end();
	for ( ; iter!=iter_end; ++iter )
	{
		CString strTemp = (*iter).m_strEffect.c_str();
		if ( !((*iter).m_strEffect.empty()) )
		{
			switch ( (*iter).m_emType )
			{
			case EFFABODY:
				{
					DxSkinChar* pSkinChar = DxEffectMan::GetInstance().GetSkinChar ( sTargetID );
					if ( !pSkinChar )	continue;
					D3DXVECTOR3 vDir = vTARPOS - pEffSG->m_vStartPos;
					D3DXVec3Normalize ( &vDir, &vDir );
					DxEffcharDataMan::GetInstance().PutEffect
					(
						pSkinChar,
						(*iter).m_strEffect.c_str(),
						&vDir,
						fTARDIR
					);
				}break;

			case EFFASINGLE:
				{
					if ( !DxEffectMan::GetInstance().IsCrowVisible(sTargetID) )	break;
					sTargetID.vPos = vTARPOS;
					D3DXMATRIX matEffect;
					D3DXMatrixTranslation ( &matEffect, vTARPOS.x, vTARPOS.y, vTARPOS.z );
					NewEff
					(
						(*iter).m_strEffect.c_str(),
						matEffect,
						&sTargetID
					);
				}break;
			};
		}
	}
	pEffSG->m_listEffSAfter.clear();
}

HRESULT DxEffExtra::FrameMove ( float fTime, float fElapsedTime )
{
	m_fTimer += fElapsedTime;
	EFFSGMAP_ITER iter = m_mapESG.begin();
	EFFSGMAP_ITER iter_end = m_mapESG.end();
	for ( ; iter!=iter_end; )
	{
		(*iter).second->FrameMove ( fTime, fElapsedTime );

		if ( (*iter).second->IsBeginAfterEffect() )
		{
			EFFSGMAP_ITER iter_del = iter;
			++iter;

			NewAfterEff ( (*iter_del).second );
		}
		else if ( (*iter).second->m_dwAliveCount==0 )
		{
			EFFSGMAP_ITER iter_del = iter;
			++iter;

			SAFE_DELETE((*iter_del).second);
			m_mapESG.erase(iter_del);
		}else{
			++iter;
		}
	}
	return S_OK;
}

HRESULT	DxEffExtra::Render ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	EFFSGMAP_ITER iter = m_mapESG.begin();
	EFFSGMAP_ITER iter_end = m_mapESG.end();
	for ( ; iter!=iter_end; ++iter )
	{
		(*iter).second->Render ( pd3dDevice );
	}
	return S_OK;
}


DxEffSingleGroup* DxEffExtra::NewClickEff ( const char* szFileName, const D3DXMATRIX &matTrans, const STARGETID* pTargetID )
{
	GASSERT(m_pd3dDevice);
	EFF_PROPGROUP* pPropGroup;

	if ( bEffClickCreated )
	{
		if ( m_pEffClick )
		{
			m_pEffClick->SetTargetID( pTargetID );
			m_pEffClick->m_matWorld = matTrans;
		}
	}

	if ( !bEffClickCreated )
	{
		pPropGroup = DxEffSinglePropGMan::GetInstance().LoadEffectGProp ( szFileName );
		if ( !pPropGroup )	return NULL;
		m_pEffClick = NewEffGroup ( pPropGroup, matTrans, pTargetID );
		bEffClickCreated = true;
	}

	return m_pEffClick;
}

void DxEffExtra::RestartClickEffect()
{
	if ( bEffClickCreated && m_pEffClick )
		m_pEffClick->ReStartEff();
}

void DxEffExtra::RemoveClickEffect( const char* szFile )
{
	bEffClickCreated = FALSE;
	m_pEffClick = NULL;
	DeleteAllEff( szFile );
}

DxEffSingleGroup* DxEffExtra::NewTargetEff ( const char* szFileName, const D3DXMATRIX &matTrans, const STARGETID* pTargetID )
{
	GASSERT(m_pd3dDevice);
	EFF_PROPGROUP* pPropGroup;

	if ( bEffTargetCreated )
	{
		if ( m_pEffTarget )
		{
			m_pEffTarget->SetTargetID( pTargetID );
			m_pEffTarget->m_matWorld = matTrans;
		}
	}

	if ( !bEffTargetCreated )
	{
		pPropGroup = DxEffSinglePropGMan::GetInstance().LoadEffectGProp ( szFileName );
		if ( !pPropGroup )	return NULL;
		m_pEffTarget = NewEffGroup ( pPropGroup, matTrans, pTargetID );
		bEffTargetCreated = true;
	}

	return m_pEffTarget;
}

void DxEffExtra::RemoveTargetEffect( const char* szFile )
{
	bEffTargetCreated = FALSE;
	m_pEffTarget = NULL;
	DeleteAllEff( szFile );
}

HRESULT DxEffExtra::PassiveEffect ( const char* szFileName, const D3DXMATRIX &matTrans, const STARGETID &sTargetID )
{
	if ( sTargetID.dwID==EMTARGET_NULL )		return E_FAIL;
	if ( !szFileName || szFileName[0]==NULL )	return E_FAIL;

	std::string strFileName = szFileName;
	std::transform ( strFileName.begin(), strFileName.end(), strFileName.begin(), tolower );

	BOOL bExist = FALSE;
	EFFSGMAP_PAIR pairESG = m_mapESG.equal_range ( sTargetID );
	EFFSGMAP_ITER iter = pairESG.first;
	for ( ; iter != pairESG.second; ++iter )
	{
		DxEffSingleGroup* pEffSingleGroup = (*iter).second;
		if ( pEffSingleGroup->m_strFileName==strFileName )
		{
			bExist = TRUE;
		}
	}

	if ( !bExist )
	{
		NewEff( strFileName.c_str(), matTrans, &sTargetID );
	}
	return S_OK;
}

HRESULT DxEffExtra::DeletePassiveEffect ( const char* szFileName, const STARGETID &sTargetID )
{
	if ( sTargetID.dwID==EMTARGET_NULL )	return E_FAIL;

	std::string strFileName = szFileName;
	std::transform ( strFileName.begin(), strFileName.end(), strFileName.begin(), tolower );

	EFFSGMAP_PAIR pairESG = m_mapESG.equal_range ( sTargetID );
	EFFSGMAP_ITER iter = pairESG.first;
	for ( ; iter != pairESG.second; )
	{
		DxEffSingleGroup* pEffSingleGroup = (*iter).second;
		if ( pEffSingleGroup->m_strFileName==strFileName )
		{
			EFFSGMAP_ITER iter_del = iter;
			++iter;

			SAFE_DELETE((*iter_del).second);
			m_mapESG.erase(iter_del);
		}
		else
		{
			++iter;
		}
	}
	return S_OK;
}

HRESULT DxEffExtra::DeletePassiveEffect ( const STARGETID &sTargetID )
{
	if ( sTargetID.dwID==EMTARGET_NULL )	return E_FAIL;

	EFFSGMAP_PAIR pairESG = m_mapESG.equal_range ( sTargetID );
	EFFSGMAP_ITER iter = pairESG.first;
	for ( ; iter != pairESG.second; )
	{
		DxEffSingleGroup* pEffSingleGroup = (*iter).second;
		EFFSGMAP_ITER iter_del = iter;
		++iter;

		SAFE_DELETE((*iter_del).second);
		m_mapESG.erase(iter_del);
	}

	return S_OK;
}

BOOL DxEffExtra::FindPassiveEffect ( const char* szFileName, const STARGETID &sTargetID )
{
	if ( sTargetID.dwID==EMTARGET_NULL )	return FALSE;

	std::string strFileName = szFileName;
	std::transform ( strFileName.begin(), strFileName.end(), strFileName.begin(), tolower );

	EFFSGMAP_PAIR pairESG = m_mapESG.equal_range ( sTargetID );
	EFFSGMAP_ITER iter = pairESG.first;
	for ( ; iter != pairESG.second; ++iter )
	{
		DxEffSingleGroup* pEffSingleGroup = (*iter).second;
		if ( pEffSingleGroup->m_strFileName==strFileName )	return TRUE;
	}

	return FALSE;
}

void DxEffExtra::RemoveMineEffect( const char* szFile )
{
	DeleteAllEff( szFile );
}

DxEffSingleGroup* DxEffExtra::NewLocatorAEff ( const char* szFileName, const D3DXMATRIX &matTrans, const STARGETID* pTargetID )
{
	GASSERT(m_pd3dDevice);
	EFF_PROPGROUP* pPropGroup;

	if ( bEffLocatorACreated )
	{
		if ( m_pEffLocatorA )
		{
			m_pEffLocatorA->SetTargetID( pTargetID );
			m_pEffLocatorA->m_matWorld = matTrans;
		}
	}

	if ( !bEffLocatorACreated )
	{
		pPropGroup = DxEffSinglePropGMan::GetInstance().LoadEffectGProp ( szFileName );
		if ( !pPropGroup )	return NULL;
		m_pEffLocatorA = NewEffGroup ( pPropGroup, matTrans, pTargetID );
		bEffLocatorACreated = true;
	}

	return m_pEffLocatorA;
}

DxEffSingleGroup* DxEffExtra::NewLocatorBEff ( const char* szFileName, const D3DXMATRIX &matTrans, const STARGETID* pTargetID )
{
	GASSERT(m_pd3dDevice);
	EFF_PROPGROUP* pPropGroup;

	if ( bEffLocatorBCreated )
	{
		if ( m_pEffLocatorB )
		{
			m_pEffLocatorB->SetTargetID( pTargetID );
			m_pEffLocatorB->m_matWorld = matTrans;
		}
	}

	if ( !bEffLocatorBCreated )
	{
		pPropGroup = DxEffSinglePropGMan::GetInstance().LoadEffectGProp ( szFileName );
		if ( !pPropGroup )	return NULL;
		m_pEffLocatorB = NewEffGroup ( pPropGroup, matTrans, pTargetID );
		bEffLocatorBCreated = true;
	}

	return m_pEffLocatorB;
}

void DxEffExtra::RemoveLocatorAEffect( const char* szFile )
{
	bEffLocatorACreated = FALSE;
	m_pEffLocatorA = NULL;
	DeleteAllEff( szFile );
}

void DxEffExtra::RemoveLocatorBEffect( const char* szFile )
{
	bEffLocatorBCreated = FALSE;
	m_pEffLocatorB = NULL;
	DeleteAllEff( szFile );
}