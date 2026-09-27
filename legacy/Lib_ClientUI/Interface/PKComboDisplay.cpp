#include "StdAfx.h"
#include "PKComboDisplay.h"
#include "PKComboType.h"
#include "InnerInterface.h"
#include "GLGaeaClient.h"
#include "DxViewPort.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPKComboDisplay::CPKComboDisplay ()
{
}

CPKComboDisplay::~CPKComboDisplay ()
{
}

void CPKComboDisplay::CreateSubControl()
{
	CString strKeyword[MAX_KILL] =
	{
		"PK_COMBO_DOUBLE_KILL",
		"PK_COMBO_TRIPLE_KILL",
		"PK_COMBO_QUARD_KILL",
		"PK_COMBO_MASTER_KILL",
		"PK_COMBO_KILLING_SPREE",
		"PK_COMBO_DOMINATING",
		"PK_COMBO_MEGA_KILL",
		"PK_COMBO_UNSTOPPABLE",
		"PK_COMBO_WICKED_SICK",
		"PK_COMBO_MONSTER_KILL",
		"PK_COMBO_GODLIKE",
		"PK_COMBO_HOLY_SHIT",
		"PK_COMBO_OWNAGE",
	};

	for ( int i = 0; i < MAX_KILL; ++i )
	{
		m_pKILL_COUNT[i] = new CPKComboType;
		m_pKILL_COUNT[i]->CreateSub ( this, strKeyword[i].GetString(), UI_FLAG_DEFAULT, DOUBLE_KILL + i );
		m_pKILL_COUNT[i]->CreateSubControl ( strKeyword[i] );
		m_pKILL_COUNT[i]->SetVisibleSingle ( FALSE );
		m_pKILL_COUNT[i]->SetUseRender ( FALSE );
		m_pKILL_COUNT[i]->STOP ();
		m_pKILL_COUNT[i]->RESET ();		
		RegisterControl ( m_pKILL_COUNT[i] );
	}

	m_pPositionControl = new CUIControl;
	m_pPositionControl->CreateSub ( this, "PKCOMBO_DISPLAY_POSITION" );
	m_pPositionControl->SetVisibleSingle ( FALSE );
	RegisterControl ( m_pPositionControl );
}

bool	CPKComboDisplay::START ( UIGUID cID )
{
	if ( cID < DOUBLE_KILL || OWNAGE < cID ) return false;
	
	int nIndex = cID - DOUBLE_KILL;
	m_pKILL_COUNT[nIndex]->SetVisibleSingle ( TRUE );
	m_pKILL_COUNT[nIndex]->START ();

	return true;
}

bool	CPKComboDisplay::RESET ( UIGUID cID )
{
	if ( cID < DOUBLE_KILL || OWNAGE < cID ) return false;

	int nIndex = cID - DOUBLE_KILL;	
	m_pKILL_COUNT[nIndex]->RESET ();

    return true;
}

bool	CPKComboDisplay::STOP ()
{
	for ( int i = 0; i>MAX_KILL; ++i )
	{
		m_pKILL_COUNT[i]->STOP ();
		m_pKILL_COUNT[i]->SetVisibleSingle ( FALSE );
	}

	return true;
}

void CPKComboDisplay::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	D3DXVECTOR3 vPos = GLGaeaClient::GetInstance().GetCharacter()->GetPosBodyHeight();

	static D3DXVECTOR3 vScreenBack;
	D3DXVECTOR3 vScreen = DxViewPort::GetInstance().ComputeVec3Project ( &vPos, NULL );

	// 마우스 움직임에 흔들림을 보정한다.
	if( abs( vScreenBack.x - vScreen.x ) < 1.0f )
	{
		vScreen.x = vScreenBack.x;
	}
	
	bool bPLAYING( false );

	for ( int i = 0; i < MAX_KILL; ++i )
	{
		const UIRECT& rcOriginPos = m_pKILL_COUNT[i]->GetGlobalPos ();

		D3DXVECTOR2 vPos;
		vPos.x = floor(vScreen.x - ( rcOriginPos.sizeX * 0.5f ));
		vPos.y = m_pPositionControl->GetGlobalPos().top;

		if ( m_pKILL_COUNT[i]->ISPLAYING () )
		{
			m_pKILL_COUNT[i]->SetGlobalPos ( vPos );

			bPLAYING = true;
		}
		else
		{
			STOP ();
		}
	}

	vScreenBack = vScreen;

	if ( !bPLAYING ) CInnerInterface::GetInstance().HideGroup ( GetWndID () );
}