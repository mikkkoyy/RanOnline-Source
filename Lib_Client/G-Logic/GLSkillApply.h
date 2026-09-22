#ifndef GLSKILL_APPLY_H_
#define GLSKILL_APPLY_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"

#include "./GLCharDefine.h"
#include "./GLSkillDefine.h"

namespace SKILL
{
	enum EMTYPES
	{
		EMFOR_HP				= 0,	//	체력치 변화.
		EMFOR_MP				= 1,	//	기력치 변화.
		EMFOR_SP				= 2,	//	근력치 변화.

		EMFOR_VARHP				= 3,	//	채력치 변화율.
		EMFOR_VARMP				= 4,	//	기력치 변화율.
		EMFOR_VARSP				= 5,	//	근력치 변화율.

		EMFOR_DEFENSE			= 6,	//	방어치 변화.
		EMFOR_CURE				= 7,	//	해독. ( 해독 종류 필요 )

		EMFOR_HITRATE			= 8,	//	명중율.
		EMFOR_AVOIDRATE			= 9,	//	회피율.

		EMFOR_VARAP				= 10,	//	모든 (채력,기력,근력) 변화율.

		EMFOR_VARDAMAGE			= 11,	//	공격치 변화량.
		EMFOR_VARDEFENSE		= 12,	//	방어치 변화량.

		EMFOR_PA				= 13,	//	격투치.
		EMFOR_SA				= 14,	//	사격치.
		EMFOR_MA				= 15,	//	방어치.

		EMFOR_HP_RATE			= 16,	//	HP 증폭율.
		EMFOR_MP_RATE			= 17,	//	MP 증폭율.
		EMFOR_SP_RATE			= 18,	//	SP 증폭율.

		EMFOR_PET_GETALLITEMS	= 19,	//	모든 아이템 줍기	// PetData
		EMFOR_PET_GETRAREITEMS	= 20,	//	레어 아이템만 줍기
		EMFOR_PET_GETPOTIONS	= 21,	//	약품류 아이템만 줍기
		EMFOR_PET_GETMONEY		= 22,	//	돈만 줍기
		EMFOR_PET_GETSTONE		= 23,	//	연마재 아이템만 줍기
		EMFOR_PET_HEAL			= 24,	//	주인의 HP 회복 속도 상승
		EMFOR_PET_SUPPROT		= 25,	//	주인의 HP, MP, SP 회복률 상승
		EMFOR_PET_BACKUP_ATK	= 26,	//	주인의 공격력(율) 상승
		EMFOR_PET_BACKUP_DEF	= 27,	//	주인의 방어력(율) 상승
		EMFOR_PET_AUTOMEDISON	= 28,	//	자동 약품 사용
		EMFOR_PET_PROTECTITEMS	= 29,	//	주인이 아이템 잃어버리지 않도록 보호

		EMFOR_RESIST			= 30,	//	저항 변화량

		EMFOR_SUMMONTIME		= 31,	/*summon time, Juver, 2017/12/12 */

		FOR_TYPE_SIZE			= 32
	};

	struct SSTATE_BLOW
	{
		float			fRATE;		//	발생 확율.
		float			fVAR1;		//	적용 값 1.
		float			fVAR2;		//	적용 값 2.

		SSTATE_BLOW () :
			fRATE(0.0f),
			fVAR1(0.0f),
			fVAR2(0.0f)
		{
		}
	};

	struct SSPEC_100
	{
		float	fVAR1;
		float	fVAR2;
		DWORD	dwFLAG;

		SSPEC_100 () :
			fVAR1(0.0f),
			fVAR2(0.0f),
			dwFLAG(NULL)
		{
		}
	};

	struct SSPEC_108
	{
		float		fVAR1;
		float		fVAR2;
		DWORD		dwFLAG;
		SNATIVEID	dwNativeID;

		SSPEC_108 () :
			fVAR1(0.0f),
			fVAR2(0.0f),
			dwFLAG(NULL),
			dwNativeID(NATIVEID_NULL())
		{
		}
	};

	struct SSPEC_109
	{
		float		fVAR1;
		float		fVAR2;
		float		fVAR3;
		float		fVAR4;
		DWORD		dwFLAG;
		SNATIVEID	dwNativeID;

		SSPEC_109 () :
			fVAR1(0.0f),
			fVAR2(0.0f),
			fVAR3(1.0f),
			fVAR4(1.0f),
			dwFLAG(NULL),
			dwNativeID(NATIVEID_NULL())
		{
		}
	};

	struct SSPEC
	{
		float		fVAR1;
		float		fVAR2;
		float		fVAR3;
		float		fVAR4;
		DWORD		dwFLAG;
		SNATIVEID	dwNativeID;
		SNATIVEID	dwLinkID;

		SSPEC () :
			fVAR1(0.0f),
			fVAR2(0.0f),
			fVAR3(1.0f),
			fVAR4(1.0f),
			dwFLAG(NULL),
			dwNativeID(NATIVEID_NULL()),
			dwLinkID(NATIVEID_NULL())
		{
		}

		void Assign( SSPEC_100 &sOldData )
		{
			fVAR1 = sOldData.fVAR1;
			fVAR2 = sOldData.fVAR2;
			fVAR3 = 1.0f;
			fVAR4 = 1.0f;
			dwFLAG = sOldData.dwFLAG;
		}

		void Assign( SSPEC_108 &sOldData )
		{
			fVAR1 = sOldData.fVAR1;
			fVAR2 = sOldData.fVAR2;
			fVAR3 = 1.0f;
			fVAR4 = 1.0f;
			dwFLAG = sOldData.dwFLAG;
			dwNativeID = sOldData.dwNativeID;
		}

		void Assign( SSPEC_109 &sOldData )
		{
			fVAR1 = sOldData.fVAR1;
			fVAR2 = sOldData.fVAR2;
			fVAR3 = 1.0f;
			fVAR4 = 1.0f;
			dwFLAG = sOldData.dwFLAG;
			dwNativeID = sOldData.dwNativeID;
		}
	};

	struct CDATA_LVL_100
	{
		float			fDELAYTIME;			//	딜래이 타임.
		float			fLIFE;				//	지속시간.
		WORD			wAPPLYRANGE;		//	적용 영역.
		WORD			wAPPLYNUM;			//	적용 갯수.
		WORD			wAPPLYANGLE;		//	적용 각도.
		WORD			wPIERCENUM;			//	관통 정도.
		float			fBASIC_VAR;			//	변화값.
		WORD			wUSE_ARROWNUM;		//	소모 화살 갯수.
		WORD			wUSE_HP;			//	사용시 HP 소진량.
		WORD			wUSE_MP;			//	사용시 MP 소진량.
		WORD			wUSE_SP;			//	사용시 SP 소진량.
		WORD			wUSE_HP_PTY;		//	사용시 HP 파티원 소진량.
		WORD			wUSE_MP_PTY;		//	사용시 MP 파티원 소진량.
		WORD			wUSE_SP_PTY;		//	사용시 SP 파티원 소진량.
	};

	struct CDATA_LVL_101
	{
		float			fDELAYTIME;			//	딜래이 타임.
		float			fLIFE;				//	지속시간.
		WORD			wAPPLYRANGE;		//	적용 영역.
		WORD			wAPPLYNUM;			//	적용 갯수.
		WORD			wAPPLYANGLE;		//	적용 각도.
		WORD			wPIERCENUM;			//	관통 정도.
		WORD			wTARNUM;			//	타겟 갯수.
		float			fBASIC_VAR;			//	변화값.
		WORD			wUSE_ARROWNUM;		//	소모 화살 갯수.
		WORD			wUSE_CHARMNUM;		//	소모 부적 갯수.
		WORD			wUSE_EXP;			//	사용시 EXP 소진량.
		WORD			wUSE_HP;			//	사용시 HP 소진량.
		WORD			wUSE_MP;			//	사용시 MP 소진량.
		WORD			wUSE_SP;			//	사용시 SP 소진량.
		WORD			wUSE_HP_PTY;		//	사용시 HP 파티원 소진량.
		WORD			wUSE_MP_PTY;		//	사용시 MP 파티원 소진량.
		WORD			wUSE_SP_PTY;		//	사용시 SP 파티원 소진량.
	};

	struct CDATA_LVL_102
	{
		float			fDELAYTIME;			//	딜래이 타임.
		float			fLIFE;				//	지속시간.
		WORD			wAPPLYRANGE;		//	적용 영역.
		WORD			wAPPLYNUM;			//	적용 갯수.
		WORD			wAPPLYANGLE;		//	적용 각도.
		WORD			wPIERCENUM;			//	관통 정도.
		WORD			wTARNUM;			//	타겟 갯수.
		float			fBASIC_VAR;			//	변화값.
		WORD			wUSE_ARROWNUM;		//	소모 화살 갯수.
		WORD			wUSE_CHARMNUM;		//	소모 부적 갯수.
		WORD			wUSE_BULLETNUM;	
		WORD			wUSE_EXP;			//	사용시 EXP 소진량.
		WORD			wUSE_HP;			//	사용시 HP 소진량.
		WORD			wUSE_MP;			//	사용시 MP 소진량.
		WORD			wUSE_SP;			//	사용시 SP 소진량.
		WORD			wUSE_CP;
		WORD			wUSE_HP_PTY;		//	사용시 HP 파티원 소진량.
		WORD			wUSE_MP_PTY;		//	사용시 MP 파티원 소진량.
		WORD			wUSE_SP_PTY;		//	사용시 SP 파티원 소진량.
	};

	//	적용 값 래밸별 설정.
	struct CDATA_LVL
	{
		float			fDELAYTIME;			//	딜래이 타임.
		float			fLIFE;				//	지속시간.
		WORD			wAPPLYRANGE;		//	적용 영역.
		WORD			wAPPLYNUM;			//	적용 갯수.
		WORD			wAPPLYANGLE;		//	적용 각도.
		WORD			wPIERCENUM;			//	관통 정도.
		WORD			wTARNUM;			//	타겟 갯수.
		float			fBASIC_VAR;			//	변화값.
		WORD			wUSE_ARROWNUM;		//	소모 화살 갯수.
		WORD			wUSE_CHARMNUM;		//	소모 부적 갯수.
		WORD			wUSE_BULLETNUM;	
		WORD			wUSE_EXP;			//	사용시 EXP 소진량.
		WORD			wUSE_HP;			//	사용시 HP 소진량.
		WORD			wUSE_MP;			//	사용시 MP 소진량.
		WORD			wUSE_SP;			//	사용시 SP 소진량.
		WORD			wUSE_CP;
		DWORD			dwDATA;				// official unknown data
		WORD			wUSE_HP_PTY;		//	사용시 HP 파티원 소진량.
		WORD			wUSE_MP_PTY;		//	사용시 MP 파티원 소진량.
		WORD			wUSE_SP_PTY;		//	사용시 SP 파티원 소진량.

		void Assign ( CDATA_LVL_100 &sOldData );
		void Assign ( CDATA_LVL_101 &sOldData );
		void Assign ( CDATA_LVL_102 &sOldData );

		CDATA_LVL (void) :
			fDELAYTIME(0.0f),
			fLIFE(0.0f),
			wAPPLYRANGE(0),
			wAPPLYNUM(1),
			wAPPLYANGLE(0),
			wPIERCENUM(1),
			wTARNUM(1),
			fBASIC_VAR(0.0f),
			wUSE_ARROWNUM(0),
			wUSE_CHARMNUM(0),
			wUSE_BULLETNUM(0),
			wUSE_EXP(0),
			wUSE_HP(0),
			wUSE_MP(0),
			wUSE_SP(0),
			wUSE_CP(0),
			dwDATA(0),
			wUSE_HP_PTY(0),
			wUSE_MP_PTY(0),
			wUSE_SP_PTY(0)
		{
		}
	};

	struct SIMPACTS_108
	{
		EMIMPACT_ADDON		emADDON;					//	부가효과 종류.
		float				fADDON_VAR[MAX_LEVEL];		//	변화 수치.

		SIMPACTS_108()
			: emADDON(EMIMPACTA_NONE)
		{
			for ( int i=0; i<MAX_LEVEL; ++i )	
				fADDON_VAR[i] = 0;
		};
	};

	struct SSPECS_108
	{
		EMSPEC_ADDON		emSPEC;						//	특수.
		SKILL::SSPEC_108	sSPEC[MAX_LEVEL];			//	특수 기능 설정값.

		SSPECS_108()
			: emSPEC(EMSPECA_NULL)
		{
			for ( int i=0; i<MAX_LEVEL; ++i )	
				sSPEC[i] = SKILL::SSPEC_108();
		};
	};

	struct SIMPACTS_109
	{
		EMIMPACT_ADDON		emADDON;					//	부가효과 종류.
		float				fADDON_VAR[MAX_LEVEL];		//	변화 수치.
		float				fADDON_VAR2[MAX_LEVEL];

		SIMPACTS_109()
			: emADDON(EMIMPACTA_NONE)
		{
			for ( int i=0; i<MAX_LEVEL; ++i )
			{
				fADDON_VAR[i] = 0.0f;
				fADDON_VAR2[i] = 1.0f;
			}
		};
	};

	struct SSPECS_109
	{
		EMSPEC_ADDON		emSPEC;						//	특수.
		SKILL::SSPEC_109		sSPEC[MAX_LEVEL];			//	특수 기능 설정값.

		SSPECS_109()
			: emSPEC(EMSPECA_NULL)
		{
			for ( int i=0; i<MAX_LEVEL; ++i )	
				sSPEC[i] = SKILL::SSPEC_109();
		};
	};

	struct SIMPACTS
	{
		EMIMPACT_ADDON		emADDON;					//	부가효과 종류.
		float				fADDON_VAR[MAX_LEVEL];		//	변화 수치.
		float				fADDON_VAR2[MAX_LEVEL];

		SIMPACTS()
			: emADDON(EMIMPACTA_NONE)
		{
			for ( int i=0; i<MAX_LEVEL; ++i )
			{
				fADDON_VAR[i] = 0.0f;
				fADDON_VAR2[i] = 1.0f;
			}
		};

		void Assign ( SIMPACTS_108 &sOldData )
		{
			emADDON = sOldData.emADDON;

			for ( int i=0; i<MAX_LEVEL; ++i )
			{
				fADDON_VAR[i] = sOldData.fADDON_VAR[i];
				fADDON_VAR2[i] = 1.0f;
			}
		}

		void Assign ( SIMPACTS_109 &sOldData )
		{
			emADDON = sOldData.emADDON;

			for ( int i=0; i<MAX_LEVEL; ++i )
			{
				fADDON_VAR[i] = sOldData.fADDON_VAR[i];
				fADDON_VAR2[i] = sOldData.fADDON_VAR2[i];
			}
		}
	};

	struct SSPECS
	{
		EMSPEC_ADDON		emSPEC;						//	특수.
		SKILL::SSPEC		sSPEC[MAX_LEVEL];			//	특수 기능 설정값.

		SSPECS()
			: emSPEC(EMSPECA_NULL)
		{
			for ( int i=0; i<MAX_LEVEL; ++i )	
				sSPEC[i] = SKILL::SSPEC();
		};

		void Assign ( SSPECS_108 &sOldData )
		{
			emSPEC = sOldData.emSPEC;

			for ( int i=0; i<MAX_LEVEL; ++i )	
				sSPEC[i].Assign( sOldData.sSPEC[i] );
		}

		void Assign ( SSPECS_109 &sOldData )
		{
			emSPEC = sOldData.emSPEC;

			for ( int i=0; i<MAX_LEVEL; ++i )	
				sSPEC[i].Assign( sOldData.sSPEC[i] );
		}
	};

	
	struct SAPPLY_100
	{
		//	기본 적용값 설정.
		SKILL::EMTYPES		emBASIC_TYPE;				//	적용값 타입.
		EMELEMENT			emELEMENT;					//	( 수, 광, 화, 독, 없음 ) 속성.
		DWORD				dwCUREFLAG;					//	해독 종류.
		SKILL::CDATA_LVL_100 sDATA_LVL[MAX_LEVEL];		//	레벨별 설정 값.

		//	부가 효과 설정.
		EMIMPACT_ADDON		emADDON;					//	부가효과 종류.
		float				fADDON_VAR[MAX_LEVEL];		//	변화 수치.

		//	상태이상 설정.
		EMSTATE_BLOW		emSTATE_BLOW;				//	상태 이상 적용 타입.
		SKILL::SSTATE_BLOW	sSTATE_BLOW[MAX_LEVEL];		//	상태 이상 수치들.

		//	특수기능 설정.
		EMSPEC_ADDON		emSPEC;						//	특수.
		SKILL::SSPEC_100	sSPEC[MAX_LEVEL];			//	특수 기능 설정값.
	};

	struct SAPPLY_101
	{
		//	기본 적용값 설정.
		SKILL::EMTYPES		emBASIC_TYPE;				//	적용값 타입.
		EMELEMENT			emELEMENT;					//	( 수, 광, 화, 독, 없음 ) 속성.
		DWORD				dwCUREFLAG;					//	해독 종류.
		SKILL::CDATA_LVL_101	sDATA_LVL[MAX_LEVEL];		//	레벨별 설정 값.

		//	부가 효과 설정.
		EMIMPACT_ADDON		emADDON;					//	부가효과 종류.
		float				fADDON_VAR[MAX_LEVEL];		//	변화 수치.

		//	상태이상 설정.
		EMSTATE_BLOW		emSTATE_BLOW;				//	상태 이상 적용 타입.
		SKILL::SSTATE_BLOW	sSTATE_BLOW[MAX_LEVEL];		//	상태 이상 수치들.

		//	특수기능 설정.
		EMSPEC_ADDON		emSPEC;						//	특수.
		SKILL::SSPEC_100	sSPEC[MAX_LEVEL];			//	특수 기능 설정값.
	};

	struct SAPPLY_102
	{
		//	기본 적용값 설정.
		SKILL::EMTYPES		emBASIC_TYPE;				//	적용값 타입.
		EMELEMENT			emELEMENT;					//	( 수, 광, 화, 독, 없음 ) 속성.
		DWORD				dwCUREFLAG;					//	해독 종류.
		SKILL::CDATA_LVL_101	sDATA_LVL[MAX_LEVEL];		//	레벨별 설정 값.

		//	부가 효과 설정.
		EMIMPACT_ADDON		emADDON;					//	부가효과 종류.
		float				fADDON_VAR[MAX_LEVEL];		//	변화 수치.

		//	상태이상 설정.
		EMSTATE_BLOW		emSTATE_BLOW;				//	상태 이상 적용 타입.
		SKILL::SSTATE_BLOW	sSTATE_BLOW[MAX_LEVEL];		//	상태 이상 수치들.

		//	특수기능 설정.
		EMSPEC_ADDON		emSPEC;						//	특수.
		SKILL::SSPEC_100	sSPEC[MAX_LEVEL];			//	특수 기능 설정값.
	};

	struct SAPPLY_103
	{
		//	기본 적용값 설정.
		SKILL::EMTYPES		emBASIC_TYPE;				//	적용값 타입.
		EMELEMENT			emELEMENT;					//	( 수, 광, 화, 독, 없음 ) 속성.
		DWORD				dwCUREFLAG;					//	해독 종류.
		SKILL::CDATA_LVL_101	sDATA_LVL[MAX_LEVEL];		//	레벨별 설정 값.

		//	부가 효과 설정.
		EMIMPACT_ADDON		emADDON;					//	부가효과 종류.
		float				fADDON_VAR[MAX_LEVEL];		//	변화 수치.

		//	상태이상 설정.
		EMSTATE_BLOW		emSTATE_BLOW;				//	상태 이상 적용 타입.
		SKILL::SSTATE_BLOW	sSTATE_BLOW[MAX_LEVEL];		//	상태 이상 수치들.

		//	특수기능 설정.
		EMSPEC_ADDON		emSPEC;						//	특수.
		SKILL::SSPEC_108	sSPEC[MAX_LEVEL];			//	특수 기능 설정값.
	};

	struct SAPPLY_200
	{
		//	기본 적용값 설정.
		SKILL::EMTYPES		emBASIC_TYPE;				//	적용값 타입.
		EMELEMENT			emELEMENT;					//	( 수, 광, 화, 독, 없음 ) 속성.
		DWORD				dwCUREFLAG;					//	해독 종류.
		SKILL::CDATA_LVL_101	sDATA_LVL[MAX_LEVEL];		//	레벨별 설정 값.

		EMSTATE_BLOW		emSTATE_BLOW;				//	상태 이상 적용 타입.
		SKILL::SSTATE_BLOW	sSTATE_BLOW[MAX_LEVEL];		//	상태 이상 수치들.

		SKILL::SIMPACTS_108	sImpacts[MAX_IMPACT];
		SKILL::SSPECS_108	sSpecs[MAX_SPEC];

		SAPPLY_200 (void) :
			emBASIC_TYPE(EMFOR_HP),
			emELEMENT(EMELEMENT_SPIRIT),
			dwCUREFLAG(NULL),
			emSTATE_BLOW(EMBLOW_NONE)
		{
			for ( int i=0; i<MAX_IMPACT; ++i )
				sImpacts[i] = SKILL::SIMPACTS_108();

			for ( int i=0; i<MAX_SPEC; ++i )
					sSpecs[i] = SKILL::SSPECS_108();
		}
	};

	struct SAPPLY_202
	{
		//	기본 적용값 설정.
		SKILL::EMTYPES		emBASIC_TYPE;				//	적용값 타입.
		EMELEMENT			emELEMENT;					//	( 수, 광, 화, 독, 없음 ) 속성.
		SKILL::CDATA_LVL_102	sDATA_LVL[MAX_LEVEL];		//	레벨별 설정 값.
		DWORD				dwCUREFLAG;					//	해독 종류.

		EMSTATE_BLOW		emSTATE_BLOW;				//	상태 이상 적용 타입.
		SKILL::SSTATE_BLOW	sSTATE_BLOW[MAX_LEVEL];		//	상태 이상 수치들.

		SKILL::SIMPACTS_108		sImpacts[MAX_IMPACT];
		SKILL::SSPECS_108		sSpecs[MAX_SPEC];

		DWORD				dwUnknownData;
	};

	struct SAPPLY_203
	{
		enum { VERSION = 0x0203, };

		//	기본 적용값 설정.
		SKILL::EMTYPES		emBASIC_TYPE;				//	적용값 타입.
		EMELEMENT			emELEMENT;					//	( 수, 광, 화, 독, 없음 ) 속성.
		SKILL::CDATA_LVL	sDATA_LVL[MAX_LEVEL];		//	레벨별 설정 값.
		DWORD				dwCUREFLAG;					//	해독 종류.

		EMSTATE_BLOW		emSTATE_BLOW;				//	상태 이상 적용 타입.
		SKILL::SSTATE_BLOW	sSTATE_BLOW[MAX_LEVEL];		//	상태 이상 수치들.

		SKILL::SIMPACTS		sImpacts[MAX_IMPACT];
		SKILL::SSPECS_109		sSpecs[MAX_SPEC];

		DWORD				dwUnknownData;

		float				fRunningEffTime;

	};

	struct SAPPLY
	{
		enum { VERSION = 0x0204, };

		//	기본 적용값 설정.
		SKILL::EMTYPES		emBASIC_TYPE;				//	적용값 타입.
		EMELEMENT			emELEMENT;					//	( 수, 광, 화, 독, 없음 ) 속성.
		SKILL::CDATA_LVL	sDATA_LVL[MAX_LEVEL];		//	레벨별 설정 값.
		DWORD				dwCUREFLAG;					//	해독 종류.

		EMSTATE_BLOW		emSTATE_BLOW;				//	상태 이상 적용 타입.
		SKILL::SSTATE_BLOW	sSTATE_BLOW[MAX_LEVEL];		//	상태 이상 수치들.

		SKILL::SIMPACTS		sImpacts[MAX_IMPACT];
		SKILL::SSPECS		sSpecs[MAX_SPEC];

		DWORD				dwUnknownData;

		float				fRunningEffTime;

		SAPPLY (void) :
			emBASIC_TYPE(EMFOR_HP),
			emELEMENT(EMELEMENT_SPIRIT),
			dwCUREFLAG(NULL),
			emSTATE_BLOW(EMBLOW_NONE),
			dwUnknownData(0),
			fRunningEffTime(0.0f)
		{
			for ( int i=0; i<MAX_IMPACT; ++i )
				sImpacts[i] = SKILL::SIMPACTS();

			for ( int i=0; i<MAX_SPEC; ++i )
				sSpecs[i] = SKILL::SSPECS();
		}

		void Assign ( SAPPLY_100 &sOldData );
		void Assign ( SAPPLY_101 &sOldData );
		void Assign ( SAPPLY_102 &sOldData );
		void Assign ( SAPPLY_103 &sOldData );
		void Assign ( SAPPLY_200 &sOldData );
		void Assign ( SAPPLY_202 &sOldData );
		void Assign ( SAPPLY_203 &sOldData );

		BOOL LOAD_0106 ( basestream &SFile );
		BOOL LOAD_0107 ( basestream &SFile );
		BOOL LOAD_0108 ( basestream &SFile );
		BOOL LOAD_0109 ( basestream &SFile );
		BOOL LOAD_0110 ( basestream &SFile );
		BOOL LOAD_0111 ( basestream &SFile );

		BOOL LOAD_0201 ( basestream &SFile );
		BOOL LOAD_0202 ( basestream &SFile );
		BOOL LOAD_0203 ( basestream &SFile );

		BOOL LOAD ( basestream &SFile );
		BOOL SAVE ( CSerialFile &SFile );

		static VOID SaveCsvHead ( std::fstream &SFile );
		VOID SaveCsv ( std::fstream &SFile );
		VOID LoadCsv ( CStringArray &StrArray, int &iCsvCur );
		VOID LoadCsv_oldj ( CStringArray &StrArray, int &iCsvCur );

		BOOL IsImpact( EMIMPACT_ADDON emImpact );
		BOOL IsSpec( EMSPEC_ADDON emSpec );

		
		std::vector<SIMPACTS>	vecADDON;			//	부가효과
		std::vector<SSPECS>		vecSPEC;			//	특수효과
	};

	struct SKILLDELAY
	{
		float fDelayTime;
		float fMAXDelayTime;
		float fDefultMaxDelayTime;
		SKILLDELAY();
	};

	typedef std::map<DWORD, SKILLDELAY> DELAY_MAP;
	typedef DELAY_MAP::iterator	              DELAY_MAP_ITER;
};

namespace COMMENT
{
	extern std::string SKILL_TYPES[SKILL::FOR_TYPE_SIZE];
	extern float SKILL_TYPES_SCALE[SKILL::FOR_TYPE_SIZE];
	inline bool IsSKILL_PER ( SKILL::EMTYPES emTYPE )			{ return SKILL_TYPES_SCALE[emTYPE]==100.0f; }
};

#endif // GLSKILL_APPLY_H_