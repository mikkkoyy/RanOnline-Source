#include "StdAfx.h"
#include "CdmRankingDisplay.h"
#include "CdmRankingDisplaySlot.h"
#include "./BasicTextBox.h"
#include "d3dfont.h"
#include "GLGaeaClient.h"
#include "UITextControl.h"
#include "GameTextControl.h"
#include "BasicLineBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CCdmRankingDisplay::CCdmRankingDisplay()
	: m_pRankText(NULL)
	, m_pClubText(NULL)
	, m_pScoreText(NULL)
	, m_pSlotOwn(NULL)
{
	for( int i=0; i<RANKING_NUM; ++i )
		m_pSlot[i] = NULL;
}

CCdmRankingDisplay::~CCdmRankingDisplay()
{
}

void CCdmRankingDisplay::CreateSubControl ()
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "PVP_CDM_RANKING_LINEBOX" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody( "PVP_CDM_RANKING_LINEBOX_HEAD" );
	RegisterControl ( pLineBox );	

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody( "PVP_CDM_RANKING_LINEBOX_HEAD" );
	RegisterControl ( pLineBox );	

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody( "PVP_CDM_RANKING_LINEBOX_LOWER" );
	RegisterControl ( pLineBox );

	m_pRankText = new CBasicTextBox;
	m_pRankText->CreateSub ( this, "PVP_CDM_RANKING_TEXT_BASE_RANK" );
	m_pRankText->SetFont ( pFont );
	m_pRankText->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pRankText->AddText( ID2GAMEWORD("PVP_CDM_RANKING_TEXT", 0 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pRankText );	

	m_pClubText = new CBasicTextBox;
	m_pClubText->CreateSub ( this, "PVP_CDM_RANKING_TEXT_BASE_CLUB" );
	m_pClubText->SetFont ( pFont );
	m_pClubText->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pClubText->AddText( ID2GAMEWORD("PVP_CDM_RANKING_TEXT", 1 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pClubText );	

	m_pScoreText = new CBasicTextBox;
	m_pScoreText->CreateSub ( this, "PVP_CDM_RANKING_TEXT_BASE_SCORE" );
	m_pScoreText->SetFont ( pFont );
	m_pScoreText->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pScoreText->AddText( ID2GAMEWORD("PVP_CDM_RANKING_TEXT", 2 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pScoreText );	

	std::string strSlots[RANKING_NUM] = 
	{
		"PVP_CDM_RANKING_SLOT_0",
		"PVP_CDM_RANKING_SLOT_1",
		"PVP_CDM_RANKING_SLOT_2",
		"PVP_CDM_RANKING_SLOT_3",
		"PVP_CDM_RANKING_SLOT_4",
		"PVP_CDM_RANKING_SLOT_5",
		"PVP_CDM_RANKING_SLOT_6",
		"PVP_CDM_RANKING_SLOT_7",
		"PVP_CDM_RANKING_SLOT_8",
		"PVP_CDM_RANKING_SLOT_9",
	};
	
	for( int i=0; i<RANKING_NUM; ++i )
	{
		m_pSlot[i] = new CCdmRankingDisplaySlot;
		m_pSlot[i]->CreateSub( this, strSlots[i].c_str(), UI_FLAG_DEFAULT, CDM_RANKING_SLOT_0 + i );
		m_pSlot[i]->CreateSubControl ();
		RegisterControl ( m_pSlot[i] );
	}

	m_pSlotOwn = new CCdmRankingDisplaySlot;
	m_pSlotOwn->CreateSub( this, "PVP_CDM_RANKING_SLOT_OWN", UI_FLAG_DEFAULT, CDM_RANKING_SLOT_OWN );
	m_pSlotOwn->CreateSubControl ();
	RegisterControl ( m_pSlotOwn );
}

void CCdmRankingDisplay::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CCdmRankingDisplay::RefreshCdmRanking()
{
	for( int i=0; i<RANKING_NUM; ++i ){
		if ( m_pSlot[i] )	m_pSlot[i]->ResetData();
	}

	const SCDM_RANK_EX& sMyRank = GLGaeaClient::GetInstance().m_sMyCdmRank;

	if ( m_pSlotOwn )
	{
		m_pSlotOwn->ResetData();
		m_pSlotOwn->SetData( sMyRank.wClubRanking, sMyRank.szClubName, sMyRank.wKillNum, sMyRank.wDeathNum );
	}

	int nRankNum = GLGaeaClient::GetInstance().m_vecCdmRank.size();
	CString strRanking;

	CDM_RANK_VEC& vecCdmRank = GLGaeaClient::GetInstance().m_vecCdmRank;

	for ( int i=0; i<nRankNum; ++i )
	{
		if ( i >= RANKING_NUM )	continue;;

		if ( m_pSlot[i] )	
			m_pSlot[i]->SetData( vecCdmRank[i].wClubRanking, vecCdmRank[i].szClubName, vecCdmRank[i].wKillNum, vecCdmRank[i].wDeathNum );
	}
}