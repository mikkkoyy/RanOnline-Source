#include "stdafx.h"
#include "DxUserSlot.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void DXSUSER_SLOT::SaveFile ( CSerialFile &SFile )
{
	SFile << strUserSlotBone;
	SFile.WriteBuffer ( matUserSlot, sizeof(D3DXMATRIX) );
	SFile << dwUserSlot;
}

void DXSUSER_SLOT::LoadFile ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	SFile >> strUserSlotBone;
	SFile.ReadBuffer ( matUserSlot, sizeof(D3DXMATRIX) );
	SFile >> dwUserSlot;
}