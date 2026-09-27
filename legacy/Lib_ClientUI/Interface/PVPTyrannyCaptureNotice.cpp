#include "StdAfx.h"
#include "PVPTyrannyCaptureNotice.h"
#include "GLPVPTyrannyDefine.h"
#include "InnerInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPTyrannyCaptureNotice::CPVPTyrannyCaptureNotice ()
	: m_pImage(NULL)
	, m_pSchoolImage(NULL)
	, m_pSchoolText(NULL)
	, m_fTimer(0.0f)
{
}

CPVPTyrannyCaptureNotice::~CPVPTyrannyCaptureNotice ()
{
}

void CPVPTyrannyCaptureNotice::CreateSubControl ( int nType )
{
	if ( nType < 0 || nType >= TYRANNY_SCHOOL_SIZE )	return;

	std::string strSchooImage[TYRANNY_SCHOOL_SIZE] = 
	{
		"PROGRESS_DISPLAY_CTF_SCHOOL_0",
		"PROGRESS_DISPLAY_CTF_SCHOOL_1",
		"PROGRESS_DISPLAY_CTF_SCHOOL_2",
	};

	std::string strSchoolText[TYRANNY_SCHOOL_SIZE] = 
	{
		"PROGRESS_DISPLAY_CTF_TEXT_CAPTURED_BY_0",
		"PROGRESS_DISPLAY_CTF_TEXT_CAPTURED_BY_1",
		"PROGRESS_DISPLAY_CTF_TEXT_CAPTURED_BY_2",
	};

	
	m_pImage = new CUIControl;
	m_pImage->CreateSub ( this, "PROGRESS_DISPLAY_CTF_BACK_1" );
	RegisterControl ( m_pImage );

	m_pSchoolImage = new CUIControl;
	m_pSchoolImage->CreateSub ( this, strSchooImage[nType].c_str() );
	RegisterControl ( m_pSchoolImage );

	m_pSchoolText = new CUIControl;
	m_pSchoolText->CreateSub ( this, strSchoolText[nType].c_str() );
	RegisterControl ( m_pSchoolText );	
}

void CPVPTyrannyCaptureNotice::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
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
