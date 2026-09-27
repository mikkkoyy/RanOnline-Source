#pragma once


#include "GLMapList.h"
// CPageEdit1 dialog
class	CsheetWithTab;
struct	SITEM;

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
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

public:
	SITEM*	m_pDummyItem;
	SITEM*	m_pItem;
	GLMapList	m_MapsList;

public:
	BOOL	ItemSet ( SITEM* pItem );
	void	ItemClear();

	BOOL	DataDefault();
	BOOL	DataShow();
	BOOL	DataSave();

public:
	void	LoadWearingfile( int nID );
	void	InitType();

	void	CheckPetCrow();
	void	CheckSkillScroll();
	void	CheckNpcRecCrow();
	void	CheckSpecMap();

	void	InitVehicleTypes();

	void	EnableClass( BOOL bMALE, BOOL bFEMALE );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonPageSave();
	afx_msg void OnBnClickedButtonPageCancel();
	afx_msg void OnBnClickedButtonPageNext();
	afx_msg void OnBnClickedButtonAllMale();
	afx_msg void OnBnClickedButtonAllFemale();
	afx_msg void OnBnClickedButtonAll();
	afx_msg void OnBnClickedButtonNothing();
	afx_msg void OnBnClickedButtonApplyM();
	afx_msg void OnBnClickedButtonClearM();
	afx_msg void OnBnClickedButtonApplyF();
	afx_msg void OnBnClickedButtonClearF();
	afx_msg void OnBnClickedButtonBrawlerM();
	afx_msg void OnBnClickedButtonBrawlerF();
	afx_msg void OnBnClickedButtonSwordsmanM();
	afx_msg void OnBnClickedButtonSwordsmanF();
	afx_msg void OnBnClickedButtonArcherM();
	afx_msg void OnBnClickedButtonArcherF();
	afx_msg void OnBnClickedButtonShamanM();
	afx_msg void OnBnClickedButtonShamanF();
	afx_msg void OnBnClickedButtonExtremeM();
	afx_msg void OnBnClickedButtonExtremeF();
	afx_msg void OnBnClickedButtonSelfbody();
	afx_msg void OnBnClickedButtonTargetBody();
	afx_msg void OnBnClickedButtonPartnereffect();
	afx_msg void OnBnClickedButtonPetwear();
	afx_msg void OnBnClickedButtonRandomoption();
	afx_msg void OnBnClickedButtonShapeinven();
	afx_msg void OnBnClickedButtonShapefield();
	afx_msg void OnCbnSelchangeComboItemtype();
	afx_msg void OnBnClickedButtonPetCrowAdd();
	afx_msg void OnBnClickedButtonSkillScrollAdd();
	afx_msg void OnBnClickedButtonSpecmapAdd();
	afx_msg void OnBnClickedButtonNpcrecAdd();
	afx_msg void OnEnChangeEditPetMid();
	afx_msg void OnEnChangeEditPetSid();
	afx_msg void OnEnChangeEditSkillMid();
	afx_msg void OnEnChangeEditSkillSid();
	afx_msg void OnEnChangeEditMapMid2();
	afx_msg void OnEnChangeEditMapSid2();
	afx_msg void OnEnChangeEditMapMid();
	afx_msg void OnEnChangeEditMapSid();
	afx_msg void OnBnClickedButtonGunnerM();
	afx_msg void OnBnClickedButtonGunnerF();
	afx_msg void OnBnClickedButtonBrawlerM2();
	afx_msg void OnBnClickedButtonBrawlerF2();
	afx_msg void OnBnClickedButtonSwordsmanM2();
	afx_msg void OnBnClickedButtonSwordsmanF2();
	afx_msg void OnBnClickedButtonArcherM2();
	afx_msg void OnBnClickedButtonArcherF2();
	afx_msg void OnBnClickedButtonShamanM2();
	afx_msg void OnBnClickedButtonShamanF2();
	afx_msg void OnBnClickedButtonExtremeM2();
	afx_msg void OnBnClickedButtonExtremeF2();
	afx_msg void OnBnClickedButtonGunnerM2();
	afx_msg void OnBnClickedButtonGunnerF2();
	afx_msg void OnBnClickedButtonApplyM2();
	afx_msg void OnBnClickedButtonClearM2();
	afx_msg void OnBnClickedButtonApplyF2();
	afx_msg void OnBnClickedButtonClearF2();
	afx_msg void OnCbnSelchangeComboVehicle();
	afx_msg void OnBnClickedButtonAssassinM();
	afx_msg void OnBnClickedButtonAssassinF();
	afx_msg void OnBnClickedButtonTrickerM();
	afx_msg void OnBnClickedButtonTrickerF();
	afx_msg void OnBnClickedButtonAssassinM2();
	afx_msg void OnBnClickedButtonAssassinF2();
	afx_msg void OnBnClickedButtonTrickerM2();
	afx_msg void OnBnClickedButtonTrickerF2();
	afx_msg void OnBnClickedButtonVehicleDefaultColor1();
	afx_msg void OnBnClickedButtonVehicleDefaultColor2();
	afx_msg void OnBnClickedButtonItemDefaultColor1();
	afx_msg void OnBnClickedButtonItemDefaultColor2();
};
