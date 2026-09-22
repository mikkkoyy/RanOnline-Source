// DlgInput.cpp : implementation file
//

#include "stdafx.h"
#include "EditorQuest.h"
#include "DlgInput.h"
#include "EtcFunction.h"
#include "GLQuestMan.h"
#include "GLQuest.h"
#include <cctype>


// CDlgInput dialog

IMPLEMENT_DYNAMIC(CDlgInput, CDialog)

CDlgInput::CDlgInput(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgInput::IDD, pParent)
	, m_nNEWID(0)
{
	m_strNEWNAME = "";
}

CDlgInput::~CDlgInput()
{
}

void CDlgInput::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgInput, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_CREATE_ID, OnBnClickedButtonCreateId)
	ON_BN_CLICKED(IDC_BUTTON_CREATE_NAME, OnBnClickedButtonCreateName)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgInput message handlers
BOOL CDlgInput::OnInitDialog()
{
	CDialog::OnInitDialog();

	OnBnClickedButtonCreateId();
	OnBnClickedButtonCreateName();

	return TRUE;
}

void CDlgInput::OnBnClickedButtonCreateId()
{
	m_nNEWID = GLQuestMan::GetInstance().MakeNewQNID();
	SetWin_Num_int( this, IDC_EDIT_ID, m_nNEWID );
}

void CDlgInput::OnBnClickedButtonCreateName()
{
	m_nNEWID = GetWin_Num_int( this, IDC_EDIT_ID );
	m_strNEWNAME.Format( "QUEST_%d.qst", m_nNEWID );
	SetWin_Text( this, IDC_EDIT_NAME, m_strNEWNAME );
}

void CDlgInput::OnBnClickedOk()
{
	m_nNEWID = GetWin_Num_int( this, IDC_EDIT_ID );
	m_strNEWNAME = GetWin_Text( this, IDC_EDIT_NAME );

	GLQUEST* pQuest = GLQuestMan::GetInstance().Find( m_nNEWID );
	if ( pQuest )
	{
		MessageBox ( "ID Already Exist" );
		return;
	}

	if ( m_strNEWNAME.GetLength() <= 0 )
	{
		MessageBox ( "Name Empty!" );
		return;
	}

	if ( m_nNEWID >= MAXWORD )
	{
		MessageBox ( "Max ID!" );
		return;
	}

	std::string strFILENAME = m_strNEWNAME.GetString();
	std::transform ( strFILENAME.begin(), strFILENAME.end(), strFILENAME.begin(), std::tolower );
	GLQUEST	sQUEST;
	BOOL bLOAD = sQUEST.LOAD( strFILENAME.c_str() );
	if ( bLOAD )
	{
		MessageBox ( "Name In Use!" );
		return;
	}


	OnOK();
}

