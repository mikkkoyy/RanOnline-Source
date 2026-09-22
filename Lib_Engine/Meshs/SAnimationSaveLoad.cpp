#include "stdafx.h"
#include "SAnimation.h"

#include "../Common/SerialFile.h"
#include "../Common/StringUtils.h"
#include "../Common/stlfunctions.h"

#include "DxSkinAniMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HRESULT SAnimation::LoadFromFile ( CSerialFile &SFile, const DxSkeleton* pSkeleton )
{
	GASSERT(pSkeleton&&"SAnimation::LoadFromFile()");

	DWORD dwVER(0);
	SFile >> dwVER;
	HRESULT hr;

	if ( dwVER>VERSION )	{	Load_0103( SFile, pSkeleton ); }
	else if ( 0x0103==dwVER )	Load_0103(SFile, pSkeleton );
	else if ( 0x0102==dwVER )	Load_0102( SFile, pSkeleton );
	else if ( 0x0101==dwVER )	Load_0101( SFile, pSkeleton );
	else if ( 0x0100==dwVER )	Load_0100( SFile, pSkeleton );
	else
	{
		CDebugSet::ErrorVersion( "SAnimation::LoadFromFile", SFile.GetFileName(), dwVER );
		return E_FAIL;
	}

	return TRUE;
}

HRESULT SAnimation::Load_0100( CSerialFile &SFile, const DxSkeleton* pSkeleton )
{
	SFile >> m_cPositionKeys;
	if ( m_cPositionKeys>0 )
	{
		m_pPositionKeys = new SPositionKey[m_cPositionKeys];
		SFile.ReadBuffer ( m_pPositionKeys, sizeof(SPositionKey)*m_cPositionKeys );
	}

	SFile >> m_cRotateKeys;
	if ( m_cRotateKeys>0 )
	{
		m_pRotateKeys = new SRotateKey[m_cRotateKeys];
		SFile.ReadBuffer ( m_pRotateKeys, sizeof(SRotateKey)*m_cRotateKeys );
	}

	SFile >> m_cScaleKeys;
	if ( m_cScaleKeys>0 )
	{
		m_pScaleKeys = new SScaleKey[m_cScaleKeys];
		SFile.ReadBuffer ( m_pScaleKeys, sizeof(SScaleKey)*m_cScaleKeys );
	}

	SFile >> m_cMatrixKeys;
	if ( m_cMatrixKeys>0 )
	{
		m_pMatrixKeys = new SMatrixKey[m_cMatrixKeys];
		SFile.ReadBuffer ( m_pMatrixKeys, sizeof(SMatrixKey)*m_cMatrixKeys );

		// Optimize 
		m_fDivFullTime = 1.f/m_pMatrixKeys[m_cMatrixKeys-1].dwTime;
		m_fMatrixKeys = (float)m_cMatrixKeys;
	}

	SFile >> m_strBone;

	//	적용될 BONE 과 연결.
	if ( pSkeleton )
		pBoneToAnimate = pSkeleton->FindBone ( m_strBone.c_str() );

	// Convert Quaternion
	ConvertMatToQuat();

	return S_OK;
}

HRESULT SAnimation::Load_0101( CSerialFile &SFile, const DxSkeleton* pSkeleton )
{
	SFile >> m_cPositionKeys;
	if ( m_cPositionKeys>0 )
	{
		m_pPositionKeys = new SPositionKey[m_cPositionKeys];
		SFile.ReadBuffer ( m_pPositionKeys, sizeof(SPositionKey)*m_cPositionKeys );
	}

	SFile >> m_cRotateKeys;
	if ( m_cRotateKeys>0 )
	{
		m_pRotateKeys = new SRotateKey[m_cRotateKeys];
		SFile.ReadBuffer ( m_pRotateKeys, sizeof(SRotateKey)*m_cRotateKeys );
	}

	SFile >> m_cScaleKeys;
	if ( m_cScaleKeys>0 )
	{
		m_pScaleKeys = new SScaleKey[m_cScaleKeys];
		SFile.ReadBuffer ( m_pScaleKeys, sizeof(SScaleKey)*m_cScaleKeys );
	}

	SFile >> m_cMatrixKeys;
	if ( m_cMatrixKeys>0 )
	{
		m_pMatrixKeys = new SMatrixKey[m_cMatrixKeys];
		SFile.ReadBuffer ( m_pMatrixKeys, sizeof(SMatrixKey)*m_cMatrixKeys );

		// Optimize 
		m_fDivFullTime = 1.f/m_pMatrixKeys[m_cMatrixKeys-1].dwTime;
		m_fMatrixKeys = (float)m_cMatrixKeys;
	}

	SFile >> m_cQuatPosKeys;
	if ( m_cQuatPosKeys>0 )
	{
		m_pQuatPosKeys = new SQuatPosKey[m_cQuatPosKeys];
		SFile.ReadBuffer ( m_pQuatPosKeys, sizeof(SQuatPosKey)*m_cQuatPosKeys );
	}

	SFile >> m_strBone;

	//	적용될 BONE 과 연결.
	if ( pSkeleton )
		pBoneToAnimate = pSkeleton->FindBone ( m_strBone.c_str() );

	// Convert Quaternion
	ConvertMatToQuat();

	return S_OK;
}

HRESULT SAnimation::Load_0102( CSerialFile &SFile, const DxSkeleton* pSkeleton )
{

	SFile >> m_strBone;

	SFile >> m_cRotateKeys;
	if ( m_cRotateKeys>0 )
	{
		m_pRotateKeys = new SRotateKey[m_cRotateKeys];
		SFile.ReadBuffer ( m_pRotateKeys, sizeof(SRotateKey)*m_cRotateKeys );
	}

	SFile >> m_cPositionKeys;
	if ( m_cPositionKeys>0 )
	{
		m_pPositionKeys = new SPositionKey[m_cPositionKeys];
		SFile.ReadBuffer ( m_pPositionKeys, sizeof(SPositionKey)*m_cPositionKeys );
	}

	DWORD dwGarbageValue;
	SFile >> dwGarbageValue;

	SFile >> m_cQuatPosKeys;
	if ( m_cQuatPosKeys>0 )
	{
		m_pQuatPosKeys = new SQuatPosKey[m_cQuatPosKeys];
		SFile.ReadBuffer ( m_pQuatPosKeys, sizeof(SQuatPosKey)*m_cQuatPosKeys );
	}

	SFile >> dwGarbageValue;

	SFile >> m_cMatrixKeys;
	if ( m_cMatrixKeys>0 )
	{
		m_pMatrixKeys = new SMatrixKey[m_cMatrixKeys];
		SFile.ReadBuffer ( m_pMatrixKeys, sizeof(SMatrixKey)*m_cMatrixKeys );

		// Optimize 
		m_fDivFullTime = 1.f/m_pMatrixKeys[m_cMatrixKeys-1].dwTime;
		m_fMatrixKeys = (float)m_cMatrixKeys;
	}

	SFile >> m_cScaleKeys;
	if ( m_cScaleKeys>0 )
	{
		m_pScaleKeys = new SScaleKey[m_cScaleKeys];
		SFile.ReadBuffer ( m_pScaleKeys, sizeof(SScaleKey)*m_cScaleKeys );
	}

	//	적용될 BONE 과 연결.
	if ( pSkeleton )
		pBoneToAnimate = pSkeleton->FindBone ( m_strBone.c_str() );

	// Convert Quaternion
	ConvertMatToQuat();

	return S_OK;
}

HRESULT SAnimation::Load_0103( CSerialFile &SFile, const DxSkeleton* pSkeleton )
{
	SFile >> m_strBone;

	SFile >> m_cRotateKeys;
	if ( m_cRotateKeys>0 )
	{
		m_pRotateKeys = new SRotateKey[m_cRotateKeys];
		SFile.ReadBuffer ( m_pRotateKeys, sizeof(SRotateKey)*m_cRotateKeys );
	}

	DWORD dwGarbageValue;
	SFile >> dwGarbageValue;

	SFile >> m_cPositionKeys;
	if ( m_cPositionKeys>0 )
	{
		m_pPositionKeys = new SPositionKey[m_cPositionKeys];
		SFile.ReadBuffer ( m_pPositionKeys, sizeof(SPositionKey)*m_cPositionKeys );
	}

	SFile >> m_cQuatPosKeys;
	if ( m_cQuatPosKeys>0 )
	{
		m_pQuatPosKeys = new SQuatPosKey[m_cQuatPosKeys];
		SFile.ReadBuffer ( m_pQuatPosKeys, sizeof(SQuatPosKey)*m_cQuatPosKeys );
	}

	SFile >> m_cMatrixKeys;
	if ( m_cMatrixKeys>0 )
	{
		m_pMatrixKeys = new SMatrixKey[m_cMatrixKeys];
		SFile.ReadBuffer ( m_pMatrixKeys, sizeof(SMatrixKey)*m_cMatrixKeys );

		// Optimize 
		m_fDivFullTime = 1.f/m_pMatrixKeys[m_cMatrixKeys-1].dwTime;
		m_fMatrixKeys = (float)m_cMatrixKeys;
	}

	SFile >> dwGarbageValue;

	SFile >> m_cScaleKeys;
	if ( m_cScaleKeys>0 )
	{
		m_pScaleKeys = new SScaleKey[m_cScaleKeys];
		SFile.ReadBuffer ( m_pScaleKeys, sizeof(SScaleKey)*m_cScaleKeys );
	}

	//	적용될 BONE 과 연결.
	if ( pSkeleton )
		pBoneToAnimate = pSkeleton->FindBone ( m_strBone.c_str() );

	// Convert Quaternion
	ConvertMatToQuat();

	return S_OK;
}

HRESULT SAnimation::SaveToFile ( CSerialFile &SFile )
{
	SFile << (DWORD)VERSION;

	SFile << m_strBone;

	SFile << m_cRotateKeys;
	if ( m_cRotateKeys>0 )
	{
		SFile.WriteBuffer ( m_pRotateKeys, sizeof(SRotateKey)*m_cRotateKeys );
	}

	DWORD dwGarbageData = m_strBone.length();
	SFile << dwGarbageData;

	SFile << m_cPositionKeys;
	if ( m_cPositionKeys>0 )
	{
		SFile.WriteBuffer ( m_pPositionKeys, sizeof(SPositionKey)*m_cPositionKeys );
	}

	SFile << m_cQuatPosKeys;
	if ( m_cQuatPosKeys>0 )
	{
		SFile.WriteBuffer ( m_pQuatPosKeys, sizeof(SQuatPosKey)*m_cQuatPosKeys );
	}

	SFile << m_cMatrixKeys;
	if ( m_cMatrixKeys>0 )
	{
		SFile.WriteBuffer ( m_pMatrixKeys, sizeof(SMatrixKey)*m_cMatrixKeys );
	}

	dwGarbageData = m_strBone.length() + VERSION;
	SFile << dwGarbageData;

	SFile << m_cScaleKeys;
	if ( m_cScaleKeys>0 )
	{
		SFile.WriteBuffer ( m_pScaleKeys, sizeof(SScaleKey)*m_cScaleKeys );
	}	

	return S_OK;
}


HRESULT SAnimContainer::LoadAnimFromFile ( const TSTRING &strFilePath )
{
	CSerialFile SFile;
	if ( !SFile.OpenFile ( FOT_READ, strFilePath.c_str() ) )		
	{
		if ( strlen( strFilePath.c_str() ) > 0 )
			CDebugSet::ErrorFile( "SAnimContainer::LoadAnimFromFile", strFilePath.c_str() );

		return E_FAIL;
	}

	TCHAR szFileType[FILETYPESIZE] = "";
	DWORD dwVersion;
	SFile.GetFileType( szFileType, FILETYPESIZE, dwVersion );

	std::string str (strFilePath);
	std::string str2 ("v2-1");

	std::size_t found = str.find(str2);
	if (found!=std::string::npos)
	{
		SFile.SetEncodeType( EMBYTECRYPT_BIN );
		switch ( dwVersion )
		{
		case 0x0100:		 Load_0100( SFile );	break;
		case 0x0101:		 Load_0101( SFile );	break;
		case 0x0102:		 Load_0102( SFile );	break;
		case 0x0103:		 Load_0103( SFile );	break;
		case 0x0104:		
		case VERSION:		 Load_0104( SFile );	break;
		default:			
			{
				CDebugSet::ErrorVersion( "SAnimContainer::LoadAnimFromFile", SFile.GetFileName(), dwVersion );
				return E_FAIL;
			}break;
		}
	}
	else
	{

		if ( dwVersion >= VERSION_ENCODE )
			SFile.SetEncodeType( EMBYTECRYPT_BIN2 );

		HRESULT hr;
		switch ( dwVersion )
		{
		case 0x0100:		Load_0100( SFile );	break;
		case 0x0101:		Load_0101( SFile );	break;
		case 0x0102:		Load_0102( SFile );	break;
		case 0x0103:		Load_0103( SFile );	break;
		case 0x0104:		Load_0104( SFile );	break;
		case VERSION:		Load_0200( SFile );	break;
		default:			
			{
				CDebugSet::ErrorVersion( "SAnimContainer::LoadAnimFromFile", SFile.GetFileName(), dwVersion );
				return E_FAIL;
			}break;
		}
	}

	/*if ( FAILED(hr) )
	{
		CSerialFile SFile;
		if ( !SFile.OpenFile ( FOT_READ, strFilePath.c_str() ) )		
		{
			if ( strlen( strFilePath.c_str() ) > 0 )
			CDebugSet::ErrorFile( "SAnimContainer::LoadAnimFromFile", strFilePath.c_str() );

			return E_FAIL;
		}

		TCHAR szFileType[FILETYPESIZE] = "";
		DWORD dwVersion;
		SFile.GetFileType( szFileType, FILETYPESIZE, dwVersion );

		SFile.SetEncodeType( EMBYTECRYPT_BIN2 );

		switch ( dwVersion )
		{
			case 0x0100:		Load_0100( SFile );	break;
			case 0x0101:		Load_0101( SFile );	break;
			case 0x0102:		Load_0102( SFile );	break;
			case 0x0103:		Load_0103( SFile );	break;
			case 0x0104:		Load_0104( SFile );	break;
			case VERSION:		Load_0200( SFile );	break;
			default:			
			{
				CDebugSet::ErrorVersion( "SAnimContainer::LoadAnimFromFile", SFile.GetFileName(), dwVersion );
				return E_FAIL;
			}break;
		}
	}*/

	return S_OK;
}

HRESULT  SAnimContainer::Load_0100( CSerialFile &SFile )
{
	std::for_each(m_listAnimation.begin(),m_listAnimation.end(),std_afunc::DeleteObject());
	std::for_each(m_listAniUPBODY.begin(),m_listAniUPBODY.end(),std_afunc::DeleteObject());
	m_listAnimation.clear();
	m_listAniUPBODY.clear();

	DWORD dwNumAni(0);
	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, m_pSkeleton );
		m_listAnimation.push_back ( pAnimation );
	}

	m_bVALID = true;

	return S_OK;
}

HRESULT  SAnimContainer::Load_0101( CSerialFile &SFile )
{
	std::for_each(m_listAnimation.begin(),m_listAnimation.end(),std_afunc::DeleteObject());
	std::for_each(m_listAniUPBODY.begin(),m_listAniUPBODY.end(),std_afunc::DeleteObject());
	m_listAnimation.clear();
	m_listAniUPBODY.clear();

	DWORD dwNumAni(0);
	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, m_pSkeleton );
		m_listAnimation.push_back ( pAnimation );
	}

	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, m_pSkeleton );
		m_listAniUPBODY.push_back ( pAnimation );
	}

	m_bVALID = true;

	return S_OK;
}

HRESULT  SAnimContainer::Load_0102( CSerialFile &SFile )
{
	std::for_each(m_listAniUPBODY.begin(),m_listAniUPBODY.end(),std_afunc::DeleteObject());
	std::for_each(m_listAnimation.begin(),m_listAnimation.end(),std_afunc::DeleteObject());
	m_listAniUPBODY.clear();
	m_listAnimation.clear();

	DWORD dwNumAni(0);
	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, m_pSkeleton );
		m_listAniUPBODY.push_back ( pAnimation );
	}

	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, m_pSkeleton );
		m_listAnimation.push_back ( pAnimation );
	}

	m_bVALID = true;

	return S_OK;
}


HRESULT  SAnimContainer::Load_0103( CSerialFile &SFile )
{
	std::for_each(m_listAniUPBODY.begin(),m_listAniUPBODY.end(),std_afunc::DeleteObject());
	std::for_each(m_listAnimation.begin(),m_listAnimation.end(),std_afunc::DeleteObject());
	m_listAniUPBODY.clear();
	m_listAnimation.clear();

	DWORD dwNumAni(0);
	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, m_pSkeleton );
		m_listAniUPBODY.push_back ( pAnimation );
	}

	DWORD dwGarbageValue(0);
	SFile >> dwGarbageValue;

	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, m_pSkeleton );
		m_listAnimation.push_back ( pAnimation );
	}

	m_bVALID = true;

	return S_OK;
}

HRESULT  SAnimContainer::Load_0104( CSerialFile &SFile )
{
	std::for_each(m_listAniUPBODY.begin(),m_listAniUPBODY.end(),std_afunc::DeleteObject());
	std::for_each(m_listAnimation.begin(),m_listAnimation.end(),std_afunc::DeleteObject());
	m_listAniUPBODY.clear();
	m_listAnimation.clear();

	DWORD dwNumAni(0);
	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, m_pSkeleton );
		m_listAnimation.push_back ( pAnimation );
	}

	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, m_pSkeleton );
		m_listAniUPBODY.push_back ( pAnimation );
	}

	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, m_pSkeleton );
		m_listAniUPBODY.push_back ( pAnimation );
	}

	m_bVALID = true;

	return S_OK;
}

HRESULT  SAnimContainer::Load_0200( CSerialFile &SFile )
{
	std::for_each(m_listAniUPBODY.begin(),m_listAniUPBODY.end(),std_afunc::DeleteObject());
	std::for_each(m_listAnimation.begin(),m_listAnimation.end(),std_afunc::DeleteObject());
	m_listAniUPBODY.clear();
	m_listAnimation.clear();

	HRESULT hr;
	DWORD dwNumAni(0);
	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		hr = pAnimation->LoadFromFile(SFile, m_pSkeleton );
		if ( FAILED(hr) ) return hr;
		m_listAniUPBODY.push_back ( pAnimation );
	}

	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		hr = pAnimation->LoadFromFile(SFile, m_pSkeleton );
		if ( FAILED(hr) ) return hr;
		m_listAnimation.push_back ( pAnimation );
	}

	m_bVALID = true;

	return S_OK;
}

HRESULT SAnimContainer::SaveToFile ()
{
	CString strBinFile;
	STRUTIL::ChangeExt ( m_szName, ".x", strBinFile, ".bin" );

	TSTRING strPath;
	strPath += DxSkinAniMan::GetInstance().GetPath ();
	strPath += strBinFile.GetString();

	CSerialFile SFile;
	SFile.SetFileType ( szFILETYPE, VERSION );

	if ( !SFile.OpenFile ( FOT_WRITE, strPath.c_str() ) )		return E_FAIL;

	SFile.SetEncodeType( EMBYTECRYPT_BIN );

	SFile << (DWORD)m_listAniUPBODY.size();
	SANILIST_ITER iter = m_listAniUPBODY.begin();
	SANILIST_ITER iter_end = m_listAniUPBODY.end();
	for ( ; iter!=iter_end; ++iter )
	{
		(*iter)->SaveToFile(SFile);
	}

	SFile << (DWORD)m_listAnimation.size();
	iter = m_listAnimation.begin();
	iter_end = m_listAnimation.end();
	for ( ; iter!=iter_end; ++iter )
	{
		(*iter)->SaveToFile(SFile);
	}

	return S_OK;
}

HRESULT SAnimContainer::LoadAnimFromFile ( const TSTRING &strFilePath, const DxSkeleton* pSkeleton, SANILIST &listANI, SANILIST &listANIUPBODY )
{
	CSerialFile SFile;
	if( strFilePath.empty() )										
		return E_FAIL;

	if ( !SFile.OpenFile ( FOT_READ, strFilePath.c_str() ) )		
	{
		if ( strlen( strFilePath.c_str() ) > 0 )
			CDebugSet::ErrorFile( "SAnimContainer::LoadAnimFromFile2", strFilePath.c_str() );

		return E_FAIL;
	}

	TCHAR szFileType[FILETYPESIZE] = _T("");
	DWORD dwVersion;
	SFile.GetFileType( szFileType, FILETYPESIZE, dwVersion );

	std::string str (strFilePath);
	std::string str2 ("v2-1");

	std::size_t found = str.find(str2);
	if (found!=std::string::npos)
	{
		SFile.SetEncodeType( EMBYTECRYPT_BIN );
		switch ( dwVersion )
		{
		case 0x0100:		 Load_0100( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		case 0x0101:		 Load_0101( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		case 0x0102:		 Load_0102( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		case 0x0103:		 Load_0103( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		case 0x0104:		
		case VERSION:		 Load_0104( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		default:			
			{
				CDebugSet::ErrorVersion( "SAnimContainer::LoadAnimFromFile2", SFile.GetFileName(), dwVersion );
				return E_FAIL;
			}break;
		}
	}
	else
	{
		if ( dwVersion >= VERSION_ENCODE )
			SFile.SetEncodeType( EMBYTECRYPT_BIN2 );

		HRESULT hr;

		switch ( dwVersion )
		{
		case 0x0100:		Load_0100( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		case 0x0101:		Load_0101( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		case 0x0102:		Load_0102( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		case 0x0103:		Load_0103( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		case 0x0104:		Load_0104( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		case VERSION:		Load_0200( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		default:			
			{
				CDebugSet::ErrorVersion( "SAnimContainer::LoadAnimFromFile2", SFile.GetFileName(), dwVersion );
				return E_FAIL;
			}break;
		}
	}

	/*if ( FAILED(hr) )
	{
		CSerialFile SFile;
		if( strFilePath.empty() )										
			return E_FAIL;

		if ( !SFile.OpenFile ( FOT_READ, strFilePath.c_str() ) )		
		{
			if ( strlen( strFilePath.c_str() ) > 0 )
				CDebugSet::ErrorFile( "SAnimContainer::LoadAnimFromFile2", strFilePath.c_str() );

			return E_FAIL;
		}

		TCHAR szFileType[FILETYPESIZE] = _T("");
		DWORD dwVersion;
		SFile.GetFileType( szFileType, FILETYPESIZE, dwVersion );

		SFile.SetEncodeType( EMBYTECRYPT_BIN2 );

		switch ( dwVersion )
		{
		case 0x0100:		Load_0100( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		case 0x0101:		Load_0101( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		case 0x0102:		Load_0102( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		case 0x0103:		Load_0103( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		case 0x0104:		Load_0104( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		case VERSION:		Load_0200( SFile, pSkeleton, listANI, listANIUPBODY );	break;
		default:			
			{
				CDebugSet::ErrorVersion( "SAnimContainer::LoadAnimFromFile2", SFile.GetFileName(), dwVersion );
				return E_FAIL;
			}break;
		}
	}*/

	return S_OK;
}

HRESULT  SAnimContainer::Load_0100( CSerialFile &SFile, const DxSkeleton* pSkeleton, SANILIST &listANI, SANILIST &listANIUPBODY )
{
	DWORD dwNumAni(0);
	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, pSkeleton );
		listANI.push_back ( pAnimation );
	}

	return S_OK;
}

HRESULT  SAnimContainer::Load_0101( CSerialFile &SFile, const DxSkeleton* pSkeleton, SANILIST &listANI, SANILIST &listANIUPBODY )
{

	DWORD dwNumAni(0);
	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, pSkeleton );
		listANI.push_back ( pAnimation );
	}

	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, pSkeleton );
		listANIUPBODY.push_back ( pAnimation );
	}

	return S_OK;
}

HRESULT  SAnimContainer::Load_0102( CSerialFile &SFile, const DxSkeleton* pSkeleton, SANILIST &listANI, SANILIST &listANIUPBODY )
{
	DWORD dwNumAni(0);

	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, pSkeleton );
		listANIUPBODY.push_back ( pAnimation );
	}

	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, pSkeleton );
		listANI.push_back ( pAnimation );
	}

	return S_OK;
}

HRESULT  SAnimContainer::Load_0103( CSerialFile &SFile, const DxSkeleton* pSkeleton, SANILIST &listANI, SANILIST &listANIUPBODY )
{
	DWORD dwNumAni(0);
	
	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, pSkeleton );
		listANIUPBODY.push_back ( pAnimation );
	}

	DWORD dwGarbageValue(0);
	SFile >> dwGarbageValue;

	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, pSkeleton );
		listANI.push_back ( pAnimation );
	}

	return S_OK;
}

HRESULT  SAnimContainer::Load_0104( CSerialFile &SFile, const DxSkeleton* pSkeleton, SANILIST &listANI, SANILIST &listANIUPBODY )
{
	DWORD dwNumAni(0);

	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, pSkeleton );
		listANI.push_back ( pAnimation );
	}

	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, pSkeleton );
		listANIUPBODY.push_back ( pAnimation );
	}

	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, pSkeleton );
		listANIUPBODY.push_back ( pAnimation );
	}

	return S_OK;
}

HRESULT  SAnimContainer::Load_0200( CSerialFile &SFile, const DxSkeleton* pSkeleton, SANILIST &listANI, SANILIST &listANIUPBODY )
{
	DWORD dwNumAni(0);

	HRESULT hr;

	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, pSkeleton );
		if ( FAILED(hr) ) return hr;
		listANIUPBODY.push_back ( pAnimation );
	}

	SFile >> dwNumAni;
	for ( DWORD i=0; i<dwNumAni; ++i )
	{
		SAnimation *pAnimation = new SAnimation;
		pAnimation->LoadFromFile(SFile, pSkeleton );
		if ( FAILED(hr) ) return hr;
		listANI.push_back ( pAnimation );
	}

	return S_OK;
}