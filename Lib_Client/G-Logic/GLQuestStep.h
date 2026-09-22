#ifndef GLQUEST_STEP_H_
#define GLQUEST_STEP_H_

#if _MSC_VER > 1000
#pragma once
#endif


#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"

#include <vector>
#include "./GLItem.h"
#include "./GLQuestGenItem.h"

/*dmk14 gs data*/
enum EMATTENDANCE_TYPE
{
	EMATTENDANCE_TYPE_NONE =  0,
	EMATTENDANCE_TYPE_CONNECTION,	//특정 시간동안 접속을 유지하라
	EMATTENDANCE_TYPE_USER_KILL,	//특정 지점에서 특정 횟수만큼 유저를 처치하라
	EMATTENDANCE_TYPE_MOB_KILL,	//특정 몹을 특정 횟수로 처치하라
	EMATTENDANCE_TYPE_LEVEL_UP,	//특정 레벨을 당성하라 
	EMATTENDANCE_TYPE_ITEM_GET,	//특정 아이템을 수집하라
	EMATTENDANCE_TYPE_ITEM_USE,	//특정 아이템을 특정 횟수로 사용하라
	EMATTENDANCE_TYPE_MOVE_POS,	//특정 맵(혹은 특정 지점)으로 이동하라
	EMATTENDANCE_TYPE_GRINDING_ITEM,	// 아이템을 특정 횟수만큼 연마하라
	EMATTENDANCE_TYPE_SPARRING,	//다른파티와 학원 대련을 특정 횟수만큼 실시하라
	EMATTENDANCE_TYPE_SIZE,
};

enum EMDAYOFWEEKFLAGS 
{
	EMF_SUN	= 0x0001,
	EMF_MON = 0x0002,
	EMF_TUE = 0x0004,
	EMF_WED = 0x0008,
	EMF_THU = 0x0010,
	EMF_FRI = 0x0020,
	EMF_SAT = 0x0040,
	EMF_ALL	= EMF_SUN|EMF_MON|EMF_TUE|EMF_WED|EMF_THU|EMF_FRI|EMF_SAT,
};

struct GLQUEST_ATTENDANCE_OPTION
{
	enum { VERSION = 0x0001, };

	bool bAdditionalQuest;
	int nType; // EMATTENDANCE_TYPE
	WORD wMinLevel; // 퀘스트 수행시 제한 레밸
	WORD wMaxLevel; // 퀘스트 수행시 제한 레밸
	DWORD dwDayofWeek; // 퀘스트 시작시 요일.

	WORD wStartYear; // 퀘스트 시작시 제한 년
	WORD WStarMonth; // 퀘스트 시작시 제한 월
	WORD WstarDay; // 퀘스트 시작시 제한 일

	WORD wEndYear; // 퀘스트 시작시 제한 년
	WORD WEndMonth; // 퀘스트 시작시 제한 월
	WORD WEndDay; // 퀘스트 시작시 제한 일


	GLQUEST_ATTENDANCE_OPTION () :
	bAdditionalQuest(false),
	nType(EMATTENDANCE_TYPE_NONE),
	wMinLevel(0),
	wMaxLevel(USHRT_MAX),
	dwDayofWeek(0),
	wStartYear(0),
	WStarMonth(0),
	WstarDay(0),
	wEndYear(0),
	WEndMonth(0),
	WEndDay(0)
	{
	}

	bool LOAD_0001 ( basestream &SFile );
	bool LOAD ( basestream &SFile );
	bool SAVE ( CSerialFile &SFile );
};

typedef std::vector<SGENQUESTITEM>	GENMOBITEMARRAY;
typedef GENMOBITEMARRAY::iterator	GENMOBITEMARRAY_ITER;

typedef std::vector<SITEMCUSTOM>	INVENQARRAY;
typedef INVENQARRAY::iterator		INVENQARRAY_ITER;

struct GLQUEST_PROGRESS_MAP
{
	SNATIVEID	nidMAP;
	WORD		wPosX;
	WORD		wPosY;
	DWORD		dwGateID;

	GLQUEST_PROGRESS_MAP() :
		nidMAP(false),
		wPosX(USHRT_MAX),
		wPosY(USHRT_MAX),
		dwGateID(0)
	{
	}

	void Init()
	{
		nidMAP   = false;
		wPosX    = USHRT_MAX;
		wPosY    = USHRT_MAX;
		dwGateID = 0;
	}
};


struct GLQUEST_STEP
{
	enum { VERSION = 0x0201, };

	std::string		m_strTITLE;				//	현재 단계에 대한 제목.
	std::string		m_strCOMMENT;			//	현재 단계에 대한 설명.

	//	이수 조건 #3.
	DWORD			m_dwNID_MOBKILL;		//	죽여야할 Mob ID.
	DWORD			m_dwNUM_MOBKILL;		//	죽여야할 Mob 숫자.
	std::string		m_strOBJ_MOBKILL;		//	이수조건 설명.

	//	이수 조건 #2.
	GENMOBITEMARRAY	m_vecMOBGEN_QITEM;		//	MOB 발생 퀘스트 아이템.
	std::string		m_strOBJ_MOBGEN_QITEM;	//	이수조건 설명.

	//	이수 조건 #1.
	DWORD			m_dwNID_NPCTALK;		//	대화가 필요한 NPC ID.
	std::string		m_strOBJ_NPCTALK;		//	이수조건 설명.


	//	이수 조건 #5.
	SNATIVEID		m_sMAPID_REACH;			//	도달해야할 MAPID.
	WORD			m_wPOSX_REACH;			//	도달해야할 x좌표.	( 미니멥에서 쓰는 좌표. )
	WORD			m_wPOSY_REACH;			//	도달해야할 y좌표.	( 미니멥에서 쓰는 좌표. )
	WORD			m_wRADIUS_REACH;		//	도달 반경.
	std::string		m_strOBJ_REACH_ZONE;	//	이수 조건 설명.

	//	이수 조건 #7.
	WORD			m_wLevel;				//	레벨 도달.

	//	이수 조건 #4.
	DWORD			m_dwNID_NPCGUARD;		//	보호해야할 NPC.
	std::string		m_strOBJ_NPCGUARD;		//	이수조건 설명.

	//	이수 조건 #6.
	SNATIVEID		m_sMAPID_DEFENSE;		//	방어해야할 MAPID.
	WORD			m_wPOSX_DEFENSE;		//	방어해야할 x좌표.	( 미니멥에서 쓰는 좌표. )
	WORD			m_wPOSY_DEFENSE;		//	방어해야할 y좌표.	( 미니멥에서 쓰는 좌표. )
	WORD			m_wRADIUS_DEFENSE;		//	방어 반경.
	float			m_fDEFENSE_TIME;		//	방어해야할 시간.
	std::string		m_strOBJ_DEFENSE_ZONE;	//	이수 조건 설명.

	//	완료시 처리.
	INVENQARRAY			 m_vecGIFT_QITEM;		//	현제 STEP 완료시 보상 Quest Item.
	DWQARRAY			 m_vecRESET_QITEM;		//	다음 스텟으로 넘어갈때 리셋될 Quest Item.

	GLQUEST_PROGRESS_MAP m_stepMoveMap;

	GLQUEST_STEP () :
		m_strTITLE("Quest Step Title"),
		m_strCOMMENT("Quest Step Description"),

		m_dwNID_MOBKILL(UINT_MAX),
		m_dwNUM_MOBKILL(0),
		m_strOBJ_MOBKILL("Kill Mob"),

		m_strOBJ_MOBGEN_QITEM("Mob Generate Quest Item"),

		m_dwNID_NPCTALK(UINT_MAX),
		m_strOBJ_NPCTALK("Talk to NPC"),

		m_sMAPID_REACH(false),
		m_wPOSX_REACH(0),
		m_wPOSY_REACH(0),
		m_wRADIUS_REACH(60),
		m_strOBJ_REACH_ZONE("Reach Zone"),

		m_wLevel(USHRT_MAX),

		m_dwNID_NPCGUARD(UINT_MAX),
		m_strOBJ_NPCGUARD("Guard NPC"),

		m_sMAPID_DEFENSE(false),
		m_wPOSX_DEFENSE(0),
		m_wPOSY_DEFENSE(0),
		m_wRADIUS_DEFENSE(0),
		m_fDEFENSE_TIME(0.0f),
		m_strOBJ_DEFENSE_ZONE("Defense Zone")	
	{
	}

	const char* GetTITLE () const				{ return m_strTITLE.c_str(); }
	const char* GetCOMMENT () const				{ return m_strCOMMENT.c_str(); }

	const char* GetOBJ_NPCTALK () const			{ return m_strOBJ_NPCTALK.c_str(); }
	const char* GetOBJ_MOBGEN_QITEM () const	{ return m_strOBJ_MOBGEN_QITEM.c_str(); }
	const char* GetOBJ_MOBKILL () const			{ return m_strOBJ_MOBKILL.c_str(); }
	const char* GetOBJ_NPCGUARD () const		{ return m_strOBJ_NPCGUARD.c_str(); }
	const char* GetOBJ_REACH_ZONE () const		{ return m_strOBJ_REACH_ZONE.c_str(); }
	const char* GetOBJ_DEFENSE_ZONE () const	{ return m_strOBJ_DEFENSE_ZONE.c_str(); }

	bool IsNEED_NPCTALK () const		{ return (m_dwNID_NPCTALK!=UINT_MAX); }
	bool IsNEED_QITEM () const			{ return (!m_vecMOBGEN_QITEM.empty()); }
	bool IsNEED_MOBKILL () const		{ return (m_dwNID_MOBKILL!=UINT_MAX); }
	bool IsNEED_NPCGUARD () const		{ return (m_dwNID_NPCGUARD!=UINT_MAX); }
	bool IsNEED_REACHZONE () const		{ return (m_sMAPID_REACH!=SNATIVEID(false)); }
	bool IsNEED_DEFENSEZONE () const	{ return (m_sMAPID_DEFENSE!=SNATIVEID(false)); }
	bool IsNEED_LEVEL () const			{ return (m_wLevel!=USHRT_MAX); }

	void RESET_NPCTALK ()
	{
		m_strOBJ_NPCTALK = "";
		m_dwNID_NPCTALK = UINT_MAX;
	}

	void RESET_MOBGEN_QITEM ()
	{
		m_strOBJ_MOBGEN_QITEM = "";
		m_vecMOBGEN_QITEM.clear();
	}

	void RESET_MOBKILL ()
	{
		m_strOBJ_MOBKILL = "";
		m_dwNID_MOBKILL = UINT_MAX;
		m_dwNUM_MOBKILL = 0;
	}

	void RESET_NPCGUARD ()
	{
		m_strOBJ_NPCGUARD = "";
		m_dwNID_NPCGUARD = UINT_MAX;
	}

	void RESET_REACHZONE ()
	{
		m_strOBJ_REACH_ZONE = "";
		m_sMAPID_REACH = SNATIVEID(false);
		m_wPOSX_REACH = (0);
		m_wPOSY_REACH = (0);
		m_wRADIUS_REACH = (60);
	}

	void RESET_DEFENSE ()
	{
		m_strOBJ_DEFENSE_ZONE = "";
		m_sMAPID_DEFENSE = SNATIVEID(false);
		m_wPOSX_DEFENSE = (0);
		m_wPOSY_DEFENSE = (0);
		m_wRADIUS_DEFENSE = (0);
		m_fDEFENSE_TIME = (0.0f);
	}

	void RESET_LEVEL ()
	{
		m_wLevel = USHRT_MAX;
	}

	bool LOAD_0001 ( basestream &SFile );
	bool LOAD_0002 ( basestream &SFile );
	bool LOAD_0003 ( basestream &SFile );
	bool LOAD_0004 ( basestream &SFile );
	bool LOAD_0005 ( basestream &SFile );
	bool LOAD_0006 ( basestream &SFile );
	bool LOAD_0007 ( basestream &SFile );
	bool LOAD_0008 ( basestream &SFile );
	bool LOAD_0009 ( basestream &SFile );
	bool LOAD_0010 ( basestream &SFile );
	bool LOAD_0011 ( basestream &SFile );
	bool LOAD_0012 ( basestream &SFile );
	bool LOAD_0013 ( basestream &SFile );
	/*dmk14 ver15 quest loader*/
	bool LOAD_0015 ( basestream &SFile );
	/*dmk14 ver18 quest loader*/
	bool LOAD_0018 ( basestream &SFile );
	/*dmk14 ver21 quest loader*/
	bool LOAD_0021 ( basestream &SFile );
	bool LOAD_0200 ( basestream &SFile );
	bool LOAD_0201 ( basestream &SFile );

	bool LOAD ( basestream &SFile );
	bool SAVE ( CSerialFile &SFile );

	GLQUEST_STEP& operator= ( const GLQUEST_STEP &rVALUE );

	void RESETITEM_ERASE ( DWORD dwIndex );
	void MOBGENITEM_ERASE ( DWORD dwIndex );
	void GIFTITEM_ERASE ( DWORD dwIndex );
};

#endif // GLQUEST_STEP_H_