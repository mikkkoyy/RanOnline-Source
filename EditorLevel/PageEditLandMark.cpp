// PageEditLandMark.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "PageEditLandMark.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "EditorLevelView.h"
#include "GenerateName.h"
#include "GLLandMark.h"

// CPageEditLandMark dialog

IMPLEMENT_DYNAMIC(CPageEditLandMark, CPropertyPage)

CPageEditLandMark::CPageEditLandMark(LOGFONT logfont)
	: CPropertyPage(CPageEditLandMark::IDD)
	, m_pFont ( NULL )
	, m_pGLLand( NULL )
	, m_pEdit( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEditLandMark::~CPageEditLandMark()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEditLandMark::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEditLandMark, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_REGNEW, OnBnClickedButtonRegnew)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_CHECK, OnBnClickedButtonCheck)
	ON_BN_CLICKED(IDC_BUTTON_NEW, OnBnClickedButtonNew)
END_MESSAGE_MAP()


// CPageEditLandMark message handlers
BOOL CPageEditLandMark::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	return TRUE;  
}

void CPageEditLandMark::SetData( GLLandMan* pLand, PLANDMARK pEdit )
{
	m_pGLLand = pLand;
	m_pEdit = pEdit;

	if ( m_pGLLand && m_pEdit )
	{
		InitUI( TRUE );
		DataShow();
		JumpTo( m_pEdit->m_pAffineParts->vTrans );
	}else if ( m_pGLLand ){
		InitUI( FALSE );
		SetWin_Text( this, IDC_EDIT_LM_NAME, LandMarkNameNew( m_pGLLand ).c_str() );
	}
}

void CPageEditLandMark::ResetData()
{
	m_pEdit = NULL;
	m_pGLLand = NULL;
}

void CPageEditLandMark::JumpTo( D3DXVECTOR3 vPOS )
{
	vPOS.y += 10.0f;
	DxViewPort::GetInstance().CameraJump( vPOS );
}

void CPageEditLandMark::InitUI( BOOL bENABLE )
{
	SetWin_Enable( this, IDC_BUTTON_REGNEW, !bENABLE );
	SetWin_Enable( this, IDC_BUTTON_SAVE, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_APPLY, bENABLE );

	SetWin_Text( this, IDC_EDIT_LM_NAME, "" );
}

void CPageEditLandMark::DataShow()
{
	if ( !m_pGLLand )	return;
	if ( !m_pEdit )		return;

	SetWin_Text( this, IDC_EDIT_LM_NAME, m_pEdit->m_strMARK.c_str() );

	CEditorLevelView::GetView()->EditSmatSet( TRUE );
	CEditorLevelView::GetView()->m_DxEditMat.SetAffineMatrix( m_pEdit );
}

BOOL CPageEditLandMark::DataSave()
{
	if ( !m_pGLLand )	return FALSE;
	if ( !m_pEdit )		return FALSE;

	std::string strNAME = GetWin_Text( this, IDC_EDIT_LM_NAME );

	if ( strNAME.size() <= 0 )
	{
		CDebugSet::MsgBox( "Name Empty!" );
		return FALSE;
	}

	if ( strcmp( m_pEdit->m_strMARK.c_str(), strNAME.c_str() ) )
	{
		if ( LandMarkNameCheck( m_pGLLand, strNAME.c_str() ) )
		{
			CDebugSet::MsgBox( "Name: %s is already used! Create New name!", strNAME.c_str() );
			return FALSE;
		}
	}

	m_pEdit->m_strMARK = strNAME.c_str();

	return TRUE;
}

void CPageEditLandMark::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		if ( m_pSheetTab )
			m_pSheetTab->PageActive( LEVELEDIT_PAGE_LANDMARKLIST );
	}
}

void CPageEditLandMark::OnBnClickedButtonClose()
{
	if ( m_pSheetTab )
		m_pSheetTab->PageActive( LEVELEDIT_PAGE_LANDMARKLIST );
}

void CPageEditLandMark::OnBnClickedButtonRegnew()
{
	if ( !m_pGLLand )	return;

	D3DXVECTOR3 vPOS = CEditorLevelView::GetView()->m_vPos;
	std::string strName = GetWin_Text( this, IDC_EDIT_LM_NAME );

	if ( strName.size() <= 0 )
	{
		CDebugSet::MsgBox( "Empty Name!" );
		return;
	}

	bool bEXIST = LandMarkNameCheck( m_pGLLand, strName.c_str() );
	if ( bEXIST )
	{
		CDebugSet::MsgBox( "Name: %s is already used! Create New name!", strName.c_str() );
		return;
	}

	PLANDMARK	pLandMark = new GLLANDMARK;
	pLandMark->m_pAffineParts->vTrans = vPOS;
	pLandMark->m_strMARK = strName.c_str();
	pLandMark->CALC_BOX();

	m_pGLLand->AddLandMark( pLandMark );
	m_pGLLand->BuildLandMarkTree();

	m_pEdit = m_pGLLand->FindLandMark( (char*)pLandMark->m_strMARK.c_str() );

	if ( m_pGLLand && m_pEdit )
	{
		InitUI( TRUE );
		DataShow();
		JumpTo( m_pEdit->m_pAffineParts->vTrans );
	}
	else
	{
		m_pGLLand->DelLandMark( pLandMark );
		CDebugSet::MsgBox( "Register New LandMark error!" );
	}
}

void CPageEditLandMark::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEditLandMark::OnBnClickedButtonCheck()
{
	if ( !m_pGLLand )	return;
	if ( !m_pEdit )		return;

	std::string strName = GetWin_Text( this, IDC_EDIT_LM_NAME );
	if ( strcmp( m_pEdit->m_strMARK.c_str(), strName.c_str() ) )
	{
		if ( LandMarkNameCheck( m_pGLLand, strName.c_str() ) )
		{
			CDebugSet::MsgBox( "Name: %s is already used! Create New name!", strName.c_str() );
		}else{
			CDebugSet::MsgBox( "Name: %s is Valid!", strName.c_str() );
		}
	}
}

void CPageEditLandMark::OnBnClickedButtonNew()
{
	if ( !m_pGLLand )	return;
	SetWin_Text( this, IDC_EDIT_LM_NAME, LandMarkNameNew( m_pGLLand ).c_str() );
}
