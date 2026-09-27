#ifndef GLSKILL_DATA_H_
#define GLSKILL_DATA_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
//	GLSkillData.h
//
//	Portable skill data boundary.
//
//	GLSKILL carries skill gameplay data only : skill ids, learn/apply parameters,
//	timing values, requirements and effect parameters, together with the file
//	layout used to serialize them.
//
//	This header, and everything it pulls in, must stay independent of any
//	renderer :
//
//		* no DirectX device, texture, mesh or shader type
//		* no animation / resource manager (DxSkinAniMan, DxSkinChar, ...)
//		* no editor (MFC dialog) entry point
//
//	Renderer / resource management and the editor CSV import / export live on the
//	other side of the boundary, in GLSkill.h (GLSkillMan).
//
//	                +----------------------+
//	                | Portable Skill Data  |	<-- GLSkillData.h
//	                | GLSKILL              |
//	                +----------+-----------+
//	                           |
//	                +----------v-----------+
//	                | Renderer / Tool      |	<-- GLSkill.h (GLSkillMan)
//	                | DirectX device       |
//	                | animation resources  |
//	                | editor CSV           |
//	                +----------------------+
//
//	Note : Remaining coupling (next step).
//
//	GLSkillExData.h still includes Lib_Engine/Meshs/DxAniKeys.h for the
//	EMANI_MAINTYPE / EMANI_SUBTYPE enums, and GLItemDef.h pulls in
//	Lib_Engine/Meshs/DxPieceDefine.h.  No DirectX device, no renderer object and
//	no animation manager is reachable from here any more, but those two mesh
//	headers keep D3DXVECTOR3 / TCHAR visible, so this chain is not yet free of
//	the Windows platform.  Removing them means relocating the animation type
//	enums, which changes renderer headers and every consumer, so it is
//	deliberately left out of this step.
//

#include "./GLItemDef.h"
#include "./GLCharDefine.h"

#include "../../Lib_Engine/Common/basestream.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"

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

#endif // GLSKILL_DATA_H_
