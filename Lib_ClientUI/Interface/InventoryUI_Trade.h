//	마켓 윈도우
//
//	최초 작성자 : 성기엽
//	이후 수정자 : 
//	로그
//		[2003.12.6]
//			@ 작성
//

#pragma	once

#include "UIWindowEx.h"

struct	SITEMCUSTOM;
class	CInventoryUI_TradeInven;
class	CBasicTextButton;

class	CInventoryUI_Trade : public CUIWindowEx
{
protected:
	enum
	{
		TRADEINVENTORY_MONEY_BUTTON = ET_CONTROL_NEXT,
		TRADEINVENTORY_POINT_BUTTON,
		TRADEINVENTORY_INVEN_BUTTON,
		TRADEINVENTORY_PLUS_MONEY_BUTTON,
		TRADEINVENTORY_MINUS_MONEY_BUTTON,
		TRADEINVENTORY_PAGE,
	};

public:
	CInventoryUI_Trade ();
	virtual	~CInventoryUI_Trade ();

public:
	void	CreateSubControl ();
	CBasicTextButton*	CreateTextButton23 ( const char* szButton, UIGUID ControlID, const char* szText );

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual HRESULT RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );

public:
	void	SetOPENER ( bool bOPENER );

private:
	void	SetMoney ( LONGLONG Money );
	bool	IsTurnITEM ( SITEMCUSTOM& sItemCustom );

public:
	void	SetOneViewSlot ( const int& nONE_VIEW_SLOT );
	const int& GetOneViewSlot () const;

private:
	CInventoryUI_TradeInven*	m_pPage;
	CUIControl*					m_pBackGround;
	CBasicTextButton*			m_pInvenButton;

private:
	CBasicTextBox*		m_pMoneyTextBox;
	CBasicTextBox*		m_pPointTextBox;

	bool	m_bTRADE_MODE;
	bool	m_bOPENER;

	int		m_nONE_VIEW_SLOT;
};