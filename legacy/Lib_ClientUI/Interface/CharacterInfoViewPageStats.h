/*!
 * \file CharacterInfoViewPageStats.h
 *
 * \author Juver
 * \date November 2017
 *
 * 
 */

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLCharData.h"

class	CBasicTextBox;
class	CBasicTextButton;
class	CItemImage;
class	CBasicButton;
class	CCharacterInfoViewPageStatsRender;
class	CCharacterInfoViewPageStatsMark;

class CCharacterInfoViewPageStats : public CUIGroup
{
public:
	enum
	{
		CHARACTER_VIEW_INFO_ITEM_IMAGE0 = NO_ID + 1,
		CHARACTER_VIEW_INFO_ITEM_IMAGE1,
		CHARACTER_VIEW_INFO_ITEM_IMAGE2,
		CHARACTER_VIEW_INFO_ITEM_IMAGE3,
		CHARACTER_VIEW_INFO_ITEM_IMAGE4,
		CHARACTER_VIEW_INFO_ITEM_IMAGE5,
		CHARACTER_VIEW_INFO_ITEM_IMAGE6,
		CHARACTER_VIEW_INFO_ITEM_IMAGE7,
		CHARACTER_VIEW_INFO_ITEM_IMAGE8,
		CHARACTER_VIEW_INFO_ITEM_IMAGE9,
		CHARACTER_VIEW_INFO_ITEM_IMAGE10,
		CHARACTER_VIEW_INFO_ITEM_IMAGE11,
		CHARACTER_VIEW_INFO_ITEM_IMAGE12,
		CHARACTER_VIEW_INFO_ITEM_IMAGE13,
		CHARACTER_VIEW_INFO_ITEM_IMAGE14,
		CHARACTER_VIEW_INFO_ITEM_IMAGE15,
		CHARACTER_VIEW_INFO_BUTTON_WHISPER,
		CHARACTER_VIEW_INFO_BUTTON_ADDFRIEND,
		CHARACTER_VIEW_INFO_BUTTON_INVITEPARTY,
		CHARACTER_VIEW_INFO_BUTTON_REFRESH,
		CHARACTER_VIEW_INFO_BUTTON_LEFT,
		CHARACTER_VIEW_INFO_BUTTON_RIGHT,
	};

	enum
	{
		INFO_ITEM_IMAGE_SIZE = 16,
		INFO_ICON_SCHOOL = 3,
		INFO_ICON_CLASS = GLCI_NUM_8CLASS,
	};

public:
	CCharacterInfoViewPageStats ();
	virtual	~CCharacterInfoViewPageStats ();

public:
	void	CreateSubControl ();

public:
	CBasicTextBox*	m_pLevelText;

	CBasicTextBox*	m_pPowText;
	CBasicTextBox*	m_pDexText;
	CBasicTextBox*	m_pSpiText;
	CBasicTextBox*	m_pStrText;
	CBasicTextBox*	m_pStaText;

	CBasicTextBox*	m_pAttackMinText;
	CBasicTextBox*	m_pAttackMaxText;
	CBasicTextBox*	m_pDefenseText;
	CBasicTextBox*	m_pFightText;
	CBasicTextBox*	m_pShootText;
	CBasicTextBox*	m_pEnergyText;
	CBasicTextBox*	m_pHitText;
	CBasicTextBox*	m_pAvoidText;
	CBasicTextBox*	m_pHPText;
	CBasicTextBox*	m_pMPText;
	CBasicTextBox*	m_pSPText;

	CBasicTextBox*	m_pFireText;
	CBasicTextBox*	m_pColdText;
	CBasicTextBox*	m_pElectricText;
	CBasicTextBox*	m_pPoisonText;
	CBasicTextBox*	m_pSpiritText;

	CBasicTextBox*	m_pLevelText_Up;

	CBasicTextBox*	m_pPowText_Up;
	CBasicTextBox*	m_pDexText_Up;
	CBasicTextBox*	m_pSpiText_Up;
	CBasicTextBox*	m_pStrText_Up;
	CBasicTextBox*	m_pStaText_Up;

	CBasicTextBox*	m_pAttackMinText_Up;
	CBasicTextBox*	m_pAttackMaxText_Up;
	CBasicTextBox*	m_pDefenseText_Up;
	CBasicTextBox*	m_pFightText_Up;
	CBasicTextBox*	m_pShootText_Up;
	CBasicTextBox*	m_pEnergyText_Up;
	CBasicTextBox*	m_pHitText_Up;
	CBasicTextBox*	m_pAvoidText_Up;
	CBasicTextBox*	m_pHPText_Up;
	CBasicTextBox*	m_pMPText_Up;
	CBasicTextBox*	m_pSPText_Up;

	CBasicTextBox*	m_pFireText_Up;
	CBasicTextBox*	m_pColdText_Up;
	CBasicTextBox*	m_pElectricText_Up;
	CBasicTextBox*	m_pPoisonText_Up;
	CBasicTextBox*	m_pSpiritText_Up;

	CCharacterInfoViewPageStatsMark*	m_pLevelText_Mark;

	CCharacterInfoViewPageStatsMark*	m_pPowText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pDexText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pSpiText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pStrText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pStaText_Mark;

	CCharacterInfoViewPageStatsMark*	m_pAttackMinText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pAttackMaxText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pDefenseText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pFightText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pShootText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pEnergyText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pHitText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pAvoidText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pHPText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pMPText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pSPText_Mark;

	CCharacterInfoViewPageStatsMark*	m_pFireText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pColdText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pElectricText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pPoisonText_Mark;
	CCharacterInfoViewPageStatsMark*	m_pSpiritText_Mark;


	CBasicTextBox*	m_pBadgeText;
	CBasicTextBox*	m_pNameText;
	CBasicTextBox*	m_pClassText;
	CBasicTextBox*	m_pSchoolText;
	CBasicTextBox*	m_pGuildText;

	CUIControl*		m_pClassIcon[INFO_ICON_CLASS];
	CUIControl*		m_pSchoolIcon[INFO_ICON_SCHOOL];

	CBasicTextButton*		m_pButtonWhisper;
	CBasicTextButton*		m_pButtonAddFriend;
	CBasicTextButton*		m_pButtonInviteParty;
	CBasicTextButton*		m_pButtonRefresh;

	CItemImage*		m_pItemImage[INFO_ITEM_IMAGE_SIZE];

	CCharacterInfoViewPageStatsRender*		m_pRender;

	CBasicButton*		m_pButtonLeft;
	CBasicButton*		m_pButtonRight;

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

public:
	SINFO_CHAR		m_sInfoChar;

	void	SetCharacterData( SINFO_CHAR &sInfoChar );
	void	DataReset();
	EMSLOT	IMAGE2EMSLOT ( int nIndex );
	void	DoStatComparison( int nVAL1, int nVAL2, CBasicTextBox* pTextBox, CCharacterInfoViewPageStatsMark* pMark );
};