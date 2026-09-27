#pragma once

#include "GLLandMan.h"

class	CsheetWithTab;
// CPageEditGate dialog

struct SGATEDATA_EDITOR
{
	std::string strGateName;
	DWORD		dwGateID;
	SNATIVEID	sidMAP;
	DWORD		dwToGATEID;
	DWORD		dwFLAG;
	D3DXVECTOR2	vec2DIV;
	D3DXVECTOR3	vMAX;
	D3DXVECTOR3	vMIN;

	SGATEDATA_EDITOR()
		: dwGateID( 0 )
		, sidMAP( false )
		, dwToGATEID( 0 )
		, dwFLAG( 0 )
		, vec2DIV( 0.0f, 0.0f )
		, vMAX( 0.0f, 0.0f, 0.0f )
		, vMIN( 0.0f, 0.0f, 0.0f )
	{
		strGateName = "";
	}
};

class CPageEditGate : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEditGate)

public:
	CPageEditGate(LOGFONT logfont);
	virtual ~CPageEditGate();

// Dialog Data
	enum { IDD = IDD_PAGE_EDIT_GATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

protected:
	GLLandMan*		m_pGLLand;
	PDXLANDGATE		m_pEdit;
	
public:
	void	SetData( GLLandMan* pLand, PDXLANDGATE pEdit );
	void	ResetData();

	void	InitUI( BOOL bENABLE );
	void	DataShow();
	BOOL	DataSave();
	void	JumpTo( D3DXVECTOR3 vMax, D3DXVECTOR3 vMin );

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonRegnew();
	afx_msg void OnEnChangeEditGateTomapMid();
	afx_msg void OnEnChangeEditGateTomapSid();
	afx_msg void OnBnClickedButtonCheck();
	afx_msg void OnBnClickedButtonNew();
	afx_msg void OnBnClickedButtonFindmap();
	afx_msg void OnBnClickedButtonMousepos();
	afx_msg void OnBnClickedButtonApply();
};
