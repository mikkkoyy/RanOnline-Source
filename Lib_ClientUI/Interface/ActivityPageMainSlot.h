/*!
 * \file ActivityPageMainSlot.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */
#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CBasicTextBox;

class	CActivityPageMainSlot : public CUIGroup
{
private:
	enum
	{
	};

public:
	CActivityPageMainSlot ();
	virtual	~CActivityPageMainSlot ();

public:
	void	CreateSubControl ();

public:
	CUIControl*		m_pStatusComplete;
	CUIControl*		m_pProgressComplete;

	CUIControl*		m_pImageBadge;
	CUIControl*		m_pImagePoint;

	CBasicTextBox*	m_pTextComplete;
	CBasicTextBox*	m_pTextTitle;
	CBasicTextBox*	m_pTextDesc;
	CBasicTextBox*	m_pTextProgress;

	CBasicTextBox*	m_pTextBadge;
	CBasicTextBox*	m_pTextPoint;

	float			m_fUpdateTime;
	DWORD			m_dwActivityID;

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

	void	ResetData();
	void	SetData( DWORD dwID );
	void	UpdateInfo();
};