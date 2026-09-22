// PageABF.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinChar.h"
#include "EditorSkinCharView.h"
#include "PageABF.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"

// CPageABF dialog

IMPLEMENT_DYNAMIC(CPageABF, CPropertyPage)

CPageABF::CPageABF(LOGFONT logfont)
	: CPropertyPage(CPageABF::IDD)
	, m_pFont( NULL )
	, m_pSkin( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageABF::~CPageABF()
{
	SAFE_DELETE( m_pFont );
}

void CPageABF::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ABF_ANIM, m_List_Anim );
	DDX_Control(pDX, IDC_LIST_ABF_PIECE, m_List_Piece );
}


BEGIN_MESSAGE_MAP(CPageABF, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_ABF_NEW, OnBnClickedButtonAbfNew)
	ON_BN_CLICKED(IDC_BUTTON_ABF_LOAD, OnBnClickedButtonAbfLoad)
	ON_BN_CLICKED(IDC_BUTTON_ABF_SAVE, OnBnClickedButtonAbfSave)
	ON_BN_CLICKED(IDC_BUTTON_ABF_PIECE_CPS, OnBnClickedButtonAbfPieceCps)
	ON_BN_CLICKED(IDC_BUTTON_ABF_PIECE_RESET, OnBnClickedButtonAbfPieceReset)
	ON_BN_CLICKED(IDC_BUTTON_ABF_ANIM_PLAY, OnBnClickedButtonAbfAnimPlay)
	ON_BN_CLICKED(IDC_BUTTON_ABF_ANIM_STOP, OnBnClickedButtonAbfAnimStop)
	ON_BN_CLICKED(IDC_BUTTON_ABF_ANIM_ADDCFG, OnBnClickedButtonAbfAnimAddcfg)
	ON_BN_CLICKED(IDC_BUTTON_ABF_ANIM_DELETE, OnBnClickedButtonAbfAnimDelete)
	ON_BN_CLICKED(IDC_BUTTON_ABF_ANIM_CLEAR, OnBnClickedButtonAbfAnimClear)
	ON_BN_CLICKED(IDC_BUTTON_ABF_SKELETON, OnBnClickedButtonAbfSkeleton)
	ON_BN_CLICKED(IDC_BUTTON_ABF_APPLY, OnBnClickedButtonAbfApply)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ABF_ANIM, OnNMDblclkListAbfAnim)
	ON_BN_CLICKED(IDC_BUTTON_ANI_INSERT_ABF, OnBnClickedButtonAniInsertAbf)
END_MESSAGE_MAP()


// CPageABF message handlers
BOOL CPageABF::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List_Anim.SetExtendedStyle ( m_List_Anim.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Anim.GetClientRect ( &rectCtrl );
	m_List_Anim.InsertColumn( 0, "Num", LVCFMT_LEFT, ( 15* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Anim.InsertColumn( 1, "Animation", LVCFMT_LEFT, ( 60* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Anim.InsertColumn( 2, "Main/Sub", LVCFMT_LEFT, ( 25* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	m_List_Piece.SetExtendedStyle ( m_List_Piece.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Piece.GetClientRect ( &rectCtrl );
	m_List_Piece.InsertColumn( 0, "Piece", LVCFMT_LEFT, ( 40* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Piece.InsertColumn( 1, "File", LVCFMT_LEFT, ( 60* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	ResetTool();

	return TRUE;  
}

void CPageABF::ResetTool()
{
	m_pSkin = NULL;
	m_pData = NULL;

	SetWin_Text( this, IDC_EDIT_ABF_FILENAME, "" );
	SetWin_Text( this, IDC_EDIT_ABF_SKELETON, "" );
	SetWin_Num_float( this, IDC_EDIT_ABF_SCALE, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_ABF_HEIGHT, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_ABF_RADIUS, 1.0f );
	SetWin_Check( this, IDC_CHECK_ABF_WORLDOBJECT, FALSE );

	m_List_Anim.DeleteAllItems();
	m_List_Piece.DeleteAllItems();

	SetWin_Text( this, IDC_EDIT_ANI_INSERT_ABF, "" );
}

void CPageABF::OnBnClickedButtonAbfNew()
{
	CEditorSkinCharView::GetView()->ABFNew();

	m_pSkin = CEditorSkinCharView::GetView()->m_pAttBoneRender;
	m_pData = CEditorSkinCharView::GetView()->m_pAttBoneData;

	if ( m_pSkin && m_pData )
		DataShow();
}

void CPageABF::OnBnClickedButtonAbfLoad()
{
	CString szFilter = "Container (*.abf)|*.abf;|";
	CFileDialog dlg ( TRUE,".ABF", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = DxAttBoneDataContainer::GetInstance().GetPath();
	if ( dlg.DoModal() != IDOK ) return;

	BOOL bOK = CEditorSkinCharView::GetView()->ABFLoad( dlg.GetFileName().GetString() );
	if ( bOK )
	{
		m_pSkin = CEditorSkinCharView::GetView()->m_pAttBoneRender;
		m_pData = CEditorSkinCharView::GetView()->m_pAttBoneData;

		if ( m_pSkin && m_pData )
			DataShow();
	}
}

void CPageABF::OnBnClickedButtonAbfSave()
{
	if( !m_pData )	return;

	if( !m_pData->GetSkeleton() )	
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Skeleton!" );
		return;
	}

	if( !m_pData->GetAniList().size() )	
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Animation Empty!" );
		return;
	}

	CString szFilter = "Container (*.abf)|*.abf;|";
	CFileDialog dlg(FALSE,"*.*", m_pData->GetFileName(), OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = DxAttBoneDataContainer::GetInstance().GetPath();
	if ( dlg.DoModal() != IDOK ) return;

	BOOL bOK = CEditorSkinCharView::GetView()->ABFSave( dlg.GetFileName().GetString() );
	if ( bOK )
	{
		if ( m_pSkin && m_pData )
			DataShow();
	}
}

void CPageABF::DataShow()
{
	if ( !m_pData )	return;

	SetWin_Text( this, IDC_EDIT_ABF_FILENAME, m_pData->GetFileName() );
	SetWin_Text( this, IDC_EDIT_ABF_SKELETON, m_pData->GetSkeletonName() );
	SetWin_Num_float( this, IDC_EDIT_ABF_SCALE, m_pData->m_fScale );
	SetWin_Num_float( this, IDC_EDIT_ABF_HEIGHT, m_pData->m_fHeight );
	SetWin_Num_float( this, IDC_EDIT_ABF_RADIUS, m_pData->m_fRadius );
	SetWin_Check( this, IDC_CHECK_ABF_WORLDOBJECT, m_pData->m_bWorldObj );

	DataShowAnim();
	DataShowPiece();
}

void CPageABF::DataShowAnim()
{
	m_List_Anim.DeleteAllItems();

	if ( !m_pData )	return;

	m_List_Anim.SetRedraw( FALSE );

	DxSkinAniControl::VECNODE vecAnim = m_pData->GetAniList ();
	for ( int i= 0; i<(int)vecAnim.size(); ++ i )
	{
		char szTempChar[512];
		sprintf( szTempChar, "%d", i );
		m_List_Anim.InsertItem( i, szTempChar );
		m_List_Anim.SetItemText( i, 1, vecAnim[i]->pAnimCont->m_szName );

		sprintf( szTempChar, "(%d)(%d)", vecAnim[i]->pAnimCont->m_MainType, vecAnim[i]->pAnimCont->m_SubType );
		m_List_Anim.SetItemText( i, 2, szTempChar );
	}

	m_List_Anim.SetRedraw( TRUE );
}

void CPageABF::DataShowPiece()
{
	m_List_Piece.DeleteAllItems();

	if ( !m_pData )	return;

	m_List_Piece.SetRedraw( FALSE );

	for ( int i= 0; i<PIECE_SIZE; ++ i )
	{
		m_List_Piece.InsertItem( i, SKINTYPE_STRING::m_szPIECETYPE[i] );

		PDXSKINPIECE pPIECE = m_pData->GetPiece( i );
		if ( pPIECE )
		{
			m_List_Piece.SetItemText( i, 1, pPIECE->m_szFileName );
		}
	}

	m_List_Piece.SetRedraw( TRUE );
}

void CPageABF::ResetAnimation()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	m_pSkin->ResetAnimationCore();
	m_pData->ResetAnimationCore();

	if ( m_pSkin->GetSkeleton() && m_pSkin->GetSkeleton()->pBoneRoot )
		m_pSkin->GetSkeleton()->pBoneRoot->ResetBone();

	if ( m_pData->GetSkeleton() && m_pData->GetSkeleton()->pBoneRoot )
		m_pData->GetSkeleton()->pBoneRoot->ResetBone();
}

void CPageABF::OnBnClickedButtonAbfPieceCps()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	CString szFilterInput = "Piece (*.cps,*.aps,*.vps)|*.cps;*.aps;*.vps|";
	CFileDialog dlgInput(true,"*.*",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput,this);
	dlgInput.m_ofn.lpstrInitialDir = DxSkinPieceContainer::GetInstance().GetPath();
	if ( dlgInput.DoModal() != IDOK ) return;

	std::string strFILE = dlgInput.GetFileName();
	PDXSKINPIECE pPiece = new DxSkinPiece;
	HRESULT hr = pPiece->LoadPiece( strFILE.c_str(), CEditorSkinCharView::GetView()->Get3DDevice(), TRUE );
	if ( FAILED(hr) )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "%s Error Loading Piece", strFILE.c_str() );
		SAFE_DELETE( pPiece );
		return;
	}

	//skeleton check
	CString strFile1( m_pData->GetSkeletonName() );
	strFile1.MakeLower();

	CString strFile2( pPiece->m_szSkeleton );
	strFile2.MakeLower();

	if( _tcscmp(strFile1.GetString(),strFile2.GetString()) != 0 )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "%s Invalid Piece Skeleton Mismatch", strFILE.c_str() );
		SAFE_DELETE( pPiece );
		return;
	}

	m_pSkin->ResetPiece( pPiece->m_emType );
	m_pData->ResetPiece( CEditorSkinCharView::GetView()->Get3DDevice(), pPiece->m_emType );

	SAFE_DELETE( pPiece );
	DxSkinPieceContainer::GetInstance().DeletePiece ( strFILE.c_str() );

	hr = m_pData->SetPiece( strFILE.c_str(), CEditorSkinCharView::GetView()->Get3DDevice(), TRUE );
	if ( FAILED(hr) )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "%s Invalid Piece", strFILE.c_str() );
		return;
	}

	hr = m_pSkin->SetPiece( strFILE.c_str(), CEditorSkinCharView::GetView()->Get3DDevice(), 0, 0, TRUE );
	if ( FAILED(hr) )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "%s Invalid Piece", strFILE.c_str() );
		return;
	}

	DataShowPiece();

	for( int nItem = 0; nItem < m_List_Piece.GetItemCount(); nItem++)
	{
		std::string str = m_List_Piece.GetItemText( nItem, 1 );
		if( _tcscmp( strFILE.c_str(), str.c_str() ) == 0 )
		{
			m_List_Piece.EnsureVisible( nItem, TRUE );
			m_List_Piece.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
		}
	}
}

void CPageABF::OnBnClickedButtonAbfPieceReset()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	int nSelect = m_List_Piece.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	m_pSkin->ResetPiece( nSelect );
	m_pData->ResetPiece( CEditorSkinCharView::GetView()->Get3DDevice(), nSelect );

	DataShowPiece();
}

void CPageABF::OnBnClickedButtonAbfAnimPlay()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	int nSelect = m_List_Anim.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	std::string str = m_List_Anim.GetItemText( nSelect, 1 );
	m_pSkin->SELECTANI( str.c_str() );
	m_pData->SELECTANI( str.c_str() );
}

void CPageABF::OnBnClickedButtonAbfAnimStop()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	ResetAnimation();
}

void CPageABF::OnBnClickedButtonAbfAnimAddcfg()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	if ( !m_pData->GetSkeleton() )	
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Skeleton!" );
		return;
	}

	CString szFilterInput = "Animation Cfg Files (*.cfg)|*.cfg|";
	CFileDialog dlg( TRUE,".bin",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput, this );
	dlg.m_ofn.lpstrInitialDir = DxSkinAniMan::GetInstance().GetPath();
	if ( dlg.DoModal() != IDOK ) return;

	CString strFile = dlg.GetFileName().GetString();
	int nIndex = strFile.ReverseFind ( '.' );
	std::string strFileName = strFile.Left(nIndex) + ".x";

	PSANIMCONTAINER pAnim = DxSkinAniMan::GetInstance().LoadAnimContainer ( strFileName.c_str(), m_pData->GetSkeletonName(), CEditorSkinCharView::GetView()->Get3DDevice(), FALSE );
	if ( !pAnim )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error Load CFG :%s", strFileName.c_str() );
		return;
	}

	ResetAnimation();

	BOOL bOK = m_pSkin->LoadAnimation( strFileName.c_str(), CEditorSkinCharView::GetView()->Get3DDevice(), FALSE );
	if ( !bOK )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error Load Animation :%s", strFileName.c_str() );
		return;
	}

	bOK = m_pData->LoadAnimation( strFileName.c_str(), CEditorSkinCharView::GetView()->Get3DDevice(), FALSE );
	if ( !bOK )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error Load Animation :%s", strFileName.c_str() );
		return;
	}

	DataShowAnim();

	for( int nItem = 0; nItem < m_List_Anim.GetItemCount(); nItem++)
	{
		std::string str = m_List_Anim.GetItemText( nItem, 1 );
		if( _tcscmp( strFileName.c_str(), str.c_str() ) == 0 )
		{
			m_List_Anim.EnsureVisible( nItem, TRUE );
			m_List_Anim.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
		}
	}
}

void CPageABF::OnBnClickedButtonAbfAnimDelete()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	int nSelect = m_List_Anim.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	std::string str = m_List_Anim.GetItemText( nSelect, 1 );

	ResetAnimation();

	m_pSkin->ReleaseAnimation( str.c_str() );
	m_pData->ReleaseAnimation( str.c_str() );

	DataShowAnim();

	int nNewSelect = nSelect > 1? nSelect-1:nSelect;
	m_List_Anim.EnsureVisible( nNewSelect, TRUE );
	m_List_Anim.SetItemState( nNewSelect, LVIS_SELECTED, LVIS_SELECTED );
}

void CPageABF::OnBnClickedButtonAbfAnimClear()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	int nRETURN = CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Clear all animations?" );
	if ( nRETURN != IDYES )	return;

	ResetAnimation();

	DxSkinAniControl::VECNODE vecNode = m_pSkin->GetAniList();
	for( size_t i=0; i<vecNode.size(); ++i )
	{
		std::string strAniName = vecNode[i]->pAnimCont->m_szName;
		m_pSkin->ReleaseAnimation( strAniName.c_str() );
	}

	vecNode = m_pData->GetAniList();
	for( size_t i=0; i<vecNode.size(); ++i )
	{
		std::string strAniName = vecNode[i]->pAnimCont->m_szName;
		m_pData->ReleaseAnimation( strAniName.c_str() );
	}

	DataShowAnim();
}

void CPageABF::OnBnClickedButtonAbfSkeleton()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	int nRETURN = CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Change Skeleton? Piece and animations will be cleared!" );
	if ( nRETURN != IDYES )	return;

	CString szFilterInput = "Skeleton (*.x)|*.x;|";
	CFileDialog dlgInput(true,"*.X*",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput, this );
	dlgInput.m_ofn.lpstrInitialDir = DxBoneCollector::GetInstance().GetPath();
	if ( dlgInput.DoModal() != IDOK ) return;

	std::string strSkeleton = dlgInput.GetFileName();
	DxSkeleton* pSkeleton = DxBoneCollector::GetInstance().Load( strSkeleton.c_str(), CEditorSkinCharView::GetView()->Get3DDevice() );
	if ( pSkeleton )
	{
		ResetAnimation();

		m_pData->ClearAll();
		m_pSkin->ClearAll();

		BOOL bOK = m_pData->LoadSkeleton( strSkeleton.c_str(), CEditorSkinCharView::GetView()->Get3DDevice() );
		if ( !bOK )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Error Load Skeleton :%s", strSkeleton.c_str() );
			return;
		}

		bOK = m_pSkin->LoadSkeleton( strSkeleton.c_str(), CEditorSkinCharView::GetView()->Get3DDevice() );
		if ( !bOK )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Error Load Skeleton :%s", strSkeleton.c_str() );
			return;
		}

		DataShow();
	}
}

void CPageABF::OnBnClickedButtonAbfApply()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	m_pData->m_fScale = GetWin_Num_float( this, IDC_EDIT_ABF_SCALE );
	m_pData->m_fHeight = GetWin_Num_float( this, IDC_EDIT_ABF_HEIGHT );
	m_pData->m_fRadius = GetWin_Num_float( this, IDC_EDIT_ABF_RADIUS );
	m_pData->m_bWorldObj = GetWin_Check( this, IDC_CHECK_ABF_WORLDOBJECT );
	m_pData->SetRadiusHeight();

	m_pSkin->SetScale( m_pData->m_fScale );
	m_pSkin->SetRadiusHeight( m_pData );
	m_pSkin->SetWorldObj( m_pData->m_bWorldObj );
}

void CPageABF::OnNMDblclkListAbfAnim(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	int nSelect = m_List_Anim.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	std::string str = m_List_Anim.GetItemText( nSelect, 1 );
	m_pSkin->SELECTANI( str.c_str() );
	m_pData->SELECTANI( str.c_str() );
}

void CPageABF::OnBnClickedButtonAniInsertAbf()
{
	if ( !m_pData )	return;
	if ( !m_pSkin )	return;

	CString strAni = GetWin_Text( this, IDC_EDIT_ANI_INSERT_ABF );
	if ( strAni.GetLength() <= 0  )	return;

	int nIndex = strAni.ReverseFind ( '.' );
	std::string strFileName = strAni.Left(nIndex) + ".x";

	PSANIMCONTAINER pAnim = DxSkinAniMan::GetInstance().LoadAnimContainer ( strFileName.c_str(), m_pData->GetSkeletonName(), CEditorSkinCharView::GetView()->Get3DDevice(), FALSE );
	if ( !pAnim )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error Load CFG :%s", strFileName.c_str() );
		return;
	}

	ResetAnimation();

	BOOL bOK = m_pSkin->LoadAnimation( strFileName.c_str(), CEditorSkinCharView::GetView()->Get3DDevice(), FALSE );
	if ( !bOK )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error Load Animation :%s", strFileName.c_str() );
		return;
	}

	bOK = m_pData->LoadAnimation( strFileName.c_str(), CEditorSkinCharView::GetView()->Get3DDevice(), FALSE );
	if ( !bOK )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error Load Animation :%s", strFileName.c_str() );
		return;
	}

	DataShowAnim();

	for( int nItem = 0; nItem < m_List_Anim.GetItemCount(); nItem++)
	{
		std::string str = m_List_Anim.GetItemText( nItem, 1 );
		if( _tcscmp( strFileName.c_str(), str.c_str() ) == 0 )
		{
			m_List_Anim.EnsureVisible( nItem, TRUE );
			m_List_Anim.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
		}
	}
}
