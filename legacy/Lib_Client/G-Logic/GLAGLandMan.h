#pragma once

#include <set>
#include "./GLLevelFile.h"
#include "../../Lib_Engine/DxOctree/DxLandGateMan.h"

class GLAGLandMan : public GLLevelFile
{
public:
	typedef std::set<DWORD>		SETPC;
	typedef SETPC::iterator		SETPC_ITER;

public:
	SETPC		m_setPC;
	DWORD		m_dwClubMapID;				//	선도 및 CDM ID
	bool		m_bGuidBattleMap;			//	선도 클럽 전투 존.
	bool		m_bClubDeathMatchMap;		//	클럽데스매치 맵
	
	bool		m_bClubWarLock;				/*clubwar map lock, Juver, 2017/06/27 */
	bool		m_bPVPTyrannyMap;			/*pvp tyranny, Juver, 2017/08/25 */
	bool		m_bPVPSchoolWarsMap;		/*school wars, Juver, 2018/01/19 */
	bool		m_bPVPCaptureTheFlagMap;	/*pvp capture the flag, Juver, 2018/01/24 */

public:
	DxLandGateMan* GetLandGateMan ()				{ return &m_cLandGateMan; }

public:
	bool DropPC ( DWORD dwID );
	bool DropOutPC ( DWORD dwID );

public:
	void SENDTOALLCLIENT ( LPVOID nmg );

protected:
	BOOL LoadWldFile ( const char* szWldFile, bool bLandGateLoad );

public:
	BOOL LoadFile ( const char *szFile );
	void CleanUp ();

public:
	GLAGLandMan(void);
	~GLAGLandMan(void);
};
