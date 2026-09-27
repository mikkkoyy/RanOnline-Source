#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLPVPTyrannyData.h"
#include "GLPVPTyrannyDefine.h"
#include "GLCharDefine.h"

class CBasicTextBox;

class CPVPTyrannyRankingPageSlot : public CUIGroup
{
private:
	enum
	{
	};

public:
	CPVPTyrannyRankingPageSlot ();
	virtual	~CPVPTyrannyRankingPageSlot ();

public:
	void	CreateSubControl( BOOL bSelf );

public:
	CBasicTextBox*	m_pTextRank;
	CBasicTextBox*	m_pTextName;
	CBasicTextBox*	m_pTextDamage;
	CBasicTextBox*	m_pTextHeal;
	CBasicTextBox*	m_pTextKill;
	CBasicTextBox*	m_pTextDeath;
	CBasicTextBox*	m_pTextResu;
	CBasicTextBox*	m_pTextScore;
	CBasicTextBox*	m_pTextPoint;

	CUIControl*		m_pSchoolImage[TYRANNY_SCHOOL_SIZE];
	CUIControl*		m_pClassImage[GLCI_NUM_8CLASS];

public:
	BOOL	m_bSelf;
	void	ResetData();
	void	SetData( TYRANNY_PLAYER_DATA* pData, BOOL bTotal );

};