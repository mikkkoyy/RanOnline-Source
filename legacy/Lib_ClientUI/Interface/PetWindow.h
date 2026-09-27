#pragma	once

#include "UIWindowEx.h"
#include "GLItem.h"

////////////////////////////////////////////////////////////////////
//	사용자 메시지 정의
const DWORD UIMSG_MOUSEIN_WEARSLOT = UIMSG_USER1;
////////////////////////////////////////////////////////////////////

class	CBasicProgressBar;
class	CBasicTextBox;
class	CItemImage;
class	CBasicComboBox;
class	CBasicComboBoxRollOver;
class	CPetQuickSkillSlot;
class	CPetQuickSkillSlot2; /*dual pet skill, Juver, 2017/12/29 */
class	CBasicButton;
struct	SITEMCUSTOM;

class	CPetWindowRender;

class	CPetWindow : public CUIWindowEx
{
	enum
	{
		SLOT_IMAGE_TYPE1 = ET_CONTROL_NEXT,
		SLOT_IMAGE_TYPE2,

		/*dual pet skill, Juver, 2017/12/29 */
		PET_SKILL_SLOT0,
		PET_SKILL_SLOT1,

		SATIETY_TEXT,
		LOYALTY_TEXT,
		
		/*dual pet skill, Juver, 2017/12/29 */
		SKILL_COMBO_OPEN0,
		SKILL_COMBO_OPEN1,
		SKILL_COMBO_ROLLOVER0,
		SKILL_COMBO_ROLLOVER1,

		PET_RENDER_BUTTON_LEFT,
		PET_RENDER_BUTTON_RIGHT,

		SKILL_BUTTON,

		SLOT_IMAGE_SIZE = 2
	};

	CBasicTextBox* m_pPetName;
	CBasicTextBox* m_pPetType;
	CUIControl*	m_pMouseOver;

	CUIControl*	m_pSlotDisplay[SLOT_IMAGE_SIZE];
	CItemImage*	m_pSlotImage[SLOT_IMAGE_SIZE];
	SITEMCUSTOM	m_ItemCustomArray[SLOT_IMAGE_SIZE];

	CBasicTextBox *	m_pSatietyText;
	CBasicTextBox *	m_pLoyaltyText;

	CBasicProgressBar * m_pSatiety;
	CBasicProgressBar * m_pLoyalty;

	/*dual pet skill, Juver, 2017/12/29 */
	CBasicComboBox * m_pcbSkill0;
	CBasicComboBox * m_pcbSkill1;
	CBasicComboBoxRollOver * m_pcbSkillRollOver0;
	CBasicComboBoxRollOver * m_pcbSkillRollOver1;

	CBasicButton*	m_pButtonLeft;
	CBasicButton*	m_pButtonRight;

	CPetWindowRender*	m_pRender;

	UIGUID	m_RollOverID;
	BOOL	m_bFirstLBUP;

public:
	CPetWindow ();
	virtual	~CPetWindow ();

public:
	void CreateSubControl ();

private:
	CBasicTextBox * CreateTextBox( char* szKeyword, CD3DFontPar* pFont );
	CUIControl*	CreateControl( const char* szControl );
	CItemImage*	CreateItemImage( const char* szControl, UIGUID ControlID );

public:
	virtual	void SetVisibleSingle ( BOOL bVisible );
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

public:
	void LoadItem ( int nIndex, SITEMCUSTOM& ref_sItemCustom );
	void UnLoadItem ( int nIndex );
	SITEMCUSTOM& GetItem ( int nIndex )			{ return m_ItemCustomArray[nIndex]; }

private:
	int m_nSatietyBack;
	WORD m_wLoyaltyBack;
	bool m_bDualSkillBack;

	void SetName( const std::string & strName );
	void SetType( const std::string & strType );
	void SetSatiety( int nNOW );
	//void SetLoyalty( WORD wNOW, WORD wMAX );

public:
	EMSUIT IMAGE2EMSLOT ( UIGUID ControlID );
	void SetSkillCombo();
};