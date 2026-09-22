#pragma once

class	CCharEditTab;
struct SCHARDATA2;

// CCharEditPageItemFood dialog

class CCharEditPageItemFood : public CPropertyPage
{
	DECLARE_DYNAMIC(CCharEditPageItemFood)

public:
	CCharEditPageItemFood(LOGFONT logfont, SCHARDATA2* pData);
	virtual ~CCharEditPageItemFood();

// Dialog Data
	enum { IDD = IDD_CHAREDIT_PAGE_ITEMFOOD };

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

private:
	CListCtrl	m_List;

public:
	void UpdateItemFood();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedChareditItemfoodButtonEdit();
	afx_msg void OnBnClickedChareditItemfoodButtonDelete();
	afx_msg void OnBnClickedChareditItemfoodButtonClear();
	afx_msg void OnBnClickedChareditItemfoodButtonUnlock();
};
