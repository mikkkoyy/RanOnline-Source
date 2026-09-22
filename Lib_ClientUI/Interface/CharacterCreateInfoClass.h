#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "../Lib_Client/G-Logic/GLCharDefine.h"

class CBasicTextBox;

class CCharacterCreateInfoClass : public CUIGroup
{
public:
	CCharacterCreateInfoClass ();
	virtual	~CCharacterCreateInfoClass ();

public:
	void	CreateSubControl ();

private:
	CUIControl*		m_pLogo[GLCI_NUM_8CLASS];
	CBasicTextBox*	m_pInfoTitle;
	CBasicTextBox*	m_pInfoDesc;

public:
	void	Reset();
	void	SetData( int nClass );
};