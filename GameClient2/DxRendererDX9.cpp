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

void DxRendererDX9::Clear ( const RendererColor& Color )
{
	if ( !m_pd3dDevice ) return;
	D3DCOLOR color = D3DCOLOR_ARGB (
		static_cast<BYTE> ( Color.a * 255.0f ),
		static_cast<BYTE> ( Color.r * 255.0f ),
		static_cast<BYTE> ( Color.g * 255.0f ),
		static_cast<BYTE> ( Color.b * 255.0f ) );
	m_pd3dDevice->Clear ( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, color, 1.0f, 0L );
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
