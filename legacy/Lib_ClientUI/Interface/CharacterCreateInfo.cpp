#include "StdAfx.h"
#include "CharacterCreateInfo.h"
#include "CharacterCreateInfoSchool.h"
#include "CharacterCreateInfoSex.h"
#include "CharacterCreateInfoClass.h"

#include "GameTextControl.h"
#include "OuterInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCharacterCreateInfo::CCharacterCreateInfo ()
	: m_nSchoolLast(-1)
	, m_nSexLast(-1)
	, m_nClassLast(-1)
	, m_pInfoSchool(NULL)
	, m_pInfoSex(NULL)
	, m_pInfoClass(NULL)
{
}

CCharacterCreateInfo::~CCharacterCreateInfo ()
{
}

void CCharacterCreateInfo::CreateSubControl ()
{
	m_pInfoSchool = new CCharacterCreateInfoSchool;
	m_pInfoSchool->CreateSub( this, "CREATE_CHAR_SCHOOL_WINDOW", UI_FLAG_DEFAULT, CHARACTER_CREATE_INFO_SCHOOL );
	m_pInfoSchool->CreateSubControl ();
	RegisterControl ( m_pInfoSchool );

	m_pInfoSex = new CCharacterCreateInfoSex;
	m_pInfoSex->CreateSub( this, "CREATE_CHAR_SEX_WINDOW", UI_FLAG_DEFAULT, CHARACTER_CREATE_INFO_SEX );
	m_pInfoSex->CreateSubControl ();
	RegisterControl ( m_pInfoSex );

	m_pInfoClass = new CCharacterCreateInfoClass;
	m_pInfoClass->CreateSub( this, "CREATE_CHAR_CLASS_WINDOW", UI_FLAG_DEFAULT, CHARACTER_CREATE_INFO_CLASS );
	m_pInfoClass->CreateSubControl ();
	RegisterControl ( m_pInfoClass );
}

void CCharacterCreateInfo::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CCharacterCreateInfo::ResetAll()
{
	m_pInfoSchool->Reset();
	m_pInfoSex->Reset();
	m_pInfoClass->Reset();

	m_pInfoSchool->SetVisibleSingle( FALSE );
	m_pInfoSex->SetVisibleSingle( FALSE );
	m_pInfoClass->SetVisibleSingle( FALSE );
}

void CCharacterCreateInfo::SetData( int nSchool, int nSex, int nClass )
{
	if ( nSchool != m_nSchoolLast )
	{
		m_nSchoolLast = nSchool;
		m_pInfoSchool->SetData( nSchool );
		m_pInfoSchool->SetVisibleSingle( BOOL( nSchool >= 0 && nSchool < GLSCHOOL_NUM ) );
	}

	if ( nSex != m_nSexLast )
	{
		m_nSexLast = nSex;
		m_pInfoSex->SetData( nSex );
		m_pInfoSex->SetVisibleSingle( BOOL( nSex >= 0 && nSex < 2 ) );
	}

	if ( nClass != m_nClassLast )
	{
		m_nClassLast = nClass;
		m_pInfoClass->SetData( nClass );
		m_pInfoClass->SetVisibleSingle( BOOL( nClass >= 0 && nClass < GLCI_NUM_8CLASS ) );
	}
}