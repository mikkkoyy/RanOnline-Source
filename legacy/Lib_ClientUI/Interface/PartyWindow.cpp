#include "StdAfx.h"
#include "PartyWindow.h"
#include "./BasicTextBox.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "GameTextControl.h"
#include "UITextControl.h"
#include "PartySlot.h"
#include "MiniMap.h"
#include "InnerInterface.h"
#include "BasicComboBox.h"
#include "BasicComboBoxRollOver.h"
#include "BasicLineBox.h"
#include "PartyWindowCheckBox.h"
#include "./Util/RnButton.h"
#include "./BasicButtonText.h"
#include "GLGaeaClient.h"
#include "DxGlobalStage.h"
#include "GLPartyClient.h"
#include "BasicTextButton.h"
#include "GLCharacter.h"
#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPartyWindow::CPartyWindow ()
	: m_pParty(NULL)
	, m_pMember(NULL)
	, m_pBuff(NULL)
	, m_pNoPartyText(NULL)
	, m_bParty(false)
	, m_pButtonParty ( NULL )
	, m_pButtonWithdraw ( NULL )
	, m_pButtonRelease ( NULL )
	, m_pButtonDelegate ( NULL )
	, m_pButtonBan ( NULL )
	, m_RollOverID ( NO_ID )
	, m_pComboBoxMoney ( NULL )
	, m_pComboBoxMoneyRollOver( NULL )
	, m_pComboBoxItem ( NULL )
	, m_pComboBoxItemRollOver( NULL )
{
	SecureZeroMemory(m_pPartySlot,sizeof(m_pPartySlot));
}

CPartyWindow::~CPartyWindow ()
{
}

CBasicTextButton*  CPartyWindow::CreateTextButton23 ( const char* szButton, UIGUID ControlID, const char* szText )
{
	const int nBUTTONSIZE = CBasicTextButton::SIZE23;
	CBasicTextButton* pTextButton = new CBasicTextButton;
	pTextButton->CreateSub ( this, "BASIC_TEXT_BUTTON23", UI_FLAG_XSIZE|UI_FLAG_YSIZE, ControlID );
	pTextButton->CreateBaseButton ( szButton, nBUTTONSIZE, CBasicButton::RADIO_FLIP, szText, 0, NS_UITEXTCOLOR::BLACK );
	RegisterControl ( pTextButton );
	return pTextButton;
}

CBasicTextBox* CPartyWindow::CreateStaticControl ( char* szControlKeyword, CD3DFontPar* pFont, int nAlign, const UIGUID& cID )
{
	CBasicTextBox* pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, szControlKeyword, UI_FLAG_DEFAULT, cID );
	pStaticText->SetFont ( pFont );
	pStaticText->SetTextAlign ( nAlign );	
	RegisterControl ( pStaticText );

	return pStaticText;
}

CPartyWindowCheckBox* CPartyWindow::CreateCheckBox ( CString strKeyword, UIGUID ControlID )
{
	CPartyWindowCheckBox* pCheck = new CPartyWindowCheckBox;
	pCheck->CreateSub ( this, strKeyword.GetString (), UI_FLAG_DEFAULT, ControlID );
	pCheck->CreateSubControl();
	pCheck->SetVisibleSingle(TRUE);
	RegisterControl ( pCheck );

	return pCheck;
}

CBasicButtonText* CPartyWindow::CreateButtonBlue ( char* szButton, char* szTextBox, CD3DFontPar* pFont, int nAlign, UIGUID ControlID, CString strText )
{
	CBasicButtonText* pButton = new CBasicButtonText;
	pButton->CreateRanButtonBlue ( this, szButton, ControlID );
	pButton->CreateTextBox ( szTextBox, pFont, nAlign );
	pButton->SetOneLineText( strText.GetString() );
	RegisterControl ( pButton );

	return pButton;
}

void CPartyWindow::CreateSubControl ()
{	
	m_pButtonParty = CreateTextButton23 ( "PARTY_RENEWAL_INFO_TAP", PARTY_TAP_BUTTON, (char*)ID2GAMEWORD("PARTY_NAME_STATIC") );

	m_pBackGround = new CUIControl;
	m_pBackGround->CreateSub ( this, "PARTY_WINDOW_RENEWAL_REGION", UI_FLAG_DEFAULT );	
	m_pBackGround->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pBackGround );


	{	
		CBasicLineBox* pBasicLineBox = new CBasicLineBox;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxQuestList( "PARTY_INFO_OPTION_REGION" );
		RegisterControl ( pBasicLineBox );
	}
	
	{
		CBasicLineBox* pBasicLineBox = new CBasicLineBox;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxQuestList( "PARTY_INFO_MEMBER_REGION" );
		RegisterControl ( pBasicLineBox );
	}

	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

	CBasicTextBox* pTextBox = NULL;
	pTextBox = CreateStaticControl ( "PARTY_NONPARTY_TEXT", pFont, TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pNoPartyText = pTextBox;
	pTextBox->AddText ( ID2GAMEWORD("MAP_HAVE_NO_PARTY", 0) );
	pTextBox = CreateStaticControl ( "PARTY_INFO_ITEM_STATIC", pFont, TEXT_ALIGN_LEFT );
	pTextBox->AddText ( ID2GAMEWORD("PARTY_RENEWAL_INFO_TEXT", 0) );
	pTextBox = CreateStaticControl ( "PARTY_INFO_MONEY_STATIC", pFont, TEXT_ALIGN_LEFT );
	pTextBox->AddText ( ID2GAMEWORD("PARTY_RENEWAL_INFO_TEXT", 2) );

	m_pMember = CreateCheckBox( "PARTY_INFO_CHECK_BOX", PARTY_INFO_CHECK_BOX );
	m_pBuff = CreateCheckBox( "PARTY_INFO_BUFF_CHECK_BOX", PARTY_INFO_BUFF_CHECK_BOX );

	m_pMember->SetTextTitle( ID2GAMEWORD("PARTY_RENEWAL_INFO_TEXT",12), NS_UITEXTCOLOR::WHITE );
	m_pBuff->SetTextTitle( ID2GAMEWORD("PARTY_RENEWAL_INFO_TEXT",4), NS_UITEXTCOLOR::WHITE );

	//m_pMember->SetCheckManual(RANPARAM::bPartyDisplay);
	//m_pBuff->SetCheckManual(RANPARAM::bPartyBuffDisplay);

	CString strMemberKeyword[MAXPARTY] = 
	{
		"PARTY_INFO_SLOT0",
		"PARTY_INFO_SLOT1",
		"PARTY_INFO_SLOT2",
		"PARTY_INFO_SLOT3",
		"PARTY_INFO_SLOT4",
		"PARTY_INFO_SLOT5",
		"PARTY_INFO_SLOT6",
		"PARTY_INFO_SLOT7"
	};

	for ( int i = 0; i < MAXPARTY; i++ )
	{
		CPartySlot* pPartySlot = new CPartySlot;
		pPartySlot->CreateSub ( this, strMemberKeyword[i].GetString (), UI_FLAG_DEFAULT, PARTY_INFO_SLOT0 + i );
		pPartySlot->CreateSubControl ();
		RegisterControl ( pPartySlot );
		m_pPartySlot[i] = pPartySlot;
	}

	RnButton::CreateArg arg;
	arg.defaultTextureKey = "RNBUTTON_DEFAULT_TEXTURE";
	arg.mouseOverTextureKey = "RNBUTTON_OVER_TEXTURE";
	arg.mouseClickTextureKey = "RNBUTTON_CLICK_TEXTURE";
	arg.pFont = pFont;
	arg.dwColor = NS_UITEXTCOLOR::WHITE;
	//////////////////////////////////////////////////////////////////////////
    m_pButtonWithdraw = new RnButton();
    m_pButtonWithdraw->CreateSub( this, "PARTY_INFO_WITHDRAW_BUTTON", UI_FLAG_DEFAULT, PARTY_INFO_LEAVE_BTN );
	arg.text = (ID2GAMEWORD("PARTY_RENEWAL_INFO_TEXT",8));
    m_pButtonWithdraw->CreateSubControl(arg);
    RegisterControl(m_pButtonWithdraw);

    m_pButtonDelegate = new RnButton();
    m_pButtonDelegate->CreateSub( this, "PARTY_INFO_DELEGATE_BUTTON", UI_FLAG_DEFAULT, PARTY_INFO_DELEGATE_BTN );
	arg.text = (ID2GAMEWORD("PARTY_RENEWAL_INFO_TEXT",6));
    m_pButtonDelegate->CreateSubControl(arg);
    RegisterControl(m_pButtonDelegate);

    m_pButtonRelease = new RnButton();
    m_pButtonRelease->CreateSub( this, "PARTY_INFO_RELEASE_PARTY_BUTTON", UI_FLAG_DEFAULT, PARTY_INFO_DISBAND_BTN );
	arg.text = (ID2GAMEWORD("PARTY_RENEWAL_INFO_TEXT",3));
    m_pButtonRelease->CreateSubControl(arg);
    RegisterControl(m_pButtonRelease);

    m_pButtonBan = new RnButton();
    m_pButtonBan->CreateSub( this, "PARTY_INFO_BAN_BUTTON", UI_FLAG_DEFAULT, PARTY_INFO_KICK_BTN );
	arg.text = (ID2GAMEWORD("PARTY_RENEWAL_INFO_TEXT",5));
    m_pButtonBan->CreateSubControl(arg);
    RegisterControl(m_pButtonBan);

	m_bParty = false;

	CBasicComboBox* pComboBox = NULL;
	
	pComboBox = new CBasicComboBox;
	pComboBox->CreateSub ( this, "BASIC_COMBOBOX", UI_FLAG_XSIZE, PARTY_INFO_ITEM_COMBO_OPEN );
	pComboBox->CreateBaseComboBox ( "PARTY_INFO_ITEM_COMBO_OPEN" );			
	RegisterControl ( pComboBox );
	m_pComboBoxItem = pComboBox;
	
	pComboBox = new CBasicComboBox;
	pComboBox->CreateSub ( this, "BASIC_COMBOBOX", UI_FLAG_XSIZE, PARTY_INFO_MONEY_COMBO_OPEN );
	pComboBox->CreateBaseComboBox ( "PARTY_INFO_MONEY_COMBO_OPEN" );			
	RegisterControl ( pComboBox );
	m_pComboBoxMoney = pComboBox;

	CBasicComboBoxRollOver*	pComboBoxRollOver = NULL;

	pComboBoxRollOver = new CBasicComboBoxRollOver;
	pComboBoxRollOver->CreateSub ( this, "BASIC_COMBOBOX_ROLLOVER", UI_FLAG_XSIZE | UI_FLAG_YSIZE, PARTY_INFO_ITEM_FILTER_COMBO_ROLLOVER );
	pComboBoxRollOver->CreateBaseComboBoxRollOver ( "PARTY_INFO_ITEM_FILTER_COMBO_ROLLOVER" );			
	pComboBoxRollOver->SetVisibleSingle ( FALSE );
	RegisterControl ( pComboBoxRollOver );
	m_pComboBoxItemRollOver = pComboBoxRollOver;

	pComboBoxRollOver = new CBasicComboBoxRollOver;
	pComboBoxRollOver->CreateSub ( this, "BASIC_COMBOBOX_ROLLOVER", UI_FLAG_XSIZE | UI_FLAG_YSIZE, PARTY_INFO_MONEY_COMBO_ROLLOVER );
	pComboBoxRollOver->CreateBaseComboBoxRollOver ( "PARTY_INFO_MONEY_COMBO_ROLLOVER" );
	pComboBoxRollOver->SetVisibleSingle ( FALSE );
	RegisterControl ( pComboBoxRollOver );
	m_pComboBoxMoneyRollOver = pComboBoxRollOver;
}

void CPartyWindow::SetSlotMember ( CPartySlot* pPartySlot, GLPARTY_CLIENT *pMember, BOOL bSameMap, int nSlot)
{
	if ( !pPartySlot )	return;
	if ( !pMember )		return;

	pPartySlot->SetSameMap(bSameMap);
	pPartySlot->SetDie(pMember->m_sHP.dwNow == 0);

	CString strSlot, strLevel, strPos;
	strSlot.Format ( "%d", nSlot);
	pPartySlot->SetSlotNum(strSlot);
	pPartySlot->SetMaster(nSlot==0);

	const int nClass = CharClassToIndex ( pMember->m_emClass );	
	DWORD dwGaeaID = pMember->m_dwGaeaID;

	pPartySlot->SetClass(nClass);
	pPartySlot->SetSchool(pMember->m_wSchool);

	const CString strName = pMember->m_szName;
	const CString strMapName = GLGaeaClient::GetInstance().GetMapName ( pMember->m_sMapID );
	
	int nPosX(0), nPosY(0);
	PLANDMANCLIENT pLandClient = GLGaeaClient::GetInstance().GetActiveMap();
	GLMapAxisInfo &sMapAxisInfo = pLandClient->GetMapAxisInfo();
	sMapAxisInfo.Convert2MapPos ( pMember->m_vPos.x, pMember->m_vPos.z, nPosX, nPosY );

	strPos.Format ( "%d/%d", nPosX, nPosY );

	BOOL bSELF = pMember->ISONESELF();
	if ( !bSELF )
	{
		PGLCHARCLIENT pCHAR = NULL;
		pCHAR = GLGaeaClient::GetInstance().GetChar( pMember->m_dwGaeaID );
		if ( pCHAR )
		{
			strLevel.Format ( "Lv. %d", pCHAR->GetLevel());
		}
	}
	else
	{
		strLevel.Format ( "Lv. %d", GLGaeaClient::GetInstance().GetCharacter()->m_wLevel);
	}
	pPartySlot->SetInfo(strLevel, strMapName, dwGaeaID, strName, strPos, bSELF);
	pPartySlot->SetVisibleSingle(TRUE);
}

void CPartyWindow::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{

	if ( !IsVisible() ) 
	{
		ResetData();
		return;
	}
	//CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
	
	if ( m_RollOverID == NO_ID )
	{
		CUIWindow::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
	}else{
		ResetMessageEx ();

		CUIControl::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

		CUIControl* pControl = m_ControlContainer.FindControl ( m_RollOverID );
		if ( !pControl )
		{
			GASSERT ( 0 && "심각한 오류, 노드가 널입니다." );
			return ;
		}

		pControl->Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

		DWORD dwControlMsg = pControl->GetMessageEx ();
		NS_UIDEBUGSET::BlockBegin ();
		if ( dwControlMsg ) TranslateUIMessage ( m_RollOverID, dwControlMsg );
		NS_UIDEBUGSET::BlockEnd ();

		//	스크롤바에 관계된 메시지가 아니고		
		if ( !(dwControlMsg & UI_MSG_COMBOBOX_ROLLOVER_SCROLL) )
		{
			DWORD dwMsg = GetMessageEx ();
			if ( dwMsg & UIMSG_LB_UP )
			{
				if ( !m_bFirstLBUP )
				{
					m_RollOverID = NO_ID;
					pControl->SetVisibleSingle ( FALSE );					
				}
				m_bFirstLBUP = FALSE;
			}
		}
	}

	m_pButtonParty->SetVisibleSingle ( TRUE );
	m_pButtonParty->SetFlip ( TRUE );

	m_pNoPartyText->SetVisibleSingle(TRUE);
	m_bParty = false;

	m_pButtonBan->SetVisibleSingle(FALSE);
	m_pButtonDelegate->SetVisibleSingle(FALSE);
	m_pButtonRelease->SetVisibleSingle(FALSE);
	m_pButtonWithdraw->SetVisibleSingle(FALSE);
	
	for(int i = 0; i < MAXPARTY; i++ ) m_pPartySlot[i]->SetVisibleSingle(FALSE);

	GLPARTY_CLIENT *pMaster = GLPartyClient::GetInstance().GetMaster();
	if ( pMaster )
	{
		GLPARTY_CLIENT *pSelf = FindSelfClient ();
		if ( !pSelf ) return ;

		const BOOL bMaster = pMaster->ISONESELF ();
		if ( bMaster )
		{
			m_pButtonBan->SetVisibleSingle(TRUE);
			m_pButtonDelegate->SetVisibleSingle(TRUE);
			m_pButtonRelease->SetVisibleSingle(TRUE);
			//m_pLeave->SetVisibleSingle(TRUE);
		}
		else m_pButtonWithdraw->SetVisibleSingle(TRUE);
		SetSlotMember ( m_pPartySlot[0], pMaster, IsSameMap ( pSelf, pMaster ), 0 );

		DWORD nMEMBER_NUM = GLPartyClient::GetInstance().GetMemberNum();
		if( nMEMBER_NUM > 1 )
		{
			nMEMBER_NUM -= 1;
			for ( DWORD i = 0; i < nMEMBER_NUM; i++ )
			{
				GLPARTY_CLIENT *pMember = GLPartyClient::GetInstance().GetMember ( i );			
				if ( pMember )
				{				
					const BOOL bONESELF = pMember->ISONESELF ();
					SetSlotMember ( m_pPartySlot[i+1], pMember, IsSameMap ( pSelf, pMember ), i+1 );
				}
			}
		}

		m_pNoPartyText->SetVisibleSingle(FALSE);
		m_bParty = true;

		const SPARTY_OPT& sPartyOption = GLPartyClient::GetInstance().GetOption ();
		if ( m_pComboBoxItem )
		{
			m_pComboBoxItem->SetText( ID2GAMEWORD("PARTY_MODAL_ITEM_OPTION",sPartyOption.emGET_ITEM) );
		}

		if ( m_pComboBoxMoney )
		{
			m_pComboBoxMoney->SetText( ID2GAMEWORD("PARTY_MODAL_MONEY_OPTION",sPartyOption.emGET_MONEY) );
		}
	}
}

BOOL CPartyWindow::IsSameMap ( GLPARTY_CLIENT *pSelf, GLPARTY_CLIENT *pMember )
{
	if ( !pSelf || !pMember ) return FALSE;
	
	return pSelf->m_sMapID.dwID == pMember->m_sMapID.dwID;
}

GLPARTY_CLIENT*	CPartyWindow::FindSelfClient ()
{
	GLPARTY_CLIENT *pMaster = GLPartyClient::GetInstance().GetMaster();	
	if ( pMaster && pMaster->ISONESELF() ) return pMaster;
	else
	{
		for ( int i = 0; i < 7; i++ )
		{
			GLPARTY_CLIENT *pMember = GLPartyClient::GetInstance().GetMember( i );
			if ( pMember && pMember->ISONESELF () ) return pMember;				
		}
	}

	return NULL;
}

void CPartyWindow::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIWindowEx::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case ET_CONTROL_TITLE:
	case ET_CONTROL_TITLE_F:
		{
			if ( (dwMsg & UIMSG_LB_DUP) && CHECK_MOUSE_IN(dwMsg)) 
			{
				CInnerInterface::GetInstance().SetDefaultPosInterface( PARTY_WINDOW );
			}
		}
		break;
	case PARTY_INFO_CHECK_BOX:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg)) 
			{
				m_pMember->SetCheck();

				if ( m_bParty )
				{
					RANPARAM::bPartyDisplay = m_pMember->IsChecked();
				}
			}
		}
		break;
	case PARTY_INFO_BUFF_CHECK_BOX:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg))
			{
				m_pBuff->SetCheck();

				if ( m_bParty )
				{
					RANPARAM::bPartyBuffDisplay = m_pBuff->IsChecked();
				}
			}
		}
		break;
	case PARTY_INFO_SLOT0:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg))
			{
				m_nMemberIndex = -1;

				for(int i = 0; i < MAXPARTY; i++ ) 
				{
					m_pPartySlot[i]->SetSelect(FALSE);
				}

				m_pPartySlot[0]->SetSelect(TRUE);
			}
		}
		break;
	case PARTY_INFO_SLOT1:
	case PARTY_INFO_SLOT2:
	case PARTY_INFO_SLOT3:
	case PARTY_INFO_SLOT4:
	case PARTY_INFO_SLOT5:
	case PARTY_INFO_SLOT6:
	case PARTY_INFO_SLOT7:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg))
			{
				int nIndex = ControlID - PARTY_INFO_SLOT1;
				m_nMemberIndex = nIndex;

				for(int i = 0; i < MAXPARTY; i++ ) 
				{
					m_pPartySlot[i]->SetSelect(FALSE);
				}

				m_pPartySlot[nIndex+1]->SetSelect(TRUE);
			}
		}
		break;
	case PARTY_INFO_KICK_BTN:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg))
			{
				if ( m_nMemberIndex < 0 ) 
				{
					CInnerInterface::GetInstance().PrintConsoleText("Select a member to kick.");
					return;
				}

				GLPartyClient::GetInstance().Secede(m_nMemberIndex);
				ResetData();
			}
		}
		break;
	case PARTY_INFO_DELEGATE_BTN:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg))
			{
				if ( m_nMemberIndex < 0 ) 
				{
					CInnerInterface::GetInstance().PrintConsoleText("Select a member to delegate.");
					return;
				}

				GLPartyClient::GetInstance().Authority(m_nMemberIndex);
				ResetData();
			}
		}
		break;
	case PARTY_INFO_DISBAND_BTN:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg))
			{
				GLPartyClient::GetInstance().Dissolve();
				ResetData();
			}
		}
		break;
	case PARTY_INFO_LEAVE_BTN:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg)) 
			{
				GLPARTY_CLIENT *pMaster = GLPartyClient::GetInstance().GetMaster();


				for(int i = 0; i < MAXPARTY - 1; i++)
					{
						GLPARTY_CLIENT *pMember = GLPartyClient::GetInstance().GetMember( i );
						if ( pMember->ISONESELF() )
						{
							GLPartyClient::GetInstance().Secede( i );	
							ResetData();
							break;
						}
					}
			}
		}
		break;
	case PARTY_INFO_ITEM_COMBO_OPEN:
		{
		}break;
	case PARTY_INFO_MONEY_COMBO_OPEN:
		{
		}break;
	case PARTY_INFO_ITEM_FILTER_COMBO_ROLLOVER:
		{
		}break;
	case PARTY_INFO_MONEY_COMBO_ROLLOVER:
		{
		}break;
	}
}

void CPartyWindow::ResetData()
{
	for(int i = 0; i < MAXPARTY; i++ ) m_pPartySlot[i]->SetSelect(FALSE);
	m_nMemberIndex = -1;
}

void CPartyWindow::SetPartyInfo ( DWORD dwPartyID, DWORD dwMasterID )
{
	m_dwPartyID = dwPartyID;
	m_dwMasterID = dwMasterID;
}

DWORD CPartyWindow::GetPartyID ()
{
	return m_dwPartyID;
}

DWORD CPartyWindow::GetMasterID()
{
	return m_dwMasterID;
}

void CPartyWindow::SetVisibleSingle ( BOOL bVisible )
{
	CUIWindow::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		LoadDefaultCondition();
	}
}

void CPartyWindow::LoadDefaultCondition ()
{
	m_pComboBoxItemRollOver->ClearText ();
	for ( int i = 1; i < EMPTYITEMOPT_SIZE; i++ )
	{
		m_pComboBoxItemRollOver->AddText( (char*)ID2GAMEWORD("PARTY_MODAL_ITEM_OPTION", i) );
	}

	m_pComboBoxMoneyRollOver->ClearText ();
	for ( int i = 1; i < EMPTYMONEYOPT_SIZE; i++ )
	{
		m_pComboBoxMoneyRollOver->AddText( (char*)ID2GAMEWORD("PARTY_MODAL_MONEY_OPTION", i) );
	}

	const SPARTY_OPT& sPartyOption = GLPartyClient::GetInstance().GetOption ();
	
	m_pComboBoxMoney->SetText ( (char*)ID2GAMEWORD("PARTY_MODAL_MONEY_OPTION", sPartyOption.emGET_MONEY) );
	m_pComboBoxItem->SetText ( (char*)ID2GAMEWORD("PARTY_MODAL_ITEM_OPTION", sPartyOption.emGET_ITEM) );
	
}