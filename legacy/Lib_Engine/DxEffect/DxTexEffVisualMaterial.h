#pragma once

#include <string>
#include "DxTexEff.h"
#include "DxVertexFVF.h"

class CSerialFile;
struct DxMeshes;


enum EMVISUALMATERIAL_TYPE
{
	EMVISUALMATERIAL_TYPE_NONE		= 0,
	EMVISUALMATERIAL_TYPE_DIFFUSE	= 1,
	EMVISUALMATERIAL_TYPE_FLOWUV	= 2,
	EMVISUALMATERIAL_TYPE_ROTATE	= 3,
	EMVISUALMATERIAL_TYPE_SPECULAR	= 4,
};

struct TEXEFF_VISUALMATERIAL_PROPERTY
{
	//original members
	std::string		strVisualMaterial;
	DWORD			dw0; //texture num
	DWORD			dw1; //flag
	std::string		strTextures[100];

	void Save( CSerialFile& SFile );
	void Load_100( LPDIRECT3DDEVICEQ pd3dDevice, CSerialFile& SFile );
	void Load_102( LPDIRECT3DDEVICEQ pd3dDevice, CSerialFile& SFile );


	TEXEFF_VISUALMATERIAL_PROPERTY();
	~TEXEFF_VISUALMATERIAL_PROPERTY();
};

class DxTexEffVisualMaterial : public DxTexEffBase
{
public:
	const static DWORD	TYPEID;
	const static DWORD	VERSION;
	const static char	NAME[];

protected:
	TEXEFF_VISUALMATERIAL_PROPERTY	m_sProp;

public:
	virtual DWORD GetTypeID()				{ return TYPEID; }
	virtual const char* GetName()			{ return NAME; }
	virtual void GetProperty( PBYTE &pProp ) { pProp = (PBYTE)&m_sProp; }
	virtual void SetProperty( LPDIRECT3DDEVICEQ pd3dDevice, PBYTE &pProp );

public:
	static void OnCreateDevice( LPDIRECT3DDEVICEQ pd3dDevice );
	static void OnReleaseDevice( LPDIRECT3DDEVICEQ pd3dDevice );

public:
	TEXEFF_VISUALMATERIAL_PROPERTY* GetProperty()	{ return &m_sProp; }

public:
	virtual void SavePSF( CSerialFile& SFile );
	virtual void LoadPSF( LPDIRECT3DDEVICEQ pd3dDevice, CSerialFile& SFile );

public:
	DxTexEffVisualMaterial();
	virtual ~DxTexEffVisualMaterial();
};
