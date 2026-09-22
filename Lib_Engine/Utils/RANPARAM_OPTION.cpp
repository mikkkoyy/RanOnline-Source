#include "stdafx.h"
#include "./RANPARAM.h"

#include "Shlwapi.h"

#include "../Lib_Engine/Common/Rijndael.h"
#include "../Lib_Engine/Common/IniLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace RANPARAM
{
	std::string			strOptionFile		= "";
	
	BOOL LOAD_GAMEOPTION( const TCHAR *szRootPath, bool bServer )
	{
		strOptionFile = szRootPath;
		strOptionFile += "\\option.ini";

		if ( !PathFileExists( strOptionFile.c_str() ) )
		{
			return FALSE;
		}

		CIniLoader cFILE;

		if( !cFILE.open( strOptionFile, true) )
		{
			return FALSE;
		}

		// param.ini use not error msg box
		cFILE.SetUseErrorMsgBox( FALSE );

		cFILE.getflag( "GAME OPTION", "nChinaRegion", 0, 1, nChinaRegion );

		{
			cFILE.getflag ( "SCREEN OPTION", "bCHECKWHQL", 0, 1, bCHECKWHQL );
			cFILE.getflag ( "SCREEN OPTION", "dwScrWidth", 0, 1, dwScrWidth );
			cFILE.getflag ( "SCREEN OPTION", "dwScrHeight", 0, 1, dwScrHeight );
			int nTEMP(0);
			cFILE.getflag ( "SCREEN OPTION", "emScrFormat", 0, 1, nTEMP );
			emScrFormat = (EMSCREEN_FORMAT) nTEMP;
			cFILE.getflag ( "SCREEN OPTION", "uScrRefreshHz", 0, 1, uScrRefreshHz );
			cFILE.getflag ( "SCREEN OPTION", "bScrWindowed", 0, 1, bScrWindowed );
			cFILE.getflag ( "SCREEN OPTION", "bScrWndFullSize", 0, 1, bScrWndFullSize );
			cFILE.getflag ( "SCREEN OPTION", "bGameCursor", 0, 1, bGameCursor );
		}

		{
			cFILE.getflag ( "SOUND OPTION", "b3D_SOUND", 0, 1, b3D_SOUND );
			cFILE.getflag ( "SOUND OPTION", "nSndSfx", 0, 1, nSndSfx );
			cFILE.getflag ( "SOUND OPTION", "nSndMap", 0, 1, nSndMap );
			cFILE.getflag ( "SOUND OPTION", "nSndMusic", 0, 1, nSndMusic );
			cFILE.getflag ( "SOUND OPTION", "dw3DAlgorithm", 0, 1, dw3DAlgorithm );
			cFILE.getflag ( "SOUND OPTION", "dwSndBufferSize", 0, 1, dwSndBufferSize );
			cFILE.getflag ( "SOUND OPTION", "bSndMuteSfx", 0, 1, bSndMuteSfx );
			cFILE.getflag ( "SOUND OPTION", "bSndMuteMap", 0, 1, bSndMuteMap );
			cFILE.getflag ( "SOUND OPTION", "bSndMuteMusic", 0, 1, bSndMuteMusic );
		}

		{
			cFILE.getflag ( "GRAPHIC OPTION", "dwVideoLevel", 0, 1, dwVideoLevel );
			cFILE.getflag ( "GRAPHIC OPTION", "dwShadowChar", 0, 1, dwShadowChar );
			cFILE.getflag ( "GRAPHIC OPTION", "bBuff", 0, 1, bBuff );
			cFILE.getflag ( "GRAPHIC OPTION", "bShadowLand", 0, 1, bShadowLand );
			cFILE.getflag ( "GRAPHIC OPTION", "bRealReflect", 0, 1, bRealReflect );
			cFILE.getflag ( "GRAPHIC OPTION", "bRefract", 0, 1, bRefract );
			cFILE.getflag ( "GRAPHIC OPTION", "bGlow", 0, 1, bGlow );
			cFILE.getflag ( "GRAPHIC OPTION", "bPost", 0, 1, bPost );
			cFILE.getflag ( "GRAPHIC OPTION", "bFrameLimit", 0, 1, bFrameLimit );
			cFILE.getflag ( "GRAPHIC OPTION", "dwSkinDetail", 0, 1, dwSkinDetail );
			cFILE.getflag ( "GRAPHIC OPTION", "dwFogRange", 0, 1, dwFogRange );
			cFILE.getflag ( "GRAPHIC OPTION", "bBUFF_SFX", 0, 1, bBUFF_SFX );
			cFILE.getflag ( "GRAPHIC OPTION", "bEMOTICON", 0, 1, bEMOTICON );
			cFILE.getflag ( "GRAPHIC OPTION", "strFontType", 0, 1, strFontType );
			cFILE.getflag ( "GRAPHIC OPTION", "bClickEffect", 0, 1, bClickEffect );
			cFILE.getflag ( "GRAPHIC OPTION", "bTargetEffect", 0, 1, bTargetEffect );
			cFILE.getflag ( "GRAPHIC OPTION", "bMineEffect", 0, 1, bMineEffect );
			cFILE.getflag ( "GRAPHIC OPTION", "dwTextureFilter", 0, 1, dwTextureFilter );
			cFILE.getflag ( "GRAPHIC OPTION", "bCharacterSimple", 0, 1, bCharacterSimple );	/*character simple, Juver, 2017/10/01 */
			cFILE.getflag ( "GRAPHIC OPTION", "bHideSkillEffect", 0, 1, bHideSkillEffect );	/*skill effect setting, Juver, 2017/10/01 */
			cFILE.getflag ( "GRAPHIC OPTION", "bHideDamageDisplay", 0 ,1, bHideDamageDisplay );
			cFILE.getflag ( "GRAPHIC OPTION", "bCharacterMisc", 0 ,1, bCharacterMisc );
			cFILE.getflag ( "GRAPHIC OPTION", "bSpecialName", 0 ,1, bSpecialName );
			cFILE.getflag ( "GRAPHIC OPTION", "bClassicNameDisplay", 0 ,1, bClassicNameDisplay );
			cFILE.getflag ( "GRAPHIC OPTION", "bKillFeedDisplay", 0 ,1, bKillFeedDisplay );
			cFILE.getflag ( "GRAPHIC OPTION", "bHideDeadName", 0 ,1, bHideDeadName );
			cFILE.getflag ( "GRAPHIC OPTION", "bHideDeadBody", 0 ,1, bHideDeadBody );

			cFILE.getflag ( "GRAPHIC OPTION", "bPartyDisplay", 0, 1, bPartyDisplay );
			cFILE.getflag ( "GRAPHIC OPTION", "bPartyBuffDisplay", 0, 1, bPartyBuffDisplay );
		}

		if ( dwScrWidth < 1024 )
			dwScrWidth = 1024;

		if ( dwScrHeight < 768 )
			dwScrHeight = 768;

		if ( !bScrWndHalfSize )		bScrWndFullSize = TRUE;

		return TRUE;		
	}

	BOOL SAVE_GAMEOPTION ()
	{
		if ( strOptionFile.empty() )	return FALSE;

		CString strBuffer;

		strBuffer += _T("[GAME OPTION]\n");


		strBuffer.AppendFormat( "nChinaRegion = %d\n", nChinaRegion );

		{
			strBuffer += _T(";\n");
			strBuffer += _T("[SCREEN OPTION]\n");
			strBuffer.AppendFormat( "bCHECKWHQL	= %d\n", bCHECKWHQL );
			strBuffer.AppendFormat( "dwScrWidth	= %d\n", dwScrWidth );
			strBuffer.AppendFormat( "dwScrHeight = %d\n", dwScrHeight );
			strBuffer.AppendFormat( "emScrFormat = %d\n", emScrFormat );
			strBuffer.AppendFormat( "uScrRefreshHz = %d\n", uScrRefreshHz );
			strBuffer.AppendFormat( "bScrWindowed = %d\n", bScrWindowed );
			strBuffer.AppendFormat( "bScrWndFullSize = %d\n", bScrWndFullSize );
			strBuffer.AppendFormat( "bGameCursor = %d\n", bGameCursor );
		}

		{
			strBuffer += _T(";\n");
			strBuffer += _T("[SOUND OPTION]\n");
			strBuffer.AppendFormat( "b3D_SOUND = %d\n", b3D_SOUND );
			strBuffer.AppendFormat( "nSndSfx = %d\n", nSndSfx );
			strBuffer.AppendFormat( "nSndMap = %d\n", nSndMap );
			strBuffer.AppendFormat( "nSndMusic = %d\n", nSndMusic );
			strBuffer.AppendFormat( "dw3DAlgorithm = %d\n", dw3DAlgorithm );
			strBuffer.AppendFormat( "dwSndBufferSize = %d\n", dwSndBufferSize );
			strBuffer.AppendFormat( "bSndMuteSfx = %d\n", bSndMuteSfx );
			strBuffer.AppendFormat( "bSndMuteMap = %d\n", bSndMuteMap );
			strBuffer.AppendFormat( "bSndMuteMusic = %d\n", bSndMuteMusic );
		}

		{
			strBuffer += _T(";\n");
			strBuffer += _T("[GRAPHIC OPTION]\n");
			strBuffer.AppendFormat( "dwVideoLevel = %d\n", dwVideoLevel );
			strBuffer.AppendFormat( "dwShadowChar = %d\n", dwShadowChar );
			strBuffer.AppendFormat( "bBuff = %d\n", bBuff );
			strBuffer.AppendFormat( "bShadowLand = %d\n", bShadowLand );
			strBuffer.AppendFormat( "bRealReflect = %d\n", bRealReflect );
			strBuffer.AppendFormat( "bRefract = %d\n", bRefract );
			strBuffer.AppendFormat( "bGlow = %d\n", bGlow );
			strBuffer.AppendFormat( "bPost = %d\n", bPost );
			strBuffer.AppendFormat( "bFrameLimit = %d\n", bFrameLimit );
			strBuffer.AppendFormat( "dwSkinDetail = %d\n", dwSkinDetail );
			strBuffer.AppendFormat( "bBUFF_SFX = %d\n", bBUFF_SFX );
			strBuffer.AppendFormat( "bEMOTICON = %d\n", bEMOTICON );
			strBuffer.AppendFormat( "dwFogRange = %d\n", dwFogRange );
			strBuffer.AppendFormat( "strFontType = %s\n", strFontType );
			strBuffer.AppendFormat( "bClickEffect = %d\n", bClickEffect );
			strBuffer.AppendFormat( "bTargetEffect = %d\n", bTargetEffect );
			strBuffer.AppendFormat( "bMineEffect = %d\n", bMineEffect );
			strBuffer.AppendFormat( "dwTextureFilter = %d\n", dwTextureFilter );
			strBuffer.AppendFormat( "bCharacterSimple = %d\n", bCharacterSimple );	/*character simple, Juver, 2017/10/01 */
			strBuffer.AppendFormat( "bHideSkillEffect = %d\n", bHideSkillEffect );	/*skill effect setting, Juver, 2017/10/01 */
			strBuffer.AppendFormat( "bCharacterMisc = %d\n", bCharacterMisc );
			strBuffer.AppendFormat( "bHideDamageDisplay = %d\n", bHideDamageDisplay );
			strBuffer.AppendFormat( "bSpecialName = %d\n", bSpecialName );
			strBuffer.AppendFormat( "bClassicNameDisplay = %d\n", bClassicNameDisplay );
			strBuffer.AppendFormat( "bKillFeedDisplay = %d\n", bKillFeedDisplay );
			strBuffer.AppendFormat( "bPartyDisplay = %d\n", bPartyDisplay );
			strBuffer.AppendFormat( "bPartyBuffDisplay = %d\n", bPartyBuffDisplay );

			strBuffer.AppendFormat( "bHideDeadName = %d\n", bHideDeadName );
			strBuffer.AppendFormat( "bHideDeadBody = %d\n", bHideDeadBody );
		}

		CRijndael oRijndael;
		oRijndael.Initialize( CRijndael::VERSION, CRijndael::sm_Version[CRijndael::VERSION-1].c_str(), CRijndael::sm_KeyLength[CRijndael::VERSION-1] );

		int nLen = oRijndael.GetEncryptStringLength( strBuffer );
		char * szBuffer = new char[nLen];
		memset( szBuffer, 0, nLen );

		oRijndael.EncryptEx( strBuffer.GetString(), szBuffer, strBuffer.GetLength() );
		int nVersion = CRijndael::VERSION;

		if ( nVersion <= 0 && strlen( szBuffer ) <= 0 )	return FALSE;

		CFile file;
		if ( ! file.Open( strOptionFile.c_str(), CFile::modeCreate|CFile::modeWrite ) )	// 파일 열기
		{
			delete [] szBuffer;
			return FALSE;
		}

		file.Write( &nVersion, sizeof(int) );							// 버젼 쓰기
		file.Write( szBuffer, nLen );		// 파일 쓰기
		file.Close();

		delete [] szBuffer;

		return TRUE;
	}

};

