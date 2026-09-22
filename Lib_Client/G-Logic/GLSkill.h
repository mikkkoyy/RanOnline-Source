#ifndef GLSKILL_H_
#define GLSKILL_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "./GLItemDef.h"
#include "./GLCharDefine.h"

#include "../../Lib_Engine/Common/basestream.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"
#include "../../Lib_Engine/Meshs/DxSkinAniMan.h"

#include "./GLSkillBasic.h"
#include "./GLSkillApply.h"
#include "./GLSkillLearn.h"
#include "./GLSkillExData.h"
#include "./GLSkillSpecial.h"

#define CHECK_FLAG_SKILL(dwBitFlag, iRealValue) (dwBitFlag & (1 << iRealValue) ? true : false)

struct GLSKILL
{
	enum
	{
		VERSION			= 0x0100,

		FILE_SBASIC			= 1,
		FILE_SAPPLY			= 2,
		FILE_SLEARN			= 3,
		FILE_SEXT_DATA		= 4,
		FILE_SPECIAL_SKILL = 5,

		FILE_END_DATA	= 0xEDEDEDED,
	};

	SKILL::SSKILLBASIC	  m_sBASIC;
	SKILL::SAPPLY		  m_sAPPLY;
	SKILL::SLEARN		  m_sLEARN;
	SKILL::SEXT_DATA	  m_sEXT_DATA;
	SKILL::SSPECIAL_SKILL m_sSPECIAL_SKILL;

	GLSKILL ()
	{
	}

	BOOL SaveFile ( CSerialFile &SFile );
	BOOL LoadFile ( basestream &SFile, bool bPastLoad );

	static VOID SaveCsvHead ( std::fstream &SFile );
	VOID SaveCsv ( std::fstream &SFile );
	VOID LoadCsv ( CStringArray &StrArray, int iLine );
	VOID LoadCsv_oldj ( CStringArray &StrArray, int iLine );
	GLSKILL*		GetSecondSkill();

	GLSKILL& operator= ( GLSKILL &value )
	{
		m_sBASIC		 = value.m_sBASIC;
		m_sAPPLY		 = value.m_sAPPLY;
		m_sLEARN		 = value.m_sLEARN;
		m_sEXT_DATA		 = value.m_sEXT_DATA;
		m_sSPECIAL_SKILL = value.m_sSPECIAL_SKILL;

		return *this;
	}

	bool	IsSkillFact ();		//	지속성 스킬
	const char* GetName();
    const std::string GetNameStr() const;
	const char* GetDesc();

	inline SNATIVEID	GetId() const { return m_sBASIC.GetId(); };

};
typedef GLSKILL* PGLSKILL;

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

	LPDIRECT3DDEVICEQ	m_pd3dDevice;

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
	HRESULT InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );
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