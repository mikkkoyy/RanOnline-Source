#ifndef GLSKILL_BASIC_H_
#define GLSKILL_BASIC_H_

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

namespace gloldskill_102
{
	enum GLSKILL_ATT_102
	{
		SKILLATT_NOTHING	= 0,	//	착용무기없음.
		SKILLATT_SWORD		= 1,	//	검류.
		SKILLATT_REV00		= 2,	//	REV00
		SKILLATT_DAGGER		= 3,	//	단검류.
		SKILLATT_SPEAR		= 4,	//	창류.
		SKILLATT_BOW		= 5,	//	활류.
		SKILLATT_THROW		= 6,	//	투척류.
		SKILLATT_NOCARE		= 7,	//	타입 상관 안함.	( "GLSKILL" 에서 Hand 무기 제한 속성에서 사용됨. )
		SKILLATT_NSIZE		= 7,	//	종류갯수.
		SKILLATT_NEAR		= 4,	//	근접 공격.
	};
};

namespace gloldskill_103
{
	enum GLSKILL_ATT_103
	{
		SKILLATT_NOTHING	= 0,	//	착용무기없음.
		SKILLATT_SWORD		= 1,	//	검.
		SKILLATT_BLADE		= 2,	//	도.
		SKILLATT_DAGGER		= 3,	//	단검.
		SKILLATT_SPEAR		= 4,	//	창.
		SKILLATT_STICK		= 5,	//	몽둥이.
		SKILLATT_BOW		= 6,	//	활.
		SKILLATT_THROW		= 7,	//	투척.
		SKILLATT_NOCARE		= 8,	//	타입 상관 안함.	( "GLSKILL" 에서 Hand 무기 제한 속성에서 사용됨. )
		SKILLATT_NSIZE		= 8,	//	종류갯수.
		SKILLATT_NEAR		= 5,	//	근접 공격.
	};
};

namespace gloldskill_108
{
	enum GLSKILL_ATT_108
	{
		SKILLATT_NOTHING	= 0,	//
		SKILLATT_SWORD		= 1,	//
		SKILLATT_BLADE		= 2,	//
		SKILLATT_SWORDBLADE	= 3,	//
		SKILLATT_DAGGER		= 4,	//
		SKILLATT_SPEAR		= 5,	//
		SKILLATT_STICK		= 6,	//
		SKILLATT_GAUNT		= 7,	//
		SKILLATT_BOW		= 8,	//
		SKILLATT_THROW		= 9,	//
		SKILLATT_NOCARE		= 10,	//
		SKILLATT_NSIZE		= 10,	//
		SKILLATT_NEAR		= 7,	//
	};
};

namespace gloldskill_110
{
	enum GLSKILL_ATT_110
	{
		SKILLATT_NOTHING	= 0,	//
		SKILLATT_SWORD		= 1,	//
		SKILLATT_BLADE		= 2,	//
		SKILLATT_SWORDBLADE	= 3,	//
		SKILLATT_DAGGER		= 4,	//
		SKILLATT_SPEAR		= 5,	//
		SKILLATT_STICK		= 6,	//
		SKILLATT_GAUNT		= 7,	//
		SKILLATT_BOW		= 8,	//
		SKILLATT_THROW		= 9,	//
		SKILLATT_GUN		= 10,	//
		SKILLATT_RAILGUN	= 11,	//
		SKILLATT_PORTALGUN	= 12,	//
		SKILLATT_SHOTGUN	= 13,	//
		SKILLATT_RAILPORTALGUN	= 14,	//
		SKILLATT_NOCARE		= 15,	//
		SKILLATT_NSIZE		= 15,	//
	};
};


enum GLSKILL_ATT
{
	SKILLATT_NOTHING	= 0,	//
	SKILLATT_SWORD		= 1,	//
	SKILLATT_BLADE		= 2,	//
	SKILLATT_SWORDBLADE	= 3,	//
	SKILLATT_DAGGER		= 4,	//
	SKILLATT_SPEAR		= 5,	//
	SKILLATT_STICK		= 6,	//
	SKILLATT_GAUNT		= 7,	//
	SKILLATT_BOW		= 8,	//
	SKILLATT_THROW		= 9,	//
	SKILLATT_DUAL_GUN	= 10,	//
	SKILLATT_RAIL_GUN	= 11,	//
	SKILLATT_PORTAL_GUN	= 12,	//
	SKILLATT_GUN		= 13,	//
	SKILLATT_SHOTGUN	= 14,	//
	SKILLATT_SCYTHE		= 15,
	SKILLATT_DUALSPEAR	= 16,
	SKILLATT_SHURIKEN	= 17,
	SKILLATT_FIST		= 18,
	SKILLATT_WAND		= 19,
	SKILLATT_CUBE		= 20,
	SKILLATT_WHIP		= 21,
	SKILLATT_NOCARE		= 22,	//
	SKILLATT_SHIELD		= 23,
	SKILLATT_HAMMER		= 24,
	SKILLATT_UMBRELLA	= 25,
	SKILLATT_NSIZE		= 26,	//
};

namespace SKILL
{
	enum EMROLE
	{
		EMROLE_NORMAL	= 0,		//	즉시.
		EMROLE_PASSIVE	= 1,		//	페시브 계열. ( 사용하는 것은 아니고 배웠을때 다른 값에 영향 )
		EMROLE_NSIZE	= 2,
	};

	enum EMAPPLY	//	물리력 가산시 효능.
	{
		EMAPPLY_PHY_SHORT	= 0,	//	물리 타격형.
		EMAPPLY_PHY_LONG	= 1,	//	물리 사격형.
		EMAPPLY_MAGIC		= 2,	//	마법 작용형.

		EMAPPLY_NSIZE		= 3
	};

	enum EMAPPLY_DWORD
	{
		EMAPPLY_DWORD_NULL		= 0x0,
		EMAPPLY_DWORD_MELEE	= (1 << EMAPPLY_PHY_SHORT),		// 격투;
		EMAPPLY_DWORD_RANGE	= (1 << EMAPPLY_PHY_LONG),		// 사격;
		EMAPPLY_DWORD_MAGIC	= (1 << EMAPPLY_MAGIC),		// 마력;
	};

	enum EMACTION
	{
		EMACTION_NORMAL		= 0,
		EMACTION_BUFF		= 1,
		EMACTION_DEBUFF		= 2,
		EMACTION_LIMIT		= 3,
		EMACTION_NSIZE		= 4,
	};

	enum EMACTION_TYPE_DWORD // 행동 타입 비트 플래그
	{
		EMACTION_TYPE_DWORD_NULL    = 0x00,
		EMACTION_TYPE_DWORD_NORMAL	= (1 << EMACTION_NORMAL),  // 일반
		EMACTION_TYPE_DWORD_BUFF    = (1 << EMACTION_BUFF),    // 버프
		EMACTION_TYPE_DWORD_DEBUFF	= (1 << EMACTION_DEBUFF),	// 디버프
		EMACTION_TYPE_DWORD_LIMIT   = (1 << EMACTION_LIMIT),	// 리미트
	};

	GLSKILL_ATT assign_att ( gloldskill_102::GLSKILL_ATT_102 emOld );
	GLSKILL_ATT assign_att ( gloldskill_103::GLSKILL_ATT_103 emOld );
	GLSKILL_ATT assign_att ( gloldskill_108::GLSKILL_ATT_108 emOld );
	GLSKILL_ATT assign_att ( gloldskill_110::GLSKILL_ATT_110 emOld );

	struct SSKILLBASIC_100
	{
		SNATIVEID			sNATIVEID;					//	고유 ID. ( 최초 생성시에 부여되는 고유 ID )

		char				szNAME[MAX_SZNAME];			//	스킬의 이름.
		DWORD				dwGRADE;					//	등급.

		EMROLE				emROLE;						//	스킬 동작 특성. ( 즉시, 지속, 페시브 )
		EMAPPLY				emAPPLY;					//	스킬 유형. ( 물리공격, 마법공격, 마법 보조 )

		//	Skill 의 영향 받는 목표.
		EMIMPACT_TAR		emIMPACT_TAR;				//	타겟. ( 자신, 대상, 위치 ) &&
		EMIMPACT_REALM		emIMPACT_REALM;				//	영역. ( 타겟, 타겟과 그 주위, 그 주위 ) &&
		EMIMPACT_SIDE		emIMPACT_SIDE;				//	"편" ( 그 주위의 자기편, 그 주위의 상대편 )

		WORD				wTARRANGE;					//	목표 가능 거리.

		gloldskill_102::GLSKILL_ATT_102	emUSE_LITEM;			//	사용가능 왼손 도구 속성.
		gloldskill_102::GLSKILL_ATT_102	emUSE_RITEM;			//	사용가능 오른손 도구 속성.
	};

	struct SSKILLBASIC_101
	{
		SNATIVEID			sNATIVEID;					//	고유 ID. ( 최초 생성시에 부여되는 고유 ID )

		char				szNAME[MAX_SZNAME];			//	스킬의 이름.
		DWORD				dwGRADE;					//	등급.

		EMROLE				emROLE;						//	스킬 동작 특성. ( 즉시, 지속, 페시브 )
		EMAPPLY				emAPPLY;					//	스킬 유형. ( 물리공격, 마법공격, 마법 보조 )

		//	Skill 의 영향 받는 목표.
		EMIMPACT_TAR		emIMPACT_TAR;				//	타겟. ( 자신, 대상, 위치 ) &&
		EMIMPACT_REALM		emIMPACT_REALM;				//	영역. ( 타겟, 타겟과 그 주위, 그 주위 ) &&
		EMIMPACT_SIDE		emIMPACT_SIDE;				//	"편" ( 그 주위의 자기편, 그 주위의 상대편 )

		WORD				wTARRANGE;					//	목표 가능 거리.

		gloldskill_102::GLSKILL_ATT_102	emUSE_LITEM;			//	사용가능 왼손 도구 속성.
		gloldskill_102::GLSKILL_ATT_102	emUSE_RITEM;			//	사용가능 오른손 도구 속성.
	};

	struct SSKILLBASIC_102
	{
		SNATIVEID			sNATIVEID;					//	고유 ID. ( 최초 생성시에 부여되는 고유 ID )

		char				szNAME[MAX_SZNAME];			//	스킬의 이름.
		DWORD				dwGRADE;					//	등급.

		EMROLE				emROLE;						//	스킬 동작 특성. ( 즉시, 지속, 페시브 )
		EMAPPLY				emAPPLY;					//	스킬 유형. ( 물리공격, 마법공격, 마법 보조 )

		//	Skill 의 영향 받는 목표.
		EMIMPACT_TAR		emIMPACT_TAR;				//	타겟. ( 자신, 대상, 위치 ) &&
		EMIMPACT_REALM		emIMPACT_REALM;				//	영역. ( 타겟, 타겟과 그 주위, 그 주위 ) &&
		EMIMPACT_SIDE		emIMPACT_SIDE;				//	"편" ( 그 주위의 자기편, 그 주위의 상대편 )

		WORD				wTARRANGE;					//	목표 가능 거리.

		gloldskill_103::GLSKILL_ATT_103	emUSE_LITEM;			//	사용가능 왼손 도구 속성.
		gloldskill_103::GLSKILL_ATT_103	emUSE_RITEM;			//	사용가능 오른손 도구 속성.
	};

	struct SSKILLBASIC_103
	{
		SNATIVEID			sNATIVEID;					//	고유 ID. ( 최초 생성시에 부여되는 고유 ID )

		char				szNAME[MAX_SZNAME];			//	스킬의 이름.
		DWORD				dwGRADE;					//	등급.

		EMROLE				emROLE;						//	스킬 동작 특성. ( 즉시, 지속, 페시브 )
		EMAPPLY				emAPPLY;					//	스킬 유형. ( 물리공격, 마법공격, 마법 보조 )

		//	Skill 의 영향 받는 목표.
		EMIMPACT_TAR		emIMPACT_TAR;				//	타겟. ( 자신, 대상, 위치 ) &&
		EMIMPACT_REALM		emIMPACT_REALM;				//	영역. ( 타겟, 타겟과 그 주위, 그 주위 ) &&
		EMIMPACT_SIDE		emIMPACT_SIDE;				//	"편" ( 그 주위의 자기편, 그 주위의 상대편 )

		WORD				wTARRANGE;					//	목표 가능 거리.

		gloldskill_108::GLSKILL_ATT_108	emUSE_LITEM;				//	사용가능 왼손 도구 속성.
		gloldskill_108::GLSKILL_ATT_108	emUSE_RITEM;				//	사용가능 오른손 도구 속성.
	};

	struct SSKILLBASIC_105
	{
		SNATIVEID			sNATIVEID;					//	고유 ID. ( 최초 생성시에 부여되는 고유 ID )

		char				szNAME[MAX_SZNAME];			//	스킬의 이름.
		DWORD				dwGRADE;					//	등급.
		DWORD				dwMAXLEVEL;					//	최대 레벨.

		EMROLE				emROLE;						//	스킬 동작 특성. ( 즉시, 지속, 페시브 )
		EMAPPLY				emAPPLY;					//	스킬 유형. ( 물리공격, 마법공격, 마법 보조 )

		//	Skill 의 영향 받는 목표.
		EMIMPACT_TAR		emIMPACT_TAR;				//	타겟. ( 자신, 대상, 위치 ) &&
		EMIMPACT_REALM		emIMPACT_REALM;				//	영역. ( 타겟, 타겟과 그 주위, 그 주위 ) &&
		EMIMPACT_SIDE		emIMPACT_SIDE;				//	"편" ( 그 주위의 자기편, 그 주위의 상대편 )

		WORD				wTARRANGE;					//	목표 가능 거리.

		gloldskill_108::GLSKILL_ATT_108	emUSE_LITEM;				//	사용가능 왼손 도구 속성.
		gloldskill_108::GLSKILL_ATT_108	emUSE_RITEM;				//	사용가능 오른손 도구 속성.
	};

	struct SSKILLBASIC_106
	{
		SNATIVEID			sNATIVEID;					//	고유 ID. ( 최초 생성시에 부여되는 고유 ID )

		char				szNAME[MAX_SZNAME];			//	스킬의 이름.
		DWORD				dwGRADE;					//	등급.
		DWORD				dwMAXLEVEL;					//	최대 레벨.

		EMROLE				emROLE;						//	스킬 동작 특성. ( 즉시, 지속, 페시브 )
		EMAPPLY				emAPPLY;					//	스킬 유형. ( 물리공격, 마법공격, 마법 보조 )

		//	Skill 의 영향 받는 목표.
		EMIMPACT_TAR		emIMPACT_TAR;				//	타겟. ( 자신, 대상, 위치 ) &&
		EMIMPACT_REALM		emIMPACT_REALM;				//	영역. ( 타겟, 타겟과 그 주위, 그 주위 ) &&
		EMIMPACT_SIDE		emIMPACT_SIDE;				//	"편" ( 그 주위의 자기편, 그 주위의 상대편 )

		WORD				wTARRANGE;					//	목표 가능 거리.

		gloldskill_108::GLSKILL_ATT_108	emUSE_LITEM;				//	사용가능 왼손 도구 속성.
		gloldskill_108::GLSKILL_ATT_108	emUSE_RITEM;				//	사용가능 오른손 도구 속성.

		bool				bLearnView;					//  GM만 보이는 스킬
	};

	struct SSKILLBASIC_107
	{
		SNATIVEID			sNATIVEID;					//	고유 ID. ( 최초 생성시에 부여되는 고유 ID )
		bool				bLearnView;					//  GM만 보이는 스킬
		char				szNAME[MAX_SZNAME];			//	스킬의 이름.
		DWORD				dwMAXLEVEL;					//	최대 레벨.
		DWORD				dwGRADE;					//	등급.
		EMAPPLY				emAPPLY;					//	스킬 유형. ( 물리공격, 마법공격, 마법 보조 )
		EMROLE				emROLE;						//	스킬 동작 특성. ( 즉시, 지속, 페시브 )
		WORD				wTARRANGE;					//	목표 가능 거리.
		EMIMPACT_SIDE		emIMPACT_SIDE;				//	"편" ( 그 주위의 자기편, 그 주위의 상대편 )
		EMIMPACT_TAR		emIMPACT_TAR;				//	타겟. ( 자신, 대상, 위치 ) &&
		EMIMPACT_REALM		emIMPACT_REALM;				//	영역. ( 타겟, 타겟과 그 주위, 그 주위 ) &&
		gloldskill_108::GLSKILL_ATT_108	emUSE_LITEM;	//	사용가능 왼손 도구 속성.
		gloldskill_108::GLSKILL_ATT_108	emUSE_RITEM;	//	사용가능 오른손 도구 속성.
	};

	struct SSKILLBASIC_108
	{
		SNATIVEID			sNATIVEID;					//	고유 ID. ( 최초 생성시에 부여되는 고유 ID )
		bool				bLearnView;					//  GM만 보이는 스킬
		char				szNAME[MAX_SZNAME];			//	스킬의 이름.
		DWORD				dwMAXLEVEL;					//	최대 레벨.
		DWORD				dwGRADE;					//	등급.
		EMAPPLY				emAPPLY;					//	스킬 유형. ( 물리공격, 마법공격, 마법 보조 )
		EMROLE				emROLE;						//	스킬 동작 특성. ( 즉시, 지속, 페시브 )
		EMACTION			emACTION;
		WORD				wTARRANGE;					//	목표 가능 거리.
		EMIMPACT_SIDE		emIMPACT_SIDE;				//	"편" ( 그 주위의 자기편, 그 주위의 상대편 )
		EMIMPACT_TAR		emIMPACT_TAR;				//	타겟. ( 자신, 대상, 위치 ) &&
		EMIMPACT_REALM		emIMPACT_REALM;				//	영역. ( 타겟, 타겟과 그 주위, 그 주위 ) &&
		gloldskill_108::GLSKILL_ATT_108	emUSE_LITEM;	//	사용가능 왼손 도구 속성.
		gloldskill_108::GLSKILL_ATT_108	emUSE_RITEM;	//	사용가능 오른손 도구 속성.
	};

	struct SSKILLBASIC_109
	{
		SNATIVEID			sNATIVEID;					//	고유 ID. ( 최초 생성시에 부여되는 고유 ID )
		WORD				wData;						// official unknown data
		bool				bLearnView;					//  GM만 보이는 스킬
		char				szNAME[MAX_SZNAME];			//	스킬의 이름.
		DWORD				dwMAXLEVEL;					//	최대 레벨.
		DWORD				dwGRADE;					//	등급.
		EMAPPLY				emAPPLY;					//	스킬 유형. ( 물리공격, 마법공격, 마법 보조 )
		EMROLE				emROLE;						//	스킬 동작 특성. ( 즉시, 지속, 페시브 )
		EMACTION			emACTION;
		WORD				wTARRANGE;					//	목표 가능 거리.
		EMIMPACT_SIDE		emIMPACT_SIDE;				//	"편" ( 그 주위의 자기편, 그 주위의 상대편 )
		EMIMPACT_TAR		emIMPACT_TAR;				//	타겟. ( 자신, 대상, 위치 ) &&
		EMIMPACT_REALM		emIMPACT_REALM;				//	영역. ( 타겟, 타겟과 그 주위, 그 주위 ) &&
		gloldskill_110::GLSKILL_ATT_110		emUSE_LITEM;				//	사용가능 왼손 도구 속성.
		gloldskill_110::GLSKILL_ATT_110		emUSE_RITEM;				//	사용가능 오른손 도구 속성.
	};

	struct SSKILLBASIC_110
	{
		SNATIVEID			sNATIVEID;					//	고유 ID. ( 최초 생성시에 부여되는 고유 ID )
		WORD				wData;						// official unknown data
		bool				bLearnView;					//  GM만 보이는 스킬
		char				szNAME[MAX_SZNAME];			//	스킬의 이름.
		DWORD				dwMAXLEVEL;					//	최대 레벨.
		DWORD				dwGRADE;					//	등급.
		EMAPPLY				emAPPLY;					//	스킬 유형. ( 물리공격, 마법공격, 마법 보조 )
		EMROLE				emROLE;						//	스킬 동작 특성. ( 즉시, 지속, 페시브 )
		EMACTION			emACTION;
		WORD				wTARRANGE;					//	목표 가능 거리.
		EMIMPACT_SIDE		emIMPACT_SIDE;				//	"편" ( 그 주위의 자기편, 그 주위의 상대편 )
		EMIMPACT_TAR		emIMPACT_TAR;				//	타겟. ( 자신, 대상, 위치 ) &&
		EMIMPACT_REALM		emIMPACT_REALM;				//	영역. ( 타겟, 타겟과 그 주위, 그 주위 ) &&
		gloldskill_110::GLSKILL_ATT_110		emUSE_LITEM;				//	사용가능 왼손 도구 속성.
		gloldskill_110::GLSKILL_ATT_110		emUSE_RITEM;				//	사용가능 오른손 도구 속성.
	};

	struct SSKILLBASIC_111
	{
		SNATIVEID			sNATIVEID;					//	고유 ID. ( 최초 생성시에 부여되는 고유 ID )
		WORD				wData;						// official unknown data
		bool				bLearnView;					//  GM만 보이는 스킬
		char				szNAME[MAX_SZNAME];			//	스킬의 이름.
		DWORD				dwMAXLEVEL;					//	최대 레벨.
		DWORD				dwGRADE;					//	등급.
		EMAPPLY				emAPPLY;					//	스킬 유형. ( 물리공격, 마법공격, 마법 보조 )
		EMROLE				emROLE;						//	스킬 동작 특성. ( 즉시, 지속, 페시브 )
		EMACTION			emACTION;
		WORD				wTARRANGE;					//	목표 가능 거리.
		EMIMPACT_SIDE		emIMPACT_SIDE;				//	"편" ( 그 주위의 자기편, 그 주위의 상대편 )
		EMIMPACT_TAR		emIMPACT_TAR;				//	타겟. ( 자신, 대상, 위치 ) &&
		EMIMPACT_REALM		emIMPACT_REALM;				//	영역. ( 타겟, 타겟과 그 주위, 그 주위 ) &&
		GLSKILL_ATT			emUSE_LITEM;				//	사용가능 왼손 도구 속성.
		GLSKILL_ATT			emUSE_RITEM;				//	사용가능 오른손 도구 속성.
	};
	
	struct SSKILLBASIC_112
	{
		SNATIVEID			sNATIVEID;					//	고유 ID. ( 최초 생성시에 부여되는 고유 ID )
		WORD				wData;						// official unknown data
		bool				bLearnView;					//  GM만 보이는 스킬
		char				szNAME[MAX_SZNAME];			//	스킬의 이름.
		DWORD				dwMAXLEVEL;					//	최대 레벨.
		DWORD				dwGRADE;					//	등급.
		EMAPPLY				emAPPLY;					//	스킬 유형. ( 물리공격, 마법공격, 마법 보조 )
		EMROLE				emROLE;						//	스킬 동작 특성. ( 즉시, 지속, 페시브 )
		EMACTION			emACTION;
		WORD				wTARRANGE;					//	목표 가능 거리.
		EMIMPACT_SIDE		emIMPACT_SIDE;				//	"편" ( 그 주위의 자기편, 그 주위의 상대편 )
		EMIMPACT_TAR		emIMPACT_TAR;				//	타겟. ( 자신, 대상, 위치 ) &&
		EMIMPACT_REALM		emIMPACT_REALM;				//	영역. ( 타겟, 타겟과 그 주위, 그 주위 ) &&
		GLSKILL_ATT			emUSE_LITEM;				//	사용가능 왼손 도구 속성.
		GLSKILL_ATT			emUSE_RITEM;				//	사용가능 오른손 도구 속성.
		DWORD				dwData;
	};

#define FLAG_SB_RUNNING_CAST                    0x00000001  // 이동하면서 스킬시전 가능여부;
	struct SSKILLBASIC_113
	{
		enum { VERSION = 0x0113, };

		SNATIVEID			sNATIVEID;					//	고유 ID. ( 최초 생성시에 부여되는 고유 ID )
		WORD				wData;						// official unknown data
		bool				bLearnView;					//  GM만 보이는 스킬
		char				szNAME[MAX_SZNAME];			//	스킬의 이름.
		DWORD				dwMAXLEVEL;					//	최대 레벨.
		DWORD				dwGRADE;					//	등급.
		EMAPPLY				emAPPLY;					//	스킬 유형. ( 물리공격, 마법공격, 마법 보조 )
		EMROLE				emROLE;						//	스킬 동작 특성. ( 즉시, 지속, 페시브 )
		EMACTION			emACTION;
		WORD				wTARRANGE;					//	목표 가능 거리.
		EMIMPACT_SIDE		emIMPACT_SIDE;				//	"편" ( 그 주위의 자기편, 그 주위의 상대편 )
		EMIMPACT_TAR		emIMPACT_TAR;				//	타겟. ( 자신, 대상, 위치 ) &&
		EMIMPACT_REALM		emIMPACT_REALM;				//	영역. ( 타겟, 타겟과 그 주위, 그 주위 ) &&
		GLSKILL_ATT			emUSE_LITEM;				//	사용가능 왼손 도구 속성.
		GLSKILL_ATT			emUSE_RITEM;				//	사용가능 오른손 도구 속성.
		DWORD				dwData;
		DWORD				dwFlags;

		SSKILLBASIC_113 (void) :
			bLearnView(FALSE),
			wData(0),
			dwMAXLEVEL(9),
			dwGRADE(0),
			emAPPLY(EMAPPLY_MAGIC),
			emROLE(EMROLE_NORMAL),
			emACTION(EMACTION_NORMAL),
			wTARRANGE(50),
			emIMPACT_SIDE(SIDE_ENEMY),
			emIMPACT_TAR(TAR_SPEC),
			emIMPACT_REALM(REALM_SELF),
			emUSE_LITEM(SKILLATT_NOCARE),
			emUSE_RITEM(SKILLATT_NOCARE),
			dwData(0),
			dwFlags(0)
		{
			SecureZeroMemory ( szNAME, sizeof(char)*MAX_SZNAME );
		}
	};

	struct SSKILLBASIC_114  // 마술부 무기 추가에 따른 데이터 변화;
	{
		enum { VERSION = 0x0114 };
		SNATIVEID			sNATIVEID;					//	고유 ID. ( 최초 생성시에 부여되는 고유 ID )
		bool				bLearnView;					//  GM만 보이는 스킬
		bool				bNonEffectRemove;			//	효과제거 불가 옵션
		bool				bMobEffectRate;				//	몹 효과 비율 적용 옵션. by luxes.

		char				szNAME[MAX_SZNAME];			//	스킬의 이름.

		DWORD				dwMAXLEVEL;					//	최대 레벨.
		DWORD				dwGRADE;					//	등급.

		EMAPPLY				emAPPLY;					//	스킬 유형. ( 물리공격, 마법공격, 마법 보조 )
		EMROLE				emROLE;						//	스킬 동작 특성. ( 즉시, 지속, 페시브 )
		EMACTION			emACTION;				//	스킬 행동타입 ( 일반,버프,디버프,리미트)

		WORD				wTARRANGE;					//	목표 가능 거리.

		//	Skill 의 영향 받는 목표.
		EMIMPACT_SIDE		emIMPACT_SIDE;				//	"편" ( 그 주위의 자기편, 그 주위의 상대편 )
		EMIMPACT_TAR		emIMPACT_TAR;				//	타겟. ( 자신, 대상, 위치 ) &&
		EMIMPACT_REALM		emIMPACT_REALM;				//	영역. ( 타겟, 타겟과 그 주위, 그 주위 ) &&

		GLSKILL_ATT 	emUSE_LITEM;				//	사용가능 왼손 도구 속성.
		GLSKILL_ATT		emUSE_RITEM;				//	사용가능 오른손 도구 속성.

		DWORD				dwFlags;					//

		SSKILLBASIC_114 (void) :
		dwGRADE(0),
			dwMAXLEVEL(9),

			emROLE(EMROLE_NORMAL),
			emAPPLY(EMAPPLY_MAGIC),
			emACTION(EMACTION_NORMAL),

			emIMPACT_TAR(TAR_SPEC),
			emIMPACT_REALM(REALM_SELF),
			emIMPACT_SIDE(SIDE_ENEMY),

			wTARRANGE(50),

			emUSE_LITEM(SKILLATT_NOCARE),
			emUSE_RITEM(SKILLATT_NOCARE),
			bLearnView(false),
			bNonEffectRemove(false),
			bMobEffectRate(false),
			dwFlags(0)
		{
			SecureZeroMemory ( szNAME, sizeof(char)*MAX_SZNAME );
		}
	};

	struct SSKILLBASIC  // 마술부 무기 추가에 따른 데이터 변화;
	{
		enum { VERSION = 0x0115 };
		SNATIVEID			sNATIVEID;					//	고유 ID. ( 최초 생성시에 부여되는 고유 ID )
		bool				bLearnView;					//  GM만 보이는 스킬
		bool				bNonEffectRemove;			//	효과제거 불가 옵션
		bool				bMobEffectRate;				//	몹 효과 비율 적용 옵션. by luxes.

		char				szNAME[MAX_SZNAME];			//	스킬의 이름.

		DWORD				dwMAXLEVEL;					//	최대 레벨.
		DWORD				dwGRADE;					//	등급.

		EMAPPLY				emAPPLY;					//	스킬 유형. ( 물리공격, 마법공격, 마법 보조 )
		EMROLE				emROLE;						//	스킬 동작 특성. ( 즉시, 지속, 페시브 )
		EMACTION			emACTION;				//	스킬 행동타입 ( 일반,버프,디버프,리미트)

		WORD				wTARRANGE;					//	목표 가능 거리.

		//	Skill 의 영향 받는 목표.
		EMIMPACT_SIDE		emIMPACT_SIDE;				//	"편" ( 그 주위의 자기편, 그 주위의 상대편 )
		EMIMPACT_TAR		emIMPACT_TAR;				//	타겟. ( 자신, 대상, 위치 ) &&
		EMIMPACT_REALM		emIMPACT_REALM;				//	영역. ( 타겟, 타겟과 그 주위, 그 주위 ) &&

		GLSKILL_ATT 	emUSE_LITEM;				//	사용가능 왼손 도구 속성.
		GLSKILL_ATT		emUSE_RITEM;				//	사용가능 오른손 도구 속성.

		DWORD				dwFlags;					//

		SSKILLBASIC (void) :
		dwGRADE(0),
			dwMAXLEVEL(9),

			emROLE(EMROLE_NORMAL),
			emAPPLY(EMAPPLY_MAGIC),
			emACTION(EMACTION_NORMAL),

			emIMPACT_TAR(TAR_SPEC),
			emIMPACT_REALM(REALM_SELF),
			emIMPACT_SIDE(SIDE_ENEMY),

			wTARRANGE(50),

			emUSE_LITEM(SKILLATT_NOCARE),
			emUSE_RITEM(SKILLATT_NOCARE),
			bLearnView(false),
			bNonEffectRemove(false),
			bMobEffectRate(false),
			dwFlags(0)
		{
			SecureZeroMemory ( szNAME, sizeof(char)*MAX_SZNAME );
		}

		void Assign ( SSKILLBASIC_100 &sOldData );
		void Assign ( SSKILLBASIC_101 &sOldData );
		void Assign ( SSKILLBASIC_102 &sOldData );
		void Assign ( SSKILLBASIC_103 &sOldData );
		void Assign ( SSKILLBASIC_105 &sOldData );
		void Assign ( SSKILLBASIC_106 &sOldData );
		void Assign ( SSKILLBASIC_107 &sOldData );
		void Assign ( SSKILLBASIC_108 &sOldData );
		void Assign ( SSKILLBASIC_109 &sOldData );
		void Assign ( SSKILLBASIC_110 &sOldData );
		void Assign ( SSKILLBASIC_111 &sOldData );
		void Assign ( SSKILLBASIC_112 &sOldData );
		void Assign ( SSKILLBASIC_113 &sOldData );
		void Assign ( SSKILLBASIC_114 &sOldData );

		inline SNATIVEID GetId() const { return sNATIVEID; }


		static VOID SaveCsvHead ( std::fstream &SFile );
		VOID SaveCsv ( std::fstream &SFile );
		VOID LoadCsv ( CStringArray &StrArray, int &iCsvCur );
		VOID LoadCsv_oldj ( CStringArray &StrArray, int &iCsvCur );
	};
};

namespace COMMENT
{
	extern std::string SKILL_ROLE[SKILL::EMROLE_NSIZE];
	extern std::string SKILL_APPLY[SKILL::EMAPPLY_NSIZE];
	extern std::string SKILLATTACK[SKILLATT_NSIZE];
	extern std::string SKILL_ACTION_TYPE[SKILL::EMACTION_NSIZE];
};

#endif // GLSKILL_BASIC_H_