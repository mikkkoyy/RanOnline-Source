
// GameViewerView.cpp : implementation of the CGameViewerView class
//

#include "stdafx.h"
#include "GameViewer.h"

#include "GameViewerDoc.h"
#include "GameViewerView.h"

#include "DxSkinCharData.h"
#include "DxEffGroupPlayer.h"

#include "SheetWithTab.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CGameViewerView::DataReset()
{
	if ( m_pSkinChar )
	{
		m_pSkinChar->ClearAll();
		DxSkinCharDataContainer::GetInstance().ReleaseData( m_pSkinChar->GetFileName() );
		SAFE_DELETE( m_pSkinChar );
	}

	/*ABL system, Juver, 2017/06/02 */
	if ( m_pAttBone )
	{
		m_pAttBone->ClearAll();
		DxAttBoneLinkContainer::GetInstance().ReleaseData( m_pAttBone->GetFileName() );
		SAFE_DELETE( m_pAttBone );
	}

	/*ABL system, Juver, 2017/06/02 */
	if ( m_pAttBoneRender )
	{
		m_pAttBoneRender->ClearAll();
		DxAttBoneDataContainer::GetInstance().ReleaseData( m_pAttBoneRender->GetFileName() );
		SAFE_DELETE( m_pAttBoneRender );
	}

	/*ABL system, Juver, 2017/06/03 */
	if ( m_pMaskPiece )
	{
		DxSkinPieceContainer::GetInstance().ReleasePiece( m_pMaskPiece->m_szFileName );
		m_pMaskPiece = NULL;
	}

	if ( m_pEffSingle )
	{
		m_pEffSingle->CleanUp();
		DxEffSinglePropGMan::GetInstance().DeleteEffectGProp( m_pEffSingle->m_strFileName.c_str() );
		SAFE_DELETE ( m_pEffSingle );
	}

	if ( m_pPIE )
	{
		m_pPIE->CleanUp();
		SAFE_DELETE(m_pPIE);
	}

	/*vehicle system, Juver, 2017/08/06 */
	if ( m_pVehicle )
	{
		m_pVehicle->ClearAll();
		DxVehicleDataContainer::GetInstance().ReleaseData( m_pVehicle->GetFileName() );
		SAFE_DELETE( m_pVehicle );
	}

	CMainFrame	*pMainFrame = (CMainFrame*) AfxGetMainWnd();
	if ( pMainFrame )
	{
		CsheetWithTab*  pSheetTab = pMainFrame->m_wndEditor.m_pSheetTab;
		if ( pSheetTab )
			pSheetTab->ResetData();
	}

	m_pPropGroup = NULL;
}

BOOL CGameViewerView::DataLoadCHF( std::string strFILE )
{
	if ( strFILE.size() <= 0  )	return FALSE;

	DataReset();

	DxSkinCharData* pData = DxSkinCharDataContainer::GetInstance().LoadData( strFILE.c_str(), m_pd3dDevice, TRUE );
	if ( !pData )	return FALSE;

	m_pSkinChar = new DxSkinChar;
	m_pSkinChar->SetCharData( pData, m_pd3dDevice, TRUE );
	m_pSkinChar->InitDeviceObjects( m_pd3dDevice );
	m_pSkinChar->RestoreDeviceObjects( m_pd3dDevice );
	
	m_pSkinChar->SELECTANI ( AN_GUARD_N, AN_SUB_NONE );
	if ( !m_pSkinChar->GETCURANIMNODE() )
		m_pSkinChar->DEFAULTANI();

	return TRUE;
}

/*ABL system, Juver, 2017/06/02 */
BOOL CGameViewerView::DataLoadABF( std::string strFILE )
{
	if ( strFILE.size() <= 0  )	return FALSE;

	DataReset();

	DxAttBoneData* pData = DxAttBoneDataContainer::GetInstance().LoadData( strFILE.c_str(), m_pd3dDevice, TRUE );
	if ( !pData )	return FALSE;

	m_pAttBoneRender = new DxAttBoneRender;
	m_pAttBoneRender->SetCharData( pData, m_pd3dDevice, TRUE );
	m_pAttBoneRender->InitDeviceObjects( m_pd3dDevice );
	m_pAttBoneRender->RestoreDeviceObjects( m_pd3dDevice );

	m_pAttBoneRender->SELECTANI ( AN_GUARD_N, AN_SUB_NONE );
	if ( !m_pAttBoneRender->GETCURANIMNODE() )
		m_pAttBoneRender->DEFAULTANI();

	return TRUE;
}

BOOL CGameViewerView::DataLoadABL( std::string strFILE )
{
	if ( strFILE.size() <= 0  )	return FALSE;

	DataReset();

	DxAttBoneLink* pData = DxAttBoneLinkContainer::GetInstance().LoadData( strFILE.c_str(), m_pd3dDevice, TRUE );
	if ( !pData )	return FALSE;

	m_pAttBone = new DxAttBone;
	m_pAttBone->SetCharData( pData, m_pd3dDevice, TRUE );
	m_pAttBone->InitDeviceObjects( m_pd3dDevice );
	m_pAttBone->RestoreDeviceObjects( m_pd3dDevice );

	if ( m_pAttBone->m_strMaskPiece.size() != 0 )
		m_pMaskPiece = DxSkinPieceContainer::GetInstance().LoadPiece( m_pAttBone->m_strMaskPiece.c_str(), m_pd3dDevice, TRUE );

	m_pAttBone->SELECTANI ( AN_GUARD_N, AN_SUB_NONE );
	if ( !m_pAttBone->GETCURANIMNODE() )
		m_pAttBone->DEFAULTANI();

	return TRUE;
}


BOOL CGameViewerView::DataLoadEGP( std::string strFILE )
{
	if ( strFILE.size() <= 0  )	return FALSE;

	DataReset();

	EFF_PROPGROUP*  pPropGroup = DxEffSinglePropGMan::GetInstance().LoadEffectGProp ( strFILE.c_str() );
	if ( !pPropGroup )	return FALSE;

	m_pEffSingle = pPropGroup->NEWEFFGROUP ();
	if ( !m_pEffSingle )	return FALSE;

	m_pPropGroup = NULL;
	m_pPropGroup = pPropGroup;

	STARGETID sTargetID( CROW_MOB, 0, D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) );
	m_pEffSingle->SetTargetID ( &sTargetID );
	m_pEffSingle->m_matWorld = m_matLocal;
	m_pEffSingle->Create ( m_pd3dDevice );

	return TRUE;
}

BOOL CGameViewerView::DataSaveEGP( std::string strFILE )
{
	if ( strFILE.size() <= 0  )	return FALSE;

	if ( !m_pPropGroup )	return FALSE;

	HRESULT hr = m_pPropGroup->SaveFile( strFILE.c_str() );
	if ( FAILED( hr ))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CGameViewerView::DataLoadPIE( std::string strFILE )
{
	if ( strFILE.size() <= 0  )	return FALSE;

	DataReset();

	char szPathName[MAX_PATH] = "";
	StringCchCopy( szPathName, MAX_PATH, DxPieceContainer::GetInstance().GetPath() );
	StringCchCat( szPathName, MAX_PATH, strFILE.c_str() );

	m_pPIE = new DxPieceEdit;
	HRESULT hr = m_pPIE->LoadPIE( m_pd3dDevice, szPathName );
	if ( FAILED(hr) )
	{
		SAFE_DELETE(m_pPIE);
		return FALSE;
	}

	return TRUE;
}

BOOL CGameViewerView::DataSavePIE( std::string strFILE )
{
	if ( strFILE.size() <= 0  )	return FALSE;

	if ( !m_pPIE )	return FALSE;

	m_pPIE->SavePIE( strFILE.c_str() );
	

	return TRUE;
}

/*vehicle system, Juver, 2017/08/06 */
BOOL CGameViewerView::DataLoadVCF( std::string strFILE )
{
	if ( strFILE.size() <= 0  )	return FALSE;

	DataReset();

	DxVehicleData* pData = DxVehicleDataContainer::GetInstance().LoadData( strFILE.c_str(), m_pd3dDevice, TRUE );
	if ( !pData )	return FALSE;

	m_pVehicle = new DxVehicle;
	m_pVehicle->SetCharData( pData, m_pd3dDevice, TRUE );
	m_pVehicle->InitDeviceObjects( m_pd3dDevice );
	m_pVehicle->RestoreDeviceObjects( m_pd3dDevice );

	m_pVehicle->SELECTANI ( AN_VEHICLE_STAY, AN_SUB_NONE );
	if ( !m_pVehicle->GETCURANIMNODE() )
		m_pVehicle->DEFAULTANI();

	return TRUE;
}