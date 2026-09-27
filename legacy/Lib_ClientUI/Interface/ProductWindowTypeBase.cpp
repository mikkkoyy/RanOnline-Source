#include "StdAfx.h"
#include "ProductWindowTypeBase.h"
#include "BasicButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CProductWindowTypeBase::CProductWindowTypeBase ()
	: m_pImageBase(NULL)
	, m_pImageSelect(NULL)
	, m_pImageOver(NULL)
{
}

CProductWindowTypeBase::~CProductWindowTypeBase ()
{
}

void CProductWindowTypeBase::CreateSubControl ( std::string strbase, std::string strselect, std::string strover )
{
	m_pImageBase = new CUIControl;
	m_pImageBase->CreateSub ( this, strbase.c_str(), UI_FLAG_DEFAULT );
	RegisterControl ( m_pImageBase );

	m_pImageSelect = new CUIControl;
	m_pImageSelect->CreateSub ( this, strselect.c_str(), UI_FLAG_DEFAULT );
	m_pImageSelect->SetVisibleSingle( FALSE );
	RegisterControl ( m_pImageSelect );

	m_pImageOver = new CUIControl;
	m_pImageOver->CreateSub ( this, strover.c_str(), UI_FLAG_DEFAULT );
	m_pImageOver->SetVisibleSingle( FALSE );
	RegisterControl ( m_pImageOver );
}

void CProductWindowTypeBase::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( m_pImageOver )
		m_pImageOver->SetVisibleSingle( FALSE );

	if ( CHECK_MOUSE_IN ( GetMessageEx() ) ){
		if ( m_pImageOver && m_pImageSelect && !m_pImageSelect->IsVisible() )
			m_pImageOver->SetVisibleSingle( TRUE );		
	}
}

void CProductWindowTypeBase::SetSelect( BOOL bSelect )
{
	if ( m_pImageSelect )
		m_pImageSelect->SetVisibleSingle( bSelect );
}
