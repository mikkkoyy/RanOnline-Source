#include "stdafx.h"
#include "./RANPARAM.h"

#include "../Lib_Engine/Common/IniLoader.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace RANPARAM
{
	BOOL LOAD ( const TCHAR *szRootPath, bool bServer )
	{
		if ( !LOAD_PARAM ( szRootPath ) )
		{
			return FALSE;
		}

		if ( !LOAD_GAMEOPTION( szRootPath, bServer ) )
		{
			return FALSE;
		}

		if ( !LOAD_FEATURE( szRootPath ) )
		{
			return FALSE;
		}

		//	중국일 경우 지역 선택시 값을 갱신해준다.
#ifdef CH_PARAM

		if ( nChinaRegion == MAX_CHINA_REGION )	return TRUE;

		StringCchCopy( LoginAddress, STRING_NUM_128, China_Region[nChinaRegion].LoginAddress.GetString() );
		nLoginPort =  China_Region[nChinaRegion].nLoginPort; /*login port, Juver, 2017/11/16 */

		for( int i=0 ; ; ++i )
		{
			if( China_Region[nChinaRegion].HttpAddressTable[i].IsEmpty() ) 
				break;

			StringCchCopy( HttpAddressTable[i], STRING_NUM_128, China_Region[nChinaRegion].HttpAddressTable[i].GetString() );
		}

		strNewsURL = China_Region[nChinaRegion].strNewsURL;
#endif

		return TRUE;
	}

	BOOL LOAD_PARAM( const TCHAR *szRootPath )
	{
		std::string	strParamFile(szRootPath);
		strParamFile += "\\param.ini";

		CIniLoader cFILE;

		if( !cFILE.open( strParamFile, true) )
		{
			return FALSE;
		}

		// param.ini use not error msg box
		cFILE.SetUseErrorMsgBox( FALSE );

		{
			cFILE.getflag_sz( "SERVER SET", "LoginAddress", 0, 1, LoginAddress, STRING_NUM_128 );
			cFILE.getflag ( "SERVER SET", "nLoginPort", 0, 1, nLoginPort );	 /*login port, Juver, 2017/11/16 */

			UINT nNum = cFILE.GetKeyDataSize( "SERVER SET", "HttpAddress" );

			memset( HttpAddressTable, 0, sizeof(HttpAddressTable) );

			for ( UINT i=0; i<nNum; ++i )
			{
				CString strHttpAddress;
				cFILE.getflag( "SERVER SET", "HttpAddress", i, nNum, strHttpAddress );
				strHttpAddress.Trim( _T(" ") );

				StringCchCopy( HttpAddressTable[i], STRING_NUM_128, strHttpAddress );
			}

			cFILE.getflag ( "SERVER SET", "strNewsURL", 0, 1, strNewsURL );
			cFILE.getflag ( "SERVER SET", "strConnectionURL", 0, 1, strConnectionURL );
			cFILE.getflag ( "SERVER SET", "bUsePassiveDN", 0, 1, bUsePassiveDN );			
		}

#ifdef CH_PARAM
		{
			CString strSection;

			for( UINT i=0; i<MAX_CHINA_REGION; ++i )
			{
				strSection.Format( "CH_REGION_%d", i );
				cFILE.getflag( strSection.GetString(), "strName", 0, 1, China_Region[i].strName );
				cFILE.getflag( strSection.GetString(), "LoginAddress", 0, 1, China_Region[i].LoginAddress );
				cFILE.getflag( strSection.GetString(), "nLoginPort", 0, 1, China_Region[i].nLoginPort );	 /*login port, Juver, 2017/11/16 */

				UINT nNum = cFILE.GetKeyDataSize( strSection.GetString(), "HttpAddress" );
				if( nNum > MAX_HTTP ) nNum = MAX_HTTP;
				for( UINT j=0; j<nNum; ++j )
				{
					cFILE.getflag( strSection.GetString(), "HttpAddress", j, nNum, China_Region[i].HttpAddressTable[j] );
					China_Region[i].HttpAddressTable[j].Trim( _T(" ") );
				}

				cFILE.getflag( strSection.GetString(), "strNewsURL", 0, 1, China_Region[i].strNewsURL );

				nNum = cFILE.GetKeyDataSize( strSection.GetString(), "ServerName" );
				if( nNum > MAX_SERVER ) nNum = MAX_SERVER;
				for( UINT j=0; j<nNum; ++j )
				{
					cFILE.getflag( strSection.GetString(), "ServerName", j, nNum, China_Region[i].ServerName[j] );
					China_Region[i].ServerName[j].Trim( _T(" ") );
				}

				cFILE.getflag( strSection.GetString(), "ItemShopAddress", 0, 1, China_Region[i].strItemShop );
			}
		}
#endif

		{
			cFILE.getflag ( "GUI OPTION", "dwLangSet", 0, 1, dwLangSet );
			cFILE.getflag ( "GUI OPTION", "strGDIFont", 0, 1, strGDIFont );
		}

		{
			cFILE.getflag ( "TEXT FILE", "strCountry", 0, 1, strCountry );
			cFILE.getflag ( "TEXT FILE", "strLauncherText", 0, 1, strLauncherText );	
			cFILE.getflag ( "TEXT FILE", "strGameInText", 0, 1, strGameInText );
			cFILE.getflag ( "TEXT FILE", "strGameExText", 0, 1, strGameExText );
			cFILE.getflag ( "TEXT FILE", "strGameWord", 0, 1, strGameWord );
			cFILE.getflag ( "TEXT FILE", "strServerText", 0, 1, strServerText );
			//xml translation
			cFILE.getflag ( "TEXT FILE", "strGameWordNpc", 0, 1, strGameWordNpc );
			cFILE.getflag ( "TEXT FILE", "strGameWordQuest", 0, 1, strGameWordQuest );

			cFILE.getflag ( "TEXT FILE", "strLangPlacer", 0, 1, strLangPlacer );
		}

		// 도움말 웹 페이지 주소, 아직 사용 안함 ( 준혁 )
		cFILE.getflag_sz( "ETC OPTION", "HelpAddress", 0, 1, HelpAddress, STRING_NUM_128 );
		cFILE.getflag_sz( "ETC OPTION", "ItemShopAddress", 0, 1, ItemShopAddress, STRING_NUM_128 );
		cFILE.getflag_sz( "ETC OPTION", "RPFDataPath", 0, 1, RPFDataPath, STRING_NUM_128 );
		cFILE.getflag_sz( "ETC OPTION", "ClientWindowTitle", 0, 1, ClientWindowTitle, STRING_NUM_128 );

		//	Note : 유효성 체크.
		//
		if ( COMMENT::FONTSYS[0]!=strFontType.GetString() && COMMENT::FONTSYS[1]!=strFontType.GetString() )
		{
			strFontType = COMMENT::FONTSYS[0].c_str();
		}		

		//for debug set
		{
			BOOL bErrorVersion_MessageBox	= TRUE;
			BOOL bErrorVersion_WriteLog		= TRUE;

			BOOL bErrorFile_MessageBox	= TRUE;
			BOOL bErrorFile_WriteLog	= TRUE;
			BOOL bErrorFile_Remember	= TRUE;
			BOOL bErrorFile_ClipBoard	= TRUE;

			BOOL bErrorCode_MessageBox	= TRUE;
			BOOL bErrorCode_WriteLog	= TRUE;

			std::string strErrorVersion_LogFileName = "";
			std::string strErrorFile_LogFileName = "";
			std::string strErrorCode_LogFileName = "";

			cFILE.getflag ( "DEBUGSET", "bErrorVersion_MessageBox", 0, 1, bErrorVersion_MessageBox );
			cFILE.getflag ( "DEBUGSET", "bErrorVersion_WriteLog", 0, 1, bErrorVersion_WriteLog );

			cFILE.getflag ( "DEBUGSET", "bErrorFile_MessageBox", 0, 1, bErrorFile_MessageBox );
			cFILE.getflag ( "DEBUGSET", "bErrorFile_WriteLog", 0, 1, bErrorFile_WriteLog );
			cFILE.getflag ( "DEBUGSET", "bErrorFile_Remember", 0, 1, bErrorFile_Remember );
			cFILE.getflag ( "DEBUGSET", "bErrorFile_ClipBoard", 0, 1, bErrorFile_ClipBoard );

			cFILE.getflag ( "DEBUGSET", "bErrorCode_MessageBox", 0, 1, bErrorCode_MessageBox );
			cFILE.getflag ( "DEBUGSET", "bErrorCode_WriteLog", 0, 1, bErrorCode_WriteLog );

			cFILE.getflag ( "DEBUGSET", "strErrorVersion_LogFileName", 0, 1, strErrorVersion_LogFileName );
			cFILE.getflag ( "DEBUGSET", "strErrorFile_LogFileName", 0, 1, strErrorFile_LogFileName );
			cFILE.getflag ( "DEBUGSET", "strErrorCode_LogFileName", 0, 1, strErrorCode_LogFileName );

			CDebugSet::SetErrorParam_Version( bErrorVersion_MessageBox, bErrorVersion_WriteLog, strErrorVersion_LogFileName );
			CDebugSet::SetErrorParam_File( bErrorFile_MessageBox, bErrorFile_WriteLog, bErrorFile_Remember, bErrorFile_ClipBoard, strErrorFile_LogFileName );
			CDebugSet::SetErrorParam_Code( bErrorCode_MessageBox, bErrorCode_WriteLog, strErrorCode_LogFileName );
		}


		return TRUE;
	}

};

