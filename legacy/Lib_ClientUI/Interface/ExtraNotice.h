/*!
 * \file ExtraNotice.h
 *
 * \author Juver
 * \date June 2017
 *
 * 
 */

#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class	CBasicTextBox;

class	CExtraNotice : public CUIGroup
{
public:
	CExtraNotice ();
	virtual	~CExtraNotice ();

public:
	void	CreateSubControl ();

public:
	void	AddText ( CString strMessage, D3DCOLOR dwMessageColor );

public:
	virtual void	Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

private:
	CBasicTextBox*	m_pTextBox;	

private:
	float	m_fLifeTime;
};