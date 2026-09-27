#include "stdafx.h"

#include "../DxCommon/DxFontMan.h"
#include "../DxCommon/TextureManager.h"
#include "../TextTexture/TextUtil.h"
#include "../DxResponseMan.h"

#include "./CustomDevice.h"

CCustomDevice* g_pCustomDevice = NULL;

CCustomDevice::CCustomDevice(
	EMSERVICE_PROVIDER eProvider,
	RANPATH* pSubPath,
	const CString& strPath,
	CWnd* pParent,
	const CString& strFontType,
	language::LANGFLAG emLangSet,
	const CString& strGDIFont )
	: m_strPath( strPath )
	, m_pParent( pParent )
	, m_strFontType( strFontType )
	, m_emLangSet( emLangSet )
	, m_strGDIFont( strGDIFont )
{
}

CCustomDevice::~CCustomDevice()
{
}

HRESULT CCustomDevice::OnDeviceCreated(
    IDirect3DDevice9* pd3dDevice,
    const D3DSURFACE_DESC* pBackBufferSurfaceDesc )
{
    CTextUtil::m_bUsage = TRUE;

	DxResponseMan::GetInstance().OneTimeSceneInit(
        m_strPath.GetString(),
		m_pParent,
        m_strFontType,
        m_emLangSet,
        m_strGDIFont );	

    DxFontMan::GetInstance().InitDeviceObjects( pd3dDevice );

    DxResponseMan::GetInstance().InitDeviceObjects( pd3dDevice, FALSE );

	return S_OK;
}

VOID CCustomDevice::OnDeviceDestroyed()
{
    DxResponseMan::GetInstance().DeleteDeviceObjects();

	DxResponseMan::GetInstance().FinalCleanup();
}

HRESULT CCustomDevice::OnDeviceReset(
    IDirect3DDevice9* pd3dDevice,
    const D3DSURFACE_DESC* pBackBufferSurfaceDesc )
{
    DxResponseMan::GetInstance().RestoreDeviceObjects();

	return S_OK;
}

VOID CCustomDevice::OnDeviceLost()
{
    DxResponseMan::GetInstance().InvalidateDeviceObjects();
}

VOID CCustomDevice::OnFrameMove(
    IDirect3DDevice9* pd3dDevice,
    DOUBLE fTime,
    FLOAT fElapsedTime )
{
	DxResponseMan::GetInstance().FrameMove( FLOAT( fTime ), fElapsedTime, FALSE );
}

VOID CCustomDevice::OnFrameRender(
    IDirect3DDevice9* pd3dDevice,
    DOUBLE fTime,
    FLOAT fElapsedTime )
{
	CTextUtil::Get()->FrameMove();
}