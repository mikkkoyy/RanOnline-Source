#pragma once

#include "GLSkill.h"

// CPageEdit2 dialog
class	CsheetWithTab;

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
	PGLSKILL	m_pDummySkill;
	PGLSKILL	m_pSkill;
	int			m_nLevel;
	int			m_nIndexImpact;
	int			m_nIndexSpec;

public:
	BOOL	SkillSet ( PGLSKILL pSKILL );
	void	SkillClear();

	BOOL	DataDefault();
	BOOL	DataShow();
	BOOL	DataSave();

	void	LevelShow();
	void	LevelSave();

	void	SpecDataShow();
	void	SpecDataSave();

	void	ImpactDataShow();
	void	ImpactDataSave();

	void	InitStateBlow();
	void	InitImpact();
	void	InitSpec();
	void	InitSpecialSkill();

	void	CheckActiveSkill();
	void	CheckLinkSkill();

	void	Loadegp( int nID );
	void	LoadPiece( int nID );

	void	CheckSummonCrow();

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
	afx_msg void OnBnClickedButtonLevelMinus();
	afx_msg void OnBnClickedButtonLevelAdd();
	afx_msg void OnBnClickedButtonSkill();
	afx_msg void OnBnClickedButtonSpsEffect();
	afx_msg void OnBnClickedButtonHeadM();
	afx_msg void OnBnClickedButtonUpperM();
	afx_msg void OnBnClickedButtonLowerM();
	afx_msg void OnBnClickedButtonHandM();
	afx_msg void OnBnClickedButtonLhandM();
	afx_msg void OnBnClickedButtonRhandM();
	afx_msg void OnBnClickedButtonFootM();
	afx_msg void OnBnClickedButtonFaceM();
	afx_msg void OnBnClickedButtonHeadF();
	afx_msg void OnBnClickedButtonUpperF();
	afx_msg void OnBnClickedButtonLowerF();
	afx_msg void OnBnClickedButtonHandF();
	afx_msg void OnBnClickedButtonLhandF();
	afx_msg void OnBnClickedButtonRhandF();
	afx_msg void OnBnClickedButtonFootF();
	afx_msg void OnBnClickedButtonFaceF();
	afx_msg void OnEnChangeEditAsMid();
	afx_msg void OnEnChangeEditAsSid();
	afx_msg void OnBnClickedButtonImpactMinus();
	afx_msg void OnBnClickedButtonImpactAdd();
	afx_msg void OnBnClickedButtonSpecMinus();
	afx_msg void OnBnClickedButtonSpecAdd2();
	afx_msg void OnBnClickedButtonSumCrow();
	afx_msg void OnEnChangeEditSumMid();
	afx_msg void OnEnChangeEditSumSid();
	afx_msg void OnBnClickedButtonLsSkill();
	afx_msg void OnEnChangeEditLsMid();
	afx_msg void OnEnChangeEditLsSid();
	afx_msg void OnBnClickedCheckCureNumb();
	afx_msg void OnBnClickedCheckCureStun();
	afx_msg void OnBnClickedCheckCureStone();
	afx_msg void OnBnClickedCheckCureBurn();
	afx_msg void OnBnClickedCheckCureFrozen();
	afx_msg void OnBnClickedCheckCureMad();
	afx_msg void OnBnClickedCheckCurePoison();
	afx_msg void OnBnClickedCheckCureCurse();
};
