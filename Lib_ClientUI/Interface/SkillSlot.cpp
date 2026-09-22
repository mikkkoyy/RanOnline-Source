#include "StdAfx.h"
#include "SkillSlot.h"
#include "GameTextControl.h"
#include "UITextControl.h"
#include "SkillImage.h"
#include "GLGaeaClient.h"
#include "InnerInterface.h"
#include "SkillWindowToTray.h"
#include "ModalWindow.h"
#include "ModalCallerID.h"
#include "../Lib_ClientUI/Interface/BasicButton.h"
#include "../Lib_ClientUI/Interface/BasicTextBox.h"
#include "../Lib_Engine/GUInterface/UIKeyCheck.h"
#include "../Lib_Engine/GUInterface/BasicButtonText.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "./Util/RnButton.h"
#include "BasicLineBox.h"
#include "BasicLineBoxEx.h"
#include "SkillTrayTab.h"
#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSkillSlot::CSkillSlot () :
	m_pNotLearnImage ( NULL ),
	m_pSkillImage ( NULL ),
	m_pSkillUpButton ( NULL ),
	m_pTextBox ( NULL ),
	m_pButtonLearn ( NULL ),
	m_pButtonAdjust ( NULL ),
	m_pButtonReset ( NULL ),
	m_pTextBoxLine1 ( NULL ),
	m_pTextBoxLine2 ( NULL ),
	m_sNativeID ( NATIVEID_NULL () ),
	m_bLearnSkill ( FALSE ),
	m_bCanLearn ( FALSE ),
	m_bLEVELUP_ENABLE ( FALSE ),
	m_pSkillSlotAvtive ( NULL ),
	m_wLevel ( 0 ),
	nLevel(0)
{
}

CSkillSlot::~CSkillSlot ()
{
}

void CSkillSlot::CreateSubControl ( SNATIVEID sNativeID )
{
	D3DCOLOR dwColor = NS_UITEXTCOLOR::DEFAULT;

	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );
	int	nTextAlign = TEXT_ALIGN_LEFT;

	if ( sNativeID == NATIVEID_NULL () )
	{
//		GASSERT ( 0 && "무효화한 ID 입니다. 확인하십시오." );
	}
	else
	{
		CreateControl ( "RNSKILL_SKILL_SLOT_IMAGE" );

		{
			CBasicLineBoxEx* pLineBox = new CBasicLineBoxEx;
			pLineBox->CreateSub ( this, "BASIC_LINE_BOX_SKILL_BACK", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
			pLineBox->CreateBaseBoxDialogue ( "RNSKILL_SLOT_SIZE" );
			RegisterControl ( pLineBox );
		}
		{
			m_pSkillSlotAvtive = CreateControl ( "RNSKILL_SLOT_ACTIVE" );
			m_pSkillSlotAvtive->SetVisibleSingle( FALSE );
		}
	
		{
			m_pLineBoxGreen = new CBasicLineBoxEx;
			m_pLineBoxGreen->CreateSub ( this, "BASIC_LINE_BOX_EX_SKILL_GREEN", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
			m_pLineBoxGreen->CreateBaseBoxSkillGreen ( "RNSKILL_SLOT_GREEN" );
			m_pLineBoxGreen->SetVisibleSingle ( FALSE );
			RegisterControl ( m_pLineBoxGreen );

			m_pLineBoxBlue = new CBasicLineBoxEx;
			m_pLineBoxBlue->CreateSub ( this, "BASIC_LINE_BOX_EX_SKILL_BLUE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
			m_pLineBoxBlue->CreateBaseBoxSkillBlue ( "RNSKILL_SLOT_BLUE" );
			m_pLineBoxBlue->SetVisibleSingle ( FALSE );
			RegisterControl ( m_pLineBoxBlue );

			m_pLineBoxYellow = new CBasicLineBoxEx;
			m_pLineBoxYellow->CreateSub ( this, "BASIC_LINE_BOX_EX_SKILL_YELLOW", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
			m_pLineBoxYellow->CreateBaseBoxSkillYellow ( "RNSKILL_SLOT_YELLOW" );
			m_pLineBoxYellow->SetVisibleSingle ( FALSE );
			RegisterControl ( m_pLineBoxYellow );
		}

		{
			CSkillImage* pSkillImage = CreateSkillImage ( SKILL_IMAGE );
			pSkillImage->SetSkill ( sNativeID );
			m_pSkillImage = pSkillImage;		
		}	

		{
			m_pTextBoxLine1 = CreateStaticControl ( "RNSKILL_SKILL_SLOT_TEXT0", pFont, dwColor, nTextAlign );
			m_pTextBoxLine2 = CreateStaticControl ( "RNSKILL_SKILL_SLOT_TEXT1", pFont, dwColor, nTextAlign );
		}

		{
			m_pNotLearnImage = CreateControl ( "RNSKILL_SLOT_IMAGE_NOTLEARN" );
		}

		{
			RnButton* pRnButton = NULL;
			RnButton::CreateArg arg;
			//////////////////////////////////////////////////////////////////////////
			arg.defaultTextureKey = "RNBUTTON_DEFAULT_TEXTURE";
			arg.mouseOverTextureKey = "RNBUTTON_OVER_TEXTURE";
			arg.mouseClickTextureKey = "RNBUTTON_CLICK_TEXTURE";
			//////////////////////////////////////////////////////////////////////////
			arg.pFont = pFont;
			arg.dwColor = NS_UITEXTCOLOR::WHITE;

			m_pButtonLearn = new RnButton();
			m_pButtonLearn->CreateSub(this,"RNSKILL_SLOT_LEARN_BUTTON",UI_FLAG_DEFAULT,SKILL_SLOT_LEVEL_UP);
			arg.text = ("+");
			m_pButtonLearn->CreateSubControl(arg);
			RegisterControl(m_pButtonLearn);
		}
	}

	m_sNativeID = sNativeID;
	nLevel = 0;
}

CSkillImage* CSkillSlot::CreateSkillImage ( UIGUID ControlID )
{
	CSkillImage* pSkillImage = new CSkillImage;
	pSkillImage->CreateSub ( this, "RNSKILL_SKILL_SLOT_IMAGE", UI_FLAG_DEFAULT, ControlID );
	pSkillImage->CreateSubControl ();
	pSkillImage->SetUseRender ( TRUE );
    RegisterControl ( pSkillImage );
	return pSkillImage;
}

CUIControl*	CSkillSlot::CreateControl ( char* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl );
	RegisterControl ( pControl );
	return pControl;
}

CBasicButton* CSkillSlot::CreateFlipButton ( char* szButton, char* szButtonFlip, char* szMouseOver, UIGUID ControlID )
{
	CBasicButton* pButton = new CBasicButton;
	pButton->CreateSub ( this, szButton, UI_FLAG_DEFAULT, ControlID );
	pButton->CreateFlip ( szButtonFlip, CBasicButton::RADIO_FLIP );
	pButton->CreateMouseOver ( szMouseOver );
	RegisterControl ( pButton );
	return pButton;
}

CBasicTextBox* CSkillSlot::CreateStaticControl ( char* szControlKeyword, CD3DFontPar* pFont, D3DCOLOR dwColor, int nAlign )
{
	CBasicTextBox* pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, szControlKeyword );
	pStaticText->SetFont ( pFont );
	pStaticText->SetTextAlign ( nAlign );
	RegisterControl ( pStaticText );
	return pStaticText;
}
void CSkillSlot::SKILLSLOT_ACTIVE()
{
	if ( m_sNativeID == NATIVEID_NULL () )
	{
		return ;
	}

	if ( m_bSKILLSLOT_ACTIVE && m_bLearnSkill )
	{
		m_pSkillSlotAvtive->SetVisibleSingle( TRUE );
		m_pLineBoxBlue->SetVisibleSingle ( TRUE );
	}
	else
	{
		if ( m_bCanLearn )
			m_pLineBoxGreen->SetVisibleSingle ( TRUE );
		else
			m_pLineBoxYellow->SetVisibleSingle ( TRUE );
	}
}
void CSkillSlot::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{	
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( m_sNativeID == NATIVEID_NULL () )
	{
		return ;
	}

	{
		m_pSkillSlotAvtive->SetVisibleSingle( FALSE );
		m_pLineBoxYellow->SetVisibleSingle ( FALSE );
		m_pLineBoxGreen->SetVisibleSingle ( FALSE );
		m_pLineBoxBlue->SetVisibleSingle ( FALSE );
	}


	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_sNativeID.wMainID, m_sNativeID.wSubID );
	if ( !pSkill )
	{
		m_pButtonLearn->SetVisibleSingle ( FALSE );
		GASSERT ( 0 && "찾을 수 없는 스킬입니다." );
		return ;
	}

	m_bLearnSkill = FALSE;
	BOOL bMASTER = GLGaeaClient::GetInstance().GetCharacter()->ISMASTER_SKILL ( m_sNativeID );
	if ( m_pTextBoxLine1 && m_pTextBoxLine2 ) 
	{
		SCHARSKILL* pCharSkill = GLGaeaClient::GetInstance().GetCharacter()->GETLEARNED_SKILL ( m_sNativeID );
		WORD pCharLevel = GLGaeaClient::GetInstance().GetCharacter()->m_wLevel;
		if ( pCharSkill )
		{
			m_bLearnSkill = TRUE;
			m_bSKILLSLOT_ACTIVE = TRUE;

			WORD wLevel = pCharSkill->wLevel + 1;
			if ( m_wLevel != wLevel )
			{
				m_wLevel = wLevel;
				SKILL::SLEARN_LVL sLVL = pSkill->m_sLEARN.sLVL_STEP[wLevel];

				m_strLine1.Format ( "%s", pSkill->GetName() );

				if ( !bMASTER )
				{
					m_strLine2.Format ( "%d %s %s. %d", pSkill->m_sBASIC.dwGRADE, (char*)ID2GAMEWORD ( "RNSKILL_SLOT", 0 ), (char*)ID2GAMEWORD ( "RNSKILL_SLOT", 3 ), wLevel + nLevel );
				}
				else
					m_strLine2.Format ( "%s", (char*)ID2GAMEWORD ( "RNSKILL_SLOT", 4 ) );			

				m_pTextBoxLine1->ClearText ();
				m_pTextBoxLine2->ClearText ();
				m_pTextBoxLine1->SetText ( m_strLine1, NS_UITEXTCOLOR::WHITE );
				m_pTextBoxLine2->SetText ( m_strLine2, NS_UITEXTCOLOR::GOLD );

				AddMessageEx ( UIMSG_LEVEL_UP | UIMSG_TOTOPPARENT );
			}
		}		
		else	
		{
			m_bSKILLSLOT_ACTIVE = FALSE;

			if (m_bCanLearn)
			{
				m_strLine1.Format("%s", pSkill->GetName());
				m_pTextBoxLine1->ClearText();
				m_pTextBoxLine1->SetText(m_strLine1, NS_UITEXTCOLOR::SILVER);
			}
			else
			{
				m_strLine1.Format("%s", pSkill->GetName());
				m_pTextBoxLine1->ClearText();
				m_pTextBoxLine1->SetText(m_strLine1, NS_UITEXTCOLOR::RED);
			}

			SKILL::SLEARN_LVL sLVL = pSkill->m_sLEARN.sLVL_STEP[0];
			SNATIVEID NeedSkillID = pSkill->m_sLEARN.sSKILL;
	
			BOOL NEEDSKILL = FALSE;
			BOOL NEEDSTATUS = FALSE;
			BOOL NEEDLEVEL = FALSE;
			BOOL FONT_VALID = FALSE;

			BOOL bNEEDSKP = FALSE;
			DWORD dwSkillPoint = GLGaeaClient::GetInstance().GetCharacter ()->m_dwSkillPoint;
			if ( sLVL.dwSKP <= dwSkillPoint )
			{
				bNEEDSKP = TRUE;
			}
			if ( pCharLevel < sLVL.dwLEVEL )
			{
				NEEDLEVEL = TRUE;
			}			
			if ( NeedSkillID != NATIVEID_NULL() )
			{
				NEEDSKILL = TRUE;
			}
			
			const GLCHARLOGIC& sCharData = GLGaeaClient::GetInstance().GetCharacterLogic ();	
			

			if ( sCharData.m_sSUMSTATS.wPow < sLVL.sSTATS.wPow || 
				sCharData.m_sSUMSTATS.wDex < sLVL.sSTATS.wDex ||
				sCharData.m_sSUMSTATS.wSpi < sLVL.sSTATS.wSpi ||
				sCharData.m_sSUMSTATS.wSta < sLVL.sSTATS.wSta ||
				sCharData.m_sSUMSTATS.wStr < sLVL.sSTATS.wStr ||
				sCharData.m_sSUMSTATS.wInt < sLVL.sSTATS.wInt )
			{
				NEEDSTATUS = TRUE;
			}

			if ( NEEDLEVEL && !NEEDSKILL && !NEEDSTATUS ) 
			{
				m_strLine2.Format ( "%s %d", (char*)ID2GAMEWORD ( "RNSKILL_SLOT", 3 ), sLVL.dwLEVEL );
			}
			else if ( NEEDLEVEL && NEEDSKILL  ) 
			{
				m_strLine2.Format ( "%s %d / %s", (char*)ID2GAMEWORD ( "RNSKILL_SLOT", 3 ), sLVL.dwLEVEL ,(char*)ID2GAMEWORD ( "RNSKILL_SLOT", 5 ) );
			}
			else if ( NEEDLEVEL && !NEEDSKILL  && NEEDSTATUS )
			{
				if ( sCharData.m_sSUMSTATS.wPow < sLVL.sSTATS.wPow )
				{
					m_strLine2.Format("%s. %d / %s %d",(char*)ID2GAMEWORD ( "RNSKILL_SLOT", 3 ), sLVL.dwLEVEL ,ID2GAMEWORD( "RNSKILL_SLOT", 6 ), sLVL.sSTATS.wPow );
				}
				else if ( sCharData.m_sSUMSTATS.wStr < sLVL.sSTATS.wStr )
				{
					m_strLine2.Format("%s. %d / %s %d",(char*)ID2GAMEWORD ( "RNSKILL_SLOT", 3 ), sLVL.dwLEVEL ,ID2GAMEWORD( "RNSKILL_SLOT", 9 ), sLVL.sSTATS.wStr);
				}
				else if ( sCharData.m_sSUMSTATS.wSpi < sLVL.sSTATS.wSpi )
				{
					m_strLine2.Format("%s. %d / %s %d",(char*)ID2GAMEWORD ( "RNSKILL_SLOT", 3 ), sLVL.dwLEVEL ,ID2GAMEWORD( "RNSKILL_SLOT", 8 ), sLVL.sSTATS.wSpi);
				}
				else if ( sCharData.m_sSUMSTATS.wDex < sLVL.sSTATS.wDex )
				{
					m_strLine2.Format("%s. %d / %s %d",(char*)ID2GAMEWORD ( "RNSKILL_SLOT", 3 ), sLVL.dwLEVEL ,ID2GAMEWORD( "RNSKILL_SLOT", 7 ), sLVL.sSTATS.wDex);
				}
				else if ( sCharData.m_sSUMSTATS.wInt < sLVL.sSTATS.wInt )
				{
					m_strLine2.Format("%s. %d / %s %d",(char*)ID2GAMEWORD ( "RNSKILL_SLOT", 3 ), sLVL.dwLEVEL ,ID2GAMEWORD( "RNSKILL_SLOT", 11 ), sLVL.sSTATS.wInt);
				}
				else if ( sCharData.m_sSUMSTATS.wSta < sLVL.sSTATS.wSta )
				{
					m_strLine2.Format("%s. %d / %s %d",(char*)ID2GAMEWORD ( "RNSKILL_SLOT", 3 ), sLVL.dwLEVEL ,ID2GAMEWORD( "RNSKILL_SLOT", 10 ), sLVL.sSTATS.wSta);
				}
			}
			else if ( !NEEDLEVEL && !NEEDSKILL  && NEEDSTATUS )
			{
				if ( sCharData.m_sSUMSTATS.wPow < sLVL.sSTATS.wPow )
				{
					m_strLine2.Format("%s %d",ID2GAMEWORD( "RNSKILL_SLOT", 6 ), sLVL.sSTATS.wPow );
				}
				else if ( sCharData.m_sSUMSTATS.wStr < sLVL.sSTATS.wStr )
				{
					m_strLine2.Format("%s %d",ID2GAMEWORD( "RNSKILL_SLOT", 9 ), sLVL.sSTATS.wStr);
				}
				else if ( sCharData.m_sSUMSTATS.wSpi < sLVL.sSTATS.wSpi )
				{
					m_strLine2.Format("%s %d",ID2GAMEWORD( "RNSKILL_SLOT", 8 ), sLVL.sSTATS.wSpi);
				}
				else if ( sCharData.m_sSUMSTATS.wDex < sLVL.sSTATS.wDex )
				{
					m_strLine2.Format("%s %d",ID2GAMEWORD( "RNSKILL_SLOT", 7 ), sLVL.sSTATS.wDex);
				}
				else if ( sCharData.m_sSUMSTATS.wInt < sLVL.sSTATS.wInt )
				{
					m_strLine2.Format("%s %d",ID2GAMEWORD( "RNSKILL_SLOT", 11 ), sLVL.sSTATS.wInt);
				}
				else if ( sCharData.m_sSUMSTATS.wSta < sLVL.sSTATS.wSta )
				{
					m_strLine2.Format("%s %d",ID2GAMEWORD( "RNSKILL_SLOT", 10 ), sLVL.sSTATS.wSta);
				}
			}
			else
			{
				if ( bNEEDSKP )
				{
					m_bCanLearn = TRUE;
					FONT_VALID = TRUE;
					m_bSKILLSLOT_ACTIVE = FALSE;
					m_strLine2.Format ( "%s %d %s. %d", (char*)ID2GAMEWORD ( "RNSKILL_SLOT", 0 ), pSkill->m_sBASIC.dwGRADE, (char*)ID2GAMEWORD ( "RNSKILL_SLOT", 3 ), nLevel );
				}
				else
				{	
					m_bCanLearn = FALSE;
					FONT_VALID = FALSE;
					m_bSKILLSLOT_ACTIVE = TRUE;
					m_strLine2.Format ( "Skill Point %d", sLVL.dwSKP);
				}
			}

			GLCharacter* const pCharacter = GLGaeaClient::GetInstance().GetCharacter();
			if ( pCharacter->m_lnMoney <= int(GLCONST_CHAR::lnSKILL_PREREQ[pSkill->m_sBASIC.sNATIVEID.wMainID][pSkill->m_sBASIC.sNATIVEID.wSubID])) m_bCanLearn = FALSE;
			
			m_pTextBoxLine2->ClearText ();
			if ( !FONT_VALID )
				m_pTextBoxLine2->SetText ( m_strLine2, NS_UITEXTCOLOR::RED );
			else
				m_pTextBoxLine2->SetText ( m_strLine2, NS_UITEXTCOLOR::SILVER );
		}
	}

	if ( m_pNotLearnImage ) m_pNotLearnImage->SetVisibleSingle ( !m_bLearnSkill );

	if ( m_bLearnSkill )
	{
		if ( bMASTER )
		{
			m_pButtonLearn->SetVisibleSingle ( FALSE );
		}
		else
		{
			m_bLEVELUP_ENABLE = GLGaeaClient::GetInstance().GetCharacter()->CHECKLEARNABLE_SKILL ( m_sNativeID ) == EMSKILL_LEARN_OK;
			m_pButtonLearn->SetVisibleSingle ( TRUE );
		}
	}
	else
	{
		m_pButtonLearn->SetVisibleSingle ( TRUE );
		m_bLEVELUP_ENABLE = FALSE;
	}

	if ( m_bLearnSkill )
	{
		if ( m_pSkillImage ) m_pSkillImage->SetSkillProcess ( m_sNativeID );
	}
}

void CSkillSlot::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	switch ( ControlID )
	{
		case SKILL_IMAGE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				AddMessageEx ( UIMSG_MOUSEIN_SKILLIMAGE | UIMSG_TOTOPPARENT );
			}

			if ( CHECK_MOUSE_IN ( dwMsg ) && m_bLearnSkill )
			{
				if ( dwMsg & UIMSG_LB_DOWN )
				{
					PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_sNativeID.wMainID, m_sNativeID.wSubID );
					if ( !pSkill )
					{
						GASSERT ( 0 && "?? ? ?? ?????." );
						return ;
					}
					//	??? ??? ? ? ??.
					if ( pSkill->m_sBASIC.emROLE == SKILL::EMROLE_PASSIVE ) return ;

					{
						CSkillWindowToTray* pSkillWindowToTray = CInnerInterface::GetInstance().GetSkillWindowToTray();
						if (!pSkillWindowToTray) return;

						pSkillWindowToTray->SetSkill(m_sNativeID);
					}

					CInnerInterface::GetInstance().HideGroup ( QUICK_SKILL_TRAY_OPEN_BUTTON );
					CInnerInterface::GetInstance().ShowGroupBottom ( QUICK_SKILL_TRAY_TAB_WINDOW );
				}

				{
					for( int i = 0; i < QUICK_SKILL_SLOT_MAX; ++i)
					{
						if ( UIKeyCheck::GetInstance()->Check ( RANPARAM::SkillSlot[i], DXKEY_DOWN ) )
						{
							PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_sNativeID.wMainID, m_sNativeID.wSubID );
							if ( !pSkill )
							{
								GASSERT ( 0 && "?? ? ?? ?????." );
								return ;
							}
							if ( pSkill->m_sBASIC.emROLE == SKILL::EMROLE_PASSIVE ) return ;

							INT nIndex = CInnerInterface::GetInstance().GetSkillTrayTab()->GetTabIndex() * QUICK_SKILL_SLOT_MAX + i;
							GLGaeaClient::GetInstance().GetCharacter()->ReqSkillQuickSet (m_sNativeID, nIndex );
						}
					}
				}
			}
		}
		break;

	case SKILL_SLOT_LEVEL_UP:
		{
			//	스킬 레벨 업
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( m_bLearnSkill )
					AddMessageEx ( UIMSG_MOUSEIN_BUTTON | UIMSG_TOTOPPARENT );
				else
					AddMessageEx ( UIMSG_MOUSEIN_BUTTON2 | UIMSG_TOTOPPARENT );

				if ( m_bCanLearn && dwMsg & UIMSG_LB_UP )
				{
					GLGaeaClient::GetInstance().GetCharacter()->ReqNonInvenSkill ( m_sNativeID );
				}

				if ( m_bLEVELUP_ENABLE && dwMsg & UIMSG_LB_UP )
				{
					SCHARSKILL* pCharSkill = GLGaeaClient::GetInstance().GetCharacter()->GETLEARNED_SKILL ( m_sNativeID );					
					BOOL bMASTER = GLGaeaClient::GetInstance().GetCharacter()->ISMASTER_SKILL ( m_sNativeID );
					if ( pCharSkill )
					{
						if ( !bMASTER )
						{
							PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( m_sNativeID.wMainID, m_sNativeID.wSubID );
							if ( !pSkill )
							{
								GASSERT ( 0 && "" );
								return ;
							}

							bool bLVLUP = GLGaeaClient::GetInstance().GetCharacter()->CHECKLEARNABLE_SKILL ( m_sNativeID ) == EMSKILL_LEARN_OK;
							CInnerInterface::GetInstance().SetSkillUpID ( m_sNativeID.dwID );

							CString strLevelUp = CInnerInterface::GetInstance().MakeString ( ID2GAMEINTEXT("SKILL_LEVEL_UP_CONFIRM"), pSkill->GetName() );                            
							DoModal ( strLevelUp, MODAL_QUESTION, YESNO, MODAL_SKILL_LEVEL_UP_CONFIRM );//							
						}
					}
				}
			}
		}
		break;
	}

	CUIGroup::TranslateUIMessage( ControlID, dwMsg );
}