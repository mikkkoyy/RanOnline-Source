#include "stdafx.h"
#include "EditorSkill.h"
#include "PageEdit1.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "GLStringTable.h"

BOOL CPageEdit1::DataDefault ()
{
	SetWin_Combo_Init( this, IDC_COMBO_EMROLE, COMMENT::SKILL_ROLE, SKILL::EMROLE_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_EMAPPLY, COMMENT::SKILL_APPLY, SKILL::EMAPPLY_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_EMACTION, COMMENT::SKILL_ACTION_TYPE, SKILL::EMACTION_NSIZE );

	SetWin_Combo_Init( this, IDC_COMBO_EMIMPACT_TAR, COMMENT::IMPACT_TAR, TAR_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_EMIMPACT_REALM, COMMENT::IMPACT_REALM, REALM_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_EMIMPACT_SIDE, COMMENT::IMPACT_SIDE, SIDE_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_LITEM, COMMENT::SKILLATTACK, SKILLATT_NSIZE+1 );
	SetWin_Combo_Init( this, IDC_COMBO_RITEM, COMMENT::SKILLATTACK, SKILLATT_NSIZE+1 );
	SetWin_Combo_Init( this, IDC_COMBO_ATTRIBUTE, COMMENT::BRIGHT, BRIGHT_SIZE );

	SetWin_Combo_Init( this, IDC_COMBO_EMTYPES, COMMENT::SKILL_TYPES, SKILL::FOR_TYPE_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_EMELEMENT, COMMENT::ELEMENT, EMELEMENT_MAXNUM2 );
	SetWin_Combo_Init( this, IDC_COMBO_EMSTATEBLOW, COMMENT::BLOW, EMBLOW_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_EMIMPACT, COMMENT::IMPACT_ADDON, EIMPACTA_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_EMSPEC, COMMENT::SPEC_ADDON, EMSPECA_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_EMSKILLSPECIAL, COMMENT::SPECIAL_SKILL_TYPE, SKILL::EMSSTYPE_NSIZE );

	SetWin_Combo_Init( this, IDC_COMBO_ANI_MAIN, COMMENT::ANI_MAINTYPE_CHAR, AN_TYPE_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_ANI_SUB, COMMENT::ANI_SUBTYPE_SKILL, AN_SUB_00_SIZE );

	SetWin_Combo_Init( this, IDC_COMBO_TIME_TARGET1, COMMENT::SKILL_EFFTIME, SKILL::EMTIME_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_TIME_TARGET2, COMMENT::SKILL_EFFTIME, SKILL::EMTIME_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_TIME_TARGET3, COMMENT::SKILL_EFFTIME, SKILL::EMTIME_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_TIME_SELF1, COMMENT::SKILL_EFFTIME, SKILL::EMTIME_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_TIME_SELF2, COMMENT::SKILL_EFFTIME, SKILL::EMTIME_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_TIME_SELF3, COMMENT::SKILL_EFFTIME, SKILL::EMTIME_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_TIME_TARG, COMMENT::SKILL_EFFTIME, SKILL::EMTIME_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_TIME_SELFBODY, COMMENT::SKILL_EFFTIME, SKILL::EMTIME_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_TIME_TARGETBODY1, COMMENT::SKILL_EFFTIME, SKILL::EMTIME_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_TIME_TARGETBODY2, COMMENT::SKILL_EFFTIME, SKILL::EMTIME_NSIZE );

	SetWin_Combo_Init( this, IDC_COMBO_POS_TARGET1, COMMENT::SKILL_EFFPOS, SKILL::EMPOS_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_POS_TARGET2, COMMENT::SKILL_EFFPOS, SKILL::EMPOS_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_POS_TARGET3, COMMENT::SKILL_EFFPOS, SKILL::EMPOS_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_POS_SELF1, COMMENT::SKILL_EFFPOS, SKILL::EMPOS_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_POS_SELF2, COMMENT::SKILL_EFFPOS, SKILL::EMPOS_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_POS_SELF3, COMMENT::SKILL_EFFPOS, SKILL::EMPOS_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_POS_TARG_A, COMMENT::SKILL_EFFPOS, SKILL::EMPOS_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_POS_TARG_B, COMMENT::SKILL_EFFPOS, SKILL::EMPOS_NSIZE );
	
	m_nElement = 0;
	m_nLevel = 0;
	m_nIndexImpact = 0;
	m_nIndexSpec = 0;

	return TRUE;
}

BOOL CPageEdit1::DataShow ()
{
	if ( !m_pDummySkill )	return FALSE;

	//basic
	SetWin_Num_int( this, IDC_EDIT_SKILLID_MID, m_pDummySkill->m_sBASIC.sNATIVEID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SKILLID_SID, m_pDummySkill->m_sBASIC.sNATIVEID.wSubID );
	//SetWin_Num_int( this, IDC_EDIT_SKILLID_GROUP, m_pDummySkill->m_sBASIC.wData );

	SetWin_Text( this, IDC_EDIT_NAME, m_pDummySkill->m_sBASIC.szNAME );
	SetWin_Text( this, IDC_EDIT_COMMENT, m_pDummySkill->m_sEXT_DATA.strCOMMENTS.c_str() );

	const char* szName =  GLStringTable::GetInstance().GetString( GetWin_Text( this, IDC_EDIT_NAME ).GetString(), GLStringTable::SKILL );
	const char* szComment =  GLStringTable::GetInstance().GetString( GetWin_Text( this, IDC_EDIT_COMMENT ).GetString(), GLStringTable::SKILL );
	SetWin_Text ( this, IDC_EDIT_NAME2, szName? szName: "" );	
	SetWin_Text ( this, IDC_EDIT_COMMENT2, szComment? szComment: "" );	

	SetWin_Combo_Sel( this, IDC_COMBO_EMROLE, COMMENT::SKILL_ROLE[m_pDummySkill->m_sBASIC.emROLE].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_EMAPPLY, COMMENT::SKILL_APPLY[m_pDummySkill->m_sBASIC.emAPPLY].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_EMACTION, COMMENT::SKILL_ACTION_TYPE[m_pDummySkill->m_sBASIC.emACTION].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_EMIMPACT_TAR, COMMENT::IMPACT_TAR[m_pDummySkill->m_sBASIC.emIMPACT_TAR].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_EMIMPACT_REALM, COMMENT::IMPACT_REALM[m_pDummySkill->m_sBASIC.emIMPACT_REALM].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_EMIMPACT_SIDE, COMMENT::IMPACT_SIDE[m_pDummySkill->m_sBASIC.emIMPACT_SIDE].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_LITEM, COMMENT::SKILLATTACK[m_pDummySkill->m_sBASIC.emUSE_LITEM].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_RITEM, COMMENT::SKILLATTACK[m_pDummySkill->m_sBASIC.emUSE_RITEM].c_str() );
	SetWin_Num_int( this, IDC_EDIT_GRADE, m_pDummySkill->m_sBASIC.dwGRADE );
	SetWin_Num_int( this, IDC_EDIT_MAXLEVEL, m_pDummySkill->m_sBASIC.dwMAXLEVEL );
	SetWin_Num_int( this, IDC_EDIT_TARRANGE, m_pDummySkill->m_sBASIC.wTARRANGE );
	SetWin_Check( this, IDC_CHECK_LEARN, m_pDummySkill->m_sBASIC.bLearnView );

	SetWin_Check ( this, IDC_CHECK_RUNNINGCAST, m_pDummySkill->m_sBASIC.dwFlags & FLAG_SB_RUNNING_CAST );
	
	if( m_pDummySkill->m_sBASIC.dwFlags & FLAG_SB_RUNNING_CAST )
	{
		SetWin_Num_float ( this, IDC_EDIT_RUNNINGCAST, m_pDummySkill->m_sAPPLY.fRunningEffTime );
	}
	
	//apply
	SetWin_Combo_Sel( this, IDC_COMBO_EMTYPES, COMMENT::SKILL_TYPES[m_pDummySkill->m_sAPPLY.emBASIC_TYPE].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_EMELEMENT, COMMENT::ELEMENT[m_pDummySkill->m_sAPPLY.emELEMENT].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_EMSTATEBLOW, COMMENT::BLOW[m_pDummySkill->m_sAPPLY.emSTATE_BLOW].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_EMSKILLSPECIAL, COMMENT::SPECIAL_SKILL_TYPE[m_pDummySkill->m_sSPECIAL_SKILL.emSSTYPE].c_str() );

	SetWin_Check ( this, IDC_CHECK_CURE_NUMB, m_pDummySkill->m_sAPPLY.dwCUREFLAG&DIS_NUMB );
	SetWin_Check ( this, IDC_CHECK_CURE_STUN, m_pDummySkill->m_sAPPLY.dwCUREFLAG&DIS_STUN );
	SetWin_Check ( this, IDC_CHECK_CURE_STONE, m_pDummySkill->m_sAPPLY.dwCUREFLAG&DIS_STONE );
	SetWin_Check ( this, IDC_CHECK_CURE_BURN, m_pDummySkill->m_sAPPLY.dwCUREFLAG&DIS_BURN );
	SetWin_Check ( this, IDC_CHECK_CURE_FROZEN, m_pDummySkill->m_sAPPLY.dwCUREFLAG&DIS_FROZEN );
	SetWin_Check ( this, IDC_CHECK_CURE_MAD, m_pDummySkill->m_sAPPLY.dwCUREFLAG&DIS_MAD );
	SetWin_Check ( this, IDC_CHECK_CURE_POISON, m_pDummySkill->m_sAPPLY.dwCUREFLAG&DIS_POISON );
	SetWin_Check ( this, IDC_CHECK_CURE_CURSE, m_pDummySkill->m_sAPPLY.dwCUREFLAG&DIS_CURSE );

	//learn
	SetWin_Combo_Sel( this, IDC_COMBO_ATTRIBUTE, COMMENT::BRIGHT[m_pDummySkill->m_sLEARN.emBRIGHT].c_str() );
	SetWin_Num_int( this, IDC_EDIT_REQSKILL_MID, m_pDummySkill->m_sLEARN.sSKILL.wMainID );
	SetWin_Num_int( this, IDC_EDIT_REQSKILL_SID, m_pDummySkill->m_sLEARN.sSKILL.wSubID );

	//ext data
	SetWin_Combo_Sel( this, IDC_COMBO_ANI_MAIN, COMMENT::ANI_MAINTYPE_CHAR[m_pDummySkill->m_sEXT_DATA.emANIMTYPE].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_ANI_SUB, COMMENT::ANI_SUBTYPE_SKILL[m_pDummySkill->m_sEXT_DATA.emANISTYPE].c_str() );
	SetWin_Num_int( this, IDC_EDIT_EXT_DATA, m_pDummySkill->m_sEXT_DATA.dwVal );

	SetWin_Text( this, IDC_EDIT_ICONFILE, m_pDummySkill->m_sEXT_DATA.strICONFILE.c_str() );
	SetWin_Num_int( this, IDC_EDIT_ICON_INDEX_X, m_pDummySkill->m_sEXT_DATA.sICONINDEX.wMainID );
	SetWin_Num_int( this, IDC_EDIT_ICON_INDEX_Y, m_pDummySkill->m_sEXT_DATA.sICONINDEX.wSubID );
	SetWin_Num_float( this, IDC_EDIT_DELAYDAMAGE, m_pDummySkill->m_sEXT_DATA.fDELAY4DAMAGE );
	SetWin_Check( this, IDC_CHECK_EFF_SINGLE, m_pDummySkill->m_sEXT_DATA.bTARG_ONE );

	SetWin_Combo_Sel( this, IDC_COMBO_TIME_TARGET1, COMMENT::SKILL_EFFTIME[m_pDummySkill->m_sEXT_DATA.emTARGZONE01].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_TIME_TARGET2, COMMENT::SKILL_EFFTIME[m_pDummySkill->m_sEXT_DATA.emTARGZONE02].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_TIME_TARGET3, COMMENT::SKILL_EFFTIME[m_pDummySkill->m_sEXT_DATA.emTARGZONE03].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_TIME_SELF1, COMMENT::SKILL_EFFTIME[m_pDummySkill->m_sEXT_DATA.emSELFZONE01].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_TIME_SELF2, COMMENT::SKILL_EFFTIME[m_pDummySkill->m_sEXT_DATA.emSELFZONE02].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_TIME_SELF3, COMMENT::SKILL_EFFTIME[m_pDummySkill->m_sEXT_DATA.emSELFZONE03].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_TIME_TARG, COMMENT::SKILL_EFFTIME[m_pDummySkill->m_sEXT_DATA.emTARG].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_TIME_SELFBODY, COMMENT::SKILL_EFFTIME[m_pDummySkill->m_sEXT_DATA.emSELFBODY].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_TIME_TARGETBODY1, COMMENT::SKILL_EFFTIME[m_pDummySkill->m_sEXT_DATA.emTARGBODY01].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_TIME_TARGETBODY2, COMMENT::SKILL_EFFTIME[m_pDummySkill->m_sEXT_DATA.emTARGBODY02].c_str() );

	SetWin_Combo_Sel( this, IDC_COMBO_POS_TARGET1, COMMENT::SKILL_EFFPOS[m_pDummySkill->m_sEXT_DATA.emTARGZONE01_POS].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_POS_TARGET2, COMMENT::SKILL_EFFPOS[m_pDummySkill->m_sEXT_DATA.emTARGZONE02_POS].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_POS_TARGET3, COMMENT::SKILL_EFFPOS[m_pDummySkill->m_sEXT_DATA.emTARGZONE03_POS].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_POS_SELF1, COMMENT::SKILL_EFFPOS[m_pDummySkill->m_sEXT_DATA.emSELFZONE01_POS].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_POS_SELF2, COMMENT::SKILL_EFFPOS[m_pDummySkill->m_sEXT_DATA.emSELFZONE02_POS].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_POS_SELF3, COMMENT::SKILL_EFFPOS[m_pDummySkill->m_sEXT_DATA.emSELFZONE03_POS].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_POS_TARG_A, COMMENT::SKILL_EFFPOS[m_pDummySkill->m_sEXT_DATA.emTARG_POSA].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_POS_TARG_B, COMMENT::SKILL_EFFPOS[m_pDummySkill->m_sEXT_DATA.emTARG_POSB].c_str() );

	SetWin_Text( this, IDC_EDIT_EFF_HOLDOUT, m_pDummySkill->m_sEXT_DATA.strHOLDOUT.c_str() );

	//other
	CheckReqSkill();
	ShowAnimationNumber();
	InitBasicType();

	EffectsShow(); //efects
	SkillLearnShow(); //learn level
	ShowImpact(); //impact
	ShowSpec(); //spec


	//unknown data
	//SetWin_Num_int( this, IDC_EDIT_EX_DATA_1, m_pDummySkill->m_sBASIC.dwData );
	SetWin_Num_int( this, IDC_EDIT_RUNNINGCAST, m_pDummySkill->m_sAPPLY.dwUnknownData );
	SetWin_Num_int( this, IDC_EDIT_EX_DATA_3, m_pDummySkill->m_sEXT_DATA.sADDSKILL.wMainID );
	SetWin_Num_int( this, IDC_EDIT_EX_DATA_4, m_pDummySkill->m_sEXT_DATA.sADDSKILL.wSubID );
	SetWin_Num_int( this, IDC_EDIT_EX_DATA_5, m_pDummySkill->m_sEXT_DATA.dwADDSKILLLEVEL );

	SetWin_Num_int( this, IDC_EDIT_EX_DATA_6, m_pDummySkill->m_sEXT_DATA.sLINKSKILL.wMainID );
	SetWin_Num_int( this, IDC_EDIT_EX_DATA_7, m_pDummySkill->m_sEXT_DATA.sLINKSKILL.wSubID );

	SetWin_Check( this, IDC_CHECK_HIDDENWEAPON, m_pDummySkill->m_sLEARN.bHiddenWeapon );

	if( m_pDummySkill->m_sLEARN.bHiddenWeapon )
	{
		SetWin_Enable ( this, IDC_EDIT_HIDDENWEAPON_MID, TRUE );
		SetWin_Enable ( this, IDC_EDIT_HIDDENWEAPON_SID, TRUE );
	}
	else
	{
		SetWin_Enable ( this, IDC_EDIT_HIDDENWEAPON_MID, FALSE );
		SetWin_Enable ( this, IDC_EDIT_HIDDENWEAPON_SID, FALSE );
	}

	//	글러브 착용 후 스킬 사용 시 아이템 변경ID
	SetWin_Num_int ( this, IDC_EDIT_HIDDENWEAPON_MID, m_pDummySkill->m_sLEARN.sHiddenWeapon.wMainID );
	SetWin_Num_int ( this, IDC_EDIT_HIDDENWEAPON_SID, m_pDummySkill->m_sLEARN.sHiddenWeapon.wSubID );

	return TRUE;
}

void CPageEdit1::OnBnClickedCheckHiddenWeapon()
{
	if( GetWin_Check( this, IDC_CHECK_HIDDENWEAPON ) )
	{
		SetWin_Enable ( this, IDC_EDIT_HIDDENWEAPON_MID, TRUE );
		SetWin_Enable ( this, IDC_EDIT_HIDDENWEAPON_SID, TRUE );
	}
	else
	{
		SetWin_Enable ( this, IDC_EDIT_HIDDENWEAPON_MID, FALSE );
		SetWin_Enable ( this, IDC_EDIT_HIDDENWEAPON_SID, FALSE );
	}
}


BOOL CPageEdit1::DataSave ()
{
	if ( !m_pDummySkill )	return FALSE;

	//basic
	m_pDummySkill->m_sBASIC.sNATIVEID.wMainID = GetWin_Num_int( this, IDC_EDIT_SKILLID_MID );
	m_pDummySkill->m_sBASIC.sNATIVEID.wSubID = GetWin_Num_int( this, IDC_EDIT_SKILLID_SID );
	//m_pDummySkill->m_sBASIC.wData = GetWin_Num_int( this, IDC_EDIT_SKILLID_GROUP );

	std::string strNAME = GetWin_Text( this, IDC_EDIT_NAME ).GetString();
	StringCchCopy( m_pDummySkill->m_sBASIC.szNAME, SKILL::MAX_SZNAME, strNAME.c_str() );
	m_pDummySkill->m_sEXT_DATA.strCOMMENTS = GetWin_Text( this, IDC_EDIT_COMMENT ).GetString();

	m_pDummySkill->m_sBASIC.emROLE = static_cast<SKILL::EMROLE> ( GetWin_Combo_Sel( this, IDC_COMBO_EMROLE ) );
	m_pDummySkill->m_sBASIC.emAPPLY = static_cast<SKILL::EMAPPLY> ( GetWin_Combo_Sel( this, IDC_COMBO_EMAPPLY ) );
	m_pDummySkill->m_sBASIC.emACTION = static_cast<SKILL::EMACTION> ( GetWin_Combo_Sel( this, IDC_COMBO_EMACTION ) );
	m_pDummySkill->m_sBASIC.emIMPACT_TAR = static_cast<EMIMPACT_TAR> ( GetWin_Combo_Sel( this, IDC_COMBO_EMIMPACT_TAR ) );
	m_pDummySkill->m_sBASIC.emIMPACT_REALM = static_cast<EMIMPACT_REALM> ( GetWin_Combo_Sel( this, IDC_COMBO_EMIMPACT_REALM ) );
	m_pDummySkill->m_sBASIC.emIMPACT_SIDE = static_cast<EMIMPACT_SIDE> ( GetWin_Combo_Sel( this, IDC_COMBO_EMIMPACT_SIDE ) );
	m_pDummySkill->m_sBASIC.emUSE_LITEM = static_cast<GLSKILL_ATT> ( GetWin_Combo_Sel( this, IDC_COMBO_LITEM ) );
	m_pDummySkill->m_sBASIC.emUSE_RITEM = static_cast<GLSKILL_ATT> ( GetWin_Combo_Sel( this, IDC_COMBO_RITEM ) );
	m_pDummySkill->m_sBASIC.dwGRADE = GetWin_Num_int( this, IDC_EDIT_GRADE );
	m_pDummySkill->m_sBASIC.dwMAXLEVEL = GetWin_Num_int( this, IDC_EDIT_MAXLEVEL );
	m_pDummySkill->m_sBASIC.wTARRANGE = GetWin_Num_int( this, IDC_EDIT_TARRANGE );
	m_pDummySkill->m_sBASIC.bLearnView = ( GetWin_Check( this, IDC_CHECK_LEARN ) == TRUE );
	m_pDummySkill->m_sBASIC.dwFlags = 0;
	m_pDummySkill->m_sBASIC.dwFlags |= (GetWin_Check( this, IDC_CHECK_RUNNINGCAST ) ? FLAG_SB_RUNNING_CAST : 0); // by paran

	if( m_pDummySkill->m_sBASIC.dwFlags & FLAG_SB_RUNNING_CAST )
		m_pDummySkill->m_sAPPLY.fRunningEffTime					= GetWin_Num_float( this, IDC_EDIT_RUNNINGCAST );

	//apply
	m_pDummySkill->m_sAPPLY.emBASIC_TYPE = static_cast<SKILL::EMTYPES> ( GetWin_Combo_Sel( this, IDC_COMBO_EMTYPES ) );
	m_pDummySkill->m_sAPPLY.emELEMENT = static_cast<EMELEMENT> ( GetWin_Combo_Sel( this, IDC_COMBO_EMELEMENT ) );
	m_pDummySkill->m_sAPPLY.emSTATE_BLOW = static_cast<EMSTATE_BLOW> ( GetWin_Combo_Sel( this, IDC_COMBO_EMSTATEBLOW ) );
	m_pDummySkill->m_sSPECIAL_SKILL.emSSTYPE = static_cast<SKILL::EMSPECIALSKILLTYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_EMSKILLSPECIAL ) );

	SetCheck_Flags( GetWin_Check ( this, IDC_CHECK_CURE_NUMB ), m_pDummySkill->m_sAPPLY.dwCUREFLAG, DIS_NUMB );
	SetCheck_Flags( GetWin_Check ( this, IDC_CHECK_CURE_STUN ), m_pDummySkill->m_sAPPLY.dwCUREFLAG, DIS_STUN );
	SetCheck_Flags( GetWin_Check ( this, IDC_CHECK_CURE_STONE ), m_pDummySkill->m_sAPPLY.dwCUREFLAG, DIS_STONE );
	SetCheck_Flags( GetWin_Check ( this, IDC_CHECK_CURE_BURN ), m_pDummySkill->m_sAPPLY.dwCUREFLAG, DIS_BURN );
	SetCheck_Flags( GetWin_Check ( this, IDC_CHECK_CURE_FROZEN ), m_pDummySkill->m_sAPPLY.dwCUREFLAG, DIS_FROZEN );
	SetCheck_Flags( GetWin_Check ( this, IDC_CHECK_CURE_MAD ), m_pDummySkill->m_sAPPLY.dwCUREFLAG, DIS_MAD );
	SetCheck_Flags( GetWin_Check ( this, IDC_CHECK_CURE_POISON ), m_pDummySkill->m_sAPPLY.dwCUREFLAG, DIS_POISON );
	SetCheck_Flags( GetWin_Check ( this, IDC_CHECK_CURE_CURSE ), m_pDummySkill->m_sAPPLY.dwCUREFLAG, DIS_CURSE );

	//learn
	m_pDummySkill->m_sLEARN.emBRIGHT = static_cast<EMBRIGHT> ( GetWin_Combo_Sel( this, IDC_COMBO_ATTRIBUTE ) );
	m_pDummySkill->m_sLEARN.sSKILL.wMainID = GetWin_Num_int( this, IDC_EDIT_REQSKILL_MID );
	m_pDummySkill->m_sLEARN.sSKILL.wSubID = GetWin_Num_int( this, IDC_EDIT_REQSKILL_SID );

	//ext data
	m_pDummySkill->m_sEXT_DATA.emANIMTYPE = static_cast<EMANI_MAINTYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_ANI_MAIN ) );
	m_pDummySkill->m_sEXT_DATA.emANISTYPE = static_cast<EMANI_SUBTYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_ANI_SUB ) );
	m_pDummySkill->m_sEXT_DATA.dwVal = GetWin_Num_int( this, IDC_EDIT_EXT_DATA );
	m_pDummySkill->m_sEXT_DATA.strICONFILE = GetWin_Text( this, IDC_EDIT_ICONFILE ).GetString();
	m_pDummySkill->m_sEXT_DATA.sICONINDEX.wMainID = GetWin_Num_int( this, IDC_EDIT_ICON_INDEX_X );
	m_pDummySkill->m_sEXT_DATA.sICONINDEX.wSubID = GetWin_Num_int( this, IDC_EDIT_ICON_INDEX_Y );
	m_pDummySkill->m_sEXT_DATA.fDELAY4DAMAGE = GetWin_Num_float( this, IDC_EDIT_DELAYDAMAGE );
	m_pDummySkill->m_sEXT_DATA.bTARG_ONE = ( GetWin_Check( this, IDC_CHECK_EFF_SINGLE ) == TRUE );

	m_pDummySkill->m_sEXT_DATA.emTARGZONE01 = static_cast<SKILL::EMEFFECTIME> ( GetWin_Combo_Sel( this, IDC_COMBO_TIME_TARGET1 ) );
	m_pDummySkill->m_sEXT_DATA.emTARGZONE02 = static_cast<SKILL::EMEFFECTIME> ( GetWin_Combo_Sel( this, IDC_COMBO_TIME_TARGET2 ) );
	m_pDummySkill->m_sEXT_DATA.emTARGZONE03 = static_cast<SKILL::EMEFFECTIME> ( GetWin_Combo_Sel( this, IDC_COMBO_TIME_TARGET3 ) );
	m_pDummySkill->m_sEXT_DATA.emSELFZONE01 = static_cast<SKILL::EMEFFECTIME> ( GetWin_Combo_Sel( this, IDC_COMBO_TIME_SELF1 ) );
	m_pDummySkill->m_sEXT_DATA.emSELFZONE02 = static_cast<SKILL::EMEFFECTIME> ( GetWin_Combo_Sel( this, IDC_COMBO_TIME_SELF2 ) );
	m_pDummySkill->m_sEXT_DATA.emSELFZONE03 = static_cast<SKILL::EMEFFECTIME> ( GetWin_Combo_Sel( this, IDC_COMBO_TIME_SELF3 ) );
	m_pDummySkill->m_sEXT_DATA.emTARG = static_cast<SKILL::EMEFFECTIME> ( GetWin_Combo_Sel( this, IDC_COMBO_TIME_TARG ) );
	m_pDummySkill->m_sEXT_DATA.emSELFBODY = static_cast<SKILL::EMEFFECTIME> ( GetWin_Combo_Sel( this, IDC_COMBO_TIME_SELFBODY ) );
	m_pDummySkill->m_sEXT_DATA.emTARGBODY01 = static_cast<SKILL::EMEFFECTIME> ( GetWin_Combo_Sel( this, IDC_COMBO_TIME_TARGETBODY1 ) );
	m_pDummySkill->m_sEXT_DATA.emTARGBODY02 = static_cast<SKILL::EMEFFECTIME> ( GetWin_Combo_Sel( this, IDC_COMBO_TIME_TARGETBODY2 ) );

	m_pDummySkill->m_sEXT_DATA.emTARGZONE01_POS = static_cast<SKILL::EMEFFECTPOS> ( GetWin_Combo_Sel( this, IDC_COMBO_POS_TARGET1 ) );
	m_pDummySkill->m_sEXT_DATA.emTARGZONE02_POS = static_cast<SKILL::EMEFFECTPOS> ( GetWin_Combo_Sel( this, IDC_COMBO_POS_TARGET2 ) );
	m_pDummySkill->m_sEXT_DATA.emTARGZONE03_POS = static_cast<SKILL::EMEFFECTPOS> ( GetWin_Combo_Sel( this, IDC_COMBO_POS_TARGET3 ) );
	m_pDummySkill->m_sEXT_DATA.emSELFZONE01_POS = static_cast<SKILL::EMEFFECTPOS> ( GetWin_Combo_Sel( this, IDC_COMBO_POS_SELF1 ) );
	m_pDummySkill->m_sEXT_DATA.emSELFZONE02_POS = static_cast<SKILL::EMEFFECTPOS> ( GetWin_Combo_Sel( this, IDC_COMBO_POS_SELF2 ) );
	m_pDummySkill->m_sEXT_DATA.emSELFZONE03_POS = static_cast<SKILL::EMEFFECTPOS> ( GetWin_Combo_Sel( this, IDC_COMBO_POS_SELF3 ) );
	m_pDummySkill->m_sEXT_DATA.emTARG_POSA = static_cast<SKILL::EMEFFECTPOS> ( GetWin_Combo_Sel( this, IDC_COMBO_POS_TARG_A ) );
	m_pDummySkill->m_sEXT_DATA.emTARG_POSB = static_cast<SKILL::EMEFFECTPOS> ( GetWin_Combo_Sel( this, IDC_COMBO_POS_TARG_B ) );

	m_pDummySkill->m_sEXT_DATA.strHOLDOUT = GetWin_Text( this, IDC_EDIT_EFF_HOLDOUT ).GetString();

	EffectsSave();	//effects
	SkillLearnSave();	//learn level
	SaveImpact();	//impact
	SaveSpec();		//spec

	//unknown data
	//m_pDummySkill->m_sBASIC.dwData = GetWin_Num_int( this, IDC_EDIT_EX_DATA_1 );
	m_pDummySkill->m_sAPPLY.dwUnknownData = GetWin_Num_int( this, IDC_EDIT_RUNNINGCAST );
	m_pDummySkill->m_sEXT_DATA.sADDSKILL.wMainID = GetWin_Num_int( this, IDC_EDIT_EX_DATA_3 );
	m_pDummySkill->m_sEXT_DATA.sADDSKILL.wSubID = GetWin_Num_int( this, IDC_EDIT_EX_DATA_4 );
	m_pDummySkill->m_sEXT_DATA.dwADDSKILLLEVEL = GetWin_Num_int( this, IDC_EDIT_EX_DATA_5 );

	m_pDummySkill->m_sEXT_DATA.sLINKSKILL.wMainID = GetWin_Num_int( this, IDC_EDIT_EX_DATA_6 );
	m_pDummySkill->m_sEXT_DATA.sLINKSKILL.wSubID = GetWin_Num_int( this, IDC_EDIT_EX_DATA_7 );

	m_pDummySkill->m_sLEARN.bHiddenWeapon = GetWin_Check( this, IDC_CHECK_HIDDENWEAPON );

	//	글러브 착용 후 스킬 사용 시 아이템 변경ID
	m_pDummySkill->m_sLEARN.sHiddenWeapon.wMainID = GetWin_Num_int ( this, IDC_EDIT_HIDDENWEAPON_MID );
	m_pDummySkill->m_sLEARN.sHiddenWeapon.wSubID = GetWin_Num_int ( this, IDC_EDIT_HIDDENWEAPON_SID );

	return TRUE;
}

void CPageEdit1::EffectsShow()
{
	if ( !m_pDummySkill )	return;
	if ( m_nElement < 0	)		return;
	if ( m_nElement >= EMELEMENT_MAXNUM )	return;

	SetWin_Text( this, IDC_EDIT_EFFECT, COMMENT::ELEMENT[m_nElement].c_str() );

	SetWin_Text( this, IDC_EDIT_EFF_TARGET1, m_pDummySkill->m_sEXT_DATA.strTARGZONE01[m_nElement].c_str() );
	SetWin_Text( this, IDC_EDIT_EFF_TARGET2, m_pDummySkill->m_sEXT_DATA.strTARGZONE02[m_nElement].c_str() );
	SetWin_Text( this, IDC_EDIT_EFF_TARGET3, m_pDummySkill->m_sEXT_DATA.strTARGZONE03[m_nElement].c_str() );
	SetWin_Text( this, IDC_EDIT_EFF_SELF1, m_pDummySkill->m_sEXT_DATA.strSELFZONE01[m_nElement].c_str() );
	SetWin_Text( this, IDC_EDIT_EFF_SELF2, m_pDummySkill->m_sEXT_DATA.strSELFZONE02[m_nElement].c_str() );
	SetWin_Text( this, IDC_EDIT_EFF_SELF3, m_pDummySkill->m_sEXT_DATA.strSELFZONE03[m_nElement].c_str() );
	SetWin_Text( this, IDC_EDIT_EFF_TARG, m_pDummySkill->m_sEXT_DATA.strTARG[m_nElement].c_str() );
	SetWin_Text( this, IDC_EDIT_EFF_SELFBODY, m_pDummySkill->m_sEXT_DATA.strSELFBODY[m_nElement].c_str() );
	SetWin_Text( this, IDC_EDIT_EFF_TARGETBODY1, m_pDummySkill->m_sEXT_DATA.strTARGBODY01[m_nElement].c_str() );
	SetWin_Text( this, IDC_EDIT_EFF_TARGETBODY2, m_pDummySkill->m_sEXT_DATA.strTARGBODY02[m_nElement].c_str() );
}

void CPageEdit1::EffectsSave()
{
	if ( !m_pDummySkill )	return;
	if ( m_nElement < 0	)		return;
	if ( m_nElement >= EMELEMENT_MAXNUM )	return;

	m_pDummySkill->m_sEXT_DATA.strTARGZONE01[m_nElement] = GetWin_Text( this, IDC_EDIT_EFF_TARGET1 ).GetString();
	m_pDummySkill->m_sEXT_DATA.strTARGZONE02[m_nElement] = GetWin_Text( this, IDC_EDIT_EFF_TARGET2 ).GetString();
	m_pDummySkill->m_sEXT_DATA.strTARGZONE03[m_nElement] = GetWin_Text( this, IDC_EDIT_EFF_TARGET3 ).GetString();
	m_pDummySkill->m_sEXT_DATA.strSELFZONE01[m_nElement] = GetWin_Text( this, IDC_EDIT_EFF_SELF1 ).GetString();
	m_pDummySkill->m_sEXT_DATA.strSELFZONE02[m_nElement] = GetWin_Text( this, IDC_EDIT_EFF_SELF2 ).GetString();
	m_pDummySkill->m_sEXT_DATA.strSELFZONE03[m_nElement] = GetWin_Text( this, IDC_EDIT_EFF_SELF3 ).GetString();
	m_pDummySkill->m_sEXT_DATA.strTARG[m_nElement] = GetWin_Text( this, IDC_EDIT_EFF_TARG ).GetString();
	m_pDummySkill->m_sEXT_DATA.strSELFBODY[m_nElement] = GetWin_Text( this, IDC_EDIT_EFF_SELFBODY ).GetString();
	m_pDummySkill->m_sEXT_DATA.strTARGBODY01[m_nElement] = GetWin_Text( this, IDC_EDIT_EFF_TARGETBODY1 ).GetString();
	m_pDummySkill->m_sEXT_DATA.strTARGBODY02[m_nElement] = GetWin_Text( this, IDC_EDIT_EFF_TARGETBODY2 ).GetString();
}

void CPageEdit1::SkillLearnShow()
{
	if ( !m_pDummySkill )	return;
	if ( m_nLevel < 0 )		return;
	if ( m_nLevel >= SKILL::MAX_LEVEL )	return;


	SetWin_Text( this, IDC_EDIT_SKLVL, COMMENT::SKILL_LEVEL[m_nLevel].c_str() );

	//learn
	SetWin_Num_int( this, IDC_EDIT_LEARN_LEVEL, m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].dwLEVEL );
	SetWin_Num_int( this, IDC_EDIT_LEARN_SKILLPOINT, m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].dwSKP );
	SetWin_Num_int( this, IDC_EDIT_LEARN_SKILLLEVEL, m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].dwSKILL_LVL );
	SetWin_Num_int( this, IDC_EDIT_LEARN_STAT_POW, m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].sSTATS.wPow );
	SetWin_Num_int( this, IDC_EDIT_LEARN_STAT_SPI, m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].sSTATS.wSpi );
	SetWin_Num_int( this, IDC_EDIT_LEARN_STAT_INT, m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].sSTATS.wInt );
	SetWin_Num_int( this, IDC_EDIT_LEARN_STAT_STR, m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].sSTATS.wStr );
	SetWin_Num_int( this, IDC_EDIT_LEARN_STAT_DEX, m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].sSTATS.wDex );
	SetWin_Num_int( this, IDC_EDIT_LEARN_STAT_STA, m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].sSTATS.wSta );
	//Money Required to Unlock Skill
	//SetWin_Num_int( this, IDC_EDIT_REQMONEY, m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].dwMoneyReq );
}

void CPageEdit1::SkillLearnSave()
{
	if ( !m_pDummySkill )	return;
	if ( m_nLevel < 0 )		return;
	if ( m_nLevel >= SKILL::MAX_LEVEL )	return;

	//learn
	m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].dwLEVEL = GetWin_Num_int( this, IDC_EDIT_LEARN_LEVEL );
	m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].dwSKP = GetWin_Num_int( this, IDC_EDIT_LEARN_SKILLPOINT );
	m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].dwSKILL_LVL = GetWin_Num_int( this, IDC_EDIT_LEARN_SKILLLEVEL );
	m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].sSTATS.wPow = GetWin_Num_int( this, IDC_EDIT_LEARN_STAT_POW );
	m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].sSTATS.wSpi = GetWin_Num_int( this, IDC_EDIT_LEARN_STAT_SPI );
	m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].sSTATS.wInt = GetWin_Num_int( this, IDC_EDIT_LEARN_STAT_INT );
	m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].sSTATS.wStr = GetWin_Num_int( this, IDC_EDIT_LEARN_STAT_STR );
	m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].sSTATS.wDex = GetWin_Num_int( this, IDC_EDIT_LEARN_STAT_DEX );
	m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].sSTATS.wSta = GetWin_Num_int( this, IDC_EDIT_LEARN_STAT_STA );
	//Money Required to Unlock Skill
	//m_pDummySkill->m_sLEARN.sLVL_STEP[m_nLevel].dwMoneyReq = GetWin_Num_int( this, IDC_EDIT_REQMONEY );
}

void CPageEdit1::ShowImpact()
{
	if ( !m_pDummySkill )		return;
	if ( m_nIndexImpact < 0 )	return;
	if ( m_nIndexImpact >= SKILL::MAX_IMPACT )	return;

	SetWin_Num_int( this, IDC_EDIT_IMPACT_NUM, m_nIndexImpact+1 );

	SetWin_Combo_Sel( this, IDC_COMBO_EMIMPACT, COMMENT::IMPACT_ADDON[m_pDummySkill->m_sAPPLY.sImpacts[m_nIndexImpact].emADDON].c_str() );
}

void CPageEdit1::ShowSpec()
{
	if ( !m_pDummySkill )		return;
	if ( m_nIndexSpec < 0 )		return;
	if ( m_nIndexSpec >= SKILL::MAX_SPEC )	return;

	SetWin_Num_int( this, IDC_EDIT_SPEC_NUM, m_nIndexSpec+1 );

	SetWin_Combo_Sel( this, IDC_COMBO_EMSPEC, COMMENT::SPEC_ADDON[m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].emSPEC].c_str() );
}

void CPageEdit1::SaveImpact()
{
	if ( !m_pDummySkill )		return;
	if ( m_nIndexImpact < 0 )	return;
	if ( m_nIndexImpact >= SKILL::MAX_IMPACT )	return;

	m_pDummySkill->m_sAPPLY.sImpacts[m_nIndexImpact].emADDON = static_cast<EMIMPACT_ADDON> ( GetWin_Combo_Sel( this, IDC_COMBO_EMIMPACT ) ); 
}

void CPageEdit1::SaveSpec()
{
	if ( !m_pDummySkill )		return;
	if ( m_nIndexSpec < 0 )		return;
	if ( m_nIndexSpec >= SKILL::MAX_SPEC )	return;

	m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].emSPEC = static_cast<EMSPEC_ADDON> ( GetWin_Combo_Sel( this, IDC_COMBO_EMSPEC ) );
}