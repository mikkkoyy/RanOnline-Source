#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "../Lib_Client/G-Logic/GLCharDefine.h"

class CBasicTextBox;

class CCharacterCreateInfoSchool : public CUIGroup
{
public:
	CCharacterCreateInfoSchool ();
	virtual	~CCharacterCreateInfoSchool ();

public:
	void	CreateSubControl ();

private:
	CUIControl*		m_pLogo[GLSCHOOL_NUM];
	CBasicTextBox*	m_pInfoTitle;
	CBasicTextBox*	m_pInfoDesc;

public:
	void	Reset();
	void	SetData( int nSchool );
};