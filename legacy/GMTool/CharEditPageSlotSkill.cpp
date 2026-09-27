// CharEditPageSlotSkill.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "CharEditPageSlotSkill.h"


#include "CharEditTab.h"
#include "GLCharData.h"
#include "GLSkill.h"
#include "HLibDataConvert.h"
#include "DlgSkillList.h"

// CCharEditPageSlotSkill dialog

IMPLEMENT_DYNAMIC(CCharEditPageSlotSkill, CPropertyPage)

CCharEditPageSlotSkill::CCharEditPageSlotSkill(LOGFONT logfont, SCHARDATA2* pData)
	: CPropertyPage(CCharEditPageSlotSkill::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CCharEditPageSlotSkill::~CCharEditPageSlotSkill()
{
	SAFE_DELETE ( m_pFont );
}

void CCharEditPageSlotSkill::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHAREDIT_SKILLSLOT_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CCharEditPageSlotSkill, CPropertyPage)
	ON_BN_CLICKED(IDC_CHAREDIT_SKILLSLOT_BUTTON_EDIT, OnBnClickedChareditSkillslotButtonEdit)
	ON_BN_CLICKED(IDC_CHAREDIT_SKILLSLOT_BUTTON_DELETE, OnBnClickedChareditSkillslotButtonDelete)
	ON_BN_CLICKED(IDC_CHAREDIT_SKILLSLOT_BUTTON_CLEAR, OnBnClickedChareditSkillslotButtonClear)
	ON_NOTIFY(NM_DBLCLK, IDC_CHAREDIT_SKILLSLOT_LIST, OnNMDblclkChareditSkillslotList)
END_MESSAGE_MAP()


// CCharEditPageSlotSkill message handlers
BOOL CCharEditPageSlotSkill::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "", LVCFMT_LEFT, ( 5* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Slot", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 2, "Skill", LVCFMT_LEFT, ( 70* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	DataShow();

	return TRUE;  
}

void CCharEditPageSlotSkill::DataShow()
{
	m_List.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_List.SetRedraw( FALSE );

	for( int i=0; i<EMSKILLQUICK_SIZE; ++i )
	{
		SNATIVEID sID = m_pData->m_sSKILLQUICK[i];

		std::string strName = "Unknown Skill";
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( sID );
		if ( pSkill ){
			strName = pSkill->GetName();
		}

		m_List.InsertItem( i, "" );
		m_List.SetItemText( i, 1, _HLIB::cstringformat( "Slot %02d", i ).GetString() );
		m_List.SetItemText( i, 2, _HLIB::cstringformat( "[%03d/%03d] %s", sID.wMainID, sID.wSubID, strName.c_str() ).GetString() );
		m_List.SetItemData( i, i );
	}

	m_List.SetRedraw( TRUE );
}

void CCharEditPageSlotSkill::DataEdit()
{
	if ( !m_pData ){
		return;
	}

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwSLOT = (DWORD) m_List.GetItemData ( nSelect );

	if ( dwSLOT >= EMSKILLQUICK_SIZE ){
		return;
	}

	CDlgSkillList dlg( m_pData, this );
	if( dlg.DoModal() == IDOK ){

		SNATIVEID sID = dlg.m_SELECTED;
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sID );
		if ( pSKILL ){
			if ( pSKILL->m_sBASIC.emROLE == SKILL::EMROLE_PASSIVE ){
				CDebugSet::MsgBox( GetSafeHwnd(), "Selected skill is passive and cannot be used on slot!" );
				return;
			}

			m_pData->m_sSKILLQUICK[dwSLOT] = dlg.m_SELECTED;
			DataShow();

			for( int nItem=0; nItem<m_List.GetItemCount(); nItem++){
				DWORD dwID = m_List.GetItemData( nItem );
				if( dwID == dwSLOT ){
					m_List.EnsureVisible( nItem, TRUE );
					m_List.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
					GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
				}
			}
		}
	}
}

void CCharEditPageSlotSkill::OnBnClickedChareditSkillslotButtonEdit()
{
	DataEdit();
}

void CCharEditPageSlotSkill::OnBnClickedChareditSkillslotButtonDelete()
{
	if ( !m_pData ){
		return;
	}

	int nCHECKED = 0;
	for( int nItem=0; nItem<m_List.GetItemCount(); nItem++ )
	{
		BOOL bChecked = m_List.GetCheck( nItem );
		if( bChecked){
			DWORD dwID = (DWORD) m_List.GetItemData ( nItem );
			m_pData->m_sSKILLQUICK[dwID] = NATIVEID_NULL();
			nCHECKED ++;
		}
	}

	if ( nCHECKED == 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected Use Check boxes to select" );
	}

	DataShow();
}

void CCharEditPageSlotSkill::OnBnClickedChareditSkillslotButtonClear()
{
	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All Slot Data?" ) == IDYES ){
		for( int i=0; i<EMSKILLQUICK_SIZE; ++i ){
			m_pData->m_sSKILLQUICK[i] = NATIVEID_NULL();
		}

		DataShow();
	}
}

void CCharEditPageSlotSkill::OnNMDblclkChareditSkillslotList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	DataEdit();
}
