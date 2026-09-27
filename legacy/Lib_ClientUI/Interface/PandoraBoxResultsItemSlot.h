

#pragma	once

#include "./UIGroup.h"
#include "../../Lib_Client/G-Logic/GLCharData.h"
#include "../../Lib_Client/G-Logic/GLItem.h"
#include "../../Lib_Client/G-Logic/GLItemMan.h"

class GLGaeaClient;
class CBasicLineBox;
class CBasicTextBox;
class CItemImage;
class CSwapImage;
class CBasicButton;
class CBasicTextButton;

const DWORD UIMSG_PANDORA_BOX_RESULT_SLOT_ITEM_IMAGE = UIMSG_USER1;
const DWORD UIMSG_PANDORA_BOX_RESULT_SLOT_ITEM_BUY = UIMSG_USER4;
const DWORD UIMSG_PANDORA_BOX_RESULT_SLOT_ITEM_REMOVE = UIMSG_USER2;

class	CPandoraBoxResultsItemSlot : public CUIGroup
{
private:
	static	const	float	fBLINK_TIME_LOOP;
	enum
	{
		PANDORA_BOX_SLOT_ITEM_IMAGE = NO_ID + 1,
		PANDORA_BOX_SLOT_ITEM_BUY,
		PANDORA_BOX_SLOT_ITEM_REMOVE
	};


public:
	CPandoraBoxResultsItemSlot (GLGaeaClient* pGaeaClient );
	virtual	~CPandoraBoxResultsItemSlot ();

public:
	void	CreateSubControl ();

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void	Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

private:
	CUIControl*		m_pSelectImage;
	CBasicTextBox*	m_pTextName;
	CBasicTextBox*	m_pTextPrice;
	CItemImage*		m_pItemImage;
	CBasicTextButton*	m_pBuyButton;
	CBasicButton*	m_pRemoveButton;

	bool m_bAlarm;
	float m_fBLINK_TIME;

private:
	GLPANDORA_BOX  m_sPandoraItem;
	GLGaeaClient* m_pGaeaClient;

public:
	void	ItemSet( GLPANDORA_BOX sShopItem );
	void	ItemReset();
	void	ItemSelect( BOOL bSelect );
	GLPANDORA_BOX	ItemGet()	{	return m_sPandoraItem;	}

};