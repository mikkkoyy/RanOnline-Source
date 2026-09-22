#include "StdAfx.h"
#include "BikeColorSelect.h"
#include "BikeColorSelectSlot.h"

#include "BasicTextBox.h"
#include "BasicLineBox.h"
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

const int CBikeColorSelect::nLIMIT_NUMBER_CHAR = 2;
const int CBikeColorSelect::nMAX_COLOR = 31;
const float CBikeColorSelect::fRGB_RATIO = 8.225806f;

CBikeColorSelect::CBikeColorSelect ()
	: m_emSelected( BIKE_COLOR_SLOT_PART_SIZE )
	, m_pRProgressBar(NULL)
	, m_pGProgressBar(NULL)
	, m_pBProgressBar(NULL)
	, m_pREditBox(NULL)
	, m_pGEditBox(NULL)
	, m_pBEditBox(NULL)
	, m_nR(0)
	, m_nG(0)
	, m_nB(0)
	, m_nRBack(0)
	, m_nGBack(0)
	, m_nBBack(0)
	, m_nRegR(0)
	, m_nRegG(0)
	, m_nRegB(0)
	, m_pButtonSave(NULL)
	, m_pButtonClose(NULL)
	, m_nSelected(-1)
{
	for( int i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
		m_pColorPart[i] = NULL;
}

CBikeColorSelect::~CBikeColorSelect ()
{
}

void CBikeColorSelect::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "RAN_BIKE_COLOR_PAGE_LINEBOX" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub( this, "BASIC_LINE_BOX_EDIT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxEditBox( "RAN_BIKE_COLOR_R_EDITBOX_BACK" );
	RegisterControl( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub( this, "BASIC_LINE_BOX_EDIT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxEditBox( "RAN_BIKE_COLOR_G_EDITBOX_BACK" );
	RegisterControl( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub( this, "BASIC_LINE_BOX_EDIT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxEditBox( "RAN_BIKE_COLOR_B_EDITBOX_BACK" );
	RegisterControl( pBasicLineBox );

	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, "RAN_BIKE_COLOR_IN_TEXT" );
	pTextBox->SetFont ( pFont9 );
	pTextBox->SetTextAlign ( TEXT_ALIGN_LEFT );
	pTextBox->SetText( ID2GAMEINTEXT( "BIKE_COLOR_IN_TEXT" ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pTextBox );

	pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, "RAN_BIKE_COLOR_OUT_TEXT" );
	pTextBox->SetFont ( pFont9 );
	pTextBox->SetTextAlign ( TEXT_ALIGN_LEFT );
	pTextBox->SetText( ID2GAMEINTEXT( "BIKE_COLOR_OUT_TEXT" ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pTextBox );

	pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, "RAN_BIKE_COLOR_BODY_TEXT" );
	pTextBox->SetFont ( pFont9 );
	pTextBox->SetTextAlign ( TEXT_ALIGN_LEFT );
	pTextBox->SetText( ID2GAMEINTEXT( "RAN_BIKE_COLOR_BODY_TEXT" ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pTextBox );

	pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, "RAN_BIKE_COLOR_HEAD_TEXT" );
	pTextBox->SetFont ( pFont9 );
	pTextBox->SetTextAlign ( TEXT_ALIGN_LEFT );
	pTextBox->SetText( ID2GAMEINTEXT( "RAN_BIKE_COLOR_HEAD_TEXT" ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pTextBox );

	pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, "RAN_BIKE_COLOR_TIRE_TEXT" );
	pTextBox->SetFont ( pFont9 );
	pTextBox->SetTextAlign ( TEXT_ALIGN_LEFT );
	pTextBox->SetText( ID2GAMEINTEXT( "RAN_BIKE_COLOR_TIRE_TEXT" ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pTextBox );

	pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, "RAN_BIKE_COLOR_MUFFLER_TEXT" );
	pTextBox->SetFont ( pFont9 );
	pTextBox->SetTextAlign ( TEXT_ALIGN_LEFT );
	pTextBox->SetText( ID2GAMEINTEXT( "RAN_BIKE_COLOR_MUFFLER_TEXT" ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pTextBox );

	pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, "RAN_BIKE_COLOR_SUB_SEAT_TEXT" );
	pTextBox->SetFont ( pFont9 );
	pTextBox->SetTextAlign ( TEXT_ALIGN_LEFT );
	pTextBox->SetText( ID2GAMEINTEXT( "RAN_BIKE_COLOR_SUB_SEAT_TEXT" ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pTextBox );

	pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, "RAN_BIKE_COLOR_ACC_TEXT" );
	pTextBox->SetFont ( pFont9 );
	pTextBox->SetTextAlign ( TEXT_ALIGN_LEFT );
	pTextBox->SetText( ID2GAMEINTEXT( "RAN_BIKE_COLOR_SUB_ACC_TEXT" ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pTextBox );

	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_BIKE_COLOR_TITLE_BACK_L", UI_FLAG_DEFAULT );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_BIKE_COLOR_TITLE_BACK_R", UI_FLAG_DEFAULT );
	RegisterControl ( pControl );

	pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, "RAN_BIKE_COLOR_TITLE" );
	pTextBox->SetFont ( pFont9 );
	pTextBox->SetTextAlign ( TEXT_ALIGN_CENTER_X );
	pTextBox->SetText( ID2GAMEINTEXT( "RAN_BIKE_COLOR_TITLE" ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pTextBox );


	std::string strSlot[BIKE_COLOR_SLOT_PART_SIZE] = 
	{
		"RAN_BIKE_COLOR_BODY_FOCUS_0",
		"RAN_BIKE_COLOR_BODY_FOCUS_1",
		"RAN_BIKE_COLOR_BODY_FOCUS_2",
		"RAN_BIKE_COLOR_BODY_FOCUS_3",
		"RAN_BIKE_COLOR_BODY_FOCUS_4",
		"RAN_BIKE_COLOR_BODY_FOCUS_5",
		"RAN_BIKE_COLOR_BODY_FOCUS_6",
		"RAN_BIKE_COLOR_BODY_FOCUS_7",
		"RAN_BIKE_COLOR_BODY_FOCUS_8",
		"RAN_BIKE_COLOR_BODY_FOCUS_9",
		"RAN_BIKE_COLOR_BODY_FOCUS_10",
		"RAN_BIKE_COLOR_BODY_FOCUS_11",
	};

	for ( int i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
	{
		m_pColorPart[i] = new CBikeColorSelectSlot;
		m_pColorPart[i]->CreateSub( this, strSlot[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE, BIKE_COLOR_SELECT_SLOT_A1 + i );
		m_pColorPart[i]->CreateSubControl ( (BIKE_COLOR_SLOT_PART) i );
		RegisterControl ( m_pColorPart[i] );
	}


	pControl = new CUIControl;
	pControl->CreateSub( this, "RAN_BIKE_COLOR_R" );
	RegisterControl( pControl );

	pControl = new CUIControl;
	pControl->CreateSub( this, "RAN_BIKE_COLOR_G" );
	RegisterControl( pControl );

	pControl = new CUIControl;
	pControl->CreateSub( this, "RAN_BIKE_COLOR_B" );
	RegisterControl( pControl );

	m_pRProgressBar = new CBasicProgressBar;
	m_pRProgressBar->CreateSub ( this, "RAN_BIKE_COLOR_R_PROGRESS" );
	m_pRProgressBar->CreateOverImage ( "RAN_BIKE_COLOR_R_PROGRESS_OVERIMAGE" );
	RegisterControl ( m_pRProgressBar );

	m_pGProgressBar = new CBasicProgressBar;
	m_pGProgressBar->CreateSub ( this, "RAN_BIKE_COLOR_G_PROGRESS" );
	m_pGProgressBar->CreateOverImage ( "RAN_BIKE_COLOR_G_PROGRESS_OVERIMAGE" );
	RegisterControl ( m_pGProgressBar );

	m_pBProgressBar = new CBasicProgressBar;
	m_pBProgressBar->CreateSub ( this, "RAN_BIKE_COLOR_B_PROGRESS" );
	m_pBProgressBar->CreateOverImage ( "RAN_BIKE_COLOR_B_PROGRESS_OVERIMAGE" );
	RegisterControl ( m_pBProgressBar );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_BIKE_COLOR_R_PROGRESS_DUMMY", UI_FLAG_DEFAULT, BIKE_COLOR_SELECT_R_PROGRESS );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_BIKE_COLOR_G_PROGRESS_DUMMY", UI_FLAG_DEFAULT, BIKE_COLOR_SELECT_G_PROGRESS );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_BIKE_COLOR_B_PROGRESS_DUMMY", UI_FLAG_DEFAULT, BIKE_COLOR_SELECT_B_PROGRESS );
	RegisterControl ( pControl );

	m_pREditBox = new CUIEditBox;
	m_pREditBox->CreateSub( this, "RAN_BIKE_COLOR_R_EDITBOX", UI_FLAG_XSIZE | UI_FLAG_RIGHT, BIKE_COLOR_SELECT_R_EDITBOX );
	m_pREditBox->CreateCarrat( "MODAL_CARRAT", TRUE, UINT_MAX );
	m_pREditBox->SetLimitInput( nLIMIT_NUMBER_CHAR );
	m_pREditBox->SetFont( pFont9 );
	m_pREditBox->DoUSE_NUMBER( true );
	RegisterControl( m_pREditBox );

	m_pGEditBox = new CUIEditBox;
	m_pGEditBox->CreateSub( this, "RAN_BIKE_COLOR_G_EDITBOX", UI_FLAG_XSIZE | UI_FLAG_RIGHT, BIKE_COLOR_SELECT_G_EDITBOX );
	m_pGEditBox->CreateCarrat( "MODAL_CARRAT", TRUE, UINT_MAX );
	m_pGEditBox->SetLimitInput( nLIMIT_NUMBER_CHAR );
	m_pGEditBox->SetFont( pFont9 );	
	m_pGEditBox->DoUSE_NUMBER( true );
	RegisterControl( m_pGEditBox );

	m_pBEditBox = new CUIEditBox;
	m_pBEditBox->CreateSub( this, "RAN_BIKE_COLOR_B_EDITBOX", UI_FLAG_XSIZE | UI_FLAG_RIGHT, BIKE_COLOR_SELECT_B_EDITBOX );
	m_pBEditBox->CreateCarrat( "MODAL_CARRAT", TRUE, UINT_MAX );
	m_pBEditBox->SetLimitInput( nLIMIT_NUMBER_CHAR );
	m_pBEditBox->SetFont( pFont9 );
	m_pBEditBox->DoUSE_NUMBER( true );
	RegisterControl( m_pBEditBox );

	m_pButtonSave = new CBasicTextButton;
	m_pButtonSave->CreateSub ( this, "BASIC_TEXT_BUTTON14", UI_FLAG_XSIZE, BIKE_COLOR_SELECT_SAVE );
	m_pButtonSave->CreateBaseButton ( "RAN_BIKE_COLOR_OK", CBasicTextButton::SIZE14, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_BIKE_COLOR_CHANGE_TEXT", 0) );
	RegisterControl ( m_pButtonSave );

	m_pButtonClose = new CBasicTextButton;
	m_pButtonClose->CreateSub ( this, "BASIC_TEXT_BUTTON14", UI_FLAG_XSIZE, BIKE_COLOR_SELECT_CLOSE );
	m_pButtonClose->CreateBaseButton ( "RAN_BIKE_COLOR_CANCEL", CBasicTextButton::SIZE14, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_BIKE_COLOR_CHANGE_TEXT", 1) );
	RegisterControl ( m_pButtonClose );
}

void CBikeColorSelect::Update ( int x ,int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

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

			if ( m_nSelected >= 0 && m_nSelected < BIKE_COLOR_SLOT_PART_SIZE )
			{
				GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
				if ( pCharacter && pCharacter->m_bEnableBikeColor )
				{
					WORD wTemp = m_nR << 10	| m_nG << 5 | m_nB;
					pCharacter->m_wBikeColor[m_nSelected] = wTemp;
					pCharacter->BikeColorUpdate();
				}			
			}
		}
	}
}

void CBikeColorSelect::TranslateUIMessage ( UIGUID cID, DWORD dwMsg )
{
	switch( cID )
	{
	case BIKE_COLOR_SELECT_SLOT_A1:
	case BIKE_COLOR_SELECT_SLOT_A2:
	case BIKE_COLOR_SELECT_SLOT_B1:
	case BIKE_COLOR_SELECT_SLOT_B2:
	case BIKE_COLOR_SELECT_SLOT_C1:
	case BIKE_COLOR_SELECT_SLOT_C2:
	case BIKE_COLOR_SELECT_SLOT_D1:
	case BIKE_COLOR_SELECT_SLOT_D2:
	case BIKE_COLOR_SELECT_SLOT_E1:
	case BIKE_COLOR_SELECT_SLOT_E2:
	case BIKE_COLOR_SELECT_SLOT_F1:
	case BIKE_COLOR_SELECT_SLOT_F2:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				int nSelect = cID - BIKE_COLOR_SELECT_SLOT_A1;
				SelectSlot( nSelect );
			}

		}break;

	case BIKE_COLOR_SELECT_R_PROGRESS: TranslateProgressBarMsg( m_pRProgressBar, dwMsg ); break;
	case BIKE_COLOR_SELECT_G_PROGRESS: TranslateProgressBarMsg( m_pGProgressBar, dwMsg ); break;
	case BIKE_COLOR_SELECT_B_PROGRESS: TranslateProgressBarMsg( m_pBProgressBar, dwMsg ); break;

	case BIKE_COLOR_SELECT_R_EDITBOX: TranslateEditBoxMsg( m_pREditBox, dwMsg ); break;
	case BIKE_COLOR_SELECT_G_EDITBOX: TranslateEditBoxMsg( m_pGEditBox, dwMsg ); break;
	case BIKE_COLOR_SELECT_B_EDITBOX: TranslateEditBoxMsg( m_pBEditBox, dwMsg ); break;

	case BIKE_COLOR_SELECT_CLOSE:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				AddMessageEx ( UIMSG_BIKE_COLOR_SELECT_CLOSE );
			}
		}break;

	case BIKE_COLOR_SELECT_SAVE:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				AddMessageEx ( UIMSG_BIKE_COLOR_SELECT_SAVE );
			}
		}break;
	};

	CUIGroup::TranslateUIMessage ( cID, dwMsg );
}

BOOL CBikeColorSelect::UpdateProgressBar( CBasicProgressBar * pBar, int x )
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

void CBikeColorSelect::UpdateProgressBarToEditBox( CBasicProgressBar * pBar, CUIEditBox * pEditBox, INT & nColorVal )
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

BOOL CBikeColorSelect::UpdateEditBoxToProgressBar( CUIEditBox * pEditBox, CBasicProgressBar * pBar, INT & nColorVal )
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

void CBikeColorSelect::TranslateProgressBarMsg( CBasicProgressBar * pBar, DWORD dwMsg )
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

void CBikeColorSelect::TranslateEditBoxMsg( CUIEditBox * pEditBox, DWORD dwMsg )
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

void CBikeColorSelect::RegulateRGB()
{
	m_nRegR = int(m_nR * fRGB_RATIO);
	m_nRegG = int(m_nG * fRGB_RATIO);
	m_nRegB = int(m_nB * fRGB_RATIO);
}

void CBikeColorSelect::ClearEditBox()
{
	m_pREditBox->ClearEdit();
	m_pREditBox->EndEdit();

	m_pGEditBox->ClearEdit();
	m_pGEditBox->EndEdit();

	m_pBEditBox->ClearEdit();
	m_pBEditBox->EndEdit();

	m_nSelected = -1;
}

void CBikeColorSelect::SelectSlot( int nSlot )
{
	for( int i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
	{
		if ( m_pColorPart[i] )	m_pColorPart[i]->SetSelect( FALSE );
	}

	if ( nSlot < 0 )	return;
	if ( nSlot >= BIKE_COLOR_SLOT_PART_SIZE )	return;

	m_nSelected = nSlot;

	if ( m_pColorPart[nSlot] )
	{
		m_pColorPart[nSlot]->SetSelect( TRUE );
	}

	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pCharacter )	return;
	if ( !pCharacter->m_bEnableBikeColor )	return;

	const WORD & wColor = pCharacter->m_wBikeColor[m_nSelected];

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