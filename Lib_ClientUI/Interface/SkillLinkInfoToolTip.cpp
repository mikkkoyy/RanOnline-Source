#include "StdAfx.h"
#include "../../../Lib_Engine/Common/StringFormat.h"
#include "BasicLineBoxSmart.h"
#include "BasicTextBox.h"
#include "UITextControl.h"
#include "GameTextControl.h"
#include "BasicVarTextBox.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "../Lib_Engine/DxCommon/d3dfont.h"
#include "../Lib_Client/DxGlobalStage.h"
#include "../Lib_Client/G-Logic/GLGaeaClient.h"

#include "InnerInterface.h"
#include "SkillImage.h"
#include "SkillLinkInfoTooltip.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSkillLinkInfoTooltip::CSkillLinkInfoTooltip()
: m_wLinkCount(0)
{
	for( int i=1; i<3; ++i )
	{
		m_pLinkSkillTooltip[i] = NULL;
	}
}

CSkillLinkInfoTooltip::~CSkillLinkInfoTooltip()
{
}

void CSkillLinkInfoTooltip::CreateSubControl()
{
	for( int i=0; i<3; ++i )
	{
		m_pLinkSkillTooltip[i] = new CSkillInfoTooltip();
		m_pLinkSkillTooltip[i]->CreateSub( this, "SKILL_INFOR_TOOLTIP", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		m_pLinkSkillTooltip[i]->CreateSubControl();
		m_pLinkSkillTooltip[i]->SetVisibleSingle( TRUE );
		RegisterControl( m_pLinkSkillTooltip[i] );
		m_pLinkSkillTooltip[i]->SetBGAlpha(180);
	}

	for( int i=1; i<3; ++i )
	{
		if( m_pLinkSkillTooltip[i] && m_pLinkSkillTooltip[i-1] )
		{
			m_pLinkSkillTooltip[i]->SetOtherMultiSkillTooltip( m_pLinkSkillTooltip[i-1] );
		}
	}
}

void CSkillLinkInfoTooltip::SetBGAlpha( int nValue )
{
	for( int i=0; i<3; ++i )
	{
		m_pLinkSkillTooltip[i]->SetDiffuseAlpha(nValue);
	}
}

void CSkillLinkInfoTooltip::DisplaySkillInformation( GLCHARLOGIC* pTarget, const GLSKILL* pSkill, const WORD wShowLevel, const WORD _emTooltipType, const bool bShowReq /* = false */ )
{
	if( 3 < m_wLinkCount || 0 > m_wLinkCount )
		return;

	m_pLinkSkillTooltip[m_wLinkCount]->DisplaySkillInformation(
		pTarget,
		pSkill,
		wShowLevel,
		_emTooltipType,
		bShowReq );

	m_pLinkSkillTooltip[m_wLinkCount]->SetVisibleSingle( TRUE );

	m_wLinkCount++;
}

void CSkillLinkInfoTooltip::Reset()
{
	for( int i=0; i<m_wLinkCount; ++i )
	{
		m_pLinkSkillTooltip[i]->ResetInformation();
		m_pLinkSkillTooltip[i]->SetVisibleSingle( FALSE );
	}
	ResetCount();
}

bool CSkillLinkInfoTooltip::IsHaveSkill( SNATIVEID sNativeID )
{
	for( WORD i=0; i<m_wLinkCount; i++ )
	{
		if( m_pLinkSkillTooltip[i]->GetSkillID() == sNativeID )
			return true;
	}

	return false;
}

void CSkillLinkInfoTooltip::SetDealyActionSSkillTooltip( const CSkillInfoTooltip* pSkillTooltip )
{
	m_pLinkSkillTooltip[0]->SetDealyActionSSkillTooltip( pSkillTooltip );
}

void CSkillLinkInfoTooltip::SetOtherMultiSkillTooltip( const CSkillInfoTooltip* pSkillTooltip )
{
	m_pLinkSkillTooltip[0]->SetOtherMultiSkillTooltip( pSkillTooltip );
}

void CSkillLinkInfoTooltip::SetMainSkillTooltip( const CSkillInfoTooltip* pSkillTooltip )
{
	m_pLinkSkillTooltip[0]->SetLinkSkillTooltip( pSkillTooltip );
}

void CSkillLinkInfoTooltip::Update( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if( 3 < m_wLinkCount || 0 >= m_wLinkCount )
		return;

	for( int i=0; i<m_wLinkCount; ++i )
	{
		m_pLinkSkillTooltip[i]->AllControlRePos( x , y );
	}
}

void CSkillLinkInfoTooltip::TranslateUIMessage( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage( ControlID, dwMsg );
}

void CSkillLinkInfoTooltip::SetVisibleSingle( bool bVisible )
{
	CUIGroup::SetVisibleSingle( bVisible );

	for( int i=0; i<3; ++i )
	{
		m_pLinkSkillTooltip[i]->SetVisibleSingle( bVisible );
	}
}

