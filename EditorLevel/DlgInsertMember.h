#pragma once

#include "GLLandMan.h"
#include "GLMobSchedule.h"

// CDlgInsertMember dialog

class CDlgInsertMember : public CDialog
{
	DECLARE_DYNAMIC(CDlgInsertMember)

public:
	CDlgInsertMember(CWnd* pParent = NULL, GLLandMan* pLand = NULL, GLMobSchedule* pSCHED = NULL);   // standard constructor
	virtual ~CDlgInsertMember();

// Dialog Data
	enum { IDD = IDD_DLG_SET_MEMBER };

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
