#include "stdafx.h"
#include "../Common/SerialFile.h"
#include "../G-Logic/GLogic.h"
#include "DxAttBoneLink.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DxAttBoneLink::DxAttBoneLink(void):
	m_pAttBoneData( NULL ),
	m_dwRef(0),
	m_emPieceStrike(PIECE_HEAD),
	m_emPieceType(PIECE_HEAD),
	m_emWeaponWhereBack(EMPEACE_WEAPON_RSLOT)
{
	memset( m_szFileName, 0, FILE_LENGTH );
	m_strAttBoneData = "";
	m_strMaskPiece = "";
	m_strSkeleton = "";
	m_strBoneTrans = "";

	m_affBoneTrans = DXAFFINEPARTS();
	m_affPeaceZone = DXAFFINEPARTS();
}

DxAttBoneLink::~DxAttBoneLink(void)
{
	if ( m_pAttBoneData )
	{
		DxAttBoneDataContainer::GetInstance().ReleaseData( m_pAttBoneData->GetFileName() );
		m_pAttBoneData = NULL;
	}
}

void DxAttBoneLink::SetData( DxAttBoneData* pAttBoneData )
{
	if( !pAttBoneData )	return;

	if ( m_pAttBoneData && strcmp( m_pAttBoneData->GetFileName(), pAttBoneData->GetFileName() ) != 0 )
	{
		DxAttBoneDataContainer::GetInstance().ReleaseData( m_pAttBoneData->GetFileName() );
		m_pAttBoneData = NULL;
	}

	m_pAttBoneData = pAttBoneData;
	m_strAttBoneData = m_pAttBoneData->GetFileName();
}

DxAttBoneLinkContainer::DxAttBoneLinkContainer ()
{
	memset( m_szPath, 0, sizeof(char)*MAX_PATH );
}

DxAttBoneLinkContainer::~DxAttBoneLinkContainer ()
{
}

DxAttBoneLinkContainer& DxAttBoneLinkContainer::GetInstance()
{
	static DxAttBoneLinkContainer Instance;
	return Instance;
}

HRESULT DxAttBoneLinkContainer::CleanUp ()
{
	ATTBONELINKMAP_ITER iter = m_mapAttBoneLink.begin ();
	ATTBONELINKMAP_ITER iterEnd = m_mapAttBoneLink.end ();
	
	for ( ; iter!=iterEnd; iter++ )
	{
		delete (*iter).second;
	}

	m_mapAttBoneLink.clear ();

	return S_OK;
}

DxAttBoneLink* DxAttBoneLinkContainer::FindData ( const char* szFile )
{
	ATTBONELINKMAP_ITER iter = m_mapAttBoneLink.find ( std::string(szFile) );
	if ( iter != m_mapAttBoneLink.end() )
	{
		return iter->second;
	}

	return NULL;
}

BOOL DxAttBoneLinkContainer::DeleteData ( const char* szFile )
{
	ATTBONELINKMAP_ITER iter = m_mapAttBoneLink.find ( std::string(szFile) );
	if ( iter == m_mapAttBoneLink.end() )	return FALSE;

	delete (*iter).second;
	m_mapAttBoneLink.erase ( iter );

	return TRUE;
}

void DxAttBoneLinkContainer::ReleaseData ( const char* szFile )
{
	DxAttBoneLink* pRes = FindData ( szFile );
	if ( !pRes )	return;
	
	if ( pRes->m_dwRef<=1 )
	{
		DeleteData ( szFile );
		return;
	}

	pRes->m_dwRef--;
	return;
}

DxAttBoneLink* DxAttBoneLinkContainer::LoadData( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	if ( !szFile ) return NULL;
	if ( strlen(szFile) == 0 ) return NULL;

	DxAttBoneLink* pBoneLink;
	pBoneLink = FindData( szFile );
	if ( pBoneLink )
	{
		pBoneLink->m_dwRef++;
		return pBoneLink;
	}

	pBoneLink = new DxAttBoneLink;
	BOOL bOk = pBoneLink->LoadFile( szFile, pd3dDevice, bThread );
	if ( !bOk )
	{
		SAFE_DELETE(pBoneLink);
		return NULL;
	}

	pBoneLink->m_dwRef++;

	CDebugSet::ToListView ( "DxAttBoneLink::LoadFile, %s", szFile );

	m_mapAttBoneLink[std::string(pBoneLink->GetFileName())] = pBoneLink;

	return pBoneLink;
}