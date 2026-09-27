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

CEquipmentLockEnable::CEquipmentLockEnable( DWORD dwClientID, DWORD dwChaNum, const char* szChaName, const char* szEPin )
{	
	m_dwClient = dwClientID;
	m_dwChaNum = dwChaNum;

	memset(m_szChaName, 0, sizeof(char) * CHR_ID_LENGTH+1);
	memset(m_szEPin, 0, sizeof(char) * CHAR_EQUIPMENT_LOCK_PASS_SIZE+1);

	if (szChaName != NULL)
		StringCchCopy(m_szChaName, CHR_ID_LENGTH+1, szChaName);

	if (szEPin != NULL)
		StringCchCopy(m_szEPin, CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, szEPin);
}

int CEquipmentLockEnable::Execute (CServer* pServer)
{
	if (pServer == NULL)
	{
		return DB_ERROR;
	}
	else
	{
		int nResult = m_pDbManager->EquipmentLockEnable(m_dwChaNum, m_szChaName, m_szEPin );

		GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_ENABLE_FROM_DB NetMsgDB2Fld;
		StringCchCopy ( NetMsgDB2Fld.szName, CHAR_SZNAME+1, m_szChaName );
		StringCchCopy ( NetMsgDB2Fld.szPin, CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, m_szEPin );

		if ( nResult == DB_OK ) 
		{
			NetMsgDB2Fld.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_DONE;
		}
		else if( nResult == 1 )
		{
			NetMsgDB2Fld.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_ALREADY_ENABLED_DB;
		}
		else if( nResult == 2 )
		{
			NetMsgDB2Fld.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_CHAR_NOT_EXIST;
		}
		else 
		{
			NetMsgDB2Fld.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_FAILED;
		}

		CFieldServer* pTemp = reinterpret_cast<CFieldServer*> (pServer);
		pTemp->InsertMsg(m_dwClient, (char*) &NetMsgDB2Fld );
		return nResult;
	}
}

CEquipmentLockInput::CEquipmentLockInput( DWORD dwClientID, DWORD dwChaNum, const char* szChaName, const char* szEPin )
{	
	m_dwClient = dwClientID;
	m_dwChaNum = dwChaNum;

	memset(m_szChaName, 0, sizeof(char) * CHR_ID_LENGTH+1);
	memset(m_szEPin, 0, sizeof(char) * CHAR_EQUIPMENT_LOCK_PASS_SIZE+1);

	if (szChaName != NULL)
		StringCchCopy(m_szChaName, CHR_ID_LENGTH+1, szChaName);

	if (szEPin != NULL)
		StringCchCopy(m_szEPin, CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, szEPin);
}

int CEquipmentLockInput::Execute (CServer* pServer)
{
	if (pServer == NULL)
	{
		return DB_ERROR;
	}
	else
	{
		int nResult = m_pDbManager->EquipmentLockInput(m_dwChaNum, m_szChaName, m_szEPin );

		GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_INPUT_FROM_DB NetMsgDB2Fld;
		StringCchCopy ( NetMsgDB2Fld.szName, CHAR_SZNAME+1, m_szChaName );
		StringCchCopy ( NetMsgDB2Fld.szPin, CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, m_szEPin );

		if ( nResult == DB_OK ) 
		{
			NetMsgDB2Fld.emFB = EMFB_EQUIPMENT_LOCK_INPUT_DONE;
		}
		else if( nResult == 1 )
		{
			NetMsgDB2Fld.emFB = EMFB_EQUIPMENT_LOCK_INPUT_DATA_NOT_EXIST;
		}
		else 
		{
			NetMsgDB2Fld.emFB = EMFB_EQUIPMENT_LOCK_INPUT_FAILED;
		}

		CFieldServer* pTemp = reinterpret_cast<CFieldServer*> (pServer);
		pTemp->InsertMsg(m_dwClient, (char*) &NetMsgDB2Fld );
		return nResult;
	}
}

CEquipmentLockRecover::CEquipmentLockRecover( DWORD dwClientID, DWORD dwChaNum, const char* szChaName )
{	
	m_dwClient = dwClientID;
	m_dwChaNum = dwChaNum;

	memset(m_szChaName, 0, sizeof(char) * CHR_ID_LENGTH+1);

	if (szChaName != NULL)
		StringCchCopy(m_szChaName, CHR_ID_LENGTH+1, szChaName);
}

int CEquipmentLockRecover::Execute (CServer* pServer)
{
	if (pServer == NULL)
	{
		return DB_ERROR;
	}
	else
	{
		std::string strPin("");
		int nResult = m_pDbManager->EquipmentLockRecover(m_dwChaNum, m_szChaName, strPin );

		GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_FROM_DB NetMsgDB2Fld;
		StringCchCopy ( NetMsgDB2Fld.szName, CHAR_SZNAME+1, m_szChaName );
		StringCchCopy ( NetMsgDB2Fld.szPin, CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, strPin.c_str() );

		if ( nResult == DB_OK ) 
		{
			NetMsgDB2Fld.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_DONE;
		}else {
			NetMsgDB2Fld.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_NOT_EXIST;
		}

		CFieldServer* pTemp = reinterpret_cast<CFieldServer*> (pServer);
		pTemp->InsertMsg(m_dwClient, (char*) &NetMsgDB2Fld );
		return nResult;
	}
}

CEquipmentLockRecoverChange::CEquipmentLockRecoverChange( DWORD dwClientID, DWORD dwChaNum, const char* szChaName, const char* szEPin )
{	
	m_dwClient = dwClientID;
	m_dwChaNum = dwChaNum;

	memset(m_szChaName, 0, sizeof(char) * CHR_ID_LENGTH+1);
	memset(m_szEPin, 0, sizeof(char) * CHAR_EQUIPMENT_LOCK_PASS_SIZE+1);

	if (szChaName != NULL)
		StringCchCopy(m_szChaName, CHR_ID_LENGTH+1, szChaName);

	if (szEPin != NULL)
		StringCchCopy(m_szEPin, CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, szEPin);
}

int CEquipmentLockRecoverChange::Execute (CServer* pServer)
{
	if (pServer == NULL)
	{
		return DB_ERROR;
	}
	else
	{
		int nResult = m_pDbManager->EquipmentLockRecoverChange(m_dwChaNum, m_szChaName, m_szEPin );

		GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE_FROM_DB NetMsgDB2Fld;
		StringCchCopy ( NetMsgDB2Fld.szName, CHAR_SZNAME+1, m_szChaName );
		StringCchCopy ( NetMsgDB2Fld.szPin, CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, m_szEPin );

		if ( nResult == DB_OK ) 
		{
			NetMsgDB2Fld.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_DONE;
		}
		else 
		{
			NetMsgDB2Fld.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_NOT_FOUND;
		}

		CFieldServer* pTemp = reinterpret_cast<CFieldServer*> (pServer);
		pTemp->InsertMsg(m_dwClient, (char*) &NetMsgDB2Fld );
		return nResult;
	}
}

CEquipmentLockRecoverDelete::CEquipmentLockRecoverDelete( DWORD dwClientID, DWORD dwChaNum, const char* szChaName )
{	
	m_dwClient = dwClientID;
	m_dwChaNum = dwChaNum;

	memset(m_szChaName, 0, sizeof(char) * CHR_ID_LENGTH+1);

	if (szChaName != NULL)
		StringCchCopy(m_szChaName, CHR_ID_LENGTH+1, szChaName);
}

int CEquipmentLockRecoverDelete::Execute (CServer* pServer)
{
	if (pServer == NULL)
	{
		return DB_ERROR;
	}
	else
	{
		int nResult = m_pDbManager->EquipmentLockRecoverDelete(m_dwChaNum, m_szChaName );

		GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE_FROM_DB NetMsgDB2Fld;
		StringCchCopy ( NetMsgDB2Fld.szName, CHAR_SZNAME+1, m_szChaName );

		if ( nResult == DB_OK ) 
		{
			NetMsgDB2Fld.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_DELETE_DONE;
		}
		else 
		{
			NetMsgDB2Fld.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_DELETE_NOT_FOUND;
		}

		CFieldServer* pTemp = reinterpret_cast<CFieldServer*> (pServer);
		pTemp->InsertMsg(m_dwClient, (char*) &NetMsgDB2Fld );
		return nResult;
	}
}