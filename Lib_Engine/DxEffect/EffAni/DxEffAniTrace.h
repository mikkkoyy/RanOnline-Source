#pragma once

#include "DxEffAni.h"

class DxSkinChar;
/*ABL system, Juver, 2017/05/29 */
class DxAttBone;
class DxAttBoneRender;

/*vehicle system, Juver, 2017/08/03 */
class DxVehicle;

class DxEffSingleGroup;

struct EFFANI_PROPERTY_TRACE : public EFFANI_PROPERTY
{
	float		m_fScale;
	std::string m_strTexName;	// 문제 있다. 

	EFFANI_PROPERTY_TRACE () :
		m_fScale(1.f)
	{
		m_strTexName = "";
	}
};

class DxEffAniData_Trace : public DxEffAniData
{
public:
	static DWORD		VERSION;
	static DWORD		TYPEID;
	static char			NAME[64];

public:
	virtual DWORD GetTypeID ()		{ return TYPEID; }
	virtual DWORD GetFlag ()		{ return NULL; }
	virtual const char* GetName ()	{ return NAME; }

public:
	union
	{
		struct
		{
			EFFANI_PROPERTY_TRACE		m_Property;
		};

		struct
		{
			float		m_fScale;
			std::string m_strTexName;	// 문제 있다. 
		};
	};

public:
	virtual void SetProperty ( EFFANI_PROPERTY *pProperty )
	{
		m_Property = *((EFFANI_PROPERTY_TRACE*)pProperty);
	}
	
	virtual EFFANI_PROPERTY* GetProperty ()
	{
		return &m_Property;
	}

public:
	virtual void SetEffAniData ( DxEffAniData*	pData );

public:
	virtual DxEffAni* NEWOBJ ( SAnimContainer* pAnimContainer );	//	Note : 이팩트 생성자.
	virtual HRESULT LoadFile ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	virtual HRESULT SaveFile ( CSerialFile &SFile );

public:
	virtual HRESULT InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );
	virtual HRESULT DeleteDeviceObjects ();

public:
	DxEffAniData_Trace () :
		DxEffAniData()
	{
	}
};

class DxEffAniTrace : public DxEffAni
{
public:
	float		m_fScale;
	std::string m_strTexName;

public:
	static HRESULT CreateDevice ( LPDIRECT3DDEVICEQ pd3dDevice );
	static HRESULT ReleaseDevice ( LPDIRECT3DDEVICEQ pd3dDevice );

public:
	virtual HRESULT OneTimeSceneInit ();
	virtual HRESULT InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );
	virtual HRESULT RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );
	virtual HRESULT InvalidateDeviceObjects ();
	virtual HRESULT DeleteDeviceObjects ();
	virtual HRESULT FinalCleanup ();

public:
	virtual HRESULT FrameMove( float fTime, float fElapsedTime );
	virtual HRESULT	Render( const LPDIRECT3DDEVICEQ pd3dDevice, DxSkinChar* pSkinChar, const D3DXMATRIX& matCurPos );

	/*ABL system, Juver, 2017/05/29 */
	virtual HRESULT	Render ( const LPDIRECT3DDEVICEQ pd3dDevice, DxAttBone* pAttBone, const D3DXMATRIX& matCurPos );
	virtual HRESULT	Render ( const LPDIRECT3DDEVICEQ pd3dDevice, DxAttBoneRender* pAttBone, const D3DXMATRIX& matCurPos );

	/*vehicle system, Juver, 2017/08/02 */
	virtual HRESULT	Render ( const LPDIRECT3DDEVICEQ pd3dDevice, DxVehicle* pVehicle, const D3DXMATRIX& matCurPos );
public:
	DxEffAniTrace(void);
	~DxEffAniTrace(void);
};

