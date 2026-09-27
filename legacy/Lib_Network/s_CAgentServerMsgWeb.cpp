/*dmk14 ingame web*/

#include "stdafx.h"
#include "s_CAgentServer.h"

#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CAgentServer::MsgForgotPass(MSG_LIST* pMsg)
{		
	if ( (m_nServiceProvider != SP_CHINA && m_nServiceProvider != SP_TAIWAN ) || pMsg == NULL) return;	

	if (m_pClientManager->IsAccountPassing( pMsg->dwClient ) == true)
	{
		return;
	}
	else
	{
		m_pClientManager->SetAccountPassing( pMsg->dwClient, true );
	}

	m_pClientManager->ResetUserID( pMsg->dwClient );

	NET_FORGOTPASS_DATA* pNml = NULL;
	DWORD dwFlags    = 0;
	DWORD dwSndBytes = 0;
	DWORD dwClient   = 0;

	dwClient = pMsg->dwClient;

	pNml = reinterpret_cast<NET_FORGOTPASS_DATA *> (pMsg->Buffer);

	if (sizeof(NET_FORGOTPASS_DATA) != pNml->nmg.dwSize)
	{
		CConsoleMessage::GetInstance()->Write(
			
			_T("ERROR:NET_FORGOTPASS_DATA Wrong Message Size") );
		return;
	}

	m_Tea.decrypt( pNml->szUserid, USR_ID_LENGTH+1 );
	m_Tea.decrypt( pNml->szPassword, USR_PASS_LENGTH );


	TCHAR szUserid[USR_ID_LENGTH+1] = {0};
	StringCchCopy( szUserid, USR_ID_LENGTH+1, pNml->szUserid );

	if( STRUTIL::CheckString( pNml->szUserid ) ) 
	{
		CConsoleMessage::GetInstance()->Write(
			_T("ERROR:NET_FORGOTPASS_DATA Wrong User input") );
		return;
	}

	m_pClientManager->SetUserID( dwClient, szUserid );

	CAgentUserForgotPass* pAction = new CAgentUserForgotPass(
		pNml->szUserid, 
		pNml->szPassword,
		dwClient );
	
	COdbcManager::GetInstance()->AddUserJob( (CDbAction*) pAction );
}

void CAgentServer::MsgForgotPassBack (NET_FORGOTPASS_FEEDBACK_DATA2* nlfd2)
{
	NET_FORGOTPASS_FEEDBACK_DATA		nlfd;
	nlfd.nResult = nlfd2->nResult;

	DWORD	dwClient   = (DWORD) nlfd2->nClient;
	SendClient( dwClient, &nlfd );
}

void CAgentServer::MsgForgotChangePass(MSG_LIST* pMsg)
{		
	if ( (m_nServiceProvider != SP_CHINA && m_nServiceProvider != SP_TAIWAN ) || pMsg == NULL) return;	

	NET_CHANGEPASS_DATA* pNml = NULL;
	DWORD dwFlags    = 0;
	DWORD dwSndBytes = 0;
	DWORD dwClient   = 0;

	dwClient = pMsg->dwClient;

	pNml = reinterpret_cast<NET_CHANGEPASS_DATA *> (pMsg->Buffer);

	if (sizeof(NET_CHANGEPASS_DATA) != pNml->nmg.dwSize)
	{
		CConsoleMessage::GetInstance()->Write(
			
			_T("ERROR:NET_CHANGEPASS_DATA Wrong Message Size") );
		return;
	}

	m_Tea.decrypt( pNml->szPassword, USR_PASS_LENGTH );
	m_Tea.decrypt( pNml->szPassword2, USR_PASS_LENGTH );

	CAgentUserForgotChangePass* pAction = new CAgentUserForgotChangePass(
		m_pClientManager->GetUserID(dwClient),
		pNml->szPassword, 
		pNml->szPassword2,
		dwClient );
	
	COdbcManager::GetInstance()->AddUserJob( (CDbAction*) pAction );
}

void CAgentServer::MsgForgotChangePassBack (NET_CHANGEPASS_FEEDBACK_DATA2* nlfd2)
{
	NET_CHANGEPASS_FEEDBACK_DATA		nlfd;
	nlfd.nResult = nlfd2->nResult;

	DWORD	dwClient   = (DWORD) nlfd2->nClient;
	SendClient( dwClient, &nlfd );
}

void CAgentServer::MsgSelcharChangePass(MSG_LIST* pMsg)
{		
	if ( (m_nServiceProvider != SP_CHINA && m_nServiceProvider != SP_TAIWAN ) || pMsg == NULL) return;	

	NET_SELCHAR_CHANGEPASS* pNml = NULL;
	DWORD dwFlags    = 0;
	DWORD dwSndBytes = 0;
	DWORD dwClient   = 0;

	dwClient = pMsg->dwClient;

	pNml = reinterpret_cast<NET_SELCHAR_CHANGEPASS *> (pMsg->Buffer);

	if (sizeof(NET_SELCHAR_CHANGEPASS) != pNml->nmg.dwSize)
	{
		CConsoleMessage::GetInstance()->Write(
			
			_T("ERROR:NET_SELCHAR_CHANGEPASS Wrong Message Size") );
		return;
	}

	m_Tea.decrypt( pNml->szPassword, USR_PASS_LENGTH );
	m_Tea.decrypt( pNml->szPincode, USR_PASS_LENGTH );
	m_Tea.decrypt( pNml->szNewPassword, USR_PASS_LENGTH );

	CAgentUserSelcharChangePass* pAction = new CAgentUserSelcharChangePass(
		m_pClientManager->GetUserID(dwClient),
		pNml->szPassword, 
		pNml->szPincode,
		pNml->szNewPassword, 
		dwClient );
	
	COdbcManager::GetInstance()->AddUserJob( (CDbAction*) pAction );
}

void CAgentServer::MsgSelcharChangePassBack (NET_SELCHAR_CHANGEPASS_FEEDBACK2* nlfd2)
{
	NET_SELCHAR_CHANGEPASS_FEEDBACK		nlfd;
	nlfd.nResult = nlfd2->nResult;

	DWORD	dwClient   = (DWORD) nlfd2->nClient;
	SendClient( dwClient, &nlfd );
}

void CAgentServer::MsgSelcharChangePin(MSG_LIST* pMsg)
{		
	if ( (m_nServiceProvider != SP_CHINA && m_nServiceProvider != SP_TAIWAN ) || pMsg == NULL) return;	

	NET_SELCHAR_CHANGEPIN* pNml = NULL;
	DWORD dwFlags    = 0;
	DWORD dwSndBytes = 0;
	DWORD dwClient   = 0;

	dwClient = pMsg->dwClient;

	pNml = reinterpret_cast<NET_SELCHAR_CHANGEPIN *> (pMsg->Buffer);

	if (sizeof(NET_SELCHAR_CHANGEPIN) != pNml->nmg.dwSize)
	{
		CConsoleMessage::GetInstance()->Write(
			
			_T("ERROR:NET_SELCHAR_CHANGEPIN Wrong Message Size") );
		return;
	}

	m_Tea.decrypt( pNml->szPassword, USR_PASS_LENGTH );
	m_Tea.decrypt( pNml->szPincode, USR_PASS_LENGTH );
	m_Tea.decrypt( pNml->szNewPincode, USR_PASS_LENGTH );

	CAgentUserSelcharChangePin* pAction = new CAgentUserSelcharChangePin(
		m_pClientManager->GetUserID(dwClient),
		pNml->szPassword, 
		pNml->szPincode,
		pNml->szNewPincode, 
		dwClient );
	
	COdbcManager::GetInstance()->AddUserJob( (CDbAction*) pAction );
}

void CAgentServer::MsgSelcharChangePinBack (NET_SELCHAR_CHANGEPIN_FEEDBACK2* nlfd2)
{
	NET_SELCHAR_CHANGEPIN_FEEDBACK		nlfd;
	nlfd.nResult = nlfd2->nResult;

	DWORD	dwClient   = (DWORD) nlfd2->nClient;
	SendClient( dwClient, &nlfd );
}

void CAgentServer::MsgReqGameTimeConvert(MSG_LIST* pMsg)
{		
	if ( (m_nServiceProvider != SP_CHINA && m_nServiceProvider != SP_TAIWAN ) || pMsg == NULL) return;	

	NET_REQ_GAMETIME_CONVERT* pNml = NULL;
	DWORD dwFlags    = 0;
	DWORD dwSndBytes = 0;
	DWORD dwClient   = 0;

	dwClient = pMsg->dwClient;

	pNml = reinterpret_cast<NET_REQ_GAMETIME_CONVERT *> (pMsg->Buffer);

	if (sizeof(NET_REQ_GAMETIME_CONVERT) != pNml->nmg.dwSize)
	{
		CConsoleMessage::GetInstance()->Write(
			
			_T("ERROR:NET_REQ_GAMETIME_CONVERT Wrong Message Size") );
		return;
	}

	CAgentReqGameTimeConvert* pAction = new CAgentReqGameTimeConvert(m_pClientManager->GetUserID(dwClient),dwClient);
	COdbcManager::GetInstance()->AddUserJob( (CDbAction*) pAction );
}

void CAgentServer::MsgReqGameTimeConvertBack (NET_REQ_GAMETIME_CONVERT_FEEDBACK2* nlfd2)
{
	NET_REQ_GAMETIME_CONVERT_FEEDBACK		nlfd;
	nlfd.nResult = nlfd2->nResult;
	nlfd.dwPoint = nlfd2->dwPoint;

	DWORD	dwClient   = (DWORD) nlfd2->nClient;
	SendClient( dwClient, &nlfd );
}

void CAgentServer::MsgSelcharResetPin(MSG_LIST* pMsg)
{		
	if ( (m_nServiceProvider != SP_CHINA && m_nServiceProvider != SP_TAIWAN ) || pMsg == NULL) return;	

	NET_SELCHAR_RESETPIN* pNml = NULL;
	DWORD dwFlags    = 0;
	DWORD dwSndBytes = 0;
	DWORD dwClient   = 0;

	dwClient = pMsg->dwClient;

	pNml = reinterpret_cast<NET_SELCHAR_RESETPIN *> (pMsg->Buffer);

	if (sizeof(NET_SELCHAR_RESETPIN) != pNml->nmg.dwSize)
	{
		CConsoleMessage::GetInstance()->Write(
			
			_T("ERROR:NET_SELCHAR_RESETPIN Wrong Message Size") );
		return;
	}

	m_Tea.decrypt( pNml->szPassword, USR_PASS_LENGTH );
	m_Tea.decrypt( pNml->szEmail, USR_INFOMAIL_LENGTH );


	CAgentUserSelcharResetPin* pAction = new CAgentUserSelcharResetPin(
		m_pClientManager->GetUserID(dwClient),
		pNml->szPassword, 
		pNml->szEmail,
		dwClient );
	
	COdbcManager::GetInstance()->AddUserJob( (CDbAction*) pAction );
}

void CAgentServer::MsgSelcharResetPinBack (NET_SELCHAR_RESETPIN_FEEDBACK2* nlfd2)
{
	NET_SELCHAR_RESETPIN_FEEDBACK		nlfd;
	nlfd.nResult = nlfd2->nResult;
	StringCchCopy( nlfd.szNewPin, USR_PASS_LENGTH, nlfd2->szNewPin );

	DWORD	dwClient   = (DWORD) nlfd2->nClient;
	SendClient( dwClient, &nlfd );
}

void CAgentServer::MsgSelcharTopUp( MSG_LIST* pMsg )
{
	if ( (m_nServiceProvider != SP_CHINA && m_nServiceProvider != SP_TAIWAN ) || pMsg == NULL) return;	

	NET_SELCHAR_TOPUP* pNml = NULL;
	DWORD dwFlags    = 0;
	DWORD dwSndBytes = 0;
	DWORD dwClient   = 0;

	dwClient = pMsg->dwClient;

	pNml = reinterpret_cast<NET_SELCHAR_TOPUP *> (pMsg->Buffer);

	if (sizeof( NET_SELCHAR_TOPUP) != pNml->nmg.dwSize)
	{
		CConsoleMessage::GetInstance()->Write(
			
			_T("ERROR:NET_SELCHAR_TOPUP Wrong Message Size") );
		return;
	}

	m_Tea.decrypt( pNml->szCode, TOPUP_CODE_LENGTH );
	m_Tea.decrypt( pNml->szPin, TOPUP_PIN_LENGTH );

	CAgentUserSelcharTopUp* pAction = new CAgentUserSelcharTopUp(
		m_pClientManager->GetUserID(dwClient),
		pNml->szCode, 
		pNml->szPin,
		dwClient );
	
	COdbcManager::GetInstance()->AddUserJob( (CDbAction*) pAction );
}

void CAgentServer::MsgSelcharTopUpBack( NET_SELCHAR_TOPUP_FEEDBACK2* nlfd2 )
{
	NET_SELCHAR_TOPUP_FEEDBACK		nlfd;
	nlfd.nResult = nlfd2->nResult;
	nlfd.dwValue = nlfd2->dwValue;

	DWORD	dwClient   = (DWORD) nlfd2->nClient;
	SendClient( dwClient, &nlfd );
}

void CAgentServer::MsgSelcharChangeMail( MSG_LIST* pMsg )
{
	if ( (m_nServiceProvider != SP_CHINA && m_nServiceProvider != SP_TAIWAN ) || pMsg == NULL) return;	

	NET_SELCHAR_CHANGEMAIL* pNml = NULL;
	DWORD dwFlags    = 0;
	DWORD dwSndBytes = 0;
	DWORD dwClient   = 0;

	dwClient = pMsg->dwClient;

	pNml = reinterpret_cast<NET_SELCHAR_CHANGEMAIL *> (pMsg->Buffer);

	if (sizeof(NET_SELCHAR_CHANGEMAIL) != pNml->nmg.dwSize)
	{
		CConsoleMessage::GetInstance()->Write(
			
			_T("ERROR:NET_SELCHAR_CHANGEMAIL Wrong Message Size") );
		return;
	}

	m_Tea.decrypt( pNml->szPassword, USR_PASS_LENGTH );
	m_Tea.decrypt( pNml->szPincode, USR_PASS_LENGTH );
	m_Tea.decrypt( pNml->szEmailOld, USR_INFOMAIL_LENGTH );
	m_Tea.decrypt( pNml->szEmailNew, USR_INFOMAIL_LENGTH );

	CAgentUserSelcharChangeMail* pAction = new CAgentUserSelcharChangeMail(
		m_pClientManager->GetUserID(dwClient),
		pNml->szPassword, 
		pNml->szPincode,
		pNml->szEmailOld,
		pNml->szEmailNew,
		dwClient );
	
	COdbcManager::GetInstance()->AddUserJob( (CDbAction*) pAction );
}

void CAgentServer::MsgSelcharChangeMailBack( NET_SELCHAR_CHANGEMAIL_FEEDBACK2* nlfd2 )
{
	NET_SELCHAR_CHANGEMAIL_FEEDBACK		nlfd;
	nlfd.nResult = nlfd2->nResult;

	DWORD	dwClient   = (DWORD) nlfd2->nClient;
	SendClient( dwClient, &nlfd );
}