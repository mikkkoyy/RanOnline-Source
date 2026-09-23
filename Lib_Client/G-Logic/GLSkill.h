#ifndef GLSKILL_H_
#define GLSKILL_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
//	GLSkill.h
//
//	Renderer / tool boundary for the skill data (GLSkillMan).
//
//	The portable skill data (GLSKILL) lives in GLSkillData.h and does not know
//	about any renderer.  This header adds the manager, which owns everything that
//	is not portable :
//
//		* the DirectX device handle the client hands over to the manager
//		* the animation / resource setup the skill data is validated against
//		* the editor (CWnd based) CSV import / export entry points
//
//	A DirectX device is therefore only required here, and only for the renderer
//	dependent part of the manager - never merely to hold skill data.
//

#include "./GLSkillData.h"

//
//	Note : Renderer / tool side dependency.
//
//	Because this header is the renderer / tool side of the boundary it keeps the
//	animation / resource header available, exactly as before, for the consumers
//	that need the animation metadata next to the skill manager (the skill editor
//	and the skill UI).  The portable skill data header GLSkillData.h does not
//	include it.
//
#include "../../Lib_Engine/Meshs/DxSkinAniMan.h"

//
//	Note : Opaque renderer handle.
//
//	GLSkillMan only stores the device pointer handed over by the renderer, it never
//	dereferences it, so the DirectX declaration itself is not needed to declare the
//	skill manager.  The DirectX device macro used by the client expands to a
//	pointer to this forward declared struct, therefore existing callers keep
//	working unchanged.
//
struct IDirect3DDevice9;
typedef IDirect3DDevice9* GLSkillDeviceHandle;

class GLSkillMan
{
public:
	enum
	{
		ENCODE_VER_OLD	= 0x0101,
		ENCODE_VER		= 0x0200,

		VERSION			= 0x0200,
		MAX_CLASSSKILL	= 80,
	};

	static const char* _FILEHEAD;
	static const char* _LOGFILE;
	static const char* _STRINGTABLE;

protected:
	char				m_szFileName[MAX_PATH];
	
	bool				m_bModify;
	PGLSKILL			m_pSkills[EMSKILLCLASS_NSIZE][MAX_CLASSSKILL];

	GLSkillDeviceHandle	m_pd3dDevice;

public:
	PGLSKILL GetData ( const WORD wClass, const WORD Index );
	PGLSKILL GetData ( const SNATIVEID &sID )	{ return GetData(sID.wMainID,sID.wSubID); }

	void	SetData ( WORD wClass, WORD Index, const PGLSKILL SkillData, bool binner=false );

public:
	WORD	FindFreeSkillIndex ( WORD wClass );
	BOOL	DeleteSkill ( WORD wClass, WORD Index );
	void	GetMaxSkill ( WORD& wClass, WORD& rIndex );	

public:
	bool	ValidData ();

public:
	bool	IsModify ()							{ return m_bModify; }

public:
	HRESULT OneTimeSceneInit ();
	HRESULT InitDeviceObjects ( GLSkillDeviceHandle pd3dDevice );
	HRESULT DeleteDeviceObjects ();
	HRESULT FinalCleanup ();

public:
	HRESULT LoadFile ( const char* szFile, bool bPastLoad );
	HRESULT SaveFile ( const char* szFile );

public:
	HRESULT	SyncStringTable();
	HRESULT SyncUpdateData ();

public:
	HRESULT SaveCsvFile( CWnd* pWnd );
	HRESULT LoadCsvFile( CWnd* pWnd );
	HRESULT LoadCsvFile_oldj( CWnd* pWnd );

public:
	GLSkillMan ();

public:
	~GLSkillMan ();

public:
	static GLSkillMan& GetInstance();
};




#endif // GLSKILL_H_
