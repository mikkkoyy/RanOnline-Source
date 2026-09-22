#include "stdafx.h"
#include "./GLQuest.h"
#include "./GLQuestMan.h"

#include "../../Lib_Engine/G-Logic/GLogic.h"
#include "../../Lib_Engine/Common/StringUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GLQUEST& GLQUEST::operator= ( const GLQUEST &rVALUE )
{
	m_sNID = rVALUE.m_sNID;
	m_dwFlags = rVALUE.m_dwFlags;
	m_dwEDITVER = rVALUE.m_dwEDITVER;

	m_strTITLE = rVALUE.m_strTITLE;
	m_strCOMMENT = rVALUE.m_strCOMMENT;

	m_sSTARTOPT = rVALUE.m_sSTARTOPT;

	//	최초 부여 조건.
	m_dwBeginMoney = rVALUE.m_dwBeginMoney;
	m_dwBeginPartyMemNum = rVALUE.m_dwBeginPartyMemNum;

	//	진행 조건.
	m_dwLimitTime = rVALUE.m_dwLimitTime;
	m_dwLimitPartyMemNum = rVALUE.m_dwLimitPartyMemNum;
	m_bNonDie = rVALUE.m_bNonDie;

	m_vecProgStep = rVALUE.m_vecProgStep;

	//	완료시 보상.
	m_llGiftEXP = rVALUE.m_llGiftEXP;
	m_dwGiftMONEY = rVALUE.m_dwGiftMONEY;
	m_dwGiftELEMENT = rVALUE.m_dwGiftELEMENT;
	m_dwGiftLIFEPOINT = rVALUE.m_dwGiftLIFEPOINT;
	m_dwGiftDisPK = rVALUE.m_dwGiftDisPK;
	m_dwGiftSKILLPOINT = rVALUE.m_dwGiftSKILLPOINT;
	m_dwGiftSTATSPOINT = rVALUE.m_dwGiftSTATSPOINT;

	m_vecGiftITEM = rVALUE.m_vecGiftITEM;
	m_vecGiftSKILL = rVALUE.m_vecGiftSKILL;
	m_dwGiftQUEST = rVALUE.m_dwGiftQUEST;

	m_emQuestArea = rVALUE.m_emQuestArea;

	return *this;
}

bool GLQUEST::LOAD_0001 ( basestream &SFile )
{
	DWORD dwSize(0);
	typedef std::vector<SITEMCUSTOM_103>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	SFile >> m_dwEDITVER;
	SFile >> m_dwFlags;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	//	최초 부여 조건.
	SFile >> m_dwBeginMoney;
	SFile >> m_dwBeginPartyMemNum;

	//	진행 조건.
	SFile >> m_dwLimitTime;
	SFile >> m_dwLimitPartyMemNum;
	SFile >> m_bNonDie;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	//	완료시 보상.
	DWORD dwGiftEXP(0);
	SFile >> dwGiftEXP;
	m_llGiftEXP = dwGiftEXP;

	SFile >> m_dwGiftMONEY;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}

	SFile >> m_vecGiftSKILL;

	return true;
}

bool GLQUEST::LOAD_0002 ( basestream &SFile )
{
	DWORD dwSize(0);
	typedef std::vector<SITEMCUSTOM_104>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	SFile >> m_dwEDITVER;
	SFile >> m_dwFlags;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	//	최초 부여 조건.
	SFile >> m_dwBeginMoney;
	SFile >> m_dwBeginPartyMemNum;

	//	진행 조건.
	SFile >> m_dwLimitTime;
	SFile >> m_dwLimitPartyMemNum;
	SFile >> m_bNonDie;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	//	완료시 보상.
	DWORD dwGiftEXP(0);
	SFile >> dwGiftEXP;
	m_llGiftEXP = dwGiftEXP;

	SFile >> m_dwGiftMONEY;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}

	SFile >> m_vecGiftSKILL;

	return true;
}

bool GLQUEST::LOAD_0003 ( basestream &SFile )
{
	DWORD dwSize(0);
	typedef std::vector<SITEMCUSTOM_105>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	SFile >> m_dwEDITVER;
	SFile >> m_dwFlags;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	//	최초 부여 조건.
	SFile >> m_dwBeginMoney;
	SFile >> m_dwBeginPartyMemNum;

	//	진행 조건.
	SFile >> m_dwLimitTime;
	SFile >> m_dwLimitPartyMemNum;
	SFile >> m_bNonDie;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	//	완료시 보상.
	DWORD dwGiftEXP(0);
	SFile >> dwGiftEXP;
	m_llGiftEXP = dwGiftEXP;

	SFile >> m_dwGiftMONEY;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}

	SFile >> m_vecGiftSKILL;

	return true;
}

bool GLQUEST::LOAD_0004 ( basestream &SFile )
{
	DWORD dwSize(0);
	typedef std::vector<SITEMCUSTOM_106>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	SFile >> m_dwEDITVER;
	SFile >> m_dwFlags;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	//	최초 부여 조건.
	SFile >> m_dwBeginMoney;
	SFile >> m_dwBeginPartyMemNum;

	//	진행 조건.
	SFile >> m_dwLimitTime;
	SFile >> m_dwLimitPartyMemNum;
	SFile >> m_bNonDie;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	//	완료시 보상.
	DWORD dwGiftEXP(0);
	SFile >> dwGiftEXP;
	m_llGiftEXP = dwGiftEXP;

	SFile >> m_dwGiftMONEY;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}

	SFile >> m_vecGiftSKILL;

	return true;
}

bool GLQUEST::LOAD_0005 ( basestream &SFile )
{
	DWORD dwSize(0);
	typedef std::vector<SITEMCUSTOM_106>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	SFile >> m_dwEDITVER;
	SFile >> m_dwFlags;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	m_sSTARTOPT.LOAD ( SFile );

	//	최초 부여 조건.
	SFile >> m_dwBeginMoney;
	SFile >> m_dwBeginPartyMemNum;

	//	진행 조건.
	SFile >> m_dwLimitTime;
	SFile >> m_dwLimitPartyMemNum;
	SFile >> m_bNonDie;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	//	완료시 보상.
	DWORD dwGiftEXP(0);
	SFile >> dwGiftEXP;
	m_llGiftEXP = dwGiftEXP;

	SFile >> m_dwGiftMONEY;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}

	SFile >> m_vecGiftSKILL;
	SFile >> m_dwGiftQUEST;

	return true;
}

bool GLQUEST::LOAD_0006 ( basestream &SFile )
{
	DWORD dwSize(0);
	typedef std::vector<SITEMCUSTOM_108>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	SFile >> m_dwEDITVER;
	SFile >> m_dwFlags;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	m_sSTARTOPT.LOAD ( SFile );

	//	최초 부여 조건.
	SFile >> m_dwBeginMoney;
	SFile >> m_dwBeginPartyMemNum;

	//	진행 조건.
	SFile >> m_dwLimitTime;
	SFile >> m_dwLimitPartyMemNum;
	SFile >> m_bNonDie;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	//	완료시 보상.
	DWORD dwGiftEXP(0);
	SFile >> dwGiftEXP;
	m_llGiftEXP = dwGiftEXP;

	SFile >> m_dwGiftMONEY;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}

	SFile >> m_vecGiftSKILL;
	SFile >> m_dwGiftQUEST;

	return true;
}

bool GLQUEST::LOAD_0007 ( basestream &SFile )
{
	DWORD dwSize(0);
	typedef std::vector<SITEMCUSTOM_108>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	SFile >> m_dwEDITVER;
	SFile >> m_dwFlags;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	m_sSTARTOPT.LOAD ( SFile );

	//	최초 부여 조건.
	SFile >> m_dwBeginMoney;
	SFile >> m_dwBeginPartyMemNum;

	//	진행 조건.
	SFile >> m_dwLimitTime;
	SFile >> m_dwLimitPartyMemNum;
	SFile >> m_bNonDie;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	//	완료시 보상.
	DWORD dwGiftEXP(0);
	SFile >> dwGiftEXP;
	m_llGiftEXP = dwGiftEXP;

	SFile >> m_dwGiftMONEY;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}

	SFile >> m_vecGiftSKILL;
	SFile >> m_dwGiftQUEST;

	return true;
}

bool GLQUEST::LOAD_0008 ( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_109>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_dwEDITVER;
	SFile >> m_dwFlags;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	m_sSTARTOPT.LOAD ( SFile );

	//	최초 부여 조건.
	SFile >> m_dwBeginMoney;
	SFile >> m_dwBeginPartyMemNum;

	//	진행 조건.
	SFile >> m_dwLimitTime;
	SFile >> m_dwLimitPartyMemNum;
	SFile >> m_bNonDie;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	//	완료시 보상.
	DWORD dwGiftEXP(0);
	SFile >> dwGiftEXP;
	m_llGiftEXP = dwGiftEXP;

	SFile >> m_dwGiftMONEY;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}

	//SFile >> m_vecGiftITEM;
	SFile >> m_vecGiftSKILL;
	SFile >> m_dwGiftQUEST;

	return true;
}

bool GLQUEST::LOAD_0009 ( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_110>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_dwEDITVER;
	SFile >> m_dwFlags;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	m_sSTARTOPT.LOAD ( SFile );

	//	최초 부여 조건.
	SFile >> m_dwBeginMoney;
	SFile >> m_dwBeginPartyMemNum;

	//	진행 조건.
	SFile >> m_dwLimitTime;
	SFile >> m_dwLimitPartyMemNum;
	SFile >> m_bNonDie;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	//	완료시 보상.
	DWORD dwGiftEXP(0);
	SFile >> dwGiftEXP;
	m_llGiftEXP = dwGiftEXP;

	SFile >> m_dwGiftMONEY;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}

	//SFile >> m_vecGiftITEM;
	SFile >> m_vecGiftSKILL;
	SFile >> m_dwGiftQUEST;

	return true;
}

bool GLQUEST::LOAD_0010 ( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_111>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_dwEDITVER;
	SFile >> m_dwFlags;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	m_sSTARTOPT.LOAD ( SFile );

	//	최초 부여 조건.
	SFile >> m_dwBeginMoney;
	SFile >> m_dwBeginPartyMemNum;

	//	진행 조건.
	SFile >> m_dwLimitTime;
	SFile >> m_dwLimitPartyMemNum;
	SFile >> m_bNonDie;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	//	완료시 보상.
	DWORD dwGiftEXP(0);
	SFile >> dwGiftEXP;
	m_llGiftEXP = dwGiftEXP;

	SFile >> m_dwGiftMONEY;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}

	//SFile >> m_vecGiftITEM;
	SFile >> m_vecGiftSKILL;
	SFile >> m_dwGiftQUEST;

	return true;
}

bool GLQUEST::LOAD_0011 ( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_112>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_dwEDITVER;
	SFile >> m_dwFlags;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	m_sSTARTOPT.LOAD ( SFile );

	//	최초 부여 조건.
	SFile >> m_dwBeginMoney;
	SFile >> m_dwBeginPartyMemNum;

	//	진행 조건.
	SFile >> m_dwLimitTime;
	SFile >> m_dwLimitPartyMemNum;
	SFile >> m_bNonDie;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	//	완료시 보상.
	DWORD dwGiftEXP(0);
	SFile >> dwGiftEXP;
	m_llGiftEXP = dwGiftEXP;

	SFile >> m_dwGiftMONEY;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}

	//SFile >> m_vecGiftITEM;
	SFile >> m_vecGiftSKILL;
	SFile >> m_dwGiftQUEST;

	return true;
}

bool GLQUEST::LOAD_0012 ( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_112>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_dwEDITVER;
	SFile >> m_dwFlags;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	m_sSTARTOPT.LOAD ( SFile );

	//	최초 부여 조건.
	SFile >> m_dwBeginMoney;
	SFile >> m_dwBeginPartyMemNum;

	//	진행 조건.
	SFile >> m_dwLimitTime;
	SFile >> m_dwLimitPartyMemNum;
	SFile >> m_bNonDie;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	//	완료시 보상.
	DWORD dwGiftEXP(0);
	SFile >> dwGiftEXP;
	m_llGiftEXP = dwGiftEXP;

	SFile >> m_dwGiftMONEY;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}

	//SFile >> m_vecGiftITEM;
	SFile >> m_vecGiftSKILL;
	SFile >> m_dwGiftQUEST;

	SFile >> m_ProgressEvent.bUseProgressEvent;

	if( m_ProgressEvent.bUseProgressEvent )
	{
		SFile >> m_ProgressEvent.dwStartProgress;
		SFile >> m_ProgressEvent.dwEndProgress;
		SFile >> m_ProgressEvent.dwFailProgress;

		SFile >> m_ProgressEvent.dwfailType;

		SFile >> m_ProgressEvent.startMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.startMap.wPosX;
		SFile >> m_ProgressEvent.startMap.wPosY;
		SFile >> m_ProgressEvent.startMap.dwGateID;

		SFile >> m_ProgressEvent.endMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.endMap.wPosX;
		SFile >> m_ProgressEvent.endMap.wPosY;
		SFile >> m_ProgressEvent.endMap.dwGateID;

		SFile >> m_ProgressEvent.failMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.failMap.wPosX;
		SFile >> m_ProgressEvent.failMap.wPosY;
		SFile >> m_ProgressEvent.failMap.dwGateID;
	}

	return true;
}

bool GLQUEST::LOAD_0013 ( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_112>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_dwEDITVER;
	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;
	SFile >> m_dwFlags;

	m_sSTARTOPT.LOAD ( SFile );

	SFile >> m_dwBeginPartyMemNum;
	SFile >> m_dwBeginMoney;
	SFile >> m_dwLimitTime;
	SFile >> m_bNonDie;
	SFile >> m_dwLimitPartyMemNum;
	
	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	SFile >> m_dwGiftMONEY;

	DWORD dwGiftEXP(0);
	SFile >> dwGiftEXP;
	m_llGiftEXP = dwGiftEXP;

	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftQUEST;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}
	//SFile >> m_vecGiftITEM;

	SFile >> m_vecGiftSKILL;
	
	SFile >> m_ProgressEvent.bUseProgressEvent;

	if( m_ProgressEvent.bUseProgressEvent )
	{
		SFile >> m_ProgressEvent.dwStartProgress;
		SFile >> m_ProgressEvent.dwEndProgress;
		SFile >> m_ProgressEvent.dwFailProgress;
		SFile >> m_ProgressEvent.dwfailType;
		SFile >> m_ProgressEvent.startMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.startMap.wPosX;
		SFile >> m_ProgressEvent.startMap.wPosY;
		SFile >> m_ProgressEvent.startMap.dwGateID;
		SFile >> m_ProgressEvent.endMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.endMap.wPosX;
		SFile >> m_ProgressEvent.endMap.wPosY;
		SFile >> m_ProgressEvent.endMap.dwGateID;
		SFile >> m_ProgressEvent.failMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.failMap.wPosX;
		SFile >> m_ProgressEvent.failMap.wPosY;
		SFile >> m_ProgressEvent.failMap.dwGateID;
	}

	return true;
}

bool GLQUEST::LOAD_0014 ( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_112>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_dwEDITVER;
	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;
	SFile >> m_dwFlags;

	m_sSTARTOPT.LOAD ( SFile );

	SFile >> m_dwBeginPartyMemNum;
	SFile >> m_dwBeginMoney;
	SFile >> m_dwLimitTime;
	SFile >> m_bNonDie;
	SFile >> m_dwLimitPartyMemNum;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	SFile >> m_dwGiftMONEY;
	SFile.ReadBuffer( &m_llGiftEXP, sizeof( m_llGiftEXP ));
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftQUEST;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}
	//SFile >> m_vecGiftITEM;

	SFile >> m_vecGiftSKILL;

	SFile >> m_ProgressEvent.bUseProgressEvent;

	if( m_ProgressEvent.bUseProgressEvent )
	{
		SFile >> m_ProgressEvent.dwStartProgress;
		SFile >> m_ProgressEvent.dwEndProgress;
		SFile >> m_ProgressEvent.dwFailProgress;
		SFile >> m_ProgressEvent.dwfailType;
		SFile >> m_ProgressEvent.startMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.startMap.wPosX;
		SFile >> m_ProgressEvent.startMap.wPosY;
		SFile >> m_ProgressEvent.startMap.dwGateID;
		SFile >> m_ProgressEvent.endMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.endMap.wPosX;
		SFile >> m_ProgressEvent.endMap.wPosY;
		SFile >> m_ProgressEvent.endMap.dwGateID;
		SFile >> m_ProgressEvent.failMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.failMap.wPosX;
		SFile >> m_ProgressEvent.failMap.wPosY;
		SFile >> m_ProgressEvent.failMap.dwGateID;
	}

	return true;
}

bool GLQUEST::LOAD_0015 ( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_QUEST_115>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_dwEDITVER;
	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;
	SFile >> m_dwFlags;

	m_sSTARTOPT.LOAD ( SFile );

	SFile >> m_dwBeginPartyMemNum;
	SFile >> m_dwBeginMoney;
	SFile >> m_dwLimitTime;
	SFile >> m_bNonDie;
	SFile >> m_dwLimitPartyMemNum;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	SFile >> m_dwGiftMONEY;
	SFile.ReadBuffer( &m_llGiftEXP, sizeof( m_llGiftEXP ));
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftQUEST;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );
	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}

	//SFile >> m_vecGiftITEM;

	SFile >> m_vecGiftSKILL;

	SFile >> m_ProgressEvent.bUseProgressEvent;

	if( m_ProgressEvent.bUseProgressEvent )
	{
		SFile >> m_ProgressEvent.dwStartProgress;
		SFile >> m_ProgressEvent.dwEndProgress;
		SFile >> m_ProgressEvent.dwFailProgress;
		SFile >> m_ProgressEvent.dwfailType;
		SFile >> m_ProgressEvent.startMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.startMap.wPosX;
		SFile >> m_ProgressEvent.startMap.wPosY;
		SFile >> m_ProgressEvent.startMap.dwGateID;
		SFile >> m_ProgressEvent.endMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.endMap.wPosX;
		SFile >> m_ProgressEvent.endMap.wPosY;
		SFile >> m_ProgressEvent.endMap.dwGateID;
		SFile >> m_ProgressEvent.failMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.failMap.wPosX;
		SFile >> m_ProgressEvent.failMap.wPosY;
		SFile >> m_ProgressEvent.failMap.dwGateID;
	}

	return true;
}

bool GLQUEST::LOAD_0016 ( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_QUEST_115>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);
	DWORD dwData(0);

	SFile >> m_dwEDITVER;
	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;
	SFile >> m_dwFlags;
	SFile >> dwData;	m_emQuestArea = (EMQUEST_AREA) dwData;

	m_sSTARTOPT.LOAD ( SFile );

	SFile >> m_dwBeginPartyMemNum;
	SFile >> m_dwBeginMoney;
	SFile >> m_dwLimitTime;
	SFile >> m_bNonDie;
	SFile >> m_dwLimitPartyMemNum;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	SFile >> m_dwGiftMONEY;
	SFile.ReadBuffer( &m_llGiftEXP, sizeof( m_llGiftEXP ));
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftQUEST;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );
	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}

	//SFile >> m_vecGiftITEM;

	SFile >> m_vecGiftSKILL;

	SFile >> m_ProgressEvent.bUseProgressEvent;

	if( m_ProgressEvent.bUseProgressEvent )
	{
		SFile >> m_ProgressEvent.dwStartProgress;
		SFile >> m_ProgressEvent.dwEndProgress;
		SFile >> m_ProgressEvent.dwFailProgress;
		SFile >> m_ProgressEvent.dwfailType;
		SFile >> m_ProgressEvent.startMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.startMap.wPosX;
		SFile >> m_ProgressEvent.startMap.wPosY;
		SFile >> m_ProgressEvent.startMap.dwGateID;
		SFile >> m_ProgressEvent.endMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.endMap.wPosX;
		SFile >> m_ProgressEvent.endMap.wPosY;
		SFile >> m_ProgressEvent.endMap.dwGateID;
		SFile >> m_ProgressEvent.failMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.failMap.wPosX;
		SFile >> m_ProgressEvent.failMap.wPosY;
		SFile >> m_ProgressEvent.failMap.dwGateID;
	}

	return true;
}

/*dmk14 ver18 quest loader*/
bool GLQUEST::LOAD_0018 ( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_QUEST_116>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);
	DWORD dwData(0);

	SFile >> m_dwEDITVER;
	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;
	SFile >> m_dwFlags;
	SFile >> dwData;	m_emQuestArea = (EMQUEST_AREA) dwData;

	m_sSTARTOPT.LOAD ( SFile );

	SFile >> m_dwBeginPartyMemNum;
	SFile >> m_dwBeginMoney;
	SFile >> m_dwLimitTime;
	SFile >> m_bNonDie;
	SFile >> m_dwLimitPartyMemNum;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	SFile >> m_dwGiftMONEY;
	SFile.ReadBuffer( &m_llGiftEXP, sizeof( m_llGiftEXP ));
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftQUEST;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );
	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}
	//SFile >> m_vecGiftITEM;
	SFile >> m_vecGiftSKILL;

	SFile >> m_ProgressEvent.bUseProgressEvent;

	if( m_ProgressEvent.bUseProgressEvent )
	{
		SFile >> m_ProgressEvent.dwStartProgress;
		SFile >> m_ProgressEvent.dwEndProgress;
		SFile >> m_ProgressEvent.dwFailProgress;

		SFile >> m_ProgressEvent.dwfailType;

		SFile >> m_ProgressEvent.startMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.startMap.wPosX;
		SFile >> m_ProgressEvent.startMap.wPosY;
		SFile >> m_ProgressEvent.startMap.dwGateID;

		SFile >> m_ProgressEvent.endMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.endMap.wPosX;
		SFile >> m_ProgressEvent.endMap.wPosY;
		SFile >> m_ProgressEvent.endMap.dwGateID;

		SFile >> m_ProgressEvent.failMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.failMap.wPosX;
		SFile >> m_ProgressEvent.failMap.wPosY;
		SFile >> m_ProgressEvent.failMap.dwGateID;
	}

	return true;
}

/*dmk14 ver24 quest loader*/
bool GLQUEST::LOAD_0024 ( basestream &SFile )
{
	DWORD dwSize(0);
	DWORD dwData(0);

	SFile >> m_dwEDITVER;
	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;
	SFile >> m_dwFlags;
	SFile >> dwData;	m_emQuestArea = (EMQUEST_AREA) dwData;

	m_sSTARTOPT.LOAD ( SFile );
	m_sAttendanceOption.LOAD( SFile );

	SFile >> m_dwBeginPartyMemNum;
	SFile >> m_dwBeginMoney;
	SFile >> m_dwLimitTime;

	SFile >> m_bTimeLimiorKeep;
	SFile >> m_bPastOnlyOnOline;
	SFile >> m_bNonDie;

	SFile >> m_dwLimitPartyMemNum;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	SFile >> m_dwGiftMONEY;
	//SFile >> m_llGiftEXP;	
	SFile.ReadBuffer( &m_llGiftEXP, sizeof( m_llGiftEXP ));
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftQUEST;

	// 아이템 정보가 바뀌면 작업을 해야된다.
	typedef std::vector< SITEMCUSTOM_119 > INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator INVENQARRAY_OLD_ITER;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;
	{
		m_vecGiftITEM.clear();
		m_vecGiftITEM.reserve( vecGIFT_QITEM.size() );

		INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
		INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
		for ( ; iter!=iter_end; ++iter )
		{
			SITEMCUSTOM sCUSTOM;
			sCUSTOM.Assign( ( *iter ) );
			m_vecGiftITEM.push_back( sCUSTOM );
		}
	}

	SFile >> m_vecGiftSKILL;


	SFile >> m_ProgressEvent.bUseProgressEvent;

	if( m_ProgressEvent.bUseProgressEvent )
	{
		SFile >> m_ProgressEvent.dwStartProgress;
		SFile >> m_ProgressEvent.dwEndProgress;
		SFile >> m_ProgressEvent.dwFailProgress;

		SFile >> m_ProgressEvent.dwfailType;

		SFile >> m_ProgressEvent.startMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.startMap.wPosX;
		SFile >> m_ProgressEvent.startMap.wPosY;
		SFile >> m_ProgressEvent.startMap.dwGateID;

		SFile >> m_ProgressEvent.endMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.endMap.wPosX;
		SFile >> m_ProgressEvent.endMap.wPosY;
		SFile >> m_ProgressEvent.endMap.dwGateID;

		SFile >> m_ProgressEvent.failMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.failMap.wPosX;
		SFile >> m_ProgressEvent.failMap.wPosY;
		SFile >> m_ProgressEvent.failMap.dwGateID;
	}

	return true;
}

bool GLQUEST::LOAD_0200 ( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_112>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_dwEDITVER;
	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;
	SFile >> m_dwFlags;

	m_sSTARTOPT.LOAD ( SFile );

	SFile >> m_dwBeginPartyMemNum;
	SFile >> m_dwBeginMoney;
	SFile >> m_dwLimitTime;
	SFile >> m_bNonDie;
	SFile >> m_dwLimitPartyMemNum;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	SFile >> m_dwGiftMONEY;
	SFile.ReadBuffer( &m_llGiftEXP, sizeof( m_llGiftEXP ));
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftQUEST;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}

	//SFile >> m_vecGiftITEM;

	SFile >> m_vecGiftSKILL;

	SFile >> m_ProgressEvent.bUseProgressEvent;

	if( m_ProgressEvent.bUseProgressEvent )
	{
		SFile >> m_ProgressEvent.dwStartProgress;
		SFile >> m_ProgressEvent.dwEndProgress;
		SFile >> m_ProgressEvent.dwFailProgress;
		SFile >> m_ProgressEvent.dwfailType;
		SFile >> m_ProgressEvent.startMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.startMap.wPosX;
		SFile >> m_ProgressEvent.startMap.wPosY;
		SFile >> m_ProgressEvent.startMap.dwGateID;
		SFile >> m_ProgressEvent.endMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.endMap.wPosX;
		SFile >> m_ProgressEvent.endMap.wPosY;
		SFile >> m_ProgressEvent.endMap.dwGateID;
		SFile >> m_ProgressEvent.failMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.failMap.wPosX;
		SFile >> m_ProgressEvent.failMap.wPosY;
		SFile >> m_ProgressEvent.failMap.dwGateID;
	}

	return true;
}

bool GLQUEST::LOAD_0201 ( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_112>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);
	DWORD dwData(0);

	SFile >> m_dwEDITVER;
	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;
	SFile >> m_dwFlags;
	SFile >> dwData;	m_emQuestArea = (EMQUEST_AREA) dwData;

	m_sSTARTOPT.LOAD ( SFile );

	SFile >> m_dwBeginPartyMemNum;
	SFile >> m_dwBeginMoney;
	SFile >> m_dwLimitTime;
	SFile >> m_bNonDie;
	SFile >> m_dwLimitPartyMemNum;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	SFile >> m_dwGiftMONEY;
	SFile.ReadBuffer( &m_llGiftEXP, sizeof( m_llGiftEXP ));
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftQUEST;

	INVENQARRAY_OLD vecGiftITEM;
	SFile >> vecGiftITEM;
	m_vecGiftITEM.clear();
	m_vecGiftITEM.reserve ( vecGiftITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGiftITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGiftITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGiftITEM.push_back ( sCUSTOM );
	}

	//SFile >> m_vecGiftITEM;

	SFile >> m_vecGiftSKILL;

	SFile >> m_ProgressEvent.bUseProgressEvent;

	if( m_ProgressEvent.bUseProgressEvent )
	{
		SFile >> m_ProgressEvent.dwStartProgress;
		SFile >> m_ProgressEvent.dwEndProgress;
		SFile >> m_ProgressEvent.dwFailProgress;
		SFile >> m_ProgressEvent.dwfailType;
		SFile >> m_ProgressEvent.startMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.startMap.wPosX;
		SFile >> m_ProgressEvent.startMap.wPosY;
		SFile >> m_ProgressEvent.startMap.dwGateID;
		SFile >> m_ProgressEvent.endMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.endMap.wPosX;
		SFile >> m_ProgressEvent.endMap.wPosY;
		SFile >> m_ProgressEvent.endMap.dwGateID;
		SFile >> m_ProgressEvent.failMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.failMap.wPosX;
		SFile >> m_ProgressEvent.failMap.wPosY;
		SFile >> m_ProgressEvent.failMap.dwGateID;
	}

	return true;
}


bool GLQUEST::LOAD_0202 ( basestream &SFile )
{
	DWORD dwSize(0);
	DWORD dwData(0);

	SFile >> m_dwEDITVER;
	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;
	SFile >> m_dwFlags;
	SFile >> dwData;	m_emQuestArea = (EMQUEST_AREA) dwData;

	m_sSTARTOPT.LOAD ( SFile );
	m_sAttendanceOption.LOAD( SFile );

	SFile >> m_dwBeginPartyMemNum;
	SFile >> m_dwBeginMoney;
	SFile >> m_dwLimitTime;
	SFile >> m_bTimeLimiorKeep;
	SFile >> m_bPastOnlyOnOline;
	SFile >> m_bNonDie;
	SFile >> m_dwLimitPartyMemNum;

	SFile >> dwSize;
	for ( DWORD i=0; i<dwSize; ++i )
	{
		GLQUEST_STEP sSTEP;
		sSTEP.LOAD ( SFile );
		m_vecProgStep.push_back ( sSTEP );
	}

	SFile >> m_dwGiftMONEY;
	SFile.ReadBuffer( &m_llGiftEXP, sizeof( m_llGiftEXP ));
	SFile >> m_dwGiftSKILLPOINT;
	SFile >> m_dwGiftSTATSPOINT;
	SFile >> m_dwGiftLIFEPOINT;
	SFile >> m_dwGiftDisPK;
	SFile >> m_dwGiftELEMENT;
	SFile >> m_dwGiftQUEST;
	SFile >> m_vecGiftITEM;
	SFile >> m_vecGiftSKILL;

	SFile >> m_ProgressEvent.bUseProgressEvent;

	if( m_ProgressEvent.bUseProgressEvent )
	{
		SFile >> m_ProgressEvent.dwStartProgress;
		SFile >> m_ProgressEvent.dwEndProgress;
		SFile >> m_ProgressEvent.dwFailProgress;
		SFile >> m_ProgressEvent.dwfailType;
		SFile >> m_ProgressEvent.startMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.startMap.wPosX;
		SFile >> m_ProgressEvent.startMap.wPosY;
		SFile >> m_ProgressEvent.startMap.dwGateID;
		SFile >> m_ProgressEvent.endMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.endMap.wPosX;
		SFile >> m_ProgressEvent.endMap.wPosY;
		SFile >> m_ProgressEvent.endMap.dwGateID;
		SFile >> m_ProgressEvent.failMap.nidMAP.dwID;
		SFile >> m_ProgressEvent.failMap.wPosX;
		SFile >> m_ProgressEvent.failMap.wPosY;
		SFile >> m_ProgressEvent.failMap.dwGateID;
	}

	return true;
}


bool GLQUEST::LOAD ( const char *szFile )
{
	std::string strLIST_FILE = std::string(GLQuestMan::GetInstance().GetPath()) + szFile;

	std::auto_ptr<basestream> pBStream( GLOGIC::openfile_basestream(GLOGIC::bGLOGIC_ZIPFILE, 
																	GLOGIC::strQUEST_ZIPFILE.c_str(),
																	strLIST_FILE.c_str(), 
																	szFile, 
																	EMBYTECRYPT_OLD,
																	GLOGIC::bGLOGIC_PACKFILE ) );

	if ( !pBStream.get() )								
	{
		if ( strlen( szFile ) > 0 )
			CDebugSet::ErrorFile( "GLQUEST::LOAD", szFile );

		return false;
	}

	basestream &SFile = *pBStream;

	DWORD dwVer(0);
	SFile >> dwVer;

	switch ( dwVer )
	{
	case 0x0001:	LOAD_0001(SFile);	break;
	case 0x0002:	LOAD_0002(SFile);	break;
	case 0x0003:	LOAD_0003(SFile);	break;
	case 0x0004:	LOAD_0004(SFile);	break;
	case 0x0005:	LOAD_0005(SFile);	break;
	case 0x0006:	LOAD_0006(SFile);	break;
	case 0x0007:	LOAD_0007(SFile);	break;
	case 0x0008:	LOAD_0008(SFile);	break;
	case 0x0009:	LOAD_0009(SFile);	break;
	case 0x0010:	LOAD_0010(SFile);	break;
	case 0x0011:	LOAD_0011(SFile);	break;
	case 0x0012:	LOAD_0012(SFile);	break;
	case 0x0013:	LOAD_0013(SFile);	break;
	case 0x0014:	LOAD_0014(SFile);	break;
	case 0x0015:	LOAD_0015(SFile);	break;
	case 0x0016:	LOAD_0016(SFile);	break;
		/*dmk14 ver18 quest loader*/
	case 0x0018:	LOAD_0018(SFile);	break;
		/*dmk14 ver24 quest loader*/
	case 0x0024:	LOAD_0024(SFile);	break;
	case 0x0200:	LOAD_0200(SFile);	break;
	case 0x0201:	LOAD_0201(SFile);	break;
	case VERSION:	LOAD_0202(SFile);	break;

	default:
		CDebugSet::ErrorVersion( "GLQUEST::LOAD", dwVer );
		return false;
	};

	return true;
}

void GLQUEST::LoadCsv( CStringArray &StrArray, CStringArray &StrArrayProgress )
{
	LoadCsvInfoDate( StrArray );
	LoadCsvBaseData( StrArray );
	LoadCsvProgress( StrArrayProgress );
}

void GLQUEST::LoadCsvInfoDate( CStringArray &StrArray )
{
	return;
}

void GLQUEST::LoadCsvBaseData( CStringArray &StrArray )
{
	m_dwBeginMoney = (DWORD) atoi( StrArray[2] );

	int nActionCondition = atoi( StrArray[3] ); // 퀘스트 수행 특성
	
	switch ( nActionCondition )
	{
	case 1: 
		m_dwFlags = EMQF_GIVEUP;
		break;
	case 2: 
		m_dwFlags = EMQF_GIVEUP;
		break;
	case 3: 
		m_dwFlags = EMQF_GIVEUP;
		break;
	case 4: 
		m_dwFlags = EMQF_REPEAT + EMQF_AGAIN;
		break;
	case 5: 
		m_dwFlags = EMQF_REPEAT + EMQF_GIVEUP;
		break;
	case 6: 
		m_dwFlags = EMQF_AGAIN + EMQF_GIVEUP;
		break;
	case 7: 
		m_dwFlags = EMQF_REPEAT + EMQF_AGAIN + EMQF_GIVEUP;
		break;
	default:
		break;
	}

	m_dwLimitTime = (DWORD) atoi( StrArray[4] ); // 제한시간
	m_sSTARTOPT.dwCLASS = (DWORD) atoi( StrArray[5] ); // 클래스조건
	
	int nSchool = atoi( StrArray[6] ); // 학원 조건 

	if ( nSchool == 0 ) m_sSTARTOPT.wSCHOOL = 10; // 전체
	else if ( nSchool == 1 ) m_sSTARTOPT.wSCHOOL = 0; // 성문
	else if ( nSchool == 2 ) m_sSTARTOPT.wSCHOOL = 1; // 현암
	else if ( nSchool == 3 ) m_sSTARTOPT.wSCHOOL = 2; // 봉황

	// 퀘스트 조건
	m_sSTARTOPT.nidITEM.wMainID = (WORD)atoi( StrArray[7] );
	m_sSTARTOPT.nidITEM.wSubID = (WORD)atoi( StrArray[8] );
	m_sSTARTOPT.nidSKILL.wMainID = (WORD)atoi( StrArray[9] );
	m_sSTARTOPT.nidSKILL.wSubID = (WORD)atoi( StrArray[10] );
	m_sSTARTOPT.wLEVEL = (WORD)atoi( StrArray[11] );

	m_llGiftEXP = (LONGLONG)_atoi64( StrArray[12] );
	m_dwGiftDisPK = (DWORD)atoi( StrArray[13] ); 
	m_dwGiftMONEY = (DWORD)atoi( StrArray[14] );
	m_dwGiftSKILLPOINT = (DWORD)atoi( StrArray[15] );
	m_dwGiftELEMENT = (DWORD)atoi( StrArray[16] );
	m_dwGiftSTATSPOINT = (DWORD)atoi( StrArray[17] );
	m_dwGiftLIFEPOINT = (DWORD)atoi( StrArray[18] );
	m_dwGiftQUEST = (DWORD)atoi( StrArray[19] );

	// 보상 아이템
	const int nItemMAX = 10;
	const int nSkillMAX = 6;

	SITEMCUSTOM sItemCustom;
	
	m_vecGiftITEM.clear();	
	
	for( int i=0; i<nItemMAX; ++i )
	{
		if( StrArray[20+(i*10)] == " " ) break;

		sItemCustom.sNativeID.wMainID = (WORD)atoi( StrArray[20+(i*10) ] );
		sItemCustom.sNativeID.wSubID = (WORD)atoi( StrArray[21+(i*10) ] );
		sItemCustom.cDAMAGE = (BYTE) atoi( StrArray[22+(i*10) ] );
		sItemCustom.cDEFENSE = (BYTE)atoi( StrArray[23+(i*10) ] );
		sItemCustom.wTurnNum = (WORD)atoi( StrArray[24+(i*10) ] );
		sItemCustom.cRESIST_ELEC = (BYTE)atoi( StrArray[25+(i*10) ] );
		sItemCustom.cRESIST_FIRE = (BYTE)atoi( StrArray[26+(i*10) ] );
		sItemCustom.cRESIST_ICE = (BYTE)atoi( StrArray[27+(i*10) ] );
		sItemCustom.cRESIST_POISON = (BYTE)atoi( StrArray[28+(i*10) ] );
		sItemCustom.cRESIST_SPIRIT = (BYTE)atoi( StrArray[29+(i*10) ] );

		m_vecGiftITEM.push_back(sItemCustom);
	}

	SNATIVEID sNativeID;
	
	m_vecGiftSKILL.clear();

	for( int i=0; i<nSkillMAX; ++i )
	{
		if (StrArray[120+(i*10)] == " " ) break;
		
		sNativeID.wMainID = (WORD)atoi( StrArray[120+(i*10) ] );
		sNativeID.wSubID = (WORD)atoi( StrArray[121+(i*10) ] );
		m_vecGiftSKILL.push_back( sNativeID.dwID );
	}
}

void GLQUEST::LoadCsvProgress( CStringArray &StrArray )
{
	LoadCsvProgressBaseHead( StrArray );
	LoadCsvProgressBaseData( StrArray );
}

void GLQUEST::LoadCsvProgressBaseHead( CStringArray &StrArray )
{
	return;
}

void GLQUEST::LoadCsvProgressBaseData( CStringArray &StrArray )
{
	return;
}

bool GLQUEST::SAVE ( const char *szFile )
{
	std::string strLIST_FILE = std::string(GLQuestMan::GetInstance().GetPath()) + szFile;

	CSerialFile SFile;
	BOOL bOPEN = SFile.OpenFile ( FOT_WRITE, strLIST_FILE.c_str(), EMBYTECRYPT_OLD );
	if ( !bOPEN )					
		return false;

	SFile << (DWORD) VERSION;
	SFile << m_dwEDITVER;
	SFile << m_strTITLE;
	SFile << m_strCOMMENT;
	SFile << m_dwFlags;
	SFile << (DWORD)m_emQuestArea;

	m_sSTARTOPT.SAVE ( SFile );
	m_sAttendanceOption.SAVE( SFile );

	SFile << m_dwBeginPartyMemNum;
	SFile << m_dwBeginMoney;
	SFile << m_dwLimitTime;

	SFile << m_bTimeLimiorKeep;
	SFile << m_bPastOnlyOnOline;

	SFile << m_bNonDie;
	SFile << m_dwLimitPartyMemNum;
	
	SFile << (DWORD) m_vecProgStep.size();
	VECQSTEP_ITER iter = m_vecProgStep.begin();
	VECQSTEP_ITER iter_end = m_vecProgStep.end();
	for ( ; iter!=iter_end; ++iter )
	{
		(*iter).SAVE ( SFile );
	}

	SFile << m_dwGiftMONEY;
	SFile.WriteBuffer( &m_llGiftEXP, sizeof( m_llGiftEXP ) );
	SFile << m_dwGiftSKILLPOINT;
	SFile << m_dwGiftSTATSPOINT;
	SFile << m_dwGiftLIFEPOINT;
	SFile << m_dwGiftDisPK;
	SFile << m_dwGiftELEMENT;
	SFile << m_dwGiftQUEST;
	SFile << m_vecGiftITEM;
	SFile << m_vecGiftSKILL;
	
	SFile << m_ProgressEvent.bUseProgressEvent;
	if( m_ProgressEvent.bUseProgressEvent )
	{
		SFile << m_ProgressEvent.dwStartProgress;
		SFile << m_ProgressEvent.dwEndProgress;
		SFile << m_ProgressEvent.dwFailProgress;
		SFile << m_ProgressEvent.dwfailType;
		SFile << m_ProgressEvent.startMap.nidMAP.dwID;
		SFile << m_ProgressEvent.startMap.wPosX;
		SFile << m_ProgressEvent.startMap.wPosY;
		SFile << m_ProgressEvent.startMap.dwGateID;
		SFile << m_ProgressEvent.endMap.nidMAP.dwID;
		SFile << m_ProgressEvent.endMap.wPosX;
		SFile << m_ProgressEvent.endMap.wPosY;
		SFile << m_ProgressEvent.endMap.dwGateID;
		SFile << m_ProgressEvent.failMap.nidMAP.dwID;
		SFile << m_ProgressEvent.failMap.wPosX;
		SFile << m_ProgressEvent.failMap.wPosY;
		SFile << m_ProgressEvent.failMap.dwGateID;
	}

	return true;
}

void GLQUEST::SaveCsv( std::fstream &SFile, CString strFileName )
{
	SaveCsvInfoHead( SFile, strFileName );
	SaveCsvBaseHead( SFile );
	SaveCsvInfoData( SFile );
	SaveCsvBaseData( SFile );

	SaveCsvProgress( SFile );
}

void GLQUEST::SaveCsvInfoHead( std::fstream &SFile,  CString strFileName)
{
	std::string strTemp;

	strTemp = strFileName.GetString();
	STRUTIL::OutputStrCsv( SFile, strTemp );

	SFile << "Quest_ID" << ",";
}

void GLQUEST::SaveCsvBaseHead( std::fstream &SFile )
{
	SFile << "ConditionMoney" << ",";
	SFile << "BasicActionCondition" << ",";
	SFile << "ConditionTime" << ",";
	SFile << "UsedCharClass" << ",";
	SFile << "UsedSchool" << ",";	
	
	SFile << "ConditionItemMid" << ",";
	SFile << "ConditionItemSid" << ",";
	SFile << "ConditionSkillMid" << ",";
	SFile << "ConditionSkillSid" << ",";
	SFile << "ConditionLevel" << ",";

	SFile << "CompensationExp" << ",";
	SFile << "CompensationPK" << ",";
	SFile << "CompensationMoney" << ",";
	SFile << "CompensationSkillPoint" << ",";
	SFile << "CompensationAttribute" << ",";
	SFile << "CompensationStatsPoint" << ",";
	SFile << "CompensationLiving" << ",";
	SFile << "ConnectionNextQuest";
	
	CString strTemp;
	const int nItemMAX = 10;
	const int nSkillMAX = 6;

	for ( int i = 0 ; i < nItemMAX; ++i ) 
	{
		SFile << ",";
		strTemp.Format( "%d" , i );
		SFile << "CompensationItemMid" << strTemp << ",";
		SFile << "CompensationItemSid" << strTemp << ",";
		SFile << "InchantDamage" << strTemp << ",";
		SFile << "InchantDefense" << strTemp << ",";
		SFile << "TrunNum" << strTemp << ",";
		SFile << "ResistElectric" << strTemp << ",";
		SFile << "ResistFire" << strTemp << ",";
		SFile << "ResistIce" << strTemp << ",";
		SFile << "ResistPoison" << strTemp << ",";
		SFile << "ResistSpirit" << strTemp;
	}	

	for ( int i = 0; i < nSkillMAX; ++ i ) 
	{
		SFile << ",";
		strTemp.Format( "%d", i );
		SFile << "CompensationSkillMid" << strTemp << ",";
		SFile << "CompensationSkillSid" << strTemp;
	}

	SFile << std::endl;
}

void GLQUEST::SaveCsvInfoData( std::fstream &SFile )
{
	SFile << " " << "," ;
	SFile << m_sNID.dwID << ",";
}

void GLQUEST::SaveCsvBaseData( std::fstream &SFile )
{
	SFile << m_dwBeginMoney << ",";


	int nActionCondition=0; // 퀘스트 수행 특성
	if ( IsREPEAT () && IsAGAIN () && IsGIVEUP () )	nActionCondition = 7;
	else if ( IsAGAIN() && IsGIVEUP() ) nActionCondition = 6;
	else if ( IsREPEAT() && IsGIVEUP() ) nActionCondition = 5;
	else if ( IsREPEAT() && IsAGAIN() ) nActionCondition = 4;
	else if ( IsGIVEUP() ) nActionCondition = 3;
	else if ( IsAGAIN() ) nActionCondition = 2;
	else if ( IsREPEAT() ) nActionCondition = 1;

	SFile << nActionCondition << ",";
	SFile << m_dwLimitTime << ",";
	SFile << m_sSTARTOPT.dwCLASS << ",";
	
	
	int nSchool = 0; // 학원 조건 
	if ( m_sSTARTOPT.wSCHOOL == 10 ) nSchool = 0; // 전체
	else if ( m_sSTARTOPT.wSCHOOL == 0 ) nSchool = 1; // 성문
	else if ( m_sSTARTOPT.wSCHOOL == 1 ) nSchool = 2; // 현암
	else if ( m_sSTARTOPT.wSCHOOL == 2 ) nSchool = 3; // 봉황

	// 퀘스트 조건
    SFile << nSchool << ",";
	SFile << m_sSTARTOPT.nidITEM.wMainID << ",";
	SFile << m_sSTARTOPT.nidITEM.wSubID << ",";
	SFile << m_sSTARTOPT.nidSKILL.wMainID << ",";
	SFile << m_sSTARTOPT.nidSKILL.wSubID << ",";
	SFile << m_sSTARTOPT.wLEVEL << ",";

	// 보상
	SFile << m_llGiftEXP << ",";
	SFile << m_dwGiftDisPK << ",";
	SFile << m_dwGiftMONEY << ",";
	SFile << m_dwGiftSKILLPOINT << ",";
	SFile << m_dwGiftELEMENT << ",";
	SFile << m_dwGiftSTATSPOINT << ",";	
	SFile << m_dwGiftLIFEPOINT << ",";	
	int nTemp = (int) m_dwGiftQUEST;
	SFile << nTemp;


	// 보상 아이템
	int iGiftItem = (int)m_vecGiftITEM.size();
	const int nItemMAX = 10;
	const int nSkillMAX = 6;

	for( int i=0; i<nItemMAX; ++i )
	{
		SFile << ",";
		if ( i < iGiftItem )
		{
			SFile << m_vecGiftITEM[i].sNativeID.wMainID << ",";
			SFile << m_vecGiftITEM[i].sNativeID.wSubID << ",";

			SFile << (int)m_vecGiftITEM[i].cDAMAGE << ",";
			SFile << (int)m_vecGiftITEM[i].cDEFENSE << ",";
			SFile << m_vecGiftITEM[i].wTurnNum << ",";
	        
			SFile << (int)m_vecGiftITEM[i].cRESIST_ELEC << ",";
			SFile << (int)m_vecGiftITEM[i].cRESIST_FIRE << ",";
			SFile << (int)m_vecGiftITEM[i].cRESIST_ICE << ",";
			SFile << (int)m_vecGiftITEM[i].cRESIST_POISON << ",";
			SFile << (int)m_vecGiftITEM[i].cRESIST_SPIRIT;
		}
		else
		{
			SFile << " " << ",";
			SFile << " " << ",";
			SFile << " " << ",";
			SFile << " " << ",";
			SFile << " " << ",";
			SFile << " " << ",";
			SFile << " " << ",";
			SFile << " " << ",";
			SFile << " " << ",";
			SFile << " ";
		}
	}

	// 보상 스킬
	int iGiftSkill = (int)m_vecGiftSKILL.size();
	SNATIVEID sNativeID;

	for( int i=0; i<nSkillMAX; ++i )
	{
		SFile << ",";
		if ( i < iGiftSkill ) 
		{
			sNativeID = m_vecGiftSKILL[i];
			SFile << sNativeID.wMainID << ",";
			SFile << sNativeID.wSubID;
		}
		else
		{
			SFile << " " << ",";
			SFile << " ";
		}
	}

	SFile << std::endl;
}

void GLQUEST::SaveCsvProgress( std::fstream &SFile )
{
	SaveCsvProgressBaseHead( SFile );	
	SaveCsvProgressBaseData( SFile );
}

void GLQUEST::SaveCsvProgressBaseHead( std::fstream &SFile )
{
	int iProgStep = (int)m_vecProgStep.size();
	const int nStepMAX = 30;

	SFile << "ProgStepCnt";

	CString strTemp;

	for ( int i = 0; i < nStepMAX; ++i ) 
	{
		if ( i < iProgStep )
		{
			strTemp.Format("%d",i);
			SFile << ",";
			SFile << "TryNPCTalk" << strTemp << ",";
			SFile << "KillMOB" << strTemp << ",";
			SFile << "ReachMap" << strTemp << ",";
			SFile << "TakeItem" << strTemp ;
		}
		else
		{
			SFile << ",";
			SFile << " " << ",";
			SFile << " " << ",";
			SFile << " " << ",";
			SFile << " ";
		}
	}

	SFile << std::endl;
}

void GLQUEST::SaveCsvProgressBaseData( std::fstream &SFile )
{
	int iProgStep = (int)m_vecProgStep.size();
	const int nStepMAX = 30;

	SFile << iProgStep;

	CString strTemp;

	for ( int i = 0; i < nStepMAX; ++i ) 
	{
		if ( i < iProgStep )
		{
			strTemp.Format("%d",i);
			SFile << ",";
			SFile << m_vecProgStep[i].IsNEED_NPCTALK() << ",";
			SFile << m_vecProgStep[i].IsNEED_MOBKILL() << ",";
			SFile << m_vecProgStep[i].IsNEED_REACHZONE() << ",";
			SFile << m_vecProgStep[i].IsNEED_QITEM() ;
		}
		else
		{
			SFile << ",";
			SFile << " " << ",";
			SFile << " " << ",";
			SFile << " " << ",";
			SFile << " ";
		}
	}

	SFile << std::endl;
}

void GLQUEST::ExportText( std::fstream &SFile )
{
	
	SFile << "<BEGIN>" << std::endl;
	SFile << "<QUEST_ID>" << m_sNID.dwID << std::endl;
	ExportTextSubJect(SFile);
	ExportTextProgress(SFile);
	SFile << "<END>" << std::endl;

}

void GLQUEST::ExportTextSubJect( std::fstream &SFile )
{
	
	SFile << "<SUBJECT>" << std::endl;
	SFile << m_strTITLE << std::endl;
	SFile << "</SUBJECT>" << std::endl;

	SFile << "<DESCRIPTION>" << std::endl;
	SFile << m_strCOMMENT << std::endl;
	SFile << "</DESCRIPTION>"<< std::endl;

}

void GLQUEST::ExportTextProgress( std::fstream &SFile )
{
	int iProgStep = (int)m_vecProgStep.size();
	
	for ( int i = 0; i< iProgStep; ++i)
	{
		SFile << "<PROGRESS>" << std::endl;
		SFile << m_vecProgStep[i].m_strTITLE << std::endl;
		SFile << "</PROGRESS>"<< std::endl;

		SFile << "<DESCRIPTION>" << std::endl;
		SFile << m_vecProgStep[i].m_strCOMMENT << std::endl;
		SFile << "</DESCRIPTION>"<< std::endl;
	}

}

void GLQUEST::ImportText( CStringFile& strFile )
{
	ImportTextSubject( strFile );
	ImportTextProgress( strFile );
}

void GLQUEST::ImportTextSubject( CStringFile& strFile )
{
	CString strLine;
	
	strFile.GetNextLine( strLine );
			
	if ( strLine != "<SUBJECT>" ) return;

	m_strTITLE = "";
	
	strFile.GetNextLine( strLine );
	
	while ( strLine != "</SUBJECT>" )
	{
		m_strTITLE += strLine;	
		
		strFile.GetNextLine( strLine );
		
		if ( strLine != "</SUBJECT>" ) m_strTITLE += "\r\n";
	}


	strFile.GetNextLine( strLine );
			
	if ( strLine != "<DESCRIPTION>" ) return;

	m_strCOMMENT = "";
	
	strFile.GetNextLine( strLine );

	while ( strLine != "</DESCRIPTION>" )	{
		
		m_strCOMMENT += strLine;
		
		strFile.GetNextLine( strLine );

		if ( strLine != "</DESCRIPTION>" ) m_strCOMMENT += "\r\n";

	}

	return;
}

void GLQUEST::ImportTextProgress( CStringFile& strFile )
{
	CString strLine;
	CString strError;
	
	strFile.GetNextLine( strLine );
	
	int index = 0;
	int nStep = m_vecProgStep.size();


	while ( strLine != "<END>" )
	{
		if ( index >= nStep )
		{
			strError.Format("ID=%d overfull progress step",m_sNID.dwID);
			AfxMessageBox(strError,MB_OK );
			return;
		}
		if ( strLine != "<PROGRESS>" ) return;

		strFile.GetNextLine( strLine );

		m_vecProgStep[index].m_strTITLE = "";

		while ( strLine != "</PROGRESS>" )
		{
			m_vecProgStep[index].m_strTITLE += strLine;

			strFile.GetNextLine( strLine );

			if ( strLine != "</PROGRESS>" ) m_vecProgStep[index].m_strTITLE += "\r\n";

		}

		strFile.GetNextLine( strLine ); 

		if ( strLine != "<DESCRIPTION>" ) return;

		strFile.GetNextLine( strLine ); 

		m_vecProgStep[index].m_strCOMMENT = "";

		while ( strLine != "</DESCRIPTION>" )
		{
			m_vecProgStep[index].m_strCOMMENT += strLine;

			strFile.GetNextLine( strLine );

			if ( strLine != "</DESCRIPTION>" ) m_vecProgStep[index].m_strCOMMENT += "\r\n";

		} 

		strFile.GetNextLine( strLine );
		++index;
	}
}


GLQUEST_STEP* GLQUEST::GetSTEP ( DWORD dwSTEP )
{
	if ( dwSTEP >= m_vecProgStep.size() )	return NULL;
	return &(m_vecProgStep[dwSTEP]);
}

DWORD GLQUEST::ToUpStep ( DWORD dwSTEP )
{
	if ( m_vecProgStep.empty() )			return 0;
	if ( dwSTEP >= m_vecProgStep.size() )	return UINT_MAX;
	if ( dwSTEP < 1 )						return dwSTEP;

	GLQUEST_STEP sSTEP_A, sSTEP_B;

	sSTEP_A = m_vecProgStep[dwSTEP-1];
	sSTEP_B = m_vecProgStep[dwSTEP];

	m_vecProgStep[dwSTEP-1] = sSTEP_B;
	m_vecProgStep[dwSTEP] = sSTEP_A;

	return dwSTEP-1;
}

DWORD GLQUEST::ToDownStep ( DWORD dwSTEP )
{
	if ( m_vecProgStep.empty() )				return 0;
	if ( (dwSTEP+1) >= m_vecProgStep.size() )	return DWORD(m_vecProgStep.size()-1);

	GLQUEST_STEP sSTEP_A, sSTEP_B;

	sSTEP_A = m_vecProgStep[dwSTEP+1];
	sSTEP_B = m_vecProgStep[dwSTEP];

	m_vecProgStep[dwSTEP+1] = sSTEP_B;
	m_vecProgStep[dwSTEP] = sSTEP_A;

	return dwSTEP+1;
}


GLQUESTPROG_STEP& GLQUESTPROG_STEP::operator= ( const GLQUESTPROG_STEP &rVALUE )
{
	 m_bTALK = rVALUE.m_bTALK;
	 m_bQITEM = rVALUE.m_bQITEM;

	 m_bMOBKILL_ALL = rVALUE.m_bMOBKILL_ALL;
	 m_dwNUM_MOBKILL = rVALUE.m_dwNUM_MOBKILL;
	 m_bNPCGUARD_DEATH = rVALUE.m_bNPCGUARD_DEATH;

	 m_bREACH_ZONE = rVALUE.m_bREACH_ZONE;
	
	 m_bDEFENSE_ZONE_BEGIN = rVALUE.m_bDEFENSE_ZONE_BEGIN;
	 m_bDEFENSE_ZONE_COMPLETE = rVALUE.m_bDEFENSE_ZONE_COMPLETE;
	 m_fDEFENSE_ZONE_TIMER = rVALUE.m_fDEFENSE_ZONE_TIMER;

	 m_bLevel = rVALUE.m_bLevel;

	 return *this;
}

BOOL GLQUESTPROG_STEP::SET_BYBUFFER ( CByteStream &ByteStream ) const
{
	ByteStream << (DWORD) VERSION;

	ByteStream << m_bTALK;
	ByteStream << m_bQITEM;

	ByteStream << m_bMOBKILL_ALL;
	ByteStream << m_dwNUM_MOBKILL;
	ByteStream << m_bNPCGUARD_DEATH;

	ByteStream << m_bREACH_ZONE;
	
	ByteStream << m_bDEFENSE_ZONE_BEGIN;
	ByteStream << m_bDEFENSE_ZONE_COMPLETE;
	ByteStream << m_fDEFENSE_ZONE_TIMER;

	ByteStream << m_bLevel;

	return TRUE;
}

BOOL GLQUESTPROG_STEP::GET_BYBUFFER ( CByteStream &ByteStream )
{
	DWORD dwVer(0);

	ByteStream >> dwVer;

	switch ( dwVer )
	{
	case 0x0001:
		ByteStream >> m_bTALK;
		ByteStream >> m_bQITEM;

		ByteStream >> m_bMOBKILL_ALL;
		ByteStream >> m_dwNUM_MOBKILL;
		ByteStream >> m_bNPCGUARD_DEATH;

		ByteStream >> m_bREACH_ZONE;
		
		ByteStream >> m_bDEFENSE_ZONE_BEGIN;
		ByteStream >> m_bDEFENSE_ZONE_COMPLETE;
		ByteStream >> m_fDEFENSE_ZONE_TIMER;
		break;

	case VERSION:
		ByteStream >> m_bTALK;
		ByteStream >> m_bQITEM;

		ByteStream >> m_bMOBKILL_ALL;
		ByteStream >> m_dwNUM_MOBKILL;
		ByteStream >> m_bNPCGUARD_DEATH;

		ByteStream >> m_bREACH_ZONE;
		
		ByteStream >> m_bDEFENSE_ZONE_BEGIN;
		ByteStream >> m_bDEFENSE_ZONE_COMPLETE;
		ByteStream >> m_fDEFENSE_ZONE_TIMER;

		ByteStream >> m_bLevel;
		break;
	};

	return TRUE;
}

bool GLQUESTPROG_STEP::BEGIN_PROG ( const GLQUEST_STEP &sQSTEP )
{
	m_bTALK = !sQSTEP.IsNEED_NPCTALK();
	
	m_bQITEM = !sQSTEP.IsNEED_QITEM();

	m_bMOBKILL_ALL = !sQSTEP.IsNEED_MOBKILL();
	m_dwNUM_MOBKILL = 0;

	m_bNPCGUARD_DEATH = false;

	m_bREACH_ZONE = !sQSTEP.IsNEED_REACHZONE();

	m_bDEFENSE_ZONE_BEGIN = !sQSTEP.IsNEED_DEFENSEZONE();
	m_bDEFENSE_ZONE_COMPLETE = !sQSTEP.IsNEED_DEFENSEZONE();
	m_fDEFENSE_ZONE_TIMER = 0.0f;

	m_bLevel = !sQSTEP.IsNEED_LEVEL();

	return true;
}

void GLQUESTPROG_STEP::RESET ()
{
	m_bTALK = (true);
	m_bQITEM = (true);

	m_bMOBKILL_ALL = (true);
	m_dwNUM_MOBKILL = (UINT_MAX);
	m_bNPCGUARD_DEATH = (false);

	m_bREACH_ZONE = (true);

	m_bDEFENSE_ZONE_BEGIN = (true);
	m_bDEFENSE_ZONE_COMPLETE = (true);
	m_fDEFENSE_ZONE_TIMER = (FLT_MAX);

	m_bLevel = true;
}

bool GLQUESTPROG_STEP::IsCOMPLETE ()
{
	if ( !m_bTALK )						return false;
	if ( !m_bQITEM )					return false;
	if ( !m_bMOBKILL_ALL )				return false;

	if ( m_bNPCGUARD_DEATH )			return false;

	if ( !m_bREACH_ZONE )				return false;
	
	if ( !m_bDEFENSE_ZONE_COMPLETE )	return false;

	if ( !m_bLevel )					return false;

	return true;
}

GLQUESTPROG& GLQUESTPROG::operator= ( const GLQUESTPROG &rVALUE )
{
	m_sNID = rVALUE.m_sNID;
	m_dwEDITVER = rVALUE.m_dwEDITVER;

	m_dwFlags = rVALUE.m_dwFlags;
	m_bCOMPLETE = rVALUE.m_bCOMPLETE;
	m_dwCOUNT = rVALUE.m_dwCOUNT;
	m_fLAPSTIME = rVALUE.m_fLAPSTIME;
	
	m_sINVENTORY.Assign ( rVALUE.m_sINVENTORY );

	m_dwSTEP = rVALUE.m_dwSTEP;

	m_sSTEP_PROG = rVALUE.m_sSTEP_PROG;

	m_tStartTime = rVALUE.m_tStartTime;

	/*dmk14 daily quest*/
	m_tEndTime = rVALUE.m_tEndTime;

	return *this;
}

BOOL GLQUESTPROG::SET_BYBUFFER ( CByteStream &ByteStream ) const
{
	ByteStream << (DWORD) VERSION;
	
	ByteStream << m_sNID.dwID;
	ByteStream << m_dwEDITVER;

	ByteStream << m_dwFlags;
	ByteStream << m_dwCOUNT;

	ByteStream << m_bCOMPLETE;
	ByteStream << m_fLAPSTIME;
	
	ByteStream << m_dwSTEP;
	ByteStream << m_tStartTime;

	/*dmk14 daily quest*/
	ByteStream << m_tEndTime;

	m_sSTEP_PROG.SET_BYBUFFER ( ByteStream );

	m_sINVENTORY.GETITEM_BYBUFFER ( ByteStream );

	return TRUE;
}

BOOL GLQUESTPROG::GET_BYBUFFER ( CByteStream &ByteStream )
{
	DWORD dwVer(0);
	ByteStream >> dwVer;

	switch ( dwVer )
	{
	case 0x0001:	LOAD_001( ByteStream ); break;
	case 0x0002:	LOAD_002( ByteStream ); break;
	case VERSION:	LOAD_003( ByteStream ); break;
	default:	break;
	}

	return TRUE;
}

BOOL GLQUESTPROG::LOAD_001 ( CByteStream &ByteStream )
{
	ByteStream >> m_sNID.dwID;
	ByteStream >> m_dwEDITVER;

	ByteStream >> m_dwFlags;
	ByteStream >> m_dwCOUNT;

	ByteStream >> m_bCOMPLETE;
	ByteStream >> m_fLAPSTIME;
	
	ByteStream >> m_dwSTEP;
	m_sSTEP_PROG.GET_BYBUFFER ( ByteStream );

	m_sINVENTORY.SETITEM_BYBUFFER ( ByteStream );

	return TRUE;
}

BOOL GLQUESTPROG::LOAD_002 ( CByteStream &ByteStream )
{
	ByteStream >> m_sNID.dwID;
	ByteStream >> m_dwEDITVER;

	ByteStream >> m_dwFlags;
	ByteStream >> m_dwCOUNT;

	ByteStream >> m_bCOMPLETE;
	ByteStream >> m_fLAPSTIME;
	
	ByteStream >> m_dwSTEP;
	
	ByteStream >> m_tStartTime;

	m_sSTEP_PROG.GET_BYBUFFER ( ByteStream );

	m_sINVENTORY.SETITEM_BYBUFFER ( ByteStream );

	return TRUE;
}

/*dmk14 daily quest*/
BOOL GLQUESTPROG::LOAD_003 ( CByteStream &ByteStream )
{
	ByteStream >> m_sNID.dwID;
	ByteStream >> m_dwEDITVER;

	ByteStream >> m_dwFlags;
	ByteStream >> m_dwCOUNT;

	ByteStream >> m_bCOMPLETE;
	ByteStream >> m_fLAPSTIME;
	
	ByteStream >> m_dwSTEP;
	
	ByteStream >> m_tStartTime;

	/*dmk14 daily quest*/
	ByteStream >> m_tEndTime;

	m_sSTEP_PROG.GET_BYBUFFER ( ByteStream );

	m_sINVENTORY.SETITEM_BYBUFFER ( ByteStream );

	return TRUE;
}

bool GLQUESTPROG::START ( GLQUEST* pQUEST, GLQUESTPROG *pOLD_PROG )
{
	GASSERT(pQUEST&&"GLQUESTPROG::START");
	if ( !pQUEST )		return false;

	//	이전 시도 횟수를 가져온다.
	DWORD dwOLD_COUNT(0);
	if ( pOLD_PROG )	dwOLD_COUNT = pOLD_PROG->m_dwCOUNT;

	//	퀘스트 기본 정보 설정.
	m_sNID = pQUEST->m_sNID;
	m_dwEDITVER = pQUEST->m_dwEDITVER;

	//	퀘스트 정보 읽기를 요청.
	ReqREADING ();

	//	진행 시도 횟수 설정.
	m_dwCOUNT = dwOLD_COUNT + 1;

	m_bCOMPLETE = false;
	m_fLAPSTIME = 0.0f;

	// 퀘스트 시작시간 설정
	CTime cTime = CTime::GetCurrentTime();
	m_tStartTime = cTime.GetTime();

	//	현제 진행 퀘스트 STEP.
	m_dwSTEP = 0;

	//	퀘스트 초기 스텝 설정.
	GLQUEST_STEP *pQSTEP = pQUEST->GetSTEP ( m_dwSTEP );
	GASSERT(pQSTEP&&"pQUEST->GetSTEP");
	if ( !pQSTEP )		return false;

	//	현제 진행 정보.
	bool bOK = m_sSTEP_PROG.BEGIN_PROG ( *pQSTEP );
	if ( !bOK )			return false;

	//	인벤토리 리셋.
	m_sINVENTORY.DeleteItemAll ();

	return true;
}

bool GLQUESTPROG::DoPROGRESS ( bool &_bModifyInven, GLQUEST_PROGRESS_MAP &stepMoveMap )
{
	GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( m_sNID.dwID );
	
	_bModifyInven = false;

	DWORD dwNum = pQUEST->GetSTEPNUM();

	//	현제 진행 부분이 마지막 step일때.
	if ( m_dwSTEP==(dwNum-1) )	return false;

	if ( m_sSTEP_PROG.IsCOMPLETE() )
	{
		GLQUEST_STEP* pSTEP = pQUEST->GetSTEP ( m_dwSTEP );
		GASSERT(pSTEP&&"pQUEST->GetSTEP");
		if ( !pSTEP )			return false;

		//	퀘스트 인벤토리 정리.
		//
		{
			DWQARRAY_ITER iter = pSTEP->m_vecRESET_QITEM.begin();
			DWQARRAY_ITER iter_end = pSTEP->m_vecRESET_QITEM.end();
			for ( ; iter!=iter_end; ++iter )
			{
				_bModifyInven = true;

				SNATIVEID sNID = SNATIVEID(*iter);
				SINVENITEM* pINVEN = m_sINVENTORY.FindItem(sNID);
				if ( !pINVEN )		continue;
				m_sINVENTORY.DeleteItem ( pINVEN->wPosX, pINVEN->wPosY );
			}
		}

		//	퀘스트 인벤토리에 아이템 삽입.
		//
		{
			INVENQARRAY_ITER iter = pSTEP->m_vecGIFT_QITEM.begin();
			INVENQARRAY_ITER iter_end = pSTEP->m_vecGIFT_QITEM.end();
			for ( ; iter!=iter_end; ++iter )
			{
				_bModifyInven = true;

				SITEMCUSTOM &sCUSTOM = (*iter);
				m_sINVENTORY.InsertItem ( sCUSTOM );
			}
		}

		if( pSTEP->m_stepMoveMap.nidMAP.wMainID  != UINT_MAX &&
			pSTEP->m_stepMoveMap.nidMAP.wSubID   != UINT_MAX &&
			(pSTEP->m_stepMoveMap.wPosX != UINT_MAX &&
			 pSTEP->m_stepMoveMap.wPosY != UINT_MAX ||
			 pSTEP->m_stepMoveMap.dwGateID != 0 ) )
		{
			stepMoveMap = pSTEP->m_stepMoveMap;
		}

		m_dwSTEP++;
		pSTEP = pQUEST->GetSTEP ( m_dwSTEP );
		GASSERT(pSTEP&&"pQUEST->GetSTEP");
		if ( !pSTEP )			return false;

		m_sSTEP_PROG.BEGIN_PROG ( *pSTEP );

		//	퀘스트 정보 읽기를 요청.
		ReqREADING ();

		return true;
	}

	return false;
}

bool GLQUESTPROG::DoCOMPLETE ()
{
	if ( CheckCOMPLETE() )
	{
		m_dwFlags = NULL;

		//	퀘스트 정보 읽기를 요청.
		ReqREADING ();

		m_bCOMPLETE = true;
		m_fLAPSTIME = 0.0f;

		m_dwSTEP = 0;
		m_sSTEP_PROG.RESET();

		m_sINVENTORY.DeleteItemAll();

		/*dmk14 daily quest*/
		CTime cTime = CTime::GetCurrentTime();
		m_tEndTime = cTime.GetTime();

		return true;
	}

	return false;
}

bool GLQUESTPROG::CheckCOMPLETE ()
{
	GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( m_sNID.dwID );
	if ( !pQUEST )							return false;
	
	DWORD dwNum = pQUEST->GetSTEPNUM();

	if ( m_dwSTEP==(dwNum-1) )
	{
		return m_sSTEP_PROG.IsCOMPLETE ();
	}

	return false;
}

void GLQUESTPROG::DoGIVEUP ()
{
	m_bCOMPLETE = false;
	m_dwSTEP = 0;
	m_sSTEP_PROG.RESET();

	m_dwFlags = NULL;
	m_fLAPSTIME = 0.0f;
	m_sINVENTORY.DeleteItemAll();
}

bool GLQUESTPROG::CheckTIMEOVER ()
{
	GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( m_sNID.dwID );
	if ( !pQUEST )							return false;

	if ( pQUEST->m_dwLimitTime==0 )			return false;

	return ( pQUEST->m_dwLimitTime*60 <= m_fLAPSTIME );
}

bool GLQUESTPROG::IsNonDIE ()
{
	GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( m_sNID.dwID );
	if ( !pQUEST )							return false;

	return pQUEST->IsNonDIE();
}

HRESULT GLQUESTPROG::FrameMove ( float fTime, float fElapsedTime )
{
	
	GLQUEST* pQUEST = GLQuestMan::GetInstance().Find ( m_sNID.dwID );
	if ( !pQUEST )							return E_FAIL;

	if ( pQUEST->m_dwLimitTime>0 )
	{
		// 지나간시간 = 현재시간 - 시작시간
		CTime cTime = CTime::GetCurrentTime();
		__time64_t	tCurTime = cTime.GetTime();
		m_fLAPSTIME = (float) tCurTime - m_tStartTime;

		if ( m_fLAPSTIME < 0.0f ) m_fLAPSTIME = 0.0f;

		//	초과되었을 경우.
		if ( m_fLAPSTIME >= pQUEST->m_dwLimitTime*60 )
			m_fLAPSTIME = float(pQUEST->m_dwLimitTime*60);
	}

	return S_OK;
}

namespace COMMENT
{
	std::string QUEST_PROGRESS[EMQP_SIZE] =
	{
		"NoEvent",
		"CharDeath",
		"MovetoStart"
		"MovetoSpecifiedPlace"
	};

	std::string QUEST_AREA[EMQUEST_AREA_SIZE] =
	{
		"SG MP PH Campus",
		"SG MP PH GateHole",
		"Dormitory",
		"Tutorial",
		"Root Hole",
		"Head.B Area",
		"PhoenixHole",
		"PhoenixCampus",
		"TradingHole",
		"Trading4Passage",
		"SacredGateHole",
		"SG_Campus",
		"Prison",
		"Saint Research Complex",
		"Complex Basement",
		"PracticingYard",
		"Middle Hole",
		"WharfPassage",
		"MarketPlace",
		"MysticPeakHole",
		"MP_Campus",
		"LeonineCampus1F",
		"LeonineCampus",
		"Map Entry Quest",
		"Event Quest",
		"Regional Quest",
		"Level Achieving Quest",
		"Disabled Quest",
		"Saint power plant",
		"Lost City-Chaos Island",
	};
};
