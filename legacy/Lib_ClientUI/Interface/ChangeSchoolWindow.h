/*!
 * \file ChangeSchoolWindow.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */

#pragma once

#include "UIWindowEx.h"
#include "GLCharDefine.h"

class CBasicTextBox;
class CBasicButton;
class CBasicTextButton;


class CChangeSchoolWindow : public CUIWindowEx
{
	enum
	{
		CHANGE_SCHOOL_WINDOW_BUTTON_SAVE = ET_CONTROL_NEXT,
		CHANGE_SCHOOL_WINDOW_BUTTON_CANCEL,
		CHANGE_SCHOOL_WINDOW_SELECT_SG,
		CHANGE_SCHOOL_WINDOW_SELECT_MP,
		CHANGE_SCHOOL_WINDOW_SELECT_PHX
	};

public:
	CChangeSchoolWindow(void);
	virtual ~CChangeSchoolWindow(void);

public:
	void CreateSubControl();

private:
	CUIControl*		m_pTypeSchool_Off[GLSCHOOL_NUM];
	CUIControl*		m_pTypeSchool_On[GLSCHOOL_NUM];
	CUIControl*		m_pTypeSchool_Box[GLSCHOOL_NUM];
	CUIControl*		m_pTypeSchool_Focus[GLSCHOOL_NUM];

	CBasicTextBox*	m_pInfoTitle;
	CBasicTextBox*	m_pInfoDesc;

	CBasicTextButton*	m_pButtonClose;
	CBasicTextButton*	m_pButtonSave;

public:
	int		m_nSelectedLastSchool;
	int		m_nSelectedSchool;

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage( UIGUID cID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

	void	ResetAll();
	void	UpdateUI();
	void	SchoolSelect( int nSelect );
	void	SchoolMouseIn( int nSelect );


};