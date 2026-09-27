#pragma	once

#include "UIWindowEx.h"
#include "GlogicData.h"

class	CMobPreviewWindowRender;
class 	CMobDBWindowItem;
class	CBasicTextBox;

class	CMobPreviewWindow : public CUIWindowEx
{
private:
	enum
	{
		BUTTON_LEFT = ET_CONTROL_NEXT,
		BUTTON_RIGHT,
		ANI_CHECKBOX,
		ITEM_LIST,
	};

    enum
    {
        TITLE_HEAD  = 0,
        TITLE_BODY  = 1,
        TITLE_TAIL  = 2,
        TITLE_MAX,
    };

public:
	CMobPreviewWindow ();
	virtual	~CMobPreviewWindow ();

public:
	void	CreateSubControl ();

private:
	CMobDBWindowItem* 		m_pMobDBItemList;	//--CLASS

private:
    CUIControl*					m_pMobName[TITLE_MAX];
	CUIControl*					m_pBackGround;
	CMobPreviewWindowRender*	m_pRender;
	CBasicButton*				m_pButtonLeft;
	CBasicButton*				m_pButtonRight;
	CBasicTextBox*				m_pMobNameText;

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

public:
	void	ClearRender();
	void	StartPreviewMob ( SNATIVEID sMobID );
};