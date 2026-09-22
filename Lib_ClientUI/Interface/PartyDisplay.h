#pragma	once

#include "UIWindowEx.h"
#include "GLParty.h"

class	CPartySlotDisplay;
struct	GLPARTY_CLIENT;
class	CPartyDisplayBuffInfo;
class	CBasicLineBox;

class	CPartyDisplay : public CUIWindowEx
{
protected:
	enum
	{
		PARTYDISPLAY_SLOT_MEMBER0 = ET_CONTROL_NEXT,
		PARTYDISPLAY_SLOT_MEMBER1,
		PARTYDISPLAY_SLOT_MEMBER2,
		PARTYDISPLAY_SLOT_MEMBER3,
		PARTYDISPLAY_SLOT_MEMBER4,
		PARTYDISPLAY_SLOT_MEMBER5,
		PARTYDISPLAY_SLOT_MEMBER6,
		PARTYDISPLAY_SLOT_MEMBER7,
		PARTYDISPLAY_BUFF_INFO0,
		PARTYDISPLAY_BUFF_INFO1,
		PARTYDISPLAY_BUFF_INFO2,
		PARTYDISPLAY_BUFF_INFO3,
		PARTYDISPLAY_BUFF_INFO4,
		PARTYDISPLAY_BUFF_INFO5,
		PARTYDISPLAY_BUFF_INFO6,
		PARTYDISPLAY_BUFF_INFO7,
	};

public:
	CPartyDisplay ();
	virtual	~CPartyDisplay ();

public:
	void CreateSubControl ();

private:
	GLPARTY_CLIENT*		FindSelfClient ();
	BOOL	IsSameMap ( GLPARTY_CLIENT *pSelf, GLPARTY_CLIENT *pMember );

private:
	void	SetSlotMember ( CPartySlotDisplay* pPartySlot, GLPARTY_CLIENT *pMember, BOOL bSameMap, const int nPartySlotID );

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	void SetBuff ();
	void HideBuff ();

private:
	CPartySlotDisplay*		m_pPartySlot[MAXPARTY];
	CPartyDisplayBuffInfo*		m_pPartyDisplayBuffInfo;
	CPartyDisplayBuffInfo*		m_pPartyDisplayBuffInfo1;
	CPartyDisplayBuffInfo*		m_pPartyDisplayBuffInfo2;
	CPartyDisplayBuffInfo*		m_pPartyDisplayBuffInfo3;
	CPartyDisplayBuffInfo*		m_pPartyDisplayBuffInfo4;
	CPartyDisplayBuffInfo*		m_pPartyDisplayBuffInfo5;
	CPartyDisplayBuffInfo*		m_pPartyDisplayBuffInfo6;
	CPartyDisplayBuffInfo*		m_pPartyDisplayBuffInfo7;
	bool				m_bMOUSEIN;
	bool				m_bBuffOK;
	void				CheckMouseState();

public:
	int			m_PosX;
	int			m_PosY;
	D3DXVECTOR2	m_vGap;
	
	bool		m_bCHECK_MOUSE_STATE;

	bool		IsBuffOK() { return m_bBuffOK; }

	UIRECT m_rcOriginalPos;
};