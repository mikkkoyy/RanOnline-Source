#pragma once

class	CVehicleEditTab;
struct GLVEHICLE;

// CVehicleEditPageBasic dialog

class CVehicleEditPageBasic : public CPropertyPage
{
	DECLARE_DYNAMIC(CVehicleEditPageBasic)

public:
	CVehicleEditPageBasic(LOGFONT logfont, GLVEHICLE* pData);
	virtual ~CVehicleEditPageBasic();

// Dialog Data
	enum { IDD = IDD_VEHICLE_PAGE_BASIC };

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
	CSliderCtrl	m_slider_Battery;

public:
	void	DataDefault();
	void	DataShow();
	BOOL	DataSave();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnNMCustomdrawVehicleeditbasicSliderVehiclefull(NMHDR *pNMHDR, LRESULT *pResult);
};
