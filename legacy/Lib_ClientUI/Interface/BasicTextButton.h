//	기본 정보 표시
//
//	최초 작성자 : 성기엽
//	이후 수정자 : 
//	로그
//		[2003.11.27]
//			@ 작성
//

#pragma	once

#include "BasicButton.h"
#include "UITextControl.h"

class CBasicTextButtonImage;
class CBasicTextBox;

class	CBasicTextButton : public CBasicButton
{
public:
	enum
	{
		SIZE14 = 0,
		SIZE16,
		SIZE16s,
		SIZE18,
		SIZE19,
		SIZE19_RECT,
		SIZE22,

		/////////////////////////////////////////////////////////////////////////////
		// LG-7 AutoSystem
		SIZE23,
		SIZE24,
		SIZE40,
		SIZE_NEW,
		SIZE41,
		
		SIZE_NSIZE = 12
		/////////////////////////////////////////////////////////////////////////////
	};

public:
	CBasicTextButton ();
	virtual	~CBasicTextButton ();

protected:
	virtual	void	SetFlipPosition ();
	virtual	void	ResetFlipPosition ();

public:
	void	SetOneLineText ( CString strTemp ,const D3DCOLOR& TextColor = NS_UITEXTUTIL::ENABLE );
	void	SetAlignText( int Align );

public:
	virtual HRESULT Render ( LPDIRECT3DDEVICEQ pd3dDevice );

public:
	virtual	void	SetFlip ( BOOL bFlip );

public:
	void	CreateBaseButton (	const char* szButtonControl, 
								int nType, 
								WORD wFlipMode, 
								const char* szText, 
								DWORD dwFlags=0L,
								DWORD dwColor = NS_UITEXTCOLOR::ENABLE );

	void	CreateBaseButtonEx (	const char* szButtonControl, 
								int nType, 
								WORD wFlipMode, 
								const char* szText, 
								DWORD dwFlags=0L );

	void	CreateTextBox( char* szControlKeyword, CD3DFontPar* pFont, int nAlign );

private:
	void	SetRadioButton ();
	void	SetGeneralButton ();
	void	SetRadioButtonEx ();
	void	SetGeneralButtonEx ();

private:
	void	CreateButtonImage ( int nType, CD3DFontPar* pFont, BOOL bReversal );

private:
	CBasicTextButtonImage*	m_pButton;
	CBasicTextButtonImage*	m_pButtonFlip;

	CBasicTextBox*		m_pButtonText;
	CUIControl*			m_pButtonTextBack;

	D3DCOLOR	m_TextColor;
	D3DCOLOR	m_TextFlipColor;
};