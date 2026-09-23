#include "stdafx.h"
#include "DxRendererDX11.h"

DxRendererDX11::DxRendererDX11 () :
	m_pd3dDevice ( NULL ),
	m_pContext ( NULL ),
	m_pSwapChain ( NULL ),
	m_pRenderTarget ( NULL ),
	m_bValid ( false )
{
	ZeroMemory ( &m_Viewport, sizeof ( m_Viewport ) );
}

DxRendererDX11::~DxRendererDX11 ()
{
	Shutdown ();
}

bool DxRendererDX11::Initialize ( HWND hWnd )
{
	if ( !hWnd ) return false;

	HRESULT hr;

	UINT createFlags = 0;
#ifdef _DEBUG
	createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels [] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
	D3D_FEATURE_LEVEL featuredLevel;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory ( &sd, sizeof ( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	hr = D3D11CreateDeviceAndSwapChain (
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		createFlags,
		featureLevels,
		sizeof ( featureLevels ) / sizeof ( featureLevels [0] ),
		D3D11_SDK_VERSION,
		&sd,
		&m_pSwapChain,
		&m_pd3dDevice,
		&featuredLevel,
		&m_pContext );

	if ( FAILED ( hr ) )
	{
		hr = D3D11CreateDeviceAndSwapChain (
			NULL,
			D3D_DRIVER_TYPE_WARP,
			NULL,
			createFlags,
			featureLevels,
			sizeof ( featureLevels ) / sizeof ( featureLevels [0] ),
			D3D11_SDK_VERSION,
			&sd,
			&m_pSwapChain,
			&m_pd3dDevice,
			&featuredLevel,
			&m_pContext );
	}

	if ( FAILED ( hr ) )
	{
		m_pd3dDevice = NULL;
		m_pContext = NULL;
		m_pSwapChain = NULL;
		return false;
	}

	ID3D11Texture2D* pBackBuffer = NULL;
	hr = m_pSwapChain->GetBuffer ( 0, __uuidof ( ID3D11Texture2D ), ( LPVOID* ) &pBackBuffer );
	if ( FAILED ( hr ) || !pBackBuffer )
	{
		if ( pBackBuffer ) pBackBuffer->Release ();
		Shutdown ();
		return false;
	}

	hr = m_pd3dDevice->CreateRenderTargetView ( pBackBuffer, NULL, &m_pRenderTarget );
	if ( pBackBuffer ) pBackBuffer->Release ();
	if ( FAILED ( hr ) || !m_pRenderTarget )
	{
		Shutdown ();
		return false;
	}

	m_Viewport.TopLeftX = 0.0f;
	m_Viewport.TopLeftY = 0.0f;
	m_Viewport.Width = 0.0f;
	m_Viewport.Height = 0.0f;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	m_bValid = true;
	return true;
}

void DxRendererDX11::Shutdown ()
{
	if ( m_pRenderTarget ) { m_pRenderTarget->Release (); m_pRenderTarget = NULL; }
	if ( m_pSwapChain )  { m_pSwapChain->Release ();  m_pSwapChain = NULL; }
	if ( m_pContext )    { m_pContext->Release ();     m_pContext = NULL; }
	if ( m_pd3dDevice )  { m_pd3dDevice->Release ();   m_pd3dDevice = NULL; }
	m_bValid = false;
}

void DxRendererDX11::Clear ( D3DCOLOR Color )
{
	if ( !m_pd3dDevice || !m_pRenderTarget ) return;
	float color[4];
	color[0] = ( ( ( Color >> 16 ) & 0xFF ) / 255.0f );
	color[1] = ( ( ( Color >> 8 ) & 0xFF ) / 255.0f );
	color[2] = ( ( Color & 0xFF ) / 255.0f );
	color[3] = ( ( ( Color >> 24 ) & 0xFF ) / 255.0f );
	m_pContext->ClearRenderTargetView ( m_pRenderTarget, color );
}

bool DxRendererDX11::BeginScene ()
{
	return m_bValid && ( m_pd3dDevice != NULL );
}

void DxRendererDX11::EndScene ()
{
	if ( !m_pSwapChain ) return;
	m_pSwapChain->Present ( 0, 0 );
}

bool DxRendererDX11::IsValid () const
{
	return m_bValid && ( m_pd3dDevice != NULL );
}
