#ifndef GLQUEST_H_
#define GLQUEST_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <vector>

#include "./GLItem.h"
#include "./GLogicData.h"

#include "../../Lib_Engine/G-Logic/GLDefine.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/COMMON/StringFile.h"

#include "./GLQuestGenItem.h"
#include "./GLQuestStart.h"
#include "./GLQuestStep.h"

enum EMQUESTFLAGS
{
	EMQF_REPEAT		= 0x0001,		//	완료된 퀘스트 수행 가능 여부.
	EMQF_AGAIN		= 0x0002,		//	실패한 퀘스트 수행 가능 여부.
	EMQF_ONLYONE	= 0x0004,		//	이 퀘스트는 다른 퀘스트와 동시 진행 불가능.
	EMQF_GIVEUP		= 0x0008,		//	퀘스트 포기 가능 여부.
	EMQF_DAILY		= 0x0010,		/*dmk14 daily quest*/
	
	EMQF_PARTY		= 0x1000,		//	파티만 참여 가능한 퀘스트.
	EMQF_PARTYQUEST	= 0x2000,		//	파티원 동시 진행 퀘스트.
};

enum EMQUESTPROGRESS
{
	EMQP_NON_EVENT = 0, // 이벤트 없음
	EMQP_CHAR_DEAD,		// 캐릭터 사망
	EMQP_START_MOVE,	// 시작지점으로 이동
    EMQP_SELECT_MOVE,   // 지정된 위치로 이동
	EMQP_SIZE,
};

enum EMFAILTYPE
{
	EMFAIL_NONE		= 0x0000,		// 타입없음
	EMFAIL_TIMEOVER = 0x0001,		// 타임오버로 실패시
	EMFAIL_DIE      = 0x0002,		// 죽어서 실패시
	EMFAIL_LEAVE    = 0x0004,		// 특정 맵에서 떠나서 실패시
};

enum EMQUEST_AREA
{
	EMQUEST_AREA_00		= 0,
	EMQUEST_AREA_01		= 1,
	EMQUEST_AREA_02		= 2,
	EMQUEST_AREA_03		= 3,
	EMQUEST_AREA_04		= 4,
	EMQUEST_AREA_05		= 5,
	EMQUEST_AREA_06		= 6,
	EMQUEST_AREA_07		= 7,
	EMQUEST_AREA_08		= 8,
	EMQUEST_AREA_09		= 9,
	EMQUEST_AREA_10		= 10,
	EMQUEST_AREA_11		= 11,
	EMQUEST_AREA_12		= 12,
	EMQUEST_AREA_13		= 13,
	EMQUEST_AREA_14		= 14,
	EMQUEST_AREA_15		= 15,
	EMQUEST_AREA_16		= 16,
	EMQUEST_AREA_17		= 17,
	EMQUEST_AREA_18		= 18,
	EMQUEST_AREA_19		= 19,
	EMQUEST_AREA_20		= 20,
	EMQUEST_AREA_21		= 21,
	EMQUEST_AREA_22		= 22,
	EMQUEST_AREA_23		= 23,
	EMQUEST_AREA_24		= 24,
	EMQUEST_AREA_25		= 25,
	EMQUEST_AREA_26		= 26,
	EMQUEST_AREA_27		= 27,
	EMQUEST_AREA_28		= 28,
	EMQUEST_AREA_29		= 29,
	EMQUEST_AREA_SIZE	= 30,
};

struct GLQUEST_PROGRESS 
{
	DWORD dwStartProgress;  // 시작시 이벤트
	DWORD dwEndProgress;	  // 종료시 이벤트
	DWORD dwFailProgress;	  // 실패시 이벤트
		
	DWORD	dwfailType;		  // 실패의 조건

	GLQUEST_PROGRESS_MAP startMap;
	GLQUEST_PROGRESS_MAP endMap;
	GLQUEST_PROGRESS_MAP failMap;

	bool	bUseProgressEvent;

	GLQUEST_PROGRESS()
	{
		Init();
	}

	VOID Init()
	{
		dwStartProgress = EMQP_NON_EVENT;
		dwEndProgress   = EMQP_NON_EVENT;
		dwFailProgress  = EMQP_NON_EVENT;

		dwfailType    = 0;

		startMap.Init();	
		endMap.Init();
		failMap.Init();

		bUseProgressEvent = FALSE;
	};

	bool IsTIMEOVER ()		{ return (dwfailType&EMFAIL_TIMEOVER)!=NULL; }
	bool IsDIE ()			{ return (dwfailType&EMFAIL_DIE)!=NULL; }	
	bool IsLEAVE ()			{ return (dwfailType&EMFAIL_LEAVE)!=NULL; }	
    
};

class GLQUEST
{
public:
	enum { VERSION = 0x0202, };

	typedef std::vector<GLQUEST_STEP>	VECQSTEP;
	typedef VECQSTEP::iterator			VECQSTEP_ITER;

public:
	SNATIVEID		m_sNID;					//	고유 ID. 
	DWORD			m_dwEDITVER;			//	편집 버전.
	std::string		m_strTITLE;				//	퀘스트 타이틀.
	std::string		m_strCOMMENT;			//	퀘스트 설명.	
	DWORD			m_dwFlags;				//	속성.
	EMQUEST_AREA	m_emQuestArea;	
	GLQUEST_START	m_sSTARTOPT;			//	자동 시작 조건.

	GLQUEST_ATTENDANCE_OPTION m_sAttendanceOption; // 출석부 퀘스트 옵션

	DWORD			m_dwBeginPartyMemNum;	//	퀘스트를 받기 위한 최소 파티 인원.
	DWORD			m_dwBeginMoney;			//	퀘스트 받기 위한 비용.
	DWORD			m_dwLimitTime;			//	제한 시간.	( 실제 시간의 분단위. )

	bool			m_bTimeLimiorKeep;		//	제한, 유지 시간 ex) 제한시간 10분, 10분동안 접속유지
	bool			m_bPastOnlyOnOline;	//	클라이언트 종료후에도 시간 흐름
	bool			m_bNonDie;				//	수행 도중 사망하면 안됨.
	DWORD			m_dwLimitPartyMemNum;	//	제한 파티 인원.
	VECQSTEP		m_vecProgStep;			//	전체 진행 스텝 정보.
	DWORD			m_dwGiftMONEY;			//	완료시 보상.
	LONGLONG		m_llGiftEXP;
	DWORD			m_dwGiftSKILLPOINT;
	DWORD			m_dwGiftSTATSPOINT;
	DWORD			m_dwGiftLIFEPOINT;
	DWORD			m_dwGiftDisPK;
	DWORD			m_dwGiftELEMENT;
	DWORD			m_dwGiftQUEST;
	INVENQARRAY		m_vecGiftITEM;
	DWQARRAY		m_vecGiftSKILL;
	GLQUEST_PROGRESS m_ProgressEvent;		//	퀘스트 진행 중에 적용되는 이벤트

public:
	GLQUEST () :
		m_dwEDITVER(1),
		m_strTITLE("Quest Title"),
		m_strCOMMENT("Quest Description"),
		m_dwFlags(0),
		m_emQuestArea(EMQUEST_AREA_00),
		m_dwBeginPartyMemNum(0),
		m_dwBeginMoney(0),
		m_dwLimitTime(0),
		m_bNonDie(false),
		m_dwLimitPartyMemNum(0),
		m_dwGiftMONEY(0),
		m_llGiftEXP(0),
		m_dwGiftSKILLPOINT(0),
		m_dwGiftSTATSPOINT(0),
		m_dwGiftLIFEPOINT(0),
		m_dwGiftDisPK(0),
		m_dwGiftELEMENT(0),
		m_dwGiftQUEST(UINT_MAX)
	{
	}

	GLQUEST& operator= ( const GLQUEST &rVALUE );

public:
	const char* GetTITLE()			{ return m_strTITLE.c_str(); }
	DWORD GetSTEPNUM ()				{ return (DWORD) m_vecProgStep.size(); }
	GLQUEST::VECQSTEP& GetSTEP ()	{ return m_vecProgStep; }
	GLQUEST_STEP* GetSTEP ( DWORD dwSTEP );

public:
	DWORD ToUpStep ( DWORD dwSTEP );
	DWORD ToDownStep ( DWORD dwSTEP );

public:
	bool IsREPEAT ()		{ return (m_dwFlags&EMQF_REPEAT)!=NULL; }		//	여러번 수행 가능.
	bool IsAGAIN ()			{ return (m_dwFlags&EMQF_AGAIN)!=NULL; }		//	(포기한것) 다시 수행 가능.
	bool IsPARTY ()			{ return (m_dwFlags&EMQF_PARTY)!=NULL; }		//	파티일때 수행 가능.
	bool IsPARTYQUEST ()	{ return (m_dwFlags&EMQF_PARTYQUEST)!=NULL; }	//	파티원 동시 진행 퀘스트.

	bool IsONLYONE ()		{ return (m_dwFlags&EMQF_ONLYONE)!=NULL; }		//	동시에 진행불가?
	bool IsGIVEUP ()		{ return (m_dwFlags&EMQF_GIVEUP)!=NULL; }		//	포기 가능
	
	bool IsLIMITTIME ()		{ return m_dwLimitTime!=0; }
	bool IsNonDIE ()		{ return m_bNonDie; }

	bool IsDaily()			{ return (m_dwFlags&EMQF_DAILY)!=NULL;	}		/*dmk14 daily quest*/

protected:
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
	bool LOAD_0014 ( basestream &SFile );
	bool LOAD_0015 ( basestream &SFile );
	bool LOAD_0016 ( basestream &SFile );
	/*dmk14 ver18 quest loader*/
	bool LOAD_0018 ( basestream &SFile );
	/*dmk14 ver24 quest loader*/
	bool LOAD_0024 ( basestream &SFile );
	bool LOAD_0200 ( basestream &SFile );
	bool LOAD_0201 ( basestream &SFile );
	bool LOAD_0202 ( basestream &SFile );
	
public:
	bool LOAD ( const char *szFile );
	bool SAVE ( const char *szFile );

	void SaveCsv( std::fstream &SFile, CString strFileName );
	void SaveCsvInfoHead( std::fstream &SFile, CString strFileName );
	void SaveCsvBaseHead( std::fstream &SFile );
	void SaveCsvInfoData( std::fstream &SFile );
	void SaveCsvBaseData( std::fstream &SFile );
	void SaveCsvProgress( std::fstream &SFile );
	void SaveCsvProgressBaseHead( std::fstream &SFile );
	void SaveCsvProgressBaseData( std::fstream &SFile );

	void LoadCsv( CStringArray &StrArray, CStringArray &StrArrayProgress );
	void LoadCsvInfoDate( CStringArray &StrArray );
	void LoadCsvBaseData( CStringArray &StrArray );
	void LoadCsvProgress( CStringArray &StrArray );
	void LoadCsvProgressBaseHead( CStringArray &StrArray );
	void LoadCsvProgressBaseData( CStringArray &StrArray );

	void ExportText( std::fstream &SFile );
	void ExportTextSubJect( std::fstream &SFile );
	void ExportTextProgress( std::fstream &SFile );
	void ImportText( CStringFile& strFile );
	void ImportTextSubject( CStringFile& strFile );
	void ImportTextProgress( CStringFile& strFile );
};

enum EMQUESTPROG
{
	EMQP_REQREADING		= 0x0001,	// 퀘스트를 열어서 읽어보기를 요청.
};

struct GLQUESTPROG_STEP
{
	enum { VERSION = 0x0002, };

	//	이수 조건.
	bool		m_bTALK;					//	대화했는지 여부.
	bool		m_bQITEM;					//	필요 퀘스트 아이템 결과.

	bool		m_bMOBKILL_ALL;				//	죽여야할 Mob 모두 죽임.
	DWORD		m_dwNUM_MOBKILL;			//	죽인 Mob 숫자.
	bool		m_bNPCGUARD_DEATH;			//	보호해야할 NPC 사망 여부.

	bool		m_bREACH_ZONE;				//	도달 위치에 도착 여부.
	
	bool		m_bDEFENSE_ZONE_BEGIN;		//	방어 위치 사수 시작 여부.
	bool		m_bDEFENSE_ZONE_COMPLETE;	//	방어 위치 사수 완료.
	float		m_fDEFENSE_ZONE_TIMER;		//	방어 위치 사수 경과시간.

	bool		m_bLevel;

public:
	GLQUESTPROG_STEP () :
		m_bTALK(true),
		m_bQITEM(true),

		m_bMOBKILL_ALL(true),
		m_dwNUM_MOBKILL(UINT_MAX),
		m_bNPCGUARD_DEATH(false),

		m_bREACH_ZONE(true),

		m_bDEFENSE_ZONE_BEGIN(true),
		m_bDEFENSE_ZONE_COMPLETE(true),
		m_fDEFENSE_ZONE_TIMER(FLT_MAX),

		m_bLevel(true)
	{
	}

public:
	bool BEGIN_PROG ( const GLQUEST_STEP &sSTEP );
	void RESET ();

public:
	bool IsCOMPLETE ();

public:
	GLQUESTPROG_STEP& operator= ( const GLQUESTPROG_STEP &rVALUE );

public:
	BOOL SET_BYBUFFER ( CByteStream &ByteStream ) const;
	BOOL GET_BYBUFFER ( CByteStream &ByteStream );
};

class GLQUESTPROG
{
public:
	enum { VERSION = 0x0003, INVEN_SIZEX=8, INVEN_SIZEY=1 };

	typedef std::vector<DWORD>	DWARRAY;
	typedef DWARRAY::iterator	DWARRAY_ITER;

public:
	SNATIVEID	m_sNID;					//	고유 ID.
	DWORD		m_dwEDITVER;
	DWORD		m_dwFlags;				//	속성.
	DWORD		m_dwCOUNT;				//	시도 횟수.

	bool		m_bCOMPLETE;			//	완료 여부.
	float		m_fLAPSTIME;			//	시작후 경과 시간.

	DWORD				m_dwSTEP;		//	현재 진행 퀘스트 STEP.
	GLQUESTPROG_STEP	m_sSTEP_PROG;	//	현재 진행 정보.

	GLInventory			m_sINVENTORY;	//	퀘스트 전용 인벤.

	__time64_t			m_tStartTime;		//	퀘스트 시작 시간

	__time64_t			m_tEndTime;		/*dmk14 daily quest*/

public:
	bool START ( GLQUEST* pQUEST, GLQUESTPROG *pOLD_PROG );
	bool DoPROGRESS ( bool &_bModifyInven, GLQUEST_PROGRESS_MAP &stepMoveMap );
	void DoGIVEUP ();
	bool DoCOMPLETE ();

	bool CheckCOMPLETE ();
	bool CheckTIMEOVER ();

public:
	HRESULT FrameMove ( float fTime, float fElapsedTime );

public:
	void ResetReqREADING ()	{ m_dwFlags &= ~EMQP_REQREADING; }

public:
	void ReqREADING ()		{ m_dwFlags |= EMQP_REQREADING; }
	bool IsReqREADING ()	{ return (m_dwFlags&EMQP_REQREADING)!=NULL; }
	bool IsNonDIE ();

public:
	GLQUESTPROG& operator= ( const GLQUESTPROG &rVALUE );

public:
	BOOL SET_BYBUFFER ( CByteStream &ByteStream ) const;
	BOOL GET_BYBUFFER ( CByteStream &ByteStream );

public:
	BOOL LOAD_001 ( CByteStream &ByteStream );
	BOOL LOAD_002 ( CByteStream &ByteStream );
	/*dmk14 daily quest*/
	BOOL LOAD_003 ( CByteStream &ByteStream );
public:
	GLQUESTPROG () :
		m_sNID(false),
		m_dwEDITVER(1),
		m_dwFlags(NULL),
		m_bCOMPLETE(false),
		m_dwCOUNT(0),
		m_fLAPSTIME(0.0f),
		m_tStartTime(0),
		m_dwSTEP(0)
	{
		m_sINVENTORY.SetState ( INVEN_SIZEX, INVEN_SIZEY );
	}
};

namespace COMMENT
{
	extern std::string QUEST_PROGRESS[EMQP_SIZE];
	extern std::string QUEST_AREA[EMQUEST_AREA_SIZE];
}

#endif // GLQUEST_H_