#pragma	once

#include "UIWindowEx.h"
#include "GLItem.h"
#include "GLItemDef.h"

class	CItemPreviewWindowRender;
class	CItemImage;
class	CBasicTextBox;

class	CItemPreviewWindow : public CUIWindowEx
{
private:
	enum
	{
		ITEM_PREVIEW_BUTTON_LEFT = ET_CONTROL_NEXT,
		ITEM_PREVIEW_BUTTON_RIGHT,
		ITEM_PREVIEW_ITEMSLOT_HEAD,
		ITEM_PREVIEW_ITEMSLOT_UPPER,
		ITEM_PREVIEW_ITEMSLOT_LOWER,
		ITEM_PREVIEW_ITEMSLOT_HAND,
		ITEM_PREVIEW_ITEMSLOT_FOOT,
		ITEM_PREVIEW_ITEMSLOT_WEAPON,
		ITEM_PREVIEW_ITEMSLOT_WAIST,
		ITEM_PREVIEW_ITEMSLOT_WRIST,
		ITEM_PREVIEW_ITEMSLOT_NECK,
		ITEM_PREVIEW_ITEMSLOT_FACE,
		ITEM_PREVIEW_ITEMSLOT_MISC,
		ITEM_PREVIEW_STANCE_BUTTON,
	};

public:
	CItemPreviewWindow ();
	virtual	~CItemPreviewWindow ();

public:
	void	CreateSubControl ();

private:
	CItemPreviewWindowRender*	m_pRender;
	CItemImage*			m_pSlotItem[EMITEM_PREVIEW_SLOT_TOTAL];
	CBasicTextBox*		m_pSlotText[EMITEM_PREVIEW_SLOT_TOTAL];
	CBasicButton*		m_pButtonLeft;
	CBasicButton*		m_pButtonRight;

	CBasicButton*		m_pStanceButton;
	CBasicTextBox*		m_pStanceText;

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

private:

public:
	void	ClearRender();
	void	StartPreview( SITEMCUSTOM sItem );
};