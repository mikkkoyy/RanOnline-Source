//! Copyright (c) Min Communications. All rights reserved.
#include "stdafx.h"
#include "s_CLoginServer.h"
#include "s_CRandomNumber.h"
#include "s_CIPFilter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CLoginServer::CLoginServer( HWND hWnd, HWND hEditBox, HWND hEditBoxInfo, HWND hEditBoxStats )
	: CServer( hWnd, hEditBox, hEditBoxInfo, hEditBoxStats )
	, m_pClientManager( NULL )
	, m_dwLastGameSvrUpdateTime( 0 )
	, m_bInspecting( false )
{	
	for ( int i=0; i<MAX_SERVER_GROUP; i++ )
	{
		for ( int j=0; j<MAX_SERVER_NUMBER; j++ )
		{
			::SecureZeroMemory( &m_sGame[i][j], sizeof(G_SERVER_CUR_INFO_LOGIN) );
		}
	}
}

CLoginServer::~CLoginServer()
{
	SAFE_DELETE( m_pClientManager );
}

// 클라이언트 정보저장 클래스 생성
int CLoginServer::StartClientManager()
{	
	if (m_pClientManager == NULL)
	{
		//m_pClientManager = new CClientLogin( m_nMaxClient, m_hIOServer );
		m_pClientManager = new CClientLogin(m_nMaxClient, m_pSendIOCP, m_pRecvIOCP, m_hIOServer);
	}

	if (!m_pClientManager) 
	{
		m_bIsRunning = false;
		return NET_ERROR;
	}
	else
	{
		return NET_OK;
	}
}

int CLoginServer::Start()
{
	int		nRetCode;	
	
	m_bIsRunning = true;	
	
	if( StartCfg()			== NET_ERROR ) return NET_CFG_LOADERROR;	
	if( StartIOCPList()		== NET_ERROR ) return NET_ERROR;    	
	if( StartMsgManager()	== NET_ERROR ) return NET_ERROR;
	if( StartIOCP()			== NET_ERROR ) return NET_ERROR;
	if( StartClientManager()== NET_ERROR ) return NET_ERROR;	
	if( StartWorkThread()	== NET_ERROR ) return NET_ERROR;
	if( StartUpdateThread() == NET_ERROR ) return NET_ERROR;	
	if( StartListenThread() == NET_ERROR ) return NET_ERROR;	 

	// 인증키 생성
	GenerateEncrypt();
		
	// Connect to Session Server
	nRetCode = SessionConnect(CCfg::GetInstance()->GetSessionServerIP(), CCfg::GetInstance()->GetSessionServerPort());
	if (nRetCode != NET_OK)
	{		
		m_bIsRunning = false;
		return NET_ERROR;
	}	

	// 게임서버의 정보를 요청한다.
	SessionReqSvrInfo(); 

	m_dwLastGameSvrUpdateTime = timeGetTime();

	CConsoleMessage::GetInstance()->Write(_T("================================"));
	CConsoleMessage::GetInstance()->Write(_T("Server Start OK"));
	CConsoleMessage::GetInstance()->Write(_T("================================"));
	
	m_nStatus = S_SERVER_RUNING;

	

	// SERVER_UTIL::CAddressChecker::GetInstance()->AddKnownAddress(CCfg::GetInstance()->GetServerIP());
	// SERVER_UTIL::CAddressChecker::GetInstance()->AddKnownAddress(CCfg::GetInstance()->GetSessionServerIP());
	CIPFilter::GetInstance()->SetServer( this );
	CIPFilter::GetInstance()->SetConsole( CConsoleMessage::GetInstance() );
	CIPFilter::GetInstance()->ListLoad();
	CIPFilter::GetInstance()->AddIPKnown( CCfg::GetInstance()->GetServerIP() );
	CIPFilter::GetInstance()->AddIPKnown( CCfg::GetInstance()->GetSessionServerIP() );

	const TCHAR* szServerName = CCfg::GetInstance()->GetServerName();
	if ( szServerName )
		SetWindowText( m_hWnd, szServerName );

	//iplimit
	if ( m_pClientManager )
	{
		m_pClientManager->IPClear();
		/*dmk14 blocker*/
		m_pClientManager->IPSetClientMax( CCfg::GetInstance()->GetIPClientMax(), true );
		m_pClientManager->IPAddExcluded( CCfg::GetInstance()->GetServerIP() );
		m_pClientManager->IPAddExcluded( CCfg::GetInstance()->GetSessionServerIP() );
	}

	return NET_OK;
}

int CLoginServer::Stop()
{
	CConsoleMessage::GetInstance()->Write(_T("== Please wait until server stop "));

	// Stop All Thread and exit
	DWORD dwExitCode = 0;	

	//////////////////////////////////////////////////////////////////////////////
	// Disconnect Session Server
	// SessionCloseConnect();

	EnterCriticalSection(&m_CriticalSection);
	m_bIsRunning = false;
	LeaveCriticalSection(&m_CriticalSection);

	if( m_bUseEventThread )	
	{
		::SetEvent( m_hUpdateQuitEvent );

		::WaitForSingleObject( m_hUpdateThread, INFINITE );
		SAFE_CLOSE_HANDLE( m_hUpdateQuitEvent );
	}else{
		Sleep( 1000 );
	}

	StopListenThread();
	StopIOCPList();
	StopWorkThread();
	StopIOCP();

	CloseAllClient(); 	// close all client connections
	
	// 클라이언트 저장 클래스 종료
	SAFE_DELETE(m_pSendIOCP);
	SAFE_DELETE(m_pRecvIOCP);	
	CConsoleMessage::GetInstance()->Write(_T("Stop IOCP"));
	// Message Queue 종료	
	SAFE_DELETE(m_pRecvMsgManager);
	CConsoleMessage::GetInstance()->Write(_T("Stop Meessage Queue"));
	// DB 연결종료
	COdbcManager::GetInstance()->ReleaseInstance();
	CConsoleMessage::GetInstance()->Write(_T("Stop DataBase"));
	// CFG class 종료
	CCfg::GetInstance()->ReleaseInstance();
	CConsoleMessage::GetInstance()->Write(_T("Stop CFG"));
	
	// Put message to console
	CConsoleMessage::GetInstance()->Write(_T("================================"));
	CConsoleMessage::GetInstance()->Write(_T("Server Stop OK"));
	CConsoleMessage::GetInstance()->Write(_T("================================"));
	
	m_nStatus = S_SERVER_STOP;

	return NET_OK;
}

int	CLoginServer::Pause()
{
	::closesocket(m_sServer);
	m_sServer = INVALID_SOCKET;
	CloseAllClient();
	m_nStatus = S_SERVER_PAUSE;
	// Put message to console
	CConsoleMessage::GetInstance()->Write(_T("================================"));
	CConsoleMessage::GetInstance()->Write(_T("Server Pause OK"));
	CConsoleMessage::GetInstance()->Write(_T("================================"));

	return NET_OK;	
}

int CLoginServer::Resume()
{
	if (StartListenThread() == NET_OK)
	{
		m_nStatus = S_SERVER_RUNING;
		// Put message to console
		CConsoleMessage::GetInstance()->Write(_T("================================"));
		CConsoleMessage::GetInstance()->Write(_T("Server Resume OK"));
		CConsoleMessage::GetInstance()->Write(_T("================================"));

		return NET_OK;
	}
	else
	{
		// Put message to console
		CConsoleMessage::GetInstance()->Write(_T("================================"));
		CConsoleMessage::GetInstance()->Write(_T("Server Resume Failed"));
		CConsoleMessage::GetInstance()->Write(_T("================================"));

		return NET_ERROR;
	}
}

int	CLoginServer::ReStart()
{
	Stop();
	Start();
	return NET_OK;
}

//bool CLoginServer::CloseClient(
//	DWORD dwClient )
//{
//	if (dwClient<0) return false;
//
//	bool bReturn = false;
//
//	LockOn();
//	
//	if ( dwClient < NET_RESERVED_SLOT )
//	{
//		if ( dwClient == m_dwSession )
//		{
//			// Session 서버와 연결끊김
//			if ( true == m_pClientManager->CloseClient( m_dwSession ) )
//			{
//				bReturn = true;
//				// 연결이 끊겼을 경우 바로 다시 연결 시도 하지 않는다.
//				// Heart Beat에서 다시 체크하기 때문이다.
//				/*
//				if ( true == m_bIsRunning )
//				{
//					// 서버가 가동중이라면 다시 연결 시도.
//					SessionConnect();
//				}
//				*/
//				CConsoleMessage::GetInstance()->Write(_T("================================"));
//				CConsoleMessage::GetInstance()->Write(_T("Session Server Connection Failed"));
//				CConsoleMessage::GetInstance()->Write(_T("================================"));
//			}
//		}
//	}
//	else
//	{
//		if ( true == m_pClientManager->CloseClient(dwClient) )
//		{
//			bReturn = true;
//		}
//	}
//
//	LockOff();
//
//	return bReturn;
//}

void CLoginServer::CloseClient(DWORD dwClient)
{	
	LockOn();

	if ( m_pClientManager->IsOnline(dwClient) )
	{
		if ((dwClient >= 0) && (dwClient < NET_RESERVED_SLOT))
		{
			if ( dwClient == NET_RESERVED_SESSION )

			{
				// Session 서버와 연결끊김
				m_pClientManager->CloseClient( NET_RESERVED_SESSION );
				// 연결이 끊겼을 경우 바로 다시 연결 시도 하지 않는다. Heart Beat에서 다시 체크하기 때문이다.
				/*
				if ( true == m_bIsRunning )
				{
				// 서버가 가동중이라면 다시 연결 시도.
				SessionConnect();
				}
				*/
				CConsoleMessage::GetInstance()->Write(_T("================================"));
				CConsoleMessage::GetInstance()->Write(_T("Session Server Connection Failed"));
				CConsoleMessage::GetInstance()->Write(_T("================================"));
			}
		}
		else
		{
			// 등록된 IP 리스트를 제거한다.
			// SERVER_UTIL::CAddressChecker::GetInstance()->Delete(m_pClientManager->GetClientIP(dwClient));
			m_pClientManager->CloseClient(dwClient);
		}
	}

	LockOff();
}

// close all client connections
void CLoginServer::CloseAllClient()
{	
	CConsoleMessage::GetInstance()->Write(_T("CloseAllClient"));
	for (DWORD dwNum=NET_RESERVED_SLOT; dwNum<(DWORD) m_nMaxClient; ++dwNum)
	{
		CloseClient(dwNum);
	}
}

void CLoginServer::GenerateEncrypt()
{
	SERVER_UTIL::CRandomNumber random;
	CString str;
	random.GenerateRandomString( str, ENCRYPT_KEY +1 , 1, 2, 3 );

	::StringCchCopy( m_szEncrypt, ENCRYPT_KEY +1 , str );
}

int CLoginServer::ListenProc()
{
	LPPER_IO_OPERATION_DATA pIOData = NULL;
	SOCKET Accept;
	DWORD  dwRecvNumBytes = 0;
	DWORD  dwFlags        = 0;
	HANDLE hRetCode       = NULL;
	int    nClientNumber  = 0;
	DWORD  dwClient       = 0;
	int	   nRetCode       = 0;

	while (m_bIsRunning)
	{
		Accept = ::WSAAccept(m_sServer, NULL, NULL, NULL, 0);
		if (Accept == INVALID_SOCKET) 
		{
			nRetCode = ::WSAGetLastError();
			CConsoleMessage::GetInstance()->Write(_T("ERROR:WSAAccept %d"), nRetCode);
			if (nRetCode == WSAENOTSOCK || nRetCode == WSAEINTR)
			{	
				// CConsoleMessage::GetInstance()->Write("Listen Thread Stopped");
				break;
			}
			else
			{
				continue;
			}
		}

		if ( CIPFilter::GetInstance()->IsIPBlocked( Accept ) )
		{
			::shutdown(Accept, SB_BOTH);
			::closesocket(Accept);
			continue;
		}

		//iplimit
		if ( !m_pClientManager->IPAllowConnect( Accept ) )
		{
			::shutdown(Accept, SB_BOTH);
			::closesocket(Accept);
			continue;
		}

		///////////////////////////////////////////////////////////////////////
		// Create per-handle data information structure to associate with the socket
		nClientNumber = m_pClientManager->GetFreeClientID(); // Get free client slot number
		if (nClientNumber == NET_ERROR)
		{
			/*
			LINGER      lingerStruct;     
			lingerStruct.l_onoff  = 1;  
			lingerStruct.l_linger = 5;
			nRetCode = ::setsockopt(Accept, 
									SOL_SOCKET, 
									SO_LINGER, 
									(char *) &lingerStruct, 
									sizeof(lingerStruct));
			*/

			::closesocket(Accept);
			CConsoleMessage::GetInstance()->Write(_T("Reached Max Client Number, Not available client slot"));
			CloseAllClient();
			continue;
		}

		if( m_pClientManager->GetCurrentClientNumber() % 100 == 0  )
		{
			CConsoleMessage::GetInstance()->Write( _T("Current Connect Client Number %d"), m_pClientManager->GetCurrentClientNumber() );
		}


		dwClient = (DWORD) nClientNumber;
		
		// Set client information
		if (m_pClientManager->SetAcceptedClient(dwClient, Accept) == NET_ERROR)
		{
			m_pClientManager->ReleaseClientID(dwClient);
			continue;
		}

		// IP 를 리스트에 등록한다.
		// SERVER_UTIL::CAddressChecker::GetInstance()->IsBlock(m_pClientManager->GetClientIP(dwClient));

		// Associate the accepted socket with the completion port
		hRetCode = ::CreateIoCompletionPort((HANDLE) Accept, 
										    m_hIOServer, 
										    (DWORD) nClientNumber, 
										    0);

		if (hRetCode == NULL) 
		{
			CConsoleMessage::GetInstance()->Write(_T("CreateIoCompletionPort Error"));
			CloseClient((DWORD) nClientNumber);
			continue;
		}

		// Start processing I/O on ther accepted socket
		// First WSARecv, TCP/IP Send 8 bytes (ignored byte)
		// Client 버전 정보를 받는다.
		dwRecvNumBytes = sizeof(NET_MSG_GENERIC);
        pIOData = (LPPER_IO_OPERATION_DATA) GetFreeOverIO(NET_RECV_POSTED);
		//pIOData = getRecvIO( nClientNumber );

		if (pIOData == NULL) 
		{
			CloseClient((DWORD) nClientNumber);
			continue;
		}
		
		pIOData->dwRcvBytes   = 0;
		pIOData->dwTotalBytes = dwRecvNumBytes;
		// Head receive mode
		m_pClientManager->SetNetMode(nClientNumber, NET_PACKET_HEAD);
		
		::WSARecv(Accept,
		  		  &(pIOData->DataBuf), 
				  1,
				  &dwRecvNumBytes,
				  &dwFlags ,
				  &(pIOData->Overlapped),
				  NULL);

		if( !m_bUseEventThread ) Sleep( 0 );
	}
	return 0;	
}

int CLoginServer::UpdateProc()
{
	// 프레임 측정을 위한 변수들
	DWORD dwTimeS          = timeGetTime();
	DWORD dwHeartBeatStart1 = dwTimeS; // 연결 끊김 감지 시간변수
	DWORD dwHeartBeatStart2 = dwTimeS; // 연결 끊김 감지 시간변수
	bool  bHeartBeat	   = false;
	DWORD dwMaxClient      = m_nMaxClient;
	DWORD dwN              = 0;
	DWORD dwTimeTemp       = timeGetTime();

	float fFrame = 0;
	float fCount = 0;
	
	MSG_LIST* pMsg = NULL;

	// Event 방식의 Thread를 위한 변수
	DWORD dwNewWaitTime = 0;
	DWORD dwReturn = 0;
	DWORD dwNewTime = 0;
	DWORD dwUpdateFrameTime = 0;

	double fBeforeTime = 0.0f;
	double fCurrentTime = 0.0f;
	double fTimeDelta = 0.0f;

	while (m_bIsRunning)
	{

		if( m_bUseEventThread )	
		{
			dwNewTime = ::GetTickCount();
			dwUpdateFrameTime = dwNewTime - m_dwUpdateOldTime;
			m_dwUpdateOldTime = dwNewTime;

			// 경과시간 체크
			int nUpdateElspTime = (int) (m_dwUpdateWaitTime - dwUpdateFrameTime );
			if ( 0 >= nUpdateElspTime ) 
			{
				dwNewWaitTime = 0;
			} else {
				dwNewWaitTime = nUpdateElspTime;
			}
			dwReturn = ::WaitForSingleObject( m_hUpdateQuitEvent, dwNewWaitTime );

			if ( WAIT_OBJECT_0 == dwReturn )
			{
				break;
			}else if ( WAIT_TIMEOUT != dwReturn )
			{
				continue;
			}
		}else{
			// 현재 시간을 얻어옴
			fCurrentTime = DXUtil_Timer( TIMER_GETAPPTIME );


			// 다음 랜더링 될 시점을 구함
			fTimeDelta = fBeforeTime+0.001f;

			// 만약 현재가 첫 프레임이나 timeGetTime()이 랜더링될 시점의 시간보다 크면 랜더링
			if( fBeforeTime == 0.0f || fCurrentTime >= fTimeDelta )	
			{
				// 메인루프 처리
				// 현재 시간을 g_BeforeTime에 넣음
				fBeforeTime = fCurrentTime;
			}
			else
			{
				Sleep(0);
				continue;
			}
		}

        m_pClientManager->ResetPreSleepCID ();
		m_pRecvMsgManager->MsgQueueFlip();	// Flip Messge Queue
		pMsg = m_pRecvMsgManager->GetMsg();// Get Message	

		while (pMsg != NULL)
		{
			MsgProcess(pMsg);
			pMsg = m_pRecvMsgManager->GetMsg();
		}


		// HeartBeat Check
		dwTimeTemp = timeGetTime();
		
		/////////////////////////////////////////////////////////////////////////////
		// Session Server State Check
		if ((dwTimeTemp - dwHeartBeatStart1) > SESSION_CHECK_TIME)
		{
			if( !m_pClientManager->IsOnline( NET_RESERVED_SESSION ) )
			{
				SessionConnectSndSvrInfo();
			}
			dwHeartBeatStart1 = timeGetTime();
		}

		if ( (dwTimeTemp - dwHeartBeatStart2) > HEARTBEAT_TIME )
		{
			if (bHeartBeat) // Send Heartbeat
			{
				CConsoleMessage::GetInstance()->WriteInfo(
					_T("Send Heartbeat Request") );

				// Check Session Server
				if ( m_pClientManager->IsOnline( NET_RESERVED_SESSION ) )
				{
					m_pClientManager->ResetHeartBeat( NET_RESERVED_SESSION );
                    NET_HEARTBEAT_SERVER_REQ HeartBeatMsgReqServer;
					SendClient( NET_RESERVED_SESSION, &HeartBeatMsgReqServer );
				}
				else
				{
					CConsoleMessage::GetInstance()->Write(_T("================================"));
					CConsoleMessage::GetInstance()->Write(_T("Session Server Connection Failed"));
					CConsoleMessage::GetInstance()->Write(_T("Reconnecting Session Server."));
					CConsoleMessage::GetInstance()->Write(_T("================================"));

					//SessionConnect();
					//SessionConnectSndSvrInfo();
				}

				// Check Client
				NET_HEARTBEAT_CLIENT_REQ HeartBeatMsgReq;
				for ( dwN = NET_RESERVED_SLOT; dwN < dwMaxClient; dwN++ )
				{
					if ( m_pClientManager->IsOnline(dwN) )
					{
						m_pClientManager->ResetHeartBeat( dwN );
						SendClient( dwN, &HeartBeatMsgReq );
					}
				}
			}
			else // Check Heartbeat
			{
				CConsoleMessage::GetInstance()->WriteInfo(_T("Check Heartbeat"));
				// Check Sessein Server
				if ( true == m_pClientManager->IsOnline( NET_RESERVED_SESSION ) &&
					 false == m_pClientManager->CheckHeartBeat( NET_RESERVED_SESSION ) )
				{
					CloseClient( NET_RESERVED_SESSION );
				}

				// Check Client
				for (dwN = NET_RESERVED_SLOT; dwN < dwMaxClient; dwN++)
				{					
					if ( true == m_pClientManager->IsOnline(dwN) && 
						 false == m_pClientManager->CheckHeartBeat(dwN) )
					{
						CloseClient( dwN );
					}
				}
			}
			bHeartBeat = !bHeartBeat;
			dwHeartBeatStart2 = dwTimeTemp;
		}

		// 프레임 측정
		if ( (dwTimeTemp - dwTimeS) >= FPS_UPDATE_TIME )
		{			
			fFrame = ( (float)((fCount * FPS_UPDATE_TIME) /(float)(dwTimeTemp - dwTimeS)) );

			// 현재 사용자수/프레임 측정 디스플레이
			CConsoleMessage::GetInstance()->WriteInfo(
				_T("Update %d FPS Avg %d msec User %d/%d"),
				(int) (fFrame/(FPS_UPDATE_TIME/1000)),
				(int) (FPS_UPDATE_TIME/fFrame),
				(int) m_pClientManager->GetCurrentClientNumber(),
				(int) m_pClientManager->GetMaxClient() );
			
			// 패킷 처리량 디스플레이 (압축전)
			CConsoleMessage::GetInstance()->WriteInfo(
				_T("In Packet Count %d/sec Traffic %d(bytes)/sec"),
				(int) (m_pClientManager->getInPacketCount() / (FPS_UPDATE_TIME/1000)),
				(int) (m_pClientManager->getInPacketSize() / (FPS_UPDATE_TIME/1000)) );

			// 패킷 처리량 디스플레이 (압축후)
			CConsoleMessage::GetInstance()->WriteInfo(
				_T("Out Packet Count %d/sec Traffic %d(bytes)/sec"),
				(int) (m_pClientManager->getOutPacketCount() / (FPS_UPDATE_TIME/1000)),
				(int) (m_pClientManager->getOutPacketSize() / (FPS_UPDATE_TIME/1000)) );

			// Message Queue 및 IOCP 메모리 사용량 디스플레이
			//CConsoleMessage::GetInstance()->WriteInfo(
			//	_T("Msg Queue Size %d Send %d Recv %d"),
			//	m_pRecvMsgManager->m_pMsgBack->GetCount(),
			//	m_pClientManager->getSendUseCount(),
			//	m_pClientManager->getRecvUseCount() );
			CConsoleMessage::GetInstance()->WriteInfo(
				_T("Msg Queue Size %d Send %d/%d Recv %d/%d"),
				m_pRecvMsgManager->m_pMsgBack->GetCount(),
				m_pSendIOCP->GetUseCount(),
				m_pSendIOCP->GetUnUseCount(),
				m_pRecvIOCP->GetUseCount(),
				m_pRecvIOCP->GetUnUseCount() );

			//stats line
			CConsoleMessage::GetInstance()->WriteStats( _T("IPFILTER IPKnown :%u KnownRef:%I64u"),
				CIPFilter::GetInstance()->GetIPKnownNum(), CIPFilter::GetInstance()->GetRefKnown() );

			CConsoleMessage::GetInstance()->WriteStats( _T("IPFILTER IPBlock :%u BlockRef:%I64u"),
				CIPFilter::GetInstance()->GetIPBlockNum(), CIPFilter::GetInstance()->GetRefBlocked() );

			//iplimit
			CConsoleMessage::GetInstance()->WriteStats( _T("IPLIMIT IP:%u Client:%u Excluded:%u Denied:%I64u"),
				m_pClientManager->IPGetCount(), m_pClientManager->IPGetTotalClient(), 
				m_pClientManager->IPGetExcludedCount(), m_pClientManager->IPGetDenyNum());

			CConsoleMessage::GetInstance()->WriteStats( _T("...") );
			CConsoleMessage::GetInstance()->WriteStats( _T("..") );
			CConsoleMessage::GetInstance()->WriteStats( _T(".") );

			m_pClientManager->resetPacketCount();
			
			dwTimeS = dwTimeTemp;
			fCount = 0;
		}
		else
		{
			fCount++;
		}
	}
	return 0;
}

int CLoginServer::WorkProc()
{
	PER_IO_OPERATION_DATA* pPerIoData = NULL;
	DWORD dwSndBytes  = 0;
	DWORD dwRcvBytes  = 0;
	DWORD dwByteTrans = 0;
	DWORD dwFlags     = 0;
	DWORD dwClient    = -1;
	int	  nRetCode    = 0;
	int   nLastError  = 0;
	DWORD dwMaxClient = (DWORD) m_pClientManager->GetMaxClient();
	NET_MSG_GENERIC* pNmg = NULL;

	while (m_bIsRunning)
	{
		/////////////////////////////////////////////////////////////////////////////
		// Wait for I/O to complete on any socket
		nRetCode = GetQueuedCompletionStatus(m_hIOServer,
											 &dwByteTrans, 
											 (LPDWORD) &dwClient,
											 (LPOVERLAPPED *) &pPerIoData,
											 INFINITE);
		
		// 서버를 종료하기 위해서 종료처리 루틴을 호출했을때...
		if (dwClient == m_dwCompKey && pPerIoData == NULL && dwByteTrans == 0)
		{			
			break;
		}		
		
        // Illegal Message Skip
		if ((dwClient < 0) || (dwClient >= dwMaxClient))
		{		
			ReleaseOperationData(pPerIoData);
			continue;
		}
		
		if (nRetCode == 0) // 강제종료
		{
			// 64 번 에러이면 무조건 종료
			nLastError = GetLastError();
			//releaseIO( dwClient, pPerIoData );
			CloseClient(dwClient);
			ReleaseOperationData(pPerIoData);
			continue;
		}

		if (nRetCode != 0 && pPerIoData == NULL)
		{
			CloseClient(dwClient);
			continue;
		}		
		
		if (dwByteTrans == 0) // 정상종료
		{
			//releaseIO( dwClient, pPerIoData );
			CloseClient( dwClient );
			ReleaseOperationData(pPerIoData);
			continue;
		}
		
		switch (pPerIoData->OperationType)
		{
		// 읽기 완료 통보일때...
		case NET_RECV_POSTED :			
			{
                m_pClientManager->addRcvMsg(dwClient, pPerIoData->Buffer, dwByteTrans);				
				// Get one Message
				while (true) {
					pNmg = (NET_MSG_GENERIC*) m_pClientManager->getRcvMsg(dwClient,FALSE);
					if (NULL == pNmg) {
						break;
					}
					else {
						m_pRecvMsgManager->MsgQueueInsert(dwClient, pNmg, pNmg->dwSize);
					}
				}

				//releaseRecvIO( dwClient , pPerIoData );
				ReleaseOperationData(pPerIoData);
				
				dwRcvBytes = sizeof(NET_MSG_GENERIC);
				//pPerIoData = getRecvIO( dwClient );
				pPerIoData = (LPPER_IO_OPERATION_DATA) GetFreeOverIO(NET_RECV_POSTED);

				if (pPerIoData == NULL) continue;
				
				nRetCode = ::WSARecv(m_pClientManager->GetSocket(dwClient),
									 &(pPerIoData->DataBuf), 
									 1, 
									 &dwRcvBytes,
									 &dwFlags,
									 &(pPerIoData->Overlapped),
									 NULL);
				if ((nRetCode == SOCKET_ERROR) && ((nLastError=WSAGetLastError()) != WSA_IO_PENDING))
				{
					//releaseRecvIO( dwClient, pPerIoData );
					ReleaseOperationData(pPerIoData);
					CloseClient(dwClient);
					CConsoleMessage::GetInstance()->Write(
							_T("ERROR:WorkProc WSARecv Client(%d) ERR(%d)"), 
							dwClient,
							nLastError );
				}
			}
			break;
		// 보내기 완료 통보 일때...
		case NET_SEND_POSTED :
			{				
				dwSndBytes = dwByteTrans + pPerIoData->dwSndBytes; // 총 보낸 바이트수
				if (dwSndBytes >= pPerIoData->dwTotalBytes) // 보내기 완료됨
				{
					//releaseSendIO( dwClient, pPerIoData );
					ReleaseOperationData(pPerIoData);
				}
				else // 보내기 완료되지 않음 남은 바이트를 전송
				{
					CConsoleMessage::GetInstance()->Write( 
						_T("ERROR:WorkProc have bytes, not send completed"));

					pPerIoData->dwSndBytes  = dwSndBytes; // 보낸 바이트수 업데이트
					pPerIoData->DataBuf.buf = pPerIoData->Buffer + dwSndBytes; // 보내야할 데이타 포인터 업데이트
					dwSndBytes              = pPerIoData->dwTotalBytes - dwSndBytes; // 보내야할 바이트수 업데이트
					pPerIoData->DataBuf.len = (u_long) dwSndBytes; // 버퍼길이 업데이트

					// 남은 메시지 재전송요청
					nRetCode = m_pClientManager->SendClient2( dwClient, pPerIoData, dwSndBytes );
					if (nRetCode == NET_ERROR)
					{
						//releaseSendIO( dwClient, pPerIoData );
						CloseClient( dwClient );			
						ReleaseOperationData(pPerIoData);
					}
				}
			}
			break;
		default:
			{
				CloseClient(dwClient);
			}
			break;
		}
		if( !m_bUseEventThread ) Sleep( 0 );
	}
	// ServerWorkerThread Running End
	CConsoleMessage::GetInstance()->Write(_T("WorkProc End"));
	return 0;
}

void CLoginServer::Send(DWORD dwClient, LPPER_IO_OPERATION_DATA PerIoData, DWORD dwSize)
{	
}

int CLoginServer::SendClient(DWORD dwClient, LPVOID pBuffer)
{
    if (pBuffer == NULL) return NET_ERROR;

	return m_pClientManager->SendClient2(dwClient, pBuffer);
}

/*
void CLoginServer::MsgSndPingAnswer(MSG_LIST* pMsg)
{
	if (pMsg == NULL) return;

	NET_MSG_PING*	nmpr = NULL;	
	NET_MSG_PING	nmpa;
	DWORD			dwClient = pMsg->dwClient;
	
	nmpr			= (NET_MSG_PING*) pMsg->Buffer;	
	nmpa.nmg.dwSize	= sizeof(NET_MSG_PING);;
	nmpa.nmg.nType	= NET_MSG_PING_ANSWER;
	nmpa.stime		= nmpr->stime;
	SendClient(dwClient, (char*) &nmpa);	
}
*/

/// 서버 실행중 버전 변경
void CLoginServer::SetVersion(int nGameVer, int nPatchVer)
{
	m_nVersion      = nGameVer;
	m_nPatchVersion = nPatchVer;

	CCfg::GetInstance()->SetServerVersion(nGameVer);
	CCfg::GetInstance()->SetPatchVersion(nPatchVer);
}

// 점검중/정상상태 상태 변화
void CLoginServer::SetInspecting(bool bStatus)
{
	m_bInspecting = bStatus;

	if (m_bInspecting)
		CConsoleMessage::GetInstance()->Write(_T("Inspecting Start"));
	else
		CConsoleMessage::GetInstance()->Write(_T("Inspecting End"));
}

// 서버 상태 반환 점검중/정상상태
bool CLoginServer::GetInspection()
{
	return m_bInspecting;
}

int CLoginServer::ExecuteCommand(char* strCmd)
{
    if (strCmd == NULL) return NET_ERROR;

	// 토큰으로 분리할 분리자.
	// space
	char seps[]   = " ";
	char *token;
//	char strTemp[100];

	// Establish string and get the first token
	token = strtok(strCmd, seps);
	while (token != NULL)
	{
		if (strcmp(token, "print_ip") == 0)
		{
			// SERVER_UTIL::CAddressChecker::GetInstance()->Save();
			return 0;
		}
		// Request
		if (strcmp(token, "req") == 0)
		{
			token = strtok(NULL, seps);
			if (token)
			{
				if (strcmp(token, "info") == 0)
				{
					// SessionReqSvrInfo();
				}
				return 0;
			}
			else
			{
				return 0;
			}
		}
		//	Memo :	PROFILE 사용.
		if( strcmp(token, "profile_on") == 0 )
		{
			g_bProfile = TRUE;
			CConsoleMessage::GetInstance()->Write("PROFILE ON");
			return 0;
		}
		//	Memo :	PROFILE 사용안함.
		if( strcmp(token, "profile_off") == 0 )
		{
			g_bProfile = FALSE;
			CConsoleMessage::GetInstance()->Write("PROFILE OFF");
			return 0;
		}

		if (strcmp(token, "ipfilter_add_block") == 0)
		{
			token = strtok(NULL, seps);
			if (token){
				CIPFilter::GetInstance()->AddIPBlock( std::string(token), true );
				return 0;
			}else{
				return 0;
			}
		}

		if (strcmp(token, "ipfilter_add_known") == 0)
		{
			token = strtok(NULL, seps);
			if (token){
				CIPFilter::GetInstance()->AddIPKnown( std::string(token), true );
				return 0;
			}else{
				return 0;
			}
		}

		if (strcmp(token, "ipfilter_del_block") == 0)
		{
			token = strtok(NULL, seps);
			if (token){
				CIPFilter::GetInstance()->RemoveIPBlock( std::string(token), true );
				return 0;
			}else{
				return 0;
			}
		}

		if (strcmp(token, "ipfilter_del_known") == 0)
		{
			token = strtok(NULL, seps);
			if (token){
				CIPFilter::GetInstance()->RemoveIPKnown( std::string(token), true );
				return 0;
			}else{
				return 0;
			}
		}

		if( strcmp(token, "ipfilter_save") == 0 )
		{
			CIPFilter::GetInstance()->DoListSave();
			return 0;
		}


		//iplimit
		if( strcmp(token, "iplimit_on") == 0 )
		{
			if ( m_pClientManager ){
				/*dmk14 blocker*/
				m_pClientManager->IPSetClientMax( CCfg::GetInstance()->GetIPClientMax(), true );
			}
			return 0;
		}

		if( strcmp(token, "iplimit_off") == 0 )
		{
			if ( m_pClientManager ){
				m_pClientManager->IPSetClientMax(0);
			}
			return 0;
		}

		if (strcmp(token, "iplimit_ip_remove") == 0)
		{
			token = strtok(NULL, seps);
			if (token){
				if ( m_pClientManager ){
					m_pClientManager->IPRemove( std::string(token) );
				}
				return 0;
			}else{
				return 0;
			}
		}

		if (strcmp(token, "iplimit_ip_clear") == 0)
		{
			if ( m_pClientManager ){
				m_pClientManager->IPClear();	
			}

			return 0;
		}

		if (strcmp(token, "iplimit_exclude_add") == 0)
		{
			token = strtok(NULL, seps);
			if (token){
				if ( m_pClientManager ){
					m_pClientManager->IPAddExcluded( std::string(token) );
				}

				return 0;
			}else{
				return 0;
			}
		}

		if (strcmp(token, "iplimit_exclude_del") == 0)
		{
			token = strtok(NULL, seps);
			if (token){
				if ( m_pClientManager ){
					m_pClientManager->IPRemoveExcluded( std::string(token) );
				}

				return 0;
			}else{
				return 0;
			}
		}

		// Get next token
		token = strtok(NULL, seps);
	}	
	return 0;
}

void CLoginServer::IPFilterCleanIP( std::string _strIP )
{
	if ( !m_pClientManager )	return;
	if ( !_strIP.size() )		return;

	for( int i=NET_RESERVED_SLOT; i<m_nMaxClient; ++i )
	{
		if( !m_pClientManager->IsOnline(i) )	continue;

		std::string strClientIP = m_pClientManager->GetClientIP( i );
		if ( !strClientIP.size() )	continue;

		if ( strClientIP.compare( _strIP ) == 0)
		{
			CConsoleMessage::GetInstance()->Write(_T("IPFILTER Drop Client:%d IP:%s"), i, _strIP.c_str());
			CloseClient( i );
		}
	}
}

//! -----------------------------------------------------------------------
//! I/O operation memory
//int CLoginServer::StopIOCPList()
//{
//	/*
//	DWORD dwCount = 0;
//	
//	dwCount = m_pClientManager->getSendUseCount();
//
//	for (DWORD i=0; i<dwCount; ++i)
//	{
//		::PostQueuedCompletionStatus(
//			m_hIOServer,
//			0,
//			m_dwCompKey,
//			NULL );
//	}
//	Sleep( 1000 );
//
//	nCount = m_pSendIOCP->GetUseCount();
//	for (int i=0; i<nCount; i++) 
//	{
//		::PostQueuedCompletionStatus(m_hIOServer,
//							0,
//							m_dwCompKey, 
//							NULL);
//	}
//	Sleep( 1000 );
//	*/
//
//	return NET_OK;
//}

/*
PER_IO_OPERATION_DATA* CLoginServer::getSendIO(
	DWORD dwClient )
{
	return m_pClientManager->getSendIO( dwClient );
}

PER_IO_OPERATION_DATA* CLoginServer::getRecvIO(
	DWORD dwClient )
{
	return m_pClientManager->getRecvIO( dwClient );
}

void CLoginServer::releaseSendIO(
	DWORD dwClient,
	PER_IO_OPERATION_DATA* pData )
{
	m_pClientManager->releaseSendIO( dwClient, pData );
}

void CLoginServer::releaseRecvIO(
	DWORD dwClient,
	PER_IO_OPERATION_DATA* pData )
{
	m_pClientManager->releaseRecvIO( dwClient, pData );
}

void CLoginServer::releaseIO(
	DWORD dwClient,
	PER_IO_OPERATION_DATA* pData )
{
	if (NULL != pData)
	{
		if (NET_SEND_POSTED == pData->OperationType)
		{
			releaseSendIO( dwClient, pData );
		}
		else
		{
			releaseRecvIO( dwClient, pData );
		}
	}
}
*/

//      _(,__           __),
//  (_,d888888888b,d888888888b
//   d888888888888/888888888888b_)
//(_8888888P'""'`Y8Y`'""'"Y88888b
//  Y8888P.-'     `      '-.Y8888b_)
// ,_Y88P (_(_(        )_)_) d88Y_,
//  Y88b,  (o  )      (o  ) d8888P
//  `Y888   '-'        '-'  `88Y`
//  ,d/O\         c         /O\b,
//    \_/'.,______w______,.'\_/
//       .-`             `-.
//      /   , d88b  d88b_   \
//     /   / 88888bd88888`\  \
//    /   / \ Y88888888Y   \  \
//    \  \   \ 88888888    /  /
//     `\ `.  \d8888888b, /\\/
//       `.//.d8888888888b; |
//         |/d888888888888b/
//         d8888888888888888b
//      ,_d88p""q88888p""q888b,
//      `""'`\    "`|    /`'""`
//            `.    |===/
//              >   |   |
//              /   |   |
//             |    |   |
//             |    Y  /
//             \   /  /
//              | /| /
//             / / / |
//            /=/  |=/
//           `"`   `"`
