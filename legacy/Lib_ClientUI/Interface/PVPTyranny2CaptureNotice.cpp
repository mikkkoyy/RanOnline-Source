#include "StdAfx.h"
#include "PVPTyranny2CaptureNotice.h"
#include "GLPVPTyrannyDefine.h"
#include "InnerInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPTyranny2CaptureNotice::CPVPTyranny2CaptureNotice ()
	: m_pImage(NULL)
	, m_fTimer(0.0f)
{
}

CPVPTyranny2CaptureNotice::~CPVPTyranny2CaptureNotice ()
{
}

void CPVPTyranny2CaptureNotice::CreateSubControl ( int nType )
{
	if ( nType < 0 )	return;

	if ( nType >= TYRANNY_SCHOOL_SIZE )
	{
		m_pImage = new CUIControl;
		m_pImage->CreateSub ( this, "NOTIFY_CTF_SPURT_NOTICE" );
		RegisterControl ( m_pImage );
	}
	else
	{
		std::string strSchooImage[TYRANNY_SCHOOL_SIZE] = 
		{
			"NOTIFY_CTF_SPURT_WIN_SM",
			"NOTIFY_CTF_SPURT_WIN_HA",
			"NOTIFY_CTF_SPURT_WIN_BH",
		};

		m_pImage = new CUIControl;
		m_pImage->CreateSub ( this, strSchooImage[nType].c_str() );
		RegisterControl ( m_pImage );
	}
}

void CPVPTyranny2CaptureNotice::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
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
