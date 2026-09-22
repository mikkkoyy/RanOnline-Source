//	파티 윈도우
//
//	최초 작성자 : 성기엽
//	이후 수정자 : 
//	로그
//		[2003.11.27]
//			@ 작성
//

#pragma	once

#include "UIWindowEx.h"
#include "GLParty.h"

class	CPartySlot;
struct	GLPARTY_CLIENT;
class	CBasicTextBox;
class	CBasicTextButton;
class	CBasicComboBox;
class	CBasicComboBoxRollOver;
class	CPartyWindowCheckBox;
class	CBasicButtonText;
class	RnButton;
class	CPartyWindow : public CUIWindowEx
{
protected:
	enum
	{
		BUTTON_PARTY = ET_CONTROL_NEXT,
		PARTY_INFO_ITEM_COMBO_OPEN,
		PARTY_INFO_MONEY_COMBO_OPEN,
		PARTY_INFO_ITEM_FILTER_COMBO_ROLLOVER,
		PARTY_INFO_MONEY_COMBO_ROLLOVER,
		PARTY_INFO_CHECK_BOX,
		PARTY_INFO_BUFF_CHECK_BOX,
		PARTY_INFO_SLOT0,
		PARTY_INFO_SLOT1,
		PARTY_INFO_SLOT2,
		PARTY_INFO_SLOT3,
		PARTY_INFO_SLOT4,
		PARTY_INFO_SLOT5,
		PARTY_INFO_SLOT6,
		PARTY_INFO_SLOT7,
		PARTY_TAP_BUTTON,
		PARTY_INFO_KICK_BTN,
		PARTY_INFO_DELEGATE_BTN,
		PARTY_INFO_DISBAND_BTN,
		PARTY_INFO_LEAVE_BTN
	};

public:
	CPartyWindow ();
	virtual	~CPartyWindow ();

public:
    void	CreateSubControl ();

public:
	void	SetPartyInfo ( DWORD dwPartyID, DWORD dwMasterID );

	DWORD	GetPartyID ();
	DWORD	GetMasterID();

private:
	GLPARTY_CLIENT*		FindSelfClient ();
	BOOL	IsSameMap ( GLPARTY_CLIENT *pSelf, GLPARTY_CLIENT *pMember );
	CBasicButtonText*	CreateButtonBlue ( char* szButton, char* szTextBox, CD3DFontPar* pFont, int nAlign, UIGUID ControlID, CString strText );

private:
	void	SetSlotMember ( CPartySlot* pPartySlot, GLPARTY_CLIENT *pMember, BOOL bSameMap, int nSlot );

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );
	void LoadDefaultCondition ();

private:
	SPARTY_OPT	m_sPartyOption;

private:
	DWORD	m_dwPartyID;
	DWORD	m_dwMasterID;

protected:
    RnButton*		m_pButtonWithdraw;
    RnButton*		m_pButtonRelease;
    RnButton*		m_pButtonDelegate;
    RnButton*		m_pButtonBan;

private:
	CBasicTextBox*		CreateStaticControl ( char* szControlKeyword, CD3DFontPar* pFont, int nAlign, const UIGUID& cID = NO_ID );
	CBasicTextButton*	CreateTextButton23 ( const char* szButton, UIGUID ControlID, const char* szText );
	CBasicTextButton*	m_pParty;
	
	CBasicTextButton*	m_pButtonParty;

	CUIControl*			m_pBackGround;
	CBasicComboBox*		m_pComboBoxItem;
	CBasicComboBox*		m_pComboBoxMoney;
	CBasicComboBoxRollOver*		m_pComboBoxItemRollOver;
	CBasicComboBoxRollOver*		m_pComboBoxMoneyRollOver;

	CPartyWindowCheckBox*	CreateCheckBox ( CString strKeyword, UIGUID ControlID );
	CPartyWindowCheckBox*	m_pMember;
	CPartyWindowCheckBox*	m_pBuff;
	CBasicTextBox*		m_pNoPartyText;

	CPartySlot*			m_pPartySlot[MAXPARTY];

	CBasicButtonText*	m_pKick;
	CBasicButtonText*	m_pDelegate;
	CBasicButtonText*	m_pDisband;
	CBasicButtonText*	m_pLeave;

	int					m_nMemberIndex;

	void				ResetData();

private:
	bool				m_bParty;
	static	const	int		nOUTOFRANGE;
	UIGUID					m_RollOverID;
	BOOL					m_bFirstLBUP;

public:
	bool				IsParty()	{ return m_bParty; }
};