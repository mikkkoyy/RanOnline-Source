#pragma once

#include "Logic/GMToolData.h"

class	CsheetWithTab;
// CPageItemShop dialog

class CPageItemShop : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageItemShop)

public:
	CPageItemShop(LOGFONT logfont);
	virtual ~CPageItemShop();

// Dialog Data
	enum { IDD = IDD_MAINPAGE_ITEMSHOP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

public:
	CListCtrl	m_List;
	SGMTOOL_SHOP_ITEM_VEC m_vecResult;

public:
	void ShowAllItems();
	void ShowResults();
	void EditItem();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedItemshopPageButtonSearch();
	afx_msg void OnBnClickedItemshopPageButtonAdd();
	afx_msg void OnBnClickedItemshopPageButtonEdit();
	afx_msg void OnBnClickedItemshopPageButtonDelete();
	afx_msg void OnNMDblclkListItemshop(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButton1();
};
