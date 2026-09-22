#include "stdafx.h"
#include "DxSkinPieceMaterial.h"
#include "../DxCommon/TextureManager.h"

#include "./DxMaterial.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HRESULT SMATERIAL_PIECE::LoadTexture ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;

	hr = TextureManager::LoadTexture ( m_strTexture.c_str(), pd3dDevice, m_pTexture, NULL, 0 );

	if (FAILED(hr))
	{
		hr = S_OK;
		m_pTexture = NULL;
	}
	else
	{
		m_dwTexID = TextureManager::GetTextureID( m_strTexture.c_str() );;

		//	Note : 텍스쳐 읽기에 성공시에 이 메터리얼의 초기 값을 결정.
		TextureManager::EM_TEXTYPE emType = TextureManager::GetTexType( m_strTexture.c_str() );

		if( emType==TextureManager::EMTT_ALPHA_HARD )			m_dwFlags |= DXMATERIAL::FLAG_ALPHA_HARD;
		else if( emType==TextureManager::EMTT_ALPHA_SOFT )		m_dwFlags |= DXMATERIAL::FLAG_ALPHA_SOFT;
		else if( emType==TextureManager::EMTT_ALPHA_SOFT01 )	m_dwFlags |= DXMATERIAL::FLAG_ALPHA_SOFT;
		else if( emType==TextureManager::EMTT_ALPHA_SOFT02 )	m_dwFlags |= DXMATERIAL::FLAG_ALPHA_SOFT;
	}

	return hr;
}

void SMATERIAL_PIECE::SaveFile ( CSerialFile &SFile )
{
	SFile << (DWORD)VERSION;
	SFile.BeginBlock();
	{
		SFile << m_dwFlags;
		SFile << m_dwZBias;

		DWORD dwLength = static_cast<DWORD> ( m_strTexture.length() + 1 );
		SFile << dwLength;
		SFile.WriteBuffer ( m_strTexture.c_str(), dwLength );
	}
	SFile.EndBlock();
}

void SMATERIAL_PIECE::LoadFile ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	DWORD dwVERSION, dwBLOCKSIZE;
	SFile >> dwVERSION;
	SFile >> dwBLOCKSIZE;

	if ( dwVERSION == VERSION )
	{
		SFile >> m_dwFlags;
		SFile >> m_dwZBias;

		m_fZBias = (float)m_dwZBias*0.00001f;

		DWORD dwLength;
		SFile >> dwLength;
		char* szBuffer = (char*) _alloca ( sizeof(char)*dwLength );
		SFile.ReadBuffer ( szBuffer, dwLength );
		m_strTexture = szBuffer;
	}
	else if ( dwVERSION == 0x0104 )
	{
		//must be shader by official
		DWORD dw0104_1, dw0104_2, dw0104_3;
		D3DXVECTOR3 v0104_1, v0104_2, v0104_3;
		D3DXVECTOR2 v0104_4, v0104_5;

		SFile >> dw0104_1;
		SFile >> dw0104_2;
		SFile >> v0104_1;
		SFile >> v0104_2;
		SFile >> v0104_3;
		SFile >> dw0104_3;
		SFile.ReadBuffer ( v0104_4, sizeof(D3DXVECTOR2) );
		SFile.ReadBuffer ( v0104_5, sizeof(D3DXVECTOR2) );

		DWORD dwLength;
		SFile >> dwLength;
		char* szBuffer = (char*) _alloca ( sizeof(char)*dwLength );
		SFile.ReadBuffer ( szBuffer, dwLength );
		m_strTexture = szBuffer;
		m_fZBias = (float)m_dwZBias*0.00001f;
	}
	else if ( dwVERSION == 0x0102 )
	{
		DWORD dw0102_1;

		SFile >> m_dwFlags;
		SFile >> m_dwZBias;
		SFile >> dw0102_1;

		m_fZBias = (float)m_dwZBias*0.00001f;

		DWORD dwLength;
		SFile >> dwLength;
		char* szBuffer = (char*) _alloca ( sizeof(char)*dwLength );
		SFile.ReadBuffer ( szBuffer, dwLength );
		m_strTexture = szBuffer;
	}
	else if ( dwVERSION == 0x0101 )
	{
		SFile >> m_dwFlags;
		SFile >> m_dwZBias;

		m_fZBias = (float)m_dwZBias*0.00001f;

		DWORD dwLength;
		SFile >> dwLength;
		char* szBuffer = (char*) _alloca ( sizeof(char)*dwLength );
		SFile.ReadBuffer ( szBuffer, dwLength );
		m_strTexture = szBuffer;
	}
	else if ( dwVERSION == 0x0100 )
	{
		DWORD dwLength;
		SFile >> dwLength;
		char* szBuffer = (char*) _alloca ( sizeof(char)*dwLength );
		SFile.ReadBuffer ( szBuffer, dwLength );
		m_strTexture = szBuffer;

		SFile >> m_dwFlags;
		SFile >> m_dwZBias;

		m_fZBias = (float)m_dwZBias*0.00001f;
	}
	else
	{
		CDebugSet::ErrorVersion ( "SMATERIAL_PIECE::LoadFile", SFile.GetFileName(), dwVERSION );
		SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	LoadTexture ( pd3dDevice );
}