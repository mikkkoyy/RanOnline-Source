#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CCharacterCreatePageSet;
class CCharacterCreatePageStyle;
class CBasicTextButton;
class CBasicTextBox;

class CCharacterCreatePage : public CUIGroup
{
private:
	enum
	{
		CHARACTER_CREATE_PAGE_SET = NO_ID + 1,
		CHARACTER_CREATE_PAGE_STYLE,
		CHARACTER_CREATE_PAGE_BUTTON_CREATE_NEW,
		CHARACTER_CREATE_PAGE_BUTTON_SELECT_CHAR,
		CHARACTER_CREATE_PAGE_BUTTON_CHANGE_SERVER,
		CHARACTER_CREATE_PAGE_BUTTON_GAME_START,
	};

public:
	CCharacterCreatePage ();
	virtual	~CCharacterCreatePage ();

public:
	void CreateSubControl ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );


private:
	CCharacterCreatePageSet*	m_pPageSet;
	CCharacterCreatePageStyle*	m_pPageStyle;
	CBasicTextButton*	m_pNewButton;
	CBasicTextButton*	m_pSelectCharacterButton;
	CBasicTextButton*	m_pChangeServerButton;
	CBasicTextButton*	m_pGameStartButton;
	CBasicTextBox*		m_pServerName;

public:
	BOOL	m_bUseCancel;
	BOOL	m_bTypePage;
	int		m_nServerGroup;
	int		m_nServerChannel;

private:


public:
	void ToTypePage();
	void ToStylePage();
	void ResetAll();
	void ResetNameEdit();
	void SetUseCancel( BOOL bSet )	{ m_bUseCancel = bSet;	}
	void SetCreateExtremeM( BOOL bCreate );
	void SetCreateExtremeW( BOOL bCreate );
	void CreateCharacter();
	BOOL CheckStringName( CString strTemp );
	BOOL CheckValidCharacterSet();
};