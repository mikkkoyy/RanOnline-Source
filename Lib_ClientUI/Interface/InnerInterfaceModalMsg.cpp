#include "StdAfx.h"
#include "ClubWindow.h"
#include "ConftModalWindow.h"
#include "DialogueWindow.h"
#include "DialogueWindowRenewal.h"
#include "DxGlobalStage.h"
#include "FriendWindow.h"
#include "GameTextControl.h"
#include "GLFriendClient.h"
#include "GLGaeaClient.h"
#include "InnerInterface.h"
//#include "InventoryWindow.h"
#include "InventoryUI.h"
#include "MarketWindow.h"
#include "ModalCallerID.h"
#include "ModalWindow.h"
#include "PartyModalWindow.h"
#include "PartyWindow.h"
#include "PrivateMarketSellWindow.h"
#include "PrivateMarketWindow.h"
#include "PtoPWindow.h"
#include "QuestWindow.h"
#include "RANPARAM.h"
#include "StorageChargeCard.h"
#include "StorageWindow.h"
#include "TradeWindow.h"
#include "UITextControl.h"
#include "ItemRebuild.h"
#include "WaitServerDialogue.h"
#include "RebirthDialogue.h"
#include "RebirthWindow.h"
#include "ItemMixWindow.h"
#include "ModernQuestWindow.h"
#include "AdminMessageDisplay.h"
#include "NPCItemExchangeWindow.h"
#include "NPCItemExchangeSlot.h"

/*dmk14 ingame web*/
#include "ItemMallWindow.h"
#include "ItemMallWindowMenuPage.h"

/*dmk14 itemshop new ui*/
#include "ItemShopWindow.h"

//PANDORA
#include "./PandoraBoxWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CInnerInterface::MsgProcess ( LPVOID msgBuffer )
{
	if( !msgBuffer) return ;
	CString strTEXT;

	NET_MSG_GENERIC *nmg = (NET_MSG_GENERIC*) msgBuffer;

	switch ( nmg->nType )
	{
	//PANDORA
	case NET_MSG_PANDORA_BOX_RESULT:
		{
			GLMSG::SNET_PANDORA_BOX_RESULT* pNetMsg = ( GLMSG::SNET_PANDORA_BOX_RESULT* ) nmg;
			m_pPandoraBoxWindow->InitWindow();
		}
		break;
	case NET_MSG_PANDORA_BOX_BUY_ITEM_FB:
		{
			GLMSG::SNET_PANDORA_BOX_BUY_ITEM_FB* pNetMsg = ( GLMSG::SNET_PANDORA_BOX_BUY_ITEM_FB* ) nmg;
			m_pPandoraBoxWindow->InitWindow();
		}
		break;

	case NET_MSG_EVENT_LOTTERY_FB:  
		{
			//	�ζ�
			NET_EVENT_LOTTERY_FB* pNetMsg = (NET_EVENT_LOTTERY_FB*)nmg;
			const int nResult = pNetMsg->nResult;

			enum
			{
				LOTTERY_FB_NO1 = 1,
				LOTTERY_FB_NO2,
				LOTTERY_FB_NO3,
				LOTTERY_FB_NO4,
				LOTTERY_FB_NO5,
				LOTTERY_FB_NO6,
				LOTTERY_FB_ALREADY_INPUTED,
				LOTTERY_FB_NOT_PCROOM,
				LOTTERY_FB_ALREADY_USED,
				LOTTERY_FB_ERROR = 10,
			};

			switch ( nResult )
			{
			case LOTTERY_FB_NO1:
			case LOTTERY_FB_NO2:
			case LOTTERY_FB_NO3:
			case LOTTERY_FB_NO4:
			case LOTTERY_FB_NO5:
			case LOTTERY_FB_NO6:
				{
					CString strTemp = MakeString ( ID2GAMEINTEXT("LOTTERY_FB_NO1_6"), nResult );
					DoModal ( strTemp );
				}
				break;

			case LOTTERY_FB_ALREADY_INPUTED:
				{
					DoModal ( ID2GAMEINTEXT("LOTTERY_FB_ALREADY_INPUTED") );
				}
				break;
			case LOTTERY_FB_NOT_PCROOM:
				{
					DoModal ( ID2GAMEINTEXT("LOTTERY_FB_NOT_PCROOM") );
				}
				break;
			case LOTTERY_FB_ALREADY_USED:
				{
					DoModal ( ID2GAMEINTEXT("LOTTERY_FB_ALREADY_USED") );
				}
				break;
			case LOTTERY_FB_ERROR:
				{
					DoModal ( ID2GAMEINTEXT("LOTTERY_FB_ERROR") );
				}
				break;
			}
		}
		break;
	
	case NET_MSG_APEX_ANSWER: 
		{
			NET_APEX_ANSWER* pNetMsg = (NET_APEX_ANSWER*) nmg;
			switch( pNetMsg->wAction )
			{
			case 1: // ���� ����ϴ°��� �߰������� �������� ����� �ִ� �޼���
				{
					CString strTemp( ID2GAMEINTEXT( "APEX_ACTION_MESSAGE_WARNING" ) );
					PrintConsoleText( strTemp.GetString() );
					DoModal( strTemp, MODAL_INFOMATION, OK, NO_ID );
				}
				break;
			case 2: // ��ȭ ����
				{
					CString strTemp;
					strTemp.Format( ID2GAMEINTEXT( "APEX_ACTION_MESSAGE_CHATBLOCK" ), (pNetMsg->wHour*60) );
					PrintConsoleText( strTemp.GetString() );
					DoModal( strTemp, MODAL_INFOMATION, OK, NO_ID );
				}
				break;
			case 3: // ���� ű
			case 4: // ���� ����
				{
					CString strTemp( ID2GAMEINTEXT( "APEX_ACTION_MESSAGE_KICK" ) );
					PrintConsoleText( strTemp.GetString() );
					WAITSERVER_DIALOGUE_OPEN( strTemp, WAITSERVER_CLOSEGAME, 30.0f );
				}
				break;
			default:
				GASSERT( 0 && "ERROR : DxGameStage::MsgProcess, NET_MSG_APEX_ANSER" );
				break;
			}
		}
		break;
	
	case NET_MSG_CHAT_CTRL_FB:			// �����ڿ� ä�ø޽���
		{
			NET_CHAT_CTRL_FB* pNc = (NET_CHAT_CTRL_FB*) nmg;

			/*item link, Juver, 2017/07/31 */
			/*staff name in chat, Juver, 2017/12/26 */
			if ( RANPARAM::bFeatureChatShowStaffName )
				DisplayChatMessage ( pNc->emType, pNc->szName, pNc->szChatMsg, pNc->sItemLink );
			else
				DisplayChatMessage ( pNc->emType, NULL, pNc->szChatMsg, pNc->sItemLink );
		}break;

	case NET_MSG_CHAT_FB: // ä�ø޽���  
		{
			NET_CHAT_FB* pNc = (NET_CHAT_FB*) nmg;

			/*item link, Juver, 2017/07/31 */
			DisplayChatMessage ( pNc->emType, pNc->szName, pNc->szChatMsg, pNc->sItemLink );
		}break;

	case NET_MSG_CHAT_PRIVATE_FAIL: 
		{
			GLMSG::SNETPC_CHAT_PRIVATE_FAIL *pNetMsg = (GLMSG::SNETPC_CHAT_PRIVATE_FAIL *) nmg;
			PrintConsoleText ( ID2GAMEINTEXT("CHAT_PRIVATE_FAIL"), pNetMsg->szName );
		}
		break;
	
	case NET_MSG_CHAT_BLOCK: 
		{
#ifndef CH_PARAM
			GLMSG::SNETPC_CHAT_BLOCK *pNetMsg = (GLMSG::SNETPC_CHAT_BLOCK *) nmg;
			CTime cTime( pNetMsg->tChatBlock ); // �������� 0�� �Ѿ���� �ʴ´ٰ� �����Ѵ�.
			PrintConsoleText( "[%s]:%02d/%02d/%02d %02d:%02d%s", 
															ID2GAMEWORD("NOTIFY_NAME"),
															cTime.GetYear()%2000, cTime.GetMonth(), cTime.GetDay(),
															cTime.GetHour(), cTime.GetMinute(),
															ID2GAMEINTEXT("CHAT_BLOCK_MSG") );
#endif
		}
		break;

	case NET_MSG_GCTRL_DROP_OUT_FORCED:
		{
			GLMSG::SNET_DROP_OUT_FORCED *pNetMsg = (GLMSG::SNET_DROP_OUT_FORCED *) msgBuffer;

			//CDebugSet::ToLogFile ( "error : overlapping login " );

			if ( pNetMsg->emForced==EMDROPOUT_REQLOGIN )
			{
				DoModal ( ID2GAMEINTEXT ("PLAYINTERFACE_8"), MODAL_INFOMATION, OK, MODAL_CLOSEGAME );
			}
			else
			{
				DoModal ( ID2GAMEINTEXT ("PLAYINTERFACE_9"), MODAL_INFOMATION, OK, MODAL_CLOSEGAME );
			}

			DxGlobalStage::GetInstance().GetNetClient()->CloseConnect();
		}
		break;

	case NET_MSG_GCTRL_REQ_TAKE_FB:  
		{
			GLMSG::SNETPC_REQ_TAKE_FB *pNetMsg = (GLMSG::SNETPC_REQ_TAKE_FB *)nmg;

			switch ( pNetMsg->emTakeFB )
			{
			case EMTAKE_FB_OFF:
				//pNetMsg->emCrow �����. (GUI)
				switch ( pNetMsg->emCrow)
				{
				case CROW_ITEM:
					{
						//PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("PLAYINTERFACE_10") );
					}
					break;
				case CROW_MONEY:
					{
						//PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("PLAYINTERFACE_11") );
					}
					break;
				}
				break;
			case EMTAKE_FB_RESERVED:
				{
					PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("PLAYINTERFACE_12") );
				}
				break;

			case EMTAKE_FB_INVEN_ERROR:
				{
					PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("PLAYINTERFACE_13") );
				}
				break;
			case EMTAKE_FB_TOO_EVENT_ITEM:
				{
					PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("PLAYINTERFACE_14") );
				}
				break;
			case EMTAKE_FB_DISTANCE:
				{
//					PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMTAKE_FB_DISTANCE") );
				}
				break;
			};
		}
		break;

	case NET_MSG_GCTRL_PARTY_LURE_TAR:  
		{
			GLMSG::SNET_PARTY_LURE_TAR *pNetMsg = (GLMSG::SNET_PARTY_LURE_TAR *) nmg;
			
			PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();
			if( !pLand ) break;

			PGLCHARCLIENT pChar = pLand->GetChar ( pNetMsg->dwMasterID );
			if( !pChar ) break;

			if ( IsVisibleGroup ( PARTY_MODAL_WINDOW ) )
			{
				//	Note : ���� ��Ƽ��û�� ���� �亯�� ���� ���� ���
				//		   ���� ������ ������ ��� �����Ѵ�.
				GLMSG::SNET_PARTY_LURE_TAR_ANS NetMsg;
				NetMsg.dwMasterID = pNetMsg->dwMasterID;
				NetMsg.dwPartyID = pNetMsg->dwPartyID;
				NetMsg.emAnswer = EMPARTY_LURE_REFUSE;
				NETSEND ( &NetMsg );
			}

			//	Note : ��Ƽ �ڵ� �ź�.
			//
			if ( RANPARAM::bDIS_PARTY )
			{
				//	��Ƽ ���� ó��
				GLMSG::SNET_PARTY_LURE_TAR_ANS NetMsg;
				NetMsg.dwMasterID = pNetMsg->dwMasterID;
				NetMsg.dwPartyID = pNetMsg->dwPartyID;
				NetMsg.emAnswer = EMPARTY_LURE_REFUSE;
				NETSEND ( &NetMsg );

				PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("PARTY_LURE_REFUSE_AUTO"), pChar->GetName() );
				break;
			}

			//	Note : ��û�� ��Ƽ �ɼ�.
			//
			m_sReqPartyOption = pNetMsg->sOption;

			GetPartyWindow()->SetPartyInfo ( pNetMsg->dwPartyID, pNetMsg->dwMasterID );

			CString strTemp;
			strTemp = MakeString ( ID2GAMEINTEXT("PARTY_TAR"), pChar->GetName() );
			DoPartyModal ( strTemp, MODAL_PARTY_TAR );	
		}
		break;

	case NET_MSG_GCTRL_TRADE_TAR: 
		{
			GLMSG::SNET_TRADE_TAR *pNetMsg = (GLMSG::SNET_TRADE_TAR *) nmg;
			PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();
			if( !pLand ) break;

			PGLCHARCLIENT pChar = pLand->GetChar( pNetMsg->dwMasterID );
			if( !pChar ) break;
			
			CString strName( pChar->GetName() );

			//	Note : �ŷ� �ڵ� �ź�.
			//
			bool bPMARKETOPEN = GLGaeaClient::GetInstance().GetCharacter()->m_sPMarket.IsOpen();
			if ( RANPARAM::bDIS_TRADE || bPMARKETOPEN )
			{
				//	P2P �ŷ� ���� ó��
				GLMSG::SNET_TRADE_TAR_ANS NetMsg;
				NetMsg.dwMasterID = pNetMsg->dwMasterID;
				NetMsg.emAns = EMTRADE_CANCEL;
				NETSEND ( &NetMsg );

				PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("TRADE_FB_CANCEL_AUTO"), strName );
				break;
			}

			CString strTemp;
			strTemp = MakeString ( ID2GAMEINTEXT("TRADE_TAR"), strName );
			DoModal ( strTemp, MODAL_QUESTION, OKCANCEL, MODAL_TRADE_REQ );

			GetTradeWindow()->SetTradeInfo ( strName, pNetMsg->dwMasterID );
		}
		break;

	case NET_MSG_GCTRL_CONFRONT_TAR:  
		{
			GLMSG::SNETPC_REQ_CONFRONT_TAR *pNetMsg = (GLMSG::SNETPC_REQ_CONFRONT_TAR *)nmg;
			
			PGLCHARCLIENT pChar = GLGaeaClient::GetInstance().GetChar ( pNetMsg->dwID );
			if( !pChar ) break;
			
			if ( IsVisibleGroup ( CONFT_MODAL_WINDOW ) )
			{
				//	Note : ���� ��ý�û�� ���� �亯�� ���� ���� ���
				//		   ���� ������ ������ ��� �����Ѵ�.
				GLMSG::SNETPC_REQ_CONFRONT_ANS NetMsgAns;
				NetMsgAns.emTYPE = pNetMsg->emTYPE;
				NetMsgAns.dwID = pNetMsg->dwID;
				NetMsgAns.emFB = EMCONFRONT_REFUSE;
				NetMsgAns.sOption = pNetMsg->sOption;
				NETSEND ( &NetMsgAns );
			}
			
			//	Note : ��� �ڵ� �ź�.
			//
			if ( RANPARAM::bDIS_CONFT )
			{
				//	Note : ��� �����ÿ� �޽��� ����.
				GLMSG::SNETPC_REQ_CONFRONT_ANS NetMsgAns;
				NetMsgAns.emTYPE = pNetMsg->emTYPE;
				NetMsgAns.dwID = pNetMsg->dwID;
				NetMsgAns.emFB = EMCONFRONT_REFUSE;
				NetMsgAns.sOption = pNetMsg->sOption;
				NETSEND ( &NetMsgAns );

				PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMCONFRONT_REFUSE_AUTO"), pChar->GetName() );
				break;
			}

			const char *szReqName = pChar->GetName();
			m_ConflictReqID = pNetMsg->dwID;
			m_emConflictReqType = pNetMsg->emTYPE;
			m_sReqConflictOption = pNetMsg->sOption;
			UIGUID MODAL_CONFT_TYPE = NO_ID;

			CString strTemp;
			switch (pNetMsg->emTYPE)
			{
			case EMCONFT_NONE:
//				GASSERT ( 0 && "����" );
				return ;
				break;
			case EMCONFT_ONE:
				strTemp = MakeString ( ID2GAMEINTEXT("CONFLICT_ONE_QUESTION_TAR"), szReqName );
				MODAL_CONFT_TYPE = MODAL_CONFLICT_ONE_TAR;
				break;
			case EMCONFT_PARTY:
				strTemp = MakeString ( ID2GAMEINTEXT("CONFLICT_PARTY_QUESTION_TAR"), szReqName );
				MODAL_CONFT_TYPE = MODAL_CONFLICT_PARTY_TAR;
				break;
			case EMCONFT_GUILD:
				strTemp = MakeString ( ID2GAMEINTEXT("CONFLICT_CLUB_QUESTION_TAR"), szReqName );
				MODAL_CONFT_TYPE = MODAL_CONFLICT_GUILD_TAR;
				break;
			};

			CString strMyName = GLGaeaClient::GetInstance().GetCharacter ()->GetName ();
			DoConftModal ( strTemp, MODAL_CONFT_TYPE, szReqName, strMyName );
		}
		break;

	case NET_MSG_ALLIANCE_BATTLE_BEGIN:
		{
			GLMSG::SNET_ALLIANCE_BATTLE_BEGIN *pNetMsg1 = (GLMSG::SNET_ALLIANCE_BATTLE_BEGIN*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("ALLIANCE_BATTLE_BEGIN"),
				pNetMsg1->szParty1, pNetMsg1->szParty2 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_ALLIANCE_BATTLE_BEGIN_REFUSE:
		{
			GLMSG::SNET_ALLIANCE_BATTLE_BEGIN_REFUSE *pNetMsg2 = (GLMSG::SNET_ALLIANCE_BATTLE_BEGIN_REFUSE*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("ALLIANCE_BATTLE_BEGIN_REFUSE"),
				pNetMsg2->szParty1, pNetMsg2->szParty2 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_ALLIANCE_BATTLE_OVER_ARMISTICE:
		{
			GLMSG::SNET_ALLIANCE_BATTLE_OVER_ARMISTICE *pNetMsg3 = (GLMSG::SNET_ALLIANCE_BATTLE_OVER_ARMISTICE*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("ALLIANCE_BATTLE_OVER_ARMISTICE"),
				pNetMsg3->szParty1, pNetMsg3->szParty2 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_ALLIANCE_BATTLE_OVER_ARMISTICE_RESULT:
		{
			GLMSG::SNET_ALLIANCE_BATTLE_OVER_ARMISTICE_RESULT *pNetMsg4 = (GLMSG::SNET_ALLIANCE_BATTLE_OVER_ARMISTICE_RESULT*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("ALLIANCE_BATTLE_OVER_ARMISTICE_RESULT"),
				pNetMsg4->dwRes1, pNetMsg4->dwRes1 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_ALLIANCE_BATTLE_OVER_DRAW:
		{
			GLMSG::SNET_ALLIANCE_BATTLE_OVER_DRAW *pNetMsg5 = (GLMSG::SNET_ALLIANCE_BATTLE_OVER_DRAW*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("ALLIANCE_BATTLE_OVER_DRAW"),
				pNetMsg5->szParty1, pNetMsg5->szParty2 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_ALLIANCE_BATTLE_OVER_SUBMISSION:
		{
			GLMSG::SNET_ALLIANCE_BATTLE_OVER_SUBMISSION *pNetMsg6 = (GLMSG::SNET_ALLIANCE_BATTLE_OVER_SUBMISSION*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("ALLIANCE_BATTLE_OVER_SUBMISSION"),
				pNetMsg6->szParty1, pNetMsg6->szParty2 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_ALLIANCE_BATTLE_OVER_WIN:
		{
			GLMSG::SNET_ALLIANCE_BATTLE_OVER_WIN *pNetMsg7 = (GLMSG::SNET_ALLIANCE_BATTLE_OVER_WIN*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("ALLIANCE_BATTLE_OVER_WIN"),
				pNetMsg7->szParty1, pNetMsg7->szParty2 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_ALLIANCE_BATTLE_RESULT:
		{
			GLMSG::SNET_ALLIANCE_BATTLE_RESULT *pNetMsg8 = (GLMSG::SNET_ALLIANCE_BATTLE_RESULT*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("ALLIANCE_BATTLE_RESULT"),
				pNetMsg8->szString1, pNetMsg8->dwWinner, pNetMsg8->dwLose, pNetMsg8->dwDraw );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_CLUB_BATTLE_BEGIN:
		{
			GLMSG::SNET_CLUB_BATTLE_BEGIN *pNetMsg9 = (GLMSG::SNET_CLUB_BATTLE_BEGIN*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("CLUB_BATTLE_BEGIN"),
				pNetMsg9->szGuild1, pNetMsg9->szGuild2 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_CLUB_BATTLE_BEGIN_REFUSE:
		{
			GLMSG::SNET_CLUB_BATTLE_BEGIN_REFUSE *pNetMsg10 = (GLMSG::SNET_CLUB_BATTLE_BEGIN_REFUSE*) nmg;

			strTEXT.Format ( ID2GAMEINTEXT("CLUB_BATTLE_BEGIN_REFUSE"),
				pNetMsg10->szGuild1, pNetMsg10->szGuild2 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_CLUB_BATTLE_OVER_ARMISTICE:
		{
			GLMSG::SNET_CLUB_BATTLE_OVER_ARMISTICE *pNetMsg11 = (GLMSG::SNET_CLUB_BATTLE_OVER_ARMISTICE*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("CLUB_BATTLE_OVER_ARMISTICE"),
				pNetMsg11->szGuild1, pNetMsg11->szGuild2 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_CLUB_BATTLE_OVER_ARMISTICE_RESULT:
		{
			GLMSG::SNET_CLUB_BATTLE_OVER_ARMISTICE_RESULT *pNetMsg12 = (GLMSG::SNET_CLUB_BATTLE_OVER_ARMISTICE_RESULT*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("CLUB_BATTLE_OVER_ARMISTICE_RESULT"),
				pNetMsg12->dwRes1, pNetMsg12->dwRes2 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_CLUB_BATTLE_OVER_SUBMISSION:
		{
			GLMSG::SNET_CLUB_BATTLE_OVER_SUBMISSION *pNetMsg13 = (GLMSG::SNET_CLUB_BATTLE_OVER_SUBMISSION*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("CLUB_BATTLE_OVER_SUBMISSION"),
				pNetMsg13->szGuild1, pNetMsg13->szGuild2 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_CLUB_BATTLE_OVER_WIN:
		{
			GLMSG::SNET_CLUB_BATTLE_OVER_WIN *pNetMsg14 = (GLMSG::SNET_CLUB_BATTLE_OVER_WIN*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("CLUB_BATTLE_OVER_WIN"),
				pNetMsg14->szGuild1, pNetMsg14->szGuild2 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_CLUB_BATTLE_RESULT:
		{
			GLMSG::SNET_CLUB_BATTLE_RESULT *pNetMsg15 = (GLMSG::SNET_CLUB_BATTLE_RESULT*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("CLUB_BATTLE_RESULT"),
				pNetMsg15->szString1, pNetMsg15->dwWin, pNetMsg15->dwLose, pNetMsg15->dwDraw );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_EMCONFRONT_END_CDRAWN:
		{
			GLMSG::SNET_EMCONFRONT_END_CDRAWN *pNetMsg16 = (GLMSG::SNET_EMCONFRONT_END_CDRAWN*) nmg;

			strTEXT.Format ( ID2GAMEINTEXT("EMCONFRONT_END_CDRAWN"),
				pNetMsg16->szMapName, pNetMsg16->szGuild1, pNetMsg16->szGuild2 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_EMCONFRONT_END_CWIN:
		{
			GLMSG::SNET_EMCONFRONT_END_CWIN *pNetMsg17 = (GLMSG::SNET_EMCONFRONT_END_CWIN*) nmg;

			strTEXT.Format ( ID2GAMEINTEXT("EMCONFRONT_END_CWIN"),
				pNetMsg17->szMapName, pNetMsg17->szGuild1, pNetMsg17->szGuild2, pNetMsg17->szWinner );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_EMCONFRONT_END_PDRAWN:
		{
			GLMSG::SNET_EMCONFRONT_END_PDRAWN *pNetMsg18 = (GLMSG::SNET_EMCONFRONT_END_PDRAWN*) nmg;

			strTEXT.Format ( ID2GAMEINTEXT("EMCONFRONT_END_PDRAWN"),
				pNetMsg18->szMapName, pNetMsg18->szGuild1, pNetMsg18->szGuild2 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_EMCONFRONT_END_PWIN:
		{
			GLMSG::SNET_EMCONFRONT_END_PWIN *pNetMsg19 = (GLMSG::SNET_EMCONFRONT_END_PWIN*) nmg;

			strTEXT.Format ( ID2GAMEINTEXT("EMCONFRONT_END_PWIN"),
				pNetMsg19->szMapName, pNetMsg19->szGuild1, pNetMsg19->szGuild2, pNetMsg19->szWinner );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_EMCONFRONT_START_PARTY:
		{
			GLMSG::SNET_EMCONFRONT_START_PARTY *pNetMsg20 = (GLMSG::SNET_EMCONFRONT_START_PARTY*) nmg;

			strTEXT.Format ( ID2GAMEINTEXT("EMCONFRONT_START_PARTY"),
				pNetMsg20->szMapName, pNetMsg20->szGuild1, pNetMsg20->szGuild2 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_EMCROWACT_KNOCK:
		{
			GLMSG::SNET_EMCROWACT_KNOCK *pNetMsg21 = (GLMSG::SNET_EMCROWACT_KNOCK*) nmg;

			const char *szMAP_NAME = GLGaeaClient::GetInstance().GetMapName ( pNetMsg21->nidMapID );
			PCROWDATA m_pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( pNetMsg21->nidBossID );

			if( m_pCrowData && szMAP_NAME ){
				strTEXT.Format ( ID2GAMEINTEXT("EMCROWACT_KNOCK"),
					m_pCrowData->GetName(), szMAP_NAME );
				DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT.GetString() );
			}
		}
		break;
	case NET_MSG_EMCROWACT_REPULSE:
		{
			GLMSG::SNET_EMCROWACT_REPULSE *pNetMsg22 = (GLMSG::SNET_EMCROWACT_REPULSE*) nmg;

			const char *szMAP_NAME = GLGaeaClient::GetInstance().GetMapName ( pNetMsg22->nidMapID );
			PCROWDATA m_pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( pNetMsg22->nidBossID );

			if( m_pCrowData && szMAP_NAME ){
				strTEXT.Format ( ID2GAMEINTEXT("EMCROWACT_REPULSE"),
					m_pCrowData->GetName(), pNetMsg22->szKiller, szMAP_NAME );
				DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
			}
		}
		break;
	case NET_MSG_EMGUIDCLUB_CERTIFIED:
		{
			GLMSG::SNET_EMGUIDCLUB_CERTIFIED *pNetMsg23 = (GLMSG::SNET_EMGUIDCLUB_CERTIFIED*) nmg;

			strTEXT.Format ( ID2GAMEINTEXT("EMGUIDCLUB_CERTIFIED"),
				pNetMsg23->szGuildName, pNetMsg23->szMapName );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;

	case NET_MSG_BRIGHT_EVENT_MSG:
		{
			GLMSG::SNET_BRIGHT_EVENT_MSG *pNetMsg25 = (GLMSG::SNET_BRIGHT_EVENT_MSG*) nmg;

			strTEXT.Format ( ID2GAMEINTEXT("BRIGHT_EVENT_MSG"));
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;

	case NET_MSG_CLUB_BATTLE_OVER_DRAW:
		{
			GLMSG::SNET_CLUB_BATTLE_OVER_DRAW *pNetMsg26 = (GLMSG::SNET_CLUB_BATTLE_OVER_DRAW*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("CLUB_BATTLE_OVER_DRAW"),
				pNetMsg26->szGuild1, pNetMsg26->szGuild2 );
			DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;


	case NET_MSG_SERVER_GENERALCHAT: 
		{
			GLMSG::SNET_SERVER_GENERALCHAT* pNetMsg = (GLMSG::SNET_SERVER_GENERALCHAT*) nmg;
			if (pNetMsg->bNotify) 
			{
				switch(pNetMsg->iColor)
				{
				case 1:
					PrintMsgTextDlg ( NS_UITEXTCOLOR::GREENYELLOW, pNetMsg->szTEXT );
					break;
				case 2:
					PrintMsgTextDlg ( NS_UITEXTCOLOR::WHITE, pNetMsg->szTEXT );
					break;
				case 3:
					PrintMsgTextDlg ( NS_UITEXTCOLOR::LIGHTYELLOW, pNetMsg->szTEXT );
					break;
				case 4:
					PrintMsgTextDlg ( NS_UITEXTCOLOR::AQUABLUE, pNetMsg->szTEXT );
					break;
				case 5:
					PrintMsgText ( NS_UITEXTCOLOR::YELLOW, pNetMsg->szTEXT );
					break;
				case 6:
					PrintMsgText ( NS_UITEXTCOLOR::GREENYELLOW, pNetMsg->szTEXT );
					break;
				case 7:
					PrintMsgText ( NS_UITEXTCOLOR::DISABLE, pNetMsg->szTEXT );
					break;
				case 8:
					PrintMsgTextDlg(D3DCOLOR_RGBA(101,210,210,255),pNetMsg->szTEXT);
					break;
				default:
					DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", pNetMsg->szTEXT );
					break;
				}
			}
			else{
				DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", pNetMsg->szTEXT );
			}

			/*if ( pNetMsg->bUseAdminDisplay )
			{
				if ( m_pAdminMessageDisplay )
				{
					CString strAdminText;
					strAdminText.Format( "%s", pNetMsg->szTEXT );
					m_pAdminMessageDisplay->AddText ( strAdminText, NS_UITEXTCOLOR::ADMIN_COLOR );

					ShowGroupTop ( ADMIN_MESSAGE_DISPLAY );	
				}
			}*/
		}
		break;

//		NOTE
//			CLOSING MESSAGE ó��
//	case BLAH_BLAH:
//		{
//			SetWaitServerDialogueClose ();
//		}
//		break;
	};
}

void CInnerInterface::ModalMsgProcess ( UIGUID nCallerID, DWORD dwMsg )
{
	switch ( nCallerID )
	{
	case MODAL_CLOSEGAME:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				WAITSERVER_DIALOGUE_OPEN ( ID2GAMEINTEXT("WAITSERVER_MESSAGE"), WAITSERVER_CLOSEGAME );
			}
		}
		break;
	
	case MODAL_TEST_MODAL:
		break;	

	case MODAL_INVENTORY_MONEY_TO_FIELD:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
				if ( pCharacter )
				{
					CString strTemp = GetModalWindow()->GetEditString ();
					LONGLONG lnDropMoney = _ttoi64 ( strTemp.GetString () );
					LONGLONG lnMoney = pCharacter->m_lnMoney;
					if ( lnMoney < lnDropMoney ) lnDropMoney = lnMoney;

					pCharacter->ReqMoneyToField ( lnDropMoney );
				}
			}
		}
		break;

	case MODAL_PARTY_TAR:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				//	��Ƽ ���� ó��
				GLMSG::SNET_PARTY_LURE_TAR_ANS NetMsg;
				NetMsg.dwMasterID = GetPartyWindow()->GetMasterID ();
				NetMsg.dwPartyID = GetPartyWindow()->GetPartyID ();
				NetMsg.emAnswer = EMPARTY_LURE_OK;
				NetMsg.sOption = m_sReqPartyOption;
				NETSEND ( &NetMsg );
			}
			else if ( dwMsg & UIMSG_MODAL_CANCEL )
			{
				//	��Ƽ ���� ó��
				GLMSG::SNET_PARTY_LURE_TAR_ANS NetMsg;
				NetMsg.dwMasterID = GetPartyWindow()->GetMasterID ();
				NetMsg.dwPartyID = GetPartyWindow()->GetPartyID ();
				NetMsg.emAnswer = EMPARTY_LURE_REFUSE;
				NETSEND ( &NetMsg );
			}
		}
		break;

	case MODAL_PARTY_REQ:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				DWORD dwGaeaID = m_pPtoPWindow->GetPlayerID ();

				SPARTY_OPT sOption;
				sOption.emGET_ITEM = (EMPARTY_ITEM_OPT)RANPARAM::dwPARTY_GET_ITEM;
				sOption.emGET_MONEY = (EMPARTY_MONEY_OPT)RANPARAM::dwPARTY_GET_MONEY;

				//	Note : ������ ��������� ��Ƽ ��û.
				//
				GLMSG::SNET_PARTY_LURE NetMsg;
				NetMsg.dwGaeaID = dwGaeaID;
				NetMsg.sOption = sOption;
				NETSEND ( (NET_MSG_GENERIC*) &NetMsg );
			}
		}
		break;
	
	case MODAL_TRADE_REQ:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				//	P2P �ŷ� ���� ó��
				GLMSG::SNET_TRADE_TAR_ANS NetMsg;
				NetMsg.dwMasterID = GetTradeWindow()->GetMasterID ();
				NetMsg.emAns = EMTRADE_OK;
				NETSEND ( &NetMsg );
			}
			else if ( dwMsg & UIMSG_MODAL_CANCEL )
			{
				//	P2P �ŷ� ���� ó��
				GLMSG::SNET_TRADE_TAR_ANS NetMsg;
				NetMsg.dwMasterID = GetTradeWindow()->GetMasterID ();
				NetMsg.emAns = EMTRADE_CANCEL;
				NETSEND ( &NetMsg );
			}
		}
		break;
	
	case MODAL_REBIRTH:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				if ( GLGaeaClient::GetInstance().GetCharacter()->IsDie () )
				{
					// ��Ȱ�ϱ�
					GLGaeaClient::GetInstance().GetCharacter()->ReqReBirth();
					
					// ������ â�� �ݱ�
					CloseAllWindow ();				
				}
			}

			if ( dwMsg & UIMSG_MODAL_CANCEL )
			{
				if ( GLGaeaClient::GetInstance().GetCharacter()->IsDie () )
				{
					if ( m_pRebirthDialogue->IsRecoveryAction() )
					{	
						GLGaeaClient::GetInstance().GetCharacter()->ReqGetReExp ();
					}
					else
					{
						// ��ȥ�� ���
						GLGaeaClient::GetInstance().GetCharacter()->ReqReGenRevive ();
						// ������ â�� �ݱ�
						CloseAllWindow ();						
					}
				}
			}
		}
		break;
	case MODAL_RECOVERY_EXP:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				if ( GLGaeaClient::GetInstance().GetCharacter()->IsDie () )
				{
					GLGaeaClient::GetInstance().GetCharacter()->ReqRecoveryExp();				
				}
			}
			if ( dwMsg & UIMSG_MODAL_CANCEL )
			{
				if ( GLGaeaClient::GetInstance().GetCharacter()->IsDie () )
				{
					// ��Ȱ�ϱ�
					GLGaeaClient::GetInstance().GetCharacter()->ReqReBirth();
					
					// ������ â�� �ݱ�
					CloseAllWindow ();				
				}
			}
		}
		break;

	case MODAL_RECOVERY_NPC_EXP:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				int NpcID = 0;
				int DummyData = 0;
				GetModalWindow()->GetModalData( &NpcID, &DummyData );

				GLGaeaClient::GetInstance().GetCharacter()->ReqRecoveryExpNpc( NpcID );				
			}			
		}
		break;


	//case MODAL_DROPMONEY:
	//	{
	//		if ( dwMsg & UIMSG_MODAL_OK )
	//		{
	//			GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	//			if ( pCharacter )
	//			{
	//				CString strTemp = GetModalWindow()->GetEditString ();
	//				LONGLONG lnDropMoney = _ttoi64 ( strTemp.GetString () );
	//				LONGLONG lnMoney = pCharacter->m_lnMoney;
	//				if ( lnMoney < lnDropMoney ) lnDropMoney = lnMoney;

	//				pCharacter->ReqMoneyToField ( lnDropMoney );
	//			}
	//		}
	//	}
	//	break;

	case MODAL_TRADEMONEY:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				CString strTemp = GetModalWindow()->GetEditString ();
				LONGLONG lnMoney = _ttoi64 ( strTemp.GetString () );
				GLTradeClient::GetInstance().ReqTradeMoney ( lnMoney );
			}
		}
		break;

	case MODAL_SAVEMONEY:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
				if ( pCharacter )
				{
					CString strTemp = GetModalWindow()->GetEditString ();
					LONGLONG lnSaveMoney = _ttoi64 ( strTemp.GetString () );
					LONGLONG lnMoney = pCharacter->m_lnMoney;
					if ( lnMoney < lnSaveMoney ) lnSaveMoney = lnMoney;

					pCharacter->ReqStorageSaveMoney ( lnSaveMoney );
				}
			}
		}
		break;

	case MODAL_CLUB_SAVEMONEY:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
				if ( pCharacter )
				{
					CString strTemp = GetModalWindow()->GetEditString ();
					LONGLONG lnSaveMoney = _ttoi64 ( strTemp.GetString () );
					LONGLONG lnMoney = pCharacter->m_lnMoney;
					if ( lnMoney < lnSaveMoney ) lnSaveMoney = lnMoney;

					pCharacter->ReqClubStorageSaveMoney ( lnSaveMoney );
				}
			}
		}
		break;

	case MODAL_LOADMONEY:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
				if ( pCharacter )
				{
					CString strTemp = GetModalWindow()->GetEditString ();
					LONGLONG lnLoadMoney = _ttoi64 ( strTemp.GetString () );
					LONGLONG lnMoney = pCharacter->m_lnStorageMoney;
					if ( lnMoney < lnLoadMoney ) lnLoadMoney = lnMoney;

					pCharacter->ReqStorageDrawMoney ( lnLoadMoney );
				}
			}
		}
		break;

	case MODAL_CLUB_LOADMONEY:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
				if ( pCharacter )
				{
					CString strTemp = GetModalWindow()->GetEditString ();
					LONGLONG lnLoadMoney = _ttoi64 ( strTemp.GetString () );
					LONGLONG lnMoney = pCharacter->m_sCLUB.m_lnStorageMoney;
					if ( lnMoney < lnLoadMoney ) lnLoadMoney = lnMoney;

					pCharacter->ReqClubStorageDrawMoney ( lnLoadMoney );
				}
			}
		}
		break;

	case MODAL_SPLIT_ITEM:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
				if ( pCharacter )
				{
					CString strTemp = GetModalWindow()->GetEditString ();
					WORD wSplitNum = (WORD) atoi ( strTemp.GetString () );

					WORD wPosX, wPosY;
					GetInventoryUI ()->GetSplitPos ( &wPosX, &wPosY );
					pCharacter->ReqInvenSplit ( wPosX, wPosY, wSplitNum );
				}
			}
		}
		break;

	case MODAL_STORAGE_SPLIT_ITEM:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
				if ( pCharacter )
				{
					CString strTemp = GetModalWindow()->GetEditString ();
					WORD wSplitNum = (WORD) atoi ( strTemp.GetString () );

					WORD wPosX, wPosY;
					int nChannel = m_pStorageWindow->GetChannel ();
					m_pStorageWindow->GetSplitPos ( &wPosX, &wPosY );					
					pCharacter->ReqStorageSplit ( nChannel, wPosX, wPosY, wSplitNum );
				}
			}
		}
		break;

	case MODAL_CLUB_STORAGE_SPLIT_ITEM:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
				if ( pCharacter )
				{
					CString strTemp = GetModalWindow()->GetEditString ();
					WORD wSplitNum = (WORD) atoi ( strTemp.GetString () );

					WORD wPosX, wPosY;
					int nChannel = m_pClubStorageWindow->GetChannel ();
					m_pClubStorageWindow->GetSplitPos ( &wPosX, &wPosY );
					pCharacter->ReqClubStorageSplit ( nChannel, wPosX, wPosY, wSplitNum );
				}
			}
		}
		break;

	case MODAL_CONFLICT_ONE_TAR:
	case MODAL_CONFLICT_PARTY_TAR:
	case MODAL_CONFLICT_GUILD_TAR:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				//	Note : ��� �����ÿ� �޽��� ����.
				GLMSG::SNETPC_REQ_CONFRONT_ANS NetMsgAns;
				NetMsgAns.emTYPE = m_emConflictReqType;
				NetMsgAns.dwID = m_ConflictReqID;
				NetMsgAns.emFB = EMCONFRONT_AGREE;
				NetMsgAns.sOption = m_sReqConflictOption;
				NETSEND ( &NetMsgAns );
			}
			else if ( dwMsg & UIMSG_MODAL_CANCEL )
			{
				//	Note : ��� �����ÿ� �޽��� ����.
				GLMSG::SNETPC_REQ_CONFRONT_ANS NetMsgAns;
				NetMsgAns.emTYPE = m_emConflictReqType;
				NetMsgAns.dwID = m_ConflictReqID;
				NetMsgAns.emFB = EMCONFRONT_REFUSE;
				NetMsgAns.sOption = m_sReqConflictOption;
				NETSEND ( &NetMsgAns );
			}
		}
		break;

	case MODAL_CONFLICT_ONE_REQ:
	case MODAL_CONFLICT_PARTY_REQ:
	case MODAL_CONFLICT_GUILD_REQ:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				DWORD dwGaeaID = m_pPtoPWindow->GetPlayerID ();

				SCONFT_OPTION sOption;
				sOption.bBOUND = RANPARAM::bCONFT_BOUND!=FALSE;
				sOption.wRECOVER = RANPARAM::wCONFT_RECOVER;
				sOption.fHP_RATE = RANPARAM::fCONFT_HP_RATE;
				sOption.fTAR_HP_RATE = RANPARAM::fCONFT_TAR_HP_RATE;

				GLGaeaClient::GetInstance().GetCharacter()->ReqConflict ( dwGaeaID, sOption );
			}
		}
		break;

		
	case MODAL_DIALOGUE_PILE_ITEM:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				CString strTemp = GetModalWindow()->GetEditString ();
				DWORD dwCount = (DWORD) atoi ( strTemp.GetString () );
				if ( dwCount )
				{
					SNATIVEID sMarketID = m_pDialogueWindowRenewal->GetNPCID();
					int	nPageIndex = m_pDialogueWindowRenewal->GetPageIndex();

					int nPosX = nOUTOFRANGE;
					int nPosY = nOUTOFRANGE;
					m_pDialogueWindowRenewal->GetItemIndex ( &nPosX, &nPosY );
					if ( nPosX < 0 || nPosY < 0 ) return ;

					LONGLONG lnMoney = GLGaeaClient::GetInstance().GetCharacter ()->m_lnMoney;
					SINVENITEM sInvenItem = m_pDialogueWindowRenewal->GetItem ( nPosX, nPosY );
					SITEM* pItem = GLItemMan::GetInstance().GetItem ( sInvenItem.sItemCustom.sNativeID );
					if ( !pItem ) return ;

					/*float fSHOP_RATE = GLGaeaClient::GetInstance().GetCharacter()->GetBuyRate();
					float fSHOP_RATE_C = fSHOP_RATE * 0.01f;
					DWORD dwPrice = static_cast<DWORD>( pItem->sBasicOp.dwBuyPrice * fSHOP_RATE_C );
					DWORD dwMaxCount = (DWORD)(lnMoney / (LONGLONG)dwPrice);
					if ( dwMaxCount < 1 )
					{					
						PrintMsgTextDlg ( NS_UITEXTCOLOR::ORANGE, ID2GAMEINTEXT("BUYITEM_NOMONEY") );
						return ;
					}*/
					//	��ư ��������´�� �ֱ�
					//	�ִ� ������ ���
					/*if ( dwMaxCount < dwCount )
					{												
						PrintMsgText ( NS_UITEXTCOLOR::ORANGE,
							MakeString ( ID2GAMEINTEXT("BUYITEM_MAXIMUM"),
							pItem->GetName(), dwMaxCount )
							);
						dwCount = dwMaxCount;
					}*/
					if ( dwCount > MAXWORD )
					{
						dwCount = MAXWORD;
					}
					WORD wBuyNum = static_cast<WORD>( dwCount );
					//GLGaeaClient::GetInstance().GetCharacter()->ReqNpcTo ( GetMarketWindowNPCID(), nPageIndex, nPosX, nPosY, (WORD)dwCount );
					m_pDialogueWindowRenewal->AddCartList( nPageIndex, nPosX, nPosY, wBuyNum );
				}
			}
		}
		break;

	case MODAL_PILE_ITEM:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				CString strTemp = GetModalWindow()->GetEditString ();
				DWORD dwCount = (DWORD) atoi ( strTemp.GetString () );
				if ( dwCount )
				{
					//CMarketWindow* pMarketWindow = GetMarketWindow ();
					SNATIVEID sMarketID = m_pDialogueWindowRenewal->GetNPCID();
					int	nPageIndex = m_pDialogueWindowRenewal->GetPageIndex();

					int nPosX = nOUTOFRANGE;
					int nPosY = nOUTOFRANGE;
					m_pDialogueWindowRenewal->GetItemIndex ( &nPosX, &nPosY );
					if ( nPosX < 0 || nPosY < 0 ) return ;

					LONGLONG lnMoney = GLGaeaClient::GetInstance().GetCharacter ()->m_lnMoney;
					SINVENITEM sInvenItem = m_pDialogueWindowRenewal->GetItem ( nPosX, nPosY );
					SITEM* pItem = GLItemMan::GetInstance().GetItem ( sInvenItem.sItemCustom.sNativeID );
					if ( !pItem ) return ;

					float fSHOP_RATE = GLGaeaClient::GetInstance().GetCharacter()->GetBuyRate();
					float fSHOP_RATE_C = fSHOP_RATE * 0.01f;
					DWORD dwPrice = static_cast<DWORD>( pItem->sBasicOp.dwBuyPrice * fSHOP_RATE_C );
					DWORD dwMaxCount = (DWORD)(lnMoney / (LONGLONG)dwPrice);
					if ( dwMaxCount < 1 )
					{					
						PrintMsgTextDlg ( NS_UITEXTCOLOR::ORANGE, ID2GAMEINTEXT("BUYITEM_NOMONEY") );
						return ;
					}
					//	��ư ��������´�� �ֱ�
					//	�ִ� ������ ���
					if ( dwMaxCount < dwCount )
					{												
						PrintMsgText ( NS_UITEXTCOLOR::ORANGE,
							MakeString ( ID2GAMEINTEXT("BUYITEM_MAXIMUM"),
							pItem->GetName(), dwMaxCount )
							);
						dwCount = dwMaxCount;
					}
					GLGaeaClient::GetInstance().GetCharacter()->ReqNpcTo ( GetMarketWindowNPCID(), nPageIndex, nPosX, nPosY, (WORD)dwCount );
				}
			}
		}
		break;

	case MODAL_SERVER_STAGE:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				WAITSERVER_DIALOGUE_OPEN ( ID2GAMEINTEXT("WAITSERVER_MESSAGE"), WAITSERVER_TOLOBY );
			}
		}
		break;

	case MODAL_BUY_ITEM:
	case MODAL_SELL_ITEM:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				//CMarketWindow* pMarketWindow = GetMarketWindow ();
				//SNATIVEID sMarketID = pMarketWindow->GetMarketID ();
				int	nPageIndex = m_pDialogueWindowRenewal->GetPageIndex ();

				int nPosX = nOUTOFRANGE;
				int nPosY = nOUTOFRANGE;
				if ( nCallerID == MODAL_BUY_ITEM )
				{
					m_pDialogueWindowRenewal->GetItemIndex ( &nPosX, &nPosY );
					if ( nPosX < 0 || nPosY < 0 ) return ;
				}
				CDebugSet::ToView ( 1, 3, "��� ������ : %d %d", nPosX, nPosY );

				GLGaeaClient::GetInstance().GetCharacter()->ReqNpcTo ( GetMarketWindowNPCID(), nPageIndex, nPosX, nPosY );
			}
		}
		break;

	case MODAL_QUEST_GIVEUP:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				//	����
				/*quest ui, Juver, 2017/11/11 */
				if ( RANPARAM::bFeatureModernQuestWindow )
				{
					DWORD dwQuestID = GetModernQuestWindow()->GetQuestID ();
					GLGaeaClient::GetInstance().GetCharacter ()->ReqQuestGiveUp ( dwQuestID );
				}
				else
				{	
					DWORD dwQuestID = GetQuestWindow()->GetQuestID ();
					GLGaeaClient::GetInstance().GetCharacter ()->ReqQuestGiveUp ( dwQuestID );
				}
			}
		}
		break;

	case MODAL_FRIEND_ADD:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				CString strName = m_pFriendWindow->GET_FRIEND_NAME ();
				GLFriendClient::GetInstance().ReqFriendAdd ( strName.GetString () );
			}
		}
		break;

	case MODAL_FRIEND_DEL:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				CString strName = m_pFriendWindow->GET_FRIEND_NAME ();
				GLFriendClient::GetInstance().ReqFriendDel ( strName.GetString () );
			}
		}
		break;

	case MODAL_FRIEND_MOVE:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{				
				SetFriendName ( m_pFriendWindow->GET_FRIEND_NAME () );
				/*dmk14 fast teleport fix*/
				GLGaeaClient::GetInstance().GetCharacter()->Req2Friend ( m_pFriendWindow->GET_FRIEND_NAME () );
				WAITSERVER_DIALOGUE_OPEN ( ID2GAMEINTEXT("WAITSERVER_MESSAGE"), WAITSERVER_TOFRIEND, 10.0f );
			}
		}
		break;

	case MODAL_ADD_FRIEND_LURE_ANS:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				if ( !m_strFriendName.GetLength () )
				{
					GASSERT ( 0 && "�̸��� ��ϵ��� �ʾҽ��ϴ�." );
					return ;
				}

				GLFriendClient::GetInstance ().ReqFriendLureAns ( m_strFriendName.GetString(), true );
			}
			else if ( UIMSG_MODAL_CANCEL & dwMsg )
			{
				if ( !m_strFriendName.GetLength () )
				{
					GASSERT ( 0 && "�̸��� ��ϵ��� �ʾҽ��ϴ�." );
					return ;
				}

				GLFriendClient::GetInstance ().ReqFriendLureAns ( m_strFriendName.GetString(), false );
			}
		}
		break;

	case MODAL_DEFAULT_POS:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				MOVE_DEFAULT_POSITION ();
			}
		}
		break;

	case MODAL_LOTTO:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				CString strLotto = GetModalWindow()->GetEditString ();
				int nLength = strLotto.GetLength ();
				int nGAP = 10 - nLength;
				if ( nGAP )
				{
					strLotto += "0";
				}
				DxGlobalStage::GetInstance().NetSendEventLottery ( strLotto );
			}
		}
		break;

	case MODAL_STORAGE_CHARGE:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				WORD wPosX, wPosY, wSTORAGE;
				m_pStorageChargeCard->GetSelectedChargeCard ( wPosX, wPosY, wSTORAGE );

				GLGaeaClient::GetInstance().GetCharacter()->ReqStorageCard ( wPosX, wPosY, wSTORAGE );
			}
		}
		break;

	case MODAL_PRIVATE_MARKET_SELL:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				WORD wPosX, wPosY;
				m_pPrivateMarketSellWindow->GetItemIndex ( wPosX, wPosY );

				DWORD dwNum = m_pPrivateMarketSellWindow->GetItemCount ();
				LONGLONG llMoney = m_pPrivateMarketSellWindow->GetItemCost ();

				GLGaeaClient::GetInstance().GetCharacter()->ReqPMarketRegItem ( wPosX, wPosY, llMoney, dwNum );
			}
		}
		break;

	case MODAL_NPCEXCHANGE:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{	
				//todo
				GetNPCItemExchangeWindow()->m_pCurrentSlot->DoExchange();
			}
		}
		break;

	case MODAL_PRIVATE_MARKET_BUY:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{	
				DWORD dwMarketID = m_pPrivateMarketWindow->GetPrivateMarketID ();
//				DWORD dwNum = static_cast<DWORD>(atoi(GetModalWindow()->GetEditString ()));

				int nPosX = nOUTOFRANGE;
				int nPosY = nOUTOFRANGE;

				GetModalWindow()->GetModalData ( &nPosX, &nPosY );
				if ( nPosX < 0 || nPosY < 0 ) return ;

				//CDebugSet::ToView ( 1, 3, "��� ������ : %d %d", nPosX, nPosY );

				GLGaeaClient::GetInstance().GetCharacter()->ReqPMarketBuy ( dwMarketID, nPosX, nPosY, 1 );
			}
		}
		break;

	case MODAL_PRIVATE_MARKET_PILE:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				CString strTemp = GetModalWindow()->GetEditString ();
				DWORD dwCount = (DWORD) atoi ( strTemp.GetString () );
				if ( dwCount )
				{
//					CMarketWindow* pMarketWindow = GetMarketWindow ();
					DWORD dwMarketID = m_pPrivateMarketWindow->GetPrivateMarketID ();
//					int	nPageIndex = pMarketWindow->GetPageIndex ();

					int nPosX = nOUTOFRANGE;
					int nPosY = nOUTOFRANGE;
					GetModalWindow()->GetModalData ( &nPosX, &nPosY );
					if ( nPosX < 0 || nPosY < 0 ) return ;

					LONGLONG lnMoney = GLGaeaClient::GetInstance().GetCharacter ()->m_lnMoney;
					SINVENITEM& sInvenItem = m_pPrivateMarketWindow->GetItem ( nPosX, nPosY );
					SITEM* pItem = GLItemMan::GetInstance().GetItem ( sInvenItem.sItemCustom.sNativeID );
					if ( !pItem ) return ;

					DWORD dwPrice = pItem->sBasicOp.dwBuyPrice;
//					float fSHOP_RATE = GLGaeaClient::GetInstance().GetCharacter()->GetBuyRate();					
//					DWORD dwPrice = static_cast<DWORD>(pItem->sBasicOp.dwBuyPrice*fSHOP_RATE/100.0f+0.1f);

					DWORD dwMaxCount = (DWORD)(lnMoney / (LONGLONG)dwPrice);
					if ( dwMaxCount < 1 )
					{					
						PrintMsgTextDlg ( NS_UITEXTCOLOR::ORANGE, ID2GAMEINTEXT("BUYITEM_NOMONEY") );
						return ;
					}
					//	��ư ��������´�� �ֱ�
					//	�ִ� ������ ���
					if ( dwMaxCount < dwCount )
					{												
						PrintMsgText ( NS_UITEXTCOLOR::ORANGE,
							MakeString ( ID2GAMEINTEXT("BUYITEM_MAXIMUM"),
								pItem->GetName(), dwMaxCount )
							);
						dwCount = dwMaxCount;
					}
					GLGaeaClient::GetInstance().GetCharacter()->ReqPMarketBuy ( dwMarketID, nPosX, nPosY, (WORD)dwCount );
				}
			}
		}
		break;

	case MODAL_PRIVATE_MARKET_QUIT:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				SetPrivateMarketClose ();
			}
		}
		break;

	case MODAL_CLUB_KICK:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				const DWORD dwGaeaID = m_pClubWindow->GetClubMemberID ();				
				GLGaeaClient::GetInstance().GetCharacter ()->ReqClubMemberDel ( dwGaeaID );
			}
		}
		break;

	case MODAL_CLUB_AUTHORITY:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				const DWORD dwGaeaID = m_pClubWindow->GetClubMemberID ();				
				GLGaeaClient::GetInstance().GetCharacter ()->ReqClubAuthority ( dwGaeaID );
			}
		}
		break;
	case MODAL_CLUB_AUTHORITY_ASK:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{						
				GLGaeaClient::GetInstance().GetCharacter()->ReqClubAuthorityAns( true );
			}
			else if ( UIMSG_MODAL_CANCEL & dwMsg )
			{
				GLGaeaClient::GetInstance().GetCharacter()->ReqClubAuthorityAns( false );
			}
		}
		break;
	case MODAL_CLUB_BREAKUP:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				GLGaeaClient::GetInstance().GetCharacter ()->ReqClubDissolution ();
			}
		}
		break;

	case MODAL_CLUB_CHANGE_MARK:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				CString strFileName = m_pModalWindow->GetEditString ();
				GLGaeaClient::GetInstance().GetCharacter ()->ReqClubMarkChange ( strFileName.GetString() );
			}
		}
		break;

	case MODAL_CLUB_NICK:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				CString strNickName = m_pModalWindow->GetEditString ();				
				GLGaeaClient::GetInstance().GetCharacter ()->ReqClubNick ( strNickName.GetString() );
			}
		}
		break;

	case MODAL_CLUB_JOIN_ASK:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{						
				GLGaeaClient::GetInstance().GetCharacter ()->ReqClubJoinAns ( m_dwClubMasterID, true );
			}
			else if ( UIMSG_MODAL_CANCEL & dwMsg )
			{
				GLGaeaClient::GetInstance().GetCharacter ()->ReqClubJoinAns ( m_dwClubMasterID, false );
			}
		}
		break;

	case MODAL_CLUB_JOIN:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				DWORD dwGaeaID = m_pPtoPWindow->GetPlayerID ();
				GLGaeaClient::GetInstance().GetCharacter ()->ReqClubJoin ( dwGaeaID );
			}
		}
		break;

	case MODAL_CLUB_BREAKUP_CANCEL:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				GLGaeaClient::GetInstance().GetCharacter ()->ReqClubDissolutionCancel ();
			}
		}
		break;

	case MODAL_CLUB_SELFQUIT:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				//	Ż�� Req
				GLGaeaClient::GetInstance().GetCharacter ()->ReqClubSecede ();
			}
		}
		break;

	case MODAL_SKILL_LEVEL_UP_CONFIRM:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				SNATIVEID sNativeID ( GetSkillUpID () );
				GLGaeaClient::GetInstance().GetCharacter()->ReqSkillUp ( sNativeID );
			}
		}
		break;

	case MODAL_COMMISSION:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				CString strCommission = m_pModalWindow->GetEditString ();
				float fCommission = (float) atof ( strCommission.GetString () );
				STARGETID sCrowID = m_pDialogueWindow->GetCrowID ();

				GLGaeaClient::GetInstance().GetCharacter()->ReqGuidCommission ( sCrowID.dwID, fCommission );
			}
		}
		break;

	case MODAL_FRIEND_BLOCK_ADD_N:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				CString strName = m_pFriendWindow->GET_FRIEND_NAME();
				GLFriendClient::GetInstance().ReqFriendBlock ( strName.GetString (), true );
			}
		}
		break;
        
	case MODAL_FRIEND_BLOCK_ADD_B:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				CString strName = m_pFriendWindow->GET_BLOCK_NAME();
				GLFriendClient::GetInstance().ReqFriendBlock ( strName.GetString (), true );
			}
		}
		break;

	case MODAL_FRIEND_BLOCK_DEL:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				CString strName = m_pFriendWindow->GET_BLOCK_NAME ();
				GLFriendClient::GetInstance().ReqFriendBlock ( strName.GetString (), false );
			}
		}
		break;

	case MODAL_UNION_JOIN_ASK:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{						
				GLGaeaClient::GetInstance().GetCharacter()->ReqClubAllianceAns( m_dwClubMasterID, true );
			}
			else if ( UIMSG_MODAL_CANCEL & dwMsg )
			{
				GLGaeaClient::GetInstance().GetCharacter()->ReqClubAllianceAns( m_dwClubMasterID, false );
			}
		}
		break;

	case MODAL_UNION_JOIN:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				DWORD dwGaeaID = m_pPtoPWindow->GetPlayerID ();
				GLGaeaClient::GetInstance().GetCharacter()->ReqClubAlliance( dwGaeaID );
			}
		}
		break;

	case MODAL_CLUB_BATTLE_ASK:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{						
				GLGaeaClient::GetInstance().GetCharacter()->ReqClubBattleAns( m_dwClubMasterID, true );
			}
			else if ( UIMSG_MODAL_CANCEL & dwMsg )
			{
				GLGaeaClient::GetInstance().GetCharacter()->ReqClubBattleAns( m_dwClubMasterID, false );
			}
		}
		break;

	case MODAL_ALLIANCE_BATTLE_ASK:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{						
				GLGaeaClient::GetInstance().GetCharacter()->ReqAllianceBattleAns( m_dwClubMasterID, true );
			}
			else if ( UIMSG_MODAL_CANCEL & dwMsg )
			{
				GLGaeaClient::GetInstance().GetCharacter()->ReqAllianceBattleAns( m_dwClubMasterID, false );
			}
		}
		break;

	case MODAL_CLUB_BATTLE_ARMISTICE:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				DWORD dwClubID = m_pClubWindow->GetBattleClubID ();
				GLGaeaClient::GetInstance().GetCharacter()->ReqClubBattleArmistice( dwClubID );
			}
		}
		break;

	case MODAL_ALLIANCE_BATTLE_ARMISTICE:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				DWORD dwClubID = m_pClubWindow->GetBattleClubID ();
				GLGaeaClient::GetInstance().GetCharacter()->ReqAllianceBattleArmistice( dwClubID );
			}
		}
		break;

	case MODAL_CLUB_BATTLE_ARMISTICE_ASK:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{						
				GLGaeaClient::GetInstance().GetCharacter()->ReqClubBattleArmisticeAns( m_dwClubMasterID, true );
			}
			else if ( UIMSG_MODAL_CANCEL & dwMsg )
			{
				GLGaeaClient::GetInstance().GetCharacter()->ReqClubBattleArmisticeAns( m_dwClubMasterID, false );
			}
		}
		break;

	case MODAL_ALLIANCE_BATTLE_ARMISTICE_ASK:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{						
				GLGaeaClient::GetInstance().GetCharacter()->ReqAllianceBattleArmisticeAns( m_dwClubMasterID, true );
			}
			else if ( UIMSG_MODAL_CANCEL & dwMsg )
			{
				GLGaeaClient::GetInstance().GetCharacter()->ReqAllianceBattleArmisticeAns( m_dwClubMasterID, false );
			}
		}
		break;

	case MODAL_CLUB_BATTLE_SUBMISSION:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				DWORD dwClubID = m_pClubWindow->GetBattleClubID ();
				GLGaeaClient::GetInstance().GetCharacter()->ReqClubBattleSubmission( dwClubID );
			}
		}
		break;

	case MODAL_ALLIANCE_BATTLE_SUBMISSION:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				DWORD dwClubID = m_pClubWindow->GetBattleClubID ();
				GLGaeaClient::GetInstance().GetCharacter()->ReqAllianceBattleSubmission( dwClubID );
			}
		}
		break;

	case MODAL_UNION_KICK:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				const DWORD dwClubID = m_pClubWindow->GetUnionMemberID();
				GLGaeaClient::GetInstance().GetCharacter()->ReqClubAllianceDel( dwClubID );
			}
		}
		break;

	case MODAL_UNION_LEAVE:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				//	Ż�� Req
				GLGaeaClient::GetInstance().GetCharacter ()->ReqClubAllianceSec();
			}
		}
		break;

	case MODAL_UNION_BREAKUP:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				GLGaeaClient::GetInstance().GetCharacter ()->ReqClubAllianceDis();
			}
		}
		break;

	case MODAL_CHAR_RENAME:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				CString strName = m_pModalWindow->GetEditString ();
				GLGaeaClient::GetInstance().GetCharacter()->ReqInvenRename( strName.GetString() );
			}
		}
		break;

	case MODAL_PET_RENAME:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				CString strName = m_pModalWindow->GetEditString ();
				GLGaeaClient::GetInstance().GetPetClient()->ReqRename( strName.GetString() );
			}
		}
		break;
	case MODAL_RESET_SKILLSTATS:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				GLGaeaClient::GetInstance().GetCharacter()->ReqResetSkillStats( );
			}

		}
		break;

	//CNDev | 2-7-2020 | e-point card system
	case MODAL_ADD_EPOINT:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				GLGaeaClient::GetInstance().GetCharacter()->ReqAddEPoint();
			}
		}
		break;
		
	case MODAL_RESET_BRIGHT:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				GLGaeaClient::GetInstance().GetCharacter()->ReqResetBrightPoint();
			}
		}
		break;
		
	case MODAL_ADD_CONTRIBUTION:
		{
			if ( UIMSG_MODAL_OK & dwMsg )
			{
				GLGaeaClient::GetInstance().GetCharacter()->ReqContributionPointCard();
			}
		}


	case MODAL_ITEMREBUILD_MONEY:	// ITEMREBUILD_MARK
		{
			if( UIMSG_MODAL_OK & dwMsg )
			{
				CString strInputMoney = GetModalWindow()->GetEditString();
				LONGLONG i64InputMoney = _ttoi64( strInputMoney.GetString() );
				GLGaeaClient::GetInstance().GetCharacter()->ReqRebuildInputMoney( i64InputMoney );
			}
		}
		break;

	case MODAL_ITEMREBUILD_OKQUESTION:
		{
			if( UIMSG_MODAL_OK & dwMsg )
			{
				/*dmk14 reform*/
				GLGaeaClient::GetInstance().GetCharacter()->ReqRebuildResult();
			}
		}
		break;
		/*dmk14 reform*/
	case MODAL_ITEMREBUILD_PREVQUESTION:
		{
			if( UIMSG_MODAL_OK & dwMsg )
			{
				GLGaeaClient::GetInstance().GetCharacter()->ReqRebuildPREV();
			}
		}
		break;

	case MODAL_ITEMGARBAGE_OKQUESTION:
		{
			if( UIMSG_MODAL_OK & dwMsg )
			{
				GLGaeaClient::GetInstance().GetCharacter()->ReqGarbageResult();
			}
		}
		break;

	case MODAL_HAIRCOLOR_INFO:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				ShowGroupFocus( HAIRCOLORCARD_WINDOW );
			}
		}
		break;

	case MODAL_PETCOLOR_INFO:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				ShowGroupFocus( PETCOLORCARD_WINDOW );
			}
		}
		break;

	case MODAL_MOBILE_NUMBER:
		{
			if( UIMSG_MODAL_OK & dwMsg )
			{
				CString strInputNumber = GetModalWindow()->GetEditString();
				GLGaeaClient::GetInstance().GetCharacter()->ReqSetPhoneNumber( strInputNumber );
			}
		}
		break;

	case MODAL_GENDER_CHANGE:
		{
			if( UIMSG_MODAL_OK & dwMsg )
			{
				ShowGroupFocus( GENDER_CHANGE_WINDOW );
			}
		}
		break;
	case MODAL_GENDER_CHANGE_END:
		{
			if( UIMSG_MODAL_OK & dwMsg )
			{
				WAITSERVER_DIALOGUE_OPEN ( ID2GAMEINTEXT("WAITSERVER_MESSAGE"), WAITSERVER_TOLOBY );
			}
		}
		break;

	case MODAL_ITEMMIX_OKQUESTION:
		{
			if( UIMSG_MODAL_OK & dwMsg )
			{
				m_pItemMixWindow->SetAnimation();
			}
		}
		break;

	case MODAL_LOCKER_CONNECT_REQ:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				GLGaeaClient::GetInstance().GetCharacter()->ReqStorageOpenCardInvenConfirmed ();
			}
		}
		break;

		/*npc shop, Juver, 2017/07/27 */
	case MODAL_NPCSHOP_PURCHASE:
		{
			if( UIMSG_MODAL_OK & dwMsg )
			{
				if ( dwMsg & UIMSG_MODAL_OK )
				{
					GLGaeaClient::GetInstance().GetCharacter()->ReqNpcShopDoPurchase();
				}
				else if ( dwMsg & UIMSG_MODAL_CANCEL )
				{
					GLGaeaClient::GetInstance().GetCharacter()->ReqNpcShopPurchaseReset();
				}
			}
		}break;

		/*product item, Juver, 2017/10/18 */
	case MODAL_ITEM_PRODUCT_PRODUCE:
		{
			if( UIMSG_MODAL_OK & dwMsg )
			{
				if ( dwMsg & UIMSG_MODAL_OK )
				{
					DWORD dwid = GLGaeaClient::GetInstance().GetCharacter()->GetItemCompoundID();
					int nCount = GLGaeaClient::GetInstance().GetCharacter()->GetItemCompoundCount();
					GLGaeaClient::GetInstance().GetCharacter()->ReqItemCompound( dwid, nCount );
				}
			}
		}break;

		/*item wrapper, Juver, 2018/01/12 */
	case MODAL_WRAP_ITEM:
		{
			if( UIMSG_MODAL_OK & dwMsg )
			{
				if ( dwMsg & UIMSG_MODAL_OK )
					GLGaeaClient::GetInstance().GetCharacter()->ReqItemWrapMsg();
			}
		}break;

		/*item wrapper, Juver, 2018/01/12 */
	case MODAL_UNWRAP_ITEM:
		{
			if( UIMSG_MODAL_OK & dwMsg )
			{
				if ( dwMsg & UIMSG_MODAL_OK )
					GLGaeaClient::GetInstance().GetCharacter()->ReqItemUnwrapMsg();
			}
		}break;

	/*dmk14 ingame web*/
	case MODAL_ITEMMALL_BUY_ITEM:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				//CItemMallWindow* pItemMall = GetItemMallWindow();
				//ITEMMALLDATA sData = pItemMall->GetMenuPage()->GetItemMallData();
				//GLGaeaClient::GetInstance().GetCharacter()->ReqItemMallBuy( sData );

				/*dmk14 itemshop new ui*/
				GLGaeaClient::GetInstance().GetCharacter()->ReqItemMallBuy ( GetItemShopWindow()->GetItemMallData() );
			}
		}
		break;

	/*dmk14 remove active buff selection*/
	case MODAL_SKILLFACT_ASK_RESET:
		{
			if ( dwMsg & UIMSG_MODAL_OK )
			{
				if( !IsFBuffRemove() )
				{
					SNATIVEID sSKILLID = GetSKEFFID();
					for ( int i = 0; i < SKILLFACT_SIZE; i++ )
					{
						const SSKILLFACT& sSKILLFACT = GLGaeaClient::GetInstance().GetCharacter()->m_sSKILLFACT[i];
						if ( sSKILLID == sSKILLFACT.sNATIVEID ) GLGaeaClient::GetInstance().GetCharacter()->ReqDisableSkill( i, sSKILLID );
					}
				}
				else
				{
					SNATIVEID sSKILLID = GetSKEFFID();
					for ( int i = 0; i < FITEMFACT_SIZE; i++ )
					{
						const SFITEMFACT& sFITEMFACT = GLGaeaClient::GetInstance().GetCharacter()->m_sFITEMFACT[i];
						if ( sSKILLID == sFITEMFACT.sNATIVEID ) GLGaeaClient::GetInstance().GetCharacter()->ReqDisableItemFact( i, sSKILLID );
					}
				}
			}
		}
		break;

	/*Codex System, Jhoniex 2/14/2024 */
	case MODAL_REGISTER_CODEX:
		{
			if( UIMSG_MODAL_OK & dwMsg )
			{
				if ( dwMsg & UIMSG_MODAL_OK )
				{
					SNATIVEID msidItem;
					msidItem.wMainID = 0;
					msidItem.wSubID = 0;
					DWORD dwActivityID = 0;
					WORD wGrade = 0;
					WORD wQuantity = 0;

					GetModalWindow()->GetCodexModalData ( &msidItem, &dwActivityID, &wGrade, &wQuantity );
					GLGaeaClient::GetInstance().GetCharacter()->ReqCodexRegister(msidItem, dwActivityID, wGrade, wQuantity);
				}
			}
		}break;
	}
}