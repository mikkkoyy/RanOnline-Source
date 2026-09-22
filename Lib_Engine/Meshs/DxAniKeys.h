#ifndef DXANIKEYS_H_
#define DXANIKEYS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
//
//	Note : xFile 에니메이션 데이터의 구조체.
//
//
//
//
//

#define UNITANIKEY_PERSEC	(4800.0f)

// X File formation rotate key
struct SRotateKeyXFile
{
	DWORD			dwTime;
	DWORD			dwFloats;	
	float			w;
	float			x;
	float			y;
	float			z;
};

struct SScaleKeyXFile
{
	DWORD		dwTime; 
	DWORD		dwFloats;
	D3DXVECTOR3	vScale;
};

struct SPositionKeyXFile
{
	DWORD		dwTime;
	DWORD		dwFloats;	
	D3DXVECTOR3	vPos;	
};

struct SMatrixKeyXFile
{
	DWORD		dwTime;
	DWORD		dwFloats;	
	D3DXMATRIX	mat;	
};

// in memory versions
struct SRotateKey
{
	DWORD			dwTime;
	D3DXQUATERNION	quatRotate;	
};

struct SPositionKey
{
	DWORD		dwTime;
	D3DXVECTOR3	vPos;	
};

struct SScaleKey
{
	DWORD		dwTime;
	D3DXVECTOR3	vScale;	
};

struct SMatrixKey
{
	DWORD			dwTime;
	D3DXMATRIXA16	mat;

	SMatrixKey() :
		dwTime(0)
	{
	}

	~SMatrixKey()
	{
	}

	bool operator < ( const SMatrixKey &rvalue ) const
	{
		return dwTime < rvalue.dwTime;
	}
};

struct QUATCOMP
{
	DWORD dwFirst;	// x,y
	DWORD dwSecond;	// z,w

	QUATCOMP()	{}
	QUATCOMP( DWORD a, DWORD b ) :
		dwFirst(a),
		dwSecond(dwSecond)
	{
	}
};

struct SQuatPosKey
{
	DWORD			m_dwTime;
	D3DXVECTOR3		m_vScale;
	D3DXVECTOR3		m_vPos;
	QUATCOMP		m_vQuatComp;
//	D3DXQUATERNION	m_vQuat;
	

	SQuatPosKey() :
		m_dwTime(0L),
		m_vScale(0.f,0.f,0.f),
		m_vPos(0.f,0.f,0.f),
		m_vQuatComp(0,0)
//		m_vQuat(0.f,0.f,0.f,1.f)
	{
	}

	~SQuatPosKey()
	{
	}

	bool operator < ( const SQuatPosKey &rvalue ) const
	{
		return m_dwTime < rvalue.m_dwTime;
	}
};

struct SQuatPos
{
	D3DXVECTOR3		m_vScale;
	D3DXVECTOR3		m_vPos;
	D3DXQUATERNION	m_vQuat;

	SQuatPos() :
		m_vScale(1.f,1.f,1.f),
		m_vPos(0.f,0.f,0.f),
		m_vQuat(0.f,0.f,0.f,0.f)
	{
	}

	~SQuatPos()
	{
	}
};

enum EMANI_MAINTYPE
{
	AN_GUARD_N			= 0,
	AN_PLACID			= 1,
	AN_WALK				= 2,
	AN_RUN				= 3,
	AN_ATTACK			= 4,
	AN_SHOCK			= 5,
	AN_DIE				= 6,
	AN_CREATE			= 7,
	AN_TALK				= 8,
	AN_SKILL_A			= 9,
	AN_GESTURE			= 10,
	AN_SHOCK_MIX		= 11,
	AN_GUARD_L			= 12,
	AN_CONFT_WIN		= 13,
	AN_CONFT_LOSS		= 14,
	AN_SPEC				= 15,
	AN_SKILL_B			= 16,
	AN_SKILL_C			= 17,
	AN_SKILL_D			= 18,
	AN_SKILL_E			= 19,
	AN_GATHERING		= 20,

	AN_BIKE_A			= 21,
	AN_BIKE_B			= 22,
	AN_BIKE_C			= 23,
	AN_BIKE_EV1			= 24,
	AN_FLIGHT			= 25,
	AN_SKILL_F			= 26,
	AN_SKILL_G			= 27,
	AN_SKILL_H			= 28,

	AN_CAR_A			= 29,
	AN_CAR_B			= 30,
	AN_CAR_C			= 31,
	AN_CAR_D			= 32,

	AN_REARCAR_A		= 33,

	//ABL animation (abf)
	AN_ABL_STAY			= 0,
	AN_ABL_ATTACK		= 1,
	AN_ABL_FREE			= 2,

	//vehicle animation (vcf)
	AN_VEHICLE_STAY			= 0,
	AN_VEHICLE_WALK			= 1,
	AN_VEHICLE_BOOSTER		= 2,
	AN_VEHICLE_RUN			= 3,
	AN_VEHICLE_DOWN			= 4,
	AN_VEHICLE_SHOCK		= 5,
	AN_VEHICLE_ATTACK		= 6,
	AN_VEHICLE_SKILL_A		= 7,
	AN_VEHICLE_SKILL_B		= 8,
	AN_VEHICLE_SKILL_C		= 9,
	AN_VEHICLE_SKILL_D		= 10,
	AN_VEHICLE_ANIM_001		= 11,
	AN_VEHICLE_ANIM_002		= 12,
	AN_VEHICLE_ANIM_003		= 13,
	AN_VEHICLE_ANIM_004		= 14,
	AN_VEHICLE_ANIM_005		= 15,
	AN_VEHICLE_ANIM_NONE	= 16,

	AN_NONE				= 34,
	AN_TYPE_SIZE		= 34, //over all maintype size

	//AN_NONE			= 21, //old ep7
	//AN_TYPE_SIZE		= 21,
};

enum EMANI_SUBTYPE
{
	AN_SUB_NONE			= 0,
	AN_SUB_ONEHSWORD	= 1,
	AN_SUB_TWOHSWORD	= 2,
	AN_SUB_EITHERSWORD	= 3,
	AN_SUB_DAGGER		= 4,
	AN_SUB_SPEAR		= 5,
	AN_SUB_BOW			= 6,
	AN_SUB_THROW		= 7,
	AN_SUB_MANA			= 8,
	AN_SUB_BIGHSWORD	= 9,
	AN_SUB_STICK		= 10,
	AN_SUB_ONEBLADE		= 11,
	AN_SUB_TWOBLADE		= 12,
	AN_SUB_BIGBLADE		= 13,
	AN_SUB_EITHERBLADE	= 14,
	AN_SUB_GAUNT		= 15,
	AN_SUB_BROOM		= 16,
	AN_SUB_HOVERBOARD	= 17,

	AN_SUB_DUALGUN		= 18, //gunner dual gun
	AN_SUB_GUN			= 19, //gunner portal, rail gun

	AN_SUB_SCYTHE		= 20, //assassin scythe
	AN_SUB_DUALSPEAR	= 21, //assassin dual spear
	AN_SUB_SHURIKEN		= 22, //assassin Shuriken

	AN_SUB_EXTREME_FIST	= 23, //extreme fist

	AN_SUB_TRICKER_WAND	= 24, //magician Wand
	AN_SUB_TRICKER_CUBE	= 25, //magician Cube
	AN_SUB_TRICKER_ROPE	= 26, //magician Rope

	AN_SUB_SHAPER_HAMMER	= 27, //shaper Hammer
	AN_SUB_SHAPER_SHIELD	= 28, //shaper Shield
	AN_SUB_SHAPER_UMBRELLA	= 29, //shaper Umbrella

	//vehicle animations
	AN_SUB_VEHICLE_01_BOARD		= 0,
	AN_SUB_VEHICLE_01_STAY		= 1, //stay
	AN_SUB_VEHICLE_01_WALK		= 2, //walk
	AN_SUB_VEHICLE_01_RUN		= 3, //run
	AN_SUB_VEHICLE_01_BOOST		= 4, //boost
	AN_SUB_VEHICLE_01_DEATH		= 5, //die
	AN_SUB_VEHICLE_02_BOARD		= 6,
	AN_SUB_VEHICLE_02_STAY		= 7,
	AN_SUB_VEHICLE_02_WALK		= 8,
	AN_SUB_VEHICLE_02_RUN		= 9,
	AN_SUB_VEHICLE_02_BOOST		= 10,
	AN_SUB_VEHICLE_02_DEATH		= 11,

	//skill/normal animations
	AN_SUB_00			= 0,
	AN_SUB_01			= 1,
	AN_SUB_02			= 2,
	AN_SUB_03			= 3,
	AN_SUB_04			= 4,
	AN_SUB_05			= 5,
	AN_SUB_06			= 6,
	AN_SUB_07			= 7,
	AN_SUB_08			= 8,
	AN_SUB_09			= 9,
	AN_SUB_10			= 10,
	AN_SUB_11			= 11,
	AN_SUB_12			= 12,
	AN_SUB_13			= 13,
	AN_SUB_14			= 14,
	AN_SUB_15			= 15,
	AN_SUB_16			= 16,
	AN_SUB_17			= 17,
	AN_SUB_18			= 18,
	AN_SUB_19			= 19,
	AN_SUB_20			= 20,
	AN_SUB_21			= 21,
	AN_SUB_22			= 22,
	AN_SUB_23			= 23,
	AN_SUB_24			= 24,
	AN_SUB_25			= 25,
	AN_SUB_26			= 26,
	AN_SUB_27			= 27,
	AN_SUB_28			= 28,
	AN_SUB_29			= 29,
	AN_SUB_30			= 30,
	AN_SUB_31			= 31,
	AN_SUB_32			= 32,
	AN_SUB_33			= 33,
	AN_SUB_34			= 34,
	AN_SUB_35			= 35,
	AN_SUB_36			= 36,
	AN_SUB_37			= 37,
	AN_SUB_38			= 38,
	AN_SUB_39			= 39,
	AN_SUB_40			= 40,
	AN_SUB_41			= 41,
	AN_SUB_42			= 42,
	AN_SUB_43			= 43,
	AN_SUB_44			= 44,
	AN_SUB_45			= 45,
	AN_SUB_46			= 46,
	AN_SUB_47			= 47,
	AN_SUB_48			= 48,
	AN_SUB_49			= 49,
	AN_SUB_50			= 50,
	AN_SUB_51			= 51,
	AN_SUB_52			= 52,
	AN_SUB_53			= 53,
	AN_SUB_54			= 54,
	AN_SUB_55			= 55,
	AN_SUB_56			= 56,
	AN_SUB_57			= 57,
	AN_SUB_58			= 58,
	AN_SUB_59			= 59,
	AN_SUB_60			= 60,
	AN_SUB_61			= 61,
	AN_SUB_62			= 62,
	AN_SUB_63			= 63,
	AN_SUB_64			= 64,
	AN_SUB_65			= 65,
	AN_SUB_66			= 66,
	AN_SUB_67			= 67,
	AN_SUB_68			= 68,
	AN_SUB_69			= 69,
	AN_SUB_70			= 70,
	AN_SUB_71			= 71,
	AN_SUB_72			= 72,
	AN_SUB_73			= 73,
	AN_SUB_74			= 74,
	AN_SUB_75			= 75,
	AN_SUB_76			= 76,
	AN_SUB_77			= 77,
	AN_SUB_78			= 78,
	AN_SUB_79			= 79,
	AN_SUB_80			= 80,
	AN_SUB_81			= 81,
	AN_SUB_82			= 82,
	AN_SUB_83			= 83,
	AN_SUB_84			= 84,
	AN_SUB_85			= 85,
	AN_SUB_86			= 86,
	AN_SUB_87			= 87,
	AN_SUB_88			= 88,
	AN_SUB_89			= 89,
	AN_SUB_90			= 90,
	AN_SUB_91			= 91,
	AN_SUB_92			= 92,
	AN_SUB_93			= 93,
	AN_SUB_94			= 94,
	AN_SUB_95			= 95,
	AN_SUB_96			= 96,
	AN_SUB_97			= 97,
	AN_SUB_98			= 98,
	AN_SUB_99			= 99,

	AN_SUB_00_SIZE		= 100, //over all size for animation sub
	//AN_SUB_00_SIZE	= 50, //ep7 
};

#endif // DXANIKEYS_H_
