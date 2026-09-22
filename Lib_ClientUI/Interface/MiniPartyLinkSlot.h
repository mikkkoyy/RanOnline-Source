#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLCharDefine.h"
#include "GLParty.h"
#include "BasicTextBox.h"
#include "BasicProgressBar.h"

////////////////////////////////////////////////////////////////////
//	사용자 메시지 정의
const DWORD UIMSG_MOUSEIN_RB_EVENT = UIMSG_USER1;
////////////////////////////////////////////////////////////////////

class	CBasicTextBox;
class	CBasicProgressBar;

class	CMiniPartyLinkSlot : public CUIGroup
{
protected:
	enum
	{
		HP_BAR = NO_ID + 1,
	};

	CBasicTextBox*		m_pPlayerNum;
	CBasicTextBox*		m_pPlayerName;
	CBasicTextBox*		m_pPlayerLevel;
	CBasicTextBox*		m_pPlayerClass;

public:
	CMiniPartyLinkSlot ();
	virtual	~CMiniPartyLinkSlot ();

public:
	void	CreateSubControl ();

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

public:
	void	SetPlayerData ( int nSlotNumber, CString strName, int nLevel, int nClass );
	void	ReSetPlayerData ();

protected:
	CUIControl*		CreateControl ( const char* szControl );
	CBasicTextBox*	CreateStaticControl ( char* szControlKeyword, CD3DFontPar* pFont, D3DCOLOR D3DCOLOR, int nAlign );
};