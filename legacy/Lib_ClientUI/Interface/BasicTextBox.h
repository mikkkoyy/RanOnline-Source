//	BasicTextBox Ŭ����
//
//	���� �ۼ��� : ���⿱
//	���� ������ :
//	�α�
//		[2004.02.02]
//			@ �ʿ� ���� ��� ����
//			@ �ǹ̺� �⺻ ��� ��ȭ
//			@ �ؽ�Ʈ�ڽ� ��ü���� ��� ���� ����
//		[2003.11.20]
//			@ ���� ���� �߰�
//		[2003.11.19]
//			@ ��������

#pragma	once

#include "UIGroup.h"
#include "UITextUtil.h"
#include <deque>

/*item link, Juver, 2017/07/31 */
#include "../Lib_Client/G-Logic/GLItemLink.h"

//	���� ����
const int TEXT_ALIGN_LEFT		= 0x01;
const int TEXT_ALIGN_CENTER_X	= 0x02;
const int TEXT_ALIGN_RIGHT		= 0x04;
const int TEXT_ALIGN_TOP		= 0x08;
const int TEXT_ALIGN_CENTER_Y	= 0x10;
const int TEXT_ALIGN_BOTTOM		= 0x20;
const int TEXT_ALIGN_BOTH_X		= 0x40;

class	CD3DFontPar;
class	CBasicTextBox : public CUIGroup
{
private:
	enum
	{
		MULTITEXTSPLIT_LENGTH = 75,
		BASE_IMAGE_CONTROLID = 10000,
		BASE_IMAGE_CONTROLID_END = BASE_IMAGE_CONTROLID + 1000,
	};

public:
static	const	float	fDEFAULT_LINE_INTERVAL;
static	const	float	fDEFAULT_PART_INTERVAL;

protected:
static	const	int		nDEFAULT_STARTLINE;
static	const	int		nDEFAULT_VISIBLE_LINE;
static	const	int		nDEFAULT_LINE_LIMIT;

static	const	UIGUID	nID_IMAGELIST;

protected:
	SIZE	BLANKSIZE;

public:
	struct MULTITEXT
	{
		CString		strText[100];
		D3DCOLOR	dwColor[100];
		MULTITEXT()
		{
		}
	};

	struct	STEXTMULTIWORD
	{
		CString		strWord;
		D3DCOLOR	dwColor;

		STEXTMULTIWORD()
			: dwColor ( NS_UITEXTUTIL::ENABLE )			
		{
		}
	};

	typedef	std::vector<STEXTMULTIWORD>	STEXTMULTIWORDVEC;

	struct	STEXTWORD	//	�ܾ� ( ���� ���� ���� )
	{
		float		x;			//	�Ѹ� ��ġ
		float		y;		
		CString		strWord;	//	�Ѹ� ����
		D3DCOLOR	dwColor;	//	����
		SIZE		strSize;	//	������

		STEXTWORD()
			: x( 0.0f )
			, y( 0.0f )
			, dwColor ( UINT_MAX )			
		{
			strSize.cx = 0;
			strSize.cy = 0;
		}
	};
	typedef	std::vector<STEXTWORD>	VECTORTEXTWORD;

	struct	STEXTLINE	//	�� �� ( �ǹ� ����, ���� �� ������ �ڸ��� ���� )
	{
		VECTORTEXTWORD	vectorWord;	
		SIZE			strSize;		//	������
		bool			bLASTLINE;

		STEXTLINE()
			: bLASTLINE( false )
		{
			strSize.cx = 0;
			strSize.cy = 0;
		}
	};
	typedef	std::deque<STEXTLINE>	DEQUETEXTLINE;
	typedef	DEQUETEXTLINE::iterator	DEQUETEXTLINE_ITER;

	struct	STEXTPART	//	AddText, SetText�� �� �� ȣ��� ����
	{
		DEQUETEXTLINE	dequeLine;
		
		BOOL			m_bUseTextColor;
		D3DCOLOR		m_dwColor;
		DWORD_PTR		m_dwData;		//	����Ÿ
		DWORD_PTR		m_dwDataEx;		//	Ȯ�� ����Ÿ		
		CUIControl*		m_pImage;
		UIRECT			m_rcPart;
		SITEMLINK		m_sItemLink; /*item link, Juver, 2017/07/31 */

		STEXTPART()
			: m_bUseTextColor( FALSE )
			, m_dwColor( UINT_MAX )
			, m_dwData( UINT_MAX )
			, m_dwDataEx( UINT_MAX )
			, m_pImage( NULL )
		{
			m_sItemLink = SITEMLINK(); /*item link, Juver, 2017/07/31 */
		}
	};

public:
	typedef std::deque<STEXTPART>		STEXTBOX;
	typedef STEXTBOX::iterator			STEXTBOX_ITER;	
public:
	CBasicTextBox ();
	virtual	~CBasicTextBox ();

public:
	virtual HRESULT Render ( LPDIRECT3DDEVICEQ pd3dDevice );
	virtual	HRESULT InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );
	virtual	HRESULT RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );

private:
	HRESULT	RenderText ();
	HRESULT RenderPart ( DEQUETEXTLINE& dequeTextLine );
	HRESULT RenderLine ( VECTORTEXTWORD& vectorTextWord );
	HRESULT	RenderWord ( STEXTWORD& sTextWord );

public:
	virtual void SetGlobalPos ( const UIRECT& rcPos );
	virtual void SetGlobalPos ( const D3DXVECTOR2& vPos );
	virtual	void AlignSubControl ( const UIRECT& rcParentOldPos, const UIRECT& rcParentNewPos );

public:	
	void SetFont ( CD3DFontPar* pFont );
	void SetTextAlign ( int nAlign );
	void SetLineInterval ( const float& fLineInterval );
	void SetPartInterval ( const float& fPartInterval );
	CD3DFontPar* GetFont()	{ return m_pFont; }

public:
	const STEXTPART& GetTextPart ( const int& nIndex );

private:
	void ReCalcRenderPos ();

public:
	virtual	void ClearText ();

public:	//	����
	void SetOneLineText ( const CString& strText, const D3DCOLOR& TextColor = NS_UITEXTUTIL::ENABLE );

public:
	int AddTextNoSplit ( const CString& strText, const D3DCOLOR& TextColor = NS_UITEXTUTIL::ENABLE );
	int SetTextNoSplit ( const CString& strText, const D3DCOLOR& TextColor = NS_UITEXTUTIL::ENABLE );
	int AddMultiTextNoSplit ( STEXTMULTIWORDVEC &vecMultiWord );
	int AddMultiText ( MULTITEXT strText ); //Add MultiText by NaJ
	int	AddMultiTextLongestLine ( MULTITEXT strText ); //Add MultiTextLongestLine by NaJ

public:	
	void SetText ( const CString& strText, const D3DCOLOR& TextColor = NS_UITEXTUTIL::ENABLE );
	int	AddText ( const CString& strText, const D3DCOLOR& TextColor = NS_UITEXTUTIL::ENABLE );
	int AddTextNoSplitBig ( const CString& strText, const D3DCOLOR& TextColor );
	int	AddOneLineText ( const CString& strText, bool bUseEllipsis = TRUE, const D3DCOLOR& TextColor = NS_UITEXTUTIL::ENABLE );
	int AddTextWithCRLF ( const CString& strText, const D3DCOLOR& TextColor = NS_UITEXTUTIL::ENABLE );
	int	AddString ( int nIndex, const CString& strText, const D3DCOLOR& TextColor = NS_UITEXTUTIL::ENABLE );
	int	AddStringNoSplit ( int nIndex, const CString& strText, const D3DCOLOR& TextColor = NS_UITEXTUTIL::ENABLE );

	/*item link, Juver, 2017/07/31 */
	int	AddText ( const CString& strText, SITEMLINK* pItemLink, const D3DCOLOR& TextColor = NS_UITEXTUTIL::ENABLE );

public:
	void PopFrontText()								{ m_TextBox.pop_front (); }
	void PopFrontTextLine( int nIndex );

public:
	void SetOverColor( int nIndex, D3DCOLOR TextColor );
	D3DCOLOR GetOverColor( int nIndex )				{ return m_dwOverColor; }

	void SetUseOverColor( BOOL bUseOverColor )		{ m_bUseOverColor = bUseOverColor; }
	BOOL IsUseOverColor()							{ return m_bUseOverColor; }

	void SetTextImage( int nIndex, const CString& strKeyword, int nXpos = 0 );
	CUIControl*	GetTextImage( int nIndex )			{ return NULL; }

public:
	void SetTextColor ( int nIndex, D3DCOLOR dwTextColor );
	D3DCOLOR GetTextColor ( int nIndex );

	void SetUseTextColor ( int nIndex, BOOL bUseTextColor );
	BOOL IsUseTextColor ( int nIndex );

public:
	bool GetPartInfo ( const int nIndex, UIRECT& rcPart );

public:
	CString	GetText ( int nIndex );

	void SetTextData ( int nIndex, DWORD dwData );
	DWORD GetTextData ( int nIndex );

	void SetTextDataEx ( int nIndex, DWORD dwData );
	DWORD GetTextDataEx ( int nIndex );
	
	int	GetLineCount ( int nIndex );

	/*item link, Juver, 2017/07/31 */
	SITEMLINK*	GetItemLink ( int nIndex );

public:	
	void SetCurLine( int nCurLine );
	int	GetCurLine()					{ return m_nCurLine; }

public:
	long	GetLongestLine ();
	long	GetMaxTextWidth ();

public:
	int	GetCount()						{ return static_cast<int>( m_TextBox.size() ); }
	int	GetVisibleLine()				{ return m_nVisibleLine; }
	bool empty()						{ return m_TextBox.empty(); }

public:
	void CalcMaxHEIGHT( float* pHEIGHT, int* pVISIBLELINE, int nPartIndexCOPY, int nLineIndexCOPY );
	void CalcMaxHEIGHT( float* pHEIGHT, int* pVISIBLELINE );
	float CalcMaxHEIGHT( int nBeginLine, int nEndLine );

public:
	int GetTotalLine()						{ return m_nTotalLine; }

	void SetLimitLine( int nLimitLine )		{ m_nLimitLine = nLimitLine; }
	int GetLimitLine()						{ return m_nLimitLine; }
	void SetAutoChangeFontSize( bool bEnable )  { m_bAutoChangeFontSize = bEnable; }
	
	const float& GetLineInterval () const	{ return m_fLineInterval; }
	const float& GetPartInterval () const	{ return m_fPartInterval; }

protected:
	void AutoChangeFontSize( const CString& strText, NS_UITEXTUTIL::MULTILINETEXT& MLText, int& nSplitLine, bool bCRLF );
	BOOL FindStartIndex( int& nPartIndex, int& nLineIndex, const int nCurLine );

private:
	STEXTBOX	m_TextBox;

	int			m_nRenderdLine;		//	��� �Ϸ��� ���μ�
	int			m_nRenderPartIndex;	//	������� ��Ʈ �ε���
	int			m_nRenderLineIndex;	//	������� ���� �ε���
	BOOL		m_bRenderTextColor;	//	���콺 ������ �ؽ�Ʈ Į�� ���
	D3DCOLOR	m_dwRenderTextColor;//	���콺 ������ �ؽ�Ʈ Į��

	CD3DFontPar*	m_pFont;

	float	m_fLineInterval;
	float	m_fPartInterval;

	int		m_nAlign;

	int		m_nVisibleLine;

	int		m_nCurLine;
	int		m_nPartIndex;
	int		m_nLineIndex;

	int		m_nLimitLine;
	int		m_nTotalLine;

	std::deque<UIGUID>		m_ImageList;
	UIGUID	m_nImageGenID;

	LPDIRECT3DDEVICEQ	m_pd3dDevice;

	D3DCOLOR	m_dwOverColor;	//	���콺 ������ �÷�	
	int			m_nOverIndex;
	BOOL		m_bUseOverColor;	
	bool m_bAutoChangeFontSize;	
};