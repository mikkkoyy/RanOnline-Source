#pragma	once

#include "UIOuterWindow.h"

class CUIEditBoxMan;

class CForgotPassPage : public CUIOuterWindow
{
enum
	{
		FORGOT_OK = ET_CONTROL_NEXT,
		FORGOT_CANCEL,
		FORGOT_EDIT_MAN,
		FORGOT_EDIT_ID,
		FORGOT_EDIT_PW,
		FORGOT_EDIT_CAPTCHA,
	};

public:
	static int nLIMIT_ID;
	static int nLIMIT_PW;

public:
	CForgotPassPage();
	virtual	~CForgotPassPage();

public:
	virtual void Update( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage( UIGUID ControlID, DWORD dwMsg );


public:
	void CreateSubControl();
	CBasicButton * CreateFlipButton( char* szButton, char* szButtonFlip, UIGUID ControlID );

	void ResetAll( bool bEdit = false );
	void SetKey ( std::string key )		{ strKey = key; }

	std::string GetKey ()			{ return strKey; }
	CString GetUser();

private:
	BOOL CheckString( CString strTemp );

private:
	CUIEditBoxMan * m_pEditBoxMan;
	CBasicTextBox*	pTextBoxCaptcha;

	int nLIMIT_CAPTCHA;
	std::string	strCaptcha;
	std::string strKey;

};