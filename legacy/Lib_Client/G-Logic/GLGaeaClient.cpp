#include "stdafx.h"
#include "shlobj.h"
#include "SUBPATH.h"
#include "GLOGIC.h"

#include "../../Lib_ClientUI/Interface/GameTextControl.h"
#include "../../Lib_ClientUI/Interface/InnerInterface.h"
#include "../../Lib_ClientUI/Interface/UITextControl.h"

//#include <strstream>
#include "DxShadowMap.h"
#include "DxGlobalStage.h"
#include "DxViewPort.h"
#include "COLLISION.h"
#include "stl_Func.h"
#include "DxSimpleMeshMan.h"
#include "GLItemMan.h"
#include "DxEffectMan.h"
#include "GLPeriod.h"
#include "DxResponseMan.h"
#include "../../Lib_ClientUI/Interface/LoadingThread.h"
#include "DxSkinMeshMan.h"
#include "GLCrowRenList.h"
#include "GLPartyClient.h"
#include "DxClubMan.h"
#include "DxSoundLib.h"
#include "stringutils.h"
#include "DxEffGroupPlayer.h"
#include "GLFriendClient.h"
#include "GLGaeaClient.h"
#include "../../Lib_ClientUI/Interface/MiniMap.h"
#include "../Lib_ClientUI/Interface/ModalCallerID.h"
#include "../Lib_ClientUI/Interface/MapRequireCheck.h"
#include "../Lib_ClientUI/Interface/ModalWindow.h"
#include "../Lib_ClientUI/Interface/DamageDisplay.h"
#include "GLFactEffect.h"
#include "PKRankData.h"
#include "DxEffectMan.h"

/*pvp tyranny, Juver, 2017/08/24 */
#include "GLPVPTyrannyClient.h" 

/*school wars, Juver, 2018/01/19 */
#include "GLPVPSchoolWarsClient.h"

/*pvp capture the flag, Juver, 2018/02/01 */
#include "GLPVPCaptureTheFlagClient.h"

/////////////////////////////////////////////////////////////////////////////
// LG-7 CWRanking
#include "../Lib_ClientUI/Interface/CWRankingDisplay/CWRankingDisplay.h"
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern BOOL g_bPREVIEW_CHAR_RENDER;

GLGaeaClient& GLGaeaClient::GetInstance()
{
	static GLGaeaClient Instance;
	return Instance;
}

GLGaeaClient::GLGaeaClient(void) 
	: m_hWnd(NULL)
	
	, m_fAge(0.0f)
	, m_pd3dDevice(NULL)
	, m_pLandMClient(NULL)
	, m_vCharPos(0,0,0)
	, m_bSCHOOL_FREEPK(false)
	, m_bBRIGHTEVENT(false)
	, m_bCLUBBATTLE(false)
	, m_fCLUBBATTLETimer(0.0f)
	, m_bClubDeathMatch(false)
	, m_fClubDeathMatchTimer(0.0f)
{
	memset(m_szAppPath, 0, sizeof(char) * (MAX_PATH));
	memset(m_szShopInfoFile, 0, sizeof(char) * (MAX_PATH));
	m_vecDetectID.reserve(200);
}

GLGaeaClient::~GLGaeaClient(void)
{
	//	Note : ���� ���� Ŭ����.
	//    
	SAFE_DELETE(m_pLandMClient);
}

HRESULT GLGaeaClient::OneTimeSceneInit ( const char* szAppPath, HWND hWnd )
{
	m_hWnd = hWnd;
	StringCchCopy ( m_szAppPath, MAX_PATH, szAppPath );

	return S_OK;
}

HRESULT	GLGaeaClient::CreatePET ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_REQ_USEPETCARD_FB* pNetMsg = ( GLMSG::SNETPET_REQ_USEPETCARD_FB* ) nmg;

	GLPET sPet;
	
	sPet.m_emTYPE			= pNetMsg->m_emTYPE;
	sPet.m_dwGUID			= pNetMsg->m_dwGUID;
	sPet.m_sPetID			= pNetMsg->m_sPetID;

	/*dual pet skill, Juver, 2017/12/27 */
	sPet.m_sActiveSkillID_A	= pNetMsg->m_sActiveSkillID_A;
	sPet.m_sActiveSkillID_B	= pNetMsg->m_sActiveSkillID_B;
	sPet.m_bDualSkill		= pNetMsg->m_bDualSkill;

	sPet.m_dwOwner			= pNetMsg->m_dwOwner;
	sPet.m_wStyle			= pNetMsg->m_wStyle;
	sPet.m_wColor			= pNetMsg->m_wColor;
	sPet.m_fWalkSpeed		= pNetMsg->m_fWalkSpeed;
	sPet.m_fRunSpeed		= pNetMsg->m_fRunSpeed;
	sPet.m_nFull			= pNetMsg->m_nFull;
	sPet.m_sMapID			= pNetMsg->m_sMapID;
	sPet.m_dwCellID			= pNetMsg->m_dwCellID;
	StringCchCopy ( sPet.m_szName, PETNAMESIZE+1, pNetMsg->m_szName );

	sPet.m_dwPetID			= pNetMsg->m_dwPetID;

	WORD i(0);
	for ( ; i < pNetMsg->m_wSkillNum; ++i )
	{
		sPet.m_ExpSkills.insert ( std::make_pair ( pNetMsg->m_Skills[i].sNativeID.dwID, 
			pNetMsg->m_Skills[i] ) );
	}

	for ( WORD i = 0; i < PET_ACCETYPE_SIZE; ++i )
	{
		sPet.m_PutOnItems[i] = pNetMsg->m_PutOnItems[i];
	}

	// ���� ���� PET ����
	HRESULT hr = m_Pet.Create ( &sPet, pNetMsg->m_vPos, pNetMsg->m_vDir, m_pLandMClient->GetNaviMesh(), m_pd3dDevice );

	// ����ó�� ( ������ ������ ���� �����ϵ��� )
	if ( FAILED(hr) )
	{
		SINVENITEM* pInvenItem = m_Character.m_cInventory.FindItemByGenNumber ( 
			m_Character.m_llPetCardGenNum, m_Character.m_sPetCardNativeID, m_Character.m_cPetCardGenType );
		if ( pInvenItem )
		{
			GLMSG::SNETPET_REQ_UNUSEPETCARD NetMsg;
			NetMsg.dwGUID = sPet.m_dwGUID;
			NetMsg.dwPetID = sPet.m_dwPetID;
			NETSENDTOFIELD ( &NetMsg );
		}
		// ���н� GenNum ó��
		m_Character.m_llPetCardGenNum = 0;

		return FALSE;
	}

	// ���� ȿ��
	D3DXMATRIX matEffect;
	D3DXVECTOR3 vPos = m_Pet.GetPosition ();
	D3DXMatrixTranslation ( &matEffect, vPos.x, vPos.y, vPos.z );

	std::string strGEN_EFFECT = GLCONST_CHAR::strPET_GEN_EFFECT.c_str();
	STARGETID sTargetID(CROW_PET,m_Pet.m_dwGUID,vPos);
	DxEffGroupPlayer::GetInstance().NewEffGroup
	(
		strGEN_EFFECT.c_str(),
		matEffect,
		&sTargetID
	);

	// ���õ� ��ų�� �ִٸ� �������� �ش�
	/*dual pet skill, Juver, 2017/12/27 */
	if ( m_Pet.m_sActiveSkillID_A != NATIVEID_NULL() )
		m_Pet.ReqChangeActiveSkill_A ( m_Pet.m_sActiveSkillID_A );

	/*dual pet skill, Juver, 2017/12/27 */
	if ( m_Pet.m_sActiveSkillID_B != NATIVEID_NULL() )
		m_Pet.ReqChangeActiveSkill_B ( m_Pet.m_sActiveSkillID_B );

	return S_OK;
}


PLANDMANCLIENT GLGaeaClient::CreateLandMClient ( char* szLandFileName, D3DXVECTOR3 &vBasicPos )
{
	GASSERT(szLandFileName);
	GASSERT(m_pd3dDevice&&"m_pd3dDevice �� �ʱ�ȭ ���� �ʾҽ��ϴ�.");

	PLANDMANCLIENT pLandMClient = NULL;

	pLandMClient = new GLLandManClient;
	pLandMClient->Create ( szLandFileName, m_pd3dDevice, vBasicPos );

	return pLandMClient;
}

PLANDMANCLIENT GLGaeaClient::CreateLandMClient ( SNATIVEID sMapID, D3DXVECTOR3 &vBasicPos )
{
	GASSERT(m_pd3dDevice&&"m_pd3dDevice �� �ʱ�ȭ ���� �ʾҽ��ϴ�.");

	PLANDMANCLIENT pLandMClient = NULL;
	SMAPNODE *pMapNode = FindMapNode ( sMapID );
	if ( pMapNode )
	{
		pLandMClient = new GLLandManClient;
		pLandMClient->Create ( pMapNode->strFile.c_str(), m_pd3dDevice, vBasicPos, &sMapID, pMapNode->bPeaceZone );
	}

	return pLandMClient;
}

PLANDMANCLIENT GLGaeaClient::CreateInstantMapClient ( SNATIVEID sBaseMapID, SNATIVEID sInstantMapID, D3DXVECTOR3 &vBasicPos )
{
	PLANDMANCLIENT pLandMClient = NULL;
	SMAPNODE *pMapNode = FindMapNode ( sBaseMapID );
	if ( pMapNode )
	{
		pLandMClient = new GLLandManClient;
		pLandMClient->Create ( pMapNode->strFile.c_str(), m_pd3dDevice, vBasicPos, &sInstantMapID, pMapNode->bPeaceZone );
	}

	return pLandMClient;
}

HRESULT GLGaeaClient::SetActiveMap ( char* szLandFileName, D3DXVECTOR3 &vBasicPos )
{
	//	Note : ���ο� ���� �ε���.
	//
	PLANDMANCLIENT pLandMClient = CreateLandMClient ( szLandFileName, vBasicPos );
	if ( !pLandMClient )	return E_FAIL;

	//	Note : ���� ���� Ŭ����.
	//
	SAFE_DELETE(m_pLandMClient);

	//	Note : �ε��� ���� Ȱ��ȭ.
	//
	m_pLandMClient = pLandMClient;
	m_pLandMClient->ActiveMap ();

	return S_OK;
}

HRESULT GLGaeaClient::SetActiveMap ( SNATIVEID sMapID, D3DXVECTOR3 &vBasicPos )
{
	//	Note : ���ο� ���� �ε���.
	//
	PLANDMANCLIENT pLandMClient = CreateLandMClient ( sMapID, vBasicPos );
	if ( !pLandMClient )	return E_FAIL;

	//	Note : ���� ���� Ŭ����.
	//
	SAFE_DELETE(m_pLandMClient);

	//	Note : �ε��� ���� Ȱ��ȭ.
	//
	m_pLandMClient = pLandMClient;
	m_pLandMClient->ActiveMap ();

	return S_OK;
}

PLANDMANCLIENT GLGaeaClient::GetActiveMap ()
{
	return m_pLandMClient;
}

SNATIVEID GLGaeaClient::GetActiveMapID ()
{
	if ( !m_pLandMClient ) return NATIVEID_NULL();
	
	return m_pLandMClient->GetMapID();
}

HRESULT GLGaeaClient::MoveActiveMap ( SNATIVEID sMapID, D3DXVECTOR3 &vPos )
{
	//	Note : ���� ���� Ŭ����.
	//
	SAFE_DELETE(m_pLandMClient);

	CInnerInterface::GetInstance().ClearNameList();
	// MEMO : ���̵��� ���� �����츦 �ݱ� ���ؼ� �߰��ߴ�. ���� â�� �������� ������ ���Ѻ��߰ڴ�.
	// �Ʒ� �Լ��� ��Ȯ�� �ǹ̷� ���̴��� �𸣰ڴ�.
	CInnerInterface::GetInstance().CloseAllWindow();

	//	Note : ��ġ �����͸� ����.
	//
	DxResponseMan::GetInstance().DoInterimClean ( m_pd3dDevice );

	//	Note : �ɸ��� �⺻ ������ �ٽ� �θ���.
	for ( int i=0; i<GLCI_NUM_8CLASS; ++i )
		DxSkinCharDataContainer::GetInstance().LoadData( GLCONST_CHAR::szCharSkin[i], m_pd3dDevice, TRUE );

	//	Note : �ڱ� �ɸ��� ��Ų�� �ٽ� �о� �´�.
	//
	m_Character.SkinLoad ( m_pd3dDevice );

	//	Note : ���ο� ���� �ε���.
	//
	PLANDMANCLIENT pLandMClient = CreateLandMClient ( sMapID, vPos );
	if ( !pLandMClient )	return E_FAIL;

	//	Note : �ε��� ���� Ȱ��ȭ.
	//
	m_pLandMClient = pLandMClient;
	m_pLandMClient->ActiveMap ();

	CString strText = GLGaeaClient::GetInstance().GetMapName ( m_pLandMClient->GetMapID() );
	CInnerInterface::GetInstance().SetMiniMapInfo ( m_pLandMClient->GetMapAxisInfo(), strText );

	// �̴ϸ� ���콺 �̵� ��ǥ���� ����
	CInnerInterface::GetInstance().DisableMinimapTarget();

	return S_OK;
}

HRESULT GLGaeaClient::CreateInstantMap ( SNATIVEID sBaseMapID, SNATIVEID sInstantMapID, D3DXVECTOR3 &vBasicPos )
{
	//	Note : ���� ���� Ŭ����.
	//
	SAFE_DELETE(m_pLandMClient);

	CInnerInterface::GetInstance().ClearNameList();
	// MEMO : ���̵��� ���� �����츦 �ݱ� ���ؼ� �߰��ߴ�. ���� â�� �������� ������ ���Ѻ��߰ڴ�.
	// �Ʒ� �Լ��� ��Ȯ�� �ǹ̷� ���̴��� �𸣰ڴ�.
	CInnerInterface::GetInstance().CloseAllWindow();

	//	Note : ��ġ �����͸� ����.
	//
	DxResponseMan::GetInstance().DoInterimClean ( m_pd3dDevice );

	//	Note : �ɸ��� �⺻ ������ �ٽ� �θ���.
	for ( int i=0; i<GLCI_NUM_8CLASS; ++i )
		DxSkinCharDataContainer::GetInstance().LoadData( GLCONST_CHAR::szCharSkin[i], m_pd3dDevice, TRUE );

	//	Note : �ڱ� �ɸ��� ��Ų�� �ٽ� �о� �´�.
	//
	m_Character.SkinLoad ( m_pd3dDevice );

	//	Note : ���ο� ���� �ε���.
	//
	PLANDMANCLIENT pLandMClient = CreateInstantMapClient ( sBaseMapID, sInstantMapID, vBasicPos );
	if ( !pLandMClient )	return E_FAIL;

	//	Note : �ε��� ���� Ȱ��ȭ.
	//
	m_pLandMClient = pLandMClient;
	m_pLandMClient->ActiveMap ();
	m_pLandMClient->SetInstantMap(TRUE);

	CString strText = GLGaeaClient::GetInstance().GetMapName ( m_pLandMClient->GetMapID() );
	CInnerInterface::GetInstance().SetMiniMapInfo ( m_pLandMClient->GetMapAxisInfo(), strText );

	// �̴ϸ� ���콺 �̵� ��ǥ���� ����
	CInnerInterface::GetInstance().DisableMinimapTarget();

	return S_OK;
}

HRESULT GLGaeaClient::MoveActiveMap ( NET_MSG_GENERIC *nmg )
{
	if ( m_Pet.IsVALID() )
	{
		m_Pet.DeleteDeviceObjects ();
		m_Character.m_bIsPetActive = TRUE;
	}

	/*if ( m_Summon.IsVALID() )
	{
		m_Summon.DeleteDeviceObjects();
	}*/
	
	/*skill summon, Juver, 2017/10/08 */
	for( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
	{
		if ( m_Summon[i].IsVALID() )
			m_Summon[i].DeleteDeviceObjects();
	}

	HRESULT hr=S_OK;

	GLMSG::SNETREQ_GATEOUT_FB *pNetMsg = (GLMSG::SNETREQ_GATEOUT_FB *) nmg;

	m_Character.m_vecMarketClick.clear();

	m_Character.ReSetSTATE(EM_REQ_GATEOUT);

	if ( pNetMsg->emFB!=EMCHAR_GATEOUT_OK )
	{
		CDebugSet::ToListView ( "[MSG] SNETREQ_GATEOUT_FB FAIL : %d", pNetMsg->emFB );

		switch(pNetMsg->emFB)
		{
	case EMCHAR_GATEOUT_FAIL:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("REQ_GATEOUT_FB_FAIL"), pNetMsg->emFB );
			break;
		case EMCHAR_GATEOUT_MAPID:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("REQ_GATEOUT_FB_DATA_ERR"), pNetMsg->emFB );
			break;
		case EMCHAR_GATEOUT_GATEID:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("REQ_GATEOUT_FB_DATA_ERR"), pNetMsg->emFB );
			break;
		case EMCHAR_GATEOUT_TARMAPID:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("REQ_GATEOUT_FB_DATA_ERR"), pNetMsg->emFB );
			break;
		case EMCHAR_GATEOUT_TARGATEID:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("REQ_GATEOUT_FB_DATA_ERR"), pNetMsg->emFB );
			break;
		case EMCHAR_GATEOUT_FIELDSVR:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("REQ_GATEOUT_FB_SVR_ERR"), pNetMsg->emFB );
			break;
		case EMCHAR_GATEOUT_HOLD:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMCHAR_GATEOUT_HOLD") );
			break;
		case EMCHAR_GATEOUT_CONDITION:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMCHAR_GATEOUT_CONDITION"), pNetMsg->emFB );
			break;
		case EMCHAR_GATEOUT_OTHERCLUB_NOTALLOWED_INPARTY:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMCHAR_GATEOUT_OTHERCLUB_NOTALLOWED_INPARTY") );
			break;
		case EMCHAR_GATEOUT_CLUBBATTLE:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMCHAR_GATEOUT_CLUBBATTLE"), GLCONST_CHAR::dwCLUB_BATTLE_GUID_TIME );
			break;
		case EMCHAR_GATEOUT_CLUBBATTLE2:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMCHAR_GATEOUT_CLUBBATTLE2") );
			break;

			/*clubwar map lock, Juver, 2017/06/27 */
		case EMCHAR_GATEOUT_CLUBBATTLE_LOCK:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMCHAR_GATEOUT_CLUBBATTLE_LOCK") );
			break;
		};

		return S_FALSE;
	}

	CDebugSet::ToListView ( "[MSG] SNETREQ_GATEOUT_FB OK" );


	DWORD dwColorArg1(NULL),dwOldMod(NULL);
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLORARG1,	&dwColorArg1 );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE );

	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLOROP,		&dwOldMod );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1 );

	GLMapList::FIELDMAP MapsList = GetMapList();
	GLMapList::FIELDMAP_ITER iter = MapsList.find( pNetMsg->sMapID.dwID );
	if( iter == MapsList.end() )				return E_FAIL;

	if ( m_Character.m_bVehicle )
	{
		m_Character.ReqSetVehicle( false );
		m_Character.SetVehicle ( false );
		m_Character.m_bIsVehicleActive = TRUE;
	}

	m_Character.m_vecMarketClick.clear();

	m_Character.m_sVehicle.SetActiveValue( false );
	m_Character.m_sVehicle.RESET();

	const SMAPNODE *pMapNode = &(*iter).second;

	CInnerInterface::GetInstance().SetPrivateMarketClose();
	NLOADINGTHREAD::StartThreadLOAD( &m_pd3dDevice, m_hWnd, m_szAppPath, pMapNode->strLoadingImageName.c_str(), pMapNode->strMapName.c_str(), TRUE );
	NLOADINGTHREAD::WaitThread();

	LOADINGSTEP::SETSTEP ( 1 );

	m_pLandMClient->ClearDropObj ();

	LOADINGSTEP::SETSTEP ( 3 );

	// �ٸ� ������ �̵��ϸ�
	if ( pNetMsg->sMapID != m_pLandMClient->GetMapID() )
	{
		
		// ���̵��� �ڽſ��� �ɸ� ������ �����̻� ����
		m_Character.InitAllSkillFact ();
		m_Character.DISABLEALLLANDEFF();

		hr = MoveActiveMap ( pNetMsg->sMapID, pNetMsg->vPos );
		if ( SUCCEEDED(hr) )
		{
			LOADINGSTEP::SETSTEP ( 4 );
			m_Character.MoveActiveMap ( m_pLandMClient->GetLandMan()->GetNaviMesh(), pNetMsg->vPos );
		}
	}
	else
	{
		LOADINGSTEP::SETSTEP ( 4 );
		m_Character.SetPosition ( pNetMsg->vPos );
		m_Character.DoActWait ();		
	}

	DxResponseMan::GetInstance().SetRenderState ();

	LOADINGSTEP::SETSTEP ( 7 );

	// Note : �� �̵��� ���� �޼����� �����Ѵ�.
	CInnerInterface::GetInstance().WARNING_MSG_OFF();

	// �̴ϸ� ���콺 �̵� ��ǥ���� ����
	CInnerInterface::GetInstance().DisableMinimapTarget();

    LOADINGSTEP::SETSTEP ( 11 );
	NLOADINGTHREAD::EndThread();

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,		dwOldMod );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	dwColorArg1 );

	return S_OK;
}


HRESULT GLGaeaClient::CreateInstantMap ( NET_MSG_GENERIC *nmg )
{
	if ( m_Pet.IsVALID() )
	{
		m_Pet.DeleteDeviceObjects ();
		m_Character.m_bIsPetActive = TRUE;
	}

	/*if ( m_Summon.IsVALID() )
	{
		m_Summon.DeleteDeviceObjects();
	}*/

	/*skill summon, Juver, 2017/10/08 */
	for( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
	{
		if ( m_Summon[i].IsVALID() )
			m_Summon[i].DeleteDeviceObjects();
	}

	HRESULT hr=S_OK;

	GLMSG::SNETREQ_CREATE_INSTANT_MAP_FB *pNetMsg = (GLMSG::SNETREQ_CREATE_INSTANT_MAP_FB *) nmg;

	m_Character.ReSetSTATE(EM_REQ_GATEOUT);

	if ( pNetMsg->emFB!=EMCHAR_CREATE_INSTANT_MAP_OK )
	{
		CDebugSet::ToListView ( "[MSG] SNETREQ_GATEOUT_FB FAIL : %d", pNetMsg->emFB );

		switch(pNetMsg->emFB)
		{
			case EMCHAR_CREATE_INSTANT_MAP_FAIL:		  
			case EMCHAR_CREATE_INSTANT_MAP_FAIL_CREATE:	  
			case EMCHAR_CREATE_INSTANT_MAP_FAIL_MAPID:	  
			case EMCHAR_CREATE_INSTANT_MAP_FAIL_GATEID:	  
			case EMCHAR_CREATE_INSTANT_MAP_FAIL_TARMAPID: 
			case EMCHAR_CREATE_INSTANT_MAP_FAIL_CONDITION:
			case EMCHAR_CREATE_INSTANT_MAP_FAIL_FIELDSVR: 
			case EMCHAR_CREATE_INSTANT_MAP_FAIL_NOT:
			case EMCHAR_CREATE_INSTANT_MAP_FAIL_CREATEMAX:
				{
					CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMCHAR_CREATE_INSTANT_MAP_FAIL") );
				}break;
		};

		return S_FALSE;
	}

	CDebugSet::ToListView ( "[MSG] SNETREQ_GATEOUT_FB OK" );


	DWORD dwColorArg1(NULL),dwOldMod(NULL);
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLORARG1,	&dwColorArg1 );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE );

	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLOROP,		&dwOldMod );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1 );

	GLMapList::FIELDMAP MapsList = GetMapList();
	GLMapList::FIELDMAP_ITER iter = MapsList.find( pNetMsg->sBaseMapID.dwID );
	if( iter == MapsList.end() )				return E_FAIL;

	if ( m_Character.m_bVehicle )
	{
		m_Character.ReqSetVehicle( false );
		m_Character.SetVehicle ( false );
		m_Character.m_bIsVehicleActive = TRUE;
	}

	m_Character.m_sVehicle.SetActiveValue( false );
	m_Character.m_sVehicle.RESET();

	const SMAPNODE *pMapNode = &(*iter).second;

	CInnerInterface::GetInstance().SetPrivateMarketClose();
	NLOADINGTHREAD::StartThreadLOAD( &m_pd3dDevice, m_hWnd, m_szAppPath, pMapNode->strLoadingImageName.c_str(), pMapNode->strMapName.c_str(), TRUE );
	NLOADINGTHREAD::WaitThread();

	LOADINGSTEP::SETSTEP ( 1 );

	m_pLandMClient->ClearDropObj ();

	LOADINGSTEP::SETSTEP ( 3 );

	// ���̵��� �ڽſ��� �ɸ� ������ �����̻� ����
	m_Character.InitAllSkillFact ();
	m_Character.DISABLEALLLANDEFF();

	SMAPNODE smap_node;
	smap_node = *pMapNode;
	smap_node.sLEVEL_REQUIRE  = pMapNode->sLEVEL_REQUIRE;
	smap_node.sLEVEL_ETC_FUNC = pMapNode->sLEVEL_ETC_FUNC;
	smap_node.sNativeID = pNetMsg->sInstantMapID;
	BOOL bInsert = InsertMapList( smap_node );
	
	hr = CreateInstantMap ( pNetMsg->sBaseMapID, pNetMsg->sInstantMapID, pNetMsg->vPos );
	if ( SUCCEEDED(hr) )
	{
		LOADINGSTEP::SETSTEP ( 4 );
		m_Character.MoveActiveMap ( m_pLandMClient->GetLandMan()->GetNaviMesh(), pNetMsg->vPos );
	}

	DxResponseMan::GetInstance().SetRenderState ();

	LOADINGSTEP::SETSTEP ( 7 );

	// Note : �� �̵��� ���� �޼����� �����Ѵ�.
	CInnerInterface::GetInstance().WARNING_MSG_OFF();

	// �̴ϸ� ���콺 �̵� ��ǥ���� ����
	CInnerInterface::GetInstance().DisableMinimapTarget();

	LOADINGSTEP::SETSTEP ( 11 );
	NLOADINGTHREAD::EndThread();

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,		dwOldMod );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	dwColorArg1 );

	return S_OK;
}

HRESULT GLGaeaClient::ReBirthFB ( NET_MSG_GENERIC* nmg )
{
	if ( m_Pet.IsVALID() )
	{
		m_Pet.DeleteDeviceObjects ();
		m_Character.m_bIsPetActive = TRUE;
	}

	/*if ( m_Summon.IsVALID() )
	{
		m_Summon.DeleteDeviceObjects();
	}*/

	/*skill summon, Juver, 2017/10/08 */
	for( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
	{
		if ( m_Summon[i].IsVALID() )
			m_Summon[i].DeleteDeviceObjects();
	}

	if ( m_Character.m_bVehicle )
	{
		m_Character.ReqSetVehicle( false );
		m_Character.SetVehicle ( false );
		m_Character.m_bIsVehicleActive = TRUE;
	}

	HRESULT hr=S_OK;

	DWORD dwColorArg1(NULL),dwOldMod(NULL);
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLORARG1,	&dwColorArg1 );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE );

	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLOROP,		&dwOldMod );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1 );

	GLMSG::SNETPC_REQ_REBIRTH_FB *pNetMsg = (GLMSG::SNETPC_REQ_REBIRTH_FB *) nmg;

	GLMapList::FIELDMAP MapsList = GetMapList();
	GLMapList::FIELDMAP_ITER iter = MapsList.find( pNetMsg->sMapID.dwID );
	if( iter == MapsList.end() )				return E_FAIL;

	const SMAPNODE *pMapNode = &(*iter).second;

	CInnerInterface::GetInstance().SetPrivateMarketClose();
	NLOADINGTHREAD::StartThreadLOAD( &m_pd3dDevice, m_hWnd, m_szAppPath, pMapNode->strLoadingImageName.c_str(), pMapNode->strMapName.c_str(), TRUE );
	NLOADINGTHREAD::WaitThread();

	LOADINGSTEP::SETSTEP ( 1 );

	m_pLandMClient->ClearDropObj ();

	LOADINGSTEP::SETSTEP ( 3 );

	//	��Ȱ�� Map�� Ʋ�� ���.
	if ( pNetMsg->sMapID != m_pLandMClient->GetMapID() )
	{
		hr = MoveActiveMap ( pNetMsg->sMapID, pNetMsg->vPos );
		if ( SUCCEEDED(hr) )
		{
			LOADINGSTEP::SETSTEP ( 5 );
			m_Character.MoveActiveMap ( m_pLandMClient->GetLandMan()->GetNaviMesh(), pNetMsg->vPos );
		}
	}
	else
	{
		LOADINGSTEP::SETSTEP ( 5 );
		m_Character.SetPosition ( pNetMsg->vPos );
		m_Character.DoActWait ();
	}

	LOADINGSTEP::SETSTEP ( 7 );

	// �̴ϸ� ���콺 �̵� ��ǥ���� ����
	CInnerInterface::GetInstance().DisableMinimapTarget();

	LOADINGSTEP::SETSTEP ( 9 );

	//	��Ȱó��.
	m_Character.ReBirth ( pNetMsg->dwNowHP, pNetMsg->dwNowMP, pNetMsg->dwNowSP, pNetMsg->vPos, false );

	if( pNetMsg->bRegenEntryFailed )
	{
		DoModal( ID2GAMEINTEXT("REGEN_ENTRY_FAIL"), MODAL_INFOMATION, OK );
	}

	LOADINGSTEP::SETSTEP ( 11 );
	NLOADINGTHREAD::EndThread();

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,		dwOldMod );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	dwColorArg1 );

	return S_OK;
}

HRESULT GLGaeaClient::ReLvMustLeaveFB( NET_MSG_GENERIC* nmg )
{
	HRESULT hr=S_OK;

	GLMSG::SNETPC_REQ_MUST_LEAVE_MAP_FB *pNetMsg = (GLMSG::SNETPC_REQ_MUST_LEAVE_MAP_FB *) nmg;
	if ( pNetMsg->emFB != EMREQ_MUST_LEAVE_MAP_FB_OK )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_MUST_LEAVE_MAP_FB_FAIL") );
		return S_OK;
	}

	// �̵��� ���� ���� ��쿡�� ���� �����Ѵ�.
	if ( m_Pet.IsVALID() )
	{
		m_Pet.DeleteDeviceObjects ();
		m_Character.m_bIsPetActive = TRUE;
	}

	/*if ( m_Summon.IsVALID() )
	{
		m_Summon.DeleteDeviceObjects();
	}*/

	/*skill summon, Juver, 2017/10/08 */
	for( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
	{
		if ( m_Summon[i].IsVALID() )
			m_Summon[i].DeleteDeviceObjects();
	}

	if ( m_Character.m_bVehicle )
	{
		m_Character.ReqSetVehicle( false );
		m_Character.SetVehicle ( false );
		m_Character.m_bIsVehicleActive = TRUE;
	}

	m_Character.ResetAction();

	DWORD dwColorArg1(NULL),dwOldMod(NULL);
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLORARG1,	&dwColorArg1 );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE );

	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLOROP,		&dwOldMod );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1 );

	GLMapList::FIELDMAP MapsList = GetMapList();
	GLMapList::FIELDMAP_ITER iter = MapsList.find( pNetMsg->sMAPID.dwID );
	if( iter == MapsList.end() )				return E_FAIL;

	const SMAPNODE *pMapNode = &(*iter).second;

	CInnerInterface::GetInstance().SetPrivateMarketClose();
	NLOADINGTHREAD::StartThreadLOAD( &m_pd3dDevice, m_hWnd, m_szAppPath, pMapNode->strLoadingImageName.c_str(), pMapNode->strMapName.c_str(), TRUE );
	NLOADINGTHREAD::WaitThread();

	LOADINGSTEP::SETSTEP ( 1 );

	m_pLandMClient->ClearDropObj ();

	LOADINGSTEP::SETSTEP ( 4 );

	//	�̵��� Map�� Ʋ�� ���.
	if ( pNetMsg->sMAPID != m_pLandMClient->GetMapID() )
	{
		// ���̵��� �ڽſ��� �ɸ� ������ �����̻� ����
		m_Character.InitAllSkillFact ();
		m_Character.DISABLEALLLANDEFF();

		hr = MoveActiveMap ( pNetMsg->sMAPID, pNetMsg->vPOS );
		if ( SUCCEEDED(hr) )
		{
			LOADINGSTEP::SETSTEP ( 7 );
			m_Character.MoveActiveMap ( m_pLandMClient->GetLandMan()->GetNaviMesh(), pNetMsg->vPOS );
		}
	}
	else
	{
		LOADINGSTEP::SETSTEP ( 7 );
		m_Character.SetPosition ( pNetMsg->vPOS );
		m_Character.DoActWait ();
	}

	LOADINGSTEP::SETSTEP ( 9 );

	// �̴ϸ� ���콺 �̵� ��ǥ���� ����
	CInnerInterface::GetInstance().DisableMinimapTarget();

	LOADINGSTEP::SETSTEP ( 11 );
	NLOADINGTHREAD::EndThread();

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,		dwOldMod );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	dwColorArg1 );

	return S_OK;
}

HRESULT GLGaeaClient::ReCallFB ( NET_MSG_GENERIC* nmg )
{
	HRESULT hr=S_OK;

	GLMSG::SNETPC_REQ_RECALL_FB *pNetMsg = (GLMSG::SNETPC_REQ_RECALL_FB *) nmg;
	if ( pNetMsg->emFB!=EMREQ_RECALL_FB_OK )
	{
		switch ( pNetMsg->emFB )
		{
		case EMREQ_RECALL_FB_FAIL:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_RECALL_FB_FAIL") );
			return S_OK;

		case EMREQ_RECALL_FB_ITEM:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_RECALL_FB_ITEM") );
			return S_OK;

		case EMREQ_RECALL_FB_CONDITION:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_RECALL_FB_CONDITION") );
			return S_OK;

		case EMREQ_RECALL_FB_NOTLASTCALL:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_RECALL_FB_NOTLASTCALL") );
			return S_OK;

		case EMREQ_RECALL_FB_JOINCON:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_RECALL_FB_JOINCON") );
			return S_OK;
			
		case EMREQ_RECALL_FB_IMMOVABLE:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_RECALL_FB_IMMOVABLE") );
			return S_OK;
		};
	}

	// �̵��� ���� ���� ��쿡�� ���� �����Ѵ�.
	if ( m_Pet.IsVALID() )
	{
		m_Pet.DeleteDeviceObjects ();
		m_Character.m_bIsPetActive = TRUE;
	}

	/*if ( m_Summon.IsVALID() )
	{
		m_Summon.DeleteDeviceObjects();
	}*/

	/*skill summon, Juver, 2017/10/08 */
	for( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
	{
		if ( m_Summon[i].IsVALID() )
			m_Summon[i].DeleteDeviceObjects();
	}

	if ( m_Character.m_bVehicle )
	{
		m_Character.ReqSetVehicle( false );
		m_Character.SetVehicle ( false );
		m_Character.m_bIsVehicleActive = TRUE;
	}

	m_Character.ResetAction();

	DWORD dwColorArg1(NULL),dwOldMod(NULL);
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLORARG1,	&dwColorArg1 );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE );

	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLOROP,		&dwOldMod );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1 );

	GLMapList::FIELDMAP MapsList = GetMapList();
	GLMapList::FIELDMAP_ITER iter = MapsList.find( pNetMsg->sMAPID.dwID );
	if( iter == MapsList.end() )				return E_FAIL;

	const SMAPNODE *pMapNode = &(*iter).second;

	CInnerInterface::GetInstance().SetPrivateMarketClose();
	NLOADINGTHREAD::StartThreadLOAD( &m_pd3dDevice, m_hWnd, m_szAppPath, pMapNode->strLoadingImageName.c_str(), pMapNode->strMapName.c_str(), TRUE );
	NLOADINGTHREAD::WaitThread();

	LOADINGSTEP::SETSTEP ( 1 );

	m_pLandMClient->ClearDropObj ();

	LOADINGSTEP::SETSTEP ( 4 );

	//	�̵��� Map�� Ʋ�� ���.
	if ( pNetMsg->sMAPID != m_pLandMClient->GetMapID() )
	{
		// ���̵��� �ڽſ��� �ɸ� ������ �����̻� ����
		m_Character.InitAllSkillFact ();
		m_Character.DISABLEALLLANDEFF();

		hr = MoveActiveMap ( pNetMsg->sMAPID, pNetMsg->vPOS );
		if ( SUCCEEDED(hr) )
		{
			LOADINGSTEP::SETSTEP ( 7 );
			m_Character.MoveActiveMap ( m_pLandMClient->GetLandMan()->GetNaviMesh(), pNetMsg->vPOS );
		}
	}
	else
	{
		LOADINGSTEP::SETSTEP ( 7 );
		m_Character.SetPosition ( pNetMsg->vPOS );
		m_Character.DoActWait ();
	}

	LOADINGSTEP::SETSTEP ( 9 );

	// �̴ϸ� ���콺 �̵� ��ǥ���� ����
	CInnerInterface::GetInstance().DisableMinimapTarget();

	LOADINGSTEP::SETSTEP ( 11 );
	NLOADINGTHREAD::EndThread();

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,		dwOldMod );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	dwColorArg1 );

	return S_OK;
}

HRESULT GLGaeaClient::TeleportFB ( NET_MSG_GENERIC* nmg )
{
	HRESULT hr=S_OK;

	GLMSG::SNETPC_REQ_TELEPORT_FB *pNetMsg = (GLMSG::SNETPC_REQ_TELEPORT_FB *) nmg;
	if ( pNetMsg->emFB!=EMREQ_TELEPORT_FB_OK )
	{
		switch ( pNetMsg->emFB )
		{
		case EMREQ_TELEPORT_FB_FAIL:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_TELEPORT_FB_FAIL") );
			return S_OK;

		case EMREQ_TELEPORT_FB_ITEM:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_TELEPORT_FB_ITEM") );
			return S_OK;

		case EMREQ_TELEPORT_FB_CONDITION:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_TELEPORT_FB_CONDITION") );
			return S_OK;

		case EMREQ_TELEPORT_FB_JOINCON:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_TELEPORT_FB_JOINCON") );
			return S_OK;
			
		case EMREQ_TELEPORT_FB_IMMOVABLE:
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_TELEPORT_FB_IMMOVABLE") );
			return S_OK;
		};
	}

	// �̵��� ���� ���� ��쿡�� ���� �����Ѵ�.
	if ( m_Pet.IsVALID() )
	{
		m_Pet.DeleteDeviceObjects ();
		m_Character.m_bIsPetActive = TRUE;
	}

	/*if ( m_Summon.IsVALID() )
	{
		m_Summon.DeleteDeviceObjects();
	}*/

	/*skill summon, Juver, 2017/10/08 */
	for( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
	{
		if ( m_Summon[i].IsVALID() )
			m_Summon[i].DeleteDeviceObjects();
	}

	if ( m_Character.m_bVehicle )
	{
		m_Character.ReqSetVehicle( false );
		m_Character.SetVehicle ( false );
		m_Character.m_bIsVehicleActive = TRUE;
	}

	DWORD dwColorArg1(NULL),dwOldMod(NULL);
	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLORARG1,	&dwColorArg1 );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE );

	m_pd3dDevice->GetTextureStageState( 0, D3DTSS_COLOROP,		&dwOldMod );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1 );

	GLMapList::FIELDMAP MapsList = GetMapList();
	GLMapList::FIELDMAP_ITER iter = MapsList.find( pNetMsg->sMAPID.dwID );
	if( iter == MapsList.end() )				return E_FAIL;

	const SMAPNODE *pMapNode = &(*iter).second;

	CInnerInterface::GetInstance().SetPrivateMarketClose();
	NLOADINGTHREAD::StartThreadLOAD( &m_pd3dDevice, m_hWnd, m_szAppPath, pMapNode->strLoadingImageName.c_str(), pMapNode->strMapName.c_str(), TRUE );
	NLOADINGTHREAD::WaitThread();

	LOADINGSTEP::SETSTEP ( 1 );

	m_pLandMClient->ClearDropObj ();

	LOADINGSTEP::SETSTEP ( 4 );

	//	�̵��� Map�� Ʋ�� ���.
	if ( pNetMsg->sMAPID != m_pLandMClient->GetMapID() )
	{
		// ���̵��� �ڽſ��� �ɸ� ������ �����̻� ����
		m_Character.InitAllSkillFact ();
		m_Character.DISABLEALLLANDEFF();

		hr = MoveActiveMap ( pNetMsg->sMAPID, pNetMsg->vPOS );
		if ( SUCCEEDED(hr) )
		{
			LOADINGSTEP::SETSTEP ( 7 );
			m_Character.MoveActiveMap ( m_pLandMClient->GetLandMan()->GetNaviMesh(), pNetMsg->vPOS );
		}
	}
	else
	{
		LOADINGSTEP::SETSTEP ( 7 );
		m_Character.SetPosition ( pNetMsg->vPOS );
		m_Character.DoActWait ();
	}

	LOADINGSTEP::SETSTEP ( 9 );

	// �̴ϸ� ���콺 �̵� ��ǥ���� ����
	CInnerInterface::GetInstance().DisableMinimapTarget();

	LOADINGSTEP::SETSTEP ( 11 );
	NLOADINGTHREAD::EndThread();

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,		dwOldMod );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	dwColorArg1 );

	return S_OK;
}

HRESULT GLGaeaClient::ReqBusFB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_BUS_FB *pNetMsg = (GLMSG::SNETPC_REQ_BUS_FB *) nmg;
	switch ( pNetMsg->emFB )
	{
	case EMBUS_TAKE_FAIL:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMBUS_TAKE_FAIL") );
		break;
	case EMBUS_TAKE_OK:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("EMBUS_TAKE_OK") );
		break;
	case EMBUS_TAKE_TICKET:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMBUS_TAKE_TICKET") );
		break;
	case EMBUS_TAKE_CONDITION:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMBUS_TAKE_CONDITION") );
		break;
	};

	return S_OK;
}

HRESULT GLGaeaClient::ReqTaxiFB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_TAXI_FB *pNetMsg = (GLMSG::SNETPC_REQ_TAXI_FB *) nmg;
	switch ( pNetMsg->emFB )
	{
	case EMTAXI_TAKE_FAIL:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMTAXI_TAKE_FAIL") );
		break;
	case EMTAXI_TAKE_OK:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("EMTAXI_TAKE_OK") );
		break;
	case EMTAXI_TAKE_TICKET:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMTAXI_TAKE_TICKET") );
		break;
	case EMTAXI_TAKE_MONEY:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMTAXI_TAKE_MONEY") );
		break;
	case EMTAXI_TAKE_CONDITION:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMTAXI_TAKE_CONDITION") );
		break;
	case EMTAXI_TAKE_MAPFAIL:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMTAXI_TAKE_MAPFAIL") );
		break;
	case EMTAXI_TAKE_STATIONFAIL:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMTAXI_TAKE_STATIONFAIL") );
		break;
	case EMTAXI_TAKE_NPCFAIL:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMTAXI_TAKE_NPCFAIL") );
		break;	
	};

	return S_OK;

}

HRESULT GLGaeaClient::DropOutCrow ( NET_MSG_GENERIC *nmg )
{
	if ( !m_pLandMClient )	return E_FAIL;
	GLMSG::SNETDROP_OUT *pNetMsg = (GLMSG::SNETDROP_OUT *) nmg;
	//CDebugSet::ToListView ( "DROPOUT_CROW [%s] : %d", COMMENT::CROW[pNetMsg->emCrow].c_str(), pNetMsg->dwID );

	STARID sTARID;
	BYTE cNUM = pNetMsg->GETAMOUNT();
	for ( BYTE i=0; i<cNUM; ++i )
	{
		bool bOK = pNetMsg->POP_CROW( sTARID );
		if ( !bOK )		return E_FAIL;

		switch ( sTARID.wCrow )
		{
		case CROW_PC:
			m_pLandMClient->DropOutChar ( sTARID.wID );
			break;

		case CROW_NPC:
		case CROW_MOB:
			m_pLandMClient->DropOutCrow ( sTARID.wID );
			break;
		case CROW_MATERIAL:
			m_pLandMClient->DropOutMaterial ( sTARID.wID );
			break;

		case CROW_ITEM:
			m_pLandMClient->DropOutItem ( sTARID.wID );
			break;

		case CROW_MONEY:
			m_pLandMClient->DropOutMoney ( sTARID.wID );
			break;

		case CROW_PET:
			m_pLandMClient->DropOutPet ( sTARID.wID );
			break;

		case CROW_SUMMON:
			m_pLandMClient->DropOutSummon ( sTARID.wID );
			break;
		};
	}

	return S_OK;
}

BOOL GLGaeaClient::IsVisibleCV ( const STARGETID & sTargetID )
{
	if ( !m_pLandMClient )	return FALSE;

	if ( sTargetID.dwID == EMTARGET_NULL )		return FALSE;

	if ( sTargetID.emCrow == CROW_PC )
	{
		PGLCHARCLIENT pChar = m_pLandMClient->GetChar ( sTargetID.dwID );
		if ( pChar )	return pChar->IsCollisionVolume();

		if ( GLGaeaClient::GetInstance().GetCharacter()->m_dwGaeaID == sTargetID.dwID )
			return TRUE;
	}
	else if ( sTargetID.emCrow == CROW_NPC || sTargetID.emCrow == CROW_MOB ) 
	{
		PGLCROWCLIENT pCrow = m_pLandMClient->GetCrow ( sTargetID.dwID );
		if ( pCrow )	return pCrow->IsCollisionVolume();
	}
	else if ( sTargetID.emCrow == CROW_MATERIAL ) 
	{
		PGLMATERIALCLIENT pMaterial = m_pLandMClient->GetMaterial ( sTargetID.dwID );
		if ( pMaterial )	return pMaterial->IsCollisionVolume();
	}
	else if ( sTargetID.emCrow == CROW_SUMMON )
	{
		PGLANYSUMMON pSummon = m_pLandMClient->GetSummon ( sTargetID.dwID );
		if ( pSummon )	return pSummon->IsCollisionVolume();

	}
	else
		GASSERT(0&&"emCrow�� �߸��� ������ �Դϴ�." );

	return FALSE;
}

PGLCHARCLIENT GLGaeaClient::GetChar ( DWORD dwID )
{
	if ( !m_pLandMClient )	return NULL;

	return m_pLandMClient->GetChar ( dwID );
}

PGLANYSUMMON GLGaeaClient::GetSummon ( DWORD dwID )
{
	if ( !m_pLandMClient )	return NULL;

	return m_pLandMClient->GetSummon ( dwID );
}

BOOL GLGaeaClient::ValidCheckTarget ( const STARGETID & sTargetID )
{
	if ( !m_pLandMClient )	return FALSE;

	if ( sTargetID.dwID == EMTARGET_NULL )		return FALSE;

	if ( sTargetID.emCrow == CROW_PC )
	{
		PGLCHARCLIENT pChar = m_pLandMClient->GetChar ( sTargetID.dwID );
		if ( pChar ) return TRUE;

		if ( GLGaeaClient::GetInstance().GetCharacter()->m_dwGaeaID == sTargetID.dwID )
			return TRUE;
	}
	else if ( sTargetID.emCrow == CROW_NPC || sTargetID.emCrow == CROW_MOB )
	{
		PGLCROWCLIENT pCrow = m_pLandMClient->GetCrow ( sTargetID.dwID );
		if ( pCrow ) return TRUE;
	}
	else if ( sTargetID.emCrow == CROW_MATERIAL )
	{
		PGLMATERIALCLIENT pMaterial = m_pLandMClient->GetMaterial ( sTargetID.dwID );
		if ( pMaterial ) return TRUE;
	}
	else if ( sTargetID.emCrow == CROW_SUMMON )
	{
		PGLANYSUMMON pSummon = m_pLandMClient->GetSummon ( sTargetID.dwID );
		if ( pSummon ) return TRUE;

		/*skill summon, Juver, 2017/10/08 */
		GLSummonClient*	psummon_client = GetSummonClient( sTargetID.dwID );
		if( psummon_client )	return TRUE;

		//if ( GLGaeaClient::GetInstance().GetSummonClient()->m_dwGUID == sTargetID.dwID )
		//	return TRUE;
	}
	else
		GASSERT(0&&"emCrow�� �߸��� ������ �Դϴ�." );

	return FALSE;
}

D3DXVECTOR3 GLGaeaClient::GetTargetPos ( const STARGETID &sTargetID )
{
	if ( !m_pLandMClient )	return VERROR();

	//	Note : Ÿ���� ��ġ ������ ������.
	if ( sTargetID.emCrow == CROW_PC )
	{
		PGLCHARCLIENT pChar = m_pLandMClient->GetChar ( sTargetID.dwID );
		if ( pChar )
			return pChar->GetPosition();

		if ( GetCharacter()->m_dwGaeaID == sTargetID.dwID )
			return GetCharacterPos();
	}
	else if ( sTargetID.emCrow == CROW_NPC || sTargetID.emCrow == CROW_MOB )
	{
		PGLCROWCLIENT pCrow = m_pLandMClient->GetCrow ( sTargetID.dwID );
		if ( pCrow )	return pCrow->GetPosition();
	}
	else if ( sTargetID.emCrow == CROW_MATERIAL ) 
	{
		PGLMATERIALCLIENT pMaterial = m_pLandMClient->GetMaterial ( sTargetID.dwID );
		if ( pMaterial )	return pMaterial->GetPosition();
	}
	else if ( sTargetID.emCrow == CROW_ITEM )
	{
		PITEMCLIENTDROP pItem = m_pLandMClient->GetItem ( sTargetID.dwID );
		if ( pItem )	return pItem->vPos;
	}
	else if ( sTargetID.emCrow == CROW_MONEY )
	{
		PMONEYCLIENTDROP pMoney = m_pLandMClient->GetMoney ( sTargetID.dwID );
		if ( pMoney )	return pMoney->vPos;
	}
	else if ( sTargetID.emCrow == CROW_PET )
	{
		if ( GetPetClient()->m_dwGUID == sTargetID.dwID ) return GetPetClient()->GetPosition ();
		else
		{
			PGLANYPET pAnyPet = m_pLandMClient->GetPet ( sTargetID.dwID );
			if ( pAnyPet ) return pAnyPet->GetPosition ();
		}
	}
	else if ( sTargetID.emCrow == CROW_SUMMON )
	{
		//if ( GetSummonClient()->m_dwGUID == sTargetID.dwID ) return GetPetClient()->GetPosition ();

		/*skill summon, Juver, 2017/10/08 */
		GLSummonClient*	psummon_client = GetSummonClient( sTargetID.dwID );
		if( psummon_client )	return psummon_client->GetPosition ();
		else
		{
			PGLANYSUMMON pAnySummon = m_pLandMClient->GetSummon ( sTargetID.dwID );
			if ( pAnySummon ) return pAnySummon->GetPosition ();
		}
	}
	else	GASSERT(0&&"emCrow�� �߸��� ������ �Դϴ�." );

	return VERROR();
}


namespace
{
	void MINDETECTAABB ( D3DXVECTOR3 &vMax, D3DXVECTOR3 &vMin, const float fRate, const float fMinLeng )
	{
		float fLength(0);
		fLength = vMax.x - vMin.x;
		vMax.x += ( fLength*fRate - fLength ) / 2.0f;
		vMin.x -= ( fLength*fRate - fLength ) / 2.0f;

		fLength = vMax.x - vMin.x;
		if ( fLength < fMinLeng )
		{
			vMax.x += fMinLeng/2.0f;
			vMin.x -= fMinLeng/2.0f;
		}

		fLength = vMax.y - vMin.y;
		vMax.y += ( fLength*fRate - fLength ) / 2.0f;
		vMin.y -= ( fLength*fRate - fLength ) / 2.0f;

		fLength = vMax.y - vMin.y;
		if ( fLength < fMinLeng )
		{
			vMax.y += fMinLeng/2.0f;
			vMin.y -= fMinLeng/2.0f;
		}

		fLength = vMax.z - vMin.z;
		vMax.z += ( fLength*fRate - fLength ) / 2.0f;
		vMin.z -= ( fLength*fRate - fLength ) / 2.0f;

		fLength = vMax.z - vMin.z;
		if ( fLength < fMinLeng )
		{
			vMax.z += fMinLeng/2.0f;
			vMin.z -= fMinLeng/2.0f;
		}
	}
}

DETECTMAP* GLGaeaClient::DetectCrowDie ( const D3DXVECTOR3 &vFromPt, const D3DXVECTOR3 &vTargetPt )
{
	STARGETID TargetID(CROW_MOB,EMTARGET_NULL);
	if ( !m_pLandMClient )	return NULL;

	//	DetectID Ŭ����.
	if ( !m_vecDetectID.empty() )	m_vecDetectID.erase ( m_vecDetectID.begin(), m_vecDetectID.end() );

	STARGETID findTargetID;

	//	��.
	{
		GLCROWCLIENTLIST *pCrowList = m_pLandMClient->GetCrowList ();
		GLCROWCLIENTNODE *pCrowCur = pCrowList->m_pHead;
		for ( ; pCrowCur; pCrowCur = pCrowCur->pNext )
		{
			PGLCROWCLIENT pCrow= pCrowCur->Data;

			if ( !pCrow->IsDie() )			continue;

			BOOL bCol = COLLISION::IsCollisionLineToAABB ( vFromPt, vTargetPt, pCrow->m_vMax, pCrow->m_vMin );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = pCrow->m_pCrowData->m_emCrow;
				findTargetID.dwID = pCrow->m_dwGlobID;
				findTargetID.vPos = pCrow->GetPosition ();

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	//	ĳ����.
	{
		GLCHARCLIENTLIST *pCharList = m_pLandMClient->GetCharList ();
		GLCHARCLIENTNODE *pCharCur = pCharList->m_pHead;
		for ( ; pCharCur; pCharCur = pCharCur->pNext )
		{
			PGLCHARCLIENT pChar = pCharCur->Data;
			if ( !pChar->IsVisibleDetect() )	continue;
			if ( !pChar->IsDie() )				continue;

			BOOL bCol = COLLISION::IsCollisionLineToAABB ( vFromPt, vTargetPt, pChar->m_vMax, pChar->m_vMin );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = CROW_PC;
				findTargetID.dwID = pChar->m_dwGaeaID;
				findTargetID.vPos = pChar->GetPosition ();

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	if ( m_vecDetectID.empty() )	return NULL;
	
	std::sort ( m_vecDetectID.begin(), m_vecDetectID.end(), STAR_ORDER() );
	return &m_vecDetectID;
}

DETECTMAP* GLGaeaClient::DetectCrow ( DWORD emCrow, const D3DXVECTOR3 &vFromPt, const D3DXVECTOR3 &vTargetPt )
{
	STARGETID TargetID(CROW_MOB,EMTARGET_NULL);
	if ( !m_pLandMClient )	return NULL;

	//	DetectID Ŭ����.
	if ( !m_vecDetectID.empty() )	m_vecDetectID.erase ( m_vecDetectID.begin(), m_vecDetectID.end() );

	STARGETID findTargetID;
	
	//	��.
	if ( emCrow & CROW_EX_MOB )
	{
		GLCROWCLIENTLIST *pCrowList = m_pLandMClient->GetCrowList ();
		GLCROWCLIENTNODE *pCrowCur = pCrowList->m_pHead;
		for ( ; pCrowCur; pCrowCur = pCrowCur->pNext )
		{
			PGLCROWCLIENT pCrow= pCrowCur->Data;

			if ( !pCrow->IsValidBody() )							continue;

			BOOL bCol = COLLISION::IsCollisionLineToAABB ( vFromPt, vTargetPt, pCrow->m_vMax, pCrow->m_vMin );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = pCrow->m_pCrowData->m_emCrow;
				findTargetID.dwID = pCrow->m_dwGlobID;
				findTargetID.vPos = pCrow->GetPosition ();

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	//	Material
	if ( emCrow & CROW_EX_MATERIAL )
	{
		GLMATERIALCLIENTLIST *pMaterialList = m_pLandMClient->GetMaterialList ();
		GLMATERIALCLIENTNODE *pMaterialCur = pMaterialList->m_pHead;
		for ( ; pMaterialCur; pMaterialCur = pMaterialCur->pNext )
		{
			PGLMATERIALCLIENT pMaterial= pMaterialCur->Data;

			if ( !pMaterial->IsValidBody() )							continue;

			BOOL bCol = COLLISION::IsCollisionLineToAABB ( vFromPt, vTargetPt, pMaterial->m_vMax, pMaterial->m_vMin );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = pMaterial->m_pCrowData->m_emCrow;
				findTargetID.dwID = pMaterial->m_dwGlobID;
				findTargetID.vPos = pMaterial->GetPosition ();

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	//	ĳ����.
	if ( emCrow & CROW_EX_PC )
	{
		GLCHARCLIENTLIST *pCharList = m_pLandMClient->GetCharList ();
		GLCHARCLIENTNODE *pCharCur = pCharList->m_pHead;
		for ( ; pCharCur; pCharCur = pCharCur->pNext )
		{
			PGLCHARCLIENT pChar = pCharCur->Data;
			if ( !pChar->IsVisibleDetect() )	continue;
			if ( !pChar->IsValidBody() )		continue;

			BOOL bCol = COLLISION::IsCollisionLineToAABB ( vFromPt, vTargetPt, pChar->m_vMax, pChar->m_vMin );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = CROW_PC;
				findTargetID.dwID = pChar->m_dwGaeaID;
				findTargetID.vPos = pChar->GetPosition ();

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	//	��ȯ��
	if ( emCrow & CROW_EX_SUMMON )
	{
		GLANYSUMMONLIST *pSummonList = m_pLandMClient->GetSummonList();
		GLANYSUMMONNODE *pSummonCur = pSummonList->m_pHead;
		for ( ; pSummonCur; pSummonCur = pSummonCur->pNext )
		{
			PGLANYSUMMON pSummon = pSummonCur->Data;
			if ( !pSummon->IsValidBody() )		continue;

			BOOL bCol = COLLISION::IsCollisionLineToAABB ( vFromPt, vTargetPt, pSummon->m_vMax, pSummon->m_vMin );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = CROW_SUMMON;
				findTargetID.dwID = pSummon->m_dwGUID;
				findTargetID.vPos = pSummon->GetPosition ();

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	//	Item.
	if ( emCrow & CROW_EX_ITEM )
	{
		ITEMCLIENTDROPLIST *pItemList = m_pLandMClient->GetItemList();
		ITEMCLIENTDROPNODE *pItemCur = pItemList->m_pHead;
		for ( ; pItemCur; pItemCur = pItemCur->pNext )
		{
			const CItemClientDrop &sItemDrop = *pItemCur->Data;

			bool bCOLL = sItemDrop.IsCollision ( vFromPt, vTargetPt );

			if ( bCOLL )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = CROW_ITEM;
				findTargetID.dwID = sItemDrop.dwGlobID;
				findTargetID.vPos = sItemDrop.vPos;

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	//	Money.
	if ( emCrow & CROW_EX_MONEY )
	{
		MONEYCLIENTDROPLIST *pMoneyList = m_pLandMClient->GetMoneyList();
		MONEYCLIENTDROPNODE *pMoneyCur = pMoneyList->m_pHead;
		for ( ; pMoneyCur; pMoneyCur = pMoneyCur->pNext )
		{
			const CMoneyClientDrop &sMoneyDrop = *pMoneyCur->Data;

			bool bCOLL = sMoneyDrop.IsCollision ( vFromPt, vTargetPt );
			if ( bCOLL )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = CROW_MONEY;
				findTargetID.dwID = sMoneyDrop.dwGlobID;
				findTargetID.vPos = sMoneyDrop.vPos;

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	if ( m_vecDetectID.empty() )	return NULL;

	std::sort ( m_vecDetectID.begin(), m_vecDetectID.end(), STAR_ORDER() );
	return &m_vecDetectID;
}

DETECTMAP* GLGaeaClient::DetectCrow ( DWORD emCrow, const D3DXVECTOR3 &vTarPos, const float fLength )
{
	STARGETID TargetID(CROW_MOB,EMTARGET_NULL);
	if ( !m_pLandMClient )	return NULL;

	DxLandMan *pLandMan = GLGaeaClient::GetInstance().GetActiveMap()->GetLandMan();
	if( !pLandMan )			return NULL;

	//	DetectID Ŭ����.
	if ( !m_vecDetectID.empty() )	m_vecDetectID.erase ( m_vecDetectID.begin(), m_vecDetectID.end() );

	STARGETID findTargetID;

	//	��.
	if ( emCrow & CROW_EX_MOB )
	{
		GLCROWCLIENTLIST *pCrowList = m_pLandMClient->GetCrowList ();
		GLCROWCLIENTNODE *pCrowCur = pCrowList->m_pHead;
		for ( ; pCrowCur; pCrowCur = pCrowCur->pNext )
		{
			PGLCROWCLIENT pCrow= pCrowCur->Data;

			if ( !pCrow->IsValidBody() ) continue;

			/*anti shit*/
			if ( pLandMan->IsLayerSimpleCollision(vTarPos, pCrow->GetPosition() ) )	continue;

			float fTarRange = pCrow->GETBODYRADIUS() + fLength;

			BOOL bCol = COLLISION::IsSpherePointCollision ( pCrow->GetPosition(), vTarPos, fTarRange );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = pCrow->m_pCrowData->m_emCrow;
				findTargetID.dwID = pCrow->m_dwGlobID;
				findTargetID.vPos = pCrow->GetPosition ();

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

    //	Material
	if ( emCrow & CROW_EX_MATERIAL )
	{
		GLMATERIALCLIENTLIST *pMaterialList = m_pLandMClient->GetMaterialList ();
		GLMATERIALCLIENTNODE *pMaterialCur = pMaterialList->m_pHead;
		for ( ; pMaterialCur; pMaterialCur = pMaterialCur->pNext )
		{
			PGLMATERIALCLIENT pMaterial= pMaterialCur->Data;

			if ( !pMaterial->IsValidBody() ) continue;

			float fTarRange = pMaterial->GetBodyRadius() + fLength;

			BOOL bCol = COLLISION::IsSpherePointCollision ( pMaterial->GetPosition(), vTarPos, fTarRange );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = pMaterial->m_pCrowData->m_emCrow;
				findTargetID.dwID = pMaterial->m_dwGlobID;
				findTargetID.vPos = pMaterial->GetPosition ();

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	//	ĳ����.
	if ( emCrow & CROW_EX_PC )
	{
		GLCHARCLIENTLIST *pCharList = m_pLandMClient->GetCharList ();
		GLCHARCLIENTNODE *pCharCur = pCharList->m_pHead;
		for ( ; pCharCur; pCharCur = pCharCur->pNext )
		{
			PGLCHARCLIENT pChar = pCharCur->Data;
			if ( !pChar->IsVisibleDetect() )		continue;
			if ( !pChar->IsValidBody() )			continue;

			/*anti shit*/
			if ( pLandMan->IsLayerSimpleCollision(vTarPos, pChar->GetPosition() ) )	continue;

			BOOL bCol = COLLISION::IsSpherePointCollision ( pChar->GetPosition(), vTarPos, fLength );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = CROW_PC;
				findTargetID.dwID = pChar->m_dwGaeaID;
				findTargetID.vPos = pChar->GetPosition ();

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	//	Item.
	if ( emCrow & CROW_EX_ITEM )
	{
		ITEMCLIENTDROPLIST *pItemList = m_pLandMClient->GetItemList();
		ITEMCLIENTDROPNODE *pItemCur = pItemList->m_pHead;
		for ( ; pItemCur; pItemCur = pItemCur->pNext )
		{
			CItemClientDrop &sItemDrop = *pItemCur->Data;

			BOOL bCol = COLLISION::IsSpherePointCollision ( sItemDrop.vPos, vTarPos, fLength );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = CROW_ITEM;
				findTargetID.dwID = sItemDrop.dwGlobID;
				findTargetID.vPos = sItemDrop.vPos;

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	//	Money.
	if ( emCrow & CROW_EX_MONEY )
	{
		MONEYCLIENTDROPLIST *pMoneyList = m_pLandMClient->GetMoneyList();
		MONEYCLIENTDROPNODE *pMoneyCur = pMoneyList->m_pHead;
		for ( ; pMoneyCur; pMoneyCur = pMoneyCur->pNext )
		{
			CMoneyClientDrop &sMoneyDrop = *pMoneyCur->Data;

			BOOL bCol = COLLISION::IsSpherePointCollision ( sMoneyDrop.vPos, vTarPos, fLength );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = CROW_MONEY;
				findTargetID.dwID = sMoneyDrop.dwGlobID;
				findTargetID.vPos = sMoneyDrop.vPos;

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	/*skill summon, Juver, 2017/10/08 */
	if ( emCrow & CROW_EX_SUMMON )
	{
		GLANYSUMMONLIST *pSummonList = m_pLandMClient->GetSummonList();
		GLANYSUMMONNODE *pSummonCur = pSummonList->m_pHead;
		for ( ; pSummonCur; pSummonCur = pSummonCur->pNext )
		{
			PGLANYSUMMON pSummon = pSummonCur->Data;
			if ( !pSummon->IsValidBody() )		continue;

			float fTarRange = pSummon->GETBODYRADIUS() + fLength;

			BOOL bCol = COLLISION::IsSpherePointCollision ( pSummon->GetPosition(), vTarPos, fTarRange );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = CROW_SUMMON;
				findTargetID.dwID = pSummon->m_dwGUID;
				findTargetID.vPos = pSummon->GetPosition ();

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	if ( m_vecDetectID.empty() )	return NULL;
	
	std::sort ( m_vecDetectID.begin(), m_vecDetectID.end(), STAR_ORDER() );
	return &m_vecDetectID;
}

BOOL GLGaeaClient::IsCollisionMobToPoint ( const D3DXVECTOR3 &vPoint, const WORD wBodyRadius )
{
	if ( !m_pLandMClient )	return FALSE;

	GLCROWCLIENTLIST *pCrowList = m_pLandMClient->GetCrowList ();
	GLCROWCLIENTNODE *pCrowCur = pCrowList->m_pHead;
	for ( ; pCrowCur; pCrowCur = pCrowCur->pNext )
	{
		PGLCROWCLIENT pCrow = pCrowCur->Data;
		if ( !pCrow->IsValidBody() )		continue;
		if ( !pCrow->IsHaveVisibleBody() )	continue;

		D3DXVECTOR3 vDist = pCrow->GetPosition() - vPoint;
		if ( DxIsZeroVector(vDist) )	return TRUE;

		vDist.y = 0.0f;
		float fDist = D3DXVec3Length ( &vDist );

		if ( fDist < (wBodyRadius+pCrow->GETBODYRADIUS()) )		return TRUE;
	}

	return FALSE;
}

HRESULT GLGaeaClient::InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	GASSERT(pd3dDevice);
	m_pd3dDevice = pd3dDevice;

	if ( m_pLandMClient )
	{
		m_pLandMClient->ActiveMap();
		m_pLandMClient->InitDeviceObjects ( m_pd3dDevice );
	}

	DxEffectMan::GetInstance().SetCrowTracer ( this );

	DxSoundLib::GetInstance()->CreateSound ( "GRINDING_SUCCEED", GLCONST_CHAR::strGRINDING_SUCCEED.c_str(), SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "GRINDING_FAIL", GLCONST_CHAR::strGRINDING_FAIL.c_str(), SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "GRINDING_RESET", GLCONST_CHAR::strGRINDING_RESET.c_str(), SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "GRINDING_BROKEN", GLCONST_CHAR::strGRINDING_BROKEN.c_str(), SFX_SOUND );

	DxSoundLib::GetInstance()->CreateSound ( "ITEMDROP_SUIT", GLCONST_CHAR::strITEMDROP_SUIT.c_str(), SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "ITEMDROP_WAPON", GLCONST_CHAR::strITEMDROP_WAPON.c_str(), SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "ITEMDROP_SHOES", GLCONST_CHAR::strITEMDROP_SHOES.c_str(), SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "ITEMDROP_RING", GLCONST_CHAR::strITEMDROP_RING.c_str(), SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "ITEMDROP_?BOX", GLCONST_CHAR::strITEMDROP_QBOX.c_str(), SFX_SOUND );

	DxSoundLib::GetInstance()->CreateSound ( "ITEMDROP_SCROLL", GLCONST_CHAR::strITEMDROP_SCROLL.c_str(), SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "ITEMDROP_COIN", GLCONST_CHAR::strITEMDROP_COIN.c_str(), SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "ITEMDROP_DRUGS", GLCONST_CHAR::strITEMDROP_DRUGS.c_str(), SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "PICKUP_ITEM", GLCONST_CHAR::strPICKUP_ITEM.c_str(), SFX_SOUND );
	
	DxSoundLib::GetInstance()->CreateSound ( "QITEM_FACT", GLCONST_CHAR::strQITEM_FACT.c_str(), SFX_SOUND );
	
	
	DxSoundLib::GetInstance()->CreateSound ( "GAMBLING_SHUFFLE", GLCONST_CHAR::strGAMBLING_SHUFFLE.c_str(), SFX_SOUND ); //Monster7j
	DxSoundLib::GetInstance()->CreateSound ( "GAMBLING_WIN", GLCONST_CHAR::strGAMBLING_WIN.c_str(), SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "GAMBLING_LOSE", GLCONST_CHAR::strGAMBLING_LOSE.c_str(), SFX_SOUND );

	DxSoundLib::GetInstance()->CreateSound ( "PKCOMBO_DOUBLE", GLCONST_CHAR::strPKCOMBO_DOUBLE.c_str(), SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "PKCOMBO_TRIPLE", GLCONST_CHAR::strPKCOMBO_TRIPLE.c_str(), SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "PKCOMBO_ULTRA", GLCONST_CHAR::strPKCOMBO_ULTRA.c_str(), SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "PKCOMBO_RAMPAGE", GLCONST_CHAR::strPKCOMBO_RAMPAGE.c_str(), SFX_SOUND );

	DxSoundLib::GetInstance()->CreateSound ( "PKCOMBO_KILLING_SPREE", "killing_spree.wav", SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "PKCOMBO_DOMINATING", "dominating.wav", SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "PKCOMBO_MEGA_KILL", "megakill.wav", SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "PKCOMBO_UNSTOPPABLE", "unstoppable.wav", SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "PKCOMBO_WICKED_SICK", "whickedsick.wav", SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "PKCOMBO_MONSTER_KILL", "monster_kill.wav", SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "PKCOMBO_GODLIKE", "godlike.wav", SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "PKCOMBO_HOLY_SHIT", "holyshit.wav", SFX_SOUND );
	DxSoundLib::GetInstance()->CreateSound ( "PKCOMBO_OWNAGE", "ownage.wav", SFX_SOUND );
	return S_OK;
}

HRESULT GLGaeaClient::RestoreDeviceObjects ()
{
	GASSERT(m_pd3dDevice&&"m_pd3dDevice �� �ʱ�ȭ ���� �ʾҽ��ϴ�.");

	if ( m_pLandMClient )
	{
		m_pLandMClient->RestoreDeviceObjects ();
	}

	m_Character.RestoreDeviceObjects ( m_pd3dDevice );
	m_Pet.RestoreDeviceObjects ( m_pd3dDevice );

	//m_Summon.RestoreDeviceObjects ( m_pd3dDevice );
	
	/*skill summon, Juver, 2017/10/08 */
	for( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		m_Summon[i].RestoreDeviceObjects( m_pd3dDevice );

	return S_OK;
}

HRESULT GLGaeaClient::InvalidateDeviceObjects ()
{
	if ( m_pLandMClient )
	{
		m_pLandMClient->InvalidateDeviceObjects ();
	}

	m_Character.InvalidateDeviceObjects ();
	m_Pet.InvalidateDeviceObjects ();

	//m_Summon.InvalidateDeviceObjects();

	/*skill summon, Juver, 2017/10/08 */
	for( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		m_Summon[i].InvalidateDeviceObjects();

	return S_OK;
}

HRESULT GLGaeaClient::DeleteDeviceObjects ()
{
	if ( m_pLandMClient )
	{
		m_pLandMClient->DeleteDeviceObjects ();
		SAFE_DELETE(m_pLandMClient);
	}

	m_Character.DeleteDeviceObjects();
	m_Pet.DeleteDeviceObjects ();

	//m_Summon.DeleteDeviceObjects ();

	/*skill summon, Juver, 2017/10/08 */
	for( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		m_Summon[i].DeleteDeviceObjects();

	return S_OK;
}

HRESULT GLGaeaClient::FrameMove ( float fTime, float fElapsedTime )
{
	m_fAge += fElapsedTime;

	//	Note : ����Ʈ �˻��� ����Ʈ ���� ������Ʈ.
	//
	CInnerInterface::GetInstance().SetGateOpen ( FALSE );
	if ( m_pLandMClient )
	{
		PROFILE_BEGIN("GLLandManClient::FrameMove");
		m_pLandMClient->FrameMove ( fTime, fElapsedTime );
		PROFILE_END("GLLandManClient::FrameMove");
	}

	PROFILE_BEGIN("GLCharacter::FrameMove");
	m_Character.FrameMove ( fTime, fElapsedTime );
	PROFILE_END("GLCharacter::FrameMove");

	// PET
	PROFILE_BEGIN("GLPetClient::FrameMove");
	if ( m_Pet.IsVALID () ) m_Pet.FrameMove ( fTime, fElapsedTime );
	PROFILE_END("GLPetClient::FrameMove");

	// SUMMON
	//if ( m_Summon.IsVALID () ) m_Summon.FrameMove ( fTime, fElapsedTime );

	/*skill summon, Juver, 2017/10/08 */
	for( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
	{
		if ( m_Summon[i].IsVALID() )
			m_Summon[i].FrameMove ( fTime, fElapsedTime );
	}
		
	// ����Ŭ�� ������ �ð� ����
	if ( m_bCLUBBATTLE )
	{
		m_fCLUBBATTLETimer -= fElapsedTime;
		if ( m_fAge > 1.0f ) CInnerInterface::GetInstance().UpdateClubBattleTime( m_fCLUBBATTLETimer );
	}

	if ( m_bClubDeathMatch )
	{
		m_fClubDeathMatchTimer -= fElapsedTime;
		if ( m_fAge > 1.0f ) CInnerInterface::GetInstance().UpdateClubBattleTime( m_fClubDeathMatchTimer );
	}

	// ���� ����ð�
	if ( m_fAge > 1.0f )
	{
		CTimeSpan cElapsedTime(0,0,0,1);
		m_cServerTime += cElapsedTime;

		m_fAge = m_fAge - 1.0f;
	}

	for( int i = 0; i < (int)m_vecPKHistory.size(); ++ i )
	{
		m_vecPKHistory[i].fTimer += fElapsedTime;

		if ( m_vecPKHistory[i].fTimer >= HISTORY_TIMER )
		{
			m_vecPKHistory.erase( m_vecPKHistory.begin() + i );
		}
	}

	return S_OK;
}

HRESULT GLGaeaClient::Render ( CLIPVOLUME &CV )
{
	GASSERT(m_pd3dDevice);

	//	Note : �ϴ� ������..!!
	PROFILE_BEGIN2("DxSkyMan::Render");
	DxSkyMan::GetInstance().Render( m_pd3dDevice );
	PROFILE_END2("DxSkyMan::Render");

	//	Note : ���� ������..
	PROFILE_BEGIN2("m_pLandMClient::Render");
	if ( m_pLandMClient )
	{
		m_pLandMClient->Render ( CV );
	}
	PROFILE_END2("m_pLandMClient::Render");

	// Note : �׸��� Clear
	PROFILE_BEGIN2("DxShadowMap::Render");
	DxShadowMap::GetInstance().ClearShadow( m_pd3dDevice );
	PROFILE_END2("DxShadowMap::Render");
	
	g_bPREVIEW_CHAR_RENDER = FALSE;

	PROFILE_BEGIN2("m_Character::Render");
	m_Character.Render( m_pd3dDevice );
	PROFILE_END2("m_Character::Render");

	PROFILE_BEGIN2("m_Character::RenderShadow,RenderReflect");
	m_Character.RenderShadow( m_pd3dDevice );
	m_Character.RenderReflect( m_pd3dDevice );
	PROFILE_END2("m_Character::RenderShadow,RenderReflect");

	// PET
	PROFILE_BEGIN2("m_Pet::Render,RenderShadow,RenderReflect");
	m_Pet.Render ( m_pd3dDevice );
	m_Pet.RenderShadow ( m_pd3dDevice );
	m_Pet.RenderReflect ( m_pd3dDevice );
	PROFILE_END2("m_Pet::Render,RenderShadow,RenderReflect");

	//m_Summon.Render ( m_pd3dDevice );
	//m_Summon.RenderShadow ( m_pd3dDevice );
	//m_Summon.RenderReflect ( m_pd3dDevice );

	/*skill summon, Juver, 2017/10/08 */
	for( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
	{
		m_Summon[i].Render ( m_pd3dDevice );
		m_Summon[i].RenderShadow ( m_pd3dDevice );
		m_Summon[i].RenderReflect ( m_pd3dDevice );
	}

	PROFILE_BEGIN2("m_pLandMClient::char,crow,item");
	if ( m_pLandMClient )
	{
		m_pLandMClient->Render_MobItem ( CV );
	}
	PROFILE_END2("m_pLandMClient::char,crow,item");

	// Note : ShadowMap ImageBlur
	PROFILE_BEGIN2("m_pLandMClient::LastImageBlur");
	DxShadowMap::GetInstance().LastImageBlur( m_pd3dDevice );
	PROFILE_END2("m_pLandMClient::LastImageBlur");

	return S_OK;
}

HRESULT GLGaeaClient::RenderEff ( CLIPVOLUME &CV )
{
	GASSERT(m_pd3dDevice);

	if ( m_pLandMClient )
	{
		m_pLandMClient->Render_EFF ( CV );
	}

	//	Note : �ϴ� ������..!!
	DxSkyMan::GetInstance().Render_AFTER( m_pd3dDevice );

	return S_OK;
}

void GLGaeaClient::RenderPickAlpha()
{
	if ( m_pLandMClient )
	{
		m_pLandMClient->RenderPickAlpha();
	}
}

void GLGaeaClient::MsgProcess ( NET_MSG_GENERIC* nmg )
{
	if ( !m_pLandMClient )	return;
	CString strTEXT;

	switch ( nmg->nType )
	{
	//PANDORA
	case NET_MSG_PANDORA_BOX_REWARD_RARE_BRD:
		{
			GLMSG::SNET_PANDORA_BOX_REWARD_RARE_BRD* pNetMsg = ( GLMSG::SNET_PANDORA_BOX_REWARD_RARE_BRD* ) nmg;
		}
		break;
	case NET_MSG_PANDORA_BOX_RESULT:
		{
			GLMSG::SNET_PANDORA_BOX_RESULT* pNetMsg = ( GLMSG::SNET_PANDORA_BOX_RESULT* ) nmg;
			GetCharacter()->m_mapPandoraResults.clear();

			for ( int i = 0; i < GLPANDORA_BOX::PANDORA_RESULT_SIZE; ++i )
				GetCharacter()->m_mapPandoraResults.insert( std::make_pair( pNetMsg->sBOX[i].nIndex, pNetMsg->sBOX[i] ) );

			GetCharacter()->m_sPandoraStatus = pNetMsg->sStatus;
			CInnerInterface::GetInstance().MsgProcess( nmg );
		}
		break;
	case NET_MSG_PANDORA_BOX_BUY_ITEM_FB:
		{
			GLMSG::SNET_PANDORA_BOX_BUY_ITEM_FB* pNetMsg = ( GLMSG::SNET_PANDORA_BOX_BUY_ITEM_FB* ) nmg;
			switch ( pNetMsg->emFB )
			{
			case EMPANDORA_ITEM_BUY_FAILED:
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMPANDORA_ITEM_BUY_FAILED") );
				break;
			case EMPANDORA_ITEM_BUY_GOLD_OK:
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::GREENYELLOW, ID2GAMEINTEXT("EMPANDORA_ITEM_BUY_GOLD_OK") );
				break;
			case EMPANDORA_ITEM_BUY_PREM_OK:
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::GREENYELLOW, ID2GAMEINTEXT("EMPANDORA_ITEM_BUY_PREM_OK") );
				break;
			case EMPANDORA_ITEM_REMOVE_OK:
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::GREENYELLOW, ID2GAMEINTEXT("EMPANDORA_ITEM_REMOVE_OK") );
				break;
			case EMPANDORA_ITEM_BUY_NOGOLD:
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMPANDORA_ITEM_BUY_NOGOLD") );
				break;
			case EMPANDORA_ITEM_BUY_NOEP:
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMPANDORA_ITEM_BUY_NOEP") );
				break;

			}
			CInnerInterface::GetInstance().MsgProcess( nmg );
		}
		break;
	case NET_MSG_PANDORA_BOX_CLEAR_RESULTS_FB:
		{
			GLMSG::SNET_PANDORA_BOX_CLEAR_RESULTS_FB* pNetMsg = ( GLMSG::SNET_PANDORA_BOX_CLEAR_RESULTS_FB* ) nmg;
			switch ( pNetMsg->emFB )
			{
			case EMPANDORA_CLEAR_RESULT_FAILED:
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMPANDORA_CLEAR_RESULT_FAILED") );
				break;
			case EMPANDORA_CLEAR_RESULT_OK:
				GetCharacter()->m_mapPandoraResults.clear();
				break;
			}
		}
		break;
	case NET_MSG_PANDORA_BOX_OPEN_FB:
		{
			GLMSG::SNET_PANDORA_BOX_OPEN_FB* pNetMsg = ( GLMSG::SNET_PANDORA_BOX_OPEN_FB* ) nmg;
			switch ( pNetMsg->emFB )
			{
			case EMPANDORA_OPEN_BOX_FB_FAILED:
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMPANDORA_OPEN_BOX_FB_FAILED") );
				break;
			case EMPANDORA_OPEN_BOX_FB_NOEP:
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMPANDORA_OPEN_BOX_FB_NOEP") );
				break;
			case EMPANDORA_OPEN_BOX_FB_NOGOLD:
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMPANDORA_OPEN_BOX_FB_NOGOLD") );
				break;
			case EMPANDORA_OPEN_BOX_GOLD_FB_OK:
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::GREENYELLOW, ID2GAMEINTEXT("EMPANDORA_OPEN_BOX_GOLD_FB_OK") );
				break;
			case EMPANDORA_OPEN_BOX_PREM_FB_OK:
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::GREENYELLOW, ID2GAMEINTEXT("EMPANDORA_OPEN_BOX_PREM_FB_OK") );
				break;
			}
		}
		break;
	case NET_MSG_PANDORA_BOX_SETTING_INFO:
		{
			GLMSG::SNET_PANDORA_BOX_SETTING_INFO* pNetMsg = ( GLMSG::SNET_PANDORA_BOX_SETTING_INFO* ) nmg;
			GLCONST_CHAR::sPandoraSetting.fPremChance = pNetMsg->fPremChance;
			GLCONST_CHAR::sPandoraSetting.llGold = pNetMsg->llGold;
			GLCONST_CHAR::sPandoraSetting.wEP = pNetMsg->wEP;
			GLCONST_CHAR::sPandoraSetting.fReqDelay = pNetMsg->fReqDelay;
		}
		break;

		/////////////////////////////////////////////////////////////////////////////
	// LG-7 CWRanking
	case NET_MSG_GCTRL_REQ_CW_RANKING_FB:
	{
		GLMSG::SNET_REQ_CW_RANKING_FB* pNetMsg = (GLMSG::SNET_REQ_CW_RANKING_FB*)nmg;

		CString strTemp = (char*)ID2GAMEWORD("CW_RANKING_TITLE", pNetMsg->dwID);

		CInnerInterface::GetInstance().GetCWRankingDisplay()->ResetAll();
		CInnerInterface::GetInstance().GetCWRankingDisplay()->SetCWRankingTitle(strTemp);

		for (int i = 0; i < 10; i++)
		{
			if (pNetMsg->sCWRanking_[i].nIndex >= 0)
				CInnerInterface::GetInstance().GetCWRankingDisplay()->AddCWRanking(pNetMsg->sCWRanking_[i]);
		}

		GLCharacter* pCharacter = GetCharacter();
		if (pCharacter) pCharacter->ReqMyCWRanking();
	}
	break;
	case NET_MSG_GCTRL_REQ_MY_CW_RANKING_FB:
	{
		GLMSG::SNET_REQ_MY_CW_RANKING_FB* pNetMsg = (GLMSG::SNET_REQ_MY_CW_RANKING_FB*)nmg;

		if (pNetMsg->sCWRanking.nIndex >= 0)
			CInnerInterface::GetInstance().GetCWRankingDisplay()->AddMyCWRanking(pNetMsg->sCWRanking);

		CInnerInterface::GetInstance().SetCWRankingDisplayOpen(TRUE);
	}
	break;
	/////////////////////////////////////////////////////////////////////////////
	case NET_MSG_GCTRL_PERIOD:
		{
			GLMSG::SNET_PERIOD *pNetMsg = reinterpret_cast<GLMSG::SNET_PERIOD *> ( nmg );
			GLPeriod::GetInstance().SetPeriod ( pNetMsg->sPRERIODTIME );
		}
		break;

	case NET_MSG_GCTRL_WEATHER:
		{
			GLMSG::SNETPC_WEATHER *pNetMsg = reinterpret_cast<GLMSG::SNETPC_WEATHER *> ( nmg );
			GLPeriod::GetInstance().SetWeather ( pNetMsg->dwWeather );
		}
		break;

	case NET_MSG_GCTRL_MAPWEATHER:
		{
			GLMSG::SNETPC_MAPWEATHER *pNetMsg = reinterpret_cast<GLMSG::SNETPC_MAPWEATHER *> ( nmg );
			DWORD i;
			GLPeriod::GetInstance().ResetOneMapWeather();
			for( i = 0; i < pNetMsg->dwMapWeatherSize; i++ )
			{
				GLPeriod::GetInstance().SetOneMapActiveWeather( pNetMsg->MapWeather[i], FALSE );
			}
			
		}
		break;

	case NET_MSG_GCTRL_WHIMSICAL:
		{
			GLMSG::SNETPC_WHIMSICALWEATHER *pNetMsg = reinterpret_cast<GLMSG::SNETPC_WHIMSICALWEATHER *> ( nmg );
			DxWeatherMan::GetInstance()->ReceiveWhimsical ( pNetMsg->dwWhimsical );
		}
		break;

	case NET_MSG_GCTRL_MAPWHIMSICAL:
		{
			GLMSG::SNETPC_MAPWHIMSICALWEATHER *pNetMsg = reinterpret_cast<GLMSG::SNETPC_MAPWHIMSICALWEATHER *> ( nmg );
			DxWeatherMan::GetInstance()->ReceiveMapWhimsical ( pNetMsg->MapWeather.map_mID, pNetMsg->MapWeather.map_sID, 
															   pNetMsg->MapWeather.dwWhimsicalWeather );
		}
		break;

	case NET_MSG_GCTRL_DROP_ITEM:
		{
			GLMSG::SNETDROP_ITEM *pNetMsg = reinterpret_cast<GLMSG::SNETDROP_ITEM *> ( nmg );
			m_pLandMClient->DropItem ( &pNetMsg->Data );
		}
		break;

	case NET_MSG_GCTRL_DROP_MONEY:
		{
			GLMSG::SNETDROP_MONEY *pNetMsg = reinterpret_cast<GLMSG::SNETDROP_MONEY *> ( nmg );
			m_pLandMClient->DropMoney ( pNetMsg->lnAmount, pNetMsg->vPos, pNetMsg->dwGlobID, pNetMsg->fAge );
		}
		break;


	case NET_MSG_GCTRL_DROP_PC:
		{
			GLMSG::SNETDROP_PC *pNetMsg = (GLMSG::SNETDROP_PC*) nmg;
			SDROP_CHAR &sDropChar = pNetMsg->Data;
			if ( sDropChar.sMapID == m_pLandMClient->GetMapID() )
			{
				if ( sDropChar.dwGaeaID == GETMYGAEAID() )
				{
					CDebugSet::ToListView ( "[ERROR] Drop Char : %s, %d �ڱ��ڽ��� ���̵�.", sDropChar.szName, sDropChar.dwGaeaID );
				}
				else
				{
					//CDebugSet::ToListView ( "Drop Char : %s, %d", sDropChar.szName, sDropChar.dwGaeaID );
					m_pLandMClient->DropChar ( &sDropChar );
				}
			}
			else
			{
				CDebugSet::ToListView ( "sDropChar.sMapID != m_pLandMClient->GetMapID()" );
			}
		}
		break;

	case NET_MSG_GCTRL_DROP_CROW:
		{
			GLMSG::SNETDROP_CROW *pNetMsg = (GLMSG::SNETDROP_CROW*) nmg;
			SDROP_CROW &sDropCrow = pNetMsg->Data;
			if ( sDropCrow.sMapID == m_pLandMClient->GetMapID() )
			{
				//CDebugSet::ToListView ( "Drop Crow : [%d,%d], GLOB ID %d",
				//	sDropCrow.sNativeID.wMainID, sDropCrow.sNativeID.wSubID, sDropCrow.dwGlobID);

				m_pLandMClient->DropCrow ( &sDropCrow );
			}
			else
			{
				CDebugSet::ToListView ( "sDropCrow.sMapID != m_pLandMClient->GetMapID()" );
			}
		}
		break;
	case NET_MSG_GCTRL_DROP_MATERIAL:
		{
			GLMSG::SNETDROP_MATERIAL *pNetMsg = (GLMSG::SNETDROP_MATERIAL*) nmg;
			SDROP_MATERIAL &sDropMaterial = pNetMsg->Data;
			if ( sDropMaterial.sMapID == m_pLandMClient->GetMapID() )
			{
				m_pLandMClient->DropMaterial ( &sDropMaterial );
			}
			else
			{
				CDebugSet::ToListView ( "sDropMaterial.sMapID != m_pLandMClient->GetMapID()" );
			}
		}
		break;

	

	case NET_MSG_GCTRL_CHANGE_NAMEMAP:
		{
			GLMSG::SNETPC_CHANGE_NAMEMAP *pNetMsg = (GLMSG::SNETPC_CHANGE_NAMEMAP*) nmg;
			m_pLandMClient->ChangeCharMap ( pNetMsg->szOldName, pNetMsg->szNewName );
		}
		break;

	case NET_MSG_GCTRL_DROP_OUT:		DropOutCrow(nmg);	break;
	case NET_MSG_GCTRL_REQ_GATEOUT_FB:	MoveActiveMap(nmg);	break;
	case NET_MSG_GCTRL_CREATE_INSTANT_MAP_FB: CreateInstantMap(nmg);	break;
	case NET_MSG_GCTRL_REQ_REBIRTH_FB:	ReBirthFB(nmg);		break;
	case NET_MSG_GCTRL_REQ_RECALL_FB:	ReCallFB(nmg);		break;
	case NET_MSG_REQ_MUST_LEAVE_MAP_FB: ReLvMustLeaveFB(nmg); break;
	case NET_MSG_GCTRL_REQ_BUS_FB:		ReqBusFB(nmg);		break;
	case NET_MSG_GCTRL_REQ_TAXI_FB:		ReqTaxiFB(nmg);		break;
	case NET_MSG_GCTRL_REQ_TELEPORT_FB:	TeleportFB(nmg);	break;

	case NET_MSG_ALLIANCE_BATTLE_BEGIN:
		{
			GLMSG::SNET_ALLIANCE_BATTLE_BEGIN *pNetMsg1 = (GLMSG::SNET_ALLIANCE_BATTLE_BEGIN*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("ALLIANCE_BATTLE_BEGIN"),
				pNetMsg1->szParty1, pNetMsg1->szParty2 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_ALLIANCE_BATTLE_BEGIN_REFUSE:
		{
			GLMSG::SNET_ALLIANCE_BATTLE_BEGIN_REFUSE *pNetMsg2 = (GLMSG::SNET_ALLIANCE_BATTLE_BEGIN_REFUSE*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("ALLIANCE_BATTLE_BEGIN_REFUSE"),
				pNetMsg2->szParty1, pNetMsg2->szParty2 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_ALLIANCE_BATTLE_OVER_ARMISTICE:
		{
			GLMSG::SNET_ALLIANCE_BATTLE_OVER_ARMISTICE *pNetMsg3 = (GLMSG::SNET_ALLIANCE_BATTLE_OVER_ARMISTICE*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("ALLIANCE_BATTLE_OVER_ARMISTICE"),
				pNetMsg3->szParty1, pNetMsg3->szParty2 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_ALLIANCE_BATTLE_OVER_ARMISTICE_RESULT:
		{
			GLMSG::SNET_ALLIANCE_BATTLE_OVER_ARMISTICE_RESULT *pNetMsg4 = (GLMSG::SNET_ALLIANCE_BATTLE_OVER_ARMISTICE_RESULT*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("ALLIANCE_BATTLE_OVER_ARMISTICE_RESULT"),
				pNetMsg4->dwRes1, pNetMsg4->dwRes1 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_ALLIANCE_BATTLE_OVER_DRAW:
		{
			GLMSG::SNET_ALLIANCE_BATTLE_OVER_DRAW *pNetMsg5 = (GLMSG::SNET_ALLIANCE_BATTLE_OVER_DRAW*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("ALLIANCE_BATTLE_OVER_DRAW"),
				pNetMsg5->szParty1, pNetMsg5->szParty2 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_ALLIANCE_BATTLE_OVER_SUBMISSION:
		{
			GLMSG::SNET_ALLIANCE_BATTLE_OVER_SUBMISSION *pNetMsg6 = (GLMSG::SNET_ALLIANCE_BATTLE_OVER_SUBMISSION*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("ALLIANCE_BATTLE_OVER_SUBMISSION"),
				pNetMsg6->szParty1, pNetMsg6->szParty2 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_ALLIANCE_BATTLE_OVER_WIN:
		{
			GLMSG::SNET_ALLIANCE_BATTLE_OVER_WIN *pNetMsg7 = (GLMSG::SNET_ALLIANCE_BATTLE_OVER_WIN*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("ALLIANCE_BATTLE_OVER_WIN"),
				pNetMsg7->szParty1, pNetMsg7->szParty2 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_ALLIANCE_BATTLE_RESULT:
		{
			GLMSG::SNET_ALLIANCE_BATTLE_RESULT *pNetMsg8 = (GLMSG::SNET_ALLIANCE_BATTLE_RESULT*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("ALLIANCE_BATTLE_RESULT"),
				pNetMsg8->szString1, pNetMsg8->dwWinner, pNetMsg8->dwLose, pNetMsg8->dwDraw );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_CLUB_BATTLE_BEGIN:
		{
			GLMSG::SNET_CLUB_BATTLE_BEGIN *pNetMsg9 = (GLMSG::SNET_CLUB_BATTLE_BEGIN*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("CLUB_BATTLE_BEGIN"),
				pNetMsg9->szGuild1, pNetMsg9->szGuild2 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_CLUB_BATTLE_BEGIN_REFUSE:
		{
			GLMSG::SNET_CLUB_BATTLE_BEGIN_REFUSE *pNetMsg10 = (GLMSG::SNET_CLUB_BATTLE_BEGIN_REFUSE*) nmg;

			strTEXT.Format ( ID2GAMEINTEXT("CLUB_BATTLE_BEGIN_REFUSE"),
				pNetMsg10->szGuild1, pNetMsg10->szGuild2 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_CLUB_BATTLE_OVER_ARMISTICE:
		{
			GLMSG::SNET_CLUB_BATTLE_OVER_ARMISTICE *pNetMsg11 = (GLMSG::SNET_CLUB_BATTLE_OVER_ARMISTICE*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("CLUB_BATTLE_OVER_ARMISTICE"),
				pNetMsg11->szGuild1, pNetMsg11->szGuild2 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_CLUB_BATTLE_OVER_ARMISTICE_RESULT:
		{
			GLMSG::SNET_CLUB_BATTLE_OVER_ARMISTICE_RESULT *pNetMsg12 = (GLMSG::SNET_CLUB_BATTLE_OVER_ARMISTICE_RESULT*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("CLUB_BATTLE_OVER_ARMISTICE_RESULT"),
				pNetMsg12->dwRes1, pNetMsg12->dwRes2 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_CLUB_BATTLE_OVER_SUBMISSION:
		{
			GLMSG::SNET_CLUB_BATTLE_OVER_SUBMISSION *pNetMsg13 = (GLMSG::SNET_CLUB_BATTLE_OVER_SUBMISSION*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("CLUB_BATTLE_OVER_SUBMISSION"),
				pNetMsg13->szGuild1, pNetMsg13->szGuild2 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_CLUB_BATTLE_OVER_WIN:
		{
			GLMSG::SNET_CLUB_BATTLE_OVER_WIN *pNetMsg14 = (GLMSG::SNET_CLUB_BATTLE_OVER_WIN*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("CLUB_BATTLE_OVER_WIN"),
				pNetMsg14->szGuild1, pNetMsg14->szGuild2 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_CLUB_BATTLE_RESULT:
		{
			GLMSG::SNET_CLUB_BATTLE_RESULT *pNetMsg15 = (GLMSG::SNET_CLUB_BATTLE_RESULT*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("CLUB_BATTLE_RESULT"),
				pNetMsg15->szString1, pNetMsg15->dwWin, pNetMsg15->dwLose, pNetMsg15->dwDraw );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_EMCONFRONT_END_CDRAWN:
		{
			GLMSG::SNET_EMCONFRONT_END_CDRAWN *pNetMsg16 = (GLMSG::SNET_EMCONFRONT_END_CDRAWN*) nmg;

			strTEXT.Format ( ID2GAMEINTEXT("EMCONFRONT_END_CDRAWN"),
				pNetMsg16->szMapName, pNetMsg16->szGuild1, pNetMsg16->szGuild2 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_EMCONFRONT_END_CWIN:
		{
			GLMSG::SNET_EMCONFRONT_END_CWIN *pNetMsg17 = (GLMSG::SNET_EMCONFRONT_END_CWIN*) nmg;

			strTEXT.Format ( ID2GAMEINTEXT("EMCONFRONT_END_CWIN"),
				pNetMsg17->szMapName, pNetMsg17->szGuild1, pNetMsg17->szGuild2, pNetMsg17->szWinner );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_EMCONFRONT_END_PDRAWN:
		{
			GLMSG::SNET_EMCONFRONT_END_PDRAWN *pNetMsg18 = (GLMSG::SNET_EMCONFRONT_END_PDRAWN*) nmg;

			strTEXT.Format ( ID2GAMEINTEXT("EMCONFRONT_END_PDRAWN"),
				pNetMsg18->szMapName, pNetMsg18->szGuild1, pNetMsg18->szGuild2 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_EMCONFRONT_END_PWIN:
		{
			GLMSG::SNET_EMCONFRONT_END_PWIN *pNetMsg19 = (GLMSG::SNET_EMCONFRONT_END_PWIN*) nmg;

			strTEXT.Format ( ID2GAMEINTEXT("EMCONFRONT_END_PWIN"),
				pNetMsg19->szMapName, pNetMsg19->szGuild1, pNetMsg19->szGuild2, pNetMsg19->szWinner );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_EMCONFRONT_START_PARTY:
		{
			GLMSG::SNET_EMCONFRONT_START_PARTY *pNetMsg20 = (GLMSG::SNET_EMCONFRONT_START_PARTY*) nmg;

			strTEXT.Format ( ID2GAMEINTEXT("EMCONFRONT_START_PARTY"),
				pNetMsg20->szMapName, pNetMsg20->szGuild1, pNetMsg20->szGuild2 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;
	case NET_MSG_EMCROWACT_KNOCK:
		{
			GLMSG::SNET_EMCROWACT_KNOCK *pNetMsg21 = (GLMSG::SNET_EMCROWACT_KNOCK*) nmg;

			const char *szMAP_NAME = GLGaeaClient::GetInstance().GetMapName ( pNetMsg21->nidMapID );
			PCROWDATA m_pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( pNetMsg21->nidBossID );

			if( m_pCrowData && szMAP_NAME ){
				strTEXT.Format ( ID2GAMEINTEXT("EMCROWACT_KNOCK"),
					m_pCrowData->GetName(), szMAP_NAME );
				CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT.GetString() );
			}
		}
		break;
	case NET_MSG_EMCROWACT_REPULSE:
		{
			GLMSG::SNET_EMCROWACT_REPULSE *pNetMsg22 = (GLMSG::SNET_EMCROWACT_REPULSE*) nmg;

			const char *szMAP_NAME = GLGaeaClient::GetInstance().GetMapName ( pNetMsg22->nidMapID );
			PCROWDATA m_pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( pNetMsg22->nidBossID );

			if( m_pCrowData && szMAP_NAME ){
				strTEXT.Format ( ID2GAMEINTEXT("EMCROWACT_REPULSE"),
					m_pCrowData->GetName(), pNetMsg22->szKiller, szMAP_NAME );
				CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
			}	
		}
		break;
	case NET_MSG_EMGUIDCLUB_CERTIFIED:
		{
			GLMSG::SNET_EMGUIDCLUB_CERTIFIED *pNetMsg23 = (GLMSG::SNET_EMGUIDCLUB_CERTIFIED*) nmg;

			strTEXT.Format ( ID2GAMEINTEXT("EMGUIDCLUB_CERTIFIED"),
				pNetMsg23->szGuildName, pNetMsg23->szMapName );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;

	case NET_MSG_BRIGHT_EVENT_MSG:
		{
			GLMSG::SNET_BRIGHT_EVENT_MSG *pNetMsg25 = (GLMSG::SNET_BRIGHT_EVENT_MSG*) nmg;

			strTEXT.Format ( ID2GAMEINTEXT("BRIGHT_EVENT_MSG"));
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;

	case NET_MSG_CLUB_BATTLE_OVER_DRAW:
		{
			GLMSG::SNET_CLUB_BATTLE_OVER_DRAW *pNetMsg26 = (GLMSG::SNET_CLUB_BATTLE_OVER_DRAW*) nmg;

			strTEXT.Format ( ID2SERVERTEXT("CLUB_BATTLE_OVER_DRAW"),
				pNetMsg26->szGuild1, pNetMsg26->szGuild2 );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_CTRL_GLOBAL, "", strTEXT );
		}
		break;

		/*event map move, Juver, 2017/08/25 */
	case NET_MSG_GCTRL_REQ_EVENT_MOVEMAP_FB:
		{
			GLMSG::SNETPC_REQ_EVENT_MOVEMAP_FB *pnetmsg = (GLMSG::SNETPC_REQ_EVENT_MOVEMAP_FB *) nmg;
			switch( pnetmsg->emFB )
			{
			case EMEVENT_MOVEMAP_FB_FAIL:
				CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMEVENT_MOVEMAP_FB_FAIL") );
				break;
			case EMEVENT_MOVEMAP_FB_CONDITION:
				CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMEVENT_MOVEMAP_FB_CONDITION") );
				break;
			case EMEVENT_MOVEMAP_FB_OK:
				CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("EMEVENT_MOVEMAP_FB_OK") );
				break;
			};
		}break;

	//	pc client����.
	//
	case NET_MSG_GCTRL_MOVESTATE_BRD:
	case NET_MSG_GCTRL_JUMP_POS_BRD:

	case NET_MSG_GCTRL_GOTO_BRD:
	case NET_MSG_GCTRL_ATTACK_BRD:
	case NET_MSG_GCTRL_ATTACK_CANCEL_BRD:
	case NET_MSG_GCTRL_ATTACK_AVOID_BRD:
	case NET_MSG_GCTRL_ATTACK_DAMAGE_BRD:
	case NET_MSG_GCTRL_PUTON_RELEASE_BRD:
	case NET_MSG_GCTRL_PUTON_UPDATE_BRD:
	case NET_MSG_GCTRL_PUTON_CHANGE_BRD:
	case NET_MSG_GCTRL_REQ_LEVELUP_BRD:
	case NET_MSG_GCTRL_CONFRONT_END2_CLT_BRD:

	case NET_MSG_GCTRL_UPDATE_STATE_BRD:
	case NET_MSG_GCTRL_PARTY_BRD:

	case NET_MSG_GCTRL_UPDATE_PASSIVE_BRD:
	case NET_MSG_GCTRL_UPDATE_FLAGS:

	case NET_MSG_GCTRL_REQ_GESTURE_BRD:

	case NET_MSG_GCTRL_PMARKET_OPEN_BRD:
	case NET_MSG_GCTRL_PMARKET_CLOSE_BRD:
	case NET_MSG_GCTRL_PMARKET_ITEM_INFO_BRD:
	case NET_MSG_GCTRL_PMARKET_ITEM_UPDATE_BRD:
	case NET_MSG_GCTRL_CLUB_INFO_BRD:
	case NET_MSG_GCTRL_CLUB_INFO_MARK_BRD:
	case NET_MSG_GCTRL_CLUB_INFO_NICK_BRD:
	case NET_MSG_GCTRL_CLUB_DEL_BRD:
	case NET_MSG_GCTRL_CLUB_AUTHORITY_BRD:
	case NET_MSG_GCTRL_UPDATE_BRIGHT_BRD:

	case NET_MSG_GCTRL_INVEN_HAIR_CHANGE_BRD:
	case NET_MSG_GCTRL_INVEN_HAIRCOLOR_CHANGE_BRD:
	case NET_MSG_GCTRL_INVEN_FACE_CHANGE_BRD:
	case NET_MSG_GCTRL_INVEN_RENAME_BRD:

	case NET_MSG_GCTRL_QITEMFACT_BRD:
	case NET_MSG_GCTRL_QITEMFACT_END_BRD:

	case NET_MSG_GCTRL_PKCOMBO_BRD:
	case NET_MSG_GCTRL_PKCOMBO_END_BRD:

	case NET_MSG_GCTRL_EVENTFACT_BRD:
	case NET_MSG_GCTRL_EVENTFACT_END_BRD:

	case NET_MSG_GCTRL_ACTIVE_VEHICLE_BRD:
	case NET_MSG_GCTRL_GET_VEHICLE_BRD:
	case NET_MSG_GCTRL_UNGET_VEHICLE_BRD:
	case NET_MSG_VEHICLE_ACCESSORY_DELETE_BRD:
	case NET_MSG_VEHICLE_REQ_SLOT_EX_HOLD_BRD:
	case NET_MSG_VEHICLE_REMOVE_SLOTITEM_BRD:

	case NET_MSG_GCTRL_ITEMSHOPOPEN_BRD:
	
	case NET_MSG_GCTRL_SKILL_CANCEL_BRD:
	case NET_MSG_REQ_GATHERING_BRD:
	case NET_MSG_REQ_GATHERING_RESULT_BRD:
	case NET_MSG_REQ_GATHERING_CANCEL_BRD:

	case NET_MSG_GCTRL_LANDEFFECT:

		/*itemfood system, Juver, 2017/05/25 */
	case NET_MSG_GCTRL_FITEMFACT_BRD:

		/*vehicle booster system, Juver, 2017/08/12 */
	case NET_MSG_GCTRL_VEHICLE_BOOSTER_STATE_START_BRD:
	case NET_MSG_GCTRL_VEHICLE_BOOSTER_STATE_RESET_BRD:

		/*system buffs, Juver, 2017/09/05 */
	case NET_MSG_GCTRL_SYSTEM_BUFF_BRD:

		/*activity system, Juver, 2017/11/01 */
	case NET_MSG_GCTRL_ACTIVITY_COMPLETE_BRD:
	case NET_MSG_GCTRL_CHARACTER_BADGE_CHANGE_BRD:

	/*Codex System, Jhoniex 2/14/2024 */
	case NET_MSG_GCTRL_CODEX_COMPLETE_BRD:

		/*bike color , Juver, 2017/11/13 */
	case NET_MSG_VEHICLE_REQ_CHANGE_COLOR_BRD:

		/*change scale card, Juver, 2018/01/04 */
	case NET_MSG_GCTRL_INVEN_SCALE_CHANGE_BRD:

		/*item color, Juver, 2018/01/10 */
	case NET_MSG_GCTRL_INVEN_ITEMCOLOR_CHANGE_BRD:

		/*pvp capture the flag, Juver, 2018/01/30 */
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_PLAYER_TEAM_BRD:
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_FLAG_HOLD_BRD:

		/* car, cart color, Juver, 2018/02/14 */
	case NET_MSG_VEHICLE_REQ_CHANGE_CAR_COLOR_BRD:
		/*dmk14 remove active buff selection*/
	case NET_MSG_GCTRL_REQ_DISABLESKILLEFF_BRD:
	case NET_MSG_GCTRL_REQ_DISABLEITEMFACT_BRD:
	case NET_MSG_GCTRL_REMOVE_SKILL_EFFECT_BRD:
		
		{
			GLMSG::SNETPC_BROAD *pNetMsg = (GLMSG::SNETPC_BROAD *) nmg;
			
			if ( pNetMsg->dwGaeaID==m_Character.m_dwGaeaID )
			{
				m_Character.MsgProcess ( nmg );
			}
			else
			{
				PGLCHARCLIENT pChar = m_pLandMClient->GetChar ( pNetMsg->dwGaeaID );
				if ( !pChar )
				{
					CDebugSet::ToListView ( "[PGLCHARCLIENT NULL] ���� PC�� ���� �޼��� �߻�. gaeaid %d", pNetMsg->dwGaeaID );
					return;
				}

				pChar->MsgProcess ( nmg );
			}
		}
		break;

	case NET_MSG_GCTRL_UPDATE_STATE:
	case NET_MSG_GCTRL_UPDATE_EXP:
	case NET_MSG_GCTRL_UPDATE_MONEY:
	case NET_MSG_GCTRL_UPDATE_SP:
	case NET_MSG_GCTRL_UPDATE_LP:
	case NET_MSG_GCTRL_UPDATE_SKP:
	case NET_MSG_GCTRL_UPDATE_BRIGHT:
	case NET_MSG_GCTRL_UPDATE_STATS:

	case NET_MSG_GCTRL_PICKUP_MONEY:
	case NET_MSG_GCTRL_PICKUP_ITEM:

	case NET_MSG_GCTRL_REQ_HOLD_FB:

	case NET_MSG_GCTRL_INVEN_INSERT:
	case NET_MSG_GCTRL_REQ_VNINVEN_TO_INVEN_FB:
	case NET_MSG_GCTRL_INVEN_DELETE:
	case NET_MSG_GCTRL_INVEN_DEL_INSERT:

	case NET_MSG_GCTRL_ITEM_COOLTIME_UPDATE:
	case NET_MSG_GCTRL_ITEM_COOLTIME_ERROR:

	case NET_MSG_GCTRL_PUTON_RELEASE:
	case NET_MSG_GCTRL_PUTON_UPDATE:
	case NET_MSG_GCTRL_PUTON_CHANGE:

	case NET_MSG_GCTRL_REQ_SKILLQ_FB:
	case NET_MSG_GCTRL_REQ_ACTIONQ_FB:
	
	case NET_MSG_GCTRL_ATTACK_AVOID:
	case NET_MSG_GCTRL_ATTACK_DAMAGE:
	case NET_MSG_GCTRL_DEFENSE_SKILL_ACTIVE:

	case NET_MSG_GCTRL_SUMMON_ATTACK_AVOID:
	case NET_MSG_GCTRL_SUMMON_ATTACK_DAMAGE:

	case NET_MSG_GCTRL_REQ_LEVELUP_FB:
	case NET_MSG_GCTRL_REQ_STATSUP_FB:
	case NET_MSG_GCTRL_REQ_STATSUPCMD_FB:
	case NET_MSG_GCTRL_REQ_LEARNSKILL_FB:
	case NET_MSG_GCTRL_REQ_SKILLUP_FB:

	case NET_MSG_GCTRL_INVEN_DRUG_UPDATE:
	case NET_MSG_GCTRL_PUTON_DRUG_UPDATE:

	case NET_MSG_GCTRL_REQ_SKILL_FB:
	case NET_MSG_REQ_SKILL_REVIVEL_FAILED:
	case NET_MSG_GCTRL_SKILLCONSUME_FB:

	case NET_QBOX_OPTION_MEMBER:

	case NET_MSG_GCTRL_REQ_GETSTORAGE_FB:
	case NET_MSG_GCTRL_REQ_GETSTORAGE_ITEM:

	case NET_MSG_GCTRL_STORAGE_INSERT:
	case NET_MSG_GCTRL_STORAGE_DELETE:
	case NET_MSG_GCTRL_STORAGE_ITEM_UPDATE:

	case NET_MSG_GCTRL_STORAGE_DEL_INSERT:
	case NET_MSG_GCTRL_STORAGE_DRUG_UPDATE:

	case NET_MSG_GCTRL_STORAGE_UPDATE_MONEY:

	case NET_MSG_GCTRL_TRADE_FB:
	case NET_MSG_GCTRL_TRADE_AGREE_TAR:
	case NET_MSG_GCTRL_TRADE_LOCK_TAR: /*trade lock, Juver, 2018/01/02 */
	case NET_MSG_GCTRL_TRADE_ITEM_REGIST_TAR:
	case NET_MSG_GCTRL_TRADE_ITEM_REMOVE_TAR:
	case NET_MSG_GCTRL_TRADE_MONEY_TAR:
	case NET_MSG_GCTRL_TRADE_COMPLETE_TAR:
	case NET_MSG_GCTRL_TRADE_CANCEL_TAR:


	case NET_MSG_GCTRL_INVEN_ITEM_UPDATE:
	case NET_MSG_GCTRL_INVEN_GRINDING_FB:
	case NET_MSG_GCTRL_INVEN_BOXOPEN_FB:
	case NET_MSG_GCTRL_INVEN_DISGUISE_FB:
	case NET_MSG_GCTRL_INVEN_CLEANSER_FB:
	case NET_MSG_GCTRL_INVEN_DEL_ITEM_TIMELMT:
	case NET_MSG_GCTRL_CHARGED_ITEM_GET_FB:
	case NET_MSG_GCTRL_CHARGED_ITEM_DEL:
	case NET_MSG_GCTRL_INVEN_RESET_SKST_FB:
	case NET_MSG_GCTRL_GET_CHARGEDITEM_FROMDB_FB:

	case NET_MSG_GCTRL_INVEN_HAIR_CHANGE_FB:
	case NET_MSG_GCTRL_INVEN_HAIRCOLOR_CHANGE_FB:
	case NET_MSG_GCTRL_INVEN_FACE_CHANGE_FB:
	case NET_MSG_GCTRL_INVEN_GENDER_CHANGE_FB:
	case NET_MSG_GCTRL_INVEN_RENAME_FB:	

	case NET_MSG_GCTRL_REGEN_GATE_FB:

	case NET_MSG_GCTRL_CARD_BRIGHT_FB:
	//CNDev | 2-7-2020 | e-point card system
	case NET_MSG_GCTRL_CARD_EPOINT_FB:

	case NET_MSG_GCTRL_CARD_CONTRIBUTION_FB:

	case NET_MSG_GCTRL_CONFRONT_FB:
	case NET_MSG_GCTRL_CONFRONT_START2_CLT:
	case NET_MSG_GCTRL_CONFRONT_FIGHT2_CLT:
	case NET_MSG_GCTRL_CONFRONT_END2_CLT:

	case NET_MSG_GCTRL_CONFRONTPTY_START2_CLT:
	case NET_MSG_GCTRL_CONFRONTPTY_END2_CLT:

	case NET_MSG_GCTRL_CONFRONTCLB_START2_CLT:
	case NET_MSG_GCTRL_CONFRONTCLB_END2_CLT:

	case NET_MSG_GCTRL_CONFRONT_RECOVE:
	case NET_MSG_GCTRL_CURE_FB:

	case NET_MSG_GCTRL_CHARRESET_FB:
	case NET_MSG_GCTRL_INVEN_CHARCARD_FB:
	case NET_MSG_GCTRL_INVEN_STORAGECARD_FB:
	case NET_MSG_GCTRL_INVEN_STORAGEOPEN_FB:
	case NET_MSG_GCTRL_INVEN_REMODELOPEN_FB:
	
	case NET_MSG_GCTRL_INVEN_GARBAGEOPEN_FB:
	case NET_MSG_GCTRL_GARBAGE_RESULT_FB:
	
	case NET_MSG_GCTRL_INVEN_PREMIUMSET_FB:
	case NET_MSG_CHAT_LOUDSPEAKER_FB:
	case NET_MSG_GCTRL_INVEN_INVENLINE_FB:
	case NET_MSG_GCTRL_INVEN_RANDOMBOXOPEN_FB:
	case NET_MSG_GCTRL_INVEN_DISJUNCTION_FB:
	
	case NET_MSG_GCTRL_PREMIUM_STATE:
	case NET_MSG_GCTRL_STORAGE_STATE:
	case NET_MSG_GCTRL_REVIVE_FB:
	case NET_MSG_GCTRL_GETEXP_RECOVERY_FB:
	case NET_MSG_GCTRL_GETEXP_RECOVERY_NPC_FB:
	case NET_MSG_GCTRL_RECOVERY_FB:
	case NET_MSG_GCTRL_RECOVERY_NPC_FB:

	case NET_MSG_GCTRL_FIRECRACKER_FB:
	case NET_MSG_GCTRL_FIRECRACKER_BRD:

	case NET_MSG_GM_MOVE2GATE_FB:
	case NET_MSG_GCTRL_2_FRIEND_FB:
	case NET_MSG_GM_SHOWMETHEMONEY:


	case NET_MSG_GCTRL_NPC_ITEM_TRADE_FB:

	case NET_MSG_GCTRL_REQ_QUEST_START_FB:

	case NET_MSG_GCTRL_QUEST_PROG_STREAM:
	case NET_MSG_GCTRL_QUEST_PROG_DEL:

	case NET_MSG_GCTRL_QUEST_PROG_STEP_STREAM:
	case NET_MSG_GCTRL_QUEST_PROG_INVEN:

	case NET_MSG_GCTRL_QUEST_END_STREAM:
	case NET_MSG_GCTRL_QUEST_END_DEL:

	case NET_MSG_GCTRL_QUEST_PROG_NPCTALK_FB:
	case NET_MSG_GCTRL_QUEST_PROG_MOBKILL:
	case NET_MSG_GCTRL_QUEST_PARTY_PROG_MOBKILL:
	case NET_MSG_GCTRL_QUEST_PROG_QITEM:
	case NET_MSG_GCTRL_QUEST_PARTY_PROG_QITEM:
	case NET_MSG_GCTRL_QUEST_PROG_REACHZONE:
	case NET_MSG_GCTRL_QUEST_PROG_TIMEOVER:
	case NET_MSG_GCTRL_QUEST_PROG_TIME:
	case NET_MSG_GCTRL_QUEST_PROG_NONDIE:
	case NET_MSG_GCTRL_QUEST_PROG_LEAVEMAP:

	case NET_MSG_GCTRL_QUEST_PROG_INVEN_INSERT:
	case NET_MSG_GCTRL_QUEST_PROG_INVEN_DELETE:
	case NET_MSG_GCTRL_QUEST_PROG_INVEN_TURN:
	case NET_MSG_GCTRL_QUEST_PROG_INVEN_PICKUP:
	case NET_MSG_GCTRL_QUEST_COMPLETE_FB:

	case NET_MSG_GCTRL_PMARKET_TITLE_FB:
	case NET_MSG_GCTRL_PMARKET_REGITEM_FB:
	case NET_MSG_GCTRL_PMARKET_DISITEM_FB:
	case NET_MSG_GCTRL_PMARKET_OPEN_FB:
	case NET_MSG_GCTRL_PMARKET_BUY_FB:

	case NET_MSG_GCTRL_CLUB_INFO_2CLT:
	case NET_MSG_GCTRL_CLUB_DEL_2CLT:
	case NET_MSG_GCTRL_CLUB_INFO_DISSOLUTION:
	case NET_MSG_GCTRL_CLUB_MEMBER_2CLT:
	case NET_MSG_GCTRL_CLUB_NEW_FB:
	case NET_MSG_GCTRL_CLUB_DISSOLUTION_FB:
	case NET_MSG_GCTRL_CLUB_MEMBER_REQ_ASK:
	case NET_MSG_GCTRL_CLUB_MEMBER_REQ_FB:
	case NET_MSG_GCTRL_CLUB_MEMBER_DEL_2CLT:
	case NET_MSG_GCTRL_CLUB_MEMBER_DEL_FB:
	case NET_MSG_GCTRL_CLUB_MEMBER_SECEDE_FB:
	case NET_MSG_GCTRL_CLUB_AUTHORITY_REQ_ASK:
	case NET_MSG_GCTRL_CLUB_AUTHORITY_REQ_FB:
	case NET_MSG_GCTRL_CLUB_AUTHORITY_CLT:

	case NET_MSG_GCTRL_CLUB_MARK_CHANGE_2CLT:
	case NET_MSG_GCTRL_CLUB_RANK_2CLT:
	case NET_MSG_GCTRL_CLUB_RANK_FB:
	case NET_MSG_GCTRL_CLUB_MEMBER_NICK_FB:
	case NET_MSG_GCTRL_CLUB_MEMBER_STATE:
	case NET_MSG_GCTRL_CLUB_MEMBER_POS:
	case NET_MSG_GCTRL_CONFRONT_END2_CLT_MBR:

	case NET_MSG_GCTRL_CLUB_CD_CERTIFY_FB:
	case NET_MSG_GCTRL_CLUB_COMMISSION_FB:

	case NET_MSG_GCTRL_CLUB_STORAGE_RESET:
	case NET_MSG_GCTRL_CLUB_GETSTORAGE_ITEM:

	case NET_MSG_GCTRL_CLUB_STORAGE_INSERT:
	case NET_MSG_GCTRL_CLUB_STORAGE_DELETE:
	case NET_MSG_GCTRL_CLUB_STORAGE_DEL_INS:
	case NET_MSG_GCTRL_CLUB_STORAGE_UPDATE_ITEM:
	case NET_MSG_GCTRL_CLUB_STORAGE_UPDATE_MONEY:

	case NET_MSG_GCTRL_CLUB_NOTICE_FB:
	case NET_MSG_GCTRL_CLUB_NOTICE_CLT:

	case NET_MSG_GCTRL_CLUB_SUBMASTER_FB:
	case NET_MSG_GCTRL_CLUB_SUBMASTER_BRD:
	case NET_MSG_GCTRL_CLUB_ALLIANCE_REQ_ASK:
	case NET_MSG_GCTRL_CLUB_ALLIANCE_ADD_CLT:
	case NET_MSG_GCTRL_CLUB_ALLIANCE_DEL_CLT:
	case NET_MSG_GCTRL_CLUB_ALLIANCE_DIS_CLT:

	case NET_MSG_GCTRL_CLUB_ALLIANCE_REQ_FB:
	case NET_MSG_GCTRL_CLUB_ALLIANCE_DEL_FB:
	case NET_MSG_GCTRL_CLUB_ALLIANCE_SEC_FB:
	case NET_MSG_GCTRL_CLUB_ALLIANCE_DIS_FB:

	case NET_MSG_GCTRL_CLUB_BATTLE_REQ_FB:
	case NET_MSG_GCTRL_CLUB_BATTLE_REQ_ASK:
	case NET_MSG_GCTRL_CLUB_BATTLE_BEGIN_CLT:
	case NET_MSG_GCTRL_CLUB_BATTLE_BEGIN_CLT2:
	case NET_MSG_GCTRL_CLUB_BATTLE_ARMISTICE_REQ_FB:
	case NET_MSG_GCTRL_CLUB_BATTLE_ARMISTICE_REQ_ASK:
	case NET_MSG_GCTRL_CLUB_BATTLE_OVER_CLT:
	case NET_MSG_GCTRL_CLUB_BATTLE_SUBMISSION_REQ_FB:
	case NET_MSG_GCTRL_CLUB_BATTLE_KILL_UPDATE:
	case NET_MSG_GCTRL_CLUB_BATTLE_POINT_UPDATE:
	
	case NET_MSG_GCTRL_ALLIANCE_BATTLE_REQ_FB:
	case NET_MSG_GCTRL_ALLIANCE_BATTLE_REQ_ASK:
	case NET_MSG_GCTRL_ALLIANCE_BATTLE_ARMISTICE_REQ_FB:
	case NET_MSG_GCTRL_ALLIANCE_BATTLE_ARMISTICE_REQ_ASK:
	case NET_MSG_GCTRL_ALLIANCE_BATTLE_SUBMISSION_REQ_FB:

	case NET_MSG_GCTRL_PLAYERKILLING_ADD:
	case NET_MSG_GCTRL_PLAYERKILLING_DEL:

	case NET_MSG_GCTRL_CLUB_MBR_RENAME_CLT:

	case NET_MSG_REBUILD_RESULT:	// ITEMREBUILD_MARK
	case NET_MSG_REBUILD_MOVE_ITEM:
		/*dmk14 reform*/
	case NET_MSG_REBUILD_MOVE_ITEMSEALED:
	case NET_MSG_REBUILD_COST_MONEY:
	case NET_MSG_REBUILD_INPUT_MONEY:

	case NET_MSG_GCTRL_UPDATE_LASTCALL:
	case NET_MSG_GCTRL_UPDATE_STARTCALL:

	case NET_MSG_SMS_PHONE_NUMBER_FB:
	case NET_MSG_SMS_SEND_FB:

	case NET_MSG_MGAME_ODDEVEN_FB:		// �̴ϰ��� - Ȧ¦
	case NET_MSG_CHINA_GAINTYPE:

	case NET_MSG_GM_LIMIT_EVENT_APPLY_START:
	case NET_MSG_GM_LIMIT_EVENT_APPLY_END:

	case NET_MSG_GM_LIMIT_EVENT_BEGIN_FB:
	case NET_MSG_GM_LIMIT_EVENT_END_FB:

	case NET_MSG_GCTRL_PMARKET_SEARCH_ITEM_RESULT:

	/// ��Ʈ�� �޽���
	case NET_MSG_VIETNAM_TIME_REQ_FB:
	case NET_MSG_VIETNAM_GAIN_EXP:
	case NET_MSG_VIETNAM_GAINTYPE:
	case NET_MSG_VIETNAM_GAIN_MONEY:
	case NET_MSG_GCTRL_INVEN_VIETNAM_ITEMGETNUM_UPDATE:
	case NET_MSG_GCTRL_INVEN_VIETNAM_EXPGET_FB:
	case NET_MSG_GCTRL_INVEN_VIETNAM_ITEMGET_FB:
	case NET_MSG_VIETNAM_ALLINITTIME:

	case NET_MSG_GCTRL_EVENTFACT_INFO:

	case NET_MSG_GM_LIMIT_EVENT_TIME_REQ_FB:
	case NET_MSG_GCTRL_ACTIVE_VEHICLE_FB:
	case NET_MSG_GCTRL_GET_VEHICLE_FB:
	case NET_MSG_GCTRL_UNGET_VEHICLE_FB:
	case NET_MSG_VEHICLE_REQ_SLOT_EX_HOLD_FB:
	case NET_MSG_VEHICLE_REQ_HOLD_TO_SLOT_FB:
	case NET_MSG_VEHICLE_REQ_SLOT_TO_HOLD_FB:
	case NET_MSG_VEHICLE_REMOVE_SLOTITEM_FB:
	case NET_MSG_VEHICLE_ACCESSORY_DELETE:
	case NET_MSG_VEHICLE_UPDATE_CLIENT_BATTERY:
	case NET_MSG_VEHICLE_REQ_GIVE_BATTERY_FB:
	case NET_MSG_VEHICLE_REQ_ITEM_INFO_FB:
	case NET_MSG_UPDATE_TRACING_CHARACTER:
	case NET_MSG_REQ_ATTENDLIST_FB:
	case NET_MSG_REQ_ATTENDANCE_FB:
	case NET_MSG_REQ_ATTEND_REWARD_CLT:
	case NET_MSG_GCTRL_NPC_RECALL_FB:
	case NET_MSG_GCTRL_INVEN_ITEM_MIX_FB:
	case NET_MSG_REQ_GATHERING_FB:
	case NET_MSG_REQ_GATHERING_RESULT:
		/*itemfood system, Juver, 2017/05/25 */
	case NET_MSG_GCTRL_INVEN_CONSUME_FOOD_FB: 
	case NET_MSG_GCTRL_INVEN_UNLOCK_FOOD_FB: 

		/*combatpoint logic, Juver, 2017/05/29 */
	case NET_MSG_GCTRL_UPDATE_CP: 

		/*game stats, Juver, 2017/06/21 */
	case NET_MSG_GCTRL_PING_PACKET_FB: 

		/*npc shop, Juver, 2017/07/27 */
	case NET_MSG_GCTRL_NPCSHOP_PURCHASE_MONEY_FB: 

		 /*vehicle booster system, Juver, 2017/08/10 */
	case NET_MSG_VEHICLE_REQ_ENABLE_BOOSTER_FB:
	case NET_MSG_GCTRL_VEHICLE_BOOSTER_STATE_CHARGE:
	case NET_MSG_GCTRL_VEHICLE_BOOSTER_STATE_START:
	case NET_MSG_GCTRL_VEHICLE_BOOSTER_STATE_RESET:

		/*contribution point, Juver, 2017/08/23 */
	case NET_MSG_GCTRL_UPDATE_CONTRIBUTION_POINT: 

		/*activity point, Juver, 2017/08/23 */
	case NET_MSG_GCTRL_UPDATE_ACTIVITY_POINT:

		/*item exchange, Juver, 2017/10/13 */
	case NET_MSG_GCTRL_NPC_ITEM_EXCHANGE_TRADE_FB:

		/*product item, Juver, 2017/10/18 */
	case NET_MSG_GCTRL_ITEM_COMPOUND_START_FB:
	case NET_MSG_GCTRL_ITEM_COMPOUND_PROCESS_FB:

		/*activity system, Juver, 2017/10/30 */
	case NET_MSG_GCTRL_ACTIVITY_UPDATE:
	case NET_MSG_GCTRL_ACTIVITY_COMPLETE:
	case NET_MSG_GCTRL_ACTIVITY_NOTIFY_CLIENT:

	/*Codex System, Jhoniex 2/14/2024 */
	case NET_MSG_GCTRL_CODEX_UPDATE:
	case NET_MSG_GCTRL_CODEX_COMPLETE:
	case NET_MSG_GCTRL_CODEX_NOTIFY_CLIENT:

		/*activity system, Juver, 2017/11/05 */
	case NET_MSG_GCTRL_CHARACTER_BADGE_CHANGE_FB:
	/*PartySearch - Added By Bubu*/
	case NET_MSG_GCTRL_GET_PARTYLIST_FB:

		/*charinfoview , Juver, 2017/11/11 */
	case NET_MSG_GCTRL_REQ_CHARINFO_FB:

		/*bike color , Juver, 2017/11/13 */
	case NET_MSG_VEHICLE_REQ_CHANGE_COLOR_FB:

		/*pk info, Juver, 2017/11/17 */
	case NET_MSG_GCTRL_UPDATE_PK_SCORE:
	case NET_MSG_GCTRL_UPDATE_PK_DEATH:

		/*rv card, Juver, 2017/11/25 */
	case NET_MSG_GCTRL_INVEN_RANDOM_OPTION_CHANGE_FB:

		/*nondrop card, Juver, 2017/11/26 */
	case NET_MSG_GCTRL_INVEN_NONDROP_CARD_FB:

		/*change scale card, Juver, 2018/01/04 */
	case NET_MSG_GCTRL_INVEN_SCALE_CHANGE_FB:

		/*item color, Juver, 2018/01/10 */
	case NET_MSG_GCTRL_INVEN_ITEMCOLOR_CHANGE_FB:

		/*item wrapper, Juver, 2018/01/12 */
	case NET_MSG_GCTRL_INVEN_WRAP_FB:
	case NET_MSG_GCTRL_INVEN_UNWRAP_FB:

		/*change school card, Juver, 2018/01/12 */
	case NET_MSG_GCTRL_INVEN_CHANGE_SCHOOL_FB:

		/*equipment lock, Juver, 2018/01/14 */
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_ENABLE_FB:
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_INPUT_FB:
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_FB:
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE_FB:
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE_FB:

		/*storage lock, Juver, 2018/01/14 */
	case NET_MSG_GCTRL_STORAGE_LOCK_ENABLE_FB:
	case NET_MSG_GCTRL_STORAGE_LOCK_INPUT_FB:
	case NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_FB:
	case NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_CHANGE_FB:
	case NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_DELETE_FB:

		/*inventory lock, Juver, 2018/01/14 */
	case NET_MSG_GCTRL_INVENTORY_LOCK_ENABLE_FB:
	case NET_MSG_GCTRL_INVENTORY_LOCK_INPUT_FB:
	case NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_FB:
	case NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_CHANGE_FB:
	case NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_DELETE_FB:

		/*item transfer card, Juver, 2018/01/18 */
	case NET_MSG_GCTRL_INVEN_ITEM_TRANSFER_FB:

	case NET_MSG_GCTRL_MYSTERY_BOX_FB: //Mystery Box
		/* car, cart color, Juver, 2018/02/14 */
	case NET_MSG_VEHICLE_REQ_CHANGE_CAR_COLOR_FB:

		/* booster all vehicle, Juver, 2018/02/14 */
	case NET_MSG_ALLVEHICLE_REQ_ENABLE_BOOSTER_FB:
	
	case NET_MSG_GCTRL_REQ_LEARNSKILL_NONINVEN_FB:	
	/*dmk14 remove active buff selection*/
	case NET_MSG_GCTRL_REQ_DISABLESKILLEFF_FB:
	case NET_MSG_GCTRL_REQ_DISABLEITEMFACT_FB:
	case NET_MSG_GCTRL_INVEN_GMITEM_FB:
	case NET_MSG_RESET_STATS_FB:
	case NET_MSG_GCTRL_INVEN_REFORM_WEAPON_FB: //Weapon Skill Reform Card - SqueeeAK
	/////////////////////////////////////////////////////////////////////////////
	// LG-7 GlobalRanking
	case NET_MSG_GCTRL_REQ_GLOBAL_RANKING_FB:
	case NET_MSG_GCTRL_REQ_GLOBAL_RANKING_KILL_AGT:
	case NET_MSG_GCTRL_REQ_GLOBAL_RANKING_RICH_AGT:
	case NET_MSG_GCTRL_REQ_GLOBAL_RANKING_GUILD_AGT:
	/////////////////////////////////////////////////////////////////////////////
		{
			m_Character.MsgProcess ( nmg );
		}
		break;

	case NET_MSG_GCTRL_ACTION_BRD:

	case NET_MSG_GCTRL_SKILLFACT_BRD:
	case NET_MSG_GCTRL_SKILLHOLD_BRD:
	case NET_MSG_GCTRL_STATEBLOW_BRD:
	case NET_MSG_GCTRL_CURESTATEBLOW_BRD:

	case NET_MSG_GCTRL_REQ_SKILL_BRD:
	case NET_MSG_GCTRL_SKILLHOLD_RS_BRD:
	case NET_MSG_GCTRL_SKILLHOLDEX_BRD:

	case NET_MSG_GCTRL_PUSHPULL_BRD:
	case NET_MSG_GCTRL_POSITIONCHK_BRD:
	case NET_MSG_GCTRL_TELEPORT_BRD: //Teleport Skill Effect
		{
			GLMSG::SNETCROW_BROAD *pNetMsg = (GLMSG::SNETCROW_BROAD *) nmg;
			GLCOPY* pCopy = GetCopyActor ( pNetMsg->emCrow, pNetMsg->dwID );
			if ( !pCopy )		return;
				
			pCopy->MsgProcess ( nmg );
		}
		break;

	case NET_MSG_GCTRL_CROW_MOVETO:
	case NET_MSG_GCTRL_CROW_ATTACK:
	case NET_MSG_GCTRL_CROW_AVOID:
	case NET_MSG_GCTRL_CROW_DAMAGE:

	case NET_MSG_GCTRL_CROW_SKILL:

		/*pvp tyranny, Juver, 2017/08/24 */
	case NET_MSG_GCTRL_TYRANNY_CROW_OWNER:
	case NET_MSG_GCTRL_TYRANNY_CROW_DAMAGE:

		/*pvp capture the flag, Juver, 2018/02/06 */
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_CROW_HIDE:

		{
			GLMSG::SNETCROW *pNetMsg = (GLMSG::SNETCROW *) nmg;
			PGLCROWCLIENT pGLCrow = m_pLandMClient->GetCrow ( pNetMsg->dwGlobID );
			if ( !pGLCrow )		return;

			pGLCrow->MsgProcess ( nmg );
		}
		break;

	case NET_MSG_GCTRL_ETNRY_FAILED:
		{
			CInnerInterface::GetInstance().GetRequireCheck()->SetMapCheckType( EMMAPCHECK_ENTRYFAIL );
			CInnerInterface::GetInstance().ShowGroupFocus ( MAP_REQUIRE_CHECK );			
		}
		break;

	case NET_MSG_GCTRL_LIMITTIME_OVER:
		{
			GLMSG::SNETLIMITTIME_OVER *pNetMsg = (GLMSG::SNETLIMITTIME_OVER *) nmg;
			if( pNetMsg->nRemainTime != 0 )
			{
				CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("LIMITTIME_REMAINTIME"), pNetMsg->nRemainTime );
			}else{
				CInnerInterface::GetInstance().GetRequireCheck()->SetMapCheckType( EMMAPCHECK_LIMITTIME );
				CInnerInterface::GetInstance().ShowGroupFocus ( MAP_REQUIRE_CHECK );
			}
		}
		break;

	case NET_MSG_CYBERCAFECLASS_UPDATE:
		{
			GLMSG::SNET_CYBERCAFECLASS_UPDATE *pNetMsg = (GLMSG::SNET_CYBERCAFECLASS_UPDATE *)nmg;
			GetCharacter()->m_dwThaiCCafeClass = pNetMsg->dwCyberCafeClass;
			GetCharacter()->m_nMyCCafeClass	   = pNetMsg->dwCyberCafeClass;
			CInnerInterface::GetInstance().SetThaiCCafeClass( pNetMsg->dwCyberCafeClass );
		}
		break;

	case NET_MSG_DROPCHAR_TOCHEATER:
		{
			GLMSG::SNET_DROPCHAR_TOCHEATER *pNetMsg = (GLMSG::SNET_DROPCHAR_TOCHEATER *) nmg;
			CInnerInterface::GetInstance().WARNING_MSG_ON();
			switch ( pNetMsg->nCheatType )
			{
			case 1:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("CHEAT_DETECTION_NOTICE_MULTIHIT"));
				break;
			case 2:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("CHEAT_DETECTION_NOTICE_SELFBUFF"));
				break;
			case 3:
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("CHEAT_DETECTION_NOTICE_PARTYBUFF"));
				break;
			case 4:
				//CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::NEGATIVE, "We've warned you already, yet you still persisted. You have been kicked out of the game and won't be able to login for a few minutes due to auto potion usage.");
				CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::NEGATIVE, "We've warned you already, yet you still persisted. You have been kicked out of the game due to auto potion usage.");
				break;
			}	
			DxGlobalStage::GetInstance().GetNetClient()->CloseConnect();
		}
		break;

	case NET_MSG_GM_EVENT_EXP_FB:
		{
			GLMSG::SNET_GM_EVENT_EXP_FB *pNetMsg = (GLMSG::SNET_GM_EVENT_EXP_FB *) nmg;
			if ( pNetMsg->bFAIL )
				CInnerInterface::GetInstance().PrintConsoleText ( "Experience Event Start Failed! Set Proper Rate!" );
			else
				CInnerInterface::GetInstance().PrintConsoleText ( "Experience Event Start New Rate : %3.2f", pNetMsg->fRATE );
		}break;

	case NET_MSG_GM_EVENT_EXP_END_FB:
		{
			GLMSG::SNET_GM_EVENT_EXP_END_FB *pNetMsg = (GLMSG::SNET_GM_EVENT_EXP_END_FB *) nmg;
			CInnerInterface::GetInstance().PrintConsoleText ( "Experience Event End! Restore to original Rate : %3.2f", pNetMsg->fRATE );
		}break;

	case NET_MSG_GM_EVENT_ITEM_GEN_FB:
		{
			GLMSG::SNET_GM_EVENT_ITEM_GEN_FB *pNetMsg = (GLMSG::SNET_GM_EVENT_ITEM_GEN_FB *) nmg;
			if ( pNetMsg->bFAIL )
				CInnerInterface::GetInstance().PrintConsoleText ( "Item Drop Rate Event Start Failed! Set Proper Rate!" );
			else
				CInnerInterface::GetInstance().PrintConsoleText ( "Item Drop Rate Event Start New Rate : %3.2f", pNetMsg->fRATE );
		}break;

	case NET_MSG_GM_EVENT_ITEM_GEN_END_FB:
		{
			GLMSG::SNET_GM_EVENT_ITEM_GEN_END_FB *pNetMsg = (GLMSG::SNET_GM_EVENT_ITEM_GEN_END_FB *) nmg;
			CInnerInterface::GetInstance().PrintConsoleText ( "Item Drop Event End! Restore to original Rate : %3.2f", pNetMsg->fRATE );
		}break;

	case NET_MSG_GM_EVENT_MONEY_GEN_FB:
		{
			GLMSG::SNET_GM_EVENT_MONEY_GEN_FB *pNetMsg = (GLMSG::SNET_GM_EVENT_MONEY_GEN_FB *) nmg;
			if ( pNetMsg->bFAIL )
				CInnerInterface::GetInstance().PrintConsoleText ( "Money Drop Rate Event Start Failed! Set Proper Rate!" );
			else
				CInnerInterface::GetInstance().PrintConsoleText ( "Money Drop Rate Event Start New Rate : %3.2f", pNetMsg->fRATE );
		}break;

	case NET_MSG_GM_EVENT_MONEY_GEN_END_FB:
		{
			GLMSG::SNET_GM_EVENT_MONEY_GEN_END_FB *pNetMsg = (GLMSG::SNET_GM_EVENT_MONEY_GEN_END_FB *) nmg;
			CInnerInterface::GetInstance().PrintConsoleText ( "Money Drop Event End! Restore to original Rate : %3.2f", pNetMsg->fRATE );
		}break;

	case NET_MSG_GM_BIGHEAD_BRD:
		{
			GLMSG::SNET_GM_BIGHEAD *pNetMsg = (GLMSG::SNET_GM_BIGHEAD *) nmg;
			DxSkinAniControl::m_bBIGHEAD = pNetMsg->bBIGHEAD;
		}
		break;

	case NET_MSG_GM_BIGHAND_BRD:
		{
			GLMSG::SNET_GM_BIGHAND *pNetMsg = (GLMSG::SNET_GM_BIGHAND *) nmg;
			DxSkinAniControl::m_bBIGHAND = pNetMsg->bBIGHAND;
		}
		break;

	case NET_MSG_GM_FREEPK_BRD:
		{
			// Ŭ���̾�Ʈ�� ���� ������ �ִٸ� ���⿡ �ڵ�
			GLMSG::SNET_GM_FREEPK_BRD *pNetMsg = (GLMSG::SNET_GM_FREEPK_BRD *) nmg;
			if ( pNetMsg->bSTATEUPDATE == false )
			{
				if ( pNetMsg->dwPKTIME != 0 )
				{
					m_bBRIGHTEVENT = true;
					CInnerInterface::GetInstance().PrintConsoleText ( ID2GAMEINTEXT("BRIGHT_EVENT_START") );
				}
				else
				{
					m_bBRIGHTEVENT = false;
					CInnerInterface::GetInstance().PrintConsoleText ( ID2GAMEINTEXT("BRIGHT_EVENT_END") );

					// ������� ��� ����
					GetCharacter()->DEL_PLAYHOSTILE_ALL();
				}
			}
		}
		break;

		/*private market set, Juver, 2018/01/02 */
	case NET_MSG_GM_SET_PRIVATE_MARKET_BRD:
		{
			GLMSG::SNET_GM_SET_PRIVATE_MARKET_BRD *pNetMsg = (GLMSG::SNET_GM_SET_PRIVATE_MARKET_BRD *) nmg;
			if ( pNetMsg->bEnable )
				CInnerInterface::GetInstance().PrintConsoleText ( ID2GAMEINTEXT("PRIVATE_MARKET_SET_ON") );
			else
				CInnerInterface::GetInstance().PrintConsoleText ( ID2GAMEINTEXT("PRIVATE_MARKET_SET_OFF") );
		}break;

		/*megaphone set, Juver, 2018/01/02 */
	case NET_MSG_GM_SET_MEGAPHONE_BRD:
		{
			GLMSG::SNET_GM_SET_MEGAPHONE_BRD *pNetMsg = (GLMSG::SNET_GM_SET_MEGAPHONE_BRD *) nmg;
			if ( pNetMsg->bEnable )
				CInnerInterface::GetInstance().PrintConsoleText ( ID2GAMEINTEXT("MEGAPHONE_SET_ON") );
			else
				CInnerInterface::GetInstance().PrintConsoleText ( ID2GAMEINTEXT("MEGAPHONE_SET_OFF") );
		}break;

	case NET_MSG_GM_SHOP_INFO_FB:
		{
			GLMSG::SNET_GM_SHOP_INFO_FB *pNetMsg = (GLMSG::SNET_GM_SHOP_INFO_FB *)nmg;

			if( pNetMsg->bSTATE == GLMSG::SNET_GM_SHOP_INFO_FB::END )
			{
				CInnerInterface::GetInstance().PrintConsoleText ( "Shop Info Search End! [%s][%s]", m_szShopInfoFile, pNetMsg->szBasicInfo );
				break;
			}

			if( pNetMsg->bSTATE == GLMSG::SNET_GM_SHOP_INFO_FB::FIRST )
			{
				CInnerInterface::GetInstance().PrintConsoleText ( "Shop Info Search Start!" );

				TCHAR szPROFILE[MAX_PATH] = {0};
				memset(m_szShopInfoFile, 0, sizeof(char) * (MAX_PATH));
				SHGetSpecialFolderPath( NULL, szPROFILE, CSIDL_PERSONAL, FALSE );

				StringCchCopy( m_szShopInfoFile, MAX_PATH, szPROFILE );
				StringCchCat( m_szShopInfoFile, MAX_PATH, SUBPATH::SAVE_ROOT );
				CreateDirectory( m_szShopInfoFile, NULL );

				CTime curTime = GLGaeaClient::GetInstance().GetCurrentTime();

				CHAR  szFileName[MAX_PATH] = {0};
				sprintf( szFileName, "ShopInfo_[%d%d%d%d%d%d].csv", curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(), 
																  curTime.GetHour(), curTime.GetMinute(), curTime.GetSecond() );

				StringCchCat ( m_szShopInfoFile, MAX_PATH, szFileName );
			}

			FILE*	fp = NULL;
			fp = fopen( m_szShopInfoFile, "a+" );
			if( fp )
			{
				if( pNetMsg->bSTATE == GLMSG::SNET_GM_SHOP_INFO_FB::FIRST )
				{
					if( pNetMsg->bBasicInfo )
					{
						char szHeader[] = "UserNum,CharNum,ItemMid,ItemSid,Price\n";
						fprintf( fp, szHeader );
					}else{
						char szHeader[] = "UserNum,CharNum,ItemMid,ItemSid,Price,RanOptType1,RandValue1,RanOptType2,RandValue2,RanOptType3,RandValue3,RanOptType4,RandValue4,DAMAGE,DEFENSE,RESIST_FIRE,RESIST_ICE,RESIST_ELEC,RESIST_POISON,RESIST_SPIRIT\n";
						fprintf( fp, szHeader );																			  
					}																										  
				}																											  
				fprintf( fp, pNetMsg->szBasicInfo );																		  
				fclose( fp );
			}		

		}
		break;

		// �븸���� ��û�� ���ο� GM ���ɾ� ( /dsp player �� Ȯ���� )
	case NET_MSG_GM_VIEWALLPLAYER_FLD_FB:
		{
			GLMSG::SNET_GM_VIEWALLPLAYER_FLD_FB *pNetMsg = (GLMSG::SNET_GM_VIEWALLPLAYER_FLD_FB *)nmg;
			
			if( pNetMsg->dwPlayerNum != 0 )
				CInnerInterface::GetInstance().PrintConsoleText ( "Player Count : %d", pNetMsg->dwPlayerNum + 1 );
			else
				CInnerInterface::GetInstance().PrintConsoleText ( "%s	: charid = %d", pNetMsg->szCHARNAME, pNetMsg->dwCHARID );
		}
		break;


	case NET_MSG_GM_VIEWWORKEVENT_FB:
		{
			GLMSG::SNET_GM_VIEWWORKEVENT_FB *pNetMsg = (GLMSG::SNET_GM_VIEWWORKEVENT_FB *)nmg;
			CInnerInterface::GetInstance().PrintMsgText( NS_UITEXTCOLOR::NEGATIVE, "%s", pNetMsg->szWorkEventInfo );
		}
		break;

	case NET_MSG_GCTRL_CLUB_MARK_INFO_FB:
		{
			GLMSG::SNET_CLUB_MARK_INFO_FB *pNetMsg = (GLMSG::SNET_CLUB_MARK_INFO_FB *)nmg;
			
			//	Note : �̹����� ����.
			DWORD dwServerID = GLGaeaClient::GetInstance().GetCharacter()->m_dwServerID;
			DxClubMan::GetInstance().SetClubData ( DxGlobalStage::GetInstance().GetD3dDevice(), dwServerID, pNetMsg->dwClubID, pNetMsg->dwMarkVER, pNetMsg->aryMark );
		}
		break;

	case NET_MSG_GCTRL_CLUB_BATTLE_START_BRD:
		{
			GLMSG::SNET_CLUB_BATTLE_START_BRD *pNetMsg = (GLMSG::SNET_CLUB_BATTLE_START_BRD *)nmg;
			
			if ( pNetMsg->nTIME==0 )
			{
				CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("CLUB_BATTLE_START"), pNetMsg->szName );
				m_bCLUBBATTLE = true;

				CMiniMap * pMiniMap = CInnerInterface::GetInstance().GetMiniMap();
				if( pMiniMap ) pMiniMap->StartClubTime();
			}
			else
			{
				CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("CLUB_BATTLE_MIN_START"), pNetMsg->szName, pNetMsg->nTIME );
			}
		}
		break;

	case NET_MSG_GCTRL_CLUB_DEATHMATCH_START_BRD:
		{
			GLMSG::SNET_CLUB_DEATHMATCH_START_BRD *pNetMsg = (GLMSG::SNET_CLUB_DEATHMATCH_START_BRD *)nmg;
			
			if ( pNetMsg->nTIME==0 )
			{
				CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("CLUB_DEATHMATCH_START"), pNetMsg->szName );
				m_bClubDeathMatch = true;

				CMiniMap * pMiniMap = CInnerInterface::GetInstance().GetMiniMap();
				if( pMiniMap ) pMiniMap->StartClubTime();
			}
			else
			{
				CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("CLUB_DEATHMATCH_MIN_START"), pNetMsg->szName, pNetMsg->nTIME );
			}
		}
		break;

	case NET_MSG_GCTRL_CLUB_DEATHMATCH_END_BRD:
		{
			GLMSG::SNET_CLUB_DEATHMATCH_END_BRD *pNetMsg = (GLMSG::SNET_CLUB_DEATHMATCH_END_BRD *)nmg;
			CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("CLUB_DEATHMATCH_END"), pNetMsg->szName, pNetMsg->szClubName );

			m_bClubDeathMatch = false;
			m_fClubDeathMatchTimer = 0.0f;

			CMiniMap * pMiniMap = CInnerInterface::GetInstance().GetMiniMap();
			if( pMiniMap ) pMiniMap->EndClubTime();

			// ������� ��� ����
			GetCharacter()->DEL_PLAYHOSTILE_ALL();
		}
		break;

	case NET_MSG_GCTRL_CLUB_DEATHMATCH_REMAIN_BRD:
		{
			GLMSG::SNET_CLUB_DEATHMATCH_REMAIN_BRD *pNetMsg = (GLMSG::SNET_CLUB_DEATHMATCH_REMAIN_BRD*)nmg;
			m_fClubDeathMatchTimer = (float)pNetMsg->dwTime;
		}
		break;

	case NET_MSG_GCTRL_CLUB_DEATHMATCH_POINT_UPDATE:
		{
			GLMSG::SNET_CLUB_DEATHMATCH_POINT_UPDATE *pNetMsg = (GLMSG::SNET_CLUB_DEATHMATCH_POINT_UPDATE*)nmg;	
			if ( pNetMsg->bKillPoint )	m_sMyCdmRank.wKillNum++;
			else						m_sMyCdmRank.wDeathNum++;

			//	UI Refrash
			CInnerInterface::GetInstance().VisibleCDMRanking(true);
			CInnerInterface::GetInstance().RefreshCDMRanking();
		}
		break;
	case NET_MSG_GCTRL_CLUB_DEATHMATCH_MYRANK_UPDATE:
		{

			GLMSG::SNET_CLUB_DEATHMATCH_MYRANK_UPDATE *pNetMsg = (GLMSG::SNET_CLUB_DEATHMATCH_MYRANK_UPDATE*)nmg;	

			m_sMyCdmRank = pNetMsg->sMyCdmRank;

			//	UI Refrash �� ����
			CInnerInterface::GetInstance().VisibleCDMRanking(true);
			CInnerInterface::GetInstance().RefreshCDMRanking();
		}
		break;

	case NET_MSG_GCTRL_CLUB_DEATHMATCH_RANKING_UPDATE:
		{
			GLMSG::SNET_CLUB_DEATHMATCH_RANKING_UPDATE *pNetMsg = (GLMSG::SNET_CLUB_DEATHMATCH_RANKING_UPDATE*)nmg;	

			int nRankNum = pNetMsg->wRankNum;
			
			for ( int i = 0; i < nRankNum; ++i )
			{
				int nIndex = pNetMsg->sCdmRank[i].nIndex;
				if ( nIndex < 0 ) continue;
				
				int nSize = m_vecCdmRank.size();				
				if ( nIndex < nSize )
				{
					m_vecCdmRank[nIndex] = pNetMsg->sCdmRank[i];
				}
				else
				{
					m_vecCdmRank.resize( nIndex+1 );
					m_vecCdmRank[nIndex] = pNetMsg->sCdmRank[i];
				}
			}

			//	UI Refrash �� ����
			CInnerInterface::GetInstance().VisibleCDMRanking(true);
			CInnerInterface::GetInstance().RefreshCDMRanking();

		}
		break;

	case NET_MSG_GCTRL_CLUB_CD_CERTIFY_BRD:
		{
			GLMSG::SNET_CLUB_CD_CERTIFY_BRD *pNetMsg = (GLMSG::SNET_CLUB_CD_CERTIFY_BRD *)nmg;
			if ( strlen ( pNetMsg->szAlliance ) != 0 )
			{
				CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("CLUB_BATTLE_CERTIFY_START_EX"), pNetMsg->szZone, pNetMsg->szAlliance, pNetMsg->szClub, pNetMsg->szName );
			}
			else
			{
				CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("CLUB_BATTLE_CERTIFY_START"), pNetMsg->szZone, pNetMsg->szClub, pNetMsg->szName );
			}
		}
		break;

	case NET_MSG_GCTRL_CLUB_CD_CERTIFY_ING_BRD:
		{
			GLMSG::SNET_CLUB_CD_CERTIFY_ING_BRD *pNetMsg = ( GLMSG::SNET_CLUB_CD_CERTIFY_ING_BRD* ) nmg;
			switch ( pNetMsg->emFB )
			{
			case EMCDCERTIFY_ING_DIE:
				{
					if ( strlen ( pNetMsg->szAlliance ) != 0 )
					{
						CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("EMCDCERTIFY_ING_DIE_EX_ALLI"), pNetMsg->szZone, pNetMsg->szAlliance, pNetMsg->szClub, pNetMsg->szName );
					}
					else
					{
						CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("EMCDCERTIFY_ING_DIE_EX"), pNetMsg->szZone, pNetMsg->szClub, pNetMsg->szName );
					}
				}
				break;
			case EMCDCERTIFY_ING_TIMEOUT:
				{
					if ( strlen ( pNetMsg->szAlliance ) != 0 )
					{
						CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("EMCDCERTIFY_ING_TIMEOUT_EX_ALLI"), pNetMsg->szZone, pNetMsg->szAlliance, pNetMsg->szClub, pNetMsg->szName );
					}
					else
					{
						CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("EMCDCERTIFY_ING_TIMEOUT_EX"), pNetMsg->szZone, pNetMsg->szClub, pNetMsg->szName );
					}
				}
				break;
			case EMCDCERTIFY_ING_DISTANCE:
				{
					if ( strlen ( pNetMsg->szAlliance ) != 0 )
					{
						CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("EMCDCERTIFY_ING_DISTANCE_EX_ALLI"), pNetMsg->szZone, pNetMsg->szAlliance, pNetMsg->szClub, pNetMsg->szName );
					}
					else
					{
						CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("EMCDCERTIFY_ING_DISTANCE_EX"), pNetMsg->szZone, pNetMsg->szClub, pNetMsg->szName );
					}
				}
				break;
			}
		}
		break;

	case NET_MSG_GCTRL_CLUB_BATTLE_END_BRD:
		{
			GLMSG::SNET_CLUB_BATTLE_END_BRD *pNetMsg = (GLMSG::SNET_CLUB_BATTLE_END_BRD *)nmg;
			CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("CLUB_BATTLE_END"), pNetMsg->szName, pNetMsg->szClubName );

			m_bCLUBBATTLE = false;
			m_fCLUBBATTLETimer = 0.0f;

			CMiniMap * pMiniMap = CInnerInterface::GetInstance().GetMiniMap();
			if( pMiniMap ) pMiniMap->EndClubTime();

			// ������� ��� ����
			GetCharacter()->DEL_PLAYHOSTILE_ALL();
		}
		break;

	case NET_MSG_GCTRL_CLUB_BATTLE_REMAIN_BRD:
		{
			GLMSG::SNET_CLUB_BATTLE_REMAIN_BRD *pNetMsg = (GLMSG::SNET_CLUB_BATTLE_REMAIN_BRD*)nmg;
			m_fCLUBBATTLETimer = (float)pNetMsg->dwTime;
		}
		break;

	case NET_MSG_GCTRL_LAND_INFO:
		{
			GLMSG::SNETPC_LAND_INFO *pNetMsg = (GLMSG::SNETPC_LAND_INFO *)nmg;

			PLANDMANCLIENT pLand = GetActiveMap();
			if ( pLand && pLand->GetMapID()==pNetMsg->nidMAP )
			{
				/*dmk14 large map window ep9*/
				GetCharacter()->ReqRetrieveMobInMap( pNetMsg->nidMAP );

				pLand->m_bClubBattle = pNetMsg->bClubBattle;
				pLand->m_bClubBattleHall = pNetMsg->bClubBattleHall;
				pLand->m_bClubDeathMatch = pNetMsg->bClubDeathMatch;
				pLand->m_bClubDeathMatchHall = pNetMsg->bClubDeathMatchHall;
				pLand->SetPKZone ( pNetMsg->bPK );
				pLand->m_fCommission = pNetMsg->fCommission;

				pLand->m_dwGuidClub = pNetMsg->dwGuidClub;
				pLand->m_dwGuidClubMarkVer = pNetMsg->dwGuidClubMarkVer;
				StringCchCopy ( pLand->m_szGuidClubName, CHAR_SZNAME, pNetMsg->szGuidClubName );

				/*pvp tyranny, Juver, 2017/08/24 */
				pLand->m_bPVPTyrannyMap = pNetMsg->bPVPTyranny;

				/*school wars, Juver, 2018/01/19 */
				pLand->m_bPVPSchoolWarsMap = pNetMsg->bPVPSchoolWars;

				/*pvp capture the flag, Juver, 2018/01/31 */
				pLand->m_bPVPCaptureTheFlagMap = pNetMsg->bPVPCaptureTheFlag;

				/*pvp tyranny, Juver, 2017/08/24 */
				if ( pLand->m_bPVPTyrannyMap )
					CInnerInterface::GetInstance().ShowGroupBottom( PVP_TYRANNY_TOWER_CAPTURE );
				else
					CInnerInterface::GetInstance().HideGroup( PVP_TYRANNY_TOWER_CAPTURE );

				/*school wars, Juver, 2018/01/20 */
				if ( pLand->m_bPVPSchoolWarsMap )
					CInnerInterface::GetInstance().ShowGroupBottom( PVP_SCHOOLWARS_SCORE );
				else
					CInnerInterface::GetInstance().HideGroup( PVP_SCHOOLWARS_SCORE );

				/*pvp capture the flag, Juver, 2018/02/02 */
				if ( pLand->m_bPVPCaptureTheFlagMap )
					CInnerInterface::GetInstance().ShowGroupBottom( PVP_CAPTURE_THE_FLAG_SCORE );
				else
					CInnerInterface::GetInstance().HideGroup( PVP_CAPTURE_THE_FLAG_SCORE );


				/////////////////////////////////////////////////////////////////////////////
				// LG-7 CWRanking
				if (m_bCLUBBATTLE)
				{
					GLCharacter* pCharacter = GetCharacter();
					if (pCharacter) pCharacter->ReqCWRanking();
				}
				else CInnerInterface::GetInstance().SetCWRankingDisplayOpen(FALSE);
				/////////////////////////////////////////////////////////////////////////////

				if ( pLand->m_bClubBattle )
				{
					CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("CLUB_BATTLE_LAND") );
					CInnerInterface::GetInstance().SetCWRankingDisplayOpen ( TRUE );
				}

				if ( pLand->m_bClubDeathMatch && m_vecCdmRank.empty() )
				{
					//	���� ��û
					ReqClubDeathMatchInfo();
					CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("CLUB_DEATHMATCH_LAND") );
				}
				else if ( !pLand->m_bClubDeathMatch )
				{
					m_vecCdmRank.clear();
					m_sMyCdmRank.Init();
					//	UI �ݱ�
					CInnerInterface::GetInstance().VisibleCDMRanking( false );
				}


				CString str;
				str.Format( ID2GAMEINTEXT("LAND_TRADE_COMMISSION"), pLand->m_fCommission );

				if ( m_bBRIGHTEVENT )
				{
					str += _T("\r\n");
					str += ID2GAMEINTEXT("BRIGHT_EVENT_MSG");
				}

				if ( m_bSCHOOL_FREEPK )
				{
					str += _T("\r\n");
					str += ID2GAMEINTEXT("OTHER_SCHOOL_FREEPK_ON");
				}

				// ������� ��� ����
				GetCharacter()->DEL_PLAYHOSTILE_ALL();

				/*pvp tyranny, Juver, 2017/08/24 */
				/*school wars, Juver, 2018/01/19 */
				/*pvp capture the flag, Juver, 2018/01/31 */
				if ( !pLand->m_bPVPTyrannyMap && !pLand->m_bPVPSchoolWarsMap && !pLand->m_bPVPCaptureTheFlagMap )
					CInnerInterface::GetInstance().PrintConsoleTextDlg( str );

				/*pvp tyranny, Juver, 2017/08/24 */
				if ( pLand->m_bPVPTyrannyMap )
				{
					GLPVPTyrannyClient::GetInstance().RequestTowerInfo();

					if ( CInnerInterface::GetInstance().PVPTyrannyToShowRanking() )
						CInnerInterface::GetInstance().PVPTyrannyShowRanking();


					if ( !GLPVPTyrannyClient::GetInstance().m_bShowStartNotice &&
						GLPVPTyrannyClient::GetInstance().IsBattle() )
					{
						CInnerInterface::GetInstance().PVPTyrannyShowStartNotice();
						GLPVPTyrannyClient::GetInstance().m_bShowStartNotice = TRUE;
					}
				}

				/*school wars, Juver, 2018/01/20 */
				if ( pLand->m_bPVPSchoolWarsMap )
				{
					GLPVPSchoolWarsClient::GetInstance().RequestScoreInfo();

					if ( CInnerInterface::GetInstance().PVPSchoolWarsToShowRanking() )
						CInnerInterface::GetInstance().PVPSchoolWarsShowRanking();

					if ( !GLPVPSchoolWarsClient::GetInstance().m_bShowStartNotice &&
						GLPVPSchoolWarsClient::GetInstance().IsBattle() )
					{
						CInnerInterface::GetInstance().PVPSchoolWarsShowStartNotice();
						GLPVPSchoolWarsClient::GetInstance().m_bShowStartNotice = TRUE;
					}
				}

				/*pvp capture the flag, Juver, 2018/02/02 */
				if ( pLand->m_bPVPCaptureTheFlagMap )
				{
					GLPVPCaptureTheFlagClient::GetInstance().RequestScoreInfo();

					if ( CInnerInterface::GetInstance().PVPCaptureTheFlagToShowRanking() )
						CInnerInterface::GetInstance().PVPCaptureTheFlagShowRanking();

					if ( !GLPVPCaptureTheFlagClient::GetInstance().m_bShowStartNotice &&
						GLPVPCaptureTheFlagClient::GetInstance().IsBattle() )
					{
						CInnerInterface::GetInstance().PVPCaptureTheFlagShowStartNotice();
						GLPVPCaptureTheFlagClient::GetInstance().m_bShowStartNotice = TRUE;
					}
				}

				/*pvp capture the flag, Juver, 2018/02/08 */
				if ( pLand->m_bPVPCaptureTheFlagMap )
					GetCharacter()->m_bCaptureTheFlagLocatorOn = TRUE;
				else
					GetCharacter()->m_bCaptureTheFlagLocatorOn = FALSE;
			}
		}
		break;

	case NET_MSG_GCTRL_SERVER_INFO:
		{
			GLMSG::SNETPC_SERVER_INFO *pNetMsg = (GLMSG::SNETPC_SERVER_INFO *)nmg;

			if ( m_bSCHOOL_FREEPK != pNetMsg->bSCHOOL_FREEPK )
			{
				if ( pNetMsg->bSCHOOL_FREEPK )
				{
					CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("OTHER_SCHOOL_FREEPK_ON") );
				}
				else
				{
					CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("OTHER_SCHOOL_FREEPK_OFF") );

					// ������� ��� ����
					GetCharacter()->DEL_PLAYHOSTILE_ALL();
				}
			}

			m_bSCHOOL_FREEPK = pNetMsg->bSCHOOL_FREEPK;

		}
		break;

	case NET_MSG_GCTRL_SERVER_BRIGHTEVENT_INFO:
		{
			GLMSG::SNETPC_SERVER_BRIGHTEVENT_INFO *pNetMsg = ( GLMSG::SNETPC_SERVER_BRIGHTEVENT_INFO *)nmg;

			m_bBRIGHTEVENT = pNetMsg->bBRIGHTEVENT;
		}
		break;

	case NET_MSG_GCTRL_SERVER_CLUB_BATTLE_INFO:
		{
			GLMSG::SNETPC_SERVER_CLUB_BATTLE_INFO* pNetMsg = ( GLMSG::SNETPC_SERVER_CLUB_BATTLE_INFO* )nmg;
			m_bCLUBBATTLE = pNetMsg->bClubBattle;
		}
		break;
	
	case NET_MSG_GCTRL_SERVER_CLUB_DEATHMATCH_INFO:
		{
			GLMSG::SNETPC_SERVER_CLUB_DEATHMATCH_INFO* pNetMsg = ( GLMSG::SNETPC_SERVER_CLUB_DEATHMATCH_INFO* )nmg;
			m_bClubDeathMatch = pNetMsg->bClubDeathMatch;
		}
		break;

	case NET_MSG_GCTRL_CLUB_COMMISSION_BRD:
		{
			GLMSG::SNET_CLUB_GUID_COMMISSION_BRD *pNetMsg = (GLMSG::SNET_CLUB_GUID_COMMISSION_BRD *)nmg;

			PLANDMANCLIENT pLand = GetActiveMap();
			if ( pLand )
			{
				pLand->m_fCommission = pNetMsg->fCommission;
				CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("LAND_TRADE_NEW_COMMISSION"), pLand->m_fCommission );
			}
		}
		break;

	case NET_MSG_GCTRL_CLUB_COMMISSION_RV_BRD:
		{
			GLMSG::SNET_CLUB_GUID_COMMISSION_RESERVE_BRD *pNetMsg = (GLMSG::SNET_CLUB_GUID_COMMISSION_RESERVE_BRD *)nmg;

			CInnerInterface::GetInstance().PrintConsoleTextDlg ( ID2GAMEINTEXT("LAND_TRADE_NEW_RV_COMMISSION"), pNetMsg->fCommission );
		}
		break;

	case NET_MSG_GM_WHERE_NPC_FB:
		{
			GLMSG::SNET_GM_WHERE_NPC_FB *pNetMsg = (GLMSG::SNET_GM_WHERE_NPC_FB *) nmg;

			CInnerInterface::GetInstance().PrintConsoleText ( "npc_pos : %d, %d", pNetMsg->nPosX, pNetMsg->nPosY );
		}
		break;

	case NET_MSG_GM_PRINT_CROWLIST_FB:
		{
			GLMSG::SNET_GM_PRINT_CROWLIST_FB *pNetMsg = (GLMSG::SNET_GM_PRINT_CROWLIST_FB *) nmg;

			PCROWDATA pCrow = GLCrowDataMan::GetInstance().GetCrowData( pNetMsg->mID, pNetMsg->sID );
			if ( !pCrow ) break;

			if( pNetMsg->emCrow == CROW_NPC )
			{
				
				CInnerInterface::GetInstance().PrintMsgText( 
					NS_UITEXTCOLOR::PALEGREEN, "type : npc, mid : %d, SID : %d, num : %d, name : %s", 
					pNetMsg->mID, pNetMsg->sID, pNetMsg->wNum, pCrow->GetName() );
			}else{
				CInnerInterface::GetInstance().PrintMsgText( 
					NS_UITEXTCOLOR::RED, "type : monster, mid : %d, SID : %d, num : %d, name : %s", 
					pNetMsg->mID, pNetMsg->sID, pNetMsg->wNum, pCrow->GetName() );
			}
			
		}
		break;


	case NET_MSG_GM_WHERE_PC_MAP_FB:
		{
			GLMSG::SNET_GM_WHERE_PC_MAP_FB *pNetMsg = (GLMSG::SNET_GM_WHERE_PC_MAP_FB *) nmg;

			const char *pMapName = GetMapName ( pNetMsg->nidMAP );
			if ( pMapName )
			{
				CInnerInterface::GetInstance().PrintConsoleText ( "pc_map : %s, channel %d, mapid[%d/%d]",
					pMapName, pNetMsg->dwChannel, pNetMsg->nidMAP.wMainID, pNetMsg->nidMAP.wSubID );
			}
		}
		break;

	case NET_MSG_GM_WHERE_PC_POS_FB:
		{
			GLMSG::SNET_GM_WHERE_PC_POS_FB *pNetMsg = (GLMSG::SNET_GM_WHERE_PC_POS_FB *) nmg;
			
			if ( pNetMsg->bFOUND )		CInnerInterface::GetInstance().PrintConsoleText ( "pc_pos : %d, %d", pNetMsg->nPosX, pNetMsg->nPosY );
			else						CInnerInterface::GetInstance().PrintConsoleText ( "pc_pos : not found" );
		}
		break;

	case NET_MSG_GM_MOVE2CHAR_FB:
		{
			GLMSG::SNETPC_GM_MOVE2CHAR_FB *pNetMsg = (GLMSG::SNETPC_GM_MOVE2CHAR_FB *) nmg;

			switch ( pNetMsg->emFB )
			{
			case EMGM_MOVE2CHAR_FB_FAIL:
				CInnerInterface::GetInstance().PrintConsoleText ( ID2GAMEINTEXT("EMGM_MOVE2CHAR_FB_FAIL") );
				break;
			case EMGM_MOVE2CHAR_FB_OK:
				CInnerInterface::GetInstance().PrintConsoleText ( ID2GAMEINTEXT("EMGM_MOVE2CHAR_FB_OK") );
				break;
			case EMGM_MOVE2CHAR_FB_TO_CONDITION:
				CInnerInterface::GetInstance().PrintConsoleText ( ID2GAMEINTEXT("EMGM_MOVE2CHAR_FB_TO_CONDITION") );
				break;
			case EMGM_MOVE2CHAR_FB_MY_CONDITION:
				CInnerInterface::GetInstance().PrintConsoleText ( ID2GAMEINTEXT("EMGM_MOVE2CHAR_FB_MY_CONDITION") );
				break;
			case EMGM_MOVE2CHAR_FB_CHANNEL:
				CInnerInterface::GetInstance().PrintConsoleText ( ID2GAMEINTEXT("EMGM_MOVE2CHAR_FB_CHANNEL"), pNetMsg->nChannel );
				break;
			};
		}
		break;

	case NET_MSG_GM_CHAT_BLOCK_FB:
		{
			GLMSG::SNET_GM_CHAT_BLOCK_FB *pNetMsg = (GLMSG::SNET_GM_CHAT_BLOCK_FB *) nmg;

			CInnerInterface::GetInstance().PrintConsoleText ( "block        : %s", bool2sz(pNetMsg->bBLOCK) );
			CInnerInterface::GetInstance().PrintConsoleText ( "user account : %s", pNetMsg->szUACCOUNT );
			CInnerInterface::GetInstance().PrintConsoleText ( "char name    : %s", pNetMsg->szCHARNAME );
			CInnerInterface::GetInstance().PrintConsoleText ( "block minute : %d", pNetMsg->dwBLOCK_MINUTE );
		}
		break;

	case NET_MSG_GM_CHAR_INFO_AGT_FB:
		{
			GLMSG::SNET_GM_CHAR_INFO_AGT_FB *pNetMsg = (GLMSG::SNET_GM_CHAR_INFO_AGT_FB *) nmg;

			CInnerInterface::GetInstance().PrintConsoleText ( "user account : %s", pNetMsg->szUACCOUNT );
			CInnerInterface::GetInstance().PrintConsoleText ( "char name    : %s", pNetMsg->szCHARNAME );
			//CInnerInterface::GetInstance().PrintConsoleText ( "server       : %d", pNetMsg->dwSERVER );
			CInnerInterface::GetInstance().PrintConsoleText ( "channel      : %d", pNetMsg->dwCHANNEL );
			CInnerInterface::GetInstance().PrintConsoleText ( "charid       : %d", pNetMsg->dwCHARID );
			CInnerInterface::GetInstance().PrintConsoleText ( "gaeaid		  : %d", pNetMsg->dwGAEAID );
		}
		break;

	case NET_MSG_USER_CHAR_INFO_AGT_FB:
		{
			GLMSG::SNET_USER_CHAR_INFO_AGT_FB *pNetMsg = (GLMSG::SNET_USER_CHAR_INFO_AGT_FB *) nmg;

			CInnerInterface::GetInstance().PrintConsoleText ( "char name : %s", pNetMsg->szCHARNAME );
		}
		break;

	case NET_MSG_GM_CHAR_INFO_FLD_FB:
		{
			GLMSG::SNET_GM_CHAR_INFO_FLD_FB *pNetMsg = (GLMSG::SNET_GM_CHAR_INFO_FLD_FB *) nmg;

			CInnerInterface::GetInstance().PrintConsoleText ( "charid       : %d", pNetMsg->dwCHARID );
			CInnerInterface::GetInstance().PrintConsoleText ( "mapid        : %d/%d", pNetMsg->nidMAP.wMainID, pNetMsg->nidMAP.wSubID );
			CInnerInterface::GetInstance().PrintConsoleText ( "position     : %d/%d", pNetMsg->wPOSX, pNetMsg->wPOSY );
			CInnerInterface::GetInstance().PrintConsoleText ( "level        : %d", pNetMsg->wLEVEL );

			CInnerInterface::GetInstance().PrintConsoleText ( "hp           : %d/%d", pNetMsg->sHP.dwNow, pNetMsg->sHP.dwMax );
			CInnerInterface::GetInstance().PrintConsoleText ( "mp           : %d/%d", pNetMsg->sMP.dwNow, pNetMsg->sMP.dwMax );
			CInnerInterface::GetInstance().PrintConsoleText ( "sp           : %d/%d", pNetMsg->sSP.dwNow, pNetMsg->sSP.dwMax );
			CInnerInterface::GetInstance().PrintConsoleText ( "cp           : %d/%d", pNetMsg->sCP.dwNow, pNetMsg->sCP.dwMax ); /*combatpoint logic, Juver, 2017/05/29 */
			
			//std::strstream strSTREAM;
			//strSTREAM << pNetMsg->sEXP.lnNow << '/' << pNetMsg->sEXP.lnMax << std::ends;

			CInnerInterface::GetInstance().PrintConsoleText ( "exp          : %I64d/%I64d", pNetMsg->sEXP.lnNow, pNetMsg->sEXP.lnMax );
			CInnerInterface::GetInstance().PrintConsoleText ( "club         : %s", pNetMsg->szCLUB );

			//strSTREAM.freeze( false );	// Note : std::strstream�� freeze. �� �ϸ� Leak �߻�.
		}
		break;

	case NET_MSG_USER_CHAR_INFO_FLD_FB:
		{
			GLMSG::SNET_USER_CHAR_INFO_FLD_FB *pNetMsg = (GLMSG::SNET_USER_CHAR_INFO_FLD_FB *) nmg;

			CInnerInterface::GetInstance().PrintConsoleText ( "club : %s", pNetMsg->szCLUB );
		}
		break;

	case NET_MSG_GM_WARNING_MSG_BRD:
		{
			GLMSG::SNET_GM_WARNING_MSG_BRD* pNetMsg = (GLMSG::SNET_GM_WARNING_MSG_BRD*) nmg;
			if ( pNetMsg->bOn )
			{
				CInnerInterface::GetInstance().WARNING_MSG_ON();
			}
			else
			{
				CInnerInterface::GetInstance().WARNING_MSG_OFF();
			}
		}
		break;

	case NET_MSG_GM_COUNTDOWN_MSG_BRD:
		{
			GLMSG::SNET_GM_COUNTDOWN_MSG_BRD* pNetMsg = (GLMSG::SNET_GM_COUNTDOWN_MSG_BRD*) nmg;
			CInnerInterface::GetInstance().SET_COUNT_MSG( pNetMsg->nCount );
		}
		break;

		//slot event
	case NET_MSG_GCTRL_SLOT_EVENT_GEN_BRD:
		{
			GLMSG::SNET_SLOT_EVENT_GEN_BRD* pNetMsg = (GLMSG::SNET_SLOT_EVENT_GEN_BRD*) nmg;
			CInnerInterface::GetInstance().SET_SLOT_COUNT_MSG( pNetMsg->wMin, pNetMsg->wMax, pNetMsg->wWin, pNetMsg->bRoll );
		}
		break;

	case NET_MSG_GCTRL_SERVERTEST_FB:
		{
			m_pLandMClient->ResetClientObjects ();
			CString strCombine;
			strCombine.Format ( "%s", ID2GAMEINTEXT("SERVER_STOP_SOON") );
			CInnerInterface::GetInstance().DisplayChatMessage ( CHAT_TYPE_GLOBAL, NULL, strCombine.GetString() );
			
			/*dual pet skill, Juver, 2017/12/27 */
			m_Character.m_sPETSKILLFACT_A.RESET ();
			m_Character.m_sPETSKILLFACT_B.RESET ();

			/*dual pet skill, Juver, 2017/12/27 */
			FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_Character.m_dwGaeaID,m_Character.GetPosition() ), m_Character.GetSkinChar (), m_Pet.m_sActiveSkillID_A );
			FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_Character.m_dwGaeaID,m_Character.GetPosition() ), m_Character.GetSkinChar (), m_Pet.m_sActiveSkillID_B );

			m_Pet.DeleteDeviceObjects ();
		}
		break;

	case NET_MSG_SERVERTIME_BRD:
		{
			GLMSG::SNET_MSG_SERVERTIME_BRD* pNetMsg = (GLMSG::SNET_MSG_SERVERTIME_BRD*) nmg;
			m_cServerTime = pNetMsg->t64Time;
		}
		break;		

	/*dmk14 ingame web*/
	case NET_MSG_GCTRL_GET_ITEMMALL_FROMDB_FB:
		{
			GLMSG::SNET_GET_ITEMMALL_FROMDB_FB* pNetMsg = (GLMSG::SNET_GET_ITEMMALL_FROMDB_FB*)nmg;

			switch ( pNetMsg->emFB )
			{
			case EMREQ_ITEMMALL_FROMDB_FB_CLEAR:
				m_vecItemMall.clear();
				break;
			case EMREQ_ITEMMALL_FROMDB_FB_END:
				CInnerInterface::GetInstance().SetItemMallInfo ();
				break;
			case EMREQ_ITEMMALL_FROMDB_FB_BUY_SUCCESS:
				{
					CInnerInterface::GetInstance().SetItemMallInfo ();
					SITEM* pItem = GLItemMan::GetInstance().GetItem( pNetMsg->sItemMall.sNativeID );
					if( pItem )
					{
						const std::string& strItemName = pItem->GetName();
						CString strCombine = CInnerInterface::GetInstance().MakeString ( ID2GAMEINTEXT("BUY_ITEMMALL_SUCCESS"), strItemName.c_str() );
						CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::ENABLE, strCombine );
					}
				}
				break;
			case EMREQ_ITEMMALL_FROMDB_FB_BUY_FAIL:
				{
					CInnerInterface::GetInstance().SetItemMallInfo ();
					SITEM* pItem = GLItemMan::GetInstance().GetItem( pNetMsg->sItemMall.sNativeID );
					if( pItem )
					{
						const std::string& strItemName = pItem->GetName();
						CString strCombine = CInnerInterface::GetInstance().MakeString ( ID2GAMEINTEXT("BUY_ITEMMALL_FAIL"), strItemName.c_str() );
						CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, strCombine );
					}
				}
				break;
			case EMREQ_ITEMMALL_FROMDB_FB_ADD:
				{
					int nIndex = pNetMsg->sItemMall.dwIndex;
					int nSize = m_vecItemMall.size();

					if ( nIndex < nSize )
					{
						m_vecItemMall[nIndex] = pNetMsg->sItemMall;
					}
					else
					{
						m_vecItemMall.resize( nIndex+1 );
						m_vecItemMall[nIndex] = pNetMsg->sItemMall;
					}

				}
				break;
			case EMREQ_ITEMMALL_FROMDB_FB_UPDATE:
				{
					int nIndex = pNetMsg->sItemMall.dwIndex;
					m_vecItemMall[nIndex] = pNetMsg->sItemMall;
				}
				break;
			}			
		}
		break;


//----------------------------------------------------------------------------------------------------------------
// Message Processing about PET (START)
//----------------------------------------------------------------------------------------------------------------
	
	// =======  Message Processing about MyPET (START)  ======= //

	case NET_MSG_PET_REQ_USECARD_FB:
		{
			GLMSG::SNETPET_REQ_USEPETCARD_FB* pNetMsg = ( GLMSG::SNETPET_REQ_USEPETCARD_FB* ) nmg;
			switch ( pNetMsg->emFB )
			{
			case EMPET_USECARD_FB_OK:
				{
					// ���ʻ����� ���� ������������ ����ؼ� Ŭ���̾�Ʈ�� �����ۿ� �ֹ�ȣ ����
					SINVENITEM* pItem = m_Character.m_cInventory.GetItem ( m_Character.m_wInvenPosX1, m_Character.m_wInvenPosY1 );
					if ( !pItem ) return;
					if ( pItem->sItemCustom.dwPetID == 0 ) pItem->sItemCustom.dwPetID = pNetMsg->m_dwPetID;
					
					m_Pet.m_sPetSkinPackState.bUsePetSkinPack = pNetMsg->m_sPetSkinPackData.bUsePetSkinPack;
					m_Pet.m_sPetSkinPackState.petSkinMobID	  = pNetMsg->m_sPetSkinPackData.sMobID;
					m_Pet.m_sPetSkinPackState.fPetScale		  = pNetMsg->m_sPetSkinPackData.fScale;
					// ����
					if ( !FAILED( CreatePET ( nmg ) ) )
					{
						CInnerInterface::GetInstance().PrintMsgText ( 
							NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("EMPET_USECARD_FB_OK"), m_Pet.m_szName );

						/*pet status, Juver, 2017/07/30 */
						CInnerInterface::GetInstance().ShowPetStatus( TRUE );	
						
					}

					if ( m_Pet.IsUsePetSkinPack() )
					{
						CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMINVEN_PETSKINPACK_FB_OK") );
					}
				}
				break;
			case EMPET_USECARD_FB_FAIL:
				CInnerInterface::GetInstance().PrintMsgText ( 
						NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_USECARD_FB_FAIL") );
				// ���н� GenNum ó��
				m_Character.m_llPetCardGenNum = 0;
				break;
			case EMPET_USECARD_FB_INVALIDCARD:
				CInnerInterface::GetInstance().PrintMsgText ( 
						NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_USECARD_FB_INVALIDCARD") );
				// ���н� GenNum ó��
				m_Character.m_llPetCardGenNum = 0;
				break;
			case EMPET_USECARD_FB_NOITEM:
				CInnerInterface::GetInstance().PrintMsgText ( 
						NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_USECARD_FB_NOITEM") );
				// ���н� GenNum ó��
				m_Character.m_llPetCardGenNum = 0;
				break;
			case EMPET_USECARD_FB_NOTENOUGHFULL:
				CInnerInterface::GetInstance().PrintMsgText ( 
						NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_USECARD_FB_NOTENOUGHFULL") );
				// ���н� GenNum ó��
				m_Character.m_llPetCardGenNum = 0;
				break;

			case EMPET_USECARD_FB_INVALIDZONE:
				CInnerInterface::GetInstance().PrintMsgText ( 
						NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_USECARD_FB_INVALIDZONE") );
				// ���н� GenNum ó��
				m_Character.m_llPetCardGenNum = 0;
				break;
			case EMPET_USECARD_FB_NODATA:

				// ��ī�� ���� ����
				SINVENITEM* pItem = m_Character.m_cInventory.GetItem ( m_Character.m_wInvenPosX1, m_Character.m_wInvenPosY1 );
				if ( !pItem ) return;
				pItem->sItemCustom.dwPetID = 0;

				CInnerInterface::GetInstance().PrintMsgText ( 
						NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_USECARD_FB_NODATA") );
				m_Character.m_llPetCardGenNum = 0;
				break;
			}
		}		
		break;

	
	case NET_MSG_PET_PETSKINPACKOPEN_FB:
		{
			GLMSG::SNETPET_SKINPACKOPEN_FB *pNetMsg = (GLMSG::SNETPET_SKINPACKOPEN_FB *)nmg;

			if( pNetMsg->emFB == EMPET_PETSKINPACKOPEN_FB_FAIL )
			{
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMINVEN_PETSKINPACK_FB_FAIL") );
				break;
			}
			if( pNetMsg->emFB == EMPET_PETSKINPACKOPEN_FB_PETCARD_FAIL )
			{
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMPET_PETSKINPACKOPEN_FB_PETCARD_FAIL") );
				break;
			}

			m_Pet.m_sPetSkinPackState.Init();

			m_Pet.m_sPetSkinPackState.bUsePetSkinPack = pNetMsg->sPetSkinPackData.bUsePetSkinPack;
			m_Pet.m_sPetSkinPackState.petSkinMobID	  = pNetMsg->sPetSkinPackData.sMobID;
			m_Pet.m_sPetSkinPackState.fPetScale		  = pNetMsg->sPetSkinPackData.fScale;

			if( pNetMsg->emFB == EMPET_PETSKINPACKOPEN_FB_OK )
			{
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMINVEN_PETSKINPACK_FB_OK") );				
			}
			if( pNetMsg->emFB == EMPET_PETSKINPACKOPEN_FB_END )
			{
				CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMINVEN_PETSKINPACK_FB_END") );
			}

			PetSkinPackApplyEffect();

		}
		break;


	case NET_MSG_PET_REQ_UNUSECARD_FB:
		{
            GLMSG::SNETPET_REQ_UNUSEPETCARD_FB *pNetMsg = ( GLMSG::SNETPET_REQ_UNUSEPETCARD_FB* ) nmg;

			// �� �̵��߿� ���� ������� �ϴ� ���
			if ( pNetMsg->bMoveMap ) m_Character.m_bIsPetActive = TRUE;
			
			/*dual pet skill, Juver, 2017/12/27 */
			m_Character.m_sPETSKILLFACT_A.RESET ();
			m_Character.m_sPETSKILLFACT_B.RESET ();

			/*dual pet skill, Juver, 2017/12/27 */
			FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_Character.m_dwGaeaID,m_Character.GetPosition() ), m_Character.GetSkinChar (), m_Pet.m_sActiveSkillID_A );
			FACTEFF::DeleteSkillFactEffect ( STARGETID(CROW_PC,m_Character.m_dwGaeaID,m_Character.GetPosition() ), m_Character.GetSkinChar (), m_Pet.m_sActiveSkillID_B );

			m_Pet.DeleteDeviceObjects ();

			CInnerInterface::GetInstance().PrintMsgText ( 
				NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("EMPET_UNUSECARD_FB_OK"), m_Pet.m_szName );

			/*pet status, Juver, 2017/07/30 */
			CInnerInterface::GetInstance().ShowPetStatus( FALSE );	
		}
		break;

	case NET_MSG_PET_REQ_GIVEFOOD_FB:
		{
			GLMSG::SNETPET_REQ_GIVEFOOD_FB* pNetMsg = ( GLMSG::SNETPET_REQ_GIVEFOOD_FB* ) nmg;
			SITEM* pItem = GLItemMan::GetInstance().GetItem ( pNetMsg->sNativeID );
			if ( pItem )
			{
				// �������� ����
				if ( m_Pet.IsVALID () ) m_Pet.IncreaseFull ( pItem->sDrugOp.wCureVolume, pItem->sDrugOp.bRatio );

				if ( DxGlobalStage::GetInstance().IsEmulator() )
				{
					CInnerInterface::GetInstance().PrintMsgText ( 
						NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("EMPET_REQ_GIVEFOOD_FB_OK"), m_Pet.m_szName, pItem->GetName (), pItem->sDrugOp.wCureVolume );
					break;
				}

				PETCARDINFO_MAP_ITER iter = m_Character.m_mapPETCardInfo.find ( pNetMsg->dwPetID );
				if ( iter!=m_Character.m_mapPETCardInfo.end() )
				{
					SPETCARDINFO& sPet = (*iter).second;
					// ��ī���� ���� ����
					sPet.m_nFull = pNetMsg->nFull;

					CInnerInterface::GetInstance().PrintMsgText ( 
						NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("EMPET_REQ_GIVEFOOD_FB_OK"), sPet.m_szName, pItem->GetName (), pItem->sDrugOp.wCureVolume );
				}
			}
		}
		break;

	case NET_MSG_PET_REQ_PETCARDINFO_FB:
		{
			GLMSG::SNETPET_REQ_PETCARDINFO_FB* pNetMsg = ( GLMSG::SNETPET_REQ_PETCARDINFO_FB* ) nmg;

			// DB�� ������ �׳� �ʱⰪ���� �Ѿ���� ��찡 �����Ƿ� üũ���ش�.
			if ( pNetMsg->emTYPE == PET_TYPE_NONE ) break;

			SPETCARDINFO sPetCardInfo;
			sPetCardInfo.m_emTYPE = pNetMsg->emTYPE;
			sPetCardInfo.m_nFull  = pNetMsg->nFull;

			/*dual pet skill, Juver, 2017/12/27 */
			sPetCardInfo.m_sActiveSkillID_A  = pNetMsg->sActiveSkillID_A;
			sPetCardInfo.m_sActiveSkillID_B  = pNetMsg->sActiveSkillID_B;
			sPetCardInfo.m_bDualSkill		 = pNetMsg->bDualSkill;

			StringCchCopy ( sPetCardInfo.m_szName, PETNAMESIZE+1, pNetMsg->szName );

			for ( WORD i = 0; i < pNetMsg->wSkillNum; ++i )
			{
				PETSKILL sPetSkill = pNetMsg->Skills[i];
				sPetCardInfo.m_ExpSkills.insert ( std::make_pair(sPetSkill.sNativeID.dwID,sPetSkill) );
			}

			for ( WORD i = 0; i < PET_ACCETYPE_SIZE; ++i )
			{
				sPetCardInfo.m_PutOnItems[i] = pNetMsg->PutOnItems[i];
			}

			if ( !pNetMsg->bTrade )
			{
				PETCARDINFO_MAP_ITER iter = m_Character.m_mapPETCardInfo.find ( pNetMsg->dwPetID );
				if ( iter != m_Character.m_mapPETCardInfo.end() ) m_Character.m_mapPETCardInfo.erase ( iter );

				m_Character.m_mapPETCardInfo.insert ( std::make_pair(pNetMsg->dwPetID, sPetCardInfo) );
			}
			else
			{
				m_Character.m_mapPETCardInfoTemp.insert ( std::make_pair(pNetMsg->dwPetID, sPetCardInfo) );
			}
			
		}
		break;

	case NET_MSG_PET_REQ_PETREVIVEINFO_FB:
		{
			GLMSG::SNETPET_REQ_PETREVIVEINFO_FB* pNetMsg = ( GLMSG::SNETPET_REQ_PETREVIVEINFO_FB* ) nmg;
			m_Character.m_mapPETReviveInfo.clear();
			for ( WORD i = 0; i < pNetMsg->wPetReviveNum; ++i )
			{
				m_Character.m_mapPETReviveInfo.insert ( 
					std::make_pair(pNetMsg->arryPetReviveInfo[i].dwPetID, pNetMsg->arryPetReviveInfo[i] ) );
			}
		}
		break;

	case NET_MSG_PET_REQ_REVIVE_FB:
		{
			GLMSG::SNETPET_REQ_REVIVE_FB* pNetMsg = ( GLMSG::SNETPET_REQ_REVIVE_FB* ) nmg;
			
			PETREVIVEINFO sPetRevInfo;
			PETREVIVEINFO_MAP_ITER iter = m_Character.m_mapPETReviveInfo.find ( pNetMsg->dwPetID );
			if ( iter != m_Character.m_mapPETReviveInfo.end() ) sPetRevInfo = (*iter).second;
			else												break;

			switch ( pNetMsg->emFB )
			{
			case EMPET_REQ_REVIVE_FB_OK:
				{
					CInnerInterface::GetInstance().PrintMsgText ( 
						NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("EMPET_REQ_REVIVE_FB_OK"), sPetRevInfo.szPetName );
					SINVENITEM* pINVENITEM = m_Character.m_cInventory.GetItem ( m_Character.m_wInvenPosX1, m_Character.m_wInvenPosY1 );
					if ( pINVENITEM ) pINVENITEM->sItemCustom.dwPetID = sPetRevInfo.dwPetID;
					m_Character.m_mapPETReviveInfo.erase ( iter );

				}
				break;
			case EMPET_REQ_REVIVE_FB_FAIL:
				CInnerInterface::GetInstance().PrintMsgText ( 
						NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_REVIVE_FB_FAIL"), sPetRevInfo.szPetName );
				break;
			case EMPET_REQ_REVIVE_FB_DBFAIL:
				CInnerInterface::GetInstance().PrintMsgText ( 
						NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_REVIVE_FB_DBFAIL"), sPetRevInfo.szPetName );
				break;
			}
		}
		break;
	case NET_MSG_PET_NOTENOUGHINVEN:
		{
			CInnerInterface::GetInstance().PrintConsoleTextDlg( ID2GAMEINTEXT("EMPET_NOTENOUGHINVEN") );
		}
		break;

		/*dual pet skill, Juver, 2017/12/29 */
	case NET_MSG_PET_REQ_ENABLE_DUAL_SKILL_FB:
		{
			GLMSG::SNETPET_REQ_ENABLE_DUAL_SKILL_FB* pNetMsg = ( GLMSG::SNETPET_REQ_ENABLE_DUAL_SKILL_FB* ) nmg;
			switch( pNetMsg->emFB )
			{
			case EMPET_REQ_ENABLE_DUAL_SKILL_FB_OK:
				{
					SITEM* pItem = GLItemMan::GetInstance().GetItem ( pNetMsg->sNativeID );
					if ( pItem )
					{
						if ( m_Pet.IsVALID () )
						{
							m_Pet.m_bDualSkill = pNetMsg->bDualSkill;

							D3DXMATRIX matEffect;
							D3DXVECTOR3 vPos = m_Pet.GetPosition ();
							D3DXMatrixTranslation ( &matEffect, vPos.x, vPos.y, vPos.z );

							STARGETID sTargetID(CROW_PET, m_Pet.m_dwGUID, vPos );
							DxEffGroupPlayer::GetInstance().NewEffGroup( GLCONST_CHAR::strSKILL_LEARN_EFFECT.c_str(), matEffect, &sTargetID );
						}

						if ( DxGlobalStage::GetInstance().IsEmulator() )
						{
							CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("EMPET_REQ_ENABLE_DUAL_SKILL_FB_OK"), m_Pet.m_szName );
						}
						else
						{
							PETCARDINFO_MAP_ITER iter = m_Character.m_mapPETCardInfo.find ( pNetMsg->dwPetID );
							if ( iter!=m_Character.m_mapPETCardInfo.end() )
							{
								SPETCARDINFO& sPet = (*iter).second;
								sPet.m_bDualSkill = pNetMsg->bDualSkill;

								CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::PALEGREEN, ID2GAMEINTEXT("EMPET_REQ_ENABLE_DUAL_SKILL_FB_OK"), sPet.m_szName );
							}
						}
					}
				}break;

			case EMPET_REQ_ENABLE_DUAL_SKILL_FB_FAIL:
				{
					CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_ENABLE_DUAL_SKILL_FB_FAIL") );
				}break;

			case EMPET_REQ_ENABLE_DUAL_SKILL_INVALID_ITEM:
				{
					CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_ENABLE_DUAL_SKILL_INVALID_ITEM") );
				}break;

			case EMPET_REQ_ENABLE_DUAL_SKILL_INVALID_CARD:
				{
					CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_ENABLE_DUAL_SKILL_INVALID_CARD") );
				}break;

			case EMPET_REQ_ENABLE_DUAL_SKILL_ALREADY_ENABLED:
				{
					CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_ENABLE_DUAL_SKILL_ALREADY_ENABLED") );
				}break;

			case EMPET_REQ_ENABLE_DUAL_SKILL_NOT_ACTIVE:
				{
					CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMPET_REQ_ENABLE_DUAL_SKILL_NOT_ACTIVE") );
				}break;
			};
		}break;


	case NET_MSG_PET_REQ_UPDATE_MOVE_STATE_FB:

		/*dual pet skill, Juver, 2017/12/28 */
	case NET_MSG_PET_GETRIGHTOFITEM_A_FB:
	case NET_MSG_PET_GETRIGHTOFITEM_B_FB:

	case NET_MSG_PET_UPDATECLIENT_FULL:

		/*dual pet skill, Juver, 2017/12/27 */
	case NET_MSG_PET_REQ_SKILLCHANGE_A_FB:
	case NET_MSG_PET_REQ_SKILLCHANGE_B_FB:

	case NET_MSG_PET_REQ_RENAME_FB:
	case NET_MSG_PET_REQ_CHANGE_COLOR_FB:
	case NET_MSG_PET_REQ_CHANGE_STYLE_FB:
	case NET_MSG_PET_REQ_SLOT_EX_HOLD_FB:
	case NET_MSG_PET_REQ_HOLD_TO_SLOT_FB:
	case NET_MSG_PET_REQ_SLOT_TO_HOLD_FB:

		/*dual pet skill, Juver, 2017/12/27 */
	case NET_MSG_PET_ADD_SKILLFACT_A:
	case NET_MSG_PET_REMOVE_SKILLFACT_A:
	case NET_MSG_PET_ADD_SKILLFACT_B:
	case NET_MSG_PET_REMOVE_SKILLFACT_B:

	case NET_MSG_PET_REQ_LEARNSKILL_FB:
	case NET_MSG_PET_REMOVE_SLOTITEM_FB:
	case NET_MSG_PET_ATTACK:
	case NET_MSG_PET_SAD:
	case NET_MSG_PET_ACCESSORY_DELETE:
		{
			if ( m_Pet.IsVALID () )
			{
				m_Pet.MsgProcess ( nmg );
			}
		}
		break;

	// =======  Message Processing about MyPET (END)  ======= //
	case NET_MSG_SUMMON_ATTACK:
		{
			/*skill summon, Juver, 2017/10/09 */
			GLMSG::SNET_SUMMON_ATTACK* pNetMsg = ( GLMSG::SNET_SUMMON_ATTACK* ) nmg;
			GLSummonClient* psummon_client = GetSummonClient( pNetMsg->dwGUID );
			if ( psummon_client )	psummon_client->MsgProcess ( nmg );
		}break;

	case NET_MSG_SUMMON_END_REST:
		{
			/*skill summon, Juver, 2017/10/09 */
			GLMSG::SNET_SUMMON_END_REST* pNetMsg = ( GLMSG::SNET_SUMMON_END_REST* ) nmg;
			GLSummonClient* psummon_client = GetSummonClient( pNetMsg->dwGUID );
			if ( psummon_client )	psummon_client->MsgProcess ( nmg );
		}break;

	case NET_MSG_SUMMON_END_LIFE:
		{
			/*skill summon, Juver, 2017/10/09 */
			GLMSG::SNET_SUMMON_END_LIFE* pNetMsg = ( GLMSG::SNET_SUMMON_END_LIFE* ) nmg;
			GLSummonClient* psummon_client = GetSummonClient( pNetMsg->dwGUID );
			if ( psummon_client )	psummon_client->MsgProcess ( nmg );
		}break;

	case NET_MSG_SUMMON_RESET_TARGET:
		{
			/*skill summon, Juver, 2017/10/09 */
			GLMSG::SNET_SUMMON_RESET_TARGET* pNetMsg = ( GLMSG::SNET_SUMMON_RESET_TARGET* ) nmg;
			GLSummonClient* psummon_client = GetSummonClient( pNetMsg->dwGUID );
			if ( psummon_client )	psummon_client->MsgProcess ( nmg );
		}break;


	// =======  Message Processing about AnyPET (START)  ======= //

	case NET_MSG_PET_DROPPET:
		{
			GLMSG::SNETPET_DROP_PET* pNetMsg = ( GLMSG::SNETPET_DROP_PET* ) nmg;
			SDROPPET& sDropPet = pNetMsg->Data;

			if ( sDropPet.m_sMapID == m_pLandMClient->GetMapID() )
			{
				m_pLandMClient->DropPet ( &sDropPet );
			}
			else
			{
				CDebugSet::ToListView ( "sDropCrow.sMapID != m_pLandMClient->GetMapID()" );
			}
		}
		break;

	case NET_MSG_CREATE_ANYPET:
		{
			GLMSG::SNETPET_CREATE_ANYPET* pNetMsg = ( GLMSG::SNETPET_CREATE_ANYPET* ) nmg;
			SDROPPET& sDropPet = pNetMsg->Data;

			if ( sDropPet.m_sMapID == m_pLandMClient->GetMapID() )
			{				
				m_pLandMClient->CreateAnyPet ( &sDropPet );
			}
			else
			{
				CDebugSet::ToListView ( "sDropCrow.sMapID != m_pLandMClient->GetMapID()" );
			}
		}
		break;

	case NET_MSG_PET_GOTO_BRD:
	case NET_MSG_PET_REQ_UPDATE_MOVE_STATE_BRD:

		/*dual pet skill, Juver, 2017/12/27 */
	case NET_MSG_PET_REQ_SKILLCHANGE_A_BRD:
	case NET_MSG_PET_REQ_SKILLCHANGE_B_BRD:

	case NET_MSG_PET_STOP_BRD:
	case NET_MSG_PET_REQ_RENAME_BRD:
	case NET_MSG_PET_REQ_CHANGE_COLOR_BRD:
	case NET_MSG_PET_REQ_CHANGE_STYLE_BRD:
	case NET_MSG_PET_REQ_SLOT_EX_HOLD_BRD:
	case NET_MSG_PET_REQ_LEARNSKILL_BRD:
	case NET_MSG_PET_REQ_FUNNY_BRD:
	case NET_MSG_PET_REMOVE_SLOTITEM_BRD:
	case NET_MSG_PET_ATTACK_BRD:
	case NET_MSG_PET_SAD_BRD:
	case NET_MSG_PET_ACCESSORY_DELETE_BRD:
	case NET_MSG_PET_PETSKINPACKOPEN_BRD:
		{
			GLMSG::SNETPET_BROAD *pNetMsg = (GLMSG::SNETPET_BROAD *) nmg;

			PGLANYPET pPet = m_pLandMClient->GetPet ( pNetMsg->dwGUID );
			if ( !pPet )
			{
				CDebugSet::ToListView ( "[PGLCHARCLIENT NULL] ���� PET�� ���� �޼��� �߻�. gaeaid %d", pNetMsg->dwGUID );
				return;
			}

			pPet->MsgProcess ( nmg );
		}
		break;
	// =======  Message Processing about AnyPet (END)  ======= //

	// =======  Message Processing about Summon (START)  ======= //
	// =======  Message Processing about Summon (END)  ======= //

	// =======  Message Processing about AnySummon (START)  ======= //

	case NET_MSG_DROP_ANYSUMMON:
		{
			GLMSG::SNET_SUMMON_DROP_SUMMON* pNetMsg = ( GLMSG::SNET_SUMMON_DROP_SUMMON* ) nmg;
			SDROPSUMMON& sDropSummon = pNetMsg->Data;

			if ( sDropSummon.m_sMapID == m_pLandMClient->GetMapID() )
			{
				m_pLandMClient->DropSummon ( &sDropSummon );
			}
			else
			{
				CDebugSet::ToListView ( "sDropCrow.sMapID != m_pLandMClient->GetMapID()" );
			}
		}
		break;
	case NET_MSG_CREATE_ANYSUMMON:
		{
			GLMSG::SNET_SUMMON_CREATE_ANYSUMMON* pNetMsg = ( GLMSG::SNET_SUMMON_CREATE_ANYSUMMON* ) nmg;
			SDROPSUMMON& sDropSummon = pNetMsg->Data;

			if ( sDropSummon.m_sMapID == m_pLandMClient->GetMapID() )
			{				
				m_pLandMClient->CreateAnySummon ( &sDropSummon );
			}
			else
			{
				CDebugSet::ToListView ( "sDropCrow.sMapID != m_pLandMClient->GetMapID()" );
			}
		}
		break;

	case NET_MSG_SUMMON_ATTACK_BRD:
	case NET_MSG_SUMMON_REQ_STOP_BRD:
	case NET_MSG_SUMMON_GOTO_BRD:
	case NET_MSG_SUMMON_REQ_UPDATE_MOVE_STATE_BRD:

		/*skill summon, Juver, 2017/10/09 */
	case NET_MSG_SUMMON_END_REST_BRD: 
	case NET_MSG_SUMMON_END_LIFE_BRD:
	case NET_MSG_SUMMON_RESET_TARGET_BRD:
		{
			GLMSG::SNET_SUMMON_BROAD *pNetMsg = (GLMSG::SNET_SUMMON_BROAD *) nmg;

			PGLANYSUMMON pSummon = m_pLandMClient->GetSummon ( pNetMsg->dwGUID );
			if ( !pSummon )
			{
				CDebugSet::ToListView ( "[PGLCHARCLIENT NULL] ���� SUMMON�� ���� �޼��� �߻�. gaeaid %d", pNetMsg->dwGUID );
				return;
			}

			pSummon->MsgProcess ( nmg );
		}
		break;

	// =======  Message Processing about AnySummon (END)  ======= //


//----------------------------------------------------------------------------------------------------------------
// Message Processing about PET (END)
//----------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------
// Message Processing about SUMMON (START)
//----------------------------------------------------------------------------------------------------------------

	case NET_MSG_REQ_USE_SUMMON_FB:
		{
			GLMSG::SNETPC_REQ_USE_SUMMON_FB* pNetMsg = ( GLMSG::SNETPC_REQ_USE_SUMMON_FB* ) nmg;
			switch ( pNetMsg->emFB )
			{
			case EMUSE_SUMMON_FB_OK:
				{
					// ����
					if ( !FAILED( CreateSummon ( nmg ) ) )
					{
						//CInnerInterface::GetInstance().ShowGroupBottom( SUMMON_HP );	
						//CInnerInterface::GetInstance().ShowGroupBottom( SUMMON_POSION_DISPLAY );
					}

				}
				break;
	
			case EMUSE_SUMMON_FB_FAIL_INVALIDZONE:
				CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMSUMMON_FB_INVALIDZONE") );
				break;
			case EMUSE_SUMMON_FB_FAIL_NODATA:
				CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMSUMMON_FB_NODATA") );
				break;
			case EMUSE_SUMMON_FB_FAIL_VEHICLE:
				CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMSUMMON_FB_VEHICLE") );
				break;
			case EMUSE_SUMMON_FB_FAIL_CONFRONT:
				CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMSUMMON_FB_CONFRONT") );
				break;

				/*skill summon, Juver, 2017/10/09 */
			case EMUSE_SUMMON_FB_FAIL_MAX:
				CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMSUMMON_FB_FULL_TOTAL") );
				break;

				/*skill summon, Juver, 2017/10/09 */
			case EMUSE_SUMMON_FB_FAIL_MAX_SKILL:
				CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMSUMMON_FB_FULL_SKILL") );
				break;
			}
		}		
		break;

	case NET_MSG_REQ_USE_SUMMON_DEL:
		{
			/*skill summon, Juver, 2017/10/08 */
			GLMSG::SNETPC_REQ_USE_SUMMON_DEL* pNetMsg = ( GLMSG::SNETPC_REQ_USE_SUMMON_DEL* ) nmg;
			GLSummonClient* psummon_client = GetSummonClient( pNetMsg->dwGUID );
			if ( psummon_client )
				psummon_client->DeleteDeviceObjects();

			//m_Summon.DeleteDeviceObjects ();
	
		}break;
	case NET_MSG_PK_RANK_HISTORY_UPDATE:
		{
			GLMSG::SNET_PK_RANK_HISTORY_UPDATE* pNetMsg = ( GLMSG::SNET_PK_RANK_HISTORY_UPDATE* ) nmg;
			m_vecPKHistory.push_back( pNetMsg->sPKHistory );

			if ( m_vecPKHistory.size() > PKNOTIF_NUM )
			{
				m_vecPKHistory.erase( m_vecPKHistory.begin() );
			}
		}break;
//----------------------------------------------------------------------------------------------------------------
// Message Processing about SUMMON (END)
//----------------------------------------------------------------------------------------------------------------
		//fastrevive fix
	case NET_MSG_GCTRL_DEATHTIMER_FB_AG:
		{
			GLMSG::SNETPC_REQ_DEATHTIMER_FB_AG *pNetMsg = (GLMSG::SNETPC_REQ_DEATHTIMER_FB_AG *) nmg;
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("DEATH_TIMER_NOTIME") );
			CInnerInterface::GetInstance().HideGroup ( REBIRTH_DIALOGUE );
		}break;

	case NET_MSG_GCTRL_DEATHTIMER_FB_FLD:
		{
			GLMSG::SNETPC_REQ_REVIVE_DEATHTIMER_FB *pNetMsg = (GLMSG::SNETPC_REQ_REVIVE_DEATHTIMER_FB *) nmg;
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("DEATH_TIMER_NOTIME") );
			CInnerInterface::GetInstance().HideGroup ( REBIRTH_DIALOGUE );
		}break;

	default:
		CDebugSet::ToListView ( "GLGaeaClient::MsgProcess() TYPE[%d] massage leak", nmg->nType );
		break;
	};
}

GLCOPY* GLGaeaClient::GetCopyActor ( const EMCROW emCrow, const DWORD dwID )
{
	if ( emCrow == CROW_PC )
	{
		if ( GetCharacter()->m_dwGaeaID == dwID )		return GetCharacter();
		else if ( dwID != EMTARGET_NULL )				return m_pLandMClient->GetChar ( dwID );
	}
	else if ( emCrow == CROW_NPC || emCrow == CROW_MOB )
	{
		if ( dwID != EMTARGET_NULL )					return m_pLandMClient->GetCrow ( dwID );
		else											return NULL;
	}
	else if ( emCrow == CROW_MATERIAL )
	{
		if ( dwID != EMTARGET_NULL )					return m_pLandMClient->GetMaterial ( dwID );
		else											return NULL;
	}
	else if ( emCrow == CROW_SUMMON )
	{
		//if ( GetSummonClient()->m_dwGUID == dwID )		return GetSummonClient();

		/*skill summon, Juver, 2017/10/08 */
		GLSummonClient*	psummon_client = GetSummonClient( dwID );
		if( psummon_client )	return psummon_client;

		if ( dwID != EMTARGET_NULL )					return m_pLandMClient->GetSummon ( dwID );
		else											return NULL;
	}
	else if ( !m_bBRIGHTEVENT )
	{
       GASSERT(0&&"emCrow�� �߸��� ������ �Դϴ�.");
	}

	return NULL;
}

GLCOPY* GLGaeaClient::GetCopyActor ( const std::string &strName )
{
	if ( strName == GetCharacter()->m_szName )		return GetCharacter();
	else											return m_pLandMClient->FindChar ( strName );

	return NULL;
};

D3DXVECTOR3* GLGaeaClient::FindCharPos ( std::string strName )
{
	if ( strName==std::string(GetCharacter()->GetCharData().m_szName) )
	{
		m_vCharPos = GetCharacter()->GetPosition();
		return &m_vCharPos;
	}

	PGLCHARCLIENT pCharClient = GetActiveMap()->FindChar ( strName );
	if ( !pCharClient )	return NULL;

	m_vCharPos = pCharClient->GetPosition();

	return &m_vCharPos;
}

D3DXVECTOR3* GLGaeaClient::FindCharHeadPos ( std::string strName )
{
	if ( strName==std::string(GetCharacter()->GetCharData().m_szName) )
	{
		m_vCharPos = GetCharacter()->GetPosBodyHeight();

		return &m_vCharPos;
	}

	PGLCHARCLIENT pCharClient = GetActiveMap()->FindChar ( strName );
	if ( !pCharClient )	return NULL;

	m_vCharPos = pCharClient->GetPosBodyHeight();

	return &m_vCharPos;
}

D3DXVECTOR3* GLGaeaClient::FindCharHeadPos ( DWORD dwGaeaID ) 
{
	if ( dwGaeaID == GetCharacter()->m_dwGaeaID )
	{
		m_vCharPos = GetCharacter()->GetPosBodyHeight();

		return &m_vCharPos;
	}

	PGLCHARCLIENT pCharClient = GetActiveMap()->FindChar ( dwGaeaID );
	if ( !pCharClient )	return NULL;

	m_vCharPos = pCharClient->GetPosBodyHeight();

	return &m_vCharPos;
}

D3DXVECTOR3* GLGaeaClient::FindMaterialHeadPos ( DWORD dwGaeaID ) 
{
	PGLMATERIALCLIENT pMaterialClient = GetActiveMap()->GetMaterial ( dwGaeaID );
	if ( !pMaterialClient )	return NULL;

	m_vCharPos = pMaterialClient->GetPosBodyHeight();

	return &m_vCharPos;
}

DxSkinChar* GLGaeaClient::GetSkinChar ( const STARGETID &sTargetID )
{
	if ( !m_pLandMClient )	return NULL;

	//	Note : Ÿ���� ��ġ ������ ������.
	if ( sTargetID.emCrow == CROW_PC )
	{
		if ( GetCharacter()->m_dwGaeaID == sTargetID.dwID )	return GetCharacter()->GetSkinChar();

		PGLCHARCLIENT pChar = m_pLandMClient->GetChar ( sTargetID.dwID );
		if ( pChar )	return pChar->GetCharSkin();
	}
	else if ( sTargetID.emCrow == CROW_NPC || sTargetID.emCrow == CROW_MOB )
	{
		PGLCROWCLIENT pCrow = m_pLandMClient->GetCrow ( sTargetID.dwID );
		if ( pCrow )	return pCrow->GetCharSkin();
	}
	else if ( sTargetID.emCrow == CROW_MATERIAL )
	{
		PGLMATERIALCLIENT pMaterial = m_pLandMClient->GetMaterial ( sTargetID.dwID );
		if ( pMaterial )	return pMaterial->GetCharSkin();
	}
	else if ( sTargetID.emCrow == CROW_PET )
	{
		if ( GetPetClient()->m_dwGUID == sTargetID.dwID ) return GetPetClient()->GetSkinChar ();

		PGLANYPET pAnyPet = m_pLandMClient->GetPet ( sTargetID.dwID );
		if ( pAnyPet ) return pAnyPet->GetSkinChar ();
	}
	else if ( sTargetID.emCrow == CROW_SUMMON )
	{
		//if ( GetSummonClient()->m_dwGUID == sTargetID.dwID ) return GetSummonClient()->GetSkinChar ();

		/*skill summon, Juver, 2017/10/08 */
		GLSummonClient*	psummon_client = GetSummonClient( sTargetID.dwID );
		if( psummon_client )	return psummon_client->GetSkinChar ();

		PGLANYSUMMON pAnySummon = m_pLandMClient->GetSummon ( sTargetID.dwID );
		if ( pAnySummon ) return pAnySummon->GetSkinChar ();
	}
	else
	{
		//GASSERT(0&&"emCrow�� �߸��� ������ �Դϴ�." );
		CDebugSet::ToLogFile( "GLGaeaClient::GetSkinChar() emCrow�� �߸��� ������ �Դϴ�." );
	}
		

	return NULL;
}

float GLGaeaClient::GetCrowDir ( const STARGETID &sTargetID )
{
	if ( !m_pLandMClient )	return 0.0f;

	//	Note : Ÿ���� ��ġ ������ ������.
	if ( sTargetID.emCrow == CROW_PC )
	{
		if ( GetCharacter()->m_dwGaeaID == sTargetID.dwID )	return GetCharacter()->GetDirection();

		PGLCHARCLIENT pChar = m_pLandMClient->GetChar ( sTargetID.dwID );
		if ( pChar ) return pChar->GetDirection();
	}
	else if ( sTargetID.emCrow == CROW_NPC || sTargetID.emCrow == CROW_MOB )
	{
		PGLCROWCLIENT pCrow = m_pLandMClient->GetCrow ( sTargetID.dwID );
		if ( pCrow ) return pCrow->GetDirection();
	}
	else if ( sTargetID.emCrow == CROW_MATERIAL )
	{
		PGLMATERIALCLIENT pMaterial = m_pLandMClient->GetMaterial ( sTargetID.dwID );
		if ( pMaterial ) return pMaterial->GetDirection();
	}
	else if ( sTargetID.emCrow == CROW_SUMMON )
	{
		//if ( GetSummonClient()->m_dwGUID == sTargetID.dwID ) return GetSummonClient()->GetDirection ();

		/*skill summon, Juver, 2017/10/08 */
		GLSummonClient*	psummon_client = GetSummonClient( sTargetID.dwID );
		if( psummon_client )	return psummon_client->GetDirection ();

		PGLANYSUMMON pAnySummon = m_pLandMClient->GetSummon ( sTargetID.dwID );
		if ( pAnySummon ) return pAnySummon->GetDirection();
	}
	else
		GASSERT(0&&"emCrow�� �߸��� ������ �Դϴ�." );

	return 0.0f;
}

DWORD GLGaeaClient::GetMobNameColor ( DWORD dwGlobID )
{
	DWORD dwCOLOR(CROWCOLOR::MOB_COLOR_GRAY);

	PGLCROWCLIENT pCROW = GetActiveMap()->GetCrow ( dwGlobID );
	if ( !pCROW )	return dwCOLOR;

	if ( pCROW->m_pCrowData->m_emCrow == CROW_NPC )
	{
		dwCOLOR = CROWCOLOR::MOB_COLOR_GRAY;
	}
	else
	{
		WORD wMY_LEVEL = m_Character.GETLEVEL();
		WORD wMOB_LEVEL = pCROW->m_pCrowData->m_wLevel;
		float fEXP_RATE = GLCONST_CHAR::GETEXP_RATE ( wMY_LEVEL, wMOB_LEVEL );

		if ( fEXP_RATE==0 )		dwCOLOR = CROWCOLOR::MOB_COLOR_GRAY;
		else					dwCOLOR = CROWCOLOR::MOB_COLOR_RED;
	}

	return dwCOLOR;
}

BOOL GLGaeaClient::IsMapCollsion( D3DXVECTOR3& vTargetPt, D3DXVECTOR3& vFromPt )
{
	if( !m_pLandMClient )	return FALSE;

	DxLandMan* pLandMan = m_pLandMClient->GetLandMan();
	if( !pLandMan )			return FALSE;

	LPDXFRAME	pFrame(NULL);
	BOOL		bCollision(FALSE);
	D3DXVECTOR3 vCollision( 0.f, 0.f, 0.f );
	pLandMan->IsCollision( vTargetPt, vFromPt, vCollision, bCollision, pFrame, FALSE );

	return bCollision;
}

void GLGaeaClient::PetSkinPackApplyEffect()
{

	if( m_Pet.IsVALID() )
	{
		// ���� ȿ��
		D3DXMATRIX matEffect;
		D3DXVECTOR3 vPos = m_Pet.GetPosition ();
		D3DXMatrixTranslation ( &matEffect, vPos.x, vPos.y, vPos.z );

		std::string strGEN_EFFECT = GLCONST_CHAR::strPET_GEN_EFFECT.c_str();
		STARGETID sTargetID(CROW_PET,m_Pet.m_dwGUID,vPos);
		DxEffGroupPlayer::GetInstance().NewEffGroup( strGEN_EFFECT.c_str(), matEffect, &sTargetID );				
		m_Pet.SkinLoad( m_pd3dDevice );

		// Note : 1.AABB Box�� �����´�. 2.���̸� ����� ���´�.
		m_Pet.GetSkinChar()->GetAABBBox( m_Pet.m_vMaxOrg, m_Pet.m_vMinOrg );
		m_Pet.m_fHeight = ( m_Pet.m_vMaxOrg.y - m_Pet.m_vMinOrg.y ) * m_Pet.m_sPetSkinPackState.fPetScale;

		if( !m_Pet.IsUsePetSkinPack() ) m_Pet.UpdateSuit ( TRUE );
	}

}

HRESULT GLGaeaClient::CreateSummon( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_USE_SUMMON_FB* pNetMsg = ( GLMSG::SNETPC_REQ_USE_SUMMON_FB* ) nmg;

	/*skill summon, Juver, 2017/10/08 */
	GLSummonClient* psummon_client = GetSummonClient( pNetMsg->dwGUID );
	if ( psummon_client )
	{
		CDebugSet::ToFileWithTime( "summon.txt", "%d id exist", pNetMsg->dwGUID );
		return E_FAIL;
	}

	/*skill summon, Juver, 2017/10/08 */
	if ( pNetMsg->wIndex >= SKILL_SUMMON_MAX_CLIENT_NUM )
	{
		CDebugSet::ToFileWithTime( "summon.txt", "summon maxed out" );
		return E_FAIL;
	}

	m_Summon[pNetMsg->wIndex].DeleteDeviceObjects();

	GLSUMMON sSummon;
	sSummon.m_emTYPE	 = pNetMsg->emTYPE;
	sSummon.m_dwGUID	 = pNetMsg->dwGUID;
	sSummon.m_sSummonID  = pNetMsg->sSummonID;
	sSummon.m_dwOwner	 = pNetMsg->dwOwner;	
	sSummon.m_sMapID	 = pNetMsg->sMapID;
	sSummon.m_dwCellID   = pNetMsg->dwCellID;
	sSummon.m_dwNowHP	 = pNetMsg->dwNowHP;
	sSummon.m_wNowMP	 = pNetMsg->wNowMP;

	/*skill summon, Juver, 2017/10/09 */
	sSummon.m_wArrayIndex = pNetMsg->wIndex;		
	sSummon.m_Summon      = pNetMsg->sSummon;

	/*skill summon, Juver, 2017/10/08 */
	HRESULT hr = m_Summon[pNetMsg->wIndex].Create ( &sSummon, pNetMsg->vPos, pNetMsg->vDir, m_pLandMClient->GetNaviMesh(), m_pd3dDevice );

	// ����ó�� ( ������ ������ ���� �����ϵ��� )
	if ( FAILED(hr) )
	{
		return S_FALSE;
	}

	// ���� ȿ��
	D3DXMATRIX matEffect;
	D3DXVECTOR3 vPos = m_Summon[pNetMsg->wIndex].GetPosition ();
	D3DXMatrixTranslation ( &matEffect, vPos.x, vPos.y, vPos.z );

	std::string strGEN_EFFECT = GLCONST_CHAR::strPET_GEN_EFFECT.c_str();
	STARGETID sTargetID(CROW_PET,m_Summon[pNetMsg->wIndex].m_dwGUID,vPos);
	DxEffGroupPlayer::GetInstance().NewEffGroup( strGEN_EFFECT.c_str(), matEffect, &sTargetID );

	return S_OK;

}

HRESULT GLGaeaClient::ReqClubDeathMatchInfo()
{
	PLANDMANCLIENT pLand = GetActiveMap();
	if ( !pLand || !pLand->m_bClubDeathMatch ) return E_FAIL;

	GLMSG::SNET_CLUB_DEATHMATCH_RANKING_REQ NetMsg;
	NetMsg.dwMapID = pLand->GetMapID().dwID;
	NETSENDTOFIELD ( &NetMsg );

	return TRUE;
}

/*character simple, Juver, 2017/10/01 */
void GLGaeaClient::SetCharacterSimple( BOOL bCharacterSimple, BOOL bCharacterMisc )
{
	if ( !m_pLandMClient )	return;

	GLCHARCLIENTLIST *pCharList = m_pLandMClient->GetCharList ();
	GLCHARCLIENTNODE *pCharCur = pCharList->m_pHead;
	for ( ; pCharCur; pCharCur = pCharCur->pNext )
	{
		PGLCHARCLIENT pChar = pCharCur->Data;
		if ( !pChar )	continue;
		pChar->UpdateSuit();
	}
}

/*skill summon, Juver, 2017/10/08 */
GLSummonClient* GLGaeaClient::GetSummonClient( DWORD dwguid )
{
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
	{
		if ( m_Summon[i].m_dwGUID == dwguid )
		{
			return &m_Summon[i];
			break;
		}
	}

	return NULL;
}

DETECTMAP* GLGaeaClient::DetectCrowBot ( DWORD emCrow, const D3DXVECTOR3 &vTarPos, const float fLength )
{
	STARGETID TargetID(CROW_MOB,EMTARGET_NULL);
	if ( !m_pLandMClient )	return NULL;
	
	DxLandMan *pLandMan = GLGaeaClient::GetInstance().GetActiveMap()->GetLandMan();
	if( !pLandMan )			return NULL;

	//	DetectID Ŭ����.
	if ( !m_vecDetectID.empty() )	m_vecDetectID.erase ( m_vecDetectID.begin(), m_vecDetectID.end() );

	STARGETID findTargetID;

	//	��.
	if ( emCrow & CROW_EX_MOB )
	{
		GLCROWCLIENTLIST *pCrowList = m_pLandMClient->GetCrowList ();
		GLCROWCLIENTNODE *pCrowCur = pCrowList->m_pHead;
		for ( ; pCrowCur; pCrowCur = pCrowCur->pNext )
		{
			PGLCROWCLIENT pCrow= pCrowCur->Data;

			if ( !pCrow->IsValidBody() ) continue;

			/*anti shit*/
			if ( pLandMan->IsLayerSimpleCollision(vTarPos, pCrow->GetPosition() ) )	continue;

			float fTarRange = pCrow->GETBODYRADIUS() + fLength;

			BOOL bCol = COLLISION::IsSpherePointCollision ( pCrow->GetPosition(), vTarPos, fTarRange );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = pCrow->m_pCrowData->m_emCrow;
				findTargetID.dwID = pCrow->m_dwGlobID;
				findTargetID.vPos = pCrow->GetPosition ();

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	//	Material
	if ( emCrow & CROW_EX_MATERIAL )
	{
		GLMATERIALCLIENTLIST *pMaterialList = m_pLandMClient->GetMaterialList ();
		GLMATERIALCLIENTNODE *pMaterialCur = pMaterialList->m_pHead;
		for ( ; pMaterialCur; pMaterialCur = pMaterialCur->pNext )
		{
			PGLMATERIALCLIENT pMaterial= pMaterialCur->Data;

			if ( !pMaterial->IsValidBody() ) continue;

			float fTarRange = pMaterial->GetBodyRadius() + fLength;

			BOOL bCol = COLLISION::IsSpherePointCollision ( pMaterial->GetPosition(), vTarPos, fTarRange );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = pMaterial->m_pCrowData->m_emCrow;
				findTargetID.dwID = pMaterial->m_dwGlobID;
				findTargetID.vPos = pMaterial->GetPosition ();

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	//	ĳ����.
	if ( emCrow & CROW_EX_PC )
	{
		GLCHARCLIENTLIST *pCharList = m_pLandMClient->GetCharList ();
		GLCHARCLIENTNODE *pCharCur = pCharList->m_pHead;
		for ( ; pCharCur; pCharCur = pCharCur->pNext )
		{
			PGLCHARCLIENT pChar = pCharCur->Data;
			if ( !pChar->IsVisibleDetect() )		continue;
			//if ( !pChar->IsValidBody() )			continue;

			BOOL bCol = COLLISION::IsSpherePointCollision ( pChar->GetPosition(), vTarPos, fLength );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = CROW_PC;
				findTargetID.dwID = pChar->m_dwGaeaID;
				findTargetID.vPos = pChar->GetPosition ();

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	//	Item.
	if ( emCrow & CROW_EX_ITEM )
	{
		ITEMCLIENTDROPLIST *pItemList = m_pLandMClient->GetItemList();
		ITEMCLIENTDROPNODE *pItemCur = pItemList->m_pHead;
		for ( ; pItemCur; pItemCur = pItemCur->pNext )
		{
			CItemClientDrop &sItemDrop = *pItemCur->Data;

			BOOL bCol = COLLISION::IsSpherePointCollision ( sItemDrop.vPos, vTarPos, fLength );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = CROW_ITEM;
				findTargetID.dwID = sItemDrop.dwGlobID;
				findTargetID.vPos = sItemDrop.vPos;

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	//	Money.
	if ( emCrow & CROW_EX_MONEY )
	{
		MONEYCLIENTDROPLIST *pMoneyList = m_pLandMClient->GetMoneyList();
		MONEYCLIENTDROPNODE *pMoneyCur = pMoneyList->m_pHead;
		for ( ; pMoneyCur; pMoneyCur = pMoneyCur->pNext )
		{
			CMoneyClientDrop &sMoneyDrop = *pMoneyCur->Data;

			BOOL bCol = COLLISION::IsSpherePointCollision ( sMoneyDrop.vPos, vTarPos, fLength );
			if ( bCol )
			{
				//	Note : ����� Ÿ�� ���.
				findTargetID.emCrow = CROW_MONEY;
				findTargetID.dwID = sMoneyDrop.dwGlobID;
				findTargetID.vPos = sMoneyDrop.vPos;

				m_vecDetectID.push_back ( findTargetID );
			}
		}
	}

	if ( m_vecDetectID.empty() )	return NULL;

	std::sort ( m_vecDetectID.begin(), m_vecDetectID.end(), STAR_ORDER() );
	return &m_vecDetectID;
}

/*dmk14 ingame web*/
HRESULT GLGaeaClient::ReqItemMallInfo ()
{
	bool m_bRequest = ( m_vecItemMall.empty() ) ? true : false;

	GLMSG::SNET_GET_ITEMMALL_FROMDB NetMsg;
	NetMsg.bRequest = m_bRequest;
	NETSENDTOFIELD ( &NetMsg );

	return S_OK;
}