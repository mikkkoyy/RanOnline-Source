#pragma once

struct SGMTOOL_SHOP_ITEM;

// CDlgShopItemEdit dialog

class CDlgShopItemEdit : public CDialog
{
	DECLARE_DYNAMIC(CDlgShopItemEdit)

public:
	CDlgShopItemEdit(CWnd* pParent = NULL, SGMTOOL_SHOP_ITEM* pDATA = NULL );
	virtual ~CDlgShopItemEdit();

// Dialog Data
	enum { IDD = IDD_DLG_SHOPITEM_EDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	SGMTOOL_SHOP_ITEM* m_pData;

public:
	void	DataShow();
	BOOL	DataSave();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedShopitemButtonEditItem();
	afx_msg void OnBnClickedShopitemButtonUseDefault();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnEnChangeShopitemEditItemMid();
	afx_msg void OnEnChangeShopitemEditItemSid();
};
