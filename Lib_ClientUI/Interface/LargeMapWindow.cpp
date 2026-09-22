#include "StdAfx.h"
#include "LargeMapWindow.h"
#include "LargeMapWindowMob.h"
#include "LargeMapWindowImage.h"
#include "LargeMapWindowRadio.h"
#include "LargeMapWindowSetting.h"

#include "GameTextControl.h"
#include "InnerInterface.h"
#include "BasicLineBox.h"
#include "BasicLineBoxEx.h"

#include "BasicComboBoxRollOver.h"

#include "../Lib_Engine/DxCommon/d3dfont.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"

#include "Util/RnButton.h"

#include "GLGaeaClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CLargeMapWindow::CLargeMapWindow ():
	 m_pBackGround(NULL)
	,m_pImage(NULL)
	,m_pSetting(NULL)
	,m_pInfo(NULL)
	/*dmk14 map preview*/
	,m_pButtonMyPos(NULL)
	,m_pButtonPrev(NULL)
	,m_pButtonNext(NULL)
	,m_pButtonMapList(NULL)
	,m_pMapListRollOver(NULL)
	,m_sActiveMapID( NATIVEID_NULL() )
	,m_sSelectedMapID( NATIVEID_NULL() )
	,m_bLoadMap(false)
{
	SecureZeroMemory(m_pRadio, sizeof(m_pRadio));
	SecureZeroMemory(m_bSUBSETTING, sizeof(m_bSUBSETTING));
}

CLargeMapWindow::~CLargeMapWindow ()
{
	for(int i = 0; i < MAXCROW; i++ ) 
	{
		m_sNativeID[i] = NATIVEID_NULL();
		m_bMOB[i] = TRUE;
	}
}

void CLargeMapWindow::CreateSubControl()
{

	m_pBackGround = new CUIControl;
	m_pBackGround->CreateSub ( this, "MAP_WINDOW_MAIN_REGION" );	
	m_pBackGround->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pBackGround );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "MAP_WINDOW_TAP_REGION" );
	RegisterControl ( pBasicLineBox );

	m_pImage = new CLargeMapWindowImage;
	m_pImage->CreateSub( this, "MAP_WINDOW_IMAGE" );
	m_pImage->CreateSubControl();
	RegisterControl( m_pImage );

	{
		CString strKeyword[MAX_RADIO_SETTINGS] =
		{
			"MAP_WINDOW_MAIN_SETTING_RADIO_BUTTON",
			"MAP_WINDOW_MAIN_INFO_RADIO_BUTTON"
		};

		for(int i = 0; i < MAX_RADIO_SETTINGS; i++ )
		{
			m_pRadio[i] = new CLargeMapWindowRadio;
			m_pRadio[i]->CreateSub(this, strKeyword[i].GetString(), UI_FLAG_DEFAULT, MAP_WINDOW_MAIN_SETTING_RADIO_BUTTON + i );
			m_pRadio[i]->CreateSubControl();
			m_pRadio[i]->SetRadioText( ID2GAMEWORD("LARGEMAP_WINDOW_RADIO_TEXT",i));
			RegisterControl( m_pRadio[i] );
		}
	}

	m_pSetting = new CLargeMapWindowSetting;
	m_pSetting->CreateSub(this, "MAP_WINDOW_TAP_SETTINGS", UI_FLAG_DEFAULT, MAP_WINDOW_TAP_SETTINGS );
	m_pSetting->CreateSubControl();
	RegisterControl( m_pSetting );
	
	m_pInfo = new CLargeMapWindowMob;
	m_pInfo->CreateSub(this, "MAP_WINDOW_TAP_INFO", UI_FLAG_DEFAULT, MAP_WINDOW_TAP_INFO );
	m_pInfo->CreateSubControl();
	RegisterControl( m_pInfo );

	/*dmk14 map preview*/
	const int nAlignCenterBoth = TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y;
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );
	//////////////////////////////////////////////////////////////////////////
	RnButton::CreateArg arg;
	arg.defaultTextureKey = "RNBUTTON_DEFAULT_TEXTURE";
	arg.mouseOverTextureKey = "RNBUTTON_OVER_TEXTURE";
	arg.mouseClickTextureKey = "RNBUTTON_CLICK_TEXTURE";
	arg.pFont = pFont;
	arg.dwColor = NS_UITEXTCOLOR::WHITE;
	//////////////////////////////////////////////////////////////////////////
    m_pButtonMyPos = new RnButton();
    m_pButtonMyPos->CreateSub( this, "MAP_WINDOW_CHANGEMAP_MYPOS_BTN", UI_FLAG_DEFAULT, MAP_WINDOW_MAP_MYPOS_BTN );
	arg.text = ("<<");
    m_pButtonMyPos->CreateSubControl(arg);
    RegisterControl(m_pButtonMyPos);

    m_pButtonPrev = new RnButton();
    m_pButtonPrev->CreateSub( this, "MAP_WINDOW_CHANGEMAP_PRE_BTN", UI_FLAG_DEFAULT, MAP_WINDOW_MAP_PRE_BTN );
	arg.text = ("<");
    m_pButtonPrev->CreateSubControl(arg);
    RegisterControl(m_pButtonPrev);

    m_pButtonNext = new RnButton();
    m_pButtonNext->CreateSub( this, "MAP_WINDOW_CHANGEMAP_NEXT_BTN", UI_FLAG_DEFAULT, MAP_WINDOW_MAP_NEXT_BTN );
	arg.text = (">");
    m_pButtonNext->CreateSubControl(arg);
    RegisterControl(m_pButtonNext);

    m_pButtonMapList = new RnButton();
    m_pButtonMapList->CreateSub( this, "MAP_WINDOW_CHANGEMAP_BTN", UI_FLAG_DEFAULT, MAP_WINDOW_MAP_BTN );
	arg.text = ("MAPNAME");
    m_pButtonMapList->CreateSubControl(arg);
    RegisterControl(m_pButtonMapList);

	m_pMapListRollOver = new CBasicComboBoxRollOver;
	m_pMapListRollOver->CreateSub ( this, "BASIC_COMBOBOX_ROLLOVER", UI_FLAG_XSIZE | UI_FLAG_YSIZE, MAP_WINDOW_MAP_ROLLOVER );
	m_pMapListRollOver->CreateBaseComboBoxRollOver ( "MAP_WINDOW_CHANGEMAP_ROLLOVER" );			
	m_pMapListRollOver->SetVisibleSingle ( FALSE );
	RegisterControl ( m_pMapListRollOver );

	m_pMapListRollOver->ClearText();


	GLMapList::FIELDMAP map = GLGaeaClient::GetInstance().GetMapList();
	GLMapList::FIELDMAP_ITER iter_start = map.begin();
	GLMapList::FIELDMAP_ITER iter_end = map.end();
	int nIndex = 0;
	for ( ; iter_start != iter_end; ++ iter_start )
	{
		SMAPNODE sNODE = (*iter_start).second;	
		if( !sNODE.bUIMapSelect )	continue;

		m_pMapListRollOver->AddText( sNODE.strMapName.c_str() );
		m_pMapListRollOver->SetTextData( nIndex, sNODE.sNativeID.dwID );
		nIndex++;
	}


	SetPage(0);
	
	for(int i = 0; i < MAX_SUB_SETTING; i++) m_bSUBSETTING[i] = TRUE;
	m_bSUBMOTION = TRUE;
	for(int j = 0; j < MAXCROW; j++ ) m_bMOB[j] = TRUE;
}

void CLargeMapWindow::SetPage(int nIndex)
{
	for(int i = 0; i < MAX_RADIO_SETTINGS; i++ ) m_pRadio[i]->SetRadioOn(FALSE);
	m_pRadio[nIndex]->SetRadioOn(TRUE);
	m_pSetting->SetVisibleSingle(FALSE);
	m_pInfo->SetVisibleSingle(FALSE);
	switch(nIndex)
	{
	case 0:	m_pSetting->SetVisibleSingle(TRUE);		break;
	case 1:	m_pInfo->SetVisibleSingle(TRUE);		break;
	}
}

void CLargeMapWindow::TranslateUIMessage ( UIGUID cID, DWORD dwMsg )
{
	CUIWindowEx::TranslateUIMessage ( cID, dwMsg );

	switch ( cID )
	{
	case ET_CONTROL_BUTTON:
			{
				if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
				{
					CInnerInterface::GetInstance().HideGroup ( LARGEMAP_WINDOW );
					CInnerInterface::GetInstance().HideGroup ( MOBPREVIEW_WINDOW );
				}
				break;
			}
	case ET_CONTROL_TITLE:
	case ET_CONTROL_TITLE_F:
		{
			if ( (dwMsg & UIMSG_LB_DUP) && CHECK_MOUSE_IN ( dwMsg ) ) 
			{
				CInnerInterface::GetInstance().SetDefaultPosInterface( LARGEMAP_WINDOW );
			}
		}
		break;
	case MAP_WINDOW_MAIN_SETTING_RADIO_BUTTON:
	case MAP_WINDOW_MAIN_INFO_RADIO_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg))
			{
				int nIndex = cID - MAP_WINDOW_MAIN_SETTING_RADIO_BUTTON;
				SetPage(nIndex);
			}
		}
		break;
	case MAP_WINDOW_MAP_MYPOS_BTN:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg) && !m_bLoadMap )
			{
				LoadMap( m_sActiveMapID );
			}
		}
		break;
	case MAP_WINDOW_MAP_PRE_BTN:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg) && !m_bLoadMap )
			{
				GLMapList::FIELDMAP MapsList = GLGaeaClient::GetInstance().GetMapList();
				GLMapList::FIELDMAP_ITER iter = MapsList.find( m_sSelectedMapID.dwID );
				if( iter == MapsList.end() ) break;
		
				while ( iter != MapsList.begin() )
				{		
					--iter;
					const SMAPNODE& node = iter->second;					
					if( node.bUIMapSelect )	
					{
						LoadMap( node.sNativeID );
						break;
					}
				}				
			}
		}
		break;
	case MAP_WINDOW_MAP_NEXT_BTN:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE(dwMsg) && !m_bLoadMap )
			{
				GLMapList::FIELDMAP MapsList = GLGaeaClient::GetInstance().GetMapList();
				GLMapList::FIELDMAP_ITER iter = MapsList.find( m_sSelectedMapID.dwID );
				if( iter == MapsList.end() ) break;
				++iter;
				if( iter == MapsList.end() ) break;
		
				while ( iter != MapsList.end() )
				{					
					const SMAPNODE& node = iter->second;					
					if( node.bUIMapSelect )	
					{
						LoadMap( node.sNativeID );
						break;
					}
					++iter;
				}				
			}
		}
		break;
	case MAP_WINDOW_MAP_BTN:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( dwMsg & UIMSG_LB_DOWN )
				{
					if( !m_pMapListRollOver->IsVisible() )	m_pMapListRollOver->SetVisibleSingle ( TRUE );
					else									m_pMapListRollOver->SetVisibleSingle ( FALSE );
					m_pMapListRollOver->SetScrollPercent ( 0.0f );				
				}
			}
		}
		break;
	case MAP_WINDOW_MAP_ROLLOVER:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) && !m_bLoadMap )
			{	
				int nIndex = m_pMapListRollOver->GetSelectIndex ();
				if ( nIndex < 0 ) 
					return ;

				DWORD dwData = m_pMapListRollOver->GetTextData( nIndex );
				SNATIVEID sID = SNATIVEID( dwData );
				LoadMap( sID );

				m_pMapListRollOver->SetVisibleSingle( FALSE );	
			}
		}
		break;
	}
}

void CLargeMapWindow::SetMapAxisInfo ( GLMapAxisInfo& sInfo,CString strMapName )
{
	if ( m_pImage ) m_pImage->SetMapAxisInfo( sInfo, strMapName );
}

void CLargeMapWindow::VISIBLE_TARGET_POS( BOOL bVisible )
{
	if ( m_pImage ) m_pImage->VISIBLE_TARGET_POS( bVisible );
}

void CLargeMapWindow::SetLargeMap( SNATIVEID sMapID, GLCHARLOGIC *pCHAR )
{
	m_pCHAR = pCHAR;
	m_sActiveMapID = sMapID; 
	LoadMap( m_sActiveMapID );
}

void CLargeMapWindow::LoadMap( SNATIVEID sMapID )
{
	m_bLoadMap = true;
	ClearMobList( sMapID );
	m_sSelectedMapID = sMapID;

	DWORD dwID;
	map<DWORD, int> vecCrowDataList;
	SMAPNODE *pMapNode = GLGaeaClient::GetInstance().FindMapNode ( sMapID );
	if ( pMapNode )
	{
		m_pButtonMapList->SetText( pMapNode->strMapName.c_str(), NS_UITEXTCOLOR::WHITE );
		GLLevelFile cLevelFile;
		if( cLevelFile.LoadFile ( pMapNode->strFile.c_str(), TRUE, NULL ) )
		{
			CString strText = GLGaeaClient::GetInstance().GetMapName ( sMapID );
			SetMapAxisInfo ( cLevelFile.GetMapAxisInfo(), strText );

			MOBSCHEDULELIST * GLMobSchList = cLevelFile.GetMobSchManClient()->GetMobSchList();
			SGLNODE<GLMobSchedule*>* pCur = GLMobSchList->m_pHead;
			while ( pCur )
			{
				PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( pCur->Data->m_CrowID );
				if ( !pCROW ) 
				{
					pCur = pCur->pNext;
					continue;
				}

				if ( pCROW )
				{
					LoadMob( pCROW, pCur->Data->m_pAffineParts->vTrans, m_pCHAR );
				}

				if ( pCROW->m_emCrow == CROW_MOB )
				{
					dwID = pCROW->sNativeID.dwID;
					if( vecCrowDataList.count( dwID ) == 0 ) vecCrowDataList.insert( std::make_pair( dwID, 1) );
					else vecCrowDataList[dwID] += 1;
				}
				pCur = pCur->pNext;
			}
		}

		map<DWORD, int>::iterator iter = vecCrowDataList.begin();
		map<DWORD, int>::iterator iter_end = vecCrowDataList.end();

		for ( ; iter != iter_end; ++iter )
		{
			SNATIVEID sID = iter->first;
			PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData( sID );
			if( pCROW )	LoadMob( pCROW );
		}
	}	

	if( m_pImage ) m_pImage->SetGateList( sMapID );
	if( m_pInfo ) 
	{
		m_pInfo->SetMonsterInfo();
	}

	m_bLoadMap = false;
}

void CLargeMapWindow::ClearMobList( SNATIVEID sMapID )
{
	if( m_pMapListRollOver->IsVisible() )	m_pMapListRollOver->SetVisibleSingle ( FALSE );

	for(int i = 0; i < MAXCROW; i++ ) 
	{
		m_sNativeID[i] = NATIVEID_NULL();
		m_bMOB[i] = TRUE;
	}

	m_pImage->Reset();
	m_pInfo->Reset();
	m_pImage->ClearMobList();
	m_pInfo->ClearMobList();
	m_nIndex = 0;
	
}

void CLargeMapWindow::LoadMob( PCROWDATA pCrow, D3DXVECTOR3 vPos, GLCHARLOGIC *pCHAR )
{
	if ( !pCrow ) return;
	if( m_pImage )	m_pImage->LoadMob(pCrow, vPos, pCHAR);
}

void CLargeMapWindow::LoadMob( PCROWDATA pCrow )
{
	if ( !pCrow ) return;
	if ( m_pInfo ) m_pInfo->LoadMob(pCrow);
	m_sNativeID[m_nIndex] = pCrow->sNativeID;
	m_nIndex++;
}

void CLargeMapWindow::Update( INT x, INT y, BYTE LB, BYTE MB, BYTE RB, INT nScroll, FLOAT fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}