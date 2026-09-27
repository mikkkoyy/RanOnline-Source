#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CPVPTyranny2CaptureNotice : public CUIGroup
{
private:
	enum
	{
		
	};


public:
	CPVPTyranny2CaptureNotice ();
	virtual	~CPVPTyranny2CaptureNotice ();

public:
	void	CreateSubControl ( int nType );

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

	CUIControl*		m_pImage;
	float			m_fTimer;
};