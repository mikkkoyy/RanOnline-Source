#pragma once

#include "GLQuest.h"
// CDlgMobGen dialog

class CDlgMobGen : public CDialog
{
	DECLARE_DYNAMIC(CDlgMobGen)

public:
	CDlgMobGen(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgMobGen();

// Dialog Data
	enum { IDD = IDD_DIALOG_MOBGEN };

public:
	SGENQUESTITEM	m_sGENQITEM;
	CListBox		m_listboxGENMOB;

public:
	void	UpdateDlgItem ();
	void	ShowItem();
	void	ShowCrowList();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnEnChangeEditGenitemMid();
	afx_msg void OnEnChangeEditGenitemSid();
	afx_msg void OnBnClickedButtonGenitem();
	afx_msg void OnBnClickedButtonGenMobAdd();
	afx_msg void OnBnClickedButtonGenMobDel();
	afx_msg void OnBnClickedButtonGenMobInsert();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
