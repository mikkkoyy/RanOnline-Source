#include "stdafx.h"
#include <algorithm>
#include "./dxeffchardata.h"
#include "../G-Logic/GLogic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*ABL system, Juver, 2017/06/02 */
BOOL DxEffCharDataArray::SetAttBone ( DxAttBone *pAttBone, LPDIRECT3DDEVICEQ pd3dDevice, D3DXVECTOR3 *pDir, float fTarDir )
{
	for ( int i=0; i<PIECE_SIZE; i++ )
	{
		PDXCHARPART pCharPart = pAttBone->GetPiece(i);
		if ( pCharPart && pCharPart->m_pmcMesh && m_pEffCharDataArray[i] )
		{
			m_pEffCharDataArray[i]->SetPart ( pCharPart, pd3dDevice, pDir, fTarDir );
		}
	}

	return TRUE;
}

/*ABL system, Juver, 2017/06/02 */
BOOL DxEffCharDataArray::SetAttBone ( DxAttBoneRender *pAttBone, LPDIRECT3DDEVICEQ pd3dDevice, D3DXVECTOR3 *pDir, float fTarDir )
{
	for ( int i=0; i<PIECE_SIZE; i++ )
	{
		PDXCHARPART pCharPart = pAttBone->GetPiece(i);
		if ( pCharPart && pCharPart->m_pmcMesh && m_pEffCharDataArray[i] )
		{
			m_pEffCharDataArray[i]->SetPart ( pCharPart, pd3dDevice, pDir, fTarDir );
		}
	}

	return TRUE;
}


/*ABL system, Juver, 2017/06/02 */
BOOL DxEffCharDataArray::SetAttBoneData ( DxAttBoneData *pAttBoneData, LPDIRECT3DDEVICEQ pd3dDevice )
{
	for ( int i=0; i<PIECE_SIZE; i++ )
	{
		PDXSKINPIECE pSkinPiece = pAttBoneData->GetPiece(i);
		if ( pSkinPiece && m_pEffCharDataArray[i] )
		{
			m_pEffCharDataArray[i]->SetPiece ( pSkinPiece, pd3dDevice );
		}
	}

	return TRUE;
}

/*vehicle system, Juver, 2017/08/06 */
BOOL DxEffCharDataArray::SetVehicle ( DxVehicle *pVehicle, LPDIRECT3DDEVICEQ pd3dDevice, D3DXVECTOR3 *pDir, float fTarDir )
{
	for ( int i=0; i<PIECE_SIZE; i++ )
	{
		PDXCHARPART pCharPart = pVehicle->GetPiece(i);
		if ( pCharPart && pCharPart->m_pmcMesh && m_pEffCharDataArray[i] )
		{
			m_pEffCharDataArray[i]->SetPart ( pCharPart, pd3dDevice, pDir, fTarDir );
		}
	}

	return TRUE;
}

/*vehicle system, Juver, 2017/08/06 */
BOOL DxEffCharDataArray::SetVehicleData ( DxVehicleData *pVehicleData, LPDIRECT3DDEVICEQ pd3dDevice )
{
	for ( int i=0; i<PIECE_SIZE; i++ )
	{
		PDXSKINPIECE pSkinPiece = pVehicleData->GetPiece(i);
		if ( pSkinPiece && m_pEffCharDataArray[i] )
		{
			m_pEffCharDataArray[i]->SetPiece ( pSkinPiece, pd3dDevice );
		}
	}

	return TRUE;
}


/*ABL system, Juver, 2017/06/02 */
BOOL DxEffcharDataMan::PutEffect ( DxAttBone *pAttBone, const char* szEffFile, D3DXVECTOR3 *pDir, float fTarDir )
{
	if ( !pAttBone )	return FALSE;

	DxEffCharDataArray *pEffCharDataArray = NULL;

	EFFDATA_ARRAYMAP_ITER iter = m_mapEffDataArray.find ( std::string(szEffFile) );
	if ( iter == m_mapEffDataArray.end() )
	{
		pEffCharDataArray = new DxEffCharDataArray;
		BOOL bOk = pEffCharDataArray->LoadFile ( szEffFile );
		if ( !bOk )
		{
			delete pEffCharDataArray;
			return FALSE;
		}

		m_mapEffDataArray[std::string(szEffFile)] = pEffCharDataArray;
	}
	else
	{
		pEffCharDataArray = iter->second;
	}

	pEffCharDataArray->SetAttBone ( pAttBone, m_pd3dDevice, pDir, fTarDir );

	return TRUE;
}

/*ABL system, Juver, 2017/06/02 */
BOOL DxEffcharDataMan::PutPassiveEffect ( DxAttBone *pAttBone, const char* szEffFile, D3DXVECTOR3 *pDir, float fTarDir )
{
	if ( !pAttBone )	return FALSE;
	if ( !szEffFile || szEffFile[0]==NULL )	return E_FAIL;

	DxEffCharDataArray *pEffCharDataArray = NULL;

	EFFDATA_ARRAYMAP_ITER iter = m_mapEffDataArray.find ( std::string(szEffFile) );
	if ( iter == m_mapEffDataArray.end() )
	{
		pEffCharDataArray = new DxEffCharDataArray;
		BOOL bOk = pEffCharDataArray->LoadFile ( szEffFile );
		if ( !bOk )
		{
			delete pEffCharDataArray;
			return FALSE;
		}

		m_mapEffDataArray[std::string(szEffFile)] = pEffCharDataArray;
	}
	else
	{
		pEffCharDataArray = iter->second;
	}

	//	Note : 지속 이팩트의 경우 파트가 변경되어 이팩트가 삭제될 경우를 대비해서 개별 확인후 삽입 해준다.
	//
	for ( int i=0; i<PIECE_SIZE; i++ )
	{
		PDXCHARPART pCharPart = pAttBone->GetPiece(i);
		BOOL bFind = pCharPart->FindEffList ( szEffFile );
		if ( !bFind )
		{
			pEffCharDataArray->SetSkinPart ( pCharPart, m_pd3dDevice, pDir, fTarDir );
		}
	}

	return TRUE;
}

/*ABL system, Juver, 2017/06/02 */
void DxEffcharDataMan::OutEffect ( DxAttBone *pAttBone, std::string strEffFile )
{
	if ( !pAttBone )	return;

	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		PDXCHARPART pCharPart = pAttBone->GetPiece(i);
		pCharPart->DelEffList ( strEffFile.c_str() );
	}
}

/*ABL system, Juver, 2017/06/02 */
BOOL DxEffcharDataMan::PutEffect ( DxAttBoneRender *pAttBone, const char* szEffFile, D3DXVECTOR3 *pDir, float fTarDir )
{
	if ( !pAttBone )	return FALSE;

	DxEffCharDataArray *pEffCharDataArray = NULL;

	EFFDATA_ARRAYMAP_ITER iter = m_mapEffDataArray.find ( std::string(szEffFile) );
	if ( iter == m_mapEffDataArray.end() )
	{
		pEffCharDataArray = new DxEffCharDataArray;
		BOOL bOk = pEffCharDataArray->LoadFile ( szEffFile );
		if ( !bOk )
		{
			delete pEffCharDataArray;
			return FALSE;
		}

		m_mapEffDataArray[std::string(szEffFile)] = pEffCharDataArray;
	}
	else
	{
		pEffCharDataArray = iter->second;
	}

	pEffCharDataArray->SetAttBone ( pAttBone, m_pd3dDevice, pDir, fTarDir );

	return TRUE;
}

/*ABL system, Juver, 2017/06/02 */
BOOL DxEffcharDataMan::PutPassiveEffect ( DxAttBoneRender *pAttBone, const char* szEffFile, D3DXVECTOR3 *pDir, float fTarDir )
{
	if ( !pAttBone )	return FALSE;
	if ( !szEffFile || szEffFile[0]==NULL )	return E_FAIL;

	DxEffCharDataArray *pEffCharDataArray = NULL;

	EFFDATA_ARRAYMAP_ITER iter = m_mapEffDataArray.find ( std::string(szEffFile) );
	if ( iter == m_mapEffDataArray.end() )
	{
		pEffCharDataArray = new DxEffCharDataArray;
		BOOL bOk = pEffCharDataArray->LoadFile ( szEffFile );
		if ( !bOk )
		{
			delete pEffCharDataArray;
			return FALSE;
		}

		m_mapEffDataArray[std::string(szEffFile)] = pEffCharDataArray;
	}
	else
	{
		pEffCharDataArray = iter->second;
	}

	//	Note : 지속 이팩트의 경우 파트가 변경되어 이팩트가 삭제될 경우를 대비해서 개별 확인후 삽입 해준다.
	//
	for ( int i=0; i<PIECE_SIZE; i++ )
	{
		PDXCHARPART pCharPart = pAttBone->GetPiece(i);
		BOOL bFind = pCharPart->FindEffList ( szEffFile );
		if ( !bFind )
		{
			pEffCharDataArray->SetSkinPart ( pCharPart, m_pd3dDevice, pDir, fTarDir );
		}
	}

	return TRUE;
}

/*ABL system, Juver, 2017/06/02 */
void DxEffcharDataMan::OutEffect ( DxAttBoneRender *pAttBone, std::string strEffFile )
{
	if ( !pAttBone )	return;

	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		PDXCHARPART pCharPart = pAttBone->GetPiece(i);
		pCharPart->DelEffList ( strEffFile.c_str() );
	}
}

/*vehicle system, Juver, 2017/08/06 */
BOOL DxEffcharDataMan::PutEffect ( DxVehicle *pVehicle, const char* szEffFile, D3DXVECTOR3 *pDir, float fTarDir )
{
	if ( !pVehicle )	return FALSE;

	DxEffCharDataArray *pEffCharDataArray = NULL;

	EFFDATA_ARRAYMAP_ITER iter = m_mapEffDataArray.find ( std::string(szEffFile) );
	if ( iter == m_mapEffDataArray.end() )
	{
		pEffCharDataArray = new DxEffCharDataArray;
		BOOL bOk = pEffCharDataArray->LoadFile ( szEffFile );
		if ( !bOk )
		{
			delete pEffCharDataArray;
			return FALSE;
		}

		m_mapEffDataArray[std::string(szEffFile)] = pEffCharDataArray;
	}
	else
	{
		pEffCharDataArray = iter->second;
	}

	pEffCharDataArray->SetVehicle ( pVehicle, m_pd3dDevice, pDir, fTarDir );

	return TRUE;
}

/*vehicle system, Juver, 2017/08/06 */
BOOL DxEffcharDataMan::PutPassiveEffect ( DxVehicle *pVehicle, const char* szEffFile, D3DXVECTOR3 *pDir, float fTarDir )
{
	if ( !pVehicle )	return FALSE;
	if ( !szEffFile || szEffFile[0]==NULL )	return E_FAIL;

	DxEffCharDataArray *pEffCharDataArray = NULL;

	EFFDATA_ARRAYMAP_ITER iter = m_mapEffDataArray.find ( std::string(szEffFile) );
	if ( iter == m_mapEffDataArray.end() )
	{
		pEffCharDataArray = new DxEffCharDataArray;
		BOOL bOk = pEffCharDataArray->LoadFile ( szEffFile );
		if ( !bOk )
		{
			delete pEffCharDataArray;
			return FALSE;
		}

		m_mapEffDataArray[std::string(szEffFile)] = pEffCharDataArray;
	}
	else
	{
		pEffCharDataArray = iter->second;
	}

	//	Note : 지속 이팩트의 경우 파트가 변경되어 이팩트가 삭제될 경우를 대비해서 개별 확인후 삽입 해준다.
	//
	for ( int i=0; i<PIECE_SIZE; i++ )
	{
		PDXCHARPART pCharPart = pVehicle->GetPiece(i);
		BOOL bFind = pCharPart->FindEffList ( szEffFile );
		if ( !bFind )
		{
			pEffCharDataArray->SetSkinPart ( pCharPart, m_pd3dDevice, pDir, fTarDir );
		}
	}

	return TRUE;
}

/*vehicle system, Juver, 2017/08/06 */
void DxEffcharDataMan::OutEffect ( DxVehicle *pVehicle, std::string strEffFile )
{
	if ( !pVehicle )	return;

	for ( int i=0; i<PIECE_SIZE; ++i )
	{
		PDXCHARPART pCharPart = pVehicle->GetPiece(i);
		pCharPart->DelEffList ( strEffFile.c_str() );
	}
}