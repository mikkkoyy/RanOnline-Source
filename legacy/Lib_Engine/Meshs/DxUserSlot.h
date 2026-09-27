/*!
 * \file DxUserSlot.h
 *
 * \author Juver
 * \date May 2017
 *
 * 
 */

#pragma once

#include "../Common/basestream.h"
#include "../Common/SerialFile.h"

struct DXSUSER_SLOT
{
	std::string strUserSlotBone;
	D3DXMATRIX	matUserSlot;
	DWORD		dwUserSlot;

	DXSUSER_SLOT () :
		strUserSlotBone(""),
		dwUserSlot(NULL)
	{
		D3DXMatrixIdentity( &matUserSlot );
	}

	void SaveFile ( CSerialFile &SFile );
	void LoadFile ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
};
