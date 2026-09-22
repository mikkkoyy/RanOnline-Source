#include "stdafx.h"

#include "s_CDbAction.h"
#include "s_CSessionServer.h"
#include "s_CFieldServer.h"
#include "s_CAgentServer.h"

#include "../Lib_Client/G-Logic/GLCharData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*register page, Juver, 2017/11/18 */
CAgentUserRegister::CAgentUserRegister(
									   const TCHAR* szUser,
									   const TCHAR* szPass,
									   const TCHAR* szPass2,
									   const TCHAR* szSA,
									   const TCHAR* szMail,		
									   const TCHAR* szUserIP,
									   int nSvrGrp,
									   int nSvrNum,
									   DWORD dwClient,
									   DWORD dwClientNum )
	: m_nSvrGrp( nSvrGrp )
	, m_nSvrNum( nSvrNum )

{	
	memset(m_szUser, 0, sizeof(char) * USR_ID_LENGTH);
	memset(m_szPass, 0, sizeof(char) * USR_PASS_LENGTH);
	memset(m_szPass2, 0, sizeof(char) * USR_PASS_LENGTH);
	memset(m_szSA, 0, sizeof(char) * USR_PASS_LENGTH);
	memset(m_szMail, 0, sizeof(char) * USR_INFOMAIL_LENGTH);

	if ( szUser != NULL)	
		StringCchCopy( m_szUser, CHR_ID_LENGTH, szUser );

	if ( szPass != NULL)	
		StringCchCopy( m_szPass, CHR_ID_LENGTH, szPass );

	if ( szPass2 != NULL)	
		StringCchCopy( m_szPass2, CHR_ID_LENGTH, szPass2 );

	if ( szSA != NULL)	
		StringCchCopy( m_szSA, CHR_ID_LENGTH, szSA );

	if ( szMail != NULL)	
		StringCchCopy( m_szMail, CHR_ID_LENGTH, szMail );

	m_dwClientNum = dwClientNum;
	m_dwClient = dwClient;
	m_strUserIP = szUserIP;
}

int CAgentUserRegister::Execute(CServer* pServer)
{
	CAgentServer* pTemp = reinterpret_cast<CAgentServer*> (pServer);

	if (pTemp == NULL) return NET_ERROR;

	int nRetCode = 0;	
	int nResult = COdbcManager::GetInstance()->UserRegister ( m_szUser, m_szPass, m_szPass2, m_szSA, m_szMail, m_nSvrGrp, m_nSvrNum );

	NET_REGISTER_ACTION_FB2 fb2;
	fb2.nClient = m_dwClientNum;
	
	StringCchCopy( fb2.szIp, MAX_IP_LENGTH+1,	m_strUserIP.GetString());
	StringCchCopy( fb2.szUserid, USR_ID_LENGTH+1, m_szUser );
	
	switch (nResult)
	{
	case DB_ERROR : 
		{
			fb2.emFB = EMREGISTER_FB2_ERROR;
		}break;
	case 0:
		{
			fb2.emFB = EMREGISTER_FB2_OK;
		}break;
	case 1:
		{
			fb2.emFB = EMREGISTER_FB2_TAKEN;
		}break;
	};
	
	pTemp->RegisterFeedback( &fb2 );

	return NET_OK;
}

