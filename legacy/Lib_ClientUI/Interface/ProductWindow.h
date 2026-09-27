/*!
 * \file ProductWindow.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */

#pragma	once

#include "UIWindowEx.h"

class CProductWindowType;
class CProductWindowList;
class CProductWindowProduct;
class CBasicTextBox;

class CProductWindow : public CUIWindowEx
{
public:
	enum
	{
		PRODUCT_WINDOW_TYPE = ET_CONTROL_NEXT,
		PRODUCT_WINDOW_LIST,
		PRODUCT_WINDOW_PRODUCT,
	};


public:
	CProductWindow ();
	virtual	~CProductWindow ();

public:
	CProductWindowType*		m_pType;
	CProductWindowList*		m_pList;
	CBasicTextBox*			m_pSelectCategory;
	CBasicTextBox*			m_pSelectProduct;
	CProductWindowProduct*	m_pProduct;

public:
	void	CreateSubControl ();

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

public:
	int		m_nSelectType;

public:
	void	LoadData();
	void	ResetData();
	void	SelectCategory( int nSelect );
};