#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CBasicTextBox;

const DWORD UIMSG_CAPTURE_THE_FLAG_SCORE_WINDOW_LBDUP = UIMSG_USER1;

class CPVPCaptureTheFlagScore : public CUIGroup
{
private:
	enum
	{
		CAPTURE_THE_FLAG_STATUS_BOX1 = NO_ID + 1,
		CAPTURE_THE_FLAG_STATUS_BOX2,
		CAPTURE_THE_FLAG_STATUS_BOX3,
		CAPTURE_THE_FLAG_STATUS_BOX4,
	};

public:
	CPVPCaptureTheFlagScore ();
	virtual	~CPVPCaptureTheFlagScore ();

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
	float		m_fBLINK_TIME_A;
	float		m_fBLINK_TIME_B;

public:
	CBasicTextBox*		m_pTextTeamLeft;
	CBasicTextBox*		m_pTextTeamRight;
	CBasicTextBox*		m_pTextScoreLeft;
	CBasicTextBox*		m_pTextScoreRight;
	CBasicTextBox*		m_pTextMaxScore;
	CBasicTextBox*		m_pTextTimer;
	CUIControl*			m_pImageWinLeft;
	CUIControl*			m_pImageLoseLeft;
	CUIControl*			m_pImageWinRight;
	CUIControl*			m_pImageLoseRight;

	CUIControl*			m_pImageFlagLeft;
	CUIControl*			m_pImageFlagRight;


public:
	void	UpdateInfo();

};