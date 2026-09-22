#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLPVPSchoolWarsData.h"

class CBasicTextBox;
class CBasicLineBox;
class CD3DFontPar;
class CBasicButton;

class CPVPSchoolWarsPageHistory : public CUIGroup
{
private:
	enum
	{
		SCHOOLWARS_PAGE_INFO_HISTORY_BUTTON_RANKING = NO_ID + 1,
	};

public:
	CPVPSchoolWarsPageHistory ();
	virtual	~CPVPSchoolWarsPageHistory ();

public:
	void	CreateSubControl ();
	CBasicTextBox*	CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign );
	CUIControl*		CreateUIControl( const char* szControl );

public:
	CBasicTextBox*		m_pTextHistoryTitleStatic;
	CBasicTextBox*		m_pTextHistoryTimeStatic;
	CBasicTextBox*		m_pTextHistoryAuthStatic;
	CBasicTextBox*		m_pTextHistoryRankingStatic;

	CBasicTextBox*		m_pTextHistoryTimeText;
	CBasicTextBox*		m_pTextHistoryAuthText;
	CBasicTextBox*		m_pTextHistoryRankingText;

	CUIControl*			m_pAuthSchoolImage[SCHOOLWARS_SCHOOL_SIZE+1];

	CBasicButton*		m_pButtonRanking;

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

	float	m_fUpdateTime;

	void	ShowInfo();
};