#pragma once

#include "GLSkill.h"

// CPageEdit1 dialog
class	CsheetWithTab;

class CPageEdit1 : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEdit1)

public:
	CPageEdit1(LOGFONT logfont);
	virtual ~CPageEdit1();

// Dialog Data
	enum { IDD = IDD_EDIT1 };

private:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	PGLSKILL	m_pDummySkill;
	PGLSKILL	m_pSkill;
	int			m_nElement;
	int			m_nLevel;
	int			m_nIndexImpact;
	int			m_nIndexSpec;

public:
	BOOL	SkillSet ( PGLSKILL pSKILL );
	void	SkillClear();

	BOOL	DataDefault();
	BOOL	DataShow();
	BOOL	DataSave();

	void	CheckReqSkill();
	void	ShowAnimationNumber();
	void	InitBasicType();

	void	EffectsShow();
	void	EffectsSave();

	void	SkillLearnShow();
	void	SkillLearnSave();

	void	ShowImpact();
	void	ShowSpec();
	void	SaveImpact();
	void	SaveSpec();

	void	Loadegp( int nID );
	void	Loadeffskin( int nID );
	void	Loadtexture( int nID );

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonPageNext();
	afx_msg void OnBnClickedButtonPageCancel();
	afx_msg void OnBnClickedButtonPageSave();;
	afx_msg void OnCbnSelchangeComboAniMain();
	afx_msg void OnCbnSelchangeComboAniSub();
	afx_msg void OnBnClickedButtonEffectMinus();
	afx_msg void OnBnClickedButtonEffectAdd();
	afx_msg void OnBnClickedButtonEffTarget1();
	afx_msg void OnBnClickedButtonEffTarget2();
	afx_msg void OnBnClickedButtonEffSelf1();
	afx_msg void OnBnClickedButtonEffSelf2();
	afx_msg void OnBnClickedButtonEffTarg();
	afx_msg void OnBnClickedButtonEffSelfbody();
	afx_msg void OnBnClickedButtonEffTargetbody1();
	afx_msg void OnBnClickedButtonEffTargetbody2();
	afx_msg void OnBnClickedButtonEffHoldout();
	afx_msg void OnCbnSelchangeComboEmelement();
	afx_msg void OnCbnSelchangeComboEmtypes();
	afx_msg void OnBnClickedButtonIcon();
	afx_msg void OnBnClickedButtonSklvlMinus();
	afx_msg void OnBnClickedButtonSklvlAdd();
	afx_msg void OnBnClickedButtonSkill();
	afx_msg void OnEnChangeEditReqskillMid();
	afx_msg void OnEnChangeEditReqskillSid();
	afx_msg void OnBnClickedButtonImpactMinus();
	afx_msg void OnBnClickedButtonImpactAdd();
	afx_msg void OnBnClickedButtonSpecMinus();
	afx_msg void OnBnClickedButtonSpecAdd();
	afx_msg void OnBnClickedButtonEffTarget3();
	afx_msg void OnBnClickedButtonEffSelf3();
	afx_msg void OnBnClickedCheckHiddenWeapon();
};
