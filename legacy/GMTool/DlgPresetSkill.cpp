// DlgPresetSkill.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgPresetSkill.h"


#include "Logic/GMToolPresetSkill.h"
#include "Logic/GMToolGlobal.h"

#include "DlgPresetSkillEdit.h"
#include "HLibDataConvert.h"

// CDlgPresetSkill dialog

IMPLEMENT_DYNAMIC(CDlgPresetSkill, CDialog)

CDlgPresetSkill::CDlgPresetSkill(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPresetSkill::IDD, pParent)
{

}

CDlgPresetSkill::~CDlgPresetSkill()
{
}

void CDlgPresetSkill::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PRESET_SKILL, m_List );
}


BEGIN_MESSAGE_MAP(CDlgPresetSkill, CDialog)
	ON_BN_CLICKED(IDC_PRESET_SKILL_BUTTON_NEW, OnBnClickedPresetSkillButtonNew)
	ON_BN_CLICKED(IDC_PRESET_SKILL_BUTTON_EDIT, OnBnClickedPresetSkillButtonEdit)
	ON_BN_CLICKED(IDC_PRESET_SKILL_BUTTON_DELETE, OnBnClickedPresetSkillButtonDelete)
	ON_BN_CLICKED(IDC_PRESET_SKILL_BUTTON_CLEAR, OnBnClickedPresetSkillButtonClear)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PRESET_SKILL, OnNMDblclkListPresetSkill)
END_MESSAGE_MAP()


// CDlgPresetSkill message handlers
BOOL CDlgPresetSkill::OnInitDialog()
{
	CDialog::OnInitDialog();

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "ID", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Name", LVCFMT_LEFT, ( 75* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	DataShow();

	return TRUE;  
}

void CDlgPresetSkill::DataShow()
{
	m_List.DeleteAllItems();
	m_List.SetRedraw( FALSE );

	SGMTOOL_PRESET_SKILL_MAP mapPreset = CGMToolPresetSkill::GetInstance().m_MapPresetSkill;
	SGMTOOL_PRESET_SKILL_MAP_ITER iter_start = mapPreset.begin();

	for ( int i=0; iter_start != mapPreset.end(); ++iter_start, ++i ){
		const SGMTOOL_PRESET_SKILL &sPRESET = (*iter_start).second;

		m_List.InsertItem( i, _HLIB::cstring_dword( sPRESET.wID ).GetString() );
		m_List.SetItemText( i, 1, sPRESET.strNAME.c_str() );
		m_List.SetItemData ( i, sPRESET.wID );
	}

	m_List.SetRedraw( TRUE );
}

void CDlgPresetSkill::DataEdit()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	WORD wID = (WORD) m_List.GetItemData ( nSelect );

	SGMTOOL_PRESET_SKILL* pPreset = CGMToolPresetSkill::GetInstance().PresetSkillGet( wID );
	if ( pPreset ){
		CDlgPresetSkillEdit dlg( pPreset, this );
		if ( dlg.DoModal() == IDOK ){
			DataShow();

			for( int nItem=0; nItem<m_List.GetItemCount(); nItem++){
				DWORD dwID = m_List.GetItemData( nItem );
				if( dwID == wID ){
					m_List.EnsureVisible( nItem, TRUE );
					m_List.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
					GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
				}
			}
		}
	}
}

void CDlgPresetSkill::OnBnClickedPresetSkillButtonNew()
{
	WORD wID = CGMToolPresetSkill::GetInstance().PresetSkillNewID();
	if( wID == MAX_PRESET_SKILL_ID ){
		CDebugSet::MsgBox( GetSafeHwnd(), "No Vacant Preset ID! Max Preset:%d", MAX_PRESET_SKILL_ID );
		return;
	}

	SGMTOOL_PRESET_SKILL sPreset;
	sPreset.wID = wID;

	CDlgPresetSkillEdit dlg( &sPreset, this);
	if ( dlg.DoModal() == IDOK ){
		CGMToolPresetSkill::GetInstance().PresetSkillAdd( sPreset );

		DataShow();
		for( int nItem=0; nItem<m_List.GetItemCount(); nItem++){
			DWORD dwID = m_List.GetItemData( nItem );
			if( dwID == wID ){
				m_List.EnsureVisible( nItem, TRUE );
				m_List.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
				GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
			}
		}
	}
}

void CDlgPresetSkill::OnBnClickedPresetSkillButtonEdit()
{
	DataEdit();
}

void CDlgPresetSkill::OnBnClickedPresetSkillButtonDelete()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	WORD wID = (WORD) m_List.GetItemData ( nSelect );
	CGMToolPresetSkill::GetInstance().PresetSkillDel( wID );

	DataShow();

	int nNewSelect = nSelect > 1? nSelect-1:nSelect;
	m_List.EnsureVisible( nNewSelect, TRUE );
	m_List.SetItemState( nNewSelect, LVIS_SELECTED, LVIS_SELECTED );
	GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
}

void CDlgPresetSkill::OnBnClickedPresetSkillButtonClear()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All Skill Presets?" ) == IDYES ){
		CGMToolPresetSkill::GetInstance().PresetSkillDelAll();
		DataShow();
	}
}

void CDlgPresetSkill::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}

void CDlgPresetSkill::OnNMDblclkListPresetSkill(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	DataEdit();
}
