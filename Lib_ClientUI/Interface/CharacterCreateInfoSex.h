#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CBasicTextBox;

class CCharacterCreateInfoSex : public CUIGroup
{
public:
	CCharacterCreateInfoSex ();
	virtual	~CCharacterCreateInfoSex ();

public:
	void	CreateSubControl ();

private:
	CUIControl*		m_pLogo[2];
	CBasicTextBox*	m_pInfoTitle;

public:
	void	Reset();
	void	SetData( int nSex );
};