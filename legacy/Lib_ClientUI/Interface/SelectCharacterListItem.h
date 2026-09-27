#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "../Lib_Client/G-Logic/GLCharDefine.h"

class	CBasicTextBox;

class CSelectCharacterListItem : public CUIGroup
{
private:
	enum
	{
		SELECT_CHARACTER_LIST_MAX = 6,
	};

public:
	CSelectCharacterListItem ();
	virtual	~CSelectCharacterListItem ();

public:
	void	CreateSubControl ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void SetVisibleSingle ( BOOL bVisible );

protected:
	CUIControl*			m_pClassImageOn[GLCI_NUM_8CLASS];
	CUIControl*			m_pClassImageOff[GLCI_NUM_8CLASS];
	CBasicTextBox*		m_pClassText;
	CBasicTextBox*		m_pLevelText;
	CBasicTextBox*		m_pNameText;
	CUIControl*			m_pItemSelect;
	CUIControl*			m_pItemSelectMouseIn;

public:
	int		m_nIndex;
	DWORD	m_dwCharID;
	DWORD	m_dwCharIDSelected;

public:
	void	ResetData();
	void	SetIndex( int nIndex )	{ m_nIndex = nIndex; }
	void	UpdateData();
	void	SetMouseIn( BOOL bEnable );
};