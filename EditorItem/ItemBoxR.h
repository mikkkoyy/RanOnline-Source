#pragma once

#include "GLItemMan.h"
#include "GLItem.h"
#include <vector>

// CItemBoxR dialog

class CItemBoxR : public CDialog
{
	DECLARE_DYNAMIC(CItemBoxR)

public:
	CItemBoxR(CWnd* pParent = NULL);   // standard constructor
	virtual ~CItemBoxR();

// Dialog Data
	enum { IDD = IDD_DIALOG_ITEMBOXR };

public:
	CListBox m_listSELECT;
	CListBox m_listSELECTED;

	typedef std::vector<ITEM::SRANDOMITEM>	VEC;
	typedef VEC::iterator					VEC_ITER;

	VEC m_vecSELECTED;

	std::vector<SNATIVEID> m_vecData;
	std::vector<SNATIVEID> m_vecResult;

public:
	void	ReadData();
	void	ResetSelection();
	void	ShowSelections();
	void	ShowSelected();
	void	SearchData();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnLbnSelchangeListSelect();
	afx_msg void OnBnClickedButtonInsert();
	afx_msg void OnBnClickedButtonRemove();
	afx_msg void OnBnClickedButtonSearch();
};
