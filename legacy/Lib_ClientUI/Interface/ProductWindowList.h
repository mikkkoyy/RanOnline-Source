/*!
 * \file ProductWindowType.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */
#pragma	once

#include "BasicLineBoxSmart.h"

#include "../Lib_Engine/GUInterface/UIGroup.h"

class	CBasicTextBoxEx;
class	CBasicScrollBarEx;
class	CBasicLineBoxSmart;

const DWORD UIMSG_PRODUCT_WINDOW_LIST_SELECT = UIMSG_USER1;

class CProductWindowList : public CBasicLineBoxSmart
{
private:
	enum
	{
        START_ID = NO_ID + 1,
        ID_PRODUCT_TYPE_LIST = START_ID,
        ID_PRODUCT_TYPE_LIST_TEXTLIST,
        ID_PRODUCT_TYPE_LIST_SCROOL,
	};

public:
	CProductWindowList ();
	virtual	~CProductWindowList ();

public:
	void	CreateSubControl ();

protected:
	CBasicTextBoxEx*	m_pTextBox;
	CBasicScrollBarEx*	m_pScrollBar;

	int		m_nSelectIndex;
	DWORD	m_dwSelectedData;
	std::string m_strSelectedText;

	
    CBasicLineBoxSmart*		m_pSeleted;
    UIRECT					m_rcSeleted;
    int						m_nSelectedPos;

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

public:
	void	LoadData( WORD wCategory, WORD wFilter );
	void	ClearData();
	DWORD	GetSelectData()	{ return m_dwSelectedData; }
	std::string GetSelectText() { return m_strSelectedText; }
};