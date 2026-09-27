#pragma once

class	CCharEditTab;
struct SCHARDATA2;

// CCharEditPageInven dialog

class CCharEditPageInven : public CPropertyPage
{
	DECLARE_DYNAMIC(CCharEditPageInven)

public:
	CCharEditPageInven(LOGFONT logfont, SCHARDATA2* pData);
	virtual ~CCharEditPageInven();

// Dialog Data
	enum { IDD = IDD_CHAREDIT_PAGE_INVEN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CCharEditTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;
	SCHARDATA2*		m_pData;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CCharEditTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

private:
	CListCtrl	m_List;

public:
	void	ShowData();
	void	EditItem();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedChareditInvenButtonAdd();
	afx_msg void OnBnClickedChareditInvenButtonEdit();
	afx_msg void OnBnClickedChareditInvenButtonDelete();
	afx_msg void OnBnClickedChareditInvenButtonClear();
	afx_msg void OnNMDblclkChareditInvenList(NMHDR *pNMHDR, LRESULT *pResult);
};
