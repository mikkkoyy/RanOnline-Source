#pragma once

class	CVehicleEditTab;
struct GLVEHICLE;

// CVehicleEditPageInven dialog

class CVehicleEditPageInven : public CPropertyPage
{
	DECLARE_DYNAMIC(CVehicleEditPageInven)

public:
	CVehicleEditPageInven(LOGFONT logfont, GLVEHICLE* pData);
	virtual ~CVehicleEditPageInven();

// Dialog Data
	enum { IDD = IDD_VEHICLE_PAGE_INVEN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CVehicleEditTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;
	GLVEHICLE*		m_pData;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CVehicleEditTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

public:
	CListCtrl	m_List;

public:
	void ShowData();
	void EditItem();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedVehicleeditinvenButtonEdit();
	afx_msg void OnBnClickedVehicleeditinvenButtonDelete();
	afx_msg void OnBnClickedVehicleeditinvenButtonClear();
	afx_msg void OnNMDblclkListVehicleinven(NMHDR *pNMHDR, LRESULT *pResult);
};
