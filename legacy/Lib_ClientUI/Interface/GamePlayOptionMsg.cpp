#include "StdAfx.h"
#include "GamePlayOption.h"
#include "./BasicButton.h"
#include "./BasicTextBox.h"
#include "DxParamSet.h"
#include "InnerInterface.h"
#include "ModalCallerID.h"
#include "ModalWindow.h"
#include "GameTextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CBasicGamePlayOption::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	switch ( ControlID )
	{
	case HWOPTION_GAMEPLAY_CONFT_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				//DXPARAMSET::GetInstance().m_bDIS_CONFT = !DXPARAMSET::GetInstance().m_bDIS_CONFT;
				//LoadConft ();
				m_bConft = !m_bConft;
				m_pConftButton->SetFlip( m_bConft );
			}
		}
		break;

	case HWOPTION_GAMEPLAY_REVIVAL_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				//DXPARAMSET::GetInstance().m_bDIS_Revival = !DXPARAMSET::GetInstance().m_bDIS_Revival;
				//LoadRevival ();
				m_bNon_Rebirth = !m_bNon_Rebirth;
				m_pRevivalButton->SetFlip( m_bNon_Rebirth );
			}
		}
		break;

	case HWOPTION_GAMEPLAY_TRADE_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				//DXPARAMSET::GetInstance().m_bDIS_TRADE = !DXPARAMSET::GetInstance().m_bDIS_TRADE;
				//LoadTrade ();
				m_bTrade = !m_bTrade;
				m_pTradeButton->SetFlip ( m_bTrade );
			}
		}
		break;

	case HWOPTION_GAMEPLAY_PARTY_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				//DXPARAMSET::GetInstance().m_bDIS_PARTY = !DXPARAMSET::GetInstance().m_bDIS_PARTY;
				//LoadParty ();
				m_bParty = !m_bParty;
				m_pPartyButton->SetFlip ( m_bParty );
			}
		}
		break;

	case HWOPTION_GAMEPLAY_SIMPLEHP_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				//DXPARAMSET::GetInstance().m_bSHOW_SIMPLEHP = !DXPARAMSET::GetInstance().m_bSHOW_SIMPLEHP;
				//LoadSimpleHP ();
				m_bSimpleHP = !m_bSimpleHP;
				m_pSimpleHPButton->SetFlip ( m_bSimpleHP );
			}
		}
		break;

	case HWOPTION_GAMEPLAY_FRIEND_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				//RANPARAM::bDIS_FRIEND = !RANPARAM::bDIS_FRIEND;
				//LoadFriend ();
				m_bFriend = !m_bFriend;
				m_pFriendButton->SetFlip ( m_bFriend );
			}
		}
		break;

	case HWOPTION_GAMEPLAY_DEFAULTPOS_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				DoModal ( ID2GAMEINTEXT("DEFAULT_POSITION"), MODAL_QUESTION, YESNO, MODAL_DEFAULT_POS );					
			}
		}
		break;

	case HWOPTION_GAMEPLAY_MOVABLE_ON_CHAT_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				//RANPARAM::bMOVABLE_ON_CHAT = !RANPARAM::bMOVABLE_ON_CHAT;
				//LoadMovableOnChat ();
				m_bMovableOnChat = !m_bMovableOnChat;
				m_pMovableOnChatButton->SetFlip ( m_bMovableOnChat );
			}
		}
		break;

	case HWOPTION_GAMEPLAY_SHOW_TIP_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				//RANPARAM::bSHOW_TIP = !RANPARAM::bSHOW_TIP;					
				//LoadShowTip ();
				m_bShowTip = !m_bShowTip;
				m_pShowTipButton->SetFlip ( m_bShowTip );
			}
		}
		break;

	case HWOPTION_GAMEPLAY_FORCED_ATTACK_BUTTON:			// 강제 공격
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				//RANPARAM::bFORCED_ATTACK = !RANPARAM::bFORCED_ATTACK;
				//LoadForcedAttack();
				m_bForcedAttack = !m_bForcedAttack;
				m_pForcedAttackButton->SetFlip ( m_bForcedAttack );
			}
		}
		break;

	case HWOPTION_GAMEPLAY_NAME_DISPLAY_BUTTON:			// 이름 항상 표시
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				//RANPARAM::bNAME_DISPLAY = !RANPARAM::bNAME_DISPLAY;
				//LoadNameDisplay();
				m_bNameDisplay = !m_bNameDisplay;
				m_pNameDisplayButton->SetFlip ( m_bNameDisplay );
			}
		}
		break;

		/*character simple, Juver, 2017/10/01 */
	case HWOPTION_GAMEPLAY_CHARACTER_SIMPLE_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_bCharacterSimple = !m_bCharacterSimple;
				m_pCharacterSimpleButton->SetFlip ( m_bCharacterSimple );
			}
		}break;


	case HWOPTION_GAMEPLAY_CHARACTER_MISC_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_bCharacterMisc = !m_bCharacterMisc;
				m_pCharacterMiscButton->SetFlip ( m_bCharacterMisc );
			}
		}break;

		/*skill effect setting, Juver, 2017/10/01 */
	case HWOPTION_GAMEPLAY_HIDE_SKILL_EFFECT_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_bHideSkillEffect = !m_bHideSkillEffect;
				m_pHideSkillEffectButton->SetFlip ( m_bHideSkillEffect );
			}
		}break;


	case HWOPTION_GAMEPLAY_HIDE_DAMAGE_DISPLAY_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_bHideDamageDisplay = !m_bHideDamageDisplay;
				m_pHideDamageDisplayButton->SetFlip ( m_bHideDamageDisplay );
			}
		}break;


	case HWOPTION_GAMEPLAY_SPECIAL_NAME_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_bSpecialName = !m_bSpecialName;
				m_pSpecialNameButton->SetFlip ( m_bSpecialName );
			}
		}break;


	case HWOPTION_GAMEPLAY_CLASSIC_NAME_DISPLAY_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_bClassicNameDisplay = !m_bClassicNameDisplay;
				m_pClassicNameDisplayButton->SetFlip ( m_bClassicNameDisplay );
			}
		}break;


	case HWOPTION_GAMEPLAY_KILL_FEED_DISPLAY_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_bKillFeedDisplay = !m_bKillFeedDisplay;
				m_pKillFeedDisplayButton->SetFlip ( m_bKillFeedDisplay );
			}
		}break;


	case HWOPTION_GAMEPLAY_HIDE_DEAD_BODY_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_bHideDeadBody = !m_bHideDeadBody;
				m_pHideDeadBodyButton->SetFlip ( m_bHideDeadBody );
			}
		}break;


	case HWOPTION_GAMEPLAY_HIDE_DEAD_NAME_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_bHideDeadName = !m_bHideDeadName;
				m_pHideDeadNameButton->SetFlip ( m_bHideDeadName );
			}
		}break;

		/*charinfoview , Juver, 2017/11/12 */
	case HWOPTION_GAMEPLAY_PRIVATE_STATS_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_bPrivateStats = !m_bPrivateStats;
				m_pPrivateStats->SetFlip ( m_bPrivateStats );
			}
		}break;

	}
}