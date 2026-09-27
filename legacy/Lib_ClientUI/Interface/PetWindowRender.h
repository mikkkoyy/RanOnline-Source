/*!
 * \file PetWindowRender.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "DxSkinChar.h"
#include "GLItem.h"
#include "GLCharData.h"

class	CPetWindowRender : public CUIGroup
{

private:
	enum
	{
	};

public:
	CPetWindowRender ();
	virtual	~CPetWindowRender ();

public:
	void	CreateSubControl ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual HRESULT RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );
	virtual HRESULT DeleteDeviceObjects ();
	virtual HRESULT Render ( LPDIRECT3DDEVICEQ pd3dDevice );
	virtual HRESULT FrameMove( LPDIRECT3DDEVICEQ pd3dDevice, float fTime, float fElapsedTime );

public:
	DxSkinChar*			m_pSkinChar;
	D3DXMATRIX			m_matTrans;
	
	WORD				m_wLastCOLOR;
	WORD				m_wLastSTYLE;
	SITEMCUSTOM			m_PutOnItems[PET_ACCETYPE_SIZE];

	bool				m_bReady;
	float				m_fRenderDelay;
	D3DXVECTOR3			m_vRot;

	float				m_fScale;
	D3DXVECTOR3			m_vTrans;

public:
	void	CreateModel( LPDIRECT3DDEVICEQ pd3dDevice );
	void	RenderModel( LPDIRECT3DDEVICEQ pd3dDevice );
	void	UpdateSuit( LPDIRECT3DDEVICEQ pd3dDevice );
	void	ClearRender();

};