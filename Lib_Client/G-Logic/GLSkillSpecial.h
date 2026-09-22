#ifndef GLSKILL_SPECIAL_H_
#define GLSKILL_SPECIAL_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"

#include "GLSkillDefine.h"

namespace SKILL
{
	enum EMTRANSFORM_PART
	{
		EMTRANSFORM_HEAD	= 0,		    // 핼멧
		EMTRANSFORM_UPPER	= 1,			// 상체
		EMTRANSFORM_LOWER	= 2,			// 하체
		EMTRANSFORM_HAND	= 3,			// 손
		EMTRANSFORM_LHAND	= 4,			// 왼손 장비
		EMTRANSFORM_RHAND	= 5,			// 오른손 장비
		EMTRANSFORM_FOOT	= 6,			// 다리 장비
		EMTRANSFORM_FACE	= 7,			// 얼굴		
		EMTRANSFORM_NSIZE	= 8,				
	};

	enum EMSPECIALSKILLTYPE
	{
		EMSSTYPE_NONE		= 0,		//	사용안함
		EMSSTYPE_TRANSFORM	= 1,		//	변신
		EMSSTYPE_SUMMON		= 2,
		EMSSTYPE_NSIZE		= 3,
	};

	struct SSKILL_SUMMON_DATA
	{
		SNATIVEID	sidCrow;	//summon crow
		float		fDelay;		//time to summon
		float		fLife;		//summon lifetime
		DWORD		dwMaxNum;	//maximum summon num for this skill
		WORD		wSkillLevel; //summon skill level
		DWORD				dwType;				// 소환 타입;
		bool				bEventSkill;		// 이벤트 스킬 반응 여부;

		SSKILL_SUMMON_DATA() :
			sidCrow(NATIVEID_NULL()),
			fDelay(0.0f),
			fLife(0.0f),
			dwMaxNum(0),
			wSkillLevel(0),
			dwType(0),
			bEventSkill(false)
		{
		}
	};

	struct SSPECIAL_SKILL
	{
		enum { VERSION = 0x0104, };

		EMSPECIALSKILLTYPE	emSSTYPE;					//  특수 스킬 타입
		std::string			strEffectName;				//  종료전 스킬 이름

		DWORD				dwRemainSecond;				//  남은 초
		std::string			strTransform_Man[EMTRANSFORM_NSIZE];	//	변신시 사용하는 파일이름
		std::string			strTransform_Woman[EMTRANSFORM_NSIZE];	//	변신시 사용하는 파일이름

		SSKILL_SUMMON_DATA	sSummon[MAX_LEVEL];

		SSPECIAL_SKILL() :
			emSSTYPE(EMSSTYPE_NONE),
			dwRemainSecond(0)
		{
			for( int i=0; i<MAX_LEVEL; ++i )
				sSummon[i] = SSKILL_SUMMON_DATA(); 
		}

		void ResetAll()
		{
			strEffectName.clear();
			dwRemainSecond = 0;
			for( BYTE i = 0; i < SKILL::EMTRANSFORM_NSIZE; i++ )
			{
				strTransform_Man[i].clear();
				strTransform_Woman[i].clear();
			}

			for( int i=0; i<MAX_LEVEL; ++i )
				sSummon[i] = SSKILL_SUMMON_DATA(); 
		}

		BOOL SAVE ( CSerialFile &SFile );
		BOOL LOAD ( basestream &SFile );
		BOOL LOAD_0100 ( basestream &SFile );
		BOOL LOAD_0101 ( basestream &SFile );
		BOOL LOAD_0103 ( basestream &SFile );
		
		static VOID SaveCsvHead ( std::fstream &SFile );
		VOID SaveCsv ( std::fstream &SFile );
		VOID LoadCsv ( CStringArray &StrArray, int &iCsvCur );
		VOID LoadCsv_oldj ( CStringArray &StrArray, int &iCsvCur );

		SSPECIAL_SKILL & operator= ( const SSPECIAL_SKILL  &value );

	};
};

namespace COMMENT
{
	extern std::string SPECIAL_SKILL_TYPE[SKILL::EMSSTYPE_NSIZE];
};

#endif // GLSKILL_SPECIAL_H_