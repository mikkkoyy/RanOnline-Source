/*!
 * \file MultiModeButton.h
 *
 * \author Juver
 * \date February 2017
 *
 * 
 */

#pragma once

#include "UIGroup.h"

class CBasicTextBox;
class CMultiModeButtonImage;

class CMultiModeButton : public CUIGroup
{
public:
static	const	float	fBUTTON_CLICK;

public:
	CMultiModeButton();
	virtual ~CMultiModeButton();

private:
	CBasicTextBox*	m_pTextBox;
	CUIControl*	m_pImageBase;
	CUIControl*	m_pImageFlip;
	CUIControl*	m_pImageHold;
	CUIControl*	m_pImageOver;
	CMultiModeButtonImage*	m_pImageBaseMulti;
	CMultiModeButtonImage*	m_pImageFlipMulti;
	CMultiModeButtonImage*	m_pImageHoldMulti;
	CMultiModeButtonImage*	m_pImageOverMulti;

protected:
	BOOL	m_bActionFlip;
	BOOL	m_bActionHold;
	BOOL	m_bActionOver;

protected:
	BOOL	m_bUseActionFlip;
	BOOL	m_bUseActionDown;
	BOOL	m_bUseGlobalAction;
	BYTE	m_KeyCode;
	BYTE	m_KeyCodeEx;
	DWORD	m_dwTextColorFlip;
	DWORD	m_dwTextColorHold;
	DWORD	m_dwTextColorOver;

public:
	void	CreateImageBase( const char* szControl );
	void	CreateImageFlip( const char* szControl );
	void	CreateImageHold( const char* szControl );
	void	CreateImageOver( const char* szControl );

	void	CreateImageBaseMulti ( const char* szControl, const char* szLeft, const char* szCenter, const char* szRight );
	void	CreateImageFlipMulti ( const char* szControl, const char* szLeft, const char* szCenter, const char* szRight );
	void	CreateImageHoldMulti ( const char* szControl, const char* szLeft, const char* szCenter, const char* szRight );
	void	CreateImageOverMulti ( const char* szControl, const char* szLeft, const char* szCenter, const char* szRight );

	void	CreateTextBox( const char* szControl, int nTextSize, int nTextAlign, DWORD dwFlags=0L );
	void	AlignToControl( const char* szControl );
	void	SetTextColor( DWORD dwFlip, DWORD dwHold, DWORD dwOver );
	void	SetOneLineText ( CString strTemp, DWORD dwColor );

	void SetUseActionFlip( BOOL bUse )		{ m_bUseActionFlip = bUse; }
	BOOL IsUseActionFlip()					{ return m_bUseActionFlip; }

	void SetUseActionDown( BOOL bUse )		{ m_bUseActionDown = bUse; }
	BOOL IsUseActionDown()					{ return m_bUseActionDown; }

	void SetUseGlobalAction( BOOL bUseGlobalAction )	{ m_bUseGlobalAction = bUseGlobalAction; }
	BOOL IsUseGlobalAction()							{ return m_bUseGlobalAction; }

	void SetShortcutKey ( BYTE nKeyCode, BYTE nKeyCodeEx = NULL )
	{ m_KeyCode = nKeyCode; m_KeyCodeEx = nKeyCodeEx; }

public:
	virtual	void	DoImageFlip( BOOL bSHOW );
	virtual	void	DoImageHold( BOOL bSHOW );
	virtual	void	DoImageOver( BOOL bSHOW );
	virtual	void	DoImageDown( BOOL bSHOW );

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

private:
	void	TranslateMouseMessage ( DWORD dwMsg );
};
