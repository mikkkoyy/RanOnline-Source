#include "StdAfx.h"
#include "CompetitionNotifyButtonAlarm.h"
#include "./BasicButton.h"
#include "./BasicTextBox.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "./UITextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCompetitionNotifyButtonAlarm::CCompetitionNotifyButtonAlarm ()
	/*:*/ 
{
}

CCompetitionNotifyButtonAlarm::~CCompetitionNotifyButtonAlarm ()
{
}

void CCompetitionNotifyButtonAlarm::CreateSubControl ()
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont( _DEFAULT_FONT, 11 );

	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, "COMPETITION_NOTIFY_ALARM_HEAD", UI_FLAG_DEFAULT );
	pControl->SetVisibleSingle( TRUE );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "COMPETITION_NOTIFY_ALARM_BODY", UI_FLAG_DEFAULT );
	pControl->SetVisibleSingle( TRUE );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "COMPETITION_NOTIFY_ALARM_TAIL", UI_FLAG_DEFAULT );
	pControl->SetVisibleSingle( TRUE );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "COMPETITION_NOTIFY_ALARM_ARROW", UI_FLAG_DEFAULT );
	pControl->SetVisibleSingle( TRUE );
	RegisterControl ( pControl );

	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, "COMPETITION_NOTIFY_ALARM_TEXT", UI_FLAG_DEFAULT );
	pTextBox->SetVisibleSingle( TRUE );
	pTextBox->SetFont ( pFont );
	pTextBox->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );	
	pTextBox->AddText("!", NS_UITEXTCOLOR::BLACK );
	RegisterControl ( pTextBox );
	
}
