#include "stdafx.h"
#include "DxRendererDX9.h"

DxRendererDX9::DxRendererDX9 () :
	m_pd3dDevice ( NULL ),
	m_bValid ( false )
{
}

DxRendererDX9::~DxRendererDX9 ()
{
	Shutdown ();
}

bool DxRendererDX9::Initialize ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	m_pd3dDevice = pd3dDevice;
	m_bValid = ( m_pd3dDevice != NULL );
	return m_bValid;
}

void DxRendererDX9::Shutdown ()
{
	m_pd3dDevice = NULL;
	m_bValid = false;
}

void DxRendererDX9::Clear ( D3DCOLOR Color )
{
	if ( !m_pd3dDevice ) return;
	m_pd3dDevice->Clear ( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, Color, 1.0f, 0L );
}

bool DxRendererDX9::BeginScene ()
{
	if ( !m_pd3dDevice ) return false;
	return SUCCEEDED ( m_pd3dDevice->BeginScene () );
}

void DxRendererDX9::EndScene ()
{
	if ( !m_pd3dDevice ) return;
	m_pd3dDevice->EndScene ();
}

bool DxRendererDX9::IsValid () const
{
	return m_bValid && ( m_pd3dDevice != NULL );
}
