#include "StdAfx.h"
#include "BasicTextBox.h"
#include "UIRenderQueue.h"
#include "../DxCommon/DxFontMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const	float	CBasicTextBox::fDEFAULT_LINE_INTERVAL = 2.0f;
const	float	CBasicTextBox::fDEFAULT_PART_INTERVAL = 2.0f;
const	int		CBasicTextBox::nDEFAULT_STARTLINE = 0;
const	int		CBasicTextBox::nDEFAULT_VISIBLE_LINE = 1;
const	int		CBasicTextBox::nDEFAULT_LINE_LIMIT = 100;

CBasicTextBox::CBasicTextBox () :
	m_pFont ( NULL ),
	m_fLineInterval ( fDEFAULT_LINE_INTERVAL ),
	m_nCurLine ( nDEFAULT_STARTLINE ),
	m_nVisibleLine ( nDEFAULT_VISIBLE_LINE ),
	m_nAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP ),
	m_nLimitLine ( nDEFAULT_LINE_LIMIT ),
	m_nTotalLine ( 0 ),
	m_bUseOverColor ( FALSE ),
	m_nImageGenID ( BASE_IMAGE_CONTROLID ),
	m_pd3dDevice(NULL),
	m_fPartInterval ( fDEFAULT_PART_INTERVAL ),

	m_nPartIndex(0),
	m_nLineIndex(0),
	m_nRenderdLine(0),
	m_nRenderPartIndex(m_nPartIndex),
	m_nRenderLineIndex(m_nLineIndex),
	m_bRenderTextColor(TRUE),
	m_dwRenderTextColor(0xffffffff),
	m_dwOverColor(0xffffffff),
	m_nOverIndex(0),
    m_bAutoChangeFontSize(false)

{
}

CBasicTextBox::~CBasicTextBox ()
{
}

HRESULT CBasicTextBox::RenderWord ( STEXTWORD& sTextWord )
{
	const float fPosX = sTextWord.x;
	const float fPosY = sTextWord.y;
	CString& strWord = sTextWord.strWord;
	D3DCOLOR dwColor = sTextWord.dwColor;

	if ( m_bRenderTextColor )
		dwColor = m_dwRenderTextColor;

	bool bOverIndex(m_nRenderPartIndex==m_nOverIndex);	

	if( bOverIndex && m_bUseOverColor )
		dwColor = m_dwOverColor;	

	//	���̵� ��/�ƿ�
	D3DXCOLOR TempColor( dwColor );
	TempColor.a *= GetVisibleRate ();

	// �ؽ��Ŀ� ���ڸ� �������ϴ� ������� ���� ( ���� )
	return m_pFont->DrawText( fPosX, fPosY, TempColor, (TCHAR*)strWord.GetString(), 0L, FALSE );
}

HRESULT CBasicTextBox::RenderLine ( VECTORTEXTWORD& vectorTextWord )
{
	HRESULT hr;

	const int nTextWord = (int)vectorTextWord.size ();

	for ( int i = 0; i < nTextWord; ++i )
	{
		hr = RenderWord ( vectorTextWord[i] );
		if ( FAILED ( hr ) ) return hr;
	}

	return S_OK;
}

HRESULT CBasicTextBox::RenderPart ( DEQUETEXTLINE& dequeTextLine )
{
	HRESULT hr = S_OK;

	const int nTextLine = (int)dequeTextLine.size ();

	for ( ; m_nRenderLineIndex < nTextLine; ++m_nRenderLineIndex )
	{
		hr = RenderLine ( dequeTextLine[m_nRenderLineIndex].vectorWord );
		if ( FAILED ( hr ) ) return hr;

		++m_nRenderdLine;

		if ( m_nVisibleLine == m_nRenderdLine ) break;
	}

	return S_OK;
}

HRESULT	 CBasicTextBox::RenderText ()
{
	if ( !m_pFont )
	{
		GASSERT ( 0 && "��Ʈ�� ���Դϴ�. SetFont()�� �̿��Ͽ� ��Ʈ���� ���� �����ʽÿ�." );
		return E_FAIL;
	}

	HRESULT hr;

	const int nTextSize = (int)m_TextBox.size ();
	if ( nTextSize < 1 ) return S_FALSE;

    m_nRenderdLine = 0;
	m_nRenderPartIndex = m_nPartIndex;
	m_nRenderLineIndex = m_nLineIndex;

	for ( ; m_nRenderPartIndex < nTextSize; ++m_nRenderPartIndex )
	{
		m_bRenderTextColor = m_TextBox[m_nRenderPartIndex].m_bUseTextColor;
		m_dwRenderTextColor = m_TextBox[m_nRenderPartIndex].m_dwColor;

		hr = RenderPart ( m_TextBox[m_nRenderPartIndex].dequeLine );
		if ( FAILED ( hr ) ) return hr;

		m_nRenderLineIndex = 0;
        
		if ( m_nVisibleLine <= m_nRenderdLine ) break;
	}

	return S_OK;
}

HRESULT CBasicTextBox::Render ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;

	if ( !IsVisible () ) return S_OK;

	hr = CUIGroup::Render ( pd3dDevice );
	if ( FAILED ( hr ) ) return hr;

	CUIRenderQueue::Get()->RenderImmediately();
	


	hr = RenderText ();
	if ( FAILED ( hr ) ) return hr;

	return S_OK;
}

int CBasicTextBox::AddOneLineText( const CString& strText, bool bUseEllipsis /* = FALSE */, const D3DCOLOR& TextColor /* = NS_UITEXTUTIL::ENABLE */ )
{
	if ( !m_pFont )
	{
		GASSERT ( 0 && "��Ʈ�� ���Դϴ�. SetFont()�� �̿��Ͽ� ��Ʈ���� ���� �����ʽÿ�." );
		return 0;
	}
	
    float fWidth = GetLocalPos().sizeX;

	if( bUseEllipsis ) fWidth -= 20.0f;

	//	���ڿ� �ڸ���
	NS_UITEXTUTIL::ClearCRLFText ();
	NS_UITEXTUTIL::MULTILINETEXT& MLText = NS_UITEXTUTIL::GetMLTextWithoutCRLF ( strText, fWidth, m_pFont );

	int nSplitLine = (int)MLText.size ();

	if ( m_bAutoChangeFontSize )
        AutoChangeFontSize( strText, MLText, nSplitLine, false );

	STEXTPART sTextPart;

	//	����Ÿ�� ����ֱ�
	STEXTLINE sTextLine;

	SIZE strSize;

	if( nSplitLine >= 2 && bUseEllipsis ) 
		MLText[0].strLine += "...";

	m_pFont->GetTextExtent ( MLText[0].strLine.GetString(), strSize );

	sTextLine.strSize = strSize;
	sTextLine.bLASTLINE = MLText[0].bLASTLINE;
	if ( m_nAlign == TEXT_ALIGN_BOTH_X )
	{				
		NS_UITEXTUTIL::ClearWordText ();
		NS_UITEXTUTIL::STRINGVECTOR& WordText = NS_UITEXTUTIL::GetWordText ( MLText[0].strLine );

		for ( int k = 0; k < (int)WordText.size(); ++k )
		{
			STEXTWORD sTextWord;

			sTextWord.strWord = WordText[k];
			sTextWord.dwColor = TextColor;

			if ( WordText[k].GetLength () )
			{
				m_pFont->GetTextExtent ( WordText[k].GetString(), strSize );
				m_pFont->PushText( WordText[k].GetString() );
			}
			else
			{
				strSize.cx = 0;
				strSize.cy = BLANKSIZE.cy;
			}

			sTextWord.strSize = strSize;

			sTextLine.vectorWord.push_back ( sTextWord );
		}

		m_nTotalLine++;
		sTextPart.dequeLine.push_back ( sTextLine );
	}
	else
	{
		STEXTWORD sTextWord;
		sTextWord.strWord = MLText[0].strLine;
		sTextWord.dwColor = TextColor;

		sTextWord.strSize = strSize;

		if( MLText[0].strLine.GetLength() )
			m_pFont->PushText( MLText[0].strLine.GetString() );

		sTextLine.vectorWord.push_back ( sTextWord );
		sTextPart.dequeLine.push_back ( sTextLine );

		m_nTotalLine++;
	}	

	m_TextBox.push_back ( sTextPart );	

	ReCalcRenderPos ();

	return (int)m_TextBox.size () - 1;
}

int	CBasicTextBox::AddText ( const CString& strText, const D3DCOLOR& TextColor )
{
	if ( !m_pFont )
	{
		GASSERT ( 0 && "��Ʈ�� ���Դϴ�. SetFont()�� �̿��Ͽ� ��Ʈ���� ���� �����ʽÿ�." );
		return 0;
	}

	//	���ڿ� �ڸ���
	NS_UITEXTUTIL::ClearCRLFText ();
	NS_UITEXTUTIL::MULTILINETEXT& MLText = NS_UITEXTUTIL::GetMLTextWithoutCRLF ( strText, GetLocalPos().sizeX, m_pFont );

	int nSplitLine = (int)MLText.size ();

	if ( m_bAutoChangeFontSize )
			AutoChangeFontSize( strText, MLText, nSplitLine, false );
	
	STEXTPART sTextPart;

	//	����Ÿ�� ����ֱ�
	for ( int i = 0; i < nSplitLine; ++i )
	{		
		STEXTLINE sTextLine;

		if ( (i - 1 == nSplitLine) && !MLText[i].strLine.GetLength () )
		{
			//	������������ �� ���.
		}
		else
		{
			SIZE strSize;
			m_pFont->GetTextExtent ( MLText[i].strLine.GetString(), strSize );

			sTextLine.strSize = strSize;
			sTextLine.bLASTLINE = MLText[i].bLASTLINE;
			if ( m_nAlign == TEXT_ALIGN_BOTH_X )
			{				
				NS_UITEXTUTIL::ClearWordText ();
				NS_UITEXTUTIL::STRINGVECTOR& WordText = NS_UITEXTUTIL::GetWordText ( MLText[i].strLine );

				for ( int k = 0; k < (int)WordText.size(); ++k )
				{
					STEXTWORD sTextWord;

					sTextWord.strWord = WordText[k];
					sTextWord.dwColor = TextColor;

					if ( WordText[k].GetLength () )
					{
						m_pFont->GetTextExtent ( WordText[k].GetString(), strSize );
						m_pFont->PushText( WordText[k].GetString() );
					}
					else
					{
						strSize.cx = 0;
						strSize.cy = BLANKSIZE.cy;
					}

					sTextWord.strSize = strSize;

					sTextLine.vectorWord.push_back ( sTextWord );
				}
				
				m_nTotalLine++;
				sTextPart.dequeLine.push_back ( sTextLine );
			}
			else
			{
				STEXTWORD sTextWord;
				sTextWord.strWord = MLText[i].strLine;
				sTextWord.dwColor = TextColor;

				sTextWord.strSize = strSize;

				if( MLText[i].strLine.GetLength() )
					m_pFont->PushText( MLText[i].strLine.GetString() );

				sTextLine.vectorWord.push_back ( sTextWord );
				sTextPart.dequeLine.push_back ( sTextLine );

				m_nTotalLine++;
			}
		}
	}	

	m_TextBox.push_back ( sTextPart );

	//	���� ���� ������ ���� ���, �߶�
	for ( ; m_nLimitLine < m_nTotalLine; --m_nTotalLine )
	{		
		STEXTPART& sDelTextPart = m_TextBox.front ();
		DEQUETEXTLINE& dequeLine = sDelTextPart.dequeLine;

		dequeLine.pop_front ();

		//	����, ���ܿ� ���� ���� ���� ���,
		//	������ ������.
		if ( dequeLine.size () < 1 )
		{
			if ( m_ImageList.size () )
			{
				DeleteControl ( m_ImageList[0], 0 );
				m_ImageList.pop_front ();
			}
			m_TextBox.pop_front ();
		}
	}
	
	ReCalcRenderPos ();

	return (int)m_TextBox.size () - 1;
}

int CBasicTextBox::AddTextWithCRLF ( const CString& strText, const D3DCOLOR& TextColor )
{
	if ( !m_pFont )
	{
		GASSERT ( 0 && "��Ʈ�� ���Դϴ�. SetFont()�� �̿��Ͽ� ��Ʈ���� ���� �����ʽÿ�." );
		return 0;
	}

	//	���ڿ� �ڸ���
	NS_UITEXTUTIL::ClearCRLFText ();
	NS_UITEXTUTIL::MULTILINETEXT& MLText = NS_UITEXTUTIL::GetMLTextWithCRLF ( strText, GetLocalPos().sizeX, m_pFont );

	int nSplitLine = (int)MLText.size ();

    if ( m_bAutoChangeFontSize )
        AutoChangeFontSize( strText, MLText, nSplitLine, true );

	STEXTPART sTextPart;

	//	����Ÿ�� ����ֱ�
	for ( int i = 0; i < nSplitLine; ++i )
	{		
		STEXTLINE sTextLine;

		if ( (i - 1 == nSplitLine) && !MLText[i].strLine.GetLength () )
		{
			//	������������ �� ���.
		}
		else
		{
			SIZE strSize;
			m_pFont->GetTextExtent ( MLText[i].strLine.GetString(), strSize );

			sTextLine.strSize = strSize;
			sTextLine.bLASTLINE = MLText[i].bLASTLINE;
			if ( m_nAlign == TEXT_ALIGN_BOTH_X )
			{				
				NS_UITEXTUTIL::ClearWordText ();
				NS_UITEXTUTIL::STRINGVECTOR& WordText = NS_UITEXTUTIL::GetWordText ( MLText[i].strLine );

				for ( int k = 0; k < (int)WordText.size(); ++k )
				{
					STEXTWORD sTextWord;

					sTextWord.strWord = WordText[k];
					sTextWord.dwColor = TextColor;

					if ( WordText[k].GetLength () )
					{
						m_pFont->GetTextExtent ( WordText[k].GetString(), strSize );
						m_pFont->PushText( WordText[k].GetString() );
					}
					else
					{
						strSize.cx = 0;
						strSize.cy = BLANKSIZE.cy;
					}

					sTextWord.strSize = strSize;

					sTextLine.vectorWord.push_back ( sTextWord );
				}

				m_nTotalLine++;
				sTextPart.dequeLine.push_back ( sTextLine );
			}
			else
			{
				STEXTWORD sTextWord;
				sTextWord.strWord = MLText[i].strLine;
				sTextWord.dwColor = TextColor;

				sTextWord.strSize = strSize;

				if( MLText[i].strLine.GetLength() )
					m_pFont->PushText( MLText[i].strLine.GetString() );

				sTextLine.vectorWord.push_back ( sTextWord );
				sTextPart.dequeLine.push_back ( sTextLine );

				m_nTotalLine++;
			}
		}
	}	

	m_TextBox.push_back ( sTextPart );

	//	���� ���� ������ ���� ���, �߶�
	for ( ; m_nLimitLine < m_nTotalLine; --m_nTotalLine )
	{		
		STEXTPART& sDelTextPart = m_TextBox.front ();
		DEQUETEXTLINE& dequeLine = sDelTextPart.dequeLine;

		dequeLine.pop_front ();

		//	����, ���ܿ� ���� ���� ���� ���,
		//	������ ������.
		if ( dequeLine.size () < 1 )
		{
			if ( m_ImageList.size () )
			{
				DeleteControl ( m_ImageList[0], 0 );
				m_ImageList.pop_front ();
			}
			m_TextBox.pop_front ();
		}
	}

	ReCalcRenderPos ();

	return (int)m_TextBox.size () - 1;
}

void CBasicTextBox::AutoChangeFontSize( const CString& strText, NS_UITEXTUTIL::MULTILINETEXT& MLText, int& nSplitLine, bool bCRLF )
{
	if (!m_pFont )
	{
		GASSERT ( 0 && "��Ʈ�� ���Դϴ�. SetFont()�� �̿��Ͽ� ��Ʈ���� ���� �����ʽÿ�." );
		return;
	}

    const UIRECT& rcGlobalPos = GetGlobalPos();
    CD3DFontPar* pAutoFont = m_pFont;
    DWORD dwCurFontHeight = pAutoFont->GetFontHeight();
    float fTotalSize = 0.0f;

    do 
    {
        //	���ڿ� �ڸ���
        if ( bCRLF )
            MLText = NS_UITEXTUTIL::GetMLTextWithCRLF( strText, GetLocalPos().sizeX, pAutoFont );
        else
            MLText = NS_UITEXTUTIL::GetMLTextWithoutCRLF( strText, GetLocalPos().sizeX, pAutoFont );
        nSplitLine = static_cast<int>(MLText.size());

        fTotalSize = 0.0f;
        for ( int i = 0; i < nSplitLine; ++i )
        {
            SIZE strSize;
            pAutoFont->GetTextExtent( MLText[i].strLine.GetString(), strSize );
            fTotalSize += strSize.cy;
            if ( i != nSplitLine-1 )
                fTotalSize += GetLineInterval();
        }

        if ( rcGlobalPos.sizeY < fTotalSize )
        {
            dwCurFontHeight = pAutoFont->GetFontHeight();
            dwCurFontHeight -= 1;
            if ( dwCurFontHeight < 1 )
                dwCurFontHeight = 1;
			pAutoFont = DxFontMan::GetInstance().LoadDxFont( m_pFont->GetFontName(),
                dwCurFontHeight,
                m_pFont->GetFontFlags() );
        }
    } while ( dwCurFontHeight > 1 && rcGlobalPos.sizeY < fTotalSize );

    m_pFont = pAutoFont;
}

int	CBasicTextBox::AddString ( int nIndex, const CString& strText, const D3DCOLOR& TextColor )
{
	if ( !m_pFont )
	{
		GASSERT ( 0 && "��Ʈ�� ���Դϴ�. SetFont()�� �̿��Ͽ� ��Ʈ���� ���� �����ʽÿ�." );
		return -1;
	}

	if ( (int) m_TextBox.size () <= nIndex )
	{
		GASSERT ( 0 && "�ε����� ������ �Ѿ�ϴ�." );
		return -1;
	}

	if ( !strText.GetLength () )
	{
		GASSERT ( 0 && "���ڿ��� �����ϴ�." );
		return -1;
	}

	if ( m_bAutoChangeFontSize )
    {
        CString strTotal = "";
        CString strOrigin = "";
        for ( size_t i = 0; i < m_TextBox.size(); i++ )
        {
            strTotal.Append( GetText(i) );
            strOrigin.Append( GetText(i) );
        }
        strTotal.Append( strText );

        NS_UITEXTUTIL::MULTILINETEXT MLText;
        int nSplitLine;

        AutoChangeFontSize( strTotal, MLText, nSplitLine, false );
        ClearText();
    }

	CString strTextCopy = strText;

	STEXTPART& sTextPart = m_TextBox[nIndex];	

	{
		STEXTLINE& sTextLine = sTextPart.dequeLine.back ();

		//	���� ���ڿ� ������ ���ο� �� ���̱�		
		float fOldSize = (float) sTextLine.strSize.cx;

		//	������ ���ڿ��� ���� ��ŭ�� �ڸ���
		NS_UITEXTUTIL::ClearCRLFText ();
		NS_UITEXTUTIL::MULTILINETEXT& MLText =
			NS_UITEXTUTIL::GetMLTextWithoutCRLF ( strTextCopy, GetLocalPos().sizeX - fOldSize, m_pFont );

		if ( MLText.size () )
		{
			STEXTWORD sTextWord;
			sTextWord.strWord = MLText[0].strLine;
			sTextWord.dwColor = TextColor;
			m_pFont->GetTextExtent ( sTextWord.strWord.GetString(), sTextWord.strSize );

			if( MLText[0].strLine.GetLength() )
				m_pFont->PushText( MLText[0].strLine.GetString() );

			sTextLine.vectorWord.push_back ( sTextWord );
			sTextLine.strSize.cx += sTextWord.strSize.cx;
			sTextLine.strSize.cy = sTextWord.strSize.cy;

			int nCutPos = strTextCopy.GetLength () - sTextWord.strWord.GetLength ();
			strTextCopy = strTextCopy.Right ( nCutPos );
		}		
	}

	if ( strTextCopy.GetLength () )
	{
		//	������ ���ڿ��� ���� ��ŭ�� �ڸ���
		NS_UITEXTUTIL::ClearCRLFText ();
		NS_UITEXTUTIL::MULTILINETEXT& MLText =
			NS_UITEXTUTIL::GetMLTextWithoutCRLF ( strTextCopy, GetLocalPos().sizeX, m_pFont );

		int nSplitLine = (int)MLText.size ();
		
		//	����Ÿ�� ����ֱ�
		for ( int i = 0; i < nSplitLine; ++i )
		{		
			STEXTLINE sTextLine;
			STEXTWORD sTextWord;

			sTextWord.strWord = MLText[i].strLine;
			sTextWord.dwColor = TextColor;
			
			if ( (i - 1 == nSplitLine) && !sTextWord.strWord.GetLength () )
			{
				//	������������ �� ���.
			}
			else
			{
				m_pFont->GetTextExtent ( sTextWord.strWord.GetString(), sTextWord.strSize );

				if( MLText[i].strLine.GetLength() )
					m_pFont->PushText( MLText[i].strLine.GetString() );

				//	������ word�� �� �ǹ̸� ������ ���Ѵ�.
				//	���� �׻� 1���� �����Ѵٰ� �����ϰ�, 0�� �ε����� ����Ѵٰ�
				//	����Ѵ�.
				sTextLine.vectorWord.push_back ( sTextWord );

				sTextLine.strSize.cx = sTextWord.strSize.cx;
				sTextLine.strSize.cy = sTextWord.strSize.cy;
				sTextPart.dequeLine.push_back ( sTextLine );

				m_nTotalLine++;

				//	���� ������ ���ǰ�, �ɸ��� ���
	//			if ( m_nLimitLine && m_nLimitLine == i )	break;
			}
		}	
	}

//	m_TextBox.push_back ( sTextString );

	//	���� ���� ������ ���� ���, �߶�
	for ( ; m_nLimitLine < m_nTotalLine; --m_nTotalLine )
	{		
		sTextPart = m_TextBox.front ();
		DEQUETEXTLINE& dequeLine = sTextPart.dequeLine;

		dequeLine.pop_front ();

		//	����, ���ܿ� ���� ���� ���� ���,
		//	������ ������.
		if ( dequeLine.size () < 1 )
		{
			if ( m_ImageList.size () )
			{
				DeleteControl ( m_ImageList[0], 0 );
				m_ImageList.pop_front ();
			}
			m_TextBox.pop_front ();
		}
	}

	ReCalcRenderPos ();

	return (int) m_TextBox.size () - 1;
}

void	CBasicTextBox::SetText ( const CString& strText, const D3DCOLOR& TextColor )
{
	if(  1 == GetCount () )
	{

		if( strText == GetText( 0 ) ) return;
	}

	ClearText ();
	AddText ( strText, TextColor );
}

void CBasicTextBox::SetOneLineText ( const CString& strText, const D3DCOLOR& TextColor )
{
	if ( 1 == GetCount () )
	{
		CString& strOldText = GetText ( 0 );
		if ( strOldText == strText ) return ;
	}

	ClearText ();
	AddText ( strText, TextColor );
}

int CBasicTextBox::SetTextNoSplit ( const CString& strText, const D3DCOLOR& TextColor )
{
	if(  1 == GetCount () )
	{
		if( strText == GetText( 0 ) ) return -1;
	}

	ClearText ();	
	return AddTextNoSplit( strText, TextColor );
}

int CBasicTextBox::AddTextNoSplit ( const CString& strText, const D3DCOLOR& TextColor )
{
	if ( !m_pFont )
	{
		GASSERT ( 0 && "��Ʈ�� ���Դϴ�. SetFont()�� �̿��Ͽ� ��Ʈ���� ���� �����ʽÿ�." );
		return 0;
	}

	STEXTPART sTEXTPART;
	STEXTLINE sTEXTLINE;

	{
		STEXTWORD sTEXTWORD;
		sTEXTWORD.strWord = strText;
		sTEXTWORD.dwColor = TextColor;
		m_pFont->GetTextExtent ( sTEXTWORD.strWord.GetString(), sTEXTWORD.strSize );

		if( strText.GetLength() )
			m_pFont->PushText( strText.GetString() );

		sTEXTLINE.vectorWord.push_back ( sTEXTWORD );

		sTEXTLINE.strSize.cx = sTEXTWORD.strSize.cx;
		sTEXTLINE.strSize.cy = sTEXTWORD.strSize.cy;
	}

	sTEXTPART.dequeLine.push_back ( sTEXTLINE );
	m_TextBox.push_back ( sTEXTPART );

	m_nTotalLine++;

	//	���� ���� ������ ���� ���, �߶�
	for ( ; m_nLimitLine < m_nTotalLine; --m_nTotalLine )
	{		
		sTEXTPART = m_TextBox.front ();
		DEQUETEXTLINE& dequeLine = sTEXTPART.dequeLine;

		dequeLine.pop_front ();

		//	����, ���ܿ� ���� ���� ���� ���,
		//	������ ������.
		if ( dequeLine.size () < 1 )
		{
			if ( m_ImageList.size () )
			{
				DeleteControl ( m_ImageList[0], 0 );
				m_ImageList.pop_front ();
			}
			m_TextBox.pop_front ();
		}
	}
    
	ReCalcRenderPos ();

	return (int)m_TextBox.size () - 1;
}

void CBasicTextBox::CalcMaxHEIGHT ( float* pHEIGHT, int* pVISIBLELINE )
{
	float& fTextHeight = *pHEIGHT;
	int& nVisibleLine = *pVISIBLELINE;

	const UIRECT& rcGlobalPos = GetGlobalPos ();

	//	�ʱ�ȭ
	fTextHeight = 0.0f;
	nVisibleLine = 0;

	int nPartSize = (int) m_TextBox.size ();
	for ( int i = nPartSize-1; 0 <= i; --i )
	{
		STEXTPART& sTextPart = m_TextBox[i];
		DEQUETEXTLINE& sDequeTextLine = sTextPart.dequeLine;
		int nLineCount = (int) sDequeTextLine.size ();

		//	���ε� �� �ִ밪�� �̾ƿ�
		for ( int j = nLineCount-1; 0 <= j; --j )
		{
			STEXTLINE& sTextLine = sDequeTextLine[j];

			//	�� ���� ����
			float fLINE_HEIGHT = (float)sTextLine.strSize.cy;

			//	�� ���� �Ѹ� �� �ִ°�?
			if ( rcGlobalPos.sizeY <= (fTextHeight+fLINE_HEIGHT) )
			{
				if ( !nVisibleLine )
				{
//					CDebugSet::ToLogFile ( "[%s] ����� �ʹ� �۽��ϴ�.", GetControlName() );

					//	������ �� ������ �������� ������
					nVisibleLine = 1;
				}

				return ;
			}

			fTextHeight += fLINE_HEIGHT;

			//	�������� �ִ� �� ��
			++nVisibleLine;

			//	�� ����
			if ( j != 0 )
			{
				float fLINE_GAP = GetLineInterval ();
				fTextHeight += fLINE_GAP;
			}
		}			

		//	���� ����
		if ( i != 0 )
		{
			float fPART_GAP = GetPartInterval ();
			fTextHeight += fPART_GAP;
		}
	}
}

void CBasicTextBox::CalcMaxHEIGHT ( float* pHEIGHT, int* pVISIBLELINE, int nPartIndexCOPY, int nLineIndexCOPY )
{
	float& fTextHeight = *pHEIGHT;
	int& nVisibleLine = *pVISIBLELINE;

	const UIRECT& rcGlobalPos = GetGlobalPos ();

	//	�ʱ�ȭ
	fTextHeight = 0.0f;
	nVisibleLine = 0;

	int nPartSize = (int) m_TextBox.size ();
	for ( int i = nPartIndexCOPY; i < nPartSize; ++i )
	{
		STEXTPART& sTextPart = m_TextBox[i];
		DEQUETEXTLINE& sDequeTextLine = sTextPart.dequeLine;
		int nLineCount = (int) sDequeTextLine.size ();

		//	���ε� �� �ִ밪�� �̾ƿ�
		for ( int j = nLineIndexCOPY; j < nLineCount; ++j )
		{
			STEXTLINE& sTextLine = sDequeTextLine[j];

			//	�� ���� ����
			float fLINE_HEIGHT = (float)sTextLine.strSize.cy;

			//	�� ���� �Ѹ� �� �ִ°�?
			if ( rcGlobalPos.sizeY < (fTextHeight+fLINE_HEIGHT) )
			{
				if ( !nVisibleLine )
				{
//					CDebugSet::ToLogFile ( "[%s] ����� �ʹ� �۽��ϴ�.", GetControlName() );

					//	������ �� ������ �������� ������
					nVisibleLine = 1;
				}

				return ;
			}

			fTextHeight += fLINE_HEIGHT;

			//	�������� �ִ� �� ��
			++nVisibleLine;

			//	�� ����
			if ( (j+1) != nLineCount )
			{
				float fLINE_GAP = GetLineInterval ();
				fTextHeight += fLINE_GAP;
			}
		}			

		//	���� ����
		if ( (i+1) != nPartSize )
		{
			float fPART_GAP = GetPartInterval ();
			fTextHeight += fPART_GAP;
		}

		nLineIndexCOPY = 0;
	}
}

float CBasicTextBox::CalcMaxHEIGHT ( int nBeginLine, int nEndLine )
{
	if ( nBeginLine < 0 ) return 0.0f;	
	if ( nEndLine < nBeginLine ) return 0.0f;

	//	�ʱ�ȭ
	float fTextHeight = 0.0f;
	int nBEGIN_PART = 0;
	int nBEGIN_LINE = 0;
	int nAllLineCountEnd = nEndLine - nBeginLine;
	int nAllLineCount = 0;

	if ( !FindStartIndex ( nBEGIN_PART, nBEGIN_LINE, nBeginLine ) ) return 0.0f;

	int nPartSize = (int) m_TextBox.size ();
	for ( int i = nBEGIN_PART; i < nPartSize; ++i )
	{
		STEXTPART& sTextPart = m_TextBox[i];
		DEQUETEXTLINE& sDequeTextLine = sTextPart.dequeLine;
		int nLineCount = (int) sDequeTextLine.size ();

		//	���ε� �� �ִ밪�� �̾ƿ�
		for ( int j = nBEGIN_LINE; j < nLineCount; ++j )
		{
			STEXTLINE& sTextLine = sDequeTextLine[j];

			//	�� ���� ����			
			fTextHeight += (float)sTextLine.strSize.cy;

			//	���� ���
			++nAllLineCount;
			if ( nAllLineCountEnd < nAllLineCount ) return fTextHeight;

			//	�� ����
			if ( (j+1) != nLineCount ) fTextHeight += m_fLineInterval;
		}	

		//	���� ����
		if ( (i+1) != nPartSize ) fTextHeight += m_fPartInterval;

		nBEGIN_LINE = 0;
	}

	return fTextHeight;
}

void CBasicTextBox::ReCalcRenderPos ()
{	
	const UIRECT& rcGlobalPos = GetGlobalPos ();

	//	�Ѹ� ���ڿ� ��ġ Ȯ��	
	if ( !(m_nAlign & TEXT_ALIGN_BOTTOM) )
	{
		if ( !FindStartIndex ( m_nPartIndex, m_nLineIndex, m_nCurLine ) ) return ;
	}

	//	������ ����
	for ( int nImage = 0; nImage < (int)m_ImageList.size (); ++nImage )
	{
		CUIControl* pImage = FindControl ( m_ImageList[nImage] );
		if ( pImage ) pImage->SetVisibleSingle ( FALSE );
	}

	const int nVisibleLineBack = m_nVisibleLine;

	//	�Ѹ� �ִ� Y ������ ���
	float fTextHeight = 0.0f;
	if ( m_nAlign & TEXT_ALIGN_BOTTOM )
	{
		CalcMaxHEIGHT ( &fTextHeight, &m_nVisibleLine );
		int nEND_LINE = m_nTotalLine - m_nVisibleLine;
		if ( nEND_LINE < m_nCurLine ) m_nCurLine = nEND_LINE;
		if ( !FindStartIndex ( m_nPartIndex, m_nLineIndex, m_nCurLine ) ) return ;
	}
	else
	{
		CalcMaxHEIGHT ( &fTextHeight, &m_nVisibleLine, m_nPartIndex, m_nLineIndex );
	}

	//	���纻
	int nPartIndex = m_nPartIndex;
	int nLineIndex = m_nLineIndex;

	//	X�� �⺻ ������
	float fLEFT = (float)ceil(rcGlobalPos.left);

	//	Y�� �⺻ ������
	float fTOP = (float)ceil(rcGlobalPos.top);			
	if ( m_nAlign & TEXT_ALIGN_TOP )
		;
	else if ( m_nAlign & TEXT_ALIGN_BOTTOM )
		fTOP = (float)ceil(rcGlobalPos.bottom - fTextHeight);
	else if ( m_nAlign & TEXT_ALIGN_CENTER_Y )
		fTOP = (float)ceil(rcGlobalPos.top + (rcGlobalPos.sizeY-fTextHeight) * 0.5f);
    
	bool bBREAK = false;
	int nCurrentLine = 0;
	int nPartSize = (int) m_TextBox.size ();
	float fTEXT_POS_Y = fTOP;
	for ( int i = nPartIndex; i < nPartSize; ++i )
	{
		STEXTPART& sTextPart = m_TextBox[i];
		UIRECT& rcPart = sTextPart.m_rcPart;
		DEQUETEXTLINE& sDequeTextLine = sTextPart.dequeLine;
		int nLineCount = (int)sDequeTextLine.size ();

		D3DXVECTOR2 vSIZE ( 0.0f, 0.0f );
		D3DXVECTOR2 vBEGIN( 0.0f, 0.0f );

		//	NOTE
		//		���ο� �����ΰ�?
		//		������ ���� �Ǵ� ���� ���� ���� �����ΰ�?		
		bool bNEWLINE = true;
		for ( int j = nLineIndex; j < nLineCount; ++j )
		{
			++nCurrentLine;
			if ( m_nVisibleLine < nCurrentLine )
			{
				bBREAK = true;
				break;
			}

			STEXTLINE& sTextLine = sDequeTextLine[j];
			float fSizeX = (float) sTextLine.strSize.cx;
			float fSizeY = (float) sTextLine.strSize.cy;

			//	X ����
			float fPosX = fLEFT;
			if ( m_nAlign & TEXT_ALIGN_LEFT  )
				;
			else if ( m_nAlign & TEXT_ALIGN_RIGHT )
				fPosX = (float)ceil(rcGlobalPos.right - fSizeX);
			else if ( m_nAlign & TEXT_ALIGN_CENTER_X )
				fPosX = (float)ceil(fLEFT + (rcGlobalPos.sizeX - fSizeX) / 2.0f);            
			
			//	Y ����
			float fPosY = fTEXT_POS_Y;

			fTEXT_POS_Y += fSizeY;				//	���� �ִ� ��
			if ( j != (nLineCount-1) ) fTEXT_POS_Y += GetLineInterval();	//	�ٰ�

			//	��Ʈ�� ù��° �ٿ��� ������ ���
			if ( j == 0 )
			{
				if ( i < (int) m_ImageList.size () )
				{
					CUIControl* pImage = FindControl ( m_ImageList[i] );
					if ( pImage )
					{
						const UIRECT& rcImagePos = pImage->GetGlobalPos ();
						pImage->SetGlobalPos (D3DXVECTOR2(fPosX-rcImagePos.sizeX-5.0f,fPosY));
						pImage->SetVisibleSingle ( TRUE );					
					}
				}
			}


			VECTORTEXTWORD& vectorTextWord = sTextLine.vectorWord;
			if ( m_nAlign & TEXT_ALIGN_BOTH_X && 1 < vectorTextWord.size() )
			{
				long lGAP = 0;
				long OneGap = 0;
				int nBlankCount = (int)vectorTextWord.size() - 1;
				if ( !nBlankCount ) nBlankCount = 1;

				//	NOTE
				//		�ǵ������� ùĭ�� ������ �ƴѵ�,
				//		ùĭ�� ����, ������� �ʴ´�.
				if ( !vectorTextWord[0].strWord.GetLength() && !bNEWLINE )
				{
					lGAP += BLANKSIZE.cx;
					nBlankCount--;
					if ( nBlankCount == 0 ) nBlankCount = 1;
				}

				//	NOTE
				//		������ ĭ�� ��ĭ�� ��� ���� ��� ������� �ʴ´�.				
				if ( !vectorTextWord[vectorTextWord.size()-1].strWord.GetLength() )
				{
					lGAP += BLANKSIZE.cx;
					nBlankCount--;
					if ( nBlankCount == 0 ) nBlankCount = 1;
				}

				//	NOTE
				//		������ ��µǰ� ���� �κ��� ���鿡 ���ؼ�
				//		�󸶸�ŭ���� ���� ������ ���鿡 �����ų�������� ����
				//		����Ѵ�.
				lGAP += (long(ceil(rcGlobalPos.sizeX)) - sTextLine.strSize.cx);				
				OneGap = (long)ceil(float(lGAP) / float(nBlankCount));

				for ( int k = 0; k < (int) vectorTextWord.size (); ++k )
				{
					STEXTWORD& sTextWord = vectorTextWord[k];

					if ( k == 0 && j == nLineIndex )
					{
						vBEGIN.x = fPosX;
						vBEGIN.y = fPosY;
					}

					//	NOTE
					//		�ǵ������� ùĭ�� ������ �ƴѵ�,
					//		ùĭ�� �� ���, �����Ѵ�.
					if ( k == 0 && !bNEWLINE )
					{
						if ( !sTextWord.strWord.GetLength () )
						{
							continue;
						}
					}

					//	NOTE
					//		������ ĭ�� �� ��� ������� �ʴ´�.
					bool bLINE_END = (k == (vectorTextWord.size () - 1));
					if ( bLINE_END )
					{
						if ( !sTextWord.strWord.GetLength () )
						{
							continue;
						}
					}
					
					sTextWord.x = fPosX;
					sTextWord.y = fPosY;

					fPosX += sTextWord.strSize.cx;
					fPosX += BLANKSIZE.cx;

					//	NOTE
					//		���������� CR/LF�� ���� ����Ƿ��� �ϴ°�?
					//		bLASTLINE�� true�� ���, ������ �ڽ��� ������ �ʰ�
					//		����ȴٴ� ���� �ǹ��Ѵ�.
					if ( !sTextLine.bLASTLINE )
					{
						if ( 0 < lGAP )
						{
							//	����, ������ ���̸� '��'�� �� �Һ��Ѵ�.
							if ( bLINE_END )
							{
								fPosX += lGAP;
								lGAP = 0;
							}
							else
							{
								if ( OneGap < lGAP )
								{
									fPosX += OneGap;
									lGAP -= OneGap;
								}
								else
								{
									fPosX += lGAP;
									lGAP = 0;
								}
							}
						}

						bNEWLINE = false;
					}
					else
					{
						bNEWLINE = true;
					}
				}
			}
			else
			{
				for ( int k = 0; k < (int) vectorTextWord.size (); ++k )
				{
					STEXTWORD& sTextWord = vectorTextWord[k];
					sTextWord.x = fPosX;
					sTextWord.y = fPosY;

					if ( k == 0 && j == nLineIndex )
					{
						vBEGIN.x = fPosX;
						vBEGIN.y = fPosY;
					}

					fPosX += sTextWord.strSize.cx;
				}
			}

			vSIZE.x = max(sTextLine.strSize.cx, vSIZE.x);
			vSIZE.y += fSizeY;

			if ( j != (nLineCount-1) )
			{
				vSIZE.y += m_fLineInterval;
			}
		}

		if ( bBREAK ) return;

		if ( i != (nPartSize-1) ) fTEXT_POS_Y += GetPartInterval ();	//	��Ʈ ���͹�

		rcPart = UIRECT ( vBEGIN.x, vBEGIN.y, vSIZE.x, vSIZE.y );

		nLineIndex = 0;
	}	
}

BOOL CBasicTextBox::FindStartIndex ( int& nPartIndex, int& nLineIndex, const int nCurLine )
{
	if ( m_nTotalLine < 1 )	return FALSE;

	const int nPartSize = (int)m_TextBox.size ();
    
	int nCurLineAmount = 0;
	for ( int nPartIterIndex = 0; nPartIterIndex < nPartSize; ++nPartIterIndex )
	{
		const STEXTPART& sTextPart = m_TextBox[nPartIterIndex];

        const int nLineSize = (int)sTextPart.dequeLine.size ();
		nCurLineAmount += nLineSize;

		if ( nCurLine < nCurLineAmount )
		{
			nPartIndex = nPartIterIndex;			
			nLineIndex = nCurLine - (nCurLineAmount - nLineSize);

			return TRUE;
		}
	}

	return FALSE;
}

HRESULT CBasicTextBox::InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	m_pd3dDevice = pd3dDevice;
	return CUIGroup::InitDeviceObjects ( pd3dDevice );
}

int CBasicTextBox::AddTextNoSplitBig ( const CString& strText, const D3DCOLOR& TextColor )
{
	if ( !m_pFont )
	{
		GASSERT ( 0 && "��Ʈ�� ���Դϴ�. SetFont()�� �̿��Ͽ� ��Ʈ���� ���� �����ʽÿ�." );
		return 0;
	}

	STEXTPART sTEXTPART;
	STEXTLINE sTEXTLINE;

	{
		STEXTWORD sTEXTWORD;
		sTEXTWORD.strWord = strText;
		sTEXTWORD.dwColor = TextColor;
		m_pFont->GetTextExtent ( sTEXTWORD.strWord.GetString(), sTEXTWORD.strSize );

		if( strText.GetLength() )
			m_pFont->PushText( strText.GetString() );

		sTEXTLINE.vectorWord.push_back ( sTEXTWORD );

		sTEXTLINE.strSize.cx = sTEXTWORD.strSize.cx;
		sTEXTLINE.strSize.cy = sTEXTWORD.strSize.cy;
	}

	sTEXTPART.dequeLine.push_back ( sTEXTLINE );
	m_TextBox.push_back ( sTEXTPART );

	m_nTotalLine++;

	//	���� ���� ������ ���� ���, �߶�
	for ( ; m_nLimitLine < m_nTotalLine; --m_nTotalLine )
	{		
		sTEXTPART = m_TextBox.front ();
		DEQUETEXTLINE& dequeLine = sTEXTPART.dequeLine;

		dequeLine.pop_front ();

		//	����, ���ܿ� ���� ���� ���� ���,
		//	������ ������.
		if ( dequeLine.size () < 1 )
		{
			if ( m_ImageList.size () )
			{
				DeleteControl ( m_ImageList[0], 0 );
				m_ImageList.pop_front ();
			}
			m_TextBox.pop_front ();
		}
	}
    
	ReCalcRenderPos ();

	return (int)m_TextBox.size () - 1;
}
//Add MultiText by NaJ
int	CBasicTextBox::AddMultiText ( MULTITEXT strText )
{
	if ( !m_pFont )
	{
		GASSERT ( 0 && " " );
		return 0;
	}

	STEXTPART sTEXTPART;
	STEXTLINE sTEXTLINE;

	for ( int i = 0; i < 100; i++ )
	{
		if ( strText.strText[i] == "" ) continue;

		if ( strText.strText[i] )
		{
			STEXTWORD sTEXTWORD;
			sTEXTWORD.strWord = strText.strText[i];
			sTEXTWORD.dwColor = strText.dwColor[i];
			m_pFont->GetTextExtent ( sTEXTWORD.strWord.GetString(), sTEXTWORD.strSize );

			if( strText.strText[i].GetLength() ) m_pFont->PushText( strText.strText[i].GetString() );

			sTEXTLINE.vectorWord.push_back ( sTEXTWORD );
			sTEXTLINE.strSize.cx += sTEXTWORD.strSize.cx; 
			sTEXTLINE.strSize.cy = sTEXTWORD.strSize.cy;
		}
	}

	sTEXTPART.dequeLine.push_back ( sTEXTLINE );
	m_TextBox.push_back ( sTEXTPART );
	m_nTotalLine++;

	for ( ; m_nLimitLine < m_nTotalLine; --m_nTotalLine )
	{		
		sTEXTPART = m_TextBox.front ();
		DEQUETEXTLINE& dequeLine = sTEXTPART.dequeLine;

		dequeLine.pop_front ();

		if ( dequeLine.size () < 1 )
		{
			if ( m_ImageList.size () )
			{
				DeleteControl ( m_ImageList[0], 0 );
				m_ImageList.pop_front ();
			}
			m_TextBox.pop_front ();
		}
	}
    
	ReCalcRenderPos ();

	return (int)m_TextBox.size () - 1;
}

//Add MultiTextLongestLine by NaJ
int	CBasicTextBox::AddMultiTextLongestLine ( MULTITEXT strText )
{
	if ( !m_pFont )
	{
		GASSERT ( 0 && " " );
		return 0;
	}

	STEXTPART sTEXTPART;
	STEXTLINE sTEXTLINE;

	int nLength = 0;
	for ( int i = 0; i < 100; i++ )
	{
		if ( strText.strText[i] == "" ) continue;
		if ( !strText.strText[i] ) continue;

		CString strCurText = strText.strText[i];
		for( int j = 0; j < strCurText.GetLength(); j++ )
		{
			if ( nLength < MULTITEXTSPLIT_LENGTH )
			{
				STEXTWORD sTEXTWORD;
				sTEXTWORD.strWord = strCurText[j];
				sTEXTWORD.dwColor = strText.dwColor[i];
				m_pFont->GetTextExtent ( sTEXTWORD.strWord.GetString(), sTEXTWORD.strSize );

				if( strCurText.GetLength() ) m_pFont->PushText( sTEXTWORD.strWord.GetString() );

				sTEXTLINE.vectorWord.push_back ( sTEXTWORD );

				sTEXTLINE.strSize.cx += sTEXTWORD.strSize.cx; 
				sTEXTLINE.strSize.cy = sTEXTWORD.strSize.cy;
			}
			else
			{
				sTEXTPART.dequeLine.push_back ( sTEXTLINE );
				m_TextBox.push_back ( sTEXTPART );
				m_nTotalLine++;

				for ( ; m_nLimitLine < m_nTotalLine; --m_nTotalLine )
				{		
					sTEXTPART = m_TextBox.front ();
					DEQUETEXTLINE& dequeLine = sTEXTPART.dequeLine;

					dequeLine.pop_front ();

					if ( dequeLine.size () < 1 )
					{
						if ( m_ImageList.size () )
						{
							DeleteControl ( m_ImageList[0], 0 );
							m_ImageList.pop_front ();
						}
						m_TextBox.pop_front ();
					}
				}
			    
				ReCalcRenderPos ();

				sTEXTPART = STEXTPART();
				sTEXTLINE = STEXTLINE();

				STEXTWORD sTEXTWORD;
				sTEXTWORD.strWord = strCurText[j];
				sTEXTWORD.dwColor = strText.dwColor[i];
				m_pFont->GetTextExtent ( sTEXTWORD.strWord.GetString(), sTEXTWORD.strSize );

				if( strCurText.GetLength() ) m_pFont->PushText( sTEXTWORD.strWord.GetString() );

				sTEXTLINE.vectorWord.push_back ( sTEXTWORD );

				sTEXTLINE.strSize.cx += sTEXTWORD.strSize.cx; 
				sTEXTLINE.strSize.cy = sTEXTWORD.strSize.cy;

				nLength = 0;
			}
			nLength++;
		}
	}

	if ( nLength )
	{
		sTEXTPART.dequeLine.push_back ( sTEXTLINE );
		m_TextBox.push_back ( sTEXTPART );
		m_nTotalLine++;

		for ( ; m_nLimitLine < m_nTotalLine; --m_nTotalLine )
		{		
			sTEXTPART = m_TextBox.front ();
			DEQUETEXTLINE& dequeLine = sTEXTPART.dequeLine;

			dequeLine.pop_front ();

			if ( dequeLine.size () < 1 )
			{
				if ( m_ImageList.size () )
				{
					DeleteControl ( m_ImageList[0], 0 );
					m_ImageList.pop_front ();
				}
				m_TextBox.pop_front ();
			}
		}
	    
		ReCalcRenderPos ();
	}

	return (int)m_TextBox.size () - 1;
}