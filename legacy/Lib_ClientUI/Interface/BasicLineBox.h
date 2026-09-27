//	라인 박스 4조각
//
//	최초 작성자 : 성기엽
//	이후 수정자 : 
//	로그
//		[2003.11.31]
//			@ 작성
//

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class	CBasicLineBox : public CUIGroup
{
public:
	enum
	{
		LEFT = 1,
		RIGHT,
		TOP,
		BOTTOM	
	};

public:
	CBasicLineBox ();
	virtual	~CBasicLineBox ();

public: // MEMO
	void	CreateBaseBoxOption ( char* szBoxControl );
	void	CreateBaseBoxSkill ( char* szBoxControl );
	void	CreateBaseBoxCombo ( char* szBoxControl );
	void	CreateBaseBoxMiniParty ( char* szBoxControl );
	void	CreateBaseBoxModal ( char* szBoxControl );
	void	CreateBaseBoxDialogue ( char* szBoxControl );
	void	CreateBaseBoxDialogueAnswer ( char* szBoxControl );
	void	CreateBaseBoxOuter ( char* szBoxControl );
	void	CreateBaseBoxOuterFat ( char* szBoxControl );
	void	CreateBaseBoxMapMove ( char* szBoxControl );	
	void	CreateBaseBoxQuestList ( char* szBoxControl );
	void	CreateBaseBoxQuestStepDesc ( char* szBoxControl );
	void	CreateBaseBoxQuestStepItem ( char* szBoxControl );
	void	CreateBaseBoxQuestRunDesc ( char* szBoxControl );
	void	CreateBaseBoxQuestRunItem ( char* szBoxControl );
	void	CreateBaseBoxEditBox ( char* szBoxControl );
	void	CreateBaseBoxWaitServer ( char* szBoxControl );
	void	CreateBaseBoxSystemMessage ( char* szBoxControl );
	void	CreateBaseBoxSystemMessageUp ( char* szBoxControl );
	void	CreateBaseBoxWhite ( char* szBoxControl );
	void	CreateBaseBoxWhiteBlankBody ( char* szBoxControl );
	void	CreateBaseBoxYellow ( char* szBoxControl );
	void	CreateBaseBoxManual( char* szBoxControl, char* szBody, char* szLeft, char* szRight, char* szTop, char* szBottom );

	/////////////////////////////////////////////////////////////////////////////
	// LG-7 GlobalRanking
	void	CreateBaseBoxOuterBlank(char* szBoxControl);
	void	CreateBaseBoxOuterBlankWhite(char* szBoxControl);
	void	CreateBaseBoxOuterWhite(char* szBoxControl);
	/////////////////////////////////////////////////////////////////////////////

public:
	void	SetUseRenderLine ( int nLine, BOOL bUseRender );

protected:
	CUIControl*		CreateControl ( char* szControl, WORD wAlignFlag );

protected:
	void	ResizeControl ( char* szBoxControl );
	void	CreateLineImage ( char* szLeft, char* szRight, char* szTop, char* szBottom );
	void	CreateBodyImage ( char* szBody );

public:
	void	ResetAlignFlagLine ();
	void	SetAlignFlagLine ( int nLine, WORD wAlignFlag );

protected:
	CUIControl*		m_pLeft;
	CUIControl*		m_pRight;
	CUIControl*		m_pTop;
	CUIControl*		m_pBottom;
};
