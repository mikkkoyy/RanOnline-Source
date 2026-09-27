#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CBasicButton;
class CBasicTextBox;
class CUIEditBox;
class CBasicLineBox;
class CBasicProgressBar;

class CCharacterCreatePageStyle : public CUIGroup
{
private:
	static	const int nLIMITCHAR;

private:
	enum
	{
		CHARACTER_CREATE_PAGE_STYLE_FACETYPE_LEFT = NO_ID + 1,
		CHARACTER_CREATE_PAGE_STYLE_FACETYPE_RIGHT,
		CHARACTER_CREATE_PAGE_STYLE_HAIRSTYLE_LEFT,
		CHARACTER_CREATE_PAGE_STYLE_HAIRSTYLE_RIGHT,
		CHARACTER_CREATE_PAGE_STYLE_HAIRCOLOR_LEFT,
		CHARACTER_CREATE_PAGE_STYLE_HAIRCOLOR_RIGHT,
		CHARACTER_CREATE_PAGE_STYLE_CHARSCALE_LEFT,
		CHARACTER_CREATE_PAGE_STYLE_CHARSCALE_RIGHT,
		CHARACTER_CREATE_PAGE_STYLE_CHARNAME_EDIT,
	};

public:
	CCharacterCreatePageStyle ();
	virtual	~CCharacterCreatePageStyle ();

public:
	void CreateSubControl ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

private:
	CBasicButton*	m_pFaceTypeButtonLeft;
	CBasicButton*	m_pFaceTypeButtonRight;
	CBasicButton*	m_pHairTypeButtonLeft;
	CBasicButton*	m_pHairTypeButtonRight;
	CBasicButton*	m_pHairColorButtonLeft;
	CBasicButton*	m_pHairColorButtonRight;
	CBasicButton*	m_pCharScaleButtonLeft;
	CBasicButton*	m_pCharScaleButtonRight;
	CBasicTextBox*	m_pStylePageStatic;
	CBasicTextBox*	m_pFaceTypeStatic;
	CBasicTextBox*	m_pHairTypeStatic;
	CBasicTextBox*	m_pHairColorStatic;
	CBasicTextBox*	m_pCharScaleStatic;
	CBasicTextBox*	m_pNameStatic;
	CBasicLineBox*	m_pHairColorLineBox;
	CBasicTextBox*	m_pFaceTypeTextBox;
	CBasicTextBox*	m_pHairTypeTextBox;
	CBasicProgressBar*	m_pCharacterScaleBar;
	CUIEditBox*		m_pNameEditBox;

private:
	int		m_nFaceTypeBack;
	int		m_nHairTypeBack;
	WORD	m_wHairColorBack;
	float	m_fCharScaleBack;
	int		m_nHairColorIndex;
	float	m_fCharScaleBase;

public:
	void	ResetAll();
	CUIEditBox*		GetEditBox() { return m_pNameEditBox; }
	void	HairColorNext();
	void	HairColorPrev();
	void	CharacterScaleIncrease();
	void	CharacterScaleDecrease();
	void	ResetNameEdit();
};