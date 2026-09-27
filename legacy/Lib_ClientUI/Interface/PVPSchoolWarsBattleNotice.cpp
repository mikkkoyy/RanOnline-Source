#include "StdAfx.h"
#include "PVPSchoolWarsBattleNotice.h"
#include "GLPVPSchoolWarsDefine.h"
#include "InnerInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPSchoolWarsBattleNotice::CPVPSchoolWarsBattleNotice ()
	: m_pImage(NULL)
	, m_pImageBG(NULL)
	, m_fTimer(0.0f)
{
}

CPVPSchoolWarsBattleNotice::~CPVPSchoolWarsBattleNotice ()
{
}

void CPVPSchoolWarsBattleNotice::CreateSubControlStartImage ()
{
	m_pImageBG = new CUIControl;
	m_pImageBG->CreateSub ( this, "PROGRESS_DISPLAY_SCHOOLWARS_BACK_0" );
	RegisterControl ( m_pImageBG );

	m_pImage = new CUIControl;
	m_pImage->CreateSub ( this, "PROGRESS_DISPLAY_SCHOOLWARS_TEXT_START" );
	RegisterControl ( m_pImage );
}

void CPVPSchoolWarsBattleNotice::CreateSubControlEndImage ()
{
	m_pImageBG = new CUIControl;
	m_pImageBG->CreateSub ( this, "PROGRESS_DISPLAY_SCHOOLWARS_BACK_0" );
	RegisterControl ( m_pImageBG );

	m_pImage = new CUIControl;
	m_pImage->CreateSub ( this, "PROGRESS_DISPLAY_SCHOOLWARS_TEXT_STOP" );
	RegisterControl ( m_pImage );
}

void CPVPSchoolWarsBattleNotice::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_fTimer += fElapsedTime;
	if ( m_fTimer >= 5.0f )
	{
		m_fTimer = 0.0f;
		CInnerInterface::GetInstance().HideGroup( GetWndID() );
	}
}
