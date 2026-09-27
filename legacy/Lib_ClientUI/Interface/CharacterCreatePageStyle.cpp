#include "StdAfx.h"
#include "CharacterCreatePageStyle.h"

#include "d3dfont.h"
#include "UITextControl.h"
#include "GameTextControl.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicTextButton.h"
#include "BasicProgressBar.h"
#include "UIEditBox.h"

#include "DxGlobalStage.h"
#include "DxLobyStage.h"
#include "ModalWindow.h"
#include "OuterInterface.h"

#include "../Lib_Client/G-Logic/GLCharData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int CCharacterCreatePageStyle::nLIMITCHAR = 16;

CCharacterCreatePageStyle::CCharacterCreatePageStyle ()
	: m_nFaceTypeBack( -1 )
	, m_nHairTypeBack( -1 )
	, m_wHairColorBack( 32768 )
	, m_fCharScaleBack( 0.0f )
	, m_nHairColorIndex( -1 )
	, m_fCharScaleBase( 1.0f )
	, m_pFaceTypeButtonLeft( NULL )
	, m_pFaceTypeButtonRight( NULL )
	, m_pHairTypeButtonLeft( NULL )
	, m_pHairTypeButtonRight( NULL )
	, m_pHairColorButtonLeft( NULL )
	, m_pHairColorButtonRight( NULL )
	, m_pCharScaleButtonLeft( NULL )
	, m_pCharScaleButtonRight( NULL )
	, m_pStylePageStatic( NULL )
	, m_pFaceTypeStatic( NULL )
	, m_pHairTypeStatic( NULL )
	, m_pHairColorStatic( NULL )
	, m_pCharScaleStatic( NULL )
	, m_pNameStatic( NULL )
	, m_pFaceTypeTextBox( NULL )
	, m_pHairTypeTextBox( NULL )
	, m_pHairColorLineBox( NULL )
	, m_pCharacterScaleBar( NULL )
	, m_pNameEditBox( NULL )
{
}

CCharacterCreatePageStyle::~CCharacterCreatePageStyle ()
{
}

void CCharacterCreatePageStyle::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "CREATE_CHAR_SET_TEXT" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "CREATE_CHAR_FACE_TEXT" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "CREATE_CHAR_FACE_TYPE_TEXT" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "CREATE_CHAR_HAIR_TEXT" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "CREATE_CHAR_HAIR_TYPE_TEXT" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "CREATE_CHAR_HAIRCOLOR_TEXT" );
	RegisterControl ( pBasicLineBox );

	m_pHairColorLineBox = new CBasicLineBox;
	m_pHairColorLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pHairColorLineBox->CreateBaseBoxOuter ( "CREATE_CHAR_HAIRCOLOR_TYPE_TEXT" );
	RegisterControl ( m_pHairColorLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "CREATE_CHAR_SCALE_TEXT" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "CREATE_CHAR_SCALE_TYPE_TEXT" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "CREATE_CHAR_NAME_TEXT" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "CREATE_CHAR_NAME_EDIT_TEXT" );
	RegisterControl ( pBasicLineBox );

	CUIControl* pNameLine = new CUIControl;
	pNameLine->CreateSub ( this, "CREATE_CHAR_NAME_EDIT_TEXT_LINE" );	
	RegisterControl ( pNameLine );

	m_pFaceTypeButtonLeft = new CBasicButton;
	m_pFaceTypeButtonLeft->CreateSub ( this,  "CREATE_CHAR_FACE_L", UI_FLAG_DEFAULT, CHARACTER_CREATE_PAGE_STYLE_FACETYPE_LEFT );
	m_pFaceTypeButtonLeft->CreateFlip ( "CREATE_CHAR_FACE_L_F", CBasicButton::CLICK_FLIP );
	m_pFaceTypeButtonLeft->SetFlip(false);
	m_pFaceTypeButtonLeft->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pFaceTypeButtonLeft );

	m_pFaceTypeButtonRight = new CBasicButton;
	m_pFaceTypeButtonRight->CreateSub ( this,  "CREATE_CHAR_FACE_R", UI_FLAG_DEFAULT, CHARACTER_CREATE_PAGE_STYLE_FACETYPE_RIGHT );
	m_pFaceTypeButtonRight->CreateFlip ( "CREATE_CHAR_FACE_R_F", CBasicButton::CLICK_FLIP );
	m_pFaceTypeButtonRight->SetFlip(false);
	m_pFaceTypeButtonRight->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pFaceTypeButtonRight );

	m_pHairTypeButtonLeft = new CBasicButton;
	m_pHairTypeButtonLeft->CreateSub ( this,  "CREATE_CHAR_HAIR_L", UI_FLAG_DEFAULT, CHARACTER_CREATE_PAGE_STYLE_HAIRSTYLE_LEFT );
	m_pHairTypeButtonLeft->CreateFlip ( "CREATE_CHAR_HAIR_L_F", CBasicButton::CLICK_FLIP );
	m_pHairTypeButtonLeft->SetFlip(false);
	m_pHairTypeButtonLeft->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pHairTypeButtonLeft );

	m_pHairTypeButtonRight = new CBasicButton;
	m_pHairTypeButtonRight->CreateSub ( this,  "CREATE_CHAR_HAIR_R", UI_FLAG_DEFAULT, CHARACTER_CREATE_PAGE_STYLE_HAIRSTYLE_RIGHT );
	m_pHairTypeButtonRight->CreateFlip ( "CREATE_CHAR_HAIR_R_F", CBasicButton::CLICK_FLIP );
	m_pHairTypeButtonRight->SetFlip(false);
	m_pHairTypeButtonRight->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pHairTypeButtonRight );

	m_pHairColorButtonLeft = new CBasicButton;
	m_pHairColorButtonLeft->CreateSub ( this,  "CREATE_CHAR_HAIRCOLOR_L", UI_FLAG_DEFAULT, CHARACTER_CREATE_PAGE_STYLE_HAIRCOLOR_LEFT );
	m_pHairColorButtonLeft->CreateFlip ( "CREATE_CHAR_HAIRCOLOR_L_F", CBasicButton::CLICK_FLIP );
	m_pHairColorButtonLeft->SetFlip(false);
	m_pHairColorButtonLeft->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pHairColorButtonLeft );

	m_pHairColorButtonRight = new CBasicButton;
	m_pHairColorButtonRight->CreateSub ( this,  "CREATE_CHAR_HAIRCOLOR_R", UI_FLAG_DEFAULT, CHARACTER_CREATE_PAGE_STYLE_HAIRCOLOR_RIGHT );
	m_pHairColorButtonRight->CreateFlip ( "CREATE_CHAR_HAIRCOLOR_R_F", CBasicButton::CLICK_FLIP );
	m_pHairColorButtonRight->SetFlip(false);
	m_pHairColorButtonRight->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pHairColorButtonRight );

	m_pCharScaleButtonLeft = new CBasicButton;
	m_pCharScaleButtonLeft->CreateSub ( this,  "CREATE_CHAR_SCALE_L", UI_FLAG_DEFAULT, CHARACTER_CREATE_PAGE_STYLE_CHARSCALE_LEFT );
	m_pCharScaleButtonLeft->CreateFlip ( "CREATE_CHAR_SCALE_L_F", CBasicButton::CLICK_FLIP );
	m_pCharScaleButtonLeft->SetFlip(false);
	m_pCharScaleButtonLeft->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pCharScaleButtonLeft );

	m_pCharScaleButtonRight = new CBasicButton;
	m_pCharScaleButtonRight->CreateSub ( this,  "CREATE_CHAR_SCALE_R", UI_FLAG_DEFAULT, CHARACTER_CREATE_PAGE_STYLE_CHARSCALE_RIGHT );
	m_pCharScaleButtonRight->CreateFlip ( "CREATE_CHAR_SCALE_R_F", CBasicButton::CLICK_FLIP );
	m_pCharScaleButtonRight->SetFlip(false);
	m_pCharScaleButtonRight->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pCharScaleButtonRight );

	m_pStylePageStatic = new CBasicTextBox;
	m_pStylePageStatic->CreateSub ( this, "CREATE_CHAR_SET_TEXT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pStylePageStatic->SetFont ( pFont9 );
	m_pStylePageStatic->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pStylePageStatic );

	m_pFaceTypeStatic = new CBasicTextBox;
	m_pFaceTypeStatic->CreateSub ( this, "CREATE_CHAR_FACE_TEXT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pFaceTypeStatic->SetFont ( pFont9 );
	m_pFaceTypeStatic->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pFaceTypeStatic );

	m_pHairTypeStatic = new CBasicTextBox;
	m_pHairTypeStatic->CreateSub ( this, "CREATE_CHAR_HAIR_TEXT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pHairTypeStatic->SetFont ( pFont9 );
	m_pHairTypeStatic->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pHairTypeStatic );

	m_pHairColorStatic = new CBasicTextBox;
	m_pHairColorStatic->CreateSub ( this, "CREATE_CHAR_HAIRCOLOR_TEXT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pHairColorStatic->SetFont ( pFont9 );
	m_pHairColorStatic->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pHairColorStatic );

	m_pCharScaleStatic = new CBasicTextBox;
	m_pCharScaleStatic->CreateSub ( this, "CREATE_CHAR_SCALE_TEXT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pCharScaleStatic->SetFont ( pFont9 );
	m_pCharScaleStatic->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pCharScaleStatic );

	m_pNameStatic = new CBasicTextBox;
	m_pNameStatic->CreateSub ( this, "CREATE_CHAR_NAME_TEXT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pNameStatic->SetFont ( pFont9 );
	m_pNameStatic->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pNameStatic );

	m_pFaceTypeTextBox = new CBasicTextBox;
	m_pFaceTypeTextBox->CreateSub ( this, "CREATE_CHAR_FACE_TYPE_TEXT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pFaceTypeTextBox->SetFont ( pFont9 );
	m_pFaceTypeTextBox->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pFaceTypeTextBox );

	m_pHairTypeTextBox = new CBasicTextBox;
	m_pHairTypeTextBox->CreateSub ( this, "CREATE_CHAR_HAIR_TYPE_TEXT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pHairTypeTextBox->SetFont ( pFont9 );
	m_pHairTypeTextBox->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pHairTypeTextBox );

	m_pCharacterScaleBar = new CBasicProgressBar;
	m_pCharacterScaleBar->CreateSub ( this, "CREATE_CHAR_SCALE_BAR" );
	m_pCharacterScaleBar->CreateOverImage ( "CREATE_CHAR_SCALE_BAR_OVERIMAGE" );
	m_pCharacterScaleBar->SetControlNameEx ( "Character Scale" );
	RegisterControl ( m_pCharacterScaleBar );
	m_pCharacterScaleBar->SetVisibleSingle(TRUE);

	m_pNameEditBox = new CUIEditBox;
	m_pNameEditBox->CreateSub ( this, "CREATE_CHAR_NAME_EDIT_TEXT", UI_FLAG_DEFAULT, CHARACTER_CREATE_PAGE_STYLE_CHARNAME_EDIT );
	m_pNameEditBox->CreateCarrat ( "CREATE_CHAR_NAME_EDIT_TEXT_CARRAT", TRUE, UINT_MAX );
	m_pNameEditBox->SetLimitInput ( nLIMITCHAR );
	m_pNameEditBox->SetFont ( pFont9 );
	m_pNameEditBox->SetTextColor( NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pNameEditBox );

	m_pStylePageStatic->AddText ( ID2GAMEWORD("CREATE_CHARACTER_PAGE", 1 ), NS_UITEXTCOLOR::WHITE );
	m_pFaceTypeStatic->AddText ( ID2GAMEWORD("CREATE_CHARACTER_PAGE_STATIC", 2 ), NS_UITEXTCOLOR::WHITE );
	m_pHairTypeStatic->AddText ( ID2GAMEWORD("CREATE_CHARACTER_PAGE_STATIC", 3 ), NS_UITEXTCOLOR::WHITE );
	m_pHairColorStatic->AddText ( ID2GAMEWORD("CREATE_CHARACTER_PAGE_STATIC", 7 ), NS_UITEXTCOLOR::WHITE );
	m_pCharScaleStatic->AddText ( ID2GAMEWORD("CREATE_CHARACTER_PAGE_STATIC", 5 ), NS_UITEXTCOLOR::WHITE );
	m_pNameStatic->AddText ( ID2GAMEWORD("CREATE_CHARACTER_PAGE_STATIC", 4 ), NS_UITEXTCOLOR::WHITE );
}

void CCharacterCreatePageStyle::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch( ControlID )
	{
	case CHARACTER_CREATE_PAGE_STYLE_FACETYPE_LEFT:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
				DxGlobalStage::GetInstance().GetLobyStage()->ShiftCharFace(DxLobyStage::EM_PREV);
			}
		}break;

	case CHARACTER_CREATE_PAGE_STYLE_FACETYPE_RIGHT:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
				DxGlobalStage::GetInstance().GetLobyStage()->ShiftCharFace(DxLobyStage::EM_NEXT);
			}
		}break;

	case CHARACTER_CREATE_PAGE_STYLE_HAIRSTYLE_LEFT:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
				DxGlobalStage::GetInstance().GetLobyStage()->ShiftCharHair(DxLobyStage::EM_PREV);
			}
		}break;

	case CHARACTER_CREATE_PAGE_STYLE_HAIRSTYLE_RIGHT:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
				DxGlobalStage::GetInstance().GetLobyStage()->ShiftCharHair(DxLobyStage::EM_NEXT);
			}
		}break;

	case CHARACTER_CREATE_PAGE_STYLE_HAIRCOLOR_LEFT:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
				HairColorPrev();
			}
		}break;

	case CHARACTER_CREATE_PAGE_STYLE_HAIRCOLOR_RIGHT:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
				HairColorNext();
			}
		}break;

	case CHARACTER_CREATE_PAGE_STYLE_CHARSCALE_LEFT:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
				CharacterScaleDecrease();
			}
		}break;

	case CHARACTER_CREATE_PAGE_STYLE_CHARSCALE_RIGHT:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
				CharacterScaleIncrease();
			}
		}break;

	case CHARACTER_CREATE_PAGE_STYLE_CHARNAME_EDIT:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
				ResetNameEdit();
			}
		}break;
	};
}

void CCharacterCreatePageStyle::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	CNetClient* pNetClient = DxGlobalStage::GetInstance().GetNetClient ();
	if ( !pNetClient->IsOnline() ){
		if ( !COuterInterface::GetInstance().IsVisibleGroup ( MODAL_WINDOW_OUTER ) ){
			DoModalOuter ( ID2GAMEEXTEXT ("CHARACTERSTAGE_2"), MODAL_INFOMATION, OK, OUTER_MODAL_CONNECTCLOSED );
		}
	}

	DxLobyStage *pLobyStage = DxGlobalStage::GetInstance().GetLobyStage();
	if ( pLobyStage ){
		const GLCHAR_NEWINFO& sNEWINFO = pLobyStage->GetNewCharInfo ();	
		const EMCHARINDEX& emCharIndex = sNEWINFO.m_emIndex;

		if ( m_nFaceTypeBack != (int)sNEWINFO.m_wFace ){
			m_nFaceTypeBack = (int)sNEWINFO.m_wFace;

			CString strText;
			strText.Format( "%s :%d", ID2GAMEWORD("CREATE_CHARACTER_PAGE_STATIC", 2 ), m_nFaceTypeBack );
			m_pFaceTypeTextBox->SetText( strText.GetString(), NS_UITEXTCOLOR::WHITE );
		}

		if ( m_nHairTypeBack != (int)sNEWINFO.m_wHair ){
			m_nHairTypeBack = (int)sNEWINFO.m_wHair;

			CString strText;
			strText.Format( "%s :%d", ID2GAMEWORD("CREATE_CHARACTER_PAGE_STATIC", 3 ), m_nHairTypeBack );
			m_pHairTypeTextBox->SetText( strText.GetString(), NS_UITEXTCOLOR::WHITE );

			if ( m_nHairTypeBack >= 0 && m_nHairTypeBack < MAX_HAIR ){
				m_nHairColorIndex = m_nHairTypeBack;
			}
		}

		if ( m_wHairColorBack != sNEWINFO.m_wHairColor ){
			m_wHairColorBack = sNEWINFO.m_wHairColor;

			if ( m_pHairColorLineBox ){

				WORD wR, wG, wB;
				wR = m_wHairColorBack >> 10;
				wG = m_wHairColorBack >> 5;
				wG = wG&0x1f;
				wB = m_wHairColorBack&0x1f;

				wR = (WORD)( wR * 8.225806f );	//<< 3;
				wG = (WORD)( wG * 8.225806f );	//<< 3;
				wB = (WORD)( wB * 8.225806f );	//<< 3;

				DWORD dwColor = 0xff000000 + (wR<<16) + (wG<<8) + wB;

				m_pHairColorLineBox->SetUseRender( TRUE );
				m_pHairColorLineBox->SetDiffuse( dwColor );
			}
		}

		if ( m_fCharScaleBack != sNEWINFO.m_fScaleRange ){
			m_fCharScaleBack = sNEWINFO.m_fScaleRange;
			m_fCharScaleBase = m_fCharScaleBack;
			float fPercent = ( m_fCharScaleBack - SCALERANGE_MIN ) / ( SCALERANGE_MAX - SCALERANGE_MIN );
			m_pCharacterScaleBar->SetPercent ( fPercent );
		}
	}
}

void CCharacterCreatePageStyle::ResetAll()
{
	m_nFaceTypeBack = -1;
	m_nHairTypeBack = -1;
	m_wHairColorBack = 32768;
	m_fCharScaleBack = 0.0f;
	m_nHairColorIndex = -1;
	m_fCharScaleBase = 1.0f;

	m_pCharacterScaleBar->SetPercent( 50.0f );
	m_pFaceTypeTextBox->ClearText();
	m_pHairTypeTextBox->ClearText();
	m_pNameEditBox->EndEdit();
	m_pNameEditBox->ClearEdit();
}

void CCharacterCreatePageStyle::HairColorNext()
{
	m_nHairColorIndex ++;
	if ( m_nHairColorIndex >= MAX_HAIR ){
		m_nHairColorIndex --;
	}

	DxLobyStage *pLobyStage = DxGlobalStage::GetInstance().GetLobyStage();
	if ( pLobyStage ){
		const GLCHAR_NEWINFO& sNEWINFO = pLobyStage->GetNewCharInfo ();	
		const EMCHARINDEX& emCharIndex = sNEWINFO.m_emIndex;

		WORD wHairColor = HAIRCOLOR::GetHairColor( (WORD)emCharIndex, m_nHairColorIndex );
		pLobyStage->SelCharHairColor( wHairColor );
	}
}

void CCharacterCreatePageStyle::HairColorPrev()
{
	m_nHairColorIndex --;
	if ( m_nHairColorIndex < 0 ){
		m_nHairColorIndex ++;
	}

	DxLobyStage *pLobyStage = DxGlobalStage::GetInstance().GetLobyStage();
	if ( pLobyStage ){
		const GLCHAR_NEWINFO& sNEWINFO = pLobyStage->GetNewCharInfo ();	
		const EMCHARINDEX& emCharIndex = sNEWINFO.m_emIndex;

		WORD wHairColor = HAIRCOLOR::GetHairColor( (WORD)emCharIndex, m_nHairColorIndex );
		pLobyStage->SelCharHairColor( wHairColor );
	}
}

void CCharacterCreatePageStyle::CharacterScaleIncrease()
{
	m_fCharScaleBase += 0.015f;
	if ( m_fCharScaleBase >= SCALERANGE_MAX ){
		m_fCharScaleBase = SCALERANGE_MAX;
	}

	DxLobyStage *pLobyStage = DxGlobalStage::GetInstance().GetLobyStage();
	if ( pLobyStage ){
		pLobyStage->SelCharScaleRange( m_fCharScaleBase );
	}
}

void CCharacterCreatePageStyle::CharacterScaleDecrease()
{
	m_fCharScaleBase -= 0.015f;
	if ( m_fCharScaleBase <= SCALERANGE_MIN ){
		m_fCharScaleBase = SCALERANGE_MIN;
	}

	DxLobyStage *pLobyStage = DxGlobalStage::GetInstance().GetLobyStage();
	if ( pLobyStage ){
		pLobyStage->SelCharScaleRange( m_fCharScaleBase );
	}
}

void CCharacterCreatePageStyle::ResetNameEdit()
{
	m_pNameEditBox->EndEdit();
	m_pNameEditBox->ClearEdit();

	if( !m_pNameEditBox->IsBegin() ){
		m_pNameEditBox->BeginEdit();
	}
}