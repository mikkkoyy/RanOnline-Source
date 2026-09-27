// DlgDatetime.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgDatetime.h"



// CDlgDatetime dialog

IMPLEMENT_DYNAMIC(CDlgDatetime, CDialog)

CDlgDatetime::CDlgDatetime(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgDatetime::IDD, pParent)
	, m_cDateMIN( 1970, 2, 1, 1, 1, 1 )
	, m_cDateMAX( 2038, 1, 19, 1, 1, 1 )
{

}

CDlgDatetime::~CDlgDatetime()
{
}

void CDlgDatetime::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MONTHCALENDAR, m_cDate);
	DDX_Control(pDX, IDC_DATETIMEPICKER, m_cTime);
}


BEGIN_MESSAGE_MAP(CDlgDatetime, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_CURTIME, OnBnClickedButtonCurtime)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgDatetime message handlers
BOOL CDlgDatetime::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_cDate.SetRange( &m_cDateMIN, &m_cDateMAX );
	m_cDate.SetCurSel( m_ccTIME );
	m_cTime.SetTime( &m_ccTIME );

	return TRUE;  
}

void CDlgDatetime::OnBnClickedButtonCurtime()
{
	m_ccTIME = CTime::GetCurrentTime();
	m_cDate.SetCurSel( m_ccTIME );
	m_cTime.SetTime( &m_ccTIME );
}

void CDlgDatetime::OnBnClickedOk()
{
	SYSTEMTIME sDATETIME;
	SYSTEMTIME sDATE;
	SYSTEMTIME sTIME;

	m_cDate.GetCurSel( &sDATE );
	m_cTime.GetTime( &sTIME );

	sDATETIME.wYear = sDATE.wYear;
	sDATETIME.wMonth = sDATE.wMonth;
	sDATETIME.wDay = sDATE.wDay;
	sDATETIME.wHour = sTIME.wHour;
	sDATETIME.wMinute = sTIME.wMinute;
	sDATETIME.wSecond = sTIME.wSecond;

	m_ccTIME = sDATETIME;

	CDialog::OnOK();
}

void CDlgDatetime::OnBnClickedCancel()
{
	CDialog::OnCancel();
}
