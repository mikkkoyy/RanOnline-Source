// CharEditPageSkills.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "CharEditPageSkills.h"


#include "CharEditTab.h"
#include "EtcFunction.h"
#include "HLibDataConvert.h"
#include "GLCharData.h"
#include "GLSkill.h"
#include "DlgSkillAdd.h"
#include "DlgSkillLevelEdit.h"

#include "Logic/GMToolGlobal.h"
#include "Logic/GMToolPresetSkill.h"

// CCharEditPageSkills dialog

IMPLEMENT_DYNAMIC(CCharEditPageSkills, CPropertyPage)

CCharEditPageSkills::CCharEditPageSkills(LOGFONT logfont, SCHARDATA2* pData)
	: CPropertyPage(CCharEditPageSkills::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CCharEditPageSkills::~CCharEditPageSkills()
{
	SAFE_DELETE ( m_pFont );
}

void CCharEditPageSkills::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHAREDIT_SKILLS_LIST, m_List);
	DDX_Control(pDX, IDC_CHAREDIT_SKILLS_COMBO_PRESET, m_cPreset );
}


BEGIN_MESSAGE_MAP(CCharEditPageSkills, CPropertyPage)
	ON_BN_CLICKED(IDC_CHAREDIT_SKILLS_BUTTON_ADD, OnBnClickedChareditSkillsButtonAdd)
	ON_BN_CLICKED(IDC_CHAREDIT_SKILLS_BUTTON_EDIT, OnBnClickedChareditSkillsButtonEdit)
	ON_BN_CLICKED(IDC_CHAREDIT_SKILLS_BUTTON_DELETE, OnBnClickedChareditSkillsButtonDelete)
	ON_BN_CLICKED(IDC_CHAREDIT_SKILLS_BUTTON_CLEAR, OnBnClickedChareditSkillsButtonClear)
	ON_NOTIFY(NM_DBLCLK, IDC_CHAREDIT_SKILLS_LIST, OnNMDblclkChareditSkillsList)
	ON_BN_CLICKED(IDC_CHAREDIT_SKILLS_BUTTON_PRESET, OnBnClickedChareditSkillsButtonPreset)
END_MESSAGE_MAP()


// CCharEditPageSkills message handlers
BOOL CCharEditPageSkills::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "", LVCFMT_LEFT, ( 5* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Skill", LVCFMT_LEFT, ( 70* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 2, "Level", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	SGMTOOL_PRESET_SKILL_MAP mapPreset = CGMToolPresetSkill::GetInstance().m_MapPresetSkill;
	SGMTOOL_PRESET_SKILL_MAP_ITER iter_start = mapPreset.begin();
	for( int i=0; iter_start != mapPreset.end(); ++iter_start, ++i ){
		const SGMTOOL_PRESET_SKILL sPRESET = (*iter_start).second;
		m_cPreset.InsertString( i, sPRESET.strNAME.c_str() );
		m_cPreset.SetItemData( i, sPRESET.wID );
	}
	if ( m_cPreset.GetCount() > 0 ){
		m_cPreset.SetCurSel(0);
	}

	DataShow();

	return TRUE;  
}

void CCharEditPageSkills::DataShow()
{
	m_List.DeleteAllItems();

	if ( !m_pData ){
		return;
	}

	m_List.SetRedraw( FALSE );

	SCHARDATA2::SKILL_MAP_ITER iter_start = m_pData->m_ExpSkills.begin();
	SCHARDATA2::SKILL_MAP_ITER iter_end = m_pData->m_ExpSkills.end();

	for ( int i=0; iter_start != iter_end; ++iter_start, ++i ){
		const SCHARSKILL &sSkill = (*iter_start).second;

		std::string strNAME = "Unknown Skill";
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( sSkill.sNativeID );
		if ( pSkill ){	
			strNAME = pSkill->GetName();
		}

		m_List.InsertItem( i, "" );
		m_List.SetItemText( i, 1, _HLIB::cstringformat( "[%03d/%03d] %s", sSkill.sNativeID.wMainID, sSkill.sNativeID.wSubID, strNAME.c_str() ).GetString() );
		m_List.SetItemText( i, 2, COMMENT::SKILL_LEVEL[sSkill.wLevel].c_str() );
		m_List.SetItemData ( i, sSkill.sNativeID.dwID );
	}

	m_List.SetRedraw( TRUE );
}

void CCharEditPageSkills::DataEdit()
{
	if ( !m_pData ){
		return;
	}

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = (DWORD) m_List.GetItemData ( nSelect );

	SCHARDATA2::SKILL_MAP_ITER iterSkill = m_pData->m_ExpSkills.find(dwID);
	if ( iterSkill != m_pData->m_ExpSkills.end() ){
		SCHARSKILL& sData = (*iterSkill).second;

		CDlgSkillLevelEdit dlg( &sData, this );
		if ( dlg.DoModal() == IDOK ){

			DataShow();

			for( int nItem = 0; nItem < m_List.GetItemCount(); nItem++){
				DWORD dwSkillID = m_List.GetItemData( nItem );
				if( dwSkillID == dwID ){
					m_List.EnsureVisible( nItem, TRUE );
					m_List.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
					GotoDlgCtrl(GetDlgItem(m_List.GetDlgCtrlID())); 
				}
			}
		}
	}
}

void CCharEditPageSkills::OnBnClickedChareditSkillsButtonAdd()
{
	if ( !m_pData ){
		return;
	}

	CDlgSkillAdd dlg( EMSKILLADD_TYPE_CHAR, m_pData->m_emClass, this );
	if ( dlg.DoModal() == IDOK ){
		SGMTOOL_SKILL_DATA_SELECTED_MAP mapSkills = dlg.m_mapSelect;
		SGMTOOL_SKILL_DATA_SELECTED_MAP_ITER iterData = mapSkills.begin();
		for( ; iterData != mapSkills.end(); ++iterData ){
			const SGMTOOL_SKILL_DATA_SELECTED sInsertData = (*iterData).second;
			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sInsertData.sID );
			if ( !pSkill ){
				continue;
			}

			SCHARDATA2::SKILL_MAP_ITER iterSkill = m_pData->m_ExpSkills.find(sInsertData.sID.dwID);
			if ( iterSkill != m_pData->m_ExpSkills.end() ){
				m_pData->m_ExpSkills.erase( iterSkill );
			}

			m_pData->m_ExpSkills.insert( std::make_pair(sInsertData.sID.dwID, SCHARSKILL(sInsertData.sID, sInsertData.wLEVEL)) );	
		}

		DataShow();
	}
}

void CCharEditPageSkills::OnBnClickedChareditSkillsButtonEdit()
{
	DataEdit();
}

void CCharEditPageSkills::OnBnClickedChareditSkillsButtonDelete()
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
			SCHARDATA2::SKILL_MAP_ITER iter = m_pData->m_ExpSkills.find ( dwID );
			if ( iter != m_pData->m_ExpSkills.end() ){
				m_pData->m_ExpSkills.erase( iter );
			}

			nCHECKED ++;
		}
	}

	if ( nCHECKED == 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Nothing selected Use Check boxes to select" );
	}

	DataShow();
}

void CCharEditPageSkills::OnBnClickedChareditSkillsButtonClear()
{
	if ( !m_pData ){
		return;
	}

	if ( CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Delete All Skills?" ) == IDYES ){
		m_pData->m_ExpSkills.clear();
		DataShow();
	}
}

void CCharEditPageSkills::OnNMDblclkChareditSkillsList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	DataEdit();
}

void CCharEditPageSkills::OnBnClickedChareditSkillsButtonPreset()
{
	if ( !m_pData ){
		return;
	}

	int nIndex = m_cPreset.GetCurSel();
	if ( nIndex == CB_ERR ){
		return;
	}

	WORD wID = (WORD)m_cPreset.GetItemData( nIndex );
	SGMTOOL_PRESET_SKILL* pPRESET = CGMToolPresetSkill::GetInstance().PresetSkillGet( wID );
	if ( pPRESET ){
		SGMTOOL_PRESET_SKILL_DATA_MAP_ITER iter_data = pPRESET->mapData.begin();
		for( ; iter_data != pPRESET->mapData.end(); ++ iter_data ){
			SGMTOOL_PRESET_SKILL_DATA sDATA = (*iter_data).second; 

			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sDATA.sSKILLID );
			if ( !pSkill ){
				continue;
			}

			if ( !(pSkill->m_sLEARN.dwCLASS&m_pData->m_emClass) ){
				continue;
			}

			GMTOOL_GLOBAL::SkillLevelCheck( sDATA.wLEVEL, sDATA.sSKILLID );

			SCHARDATA2::SKILL_MAP_ITER iterSkill = m_pData->m_ExpSkills.find(sDATA.sSKILLID.dwID);
			if ( iterSkill != m_pData->m_ExpSkills.end() ){
				m_pData->m_ExpSkills.erase( iterSkill );
			}

			m_pData->m_ExpSkills.insert( std::make_pair(sDATA.sSKILLID.dwID, SCHARSKILL(sDATA.sSKILLID, sDATA.wLEVEL)) );	
		}

		DataShow();
	}
}
