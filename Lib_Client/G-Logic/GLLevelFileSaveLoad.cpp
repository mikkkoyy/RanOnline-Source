#include "stdafx.h"
#include "./GLLevelFile.h"
#include "./GLLandMark.h"

#include "../../Lib_Engine/G-Logic/GLOGIC.h"
#include "../../Lib_Engine/DxOctree/DxLandEff.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL GLLevelFile::SaveCsvFile( std::fstream &SFile, bool bAllSave )
{
	// Save Csv Head
	SFile << "Level File Name" << ",";
	SFile << "Quad Count" << ",";
	SLEVEL_REQUIRE::SaveCsvHead( SFile );
	SFile << std::endl;

	// Save Csv Basic info
	char szTempName[256];
	if( !bAllSave )
		strncpy( szTempName, m_strFileName.c_str(), m_strFileName.size()-4 );
	else 
		strcpy( szTempName, m_strFileName.c_str() );
	SFile << szTempName << ",";
	SFile << (INT)m_sLevelHead.m_eDivision << ",";
	m_sLevelRequire.SaveCsv( SFile );
	SFile << std::endl;

	// Save Csv Gate Head
	DxLandGateMan::SaveCsvHead( SFile );
	SFile << std::endl;

	// Save Csv Gate Data
	m_cLandGateMan.SaveCsv( SFile );
	//	SFile << std::endl;

	return TRUE;
}

BOOL GLLevelFile::LoadCsvFile( const char *szFile )
{
	CStringFile StrFile( 10240 );
	if( !StrFile.Open ( szFile ) )
		return FALSE;

	STRUTIL::ClearSeparator();
	STRUTIL::RegisterSeparator( "," );

	CString strLine;
	CStringArray StrArray;
	int iCsvCur = 0;

	// Skip Csv Basic Head
	StrFile.GetNextLine( strLine );

	// Load Csv Basic Info
	StrFile.GetNextLine( strLine );
	STRUTIL::StringSeparate( strLine, StrArray );

	STRUTIL::InputStrCsv( StrArray[ iCsvCur++ ], m_strFileName );
	m_sLevelHead.m_eDivision = (EMDIVISION)atoi( StrArray[ iCsvCur++ ] );
	m_sLevelRequire.LoadCsv( StrArray );

	// Skip Csv Gate Head
	StrFile.GetNextLine( strLine );

	// Load Csv Gate Data
	m_cLandGateMan.CleanUp();
	while( StrFile.GetNextLine( strLine ) )
	{
		STRUTIL::StringSeparate( strLine, StrArray );
		m_cLandGateMan.LoadCsv( StrArray );
	}
	m_cLandGateMan.BuildTree();

	return TRUE;
}

BOOL GLLevelFile::LoadAllCsvFile( LPDIRECT3DDEVICEQ pd3dDevice )
{
	char szPathName[MAX_PATH] = {0};
	StringCchCopy(szPathName,MAX_PATH,GLOGIC::GetPath ());
	StringCchCat(szPathName,MAX_PATH,"level.csv");

	CStringFile StrFile( 10240 );
	if( !StrFile.Open ( szPathName ) )	return FALSE;

	STRUTIL::ClearSeparator();
	STRUTIL::RegisterSeparator( "," );

	CString strLine;
	CStringArray StrArray;
	std::string strTemp;

	int iCsvCur;

	// First Skip Csv Basic Head
	StrFile.GetNextLine( strLine );

	// Load Csv Basic Head
	while( StrFile.GetNextLine( strLine ) )
	{
		InvalidateDeviceObjects();
		DeleteDeviceObjects();

		m_cLandGateMan.CleanUp ();
		m_MobSchMan.Reset ();
		m_MobSchMan.CleanUp ();

		iCsvCur = 0;

		STRUTIL::StringSeparate( strLine, StrArray );
		STRUTIL::InputStrCsv( StrArray[iCsvCur++], strTemp );

		if( !LoadFile ( strTemp.c_str(), false, pd3dDevice ) )
		{
			strTemp			    += " Load Failed!";
			MessageBox( NULL, strTemp.c_str(), "error", MB_OK );
			continue;
		}

		m_strFileName = strTemp;
		m_sLevelHead.m_eDivision = (EMDIVISION)atoi( StrArray[ iCsvCur++ ] );
		m_sLevelRequire.LoadCsv( StrArray );

		// Skip Csv Gate Head
		StrFile.GetNextLine( strLine );

		// Load Csv Gate Data
		m_cLandGateMan.CleanUp();
		while( StrFile.GetNextLine( strLine ) )
		{
			STRUTIL::StringSeparate( strLine, StrArray );
			STRUTIL::InputStrCsv( StrArray[0], strTemp );	
			if( strTemp == "Level File Name" ) break;

			m_cLandGateMan.LoadCsv( StrArray );
		}

		m_cLandGateMan.BuildTree();

		SaveFile( m_strFileName.c_str() );
	}

	return TRUE;
}

//	Note : 파일 구조가 변경시에는 GLAGLandMan의 LoadFile() 함수의 파일 읽기 구조 또한 검토 하여야 함.
//
BOOL GLLevelFile::LoadFile ( const char *szFile, BOOL bCLIENT, LPDIRECT3DDEVICEQ pd3dDevice )
{
	GASSERT(szFile);

	char szPathName[MAX_PATH] = {0};
	StringCchCopy(szPathName,MAX_PATH, GetLevelPath() );
	StringCchCat(szPathName,MAX_PATH,szFile);

	std::auto_ptr<basestream> pBStream(GLOGIC::openfile_basestream(	GLOGIC::bGLOGIC_ZIPFILE, 
										GLOGIC::strLEVEL_ZIPFILE.c_str(),
										szPathName, 
										szFile,
										EMBYTECRYPT_NONE,
										GLOGIC::bGLOGIC_PACKFILE ) );

	if ( !pBStream.get() )								
	{
		if ( strlen( szFile ) > 0 )
			CDebugSet::ErrorFile( "GLLevelFile::LoadFile", szFile );

		return FALSE;
	}

	basestream &SFile = *pBStream;

	m_strFileName = szFile;

	char szFileType[FILETYPESIZE];
	DWORD dwFileVER;
	SFile.GetFileType( szFileType, FILETYPESIZE, dwFileVER );

	if ( dwFileVER>=VERSION_BEFORE_ENCODE )
		SFile.SetEncodeType ( EMBYTECRYPT_OLD );

	if ( dwFileVER>=VERSION_NEW_ENCODE )
		SFile.SetEncodeType ( EMBYTECRYPT_LEVEL );

	m_dwFileVer = dwFileVER;

	BOOL bLOAD(FALSE);
	switch ( dwFileVER )
	{
	case 0x000:
		bLOAD = LOAD_000 ( SFile, bCLIENT );
		break;

	case 0x101:
		bLOAD = LOAD_101 ( SFile, bCLIENT );
		break;

	case 0x102:
		bLOAD = LOAD_102 ( SFile, bCLIENT, pd3dDevice );
		break;

	case 0x103:
		bLOAD = LOAD_103 ( SFile, bCLIENT, pd3dDevice );
		break;

	case 0x104:
		bLOAD = LOAD_104 ( SFile, bCLIENT, pd3dDevice );
		break;

	case 0x105:
		bLOAD = LOAD_105 ( SFile, bCLIENT, pd3dDevice );
		break;

	case 0x106:
		bLOAD = LOAD_106 ( SFile, bCLIENT, pd3dDevice );
		break;

	case 0x107:
		bLOAD = LOAD_107 ( SFile, bCLIENT, pd3dDevice );
		break;

	case VERSION:
		bLOAD = LOAD_200 ( SFile, bCLIENT, pd3dDevice );
		break;

	default:
		CDebugSet::ErrorVersion( "GLLevelFile::LoadFile", dwFileVER );
		return FALSE;
	};

	if ( !bLOAD )
	{
		CDebugSet::MsgBox( "GLLevelFile::LoadFile Failure to Load" );
		return FALSE;
	}

	//	Note : 현제 래벨에 맞는 미니멥 설정 로드.
	//
	m_sLevelAxisInfo.LoadFile ( szFile );

	return TRUE;
}

BOOL GLLevelFile::LOAD_000 ( basestream &SFile, BOOL bCLIENT )
{
	//	Note : 파일 구조 변경전
	//
	DWORD dwVer;
	SFile >> dwVer;
	if ( dwVer!=0x0100 )
	{
		CDebugSet::ErrorVersion( "GLLevelFile::LOAD_000", dwVer );
		return FALSE;
	}

	SLEVEL_HEAD_100 sOldHead;
	SFile.ReadBuffer ( &sOldHead, sizeof(SLEVEL_HEAD_100) );
	m_sLevelHead.Assign ( sOldHead );

	if ( !bCLIENT )
	{
		m_MobSchMan.LoadFile ( SFile );
	}
	/*dmk14 large map window ep9*/
	else m_MobSchManClient.LoadFile( SFile );

	return TRUE;
}

BOOL GLLevelFile::LOAD_101 ( basestream &SFile, BOOL bCLIENT )
{
	m_sLevelHead.LOAD ( SFile );
	m_sLevelRequire.LOAD ( SFile );

	if ( !bCLIENT )
	{
		m_MobSchMan.LoadFile ( SFile );
	}
	/*dmk14 large map window ep9*/
	else m_MobSchManClient.LoadFile( SFile );

	return TRUE;
}

BOOL GLLevelFile::LOAD_102 ( basestream &SFile, BOOL bCLIENT, LPDIRECT3DDEVICEQ pd3dDevice )
{
	m_sLevelHead.LOAD ( SFile );
	m_sLevelRequire.LOAD ( SFile );

	BOOL bLOAD(FALSE);
	SFile >> bLOAD;
	if ( bLOAD )
	{
		while ( bLOAD )
		{
			PLANDEFF pLandEff = new DXLANDEFF;
			pLandEff->Load ( SFile, pd3dDevice );
			AddLandEff ( pLandEff );
			
			SFile >> bLOAD;
		}
	
		BuildSingleEffTree ();
	}

	bLOAD = FALSE;
	SFile >> bLOAD;
	if ( bLOAD )
	{
		while ( bLOAD )
		{
			PLANDMARK pLandMark = new GLLANDMARK;
			pLandMark->LOAD ( SFile );
			AddLandMark ( pLandMark );
			
			SFile >> bLOAD;
		}
	
		BuildLandMarkTree ();
	}

	m_cLandGateMan.Load ( SFile );

	if ( !bCLIENT )
	{
		m_MobSchMan.LoadFile ( SFile );
	}
	/*dmk14 large map window ep9*/
	else m_MobSchManClient.LoadFile( SFile );

	return TRUE;
}

BOOL GLLevelFile::LOAD_103 ( basestream &SFile, BOOL bCLIENT, LPDIRECT3DDEVICEQ pd3dDevice )
{
	DWORD dwBLOCKSIZE(0);

	m_sLevelHead.LOAD ( SFile );
	m_sLevelRequire.LOAD ( SFile );

	BOOL bLOAD(FALSE);
	SFile >> bLOAD;
	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( pd3dDevice )
	{
		if ( bLOAD )
		{
			while ( bLOAD )
			{
				PLANDEFF pLandEff = new DXLANDEFF;
				pLandEff->Load ( SFile, pd3dDevice );
				AddLandEff ( pLandEff );
				
				SFile >> bLOAD;
			}
		
			BuildSingleEffTree ();
		}
	}
	else
	{
		SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	bLOAD = FALSE;
	SFile >> bLOAD;
	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( pd3dDevice )
	{
		if ( bLOAD )
		{
			while ( bLOAD )
			{
				PLANDMARK pLandMark = new GLLANDMARK;
				pLandMark->LOAD ( SFile );
				AddLandMark ( pLandMark );
				
				SFile >> bLOAD;
			}
		
			BuildLandMarkTree ();
		}
	}
	else
	{
		SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	dwBLOCKSIZE = SFile.ReadBlockSize();
	m_cLandGateMan.Load ( SFile );

	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( !bCLIENT )
	{
		m_MobSchMan.LoadFile ( SFile );
	}
	else
	{
		/*dmk14 large map window ep9*/
		m_MobSchManClient.LoadFile( SFile );

		//SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	return TRUE;
}

BOOL GLLevelFile::LOAD_104 ( basestream &SFile, BOOL bCLIENT, LPDIRECT3DDEVICEQ pd3dDevice )
{
	DWORD dwBLOCKSIZE(0);

	m_sLevelHead.LOAD ( SFile );
	m_sLevelRequire.LOAD ( SFile );
	m_sLevelEtcFunc.LOAD ( SFile );

	BOOL bLOAD(FALSE);
	SFile >> bLOAD;
	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( pd3dDevice )
	{
		if ( bLOAD )
		{
			while ( bLOAD )
			{
				PLANDEFF pLandEff = new DXLANDEFF;
				pLandEff->Load ( SFile, pd3dDevice );
				AddLandEff ( pLandEff );

				SFile >> bLOAD;
			}

			BuildSingleEffTree ();
		}
	}
	else
	{
		SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	bLOAD = FALSE;
	SFile >> bLOAD;
	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( pd3dDevice )
	{
		if ( bLOAD )
		{
			while ( bLOAD )
			{
				PLANDMARK pLandMark = new GLLANDMARK;
				pLandMark->LOAD ( SFile );
				AddLandMark ( pLandMark );

				SFile >> bLOAD;
			}

			BuildLandMarkTree ();
		}
	}
	else
	{
		SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	dwBLOCKSIZE = SFile.ReadBlockSize();
	m_cLandGateMan.Load ( SFile );

	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( !bCLIENT )
	{
		m_MobSchMan.LoadFile ( SFile );
	}
	else
	{
		/*dmk14 large map window ep9*/
		m_MobSchManClient.LoadFile( SFile );

		//SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	return TRUE;
}

BOOL GLLevelFile::LOAD_105 ( basestream &SFile, BOOL bCLIENT, LPDIRECT3DDEVICEQ pd3dDevice )
{
	DWORD dwBLOCKSIZE(0);

	m_sLevelHead.LOAD ( SFile );
	m_sLevelRequire.LOAD ( SFile );

	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( !bCLIENT )
	{
		m_MobSchMan.LoadFile ( SFile );
	}else{
		/*dmk14 large map window ep9*/
		m_MobSchManClient.LoadFile( SFile );

		//SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	dwBLOCKSIZE = SFile.ReadBlockSize();
	m_cLandGateMan.Load ( SFile );

	BOOL bLOAD(FALSE);
	SFile >> bLOAD;
	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( pd3dDevice )
	{
		if ( bLOAD )
		{
			while ( bLOAD )
			{
				PLANDEFF pLandEff = new DXLANDEFF;
				pLandEff->Load ( SFile, pd3dDevice );
				AddLandEff ( pLandEff );

				SFile >> bLOAD;
			}

			BuildSingleEffTree ();
		}
	}
	else
	{
		SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	bLOAD = FALSE;
	SFile >> bLOAD;
	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( pd3dDevice )
	{
		if ( bLOAD )
		{
			while ( bLOAD )
			{
				PLANDMARK pLandMark = new GLLANDMARK;
				pLandMark->LOAD ( SFile );
				AddLandMark ( pLandMark );

				SFile >> bLOAD;
			}

			BuildLandMarkTree ();
		}
	}
	else
	{
		SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	m_sLevelEtcFunc.LOAD ( SFile );

	return TRUE;
}

BOOL GLLevelFile::LOAD_106 ( basestream &SFile, BOOL bCLIENT, LPDIRECT3DDEVICEQ pd3dDevice )
{
	DWORD dwBLOCKSIZE(0);

	m_sLevelHead.LOAD ( SFile );
	m_sLevelRequire.LOAD ( SFile );

	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( !bCLIENT )
	{
		m_MobSchMan.LoadFile_106 ( SFile );
	}
	else
	{
		SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	dwBLOCKSIZE = SFile.ReadBlockSize();


	m_cLandGateMan.Load ( SFile );

	BOOL bLOAD(FALSE);
	SFile >> bLOAD;
	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( pd3dDevice )
	{
		if ( bLOAD )
		{
			while ( bLOAD )
			{
				PLANDEFF pLandEff = new DXLANDEFF;
				pLandEff->Load ( SFile, pd3dDevice );
				AddLandEff ( pLandEff );

				SFile >> bLOAD;
			}

			BuildSingleEffTree ();
		}
	}
	else
	{
		SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	bLOAD = FALSE;
	SFile >> bLOAD;
	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( pd3dDevice )
	{
		if ( bLOAD )
		{
			while ( bLOAD )
			{
				PLANDMARK pLandMark = new GLLANDMARK;
				pLandMark->LOAD ( SFile );
				AddLandMark ( pLandMark );

				SFile >> bLOAD;
			}

			BuildLandMarkTree ();
		}
	}
	else
	{
		SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	m_sLevelEtcFunc.LOAD ( SFile );

	return TRUE;
}

BOOL GLLevelFile::LOAD_200 ( basestream &SFile, BOOL bCLIENT, LPDIRECT3DDEVICEQ pd3dDevice )
{
	DWORD dwBLOCKSIZE(0);

	m_sLevelHead.LOAD ( SFile );
	m_sLevelRequire.LOAD ( SFile );

	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( !bCLIENT )
	{
		m_MobSchMan.LoadFile ( SFile );
	}else{
		/*dmk14 large map window ep9*/
		m_MobSchManClient.LoadFile( SFile );

		//SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	dwBLOCKSIZE = SFile.ReadBlockSize();
	m_cLandGateMan.Load ( SFile );

	m_sLevelEtcFunc.LOAD ( SFile );

	BOOL bLOAD(FALSE);
	SFile >> bLOAD;
	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( pd3dDevice )
	{
		if ( bLOAD )
		{
			while ( bLOAD )
			{
				PLANDEFF pLandEff = new DXLANDEFF;
				pLandEff->Load ( SFile, pd3dDevice );
				AddLandEff ( pLandEff );

				SFile >> bLOAD;
			}

			BuildSingleEffTree ();
		}
	}
	else
	{
		SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	bLOAD = FALSE;
	SFile >> bLOAD;
	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( pd3dDevice )
	{
		if ( bLOAD )
		{
			while ( bLOAD )
			{
				PLANDMARK pLandMark = new GLLANDMARK;
				pLandMark->LOAD ( SFile );
				AddLandMark ( pLandMark );

				SFile >> bLOAD;
			}

			BuildLandMarkTree ();
		}
	}
	else
	{
		SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	return TRUE;
}

BOOL GLLevelFile::LOAD_107 ( basestream &SFile, BOOL bCLIENT, LPDIRECT3DDEVICEQ pd3dDevice )
{
	DWORD dwBLOCKSIZE(0);

	m_sLevelHead.LOAD ( SFile );
	m_sLevelRequire.LOAD ( SFile );

	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( !bCLIENT )
	{
		m_MobSchMan.LoadFile_107 ( SFile );
	}
	else
	{
		SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	dwBLOCKSIZE = SFile.ReadBlockSize();

	

	m_cLandGateMan.Load ( SFile );

	

	BOOL bLOAD(FALSE);
	SFile >> bLOAD;
	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( pd3dDevice )
	{
		if ( bLOAD )
		{
			while ( bLOAD )
			{
				PLANDEFF pLandEff = new DXLANDEFF;
				pLandEff->Load ( SFile, pd3dDevice );
				AddLandEff ( pLandEff );

				SFile >> bLOAD;
			}

			BuildSingleEffTree ();
		}
	}
	else
	{
		SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	bLOAD = FALSE;
	SFile >> bLOAD;
	dwBLOCKSIZE = SFile.ReadBlockSize();
	if ( pd3dDevice )
	{
		if ( bLOAD )
		{
			while ( bLOAD )
			{
				PLANDMARK pLandMark = new GLLANDMARK;
				pLandMark->LOAD ( SFile );
				AddLandMark ( pLandMark );

				SFile >> bLOAD;
			}

			BuildLandMarkTree ();
		}
	}
	else
	{
		SFile.SetOffSet ( SFile.GetfTell() + dwBLOCKSIZE );
	}

	m_sLevelEtcFunc.LOAD ( SFile );

	return TRUE;
}

BOOL GLLevelFile::SaveFile ( const char *szFile )
{
	/// 저장하기전에 몹 스케쥴에 리더 그룹이 제대로 설정되어 있는지 체크한다.
	MOBSCHEDULELIST *pMobScheduleList = m_MobSchMan.GetMobSchList();
	MOBSCHEDULENODE *pHead = pMobScheduleList->m_pHead;

	std::map<std::string,BOOL> mapMobGroupInfo;
	while(pHead)
	{		
		GLMobSchedule *pSchdule = pHead->Data;
		if( pSchdule->m_strGroupName != ""  )
		{
			if( mapMobGroupInfo.find( pSchdule->m_strGroupName ) == mapMobGroupInfo.end() )
			{
				mapMobGroupInfo[pSchdule->m_strGroupName] = FALSE;
			}
			if( pSchdule->m_bLeaderMonster )
			{
				mapMobGroupInfo[pSchdule->m_strGroupName] = TRUE;
			}
		}
		pHead = pHead->pNext;
	}

	std::map<std::string,BOOL>::iterator mapMobGroup_Iter = mapMobGroupInfo.begin();
	for( ; mapMobGroup_Iter != mapMobGroupInfo.end(); ++mapMobGroup_Iter )
	{
		if( !mapMobGroup_Iter->second )
		{
			char szTempChar[256] = {0,};
			sprintf( szTempChar, "You must set up leader of the \"%s\" group", mapMobGroup_Iter->first.c_str() );
			MessageBox( NULL, szTempChar, "Warning", MB_OK );
			return FALSE;

		}
	}


	GASSERT(szFile);

	char szPathName[MAX_PATH] = {0};
	StringCchCopy(szPathName,MAX_PATH, GetLevelPath() );
	StringCchCat(szPathName,MAX_PATH,szFile);

	CSerialFile SFile;
	SFile.SetFileType ( FILE_EXT, VERSION );
	if ( !SFile.OpenFile ( FOT_WRITE, szPathName ) )	return FALSE;
	SFile.SetEncodeType ( EMBYTECRYPT_LEVEL );

	m_strFileName = szFile;

	m_sLevelHead.SAVE ( SFile );
	m_sLevelRequire.SAVE ( SFile );

	SFile.BeginBlock();
	{
		m_MobSchMan.SaveFile ( SFile );
	}
	SFile.EndBlock();

	SFile.BeginBlock();
	{
		m_cLandGateMan.Save ( SFile );
	}
	SFile.EndBlock();

	m_sLevelEtcFunc.SAVE ( SFile );

	SFile << BOOL ( m_pLandEffList!=NULL );
	SFile.BeginBlock();
	{
		PLANDEFF pCur = m_pLandEffList;
		while ( pCur )
		{
			pCur->Save ( SFile );

			pCur = pCur->m_pNext;
			SFile << BOOL ( pCur!=NULL );
		}
	}
	SFile.EndBlock();

	SFile << BOOL ( m_pLandMarkList!=NULL );
	SFile.BeginBlock();
	{
		PLANDMARK pCur = m_pLandMarkList;
		while ( pCur )
		{
			pCur->SAVE ( SFile );

			pCur = pCur->m_pNext;
			SFile << BOOL ( pCur!=NULL );
		}
	}
	SFile.EndBlock();

	return TRUE;
}