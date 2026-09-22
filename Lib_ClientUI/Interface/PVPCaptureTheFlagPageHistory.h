#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CBasicTextBox;
class CBasicLineBox;
class CD3DFontPar;
class CBasicButton;

class CPVPCaptureTheFlagPageHistory : public CUIGroup
{
private:
	enum
	{
		CAPTURE_THE_FLAG_PAGE_INFO_HISTORY_BUTTON_RANKING = NO_ID + 1,
	};

public:
	CPVPCaptureTheFlagPageHistory ();
	virtual	~CPVPCaptureTheFlagPageHistory ();

public:
	void	CreateSubControl ();
	CBasicTextBox*	CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign );
	CUIControl*		CreateUIControl( const char* szControl );

public:
	CBasicTextBox*		m_pTextHistoryTimeStatic;
	CBasicTextBox*		m_pTextHistoryAuthStatic;
	CBasicTextBox*		m_pTextHistoryRankingStatic;

	CBasicTextBox*		m_pTextHistoryTimeText;
	CBasicTextBox*		m_pTextHistoryAuthText;
	CBasicTextBox*		m_pTextHistoryRankingText;

	CBasicButton*		m_pButtonRanking;

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

	float	m_fUpdateTime;

	void	ShowInfo();
};