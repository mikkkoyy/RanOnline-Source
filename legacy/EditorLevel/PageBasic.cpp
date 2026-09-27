// PageBasic.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "PageBasic.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "EditorLevelView.h"
// CPageBasic dialog

IMPLEMENT_DYNAMIC(CPageBasic, CPropertyPage)

CPageBasic::CPageBasic(LOGFONT logfont)
	: CPropertyPage(CPageBasic::IDD)
	, m_pFont ( NULL )
	, m_pGLLand( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageBasic::~CPageBasic()
{
	SAFE_DELETE ( m_pFont );
}

void CPageBasic::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageBasic, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_WLD, OnBnClickedButtonWld)
	ON_BN_CLICKED(IDC_BUTTON_QUAD, OnBnClickedButtonQuad)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
END_MESSAGE_MAP()


// CPageBasic message handlers
BOOL CPageBasic::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	std::string strDevision[10] = 
	{
		"2","4","8","16","32","64","128","256","512","1024",
	};

	SetWin_Combo_Init( this, IDC_COMBO_QUAD, strDevision, 10 );


	return TRUE;  
}

int CPageBasic::QuadToIndex( EMDIVISION emQuad )
{
	int nRETURN = 0;
	switch( emQuad )
	{
		case QUAD_DIV_01:	nRETURN = 0;	break;
		case QUAD_DIV_02:	nRETURN = 1;	break;
		case QUAD_DIV_03:	nRETURN = 2;	break;
		case QUAD_DIV_04:	nRETURN = 3;	break;
		case QUAD_DIV_05:	nRETURN = 4;	break;
		case QUAD_DIV_06:	nRETURN = 5;	break;
		case QUAD_DIV_07:	nRETURN = 6;	break;
		case QUAD_DIV_08:	nRETURN = 7;	break;
		case QUAD_DIV_09:	nRETURN = 8;	break;
		case QUAD_DIV_10:	nRETURN = 9;	break;
		default:	nRETURN = 0;	break;
	};

	return nRETURN;
}

EMDIVISION CPageBasic::IndexToQuad( int nIndex )
{
	EMDIVISION emRETURN = QUAD_DIV_01;
	switch( nIndex )
	{
	case 0:	emRETURN = QUAD_DIV_01;	break;
	case 1:	emRETURN = QUAD_DIV_02;	break;
	case 2:	emRETURN = QUAD_DIV_03;	break;
	case 3:	emRETURN = QUAD_DIV_04;	break;
	case 4:	emRETURN = QUAD_DIV_05;	break;
	case 5:	emRETURN = QUAD_DIV_06;	break;
	case 6:	emRETURN = QUAD_DIV_07;	break;
	case 7:	emRETURN = QUAD_DIV_08;	break;
	case 8:	emRETURN = QUAD_DIV_09;	break;
	case 9:	emRETURN = QUAD_DIV_10;	break;
	};

	return emRETURN;
}

void CPageBasic::DataSet( GLLandMan* pLand )
{
	m_pGLLand = pLand;
	DataShow();
}

void CPageBasic::ResetEditor()
{
	m_pGLLand = NULL;
}

void CPageBasic::DataShow()
{
	if ( !m_pGLLand )	return;

	SetWin_Text( this, IDC_EDIT_WLDFILE, m_pGLLand->GetWldFileName() );
	SetWin_Text( this, IDC_EDIT_LEVFILE, m_pGLLand->GetFileName() );
	SetWin_Combo_Sel( this, IDC_COMBO_QUAD, QuadToIndex( m_pGLLand->GetLandTreeDiv() ) );
}

BOOL CPageBasic::DataSave()
{
	if ( !m_pGLLand )	return FALSE; 

	m_pGLLand->SetWldFileName( GetWin_Text( this, IDC_EDIT_WLDFILE ) );
	m_pGLLand->SetFileName( GetWin_Text( this, IDC_EDIT_LEVFILE ) );

	int nQUAD = GetWin_Combo_Sel( this, IDC_COMBO_QUAD );
	int nINDEX = QuadToIndex( m_pGLLand->GetLandTreeDiv() );
	m_pGLLand->SetLandTreeDiv( IndexToQuad( nQUAD ) );

	if ( nQUAD != nINDEX )
	{
		m_pGLLand->SetMap();
	}	

	return TRUE;
}

void CPageBasic::OnBnClickedButtonWld()
{
	if ( !m_pGLLand )	return;

	CString szFilter = "WLD|*.wld|";
	CFileDialog dlg ( TRUE,".WLD",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = DxLandMan::GetPath();
	if ( dlg.DoModal() == IDOK )	
	{
		BOOL bOK = m_pGLLand->LoadWldFile ( dlg.GetFileName(), m_pGLLand->GetFileVer()<=GLLevelFile::VERSION_BEFORE_GATE );
		if ( !bOK )
		{
			CDebugSet::MsgBox( "Error Load Wld:%s", dlg.GetFileName() );
			return;
		}

		CEditorLevelView::GetView()->LoadWLD( dlg.GetFileName().GetString() );

		DataShow();
	}
}

void CPageBasic::OnBnClickedButtonQuad()
{
	if ( !m_pGLLand )	return;

	int nQUAD = GetWin_Combo_Sel( this, IDC_COMBO_QUAD );
	m_pGLLand->SetLandTreeDiv( IndexToQuad( nQUAD ) );
	m_pGLLand->SetMap();
}

void CPageBasic::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		if ( m_pSheetTab )
			m_pSheetTab->PageActive( LEVELEDIT_PAGE_MAIN );
	}else{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error Save!" );
	}
}

void CPageBasic::OnBnClickedButtonClose()
{
	if ( m_pSheetTab )
		m_pSheetTab->PageActive( LEVELEDIT_PAGE_MAIN );
}
