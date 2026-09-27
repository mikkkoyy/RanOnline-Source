#include "StdAfx.h"
#include "ItemColorWindow.h"
#include "ItemColorSelectSlot.h"
#include "InnerInterface.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicProgressBar.h"
#include "UIEditBox.h"
#include "BasicTextButton.h"

#include "UITextControl.h"
#include "GameTextControl.h"
#include "../Lib_Engine/DxCommon/d3dfont.h"

#include "GLGaeaClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


const int CItemColorWindow::nLIMIT_NUMBER_CHAR = 2;
const int CItemColorWindow::nMAX_COLOR = 31;
const float CItemColorWindow::fRGB_RATIO = 8.225806f;

CItemColorWindow::CItemColorWindow(void)
	: m_pColor1(NULL)
	, m_pColor2(NULL)
	, m_pText1(NULL)
	, m_pText2(NULL)
	, m_pRProgressBar(NULL)
	, m_pGProgressBar(NULL)
	, m_pBProgressBar(NULL)
	, m_pREditBox(NULL)
	, m_pGEditBox(NULL)
	, m_pBEditBox(NULL)
	, m_pButtonSave(NULL)
	, m_pButtonClose(NULL)
	, m_bPrimary(FALSE)
	, m_nR(0)
	, m_nG(0)
	, m_nB(0)
	, m_nRBack(0)
	, m_nGBack(0)
	, m_nBBack(0)
	, m_nRegR(0)
	, m_nRegG(0)
	, m_nRegB(0)
{
}

CItemColorWindow::~CItemColorWindow(void)
{
}

void CItemColorWindow::CreateSubControl()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "RAN_ITEM_COLOR_WINDOW_LINEBOX" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub( this, "BASIC_LINE_BOX_EDIT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxEditBox( "RAN_ITEM_COLOR_R_EDITBOX_BACK" );
	RegisterControl( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub( this, "BASIC_LINE_BOX_EDIT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxEditBox( "RAN_ITEM_COLOR_G_EDITBOX_BACK" );
	RegisterControl( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub( this, "BASIC_LINE_BOX_EDIT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxEditBox( "RAN_ITEM_COLOR_B_EDITBOX_BACK" );
	RegisterControl( pBasicLineBox );

	m_pColor1 = new CItemColorSelectSlot;
	m_pColor1->CreateSub( this, "RAN_ITEM_COLOR_FOCUS_0", UI_FLAG_XSIZE | UI_FLAG_YSIZE, ITEM_COLOR_SLOT_1 );
	m_pColor1->CreateSubControl ( TRUE );
	RegisterControl ( m_pColor1 );

	m_pColor2 = new CItemColorSelectSlot;
	m_pColor2->CreateSub( this, "RAN_ITEM_COLOR_FOCUS_1", UI_FLAG_XSIZE | UI_FLAG_YSIZE, ITEM_COLOR_SLOT_2 );
	m_pColor2->CreateSubControl ( FALSE );
	RegisterControl ( m_pColor2 );

	m_pText1 = new CBasicTextBox;
	m_pText1->CreateSub ( this, "RAN_ITEM_COLOR_TEXT_0" );
	m_pText1->SetFont ( pFont9 );
	m_pText1->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pText1->SetText( ID2GAMEWORD( "RAN_ITEM_COLOR_WINDOW", 1 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pText1 );

	m_pText2 = new CBasicTextBox;
	m_pText2->CreateSub ( this, "RAN_ITEM_COLOR_TEXT_1" );
	m_pText2->SetFont ( pFont9 );
	m_pText2->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pText2->SetText( ID2GAMEWORD( "RAN_ITEM_COLOR_WINDOW", 2 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pText2 );

	CUIControl* pControl = new CUIControl;
	pControl->CreateSub( this, "RAN_ITEM_COLOR_R" );
	RegisterControl( pControl );

	pControl = new CUIControl;
	pControl->CreateSub( this, "RAN_ITEM_COLOR_G" );
	RegisterControl( pControl );

	pControl = new CUIControl;
	pControl->CreateSub( this, "RAN_ITEM_COLOR_B" );
	RegisterControl( pControl );


	m_pRProgressBar = new CBasicProgressBar;
	m_pRProgressBar->CreateSub ( this, "RAN_ITEM_COLOR_R_PROGRESS" );
	m_pRProgressBar->CreateOverImage ( "RAN_ITEM_COLOR_R_PROGRESS_OVERIMAGE" );
	RegisterControl ( m_pRProgressBar );

	m_pGProgressBar = new CBasicProgressBar;
	m_pGProgressBar->CreateSub ( this, "RAN_ITEM_COLOR_G_PROGRESS" );
	m_pGProgressBar->CreateOverImage ( "RAN_ITEM_COLOR_G_PROGRESS_OVERIMAGE" );
	RegisterControl ( m_pGProgressBar );

	m_pBProgressBar = new CBasicProgressBar;
	m_pBProgressBar->CreateSub ( this, "RAN_ITEM_COLOR_B_PROGRESS" );
	m_pBProgressBar->CreateOverImage ( "RAN_ITEM_COLOR_B_PROGRESS_OVERIMAGE" );
	RegisterControl ( m_pBProgressBar );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_ITEM_COLOR_R_PROGRESS_DUMMY", UI_FLAG_DEFAULT, ITEM_COLOR_SELECT_R_PROGRESS );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_ITEM_COLOR_G_PROGRESS_DUMMY", UI_FLAG_DEFAULT, ITEM_COLOR_SELECT_G_PROGRESS );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_ITEM_COLOR_B_PROGRESS_DUMMY", UI_FLAG_DEFAULT, ITEM_COLOR_SELECT_B_PROGRESS );
	RegisterControl ( pControl );

	m_pREditBox = new CUIEditBox;
	m_pREditBox->CreateSub( this, "RAN_ITEM_COLOR_R_EDITBOX", UI_FLAG_XSIZE | UI_FLAG_RIGHT, ITEM_COLOR_SELECT_R_EDITBOX );
	m_pREditBox->CreateCarrat( "MODAL_CARRAT", TRUE, UINT_MAX );
	m_pREditBox->SetLimitInput( nLIMIT_NUMBER_CHAR );
	m_pREditBox->SetFont( pFont9 );
	m_pREditBox->DoUSE_NUMBER( true );
	RegisterControl( m_pREditBox );

	m_pGEditBox = new CUIEditBox;
	m_pGEditBox->CreateSub( this, "RAN_ITEM_COLOR_G_EDITBOX", UI_FLAG_XSIZE | UI_FLAG_RIGHT, ITEM_COLOR_SELECT_G_EDITBOX );
	m_pGEditBox->CreateCarrat( "MODAL_CARRAT", TRUE, UINT_MAX );
	m_pGEditBox->SetLimitInput( nLIMIT_NUMBER_CHAR );
	m_pGEditBox->SetFont( pFont9 );	
	m_pGEditBox->DoUSE_NUMBER( true );
	RegisterControl( m_pGEditBox );

	m_pBEditBox = new CUIEditBox;
	m_pBEditBox->CreateSub( this, "RAN_ITEM_COLOR_B_EDITBOX", UI_FLAG_XSIZE | UI_FLAG_RIGHT, ITEM_COLOR_SELECT_B_EDITBOX );
	m_pBEditBox->CreateCarrat( "MODAL_CARRAT", TRUE, UINT_MAX );
	m_pBEditBox->SetLimitInput( nLIMIT_NUMBER_CHAR );
	m_pBEditBox->SetFont( pFont9 );
	m_pBEditBox->DoUSE_NUMBER( true );
	RegisterControl( m_pBEditBox );

	m_pButtonSave = new CBasicTextButton;
	m_pButtonSave->CreateSub ( this, "BASIC_TEXT_BUTTON14", UI_FLAG_XSIZE, ITEM_COLOR_SELECT_SAVE );
	m_pButtonSave->CreateBaseButton ( "RAN_ITEM_COLOR_OK", CBasicTextButton::SIZE14, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_ITEM_COLOR_WINDOW", 4) );
	RegisterControl ( m_pButtonSave );

	m_pButtonClose = new CBasicTextButton;
	m_pButtonClose->CreateSub ( this, "BASIC_TEXT_BUTTON14", UI_FLAG_XSIZE, ITEM_COLOR_SELECT_CLOSE );
	m_pButtonClose->CreateBaseButton ( "RAN_ITEM_COLOR_CANCEL", CBasicTextButton::SIZE14, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_ITEM_COLOR_WINDOW", 3) );
	RegisterControl ( m_pButtonClose );

}

void CItemColorWindow::Update ( int x ,int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if( bFirstControl )
	{
		BOOL bChange( FALSE );

		if( UpdateProgressBar( m_pRProgressBar, x ) )
		{
			UpdateProgressBarToEditBox( m_pRProgressBar, m_pREditBox, m_nR );
			bChange = TRUE;
		}
		else if( UpdateProgressBar( m_pGProgressBar, x ) )
		{
			UpdateProgressBarToEditBox( m_pGProgressBar, m_pGEditBox, m_nG );
			bChange = TRUE;
		}
		else if( UpdateProgressBar( m_pBProgressBar, x ) )
		{
			UpdateProgressBarToEditBox( m_pBProgressBar, m_pBEditBox, m_nB );
			bChange = TRUE;
		}

		if( UpdateEditBoxToProgressBar( m_pREditBox, m_pRProgressBar, m_nR ) )		bChange = TRUE;
		else if( UpdateEditBoxToProgressBar( m_pGEditBox, m_pGProgressBar, m_nG ) )	bChange = TRUE;
		else if( UpdateEditBoxToProgressBar( m_pBEditBox, m_pBProgressBar, m_nB ) )	bChange = TRUE;

		if( bChange )
		{
			RegulateRGB();

			GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
			if ( pCharacter && pCharacter->m_bEnableItemColor )
			{
				WORD wTemp = m_nR << 10	| m_nG << 5 | m_nB;

				if ( m_bPrimary )
					pCharacter->m_wItemColor1 = wTemp;
				else
					pCharacter->m_wItemColor2 = wTemp;

				pCharacter->ItemColorUpdate();
			}			
		}
	}
}

void CItemColorWindow::TranslateUIMessage( UIGUID cID, DWORD dwMsg )
{
	CUIWindowEx::TranslateUIMessage( cID, dwMsg );

	switch ( cID )
	{
	case ET_CONTROL_TITLE:
	case ET_CONTROL_TITLE_F:
		{
			if ( (dwMsg & UIMSG_LB_DUP) && CHECK_MOUSE_IN ( dwMsg ) ){
				CInnerInterface::GetInstance().SetDefaultPosInterface( GetWndID() );
			}
		}break;

	case ET_CONTROL_BUTTON:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			{
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
			}
		}break;

	case ITEM_COLOR_SLOT_1:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				SelectSlot( TRUE );
			}
		}break;

	case ITEM_COLOR_SLOT_2:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				SelectSlot( FALSE );
			}
		}break;

	case ITEM_COLOR_SELECT_R_PROGRESS: TranslateProgressBarMsg( m_pRProgressBar, dwMsg ); break;
	case ITEM_COLOR_SELECT_G_PROGRESS: TranslateProgressBarMsg( m_pGProgressBar, dwMsg ); break;
	case ITEM_COLOR_SELECT_B_PROGRESS: TranslateProgressBarMsg( m_pBProgressBar, dwMsg ); break;

	case ITEM_COLOR_SELECT_R_EDITBOX: TranslateEditBoxMsg( m_pREditBox, dwMsg ); break;
	case ITEM_COLOR_SELECT_G_EDITBOX: TranslateEditBoxMsg( m_pGEditBox, dwMsg ); break;
	case ITEM_COLOR_SELECT_B_EDITBOX: TranslateEditBoxMsg( m_pBEditBox, dwMsg ); break;

	case ITEM_COLOR_SELECT_CLOSE:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
			}
		}break;

	case ITEM_COLOR_SELECT_SAVE:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				GLGaeaClient::GetInstance().GetCharacter()->ItemColorChange();
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
			}
		}break;

	};

}

void CItemColorWindow::SetVisibleSingle( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		SelectSlot( TRUE );
	}
	else
	{
		ClearEditBox();
		GLGaeaClient::GetInstance().GetCharacter()->ItemColorEnd();
	}
}

void CItemColorWindow::SelectSlot( BOOL bPrimary )
{
	if ( m_pColor1 )	m_pColor1->SetSelect( bPrimary );
	if ( m_pColor2 )	m_pColor2->SetSelect( !bPrimary );

	m_bPrimary = bPrimary;

	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pCharacter )	return;
	if ( !pCharacter->m_bEnableItemColor )	return;

	WORD wColor = pCharacter->m_wItemColor1;
	if ( !bPrimary )
		wColor = pCharacter->m_wItemColor2;

	m_nRBack = m_nR = (wColor & 0x7c00) >> 10;
	m_nGBack = m_nG = (wColor & 0x3e0) >> 5;
	m_nBBack = m_nB = (wColor & 0x1f);

	CString strTemp;

	strTemp.Format( "%d", m_nR );
	m_pREditBox->SetEditString( strTemp );

	strTemp.Format( "%d", m_nG );
	m_pGEditBox->SetEditString( strTemp );

	strTemp.Format( "%d", m_nB );
	m_pBEditBox->SetEditString( strTemp );

	float fPercent(0);
	if( m_nR > 0 )	fPercent = m_nR / float(nMAX_COLOR);
	m_pRProgressBar->SetPercent( fPercent );

	fPercent = 0;
	if( m_nG > 0 )	fPercent = m_nG / float(nMAX_COLOR);
	m_pGProgressBar->SetPercent( fPercent );

	fPercent = 0;
	if( m_nB > 0 )	fPercent = m_nB / float(nMAX_COLOR);
	m_pBProgressBar->SetPercent( fPercent );

	RegulateRGB();
}

BOOL CItemColorWindow::UpdateProgressBar( CBasicProgressBar * pBar, int x )
{
	if( pBar && pBar->IsExclusiveSelfControl() )
	{
		const float fSize = pBar->GetGlobalPos ().sizeX;
		const float fPos = float(x) - pBar->GetGlobalPos ().left;
		const float fPercent = fPos / fSize;
		pBar->SetPercent ( fPercent );

		return TRUE;
	}

	return FALSE;
}

void CItemColorWindow::UpdateProgressBarToEditBox( CBasicProgressBar * pBar, CUIEditBox * pEditBox, INT & nColorVal )
{
	int nVal(0);
	float fPercent = pBar->GetPercent();
	if( fPercent > 0 )	nVal = (int)(nMAX_COLOR * fPercent);

	CString strTemp;
	strTemp.Format( "%d", nVal );

	pEditBox->ClearEdit();
	pEditBox->SetEditString( strTemp );

	nColorVal = nVal;
}

BOOL CItemColorWindow::UpdateEditBoxToProgressBar( CUIEditBox * pEditBox, CBasicProgressBar * pBar, INT & nColorVal )
{
	if( pEditBox && pEditBox->IsBegin() )
	{
		const CString & strRVal = pEditBox->GetEditString();
		nColorVal = _ttoi( strRVal.GetString() );
		if( nColorVal > nMAX_COLOR )
		{
			nColorVal = nMAX_COLOR;

			pEditBox->ClearEdit();
			pEditBox->SetEditString( CString("31") );
		}

		float fPercent(0);
		if( nColorVal > 0 )	fPercent = nColorVal / float(nMAX_COLOR);
		pBar->SetPercent( fPercent );

		return TRUE;
	}

	return FALSE;
}

void CItemColorWindow::TranslateProgressBarMsg( CBasicProgressBar * pBar, DWORD dwMsg )
{
	if( !pBar ) return;

	if ( CHECK_MOUSE_IN ( dwMsg ) )
	{
		if ( CHECK_LB_DOWN_LIKE ( dwMsg ) )
		{			
			pBar->SetExclusiveControl();
		}
		else if ( CHECK_LB_UP_LIKE ( dwMsg ) )
		{
			pBar->ResetExclusiveControl();
		}
	}
	else if ( CHECK_LB_UP_LIKE ( dwMsg ) )
	{									
		pBar->ResetExclusiveControl();
	}
}

void CItemColorWindow::TranslateEditBoxMsg( CUIEditBox * pEditBox, DWORD dwMsg )
{
	if( !pEditBox ) return;

	if( CHECK_MOUSE_IN_LBDOWNLIKE( dwMsg ) )
	{
		pEditBox->BeginEdit();
	}
	else if( CHECK_LB_DOWN_LIKE( dwMsg ) )
	{
		pEditBox->EndEdit();
	}
}

void CItemColorWindow::RegulateRGB()
{
	m_nRegR = int(m_nR * fRGB_RATIO);
	m_nRegG = int(m_nG * fRGB_RATIO);
	m_nRegB = int(m_nB * fRGB_RATIO);
}

void CItemColorWindow::ClearEditBox()
{
	m_pREditBox->ClearEdit();
	m_pREditBox->EndEdit();

	m_pGEditBox->ClearEdit();
	m_pGEditBox->EndEdit();

	m_pBEditBox->ClearEdit();
	m_pBEditBox->EndEdit();

	m_bPrimary = FALSE;
}