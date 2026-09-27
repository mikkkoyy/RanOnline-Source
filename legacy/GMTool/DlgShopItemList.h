#pragma once

#include "Logic/GMToolData.h"

// CDlgShopItemList dialog

class CDlgShopItemList : public CDialog
{
	DECLARE_DYNAMIC(CDlgShopItemList)

public:
	CDlgShopItemList(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgShopItemList();

// Dialog Data
	enum { IDD = IDD_DLG_SHOP_ITEMLIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CListBox	m_listSELECT;
	DWORD		m_dwSELECTED;

	SGMTOOL_SHOP_ITEM_VEC m_vecData;
	SGMTOOL_SHOP_ITEM_VEC m_vecResult;

public:
	void	ReadData();
	void	ShowData();
	void	SelectionReset();
	void	SearchData();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonSearch();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnLbnSelchangeListSelect();
};
