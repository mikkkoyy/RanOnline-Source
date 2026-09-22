/*!
 * \file s_CIPFilter.h
 *
 * \author Juver
 * \date April 2017
 *
 * 
 */


#pragma once

#include "WinSock2.h"
#include <set>

#define IPFILTER_CLEANUP_TIME	15000

class CServer;
class CConsoleMessage;

class CIPFilter
{
public:
	static CIPFilter* GetInstance();
	static void	ReleaseInstance();


private:
	static CIPFilter* SelfInstance;

public:
	CIPFilter();
	~CIPFilter();

public:
	typedef std::set<std::string>	SET_IP;
	typedef SET_IP::iterator		SET_IP_ITER;

private:
	CRITICAL_SECTION		m_CSPCLock;
	ULONGLONG	m_llBlockRef;
	ULONGLONG	m_llKnownRef;
	SET_IP		m_setIPBlock;
	SET_IP		m_setIPKnown;

public:
	CServer*			m_pServer;
	CConsoleMessage*	m_pConsole;

public:
	void		SetServer( CServer* _pServer );
	void		SetConsole( CConsoleMessage* _pConsole );
	ULONGLONG	GetRefBlocked()	{	return m_llBlockRef;	}
	ULONGLONG	GetRefKnown()	{	return m_llKnownRef;	}
	BOOL		IsIPBlocked( SOCKET _sSocket );
	BOOL		IsIPBlocked( std::string _strIP );
	BOOL		IsIPKnown( std::string _strIP );
	BOOL		IsBlocked( std::string _strIP );
	/*dmk14 blocker*/
	void		AddIPBlock( std::string _strIP, bool _bBroadcast = false, bool _bFromFile = false );
	void		AddIPKnown( std::string _strIP, bool _bBroadcast = false );
	void		RemoveIPBlock( std::string _strIP, bool _bBroadcast = false );
	void		RemoveIPKnown( std::string _strIP, bool _bBroadcast = false );
	DWORD		GetIPBlockNum()	{	return (DWORD) m_setIPBlock.size();	}
	DWORD		GetIPKnownNum()	{	return (DWORD) m_setIPKnown.size();	}
	void		DoListSave();

public:
	void		ListLoad();
	void		ListSave();

};



