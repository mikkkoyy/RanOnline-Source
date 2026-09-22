#pragma once

#include <map>
class CVehicleEditTab;
struct GLVEHICLE;

// CDlgVehicleEdit dialog

class CDlgVehicleEdit : public CDialog
{
	DECLARE_DYNAMIC(CDlgVehicleEdit)

public:
	CDlgVehicleEdit(CWnd* pParent = NULL);   // standard constructor
	CDlgVehicleEdit(int nDialogID, DWORD dwVEHICLECHARID, DWORD dwVEHICLEID, std::string strNAME, CWnd* pParent = NULL);
	virtual ~CDlgVehicleEdit();

	BOOL Create(CWnd * pWnd){ 
		return CDialog::Create( CDlgVehicleEdit::IDD, pWnd );
	}

// Dialog Data
	enum { IDD = IDD_DLG_VEHICLE_EDIT };

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
	DWORD		m_dwVehicleCharID;
	DWORD		m_dwVehicleID;
	std::string m_strNAME;

public:
	CVehicleEditTab*	m_pSheetTab;
	GLVEHICLE*		m_pData;

public:
	BOOL	DataSave();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedOk2();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
