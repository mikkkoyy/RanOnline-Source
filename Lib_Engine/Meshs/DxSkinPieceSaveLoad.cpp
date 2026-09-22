#include "stdafx.h"
#include "DxSkinPieceContainer.h"

#include "../G-Logic/GLogic.h"
#include "../DxEffect/Char/DxEffChar.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


HRESULT DxSkinPiece::LoadPiece( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr;

	if( !szFile )	return E_FAIL;

	SAFE_DELETE_ARRAY(m_szFileName);
	int nStrLen = strlen(szFile)+1;
	m_szFileName = new char[nStrLen];
	StringCchCopy( m_szFileName, nStrLen, szFile );

	char szPathName[MAX_PATH] = {0};
	StringCchCopy( szPathName, MAX_PATH, DxSkinPieceContainer::GetInstance().GetPath() );
	StringCchCat( szPathName, MAX_PATH, m_szFileName );

	std::auto_ptr<basestream> pBStream(	GLOGIC::openfile_basestream(GLOGIC::bENGLIB_ZIPFILE, 
										GLOGIC::strSKINOBJ_ZIPFILE.c_str(), 
										szPathName, 
										szFile ) );

	if ( !pBStream.get() )
	{
		if ( strlen( szFile ) > 0 )
			CDebugSet::ErrorFile( "DxSkinPiece::LoadPiece", szFile );

		return E_FAIL;
	}

	basestream &SFile = *pBStream;

	DWORD dwVer;
	SFile >> dwVer;

	if ( dwVer >= VERSION_ENCRYPT )
		SFile.SetEncodeType( EMBYTECRYPT_PIECE );

	switch ( dwVer )
	{
	case VERSION_CPS:
		hr = LoadPiece_0201( SFile, pd3dDevice, bThread );
		break;

	case 0x0200:
		hr = LoadPiece_0200( SFile, pd3dDevice, bThread );
		break;

	case 0x0116:
		hr = LoadPiece_0116( SFile, pd3dDevice, bThread );
		break;

	case 0x0115:
	case 0x0114:
		hr = LoadPiece_0114_0115( SFile, pd3dDevice, bThread );
		break;

	case 0x0113:
	case 0x0112:
		hr = LoadPiece_0112_0113( SFile, pd3dDevice, bThread );
		break;

	case 0x0110:
		hr = LoadPiece_0110( SFile, pd3dDevice, bThread );
		break;

	case 0x0109:
		hr = LoadPiece_0109( SFile, pd3dDevice, bThread );
		break;

	case 0x0108:
		hr = LoadPiece_0108( SFile, pd3dDevice, bThread );
		break;

	case 0x0107:
		hr = LoadPiece_0107( SFile, pd3dDevice, bThread );
		break;

	case 0x0106:
		hr = LoadPiece_0106( SFile, pd3dDevice, bThread );
		break;

	case 0x0105:
		hr = LoadPiece_0105( SFile, pd3dDevice, bThread );
		break;

	case 0x0104:
		hr = LoadPiece_0104( SFile, pd3dDevice, bThread );
		break;

	case 0x0103:
		hr = LoadPiece_0103( SFile, pd3dDevice, bThread );
		break;

	case 0x0102:
		hr = LoadPiece_0102( SFile, pd3dDevice, bThread );
		break;

	case 0x0101:
		hr = LoadPiece_0101( SFile, pd3dDevice, bThread );
		break;

	default:
		{
			CDebugSet::ErrorVersion ( "DxSkinPiece::LoadPiece", SFile.GetFileName(), dwVer );
			return E_FAIL;
		}break;
	};

	if ( FAILED(hr) )	
	{
		CDebugSet::ErrorCode( "DxSkinPiece::LoadPiece", "Error Load SkinPiece :%s", m_szFileName );
		return hr;
	}

	if ( m_dwMaterialNum!=m_pmcMesh->GetNumMaterials() )
	{
		m_dwMaterialNum = m_pmcMesh->GetNumMaterials();

		SAFE_DELETE_ARRAY(m_pMaterialPiece);
		m_pMaterialPiece = new SMATERIAL_PIECE[m_dwMaterialNum];
	}

	// Note : 이제부터는 정점 갯수를 가지고 있을 것이다.
	m_dwVertexNUM = m_pmcMesh->m_dwVertexNUM;

	// Note : Cartoon Mesh를 생성한다.
	m_pSkinMesh->Load( m_szXFileName, pd3dDevice );

	return S_OK;
}


HRESULT DxSkinPiece::LoadPiece_0101 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr = S_OK;

	SFile >> m_dwRef;

	DWORD dwSize;

	SFile >> dwSize;
	char *szXFileName = new char[dwSize];
	SFile.ReadBuffer ( szXFileName, sizeof(char)*dwSize );

	SFile >> dwSize;
	char *szSkeleton = new char[dwSize];
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*dwSize );

	//	Note : 스킨 메쉬 읽기.
	//
	hr = SetSkinMesh ( szXFileName, szSkeleton, pd3dDevice, bThread );
	SAFE_DELETE_ARRAY(szXFileName);
	SAFE_DELETE_ARRAY(szSkeleton);
	if ( FAILED(hr) )	return hr;

	SFile >> dwSize;
	//m_emType = (EMPIECECHAR) dwSize;
	EMPIECECHAR_0113 emPIECEOLD = (EMPIECECHAR_0113) dwSize;
	m_emType = PIECE_0113_TO_PIECE( emPIECEOLD );

	SFile >> dwSize;
	m_szMeshName = new char[dwSize];
	SFile.ReadBuffer ( m_szMeshName, sizeof(char)*dwSize );

	m_pmcMesh = m_pSkinMesh->FindMeshContainer ( m_szMeshName );
	if ( !m_pmcMesh )	return E_FAIL;

	SVERTEXINFLU sTraceArray[10];
	for ( int i=0; i<8; i++ )
	{
		sTraceArray[i].LoadFile ( SFile );
	}

	if ( sTraceArray[0].m_dwIndex != COL_ERR )
		m_vecTrace.push_back ( STRACOR(STRACE_IMAGE01,sTraceArray[0],D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	if ( sTraceArray[1].m_dwIndex != COL_ERR )
		m_vecTrace.push_back ( STRACOR(STRACE_IMAGE02,sTraceArray[1],D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );

	if ( sTraceArray[2].m_dwIndex != COL_ERR )
		m_vecTrace.push_back ( STRACOR(STRACE_IMAGE11,sTraceArray[2],D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	if ( sTraceArray[3].m_dwIndex != COL_ERR )
		m_vecTrace.push_back ( STRACOR(STRACE_IMAGE12,sTraceArray[3],D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );

	std::sort ( m_vecTrace.begin(), m_vecTrace.end(), STRACOR_OPER() );

	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		DWORD TypeID;
		SFile >> TypeID;

		if ( IsEffectNotSupported ( TypeID ) )
		{
			//we need to bypass unsupported official effects
			DWORD dwEffectVer(0), dwEffectSize(0);
			SFile >> dwEffectVer;	
			SFile >> dwEffectSize;

			SFile.SetOffSet ( SFile.GetfTell()+dwEffectSize );

			CDebugSet::ErrorCode( "Unsupported DxEffChar Effect", "File:%s TypeID:%d Version:%d(%x) Size:%d", SFile.GetFileName(), TypeID, dwEffectVer, dwEffectVer, dwEffectSize );
			continue;
		}

		DxEffChar* pEffChar = DxEffCharMan::GetInstance().CreateEffInstance ( TypeID );
		if ( !pEffChar )	
		{
			CDebugSet::ErrorCode( "DxSkinPiece::LoadPiece_0101", "Unable to DxEffCharMan::GetInstance().CreateEffInstance  TypeID:%d", TypeID );
			return E_FAIL;
		}

		pEffChar->SetLinkObj ( NULL, this );
		hr = pEffChar->LoadFile ( SFile, pd3dDevice );
		if (FAILED(hr))
		{
			SAFE_DELETE(pEffChar);
			continue;
		}

		m_dwFlag |= pEffChar->GetFlag();
		m_vecEFFECT.push_back(pEffChar);
	}

	std::sort ( m_vecEFFECT.begin(), m_vecEFFECT.end(), DXEFFCHAR_OPER() );

	// Note : Add for Version.106
	m_emWeaponWhereBack = EMPEACE_WEAPON_RSLOT;

	return S_OK;
}

HRESULT DxSkinPiece::LoadPiece_0102 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr = S_OK;

	SFile >> m_dwRef;

	DWORD dwSize;

	SFile >> dwSize;
	char *szXFileName = new char[dwSize];
	SFile.ReadBuffer ( szXFileName, sizeof(char)*dwSize );

	SFile >> dwSize;
	char *szSkeleton = new char[dwSize];
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*dwSize );

	//	Note : 스킨 메쉬 읽기.
	//
	hr = SetSkinMesh ( szXFileName, szSkeleton, pd3dDevice, bThread );
	SAFE_DELETE_ARRAY(szXFileName);
	SAFE_DELETE_ARRAY(szSkeleton);
	if ( FAILED(hr) )	return hr;

	SFile >> dwSize;
	//m_emType = (EMPIECECHAR) dwSize;
	EMPIECECHAR_0113 emPIECEOLD = (EMPIECECHAR_0113) dwSize;
	m_emType = PIECE_0113_TO_PIECE( emPIECEOLD );

	SFile >> dwSize;
	m_szMeshName = new char[dwSize];
	SFile.ReadBuffer ( m_szMeshName, sizeof(char)*dwSize );

	m_pmcMesh = m_pSkinMesh->FindMeshContainer ( m_szMeshName );
	if ( !m_pmcMesh )	return E_FAIL;

	SVERTEXINFLU sTraceArray[10];
	for ( int i=0; i<10; i++ )
	{
		sTraceArray[i].LoadFile ( SFile );
	}

	if ( sTraceArray[0].m_dwIndex != COL_ERR )
		m_vecTrace.push_back ( STRACOR(STRACE_IMAGE01,sTraceArray[0],D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	if ( sTraceArray[1].m_dwIndex != COL_ERR )
		m_vecTrace.push_back ( STRACOR(STRACE_IMAGE02,sTraceArray[1],D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );

	if ( sTraceArray[2].m_dwIndex != COL_ERR )
		m_vecTrace.push_back ( STRACOR(STRACE_IMAGE11,sTraceArray[2],D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );

	if ( sTraceArray[3].m_dwIndex != COL_ERR )
		m_vecTrace.push_back ( STRACOR(STRACE_IMAGE12,sTraceArray[3],D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );

	std::sort ( m_vecTrace.begin(), m_vecTrace.end(), STRACOR_OPER() );

	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		DWORD TypeID;
		SFile >> TypeID;

		if ( IsEffectNotSupported ( TypeID ) )
		{
			//we need to bypass unsupported official effects
			DWORD dwEffectVer(0), dwEffectSize(0);
			SFile >> dwEffectVer;	
			SFile >> dwEffectSize;

			SFile.SetOffSet ( SFile.GetfTell()+dwEffectSize );

			CDebugSet::ErrorCode( "Unsupported DxEffChar Effect", "File:%s TypeID:%d Version:%d(%x) Size:%d", SFile.GetFileName(), TypeID, dwEffectVer, dwEffectVer, dwEffectSize );
			continue;
		}

		DxEffChar* pEffChar = DxEffCharMan::GetInstance().CreateEffInstance ( TypeID );
		if ( !pEffChar )	
		{
			CDebugSet::ErrorCode( "DxSkinPiece::LoadPiece_0102", "Unable to DxEffCharMan::GetInstance().CreateEffInstance  TypeID:%d", TypeID );
			return E_FAIL;
		}

		pEffChar->SetLinkObj ( NULL, this );
		hr = pEffChar->LoadFile ( SFile, pd3dDevice );
		if (FAILED(hr))
		{
			SAFE_DELETE(pEffChar);
			continue;
		}

		m_dwFlag |= pEffChar->GetFlag();
		m_vecEFFECT.push_back(pEffChar);
	}

	std::sort ( m_vecEFFECT.begin(), m_vecEFFECT.end(), DXEFFCHAR_OPER() );

	// Note : Add for Version.106
	m_emWeaponWhereBack = EMPEACE_WEAPON_RSLOT;

	return S_OK;
}

HRESULT DxSkinPiece::LoadPiece_0103 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr = S_OK;

	SFile >> m_dwRef;

	DWORD dwSize;

	SFile >> dwSize;
	char *szXFileName = new char[dwSize];
	SFile.ReadBuffer ( szXFileName, sizeof(char)*dwSize );

	SFile >> dwSize;
	char *szSkeleton = new char[dwSize];
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*dwSize );

	//	Note : 스킨 메쉬 읽기.
	//
	hr = SetSkinMesh ( szXFileName, szSkeleton, pd3dDevice, bThread );
	SAFE_DELETE_ARRAY(szXFileName);
	SAFE_DELETE_ARRAY(szSkeleton);
	if ( FAILED(hr) )	return hr;

	SFile >> dwSize;
	//m_emType = (EMPIECECHAR) dwSize;
	EMPIECECHAR_0113 emPIECEOLD = (EMPIECECHAR_0113) dwSize;
	m_emType = PIECE_0113_TO_PIECE( emPIECEOLD );

	SFile >> dwSize;
	m_szMeshName = new char[dwSize];
	SFile.ReadBuffer ( m_szMeshName, sizeof(char)*dwSize );

	m_pmcMesh = m_pSkinMesh->FindMeshContainer ( m_szMeshName );
	if ( !m_pmcMesh )	return E_FAIL;

	std::string strBuffer;
	SVERTEXINFLU sVertexInflu;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		SFile >> strBuffer;
		sVertexInflu.LoadFile ( SFile );

		m_vecTrace.push_back ( STRACOR(strBuffer.c_str(),sVertexInflu,D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	}

	std::sort ( m_vecTrace.begin(), m_vecTrace.end(), STRACOR_OPER() );

	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		DWORD TypeID;
		SFile >> TypeID;

		if ( IsEffectNotSupported ( TypeID ) )
		{
			//we need to bypass unsupported official effects
			DWORD dwEffectVer(0), dwEffectSize(0);
			SFile >> dwEffectVer;	
			SFile >> dwEffectSize;

			SFile.SetOffSet ( SFile.GetfTell()+dwEffectSize );

			CDebugSet::ErrorCode( "Unsupported DxEffChar Effect", "File:%s TypeID:%d Version:%d(%x) Size:%d", SFile.GetFileName(), TypeID, dwEffectVer, dwEffectVer, dwEffectSize );
			continue;
		}

		DxEffChar* pEffChar = DxEffCharMan::GetInstance().CreateEffInstance ( TypeID );
		if ( !pEffChar )	
		{
			CDebugSet::ErrorCode( "DxSkinPiece::LoadPiece_0103", "Unable to DxEffCharMan::GetInstance().CreateEffInstance  TypeID:%d", TypeID );
			return E_FAIL;
		}

		pEffChar->SetLinkObj ( NULL, this );
		hr = pEffChar->LoadFile ( SFile, pd3dDevice );
		if (FAILED(hr))
		{
			SAFE_DELETE(pEffChar);
			continue;
		}

		m_dwFlag |= pEffChar->GetFlag();
		m_vecEFFECT.push_back(pEffChar);
	}

	std::sort ( m_vecEFFECT.begin(), m_vecEFFECT.end(), DXEFFCHAR_OPER() );

	// Note : Add for Version.106
	m_emWeaponWhereBack = EMPEACE_WEAPON_RSLOT;

	return S_OK;
}

HRESULT DxSkinPiece::LoadPiece_0104 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr = S_OK;

	SFile >> m_dwRef;

	DWORD dwSize;

	SFile >> dwSize;
	char *szXFileName = new char[dwSize];
	SFile.ReadBuffer ( szXFileName, sizeof(char)*dwSize );

	SFile >> dwSize;
	char *szSkeleton = new char[dwSize];
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*dwSize );

	//	Note : 스킨 메쉬 읽기.
	//
	hr = SetSkinMesh ( szXFileName, szSkeleton, pd3dDevice, bThread );
	SAFE_DELETE_ARRAY(szXFileName);
	SAFE_DELETE_ARRAY(szSkeleton);
	if ( FAILED(hr) )	return hr;

	SFile >> dwSize;
	//m_emType = (EMPIECECHAR) dwSize;
	EMPIECECHAR_0113 emPIECEOLD = (EMPIECECHAR_0113) dwSize;
	m_emType = PIECE_0113_TO_PIECE( emPIECEOLD );

	SFile >> dwSize;
	m_szMeshName = new char[dwSize];
	SFile.ReadBuffer ( m_szMeshName, sizeof(char)*dwSize );

	m_pmcMesh = m_pSkinMesh->FindMeshContainer ( m_szMeshName );
	if ( !m_pmcMesh )	return E_FAIL;

	SFile >> m_dwMaterialNum;
	if ( m_dwMaterialNum!=0 )
	{
		SAFE_DELETE_ARRAY(m_pMaterialPiece);

		m_pMaterialPiece = new SMATERIAL_PIECE[m_dwMaterialNum];
		for ( DWORD i=0; i<m_dwMaterialNum; ++i )
		{
			m_pMaterialPiece[i].LoadFile ( SFile, pd3dDevice );
		}
	}

	std::string strBuffer;
	SVERTEXINFLU sVertexInflu;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		SFile >> strBuffer;
		sVertexInflu.LoadFile ( SFile );

		m_vecTrace.push_back ( STRACOR(strBuffer.c_str(),sVertexInflu,D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	}

	std::sort ( m_vecTrace.begin(), m_vecTrace.end(), STRACOR_OPER() );

	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		DWORD TypeID;
		SFile >> TypeID;

		if ( IsEffectNotSupported ( TypeID ) )
		{
			//we need to bypass unsupported official effects
			DWORD dwEffectVer(0), dwEffectSize(0);
			SFile >> dwEffectVer;	
			SFile >> dwEffectSize;

			SFile.SetOffSet ( SFile.GetfTell()+dwEffectSize );

			CDebugSet::ErrorCode( "Unsupported DxEffChar Effect", "File:%s TypeID:%d Version:%d(%x) Size:%d", SFile.GetFileName(), TypeID, dwEffectVer, dwEffectVer, dwEffectSize );
			continue;
		}

		DxEffChar* pEffChar = DxEffCharMan::GetInstance().CreateEffInstance ( TypeID );
		if ( !pEffChar )	
		{
			CDebugSet::ErrorCode( "DxSkinPiece::LoadPiece_0104", "Unable to DxEffCharMan::GetInstance().CreateEffInstance  TypeID:%d", TypeID );
			return E_FAIL;
		}

		pEffChar->SetLinkObj ( NULL, this );
		hr = pEffChar->LoadFile ( SFile, pd3dDevice );
		if (FAILED(hr))
		{
			SAFE_DELETE(pEffChar);
			continue;
		}

		m_dwFlag |= pEffChar->GetFlag();
		m_vecEFFECT.push_back(pEffChar);
	}

	std::sort ( m_vecEFFECT.begin(), m_vecEFFECT.end(), DXEFFCHAR_OPER() );

	// Note : Add for Version.106
	m_emWeaponWhereBack = EMPEACE_WEAPON_RSLOT;

	return S_OK;
}

HRESULT DxSkinPiece::LoadPiece_0105 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr = S_OK;

	SFile >> m_dwRef;

	DWORD dwSize;

	SFile >> dwSize;
	char *szXFileName = new char[dwSize];
	SFile.ReadBuffer ( szXFileName, sizeof(char)*dwSize );

	SFile >> dwSize;
	char *szSkeleton = new char[dwSize];
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*dwSize );

	//	Note : 스킨 메쉬 읽기.
	//
	hr = SetSkinMesh ( szXFileName, szSkeleton, pd3dDevice, bThread );
	SAFE_DELETE_ARRAY(szXFileName);
	SAFE_DELETE_ARRAY(szSkeleton);
	if ( FAILED(hr) )	return hr;

	SFile >> dwSize;
	//m_emType = (EMPIECECHAR) dwSize;
	EMPIECECHAR_0113 emPIECEOLD = (EMPIECECHAR_0113) dwSize;
	m_emType = PIECE_0113_TO_PIECE( emPIECEOLD );

	SFile >> dwSize;
	m_szMeshName = new char[dwSize];
	SFile.ReadBuffer ( m_szMeshName, sizeof(char)*dwSize );

	m_pmcMesh = m_pSkinMesh->FindMeshContainer ( m_szMeshName );
	if ( !m_pmcMesh )	return E_FAIL;

	SFile >> m_dwMaterialNum;
	if ( m_dwMaterialNum!=0 )
	{
		SAFE_DELETE_ARRAY(m_pMaterialPiece);

		m_pMaterialPiece = new SMATERIAL_PIECE[m_dwMaterialNum];
		for ( DWORD i=0; i<m_dwMaterialNum; ++i )
		{
			m_pMaterialPiece[i].LoadFile ( SFile, pd3dDevice );
		}
	}

	std::string strBuffer;
	SVERTEXINFLU sVertexInflu;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		SFile >> strBuffer;
		sVertexInflu.LoadFile ( SFile );

		m_vecTrace.push_back ( STRACOR(strBuffer.c_str(),sVertexInflu,D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	}

	std::sort ( m_vecTrace.begin(), m_vecTrace.end(), STRACOR_OPER() );

	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		DWORD TypeID;
		SFile >> TypeID;

		if ( IsEffectNotSupported ( TypeID ) )
		{
			//we need to bypass unsupported official effects
			DWORD dwEffectVer(0), dwEffectSize(0);
			SFile >> dwEffectVer;	
			SFile >> dwEffectSize;

			SFile.SetOffSet ( SFile.GetfTell()+dwEffectSize );

			CDebugSet::ErrorCode( "Unsupported DxEffChar Effect", "File:%s TypeID:%d Version:%d(%x) Size:%d", SFile.GetFileName(), TypeID, dwEffectVer, dwEffectVer, dwEffectSize );
			continue;
		}

		DxEffChar* pEffChar = DxEffCharMan::GetInstance().CreateEffInstance ( TypeID );
		if ( !pEffChar )	
		{
			CDebugSet::ErrorCode( "DxSkinPiece::LoadPiece_0105", "Unable to DxEffCharMan::GetInstance().CreateEffInstance  TypeID:%d", TypeID );
			return E_FAIL;
		}

		pEffChar->SetLinkObj ( NULL, this );
		hr = pEffChar->LoadFile ( SFile, pd3dDevice );
		if (FAILED(hr))
		{
			SAFE_DELETE(pEffChar);
			continue;
		}

		m_vecEFFECT.push_back ( pEffChar );
	}

	std::sort ( m_vecEFFECT.begin(), m_vecEFFECT.end(), DXEFFCHAR_OPER() );

	SFile >> m_dwFlag;

	// Note : Add for Version.106
	m_emWeaponWhereBack = EMPEACE_WEAPON_RSLOT;

	return S_OK;
}

HRESULT DxSkinPiece::LoadPiece_0106 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr = S_OK;

	SFile >> m_dwRef;

	DWORD dwSize;

	SFile >> dwSize;
	char *szXFileName = new char[dwSize];
	SFile.ReadBuffer ( szXFileName, sizeof(char)*dwSize );

	SFile >> dwSize;
	char *szSkeleton = new char[dwSize];
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*dwSize );

	//	Note : 스킨 메쉬 읽기.
	//
	hr = SetSkinMesh ( szXFileName, szSkeleton, pd3dDevice, bThread );
	SAFE_DELETE_ARRAY(szXFileName);
	SAFE_DELETE_ARRAY(szSkeleton);
	if ( FAILED(hr) )	return hr;

	SFile >> dwSize;
	//m_emType = (EMPIECECHAR) dwSize;
	EMPIECECHAR_0113 emPIECEOLD = (EMPIECECHAR_0113) dwSize;
	m_emType = PIECE_0113_TO_PIECE( emPIECEOLD );

	SFile >> dwSize;
	m_emWeaponWhereBack = (EMPEACEZONEWEAPON) dwSize;

	SFile >> dwSize;
	m_szMeshName = new char[dwSize];
	SFile.ReadBuffer ( m_szMeshName, sizeof(char)*dwSize );

	m_pmcMesh = m_pSkinMesh->FindMeshContainer ( m_szMeshName );
	if ( !m_pmcMesh )	return E_FAIL;

	SFile >> m_dwMaterialNum;
	if ( m_dwMaterialNum!=0 )
	{
		SAFE_DELETE_ARRAY(m_pMaterialPiece);

		m_pMaterialPiece = new SMATERIAL_PIECE[m_dwMaterialNum];
		for ( DWORD i=0; i<m_dwMaterialNum; ++i )
		{
			m_pMaterialPiece[i].LoadFile ( SFile, pd3dDevice );
		}
	}

	std::string strBuffer;
	SVERTEXINFLU sVertexInflu;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		SFile >> strBuffer;
		sVertexInflu.LoadFile ( SFile );

		m_vecTrace.push_back ( STRACOR(strBuffer.c_str(),sVertexInflu,D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	}

	std::sort ( m_vecTrace.begin(), m_vecTrace.end(), STRACOR_OPER() );

	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		DWORD TypeID;
		SFile >> TypeID;

		if ( IsEffectNotSupported ( TypeID ) )
		{
			//we need to bypass unsupported official effects
			DWORD dwEffectVer(0), dwEffectSize(0);
			SFile >> dwEffectVer;	
			SFile >> dwEffectSize;

			SFile.SetOffSet ( SFile.GetfTell()+dwEffectSize );

			CDebugSet::ErrorCode( "Unsupported DxEffChar Effect", "File:%s TypeID:%d Version:%d(%x) Size:%d", SFile.GetFileName(), TypeID, dwEffectVer, dwEffectVer, dwEffectSize );
			continue;
		}

		DxEffChar* pEffChar = DxEffCharMan::GetInstance().CreateEffInstance ( TypeID );
		if ( !pEffChar )	
		{
			CDebugSet::ErrorCode( "DxSkinPiece::LoadPiece_0106", "Unable to DxEffCharMan::GetInstance().CreateEffInstance  TypeID:%d", TypeID );
			return E_FAIL;
		}

		pEffChar->SetLinkObj ( NULL, this );
		hr = pEffChar->LoadFile ( SFile, pd3dDevice );
		if (FAILED(hr))
		{
			SAFE_DELETE(pEffChar);
			continue;
		}

		m_vecEFFECT.push_back ( pEffChar );
	}

	std::sort ( m_vecEFFECT.begin(), m_vecEFFECT.end(), DXEFFCHAR_OPER() );

	SFile >> m_dwFlag;

	return S_OK;
}

HRESULT DxSkinPiece::LoadPiece_0107 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr = S_OK;

	SFile >> m_dwRef;

	DWORD dwSize;

	SFile >> dwSize;
	char *szXFileName = new char[dwSize];
	SFile.ReadBuffer ( szXFileName, sizeof(char)*dwSize );

	SFile >> dwSize;
	char *szSkeleton = new char[dwSize];
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*dwSize );

	//	Note : 스킨 메쉬 읽기.
	//
	hr = SetSkinMesh ( szXFileName, szSkeleton, pd3dDevice, bThread );
	SAFE_DELETE_ARRAY(szXFileName);
	SAFE_DELETE_ARRAY(szSkeleton);
	if ( FAILED(hr) )	return hr;

	SFile >> dwSize;
	//m_emType = (EMPIECECHAR) dwSize;
	EMPIECECHAR_0113 emPIECEOLD = (EMPIECECHAR_0113) dwSize;
	m_emType = PIECE_0113_TO_PIECE( emPIECEOLD );

	SFile >> dwSize;
	m_emWeaponWhereBack = (EMPEACEZONEWEAPON) dwSize;

	SFile >> dwSize;
	m_szMeshName = new char[dwSize];
	SFile.ReadBuffer ( m_szMeshName, sizeof(char)*dwSize );

	m_pmcMesh = m_pSkinMesh->FindMeshContainer ( m_szMeshName );
	if ( !m_pmcMesh )	return E_FAIL;

	SFile >> m_dwMaterialNum;
	if ( m_dwMaterialNum!=0 )
	{
		SAFE_DELETE_ARRAY(m_pMaterialPiece);

		m_pMaterialPiece = new SMATERIAL_PIECE[m_dwMaterialNum];
		for ( DWORD i=0; i<m_dwMaterialNum; ++i )
		{
			m_pMaterialPiece[i].LoadFile ( SFile, pd3dDevice );
		}
	}

	std::string strBuffer;
	SVERTEXINFLU sVertexInflu;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		SFile >> strBuffer;
		sVertexInflu.LoadFile ( SFile );

		m_vecTrace.push_back ( STRACOR(strBuffer.c_str(),sVertexInflu,D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	}

	std::sort ( m_vecTrace.begin(), m_vecTrace.end(), STRACOR_OPER() );

	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		DWORD TypeID;
		SFile >> TypeID;

		if ( IsEffectNotSupported ( TypeID ) )
		{
			//we need to bypass unsupported official effects
			DWORD dwEffectVer(0), dwEffectSize(0);
			SFile >> dwEffectVer;	
			SFile >> dwEffectSize;

			SFile.SetOffSet ( SFile.GetfTell()+dwEffectSize );

			CDebugSet::ErrorCode( "Unsupported DxEffChar Effect", "File:%s TypeID:%d Version:%d(%x) Size:%d", SFile.GetFileName(), TypeID, dwEffectVer, dwEffectVer, dwEffectSize );
			continue;
		}

		DxEffChar* pEffChar = DxEffCharMan::GetInstance().CreateEffInstance ( TypeID );
		if ( !pEffChar )	
		{
			CDebugSet::ErrorCode( "DxSkinPiece::LoadPiece_0107", "Unable to DxEffCharMan::GetInstance().CreateEffInstance  TypeID:%d", TypeID );
			return E_FAIL;
		}

		pEffChar->SetLinkObj ( NULL, this );
		hr = pEffChar->LoadFile ( SFile, pd3dDevice );
		if (FAILED(hr))
		{
			SAFE_DELETE(pEffChar);
			continue;
		}

		m_vecEFFECT.push_back ( pEffChar );
	}

	std::sort ( m_vecEFFECT.begin(), m_vecEFFECT.end(), DXEFFCHAR_OPER() );

	SFile >> m_dwFlag;

	return S_OK;
}

HRESULT DxSkinPiece::LoadPiece_0108 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr(S_OK);

	SFile >> m_dwRef;

	DWORD dwSize;

	//skeleton
	SFile >> dwSize;
	char *szSkeleton = new char[dwSize];
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*dwSize );

	//skin
	SFile >> dwSize;
	char *szXFileName = new char[dwSize];
	SFile.ReadBuffer ( szXFileName, sizeof(char)*dwSize );

	hr = SetSkinMesh ( szXFileName, szSkeleton, pd3dDevice, bThread );
	SAFE_DELETE_ARRAY(szXFileName);
	SAFE_DELETE_ARRAY(szSkeleton);
	if ( FAILED(hr) )	return hr;

	//mesh
	SFile >> dwSize;
	m_szMeshName = new char[dwSize];
	SFile.ReadBuffer ( m_szMeshName, sizeof(char)*dwSize );
	m_pmcMesh = m_pSkinMesh->FindMeshContainer ( m_szMeshName );
	if ( !m_pmcMesh )	return E_FAIL;

	//material
	SFile >> m_dwMaterialNum;
	if ( m_dwMaterialNum!=0 )
	{
		SAFE_DELETE_ARRAY(m_pMaterialPiece);

		m_pMaterialPiece = new SMATERIAL_PIECE[m_dwMaterialNum];
		for ( DWORD i=0; i<m_dwMaterialNum; ++i )
		{
			m_pMaterialPiece[i].LoadFile ( SFile, pd3dDevice );
		}
	}

	SFile >> dwSize;
	//m_emType = (EMPIECECHAR) dwSize;
	EMPIECECHAR_0113 emPIECEOLD = (EMPIECECHAR_0113) dwSize;
	m_emType = PIECE_0113_TO_PIECE( emPIECEOLD );

	SFile >> dwSize;
	m_emWeaponWhereBack = (EMPEACEZONEWEAPON) dwSize;

	//effect
	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		DWORD TypeID;
		SFile >> TypeID;

		if ( IsEffectNotSupported ( TypeID ) )
		{
			//we need to bypass unsupported official effects
			DWORD dwEffectVer(0), dwEffectSize(0);
			SFile >> dwEffectVer;	
			SFile >> dwEffectSize;

			SFile.SetOffSet ( SFile.GetfTell()+dwEffectSize );

			CDebugSet::ErrorCode( "Unsupported DxEffChar Effect", "File:%s TypeID:%d Version:%d(%x) Size:%d", SFile.GetFileName(), TypeID, dwEffectVer, dwEffectVer, dwEffectSize );
			continue;
		}

		DxEffChar* pEffChar = DxEffCharMan::GetInstance().CreateEffInstance ( TypeID );
		if ( !pEffChar )	
		{
			CDebugSet::ErrorCode( "DxSkinPiece::LoadPiece_0108", "Unable to DxEffCharMan::GetInstance().CreateEffInstance  TypeID:%d", TypeID );
			return E_FAIL;
		}

		pEffChar->SetLinkObj ( NULL, this );
		hr = pEffChar->LoadFile ( SFile, pd3dDevice );
		if (FAILED(hr))
		{
			SAFE_DELETE(pEffChar);
			continue;
		}

		m_vecEFFECT.push_back ( pEffChar );
	}

	std::sort ( m_vecEFFECT.begin(), m_vecEFFECT.end(), DXEFFCHAR_OPER() );

	//trace
	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		std::string strBuffer;
		SVERTEXINFLU sVertexInflu;
		SFile >> strBuffer;
		sVertexInflu.LoadFile ( SFile );

		m_vecTrace.push_back ( STRACOR(strBuffer.c_str(),sVertexInflu,D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	}

	std::sort ( m_vecTrace.begin(), m_vecTrace.end(), STRACOR_OPER() );

	//flag
	SFile >> m_dwFlag;

	//official bug!
	//some effects that require trace-info will fail
	//since effect is loaded before trace 
	//the effect wont be able to find the trace

	return S_OK;
}

HRESULT DxSkinPiece::LoadPiece_0109 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr(S_OK);

	SFile >> m_dwRef;

	DWORD dwSize;

	//skeleton
	SFile >> dwSize;
	char *szSkeleton = new char[dwSize];
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*dwSize );

	//skin
	SFile >> dwSize;
	char *szXFileName = new char[dwSize];
	SFile.ReadBuffer ( szXFileName, sizeof(char)*dwSize );

	hr = SetSkinMesh ( szXFileName, szSkeleton, pd3dDevice, bThread );
	SAFE_DELETE_ARRAY(szXFileName);
	SAFE_DELETE_ARRAY(szSkeleton);
	if ( FAILED(hr) )	return hr;

	//mesh
	SFile >> dwSize;
	m_szMeshName = new char[dwSize];
	SFile.ReadBuffer ( m_szMeshName, sizeof(char)*dwSize );
	m_pmcMesh = m_pSkinMesh->FindMeshContainer ( m_szMeshName );
	if ( !m_pmcMesh )	return E_FAIL;

	//material
	SFile >> m_dwMaterialNum;
	if ( m_dwMaterialNum!=0 )
	{
		SAFE_DELETE_ARRAY(m_pMaterialPiece);

		m_pMaterialPiece = new SMATERIAL_PIECE[m_dwMaterialNum];
		for ( DWORD i=0; i<m_dwMaterialNum; ++i )
		{
			m_pMaterialPiece[i].LoadFile ( SFile, pd3dDevice );
		}
	}

	//piece type
	SFile >> dwSize;
	//m_emType = (EMPIECECHAR) dwSize;
	EMPIECECHAR_0113 emPIECEOLD = (EMPIECECHAR_0113) dwSize;
	m_emType = PIECE_0113_TO_PIECE( emPIECEOLD );

	//weapon where back
	SFile >> dwSize;
	m_emWeaponWhereBack = (EMPEACEZONEWEAPON) dwSize;

	//trace
	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		std::string strBuffer;
		SVERTEXINFLU sVertexInflu;
		SFile >> strBuffer;
		sVertexInflu.LoadFile ( SFile );

		m_vecTrace.push_back ( STRACOR(strBuffer.c_str(),sVertexInflu,D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	}

	std::sort ( m_vecTrace.begin(), m_vecTrace.end(), STRACOR_OPER() );

	//effect
	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		DWORD TypeID;
		SFile >> TypeID;

		if ( IsEffectNotSupported ( TypeID ) )
		{
			//we need to bypass unsupported official effects
			DWORD dwEffectVer(0), dwEffectSize(0);
			SFile >> dwEffectVer;	
			SFile >> dwEffectSize;

			SFile.SetOffSet ( SFile.GetfTell()+dwEffectSize );

			CDebugSet::ErrorCode( "Unsupported DxEffChar Effect", "File:%s TypeID:%d Version:%d(%x) Size:%d", SFile.GetFileName(), TypeID, dwEffectVer, dwEffectVer, dwEffectSize );
			continue;
		}

		DxEffChar* pEffChar = DxEffCharMan::GetInstance().CreateEffInstance ( TypeID );
		if ( !pEffChar )	
		{
			CDebugSet::ErrorCode( "DxSkinPiece::LoadPiece_0109", "Unable to DxEffCharMan::GetInstance().CreateEffInstance  TypeID:%d", TypeID );
			return E_FAIL;
		}

		pEffChar->SetLinkObj ( NULL, this );
		hr = pEffChar->LoadFile ( SFile, pd3dDevice );
		if (FAILED(hr))
		{
			SAFE_DELETE(pEffChar);
			continue;
		}

		m_vecEFFECT.push_back ( pEffChar );
	}

	std::sort ( m_vecEFFECT.begin(), m_vecEFFECT.end(), DXEFFCHAR_OPER() );

	//flag
	SFile >> m_dwFlag;

	return S_OK;
}

HRESULT DxSkinPiece::LoadPiece_0110 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr(S_OK);

	SFile >> m_dwRef;

	DWORD dwSize;

	//skeleton
	SFile >> dwSize;
	char *szSkeleton = new char[dwSize];
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*dwSize );

	//skin
	SFile >> dwSize;
	char *szXFileName = new char[dwSize];
	SFile.ReadBuffer ( szXFileName, sizeof(char)*dwSize );

	hr = SetSkinMesh ( szXFileName, szSkeleton, pd3dDevice, bThread );
	SAFE_DELETE_ARRAY(szXFileName);
	SAFE_DELETE_ARRAY(szSkeleton);
	if ( FAILED(hr) )	return hr;

	//mesh
	SFile >> dwSize;
	m_szMeshName = new char[dwSize];
	SFile.ReadBuffer ( m_szMeshName, sizeof(char)*dwSize );
	m_pmcMesh = m_pSkinMesh->FindMeshContainer ( m_szMeshName );
	if ( !m_pmcMesh )	return E_FAIL;

	//material
	SFile >> m_dwMaterialNum;
	if ( m_dwMaterialNum!=0 )
	{
		SAFE_DELETE_ARRAY(m_pMaterialPiece);

		m_pMaterialPiece = new SMATERIAL_PIECE[m_dwMaterialNum];
		for ( DWORD i=0; i<m_dwMaterialNum; ++i )
		{
			m_pMaterialPiece[i].LoadFile ( SFile, pd3dDevice );
		}
	}

	//piece type
	SFile >> dwSize;
	//m_emType = (EMPIECECHAR) dwSize;
	EMPIECECHAR_0113 emPIECEOLD = (EMPIECECHAR_0113) dwSize;
	m_emType = PIECE_0113_TO_PIECE( emPIECEOLD );

	//weapon where back
	SFile >> dwSize;
	m_emWeaponWhereBack = (EMPEACEZONEWEAPON) dwSize;

	//trace
	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		std::string strBuffer;
		SVERTEXINFLU sVertexInflu;
		SFile >> strBuffer;
		sVertexInflu.LoadFile ( SFile );

		m_vecTrace.push_back ( STRACOR(strBuffer.c_str(),sVertexInflu,D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	}

	std::sort ( m_vecTrace.begin(), m_vecTrace.end(), STRACOR_OPER() );

	//effect
	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		DWORD TypeID;
		SFile >> TypeID;

		if ( IsEffectNotSupported ( TypeID ) )
		{
			//we need to bypass unsupported official effects
			DWORD dwEffectVer(0), dwEffectSize(0);
			SFile >> dwEffectVer;	
			SFile >> dwEffectSize;

			SFile.SetOffSet ( SFile.GetfTell()+dwEffectSize );

			CDebugSet::ErrorCode( "Unsupported DxEffChar Effect", "File:%s TypeID:%d Version:%d(%x) Size:%d", SFile.GetFileName(), TypeID, dwEffectVer, dwEffectVer, dwEffectSize );
			continue;
		}

		DxEffChar* pEffChar = DxEffCharMan::GetInstance().CreateEffInstance ( TypeID );
		if ( !pEffChar )	
		{
			CDebugSet::ErrorCode( "DxSkinPiece::LoadPiece_0110", "Unable to DxEffCharMan::GetInstance().CreateEffInstance  TypeID:%d", TypeID );
			return E_FAIL;
		}

		pEffChar->SetLinkObj ( NULL, this );
		hr = pEffChar->LoadFile ( SFile, pd3dDevice );
		if (FAILED(hr))
		{
			SAFE_DELETE(pEffChar);
			continue;
		}

		m_vecEFFECT.push_back ( pEffChar );
	}

	std::sort ( m_vecEFFECT.begin(), m_vecEFFECT.end(), DXEFFCHAR_OPER() );

	//flag
	SFile >> m_dwFlag;

	return S_OK;
}

HRESULT DxSkinPiece::LoadPiece_0112_0113 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr(S_OK);

	SFile >> m_dwRef;

	DWORD dwSize;

	//skeleton
	SFile >> dwSize;
	char *szSkeleton = new char[dwSize];
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*dwSize );

	//skin
	SFile >> dwSize;
	char *szXFileName = new char[dwSize];
	SFile.ReadBuffer ( szXFileName, sizeof(char)*dwSize );

	hr = SetSkinMesh ( szXFileName, szSkeleton, pd3dDevice, bThread );
	SAFE_DELETE_ARRAY(szXFileName);
	SAFE_DELETE_ARRAY(szSkeleton);
	if ( FAILED(hr) )	return hr;

	//mesh
	SFile >> dwSize;
	m_szMeshName = new char[dwSize];
	SFile.ReadBuffer ( m_szMeshName, sizeof(char)*dwSize );
	m_pmcMesh = m_pSkinMesh->FindMeshContainer ( m_szMeshName );
	if ( !m_pmcMesh )	return E_FAIL;

	//added from 0x0112
	DWORD m_w0112_1;
	SFile >> m_w0112_1;

	//material
	SFile >> m_dwMaterialNum;
	if ( m_dwMaterialNum!=0 )
	{
		SAFE_DELETE_ARRAY(m_pMaterialPiece);

		m_pMaterialPiece = new SMATERIAL_PIECE[m_dwMaterialNum];
		for ( DWORD i=0; i<m_dwMaterialNum; ++i )
		{
			m_pMaterialPiece[i].LoadFile ( SFile, pd3dDevice );
		}
	}

	//piece type
	SFile >> dwSize;
	//m_emType = (EMPIECECHAR) dwSize;
	EMPIECECHAR_0113 emPIECEOLD = (EMPIECECHAR_0113) dwSize;
	m_emType = PIECE_0113_TO_PIECE( emPIECEOLD );

	//weapon where back
	SFile >> dwSize;
	m_emWeaponWhereBack = (EMPEACEZONEWEAPON) dwSize;

	//added from 0x0112
	//userslot
	m_dxUserSlot.LoadFile( SFile, pd3dDevice );

	//trace
	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		std::string strBuffer;
		SVERTEXINFLU sVertexInflu;
		SFile >> strBuffer;
		sVertexInflu.LoadFile ( SFile );

		m_vecTrace.push_back ( STRACOR(strBuffer.c_str(),sVertexInflu,D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	}

	std::sort ( m_vecTrace.begin(), m_vecTrace.end(), STRACOR_OPER() );

	//effect
	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		DWORD TypeID;
		SFile >> TypeID;

		if ( IsEffectNotSupported ( TypeID ) )
		{
			//we need to bypass unsupported official effects
			DWORD dwEffectVer(0), dwEffectSize(0);
			SFile >> dwEffectVer;	
			SFile >> dwEffectSize;

			SFile.SetOffSet ( SFile.GetfTell()+dwEffectSize );

			CDebugSet::ErrorCode( "Unsupported DxEffChar Effect", "File:%s TypeID:%d Version:%d(%x) Size:%d", SFile.GetFileName(), TypeID, dwEffectVer, dwEffectVer, dwEffectSize );
			continue;
		}

		DxEffChar* pEffChar = DxEffCharMan::GetInstance().CreateEffInstance ( TypeID );
		if ( !pEffChar )	
		{
			CDebugSet::ErrorCode( "DxSkinPiece::LoadPiece_0112_0113", "Unable to DxEffCharMan::GetInstance().CreateEffInstance  TypeID:%d", TypeID );
			return E_FAIL;
		}

		pEffChar->SetLinkObj ( NULL, this );
		hr = pEffChar->LoadFile ( SFile, pd3dDevice );
		if (FAILED(hr))
		{
			SAFE_DELETE(pEffChar);
			continue;
		}

		m_vecEFFECT.push_back ( pEffChar );
	}

	std::sort ( m_vecEFFECT.begin(), m_vecEFFECT.end(), DXEFFCHAR_OPER() );

	//flag
	SFile >> m_dwFlag;

	return S_OK;
}

HRESULT DxSkinPiece::LoadPiece_0114_0115 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr(S_OK);

	SFile >> m_dwRef;

	DWORD dwSize;

	//skeleton
	SFile >> dwSize;
	char *szSkeleton = new char[dwSize];
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*dwSize );

	//skin
	SFile >> dwSize;
	char *szXFileName = new char[dwSize];
	SFile.ReadBuffer ( szXFileName, sizeof(char)*dwSize );

	hr = SetSkinMesh ( szXFileName, szSkeleton, pd3dDevice, bThread );
	SAFE_DELETE_ARRAY(szXFileName);
	SAFE_DELETE_ARRAY(szSkeleton);
	if ( FAILED(hr) )	return hr;

	//mesh
	SFile >> dwSize;
	m_szMeshName = new char[dwSize];
	SFile.ReadBuffer ( m_szMeshName, sizeof(char)*dwSize );
	m_pmcMesh = m_pSkinMesh->FindMeshContainer ( m_szMeshName );
	if ( !m_pmcMesh )	return E_FAIL;

	//added from 0x0112
	DWORD m_w0112_1;
	SFile >> m_w0112_1;

	//material
	SFile >> m_dwMaterialNum;
	if ( m_dwMaterialNum!=0 )
	{
		SAFE_DELETE_ARRAY(m_pMaterialPiece);

		m_pMaterialPiece = new SMATERIAL_PIECE[m_dwMaterialNum];
		for ( DWORD i=0; i<m_dwMaterialNum; ++i )
		{
			m_pMaterialPiece[i].LoadFile ( SFile, pd3dDevice );
		}
	}

	//piece type
	SFile >> dwSize;
	//m_emType = (EMPIECECHAR) dwSize;
	EMPIECECHAR_0116 emPIECEOLD = (EMPIECECHAR_0116) dwSize;
	m_emType = PIECE_0116_TO_PIECE( emPIECEOLD );

	//weapon where back
	SFile >> dwSize;
	m_emWeaponWhereBack = (EMPEACEZONEWEAPON) dwSize;

	//added from 0x0112
	//userslot
	m_dxUserSlot.LoadFile( SFile, pd3dDevice );

	//trace
	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		std::string strBuffer;
		SVERTEXINFLU sVertexInflu;
		SFile >> strBuffer;
		sVertexInflu.LoadFile ( SFile );

		m_vecTrace.push_back ( STRACOR(strBuffer.c_str(),sVertexInflu,D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	}

	std::sort ( m_vecTrace.begin(), m_vecTrace.end(), STRACOR_OPER() );

	//effect
	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		DWORD TypeID;
		SFile >> TypeID;

		if ( IsEffectNotSupported ( TypeID ) )
		{
			//we need to bypass unsupported official effects
			DWORD dwEffectVer(0), dwEffectSize(0);
			SFile >> dwEffectVer;	
			SFile >> dwEffectSize;

			SFile.SetOffSet ( SFile.GetfTell()+dwEffectSize );

			CDebugSet::ErrorCode( "Unsupported DxEffChar Effect", "File:%s TypeID:%d Version:%d(%x) Size:%d", SFile.GetFileName(), TypeID, dwEffectVer, dwEffectVer, dwEffectSize );
			continue;
		}

		DxEffChar* pEffChar = DxEffCharMan::GetInstance().CreateEffInstance ( TypeID );
		if ( !pEffChar )	
		{
			CDebugSet::ErrorCode( "DxSkinPiece::LoadPiece_0114_0115", "Unable to DxEffCharMan::GetInstance().CreateEffInstance  TypeID:%d", TypeID );
			return E_FAIL;
		}

		pEffChar->SetLinkObj ( NULL, this );
		hr = pEffChar->LoadFile ( SFile, pd3dDevice );
		if (FAILED(hr))
		{
			SAFE_DELETE(pEffChar);
			continue;
		}

		m_vecEFFECT.push_back ( pEffChar );
	}

	std::sort ( m_vecEFFECT.begin(), m_vecEFFECT.end(), DXEFFCHAR_OPER() );

	//flag
	SFile >> m_dwFlag;

	return S_OK;
}

HRESULT DxSkinPiece::LoadPiece_0116 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr(S_OK);

	SFile >> m_dwRef;

	DWORD dwSize;

	//skeleton
	SFile >> dwSize;
	char *szSkeleton = new char[dwSize];
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*dwSize );

	//skin
	SFile >> dwSize;
	char *szXFileName = new char[dwSize];
	SFile.ReadBuffer ( szXFileName, sizeof(char)*dwSize );

	hr = SetSkinMesh ( szXFileName, szSkeleton, pd3dDevice, bThread );
	SAFE_DELETE_ARRAY(szXFileName);
	SAFE_DELETE_ARRAY(szSkeleton);
	if ( FAILED(hr) )	return hr;

	//mesh
	SFile >> dwSize;
	m_szMeshName = new char[dwSize];
	SFile.ReadBuffer ( m_szMeshName, sizeof(char)*dwSize );
	m_pmcMesh = m_pSkinMesh->FindMeshContainer ( m_szMeshName );
	if ( !m_pmcMesh )	return E_FAIL;

	//added from 0x0112
	DWORD m_w0112_1;
	SFile >> m_w0112_1;

	//material
	SFile >> m_dwMaterialNum;
	if ( m_dwMaterialNum!=0 )
	{
		SAFE_DELETE_ARRAY(m_pMaterialPiece);

		m_pMaterialPiece = new SMATERIAL_PIECE[m_dwMaterialNum];
		for ( DWORD i=0; i<m_dwMaterialNum; ++i )
		{
			m_pMaterialPiece[i].LoadFile ( SFile, pd3dDevice );
		}
	}

	//piece type
	SFile >> dwSize;
	//m_emType = (EMPIECECHAR) dwSize;
	EMPIECECHAR_0116 emPIECEOLD = (EMPIECECHAR_0116) dwSize;
	m_emType = PIECE_0116_TO_PIECE( emPIECEOLD );

	//weapon where back
	SFile >> dwSize;
	m_emWeaponWhereBack = (EMPEACEZONEWEAPON) dwSize;

	//added from 0x0112
	//userslot
	m_dxUserSlot.LoadFile( SFile, pd3dDevice );

	//trace
	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		std::string strBuffer;
		SVERTEXINFLU sVertexInflu;
		SFile >> strBuffer;
		sVertexInflu.LoadFile ( SFile );

		m_vecTrace.push_back ( STRACOR(strBuffer.c_str(),sVertexInflu,D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	}

	std::sort ( m_vecTrace.begin(), m_vecTrace.end(), STRACOR_OPER() );

	//effect
	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		DWORD TypeID;
		SFile >> TypeID;

		if ( IsEffectNotSupported ( TypeID ) )
		{
			//we need to bypass unsupported official effects
			DWORD dwEffectVer(0), dwEffectSize(0);
			SFile >> dwEffectVer;	
			SFile >> dwEffectSize;

			SFile.SetOffSet ( SFile.GetfTell()+dwEffectSize );

			CDebugSet::ErrorCode( "Unsupported DxEffChar Effect", "File:%s TypeID:%d Version:%d(%x) Size:%d", SFile.GetFileName(), TypeID, dwEffectVer, dwEffectVer, dwEffectSize );
			continue;
		}

		DxEffChar* pEffChar = DxEffCharMan::GetInstance().CreateEffInstance ( TypeID );
		if ( !pEffChar )	
		{
			CDebugSet::ErrorCode( "DxSkinPiece::LoadPiece_0116", "Unable to DxEffCharMan::GetInstance().CreateEffInstance  TypeID:%d", TypeID );
			return E_FAIL;
		}

		pEffChar->SetLinkObj ( NULL, this );
		hr = pEffChar->LoadFile ( SFile, pd3dDevice );
		if (FAILED(hr))
		{
			SAFE_DELETE(pEffChar);
			continue;
		}

		m_vecEFFECT.push_back ( pEffChar );
	}

	std::sort ( m_vecEFFECT.begin(), m_vecEFFECT.end(), DXEFFCHAR_OPER() );

	//flag
	SFile >> m_dwFlag;

	//added from 0x0116
	DWORD dw0116_1;
	std::string str0116;
	SFile >> dw0116_1;
	SFile >> str0116;

	return S_OK;
}

HRESULT DxSkinPiece::LoadPiece_0200 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr(S_OK);

	SFile >> m_dwRef;

	DWORD dwSize;

	//skeleton
	SFile >> dwSize;
	char *szSkeleton = new char[dwSize];
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*dwSize );

	//skin
	SFile >> dwSize;
	char *szXFileName = new char[dwSize];
	SFile.ReadBuffer ( szXFileName, sizeof(char)*dwSize );

	hr = SetSkinMesh ( szXFileName, szSkeleton, pd3dDevice, bThread );
	SAFE_DELETE_ARRAY(szXFileName);
	SAFE_DELETE_ARRAY(szSkeleton);
	if ( FAILED(hr) )	return hr;

	//mesh
	SFile >> dwSize;
	m_szMeshName = new char[dwSize];
	SFile.ReadBuffer ( m_szMeshName, sizeof(char)*dwSize );
	m_pmcMesh = m_pSkinMesh->FindMeshContainer ( m_szMeshName );
	if ( !m_pmcMesh )	return E_FAIL;

	//material
	SFile >> m_dwMaterialNum;
	if ( m_dwMaterialNum!=0 )
	{
		SAFE_DELETE_ARRAY(m_pMaterialPiece);

		m_pMaterialPiece = new SMATERIAL_PIECE[m_dwMaterialNum];
		for ( DWORD i=0; i<m_dwMaterialNum; ++i )
		{
			m_pMaterialPiece[i].LoadFile ( SFile, pd3dDevice );
		}
	}

	//piece type
	SFile >> dwSize;
	//m_emType = (EMPIECECHAR) dwSize;
	EMPIECECHAR_0116 emPIECEOLD = (EMPIECECHAR_0116) dwSize;
	m_emType = PIECE_0116_TO_PIECE( emPIECEOLD );

	//weapon where back
	SFile >> dwSize;
	m_emWeaponWhereBack = (EMPEACEZONEWEAPON) dwSize;

	//userslot
	m_dxUserSlot.LoadFile( SFile, pd3dDevice );

	//trace
	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		std::string strBuffer;
		SVERTEXINFLU sVertexInflu;
		SFile >> strBuffer;
		sVertexInflu.LoadFile ( SFile );

		m_vecTrace.push_back ( STRACOR(strBuffer.c_str(),sVertexInflu,D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	}

	std::sort ( m_vecTrace.begin(), m_vecTrace.end(), STRACOR_OPER() );

	//effect
	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		DWORD TypeID;
		SFile >> TypeID;

		DxEffChar* pEffChar = DxEffCharMan::GetInstance().CreateEffInstance ( TypeID );
		if ( !pEffChar )	
		{
			CDebugSet::ErrorCode( "DxSkinPiece::LoadPiece_0200", "Unable to DxEffCharMan::GetInstance().CreateEffInstance  TypeID:%d", TypeID );
			return E_FAIL;
		}

		pEffChar->SetLinkObj ( NULL, this );
		hr = pEffChar->LoadFile ( SFile, pd3dDevice );
		if (FAILED(hr))
		{
			SAFE_DELETE(pEffChar);
			continue;
		}

		m_vecEFFECT.push_back ( pEffChar );
	}

	std::sort ( m_vecEFFECT.begin(), m_vecEFFECT.end(), DXEFFCHAR_OPER() );

	//flag
	SFile >> m_dwFlag;

	return S_OK;
}

HRESULT DxSkinPiece::LoadPiece_0201 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr(S_OK);

	SFile >> m_dwRef;

	DWORD dwSize;

	//skeleton
	SFile >> dwSize;
	char *szSkeleton = new char[dwSize];
	SFile.ReadBuffer ( szSkeleton, sizeof(char)*dwSize );

	//skin
	SFile >> dwSize;
	char *szXFileName = new char[dwSize];
	SFile.ReadBuffer ( szXFileName, sizeof(char)*dwSize );

	hr = SetSkinMesh ( szXFileName, szSkeleton, pd3dDevice, bThread );
	SAFE_DELETE_ARRAY(szXFileName);
	SAFE_DELETE_ARRAY(szSkeleton);
	if ( FAILED(hr) )	return hr;

	//mesh
	SFile >> dwSize;
	m_szMeshName = new char[dwSize];
	SFile.ReadBuffer ( m_szMeshName, sizeof(char)*dwSize );
	m_pmcMesh = m_pSkinMesh->FindMeshContainer ( m_szMeshName );
	if ( !m_pmcMesh )	return E_FAIL;

	//material
	SFile >> m_dwMaterialNum;
	if ( m_dwMaterialNum!=0 )
	{
		SAFE_DELETE_ARRAY(m_pMaterialPiece);

		m_pMaterialPiece = new SMATERIAL_PIECE[m_dwMaterialNum];
		for ( DWORD i=0; i<m_dwMaterialNum; ++i )
		{
			m_pMaterialPiece[i].LoadFile ( SFile, pd3dDevice );
		}
	}

	//piece type
	SFile >> dwSize;
	m_emType = (EMPIECECHAR) dwSize;

	//weapon where back
	SFile >> dwSize;
	m_emWeaponWhereBack = (EMPEACEZONEWEAPON) dwSize;

	//userslot
	m_dxUserSlot.LoadFile( SFile, pd3dDevice );

	//trace
	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		std::string strBuffer;
		SVERTEXINFLU sVertexInflu;
		SFile >> strBuffer;
		sVertexInflu.LoadFile ( SFile );

		m_vecTrace.push_back ( STRACOR(strBuffer.c_str(),sVertexInflu,D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	}

	std::sort ( m_vecTrace.begin(), m_vecTrace.end(), STRACOR_OPER() );

	//effect
	dwSize = 0;
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; i++ )
	{
		DWORD TypeID;
		SFile >> TypeID;

		DxEffChar* pEffChar = DxEffCharMan::GetInstance().CreateEffInstance ( TypeID );
		if ( !pEffChar )	
		{
			CDebugSet::ErrorCode( "DxSkinPiece::LoadPiece_0200", "Unable to DxEffCharMan::GetInstance().CreateEffInstance  TypeID:%d", TypeID );
			return E_FAIL;
		}

		pEffChar->SetLinkObj ( NULL, this );
		hr = pEffChar->LoadFile ( SFile, pd3dDevice );
		if (FAILED(hr))
		{
			SAFE_DELETE(pEffChar);
			continue;
		}

		m_vecEFFECT.push_back ( pEffChar );
	}

	std::sort ( m_vecEFFECT.begin(), m_vecEFFECT.end(), DXEFFCHAR_OPER() );

	//flag
	SFile >> m_dwFlag;

	return S_OK;
}

HRESULT DxSkinPiece::SavePiece ( const char* szFile )
{
	//	GASSERT(szFile);

	SAFE_DELETE_ARRAY(m_szFileName);
	int nStrLen = strlen(szFile)+1;
	m_szFileName = new char[nStrLen];
	StringCchCopy( m_szFileName, nStrLen, szFile );

	char szPathName[MAX_PATH] = "";
	StringCchCopy( szPathName, MAX_PATH, DxSkinPieceContainer::GetInstance().GetPath() );
	StringCchCat( szPathName, MAX_PATH, m_szFileName );

	if ( !m_szXFileName )	return E_FAIL;
	if ( !m_szSkeleton )	return E_FAIL;
	if ( !m_szMeshName )	return E_FAIL;

	CSerialFile SFile;
	if ( !SFile.OpenFile ( FOT_WRITE, (char*)szPathName ) )	return E_FAIL;

	SFile << (DWORD)VERSION_CPS;

	SFile.SetEncodeType( EMBYTECRYPT_PIECE );

	SFile << m_dwRef;

	DWORD dwSize;

	//skeleton
	dwSize = strlen(m_szSkeleton)+1;
	SFile << dwSize;
	SFile.WriteBuffer ( m_szSkeleton, sizeof(char)*dwSize );
	
	//skin
	dwSize = strlen(m_szXFileName)+1;
	SFile << dwSize;
	SFile.WriteBuffer ( m_szXFileName, sizeof(char)*dwSize );

	//mesh
	dwSize = strlen(m_szMeshName)+1;
	SFile << dwSize;
	SFile.WriteBuffer ( m_szMeshName, sizeof(char)*dwSize );
	
	//material
	SFile << m_dwMaterialNum;
	for ( DWORD i=0; i<m_dwMaterialNum; ++i )
	{
		m_pMaterialPiece[i].SaveFile ( SFile );
	}

	//piece type
	SFile << (DWORD)m_emType;

	//weapon where back
	SFile << (DWORD)m_emWeaponWhereBack;

	//userslot
	m_dxUserSlot.SaveFile( SFile );

	//trace
	SFile << DWORD(m_vecTrace.size());
	for ( size_t n=0; n<m_vecTrace.size(); ++n )
	{
		SFile << std::string(m_vecTrace[n].m_szName);
		m_vecTrace[n].m_sVertexInflu.SaveFile ( SFile );
	}
	
	//effchar
	SFile << (DWORD) m_vecEFFECT.size ();
	for ( size_t n=0; n<m_vecEFFECT.size(); ++n )
	{
		DxEffChar* pEffChar = m_vecEFFECT[n];
		pEffChar->SaveFile ( SFile );
	}

	//flag
	SFile << m_dwFlag;

	m_pSkinMesh->SaveNORMAL( m_szXFileName );


	return S_OK;
}