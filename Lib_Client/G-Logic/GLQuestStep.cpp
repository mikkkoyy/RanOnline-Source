#include "stdafx.h"
#include "GLQuestStep.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


GLQUEST_STEP& GLQUEST_STEP::operator= ( const GLQUEST_STEP &rVALUE )
{
	m_strTITLE = rVALUE.m_strTITLE;
	m_strCOMMENT = rVALUE.m_strCOMMENT;

	m_strOBJ_NPCTALK = rVALUE.m_strOBJ_NPCTALK;
	m_dwNID_NPCTALK = rVALUE.m_dwNID_NPCTALK;

	m_strOBJ_MOBGEN_QITEM = rVALUE.m_strOBJ_MOBGEN_QITEM;
	m_vecMOBGEN_QITEM = rVALUE.m_vecMOBGEN_QITEM;

	m_strOBJ_MOBKILL = rVALUE.m_strOBJ_MOBKILL;
	m_dwNID_MOBKILL = rVALUE.m_dwNID_MOBKILL;
	m_dwNUM_MOBKILL = rVALUE.m_dwNUM_MOBKILL;

	m_strOBJ_NPCGUARD = rVALUE.m_strOBJ_NPCGUARD;
	m_dwNID_NPCGUARD = rVALUE.m_dwNID_NPCGUARD;

	m_strOBJ_REACH_ZONE = rVALUE.m_strOBJ_REACH_ZONE;
	m_sMAPID_REACH = rVALUE.m_sMAPID_REACH;
	m_wPOSX_REACH = rVALUE.m_wPOSX_REACH;
	m_wPOSY_REACH = rVALUE.m_wPOSY_REACH;
	m_wRADIUS_REACH = rVALUE.m_wRADIUS_REACH;

	m_strOBJ_DEFENSE_ZONE = rVALUE.m_strOBJ_DEFENSE_ZONE;
	m_sMAPID_DEFENSE = rVALUE.m_sMAPID_DEFENSE;
	m_wPOSX_DEFENSE = rVALUE.m_wPOSX_DEFENSE;
	m_wPOSY_DEFENSE = rVALUE.m_wPOSY_DEFENSE;
	m_wRADIUS_DEFENSE = rVALUE.m_wRADIUS_DEFENSE;
	m_fDEFENSE_TIME = rVALUE.m_fDEFENSE_TIME;

	m_vecRESET_QITEM = rVALUE.m_vecRESET_QITEM;
	m_vecGIFT_QITEM = rVALUE.m_vecGIFT_QITEM;
	m_wLevel = rVALUE.m_wLevel;

	return *this;
}

bool GLQUEST_STEP::LOAD_0001 ( basestream &SFile )
{
	DWORD dwSize(0);

	typedef std::vector<SITEMCUSTOM_103>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> m_strOBJ_NPCTALK;
	SFile >> m_dwNID_NPCTALK;

	SFile >> m_strOBJ_MOBGEN_QITEM;

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}

	SFile >> m_strOBJ_MOBKILL;
	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;

	SFile >> m_strOBJ_NPCGUARD;
	SFile >> m_dwNID_NPCGUARD;

	SFile >> m_strOBJ_REACH_ZONE;
	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;

	SFile >> m_strOBJ_DEFENSE_ZONE;
	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;

	SFile >> m_vecRESET_QITEM;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;

	m_vecGIFT_QITEM.clear();
	m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGIFT_QITEM.push_back ( sCUSTOM );
	}

	return true;
}

bool GLQUEST_STEP::LOAD_0002 ( basestream &SFile )
{
	DWORD dwSize(0);

	typedef std::vector<SITEMCUSTOM_104>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> m_strOBJ_NPCTALK;
	SFile >> m_dwNID_NPCTALK;

	SFile >> m_strOBJ_MOBGEN_QITEM;

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}

	SFile >> m_strOBJ_MOBKILL;
	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;

	SFile >> m_strOBJ_NPCGUARD;
	SFile >> m_dwNID_NPCGUARD;

	SFile >> m_strOBJ_REACH_ZONE;
	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;

	SFile >> m_strOBJ_DEFENSE_ZONE;
	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;

	SFile >> m_vecRESET_QITEM;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;

	m_vecGIFT_QITEM.clear();
	m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGIFT_QITEM.push_back ( sCUSTOM );
	}

	return true;
}

bool GLQUEST_STEP::LOAD_0003 ( basestream &SFile )
{
	DWORD dwSize(0);

	typedef std::vector<SITEMCUSTOM_105>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> m_strOBJ_NPCTALK;
	SFile >> m_dwNID_NPCTALK;

	SFile >> m_strOBJ_MOBGEN_QITEM;

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}

	SFile >> m_strOBJ_MOBKILL;
	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;

	SFile >> m_strOBJ_NPCGUARD;
	SFile >> m_dwNID_NPCGUARD;

	SFile >> m_strOBJ_REACH_ZONE;
	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;

	SFile >> m_strOBJ_DEFENSE_ZONE;
	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;

	SFile >> m_vecRESET_QITEM;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;

	m_vecGIFT_QITEM.clear();
	m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGIFT_QITEM.push_back ( sCUSTOM );
	}

	return true;
}

bool GLQUEST_STEP::LOAD_0004 ( basestream &SFile )
{
	DWORD dwSize(0);
	typedef std::vector<SITEMCUSTOM_106>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> m_strOBJ_NPCTALK;
	SFile >> m_dwNID_NPCTALK;

	SFile >> m_strOBJ_MOBGEN_QITEM;

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}

	SFile >> m_strOBJ_MOBKILL;
	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;

	SFile >> m_strOBJ_NPCGUARD;
	SFile >> m_dwNID_NPCGUARD;

	SFile >> m_strOBJ_REACH_ZONE;
	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;

	SFile >> m_strOBJ_DEFENSE_ZONE;
	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;

	SFile >> m_vecRESET_QITEM;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;

	m_vecGIFT_QITEM.clear();
	m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGIFT_QITEM.push_back ( sCUSTOM );
	}

	return true;
}

bool GLQUEST_STEP::LOAD_0005( basestream &SFile )
{
	DWORD dwSize(0);
	typedef std::vector<SITEMCUSTOM_106>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> m_strOBJ_NPCTALK;
	SFile >> m_dwNID_NPCTALK;

	SFile >> m_strOBJ_MOBGEN_QITEM;

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}

	SFile >> m_strOBJ_MOBKILL;
	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;

	SFile >> m_strOBJ_NPCGUARD;
	SFile >> m_dwNID_NPCGUARD;

	SFile >> m_strOBJ_REACH_ZONE;
	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;

	SFile >> m_strOBJ_DEFENSE_ZONE;
	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;

	SFile >> m_wLevel;

	SFile >> m_vecRESET_QITEM;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;

	m_vecGIFT_QITEM.clear();
	m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGIFT_QITEM.push_back ( sCUSTOM );
	}

	return true;
}

bool GLQUEST_STEP::LOAD_0006( basestream &SFile )
{
	DWORD dwSize(0);
	typedef std::vector<SITEMCUSTOM_108>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> m_strOBJ_NPCTALK;
	SFile >> m_dwNID_NPCTALK;

	SFile >> m_strOBJ_MOBGEN_QITEM;

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}

	SFile >> m_strOBJ_MOBKILL;
	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;

	SFile >> m_strOBJ_NPCGUARD;
	SFile >> m_dwNID_NPCGUARD;

	SFile >> m_strOBJ_REACH_ZONE;
	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;

	SFile >> m_strOBJ_DEFENSE_ZONE;
	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;

	SFile >> m_wLevel;

	SFile >> m_vecRESET_QITEM;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;

	m_vecGIFT_QITEM.clear();
	m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGIFT_QITEM.push_back ( sCUSTOM );
	}

	return true;
}

bool GLQUEST_STEP::LOAD_0007( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_109>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> m_strOBJ_NPCTALK;
	SFile >> m_dwNID_NPCTALK;

	SFile >> m_strOBJ_MOBGEN_QITEM;

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}

	SFile >> m_strOBJ_MOBKILL;
	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;

	SFile >> m_strOBJ_NPCGUARD;
	SFile >> m_dwNID_NPCGUARD;

	SFile >> m_strOBJ_REACH_ZONE;
	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;

	SFile >> m_strOBJ_DEFENSE_ZONE;
	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;

	SFile >> m_wLevel;

	SFile >> m_vecRESET_QITEM;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;

	m_vecGIFT_QITEM.clear();
	m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGIFT_QITEM.push_back ( sCUSTOM );
	}

	return true;
}

bool GLQUEST_STEP::LOAD_0008( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_110>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> m_strOBJ_NPCTALK;
	SFile >> m_dwNID_NPCTALK;

	SFile >> m_strOBJ_MOBGEN_QITEM;

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}

	SFile >> m_strOBJ_MOBKILL;
	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;

	SFile >> m_strOBJ_NPCGUARD;
	SFile >> m_dwNID_NPCGUARD;

	SFile >> m_strOBJ_REACH_ZONE;
	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;

	SFile >> m_strOBJ_DEFENSE_ZONE;
	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;

	SFile >> m_wLevel;

	SFile >> m_vecRESET_QITEM;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;

	m_vecGIFT_QITEM.clear();
	m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGIFT_QITEM.push_back ( sCUSTOM );
	}

	return true;
}

bool GLQUEST_STEP::LOAD_0009( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_111>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> m_strOBJ_NPCTALK;
	SFile >> m_dwNID_NPCTALK;

	SFile >> m_strOBJ_MOBGEN_QITEM;

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}

	SFile >> m_strOBJ_MOBKILL;
	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;

	SFile >> m_strOBJ_NPCGUARD;
	SFile >> m_dwNID_NPCGUARD;

	SFile >> m_strOBJ_REACH_ZONE;
	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;

	SFile >> m_strOBJ_DEFENSE_ZONE;
	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;

	SFile >> m_wLevel;

	SFile >> m_vecRESET_QITEM;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;

	m_vecGIFT_QITEM.clear();
	m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGIFT_QITEM.push_back ( sCUSTOM );
	}

	return true;
}

bool GLQUEST_STEP::LOAD_0010( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_112>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> m_strOBJ_NPCTALK;
	SFile >> m_dwNID_NPCTALK;

	SFile >> m_strOBJ_MOBGEN_QITEM;

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}

	SFile >> m_strOBJ_MOBKILL;
	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;

	SFile >> m_strOBJ_NPCGUARD;
	SFile >> m_dwNID_NPCGUARD;

	SFile >> m_strOBJ_REACH_ZONE;
	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;

	SFile >> m_strOBJ_DEFENSE_ZONE;
	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;

	SFile >> m_wLevel;

	SFile >> m_vecRESET_QITEM;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;

	m_vecGIFT_QITEM.clear();
	m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGIFT_QITEM.push_back ( sCUSTOM );
	}
	//SFile >> m_vecGIFT_QITEM;

	return true;
}


bool GLQUEST_STEP::LOAD_0011( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_112>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> m_strOBJ_NPCTALK;
	SFile >> m_dwNID_NPCTALK;

	SFile >> m_strOBJ_MOBGEN_QITEM;

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}

	SFile >> m_strOBJ_MOBKILL;
	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;

	SFile >> m_strOBJ_NPCGUARD;
	SFile >> m_dwNID_NPCGUARD;

	SFile >> m_strOBJ_REACH_ZONE;
	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;

	SFile >> m_strOBJ_DEFENSE_ZONE;
	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;

	SFile >> m_wLevel;

	SFile >> m_vecRESET_QITEM;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;

	m_vecGIFT_QITEM.clear();
	m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGIFT_QITEM.push_back ( sCUSTOM );
	}

	//SFile >> m_vecGIFT_QITEM;

	SFile >> m_stepMoveMap.nidMAP.dwID;
	SFile >> m_stepMoveMap.wPosX;
	SFile >> m_stepMoveMap.wPosY;
	SFile >> m_stepMoveMap.dwGateID;

	return true;
}

bool GLQUEST_STEP::LOAD_0012( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_112>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;
	SFile >> m_strOBJ_MOBKILL;

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}

	SFile >> m_strOBJ_MOBGEN_QITEM;

	SFile >> m_dwNID_NPCTALK;
	SFile >> m_strOBJ_NPCTALK;

	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;
	SFile >> m_strOBJ_REACH_ZONE;

	SFile >> m_wLevel;

	SFile >> m_dwNID_NPCGUARD;
	SFile >> m_strOBJ_NPCGUARD;

	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;
	SFile >> m_strOBJ_DEFENSE_ZONE;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;

	m_vecGIFT_QITEM.clear();
	m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGIFT_QITEM.push_back ( sCUSTOM );
	}
	//SFile >> m_vecGIFT_QITEM;

	SFile >> m_vecRESET_QITEM;

	SFile >> m_stepMoveMap.nidMAP.dwID;
	SFile >> m_stepMoveMap.wPosX;
	SFile >> m_stepMoveMap.wPosY;
	SFile >> m_stepMoveMap.dwGateID;

	return true;
}

bool GLQUEST_STEP::LOAD_0013( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_QUEST_115>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;
	SFile >> m_strOBJ_MOBKILL;

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}

	SFile >> m_strOBJ_MOBGEN_QITEM;

	SFile >> m_dwNID_NPCTALK;
	SFile >> m_strOBJ_NPCTALK;

	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;
	SFile >> m_strOBJ_REACH_ZONE;

	SFile >> m_wLevel;

	SFile >> m_dwNID_NPCGUARD;
	SFile >> m_strOBJ_NPCGUARD;

	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;
	SFile >> m_strOBJ_DEFENSE_ZONE;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;
	m_vecGIFT_QITEM.clear();
	m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );
	INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGIFT_QITEM.push_back ( sCUSTOM );
	}
	//SFile >> m_vecGIFT_QITEM;


	SFile >> m_vecRESET_QITEM;

	SFile >> m_stepMoveMap.nidMAP.dwID;
	SFile >> m_stepMoveMap.wPosX;
	SFile >> m_stepMoveMap.wPosY;
	SFile >> m_stepMoveMap.dwGateID;

	return true;
}

/*dmk14 ver15 quest loader*/
bool GLQUEST_STEP::LOAD_0015( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_QUEST_116>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;
	SFile >> m_strOBJ_MOBKILL;

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}

	SFile >> m_strOBJ_MOBGEN_QITEM;

	SFile >> m_dwNID_NPCTALK;

	SFile >> m_strOBJ_NPCTALK;

	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;
	SFile >> m_strOBJ_REACH_ZONE;

	SFile >> m_wLevel;

	SFile >> m_dwNID_NPCGUARD;
	SFile >> m_strOBJ_NPCGUARD;

	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;
	SFile >> m_strOBJ_DEFENSE_ZONE;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;
	m_vecGIFT_QITEM.clear();
	m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );
	INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGIFT_QITEM.push_back ( sCUSTOM );
	}
	//SFile >> m_vecGIFT_QITEM;
	SFile >> m_vecRESET_QITEM;

	SFile >> m_stepMoveMap.nidMAP.dwID;
	SFile >> m_stepMoveMap.wPosX;
	SFile >> m_stepMoveMap.wPosY;
	SFile >> m_stepMoveMap.dwGateID;

	return true;
}

/*dmk14 ver18 quest loader*/
bool GLQUEST_STEP::LOAD_0018( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_QUEST_116>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	std::string strDummy;
	DWORD		dwDummy;
	bool		bDummy;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> dwDummy;	//		SFile >> m_dwNUM_LEVELUP;	//new
	SFile >> strDummy;	//		SFile >> m_strOBJ_LEVELUP;	//new

	SFile >> dwDummy;	//		SFile >> m_dwType_CONFTING;	//new
	SFile >> bDummy;	//		SFile >> m_bSCHOOL;			//new
	SFile >> dwDummy;	//		SFile >> m_dwNUM_CONFTING;	//new
	SFile >> strDummy;	//		SFile >> m_strOBJ_CONFTING;	//new

	SFile >> dwDummy;	//		SFile >> m_dwNID_ITEMGRINDING;	//new
	SFile >> dwDummy;	//		SFile >> m_dwNUM_ITEMGRINDING;	//new
	SFile >> dwDummy;	//		SFile >> m_dwItemType;			//new
	SFile >> bDummy;	//		SFile >> m_bConsecution;		//new
	SFile >> strDummy;	//		SFile >> m_strOBJ_ITEMGRINDING;	//new

	SFile >> dwDummy;	//		SFile >> m_dwNID_ITEMUSE;		//new
	SFile >> dwDummy;	//		SFile >> m_dwNUM_ITEMUSE;		//new
	SFile >> strDummy;	//		SFile >> m_strOBJ_ITEMUSE;		//new

	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;
	SFile >> m_strOBJ_MOBKILL;
	SFile >> bDummy;	//		SFile >> m_bIsPLAYERKILL;	//new

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}
	SFile >> m_strOBJ_MOBGEN_QITEM;

	SFile >> m_dwNID_NPCTALK;
	SFile >> m_strOBJ_NPCTALK;

	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;
	SFile >> m_strOBJ_REACH_ZONE;

	SFile >> m_wLevel;	

	SFile >> m_dwNID_NPCGUARD;
	SFile >> m_strOBJ_NPCGUARD;

	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;
	SFile >> m_strOBJ_DEFENSE_ZONE;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;

	m_vecGIFT_QITEM.clear();
	m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGIFT_QITEM.push_back ( sCUSTOM );
	}
	//SFile >> m_vecGIFT_QITEM;
	SFile >> m_vecRESET_QITEM;

	SFile >> m_stepMoveMap.nidMAP.dwID;
	SFile >> m_stepMoveMap.wPosX;
	SFile >> m_stepMoveMap.wPosY;
	SFile >> m_stepMoveMap.dwGateID;

	return true;
}

/*dmk14 ver18 quest loader*/
bool GLQUEST_STEP::LOAD_0021( basestream &SFile )
{
		DWORD dwSize(0);

	std::string strDummy;
	DWORD		dwDummy;
	bool		bDummy;

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> dwDummy;	//		SFile >> m_dwNUM_LEVELUP;	//new
	SFile >> strDummy;	//		SFile >> m_strOBJ_LEVELUP;	//new

	SFile >> dwDummy;	//		SFile >> m_dwType_CONFTING;	//new
	SFile >> bDummy;	//		SFile >> m_bSCHOOL;			//new
	SFile >> dwDummy;	//		SFile >> m_dwNUM_CONFTING;	//new
	SFile >> strDummy;	//		SFile >> m_strOBJ_CONFTING;	//new

	SFile >> dwDummy;	//		SFile >> m_dwNID_ITEMGRINDING;	//new
	SFile >> dwDummy;	//		SFile >> m_dwNUM_ITEMGRINDING;	//new
	SFile >> dwDummy;	//		SFile >> m_dwItemType;			//new
	SFile >> bDummy;	//		SFile >> m_bConsecution;		//new
	SFile >> strDummy;	//		SFile >> m_strOBJ_ITEMGRINDING;	//new

	SFile >> dwDummy;	//		SFile >> m_dwNID_ITEMUSE;		//new
	SFile >> dwDummy;	//		SFile >> m_dwNUM_ITEMUSE;		//new
	SFile >> strDummy;	//		SFile >> m_strOBJ_ITEMUSE;		//new

	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;
	SFile >> m_strOBJ_MOBKILL;
	SFile >> bDummy;	//		SFile >> m_bIsPLAYERKILL;	//new

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}
	SFile >> m_strOBJ_MOBGEN_QITEM;

	SFile >> m_dwNID_NPCTALK;
	SFile >> m_strOBJ_NPCTALK;

	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;
	SFile >> m_strOBJ_REACH_ZONE;

	SFile >> m_wLevel;	

	SFile >> m_dwNID_NPCGUARD;
	SFile >> m_strOBJ_NPCGUARD;

	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;
	SFile >> m_strOBJ_DEFENSE_ZONE;

	// 아이템 정보가 바뀌면 작업을 해야된다.
	typedef std::vector< SITEMCUSTOM_119 > INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator INVENQARRAY_OLD_ITER;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;
	{
		m_vecGIFT_QITEM.clear();
		m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );

		INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
		INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
		for ( ; iter!=iter_end; ++iter )
		{
			SITEMCUSTOM sCUSTOM;
			sCUSTOM.Assign( ( *iter ) );
			m_vecGIFT_QITEM.push_back ( sCUSTOM );
		}
	}

	SFile >> m_vecRESET_QITEM;

	SFile >> m_stepMoveMap.nidMAP.dwID;
	SFile >> m_stepMoveMap.wPosX;
	SFile >> m_stepMoveMap.wPosY;
	SFile >> m_stepMoveMap.dwGateID;

	return true;
}

bool GLQUEST_STEP::LOAD_0200( basestream &SFile )
{
	typedef std::vector<SITEMCUSTOM_112>	INVENQARRAY_OLD;
	typedef INVENQARRAY_OLD::iterator		INVENQARRAY_OLD_ITER;

	DWORD dwSize(0);

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;
	SFile >> m_strOBJ_MOBKILL;

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}

	SFile >> m_strOBJ_MOBGEN_QITEM;

	SFile >> m_dwNID_NPCTALK;
	SFile >> m_strOBJ_NPCTALK;

	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;
	SFile >> m_strOBJ_REACH_ZONE;

	SFile >> m_wLevel;

	SFile >> m_dwNID_NPCGUARD;
	SFile >> m_strOBJ_NPCGUARD;

	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;
	SFile >> m_strOBJ_DEFENSE_ZONE;

	INVENQARRAY_OLD vecGIFT_QITEM;
	SFile >> vecGIFT_QITEM;

	m_vecGIFT_QITEM.clear();
	m_vecGIFT_QITEM.reserve ( vecGIFT_QITEM.size() );

	INVENQARRAY_OLD_ITER iter = vecGIFT_QITEM.begin();
	INVENQARRAY_OLD_ITER iter_end = vecGIFT_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.Assign ( (*iter) );
		m_vecGIFT_QITEM.push_back ( sCUSTOM );
	}

	//SFile >> m_vecGIFT_QITEM;

	SFile >> m_vecRESET_QITEM;

	SFile >> m_stepMoveMap.nidMAP.dwID;
	SFile >> m_stepMoveMap.wPosX;
	SFile >> m_stepMoveMap.wPosY;
	SFile >> m_stepMoveMap.dwGateID;

	return true;
}

bool GLQUEST_STEP::LOAD_0201( basestream &SFile )
{
	DWORD dwSize(0);

	SFile >> m_strTITLE;
	SFile >> m_strCOMMENT;

	SFile >> m_dwNID_MOBKILL;
	SFile >> m_dwNUM_MOBKILL;
	SFile >> m_strOBJ_MOBKILL;

	DWORD dwVerMOBGENQITEM(0);
	SFile >> dwVerMOBGENQITEM;

	SFile >> dwSize;
	m_vecMOBGEN_QITEM.reserve (dwSize);
	for ( DWORD i=0; i<dwSize; ++i )
	{
		SGENQUESTITEM sGENQ;
		sGENQ.LOAD ( SFile );
		m_vecMOBGEN_QITEM.push_back ( sGENQ );
	}

	SFile >> m_strOBJ_MOBGEN_QITEM;

	SFile >> m_dwNID_NPCTALK;
	SFile >> m_strOBJ_NPCTALK;

	SFile >> m_sMAPID_REACH.dwID;
	SFile >> m_wPOSX_REACH;
	SFile >> m_wPOSY_REACH;
	SFile >> m_wRADIUS_REACH;
	SFile >> m_strOBJ_REACH_ZONE;

	SFile >> m_wLevel;

	SFile >> m_dwNID_NPCGUARD;
	SFile >> m_strOBJ_NPCGUARD;

	SFile >> m_sMAPID_DEFENSE.dwID;
	SFile >> m_wPOSX_DEFENSE;
	SFile >> m_wPOSY_DEFENSE;
	SFile >> m_wRADIUS_DEFENSE;
	SFile >> m_fDEFENSE_TIME;
	SFile >> m_strOBJ_DEFENSE_ZONE;

	SFile >> m_vecGIFT_QITEM;
	SFile >> m_vecRESET_QITEM;

	SFile >> m_stepMoveMap.nidMAP.dwID;
	SFile >> m_stepMoveMap.wPosX;
	SFile >> m_stepMoveMap.wPosY;
	SFile >> m_stepMoveMap.dwGateID;

	return true;
}

bool GLQUEST_STEP::LOAD ( basestream &SFile )
{
	DWORD dwVer(0);
	DWORD dwSize(0);

	SFile >> dwVer;

	switch ( dwVer )
	{
	case 0x0001:	LOAD_0001 ( SFile ); break;
	case 0x0002:	LOAD_0002 ( SFile ); break;
	case 0x0003:	LOAD_0003 ( SFile ); break;
	case 0x0004:	LOAD_0004 ( SFile ); break;
	case 0x0005:	LOAD_0005 ( SFile ); break;
	case 0x0006:	LOAD_0006 ( SFile ); break;
	case 0x0007:	LOAD_0007 ( SFile ); break;
	case 0x0008:	LOAD_0008 ( SFile ); break;
	case 0x0009:	LOAD_0009 ( SFile ); break;
	case 0x0010:	LOAD_0010 ( SFile ); break;
	case 0x0011:	LOAD_0011 ( SFile ); break;
	case 0x0012:	LOAD_0012 ( SFile ); break;
	case 0x0013:	LOAD_0013 ( SFile ); break;
	case 0x0015:	LOAD_0015 ( SFile ); break;	/*dmk14 ver15 quest loader*/
	case 0x0018:	LOAD_0018 ( SFile ); break;	/*dmk14 ver18 quest loader*/
	case 0x0021:	LOAD_0021 ( SFile ); break;	/*dmk14 ver21 quest loader*/
	case 0x0200:	LOAD_0200 ( SFile ); break;
	case VERSION:	LOAD_0201 ( SFile ); break;

	default:
		{
			CDebugSet::ErrorVersion( "GLQUEST_STEP::LOAD", dwVer );
			break;
		}
	};

	return true;
}

bool GLQUEST_STEP::SAVE ( CSerialFile &SFile )
{
	SFile << (DWORD) VERSION;

	SFile << m_strTITLE;
	SFile << m_strCOMMENT;

	SFile << m_dwNID_MOBKILL;
	SFile << m_dwNUM_MOBKILL;
	SFile << m_strOBJ_MOBKILL;

	SFile << (DWORD)SGENQUESTITEM::VERSION;
	SFile << (DWORD)m_vecMOBGEN_QITEM.size();
	GENMOBITEMARRAY_ITER iter = m_vecMOBGEN_QITEM.begin();
	GENMOBITEMARRAY_ITER iter_end = m_vecMOBGEN_QITEM.end();
	for ( ; iter!=iter_end; ++iter )
	{
		(*iter).SAVE ( SFile );
	}

	SFile << m_strOBJ_MOBGEN_QITEM;

	SFile << m_dwNID_NPCTALK;
	SFile << m_strOBJ_NPCTALK;

	SFile << m_sMAPID_REACH.dwID;
	SFile << m_wPOSX_REACH;
	SFile << m_wPOSY_REACH;
	SFile << m_wRADIUS_REACH;
	SFile << m_strOBJ_REACH_ZONE;

	SFile << m_wLevel;

	SFile << m_dwNID_NPCGUARD;
	SFile << m_strOBJ_NPCGUARD;

	SFile << m_sMAPID_DEFENSE.dwID;
	SFile << m_wPOSX_DEFENSE;
	SFile << m_wPOSY_DEFENSE;
	SFile << m_wRADIUS_DEFENSE;
	SFile << m_fDEFENSE_TIME;
	SFile << m_strOBJ_DEFENSE_ZONE;

	SFile << m_vecGIFT_QITEM;
	SFile << m_vecRESET_QITEM;

	SFile << m_stepMoveMap.nidMAP.dwID;
	SFile << m_stepMoveMap.wPosX;
	SFile << m_stepMoveMap.wPosY;
	SFile << m_stepMoveMap.dwGateID;

	return true;
}

void GLQUEST_STEP::RESETITEM_ERASE ( DWORD dwIndex )
{
	GASSERT ( m_vecRESET_QITEM.size() > dwIndex && "GLQUEST_STEP::RESETITEM_ERASE()" );
	if ( m_vecRESET_QITEM.size() <= dwIndex )	return;

	DWQARRAY_ITER iter = m_vecRESET_QITEM.begin()+dwIndex;
	m_vecRESET_QITEM.erase ( iter, iter+1 );
}

void GLQUEST_STEP::MOBGENITEM_ERASE ( DWORD dwIndex )
{
	GASSERT ( m_vecMOBGEN_QITEM.size() > dwIndex && "GLQUEST_STEP::MOBGENITEM_ERASE()" );
	if ( m_vecMOBGEN_QITEM.size() <= dwIndex )	return;

	GENMOBITEMARRAY_ITER iter = m_vecMOBGEN_QITEM.begin()+dwIndex;
	m_vecMOBGEN_QITEM.erase ( iter, iter+1 );
}

void GLQUEST_STEP::GIFTITEM_ERASE ( DWORD dwIndex )
{
	GASSERT ( m_vecGIFT_QITEM.size() > dwIndex && "GLQUEST_STEP::GIFTITEM_ERASE()" );
	if ( m_vecGIFT_QITEM.size() <= dwIndex )	return;

	INVENQARRAY_ITER iter = m_vecGIFT_QITEM.begin()+dwIndex;
	m_vecGIFT_QITEM.erase ( iter, iter+1 );
}

/*dmk14 GS attendance loader*/
bool GLQUEST_ATTENDANCE_OPTION::LOAD_0001( basestream &SFile )
{
	SFile >> bAdditionalQuest;
	SFile >> nType;
	SFile >> wMinLevel;
	SFile >> wMaxLevel;
	SFile >> dwDayofWeek;
	SFile >> wStartYear;
 	SFile >> WStarMonth;
 	SFile >> WstarDay; 
 	SFile >> wEndYear; 
 	SFile >> WEndMonth;
 	SFile >> WEndDay;
	return true;
}

bool GLQUEST_ATTENDANCE_OPTION::LOAD( basestream &SFile )
{
	DWORD dwVERSION(0), dwSIZE(0);

	SFile >> dwVERSION;
	SFile >> dwSIZE;

	switch ( dwVERSION )
	{
	case VERSION:	LOAD_0001 ( SFile ); break;
	default:
		{
			CDebugSet::ErrorVersion( "GLQUEST_ATTENDANCE_OPTION::LOAD(), unknown data version.", dwVERSION );
			SFile.SetOffSet ( SFile.GetfTell()+dwSIZE );
			break;
		}
	};

	return true;
}

bool GLQUEST_ATTENDANCE_OPTION::SAVE( CSerialFile &SFile )
{
	SFile << (DWORD) VERSION;
	SFile.BeginBlock (); // 현재 위치에 저장 사이즈를 저장한다
	{
		SFile << bAdditionalQuest;
		SFile << nType;
		SFile << wMinLevel;
		SFile << wMaxLevel;
		SFile << dwDayofWeek;
		SFile << wStartYear;
		SFile << WStarMonth;
		SFile << WstarDay; 
		SFile << wEndYear; 
		SFile << WEndMonth;
		SFile << WEndDay;
	}
	SFile.EndBlock ();

	return true;
}