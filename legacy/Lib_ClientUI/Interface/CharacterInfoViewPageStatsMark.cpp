#include "StdAfx.h"
#include "CharacterInfoViewPageStatsMark.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCharacterInfoViewPageStatsMark::CCharacterInfoViewPageStatsMark ()
	: m_pUpImage(NULL)
	, m_pDownImage(NULL)
{
}

CCharacterInfoViewPageStatsMark::~CCharacterInfoViewPageStatsMark ()
{
}

void CCharacterInfoViewPageStatsMark::CreateSubControl ()
{
	m_pUpImage = new CUIControl;
	m_pUpImage->CreateSub ( this, "RAN_ANOTHER_CHARACTER_WEAR_UPPER_ARROW", UI_FLAG_DEFAULT, MARK_IMAGE_UP );
	m_pUpImage->SetVisibleSingle( FALSE );
	RegisterControl ( m_pUpImage );

	m_pDownImage = new CUIControl;
	m_pDownImage->CreateSub ( this, "RAN_ANOTHER_CHARACTER_WEAR_LOWER_ARROW", UI_FLAG_DEFAULT, MARK_IMAGE_DOWN );
	m_pDownImage->SetVisibleSingle( FALSE );
	RegisterControl ( m_pDownImage );
}

void CCharacterInfoViewPageStatsMark::Update ( int x ,int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CCharacterInfoViewPageStatsMark::TranslateUIMessage ( UIGUID cID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( cID, dwMsg );
}

void CCharacterInfoViewPageStatsMark::SetVisibleSingle ( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle ( bVisible );
	if ( bVisible )
	{
		Reset();
	}
}

void CCharacterInfoViewPageStatsMark::Reset()
{
	if ( !m_pUpImage || !m_pDownImage )	return;

	m_pUpImage->SetVisibleSingle( FALSE );
	m_pDownImage->SetVisibleSingle( FALSE );
}

void CCharacterInfoViewPageStatsMark::ShowImage( bool bUp )
{
	if ( !m_pUpImage || !m_pDownImage )	return;
	Reset();

	m_pUpImage->SetVisibleSingle( bUp );
	m_pDownImage->SetVisibleSingle( !bUp );
}

bool CCharacterInfoViewPageStatsMark::CheckValue( int nOrig, int nToCheck )
{
	if ( nToCheck == nOrig )
	{
		Reset();
		return false;
	}else{
		ShowImage( nToCheck > nOrig );
		return true;
	}

	return false;
}

bool CCharacterInfoViewPageStatsMark::CheckValue( int nCheck )
{
	ShowImage( nCheck > 0 );
	return true;
}