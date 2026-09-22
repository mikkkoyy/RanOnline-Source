#include "stdafx.h"
#include "DxSkinPieceContainer.h"

#include "../DxCommon/DxViewPort.h"
#include "../DxCommon/EditMeshs.h"
#include "../G-Logic/GLogic.h"
#include "../DxEffect/Char/DxEffChar.h"
#include "./DxSkinMeshMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DxSkinPiece::DxSkinPiece () :
	m_szFileName(NULL),
	m_dwRef(0),
	m_szXFileName(NULL),
	m_pSkinMesh(NULL),
	m_szSkeleton(NULL),
	m_pSkeleton(NULL),
	m_emType(PIECE_HEAD),
	m_emWeaponWhereBack(EMPEACE_WEAPON_RSLOT),
	m_szMeshName(NULL),
	m_dwFlags(NULL),
	m_dwFlag(0L),
	m_pmcMesh(NULL),
	m_dwVertexNUM(0),
	m_dwMaterialNum(0),
	m_pMaterialPiece(NULL)
{
	m_dwFlag |= EMCF_SHADOW;

	m_dxUserSlot = DXSUSER_SLOT();
}

DxSkinPiece::~DxSkinPiece ()
{
	SAFE_DELETE_ARRAY(m_szFileName);
	SAFE_DELETE_ARRAY(m_szXFileName);
	SAFE_DELETE_ARRAY(m_szSkeleton);
	SAFE_DELETE_ARRAY(m_szMeshName);

	SAFE_DELETE_ARRAY(m_pMaterialPiece);

	std::for_each ( m_vecEFFECT.begin(), m_vecEFFECT.end(), std_afunc::DeleteObject() );
	//m_vecEFFECT.erase ( m_vecEFFECT.begin(), m_vecEFFECT.end() );
	m_vecEFFECT.clear();

	// don't delete m_pmcMesh
}

void DxSkinPiece::SumEffFlag ()
{
	m_dwFlags = NULL;

	for ( size_t n=0; n<m_vecEFFECT.size(); ++n )
	{
		m_dwFlags |= m_vecEFFECT[n]->GetFlag();
	}
}

void DxSkinPiece::AddEffList ( DxEffChar* pEffChar )
{
	GASSERT(pEffChar);

	m_vecEFFECT.push_back ( pEffChar );
	std::sort ( m_vecEFFECT.begin(), m_vecEFFECT.end(), DXEFFCHAR_OPER() );

	//	flag가 지정된 효과의 경우 flag를 합산.
	m_dwFlags |= pEffChar->GetFlag();
}

void DxSkinPiece::DelEffList ( DxEffChar* pEffChar )
{
	if ( !pEffChar )	return;

	DWORD dwFlag = pEffChar->GetFlag();
	DWORD dwOrder = pEffChar->GetStateOrder();

	GLEFFCHAR_VEC_ITER pos = std::lower_bound ( m_vecEFFECT.begin(), m_vecEFFECT.end(), dwOrder, DXEFFCHAR_OPER() );
	for ( ; pos!=m_vecEFFECT.end(); ++pos )
	{
		DxEffChar* pEFFECT = (*pos);
		if ( pEFFECT->GetStateOrder() > dwOrder )	break;	//	존제하지 않는다.
		if ( pEFFECT!=pEffChar )					continue;

		//	삭제.
		SAFE_DELETE(pEFFECT);
		m_vecEFFECT.erase(pos);
		break;
	}

	//	flag가 있었던 효과가 삭제될 경우 flag를 다시 합산.
	if ( dwFlag )		SumEffFlag ();
}

int DxSkinPiece::EffUp( DWORD dwIndex )
{
	if( dwIndex == 0 )	return dwIndex;

	DxEffChar* pTEMP;
	int nCount(0);
	GLEFFCHAR_VEC_ITER iter = m_vecEFFECT.begin();
	for( ; iter!=m_vecEFFECT.end(); ++iter, ++nCount )
	{
		if( nCount==dwIndex )
		{
			pTEMP = (*iter);
			m_vecEFFECT.erase( iter );
			break;
		}
	}

	nCount = 0;
	dwIndex -= 1;

	iter = m_vecEFFECT.begin();
	for( ; iter!=m_vecEFFECT.end(); ++iter, ++nCount )
	{
		if( nCount==dwIndex )
		{
			m_vecEFFECT.insert( iter, pTEMP );
			return nCount;
		}
	}

	return 0;
}

int DxSkinPiece::EffDown( DWORD dwIndex )
{
	if( dwIndex >= (m_vecEFFECT.size()-1) )	return dwIndex;

	DxEffChar* pTEMP;
	int nCount(0);
	GLEFFCHAR_VEC_ITER iter = m_vecEFFECT.begin();
	for( ; iter!=m_vecEFFECT.end(); ++iter, ++nCount )
	{
		if( nCount==dwIndex )
		{
			pTEMP = (*iter);
			m_vecEFFECT.erase( iter );
			break;
		}
	}

	nCount = 0;
	dwIndex += 1;
	iter = m_vecEFFECT.begin();
	for( ; iter!=m_vecEFFECT.end(); ++iter, ++nCount )
	{
		if( nCount==dwIndex )
		{
			m_vecEFFECT.insert( iter, pTEMP );
			return nCount;
		}
	}

	m_vecEFFECT.push_back( pTEMP );

	return nCount;
}

HRESULT DxSkinPiece::SetSkinMesh ( const char* szXFileName, const char* szSkeleton, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	GASSERT(szXFileName);
	GASSERT(szSkeleton);
	GASSERT(pd3dDevice);

	SAFE_DELETE_ARRAY(m_szSkeleton);
	int nStrLen = strlen(szSkeleton)+1;
	m_szSkeleton = new char[nStrLen];
	StringCchCopy( m_szSkeleton, nStrLen, szSkeleton );
	m_pSkeleton = DxBoneCollector::GetInstance().Load ( m_szSkeleton, pd3dDevice );
	if ( !m_pSkeleton )		return E_FAIL;

	SAFE_DELETE_ARRAY(m_szXFileName);
	nStrLen = strlen(szXFileName)+1;
	m_szXFileName = new char[nStrLen];
	StringCchCopy( m_szXFileName, nStrLen, szXFileName );
	m_pSkinMesh = DxSkinMeshMan::GetInstance().LoadSkinMesh( m_szXFileName, m_szSkeleton, pd3dDevice, bThread );
	if ( !m_pSkinMesh )		return E_FAIL;

	return S_OK;
}

void DxSkinPiece::SelectMesh ( PSMESHCONTAINER pmcMesh )
{
	GASSERT(pmcMesh);

	m_pmcMesh = (PSMESHCONTAINER) pmcMesh;

	if( !m_pmcMesh->Name )	return;

	SAFE_DELETE_ARRAY(m_szMeshName);
	int nStrLen = strlen(m_pmcMesh->Name)+1;
	m_szMeshName = new char[nStrLen];
	StringCchCopy( m_szMeshName, nStrLen, m_pmcMesh->Name );

	m_pmcMesh->GetNumMaterials();
	if ( m_pmcMesh->GetNumMaterials() == m_dwMaterialNum && m_pMaterialPiece )
	{
		return;
	}

	//	Note : 초기화.
	//
	m_dwMaterialNum = 0;
	SAFE_DELETE_ARRAY(m_pMaterialPiece);

	m_dwMaterialNum = m_pmcMesh->GetNumMaterials();
	if ( m_dwMaterialNum==0 )
	{
		return;
	}

	m_pMaterialPiece = new SMATERIAL_PIECE[m_dwMaterialNum];

	return;
}

SVERTEXINFLU* DxSkinPiece::GetTracePos ( std::string strTrace )
{
	VECTRACE_ITER found = std::lower_bound ( m_vecTrace.begin(), m_vecTrace.end(), strTrace.c_str(),STRACOR_OPER() );
	if ( found == m_vecTrace.end() )						return NULL;

	if ( !strcmp((*found).m_szName,strTrace.c_str()) )		return &(*found).m_sVertexInflu;
	return NULL;
}

void DxSkinPiece::SetTracePos ( std::string strTrace, SVERTEXINFLU &sVertInflu )
{
	m_vecTrace.push_back ( STRACOR(strTrace.c_str(),sVertInflu,D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX)) );
	std::sort  ( m_vecTrace.begin(), m_vecTrace.end(), STRACOR_OPER() );
}

void DxSkinPiece::DelTracePos ( std::string strTrace )
{
	VECTRACE_ITER found = std::lower_bound ( m_vecTrace.begin(), m_vecTrace.end(), strTrace.c_str(), STRACOR_OPER() );
	if ( found==m_vecTrace.end() )	return;

	if ( !strcmp((*found).m_szName,strTrace.c_str()) )
	{
		m_vecTrace.erase ( found );
	}
}

HRESULT DxSkinPiece::InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	for ( size_t n=0; n<m_vecEFFECT.size(); ++n )
	{
		DxEffChar* pEffChar = m_vecEFFECT[n];
		pEffChar->InitDeviceObjects ( pd3dDevice );
	}

	return S_OK;
}

HRESULT DxSkinPiece::RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	for ( size_t n=0; n<m_vecEFFECT.size(); ++n )
	{
		DxEffChar* pEffChar = m_vecEFFECT[n];
		pEffChar->RestoreDeviceObjects ( pd3dDevice );
	}

	return S_OK;
}

HRESULT DxSkinPiece::InvalidateDeviceObjects ()
{
	for ( size_t n=0; n<m_vecEFFECT.size(); ++n )
	{
		DxEffChar* pEffChar = m_vecEFFECT[n];
		pEffChar->InvalidateDeviceObjects ();
	}

	return S_OK;
}

HRESULT DxSkinPiece::DeleteDeviceObjects ()
{
	for ( size_t n=0; n<m_vecEFFECT.size(); ++n )
	{
		DxEffChar* pEffChar = m_vecEFFECT[n];
		pEffChar->DeleteDeviceObjects ();
	}

	return S_OK;
}

HRESULT DxSkinPiece::FrameMove ( float fTime, float fElapsedTime )
{
	//	Note : 이팩트 업데이트.
	//
	for ( size_t n=0; n<m_vecEFFECT.size(); ++n )
	{
		DxEffChar* pEffChar = m_vecEFFECT[n];

		pEffChar->FrameMove ( fTime, fElapsedTime );

		//	Note : 종료시점 도달시에 Effect 제거.
		//if ( pEffChar->IsEnd() )
		//{
		//	SAFE_DELETE(pEFFECT);
		//	m_vecEFFECT.erase ( m_vecEFFECT.begin()+n );
		//  break;// 이럼.. 다음것을 못하지. 쩝.
		//
		//	if ( dwFlag )	SumEffFlag();
		//}
	}

	SumEffFlag();

	return S_OK;
}

HRESULT DxSkinPiece::DrawMeshContainer ( LPDIRECT3DDEVICEQ pd3dDevice, BOOL bOriginDraw, BOOL bAlpha, BOOL bShadow, BOOL bAlphaTex )
{
	if ( !m_pmcMesh )		return E_FAIL;

	HRESULT hr = S_OK;

	if ( m_pSkinMesh )
	{
		CHARSETTING sCharSetting;
		sCharSetting.pMeshContainerBase = m_pmcMesh;
		sCharSetting.pmtrlPiece			= m_pMaterialPiece;
		sCharSetting.pmtrlSpecular		= NULL;
		sCharSetting.bWorldIdentity		= FALSE;
		sCharSetting.emRDOP				= CTOP_BASE;

		m_pSkinMesh->SetDrawState ( bOriginDraw, bAlpha, bShadow, bAlphaTex );
		m_pSkinMesh->DrawMeshContainer ( pd3dDevice, sCharSetting );

		return S_OK;
	}

	return E_FAIL;
}




HRESULT DxSkinPiece::MeshEditRender ( LPDIRECT3DDEVICEQ pd3dDevice, D3DXMATRIX &matRot, DxSkeleton* pSkeleton )
{
	HRESULT hr = S_OK;
	if ( !m_pSkeleton || !m_pSkinMesh )		return S_FALSE;

	BOOL	bOriginDraw	= TRUE;
	BOOL	bAlphaTex	= TRUE;
	BOOL	bAlpha		= FALSE;
	BOOL bCrashRender(FALSE);

	if( (m_dwFlags&EMECF_NORMALMAP) && (m_dwFlags&EMECF_NONHLSL) )	bCrashRender = TRUE;	// 렌더시 충돌이 일어난다.


	if ( m_dwFlags & EMECF_ORGINNON )	bOriginDraw = FALSE;
	if ( m_dwFlags&(EMECF_NOALPHA|EMECF_ALPHA) )	bAlphaTex	= FALSE;
	if ( m_dwFlags&(EMECF_NOALPHA|EMECF_ALPHA) )	bAlpha		= TRUE;

	//	Note : 계층 메트릭스 업데이트.
	//
	if ( !pSkeleton )
	{
		m_pSkeleton->UpdateBones ( m_pSkeleton->pBoneRoot, matRot );
	}

	//	Note : 일반 메시일 경우 Bone이 틀릴 가능성을 고려함.
	//
	if ( !pSkeleton )	pSkeleton = m_pSkeleton;

	DWORD dwOldFill, dwOldLight;

	pd3dDevice->GetRenderState( D3DRS_FILLMODE, &dwOldFill );
	pd3dDevice->GetRenderState( D3DRS_LIGHTING, &dwOldLight );

	pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

	pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR,	0xffffffff );


	//	Note : 캐릭터 텍스쳐 파일에 스펙큘러 맵이 들어 있을 경우 1번만 뿌리기 위해서 기본 조각파일을 뿌리지 않는다.
	//
	if ( bOriginDraw )
	{
		//	Note : SkinMesh 선택부분 그리기.
		//
		if ( !m_pmcMesh )	return S_FALSE;



		D3DXMATRIX mat;
		D3DXMatrixIdentity( &mat );
		pd3dDevice->SetTransform( D3DTS_WORLD, &mat );

		LPD3DXBONECOMBINATION pBoneComb;
		LPD3DXBONECOMBINATION pBoneCur;
		UINT				  iMatrixIndex;
		pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>(m_pmcMesh->pBoneCombinationBuf->GetBufferPointer());


		for( DWORD iAttrib = 0; iAttrib < m_pmcMesh->NumAttributeGroups; iAttrib++ )
		{
			pBoneCur = &pBoneComb[iAttrib];

			for( DWORD i = 0; i < m_pmcMesh->NumInfl; i++ )
			{				
				iMatrixIndex = pBoneCur->BoneId[i];
				if (iMatrixIndex != UINT_MAX)
				{
					D3DXMatrixMultiply( &mat, &m_pmcMesh->pBoneOffsetMatrices[iMatrixIndex], m_pmcMesh->ppBoneMatrixPtrs[iMatrixIndex] );
					pd3dDevice->SetTransform( D3DTS_WORLDMATRIX( i ), &mat );
				}
			}

			//			D3DXMatrixMultiply( &mat, &m_pmcMesh->pBoneOffsetMatrices[i], m_pmcMesh->ppBoneMatrixPtrs[i] );
			m_pmcMesh->MeshData.pMesh->DrawSubset(iAttrib);
		}
	}

	pd3dDevice->SetRenderState( D3DRS_FILLMODE, dwOldFill );
	pd3dDevice->SetRenderState( D3DRS_LIGHTING, dwOldLight );

	return S_OK;
}

HRESULT DxSkinPiece::Render ( LPDIRECT3DDEVICEQ pd3dDevice, D3DXMATRIX &matRot, DxSkeleton* pSkeleton )
{
	HRESULT hr = S_OK;
	if ( !m_pSkeleton || !m_pSkinMesh )		return S_FALSE;

	BOOL	bOriginDraw	= TRUE;
	BOOL	bAlphaTex	= TRUE;
	BOOL	bAlpha		= FALSE;
	BOOL bCrashRender(FALSE);

	if( (m_dwFlags&EMECF_NORMALMAP) && (m_dwFlags&EMECF_NONHLSL) )	bCrashRender = TRUE;	// 렌더시 충돌이 일어난다.

	if ( m_dwFlags & EMECF_ORGINNON )	bOriginDraw = FALSE;
	if ( m_dwFlags&(EMECF_NOALPHA|EMECF_ALPHA) )	bAlphaTex	= FALSE;
	if ( m_dwFlags&(EMECF_NOALPHA|EMECF_ALPHA) )	bAlpha		= TRUE;

	//	Note : 계층 메트릭스 업데이트.
	//
	if ( !pSkeleton )
	{
		m_pSkeleton->UpdateBones ( m_pSkeleton->pBoneRoot, matRot );
	}

	//	Note : 일반 메시일 경우 Bone이 틀릴 가능성을 고려함.
	//
	if ( !pSkeleton )	pSkeleton = m_pSkeleton;

	if ( !m_pSkinMesh )
	{
		//if ( m_pmcMesh->m_numBoneComb == 3 )
		//{
		//	//	Note : 영향 받는 본의 matrix 포인터를 다시 찾는다.
		//	//

		//	//	m_pmcMesh->m_szBoneNames[0];	//	[LOCAL]
		//	//	m_pmcMesh->m_szBoneNames[1];	//	[LOCAL]
		//	//	m_pmcMesh->m_szBoneNames[2];	//	[COMBINED] <-- 이것만 실제로 BONE에서 영향을 받는것.

		//	DxBoneTrans* pBoneTrans = pSkeleton->FindBone ( m_pmcMesh->m_szBoneNames[2].c_str() );
		//	if ( pBoneTrans )	m_pmcMesh->m_pBoneMatrix[2] = &pBoneTrans->matCombined;
		//	else				m_pmcMesh->m_pBoneMatrix[2] = NULL;
		//}
	}

	DxEffChar*	pEffAmbient = NULL;
	DxEffChar*	pEffToon = NULL;

	//	Note : Setting State.
	//
	{
		SKINEFFDATA sSkinEffData;

		size_t nSIZE = m_vecEFFECT.size();
		for ( size_t n=0; n<nSIZE; ++n )
		{
			DxEffChar* pEFFECT = m_vecEFFECT[n];
			if ( pEFFECT->GetStateOrder() > EMEFFSO_RENDERSTATE )	break;

			pEFFECT->SettingState( pd3dDevice, sSkinEffData );
		}
	}

	//	Note : 캐릭터 텍스쳐 파일에 스펙큘러 맵이 들어 있을 경우 1번만 뿌리기 위해서 기본 조각파일을 뿌리지 않는다.
	//
	if ( bOriginDraw )
	{
		//	Note : SkinMesh 선택부분 그리기.
		//
		if ( !m_pmcMesh )	return S_FALSE;

		m_pSkinMesh->SetDrawState( bOriginDraw, bAlpha, FALSE, bAlphaTex );

		CHARSETTING sCharSetting;
		sCharSetting.pMeshContainerBase = m_pmcMesh;
		sCharSetting.pmtrlPiece			= m_pMaterialPiece;
		sCharSetting.pmtrlSpecular		= NULL;
		sCharSetting.bWorldIdentity		= TRUE;
		sCharSetting.emRDOP				= CTOP_BASE;

		m_pSkinMesh->DrawMeshContainer( pd3dDevice, sCharSetting );
	}

	//	Note : render effect skin
	//
	{
		GLEFFCHAR_VEC_ITER pos = std::lower_bound ( m_vecEFFECT.begin(), m_vecEFFECT.end(), EMEFFSO_NORMAL, DXEFFCHAR_OPER() );
		for ( ; pos!=m_vecEFFECT.end(); ++pos )
		{
			DxEffChar* pEFFECT = (*pos);
			if ( pEFFECT->GetStateOrder() > EMEFFSO_NORMAL )	break;	//	EMEFFSO_NORMAL이 아니면 중단.
			if ( pEFFECT->GetFlag() & EMECF_NORMALMAP )	continue;
			pEFFECT->Render( pd3dDevice, TRUE );
		}
	}

	//	Note : Setting State.
	//
	{
		GLEFFCHAR_VEC_RITER pos = std::lower_bound ( m_vecEFFECT.rbegin(), m_vecEFFECT.rend(), EMEFFSO_RENDERSTATE, DXEFFCHAR_DISOPER() );
		for ( ; pos!=m_vecEFFECT.rend(); ++pos )
		{
			DxEffChar* pEFFECT = (*pos);

			pEFFECT->RestoreState ( pd3dDevice );
		}
	}


	//	Note : Glow
	//
	{
		GLEFFCHAR_VEC_ITER pos = std::lower_bound ( m_vecEFFECT.begin(), m_vecEFFECT.end(), EMEFFSO_GLOW, DXEFFCHAR_OPER() );
		for ( ; pos!=m_vecEFFECT.end(); ++pos )
		{
			DxEffChar* pEFFECT = (*pos);
			if ( pEFFECT->GetStateOrder() > EMEFFSO_GLOW )	break;	//	EMEFFSO_GLOW이 아니면 중단.

			pEFFECT->Render( pd3dDevice, TRUE );
		}
	}

	//	Note : Single Eff
	//
	{
		GLEFFCHAR_VEC_ITER pos = std::lower_bound ( m_vecEFFECT.begin(), m_vecEFFECT.end(), EMEFFSO_SINGLE_EFF, DXEFFCHAR_OPER() );
		for ( ; pos!=m_vecEFFECT.end(); ++pos )
		{
			DxEffChar* pEFFECT = (*pos);
			if ( pEFFECT->GetStateOrder() > EMEFFSO_SINGLE_EFF )	break;	//	EMEFFSO_GLOW이 아니면 중단.

			pEFFECT->SetSkeleton( pSkeleton );
			pEFFECT->RenderEff( pd3dDevice, NULL, 1.f );
		}
	}

	return S_OK;
}

HRESULT DxSkinPiece::RenderTEST( LPDIRECT3DDEVICEQ pd3dDevice, D3DXMATRIX &matRot, DxSkeleton* pSkeleton )
{
	//	Note : Single Eff
	//
	{
		GLEFFCHAR_VEC_ITER pos = std::lower_bound ( m_vecEFFECT.begin(), m_vecEFFECT.end(), EMEFFSO_SINGLE_EFF, DXEFFCHAR_OPER() );
		for ( ; pos!=m_vecEFFECT.end(); ++pos )
		{
			DxEffChar* pEFFECT = (*pos);
			if ( pEFFECT->GetStateOrder() > EMEFFSO_SINGLE_EFF )	break;	//	EMEFFSO_GLOW이 아니면 중단.

			pEFFECT->RenderEDIT( pd3dDevice, NULL, 1.f );
		}
	}

	return S_OK;
}

HRESULT DxSkinPiece::DrawTraceVert ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( !m_pmcMesh )	return S_FALSE;


	VECTRACE_ITER iter = m_vecTrace.begin ();
	VECTRACE_ITER iter_end = m_vecTrace.end ();
	for ( ; iter!=iter_end; ++iter )
	{
		SVERTEXINFLU* pVertInflu = &((*iter).m_sVertexInflu);

		D3DXVECTOR3 vVecOutput;
		D3DXVECTOR3 vNorOutput;
		m_pmcMesh->CalculateVertexInfluences( pVertInflu, vVecOutput, vNorOutput, NULL, TRUE );

		pd3dDevice->SetRenderState( D3DRS_ZFUNC,		D3DCMP_ALWAYS );
		EDITMESHS::RENDERSPHERE ( pd3dDevice, vVecOutput, 0.2f );
		pd3dDevice->SetRenderState( D3DRS_ZFUNC,		D3DCMP_LESSEQUAL );
	}

	return S_OK;
}

HRESULT DxSkinPiece::DrawSelTraceVert ( LPDIRECT3DDEVICEQ pd3dDevice, const char* szSelVert )
{
	if ( !m_pmcMesh )	return S_FALSE;

	SVERTEXINFLU* pVertInflu = GetTracePos(szSelVert);
	if ( !pVertInflu )	return S_FALSE;

	D3DXVECTOR3 vVecOutput;
	D3DXVECTOR3 vNorOutput;

	m_pmcMesh->CalculateVertexInfluences ( pVertInflu, vVecOutput, vNorOutput, NULL, TRUE );

	pd3dDevice->SetRenderState( D3DRS_ZFUNC,		D3DCMP_ALWAYS );
	EDITMESHS::RENDERSPHERE ( pd3dDevice, vVecOutput, 0.35f );
	pd3dDevice->SetRenderState( D3DRS_ZFUNC,		D3DCMP_LESSEQUAL );

	return S_OK;
}

void DxSkinPiece::ClearAll ()
{
	SAFE_DELETE_ARRAY(m_szFileName);
	SAFE_DELETE_ARRAY(m_szXFileName);
	SAFE_DELETE_ARRAY(m_szSkeleton);
	SAFE_DELETE_ARRAY(m_szMeshName);

	SAFE_DELETE_ARRAY(m_pMaterialPiece);

	m_pSkinMesh = NULL;
	m_pSkeleton = NULL;

	m_emType = PIECE_HEAD;
	m_pmcMesh = NULL;

	m_vecTrace.clear ();

	m_dwFlags = NULL;

	std::for_each ( m_vecEFFECT.begin(), m_vecEFFECT.end(), std_afunc::DeleteObject() );
	//m_vecEFFECT.erase ( m_vecEFFECT.begin(), m_vecEFFECT.end() );
	m_vecEFFECT.clear();
	m_dwMaterialNum = 0;

	m_dxUserSlot = DXSUSER_SLOT();
}

HRESULT DxSkinPiece::CalculateBoundingBox ( LPDIRECT3DDEVICEQ pd3dDevice, D3DXVECTOR3 &vMax, D3DXVECTOR3 &vMin )
{
	if ( !m_pSkinMesh || !m_pmcMesh )		return E_FAIL;

	D3DXMATRIX matIdentity;
	D3DXMatrixIdentity ( &matIdentity );

	//	Note : 계층 메트릭스 업데이트.
	//
	if ( m_pSkeleton )
	{
		m_pSkeleton->UpdateBones ( m_pSkeleton->pBoneRoot, matIdentity );
	}

	HRESULT hr = m_pSkinMesh->CalculateBoundingBox ( pd3dDevice, m_pmcMesh, vMax, vMin );

	return S_OK;
}

//	pBoneMatrice -> (주의) 영향을 받는 본의 갯수가 여러개일 경우에 문제가 생김.
//
HRESULT DxSkinPiece::CalculateVertexInflu ( std::string strTrace, D3DXVECTOR3 &vVert, D3DXVECTOR3 &vNormal, LPD3DXMATRIX pBoneMatrice )
{
	SVERTEXINFLU* pVertInflu = GetTracePos(strTrace);
	if ( !pVertInflu )	return E_FAIL;
	if ( !m_pmcMesh )	return E_FAIL;

	////	TODO::
	////	Note : 일반 메시일 경우 Bone에 링크되어 있는걸 가정.
	////		연결 설정된 본과 다른 곳에 사용된다는걸 가정하여 본을 다시 연결	시켜줌.
	////
	//if ( !m_pmcMesh->pSkinInfo )
	//{
	//	//	Note : 영향 받는 본의 matrix 포인터를 다시 찾는다.
	//	//
	//	//	m_pmcMesh->m_szBoneNames[2];	//	[COMBINED] <-- 이것만 실제로 BONE에서 영향을 받는것.

	//	DxBoneTrans* pHandHeld = m_pSkeleton->FindBone ( m_pmcMesh->szBoneName );

	//	if ( pHandHeld )	m_pmcMesh->ppBoneMatrixPtrs[2] = &pHandHeld->matCombined;
	//	else				m_pmcMesh->ppBoneMatrixPtrs[2] = NULL;
	//}

	//return m_pmcMesh->CalculateVertexInfluences ( pVertInflu, vVert, vNormal, pBoneMatrice );

	if ( !m_pmcMesh->pSkinInfo )
	{
		vVert = pVertInflu->m_vVector;
		vNormal = pVertInflu->m_vNormal;

		return S_OK;
	}
	else
	{
		return m_pmcMesh->CalculateVertexInfluences ( pVertInflu, vVert, vNormal, pBoneMatrice, FALSE );
	}
}

void DxSkinPiece::CreateCartoonMesh( LPDIRECT3DDEVICEQ pd3dDevice, float fThickness, float fSizeADD )
{
	if( !m_pmcMesh )	return;
	m_pmcMesh->IsCartoon_CreateMesh( pd3dDevice, fThickness, fSizeADD );
}


DxSkinPieceContainer& DxSkinPieceContainer::GetInstance()
{
	static DxSkinPieceContainer Instance;
	return Instance;
}

DxSkinPieceContainer::DxSkinPieceContainer() :
	m_pd3dDevice(NULL)
{
	memset( m_szPath, 0, sizeof(char)*MAX_PATH );
}

DxSkinPieceContainer::~DxSkinPieceContainer()
{
}

DxSkinPiece* DxSkinPieceContainer::LoadPiece ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread )
{
	HRESULT hr;
	GASSERT(szFile);

	DxSkinPiece* pSkinPiece = NULL;

	pSkinPiece = FindPiece ( szFile );
	if ( pSkinPiece )	return pSkinPiece;

	//	Note : SkinPiece Load 수행.
	//
	pSkinPiece = new DxSkinPiece;

	hr = pSkinPiece->LoadPiece( szFile, pd3dDevice, bThread );
	if ( FAILED(hr) )
	{
		SAFE_DELETE(pSkinPiece);
		return NULL;
	}

	if( pSkinPiece->m_szFileName )
	{
		int nStrLen = strlen(pSkinPiece->m_szFileName)+1;
		StringCchCopy( pSkinPiece->m_szFileName, nStrLen, szFile );

		//	Note : 리스트에 삽입.
		m_mapSkinPiece[std::string(pSkinPiece->m_szFileName)] = pSkinPiece;
	}
	else	
	{
		return NULL;
	}

	return pSkinPiece;
}

HRESULT DxSkinPieceContainer::ReleasePiece ( const char* szFile )
{
	GASSERT(szFile);

	//	Note : 만약 전역 객채 파괴 순서상에서 후순위일때 에러 유발 가능성을 막음.
	if ( m_mapSkinPiece.empty() )		return E_FAIL;

	PIECEMAP_ITER iter = m_mapSkinPiece.find ( std::string(szFile) );
	if ( iter==m_mapSkinPiece.end() )	return E_FAIL;

	iter->second->m_dwRef--;
	if ( iter->second->m_dwRef == 0 )
	{
		delete iter->second;
		m_mapSkinPiece.erase ( iter );
	}

	return S_OK;
}

HRESULT DxSkinPieceContainer::DeletePiece ( const char* szFile )
{
	if ( !szFile )				return S_FALSE;

	PIECEMAP_ITER iter = m_mapSkinPiece.find ( std::string(szFile) );
	if ( iter==m_mapSkinPiece.end() )	return E_FAIL;

	delete iter->second;
	m_mapSkinPiece.erase ( iter );

	return S_OK;
}

DxSkinPiece* DxSkinPieceContainer::FindPiece ( const char* szFile )
{
	GASSERT(szFile);

	PIECEMAP_ITER iter = m_mapSkinPiece.find ( std::string(szFile) );
	if ( iter!=m_mapSkinPiece.end() )	return iter->second;

	return NULL;
}

HRESULT DxSkinPieceContainer::InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	m_pd3dDevice = pd3dDevice;

	PIECEMAP_ITER iter = m_mapSkinPiece.begin();
	PIECEMAP_ITER iter_end = m_mapSkinPiece.end();

	for ( ; iter!=iter_end; iter++ )
	{
		iter->second->InitDeviceObjects ( pd3dDevice );
	}

	return S_OK;
}

HRESULT DxSkinPieceContainer::RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	PIECEMAP_ITER iter = m_mapSkinPiece.begin();
	PIECEMAP_ITER iter_end = m_mapSkinPiece.end();

	for ( ; iter!=iter_end; iter++ )
	{
		iter->second->RestoreDeviceObjects ( pd3dDevice );
	}

	return S_OK;
}

HRESULT DxSkinPieceContainer::InvalidateDeviceObjects ()
{
	PIECEMAP_ITER iter = m_mapSkinPiece.begin();
	PIECEMAP_ITER iter_end = m_mapSkinPiece.end();

	for ( ; iter!=iter_end; iter++ )
	{
		iter->second->InvalidateDeviceObjects ();
	}

	return S_OK;
}

HRESULT DxSkinPieceContainer::DeleteDeviceObjects()
{
	PIECEMAP_ITER iter = m_mapSkinPiece.begin();
	PIECEMAP_ITER iter_end = m_mapSkinPiece.end();

	for ( ; iter!=iter_end; iter++ )
	{
		iter->second->DeleteDeviceObjects ();
	}

	return S_OK;
}

HRESULT DxSkinPieceContainer::FinalCleanup ()
{
	std::for_each ( m_mapSkinPiece.begin(), m_mapSkinPiece.end(), std_afunc::DeleteMapObject() );
	m_mapSkinPiece.clear ();

	return S_OK;
}

HRESULT DxSkinPieceContainer::CleanUp ()
{
	InvalidateDeviceObjects ();
	DeleteDeviceObjects ();
	FinalCleanup ();

	return S_OK;
}


