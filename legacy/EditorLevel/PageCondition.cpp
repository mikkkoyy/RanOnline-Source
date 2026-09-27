// PageCondition.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "PageCondition.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "GLItemMan.h"
#include "GLItem.h"
#include "GLSkill.h"
#include "GLQuest.h"
#include "GLQuestMan.h"

#include "DlgFindItem.h"
#include "DlgFindSkill.h"
#include "DlgFindQuest.h"
#include ".\pagecondition.h"

// CPageCondition dialog

IMPLEMENT_DYNAMIC(CPageCondition, CPropertyPage)

CPageCondition::CPageCondition(LOGFONT logfont)
	: CPropertyPage(CPageCondition::IDD)
	, m_pFont ( NULL )
	, m_pGLLand( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageCondition::~CPageCondition()
{
	SAFE_DELETE ( m_pFont );
}

void CPageCondition::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageCondition, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_REQLEVEL, OnBnClickedButtonReqlevel)
	ON_BN_CLICKED(IDC_BUTTON_REQITEM, OnBnClickedButtonReqitem)
	ON_BN_CLICKED(IDC_BUTTON_REQSKILL, OnBnClickedButtonReqskill)
	ON_BN_CLICKED(IDC_BUTTON_REQCOMQUEST, OnBnClickedButtonReqcomquest)
	ON_BN_CLICKED(IDC_BUTTON_REQACTQUEST, OnBnClickedButtonReqactquest)
	ON_BN_CLICKED(IDC_BUTTON_REQLIFE, OnBnClickedButtonReqlife)
	ON_BN_CLICKED(IDC_BUTTON_REQBRIGHT, OnBnClickedButtonReqbright)
	ON_EN_CHANGE(IDC_EDIT_REQITEM_MID, OnEnChangeEditReqitemMid)
	ON_EN_CHANGE(IDC_EDIT_REQITEM_SID, OnEnChangeEditReqitemSid)
	ON_EN_CHANGE(IDC_EDIT_REQSKILL_MID, OnEnChangeEditReqskillMid)
	ON_EN_CHANGE(IDC_EDIT_REQSKILL_SID, OnEnChangeEditReqskillSid)
	ON_EN_CHANGE(IDC_EDIT_REQCOMQUESTID, OnEnChangeEditReqcomquestid)
	ON_EN_CHANGE(IDC_EDIT_REQACTQUESTID, OnEnChangeEditReqactquestid)
	ON_BN_CLICKED(IDC_BUTTON_REQITEM_REMOVE, OnBnClickedButtonReqitemRemove)
	ON_BN_CLICKED(IDC_BUTTON_REQSKILL_REMOVE, OnBnClickedButtonReqskillRemove)
	ON_BN_CLICKED(IDC_BUTTON_REQCOMQUEST_REMOVE, OnBnClickedButtonReqcomquestRemove)
	ON_BN_CLICKED(IDC_BUTTON_REQACTQUEST_REMOVE, OnBnClickedButtonReqactquestRemove)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_RESET, OnBnClickedButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_REQ_ACT_P, OnBnClickedButtonReqActP)
	ON_BN_CLICKED(IDC_BUTTON_REQ_CON_P, OnBnClickedButtonReqConP)
END_MESSAGE_MAP()


// CPageCondition message handlers
BOOL CPageCondition::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	SetWin_Combo_Init( this, IDC_COMBO_REQLEVEL, COMMENT::CDT_SIGN, EMSIGN_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_REQLIFE, COMMENT::CDT_SIGN, EMSIGN_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_REQBRIGHT, COMMENT::CDT_SIGN, EMSIGN_SIZE );

	/* map requirement contri and activity p, Juver, 2018/02/11 */
	SetWin_Combo_Init( this, IDC_COMBO_REQ_ACT_P, COMMENT::CDT_SIGN, EMSIGN_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_REQ_CON_P, COMMENT::CDT_SIGN, EMSIGN_SIZE );

	return TRUE;  
}

void CPageCondition::DataSet( GLLandMan* pLand )
{
	m_pGLLand = pLand;
	DataShow();
}

void CPageCondition::ResetEditor()
{
	m_pGLLand = NULL;
}

void CPageCondition::DataShow()
{
	if ( !m_pGLLand )	return;

	SLEVEL_REQUIRE* pREQ = m_pGLLand->GetLevelRequire();
	if ( !pREQ )	return;

	SetWin_Combo_Sel( this, IDC_COMBO_REQLEVEL, COMMENT::CDT_SIGN[pREQ->m_signLevel].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_REQLIFE, COMMENT::CDT_SIGN[pREQ->m_signLiving].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_REQBRIGHT, COMMENT::CDT_SIGN[pREQ->m_signBright].c_str() );
	SetWin_Num_int( this, IDC_EDIT_REQLEVEL_LOW, pREQ->m_wLevel );
	SetWin_Num_int( this, IDC_EDIT_REQLEVEL_HIGH, pREQ->m_wLevel2 );
	SetWin_Num_int( this, IDC_EDIT_REQITEM_MID, pREQ->m_sItemID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_REQITEM_SID, pREQ->m_sItemID.wSubID );
	SetWin_Num_int( this, IDC_EDIT_REQSKILL_MID, pREQ->m_sSkillID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_REQSKILL_SID, pREQ->m_sSkillID.wSubID );
	SetWin_Num_int( this, IDC_EDIT_REQCOMQUESTID, pREQ->m_sComQuestID.dwID );
	SetWin_Num_int( this, IDC_EDIT_REQACTQUESTID, pREQ->m_sActQuestID.dwID );
	SetWin_Num_int( this, IDC_EDIT_REQLIFE, pREQ->m_nLiving );
	SetWin_Num_int( this, IDC_EDIT_REQBRIGHT, pREQ->m_nBright );
	SetWin_Check( this, IDC_CHECK_PARTYMEMBER, pREQ->m_bPartyMbr );

	/* map requirement contri and activity p, Juver, 2018/02/11 */
	SetWin_Combo_Sel( this, IDC_COMBO_REQ_ACT_P, COMMENT::CDT_SIGN[pREQ->m_signActivityPoint].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_REQ_CON_P, COMMENT::CDT_SIGN[pREQ->m_signContributionPoint].c_str() );
	SetWin_Num_int( this, IDC_EDIT_REQ_ACT_P, pREQ->m_dwActivityPoint );
	SetWin_Num_int( this, IDC_EDIT_REQ_CON_P, pREQ->m_dwContributionPoint );
}

BOOL CPageCondition::DataSave()
{
	if ( !m_pGLLand )	return FALSE; 

	SLEVEL_REQUIRE* pREQ = m_pGLLand->GetLevelRequire();
	if ( !pREQ )	return FALSE;

	WORD wITEMMID = GetWin_Num_int( this, IDC_EDIT_REQITEM_MID );
	WORD wITEMSID = GetWin_Num_int( this, IDC_EDIT_REQITEM_SID );
	WORD wSKILLMID = GetWin_Num_int( this, IDC_EDIT_REQSKILL_MID );
	WORD wSKILLSID = GetWin_Num_int( this, IDC_EDIT_REQSKILL_SID );
	DWORD dwCOMQUESTID = GetWin_Num_int( this, IDC_EDIT_REQCOMQUESTID );
	DWORD dwACTQUESTID = GetWin_Num_int( this, IDC_EDIT_REQACTQUESTID );

	if ( wITEMMID != SNATIVEID::ID_NULL && wITEMSID != SNATIVEID::ID_NULL )
	{
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( wITEMMID, wITEMSID );
		if( !pITEM )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Item( %d~%d )", wITEMMID, wITEMSID );
			return FALSE;
		}
	}

	if ( wSKILLMID != SNATIVEID::ID_NULL && wSKILLSID != SNATIVEID::ID_NULL )
	{
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( wSKILLMID, wSKILLSID );
		if( !pSKILL )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Skill( %d~%d )", wSKILLMID, wSKILLSID );
			return FALSE;
		}
	}

	if ( dwCOMQUESTID  != NATIVEID_NULL().dwID )
	{
		GLQUEST* pQUEST = GLQuestMan::GetInstance().Find( dwCOMQUESTID );
		if ( !pQUEST )
		{	
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Quest( %d )", dwCOMQUESTID );
			return FALSE;
		}
	}

	if ( dwACTQUESTID != NATIVEID_NULL().dwID )
	{
		GLQUEST* pQUEST = GLQuestMan::GetInstance().Find( dwACTQUESTID );
		if ( !pQUEST )
		{	
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Quest( %d )", dwACTQUESTID );
			return FALSE;
		}
	}

	pREQ->m_signLevel = static_cast<EMCDT_SIGN>( GetWin_Combo_Sel( this, IDC_COMBO_REQLEVEL ) );
	pREQ->m_signLiving = static_cast<EMCDT_SIGN>( GetWin_Combo_Sel( this, IDC_COMBO_REQLIFE ) );
	pREQ->m_signBright = static_cast<EMCDT_SIGN>( GetWin_Combo_Sel( this, IDC_COMBO_REQBRIGHT ) );
	pREQ->m_wLevel = GetWin_Num_int( this, IDC_EDIT_REQLEVEL_LOW );
	pREQ->m_wLevel2 = GetWin_Num_int( this, IDC_EDIT_REQLEVEL_HIGH );
	pREQ->m_nLiving = GetWin_Num_int( this, IDC_EDIT_REQLIFE );
	pREQ->m_nBright = GetWin_Num_int( this, IDC_EDIT_REQBRIGHT );
	pREQ->m_bPartyMbr = ( GetWin_Check( this, IDC_CHECK_PARTYMEMBER ) == TRUE );

	/* map requirement contri and activity p, Juver, 2018/02/11 */
	pREQ->m_signActivityPoint = static_cast<EMCDT_SIGN>( GetWin_Combo_Sel( this, IDC_COMBO_REQ_ACT_P ) );
	pREQ->m_signContributionPoint = static_cast<EMCDT_SIGN>( GetWin_Combo_Sel( this, IDC_COMBO_REQ_CON_P ) );
	pREQ->m_dwActivityPoint = GetWin_Num_int( this, IDC_EDIT_REQ_ACT_P );
	pREQ->m_dwContributionPoint = GetWin_Num_int( this, IDC_EDIT_REQ_CON_P );

	pREQ->m_sItemID.wMainID = wITEMMID;
	pREQ->m_sItemID.wSubID = wITEMSID;
	pREQ->m_sSkillID.wMainID = wSKILLMID;
	pREQ->m_sSkillID.wSubID = wSKILLSID;
	pREQ->m_sComQuestID.dwID = dwCOMQUESTID;
	pREQ->m_sActQuestID.dwID = dwACTQUESTID;

	return TRUE;
}

void CPageCondition::ShowItemName()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_REQITEM_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_REQITEM_SID );

	std::string strITEMNAME = "Unknown Item";
	SITEM* pITEM = GLItemMan::GetInstance().GetItem( wMID, wSID );
	if ( pITEM )
		strITEMNAME = pITEM->GetName();

	SetWin_Text( this, IDC_EDIT_REQITEM_NAME, strITEMNAME.c_str() );
}

void CPageCondition::ShowSkillName()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_REQSKILL_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_REQSKILL_SID );

	std::string strSKILLNAME = "Unknown Skill";
	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( wMID, wSID );
	if ( pSKILL )
		strSKILLNAME = pSKILL->GetName();

	SetWin_Text( this, IDC_EDIT_REQSKILL_NAME, strSKILLNAME.c_str() );
}

void CPageCondition::ShowQuestName( int nID_IN, int nID_OUT )
{
	DWORD dwID = GetWin_Num_int( this, nID_IN );

	std::string strQUESTNAME = "Unknown Quest";
	GLQUEST* pQUEST = GLQuestMan::GetInstance().Find( dwID );
	if ( pQUEST )
		strQUESTNAME = pQUEST->GetTITLE();

	SetWin_Text( this, nID_OUT, strQUESTNAME.c_str() );
}

void CPageCondition::OnBnClickedButtonReqlevel()
{
}

void CPageCondition::OnBnClickedButtonReqitem()
{
	CDlgFindItem dlg( this );
	if ( dlg.DoModal() == IDOK )
	{
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( dlg.m_sSELECTED );
		if( pITEM )
		{
			SetWin_Num_int( this, IDC_EDIT_REQITEM_MID, pITEM->sBasicOp.sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_REQITEM_SID, pITEM->sBasicOp.sNativeID.wSubID );
		}
	}
}

void CPageCondition::OnBnClickedButtonReqskill()
{
	CDlgFindSkill dlg( this );
	if ( dlg.DoModal() == IDOK )
	{
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( dlg.m_sSELECTED );
		if( pSKILL )
		{
			SetWin_Num_int( this, IDC_EDIT_REQSKILL_MID, pSKILL->m_sBASIC.sNATIVEID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_REQSKILL_SID, pSKILL->m_sBASIC.sNATIVEID.wSubID );
		}
	}
}

void CPageCondition::OnBnClickedButtonReqcomquest()
{
	CDlgFindQuest dlg( this );
	if ( dlg.DoModal() == IDOK )
	{
		GLQUEST* pQUEST = GLQuestMan::GetInstance().Find( dlg.m_sSelected.dwQUESTID );
		if ( pQUEST )
		{
			SetWin_Num_int( this, IDC_EDIT_REQCOMQUESTID, pQUEST->m_sNID.dwID );
		}
	}
}

void CPageCondition::OnBnClickedButtonReqactquest()
{
	CDlgFindQuest dlg( this );
	if ( dlg.DoModal() == IDOK )
	{
		GLQUEST* pQUEST = GLQuestMan::GetInstance().Find( dlg.m_sSelected.dwQUESTID );
		if ( pQUEST )
		{
			SetWin_Num_int( this, IDC_EDIT_REQACTQUESTID, pQUEST->m_sNID.dwID );
		}
	}
}

void CPageCondition::OnBnClickedButtonReqlife()
{
}

void CPageCondition::OnBnClickedButtonReqbright()
{
}

void CPageCondition::OnEnChangeEditReqitemMid()
{
	ShowItemName();
}

void CPageCondition::OnEnChangeEditReqitemSid()
{
	ShowItemName();
}

void CPageCondition::OnEnChangeEditReqskillMid()
{
	ShowSkillName();
}

void CPageCondition::OnEnChangeEditReqskillSid()
{
	ShowSkillName();
}

void CPageCondition::OnEnChangeEditReqcomquestid()
{
	ShowQuestName( IDC_EDIT_REQCOMQUESTID, IDC_EDIT_REQCOMQUEST_NAME );
}

void CPageCondition::OnEnChangeEditReqactquestid()
{
	ShowQuestName( IDC_EDIT_REQACTQUESTID, IDC_EDIT_REQACTQUEST_NAME );
}

void CPageCondition::OnBnClickedButtonReqitemRemove()
{
	SNATIVEID sID( false );
	SetWin_Num_int( this, IDC_EDIT_REQITEM_MID, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_REQITEM_SID, sID.wSubID );
}

void CPageCondition::OnBnClickedButtonReqskillRemove()
{
	SNATIVEID sID( false );
	SetWin_Num_int( this, IDC_EDIT_REQSKILL_MID, sID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_REQSKILL_SID, sID.wSubID );
}

void CPageCondition::OnBnClickedButtonReqcomquestRemove()
{
	SNATIVEID sID( false );
	SetWin_Num_int( this, IDC_EDIT_REQCOMQUESTID, sID.dwID );
}

void CPageCondition::OnBnClickedButtonReqactquestRemove()
{
	SNATIVEID sID( false );
	SetWin_Num_int( this, IDC_EDIT_REQACTQUESTID, sID.dwID );
}

void CPageCondition::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		if ( m_pSheetTab )
			m_pSheetTab->PageActive( LEVELEDIT_PAGE_MAIN );
	}else{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error Save!" );
	}
}

void CPageCondition::OnBnClickedButtonClose()
{
	if ( m_pSheetTab )
		m_pSheetTab->PageActive( LEVELEDIT_PAGE_MAIN );
}

void CPageCondition::OnBnClickedButtonReset()
{
	if ( !m_pGLLand )	return;

	SLEVEL_REQUIRE* pREQ = m_pGLLand->GetLevelRequire();
	if ( !pREQ )	return;

	*pREQ = SLEVEL_REQUIRE();
	DataShow();
}

void CPageCondition::OnBnClickedButtonReqActP()
{
	// TODO: Add your control notification handler code here
}

void CPageCondition::OnBnClickedButtonReqConP()
{
	// TODO: Add your control notification handler code here
}
