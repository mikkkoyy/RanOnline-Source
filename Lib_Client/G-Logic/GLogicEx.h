#ifndef GLOGIXEX_H_
#define GLOGIXEX_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// GLCHARLOGIC, GLCROWLOGIC
//	두 클래스가 공통으로 가지는 함수들은 데이터의 근원 "케릭터 데이터", "NPC 데이터" 에서
//	각각 받아서 같은 내용을 함수로 제작하게됨. 실제 함수 부분은 각각 따로 있음. ( 동작 효율성 고려. )
//	단, 함수 내용은 동일하게 유지. ( 다른 데이터 근원에서 가져오더라도 인라인 함수 정의를 동일하게하여 맞춤. )
//
//
//
//
#define MOVE_OPTION_SHOW_EFF			0x00000001
#define MOVE_OPTION_NOT_TURN_IDLE		0x00000002

#include <map>

#include "./GLSKILL.h"
#include "./GLCharData.h"
#include "./GLCrowData.h"
#include "./GLogicData.h"
#include "./GLContrlMsg.h"
#include "./GLPVPTyrannyDefine.h" /*pvp tyranny, Juver, 2017/08/25 */

/*activity system, Juver, 2017/10/30 */
#include "./GLActivityData.h"

#ifndef GLSKILL
	struct GLSKILL;
#endif //GLSKILL

typedef std::map<DWORD,float>	DELAY_MAP;
typedef DELAY_MAP::iterator				DELAY_MAP_ITER;

class STARDIST
{
public:
	float		m_fDIST;
	STARGETID	m_sTARGETID;

public:
	STARDIST () {}

	STARDIST ( const float fDIST, const STARGETID &sTARID )
	{
		m_fDIST = fDIST;
		m_sTARGETID = sTARID;
	}

	bool operator < ( const STARDIST &rvalue )
	{
		return m_fDIST < rvalue.m_fDIST;
	}
};

typedef std::vector<STARDIST>			TARGETMAP;
typedef TARGETMAP::iterator				TARGETMAP_ITER;

typedef std::vector<STARGETID>						DETECTMAP;
typedef DETECTMAP::iterator							DETECTMAP_ITER;
typedef std::pair<DETECTMAP_ITER,DETECTMAP_ITER>	DETECTMAP_RANGE;

class STAR_ORDER
{
public:
	bool operator() ( const STARGETID &lvalue, const STARGETID &rvalue )
	{
		return lvalue.emCrow < rvalue.emCrow;
	}

	bool operator() ( const STARGETID &lvalue, const EMCROW emCROW )
	{
		return lvalue.emCrow < emCROW;
	}

	bool operator() ( const EMCROW emCROW, const STARGETID &rvalue )
	{
		return emCROW < rvalue.emCrow;
	}
};

//	적대자.
struct SPLAYHOSTILE
{
	BOOL	bBAD;
	float	fTIME;

	SPLAYHOSTILE ()
	{
		bBAD     = FALSE;
		fTIME    = 0;
	}
};

typedef std::map<DWORD,SPLAYHOSTILE*>	MAPPLAYHOSTILE;
typedef MAPPLAYHOSTILE::iterator				MAPPLAYHOSTILE_ITER;

struct SSUM_ITEM
{
	GLDWDATA	gdDamage;
	int			nDefense;

	int			nAvoidRate;
	int			nHitRate;

	int			nHP;
	int			nMP;
	int			nSP;

	float		fIncR_HP;
	float		fIncR_MP;
	float		fIncR_SP;

	float		fInc_HP;
	float		fInc_MP;
	float		fInc_SP;

	SCHARSTATS	sStats;
	SRESIST		sResist;

	int			nPA;
	int			nSA;
	int			nMA;

	float		fIncR_MoveSpeed;
	float		fIncR_AtkSpeed;
	float		fIncR_Critical;
	float		fIncR_CrushingBlow;

	float		fInc_MoveSpeed;
	float		fInc_AtkSpeed;
	float		fInc_Critical;
	float		fInc_CrushingBlow;
	
	DAMAGE_SPEC m_sDamageSpec;

	//new itemvar
	float		fDecR_Damage_MeleeSkill; // 물리 근접 스킬 피격시 경우 데미지 감소;
	float		fDecR_Damage_RangeSkill; // 사격 스킬 피격시 " ;
	float		fDecR_Damage_MagicSkill; // 마법 스킬 피격시 " ;

	//add directval
	int		nHP_Potion_Rate;
	int		nMP_Potion_Rate;
	int		nSP_Potion_Rate;

	//add directval hitrate and avoidrate
	float		fRateHit_Per;
	float		fRateAvoid_Per;


	SSUM_ITEM () :
		nDefense(0),

		nAvoidRate(0),
		nHitRate(0),
		nHP(0),
		nMP(0),
		nSP(0),
		fIncR_HP(0.0f),
		fIncR_MP(0.0f),
		fIncR_SP(0.0f),

		fInc_HP(0.0f),
		fInc_MP(0.0f),
		fInc_SP(0.0f),

		nPA(0),
		nSA(0),
		nMA(0),
		fIncR_MoveSpeed(0.0f),
		fIncR_AtkSpeed(0.0f),
		fIncR_Critical(0.0f),
		fIncR_CrushingBlow(0.0f),
		fInc_MoveSpeed(0.0f),
		fInc_AtkSpeed(0.0f),
		fInc_Critical(0.0f),
		fInc_CrushingBlow(0.0f),
		nHP_Potion_Rate(0), //add directval
		nMP_Potion_Rate(0),//add directval
		nSP_Potion_Rate(0),//add directval
		fRateHit_Per(0.0f), //add directval
		fRateAvoid_Per(0.0f),//add directval
		fDecR_Damage_MeleeSkill(0.0f),
		fDecR_Damage_RangeSkill(0.0f),
		fDecR_Damage_MagicSkill(0.0f)
	{
	}

	void RESET ()
	{
		*this = SSUM_ITEM();
	}
};

namespace GLOGICEX
{
	EM_BRIGHT_FB GLSPACEGAP ( const EMBRIGHT emACTOR, const EMBRIGHT emRECEP, const EMBRIGHT emSPACE );

	//	공격 성공율 계산.
	int GLHITRATE ( const int nHit, const int nAvoid, const EM_BRIGHT_FB bFB );

	//	완전 쇼크 확율.
	BOOL CHECKSHOCK ( const int AttackerLev, const int DefenserLev, const int nDamage, const bool bCritical );

	//	방어력 계산.
	int GLDEFENSE ( const int nDEFENSE, const EMBRIGHT emACTOR, const EMBRIGHT emRecep, const EMBRIGHT emSPACE );

	//	경험치 획득 ( 타격시 )
	int GLATTACKEXP ( int AttackerLev, int DefenserLev, DWORD dwDamage, DWORD wMaxHP, DWORD dwBonusExp=0 );

	//	경험치 획득 ( 제거시 )
	int	GLKILLEXP ( int AttackerLev, int DefenserLev, DWORD dwBonusExp=0 );

	//	경험치 획득 ( 타격시 )
	int CALCATTACKEXP ( WORD wMYLEVEL, const STARGETID &cTargetID, const GLLandMan* pLandMan, DWORD dwDamage );
	
	//	경험치 획득 ( 제거시 )
	int CALCKILLEXP ( WORD wMYLEVEL, const STARGETID &cTargetID, const GLLandMan* pLandMan );

	//	죽었을때 경험치 감소.
	float GLDIE_DECEXP ( WORD wACTLEV );

	//	경험치 복구 비율
	float GLDIE_RECOVERYEXP ( WORD wACTLEV );

	//  경험치1당 돈
	float GLDIE_EXPMONEY ( WORD wACTLEV );

	//	LevelUp을 하기 위한 경험치
	LONGLONG GLNEEDEXP ( WORD wLev );
	LONGLONG GLNEEDEXP2 ( WORD wLev );

	//	상태이상 발생할 수 있는 확율 산출.
	BOOL CHECKSTATEBLOW ( float fACTRATE, WORD wACTLEVEL, WORD wLEVEL, WORD wRESIST );

	//	스킬 딜레이 산출.
	float SKILLDELAY ( DWORD dwSKILL_GRADE, WORD wSKILL_LEV, WORD wCHAR_LEVEL, float fDelay );

	//	Note : 파라메타 변경.
	//		계산시에는 int 형으로 계산을 수행하여 연산시 손실을 막는다.
	//		값 적용시에는 WORD 형 값의 영역 이내로 제한.
	inline WORD VARIATION ( WORD &wNow, const WORD wMax, const int nValue )
	{
		int nOld = int(wNow);

		int nNew = nOld + nValue;

		//	WORD 형 값의 영역 이내로 제한.
		if ( nNew < 0 )					nNew = 0;
		if ( nNew > USHRT_MAX )			nNew = USHRT_MAX;

		wNow = nNew;
		if ( wNow >= wMax )				wNow = wMax;

		WORD wDX = (WORD) abs(nOld-int(wNow));
		return wDX;
	}

	//	Note : 파라메타 변경.
	//		계산시에는 int 형으로 계산을 수행하여 연산시 손실을 막는다.
	//		값 적용시에는 WORD 형 값의 영역 이내로 제한.
	inline DWORD VARIATION ( DWORD &dwNow, const DWORD dwMax, const int nValue )
	{
		int nOld = int(dwNow);

		int nNew = nOld + nValue;

		//	WORD 형 값의 영역 이내로 제한.
		if ( nNew < 0 )					nNew = 0;

		dwNow = nNew;
		if ( dwNow >= dwMax )			dwNow = dwMax;

		DWORD dwDX = (DWORD) abs(nOld-int(dwNow));
		return dwDX;
	}

	//	날씨와 속성간 증폭율.
	float WEATHER_ELEMENT_POW ( EMELEMENT emElement, DWORD dwWeather, BOOL bWeatherActive );

	float WEATHER_BLOW_POW ( EMSTATE_BLOW emBlow, DWORD dwWeather, BOOL bWeatherActive );

	//	Note : 체력 같은 수치들을 변화율에 따라 갱신해준다.
	//		계산시에는 int 형으로 계산을 수행하여 연산시 손실을 막는다.
	//		값 적용시에는 WORD 형 값의 영역 이내로 제한.
	inline void UPDATE_POINT ( GLDWDATA &sPOINT, float &fELP_VAR, const float fVAR, const DWORD dwLOW_LMT )
	{
		fELP_VAR += fVAR;
		int nNEWP = int(sPOINT.dwNow) + int(fELP_VAR);
		fELP_VAR -= int(fELP_VAR);

		//	WORD 형 값의 영역 이내로 제한.
		if ( nNEWP < 0 )			nNEWP = 0;
		if ( nNEWP > UINT_MAX )	nNEWP = UINT_MAX;

		if ( nNEWP < dwLOW_LMT )		nNEWP = dwLOW_LMT;

		sPOINT.dwNow = nNEWP;
		sPOINT.LIMIT ();
	}

	//	Note : 체력 같은 수치들을 변화율에 따라 갱신해준다.
	//		계산시에는 int 형으로 계산을 수행하여 연산시 손실을 막는다.
	//		값 적용시에는 WORD 형 값의 영역 이내로 제한.
	inline void UPDATE_POINT ( DWORD &dwNowP, float &fELP_VAR, const DWORD MaxP, const float fVAR, const DWORD dwLOW_LMT )
	{
		fELP_VAR += fVAR;
		int nNEWP = int(dwNowP) + int(fELP_VAR);
		fELP_VAR -= int(fELP_VAR);

		//	WORD 형 값의 영역 이내로 제한.
		if ( nNEWP < 0 )				nNEWP = 0;
		if ( nNEWP < (int)dwLOW_LMT )	nNEWP = dwLOW_LMT;

		dwNowP = nNEWP;
		if ( dwNowP > MaxP )		dwNowP = MaxP;
	}

	inline void UPDATE_POINT ( WORD &wNowP, float &fELP_VAR, const WORD MaxP, const float fVAR, const WORD wLOW_LMT )
	{
		fELP_VAR += fVAR;
		int nNEWP = int(wNowP) + int(fELP_VAR);
		fELP_VAR -= int(fELP_VAR);

		//	WORD 형 값의 영역 이내로 제한.
		if ( nNEWP < 0 )			nNEWP = 0;
		if ( nNEWP > USHRT_MAX )	nNEWP = USHRT_MAX;

		if ( nNEWP < wLOW_LMT )		nNEWP = wLOW_LMT;

		wNowP = nNEWP;
		if ( wNowP > MaxP )			wNowP = MaxP;
	}
};

enum EMBEGINATTACK_FB
{
	EMBEGINA_OK		= 0,
	EMBEGINA_SP		= 1,
	EMBEGINA_ARROW	= 2,
	EMBEGINA_BULLET	= 3, /*gun-bullet logic, Juver, 2017/05/27 */
};


//	Note : 변수 추가시에. RESET_DATA() 함수에 변수 초기화 넣어야됨.
//
struct GLCHARLOGIC :public SCHARDATA2
{	
	EMCHARINDEX			m_CHARINDEX;
	DELAY_MAP			m_SKILLDELAY;					//	사용된 스킬의 딜래이.

	SSKILLFACT			m_sSKILLFACT[SKILLFACT_SIZE];	//	스킬효과들.

	/*dual pet skill, Juver, 2017/12/27 */
	SSKILLFACT			m_sPETSKILLFACT_A;				//  팻의 보조스킬효과
	SSKILLFACT			m_sPETSKILLFACT_B;				//  팻의 보조스킬효과
	
	SLANDEFFECT			m_sLandEffect[EMLANDEFFECT_MULTI];	//  지형 이상 효과들

	DWORD				m_dwTransfromSkill;				//  변신 스킬 넘버


	float				m_fHP_RATE;						//	active 스킬의 HP 증폭율.
	float				m_fMP_RATE;						//	active 스킬의 MP 증폭율.
	float				m_fSP_RATE;						//	active 스킬의 SP 증폭율.

	DWORD				m_dwHOLDBLOW;					//	상태이상이 방지됨.
	BOOL				m_bSTATEBLOW;					//	상태이상이 하나 이상 발생했을때.
	SSTATEBLOW			m_sSTATEBLOWS[EMBLOW_MULTI];	//	상태이상.
	float				m_fSTATE_MOVE;					//	상태 이상 이동속도. ( 1.0f == 100% )
	float				m_fSTATE_DELAY;					//	상태 이상 딜래이. ( 1.0f == 100% ) (+)
	float				m_fSTATE_DAMAGE;				//	상태 이상 대미지 증가.

	SQITEM_FACT			m_sQITEMFACT;					//	? 아이템 적용값.
	SEVENT_FACT			m_sEVENTFACT;					//	이벤트 ? 아이템 적용값.
	//SEventState			m_sEventState;

	SPKCOMBO_COUNT		m_sPKCOMBOCOUNT;


	bool				m_bSTATE_PANT;					//	상태이상 헐떡임.
	bool				m_bSTATE_STUN;					//	상태이상 기절.
	bool				m_bINVISIBLE;					//	투명.
	bool				m_bRECVISIBLE;					//	투명 확인.

	SSTIGMA				m_sSTIGMA_USER;		// 낙인 효과 - 사용.
	SSTIGMA				m_sSTIGMA_TAKER;		// 낙인 효과 - 적용.
	SLINKHP					m_sLINKHP;					// 고리 효과.
	SINCREASEEFF			m_sIncreaseEff;			// 강화 효과;

	float				m_fSKILL_MOVE;					//	스킬 이동속도.
	float				m_fOPTION_MOVE;					//	추가적인 이동속도. 월드에디터에서 사용.


	DEFENSE_SKILL		m_sDefenseSkill;				//  발동형 스킬
	SCHARSTATS			m_sSUMSTATS;					//	합산 Stats.

	WORD				m_wSUM_AP;						//	기본 공격력.
	WORD				m_wSUM_DP;						//	기본 방어력.

	WORD				m_wSUM_PA;						//	격투치.
	WORD				m_wSUM_SA;						//	사격치.
	WORD				m_wSUM_MA;						//	마법치.

	SRESIST				m_sSUMRESIST;					//	저항값. ( 아이템 + 패시브 )
	SRESIST				m_sSUMRESIST_SKILL;				//  스킬 저항값 ( 아이템 + 패시브 + 버프 ) 
	SSUM_ITEM			m_sSUMITEM;						//	아이템 가산값.
	SPASSIVE_SKILL_DATA	m_sSUM_PASSIVE;					//	페시브 가산값.

	int					m_nSUM_PIERCE;					//	관통 정도 합산.
	float				m_fSUM_TARRANGE;				//	공격 가능거리 합산.

	/*skill range spec logic, Juver, 2017/06/06 */
	float				m_fSUM_SKILL_ATTACKRANGE;
	float				m_fSUM_SKILL_APPLYRANGE;

	WORD				m_wACCEPTP;						//	무기 아이템에 능력제한 허용 수치.
	WORD				m_wSUM_DisSP;					//	소모 SP 합산.

	int					m_nHIT;							//	명중율.
	int					m_nSUM_HIT;						//	명중율 + 스킬효과.
	int					m_nAVOID;						//	회피율.
	int					m_nSUM_AVOID;					//	회피율 + 스킬효과.

	int					m_nDEFENSE_BODY;				//	방어력 ( DP + DEX+계수 ).
	int					m_nDEFENSE;						//	방어력 ( DP + DEX*계수 + ITEM_DEF ) * 공간적응 보정.
	int					m_nDEFENSE_SKILL;				//	방어력 ( DP + DEX*계수 + ITEM_DEF ) * 공간적응 보정 + 스킬.

	GLDWDATA			m_gdDAMAGE;						//	기본 공격력. ( AP + 페시브 스킬로 가산된 공업 )
	GLDWDATA			m_gdDAMAGE_SKILL;				//	기본 공격력 + temp 스킬효과.
	GLDWDATA			m_gdDAMAGE_PHYSIC;				//	기본 공격력 + 무기공업 + (격투치or사격치).

	float				m_fDamageRate;					//	공격력 변화율.
	float				m_fDefenseRate;					//	방어력 변화율.

	DAMAGE_SPEC			m_sDamageSpec;					//	데미지 흡수 및 반사

	float				m_fINCR_HP;						//	HP 회복율.
	float				m_fINCR_MP;						//	MP 회복율.
	float				m_fINCR_SP;						//	SP 회복율.

	float				m_fATTVELO;						//	스킬 공격속도 변화.
	WORD				m_wATTRANGE;					//	공격 가능 거리.
	WORD				m_wSUM_ATTRANGE;				//	공격 가능 거리.
	GLITEM_ATT			m_emITEM_ATT;					//	아이탬 공격 속성.

	/*anti shit gs*/
	float				m_fSUM_TARRANGE_ALL;			// 사정거리 증가 옵션 ( 전체 스킬 타입 ) 
	float				m_fSUM_APPLYRANGE_ALL;			// 적용영역 증가 옵션 ( 전체 스킬 타입 )

	float				m_fSKILLDELAY;

	float				m_fEXP_RATE;					//	경험치 수신율.
	float				m_fGEN_RATE;					//	아이템 발생율.

	EMANI_SUBTYPE		m_emANISUBTYPE;					//	Item 착용에 따른 에니메이션 종류.

	SNATIVEID			m_idACTIVESKILL;				//	현제 액티브된 스킬.
	EMANI_MAINTYPE		m_emANIMAINSKILL;				//	스킬에 따른 에니메이션.
	EMANI_SUBTYPE		m_emANISUBSKILL;				//	스킬에 따른 에니메이션.

	SITEM*				m_pITEMS[SLOT_TSIZE];

	BOOL				m_bUseArmSub;					// 극강부를 위한 보조 무기 사용여부

	//	Note : 회복되는 수치 산출용 임시 변수.
	float				m_fIncHP;
	float				m_fIncMP;
	float				m_fIncSP;

	MAPPLAYHOSTILE		m_mapPlayHostile;				//	적대 행위자. ( 쌍방 )

	float				m_fJumpRange;

//	탈것 추가
	BOOL				m_bVehicle;						//  탈것 활성화 여부
	float				m_fVehicleSpeedRate;
	float				m_fVehicleSpeedVol;
	
	bool				m_bSafeZone;

	bool				m_bProhibitPotion; /*prohibit potion skill logic, Juver, 2017/06/06 */
	bool				m_bProhibitSkill; /*prohibit skill logic, Juver, 2017/06/06 */

	/*continuous damage skill logic, Juver, 2017/06/10 */
	CONTINUOUS_DAMAGE_DATA_MAP	m_mapContinuousDamage;

	/*curse skill logic, Juver, 2017/06/10 */
	bool				m_bCurse;
	float				m_fCurseDamage;

	/*vehicle booster system, Juver, 2017/08/11 */
	bool				m_bBoosterCharge;
	bool				m_bBoosterStart;
	float				m_fBoosterTimer;

	int					m_nSummonTime; /*summon time, Juver, 2017/12/12 */

	bool				m_bIgnoreDamage; /*ignore damage, Juver, 2017/12/12 */
	/*block back damage, Juver, 2017/12/12 */
	float				m_fBlockBackDamageTime;
	bool				m_bBlockBackDamage;

	//damagecheck
	WORD				m_wDamageHistory;
	float				m_fDamageHistoryTimer;

	WORD				m_wAutoPotsAttemptCount;
	float				m_fAutoPotsAttemptTimer;
	bool				m_bActionType[SKILL::EMACTION_NSIZE];

	GLCHARLOGIC () :
		m_CHARINDEX(GLCI_BRAWLER_M),

		m_dwHOLDBLOW(NULL),
		m_bSTATEBLOW(FALSE),
		m_fSTATE_MOVE(1.0f),
		m_fSTATE_DELAY(1.0f),
		m_fSTATE_DAMAGE(1.0f),
		
		m_bSTATE_PANT(false),
		m_bSTATE_STUN(false),
		m_bINVISIBLE(false),
		m_bRECVISIBLE(false),

		m_fHP_RATE(0),
		m_fMP_RATE(0),
		m_fSP_RATE(0),

		m_fSKILL_MOVE(0.0f),
		m_fOPTION_MOVE(0.0f),

		m_wSUM_AP(0),
		m_wSUM_DP(0),
		m_wSUM_PA(0),
		m_wSUM_SA(0),
		m_wSUM_MA(0),

		m_nSUM_PIERCE(0),
		m_fSUM_TARRANGE(0),

		/*skill range spec logic, Juver, 2017/06/06 */
		m_fSUM_SKILL_ATTACKRANGE(0.0f),
		m_fSUM_SKILL_APPLYRANGE(0.0f),

		m_nHIT(0),
		m_nSUM_HIT(0),
		m_nAVOID(0),
		m_nSUM_AVOID(0),

		m_wACCEPTP(0),
		m_wSUM_DisSP(0),

		m_nDEFENSE(0),

		m_fDamageRate(1.0f),
		m_fDefenseRate(1.0f),

		m_fINCR_HP(0.0f),
		m_fINCR_MP(0.0f),
		m_fINCR_SP(0.0f),

		m_fIncHP(0.0f),
		m_fIncMP(0.0f),
		m_fIncSP(0.0f),

		m_fATTVELO(0.0f),
		m_wATTRANGE(4),
		m_wSUM_ATTRANGE(4),

		/*anti shit gs*/
		m_fSUM_TARRANGE_ALL(0),
		m_fSUM_APPLYRANGE_ALL(0),

		m_fSKILLDELAY(0.0f),

		m_fEXP_RATE(1),
		m_fGEN_RATE(1),

		m_emITEM_ATT(ITEMATT_NOTHING),

		m_emANISUBTYPE(AN_SUB_NONE),

		m_idACTIVESKILL(NATIVEID_NULL()),

		m_emANIMAINSKILL(AN_NONE),
		m_emANISUBSKILL(AN_SUB_NONE),

		m_bUseArmSub(FALSE),
		m_bVehicle(FALSE),
		m_fVehicleSpeedRate( 0.0f ),
		m_fVehicleSpeedVol( 0.0f ),
		m_bSafeZone(false),

		m_bProhibitPotion(false), /*prohibit potion skill logic, Juver, 2017/06/06 */
		m_bProhibitSkill(false), /*prohibit skill logic, Juver, 2017/06/06 */

		/*curse skill logic, Juver, 2017/06/10 */
		m_bCurse(false),
		m_fCurseDamage(0.0f),

		/*vehicle booster system, Juver, 2017/08/11 */
		m_bBoosterCharge(false),
		m_bBoosterStart(false),
		m_fBoosterTimer(0.0f),

		m_nSummonTime(0), /*summon time, Juver, 2017/12/12 */
		m_bIgnoreDamage(false), /*ignore damage, Juver, 2017/12/12 */

		/*block back damage, Juver, 2017/12/12 */
		m_fBlockBackDamageTime(0.0f),
		m_bBlockBackDamage(false),
		//damagecheck
		m_wDamageHistory(0),
		m_fDamageHistoryTimer(0.0f),
		m_wAutoPotsAttemptCount(0),
		m_fAutoPotsAttemptTimer(0.0f),
		m_fJumpRange(0)
	{
		memset ( m_pITEMS, 0x00, sizeof(SITEM*)*SLOT_TSIZE );
		memset ( m_bActionType, false, sizeof( bool ) * SKILL::EMACTION_NSIZE );
	}

	~GLCHARLOGIC ()
	{
		DEL_PLAYHOSTILE_ALL();
	}
public:
	float GetQuestionTime()			{ return m_sQITEMFACT.fTime; }
	float GetPKComboCountTime()		{ return m_sPKCOMBOCOUNT.fTime; }

public:
	EMCROW GETCROW () const			{ return CROW_PC; }
	WORD GETLEVEL () const			{ return m_wLevel; }
	
	DWORD GETHP () const			{ return m_sHP.dwNow; }
	DWORD GETMAXHP () const			{ return m_sHP.dwMax; }

	float GETMAXHPRATE() const      { return m_sLINKHP.fRef; }
	void  SETMAXHPRATE( float fRate ) { m_sLINKHP.fRef = fRate; }

	int GETMP () const				{ return m_sMP.dwNow; }
	int GETMAXMP () const			{ return m_sMP.dwMax; }

	int GETSP () const				{ return m_sSP.dwNow; }
	int GETMAXSP () const			{ return m_sSP.dwMax; }

	/*combatpoint logic, Juver, 2017/05/28 */
	int GETCP () const				{ return m_sCombatPoint.dwNow; }
	int GETMAXCP () const			{ return m_sCombatPoint.dwMax; }

	int GETHIT () const				{ return m_nSUM_HIT; }
	int GETAVOID () const			{ return m_nSUM_AVOID; }
	EMBRIGHT GETBRIGHT () const		{ if(m_nBright<0) return BRIGHT_DARK; return BRIGHT_LIGHT; }
	int GETBRIGHTPER () const		{ return m_nBright; }
	int GETFORCE_LOW () const		{ return m_gdDAMAGE_PHYSIC.dwLow; }
	int GETFORCE_HIGH () const		{ return m_gdDAMAGE_PHYSIC.dwHigh; }
	int GETDEFENSE () const			{ return m_nDEFENSE_SKILL; }

	/*Codex System, Jhoniex 2/14/2024 */
	int GETHPINC() const { return m_dwHPIncrease; }
	int GETMPINC() const { return m_dwMPIncrease; }
	int GETSPINC() const { return m_dwSPIncrease; }
	int GETATTINC() const { return m_dwAttackIncrease; }
	int GETDEFINC() const { return m_dwDefenseIncrease; }
	int GETSHOOTINC() const { return m_dwShootingIncrease; }
	int GETMELEEINC() const { return m_dwMeleeIncrease; }
	int GETENERGYINC() const { return m_dwEnergyIncrease; }
	int GETRESISTINC() const { return m_dwResistanceIncrease; }
	int GETHITRATEINC() const { return m_dwHitrateIncrease; }
	int GETAVOIDINC() const { return m_dwAvoidrateIncrease; }
	
	float GETSUM_TARRANGE () const	{ return m_fSUM_TARRANGE; }		//	장거리 무기(스킬)에만 가산.
	int GETSUM_PIERCE () const		{ return m_nSUM_PIERCE; }

	WORD GETBODYRADIUS () const		{ return GLCONST_CHAR::wBODYRADIUS; }
	WORD GETATTACKRANGE () const	{ return m_wSUM_ATTRANGE; }	

	/*anti shit gs*/
	float GETSUM_TARRANGE_ALL () const { return m_fSUM_TARRANGE_ALL; }
	float GETSUM_APPLYRANGE_ALL () const { return m_fSUM_APPLYRANGE_ALL; }

	const SCHARSTATS& GETSTATS_ADD () const;
	const SCHARSTATS& GETSTATS_ITEM () const;

	void OptionMoveUp();
	void OptionMoveDown();

	void SetUseArmSub( BOOL bSub )	{ m_bUseArmSub = bSub; }
	BOOL IsUseArmSub() const		{ return m_bUseArmSub; }

	EMSLOT GetCurRHand();
	EMSLOT GetCurLHand();

	BOOL IsCurUseArm( EMSLOT emSlot );

	bool IsDefenseSkill()				{ return m_sDefenseSkill.m_bActive; }
	void SetDefenseSkill(bool bDefense)	{ m_sDefenseSkill.m_bActive = bDefense; }

public:
	void SLOT_ITEM ( const SITEMCUSTOM &sItemCustom, EMSLOT _slot );
	void RELEASE_SLOT_ITEM ( EMSLOT _slot );

	const SITEMCUSTOM& GET_SLOT_ITEM ( EMSLOT _slot )				{ return m_PutOnItems[_slot]; }
	const SNATIVEID& GET_SLOT_NID ( EMSLOT _slot )					{ return m_PutOnItems[_slot].sNativeID; }
	SITEM* GET_SLOT_ITEMDATA ( EMSLOT _slot )						{ return m_pITEMS[_slot]; }
	BOOL VALID_SLOT_ITEM ( EMSLOT _slot );

	void HOLD_ITEM ( const SITEMCUSTOM &sItemCustom );
	void RELEASE_HOLD_ITEM ();
	const SITEMCUSTOM& GET_HOLD_ITEM ()					{ return m_PutOnItems[SLOT_HOLD]; }
	BOOL VALID_HOLD_ITEM ()								{ return GET_HOLD_ITEM().sNativeID!=NATIVEID_NULL(); }

	BOOL ISLONGRANGE_ARMS ();

	BOOL ISHAVEITEM ( SNATIVEID &sNID, DWORD *pNum=NULL );
	BOOL ISREVIVE ();

	SITEM* GET_ELMT_ITEM ();	//	왼손, 오른손 아이템중에서 'BLOW' 속성이 있는 아이템을 반환.

	WORD GETSKILLRANGE_TAR ( const GLSKILL &sSKILL ) const;
	WORD GETSKILLRANGE_APPLY ( const GLSKILL &sSKILL, const WORD dwLEVEL ) const;

public:
	EMBEGINATTACK_FB BEGIN_ATTACK ( WORD wStrikeNum );
	BOOL VALID_LEVELUP () const;
	LONGLONG GET_LEVELUP_EXP () const;
	void LEVLEUP ( bool bInitNowExp = TRUE );
	void STATSUP ( EMSTATS emStats );

	void STATSUP ( SCHARSTATS sStats );
	
	/*Codex System, Jhoniex 2/14/2024 */
	void CODEX_STATS( int nIndex, DWORD dwStatPoint );

	void STATSUP_CMD ( EMSTATS emStats, DWORD value);		//add addstats cmd

	bool RESET_STATS_SKILL ( const WORD wDIS_STAT=USHRT_MAX );
	bool RESET_SKILL ();
	bool RESET_STATS ( const WORD wDIS=USHRT_MAX );

protected:
	void SUM_ITEM ();
	void SUM_PASSIVE ();
	void SUM_ADDITION ( float fCONFT_POINT_RATE );

public:
	void INIT_NEW_CHAR ( const EMCHARINDEX _emCIndex, const DWORD dwUserID, const DWORD dwServerID, const char* szCharName,
		const WORD _wSchool=0, const WORD wHair=0, const WORD wFace=0, const WORD _wHairColor=0, const WORD _wSex=0, const float _fScaleRange=1.0f );

	BOOL INIT_DATA ( BOOL bNEW, BOOL bReGen, float fCONFT_POINT_RATE = 1.0f, bool bInitNowExp = TRUE );

	void INIT_RECOVER ( int nRECOVER=30 );

	void RESET_DATA ();

	void DoPkScore();
	void DoPkDeath();
	void DoPkReset();

public:
	void UPDATE_DATA ( float fTime, float fElapsedTime, BOOL bClient=FALSE, float fCONFT_POINT_RATE=1.0f );
	void UPDATE_MAX_POINT ( float fCONFT_POINT_RATE );
	bool UPDATE_QITEMFACT ( float fElapsedTime );
	bool UPDATE_PKCOMBO ( float fElapsedTime );

public:
	BOOL CHECKSLOT_ITEM ( SNATIVEID sNativeID, EMSLOT emSlot );
	BOOL ACCEPT_ITEM ( SNATIVEID sNativeID );
	BOOL ACCEPT_ITEM ( SNATIVEID sNativeID, SNATIVEID sDisguiseID );
	BOOL ISEMPTY_SLOT ( SNATIVEID sNativeID, EMSLOT emSlot );
	WORD CALC_ACCEPTP ( SNATIVEID sNativeID );
	BOOL SIMPLE_CHECK_ITEM ( SNATIVEID sNativeID );
	BOOL SIMPLE_CHECK_ITEM ( SNATIVEID sNativeID, SNATIVEID sDisguiseID );

public:
	BOOL CHECKHIT ( const STARGETID &cTargetID, const GLLandMan* pLandMan, const BOOL bLowSP );

	/**
	* 캐릭터가 몹을 공격했을때 데미지 계산 공식
	*/
	DWORD CALCDAMAGE(
		int &rResultDAMAGE,
		const DWORD	dwGaeaID,
		const STARGETID &cTargetID,
		const GLLandMan* pLandMan,
		const GLSKILL* pSkill=NULL,
		const DWORD dwskill_lev=0,
		const DWORD dwWeatherFlag=NULL,
		const int dwDivCount=0);
	
	DWORD CALCDAMAGE_20060328(
		int &rResultDAMAGE,
		const DWORD	dwGaeaID,
		const STARGETID &cTargetID,
		const GLLandMan* pLandMan,
		const GLSKILL* pSkill=NULL,
		const DWORD dwskill_lev=0,
		const DWORD dwWeatherFlag=NULL,
		const int dwDivCount=0);

	DWORD CALCDAMAGE_2004(
		int &rResultDAMAGE,
		const DWORD	dwGaeaID,
		const STARGETID &cTargetID,
		const GLLandMan* pLandMan,
		const GLSKILL* pSkill=NULL,
		const DWORD dwskill_lev=0,
		const DWORD dwWeatherFlag=NULL,
		const int dwDivCount=0);

	

	DWORD RECEIVE_DAMAGE ( const DWORD wDamage );

public:
	/*qbox check, Juver, 2017/12/05 */
	BOOL RECEIVE_QITEMFACT ( const SNATIVEID &nidITEM, BOOL bMobGen );
	//	Memo :	이벤트로 인해 받는 ?아이템 효과를 설정한다.
	void RECEIVE_EVENTFACT_BEGIN ( const EMGM_EVENT_TYPE emType, const WORD wValue );
	//	Memo :	이벤트로 인해 받는 ?아이템 효과를 해제한다.
	void RECEIVE_EVENTFACT_END ( const EMGM_EVENT_TYPE emType );
	BOOL RECEIVE_PKCOMBO ( int nCount );

public:
	BOOL ISLEARNED_SKILL ( SNATIVEID skill_id, WORD wLEVEL=0 );
	BOOL ISMASTER_SKILL ( SNATIVEID skill_id );
	SCHARSKILL* GETLEARNED_SKILL ( SNATIVEID skill_id );

	EMSKILL_LEARNCHECK CHECKLEARNABLE_SKILL ( SNATIVEID skill_id );
	void LEARN_SKILL ( SNATIVEID skill_id );
	VOID LVLUP_SKILL ( SNATIVEID skill_id, WORD wToLevel );

	bool LEARN_SKILL_QUEST ( SNATIVEID skill_id );

public:
	float GETATTVELO ();
	float GETMOVEVELO ();
	float GETATT_ITEM (); // 절대치 증가
	float GETMOVE_ITEM (); // 절대치 증가 % 

public:
	//	Note : 적대자 등록. ( pk 관련 ),	true : 신규 등록시, false : 기존 등록 갱신시.
	bool ADD_PLAYHOSTILE ( DWORD dwCHARID, BOOL bBAD );
	bool DEL_PLAYHOSTILE ( DWORD dwCHARID );
	void DEL_PLAYHOSTILE_ALL ();

public:
	//	Note : 적대 리스트에 등록 되어 있는가?
	bool IS_PLAYHOSTILE ( DWORD dwCHARID );
	//	Note : 적대행위자?
	bool IS_HOSTILE_ACTOR ( DWORD dwCHARID );
	//	Note : 자신이 범죄자인가?
	bool ISOFFENDER ();

	bool ISPLAYKILLING ();

public:
	//	Note : 현제 pk 레벨을 알아본다. ( UINT_MAX 일경우 pk 해당사항 없음. )
	DWORD GET_PK_LEVEL ();

	//	Note : 죽을때 경험치 감소 pk 배수.
	float GET_PK_DECEXP_RATE ();
	std::string GET_PK_NAME ();
	DWORD GET_PK_COLOR ();
	float GET_PK_SHOP2BUY ();
	float GET_PK_SHOP2SALE ();

	DWORD GET_PK_ITEMDROP_NUM ();
	float GET_PK_ITEMDROP_RATE ();

public:
	BOOL DOGRINDING(SITEMCUSTOM &sCusItem, 
					const SITEM *pHold, 
					bool &bRESET, 
					bool &bTERMINATE, 
					EMANTIDISAPPEAR &emANTIDISAPPEAR );

public:
	BOOL ISSTATEBLOW ()							{ return m_bSTATEBLOW; }
	void DISABLESKEFF ( int i )					
	{ 
		CONTINUOUSDAMAGE_DELETE(m_sSKILLFACT[i].sNATIVEID);	/*continuous damage skill logic, Juver, 2017/06/10 */		
		m_sSKILLFACT[i].RESET(); 
	}
	void DISABLEBLOW ( int i )					{ m_sSTATEBLOWS[i].emBLOW = EMBLOW_NONE; }
	void RESETAGESKEFF ( int i )				{ m_sSKILLFACT[i].fAGE = 0.0f; }
	void RESETAGEBLOW ( int i )					{ m_sSTATEBLOWS[i].fAGE = 0.0f; }

	void DISABLEALLLANDEFF();
	void ADDLANDEFF( SLANDEFFECT landEffect, int iNum );


	void UPDATESKILLDELAY ( float fElapsedTime );

	void SETACTIVESKILL ( SNATIVEID skill_id );
	SNATIVEID GETACTIVESKILL ()					{ return m_idACTIVESKILL; }
	EMSKILLCHECK CHECHSKILL ( SNATIVEID skill_id, WORD wStrikeNum, bool bNotLearn = false );
	void ACCOUNTSKILL ( SNATIVEID skill_id, WORD wStrikeNum, bool bServer=false );

	void LEARN_SKILL_NONSCROLL ( SNATIVEID skill_id );
	
	/*continuous damage skill logic, Juver, 2017/06/10 */
	BOOL CONTINUOUSDAMAGE_EXIST( SNATIVEID _sID );
	void CONTINUOUSDAMAGE_INSERT( SCONTINUOUS_DAMAGE_DATA _sDATA );
	void CONTINUOUSDAMAGE_DELETE( SNATIVEID _sID );

	/*npc talk buff condition, Juver, 2017/10/03 */
	BOOL ISHAVE_BUFF( SNATIVEID skill_id, WORD wLEVEL=1 );

	int ISHAVE_BUFF( EMSPEC_ADDON emADDON );
	int ISHAVE_BUFF( EMIMPACT_ADDON emADDON );

	/*activity system, Juver, 2017/10/30 */
	SACTIVITY_CHAR_DATA*	GetActivityProg( DWORD dwActivityID );
	SACTIVITY_CHAR_DATA*	GetActivityDone( DWORD dwActivityID );
	DWORD	GetActivityProgNum();
	DWORD	GetActivityDoneNum();

	/*Codex System, Jhoniex 2/14/2024 */
	SCODEX_CHAR_DATA*	GetCodexProg( DWORD dwCodexID );
	SCODEX_CHAR_DATA*	GetCodexDone( DWORD dwCodexID );
	DWORD	GetCodexProgNum();
	DWORD	GetCodexDoneNum();

public:
	virtual void ReSelectAnimation ()	{}
};

//	Note : 변수 추가시에. RESET_DATA() 함수에 변수 초기화 넣어야됨.
//
struct GLCROWLOGIC
{
	DWORD				m_dwGlobalID;				//	생성 메모리 인덱스용 ( 해당 GLLandMan내에서 유니크함 )
	SNATIVEID			m_sNativeID;					//	Crow NativeID.
	PCROWDATA			m_pCrowData;					//	Crow 기본 정보.

	float				m_fACTIONDELAY[SCROWDATA::EMMAXATTACK];	//	동작 cool time;

	SSKILLFACT			m_sSKILLFACT[SKILLFACT_SIZE];	//	스킬효과들.

	DWORD				m_dwHOLDBLOW;					//	상태이상이 방지됨.

	SSTATEBLOW			m_sSTATEBLOWS[EMBLOW_MULTI];	//	상태이상.
	float				m_fSTATE_MOVE;					//	상태 이상 이동속도. ( 1.0f == 100% )
	float				m_fSTATE_DELAY;					//	상태 이상 딜래이. ( 1.0f == 100% ) (+)
	float				m_fSTATE_DAMAGE;				//	상태 이상 대미지 증가.
	
	bool				m_bSTATE_PANT;					//	상태이상 헐떡임.
	bool				m_bSTATE_STUN;					//	상태이상 기절.
	bool				m_bINVISIBLE;					//	투명.
	bool				m_bRECVISIBLE;					//	투명 확인.

	SSTIGMA				m_sSTIGMA_USER;		// 낙인 효과 - 사용.
	SSTIGMA				m_sSTIGMA_TAKER;		// 낙인 효과 - 적용.
	SLINKHP				m_sLINKHP;					// 고리 효과.
	SINCREASEEFF		m_sIncreaseEff;			// 강화 효과;

	float				m_fATTVELO;						//	공격속도.
	float				m_fSKILL_MOVE;					//	스킬 이동속도.


	int					m_nSUM_HIT;						//	명중율 + 스킬효과.
	int					m_nSUM_AVOID;					//	회피율 + 스킬효과.

	int					m_nSUM_DEFENSE;					//	방어력 ( DP + POW*계수 + ITEM_DEF ) * 공간적응 보정 + 스킬.
	int					m_nDX_DAMAGE;					//	기본 공격력. + 스킬효과.
	
	SRESIST				m_sSUMRESIST;					//	저항값.

	int					m_nSUM_PIERCE;					//	추가 관통 정도.
	float				m_fSUM_TARRANGE;				//	공격 가능거리 합산.

	/*skill range spec logic, Juver, 2017/06/06 */
	float				m_fSUM_SKILL_ATTACKRANGE;
	float				m_fSUM_SKILL_APPLYRANGE;

	float				m_fDamageRate;					//	공격력 변화율.
	float				m_fDefenseRate;					//	방어력 변화율.

	DAMAGE_SPEC			m_sDamageSpec;					//	데미지 흡수 및 반사
	DEFENSE_SKILL		m_sDefenseSkill;				//  발동형 스킬

	DWORD				m_dwAType;						//	공격 속성 순번.
	
	DWORD				m_dwNowHP;						//	생명량.
	WORD				m_wNowMP;						//	정신량.
	WORD				m_wNowSP;						//	정신량.

	SNATIVEID			m_idACTIVESKILL;				//	활성 스킬.
	WORD				m_wACTIVESKILL_LVL;				//	활성 스킬  래벨.

	//	Note : 회복되는 수치 산출용 임시 변수.
	float				m_fIncHP;
	float				m_fIncMP;
	float				m_fIncSP;

	//	Note : 변수 추가시에. RESET_DATA() 함수에 변수 초기화 넣어야됨.
	//

	bool				m_bProhibitPotion; /*prohibit potion skill logic, Juver, 2017/06/06 */
	bool				m_bProhibitSkill; /*prohibit skill logic, Juver, 2017/06/06 */

	/*continuous damage skill logic, Juver, 2017/06/10 */
	CONTINUOUS_DAMAGE_DATA_MAP	m_mapContinuousDamage;

	/*curse skill logic, Juver, 2017/06/10 */
	bool				m_bCurse;
	float				m_fCurseDamage;

	/*pvp tyranny, Juver, 2017/08/25 */
	WORD				m_wTyrannyOwner;
	float				m_fTyrannyDamage[TYRANNY_SCHOOL_SIZE];

	bool				m_bIgnoreDamage; /*ignore damage, Juver, 2017/12/12 */

	/*block back damage, Juver, 2017/12/12 */
	float				m_fBlockBackDamageTime;
	bool				m_bBlockBackDamage;

	/*pvp capture the flag, Juver, 2018/02/06 */
	bool				m_bCaptureTheFlagCrowHide;

	GLCROWLOGIC () :
		m_dwGlobalID(0),
		m_sNativeID(0,0),
		m_pCrowData(NULL),

		m_dwHOLDBLOW(NULL),
		m_fSTATE_MOVE(1.0f),
		m_fSTATE_DELAY(1.0f),
		m_fSTATE_DAMAGE(1.0f),

		m_bSTATE_PANT(false),
		m_bSTATE_STUN(false),
		m_bINVISIBLE(false),
		m_bRECVISIBLE(false),

		m_fATTVELO(0.0f),
		m_fSKILL_MOVE(0.0f),

		m_nSUM_HIT(0),
		m_nSUM_AVOID(0),

		m_nSUM_DEFENSE(0),
		m_nDX_DAMAGE(0),

		m_fSUM_TARRANGE(0),
		m_nSUM_PIERCE(0),

		/*skill range spec logic, Juver, 2017/06/06 */
		m_fSUM_SKILL_ATTACKRANGE(0.0f),
		m_fSUM_SKILL_APPLYRANGE(0.0f),

		m_fDamageRate(1.0f),
		m_fDefenseRate(1.0f),

		m_dwAType(0),

		m_dwNowHP(0),
		m_wNowMP(0),
		m_wNowSP(0),
		
		m_idACTIVESKILL(false),
		m_wACTIVESKILL_LVL(0),

		m_fIncHP(0.0f),
		m_fIncMP(0.0f),
		m_fIncSP(0.0f),

		m_bProhibitPotion(false), /*prohibit potion skill logic, Juver, 2017/06/06 */
		m_bProhibitSkill(false), /*prohibit skill logic, Juver, 2017/06/06 */

		/*curse skill logic, Juver, 2017/06/10 */
		m_bCurse(false),
		m_fCurseDamage(0.0f),

		m_bIgnoreDamage(false), /*ignore damage, Juver, 2017/12/12 */

		/*block back damage, Juver, 2017/12/12 */
		m_fBlockBackDamageTime(0.0f),
		m_bBlockBackDamage(false),

		/*pvp tyranny, Juver, 2017/08/25 */
		m_wTyrannyOwner(TYRANNY_SCHOOL_SIZE),

		/*pvp capture the flag, Juver, 2018/02/06 */
		m_bCaptureTheFlagCrowHide(false)
	{
		for ( int i=0; i< SCROWDATA::EMMAXATTACK; ++i )
			m_fACTIONDELAY[i] = 0.0f;

		/*pvp tyranny, Juver, 2017/08/25 */
		for ( int i=0; i<TYRANNY_SCHOOL_SIZE; ++ i )
			m_fTyrannyDamage[i] = 0.0f;
	}

	void RESET_DATA ();

	bool VALIDACTION ()
	{
		const SCROWATTACK *pAttack = m_pCrowData->m_sCrowAttack;
		for ( int i=0; i<SCROWDATA::EMMAXATTACK; ++i )
		{
			if ( pAttack[i].bUsed )		return true;
		}

		return true;
	}

	bool VALIDSKILL ()
	{
		const SCROWATTACK *pAttack = m_pCrowData->m_sCrowAttack;
		for ( int i=0; i<SCROWDATA::EMMAXATTACK; ++i )
		{
			if ( pAttack[i].bUsed && pAttack[i].skill_id!=SNATIVEID(false) )		return true;
		}

		return true;
	}

	bool VALIDATTACK ()
	{
		const SCROWATTACK *pAttack = m_pCrowData->m_sCrowAttack;
		for ( int i=0; i<SCROWDATA::EMMAXATTACK; ++i )
		{
			if ( pAttack[i].bUsed && pAttack[i].skill_id==SNATIVEID(false) )		return true;
		}

		return true;
	}

	EMCROW GETCROW () const 	{ return m_pCrowData->m_emCrow; }
	WORD GETLEVEL () const		{ return m_pCrowData->m_wLevel; }
	const char* GETNAME ();
	
	DWORD GETHP () const		{ return m_dwNowHP; }
	DWORD GETMAXHP () const		{ return m_pCrowData->m_dwHP; }

	float GETMAXHPRATE() const;
	void  SETMAXHPRATE( float fRate );

	int GETMP () const			{ return m_wNowMP; }
	int GETMAXMP () const		{ return m_pCrowData->m_wMP; }

	int GETSP () const			{ return m_wNowSP; }
	int GETMAXSP () const		{ return m_pCrowData->m_wSP; }

	int GETHIT () const			{ return m_nSUM_HIT; }
	int GETAVOID () const		{ return m_nSUM_AVOID; }

	EMBRIGHT GETBRIGHT () const	{ return m_pCrowData->m_emBright; }

	int GETFORCE_LOW () const;
	int GETFORCE_HIGH () const;
	int GETDEFENSE () const			{ return m_nSUM_DEFENSE; }

	float GETSUM_TARRANGE () const	{ return m_fSUM_TARRANGE; }		//	장거리 무기(스킬)에만 가산.
	int GETSUM_PIERCE () const		{ return m_nSUM_PIERCE; }

	WORD GETBODYRADIUS () const		{ return m_pCrowData->m_sAction.m_wBodyRadius; }
	WORD GETATTACKRANGE () const	{ return m_pCrowData->m_sCrowAttack[m_dwAType].wRange; }

	WORD GETSKILLRANGE_APPLY ( const GLSKILL &sSKILL, const WORD dwLEVEL ) const;

	BOOL INIT_DATA ();

	BOOL CHECKHIT ( const STARGETID &cTargetID, const GLLandMan* pLandMan );
	DWORD CALCDAMAGE ( int& rResultDAMAGE, const DWORD dwGaeaID, const STARGETID &cTargetID, const GLLandMan* pLandMan,
		const GLSKILL* pSkill=NULL, DWORD dwskill_lev=0, DWORD dwWeatherFlag=NULL, DWORD dwDivCount=0 );

	DWORD RECEIVE_DAMAGE ( const DWORD wDamage );

	
	void DISABLESKEFF ( int i )					
	{ 
		CONTINUOUSDAMAGE_DELETE(m_sSKILLFACT[i].sNATIVEID); /*continuous damage skill logic, Juver, 2017/06/10 */
		m_sSKILLFACT[i].sNATIVEID = NATIVEID_NULL(); 
	} 

	void DISABLEBLOW ( int i )					{ m_sSTATEBLOWS[i].emBLOW = EMBLOW_NONE; }

	void UPDATE_DATA ( float fTime, float fElapsedTime, BOOL bClient=FALSE );

	void SETACTIVESKILL ( SNATIVEID skill_id, WORD wLevel );
	SNATIVEID GETACTIVESKILL ()						{ return m_idACTIVESKILL; }
	EMSKILLCHECK CHECHSKILL ( DWORD dwAType, bool bNotLearn = false );
	void ACCOUNTSKILL ( WORD wStrikeNum );

	float GETATTVELO ();
	float GETMOVEVELO ();

	bool IsDefenseSkill()				{ return m_sDefenseSkill.m_bActive; }
	void SetDefenseSkill(bool bDefense)	{ m_sDefenseSkill.m_bActive = bDefense; }

	/*continuous damage skill logic, Juver, 2017/06/10 */
	BOOL CONTINUOUSDAMAGE_EXIST( SNATIVEID _sID );
	void CONTINUOUSDAMAGE_INSERT( SCONTINUOUS_DAMAGE_DATA _sDATA );
	void CONTINUOUSDAMAGE_DELETE( SNATIVEID _sID );
	BOOL ISHAVE_BUFF( SNATIVEID skill_id, WORD wLEVEL=1 );

	int ISHAVE_BUFF( EMSPEC_ADDON emADDON );
	int ISHAVE_BUFF( EMIMPACT_ADDON emADDON );
};


EMANI_SUBTYPE CHECK_ANISUB ( SITEM* pRHAND, SITEM* pLHAND );
EMANI_SUBTYPE CHECK_ATTACK_ANISUB ( SITEM* pRHAND, SITEM* pLHAND );

//! 아이템 획득 권한을 주기위한...
//! 데미지 로그를 기록하기 위한 구조체
struct SDAMAGELOG
{
	DWORD dwUserID; ///< User ID
	DWORD dwGaeaID; ///< 가이아 ID
	DWORD dwDamage; ///< 준 데미지	

	SDAMAGELOG () 
		: dwUserID(0)
		, dwGaeaID(0)
		, dwDamage(0)
	{
	}

	SDAMAGELOG ( DWORD dwuserid, DWORD dwgaeaid, DWORD dwdamage ) 
		: dwUserID(dwuserid)
		, dwGaeaID(dwgaeaid)
		, dwDamage(dwdamage)
	{
	}
};

typedef std::map<DWORD, SDAMAGELOG>	DAMAGELOG;
typedef DAMAGELOG::iterator					DAMAGELOG_ITER;

inline void AddDamageLog ( DAMAGELOG &cDamageLog, DWORD dwGaeaID, DWORD dwUserID, DWORD dwDamage )
{
	DAMAGELOG_ITER iter = cDamageLog.find ( dwUserID );
	DAMAGELOG_ITER iter_end = cDamageLog.end();
	
	if ( iter != iter_end )
	{
		SDAMAGELOG &sDamageLog = (*iter).second;
		if ( sDamageLog.dwUserID == dwUserID )	dwDamage += sDamageLog.dwDamage;
	}
	
	cDamageLog[dwUserID] = SDAMAGELOG(dwUserID,dwGaeaID,dwDamage);
}

//! 추가:2006-04-12 Jgkim
//! 파티 데미지 로그기록을 위한 구조체
struct SDAMAGELOGPARTY
{
	DWORD m_dwPartyID; ///< 파티 고유번호
	DWORD m_dwDamage; ///< 파티가 준 데미지

	SDAMAGELOGPARTY()
		: m_dwPartyID(0)
		, m_dwDamage(0)
	{
	}

	SDAMAGELOGPARTY(DWORD dwPartyID, DWORD dwDamage)
		: m_dwPartyID(dwPartyID)
		, m_dwDamage(dwDamage)
	{
	}
};
typedef std::map<DWORD, SDAMAGELOGPARTY>	DAMAGELOGPARTY;
typedef DAMAGELOGPARTY::iterator					DAMAGELOGPARTY_ITER;

inline void AddDamageLogParty(DAMAGELOGPARTY &cDamageLogParty, DWORD dwPartyID, DWORD dwDamage)
{
	if (dwPartyID == PARTY_NULL) return;

	DAMAGELOGPARTY_ITER iter     = cDamageLogParty.find(dwPartyID);
	DAMAGELOGPARTY_ITER iter_end = cDamageLogParty.end();
	
	if (iter != iter_end)
	{
		SDAMAGELOGPARTY &sDamageLogParty = (*iter).second;
		if (sDamageLogParty.m_dwPartyID == dwPartyID)
		{
			dwDamage += sDamageLogParty.m_dwDamage;
		}
	}	
	cDamageLogParty[dwPartyID] = SDAMAGELOGPARTY(dwPartyID, dwDamage);
}

inline bool CHECHSKILL_ITEM ( GLSKILL_ATT emSKILL, GLITEM_ATT emITEM, const bool bHiddenWeapon )
{
	// 극강부 글러브 착용 시 스킬 사용 가능 유무;
	if ( emITEM == ITEMATT_FIST && bHiddenWeapon )
	{
		return true;
	}

	switch ( emSKILL )
	{
	case SKILLATT_NOTHING:			
		return ( emITEM == ITEMATT_NOTHING );
	case SKILLATT_SWORD:			
		return ( emITEM == ITEMATT_SWORD );
	case SKILLATT_BLADE:			
		return ( emITEM == ITEMATT_BLADE );
	case SKILLATT_SWORDBLADE:		
		return ( emITEM == ITEMATT_SWORD || emITEM == ITEMATT_BLADE );
	case SKILLATT_DAGGER:			
		return ( emITEM == ITEMATT_DAGGER );
	case SKILLATT_SPEAR:			
		return ( emITEM == ITEMATT_SPEAR );
	case SKILLATT_STICK:			
		return ( emITEM == ITEMATT_STICK );
	case SKILLATT_GAUNT:			
		return ( emITEM == ITEMATT_GAUNT );
	case SKILLATT_BOW:				
		return ( emITEM == ITEMATT_BOW );
	case SKILLATT_THROW:			
		return ( emITEM == ITEMATT_THROW );

	case SKILLATT_FIST:
		return ( emITEM==ITEMATT_FIST );

		/*gun-bullet logic, Juver, 2017/05/27 */
	case SKILLATT_DUAL_GUN:			
		return ( emITEM == ITEMATT_GUN);
	case SKILLATT_RAIL_GUN:			
		return ( emITEM == ITEMATT_RAILGUN);
	case SKILLATT_PORTAL_GUN:			
		return ( emITEM == ITEMATT_PORTALGUN);
	case SKILLATT_SHOTGUN:			
		return ( emITEM == ITEMATT_RAILGUN || emITEM == ITEMATT_PORTALGUN );
	case SKILLATT_GUN:			
		return ( emITEM == ITEMATT_GUN || emITEM == ITEMATT_RAILGUN || emITEM == ITEMATT_PORTALGUN );

	case SKILLATT_SCYTHE:			
		return ( emITEM == ITEMATT_SCYTHE );

	case SKILLATT_DUALSPEAR:			
		return ( emITEM == ITEMATT_DUALSPEAR );

	case SKILLATT_SHURIKEN:			
		return ( emITEM == ITEMATT_SHURIKEN );

	case SKILLATT_NOCARE:			
		return ( emITEM == ITEMATT_NOCARE );
	};

	return	false;
}

/*vehicle system, Juver, 2017/08/07 */
EMANI_MAINTYPE	CHECK_VEHICLE_MAIN( SNATIVEID sVehicleID );
EMANI_SUBTYPE   CHECK_VEHICLE_SUB( EMANI_MAINTYPE emCharMainAni, bool bPassenger = FALSE );


D3DXVECTOR3 UpdateSkillDirection ( const D3DXVECTOR3 &vMyPos, const D3DXVECTOR3 vMyDir, const SNATIVEID sSKILLID, const D3DXVECTOR3 vTARPOS, const STARID *pTARID );


inline void GetRandomPos( D3DXVECTOR3* pDestPos, D3DXVECTOR3* pOrgPos, float fDist )
{
	D3DXVECTOR3			vDir;
	vDir.x = (((((float) rand())/RAND_MAX)*2.f)-1.f);
	vDir.y = (((((float) rand())/RAND_MAX)*2.f)-1.f);
	vDir.z = (((((float) rand())/RAND_MAX)*2.f)-1.f);

	D3DXVec3Normalize( &vDir, &vDir );

	pDestPos->x = pOrgPos->x + ( vDir.x * fDist );
	pDestPos->y = pOrgPos->y + ( vDir.y * fDist );
	pDestPos->z = pOrgPos->z + ( vDir.z * fDist );
} //GetRandomPos

#endif // GLOGIXEX_H_