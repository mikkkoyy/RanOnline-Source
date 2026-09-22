#include "StdAfx.h"
#include "s_NetGlobal.h"
#include "GLContrlMsg.h"
#include "GLGaeaClient.h"
#include "DxGlobalStage.h"

#include "RANPARAM.h"
#include "../Lib_ClientUI/Interface/UITextControl.h"
#include "../Lib_ClientUI/Interface/GameTextControl.h"
#include "../Lib_ClientUI/Interface/InnerInterface.h"
#include "../Lib_ClientUI/Interface/PartySearch/PartyRequestModalWindow.h"
#include "../Lib_ClientUI/Interface/QBoxButton.h"
#include "../Lib_ClientUI/Interface/BasicChat.h"
#include "glpartyclient.h"
#include "GLItemMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GLPartyClient& GLPartyClient::GetInstance()
{
	static GLPartyClient Instance;
	return Instance;
}

BOOL GLPARTY_CLIENT::ISONESELF ()
{
	return ( GLGaeaClient::GetInstance().GetCharacter()->m_dwGaeaID == m_dwGaeaID );
}

GLPartyClient::GLPartyClient(void) :
	m_dwPartyID(PARTY_NULL)
{
	m_mapParty.reserve(MAXPARTY);
}

GLPartyClient::~GLPartyClient(void)
{
}

void GLPartyClient::Lure ( DWORD dwGaeaID )
{
	if ( FindMember(dwGaeaID)!=NULL )	return;
	
	SPARTY_OPT sOption;
	sOption.emGET_ITEM = (EMPARTY_ITEM_OPT)RANPARAM::dwPARTY_GET_ITEM;
	sOption.emGET_MONEY = (EMPARTY_MONEY_OPT)RANPARAM::dwPARTY_GET_MONEY;

	//	Note : 파티원 모집 요청.
	//
	GLMSG::SNET_PARTY_LURE NetMsg;
	NetMsg.dwGaeaID = dwGaeaID;
	NetMsg.sOption = sOption;

	NETSEND ( (NET_MSG_GENERIC *) &NetMsg );
}

void GLPartyClient::Secede ( DWORD dwIndex )
{
	GLPARTY_CLIENT* pParty = GetMember(dwIndex);
	if ( !pParty )	return;
	
	DWORD dwMyGaeaID = GLGaeaClient::GetInstance().GetCharacter()->m_dwGaeaID;

	if ( pParty->m_dwGaeaID != dwMyGaeaID && !m_sMaster.ISONESELF() )	return;
	
	//	Note : 탈퇴 ( 혹은 강퇴 ) 요청.
	//
	GLMSG::SNET_PARTY_SECEDE NetMsg;
	NetMsg.dwGaeaID = pParty->m_dwGaeaID;
	NetMsg.dwPartyID = m_dwPartyID;

	NETSEND ( (NET_MSG_GENERIC *) &NetMsg );
}

void GLPartyClient::Dissolve ()
{
	if ( !m_sMaster.ISONESELF() )	return;

	//	Note : 파티 해체.
	//
	GLMSG::SNET_PARTY_DISSOLVE NetMsg;
	NetMsg.dwPartyID = m_dwPartyID;

	NETSEND ( (NET_MSG_GENERIC *) &NetMsg );
}

void GLPartyClient::Authority( DWORD dwIndex )
{
	GLPARTY_CLIENT* pParty = GetMember(dwIndex);
	if ( !pParty )	return;

	DWORD dwMyGaeaID = GLGaeaClient::GetInstance().GetCharacter()->m_dwGaeaID;

	if ( pParty->m_dwGaeaID == dwMyGaeaID || 
		GLGaeaClient::GetInstance().GetCharacter()->m_dwGaeaID != m_sMaster.m_dwGaeaID )	
		return;

	//	Note : 위임 요청.
	//
	GLMSG::SNET_PARTY_AUTHORITY NetMsg;
	NetMsg.dwGaeaID = pParty->m_dwGaeaID;
	NetMsg.dwPartyID = m_dwPartyID;

	NETSEND ( (NET_MSG_GENERIC *) &NetMsg );
}

void GLPartyClient::ResetParty ()
{
	m_dwPartyID = PARTY_NULL;

	m_sMaster.RESET();
	m_mapParty.clear();

	SCONFTING_CLT &sConfting = GLGaeaClient::GetInstance().GetCharacter()->GetConfting();
	if ( sConfting.emTYPE==EMCONFT_PARTY )
	{
		sConfting.RESET();
	}
}

DWORD GLPartyClient::GetMemberNum ()
{
	if ( GetMaster() )
	{
		return DWORD ( 1 + m_mapParty.size() );
	}

	return 0;
}

GLPARTY_CLIENT* GLPartyClient::GetMaster ()
{
	if ( !m_sMaster.VALID() )				return NULL;
	return &m_sMaster;
}

GLPARTY_CLIENT* GLPartyClient::GetMember ( DWORD dwIndex )
{
	if ( dwIndex >= m_mapParty.size() )		return NULL;

	PARTYMAP_ITER iter = m_mapParty.begin();
	for ( DWORD j=0; j<dwIndex; ++j )		++iter;

	return &(*iter).second;
}

GLPARTY_CLIENT* GLPartyClient::FindMember ( DWORD dwGaeaID )
{
	if ( m_sMaster.m_dwGaeaID==dwGaeaID )		return &m_sMaster;

	PARTYMAP_ITER iter = m_mapParty.begin();
	PARTYMAP_ITER iter_end = m_mapParty.end();
	for( ; iter != iter_end; ++iter )
	{
		GLPARTY_CLIENT &sMember = (*iter).second;
		if ( sMember.m_dwGaeaID == dwGaeaID )
		{
			return &sMember;
		}
	}

	return NULL;
}

bool GLPartyClient::IsMember( DWORD dwGaeaID )
{
	if ( m_mapParty.empty() ) return false;
	if ( m_sMaster.m_dwGaeaID==dwGaeaID )		return true;

	PARTYMAP_ITER iter = m_mapParty.begin();
	PARTYMAP_ITER iter_end = m_mapParty.end();
	for( ; iter != iter_end; ++iter )
	{
		GLPARTY_CLIENT &sMember = (*iter).second;
		if ( sMember.m_dwGaeaID == dwGaeaID )
		{
			return true;
		}
	}

	return false;
}

bool GLPartyClient::IsMember( const char* szCharName )
{
	if ( m_mapParty.empty() ) return false;
	if ( !_tcscmp( m_sMaster.m_szName, szCharName ) ) return true;

	PARTYMAP_ITER iter = m_mapParty.begin();
	PARTYMAP_ITER iter_end = m_mapParty.end();
	for( ; iter != iter_end; ++iter )
	{
		GLPARTY_CLIENT &sMember = (*iter).second;
		if ( !_tcscmp( sMember.m_szName, szCharName ) )
		{
			return true;
		}
	}

	return false;
}

void GLPartyClient::DELETEMEMBER ( DWORD dwGaeaID )
{
	PARTYMAP_ITER iter = m_mapParty.begin();
	PARTYMAP_ITER iter_end = m_mapParty.end();
	for( ; iter != iter_end; ++iter )
	{
		GLPARTY_CLIENT &sMember = (*iter).second;
		if ( sMember.m_dwGaeaID == dwGaeaID )
		{
			m_mapParty.erase ( iter );
		}
	}
}

BOOL GLPartyClient::MsgPartyFNew ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_PARTY_FNEW *pNetMsg = ( GLMSG::SNET_PARTY_FNEW * ) nmg;

	ResetParty ();

	m_dwPartyID = pNetMsg->dwPartyID;
	m_sOption = pNetMsg->sOption;

	for ( DWORD i=0; i<pNetMsg->dwPartyNum; ++i )
	{
		GLPARTY_FNET &sPartyNet = pNetMsg->sPartyInfo[i];

		if ( sPartyNet.m_dwGaeaID==pNetMsg->dwMaster )
		{
			m_sMaster.ASSIGN ( sPartyNet );
		}
		else
		{
			GLPARTY_CLIENT sPartyClient;
			sPartyClient.ASSIGN ( sPartyNet );
			m_mapParty.push_back ( PARTYPAIRCLIENT(sPartyClient.m_dwGaeaID,sPartyClient) );
		}
	}

	// 파티를 처음 생성했을 때 QBox옵션을 ON 상태로 바꾼다.
	CInnerInterface::GetInstance().GetQBoxButton()->SetQBoxEnable(TRUE);

	return TRUE;
}

BOOL GLPartyClient::MsgPartyAdd ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_PARTY_ADD *pNetMsg = ( GLMSG::SNET_PARTY_ADD * ) nmg;
	if ( m_dwPartyID != pNetMsg->dwPartyID )	return FALSE;

	if ( FindMember ( pNetMsg->sPartyInfo.m_dwGaeaID ) ) return FALSE;

	GLPARTY_CLIENT sPartyClient;
	sPartyClient.ASSIGN ( pNetMsg->sPartyInfo );
	m_mapParty.push_back ( PARTYPAIRCLIENT(sPartyClient.m_dwGaeaID,sPartyClient) );

	return TRUE;
}

BOOL GLPartyClient::MsgPartyDel ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_PARTY_DEL *pNetMsg = ( GLMSG::SNET_PARTY_DEL * ) nmg;
	if ( m_dwPartyID != pNetMsg->dwPartyID )	return FALSE;

	PARTYMAP_ITER iter = m_mapParty.begin();
	PARTYMAP_ITER iter_end = m_mapParty.end();
	for( ; iter != iter_end; ++iter )
	{
		GLPARTY_CLIENT &sMember = (*iter).second;
		if ( sMember.m_dwGaeaID == pNetMsg->dwDelMember )
		{
			if ( sMember.ISONESELF() )		ResetParty ();				//	자기 자신이 탈퇴 될때.
			else							m_mapParty.erase ( iter );	//	다른 사람이 탈퇴할때.

			return TRUE;
		}
	}
	
	return FALSE;
}

BOOL GLPartyClient::MsgPartyDissolve ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_PARTY_DISSOLVE *pNetMsg = ( GLMSG::SNET_PARTY_DISSOLVE * ) nmg;

	ResetParty ();

	return TRUE;
}

BOOL GLPartyClient::MsgPartyAuthority( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_PARTY_AUTHORITY *pNetMsg = ( GLMSG::SNET_PARTY_AUTHORITY* ) nmg;
	
	if( m_dwPartyID != pNetMsg->dwPartyID )
		return FALSE;

	if( m_sMaster.m_dwGaeaID == pNetMsg->dwGaeaID )
		return FALSE;

	m_mapParty.push_back ( PARTYPAIRCLIENT( m_sMaster.m_dwGaeaID, m_sMaster ) );

	// 기존 마스터 제거
	m_sMaster.RESET();


	// 새로운 마스터를 설정
	GLPARTY_CLIENT* pNewMaster = FindMember ( pNetMsg->dwGaeaID );
	if ( pNewMaster )
	{
		m_sMaster.m_vPos = pNewMaster->m_vPos;
		m_sMaster.m_sHP  = pNewMaster->m_sHP;
		m_sMaster.m_sMP  = pNewMaster->m_sMP;
		m_sMaster.ASSIGN ( GLPARTY_FNET( pNewMaster->m_dwGaeaID, 
			pNewMaster->m_szName, 
			pNewMaster->m_emClass, 
			pNewMaster->m_sMapID,
			pNewMaster->m_wSchool,
			pNewMaster->m_wLevel) );
	}

	DELETEMEMBER( pNetMsg->dwGaeaID );

	// 파티장이 바뀌었을때 QBox옵션을 ON 상태로 바꾼다.
	CInnerInterface::GetInstance().GetQBoxButton()->SetQBoxEnable(TRUE);

	return TRUE;
}

BOOL GLPartyClient::MsgPartyMoveMap ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_PARTY_MBR_MOVEMAP *pNetMsg = ( GLMSG::SNET_PARTY_MBR_MOVEMAP * ) nmg;

	GLPARTY_CLIENT* pParty = FindMember ( pNetMsg->dwGaeaID );
	if ( !pParty )		return FALSE;

	pParty->m_sMapID = pNetMsg->sMapID;

	return TRUE;
}

BOOL GLPartyClient::MsgPartyMbrDetail ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_PARTY_MBR_DETAIL *pNetMsg = ( GLMSG::SNET_PARTY_MBR_DETAIL * ) nmg;

	GLPARTY_CLIENT* pMember = FindMember ( pNetMsg->dwGaeaID );
	if ( !pMember )		return FALSE;
	
	pMember->m_vPos = pNetMsg->vPos;
	pMember->m_sHP = pNetMsg->sHP;
	pMember->m_sMP = pNetMsg->sMP;
	pMember->m_wLevel = pNetMsg->m_wLevel;

	return TRUE;
}
/*PartySearch - Added By Bubu*/
BOOL GLPartyClient::MsgPartyLureFb ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_PARTY_LURE_FB *pNetMsg = ( GLMSG::SNET_PARTY_LURE_FB * ) nmg;
	/*PartySearch - Added By Bubu*/
	CString strNAME;

	PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap (); 
	PGLCHARCLIENT pChar = pLand->GetChar ( pNetMsg->dwGaeaID );
	if ( pChar )	strNAME = pChar->GetName();


	switch ( pNetMsg->emAnswer )
	{
	case EMPARTY_LURE_REFUSE:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("PARTY_LURE_REFUSE"), strNAME.GetString() );
		break;
	
	case EMPARTY_LURE_FULL:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("PARTY_LURE_FULL"), strNAME.GetString() );
		break;

	case EMPARTY_LURE_OTHER:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("PARTY_LURE_OTHER"), strNAME.GetString() );
		break;

	case EMPARTY_LURE_OTHERSCHOOL:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMPARTY_LURE_OTHERSCHOOL") );
		break;
	case EMPARTY_LURE_CLUBBATTLE:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMPARTY_LURE_CLUBBATTLE") );
		break;

		/*pvp capture the flag, Juver, 2018/02/03 */
	case EMPARTY_LURE_OTHER_TEAM:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMPARTY_LURE_OTHER_TEAM") );
		break;
	};

	return TRUE;
}

BOOL GLPartyClient::MsgPartyMbrPoint ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_PARTY_MBR_POINT *pNetMsg = (GLMSG::SNET_PARTY_MBR_POINT *) nmg;

	GLPARTY_CLIENT* pMember = FindMember ( pNetMsg->dwGaeaID );
	if ( !pMember )		return FALSE;
	
	pMember->m_sHP = pNetMsg->sHP;
	pMember->m_sMP = pNetMsg->sMP;
	pMember->m_wLevel = pNetMsg->m_wLevel;

	return TRUE;
}

BOOL GLPartyClient::MsgPartyMbrPos ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_PARTY_MBR_POS *pNetMsg = (GLMSG::SNET_PARTY_MBR_POS *) nmg;

	GLPARTY_CLIENT* pMember = FindMember ( pNetMsg->dwGaeaID );
	if ( !pMember )		return FALSE;
	
	pMember->m_vPos = pNetMsg->vPos;
	pMember->m_sHP = pNetMsg->sHP;
	pMember->m_sMP = pNetMsg->sMP;
	pMember->m_wLevel = pNetMsg->m_wLevel;

	return TRUE;
}

BOOL GLPartyClient::MsgPartyMbrItemPickUp ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_PARTY_MBR_PICKUP_BRD *pNetMsg = (GLMSG::SNETPC_PARTY_MBR_PICKUP_BRD *) nmg;

	GLPARTY_CLIENT *pMember = FindMember ( pNetMsg->dwGaeaID );
	if ( !pMember )		return FALSE;

	SITEM *pITEM = GLItemMan::GetInstance().GetItem ( pNetMsg->sNID_ITEM );
	if ( !pITEM )		return FALSE;

	SITEM *pITEML = GLItemMan::GetInstance().GetItem ( pNetMsg->sITEMCUSTOM.sNativeID );
	if ( !pITEML )	
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::LIGHTSKYBLUE, ID2GAMEINTEXT("PARTY_MBR_PICKUP_BRD"),
		pMember->m_szName, pITEM->GetName() );
	}
	else
	{
		if ( pITEML && pITEML->sBasicOp.emLevel >= LEVEL_RARE )
		{

			SITEMLINK sLINK;
			SITEMCUSTOM sITEMCUSTOM;
			sITEMCUSTOM = pNetMsg->sITEMCUSTOM;
			sITEMCUSTOM.Convert( sLINK );
			
			BOOL bWrapped(FALSE);
			std::string strorigname;
			if ( pITEML && pITEML->sBasicOp.emItemType == ITEM_WRAPPER_BOX && sITEMCUSTOM.nidDISGUISE != NATIVEID_NULL() )
			{
				strorigname = pITEML->GetName();
				bWrapped = TRUE;
			}
		
			if ( bWrapped )
			{
				pITEML = NULL;
				pITEML = GLItemMan::GetInstance().GetItem( sITEMCUSTOM.nidDISGUISE );
				sITEMCUSTOM.sNativeID = sITEMCUSTOM.nidDISGUISE;
				sITEMCUSTOM.nidDISGUISE = NATIVEID_NULL();
				sITEMCUSTOM.tDISGUISE = 0;
			}

			if ( bWrapped ) sLINK.BuildFilter( strorigname.c_str(),sITEMCUSTOM.GETGRADE(EMGRINDING_NONE),bWrapped,pITEML->GetName() );
			else sLINK.BuildFilter( pITEML->GetName(),sITEMCUSTOM.GETGRADE(EMGRINDING_NONE),FALSE, "" );

			sLINK.bUSE = true;
			
			SITEMLINK* pLINK = new SITEMLINK;
			pLINK = &sLINK;
			if ( pLINK )
			{
				pLINK->bUSE = true;
				CBasicChat* pChat = CInnerInterface::GetInstance().GetChat();
				if ( pChat )
				{
					CString strNew;
					strNew.Format(ID2GAMEINTEXT("PARTY_MBR_PICKUP_LINK_BRD"),pMember->m_szName,pLINK->GetFilter());
					pChat->AddStringToChatEx(strNew,4,pLINK);
				}
			}
		}
		else
		{
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::LIGHTSKYBLUE, ID2GAMEINTEXT("PARTY_MBR_PICKUP_BRD"),
			pMember->m_szName, pITEM->GetName() );
		}
	}

	return TRUE;
}

BOOL GLPartyClient::MsgPartyMbrRename ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_PARTY_MEMBER_RENAME_CLT *pNetMsg = ( GLMSG::SNET_PARTY_MEMBER_RENAME_CLT * ) nmg;

	GLPARTY_CLIENT* pMember = FindMember ( pNetMsg->dwGaeaID );
	if ( !pMember )		return FALSE;

	StringCchCopy ( pMember->m_szName, CHAR_SZNAME, pNetMsg->szName );

	return TRUE;
}

BOOL GLPartyClient::MsgPartyMasterRenew ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_PARTY_MASTER_RENEW* pNetMsg = ( GLMSG::SNET_PARTY_MASTER_RENEW* ) nmg;

	// 기존 마스터 제거
	m_sMaster.RESET();

	// 새로운 마스터를 설정
	GLPARTY_CLIENT* pNewMaster = FindMember ( pNetMsg->dwNewMaster );
	if ( pNewMaster )
	{
		m_sMaster.ASSIGN ( GLPARTY_FNET( pNewMaster->m_dwGaeaID, 
										 pNewMaster->m_szName, 
										 pNewMaster->m_emClass, 
										 pNewMaster->m_sMapID,
										 pNewMaster->m_wSchool,
										 pNewMaster->m_wLevel) );
	}

	// 기존 마스터를 맵버목록에서 삭제
	DELETEMEMBER ( pNetMsg->dwNewMaster );

	// 파티장이 바뀌었을때 QBox옵션을 ON 상태로 바꾼다.
	CInnerInterface::GetInstance().GetQBoxButton()->SetQBoxEnable(TRUE);

	return FALSE;
}
/*PartySearch - Added By Bubu*/
BOOL GLPartyClient::MsgPartySearchLureRequestorFb ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_PARTY_SEARCH_LURE_REQUESTOR_FB *pNetMsg = ( GLMSG::SNET_PARTY_SEARCH_LURE_REQUESTOR_FB * ) nmg;

	std::string strNAME;
	std::string strMasterNAME;

	PLANDMANCLIENT pLand = GLGaeaClient::GetInstance().GetActiveMap (); 
	PGLCHARCLIENT pMyChar = GLGaeaClient::GetInstance().GetChar ( pNetMsg->dwMyGaeaID );
	PGLCHARCLIENT pMasterChar = GLGaeaClient::GetInstance().GetChar ( pNetMsg->dwMasterGaeaID );

	if ( pMyChar )	strNAME = pMyChar->GetName();
	strMasterNAME = pNetMsg->m_szName;

	switch ( pNetMsg->emAnswer )
	{
	case EMPARTY_LURE_REFUSE:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("PARTY_LURE_REFUSE"), strMasterNAME.c_str() );
		break;

	case EMPARTY_LURE_FULL:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("PARTY_LURE_FULL"), strMasterNAME.c_str() );
		break;

	case EMPARTY_LURE_OTHER:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("PARTY_LURE_OTHER"), strNAME.c_str() );
		break;

	case EMPARTY_LURE_OTHERSCHOOL:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMPARTY_LURE_OTHERSCHOOL") );
		break;
	case EMPARTY_LURE_CLUBBATTLE:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMPARTY_LURE_CLUBBATTLE") );
		break;

		/*pvp capture the flag, Juver, 2018/02/03 */
	case EMPARTY_LURE_OTHER_TEAM:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMPARTY_LURE_OTHER_TEAM") );
		break;
	case EMPARTY_LURE_NOT_AVAILABLE:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMPARTY_LURE_NOT_AVAILABLE") );
		break;
	case EMPARTY_LURE_ALREADY_HAVE:
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMPARTY_LURE_ALREADY_HAVE") );
		break;
		
		
	};

	return TRUE;
}

BOOL GLPartyClient::MsgPartySearchLureFb ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_PARTY_SEARCH_LURE_FB *pNetMsg = ( GLMSG::SNET_PARTY_SEARCH_LURE_FB * ) nmg;

	std::string strNAME;
	WORD wLevel = pNetMsg->m_wLevel;
	CString strClass = COMMENT::CHARCLASS[CharClassToIndex( pNetMsg->m_emClass)].c_str();
	CString strSchool = ID2GAMEWORD("ACADEMY_NAME",pNetMsg->m_wSchool);

	PGLCHARCLIENT pChar = GLGaeaClient::GetInstance().GetChar ( pNetMsg->dwGaeaID );
	if ( pChar )	strNAME = pChar->GetName();

	BOOL bAvailable(TRUE);
	if ( RANPARAM::bDIS_PARTY || CInnerInterface::GetInstance().IsVisibleGroup( PARTYREQUEST_MODAL ) )
	{
		bAvailable = FALSE;
		GLMSG::SNET_PARTY_SEARCH_LURE_TAR_ANS NetMsg;
		NetMsg.dwRequestorID = pNetMsg->dwGaeaID;
		NetMsg.dwPartyID = PARTY_NULL;
		NetMsg.emAnswer = EMPARTY_LURE_REFUSE;
		NETSEND ( &NetMsg );

		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("PARTY_LURE_REFUSE_AUTO"), pNetMsg->m_szName );
		CInnerInterface::GetInstance().SetPartyRequestModalClose();
		return FALSE;
		
	}
	if ( bAvailable )
	{
		CInnerInterface::GetInstance().GetPartyReqModalWindow()->PartyRequestSet(pNetMsg->m_szName,pNetMsg->dwGaeaID,wLevel,pNetMsg->m_wSchool,pNetMsg->m_emClass);
		CInnerInterface::GetInstance().SetPartyRequestModalOpen(TRUE);
	}

	return TRUE;
}

HRESULT GLPartyClient::MsgProcess ( NET_MSG_GENERIC* nmg )
{
	switch ( nmg->nType )
	{
	case NET_MSG_GCTRL_PARTY_FNEW:
		MsgPartyFNew ( nmg );
		break;

	case NET_MSG_GCTRL_PARTY_LURE_FB:
		MsgPartyLureFb ( nmg );
		break;

	case NET_MSG_GCTRL_PARTY_ADD:
		MsgPartyAdd ( nmg );
		break;

	case NET_MSG_GCTRL_PARTY_DEL:
		MsgPartyDel ( nmg );
		break;

	case NET_MSG_GCTRL_PARTY_AUTHORITY:
		MsgPartyAuthority( nmg );
		break;
	
	case NET_MSG_GCTRL_PARTY_DISSOLVE:
		MsgPartyDissolve ( nmg );
		break;

	case NET_MSG_GCTRL_PARTY_MBR_MOVEMAP:
		MsgPartyMoveMap ( nmg );
		break;

	case NET_MSG_GCTRL_PARTY_MBR_DETAIL:
		MsgPartyMbrDetail ( nmg );
		break;

	case NET_MSG_GCTRL_PARTY_MBR_POINT:
		MsgPartyMbrPoint ( nmg );
		break;

	case NET_MSG_GCTRL_PARTY_MBR_POS:
		MsgPartyMbrPos ( nmg );
		break;
	
	case NET_MSG_GCTRL_PARTY_MBR_PICKUP_BRD:
		MsgPartyMbrItemPickUp ( nmg );
		break;

	case NET_MSG_GCTRL_PARTY_MBR_RENAME_CLT:
		MsgPartyMbrRename ( nmg );
		break;
	case NET_MSG_GCTRL_PARTY_MASTER_RENEW:
		MsgPartyMasterRenew ( nmg );
		break;
	/*PartySearch - Added By Bubu*/
	case NET_MSG_GCTRL_PARTY_SEARCH_LURE_REQUESTOR_FB:
		MsgPartySearchLureRequestorFb ( nmg );
		break;
	case NET_MSG_GCTRL_PARTY_SEARCH_LURE_FB:
		MsgPartySearchLureFb ( nmg );
		break;
	
	};

	return S_OK;
}

BOOL GLPartyClient::SetConfront ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_CONFRONTPTY_START2_CLT *pNetMsg = (GLMSG::SNETPC_CONFRONTPTY_START2_CLT *) nmg;
	
	m_sMaster.m_bConfront = false;
	PARTYMAP_ITER iter = m_mapParty.begin();
	PARTYMAP_ITER iter_end = m_mapParty.end();
	for ( ; iter!=iter_end; ++iter )
	{
		GLPARTY_CLIENT &sMember = (*iter).second;
		sMember.m_bConfront = false;
	}

	int i(0);
	for ( i=0; i<MAXPARTY; ++i )
	{
		DWORD dwMEMBER = pNetMsg->dwCONFRONT_MY_MEMBER[i];
		GLPARTY_CLIENT* pMember = FindMember(dwMEMBER);
		if ( !pMember )		continue;

		pMember->m_bConfront = true;
	}

	return TRUE;
}

DWORD GLPartyClient::GetConfrontNum ()
{
	DWORD dwNum(0);

	if ( m_sMaster.m_bConfront )			dwNum++;

	PARTYMAP_ITER iter = m_mapParty.begin();
	PARTYMAP_ITER iter_end = m_mapParty.end();
	for ( ; iter!=iter_end; ++iter )
	{
		GLPARTY_CLIENT &sMember = (*iter).second;
		if ( sMember.m_bConfront )	dwNum++;;
	}

	return dwNum;
}

void GLPartyClient::ReSetConfront ()
{
	m_sMaster.m_bConfront = false;
	PARTYMAP_ITER iter = m_mapParty.begin();
	PARTYMAP_ITER iter_end = m_mapParty.end();
	for ( ; iter!=iter_end; ++iter )
	{
		GLPARTY_CLIENT &sMember = (*iter).second;
		sMember.m_bConfront = false;
	}
}

void GLPartyClient::ResetConfrontMember ( DWORD dwGaeaID )
{
	GLPARTY_CLIENT* pMember = FindMember(dwGaeaID);
	if ( pMember )	pMember->m_bConfront = false;

	return;
}