// PageEdit1.cpp : implementation file
//

#include "stdafx.h"
#include "EditorMobNpc.h"
#include "PageEdit1.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "GLItemMan.h"
#include "GLStringTable.h"

#include "GLOGIC.h"
#include "DxEffcharData.h"
#include "DxSkinPieceContainer.h"
#include "DxSimpleMeshMan.h"

#include "DlgCrow.h"
#include "DlgItem.h"

// CPageEdit1 dialog

IMPLEMENT_DYNAMIC(CPageEdit1, CPropertyPage)

CPageEdit1::CPageEdit1(LOGFONT logfont)
	: CPropertyPage(CPageEdit1::IDD),
	m_pFont ( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pCrow = NULL;
	m_pDummyCrow = new SCROWDATA;
}

CPageEdit1::~CPageEdit1()
{
	SAFE_DELETE ( m_pFont );
	SAFE_DELETE ( m_pDummyCrow );
}

void CPageEdit1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEdit1, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_NEXT, OnBnClickedButtonPageNext)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_CANCEL, OnBnClickedButtonPageCancel)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_SAVE, OnBnClickedButtonPageSave)
	ON_BN_CLICKED(IDC_BUTTON_SKINFILE, OnBnClickedButtonSkinfile)
	ON_BN_CLICKED(IDC_BUTTON_TALKFILE, OnBnClickedButtonTalkfile)
	ON_BN_CLICKED(IDC_BUTTON_BIRTHEFFECT, OnBnClickedButtonBirtheffect)
	ON_BN_CLICKED(IDC_BUTTON_DEATHEFFECT, OnBnClickedButtonDeatheffect)
	ON_BN_CLICKED(IDC_BUTTON_BLOWEFFECT, OnBnClickedButtonBloweffect)
	ON_BN_CLICKED(IDC_BUTTON_CROWSALE_00, OnBnClickedButtonCrowsale00)
	ON_BN_CLICKED(IDC_BUTTON_CROWSALE_01, OnBnClickedButtonCrowsale01)
	ON_BN_CLICKED(IDC_BUTTON_CROWSALE_02, OnBnClickedButtonCrowsale02)
	ON_BN_CLICKED(IDC_BUTTON_ITEMGEN, OnBnClickedButtonItemgen)
	ON_BN_CLICKED(IDC_BUTTON_ITEMGENQ, OnBnClickedButtonItemgenq)
	ON_BN_CLICKED(IDC_BUTTON_MOBLINK, OnBnClickedButtonMoblink)
	ON_BN_CLICKED(IDC_BUTTON_GENITEM, OnBnClickedButtonGenitem)
	ON_EN_CHANGE(IDC_EDIT_MOBLINK_MID, OnEnChangeEditMoblinkMid)
	ON_EN_CHANGE(IDC_EDIT_MOBLINK_SID, OnEnChangeEditMoblinkSid)
	ON_EN_CHANGE(IDC_EDIT_GEN_ITEM_MID, OnEnChangeEditGenItemMid)
	ON_EN_CHANGE(IDC_EDIT_GEN_ITEM_SID, OnEnChangeEditGenItemSid)
	ON_BN_CLICKED(IDC_BUTTON_NPCSHOP, OnBnClickedButtonNpcshop)
	ON_BN_CLICKED(IDC_BUTTON_NPC_ITEM_EXCHANGE, OnBnClickedButtonNpcItemExchange)
END_MESSAGE_MAP()


// CPageEdit1 message handlers
BOOL CPageEdit1::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	DataDefault();
	DataShow();		

	return TRUE;  
}

BOOL CPageEdit1::CrowSet ( PCROWDATA pCROW )
{
	if ( m_pCrow )	return FALSE;
	m_pCrow = pCROW;
	m_pDummyCrow->Assign( *m_pCrow );

	if ( m_bDlgInit ) 
	{
		DataDefault();
		DataShow();	
	}

	return TRUE;
}

void CPageEdit1::CrowClear()
{
	m_pCrow = NULL;
}

void CPageEdit1::OnBnClickedButtonPageNext()
{
	if ( !DataSave() )	return;

	SCROWDATA sCROW;
	sCROW.Assign( *m_pDummyCrow );
	GLCrowDataMan::GetInstance().InsertCrowData ( m_pDummyCrow->m_sBasic.sNativeID, &sCROW );

	std::string strNAME = GetWin_Text(this, IDC_EDIT_NAME2 ).GetString();
	GLStringTable::GetInstance().InsertString ( std::string( m_pDummyCrow->m_sBasic.m_szName), strNAME, GLStringTable::CROW );

	m_pSheetTab->ActivePageEdit2( m_CallPage, m_pCrow );
	m_pCrow = NULL;
}

void CPageEdit1::OnBnClickedButtonPageCancel()
{
	m_pCrow = NULL;
	if ( m_pSheetTab )
		m_pSheetTab->ActivePageTree( FALSE );
}

void CPageEdit1::OnBnClickedButtonPageSave()
{
	if ( !DataSave() )	return;

	SCROWDATA sCROW;
	sCROW.Assign( *m_pDummyCrow );
	GLCrowDataMan::GetInstance().InsertCrowData ( m_pDummyCrow->m_sBasic.sNativeID, &sCROW );

	std::string strNAME = GetWin_Text(this, IDC_EDIT_NAME2 ).GetString();
	GLStringTable::GetInstance().InsertString ( std::string( m_pDummyCrow->m_sBasic.m_szName), strNAME, GLStringTable::CROW );

	m_pCrow = NULL;
	m_pSheetTab->ActivePageTree( FALSE );
}

void CPageEdit1::InitMobLink()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_MOBLINK_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_MOBLINK_SID );

	PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( wMID, wSID );
	SetWin_Text( this, IDC_EDIT_MOBLINK_NAME, pCROW? pCROW->GetName() : "Unknown Crow" );
}

void CPageEdit1::InitGenItem()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_GEN_ITEM_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_GEN_ITEM_SID );

	SITEM* pITEM = GLItemMan::GetInstance().GetItem( wMID, wSID );
	SetWin_Text( this, IDC_EDIT_GEN_ITEM_NAME, pITEM? pITEM->GetName() : "Unknown Item" );
}

void CPageEdit1::OnBnClickedButtonSkinfile()
{
	CString szFilter = "Skin File (*.CHF)|*.CHF|";
	CFileDialog dlg(TRUE,".chf",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = DxSkinCharDataContainer::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_SKINFILE, dlg.GetFileName().GetString() );
}

void CPageEdit1::OnBnClickedButtonTalkfile()
{
	CString szFilter = "Npc Conversation File (*.ntk)|*.NTK|";
	CFileDialog dlg(TRUE,".ntk",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = CNpcDialogueSet::GetPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_TALKFILE, dlg.GetFileName().GetString() );
}

void CPageEdit1::OnBnClickedButtonBirtheffect()
{
	CString szFilter = "Effect File (*.egp)|*.egp|";
	CFileDialog dlg(TRUE,".egp",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = DxEffSinglePropGMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_BIRTHEFFECT, dlg.GetFileName().GetString() );
}

void CPageEdit1::OnBnClickedButtonDeatheffect()
{
	CString szFilter = "Effect File (*.egp)|*.egp|";
	CFileDialog dlg(TRUE,".egp",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = DxEffSinglePropGMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_DEATHEFFECT, dlg.GetFileName().GetString() );
}

void CPageEdit1::OnBnClickedButtonBloweffect()
{
	CString szFilter = "Effect File (*.egp)|*.egp|";
	CFileDialog dlg(TRUE,".egp",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = DxEffSinglePropGMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_BLOWEFFECT, dlg.GetFileName().GetString() );
}

void CPageEdit1::OnBnClickedButtonCrowsale00()
{
	CString szFilter = "Item Sale File (*.crowsale)|*.CROWSALE|";
	CFileDialog dlg(TRUE,".crowsale",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_SALEFILE_00, dlg.GetFileName().GetString() );
}

void CPageEdit1::OnBnClickedButtonCrowsale01()
{
	CString szFilter = "Item Sale File (*.crowsale)|*.CROWSALE|";
	CFileDialog dlg(TRUE,".crowsale",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_SALEFILE_01, dlg.GetFileName().GetString() );
}

void CPageEdit1::OnBnClickedButtonCrowsale02()
{
	CString szFilter = "Item Sale File (*.crowsale)|*.CROWSALE|";
	CFileDialog dlg(TRUE,".crowsale",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_SALEFILE_02, dlg.GetFileName().GetString() );
}

void CPageEdit1::OnBnClickedButtonItemgen()
{
	CString szFilter = "Item Gen File (*.genitem)|*.GENITEM|";
	CFileDialog dlg(TRUE,".genitem",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetServerPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_BUTTON_ITEMGEN, dlg.GetFileName().GetString() );
}

void CPageEdit1::OnBnClickedButtonItemgenq()
{
	CString szFilter = "Item Gen File (*.genitem)|*.GENITEM|";
	CFileDialog dlg(TRUE,".genitem",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetServerPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_BUTTON_ITEMGENQ, dlg.GetFileName().GetString() );
}

void CPageEdit1::OnBnClickedButtonMoblink()
{
	CDlgCrow dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_SELECTED;
		SetWin_Num_int( this, IDC_EDIT_MOBLINK_MID, sID.wMainID );
		SetWin_Num_int( this, IDC_EDIT_MOBLINK_SID, sID.wSubID );
		InitMobLink();
	}
}

void CPageEdit1::OnBnClickedButtonGenitem()
{
	CDlgItem dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_SELECTED;
		SetWin_Num_int( this, IDC_EDIT_GEN_ITEM_MID, sID.wMainID );
		SetWin_Num_int( this, IDC_EDIT_GEN_ITEM_SID, sID.wSubID );
		InitGenItem();
	}
}

void CPageEdit1::OnEnChangeEditMoblinkMid()
{
	InitMobLink();
}

void CPageEdit1::OnEnChangeEditMoblinkSid()
{
	InitMobLink();
}

void CPageEdit1::OnEnChangeEditGenItemMid()
{
	InitGenItem();
}

void CPageEdit1::OnEnChangeEditGenItemSid()
{
	InitGenItem();
}

/*npc shop, Juver, 2017/07/25 */
void CPageEdit1::OnBnClickedButtonNpcshop()
{
	CString szFilter = "NPC Shop File (*.npcshop)|*.NPCSHOP|";
	CFileDialog dlg(TRUE,".npcshop",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_NPCSHOP, dlg.GetFileName().GetString() );
}

/*item exchange, Juver, 2017/10/11 */
void CPageEdit1::OnBnClickedButtonNpcItemExchange()
{
	CString szFilter = "NPC ItemExchange File (*.npcex)|*.NPCEX|";
	CFileDialog dlg(TRUE,".npcex",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_NPC_ITEM_EXCHANGE, dlg.GetFileName().GetString() );
}
