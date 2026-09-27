#pragma once

struct SGMTOOL_PRESET_ITEM;

// CDlgPresetItemEdit dialog

class CDlgPresetItemEdit : public CDialog
{
	DECLARE_DYNAMIC(CDlgPresetItemEdit)

public:
	CDlgPresetItemEdit( CWnd* pParent = NULL, SGMTOOL_PRESET_ITEM* pPreset = NULL );   // standard constructor
	virtual ~CDlgPresetItemEdit();

// Dialog Data
	enum { IDD = IDD_DLG_PRESET_ITEM_EDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	SGMTOOL_PRESET_ITEM*	m_pPreset;

public:
	void	DataShow();
	BOOL	DataSave();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
