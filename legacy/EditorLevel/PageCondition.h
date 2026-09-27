#pragma once

#include "GLLandMan.h"

class	CsheetWithTab;
// CPageCondition dialog

class CPageCondition : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageCondition)

public:
	CPageCondition(LOGFONT logfont);
	virtual ~CPageCondition();

// Dialog Data
	enum { IDD = IDD_PAGE_CONDITION };

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

public:
	void	DataSet( GLLandMan* pLand );
	void	ResetEditor();

	void	DataShow();
	BOOL	DataSave();

	void	ShowItemName();
	void	ShowSkillName();
	void	ShowQuestName( int nID_IN, int nID_OUT );

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonReqlevel();
	afx_msg void OnBnClickedButtonReqitem();
	afx_msg void OnBnClickedButtonReqskill();
	afx_msg void OnBnClickedButtonReqcomquest();
	afx_msg void OnBnClickedButtonReqactquest();
	afx_msg void OnBnClickedButtonReqlife();
	afx_msg void OnBnClickedButtonReqbright();
	afx_msg void OnEnChangeEditReqitemMid();
	afx_msg void OnEnChangeEditReqitemSid();
	afx_msg void OnEnChangeEditReqskillMid();
	afx_msg void OnEnChangeEditReqskillSid();
	afx_msg void OnEnChangeEditReqcomquestid();
	afx_msg void OnEnChangeEditReqactquestid();
	afx_msg void OnBnClickedButtonReqitemRemove();
	afx_msg void OnBnClickedButtonReqskillRemove();
	afx_msg void OnBnClickedButtonReqcomquestRemove();
	afx_msg void OnBnClickedButtonReqactquestRemove();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonReset();
	afx_msg void OnBnClickedButtonReqActP();
	afx_msg void OnBnClickedButtonReqConP();
};
