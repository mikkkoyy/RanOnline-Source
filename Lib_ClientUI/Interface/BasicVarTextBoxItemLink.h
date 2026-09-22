
#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLItem.h"
#include "BasicTextBox.h"

class	CBasicTextBox;
class	CBasicLineBoxEx;
class	CBasicButton;
class	CD3DFontPar;
class	CItemImage;

struct	SNATIVEID;

class	CBasicVarTextBoxItemLink : public CUIGroup
{
private:
	static	const	float	fMOUSEPOINT_GAP;

protected:
	enum
	{
		BUTTON_CLOSE = NO_ID + 1,
	};

public:
	CBasicVarTextBoxItemLink ();
	virtual	~CBasicVarTextBoxItemLink ();

public:
	void	CreateSubControl ();

public:
	void	ClearText ();
	int		AddTextNoSplitBig ( CString strText, D3DCOLOR dwColor );
	int		AddTextNoSplit ( CString strText, D3DCOLOR dwColor );
	int		SetTextNoSplit ( CString strText, D3DCOLOR dwColor );
	void	SetText( CString strText, D3DCOLOR	dwColor );
	void	AddString ( int nIndex, const CString& strText, const D3DCOLOR& dwColor );
	int		AddText ( CString strText, D3DCOLOR dwColor );
	void	AddTextLongestLineSplit ( CString strText, D3DCOLOR dwColor );

public:
	void	SetUseOverColor ( bool bUseColor );
	void	SetOverColor ( const D3DCOLOR& dwColor );
	void	SetLineInterval ( const float fLineInterval );

public:
	int		GetCount ();

	void	RePosControl ( int x, int y );

public:
	void	SetBlockMouseTracking ( bool bBlock );

public:
	void	SetMousePointGap ( D3DXVECTOR2 vGap );

public:
	float	GetLongestLine ();

	void	SetTextAlign ( int nALIGN );

	void	ShowCloseButton( bool bSHOW );

	CItemImage*	CreateItemImage		( const char* szControl, UIGUID controlID );
	CItemImage*						m_pItemImageRender;

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

private:
	bool	m_bBLOCK_MOUSETRACKING;

private:
	CBasicTextBox*		m_pTextBox;
	CBasicTextBox*		m_pTextBoxName;
	CBasicLineBoxEx*	m_pLineBox;
	CBasicButton*		m_pCloseButton;
	D3DXVECTOR2			m_vMousePointGap;

private:
	CUIControl*			m_pSelfDummy;

public:
	int		AddMultiText ( CBasicTextBox::MULTITEXT mlText ); //Add MultiText by NaJ
	int		AddMultiTextLongestLineSplit ( CBasicTextBox::MULTITEXT mlText ); //Add MultiTextLongestLine by NaJ

private:
	CD3DFontPar*	m_pFont;
	CD3DFontPar*	m_pFont12;
};