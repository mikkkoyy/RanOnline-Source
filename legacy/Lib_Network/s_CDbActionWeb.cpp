/*dmk14 ingame web*/
#include "stdafx.h"

#include "s_CDbAction.h"
#include "s_CSessionServer.h"
#include "s_CFieldServer.h"
#include "s_CAgentServer.h"

#include "../Lib_Client/G-Logic/GLCharData.h"

#include "china_md5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAgentUserForgotPass::CAgentUserForgotPass(
	const char* szUserID,
	const char* szPasswd, 
	DWORD dwClient )
	: m_dwClientNum( dwClient )
{	
	memset(m_strUserID, 0, sizeof(char) * USR_ID_LENGTH+1);
	memset(m_strPasswd, 0, sizeof(char) * USR_PASS_LENGTH);

	if (szUserID != NULL)
		StringCchCopy(m_strUserID, USR_ID_LENGTH+1, szUserID);
	if (szPasswd != NULL)
		StringCchCopy(m_strPasswd, USR_PASS_LENGTH, szPasswd);
}

int CAgentUserForgotPass::Execute(CServer* pServer)
{
	CAgentServer* pTemp = reinterpret_cast<CAgentServer*> (pServer);

	if (pTemp == NULL) return NET_ERROR;

	int nRetCode = 0;	
	int nResult = COdbcManager::GetInstance()->UserForgotPass(m_strUserID, m_strPasswd );

	NET_FORGOTPASS_FEEDBACK_DATA2 NetMsgFB;
	NetMsgFB.nClient	= m_dwClientNum;

	switch (nResult)
	{
	case DB_ERROR :
		NetMsgFB.nResult = EM_FORGOTPASS_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserForgotPass result %d", nResult);
		break;
	case 1 :	//second pass not success
		NetMsgFB.nResult = EM_FORGOTPASS_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserForgotPass result %d", nResult);
		break;
	case 2 :	//Successfull reset pass
		NetMsgFB.nResult = EM_FORGOTPASS_FB_SUB_SUCCESS;
		CConsoleMessage::GetInstance()->Write("CAgentUserForgotPass result %d", nResult);
		break;
	default : 
		NetMsgFB.nResult = EM_FORGOTPASS_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserForgotPass result %d", nResult);
		break;
	}
	
    pTemp->MsgForgotPassBack( &NetMsgFB );

	return NET_OK;
}

CAgentUserForgotChangePass::CAgentUserForgotChangePass(
	const char* szUserID,
	const char* szPasswd, 
	const char* szPasswd2, 
	DWORD dwClient )
	: m_dwClientNum( dwClient )
{	
	memset(m_strUserID, 0, sizeof(char) * USR_ID_LENGTH+1);
	memset(m_strPasswd, 0, sizeof(char) * USR_PASS_LENGTH);
	memset(m_strPasswd2, 0, sizeof(char) * USR_PASS_LENGTH);

	if (szUserID != NULL)
		StringCchCopy(m_strUserID, USR_ID_LENGTH+1, szUserID);
	if (szPasswd != NULL)
		StringCchCopy(m_strPasswd, USR_PASS_LENGTH, szPasswd);
	if (szPasswd2 != NULL)
		StringCchCopy(m_strPasswd2, USR_PASS_LENGTH, szPasswd2);
}

int CAgentUserForgotChangePass::Execute(CServer* pServer)
{
	CAgentServer* pTemp = reinterpret_cast<CAgentServer*> (pServer);

	if (pTemp == NULL) return NET_ERROR;

	int nRetCode = 0;	
	int nResult = COdbcManager::GetInstance()->UserChangePass(m_strUserID,
															m_strPasswd,
															m_strPasswd2
															);

	NET_CHANGEPASS_FEEDBACK_DATA2 NetMsgFB;
	NetMsgFB.nClient	= m_dwClientNum;

	switch (nResult)
	{
	case DB_ERROR :
		NetMsgFB.nResult = EM_CHANGEPASS_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserForgotChangePass result %d", nResult);
		break;
	case 1 :	//second pass not success
		NetMsgFB.nResult = EM_CHANGEPASS_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserForgotChangePass result %d", nResult);
		break;
	case 2 :	//Successfull reset pass
		NetMsgFB.nResult = EM_CHANGEPASS_FB_SUB_SUCCESS;
		CConsoleMessage::GetInstance()->Write("CAgentUserForgotChangePass result %d", nResult);
		break;
	default : 
		NetMsgFB.nResult = EM_CHANGEPASS_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserForgotChangePass result %d", nResult);
		break;
	}
	
    pTemp->MsgForgotChangePassBack( &NetMsgFB );

	return NET_OK;
}

CAgentUserSelcharChangePass::CAgentUserSelcharChangePass(
	const char* szUserID,
	const char* szPasswd, 
	const char* szPincode, 
	const char* szNewPasswd, 
	DWORD dwClient )
	: m_dwClientNum( dwClient )
{	
	memset(m_strUserID, 0, sizeof(char) * USR_ID_LENGTH+1);
	memset(m_strPasswd, 0, sizeof(char) * USR_PASS_LENGTH);
	memset(m_strPincode, 0, sizeof(char) * USR_PASS_LENGTH);
	memset(m_strNewPasswd, 0, sizeof(char) * USR_PASS_LENGTH);

	if (szUserID != NULL)
		StringCchCopy(m_strUserID, USR_ID_LENGTH+1, szUserID);
	if (szPasswd != NULL)
		StringCchCopy(m_strPasswd, USR_PASS_LENGTH, szPasswd);
	if (szPincode != NULL)
		StringCchCopy(m_strPincode, USR_PASS_LENGTH, szPincode);
	if (szNewPasswd != NULL)
		StringCchCopy(m_strNewPasswd, USR_PASS_LENGTH, szNewPasswd);
}

int CAgentUserSelcharChangePass::Execute(CServer* pServer)
{
	CAgentServer* pTemp = reinterpret_cast<CAgentServer*> (pServer);

	if (pTemp == NULL) return NET_ERROR;

	int nRetCode = 0;	

	int nResult = COdbcManager::GetInstance()->SelcharChangePass(m_strUserID,
															m_strPasswd,
															m_strPincode,
															m_strNewPasswd
															);

	NET_SELCHAR_CHANGEPASS_FEEDBACK2 NetMsgFB;
	NetMsgFB.nClient	= m_dwClientNum;

	switch (nResult)
	{
	case DB_ERROR :
		NetMsgFB.nResult = EM_CHANGEPASS_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharChangePass result %d", nResult);
		break;
	case 1 :	//second pass not success
		NetMsgFB.nResult = EM_CHANGEPASS_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharChangePass result %d", nResult);
		break;
	case 2 :	//Successfull reset pass
		NetMsgFB.nResult = EM_CHANGEPASS_FB_SUB_SUCCESS;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharChangePass result %d", nResult);
		break;
	default : 
		NetMsgFB.nResult = EM_CHANGEPASS_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharChangePass result %d", nResult);
		break;
	}
	
    pTemp->MsgSelcharChangePassBack( &NetMsgFB );

	return NET_OK;
}

CAgentUserSelcharChangePin::CAgentUserSelcharChangePin(
	const char* szUserID,
	const char* szPasswd, 
	const char* szPincode, 
	const char* szNewPincode, 
	DWORD dwClient )
	: m_dwClientNum( dwClient )
{	
	memset(m_strUserID, 0, sizeof(char) * USR_ID_LENGTH+1);
	memset(m_strPasswd, 0, sizeof(char) * USR_PASS_LENGTH);
	memset(m_strPincode, 0, sizeof(char) * USR_PASS_LENGTH);
	memset(m_strNewPincode, 0, sizeof(char) * USR_PASS_LENGTH);

	if (szUserID != NULL)
		StringCchCopy(m_strUserID, USR_ID_LENGTH+1, szUserID);
	if (szPasswd != NULL)
		StringCchCopy(m_strPasswd, USR_PASS_LENGTH, szPasswd);
	if (szPincode != NULL)
		StringCchCopy(m_strPincode, USR_PASS_LENGTH, szPincode);
	if (szNewPincode != NULL)
		StringCchCopy(m_strNewPincode, USR_PASS_LENGTH, szNewPincode);
}

int CAgentUserSelcharChangePin::Execute(CServer* pServer)
{
	CAgentServer* pTemp = reinterpret_cast<CAgentServer*> (pServer);

	if (pTemp == NULL) return NET_ERROR;

	int nRetCode = 0;	
	int nResult = COdbcManager::GetInstance()->SelcharChangePin(m_strUserID,
															m_strPasswd,
															m_strPincode,
															m_strNewPincode
															);

	NET_SELCHAR_CHANGEPIN_FEEDBACK2 NetMsgFB;
	NetMsgFB.nClient	= m_dwClientNum;

	switch (nResult)
	{
	case DB_ERROR :
		NetMsgFB.nResult = EM_CHANGEPASS_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharChangePin result %d", nResult);
		break;
	case 1 :	//second pass not success
		NetMsgFB.nResult = EM_CHANGEPASS_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharChangePin result %d", nResult);
		break;
	case 2 :	//Successfull reset pass
		NetMsgFB.nResult = EM_CHANGEPASS_FB_SUB_SUCCESS;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharChangePin result %d", nResult);
		break;
	default : 
		NetMsgFB.nResult = EM_CHANGEPASS_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharChangePin result %d", nResult);
		break;
	}
	
    pTemp->MsgSelcharChangePinBack( &NetMsgFB );

	return NET_OK;
}

CAgentReqGameTimeConvert::CAgentReqGameTimeConvert(
	const char* szUserID,
	DWORD dwClient )
	: m_dwClientNum( dwClient )
{	
	memset(m_strUserID, 0, sizeof(char) * USR_ID_LENGTH+1);

	if (szUserID != NULL)	StringCchCopy(m_strUserID, USR_ID_LENGTH+1, szUserID);
}

int CAgentReqGameTimeConvert::Execute(CServer* pServer)
{
	CAgentServer* pTemp = reinterpret_cast<CAgentServer*> (pServer);

	if (pTemp == NULL) return NET_ERROR;

	int nRetCode = 0;	

	int nResult = COdbcManager::GetInstance()->ReqGameTimeConvert(m_strUserID);

	NET_REQ_GAMETIME_CONVERT_FEEDBACK2 NetMsgFB;
	NetMsgFB.nClient	= m_dwClientNum;

	if( nResult != DB_ERROR )
	{
		NetMsgFB.dwPoint = nResult;
		NetMsgFB.nResult = EM_REQ_GAMETIME_FB_SUB_SUCCESS;
		CConsoleMessage::GetInstance()->Write("CAgentReqGameTimeConvert Success %d receive.", nResult);
	}
	else
	{
		NetMsgFB.nResult = EM_REQ_GAMETIME_FB_SUB_FAIL;
		NetMsgFB.dwPoint = 0;
		CConsoleMessage::GetInstance()->Write("CAgentReqGameTimeConvert failed!");
	}
	
    pTemp->MsgReqGameTimeConvertBack( &NetMsgFB );

	return NET_OK;
}

CAgentUserSelcharResetPin::CAgentUserSelcharResetPin(
	const char* szUserID,
	const char* szPasswd, 
	const char* szMail, 
	DWORD dwClient )
	: m_dwClientNum( dwClient )
{	
	memset(m_strUserID, 0, sizeof(char) * USR_ID_LENGTH+1);
	memset(m_strPasswd, 0, sizeof(char) * USR_PASS_LENGTH);
	memset(m_strMail, 0, sizeof(char) * USR_INFOMAIL_LENGTH);
	memset(m_strNewpin, 0, sizeof(char) * USR_PASS_LENGTH);

	if (szUserID != NULL)
		StringCchCopy(m_strUserID, USR_ID_LENGTH+1, szUserID);
	if (szPasswd != NULL)
		StringCchCopy(m_strPasswd, USR_PASS_LENGTH, szPasswd);
	if (szMail != NULL)
		StringCchCopy(m_strMail, USR_INFOMAIL_LENGTH, szMail);
}

int CAgentUserSelcharResetPin::Execute(CServer* pServer)
{
	CAgentServer* pTemp = reinterpret_cast<CAgentServer*> (pServer);

	if (pTemp == NULL) return NET_ERROR;



	WORD wRandomNumber = (WORD)rand() % 32000 + 10001;
	CString strRandom;
	strRandom.Format( "%d", wRandomNumber );

	TCHAR szKey[USR_PASS_LENGTH] = {0};
	StringCchCopy( szKey, USR_PASS_LENGTH, strRandom.GetString() );

	CConsoleMessage::GetInstance()->Write("New pin %s", strRandom.GetString() );

	CHINA_MD5::MD5 md5_Pass;
	size_t nLength_Pass = 0;
	HRESULT sRet_Pass = StringCchLength( szKey, USR_PASS_LENGTH, &nLength_Pass );

	if ( sRet_Pass == STRSAFE_E_INVALID_PARAMETER )				return NET_ERROR;

	NET_SELCHAR_RESETPIN_FEEDBACK2 NetMsgFB;
	NetMsgFB.nClient	= m_dwClientNum;
	md5_Pass.update( reinterpret_cast<const unsigned char*> (szKey), nLength_Pass );

	StringCchCopy(m_strNewpin, USR_PASS_LENGTH, md5_Pass.asString().c_str() );

	int nRetCode = 0;	
	int nResult = COdbcManager::GetInstance()->SelcharResetPin(m_strUserID,
															m_strPasswd,
															m_strMail,
															m_strNewpin
															);

	CConsoleMessage::GetInstance()->Write("%s,%s",m_strMail,m_strNewpin);
	switch (nResult)
	{
	case DB_ERROR :
		NetMsgFB.nResult = EM_RESETPIN_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharResetPin result %d", nResult);
		break;
	case 1 :	//second pass not success
		NetMsgFB.nResult = EM_RESETPIN_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharResetPin result %d", nResult);
		break;
	case 2 :	//Successfull reset pass
		NetMsgFB.nResult = EM_RESETPIN_FB_SUB_SUCCESS;
		StringCchCopy(NetMsgFB.szNewPin,USR_PASS_LENGTH+1,strRandom.GetString() );
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharResetPin result %d", nResult);
		break;
	default : 
		NetMsgFB.nResult = EM_RESETPIN_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharResetPin result %d", nResult);
		break;
	}
	
    pTemp->MsgSelcharResetPinBack( &NetMsgFB );

	return NET_OK;
}

CAgentUserSelcharTopUp::CAgentUserSelcharTopUp(
	const char* szUserID,
	const char* szCode, 
	const char* szPin, 
	DWORD dwClient )
	: m_dwClientNum( dwClient )
{	
	memset(m_strUserID, 0, sizeof(char) * USR_ID_LENGTH+1);
	memset(m_strCode, 0, sizeof(char) * TOPUP_CODE_LENGTH);
	memset(m_strPin, 0, sizeof(char) * TOPUP_PIN_LENGTH);

	if (szUserID != NULL)
		StringCchCopy(m_strUserID, USR_ID_LENGTH+1, szUserID);
	if (szCode != NULL)
		StringCchCopy(m_strCode, TOPUP_CODE_LENGTH, szCode);
	if (szPin != NULL)
		StringCchCopy(m_strPin, TOPUP_PIN_LENGTH, szPin);
}

int CAgentUserSelcharTopUp::Execute(CServer* pServer)
{
	CAgentServer* pTemp = reinterpret_cast<CAgentServer*> (pServer);

	if (pTemp == NULL) return NET_ERROR;

	int nRetCode = 0;	
	int nResult = COdbcManager::GetInstance()->SelcharTopUp(m_strUserID,
															m_strCode,
															m_strPin
															);

	NET_SELCHAR_TOPUP_FEEDBACK2 NetMsgFB;
	NetMsgFB.nClient	= m_dwClientNum;
	switch (nResult)
	{
	case DB_ERROR :
		NetMsgFB.nResult = EM_TOPUP_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharTopUp result %d", nResult);
		break;
	case 1 :	//second pass not success
		NetMsgFB.nResult = EM_TOPUP_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharTopUp result %d", nResult);
		break;
	case 2 :	//Successfull reset pass
		NetMsgFB.nResult = EM_TOPUP_FB_SUB_SUCCESS;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharTopUp result %d", nResult);
		break;
	default : 
		NetMsgFB.nResult = EM_TOPUP_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharTopUp result %d", nResult);
		break;
	}
	
    pTemp->MsgSelcharTopUpBack( &NetMsgFB );

	return NET_OK;
}

CAgentUserSelcharChangeMail::CAgentUserSelcharChangeMail(
	const char* szUserID,
	const char* szPasswd, 
	const char* szPincode, 
	const char* szMailOld, 
	const char* szMailNew, 
	DWORD dwClient )
	: m_dwClientNum( dwClient )
{	
	memset(m_strUserID, 0, sizeof(char) * USR_ID_LENGTH+1);
	memset(m_strPasswd, 0, sizeof(char) * USR_PASS_LENGTH);
	memset(m_strPincode, 0, sizeof(char) * USR_PASS_LENGTH);
	memset(m_strMailOld, 0, sizeof(char) * USR_INFOMAIL_LENGTH);
	memset(m_strMailNew, 0, sizeof(char) * USR_INFOMAIL_LENGTH);

	if (szUserID != NULL)
		StringCchCopy(m_strUserID, USR_ID_LENGTH+1, szUserID);
	if (szPasswd != NULL)
		StringCchCopy(m_strPasswd, USR_PASS_LENGTH, szPasswd);
	if (szPincode != NULL)
		StringCchCopy(m_strPincode, USR_PASS_LENGTH, szPincode);
	if (szMailOld != NULL)
		StringCchCopy(m_strMailOld, USR_INFOMAIL_LENGTH, szMailOld);
	if (szMailNew != NULL)
		StringCchCopy(m_strMailNew, USR_INFOMAIL_LENGTH, szMailNew);
}

int CAgentUserSelcharChangeMail::Execute(CServer* pServer)
{
	CAgentServer* pTemp = reinterpret_cast<CAgentServer*> (pServer);

	if (pTemp == NULL) return NET_ERROR;	
	
	int nRetCode = 0;	
	int nResult = COdbcManager::GetInstance()->SelcharChangeMail(m_strUserID,
															m_strPasswd,
															m_strPincode,
															m_strMailOld,
															m_strMailNew
															);

	NET_SELCHAR_CHANGEMAIL_FEEDBACK2 NetMsgFB;
	NetMsgFB.nClient	= m_dwClientNum;

	switch (nResult)
	{
	case DB_ERROR :
		NetMsgFB.nResult = EM_CHANGEMAIL_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharChangeMail result %d", nResult);
		break;
	case 1 :	//second pass not success
		NetMsgFB.nResult = EM_CHANGEMAIL_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharChangeMail result %d", nResult);
		break;
	case 2 :	//Successfull reset pass
		NetMsgFB.nResult = EM_CHANGEMAIL_FB_SUB_SUCCESS;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharChangeMail result %d", nResult);
		break;
	case 3: //email used
		NetMsgFB.nResult = EM_CHANGEMAIL_FB_SUB_EXIST;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharChangeMail result %d", nResult);
		break;
	default : 
		NetMsgFB.nResult = EM_CHANGEMAIL_FB_SUB_FAIL;
		CConsoleMessage::GetInstance()->Write("CAgentUserSelcharChangeMail result %d", nResult);
		break;
	}
	
    pTemp->MsgSelcharChangeMailBack( &NetMsgFB );

	return NET_OK;
}