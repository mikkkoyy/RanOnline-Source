#pragma	once
/*dmk14 reform*/

#include "UIWindowEx.h"
#include "GLDefine.h"
#include "GLItem.h"

class CItemImage;
class CBasicTextButton;
class CItemRebuildOption;

class	RnButton;

//--------------------------------------------------------------------
// ItemRebuild : √÷¡ÿ«ı
//--------------------------------------------------------------------
class CItemRebuild : public CUIWindowEx	// ITEMREBUILD_MARK
{
protected:
	enum
	{
		ITEM_REBUILD_ITEM_SLOT = ET_CONTROL_NEXT,
		ITEM_REBUILD_ITEM_SEAL_SLOT,
		ITEM_REBUILD_COAST_TEXT_STATIC,
		ITEM_REBUILD_COAST_TEXT,
		ITEM_REBUILD_MONEY_BACK,
		ITEM_REBUILD_MONEY_IMAGE,
		ITEM_REBUILD_MONEY_TEXT,
		ITEM_REBUILD_OK_BUTTON,
		ITEM_REBUILD_PREV_BUTTON,
		ITEM_REBUILD_CANCEL_BUTTON,
		ITEM_REBUILD_ITEM_OPTION00,
		ITEM_REBUILD_ITEM_OPTION01,
		ITEM_REBUILD_ITEM_OPTION02,
		ITEM_REBUILD_ITEM_OPTION03,
	};

	enum
	{
		MAX_OPTION = 4
	};


protected:
	CItemImage*		m_pItemImage;
	CItemImage*		m_pItemSealImage;
	CBasicTextBox*	m_pMoneyTextBox;
	//CBasicTextButton* m_pOkButton;
	//CBasicTextButton* m_pPrevButton;

	
    RnButton*		m_pOkButton;
    RnButton*		m_pPrevButton;
    RnButton*		m_pCancelButton;
	
    RnButton*		m_pOkBlankButton;
    RnButton*		m_pPrevBlankButton;
	//
	CBasicTextBox* m_pHelpTextBox;

	CBasicTextBox*		m_pOptionBefore[ SRANDOM_GEN::MAX_OPT ];
	CItemRebuildOption* m_pOption[ SRANDOM_GEN::MAX_OPT ];
	CItemRebuildOption*	m_pBasicOption;
	CItemRebuildOption*	m_pBasicOptionBefore;
	EMRANDOM_OPT		m_emLockOpt[ SRANDOM_GEN::MAX_OPT ];

public:
	CItemRebuild();
	virtual	~CItemRebuild();

	void Init( bool bClear );
	void ResetPrevDATA();
	EMRANDOM_OPT GetLockOpt( int nIndex )		{ return m_emLockOpt[ nIndex ]; }

private:
	int nNumSelOpt;
	int	m_wMaxRemodel;
	bool m_bSortOpt;
	SNATIVEID sRebuildBack;
	SNATIVEID sSealedBack;

public:
	VOID CreateSubControl();
	void SetBeforeData( SITEMCUSTOM sItemCust, int nIndex );
public:
	virtual	VOID Update( INT x, INT y, BYTE LB, BYTE MB, BYTE RB, INT nScroll, FLOAT fElapsedTime, BOOL bFirstControl );
	virtual	VOID TranslateUIMessage( UIGUID ControlID, DWORD dwMsg );

	CItemRebuildOption* CreateCheckBox ( CString strKeyword, UIGUID ControlID );
};