#pragma once


// CDlgPresetItem dialog

class CDlgPresetItem : public CDialog
{
	DECLARE_DYNAMIC(CDlgPresetItem)

public:
	CDlgPresetItem(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgPresetItem();

// Dialog Data
	enum { IDD = IDD_DLG_PRESET_ITEM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	CListCtrl	m_List;

	DECLARE_MESSAGE_MAP()

public:
	void DataShow();
	void DataEdit();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedPresetItemButtonNew();
	afx_msg void OnBnClickedPresetItemButtonEdit();
	afx_msg void OnBnClickedPresetItemButtonDelete();
	afx_msg void OnBnClickedPresetItemButtonClear();
	afx_msg void OnNMDblclkListPresetItem(NMHDR *pNMHDR, LRESULT *pResult);
};
