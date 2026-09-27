#include "stdafx.h"
#include "./DxLandDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void SLAND_FILEMARK::Assign ( SLAND_FILEMARK_000 &sOldMark )
{
	dwNAVI_MARK = sOldMark.dwNAVI_MARK;
	dwGATE_MARK = sOldMark.dwGATE_MARK;
	dwCOLL_MARK = sOldMark.dwCOLL_MARK;
}

void SLAND_FILEMARK::Assign ( SLAND_FILEMARK_100 &sOldMark )
{
	dwNAVI_MARK = sOldMark.dwNAVI_MARK;
	dwGATE_MARK = sOldMark.dwGATE_MARK;
	dwCOLL_MARK = sOldMark.dwCOLL_MARK;
	dwWEATHER_MARK = sOldMark.dwWEATHER_MARK;
}

BOOL SLAND_FILEMARK::LoadSet ( CSerialFile &SFile )
{
	DWORD dwVer, dwSize;

	SFile >> dwVer;
	SFile >> dwSize;

	if ( dwVer == VERSION )
	{
		SFile.ReadBuffer ( this, sizeof(SLAND_FILEMARK) );
	}
	else if ( dwVer == 0x0100 )
	{
		SLAND_FILEMARK_100 sProp;
		SFile.ReadBuffer ( &sProp, sizeof(SLAND_FILEMARK_100) );
		Assign( sProp );
	}
	else
	{
		CDebugSet::ErrorVersion( "SLAND_FILEMARK::LoadSet", SFile.GetFileName(), dwVer );
		SFile.SetOffSet ( SFile.GetfTell() + dwSize );
		return FALSE;
	}

	return TRUE;
}

BOOL SLAND_FILEMARK::SaveSet ( CSerialFile &SFile )
{
	SFile << static_cast<DWORD> ( VERSION );
	SFile << static_cast<DWORD> ( sizeof(SLAND_FILEMARK) );

	SFile.WriteBuffer ( this, sizeof(SLAND_FILEMARK) );

	return TRUE;
}
