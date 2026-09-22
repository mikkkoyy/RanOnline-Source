/*!
 * \file ProductWindowType.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */
#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class	CBasicButton;


class CProductWindowTypeBase : public CUIGroup
{
private:

public:
	CProductWindowTypeBase ();
	virtual	~CProductWindowTypeBase ();

public:
	void	CreateSubControl ( std::string strbase, std::string strselect, std::string strover );

private:
	CUIControl*		m_pImageBase;
	CUIControl*		m_pImageSelect;
	CUIControl*		m_pImageOver;

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	void	SetSelect( BOOL bSelect );
};