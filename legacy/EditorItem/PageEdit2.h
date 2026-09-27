#pragma once


// CPageEdit2 dialog
class	CsheetWithTab;
struct	SITEM;

class CPageEdit2 : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEdit2)

public:
	CPageEdit2(LOGFONT logfont);
	virtual ~CPageEdit2();

// Dialog Data
	enum { IDD = IDD_EDIT2 };

private:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

public:
	SITEM*	m_pDummyItem;
	SITEM*	m_pItem;
	CListBox  m_listBOX;
	CListBox  m_listBOXR;
	CListCtrl m_listPetSkin;
	CListCtrl m_listRVCardSet; /*rv card, Juver, 2017/11/25 */

public:
	BOOL	ItemSet ( SITEM* pItem );
	void	ItemClear();

	BOOL	DataDefault();
	BOOL	DataShow();
	BOOL	DataSave();

	void	InitBlow();
	void	InitQuestion();
	void	InitDrug();

	void	InitBox();
	void	InitBoxR();
	void	InitPetSkin();

	/*rv card, Juver, 2017/11/25 */
	void	InitRVCard();

	/*item wrapper, Juver, 2018/01/11 */
	void	CheckWrapperBox();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonPageSave();
	afx_msg void OnBnClickedButtonPageCancel();
	afx_msg void OnBnClickedButtonPageBack();
	afx_msg void OnCbnSelchangeComboBlowType();
	afx_msg void OnCbnSelchangeComboQuestionType();
	afx_msg void OnCbnSelchangeComboDrugType();
	afx_msg void OnBnClickedButtonBoxUp();
	afx_msg void OnBnClickedButtonBoxDown();
	afx_msg void OnBnClickedButtonBoxDel();
	afx_msg void OnBnClickedButtonBoxInsert();
	afx_msg void OnBnClickedButtonBoxAdd();
	afx_msg void OnBnClickedButtonBoxAdd2();
	afx_msg void OnBnClickedButtonBoxDel2();
	afx_msg void OnBnClickedButtonBoxUp2();
	afx_msg void OnBnClickedButtonBoxDown2();
	afx_msg void OnBnClickedButtonBoxInsert2();
	afx_msg void OnBnClickedButtonPsInsert();
	afx_msg void OnBnClickedButtonPsAdd();
	afx_msg void OnBnClickedButtonPsDel();
	afx_msg void OnBnClickedButtonRvcardSet();
	afx_msg void OnBnClickedButtonRvcardDel();
	afx_msg void OnBnClickedButtonWrapperBox();
	afx_msg void OnEnChangeEditWrapperBoxMid();
	afx_msg void OnEnChangeEditWrapperBoxSid();
};
