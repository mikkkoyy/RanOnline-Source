#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLDefine.h"

#define PET_SKILL_INFO_DISPLAY_UPDATE_TIME 0.2f

class CPetSkillImage;
class CBasicTextBox;

class	CPetSkillInfoDisplay : public CUIGroup
{
private:
	CPetSkillImage*	m_pSkillImage;
	CUIControl*		m_pDummyControl;
	std::string		m_strSkillName;
	float			m_fUpdateTime;

public:
	CPetSkillInfoDisplay ();
	virtual	~CPetSkillInfoDisplay ();

public:
	void	CreateSubControl ();
public:
	virtual void	Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
};