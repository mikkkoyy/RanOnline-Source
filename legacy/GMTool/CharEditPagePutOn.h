#pragma once

class	CCharEditTab;
struct SCHARDATA2;

// CCharEditPagePutOn dialog

class CCharEditPagePutOn : public CPropertyPage
{
	DECLARE_DYNAMIC(CCharEditPagePutOn)

public:
	CCharEditPagePutOn(LOGFONT logfont, SCHARDATA2* pData);
	virtual ~CCharEditPagePutOn();

// Dialog Data
	enum { IDD = IDD_CHAREDIT_PAGE_PUTON };

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

private:
	void	DataShow();
	void	DataEdit();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedChareditPutonButtonEdit();
	afx_msg void OnBnClickedChareditPutonButtonDelete();
	afx_msg void OnBnClickedChareditPutonButtonClear();
	afx_msg void OnNMDblclkChareditPutonList(NMHDR *pNMHDR, LRESULT *pResult);
};
