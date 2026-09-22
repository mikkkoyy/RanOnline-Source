#pragma once


class CsheetWithTab;
// CEditorBar dialog

class CEditorBar : public CDialogBar
{
	DECLARE_DYNAMIC(CEditorBar)

public:
	CEditorBar();   // standard constructor
	virtual ~CEditorBar();

// Dialog Data
	enum { IDD = IDD_DLG_MAIN };

public:
	CsheetWithTab	*m_pSheetTab;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg LONG OnInitDialog ( UINT, LONG );
	DECLARE_MESSAGE_MAP()


};
