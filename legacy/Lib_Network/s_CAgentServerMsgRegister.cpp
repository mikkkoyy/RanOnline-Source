#include "stdafx.h"
#include "s_CAgentServer.h"

#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//set register flag to client 
void CAgentServer::RegisterInit( MSG_LIST* pMsg )
{
	if ( pMsg == NULL) return;	

	if (m_pClientManager->IsAccountPassing( pMsg->dwClient ) == true)
	{
		CConsoleMessage::GetInstance()->Write( _T("ERROR::REGISTER_INIT IsAccountPassing()") );
		return;
	}
	

	NET_REGISTER_INIT* pNmr = NULL;
	DWORD dwClient   = 0;

	dwClient = pMsg->dwClient;

	pNmr = reinterpret_cast<NET_REGISTER_INIT *> (pMsg->Buffer);

	if (sizeof(NET_REGISTER_INIT) != pNmr->nmg.dwSize)
	{
		CConsoleMessage::GetInstance()->Write( _T("ERROR::REGISTER_INIT Wrong Message Size") );
		return;
	}

	int nChannel = pNmr->nChannel;

	if ( nChannel < 0 || nChannel >= m_nServerChannelNumber)
	{
		CConsoleMessage::GetInstance()->Write( _T("ERROR::REGISTER_INIT Invalid Channel Number (Channel:%d)"), nChannel );
		return;
	}


	m_pClientManager->SetRegister( dwClient, pNmr->bInit );

	//this could happen if player close the registration page and player is in the process of registration
	if ( m_pClientManager->IsRegisterWait( dwClient ) )
	{
		m_pClientManager->SetRegisterWait( dwClient, FALSE );
	}
}

void CAgentServer::RegisterAction( MSG_LIST* pMsg )
{
	if ( !RANPARAM::bFeatureRegister )	return;

	NET_REGISTER_ACTION_FB nmfb;

	if ( pMsg == NULL) 
	{
		nmfb.emFB = EMREGISTER_FB_ERROR_SERVER;
		SendClient(pMsg->dwClient, &nmfb);
		return;	
	}
	
	if (m_pClientManager->IsAccountPassing( pMsg->dwClient ) == true)
	{
		CConsoleMessage::GetInstance()->Write( _T("ERROR::REGISTER_ACTION IsAccountPassing()") );
		return;
	}

	if ( !m_pClientManager->IsRegister( pMsg->dwClient ) )
	{
		nmfb.emFB = EMREGISTER_FB_ERROR_NOTREG;
		SendClient(pMsg->dwClient, &nmfb);
		return;
	}

	if ( m_pClientManager->IsRegisterWait( pMsg->dwClient ) )
	{
		nmfb.emFB = EMREGISTER_FB_ERROR_ONREGWAIT;
		SendClient(pMsg->dwClient, &nmfb);
		return;
	}

	NET_REGISTER_ACTION* pNmr = NULL;
	
	DWORD dwClient   = 0;

	dwClient = pMsg->dwClient;

	pNmr = reinterpret_cast<NET_REGISTER_ACTION *> (pMsg->Buffer);

	if (sizeof(NET_REGISTER_ACTION) != pNmr->nmg.dwSize)
	{
		CConsoleMessage::GetInstance()->Write( _T("ERROR::REGISTER_ACTION Wrong Message Size") );
		nmfb.emFB = EMREGISTER_FB_ERROR_SERVER;
		SendClient(dwClient, &nmfb);
		return;
	}

	int nChannel = pNmr->nChannel;

	if ( nChannel < 0 || nChannel >= m_nServerChannelNumber)
	{
		CConsoleMessage::GetInstance()->Write( _T("ERROR::REGISTER_ACTION Invalid Channel Number (Channel:%d)"), nChannel );
		nmfb.emFB = EMREGISTER_FB_ERROR_SERVER;
		SendClient(dwClient, &nmfb);
		return;
	}

	m_Tea.decrypt( pNmr->szUser, USR_ID_LENGTH ); 
	m_Tea.decrypt( pNmr->szMail, USR_INFOMAIL_LENGTH ); 
	m_Tea.decrypt( pNmr->szSA, USR_PASS_LENGTH ); 

	TCHAR szUserid[USR_ID_LENGTH] = {0};
	TCHAR szPass[USR_PASS_LENGTH] = {0};
	TCHAR szPass2[USR_PASS_LENGTH] = {0};
	TCHAR szSA[USR_PASS_LENGTH] = {0};
	TCHAR szMail[USR_INFOMAIL_LENGTH] = {0};
	
	StringCchCopy( szUserid, USR_ID_LENGTH, pNmr->szUser );
	StringCchCopy( szPass, USR_PASS_LENGTH, pNmr->szPass );
	StringCchCopy( szPass2, USR_PASS_LENGTH, pNmr->szPass2 );
	StringCchCopy( szSA, USR_PASS_LENGTH, pNmr->szSA );
	StringCchCopy( szMail, USR_INFOMAIL_LENGTH, pNmr->szMail );

	if ( strlen( szUserid ) < USR_INFO_MIN )
	{
		nmfb.emFB = EMREGISTER_FB_ERROR_LESS_USERID;
		nmfb.nERRORVAR = USR_INFO_MIN;
		SendClient(dwClient, &nmfb);
		return;
	}

	if ( strlen( szPass ) < USR_INFO_MIN )
	{
		nmfb.emFB = EMREGISTER_FB_ERROR_LESS_PASS1;
		nmfb.nERRORVAR = USR_INFO_MIN;
		SendClient(dwClient, &nmfb);
		return;
	}

	if ( strlen( szPass2 ) < USR_INFO_MIN )
	{
		nmfb.emFB = EMREGISTER_FB_ERROR_LESS_PASS2;
		nmfb.nERRORVAR = USR_INFO_MIN;
		SendClient(dwClient, &nmfb);
		return;
	}

	if ( strlen( szSA ) < USR_INFO_MIN )
	{
		nmfb.emFB = EMREGISTER_FB_ERROR_LESS_PASS1;
		nmfb.nERRORVAR = USR_INFO_MIN;
		SendClient(dwClient, &nmfb);
		return;
	}

	if ( strlen( szMail ) < USR_INFO_MIN )
	{
		nmfb.emFB = EMREGISTER_FB_ERROR_LESS_MAIL;
		nmfb.nERRORVAR = USR_INFO_MIN;
		SendClient(dwClient, &nmfb);
		return;
	}


	if ( strlen( szUserid ) > USR_ID_LENGTH )
	{
		nmfb.emFB = EMREGISTER_FB_ERROR_MAX_USERID;
		nmfb.nERRORVAR = USR_ID_LENGTH;
		SendClient(dwClient, &nmfb);
		return;
	}

	if ( strlen( szPass ) > USR_PASS_LENGTH )
	{
		nmfb.emFB = EMREGISTER_FB_ERROR_MAX_PASS1;
		nmfb.nERRORVAR = USR_PASS_LENGTH;
		SendClient(dwClient, &nmfb);
		return;
	}

	if ( strlen( szPass2 ) > USR_PASS_LENGTH )
	{
		nmfb.emFB = EMREGISTER_FB_ERROR_MAX_PASS2;
		nmfb.nERRORVAR = USR_PASS_LENGTH;
		SendClient(dwClient, &nmfb);
		return;
	}

	if ( strlen( szSA ) > USR_PASS_LENGTH )
	{
		nmfb.emFB = EMREGISTER_FB_ERROR_MAX_PASS1;
		nmfb.nERRORVAR = USR_PASS_LENGTH;
		SendClient(dwClient, &nmfb);
		return;
	}

	if ( strlen( szMail ) > USR_INFOMAIL_LENGTH )
	{
		nmfb.emFB = EMREGISTER_FB_ERROR_MAX_MAIL;
		nmfb.nERRORVAR = USR_INFOMAIL_LENGTH;
		SendClient(dwClient, &nmfb);
		return;
	}

	
	{
		CString str = szUserid;
		str.Trim();
		if ( STRUTIL::CheckString( str.GetString() ) )
		{
			nmfb.emFB = EMREGISTER_FB_ERROR_INVALID_USERID;
			SendClient(dwClient, &nmfb);
			return;
		}
	}

	{
		CString str = szPass;
		str.Trim();
		if ( STRUTIL::CheckString( str.GetString() ) )
		{
			nmfb.emFB = EMREGISTER_FB_ERROR_INVALID_PASS1;
			SendClient(dwClient, &nmfb);
			return;
		}
	}

	{
		CString str = szPass2;
		str.Trim();
		if ( STRUTIL::CheckString( str.GetString() ) )
		{
			nmfb.emFB = EMREGISTER_FB_ERROR_INVALID_PASS2;
			SendClient(dwClient, &nmfb);
			return;
		}
	}

	{
		CString str = szSA;
		str.Trim();
		if ( STRUTIL::CheckString( str.GetString() ) )
		{
			nmfb.emFB = EMREGISTER_FB_ERROR_INVALID_SA;
			SendClient(dwClient, &nmfb);
			return;
		}
	}

	
	{
		CString str = szMail;
		str.Trim();
		if ( STRUTIL::CheckString_Special2( str.GetString() ) )
		{
			nmfb.emFB = EMREGISTER_FB_ERROR_INVALID_MAIL;
			SendClient(dwClient, &nmfb);
			return;
		}
	}

	m_pClientManager->SetRegisterWait( dwClient, true );
	m_pClientManager->SetUserID( dwClient, szUserid );

	nmfb.emFB = EMREGISTER_FB_GOOD_WAITING;
	SendClient(dwClient, &nmfb);

	CAgentUserRegister* pAction = new CAgentUserRegister( 
		szUserid,
		szPass, 
		szPass2,
		szSA, 
		szMail,
		m_pClientManager->GetClientIP( dwClient ),
		m_nServerGroup,
		m_nServerNum,
		0,
		dwClient );
	COdbcManager::GetInstance()->AddUserJob((CDbAction*) pAction);

	CDebugSet::ToFileWithTime( "_userreg.txt", "Client ID:%d User:%s Pass:%s Pass2:%s SA:%s Email:%s IP:%s", 
		dwClient, szUserid, szPass, szPass2, szSA, szMail, m_pClientManager->GetClientIP( dwClient ) );
}

void CAgentServer::RegisterFeedback( NET_REGISTER_ACTION_FB2* fb2 )
{
	NET_REGISTER_ACTION_FB nmfb;
	if ( !fb2 )
		return;

	DWORD dwCLIENTID = fb2->nClient;
	BOOL bERROR = FALSE;

	if ( m_pClientManager->IsAccountPassing( dwCLIENTID ) == true)
	{
		CConsoleMessage::GetInstance()->Write( _T("ERROR::RegisterFeedback IsAccountPassing()") );
		bERROR = TRUE;
	}

	if ( !m_pClientManager->IsRegister( dwCLIENTID ) )
		bERROR = TRUE;

	if ( !m_pClientManager->IsRegisterWait(dwCLIENTID ) )
		bERROR = TRUE;

	BOOL bOK = ( (m_pClientManager->IsOnline( dwCLIENTID ) == true) && 
		(strcmp( m_pClientManager->GetClientIP( dwCLIENTID ), fb2->szIp ) == 0) && 
		(strcmp( m_pClientManager->GetUserID( dwCLIENTID ),   fb2->szUserid ) == 0) );

	if ( !bOK )
		bERROR = TRUE;

	if( bERROR )
	{
		m_pClientManager->SetRegisterWait( dwCLIENTID, FALSE );
		m_pClientManager->ResetUserID( dwCLIENTID );
		m_pClientManager->SetRegister( dwCLIENTID, FALSE );
		nmfb.emFB = EMREGISTER_FB_GOOD_ERROR;
		SendClient( dwCLIENTID, &nmfb);
		return;
	}

	switch( fb2->emFB )
	{
	case EMREGISTER_FB2_OK:
		{
			nmfb.emFB = EMREGISTER_FB_GOOD_DONE;
			SendClient( dwCLIENTID, &nmfb);
		}break;
	case EMREGISTER_FB2_TAKEN:
		{
			nmfb.emFB = EMREGISTER_FB_GOOD_TAKEN;
			SendClient( dwCLIENTID, &nmfb);
		}break;
	case EMREGISTER_FB2_ERROR:
		{	
			m_pClientManager->SetRegister( dwCLIENTID, FALSE );
			nmfb.emFB = EMREGISTER_FB_GOOD_ERROR;
			SendClient( dwCLIENTID, &nmfb);
		}break;
	};	

	m_pClientManager->SetRegisterWait( dwCLIENTID, FALSE );
	m_pClientManager->ResetUserID( dwCLIENTID );
}
	