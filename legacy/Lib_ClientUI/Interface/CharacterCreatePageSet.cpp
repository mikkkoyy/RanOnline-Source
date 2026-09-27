#include "StdAfx.h"
#include "CharacterCreatePageSet.h"
#include "CharacterCreateInfo.h"

#include "d3dfont.h"
#include "UITextControl.h"
#include "GameTextControl.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "ModalWindow.h"

#include "OuterInterface.h"
#include "DxGlobalStage.h"
#include "DxLobyStage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCharacterCreatePageSet::CCharacterCreatePageSet ()
	: m_bUseCancel( TRUE )
	, m_bCreateClassExtremeM( TRUE )
	, m_bCreateClassExtremeW( TRUE )
	, m_nSelectedSchool( -1 )
	, m_nSelectedSex( -1 )
	, m_nSelectedClass( -1 )
	, m_nSelectedLastSchool( -1 )
	, m_nSelectedLastSex( -1 )
	, m_nSelectedLastClass( -1 )

	//jdevtodo
	//enable later when new classes are ready
	, m_bCreateClassGunner(TRUE)
	, m_bCreateClassAssassin(FALSE)
	, m_bCreateClassMagician(FALSE)
{
	for( int i=0; i<GLSCHOOL_NUM; ++i ){
		m_pTypeSchool_Off[i] = NULL;
		m_pTypeSchool_On[i] = NULL;
		m_pTypeSchool_Box[i] = NULL;
		m_pTypeSchool_Focus[i] = NULL;
	}

	for( int i=0; i<2; ++i ){
		m_pTypeSex_Off[i] = NULL;
		m_pTypeSex_On[i] = NULL;
		m_pTypeSex_Box[i] = NULL;
		m_pTypeSex_Focus[i] = NULL;
	}

	for( int i=0; i<GLCI_NUM_8CLASS; ++i ){
		m_pTypeClass_Off[i] = NULL;
		m_pTypeClass_On[i] = NULL;
		m_pTypeClass_Box[i] = NULL;
		m_pTypeClass_Focus[i] = NULL;
	}
}

CCharacterCreatePageSet::~CCharacterCreatePageSet ()
{
}

void CCharacterCreatePageSet::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter( "CREATE_CHAR_SCHOOL_TEXT" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter( "CREATE_CHAR_SEX_TEXT" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter( "CREATE_CHAR_CLASS_TEXT" );
	RegisterControl ( pBasicLineBox );

	CBasicTextBox* pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "CREATE_CHAR_SCHOOL_TEXT" );
	pStaticText->SetFont ( pFont9 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );	
	pStaticText->SetText( (char*)ID2GAMEWORD ("CREATE_CHARACTER_PAGE_STATIC", 0), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "CREATE_CHAR_SEX_TEXT" );
	pStaticText->SetFont ( pFont9 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );	
	pStaticText->SetText( (char*)ID2GAMEWORD ("CREATE_CHARACTER_PAGE_STATIC", 6), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "CREATE_CHAR_CLASS_TEXT" );
	pStaticText->SetFont ( pFont9 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );	
	pStaticText->SetText( (char*)ID2GAMEWORD ("CREATE_CHARACTER_PAGE_STATIC", 1), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );


	std::string strSchoolOff[GLSCHOOL_NUM] = 
	{
		"CREATE_CHAR_MARK_SM_OFF",
		"CREATE_CHAR_MARK_HA_OFF",
		"CREATE_CHAR_MARK_BH_OFF",
	};

	std::string strSchoolOn[GLSCHOOL_NUM] = 
	{
		"CREATE_CHAR_MARK_SM_ON",
		"CREATE_CHAR_MARK_HA_ON",
		"CREATE_CHAR_MARK_BH_ON",
	};

	std::string strSchoolBox[GLSCHOOL_NUM] = 
	{
		"CREATE_CHAR_MARK_SM_BOX",
		"CREATE_CHAR_MARK_HA_BOX",
		"CREATE_CHAR_MARK_BH_BOX",
	};

	std::string strSchoolFocus[GLSCHOOL_NUM] = 
	{
		"CREATE_CHAR_MARK_SM_FOCUS",
		"CREATE_CHAR_MARK_HA_FOCUS",
		"CREATE_CHAR_MARK_BH_FOCUS",
	};

	for( int i=0; i<GLSCHOOL_NUM; ++i ){

		m_pTypeSchool_Off[i] = new CUIControl;
		m_pTypeSchool_Off[i]->CreateSub ( this, strSchoolOff[i].c_str() );	
		m_pTypeSchool_Off[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pTypeSchool_Off[i] );

		m_pTypeSchool_On[i] = new CUIControl;
		m_pTypeSchool_On[i]->CreateSub ( this, strSchoolOn[i].c_str() );	
		m_pTypeSchool_On[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pTypeSchool_On[i] );

		m_pTypeSchool_Box[i] = new CUIControl;
		m_pTypeSchool_Box[i]->CreateSub ( this, strSchoolBox[i].c_str() );	
		m_pTypeSchool_Box[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pTypeSchool_Box[i] );

		m_pTypeSchool_Focus[i] = new CUIControl;
		m_pTypeSchool_Focus[i]->CreateSub ( this, strSchoolFocus[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE, CHARACTER_CREATE_PAGE_TYPE_SCHOOL_0 + i );	
		m_pTypeSchool_Focus[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pTypeSchool_Focus[i] );
	}


	std::string strSexOff[2] = 
	{
		"CREATE_CHAR_SEX_MALE_OFF",
		"CREATE_CHAR_SEX_FEMALE_OFF",
	};

	std::string strSexOn[2] = 
	{
		"CREATE_CHAR_SEX_MALE_ON",
		"CREATE_CHAR_SEX_FEMALE_ON",
	};

	std::string strSexBox[2] = 
	{
		"CREATE_CHAR_SEX_MALE_BOX",
		"CREATE_CHAR_SEX_FEMALE_BOX",
	};

	std::string strSexFocus[2] = 
	{
		"CREATE_CHAR_SEX_MALE_FOCUS",
		"CREATE_CHAR_SEX_FEMALE_FOCUS",
	};

	for( int i=0; i<2; ++i ){

		m_pTypeSex_Off[i] = new CUIControl;
		m_pTypeSex_Off[i]->CreateSub ( this, strSexOff[i].c_str() );	
		m_pTypeSex_Off[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pTypeSex_Off[i] );

		m_pTypeSex_On[i] = new CUIControl;
		m_pTypeSex_On[i]->CreateSub ( this, strSexOn[i].c_str() );	
		m_pTypeSex_On[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pTypeSex_On[i] );

		m_pTypeSex_Box[i] = new CUIControl;
		m_pTypeSex_Box[i]->CreateSub ( this, strSexBox[i].c_str() );	
		m_pTypeSex_Box[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pTypeSex_Box[i] );

		m_pTypeSex_Focus[i] = new CUIControl;
		m_pTypeSex_Focus[i]->CreateSub ( this, strSexFocus[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE, CHARACTER_CREATE_PAGE_TYPE_SEX_MALE + i );	
		m_pTypeSex_Focus[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pTypeSex_Focus[i] );
	}


	std::string strClass_Off[GLCI_NUM_8CLASS] = 
	{
		"CREATE_CHAR_CLASS_BRAWLER_MALE_OFF",
		"CREATE_CHAR_CLASS_SWORDMAN_MALE_OFF",
		"CREATE_CHAR_CLASS_ARCHER_FEMALE_OFF",
		"CREATE_CHAR_CLASS_SHAMAN_FEMALE_OFF",
		"CREATE_CHAR_CLASS_EXTREME_MALE_OFF",
		"CREATE_CHAR_CLASS_EXTREME_FEMALE_OFF",
		"CREATE_CHAR_CLASS_BRAWLER_FEMALE_OFF",
		"CREATE_CHAR_CLASS_SWORDMAN_FEMALE_OFF",
		"CREATE_CHAR_CLASS_ARCHER_MALE_OFF",
		"CREATE_CHAR_CLASS_SHAMAN_MALE_OFF",
		"CREATE_CHAR_CLASS_SCIENTIST_MALE_OFF",
		"CREATE_CHAR_CLASS_SCIENTIST_FEMALE_OFF",
		"CREATE_CHAR_CLASS_ASSASSIN_MALE_OFF",
		"CREATE_CHAR_CLASS_ASSASSIN_FEMALE_OFF",
		"CREATE_CHAR_CLASS_TRICKER_MALE_OFF",
		"CREATE_CHAR_CLASS_TRICKER_FEMALE_OFF",

	};

	std::string strClass_On[GLCI_NUM_8CLASS] = 
	{
		"CREATE_CHAR_CLASS_BRAWLER_MALE_ON",
		"CREATE_CHAR_CLASS_SWORDMAN_MALE_ON",
		"CREATE_CHAR_CLASS_ARCHER_FEMALE_ON",
		"CREATE_CHAR_CLASS_SHAMAN_FEMALE_ON",
		"CREATE_CHAR_CLASS_EXTREME_MALE_ON",
		"CREATE_CHAR_CLASS_EXTREME_FEMALE_ON",
		"CREATE_CHAR_CLASS_BRAWLER_FEMALE_ON",
		"CREATE_CHAR_CLASS_SWORDMAN_FEMALE_ON",
		"CREATE_CHAR_CLASS_ARCHER_MALE_ON",
		"CREATE_CHAR_CLASS_SHAMAN_MALE_ON",
		"CREATE_CHAR_CLASS_SCIENTIST_MALE_ON",
		"CREATE_CHAR_CLASS_SCIENTIST_FEMALE_ON",
		"CREATE_CHAR_CLASS_ASSASSIN_MALE_ON",
		"CREATE_CHAR_CLASS_ASSASSIN_FEMALE_ON",
		"CREATE_CHAR_CLASS_TRICKER_MALE_ON",
		"CREATE_CHAR_CLASS_TRICKER_FEMALE_ON",
	};

	std::string strClass_Box[GLCI_NUM_8CLASS] = 
	{
		"CREATE_CHAR_CLASS_BRAWLER_MALE_BOX",
		"CREATE_CHAR_CLASS_SWORDMAN_MALE_BOX",
		"CREATE_CHAR_CLASS_ARCHER_FEMALE_BOX",
		"CREATE_CHAR_CLASS_SHAMAN_FEMALE_BOX",
		"CREATE_CHAR_CLASS_EXTREME_MALE_BOX",
		"CREATE_CHAR_CLASS_EXTREME_FEMALE_BOX",
		"CREATE_CHAR_CLASS_BRAWLER_FEMALE_BOX",
		"CREATE_CHAR_CLASS_SWORDMAN_FEMALE_BOX",
		"CREATE_CHAR_CLASS_ARCHER_MALE_BOX",
		"CREATE_CHAR_CLASS_SHAMAN_MALE_BOX",
		"CREATE_CHAR_CLASS_SCIENTIST_MALE_BOX",
		"CREATE_CHAR_CLASS_SCIENTIST_FEMALE_BOX",
		"CREATE_CHAR_CLASS_ASSASSIN_MALE_BOX",
		"CREATE_CHAR_CLASS_ASSASSIN_FEMALE_BOX",
		"CREATE_CHAR_CLASS_TRICKER_MALE_BOX",
		"CREATE_CHAR_CLASS_TRICKER_FEMALE_BOX",
	};

	std::string strClass_Focus[GLCI_NUM_8CLASS] = 
	{
		"CREATE_CHAR_CLASS_BRAWLER_MALE_FOCUS",
		"CREATE_CHAR_CLASS_SWORDMAN_MALE_FOCUS",
		"CREATE_CHAR_CLASS_ARCHER_FEMALE_FOCUS",
		"CREATE_CHAR_CLASS_SHAMAN_FEMALE_FOCUS",
		"CREATE_CHAR_CLASS_EXTREME_MALE_FOCUS",
		"CREATE_CHAR_CLASS_EXTREME_FEMALE_FOCUS",
		"CREATE_CHAR_CLASS_BRAWLER_FEMALE_FOCUS",
		"CREATE_CHAR_CLASS_SWORDMAN_FEMALE_FOCUS",
		"CREATE_CHAR_CLASS_ARCHER_MALE_FOCUS",
		"CREATE_CHAR_CLASS_SHAMAN_MALE_FOCUS",
		"CREATE_CHAR_CLASS_SCIENTIST_MALE_FOCUS",
		"CREATE_CHAR_CLASS_SCIENTIST_FEMALE_FOCUS",
		"CREATE_CHAR_CLASS_ASSASSIN_MALE_FOCUS",
		"CREATE_CHAR_CLASS_ASSASSIN_FEMALE_FOCUS",
		"CREATE_CHAR_CLASS_TRICKER_MALE_FOCUS",
		"CREATE_CHAR_CLASS_TRICKER_FEMALE_FOCUS",
	};

	for( int i=0; i<GLCI_NUM_8CLASS; ++i )
	{
		//int nIndex = 0;
		//switch ( i )
		//{
		//case GLCI_BRAWLER_M:
		//case GLCI_BRAWLER_W:
		//	{
		//		nIndex = 1;
		//	}
		//	break;

		//case GLCI_SWORDSMAN_M:
		//case GLCI_SWORDSMAN_W:
		//	{
		//		nIndex = 2;
		//	}
		//	break;

		//case GLCI_ARCHER_M:
		//case GLCI_ARCHER_W:
		//	{
		//		nIndex = 3;
		//	}
		//	break;

		//case GLCI_SHAMAN_M:
		//case GLCI_SHAMAN_W:
		//	{
		//		nIndex = 4;
		//	}
		//	break;	

		//case GLCI_EXTREME_M:
		//case GLCI_EXTREME_W:
		//	{
		//		nIndex = 0;
		//	}
		//	break;
		//	//add class
		//case GLCI_GUNNER_M:
		//case GLCI_GUNNER_W:
		//	{
		//		nIndex = 5;
		//	}
		//	break;

		//case GLCI_ASSASSIN_M:
		//case GLCI_ASSASSIN_W:
		//	{
		//		nIndex = 6;
		//	}
		//	break;

		//case GLCI_TRICKER_M:
		//case GLCI_TRICKER_W:
		//	{
		//		nIndex = 7;
		//	}
		//	break;
		//};

		m_pTypeClass_Off[i] = new CUIControl;
		m_pTypeClass_Off[i]->CreateSub ( this, strClass_Off[i].c_str() );	
		m_pTypeClass_Off[i]->SetVisibleSingle ( FALSE );

		//if ( RANPARAM::bCreateClass[i] )
		RegisterControl ( m_pTypeClass_Off[i] );

		m_pTypeClass_On[i] = new CUIControl;
		m_pTypeClass_On[i]->CreateSub ( this, strClass_On[i].c_str() );	
		m_pTypeClass_On[i]->SetVisibleSingle ( FALSE );
		//if ( RANPARAM::bCreateClass[i] )
		RegisterControl ( m_pTypeClass_On[i] );

		m_pTypeClass_Box[i] = new CUIControl;
		m_pTypeClass_Box[i]->CreateSub ( this, strClass_Box[i].c_str() );	
		m_pTypeClass_Box[i]->SetVisibleSingle ( FALSE );
		//if ( RANPARAM::bCreateClass[i] )
		RegisterControl ( m_pTypeClass_Box[i] );

		m_pTypeClass_Focus[i] = new CUIControl;
		m_pTypeClass_Focus[i]->CreateSub ( this, strClass_Focus[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE, CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_BRAWLER + i );	
		m_pTypeClass_Focus[i]->SetVisibleSingle ( FALSE );
		//if ( RANPARAM::bCreateClass[i] )
		RegisterControl ( m_pTypeClass_Focus[i] );
	}

	ResetAll();
}

void CCharacterCreatePageSet::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	
	switch( ControlID )
	{
	case CHARACTER_CREATE_PAGE_TYPE_SCHOOL_0:
	case CHARACTER_CREATE_PAGE_TYPE_SCHOOL_1:
	case CHARACTER_CREATE_PAGE_TYPE_SCHOOL_2:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) ) {
				int nSelect = ControlID - CHARACTER_CREATE_PAGE_TYPE_SCHOOL_0;
				SchoolMouseIn( nSelect );
				
				if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
					SchoolSelect( nSelect );
				}
			}
			
		}break;

	case CHARACTER_CREATE_PAGE_TYPE_SEX_MALE:
	case CHARACTER_CREATE_PAGE_TYPE_SEX_FEMALE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) ) {
				int nSelect = ControlID - CHARACTER_CREATE_PAGE_TYPE_SEX_MALE;
				SexMouseIn( nSelect );
	
				if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
					SexSelect( nSelect );
				}
			}
		}break;

	case CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_BRAWLER:
	case CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_SWORDSMAN:
	case CHARACTER_CREATE_PAGE_TYPE_CLASS_FEMALE_ARCHER:
	case CHARACTER_CREATE_PAGE_TYPE_CLASS_FEMALE_SHAMAN:
	case CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_EXTREME:
	case CHARACTER_CREATE_PAGE_TYPE_CLASS_FEMALE_EXTREME:
	case CHARACTER_CREATE_PAGE_TYPE_CLASS_FEMALE_BRAWLER:
	case CHARACTER_CREATE_PAGE_TYPE_CLASS_FEMALE_SWORDSMAN:
	case CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_ARCHER:
	case CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_SHAMAN:
	case CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_GUNNER:
	case CHARACTER_CREATE_PAGE_TYPE_CLASS_FEMALE_GUNNER:
	case CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_ASSASSIN:
	case CHARACTER_CREATE_PAGE_TYPE_CLASS_FEMALE_ASSASSIN:
	case CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_TRICKER:
	case CHARACTER_CREATE_PAGE_TYPE_CLASS_FEMALE_TRICKER:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) ) {
				int nSelect = ControlID - CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_BRAWLER;
				ClassMouseIn( nSelect );

				if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) ){
					ClassSelect( nSelect );
				}
			}
		}break;

	};
}

void CCharacterCreatePageSet::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	for( int i=0; i<GLSCHOOL_NUM; ++i ){
		m_pTypeSchool_Box[i]->SetVisibleSingle( FALSE );
	}

	for( int i=0; i<2; ++i ){
		m_pTypeSex_Box[i]->SetVisibleSingle( FALSE );
	}

	for( int i=0; i<GLCI_NUM_8CLASS; ++i ){
		m_pTypeClass_Box[i]->SetVisibleSingle( FALSE );
	}

	CUIGroup::Update( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( m_nSelectedSchool != m_nSelectedLastSchool || 
		 m_nSelectedSex != m_nSelectedLastSex ||
		  m_nSelectedClass != m_nSelectedLastClass )
	{
		UpdateUI();

		m_nSelectedLastSchool = m_nSelectedSchool;
		m_nSelectedLastSex = m_nSelectedSex;
		m_nSelectedLastClass = m_nSelectedClass;
	}
}

void CCharacterCreatePageSet::ResetAll()
{
	m_nSelectedSchool = -1;
	m_nSelectedSex = -1;
	m_nSelectedClass = -1;

	for( int i=0; i<GLSCHOOL_NUM; ++i ){
		m_pTypeSchool_Off[i]->SetVisibleSingle( TRUE );
		m_pTypeSchool_On[i]->SetVisibleSingle( FALSE );
		m_pTypeSchool_Box[i]->SetVisibleSingle( FALSE );
		m_pTypeSchool_Focus[i]->SetVisibleSingle( TRUE );
	}

	for( int i=0; i<2; ++i ){
		m_pTypeSex_Off[i]->SetVisibleSingle( TRUE );
		m_pTypeSex_On[i]->SetVisibleSingle( FALSE );
		m_pTypeSex_Box[i]->SetVisibleSingle( FALSE );
		m_pTypeSex_Focus[i]->SetVisibleSingle( TRUE );
	}

	for( int i=0; i<GLCI_NUM_8CLASS; ++i ){
		m_pTypeClass_On[i]->SetVisibleSingle( FALSE );
		m_pTypeClass_Box[i]->SetVisibleSingle( FALSE );
	}

	for( int i=0; i<GLCI_NUM_8CLASS; ++i ){
		BOOL bVisible = ( i == GLCI_BRAWLER_M || 
			i == GLCI_SWORDSMAN_M ||
			i == GLCI_ARCHER_M ||
			i == GLCI_SHAMAN_M ||
			i == GLCI_EXTREME_M ||
			i == GLCI_GUNNER_M ||
			i == GLCI_ASSASSIN_M||
			i == GLCI_TRICKER_M );

		if ( i == GLCI_EXTREME_M && !COuterInterface::GetInstance().GetCharRemainExM() ){
			bVisible = FALSE;
		}

		if ( i == GLCI_EXTREME_W && !COuterInterface::GetInstance().GetCharRemainExW() ){
			bVisible = FALSE;
		}

		if ( (i == GLCI_GUNNER_M || i == GLCI_GUNNER_W ) && !m_bCreateClassGunner ){
			bVisible = FALSE;
		}

		if ( (i == GLCI_ASSASSIN_M || i == GLCI_ASSASSIN_W ) && !m_bCreateClassAssassin ){
			bVisible = FALSE;
		}

		if ( (i == GLCI_TRICKER_M || i == GLCI_TRICKER_W ) && !m_bCreateClassMagician ){
			bVisible = FALSE;
		}

		m_pTypeClass_Off[i]->SetVisibleSingle( bVisible );
		m_pTypeClass_Focus[i]->SetVisibleSingle( bVisible );
	}
}

void CCharacterCreatePageSet::UpdateUI()
{
	//m_bCreateClassGunner = RANPARAM::bCreateClass[5];
	//m_bCreateClassAssassin = RANPARAM::bCreateClass[6];
	//m_bCreateClassMagician = RANPARAM::bCreateClass[7];

	for( int i=0; i<GLSCHOOL_NUM; ++i ){
		m_pTypeSchool_Off[i]->SetVisibleSingle( TRUE );
		m_pTypeSchool_On[i]->SetVisibleSingle( FALSE );
	}

	for( int i=0; i<2; ++i ){
		m_pTypeSex_Off[i]->SetVisibleSingle( TRUE );
		m_pTypeSex_On[i]->SetVisibleSingle( FALSE );
	}

	for( int i=0; i<GLCI_NUM_8CLASS; ++i ){
		BOOL bVisible = ( i == GLCI_BRAWLER_M || 
			i == GLCI_SWORDSMAN_M ||
			i == GLCI_ARCHER_M ||
			i == GLCI_SHAMAN_M ||
			i == GLCI_EXTREME_M ||
			i == GLCI_GUNNER_M ||
			i == GLCI_ASSASSIN_M ||
			i == GLCI_TRICKER_M );

		if ( i == GLCI_EXTREME_M && !COuterInterface::GetInstance().GetCharRemainExM() ){
			bVisible = FALSE;
		}

		if ( i == GLCI_EXTREME_W && !COuterInterface::GetInstance().GetCharRemainExW() ){
			bVisible = FALSE;
		}

		if ( (i == GLCI_GUNNER_M || i == GLCI_GUNNER_W ) && !m_bCreateClassGunner ){
			bVisible = FALSE;
		}

		if ( (i == GLCI_ASSASSIN_M || i == GLCI_ASSASSIN_W ) && !m_bCreateClassAssassin ){
			bVisible = FALSE;
		}

		if ( (i == GLCI_TRICKER_M || i == GLCI_TRICKER_W ) && !m_bCreateClassMagician ){
			bVisible = FALSE;
		}

		m_pTypeClass_On[i]->SetVisibleSingle( FALSE );
		m_pTypeClass_Off[i]->SetVisibleSingle( bVisible );
		m_pTypeClass_Focus[i]->SetVisibleSingle( bVisible );
	}

	BOOL bValidSchool = FALSE;
	BOOL bValidSex = FALSE;
	BOOL bValidClass = FALSE;

	if ( m_nSelectedSchool >= 0 && m_nSelectedSchool < GLSCHOOL_NUM )
	{
		m_pTypeSchool_On[m_nSelectedSchool]->SetVisibleSingle( TRUE );
		bValidSchool = TRUE;
	}

	if ( m_nSelectedSex >= 0 && m_nSelectedSex < 2 )
	{
		m_pTypeSex_On[m_nSelectedSex]->SetVisibleSingle( TRUE );
		bValidSex = TRUE;

		for( int i=0; i<GLCI_NUM_8CLASS; ++i ){
			m_pTypeClass_Off[i]->SetVisibleSingle( FALSE );
			m_pTypeClass_Focus[i]->SetVisibleSingle( FALSE );
		}

		int nMale[GLCI_NUM_8CLASS/2] = { GLCI_BRAWLER_M, GLCI_SWORDSMAN_M, GLCI_ARCHER_M, GLCI_SHAMAN_M, GLCI_EXTREME_M, GLCI_GUNNER_M, GLCI_ASSASSIN_M, GLCI_TRICKER_M };
		int nFemale[GLCI_NUM_8CLASS/2] = { GLCI_BRAWLER_W, GLCI_SWORDSMAN_W, GLCI_ARCHER_W, GLCI_SHAMAN_W, GLCI_EXTREME_W, GLCI_GUNNER_W, GLCI_ASSASSIN_W, GLCI_TRICKER_W };

		for ( int i=0; i<(GLCI_NUM_8CLASS/2); ++ i ){
			int nSelect = (m_nSelectedSex == 1)? nFemale[i] : nMale[i];
			if ( nSelect == GLCI_EXTREME_M && !COuterInterface::GetInstance().GetCharRemainExM() )	continue;
			if ( nSelect == GLCI_EXTREME_W && !COuterInterface::GetInstance().GetCharRemainExW() )	continue;

			if ( nSelect == GLCI_GUNNER_M && !m_bCreateClassGunner )	continue;
			if ( nSelect == GLCI_GUNNER_W && !m_bCreateClassGunner )	continue;

			if ( nSelect == GLCI_ASSASSIN_M && !m_bCreateClassAssassin )	continue;
			if ( nSelect == GLCI_ASSASSIN_W && !m_bCreateClassAssassin )	continue;

			if ( nSelect == GLCI_TRICKER_M && !m_bCreateClassMagician )	continue;
			if ( nSelect == GLCI_TRICKER_W && !m_bCreateClassMagician  )	continue;

			m_pTypeClass_Off[nSelect]->SetVisibleSingle( TRUE );
			m_pTypeClass_Focus[nSelect]->SetVisibleSingle( TRUE );
		}
	}

	if ( m_nSelectedClass >= 0 && m_nSelectedClass < GLCI_NUM_8CLASS )
	{
		m_pTypeClass_On[m_nSelectedClass]->SetVisibleSingle( TRUE );
		bValidClass = TRUE;
	}

	COuterInterface::GetInstance().GetCharacterCreateInfo()->SetData( m_nSelectedSchool, m_nSelectedSex, m_nSelectedClass );

	if ( m_nSelectedClass == GLCI_EXTREME_M && !COuterInterface::GetInstance().GetCharRemainExM() ){
		bValidClass = FALSE;
	}

	if ( m_nSelectedClass == GLCI_EXTREME_W && !COuterInterface::GetInstance().GetCharRemainExW() ){
		bValidClass = FALSE;
	}

	if ( ( m_nSelectedClass == GLCI_GUNNER_M || m_nSelectedClass == GLCI_GUNNER_W ) && !m_bCreateClassGunner ){
		bValidClass = FALSE;
	}

	if ( ( m_nSelectedClass == GLCI_ASSASSIN_M || m_nSelectedClass == GLCI_ASSASSIN_W ) && !m_bCreateClassAssassin ){
		bValidClass = FALSE;
	}

	if ( ( m_nSelectedClass == GLCI_TRICKER_M || m_nSelectedClass == GLCI_TRICKER_W ) && !m_bCreateClassMagician ){
		bValidClass = FALSE;
	}

	if ( bValidSchool && bValidSex && bValidClass )
	{
		DxGlobalStage::GetInstance().GetLobyStage()->SelCharSchool( m_nSelectedSchool );
		DxGlobalStage::GetInstance().GetLobyStage()->SelCharClass( m_nSelectedClass );
		DxGlobalStage::GetInstance().GetLobyStage()->SetStyleStep( TRUE );
	}else{
		DxGlobalStage::GetInstance().GetLobyStage()->SetStyleStep( FALSE );
	}
}

void CCharacterCreatePageSet::SchoolSelect( int nSelect )
{
	if ( nSelect < 0 )	return;
	if ( nSelect >= GLSCHOOL_NUM )	return;
	if ( nSelect == m_nSelectedSchool )	return;

	m_nSelectedSchool = nSelect;
	m_nSelectedSex = -1;
	m_nSelectedClass = -1;
}

void CCharacterCreatePageSet::SexSelect( int nSelect )
{
	if ( nSelect < 0 )	return;
	if ( nSelect >= 2 )	return;
	if ( nSelect == m_nSelectedSex )	return;

	if ( m_nSelectedSchool < 0 || m_nSelectedSchool >= GLSCHOOL_NUM )
	{
		DoModalOuter ( ID2GAMEEXTEXT ( "NEW_CHAR_SELECT_SCHOOL_ERROR" ), MODAL_INFOMATION, OK );
		return;
	}

	m_nSelectedSex = nSelect;
	m_nSelectedClass = -1;
}

void CCharacterCreatePageSet::ClassSelect( int nSelect )
{
	if ( nSelect < 0 )	return;
	if ( nSelect >= GLCI_NUM_8CLASS )	return;

	if ( m_nSelectedSchool < 0 || m_nSelectedSchool >= GLSCHOOL_NUM )
	{
		DoModalOuter ( ID2GAMEEXTEXT ( "NEW_CHAR_SELECT_SCHOOL_ERROR" ), MODAL_INFOMATION, OK );
		return;
	}

	if ( m_nSelectedSex < 0 || m_nSelectedSex >= 2 )
	{
		DoModalOuter ( ID2GAMEEXTEXT ( "NEW_CHAR_SELECT_SEX_ERROR" ), MODAL_INFOMATION, OK );
		return;
	}

	m_nSelectedClass = nSelect;
}

void CCharacterCreatePageSet::SchoolMouseIn( int nSelect )
{
	if ( nSelect < 0 )	return;
	if ( nSelect >= GLSCHOOL_NUM )	return;
	m_pTypeSchool_Box[nSelect]->SetVisibleSingle( TRUE );
}

void CCharacterCreatePageSet::SexMouseIn( int nSelect )
{
	if ( nSelect < 0 )	return;
	if ( nSelect >= 2 )	return;
	m_pTypeSex_Box[nSelect]->SetVisibleSingle( TRUE );
}

void CCharacterCreatePageSet::ClassMouseIn( int nSelect )
{
	if ( nSelect < 0 )	return;
	if ( nSelect >= GLCI_NUM_8CLASS )	return;
	m_pTypeClass_Box[nSelect]->SetVisibleSingle( TRUE );
}
