/*!
 * \file RegisterPage.h
 *
 * \author Juver
 * \date November 2017
 *
 * 
 */

#pragma	once

#include "UIOuterWindow.h"

class CUIEditBoxMan;
class CBasicTextBox;
class CBasicTextButton;

class CRegisterPage : public CUIOuterWindow
{
	enum
	{
		LOBBY_REGISTER_EDIT_MAN = ET_CONTROL_NEXT,
		LOBBY_REGISTER_EDIT_USER,
		LOBBY_REGISTER_EDIT_PASS,
		LOBBY_REGISTER_EDIT_PASS2,
		LOBBY_REGISTER_EDIT_PIN,
		LOBBY_REGISTER_EDIT_PIN2,
		LOBBY_REGISTER_EDIT_MAIL,
		LOBBY_REGISTER_EDIT_SA,
		LOBBY_REGISTER_EDIT_SA2,
		LOBBY_REGISTER_EDIT_CAPTCHA,

		LOBBY_REGISTER_BUTTON_PASS,
		LOBBY_REGISTER_BUTTON_PASS2,
		LOBBY_REGISTER_BUTTON_PIN,
		LOBBY_REGISTER_BUTTON_PIN2,
		LOBBY_REGISTER_BUTTON_SA,
		LOBBY_REGISTER_BUTTON_SA2,

		LOBBY_REGISTER_BUTTON_REGISTER,
		LOBBY_REGISTER_BUTTON_CANCEL,
		LOBBY_REGISTER_BUTTON_RESET,
		
	};

public:
	static int nLIMIT_USER;
	static int nLIMIT_PASS;
	static int nLIMIT_MAIL;
	static int nLIMIT_SA;
	static int nLIMIT_CAPTCHA;
	static int nMIN_TEXT;

public:
	CRegisterPage();
	virtual	~CRegisterPage();

public:
	virtual void Update( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage( UIGUID ControlID, DWORD dwMsg );

	virtual	void SetVisibleSingle( BOOL bVisible );

public:
	void CreateSubControl();

public:
	CUIEditBoxMan*		m_pEditBoxMan;

	CBasicTextBox*		m_pInfoUser;
	CBasicTextBox*		m_pInfoPass;
	CBasicTextBox*		m_pInfoPass2;
	CBasicTextBox*		m_pInfoPin;
	CBasicTextBox*		m_pInfoPin2;
	CBasicTextBox*		m_pInfoMail;
	CBasicTextBox*		m_pInfoSA;
	CBasicTextBox*		m_pInfoSA2;
	CBasicTextBox*		m_pInfoCaptcha;

	CBasicTextButton*	m_pButtonPass;
	CBasicTextButton*	m_pButtonPass2;
	CBasicTextButton*	m_pButtonPin;
	CBasicTextButton*	m_pButtonPin2;
	CBasicTextButton*	m_pButtonSA;
	CBasicTextButton*	m_pButtonSA2;

	CBasicTextBox*		m_pTextCaptcha;
	CBasicTextBox*		m_pTextStatus;

	

public:
	BOOL	m_bShowPass;
	BOOL	m_bShowPass2;
	BOOL	m_bShowPin;
	BOOL	m_bShowPin2;
	BOOL	m_bShowSA;
	BOOL	m_bShowSA2;
	std::string m_strCaptcha;

public:
	void	ResetAll();
	void	DoRegister();
	void	EndRegister();
	BOOL	CheckString( CString strTemp );
	BOOL	CheckString_Special2( CString strTemp );

	void	CheckHideStatusPass();
	void	CheckHideStatusPass2();
	void	CheckHideStatusPin();
	void	CheckHideStatusPin2();
	void	CheckHideStatusSA();
	void	CheckHideStatusSA2();

	void	AddErrorString( std::string strError, DWORD dwColor );

};