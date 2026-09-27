#ifndef GLLEVEL_REQUIRE_H
#define GLLEVEL_REQUIRE_H

#if _MSC_VER > 1000
#pragma once
#endif


#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"

#include "./GLDefine.h"
#include "./GLogicEx.h"

enum EMCDT_SIGN
{
	EMSIGN_SAME			= 0,	//	=
	EMSIGN_HIGHER		= 1,	//	<
	EMSIGN_LOWER		= 2,	//	>
	EMSIGN_SAMEHIGHER	= 3,	//	<=
	EMSIGN_SAMELOWER	= 4,	//	>=
	EMSIGN_ALWAYS		= 5,	//	Ç×»ó.
	EMSIGN_FROMTO		= 6,	//	~
	EMSIGN_REQSCHOOL	= 7,
	EMSIGN_SIZE			= 8
};

enum EMREQFAIL
{
	EMREQUIRE_COMPLETE	= 0,
	EMREQUIRE_LEVEL		= 1,
	EMREQUIRE_ITEM		= 2,
	EMREQUIRE_SKILL		= 3,
	EMREQUIRE_QUEST_COM	= 4,
	EMREQUIRE_QUEST_ACT	= 5,
	EMREQUIRE_LIVING	= 6,
	EMREQUIRE_BRIGHT	= 7,
	EMREQUIRE_ACTIVITY_POINT		= 8,	/* map requirement contri and activity p, Juver, 2018/02/11 */
	EMREQUIRE_CONTRIBUTION_POINT	= 9,	/* map requirement contri and activity p, Juver, 2018/02/11 */
	EMREQUIRE_SCHOOL	= 10,	/*map requirement school, CNDev 2021/11/02*/
	EMREQUIRE_SIZE		= 11,
};

struct SLEVEL_REQUIRE
{
	enum { VERSION = 0x0200, };

	EMCDT_SIGN	m_signLevel;
	WORD		m_wLevel;
	WORD		m_wLevel2;

	SNATIVEID	m_sActQuestID;
	SNATIVEID	m_sComQuestID;

	SNATIVEID	m_sSkillID;
	SNATIVEID	m_sItemID;
	
	EMCDT_SIGN	m_signBright;
	int			m_nBright;
	
	EMCDT_SIGN	m_signLiving;
	int			m_nLiving;
	
	bool		m_bPartyMbr;

	/* map requirement contri and activity p, Juver, 2018/02/11 */
	EMCDT_SIGN	m_signContributionPoint;
	EMCDT_SIGN	m_signActivityPoint;
	DWORD		m_dwContributionPoint;	
	DWORD		m_dwActivityPoint;

	SLEVEL_REQUIRE() :
		m_signLevel(EMSIGN_ALWAYS),
		m_wLevel(0),
		m_wLevel2(0),
		m_sActQuestID(false),
		m_sComQuestID(false),
		m_sSkillID(false),
		m_sItemID(false),
		m_signBright(EMSIGN_ALWAYS),
		m_nBright(0),
		m_signLiving(EMSIGN_ALWAYS),
		m_nLiving(0),
		m_bPartyMbr(false),

		/* map requirement contri and activity p, Juver, 2018/02/11 */
		m_signContributionPoint(EMSIGN_ALWAYS),
		m_signActivityPoint(EMSIGN_ALWAYS),
		m_dwContributionPoint(0),
		m_dwActivityPoint(0)
	{
	}

	BOOL SAVE ( CSerialFile &SFile );
	BOOL LOAD ( basestream &SFile );

	void LOAD_0101 ( basestream &SFile, DWORD dwSIZE );
	void LOAD_0102 ( basestream &SFile, DWORD dwSIZE );
	void LOAD_0103 ( basestream &SFile, DWORD dwSIZE );
	void LOAD_0200 ( basestream &SFile, DWORD dwSIZE );

	static VOID SaveCsvHead( std::fstream &SFile );
	VOID SaveCsv( std::fstream &SFile );
	VOID LoadCsv( CStringArray &StrArray );

	EMREQFAIL ISCOMPLETE ( GLCHARLOGIC * pCHARLOGIC ) const;
};

namespace COMMENT
{
	extern std::string CDT_SIGN[EMSIGN_SIZE];
	extern std::string CDT_SIGN_ID[EMSIGN_SIZE];
};

#endif // GLLEVEL_REQUIRE_H