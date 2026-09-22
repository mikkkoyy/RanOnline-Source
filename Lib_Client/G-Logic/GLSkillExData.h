#ifndef GLSKILL_EXDATA_H_
#define GLSKILL_EXDATA_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"

#include "../../Lib_Engine/Meshs/DxAniKeys.h"
#include "./GLSkillDefine.h"
#include "./GLCharDefine.h"

namespace SKILL
{
	enum EMEFFECTIME
	{
		EMTIME_FIRST	= 0,		//	스킬 시작과 동시에.
		EMTIME_IMPACT	= 1,		//	스킬 타격 지점.
		EMTIME_TARG_OVR	= 2,		//	스킬 타격 이팩트가 끝나는 지점.
		EMTIME_PASSIVE	= 3,		//	타겟에 부착된 스킬용.
		EMTIME_REV01	= 4,
		EMTIME_EFFECT_ACTIVATED	= 5, /*skill activated effect, Juver, 2017/12/29 */
		EMTIME_NSIZE	= 6,
	};

	enum EMEFFECTPOS
	{
		EMPOS_GROUND	= 0,		//	지면 위치.
		EMPOS_STRIKE	= 1,		//	스트라이크 위치. ( 무기, 손에 지정된 때리는(발생하는) 위치. )

		EMPOS_NSIZE		= 2,
	};

	struct SEXT_DATA_100
	{
		EMANI_MAINTYPE		emANIMTYPE;						//	스킬 사용시 캐릭터의 에니메이션 종류.		
		EMANI_SUBTYPE		emANISTYPE;						//	스킬 사용시 캐릭터의 에니메이션 종류.		

		EMEFFECTIME			emTARG;
		char				szTARG[MAX_SZFILE];				//	목표 지향 이팩트.

		EMEFFECTIME			emSELFZONE01;
		char				szSELFZONE01[MAX_SZFILE];		//	자기 위치 이팩트.

		EMEFFECTIME			emTARGZONE01;
		char				szTARGZONE01[MAX_SZFILE];		//	목표 위치 이팩트.
		EMEFFECTIME			emTARGZONE02;
		char				szTARGZONE02[MAX_SZFILE];		//	목표 위치 이팩트.

		EMEFFECTIME			emSELFBODY;
		char				szSELFBODY[MAX_SZFILE];			//	자기 자신의 몸에 붓는 이팩트.
		EMEFFECTIME			emTARGBODY01;
		char				szTARGBODY01[MAX_SZFILE];		//	목표 유닛의 몸에 붓는 이팩트.
		EMEFFECTIME			emTARGBODY02;
		char				szTARGBODY02[MAX_SZFILE];		//	목표 유닛의 몸에 붓는 이팩트.

		SNATIVEID			sICONINDEX;						//	아이콘 인덱스.
		char				szICONFILE[MAX_SZFILE];			//	아이콘 텍스쳐 파일 이름.

		char				szCOMMENTS[MAX_SZCOMMENT];		//	스킬에 대한 간단한 설명.
	};

	struct SEXT_DATA_101
	{
		EMANI_MAINTYPE		emANIMTYPE;						//	스킬 사용시 캐릭터의 에니메이션 종류.		
		EMANI_SUBTYPE		emANISTYPE;						//	스킬 사용시 캐릭터의 에니메이션 종류.		

		EMEFFECTIME			emTARG;
		char				szTARG[MAX_SZFILE];				//	목표 지향 이팩트.

		EMEFFECTIME			emSELFZONE01;
		char				szSELFZONE01[MAX_SZFILE];		//	자기 위치 이팩트.

		EMEFFECTIME			emSELFZONE02;
		char				szSELFZONE02[MAX_SZFILE];		//	자기 위치 이팩트.

		EMEFFECTIME			emTARGZONE01;
		char				szTARGZONE01[MAX_SZFILE];		//	목표 위치 이팩트.

		EMEFFECTIME			emTARGZONE02;
		char				szTARGZONE02[MAX_SZFILE];		//	목표 위치 이팩트.

		EMEFFECTIME			emSELFBODY;
		char				szSELFBODY[MAX_SZFILE];			//	자기 자신의 몸에 붓는 이팩트.
		EMEFFECTIME			emTARGBODY01;
		char				szTARGBODY01[MAX_SZFILE];		//	목표 유닛의 몸에 붓는 이팩트.
		EMEFFECTIME			emTARGBODY02;
		char				szTARGBODY02[MAX_SZFILE];		//	목표 유닛의 몸에 붓는 이팩트.

		SNATIVEID			sICONINDEX;						//	아이콘 인덱스.
		char				szICONFILE[MAX_SZFILE];			//	아이콘 텍스쳐 파일 이름.

		char				szCOMMENTS[MAX_SZCOMMENT];		//	스킬에 대한 간단한 설명.
	};

	struct SEXT_DATA_102
	{
		EMANI_MAINTYPE		emANIMTYPE;						//	스킬 사용시 캐릭터의 에니메이션 종류.		
		EMANI_SUBTYPE		emANISTYPE;						//	스킬 사용시 캐릭터의 에니메이션 종류.		

		EMEFFECTIME			emTARG;
		EMEFFECTPOS			emTARG_POSA;
		EMEFFECTPOS			emTARG_POSB;
		char				szTARG[MAX_SZFILE];				//	목표 지향 이팩트.

		EMEFFECTIME			emSELFZONE01;
		EMEFFECTPOS			emSELFZONE01_POS;
		char				szSELFZONE01[MAX_SZFILE];		//	자기 위치 이팩트.

		EMEFFECTIME			emSELFZONE02;
		EMEFFECTPOS			emSELFZONE02_POS;
		char				szSELFZONE02[MAX_SZFILE];		//	자기 위치 이팩트.

		EMEFFECTIME			emTARGZONE01;
		EMEFFECTPOS			emTARGZONE01_POS;
		char				szTARGZONE01[MAX_SZFILE];		//	목표 위치 이팩트.

		EMEFFECTIME			emTARGZONE02;
		EMEFFECTPOS			emTARGZONE02_POS;
		char				szTARGZONE02[MAX_SZFILE];		//	목표 위치 이팩트.

		EMEFFECTIME			emSELFBODY;
		char				szSELFBODY[MAX_SZFILE];			//	자기 자신의 몸에 붓는 이팩트.
		EMEFFECTIME			emTARGBODY01;
		char				szTARGBODY01[MAX_SZFILE];		//	목표 유닛의 몸에 붓는 이팩트.
		EMEFFECTIME			emTARGBODY02;
		char				szTARGBODY02[MAX_SZFILE];		//	목표 유닛의 몸에 붓는 이팩트.

		SNATIVEID			sICONINDEX;						//	아이콘 인덱스.
		char				szICONFILE[MAX_SZFILE];			//	아이콘 텍스쳐 파일 이름.

		char				szCOMMENTS[MAX_SZCOMMENT];		//	스킬에 대한 간단한 설명.
	};

	struct SEXT_DATA_103
	{
		EMANI_MAINTYPE		emANIMTYPE;								//	스킬 사용시 캐릭터의 에니메이션 종류.		
		EMANI_SUBTYPE		emANISTYPE;								//	스킬 사용시 캐릭터의 에니메이션 종류.		

		EMEFFECTIME			emTARG;
		EMEFFECTPOS			emTARG_POSA;
		EMEFFECTPOS			emTARG_POSB;
		std::string			strTARG[EMELEMENT_OLDMAX];				//	목표 지향 이팩트.

		EMEFFECTIME			emSELFZONE01;
		EMEFFECTPOS			emSELFZONE01_POS;
		std::string			strSELFZONE01[EMELEMENT_OLDMAX];		//	자기 위치 이팩트.

		EMEFFECTIME			emSELFZONE02;
		EMEFFECTPOS			emSELFZONE02_POS;
		std::string			strSELFZONE02[EMELEMENT_OLDMAX];		//	자기 위치 이팩트.

		EMEFFECTIME			emTARGZONE01;
		EMEFFECTPOS			emTARGZONE01_POS;
		std::string			strTARGZONE01[EMELEMENT_OLDMAX];		//	목표 위치 이팩트.

		EMEFFECTIME			emTARGZONE02;
		EMEFFECTPOS			emTARGZONE02_POS;
		std::string			strTARGZONE02[EMELEMENT_OLDMAX];		//	목표 위치 이팩트.

		EMEFFECTIME			emSELFBODY;
		std::string			strSELFBODY[EMELEMENT_OLDMAX];			//	자기 자신의 몸에 붓는 이팩트.

		EMEFFECTIME			emTARGBODY01;
		std::string			strTARGBODY01[EMELEMENT_OLDMAX];		//	목표 유닛의 몸에 붓는 이팩트.

		EMEFFECTIME			emTARGBODY02;
		std::string			strTARGBODY02[EMELEMENT_OLDMAX];		//	목표 유닛의 몸에 붓는 이팩트.

		std::string			strHOLDOUT;								//	방어 이펙트.

		SNATIVEID			sICONINDEX;								//	아이콘 인덱스.
		std::string			strICONFILE;							//	아이콘 텍스쳐 파일 이름.

		std::string			strCOMMENTS;							//	스킬에 대한 간단한 설명.

		BOOL LOAD ( basestream &SFile );
	};

	struct SEXT_DATA_104
	{
		EMANI_MAINTYPE		emANIMTYPE;								//	스킬 사용시 캐릭터의 에니메이션 종류.		
		EMANI_SUBTYPE		emANISTYPE;								//	스킬 사용시 캐릭터의 에니메이션 종류.		

		float				fDELAY4DAMAGE;							//	대미지 발생 딜래이.
		EMEFFECTIME			emTARG;
		EMEFFECTPOS			emTARG_POSA;
		EMEFFECTPOS			emTARG_POSB;
		std::string			strTARG[EMELEMENT_OLDMAX];				//	목표 지향 이팩트.

		EMEFFECTIME			emSELFZONE01;
		EMEFFECTPOS			emSELFZONE01_POS;
		std::string			strSELFZONE01[EMELEMENT_OLDMAX];		//	자기 위치 이팩트.

		EMEFFECTIME			emSELFZONE02;
		EMEFFECTPOS			emSELFZONE02_POS;
		std::string			strSELFZONE02[EMELEMENT_OLDMAX];		//	자기 위치 이팩트.

		EMEFFECTIME			emTARGZONE01;
		EMEFFECTPOS			emTARGZONE01_POS;
		std::string			strTARGZONE01[EMELEMENT_OLDMAX];		//	목표 위치 이팩트.

		EMEFFECTIME			emTARGZONE02;
		EMEFFECTPOS			emTARGZONE02_POS;
		std::string			strTARGZONE02[EMELEMENT_OLDMAX];		//	목표 위치 이팩트.

		EMEFFECTIME			emSELFBODY;
		std::string			strSELFBODY[EMELEMENT_OLDMAX];			//	자기 자신의 몸에 붓는 이팩트.

		EMEFFECTIME			emTARGBODY01;
		std::string			strTARGBODY01[EMELEMENT_OLDMAX];		//	목표 유닛의 몸에 붓는 이팩트.

		EMEFFECTIME			emTARGBODY02;
		std::string			strTARGBODY02[EMELEMENT_OLDMAX];		//	목표 유닛의 몸에 붓는 이팩트.

		std::string			strHOLDOUT;								//	방어 이펙트.

		SNATIVEID			sICONINDEX;								//	아이콘 인덱스.
		std::string			strICONFILE;							//	아이콘 텍스쳐 파일 이름.

		std::string			strCOMMENTS;							//	스킬에 대한 간단한 설명.

		BOOL LOAD ( basestream &SFile );
	};

	struct SEXT_DATA_105
	{
		EMANI_MAINTYPE		emANIMTYPE;								//	스킬 사용시 캐릭터의 에니메이션 종류.		
		EMANI_SUBTYPE		emANISTYPE;								//	스킬 사용시 캐릭터의 에니메이션 종류.		

		float				fDELAY4DAMAGE;							//	대미지 발생 딜래이.
		EMEFFECTIME			emTARG;
		EMEFFECTPOS			emTARG_POSA;
		EMEFFECTPOS			emTARG_POSB;
		bool				bTARG_ONE;								//	목표 지향 이펙트 단일 생성.
		std::string			strTARG[EMELEMENT_OLDMAX];				//	목표 지향 이팩트.

		EMEFFECTIME			emSELFZONE01;
		EMEFFECTPOS			emSELFZONE01_POS;
		std::string			strSELFZONE01[EMELEMENT_OLDMAX];		//	자기 위치 이팩트.

		EMEFFECTIME			emSELFZONE02;
		EMEFFECTPOS			emSELFZONE02_POS;
		std::string			strSELFZONE02[EMELEMENT_OLDMAX];		//	자기 위치 이팩트.

		EMEFFECTIME			emTARGZONE01;
		EMEFFECTPOS			emTARGZONE01_POS;
		std::string			strTARGZONE01[EMELEMENT_OLDMAX];		//	목표 위치 이팩트.

		EMEFFECTIME			emTARGZONE02;
		EMEFFECTPOS			emTARGZONE02_POS;
		std::string			strTARGZONE02[EMELEMENT_OLDMAX];		//	목표 위치 이팩트.

		EMEFFECTIME			emSELFBODY;
		std::string			strSELFBODY[EMELEMENT_OLDMAX];			//	자기 자신의 몸에 붓는 이팩트.

		EMEFFECTIME			emTARGBODY01;
		std::string			strTARGBODY01[EMELEMENT_OLDMAX];		//	목표 유닛의 몸에 붓는 이팩트.

		EMEFFECTIME			emTARGBODY02;
		std::string			strTARGBODY02[EMELEMENT_OLDMAX];		//	목표 유닛의 몸에 붓는 이팩트.

		std::string			strHOLDOUT;								//	방어 이펙트.

		SNATIVEID			sICONINDEX;								//	아이콘 인덱스.
		std::string			strICONFILE;							//	아이콘 텍스쳐 파일 이름.

		std::string			strCOMMENTS;							//	스킬에 대한 간단한 설명.

		BOOL LOAD ( basestream &SFile );

	};

	struct SEXT_DATA_106
	{
		EMANI_MAINTYPE		emANIMTYPE;								//	스킬 사용시 캐릭터의 에니메이션 종류.		
		EMANI_SUBTYPE		emANISTYPE;								//	스킬 사용시 캐릭터의 에니메이션 종류.		

		float				fDELAY4DAMAGE;							//	대미지 발생 딜래이.
		EMEFFECTIME			emTARG;
		EMEFFECTPOS			emTARG_POSA;
		EMEFFECTPOS			emTARG_POSB;
		bool				bTARG_ONE;								//	목표 지향 이펙트 단일 생성.
		std::string			strTARG[EMELEMENT_MAXNUM];				//	목표 지향 이팩트.

		EMEFFECTIME			emSELFZONE01;
		EMEFFECTPOS			emSELFZONE01_POS;
		std::string			strSELFZONE01[EMELEMENT_MAXNUM];		//	자기 위치 이팩트.

		EMEFFECTIME			emSELFZONE02;
		EMEFFECTPOS			emSELFZONE02_POS;
		std::string			strSELFZONE02[EMELEMENT_MAXNUM];		//	자기 위치 이팩트.

		EMEFFECTIME			emTARGZONE01;
		EMEFFECTPOS			emTARGZONE01_POS;
		std::string			strTARGZONE01[EMELEMENT_MAXNUM];		//	목표 위치 이팩트.

		EMEFFECTIME			emTARGZONE02;
		EMEFFECTPOS			emTARGZONE02_POS;
		std::string			strTARGZONE02[EMELEMENT_MAXNUM];		//	목표 위치 이팩트.

		EMEFFECTIME			emSELFBODY;
		std::string			strSELFBODY[EMELEMENT_MAXNUM];			//	자기 자신의 몸에 붓는 이팩트.

		EMEFFECTIME			emTARGBODY01;
		std::string			strTARGBODY01[EMELEMENT_MAXNUM];		//	목표 유닛의 몸에 붓는 이팩트.

		EMEFFECTIME			emTARGBODY02;
		std::string			strTARGBODY02[EMELEMENT_MAXNUM];		//	목표 유닛의 몸에 붓는 이팩트.

		std::string			strHOLDOUT;								//	방어 이펙트.

		SNATIVEID			sICONINDEX;								//	아이콘 인덱스.
		std::string			strICONFILE;							//	아이콘 텍스쳐 파일 이름.

		std::string			strCOMMENTS;							//	스킬에 대한 간단한 설명.

		BOOL LOAD ( basestream &SFile );
	};

	struct SEXT_DATA_107
	{
		EMANI_MAINTYPE		emANIMTYPE;								//	스킬 사용시 캐릭터의 에니메이션 종류.		
		EMANI_SUBTYPE		emANISTYPE;								//	스킬 사용시 캐릭터의 에니메이션 종류.		

		SNATIVEID			sICONINDEX;								//	아이콘 인덱스.
		std::string			strICONFILE;							//	아이콘 텍스쳐 파일 이름.

		std::string			strCOMMENTS;							//	스킬에 대한 간단한 설명.

		EMEFFECTIME			emTARG;
		EMEFFECTPOS			emTARG_POSA;
		EMEFFECTPOS			emTARG_POSB;

		float				fDELAY4DAMAGE;							//	대미지 발생 딜래이.

		std::string			strHOLDOUT;								//	방어 이펙트.

		EMEFFECTIME			emSELFZONE01;
		EMEFFECTPOS			emSELFZONE01_POS;
		std::string			strSELFZONE01[EMELEMENT_MAXNUM];		//	자기 위치 이팩트.

		EMEFFECTIME			emSELFZONE02;
		EMEFFECTPOS			emSELFZONE02_POS;
		std::string			strSELFZONE02[EMELEMENT_MAXNUM];		//	자기 위치 이팩트.

		bool				bTARG_ONE;								//	목표 지향 이펙트 단일 생성.
		std::string			strTARG[EMELEMENT_MAXNUM];				//	목표 지향 이팩트.

		EMEFFECTIME			emTARGZONE01;
		EMEFFECTPOS			emTARGZONE01_POS;
		std::string			strTARGZONE01[EMELEMENT_MAXNUM];		//	목표 위치 이팩트.

		EMEFFECTIME			emTARGZONE02;
		EMEFFECTPOS			emTARGZONE02_POS;
		std::string			strTARGZONE02[EMELEMENT_MAXNUM];		//	목표 위치 이팩트.

		EMEFFECTIME			emTARGBODY01;
		std::string			strTARGBODY01[EMELEMENT_MAXNUM];		//	목표 유닛의 몸에 붓는 이팩트.

		EMEFFECTIME			emTARGBODY02;
		std::string			strTARGBODY02[EMELEMENT_MAXNUM];		//	목표 유닛의 몸에 붓는 이팩트.

		EMEFFECTIME			emSELFBODY;
		std::string			strSELFBODY[EMELEMENT_MAXNUM];			//	자기 자신의 몸에 붓는 이팩트.

		BOOL LOAD ( basestream &SFile );
	};

	struct SEXT_DATA_108
	{
		EMANI_MAINTYPE		emANIMTYPE;								//	스킬 사용시 캐릭터의 에니메이션 종류.		
		EMANI_SUBTYPE		emANISTYPE;								//	스킬 사용시 캐릭터의 에니메이션 종류.	
		DWORD				dwVal;		//official unknown data

		SNATIVEID			sICONINDEX;								//	아이콘 인덱스.
		std::string			strICONFILE;							//	아이콘 텍스쳐 파일 이름.

		std::string			strCOMMENTS;							//	스킬에 대한 간단한 설명.

		EMEFFECTIME			emTARG;
		EMEFFECTPOS			emTARG_POSA;
		EMEFFECTPOS			emTARG_POSB;

		float				fDELAY4DAMAGE;							//	대미지 발생 딜래이.

		std::string			strHOLDOUT;								//	방어 이펙트.

		EMEFFECTIME			emSELFZONE01;
		EMEFFECTPOS			emSELFZONE01_POS;
		std::string			strSELFZONE01[EMELEMENT_MAXNUM];		//	자기 위치 이팩트.

		EMEFFECTIME			emSELFZONE02;
		EMEFFECTPOS			emSELFZONE02_POS;
		std::string			strSELFZONE02[EMELEMENT_MAXNUM];		//	자기 위치 이팩트.

		bool				bTARG_ONE;								//	목표 지향 이펙트 단일 생성.
		std::string			strTARG[EMELEMENT_MAXNUM];				//	목표 지향 이팩트.

		EMEFFECTIME			emTARGZONE01;
		EMEFFECTPOS			emTARGZONE01_POS;
		std::string			strTARGZONE01[EMELEMENT_MAXNUM];		//	목표 위치 이팩트.

		EMEFFECTIME			emTARGZONE02;
		EMEFFECTPOS			emTARGZONE02_POS;
		std::string			strTARGZONE02[EMELEMENT_MAXNUM];		//	목표 위치 이팩트.

		EMEFFECTIME			emTARGBODY01;
		std::string			strTARGBODY01[EMELEMENT_MAXNUM];		//	목표 유닛의 몸에 붓는 이팩트.

		EMEFFECTIME			emTARGBODY02;
		std::string			strTARGBODY02[EMELEMENT_MAXNUM];		//	목표 유닛의 몸에 붓는 이팩트.

		EMEFFECTIME			emSELFBODY;
		std::string			strSELFBODY[EMELEMENT_MAXNUM];			//	자기 자신의 몸에 붓는 이팩트.

		BOOL LOAD ( basestream &SFile );
	};

	struct SEXT_DATA_109
	{
		EMANI_MAINTYPE		emANIMTYPE;								//	스킬 사용시 캐릭터의 에니메이션 종류.		
		EMANI_SUBTYPE		emANISTYPE;								//	스킬 사용시 캐릭터의 에니메이션 종류.	
		DWORD				dwVal;		//official unknown data

		SNATIVEID			sICONINDEX;								//	아이콘 인덱스.
		std::string			strICONFILE;							//	아이콘 텍스쳐 파일 이름.

		std::string			strCOMMENTS;							//	스킬에 대한 간단한 설명.

		EMEFFECTIME			emTARG;
		EMEFFECTPOS			emTARG_POSA;
		EMEFFECTPOS			emTARG_POSB;

		float				fDELAY4DAMAGE;							//	대미지 발생 딜래이.

		std::string			strHOLDOUT;								//	방어 이펙트.

		EMEFFECTIME			emSELFZONE01;
		EMEFFECTPOS			emSELFZONE01_POS;
		std::string			strSELFZONE01[EMELEMENT_MAXNUM];		//	자기 위치 이팩트.

		EMEFFECTIME			emSELFZONE02;
		EMEFFECTPOS			emSELFZONE02_POS;
		std::string			strSELFZONE02[EMELEMENT_MAXNUM];		//	자기 위치 이팩트.

		EMEFFECTIME			emSELFZONE03;
		EMEFFECTPOS			emSELFZONE03_POS;
		std::string			strSELFZONE03[EMELEMENT_MAXNUM];		//	자기 위치 이팩트.

		bool				bTARG_ONE;								//	목표 지향 이펙트 단일 생성.
		std::string			strTARG[EMELEMENT_MAXNUM];				//	목표 지향 이팩트.

		EMEFFECTIME			emTARGZONE01;
		EMEFFECTPOS			emTARGZONE01_POS;
		std::string			strTARGZONE01[EMELEMENT_MAXNUM];		//	목표 위치 이팩트.

		EMEFFECTIME			emTARGZONE02;
		EMEFFECTPOS			emTARGZONE02_POS;
		std::string			strTARGZONE02[EMELEMENT_MAXNUM];		//	목표 위치 이팩트.

		EMEFFECTIME			emTARGZONE03;
		EMEFFECTPOS			emTARGZONE03_POS;
		std::string			strTARGZONE03[EMELEMENT_MAXNUM];		//	목표 위치 이팩트.

		EMEFFECTIME			emTARGBODY01;
		std::string			strTARGBODY01[EMELEMENT_MAXNUM];		//	목표 유닛의 몸에 붓는 이팩트.

		EMEFFECTIME			emTARGBODY02;
		std::string			strTARGBODY02[EMELEMENT_MAXNUM];		//	목표 유닛의 몸에 붓는 이팩트.

		EMEFFECTIME			emSELFBODY;
		std::string			strSELFBODY[EMELEMENT_MAXNUM];			//	자기 자신의 몸에 붓는 이팩트.

		SNATIVEID			sADDSKILL;
		DWORD				dwADDSKILLLEVEL;

		SNATIVEID			sLINKSKILL;

		BOOL LOAD ( basestream &SFile );
	};

	struct SEXT_DATA
	{
		enum { VERSION = 0x0110, };

		EMANI_MAINTYPE		emANIMTYPE;								//	스킬 사용시 캐릭터의 에니메이션 종류.		
		EMANI_SUBTYPE		emANISTYPE;								//	스킬 사용시 캐릭터의 에니메이션 종류.	
		DWORD				dwVal;		//official unknown data

		SNATIVEID			sICONINDEX;								//	아이콘 인덱스.
		std::string			strICONFILE;							//	아이콘 텍스쳐 파일 이름.

		std::string			strCOMMENTS;							//	스킬에 대한 간단한 설명.

		EMEFFECTIME			emTARG;
		EMEFFECTPOS			emTARG_POSA;
		EMEFFECTPOS			emTARG_POSB;

		float				fDELAY4DAMAGE;							//	대미지 발생 딜래이.

		std::string			strHOLDOUT;								//	방어 이펙트.

		EMEFFECTIME			emSELFZONE01;
		EMEFFECTPOS			emSELFZONE01_POS;
		std::string			strSELFZONE01[EMELEMENT_MAXNUM];		//	자기 위치 이팩트.

		EMEFFECTIME			emSELFZONE02;
		EMEFFECTPOS			emSELFZONE02_POS;
		std::string			strSELFZONE02[EMELEMENT_MAXNUM];		//	자기 위치 이팩트.

		EMEFFECTIME			emSELFZONE03;
		EMEFFECTPOS			emSELFZONE03_POS;
		std::string			strSELFZONE03[EMELEMENT_MAXNUM];		//	자기 위치 이팩트.

		bool				bTARG_ONE;								//	목표 지향 이펙트 단일 생성.
		std::string			strTARG[EMELEMENT_MAXNUM];				//	목표 지향 이팩트.

		EMEFFECTIME			emTARGZONE01;
		EMEFFECTPOS			emTARGZONE01_POS;
		std::string			strTARGZONE01[EMELEMENT_MAXNUM];		//	목표 위치 이팩트.

		EMEFFECTIME			emTARGZONE02;
		EMEFFECTPOS			emTARGZONE02_POS;
		std::string			strTARGZONE02[EMELEMENT_MAXNUM];		//	목표 위치 이팩트.

		EMEFFECTIME			emTARGZONE03;
		EMEFFECTPOS			emTARGZONE03_POS;
		std::string			strTARGZONE03[EMELEMENT_MAXNUM];		//	목표 위치 이팩트.

		EMEFFECTIME			emTARGBODY01;
		std::string			strTARGBODY01[EMELEMENT_MAXNUM];		//	목표 유닛의 몸에 붓는 이팩트.

		EMEFFECTIME			emTARGBODY02;
		std::string			strTARGBODY02[EMELEMENT_MAXNUM];		//	목표 유닛의 몸에 붓는 이팩트.

		EMEFFECTIME			emSELFBODY;
		std::string			strSELFBODY[EMELEMENT_MAXNUM];			//	자기 자신의 몸에 붓는 이팩트.

		SNATIVEID			sADDSKILL;
		DWORD				dwADDSKILLLEVEL;

		SNATIVEID			sLINKSKILL;

		inline void LIMIT ( EMELEMENT &emElement )
		{
			GASSERT(emElement<EMELEMENT_MAXNUM2);
			if (emElement==EMELEMENT_MAXNUM) emElement=EMELEMENT_SPIRIT;
		}

		bool VALIDTARG ( EMELEMENT emElement )					{  LIMIT(emElement); return strTARG[emElement].length()!=0; }

		const char* GETTARG ( EMELEMENT emElement )				{  LIMIT(emElement); return strTARG[emElement].c_str(); }
		const char* GETSELFZONE01 ( EMELEMENT emElement )		{  LIMIT(emElement); return strSELFZONE01[emElement].c_str(); }
		const char* GETSELFZONE02 ( EMELEMENT emElement )		{  LIMIT(emElement); return strSELFZONE02[emElement].c_str(); }
		const char* GETSELFZONE03 ( EMELEMENT emElement )		{  LIMIT(emElement); return strSELFZONE03[emElement].c_str(); }

		const char* GETTARGZONE01 ( EMELEMENT emElement )		{  LIMIT(emElement); return strTARGZONE01[emElement].c_str(); }
		const char* GETTARGZONE02 ( EMELEMENT emElement )		{  LIMIT(emElement); return strTARGZONE02[emElement].c_str(); }
		const char* GETTARGZONE03 ( EMELEMENT emElement )		{  LIMIT(emElement); return strTARGZONE03[emElement].c_str(); }

		const char* GETSELFBODY ( EMELEMENT emElement )			{  LIMIT(emElement); return strSELFBODY[emElement].c_str(); }

		const char* GETTARGBODY01 ( EMELEMENT emElement )		{  LIMIT(emElement); return strTARGBODY01[emElement].c_str(); }
		const char* GETTARGBODY02 ( EMELEMENT emElement )		{  LIMIT(emElement); return strTARGBODY02[emElement].c_str(); }

		SEXT_DATA (void) :
			emANIMTYPE(AN_SKILL_A),
			emANISTYPE(AN_SUB_00),
			dwVal(7), //default val from official
			sICONINDEX(0,0),
			emTARG_POSA(EMPOS_GROUND),
			emTARG_POSB(EMPOS_GROUND),
			fDELAY4DAMAGE(0.0f),
			emSELFZONE01_POS(EMPOS_GROUND),
			emSELFZONE02_POS(EMPOS_GROUND),
			emSELFZONE03_POS(EMPOS_GROUND),
			emTARGZONE01_POS(EMPOS_GROUND),
			emTARGZONE02_POS(EMPOS_GROUND),
			emTARGZONE03_POS(EMPOS_GROUND),
			bTARG_ONE(false),
			emTARG(EMTIME_IMPACT),
			emSELFZONE01(EMTIME_FIRST),
			emSELFZONE02(EMTIME_IMPACT),
			emSELFZONE03(EMTIME_IMPACT),
			emTARGZONE01(EMTIME_TARG_OVR),
			emTARGZONE02(EMTIME_PASSIVE),
			emTARGZONE03(EMTIME_PASSIVE),
			emTARGBODY01(EMTIME_TARG_OVR),
			emTARGBODY02(EMTIME_PASSIVE),
			emSELFBODY(EMTIME_FIRST),
			sADDSKILL(NATIVEID_NULL()),
			dwADDSKILLLEVEL(0),
			sLINKSKILL(NATIVEID_NULL())
		{
		}

		void Assign ( const SEXT_DATA_100 &OldData );
		void Assign ( const SEXT_DATA_101 &OldData );
		void Assign ( const SEXT_DATA_102 &OldData );
		void Assign ( const SEXT_DATA_103 &OldData );
		void Assign ( const SEXT_DATA_104 &OldData );
		void Assign ( const SEXT_DATA_105 &OldData );
		void Assign ( const SEXT_DATA_106 &OldData );
		void Assign ( const SEXT_DATA_107 &OldData );
		void Assign ( const SEXT_DATA_108 &OldData );
		void Assign ( const SEXT_DATA_109 &OldData );

		BOOL SAVE ( CSerialFile &SFile );
		BOOL LOAD ( basestream &SFile );

		static VOID SaveCsvHead ( std::fstream &SFile );
		VOID SaveCsv ( std::fstream &SFile );
		VOID LoadCsv ( CStringArray &StrArray, int &iCsvCur );
		VOID LoadCsv_oldj ( CStringArray &StrArray, int &iCsvCur );

		SEXT_DATA& operator= ( const SEXT_DATA &value );
	};
};

namespace COMMENT
{
	extern std::string SKILL_EFFTIME[SKILL::EMTIME_NSIZE];
	extern std::string SKILL_EFFPOS[SKILL::EMPOS_NSIZE];
};

#endif // GLSKILL_EXDATA_H_