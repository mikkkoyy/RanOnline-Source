#include "stdafx.h"
#include "GLItemGrinding.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ITEM
{
	VOID SGRINDING::SaveCsvHead ( std::fstream &SFile )
	{
		SFile << "emCLASS" << ",";
		SFile << "emGRINDER_TYPE" << ",";
		SFile << "emTYPE" << ",";
		SFile << "strGrind" << ",";
		SFile << "bNoFail" << ","; /*no fail upgrade, Juver, 2017/11/26 */
	}

	VOID SGRINDING::SaveCsv ( std::fstream &SFile )
	{
		SFile << emCLASS << ",";
		SFile << emLEVEL << ",";
		SFile << emTYPE << ",";
		STRUTIL::OutputStrCsv( SFile, strGrind );
		SFile << bNoFail << ","; /*no fail upgrade, Juver, 2017/11/26 */
	}

	VOID SGRINDING::LoadCsv ( CStringArray &StrArray, int &iCsvCur )
	{
		emCLASS = (EMGRINDING_CLASS)atoi( StrArray[ iCsvCur++ ] );
		emLEVEL = (EMGRINDING_LEVEL)atoi( StrArray[ iCsvCur++ ] );
		emTYPE = (EMGRINDING_TYPE)atoi( StrArray[ iCsvCur++ ] );
		STRUTIL::InputStrCsv( StrArray[ iCsvCur++ ], strGrind );
		bNoFail = (BOOL)atol( StrArray[ iCsvCur++ ] ); /*no fail upgrade, Juver, 2017/11/26 */
	}

	bool SGRINDING::LOAD_0105 ( basestream &SFile )
	{
		DWORD dwDATA(0);
		SFile >> dwDATA; emLEVEL = (EMGRINDING_LEVEL)dwDATA;
		SFile >> dwDATA; emTYPE = (EMGRINDING_TYPE)dwDATA;
		SFile >> dwDATA; emCLASS = (EMGRINDING_CLASS)dwDATA;
		SFile >> strGrind;

		return true;
	}

	bool SGRINDING::LOAD ( basestream &SFile )
	{
		DWORD dwDATA(0);
		SFile >> dwDATA; emLEVEL = (EMGRINDING_LEVEL)dwDATA;
		SFile >> dwDATA; emTYPE = (EMGRINDING_TYPE)dwDATA;
		SFile >> dwDATA; emCLASS = (EMGRINDING_CLASS)dwDATA;
		SFile >> strGrind; 
		SFile >> bNoFail; /*no fail upgrade, Juver, 2017/11/26 */

		return true;
	}

	bool SGRINDING::SAVE ( CSerialFile &SFile )
	{
		
		SFile << (DWORD)emLEVEL;
		SFile << (DWORD)emTYPE;
		SFile << (DWORD)emCLASS;
		SFile << strGrind;
		SFile << bNoFail; /*no fail upgrade, Juver, 2017/11/26 */

		return true;
	}

};