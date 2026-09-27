#pragma once

class	CPetEditTab;
struct GLPET;

// CPetEditPageBasic dialog

class CPetEditPageBasic : public CPropertyPage
{
	DECLARE_DYNAMIC(CPetEditPageBasic)

public:
	CPetEditPageBasic(LOGFONT logfont, GLPET* pData);
	virtual ~CPetEditPageBasic();

// Dialog Data
	enum { IDD = IDD_PETEDIT_PAGE_BASIC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CPetEditTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;
	GLPET*			m_pData;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CPetEditTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

public:
	CSliderCtrl	m_slider_Food;

public:
	void	DataDefault();
	void	DataShow();
	BOOL	DataSave();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnNMCustomdrawPeteditbasicSliderPetfood(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedPeteditbasicButtonSkincrowEdit();
	afx_msg void OnBnClickedPeteditbasicButtonSkincrowDelete();
	afx_msg void OnEnChangePeteditbasicEditPetskincrowMid();
	afx_msg void OnEnChangePeteditbasicEditPetskincrowSid();
	afx_msg void OnBnClickedPeteditbasicButtonPetcolor();
	afx_msg void OnBnClickedPeteditbasicButtonPetskinstartSet();
	afx_msg void OnBnClickedPeteditbasicButtonPetskinstartReset();
};
