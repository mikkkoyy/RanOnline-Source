#ifndef GLCHARDEFINE_H_
#define GLCHARDEFINE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <set>
#include "../../Lib_Engine/G-Logic/GLDefine.h"
#include "../../Lib_Network/s_NetGlobal.h"

/////////////////////////////////////////////////////////////////////////////
// LG-7 CWRanking
#include <map>
/////////////////////////////////////////////////////////////////////////////

#define CHAR_SZNAME CHR_ID_LENGTH //	케릭터 이름 제한.
#define CHAR_TEMPMONEY 64		  //   TempMoney 최대 Char 배열의 길이

/*equipment lock, Juver, 2018/01/13 */
#define CHAR_EQUIPMENT_LOCK_PASS_SIZE 4
/*storage lock, Juver, 2018/01/13 */
#define USER_STORAGE_LOCK_PASS_SIZE 4
/*inventory lock, Juver, 2018/01/13 */
#define CHAR_INVENTORY_LOCK_PASS_SIZE 4

/////////////////////////////////////////////////////////////////////////////
// LG-7 CWRanking
struct SCW_RANKING
{
	int		nIndex;
	DWORD	dwCharID;
	WORD	wKill;
	WORD	wDead;
	char	szChaName[CHAR_SZNAME];

	SCW_RANKING()
		: nIndex(-1)
		, dwCharID(0)
		, wKill(0)
		, wDead(0)
	{
		SecureZeroMemory(szChaName, sizeof(char) * CHAR_SZNAME);
	}
};

typedef std::map<DWORD, SCW_RANKING>	MAP_CW_RANKING;
typedef MAP_CW_RANKING::iterator		MAP_CW_RANKING_ITER;

typedef std::vector<SCW_RANKING>	VEC_CW_RANKING;
typedef VEC_CW_RANKING::iterator	VEC_CW_RANKING_ITER;

struct SCW_RANKING_REWARD
{
	SNATIVEID sReward_[10];

	SCW_RANKING_REWARD()
	{
		for (int i = 0; i < 10; i++)
			sReward_[i] = NATIVEID_NULL();
	}
};
/////////////////////////////////////////////////////////////////////////////

enum EMCHARDEFINE
{
	GAEAID_NULL			= (0xFFFFFFFF),

	CLUB_NULL			= (0),
	PARTY_NULL			= (GAEAID_NULL),

	//CHAR_SZNAME			= CHR_ID_LENGTH,	//	케릭터 이름 제한.
	CHAR_SZNOTE			= 160,				// 쪽지의 길이 제한
	CHAR_MAXSKILL		= 32,				//	습득 가능 최대 스킬 갯수.
	
	SALE_INVEN_X		= 6,
	SALE_INVEN_Y		= 8,	

	STORAGE_INVEN_X		= 6,
	STORAGE_INVEN_Y		= 4,

	CHARGED_INVEN_X		= 6,
	CHARGED_INVEN_Y		= 32,
};


enum EMCLUB
{
	EMCLUB_MARK_SX			= 16, // 클럽마크의 폭 16 픽셀
	EMCLUB_MARK_SY			= 11, // 클럽마크의 높이 11 픽셀

    EMCLUB_MARK_SIZE = sizeof(DWORD)*EMCLUB_MARK_SX*EMCLUB_MARK_SY,

	EMCLUB_NOTICE_LEN_LINE	= 40,
	EMCLUB_NOTICE_LINE		= 10,
	
	// 클럽 공지의 글자수 제한 ( 글자수 * 라인 + ( 라인개행표시 )  ) 								   
	EMCLUB_NOTICE_LEN		=  ( EMCLUB_NOTICE_LEN_LINE * EMCLUB_NOTICE_LINE ) + (( EMCLUB_NOTICE_LINE - 1 ) * 2 ),

	EMCLUB_SUBMASTER_JOIN	= 0x0001, // 클럽에 가입 시킬 수 있는 권한
	EMCLUB_SUBMASTER_KICK	= 0x0002, // 클럽에서 제명 시킬 수 있는 권한
	EMCLUB_SUBMASTER_NOTICE	= 0x0004, // 클럽 공지를 작성할 수 있는 권한
	EMCLUB_SUBMASTER_CD		= 0x0008, // 선도클럽 결정전에 CD 를 인증 할 수 있는 권한
	EMCLUB_SUBMASTER_MARK	= 0x0010, // 클럽 마크를 변경할 수 있는 권한
	EMCLUB_SUBMASTER_CDM	= 0x0020, // CDM 참가 자격

	EMCLUB_SUBMASTER_ALL		= EMCLUB_SUBMASTER_JOIN | EMCLUB_SUBMASTER_KICK | EMCLUB_SUBMASTER_NOTICE | EMCLUB_SUBMASTER_CD | EMCLUB_SUBMASTER_MARK | EMCLUB_SUBMASTER_CDM,
};

enum EMLAND
{
	MAXITEM			= 3000,		//	SUIT, 멥상에 존재 할수 있는 최대 갯수.
	MAXMONEY		= 3000,		//	MONEY, 멥상에 존재 할수 있는 최대 갯수.

	MAXCROW			= 3000,		//	CROW, 멥상에 존재 할수 있는 최대 갯수.
	MAXPET			= 3000,		//  PET, 맵상에 존재 할수 있는 최대 갯수.
};

enum EMIDTYPE
{
	ID_USER			= 1,
	ID_CHAR			= 2,
	ID_CLUB			= 3,
	ID_MOB			= 4,
	ID_SYSTEM		= 5,
};

enum EMLOGACTION
{
	EMLOGACT_DIE				= 1,
	EMLOGACT_KILL				= 2,
	EMLOGACT_HOSTILE			= 3,
	EMLOGACT_LEV_UP				= 4,
	EMLOGACT_RESTORE			= 5,

	EMLOGACT_CLUB_MAKE			= 11,
	EMLOGACT_CLUB_DELETE		= 12,
	EMLOGACT_CLUB_LEV_UP		= 13,
	EMLOGACT_CLUB_JOIN_MEMBER	= 14,
	EMLOGACT_CLUB_SECEDE_MEMBER	= 15,
	EMLOGACT_CLUB_KICK_MEMBER	= 16,	
	EMLOGACT_CLUB_CLUBDEPUTY_MEMBER = 17,

	EMLOGACT_RECOVERY			= 18,
	EMLOGACT_RECOVERY_NPC			= 19,
	EMLOGACT_PET_SKILL			= 20,


	EMLOGACT_FRIEND_ADD			= 21,
	EMLOGACT_FRIEND_DEL			= 22,
	EMLOGACT_FRIEND_BLOCK		= 23,
	
	EMLOGACT_CLUB_AUTHORITY		= 24,
	EMLOGACT_CHEATER_BAN		= 25,
};

//	Note : 캐릭터 FLAG.
//
enum EMCHARCLASS
{
	GLCC_NONE			= 0X00000000,	
	GLCC_BRAWLER_M		= 0x00000001,	
	GLCC_SWORDSMAN_M	= 0x00000002,	
	GLCC_ARCHER_W		= 0x00000004,	
	GLCC_SHAMAN_W		= 0x00000008,	
	GLCC_EXTREME_M		= 0x00000010,   
	GLCC_EXTREME_W		= 0x00000020,   
	GLCC_BRAWLER_W		= 0x00000040,
	GLCC_SWORDSMAN_W	= 0x00000080,
	GLCC_ARCHER_M		= 0x00000100,
	GLCC_SHAMAN_M		= 0x00000200,
	GLCC_GUNNER_M		= 0x00000400,
	GLCC_GUNNER_W		= 0x00000800,  
	GLCC_ASSASSIN_M		= 0x00001000,
	GLCC_ASSASSIN_W		= 0x00002000,  
	GLCC_TRICKER_M		= 0x00004000,
	GLCC_TRICKER_W		= 0x00008000,  

	GLCC_BRAWLER		= ( GLCC_BRAWLER_M | GLCC_BRAWLER_W ),
	GLCC_SWORDSMAN		= ( GLCC_SWORDSMAN_M | GLCC_SWORDSMAN_W ),
	GLCC_ARCHER			= ( GLCC_ARCHER_M | GLCC_ARCHER_W ),
	GLCC_SHAMAN			= ( GLCC_SHAMAN_M | GLCC_SHAMAN_W ),
	GLCC_EXTREME		= ( GLCC_EXTREME_M | GLCC_EXTREME_W ),
	GLCC_GUNNER			= ( GLCC_GUNNER_M | GLCC_GUNNER_W ),
	GLCC_ASSASSIN		= ( GLCC_ASSASSIN_M | GLCC_ASSASSIN_W ),
	GLCC_TRICKER		= ( GLCC_TRICKER_M | GLCC_TRICKER_W ),

	//GLCC_ALL_OLD_EX
	GLCC_ALL_5CLASS		= (GLCC_BRAWLER_M|GLCC_SWORDSMAN_M|GLCC_ARCHER_W|GLCC_SHAMAN_W|GLCC_EXTREME_M|GLCC_EXTREME_W),

	//GLCC_ALL_NEWSEX
	GLCC_ALL_5CLASS_EX	= (GLCC_BRAWLER_M | GLCC_BRAWLER_W | GLCC_SWORDSMAN_M | GLCC_SWORDSMAN_W | GLCC_ARCHER_M | GLCC_ARCHER_W | GLCC_SHAMAN_M | GLCC_SHAMAN_W | 
						GLCC_EXTREME_M | GLCC_EXTREME_W),

	GLCC_ALL_6CLASS		= (GLCC_BRAWLER_M | GLCC_BRAWLER_W | GLCC_SWORDSMAN_M | GLCC_SWORDSMAN_W | GLCC_ARCHER_M | GLCC_ARCHER_W | GLCC_SHAMAN_M | GLCC_SHAMAN_W | 
						GLCC_EXTREME_M | GLCC_EXTREME_W | GLCC_GUNNER_M | GLCC_GUNNER_W ),

	GLCC_ALL_7CLASS		= (GLCC_BRAWLER_M | GLCC_BRAWLER_W | GLCC_SWORDSMAN_M | GLCC_SWORDSMAN_W | GLCC_ARCHER_M | GLCC_ARCHER_W | GLCC_SHAMAN_M | GLCC_SHAMAN_W | 
						GLCC_EXTREME_M | GLCC_EXTREME_W | GLCC_GUNNER_M | GLCC_GUNNER_W | GLCC_ASSASSIN_M | GLCC_ASSASSIN_W ),

	GLCC_ALL_8CLASS		= (GLCC_BRAWLER_M | GLCC_BRAWLER_W | GLCC_SWORDSMAN_M | GLCC_SWORDSMAN_W | GLCC_ARCHER_M | GLCC_ARCHER_W | GLCC_SHAMAN_M | GLCC_SHAMAN_W | 
						GLCC_EXTREME_M | GLCC_EXTREME_W | GLCC_GUNNER_M | GLCC_GUNNER_W | GLCC_ASSASSIN_M | GLCC_ASSASSIN_W | GLCC_TRICKER_M | GLCC_TRICKER_W ),

	GLCC_MAN_5CLASS_EX		= (GLCC_BRAWLER_M | GLCC_SWORDSMAN_M | GLCC_ARCHER_M | GLCC_SHAMAN_M | GLCC_EXTREME_M ),
	GLCC_WOMAN_5CLASS_EX	= (GLCC_BRAWLER_W | GLCC_SWORDSMAN_W | GLCC_ARCHER_W | GLCC_SHAMAN_W | GLCC_EXTREME_W ),

	GLCC_MAN_6CLASS		= (GLCC_BRAWLER_M | GLCC_SWORDSMAN_M | GLCC_ARCHER_M | GLCC_SHAMAN_M | GLCC_EXTREME_M | GLCC_GUNNER_M ),
	GLCC_WOMAN_6CLASS	= (GLCC_BRAWLER_W | GLCC_SWORDSMAN_W | GLCC_ARCHER_W | GLCC_SHAMAN_W | GLCC_EXTREME_W | GLCC_GUNNER_W ),

	GLCC_MAN_7CLASS		= (GLCC_BRAWLER_M | GLCC_SWORDSMAN_M | GLCC_ARCHER_M | GLCC_SHAMAN_M | GLCC_EXTREME_M | GLCC_GUNNER_M | GLCC_ASSASSIN_M ),
	GLCC_WOMAN_7CLASS	= (GLCC_BRAWLER_W | GLCC_SWORDSMAN_W | GLCC_ARCHER_W | GLCC_SHAMAN_W | GLCC_EXTREME_W | GLCC_GUNNER_W | GLCC_ASSASSIN_W ),

	GLCC_MAN_8CLASS		= (GLCC_BRAWLER_M | GLCC_SWORDSMAN_M | GLCC_ARCHER_M | GLCC_SHAMAN_M | GLCC_EXTREME_M | GLCC_GUNNER_M | GLCC_ASSASSIN_M | GLCC_TRICKER_M ),
	GLCC_WOMAN_8CLASS	= (GLCC_BRAWLER_W | GLCC_SWORDSMAN_W | GLCC_ARCHER_W | GLCC_SHAMAN_W | GLCC_EXTREME_W | GLCC_GUNNER_W | GLCC_ASSASSIN_W | GLCC_TRICKER_W ),
};

typedef EMCHARCLASS*	PEMCHARCLASS;

enum EMSCHOOLFLAG
{
	GLSCHOOL_00		= 0x1,
	GLSCHOOL_01		= 0x2,
	GLSCHOOL_02		= 0x4,

	GLSCHOOL_NUM	= 3,
	GLSCHOOL_ALL	= (GLSCHOOL_00|GLSCHOOL_01|GLSCHOOL_02),
};

WORD school2index ( const EMSCHOOLFLAG emSchool );
EMSCHOOLFLAG index2school ( const WORD wSchool );

//	Note : 캐릭터 인덱스.
//

enum EMCHARINDEX
{
	GLCI_BRAWLER_M		= 0,
	GLCI_SWORDSMAN_M	= 1,
	GLCI_ARCHER_W		= 2,
	GLCI_SHAMAN_W		= 3,
	GLCI_EXTREME_M		= 4,	
	GLCI_EXTREME_W		= 5,
	GLCI_BRAWLER_W		= 6,
	GLCI_SWORDSMAN_W	= 7,
	GLCI_ARCHER_M		= 8,
	GLCI_SHAMAN_M		= 9,
	GLCI_GUNNER_M		= 10,	
	GLCI_GUNNER_W		= 11,
	GLCI_ASSASSIN_M		= 12,	
	GLCI_ASSASSIN_W		= 13,
	GLCI_TRICKER_M		= 14,	
	GLCI_TRICKER_W		= 15,

	GLCI_NUM_4CLASS		= 4,	//GLCI_NUM
	GLCI_NUM_5CLASS		= 6,	//GLCI_NUM_EX
	GLCI_NUM_5CLASS_EX	= 10,	//GLCI_NUM_NEWSEX
	GLCI_NUM_6CLASS		= 12,
	GLCI_NUM_7CLASS		= 14,
	GLCI_NUM_8CLASS		= 16,
};

enum EMCHARINDEX2
{
	GLCI2_BRAWLER	= 0,
	GLCI2_SWORDSMAN	= 1,
	GLCI2_ARCHER	= 2,
	GLCI2_SHAMAN	= 3,
	GLCI2_EXTREME	= 4,
	GLCI2_GUNNER	= 5,
	GLCI2_ASSASSIN	= 6,
	GLCI2_TRICKER	= 7,

	GLCI2_NUM_5CLASS_EX	= 5, //GLCI2_NUM
	GLCI2_NUM_6CLASS	= 6,
	GLCI2_NUM_7CLASS	= 7,
	GLCI2_NUM_8CLASS	= 8,
};

enum EMCHARGENDER 
{
	GLGENDER_W		= 0, //female gender
	GLGENDER_M		= 1, //male gender
	GLGENDER_NUM	= 2,
};

enum EMCLASSINDEX
{
	GLCL_BRAWLER		= 0, // 격투부;
	GLCL_SWORDSMAN			= 1, // 검도부;
	GLCL_ARCHER		= 2, // 양궁부;
	GLCL_SHAMAN		= 3, // 기예부;
	GLCL_EXTREME	= 4, // 극강부;
	GLCL_GUNNER	= 5, // 과학부;
    GLCL_ASSASSIN	= 6, // 인술부;
	GLCL_TRICKER		= 7, // 마술부;
    GLCL_ETC			= 8, // 기타;
	GLCL_ACTOR		= 9,
	GLCL_NUM_CLASS	= 10, // 클래스 갯수
};

EMCLASSINDEX CharClassToClassIndex ( const EMCHARCLASS emClass );
EMCHARINDEX CharClassToIndex( const EMCHARCLASS emClass );
EMCHARCLASS CharIndexToClass( const EMCHARINDEX emIndex );
EMCHARCLASS CharClassToSex( const EMCHARCLASS emClass );
EMCHARINDEX CharIndexToSex( const EMCHARINDEX emIndex );
EMCHARGENDER CharClassToGender( const EMCHARCLASS emClass );
EMCHARGENDER CharIndexToGender( const EMCHARINDEX emIndex );

VOID GetCharSkillClassIndex( const EMCHARCLASS emClass, WORD & wBeginIndex, WORD & wEndIndex );

enum EMSTATS
{
	EMPOW	= 0,
	EMSTR	= 1,

	EMSPI	= 2,
	EMDEX	= 3,

	EMINT	= 4,
	EMSTA	= 5,

	EMSIZE	= 6,
};

struct FCHARSTATS
{
	float		fPow;	//	힘.
	float		fStr;	//	체력.
	float		fSpi;	//	정신.
	float		fDex;	//	민첩.
	float		fInt;	//	지력.
	float		fSta;	//	근력.

	FCHARSTATS () :
		fPow(0),
		fStr(0),
		fSpi(0),
		fDex(0),
		fInt(0),
		fSta(0)
	{
	}

	FCHARSTATS ( float _fPow, float _fStr, float _fSpi, float _fDex, float _fInt, float _fSta ) :
		fPow(_fPow),
		fStr(_fStr),
		fSpi(_fSpi),
		fDex(_fDex),
		fInt(_fInt),
		fSta(_fSta)
	{
	}

	float& GET ( EMSTATS emID )
	{
		switch(emID)
		{
		case EMPOW:	return fPow;
		case EMSTR:	return fStr;

		case EMSPI:	return fSpi;
		case EMDEX:	return fDex;

		case EMINT:	return fInt;
		case EMSTA:	return fSta;
		};

		static float fNULL=0.0f;
		return fNULL;
	}
};

struct SCHARSTATS
{
	WORD		wPow;	//	힘.
	WORD		wStr;	//	체력.
	WORD		wSpi;	//	정신.
	WORD		wDex;	//	민첩.
	WORD		wInt;	//	지력.
	WORD		wSta;	//	근력.		--(추가)

	SCHARSTATS () :
		wPow(0),
		wStr(0),
		wSpi(0),
		wDex(0),
		wInt(0),
		wSta(0)
	{
	}
	
	SCHARSTATS ( WORD _wPow, WORD _wStr, WORD _wSpi, WORD _wDex, WORD _wInt, WORD _wSta ) :
		wPow(_wPow),
		wStr(_wStr),
		wSpi(_wSpi),
		wDex(_wDex),
		wInt(_wInt),
		wSta(_wSta)
	{
	}

	//	Note : 현제의 수치가 요구수치에 부합되는지 검사.
	BOOL CHECK_REQ ( SCHARSTATS sREQ, WORD waccept=0 )
	{
		if ( wDex+waccept < sREQ.wDex )	return FALSE;
		if ( wInt+waccept < sREQ.wInt )	return FALSE;
		if ( wPow+waccept < sREQ.wPow )	return FALSE;
		if ( wSpi+waccept < sREQ.wSpi )	return FALSE;
		if ( wSta+waccept < sREQ.wSta )	return FALSE;
		if ( wStr+waccept < sREQ.wStr )	return FALSE;
		
		return TRUE;
	}

	void RESET ()
	{
		*this = SCHARSTATS();
	}

	WORD& GET ( EMSTATS emID )
	{
		switch(emID)
		{
		case EMPOW:	return wPow;
		case EMSTR:	return wStr;

		case EMSPI:	return wSpi;
		case EMDEX:	return wDex;

		case EMINT:	return wInt;
		case EMSTA:	return wSta;
		};

		static WORD wNULL=0;
		return wNULL;
	}

	DWORD GetTotal()
	{
		DWORD dwSUM_STATS = wPow + wStr + wSpi + wDex + wInt + wSta;
		return dwSUM_STATS;
	}

	bool IsZERO ()
	{
		WORD wSUM_STATS = wPow + wStr + wSpi + wDex + wInt + wSta;
		return wSUM_STATS==0;
	}
};
typedef SCHARSTATS*	PSCHARSTATS;

inline SCHARSTATS& operator + ( const SCHARSTATS& lvalue, const SCHARSTATS& rvalue )
{
	static SCHARSTATS sSTATS;
	sSTATS = lvalue;

	sSTATS.wPow	+= rvalue.wPow;
	sSTATS.wStr	+= rvalue.wStr;
	sSTATS.wSpi	+= rvalue.wSpi;
	sSTATS.wDex	+= rvalue.wDex;
	sSTATS.wInt	+= rvalue.wInt;
	sSTATS.wSta	+= rvalue.wSta;

	return sSTATS;
}

inline SCHARSTATS& operator * ( const SCHARSTATS& lvalue, const float rvalue )
{
	static SCHARSTATS sSTATS;
	sSTATS = lvalue;

	sSTATS.wPow	= int ( sSTATS.wPow * rvalue );
	sSTATS.wStr	= int ( sSTATS.wStr * rvalue );
	sSTATS.wSpi	= int ( sSTATS.wSpi * rvalue );
	sSTATS.wDex	= int ( sSTATS.wDex * rvalue );
	sSTATS.wInt	= int ( sSTATS.wInt * rvalue );
	sSTATS.wSta	= int (sSTATS. wSta * rvalue );

	return sSTATS;
}

inline SCHARSTATS& operator * ( const SCHARSTATS& lvalue, const int rvalue )
{
	static SCHARSTATS sSTATS;
	sSTATS = lvalue;

	sSTATS.wPow	= int ( sSTATS.wPow * rvalue );
	sSTATS.wStr	= int ( sSTATS.wStr * rvalue );
	sSTATS.wSpi	= int ( sSTATS.wSpi * rvalue );
	sSTATS.wDex	= int ( sSTATS.wDex * rvalue );
	sSTATS.wInt	= int ( sSTATS.wInt * rvalue );
	sSTATS.wSta	= int (sSTATS. wSta * rvalue );

	return sSTATS;
}

inline SCHARSTATS& operator + ( const SCHARSTATS& lvalue, const FCHARSTATS& rvalue )
{
	static SCHARSTATS sSTATS;
	sSTATS = lvalue;

	sSTATS.wPow	+= static_cast<WORD> ( rvalue.fPow );
	sSTATS.wStr	+= static_cast<WORD> ( rvalue.fStr );
	sSTATS.wSpi	+= static_cast<WORD> ( rvalue.fSpi );
	sSTATS.wDex	+= static_cast<WORD> ( rvalue.fDex );
	sSTATS.wInt	+= static_cast<WORD> ( rvalue.fInt );
	sSTATS.wSta	+= static_cast<WORD> ( rvalue.fSta );

	return sSTATS;
}

inline FCHARSTATS& operator * ( const FCHARSTATS& lvalue, const float rvalue )
{
	static FCHARSTATS sSTATS;
	sSTATS = lvalue;

	sSTATS.fPow	= ( sSTATS.fPow * rvalue );
	sSTATS.fStr	= ( sSTATS.fStr * rvalue );
	sSTATS.fSpi	= ( sSTATS.fSpi * rvalue );
	sSTATS.fDex	= ( sSTATS.fDex * rvalue );
	sSTATS.fInt	= ( sSTATS.fInt * rvalue );
	sSTATS.fSta	= (sSTATS. fSta * rvalue );

	return sSTATS;
}

inline FCHARSTATS& operator * ( const FCHARSTATS& lvalue, const int rvalue )
{
	static FCHARSTATS sSTATS;
	sSTATS = lvalue;

	sSTATS.fPow	= ( sSTATS.fPow * rvalue );
	sSTATS.fStr	= ( sSTATS.fStr * rvalue );
	sSTATS.fSpi	= ( sSTATS.fSpi * rvalue );
	sSTATS.fDex	= ( sSTATS.fDex * rvalue );
	sSTATS.fInt	= ( sSTATS.fInt * rvalue );
	sSTATS.fSta	= (sSTATS. fSta * rvalue );

	return sSTATS;
}


struct SSEARCHITEMDATA
{
	char		 szItemName[MAP_NAME_LENGTH]; // 아이템 이름
	DWORD		 dwReqCharClass;			  // 착용 클레스
	DWORD		 dwSuitType;				  // 아이템 타입
	WORD		 wReqLevel;					  // 착용 최소 레벨
	SCHARSTATS	 sReqStats;					  // 아이템 착용 스텟치
	LONGLONG	 llPRICE;					  // 아이템 가격
		
	BYTE		 RandOptTYPE1;				// 랜덤 옵션타입 1
	BYTE		 RandOptTYPE2;				// 랜덤 옵션타입 2
	BYTE		 RandOptTYPE3;				// 랜덤 옵션타입 3
	BYTE		 RandOptTYPE4;				// 랜덤 옵션타입 4

	float		 fRandOptValue1;			// 랜덤 옵션값 1	
	float		 fRandOptValue2;			// 랜덤 옵션값 2
	float		 fRandOptValue3;			// 랜덤 옵션값 3	
	float		 fRandOptValue4;			// 랜덤 옵션값 4


	BYTE		 cDAMAGE;					//	공격력 개조 등급.
	BYTE		 cDEFENSE;					//	방어력 개조 등급.

	BYTE		 cRESIST_FIRE;				//	저항(화) 개조 등급.
	BYTE		 cRESIST_ICE;				//	저항(빙) 개조 등급.
	BYTE		 cRESIST_ELEC;				//	저항(전) 개조 등급.
	BYTE		 cRESIST_POISON;			//	저항(독) 개조 등급.
	BYTE		 cRESIST_SPIRIT;			//	저항(정) 개조 등급.


	SSEARCHITEMDATA() 
		: llPRICE(0)
		, dwReqCharClass(0)
		, dwSuitType(0)
		, wReqLevel(0)

		, cDAMAGE(0)
		, cDEFENSE(0)
		, cRESIST_FIRE(0)

		, cRESIST_ICE(0)
		, cRESIST_ELEC(0)
		, cRESIST_POISON(0)
		, cRESIST_SPIRIT(0)

		, RandOptTYPE1(0)
		, RandOptTYPE2(0)
		, RandOptTYPE3(0)
		, RandOptTYPE4(0)

		, fRandOptValue1(0.0f)
		, fRandOptValue2(0.0f)
		, fRandOptValue3(0.0f)
		, fRandOptValue4(0.0f)

	{		
		memset(szItemName, 0, sizeof(char) * (MAP_NAME_LENGTH+1));
	}

};	


struct SSEARCHITEMRESULT
{
	char szShopName[MAP_NAME_LENGTH];
	D3DXVECTOR3 vShopPos;
	SNATIVEID	nSearchItemID;
	LONGLONG	llPRICE;

	SSEARCHITEMRESULT() :
		nSearchItemID(false),
		llPRICE(0),
		vShopPos( 0.0f, 0.0f, 0.0f )
	{
		memset(szShopName, 0, sizeof(char) * (MAP_NAME_LENGTH+1));
	}

};	

enum EMELEMENT
{
	EMELEMENT_SPIRIT		= 0,
	
	EMELEMENT_FIRE			= 1,
	EMELEMENT_ICE			= 2,
	EMELEMENT_ELECTRIC		= 3,
	EMELEMENT_POISON		= 4,	
	EMELEMENT_OLDMAX		= 5,	// 예전 최대치
	
	EMELEMENT_STONE			= 5,	
	EMELEMENT_MAD			= 6,
	EMELEMENT_STUN			= 7,
	EMELEMENT_CURSE			= 8,	
	EMELEMENT_MAXNUM		= 9,	// New
	
	EMELEMENT_ARM			= 9,
	//	element 속성이 무기에 종속적일 경우.

	EMELEMENT_MAXNUM2		= 10,
};


//	Note : 몹 100 버전의 정보에서 사용됨.
struct SRESIST_100
{	
	int		nFire;		//	불.
	int		nIce;		//	얼음.
	
	int		nElectric;	//	전기.
	int		nPoison;	//	독.

	int		nDivine;	//	독.

	SRESIST_100 () :
		nFire(0),
		nIce(0),

		nElectric(0),
		nPoison(0),
		nDivine(0)
	{
	}
};

struct SRESIST_101
{	
	short	nFire;		//	불.
	short	nIce;		//	얼음.
	
	short	nElectric;	//	전기.
	short	nPoison;	//	독.
};

struct SRESIST
{	
	short	nFire;		//	불.
	short	nIce;		//	얼음.
	
	short	nElectric;	//	전기.
	short	nPoison;	//	독.

	short	nSpirit;

	SRESIST () :
		nFire(0),
		nIce(0),

		nElectric(0),
		nPoison(0),

		nSpirit(0)
	{
	}

	void RESET ()
	{
		*this = SRESIST();
	}

	short GetElement ( EMELEMENT emElement ) const
	{
		switch ( emElement )
		{
		case EMELEMENT_SPIRIT:		return nSpirit;
		case EMELEMENT_FIRE:		return nFire;
		case EMELEMENT_ICE:			return nIce;
		case EMELEMENT_ELECTRIC:	return nElectric;
		case EMELEMENT_POISON:		return nPoison;
		case EMELEMENT_STONE:		return nFire;
		case EMELEMENT_MAD:			return nIce;
		case EMELEMENT_STUN:		return nElectric;
		case EMELEMENT_CURSE:		return nPoison;
		};

		return 0;
	}

	SRESIST& operator + ( const SRESIST &rvalue )
	{
		static SRESIST sREG;
		sREG = *this;

		sREG.nFire += rvalue.nFire;
		sREG.nIce += rvalue.nIce;
	
		sREG.nElectric += rvalue.nElectric;
		sREG.nPoison += rvalue.nPoison;

		sREG.nSpirit += rvalue.nSpirit;

		return sREG;
	}

	SRESIST& operator += ( const SRESIST &rvalue )
	{
		nFire += rvalue.nFire;
		nIce += rvalue.nIce;
	
		nElectric += rvalue.nElectric;
		nPoison += rvalue.nPoison;

		nSpirit += rvalue.nSpirit;

		return *this;
	}

	SRESIST& operator + ( const int rvalue )
	{
		static SRESIST sREG;
		sREG = *this;

		sREG.nFire += rvalue;
		sREG.nIce += rvalue;
	
		sREG.nElectric += rvalue;
		sREG.nPoison += rvalue;

		sREG.nSpirit += rvalue;

		return sREG;
	}

	SRESIST& operator += ( const int rvalue )
	{
		nFire += rvalue;
		nIce += rvalue;
	
		nElectric += rvalue;
		nPoison += rvalue;

		nSpirit += rvalue;

		return *this;
	}

	void Assign ( const SRESIST_101 &sRESIST )
	{
		nFire = sRESIST.nFire;
		nIce = sRESIST.nIce;
	
		nElectric = sRESIST.nElectric;
		nPoison = sRESIST.nPoison;
	}

	void LIMIT()
	{
		if ( nFire < 0 ) nFire = 0;
		if ( nIce < 0 ) nIce = 0;
		if ( nElectric < 0 ) nElectric = 0;
		if ( nPoison < 0 ) nPoison = 0;
		if ( nSpirit < 0 ) nSpirit = 0;
	}
};
typedef SRESIST*	PSRESIST;

enum EMACTIONTYPE
{
	GLAT_IDLE		= 0,
	GLAT_MOVE		= 1,
	GLAT_ATTACK		= 2,
	GLAT_SKILL		= 3,
	GLAT_SHOCK		= 4,
	GLAT_PUSHPULL	= 5, 
	GLAT_SKILLMOVE	= 6, /*push pull skill logic, Juver, 2017/06/05 */
	GLAT_GATHERING	= 7,
	GLAT_TALK		= 8,
	GLAT_CONFT_END	= 9,
	GLAT_FALLING	= 10,
	GLAT_DIE		= 11,
	GLAT_SKILLDASH	= 12, /*dash skill logic, Juver, 2017/06/17 */

	GLAT_SIZE		= 13,
};
typedef EMACTIONTYPE*		PEMACTIONTYPE;

enum EMTRIBE
{
	TRIBE_HUMAN			= 0,	//	인간.
	TRIBE_MONSTER		= 1,	//	괴물.
	TRIBE_ANGEL			= 2,	//	천사.
	TRIBE_DEVIL			= 3,	//	악마.
	TRIBE_SPIRIT		= 4,	//	정령.
	TRIBE_FAIRY			= 5,	//	요정.
	TRIBE_DRAGON		= 6,	//	드래곤.

	TRIBE_NSIZE			= 7		
};

enum EMBRIGHT
{
	BRIGHT_LIGHT		= 0,
	BRIGHT_DARK			= 1,
	
	BRIGHT_BOTH			= 2,	//	모두 가능. ( item 항목에서. )

	BRIGHT_SIZE			= 3,
};

enum EM_BRIGHT_FB
{
	BFB_DIS				= 0,	//	불리.
	BFB_AVER			= 1,	//	보통.
	BFB_ADV				= 2,	//	유리.

	BFB_SIZE			= 3,
};

enum EMMOVETYPE
{
	MOVE_LAND			= 0,	//	지면 이동.
	MOVE_FLY			= 1,	//	공중 이동.
	MOVE_SWIM			= 2,	//	수면 이동.

	MOVETYPE_NSIZE		= 3
};

enum EMIMPACT_TAR
{
	TAR_SELF		= 0,		//	자기 위치 영향.
	TAR_SPEC		= 1,		//	대상 위치 영향.
	TAR_SELF_TOSPEC	= 2,		//	자기 위치에서 대상 위치까지. ( 화살 관통 )
	TAR_ZONE		= 3,		//	위치 영향.
	TAR_SPECIFIC	= 4,

	TAR_SIZE		= 5,
};

enum EMIMPACT_REALM
{
	REALM_SELF		= 0,		//	'목표' 자신.
	REALM_ZONE		= 1,		//	'목표' 주위.
	REALM_KEEP_ZONE	= 2,		//	'목표' 주위 지속.
	REALM_FANWIZE	= 3,		//	'목표'를 향한 부채꼴 모양.

	REALM_SIZE		= 4
};

enum EMIMPACT_SIDE
{
	SIDE_OUR		= 0,		//	자기편에게.
	SIDE_ENEMY		= 1,		//	적에게.
	SIDE_ANYBODY	= 2,		//	모두에게.

	SIDE_SIZE		= 3,
};

enum EMDISORDER
{
	DIS_NULL			= 0x00,	// 없음
	DIS_NUMB			= 0x01,	// 마비.
	DIS_STUN			= 0x02,	// 기절.
	DIS_STONE			= 0x04,	// 석화.
	DIS_BURN			= 0x08,	// 화염.
	DIS_FROZEN			= 0x10,	// 냉동.

	DIS_MAD				= 0x20,	// 착란.
	DIS_POISON			= 0x40,	// 중독.
	DIS_CURSE			= 0x80,	// 저주.

	DIS_ALL				= (DIS_NUMB | DIS_STUN | DIS_STONE | DIS_BURN | DIS_FROZEN | DIS_MAD | DIS_POISON | DIS_CURSE),
};

//	상태 이상 효과.
//
enum EMSTATE_BLOW
{
	EMBLOW_NONE			= 0,	//	상태 이상 유발 없음.
	EMBLOW_NUMB			= 1,	//	마비.
	EMBLOW_STUN			= 2,	//	기절.
	EMBLOW_STONE		= 3,	//	석화.
	EMBLOW_BURN			= 4,	//	화염.
	EMBLOW_FROZEN		= 5,	//	냉동.
	EMBLOW_SINGLE		= 5,	//	단독으로만 발생하는 "상태이상" 효과.

	EMBLOW_MAD			= 6,	//	착란.
	EMBLOW_POISON		= 7,	//	중독.
	EMBLOW_CURSE		= 8,	//	저주.
	
	EMBLOW_SIZE			= 9,
	EMBLOW_MULTI		= 4,
};

inline EMDISORDER STATE_TO_DISORDER ( EMSTATE_BLOW emBLOW )
{
	switch ( emBLOW )
	{
	default:			return DIS_NULL;
	case EMBLOW_NUMB:	return DIS_NUMB;
	case EMBLOW_STUN:	return DIS_STUN;
	case EMBLOW_STONE:	return DIS_STONE;
	case EMBLOW_BURN:	return DIS_BURN;
	case EMBLOW_FROZEN:	return DIS_FROZEN;

	case EMBLOW_MAD:	return DIS_MAD;
	case EMBLOW_POISON:	return DIS_POISON;
	case EMBLOW_CURSE:	return DIS_CURSE;
	};
}

inline EMELEMENT STATE_TO_ELEMENT ( EMSTATE_BLOW emBLOW )
{
	switch ( emBLOW )
	{
	case EMBLOW_NUMB:	return EMELEMENT_ELECTRIC;
	case EMBLOW_STUN:	return EMELEMENT_STUN;
	case EMBLOW_STONE:	return EMELEMENT_STONE;
	case EMBLOW_BURN:	return EMELEMENT_FIRE;
	case EMBLOW_FROZEN:	return EMELEMENT_ICE;
	case EMBLOW_MAD:	return EMELEMENT_MAD;
	case EMBLOW_POISON:	return EMELEMENT_POISON;
	case EMBLOW_CURSE:	return EMELEMENT_CURSE;
	};

	return EMELEMENT_SPIRIT;
}

enum EMGROUP
{
	EMGROUP_ONE		= 0,
	EMGROUP_PARTY	= 1,
	EMGROUP_GUILD	= 2,

	EMGROUP_NSIZE	= 3,
};

enum EMIMPACT_ADDON
{
	EMIMPACTA_NONE			= 0,
	EMIMPACTA_HITRATE		= 1,	//	명중율 변화량.
	EMIMPACTA_AVOIDRATE		= 2,	//	회피율 변화량.
	
	EMIMPACTA_DAMAGE		= 3,	//	공격치 변화량.
	EMIMPACTA_DEFENSE		= 4,	//	방어치 변화량.

	EMIMPACTA_VARHP			= 5,	//	HP 변화율.
	EMIMPACTA_VARMP			= 6,	//	MP 변화율.
	EMIMPACTA_VARSP			= 7,	//	SP 변화율.
	EMIMPACTA_VARAP			= 8,	//	HP,MP,SP 변화율.

	EMIMPACTA_DAMAGE_RATE	= 9,	//	공격치 변화율.
	EMIMPACTA_DEFENSE_RATE	= 10,	//	방어치 변화율.

	EMIMPACTA_PA			= 11,	//	격투치.
	EMIMPACTA_SA			= 12,	//	사격치.
	EMIMPACTA_MA			= 13,	//	방어치.

	EMIMPACTA_HP_RATE		= 14,	//	HP 증폭율.
	EMIMPACTA_MP_RATE		= 15,	//	MP 증폭율.
	EMIMPACTA_SP_RATE		= 16,	//	SP 증폭율.

	EMIMPACTA_RESIST		= 17,	//	저항 수치 변경
	EMIMPACTA_CHANGESTATS	= 18,

	EMIMPACTA_HP_RECOVERY_VAR	= 19,
	EMIMPACTA_MP_RECOVERY_VAR	= 20,
	EMIMPACTA_SP_RECOVERY_VAR	= 21,
	EMIMPACTA_CP_RECOVERY_VAR	= 22,
	EMIMPACTA_CP_AUTO_VAR		= 23,

	EIMPACTA_SIZE			= 24
};

enum EMSPEC_ADDON
{
	EMSPECA_NULL			= 0,	//	기능 없음.
	EMSPECA_PUSHPULL		= 1,	//	당기기/밀기.
	EMSPECA_REFDAMAGE		= 2,	//	대미지 반사.
	EMSPECA_REBIRTH			= 3,	//	부활.
	
	EMSPECA_HP_GATHER		= 4,	//	HP의 일정량을 빼앗음.
	EMSPECA_MP_GATHER		= 5,	//	MP의 일정량을 빼앗음.
	EMSPECA_SP_GATHER		= 6,	//	SP의 일정량을 빼앗음.

	EMSPECA_HP_DIV			= 7,	//	HP의 일부를 나누어 줌.
	EMSPECA_MP_DIV			= 8,	//	MP의 일부를 나누어 줌.
	EMSPECA_SP_DIV			= 9,	//	SP의 일부를 나누어 줌.

	EMSPECA_NONBLOW			= 10,	//	상태이상 방지.
	EMSPECA_RECBLOW			= 11,	//	상태이상 회복.

	EMSPECA_PIERCE			= 12,	//	관통정도.
	EMSPECA_TARRANGE		= 13,	//	사정거리.

	EMSPECA_MOVEVELO		= 14,	//	이동속도.
	EMSPECA_ONWARD			= 15,	/*dash skill logic, Juver, 2017/06/17 */
	EMSPECA_INVISIBLE		= 16,	//	투명.
	EMSPECA_RECVISIBLE		= 17,	//	투명 확인.

	EMSPECA_ATTACKVELO		= 18,	//	공격속도.
	EMSPECA_SKILLDELAY		= 19,	//	스킬 딜래이 시간 변화.
	EMSPECA_CRUSHING_BLOW	= 20,	//  강한타격 스킬

	EMSPECA_PSY_DAMAGE_REDUCE		= 21,		// 물리 데미지 흡수율
	EMSPECA_MAGIC_DAMAGE_REDUCE		= 22,		// 매직 데미지 흡수율
	EMSPECA_PSY_DAMAGE_REFLECTION	= 23,		// 물리 데미지 반사율
	EMSPECA_MAGIC_DAMAGE_REFLECTION	= 24,		// 매직 데미지 반사율

	EMSPECA_BUFF_REMOVE				= 25, /*debuff skill logic, Juver, 2017/06/10 */ //debuff (remove buffs)
	EMSPECA_STUN					= 26, /*skill stun logic, Juver, 2017/06/06 */

	EMSPECA_DEFENSE_SKILL_ACTIVE	= 27,		//	피격시 발동 

	EMSPECA_PULL					= 28, /*push pull skill logic, Juver, 2017/06/04 */ //pull target
	EMSPECA_PUSH					= 29, /*push pull skill logic, Juver, 2017/06/04 */ //push target
	EMSPECA_CONTINUOUS_DAMAGE		= 30, /*continuous damage skill logic, Juver, 2017/06/10 */ //multiple damages by seconds
	EMSPECA_CURSE					= 31, /*curse skill logic, Juver, 2017/06/10 */ //return back percentage of damage
	EMSPECA_CHANGE_ATTACK_RANGE		= 32, /*skill range spec logic, Juver, 2017/06/06 */ //skill target range
	EMSPECA_CHANGE_APPLY_RANGE		= 33, /*skill range spec logic, Juver, 2017/06/06 */ //skill apply range
	EMSPECA_PROHIBIT_POTION			= 34, /*prohibit potion skill logic, Juver, 2017/06/06 */ //disable use potions
	EMSPECA_PROHIBIT_SKILL			= 35, /*prohibit skill logic, Juver, 2017/06/06 */ //disable use skills
	EMSPECA_IGNORE_DAMAGE			= 36, /*ignore damage, Juver, 2017/12/12 *///ignore any damage for sec

	EMSPECA_ITEMDROP_RATE		= 37,
	EMSPECA_MONEYDROP_RATE		= 38,
	EMSPECA_GETEXP_RATE			= 39,
	EMSPECA_TALK_TO_NPC			= 40, /*npc talk buff, Juver, 2017/10/03 */
	EMSPECA_SPECIAL_ITEM_DROP	= 41,
	EMSPECA_TELEPORT			= 42, //maybe with flag of swappos?
	EMSPECA_POSITION_SHIFT		= 43, //
	EMSPECA_ENHANCEMENT			= 44, //?
	EMSPECA_ILLUSION			= 45, //?
	EMSPECA_VEHICLE_OFF			= 46, //takedown from vehicle
	EMSPECA_DURATION_CHANGE		= 47, //?
	EMSPECA_STIGMA				= 48, //target recieve its own damage //
	EMSPECA_TRANSFORM			= 49, //done transformation // official uses transform lua for chf list
	EMSPECA_EFFECT_DURATION		= 50, //?
	EMSPECA_DAMAGE_LOOP			= 51, //get attacker hp delta ( max-now ) and transfer as damage to target //when target is mob act as hp steal
	EMSPECA_PET_OFF				= 52, //remove pet of target
	EMSPECA_IMMUNE				= 53, //not sure.. wont recieve attack and skillfact from enemy?
	EMSPECA_HOSTILE_ON			= 54, //
	EMSPECA_HOSTILE_OFF			= 55, //
	EMSPECA_TARGET_FIX			= 56, //
	EMSPECA_MANIPULATE			= 57, //control or manipulate target 
	EMSPECA_RANDOM_EXP_RATE		= 58,
	EMSPECA_SKILLLINK			= 59,
	EMSPECA_DAMAGE_VALUE				= 60,		//	공격력(정수).
	EMSPECA_DAMAGE_RATE				= 61,		//	공격력 % 증가.

	EMSPECA_DEFENSE_VALUE				= 62,		//	방어력(정수).
	EMSPECA_DEFENSE_RATE				= 63,		//	방어력 전체 % 증가.
	
	EMSPECA_HIT_VALUE					= 64,		//	명중율(정수).
	EMSPECA_HIT_RATE						= 65,		//	명중율 전체 % 증가

	EMSPECA_AVOID_VALUE				= 66,		//	회피율(정수).
	EMSPECA_AVOID_RATE					= 67,		//	회피율 전체 % 증가 
	
	EMSPECA_HP_VALUE						= 68,		//	hp 증가.
	EMSPECA_MP_VALUE						= 69,		//	mp 증가.
	EMSPECA_SP_VALUE						= 70,		//	sp 증가.
	EMSPECA_CP_VALUE						= 71,		//	cp 증가.  %처리되지 않음%

	EMSPECA_HP_VARIATION_RATE		= 72,		//	hp 변화율(특수효과).
	EMSPECA_MP_VARIATION_RATE		= 73,		//	mp 변화율(특수효과).
	EMSPECA_SP_VARIATION_RATE		= 74,		//	sp 변화율(특수효과).
	EMSPECA_HMS_VARIATION_RATE		= 75,		//	hp, mp, sp 변화율(특수효과).

	EMSPECA_ATTACK_RANGE_VALUE	= 76,		//	공격 영역.
	EMSPECA_DIS_SP_VALUE				= 77,		//	sp 소모량.
	EMSPECA_RESIST_VALUE				= 78,		//	저항.

	EMSPECA_STATS_POW_VALUE		= 79,		//	힘(정수).
	EMSPECA_STATS_STR_VALUE			= 80,		//	체력(정수).
	EMSPECA_STATS_SPI_VALUE			= 81,		//	정신(정수).
	EMSPECA_STATS_DEX_VALUE			= 82,		//	민첩(정수).
	EMSPECA_STATS_STA_VALUE		= 83,		//	근력(정수).

	EMSPECA_PA_VALUE						= 84,		//	격투치(정수).
	EMSPECA_SA_VALUE						= 85,		//	사격치(정수).
	EMSPECA_MA_VALUE					= 86,		//	마력치(정수).

	EMSPECA_HP_POTION_INC_VALUE	= 87,		//	포션 hp 증가량(정수).
	EMSPECA_MP_POTION_INC_VALUE	= 88,		//	포션 mp 증가량(정수).
	EMSPECA_SP_POTION_INC_VALUE	= 89,		//	포션 sp 증가량(정수).

	EMSPECA_CP_INC_VALUE				= 90,		//	몬스터 킬시 CP 증가량(정수).
	EMSPECA_MOVEVELO_VALUE			= 91,		// 이동속도 증가(정수)
	EMSPECA_DAMAGE_RATE_ALL			= 92,		// 데미지 (근거리, 사격 보정치 적용 받은 최종 수치에서 곱연산)
	EMSPECA_VIEW_RANGE_RATE			= 93,		// 시야 제한;
	EMSPECA_AIRBORNE				= 94,		// 에어본;

	EMSPECA_SUMMON_CHANGE_SKILLPAGE	= 95,	// 소환수 스킬 페이지 변경;
	EMSPECA_SUMMON_EVENT_SKILL		= 96,	// 소환수 이벤트 스킬;
	EMSPECA_SUMMON_TARGETING		= 97,	// 소환수 타겟 지정;
	EMSPECA_SUMMON_RUNAWAY			= 98,	// 소환수 도망;
	EMSPECA_DELAY_ACTION_SKILL		= 99,	// 지연발동
	EMSPECA_CLONE					= 100,	// 복제효과
	
    EMSPECA_MOVEVELO_LIMIT          = 101,  //  이동속도 제한;
	EMSPECA_COUNTATTACK				= 102,  // 반격

	EMSPECA_SCREEN_THROW			= 103,  // 화면투척
	EMSPECA_ENTRANCE_EXIT			= 104,  // 입출구
	EMSPECA_FEIGN_DEATH				= 105,  // 죽은척
	EMSPECA_CAMERA_HIT				= 106,  // 카메라치기
	EMSPECA_DISGUISE				= 107,  // 변장
	EMSPECA_PROVOKE					= 108,  // 도발
	EMSPECA_SPECIAL_IMMUNE			= 109,  // 특정효과면역
	EMSPECA_ACTIVATE_DAMAGE			= 110,  // 피해발동
	EMSPECA_REMOVE_SKILL_EFFECT		= 111,	// 스킬효과 제거 ( 각 효과별로 제거 );

	EMSPECA_NSIZE				= 60,
};

enum EMSPEC_INC_TYPE
{
    EMSPEC_INC_TYPE_PLUS  = 0,//  증가
    EMSPEC_INC_TYPE_MINUS   = 1,// 감소

    EMSPEC_INC_TYPE_NSIZE   = 2,
};


enum EMCHAR_ACTSTATE
{
	EM_ACT_RUN			= 0x00000001,		//	뛰기 플래그. ( off 시에는 것기. )
	EM_ACT_CONTINUEMOVE	= 0x00000002,		//	연속 이동 플래그.
	EM_ACT_PEACEMODE	= 0x00000004,		//	비경계 모드.
	EM_GETVA_AFTER		= 0x00000010,		//	GetViewAround() 확인 전인지 점검 플래그.
	EM_ACT_WAITING		= 0x00000020,		//	캐릭터 대기 모드 플래그.
	EM_ACT_DIE			= 0x00000040,		//	캐릭터 사망 모드 플래그.
	EM_REQ_GATEOUT		= 0x00000100,		//	맵 진입후 대기 플래그.
	EM_REQ_LOGOUT		= 0x00000200,		//	로그아웃 요청 플래그.

	EM_REQ_VISIBLENONE	= 0x00001000,		//	케릭터 전혀 출력하지 않음.
	EM_REQ_VISIBLEOFF	= 0x00002000,		//	케릭터 자신에게만 반투명하게 보이기.

	EM_ACT_CONFT_WIN	= 0x00010000,		//	대련 승리 플래그.

	EM_ACT_PK_MODE		= 0x00100000,		//	PK 모드 플래그.

	EM_ACT_VEHICLE_BOOSTER	= 0x01000000, /*vehicle booster system, Juver, 2017/08/12 */
};

enum EMSKILLCLASS
{
	EMSKILL_BRAWLER_01	=	0,
	EMSKILL_BRAWLER_02	=	1,
	EMSKILL_BRAWLER_03	=	2,
	EMSKILL_BRAWLER_04	=	3,

	EMSKILL_SWORDSMAN_01	=	4,
	EMSKILL_SWORDSMAN_02	=	5,
	EMSKILL_SWORDSMAN_03	=	6,
	EMSKILL_SWORDSMAN_04	=	7,

	EMSKILL_ARCHER_01	=	8,
	EMSKILL_ARCHER_02	=	9,
	EMSKILL_ARCHER_03	=	10,
	EMSKILL_ARCHER_04	=	11,

	EMSKILL_SHAMAN_01	=	12,
	EMSKILL_SHAMAN_02	=	13,
	EMSKILL_SHAMAN_03	=	14,
	EMSKILL_SHAMAN_04	=	15,

	EMSKILL_NPC_01		=	16,
	EMSKILL_NPC_02		=	17,
	EMSKILL_NPC_03		=	18,
	EMSKILL_NPC_04		=	19,
	EMSKILL_NPC_05		=	20,
	EMSKILL_NPC_06		=	21,
	EMSKILL_NPC_07		=	22,
	EMSKILL_NPC_08		=	23,
	EMSKILL_NPC_09		=	24,
	EMSKILL_NPC_10		=	25,

	EMSKILL_PET			=	26,
	EMSKILL_NPC_12		=	27,
	EMSKILL_NPC_13		=	28,
	EMSKILL_NPC_14		=	29,
	EMSKILL_EXTREME_01	=	30,
	EMSKILL_EXTREME_02	=	31,
	EMSKILL_EXTREME_03	=	32,
	EMSKILL_EXTREME_04	=	33,
	EMSKILL_NPC_19		=	34,
	EMSKILL_NPC_20		=	35,

	EMSKILL_GUNNER_01	=	36,
	EMSKILL_GUNNER_02	=	37,
	EMSKILL_GUNNER_03	=	38,
	EMSKILL_GUNNER_04	=	39,
	EMSKILL_FOOD		=	40,
	EMSKILL_SUMMON		=	41,
	EMSKILL_TRICKER_ANIMAL	=	42,

	EMSKILL_ASSASSIN_01	=	43,
	EMSKILL_ASSASSIN_02	=	44,
	EMSKILL_ASSASSIN_03	=	45,
	EMSKILL_ASSASSIN_04	=	46,

	EMSKILL_ETC_01		=	47,
	EMSKILL_ETC_02		=	48,
	EMSKILL_ETC_03		=	49,
	EMSKILL_ETC_04		=	50,

	EMSKILL_CTF			=	51,
	EMSKILL_EVENT		=	52,
	EMSKILL_ETC			=	53,
	EMSKILL_TEST		=	54,

	EMSKILL_TRICKER_MULTI	=	55,
	EMSKILL_TRICKER_01		=	56,
	EMSKILL_TRICKER_02		=	57,
	EMSKILL_TRICKER_03		=	58,
	EMSKILL_TRICKER_04		=	59,

	EMSKILLCLASS_NSIZE	=	60,
	EMSKILL_PC_CLASSNUM	=	6,
//	EMSKILL_PC_CLASSNUM	=	3,
};

//	Note : 대련 타입.
//
enum EMCONFT_TYPE
{
	EMCONFT_NONE	= 0,	//	없음. ( 지행하고 있지 않을때. )
	EMCONFT_ONE		= 1,	//	개인 대련.
	EMCONFT_PARTY	= 2,	//	파티 대련.
	EMCONFT_GUILD	= 3		//	길드 대련.
};

enum
{
	EMCONFT_COUNT	= 5,
};

//	Note : 대련 로그.
//
struct SCONFT_LOG
{
	DWORD	dwCOUNT;		//	대련 횟수.
	float	fTIMER;			//	최근 대련후 경과.

	SCONFT_LOG () :
		dwCOUNT(0),
		fTIMER(0.0f)
	{
	}
	SCONFT_LOG ( DWORD _dwcount, float _ftimer ) :
		dwCOUNT(_dwcount),
		fTIMER(_ftimer)
	{
	}
};

struct SCONFT_OPTION
{
	bool	bSCHOOL;	//	스쿨 대련인지 검사.
	bool	bBOUND;		//	영역 제한 여부.
	WORD	wRECOVER;	//	회복약 사용 갯수 제한.

	float	fHP_RATE;		//	체력 확대 비율.
	float	fTAR_HP_RATE;	//	체력 확대 비율 [ 상대방(개인대련시만) ].

	//	학교 대련시에만.
	WORD	wMY_SCHOOL;
	WORD	wTAR_SCHOOL;

	SCONFT_OPTION () 
		: bSCHOOL(false)
		, bBOUND(true)
		, wRECOVER(USHRT_MAX)
		, fHP_RATE(1.0f)
		, fTAR_HP_RATE(1.0f)
		, wMY_SCHOOL(0)
		, wTAR_SCHOOL(0)
	{
	}

	void RESET ()
	{
		bSCHOOL = (false);
		bBOUND = (true);
		wRECOVER = (USHRT_MAX);
		fHP_RATE = (1.0f);
		fTAR_HP_RATE = (1.0f);

		wMY_SCHOOL = (0);
		wTAR_SCHOOL = (0);
	}

	bool VALID_OPT () const
	{
		if ( fHP_RATE < 1.0f )			return false;
		if ( fHP_RATE > 10.0f )			return false;

		if ( fTAR_HP_RATE < 1.0f )		return false;
		if ( fTAR_HP_RATE > 10.0f )		return false;

		return true;
	}

	bool operator!= ( const SCONFT_OPTION &rvalue ) const
	{
		if ( bBOUND!= rvalue.bBOUND )				return true;
		if ( wRECOVER!=rvalue.wRECOVER )			return true;
		if ( fHP_RATE!=rvalue.fHP_RATE )			return true;
		if ( fTAR_HP_RATE!=rvalue.fTAR_HP_RATE )	return true;
		return false;
	}

	bool operator== ( const SCONFT_OPTION &rvalue ) const
	{
		if ( bBOUND!= rvalue.bBOUND )				return false;
		if ( wRECOVER!=rvalue.wRECOVER )			return false;
		if ( fHP_RATE!=rvalue.fHP_RATE )			return false;
		if ( fTAR_HP_RATE!=rvalue.fTAR_HP_RATE )	return false;
		return true;
	}
};

struct SCONFTING
{
	EMCONFT_TYPE	emTYPE;
	DWORD			dwTAR_ID;
	DWORD			dwCHARID;
	float			fTIMER;

	D3DXVECTOR3		vPosition;

	SCONFT_OPTION	sOption;
	WORD			wRECOVER;

	bool			bFIGHT;
	float			fTIMER_FIGHT;
	float			fPOWERFULTIME;

	GLDWDATA		sBACKUP_HP;
	GLDWDATA		sBACKUP_MP;
	GLDWDATA		sBACKUP_SP;

	SCONFTING () 
		: emTYPE(EMCONFT_NONE)
		, dwTAR_ID(GAEAID_NULL)
		, dwCHARID(0)
		, fTIMER(0.0f)
		, vPosition(FLT_MAX,FLT_MAX,FLT_MAX)
		, wRECOVER(0)
		, bFIGHT(false)
		, fTIMER_FIGHT(EMCONFT_COUNT)
		, fPOWERFULTIME(0.0f)
		, sBACKUP_HP(1,1)
		, sBACKUP_MP(1,1)
		, sBACKUP_SP(1,1)
	{
	}

	void RESET ()
	{
		emTYPE = (EMCONFT_NONE);
		dwTAR_ID = (GAEAID_NULL);
		dwCHARID = 0;
		fTIMER = (0.0f);
		fPOWERFULTIME = (0.0f);

		vPosition = D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX);

		sOption = SCONFT_OPTION();
		wRECOVER = 0;

		bFIGHT = false;
		fTIMER_FIGHT = EMCONFT_COUNT;
	}

	bool IsPOWERFULTIME () const
	{
		return ( fPOWERFULTIME > 0.0f );
	}

	bool IsCONFRONTING () const
	{
		return emTYPE != EMCONFT_NONE;
	}

	bool IsSAMENESS ( EMCONFT_TYPE _emtype, DWORD _dwtarid ) const
	{
		return _emtype==emTYPE && _dwtarid==dwTAR_ID;
	}

	bool IsFIGHTING () const
	{
		return bFIGHT;
	}

	bool IsRECOVE () const
	{
		if ( sOption.wRECOVER==USHRT_MAX )	return true;
		return ( wRECOVER < sOption.wRECOVER );
	}

	void SETPOWERFULTIME ( float fTime )
	{
		fPOWERFULTIME = fTime;
	} 

	void COUNTRECOVE ()
	{
		wRECOVER++;
	}

	bool UPDATE ( float fElaps )
	{
		if ( false==IsFIGHTING() )
		{
			fTIMER_FIGHT -= fElaps;
			return bFIGHT = ( fTIMER_FIGHT < 0.0f );
		}

		return false;
	}

	void UPDATEPWRFULTIME ( float fElaps )
	{
		fPOWERFULTIME -= fElaps;
		if ( fPOWERFULTIME < 0.0f )
			fPOWERFULTIME = 0.0f;
	}
};

struct SCONFTING_CLT
{
	typedef std::set<DWORD>		CONFT_MEM;
	typedef CONFT_MEM::iterator			CONFT_MEM_ITER;


	EMCONFT_TYPE	emTYPE;
	DWORD			dwTAR_ID;
	float			fTIMER;

	std::string		strTAR_NAME;
	CONFT_MEM		setConftMember;
	D3DXVECTOR3		vPosition;

	SCONFT_OPTION	sOption;
	WORD			wRECOVER;

	bool			bFIGHT;
	float			fTIMER_FIGHT;

	SCONFTING_CLT () :
		emTYPE(EMCONFT_NONE),
		dwTAR_ID(GAEAID_NULL),
		fTIMER(0.0f),
		vPosition(FLT_MAX,FLT_MAX,FLT_MAX),
		wRECOVER(0),

		bFIGHT(0),
		fTIMER_FIGHT(EMCONFT_COUNT)
	{
	}

	void RESET ()
	{
		emTYPE = (EMCONFT_NONE);
		dwTAR_ID = (GAEAID_NULL);
		fTIMER = (0.0f);

		strTAR_NAME.clear ();

		setConftMember.clear ();

		vPosition = D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX);

		sOption = SCONFT_OPTION();
		wRECOVER = 0;

		bFIGHT = false;
		fTIMER_FIGHT = EMCONFT_COUNT;
	}

	bool IsCONFRONTING () const
	{
		return emTYPE != EMCONFT_NONE;
	}

	const char* GETTARNAME() const	{ return strTAR_NAME.c_str(); }

	bool IsFIGHTING () const
	{
		return bFIGHT;
	}

	bool IsRECOVE () const
	{
		if ( sOption.wRECOVER==USHRT_MAX )	return true;
		return ( wRECOVER < sOption.wRECOVER );
	}

	void DoFIGHTING ()
	{
		bFIGHT = true;
	}

	void DEL_CONFT_MEMBER ( DWORD dwID )
	{
		CONFT_MEM_ITER pos = setConftMember.find(dwID);
		if ( pos!=setConftMember.end() )		setConftMember.erase(pos);
	}

	DWORD UPDATE ( float fElaps )
	{
		if ( false==IsFIGHTING() )
		{
			bool bCOUNT = (int(fTIMER_FIGHT-fElaps)<int(fTIMER_FIGHT)) && (int(fTIMER_FIGHT)>=0);
			
			if ( int(fTIMER_FIGHT)>=-1 )		fTIMER_FIGHT -= fElaps;
			else								fTIMER_FIGHT = -1.0f;

			if ( !bFIGHT && bCOUNT )	return int(fTIMER_FIGHT);
			return EMCONFT_COUNT;
		}

		return EMCONFT_COUNT;
	}
};

// 파티 아이템 분배 방식
enum EMPARTY_ITEM_OPT
{
	EMPTYITEMOPT_DEFAULT= 0,
	EMPTYITEMOPT_FREE	= 1, // 자유획득
	EMPTYITEMOPT_LEADER	= 2, // 파티 리더 우선권
	EMPTYITEMOPT_ROUND	= 3, // 순차습득
	EMPTYITEMOPT_SIZE	= 4
};

enum EMPARTY_MONEY_OPT
{
	EMPTYMONEYOPT_DEFAULT	= 0,
	EMPTYMONEYOPT_FREE		= 1, // 자유획득
	EMPTYMONEYOPT_LEADER	= 2, // 파티 리더 우선권
	EMPTYMONEYOPT_EQUAL		= 3, // 돈, 균등분배
	EMPTYMONEYOPT_SIZE		= 4
};


// 레벨 에디터에서 설정하는 특이 기능들
enum EMETCFUNC
{
	EMETCFUNC_LIMITTIME	  = 0, // 제한 시간
	EMETCFUNC_CONTROLCAM  = 1, // 카메라 제어
	EMETCFUNC_LANDEFFECT  = 2, // 지형 효과
	EMETCFUNC_DIFFICULTY  = 3, // 난이도 조절
	EMETCFUNC_AROUNDREGEN = 4, // 특정 몬스터 주변만 리젠
	EMETCFUNC_MOBCHASE	  = 5, // 모든 몬스터 해당 타겟이 죽을때까지 추적

	EMETCFUNC_SIZE,
};

enum EMLANDEFFECT_TYPE
{

	EMLANDEFFECT_ATK_SPEED		= 0,	//	공격속도
	EMLANDEFFECT_MOVE_SPEED		= 1,	//	이동속도

	EMLANDEFFECT_MP_RATE		= 2,	//	HP 변화율.
	EMLANDEFFECT_HP_RATE		= 3,	//	MP 변화율

	EMLANDEFFECT_RECOVER_RATE	= 4,	//	회복율
	EMLANDEFFECT_DAMAGE_RATE	= 5,	//	공격치 변화율
	EMLANDEFFECT_DEFENSE_RATE	= 6,	//	방어치 변화율
	EMLANDEFFECT_RESIST_RATE	= 7,	//	저항 수치

	EMLANDEFFECT_CANCEL_ALLBUFF	= 8,	//	모든 버프 취소
	EMLANDEFFECT_SAFE_ZONE		= 9,	//	모든 버프 취소

	EMLANDEFFECT_SLOT_ZONE		= 10,

	EMLANDEFFECT_SIZE			= 11,	//	세이프 존

	EMLANDEFFECT_MULTI			= 4,	//  최대 겹쳐지는 지형효과
};

struct SGETWHISPERLIST
{
	DWORD dwClientID;
	DWORD dwGaeaID;
};



struct SPARTY_OPT
{
	EMPARTY_ITEM_OPT emGET_ITEM;
	EMPARTY_MONEY_OPT emGET_MONEY;

	SPARTY_OPT () 
		: emGET_ITEM(EMPTYITEMOPT_DEFAULT)
		, emGET_MONEY(EMPTYMONEYOPT_DEFAULT)
	{
	}

	bool VALID_OPT () const
	{
		if ( emGET_ITEM < EMPTYITEMOPT_DEFAULT )		return false;
		if ( emGET_ITEM >= EMPTYITEMOPT_SIZE )		return false;

		if ( emGET_MONEY < EMPTYMONEYOPT_DEFAULT )	return false;
		if ( emGET_MONEY >= EMPTYMONEYOPT_SIZE )	return false;

		return true;
	}

	bool operator!= ( const SPARTY_OPT &rvalue ) const
	{
		if ( emGET_ITEM!= rvalue.emGET_ITEM )		return true;
		if ( emGET_MONEY!=rvalue.emGET_MONEY )		return true;
		return false;
	}

	bool operator== ( const SPARTY_OPT &rvalue ) const
	{
		if ( emGET_ITEM!= rvalue.emGET_ITEM )		return false;
		if ( emGET_MONEY!=rvalue.emGET_MONEY )		return false;
		return true;
	}
};

struct RANKING
{
	char	szName[CHAR_SZNAME+1];
	int		nChaNum;
	int		nOnline;
	DWORD	dwClub;
	DWORD	dwCharID;
	WORD	wLevel;
	LONGLONG lMoney;
	WORD	wKills;
	WORD	wDeaths;
	WORD	wSchool;
	WORD	wClass;
	bool	bRich;
	bool	bLevel;
	bool	bPK;
	DWORD		m_dwMarkVER;
	DWORD		m_aryMark[EMCLUB_MARK_SX*EMCLUB_MARK_SY];

	RANKING()
	{
		nChaNum	= 0;
		dwCharID = 0;
		nOnline = 0;
		wLevel	= 0;
		lMoney	= 0;
		wKills	= 0;
		wDeaths	= 0;
		wSchool	= 0;
		wClass	= 0;
		bRich = false;
		bLevel = false;
		bPK = false;
		dwClub = 0;
		m_dwMarkVER = 0;
		SecureZeroMemory( m_aryMark, sizeof(m_aryMark) );
		memset( szName, 0, sizeof(char) * (CHAR_SZNAME+1) );
	}

	RANKING( const RANKING &value ) 
    {
		operator=(value);
	}

	RANKING& operator= ( const RANKING &value )
	{
		nOnline = value.nOnline;
		dwCharID = value.dwCharID;
		nChaNum = value.nChaNum;
		wLevel = value.wLevel;
		lMoney = value.lMoney;
		wKills = value.wKills;
		wDeaths = value.wDeaths;
		wClass = value.wClass;
		wSchool = value.wSchool;
		dwClub = value.dwClub;
		bRich = value.bRich;
		bLevel = value.bLevel;
		bPK = value.bPK;
		m_dwMarkVER = value.m_dwMarkVER;
		memcpy( m_aryMark, value.m_aryMark, sizeof(m_aryMark) );
		StringCchCopy( szName, CHAR_SZNAME+1, value.szName );

		return *this;
	}

	bool operator < ( const RANKING& sRank )
	{			
		if ( bPK )
		{
			if ( wKills > sRank.wKills ) return true;
			else if ( wKills == sRank.wKills && wDeaths < sRank.wDeaths ) return true;
		}
		if ( bLevel ) if ( wLevel > sRank.wLevel ) return true;

		if ( bRich )  if ( lMoney > sRank.lMoney ) return true;

		return  false;
	}

	bool operator == ( const RANKING& sRank ) 
	{
		if ( wKills == sRank.wKills && wLevel == sRank.wLevel && wDeaths == sRank.wDeaths && lMoney == sRank.lMoney ) return true;
		return false;
	}
};

/*crow notice, Juver, 2017/11/25 */
enum EMCROW_NOTICE
{
	EMCROW_NOTICE_RESPAWN	= 0x00000001,
	EMCROW_NOTICE_RESPAWN2	= 0x00000002,
	EMCROW_NOTICE_DEFEAT	= 0x00000004,
	EMCROW_NOTICE_DEFEAT2	= 0x00000008,	
};

namespace COMMENT
{
	extern std::string ACTIONTYPE[GLAT_SIZE];

	extern std::string TRIBE[TRIBE_NSIZE];
	extern std::string MOVETYPE[MOVETYPE_NSIZE];
	extern std::string ELEMENT[EMELEMENT_MAXNUM2];

	extern std::string BRIGHT[BRIGHT_SIZE];

	extern std::string CHARCLASS[GLCI_NUM_8CLASS];
	extern std::string CHARCLASS2[GLCI2_NUM_8CLASS+1];
	extern std::string EMSCHOOL[GLSCHOOL_NUM];
	extern std::string EMGENDER[GLGENDER_NUM];

	extern std::string CHARCOMMENT[GLCI_NUM_8CLASS];

	extern std::string BLOW[EMBLOW_SIZE];

	extern std::string BLOW_VAR1[EMBLOW_SIZE];
	extern float BLOW_VAR1_SCALE[EMBLOW_SIZE];
	inline bool IsBLOW1_PER ( EMSTATE_BLOW emTYPE )			{ return BLOW_VAR1_SCALE[emTYPE]==100.0f; }
	
	extern std::string BLOW_VAR2[EMBLOW_SIZE];
	extern float BLOW_VAR2_SCALE[EMBLOW_SIZE];
	inline bool IsBLOW2_PER ( EMSTATE_BLOW emTYPE )			{ return BLOW_VAR2_SCALE[emTYPE]==100.0f; }

	extern std::string IMPACT_ADDON[EIMPACTA_SIZE];
	extern float IMPACT_ADDON_SCALE[EIMPACTA_SIZE];
	inline bool IsIMPACT_ADDON_PER ( EMIMPACT_ADDON emTYPE )	{ return IMPACT_ADDON_SCALE[emTYPE]==100.0f; }
	
	extern std::string SPEC_ADDON[EMSPECA_NSIZE];
	extern std::string SPEC_ADDON_VAR1[EMSPECA_NSIZE];
	extern float SPEC_ADDON_VAR1_SCALE[EMSPECA_NSIZE];
	inline bool IsSPEC_ADDON1_PER ( EMSPEC_ADDON emTYPE )		{ return SPEC_ADDON_VAR1_SCALE[emTYPE]==100.0f; }

	extern std::string SPEC_ADDON_VAR2[EMSPECA_NSIZE];
	extern float SPEC_ADDON_VAR2_SCALE[EMSPECA_NSIZE];
	inline bool IsSPEC_ADDON2_PER ( EMSPEC_ADDON emTYPE )		{ return SPEC_ADDON_VAR2_SCALE[emTYPE]==100.0f; }

	extern std::string IMPACT_TAR[TAR_SIZE];
	extern std::string IMPACT_REALM[REALM_SIZE];
	extern std::string IMPACT_SIDE[SIDE_SIZE];
	
    extern std::string SPEC_INC_TYPE[EMSPEC_INC_TYPE_NSIZE];

	extern std::string SKILLCLASS[EMSKILLCLASS_NSIZE];

	extern std::string LANDEFFECT_TYPE[EMLANDEFFECT_SIZE];
};

/*dmk14 daily quest*/
enum EMQUESTJOIN
{
	EMQJ_NONE = 0,
	EMQJ_ATTENDANCE,
	EMQJ_JOIN,
};

#endif // GLCHARDEFINE_H_
