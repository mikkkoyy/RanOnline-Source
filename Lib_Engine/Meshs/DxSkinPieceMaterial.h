/*!
 * \file DxSkinPieceMaterial.h
 *
 * \author Juver
 * \date December 2016
 *
 * 
 */

#pragma once

#include "../Common/basestream.h"
#include "../Common/SerialFile.h"

struct SMATERIAL_PIECE
{
	enum
	{
		VERSION				= 0x0200,
		FLAG_ALPHATEXTURE	= 0x0001,
	};

	std::string			m_strTexture;
	LPDIRECT3DTEXTUREQ	m_pTexture;

	DWORD				m_dwFlags;
	DWORD				m_dwZBias;
	float				m_fZBias;			// Optimize - m_dwZBias -	Don`t Save Load
	DWORD				m_dwDyVertStart;	// 동적버텍스 렌더시작점-	Don`t Save Load
	DWORD				m_dwTexID;			// 텍스쳐 ID			-	Don`t Save Load

	SMATERIAL_PIECE () :
		m_pTexture(NULL),
		m_dwFlags(NULL),
		m_dwZBias(0),
		m_fZBias(0.f),
		m_dwDyVertStart(0),
		m_dwTexID(0)
	{
	}

	void SaveFile ( CSerialFile &SFile );
	void LoadFile ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );

	HRESULT LoadTexture ( LPDIRECT3DDEVICEQ pd3dDevice );

	SMATERIAL_PIECE ( const SMATERIAL_PIECE &value )
	{
		operator= ( value );
	}

	void operator= ( const SMATERIAL_PIECE &value )
	{
		m_strTexture = value.m_strTexture;
		m_pTexture = value.m_pTexture;
		m_dwFlags = value.m_dwFlags;
		m_dwZBias = value.m_dwZBias;
	}
};
