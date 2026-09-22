#include "StdAfx.h"
#include "BasicTextBox.h"
#include "UIRenderQueue.h"
#include "../DxCommon/DxFontMan.h"
#include "GLItemMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*item link, Juver, 2017/07/31 */
SITEMLINK* CBasicTextBox::GetItemLink ( int nIndex )
{
	const int nSize = (int)m_TextBox.size ();

	if ( nSize < 1 )
	{
		GASSERT ( 0 && "텍스트박스가 비어 있습니다." );
		return NULL;
	}

	if ( nIndex < 0 || nSize <= nIndex )
	{
		GASSERT ( 0 && "잘못된 인덱스입니다." );
		return NULL;
	}	

	STEXTPART& sTEXTPART = m_TextBox[nIndex];
	return &sTEXTPART.m_sItemLink;
}

/*item link, Juver, 2017/07/31 */
int	CBasicTextBox::AddText ( const CString& strText, SITEMLINK* pItemLink, const D3DCOLOR& TextColor /*= NS_UITEXTUTIL::ENABLE*/ )
{
	if ( !m_pFont )
	{
		GASSERT ( 0 && "CBasicTextBox::AddText invalid m_pFont" );
		return 0;
	}

	//check item link
	BOOL bExistItemLink = FALSE;
	DWORD dwItemLinkColor = TextColor;
	if ( pItemLink )
	{
		SITEM* pItemData = GLItemMan::GetInstance().GetItem( pItemLink->sNativeID );
		if ( pItemData )
		{
			std::string strSearch = strText.GetString();
			int nFind = strSearch.find( pItemLink->GetFilter().GetString() );
			if ( nFind != std::string::npos )
			{
				bExistItemLink = TRUE;
				dwItemLinkColor = COMMENT::ITEMCOLOR[pItemData->sBasicOp.emLevel];
			}
		}
	}

	//no item link use orig addtext function
	if ( !bExistItemLink )	return AddText( strText, TextColor );

	NS_UITEXTUTIL::ClearCRLFText ();
	NS_UITEXTUTIL::MULTILINETEXT& MLText = NS_UITEXTUTIL::GetMLTextWithoutCRLF ( strText, GetLocalPos().sizeX, m_pFont );

	int nSplitLine = (int)MLText.size ();

	CString strItemLinkText = pItemLink->GetFilter();

	//now we verify if itemlink text is not being split to lines
	BOOL bFoundItemLinkLine = FALSE;
	for ( int i = 0; i < nSplitLine; ++i )
	{
		std::string strSearch = MLText[i].strLine.GetString();
		int nFind = strSearch.find( strItemLinkText.GetString() );
		if ( nFind != std::string::npos )	
			bFoundItemLinkLine = TRUE;
	}

	if ( !bFoundItemLinkLine )
	{
		//text got split need to fix
		CString strNewItemLinkText;
		strNewItemLinkText += "\r\n";
		strNewItemLinkText += pItemLink->GetFilter();

		CString strNewText = strText;
		strNewText.Replace( strItemLinkText.GetString(), strNewItemLinkText.GetString() );

		NS_UITEXTUTIL::ClearCRLFText ();
		MLText = NS_UITEXTUTIL::GetMLTextWithoutCRLF ( strNewText, GetLocalPos().sizeX, m_pFont );
		nSplitLine = (int)MLText.size ();
	}


	STEXTPART sTextPart;
	sTextPart.m_sItemLink = *pItemLink;


	//	데이타로 집어넣기
	for ( int i = 0; i < nSplitLine; ++i )
	{		
		STEXTLINE sTextLine;

		if ( (i - 1 == nSplitLine) && !MLText[i].strLine.GetLength () )
		{
			//	마지막라인이 빈 경우.
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

	//	제한 라인 범위가 넘을 경우, 잘라냄
	for ( ; m_nLimitLine < m_nTotalLine; --m_nTotalLine )
	{		
		STEXTPART& sDelTextPart = m_TextBox.front ();
		DEQUETEXTLINE& dequeLine = sDelTextPart.dequeLine;

		dequeLine.pop_front ();

		//	만약, 문단에 실제 글이 없을 경우,
		//	문단을 날린다.
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

int CBasicTextBox::AddMultiTextNoSplit ( STEXTMULTIWORDVEC &vecMultiWord )
{
	if ( !m_pFont )
	{
		GASSERT ( 0 && "폰트가 널입니다. SetFont()를 이용하여 폰트부터 값을 넣으십시오." );
		return 0;
	}

	if ( vecMultiWord.empty() )	return 0;

	STEXTPART sTEXTPART;
	STEXTLINE sTEXTLINE;
	sTEXTLINE.strSize.cx = 0;
	sTEXTLINE.strSize.cy = 0; 

	for ( int i=0; i<(int)vecMultiWord.size(); ++i )
	{
		STEXTMULTIWORD& sText = vecMultiWord[i];

		if ( sText.strWord.GetLength() )
		{
			STEXTWORD sTEXTWORD;
			sTEXTWORD.strWord = sText.strWord;
			sTEXTWORD.dwColor = sText.dwColor;
			m_pFont->GetTextExtent ( sTEXTWORD.strWord.GetString(), sTEXTWORD.strSize );
			m_pFont->PushText( sTEXTWORD.strWord.GetString() );

			sTEXTLINE.vectorWord.push_back ( sTEXTWORD );
			sTEXTLINE.strSize.cx += sTEXTWORD.strSize.cx;

			if ( sTEXTWORD.strSize.cy > sTEXTLINE.strSize.cy )
				sTEXTLINE.strSize.cy = sTEXTWORD.strSize.cy;
		}
	}
	
	sTEXTPART.dequeLine.push_back ( sTEXTLINE );
	m_TextBox.push_back ( sTEXTPART );

	m_nTotalLine++;

	//	제한 라인 범위가 넘을 경우, 잘라냄
	for ( ; m_nLimitLine < m_nTotalLine; --m_nTotalLine )
	{		
		sTEXTPART = m_TextBox.front ();
		DEQUETEXTLINE& dequeLine = sTEXTPART.dequeLine;

		dequeLine.pop_front ();

		//	만약, 문단에 실제 글이 없을 경우,
		//	문단을 날린다.
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

int	CBasicTextBox::AddStringNoSplit ( int nIndex, const CString& strText, const D3DCOLOR& TextColor )
{
	if ( !m_pFont )
	{
		GASSERT ( 0 && "폰트가 널입니다. SetFont()를 이용하여 폰트부터 값을 넣으십시오." );
		return -1;
	}

	if ( (int) m_TextBox.size () <= nIndex )
	{
		GASSERT ( 0 && "인덱스가 범위를 넘어섭니다." );
		return -1;
	}

	if ( !strText.GetLength () )
	{
		GASSERT ( 0 && "문자열이 없습니다." );
		return -1;
	}

	STEXTPART& sTextPart = m_TextBox[nIndex];	

	{
		STEXTLINE& sTextLine = sTextPart.dequeLine.back ();

		{
			STEXTWORD sTextWord;
			sTextWord.strWord = strText;
			sTextWord.dwColor = TextColor;
			m_pFont->GetTextExtent ( sTextWord.strWord.GetString(), sTextWord.strSize );

			if( strText.GetLength() )
				m_pFont->PushText( strText.GetString() );

			sTextLine.vectorWord.push_back ( sTextWord );
			sTextLine.strSize.cx += sTextWord.strSize.cx;
			sTextLine.strSize.cy = sTextWord.strSize.cy;
		}		
	}

	for ( ; m_nLimitLine < m_nTotalLine; --m_nTotalLine )
	{		
		sTextPart = m_TextBox.front ();
		DEQUETEXTLINE& dequeLine = sTextPart.dequeLine;

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

	return (int) m_TextBox.size () - 1;
}