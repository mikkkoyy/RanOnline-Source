#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLPVPSchoolWarsData.h"

class CBasicTextBox;
class CBasicProgressBar;

const DWORD UIMSG_SCHOOLWARS_SCORE_WINDOW_LBDUP = UIMSG_USER1;

class CPVPSchoolWarsScore : public CUIGroup
{
private:
	enum
	{
		SCHOOWARS_STATUS_BOX1 = NO_ID + 1,
		SCHOOWARS_STATUS_BOX2
	};

public:
	CPVPSchoolWarsScore ();
	virtual	~CPVPSchoolWarsScore ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

public:
	void	CheckMouseState();
	void	CreateSubControl ();

private:
	BOOL		m_bCHECK_MOUSE_STATE;
	BOOL		m_bFirstGap;
	D3DXVECTOR2	m_vGap;
	int			m_nPosX;
	int			m_nPosY;
	float		m_fUpdateTime;
	float		m_fImageTime;

public:
	CBasicTextBox*	m_pTextTimer;
	CBasicTextBox*	m_pTextStatic0;
	CBasicTextBox*	m_pTextStatic1;
	CBasicTextBox*	m_pTextStatic2;
	CBasicTextBox*	m_pTextStaticDP;

	CBasicTextBox*	m_pTextScore0;
	CBasicTextBox*	m_pTextScore1;
	CBasicTextBox*	m_pTextScore2;
	CBasicTextBox*	m_pTextScoreDP;

	CBasicProgressBar*	m_pProgressTimer;
	CBasicProgressBar*	m_pProgress0;
	CBasicProgressBar*	m_pProgress1;
	CBasicProgressBar*	m_pProgress2;
	CBasicProgressBar*	m_pProgressDP;
	
	CUIControl*			m_pImage;
public:
	void	UpdateInfo();
	void	ToNotify();

};