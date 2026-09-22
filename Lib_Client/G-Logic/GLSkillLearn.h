#ifndef GLSKILL_LEARN_H_
#define GLSKILL_LEARN_H_

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
	//	래밸 별 습득 조건.
	struct SLEARN_LVL_0100_0101
	{
		DWORD				dwSKP;					//	요구 스킬 포인트.
		DWORD				dwLEVEL;				//	요구 Level.
		SCHARSTATS			sSTATS;					//	요구 Stats.
		DWORD				dwSKILL_LVL;			//	요구 보유스킬 래벨.
	};

	struct SLEARN_LVL
	{
		DWORD				dwLEVEL;				//	요구 Level.
		SCHARSTATS			sSTATS;					//	요구 Stats.
		DWORD				dwSKP;					//	요구 스킬 포인트.
		DWORD				dwSKILL_LVL;			//	요구 보유스킬 래벨.

		SLEARN_LVL (void) :
			dwLEVEL(1),
			dwSKP(1),
			dwSKILL_LVL(0)
		{
		}

		void Assign ( SLEARN_LVL_0100_0101 &OldData )
		{
			dwLEVEL		= OldData.dwLEVEL;
			sSTATS		= OldData.sSTATS;
			dwSKP		= OldData.dwSKP;
			dwSKILL_LVL	= OldData.dwSKILL_LVL;
		}
	};

	struct SLEARN_100
	{
		EMBRIGHT			emBRIGHT;					//	속성.
		DWORD				dwCLASS;					//	직업.
		SNATIVEID			sSKILL;						//	요구 보유스킬.
		SLEARN_LVL_0100_0101	sLVL_STEP[MAX_LEVEL];		//	레벨 별 요구 수치.
	};

	struct SLEARN_101
	{
		EMBRIGHT			emBRIGHT;					//	속성.
		DWORD				dwCLASS;					//	직업.
		SNATIVEID			sSKILL;						//	요구 보유스킬.
		SLEARN_LVL_0100_0101	sLVL_STEP[MAX_LEVEL];	//	레벨 별 요구 수치.
	};

	struct SLEARN_102
	{
		DWORD				dwCLASS;					//	직업.
		EMBRIGHT			emBRIGHT;					//	속성.
		SNATIVEID			sSKILL;						//	요구 보유스킬.
		SLEARN_LVL		sLVL_STEP[MAX_LEVEL];		//	레벨 별 요구 수치.
	};

	struct SLEARN
	{
		enum { VERSION = 0x0104, };

		DWORD				dwCLASS;					//	직업.
		EMBRIGHT			emBRIGHT;					//	속성.
		SNATIVEID			sSKILL;						//	요구 보유스킬.

		SLEARN_LVL			sLVL_STEP[MAX_LEVEL];		//	레벨 별 요구 수치.

		BOOL				bOnlyOneStats;				//	스탯 한가지 만족 시 가능 유무
		SNATIVEID			sHiddenWeapon;				//	스킬 사용 시 2차무기 출력 아이템
		BOOL				bHiddenWeapon;				//	스킬 사용 2차무기 출력 유무

		SLEARN (void) :
			emBRIGHT(BRIGHT_BOTH),
			dwCLASS(GLCC_NONE),
			sSKILL(NATIVEID_NULL()),
			bOnlyOneStats(FALSE),
			sHiddenWeapon(NATIVEID_NULL()),
			bHiddenWeapon(FALSE)
		{
		}

		void Assign ( SLEARN_100 &OldData )
		{
			dwCLASS		= OldData.dwCLASS;
			emBRIGHT	= OldData.emBRIGHT;
			sSKILL		= OldData.sSKILL;

			for ( int i=0; i<MAX_LEVEL; ++i )
				sLVL_STEP[i].dwSKP = 1;
		}

		void Assign ( SLEARN_101 &OldData )
		{
			dwCLASS		= OldData.dwCLASS;
			emBRIGHT	= OldData.emBRIGHT;
			sSKILL		= OldData.sSKILL;

			for ( int i=0; i<MAX_LEVEL; ++i )
				sLVL_STEP[i].Assign( OldData.sLVL_STEP[i] );
		}

		void Assign ( SLEARN_102 &OldData )
		{
			dwCLASS		= OldData.dwCLASS;
			emBRIGHT	= OldData.emBRIGHT;
			sSKILL		= OldData.sSKILL;

			for ( int i=0; i<MAX_LEVEL; ++i )
				sLVL_STEP[i] = OldData.sLVL_STEP[i];
		}

		static VOID SaveCsvHead ( std::fstream &SFile );
		VOID SaveCsv ( std::fstream &SFile );
		VOID LoadCsv ( CStringArray &StrArray, int &iCsvCur );
		VOID LoadCsv_oldj ( CStringArray &StrArray, int &iCsvCur );
	};
};

#endif // GLSKILL_LEARN_H_