/*!
 * \file ItemShopWindow.h
 *
 * \author Juver
 * \date July 2017
 *
 * 
 */

#pragma	once

#include "./UIWindowEx.h"
#include "../../Lib_Client/G-Logic/GLGaeaClient.h"
#include "../../Lib_Client/G-Logic/GLCharData.h"

class   GLGaeaClient;
class   CInnerInterface;
class	CBasicTextBoxEx;
class	CBasicScrollBarEx;
class	CBasicTextButton;
class	CBasicButton;
class	CPandoraBoxItemSlot;
class	CPandoraBoxResultsItemSlot;
class CSwapImage;
class CCheckBox;

class CPandoraBoxWindow : public CUIWindowEx
{
private:
	static	const	int	nSTARTLINE;
	static	const	int	nOUTOFRANGE;
public:
	enum
	{
		PANDORA_BOX_SELECTION_TEXTBOX = ET_CONTROL_NEXT,
		PANDORA_BOX_SELECTION_SCROLLBAR,
		PANDORA_BOX_LIST_SCROLLBAR,
		PANDORA_BOX_PURCHASE_BUTTON,
		PANDORA_BOX_CLEAR_BUTTON,
		PANDORA_BOX_CLOSE_BUTTON,
		PANDORA_BOX_TYPE1_BUTTON,
		PANDORA_BOX_TYPE2_BUTTON,
		PANDORA_BOX_ITEM00,
		PANDORA_BOX_ITEM01,
		PANDORA_BOX_ITEM02,
		PANDORA_BOX_ITEM03,
		PANDORA_BOX_ITEM04,
		PANDORA_BOX_ITEM05,
		PANDORA_BOX_ITEM06,
		PANDORA_BOX_ITEM07,
		PANDORA_BOX_ITEM08,
		PANDORA_BOX_ITEM09,
		PANDORA_BOX_ITEM010,
		PANDORA_BOX_ITEM011,
		PANDORA_BOX_RESULT_ITEM00,
		PANDORA_BOX_RESULT_ITEM01,
		PANDORA_BOX_RESULT_ITEM02,
		PANDORA_BOX_RESULT_ITEM03,
		PANDORA_BOX_RESULT_ITEM04,
		PANDORA_BOX_RESULT_ITEM05,
		PANDORA_BOX_RESULT_ITEM06,
		PANDORA_BOX_RESULT_ITEM07,
		PANDORA_BOX_RESULT_ITEM08,
		PANDORA_BOX_RESULT_ITEM09,
	};

	enum
	{
		PANDORA_BOX_MAX_ITEM = 12,
		PANDORA_BOX_MAX_RESULT_ITEM = 10,
		PANDORA_BOX_OPEN_TYPE_SIZE = 2

	};

public:
	CPandoraBoxWindow ( GLGaeaClient* pGaeaClient, CInnerInterface* pInterface );
	virtual	~CPandoraBoxWindow ();

public:
	void	CreateSubControl ();

private:
	CBasicTextBoxEx*	m_pListTextBox;
	CBasicScrollBarEx*	m_pListScrollBar;
	CBasicScrollBarEx*	m_pItemListScrollBar;
	CBasicTextButton*	m_pButtonClear;
	CBasicTextButton*	m_pButtonPurchase;
	CBasicTextButton*	m_pButtonClose;
	CBasicTextBox*		m_pTextBoxStatus;
	CBasicTextBox*		m_pTextCharGoldPoint;
	CBasicTextBox*		m_pTextCharEPoint;
	CBasicTextBox*		m_pTextGoldPoint;
	CBasicTextBox*		m_pTextEPoint;
	CSwapImage*             m_pImageBOX;
	CPandoraBoxItemSlot*	m_pItemSlot[PANDORA_BOX_MAX_ITEM];
	CUIControl*				m_pItemSlotArrayDummy[PANDORA_BOX_MAX_ITEM];
	CPandoraBoxResultsItemSlot*	m_pItemResultsSlot[PANDORA_BOX_MAX_RESULT_ITEM];
	CCheckBox*			m_pOpenType[PANDORA_BOX_OPEN_TYPE_SIZE];
	int	m_nCurPos;

protected:
	void	ResetAllItemSlotRender ();
	void	SetItemSlotRender ( int nStartIndex, int nTotal );

	void	ResetResultsSlotRender ();

private:
	SNATIVEID	m_sSelectItem;

protected:
	GLGaeaClient* m_pGaeaClient;
    CInnerInterface* m_pInterface;

private:
	PANDORABOXMAP	m_mapItem;
	VECPANDORABOX	m_vecItem;
public:
	void InitWindow();
	void ResetWindow();
	void ShowItem();
	void SelectItem( int nSelect );

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );
};