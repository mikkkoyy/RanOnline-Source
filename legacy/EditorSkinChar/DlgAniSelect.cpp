// DlgAniSelect.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinChar.h"
#include "DlgAniSelect.h"


// CDlgAniSelect dialog

IMPLEMENT_DYNAMIC(CDlgAniSelect, CDialog)

CDlgAniSelect::CDlgAniSelect(CWnd* pParent /*=NULL*/, DxSkinChar* pSkin /*= NULL*/)
	: CDialog(CDlgAniSelect::IDD, pParent)
	, m_pSkin(NULL)
	, m_strAni("")
{
	m_pSkin = pSkin;
}

CDlgAniSelect::~CDlgAniSelect()
{
}

void CDlgAniSelect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SELECT_ANIM, m_List_Anim );
}


BEGIN_MESSAGE_MAP(CDlgAniSelect, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgAniSelect message handlers
BOOL CDlgAniSelect::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	RECT rectCtrl;
	m_List_Anim.SetExtendedStyle ( m_List_Anim.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Anim.GetClientRect ( &rectCtrl );
	m_List_Anim.InsertColumn( 0, "Num", LVCFMT_LEFT, ( 15* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Anim.InsertColumn( 1, "Animation", LVCFMT_LEFT, ( 60* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Anim.InsertColumn( 2, "Main/Sub", LVCFMT_LEFT, ( 25* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	m_List_Anim.DeleteAllItems();

	if ( m_pSkin )
	{
		m_List_Anim.SetRedraw( FALSE );

		DxSkinAniControl::VECNODE vecAnim = m_pSkin->GetAniList ();
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

	return TRUE;  
}

void CDlgAniSelect::OnBnClickedOk()
{
	if ( !m_pSkin )	return;

	int nSelect = m_List_Anim.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	std::string str = m_List_Anim.GetItemText( nSelect, 1 );
	m_pSkin->SELECTANI( str.c_str() );

	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}
