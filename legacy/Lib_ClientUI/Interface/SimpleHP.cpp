#include "StdAfx.h"
#include "SimpleHP.h"
#include "./BasicProgressBar.h"
#include "BasicLineBoxEx.h"
#include "GLGaeaClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSimpleHP::CSimpleHP () :
	m_pHP ( NULL )
{
}

CSimpleHP::~CSimpleHP ()
{
}

void CSimpleHP::CreateSubControl ()
{
	CBasicLineBoxEx* pLineBox = new CBasicLineBoxEx;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_EX_SIMPLEHP", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxTargetInfo ( "BASIC_SIMPLEHP_LINE_BOX" );
	RegisterControl ( pLineBox );

	CBasicProgressBar* pHP = new CBasicProgressBar;
	pHP->CreateSub ( this, "SIMPLE_HP_IMAGE" );
	pHP->CreateOverImage ( "SIMPLE_HP_OVERIMAGE" );		
	RegisterControl ( pHP );
	m_pHP = pHP;	
}

void CSimpleHP::SetHP ( WORD wNOW, WORD wMAX )
{
	const float fPercent = float(wNOW) / float(wMAX);
	m_pHP->SetPercent ( fPercent );
}

void CSimpleHP::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	const GLCHARLOGIC& sCharData = GLGaeaClient::GetInstance().GetCharacterLogic ();
	SetHP ( sCharData.m_sHP.dwNow, sCharData.m_sHP.dwMax );
}