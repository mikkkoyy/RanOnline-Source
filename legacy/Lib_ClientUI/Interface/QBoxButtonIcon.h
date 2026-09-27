#pragma once

#include "BasicButton.h"
#include "../Lib_Client/G-Logic/GLItemDef.h"

class CQBoxButtonIcon : public CBasicButton
{
public:
	CQBoxButtonIcon(void);
	~CQBoxButtonIcon(void);

public:
	CUIControl*		m_pImageSpeedUp;
	CUIControl*		m_pImageCrazyTime;
	CUIControl*		m_pImagePowerUp;
	CUIControl*		m_pImageExpTime;
	CUIControl*		m_pImageLucky;
	CUIControl*		m_pImageSpeedUpMax;
	CUIControl*		m_pImageMadnessTime;
	CUIControl*		m_pImagePowerUpMax;

public:
	EMITEM_QUESTION		m_emType;

public:
	void	CreateSubControl ();
	void	SetType( EMITEM_QUESTION emType );
	EMITEM_QUESTION	GetType()	{	return m_emType;	}

};
