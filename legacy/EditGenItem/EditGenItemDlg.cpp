#include "stdafx.h"
#include "EditGenItem.h"
#include "EditGenItemDlg.h"

#include "StringUtils.h"
#include "../Lib_Helper/EtcFunction.h"
#include "DxSkinAniMan.h"

#include "../../Lib_Engine/G-Logic/GLOGIC.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CEditGenItemDlg dialog



CEditGenItemDlg::CEditGenItemDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditGenItemDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEditGenItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_GENNUM, Sale_List);
	DDX_Control(pDX, IDC_LIST_ITEMSPEC, ItemSpec_List);
	DDX_Control(pDX, IDC_LIST_ITEMID, ItemID_List);
}

BEGIN_MESSAGE_MAP(CEditGenItemDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_GENITEM_LOAD, OnBnClickedButtonCrowsaleLoad)
	ON_BN_CLICKED(IDC_BUTTON_GENITEM_SAVE, OnBnClickedButtonCrowsaleSave)

	ON_BN_CLICKED(IDC_BUTTON_SALELIST_ADD, OnBnClickedButtonSalelistAdd)
	ON_BN_CLICKED(IDC_BUTTON_SALELIST_DELETE, OnBnClickedButtonSalelistDelete)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_GENNUM, OnNMDblclkListSale)
	ON_BN_CLICKED(IDC_BUTTON_SALELIST_OK, OnBnClickedButtonSalelistOk)

	ON_BN_CLICKED(IDC_BUTTON_SPECLIST_ADD, OnBnClickedButtonSpeclistAdd)
	ON_BN_CLICKED(IDC_BUTTON_SPECLIST_DELETE, OnBnClickedButtonSpeclistDelete)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ITEMSPEC, OnNMDblclkListSpec)
	ON_BN_CLICKED(IDC_BUTTON_SPECLIST_OK, OnBnClickedButtonSpeclistOk)

	ON_BN_CLICKED(IDC_BUTTON_ITEMIDLIST_ADD, OnBnClickedButtonItemlistAdd)
	ON_BN_CLICKED(IDC_BUTTON_ITEMIDLIST_DELETE, OnBnClickedButtonItemlistDelete)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ITEMSPEC, OnNMDblclkListItem)
	ON_BN_CLICKED(IDC_BUTTON_ITEMIDLIST_OK, OnBnClickedButtonItemlistOk)

	ON_BN_CLICKED(IDC_BUTTON_UPDATE, OnBnClickedButtonUpdateGenItem)
END_MESSAGE_MAP()


// CEditGenItemDlg message handlers

BOOL CEditGenItemDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	RECT rectCtrl;
	LONG lnWidth;
	LV_COLUMN lvColumn;
	Sale_List.SetExtendedStyle ( Sale_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	Sale_List.GetClientRect ( &rectCtrl );
	lnWidth = rectCtrl.right - rectCtrl.left;
	const int nColumnCount = 2;	
	char* szColumnName1[nColumnCount] = { "Number of Drops", "Rate" };
	int nColumnWidthPercent1[nColumnCount] = { 20,10 };

	for ( int i = 0; i < nColumnCount; i++ )
	{		
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.pszText = szColumnName1[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = ( lnWidth*nColumnWidthPercent1[i] ) / 100;
		Sale_List.InsertColumn (i, &lvColumn );
	}
	RECT rectCtrls;
	LONG lnWidths;
	LV_COLUMN lvColumns;
	ItemSpec_List.SetExtendedStyle ( ItemSpec_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	ItemSpec_List.GetClientRect ( &rectCtrls );
	lnWidths = rectCtrls.right - rectCtrls.left;
	const int nColumnCounts = 2;	
	char* szColumnNames1[nColumnCounts] = { "Gen ID", "Rate" };
	int nColumnWidthPercents1[nColumnCounts] = { 20,10 };

	for ( int i = 0; i < nColumnCounts; i++ )
	{		
		lvColumns.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumns.fmt = LVCFMT_LEFT;
		lvColumns.pszText = szColumnNames1[i];
		lvColumns.iSubItem = i;
		lvColumns.cx = ( lnWidths*nColumnWidthPercents1[i] ) / 100;
		ItemSpec_List.InsertColumn (i, &lvColumns );
	}

	RECT rectCtrlid;
	LONG lnWidthid;
	LV_COLUMN lvColumnid;
	ItemID_List.SetExtendedStyle ( ItemID_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	ItemID_List.GetClientRect ( &rectCtrlid );
	lnWidthid = rectCtrlid.right - rectCtrlid.left;
	const int nColumnCountid = 3;	
	char* szColumnNameid1[nColumnCountid] = { "Main ID", "Sub ID", "Rate" };
	int nColumnWidthPercentid1[nColumnCountid] = { 20,20 ,10};

	for ( int i = 0; i < nColumnCountid; i++ )
	{		
		lvColumnid.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumnid.fmt = LVCFMT_LEFT;
		lvColumnid.pszText = szColumnNameid1[i];
		lvColumnid.iSubItem = i;
		lvColumnid.cx = ( lnWidthid*nColumnWidthPercentid1[i] ) / 100;
		ItemID_List.InsertColumn (i, &lvColumnid );
	}
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEditGenItemDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CEditGenItemDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CEditGenItemDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CEditGenItemDlg::OnBnClickedButtonCrowsaleLoad()
{
	// TODO: Add your control notification handler code here
	CString szFilterInput = "GENITEM|*.genitem|";
	CFileDialog dlgInput(true,"*.*",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput,this);
	dlgInput.m_ofn.lpstrInitialDir = GLOGIC::GetPath();
	if ( dlgInput.DoModal() != IDOK ) return;

	//BOOL bOK = m_pAbfCode.LoadFile( dlgInput.GetFileName() );

	//if ( bOK )
	//{
				
				SetWin_Text ( this, IDC_EDIT1, "");
				Sale_List.DeleteAllItems();
				ItemSpec_List.DeleteAllItems();
				ItemID_List.DeleteAllItems();
				hr = NULL;
				glTextFile.clean();
				StrFile.Reset();
				strTemp="";
				strLine="";
						
		SetWindowText( dlgInput.GetFileName().GetString() );
		UpdateData(dlgInput.GetFileName().GetString());
	//}
}

void CEditGenItemDlg::OnBnClickedButtonCrowsaleSave()
{
	// TODO: Add your control notification handler code here
//	if ( SaveData () )
//	{
		CString szFilter = "GENITEM (*.genitem)|*.genitem|";
		CFileDialog dlg(FALSE,".crowsale",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter,
		(CEditGenItemDlg*)this);

		dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath();
		if ( dlg.DoModal() == IDOK )
		{
			//BOOL bOK = m_pAbfCode.SaveFile ( dlg.GetFileName() );
				SaveData(dlg.GetFileName() );
				//SetWindowText ( dlg.GetFileName() );
				MessageBox ( "Save Success" );
			
		}

//	}
}

BOOL CEditGenItemDlg::UpdateData(const char* szFileName)
{
	//	hr = GLItemMan::GetInstance().LoadFile ( "item.isf", TRUE, TRUE );

	//	if( !GLStringTable::GetInstance().LOADFILE( GLItemMan::GetInstance()._STRINGTABLE, GLStringTable::ITEM ) )
	//	{
	//		MessageBox ( "Failure to load Item String Table." );
	//	}
	std::string strPath;
	strPath = GLOGIC::GetPath();
	strPath += szFileName;
	glTextFile.reg_sep ( '\t' );
	glTextFile.reg_sep ( ' ' );
	glTextFile.reg_sep ( ',' );
	glTextFile.reg_sep ( '{' );
	glTextFile.reg_sep ( '}' );
	glTextFile.reg_sep ( '[' );
	glTextFile.reg_sep ( ']' );
	glTextFile.reg_sep ( '(' );
	glTextFile.reg_sep ( ')' );
	glTextFile.reg_sep ( '|' );

	
	if ( !StrFile.Open ( szFileName, TRUE ) ) // Open and decode string file classes.
		return FALSE;


	while ( StrFile.GetNextLine(strLine) ) // It reads a line from the file one by one.
	{
		strLine += "\r\n";
		strTemp += strLine;
	}

	strTemp.SetAt(strTemp.GetLength()-2, NULL); // Remove the last \ r \ n.

	//SetWin_Text ( this, IDC_EDIT1, strTemp.GetString());

	if ( glTextFile.open ( strPath, true, GLOGIC::bGLOGIC_PACKFILE ) )
	{
		InitSaleList();
		
	}
	else
	return FALSE;

	
	return TRUE;
}

BOOL CEditGenItemDlg::SaveData(const char* lpszPathName)
{
	CString strTemp;
	strTemp = GetWin_Text ( this, IDC_EDIT1 );
	strTemp.Replace(strSaleType,GetWin_Text ( this, IDC_EDIT_SALE_NAME ));
	SetWin_Text ( this, IDC_EDIT_SALE_NAME, "");
	strSaleType.Format("");

	
	CRijndael oRijndael;
	//oRijndael.Initialize( CRijndael::sm_Version[CRijndael::VERSION-1].c_str(), CRijndael::sm_KeyLength[CRijndael::VERSION-1] );
	oRijndael.Initialize( CRijndael::VERSION, CRijndael::sm_Version[CRijndael::VERSION-1].c_str(), CRijndael::sm_KeyLength[CRijndael::VERSION-1] );
	int nLen = oRijndael.GetEncryptStringLength( strTemp );
	char * szBuffer = new char[nLen];

	oRijndael.EncryptEx( strTemp.GetString(), szBuffer, strTemp.GetLength() );
	int nVersion = CRijndael::VERSION;

	CFile file;
	file.Open( lpszPathName, CFile::modeCreate|CFile::modeWrite );	// Open file
	file.Write( &nVersion, sizeof(int) );							// Write Version
	file.Write( szBuffer, nLen );		// Write a file
	file.Close();


	Sale_List.DeleteAllItems();
	return TRUE;
}

void CEditGenItemDlg::InitSaleList()
{
	char szTempChar[256];
	char szTempChars[256];
	
	glTextFile.getflag ( "VERSION", 1, 1, dwVERSION );
	dwNUM = glTextFile.getflagnum ( "GENNUM" );
	glTextFile.getflag ( "GENRATE", 1, 1, m_fGenRate );
	sprintf( szTempChars, "%.2f", m_fGenRate );
	SetWin_Text ( this, IDC_EDIT_GEN_RATE, szTempChars );
	AppendAllTexts.AppendFormat("\r\nGENRATE\t\t%.2f\r\n",m_fGenRate);
		Sale_List.DeleteAllItems();
		
		
		for ( int i = 0; i < dwNUM; i++ )
		{
			glTextFile.getflag ( i, "GENNUM", 1, 2, wNum[i] );
			glTextFile.getflag ( i, "GENNUM", 2, 2, fRate[i] );

			sprintf( szTempChar, "%d", wNum[i] );
			Sale_List.InsertItem( i,  szTempChar );

			sprintf( szTempChar, "%.2f", fRate[i]);
			Sale_List.SetItemText( i, 1, szTempChar );
		

		}
		dwSpecNum = glTextFile.getflagnum ( "ITEMSPEC" );
		for ( DWORD i=0; i<dwSpecNum; ++i )
		{
			glTextFile.getflag ( i, "ITEMSPEC", 1, 2, dwSpecID[i] );
			glTextFile.getflag ( i, "ITEMSPEC", 2, 2, ifRate[i] );

			sprintf( szTempChar, "%d", dwSpecID[i] );
			ItemSpec_List.InsertItem( i, szTempChar );

			sprintf( szTempChar, "%.2f", ifRate[i]);
			ItemSpec_List.SetItemText( i, 1, szTempChar );
		}
		dwIDNUM = glTextFile.getflagnum ( "ITEMID" );
		for ( DWORD i=0; i<dwIDNUM; ++i )
		{
			glTextFile.getflag ( i, "ITEMID", 1, 3, wMainID[i] );
			glTextFile.getflag ( i, "ITEMID", 2, 3, wSubID[i] );
			glTextFile.getflag ( i, "ITEMID", 3, 3, idfRate[i] );

			sprintf( szTempChar, "%d", wMainID[i] );
			ItemID_List.InsertItem( i, szTempChar );

			sprintf( szTempChar, "%d", wSubID[i]);
			ItemID_List.SetItemText( i, 1, szTempChar );

			sprintf( szTempChar, "%.2f", idfRate[i]);
			ItemID_List.SetItemText( i, 2, szTempChar );
			
		}
return;
}

void CEditGenItemDlg::OnBnClickedButtonSalelistAdd()
{ 
	SetWin_Num_int ( this, IDC_EDIT_DROPS, 0);
	SetWin_Num_int ( this, IDC_EDIT_RATE1, 0);
}

void CEditGenItemDlg::OnBnClickedButtonSalelistDelete()
{
	// TODO: Add your control notification handler code here
	int nSelect = Sale_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	Sale_List.DeleteItem(nSelect);
	int itemcounts=Sale_List.GetItemCount();
	CString oneline,tryall;
	wNum[nSelect]=-1;

	//InitSaleList();
}

void CEditGenItemDlg::OnNMDblclkListSale(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int nSelect = Sale_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	SetWin_Num_int ( this, IDC_EDIT_DROPS, wNum[nSelect]);
	SetWin_Num_float ( this, IDC_EDIT_RATE1, fRate[nSelect]);
}

void CEditGenItemDlg::OnBnClickedButtonSalelistOk()
{

	int nSelect = Sale_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	int itemcounts;
	itemcounts=Sale_List.GetItemCount();
	char szTempChar[256];

	m_fGenRate=GetWin_Num_float ( this, IDC_EDIT_GEN_RATE );
	if( nSelect == -1 ) 
	{
		wNum[itemcounts]=GetWin_Num_int ( this, IDC_EDIT_DROPS );
		fRate[itemcounts]=GetWin_Num_float ( this, IDC_EDIT_RATE1 );
		sprintf( szTempChar, "%d", wNum[itemcounts] );
		Sale_List.InsertItem( itemcounts, szTempChar );
		sprintf( szTempChar, "%.2f", fRate[itemcounts]);
		Sale_List.SetItemText( itemcounts, 1, szTempChar );
	}
	else
	{
		wNum[nSelect]=GetWin_Num_int ( this, IDC_EDIT_DROPS );
		fRate[nSelect]=GetWin_Num_float ( this, IDC_EDIT_RATE1 );
		Sale_List.DeleteItem(nSelect);
		sprintf( szTempChar, "%d", wNum[nSelect] );
		Sale_List.InsertItem( nSelect, szTempChar );
		sprintf( szTempChar, "%.2f", fRate[nSelect]);
		Sale_List.SetItemText( nSelect, 1, szTempChar );
	}
		/*	CString oneline,tryall;
			for(int i =0;i<itemcounts;i++)
			{
				if(wNum[i]==-1)
				{
					i++;
					continue;
				}
			oneline.Format("GENRATE		%.2f",m_fGenRate);
			oneline.Format("\r\nGENNUM		%d		%.2f",wNum[i],fRate[i]);
			oneline.AppendFormat("\r\nITEMSPEC		%d		%.2f",dwSpecID[i],ifRate[i]);
			oneline.AppendFormat("\r\nITEMID		%d	%d	%.2f",wMainID[i],wSubID[i],ifRate[i]);
			AppendAllTexts.AppendFormat("\r\n%s",oneline);
			SetWin_Text ( this, IDC_EDIT1, AppendAllTexts);
			}
*/
}



//////////////////////////////////////////////////
void CEditGenItemDlg::OnBnClickedButtonSpeclistAdd()
{ 
	SetWin_Num_int ( this, IDC_EDIT_GENID, 0);
	SetWin_Num_int ( this, IDC_EDIT_RATE2, 0);
}

void CEditGenItemDlg::OnBnClickedButtonSpeclistDelete()
{
	// TODO: Add your control notification handler code here
	int nSelect = ItemSpec_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	ItemSpec_List.DeleteItem(nSelect);
	int itemcounts=ItemSpec_List.GetItemCount();
	CString oneline,tryall;
	dwSpecID[nSelect]=-1;

	//InitSaleList();
}

void CEditGenItemDlg::OnNMDblclkListSpec(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int nSelect = ItemSpec_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	SetWin_Num_int ( this, IDC_EDIT_GENID, dwSpecID[nSelect]);
	SetWin_Num_float ( this, IDC_EDIT_RATE2, ifRate[nSelect]);
}

void CEditGenItemDlg::OnBnClickedButtonSpeclistOk()
{

	int nSelect = ItemSpec_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	int itemcounts;
	itemcounts=ItemSpec_List.GetItemCount();
	char szTempChar[256];
	m_fGenRate=GetWin_Num_float ( this, IDC_EDIT_GEN_RATE ); 
	if( nSelect == -1 ) 
	{
		dwSpecID[itemcounts]=GetWin_Num_int ( this, IDC_EDIT_GENID );
		ifRate[itemcounts]=GetWin_Num_float ( this, IDC_EDIT_RATE2 );
		sprintf( szTempChar, "%d", dwSpecID[itemcounts] );
		ItemSpec_List.InsertItem( itemcounts, szTempChar );
		sprintf( szTempChar, "%.2f", ifRate[itemcounts]);
		ItemSpec_List.SetItemText( itemcounts, 1, szTempChar );
	}
	else
	{
		dwSpecID[nSelect]=GetWin_Num_int ( this, IDC_EDIT_GENID );
		ifRate[nSelect]=GetWin_Num_float ( this, IDC_EDIT_RATE2 );
		ItemSpec_List.DeleteItem(nSelect);
		sprintf( szTempChar, "%d", dwSpecID[nSelect] );
		ItemSpec_List.InsertItem( nSelect, szTempChar );
		sprintf( szTempChar, "%.2f", ifRate[nSelect]);
		ItemSpec_List.SetItemText( nSelect, 1, szTempChar );
	}
		/*	CString oneline,tryall;
			for(int i =0;i<itemcounts;i++)
			{
				if(wNum[i]==-1)
				{
					i++;
					continue;
				}

			oneline.Format("GENRATE		%.2f",m_fGenRate);
			oneline.Format("\r\nGENNUM		%d		%.2f",wNum[i],fRate[i]);
			oneline.AppendFormat("\r\nITEMSPEC		%d		%.2f",dwSpecID[i],ifRate[i]);
			oneline.AppendFormat("\r\nITEMID		%d	%d	%.2f",wMainID[i],wSubID[i],ifRate[i]);
			AppendAllTexts.AppendFormat("\r\n%s",oneline);
			
			}
*/
}
////////////////////////////////////////////////////
void CEditGenItemDlg::OnBnClickedButtonItemlistAdd()
{ 
	SetWin_Num_int ( this, IDC_EDIT_MID, 0);
	SetWin_Num_int ( this, IDC_EDIT_SID, 0);
	SetWin_Num_int ( this, IDC_EDIT_RATE3, 0);
}

void CEditGenItemDlg::OnBnClickedButtonItemlistDelete()
{
	// TODO: Add your control notification handler code here
	int nSelect = ItemID_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	ItemID_List.DeleteItem(nSelect);
	int itemcounts=ItemID_List.GetItemCount();
	CString oneline,tryall;
	wMainID[nSelect]=-1;

	//InitSaleList();
}

void CEditGenItemDlg::OnNMDblclkListItem(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int nSelect = ItemID_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	SetWin_Num_int ( this, IDC_EDIT_MID, wMainID[nSelect]);
	SetWin_Num_int ( this, IDC_EDIT_SID, wSubID[nSelect]);
	SetWin_Num_float ( this, IDC_EDIT_RATE3, idfRate[nSelect]);
}

void CEditGenItemDlg::OnBnClickedButtonItemlistOk()
{

	int nSelect = ItemID_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	int itemcounts;
	itemcounts=ItemID_List.GetItemCount();
	char szTempChar[256];
	m_fGenRate=GetWin_Num_float ( this, IDC_EDIT_GEN_RATE ); 
	if( nSelect == -1 ) 
	{
		wMainID[itemcounts]=GetWin_Num_int ( this, IDC_EDIT_MID );
		wSubID[itemcounts]=GetWin_Num_int ( this, IDC_EDIT_SID );
		idfRate[itemcounts]=GetWin_Num_float ( this, IDC_EDIT_RATE3 );
		sprintf( szTempChar, "%d", wMainID[itemcounts] );
		ItemID_List.InsertItem( itemcounts, szTempChar );
		sprintf( szTempChar, "%d", wSubID[itemcounts]);
		ItemID_List.SetItemText( itemcounts, 1, szTempChar );
		sprintf( szTempChar, "%.2f", idfRate[itemcounts]);
		ItemID_List.SetItemText( itemcounts, 2, szTempChar );
	}
	else
	{
		wMainID[nSelect]=GetWin_Num_int ( this, IDC_EDIT_MID );
		wSubID[nSelect]=GetWin_Num_int ( this, IDC_EDIT_SID );
		idfRate[nSelect]=GetWin_Num_float ( this, IDC_EDIT_RATE3 );
		sprintf( szTempChar, "%d", wMainID[nSelect] );
		ItemID_List.InsertItem( nSelect, szTempChar );
		sprintf( szTempChar, "%d", wSubID[nSelect]);
		ItemID_List.SetItemText( nSelect, 1, szTempChar );
		sprintf( szTempChar, "%.2f", idfRate[nSelect]);
		ItemID_List.SetItemText( nSelect, 2, szTempChar );
	}
			

}

void CEditGenItemDlg::OnBnClickedButtonUpdateGenItem()
{
	SetWin_Text ( this, IDC_EDIT1, "");
	AppendAllTexts = "";
	int specitemcounts=ItemSpec_List.GetItemCount();
	int itemidcounts=ItemID_List.GetItemCount();
	int gennumcounts=Sale_List.GetItemCount();
		CString oneline,tryall;
		AppendAllTexts.Format("/////////////////////////////////////////////////////\r\n//Generated GENITEM By EditGenitem (c)NRDEV\r\t\t\r\n//////////////////////////////////////////////////////");
		m_fGenRate=GetWin_Num_float ( this, IDC_EDIT_GEN_RATE );
		oneline.Format("GENRATE		%.2f",m_fGenRate);
			for(int i =0;i<gennumcounts;i++)
			{
				if(wNum[i]==-1)
				{
					i++;
					continue;
				}
				oneline.AppendFormat("\r\nGENNUM		%d		%.2f",wNum[i],fRate[i]);
				
			}
			for(int i =0;i<specitemcounts;i++)
			{
				if(dwSpecID[i]==-1)
				{
					i++;
					continue;
				}
				oneline.AppendFormat("\r\nITEMSPEC		%d		%.2f",dwSpecID[i],ifRate[i]);
			}
			for(int i =0;i<itemidcounts;i++)
			{
				if(wMainID[i]==-1)
				{
					i++;
					continue;
				}
				oneline.AppendFormat("\r\nITEMID		%d	%d	%.2f",wMainID[i],wSubID[i],ifRate[i]);
			}
			AppendAllTexts.AppendFormat("\r\n%s",oneline);

			SetWin_Text ( this, IDC_EDIT1, AppendAllTexts);
}