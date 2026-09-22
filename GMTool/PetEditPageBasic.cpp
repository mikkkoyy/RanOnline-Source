// PetEditPageBasic.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "PetEditPageBasic.h"

#include "PetEditTab.h"
#include "GLPet.h"
#include "EtcFunction.h"
#include "DlgCrow.h"
#include "GLCrowData.h"
#include "Logic/GMToolGlobal.h"
#include "SelColorDlg.h"
#include "HLibColorValue.h"
#include "DlgDatetime.h"
#include "HLibDataConvert.h"

// CPetEditPageBasic dialog

IMPLEMENT_DYNAMIC(CPetEditPageBasic, CPropertyPage)

CPetEditPageBasic::CPetEditPageBasic(LOGFONT logfont, GLPET* pData)
	: CPropertyPage(CPetEditPageBasic::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CPetEditPageBasic::~CPetEditPageBasic()
{
	SAFE_DELETE ( m_pFont );
}

void CPetEditPageBasic::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PETEDITBASIC_SLIDER_PETFOOD, m_slider_Food );
}


BEGIN_MESSAGE_MAP(CPetEditPageBasic, CPropertyPage)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PETEDITBASIC_SLIDER_PETFOOD, OnNMCustomdrawPeteditbasicSliderPetfood)
	ON_BN_CLICKED(IDC_PETEDITBASIC_BUTTON_SKINCROW_EDIT, OnBnClickedPeteditbasicButtonSkincrowEdit)
	ON_BN_CLICKED(IDC_PETEDITBASIC_BUTTON_SKINCROW_DELETE, OnBnClickedPeteditbasicButtonSkincrowDelete)
	ON_EN_CHANGE(IDC_PETEDITBASIC_EDIT_PETSKINCROW_MID, OnEnChangePeteditbasicEditPetskincrowMid)
	ON_EN_CHANGE(IDC_PETEDITBASIC_EDIT_PETSKINCROW_SID, OnEnChangePeteditbasicEditPetskincrowSid)
	ON_BN_CLICKED(IDC_PETEDITBASIC_BUTTON_PETCOLOR, OnBnClickedPeteditbasicButtonPetcolor)
	ON_BN_CLICKED(IDC_PETEDITBASIC_BUTTON_PETSKINSTART_SET, OnBnClickedPeteditbasicButtonPetskinstartSet)
	ON_BN_CLICKED(IDC_PETEDITBASIC_BUTTON_PETSKINSTART_RESET, OnBnClickedPeteditbasicButtonPetskinstartReset)
END_MESSAGE_MAP()


// CPetEditPageBasic message handlers
BOOL CPetEditPageBasic::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	SetWin_Combo_Init( this, IDC_PETEDITBASIC_COMBO_PETTYPE, COMMENT::PET_TYPE, PET_TYPE_SIZE );
	m_slider_Food.SetRange(0, 1000, TRUE);

	DataDefault();
	DataShow();

	return TRUE;  
}

void CPetEditPageBasic::DataDefault()
{
	SetWin_Text( this, IDC_PETEDITBASIC_EDIT_PETNAME, "" );
	SetWin_Combo_Sel( this, IDC_PETEDITBASIC_COMBO_PETTYPE, 0 );
	SetWin_Num_int( this, IDC_PETEDITBASIC_EDIT_PETCOLOR, 0 );
	SetWin_Num_int( this, IDC_PETEDITBASIC_EDIT_PETSTYLE, 0 );
	m_slider_Food.SetPos(0);
	SetWin_Num_int ( this, IDC_PETEDITBASIC_EDIT_PETFOOD, 0 );
	SetWin_Num_int ( this, IDC_PETEDITBASIC_EDIT_PETSKINCROW_MID, 0 );
	SetWin_Num_int ( this, IDC_PETEDITBASIC_EDIT_PETSKINCROW_SID, 0 );
	SetWin_Text( this, IDC_PETEDITBASIC_EDIT_PETSKINCROW_NAME, "" );
	SetWin_Num_float( this, IDC_PETEDITBASIC_EDIT_PETSKINSCALE, 0.0f );
	SetWin_Num_int ( this, IDC_PETEDITBASIC_EDIT_PETSKINTIME_D, 0 );
	SetWin_Num_int ( this, IDC_PETEDITBASIC_EDIT_PETSKINTIME_H, 0 );
	SetWin_Num_int ( this, IDC_PETEDITBASIC_EDIT_PETSKINTIME_M, 0 );
	SetWin_Text( this, IDC_PETEDITBASIC_EDIT_PETSKIN_STARTDATE, "" );

	/*dual pet skill, Juver, 2017/12/29 */
	SetWin_Check( this, IDC_CHECK_DUAL_SKILL, 0 );
}

void CPetEditPageBasic::DataShow()
{
	if ( !m_pData ){
		return;
	}

	SetWin_Text( this, IDC_PETEDITBASIC_EDIT_PETNAME, m_pData->m_szName );
	SetWin_Combo_Sel( this, IDC_PETEDITBASIC_COMBO_PETTYPE, COMMENT::PET_TYPE[m_pData->m_emTYPE].c_str() );
	SetWin_Num_int( this, IDC_PETEDITBASIC_EDIT_PETCOLOR, m_pData->m_wColor );
	SetWin_Num_int( this, IDC_PETEDITBASIC_EDIT_PETSTYLE, m_pData->m_wStyle );

	m_slider_Food.SetPos(m_pData->m_nFull);

	//petscale data
	SetWin_Num_int ( this, IDC_PETEDITBASIC_EDIT_PETSKINCROW_MID, m_pData->m_sPetSkinPackData.sMobID.wMainID );
	SetWin_Num_int ( this, IDC_PETEDITBASIC_EDIT_PETSKINCROW_SID, m_pData->m_sPetSkinPackData.sMobID.wSubID );
	SetWin_Num_float( this, IDC_PETEDITBASIC_EDIT_PETSKINSCALE, m_pData->m_sPetSkinPackData.fScale );
	CTimeSpan cSPAN(m_pData->m_sPetSkinPackData.dwPetSkinTime);
	SetWin_Num_int ( this, IDC_PETEDITBASIC_EDIT_PETSKINTIME_D, (int)cSPAN.GetDays() );
	SetWin_Num_int ( this, IDC_PETEDITBASIC_EDIT_PETSKINTIME_H, (int)cSPAN.GetHours() );
	SetWin_Num_int ( this, IDC_PETEDITBASIC_EDIT_PETSKINTIME_M, (int)cSPAN.GetMinutes() );
	SetWin_Text( this, IDC_PETEDITBASIC_EDIT_PETSKIN_STARTDATE, 
		_HLIB::cstring_ctime12(m_pData->m_sPetSkinPackData.startTime).GetString() );

	/*dual pet skill, Juver, 2017/12/29 */
	SetWin_Check( this, IDC_CHECK_DUAL_SKILL, m_pData->m_bDualSkill );
}

BOOL CPetEditPageBasic::DataSave()
{
	if( !m_pData ){
		return FALSE;
	}

	std::string strPetName = GetWin_Text( this, IDC_PETEDITBASIC_EDIT_PETNAME ).GetString();
	
	if ( strPetName.size() > PETNAMESIZE ){
		CDebugSet::MsgBox( GetSafeHwnd(), "PetName can only have up to %d characters", PETNAMESIZE );
		GotoDlgCtrl(GetDlgItem(IDC_PETEDITBASIC_EDIT_PETNAME)); 
		return FALSE;
	}

	if ( GMTOOL_GLOBAL::IsBadString2( strPetName.c_str() ) ){
		CDebugSet::MsgBox( GetSafeHwnd(), "PetName contains invalid characters!" );
		GotoDlgCtrl(GetDlgItem(IDC_PETEDITBASIC_EDIT_PETNAME)); 
		return FALSE;
	}

	StringCchCopy( m_pData->m_szName, PETNAMESIZE, strPetName.c_str() );
	m_pData->m_emTYPE = static_cast<PETTYPE> ( GetWin_Combo_Sel( this, IDC_PETEDITBASIC_COMBO_PETTYPE) );
	m_pData->m_wColor = GetWin_Num_int( this, IDC_PETEDITBASIC_EDIT_PETCOLOR );
	m_pData->m_wStyle = GetWin_Num_int( this, IDC_PETEDITBASIC_EDIT_PETSTYLE );
	m_pData->m_nFull = m_slider_Food.GetPos();

	/*dual pet skill, Juver, 2017/12/29 */
	m_pData->m_bDualSkill = GetWin_Check( this, IDC_CHECK_DUAL_SKILL );

	SNATIVEID sPETSKINID;
	sPETSKINID.wMainID = GetWin_Num_int( this, IDC_PETEDITBASIC_EDIT_PETSKINCROW_MID );
	sPETSKINID.wSubID = GetWin_Num_int( this, IDC_PETEDITBASIC_EDIT_PETSKINCROW_SID );
	if( sPETSKINID == NATIVEID_NULL() )
	{
		//just reset the petskin data
		m_pData->m_sPetSkinPackData.Init();
	}
	else
	{
		float fSCALE = GetWin_Num_float( this, IDC_PETEDITBASIC_EDIT_PETSKINSCALE );
		WORD wDAYS = GetWin_Num_int( this,IDC_PETEDITBASIC_EDIT_PETSKINTIME_D );
		WORD wHOURS = GetWin_Num_int( this,IDC_PETEDITBASIC_EDIT_PETSKINTIME_H );
		WORD wMINUTES = GetWin_Num_int( this,IDC_PETEDITBASIC_EDIT_PETSKINTIME_M );
		CTimeSpan cSPAN( wDAYS, wHOURS, wMINUTES, 0 );

		DWORD dwTIME = DWORD( cSPAN.GetTimeSpan() );

		SCROWDATA* pCrow = GLCrowDataMan::GetInstance().GetCrowData( sPETSKINID );
		if( !pCrow ){
			CDebugSet::MsgBox( GetSafeHwnd(), "Invalid PetSkin Crow!" );
			return FALSE;
		}

		if ( fSCALE <= 0.0f ){
			CDebugSet::MsgBox( GetSafeHwnd(), "PetSkin Scale too small! Value Cannot be less than 0.0" );
			return FALSE;
		}

		if ( dwTIME <= 0 ){
			CDebugSet::MsgBox( GetSafeHwnd(), "PetSkin time Value Must be more than 0" );
			return FALSE;
		}

		if ( m_pData->m_sPetSkinPackData.startTime.GetTime() <= CTime::GetCurrentTime().GetTime() ){
			CDebugSet::MsgBox( GetSafeHwnd(), "PetSkin Start Date is incorrect!" );
			return FALSE;
		}

		m_pData->m_sPetSkinPackData.sMobID = sPETSKINID;
		m_pData->m_sPetSkinPackData.fScale = fSCALE;
		m_pData->m_sPetSkinPackData.dwPetSkinTime = dwTIME;	
	}

	return TRUE;
}

void CPetEditPageBasic::OnNMCustomdrawPeteditbasicSliderPetfood(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int nData = m_slider_Food.GetPos();
	SetWin_Num_int ( this, IDC_PETEDITBASIC_EDIT_PETFOOD, nData );
}

void CPetEditPageBasic::OnBnClickedPeteditbasicButtonSkincrowEdit()
{
	CDlgCrow dlg( this );
	if ( dlg.DoModal() == IDOK ){
		SCROWDATA* pCrow = GLCrowDataMan::GetInstance().GetCrowData( dlg.m_SELECTED );
		if ( pCrow ){
			SetWin_Num_int( this, IDC_PETEDITBASIC_EDIT_PETSKINCROW_MID, pCrow->m_sBasic.sNativeID.wMainID );
			SetWin_Num_int( this, IDC_PETEDITBASIC_EDIT_PETSKINCROW_SID, pCrow->m_sBasic.sNativeID.wSubID );
		}
	}
}

void CPetEditPageBasic::OnBnClickedPeteditbasicButtonSkincrowDelete()
{
	SetWin_Num_int( this, IDC_PETEDITBASIC_EDIT_PETSKINCROW_MID, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_PETEDITBASIC_EDIT_PETSKINCROW_SID, NATIVEID_NULL().wSubID );
}

void CPetEditPageBasic::OnEnChangePeteditbasicEditPetskincrowMid()
{
	GMTOOL_GLOBAL::DataCheckCrow( this, IDC_PETEDITBASIC_EDIT_PETSKINCROW_MID, 
		IDC_PETEDITBASIC_EDIT_PETSKINCROW_SID, 
		IDC_PETEDITBASIC_EDIT_PETSKINCROW_NAME );
}

void CPetEditPageBasic::OnEnChangePeteditbasicEditPetskincrowSid()
{
	GMTOOL_GLOBAL::DataCheckCrow( this, IDC_PETEDITBASIC_EDIT_PETSKINCROW_MID, 
		IDC_PETEDITBASIC_EDIT_PETSKINCROW_SID, 
		IDC_PETEDITBASIC_EDIT_PETSKINCROW_NAME );
}

void CPetEditPageBasic::OnBnClickedPeteditbasicButtonPetcolor()
{
	WORD wColor = GetWin_Num_int( this, IDC_PETEDITBASIC_EDIT_PETCOLOR );
	DWORD dwColor = _HLIB::color_dword_word( wColor );

	CSelColorDlg dlg( BYTE( _HLIB::color_get_int_r( dwColor) ), 
		BYTE( _HLIB::color_get_int_g( dwColor ) ), 
		BYTE( _HLIB::color_get_int_b( dwColor ) ), 
		this );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_PETEDITBASIC_EDIT_PETCOLOR, _HLIB::color_word_dword( _HLIB::color_argb_rgb(crNewColor) ) );
	}
}

void CPetEditPageBasic::OnBnClickedPeteditbasicButtonPetskinstartSet()
{
	if ( !m_pData ){
		return;
	}

	CTime cPetSkinTime = m_pData->m_sPetSkinPackData.startTime;
	if ( cPetSkinTime.GetTime() < CTime::GetCurrentTime().GetTime() ){
		cPetSkinTime = CTime::GetCurrentTime().GetTime();
	}

	CDlgDatetime dlg( this );
	dlg.DateSetMin( CTime::GetCurrentTime().GetTime() );
	dlg.DateTimeSet( cPetSkinTime.GetTime() );
	if ( dlg.DoModal() == IDOK ){
		 m_pData->m_sPetSkinPackData.startTime = dlg.DateTimeGet();
		SetWin_Text( this, IDC_PETEDITBASIC_EDIT_PETSKIN_STARTDATE, 
			_HLIB::cstring_ctime12(m_pData->m_sPetSkinPackData.startTime).GetString() );
	}
}

void CPetEditPageBasic::OnBnClickedPeteditbasicButtonPetskinstartReset()
{
	if ( !m_pData ){
		return;
	}

	m_pData->m_sPetSkinPackData.startTime = CTime::GetCurrentTime().GetTime();
	SetWin_Text( this, IDC_PETEDITBASIC_EDIT_PETSKIN_STARTDATE, 
		_HLIB::cstring_ctime12(m_pData->m_sPetSkinPackData.startTime).GetString() );
}
