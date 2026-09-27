#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class	CBasicButton;
class	CBasicTextBox;
class	CBasicTextButton;
class	CSelectCharacterInfoPage;
class	CSelectCharacterList;
class	CSelectCharacterButton;
struct	SCHARINFO_LOBBY;

const DWORD UIMSG_MOUSEIN_SELECT_CHARACTERPAGE = UIMSG_USER1;

class CSelectCharacterPage : public CUIGroup
{
private:
	enum
	{
		SELECT_CHARACTER_LIST = NO_ID + 1,
		SELECT_CHARACTER_BUTTON,
	};

public:
	CSelectCharacterPage ();
	virtual	~CSelectCharacterPage ();

public:
	void	CreateSubControl ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

public:
	SCHARINFO_LOBBY * GetDelCharInfo();
	BOOL	SndGameJoin ();

public:
	void	ResetAll ();

private:
	CSelectCharacterInfoPage*	m_pSelectCharacterPage;
	CSelectCharacterList*		m_pSelectCharacterList;
	CSelectCharacterButton*		m_pSelectCharacterButton;

protected:
};