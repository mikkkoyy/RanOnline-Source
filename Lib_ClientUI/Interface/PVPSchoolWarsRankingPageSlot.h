#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLPVPSchoolWarsData.h"
#include "GLPVPSchoolWarsDefine.h"
#include "GLCharDefine.h"

class CBasicTextBox;

class CPVPSchoolWarsRankingPageSlot : public CUIGroup
{
private:
	enum
	{
	};

public:
	CPVPSchoolWarsRankingPageSlot ();
	virtual	~CPVPSchoolWarsRankingPageSlot ();

public:
	void	CreateSubControl( BOOL bSelf );

public:
	CBasicTextBox*	m_pTextRank;
	CBasicTextBox*	m_pTextName;
	CBasicTextBox*	m_pTextKill;
	CBasicTextBox*	m_pTextDeath;
	CBasicTextBox*	m_pTextResu;
	CBasicTextBox*	m_pTextHeal;
	CBasicTextBox*	m_pTextScore;
	CBasicTextBox*	m_pTextPoint;

	CUIControl*		m_pSchoolImage[SCHOOLWARS_SCHOOL_SIZE];
	CUIControl*		m_pClassImage[GLCI_NUM_8CLASS];

public:
	BOOL	m_bSelf;
	void	ResetData();
	void	SetData( SCHOOLWARS_PLAYER_DATA* pData, BOOL bTotal );

};