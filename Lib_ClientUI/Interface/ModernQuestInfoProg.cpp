#include "StdAfx.h"
#include "ModernQuestInfoProg.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicScrollBarEx.h"
#include "BasicScrollThumbFrame.h"
#include "BasicTextButton.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "ItemSlot.h"

#include "GLQuestPlay.h"
#include "GLQuest.h"
#include "GLQuestMan.h"
#include "GLGaeaClient.h"
#include "InnerInterface.h"
#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CModernQuestInfoProg::CModernQuestInfoProg ()
	: m_dwQuestID(NATIVEID_NULL().dwID)
	, m_pTextTitle(NULL)
	, m_pTextDescTitle(NULL)
	, m_pTextDescInfo(NULL)
	, m_pScrollDesc(NULL)
	, m_pTextDetailTitle(NULL)
	, m_pTextDetailInfo(NULL)
	, m_pScrollDetail(NULL)
	, m_pTextStepInfo(NULL)
	, m_pScrollStep(NULL)
	, m_pTextQuestItemTitle(NULL)
	, m_pItemSlotQuestItem(NULL)
	, m_pTextRewardTitle(NULL)
	, m_pTextRewardInfo(NULL)
	, m_pScrollReward(NULL)
	, m_pItemSlotReward(NULL)
	, m_pButtonGiveUp(NULL)
	, m_pButtonComplete(NULL)
{
}

CModernQuestInfoProg::~CModernQuestInfoProg ()
{
}

void CModernQuestInfoProg::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_SYSTEM_MESSAGE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxSystemMessage ( "RAN_QUEST_WINDOW_MORE_PAGE_BACK_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_TITLE_BACK_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_EXPLAIN_TITLE_BACK_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_EXPLAIN_BACK_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_MORE_TEXT_BACK_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_MORE_TITLE_BACK_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_STEP_BACK_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_COLLECT_ITEM_TITLE_BACK_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_COLLECT_ITEM_SLOT_BACK_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_INDEMNIFY_BACK_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_INDEMNIFY_TITLE_BACK_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_INDEMNIFY_ITEM_SLOT_BACK_LUXES" );
	RegisterControl ( pLineBox );

	m_pTextTitle = new CBasicTextBox;
	m_pTextTitle->CreateSub ( this, "RAN_QUEST_WINDOW_QUEST_TITLE_LUXES" );
	m_pTextTitle->SetFont ( pFont9 );
	m_pTextTitle->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextTitle->AddText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextTitle );

	m_pTextDescTitle = new CBasicTextBox;
	m_pTextDescTitle->CreateSub ( this, "RAN_QUEST_WINDOW_EXPLAIN_TITLE_LUXES" );
	m_pTextDescTitle->SetFont ( pFont9 );
	m_pTextDescTitle->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextDescTitle->AddText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextDescTitle );

	m_pTextDescInfo = new CBasicTextBox;
	m_pTextDescInfo->CreateSub ( this, "RAN_QUEST_WINDOW_EXPLAIN_TEXT_LUXES" );
	m_pTextDescInfo->SetFont ( pFont9 );
	m_pTextDescInfo->SetTextAlign ( TEXT_ALIGN_BOTH_X );
	m_pTextDescInfo->AddText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextDescInfo );

	m_pScrollDesc = new CBasicScrollBarEx;
	m_pScrollDesc->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, MODERN_QUEST_WINDOW_PROG_INFO_DESC_SCROLL );
	m_pScrollDesc->CreateBaseScrollBar ( "RAN_QUEST_WINDOW_EXPLAIN_SCROLL_LUXES" );
	m_pScrollDesc->GetThumbFrame()->SetState ( 1, m_pTextDescInfo->GetVisibleLine() );
	m_pScrollDesc->GetThumbFrame()->SetUseParent( m_pTextDescInfo );
	RegisterControl ( m_pScrollDesc );


	m_pTextDetailTitle = new CBasicTextBox;
	m_pTextDetailTitle->CreateSub ( this, "RAN_QUEST_WINDOW_MORE_TITLE_LUXES" );
	m_pTextDetailTitle->SetFont ( pFont9 );
	m_pTextDetailTitle->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextDetailTitle->AddText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextDetailTitle );

	m_pTextDetailInfo = new CBasicTextBox;
	m_pTextDetailInfo->CreateSub ( this, "RAN_QUEST_WINDOW_MORE_TEXT_LUXES" );
	m_pTextDetailInfo->SetFont ( pFont9 );
	m_pTextDetailInfo->SetTextAlign ( TEXT_ALIGN_BOTH_X );
	m_pTextDetailInfo->AddText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextDetailInfo );

	m_pScrollDetail = new CBasicScrollBarEx;
	m_pScrollDetail->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, MODERN_QUEST_WINDOW_PROG_INFO_DETAIL_SCROLL );
	m_pScrollDetail->CreateBaseScrollBar ( "RAN_QUEST_WINDOW_MORE_SCROLL_LUXES" );
	m_pScrollDetail->GetThumbFrame()->SetState ( 1, m_pTextDetailInfo->GetVisibleLine() );
	m_pScrollDetail->GetThumbFrame()->SetUseParent( m_pTextDetailInfo );
	RegisterControl ( m_pScrollDetail );


	m_pTextStepInfo = new CBasicTextBox;
	m_pTextStepInfo->CreateSub ( this, "RAN_QUEST_WINDOW_STEP_TEXT_LUXES" );
	m_pTextStepInfo->SetFont ( pFont9 );
	m_pTextStepInfo->SetTextAlign ( TEXT_ALIGN_BOTH_X );
	m_pTextStepInfo->AddText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextStepInfo );

	m_pScrollStep = new CBasicScrollBarEx;
	m_pScrollStep->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, MODERN_QUEST_WINDOW_PROG_INFO_STEP_SCROLL );
	m_pScrollStep->CreateBaseScrollBar ( "RAN_QUEST_WINDOW_STEP_SCROLL_LUXES" );
	m_pScrollStep->GetThumbFrame()->SetState ( 1, m_pTextStepInfo->GetVisibleLine() );
	m_pScrollStep->GetThumbFrame()->SetUseParent( m_pTextStepInfo );
	RegisterControl ( m_pScrollStep );


	m_pTextQuestItemTitle = new CBasicTextBox;
	m_pTextQuestItemTitle->CreateSub ( this, "RAN_QUEST_WINDOW_COLLECT_ITEM_TITLE_LUXES" );
	m_pTextQuestItemTitle->SetFont ( pFont9 );
	m_pTextQuestItemTitle->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextQuestItemTitle->AddText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextQuestItemTitle );

	m_pItemSlotQuestItem = new CItemSlot;
	m_pItemSlotQuestItem->CreateSub ( this, "RAN_QUEST_WINDOW_COLLECT_ITEM_SLOT_LUXES", UI_FLAG_DEFAULT, MODERN_QUEST_WINDOW_PROG_INFO_QUEST_ITEM_SLOT );
	m_pItemSlotQuestItem->CreateSubControl ( GLQUESTPROG::INVEN_SIZEX, TRUE );
	RegisterControl ( m_pItemSlotQuestItem );


	m_pTextRewardTitle = new CBasicTextBox;
	m_pTextRewardTitle->CreateSub ( this, "RAN_QUEST_WINDOW_INDEMNIFY_TITLE_LUXES" );
	m_pTextRewardTitle->SetFont ( pFont9 );
	m_pTextRewardTitle->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextRewardTitle->AddText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextRewardTitle );

	m_pTextRewardInfo = new CBasicTextBox;
	m_pTextRewardInfo->CreateSub ( this, "RAN_QUEST_WINDOW_INDEMNIFY_TEXT_LUXES" );
	m_pTextRewardInfo->SetFont ( pFont9 );
	m_pTextRewardInfo->SetTextAlign ( TEXT_ALIGN_BOTH_X );
	m_pTextRewardInfo->AddText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextRewardInfo );

	m_pScrollReward = new CBasicScrollBarEx;
	m_pScrollReward->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, MODERN_QUEST_WINDOW_PROG_INFO_REWARD_SCROLL );
	m_pScrollReward->CreateBaseScrollBar ( "RAN_QUEST_WINDOW_INDEMNIFY_TEXT_SCROLLBAR_LUXES" );
	m_pScrollReward->GetThumbFrame()->SetState ( 1, m_pTextRewardInfo->GetVisibleLine() );
	m_pScrollReward->GetThumbFrame()->SetUseParent( m_pTextRewardInfo );
	RegisterControl ( m_pScrollReward );

	m_pItemSlotReward = new CItemSlot;
	m_pItemSlotReward->CreateSub ( this, "RAN_QUEST_WINDOW_INDEMNIFY_ITEM_SLOT_LUXES", UI_FLAG_DEFAULT, MODERN_QUEST_WINDOW_PROG_INFO_REWARD_ITEM_SLOT );
	m_pItemSlotReward->CreateSubControl ( GLQUESTPROG::INVEN_SIZEX, TRUE );
	RegisterControl ( m_pItemSlotReward );

	m_pButtonGiveUp = new CBasicTextButton;
	m_pButtonGiveUp->CreateSub ( this, "BASIC_TEXT_BUTTON19", UI_FLAG_XSIZE, MODERN_QUEST_WINDOW_PROG_BUTTON_GIVEUP );
	m_pButtonGiveUp->CreateBaseButton ( "RAN_QUEST_WINDOW_COMPLETE_BUTTON_LUXES", CBasicTextButton::SIZE19, CBasicButton::CLICK_FLIP, ID2GAMEWORD("QUEST_MAIN_BUTTON",0) );
	m_pButtonGiveUp->SetVisibleSingle( FALSE );
	RegisterControl ( m_pButtonGiveUp );

	m_pButtonComplete = new CBasicTextButton;
	m_pButtonComplete->CreateSub ( this, "BASIC_TEXT_BUTTON19", UI_FLAG_XSIZE, MODERN_QUEST_WINDOW_PROG_BUTTON_COMPLETE );
	m_pButtonComplete->CreateBaseButton ( "RAN_QUEST_WINDOW_GIVE_UP_BUTTON_LUXES", CBasicTextButton::SIZE19, CBasicButton::CLICK_FLIP, ID2GAMEWORD("QUEST_MAIN_BUTTON",3) );
	m_pButtonComplete->SetVisibleSingle( FALSE );
	RegisterControl ( m_pButtonComplete );

}

void CModernQuestInfoProg::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( m_pScrollDesc && m_pTextDescInfo )
	{
		CBasicScrollThumbFrame* const pThumbFrame = m_pScrollDesc->GetThumbFrame ();
		const int nTotalLine = m_pTextDescInfo->GetTotalLine ();
		const int nLinePerOneView = m_pTextDescInfo->GetVisibleLine ();

		pThumbFrame->SetState ( nTotalLine, nLinePerOneView );
		if ( nLinePerOneView < nTotalLine )
		{
			const int nMovableLine = nTotalLine - nLinePerOneView;
			float fPercent = pThumbFrame->GetPercent ();
			int nPos = (int)floor(fPercent * nMovableLine);
			m_pTextDescInfo->SetCurLine ( nPos );			
		}
	}

	if( m_pScrollDetail && m_pTextDetailInfo )
	{
		CBasicScrollThumbFrame* const pThumbFrame = m_pScrollDetail->GetThumbFrame ();
		const int nTotalLine = m_pTextDetailInfo->GetTotalLine ();
		const int nLinePerOneView = m_pTextDetailInfo->GetVisibleLine ();

		pThumbFrame->SetState ( nTotalLine, nLinePerOneView );
		if ( nLinePerOneView < nTotalLine )
		{
			const int nMovableLine = nTotalLine - nLinePerOneView;
			float fPercent = pThumbFrame->GetPercent ();
			int nPos = (int)floor(fPercent * nMovableLine);
			m_pTextDetailInfo->SetCurLine ( nPos );			
		}
	}

	if ( m_pScrollStep && m_pTextStepInfo )
	{
		CBasicScrollThumbFrame* const pThumbFrame = m_pScrollStep->GetThumbFrame ();
		const int nTotalLine = m_pTextStepInfo->GetTotalLine ();
		const int nLinePerOneView = m_pTextStepInfo->GetVisibleLine ();

		pThumbFrame->SetState ( nTotalLine, nLinePerOneView );
		if ( nLinePerOneView < nTotalLine )
		{
			const int nMovableLine = nTotalLine - nLinePerOneView;
			float fPercent = pThumbFrame->GetPercent ();
			int nPos = (int)floor(fPercent * nMovableLine);
			m_pTextStepInfo->SetCurLine ( nPos );			
		}
	}

	if ( m_pScrollReward && m_pTextRewardInfo )
	{
		CBasicScrollThumbFrame* const pThumbFrame = m_pScrollReward->GetThumbFrame ();
		const int nTotalLine = m_pTextRewardInfo->GetTotalLine ();
		const int nLinePerOneView = m_pTextRewardInfo->GetVisibleLine ();

		pThumbFrame->SetState ( nTotalLine, nLinePerOneView );
		if ( nLinePerOneView < nTotalLine )
		{
			const int nMovableLine = nTotalLine - nLinePerOneView;
			float fPercent = pThumbFrame->GetPercent ();
			int nPos = (int)floor(fPercent * nMovableLine);
			m_pTextRewardInfo->SetCurLine ( nPos );			
		}
	}

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CModernQuestInfoProg::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case MODERN_QUEST_WINDOW_PROG_BUTTON_GIVEUP:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( UIMSG_LB_UP & dwMsg )
				{
					AddMessageEx ( UIMSG_INFO_PROG_BUTTON_GIVEUP );
				}
			}
		}break;

	case MODERN_QUEST_WINDOW_PROG_BUTTON_COMPLETE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( UIMSG_LB_UP & dwMsg )
				{
					AddMessageEx ( UIMSG_INFO_PROG_BUTTON_COMPLETE );
				}
			}
		}break;

	case MODERN_QUEST_WINDOW_PROG_INFO_QUEST_ITEM_SLOT:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( m_pItemSlotQuestItem )
				{
					int nPosX = m_pItemSlotQuestItem->GetItemIndex ();
					if ( nPosX < 0 ) return ;
					
					SINVENITEM sInvenItem = m_pItemSlotQuestItem->GetItemImage ( nPosX );		
					if ( sInvenItem.sItemCustom.sNativeID != NATIVEID_NULL () )
					{
						CInnerInterface::GetInstance().SHOW_ITEM_INFO_SIMPLE ( sInvenItem.sItemCustom );
					}
				}
			}
		}break;

	case MODERN_QUEST_WINDOW_PROG_INFO_REWARD_ITEM_SLOT:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( m_pItemSlotReward )
				{
					int nPosX = m_pItemSlotReward->GetItemIndex ();
					if ( nPosX < 0 ) return ;

					SINVENITEM sInvenItem = m_pItemSlotReward->GetItemImage ( nPosX );		
					if ( sInvenItem.sItemCustom.sNativeID != NATIVEID_NULL () )
					{
						CInnerInterface::GetInstance().SHOW_ITEM_INFO_SIMPLE ( sInvenItem.sItemCustom );

						/*item preview, Juver, 2017/07/27 */
						if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
						{
							if ( dwMsg & UIMSG_LB_UP )
							{
								CInnerInterface::GetInstance().PreviewItem( sInvenItem.sItemCustom );
								return;
							}
						}

						/*box contents, Juver, 2017/08/30 */
						if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
						{
							if ( dwMsg & UIMSG_RB_UP )
							{
								CInnerInterface::GetInstance().ShowBoxContents( sInvenItem.sItemCustom.sNativeID );
								return;
							}
						}
					}
				}
			}
		}break;
	}
}


void CModernQuestInfoProg::SetVisibleSingle( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		DataReset();
	}
}

void CModernQuestInfoProg::DataReset()
{
	m_dwQuestID = NATIVEID_NULL().dwID;

	if ( m_pTextTitle )				
		m_pTextTitle->ClearText();

	if ( m_pTextDescTitle )			
		m_pTextDescTitle->ClearText();

	if ( m_pTextDescInfo )			
		m_pTextDescInfo->ClearText();

	if ( m_pTextDetailTitle )		
		m_pTextDetailTitle->ClearText();

	if ( m_pTextDetailInfo )		
		m_pTextDetailInfo->ClearText();

	if ( m_pTextStepInfo )			
		m_pTextStepInfo->ClearText();

	if ( m_pTextQuestItemTitle )	
		m_pTextQuestItemTitle->ClearText();

	if ( m_pTextRewardTitle )		
		m_pTextRewardTitle->ClearText();

	if ( m_pTextRewardInfo )		
		m_pTextRewardInfo->ClearText();

	for( int i=0; i<CItemSlot::nLIMIT_COLUMN; ++i )
	{
		m_pItemSlotQuestItem->ResetItemImage(i);
		m_pItemSlotReward->ResetItemImage(i);
	}

	if ( m_pButtonComplete )	m_pButtonComplete->SetVisibleSingle ( FALSE );
	if ( m_pButtonGiveUp )		m_pButtonGiveUp->SetVisibleSingle ( FALSE );
}

void CModernQuestInfoProg::DataSet( DWORD dwQuestID )
{
	DataReset();

	if ( dwQuestID == NATIVEID_NULL().dwID ) return;
	m_dwQuestID = dwQuestID;

	GLQuestPlay& cQuestPlay = GLGaeaClient::GetInstance().GetCharacter ()->m_cQuestPlay;
	GLQUESTPROG* pQuestProg = cQuestPlay.FindProc ( dwQuestID );
	if ( !pQuestProg ) return;

	GLQUEST* pQuest = GLQuestMan::GetInstance().Find ( dwQuestID );
	if ( !pQuest ) return;

	const DWORD dwSTEP = pQuestProg->m_dwSTEP;
	GLQUEST_STEP* pQuestStep = pQuest->GetSTEP ( dwSTEP );	
	if ( !pQuestStep )	return;

	GLQUESTPROG_STEP& sSTEP_PROG = pQuestProg->m_sSTEP_PROG;	


	if ( m_pTextTitle )
	{
		int nPosition;
		if ( RANPARAM::strLangPlacer != "en" )
		{
			//xml translation
			CString strTemp;				
			strTemp.Format ( "QN_%03d_%03d", pQuestProg->m_sNID.wMainID, pQuestProg->m_sNID.wSubID );
			nPosition = m_pTextTitle->AddText( ID2GAMEWORDQUEST( strTemp ), NS_UITEXTCOLOR::WHITE );
		}
		else
		{
			nPosition = m_pTextTitle->AddText( pQuest->m_strTITLE.c_str(),NS_UITEXTCOLOR::WHITE );
		}
		m_pTextTitle->SetTextImage ( nPosition, "RAN_QUEST_WINDOW_STATUS_ING_ICON_LUXES" );
	}

	if ( m_pTextDescTitle )		m_pTextDescTitle->SetText( ID2GAMEWORD("RAN_QUEST_LIST_EXPLAIN", 0 ), NS_UITEXTCOLOR::ORANGE );

	if ( m_pTextDescInfo )
	{

		if ( RANPARAM::strLangPlacer != "en" )
		{
			//xml translation
			CString strTemp;
			strTemp.Format ( "QD_%03d_%03d", pQuestProg->m_sNID.wMainID, pQuestProg->m_sNID.wSubID );
			m_pTextDescInfo->SetText( ID2GAMEWORDQUEST (strTemp), NS_UITEXTCOLOR::WHITE );
		}
		else
		{
			m_pTextDescInfo->SetText( pQuest->m_strCOMMENT.c_str(), NS_UITEXTCOLOR::WHITE );
		}

		m_pTextDescInfo->AddText( "" );

		CString strCombine;		
		CString strConditionLine1, strConditionLine2;

		if ( pQuest->m_dwLimitTime )
		{
			const DWORD dwLimitTime = pQuest->m_dwLimitTime;
			const DWORD dwLimitHour = dwLimitTime / 60;
			const DWORD dwLimitMinute = dwLimitTime % 60;

			const float fLapsTime = pQuestProg->m_fLAPSTIME;
			const DWORD dwLapsTime = ((DWORD)fLapsTime) / 60;	
			const DWORD dwLapsHour = dwLapsTime / 60;
			const DWORD dwLapsMinute = dwLapsTime % 60;

			CString strLimitTime;
			if ( dwLimitHour )
			{
				strLimitTime.Format ( "%d%s %d%s", dwLimitHour, ID2GAMEWORD("QUEST_CONDITION_TIME_UNIT",0), dwLimitMinute, ID2GAMEWORD("QUEST_CONDITION_TIME_UNIT",1) );
			}
			else
			{
				strLimitTime.Format ( "%d%s", dwLimitMinute, ID2GAMEWORD("QUEST_CONDITION_TIME_UNIT",1) );
			}

			CString strLapsTime;
			if ( dwLapsHour )
			{
				strLapsTime.Format ( "%d%s %d%s", dwLapsHour, ID2GAMEWORD("QUEST_CONDITION_TIME_UNIT",0), dwLapsMinute, ID2GAMEWORD("QUEST_CONDITION_TIME_UNIT",1) );
			}
			else
			{
				strLapsTime.Format ( "%d%s", dwLapsMinute, ID2GAMEWORD("QUEST_CONDITION_TIME_UNIT",1) );
			}

			strConditionLine1.Format ( "%s : [%s/%s]", ID2GAMEWORD("QUEST_MAIN_CONDITION",0), strLapsTime, strLimitTime );			
		}

		if ( pQuest->m_dwLimitPartyMemNum )
		{
			if ( strConditionLine1.GetLength () )	
				strCombine.Format ( ", %s : %d", ID2GAMEWORD("QUEST_MAIN_CONDITION",1), pQuest->m_dwLimitPartyMemNum );
			else
				strCombine.Format ( "%s : %d", ID2GAMEWORD("QUEST_MAIN_CONDITION",1), pQuest->m_dwLimitPartyMemNum );
			strConditionLine1 += strCombine;			
		}

		if ( strConditionLine1.GetLength () )
		{
			strCombine.Format ( "%s - %s", ID2GAMEWORD("QUEST_CONDITION"), strConditionLine1 );
			m_pTextDescInfo->AddText ( strCombine, NS_UITEXTCOLOR::ORANGE );
		}

		if ( pQuest->m_bNonDie )
		{
			strConditionLine2 = ID2GAMEWORD("QUEST_MAIN_CONDITION",2);
			if ( strConditionLine1.GetLength () )
			{
				std::string strCondition = ID2GAMEWORD("QUEST_CONDITION");
				std::string strBlank ( strCondition.size(), ' ' );

				strCombine.Format ( "%s - %s", strBlank.c_str(), strConditionLine2 );				
			}
			else
			{
				strCombine.Format ( "%s - %s", ID2GAMEWORD("QUEST_CONDITION"), strConditionLine2 );				
			}

			m_pTextDescInfo->AddText ( strCombine, NS_UITEXTCOLOR::ORANGE );
		}
	}

	if ( m_pScrollDesc )		m_pScrollDesc->GetThumbFrame()->SetPercent ( 0.0f );

	if ( m_pTextDetailTitle )	m_pTextDetailTitle->SetText( ID2GAMEWORD("RAN_QUEST_LIST_MORE", 0 ), NS_UITEXTCOLOR::ORANGE );
	if ( m_pTextDetailInfo)		
	{
		if ( RANPARAM::strLangPlacer != "en" )
		{
			//xml translation
			CString strTemp;
			strTemp.Format ( "QD_%03d_%03d_D_%d", pQuestProg->m_sNID.wMainID, pQuestProg->m_sNID.wSubID, dwSTEP );
			m_pTextDetailInfo->SetText (ID2GAMEWORDQUEST( strTemp ), NS_UITEXTCOLOR::WHITE );
			//m_pTextDetailInfo->SetText ( pQuestStep->GetCOMMENT(), NS_UITEXTCOLOR::WHITE );
		}
		else
		{
			m_pTextDetailInfo->SetText ( pQuestStep->GetCOMMENT(), NS_UITEXTCOLOR::WHITE );
		}
	}

	if ( m_pScrollDetail )		m_pScrollDetail->GetThumbFrame()->SetPercent ( 0.0f );

	if ( m_pTextStepInfo )
	{
		enum
		{
			QUEST_CHECK = 0,
			QUEST_CHECK_F,
			QUEST_CHECK_DUMMY,
		};

		static	CString strCheckKeyword[] =
		{
			"RAN_QUEST_WINDOW_STEP_BUTTON_LUXES",
			"RAN_QUEST_WINDOW_STEP_BUTTON_F_LUXES",
			"RAN_QUEST_WINDOW_STEP_BUTTON_DUMMY_LUXES",
		};

		GLQUEST::VECQSTEP& vecProgStep = pQuest->m_vecProgStep;
		for ( int i = 0; i < (int)vecProgStep.size(); ++i )
		{
			const GLQUEST_STEP& sQuestStep = vecProgStep[i];
			bool	bShowStep = false;
			D3DCOLOR dwColor = NS_UITEXTCOLOR::WHITE;
			int nImageType = QUEST_CHECK;
			if ( i < (int)dwSTEP )
			{
				dwColor = NS_UITEXTCOLOR::DARKGRAY;
				nImageType = QUEST_CHECK_F;
			}
			else if ( i == (int)dwSTEP )
			{
				dwColor = NS_UITEXTCOLOR::GOLD;
				bShowStep = true;
			}
			int nIndex;
			if ( RANPARAM::strLangPlacer != "en" )
			{
				//xml translation
				CString strTemp;
				strTemp.Format ( "QN_%03d_%03d_P_%d", pQuestProg->m_sNID.wMainID, pQuestProg->m_sNID.wSubID, dwSTEP );
				nIndex = m_pTextStepInfo->AddText ( ID2GAMEWORDQUEST( strTemp ), dwColor );
			}
			else
			{
				nIndex = m_pTextStepInfo->AddText ( sQuestStep.GetTITLE (), dwColor );
			}

			m_pTextStepInfo->SetTextImage ( nIndex, strCheckKeyword[nImageType] );

			if ( bShowStep )
			{
				dwColor = NS_UITEXTCOLOR::WHITE;
				nImageType = QUEST_CHECK_DUMMY;
				CString strStepText;

				if ( sQuestStep.IsNEED_NPCTALK() )	
				{
					strStepText.Format("%s%s",ID2GAMEWORD("RAN_QUEST_LIST_STEP_PLUS",0), pQuestStep->m_strOBJ_NPCTALK.c_str () );
					nIndex = m_pTextStepInfo->AddText ( strStepText, dwColor );
					m_pTextStepInfo->SetTextImage ( nIndex, strCheckKeyword[nImageType] );
				}

				if ( sQuestStep.IsNEED_QITEM() )
				{
					strStepText.Format("%s%s",ID2GAMEWORD("RAN_QUEST_LIST_STEP_PLUS",0), pQuestStep->m_strOBJ_MOBGEN_QITEM.c_str () );
					nIndex = m_pTextStepInfo->AddText ( strStepText, dwColor );
					m_pTextStepInfo->SetTextImage ( nIndex, strCheckKeyword[nImageType] );
				}

				if ( sQuestStep.IsNEED_NPCGUARD() )	
				{
					strStepText.Format("%s%s",ID2GAMEWORD("RAN_QUEST_LIST_STEP_PLUS",0), pQuestStep->m_strOBJ_NPCGUARD.c_str () );
					nIndex = m_pTextStepInfo->AddText ( strStepText, dwColor );
					m_pTextStepInfo->SetTextImage ( nIndex, strCheckKeyword[nImageType] );
				}

				if ( sQuestStep.IsNEED_REACHZONE() )	
				{
					strStepText.Format("%s%s",ID2GAMEWORD("RAN_QUEST_LIST_STEP_PLUS",0), pQuestStep->m_strOBJ_REACH_ZONE.c_str () );
					nIndex = m_pTextStepInfo->AddText ( strStepText, dwColor );
					m_pTextStepInfo->SetTextImage ( nIndex, strCheckKeyword[nImageType] );
				}

				if ( sQuestStep.IsNEED_DEFENSEZONE() )	
				{
					strStepText.Format("%s%s",ID2GAMEWORD("RAN_QUEST_LIST_STEP_PLUS",0), pQuestStep->m_strOBJ_DEFENSE_ZONE.c_str () );
					nIndex = m_pTextStepInfo->AddText ( strStepText, dwColor );
					m_pTextStepInfo->SetTextImage ( nIndex, strCheckKeyword[nImageType] );
				}
	
				if ( sQuestStep.IsNEED_MOBKILL() )	
				{
					strStepText.Format ( "%s%s [%d/%d]",ID2GAMEWORD("RAN_QUEST_LIST_STEP_PLUS",0),pQuestStep->m_strOBJ_MOBKILL.c_str (), sSTEP_PROG.m_dwNUM_MOBKILL, pQuestStep->m_dwNUM_MOBKILL );
					dwColor = NS_UITEXTCOLOR::GOLD;	
					nIndex = m_pTextStepInfo->AddText ( strStepText, dwColor );
					m_pTextStepInfo->SetTextImage ( nIndex, strCheckKeyword[nImageType] );
				}
			}
		}

		if ( m_pScrollStep )
		{
			CBasicScrollThumbFrame* const pThumbFrame = m_pScrollStep->GetThumbFrame ();

			const int nTotalLine = m_pTextStepInfo->GetTotalLine ();
			const int nLinePerOneView = m_pTextStepInfo->GetVisibleLine ();		
			pThumbFrame->SetState ( nTotalLine, nLinePerOneView );
			m_pTextStepInfo->SetCurLine ( 0 );
			pThumbFrame->SetPercent ( 0.0f );
			const int nCount = m_pTextStepInfo->GetCount () - 1;
			if ( nLinePerOneView < nTotalLine )
			{			
				float fPercent = (float)dwSTEP / (float)nCount;
				pThumbFrame->SetPercent ( fPercent );
			}
		}
	}

	if ( m_pTextQuestItemTitle )	m_pTextQuestItemTitle->SetText( ID2GAMEWORD("QUEST_MAIN_ITEM_TITLE",0),NS_UITEXTCOLOR::ORANGE );

	if ( m_pItemSlotQuestItem )
	{
		GLInventory::CELL_MAP *ItemList = pQuestProg->m_sINVENTORY.GetItemList ();
		if ( ItemList->size () > (GLQUESTPROG::INVEN_SIZEX * GLQUESTPROG::INVEN_SIZEY) )	return;

		for ( GLInventory::CELL_MAP_ITER iter = ItemList->begin(); iter!=ItemList->end(); ++iter )
		{
			SINVENITEM* pInvenItem = (*iter).second;
			SITEM* pItemData = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
			if ( !pItemData )	continue;

			m_pItemSlotQuestItem->SetItemImage ( pInvenItem->wPosX, *pInvenItem );
		}
	}


	if ( m_pTextRewardTitle )		m_pTextRewardTitle->SetText( ID2GAMEWORD("QUEST_MAIN_BUTTON",4),NS_UITEXTCOLOR::ORANGE );

	if ( m_pTextRewardInfo )
	{
		bool bHaveReward = true;
		if ( !(pQuest->m_llGiftEXP || pQuest->m_dwGiftMONEY || pQuest->m_dwGiftELEMENT || pQuest->m_dwGiftLIFEPOINT ||
			pQuest->m_dwGiftSKILLPOINT || pQuest->m_dwGiftSTATSPOINT || pQuest->m_vecGiftITEM.size () || pQuest->m_vecGiftSKILL.size ()) )
		{
			bHaveReward = false;
		}

		if ( bHaveReward )
		{
			m_pTextRewardInfo->AddText ( ID2GAMEWORD("QUEST_GIFT",9), NS_UITEXTCOLOR::GREENYELLOW );

			CString strCombine;


			if ( pQuest->m_llGiftEXP )
			{
				/*quest rate, Juver, 2017/09/28 */
				LONGLONG llEXP = LONGLONG ( GLCONST_CHAR::fQuestExpRate * pQuest->m_llGiftEXP );
				strCombine.Format ( "%s : %I64d", ID2GAMEWORD("QUEST_GIFT",0), llEXP );
				m_pTextRewardInfo->AddText ( strCombine, NS_UITEXTCOLOR::WHITE );
			}

			if ( pQuest->m_dwGiftMONEY )
			{
				/*quest rate, Juver, 2017/09/28 */
				DWORD dwMONEY = DWORD ( GLCONST_CHAR::fQuestMoneyRate * pQuest->m_dwGiftMONEY );
				strCombine.Format ( "%s : %d", ID2GAMEWORD("QUEST_GIFT",1), dwMONEY );
				m_pTextRewardInfo->AddText ( strCombine, NS_UITEXTCOLOR::WHITE );
			}

			/*if ( pQuest->m_llGiftEXP )
			{
				strCombine.Format ( "%s : %I64d", ID2GAMEWORD("QUEST_GIFT",0), pQuest->m_llGiftEXP );
				m_pTextRewardInfo->AddText ( strCombine, NS_UITEXTCOLOR::WHITE );
			}

			if ( pQuest->m_dwGiftMONEY )
			{
				strCombine.Format ( "%s : %d", ID2GAMEWORD("QUEST_GIFT",1), pQuest->m_dwGiftMONEY );
				m_pTextRewardInfo->AddText ( strCombine, NS_UITEXTCOLOR::WHITE );
			}*/

			if ( pQuest->m_dwGiftELEMENT )
			{
				strCombine.Format ( "%s : %d", ID2GAMEWORD("QUEST_GIFT",2), pQuest->m_dwGiftELEMENT );
				m_pTextRewardInfo->AddText ( strCombine, NS_UITEXTCOLOR::WHITE );
			}

			if ( pQuest->m_dwGiftLIFEPOINT )
			{
				strCombine.Format ( "%s : %d", ID2GAMEWORD("QUEST_GIFT",3), pQuest->m_dwGiftLIFEPOINT );
				m_pTextRewardInfo->AddText ( strCombine, NS_UITEXTCOLOR::WHITE );
			}

			//if ( pQuest->m_dwGiftDisPK )
			//{
			//	strCombine.Format ( "%s : %d", ID2GAMEWORD("QUEST_GIFT",4), pQuest->m_dwGiftDisPK );
			//	m_pTextRewardInfo->AddText ( strCombine, NS_UITEXTCOLOR::WHITE );
			//}

			if ( pQuest->m_dwGiftSKILLPOINT )
			{
				strCombine.Format ( "%s : %d", ID2GAMEWORD("QUEST_GIFT",5), pQuest->m_dwGiftSKILLPOINT );
				m_pTextRewardInfo->AddText ( strCombine, NS_UITEXTCOLOR::WHITE );
			}

			if ( pQuest->m_dwGiftSTATSPOINT )
			{
				strCombine.Format ( "%s : %d", ID2GAMEWORD("QUEST_GIFT",6), pQuest->m_dwGiftSTATSPOINT );
				m_pTextRewardInfo->AddText ( strCombine, NS_UITEXTCOLOR::WHITE );
			}

			if ( pQuest->m_vecGiftITEM.size() )
			{
				for( int i=0; i<(int)pQuest->m_vecGiftITEM.size(); ++i )
				{
					SINVENITEM sinven_item;
					sinven_item.sItemCustom = pQuest->m_vecGiftITEM[i];

					SITEM* pItemData = GLItemMan::GetInstance().GetItem ( sinven_item.sItemCustom.sNativeID );
					if ( !pItemData )	continue;

					if ( i >= GLQUESTPROG::INVEN_SIZEX )	continue;
					if ( m_pItemSlotReward )	m_pItemSlotReward->SetItemImage ( i, sinven_item );
				}
			}

			if ( pQuest->m_vecGiftSKILL.size() )
			{
				CString strSkill = ID2GAMEWORD("QUEST_GIFT",8);
				strSkill += " : ";
				DWQARRAY_ITER iter = pQuest->m_vecGiftSKILL.begin ();
				DWQARRAY_ITER iter_end = pQuest->m_vecGiftSKILL.end ();
				for ( ; iter != iter_end; )
				{
					DWORD dwID = (*iter);									
					PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData ( SNATIVEID(dwID) );
					if ( !pSKILL )
					{
						++iter;
						if ( iter == iter_end ) break;
						continue;
					}
					strSkill += pSKILL->GetName();

					++iter;
					if ( iter == iter_end ) break;
					strSkill += ", ";
				}
				m_pTextRewardInfo->AddText ( strSkill, NS_UITEXTCOLOR::WHITE );
			}	
		}
	}

	if ( pQuestProg->CheckCOMPLETE() )
	{		
		if ( m_pButtonComplete )	
			m_pButtonComplete->SetVisibleSingle ( TRUE );

		return;
	}	
	else if ( pQuest->IsGIVEUP() )
	{
		if ( m_pButtonGiveUp )
			m_pButtonGiveUp->SetVisibleSingle ( TRUE );

		return;
	}

}