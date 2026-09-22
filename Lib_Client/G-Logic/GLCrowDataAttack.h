#ifndef GLCROWDATA_ATTACK_H_
#define GLCROWDATA_ATTACK_H_

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

struct SCROWATTACK_100
{
	enum { VERSION = 0x0100, };

	BOOL			bUsed;						//	사용가능.
	EMATT_RGTYPE	emAttRgType;				//	공격가능거리 유형.
	WORD			wRange;						//	공격가능 거리.
	WORD			wSphere;					//	공격영향 범위.

	//	Skill 의 영향 받는 목표.
	EMIMPACT_TAR	emImpact_Tar;				//	타겟. ( 자신, 대상, 위치 ) &&
	EMIMPACT_REALM	emImpact_Realm;				//	영역. ( 타겟, 타겟과 그 주위, 그 주위 ) &&
	EMIMPACT_SIDE	emImpact_Side;				//	"편" ( 그 주위의 자기편, 그 주위의 상대편 )

	EMELEMENT		emElement;					//	공격 속성.
	GLPADATA		sDamage;					//	타격값.
	float			fDelay;						//	딜래이.
	float			fLife;						//	지속 시간.

	WORD			wUse_MP;					//	사용시 MP 소진량.
	WORD			wUse_SP;					//	사용시 SP 소진량.

	EMSTATE_BLOW	emBlow;						//	상태이상.
	float			fBlowRATE;					//	발생 확율.
	float			fBlowVALUE;					//	적용 값.

	EMIMPACT_ADDON	emImpact;					//	부가효과 종류.
	int				nImpactVAR;					//	변화 수치.

	char			szAniFile[ACF_SZNAME];		//	에니메이션 파일 이름.
	SANIATTACK		sAniAttack;					//	에니메이션 스텝.

	char			szSelfBodyEffect[ACF_SZNAME];	//	자기 자신의 몸에 붓는 이팩트.
	char			szTargBodyEffect[ACF_SZNAME];	//	목표 유닛의 몸에 붓는 이팩트.
	char			szTargetEffect[ACF_SZNAME];		//	목표 지향 이팩트.
};

//----------------------------------------------------------------------
struct SCROWATTACK_101
{
	BOOL			bUsed;						//	사용가능.
	EMATT_RGTYPE	emAttRgType;				//	공격가능거리 유형.
	WORD			wRange;						//	공격가능 거리.

	GLPADATA		sDamage;					//	타격값.
	float			fDelay;						//	딜래이.

	WORD			wUse_SP;					//	사용시 SP 소진량.

	char			szAniFile[ACF_SZNAME];		//	에니메이션 파일 이름.
	SANIATTACK		sAniAttack;					//	에니메이션 스텝.

	SNATIVEID		skill_id;					//	설정된 스킬 종류.

	char			szSelfBodyEffect[ACF_SZNAME];	//	자기 자신의 몸에 붓는 이팩트.
	char			szTargBodyEffect[ACF_SZNAME];	//	목표 유닛의 몸에 붓는 이팩트.
	char			szTargetEffect[ACF_SZNAME];		//	목표 지향 이팩트.
};

struct SCROWATTACK_102
{
	BOOL			bUsed;						//	사용가능.
	EMATT_RGTYPE	emAttRgType;				//	공격가능거리 유형.
	WORD			wRange;						//	공격가능 거리.

	GLPADATA		sDamage;					//	타격값.
	float			fDelay;						//	딜래이.

	WORD			wUse_SP;					//	사용시 SP 소진량.

	char			szAniFile[ACF_SZNAME];		//	에니메이션 파일 이름.
	SANIATTACK		sAniAttack;					//	에니메이션 스텝.

	SNATIVEID		skill_id;					//	설정된 스킬 종류.

	EMSTATE_BLOW	emBLOW_TYPE;				//	'상태이상' 적용 타입.
	float			fBLOW_RATE;					//	발생 확율.
	float			fBLOW_LIFE;					//	지속 시간.
	float			fBLOW_VAR1;					//	적용 값 1.
	float			fBLOW_VAR2;					//	적용 값 2.

	char			szSelfBodyEffect[ACF_SZNAME];	//	자기 자신의 몸에 붓는 이팩트.
	char			szTargBodyEffect[ACF_SZNAME];	//	목표 유닛의 몸에 붓는 이팩트.
	char			szTargetEffect[ACF_SZNAME];		//	목표 지향 이팩트.
};

enum EMCROWSKTAR
{
	CROWSKTAR_OUR	= 0,
	CROWSKTAR_ENEMY	= 1,
	CROWSKTAR_SIZE	= 2,
};

struct SCROWATTACK_103
{
	BOOL			bUsed;						//	사용가능.
	EMATT_RGTYPE	emAttRgType;				//	공격가능거리 유형.
	WORD			wRange;						//	공격가능 거리.

	GLPADATA		sDamage;					//	타격값.
	float			fDelay;						//	딜래이.

	WORD			wUse_SP;					//	사용시 SP 소진량.

	char			szAniFile[ACF_SZNAME];		//	에니메이션 파일 이름.
	SANIATTACK_100	sAniAttack;					//	에니메이션 스텝.

	SNATIVEID		skill_id;					//	스킬 종류.
	WORD			skill_lev;					//	스킬 래벨.
	EMCROWSKTAR		skill_tar;					//	스킬 타겟.

	EMSTATE_BLOW	emBLOW_TYPE;				//	'상태이상' 적용 타입.
	float			fBLOW_RATE;					//	발생 확율.
	float			fBLOW_LIFE;					//	지속 시간.
	float			fBLOW_VAR1;					//	적용 값 1.
	float			fBLOW_VAR2;					//	적용 값 2.

	char			szSelfBodyEffect[ACF_SZNAME];	//	자기 자신의 몸에 붓는 이팩트.
	char			szTargBodyEffect[ACF_SZNAME];	//	목표 유닛의 몸에 붓는 이팩트.
	char			szTargetEffect[ACF_SZNAME];		//	목표 지향 이팩트.
};

struct SCROWATTACK_104
{
	BOOL			bUsed;						//	사용가능.
	EMATT_RGTYPE	emAttRgType;				//	공격가능거리 유형.
	WORD			wRange;						//	공격가능 거리.

	GLPADATA		sDamage;					//	타격값.
	float			fDelay;						//	딜래이.

	WORD			wUse_SP;					//	사용시 SP 소진량.

	char			szAniFile[ACF_SZNAME];		//	에니메이션 파일 이름.
	SANIATTACK		sAniAttack;					//	에니메이션 스텝.

	SNATIVEID		skill_id;					//	스킬 종류.
	WORD			skill_lev;					//	스킬 래벨.
	EMCROWSKTAR		skill_tar;					//	스킬 타겟.

	EMSTATE_BLOW	emBLOW_TYPE;				//	'상태이상' 적용 타입.
	float			fBLOW_RATE;					//	발생 확율.
	float			fBLOW_LIFE;					//	지속 시간.
	float			fBLOW_VAR1;					//	적용 값 1.
	float			fBLOW_VAR2;					//	적용 값 2.

	char			szSelfBodyEffect[ACF_SZNAME];	//	자기 자신의 몸에 붓는 이팩트.
	char			szTargBodyEffect[ACF_SZNAME];	//	목표 유닛의 몸에 붓는 이팩트.
	char			szTargetEffect[ACF_SZNAME];		//	목표 지향 이팩트.
};

struct SCROWATTACK_105
{
	BOOL			bUsed;						//	사용가능.
	EMATT_RGTYPE	emAttRgType;				//	공격가능거리 유형.
	WORD			wRange;						//	공격가능 거리.

	GLPADATA		sDamage;					//	타격값.
	float			fDelay;						//	딜래이.

	WORD			wUse_SP;					//	사용시 SP 소진량.

	std::string		strAniFile;					//	에니메이션 파일 이름.
	SANIATTACK		sAniAttack;					//	에니메이션 스텝.

	SNATIVEID		skill_id;					//	스킬 종류.
	WORD			skill_lev;					//	스킬 래벨.
	EMCROWSKTAR		skill_tar;					//	스킬 타겟.

	EMSTATE_BLOW	emBLOW_TYPE;				//	'상태이상' 적용 타입.
	float			fBLOW_RATE;					//	발생 확율.
	float			fBLOW_LIFE;					//	지속 시간.
	float			fBLOW_VAR1;					//	적용 값 1.
	float			fBLOW_VAR2;					//	적용 값 2.

	std::string		strSelfBodyEffect;			//	자기 자신의 몸에 붓는 이팩트.
	std::string		strTargBodyEffect;			//	목표 유닛의 몸에 붓는 이팩트.
	std::string		strTargetEffect;			//	목표 지향 이팩트.
};

//----------------------------------------------------------------------
struct SCROWATTACK
{
	enum { VERSION = 0x0200, };

	BOOL			bUsed;						//	사용가능.
	EMATT_RGTYPE	emAttRgType;				//	공격가능거리 유형.
	WORD			wRange;						//	공격가능 거리.
	WORD			wUse_SP;					//	사용시 SP 소진량.
	GLPADATA		sDamage;					//	타격값.
	float			fDelay;						//	딜래이.
	SNATIVEID		skill_id;					//	스킬 종류.
	WORD			skill_lev;					//	스킬 래벨.
	EMCROWSKTAR		skill_tar;					//	스킬 타겟.
	EMSTATE_BLOW	emBLOW_TYPE;				//	'상태이상' 적용 타입.
	float			fBLOW_RATE;					//	발생 확율.
	float			fBLOW_LIFE;					//	지속 시간.
	float			fBLOW_VAR1;					//	적용 값 1.
	float			fBLOW_VAR2;					//	적용 값 2.
	std::string		strAniFile;					//	에니메이션 파일 이름.
	SANIATTACK		sAniAttack;					//	에니메이션 스텝.
	std::string		strSelfBodyEffect;			//	자기 자신의 몸에 붓는 이팩트.
	std::string		strTargBodyEffect;			//	목표 유닛의 몸에 붓는 이팩트.
	std::string		strTargetEffect;			//	목표 지향 이팩트.

	SCROWATTACK () :
		bUsed(FALSE),
		emAttRgType(EMATT_SHORT),
		wRange(4),
		wUse_SP(0),
		sDamage(4,6),
		fDelay(0.2f),
		skill_id(NATIVEID_NULL()),
		skill_lev(0),
		skill_tar(CROWSKTAR_ENEMY),
		emBLOW_TYPE(EMBLOW_NONE),
		fBLOW_RATE(0.0f),
		fBLOW_LIFE(0.0f),
		fBLOW_VAR1(0.0f),
		fBLOW_VAR2(0.0f)
	{
	}

	SCROWATTACK (const SCROWATTACK& value )
	{
		operator= ( value );
	}

	SCROWATTACK& operator= ( const SCROWATTACK_100 &OldData );
	SCROWATTACK& operator= ( const SCROWATTACK_101 &OldData );
	SCROWATTACK& operator= ( const SCROWATTACK_102 &OldData );
	SCROWATTACK& operator= ( const SCROWATTACK_103 &OldData );
	SCROWATTACK& operator= ( const SCROWATTACK_104 &OldData );
	SCROWATTACK& operator= ( const SCROWATTACK &value );

	bool LOAD_0105 ( basestream &SFile );
	bool LOAD_0107 ( basestream &SFile );
	bool LOAD ( basestream &SFile );
	bool SAVE ( CSerialFile &SFile );

	static VOID SaveCsvHead ( std::fstream &SFile, int iIndex );
	VOID SaveCsv ( std::fstream &SFile );
	VOID LoadCsv ( CStringArray &StrArray, int& iCsvCur );
};

namespace COMMENT
{
	extern std::string CROWSKTAR[CROWSKTAR_SIZE];
};

#endif // GLCROWDATA_ATTACK_H_