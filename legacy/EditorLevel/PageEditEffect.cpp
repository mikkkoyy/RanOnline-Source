// PageEditEffect.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "PageEditEffect.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "EditorLevelView.h"
#include "GenerateName.h"

// CPageEditEffect dialog

IMPLEMENT_DYNAMIC(CPageEditEffect, CPropertyPage)

CPageEditEffect::CPageEditEffect(LOGFONT logfont)
	: CPropertyPage(CPageEditEffect::IDD)
	, m_pFont ( NULL )
	, m_pGLLand( NULL )
	, m_pEdit( NULL )
{
	D3DXMatrixIdentity( &m_matWorld_OLD );
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEditEffect::~CPageEditEffect()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEditEffect::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEditEffect, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_REGNEW, OnBnClickedButtonRegnew)
	ON_BN_CLICKED(IDC_BUTTON_RESTORE, OnBnClickedButtonRestore)
	ON_BN_CLICKED(IDC_BUTTON_OPENEFFECT, OnBnClickedButtonOpeneffect)
	ON_BN_CLICKED(IDC_BUTTON_CHECK, OnBnClickedButtonCheck)
	ON_BN_CLICKED(IDC_BUTTON_NEW, OnBnClickedButtonNew)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
END_MESSAGE_MAP()


// CPageEditEffect message handlers
BOOL CPageEditEffect::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	return TRUE;  
}

void CPageEditEffect::SetData( GLLandMan* pLand, PLANDEFF pEdit )
{
	m_pGLLand = pLand;
	m_pEdit = pEdit;

	if ( m_pGLLand && m_pEdit )
	{
		InitUI( TRUE );
		DataShow();
		JumpTo( m_pEdit->m_matWorld );
		m_matWorld_OLD = m_pEdit->m_matWorld;
	}else if ( m_pGLLand ){
		InitUI( FALSE );
		SetWin_Text( this, IDC_EDIT_EFFECT_NAME, EffectNameNew( m_pGLLand ).c_str() );
	}
}

void CPageEditEffect::ResetData()
{
	m_pEdit = NULL;
	m_pGLLand = NULL;
}

void CPageEditEffect::JumpTo( D3DXMATRIX sMat )
{
	D3DXVECTOR3 vPos = D3DXVECTOR3( sMat._41, sMat._42, sMat._43 );
	vPos.y += 10.0f;
	DxViewPort::GetInstance().CameraJump( vPos );
}

void CPageEditEffect::InitUI( BOOL bENABLE )
{
	SetWin_Enable( this, IDC_BUTTON_REGNEW, !bENABLE );
	SetWin_Enable( this, IDC_BUTTON_RESTORE, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_SAVE, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_APPLY, bENABLE );

	SetWin_Text( this, IDC_EDIT_EFFECT_NAME, "" );
	SetWin_Text( this, IDC_EDIT_EFFECT_FILE, "" );
}

void CPageEditEffect::DataShow()
{
	if ( !m_pGLLand )	return;
	if ( !m_pEdit )		return;

	SetWin_Text( this, IDC_EDIT_EFFECT_NAME, m_pEdit->m_szName );
	SetWin_Text( this, IDC_EDIT_EFFECT_FILE, m_pEdit->m_szFileName );

	CEditorLevelView::GetView()->EditMRSSet( TRUE );
	CEditorLevelView::GetView()->m_DxEditMRS.SetMatrix( &m_pEdit->m_matWorld );
}

BOOL CPageEditEffect::DataSave()
{
	if ( !m_pGLLand )	return FALSE;
	if ( !m_pEdit )		return FALSE;

	std::string strNAME = GetWin_Text( this, IDC_EDIT_EFFECT_NAME );
	std::string strFILE = GetWin_Text( this, IDC_EDIT_EFFECT_FILE );

	if ( strNAME.size() <= 0 )
	{
		CDebugSet::MsgBox( "Name Empty!" );
		return FALSE;
	}

	if ( strcmp( m_pEdit->m_szName, strNAME.c_str() ) )
	{
		bool bEXIST = EffectNameCheck( m_pGLLand, strNAME.c_str() );
		if ( bEXIST )
		{
			CDebugSet::MsgBox("Name: %s is already used! Create New Name!", strNAME.c_str() );
			return FALSE;
		}
	}

	m_pEdit->SetEffect( (char*)strNAME.c_str(), m_pEdit->m_matWorld, strFILE.c_str(), CEditorLevelView::GetView()->Get3DDevice() );
	m_pGLLand->BuildSingleEffTree();

	return TRUE;
}

void CPageEditEffect::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		if ( m_pSheetTab )
			m_pSheetTab->PageActive( LEVELEDIT_PAGE_EFFECTLIST );
	}
}

void CPageEditEffect::OnBnClickedButtonClose()
{
	if ( m_pSheetTab )
		m_pSheetTab->PageActive( LEVELEDIT_PAGE_EFFECTLIST );
}

void CPageEditEffect::OnBnClickedButtonRegnew()
{
	if ( !m_pGLLand )	return;

	std::string strNAME = GetWin_Text( this, IDC_EDIT_EFFECT_NAME );
	std::string strFILE = GetWin_Text( this, IDC_EDIT_EFFECT_FILE );
	
	if ( strNAME.size() <= 0 )
	{
		CDebugSet::MsgBox( "Name Empty!" );
		return;
	}

	if ( EffectNameCheck( m_pGLLand, strNAME.c_str() ) )
	{
		CDebugSet::MsgBox("Name: %s is already used! Create New Name!", strNAME.c_str() );
		return;
	}

	if ( strFILE.size() == 0 )
	{
		CDebugSet::MsgBox( "Empty Effect File!" );
		return;
	}

	PLANDEFF pEFF = new DXLANDEFF;

	D3DXVECTOR3 vPOS = CEditorLevelView::GetView()->m_vPos;
	D3DXMATRIX matNew;
	D3DXMatrixIdentity( &matNew );
	D3DXMatrixTranslation ( &matNew, vPOS.x, vPOS.y, vPOS.z );

	HRESULT hr = pEFF->SetEffect( (char*)strNAME.c_str(), matNew, strFILE.c_str(), CEditorLevelView::GetView()->Get3DDevice() );
	if ( FAILED( hr ) )
	{
		SAFE_DELETE( pEFF );
		CDebugSet::MsgBox( "Error Register New Eff SetEffect" );
		return;
	}

	m_pGLLand->AddLandEff( pEFF );
	m_pGLLand->BuildSingleEffTree();

	m_pEdit = m_pGLLand->FindLandEff( pEFF->m_szName );

	if ( m_pGLLand && m_pEdit )
	{
		InitUI( TRUE );
		DataShow();
		JumpTo( m_pEdit->m_matWorld );
		m_matWorld_OLD = m_pEdit->m_matWorld;
	}
	else
	{
		m_pGLLand->DelLandEff( pEFF );
		CDebugSet::MsgBox( "Error Register New Eff Cant find" );
		return;
	}
}

void CPageEditEffect::OnBnClickedButtonRestore()
{
	if ( !m_pGLLand )	return;
	if ( !m_pEdit )		return;
	m_pEdit->m_matWorld = m_matWorld_OLD;
}

void CPageEditEffect::OnBnClickedButtonOpeneffect()
{
	CString szFilter = "Effect File (*.egp)|*.egp|";
	CFileDialog dlg(TRUE,".egp",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = DxEffSinglePropGMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		SetWin_Text ( this, IDC_EDIT_EFFECT_FILE, dlg.GetFileName().GetString() );
	}
}

void CPageEditEffect::OnBnClickedButtonCheck()
{
	if ( !m_pGLLand )	return;
	if ( !m_pEdit )		return;

	std::string strName = GetWin_Text( this, IDC_EDIT_EFFECT_NAME );
	if ( strcmp( m_pEdit->m_szName, strName.c_str() ) )
	{
		if ( GateNameCheck( m_pGLLand, strName.c_str() ) )
		{
			CDebugSet::MsgBox( "Name: %s is already used! Create New name!", strName.c_str() );
		}else{
			CDebugSet::MsgBox( "Name: %s is Valid!", strName.c_str() );
		}
	}
}

void CPageEditEffect::OnBnClickedButtonNew()
{
	if ( !m_pGLLand )	return;
	SetWin_Text( this, IDC_EDIT_EFFECT_NAME, EffectNameNew( m_pGLLand ).c_str() );
}

void CPageEditEffect::OnBnClickedButtonApply()
{
	DataSave();
}
