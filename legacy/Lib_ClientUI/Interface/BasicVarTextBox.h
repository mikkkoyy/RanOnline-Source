//	기본 정보 표시
//
//	최초 작성자 : 성기엽
//	이후 수정자 : 
//	로그
//		[2003.11.27]
//			@ 작성
//

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "BasicTextBox.h"
#include "GLItem.h"
class	CBasicTextBox;
class	CBasicLineBoxEx;
class	CItemImage;
class	CD3DFontPar;
class	GLItem;

struct	SNATIVEID;

class	CBasicVarTextBox : public CUIGroup
{
private:
static	const	float	fMOUSEPOINT_GAP;

public:
	CBasicVarTextBox ();
	virtual	~CBasicVarTextBox ();

public:
	void	CreateSubControl ();

public:
	void	ClearText ();
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

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual void SetGlobalPos(const D3DXVECTOR2& vPos);

private:
	bool	m_bBLOCK_MOUSETRACKING;

private:
	CBasicTextBox*		m_pTextBox;
	CBasicLineBoxEx*	m_pLineBox;
	D3DXVECTOR2			m_vMousePointGap;

private:
	CUIControl*			m_pSelfDummy;

	CD3DFontPar*		m_pFont12;
public:

	//skill render
	void SetSkillRender(SNATIVEID sICONINDEX, const char* szTexture, SITEMCUSTOM sCustom);
	void ResetSkillRender ();

	int	 AddTextNoSplitBig ( CString strText, D3DCOLOR dwColor );

private:
	CItemImage*	CreateItemImage ( const char* szControl );
	CItemImage*			m_pItemImage; 
	CBasicTextBox*		m_pTextBoxName; 

	BOOL m_bLONG;
	BOOL IsLong()	{ return m_bLONG; }
	void SetLong(BOOL bLONG) { m_bLONG = bLONG; }
public:
	int		AddMultiText ( CBasicTextBox::MULTITEXT mlText ); //Add MultiText by NaJ
	int		AddMultiTextLongestLineSplit ( CBasicTextBox::MULTITEXT mlText ); //Add MultiTextLongestLine by NaJ

private:
	CD3DFontPar*	m_pFont;
};