#pragma once

class CPetEditTab;
struct GLPET;

// CDlgPetEdit dialog

class CDlgPetEdit : public CDialog
{
	DECLARE_DYNAMIC(CDlgPetEdit)

public:
	CDlgPetEdit(CWnd* pParent = NULL);   // standard constructor
	CDlgPetEdit(int nDialogID, DWORD dwPETCHARID, DWORD dwPETID, std::string strNAME, CWnd* pParent = NULL);
	virtual ~CDlgPetEdit();

	BOOL Create(CWnd * pWnd){ 
		return CDialog::Create( CDlgPetEdit::IDD, pWnd );
	}

// Dialog Data
	enum { IDD = IDD_DLG_PET_EDIT };

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
	DWORD		m_dwPetCharID;
	DWORD		m_dwPetID;
	std::string m_strNAME;

public:
	CPetEditTab*	m_pSheetTab;
	GLPET*		m_pData;

public:
	BOOL	DataSave();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedOk2();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
