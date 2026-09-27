#include "StdAfx.h"

#include "SlotCountMsg.h"
#include "InnerInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSlotCountMsg::CSlotCountMsg()
	: m_wMin(0)
	, m_wMax(0)
	, m_wWin(0)
	, m_bRoll(false)
	, m_bStandby(false)
	, m_fElapsedTime(0.0)
{
	memset ( m_pHUN, 0, sizeof( m_pHUN ) );
	memset ( m_pTEN, 0, sizeof( m_pTEN ) );
	memset ( m_pONE, 0, sizeof( m_pONE ) );
}

CSlotCountMsg::~CSlotCountMsg ()
{
}

void CSlotCountMsg::CreateSubControl()
{
	CString strNUMBER;
	for ( int i = 0; i < 10; ++i )
	{
		strNUMBER.Format( "SLOT_DISPLAY_NUMBER_%d", i );
		m_pHUN[i] = CreateControl ( strNUMBER.GetString() );
		m_pTEN[i] = CreateControl ( strNUMBER.GetString() );
		m_pONE[i] = CreateControl ( strNUMBER.GetString() );
	}
}

CUIControl*	CSlotCountMsg::CreateControl( CONST TCHAR* szControl )
{
	GASSERT( szControl );

	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl );
	pControl->SetVisibleSingle ( FALSE );
	RegisterControl ( pControl );

	return pControl;
}

void CSlotCountMsg::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if( m_bRoll )
	{
		if( m_fElapsedTime <= 0.01f )
		{
			m_fElapsedTime += fElapsedTime;
		}
		else
		{
			for( int i=0; i<10; ++i )
			{
				m_pHUN[i]->SetVisibleSingle( FALSE );
				m_pTEN[i]->SetVisibleSingle( FALSE );
				m_pONE[i]->SetVisibleSingle( FALSE );
			}

			// 남은 시간으로 숫자를 구성하고 출력한다.
			int nHun = m_wWin / 100;
			int nTen = m_wWin / 10 % 10;
			int nOne = m_wWin % 10;
			
			const UIRECT & hunLRect = m_pHUN[nHun]->GetLocalPos();
			const UIRECT & hunGRect = m_pHUN[nHun]->GetGlobalPos();

			const UIRECT & tenLRect = m_pTEN[nTen]->GetLocalPos();
			const UIRECT & tenGRect = m_pTEN[nTen]->GetGlobalPos();

			UIRECT tenRect = m_pTEN[nTen]->GetGlobalPos();
			tenRect.left = hunGRect.left + hunLRect.sizeX;
			m_pTEN[nTen]->SetGlobalPos( tenRect );
			UIRECT oneRect = m_pONE[nOne]->GetGlobalPos();
			oneRect.left = tenGRect.left + tenLRect.sizeX;
			m_pONE[nOne]->SetGlobalPos( oneRect );

			m_pHUN[nHun]->SetVisibleSingle( TRUE );
			m_pTEN[nTen]->SetVisibleSingle( TRUE );
			m_pONE[nOne]->SetVisibleSingle( TRUE );
				

			srand(::GetTickCount());
			WORD range = (m_wMax-m_wMin)+1;
			m_wWin = rand()%range+m_wMin;
			m_fElapsedTime = 0.0;
		}
		m_bStandby = true;
	}
	else if( m_bStandby )
	{
		for( int i=0; i<10; ++i )
		{
			m_pHUN[i]->SetVisibleSingle( FALSE );
			m_pTEN[i]->SetVisibleSingle( FALSE );
			m_pONE[i]->SetVisibleSingle( FALSE );
		}

		// 남은 시간으로 숫자를 구성하고 출력한다.
		int nHun = m_wWin / 100;
		int nTen = m_wWin / 10 % 10;
		int nOne = m_wWin % 10;
			
		const UIRECT & hunLRect = m_pHUN[nHun]->GetLocalPos();
		const UIRECT & hunGRect = m_pHUN[nHun]->GetGlobalPos();

		const UIRECT & tenLRect = m_pTEN[nTen]->GetLocalPos();
		const UIRECT & tenGRect = m_pTEN[nTen]->GetGlobalPos();

		UIRECT tenRect = m_pTEN[nTen]->GetGlobalPos();
		tenRect.left = hunGRect.left + hunLRect.sizeX;
		m_pTEN[nTen]->SetGlobalPos( tenRect );
		UIRECT oneRect = m_pONE[nOne]->GetGlobalPos();
		oneRect.left = tenGRect.left + tenLRect.sizeX;
		m_pONE[nOne]->SetGlobalPos( oneRect );

		m_pHUN[nHun]->SetVisibleSingle( TRUE );
		m_pTEN[nTen]->SetVisibleSingle( TRUE );
		m_pONE[nOne]->SetVisibleSingle( TRUE );
		
		m_fElapsedTime += fElapsedTime;
		if( m_fElapsedTime >= 5.00f )	
		{
			m_bStandby = false;
			m_fElapsedTime = 0.0;
		}
	}
	else CInnerInterface::GetInstance().HideGroup( GetWndID() );
}

void CSlotCountMsg::SetCount( WORD wMin, WORD wMax, WORD wWin, bool bRoll )
{
	m_wMin = wMin;
	m_wMax = wMax;
	m_wWin = wWin;
	if( bRoll ) 
	{
		m_bRoll = bRoll;
		m_bStandby = true;
	}
	else m_bRoll = false;
}