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


/*inventory lock*/
CInventoryLockEnable::CInventoryLockEnable( DWORD dwClientID, DWORD dwChaNum, const char* szChaName, const char* szIPin )
{	
	m_dwClient = dwClientID;
	m_dwChaNum = dwChaNum;

	memset(m_szChaName, 0, sizeof(char) * CHR_ID_LENGTH+1);
	memset(m_szIPin, 0, sizeof(char) * CHAR_INVENTORY_LOCK_PASS_SIZE+1);

	if (szChaName != NULL)
		StringCchCopy(m_szChaName, CHR_ID_LENGTH+1, szChaName);

	if (szIPin != NULL)
		StringCchCopy(m_szIPin, CHAR_INVENTORY_LOCK_PASS_SIZE+1, szIPin);
}

int CInventoryLockEnable::Execute (CServer* pServer)
{
	if (pServer == NULL)
	{
		return DB_ERROR;
	}
	else
	{
		int nResult = m_pDbManager->InventoryLockEnable(m_dwChaNum, m_szChaName, m_szIPin );

		GLMSG::SNETPC_INVENTORY_LOCK_ENABLE_FROM_DB NetMsgDB2Fld;
		StringCchCopy ( NetMsgDB2Fld.szName, CHAR_SZNAME+1, m_szChaName );
		StringCchCopy ( NetMsgDB2Fld.szPin, CHAR_INVENTORY_LOCK_PASS_SIZE+1, m_szIPin );

		if ( nResult == DB_OK ) 
		{
			NetMsgDB2Fld.emFB = EMFB_INVENTORY_LOCK_ENABLE_DONE;
		}
		else if( nResult == 1 )
		{
			NetMsgDB2Fld.emFB = EMFB_INVENTORY_LOCK_ENABLE_ALREADY_ENABLED_DB;
		}
		else if( nResult == 2 )
		{
			NetMsgDB2Fld.emFB = EMFB_INVENTORY_LOCK_ENABLE_CHAR_NOT_EXIST;
		}
		else 
		{
			NetMsgDB2Fld.emFB = EMFB_INVENTORY_LOCK_ENABLE_FAILED;
		}

		CFieldServer* pTemp = reinterpret_cast<CFieldServer*> (pServer);
		pTemp->InsertMsg(m_dwClient, (char*) &NetMsgDB2Fld );
		return nResult;
	}
}

CInventoryLockInput::CInventoryLockInput( DWORD dwClientID, DWORD dwChaNum, const char* szChaName, const char* szIPin )
{	
	m_dwClient = dwClientID;
	m_dwChaNum = dwChaNum;

	memset(m_szChaName, 0, sizeof(char) * CHR_ID_LENGTH+1);
	memset(m_szIPin, 0, sizeof(char) * CHAR_INVENTORY_LOCK_PASS_SIZE+1);

	if (szChaName != NULL)
		StringCchCopy(m_szChaName, CHR_ID_LENGTH+1, szChaName);

	if (szIPin != NULL)
		StringCchCopy(m_szIPin, CHAR_INVENTORY_LOCK_PASS_SIZE+1, szIPin);
}

int CInventoryLockInput::Execute (CServer* pServer)
{
	if (pServer == NULL)
	{
		return DB_ERROR;
	}
	else
	{
		int nResult = m_pDbManager->InventoryLockInput(m_dwChaNum, m_szChaName, m_szIPin );

		GLMSG::SNETPC_INVENTORY_LOCK_INPUT_FROM_DB NetMsgDB2Fld;
		StringCchCopy ( NetMsgDB2Fld.szName, CHAR_SZNAME+1, m_szChaName );
		StringCchCopy ( NetMsgDB2Fld.szPin, CHAR_INVENTORY_LOCK_PASS_SIZE+1, m_szIPin );

		if ( nResult == DB_OK ) 
		{
			NetMsgDB2Fld.emFB = EMFB_INVENTORY_LOCK_INPUT_DONE;
		}
		else if( nResult == 1 )
		{
			NetMsgDB2Fld.emFB = EMFB_INVENTORY_LOCK_INPUT_DATA_NOT_EXIST;
		}
		else 
		{
			NetMsgDB2Fld.emFB = EMFB_INVENTORY_LOCK_INPUT_FAILED;
		}

		CFieldServer* pTemp = reinterpret_cast<CFieldServer*> (pServer);
		pTemp->InsertMsg(m_dwClient, (char*) &NetMsgDB2Fld );
		return nResult;
	}
}

CInventoryLockRecover::CInventoryLockRecover( DWORD dwClientID, DWORD dwChaNum, const char* szChaName )
{	
	m_dwClient = dwClientID;
	m_dwChaNum = dwChaNum;

	memset(m_szChaName, 0, sizeof(char) * CHR_ID_LENGTH+1);

	if (szChaName != NULL)
		StringCchCopy(m_szChaName, CHR_ID_LENGTH+1, szChaName);
}

int CInventoryLockRecover::Execute (CServer* pServer)
{
	if (pServer == NULL)
	{
		return DB_ERROR;
	}
	else
	{
		std::string strPin("");
		int nResult = m_pDbManager->InventoryLockRecover(m_dwChaNum, m_szChaName, strPin );

		GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_FROM_DB NetMsgDB2Fld;
		StringCchCopy ( NetMsgDB2Fld.szName, CHAR_SZNAME+1, m_szChaName );
		StringCchCopy ( NetMsgDB2Fld.szPin, CHAR_INVENTORY_LOCK_PASS_SIZE+1, strPin.c_str() );

		if ( nResult == DB_OK ) 
		{
			NetMsgDB2Fld.emFB = EMFB_INVENTORY_LOCK_RECOVER_DONE;
		}else {
			NetMsgDB2Fld.emFB = EMFB_INVENTORY_LOCK_RECOVER_NOT_EXIST;
		}

		CFieldServer* pTemp = reinterpret_cast<CFieldServer*> (pServer);
		pTemp->InsertMsg(m_dwClient, (char*) &NetMsgDB2Fld );
		return nResult;
	}
}

CInventoryLockRecoverChange::CInventoryLockRecoverChange( DWORD dwClientID, DWORD dwChaNum, const char* szChaName, const char* szIPin )
{	
	m_dwClient = dwClientID;
	m_dwChaNum = dwChaNum;

	memset(m_szChaName, 0, sizeof(char) * CHR_ID_LENGTH+1);
	memset(m_szIPin, 0, sizeof(char) * CHAR_INVENTORY_LOCK_PASS_SIZE+1);

	if (szChaName != NULL)
		StringCchCopy(m_szChaName, CHR_ID_LENGTH+1, szChaName);

	if (szIPin != NULL)
		StringCchCopy(m_szIPin, CHAR_INVENTORY_LOCK_PASS_SIZE+1, szIPin);
}

int CInventoryLockRecoverChange::Execute (CServer* pServer)
{
	if (pServer == NULL)
	{
		return DB_ERROR;
	}
	else
	{
		int nResult = m_pDbManager->InventoryLockRecoverChange(m_dwChaNum, m_szChaName, m_szIPin );

		GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_CHANGE_FROM_DB NetMsgDB2Fld;
		StringCchCopy ( NetMsgDB2Fld.szName, CHAR_SZNAME+1, m_szChaName );
		StringCchCopy ( NetMsgDB2Fld.szPin, CHAR_INVENTORY_LOCK_PASS_SIZE+1, m_szIPin );

		if ( nResult == DB_OK ) 
		{
			NetMsgDB2Fld.emFB = EMFB_INVENTORY_LOCK_RECOVER_CHANGE_DONE;
		}
		else 
		{
			NetMsgDB2Fld.emFB = EMFB_INVENTORY_LOCK_RECOVER_CHANGE_NOT_FOUND;
		}

		CFieldServer* pTemp = reinterpret_cast<CFieldServer*> (pServer);
		pTemp->InsertMsg(m_dwClient, (char*) &NetMsgDB2Fld );
		return nResult;
	}
}

CInventoryLockRecoverDelete::CInventoryLockRecoverDelete( DWORD dwClientID, DWORD dwChaNum, const char* szChaName )
{	
	m_dwClient = dwClientID;
	m_dwChaNum = dwChaNum;

	memset(m_szChaName, 0, sizeof(char) * CHR_ID_LENGTH+1);

	if (szChaName != NULL)
		StringCchCopy(m_szChaName, CHR_ID_LENGTH+1, szChaName);
}

int CInventoryLockRecoverDelete::Execute (CServer* pServer)
{
	if (pServer == NULL)
	{
		return DB_ERROR;
	}
	else
	{
		int nResult = m_pDbManager->InventoryLockRecoverDelete(m_dwChaNum, m_szChaName );

		GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_DELETE_FROM_DB NetMsgDB2Fld;
		StringCchCopy ( NetMsgDB2Fld.szName, CHAR_SZNAME+1, m_szChaName );

		if ( nResult == DB_OK ) 
		{
			NetMsgDB2Fld.emFB = EMFB_INVENTORY_LOCK_RECOVER_DELETE_DONE;
		}
		else 
		{
			NetMsgDB2Fld.emFB = EMFB_INVENTORY_LOCK_RECOVER_DELETE_NOT_FOUND;
		}

		CFieldServer* pTemp = reinterpret_cast<CFieldServer*> (pServer);
		pTemp->InsertMsg(m_dwClient, (char*) &NetMsgDB2Fld );
		return nResult;
	}
}