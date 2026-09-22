#pragma once

class	CCharEditTab;
class	CLockerTab;
struct SCHARDATA2;

// CCharEditPageLocker dialog

class CCharEditPageLocker : public CPropertyPage
{
	DECLARE_DYNAMIC(CCharEditPageLocker)

public:
	CCharEditPageLocker(LOGFONT logfont, SCHARDATA2* pData);
	virtual ~CCharEditPageLocker();

// Dialog Data
	enum { IDD = IDD_CHAREDIT_PAGE_LOCKER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CCharEditTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;
	SCHARDATA2*		m_pData;

private:
	CLockerTab*		m_pLockerTab;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CCharEditTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

public:
	virtual BOOL OnInitDialog();

};
