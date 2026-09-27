#pragma once


// CDlgDatetime dialog

class CDlgDatetime : public CDialog
{
	DECLARE_DYNAMIC(CDlgDatetime)

public:
	CDlgDatetime(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgDatetime();

// Dialog Data
	enum { IDD = IDD_DLG_DATETIME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

private:
	CTime	m_cDateMIN;
	CTime	m_cDateMAX;
	CTime	m_ccTIME;
	CMonthCalCtrl	m_cDate;
	CDateTimeCtrl	m_cTime;

public:
	void	DateSetMin( __time64_t tTime ){	m_cDateMIN = tTime;	}
	void	DateSetMax( __time64_t tTime ){	m_cDateMAX = tTime;	}
	void	DateTimeSet( __time64_t tTime ){	m_ccTIME = tTime;	}
	__time64_t	DateTimeGet()	{ return m_ccTIME.GetTime();	}

public:
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonCurtime();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
