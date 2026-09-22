/*!
 * \file VehicleWindowRender.h
 *
 * \author Juver
 * \date August 2017
 *
 * \vehicle system
 */

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "DxSkinChar.h"
#include "GLItem.h"
#include "GLVehicle.h"

#define RENDER_DELAY_TIME 0.2f

class	CVehicleWindowRender : public CUIGroup
{

private:
	enum
	{
	};

public:
	CVehicleWindowRender ();
	virtual	~CVehicleWindowRender ();

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
	SVEHICLEITEMINFO	m_sVehicleInfo;
	SNATIVEID			m_sVehicleID;
	bool				m_bReady;
	float				m_fRenderDelay;
	D3DXVECTOR3			m_vRot;
	D3DXVECTOR3			m_vTrans;
	float				m_fScale;
	bool				m_bUpdateSuit;

public:
	void	CreateModel( LPDIRECT3DDEVICEQ pd3dDevice );
	void	RenderModel( LPDIRECT3DDEVICEQ pd3dDevice );
	void	UpdateSuit( LPDIRECT3DDEVICEQ pd3dDevice );
	void	ClearRender();
	void	UpdateAnimation();
	void	RenderVehicle( SNATIVEID sID, SVEHICLEITEMINFO sInfo );

};