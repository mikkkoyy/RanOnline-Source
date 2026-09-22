#include "StdAfx.h"
#include "MiniPartyLinkSlot.h"
#include "../../Lib_Engine/DxCommon/DxFontMan.h"
#include "GameTextControl.h"
#include "UITextControl.h"
#include "BasicLineBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMiniPartyLinkSlot::CMiniPartyLinkSlot () :
	m_pPlayerNum ( NULL ),
	m_pPlayerName ( NULL ),
	m_pPlayerLevel ( NULL ),
	m_pPlayerClass ( NULL )
{
}

CMiniPartyLinkSlot::~CMiniPartyLinkSlot ()
{
}

void CMiniPartyLinkSlot::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );
	int nTextAlign = TEXT_ALIGN_CENTER_X;
	D3DCOLOR dwFontColor = NS_UITEXTCOLOR::DEFAULT;
	D3DCOLOR dwNameColor = NS_UITEXTCOLOR::PARTYNAME;

	CBasicLineBox* pLineBoxCombo = new CBasicLineBox;
	pLineBoxCombo->CreateSub ( this, "BASIC_LINE_BOX_MINIPARTY", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBoxCombo->CreateBaseBoxMiniParty ( "MINIPARTYLINK_SLOT_BACK" );
	RegisterControl ( pLineBoxCombo );
	
	m_pPlayerNum   = CreateStaticControl( "MINIPARTYLINK_SLOT_PLAYER_NUM", pFont9, dwNameColor, nTextAlign );
	m_pPlayerName  = CreateStaticControl( "MINIPARTYLINK_SLOT_PLAYER_NAME", pFont9, dwNameColor, nTextAlign );
	m_pPlayerLevel = CreateStaticControl( "MINIPARTYLINK_SLOT_PLAYER_LEVEL", pFont9, dwNameColor, nTextAlign );
	m_pPlayerClass = CreateStaticControl( "MINIPARTYLINK_SLOT_PLAYER_CLASS", pFont9, dwNameColor, nTextAlign );


}

CUIControl*	CMiniPartyLinkSlot::CreateControl ( const char* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl );
	RegisterControl ( pControl );
	return pControl;
}

CBasicTextBox* CMiniPartyLinkSlot::CreateStaticControl ( char* szControlKeyword, CD3DFontPar* pFont, D3DCOLOR dwColor, int nAlign )
{
	CBasicTextBox* pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, szControlKeyword );
	pStaticText->SetFont ( pFont );
	pStaticText->SetTextAlign ( nAlign );
	RegisterControl ( pStaticText );
	return pStaticText;
}

void CMiniPartyLinkSlot::SetPlayerData ( int nSlotNumber, CString strName, int nLevel, int nClass )
{
	ReSetPlayerData();
	CString strText;
	if(m_pPlayerNum)
	{
		if ( nSlotNumber==0)	strText.Format("M");
		else	strText.Format("%d",nSlotNumber);

		m_pPlayerNum->SetOneLineText (strText.GetString());
	}
	if(m_pPlayerName)
	{
		if ( strName!="" ) m_pPlayerName->SetOneLineText (strName.GetString());
	}
	if(m_pPlayerLevel)
	{
		if ( nLevel==0)	strText.Format("1");
		else	strText.Format("Lv .%d",nLevel);
		m_pPlayerLevel->SetOneLineText (strText.GetString());
	}
	if(m_pPlayerClass)
	{
		m_pPlayerClass->SetOneLineText (COMMENT::CHARCLASS[CharClassToIndex( (EMCHARCLASS)nClass )].c_str());
	}
}

void CMiniPartyLinkSlot::ReSetPlayerData ()
{
	if(m_pPlayerNum)m_pPlayerNum->ClearText();
	if(m_pPlayerName)m_pPlayerName->ClearText();
	if(m_pPlayerLevel)m_pPlayerLevel->ClearText();
	if(m_pPlayerClass)m_pPlayerClass->ClearText();
}

void CMiniPartyLinkSlot::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	switch ( ControlID )
	{
	case HP_BAR:
		{
			if ( CHECK_MOUSEIN_RBUPLIKE ( dwMsg ) )
			{
				AddMessageEx ( UIMSG_MOUSEIN_RB_EVENT );
			}
		}
		break;
	}
}