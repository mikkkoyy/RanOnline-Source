/*!
 * \file CharacterInfoViewWindow.h
 *
 * \author Juver
 * \date November 2017
 *
 * 
 */

#pragma once

#include "UIWindowEx.h"

#include "GLCharData.h"

class CMultiModeButton;
class CCharacterInfoViewPageStats;

class CCharacterInfoViewWindow : public CUIWindowEx
{
	enum
	{
		CHARACTER_INFO_VIEW_BUTTON_STATS = ET_CONTROL_NEXT,
		CHARACTER_INFO_VIEW_PAGE_STATS,
	};

	enum
	{
		CHARACTER_INFO_VIEW_WINDOW_PAGE_1 = 0,
	};

public:
	CCharacterInfoViewWindow(void);
	virtual ~CCharacterInfoViewWindow(void);

public:
	CMultiModeButton*	CreateTabButton( const char* szButtonBox, const char* szButtonText, UIGUID ControlID );
	void CreateSubControl();

public:
	CMultiModeButton*					m_pButtonStats;
	CCharacterInfoViewPageStats*		m_pPageStats;

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage( UIGUID cID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

public:
	int		m_nPage;

public:
	SINFO_CHAR		m_sInfoChar;

public:
	void	OpenPage( int nPage );
	void	SetCharacterData( SINFO_CHAR* pInfoChar );
};