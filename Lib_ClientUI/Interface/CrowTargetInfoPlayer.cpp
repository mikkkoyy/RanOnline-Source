#include "StdAfx.h"
#include "CrowTargetInfoPlayer.h"
#include "InnerInterface.h"
#include "BasicLineBox.h"
#include "../Lib_Engine/DxCommon/d3dfont.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "BasicLineBox.h"
#include "BasicLineBoxEx.h"
#include "GameTextControl.h"
#include "UITextControl.h"
#include "BasicTextBox.h"

#include "../Lib_Client/G-Logic/GLGaeaClient.h"
#include "../Lib_Client/G-Logic/GLCrowData.h"
#include "../Lib_Client/G-Logic/GLCrow.h"
#include "../Lib_Client/G-Logic/GLCrowClient.h"
#include "../Lib_Client/G-Logic/GlogicEx.h"

#include "GLCharData.h"
#include "GLCharacter.h"
#include "GLChar.h"
#include "GLCharClient.h"
#include "GLogicData.h"
#include "GLCharDefine.h"
#include "UIInfoLoader.h"
#include "GLCrowRenList.h"
#include "GLStringTable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCrowTargetInfoPlayer::CCrowTargetInfoPlayer () :
	m_pNameBoxDummy ( NULL )
{
}

CCrowTargetInfoPlayer::~CCrowTargetInfoPlayer ()
{
}

void CCrowTargetInfoPlayer::CreateSubControl ()
{
	m_pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	/*m_pTargetBox = new CBasicLineBoxEx;
	m_pTargetBox->CreateSub ( this, "BASIC_LINE_BOX_EX_DIALOGUE_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pTargetBox->CreateBaseBoxVarTextBoxNew ( "RNCROW_TARGET_INFOBOX_PLAYER" );
	m_pTargetBox->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pTargetBox );*/

	CBasicLineBoxEx* m_pTargetBox = new CBasicLineBoxEx;
	m_pTargetBox->CreateSub ( this, "RNCROW_TARGET_INFOBOX_PLAYER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pTargetBox->CreateBaseBoxVarTextBox ( "BASIC_VAR_LINE_CHAR_BOX" );
	m_pTargetBox->SetDiffuse ( D3DCOLOR_ARGB (175,0xC0,0xC0,0xC0) );
	m_pTargetBox->SetNoUpdate ( true );
	RegisterControl ( m_pTargetBox );

	CBasicTextBox* m_pSchool = new CBasicTextBox;
	m_pSchool->CreateSub ( this, "SCHOOL_NAMEBOX_PLAYER" );
    m_pSchool->SetFont ( m_pFont8 );
	m_pSchool->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pSchool->SetControlNameEx ( "School Text" );	
	m_pSchool->SetAlignFlag ( UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	RegisterControl ( m_pSchool );
	m_pSchoolBox = m_pSchool;
	
	CBasicTextBox* m_pClass = new CBasicTextBox;
	m_pClass->CreateSub ( this, "CLASS_NAMEBOX_PLAYER" );
    m_pClass->SetFont ( m_pFont8 );
	m_pClass->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pClass->SetControlNameEx ( "Class Text" );	
	m_pClass->SetAlignFlag ( UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	RegisterControl ( m_pClass );
	m_pClassBox = m_pClass;

	CBasicTextBox* m_pReborn = new CBasicTextBox;
	m_pReborn->CreateSub ( this, "REBORN_NAMEBOX_PLAYER" );
    m_pReborn->SetFont ( m_pFont8 );
	m_pReborn->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pReborn->SetControlNameEx ( "Reborn Text" );	
	m_pReborn->SetAlignFlag ( UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	RegisterControl ( m_pReborn );
	m_pRebornBox  = m_pReborn ;

	CBasicTextBox* m_pGuild = new CBasicTextBox;
	m_pGuild->CreateSub ( this, "GUILD_NAMEBOX_PLAYER" );
    m_pGuild->SetFont ( m_pFont8 );
	m_pGuild->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pGuild->SetControlNameEx ( "Guild Text" );	
	m_pGuild->SetAlignFlag ( UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	RegisterControl ( m_pGuild );
	m_pGuildBox  = m_pGuild ;
	
	CBasicTextBox* m_pText = new CBasicTextBox;
	m_pText->CreateSub ( this, "TEXT_NAMEBOX_PLAYER" );
    m_pText->SetFont ( m_pFont8 );
	m_pText->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pText->SetControlNameEx ( "Text" );	
	m_pText->SetAlignFlag ( UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	RegisterControl ( m_pText );
	m_pTextBox = m_pText;

	CBasicTextBox* pNameBox = new CBasicTextBox;
	pNameBox->CreateSub ( this, "CROW_NAMEBOX_PLAYER" );
    pNameBox->SetFont ( m_pFont8 );
	pNameBox->SetTextAlign ( TEXT_ALIGN_LEFT );
	pNameBox->SetControlNameEx ( "Name Text" );	
	pNameBox->SetAlignFlag ( UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	RegisterControl ( pNameBox );
	m_pNameBox = pNameBox;
	
	m_pNameBoxDummy = CreateControl ( "CROW_NAMEBOX", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
}

CUIControl*	CCrowTargetInfoPlayer::CreateControl ( const char* szControl, WORD wAlignFlag )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl, wAlignFlag );
	RegisterControl ( pControl );
	return pControl;
}

void CCrowTargetInfoPlayer::SetTargetInfo ( int fNow, int fMax, int fPercent3,int fSchool, CString strClass, int strLevel, CString strPartyMember,CString strPartyM, CString strPartyNew,CString strPartyJoined,CString strParty, CString strGuild, CString strName, STARGETID sTargetID )
{	 
	PGLCHARCLIENT pCHAR = GLGaeaClient::GetInstance().GetChar ( sTargetID.dwID );
    WORD wSchool = GLGaeaClient::GetInstance().GetCharacter()->GetSchool();
	DWORD dwUSERLVL = GLGaeaClient::GetInstance().GetCharacter()->m_dwUserLvl; 

	if ( wSchool != pCHAR->GetSchool() ){
		CString strSchoolCombine;
		strSchoolCombine.Format ("%s", ID2GAMEWORD("ACADEMY_NAME",fSchool));
		m_pRebornBox->SetOneLineText ( strSchoolCombine, NS_UITEXTCOLOR::RED );
	}else{
		CString strSchoolCombine;
		strSchoolCombine.Format ("%s", ID2GAMEWORD("ACADEMY_NAME",fSchool));
		m_pRebornBox->SetOneLineText ( strSchoolCombine, NS_UITEXTCOLOR::WHITE );
	}
	{
		CString strClassCombine;
		strClassCombine.Format ("%s", strClass );
		m_pSchoolBox->SetOneLineText ( strClassCombine, NS_UITEXTCOLOR::WHITE );
	}
		if(strGuild != ""){
		CString strGuildCombine;
		strGuildCombine.Format ("%s", strGuild);
		m_pClassBox->SetOneLineText ( strGuildCombine, NS_UITEXTCOLOR::WHITE );
		}else{
		CString strGuildCombine;
		strGuildCombine.Format ("There's No Club Joined" );
		m_pClassBox->SetOneLineText ( strGuildCombine, NS_UITEXTCOLOR::YELLOW );
		}

		if(strParty != ""){ 
		CString strPartyCombine;
		strPartyCombine.Format ("Party: %s (%s)", strParty, strPartyNew );
		m_pGuildBox->SetOneLineText ( strPartyCombine, NS_UITEXTCOLOR::WHITE );
		}else{
		CString strPartyCombine;
		strPartyCombine.Format ("There's No Party Joined");
		m_pGuildBox->SetOneLineText ( strPartyCombine, NS_UITEXTCOLOR::YELLOW );
		}
		if(strPartyNew != ""){ 
		CString strPartyCombine;
		strPartyCombine.Format ("Party: %s (%s)", strParty, strPartyNew );
		m_pGuildBox->SetOneLineText ( strPartyCombine, NS_UITEXTCOLOR::WHITE );
		}else{
		CString strPartyCombine;
		strPartyCombine.Format ("There's No Party Joined");
		m_pGuildBox->SetOneLineText ( strPartyCombine, NS_UITEXTCOLOR::YELLOW );
		}
		m_pTextBox->SetOneLineText ( "Shift-Click to open the window of shortcut keys", NS_UITEXTCOLOR::GOLD );
		
		m_pNameBox->ClearText ();

		SIZE Size;
		m_pFont8->GetTextExtent ( strName.GetString (), Size );
        {
		CString strTemp;
		strTemp.Format ( "Lv.%d %s",strLevel, strName );
		m_pNameBox->SetOneLineText ( strTemp, NS_UITEXTCOLOR::WHITE );	
		m_strName = strName;
		}
}
