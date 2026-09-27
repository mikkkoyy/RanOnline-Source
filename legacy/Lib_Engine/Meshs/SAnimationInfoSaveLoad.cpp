#include "stdafx.h"
#include "SAnimationInfo.h"

#include "../Common/SerialFile.h"
#include "../Common/StringUtils.h"
#include "../Common/StlFunctions.h"
#include "../G-Logic/GLogic.h"

#include "DxBoneCollector.h"
#include "DxSkinAniMan.h"
#include "../DxSound/DxSoundMan.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL SANIMCONINFO::LoadFile ( const char *szFileName, LPDIRECT3DDEVICEQ pd3dDevice )
{
	CleanUp();

	char szFilePath[MAX_PATH] = "";
	StringCchCopy( szFilePath, MAX_PATH, DxSkinAniMan::GetInstance().GetPath() );
	StringCchCat( szFilePath, MAX_PATH, szFileName );

	std::auto_ptr<basestream> pBStream( GLOGIC::openfile_basestream(GLOGIC::bENGLIB_ZIPFILE, 
										GLOGIC::strANIMATION_ZIPFILE.c_str(),
										szFilePath, 
										szFileName ) );

	if ( !pBStream.get() )
	{
		if ( strlen( szFileName ) > 0 )
			CDebugSet::ErrorFile( "SANIMCONINFO::LoadFile", szFileName );

		return FALSE;
	}

	basestream &SFile = *pBStream;

	char szFileType[FILETYPESIZE] = {0};
	DWORD dwVersion;
	SFile.GetFileType( szFileType, FILETYPESIZE, dwVersion );

	if ( dwVersion >= VERSION_ENCODE )
		SFile.SetEncodeType( EMBYTECRYPT_CFG );

	switch( dwVersion )
	{
	case SANIMCONINFO::VERSION:	
		LoadFile_0200( SFile, pd3dDevice );	
		break;

	case 0x0115:
	case 0x0114:
	case 0x0112:
	case 0x0111:	
		LoadFile_0111_0112_0114_0115( SFile, pd3dDevice );	
		break;

	case 0x0110:
	case 0x0109:
	case 0x0108:	
		LoadFile_0108_0109_0110( SFile, pd3dDevice );	
		break;

	case 0x0107:
		LoadFile_0107( SFile, pd3dDevice );	
		break;

	case 0x0106:	
		LoadFile_0106( SFile, pd3dDevice );	
		break;

	case 0x0105:	
		LoadFile_0105( SFile, pd3dDevice );	
		break;

	case 0x0104:	
		LoadFile_0104( SFile, pd3dDevice );	
		break;

	case 0x0103:	
		LoadFile_0103( SFile, pd3dDevice );	
		break;

	case 0x0102:	
		LoadFile_0102( SFile, pd3dDevice );	
		break;

	case 0x0101:	
		LoadFile_0101( SFile, pd3dDevice );	
		break;

	default:
		{
			CDebugSet::ErrorVersion( "SANIMCONINFO::LoadFile", SFile.GetFileName(), dwVersion );
			return FALSE;
		}break;
	};

	m_dwFlag = m_dwFlag & ~ACF_NEWINFO;

	return TRUE;
}

void SANIMCONINFO::LoadFile_0101( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	SANIMCONINFO_101 OldAnimInfo;
	SFile.ReadBuffer ( &OldAnimInfo, sizeof(OldAnimInfo) );

	StringCchCopy( m_szName, ACF_SZNAME, OldAnimInfo.m_szName );
	StringCchCopy( m_szSkeletion, ACF_SZNAME, OldAnimInfo.m_szSkeletion );

	m_dwFlag = OldAnimInfo.m_dwFlag;
	m_dwSTime = OldAnimInfo.m_dwSTime;
	m_dwETime = OldAnimInfo.m_dwETime;
	m_dwETimeOrig = OldAnimInfo.m_dwETimeOrig;
	m_UNITTIME = OldAnimInfo.m_UNITTIME;

	m_MainType = OldAnimInfo.m_MainType;
	m_SubType = OldAnimInfo.m_SubType;

	m_wDivCount = OldAnimInfo.m_wDivCount;
	memcpy ( m_wDivFrame, OldAnimInfo.m_wDivFrame, sizeof(WORD)*ACF_DIV );		
}

void SANIMCONINFO::LoadFile_0102( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	SANIMCONINFO_102 OldAnimInfo;
	SFile.ReadBuffer ( &OldAnimInfo, sizeof(OldAnimInfo) );

	StringCchCopy( m_szName, ACF_SZNAME, OldAnimInfo.m_szName );
	StringCchCopy( m_szSkeletion, ACF_SZNAME, OldAnimInfo.m_szSkeletion );

	m_dwFlag = OldAnimInfo.m_dwFlag;
	m_dwSTime = OldAnimInfo.m_dwSTime;
	m_dwETime = OldAnimInfo.m_dwETime;
	m_dwETimeOrig = OldAnimInfo.m_dwETimeOrig;
	m_UNITTIME = OldAnimInfo.m_UNITTIME;

	m_MainType = OldAnimInfo.m_MainType;
	m_SubType = OldAnimInfo.m_SubType;

	m_wDivCount = OldAnimInfo.m_wDivCount;
	memcpy ( m_wDivFrame, OldAnimInfo.m_wDivFrame, sizeof(WORD)*ACF_DIV );

	m_ChaSoundData = OldAnimInfo.m_ChaSoundData;
}

void SANIMCONINFO::LoadFile_0103( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	SANIMCONINFO_103 OldAnimInfo;
	SFile.ReadBuffer ( &OldAnimInfo, sizeof(OldAnimInfo) );

	StringCchCopy( m_szName, ACF_SZNAME, OldAnimInfo.m_szName );
	StringCchCopy( m_szSkeletion, ACF_SZNAME, OldAnimInfo.m_szSkeletion );

	m_dwFlag = OldAnimInfo.m_dwFlag;
	m_dwSTime = OldAnimInfo.m_dwSTime;
	m_dwETime = OldAnimInfo.m_dwETime;
	m_dwETimeOrig = OldAnimInfo.m_dwETimeOrig;
	m_UNITTIME = OldAnimInfo.m_UNITTIME;
	m_MainType = OldAnimInfo.m_MainType;
	m_SubType = OldAnimInfo.m_SubType;

	m_wDivCount = OldAnimInfo.m_wDivCount;
	memcpy ( m_wDivFrame, OldAnimInfo.m_wDivFrame, sizeof(WORD)*ACF_DIV );

	m_wStrikeCount = 0;

	m_ChaSoundData = OldAnimInfo.m_ChaSoundData;
}

void SANIMCONINFO::LoadFile_0104( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	SANIMCONINFO_104 OldAnimInfo;
	SFile.ReadBuffer ( &OldAnimInfo, sizeof(SANIMCONINFO_104) );

	StringCchCopy( m_szName, ACF_SZNAME, OldAnimInfo.m_szName );
	StringCchCopy( m_szSkeletion, ACF_SZNAME, OldAnimInfo.m_szSkeletion );

	m_dwFlag = OldAnimInfo.m_dwFlag;
	m_dwSTime = OldAnimInfo.m_dwSTime;
	m_dwETime = OldAnimInfo.m_dwETime;
	m_dwETimeOrig = OldAnimInfo.m_dwETimeOrig;
	m_UNITTIME = OldAnimInfo.m_UNITTIME;
	m_MainType = OldAnimInfo.m_MainType;
	m_SubType = OldAnimInfo.m_SubType;

	m_wDivCount = OldAnimInfo.m_wDivCount;
	memcpy ( m_wDivFrame, OldAnimInfo.m_wDivFrame, sizeof(WORD)*ACF_DIV );

	m_wStrikeCount = OldAnimInfo.m_wStrikeCount;
	memcpy ( m_sStrikeEff, OldAnimInfo.m_sStrikeEff, sizeof(SANIMSTRIKE)*ACF_STRIKE );

	m_ChaSoundData = OldAnimInfo.m_ChaSoundData;
}

void SANIMCONINFO::LoadFile_0105( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	DWORD dwSize(0);
	DWORD dwType(0);

	SFile.ReadBuffer ( m_szName, sizeof(char)*ACF_SZNAME );
	SFile.ReadBuffer ( m_szSkeletion, sizeof(char)*ACF_SZNAME );

	SFile >> m_dwFlag;
	SFile >> m_dwSTime;
	SFile >> m_dwETime;
	SFile >> m_dwETimeOrig;
	SFile >> m_UNITTIME;

	SFile >> dwType;	m_MainType = (EMANI_MAINTYPE)dwType;
	SFile >> dwType;	m_SubType = (EMANI_SUBTYPE)dwType;

	SFile >> m_wDivCount;
	SFile.ReadBuffer ( m_wDivFrame, sizeof(WORD)*ACF_DIV );

	SFile >> m_wStrikeCount;
	SFile.ReadBuffer ( m_sStrikeEff, sizeof(SANIMSTRIKE)*ACF_STRIKE );

	SChaSoundData_103 ChaSoundData;
	SFile.ReadBuffer ( &ChaSoundData, sizeof(SChaSoundData_103) );
	m_ChaSoundData = ChaSoundData;		

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		DWORD	dwType;
		SFile >> dwType;
		DxEffAniData*	pEff = DxEffAniMan::GetInstance().CreateEffInstance ( dwType );
		if ( pEff )
		{
			pEff->LoadFile ( SFile, pd3dDevice );
			m_listEffAni.push_back ( pEff );
		}
	}
}

void SANIMCONINFO::LoadFile_0106( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	DWORD dwSize(0);
	DWORD dwType(0);

	SFile.ReadBuffer ( m_szName, sizeof(char)*ACF_SZNAME );
	SFile.ReadBuffer ( m_szSkeletion, sizeof(char)*ACF_SZNAME );

	SFile >> m_dwFlag;
	SFile >> m_dwSTime;
	SFile >> m_dwETime;
	SFile >> m_dwETimeOrig;
	SFile >> m_UNITTIME;

	SFile >> dwType;	m_MainType = (EMANI_MAINTYPE)dwType;
	SFile >> dwType;	m_SubType = (EMANI_SUBTYPE)dwType;

	SFile >> m_wDivCount;
	SFile.ReadBuffer ( m_wDivFrame, sizeof(WORD)*ACF_DIV );

	SFile >> m_wStrikeCount;
	SFile.ReadBuffer ( m_sStrikeEff, sizeof(SANIMSTRIKE)*ACF_STRIKE );

	SFile.ReadBuffer ( &m_ChaSoundData, sizeof(SChaSoundData) );

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		DWORD	dwType;
		SFile >> dwType;
		DxEffAniData*	pEff = DxEffAniMan::GetInstance().CreateEffInstance ( dwType );
		if ( pEff )
		{
			pEff->LoadFile ( SFile, pd3dDevice );
			m_listEffAni.push_back ( pEff );
		}
	}
}

void SANIMCONINFO::LoadFile_0107( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	DWORD dwSize(0);
	DWORD dwType(0);

	SFile.ReadBuffer ( m_szName, sizeof(char)*ACF_SZNAME );
	SFile.ReadBuffer ( m_szSkeletion, sizeof(char)*ACF_SZNAME );

	SFile >> m_dwFlag;
	SFile >> m_dwSTime;
	SFile >> m_dwETime;
	SFile >> m_dwETimeOrig;
	SFile >> m_UNITTIME;

	SFile >> dwType;	m_MainType = (EMANI_MAINTYPE)dwType;
	SFile >> dwType;	m_SubType = (EMANI_SUBTYPE)dwType;

	SFile >> m_wDivCount;
	SFile.ReadBuffer ( m_wDivFrame, sizeof(WORD)*ACF_DIV );

	SFile >> m_wStrikeCount;
	SFile.ReadBuffer ( m_sStrikeEff, sizeof(SANIMSTRIKE)*ACF_STRIKE );

	SFile.ReadBuffer ( &m_ChaSoundData, sizeof(SChaSoundData) );

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		DWORD	dwType;
		SFile >> dwType;
		DxEffAniData*	pEff = DxEffAniMan::GetInstance().CreateEffInstance ( dwType );
		if ( pEff )
		{
			pEff->LoadFile ( SFile, pd3dDevice );
			m_listEffAni.push_back ( pEff );
		}
	}

	m_pAniScale->Load( SFile );
}

void SANIMCONINFO::LoadFile_0108_0109_0110( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	DWORD dwSize(0);
	DWORD dwType(0);

	SFile.ReadBuffer ( m_szName, sizeof(char)*ACF_SZNAME );
	SFile.ReadBuffer ( m_szSkeletion, sizeof(char)*ACF_SZNAME );

	SFile >> m_dwFlag;
	SFile >> m_UNITTIME;
	SFile >> m_dwSTime;
	SFile >> m_dwETime;
	SFile >> m_dwETimeOrig;

	SFile >> dwType;	m_MainType = (EMANI_MAINTYPE)dwType;
	SFile >> dwType;	m_SubType = (EMANI_SUBTYPE)dwType;

	SFile >> m_wStrikeCount;
	SFile.ReadBuffer ( m_sStrikeEff, sizeof(SANIMSTRIKE)*ACF_STRIKE );

	SFile >> m_wDivCount;
	SFile.ReadBuffer ( m_wDivFrame, sizeof(WORD)*ACF_DIV );

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		DWORD	dwType;
		SFile >> dwType;
		DxEffAniData*	pEff = DxEffAniMan::GetInstance().CreateEffInstance ( dwType );
		if ( pEff )
		{
			pEff->LoadFile ( SFile, pd3dDevice );
			m_listEffAni.push_back ( pEff );
		}
	}

	m_pAniScale->Load( SFile );

	SFile.ReadBuffer ( &m_ChaSoundData, sizeof(SChaSoundData) );	
}

void SANIMCONINFO::LoadFile_0111_0112_0114_0115( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	DWORD dwSize(0);
	DWORD dwType(0);

	SFile >> dwSize;
	char *szCfgFile = new char[dwSize];
	SFile.ReadBuffer ( szCfgFile, sizeof(char)*dwSize );

	SFile >> dwSize;
	char *szXFile = new char[dwSize];
	SFile.ReadBuffer ( szXFile, sizeof(char)*dwSize );

	SFile >> dwSize;
	char *szBinFile = new char[dwSize];
	SFile.ReadBuffer ( szBinFile, sizeof(char)*dwSize );

	SFile >> dwSize;
	char *szSkeleton = new char[dwSize];
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*dwSize );

	StringCchCopy( m_szName, ACF_SZNAME, szXFile );
	StringCchCopy( m_szSkeletion, ACF_SZNAME, szSkeleton );

	SAFE_DELETE_ARRAY(szCfgFile);
	SAFE_DELETE_ARRAY(szXFile);
	SAFE_DELETE_ARRAY(szBinFile);
	SAFE_DELETE_ARRAY(szSkeleton);

	SFile >> m_dwFlag;
	SFile >> m_dwSTime;
	SFile >> m_dwETime;
	SFile >> m_dwETimeOrig;
	SFile >> m_UNITTIME;

	SFile >> dwType;	m_MainType = (EMANI_MAINTYPE)dwType;
	SFile >> dwType;	m_SubType = (EMANI_SUBTYPE)dwType;

	SFile >> m_wDivCount;
	SFile.ReadBuffer ( m_wDivFrame, sizeof(WORD)*ACF_DIV );

	SFile >> m_wStrikeCount;
	SFile.ReadBuffer ( m_sStrikeEff, sizeof(SANIMSTRIKE)*ACF_STRIKE );

	SFile.ReadBuffer ( &m_ChaSoundData, sizeof(SChaSoundData) );

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		DWORD	dwType;
		SFile >> dwType;
		DxEffAniData*	pEff = DxEffAniMan::GetInstance().CreateEffInstance ( dwType );
		if ( pEff )
		{
			pEff->LoadFile ( SFile, pd3dDevice );
			m_listEffAni.push_back ( pEff );
		}
	}

	m_pAniScale->Load( SFile );
}

void SANIMCONINFO::LoadFile_0200( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	DWORD dwSize(0);
	DWORD dwType(0);

	SFile.ReadBuffer ( m_szName, sizeof(char)*ACF_SZNAME );
	SFile.ReadBuffer ( m_szSkeletion, sizeof(char)*ACF_SZNAME );

	SFile >> m_dwFlag;
	SFile >> m_UNITTIME;
	SFile >> m_dwSTime;
	SFile >> m_dwETime;
	SFile >> m_dwETimeOrig;
	
	SFile >> dwType;	m_MainType = (EMANI_MAINTYPE)dwType;
	SFile >> dwType;	m_SubType = (EMANI_SUBTYPE)dwType;

	SFile >> m_wStrikeCount;
	SFile.ReadBuffer ( m_sStrikeEff, sizeof(SANIMSTRIKE)*ACF_STRIKE );

	SFile >> m_wDivCount;
	SFile.ReadBuffer ( m_wDivFrame, sizeof(WORD)*ACF_DIV );

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		DWORD	dwType;
		SFile >> dwType;
		DxEffAniData*	pEff = DxEffAniMan::GetInstance().CreateEffInstance ( dwType );
		if ( pEff )
		{
			pEff->LoadFile ( SFile, pd3dDevice );
			m_listEffAni.push_back ( pEff );
		}
	}

	m_pAniScale->Load( SFile );

	SFile.ReadBuffer ( &m_ChaSoundData, sizeof(SChaSoundData) );
}

BOOL SANIMCONINFO::SaveFile ( const char *szFileName )
{
	char szFilePath[MAX_PATH] = "";
	StringCchCopy( szFilePath, MAX_PATH, DxSkinAniMan::GetInstance().GetPath() );
	StringCchCat( szFilePath, MAX_PATH, szFileName );

	CSerialFile SFile;
	SFile.SetFileType ( "SANIMCONINFO", SANIMCONINFO::VERSION );
	if ( !SFile.OpenFile ( FOT_WRITE, szFilePath ) )	return FALSE;

	SFile.SetEncodeType( EMBYTECRYPT_CFG );

	SFile.WriteBuffer ( m_szName, sizeof(char)*ACF_SZNAME );
	SFile.WriteBuffer ( m_szSkeletion, sizeof(char)*ACF_SZNAME );

	SFile << m_dwFlag;
	SFile << m_UNITTIME;
	SFile << m_dwSTime;
	SFile << m_dwETime;
	SFile << m_dwETimeOrig;
	
	SFile << m_MainType;
	SFile << m_SubType;

	SFile << m_wStrikeCount;
	SFile.WriteBuffer ( m_sStrikeEff, sizeof(SANIMSTRIKE)*ACF_STRIKE );

	SFile << m_wDivCount;
	SFile.WriteBuffer ( m_wDivFrame, sizeof(WORD)*ACF_DIV );

	SFile << (DWORD) m_listEffAni.size();
	EFFANILIST_ITER iter = m_listEffAni.begin();
	EFFANILIST_ITER iter_end = m_listEffAni.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SFile << (DWORD) (*iter)->GetTypeID();
		(*iter)->SaveFile ( SFile );
	}

	m_pAniScale->Save( SFile );

	SFile.WriteBuffer ( &m_ChaSoundData, sizeof(SChaSoundData) );

	return TRUE;
}