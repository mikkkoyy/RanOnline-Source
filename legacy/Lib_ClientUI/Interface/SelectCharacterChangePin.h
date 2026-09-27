#pragma	once

#include "UIOuterWindow.h"

class CUIEditBoxMan;

class CSelectCharacterChangePin : public CUIOuterWindow
{
enum
	{
		SELCHAR_BUTTON_OK = ET_CONTROL_NEXT,
		SELCHAR_BUTTON_CANCEL,
		SELCHAR_EDIT_MAN,
		SELCHAR_EDIT_PW,
		SELCHAR_EDIT_PIN,
		SELCHAR_EDIT_NEWPIN,
		SELCHAR_EDIT_NEWPIN2,
		SELCHAR_EDIT_CAPTCHA,
	};

public:
	static int nLIMIT_ID;
	static int nLIMIT_PW;

public:
	CSelectCharacterChangePin();
	virtual	~CSelectCharacterChangePin();

public:
	virtual void Update( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage( UIGUID ControlID, DWORD dwMsg );


public:
	void CreateSubControl();
	CBasicButton * CreateFlipButton( char* szButton, char* szButtonFlip, UIGUID ControlID );

	void ResetAll( bool bEdit = false );

public:
	BOOL	CheckString( CString strTemp );

private:
	CUIEditBoxMan*	m_pEditBoxMan;
	CBasicTextBox*	pTextBoxCaptcha;
	CBasicTextBox*	pTextBoxLvlPin;

	int nLIMIT_CAPTCHA;
	std::string	strCaptcha;

};