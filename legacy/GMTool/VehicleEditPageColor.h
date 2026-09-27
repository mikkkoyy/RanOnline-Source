#pragma once

class	CVehicleEditTab;
struct GLVEHICLE;

// CVehicleEditPageColor dialog

class CVehicleEditPageColor : public CPropertyPage
{
	DECLARE_DYNAMIC(CVehicleEditPageColor)

public:
	CVehicleEditPageColor(LOGFONT logfont, GLVEHICLE* pData);
	virtual ~CVehicleEditPageColor();

// Dialog Data
	enum { IDD = IDD_VEHICLE_PAGE_COLOR };

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
	void EditColor();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedVehicleeditColorButtonEdit();
	afx_msg void OnBnClickedVehicleeditColorButtonDelete();
	afx_msg void OnBnClickedVehicleeditColorButtonClear();
	afx_msg void OnNMDblclkListVehicleColor(NMHDR *pNMHDR, LRESULT *pResult);
};
