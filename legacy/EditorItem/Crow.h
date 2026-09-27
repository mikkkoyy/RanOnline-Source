#pragma once

#include "GLCrowData.h"
#include "GLItemMan.h"
#include "GLItem.h"
// CCrow dialog

class CCrow : public CDialog
{
	DECLARE_DYNAMIC(CCrow)

public:
	CCrow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCrow();

// Dialog Data
	enum { IDD = IDD_DIALOG_MOB };

public:
	CListBox m_listSELECT;
	CListBox m_listSELECTED;

	typedef std::vector<ITEM::SPETSKINPACKITEMDATA>	VEC;
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
