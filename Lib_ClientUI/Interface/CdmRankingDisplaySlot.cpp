#include "StdAfx.h"
#include "CdmRankingDisplaySlot.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "GameTextControl.h"
#include "UITextControl.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCdmRankingDisplaySlot::CCdmRankingDisplaySlot ()
	: m_pTextRank(NULL)
	, m_pTextClub(NULL)
	, m_pTextScore(NULL)
{
}

CCdmRankingDisplaySlot::~CCdmRankingDisplaySlot ()
{
}

void CCdmRankingDisplaySlot::CreateSubControl ()
{
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG );

	m_pTextRank = new CBasicTextBox;
	m_pTextRank->CreateSub ( this, "PVP_CDM_RANKING_TEXT_BASE_RANK" );
	m_pTextRank->SetFont ( pFont8 );
	m_pTextRank->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextRank->SetText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextRank );

	m_pTextClub = new CBasicTextBox;
	m_pTextClub->CreateSub ( this, "PVP_CDM_RANKING_TEXT_BASE_CLUB" );
	m_pTextClub->SetFont ( pFont8 );
	m_pTextClub->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextClub->SetText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextClub );

	m_pTextScore = new CBasicTextBox;
	m_pTextScore->CreateSub ( this, "PVP_CDM_RANKING_TEXT_BASE_SCORE" );
	m_pTextScore->SetFont ( pFont8 );
	m_pTextScore->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextScore->SetText( "--", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextScore );
}

void CCdmRankingDisplaySlot::ResetData()
{
	if ( m_pTextRank )		m_pTextRank->ClearText();
	if ( m_pTextClub )		m_pTextClub->ClearText();
	if ( m_pTextScore )		m_pTextScore->ClearText();
}

void CCdmRankingDisplaySlot::SetData( int nRank, std::string strClub, WORD wKill, WORD wDeath )
{
	ResetData();

	CString strText;

	if ( m_pTextRank )
	{
		strText.Format( "%d",nRank );
		m_pTextRank->AddText( strText.GetString(), NS_UITEXTCOLOR::WHITE );
	}

	if ( m_pTextClub )
	{
		m_pTextClub->AddText( strClub.c_str(), NS_UITEXTCOLOR::WHITE );
	}

	if ( m_pTextScore )
	{
		strText.Format( "K:%u", wKill );
		int nIndex = m_pTextScore->AddText( strText.GetString(), NS_UITEXTCOLOR::GREENYELLOW );
		m_pTextScore->AddString( nIndex, "/" );
		strText.Format( "D:%u", wDeath );
		m_pTextScore->AddString( nIndex, strText.GetString(), NS_UITEXTCOLOR::ORNAGERED );
	}
}