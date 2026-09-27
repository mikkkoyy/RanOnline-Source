#include "StdAfx.h"
#include "PVPTyrannyPageHistoryBuff.h"

#include "SkillImage33.h"
#include "BasicLineBox.h"

#include "UITextControl.h"

#include "GLSkill.h"
#include "GLPVPTyrannyClient.h"
#include "InnerInterface.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPVPTyrannyPageHistoryBuff::CPVPTyrannyPageHistoryBuff ()
	: m_fUpdateTime(0.0f)
{
	for ( int i=0; i<TYRANNY_REWARD_BUFF_NUM; ++i )
	{
		m_pSkillLineBox[i] = NULL;
		m_pSkillImage[i] = NULL;
	}
}

CPVPTyrannyPageHistoryBuff::~CPVPTyrannyPageHistoryBuff ()
{
}

void CPVPTyrannyPageHistoryBuff::CreateSubControl ()
{
	std::string strLineBox[TYRANNY_REWARD_BUFF_NUM] = 
	{
		"COMPETITION_CTF_PAGE_HISTORY_REWARDBUFF_LINEBOX0",
		"COMPETITION_CTF_PAGE_HISTORY_REWARDBUFF_LINEBOX1",
		"COMPETITION_CTF_PAGE_HISTORY_REWARDBUFF_LINEBOX2",
		"COMPETITION_CTF_PAGE_HISTORY_REWARDBUFF_LINEBOX3",
	};

	std::string strSkillImage[TYRANNY_REWARD_BUFF_NUM] = 
	{
		"COMPETITION_CTF_PAGE_HISTORY_REWARDBUFF_ITEM0",
		"COMPETITION_CTF_PAGE_HISTORY_REWARDBUFF_ITEM1",
		"COMPETITION_CTF_PAGE_HISTORY_REWARDBUFF_ITEM2",
		"COMPETITION_CTF_PAGE_HISTORY_REWARDBUFF_ITEM3",
	};

	for ( int i=0; i<TYRANNY_REWARD_BUFF_NUM; ++i )
	{
		m_pSkillLineBox[i] = new CBasicLineBox;
		m_pSkillLineBox[i]->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		m_pSkillLineBox[i]->CreateBaseBoxWhiteBlankBody ( (char*)strLineBox[i].c_str() );
		RegisterControl ( m_pSkillLineBox[i] );

		m_pSkillImage[i] = new CSkillImage33;
		m_pSkillImage[i]->CreateSub ( this, strSkillImage[i].c_str(), UI_FLAG_DEFAULT, TYRANNY_PAGE_HISTORY_BUFF_SLOT0 + i  );	
		m_pSkillImage[i]->SetUseRender ( TRUE );
		m_pSkillImage[i]->SetVisibleSingle( FALSE );
		RegisterControl ( m_pSkillImage[i] );
	}	
}

void CPVPTyrannyPageHistoryBuff::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case TYRANNY_PAGE_HISTORY_BUFF_SLOT0:
	case TYRANNY_PAGE_HISTORY_BUFF_SLOT1:
	case TYRANNY_PAGE_HISTORY_BUFF_SLOT2:
	case TYRANNY_PAGE_HISTORY_BUFF_SLOT3:
		{
			if( CHECK_MOUSE_IN( dwMsg ) )
			{
				int nSelect = ControlID - TYRANNY_PAGE_HISTORY_BUFF_SLOT0;
				if ( nSelect < TYRANNY_REWARD_BUFF_NUM )
				{
					SNATIVEID sidSkill = GLPVPTyrannyClient::GetInstance().m_sRewardBuff[nSelect];
					PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( sidSkill );
					if ( pSkill )
					{
						CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( pSkill->GetName(), NS_UITEXTCOLOR::WHITE );
					}
				}
			}
		}break;
	};
}

void CPVPTyrannyPageHistoryBuff::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_fUpdateTime += fElapsedTime;
	if ( m_fUpdateTime >= 0.5f )
	{
		ShowInfo();
		m_fUpdateTime = 0.0f;
	}
}

void CPVPTyrannyPageHistoryBuff::ShowInfo()
{
	for ( int i=0; i<TYRANNY_REWARD_BUFF_NUM; ++i )
	{
		SNATIVEID sidSkill = GLPVPTyrannyClient::GetInstance().m_sRewardBuff[i];
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( sidSkill );

		if ( pSkill )
		{
			if ( m_pSkillLineBox[i] )	
				m_pSkillLineBox[i]->SetVisibleSingle( TRUE );

			if ( m_pSkillImage[i] )	
			{
				m_pSkillImage[i]->SetSkill( sidSkill );
				m_pSkillImage[i]->SetVisibleSingle( TRUE );
			}
		}
		else
		{
			if ( m_pSkillLineBox[i] )	
				m_pSkillLineBox[i]->SetVisibleSingle( FALSE );

			if ( m_pSkillImage[i] )
			{
				m_pSkillImage[i]->ResetSkill();
				m_pSkillImage[i]->SetVisibleSingle( FALSE );
			}
		}
	}
}