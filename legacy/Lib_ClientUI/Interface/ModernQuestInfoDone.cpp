#include "StdAfx.h"
#include "ModernQuestInfoDone.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicScrollBarEx.h"
#include "BasicScrollThumbFrame.h"

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


CModernQuestInfoDone::CModernQuestInfoDone ()
	: m_dwQuestID(NATIVEID_NULL().dwID)
	, m_pTextTitle(NULL)
	, m_pTextResultTitle(NULL)
	, m_pTextResultInfo(NULL)
	, m_pTextDescTitle(NULL)
	, m_pTextDescInfo(NULL)
	, m_pScrollDesc(NULL)
	, m_pTextDetailTitle(NULL)
	, m_pTextDetailInfo(NULL)
	, m_pScrollDetail(NULL)
	, m_pTextStepInfo(NULL)
	, m_pScrollStep(NULL)
	, m_pTextRewardTitle(NULL)
	, m_pTextRewardInfo(NULL)
	, m_pScrollReward(NULL)
	, m_pItemSlotReward(NULL)
{
}

CModernQuestInfoDone::~CModernQuestInfoDone ()
{
}

void CModernQuestInfoDone::CreateSubControl ()
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
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_RESULT_BACK_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_RESULT_TITLE_BACK_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_EXPLAIN_TITLE_BACK_END_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_EXPLAIN_BACK_END_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_MORE_TEXT_BACK_END_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_MORE_TITLE_BACK_END_LUXES" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_STEP_BACK_END_LUXES" );
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


	m_pTextResultTitle = new CBasicTextBox;
	m_pTextResultTitle->CreateSub ( this, "RAN_QUEST_WINDOW_RESULT_TITLE_LUXES" );
	m_pTextResultTitle->SetFont ( pFont9 );
	m_pTextResultTitle->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextResultTitle->AddText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextResultTitle );

	m_pTextResultInfo = new CBasicTextBox;
	m_pTextResultInfo->CreateSub ( this, "RAN_QUEST_WINDOW_RESULT_TEXT_LUXES" );
	m_pTextResultInfo->SetFont ( pFont9 );
	m_pTextResultInfo->SetTextAlign ( TEXT_ALIGN_BOTH_X );
	m_pTextResultInfo->AddText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextResultInfo );


	m_pTextDescTitle = new CBasicTextBox;
	m_pTextDescTitle->CreateSub ( this, "RAN_QUEST_WINDOW_EXPLAIN_TITLE_END_LUXES" );
	m_pTextDescTitle->SetFont ( pFont9 );
	m_pTextDescTitle->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextDescTitle->AddText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextDescTitle );

	m_pTextDescInfo = new CBasicTextBox;
	m_pTextDescInfo->CreateSub ( this, "RAN_QUEST_WINDOW_EXPLAIN_TEXT_END_LUXES" );
	m_pTextDescInfo->SetFont ( pFont9 );
	m_pTextDescInfo->SetTextAlign ( TEXT_ALIGN_BOTH_X );
	m_pTextDescInfo->AddText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextDescInfo );

	m_pScrollDesc = new CBasicScrollBarEx;
	m_pScrollDesc->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, MODERN_QUEST_WINDOW_DONE_INFO_DESC_SCROLL );
	m_pScrollDesc->CreateBaseScrollBar ( "RAN_QUEST_WINDOW_EXPLAIN_SCROLL_END_LUXES" );
	m_pScrollDesc->GetThumbFrame()->SetState ( 1, m_pTextDescInfo->GetVisibleLine() );
	m_pScrollDesc->GetThumbFrame()->SetUseParent( m_pTextDescInfo );
	RegisterControl ( m_pScrollDesc );


	m_pTextDetailTitle = new CBasicTextBox;
	m_pTextDetailTitle->CreateSub ( this, "RAN_QUEST_WINDOW_MORE_TITLE_END_LUXES" );
	m_pTextDetailTitle->SetFont ( pFont9 );
	m_pTextDetailTitle->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextDetailTitle->AddText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextDetailTitle );

	m_pTextDetailInfo = new CBasicTextBox;
	m_pTextDetailInfo->CreateSub ( this, "RAN_QUEST_WINDOW_MORE_TEXT_END_LUXES" );
	m_pTextDetailInfo->SetFont ( pFont9 );
	m_pTextDetailInfo->SetTextAlign ( TEXT_ALIGN_BOTH_X );
	m_pTextDetailInfo->AddText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextDetailInfo );

	m_pScrollDetail = new CBasicScrollBarEx;
	m_pScrollDetail->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, MODERN_QUEST_WINDOW_DONE_INFO_DETAIL_SCROLL );
	m_pScrollDetail->CreateBaseScrollBar ( "RAN_QUEST_WINDOW_MORE_SCROLL_END_LUXES" );
	m_pScrollDetail->GetThumbFrame()->SetState ( 1, m_pTextDetailInfo->GetVisibleLine() );
	m_pScrollDetail->GetThumbFrame()->SetUseParent( m_pTextDetailInfo );
	RegisterControl ( m_pScrollDetail );


	m_pTextStepInfo = new CBasicTextBox;
	m_pTextStepInfo->CreateSub ( this, "RAN_QUEST_WINDOW_STEP_TEXT_END_LUXES" );
	m_pTextStepInfo->SetFont ( pFont9 );
	m_pTextStepInfo->SetTextAlign ( TEXT_ALIGN_BOTH_X );
	m_pTextStepInfo->AddText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextStepInfo );

	m_pScrollStep = new CBasicScrollBarEx;
	m_pScrollStep->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, MODERN_QUEST_WINDOW_DONE_INFO_STEP_SCROLL );
	m_pScrollStep->CreateBaseScrollBar ( "RAN_QUEST_WINDOW_STEP_SCROLL_END_LUXES" );
	m_pScrollStep->GetThumbFrame()->SetState ( 1, m_pTextStepInfo->GetVisibleLine() );
	m_pScrollStep->GetThumbFrame()->SetUseParent( m_pTextStepInfo );
	RegisterControl ( m_pScrollStep );


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
	m_pScrollReward->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, MODERN_QUEST_WINDOW_DONE_INFO_REWARD_SCROLL );
	m_pScrollReward->CreateBaseScrollBar ( "RAN_QUEST_WINDOW_INDEMNIFY_TEXT_SCROLLBAR_LUXES" );
	m_pScrollReward->GetThumbFrame()->SetState ( 1, m_pTextRewardInfo->GetVisibleLine() );
	m_pScrollReward->GetThumbFrame()->SetUseParent( m_pTextRewardInfo );
	RegisterControl ( m_pScrollReward );

	m_pItemSlotReward = new CItemSlot;
	m_pItemSlotReward->CreateSub ( this, "RAN_QUEST_WINDOW_INDEMNIFY_ITEM_SLOT_LUXES", UI_FLAG_DEFAULT, MODERN_QUEST_WINDOW_DONE_INFO_REWARD_ITEM_SLOT );
	m_pItemSlotReward->CreateSubControl ( GLQUESTPROG::INVEN_SIZEX, TRUE );
	RegisterControl ( m_pItemSlotReward );
}

void CModernQuestInfoDone::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
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

void CModernQuestInfoDone::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case MODERN_QUEST_WINDOW_DONE_INFO_REWARD_ITEM_SLOT:
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

void CModernQuestInfoDone::SetVisibleSingle( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		DataReset();
	}
}

void CModernQuestInfoDone::DataReset()
{
	m_dwQuestID = NATIVEID_NULL().dwID;

	if ( m_pTextTitle )				
		m_pTextTitle->ClearText();

	if ( m_pTextResultTitle )			
		m_pTextResultTitle->ClearText();

	if ( m_pTextResultInfo )			
		m_pTextResultInfo->ClearText();

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

	if ( m_pTextRewardTitle )		
		m_pTextRewardTitle->ClearText();

	if ( m_pTextRewardInfo )		
		m_pTextRewardInfo->ClearText();

	for( int i=0; i<CItemSlot::nLIMIT_COLUMN; ++i )
	{
		m_pItemSlotReward->ResetItemImage(i);
	}
}

void CModernQuestInfoDone::DataSet( DWORD dwQuestID )
{
	DataReset();

	if ( dwQuestID == NATIVEID_NULL().dwID ) return;
	m_dwQuestID = dwQuestID;

	GLQuestPlay& cQuestPlay = GLGaeaClient::GetInstance().GetCharacter ()->m_cQuestPlay;
	GLQUESTPROG* pQuestDone = cQuestPlay.FindEnd ( dwQuestID );
	if ( !pQuestDone ) return;

	GLQUEST* pQuest = GLQuestMan::GetInstance().Find ( dwQuestID );
	if ( !pQuest ) return;

	const DWORD dwSTEP = pQuestDone->m_dwSTEP;
	GLQUEST_STEP* pQuestStep = pQuest->GetSTEP ( dwSTEP );	
	if ( !pQuestStep )	return;

	GLQUESTPROG_STEP& sSTEP_PROG = pQuestDone->m_sSTEP_PROG;	

	if ( m_pTextTitle )
	{
		int nPosition;
		if ( RANPARAM::strLangPlacer != "en" )
		{
			//xml translation
			CString strTemp;				
			strTemp.Format ( "QN_%03d_%03d", pQuestDone->m_sNID.wMainID, pQuestDone->m_sNID.wSubID );
			int nPosition = m_pTextTitle->AddText( ID2GAMEWORDQUEST( strTemp ), NS_UITEXTCOLOR::WHITE );
		}
		else
		{
			int nPosition = m_pTextTitle->AddText( pQuest->m_strTITLE.c_str(),NS_UITEXTCOLOR::WHITE );

			//if ( !pQuestDone->m_bCOMPLETE )	m_pTextTitle->SetTextImage ( nPosition, "RAN_QUEST_WINDOW_STATUS_FAIL_ICON_LUXES" );	
			//else	m_pTextTitle->SetTextImage ( nPosition, "RAN_QUEST_WINDOW_STATUS_DONE_ICON_LUXES" );
		}

		if ( !pQuestDone->m_bCOMPLETE )	m_pTextTitle->SetTextImage ( nPosition, "RAN_QUEST_WINDOW_STATUS_FAIL_ICON_LUXES" );	
		else	m_pTextTitle->SetTextImage ( nPosition, "RAN_QUEST_WINDOW_STATUS_DONE_ICON_LUXES" );
	}

	if ( m_pTextResultTitle )	m_pTextResultTitle->SetText( ID2GAMEWORD("RAN_QUEST_LIST_RESULT",0),NS_UITEXTCOLOR::ORANGE );

	if ( m_pTextResultInfo )
	{
		CString strCombine;
		const int nCOMPLETE_INDEX = (pQuestDone->m_bCOMPLETE)?1:2;
		const DWORD dwCOUNT = pQuestDone->m_dwCOUNT;
		strCombine.Format ( "%s/%s:%d",ID2GAMEWORD("RAN_QUEST_LIST_RESULT", nCOMPLETE_INDEX ), ID2GAMEWORD( "RAN_QUEST_LIST_RESULT", 3 ), dwCOUNT );
		D3DCOLOR dwColor = NS_UITEXTCOLOR::GREENYELLOW;
		if ( !pQuestDone->m_bCOMPLETE ) dwColor = NS_UITEXTCOLOR::RED;
		int nIndex = m_pTextResultInfo->AddText ( strCombine, dwColor );
	}

	if ( m_pTextDescTitle )		m_pTextDescTitle->SetText( ID2GAMEWORD("RAN_QUEST_LIST_EXPLAIN", 0 ), NS_UITEXTCOLOR::ORANGE );

	if ( m_pTextDescInfo )
	{
		if ( RANPARAM::strLangPlacer != "en" )
		{
			//xml translation
			CString strTemp;
			strTemp.Format ( "QD_%03d_%03d", pQuestDone->m_sNID.wMainID, pQuestDone->m_sNID.wSubID );
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

			const float fLapsTime = pQuestDone->m_fLAPSTIME;
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
			strTemp.Format ( "QD_%03d_%03d_D_%d", pQuestDone->m_sNID.wMainID, pQuestDone->m_sNID.wSubID, dwSTEP );
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
		GLQUEST::VECQSTEP& vecProgStep = pQuest->m_vecProgStep;
		for ( int i = 0; i < (int)vecProgStep.size(); ++i )
		{
			const GLQUEST_STEP& sQuestStep = vecProgStep[i];
			int nIndex;
			if ( RANPARAM::strLangPlacer != "en" )
			{
				//xml translation
				CString strTemp;
				strTemp.Format ( "QN_%03d_%03d_P_%d", pQuestDone->m_sNID.wMainID, pQuestDone->m_sNID.wSubID, dwSTEP );
				nIndex = m_pTextStepInfo->AddText ( ID2GAMEWORDQUEST( strTemp ), NS_UITEXTCOLOR::DARKGRAY );
			}
			else
			{
				nIndex = m_pTextStepInfo->AddText ( sQuestStep.GetTITLE (), NS_UITEXTCOLOR::DARKGRAY );
			}

			m_pTextStepInfo->SetTextImage ( nIndex, "RAN_QUEST_WINDOW_STEP_BUTTON_LUXES" );
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


}