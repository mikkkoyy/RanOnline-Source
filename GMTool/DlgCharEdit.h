#pragma once

class CCharEditTab;
struct SCHARDATA2;

// CDlgCharEdit dialog

class CDlgCharEdit : public CDialog
{
	DECLARE_DYNAMIC(CDlgCharEdit)

public:
	CDlgCharEdit(CWnd* pParent = NULL);   // standard constructor
	CDlgCharEdit(int nDialogID, DWORD dwUserID, DWORD dwCharID, std::string strNAME, CWnd* pParent = NULL);
	virtual ~CDlgCharEdit();

	BOOL Create(CWnd * pWnd){ 
		return CDialog::Create( CDlgCharEdit::IDD, pWnd );
	}

// Dialog Data
	enum { IDD = IDD_DLG_CHAR_EDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

public:
	CWnd*		m_pParentWnd;

private:
	int			m_nDialogID;
	DWORD		m_dwUserID;
	DWORD		m_dwCharID;
	std::string m_strNAME;

public:
	CCharEditTab*	m_pSheetTab;
	SCHARDATA2*		m_pData;

public:
	BOOL	DataSave();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk2();
};
