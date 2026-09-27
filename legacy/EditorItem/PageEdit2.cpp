// PageEdit2.cpp : implementation file
//

#include "stdafx.h"
#include "EditorItem.h"
#include "PageEdit2.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "GLStringTable.h"
#include "GLItem.h"
#include "GLItemMan.h"
#include "GLItemDef.h"
#include "ItemBox.h"
#include "ItemBoxR.h"
#include "GLCrowData.h"
#include "Crow.h"
#include "HLibDataConvert.h"
#include ".\pageedit2.h"


#include "DlgItem.h"

// CPageEdit2 dialog

IMPLEMENT_DYNAMIC(CPageEdit2, CPropertyPage)

CPageEdit2::CPageEdit2(LOGFONT logfont)
	: CPropertyPage(CPageEdit2::IDD),
	m_pFont ( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pItem = NULL;
	m_pDummyItem = new SITEM;
}

CPageEdit2::~CPageEdit2()
{
	SAFE_DELETE ( m_pFont );
	SAFE_DELETE ( m_pDummyItem );
}

void CPageEdit2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_BOX, m_listBOX);
	DDX_Control(pDX, IDC_LIST_BOX2, m_listBOXR);
	DDX_Control(pDX, IDC_LIST_PETSKIN, m_listPetSkin );
	DDX_Control(pDX, IDC_LIST_RVCARD, m_listRVCardSet ); /*rv card, Juver, 2017/11/25 */
}


BEGIN_MESSAGE_MAP(CPageEdit2, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_SAVE, OnBnClickedButtonPageSave)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_CANCEL, OnBnClickedButtonPageCancel)
	ON_BN_CLICKED(IDC_BUTTON_PAGE_BACK, OnBnClickedButtonPageBack)
	ON_CBN_SELCHANGE(IDC_COMBO_BLOW_TYPE, OnCbnSelchangeComboBlowType)
	ON_CBN_SELCHANGE(IDC_COMBO_QUESTION_TYPE, OnCbnSelchangeComboQuestionType)
	ON_CBN_SELCHANGE(IDC_COMBO_DRUG_TYPE,OnCbnSelchangeComboDrugType)
	ON_BN_CLICKED(IDC_BUTTON_BOX_UP, OnBnClickedButtonBoxUp)
	ON_BN_CLICKED(IDC_BUTTON_BOX_DOWN, OnBnClickedButtonBoxDown)
	ON_BN_CLICKED(IDC_BUTTON_BOX_DEL, OnBnClickedButtonBoxDel)
	ON_BN_CLICKED(IDC_BUTTON_BOX_INSERT, OnBnClickedButtonBoxInsert)
	ON_BN_CLICKED(IDC_BUTTON_BOX_ADD, OnBnClickedButtonBoxAdd)
	ON_BN_CLICKED(IDC_BUTTON_BOX_ADD2, OnBnClickedButtonBoxAdd2)
	ON_BN_CLICKED(IDC_BUTTON_BOX_DEL2, OnBnClickedButtonBoxDel2)
	ON_BN_CLICKED(IDC_BUTTON_BOX_UP2, OnBnClickedButtonBoxUp2)
	ON_BN_CLICKED(IDC_BUTTON_BOX_DOWN2, OnBnClickedButtonBoxDown2)
	ON_BN_CLICKED(IDC_BUTTON_BOX_INSERT2, OnBnClickedButtonBoxInsert2)
	ON_BN_CLICKED(IDC_BUTTON_PS_INSERT, OnBnClickedButtonPsInsert)
	ON_BN_CLICKED(IDC_BUTTON_PS_ADD, OnBnClickedButtonPsAdd)
	ON_BN_CLICKED(IDC_BUTTON_PS_DEL, OnBnClickedButtonPsDel)
	ON_BN_CLICKED(IDC_BUTTON_RVCARD_SET, OnBnClickedButtonRvcardSet)
	ON_BN_CLICKED(IDC_BUTTON_RVCARD_DEL, OnBnClickedButtonRvcardDel)
	ON_BN_CLICKED(IDC_BUTTON_WRAPPER_BOX, OnBnClickedButtonWrapperBox)
	ON_EN_CHANGE(IDC_EDIT_WRAPPER_BOX_MID, OnEnChangeEditWrapperBoxMid)
	ON_EN_CHANGE(IDC_EDIT_WRAPPER_BOX_SID, OnEnChangeEditWrapperBoxSid)
END_MESSAGE_MAP()


// CPageEdit2 message handlers
BOOL CPageEdit2::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	LONG lnWidth;
	LV_COLUMN lvColumn;

	m_listPetSkin.SetExtendedStyle ( m_listPetSkin.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_listPetSkin.GetClientRect ( &rectCtrl );
	lnWidth = rectCtrl.right - rectCtrl.left;

	const int nColumnCount = 4;	
	char* szColumnName1[nColumnCount] = { "Num", "Crow ID", "Scale", "Rate" };
	int nColumnWidthPercent1[nColumnCount] = { 15, 50, 17, 18 };

	for ( int i = 0; i < nColumnCount; i++ )
	{		
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.pszText = szColumnName1[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = ( lnWidth*nColumnWidthPercent1[i] ) / 100;
		m_listPetSkin.InsertColumn (i, &lvColumn );
	}

	/*rv card, Juver, 2017/11/25 */
	m_listRVCardSet.SetExtendedStyle ( m_listRVCardSet.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_listRVCardSet.GetClientRect ( &rectCtrl );
	m_listRVCardSet.InsertColumn( 0, "Suit", LVCFMT_LEFT, ( 70* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_listRVCardSet.InsertColumn( 1, "Use", LVCFMT_LEFT, ( 30* ( rectCtrl.right - rectCtrl.left ) ) / 100 );


	DataDefault();
	DataShow();		

	return TRUE;  
}

BOOL CPageEdit2::ItemSet ( SITEM* pItem )
{
	if ( m_pItem )	return FALSE;
	m_pItem = pItem;
	*m_pDummyItem = *m_pItem;

	if ( m_bDlgInit ) 
	{
		DataDefault();
		DataShow();		
	}

	return TRUE;
}

void CPageEdit2::ItemClear ()
{
	m_pItem = NULL;
}


void CPageEdit2::OnBnClickedButtonPageSave()
{
	if ( !DataSave() )	return;

	CItemNode sItemNode;
	sItemNode.m_sItem = *m_pDummyItem;

	GLItemMan::GetInstance().InsertItem ( m_pDummyItem->sBasicOp.sNativeID, &sItemNode );

	m_pItem = NULL;
	m_pSheetTab->ActivePageTree( FALSE );
}

void CPageEdit2::OnBnClickedButtonPageCancel()
{
	m_pItem = NULL;
	if ( m_pSheetTab )
		m_pSheetTab->ActivePageTree( FALSE );
}

void CPageEdit2::OnBnClickedButtonPageBack()
{
	if ( !DataSave() )	return;

	CItemNode sItemNode;
	sItemNode.m_sItem = *m_pDummyItem;
	GLItemMan::GetInstance().InsertItem ( m_pDummyItem->sBasicOp.sNativeID, &sItemNode );

	m_pSheetTab->ActivePageEdit1( m_CallPage, m_pItem );
	m_pItem = NULL;
}

void CPageEdit2::OnCbnSelchangeComboBlowType()
{
	InitBlow();
}

void CPageEdit2::InitBlow()
{
	EMSTATE_BLOW emTYPE = static_cast<EMSTATE_BLOW> ( GetWin_Combo_Sel( this, IDC_COMBO_BLOW_TYPE ));

	BOOL bSTATE_BLOW = ( emTYPE!=EMBLOW_NONE );
	BOOL bBLOW_VAR1(TRUE), bBLOW_VAR2(TRUE);
	const char *szBLOW_VAR1 = COMMENT::BLOW_VAR1[emTYPE].c_str();
	const char *szBLOW_VAR2 = COMMENT::BLOW_VAR2[emTYPE].c_str();
	if ( !strcmp(szBLOW_VAR1, COMMENT::BLOW_VAR1[EMBLOW_NONE].c_str()) )		bBLOW_VAR1 = FALSE;
	if ( !strcmp(szBLOW_VAR2, COMMENT::BLOW_VAR2[EMBLOW_NONE].c_str()) )		bBLOW_VAR2 = FALSE;

	SetWin_Enable ( this, IDC_EDIT_BLOW_TIME, bSTATE_BLOW );
	SetWin_Enable ( this, IDC_EDIT_BLOW_CHANCE, bSTATE_BLOW );
	SetWin_Enable ( this, IDC_EDIT_BLOW_VAR1, bBLOW_VAR1 );
	SetWin_Enable ( this, IDC_EDIT_BLOW_VAR2, bBLOW_VAR2 );

	SetWin_Text ( this, IDC_STATIC_BLOW_OPTION1, szBLOW_VAR1 );
	SetWin_Text ( this, IDC_STATIC_BLOW_OPTION2, szBLOW_VAR2 );
}

void CPageEdit2::OnCbnSelchangeComboQuestionType()
{
	InitQuestion();
}

void CPageEdit2::InitQuestion()
{
	EMITEM_QUESTION emTYPE = static_cast<EMITEM_QUESTION> ( GetWin_Combo_Sel( this, IDC_COMBO_QUESTION_TYPE ));

	BOOL bQUESTION = ( emTYPE!=QUESTION_NONE );
	BOOL bEXP = ( emTYPE==QUESTION_EXP_GET );
	BOOL bNOTIME = ( emTYPE==QUESTION_EXP_GET || emTYPE==QUESTION_BOMB || emTYPE==QUESTION_MOBGEN );
	BOOL bQUESTION_VAR1(TRUE), bQUESTION_VAR2(TRUE);

	const char *sVAR1 = COMMENT::ITEM_QUE_VAR1[emTYPE].c_str();
	const char *sVAR2 = COMMENT::ITEM_QUE_VAR2[emTYPE].c_str();
	if ( !strcmp(sVAR1, COMMENT::ITEM_QUE_VAR1[QUESTION_NONE].c_str()) )		bQUESTION_VAR1 = FALSE;
	if ( !strcmp(sVAR2, COMMENT::ITEM_QUE_VAR2[QUESTION_NONE].c_str()) )		bQUESTION_VAR2 = FALSE;

	SetWin_Enable ( this, IDC_EDIT_QUESTION_TIME, bQUESTION && !bNOTIME );
	SetWin_Enable ( this, IDC_EDIT_QUESTION_VAR1, bEXP );
	SetWin_Enable ( this, IDC_EDIT_QUESTION_VAR2, bQUESTION_VAR1 );
	SetWin_Enable ( this, IDC_EDIT_QUESTION_VAR3, bQUESTION_VAR2 );

	SetWin_Text ( this, IDC_STATIC_QUESTION_OPTION1, bEXP? "Exp Percentage":"None" );
	SetWin_Text ( this, IDC_STATIC_QUESTION_OPTION2, sVAR1 );
	SetWin_Text ( this, IDC_STATIC_QUESTION_OPTION3, sVAR2 );
}

void CPageEdit2::OnCbnSelchangeComboDrugType()
{
	InitDrug();
}

void CPageEdit2::InitDrug()
{
	EMITEM_DRUG emTYPE = static_cast<EMITEM_DRUG> ( GetWin_Combo_Sel( this, IDC_COMBO_DRUG_TYPE ));
	BOOL bCUREFLAG = ( emTYPE == ITEM_DRUG_CURE || emTYPE == ITEM_DRUG_HP_CURE || emTYPE == ITEM_DRUG_HP_MP_SP_CURE );
	SetWin_Enable( this, IDC_CHECK_CURE_NUMB, bCUREFLAG );
	SetWin_Enable( this, IDC_CHECK_CURE_STUN, bCUREFLAG );
	SetWin_Enable( this, IDC_CHECK_CURE_STONE, bCUREFLAG );
	SetWin_Enable( this, IDC_CHECK_CURE_BURN, bCUREFLAG );
	SetWin_Enable( this, IDC_CHECK_CURE_FROZEN, bCUREFLAG );
	SetWin_Enable( this, IDC_CHECK_CURE_MAD, bCUREFLAG );
	SetWin_Enable( this, IDC_CHECK_CURE_POISON, bCUREFLAG );
	SetWin_Enable( this, IDC_CHECK_CURE_CURSE, bCUREFLAG );
}

void CPageEdit2::InitBox()
{
	m_listBOX.ResetContent();
	m_listBOX.SetRedraw( FALSE );

	for ( int i=0; i<ITEM::SBOX::ITEM_SIZE; ++i )
	{
		SNATIVEID sITEM = m_pDummyItem->sBox.sITEMS[i].nidITEM;
		DWORD dwAMOUNT = m_pDummyItem->sBox.sITEMS[i].dwAMOUNT;
		if ( sITEM==SNATIVEID(false) )		continue;

		CString strNAME = "[Error Item]";
		SITEM* pItem = GLItemMan::GetInstance().GetItem ( sITEM );
		if ( pItem )	strNAME.Format ( "[%d/%d] %d / %s", sITEM.wMainID, sITEM.wSubID, dwAMOUNT, pItem->GetName() ); 

		int nIndex = m_listBOX.AddString ( strNAME );
		m_listBOX.SetItemData ( nIndex, i );
	}

	m_listBOX.SetRedraw( TRUE );

	SetWin_Num_int( this, IDC_EDIT_BOX_MID, 65535 );
	SetWin_Num_int( this, IDC_EDIT_BOX_SID, 65535 );
	SetWin_Num_int( this, IDC_EDIT_BOX_SID2, 65535 );
	SetWin_Num_int( this, IDC_EDIT_BOX_NUM, 1 );
}

void CPageEdit2::OnBnClickedButtonBoxUp()
{
	int nSel = m_listBOX.GetCurSel();
	int nCnt = m_listBOX.GetCount();
	if ( nSel <=0 || nSel >= nCnt ) return;

	ITEM::SBOX_ITEM sSelItem;
	sSelItem = m_pDummyItem->sBox.sITEMS[nSel];
	m_pDummyItem->sBox.sITEMS[nSel] = m_pDummyItem->sBox.sITEMS[nSel-1];
	m_pDummyItem->sBox.sITEMS[nSel-1] = sSelItem;

	InitBox();
	m_listBOX.SetCurSel( nSel-1);
}

void CPageEdit2::OnBnClickedButtonBoxDown()
{
	int nSel = m_listBOX.GetCurSel();
	int nCnt = m_listBOX.GetCount();
	if ( nSel <0 || nSel >= nCnt-1 ) return;

	ITEM::SBOX_ITEM sSelItem;
	sSelItem = m_pDummyItem->sBox.sITEMS[nSel];
	m_pDummyItem->sBox.sITEMS[nSel] = m_pDummyItem->sBox.sITEMS[nSel+1];
	m_pDummyItem->sBox.sITEMS[nSel+1] = sSelItem;

	InitBox();
	m_listBOX.SetCurSel( nSel+1);
}

void CPageEdit2::OnBnClickedButtonBoxDel()
{
	int nIndex = m_listBOX.GetCurSel();
	if ( nIndex==LB_ERR )	return;
	DWORD_PTR dwID = m_listBOX.GetItemData ( nIndex );
	m_pDummyItem->sBox.DEL ( (int)dwID );
	m_listBOX.DeleteString ( nIndex );
	InitBox ();
}

void CPageEdit2::OnBnClickedButtonBoxInsert()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_BOX_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_BOX_SID );
	WORD wEND = GetWin_Num_int( this, IDC_EDIT_BOX_SID2 );
	int nNUM = GetWin_Num_int( this, IDC_EDIT_BOX_NUM );

	if ( wEND != 65535 )
	{
		bool bInserted = FALSE;
		for ( WORD wSTART = sid.wSubID; wSTART < wEND+1; ++ wSTART )
		{
			SNATIVEID sid2;
			sid2.wMainID = sid.wMainID;
			sid2.wSubID = wSTART;
			SITEM* pITEM = GLItemMan::GetInstance().GetItem( sid2 );
			if ( pITEM )
				bInserted = m_pDummyItem->sBox.INSERT ( sid2, nNUM );
		}

		if ( bInserted )
			InitBox ();
	}
	else
	{
		SITEM* pITEM = GLItemMan::GetInstance().GetItem( sid );
		if ( pITEM )
		{
			bool bInserted = m_pDummyItem->sBox.INSERT ( sid, nNUM );
			if ( bInserted )
				InitBox ();
		}	
	}	
}

void CPageEdit2::OnBnClickedButtonBoxAdd()
{
	CItemBox Dlg;
	if ( Dlg.DoModal() == IDOK )
	{
		bool bInserted = FALSE;
		CItemBox::VEC_ITER iter  = Dlg.m_vecSELECTED.begin();
		for( ; iter != Dlg.m_vecSELECTED.end(); ++iter )
		{
			ITEM::SBOX_ITEM sITEMBOX = *iter;
			bInserted = m_pDummyItem->sBox.INSERT ( sITEMBOX.nidITEM, sITEMBOX.dwAMOUNT );
		}

		if ( bInserted )
			InitBox();
	}
}

void CPageEdit2::InitBoxR()
{
	m_listBOXR.ResetContent();
	m_listBOXR.SetRedraw( FALSE );
	
	std::vector<ITEM::SRANDOMITEM>::iterator iter = m_pDummyItem->sRandomBox.vecBOX.begin();
	std::vector<ITEM::SRANDOMITEM>::iterator iter_end = m_pDummyItem->sRandomBox.vecBOX.end();

	for ( ; iter!=iter_end; ++iter )
	{
		SNATIVEID sITEM = iter->nidITEM;
		float fRate = iter->fRATE;
		if ( sITEM==SNATIVEID(false) )		continue;

		CString strNAME = "[Error Item]";
		SITEM* pItem = GLItemMan::GetInstance().GetItem ( sITEM );
		if ( pItem )	strNAME.Format ( "[%d/%d] %.2f / %s", sITEM.wMainID, sITEM.wSubID, fRate, pItem->GetName() ); 

		int nIndex = m_listBOXR.AddString ( strNAME );
		m_listBOXR.SetItemData ( nIndex, sITEM.dwID );
	}

	m_listBOXR.SetRedraw( TRUE );

	SetWin_Num_int( this, IDC_EDIT_BOX2_MID, 65535 );
	SetWin_Num_int( this, IDC_EDIT_BOX2_SID, 65535 );
	SetWin_Num_float( this, IDC_EDIT_BOX2_RATE, 1.0f );
}

void CPageEdit2::OnBnClickedButtonBoxAdd2()
{
	CItemBoxR Dlg;
	if ( Dlg.DoModal() == IDOK )
	{
		bool bInserted = FALSE;
		CItemBoxR::VEC_ITER iter  = Dlg.m_vecSELECTED.begin();
		for( ; iter != Dlg.m_vecSELECTED.end(); ++iter )
		{
			ITEM::SRANDOMITEM sITEMBOX = *iter;
			bInserted = m_pDummyItem->sRandomBox.INSERT ( sITEMBOX.nidITEM, sITEMBOX.fRATE );
		}

		if ( bInserted )
			InitBoxR();
	}
}

void CPageEdit2::OnBnClickedButtonBoxDel2()
{
	int nIndex = m_listBOXR.GetCurSel();
	if ( nIndex==LB_ERR )	return;

	m_pDummyItem->sRandomBox.DEL ( nIndex );
	m_listBOXR.DeleteString ( nIndex );
	InitBoxR();
}

void CPageEdit2::OnBnClickedButtonBoxUp2()
{
	int nSel = m_listBOXR.GetCurSel();
	int nCnt = m_listBOXR.GetCount();

	if ( nSel <=0 || nSel >= nCnt ) return;

	ITEM::SRANDOMITEM sSelItem;	
	sSelItem = m_pDummyItem->sRandomBox.vecBOX[nSel];
	m_pDummyItem->sRandomBox.vecBOX[nSel] = m_pDummyItem->sRandomBox.vecBOX[nSel-1];
	m_pDummyItem->sRandomBox.vecBOX[nSel-1] = sSelItem;

	InitBoxR();
	m_listBOXR.SetCurSel( nSel-1 );
}

void CPageEdit2::OnBnClickedButtonBoxDown2()
{
	int nSel = m_listBOXR.GetCurSel();
	int nCnt = m_listBOXR.GetCount();

	if ( nSel <0 || nSel >= nCnt-1 ) return;

	ITEM::SRANDOMITEM sSelItem;	
	sSelItem = m_pDummyItem->sRandomBox.vecBOX[nSel];
	m_pDummyItem->sRandomBox.vecBOX[nSel] = m_pDummyItem->sRandomBox.vecBOX[nSel+1];
	m_pDummyItem->sRandomBox.vecBOX[nSel+1] = sSelItem;

	InitBoxR();
	m_listBOXR.SetCurSel( nSel+1 );
}

void CPageEdit2::OnBnClickedButtonBoxInsert2()
{
	SNATIVEID sid;
	sid.wMainID = GetWin_Num_int( this, IDC_EDIT_BOX2_MID );
	sid.wSubID = GetWin_Num_int( this, IDC_EDIT_BOX2_SID );
	float fPercent = GetWin_Num_float( this, IDC_EDIT_BOX2_RATE );

	bool bINSERT = m_pDummyItem->sRandomBox.INSERT ( sid, fPercent );
	if ( bINSERT )
		InitBoxR();
}

void CPageEdit2::InitPetSkin()
{
	m_listPetSkin.DeleteAllItems();
	m_listPetSkin.SetRedraw( FALSE );

	int nSize = (int)m_pDummyItem->sPetSkinPack.vecPetSkinData.size();
	for ( int i = 0; i < nSize; i++ )
	{
		ITEM::SPETSKINPACKITEMDATA sPetSkinPack = m_pDummyItem->sPetSkinPack.vecPetSkinData[i];
		PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( sPetSkinPack.sMobID );
		
		std::string strNAME = "Error Crow";
		if ( pCrowData )	strNAME =  pCrowData->GetName();

		m_listPetSkin.InsertItem( i, _HLIB::cstring_dword( i ).GetString() );
		m_listPetSkin.SetItemText( i, 1, _HLIB::cstringformat( "%s(%d/%d)", strNAME.c_str(), sPetSkinPack.sMobID.wMainID, sPetSkinPack.sMobID.wSubID ).GetString() );
		m_listPetSkin.SetItemText( i, 2, _HLIB::cstring_floatg( sPetSkinPack.fScale ).GetString() );
		m_listPetSkin.SetItemText( i, 3, _HLIB::cstring_floatg( sPetSkinPack.fRate ).GetString() );
	}

	m_listPetSkin.SetRedraw( TRUE );

	SetWin_Num_int( this, IDC_EDIT_PS_MID, 65535 );
	SetWin_Num_int( this, IDC_EDIT_PS_SID, 65535 );
	SetWin_Num_float( this, IDC_EDIT_PS_RATE, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_PS_SCALE, 1.0f );
}

void CPageEdit2::OnBnClickedButtonPsInsert()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_PS_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_PS_SID );
	float fRATE = GetWin_Num_float( this, IDC_EDIT_PS_RATE );
	float fSCALE = GetWin_Num_float( this, IDC_EDIT_PS_SCALE );
	
	if ( wMID >= 65535 )	return;
	if ( wSID >= 65535 )	return;
	
	BOOL bINSERT = m_pDummyItem->sPetSkinPack.INSERT( SNATIVEID( wMID, wSID ), fRATE, fSCALE );

	if ( bINSERT )
		InitPetSkin();   
	else
		MessageBox( "Unable to Insert! PetSkinPack Full!" );
}

void CPageEdit2::OnBnClickedButtonPsAdd()
{
	CCrow Dlg;
	if ( Dlg.DoModal() == IDOK )
	{
		CCrow::VEC_ITER iter  = Dlg.m_vecSELECTED.begin();
		for( ; iter != Dlg.m_vecSELECTED.end(); ++iter )
		{
			ITEM::SPETSKINPACKITEMDATA sDATA = *iter;
			BOOL bINSERT = m_pDummyItem->sPetSkinPack.INSERT ( sDATA.sMobID, sDATA.fRate, sDATA.fScale );
			if ( !bINSERT )
				MessageBox( "Unable to Insert! PetSkinPack Full!" );
		}

		InitPetSkin();
	}
}

void CPageEdit2::OnBnClickedButtonPsDel()
{
	int nSelect = m_listPetSkin.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	m_pDummyItem->sPetSkinPack.DEL( nSelect );
	InitPetSkin();
}

/*rv card, Juver, 2017/11/25 */
void CPageEdit2::InitRVCard()
{
	m_listRVCardSet.DeleteAllItems();
	m_listRVCardSet.SetRedraw( FALSE );

	for ( int i = 0; i < SUIT_NSIZE; i++ )
	{
		m_listRVCardSet.InsertItem( i, COMMENT::ITEMSUIT[i].c_str() );
		m_listRVCardSet.SetItemText( i, 1, _HLIB::cstringformat("%d", m_pDummyItem->sRvCard.bUseSuit[i] ).GetString() );
	}

	m_listRVCardSet.SetRedraw( TRUE );
}

/*rv card, Juver, 2017/11/25 */
void CPageEdit2::OnBnClickedButtonRvcardSet()
{
	int nSelect = m_listRVCardSet.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	m_pDummyItem->sRvCard.bUseSuit[nSelect] = TRUE;
	InitRVCard();
}

/*rv card, Juver, 2017/11/25 */
void CPageEdit2::OnBnClickedButtonRvcardDel()
{
	int nSelect = m_listRVCardSet.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	m_pDummyItem->sRvCard.bUseSuit[nSelect] = FALSE;
	InitRVCard();
}

void CPageEdit2::OnBnClickedButtonWrapperBox()
{
	/*item wrapper, Juver, 2018/01/11 */
	CDlgItem dlg;
	if ( dlg.DoModal() == IDOK )
	{
		SNATIVEID sID = dlg.m_SELECTED;
		SetWin_Num_int( this, IDC_EDIT_WRAPPER_BOX_MID, sID.wMainID );
		SetWin_Num_int( this, IDC_EDIT_WRAPPER_BOX_SID, sID.wSubID );
		CheckWrapperBox();
	}
}

void CPageEdit2::OnEnChangeEditWrapperBoxMid()
{
	/*item wrapper, Juver, 2018/01/11 */
	CheckWrapperBox();
}

void CPageEdit2::OnEnChangeEditWrapperBoxSid()
{
	/*item wrapper, Juver, 2018/01/11 */
	CheckWrapperBox();
}

/*item wrapper, Juver, 2018/01/11 */
void CPageEdit2::CheckWrapperBox()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_WRAPPER_BOX_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_WRAPPER_BOX_SID );

	SITEM* pITEM = GLItemMan::GetInstance().GetItem( wMID, wSID );
	SetWin_Text( this, IDC_EDIT_WRAPPER_BOX_NAME, pITEM? pITEM->GetName() : "Unknown Item" );
}