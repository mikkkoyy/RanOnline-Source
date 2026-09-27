#pragma	once

#include "../UIWindowEx.h"
#include "GLCharDefine.h"
#include "../Lib_Client/G-Logic/GLogicEx.h"

class	CD3DFontPar;
class	CBasicProgressBar;//qweq
class	CBasicTextBox;
class	CBasicButtonText;
class	CBasicButton;//qweq
class	CCharacterWindowCharStatMark;
struct	GLCHARLOGIC;

class CCharacterWindowCharAdditionalInfo : public CUIWindowEx
{
protected:
	enum
	{
		CHARACTER_STAT_HELP = ET_CONTROL_NEXT + 1,

		MOUSE_OVER_STAT_2_CRITICAL,
		MOUSE_OVER_STAT_2_STRIKE,
		MOUSE_OVER_STAT_2_MOVINGSPEED,
		MOUSE_OVER_STAT_2_ATTACKSPEED,
		MOUSE_OVER_STAT_2_EXP,
		MOUSE_OVER_STAT_2_HPRECOVERY,
		MOUSE_OVER_STAT_2_MPRECOVERY,
		MOUSE_OVER_STAT_2_SPRECOVERY,
		MOUSE_OVER_STAT_2_CPRECOVERY,
		MOUSE_OVER_STAT_2_POTIONHP,
		MOUSE_OVER_STAT_2_POTIONMP,
		MOUSE_OVER_STAT_3_POTIONSP,

		MOUSE_OVER_BUTTONS = 12,
	};
	enum
	{
		STATE_QUESTION_BUTTON = ET_CONTROL_NEXT+1,
		STATE_OVER_RECT_CRITICAL,
		STATE_OVER_RECT_CRUSHING_BLOW,
		STATE_OVER_RECT_MOVE_SPEED,
		STATE_OVER_RECT_ATK_SPEED,
		STATE_OVER_RECT_EXP_MULTIPLE,
		STATE_OVER_RECT_INC_PER_HP,
		STATE_OVER_RECT_INC_PER_MP,
		STATE_OVER_RECT_INC_PER_SP,
		STATE_OVER_RECT_INC_PER_CP,
		STATE_OVER_RECT_INC_INT_HP,
		STATE_OVER_RECT_INC_INT_MP,
		STATE_OVER_RECT_INC_INT_SP,
	};

	//
	enum
	{
		STATE_CRITICAL = 0,
		STATE_CRUSHING_BLOW = 1,
		STATE_MOVE_SPEED = 2,
		STATE_ATK_SPEED = 3,
		STATE_EXP_MULTIPLE = 4,
		STATE_INC_PER_HP = 5,
		STATE_INC_PER_MP = 6,
		STATE_INC_PER_SP = 7,
		STATE_INC_PER_CP = 8,
		STATE_INC_INT_HP = 9,
		STATE_INC_INT_MP = 10,
		STATE_INC_INT_SP = 11,
		STAT_MAX = 12
	};

public:
    CCharacterWindowCharAdditionalInfo ();
	virtual	~CCharacterWindowCharAdditionalInfo ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );
	virtual void SetGlobalPos(const D3DXVECTOR2& vPos);

public:
	void	CreateSubControl ();
	void 	StateClearText();

private:
	CBasicTextBox*	CreateStaticControl ( char* szConatrolKeyword, CD3DFontPar* pFont, int nAlign, const UIGUID& cID = NO_ID );
	CUIControl*		CreateControl ( const char* szControl, const UIGUID& cID = NO_ID );
	CBasicButtonText*	CreateButtonBlue ( char* szButton, char* szTextBox, CD3DFontPar* pFont, int nAlign, UIGUID ControlID, CString strText );
	CBasicButtonText*	CreateButtonRed ( char* szButton, char* szTextBox, CD3DFontPar* pFont, int nAlign, UIGUID ControlID, CString strText );
	CBasicButton*	CreateMouseIn( const char* szButton, const char* szMouseIn ,UIGUID ControlID );
	CCharacterWindowCharStatMark*		CreateStatMark( const char* szControl, const UIGUID& cID = NO_ID );

protected:
	CBasicTextBox*	 CreateText( const std::string& text,const std::string& key,WORD wAlignFlag,UIGUID id = NO_ID);
    CBasicTextBox*	 CreateText( const std::string& text,const std::string& key,WORD wAlignFlag,D3DCOLOR d3dColor, UIGUID id = NO_ID);
	CUIControl*      CreateControl( const std::string& key,UIGUID id = NO_ID);
	CUIControl*      CreateOverRect( const std::string& key,UIGUID id = NO_ID);
	CUIControl*      CreateLineBox( const std::string& strControl, const std::string& strTexInfo,UIGUID id = NO_ID );
	CBasicButton*	 CreateFlipButton( char* szButton, char* szButtonFlip, UIGUID ControlID, WORD wFlipType );


private:
	void	SetPointControl ();

private:
	void	SetData ( const GLCHARLOGIC& sCharData );
	void	SetUpData ( const GLCHARLOGIC& sCharData );
	void	ShowTip( int nSelect );

	void CreateOverRext();

	CBasicTextBox* m_pStatValue[STAT_MAX];
	CBasicTextBox* m_pStatUpValue[STAT_MAX];
	
	void PrintTooltip( UIGUID ControlID, DWORD dwMsg );

protected:
	GLCHARLOGIC		m_sCharLogic;

private:
	typedef std::map<UIGUID,CUIControl*> OverRectMap;
	OverRectMap    m_OverRectMap;

	CUIControl*				m_pBackGround;

	CBasicTextBox*	m_pCriticalRateText;
	CBasicTextBox*	m_pStrikeHitText;
	CBasicTextBox*	m_pMovingSpeedText;
	CBasicTextBox*	m_pAttackSpeedText;
	CBasicTextBox*	m_pEXPText;
	CBasicTextBox*	m_pHPRecoveryText;
	CBasicTextBox*	m_pMPRecoveryText;
	CBasicTextBox*	m_pSPRecoveryText;
	CBasicTextBox*	m_pCPRecoveryText;
	CBasicTextBox*	m_pPotionHPText;
	CBasicTextBox*	m_pPotionMPText;
	CBasicTextBox*	m_pPotionSPText;

	CBasicButton*	m_pHelp;

	CBasicButton*	m_pMouseOver[MOUSE_OVER_BUTTONS];

	CBasicTextBox*	m_pCriticalRateMarkStatic;
	CBasicTextBox*	m_pStrikeHitMarkStatic;
	CBasicTextBox*	m_pMovingSpeedMarkStatic;
	CBasicTextBox*	m_pAttackSpeedMarkStatic;
	CBasicTextBox*	m_pEXPMarkStatic;
	CBasicTextBox*	m_pHPRecoveryMarkStatic;
	CBasicTextBox*	m_pMPRecoveryMarkStatic;
	CBasicTextBox*	m_pSPRecoveryMarkStatic;
	CBasicTextBox*	m_pCPRecoveryMarkStatic;
	CBasicTextBox*	m_pPotionHPMarkStatic;
	CBasicTextBox*	m_pPotionMPMarkStatic;
	CBasicTextBox*	m_pPotionSPMarkStatic;

	CBasicTextBox*	m_pCriticalRateMarkValue;
	CBasicTextBox*	m_pStrikeHitMarkValue;
	CBasicTextBox*	m_pMovingSpeedMarkValue;
	CBasicTextBox*	m_pAttackSpeedMarkValue;
	CBasicTextBox*	m_pEXPMarkValue;
	CBasicTextBox*	m_pHPRecoveryMarkValue;
	CBasicTextBox*	m_pMPRecoveryMarkValue;
	CBasicTextBox*	m_pSPRecoveryMarkValue;
	CBasicTextBox*	m_pCPRecoveryMarkValue;
	CBasicTextBox*	m_pPotionHPMarkValue;
	CBasicTextBox*	m_pPotionMPMarkValue;
	CBasicTextBox*	m_pPotionSPMarkValue;

	CCharacterWindowCharStatMark*	m_pCriticalRateMark;
	CCharacterWindowCharStatMark*	m_pStrikeHitMark;
	CCharacterWindowCharStatMark*	m_pMovingSpeedMark;
	CCharacterWindowCharStatMark*	m_pAttackSpeedMark;
	CCharacterWindowCharStatMark*	m_pEXPMark;
	CCharacterWindowCharStatMark*	m_pHPRecoveryMark;
	CCharacterWindowCharStatMark*	m_pMPRecoveryMark;
	CCharacterWindowCharStatMark*	m_pSPRecoveryMark;
	CCharacterWindowCharStatMark*	m_pCPRecoveryMark;
	CCharacterWindowCharStatMark*	m_pPotionHPMark;
	CCharacterWindowCharStatMark*	m_pPotionMPMark;
	CCharacterWindowCharStatMark*	m_pPotionSPMark;

public:
	void	InitData();
	DWORD	GetTextColor( float nVal );
};