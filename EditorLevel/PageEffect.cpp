// PageEffect.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "PageEffect.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"

// CPageEffect dialog

IMPLEMENT_DYNAMIC(CPageEffect, CPropertyPage)

CPageEffect::CPageEffect(LOGFONT logfont)
	: CPropertyPage(CPageEffect::IDD)
	, m_pFont ( NULL )
	, m_pGLLand( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEffect::~CPageEffect()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEffect::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CPageEffect, CPropertyPage)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnNMDblclkList)
	ON_BN_CLICKED(IDC_BUTTON_NEW, OnBnClickedButtonNew)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnBnClickedButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnBnClickedButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_REBUILDNAME, OnBnClickedButtonRebuildname)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
END_MESSAGE_MAP()


// CPageEffect message handlers
BOOL CPageEffect::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	LONG lnWidth;
	LV_COLUMN lvColumn;

	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES );
	m_List.GetClientRect ( &rectCtrl );
	lnWidth = rectCtrl.right - rectCtrl.left;

	const int nColumnCount = 4;	
	char* szColumnName[nColumnCount] = { "", "Num", "Effect Name", "Effect FileName" };
	int nColumnWidthPercent[nColumnCount] = { 5, 10, 45, 40 };

	for ( int i = 0; i < nColumnCount; i++ )
	{		
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.pszText = szColumnName[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = ( lnWidth*nColumnWidthPercent[i] ) / 100;
		m_List.InsertColumn (i, &lvColumn );
	}

	return TRUE;  
}

void CPageEffect::DataSet( GLLandMan* pLand )
{
	m_pGLLand = pLand;
	DataShow();
}

void CPageEffect::ResetEditor()
{
	m_pGLLand = NULL;
}

void CPageEffect::DataShow()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	m_List.DeleteAllItems();

	if ( !m_pGLLand )	return;

	char szTempChar[512];
	int i = 0;

	m_List.SetRedraw( FALSE );

	PLANDEFF pCurLandEff = m_pGLLand->GetLandEffList ();
	while ( pCurLandEff )
	{
		m_List.InsertItem( i, "" );
		sprintf( szTempChar, "%d", i );
		m_List.SetItemText( i, 1, szTempChar );
		m_List.SetItemText( i, 2, pCurLandEff->m_szName );
		m_List.SetItemText( i, 3, pCurLandEff->m_szFileName );
		m_List.SetItemData( i, (DWORD_PTR)pCurLandEff );
		i++;

		pCurLandEff = pCurLandEff->m_pNext;
	}

	m_List.SetRedraw( TRUE );

	if ( nSelect == -1 && m_List.GetItemCount() > 0 )
		nSelect = 0;

	if ( nSelect != -1 )
	{
		m_List.EnsureVisible( nSelect, TRUE );
		m_List.SetItemState( nSelect, LVIS_SELECTED, LVIS_SELECTED );
	}
}

void CPageEffect::JumpTo( D3DXMATRIX sMat )
{
	D3DXVECTOR3 vPos = D3DXVECTOR3( sMat._41, sMat._42, sMat._43 );
	vPos.y += 10.0f;
	DxViewPort::GetInstance().CameraJump( vPos );
}

void CPageEffect::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pGLLand )	return;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	PLANDEFF pEff = (PLANDEFF)m_List.GetItemData( nSelect );
	if ( pEff )
		JumpTo( pEff->m_matWorld );
}

void CPageEffect::OnBnClickedButtonNew()
{
	if ( !m_pGLLand )	return;

	if ( m_pSheetTab )
		m_pSheetTab->PageEditEff( NULL );
}

void CPageEffect::OnBnClickedButtonEdit()
{
	if ( !m_pGLLand )	return;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	PLANDEFF pEff = (PLANDEFF)m_List.GetItemData( nSelect );
	if ( pEff )
	{
		if ( m_pSheetTab )
			m_pSheetTab->PageEditEff( pEff );
	}
}

void CPageEffect::OnBnClickedButtonDelete()
{
	if ( !m_pGLLand )	return;

	int nCHECKED = 0;
	for( int nItem = 0; nItem < m_List.GetItemCount(); nItem++ )
	{
		BOOL bChecked = m_List.GetCheck( nItem );
		if( bChecked )
		{
			PLANDEFF pEff = (PLANDEFF)m_List.GetItemData( nItem );
			if ( pEff )
			{
				m_pGLLand->DelLandEff( pEff );
				m_pGLLand->BuildSingleEffTree();
			}

			nCHECKED++;
		}	
	}

	if ( nCHECKED == 0 )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected Use Check boxes to select" );
	}

	DataShow();
}

void CPageEffect::OnBnClickedButtonRebuildname()
{
	if ( !m_pGLLand )	return;

	int nINDEX = 0;

	PLANDEFF pCurLandEff = m_pGLLand->GetLandEffList ();
	while ( pCurLandEff )
	{
		char szTempChar[DXLANDEFF::LANDEFF_MAXSZ];
		sprintf( szTempChar, "LANDEFF_%04d", nINDEX );
		StringCchCopy( pCurLandEff->m_szName, DXLANDEFF::LANDEFF_MAXSZ, szTempChar );
		nINDEX++;

		pCurLandEff = pCurLandEff->m_pNext;
	}

	DataShow();
}

void CPageEffect::OnBnClickedButtonSave()
{
	if ( m_pSheetTab )
		m_pSheetTab->PageActive( LEVELEDIT_PAGE_MAIN );
}
