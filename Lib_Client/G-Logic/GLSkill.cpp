#include "stdafx.h"
#include "./GLSkill.h"
#include "./GLGaeaServer.h"
#include "./GLAgentServer.h"
#include "./GLogicData.h"
#include "./GLStringTable.h"

#include "../NpcTalk/NpcDialogueSet.h"

#include "../../Lib_Engine/Common/SeqRandom.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"
#include "../../Lib_Engine/Common/SUBPATH.h"
#include "../../Lib_Engine/DxOctree/DxLandMan.h"
#include "../../Lib_Engine/DxResponseMan.h"
#include "../../Lib_Engine/G-Logic/GLOGIC.h"
#include "../../Lib_Engine/G-Logic/GLPeriod.h"

/*ABL system, Juver, 2017/05/29 */
#include "../../Lib_Engine/Meshs/DxAttBoneData.h"
#include "../../Lib_Engine/Meshs/DxAttBoneLink.h"

/*vehicle system, Juver, 2017/08/06 */
#include "../../Lib_Engine/Meshs/DxVehicleData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GLSkillMan& GLSkillMan::GetInstance()
{
	static GLSkillMan Instance;
	return Instance;
}

const char* GLSkillMan::_FILEHEAD = "GLSKILL";

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
BOOL GLSKILL::SaveFile ( CSerialFile &SFile )
{
	SFile << DWORD(VERSION);

	SFile << (DWORD)FILE_SBASIC;
	{
		SFile << DWORD(SKILL::SSKILLBASIC::VERSION);
		SFile << (DWORD)sizeof(SKILL::SSKILLBASIC);

		CString cstrName;
		cstrName.Format( _T("SN_%03d_%03d"), m_sBASIC.sNATIVEID.wMainID, m_sBASIC.sNATIVEID.wSubID ); // by 경대
		StringCchCopy( m_sBASIC.szNAME, SKILL::MAX_SZNAME, cstrName.GetString() );
		SFile.WriteBuffer ( &m_sBASIC, sizeof(SKILL::SSKILLBASIC) );
	}

	SFile << (DWORD)FILE_SAPPLY;
	{
		SFile << DWORD(SKILL::SAPPLY::VERSION);
		SFile << (DWORD)sizeof(SKILL::SAPPLY);
		m_sAPPLY.SAVE( SFile );
	}

	SFile << (DWORD)FILE_SLEARN;
	{
		SFile << DWORD(SKILL::SLEARN::VERSION);
		SFile << (DWORD)sizeof(SKILL::SLEARN);
		SFile.WriteBuffer ( &m_sLEARN, sizeof(SKILL::SLEARN) );
	}

	SFile << (DWORD)FILE_SEXT_DATA;
	{
		SFile << DWORD(SKILL::SEXT_DATA::VERSION);
		SFile << (DWORD)sizeof(SKILL::SEXT_DATA);
		
		CString cstrName;
		cstrName.Format( _T("SD_%03d_%03d"), m_sBASIC.sNATIVEID.wMainID , m_sBASIC.sNATIVEID.wSubID ); // by 경대
		m_sEXT_DATA.strCOMMENTS = cstrName.GetString();
		m_sEXT_DATA.SAVE ( SFile );
	}

	SFile << (DWORD)FILE_SPECIAL_SKILL;
	{
		SFile << DWORD(SKILL::SSPECIAL_SKILL::VERSION);
		SFile << (DWORD)sizeof(SKILL::SSPECIAL_SKILL);
		m_sSPECIAL_SKILL.SAVE( SFile );
	}
	
	SFile << (DWORD)FILE_END_DATA;

	return TRUE;
}

BOOL GLSKILL::LoadFile ( basestream &SFile, bool bPastLoad )
{
	DWORD dwVersion;
	SFile >> dwVersion;
	
	if ( dwVersion > VERSION )
	{
		CDebugSet::ErrorVersion( "GLSKILL::LoadFile", dwVersion );
		return FALSE;
	}

	DWORD dwDataType;
	DWORD dwVer=0, dwSize=0;

	SFile >> dwDataType;
	while ( dwDataType!=FILE_END_DATA )
	{
		switch ( dwDataType )
		{
		case FILE_SBASIC:
			{
				SFile >> dwVer;
				SFile >> dwSize;

				if( bPastLoad )
				{
					if ( dwVer==0x0100 )
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC_100)==dwSize);

						SKILL::SSKILLBASIC_100 sOldData;
						SFile.ReadBuffer ( &sOldData, sizeof(SKILL::SSKILLBASIC_100) );
						m_sBASIC.Assign ( sOldData );
					}
					else if ( dwVer==0x0101 )
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC_101)==dwSize);

						SKILL::SSKILLBASIC_101 sOldData;
						SFile.ReadBuffer ( &sOldData, sizeof(SKILL::SSKILLBASIC_101) );
						m_sBASIC.Assign ( sOldData );
					}
					else if ( dwVer==0x0102 )
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC_102)==dwSize);

						SKILL::SSKILLBASIC_102 sOldData;
						SFile.ReadBuffer ( &sOldData, sizeof(SKILL::SSKILLBASIC_102) );
						m_sBASIC.Assign ( sOldData );
					}
					else if ( dwVer==0x0103 )
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC_103)==dwSize);

						SKILL::SSKILLBASIC_103 sOldData;
						SFile.ReadBuffer ( &sOldData, sizeof(SKILL::SSKILLBASIC_103) );
						m_sBASIC.Assign ( sOldData );
					}
					else if (dwVer==0x0104)
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC_105)==dwSize);

						SKILL::SSKILLBASIC_105 sOldData;
						SFile.ReadBuffer ( &m_sBASIC, sizeof(SKILL::SSKILLBASIC_105) );
						m_sBASIC.Assign ( sOldData );
					}
					else if (dwVer==0x0105)
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC_105)==dwSize);

						SKILL::SSKILLBASIC_105 sOldData;
						SFile.ReadBuffer ( &sOldData, sizeof(SKILL::SSKILLBASIC_105) );
						m_sBASIC.Assign ( sOldData );
					}
					else if (dwVer==0x0106)
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC_106)==dwSize);

						SKILL::SSKILLBASIC_106 sOldData;
						SFile.ReadBuffer ( &sOldData, sizeof(SKILL::SSKILLBASIC_106) );
						m_sBASIC.Assign ( sOldData );
					}
					else if (dwVer==0x0107)
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC_107)==dwSize);

						SKILL::SSKILLBASIC_107 sOldData;
						SFile.ReadBuffer ( &sOldData, sizeof(SKILL::SSKILLBASIC_107) );
						m_sBASIC.Assign ( sOldData );
					}
					else if (dwVer==0x0108)
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC_108)==dwSize);

						SKILL::SSKILLBASIC_108 sOldData;
						SFile.ReadBuffer ( &sOldData, sizeof(SKILL::SSKILLBASIC_108) );
						m_sBASIC.Assign ( sOldData );
					}
					else if (dwVer==0x0109)
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC_109)==dwSize);

						SKILL::SSKILLBASIC_109 sOldData;
						SFile.ReadBuffer ( &sOldData, sizeof(SKILL::SSKILLBASIC_109) );
						m_sBASIC.Assign ( sOldData );
					}
					else if (dwVer==0x0110)
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC_110)==dwSize);

						SKILL::SSKILLBASIC_110 sOldData;
						SFile.ReadBuffer ( &sOldData, sizeof(SKILL::SSKILLBASIC_110) );
						m_sBASIC.Assign ( sOldData );
					}
					else if (dwVer==0x0111)
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC_111)==dwSize);

						SKILL::SSKILLBASIC_111 sOldData;
						SFile.ReadBuffer ( &sOldData, sizeof(SKILL::SSKILLBASIC_111) );
						m_sBASIC.Assign ( sOldData );
					}
					else if (dwVer==0x0112)
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC_112)==dwSize);

						SKILL::SSKILLBASIC_112 sOldData;
						SFile.ReadBuffer ( &sOldData, sizeof(SKILL::SSKILLBASIC_112) );
						m_sBASIC.Assign ( sOldData );
					}
					else if (dwVer==SKILL::SSKILLBASIC_113::VERSION)
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC_113)==dwSize);

						SKILL::SSKILLBASIC_113 sOldData;
						SFile.ReadBuffer ( &sOldData, sizeof(SKILL::SSKILLBASIC_113) );
						m_sBASIC.Assign ( sOldData );
					}
					else if (dwVer==SKILL::SSKILLBASIC_114::VERSION)
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC_114)==dwSize);

						SKILL::SSKILLBASIC_114 sOldData;
						SFile.ReadBuffer ( &sOldData, sizeof(SKILL::SSKILLBASIC_114) );
						m_sBASIC.Assign ( sOldData );
					}
					else if (dwVer==SKILL::SSKILLBASIC::VERSION)
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC)==dwSize);
						SFile.ReadBuffer ( &m_sBASIC, sizeof(SKILL::SSKILLBASIC) );
					}
					else
					{
						CDebugSet::ErrorVersion( "SKILL::SSKILLBASIC Pastload", dwVer );
						SFile.SetOffSet(SFile.GetfTell()+dwSize);
					}
				}else{
					if (dwVer==SKILL::SSKILLBASIC::VERSION)
					{
						GASSERT(sizeof(SKILL::SSKILLBASIC)==dwSize);
						SFile.ReadBuffer ( &m_sBASIC, sizeof(SKILL::SSKILLBASIC) );
					}
					else
					{
						CDebugSet::ErrorVersion( "SKILL::SSKILLBASIC", dwVer );
						SFile.SetOffSet(SFile.GetfTell()+dwSize);
						return FALSE;
					}
				}	
			}break;

		case FILE_SAPPLY:
			{
				SFile >> dwVer;
				SFile >> dwSize;

				if ( dwVer==0x0100 )
				{
					SKILL::SAPPLY_100 sOldApply;
					GASSERT(0x0100==dwVer&&sizeof(SKILL::SAPPLY_100)==dwSize);

					SFile.ReadBuffer ( &sOldApply, sizeof(SKILL::SAPPLY_100) );
					m_sAPPLY.Assign ( sOldApply );
				}
				else if ( dwVer==0x0101 )
				{
					SKILL::SAPPLY_101 sOldApply;
					GASSERT(0x0101==dwVer&&sizeof(SKILL::SAPPLY_101)==dwSize);

					SFile.ReadBuffer ( &sOldApply, sizeof(SKILL::SAPPLY_101) );
					m_sAPPLY.Assign ( sOldApply );
				}
				else if ( dwVer==0x0102 )
				{
					SKILL::SAPPLY_102 sOldApply;
					GASSERT(0x0102==dwVer&&sizeof(SKILL::SAPPLY_102)==dwSize);

					SFile.ReadBuffer ( &sOldApply, sizeof(SKILL::SAPPLY_102) );
					m_sAPPLY.Assign ( sOldApply );
				}
				else if ( dwVer==0x0103 )
				{
					SKILL::SAPPLY_103 sOldApply;
					GASSERT(0x0103==dwVer&&sizeof(SKILL::SAPPLY_103)==dwSize);

					SFile.ReadBuffer ( &sOldApply, sizeof(SKILL::SAPPLY_103) );
					m_sAPPLY.Assign ( sOldApply );
				}
				else if ( dwVer==0x0200 )
				{
					SKILL::SAPPLY_200 sOldApply;
					GASSERT(0x0200==dwVer&&sizeof(SKILL::SAPPLY_200)==dwSize);
					SFile.ReadBuffer ( &sOldApply, sizeof(SKILL::SAPPLY_200) );
					m_sAPPLY.Assign ( sOldApply );
				}
				else if ( dwVer==0x0106 )
				{
					m_sAPPLY.LOAD_0106( SFile );
				}
				else if ( dwVer==0x0107 )
				{
					m_sAPPLY.LOAD_0107( SFile );
				}
				else if ( dwVer==0x0108 )
				{
					m_sAPPLY.LOAD_0108( SFile );
				}
				else if ( dwVer==0x0109 )
				{
					m_sAPPLY.LOAD_0109( SFile );
				}
				else if ( dwVer==0x0110 )
				{
					m_sAPPLY.LOAD_0110( SFile );
				}
				else if ( dwVer==0x0111 )
				{
					m_sAPPLY.LOAD_0111( SFile );
				}
				else if ( dwVer==0x0201 )
				{
					m_sAPPLY.LOAD_0201( SFile );
				}
				else if ( dwVer==0x0202 )
				{
					m_sAPPLY.LOAD_0202 ( SFile );
				}
				else if ( dwVer==0x0203 )
				{
					m_sAPPLY.LOAD_0203 ( SFile );
				}
				else if ( dwVer==SKILL::SAPPLY::VERSION )
				{
					m_sAPPLY.LOAD( SFile );
				}
				else
				{
					CDebugSet::ErrorVersion( "SKILL::SAPPLY", dwVer );
					SFile.SetOffSet(SFile.GetfTell()+dwSize);
				}
			}break;

		case FILE_SLEARN:
			{
				SFile >> dwVer;
				SFile >> dwSize;

				if ( dwVer==0x0100 )
				{
					GASSERT(sizeof(SKILL::SLEARN_100)==dwSize);
					SKILL::SLEARN_100 sLEARN_OLD;
					SFile.ReadBuffer ( &sLEARN_OLD, sizeof(SKILL::SLEARN_100) );
					m_sLEARN.Assign ( sLEARN_OLD );
				}
				else if ( dwVer==0x0101 )
				{
					GASSERT(sizeof(SKILL::SLEARN_101)==dwSize);
					SKILL::SLEARN_101 sLEARN_OLD;
					SFile.ReadBuffer ( &sLEARN_OLD, sizeof(SKILL::SLEARN_101) );
					m_sLEARN.Assign ( sLEARN_OLD );
				}
				else if ( dwVer==0x0101 )
				{
					GASSERT(sizeof(SKILL::SLEARN_101)==dwSize);
					SKILL::SLEARN_101 sLEARN_OLD;
					SFile.ReadBuffer ( &sLEARN_OLD, sizeof(SKILL::SLEARN_101) );
					m_sLEARN.Assign ( sLEARN_OLD );
				}
				else if ( dwVer==0x0102 )
				{
					GASSERT(sizeof(SKILL::SLEARN_102)==dwSize);
					SKILL::SLEARN_102 sLEARN_OLD;
					SFile.ReadBuffer ( &sLEARN_OLD, sizeof(SKILL::SLEARN_102) );
					m_sLEARN.Assign ( sLEARN_OLD );
				}
				else if ( dwVer==SKILL::SLEARN::VERSION )
				{
					GASSERT(sizeof(SKILL::SLEARN)==dwSize);
					SFile.ReadBuffer ( &m_sLEARN, sizeof(SKILL::SLEARN) );
				}
				else
				{
					CDebugSet::ErrorVersion( "SKILL::SLEARN", dwVer );
					SFile.SetOffSet(SFile.GetfTell()+dwSize);
				}
			}break;

		case FILE_SEXT_DATA:
			{
				SFile >> dwVer;
				SFile >> dwSize;

				if ( dwVer==0x0100 )
				{
					SKILL::SEXT_DATA_100 sOLD_DATA;

					GASSERT(sizeof(SKILL::SEXT_DATA_100)==dwSize);
					SFile.ReadBuffer ( &sOLD_DATA, sizeof(SKILL::SEXT_DATA_100) );
					m_sEXT_DATA.Assign ( sOLD_DATA );
				}
				else if ( dwVer==0x0101 )
				{
					SKILL::SEXT_DATA_101 sOLD_DATA;

					GASSERT(sizeof(SKILL::SEXT_DATA_101)==dwSize);
					SFile.ReadBuffer ( &sOLD_DATA, sizeof(SKILL::SEXT_DATA_101) );
					m_sEXT_DATA.Assign ( sOLD_DATA );
				}
				else if ( dwVer==0x0102 )
				{
					SKILL::SEXT_DATA_102 sOLD_DATA;

					GASSERT(sizeof(SKILL::SEXT_DATA_102)==dwSize);
					SFile.ReadBuffer ( &sOLD_DATA, sizeof(SKILL::SEXT_DATA_102) );
					m_sEXT_DATA.Assign ( sOLD_DATA );
				}
				else if ( dwVer==0x0103 )
				{
					SKILL::SEXT_DATA_103 sOLD_DATA;
					GASSERT(sizeof(sOLD_DATA)==dwSize);
					sOLD_DATA.LOAD ( SFile );

					m_sEXT_DATA.Assign ( sOLD_DATA );
				}
				else if ( dwVer==0x0104 )
				{
					SKILL::SEXT_DATA_104 sOLD_DATA;
					GASSERT(sizeof(sOLD_DATA)==dwSize);
					sOLD_DATA.LOAD ( SFile );

					m_sEXT_DATA.Assign ( sOLD_DATA );
				}
				else if ( dwVer==0x0105 )
				{
					SKILL::SEXT_DATA_105 sOLD_DATA;
					GASSERT(sizeof(sOLD_DATA)==dwSize);
					sOLD_DATA.LOAD ( SFile );

					m_sEXT_DATA.Assign ( sOLD_DATA );
				}
				else if ( dwVer==0x0106 )
				{
					SKILL::SEXT_DATA_106 sOLD_DATA;
					GASSERT(sizeof(sOLD_DATA)==dwSize);
					sOLD_DATA.LOAD ( SFile );

					m_sEXT_DATA.Assign ( sOLD_DATA );
				}
				else if ( dwVer==0x0107 )
				{
					SKILL::SEXT_DATA_107 sOLD_DATA;
					sOLD_DATA.LOAD ( SFile );

					m_sEXT_DATA.Assign ( sOLD_DATA );
				}
				else if ( dwVer==0x0108 )
				{
					SKILL::SEXT_DATA_108 sOLD_DATA;
					sOLD_DATA.LOAD ( SFile );

					m_sEXT_DATA.Assign ( sOLD_DATA );
				}
				else if ( dwVer==0x0109 )
				{
					SKILL::SEXT_DATA_109 sOLD_DATA;
					sOLD_DATA.LOAD ( SFile );

					m_sEXT_DATA.Assign ( sOLD_DATA );
				}
				else if ( dwVer==SKILL::SEXT_DATA::VERSION)
				{
					m_sEXT_DATA.LOAD ( SFile );
				}
				else
				{
					CDebugSet::ErrorVersion( "SKILL::SEXT_DATA", dwVer );
					SFile.SetOffSet(SFile.GetfTell()+dwSize);
				}
			}break;

		case FILE_SPECIAL_SKILL:
			{
				SFile >> dwVer;
				SFile >> dwSize;

				if ( dwVer==SKILL::SSPECIAL_SKILL::VERSION )
				{
					m_sSPECIAL_SKILL.LOAD ( SFile );
				}
				else if ( dwVer==0x0101 )
				{
					m_sSPECIAL_SKILL.LOAD_0101 ( SFile );
				}
				else if ( dwVer==0x0100 )
				{
					m_sSPECIAL_SKILL.LOAD_0100 ( SFile );
				}
				else if ( dwVer==0x0103 )
				{
					m_sSPECIAL_SKILL.LOAD_0103 ( SFile );
				}
				else
				{
					CDebugSet::ErrorVersion( "SKILL::SPECIAL_SKILL", dwVer );
					SFile.SetOffSet(SFile.GetfTell()+dwSize);
				}
			}break;
		};

		SFile >> dwDataType;
	};

	//range check
	for( int i=0; i<SKILL::MAX_LEVEL; ++i )
	{
		if ( /*m_sBASIC.emIMPACT_TAR == TAR_SPEC ||*/ m_sBASIC.emIMPACT_TAR == TAR_SELF_TOSPEC )
		{
			if ( m_sAPPLY.sDATA_LVL[i].wAPPLYRANGE < m_sBASIC.wTARRANGE )
			{
				CDebugSet::ToFileWithTime( "skillrangecheck.txt", "[%03d~%03d] %s correcting mismatch range tar:%u, apply:%u", 
					m_sBASIC.sNATIVEID.wMainID, m_sBASIC.sNATIVEID.wSubID, m_sBASIC.szNAME, m_sBASIC.wTARRANGE, m_sAPPLY.sDATA_LVL[i].wAPPLYRANGE );

				m_sAPPLY.sDATA_LVL[i].wAPPLYRANGE = m_sBASIC.wTARRANGE;

				//break;
			}
		}
	}

	/*if ( m_sBASIC.sNATIVEID.wMainID >= 0 && m_sBASIC.sNATIVEID.wMainID <= 7 )
	{
		if ( m_sBASIC.wTARRANGE == 30 )
		m_sBASIC.wTARRANGE = 35;
	}

	if ( m_sBASIC.sNATIVEID.wMainID >= 12 && m_sBASIC.sNATIVEID.wMainID <= 15 )
	{
		if ( m_sBASIC.wTARRANGE == 30 )
			m_sBASIC.wTARRANGE = 35;
	}

	if ( m_sBASIC.sNATIVEID.wMainID >= 43 && m_sBASIC.sNATIVEID.wMainID <= 46 )
	{
		if ( m_sBASIC.wTARRANGE == 30 )
			m_sBASIC.wTARRANGE = 35;
	}

	if ( m_sBASIC.sNATIVEID.wMainID >= 8 && m_sBASIC.sNATIVEID.wMainID <= 11 )
	{
		if ( m_sBASIC.wTARRANGE > 45 )
			m_sBASIC.wTARRANGE = 45;
	}

	if ( m_sBASIC.sNATIVEID.wMainID >= 30 && m_sBASIC.sNATIVEID.wMainID <= 33 )
	{
		if ( m_sBASIC.wTARRANGE > 45 )
			m_sBASIC.wTARRANGE = 45;


	}

	if ( m_sBASIC.sNATIVEID.wMainID >= 36 && m_sBASIC.sNATIVEID.wMainID <= 39 )
	{
		if ( m_sBASIC.wTARRANGE > 45 )
			m_sBASIC.wTARRANGE = 45;
	}*/

	////if ( m_sBASIC.sNATIVEID.wMainID >= 30 && m_sBASIC.sNATIVEID.wMainID <= 33 )
	//if ( m_sBASIC.emUSE_LITEM > SKILLATT_NSIZE ) 
	//{
	//	CDebugSet::ToFileWithTime( "skillweapreqtxt", "[%03d~%03d] %s correcting mismatch weapon req set SKILLATT_NOCARE from %s", 
	//		m_sBASIC.sNATIVEID.wMainID, m_sBASIC.sNATIVEID.wSubID, m_sBASIC.szNAME, COMMENT::SKILLATTACK[m_sBASIC.emUSE_LITEM].c_str());
	//	m_sBASIC.emUSE_LITEM = SKILLATT_NOCARE;
	//}
	////if ( m_sBASIC.sNATIVEID.wMainID >= 30 && m_sBASIC.sNATIVEID.wMainID <= 33 )
	//if ( m_sBASIC.emUSE_RITEM > SKILLATT_NSIZE ) 
	//{
	//	CDebugSet::ToFileWithTime( "skillweapreqtxt", "[%03d~%03d] %s correcting mismatch weapon req set SKILLATT_NOCARE from %s", 
	//		m_sBASIC.sNATIVEID.wMainID, m_sBASIC.sNATIVEID.wSubID, m_sBASIC.szNAME, COMMENT::SKILLATTACK[m_sBASIC.emUSE_RITEM].c_str());
	//	m_sBASIC.emUSE_RITEM = SKILLATT_NOCARE;
	//}

	//if ( m_sBASIC.emUSE_LITEM == SKILLATT_EXTREME_GLOVE ) 
	//{
	//	CDebugSet::ToFileWithTime( "skillweapreqtxt", "[%03d~%03d] %s correcting mismatch weapon req set SKILLATT_NOCARE from %s", 
	//		m_sBASIC.sNATIVEID.wMainID, m_sBASIC.sNATIVEID.wSubID, m_sBASIC.szNAME, COMMENT::SKILLATTACK[m_sBASIC.emUSE_LITEM].c_str());
	//	m_sBASIC.emUSE_LITEM = SKILLATT_NOCARE;
	//}
	//if ( m_sBASIC.emUSE_RITEM == SKILLATT_EXTREME_GLOVE ) 
	//{
	//	CDebugSet::ToFileWithTime( "skillweapreqtxt", "[%03d~%03d] %s correcting mismatch weapon req set SKILLATT_NOCARE from %s", 
	//		m_sBASIC.sNATIVEID.wMainID, m_sBASIC.sNATIVEID.wSubID, m_sBASIC.szNAME, COMMENT::SKILLATTACK[m_sBASIC.emUSE_RITEM].c_str());
	//	m_sBASIC.emUSE_RITEM = SKILLATT_NOCARE;
	//}

	/*if ( m_sBASIC.sNATIVEID.wMainID >= 36 && m_sBASIC.sNATIVEID.wMainID <= 39 )
	{
		m_sBASIC.emUSE_RITEM = SKILLATT_SHOTGUN;
	}*/

	return TRUE;
}

VOID GLSKILL::SaveCsvHead ( std::fstream &SFile )
{
	SKILL::SSKILLBASIC::SaveCsvHead( SFile );
	SKILL::SLEARN::SaveCsvHead( SFile );
	SKILL::SEXT_DATA::SaveCsvHead( SFile );
	SKILL::SSPECIAL_SKILL::SaveCsvHead( SFile );
	SFile << std::endl;

	SKILL::SAPPLY::SaveCsvHead( SFile );
	SFile << std::endl;
}

VOID GLSKILL::SaveCsv ( std::fstream &SFile )
{
	m_sBASIC.SaveCsv( SFile );
	m_sLEARN.SaveCsv( SFile );
	m_sEXT_DATA.SaveCsv( SFile );
	m_sSPECIAL_SKILL.SaveCsv( SFile );
	SFile << std::endl;

	m_sAPPLY.SaveCsv( SFile );
	SFile << std::endl;
}

VOID GLSKILL::LoadCsv ( CStringArray &StrArray, int iLine )
{
	if( iLine )
	{
		int iCsvCur = 0;
		m_sBASIC.LoadCsv( StrArray, iCsvCur );
		m_sLEARN.LoadCsv( StrArray, iCsvCur );
		m_sEXT_DATA.LoadCsv( StrArray, iCsvCur );
		m_sSPECIAL_SKILL.LoadCsv( StrArray, iCsvCur );
	}
	else
	{
		int iCsvCur = 0;
		m_sAPPLY.LoadCsv( StrArray, iCsvCur );
	}
}

VOID GLSKILL::LoadCsv_oldj ( CStringArray &StrArray, int iLine )
{
	if( iLine )
	{
		int iCsvCur = 0;
		m_sBASIC.LoadCsv_oldj( StrArray, iCsvCur );
		m_sLEARN.LoadCsv_oldj( StrArray, iCsvCur );
		m_sEXT_DATA.LoadCsv_oldj( StrArray, iCsvCur );
		m_sSPECIAL_SKILL.LoadCsv_oldj( StrArray, iCsvCur );
	}
	else
	{
		int iCsvCur = 0;
		m_sAPPLY.LoadCsv_oldj( StrArray, iCsvCur );
	}
}

//	지속성 스킬
bool GLSKILL::IsSkillFact ()
{
	bool bFact(false);
	
	switch ( m_sAPPLY.emBASIC_TYPE )
	{
	case SKILL::EMFOR_VARHP:
	case SKILL::EMFOR_VARMP:
	case SKILL::EMFOR_VARSP:
	case SKILL::EMFOR_DEFENSE:
	case SKILL::EMFOR_HITRATE:
	case SKILL::EMFOR_AVOIDRATE:
	case SKILL::EMFOR_VARAP:
	case SKILL::EMFOR_VARDAMAGE:
	case SKILL::EMFOR_VARDEFENSE:
	case SKILL::EMFOR_PA:
	case SKILL::EMFOR_SA:
	case SKILL::EMFOR_MA:
	case SKILL::EMFOR_HP_RATE:
	case SKILL::EMFOR_MP_RATE:
	case SKILL::EMFOR_SP_RATE:
	case SKILL::EMFOR_RESIST:
	case SKILL::EMFOR_SUMMONTIME: /*summon time, Juver, 2017/12/12 */
		bFact = true;
		break;
	};

	if ( !bFact )
	{
		for( int nImpact=0; nImpact<SKILL::MAX_IMPACT; ++nImpact )
		{
			if ( m_sAPPLY.sImpacts[nImpact].emADDON!=EMIMPACTA_NONE )
				bFact = true;
		}
	}
	
	if ( !bFact )
	{
		for( int i=0; i<SKILL::MAX_SPEC; ++i )
		{
			switch ( m_sAPPLY.sSpecs[i].emSPEC )
			{
			case EMSPECA_REFDAMAGE:
			case EMSPECA_NONBLOW:
			case EMSPECA_PIERCE:
			case EMSPECA_TARRANGE:
			case EMSPECA_MOVEVELO:
			case EMSPECA_INVISIBLE:
			case EMSPECA_RECVISIBLE:
			case EMSPECA_ATTACKVELO:
			case EMSPECA_SKILLDELAY:
			case EMSPECA_PSY_DAMAGE_REDUCE:
			case EMSPECA_MAGIC_DAMAGE_REDUCE:
			case EMSPECA_PSY_DAMAGE_REFLECTION:
			case EMSPECA_MAGIC_DAMAGE_REFLECTION:
			case EMSPECA_DEFENSE_SKILL_ACTIVE:
			case EMSPECA_PROHIBIT_POTION:		/*prohibit potion skill logic, Juver, 2017/06/06 */
			case EMSPECA_PROHIBIT_SKILL:		/*prohibit skill logic, Juver, 2017/06/06 */
			case EMSPECA_CHANGE_ATTACK_RANGE:	/*skill range spec logic, Juver, 2017/06/06 */
			case EMSPECA_CHANGE_APPLY_RANGE:	/*skill range spec logic, Juver, 2017/06/06 */
			case EMSPECA_STUN:					/*skill stun logic, Juver, 2017/06/06 */
			case EMSPECA_CONTINUOUS_DAMAGE:		/*continuous damage skill logic, Juver, 2017/06/10 */
			case EMSPECA_CURSE:					/*curse skill logic, Juver, 2017/06/10 */
			case EMSPECA_TALK_TO_NPC:			/*npc talk buff, Juver, 2017/10/03 */
			case EMSPECA_IGNORE_DAMAGE:			/*ignore damage, Juver, 2017/12/12 */
			case EMSPECA_ENHANCEMENT:
				bFact = true;
				break;
			};
		}
	}
	
	return bFact;
}

const char* GLSKILL::GetName()
{
	const char* szName = GLStringTable::GetInstance().GetString( m_sBASIC.szNAME, GLStringTable::SKILL );
	if ( !szName )		return m_sBASIC.szNAME;
	
	return szName;
}
const std::string GLSKILL::GetNameStr() const
{
   // static std::string TempStr = GLStringTable::GetInstance().GetString( m_sBASIC.szNAME, GLStringTable::SKILL );
	//if ( !TempStr )	return m_sBASIC.szNAME;
	//TempStr = m_sBASIC.szNAME;
   // return TempStr;


	//static std::string TempStr(m_sBASIC.szNAME);
	//return TempStr;

	//	std::string RetStr;
// 	if (GLUseFeatures::GetInstance().IsUsingPrivateDataIntegrate())
// 		RetStr = CGameTextMan::GetInstance().GetText( m_sBASIC.szNAME, 0, CGameTextMan::EM_SKILL_TEXT );
// 	else
	std::string RetStr;
 	RetStr = GLStringTable::GetInstance().GetString(m_sBASIC.szNAME, GLStringTable::SKILL);
	if (!RetStr.empty())
    {
        return RetStr;
    }
    else
    {
        static std::string TempStr(m_sBASIC.szNAME);
        return TempStr;
    }
}

const char* GLSKILL::GetDesc()
{
	const char* szDesc = GLStringTable::GetInstance().GetString( m_sEXT_DATA.strCOMMENTS.c_str(), GLStringTable::SKILL );
	if ( !szDesc )		return NULL;
	
	return szDesc;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
GLSkillMan::GLSkillMan () 
	: m_pd3dDevice(NULL)
	, m_bModify(false)
{
	memset(m_szFileName, 0, sizeof(char) * (MAX_PATH));
}

GLSkillMan::~GLSkillMan ()
{
}

HRESULT GLSkillMan::OneTimeSceneInit ()
{		
	for ( int i=0; i<EMSKILLCLASS_NSIZE; i++ )
	for ( int j=0; j<MAX_CLASSSKILL; j++ )
	{
		m_pSkills[i][j] = NULL;
	}		

	return S_OK;
}

HRESULT GLSkillMan::InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	m_pd3dDevice = pd3dDevice;

	return S_OK;
}

HRESULT GLSkillMan::DeleteDeviceObjects ()
{
	return S_OK;
}

HRESULT GLSkillMan::FinalCleanup ()
{
	for ( int i=0; i<EMSKILLCLASS_NSIZE; i++ )
	for ( int j=0; j<MAX_CLASSSKILL; j++ )
	{
		SAFE_DELETE(m_pSkills[i][j]);
	}		

	return S_OK;
}

PGLSKILL GLSkillMan::GetData ( WORD wClass, WORD Index )
{
	if ( wClass>=EMSKILLCLASS_NSIZE )	return NULL;
	if ( Index>=MAX_CLASSSKILL )		return NULL;

	return m_pSkills[wClass][Index];
}

void GLSkillMan::SetData ( WORD wMID, WORD Index, const PGLSKILL pSkillData, bool binner )
{
	GASSERT(wMID<EMSKILLCLASS_NSIZE);
	GASSERT(Index<MAX_CLASSSKILL);

	if ( wMID>=EMSKILLCLASS_NSIZE )		return;
	if ( Index>=MAX_CLASSSKILL )		return;

	SAFE_DELETE(m_pSkills[wMID][Index]);

	m_pSkills[wMID][Index] = pSkillData;

	switch ( static_cast<EMSKILLCLASS>(wMID) )
	{
	case EMSKILL_BRAWLER_01:
	case EMSKILL_BRAWLER_02:
	case EMSKILL_BRAWLER_03:
	case EMSKILL_BRAWLER_04:
		pSkillData->m_sLEARN.dwCLASS = GLCC_BRAWLER_M | GLCC_BRAWLER_W;
		break;

	case EMSKILL_SWORDSMAN_01:
	case EMSKILL_SWORDSMAN_02:
	case EMSKILL_SWORDSMAN_03:
	case EMSKILL_SWORDSMAN_04:
		pSkillData->m_sLEARN.dwCLASS = GLCC_SWORDSMAN_M | GLCC_SWORDSMAN_W;
		break;

	case EMSKILL_ARCHER_01:
	case EMSKILL_ARCHER_02:
	case EMSKILL_ARCHER_03:
	case EMSKILL_ARCHER_04:
		pSkillData->m_sLEARN.dwCLASS = GLCC_ARCHER_M | GLCC_ARCHER_W;
		break;

	case EMSKILL_SHAMAN_01:
	case EMSKILL_SHAMAN_02:
	case EMSKILL_SHAMAN_03:
	case EMSKILL_SHAMAN_04:
		pSkillData->m_sLEARN.dwCLASS = GLCC_SHAMAN_M | GLCC_SHAMAN_W;
		break;

	case EMSKILL_EXTREME_01:
	case EMSKILL_EXTREME_02:
	case EMSKILL_EXTREME_03:
	case EMSKILL_EXTREME_04:
		pSkillData->m_sLEARN.dwCLASS = GLCC_EXTREME_M|GLCC_EXTREME_W;
		break;

	case EMSKILL_GUNNER_01:
	case EMSKILL_GUNNER_02:
	case EMSKILL_GUNNER_03:
	case EMSKILL_GUNNER_04:
		pSkillData->m_sLEARN.dwCLASS = GLCC_GUNNER_M|GLCC_GUNNER_W;
		break;

	case EMSKILL_ASSASSIN_01:
	case EMSKILL_ASSASSIN_02:
	case EMSKILL_ASSASSIN_03:
	case EMSKILL_ASSASSIN_04:
		pSkillData->m_sLEARN.dwCLASS = GLCC_ASSASSIN_M|GLCC_ASSASSIN_W;
		break;

	case EMSKILL_TRICKER_01:
	case EMSKILL_TRICKER_02:
	case EMSKILL_TRICKER_03:
	case EMSKILL_TRICKER_04:
		pSkillData->m_sLEARN.dwCLASS = GLCC_TRICKER_M|GLCC_TRICKER_W;
		break;

	default:
		pSkillData->m_sLEARN.dwCLASS = GLCC_NONE;
		break;
	};

	if ( !binner )	m_bModify = true;
}

HRESULT GLSkillMan::LoadFile ( const char* szFile, bool bPastLoad )
{
	StringCchCopy(m_szFileName,MAX_PATH,szFile);

	char szFullPath[MAX_PATH] = {0};
	StringCchCopy ( szFullPath, MAX_PATH, GLOGIC::GetPath() );
	StringCchCat ( szFullPath, MAX_PATH, m_szFileName );

	std::auto_ptr<basestream> pBStream( GLOGIC::openfile_basestream(GLOGIC::bGLOGIC_ZIPFILE, 
																	GLOGIC::strGLOGIC_ZIPFILE.c_str(),
                                                                    szFullPath, 
																	szFile,
																	EMBYTECRYPT_NONE,
																	GLOGIC::bGLOGIC_PACKFILE ) );

	if ( !pBStream.get() )
	{
		if ( strlen( szFile ) > 0 )
			CDebugSet::ErrorFile( "GLSkillMan::LoadFile", szFile );

		return E_FAIL;
	}

	basestream &SFile = *pBStream;

	DWORD dwFILEVER;
	char szFILETYPE[_MAX_FNAME];
	SFile.GetFileType( szFILETYPE, _MAX_FNAME, dwFILEVER );

	if ( dwFILEVER>=ENCODE_VER_OLD )
		SFile.SetEncodeType ( EMBYTECRYPT_OLD );

	if ( dwFILEVER>=ENCODE_VER )
		SFile.SetEncodeType ( EMBYTECRYPT_SKILL );

	DWORD dwSkillNum;
	SFile >> dwSkillNum;

	for ( DWORD i=0; i<dwSkillNum; i++ )
	{
		GLSKILL *pSkill = new GLSKILL;

		if( pSkill->LoadFile ( SFile, bPastLoad ) == FALSE )
			return E_FAIL;
		SetData ( pSkill->m_sBASIC.sNATIVEID.wMainID, pSkill->m_sBASIC.sNATIVEID.wSubID, pSkill, true );
	}

	return S_OK;
}

HRESULT GLSkillMan::SaveFile ( const char* szFile )
{
	StringCchCopy(m_szFileName,MAX_PATH,szFile);

	char szFullPath[MAX_PATH] = {0};
	StringCchCopy ( szFullPath, MAX_PATH, GLOGIC::GetPath() );
	StringCchCat ( szFullPath, MAX_PATH, m_szFileName );

	CSerialFile SFile;

	SFile.SetFileType ( _FILEHEAD, VERSION );
	if ( !SFile.OpenFile ( FOT_WRITE, szFullPath ) )	return E_FAIL;
	SFile.SetEncodeType ( EMBYTECRYPT_SKILL );

	DWORD dwSkillNum = 0;
	for ( int i=0; i<EMSKILLCLASS_NSIZE; i++ )
	for ( int j=0; j<MAX_CLASSSKILL; j++ )
	{
		if ( m_pSkills[i][j] )	dwSkillNum++;
	}

	SFile << dwSkillNum;

	for ( int i=0; i<EMSKILLCLASS_NSIZE; i++ )
	for ( int j=0; j<MAX_CLASSSKILL; j++ )
	{
		if ( m_pSkills[i][j] )	m_pSkills[i][j]->SaveFile ( SFile );
	}

	m_bModify = false;
	return S_OK;
}

WORD GLSkillMan::FindFreeSkillIndex ( WORD wClass )
{
	GASSERT(wClass<EMSKILLCLASS_NSIZE);

	for ( WORD i=0; i<MAX_CLASSSKILL; i++ )
	{
		if ( !m_pSkills[wClass][i] )			return i;
	}

	return 0xFFFF;
}

GLSKILL* GLSKILL::GetSecondSkill()
{
	if( m_sEXT_DATA.sADDSKILL == NATIVEID_NULL() )
		return NULL;

	return GLSkillMan::GetInstance().GetData ( m_sEXT_DATA.sADDSKILL.wMainID, 
											   m_sEXT_DATA.sADDSKILL.wSubID );
} //GLSKILL::GetSecondSkill

BOOL GLSkillMan::DeleteSkill ( WORD wClass, WORD Index )
{
	GASSERT(wClass<EMSKILLCLASS_NSIZE);
	GASSERT(Index<MAX_CLASSSKILL);

	SAFE_DELETE(m_pSkills[wClass][Index]);

	m_bModify = true;
	return TRUE;
}

void GLSkillMan::GetMaxSkill ( WORD& wClass, WORD& rIndex )
{
	wClass = EMSKILLCLASS_NSIZE;
	rIndex = MAX_CLASSSKILL;
}

const char* GLSkillMan::_LOGFILE = "_CrowData_Synce.txt";
const char* GLSkillMan::_STRINGTABLE = "SkillStrTable.txt";

static CString	GetAppPath ()
{
	CString strFullPath;
	CString strCommandLine;

	TCHAR szPath[MAX_PATH] = {0};
	GetModuleFileName(::AfxGetInstanceHandle(), szPath, MAX_PATH);
	strCommandLine = szPath;

	if ( !strCommandLine.IsEmpty() )
	{
		DWORD dwFind = strCommandLine.ReverseFind ( '\\' );
		if ( dwFind != -1 )
		{
			strFullPath = strCommandLine.Left ( dwFind );
			
			if ( !strFullPath.IsEmpty() )
			if ( strFullPath.GetAt(0) == '"' )
				strFullPath = strFullPath.Right ( strFullPath.GetLength() - 1 );
		}
	}

	return strFullPath;
}

bool GLSkillMan::ValidData ()
{
	for ( int i = 0; i < EMSKILLCLASS_NSIZE; i++ )
	for ( int j = 0; j < MAX_CLASSSKILL; j++ )
	{
		if ( m_pSkills[i][j] )		return true;
	}

	return false;
}

HRESULT GLSkillMan::SyncUpdateData ()
{
	//	Note : 데이터 동기화 점검을 위한 초기화.
	//
	CString strAppPath;
	char szAppPath[MAX_PATH] = {0};
	char szFullPath[MAX_PATH] = {0};
	strAppPath = GetAppPath ();
	StringCchCopy ( szAppPath, MAX_PATH, strAppPath.GetString () );

	//	Note : DxBoneCollector 기본 폴더 지정.
	//
	StringCchCopy ( szFullPath, MAX_PATH, szAppPath );
	StringCchCat ( szFullPath, MAX_PATH, SUBPATH::OBJ_FILE_SKELETON );
	DxBoneCollector::GetInstance().OneTimeSceneInit ( szFullPath );

	//	Note : 디버그샛의 초기화.
	//
	CDebugSet::OneTimeSceneInit ( szAppPath );

	//	Note : Animation 기본 폴더 지정.
	//
	StringCchCopy ( szFullPath, MAX_PATH, szAppPath );
	StringCchCat ( szFullPath, MAX_PATH, SUBPATH::OBJ_FILE_ANIMATION );
	DxSkinAniMan::GetInstance().OneTimeSceneInit ( szFullPath );

	//	Note : SkinObject 기본 폴더 지정.
	//
	StringCchCopy ( szFullPath, MAX_PATH, szAppPath );
	StringCchCat ( szFullPath, MAX_PATH, SUBPATH::OBJ_FILE_SKINOBJECT );
	DxSkinObject::SetPath ( szFullPath );
	DxSkinCharDataContainer::GetInstance().SetPath ( szFullPath );
	DxSkinPieceContainer::GetInstance().SetPath ( szFullPath );

	/*ABL system, Juver, 2017/05/29 */
	DxAttBoneDataContainer::GetInstance().SetPath ( szFullPath );
	DxAttBoneLinkContainer::GetInstance().SetPath ( szFullPath );

	/*vehicle system, Juver, 2017/08/06 */
	DxVehicleDataContainer::GetInstance().SetPath( szFullPath );

	//	Note : GLogic
	//
	StringCchCopy ( szFullPath, MAX_PATH, szAppPath );
	StringCchCat ( szFullPath, MAX_PATH, SUBPATH::GLOGIC_FILE );
	GLOGIC::SetPath ( szFullPath );

	GLCONST_CHAR::LOADFILE ( "default.charclass" );

	for ( int nClass=0; nClass<GLCI_NUM_8CLASS; ++nClass )
	{
		GLCONST_CHAR::cCONSTCLASS[nClass].LoadAniSet ( GLCONST_CHAR::szCharSkin[nClass] );
	}
	
	int nCHAR_FLAGS[GLCI_NUM_8CLASS] =
	{
		GLCC_BRAWLER_M,
		GLCC_SWORDSMAN_M,
		GLCC_ARCHER_W,
		GLCC_SHAMAN_W,
		GLCC_EXTREME_M,
		GLCC_EXTREME_W,
		GLCC_BRAWLER_W,
		GLCC_SWORDSMAN_W,
		GLCC_ARCHER_M,
		GLCC_SHAMAN_M,
		GLCC_GUNNER_M,
		GLCC_GUNNER_W,
		GLCC_ASSASSIN_M,
		GLCC_ASSASSIN_W,
		GLCC_TRICKER_M,
		GLCC_TRICKER_W,
	};

	CDebugSet::ClearFile ( _LOGFILE );

	for ( int i = 0; i < EMSKILLCLASS_NSIZE; i++ )
	for ( int j = 0; j < MAX_CLASSSKILL; j++ )
	{
		if ( !m_pSkills[i][j] )		continue;

		GLSKILL &sSKILL = *m_pSkills[i][j];

		for ( int k=0; k<SKILL::MAX_LEVEL; ++k )
		{
			if ( sSKILL.m_sAPPLY.sDATA_LVL[k].wAPPLYNUM==0 )
			{
				CDebugSet::ToFile ( _LOGFILE, "[%03d][%03d] 레벨(%d) 적용횟수가 0 입니다.", i, j, k );
			}

			if ( sSKILL.m_sAPPLY.sDATA_LVL[k].wTARNUM==0 )
			{
				CDebugSet::ToFile ( _LOGFILE, "[%03d][%03d] 레벨(%d) 타겟갯수가 0 입니다.", i, j, k );
			}
		}

		for ( int nClass=0; nClass<GLCI_NUM_8CLASS; ++nClass )
		{
			if ( sSKILL.m_sLEARN.dwCLASS & nCHAR_FLAGS[nClass] )
			{
				GLCONST_CHARCLASS &cCHARCLASS = GLCONST_CHAR::cCONSTCLASS[nClass];

				VECANIATTACK &sANILIST = cCHARCLASS.m_ANIMATION[sSKILL.m_sEXT_DATA.emANIMTYPE][sSKILL.m_sEXT_DATA.emANISTYPE];

				if ( sANILIST.empty() )
				{
					CDebugSet::ToFile ( _LOGFILE, "[%03d][%03d][%s] 지정된 '스킬모션'이 존제하지 않음 .", i, j, sSKILL.GetName() );
					continue;
				}

				SANIATTACK &sANI = sANILIST[0];

				if ( sANI.m_wDivCount==0 )
				{
					CDebugSet::ToFile ( _LOGFILE, "[%03d][%03d][%s] '스킬모션'에 '타격지점'이 없음.", i, j, sSKILL.GetName() );
				}
			}
		}
	}

	CDebugSet::ToFile ( _LOGFILE, "---------------------sync-completion--------------------------------" );	

	return S_OK;
}

HRESULT GLSkillMan::SyncStringTable()
{
	char	szFullPathFileName[MAX_PATH] = "";
	StringCchCopy ( szFullPathFileName, MAX_PATH, GLOGIC::GetPath() );
	StringCchCat ( szFullPathFileName, MAX_PATH, _STRINGTABLE );

	//DeleteFile ( szFullPathFileName );

	CString strBuffer1, strBuffer;
	std::string strTemp;
	std::string::size_type idx;

	strBuffer = "// File : Skill String Table\r\n";
	strBuffer += "// Note : 키(ID)와 내용은 반드시 탭으로 구분되어야 합니다.\r\n//\r\n";
		
	for ( int i = 0; i < EMSKILLCLASS_NSIZE; i++ )
	for ( int j = 0; j < MAX_CLASSSKILL; j++ )
	{
		if ( !m_pSkills[i][j] )		continue;
		const GLSKILL &sSKILL = *m_pSkills[i][j];

		if( sSKILL.m_sBASIC.szNAME[0] != _T('\0') )
		{
			strBuffer1.Format( _T("SN_%03d_%03d\t%s\r\n"), i, j, sSKILL.m_sBASIC.szNAME );
			strBuffer += strBuffer1;
		}
		
		if( !sSKILL.m_sEXT_DATA.strCOMMENTS.empty() )
		{
			strTemp = sSKILL.m_sEXT_DATA.strCOMMENTS;
			idx = strTemp.find( "\r\n" );
			while ( idx != std::string::npos )
			{
				strTemp.replace( idx, 2, " " );
				idx = strTemp.find( "\r\n" );
			}
			
			strBuffer1.Format( _T("SD_%03d_%03d\t%s\r\n"), i, j, strTemp.c_str() );
			strBuffer += strBuffer1;
		}
	}

	CFile file;
	file.Open( _T(szFullPathFileName), CFile::modeCreate|CFile::modeWrite ); // 파일 열기
	file.Write( strBuffer.GetString(), strBuffer.GetLength()-2 ); // 파일 쓰기
	file.Close();

	//delete [] szEncode;
	return S_OK;
}

HRESULT GLSkillMan::SaveCsvFile ( CWnd* pWnd )
{
	CFileDialog dlg( FALSE, ".csv", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		"csv file (*.csv)|*.csv|", pWnd );

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() != IDOK )
		return E_FAIL;

	std::fstream streamFILE;
	streamFILE.open ( dlg.GetPathName().GetString(), std::ios_base::out );

	for( int i=0; i<EMSKILLCLASS_NSIZE; ++i )
	{
		for( int j=0; j<MAX_CLASSSKILL; ++j )
		{
			if( !m_pSkills[i][j] )
				continue;

			// Csv Head
			GLSKILL::SaveCsvHead( streamFILE );

			// Csv Data
			GLSKILL *pSKILL = m_pSkills[i][j];
			pSKILL->SaveCsv( streamFILE );
		}
	}

	streamFILE.close();

	return S_OK;
}


HRESULT GLSkillMan::LoadCsvFile_oldj ( CWnd* pWnd )
{
	CFileDialog dlg( TRUE, ".csv", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		"csv file (*.csv)|*.csv|", pWnd );

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() != IDOK )
		return E_FAIL;

	CStringFile StrFile( 32768 );
	if( !StrFile.Open ( dlg.GetPathName().GetString() ) )
		return E_FAIL;

	STRUTIL::ClearSeparator();
	STRUTIL::RegisterSeparator( "," );

	CString strLine;
	CStringArray StrArray;

	// Csv Head
	int iHead[2] = { 0, 0 };
	for( int i=1; i<3; ++i )
	{
		StrFile.GetNextLine( strLine );
		STRUTIL::StringSeparate( strLine, StrArray );
		iHead[ i % 2 ] = (int)StrArray.GetCount();
	}

	int iLine = 2;
	bool bHead = true;
	char szError[ 256 ];

	int iMID = EMSKILLCLASS_NSIZE;
	int iSID = MAX_CLASSSKILL;

	while( StrFile.GetNextLine( strLine ) )
	{
		if( ++iLine % 2 )
			bHead = !bHead;
		if( bHead )
			continue;

		STRUTIL::StringSeparate( strLine, StrArray );

		int iCount = (int)StrArray.GetCount();
		if( iCount < iHead[ iLine % 2 ] )
		{
			StringCchPrintf( szError, 256, "%d Line = include blank column, next item load failed", iLine );
			MessageBox( pWnd->GetSafeHwnd(), _T(szError), _T("Fail"), MB_OK );
			return E_FAIL;
		}

		if( iLine % 2 )
		{
			iMID = atoi( StrArray[0] );
			iSID = atoi( StrArray[1] );
		}

		if( iMID >= EMSKILLCLASS_NSIZE || iSID >= MAX_CLASSSKILL )
			continue;

		if( !m_pSkills[ iMID ][ iSID ] )
			m_pSkills[ iMID ][ iSID ] = new GLSKILL;

		// Csv Data
		GLSKILL *pSKILL = m_pSkills[ iMID ][ iSID ];
		pSKILL->LoadCsv_oldj( StrArray, iLine % 2 );
	}

	return S_OK;
}

HRESULT GLSkillMan::LoadCsvFile ( CWnd* pWnd )
{
	CFileDialog dlg( TRUE, ".csv", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		"csv file (*.csv)|*.csv|", pWnd );

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() != IDOK )
		return E_FAIL;

	CStringFile StrFile( 32768 );
	if( !StrFile.Open ( dlg.GetPathName().GetString() ) )
		return E_FAIL;

	STRUTIL::ClearSeparator();
	STRUTIL::RegisterSeparator( "," );

	CString strLine;
	CStringArray StrArray;

	// Csv Head
	int iHead[2] = { 0, 0 };
	for( int i=1; i<3; ++i )
	{
		StrFile.GetNextLine( strLine );
		STRUTIL::StringSeparate( strLine, StrArray );
		iHead[ i % 2 ] = (int)StrArray.GetCount();
	}

	int iLine = 2;
	bool bHead = true;
	char szError[ 256 ];

	int iMID = EMSKILLCLASS_NSIZE;
	int iSID = MAX_CLASSSKILL;

	while( StrFile.GetNextLine( strLine ) )
	{
		if( ++iLine % 2 )
			bHead = !bHead;
		if( bHead )
			continue;

		STRUTIL::StringSeparate( strLine, StrArray );

		int iCount = (int)StrArray.GetCount();
		if( iCount < iHead[ iLine % 2 ] )
		{
			StringCchPrintf( szError, 256, "%d Line = include blank column, next item load failed", iLine );
			MessageBox( pWnd->GetSafeHwnd(), _T(szError), _T("Fail"), MB_OK );
			return E_FAIL;
		}

		if( iLine % 2 )
		{
			iMID = atoi( StrArray[0] );
			iSID = atoi( StrArray[1] );
		}

		if( iMID >= EMSKILLCLASS_NSIZE || iSID >= MAX_CLASSSKILL )
			continue;

		if( !m_pSkills[ iMID ][ iSID ] )
			m_pSkills[ iMID ][ iSID ] = new GLSKILL;

		// Csv Data
		GLSKILL *pSKILL = m_pSkills[ iMID ][ iSID ];
		pSKILL->LoadCsv( StrArray, iLine % 2 );
	}

	return S_OK;
}