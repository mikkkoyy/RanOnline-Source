#pragma once

#include "GLCrowData.h"

// CDlgCrowReplace dialog
struct SCROWREPLACE
{
	SNATIVEID sIDFROM;
	SNATIVEID sIDTO;

	SCROWREPLACE ()
		: sIDFROM( false )
		, sIDTO( false )
	{
	};

	bool operator < ( const SCROWREPLACE& sData )
	{			
		if ( sIDFROM.wMainID > sData.sIDFROM.wMainID ) return true;
		else if ( sIDFROM.wMainID == sData.sIDFROM.wMainID  && sIDFROM.wSubID > sData.sIDFROM.wSubID ) return true;
		return  false;
	}

	bool operator == ( const SCROWREPLACE& sData ) 
	{
		if ( sIDFROM.wMainID == sData.sIDFROM.wMainID && sIDFROM.wSubID == sData.sIDFROM.wSubID ) return true;
		return false;
	}
}; 

class CDlgCrowReplace : public CDialog
{
	DECLARE_DYNAMIC(CDlgCrowReplace)

public:
	CDlgCrowReplace(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgCrowReplace();

// Dialog Data
	enum { IDD = IDD_DLG_CROW_REPLACE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl	m_List;

public:
	std::vector<SCROWREPLACE>	m_vecReplace;

public:
	void	ShowData();
	void	ResetInPut();
	void	ShowCrowName( int nINPUT_MID, int nINPUT_SID, int nOUTPUT );

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonCrowFrom();
	afx_msg void OnBnClickedButtonCrowTo();
	afx_msg void OnEnChangeEditCrowMidFrom();
	afx_msg void OnEnChangeEditCrowSidFrom();
	afx_msg void OnEnChangeEditCrowMidTo();
	afx_msg void OnEnChangeEditCrowSidTo();
	afx_msg void OnBnClickedButtonCrowAdd();
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnBnClickedButtonDelete2();
	afx_msg void OnBnClickedOk();
};
