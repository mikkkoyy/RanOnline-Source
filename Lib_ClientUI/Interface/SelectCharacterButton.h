#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CBasicTextBox;
class CBasicTextButton;
struct SCHARINFO_LOBBY;

/*dmk14 ingame web*/
class CBasicLineBox;

class CSelectCharacterButton : public CUIGroup
{
private:
	enum
	{
		SELECT_CHARACTER_NEW = NO_ID + 1,
		SELECT_CHARACTER_DELETE,
		SELECT_CHARACTER_CHANGESERVER,
		SELECT_CHARACTER_GAMESTART,
		/*dmk14 ingame web*/
		SELECT_CHARACTER_CHANGEPASS,
		SELECT_CHARACTER_CHANGEPIN,
		SELECT_CHARACTER_RESETPIN,
		SELECT_CHARACTER_TOPUP,
		SELECT_CHARACTER_CLAIM_GAMETIME,
		SELECT_CHARACTER_CHANGEMAIL,
	};

	
public:
	CSelectCharacterButton ();
	virtual	~CSelectCharacterButton ();

public:
	void	CreateSubControl ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

public:
	void	ResetData();

public:
	SCHARINFO_LOBBY * GetDelCharInfo()				{ return m_pDelCharInfo; }

public:
	int					m_nCharRemainLast;
	int					m_nServerGroup;
	int					m_nServerChannel;

private:
	SCHARINFO_LOBBY*	m_pDelCharInfo;
	CBasicTextButton*	m_pNewButton;
	CBasicTextButton*	m_pDeleteButton;
	CBasicTextButton*	m_pChangeServerButton;
	CBasicTextButton*	m_pGameStartButton;
	CBasicTextBox*		m_pServerName;

	/*dmk14 ingame web*/
	CBasicTextButton*	m_pChangePass;
	CBasicTextButton*	m_pChangePin;
	CBasicTextButton*	m_pGameTimeClaim;
	CBasicTextButton*	m_pResetPin;
	CBasicTextButton*	m_pTopUp;
	CBasicTextButton*	m_pChangeMail;
	CBasicTextBox*		m_pPassStatic;
	CBasicTextBox*		m_pPinStatic;
	CBasicTextBox*		m_pGameTimeStatic;
	CBasicTextBox*		m_pPremiumPointStatic;
	CBasicTextBox*		m_pEmailStatic;
	CBasicTextBox*		m_pNameStatic;
	CBasicLineBox*		m_pBasicLineBox;
	CBasicLineBox*		m_pBasicLineBoxServer;
	CBasicLineBox*		m_pBasicLineBoxSelect;

};