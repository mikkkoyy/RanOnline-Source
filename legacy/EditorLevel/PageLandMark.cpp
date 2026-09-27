// PageLandMark.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "PageLandMark.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "GLLandMark.h"

// CPageLandMark dialog

IMPLEMENT_DYNAMIC(CPageLandMark, CPropertyPage)

CPageLandMark::CPageLandMark(LOGFONT logfont)
	: CPropertyPage(CPageLandMark::IDD)
	, m_pFont ( NULL )
	, m_pGLLand( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageLandMark::~CPageLandMark()
{
	SAFE_DELETE ( m_pFont );
}

void CPageLandMark::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CPageLandMark, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_NEW, OnBnClickedButtonNew)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnBnClickedButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnBnClickedButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_REBUILDNAME, OnBnClickedButtonRebuildname)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnNMDblclkList)
END_MESSAGE_MAP()


// CPageLandMark message handlers
BOOL CPageLandMark::OnInitDialog()
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

	const int nColumnCount = 3;	
	char* szColumnName[nColumnCount] = { "", "Num", "MarkName" };
	int nColumnWidthPercent[nColumnCount] = { 5, 10, 85 };

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

void CPageLandMark::DataSet( GLLandMan* pLand )
{
	m_pGLLand = pLand;
	DataShow();
}

void CPageLandMark::ResetEditor()
{
	m_pGLLand = NULL;
}

void CPageLandMark::DataShow()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	m_List.DeleteAllItems();

	if ( !m_pGLLand )	return;

	char szTempChar[512];
	int i = 0;

	m_List.SetRedraw( FALSE );

	PLANDMARK pCur = m_pGLLand->GetLandMarkList();
	while ( pCur )
	{
		m_List.InsertItem( i, "" );
		sprintf( szTempChar, "%d", i );
		m_List.SetItemText( i, 1, szTempChar );
		m_List.SetItemText( i, 2, pCur->m_strMARK.c_str() );
		m_List.SetItemData( i, (DWORD_PTR)pCur );
		i++;

		pCur = pCur->m_pNext;
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

void CPageLandMark::JumpTo( D3DXVECTOR3 vPOS )
{
	vPOS.y += 10.0f;
	DxViewPort::GetInstance().CameraJump( vPOS );
}

void CPageLandMark::OnBnClickedButtonNew()
{
	if ( !m_pGLLand )	return;

	if ( m_pSheetTab )
		m_pSheetTab->PageEditLandMark( NULL );
}

void CPageLandMark::OnBnClickedButtonEdit()
{
	if ( !m_pGLLand )	return;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ) return;

	PLANDMARK pLandMark = (PLANDMARK)m_List.GetItemData( nSelect );
	if ( pLandMark )
	{
		if ( m_pSheetTab )
			m_pSheetTab->PageEditLandMark( pLandMark );
	}
}

void CPageLandMark::OnBnClickedButtonDelete()
{
	if ( !m_pGLLand )	return;

	int nCHECKED = 0;
	for( int nItem = 0; nItem < m_List.GetItemCount(); nItem++ )
	{
		BOOL bChecked = m_List.GetCheck( nItem );
		if( bChecked )
		{
			PLANDMARK pLandMark = (PLANDMARK)m_List.GetItemData( nItem );
			if ( pLandMark )
			{
				m_pGLLand->DelLandMark( pLandMark );
				m_pGLLand->BuildLandMarkTree();
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

void CPageLandMark::OnBnClickedButtonRebuildname()
{
	if ( !m_pGLLand )	return;

	int nINDEX = 0;

	PLANDMARK pCur = m_pGLLand->GetLandMarkList();
	while ( pCur )
	{
		CString strTEMP;
		strTEMP.Format( "LANDMARK_%04d", nINDEX );
		pCur->m_strMARK = strTEMP.GetString();

		nINDEX++;

		pCur = pCur->m_pNext;
	}

	DataShow();
}

void CPageLandMark::OnBnClickedButtonSave()
{
	if ( m_pSheetTab )
		m_pSheetTab->PageActive( LEVELEDIT_PAGE_MAIN );
}

void CPageLandMark::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pGLLand )	return;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED );
	if( nSelect == -1 ) return;

	PLANDMARK pLandMark = (PLANDMARK)m_List.GetItemData( nSelect );
	if ( pLandMark )
	{
		D3DXVECTOR3 vPOS = pLandMark->m_pAffineParts->vTrans;
		JumpTo( vPOS );
	}
}
