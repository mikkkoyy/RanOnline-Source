#pragma once

#include "GLLandMan.h"
#include "GLMobSchedule.h"

// CDlgInsertLeader dialog

class CDlgInsertLeader : public CDialog
{
	DECLARE_DYNAMIC(CDlgInsertLeader)

public:
	CDlgInsertLeader(CWnd* pParent = NULL, GLLandMan* pLand = NULL, GLMobSchedule* pSCHED = NULL );   // standard constructor
	virtual ~CDlgInsertLeader();

// Dialog Data
	enum { IDD = IDD_DLG_SET_LEADER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl	m_List;

protected:
	GLLandMan*		m_pGLLand;
	GLMobSchedule*	m_pSched;

public:
	void	ShowData();
	BOOL	InsertLeader();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
