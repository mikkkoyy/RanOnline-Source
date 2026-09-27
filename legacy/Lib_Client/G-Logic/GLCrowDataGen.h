#ifndef GLCROWDATA_GEN_H_
#define GLCROWDATA_GEN_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"

#include "../../Lib_Engine/Meshs/SAnimation.h"
#include "./GLCharDefine.h"


struct SCROWGEN_100
{
	DWORD				m_dwGenMoney;				//	발생 금액.
	SNATIVEID			m_sGenItemID;				//	발생 아이템.
	WORD				m_wGenMoney_Rate;			//	발생 금액 확율.
	WORD				m_wGenItem_Rate;			//	발생 아이템 확율.
};

struct SCROWGEN_101
{
	DWORD				m_dwGenMoney;				//	발생 금액.
	SNATIVEID			m_sGenItemID;				//	발생 아이템.
	WORD				m_wGenMoney_Rate;			//	발생 금액 확율.
	WORD				m_wGenItem_Rate;			//	발생 아이템 확율.

	char				m_szGenItem[ACF_SZNAME];	//	발생 아이템 설정 파일.
};

struct SCROWGEN_102
{
	DWORD				m_dwGenMoney;				//	발생 금액.
	SNATIVEID			m_sGenItemID;				//	발생 아이템.
	WORD				m_wGenMoney_Rate;			//	발생 금액 확율.
	WORD				m_wGenItem_Rate;			//	발생 아이템 확율.

	std::string			m_strGenItem;				//	발생 아이템 설정 파일.
	std::string			m_strQtGenItem;				//	Question Item 설정 파일.
};

struct SCROWGEN_103
{
	WORD				m_wGenItem_Rate;			//	발생 아이템 확율.
	WORD				m_wGenMoney_Rate;			//	발생 금액 확율.
	SNATIVEID			m_sGenItemID;				//	발생 아이템.
	DWORD				m_dwGenMoney;				//	발생 금액.

	std::string			m_strQtGenItem;				//	Question Item 설정 파일.
	std::string			m_strGenItem;				//	발생 아이템 설정 파일.
};

struct SCROWGEN
{
	enum { VERSION = 0x0200, };

	WORD				m_wGenItem_Rate;			//	발생 아이템 확율.
	WORD				m_wGenMoney_Rate;			//	발생 금액 확율.
	SNATIVEID			m_sGenItemID;				//	발생 아이템.
	DWORD				m_dwGenMoney;				//	발생 금액.
	std::string			m_strQtGenItem;				//	Question Item 설정 파일.
	std::string			m_strGenItem;				//	발생 아이템 설정 파일.

	/*Money Luck, Juver, 2017/08/24 */
	float				m_fMoneyLuckRate;
	WORD				m_wMoneyLuckMax;

	SCROWGEN (void) :
	m_wGenItem_Rate(0),
		m_wGenMoney_Rate(0),
		m_sGenItemID(NATIVEID_NULL()),
		m_dwGenMoney(0),

		/*Money Luck, Juver, 2017/08/24 */
		m_fMoneyLuckRate(8.0f),
		m_wMoneyLuckMax(5)
	{
	}

	SCROWGEN ( const SCROWGEN &value )
	{
		operator = ( value );
	}

	void Assign ( SCROWGEN_100 &OldCrowGen )
	{
		m_dwGenMoney = OldCrowGen.m_dwGenMoney;
		m_wGenMoney_Rate = OldCrowGen.m_wGenMoney_Rate;

		m_sGenItemID = OldCrowGen.m_sGenItemID;
		m_wGenItem_Rate = OldCrowGen.m_wGenItem_Rate;
	}

	void Assign ( SCROWGEN_101 &OldCrowGen )
	{
		m_dwGenMoney = OldCrowGen.m_dwGenMoney;
		m_wGenMoney_Rate = OldCrowGen.m_wGenMoney_Rate;

		m_sGenItemID = OldCrowGen.m_sGenItemID;
		m_wGenItem_Rate = OldCrowGen.m_wGenItem_Rate;

		m_strGenItem = OldCrowGen.m_szGenItem;
	}

	SCROWGEN& operator = ( const SCROWGEN &value )
	{
		m_dwGenMoney = value.m_dwGenMoney;
		m_wGenMoney_Rate = value.m_wGenMoney_Rate;
		m_sGenItemID = value.m_sGenItemID;
		m_wGenItem_Rate = value.m_wGenItem_Rate;
		m_strGenItem = value.m_strGenItem;
		m_strQtGenItem = value.m_strQtGenItem;

		/*Money Luck, Juver, 2017/08/24 */
		m_fMoneyLuckRate = value.m_fMoneyLuckRate;
		m_wMoneyLuckMax = value.m_wMoneyLuckMax;

		return *this;
	}

	bool LOAD_0104 ( basestream &SFile );
	bool LOAD_0103 ( basestream &SFile );
	bool LOAD_0102 ( basestream &SFile );
	bool LOAD ( basestream &SFile );
	bool SAVE ( CSerialFile &SFile );

	static VOID SaveCsvHead ( std::fstream &SFile );
	VOID SaveCsv ( std::fstream &SFile );
	VOID LoadCsv ( CStringArray &StrArray, int& iCsvCur );
};

#endif // GLCROWDATA_GEN_H_