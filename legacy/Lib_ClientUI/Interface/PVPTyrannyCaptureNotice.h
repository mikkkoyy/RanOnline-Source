#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CPVPTyrannyCaptureNotice : public CUIGroup
{
private:
	enum
	{
		
	};


public:
	CPVPTyrannyCaptureNotice ();
	virtual	~CPVPTyrannyCaptureNotice ();

public:
	void	CreateSubControl ( int nType );

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

	CUIControl*		m_pImage;
	CUIControl*		m_pSchoolImage;
	CUIControl*		m_pSchoolText;

	float			m_fTimer;
};