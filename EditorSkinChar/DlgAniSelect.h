#pragma once


// CDlgAniSelect dialog
#include "DxSkinChar.h"

class CDlgAniSelect : public CDialog
{
	DECLARE_DYNAMIC(CDlgAniSelect)

public:
	CDlgAniSelect(CWnd* pParent = NULL, DxSkinChar* pSkin = NULL );   // standard constructor
	virtual ~CDlgAniSelect();

// Dialog Data
	enum { IDD = IDD_DLG_ANI_SELECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CListCtrl		m_List_Anim;

public:
	DxSkinChar*		m_pSkin;
	std::string		m_strAni;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
