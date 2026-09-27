
// EditorNpcActionDlg.h : header file
//

#pragma once

class	CsheetWithTab;

struct SAnswerTextData 
{
	std::string		   answerTalkText;
	int			   dwNpcNumber;
	int			   dwTextNumber;
	int			   dwAnswerNumber;
};

struct SNpcTextData 
{
	std::string		   TalkText;
	int			   dwNpcNumber;
	int			   dwTextNumber;
};

// CEditorNpcActionDlg dialog
class CEditorNpcActionDlg : public CDialog
{
// Construction
public:
	CEditorNpcActionDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EDITORNPCACTION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	CsheetWithTab*	m_pSheetTab;
	char			m_szAppPath[MAX_PATH];
	CString			m_strAppPath;
	std::string		m_strFileName;

public:
	BOOL	DoNew();
	BOOL	DoLoad();
	BOOL	DoSave ();

	void	SaveAll();
	void	ClassConvert();

	void	DecompileDat();

	void	EnableSaveLoad( BOOL bENABLE );

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:
	virtual void PostNcDestroy();

public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedNtkNew();
	afx_msg void OnBnClickedNtkLoad();
	afx_msg void OnBnClickedNtkSave();
	afx_msg void OnBnClickedButtonNtkSaveall();
	afx_msg void OnBnClickedButtonNtkClasscnv();
	afx_msg void OnBnClickedButtonNtkDecompileDat();
};
