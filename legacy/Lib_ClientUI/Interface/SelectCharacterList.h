#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CSelectCharacterListItem;
class CBasicScrollBarEx;

class CSelectCharacterList : public CUIGroup
{
private:
	enum
	{
		SELECT_CHARACTER_LIST_0 = NO_ID + 1,
		SELECT_CHARACTER_LIST_1,
		SELECT_CHARACTER_LIST_2,
		SELECT_CHARACTER_LIST_3,
		SELECT_CHARACTER_LIST_4,
		SELECT_CHARACTER_LIST_5,
	};

	enum
	{
		SELECT_CHARACTER_LIST_MAX = 4,
	};

public:
	CSelectCharacterList ();
	virtual	~CSelectCharacterList ();

public:
	void	CreateSubControl ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

protected:
	CSelectCharacterListItem*		m_pListItem[SELECT_CHARACTER_LIST_MAX];
	CBasicScrollBarEx*				m_pScrollBar;

public:
	void	ResetData();
};