/*!
 * \file StudentCodexWindow.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */

#pragma	once

#include "UIWindowEx.h"

class CMultiModeButton;

/*codex system, Juver, 2017/10/30 */
class CCodexPage; 
//class CBattleRecordPage;
class CStudentCodexWindow: public CUIWindowEx
{
public:
	enum
	{
		STUDENT_RECORD_WINDOW_BUTTON_PAGE1 = ET_CONTROL_NEXT,
		STUDENT_RECORD_WINDOW_BUTTON_PAGE2,

		STUDENT_RECORD_WINDOW_PAGE1,
		STUDENT_RECORD_WINDOW_PAGE2,
	};


	enum
	{
		STUDENT_RECORD_WINDOW_PAGE_TEMP_1 = 0,
		STUDENT_RECORD_WINDOW_PAGE_TEMP_2 = 1,
	};

public:
	CStudentCodexWindow ();
	virtual	~CStudentCodexWindow ();

public:
	CMultiModeButton*		m_pButtonPage1;
	CMultiModeButton*		m_pButtonPage2;
	CCodexPage*			m_pPageCodex; /*codex system, Juver, 2017/10/30 */
	
	//CBattleRecordPage*			m_pPageBattleRecord;

public:
	void	CreateSubControl ();
	CMultiModeButton*	CreateTabButton( const char* szButtonBox, const char* szButtonText, UIGUID ControlID );

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

public:
	int		m_nPage;

public:
	void	OpenPage( int nPage );
	void	LoadData();

};