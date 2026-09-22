#include "stdafx.h"
#include "GLCrowDataBasic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_100 &sCrowBasic )
{
	sNativeID = sCrowBasic.sNativeID;
	StringCbCopy ( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	m_emTribe = sCrowBasic.m_emTribe;
	m_emCrow = sCrowBasic.m_emCrow;
	m_emBright = sCrowBasic.m_emBright;

	m_wLevel = sCrowBasic.m_wLevel;

	m_dwHP = sCrowBasic.m_wHP;
	m_wMP = sCrowBasic.m_wMP;
	m_wSP = sCrowBasic.m_wSP;

	//	회복율.
	m_fIncHP = sCrowBasic.m_fIncHP;
	m_fIncMP = sCrowBasic.m_fIncMP;
	m_fIncSP = sCrowBasic.m_fIncSP;

	m_wAvoidRate = sCrowBasic.m_wAvoidRate;
	m_wHitRate = sCrowBasic.m_wHitRate;

	m_wViewRange = sCrowBasic.m_wViewRange;
	m_wDefense = sCrowBasic.m_wDefense;

	m_bOverlapAttack = FALSE;

	m_sResist.Assign ( sCrowBasic.m_sResist );

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_101 &sCrowBasic )
{
	sNativeID = sCrowBasic.sNativeID;
	StringCbCopy ( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	m_emTribe = sCrowBasic.m_emTribe;
	m_emCrow = sCrowBasic.m_emCrow;
	m_emBright = sCrowBasic.m_emBright;

	m_wLevel = sCrowBasic.m_wLevel;

	m_dwHP = sCrowBasic.m_wHP;
	m_wMP = sCrowBasic.m_wMP;
	m_wSP = sCrowBasic.m_wSP;

	//	회복율.
	m_fIncHP = sCrowBasic.m_fIncHP;
	m_fIncMP = sCrowBasic.m_fIncMP;
	m_fIncSP = sCrowBasic.m_fIncSP;

	m_wAvoidRate = sCrowBasic.m_wAvoidRate;
	m_wHitRate = sCrowBasic.m_wHitRate;

	m_wViewRange = sCrowBasic.m_wViewRange;
	m_wDefense = sCrowBasic.m_wDefense;

	m_sResist = sCrowBasic.m_sResist;

	m_bOverlapAttack = FALSE;

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_102 &sCrowBasic )
{
	sNativeID = sCrowBasic.sNativeID;
	StringCbCopy( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	m_emTribe = sCrowBasic.m_emTribe;
	m_emCrow = sCrowBasic.m_emCrow;
	m_emBright = sCrowBasic.m_emBright;

	m_wLevel = sCrowBasic.m_wLevel;

	m_dwHP = sCrowBasic.m_wHP;
	m_wMP = sCrowBasic.m_wMP;
	m_wSP = sCrowBasic.m_wSP;

	//	회복율.
	m_fIncHP = sCrowBasic.m_fIncHP;
	m_fIncMP = sCrowBasic.m_fIncMP;
	m_fIncSP = sCrowBasic.m_fIncSP;

	m_wAvoidRate = sCrowBasic.m_wAvoidRate;
	m_wHitRate = sCrowBasic.m_wHitRate;

	m_wViewRange = sCrowBasic.m_wViewRange;
	m_wDefense = sCrowBasic.m_wDefense;

	m_sResist = sCrowBasic.m_sResist;

	m_dwBonusExp = sCrowBasic.m_wBonusExp;

	m_bOverlapAttack = FALSE;

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_103 &sCrowBasic )
{
	sNativeID = sCrowBasic.sNativeID;
	StringCbCopy( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	m_emTribe = sCrowBasic.m_emTribe;
	m_emCrow = sCrowBasic.m_emCrow;
	m_emBright = sCrowBasic.m_emBright;

	m_wLevel = sCrowBasic.m_wLevel;

	m_dwHP = sCrowBasic.m_dwHP;
	m_wMP = sCrowBasic.m_wMP;
	m_wSP = sCrowBasic.m_wSP;

	//	회복율.
	m_fIncHP = sCrowBasic.m_fIncHP;
	m_fIncMP = sCrowBasic.m_fIncMP;
	m_fIncSP = sCrowBasic.m_fIncSP;

	m_wAvoidRate = sCrowBasic.m_wAvoidRate;
	m_wHitRate = sCrowBasic.m_wHitRate;

	m_wViewRange = sCrowBasic.m_wViewRange;
	m_wDefense = sCrowBasic.m_wDefense;

	m_sResist = sCrowBasic.m_sResist;

	m_dwBonusExp = sCrowBasic.m_wBonusExp;

	m_bOverlapAttack = FALSE;

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_104 &sCrowBasic )
{
	sNativeID = sCrowBasic.sNativeID;
	StringCbCopy( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	m_emTribe = sCrowBasic.m_emTribe;
	m_emCrow = sCrowBasic.m_emCrow;
	m_emBright = sCrowBasic.m_emBright;

	// 팻타입
	m_emPetType = sCrowBasic.m_emPetType;

	m_wLevel = sCrowBasic.m_wLevel;

	m_dwHP = sCrowBasic.m_dwHP;
	m_wMP = sCrowBasic.m_wMP;
	m_wSP = sCrowBasic.m_wSP;

	//	회복율.
	m_fIncHP = sCrowBasic.m_fIncHP;
	m_fIncMP = sCrowBasic.m_fIncMP;
	m_fIncSP = sCrowBasic.m_fIncSP;

	m_wAvoidRate = sCrowBasic.m_wAvoidRate;
	m_wHitRate = sCrowBasic.m_wHitRate;

	m_wViewRange = sCrowBasic.m_wViewRange;
	m_wDefense = sCrowBasic.m_wDefense;

	m_sResist = sCrowBasic.m_sResist;

	m_dwBonusExp = sCrowBasic.m_wBonusExp;

	m_bOverlapAttack = FALSE;

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_105 &sCrowBasic )
{
	sNativeID = sCrowBasic.sNativeID;
	StringCbCopy( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	m_emTribe = sCrowBasic.m_emTribe;
	m_emCrow = sCrowBasic.m_emCrow;
	m_emBright = sCrowBasic.m_emBright;

	// 팻타입
	m_emPetType = sCrowBasic.m_emPetType;

	m_wLevel = sCrowBasic.m_wLevel;

	m_dwHP = sCrowBasic.m_dwHP;
	m_wMP = sCrowBasic.m_wMP;
	m_wSP = sCrowBasic.m_wSP;

	//	회복율.
	m_fIncHP = sCrowBasic.m_fIncHP;
	m_fIncMP = sCrowBasic.m_fIncMP;
	m_fIncSP = sCrowBasic.m_fIncSP;

	m_wAvoidRate = sCrowBasic.m_wAvoidRate;
	m_wHitRate = sCrowBasic.m_wHitRate;

	m_wViewRange = sCrowBasic.m_wViewRange;
	m_wDefense = sCrowBasic.m_wDefense;

	m_sResist = sCrowBasic.m_sResist;

	m_dwBonusExp = sCrowBasic.m_wBonusExp;

	m_bOverlapAttack = FALSE;

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_106 &sCrowBasic )
{
	sNativeID = sCrowBasic.sNativeID;
	StringCbCopy( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	m_emTribe = sCrowBasic.m_emTribe;
	m_emCrow = sCrowBasic.m_emCrow;
	m_emBright = sCrowBasic.m_emBright;

	// 팻타입
	m_emPetType = sCrowBasic.m_emPetType;

	m_wLevel = sCrowBasic.m_wLevel;

	m_dwHP = sCrowBasic.m_dwHP;
	m_wMP = sCrowBasic.m_wMP;
	m_wSP = sCrowBasic.m_wSP;

	//	회복율.
	m_fIncHP = sCrowBasic.m_fIncHP;
	m_fIncMP = sCrowBasic.m_fIncMP;
	m_fIncSP = sCrowBasic.m_fIncSP;

	m_wAvoidRate = sCrowBasic.m_wAvoidRate;
	m_wHitRate = sCrowBasic.m_wHitRate;

	m_wViewRange = sCrowBasic.m_wViewRange;
	m_wDefense = sCrowBasic.m_wDefense;

	m_sResist = sCrowBasic.m_sResist;

	m_dwBonusExp = sCrowBasic.m_wBonusExp;

	m_bOverlapAttack = sCrowBasic.m_bOverlapAttack;

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_108 &sCrowBasic )
{
	sNativeID = sCrowBasic.sNativeID;
	StringCbCopy( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	m_emTribe = sCrowBasic.m_emTribe;
	m_emCrow = sCrowBasic.m_emCrow;
	m_emBright = sCrowBasic.m_emBright;

	// 팻타입
	m_emPetType = sCrowBasic.m_emPetType;

	m_wLevel = sCrowBasic.m_wLevel;

	m_dwHP = sCrowBasic.m_dwHP;
	m_wMP = sCrowBasic.m_wMP;
	m_wSP = sCrowBasic.m_wSP;

	//	회복율.
	m_fIncHP = sCrowBasic.m_fIncHP;
	m_fIncMP = sCrowBasic.m_fIncMP;
	m_fIncSP = sCrowBasic.m_fIncSP;

	m_wAvoidRate = sCrowBasic.m_wAvoidRate;
	m_wHitRate = sCrowBasic.m_wHitRate;

	m_wViewRange = sCrowBasic.m_wViewRange;
	m_wDefense = sCrowBasic.m_wDefense;

	m_sResist = sCrowBasic.m_sResist;

	m_dwBonusExp = sCrowBasic.m_wBonusExp;

	m_bOverlapAttack = sCrowBasic.m_bOverlapAttack;

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_109 &sCrowBasic )
{
	sNativeID = sCrowBasic.sNativeID;
	StringCbCopy( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	m_emTribe = sCrowBasic.m_emTribe;
	m_emCrow = sCrowBasic.m_emCrow;
	m_emBright = sCrowBasic.m_emBright;

	// 팻타입
	m_emPetType = sCrowBasic.m_emPetType;

	m_wLevel = sCrowBasic.m_wLevel;

	m_dwHP = sCrowBasic.m_dwHP;
	m_wMP = sCrowBasic.m_wMP;
	m_wSP = sCrowBasic.m_wSP;

	//	회복율.
	m_fIncHP = sCrowBasic.m_fIncHP;
	m_fIncMP = sCrowBasic.m_fIncMP;
	m_fIncSP = sCrowBasic.m_fIncSP;

	m_wAvoidRate = sCrowBasic.m_wAvoidRate;
	m_wHitRate = sCrowBasic.m_wHitRate;

	m_wViewRange = sCrowBasic.m_wViewRange;
	m_wDefense = sCrowBasic.m_wDefense;

	m_sResist = sCrowBasic.m_sResist;

	m_dwBonusExp = sCrowBasic.m_dwBonusExp;

	m_bOverlapAttack = sCrowBasic.m_bOverlapAttack;

	m_dwGenTime = sCrowBasic.m_dwGenTime;

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_110 &sCrowBasic )
{
	StringCbCopy( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	sNativeID = sCrowBasic.sNativeID;
	m_emCrow = sCrowBasic.m_emCrow;
	m_emTribe = sCrowBasic.m_emTribe;
	m_emBright = sCrowBasic.m_emBright;
	m_wLevel = sCrowBasic.m_wLevel;
	m_dwHP = sCrowBasic.m_dwHP;
	m_wMP = sCrowBasic.m_wMP;
	m_wSP = sCrowBasic.m_wSP;
	m_fIncHP = sCrowBasic.m_fIncHP;
	m_fIncMP = sCrowBasic.m_fIncMP;
	m_fIncSP = sCrowBasic.m_fIncSP;
	m_wDefense = sCrowBasic.m_wDefense;
	m_wViewRange = sCrowBasic.m_wViewRange;
	m_wAvoidRate = sCrowBasic.m_wAvoidRate;
	m_wHitRate = sCrowBasic.m_wHitRate;
	m_dwBonusExp = sCrowBasic.m_dwBonusExp;
	m_sResist = sCrowBasic.m_sResist;
	m_bOverlapAttack = sCrowBasic.m_bOverlapAttack;
	m_dwGenTime = sCrowBasic.m_dwGenTime;
	m_wGatherTimeLow = sCrowBasic.m_wGatherTimeLow;
	m_wGatherTimeHigh = sCrowBasic.m_wGatherTimeHigh;
	m_fGatherRate = sCrowBasic.m_fGatherRate;
	m_wGatherAnimation = sCrowBasic.m_wGatherAnimation;
	m_emPetType = sCrowBasic.m_emPetType;

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_113 &sCrowBasic )
{
	StringCbCopy( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	sNativeID = sCrowBasic.sNativeID;
	m_emCrow = sCrowBasic.m_emCrow;
	m_emTribe = sCrowBasic.m_emTribe;
	m_emBright = sCrowBasic.m_emBright;
	m_wLevel = sCrowBasic.m_wLevel;
	m_dwHP = sCrowBasic.m_dwHP;
	m_wMP = sCrowBasic.m_wMP;
	m_wSP = sCrowBasic.m_wSP;
	m_fIncHP = sCrowBasic.m_fIncHP;
	m_fIncMP = sCrowBasic.m_fIncMP;
	m_fIncSP = sCrowBasic.m_fIncSP;
	m_wDefense = sCrowBasic.m_wDefense;
	m_wViewRange = sCrowBasic.m_wViewRange;
	m_wAvoidRate = sCrowBasic.m_wAvoidRate;
	m_wHitRate = sCrowBasic.m_wHitRate;
	m_dwBonusExp = sCrowBasic.m_dwBonusExp;
	m_wBonusCP = sCrowBasic.m_wBonusCP;
	m_sResist = sCrowBasic.m_sResist;
	m_bOverlapAttack = sCrowBasic.m_bOverlapAttack;
	m_dwGenTime = sCrowBasic.m_dwGenTime;
	m_wGatherTimeLow = sCrowBasic.m_wGatherTimeLow;
	m_wGatherTimeHigh = sCrowBasic.m_wGatherTimeHigh;
	m_fGatherRate = sCrowBasic.m_fGatherRate;
	m_wGatherAnimation = sCrowBasic.m_wGatherAnimation;
	m_emPetType = sCrowBasic.m_emPetType;

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_116 &sCrowBasic )
{
	StringCbCopy( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	sNativeID = sCrowBasic.sNativeID;
	m_emCrow = sCrowBasic.m_emCrow;
	m_emTribe = sCrowBasic.m_emTribe;
	m_emBright = sCrowBasic.m_emBright;
	m_wLevel = sCrowBasic.m_wLevel;
	m_dwHP = sCrowBasic.m_dwHP;
	m_wMP = sCrowBasic.m_wMP;
	m_wSP = sCrowBasic.m_wSP;
	m_fIncHP = sCrowBasic.m_fIncHP;
	m_fIncMP = sCrowBasic.m_fIncMP;
	m_fIncSP = sCrowBasic.m_fIncSP;
	m_wDefense = sCrowBasic.m_wDefense;
	m_wViewRange = sCrowBasic.m_wViewRange;
	m_wAvoidRate = sCrowBasic.m_wAvoidRate;
	m_wHitRate = sCrowBasic.m_wHitRate;
	m_dwBonusExp = sCrowBasic.m_dwBonusExp;
	m_wBonusCP = sCrowBasic.m_wBonusCP;
	m_sResist = sCrowBasic.m_sResist;
	m_bOverlapAttack = sCrowBasic.m_bOverlapAttack;
	m_dwGenTime = sCrowBasic.m_dwGenTime;
	m_wGatherTimeLow = sCrowBasic.m_wGatherTimeLow;
	m_wGatherTimeHigh = sCrowBasic.m_wGatherTimeHigh;
	m_fGatherRate = sCrowBasic.m_fGatherRate;
	m_wGatherAnimation = sCrowBasic.m_wGatherAnimation;
	m_emPetType = sCrowBasic.m_emPetType;
	m_emSummonType = sCrowBasic.m_emSummonType;
	m_emNPCType = sCrowBasic.m_emNPCType;

	dummy_var_0 = sCrowBasic.dummy_var_0;

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_117 &sCrowBasic )
{
	StringCbCopy( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	sNativeID			= sCrowBasic.sNativeID;
	m_emCrow			= sCrowBasic.m_emCrow;
	m_emTribe			= sCrowBasic.m_emTribe;
	m_emBright			= sCrowBasic.m_emBright;
	m_wLevel			= sCrowBasic.m_wLevel;
	m_dwHP				= sCrowBasic.m_dwHP;
	m_wMP				= sCrowBasic.m_wMP;
	m_wSP				= sCrowBasic.m_wSP;
	m_fIncHP			= sCrowBasic.m_fIncHP;
	m_fIncMP			= sCrowBasic.m_fIncMP;
	m_fIncSP			= sCrowBasic.m_fIncSP;
	m_wDefense			= sCrowBasic.m_wDefense;
	m_wViewRange		= sCrowBasic.m_wViewRange;
	m_wAvoidRate		= sCrowBasic.m_wAvoidRate;
	m_wHitRate			= sCrowBasic.m_wHitRate;
	m_dwBonusExp		= sCrowBasic.m_dwBonusExp;
	m_wBonusCP			= sCrowBasic.m_wBonusCP;
	m_sResist			= sCrowBasic.m_sResist;
	m_bOverlapAttack	= sCrowBasic.m_bOverlapAttack;
	m_dwGenTime			= sCrowBasic.m_dwGenTime;
	m_wGatherTimeLow	= sCrowBasic.m_wGatherTimeLow;
	m_wGatherTimeHigh	= sCrowBasic.m_wGatherTimeHigh;
	m_fGatherRate		= sCrowBasic.m_fGatherRate;
	m_wGatherAnimation	= sCrowBasic.m_wGatherAnimation;
	m_emPetType			= sCrowBasic.m_emPetType;
	m_emSummonType		= sCrowBasic.m_emSummonType;
	m_emNPCType			= sCrowBasic.m_emNPCType;
	m_bHideName			= false; /*hide crow name, Juver, 2017/09/01 */
	m_dwNotice			= 0; /*crow notice, Juver, 2017/11/25 */

	dummy_var_0			= sCrowBasic.dummy_var_0;
	dummy_var_1			= false;
	dummy_var_2			= false;

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_200 &sCrowBasic )
{
	StringCbCopy( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	sNativeID = sCrowBasic.sNativeID;
	m_emCrow = sCrowBasic.m_emCrow;
	m_emTribe = sCrowBasic.m_emTribe;
	m_emBright = sCrowBasic.m_emBright;
	m_wLevel = sCrowBasic.m_wLevel;
	m_dwHP = sCrowBasic.m_dwHP;
	m_wMP = sCrowBasic.m_wMP;
	m_wSP = sCrowBasic.m_wSP;
	m_fIncHP = sCrowBasic.m_fIncHP;
	m_fIncMP = sCrowBasic.m_fIncMP;
	m_fIncSP = sCrowBasic.m_fIncSP;
	m_wDefense = sCrowBasic.m_wDefense;
	m_wViewRange = sCrowBasic.m_wViewRange;
	m_wAvoidRate = sCrowBasic.m_wAvoidRate;
	m_wHitRate = sCrowBasic.m_wHitRate;
	m_dwBonusExp = sCrowBasic.m_dwBonusExp;
	m_wBonusCP = 0;
	m_sResist = sCrowBasic.m_sResist;
	m_bOverlapAttack = sCrowBasic.m_bOverlapAttack;
	m_dwGenTime = sCrowBasic.m_dwGenTime;
	m_wGatherTimeLow = sCrowBasic.m_wGatherTimeLow;
	m_wGatherTimeHigh = sCrowBasic.m_wGatherTimeHigh;
	m_fGatherRate = sCrowBasic.m_fGatherRate;
	m_wGatherAnimation = sCrowBasic.m_wGatherAnimation;
	m_emPetType = sCrowBasic.m_emPetType;
	m_emSummonType = SUMMON_TYPE_A;
	m_emNPCType = EMNPC_TYPE_NORMAL;

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_201 &sCrowBasic )
{
	StringCbCopy( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	sNativeID = sCrowBasic.sNativeID;
	m_emCrow = sCrowBasic.m_emCrow;
	m_emTribe = sCrowBasic.m_emTribe;
	m_emBright = sCrowBasic.m_emBright;
	m_wLevel = sCrowBasic.m_wLevel;
	m_dwHP = sCrowBasic.m_dwHP;
	m_wMP = sCrowBasic.m_wMP;
	m_wSP = sCrowBasic.m_wSP;
	m_fIncHP = sCrowBasic.m_fIncHP;
	m_fIncMP = sCrowBasic.m_fIncMP;
	m_fIncSP = sCrowBasic.m_fIncSP;
	m_wDefense = sCrowBasic.m_wDefense;
	m_wViewRange = sCrowBasic.m_wViewRange;
	m_wAvoidRate = sCrowBasic.m_wAvoidRate;
	m_wHitRate = sCrowBasic.m_wHitRate;
	m_dwBonusExp = sCrowBasic.m_dwBonusExp;
	m_wBonusCP = sCrowBasic.m_wBonusCP;
	m_sResist = sCrowBasic.m_sResist;
	m_bOverlapAttack = sCrowBasic.m_bOverlapAttack;
	m_dwGenTime = sCrowBasic.m_dwGenTime;
	m_wGatherTimeLow = sCrowBasic.m_wGatherTimeLow;
	m_wGatherTimeHigh = sCrowBasic.m_wGatherTimeHigh;
	m_fGatherRate = sCrowBasic.m_fGatherRate;
	m_wGatherAnimation = sCrowBasic.m_wGatherAnimation;
	m_emPetType = sCrowBasic.m_emPetType;
	m_emSummonType = sCrowBasic.m_emSummonType;
	m_emNPCType = sCrowBasic.m_emNPCType;
	m_bHideName = false; /*hide crow name, Juver, 2017/09/01 */
	m_dwNotice = 0; /*crow notice, Juver, 2017/11/25 */

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_202 &sCrowBasic )
{
	StringCbCopy( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	sNativeID = sCrowBasic.sNativeID;
	m_emCrow = sCrowBasic.m_emCrow;
	m_emTribe = sCrowBasic.m_emTribe;
	m_emBright = sCrowBasic.m_emBright;
	m_wLevel = sCrowBasic.m_wLevel;
	m_dwHP = sCrowBasic.m_dwHP;
	m_wMP = sCrowBasic.m_wMP;
	m_wSP = sCrowBasic.m_wSP;
	m_fIncHP = sCrowBasic.m_fIncHP;
	m_fIncMP = sCrowBasic.m_fIncMP;
	m_fIncSP = sCrowBasic.m_fIncSP;
	m_wDefense = sCrowBasic.m_wDefense;
	m_wViewRange = sCrowBasic.m_wViewRange;
	m_wAvoidRate = sCrowBasic.m_wAvoidRate;
	m_wHitRate = sCrowBasic.m_wHitRate;
	m_dwBonusExp = sCrowBasic.m_dwBonusExp;
	m_wBonusCP = sCrowBasic.m_wBonusCP;
	m_sResist = sCrowBasic.m_sResist;
	m_bOverlapAttack = sCrowBasic.m_bOverlapAttack;
	m_dwGenTime = sCrowBasic.m_dwGenTime;
	m_wGatherTimeLow = sCrowBasic.m_wGatherTimeLow;
	m_wGatherTimeHigh = sCrowBasic.m_wGatherTimeHigh;
	m_fGatherRate = sCrowBasic.m_fGatherRate;
	m_wGatherAnimation = sCrowBasic.m_wGatherAnimation;
	m_emPetType = sCrowBasic.m_emPetType;
	m_emSummonType = sCrowBasic.m_emSummonType;
	m_emNPCType = sCrowBasic.m_emNPCType;
	m_bHideName = sCrowBasic.m_bHideName; /*hide crow name, Juver, 2017/09/01 */
	m_dwNotice = 0; /*crow notice, Juver, 2017/11/25 */

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_203 &sCrowBasic )
{
	StringCbCopy( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	sNativeID			= sCrowBasic.sNativeID;
	m_emCrow			= sCrowBasic.m_emCrow;
	m_emTribe			= sCrowBasic.m_emTribe;
	m_emBright			= sCrowBasic.m_emBright;
	m_wLevel			= sCrowBasic.m_wLevel;
	m_dwHP				= sCrowBasic.m_dwHP;
	m_wMP				= sCrowBasic.m_wMP;
	m_wSP				= sCrowBasic.m_wSP;
	m_fIncHP			= sCrowBasic.m_fIncHP;
	m_fIncMP			= sCrowBasic.m_fIncMP;
	m_fIncSP			= sCrowBasic.m_fIncSP;
	m_wDefense			= sCrowBasic.m_wDefense;
	m_wViewRange		= sCrowBasic.m_wViewRange;
	m_wAvoidRate		= sCrowBasic.m_wAvoidRate;
	m_wHitRate			= sCrowBasic.m_wHitRate;
	m_dwBonusExp		= sCrowBasic.m_dwBonusExp;
	m_wBonusCP			= sCrowBasic.m_wBonusCP;
	m_sResist			= sCrowBasic.m_sResist;
	m_bOverlapAttack	= sCrowBasic.m_bOverlapAttack;
	m_dwGenTime			= sCrowBasic.m_dwGenTime;
	m_wGatherTimeLow	= sCrowBasic.m_wGatherTimeLow;
	m_wGatherTimeHigh	= sCrowBasic.m_wGatherTimeHigh;
	m_fGatherRate		= sCrowBasic.m_fGatherRate;
	m_wGatherAnimation	= sCrowBasic.m_wGatherAnimation;
	m_emPetType			= sCrowBasic.m_emPetType;
	m_emSummonType		= sCrowBasic.m_emSummonType;
	m_emNPCType			= sCrowBasic.m_emNPCType;
	m_bHideName			= sCrowBasic.m_bHideName; /*hide crow name, Juver, 2017/09/01 */
	m_dwNotice			= sCrowBasic.m_dwNotice; /*crow notice, Juver, 2017/11/25 */

	dummy_var_0			= 0;
	dummy_var_1			= false;
	dummy_var_2			= false;

	return *this;
}

SCROWBASIC& SCROWBASIC::operator = ( const SCROWBASIC_204 &sCrowBasic )
{
	StringCbCopy( m_szName, CHAR_SZNAME, sCrowBasic.m_szName );
	sNativeID			= sCrowBasic.sNativeID;
	m_emCrow			= sCrowBasic.m_emCrow;
	m_emTribe			= sCrowBasic.m_emTribe;
	m_emBright			= sCrowBasic.m_emBright;
	m_wLevel			= sCrowBasic.m_wLevel;
	m_dwHP				= sCrowBasic.m_dwHP;
	m_wMP				= sCrowBasic.m_wMP;
	m_wSP				= sCrowBasic.m_wSP;
	m_fIncHP			= sCrowBasic.m_fIncHP;
	m_fIncMP			= sCrowBasic.m_fIncMP;
	m_fIncSP			= sCrowBasic.m_fIncSP;
	m_wDefense			= sCrowBasic.m_wDefense;
	m_wViewRange		= sCrowBasic.m_wViewRange;
	m_wAvoidRate		= sCrowBasic.m_wAvoidRate;
	m_wHitRate			= sCrowBasic.m_wHitRate;
	m_dwBonusExp		= sCrowBasic.m_dwBonusExp;
	m_wBonusCP			= sCrowBasic.m_wBonusCP;
	m_sResist			= sCrowBasic.m_sResist;
	m_bOverlapAttack	= sCrowBasic.m_bOverlapAttack;
	m_dwGenTime			= sCrowBasic.m_dwGenTime;
	m_wGatherTimeLow	= sCrowBasic.m_wGatherTimeLow;
	m_wGatherTimeHigh	= sCrowBasic.m_wGatherTimeHigh;
	m_fGatherRate		= sCrowBasic.m_fGatherRate;
	m_wGatherAnimation	= sCrowBasic.m_wGatherAnimation;
	m_emPetType			= sCrowBasic.m_emPetType;
	m_emSummonType		= sCrowBasic.m_emSummonType;
	m_emNPCType			= sCrowBasic.m_emNPCType;
	m_bHideName			= sCrowBasic.m_bHideName;	/*hide crow name, Juver, 2017/09/01 */
	m_dwNotice			= sCrowBasic.m_dwNotice;	/*crow notice, Juver, 2017/11/25 */

	dummy_var_0			= sCrowBasic.dummy_var_0;
	dummy_var_1			= sCrowBasic.dummy_var_1;
	dummy_var_2			= sCrowBasic.dummy_var_2;

	m_emCTFType			= EMNPC_CTF_FLAG_TYPE_NONE;	/*pvp capture the flag, Juver, 2018/02/03 */

	return *this;
}

VOID SCROWBASIC::SaveCsvHead ( std::fstream &SFile )
{
	SFile << "sNativeID wMainID" << ",";
	SFile << "sNativeID wSubID" << ",";

	SFile << "szName" << ",";

//	기획팀 요청으로 제거함
//	SFile << "szName" << ",";

	SFile << "emTribe" << ",";
	SFile << "emCrow" << ",";
	SFile << "emBright" << ",";

	SFile << "wLevel" << ",";

	SFile << "dwHP" << ",";
	SFile << "wMP" << ",";
	SFile << "wSP" << ",";

	SFile << "fIncHP" << ",";
	SFile << "fIncMP" << ",";
	SFile << "fIncSP" << ",";

	SFile << "wAvoidRate" << ",";
	SFile << "wHitRate" << ",";

	SFile << "wViewRange" << ",";
	SFile << "wDefense" << ",";

	SFile << "sResist nFire" << ",";
	SFile << "sResist nIce" << ",";
	SFile << "sResist nElectric" << ",";
	SFile << "sResist nPoison" << ",";
	SFile << "sResist nSpirit" << ",";

	SFile << "dwBonusExp" << ",";
	SFile << "m_wBonusCP" << ",";

	SFile << "bOverlapAttack" << ",";

	// PET
	SFile << "emPetType" << ",";
	SFile << "emSummonType" << ",";
	SFile << "emNPCType" << ",";

	SFile << "dwGenTime" << ",";
	
	SFile << "wGatherTimeLow" << ",";
	SFile << "wGatherTimeHigh" << ",";
	SFile << "fGatherRate" << ",";
	SFile << "wGatherAnimation" << ",";

	/*hide crow name, Juver, 2017/09/01 */
	SFile << "bHideName" << ","; 

	/*crow notice, Juver, 2017/11/25 */
	SFile << "emNotice" << ",";


	SFile << "dummy_var_0" << ",";
	SFile << "dummy_var_1" << ",";
	SFile << "dummy_var_2" << ",";

	/*pvp capture the flag, Juver, 2018/02/03 */
	SFile << "emCTFType" << ",";
}

VOID SCROWBASIC::SaveCsv ( std::fstream &SFile )
{
	SFile << sNativeID.wMainID << ",";
	SFile << sNativeID.wSubID << ",";

	std::string	strName = m_szName;
	STRUTIL::OutputStrCsv( SFile, strName );

//	기획팀 요청으로 제거함
/*
	std::string str = strName;
	const char* szpName = str.c_str();
	if( szpName )
	{
		const char* szpLongName = GLStringTable::GetInstance().GetString( szpName, GLStringTable::CROW );
		if( szpLongName )
			str = szpLongName;
	}
	STRUTIL::OutputStrCsv( SFile, str );
*/

	SFile << m_emTribe << ",";
	SFile << m_emCrow << ",";
	SFile << m_emBright << ",";

	SFile << m_wLevel << ",";

	SFile << m_dwHP << ",";
	SFile << m_wMP << ",";
	SFile << m_wSP << ",";

	SFile << m_fIncHP << ",";
	SFile << m_fIncMP << ",";
	SFile << m_fIncSP << ",";

	SFile << m_wAvoidRate << ",";
	SFile << m_wHitRate << ",";

	SFile << m_wViewRange << ",";
	SFile << m_wDefense << ",";

	SFile << m_sResist.nFire << ",";
	SFile << m_sResist.nIce << ",";
	SFile << m_sResist.nElectric << ",";
	SFile << m_sResist.nPoison << ",";
	SFile << m_sResist.nSpirit << ",";

	SFile << m_dwBonusExp << ",";
	SFile << m_wBonusCP << ",";

	SFile << m_bOverlapAttack << ",";

	// PET
	SFile << m_emPetType << ",";
	SFile << m_emSummonType << ",";
	SFile << m_emNPCType << ",";
	
	SFile << m_dwGenTime << ",";

	SFile << m_wGatherTimeLow << ",";
	SFile << m_wGatherTimeHigh << ",";
	SFile << m_fGatherRate << ",";
	SFile << m_wGatherAnimation << ",";

	/*hide crow name, Juver, 2017/09/01 */
	SFile << m_bHideName << ","; 

	/*crow notice, Juver, 2017/11/25 */
	SFile << m_dwNotice << ",";


	SFile << dummy_var_0 << ",";
	SFile << dummy_var_1 << ",";
	SFile << dummy_var_2 << ",";

	/*pvp capture the flag, Juver, 2018/02/03 */
	SFile << m_emCTFType << ",";
}

VOID SCROWBASIC::LoadCsv ( CStringArray &StrArray, int& iCsvCur )
{
	sNativeID.wMainID = (WORD)atoi( StrArray[ iCsvCur++ ] );
	sNativeID.wSubID = (WORD)atoi( StrArray[ iCsvCur++ ] );

	std::string	strName;
	STRUTIL::InputStrCsv( StrArray[ iCsvCur++ ], strName );
	const char* pName = strName.c_str();
	StringCchCopy( m_szName, CHAR_SZNAME, pName );

//	기획팀 요청으로 제거함
//	++iCsvCur;	// Crow Name

	m_emTribe = (EMTRIBE)atoi( StrArray[ iCsvCur++ ] );
	m_emCrow = (EMCROW)atoi( StrArray[ iCsvCur++ ] );
	m_emBright = (EMBRIGHT)atoi( StrArray[ iCsvCur++ ] );

	m_wLevel = (WORD)atoi( StrArray[ iCsvCur++ ] );

	m_dwHP = (DWORD)atol( StrArray[ iCsvCur++ ] );
	m_wMP = (WORD)atoi( StrArray[ iCsvCur++ ] );
	m_wSP = (WORD)atoi( StrArray[ iCsvCur++ ] );

	m_fIncHP = (float)atof( StrArray[ iCsvCur++ ] );
	m_fIncMP = (float)atof( StrArray[ iCsvCur++ ] );
	m_fIncSP = (float)atof( StrArray[ iCsvCur++ ] );

	m_wAvoidRate = (WORD)atoi( StrArray[ iCsvCur++ ] );
	m_wHitRate = (WORD)atoi( StrArray[ iCsvCur++ ] );

	m_wViewRange = (WORD)atoi( StrArray[ iCsvCur++ ] );
	m_wDefense = (WORD)atoi( StrArray[ iCsvCur++ ] );

	m_sResist.nFire = (short)atoi( StrArray[ iCsvCur++ ] );
	m_sResist.nIce = (short)atoi( StrArray[ iCsvCur++ ] );
	m_sResist.nElectric = (short)atoi( StrArray[ iCsvCur++ ] );
	m_sResist.nPoison = (short)atoi( StrArray[ iCsvCur++ ] );
	m_sResist.nSpirit = (short)atoi( StrArray[ iCsvCur++ ] );

	m_dwBonusExp = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	m_wBonusCP = (WORD)atoi( StrArray[ iCsvCur++ ] );

	m_bOverlapAttack = (bool)atoi( StrArray[ iCsvCur++ ] );

	// PET
	m_emPetType = (PETTYPE)atoi( StrArray[ iCsvCur++ ] );
	m_emSummonType = (SUMMON_TYPE)atoi( StrArray[ iCsvCur++ ] );
	m_emNPCType = (EMNPC_TYPE)atoi( StrArray[ iCsvCur++ ] );

	m_dwGenTime = (DWORD)atoi( StrArray[ iCsvCur++ ] );

	m_wGatherTimeLow = (WORD)atoi( StrArray[ iCsvCur++ ] );
	m_wGatherTimeHigh = (WORD)atoi( StrArray[ iCsvCur++ ] );
	m_fGatherRate = (float)atof( StrArray[ iCsvCur++ ] );
	m_wGatherAnimation = (WORD)atoi( StrArray[ iCsvCur++ ] );

	/*hide crow name, Juver, 2017/09/01 */
	m_bHideName = (bool)atoi( StrArray[ iCsvCur++ ] ); 

	/*crow notice, Juver, 2017/11/25 */
	m_dwNotice = (DWORD)atoi( StrArray[ iCsvCur++ ] );

	dummy_var_0 = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	dummy_var_1 = (bool)atoi( StrArray[ iCsvCur++ ] ); 
	dummy_var_2 = (bool)atoi( StrArray[ iCsvCur++ ] ); 

	/*pvp capture the flag, Juver, 2018/02/03 */
	m_emCTFType = (EMNPC_CTF_FLAG_TYPE)atoi( StrArray[ iCsvCur++ ] );
}