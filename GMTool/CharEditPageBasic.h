#pragma once

class	CCharEditTab;
struct SCHARDATA2;

// CCharEditPageBasic dialog

class CCharEditPageBasic : public CPropertyPage
{
	DECLARE_DYNAMIC(CCharEditPageBasic)

public:
	CCharEditPageBasic(LOGFONT logfont, SCHARDATA2* pData);
	virtual ~CCharEditPageBasic();

// Dialog Data
	enum { IDD = IDD_CHAREDIT_PAGE_BASIC };

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

public:
	void	DataDefault();
	void	DataShow();
	BOOL	DataSave();

public:
	BOOL	NameCheck( SCHARDATA2* pData, std::string strNewName );
	
public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedChareditBasicButtonChanameCheck();
	afx_msg void OnBnClickedChareditBasicButtonMapstartSet();
	afx_msg void OnBnClickedChareditBasicButtonMapsaveSet();
	afx_msg void OnBnClickedChareditBasicButtonMapcallSet();
	afx_msg void OnBnClickedChareditBasicButtonMapstartReset();
	afx_msg void OnBnClickedChareditBasicButtonMapsaveReset();
	afx_msg void OnBnClickedChareditBasicButtonMapcallReset();
	afx_msg void OnEnChangeChareditBasicEditMapstartMid();
	afx_msg void OnEnChangeChareditBasicEditMapstartSid();
	afx_msg void OnEnChangeChareditBasicEditMapsaveMid();
	afx_msg void OnEnChangeChareditBasicEditMapsaveSid();
	afx_msg void OnEnChangeChareditBasicEditMapcallMid();
	afx_msg void OnEnChangeChareditBasicEditMapcallSid();
	afx_msg void OnBnClickedChareditBasicButtonHaircolor();
};
