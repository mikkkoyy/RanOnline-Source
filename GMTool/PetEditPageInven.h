#pragma once

class	CPetEditTab;
struct GLPET;

// CPetEditPageInven dialog

class CPetEditPageInven : public CPropertyPage
{
	DECLARE_DYNAMIC(CPetEditPageInven)

public:
	CPetEditPageInven(LOGFONT logfont, GLPET* pData);
	virtual ~CPetEditPageInven();

// Dialog Data
	enum { IDD = IDD_PETEDIT_PAGE_INVEN };

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
	CListCtrl	m_List;

public:
	void ShowData();
	void EditItem();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedPeteditinvenButtonEdit();
	afx_msg void OnBnClickedPeteditinvenButtonDelete();
	afx_msg void OnBnClickedPeteditinvenButtonClear();
	afx_msg void OnNMDblclkListPetinven(NMHDR *pNMHDR, LRESULT *pResult);
};
