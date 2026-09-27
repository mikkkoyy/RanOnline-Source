#pragma once


// CDlgEquipmentLockAdd dialog

class CDlgEquipmentLockAdd : public CDialog
{
	DECLARE_DYNAMIC(CDlgEquipmentLockAdd)

public:
	CDlgEquipmentLockAdd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgEquipmentLockAdd();

// Dialog Data
	enum { IDD = IDD_DLG_EQUIPMENT_LOCK_ADD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk2();
	afx_msg void OnBnClickedCancel();
};
