#pragma once

#include "IRenderer.h"
#include <d3d11.h>
#include <dxgi.h>

class DxRendererDX11 : public IRenderer
{
public:
	DxRendererDX11 ();
	virtual ~DxRendererDX11 ();

	bool Initialize ( HWND hWnd );
	void Shutdown ();

	// IRenderer
	virtual void Clear ( const RendererColor& Color ) override;
	virtual bool BeginScene () override;
	virtual void EndScene () override;
	virtual bool IsValid () const override;

private:
	ID3D11Device* m_pd3dDevice;
	ID3D11DeviceContext* m_pContext;
	IDXGISwapChain* m_pSwapChain;
	ID3D11RenderTargetView* m_pRenderTarget;
	D3D11_VIEWPORT m_Viewport;
	bool m_bValid;
};
