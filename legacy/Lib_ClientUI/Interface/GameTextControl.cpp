#include "StdAfx.h"
#include "./GameTextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CGameTextMan& CGameTextMan::GetInstance()
{
	static CGameTextMan Instance;
	return Instance;
}

CGameTextMan::CGameTextMan ()
{
	memset( m_szPath, 0, sizeof( m_szPath ) );
}

CGameTextMan::~CGameTextMan ()
{
    Clear ( EM_GAME_WORD );
	Clear ( EM_GAME_IN_TEXT );
	Clear ( EM_GAME_EX_TEXT );
	Clear ( EM_SERVER_TEXT );
	//xml translation

	Clear ( EM_GAME_NPC_WORD );
	Clear ( EM_GAME_QUEST_WORD );
}

void CGameTextMan::Clear ( int nType )
{
	switch ( nType )
	{	
	case EM_GAME_WORD:		NS_TEXT_LOADER::ClearText ( m_GameWord.mapFlags );		return;
	case EM_GAME_IN_TEXT:	NS_TEXT_LOADER::ClearText ( m_GameInText.mapFlags );	return;
	case EM_GAME_EX_TEXT:	NS_TEXT_LOADER::ClearText ( m_GameExText.mapFlags );	return;
	case EM_SERVER_TEXT:	NS_TEXT_LOADER::ClearText ( m_ServerText.mapFlags );	return;

	//xml translation
	case EM_GAME_NPC_WORD:		NS_TEXT_LOADER::ClearText ( m_GameWordNpc.mapFlags );	return;
	case EM_GAME_QUEST_WORD:	NS_TEXT_LOADER::ClearText ( m_GameWordQuest.mapFlags );	return;
	}

	GASSERT ( 0 && "CGameTextMan::Clear() : 정의되지 않은 타입" );
}

BOOL CGameTextMan::LoadText ( const char* szFileName, int nType, BOOL bXML )
{
	CString strPath(m_szPath);
	strPath += szFileName;

	switch ( nType )
	{
	case EM_GAME_WORD:		return NS_TEXT_LOADER::LoadText( strPath.GetString(), SGAMEWORD::progVersion, m_GameWord.mapFlags, bXML );
	case EM_GAME_IN_TEXT:	return NS_TEXT_LOADER::LoadText( strPath.GetString(), SGAMEINTEXT::progVersion, m_GameInText.mapFlags, bXML );
	case EM_GAME_EX_TEXT:	return NS_TEXT_LOADER::LoadText( strPath.GetString(), SGAMEEXTEXT::progVersion, m_GameExText.mapFlags, bXML );
	case EM_SERVER_TEXT:	return NS_TEXT_LOADER::LoadText( strPath.GetString(), SSERVERTEXT::progVersion, m_ServerText.mapFlags, bXML, TRUE );
	//xml translation
	case EM_GAME_NPC_WORD:		return NS_TEXT_LOADER::LoadText( strPath.GetString(), SGAMEWORDNPC::progVersion, m_GameWordNpc.mapFlags, bXML );
	case EM_GAME_QUEST_WORD:	return NS_TEXT_LOADER::LoadText( strPath.GetString(), SGAMEWORDQUEST::progVersion, m_GameWordQuest.mapFlags, bXML );
	}

	GASSERT ( 0 && "CGameTextMan::LoadText() : 정의되지 않은 타입" );
	return FALSE;
}

const CString & CGameTextMan::GetGameWord ( std::string strKey, int Index )
{
	return GetText ( strKey, Index, EM_GAME_WORD );
}

const CString & CGameTextMan::GetGameInText ( std::string strKey, int Index )
{
	return GetText ( strKey, Index, EM_GAME_IN_TEXT );
}

const CString & CGameTextMan::GetGameExText ( std::string strKey, int Index )
{
	return GetText ( strKey, Index, EM_GAME_EX_TEXT );
}

const CString & CGameTextMan::GetServerText ( std::string strKey, int nIndex )
{
	return GetText ( strKey, nIndex, EM_SERVER_TEXT );
}

//xml translation
const CString & CGameTextMan::GetGameWordNpc ( std::string strKey, int Index )
{
	return GetText ( strKey, Index, EM_GAME_NPC_WORD );
}
const CString & CGameTextMan::GetGameWordQuest ( std::string strKey, int Index )
{
	return GetText ( strKey, Index, EM_GAME_QUEST_WORD );
}
const CString &	CGameTextMan::GetText ( std::string strKey, int Index, int nType )
{
	switch ( nType )
	{
	case EM_GAME_WORD:		return NS_TEXT_LOADER::GetText ( strKey, Index, m_GameWord.mapFlags );
	case EM_GAME_IN_TEXT:	return NS_TEXT_LOADER::GetText ( strKey, Index, m_GameInText.mapFlags );
	case EM_GAME_EX_TEXT:	return NS_TEXT_LOADER::GetText ( strKey, Index, m_GameExText.mapFlags );
	case EM_SERVER_TEXT:	return NS_TEXT_LOADER::GetText ( strKey, Index, m_ServerText.mapFlags );

	//xml translation
	case EM_GAME_NPC_WORD:		return NS_TEXT_LOADER::GetText ( strKey, Index, m_GameWordNpc.mapFlags );
	case EM_GAME_QUEST_WORD:	return NS_TEXT_LOADER::GetText ( strKey, Index, m_GameWordQuest.mapFlags );
	}

	GASSERT ( 0 && "CGameTextMan::GetText() : 정의되지 않은 타입" );

	static CString strEmpty; // MEMO : 에러시 반환되는 빈 스트링

	return strEmpty;
}

int	CGameTextMan::GetNumber ( std::string strKey, int Index, int nType )
{
	switch ( nType )
	{
	case EM_GAME_WORD:		return NS_TEXT_LOADER::GetNumber ( strKey, Index, m_GameWord.mapFlags );
	case EM_GAME_IN_TEXT:	return NS_TEXT_LOADER::GetNumber ( strKey, Index, m_GameInText.mapFlags );
	case EM_GAME_EX_TEXT:	return NS_TEXT_LOADER::GetNumber ( strKey, Index, m_GameExText.mapFlags );
	case EM_SERVER_TEXT:	return NS_TEXT_LOADER::GetNumber ( strKey, Index, m_ServerText.mapFlags );
	//xml translation
	case EM_GAME_NPC_WORD:		return NS_TEXT_LOADER::GetNumber ( strKey, Index, m_GameWordNpc.mapFlags );
	case EM_GAME_QUEST_WORD:	return NS_TEXT_LOADER::GetNumber ( strKey, Index, m_GameWordQuest.mapFlags );
	}

	GASSERT ( 0 && "CGameTextMan::GetNumber() : 정의되지 않은 타입" );
	return 0;
}

const char * ID2GAMEWORD( const char* szKeyword, int nIndex )
{
	return CGameTextMan::GetInstance().GetText( szKeyword, nIndex, CGameTextMan::EM_GAME_WORD );	
}

const char * ID2GAMEINTEXT( const char* szKeyword, int nIndex )
{
	return CGameTextMan::GetInstance().GetText( szKeyword, nIndex, CGameTextMan::EM_GAME_IN_TEXT );	
}

const char * ID2GAMEEXTEXT( const char* szKeyword, int nIndex )
{
	return CGameTextMan::GetInstance().GetText( szKeyword, nIndex, CGameTextMan::EM_GAME_EX_TEXT );	
}

const char * ID2SERVERTEXT( const char* szKeyword, int nIndex )
{
	return CGameTextMan::GetInstance().GetText( szKeyword, nIndex, CGameTextMan::EM_SERVER_TEXT );
}
//xml translation

const char * ID2GAMEWORDNPC( const char* szKeyword, int nIndex )
{
	return CGameTextMan::GetInstance().GetText( szKeyword, nIndex, CGameTextMan::EM_GAME_NPC_WORD );	
}
const char * ID2GAMEWORDQUEST( const char* szKeyword, int nIndex )
{
	return CGameTextMan::GetInstance().GetText( szKeyword, nIndex, CGameTextMan::EM_GAME_QUEST_WORD );	
}