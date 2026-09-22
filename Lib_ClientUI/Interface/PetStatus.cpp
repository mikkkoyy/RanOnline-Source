#include "StdAfx.h"
#include "PetStatus.h"
#include "ItemImage.h"
#include "InnerInterface.h"
#include "GLItemMan.h"
#include "UITextControl.h"
#include "GLGaeaClient.h"
#include "BasicMiniBarGauge.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPetStatus::CPetStatus () 
	: m_pPetImage(NULL)
	, m_pGauge(NULL)
	, m_pDummyControl(NULL)
	, m_sNativeID(false)
	, m_strName("")
	, m_fTotal(0.0f)
	, m_fRemain(0.0f)
	, m_fRemainBack(0.1f)
	, m_fCheckTimer(0.0f)
{
}

CPetStatus::~CPetStatus ()
{
}

void  CPetStatus::CreateSubControl ()
{
	CItemImage* pItemImage = new CItemImage;
	pItemImage->CreateSub ( this, "PET_STATUS_ITEM_IMAGE" );
	pItemImage->CreateSubControl ();
	pItemImage->SetUseRender ( TRUE );
	pItemImage->SetVisibleSingle ( FALSE );
	RegisterControl ( pItemImage );
    m_pPetImage = pItemImage;

	CUIControl* pDummyControl = new CUIControl;
	pDummyControl->CreateSub ( this, "PET_STATUS_NO_FOOD");
	RegisterControl ( pDummyControl );
	m_pDummyControl = pDummyControl;
	m_pDummyControl->SetVisibleSingle( FALSE );

	m_pGauge = new CBasicMiniBarGauge;
	m_pGauge->CreateSub( this, "PET_STATUS_FOOD_GAUGE", UI_FLAG_DEFAULT, PET_FOOD_GAUGE );
	m_pGauge->CreateSubControl();
	m_pGauge->SetVisibleSingle( TRUE );
	RegisterControl ( m_pGauge );
}

void CPetStatus::SetItem ( const SITEMCUSTOM& sItemCustom )
{
	if ( m_pPetImage )
	{
		m_pPetImage->SetItem ( sItemCustom );
		m_pPetImage->SetVisibleSingle ( TRUE );
	} 
}

void CPetStatus::ResetItem ()
{
	if ( m_pPetImage )
	{
		m_pPetImage->SetVisibleSingle ( FALSE );
		m_pPetImage->ResetItem ();
	}
}

void CPetStatus::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
	
	if ( !IsVisible() )	return;

	SITEMCUSTOM sItem = GLGaeaClient::GetInstance().GetCharacter()->m_sPetCardNativeID;
	if ( m_sNativeID != sItem.sNativeID ) 
	{
		m_sNativeID = sItem.sNativeID;
		ResetItem ();
		m_strName = "";
		m_fTotal = 0.0f;

		SITEM* pItem = GLItemMan::GetInstance().GetItem ( m_sNativeID );
		if ( pItem && pItem->sBasicOp.emItemType == ITEM_PET_CARD )
		{
			m_strName = pItem->GetName();	
			SetItem ( sItem );	

			PGLPET pPet = GLCONST_PET::GetPetData ( pItem->sPet.emPetType );
			if ( pPet )
				m_fTotal = float( pPet->m_nFull );
		}
	}

	m_fRemain = float( GLGaeaClient::GetInstance().GetPetClient()->m_nFull );

	if ( m_fRemainBack != m_fRemain )
	{
		m_fRemainBack = m_fRemain;

		float fPercent = m_fRemain / m_fTotal;

		if ( m_pGauge )
			m_pGauge->SetPercent( fPercent );

		if ( m_fRemain <= 100.0f ) 
		{
			m_pDummyControl->SetVisibleSingle( TRUE );
		}else{
			m_pDummyControl->SetVisibleSingle( FALSE );
		}
	}

	if ( CHECK_MOUSE_IN ( GetMessageEx () ) )
	{
		CString strTemp;
		float fPercent = m_fRemain / m_fTotal;
		strTemp.Format( "%s Food:%g Percent", m_strName.GetString(), fPercent * 100 );
		CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO ( strTemp.GetString() , NS_UITEXTCOLOR::WHITE );
	}

	

	if ( CHECK_MOUSEIN_LBUPLIKE ( GetMessageEx () ) )
	{
		if( GLGaeaClient::GetInstance().GetPetClient()->IsVALID() )
		{
			if ( GLTradeClient::GetInstance().Valid() ) return;
			
			if ( CInnerInterface::GetInstance().IsVisibleGroup( PET_WINDOW ) )		
				CInnerInterface::GetInstance().HideGroup ( PET_WINDOW );
			else	
				CInnerInterface::GetInstance().ShowGroupFocus( PET_WINDOW );	
		}
	}	

	m_fCheckTimer += fElapsedTime;
	if ( m_fCheckTimer >= 2.0f )
	{
		if( !GLGaeaClient::GetInstance().GetPetClient()->IsVALID() )
		{
			CInnerInterface::GetInstance().HideGroup ( GetWndID() );
		}
	}
}
