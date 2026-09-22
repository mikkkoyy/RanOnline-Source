#pragma once

#include "GLLandMan.h"

class	CsheetWithTab;
// CPageBasic dialog

class CPageBasic : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageBasic)

public:
	CPageBasic(LOGFONT logfont);
	virtual ~CPageBasic();

// Dialog Data
	enum { IDD = IDD_PAGE_BASIC };

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

private:
	int	QuadToIndex( EMDIVISION emQuad );
	EMDIVISION IndexToQuad( int nIndex );

public:
	void	DataSet( GLLandMan* pLand );
	void	ResetEditor();

	void	DataShow();
	BOOL	DataSave();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonWld();
	afx_msg void OnBnClickedButtonQuad();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
};
