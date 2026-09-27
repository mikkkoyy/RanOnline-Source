#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLCDMDefine.h"

class CBasicTextBox;
class CCdmRankingDisplaySlot;

class CCdmRankingDisplay : public CUIGroup
{
	enum
	{
		CDM_RANKING_SLOT_0 = NO_ID + 1,
		CDM_RANKING_SLOT_1,
		CDM_RANKING_SLOT_2,
		CDM_RANKING_SLOT_3,
		CDM_RANKING_SLOT_4,
		CDM_RANKING_SLOT_5,
		CDM_RANKING_SLOT_6,
		CDM_RANKING_SLOT_7,
		CDM_RANKING_SLOT_8,
		CDM_RANKING_SLOT_9,
		CDM_RANKING_SLOT_OWN,
	};

private:
	CBasicTextBox*		m_pRankText;
	CBasicTextBox*		m_pClubText;
	CBasicTextBox*		m_pScoreText;

	CCdmRankingDisplaySlot*		m_pSlot[RANKING_NUM];
	CCdmRankingDisplaySlot*		m_pSlotOwn;

public:
	CCdmRankingDisplay();
	~CCdmRankingDisplay();

	void	CreateSubControl ();
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

public:	
	void	RefreshCdmRanking();
};