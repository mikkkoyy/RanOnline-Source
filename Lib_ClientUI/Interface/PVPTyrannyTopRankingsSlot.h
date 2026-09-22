#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLPVPTyrannyData.h"
#include "GLPVPTyrannyDefine.h"
#include "GLCharDefine.h"

class CBasicTextBox;

class CPVPTyrannyTopRankingsSlot : public CUIGroup
{
private:
	enum
	{
	};

public:
	CPVPTyrannyTopRankingsSlot ();
	virtual	~CPVPTyrannyTopRankingsSlot ();

public:
	void	CreateSubControl ();

public:
	CBasicTextBox*	m_pTextRank;
	CBasicTextBox*	m_pTextName;

	CUIControl*		m_pSchoolImage[TYRANNY_SCHOOL_SIZE];
	CUIControl*		m_pClassImage[GLCI_NUM_8CLASS];

public:
	void	ResetData();
	void	SetData( TYRANNY_PLAYER_DATA* pData );

};