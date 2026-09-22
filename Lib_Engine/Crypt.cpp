#include "stdafx.h"
#include "Crypt.h"
#include "SUBPATH.h"
#include "GLOGIC.h"
#include "../Common/IniLoader.h"
#include "../Common/GLTexFile.h" // Eduj Development OFFICIAL CRYPTION READ


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace CCrypt
{
	BOOL	bCryptRCC = TRUE;
	BOOL	bEngineDebug = TRUE;

	void  LoadSetting( const TCHAR *szRootPath )
	{
		CString strFileName = szRootPath;
		strFileName += "\\GameSet.ini";

		CIniLoader cFILE;

		DWORD	dwKey = 0;
			
		if( cFILE.open( strFileName.GetString(), true) )
		{
			cFILE.SetUseErrorMsgBox( FALSE );

			cFILE.getflag ( "GAME_SET", "dwCryptKey", 0, 1, dwKey );
			cFILE.getflag ( "GAME_SET", "bEngineDebug", 0, 1, bEngineDebug );
		}
		else
		{
			CDebugSet::ToLogFile("CCrypt::LoadSetting Cant Load Setting File");
		}
		if ( dwKey == 7892756 )	bCryptRCC = FALSE;

	}

	void Encryption(BYTE *B ,int dwSize)
	{
		for( int i = 0 ; i < dwSize ; i ++ )
		{
			B[i] = B[i] ^ EN;
			B[i] += ( EN ^ EN2 );
		};
		return ;
	}
	void Decryption(BYTE *B,int dwSize)
	{
		for( int i = 0 ; i < dwSize ; i ++ )
		{
			B[i] -= ( EN ^ EN2 );
			B[i] = B[i] ^ EN;
		};
		return ;
	}
};
