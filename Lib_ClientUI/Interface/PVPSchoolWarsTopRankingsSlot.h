/*!
 * \file PVPSchoolWarsTopRankingsSlot.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLPVPSchoolWarsData.h"
#include "GLPVPSchoolWarsDefine.h"
#include "GLCharDefine.h"

class CBasicTextBox;

class CPVPSchoolWarsTopRankingsSlot : public CUIGroup
{
private:
	enum
	{
	};

public:
	CPVPSchoolWarsTopRankingsSlot ();
	virtual	~CPVPSchoolWarsTopRankingsSlot ();

public:
	void	CreateSubControl ();

public:
	CBasicTextBox*	m_pTextRank;
	CBasicTextBox*	m_pTextName;

	CUIControl*		m_pSchoolImage[SCHOOLWARS_SCHOOL_SIZE];
	CUIControl*		m_pClassImage[GLCI_NUM_8CLASS];

public:
	void	ResetData();
	void	SetData( SCHOOLWARS_PLAYER_DATA* pData );

};