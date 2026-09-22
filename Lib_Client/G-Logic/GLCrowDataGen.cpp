#include "stdafx.h"
#include "GLCrowDataGen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool SCROWGEN::LOAD_0104 ( basestream &SFile )
{
	SFile >> m_wGenItem_Rate;
	SFile >> m_wGenMoney_Rate;
	SFile >> m_sGenItemID.dwID;
	SFile >> m_dwGenMoney;
	//SFile >> m_strQtGenItem;
	//SFile >> m_strGenItem;

	m_strGenItem.clear();

	std::string strTextArray;
	SFile >> strTextArray;

	std::string strSearch = strTextArray.c_str();
	std::string strDelimiter = ";";

	size_t pos = 0;
	WORD wIndex = 0;
	std::string token;
	while ((pos = strSearch.find(strDelimiter)) != std::string::npos) 
	{
		token = strSearch.substr(0, pos);
		
		if ( token.find("q_") != std::string::npos )
		{
			m_strQtGenItem = token.c_str();
		}
		else
		{
			//more types of genitem might be available
			//add more condition
			if ( m_strGenItem.empty() )
				m_strGenItem = token.c_str();
		}
		
		strSearch.erase(0, pos + strDelimiter.length());
		wIndex++;
	}

	SFile >> strTextArray;

	return true;
}

bool SCROWGEN::LOAD_0103 ( basestream &SFile )
{
	SFile >> m_wGenItem_Rate;
	SFile >> m_wGenMoney_Rate;
	SFile >> m_sGenItemID.dwID;
	SFile >> m_dwGenMoney;
	SFile >> m_strQtGenItem;
	SFile >> m_strGenItem;

	return true;
}

bool SCROWGEN::LOAD_0102 ( basestream &SFile )
{
	SFile >> m_dwGenMoney;
	SFile >> m_sGenItemID.dwID;
	SFile >> m_wGenMoney_Rate;
	SFile >> m_wGenItem_Rate;

	SFile >> m_strGenItem;
	SFile >> m_strQtGenItem;

	return true;
}

bool SCROWGEN::LOAD ( basestream &SFile )
{
	SFile >> m_wGenItem_Rate;
	SFile >> m_wGenMoney_Rate;
	SFile >> m_sGenItemID.dwID;
	SFile >> m_dwGenMoney;
	SFile >> m_strQtGenItem;
	SFile >> m_strGenItem;

	/*Money Luck, Juver, 2017/08/24 */
	SFile >> m_fMoneyLuckRate;
	SFile >> m_wMoneyLuckMax;

	return true;
}

bool SCROWGEN::SAVE ( CSerialFile &SFile )
{
	SFile << m_wGenItem_Rate;
	SFile << m_wGenMoney_Rate;
	SFile << m_sGenItemID.dwID;
	SFile << m_dwGenMoney;
	SFile << m_strQtGenItem;
	SFile << m_strGenItem;

	/*Money Luck, Juver, 2017/08/24 */
	SFile << m_fMoneyLuckRate;
	SFile << m_wMoneyLuckMax;

	return true;
}

VOID SCROWGEN::SaveCsvHead ( std::fstream &SFile )
{
	SFile << "dwGenMoney" << ",";
	SFile << "sGenItemID wMainID" << ",";
	SFile << "sGenItemID wSubID" << ",";
	SFile << "wGenMoney_Rate" << ",";
	SFile << "wGenItem_Rate" << ",";

	/*Money Luck, Juver, 2017/08/24 */
	SFile << "fMoneyLuckRate" << ",";
	SFile << "wMoneyLuckMax" << ",";

	SFile << "strGenItem" << ",";
	SFile << "strQtGenItem" << ",";
}

VOID SCROWGEN::SaveCsv ( std::fstream &SFile )
{
	SFile << m_dwGenMoney << ",";
	SFile << m_sGenItemID.wMainID << ",";
	SFile << m_sGenItemID.wSubID << ",";
	SFile << m_wGenMoney_Rate << ",";
	SFile << m_wGenItem_Rate << ",";

	/*Money Luck, Juver, 2017/08/24 */
	SFile << m_fMoneyLuckRate << ",";
	SFile << m_wMoneyLuckMax << ",";

	STRUTIL::OutputStrCsv( SFile, m_strGenItem );
	STRUTIL::OutputStrCsv( SFile, m_strQtGenItem );
}

VOID SCROWGEN::LoadCsv ( CStringArray &StrArray, int& iCsvCur )
{
	m_dwGenMoney = (DWORD)atol( StrArray[ iCsvCur++ ] );
	m_sGenItemID.wMainID = (WORD)atoi( StrArray[ iCsvCur++ ] );
	m_sGenItemID.wSubID = (WORD)atoi( StrArray[ iCsvCur++ ] );
	m_wGenMoney_Rate = (WORD)atoi( StrArray[ iCsvCur++ ] );
	m_wGenItem_Rate = (WORD)atoi( StrArray[ iCsvCur++ ] );

	/*Money Luck, Juver, 2017/08/24 */
	m_fMoneyLuckRate = (float)atof( StrArray[ iCsvCur++ ] );
	m_wMoneyLuckMax = (WORD)atoi( StrArray[ iCsvCur++ ] );

	STRUTIL::InputStrCsv( StrArray[ iCsvCur++ ], m_strGenItem );
	STRUTIL::InputStrCsv( StrArray[ iCsvCur++ ], m_strQtGenItem );
}