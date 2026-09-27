#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CBasicTextBox;

class CLobbyTitle : public CUIGroup
{
private:

public:
	CLobbyTitle ();
	virtual	~CLobbyTitle ();

public:
	void CreateSubControl ();

private:
	CBasicTextBox*		m_pTitle;

public:
	void	SetLobbyText( CString strText );
	void	ClearLobbyText();
};