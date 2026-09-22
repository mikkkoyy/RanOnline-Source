// PageEditGate.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "PageEditGate.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "EditorLevelView.h"
#include "GenerateName.h"
#include "DlgFindMap.h"

// CPageEditGate dialog

IMPLEMENT_DYNAMIC(CPageEditGate, CPropertyPage)

CPageEditGate::CPageEditGate(LOGFONT logfont)
	: CPropertyPage(CPageEditGate::IDD)
	, m_pFont ( NULL )
	, m_pGLLand( NULL )
	, m_pEdit( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEditGate::~CPageEditGate()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEditGate::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEditGate, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_REGNEW, OnBnClickedButtonRegnew)
	ON_EN_CHANGE(IDC_EDIT_GATE_TOMAP_MID, OnEnChangeEditGateTomapMid)
	ON_EN_CHANGE(IDC_EDIT_GATE_TOMAP_SID, OnEnChangeEditGateTomapSid)
	ON_BN_CLICKED(IDC_BUTTON_CHECK, OnBnClickedButtonCheck)
	ON_BN_CLICKED(IDC_BUTTON_NEW, OnBnClickedButtonNew)
	ON_BN_CLICKED(IDC_BUTTON_FINDMAP, OnBnClickedButtonFindmap)
	ON_BN_CLICKED(IDC_BUTTON_MOUSEPOS, OnBnClickedButtonMousepos)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
END_MESSAGE_MAP()


// CPageEditGate message handlers
BOOL CPageEditGate::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	return TRUE;  
}

void CPageEditGate::SetData( GLLandMan* pLand, PDXLANDGATE pEdit )
{
	m_pGLLand = pLand;
	m_pEdit = pEdit;

	if ( m_pGLLand && m_pEdit )
	{
		InitUI( TRUE );
		DataShow();
		JumpTo( m_pEdit->GetMax(), m_pEdit->GetMin() );
	}else if ( m_pGLLand ){
		InitUI( FALSE );
		SetWin_Num_int( this, IDC_EDIT_GATE_ID, GateNewID( m_pGLLand ) );
		SetWin_Text( this, IDC_EDIT_GATE_NAME, GateNameNew( m_pGLLand ).c_str() );
	}
}

void CPageEditGate::ResetData()
{
	m_pEdit = NULL;
	m_pGLLand = NULL;
}

void CPageEditGate::InitUI( BOOL bENABLE )
{
	SetWin_Enable( this, IDC_CHECK_GATE_IN, bENABLE );
	SetWin_Enable( this, IDC_CHECK_GATE_OUT, bENABLE );
	SetWin_Enable( this, IDC_EDIT_GATE_TOMAP_MID, bENABLE );
	SetWin_Enable( this, IDC_EDIT_GATE_TOMAP_SID, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_FINDMAP, bENABLE );

	SetWin_Enable( this, IDC_EDIT_GATE_TOMAP_GATE, bENABLE );
	SetWin_Enable( this, IDC_EDIT_GATE_GATEINTERVAL_X, bENABLE );
	SetWin_Enable( this, IDC_EDIT_GATE_GATEINTERVAL_Y, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_MOUSEPOS, bENABLE );

	SetWin_Enable( this, IDC_BUTTON_REGNEW, !bENABLE );
	SetWin_Enable( this, IDC_BUTTON_SAVE, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_APPLY, bENABLE );

	SetWin_Num_int( this, IDC_EDIT_GATE_ID, 0 );
	SetWin_Text( this, IDC_EDIT_GATE_NAME, "" );
	SetWin_Check( this, IDC_CHECK_GATE_IN, FALSE );
	SetWin_Check( this, IDC_CHECK_GATE_OUT, FALSE );
	SetWin_Num_int( this, IDC_EDIT_GATE_TOMAP_MID, 0 );
	SetWin_Num_int( this, IDC_EDIT_GATE_TOMAP_SID, 0 );
	SetWin_Num_int( this, IDC_EDIT_GATE_TOMAP_GATE, 0 );
	SetWin_Num_float( this, IDC_EDIT_GATE_GATEINTERVAL_X, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_GATE_GATEINTERVAL_Y, 0.0f );
}

void CPageEditGate::DataShow()
{
	if ( !m_pGLLand )	return;
	if ( !m_pEdit )		return;

	SetWin_Num_int( this, IDC_EDIT_GATE_ID, m_pEdit->GetGateID() );
	SetWin_Text( this, IDC_EDIT_GATE_NAME, m_pEdit->GetName() );
	SetWin_Check( this, IDC_CHECK_GATE_IN, m_pEdit->GetFlags() & DxLandGate::GATE_IN );
	SetWin_Check( this, IDC_CHECK_GATE_OUT, m_pEdit->GetFlags() & DxLandGate::GATE_OUT );
	SetWin_Num_int( this, IDC_EDIT_GATE_TOMAP_MID, m_pEdit->GetToMapID().wMainID );
	SetWin_Num_int( this, IDC_EDIT_GATE_TOMAP_SID, m_pEdit->GetToMapID().wSubID );
	SetWin_Num_int( this, IDC_EDIT_GATE_TOMAP_GATE, m_pEdit->GetToGateID() );
	SetWin_Num_float( this, IDC_EDIT_GATE_GATEINTERVAL_X, m_pEdit->GetDiv().x );
	SetWin_Num_float( this, IDC_EDIT_GATE_GATEINTERVAL_Y, m_pEdit->GetDiv().y );

	CEditorLevelView::GetView()->EditBoxSet( TRUE );
	CEditorLevelView::GetView()->m_DxEditBox.SetBox( m_pEdit->GetMax(), m_pEdit->GetMin() );
}

BOOL CPageEditGate::DataSave()
{
	if ( !m_pGLLand )	return FALSE;
	if ( !m_pEdit )		return FALSE;

	SGATEDATA_EDITOR sDATA;
	sDATA.dwGateID = GetWin_Num_int( this, IDC_EDIT_GATE_ID );
	sDATA.strGateName = GetWin_Text( this, IDC_EDIT_GATE_NAME );
	sDATA.sidMAP.wMainID = GetWin_Num_int( this, IDC_EDIT_GATE_TOMAP_MID );
	sDATA.sidMAP.wSubID = GetWin_Num_int( this, IDC_EDIT_GATE_TOMAP_SID );
	sDATA.dwToGATEID = GetWin_Num_int( this, IDC_EDIT_GATE_TOMAP_GATE );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_GATE_IN ), sDATA.dwFLAG, DxLandGate::GATE_IN );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_GATE_OUT ), sDATA.dwFLAG, DxLandGate::GATE_OUT );
	sDATA.vec2DIV.x = GetWin_Num_float( this, IDC_EDIT_GATE_GATEINTERVAL_X );
	sDATA.vec2DIV.y = GetWin_Num_float( this, IDC_EDIT_GATE_GATEINTERVAL_Y );
	sDATA.vMIN = CEditorLevelView::GetView()->m_DxEditBox.m_vMin;
	sDATA.vMAX = CEditorLevelView::GetView()->m_DxEditBox.m_vMax;

	if ( sDATA.strGateName.size() <= 0 )
	{
		CDebugSet::MsgBox( "Name Empty!" );
		return FALSE;
	}

	if ( strcmp( m_pEdit->GetName(), sDATA.strGateName.c_str() ) )
	{
		if ( GateNameCheck( m_pGLLand, sDATA.strGateName.c_str() ) )
		{
			CDebugSet::MsgBox( "Name: %s is already used! Create New name!", sDATA.strGateName.c_str() );
			return FALSE;
		}
	}

	if ( m_pEdit->GetGateID() != sDATA.dwGateID )
	{
		if ( GateIDCheck( m_pGLLand, sDATA.dwGateID ) )
		{
			CDebugSet::MsgBox( "Gate ID: %d is already used, Create New ID!", sDATA.dwGateID );
			return FALSE;
		}
	}

	m_pEdit->SetName( sDATA.strGateName.c_str() );
	m_pEdit->SetGate( sDATA.dwFLAG, sDATA.dwGateID, sDATA.sidMAP, sDATA.dwToGATEID, sDATA.vec2DIV );
	m_pEdit->SetBox( sDATA.vMAX, sDATA.vMIN );
	m_pEdit->MakeStartPos( m_pGLLand->GetNavi() );
	
	return TRUE;
}

void CPageEditGate::JumpTo( D3DXVECTOR3 vMax, D3DXVECTOR3 vMin )
{
	D3DXVECTOR3	vec3Center;
	vec3Center = ((vMin + vMax ) / 2);
	DxViewPort::GetInstance().CameraJump( vec3Center );
}

void CPageEditGate::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		if ( m_pSheetTab )
			m_pSheetTab->PageActive( LEVELEDIT_PAGE_GATELIST );
	}
}

void CPageEditGate::OnBnClickedButtonClose()
{
	if ( m_pSheetTab )
		m_pSheetTab->PageActive( LEVELEDIT_PAGE_GATELIST );
}

void CPageEditGate::OnBnClickedButtonRegnew()
{
	if ( !m_pGLLand )	return;

	D3DXVECTOR3 vPOS = CEditorLevelView::GetView()->m_vPos;

	std::string strName = GetWin_Text( this, IDC_EDIT_GATE_NAME );
	DWORD dwGateID = GetWin_Num_int( this, IDC_EDIT_GATE_ID );

	if ( strName.size() <= 0 )
	{
		CDebugSet::MsgBox( "Name Empty!" );
		return;
	}

	if ( GateNameCheck( m_pGLLand, strName.c_str() ) )
	{
		CDebugSet::MsgBox( "Name: %s is already used! Create New name!", strName.c_str() );
		return;
	}

	if ( GateIDCheck( m_pGLLand, dwGateID ) )
	{
		CDebugSet::MsgBox( "Gate ID: %d is already used, Create New ID!", dwGateID );
		return;
	}

	PDXLANDGATE pNewGate = new DxLandGate;
	pNewGate->SetGateID( dwGateID );
	pNewGate->SetName( strName.c_str() );
	pNewGate->SetPos( vPOS );
	pNewGate->MakeStartPos( m_pGLLand->GetNavi() );

	m_pGLLand->GetLandGateMan().AddLandGate( pNewGate );
	m_pEdit = m_pGLLand->GetLandGateMan().FindLandGate( pNewGate->GetGateID() );

	if ( m_pGLLand && m_pEdit )
	{
		InitUI( TRUE );
		DataShow();
		JumpTo( m_pEdit->GetMax(), m_pEdit->GetMin() );
	}
	else
	{
		m_pGLLand->GetLandGateMan().DelLandGate( pNewGate->GetGateID() );
		CDebugSet::MsgBox( "Error Register New Gate: (%d) %s", dwGateID, strName.c_str() );
		return;
	}
}

void CPageEditGate::OnEnChangeEditGateTomapMid()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_GATE_TOMAP_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_GATE_TOMAP_SID );

	std::string strMAPNAME = "Unknown Map";
	SMAPNODE* pMap = CEditorLevelView::GetView()->m_sMap.FindMapNode( SNATIVEID( wMID, wSID ) );
	if ( pMap )
		strMAPNAME = pMap->strMapName.c_str();

	SetWin_Text( this, IDC_EDIT_GATE_TOMAP_NAME, strMAPNAME.c_str() );
}

void CPageEditGate::OnEnChangeEditGateTomapSid()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_GATE_TOMAP_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_GATE_TOMAP_SID );

	std::string strMAPNAME = "Unknown Map";
	SMAPNODE* pMap = CEditorLevelView::GetView()->m_sMap.FindMapNode( SNATIVEID( wMID, wSID ) );
	if ( pMap )
		strMAPNAME = pMap->strMapName.c_str();

	SetWin_Text( this, IDC_EDIT_GATE_TOMAP_NAME, strMAPNAME.c_str() );
}

void CPageEditGate::OnBnClickedButtonCheck()
{
	if ( !m_pGLLand )	return;
	if ( !m_pEdit )		return;

	std::string strName = GetWin_Text( this, IDC_EDIT_GATE_NAME );
	DWORD dwGateID = GetWin_Num_int( this, IDC_EDIT_GATE_ID );

	if ( strcmp( m_pEdit->GetName(), strName.c_str() ) )
	{
		if ( GateNameCheck( m_pGLLand, strName.c_str() ) )
		{
			CDebugSet::MsgBox( "Name: %s is already used! Create New name!", strName.c_str() );
		}else{
			CDebugSet::MsgBox( "Name: %s is Valid!", strName.c_str() );
		}
	}

	if ( m_pEdit->GetGateID() != dwGateID )
	{
		if ( GateIDCheck( m_pGLLand, dwGateID ) )
		{
			CDebugSet::MsgBox( "Gate ID: %d is already used, Create New ID!", dwGateID );
		}else{
			CDebugSet::MsgBox( "Gate ID: %d is Valid!", dwGateID );
		}
	}
}

void CPageEditGate::OnBnClickedButtonNew()
{
	if ( !m_pGLLand )	return;

	SetWin_Num_int( this, IDC_EDIT_GATE_ID, GateNewID( m_pGLLand ) );
	SetWin_Text( this, IDC_EDIT_GATE_NAME, GateNameNew( m_pGLLand ).c_str() );
}

void CPageEditGate::OnBnClickedButtonFindmap()
{
	CDlgFindMap dlg( this );
	if ( dlg.DoModal() == IDOK )
	{
		SMAPNODE* pMap = CEditorLevelView::GetView()->m_sMap.FindMapNode( dlg.m_sSELECTED.sMAPID );
		if ( pMap )
		{
			SetWin_Num_int( this, IDC_EDIT_GATE_TOMAP_MID, pMap->sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_GATE_TOMAP_SID, pMap->sNativeID.wSubID );
		}
	}
}

void CPageEditGate::OnBnClickedButtonMousepos()
{
	if ( !m_pGLLand )	return;
	if ( !m_pEdit )		return;

	//rebuild the gate pos to current mouse pos
	D3DXVECTOR3 vPOS = CEditorLevelView::GetView()->m_vPos;
	m_pEdit->SetPos( vPOS );
	m_pEdit->MakeStartPos( m_pGLLand->GetNavi() );
	CEditorLevelView::GetView()->EditBoxSet( TRUE );
	CEditorLevelView::GetView()->m_DxEditBox.SetBox( m_pEdit->GetMax(), m_pEdit->GetMin() );
}


void CPageEditGate::OnBnClickedButtonApply()
{
	DataSave();
}
