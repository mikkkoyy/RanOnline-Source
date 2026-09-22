/*!
 * \file MultiModeButtonImage.h
 *
 * \author Juver
 * \date February 2017
 *
 * 
 */

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class	CMultiModeButtonImage : public CUIGroup
{
public:
	CMultiModeButtonImage ();
	virtual	~CMultiModeButtonImage ();

public:
	void	CreateButtonImage ( const char* szLeft, const char* szCenter, const char* szRight );
};