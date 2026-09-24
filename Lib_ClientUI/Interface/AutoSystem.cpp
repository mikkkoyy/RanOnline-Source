#include "StdAfx.h"
#include "./BasicTextButton.h"
#include "./BasicLineBox.h"
#include "InnerInterface.h"
#include "AutoSystem.h"
#include "./GameTextControl.h"
#include "./UITextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAutoSystem::CAutoSystem(void)
	: m_pBackground(NULL)
	, m_pAutoCombatButton(NULL)
	, m_pAutoPotLootButton(NULL)
{
}

CAutoSystem::~CAutoSystem(void)
{
}

void CAutoSystem::CreateSubControl()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont(_DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG);

	m_pBackground = new CBasicLineBox;
	m_pBackground->CreateSub(this, "BASIC_LINE_BOX_MAPMOVE", UI_FLAG_XSIZE | UI_FLAG_YSIZE);
	m_pBackground->CreateBaseBoxMapMove("AUTOSYSTEM_BACKGROUND");
	RegisterControl(m_pBackground);

	m_pAutoCombatButton = new CBasicTextButton;
	m_pAutoCombatButton->CreateSub(this, "BASIC_TEXT_BUTTON14", UI_FLAG_XSIZE, AUTO_COMBAT_BUTTON);
	m_pAutoCombatButton->CreateBaseButton("AUTOSYSTEM_BUTTON", CBasicTextButton::SIZE14, CBasicButton::CLICK_FLIP, "AUTO COMBAT");
	m_pAutoCombatButton->SetGeneralButton();
	RegisterControl(m_pAutoCombatButton);

	m_pAutoPotLootButton = new CBasicTextButton;
	m_pAutoPotLootButton->CreateSub(this, "BASIC_TEXT_BUTTON14", UI_FLAG_XSIZE, AUTO_POT_LOOT_BUTTON);
	m_pAutoPotLootButton->CreateBaseButton("AUTOSYSTEM_BUTTON", CBasicTextButton::SIZE14, CBasicButton::CLICK_FLIP, "AUTO POT + LOOT");
	m_pAutoPotLootButton->SetGeneralButton();
	RegisterControl(m_pAutoPotLootButton);
}

void CAutoSystem::TranslateUIMessage(UIGUID ControlID, DWORD dwMsg)
{
	switch (ControlID)
	{
	case AUTO_COMBAT_BUTTON:
	{
		if (CHECK_MOUSEIN_LBUPLIKE(dwMsg))
		{
			CInnerInterface::GetInstance().ShowCommonLineInfo("AUTO COMBAT", NS_UITEXTCOLOR::WHITE);
		}
	}
	break;

	case AUTO_POT_LOOT_BUTTON:
	{
		if (CHECK_MOUSEIN_LBUPLIKE(dwMsg))
		{
			CInnerInterface::GetInstance().ShowCommonLineInfo("AUTO POT + LOOT", NS_UITEXTCOLOR::WHITE);
		}
	}
	break;
	}

	CUIGroup::TranslateUIMessage(ControlID, dwMsg);
}

void CAutoSystem::Update(int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl)
{
	if (!IsVisible()) return;

	CUIGroup::Update(x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl);
}
