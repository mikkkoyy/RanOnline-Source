#pragma	once

#include "UIWindowEx.h"
#include "GLParty.h"

class	CMiniPartyLinkSlot;
struct	GLPARTY_CLIENT;
class	CBasicTextButton;
class	RnButton;

class	CMiniPartyLinkWindow : public CUIWindowEx
{
protected:
	enum
	{
		MINIPARTYLINK_SLOT_MEMBER0 = ET_CONTROL_NEXT,
		MINIPARTYLINK_SLOT_MEMBER1,
		MINIPARTYLINK_SLOT_MEMBER2,
		MINIPARTYLINK_SLOT_MEMBER3,
		MINIPARTYLINK_SLOT_MEMBER4,
		MINIPARTYLINK_SLOT_MEMBER5,
		MINIPARTYLINK_SLOT_MEMBER6,
		MINIPARTYLINK_SLOT_MEMBER7,

		MINIPARTYLINK_BUTTON_REQ,
	};

public:
	CMiniPartyLinkWindow ();
	virtual	~CMiniPartyLinkWindow ();

protected:
	DWORD		m_dwMasterID;
	DWORD		m_dwPartyID;

public:
	void CreateSubControl ();
	void	ReSetSlotData ();
	void	SetSlotData ( GLPARTY_CLIENT *pMember, const int nPartySlotID );
	void	SetData ( DWORD dwMasterID, DWORD dwPartyID );

private:
	void	SetSlotMember ( CMiniPartyLinkSlot* pPartySlot, GLPARTY_CLIENT *pMember, const int nPartySlotID );

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

private:
	CMiniPartyLinkSlot*		m_pPartySlot[MAXPARTY];

protected:
	RnButton*		m_pButtonRequest;
};