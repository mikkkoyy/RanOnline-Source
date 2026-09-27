#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLDefine.h"

//struct	SNATIVEID;
class	CBasicProgressBar;
class CBasicMiniBarGauge;
class	CSkillImage;

class	CSkillTimeUnit : public CUIGroup
{
private:
	enum
	{
		TIMEUNIT_IMAGE = NO_ID + 1,
		TIMEUNIT_PROGRESS,
	};

public:
	CSkillTimeUnit (void);
	virtual	~CSkillTimeUnit ();

public:
	void	CreateSubControl ();

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

public:
	void	SetLeftTime ( const float& fLeftTime );
	void	SetSkill ( const DWORD& dwID, const float& fLifeTime, const CString& strSkillName, bool bFBuff = false );

private:
	float	m_fLEFT_TIME;
	float	m_fLIFE_TIME;

private:
	CString				m_strSkillName;
	CBasicMiniBarGauge*		m_pProgressBar;
	CSkillImage*		m_pSkillImage;
public:
	SNATIVEID m_sSKILLID;
	bool	  m_bFBuff;
};