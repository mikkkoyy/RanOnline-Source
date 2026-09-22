#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLPVPTyrannyData.h"

class CBasicLineBox;
class CSkillImage33;

class CPVPTyrannyPageHistoryBuff : public CUIGroup
{
private:
	enum
	{
		TYRANNY_PAGE_HISTORY_BUFF_SLOT0 = NO_ID + 1,
		TYRANNY_PAGE_HISTORY_BUFF_SLOT1,
		TYRANNY_PAGE_HISTORY_BUFF_SLOT2,
		TYRANNY_PAGE_HISTORY_BUFF_SLOT3,
	};

public:
	CPVPTyrannyPageHistoryBuff ();
	virtual	~CPVPTyrannyPageHistoryBuff ();

public:
	void	CreateSubControl ();

public:
	CBasicLineBox*		m_pSkillLineBox[TYRANNY_REWARD_BUFF_NUM];
	CSkillImage33*		m_pSkillImage[TYRANNY_REWARD_BUFF_NUM];

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

public:
	float	m_fUpdateTime;
	void	ShowInfo();

};