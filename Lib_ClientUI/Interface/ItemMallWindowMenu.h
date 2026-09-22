#pragma	once

#include "BasicTextBox.h"
#include "UIWindowEx.h"
#include "GLCharDefine.h"

class	CItemMallWindowMenuButton;
class	CBasicComboBox;
class	CBasicComboBoxRollOver;

class CItemMallWindowMenu : public CUIGroup
{
protected:
	enum 
	{
		MENU_00 = NO_ID + 1,
		MENU_01,
		MENU_02,
		MENU_03,
		MENU_04,
		MENU_05,
		MENU_06,
		MENU_07,
		MENU_08,
		MENU_09,
		MENU_10,
		MENU_11,
		MENU_12,

		ITEMMALL_COMBO_ROLLOVER,
		ITEMMALL_COMBO_OPEN,
	};
public:
	CItemMallWindowMenu ();
	virtual	~CItemMallWindowMenu ();
	void CreateSubControl();
	virtual void TranslateUIMessage( UIGUID ControlID, DWORD dwMsg );

private:
	CItemMallWindowMenuButton*				m_pMenuButton[13];
	CItemMallWindowMenuButton*				CreateMenu ( CString strKeyword, UIGUID ControlID, CString strOn, CString strOff );

	CBasicComboBox*				m_pComboBoxTypeOpen;
	CBasicComboBoxRollOver*		m_pComboBoxTypeRollOver;

	WORD									m_wType;
	int										m_nType;

public:
	void									SetType( WORD wType ) { m_wType = wType; }
	WORD									GetType() { return m_wType; }

	void									SetItemType( int nType )	{ m_nType = nType; }
	int										GetItemType() { return m_nType; }
};