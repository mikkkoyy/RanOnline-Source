
// FileCryptDlg.h : header file
//

#include "CFileFind.h"


#pragma once

enum EMCRYPT_FILETYPE
{
	EMCRYPT_FILETYPE_TEXTFILE		= 0, //text file 
	EMCRYPT_FILETYPE_STREAMFILE		= 1, //stream file
	EMCRYPT_FILETYPE_STREAMFILE_R2	= 2, //stream file ran2
	EMCRYPT_FILETYPE_XMLCRYPT		= 3, //xml gui file
	EMCRYPT_FILETYPE_TEXTURECRYPT	= 4, //texture files
	EMCRYPT_FILETYPE_XFILE_CRYPT	= 5, //x files
	EMCRYPT_FILETYPE_ANIMFILE		= 6, //animation bin
	EMCRYPT_FILETYPE_EGPCRYPT		= 7, //egp files
	EMCRYPT_FILETYPE_SIZE			= 8,
};

enum EMCRYPT_BYPASSHEADER
{
	EMCRYPT_BYPASSHEADER_0	= 0, //no bypass
	EMCRYPT_BYPASSHEADER_1	= 1, //bypass 128 bytes ( FILETYPE )
	EMCRYPT_BYPASSHEADER_2	= 2, //bypass 132 bytes ( FILETYPE + HEADER )
	EMCRYPT_BYPASSHEADER_3	= 3, //bypass 136 bytes ( FILETYPE + HEADER + VERSION )
	EMCRYPT_BYPASSHEADER_SIZE = 4,
};

// CFileCryptDlg dialog
class CFileCryptDlg : public CDialog
{
// Construction
public:
	CFileCryptDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FILECRYPT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	CEdit			m_cEdit;
	CFileFindTree	m_cTree;
	std::string		m_strInputFolder;
	std::string		m_strOutputFolder;

public:
	void InfoAdd( const char *szFormat, ... );
	void InfoReset();
	void SettingChange();

public:
	void DoCrypt( BOOL bENCRYPT );
	void TextEncrypt();
	void TextDecrypt();
	void StreamEncrypt();
	void StreamDecrypt();
	void StreamDecryptR2();

	void XMLEncrypt();
	void XMLDecrypt();

	void TextureEncrypt();
	void TextureDecrypt();

	void XFileEncrypt();
	void XFileDecrypt();

	void AniFileEncrypt();

	void EGPFileEncrypt();

public:
	afx_msg void OnCbnSelchangeCmbFiletype();
	afx_msg void OnBnClickedBtnInput();
	afx_msg void OnBnClickedBtnOutput();
	afx_msg void OnBnClickedBtnInplace();
	afx_msg void OnBnClickedBtnEncrypt();
	afx_msg void OnBnClickedBtnDecrypt();
};
