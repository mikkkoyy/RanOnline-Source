

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "../Lib_Engine/G-logic/GLdefine.h"
#include "GLCharDefine.h"

#include "GLCharData.h"

class CBasicLineBox;
class CBasicTextBox;
class CBasicTextButton;

const DWORD UIMSG_NPC_SHOP_SLOT_ITEM_IMAGE = UIMSG_USER1;

class	CPartyFinderSlot : public CUIGroup
{
	enum
	{
		PARTYFINDER_JOIN = NO_ID + 1,
		ICON_CLASS_IMAGE0,
		ICON_CLASS_IMAGE1,
		ICON_CLASS_IMAGE2,
		ICON_CLASS_IMAGE3,
		ICON_CLASS_IMAGE4,
		ICON_CLASS_IMAGE5,
		ICON_CLASS_IMAGE6,
		ICON_CLASS_IMAGE7,
		ICON_CLASS_IMAGE8,
		ICON_CLASS_IMAGE9,
		ICON_CLASS_IMAGE10,
		ICON_CLASS_IMAGE11,
		ICON_CLASS_IMAGE12,
		ICON_CLASS_IMAGE13,
	};


public:
	CPartyFinderSlot ();
	virtual	~CPartyFinderSlot ();

public:
	void	CreateSubControl ();
	void	Update ( int x ,int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	CBasicTextBox*	CreateStaticControl ( char* szControlKeyword, CD3DFontPar* pFont, D3DCOLOR D3DCOLOR, int nAlign );
	CBasicLineBox*	CreateUILineBoxWhiteNoBody( char* szBaseControl );
	CUIControl*		CreateControl ( const char* szControl, const UIGUID& cID = NO_ID );

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

private:
	CBasicLineBox*		m_pSelectParty;
	CBasicTextBox*	m_pLeaderName;
	CBasicTextBox*	m_pLeaderInfo;
	CBasicTextBox*	m_pPartyMap;
	CBasicTextBox*	m_pPartyInfo;
	CBasicTextBox*	m_pPartyCount;
public:
	CBasicTextButton*	m_pButtonJoin;

private:
	static const int nSCHOOL_MARK = 3;
	CUIControl*	m_pSchoolMark[nSCHOOL_MARK];
	CUIControl*	m_pClassImg[GLCI_NUM_6CLASS];
private:
	PARTYLINKDATA			m_sPartyData;
	DWORD		m_dwGaeaID;
	
	float		m_fCooldown;
	BOOL		m_bCanJoin;
	BOOL		m_bRequesting;
public:
	BOOL		CanJoin() { return m_bCanJoin; }
	void		SetCanJoin(bool bJoin) {  m_bCanJoin = bJoin; }
	BOOL		IsRequesting() { return m_bRequesting; }
	void		SetRequesting(bool bRequest) {  m_bRequesting = bRequest; }


public:
	void	SlotSet( PARTYLINKDATA sPartyData );
	void	SlotReset();
	void	SlotSelect( BOOL bSelect );
	BOOL	IsJoinAvailable();

};