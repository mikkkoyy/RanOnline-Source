#include "StdAfx.h"

#include "./BasicVarTextBox.h"
#include "./BasicTextBox.h"
#include "./BasicLineBoxEx.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "./UITextControl.h"
#include "../Lib_Engine/GUInterface/UIDebugSet.h"
#include "ItemImage.h"
#include "GLItemMan.h"
#include "GLItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const	float	CBasicVarTextBox::fMOUSEPOINT_GAP = 20.0f;

CBasicVarTextBox::CBasicVarTextBox () :
	m_pTextBox ( NULL ),
	m_pLineBox ( NULL ),
	m_pFont ( NULL ),
	m_pSelfDummy ( NULL ),
	m_vMousePointGap (fMOUSEPOINT_GAP,fMOUSEPOINT_GAP),
	m_bBLOCK_MOUSETRACKING ( false ),
	m_pItemImage( NULL ),
	m_pTextBoxName ( NULL )
{
}

CBasicVarTextBox::~CBasicVarTextBox ()
{
}

CItemImage*	CBasicVarTextBox::CreateItemImage ( const char* szControl )
{
	CItemImage* pItemImage = new CItemImage;
	pItemImage->CreateSub ( this, szControl, UI_FLAG_DEFAULT );
	pItemImage->CreateSubControl ();
	pItemImage->SetUseRender ( TRUE );
	pItemImage->SetVisibleSingle ( FALSE );
	RegisterControl ( pItemImage );

	return pItemImage;
}

void CBasicVarTextBox::CreateSubControl ()
{
	m_pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );
	m_pFont12 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 12,  _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBoxEx* pLineBox = new CBasicLineBoxEx;
	pLineBox->CreateSub ( this, "BASIC_VAR_LINE_BOX", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxVarTextBox ( "BASIC_VAR_LINE_BOX" );
	pLineBox->SetNoUpdate ( true );
	RegisterControl ( pLineBox );
	m_pLineBox = pLineBox;

	CUIControl* pSelfDummy = new CUIControl;
	pSelfDummy->CreateSub ( this, "BASIC_VAR_LINE_BOX" );
	pSelfDummy->SetVisibleSingle ( FALSE );
	RegisterControl ( pSelfDummy );
	m_pSelfDummy = pSelfDummy;

	CItemImage* pItemImage = new CItemImage;
	pItemImage->CreateSub ( this, "ITEM_IMAGE_TEXTURE" );
	pItemImage->CreateSubControl ();
	pItemImage->SetUseRender ( TRUE );
	pItemImage->SetVisibleSingle ( FALSE );
	RegisterControl ( pItemImage );
	m_pItemImage = pItemImage;

	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, "VAR_TEXT_BOX", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pTextBox->SetFont ( m_pFont );
	pTextBox->SetTextAlign ( TEXT_ALIGN_LEFT );
	pTextBox->SetNoUpdate ( true );
	RegisterControl ( pTextBox );
	m_pTextBox = pTextBox;
	m_pTextBox->ClearText();

	CBasicTextBox* pTextBoxName = new CBasicTextBox;
	pTextBoxName->CreateSub ( this, "VAR_TEXT_BOX", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pTextBoxName->SetFont ( m_pFont12 );
	pTextBoxName->SetTextAlign ( TEXT_ALIGN_LEFT );
	pTextBoxName->SetNoUpdate ( true );
	RegisterControl ( pTextBoxName );
	m_pTextBoxName = pTextBoxName;
	m_pTextBoxName->ClearText();
}

void CBasicVarTextBox::SetTextAlign ( int nALIGN )
{
	m_pTextBox->SetTextAlign ( nALIGN );
}

void CBasicVarTextBox::ClearText ()
{
	if ( m_pTextBox ) m_pTextBox->ClearText ();
	if ( m_pTextBoxName ) m_pTextBoxName->ClearText ();
}

int CBasicVarTextBox::AddTextNoSplit ( CString strText, D3DCOLOR dwColor )
{
	if ( m_pTextBox ) return m_pTextBox->AddTextNoSplit ( strText, dwColor );

	return -1;
}

int	CBasicVarTextBox::SetTextNoSplit ( CString strText, D3DCOLOR dwColor )
{
	if ( m_pTextBox ) return m_pTextBox->SetTextNoSplit ( strText, dwColor );

	return -1;
}

void CBasicVarTextBox::SetText( CString strText, D3DCOLOR dwColor )
{
	if ( m_pTextBox ) m_pTextBox->SetText( strText, dwColor );
}

void CBasicVarTextBox::AddString ( int nIndex, const CString& strText, const D3DCOLOR& dwColor )
{
	if ( m_pTextBox ) m_pTextBox->AddString ( nIndex, strText, dwColor );
}

int CBasicVarTextBox::AddText ( CString strText, D3DCOLOR dwColor )
{
	if ( m_pTextBox ) return m_pTextBox->AddText ( strText, dwColor );

	return -1;
}

void CBasicVarTextBox::SetUseOverColor ( bool bUseColor )
{
	m_pTextBox->SetUseOverColor ( (bUseColor)?TRUE:FALSE );
}

void CBasicVarTextBox::SetOverColor ( const D3DCOLOR& dwColor )
{
	int nIndex = m_pTextBox->GetCount () - 1;
	m_pTextBox->SetOverColor ( nIndex, dwColor );
}

void CBasicVarTextBox::RePosControl ( int x, int y )
{
	const float fTextBoxGapX_ = (float)m_pTextBox->GetLongestLine ();
	const float fTextBoxGapXName_ = (float)m_pTextBoxName->GetLongestLine ();

	if ( fTextBoxGapXName_ > fTextBoxGapX_ )
	{
		SetLong( TRUE );
		const UIRECT& rcTextBoxLocalPos = m_pTextBoxName->GetLocalPos ();
		const UIRECT& rcLocalPosDummy = m_pSelfDummy->GetLocalPos ();
		const UIRECT& rcOriginPos = GetLocalPos ();

		AlignSubControl ( rcOriginPos, rcLocalPosDummy );

		if ( m_pTextBoxName )
		{
			int nTotalLine = m_pTextBox->GetTotalLine ();
			const float fAllLine = m_pTextBox->CalcMaxHEIGHT ( 0, nTotalLine );
			const float fTextBoxGapX = (float)m_pTextBoxName->GetLongestLine () - rcTextBoxLocalPos.sizeX;
			float fTextBoxGapY(0.0f);
			fTextBoxGapY = fAllLine - rcTextBoxLocalPos.sizeY;
			
			UIRECT rcLocalNewPos = UIRECT ( float(x) + m_vMousePointGap.x, float(y) + m_vMousePointGap.y,
				rcOriginPos.sizeX + fTextBoxGapX, rcOriginPos.sizeY + fTextBoxGapY );

			AlignSubControl ( rcLocalPosDummy, rcLocalNewPos );

			CUIGroup::SetGlobalPos ( rcLocalNewPos );	
		}
	}
	else
	{
		SetLong(FALSE);
		const UIRECT& rcTextBoxLocalPos = m_pTextBox->GetLocalPos ();
		const UIRECT& rcLocalPosDummy = m_pSelfDummy->GetLocalPos ();
		const UIRECT& rcOriginPos = GetLocalPos ();

		AlignSubControl ( rcOriginPos, rcLocalPosDummy );

		if ( m_pTextBox )
		{
			int nTotalLine = m_pTextBox->GetTotalLine ();
			const float fAllLine = m_pTextBox->CalcMaxHEIGHT ( 0, nTotalLine );
			const float fTextBoxGapX = (float)m_pTextBox->GetLongestLine () - rcTextBoxLocalPos.sizeX;
			float fTextBoxGapY(0.0f);
			fTextBoxGapY = fAllLine - rcTextBoxLocalPos.sizeY;
			
			UIRECT rcLocalNewPos = UIRECT ( float(x) + m_vMousePointGap.x, float(y) + m_vMousePointGap.y,
				rcOriginPos.sizeX + fTextBoxGapX , rcOriginPos.sizeY + fTextBoxGapY );

			AlignSubControl ( rcLocalPosDummy, rcLocalNewPos );

			CUIGroup::SetGlobalPos ( rcLocalNewPos );
		}
	}
}

void CBasicVarTextBox::SetGlobalPos( const D3DXVECTOR2 &vPos )
{
	CUIGroup::SetGlobalPos( vPos );
}

void CBasicVarTextBox::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	SetUseOverColor ( false );
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( !m_bBLOCK_MOUSETRACKING ) RePosControl ( x, y );	
}

void CBasicVarTextBox::SetMousePointGap ( D3DXVECTOR2 vGap )
{
	m_vMousePointGap = vGap;
}

float CBasicVarTextBox::GetLongestLine ()
{
	float l1 = m_pTextBox->GetLongestLine ();
	float l2 = m_pTextBoxName->GetLongestLine ();
	if ( m_pTextBox || m_pTextBoxName )
	{
		if ( l1 > l2 )
			return (float)m_pTextBox->GetLongestLine ();
		else
			return (float)m_pTextBoxName->GetLongestLine ();
	}
	return 0.0f;
}

void CBasicVarTextBox::AddTextLongestLineSplit ( CString strText, D3DCOLOR dwColor )
{
	//float fLongestLine = GetLongestLine ();
	//float fLongLine = float(m_pTextBoxName->GetLongestLine()) + 40.0f;

//	if ( IsLong())
//	if ( fLongLine > fLongestLine )
	const float fTextBoxGapX_ = (float)m_pTextBox->GetLongestLine ();
	const float fTextBoxGapXName_ = (float)m_pTextBoxName->GetLongestLine ();

	if ( fTextBoxGapXName_ > fTextBoxGapX_ )
	{
		NS_UITEXTCONTROL::MULTILINETEXT& MLTextSet = NS_UITEXTCONTROL::GetMLTextWithoutCRLF ( strText, fTextBoxGapXName_, m_pFont12 );

		for ( int i = 0; i < (int)MLTextSet.size(); ++i )
		{
			CString strTemp = MLTextSet[i].strLine;
			if ( strTemp.GetLength() ) AddTextNoSplit ( strTemp, dwColor );		
		}	
	}
	else
	{
		NS_UITEXTCONTROL::MULTILINETEXT& MLTextSet = NS_UITEXTCONTROL::GetMLTextWithoutCRLF ( strText, fTextBoxGapX_, m_pFont );

		for ( int i = 0; i < (int)MLTextSet.size(); ++i )
		{
			CString strTemp = MLTextSet[i].strLine;
			if ( strTemp.GetLength() ) AddTextNoSplit ( strTemp, dwColor );		
		}
	}
}

int	CBasicVarTextBox::GetCount ()
{
	return m_pTextBox->GetCount ();
}

void CBasicVarTextBox::SetLineInterval ( const float fLineInterval )
{
	m_pTextBox->SetLineInterval ( fLineInterval );
}

void CBasicVarTextBox::SetBlockMouseTracking ( bool bBlock )
{
	m_bBLOCK_MOUSETRACKING = bBlock;
}
//Item Name
int CBasicVarTextBox::AddTextNoSplitBig ( CString strText, D3DCOLOR dwColor )
{
	if ( m_pTextBoxName ) return m_pTextBoxName->AddTextNoSplitBig ( strText, dwColor );
								 m_pTextBoxName->SetFont ( m_pFont12 );
	return -1;
}

//Add MultiText by NaJ
int	CBasicVarTextBox::AddMultiText ( CBasicTextBox::MULTITEXT mlText )
{
	if ( m_pTextBox ) return m_pTextBox->AddMultiText( mlText );

	return -1;
}

//Add MultiTextLongestLine by NaJ
int	CBasicVarTextBox::AddMultiTextLongestLineSplit ( CBasicTextBox::MULTITEXT mlText )
{
	if ( m_pTextBox ) return m_pTextBox->AddMultiTextLongestLine( mlText );

	return -1;
}