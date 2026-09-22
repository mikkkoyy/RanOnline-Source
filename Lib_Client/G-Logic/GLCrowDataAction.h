#ifndef GLCROWDATA_ACTION_H_
#define GLCROWDATA_ACTION_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"

#include "../../Lib_Engine/Meshs/SAnimation.h"
#include "../../Lib_Engine/Meshs/DxSkinAniControl.h"
#include "./GLCharDefine.h"


enum EMCROW_NPCACT
{
	EMCROWACT_TARSHORT	= 0x00004,
	EMCROWACT_CDCERTIFY	= 0x00008,

	EMCROWACT_INVISIBLE	= 0x00010,
	EMCROWACT_RECVISIBLE= 0x00020,
	EMCROWACT_BARRIER	= 0x00040,
	EMCROWACT_POSHOLD	= 0x00080,
	EMCROWACT_DIRHOLD	= 0x00100,

	EMCROWACT_KNOCK		= 0x00200,
	EMCROWACT_BOSS		= 0x00400,
	EMCROWACT_BUSUNIT	= 0x01000,

	EMCROWACT_IGNORE_SHOCK	= 0x10000,

	EMCROWACT_AUTODROP	= 0x20000,
};

enum EMCROWACT_UP
{
	EMCROWACT_UP_IDLE			= 0,
	EMCROWACT_UP_FIRSTSTRIKE	= 1,
	EMCROWACT_UP_LOWLEVEL		= 2,
	EMCROWACT_UP_LOWHP			= 3,
	EMCROWACT_UP_BRIGHT			= 4,
	EMCROWACT_UP_DARK			= 5,
	EMCROWACT_UP_BLOW			= 6,
	EMCROWACT_UP_ARMER			= 7,
	EMCROWACT_UP_RUNNER			= 8,
	EMCROWACT_UP_ESCAPE			= 9,

	EMCROWACT_UP_NSIZE			= 10
};

enum EMCROWACT_DN
{
	EMCROWACT_DN_CONTINUE		= 0,
	EMCROWACT_DN_LOWHP			= 1,
	EMCROWACT_DN_ESCAPE			= 2,

	EMCROWACT_DN_NSIZE			= 3,
};

struct SCROWPATTERN
{
	float				m_fPatternDNRate;
	EMCROWACT_UP		m_emActPattern;
	DWORD				m_dwPatternAttackSet;        

	SCROWPATTERN (void) :
	m_fPatternDNRate( 0.0f ),
		m_emActPattern( EMCROWACT_UP_IDLE ),
		m_dwPatternAttackSet( 0 )
	{

	}
};

class CROW_PATTERN_CMP
{
public:
	bool operator () ( const SCROWPATTERN &lvalue, const SCROWPATTERN &rvalue )
	{
		if( lvalue.m_fPatternDNRate > rvalue.m_fPatternDNRate ) return true;
		return false;
	}
};

struct SCROWACTION_100
{
	WORD				m_wBodyRadius;					//	몸체원통 반경.
	char				m_szSkinObj[ACF_SZNAME];		//	스킨 파일.
	char				m_szTalkFile[ACF_SZNAME];		//	대화 파일.

	BOOL				m_bTrade;						//	거래가능한지.
	char				m_szSaleFile[ACF_SZNAME];		//	Item 판매 목록.

	BOOL				m_bAfterFall_NoBody;			//	죽는 순간에 몸자체 사라짐. ( 효과로 대채시에 유용 )
	char				m_szFallingEffect[ACF_SZNAME];	//	죽는 순간에 나오는 효과.
	char				m_szBlowEffect[ACF_SZNAME];		//	타격시 이팩트, CROW 종류별로 지정할 수 있게.

	DWORD				m_dwActionUP;					//	행동 성향 HP 50%이상.
	DWORD				m_dwActionDN;					//	행동 성향 HP 50%이하.

	EMMOVETYPE			m_emMoveType;					//	이동 타입.
	float				m_fDriftHeight;					//	부양 높이.

	float				m_fWalkVelo;					//	이동 속도.
	BOOL				m_bRun;							//	뛰기 가능.
	float				m_fRunVelo;						//	뛰기 속도.
};

struct SCROWACTION_101
{
	WORD				m_wBodyRadius;					//	몸체원통 반경.
	char				m_szSkinObj[ACF_SZNAME];		//	스킨 파일.
	char				m_szTalkFile[ACF_SZNAME];		//	대화 파일.

	DWORD				m_dwActFlag;					//	각종 행동.
	char				m_szSaleFile[ACF_SZNAME];		//	Item 판매 목록.

	BOOL				m_bAfterFall_NoBody;			//	죽는 순간에 몸자체 사라짐. ( 효과로 대채시에 유용 )
	char				m_szBirthEffect[ACF_SZNAME];	//	탄생시 나오는 효과.
	char				m_szFallingEffect[ACF_SZNAME];	//	죽는 순간에 나오는 효과.
	char				m_szBlowEffect[ACF_SZNAME];		//	타격시 이팩트, CROW 종류별로 지정할 수 있게.

	EMCROWACT_UP		m_emActionUP;					//	행동 성향 HP 50%이상.
	EMCROWACT_DN		m_emActionDN;					//	행동 성향 HP 50%이하.

	EMMOVETYPE			m_emMoveType;					//	이동 타입.
	float				m_fDriftHeight;					//	부양 높이.

	float				m_fWalkVelo;					//	이동 속도.
	BOOL				m_bRun;							//	뛰기 가능.
	float				m_fRunVelo;						//	뛰기 속도.
};

struct SCROWACTION_102
{
	enum { SALENUM = 3 };
	WORD				m_wBodyRadius;					//	몸체원통 반경.
	char				m_szSkinObj[ACF_SZNAME];		//	스킨 파일.
	char				m_szTalkFile[ACF_SZNAME];		//	대화 파일.

	DWORD				m_dwActFlag;					//	각종 행동.
	char				m_szSaleFile[SALENUM][ACF_SZNAME];		//	Item 판매 목록.

	BOOL				m_bAfterFall_NoBody;			//	죽는 순간에 몸자체 사라짐. ( 효과로 대채시에 유용 )
	char				m_szBirthEffect[ACF_SZNAME];	//	탄생시 나오는 효과.
	char				m_szFallingEffect[ACF_SZNAME];	//	죽는 순간에 나오는 효과.
	char				m_szBlowEffect[ACF_SZNAME];		//	타격시 이팩트, CROW 종류별로 지정할 수 있게.

	EMCROWACT_UP		m_emActionUP;					//	행동 성향 HP 50%이상.
	EMCROWACT_DN		m_emActionDN;					//	행동 성향 HP 50%이하.

	EMMOVETYPE			m_emMoveType;					//	이동 타입.
	float				m_fDriftHeight;					//	부양 높이.

	float				m_fWalkVelo;					//	이동 속도.
	BOOL				m_bRun;							//	뛰기 가능.
	float				m_fRunVelo;						//	뛰기 속도.
};

struct SCROWACTION_103
{
	enum { VERSION = 0x0103, SALENUM = 3 };

	WORD				m_wBodyRadius;					//	몸체원통 반경.
	std::string			m_strSkinObj;					//	스킨 파일.
	std::string			m_strTalkFile;					//	대화 파일.

	DWORD				m_dwActFlag;					//	각종 행동.
	std::string			m_strSaleFile[SALENUM];			//	Item 판매 목록.

	BOOL				m_bAfterFall_NoBody;			//	죽는 순간에 몸자체 사라짐. ( 효과로 대채시에 유용 )
	std::string			m_strBirthEffect;				//	탄생시 나오는 효과.
	std::string			m_strFallingEffect;				//	죽는 순간에 나오는 효과.
	std::string			m_strBlowEffect;					//	타격시 이팩트, CROW 종류별로 지정할 수 있게.

	EMCROWACT_UP		m_emActionUP;					//	행동 성향 HP 50%이상.
	EMCROWACT_DN		m_emActionDN;					//	행동 성향 HP 50%이하.

	EMMOVETYPE			m_emMoveType;					//	이동 타입.
	float				m_fDriftHeight;					//	부양 높이.

	float				m_fWalkVelo;					//	이동 속도.
	BOOL				m_bRun;							//	뛰기 가능.
	float				m_fRunVelo;						//	뛰기 속도.
};

struct SCROWACTION_104
{
	enum { VERSION = 0x0104, SALENUM = 3 };

	WORD				m_wBodyRadius;					//	몸체원통 반경.
	std::string			m_strSkinObj;					//	스킨 파일.
	std::string			m_strTalkFile;					//	대화 파일.

	DWORD				m_dwActFlag;					//	각종 행동.
	std::string			m_strSaleFile[SALENUM];			//	Item 판매 목록.

	BOOL				m_bAfterFall_NoBody;			//	죽는 순간에 몸자체 사라짐. ( 효과로 대채시에 유용 )
	std::string			m_strBirthEffect;				//	탄생시 나오는 효과.
	std::string			m_strFallingEffect;				//	죽는 순간에 나오는 효과.
	std::string			m_strBlowEffect;					//	타격시 이팩트, CROW 종류별로 지정할 수 있게.

	EMCROWACT_UP		m_emActionUP;					//	행동 성향 HP 50%이상.
	EMCROWACT_DN		m_emActionDN;					//	행동 성향 HP 50%이하.

	EMMOVETYPE			m_emMoveType;					//	이동 타입.
	float				m_fDriftHeight;					//	부양 높이.

	float				m_fWalkVelo;					//	이동 속도.
	BOOL				m_bRun;							//	뛰기 가능.
	float				m_fRunVelo;						//	뛰기 속도.

	float				m_fLiveTime;					//  활동가능한 시간(EMCROWACT_AUTODROP 플래그일 경우)
};

struct SCROWACTION_105
{
	enum { SALENUM = 3 };

	WORD				m_wBodyRadius;					//	몸체원통 반경.
	std::string			m_strSkinObj;					//	스킨 파일.
	std::string			m_strTalkFile;					//	대화 파일.

	DWORD				m_dwActFlag;					//	각종 행동.
	std::string			m_strSaleFile[SALENUM];			//	Item 판매 목록.

	BOOL				m_bAfterFall_NoBody;			//	죽는 순간에 몸자체 사라짐. ( 효과로 대채시에 유용 )
	std::string			m_strBirthEffect;				//	탄생시 나오는 효과.
	std::string			m_strFallingEffect;				//	죽는 순간에 나오는 효과.
	std::string			m_strBlowEffect;					//	타격시 이팩트, CROW 종류별로 지정할 수 있게.

	EMCROWACT_UP		m_emActionUP;					//	행동 성향 HP 50%이상.
	EMCROWACT_DN		m_emActionDN;					//	행동 성향 HP 50%이하.

	EMMOVETYPE			m_emMoveType;					//	이동 타입.
	float				m_fDriftHeight;					//	부양 높이.

	float				m_fWalkVelo;					//	이동 속도.
	BOOL				m_bRun;							//	뛰기 가능.
	float				m_fRunVelo;						//	뛰기 속도.

	float				m_fLiveTime;					//  활동가능한 시간(EMCROWACT_AUTODROP 플래그일 경우)
	float				m_fActionDNRate;				// 행동 성향 이하 %
};

struct SCROWACTION_106
{
	enum { SALENUM = 3 };

	WORD				m_wBodyRadius;					//	몸체원통 반경.
	std::string			m_strSkinObj;					//	스킨 파일.
	std::string			m_strTalkFile;					//	대화 파일.

	DWORD				m_dwActFlag;					//	각종 행동.
	std::string			m_strSaleFile[SALENUM];			//	Item 판매 목록.

	BOOL				m_bAfterFall_NoBody;			//	죽는 순간에 몸자체 사라짐. ( 효과로 대채시에 유용 )
	std::string			m_strBirthEffect;				//	탄생시 나오는 효과.
	std::string			m_strFallingEffect;				//	죽는 순간에 나오는 효과.
	std::string			m_strBlowEffect;					//	타격시 이팩트, CROW 종류별로 지정할 수 있게.

	EMCROWACT_UP		m_emActionUP;					//	행동 성향 HP 50%이상.
	EMCROWACT_DN		m_emActionDN;					//	행동 성향 HP 50%이하.

	EMMOVETYPE			m_emMoveType;					//	이동 타입.
	float				m_fDriftHeight;					//	부양 높이.

	float				m_fWalkVelo;					//	이동 속도.
	BOOL				m_bRun;							//	뛰기 가능.
	float				m_fRunVelo;						//	뛰기 속도.

	float				m_fLiveTime;					//  활동가능한 시간(EMCROWACT_AUTODROP 플래그일 경우)
	float				m_fActionDNRate;				// 행동 성향 이하 %

	BOOL				m_bMobLink;						// 해당몬스터가 죽으면 연결되어있는 몬스터를 호출할지 여부.
	SNATIVEID			m_sMobLinkID;					// 연결된 몬스터의 ID
};

struct SCROWACTION_107
{
	enum { SALENUM = 3, PATTERNNUM = 10 };

	WORD				m_wBodyRadius;					//	몸체원통 반경.
	std::string			m_strSkinObj;					//	스킨 파일.
	std::string			m_strTalkFile;					//	대화 파일.

	DWORD				m_dwActFlag;					//	각종 행동.
	std::string			m_strSaleFile[SALENUM];			//	Item 판매 목록.

	BOOL				m_bAfterFall_NoBody;			//	죽는 순간에 몸자체 사라짐. ( 효과로 대채시에 유용 )
	std::string			m_strBirthEffect;				//	탄생시 나오는 효과.
	std::string			m_strFallingEffect;				//	죽는 순간에 나오는 효과.
	std::string			m_strBlowEffect;					//	타격시 이팩트, CROW 종류별로 지정할 수 있게.

	EMCROWACT_UP		m_emActionUP;					//	행동 성향 HP 50%이상.
	EMCROWACT_DN		m_emActionDN;					//	행동 성향 HP 50%이하.

	EMMOVETYPE			m_emMoveType;					//	이동 타입.
	float				m_fDriftHeight;					//	부양 높이.

	float				m_fWalkVelo;					//	이동 속도.
	BOOL				m_bRun;							//	뛰기 가능.
	float				m_fRunVelo;						//	뛰기 속도.

	float				m_fLiveTime;					//  활동가능한 시간(EMCROWACT_AUTODROP 플래그일 경우)
	float				m_fActionDNRate;				// 행동 성향 이하 %

	BOOL				m_bMobLink;						// 해당몬스터가 죽으면 연결되어있는 몬스터를 호출할지 여부.
	SNATIVEID			m_sMobLinkID;					// 연결된 몬스터의 ID

	SCROWPATTERN		m_Pattern;						
	std::vector< SCROWPATTERN >	m_vecPatternList;
};

struct SCROWACTION_108
{
	enum { SALENUM = 3, PATTERNNUM = 10 };

	WORD				m_wBodyRadius;					//	몸체원통 반경.
	std::string			m_strSkinObj;					//	스킨 파일.
	std::string			m_strTalkFile;					//	대화 파일.

	DWORD				m_dwActFlag;					//	각종 행동.
	std::string			m_strSaleFile[SALENUM];			//	Item 판매 목록.

	BOOL				m_bAfterFall_NoBody;			//	죽는 순간에 몸자체 사라짐. ( 효과로 대채시에 유용 )
	std::string			m_strBirthEffect;				//	탄생시 나오는 효과.
	std::string			m_strFallingEffect;				//	죽는 순간에 나오는 효과.
	std::string			m_strBlowEffect;					//	타격시 이팩트, CROW 종류별로 지정할 수 있게.

	EMCROWACT_UP		m_emActionUP;					//	행동 성향 HP 50%이상.
	EMCROWACT_DN		m_emActionDN;					//	행동 성향 HP 50%이하.

	EMMOVETYPE			m_emMoveType;					//	이동 타입.
	float				m_fDriftHeight;					//	부양 높이.

	float				m_fWalkVelo;					//	이동 속도.
	BOOL				m_bRun;							//	뛰기 가능.
	float				m_fRunVelo;						//	뛰기 속도.

	float				m_fLiveTime;					//  활동가능한 시간(EMCROWACT_AUTODROP 플래그일 경우)
	float				m_fActionDNRate;				// 행동 성향 이하 %

	BOOL				m_bMobLink;						// 해당몬스터가 죽으면 연결되어있는 몬스터를 호출할지 여부.
	SNATIVEID			m_sMobLinkID;					// 연결된 몬스터의 ID
	float				m_fMobLinkScale;				// 연결된 몬스터 몸체 크기
	float				m_fMobLinkDelay;				// 연결된 몬스터 리젠 딜레이

	SCROWPATTERN		m_Pattern;						
	std::vector< SCROWPATTERN >	m_vecPatternList;
};

struct SCROWACTION_200
{
	enum { SALENUM = 3, PATTERNNUM = 10 };

	DWORD				m_dwActFlag;					//	각종 행동.
	EMMOVETYPE			m_emMoveType;					//	이동 타입.
	float				m_fDriftHeight;					//	부양 높이.
	float				m_fWalkVelo;					//	이동 속도.
	BOOL				m_bRun;							//	뛰기 가능.
	float				m_fRunVelo;						//	뛰기 속도.
	WORD				m_wBodyRadius;					//	몸체원통 반경.
	std::string			m_strSkinObj;					//	스킨 파일.
	std::string			m_strTalkFile;					//	대화 파일.
	BOOL				m_bAfterFall_NoBody;			//	죽는 순간에 몸자체 사라짐. ( 효과로 대채시에 유용 )
	std::string			m_strBirthEffect;				//	탄생시 나오는 효과.
	std::string			m_strFallingEffect;				//	죽는 순간에 나오는 효과.
	std::string			m_strBlowEffect;				//	타격시 이팩트, CROW 종류별로 지정할 수 있게.
	std::string			m_strSaleFile[SALENUM];			//	Item 판매 목록.
	std::string			m_strShopFile;					/*npc shop, Juver, 2017/07/25 */
	float				m_fLiveTime;					//  활동가능한 시간(EMCROWACT_AUTODROP 플래그일 경우)
	BOOL				m_bMobLink;						// 해당몬스터가 죽으면 연결되어있는 몬스터를 호출할지 여부.
	SNATIVEID			m_sMobLinkID;					// 연결된 몬스터의 ID
	float				m_fMobLinkScale;				// 연결된 몬스터 몸체 크기
	float				m_fMobLinkDelay;				// 연결된 몬스터 리젠 딜레이

	std::vector< SCROWPATTERN >	m_vecPatternList;
};

struct SCROWACTION
{
	enum { VERSION = 0x0201, SALENUM = 3, PATTERNNUM = 10, ACTION_MAXATTACK = 10 };

	DWORD				m_dwActFlag;					//	각종 행동.
	EMMOVETYPE			m_emMoveType;					//	이동 타입.
	float				m_fDriftHeight;					//	부양 높이.
	float				m_fWalkVelo;					//	이동 속도.
	BOOL				m_bRun;							//	뛰기 가능.
	float				m_fRunVelo;						//	뛰기 속도.
	WORD				m_wBodyRadius;					//	몸체원통 반경.
	std::string			m_strSkinObj;					//	스킨 파일.
	std::string			m_strTalkFile;					//	대화 파일.
	BOOL				m_bAfterFall_NoBody;			//	죽는 순간에 몸자체 사라짐. ( 효과로 대채시에 유용 )
	std::string			m_strBirthEffect;				//	탄생시 나오는 효과.
	std::string			m_strFallingEffect;				//	죽는 순간에 나오는 효과.
	std::string			m_strBlowEffect;				//	타격시 이팩트, CROW 종류별로 지정할 수 있게.
	std::string			m_strSaleFile[SALENUM];			//	Item 판매 목록.
	std::string			m_strShopFile;					/*npc shop, Juver, 2017/07/25 */
	float				m_fLiveTime;					//  활동가능한 시간(EMCROWACT_AUTODROP 플래그일 경우)
	BOOL				m_bMobLink;						// 해당몬스터가 죽으면 연결되어있는 몬스터를 호출할지 여부.
	SNATIVEID			m_sMobLinkID;					// 연결된 몬스터의 ID
	float				m_fMobLinkScale;				// 연결된 몬스터 몸체 크기
	float				m_fMobLinkDelay;				// 연결된 몬스터 리젠 딜레이
	
	std::vector< SCROWPATTERN >	m_vecPatternList;

	std::string			m_strItemExchangeFile;			/*item exchange, Juver, 2017/10/11 */

	SCROWACTION ( void ) :
		m_dwActFlag( NULL ),
		m_emMoveType( MOVE_LAND ),
		m_fDriftHeight( 0.0f ),
		m_fWalkVelo( 12.0f ),
		m_bRun( FALSE ),
		m_fRunVelo( 34.0f ),
		m_wBodyRadius( 4 ),
		m_bAfterFall_NoBody( FALSE ),
		m_fLiveTime( 0.0f ),
		m_bMobLink( FALSE ),
		m_sMobLinkID( NATIVEID_NULL() ),
		m_fMobLinkScale( 0.0f ),
		m_fMobLinkDelay( 0.0f )
	{
	}

	SCROWACTION ( const SCROWACTION &value )
	{
		operator = ( value );
	}

	SCROWACTION& operator= ( const SCROWACTION_100 &OldData );
	SCROWACTION& operator= ( const SCROWACTION_101 &OldData );
	SCROWACTION& operator= ( const SCROWACTION_102 &OldData );
	SCROWACTION& operator= ( const SCROWACTION &value );

	bool LOAD103 ( basestream &SFile );
	bool LOAD104 ( basestream &SFile );
	bool LOAD105 ( basestream &SFile );
	bool LOAD106 ( basestream &SFile );
	bool LOAD107 ( basestream &SFile );
	bool LOAD108 ( basestream &SFile );
	bool LOAD110 ( basestream &SFile );
	bool LOAD200 ( basestream &SFile );

	bool LOAD ( basestream &SFile );
	bool SAVE ( CSerialFile &SFile );

	static VOID SaveCsvHead ( std::fstream &SFile );
	VOID SaveCsv ( std::fstream &SFile );
	VOID LoadCsv ( CStringArray &StrArray, int& iCsvCur );
};

namespace COMMENT
{
	extern std::string szCROWACT_UP[EMCROWACT_UP_NSIZE];
	extern std::string szCROWACT_DN[EMCROWACT_DN_NSIZE];
};

#endif // GLCROWDATA_ACTION_H_