

#pragma	once

#include "./UIGroup.h"
#include "../../Lib_Client/G-Logic/GLCharData.h"
#include "../../Lib_Client/G-Logic/GLItemMan.h"

class GLGaeaClient;
class CBasicLineBox;
class CBasicTextBox;
class CItemImage;
class CSwapImage;

const DWORD UIMSG_PANDORA_BOX_SLOT_ITEM_IMAGE = UIMSG_USER1;

class	CPandoraBoxItemSlot : public CUIGroup
{
	enum
	{
		PANDORA_BOX_SLOT_ITEM_IMAGE = NO_ID + 1,
	};


public:
	CPandoraBoxItemSlot ( GLGaeaClient* pGaeaClient );
	virtual	~CPandoraBoxItemSlot ();

public:
	void	CreateSubControl ();

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

private:
	CUIControl*		m_pSelectImage;
	CBasicTextBox*	m_pTextName;
	CItemImage*		m_pItemImage;

private:
	GLPANDORA_BOX  m_sPandoraItem;
	GLGaeaClient* m_pGaeaClient;

public:
	void	ItemSet( GLPANDORA_BOX sShopItem );
	void	ItemReset();
	void	ItemSelect( BOOL bSelect );
	GLPANDORA_BOX	ItemGet()	{	return m_sPandoraItem;	}

};