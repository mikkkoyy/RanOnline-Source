// LockerPageMain.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "LockerPageMain.h"

#include "LockerTab.h"
#include "GLCharData.h"
#include "EtcFunction.h"
#include "HLibTimeFunctions.h"
#include "HLibDataConvert.h"
#include "Logic/GMToolGlobal.h"
#include "DlgLockerGoldEdit.h"
#include "DlgDatetime.h"

// CLockerPageMain dialog

IMPLEMENT_DYNAMIC(CLockerPageMain, CPropertyPage)

CLockerPageMain::CLockerPageMain(LOGFONT logfont, SCHARDATA2* pData)
	: CPropertyPage(CLockerPageMain::IDD)
	, m_pFont( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CLockerPageMain::~CLockerPageMain()
{
	SAFE_DELETE ( m_pFont );
}

void CLockerPageMain::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLockerPageMain, CPropertyPage)
	ON_BN_CLICKED(IDC_LOCKER_MAIN_RENTAL_BUTTON_SET_1, OnBnClickedLockerMainRentalButtonSet1)
	ON_BN_CLICKED(IDC_LOCKER_MAIN_RENTAL_BUTTON_SET_2, OnBnClickedLockerMainRentalButtonSet2)
	ON_BN_CLICKED(IDC_LOCKER_MAIN_RENTAL_BUTTON_SET_3, OnBnClickedLockerMainRentalButtonSet3)
	ON_BN_CLICKED(IDC_LOCKER_MAIN_RENTAL_BUTTON_RESET_1, OnBnClickedLockerMainRentalButtonReset1)
	ON_BN_CLICKED(IDC_LOCKER_MAIN_RENTAL_BUTTON_RESET_2, OnBnClickedLockerMainRentalButtonReset2)
	ON_BN_CLICKED(IDC_LOCKER_MAIN_RENTAL_BUTTON_RESET_3, OnBnClickedLockerMainRentalButtonReset3)
	ON_BN_CLICKED(IDC_LOCKER_MAIN_GOLD_BUTTON, OnBnClickedLockerMainGoldButton)
END_MESSAGE_MAP()


// CLockerPageMain message handlers
BOOL CLockerPageMain::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	ShowData();

	return TRUE;  
}

void CLockerPageMain::ShowData()
{
	if( !m_pData ){
		return;
	}

	SetWin_Text( this, IDC_LOCKER_MAIN_RENTAL_EDIT_1, _HLIB::cstring_timet12( m_pData->m_tSTORAGE[0] ).GetString() );
	SetWin_Text( this, IDC_LOCKER_MAIN_RENTAL_EDIT_2, _HLIB::cstring_timet12( m_pData->m_tSTORAGE[1] ).GetString() );
	SetWin_Text( this, IDC_LOCKER_MAIN_RENTAL_EDIT_3, _HLIB::cstring_timet12( m_pData->m_tSTORAGE[2] ).GetString() );
	SetWin_Num_LONGLONG( this, IDC_LOCKER_MAIN_LOCKER_GOLD, m_pData->m_lnStorageMoney );
}

void CLockerPageMain::OnBnClickedLockerMainRentalButtonSet1()
{
	if( !m_pData ){
		return;
	}

	__time64_t ttime = m_pData->m_tSTORAGE[0];
	if ( ttime < CTime::GetCurrentTime().GetTime() ){
		ttime = CTime::GetCurrentTime().GetTime();
	}

	CDlgDatetime dlg( this );
	dlg.DateSetMin( CTime::GetCurrentTime().GetTime() );
	dlg.DateTimeSet( ttime );
	if ( dlg.DoModal() == IDOK ){
		m_pData->m_tSTORAGE[0] = dlg.DateTimeGet();
		ShowData();
	}
}

void CLockerPageMain::OnBnClickedLockerMainRentalButtonSet2()
{
	if( !m_pData ){
		return;
	}

	__time64_t ttime = m_pData->m_tSTORAGE[1];
	if ( ttime < CTime::GetCurrentTime().GetTime() ){
		ttime = CTime::GetCurrentTime().GetTime();
	}

	CDlgDatetime dlg( this );
	dlg.DateSetMin( CTime::GetCurrentTime().GetTime() );
	dlg.DateTimeSet( ttime );
	if ( dlg.DoModal() == IDOK ){
		m_pData->m_tSTORAGE[1] = dlg.DateTimeGet();
		ShowData();
	}
}

void CLockerPageMain::OnBnClickedLockerMainRentalButtonSet3()
{
	if( !m_pData ){
		return;
	}

	__time64_t ttime = m_pData->m_tSTORAGE[2];
	if ( ttime < CTime::GetCurrentTime().GetTime() ){
		ttime = CTime::GetCurrentTime().GetTime();
	}

	CDlgDatetime dlg( this );
	dlg.DateSetMin( CTime::GetCurrentTime().GetTime() );
	dlg.DateTimeSet( ttime );
	if ( dlg.DoModal() == IDOK ){
		m_pData->m_tSTORAGE[2] = dlg.DateTimeGet();
		ShowData();
	}
}

void CLockerPageMain::OnBnClickedLockerMainRentalButtonReset1()
{
	if( !m_pData ){
		return;
	}

	CTime cTime( 1970, 2, 1, 0, 0, 0 );
	m_pData->m_tSTORAGE[0] = cTime.GetTime();
	ShowData();
}

void CLockerPageMain::OnBnClickedLockerMainRentalButtonReset2()
{
	if( !m_pData ){
		return;
	}

	CTime cTime( 1970, 2, 1, 0, 0, 0 );
	m_pData->m_tSTORAGE[1] = cTime.GetTime();
	ShowData();
}

void CLockerPageMain::OnBnClickedLockerMainRentalButtonReset3()
{
	if( !m_pData ){
		return;
	}

	CTime cTime( 1970, 2, 1, 0, 0, 0 );
	m_pData->m_tSTORAGE[2] = cTime.GetTime();
	ShowData();
}

void CLockerPageMain::OnBnClickedLockerMainGoldButton()
{
	if( !m_pData ){
		return;
	}

	CDlgLockerGoldEdit dlg( m_pData->m_lnStorageMoney, this );
	if ( dlg.DoModal() == IDOK ){
		 m_pData->m_lnStorageMoney = dlg.m_llValue;
		 ShowData();
	}
}
