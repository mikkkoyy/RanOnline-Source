#include "StdAfx.h"

#include "../Lib_Engine/Common/StringFormat.h"

#include "UITextControl.h"
#include "GameTextControl.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "BasicLineBoxSmart.h"
#include "BasicTextButton.h"
#include "BasicTextBoxEx.h"
#include "BasicLineBox.h"
#include "BasicScrollBarEx.h"
#include "BasicScrollThumbFrame.h"
#include "ModalWindow.h"
#include "ModalCallerID.h"

#include "RANPARAM.h"

#include "../Lib_Client/NpcTalk/NpcDialogue.h"
#include "../Lib_Client/NpcTalk/NpcDialogueCase.h"
#include "../Lib_Client/NpcTalk/NpcTalkControl.h"
#include "../Lib_Client/NpcTalk/NpcTalk.h"
#include "../Lib_Client/NpcTalk/NpcTalkCondition.h"

#include "../Lib_Client/G-Logic/GLCrowDataAction.h"

#include "../Lib_Client/G-Logic/GLItem.h"
#include "../Lib_Client/G-Logic/GLItemMan.h"
#include "../Lib_Client/G-Logic/GLQuestMan.h"
#include "../Lib_Client/G-Logic/GLQuest.h"

#include "../Lib_Client/G-Logic/GLGaeaClient.h"
#include "../Lib_Client/G-Logic/GLCrowClient.h"
#include "../Lib_Client/G-Logic/GLCharacter.h"

#include <map>

#include "InnerInterface.h"
#include "NPCDialoguePage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int CNPCDialoguePage::nSTARTINDEX = 1;
const int CNPCDialoguePage::nSTARTLINE = 0;
const float CNPCDialoguePage::fANSWER_PART_INTERVAL = 2.0f;

CNPCDialoguePage::CNPCDialoguePage( )
    : m_pRandomTimeButton(NULL)
    , m_pRandomTimeText(NULL)
    , m_pRandomTimeBack(NULL)
    , m_pTalkBack(NULL)
    , m_pTalkText(NULL)
    , m_pTalkScrollBar(NULL)
    , m_bButtonRandom(false)
    , m_nTalkButtonTotalLine(0)
    , m_nTalkButtonStartIndex(0)
{
    for ( int i = 0; i < NUM_TALK_BUTTON; i++ )
    {
        m_pTalkButton[ i ] = NULL;
        m_pTalkButtonText[ i ] = NULL;
    }
}

CNPCDialoguePage::~CNPCDialoguePage()
{
}

void CNPCDialoguePage::CreateSubControl()
{
	
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );
	CD3DFontPar* pFont9Shadow = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_EX_FLAG );
	CD3DFontPar* pFont11Shadow = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 11, _DEFAULT_FONT_SHADOW_EX_FLAG );

    // RandomTime
    {
        const int nBUTTONSIZE = CBasicTextButton::SIZE16;
        m_pRandomTimeButton = new CBasicTextButton();
        m_pRandomTimeButton->CreateSub( this, "BASIC_TEXT_BUTTON19", UI_FLAG_XSIZE, RANDOM_TIME_BUTTON );
        m_pRandomTimeButton->CreateBaseButton( "RANDOM_TIME_BUTTON", nBUTTONSIZE, CBasicButton::RADIO_FLIP, ID2GAMEWORD( "RANDOM_TIME_BUTTON",0));
        m_pRandomTimeButton->SetFlip( FALSE );
        RegisterControl( m_pRandomTimeButton );

        m_pRandomTimeBack = new CBasicLineBox();
        m_pRandomTimeBack->CreateSub( this, "BASIC_LINE_BOX_EDIT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
        m_pRandomTimeBack->CreateBaseBoxEditBox( "RANDOM_TIME_BACK" );
        RegisterControl( m_pRandomTimeBack );    


        m_pRandomTimeText = new CBasicTextBox();
        m_pRandomTimeText->CreateSub( this, "RANDOM_TIME_TEXT", UI_FLAG_DEFAULT, RANDOM_TIME_TEXT );
        m_pRandomTimeText->SetFont( pFont9 );
        m_pRandomTimeText->SetTextAlign( TEXT_ALIGN_CENTER_X );	
        RegisterControl ( m_pRandomTimeText );
    }

    {
        CBasicLineBoxSmart* pLinBoxSmart = new CBasicLineBoxSmart();
        pLinBoxSmart->CreateSub( this, "DIALOGUE_WINDOW_TALK_BACK", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
        pLinBoxSmart->CreateSubControl( "DIALOGUE_WINDOW_TALK_BACK_LINE" );
        RegisterControl( pLinBoxSmart );
        m_pTalkBack = pLinBoxSmart;

        CBasicTextBox* pTextBox = new CBasicTextBox();
        pTextBox->CreateSub( this, "DIALOGUE_WINDOW_TALK_TEXT", UI_FLAG_DEFAULT, DIALOGUE_TALK_TEXT );
        pTextBox->SetFont( pFont11Shadow );
        pTextBox->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );
        RegisterControl( pTextBox );
        m_pTalkText = pTextBox;

        int nTotalLine = m_pTalkText->GetVisibleLine();

        //	스크롤바
        m_pTalkScrollBar = new CBasicScrollBarEx();
        m_pTalkScrollBar->CreateSub( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, DIALOGUE_TALK_SCROLL );
        m_pTalkScrollBar->CreateBaseScrollBar( "DIALOGUE_WINDOW_TALK_TEXT_SCROLL_BAR" );
        m_pTalkScrollBar->GetThumbFrame()->SetState( 1, nTotalLine );
        RegisterControl( m_pTalkScrollBar );

    }

    {
        char* strUIButton[ NUM_TALK_BUTTON ] =
        {
            "DIALOGUE_WINDOW_TALK_BUTTON_0"
            , "DIALOGUE_WINDOW_TALK_BUTTON_1"
            , "DIALOGUE_WINDOW_TALK_BUTTON_2"
            , "DIALOGUE_WINDOW_TALK_BUTTON_3"
            , "DIALOGUE_WINDOW_TALK_BUTTON_4"
            , "DIALOGUE_WINDOW_TALK_BUTTON_5"
        };

        char* strUIButtonText[ NUM_TALK_BUTTON ] =
        {
            "DIALOGUE_WINDOW_TALK_BUTTON_0_TEXT"
            , "DIALOGUE_WINDOW_TALK_BUTTON_1_TEXT"
            , "DIALOGUE_WINDOW_TALK_BUTTON_2_TEXT"
            , "DIALOGUE_WINDOW_TALK_BUTTON_3_TEXT"
            , "DIALOGUE_WINDOW_TALK_BUTTON_4_TEXT"
            , "DIALOGUE_WINDOW_TALK_BUTTON_5_TEXT"
        };

        for ( int i = 0; i < NUM_TALK_BUTTON; i++ )
        {
            CBasicLineBoxSmart* pLinBoxSmart = new CBasicLineBoxSmart();
            pLinBoxSmart->CreateSub( this, strUIButton[ i ], UI_FLAG_XSIZE | UI_FLAG_YSIZE, ( DIUALOGUE_TALK_BUTTON_0 + i ) );
            pLinBoxSmart->CreateSubControl( "DIALOGUE_WINDOW_TALK_BUTTON_0_LINE" );
            RegisterControl( pLinBoxSmart );
            m_pTalkButton[ i ] = pLinBoxSmart;

            CBasicTextBoxEx* pBaiscTextBoxEx = new CBasicTextBoxEx();
            pBaiscTextBoxEx->CreateSub( this, strUIButtonText[ i ], UI_FLAG_DEFAULT, ( DIUALOGUE_TALK_BUTTON_TEXT_0 + i ) );
            pBaiscTextBoxEx->SetFont( pFont9Shadow );
            pBaiscTextBoxEx->SetTextAlign( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
            pBaiscTextBoxEx->SetPartInterval( fANSWER_PART_INTERVAL );
            pBaiscTextBoxEx->SetSensitive( true );
            RegisterControl( pBaiscTextBoxEx );
            m_pTalkButtonText[ i ] = pBaiscTextBoxEx;
        }

        //	스크롤바
        m_pTalkButtonScrollBar = new CBasicScrollBarEx();
        m_pTalkButtonScrollBar->CreateSub( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, DIUALOGUE_TALK_BUTTON_SCROLL_BAR );
        m_pTalkButtonScrollBar->CreateBaseScrollBar( "DIALOGUE_WINDOW_TALK_BUTTON_SCROLL_BAR" );
        m_pTalkButtonScrollBar->GetThumbFrame()->SetState( 1, NUM_TALK_BUTTON );
        RegisterControl( m_pTalkButtonScrollBar );
    }
}

void CNPCDialoguePage::Update( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
    if ( !CUIControlNpcActionable::IsUseable( ) )
    {
        CInnerInterface::GetInstance().SetDialogueWindowClose();
        return;
    }

    for ( int i = 0; i < NUM_TALK_BUTTON; i++ )
        m_pTalkButtonText[ i ]->SetUseOverColor( FALSE );

    CUIGroup::Update( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

    if ( bFirstControl )
    {
        {
            CBasicScrollThumbFrame* const pThumbFrame = m_pTalkScrollBar->GetThumbFrame();		
            m_pTalkText->SetCurLine( 0 );
            const int nTotalLine = m_pTalkText->GetTotalLine();
            const int nLinePerOneView = m_pTalkText->GetVisibleLine();
            if ( nLinePerOneView < nTotalLine )
            {
                const int nMovableLine = nTotalLine - nLinePerOneView;
                float fPercent = pThumbFrame->GetPercent();
                int nPos = static_cast<int>( floor(fPercent * nMovableLine) );
                if ( nPos < nSTARTLINE )
                    nPos = nSTARTLINE;
                m_pTalkText->SetCurLine( nPos );			
            }
        }

        {
            CBasicScrollThumbFrame* const pThumbFrame = m_pTalkButtonScrollBar->GetThumbFrame();		
            pThumbFrame->SetState( m_nTalkButtonTotalLine, NUM_TALK_BUTTON );
            if ( NUM_TALK_BUTTON < m_nTalkButtonTotalLine )
            {
                const int nMovableLine = m_nTalkButtonTotalLine - NUM_TALK_BUTTON;
                float fPercent = pThumbFrame->GetPercent();
                int nPos = (int)floor(fPercent * nMovableLine);
                if ( nPos != m_nTalkButtonStartIndex )
                {
//					DWORD dwPageIndex(0);
//					if ( m_spDialogue )
//						dwPageIndex = m_spDialogue->FindConditionSatisfied(m_pCharLogic, m_pGaeaClient->GetMyPartyClient().GetNMember() );

                    m_nTalkButtonStartIndex = nPos;
                    LoadButtonTalk( m_bButtonRandom, m_nCaseIdx );
                }
                else
                    m_nTalkButtonStartIndex = nPos;
            }
        }
    }
}

void CNPCDialoguePage::TranslateUIMessage( UIGUID ControlID, DWORD dwMsg )
{
    CUIGroup::TranslateUIMessage( ControlID, dwMsg );

    switch ( ControlID )
    {
    case DIUALOGUE_TALK_BUTTON_TEXT_0:
    case DIUALOGUE_TALK_BUTTON_TEXT_1:
    case DIUALOGUE_TALK_BUTTON_TEXT_2:
    case DIUALOGUE_TALK_BUTTON_TEXT_3:
    case DIUALOGUE_TALK_BUTTON_TEXT_4:
    case DIUALOGUE_TALK_BUTTON_TEXT_5:
        {
            if ( CHECK_MOUSE_IN( dwMsg ) )
            {
                int nButtonIndex = ControlID - DIUALOGUE_TALK_BUTTON_TEXT_0;
                if ( nButtonIndex < 0 || nButtonIndex >= NUM_TALK_BUTTON )
                    return;

                int nIndex = m_pTalkButtonText[ nButtonIndex ]->GetSelectPos();
                if ( nIndex < 0 )
                    return;

                m_pTalkButtonText[ nButtonIndex ]->SetUseOverColor( TRUE );		
                m_pTalkButtonText[ nButtonIndex ]->SetOverColor( nIndex, NS_UITEXTCOLOR::BLACK );

                if ( dwMsg & UIMSG_LB_UP )
                {
                    if ( m_nTalkButtonStartIndex + nButtonIndex + 1 == m_nTalkButtonTotalLine )
                    {
                        // 대화 종료 클릭.
						CInnerInterface::GetInstance().HideGroup( DIALOGUE_WINDOW_RENEWAL );
                        //m_pInterface->UiHideGroup( DIALOGUE_WINDOW_RENEWAL );
                        //m_pInterface->ItemRepairWindowOpen( false );
                        return;
                    }
                }

                const DWORD dwTalkID = m_pTalkButtonText[ nButtonIndex ]->GetTextData( nIndex );
                if (!m_spDialogueCase)
                    return;
				
				CNpcTalkControl* spNpcTalkControl = m_spDialogueCase->GetTalkControl();
				if ( !spNpcTalkControl ) return ;

				SNpcTalk* spNpcTalk = spNpcTalkControl->GetTalk ( dwTalkID );
				if ( !spNpcTalk ) return ;

                if ( dwMsg & UIMSG_LB_UP )
                {
                    TranslateTalkMessage( spNpcTalk );
                }
                else
                {
                    //	퀘스트 보상치 표현
                    if ( spNpcTalk->m_nACTION == SNpcTalk::EM_QUEST_START )
                    {						
                        DWORD dwQuestID = spNpcTalk->m_dwQuestStartID[0];
                        GLQUEST* pQuest = GLQuestMan::GetInstance().Find( dwQuestID );
                        if ( !pQuest )
                        {
                            return;
                        }

                        //	보상치가 없으면 아무것도 표시하지 않음
                        if ( !(pQuest->m_llGiftEXP || pQuest->m_dwGiftMONEY ||
                            pQuest->m_dwGiftELEMENT || pQuest->m_dwGiftLIFEPOINT ||
                            pQuest->m_dwGiftSKILLPOINT || pQuest->m_dwGiftSTATSPOINT ||
                            pQuest->m_vecGiftITEM.size () || pQuest->m_vecGiftSKILL.size ()) )
                        {
                            return ;
                        }

						if ( CInnerInterface::GetInstance().BEGIN_COMMON_LINEINFO_MULTI() )
                        {
							CInnerInterface::GetInstance().ADD_COMMON_LINEINFO_MULTI( ID2GAMEWORD("QUEST_GIFT",9), NS_UITEXTCOLOR::GREENYELLOW );

                            std::string strCombine;

                            if ( pQuest->m_llGiftEXP )
                            {
                                strCombine = sc::string::format( "%s : %d", ID2GAMEWORD("QUEST_GIFT",0), pQuest->m_llGiftEXP );
                                CInnerInterface::GetInstance().ADD_COMMON_LINEINFO_MULTI( strCombine.c_str(), NS_UITEXTCOLOR::WHITE );
                            }
                            if ( pQuest->m_dwGiftMONEY )
                            {
                                strCombine = sc::string::format( "%s : %d", ID2GAMEWORD("QUEST_GIFT",1), pQuest->m_dwGiftMONEY );
                                CInnerInterface::GetInstance().ADD_COMMON_LINEINFO_MULTI( strCombine.c_str(), NS_UITEXTCOLOR::WHITE );
                            }
                            if ( pQuest->m_dwGiftELEMENT )
                            {
                                strCombine = sc::string::format( "%s : %d", ID2GAMEWORD("QUEST_GIFT",2), pQuest->m_dwGiftELEMENT );
                                CInnerInterface::GetInstance().ADD_COMMON_LINEINFO_MULTI( strCombine.c_str(), NS_UITEXTCOLOR::WHITE );
                            }
                            if ( pQuest->m_dwGiftLIFEPOINT )
                            {
                                strCombine = sc::string::format( "%s : %d", ID2GAMEWORD("QUEST_GIFT",3), pQuest->m_dwGiftLIFEPOINT );
                                CInnerInterface::GetInstance().ADD_COMMON_LINEINFO_MULTI( strCombine.c_str(), NS_UITEXTCOLOR::WHITE );
                            }
                            if ( pQuest->m_dwGiftSKILLPOINT )
                            {
                                strCombine = sc::string::format( "%s : %d", ID2GAMEWORD("QUEST_GIFT",5), pQuest->m_dwGiftSKILLPOINT );
                                CInnerInterface::GetInstance().ADD_COMMON_LINEINFO_MULTI( strCombine.c_str(), NS_UITEXTCOLOR::WHITE );
                            }
                            if ( pQuest->m_dwGiftSTATSPOINT )
                            {
                                strCombine = sc::string::format( "%s : %d", ID2GAMEWORD("QUEST_GIFT",6), pQuest->m_dwGiftSTATSPOINT );
                                CInnerInterface::GetInstance().ADD_COMMON_LINEINFO_MULTI( strCombine.c_str(), NS_UITEXTCOLOR::WHITE );
                            }

                            if ( pQuest->m_vecGiftITEM.size() )
                            {
                                std::string strItem = ID2GAMEWORD( "QUEST_GIFT",7 );
                                strItem += " : ";
                                INVENQARRAY_ITER iter = pQuest->m_vecGiftITEM.begin();
                                INVENQARRAY_ITER iter_end = pQuest->m_vecGiftITEM.end();
                                for ( ; iter != iter_end; )
                                {
                                    SITEMCUSTOM& sItemCustom = (*iter);

                                    ++iter;

                                    const SITEM* pItem = GLItemMan::GetInstance().GetItem( sItemCustom.GetNativeID() );
                                    if ( !pItem )
                                        continue;

                                    strItem += sItemCustom.GETNAME();
                                    if ( iter == iter_end )
                                        break;

                                    strItem += ", ";
                                }
                                CInnerInterface::GetInstance().ADD_COMMON_LINEINFO_MULTI( strItem.c_str(), NS_UITEXTCOLOR::WHITE );
                            }

                            if ( pQuest->m_vecGiftSKILL.size() )
                            {
                                std::string strSkill = ID2GAMEWORD("QUEST_GIFT",8);
                                strSkill += " : ";
                                DWQARRAY_ITER iter = pQuest->m_vecGiftSKILL.begin();
                                DWQARRAY_ITER iter_end = pQuest->m_vecGiftSKILL.end();
                                for ( ; iter != iter_end; )
                                {
                                    DWORD dwID = (*iter);									
                                    PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( SNATIVEID(dwID) );
                                    if ( !pSKILL ) continue;

                                    strSkill += pSKILL->GetName();

                                    ++iter;

                                    if ( iter == iter_end )
                                        break;

                                    strSkill += ", ";
                                }
                                CInnerInterface::GetInstance().ADD_COMMON_LINEINFO_MULTI( strSkill.c_str(), NS_UITEXTCOLOR::WHITE );
                            }

                            CInnerInterface::GetInstance().END_COMMON_LINEINFO_MULTI();
                        }
                    }
                }
            }
        }
        break;
    };
}
void CNPCDialoguePage::TranslateTalkMessage( SNpcTalk* spNpcTalk )
{
    m_nTalkButtonStartIndex = 0;

    if ( !spNpcTalk )
        return;

    switch ( spNpcTalk->m_nACTION )
    {
    case SNpcTalk::EM_DO_NOTHING:
        {
        }
        break;
    case SNpcTalk::EM_PAGE_MOVE:
        {
            LoadNode( spNpcTalk->m_dwACTION_NO );
        }
        break;
    case SNpcTalk::EM_BASIC:
        {
            TranslateBasicMessage( spNpcTalk );
        }
        break;
    case SNpcTalk::EM_QUEST_START:
        {
            LoadNode( spNpcTalk->m_dwACTION_NO );

            TranslateQuestStartMessage( spNpcTalk );
        }
        break;
    case SNpcTalk::EM_QUEST_STEP:
        {
            LoadNode( spNpcTalk->m_dwACTION_NO );

            TranslateQuestStepMessage( spNpcTalk );
        }
        break;
    };
}

void CNPCDialoguePage::TranslateBasicMessage( SNpcTalk* spNpcTalk )
{
    if ( !spNpcTalk )
        return;

    // Need Add New Item Con.
    DWORD dwMsg = spNpcTalk->m_dwACTION_NO;
    DWORD dwTalkID = spNpcTalk->m_dwGLOB_ID;
	SNPC_ITEM *pParamA = spNpcTalk->m_sNeedItem;
	DWORD paramB = spNpcTalk->m_dwACTION_PARAM2;

	PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();
	if ( !pLand )	return;

	PGLCROWCLIENT pCrow = pLand->GetCrow ( CUIControlNpcActionable::GetNPCGlobalID() );
	if ( !pCrow )	return;

    // ReqNpcTalkBasic() 대화문 유효성 체크 ( 서버 ) 
    switch ( dwMsg )
    {
    case SNpcTalk::EM_STORAGE:
        {
			CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );
			CInnerInterface::GetInstance().SetDefaultPosInterface( INVENTORY_WINDOW );
			CInnerInterface::GetInstance().SetDefaultPosInterface( STORAGE_WINDOW );
			//CInnerInterface::GetInstance().ShowGroupFocus ( INVENTORY_WINDOW );
			CInnerInterface::GetInstance().ShowInventoryWindow();
			CInnerInterface::GetInstance().SetStorageWindowOpen ( CUIControlNpcActionable::GetNPCGlobalID() );
        }
        break;
    case SNpcTalk::EM_MARKET:
        {
			CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );
			CInnerInterface::GetInstance().SetDefaultPosInterface( INVENTORY_WINDOW );
			CInnerInterface::GetInstance().SetDefaultPosInterface( MARKET_WINDOW );
			CInnerInterface::GetInstance().SetMarketWindowOpen ( CUIControlNpcActionable::GetNPCGlobalID() );
			//CInnerInterface::GetInstance().ShowGroupFocus ( INVENTORY_WINDOW );
			CInnerInterface::GetInstance().ShowInventoryWindow();
        }
        break;
    case SNpcTalk::EM_CURE:
        {
			CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );
			GLGaeaClient::GetInstance().GetCharacter()->ReqCure ( CUIControlNpcActionable::GetNPCGlobalID(), dwTalkID );
        }
        break;
    case SNpcTalk::EM_STARTPOINT:
        {
			CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );
			GLGaeaClient::GetInstance().GetCharacter()->ReqReGenGate ( CUIControlNpcActionable::GetNPCGlobalID() );
        }
        break;

    case SNpcTalk::EM_CHAR_RESET:
        {
			CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );
			GLGaeaClient::GetInstance().GetCharacter()->ReqCharReset ( CUIControlNpcActionable::GetNPCGlobalID() );
        }
        break;

        // Need Add New Item Con.
    case SNpcTalk::EM_ITEM_TRADE:
        {
			CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );
			GLGaeaClient::GetInstance().GetCharacter()->ReqItemTrade ( CUIControlNpcActionable::GetNPCGlobalID(), dwTalkID, pParamA, paramB );
        }
        break;

    case SNpcTalk::EM_BUSSTATION:
        {
			CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );
			
			//	NOTE : 버스 정류장 인터페이스 로드.
			CInnerInterface::GetInstance().SetBusWindowOpen ( CUIControlNpcActionable::GetNPCGlobalID(), spNpcTalk );
        }
        break;

    case SNpcTalk::EM_CLUB_NEW:
        {
			CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );

            //	NOTE : 클럽 만들기 신청
			CInnerInterface::GetInstance().SetClubMake ( CUIControlNpcActionable::GetNPCGlobalID() );
        }
        break;

    case SNpcTalk::EM_CLUB_UP:
        {
			CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );
			
			GLGaeaClient::GetInstance().GetCharacter()->ReqClubRank(CUIControlNpcActionable::GetNPCGlobalID());
        }
        break;

    case SNpcTalk::EM_CD_CERTIFY:
        {
			CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );
			GLGaeaClient::GetInstance().GetCharacter()->ReqCDCertify(CUIControlNpcActionable::GetNPCGlobalID());
        }
        break;

    case SNpcTalk::EM_COMMISSION:
        {
			CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );

			//	NOTE : 수수료 지정
			//
			DoModal ( ID2GAMEINTEXT("MODAL_COMMISSION"), MODAL_INPUT, EDITBOX, MODAL_COMMISSION );
        }
        break;

    case SNpcTalk::EM_CLUB_STORAGE:
        {
			GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter ();
			GLCLUB& sCLUB = pCharacter->m_sCLUB;
			bool bVALID_CLUB = sCLUB.m_dwID!=CLUB_NULL;
			bool bMASTER = sCLUB.m_dwMasterID==pCharacter->m_dwCharID;

			if ( bVALID_CLUB && bMASTER )
			{
				CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );

				pCharacter->ReqGetClubStorage ();

				//	NOTE : 창고 띄우기
				//			
				CInnerInterface::GetInstance().SetDefaultPosInterface( INVENTORY_WINDOW );
				CInnerInterface::GetInstance().SetDefaultPosInterface( CLUB_STORAGE_WINDOW );
				//CInnerInterface::GetInstance().ShowGroupFocus ( INVENTORY_WINDOW );
				CInnerInterface::GetInstance().ShowInventoryWindow();
				CInnerInterface::GetInstance().SetClubStorageWindowOpen ();
			}
			else
			{
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("ONLY_CLUB_MASTER_USE_CLUBSTORAGE") );
			}
        }
        break;

    case SNpcTalk::EM_ITEM_REBUILD:	// ITEMREBUILD_MARK
        {
			CInnerInterface::GetInstance().HideGroup( GetWndID() );
			CInnerInterface::GetInstance().OpenItemRebuildWindow();
        }
        break;

    case SNpcTalk::EM_ODDEVEN:
        {
			CInnerInterface::GetInstance().HideGroup( DIALOGUE_WINDOW_RENEWAL );
			CInnerInterface::GetInstance().SetDefaultPosInterface( GAMBLE_DIALOGUE );
			CInnerInterface::GetInstance().SetDefaultPosInterface( INVENTORY_WINDOW );
			CInnerInterface::GetInstance().ShowGroupFocus ( GAMBLE_DIALOGUE );
			//CInnerInterface::GetInstance().ShowGroupFocus ( INVENTORY_WINDOW );
			CInnerInterface::GetInstance().ShowInventoryWindow();
        }
        break;
    case SNpcTalk::EM_RECOVERY_EXP:
        {
			CInnerInterface::GetInstance().HideGroup( DIALOGUE_WINDOW_RENEWAL );
			GLGaeaClient::GetInstance().GetCharacter()->ReqGetReExpNpc( CUIControlNpcActionable::GetNPCGlobalID() );
        }
        break;
    case SNpcTalk::EM_RANDOM_PAGE:
        {
            int nCnt = MAX_RANDOM_PAGE;

            for ( int i = 0; i < MAX_RANDOM_PAGE; ++i )
            {
                if ( spNpcTalk->m_dwRandomPageID[i] == UINT_MAX )
                {
                    nCnt = i;
                    break;
                }
            }

            if ( nCnt <= 0 ) 
            {
                break;
            }

            int nChoice = int ( rand() % nCnt );

            LoadNode( spNpcTalk->m_dwRandomPageID[ nChoice ], true, spNpcTalk->m_dwRandomTime );
        }
        break;
    case SNpcTalk::EM_ITEMSEARCH_PAGE:
        {
			CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );
			CInnerInterface::GetInstance().HideGroup( ITEM_SEARCH_RESULT_WINDOW );
			CInnerInterface::GetInstance().SetDefaultPosInterface( ITEM_SHOP_SEARCH_WINDOW );
			CInnerInterface::GetInstance().ShowGroupFocus( ITEM_SHOP_SEARCH_WINDOW );
        }
        break;

    case SNpcTalk::EM_ATTENDANCE_BOOK:
        {
			CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );
			CInnerInterface::GetInstance().ShowGroupFocus ( ATTENDANCE_BOOK_WINDOW );
        }
        break;
    case SNpcTalk::EM_ITEM_MIX:
        {
			CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );

			/*product item, Juver, 2017/10/18 */
			if ( RANPARAM::bFeatureProduct )
				CInnerInterface::GetInstance().OpenProductWindow();
			else
				CInnerInterface::GetInstance().OpenItemMixWindow( CUIControlNpcActionable::GetNPCGlobalID() );
        }
        break;
		

		/*npc shop, Juver, 2017/07/25 */
	case SNpcTalk::EM_NPC_SHOP:
		{
			CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );
			CInnerInterface::GetInstance().HideGroup( NPC_SHOP_WINDOW );
			CInnerInterface::GetInstance().SetDefaultPosInterface( NPC_SHOP_WINDOW );
			CInnerInterface::GetInstance().SetNPCShopWindowOpen ( pCrow->m_pCrowData->sNativeID );
		}
		break;

		/*item exchange, Juver, 2017/10/12 */
	case SNpcTalk::EM_ITEM_EXCHANGE:
		{
			CInnerInterface::GetInstance().HideGroup ( DIALOGUE_WINDOW_RENEWAL );
			CInnerInterface::GetInstance().HideGroup( NPC_ITEM_EXCHANGE_WINDOW );
			CInnerInterface::GetInstance().SetDefaultPosInterface( NPC_ITEM_EXCHANGE_WINDOW );
			CInnerInterface::GetInstance().SetNPCItemExchangeWindowOpen ( pCrow->m_pCrowData->sNativeID );
		}
		break;
    };
}

void CNPCDialoguePage::TranslateQuestStartMessage( SNpcTalk* spNpcTalk )
{
    GASSERT( spNpcTalk && "CDialogueWindow::TranslateQuestStartMessage" );

    if ( !spNpcTalk )
        return;

	PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();
	if ( !pLand )	return;
    
	PGLCROWCLIENT pCrow = pLand->GetCrow ( CUIControlNpcActionable::GetNPCGlobalID() );
	if ( !pCrow )	return;

	for ( int i = 0; i < MAX_QUEST_START; ++i )
	{
		if ( spNpcTalk->m_dwQuestStartID[i] != UINT_MAX && spNpcTalk->DoTEST(m_pCharLogic, i))
		{
			GLGaeaClient::GetInstance().GetCharacter()->ReqQuestStart ( 
				CUIControlNpcActionable::GetNPCGlobalID(), spNpcTalk->m_dwGLOB_ID, spNpcTalk->m_dwQuestStartID[i] );
		}
	}
}

void CNPCDialoguePage::TranslateQuestStepMessage( SNpcTalk* spNpcTalk )
{
    GASSERT( spNpcTalk&&"CDialogueWindow::TranslateQuestStepMessage" );

    if ( !spNpcTalk )
        return;

	PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap();
	if ( !pLand )	return;
	PGLCROWCLIENT pCrow = pLand->GetCrow ( CUIControlNpcActionable::GetNPCGlobalID() );
	if ( !pCrow )	return;

	GLGaeaClient::GetInstance().GetCharacter()->ReqQuestStepNpcTalk ( CUIControlNpcActionable::GetNPCGlobalID(),
		spNpcTalk->m_dwGLOB_ID, spNpcTalk->m_dwACTION_PARAM1, spNpcTalk->m_dwACTION_PARAM2 );
}


bool CNPCDialoguePage::SetDialogueData( const SNATIVEID sNPCID, const DWORD dwNPCGlobalID, GLCHARLOGIC* pCharLogic )
{
    GASSERT ( pCharLogic );

    m_pCharLogic = pCharLogic;
	
	PGLCROWCLIENT pCrow = GLGaeaClient::GetInstance().GetActiveMap()->GetCrow(dwNPCGlobalID);
    if ( !pCrow )						
        return false;

    if ( pCrow->GETCROW() != CROW_NPC )
        return false;

    CUIControlNpcActionable::SetNpcActionableNPCID( sNPCID, dwNPCGlobalID );

	/*
    const PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData( pCrow->m_pCrowData->sNativeID );
    std::tr1::shared_ptr<NpcTalkMan> spTalkMan = GLogicData::GetInstance().GetNpcTalkMan();
    if (spTalkMan)
    {
        std::tr1::shared_ptr<CNpcDialogueSet> spDlgSet = spTalkMan->Find( pCrowData->GetTalkFile() );
        if ( spDlgSet )
        {
            m_spDialogueSet = spDlgSet;
            m_nTalkButtonStartIndex = 0;
            LoadNode( nSTARTINDEX );
            return true;
        }
    }*/

	PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData( pCrow->m_pCrowData->sNativeID );
	if ( m_spDialogueSet.Load ( pCrowData->GetTalkFile() ) )
	{
		//m_strTalkFile = pCrowData->GetTalkFile();
		//LoadNode ( nSTARTINDEX );
		//return TRUE;
        m_nTalkButtonStartIndex = 0;
		LoadNode ( nSTARTINDEX );
		return true;
	}

	return false;
}

void CNPCDialoguePage::LoadNode( DWORD dwID, bool bRandom /*= false*/, DWORD dwRandomTime /*= 0 */ )
{
    m_spDialogue = m_spDialogueSet.GetDialogue( dwID );
    //GASSERT( m_spDialogue && "m_DialogueSet.GetDialogue()" );
    if ( !m_spDialogue )
    {
        return;
    }

    DWORD dwIndex = m_spDialogue->FindConditionSatisfied( m_pCharLogic );
    if (dwIndex==UINT_MAX)
    {
        return;
    }

    CNpcDialogueCase* spCase = m_spDialogue->GetDlgCase( dwIndex );
    GASSERT( spCase && "m_pDialogueCase = NULL" );
    m_spDialogueCase = spCase;
	m_nCaseIdx = dwIndex;
    m_dwRandomTime = dwRandomTime;
    m_fRandomTime = 0.0f;

    //
    if ( m_dwRandomTime > 0 )
    {
        m_pRandomTimeButton->SetVisibleSingle( TRUE );
        m_pRandomTimeBack->SetVisibleSingle( TRUE );
        m_pRandomTimeText->SetVisibleSingle( TRUE );

        std::string strTime;
        strTime = sc::string::format( "%d", m_dwRandomTime );

        m_pRandomTimeText->SetText( strTime.c_str(), NS_UITEXTUTIL::WHITE );

    }
    else
    {
        m_pRandomTimeButton->SetVisibleSingle( FALSE );
        m_pRandomTimeBack->SetVisibleSingle( FALSE );
        m_pRandomTimeText->SetVisibleSingle( FALSE );
    }

    LoadBasicTalk(dwIndex);
    LoadButtonTalk( bRandom, m_nCaseIdx );

    // init talk button scroll bar.
    CBasicScrollThumbFrame* const pThumbFrame = m_pTalkButtonScrollBar->GetThumbFrame();
    pThumbFrame->SetState( m_nTalkButtonTotalLine, NUM_TALK_BUTTON );
    pThumbFrame->SetPercent( 0.0f );
}

void CNPCDialoguePage::LoadBasicTalk( const DWORD dwIndex )
{
    if ( !m_spDialogueCase )
        return;

	//std::tr1::shared_ptr<NpcTalkMan> spTalkMan = GLogicData::GetInstance().GetNpcTalkMan();
	std::string strBasicTalk;
	
	strBasicTalk = m_spDialogueCase->GetBasicTalk();

    m_pTalkText->SetText( strBasicTalk.c_str(), NS_UITEXTCOLOR::WHITE );

    int nTotal = m_pTalkText->GetTotalLine();
    int nViewPerPage = m_pTalkText->GetVisibleLine();
    CBasicScrollThumbFrame* const pThumbFrame = m_pTalkScrollBar->GetThumbFrame();
    pThumbFrame->SetState( nTotal, nViewPerPage );
    pThumbFrame->SetPercent( 0.0f );
}

void CNPCDialoguePage::LoadButtonTalk( bool bRandom /*= false */, const DWORD dwIndex/* = 0*/ )
{
    m_bButtonRandom = bRandom;

    for ( int i = 0; i < NUM_TALK_BUTTON; i++ )
        m_pTalkButtonText[ i ]->ClearText();

    static const int nKEYWORDSIZE = 10;

    static const std::string strKeyword[nKEYWORDSIZE] = 
    {
        "DIALOGUE_ICON_HEAL",
        "DIALOGUE_ICON_TRADE",
        "DIALOGUE_ICON_STARTPOINT",
        "DIALOGUE_ICON_STORAGE",
        "DIALOGUE_ICON_MARKET",
        "DIALOGUE_ICON_RESETPOINT",
        "DIALOGUE_ICON_BUS",
        "DIALOGUE_ICON_QUEST_START",
        "DIALOGUE_ICON_QUEST_ING",
        "DIALOGUE_ICON_TALK",
    };

    enum
    {
        ICON_HEAL,
        ICON_TRADE,
        ICON_STARTPOINT,
        ICON_STORAGE,
        ICON_MARKET,
        ICON_RESETPOINT,
        ICON_BUS,
        ICON_QUEST_START,
        ICON_QUEST_ING,
        ICON_QUEST_TALK,
    };

    //	버튼 텍스트 설정
    if ( !m_spDialogueCase )
        return;
    CNpcTalkControl* spNpcTalkControl = m_spDialogueCase->GetTalkControl();
    if ( spNpcTalkControl )
    {
        CNpcTalkControl::NPCMAP* pMap = spNpcTalkControl->GetTalkMap();
        CNpcTalkControl::NPCMAP_IT iter = pMap->begin();
        CNpcTalkControl::NPCMAP_IT iter_end = pMap->end();

		vector<SNpcTalk*> vecTalk;
		vecTalk.reserve( pMap->size() );

        for ( ; iter != iter_end; ++iter )
        {
			SNpcTalk* spNpcTalk = (*iter).second;
			if ( spNpcTalk )
				vecTalk.push_back( spNpcTalk );
        }

        if ( bRandom )
        {
            random_shuffle( vecTalk.begin(), vecTalk.end() );			
        }

		vector<SNpcTalk*>::iterator	pos = vecTalk.begin();
		vector<SNpcTalk*>::iterator	pos_end = vecTalk.end();

        std::multimap<int, int> mapTalkIndex;

        int nTalkIndex = 0;

        // Note : 대화파일에 상점 조건에 대한 정보도 있기 때문에 여기서 설정해준다.
        CInnerInterface::GetInstance().SetEnableDialogueWindowMarketPage(false);

        for ( ; pos != pos_end; ++pos, ++nTalkIndex )
        {
			SNpcTalk* spNpcTalk = (SNpcTalk*) (*pos);
			if ( !spNpcTalk->DoTEST(m_pCharLogic) )	continue;

            if (spNpcTalk->m_nACTION == SNpcTalk::EM_BASIC &&
                spNpcTalk->m_dwACTION_NO == SNpcTalk::EM_MARKET )
                CInnerInterface::GetInstance().SetEnableDialogueWindowMarketPage(true);

            switch ( spNpcTalk->m_nACTION )
            {
            case SNpcTalk::EM_QUEST_START:
                {
                    mapTalkIndex.insert( std::make_pair( TALK_QUEST_START, nTalkIndex ) );
                }
                break;

            case SNpcTalk::EM_QUEST_STEP:
                {
                    mapTalkIndex.insert( std::make_pair( TALK_QUEST_STEP, nTalkIndex ) );
                }
                break;

            case SNpcTalk::EM_BASIC:
                {
                    if ( spNpcTalk->m_dwACTION_NO != SNpcTalk::EM_MARKET )
                        mapTalkIndex.insert( std::make_pair( TALK_BASIC_TALK, nTalkIndex ) );
                }
                break;

            case SNpcTalk::EM_PAGE_MOVE:
                {
                    if ( spNpcTalk->m_dwACTION_NO == 1 )
                        mapTalkIndex.insert( std::make_pair( TALK_FIRST_TIME, nTalkIndex ) );
                    else
                        mapTalkIndex.insert( std::make_pair( TALK_BASIC_TALK, nTalkIndex ) );
                }
                break;

            default:
                {
                    mapTalkIndex.insert( std::make_pair( TALK_BASIC_TALK, nTalkIndex ) );
                }
                break;
            };
        }

        int talkIndexSize = static_cast<int>( mapTalkIndex.size() );
        m_nTalkButtonTotalLine = talkIndexSize + 1;

        int nButtonIndex = 0;
        int nCurIndex = 0;

        std::multimap<int, int>::iterator map_pos = mapTalkIndex.begin();
        std::multimap<int, int>::iterator map_pos_end = mapTalkIndex.end();

        for ( ; map_pos != map_pos_end; map_pos++ )
        {
            if ( nCurIndex < m_nTalkButtonStartIndex )
            {
                nCurIndex++;
                continue;
            }

            SNpcTalk* spNpcTalk = vecTalk[ map_pos->second ];
            
			std::string strAnswerTalk;		
			strAnswerTalk = spNpcTalk->GetTalk();

            int nImageType = 0;	
            D3DCOLOR dwColor = NS_UITEXTCOLOR::WHITE;

            switch ( map_pos->first )
            {
            case TALK_FIRST_TIME:
                {
                    nImageType = ICON_QUEST_TALK;
                }
                break;

            case TALK_QUEST_STEP:
                {
                    dwColor = NS_UITEXTCOLOR::YELLOW;
                    nImageType = ICON_QUEST_START;
                    strAnswerTalk = sc::string::format( "%s %s", ID2GAMEWORD( "DIALOGUE_TALK_BUTTON", 2 ), strAnswerTalk );
                }
                break;

            case TALK_QUEST_START:
                {
                    dwColor = NS_UITEXTCOLOR::FORESTGREEN;
                    nImageType = ICON_QUEST_START;
                    strAnswerTalk = sc::string::format( "%s %s", ID2GAMEWORD( "DIALOGUE_TALK_BUTTON", 3 ), strAnswerTalk );

                    //	NOTE : 비용이 있을 경우 추가
                    DWORD dwQuestID = spNpcTalk->m_dwQuestStartID[0];
                    GLQUEST* pQuest = GLQuestMan::GetInstance().Find( dwQuestID );
                    if ( !pQuest )
                    {
                        continue;
                    }

                    const DWORD dwBeginMoney = pQuest->m_dwBeginMoney;
                    if ( dwBeginMoney )
                    {
                        std::string strQuestCost;
                        strQuestCost = sc::string::format( " [%s:%d]", ID2GAMEWORD( "QUEST_BEGIN_MONEY" ), dwBeginMoney );
                        strAnswerTalk += strQuestCost;
                    }
                }
                break;

            case TALK_QUEST_ING:
                {
                    dwColor = NS_UITEXTCOLOR::DARKGRAY;
                    nImageType = ICON_QUEST_ING;
                    strAnswerTalk = sc::string::format( "%s %s", ID2GAMEWORD( "DIALOGUE_TALK_BUTTON", 4 ), strAnswerTalk );
                }
                break;

            case TALK_BASIC_TALK:
                {
                    switch ( spNpcTalk->m_dwACTION_NO )
                    {
                    case SNpcTalk::EM_CURE:
                        nImageType = ICON_HEAL;
                        break;

                    case SNpcTalk::EM_STARTPOINT:
                        nImageType = ICON_STARTPOINT;
                        break;

                    default:
                        nImageType = ICON_QUEST_TALK;
                        break;
                    }
                }
                break;
            };

            if ( nButtonIndex < NUM_TALK_BUTTON )
            {
                int nIndex = m_pTalkButtonText[ nButtonIndex ]->AddText( strAnswerTalk.c_str(), dwColor );
                m_pTalkButtonText[ nButtonIndex ]->SetTextData( nIndex, spNpcTalk->m_dwNID );
                m_pTalkButtonText[ nButtonIndex ]->SetTextImage( nIndex, strKeyword[nImageType].c_str() );
                nButtonIndex++;
            }
        }

        if ( nButtonIndex < NUM_TALK_BUTTON )
        {
            int nIndex = m_pTalkButtonText[ nButtonIndex ]->AddText( ID2GAMEWORD( "DIALOGUE_TALK_BUTTON", 1 ), NS_UITEXTCOLOR::WHITE );
            m_pTalkButtonText[ nButtonIndex ]->SetTextImage( nIndex, "DIALOGUE_ICON_TALK" );
        }
    }
}