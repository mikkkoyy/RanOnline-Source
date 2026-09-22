// DlgPresetSkillEdit.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgPresetSkillEdit.h"

#include "EtcFunction.h"
#include "HLibDataConvert.h"
#include "Logic/GMToolData.h"
#include "Logic/GMToolGlobal.h"
#include "GLSkill.h"
#include "DlgSkill.h"

// CDlgPresetSkillEdit dialog

IMPLEMENT_DYNAMIC(CDlgPresetSkillEdit, CDialog)

CDlgPresetSkillEdit::CDlgPresetSkillEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPresetSkillEdit::IDD, pParent)
	, m_pData( NULL )
{

}

CDlgPresetSkillEdit::CDlgPresetSkillEdit(SGMTOOL_PRESET_SKILL* pData, CWnd* pParent /*= NULL*/)
	: CDialog(CDlgPresetSkillEdit::IDD, pParent)
	, m_pData( NULL )
{
	m_pData = pData;
}

CDlgPresetSkillEdit::~CDlgPresetSkillEdit()
{
}

void CDlgPresetSkillEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PRESET_EDIT_SKILL, m_List );
}


BEGIN_MESSAGE_MAP(CDlgPresetSkillEdit, CDialog)
	ON_BN_CLICKED(IDC_PRESET_SKILL_EDIT_BUTTON_NEW, OnBnClickedPresetSkillEditButtonNew)
	ON_BN_CLICKED(IDC_PRESET_SKILL_EDIT_BUTTON_DELETE, OnBnClickedPresetSkillEditButtonDelete)
	ON_BN_CLICKED(IDC_PRESET_SKILL_EDIT_BUTTON_CLEAR, OnBnClickedPresetSkillEditButtonClear)
	ON_BN_CLICKED(IDC_PRESET_SKILL_EDIT_BUTTON_SKILL, OnBnClickedPresetSkillEditButtonSkill)
	ON_BN_CLICKED(IDC_PRESET_SKILL_EDIT_BUTTON_SAVE, OnBnClickedPresetSkillEditButtonSave)
	ON_BN_CLICKED(IDC_PRESET_SKILL_EDIT_BUTTON_CANCEL, OnBnClickedPresetSkillEditButtonCancel)
	ON_BN_CLICKED(IDC_PRESET_SKILL_EDIT_BUTTON_CLOSE, OnBnClickedPresetSkillEditButtonClose)
	ON_EN_CHANGE(IDC_PRESET_SKILL_EDIT_EDIT_SKILL_MID, OnEnChangePresetSkillEditEditSkillMid)
	ON_EN_CHANGE(IDC_PRESET_SKILL_EDIT_EDIT_SKILL_SID, OnEnChangePresetSkillEditEditSkillSid)
END_MESSAGE_MAP()


// CDlgPresetSkillEdit message handlers
BOOL CDlgPresetSkillEdit::OnInitDialog()
{
	CDialog::OnInitDialog();

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "Skill", LVCFMT_LEFT, ( 75* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Level", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	SetWin_Combo_Init( this, IDC_PRESET_SKILL_EDIT_COMBO_SKILLLEVEL, COMMENT::SKILL_LEVEL, SKILL::MAX_LEVEL );

	if ( m_pData ){
		SetWin_Num_int( this, IDC_PRESET_SKILL_EDIT_EDIT_ID, m_pData->wID );
		SetWin_Text( this, IDC_PRESET_SKILL_EDIT_EDIT_NAME, m_pData->strNAME.c_str() );
	}
	
	DataShow();

	return TRUE;  
}

void CDlgPresetSkillEdit::DataShow()
{
	m_List.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_List.SetRedraw( FALSE );

	SGMTOOL_PRESET_SKILL_DATA_MAP_ITER iter_start = m_pData->mapData.begin();

	for ( int i=0; iter_start != m_pData->mapData.end(); ++iter_start, ++i ){
		const SGMTOOL_PRESET_SKILL_DATA &sData = (*iter_start).second;

		std::string strName = "Unknown Skill";
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sData.sSKILLID );
		if ( pSKILL ){
			strName = pSKILL->GetName();
		}
		m_List.InsertItem( i, _HLIB::cstringformat( "[%03d~%03d] %s", sData.sSKILLID.wMainID, sData.sSKILLID.wSubID, strName.c_str() ).GetString() );
		m_List.SetItemText( i, 1, COMMENT::SKILL_LEVEL[sData.wLEVEL].c_str() );
		m_List.SetItemData ( i, sData.sSKILLID.dwID );
	}

	m_List.SetRedraw( TRUE );

	SetWin_Enable( this, IDC_PRESET_SKILL_EDIT_BUTTON_SKILL, FALSE );
	SetWin_Enable( this, IDC_PRESET_SKILL_EDIT_BUTTON_SAVE, FALSE );
	SetWin_Enable( this, IDC_PRESET_SKILL_EDIT_BUTTON_CANCEL, FALSE );
	SetWin_Enable( this, IDC_PRESET_SKILL_EDIT_EDIT_SKILL_MID, FALSE );
	SetWin_Enable( this, IDC_PRESET_SKILL_EDIT_EDIT_SKILL_SID, FALSE );
	SetWin_Enable( this, IDC_PRESET_SKILL_EDIT_COMBO_SKILLLEVEL, FALSE );

	SetWin_Num_int( this, IDC_PRESET_SKILL_EDIT_EDIT_SKILL_MID, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_PRESET_SKILL_EDIT_EDIT_SKILL_SID, NATIVEID_NULL().wSubID );
	SetWin_Combo_Sel( this, IDC_PRESET_SKILL_EDIT_COMBO_SKILLLEVEL, 0 );
}

BOOL CDlgPresetSkillEdit::DataSave()
{
	if ( !m_pData ){
		return FALSE;
	}

	std::string strName = GetWin_Text( this, IDC_PRESET_SKILL_EDIT_EDIT_NAME );
	if ( strName.size() <= 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Name Empty!" );
		GotoDlgCtrl(GetDlgItem(IDC_PRESET_SKILL_EDIT_EDIT_NAME)); 
		return FALSE;
	}

	m_pData->strNAME = strName.c_str();

	return TRUE;
}

void CDlgPresetSkillEdit::OnBnClickedPresetSkillEditButtonNew()
{
	if ( !m_pData ){
		return;
	}

	SetWin_Enable( this, IDC_PRESET_SKILL_EDIT_BUTTON_SKILL, TRUE );
	SetWin_Enable( this, IDC_PRESET_SKILL_EDIT_BUTTON_SAVE, TRUE );
	SetWin_Enable( this, IDC_PRESET_SKILL_EDIT_BUTTON_CANCEL, TRUE );
	SetWin_Enable( this, IDC_PRESET_SKILL_EDIT_EDIT_SKILL_MID, TRUE );
	SetWin_Enable( this, IDC_PRESET_SKILL_EDIT_EDIT_SKILL_SID, TRUE );
	SetWin_Enable( this, IDC_PRESET_SKILL_EDIT_COMBO_SKILLLEVEL, TRUE );

	SetWin_Num_int( this, IDC_PRESET_SKILL_EDIT_EDIT_SKILL_MID, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_PRESET_SKILL_EDIT_EDIT_SKILL_SID, NATIVEID_NULL().wSubID );
	SetWin_Combo_Sel( this, IDC_PRESET_SKILL_EDIT_COMBO_SKILLLEVEL, 0 );
}

void CDlgPresetSkillEdit::OnBnClickedPresetSkillEditButtonDelete()
{
	if ( !m_pData ){
		return;
	}

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ){
		return;
	}

	DWORD dwID = (DWORD) m_List.GetItemData ( nSelect );

	SGMTOOL_PRESET_SKILL_DATA_MAP_ITER iterData = m_pData->mapData.find(dwID);
	if ( iterData != m_pData->mapData.end() ){
		m_pData->mapData.erase( iterData );
		DataShow();

		int nNewSelect = nSelect > 1? nSelect-1:nSelect;
		m_List.EnsureVisible( nNewSelect, TRUE );
		m_List.SetItemState( nNewSelect, LVIS_SELECTED, LVIS_SELECTED );
		GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
	}
}

void CDlgPresetSkillEdit::OnBnClickedPresetSkillEditButtonClear()
{
	if ( !m_pData ){
		return;
	}

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All Skill Data?" ) == IDYES ){
		m_pData->mapData.clear();
		DataShow();
	}
}

void CDlgPresetSkillEdit::OnBnClickedPresetSkillEditButtonSkill()
{
	if ( !m_pData ){
		return;
	}

	CDlgSkill dlg( this );
	if ( dlg.DoModal() ==  IDOK ){
		SNATIVEID sID = dlg.m_SELECTED;
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sID );
		if ( !pSKILL ){
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Skill %03d~%03d", sID.wMainID, sID.wSubID );
			return;
		}

		SetWin_Num_int( this, IDC_PRESET_SKILL_EDIT_EDIT_SKILL_MID, sID.wMainID );
		SetWin_Num_int( this, IDC_PRESET_SKILL_EDIT_EDIT_SKILL_SID, sID.wSubID );
	}
}

void CDlgPresetSkillEdit::OnBnClickedPresetSkillEditButtonSave()
{
	if ( !m_pData ){
		return;
	}

	WORD wMID = GetWin_Num_int( this, IDC_PRESET_SKILL_EDIT_EDIT_SKILL_MID );
	WORD wSID = GetWin_Num_int( this, IDC_PRESET_SKILL_EDIT_EDIT_SKILL_SID );
	WORD wLEVEL = (WORD)GetWin_Combo_Sel( this, IDC_PRESET_SKILL_EDIT_COMBO_SKILLLEVEL );

	SNATIVEID sID( wMID, wSID );

	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sID );
	if ( !pSKILL ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Skill %03d~%03d", sID.wMainID, sID.wSubID );
		return;
	}

	GMTOOL_GLOBAL::SkillLevelCheck( wLEVEL, sID );

	SGMTOOL_PRESET_SKILL_DATA_MAP_ITER iterData = m_pData->mapData.find( sID.dwID);
	if ( iterData != m_pData->mapData.end() ){
		m_pData->mapData.erase( iterData );
	}

	SGMTOOL_PRESET_SKILL_DATA sDATA;
	sDATA.sSKILLID = sID;
	sDATA.wLEVEL = wLEVEL;

	m_pData->mapData.insert( std::make_pair( sDATA.sSKILLID.dwID, sDATA ) );

	DataShow();

	for( int nItem=0; nItem<m_List.GetItemCount(); nItem++){
		DWORD dwID = m_List.GetItemData( nItem );
		if( dwID == sDATA.sSKILLID.dwID ){
			m_List.EnsureVisible( nItem, TRUE );
			m_List.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
			GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
		}
	}
}

void CDlgPresetSkillEdit::OnBnClickedPresetSkillEditButtonCancel()
{
	DataShow();
}

void CDlgPresetSkillEdit::OnBnClickedPresetSkillEditButtonClose()
{
	if ( DataSave() ){
		CDialog::OnOK();
	}
}

void CDlgPresetSkillEdit::OnEnChangePresetSkillEditEditSkillMid()
{
	GMTOOL_GLOBAL::DataCheckSkill( this, 
		IDC_PRESET_SKILL_EDIT_EDIT_SKILL_MID, 
		IDC_PRESET_SKILL_EDIT_EDIT_SKILL_SID, 
		IDC_PRESET_SKILL_EDIT_EDIT_SKILL_NAME );
}

void CDlgPresetSkillEdit::OnEnChangePresetSkillEditEditSkillSid()
{
	GMTOOL_GLOBAL::DataCheckSkill( this, 
		IDC_PRESET_SKILL_EDIT_EDIT_SKILL_MID, 
		IDC_PRESET_SKILL_EDIT_EDIT_SKILL_SID, 
		IDC_PRESET_SKILL_EDIT_EDIT_SKILL_NAME );
}
