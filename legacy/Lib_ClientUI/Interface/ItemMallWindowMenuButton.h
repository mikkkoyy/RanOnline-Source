#pragma	once

#include "BasicTextBox.h"
#include "UIWindowEx.h"
#include "GLCharDefine.h"

class CItemMallWindowMenuButton : public CUIGroup
{
protected:
	enum 
	{
		ICON_OFF = NO_ID + 1,
		ICON_ON,
	};
public:
	CItemMallWindowMenuButton ();
	virtual	~CItemMallWindowMenuButton ();
	void CreateSubControl( CString strKeywordOff, CString strKeywordOn );
	virtual void TranslateUIMessage( UIGUID ControlID, DWORD dwMsg );

private:
	CUIControl*		m_pIcon00;
	CUIControl*		m_pIcon01;

public:
	void SetOn() { m_pIcon01->SetVisibleSingle( TRUE ); }
	void SetOff() { m_pIcon01->SetVisibleSingle( FALSE ); }
};