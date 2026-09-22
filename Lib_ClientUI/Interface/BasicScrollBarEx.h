#pragma	once

#include "./BasicScrollBar.h"

class	CBasicScrollBarEx : public CBasicScrollBar
{
public:
	CBasicScrollBarEx ();
	virtual	~CBasicScrollBarEx ();

public:
    void	CreateBaseScrollBar ( char* szBaseScrollBar, const bool& bVERTICAL = true );
};