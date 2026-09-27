#pragma once

#include "DxEffChar.h"

#include "../../DxCommon/TextureManager.h"
#include "DxMethods.h"

#define EFFCHARSPEC2_CUBE		0x0001

struct EFFCHAR_PROPERTY_SPECULAR2_104_105
{
	DWORD			m_dwFlag;
	DWORD			m_dwColorOP;
	DWORD			m_dwMaterials;
	D3DXCOLOR		m_cSpecular;
};

struct EFFCHAR_PROPERTY_SPECULAR2_106 : public EFFCHAR_PROPERTY
{
	DWORD			m_dwFlag;
	D3DXCOLOR		m_cSpecular;
	DWORD			m_dwMaterials;
	DWORD			m_dwColorOP;
	
	EFFCHAR_PROPERTY_SPECULAR2_106 () :
		m_cSpecular(0.8f,0.8f,0.8f,1.0f),
		m_dwMaterials(0L),
		m_dwColorOP(D3DTOP_MODULATE)	
	{
		m_dwFlag = 0L;
	}
};

struct EFFCHAR_PROPERTY_SPECULAR2 : public EFFCHAR_PROPERTY
{
	DWORD			m_dwFlag;
	D3DXCOLOR		m_cSpecular;
	DWORD			m_dwMaterials;
	DWORD			m_dwColorOP;
	
	BOOL			m_bCubeTexture;
	char			m_szCubeTexture[MAX_PATH];

	EFFCHAR_PROPERTY_SPECULAR2 () :
		m_cSpecular(0.8f,0.8f,0.8f,1.0f),
		m_dwMaterials(0L),
		m_dwColorOP(D3DTOP_MODULATE),
		m_bCubeTexture( FALSE )
	{
		m_dwFlag = 0L;
		SecureZeroMemory( m_szCubeTexture, sizeof(char)*MAX_PATH );
		StringCchCopy( m_szCubeTexture, MAX_PATH, "LobbyCube.dds" );
	}
};

class DxEffCharSpecular2 : public DxEffChar				// Ver.106
{
public:
	static DWORD		TYPEID;
	static DWORD		VERSION;
	static char			NAME[MAX_PATH];

	virtual DWORD GetTypeID ()		{ return TYPEID; }
	virtual DWORD GetFlag ()		{ return EMECF_SPECULAR2; }
	virtual const char* GetName ()	{ return NAME; }

protected:
	union
	{
		struct
		{
			EFFCHAR_PROPERTY_SPECULAR2 m_Property;
		};
		
		struct
		{
			DWORD			m_dwFlag;
			D3DXCOLOR		m_cSpecular;
			DWORD			m_dwMaterials;
			DWORD			m_dwColorOP;
			BOOL			m_bCubeTexture;
			char			m_szCubeTexture[MAX_PATH];
		};
	};

public:
	virtual void SetProperty ( EFFCHAR_PROPERTY *pProperty )
	{
		m_Property = *((EFFCHAR_PROPERTY_SPECULAR2*)pProperty);
	}
	
	virtual EFFCHAR_PROPERTY* GetProperty ()
	{
		return &m_Property;
	}

protected:	
	static LPDIRECT3DSTATEBLOCK9	m_pSavedStateBlock;
	static LPDIRECT3DSTATEBLOCK9	m_pEffectStateBlock;

	static LPDIRECT3DSTATEBLOCK9	m_pSavedCubeSB;
	static LPDIRECT3DSTATEBLOCK9	m_pDrawCubeSB;

protected:
	DXMATERIAL_CHAR_EFF*	m_pMaterials;
	DXMATERIAL_CHAR_EFF*	m_pTempMaterials;
	DWORD					m_dwTempMaterials;

	DWORD					m_dwMipMapCount;

	LPDIRECT3DCUBETEXTUREQ	m_pCubeTex;

protected:
	DxSkinMesh9*				m_pSkinMesh;
	PSMESHCONTAINER				m_pmcMesh;

public:
	virtual void SetMaterials ( LPDIRECT3DDEVICEQ pd3dDevice, DWORD dwMaterials, DXMATERIAL_CHAR_EFF* pMaterials );
	virtual DXMATERIAL_CHAR_EFF*		GetMaterials () { return m_pMaterials; }
	virtual DWORD						GetMaterialsNum() { return m_Property.m_dwMaterials; }

public:
	static HRESULT StaticCreateDevice ( LPDIRECT3DDEVICEQ pd3dDevice );
	static HRESULT StaticResetDevice ( LPDIRECT3DDEVICEQ pd3dDevice );
	static HRESULT StaticLostDevice();
	static void StaticDestroyDevice();

public:
	virtual HRESULT InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );
	virtual HRESULT DeleteDeviceObjects ();

public:
	virtual HRESULT FrameMove ( float fTime, float fElapsedTime );
	virtual void	Render ( LPDIRECT3DDEVICEQ pd3dDevice, BOOL bPieceRender=FALSE );

protected:
	void	Render_Vertex( LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bPieceRender );
	void	Render_Cube( LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bPieceRender );

public:
	virtual DxEffChar* CloneInstance ( LPDIRECT3DDEVICEQ pd3dDevice, DxCharPart* pCharPart, DxSkinPiece* pSkinPiece );

	virtual HRESULT LoadFile ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	virtual HRESULT SaveFile ( basestream &SFile );

public:
	DxEffCharSpecular2(void);
	~DxEffCharSpecular2(void);
};
