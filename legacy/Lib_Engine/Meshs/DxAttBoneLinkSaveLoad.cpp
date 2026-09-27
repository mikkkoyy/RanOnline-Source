#include "stdafx.h"
#include "../Common/SerialFile.h"
#include "../G-Logic/GLogic.h"
#include "DxAttBoneLink.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL DxAttBoneLink::SaveFile ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	char szPathName[MAX_PATH] = "";
	StringCchCopy( szPathName, MAX_PATH, DxAttBoneLinkContainer::GetInstance().GetPath() );
	StringCchCat( szPathName, MAX_PATH, szFile );

	CSerialFile SFile;
	if ( !SFile.OpenFile ( FOT_WRITE, szPathName ) )		return FALSE;

	SFile << (DWORD)VERSION_ABL;

	SFile.SetEncodeType( EMBYTECRYPT_CONTAINER );

	SFile << (DWORD)m_emPieceStrike;
	SFile << (DWORD)m_emPieceType;
	SFile << (DWORD)m_emWeaponWhereBack;

	SFile << m_strMaskPiece;
	SFile << m_strAttBoneData;
	SFile << m_strSkeleton;
	SFile << m_strBoneTrans;
	
	SFile.WriteBuffer( &m_affBoneTrans, sizeof(DXAFFINEPARTS) );
	SFile.WriteBuffer( &m_affPeaceZone, sizeof(DXAFFINEPARTS) );
	
	return TRUE;
}

BOOL DxAttBoneLink::LOAD_Ver_0201( basestream &SFile )
{
	DWORD dwData(0);
	SFile >> dwData;	m_emPieceStrike = (EMPIECECHAR) dwData;
	SFile >> dwData;	m_emPieceType = (EMPIECECHAR) dwData;
	SFile >> dwData;	m_emWeaponWhereBack = (EMPEACEZONEWEAPON) dwData;

	SFile >> m_strMaskPiece;
	SFile >> m_strAttBoneData;
	SFile >> m_strSkeleton;
	SFile >> m_strBoneTrans;

	SFile.ReadBuffer( &m_affBoneTrans, sizeof(DXAFFINEPARTS) );
	SFile.ReadBuffer( &m_affPeaceZone, sizeof(DXAFFINEPARTS) );

	if ( m_affBoneTrans.vScale.x < 0.0f )	m_affBoneTrans.vScale.x = -m_affBoneTrans.vScale.x;
	if ( m_affBoneTrans.vScale.y < 0.0f )	m_affBoneTrans.vScale.y = -m_affBoneTrans.vScale.y;
	if ( m_affBoneTrans.vScale.z < 0.0f )	m_affBoneTrans.vScale.z = -m_affBoneTrans.vScale.z;
	if ( m_affPeaceZone.vScale.x < 0.0f )	m_affPeaceZone.vScale.x = -m_affPeaceZone.vScale.x;
	if ( m_affPeaceZone.vScale.y < 0.0f )	m_affPeaceZone.vScale.y = -m_affPeaceZone.vScale.y;
	if ( m_affPeaceZone.vScale.z < 0.0f )	m_affPeaceZone.vScale.z = -m_affPeaceZone.vScale.z;

	return TRUE;
}

BOOL DxAttBoneLink::LOAD_Ver_0200( basestream &SFile )
{
	DWORD dwData(0);
	SFile >> dwData;	m_emPieceStrike = PIECE_0116_TO_PIECE( (EMPIECECHAR_0116)dwData );
	SFile >> dwData;	m_emPieceType = PIECE_0116_TO_PIECE( (EMPIECECHAR_0116)dwData );
	SFile >> dwData;	m_emWeaponWhereBack = (EMPEACEZONEWEAPON) dwData;

	SFile >> m_strMaskPiece;
	SFile >> m_strAttBoneData;
	SFile >> m_strSkeleton;
	SFile >> m_strBoneTrans;

	SFile.ReadBuffer( &m_affBoneTrans, sizeof(DXAFFINEPARTS) );
	SFile.ReadBuffer( &m_affPeaceZone, sizeof(DXAFFINEPARTS) );

	if ( m_affBoneTrans.vScale.x < 0.0f )	m_affBoneTrans.vScale.x = -m_affBoneTrans.vScale.x;
	if ( m_affBoneTrans.vScale.y < 0.0f )	m_affBoneTrans.vScale.y = -m_affBoneTrans.vScale.y;
	if ( m_affBoneTrans.vScale.z < 0.0f )	m_affBoneTrans.vScale.z = -m_affBoneTrans.vScale.z;
	if ( m_affPeaceZone.vScale.x < 0.0f )	m_affPeaceZone.vScale.x = -m_affPeaceZone.vScale.x;
	if ( m_affPeaceZone.vScale.y < 0.0f )	m_affPeaceZone.vScale.y = -m_affPeaceZone.vScale.y;
	if ( m_affPeaceZone.vScale.z < 0.0f )	m_affPeaceZone.vScale.z = -m_affPeaceZone.vScale.z;

	return TRUE;
}

BOOL DxAttBoneLink::LOAD_Ver_0102( basestream &SFile )
{
	DWORD dwData(0);

	SFile >> dwData;
	SFile >> m_strAttBoneData;
	SFile >> m_strMaskPiece;
	SFile >> m_strSkeleton;
	SFile >> dwData;

	SFile >> dwData;
	m_emPieceStrike = PIECE_0113_TO_PIECE( (EMPIECECHAR_0113)dwData );

	SFile >> dwData;
	m_emPieceType = PIECE_0113_TO_PIECE( (EMPIECECHAR_0113)dwData );

	DWORD dwVER;
	DWORD dwSIZE;

	SFile >> dwVER;
	SFile >> dwSIZE;

	if ( dwVER == 0x0100 )
	{
		SFile >> m_strBoneTrans;

		SFile >> dwData;	
		m_emWeaponWhereBack = (EMPEACEZONEWEAPON) dwData;

		//official changed the calculation, sometimes incorrect 
		SFile.ReadBuffer( &m_affBoneTrans, sizeof(DXAFFINEPARTS) );
		SFile.ReadBuffer( &m_affPeaceZone, sizeof(DXAFFINEPARTS) );

		if ( m_affBoneTrans.vScale.x < 0.0f )	m_affBoneTrans.vScale.x = -m_affBoneTrans.vScale.x;
		if ( m_affBoneTrans.vScale.y < 0.0f )	m_affBoneTrans.vScale.y = -m_affBoneTrans.vScale.y;
		if ( m_affBoneTrans.vScale.z < 0.0f )	m_affBoneTrans.vScale.z = -m_affBoneTrans.vScale.z;
		if ( m_affPeaceZone.vScale.x < 0.0f )	m_affPeaceZone.vScale.x = -m_affPeaceZone.vScale.x;
		if ( m_affPeaceZone.vScale.y < 0.0f )	m_affPeaceZone.vScale.y = -m_affPeaceZone.vScale.y;
		if ( m_affPeaceZone.vScale.z < 0.0f )	m_affPeaceZone.vScale.z = -m_affPeaceZone.vScale.z;

		SFile >> dwData;

		D3DXMATRIX mat;
		SFile.ReadBuffer( &mat, sizeof(D3DXMATRIX) );
	}
	else
	{
		SFile.SetOffSet ( SFile.GetfTell()+dwSIZE );
		CDebugSet::ErrorVersion( "DxAttBoneLink::LOAD_Ver102", SFile.GetFileName(), dwVER );
	}

	return TRUE;
}

BOOL DxAttBoneLink::LOAD_Ver_0101( basestream &SFile )
{
	DWORD dwData(0);

	SFile >> dwData;
	SFile >> m_strAttBoneData;
	SFile >> m_strMaskPiece;
	SFile >> m_strSkeleton;
	SFile >> m_strBoneTrans;
	SFile >> dwData;

	SFile >> dwData;
	m_emPieceStrike = PIECE_0113_TO_PIECE( (EMPIECECHAR_0113)dwData );

	SFile >> dwData;
	m_emPieceType = PIECE_0113_TO_PIECE( (EMPIECECHAR_0113)dwData );

	SFile >> dwData;	
	m_emWeaponWhereBack = (EMPEACEZONEWEAPON) dwData;

	SFile.ReadBuffer( &m_affBoneTrans, sizeof(DXAFFINEPARTS) );
	SFile.ReadBuffer( &m_affPeaceZone, sizeof(DXAFFINEPARTS) );

	if ( m_affBoneTrans.vScale.x < 0.0f )	m_affBoneTrans.vScale.x = -m_affBoneTrans.vScale.x;
	if ( m_affBoneTrans.vScale.y < 0.0f )	m_affBoneTrans.vScale.y = -m_affBoneTrans.vScale.y;
	if ( m_affBoneTrans.vScale.z < 0.0f )	m_affBoneTrans.vScale.z = -m_affBoneTrans.vScale.z;
	if ( m_affPeaceZone.vScale.x < 0.0f )	m_affPeaceZone.vScale.x = -m_affPeaceZone.vScale.x;
	if ( m_affPeaceZone.vScale.y < 0.0f )	m_affPeaceZone.vScale.y = -m_affPeaceZone.vScale.y;
	if ( m_affPeaceZone.vScale.z < 0.0f )	m_affPeaceZone.vScale.z = -m_affPeaceZone.vScale.z;


	return TRUE;
}

BOOL DxAttBoneLink::LOAD_Ver_0100( basestream &SFile )
{
	DWORD dwData(0);

	SFile >> dwData;
	SFile >> m_strAttBoneData;
	SFile >> m_strMaskPiece;
	SFile >> m_strSkeleton;
	SFile >> m_strBoneTrans;
	SFile >> dwData;

	SFile >> dwData;
	m_emPieceType = PIECE_0113_TO_PIECE( (EMPIECECHAR_0113)dwData );

	SFile >> dwData;	
	m_emWeaponWhereBack = (EMPEACEZONEWEAPON) dwData;

	SFile.ReadBuffer( &m_affBoneTrans, sizeof(DXAFFINEPARTS) );
	SFile.ReadBuffer( &m_affPeaceZone, sizeof(DXAFFINEPARTS) );

	if ( m_affBoneTrans.vScale.x < 0.0f )	m_affBoneTrans.vScale.x = -m_affBoneTrans.vScale.x;
	if ( m_affBoneTrans.vScale.y < 0.0f )	m_affBoneTrans.vScale.y = -m_affBoneTrans.vScale.y;
	if ( m_affBoneTrans.vScale.z < 0.0f )	m_affBoneTrans.vScale.z = -m_affBoneTrans.vScale.z;
	if ( m_affPeaceZone.vScale.x < 0.0f )	m_affPeaceZone.vScale.x = -m_affPeaceZone.vScale.x;
	if ( m_affPeaceZone.vScale.y < 0.0f )	m_affPeaceZone.vScale.y = -m_affPeaceZone.vScale.y;
	if ( m_affPeaceZone.vScale.z < 0.0f )	m_affPeaceZone.vScale.z = -m_affPeaceZone.vScale.z;

	return TRUE;
}

BOOL DxAttBoneLink::LoadFile ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	std::string strCheckFileExt = szFile;
	std::transform ( strCheckFileExt.begin(), strCheckFileExt.end(), strCheckFileExt.begin(), std::tolower );
	std::string::size_type idx = strCheckFileExt.find( ".abl" );
	if ( idx == std::string::npos )	return FALSE;

	char szPathName[MAX_PATH] = "";
	StringCchCopy( szPathName, MAX_PATH, DxAttBoneLinkContainer::GetInstance().GetPath() );
	StringCchCat( szPathName, MAX_PATH, szFile );

	std::auto_ptr<basestream> pBStream( GLOGIC::openfile_basestream(GLOGIC::bENGLIB_ZIPFILE, 
										GLOGIC::strSKINOBJ_ZIPFILE.c_str(), 
										szPathName, 
										szFile ) );

	if ( !pBStream.get() )	
	{
		if ( strlen( szFile ) > 0 )
			CDebugSet::ErrorFile( "DxAttBoneLink::LoadFile", szFile );

		return FALSE;
	}

	basestream &SFile = *pBStream;

	DWORD dwVer;
	SFile >> dwVer;

	if ( dwVer >= VERSION_ENCRYPT )
		SFile.SetEncodeType( EMBYTECRYPT_CONTAINER );

	switch ( dwVer )
	{
	case VERSION_ABL:
		LOAD_Ver_0201( SFile );
		break;

	case 0x0200:
		LOAD_Ver_0200( SFile );
		break;

	case 0x0102:
		LOAD_Ver_0102( SFile );
		break;

	case 0x0101:
		LOAD_Ver_0101( SFile );
		break;

	case 0x0100:
		LOAD_Ver_0100( SFile );
		break;

	default:
		{
			CDebugSet::ErrorVersion( "DxAttBoneLink::LoadFile", SFile.GetFileName(), dwVer );
			return FALSE;
		}break;
	};

	DxSkeleton*	pSkeleton = DxBoneCollector::GetInstance().Load ( m_strSkeleton.c_str(), pd3dDevice );
	if ( !pSkeleton )	return FALSE;

	DxBoneTrans* pBoneTrans = pSkeleton->FindBone( m_strBoneTrans.c_str() );
	if ( !pBoneTrans )	return FALSE;

	DxAttBoneData*	pAttBoneData = DxAttBoneDataContainer::GetInstance().LoadData( m_strAttBoneData.c_str(), pd3dDevice, bThread );
	if ( !pAttBoneData )	return FALSE;

	if ( m_strMaskPiece.size() )
	{
		DxSkinPiece* pSkinPiece = DxSkinPieceContainer::GetInstance().LoadPiece( m_strMaskPiece.c_str(), pd3dDevice, bThread );
		if ( !pSkinPiece )
			m_strMaskPiece = "";
	}
	
	m_pAttBoneData = pAttBoneData;

	StringCchCopy( m_szFileName, FILE_LENGTH, szFile );

	return TRUE;
}