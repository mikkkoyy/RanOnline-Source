/*!
 * \file CdmRankingDisplaySlot.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLCDMDefine.h"

class CBasicTextBox;

class CCdmRankingDisplaySlot : public CUIGroup
{
private:
	enum
	{
	};

public:
	CCdmRankingDisplaySlot ();
	virtual	~CCdmRankingDisplaySlot ();

public:
	void	CreateSubControl ();

public:
	CBasicTextBox*	m_pTextRank;
	CBasicTextBox*	m_pTextClub;
	CBasicTextBox*	m_pTextScore;


public:
	void	ResetData();
	void	SetData( int nRank, std::string strClub, WORD wKill, WORD wDeath );

};