// DlgCharCopy.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgCharCopy.h"

#include "GLCharData.h"
#include "Logic/GMToolOdbcBase.h"
#include "EtcFunction.h"
#include "Logic/GMToolGlobal.h"


// DlgCharCopy dialog

IMPLEMENT_DYNAMIC(CDlgCharCopy, CDialog)

CDlgCharCopy::CDlgCharCopy(CWnd* pParent /*=NULL*/, 
	DWORD dwCHARID /*= SNATIVEID::ID_NULL*/, 
	DWORD dwUSERID /*= SNATIVEID::ID_NULL */,
	std::string strNAME /*= ""*/)
	: CDialog(CDlgCharCopy::IDD, pParent)
	, m_dwCHARID(SNATIVEID::ID_NULL)
	, m_dwUSERID(SNATIVEID::ID_NULL)
	, m_strNAME("")
	, m_pData(NULL)
{
	m_dwCHARID = dwCHARID;
	m_dwUSERID = dwUSERID;
	m_strNAME = strNAME.c_str();
}

CDlgCharCopy::~CDlgCharCopy()
{
	SAFE_DELETE( m_pData );
}

void CDlgCharCopy::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgCharCopy, CDialog)
	ON_BN_CLICKED(IDC_COPY_BUTTON_CHECKUSER, OnBnClickedCopyButtonCheckuser)
	ON_BN_CLICKED(IDC_COPY_BUTTON_CHECKNAME, OnBnClickedCopyButtonCheckname)
	ON_BN_CLICKED(IDC_COPY_BUTTON_COPY, OnBnClickedCopyButtonCopy)
END_MESSAGE_MAP()


// DlgCharCopy message handlers
BOOL CDlgCharCopy::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_pData = new SCHARDATA2;
	
	int nRETURN = CGMToolOdbcBase::GetInstance()->CharDataGet( m_dwUSERID, m_dwCHARID, m_pData );
	if ( nRETURN != DB_OK ){
		SAFE_DELETE( m_pData );
		CDebugSet::MsgBox( GetSafeHwnd(), "Failure to Read Character Data! Info( UserID:%d CharID:%d ChaName:%s )", m_dwUSERID, m_dwCHARID, m_strNAME.c_str() );
		return TRUE;
	}

	SetWin_Num_int( this, IDC_COPY_CHARUSER, m_pData->GetUserID() );
	SetWin_Num_int( this, IDC_COPY_CHARID, m_pData->m_dwCharID );
	SetWin_Text( this, IDC_COPY_CHARNAME, m_pData->m_szName );
	SetWin_Text( this, IDC_COPY_NEWNAME, m_pData->m_szName );

	return TRUE;
}

BOOL CDlgCharCopy::CheckUser()
{
	int nUSER = GetWin_Num_int( this, IDC_COPY_NEWUSER );
	if ( nUSER <= 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid User:%d", nUSER );
		GotoDlgCtrl(GetDlgItem(IDC_COPY_NEWUSER)); 
		return FALSE;
	}

	SGMTOOL_DATA_USER sUser;
	int nReturn = CGMToolOdbcBase::GetInstance()->UserDataGet( nUSER, &sUser );
	if ( nReturn != DB_OK ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid User:%d", nUSER );
		GotoDlgCtrl(GetDlgItem(IDC_COPY_NEWUSER)); 
		return FALSE;
	}

	return TRUE;
}

BOOL CDlgCharCopy::CheckName()
{
	std::string strName = GetWin_Text( this, IDC_COPY_NEWNAME );

	if ( strName.size() < MIN_CHAR_ID || strName.size() > MAX_CHAR_ID ){
		CDebugSet::MsgBox( GetSafeHwnd(), "CharName can only have %d to %d characters", MIN_CHAR_ID, MAX_CHAR_ID );
		GotoDlgCtrl(GetDlgItem(IDC_COPY_NEWNAME)); 
		return FALSE;
	}

	if ( GMTOOL_GLOBAL::IsBadString( strName.c_str() ) ){
		CDebugSet::MsgBox( GetSafeHwnd(), "CharName contains invalid characters!" );
		GotoDlgCtrl(GetDlgItem(IDC_COPY_NEWNAME)); 
		return FALSE;
	}

	BOOL bExist = FALSE;
	int nChaNum = 0;
	int nReturn = CGMToolOdbcBase::GetInstance()->CharNameCheck( strName.c_str(), bExist, nChaNum );
	if ( nReturn == DB_ERROR ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Unable to check database for existing name! Report this bug!" );
		return FALSE;
	}

	if( bExist )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Name:%s already user by CharID:%d", strName.c_str(), nChaNum );
		GotoDlgCtrl(GetDlgItem(IDC_COPY_NEWNAME)); 
		return FALSE;
	}

	return TRUE;
}

void CDlgCharCopy::OnBnClickedCopyButtonCheckuser()
{
	if ( CheckUser() ){
		CDebugSet::MsgBox( GetSafeHwnd(), "User Valid" );
	}
}

void CDlgCharCopy::OnBnClickedCopyButtonCheckname()
{
	if ( CheckName() ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Name valid" );
	}
}

void CDlgCharCopy::OnBnClickedCopyButtonCopy()
{
	if ( !m_pData )	return;
	if ( !CheckUser() ) return;
	if ( !CheckName() ) return;

	int nUSER = GetWin_Num_int( this, IDC_COPY_NEWUSER );
	std::string strName = GetWin_Text( this, IDC_COPY_NEWNAME );

	StringCchCopy( m_pData->m_szName, CHAR_SZNAME, strName.c_str() );
	StringCchCopy( m_pData->m_szNick, CHAR_SZNAME, "" );
	m_pData->m_dwGuild = CLUB_NULL;

	int nReturn = CGMToolOdbcBase::GetInstance()->CharDataCopy( nUSER, m_pData );
	if ( nReturn != DB_OK ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Copy Failed!" );
		return;
	}

	CDebugSet::MsgBox( GetSafeHwnd(), "Copy Done" );
}
