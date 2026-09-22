
// EditorSkinCharView.cpp : implementation of the CEditorSkinCharView class
//

#include "stdafx.h"
#include "EditorSkinChar.h"
#include "EditorSkinCharDoc.h"
#include "EditorSkinCharView.h"
#include "MainFrm.h"
#include "SheetWithTab.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CEditorSkinCharView::ResetData()
{
	ResetMouseEdit();

	SAFE_DELETE( m_pSkinChar );
	SAFE_DELETE( m_pSkinCharData );

	/*ABL system, Juver, 2017/05/30 */
	SAFE_DELETE( m_pAttBoneRender );
	SAFE_DELETE( m_pAttBoneData );
	SAFE_DELETE( m_pAttBone );
	SAFE_DELETE( m_pAttBoneLink );
	m_pMaskPiece = NULL;

	/*vehicle system, Juver, 2017/08/05 */
	SAFE_DELETE( m_pVehicleData );
	SAFE_DELETE( m_pVehicle );
	SAFE_DELETE( m_pVehicleSkin );

	CMainFrame	*pMainFrame = (CMainFrame*) AfxGetMainWnd();
	if ( pMainFrame )
	{
		CsheetWithTab*  pSheetTab = pMainFrame->m_wndEditor.m_pSheetTab;
		if ( pSheetTab )
			pSheetTab->ResetEditor();
	}
}

BOOL CEditorSkinCharView::CHFLoad( std::string strFileName )
{
	if ( !strFileName.size() )	return FALSE;

	DxSkinCharData* pSkinData = new DxSkinCharData;
	BOOL bOK = pSkinData->LoadFile( strFileName.c_str(), m_pd3dDevice, TRUE, FALSE );
	if ( !bOK )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error LoadCHF :%s", strFileName.c_str() );
		SAFE_DELETE( pSkinData );
		return FALSE;
	}

	ResetData();

	m_pSkinCharData = pSkinData;
	m_pSkinChar = new DxSkinChar;
	m_pSkinChar->SetCharData( m_pSkinCharData, m_pd3dDevice, TRUE );
	m_pSkinChar->InitDeviceObjects( m_pd3dDevice );
	m_pSkinChar->RestoreDeviceObjects( m_pd3dDevice );

	m_pSkinChar->DEFAULTANI();
	m_pSkinChar->ResetAnimationCore();

	if ( m_pSkinChar->GetSkeleton() && m_pSkinChar->GetSkeleton()->pBoneRoot )
		m_pSkinChar->GetSkeleton()->pBoneRoot->ResetBone();

	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	if ( pFrame )
		pFrame->SetWindowText( strFileName.c_str() );

	return TRUE;
}

BOOL CEditorSkinCharView::CHFSave( std::string strFileName )
{
	if ( !strFileName.size() )	return FALSE;
	if ( !m_pSkinCharData )	return FALSE;
	if ( !m_pSkinChar )	return FALSE;

	BOOL bOK = m_pSkinCharData->SaveFile( strFileName.c_str(), m_pd3dDevice );
	if ( !bOK )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error SaveCHF :%s", strFileName.c_str() );
		return FALSE;
	}

	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	if ( pFrame )
		pFrame->SetWindowText( strFileName.c_str() );

	return TRUE;
}

BOOL CEditorSkinCharView::CHFNew()
{
	ResetData();

	m_pSkinCharData = new DxSkinCharData;
	m_pSkinChar = new DxSkinChar;
	m_pSkinChar->SetCharData( m_pSkinCharData, m_pd3dDevice, TRUE );
	m_pSkinChar->InitDeviceObjects( m_pd3dDevice );
	m_pSkinChar->RestoreDeviceObjects( m_pd3dDevice );

	m_pSkinChar->DEFAULTANI();
	m_pSkinChar->ResetAnimationCore();

	if ( m_pSkinChar->GetSkeleton() && m_pSkinChar->GetSkeleton()->pBoneRoot )
		m_pSkinChar->GetSkeleton()->pBoneRoot->ResetBone();

	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	if ( pFrame )
		pFrame->SetWindowText( "" );

	return TRUE;
}

/*ABL system, Juver, 2017/05/30 */
BOOL CEditorSkinCharView::ABFLoad( std::string strFileName )
{
	if ( !strFileName.size() )	return FALSE;

	DxAttBoneData* pBoneData = new DxAttBoneData;
	BOOL bOK = pBoneData->LoadFile( strFileName.c_str(), m_pd3dDevice, TRUE, FALSE );
	if ( !bOK )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error LoadABF :%s", strFileName.c_str() );
		SAFE_DELETE( pBoneData );
		return FALSE;
	}

	ResetData();

	m_pAttBoneData = pBoneData;
	m_pAttBoneRender = new DxAttBoneRender;
	m_pAttBoneRender->SetCharData( m_pAttBoneData, m_pd3dDevice, TRUE );
	m_pAttBoneRender->InitDeviceObjects( m_pd3dDevice );
	m_pAttBoneRender->RestoreDeviceObjects( m_pd3dDevice );

	m_pAttBoneRender->DEFAULTANI();
	m_pAttBoneRender->ResetAnimationCore();

	if ( m_pAttBoneRender->GetSkeleton() && m_pAttBoneRender->GetSkeleton()->pBoneRoot )
		m_pAttBoneRender->GetSkeleton()->pBoneRoot->ResetBone();

	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	if ( pFrame )
		pFrame->SetWindowText( strFileName.c_str() );

	return TRUE;
}

/*ABL system, Juver, 2017/05/30 */
BOOL CEditorSkinCharView::ABFSave( std::string strFileName )
{
	if ( !strFileName.size() )	return FALSE;
	if ( !m_pAttBoneData )	return FALSE;
	if ( !m_pAttBoneRender )	return FALSE;

	BOOL bOK = m_pAttBoneData->SaveFile( strFileName.c_str(), m_pd3dDevice );
	if ( !bOK )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error SaveABF :%s", strFileName.c_str() );
		return FALSE;
	}

	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	if ( pFrame )
		pFrame->SetWindowText( strFileName.c_str() );

	return TRUE;
}

/*ABL system, Juver, 2017/05/30 */
BOOL CEditorSkinCharView::ABFNew()
{
	ResetData();

	m_pAttBoneData = new DxAttBoneData;
	m_pAttBoneRender = new DxAttBoneRender;
	m_pAttBoneRender->SetCharData( m_pAttBoneData, m_pd3dDevice, TRUE );
	m_pAttBoneRender->InitDeviceObjects( m_pd3dDevice );
	m_pAttBoneRender->RestoreDeviceObjects( m_pd3dDevice );

	m_pAttBoneRender->DEFAULTANI();
	m_pAttBoneRender->ResetAnimationCore();

	if ( m_pAttBoneRender->GetSkeleton() && m_pAttBoneRender->GetSkeleton()->pBoneRoot )
		m_pAttBoneRender->GetSkeleton()->pBoneRoot->ResetBone();

	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	if ( pFrame )
		pFrame->SetWindowText( "" );

	return TRUE;
}


/*ABL system, Juver, 2017/05/30 */
BOOL CEditorSkinCharView::ABLLoad( std::string strFileName )
{
	if ( !strFileName.size() )	return FALSE;

	DxAttBoneLink* pBoneData = new DxAttBoneLink;
	BOOL bOK = pBoneData->LoadFile( strFileName.c_str(), m_pd3dDevice, TRUE );
	if ( !bOK )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error LoadABL :%s", strFileName.c_str() );
		SAFE_DELETE( pBoneData );
		return FALSE;
	}

	ResetData();

	m_pAttBoneLink = pBoneData;
	m_pAttBone = new DxAttBone;
	m_pAttBone->SetCharData( m_pAttBoneLink, m_pd3dDevice, 0, TRUE );
	m_pAttBone->InitDeviceObjects( m_pd3dDevice );
	m_pAttBone->RestoreDeviceObjects( m_pd3dDevice );

	m_pAttBone->DEFAULTANI();
	m_pAttBone->ResetAnimationCore();

	if ( m_pAttBone->GetSkeleton() && m_pAttBone->GetSkeleton()->pBoneRoot )
		m_pAttBone->GetSkeleton()->pBoneRoot->ResetBone();

	if ( m_pAttBone->m_pSkeletonLink && m_pAttBone->m_pSkeletonLink->pBoneRoot )
	{
		m_pAttBone->m_pSkeletonLink->pBoneRoot->ResetBone();
		m_pAttBone->m_pSkeletonLink->UpdateBones( m_pAttBone->m_pSkeletonLink->pBoneRoot, m_matLocal );
	}

	if ( m_pAttBone->m_strMaskPiece.size() != 0 )
	{
		m_pMaskPiece = DxSkinPieceContainer::GetInstance().LoadPiece( m_pAttBone->m_strMaskPiece.c_str(), m_pd3dDevice, TRUE );
	}

	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	if ( pFrame )
		pFrame->SetWindowText( strFileName.c_str() );

	return TRUE;
}

/*ABL system, Juver, 2017/05/30 */
BOOL CEditorSkinCharView::ABLSave( std::string strFileName )
{
	if ( !strFileName.size() )	return FALSE;
	if ( !m_pAttBoneLink )	return FALSE;
	if ( !m_pAttBone )	return FALSE;

	BOOL bOK = m_pAttBoneLink->SaveFile( strFileName.c_str(), m_pd3dDevice );
	if ( !bOK )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error SaveABL :%s", strFileName.c_str() );
		return FALSE;
	}

	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	if ( pFrame )
		pFrame->SetWindowText( strFileName.c_str() );

	return TRUE;
}

/*ABL system, Juver, 2017/05/30 */
BOOL CEditorSkinCharView::ABLNew()
{
	ResetData();

	m_pAttBoneLink = new DxAttBoneLink;
	m_pAttBone = new DxAttBone;
	m_pAttBone->SetCharData( m_pAttBoneLink, m_pd3dDevice, 0, TRUE );
	m_pAttBone->InitDeviceObjects( m_pd3dDevice );
	m_pAttBone->RestoreDeviceObjects( m_pd3dDevice );

	m_pAttBone->DEFAULTANI();
	m_pAttBone->ResetAnimationCore();

	if ( m_pAttBone->GetSkeleton() && m_pAttBone->GetSkeleton()->pBoneRoot )
		m_pAttBone->GetSkeleton()->pBoneRoot->ResetBone();


	if ( m_pAttBone->m_pSkeletonLink && m_pAttBone->m_pSkeletonLink->pBoneRoot )
	{
		m_pAttBone->m_pSkeletonLink->pBoneRoot->ResetBone();
		m_pAttBone->m_pSkeletonLink->UpdateBones( m_pAttBone->m_pSkeletonLink->pBoneRoot, m_matLocal );
	}

	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	if ( pFrame )
		pFrame->SetWindowText( "" );

	return TRUE;
}


/*vehicle system, Juver, 2017/08/05 */
BOOL CEditorSkinCharView::VCFLoad( std::string strFileName )
{
	if ( !strFileName.size() )	return FALSE;

	DxVehicleData* pVehicleData = new DxVehicleData;
	BOOL bOK = pVehicleData->LoadFile( strFileName.c_str(), m_pd3dDevice, TRUE, FALSE );
	if ( !bOK )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error VCFLoad :%s", strFileName.c_str() );
		SAFE_DELETE( pVehicleData );
		return FALSE;
	}

	ResetData();

	m_pVehicleData = pVehicleData;
	m_pVehicle = new DxVehicle;
	m_pVehicle->SetCharData( m_pVehicleData, m_pd3dDevice, TRUE );
	m_pVehicle->InitDeviceObjects( m_pd3dDevice );
	m_pVehicle->RestoreDeviceObjects( m_pd3dDevice );

	m_pVehicle->DEFAULTANI();
	m_pVehicle->ResetAnimationCore();

	if ( m_pVehicle->GetSkeleton() && m_pVehicle->GetSkeleton()->pBoneRoot )
		m_pVehicle->GetSkeleton()->pBoneRoot->ResetBone();

	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	if ( pFrame )
		pFrame->SetWindowText( strFileName.c_str() );

	return TRUE;
}

/*vehicle system, Juver, 2017/08/05 */
BOOL CEditorSkinCharView::VCFSave( std::string strFileName )
{
	if ( !strFileName.size() )	return FALSE;
	if ( !m_pVehicleData )	return FALSE;
	if ( !m_pVehicle )	return FALSE;

	BOOL bOK = m_pVehicleData->SaveFile( strFileName.c_str(), m_pd3dDevice );
	if ( !bOK )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error VCFSave :%s", strFileName.c_str() );
		return FALSE;
	}

	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	if ( pFrame )
		pFrame->SetWindowText( strFileName.c_str() );

	return TRUE;
}

/*vehicle system, Juver, 2017/08/05 */
BOOL CEditorSkinCharView::VCFNew()
{
	ResetData();

	m_pVehicleData = new DxVehicleData;
	m_pVehicle = new DxVehicle;
	m_pVehicle->SetCharData( m_pVehicleData, m_pd3dDevice, TRUE );
	m_pVehicle->InitDeviceObjects( m_pd3dDevice );
	m_pVehicle->RestoreDeviceObjects( m_pd3dDevice );

	m_pVehicle->DEFAULTANI();
	m_pVehicle->ResetAnimationCore();

	if ( m_pVehicle->GetSkeleton() && m_pVehicle->GetSkeleton()->pBoneRoot )
		m_pVehicle->GetSkeleton()->pBoneRoot->ResetBone();

	CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	if ( pFrame )
		pFrame->SetWindowText( "" );

	return TRUE;
}

/*vehicle system, Juver, 2017/08/05 */
BOOL CEditorSkinCharView::VCFLoadTest( std::string strFileName )
{
	if ( !strFileName.size() )	return FALSE;

	DxSkinCharData* pSkinData = DxSkinCharDataContainer::GetInstance().LoadData( strFileName.c_str(),  m_pd3dDevice, TRUE );
	if ( !pSkinData )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error LoadCHF :%s", strFileName.c_str() );
		return FALSE;
	}

	if ( m_pVehicleSkin )
		SAFE_DELETE( m_pVehicleSkin );

	m_pVehicleSkin = new DxSkinChar;
	m_pVehicleSkin->SetCharData( pSkinData, m_pd3dDevice, TRUE );
	m_pVehicleSkin->InitDeviceObjects( m_pd3dDevice );
	m_pVehicleSkin->RestoreDeviceObjects( m_pd3dDevice );

	m_pVehicleSkin->DEFAULTANI();
	m_pVehicleSkin->ResetAnimationCore();

	if ( m_pVehicleSkin->GetSkeleton() && m_pVehicleSkin->GetSkeleton()->pBoneRoot )
		m_pVehicleSkin->GetSkeleton()->pBoneRoot->ResetBone();
	

	return TRUE;
}