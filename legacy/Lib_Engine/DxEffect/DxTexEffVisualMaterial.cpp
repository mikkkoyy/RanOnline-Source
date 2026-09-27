#include "stdafx.h"

#include "../G-Logic/GLDefine.h"
#include "../Common/SerialFile.h"
#include "./DxTexEffVisualMaterial.h"
#include "../DxCommon/DxCubeMap.h"
#include "../DxCommon/DxDynamicVB.h"
#include "../G-Logic/glperiod.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const DWORD DxTexEffVisualMaterial::TYPEID = DEF_TEXEFF_VISUALMATERIAL;
const DWORD	DxTexEffVisualMaterial::VERSION = 0x00000102;
const char DxTexEffVisualMaterial::NAME[] = "DxTexEffVisualMaterial";

TEXEFF_VISUALMATERIAL_PROPERTY::TEXEFF_VISUALMATERIAL_PROPERTY()
	: dw0(0)
	, dw1(0)
{
	strVisualMaterial ="";
	for ( int i=0; i<100; ++i )
	{
		strTextures[i] = "";
	}
}

TEXEFF_VISUALMATERIAL_PROPERTY::~TEXEFF_VISUALMATERIAL_PROPERTY()
{
}

void TEXEFF_VISUALMATERIAL_PROPERTY::Save( CSerialFile& SFile )
{
	SFile << strVisualMaterial;
	SFile << dw0;
	SFile << dw1;

	for ( DWORD i=0; i<dw0; ++i )
	{
		SFile << strTextures[i];
	}

	SFile << DWORD(4);
	SFile << DWORD(0);
}

void TEXEFF_VISUALMATERIAL_PROPERTY::Load_100( LPDIRECT3DDEVICEQ pd3dDevice, CSerialFile& SFile )
{
	SFile >> strVisualMaterial;
	SFile >> dw0;
	SFile >> dw1;

	for ( DWORD i=0; i<dw0; ++i )
	{
		SFile >> strTextures[i];
	}
}

void TEXEFF_VISUALMATERIAL_PROPERTY::Load_102( LPDIRECT3DDEVICEQ pd3dDevice, CSerialFile& SFile )
{
	SFile >> strVisualMaterial;
	SFile >> dw0;
	SFile >> dw1;

	for ( DWORD i=0; i<dw0; ++i )
	{
		SFile >> strTextures[i];
	}

	DWORD dwBlockSize = SFile.ReadBlockSize();
	SFile.SetOffSet ( SFile.GetfTell()+dwBlockSize );
}


DxTexEffVisualMaterial::DxTexEffVisualMaterial()
{
}

DxTexEffVisualMaterial::~DxTexEffVisualMaterial()
{
}

void DxTexEffVisualMaterial::SetProperty( LPDIRECT3DDEVICEQ pd3dDevice, PBYTE &pProp )
{
	TEXEFF_VISUALMATERIAL_PROPERTY* pTemp = (TEXEFF_VISUALMATERIAL_PROPERTY*)pProp;

	m_sProp.strVisualMaterial = pTemp->strVisualMaterial;
	m_sProp.dw0	= pTemp->dw0;
	m_sProp.dw1	= pTemp->dw1;

	for ( int i=0; i<100; ++i )
	{
		m_sProp.strTextures[i] = pTemp->strTextures[i];
	}
}

void DxTexEffVisualMaterial::OnCreateDevice( LPDIRECT3DDEVICEQ pd3dDevice )
{
}

void DxTexEffVisualMaterial::OnReleaseDevice( LPDIRECT3DDEVICEQ pd3dDevice )
{
}
void DxTexEffVisualMaterial::SavePSF( CSerialFile& SFile )
{
	SFile << VERSION;
	SFile.BeginBlock( EMBLOCK_02 );
	m_sProp.Save( SFile );
	SFile.EndBlock( EMBLOCK_02 );
}

void DxTexEffVisualMaterial::LoadPSF( LPDIRECT3DDEVICEQ pd3dDevice, CSerialFile& SFile )
{
	DWORD dwSize;
	DWORD dwVer;
	SFile >> dwVer;
	SFile >> dwSize;

	if( dwVer==VERSION )
	{
		m_sProp.Load_102( pd3dDevice, SFile );
	}
	else if( dwVer==0x00000100 )
	{
		m_sProp.Load_100( pd3dDevice, SFile );
	}
	else
	{
		DWORD dwCurBuffer = SFile.GetfTell();
		SFile.SetOffSet( dwCurBuffer+dwSize );
	}
}
