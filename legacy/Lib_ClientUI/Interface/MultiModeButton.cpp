#include "StdAfx.h"
#include "MultiModeButton.h"
#include "MultiModeButtonImage.h"
#include "BasicTextBox.h"
#include "UIKeyCheck.h"
#include "UITextControl.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const float CMultiModeButton::fBUTTON_CLICK = 1.0f;

CMultiModeButton::CMultiModeButton()
	: m_pTextBox(NULL)
	, m_pImageBase(NULL)
	, m_pImageFlip(NULL)
	, m_pImageHold(NULL)
	, m_pImageOver(NULL)
	, m_pImageBaseMulti(NULL)
	, m_pImageFlipMulti(NULL)
	, m_pImageHoldMulti(NULL)
	, m_pImageOverMulti(NULL)
	, m_bActionFlip(FALSE)
	, m_bActionHold(FALSE)
	, m_bActionOver(FALSE)
	, m_bUseActionFlip(FALSE)
	, m_bUseActionDown(FALSE)
	, m_bUseGlobalAction(FALSE)
	, m_KeyCode(NULL)
	, m_KeyCodeEx(NULL)
	, m_dwTextColorFlip(NS_UITEXTCOLOR::BLACK)
	, m_dwTextColorHold(NS_UITEXTCOLOR::BLACK)
	, m_dwTextColorOver(NS_UITEXTCOLOR::BLACK)
{
}

CMultiModeButton::~CMultiModeButton()
{
}

void CMultiModeButton::CreateImageBase( const char* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl );
	pControl->SetVisibleSingle ( TRUE );
	RegisterControl ( pControl );
	m_pImageBase = pControl;
}

void CMultiModeButton::CreateImageFlip( const char* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl );
	pControl->SetVisibleSingle ( FALSE );
	RegisterControl ( pControl );
	m_pImageFlip = pControl;
}

void CMultiModeButton::CreateImageHold( const char* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl );
	pControl->SetVisibleSingle ( FALSE );
	RegisterControl ( pControl );
	m_pImageHold = pControl;
}

void CMultiModeButton::CreateImageOver( const char* szControl )
{	
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl );
	pControl->SetVisibleSingle ( FALSE );
	RegisterControl ( pControl );
	m_pImageOver = pControl;
}

void CMultiModeButton::CreateImageBaseMulti( const char* szControl, const char* szLeft, const char* szCenter, const char* szRight )
{
	CMultiModeButtonImage* pControl = new CMultiModeButtonImage;
	pControl->CreateSub ( this, szControl, UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pControl->CreateButtonImage( szLeft, szCenter, szRight );
	pControl->SetVisibleSingle ( TRUE );
	RegisterControl ( pControl );
	m_pImageBaseMulti = pControl;
}

void CMultiModeButton::CreateImageFlipMulti( const char* szControl, const char* szLeft, const char* szCenter, const char* szRight )
{
	CMultiModeButtonImage* pControl = new CMultiModeButtonImage;
	pControl->CreateSub ( this, szControl, UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pControl->CreateButtonImage( szLeft, szCenter, szRight );
	pControl->SetVisibleSingle ( FALSE );
	RegisterControl ( pControl );
	m_pImageFlipMulti = pControl;
}

void CMultiModeButton::CreateImageHoldMulti( const char* szControl, const char* szLeft, const char* szCenter, const char* szRight )
{
	CMultiModeButtonImage* pControl = new CMultiModeButtonImage;
	pControl->CreateSub ( this, szControl, UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pControl->CreateButtonImage( szLeft, szCenter, szRight );
	pControl->SetVisibleSingle ( FALSE );
	RegisterControl ( pControl );
	m_pImageHoldMulti = pControl;
}

void CMultiModeButton::CreateImageOverMulti( const char* szControl, const char* szLeft, const char* szCenter, const char* szRight )
{	
	CMultiModeButtonImage* pControl = new CMultiModeButtonImage;
	pControl->CreateSub ( this, szControl, UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pControl->CreateButtonImage( szLeft, szCenter, szRight );
	pControl->SetVisibleSingle ( FALSE );
	RegisterControl ( pControl );
	m_pImageOverMulti = pControl;
}

void CMultiModeButton::CreateTextBox( const char* szControl, int nTextSize, int nTextAlign, DWORD dwFlags )
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, nTextSize, dwFlags );

	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, szControl, UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pTextBox->SetFont ( pFont );
	pTextBox->SetTextAlign ( nTextAlign );	
	RegisterControl ( pTextBox );
	m_pTextBox = pTextBox;
}

void CMultiModeButton::AlignToControl( const char* szControl )
{
	CUIControl TempControl;
	TempControl.Create ( 1, szControl );
	const UIRECT& rcParentOldPos = GetLocalPos ();
	const UIRECT& rcParentNewPos = TempControl.GetLocalPos ();
	AlignSubControl ( rcParentOldPos, rcParentNewPos );
	SetLocalPos ( D3DXVECTOR2 ( rcParentNewPos.left, rcParentNewPos.top ) );
}

void CMultiModeButton::SetTextColor( DWORD dwFlip, DWORD dwHold, DWORD dwOver )
{
	m_dwTextColorFlip = dwFlip;
	m_dwTextColorHold = dwHold;
	m_dwTextColorOver = dwOver;
}

void CMultiModeButton::SetOneLineText ( CString strTemp, DWORD dwColor )
{
	if ( m_pTextBox ) 
		m_pTextBox->SetOneLineText ( strTemp, dwColor );
}

void CMultiModeButton::DoImageFlip( BOOL bSHOW )
{	
	if ( bSHOW ){
		DoImageHold( FALSE );
		DoImageOver( FALSE );
	}

	m_bActionFlip = bSHOW;

	if ( m_pImageFlip )	
		m_pImageFlip->SetVisibleSingle( m_bActionFlip );

	if ( m_pImageFlipMulti )
		m_pImageFlipMulti->SetVisibleSingle( m_bActionFlip );

	if( m_pTextBox && m_bActionFlip ){
		m_pTextBox->SetUseOverColor( TRUE );
		m_pTextBox->SetOverColor ( 0, m_dwTextColorFlip );
	}
}

void CMultiModeButton::DoImageHold( BOOL bSHOW )
{	
	if ( bSHOW )
		DoImageOver( FALSE );

	if ( m_bActionFlip )	return;

	m_bActionHold = bSHOW;

	if ( m_pImageHold )	
		m_pImageHold->SetVisibleSingle( m_bActionHold );

	if ( m_pImageHoldMulti )	
		m_pImageHoldMulti->SetVisibleSingle( m_bActionHold );

	if( m_pTextBox && m_bActionHold ){
		m_pTextBox->SetUseOverColor( TRUE );
		m_pTextBox->SetOverColor ( 0, m_dwTextColorHold );
	}
}

void CMultiModeButton::DoImageOver( BOOL bSHOW )
{	
	if ( m_bActionFlip )	return;
	if ( m_bActionHold )	return;

	m_bActionOver = bSHOW;

	if ( m_pImageOver )	
		m_pImageOver->SetVisibleSingle( m_bActionOver );

	if ( m_pImageOverMulti )	
		m_pImageOverMulti->SetVisibleSingle( m_bActionOver );

	if( m_pTextBox && m_bActionOver ){
		m_pTextBox->SetUseOverColor( TRUE );
		m_pTextBox->SetOverColor ( 0, m_dwTextColorOver );
	}
}

void CMultiModeButton::DoImageDown( BOOL bSHOW )
{	
	if ( m_bActionFlip )	return;
	if ( !m_bUseActionDown )	return;

	UIRECT rcGlobalPos = GetGlobalPos ();
	if ( bSHOW ){
		rcGlobalPos.left = rcGlobalPos.left + fBUTTON_CLICK;
		rcGlobalPos.top = rcGlobalPos.top + fBUTTON_CLICK;
	}

	if ( m_pImageBase )
		m_pImageBase->SetGlobalPos ( D3DXVECTOR2 ( rcGlobalPos.left, rcGlobalPos.top ) );

	if ( m_pImageFlip )
		m_pImageFlip->SetGlobalPos ( D3DXVECTOR2 ( rcGlobalPos.left, rcGlobalPos.top ) );

	if ( m_pImageHold )
		m_pImageHold->SetGlobalPos ( D3DXVECTOR2 ( rcGlobalPos.left, rcGlobalPos.top ) );

	if ( m_pImageOver )
		m_pImageOver->SetGlobalPos ( D3DXVECTOR2 ( rcGlobalPos.left, rcGlobalPos.top ) );

	if ( m_pImageBaseMulti )
		m_pImageBaseMulti->SetGlobalPos ( D3DXVECTOR2 ( rcGlobalPos.left, rcGlobalPos.top ) );

	if ( m_pImageFlipMulti )
		m_pImageFlipMulti->SetGlobalPos ( D3DXVECTOR2 ( rcGlobalPos.left, rcGlobalPos.top ) );

	if ( m_pImageHoldMulti )
		m_pImageHoldMulti->SetGlobalPos ( D3DXVECTOR2 ( rcGlobalPos.left, rcGlobalPos.top ) );

	if ( m_pImageOverMulti )
		m_pImageOverMulti->SetGlobalPos ( D3DXVECTOR2 ( rcGlobalPos.left, rcGlobalPos.top ) );

	if ( m_pTextBox )
		m_pTextBox->SetGlobalPos ( D3DXVECTOR2 ( rcGlobalPos.left, rcGlobalPos.top ) );
}

void CMultiModeButton::TranslateMouseMessage ( DWORD dwMsg )
{	
	BOOL bMouseOver = FALSE;
	if ( CHECK_MOUSE_IN ( dwMsg ) )	bMouseOver = TRUE;
	DoImageOver( bMouseOver );

	BOOL bMouseHold = FALSE;
	if ( CHECK_MOUSE_IN_LBDOWNLIKE ( dwMsg ) )	bMouseHold = TRUE;
	DoImageHold( bMouseHold );
	DoImageDown( bMouseHold );

	if ( CHECK_MOUSE_IN ( dwMsg ) && CHECK_LB_UP_LIKE ( dwMsg ) && IsUseActionFlip() ){
		DoImageFlip( TRUE );
	}
}

void CMultiModeButton::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if( m_pTextBox && !m_bActionFlip ){
		m_pTextBox->SetUseOverColor( FALSE );
	}

	DoImageDown( FALSE );

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	DoImageHold( FALSE );
	DoImageOver( FALSE );

	if ( bFirstControl )
	{		
		CUIControl* pParent = GetTopParent ();
		BOOL bFocusControl = ( pParent && pParent->IsFocusControl() ) ? TRUE : FALSE;
		if ( bFocusControl || m_bUseGlobalAction )
		{
			TranslateMouseMessage ( GetMessageEx () );
		}

		if ( bFocusControl )
		{
			if ( m_KeyCode || m_KeyCodeEx )
			{
				if ( UIKeyCheck::GetInstance()->Check ( m_KeyCode, DXKEY_DOWN ) )
				{
					AddMessageEx ( UIMSG_KEY_FOCUSED );
				}

				if ( UIKeyCheck::GetInstance()->Check ( m_KeyCodeEx, DXKEY_DOWN ) )
				{
					AddMessageEx ( UIMSG_KEY_FOCUSED );
				}
			}
		}
	}
}
