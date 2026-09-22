#include "stdafx.h"
#include "./DbActionLogic.h"
#include "./G-Logic/GLGaeaServer.h"
#include "./G-Logic/GLChar.h"

#include "../Lib_Network/s_CSessionServer.h"
#include "../Lib_Network/s_CFieldServer.h"
#include "../Lib_Network/s_CAgentServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*storage lock*/
CStorageLockEnable::CStorageLockEnable( DWORD dwClientID, DWORD dwChaNum, const char* szChaName, const char* szSPin )
{	
	m_dwClient = dwClientID;
	m_dwChaNum = dwChaNum;

	memset(m_szChaName, 0, sizeof(char) * CHR_ID_LENGTH+1);
	memset(m_szSPin, 0, sizeof(char) * USER_STORAGE_LOCK_PASS_SIZE+1);

	if (szChaName != NULL)
		StringCchCopy(m_szChaName, CHR_ID_LENGTH+1, szChaName);

	if (szSPin != NULL)
		StringCchCopy(m_szSPin, USER_STORAGE_LOCK_PASS_SIZE+1, szSPin);
}

int CStorageLockEnable::Execute (CServer* pServer)
{
	if (pServer == NULL)
	{
		return DB_ERROR;
	}
	else
	{
		int nResult = m_pDbManager->StorageLockEnable(m_dwChaNum, m_szChaName, m_szSPin );

		GLMSG::SNETPC_STORAGE_LOCK_ENABLE_FROM_DB NetMsgDB2Fld;
		StringCchCopy ( NetMsgDB2Fld.szName, CHAR_SZNAME+1, m_szChaName );
		StringCchCopy ( NetMsgDB2Fld.szPin, USER_STORAGE_LOCK_PASS_SIZE+1, m_szSPin );

		if ( nResult == DB_OK ) 
		{
			NetMsgDB2Fld.emFB = EMFB_STORAGE_LOCK_ENABLE_DONE;
		}
		else if( nResult == 1 )
		{
			NetMsgDB2Fld.emFB = EMFB_STORAGE_LOCK_ENABLE_ALREADY_ENABLED_DB;
		}
		else if( nResult == 2 )
		{
			NetMsgDB2Fld.emFB = EMFB_STORAGE_LOCK_ENABLE_CHAR_NOT_EXIST;
		}
		else 
		{
			NetMsgDB2Fld.emFB = EMFB_STORAGE_LOCK_ENABLE_FAILED;
		}

		CFieldServer* pTemp = reinterpret_cast<CFieldServer*> (pServer);
		pTemp->InsertMsg(m_dwClient, (char*) &NetMsgDB2Fld );
		return nResult;
	}
}

CStorageLockInput::CStorageLockInput( DWORD dwClientID, DWORD dwChaNum, const char* szChaName, const char* szSPin )
{	
	m_dwClient = dwClientID;
	m_dwChaNum = dwChaNum;

	memset(m_szChaName, 0, sizeof(char) * CHR_ID_LENGTH+1);
	memset(m_szSPin, 0, sizeof(char) * USER_STORAGE_LOCK_PASS_SIZE+1);

	if (szChaName != NULL)
		StringCchCopy(m_szChaName, CHR_ID_LENGTH+1, szChaName);

	if (szSPin != NULL)
		StringCchCopy(m_szSPin, USER_STORAGE_LOCK_PASS_SIZE+1, szSPin);
}

int CStorageLockInput::Execute (CServer* pServer)
{
	if (pServer == NULL)
	{
		return DB_ERROR;
	}
	else
	{
		int nResult = m_pDbManager->StorageLockInput(m_dwChaNum, m_szChaName, m_szSPin );

		GLMSG::SNETPC_STORAGE_LOCK_INPUT_FROM_DB NetMsgDB2Fld;
		StringCchCopy ( NetMsgDB2Fld.szName, CHAR_SZNAME+1, m_szChaName );
		StringCchCopy ( NetMsgDB2Fld.szPin, USER_STORAGE_LOCK_PASS_SIZE+1, m_szSPin );

		if ( nResult == DB_OK ) 
		{
			NetMsgDB2Fld.emFB = EMFB_STORAGE_LOCK_INPUT_DONE;
		}
		else if( nResult == 1 )
		{
			NetMsgDB2Fld.emFB = EMFB_STORAGE_LOCK_INPUT_DATA_NOT_EXIST;
		}
		else 
		{
			NetMsgDB2Fld.emFB = EMFB_STORAGE_LOCK_INPUT_FAILED;
		}

		CFieldServer* pTemp = reinterpret_cast<CFieldServer*> (pServer);
		pTemp->InsertMsg(m_dwClient, (char*) &NetMsgDB2Fld );
		return nResult;
	}
}

CStorageLockRecover::CStorageLockRecover( DWORD dwClientID, DWORD dwChaNum, const char* szChaName )
{	
	m_dwClient = dwClientID;
	m_dwChaNum = dwChaNum;

	memset(m_szChaName, 0, sizeof(char) * CHR_ID_LENGTH+1);

	if (szChaName != NULL)
		StringCchCopy(m_szChaName, CHR_ID_LENGTH+1, szChaName);
}

int CStorageLockRecover::Execute (CServer* pServer)
{
	if (pServer == NULL)
	{
		return DB_ERROR;
	}
	else
	{
		std::string strPin("");
		int nResult = m_pDbManager->StorageLockRecover(m_dwChaNum, m_szChaName, strPin );

		GLMSG::SNETPC_STORAGE_LOCK_RECOVER_FROM_DB NetMsgDB2Fld;
		StringCchCopy ( NetMsgDB2Fld.szName, CHAR_SZNAME+1, m_szChaName );
		StringCchCopy ( NetMsgDB2Fld.szPin, USER_STORAGE_LOCK_PASS_SIZE+1, strPin.c_str() );

		if ( nResult == DB_OK ) 
		{
			NetMsgDB2Fld.emFB = EMFB_STORAGE_LOCK_RECOVER_DONE;
		}else {
			NetMsgDB2Fld.emFB = EMFB_STORAGE_LOCK_RECOVER_NOT_EXIST;
		}

		CFieldServer* pTemp = reinterpret_cast<CFieldServer*> (pServer);
		pTemp->InsertMsg(m_dwClient, (char*) &NetMsgDB2Fld );
		return nResult;
	}
}

CStorageLockRecoverChange::CStorageLockRecoverChange( DWORD dwClientID, DWORD dwChaNum, const char* szChaName, const char* szSPin )
{	
	m_dwClient = dwClientID;
	m_dwChaNum = dwChaNum;

	memset(m_szChaName, 0, sizeof(char) * CHR_ID_LENGTH+1);
	memset(m_szSPin, 0, sizeof(char) * USER_STORAGE_LOCK_PASS_SIZE+1);

	if (szChaName != NULL)
		StringCchCopy(m_szChaName, CHR_ID_LENGTH+1, szChaName);

	if (szSPin != NULL)
		StringCchCopy(m_szSPin, USER_STORAGE_LOCK_PASS_SIZE+1, szSPin);
}

int CStorageLockRecoverChange::Execute (CServer* pServer)
{
	if (pServer == NULL)
	{
		return DB_ERROR;
	}
	else
	{
		int nResult = m_pDbManager->StorageLockRecoverChange(m_dwChaNum, m_szChaName, m_szSPin );

		GLMSG::SNETPC_STORAGE_LOCK_RECOVER_CHANGE_FROM_DB NetMsgDB2Fld;
		StringCchCopy ( NetMsgDB2Fld.szName, CHAR_SZNAME+1, m_szChaName );
		StringCchCopy ( NetMsgDB2Fld.szPin, USER_STORAGE_LOCK_PASS_SIZE+1, m_szSPin );

		if ( nResult == DB_OK ) 
		{
			NetMsgDB2Fld.emFB = EMFB_STORAGE_LOCK_RECOVER_CHANGE_DONE;
		}
		else 
		{
			NetMsgDB2Fld.emFB = EMFB_STORAGE_LOCK_RECOVER_CHANGE_NOT_FOUND;
		}

		CFieldServer* pTemp = reinterpret_cast<CFieldServer*> (pServer);
		pTemp->InsertMsg(m_dwClient, (char*) &NetMsgDB2Fld );
		return nResult;
	}
}

CStorageLockRecoverDelete::CStorageLockRecoverDelete( DWORD dwClientID, DWORD dwChaNum, const char* szChaName )
{	
	m_dwClient = dwClientID;
	m_dwChaNum = dwChaNum;

	memset(m_szChaName, 0, sizeof(char) * CHR_ID_LENGTH+1);

	if (szChaName != NULL)
		StringCchCopy(m_szChaName, CHR_ID_LENGTH+1, szChaName);
}

int CStorageLockRecoverDelete::Execute (CServer* pServer)
{
	if (pServer == NULL)
	{
		return DB_ERROR;
	}
	else
	{
		int nResult = m_pDbManager->StorageLockRecoverDelete(m_dwChaNum, m_szChaName );

		GLMSG::SNETPC_STORAGE_LOCK_RECOVER_DELETE_FROM_DB NetMsgDB2Fld;
		StringCchCopy ( NetMsgDB2Fld.szName, CHAR_SZNAME+1, m_szChaName );

		if ( nResult == DB_OK ) 
		{
			NetMsgDB2Fld.emFB = EMFB_STORAGE_LOCK_RECOVER_DELETE_DONE;
		}
		else 
		{
			NetMsgDB2Fld.emFB = EMFB_STORAGE_LOCK_RECOVER_DELETE_NOT_FOUND;
		}

		CFieldServer* pTemp = reinterpret_cast<CFieldServer*> (pServer);
		pTemp->InsertMsg(m_dwClient, (char*) &NetMsgDB2Fld );
		return nResult;
	}
}