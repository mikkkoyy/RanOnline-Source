#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CBasicButton;

class CCharacterCreateRotate : public CUIGroup
{
private:
	enum
	{
		CHARACTER_CREATE_ROTATE_LEFT = NO_ID + 1,
		CHARACTER_CREATE_ROTATE_RIGHT,
	};

public:
	CCharacterCreateRotate ();
	virtual	~CCharacterCreateRotate ();

public:
	void CreateSubControl ();

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

private:
	CBasicButton*	m_pRotateButtonLeft;
	CBasicButton*	m_pRotateButtonRight;

};