// DlgFindSkill.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "DlgFindSkill.h"
#include "EtcFunction.h"
#include <algorithm>
#include "HLibDataConvert.h"

// CDlgFindSkill dialog

IMPLEMENT_DYNAMIC(CDlgFindSkill, CDialog)

CDlgFindSkill::CDlgFindSkill(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFindSkill::IDD, pParent)
	, m_sSELECTED( false )
{
	m_vecSkill.clear();
	m_vecResult.clear();
}

CDlgFindSkill::~CDlgFindSkill()
{
	m_vecSkill.clear();
	m_vecResult.clear();
}

void CDlgFindSkill::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CDlgFindSkill, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_OK2, OnBnClickedButtonOk2)
	ON_BN_CLICKED(IDC_BUTTON_HELPER_SEARCH, OnBnClickedButtonHelperSearch)
END_MESSAGE_MAP()


// CDlgFindSkill message handlers
BOOL CDlgFindSkill::OnInitDialog()
{
	CDialog::OnInitDialog();

	RECT rectCtrl;
	LONG lnWidth;
	LV_COLUMN lvColumn;

	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	lnWidth = rectCtrl.right - rectCtrl.left;

	const int nColumnCount = 3;	
	char* szColumnName[nColumnCount] = { "Main", "Sub", "Name" };
	int nColumnWidthPercent[nColumnCount] = { 15, 15, 70, };

	for ( int i = 0; i < nColumnCount; i++ )
	{		
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.pszText = szColumnName[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = ( lnWidth*nColumnWidthPercent[i] ) / 100;
		m_List.InsertColumn (i, &lvColumn );
	}

	ReadSkill();

	const int nsIZE = 3;
	std::string	strSearchSkill[nsIZE] =
	{
		_T("Search SkillName"),
		_T("Search SkillMID"),
		_T("Search SkillSID"),
	};

	SetWin_Combo_Init( this, IDC_COMBO_SKILL_SEARCH, strSearchSkill, nsIZE );
	SetWin_Text( this, IDC_EDIT_SKILL_SEARCH, "" );

	DataSearch();

	return TRUE;  
}

void CDlgFindSkill::ReadSkill()
{
	m_vecSkill.clear();

	for ( WORD MID=0; MID<EMSKILLCLASS_NSIZE; ++MID )
	{
		for ( WORD SID=0; SID<GLSkillMan::MAX_CLASSSKILL; ++SID )
		{
			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( MID, SID );
			if ( pSkill )
			{
				SSKILLDATA sDATA;
				sDATA.sSKILLID = pSkill->m_sBASIC.sNATIVEID;
				sDATA.strSKILLNAME = pSkill->GetName();
				m_vecSkill.push_back( sDATA );
			}
		}
	}
}

void CDlgFindSkill::ShowResults()
{
	m_List.DeleteAllItems();
	
	int nSIZE = (int)m_vecResult.size();
	if ( nSIZE <= 0 )	return;

	m_List.SetRedraw( FALSE );

	for ( int i = 0; i < nSIZE; i++ )
	{
		SSKILLDATA sDATA = m_vecResult[i];
		m_List.InsertItem( i, _HLIB::cstring_dword(sDATA.sSKILLID.wMainID).GetString() );
		m_List.SetItemText( i, 1, _HLIB::cstring_dword(sDATA.sSKILLID.wSubID).GetString() );
		m_List.SetItemText( i, 2, sDATA.strSKILLNAME.c_str() );
		m_List.SetItemData ( i, sDATA.sSKILLID.dwID );
	}

	m_List.SetRedraw( TRUE );
}

void CDlgFindSkill::OnBnClickedButtonOk2()
{
	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	DWORD dwID = (DWORD) m_List.GetItemData ( nSelect );

	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( SNATIVEID( dwID ) );
	if( pSKILL )
	{
		m_sSELECTED = pSKILL->m_sBASIC.sNATIVEID;
		OnOK();
	}
}

void CDlgFindSkill::OnBnClickedButtonHelperSearch()
{
	DataSearch();
}

void CDlgFindSkill::DataSearch()
{
	int nSEARCH = GetWin_Combo_Sel( this, IDC_COMBO_SKILL_SEARCH );
	std::string strSEARCH = GetWin_Text( this, IDC_EDIT_SKILL_SEARCH );

	m_vecResult.clear();

	if ( strSEARCH.size() <= 0 )
	{
		m_vecResult = m_vecSkill;
	}
	else
	{
		for( int i = 0; i < (int)m_vecSkill.size(); ++ i )
		{
			CString strFIND;
			switch( nSEARCH )
			{
			case 0:
				{
					strFIND.Format( "%s", m_vecSkill[i].strSKILLNAME.c_str() );
				}break;
			case 1:
				{
					strFIND.Format( "%d", m_vecSkill[i].sSKILLID.wMainID );
				}break;
			case 2:
				{
					strFIND.Format( "%d", m_vecSkill[i].sSKILLID.wSubID );
				}break;
			};

			if ( strFIND.GetLength() <= 0 )	continue;

			std::string strSEARCH1 = strSEARCH.c_str();
			std::string strSEARCH2 = strFIND.GetString();
			std::transform ( strSEARCH1.begin(), strSEARCH1.end(), strSEARCH1.begin(), tolower );
			std::transform ( strSEARCH2.begin(), strSEARCH2.end(), strSEARCH2.begin(), tolower );

			unsigned found = strSEARCH2.find(strSEARCH1.c_str());
			if (found!=std::string::npos)
			{
				m_vecResult.push_back( m_vecSkill[i] );
			}
		}
	}

	ShowResults();
}