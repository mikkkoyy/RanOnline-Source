#pragma once

#include "IRenderer.h"

class DxRendererDX9 : public IRenderer
{
public:
	DxRendererDX9 ();
	virtual ~DxRendererDX9 ();

	bool Initialize ( LPDIRECT3DDEVICEQ pd3dDevice );
	void Shutdown ();

	// IRenderer
	virtual void Clear ( D3DCOLOR Color ) override;
	virtual bool BeginScene () override;
	virtual void EndScene () override;
	virtual bool IsValid () const override;

private:
	LPDIRECT3DDEVICEQ m_pd3dDevice;
	bool m_bValid;
};
