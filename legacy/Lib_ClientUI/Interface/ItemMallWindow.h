#pragma	once

#include "UIWindowEx.h"
#include "GLDefine.h"
#include "GLCrowData.h"
#include "BasicLineBox.h"

class	CBasicTextButton;
class	CItemMallWindowMenu;
class	CItemMallWindowMenuPage;
class	CBasicTextBox;

class	CItemMallWindow : public CUIWindowEx
{
protected:
	enum
	{
		ITEMMALL_WINDOW_PAGE,
		ITEMMALL_WINDOW_MENU
	};

public:
	CItemMallWindow ();
	virtual	~CItemMallWindow ();

public:
	void	CreateSubControl ();

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

public:
	void		InitItemMall ();
	CItemMallWindowMenuPage*	GetMenuPage()		{ return m_pPage; }

protected:
	CBasicTextButton*	CreateTextButton ( const char* szButton, UIGUID ControlID, const char* szText );
	void		SetMoney( DWORD dwPrem, DWORD dwCombat );

private:
	CItemMallWindowMenu*		m_pMenu;
	CItemMallWindowMenuPage*	m_pPage;
	CBasicTextBox*				m_pTextBox;
	WORD						m_wType;
	int							m_nType;
};