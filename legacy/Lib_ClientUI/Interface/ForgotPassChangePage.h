#pragma	once

#include "UIOuterWindow.h"

class CUIEditBoxMan;

class CForgotPassChangePage : public CUIOuterWindow
{
enum
	{
		CHANGEPASS_OK = ET_CONTROL_NEXT,
		CHANGEPASS_CANCEL,
		CHANGEPASS_EDIT_MAN,
		CHANGEPASS_EDIT_PW1,
		CHANGEPASS_EDIT_PW2,
		CHANGEPASS_EDIT_PIN,
		CHANGEPASS_EDIT_CAPTCHA,
	};

public:
	static int nLIMIT_PW;

public:
	CForgotPassChangePage();
	virtual	~CForgotPassChangePage();

public:
	virtual void Update( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage( UIGUID ControlID, DWORD dwMsg );


public:
	void CreateSubControl();
	CBasicButton * CreateFlipButton( char* szButton, char* szButtonFlip, UIGUID ControlID );

	void ResetAll( bool bEdit = false );

private:
	BOOL CheckString( CString strTemp );

private:
	CUIEditBoxMan * m_pEditBoxMan;
	CBasicTextBox*	pTextBoxCaptcha;

	int nLIMIT_CAPTCHA;
	std::string	strCaptcha;
	std::string strKey;

};