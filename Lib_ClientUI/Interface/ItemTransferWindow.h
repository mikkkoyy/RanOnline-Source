/*!
 * \file ItemTransferWindow.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */

#pragma	once

#include "UIWindowEx.h"

class CBasicTextBox;
class CItemImage;
class CBasicTextButton;

//--------------------------------------------------------------------
// CItemTransferWindow : 
//--------------------------------------------------------------------
class CItemTransferWindow : public CUIWindowEx	
{
protected:
	enum
	{
		ITEM_TRANSFER_ITEM_IMAGE_ITEM_OLD = ET_CONTROL_NEXT,
		ITEM_TRANSFER_ITEM_IMAGE_ITEM_NEW,
		ITEM_TRANSFER_ITEM_IMAGE_ITEM_RESULT,
		ITEM_TRANSFER_BUTTON_START,
		ITEM_TRANSFER_BUTTON_CANCEL,
	};

protected:
	
public:

public:
	CItemTransferWindow();
	virtual	~CItemTransferWindow();

public:
	void CreateSubControl();

public:
	CBasicTextBox*		m_pTextInfo;
	CItemImage*			m_pItemImageOld;
	CItemImage*			m_pItemImageNew;
	CItemImage*			m_pItemImageResult;

	CUIControl*			m_pArrowBad;
	CUIControl*			m_pArrowGood;

	CBasicTextBox*		m_pTextItemOld;
	CBasicTextBox*		m_pTextItemNew;
	CBasicTextBox*		m_pTextItemResult;

	CBasicTextBox*		m_pTextCost;

	CBasicTextButton*	m_pButtonStart;
	CBasicTextButton*	m_pButtonCancel;

public:
	virtual	void Update( INT x, INT y, BYTE LB, BYTE MB, BYTE RB, INT nScroll, FLOAT fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage( UIGUID ControlID, DWORD dwMsg );
	virtual void SetVisibleSingle( BOOL bVisible );

	void	TransferItemStats();
};