#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLPVPTyrannyData.h"

class CBasicTextBox;

const DWORD UIMSG_PVPTYRANNY_TOWER_CAPTURE_LBDUP = UIMSG_USER1;

class CPVPTyrannyTowerCapture : public CUIGroup
{
private:
	enum
	{
		TOWER_STATUS_BOX1 = NO_ID + 1,
		TOWER_STATUS_BOX2
	};

	enum
	{
		TOWER_STATUS_TYPES = TYRANNY_TOWER_SIZE+1,
	};

public:
	CPVPTyrannyTowerCapture ();
	virtual	~CPVPTyrannyTowerCapture ();

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

public:
	CBasicTextBox*	m_pTextTower0;
	CBasicTextBox*	m_pTextTower1;
	CBasicTextBox*	m_pTextTower2;

	CUIControl*		m_pStatusTower0[TOWER_STATUS_TYPES];
	CUIControl*		m_pStatusTower1[TOWER_STATUS_TYPES];
	CUIControl*		m_pStatusTower2[TOWER_STATUS_TYPES];

	CBasicTextBox*	m_pTextStatus;
	CBasicTextBox*	m_pTextTimer;
	CUIControl*		m_pStatusWinner[TOWER_STATUS_TYPES];

public:
	void	UpdateInfo();

};