#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CPVPCaptureTheFlagBattleNotice : public CUIGroup
{
private:
	enum
	{
		
	};


public:
	CPVPCaptureTheFlagBattleNotice ();
	virtual	~CPVPCaptureTheFlagBattleNotice ();

public:
	void	CreateSubControlStartImage ();
	void	CreateSubControlEndImage ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

	CUIControl*		m_pImageBG;
	CUIControl*		m_pImage;
	CUIControl*		m_pImageIcon;

	float			m_fTimer;
};