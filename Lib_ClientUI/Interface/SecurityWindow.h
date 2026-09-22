#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLPVPTyrannyData.h"

class CBasicTextBox;
class CBasicButton;

const DWORD UIMSG_SECURITY_WINDOW_LBDUP = UIMSG_USER1;

class CSecurityWindow : public CUIGroup
{
private:
	enum
	{
		SECURITY_WINDOW_BG = NO_ID + 1,
		EQUIPMENT_BUTTON_LOCK,
		EQUIPMENT_BUTTON_UNLOCK,
		EQUIPMENT_BUTTON_ACTIVATE,
		STORAGE_BUTTON_LOCK,
		STORAGE_BUTTON_UNLOCK,
		STORAGE_BUTTON_ACTIVATE,
		INVENTORY_BUTTON_LOCK,
		INVENTORY_BUTTON_UNLOCK,
		INVENTORY_BUTTON_ACTIVATE,
	};

public:
	CSecurityWindow ();
	virtual	~CSecurityWindow ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

public:
	void	CheckMouseState();
	void	CreateSubControl ();

private:
	CBasicButton*	CreateFlipButton(char* szButton, char* szButtonFlip, UIGUID ControlID, WORD wFlipType);

private:
	BOOL		m_bCHECK_MOUSE_STATE;
	BOOL		m_bFirstGap;
	D3DXVECTOR2	m_vGap;
	int			m_nPosX;
	int			m_nPosY;
	float		m_fUpdateTime;

	CBasicButton*		m_pEquipmentButtonLock;
	CBasicButton*		m_pEquipmentButtonUnlock;
	CBasicButton*		m_pEquipmentButtonActivate;

	CBasicButton*		m_pStorageButtonLock;
	CBasicButton*		m_pStorageButtonUnlock;
	CBasicButton*		m_pStorageButtonActivate;

	CBasicButton*		m_pInventoryButtonLock;
	CBasicButton*		m_pInventoryButtonUnlock;
	CBasicButton*		m_pInventoryButtonActivate;

	CUIControl*			m_pStorageMouseOver;
	CUIControl*			m_pEquipmentMouseOver;
	CUIControl*			m_pInventoryMouseOver;

public:
	void	UpdateInfo();

};