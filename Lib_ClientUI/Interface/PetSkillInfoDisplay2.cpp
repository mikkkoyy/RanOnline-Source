#include "StdAfx.h"
#include "PetSkillInfoDisplay2.h"
#include "InnerInterface.h"
#include "UITextControl.h"
#include "GLGaeaClient.h"

#include "PetSkillImage.h"
#include "GLGaeaClient.h"
#include "GLSkill.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPetSkillInfoDisplay2::CPetSkillInfoDisplay2 () 
	: m_pSkillImage ( NULL )
	, m_strSkillName("")
	, m_fUpdateTime(0.0f)
{
}

CPetSkillInfoDisplay2::~CPetSkillInfoDisplay2 ()
{
}

void  CPetSkillInfoDisplay2::CreateSubControl ()
{
	m_pSkillImage = new CPetSkillImage;
	m_pSkillImage->CreateSub ( this, "PET_SKILL_INFO_DISPLAY_IMAGE" );
	m_pSkillImage->CreateSubControl ();
	m_pSkillImage->SetUseRender ( TRUE );
	m_pSkillImage->SetVisibleSingle ( FALSE );
	RegisterControl ( m_pSkillImage );

	CUIControl* pDummyControl = new CUIControl;
	pDummyControl->CreateSub ( this, "PET_SKILL_INFO_DISPLAY_DUMMY");
	RegisterControl ( pDummyControl );
	m_pDummyControl = pDummyControl;
}

void CPetSkillInfoDisplay2::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
	
	m_fUpdateTime += fElapsedTime;
	if ( m_fUpdateTime >= PET_SKILL_INFO2_DISPLAY_UPDATE_TIME )
	{
		m_fUpdateTime = 0.0f;

		SNATIVEID sNativeID = GLGaeaClient::GetInstance().GetPetClient()->m_sActiveSkillID_B;
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sNativeID.wMainID, sNativeID.wSubID );
		if ( pSkill )
		{	
			m_pSkillImage->SetSkill ( sNativeID );
			m_pSkillImage->SetVisibleSingle ( TRUE );
			m_pDummyControl->SetVisibleSingle ( TRUE );
			m_pSkillImage->SetSkillProcess ( sNativeID );
			m_strSkillName = pSkill->GetName();
		}
		else
		{
			m_pSkillImage->SetVisibleSingle ( FALSE );
			m_pDummyControl->SetVisibleSingle ( FALSE );
			m_pSkillImage->ResetSkill ();
			m_strSkillName = "";
		}
	}

	if ( CHECK_MOUSE_IN ( GetMessageEx () ) && m_strSkillName.size() )
	{
		CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO ( m_strSkillName.c_str(), NS_UITEXTCOLOR::GREENYELLOW );
	}
}