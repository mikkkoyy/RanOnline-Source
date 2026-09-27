#pragma once

#include "GLCrowData.h"
#include "DxSkinDataDummy.h"

class	CsheetWithTab;
// CPageEdit2 dialog

class CPageEdit2 : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEdit2)

public:
	CPageEdit2(LOGFONT logfont);
	virtual ~CPageEdit2();

// Dialog Data
	enum { IDD = IDD_EDIT2 };

private:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	PCROWDATA	m_pDummyCrow;
	PCROWDATA	m_pCrow;

public:
	BOOL	CrowSet ( PCROWDATA pCROW );
	void	CrowClear();

	BOOL	DataDefault();
	BOOL	DataShow();
	BOOL	DataSave();

public:
	CListCtrl	m_listPattern;
	int			m_nAttack;
	CListBox	m_listDIV;
	CListBox	m_listANI;
	DxSkinDataDummy* m_pDataDummy;
	SANIATTACK		sAniAttack;

public:
	void AttackShow( int nATTACK );
	void AttackSave( int nATTACK );
	void InitPattern();
	void InitBlow();
	void InitSkill();
	void InitAniList();
	void InitAniInfo();

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonPageBack();
	afx_msg void OnBnClickedButtonPageCancel();
	afx_msg void OnBnClickedButtonPageSave();
	afx_msg void OnBnClickedButtonAttackMinus();
	afx_msg void OnBnClickedButtonAttackAdd();
	afx_msg void OnBnClickedButtonPatternAdd();
	afx_msg void OnBnClickedButtonPatternEdit();
	afx_msg void OnBnClickedButtonPatternDelete();
	afx_msg void OnBnClickedButtonPatternSave();
	afx_msg void OnCbnSelchangeComboBlowType();
	afx_msg void OnBnClickedButtonAttackSkill();
	afx_msg void OnBnClickedButtonSelfbody();
	afx_msg void OnBnClickedButtonTargetbody();
	afx_msg void OnBnClickedButtonTarget();
	afx_msg void OnBnClickedButtonUseAni();
	afx_msg void OnEnChangeEditAttackSkillMid();
	afx_msg void OnEnChangeEditAttackSkillSid();
};
