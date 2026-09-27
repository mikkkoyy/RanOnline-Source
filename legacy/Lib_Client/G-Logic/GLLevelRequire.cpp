#include "stdafx.h"
#include "GLLevelRequire.h"

#include "./GLQuest.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

VOID SLEVEL_REQUIRE::SaveCsvHead( std::fstream &SFile )
{
	SFile << "Entry Req bPartyMbr" << ",";

	SFile << "Entry Req signLevel" << ",";
	SFile << "Entry Req wLevel" << ",";
	SFile << "Entry Req wLevel2" << ",";
	SFile << "Entry Req sItemID_MID" << ",";
	SFile << "Entry Req sItemID_SID" << ",";
	SFile << "Entry Req sSkillID_MID" << ",";
	SFile << "Entry Req sSkillID_SID" << ",";

	SFile << "Entry Req sComQuestID_MID" << ",";
	SFile << "Entry Req sComQuestID_SID" << ",";
	SFile << "Entry Req sActQuestID_MID" << ",";
	SFile << "Entry Req sActQuestID_SID" << ",";

	SFile << "Entry Req signLiving" << ",";
	SFile << "Entry Req nLiving" << ",";
	SFile << "Entry Req signBright" << ",";
	SFile << "Entry Req nBright" << ",";

	/* map requirement contri and activity p, Juver, 2018/02/11 */
	SFile << "Entry Req signContribution P" << ",";
	SFile << "Entry Req dwContribution P" << ",";
	SFile << "Entry Req signActivity P" << ",";
	SFile << "Entry Req dwActivity P" << ",";
}

VOID SLEVEL_REQUIRE::SaveCsv( std::fstream &SFile )
{
	SFile << m_bPartyMbr << ",";

	SFile << (DWORD)m_signLevel << ",";
	SFile << m_wLevel << ",";
	SFile << m_wLevel2 << ",";
	SFile << m_sItemID.wMainID << ",";
	SFile << m_sItemID.wSubID << ",";
	SFile << m_sSkillID.wMainID << ",";
	SFile << m_sSkillID.wSubID << ",";

	SFile << m_sComQuestID.wMainID << ",";
	SFile << m_sComQuestID.wSubID << ",";
	SFile << m_sActQuestID.wMainID << ",";
	SFile << m_sActQuestID.wSubID << ",";

	SFile << (DWORD)m_signLiving << ",";
	SFile << m_nLiving << ",";
	SFile << (DWORD)m_signBright << ",";
	SFile << m_nBright << ",";

	/* map requirement contri and activity p, Juver, 2018/02/11 */
	SFile << (DWORD)m_signContributionPoint << ",";
	SFile << m_dwContributionPoint << ",";
	SFile << (DWORD)m_signActivityPoint << ",";
	SFile << m_dwActivityPoint << ",";
}

VOID SLEVEL_REQUIRE::LoadCsv( CStringArray &StrArray )
{
	int iCsvCur = 2;
	m_bPartyMbr = (bool)atol( StrArray[ iCsvCur++ ] );

	m_signLevel = (EMCDT_SIGN)atol( StrArray[ iCsvCur++ ] );
	m_wLevel  = (WORD)atol( StrArray[ iCsvCur++ ] );
	m_wLevel2 = (WORD)atol( StrArray[ iCsvCur++ ] );
	m_sItemID.wMainID = (WORD)atol( StrArray[ iCsvCur++ ] );
	m_sItemID.wSubID  = (WORD)atol( StrArray[ iCsvCur++ ] );
	m_sSkillID.wMainID = (WORD)atol( StrArray[ iCsvCur++ ] );
	m_sSkillID.wSubID  = (WORD)atol( StrArray[ iCsvCur++ ] );

	m_sComQuestID.wMainID = (WORD)atol( StrArray[ iCsvCur++ ] );
	m_sComQuestID.wSubID  = (WORD)atol( StrArray[ iCsvCur++ ] );
	m_sActQuestID.wMainID = (WORD)atol( StrArray[ iCsvCur++ ] );
	m_sActQuestID.wSubID  = (WORD)atol( StrArray[ iCsvCur++ ] );

	m_signLiving = (EMCDT_SIGN)atol( StrArray[ iCsvCur++ ] );
	m_nLiving	 = atol( StrArray[ iCsvCur++ ] );
	m_signBright = (EMCDT_SIGN)atol( StrArray[ iCsvCur++ ] );
	m_nBright = atol( StrArray[ iCsvCur++ ] );

	/* map requirement contri and activity p, Juver, 2018/02/11 */
	m_signContributionPoint = (EMCDT_SIGN)atol( StrArray[ iCsvCur++ ] );
	m_dwContributionPoint = atol( StrArray[ iCsvCur++ ] );
	m_signActivityPoint = (EMCDT_SIGN)atol( StrArray[ iCsvCur++ ] );
	m_dwActivityPoint = atol( StrArray[ iCsvCur++ ] );
}

BOOL SLEVEL_REQUIRE::LOAD ( basestream &SFile )
{
	DWORD dwVer, dwSize;
	SFile >> dwVer;
	SFile >> dwSize;

	if ( dwVer==VERSION )
	{
		LOAD_0200( SFile, dwSize );
	}
	else if ( dwVer==0x0103 )
	{
		LOAD_0103( SFile, dwSize );
	}
	else if ( dwVer==0x0102 )
	{
		LOAD_0102( SFile, dwSize );
	}
	else if ( dwVer==0x0101 )
	{
		LOAD_0101( SFile, dwSize );
	}
	else
	{
		CDebugSet::ErrorVersion( "SLEVEL_REQUIRE::LOAD", dwVer );
		SFile.SetOffSet ( SFile.GetfTell()+dwSize );
	}

	return TRUE;
}

void SLEVEL_REQUIRE::LOAD_0101 ( basestream &SFile, DWORD dwSIZE )
{
	DWORD dwRead = SFile.GetfTell();

	DWORD dwData(0);

	SFile >> m_bPartyMbr;

	SFile >> dwData;		m_signLevel = (EMCDT_SIGN) dwData;
	SFile >> m_wLevel;
	m_wLevel2 = 0;
	SFile >> m_sItemID.dwID;
	SFile >> m_sSkillID.dwID;

	SFile >> m_sComQuestID.dwID;
	SFile >> m_sActQuestID.dwID;

	SFile >> dwData;		m_signLiving = (EMCDT_SIGN) dwData;
	SFile >> m_nLiving;
	SFile >> dwData;		m_signBright = (EMCDT_SIGN) dwData;
	SFile >> m_nBright;

	dwRead = SFile.GetfTell() - dwRead;
	
	if( dwRead != dwSIZE )
		CDebugSet::MsgBox( "SLEVEL_REQUIRE::LOAD_0101 Incorrect Read Size:%d Expected:%d", dwRead, dwSIZE );
}

void SLEVEL_REQUIRE::LOAD_0102 ( basestream &SFile, DWORD dwSIZE )
{
	DWORD dwRead = SFile.GetfTell();

	DWORD dwData(0);

	SFile >> m_bPartyMbr;

	SFile >> dwData;		m_signLevel = (EMCDT_SIGN) dwData;
	SFile >> m_wLevel;
	SFile >> m_wLevel2;
	SFile >> m_sItemID.dwID;
	SFile >> m_sSkillID.dwID;

	SFile >> m_sComQuestID.dwID;
	SFile >> m_sActQuestID.dwID;

	SFile >> dwData;		m_signLiving = (EMCDT_SIGN) dwData;
	SFile >> m_nLiving;
	SFile >> dwData;		m_signBright = (EMCDT_SIGN) dwData;
	SFile >> m_nBright;

	dwRead = SFile.GetfTell() - dwRead;
	
	if( dwRead != dwSIZE )
		CDebugSet::MsgBox( "SLEVEL_REQUIRE::LOAD_0102 Incorrect Read Size:%d Expected:%d", dwRead, dwSIZE );
}

void SLEVEL_REQUIRE::LOAD_0103 ( basestream &SFile, DWORD dwSIZE )
{
	DWORD dwRead = SFile.GetfTell();

	DWORD dwData(0);
	SFile >> dwData;		m_signLevel = (EMCDT_SIGN) dwData;
	SFile >> m_wLevel;
	SFile >> m_wLevel2;

	SFile >> m_sActQuestID.dwID;
	SFile >> m_sComQuestID.dwID;

	SFile >> m_sSkillID.dwID;
	SFile >> m_sItemID.dwID;
	
	SFile >> dwData;		m_signBright = (EMCDT_SIGN) dwData;
	SFile >> m_nBright;
	
	SFile >> dwData;		m_signLiving = (EMCDT_SIGN) dwData;
	SFile >> m_nLiving;
	
	SFile >> m_bPartyMbr;

	dwRead = SFile.GetfTell() - dwRead;
	
	if( dwRead != dwSIZE )
		CDebugSet::MsgBox( "SLEVEL_REQUIRE::LOAD_0103 Incorrect Read Size:%d Expected:%d", dwRead, dwSIZE );
}

void SLEVEL_REQUIRE::LOAD_0200 ( basestream &SFile, DWORD dwSIZE )
{
	DWORD dwRead = SFile.GetfTell();

	DWORD dwData(0);
	SFile >> dwData;		m_signLevel = (EMCDT_SIGN) dwData;
	SFile >> m_wLevel;
	SFile >> m_wLevel2;

	SFile >> m_sActQuestID.dwID;
	SFile >> m_sComQuestID.dwID;

	SFile >> m_sSkillID.dwID;
	SFile >> m_sItemID.dwID;

	SFile >> dwData;		m_signBright = (EMCDT_SIGN) dwData;
	SFile >> m_nBright;

	SFile >> dwData;		m_signLiving = (EMCDT_SIGN) dwData;
	SFile >> m_nLiving;

	SFile >> m_bPartyMbr;

	/* map requirement contri and activity p, Juver, 2018/02/11 */
	SFile >> dwData;		m_signContributionPoint = (EMCDT_SIGN) dwData;
	SFile >> dwData;		m_signActivityPoint = (EMCDT_SIGN) dwData;
	SFile >> m_dwContributionPoint;
	SFile >> m_dwActivityPoint;

	dwRead = SFile.GetfTell() - dwRead;

	if( dwRead != dwSIZE )
		CDebugSet::MsgBox( "SLEVEL_REQUIRE::LOAD_0103 Incorrect Read Size:%d Expected:%d", dwRead, dwSIZE );
}

BOOL SLEVEL_REQUIRE::SAVE ( CSerialFile &SFile )
{
	SFile << DWORD(VERSION);
	SFile.BeginBlock();
	{
		SFile << (DWORD)m_signLevel;
		SFile << m_wLevel;
		SFile << m_wLevel2;

		SFile << m_sActQuestID.dwID;
		SFile << m_sComQuestID.dwID;

		SFile << m_sSkillID.dwID;
		SFile << m_sItemID.dwID;

		SFile << (DWORD)m_signBright;
		SFile << m_nBright;

		SFile << (DWORD)m_signLiving;
		SFile << m_nLiving;

		SFile << m_bPartyMbr;

		/* map requirement contri and activity p, Juver, 2018/02/11 */
		SFile << (DWORD)m_signContributionPoint;
		SFile << (DWORD)m_signActivityPoint;
		SFile << m_dwContributionPoint;
		SFile << m_dwActivityPoint;
	}
	SFile.EndBlock();

	return TRUE;
}

template<class TYPE>
bool compare ( TYPE lvalue, EMCDT_SIGN emSign, TYPE rvalue )
{
	switch ( emSign )
	{
	case EMSIGN_ALWAYS:
		return true;

	case EMSIGN_SAME:		//	=
		return lvalue == rvalue;

	case EMSIGN_HIGHER:		//	<
		return lvalue < rvalue;

	case EMSIGN_LOWER:		//	>
		return lvalue > rvalue;

	case EMSIGN_SAMEHIGHER:	//	<=
		return lvalue <= rvalue;

	case EMSIGN_SAMELOWER:	//	>=
		return lvalue >= rvalue;
	};

	return false;
}

EMREQFAIL SLEVEL_REQUIRE::ISCOMPLETE ( GLCHARLOGIC * pCHARLOGIC ) const
{
	//m_bPartyMbr;

	bool bcomplete(false);

	if ( m_signLevel == EMSIGN_FROMTO )
	{
		if( m_wLevel  > pCHARLOGIC->m_wLevel ||	m_wLevel2 < pCHARLOGIC->m_wLevel ) return EMREQUIRE_LEVEL;
	}
	else if( m_signLevel == EMSIGN_REQSCHOOL )
	{
		if( m_wLevel != pCHARLOGIC->m_wSchool )		return EMREQUIRE_SCHOOL;
	}
	else
	{
		if ( ! compare ( m_wLevel, m_signLevel, pCHARLOGIC->m_wLevel ) )	return EMREQUIRE_LEVEL;
	}


	if ( m_sItemID!=SNATIVEID(false) )
	{
		bcomplete = false;
		SINVENITEM* pInvenItem = pCHARLOGIC->m_cInventory.FindItem ( m_sItemID );
		if ( pInvenItem )
		{
			bcomplete = true;
		}
		else
		{
			for ( int i=0; i<SLOT_TSIZE; ++i )
			{
				EMSLOT emSLOT = (EMSLOT) i;
				if ( pCHARLOGIC->VALID_SLOT_ITEM(emSLOT) )
				{
					const SITEMCUSTOM& sITEM = pCHARLOGIC->GET_SLOT_ITEM ( emSLOT );
					if ( sITEM.sNativeID == m_sItemID )		bcomplete = true;
					break;
				}
			}
		}

		if ( !bcomplete )												return EMREQUIRE_ITEM;
	}

	if ( m_sSkillID!=SNATIVEID(false) )
	{
		SCHARDATA2::SKILL_MAP_ITER iter = pCHARLOGIC->m_ExpSkills.find ( m_sSkillID.dwID );
		if ( pCHARLOGIC->m_ExpSkills.end() == iter )					return EMREQUIRE_SKILL;
	}

	if ( m_sComQuestID!=SNATIVEID(false) )
	{
		GLQUESTPROG *pPROG = pCHARLOGIC->m_cQuestPlay.FindEnd ( m_sComQuestID.dwID );
		if ( !pPROG || !pPROG->m_bCOMPLETE )							return EMREQUIRE_QUEST_COM;
	}

	if ( m_sActQuestID!=SNATIVEID(false) )
	{
		GLQUESTPROG *pPROG = pCHARLOGIC->m_cQuestPlay.FindProc ( m_sActQuestID.dwID );
		if ( !pPROG )													return EMREQUIRE_QUEST_ACT;
	}

	if ( ! compare ( m_nLiving, m_signLiving, pCHARLOGIC->m_nLiving ) )	return EMREQUIRE_LIVING;
	if ( ! compare ( m_nBright, m_signBright, pCHARLOGIC->m_nBright ) )	return EMREQUIRE_BRIGHT;

	/* map requirement contri and activity p, Juver, 2018/02/11 */
	if ( ! compare ( m_dwActivityPoint, m_signActivityPoint, pCHARLOGIC->m_dwActivityPoint ) )							
		return EMREQUIRE_ACTIVITY_POINT;

	/* map requirement contri and activity p, Juver, 2018/02/11 */
	if ( ! compare ( m_dwContributionPoint, m_signContributionPoint, (DWORD)pCHARLOGIC->m_llContributionPoint  ) )		
		return EMREQUIRE_CONTRIBUTION_POINT;

	return EMREQUIRE_COMPLETE;
}

namespace COMMENT
{
	std::string CDT_SIGN[EMSIGN_SIZE] =
	{
		"=",
		">",
		"<",
		">=",
		"<=",
		"Any",
		"~"
	};

	std::string CDT_SIGN_ID[EMSIGN_SIZE] =
	{
		"EMSIGN_SAME",
		"EMSIGN_HIGHER",
		"EMSIGN_LOWER",
		"EMSIGN_SAMEHIGHER",
		"EMSIGN_SAMELOWER",
		"EMSIGN_ALWAYS",
		"EMSIGN_FROMTO"
	};
};