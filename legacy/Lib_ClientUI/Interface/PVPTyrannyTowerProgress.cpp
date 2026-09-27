#include "StdAfx.h"
#include "PVPTyrannyTowerProgress.h"

#include "BasicProgressBar.h"
#include "InnerInterface.h"
#include "GLGaeaClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPTyrannyTowerProgress::CPVPTyrannyTowerProgress ()
{
	for ( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i )
	{
		m_pTowerStatus[i] = NULL;
		m_pTowerOwner[i] = NULL;
	}
}

CPVPTyrannyTowerProgress::~CPVPTyrannyTowerProgress ()
{
}

void CPVPTyrannyTowerProgress::CreateSubControl ()
{
	std::string strProgress[TYRANNY_SCHOOL_SIZE] = 
	{
		"AUTHENTICATOR_CTF_DISPLAY_IMAGE_SM",
		"AUTHENTICATOR_CTF_DISPLAY_IMAGE_HA",
		"AUTHENTICATOR_CTF_DISPLAY_IMAGE_BH",
	};

	std::string strProgressOver[TYRANNY_SCHOOL_SIZE] = 
	{
		"AUTHENTICATOR_CTF_DISPLAY_IMAGE_SM_OVER",
		"AUTHENTICATOR_CTF_DISPLAY_IMAGE_HA_OVER",
		"AUTHENTICATOR_CTF_DISPLAY_IMAGE_BH_OVER",	
	};

	std::string strOwner[TYRANNY_SCHOOL_SIZE] = 
	{
		"AUTHENTICATOR_CTF_DISPLAY_IMAGE_SM_OWNER",
		"AUTHENTICATOR_CTF_DISPLAY_IMAGE_HA_OWNER",
		"AUTHENTICATOR_CTF_DISPLAY_IMAGE_BH_OWNER",
	};

	for ( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i )
	{
		m_pTowerStatus[i] = new CBasicProgressBar;
		m_pTowerStatus[i]->CreateSub ( this, strProgress[i].c_str() );
		m_pTowerStatus[i]->CreateOverImage ( (char*)strProgressOver[i].c_str() );
		m_pTowerStatus[i]->SetType( CBasicProgressBar::VERTICAL );
		m_pTowerStatus[i]->SetPercent( 0.0f );
		RegisterControl ( m_pTowerStatus[i] );

		m_pTowerOwner[i] = new CUIControl;
		m_pTowerOwner[i]->CreateSub ( this, strOwner[i].c_str() );
		m_pTowerOwner[i]->SetVisibleSingle( FALSE );
		RegisterControl ( m_pTowerOwner[i] );
	}
}

void CPVPTyrannyTowerProgress::UpdateInfo( WORD wOwner, float* fDamage )
{
	for ( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i )
	{
		if ( m_pTowerStatus[i] )
			m_pTowerStatus[i]->SetPercent( 0.0f );

		if ( m_pTowerOwner[i] )
			m_pTowerOwner[i]->SetVisibleSingle( FALSE );
	}

	
	if ( wOwner < TYRANNY_SCHOOL_SIZE )
	{
		if ( m_pTowerOwner[wOwner] )
			m_pTowerOwner[wOwner]->SetVisibleSingle( TRUE );
	}

	for ( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i )
	{
		float fpercent = fDamage[i];
		if ( m_pTowerStatus[i] && fpercent > 0.0f )
			m_pTowerStatus[i]->SetPercent( fpercent / 100.0f );
	}
}