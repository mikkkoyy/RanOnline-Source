#include "StdAfx.h"
#include "s_CIPFilter.h"

#include "s_NetGlobal.h"
#include "s_CServer.h"
#include "s_CConsoleMessage.h"
#include "s_CSystemInfo.h"
#include "s_CSessionServer.h"
#include "s_CLoginServer.h"
#include "s_CFieldServer.h"
#include "s_CAgentServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CIPFilter* CIPFilter::SelfInstance = NULL;

CIPFilter::CIPFilter()
	: m_llBlockRef( 0 )
	, m_llKnownRef( 0 )
	, m_pServer( NULL )
	, m_pConsole( NULL )
{
	m_setIPBlock.clear();
	m_setIPKnown.clear();

	InitializeCriticalSection(&m_CSPCLock);
}
 
CIPFilter::~CIPFilter()
{
	m_setIPBlock.clear();
	m_setIPKnown.clear();

	DeleteCriticalSection(&m_CSPCLock);
}

CIPFilter* CIPFilter::GetInstance()
{
	if (SelfInstance == NULL)
		SelfInstance = new CIPFilter();

	return SelfInstance;
}

void CIPFilter::ReleaseInstance()
{
	if (SelfInstance != NULL)
		SAFE_DELETE(SelfInstance);
}

void CIPFilter::SetServer( CServer* _pServer )
{
	m_pServer = NULL;

	if ( _pServer )
		m_pServer = _pServer;
}

void CIPFilter::SetConsole( CConsoleMessage* _pConsole )
{
	m_pConsole = NULL;

	if ( _pConsole )
		m_pConsole = _pConsole;
}

BOOL CIPFilter::IsIPBlocked( SOCKET _sSocket )
{
	sockaddr_in	sAddrIn;
	int nSize = sizeof(sockaddr_in);
	char szIp[MAX_IP_LENGTH+1] = {0};

	::getpeername(_sSocket, (sockaddr *) &sAddrIn, &nSize);
	::StringCchCopy(szIp, MAX_IP_LENGTH+1, ::inet_ntoa(sAddrIn.sin_addr));

	return IsIPBlocked( std::string(szIp) );
}

BOOL CIPFilter::IsIPBlocked( std::string _strIP )
{
	if ( _strIP.size() <= 0 )	return FALSE;

	if ( IsIPKnown( _strIP ) == TRUE )	return FALSE;

	return IsBlocked( _strIP );
}

BOOL CIPFilter::IsIPKnown( std::string _strIP )
{
	if ( _strIP.size() <= 0 )	return FALSE;

	EnterCriticalSection(&m_CSPCLock);

	SET_IP_ITER iter = m_setIPKnown.find( _strIP );
	if ( iter != m_setIPKnown.end() )	
	{
		m_llKnownRef ++;
		LeaveCriticalSection(&m_CSPCLock);
		return TRUE;
	}

	LeaveCriticalSection(&m_CSPCLock);
	return FALSE;
}

BOOL CIPFilter::IsBlocked( std::string _strIP )
{
	if ( _strIP.size() <= 0 )	return FALSE;

	EnterCriticalSection(&m_CSPCLock);

	SET_IP_ITER iter = m_setIPBlock.find( _strIP );
	if ( iter != m_setIPBlock.end() )	
	{
		m_llBlockRef ++;
		LeaveCriticalSection(&m_CSPCLock);
		return TRUE;
	}

	LeaveCriticalSection(&m_CSPCLock);
	return FALSE;
}

void CIPFilter::AddIPBlock( std::string _strIP, bool _bBroadcast /*= false*/, bool _bFromFile )
{
	if ( _strIP.size() <= 0 )	
	{
		if ( m_pConsole )
			m_pConsole->WriteFile(_T( "IPFilter IP(%s) Invalid"), _strIP.c_str() );

		return;
	}

	EnterCriticalSection(&m_CSPCLock);

	SET_IP_ITER iter = m_setIPBlock.find( _strIP );
	if ( iter == m_setIPBlock.end() )	
	{
		m_setIPBlock.insert( _strIP );

		/*dmk14 blocker bad packet*/
		if( !_bFromFile )
		{
			m_pConsole->WriteFile(_T( "Block IP(%s)"), _strIP.c_str() );
			
			char jblock[125];
			sprintf(jblock,"netsh advfirewall firewall add rule name=\"%s\" dir=in interface=any action=block remoteip=%s/32", _strIP.c_str(), _strIP.c_str() );
			WinExec(jblock, SW_HIDE);
		}
	}

	if ( m_pConsole )
		m_pConsole->WriteFile(_T( "IPFilter IP(%s) Added To BlockList"), _strIP.c_str() );

	if ( m_pServer )
	{
		int nServerType = m_pServer->GetTypeServer();

		if( nServerType ==  NET_SERVER_LOGIN )
		{
			CLoginServer* pServer = (CLoginServer*)m_pServer;
			if ( pServer  )
			pServer->IPFilterCleanIP( _strIP );
		}
		else if( nServerType == NET_SERVER_SESSION )
		{
			CSessionServer* pServer = (CSessionServer*)m_pServer;
			if ( pServer )
			pServer->IPFilterCleanIP( _strIP );
		}
		else if( nServerType == NET_SERVER_FIELD )
		{
			CFieldServer* pServer = (CFieldServer*)m_pServer;
			if ( pServer  )
			pServer->IPFilterCleanIP( _strIP );
		}
		else if( nServerType == NET_SERVER_AGENT )
		{
			CAgentServer* pServer = (CAgentServer*)m_pServer;
			if ( pServer  )
			pServer->IPFilterCleanIP( _strIP );
		}
		
		if ( _bBroadcast )
		{
			NET_IPFILTER_BLOCK_ADD_SESSION netMsg;
			StringCchCopy( netMsg.szIP, MAX_IP_LENGTH+1, _strIP.c_str() );
			netMsg.nServerType = nServerType;

			if( nServerType ==  NET_SERVER_LOGIN )
			{
				CLoginServer* pServer = (CLoginServer*)m_pServer;
				if ( pServer  )
					pServer->SendSession( &netMsg );
			}
			else if( nServerType == NET_SERVER_SESSION )
			{
				CSessionServer* pServer = (CSessionServer*)m_pServer;
				if ( pServer  )
					pServer->MsgServerBroadCastIPFilterBlockAddSelf( _strIP );
			}
			else if( nServerType == NET_SERVER_FIELD )
			{
				CFieldServer* pServer = (CFieldServer*)m_pServer;
				if ( pServer  )
					pServer->SendSession( &netMsg );
			}
			else if( nServerType == NET_SERVER_AGENT )
			{
				CAgentServer* pServer = (CAgentServer*)m_pServer;
				if ( pServer  )
					pServer->SendSession( &netMsg );
			}
		}	
	}

	LeaveCriticalSection(&m_CSPCLock);
}

void CIPFilter::AddIPKnown( std::string _strIP, bool _bBroadcast /*= false*/ )
{
	if ( _strIP.size() <= 0 )	
	{
		if ( m_pConsole )
			m_pConsole->WriteFile(_T( "IPFilter IP(%s) Invalid"), _strIP.c_str() );

		return;
	}

	EnterCriticalSection(&m_CSPCLock);

	SET_IP_ITER iter = m_setIPKnown.find( _strIP );
	if ( iter == m_setIPKnown.end() )	
	{
		m_setIPKnown.insert( _strIP );
	}

	if ( m_pConsole )
		m_pConsole->WriteFile(_T( "IPFilter IP(%s) Added To KnownList"), _strIP.c_str() );

	if ( m_pServer && _bBroadcast )
	{
		int nServerType = m_pServer->GetTypeServer();

		NET_IPFILTER_KNOWN_ADD_SESSION netMsg;
		StringCchCopy( netMsg.szIP, MAX_IP_LENGTH+1, _strIP.c_str() );
		netMsg.nServerType = nServerType;

		if( nServerType ==  NET_SERVER_LOGIN )
		{
			CLoginServer* pServer = (CLoginServer*)m_pServer;
			if ( pServer  )
				pServer->SendSession( &netMsg );
		}
		else if( nServerType == NET_SERVER_SESSION )
		{
			CSessionServer* pServer = (CSessionServer*)m_pServer;
			if ( pServer  )
				pServer->MsgServerBroadCastIPFilterKnownAddSelf( _strIP );
		}
		else if( nServerType == NET_SERVER_FIELD )
		{
			CFieldServer* pServer = (CFieldServer*)m_pServer;
			if ( pServer  )
				pServer->SendSession( &netMsg );
		}
		else if( nServerType == NET_SERVER_AGENT )
		{
			CAgentServer* pServer = (CAgentServer*)m_pServer;
			if ( pServer  )
				pServer->SendSession( &netMsg );
		}
	}

	LeaveCriticalSection(&m_CSPCLock);
}

void CIPFilter::RemoveIPBlock( std::string _strIP, bool _bBroadcast /*= false*/ )
{
	if ( _strIP.size() <= 0 )	
	{
		if ( m_pConsole )
			m_pConsole->WriteFile(_T( "IPFilter IP(%s) Invalid"), _strIP.c_str() );

		return;
	}

	EnterCriticalSection(&m_CSPCLock);

	SET_IP_ITER iter = m_setIPBlock.find( _strIP );
	if ( iter != m_setIPBlock.end() )	
	{
		m_setIPBlock.erase( iter );
	}

	if ( m_pConsole )
		m_pConsole->WriteFile(_T( "IPFilter IP(%s) Removed From BlockList"), _strIP.c_str() );

	if ( m_pServer && _bBroadcast )
	{
		int nServerType = m_pServer->GetTypeServer();

		NET_IPFILTER_BLOCK_DEL_SESSION netMsg;
		StringCchCopy( netMsg.szIP, MAX_IP_LENGTH+1, _strIP.c_str() );
		netMsg.nServerType = nServerType;

		if( nServerType ==  NET_SERVER_LOGIN )
		{
			CLoginServer* pServer = (CLoginServer*)m_pServer;
			if ( pServer  )
				pServer->SendSession( &netMsg );
		}
		else if( nServerType == NET_SERVER_SESSION )
		{
			CSessionServer* pServer = (CSessionServer*)m_pServer;
			if ( pServer  )
				pServer->MsgServerBroadCastIPFilterBlockDelSelf( _strIP );
		}
		else if( nServerType == NET_SERVER_FIELD )
		{
			CFieldServer* pServer = (CFieldServer*)m_pServer;
			if ( pServer  )
				pServer->SendSession( &netMsg );
		}
		else if( nServerType == NET_SERVER_AGENT )
		{
			CAgentServer* pServer = (CAgentServer*)m_pServer;
			if ( pServer  )
				pServer->SendSession( &netMsg );
		}
	}

	LeaveCriticalSection(&m_CSPCLock);
}

void CIPFilter::RemoveIPKnown( std::string _strIP, bool _bBroadcast /*= false*/ )
{
	if ( _strIP.size() <= 0 )	
	{
		if ( m_pConsole )
			m_pConsole->WriteFile(_T( "IPFilter IP(%s) Invalid"), _strIP.c_str() );

		return;
	}

	EnterCriticalSection(&m_CSPCLock);

	SET_IP_ITER iter = m_setIPKnown.find( _strIP );
	if ( iter != m_setIPKnown.end() )	
	{
		m_setIPKnown.erase( iter );
	}

	if ( m_pConsole )
		m_pConsole->WriteFile(_T( "IPFilter IP(%s) Removed From KnownList"), _strIP.c_str() );

	if ( m_pServer && _bBroadcast )
	{
		int nServerType = m_pServer->GetTypeServer();

		NET_IPFILTER_KNOWN_DEL_SESSION netMsg;
		StringCchCopy( netMsg.szIP, MAX_IP_LENGTH+1, _strIP.c_str() );
		netMsg.nServerType = nServerType;

		if( nServerType ==  NET_SERVER_LOGIN )
		{
			CLoginServer* pServer = (CLoginServer*)m_pServer;
			if ( pServer  )
				pServer->SendSession( &netMsg );
		}
		else if( nServerType == NET_SERVER_SESSION )
		{
			CSessionServer* pServer = (CSessionServer*)m_pServer;
			if ( pServer  )
				pServer->MsgServerBroadCastIPFilterKnownDelSelf( _strIP );
		}
		else if( nServerType == NET_SERVER_FIELD )
		{
			CFieldServer* pServer = (CFieldServer*)m_pServer;
			if ( pServer  )
				pServer->SendSession( &netMsg );
		}
		else if( nServerType == NET_SERVER_AGENT )
		{
			CAgentServer* pServer = (CAgentServer*)m_pServer;
			if ( pServer  )
				pServer->SendSession( &netMsg );
		}
	}

	LeaveCriticalSection(&m_CSPCLock);
}

void CIPFilter::DoListSave()
{
	if ( m_pConsole )
		m_pConsole->WriteFile(_T( "IPFilter Session to save iplist") );

	if ( m_pServer )
	{
		int nServerType = m_pServer->GetTypeServer();

		NET_IPFILTER_LISTSAVE netMsg;
		if( nServerType ==  NET_SERVER_LOGIN )
		{
			CLoginServer* pServer = (CLoginServer*)m_pServer;
			if ( pServer  )
				pServer->SendSession( &netMsg );
		}
		else if( nServerType == NET_SERVER_SESSION )
		{
			ListSave();
		}
		else if( nServerType == NET_SERVER_FIELD )
		{
			CFieldServer* pServer = (CFieldServer*)m_pServer;
			if ( pServer  )
				pServer->SendSession( &netMsg );
		}
		else if( nServerType == NET_SERVER_AGENT )
		{
			CAgentServer* pServer = (CAgentServer*)m_pServer;
			if ( pServer  )
				pServer->SendSession( &netMsg );
		}
	}
}

void CIPFilter::ListLoad()
{
	m_setIPBlock.clear();
	m_setIPKnown.clear();

	SERVER_UTIL::CSystemInfo SysInfo;
	CString strAppPath;	

	strAppPath = SysInfo.GetAppPath();
	strAppPath += _T("\\cfg\\");
	strAppPath += "IPFilter.cfg";

	FILE *oFile = NULL;
	
	if ((oFile = ::_tfopen(strAppPath.GetString(), _T("r"))) == NULL)	return;

	TCHAR line[300];

	while (::_fgetts(line, 200, oFile))
	{
		TCHAR seps[]   = _T(" ,\t\n");

		TCHAR* token = _tcstok(line, seps);
		while (token != NULL)
		{
			if (_tcscmp(token, _T("//")) == 0) 
			{	
				break;
			}
			else if (_tcscmp(token, "allow") == 0)
			{
				token = ::_tcstok(NULL, seps );
				if (token)
				{
					AddIPKnown( token );
				}else break;
			}
			else if (_tcscmp(token, "block") == 0)
			{
				token = ::_tcstok(NULL, seps );
				if (token)
				{
					/*dmk14 blocker*/
					AddIPBlock( token, false, true );
				}else break;
			}
			
			token = ::_tcstok( NULL, seps );
		}
	}

	::fclose( oFile );

	if ( m_pConsole )
		m_pConsole->WriteFile(_T( "IPFilter LoadList Known(%d) Blocked(%d) "), GetIPKnownNum(), GetIPBlockNum() );
}

void CIPFilter::ListSave()
{
	SERVER_UTIL::CSystemInfo SysInfo;
	CString strAppPath;	

	strAppPath = SysInfo.GetAppPath();
	strAppPath += _T("\\cfg\\");
	strAppPath += "IPFilter.cfg";

	FILE *oFile = NULL;

	if ((oFile = ::_tfopen(strAppPath.GetString(), _T("w"))) == NULL)	return;

	CString strText;
	SET_IP_ITER iter;

	for ( iter = m_setIPKnown.begin(); iter != m_setIPKnown.end(); iter++ )
	{
		CString strFormat;
		strFormat.Format( "allow %s", std::string( *iter ).c_str() );
		strText += strFormat.GetString();
		strText += "\n";
	}

	for ( iter = m_setIPBlock.begin(); iter != m_setIPBlock.end(); iter++ )
	{
		CString strFormat;
		strFormat.Format( "block %s", std::string( *iter ).c_str() );
		strText += strFormat.GetString();
		strText += "\n";
	}

	if (strText.GetLength())
		::fwrite ( strText.GetBuffer(), strText.GetLength(), 1, oFile );

	::fclose( oFile );

	if ( m_pConsole )
		m_pConsole->WriteFile(_T( "IPFilter SaveList Known(%d) Blocked(%d) "), GetIPKnownNum(), GetIPBlockNum() );
}