#include "stdafx.h"
#include "EditorSkill.h"
#include "PageEdit2.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"

BOOL CPageEdit2::DataDefault ()
{
	m_nLevel = 0;
	m_nIndexImpact = 0;
	m_nIndexSpec = 0;

	/*debuff skill logic, Juver, 2017/06/10 */
	SetWin_Combo_Init( this, IDC_COMBO_DEBUFF_ACTION, COMMENT::SKILL_ACTION_TYPE, SKILL::EMACTION_NSIZE );

	/*dash skill logic, Juver, 2017/06/17 */
	SetWin_Combo_Init( this, IDC_COMBO_ANI_SUB2, COMMENT::ANI_SUBTYPE_SKILL, AN_SUB_00_SIZE );

	return TRUE;
}

BOOL CPageEdit2::DataShow ()
{
	if ( !m_pDummySkill )	return FALSE;
	
	InitStateBlow();
	InitSpecialSkill();

	LevelShow();

	//special skill 
	SetWin_Text( this, IDC_EDIT_SPS_EFFECT, m_pDummySkill->m_sSPECIAL_SKILL.strEffectName.c_str() );
	SetWin_Num_int( this, IDC_EDIT_SPS_DURATION, m_pDummySkill->m_sSPECIAL_SKILL.dwRemainSecond );
	SetWin_Text( this, IDC_EDIT_HEAD_M, m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Man[0].c_str() );
	SetWin_Text( this, IDC_EDIT_UPPER_M, m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Man[1].c_str() );
	SetWin_Text( this, IDC_EDIT_LOWER_M, m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Man[2].c_str() );
	SetWin_Text( this, IDC_EDIT_HAND_M, m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Man[3].c_str() );
	SetWin_Text( this, IDC_EDIT_LHAND_M, m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Man[4].c_str() );
	SetWin_Text( this, IDC_EDIT_RHAND_M, m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Man[5].c_str() );
	SetWin_Text( this, IDC_EDIT_FOOT_M, m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Man[6].c_str() );
	SetWin_Text( this, IDC_EDIT_FACE_M, m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Man[7].c_str() );
	SetWin_Text( this, IDC_EDIT_HEAD_F, m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Woman[0].c_str() );
	SetWin_Text( this, IDC_EDIT_UPPER_F, m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Woman[1].c_str() );
	SetWin_Text( this, IDC_EDIT_LOWER_F, m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Woman[2].c_str() );
	SetWin_Text( this, IDC_EDIT_HAND_F, m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Woman[3].c_str() );
	SetWin_Text( this, IDC_EDIT_LHAND_F, m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Woman[4].c_str() );
	SetWin_Text( this, IDC_EDIT_RHAND_F, m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Woman[5].c_str() );
	SetWin_Text( this, IDC_EDIT_FOOT_F, m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Woman[6].c_str() );
	SetWin_Text( this, IDC_EDIT_FACE_F, m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Woman[7].c_str() );

	return TRUE;
}

BOOL CPageEdit2::DataSave ()
{
	if ( !m_pDummySkill )	return FALSE;

	LevelSave();

	//special skill 
	m_pDummySkill->m_sSPECIAL_SKILL.strEffectName = GetWin_Text( this, IDC_EDIT_SPS_EFFECT ).GetString();
	m_pDummySkill->m_sSPECIAL_SKILL.dwRemainSecond = GetWin_Num_int( this, IDC_EDIT_SPS_DURATION );
	m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Man[0] = GetWin_Text( this, IDC_EDIT_HEAD_M ).GetString();
	m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Man[1] = GetWin_Text( this, IDC_EDIT_UPPER_M ).GetString();
	m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Man[2] = GetWin_Text( this, IDC_EDIT_LOWER_M ).GetString();
	m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Man[3] = GetWin_Text( this, IDC_EDIT_HAND_M ).GetString();
	m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Man[4] = GetWin_Text( this, IDC_EDIT_LHAND_M ).GetString();
	m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Man[5] = GetWin_Text( this, IDC_EDIT_RHAND_M ).GetString();
	m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Man[6] = GetWin_Text( this, IDC_EDIT_FOOT_M ).GetString();
	m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Man[7] = GetWin_Text( this, IDC_EDIT_FACE_M ).GetString();
	m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Woman[0] = GetWin_Text( this, IDC_EDIT_HEAD_F ).GetString();
	m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Woman[1] = GetWin_Text( this, IDC_EDIT_UPPER_F ).GetString();
	m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Woman[2] = GetWin_Text( this, IDC_EDIT_LOWER_F ).GetString();
	m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Woman[3] = GetWin_Text( this, IDC_EDIT_HAND_F ).GetString();
	m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Woman[4] = GetWin_Text( this, IDC_EDIT_LHAND_F ).GetString();
	m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Woman[5] = GetWin_Text( this, IDC_EDIT_RHAND_F ).GetString();
	m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Woman[6] = GetWin_Text( this, IDC_EDIT_FOOT_F ).GetString();
	m_pDummySkill->m_sSPECIAL_SKILL.strTransform_Woman[7] = GetWin_Text( this, IDC_EDIT_FACE_F ).GetString();

	return TRUE;
}

void CPageEdit2::LevelShow()
{
	if ( !m_pDummySkill )	return;
	if ( m_nLevel < 0 )		return;
	if ( m_nLevel >= SKILL::MAX_LEVEL )	return;

	SetWin_Text( this, IDC_EDIT_LEVEL, COMMENT::SKILL_LEVEL[m_nLevel].c_str() );

	//cdata 
	SetWin_Text( this, IDC_STATIC_APPLY, COMMENT::SKILL_TYPES[m_pDummySkill->m_sAPPLY.emBASIC_TYPE].c_str() );
	SetWin_Num_float( this, IDC_EDIT_CDATA_DELAY, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].fDELAYTIME );
	SetWin_Num_float( this, IDC_EDIT_CDATA_DURATION, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].fLIFE );
	SetWin_Num_int( this, IDC_EDIT_CDATA_RANGE, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wAPPLYRANGE );
	SetWin_Num_int( this, IDC_EDIT_CDATA_STRIKENUM, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wAPPLYNUM );
	SetWin_Num_int( this, IDC_EDIT_CDATA_ANGLE, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wAPPLYANGLE );
	SetWin_Num_int( this, IDC_EDIT_CDATA_PENETRATION, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wPIERCENUM );
	SetWin_Num_int( this, IDC_EDIT_CDATA_TARGETNUM, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wTARNUM );
	SetWin_Num_float( this, IDC_EDIT_CDATA_VALUE, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].fBASIC_VAR );

	SetWin_Num_int( this, IDC_EDIT_CDATA_HP, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_HP );
	SetWin_Num_int( this, IDC_EDIT_CDATA_MP, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_MP );
	SetWin_Num_int( this, IDC_EDIT_CDATA_SP, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_SP );
	SetWin_Num_int( this, IDC_EDIT_CDATA_EXP, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_EXP );
	SetWin_Num_int( this, IDC_EDIT_CDATA_CP, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_CP );
	SetWin_Num_int( this, IDC_EDIT_CDATA_ARROW, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_ARROWNUM );
	SetWin_Num_int( this, IDC_EDIT_CDATA_CHARM, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_CHARMNUM );
	SetWin_Num_int( this, IDC_EDIT_CDATA_BULLET, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_BULLETNUM );
	SetWin_Num_int( this, IDC_EDIT_CDATA_HP_PTY, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_HP_PTY );
	SetWin_Num_int( this, IDC_EDIT_CDATA_MP_PTY, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_MP_PTY );
	SetWin_Num_int( this, IDC_EDIT_CDATA_SP_PTY, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_SP_PTY );
	SetWin_Num_int( this, IDC_EDIT_CDATA_DATA, m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].dwDATA );

	//apply
	//stateblow
	SetWin_Num_float( this, IDC_EDIT_BLOW_GEN, m_pDummySkill->m_sAPPLY.sSTATE_BLOW[m_nLevel].fRATE );
	SetWin_Num_float( this, IDC_EDIT_BLOW_VAR1, m_pDummySkill->m_sAPPLY.sSTATE_BLOW[m_nLevel].fVAR1 );
	SetWin_Num_float( this, IDC_EDIT_BLOW_VAR2, m_pDummySkill->m_sAPPLY.sSTATE_BLOW[m_nLevel].fVAR2 );

	//impact
	ImpactDataShow();

	//spec
	SpecDataShow();


	//summon
	SetWin_Num_int( this, IDC_EDIT_SUM_MID, m_pDummySkill->m_sSPECIAL_SKILL.sSummon[m_nLevel].sidCrow.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SUM_SID, m_pDummySkill->m_sSPECIAL_SKILL.sSummon[m_nLevel].sidCrow.wSubID );
	SetWin_Num_float( this, IDC_EDIT_SUM_DELAY, m_pDummySkill->m_sSPECIAL_SKILL.sSummon[m_nLevel].fDelay );
	SetWin_Num_float( this, IDC_EDIT_SUM_LIFE, m_pDummySkill->m_sSPECIAL_SKILL.sSummon[m_nLevel].fLife );
	SetWin_Num_int( this, IDC_EDIT_SUM_NUM, m_pDummySkill->m_sSPECIAL_SKILL.sSummon[m_nLevel].dwMaxNum );
	SetWin_Num_int( this, IDC_EDIT_SUM_SKILLLEVEL, m_pDummySkill->m_sSPECIAL_SKILL.sSummon[m_nLevel].wSkillLevel );
}

void CPageEdit2::LevelSave()
{
	if ( !m_pDummySkill )	return;
	if ( m_nLevel < 0 )		return;
	if ( m_nLevel >= SKILL::MAX_LEVEL )	return;

	//cdata 
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].fDELAYTIME = GetWin_Num_float( this, IDC_EDIT_CDATA_DELAY );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].fLIFE = GetWin_Num_float( this, IDC_EDIT_CDATA_DURATION );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wAPPLYRANGE = GetWin_Num_int( this, IDC_EDIT_CDATA_RANGE );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wAPPLYNUM = GetWin_Num_int( this, IDC_EDIT_CDATA_STRIKENUM );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wAPPLYANGLE = GetWin_Num_int( this, IDC_EDIT_CDATA_ANGLE );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wPIERCENUM = GetWin_Num_int( this, IDC_EDIT_CDATA_PENETRATION );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wTARNUM = GetWin_Num_int( this, IDC_EDIT_CDATA_TARGETNUM );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].fBASIC_VAR = GetWin_Num_float( this, IDC_EDIT_CDATA_VALUE );

	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_HP = GetWin_Num_int( this, IDC_EDIT_CDATA_HP );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_MP = GetWin_Num_int( this, IDC_EDIT_CDATA_MP );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_SP = GetWin_Num_int( this, IDC_EDIT_CDATA_SP );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_EXP = GetWin_Num_int( this, IDC_EDIT_CDATA_EXP );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_CP = GetWin_Num_int( this, IDC_EDIT_CDATA_CP );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_ARROWNUM = GetWin_Num_int( this, IDC_EDIT_CDATA_ARROW );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_CHARMNUM = GetWin_Num_int( this, IDC_EDIT_CDATA_CHARM );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_BULLETNUM = GetWin_Num_int( this, IDC_EDIT_CDATA_BULLET );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_HP_PTY = GetWin_Num_int( this, IDC_EDIT_CDATA_HP_PTY );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_MP_PTY = GetWin_Num_int( this, IDC_EDIT_CDATA_MP_PTY );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].wUSE_SP_PTY = GetWin_Num_int( this, IDC_EDIT_CDATA_SP_PTY );
	m_pDummySkill->m_sAPPLY.sDATA_LVL[m_nLevel].dwDATA = GetWin_Num_int( this, IDC_EDIT_CDATA_DATA );

	//apply
	//stateblow
	m_pDummySkill->m_sAPPLY.sSTATE_BLOW[m_nLevel].fRATE = GetWin_Num_float( this, IDC_EDIT_BLOW_GEN );
	m_pDummySkill->m_sAPPLY.sSTATE_BLOW[m_nLevel].fVAR1 = GetWin_Num_float( this, IDC_EDIT_BLOW_VAR1 );
	m_pDummySkill->m_sAPPLY.sSTATE_BLOW[m_nLevel].fVAR2 = GetWin_Num_float( this, IDC_EDIT_BLOW_VAR2 );

	//impact
	ImpactDataSave();

	//spec
	SpecDataSave();

	//summon
	m_pDummySkill->m_sSPECIAL_SKILL.sSummon[m_nLevel].sidCrow.wMainID = GetWin_Num_int( this, IDC_EDIT_SUM_MID );
	m_pDummySkill->m_sSPECIAL_SKILL.sSummon[m_nLevel].sidCrow.wSubID = GetWin_Num_int( this, IDC_EDIT_SUM_SID );
	m_pDummySkill->m_sSPECIAL_SKILL.sSummon[m_nLevel].fDelay = GetWin_Num_float( this, IDC_EDIT_SUM_DELAY );
	m_pDummySkill->m_sSPECIAL_SKILL.sSummon[m_nLevel].fLife = GetWin_Num_float( this, IDC_EDIT_SUM_LIFE );
	m_pDummySkill->m_sSPECIAL_SKILL.sSummon[m_nLevel].dwMaxNum = GetWin_Num_int( this, IDC_EDIT_SUM_NUM );
	m_pDummySkill->m_sSPECIAL_SKILL.sSummon[m_nLevel].wSkillLevel = GetWin_Num_int( this, IDC_EDIT_SUM_SKILLLEVEL );
}

void CPageEdit2::SpecDataShow()
{
	if ( !m_pDummySkill )	return;
	if ( m_nLevel < 0 )		return;
	if ( m_nLevel >= SKILL::MAX_LEVEL )	return;

	if ( m_nIndexSpec < 0 )		return;
	if ( m_nIndexSpec >= SKILL::MAX_SPEC )	return;

	InitSpec();

	SetWin_Num_int( this, IDC_EDIT_SPEC_NUM2, m_nIndexSpec+1 );

	//spec
	SetWin_Num_float( this, IDC_EDIT_SPEC_VAR1, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].fVAR1 );
	SetWin_Num_float( this, IDC_EDIT_SPEC_VAR2, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].fVAR2 );
	SetWin_Num_float( this, IDC_EDIT_SPEC_VAR3, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].fVAR3 );
	SetWin_Num_float( this, IDC_EDIT_SPEC_VAR4, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].fVAR4 );

	EMSPEC_ADDON emSPEC = m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].emSPEC;

	if ( emSPEC == EMSPECA_NONBLOW || emSPEC == EMSPECA_RECBLOW )
	{
		
		SetWin_Text( this, IDC_CHECK_CURE_NUMB, COMMENT::BLOW[0].c_str() );
		SetWin_Text( this, IDC_CHECK_CURE_STUN, COMMENT::BLOW[1].c_str() );
		SetWin_Text( this, IDC_CHECK_CURE_STONE, COMMENT::BLOW[2].c_str() );
		SetWin_Text ( this, IDC_CHECK_CURE_BURN, COMMENT::BLOW[3].c_str() );
		SetWin_Text ( this, IDC_CHECK_CURE_FROZEN, COMMENT::BLOW[4].c_str() );
		SetWin_Text ( this, IDC_CHECK_CURE_MAD, COMMENT::BLOW[6].c_str() );
		SetWin_Text ( this, IDC_CHECK_CURE_POISON, COMMENT::BLOW[7].c_str() );
		SetWin_Text ( this, IDC_CHECK_CURE_CURSE, COMMENT::BLOW[8].c_str() );
		//trouble cure
		SetWin_Check ( this, IDC_CHECK_CURE_NUMB, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG&DIS_NUMB ) ;
		SetWin_Check ( this, IDC_CHECK_CURE_STUN, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG&DIS_STUN );
		SetWin_Check ( this, IDC_CHECK_CURE_STONE, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG&DIS_STONE );
		SetWin_Check ( this, IDC_CHECK_CURE_BURN, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG&DIS_BURN );
		SetWin_Check ( this, IDC_CHECK_CURE_FROZEN, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG&DIS_FROZEN );
		SetWin_Check ( this, IDC_CHECK_CURE_MAD, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG&DIS_MAD );
		SetWin_Check ( this, IDC_CHECK_CURE_POISON, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG&DIS_POISON );
		SetWin_Check ( this, IDC_CHECK_CURE_CURSE, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG&DIS_CURSE );
	}
	else if ( emSPEC == EMSPECA_DEFENSE_SKILL_ACTIVE )
	{
		//active skill
		SetWin_Num_int ( this, IDC_EDIT_AS_MID, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwNativeID.wMainID );
		SetWin_Num_int ( this, IDC_EDIT_AS_SID, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwNativeID.wSubID );
		SetWin_Num_int ( this, IDC_EDIT_AS_LVL, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG );
		CheckActiveSkill();
	}	
	/*debuff skill logic, Juver, 2017/06/10 */
	else if ( emSPEC == EMSPECA_BUFF_REMOVE )
	{
		SetWin_Combo_Sel( this, IDC_COMBO_DEBUFF_ACTION, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG );
	}
	/*dash skill logic, Juver, 2017/06/17 */
	else if ( emSPEC == EMSPECA_ONWARD )
	{
		SetWin_Combo_Sel( this, IDC_COMBO_ANI_SUB2, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG );
	}
	else if ( emSPEC == EMSPECA_ENHANCEMENT )
	{
		SetWin_Text( this, IDC_CHECK_CURE_NUMB, COMMENT::SKILL_APPLY[0].c_str() );
		SetWin_Text( this, IDC_CHECK_CURE_STUN, COMMENT::SKILL_APPLY[1].c_str() );
		SetWin_Text( this, IDC_CHECK_CURE_STONE, COMMENT::SKILL_APPLY[2].c_str() );

		SetWin_Check ( this, IDC_CHECK_CURE_NUMB, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG & SKILL::EMAPPLY_DWORD_MELEE );
		SetWin_Check ( this, IDC_CHECK_CURE_STUN, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG & SKILL::EMAPPLY_DWORD_RANGE);
		SetWin_Check ( this, IDC_CHECK_CURE_STONE, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG & SKILL::EMAPPLY_DWORD_MAGIC);	
	}
	else if ( emSPEC == EMSPECA_SKILLLINK )
	{
		SetWin_Num_int ( this, IDC_EDIT_AS_MID, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].dwNativeID.wMainID );
		SetWin_Num_int ( this, IDC_EDIT_AS_SID, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].dwNativeID.wSubID );

		SetWin_Num_float( this, IDC_EDIT_SPEC_VAR1, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].fVAR1 );
		SetWin_Num_int ( this, IDC_EDIT_LS_MID, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].dwLinkID.wMainID );
		SetWin_Num_int ( this, IDC_EDIT_LS_SID, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].dwLinkID.wSubID );
	}
	else if ( emSPEC == EMSPECA_POSITION_SHIFT )
	{
		SetWin_Text( this, IDC_CHECK_CURE_NUMB, COMMENT::SPEC_SWAPPOS_DIR_TYPE[SKILL::EMSPEC_SWAPPOS_DIR_TYPE_ALL].c_str() );
		SetWin_Text( this, IDC_CHECK_CURE_STUN, COMMENT::SPEC_SWAPPOS_DIR_TYPE[SKILL::EMSPEC_SWAPPOS_DIR_TYPE_SELF].c_str() );
		SetWin_Text( this, IDC_CHECK_CURE_STONE, COMMENT::SPEC_SWAPPOS_DIR_TYPE[SKILL::EMSPEC_SWAPPOS_DIR_TYPE_TARGET].c_str() );
		int		nResId = IDC_CHECK_CURE_NUMB + min( m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].dwFLAG, SKILL::EMSPEC_SWAPPOS_DIR_TYPE_TARGET );
		SetWin_Check ( this, nResId, true );
	}
	else if ( emSPEC == EMSPECA_SKILLDELAY )
	{
		SetWin_Text( this, IDC_CHECK_CURE_NUMB, COMMENT::SPEC_ASIGN_TYPE[SKILL::EMSPEC_ASIGN_TYPE_BEFORE].c_str() );
		SetWin_Text( this, IDC_CHECK_CURE_STUN, COMMENT::SPEC_ASIGN_TYPE[SKILL::EMSPEC_ASIGN_TYPE_AFTER].c_str() );

		SetWin_Check ( this, IDC_CHECK_CURE_NUMB, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].dwFLAG & SKILL::EMSPEC_SKILL_ASING_DWORD_BEFORE );
		SetWin_Check ( this, IDC_CHECK_CURE_STUN, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].dwFLAG & SKILL::EMSPEC_SKILL_ASING_DWORD_AFTER );
	}
}

void CPageEdit2::SpecDataSave()
{
	if ( !m_pDummySkill )	return;
	if ( m_nLevel < 0 )		return;
	if ( m_nLevel >= SKILL::MAX_LEVEL )	return;

	if ( m_nIndexSpec < 0 )		return;
	if ( m_nIndexSpec >= SKILL::MAX_SPEC )	return;

	//spec
	m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].fVAR1 = GetWin_Num_float( this, IDC_EDIT_SPEC_VAR1 );
	m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].fVAR2 = GetWin_Num_float( this, IDC_EDIT_SPEC_VAR2 );
	m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].fVAR3 = GetWin_Num_float( this, IDC_EDIT_SPEC_VAR3 );
	m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].fVAR4 = GetWin_Num_float( this, IDC_EDIT_SPEC_VAR4 );

	EMSPEC_ADDON emSPEC = m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].emSPEC;

	if ( emSPEC == EMSPECA_NONBLOW || emSPEC == EMSPECA_RECBLOW )
	{
		//trouble cure
		SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_NUMB ), m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG, DIS_NUMB );
		SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_STUN ), m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG, DIS_STUN );
		SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_STONE ), m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG, DIS_STONE );
		SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_BURN ), m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG, DIS_BURN );
		SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_FROZEN ), m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG, DIS_FROZEN );
		SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_MAD ), m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG, DIS_MAD );
		SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_POISON ), m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG, DIS_POISON );
		SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_CURSE ), m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG, DIS_CURSE );
	}
	else if ( emSPEC == EMSPECA_DEFENSE_SKILL_ACTIVE )
	{
		//active skill
		m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwNativeID.wMainID = GetWin_Num_int ( this, IDC_EDIT_AS_MID ); 
		m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwNativeID.wSubID = GetWin_Num_int ( this, IDC_EDIT_AS_SID ); 
		m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG = GetWin_Num_int ( this, IDC_EDIT_AS_LVL ); 
	}
	/*debuff skill logic, Juver, 2017/06/10 */
	else if ( emSPEC == EMSPECA_BUFF_REMOVE )
	{
		m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG = GetWin_Combo_Sel( this, IDC_COMBO_DEBUFF_ACTION );
	}
	/*dash skill logic, Juver, 2017/06/17 */
	else if ( emSPEC == EMSPECA_ONWARD )
	{
		m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG = GetWin_Combo_Sel( this, IDC_COMBO_ANI_SUB2 );
	}
	else if ( emSPEC == EMSPECA_ENHANCEMENT )
	{
		SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_NUMB ), m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG, 
			SKILL::EMAPPLY_DWORD_MELEE );
		SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_STUN ), m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG, 
			SKILL::EMAPPLY_DWORD_RANGE );
		SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_STONE ), m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[ m_nLevel ].dwFLAG, 
			SKILL::EMAPPLY_DWORD_MAGIC );
	}
	else if ( emSPEC == EMSPECA_SKILLLINK )
	{
		SetWin_Num_int ( this, IDC_EDIT_AS_MID, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].dwNativeID.wMainID );
		SetWin_Num_int ( this, IDC_EDIT_AS_SID, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].dwNativeID.wSubID );

		SetWin_Num_float( this, IDC_EDIT_SPEC_VAR1, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].fVAR1 );
		SetWin_Num_int ( this, IDC_EDIT_LS_MID, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].dwLinkID.wMainID );
		SetWin_Num_int ( this, IDC_EDIT_LS_SID, m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].dwLinkID.wSubID );
	}
	else if ( emSPEC == EMSPECA_POSITION_SHIFT )
	{
		int i;
		for( i = 0; i < SKILL::EMSPEC_SWAPPOS_DIR_TYPE_SIZE; i++ )
		{
			if( GetWin_Check( this, IDC_CHECK_CURE_NUMB+i ) )
				break;
		} //for
		m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].dwFLAG = i;
	}
	else if ( emSPEC == EMSPECA_SKILLDELAY )
	{
		SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_NUMB), m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].dwFLAG, 
			SKILL::EMSPEC_SKILL_ASING_DWORD_BEFORE );
		SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_STUN), m_pDummySkill->m_sAPPLY.sSpecs[m_nIndexSpec].sSPEC[m_nLevel].dwFLAG,
			SKILL::EMSPEC_SKILL_ASING_DWORD_AFTER );
	}
}

void CPageEdit2::ImpactDataShow()
{
	if ( !m_pDummySkill )	return;
	if ( m_nLevel < 0 )		return;
	if ( m_nLevel >= SKILL::MAX_LEVEL )	return;

	if ( m_nIndexImpact < 0 )	return;
	if ( m_nIndexImpact >= SKILL::MAX_IMPACT )	return;

	InitImpact();

	SetWin_Num_int( this, IDC_EDIT_IMPACT_NUM2, m_nIndexImpact+1 );

	SetWin_Num_float( this, IDC_EDIT_IMPACT_VAR, m_pDummySkill->m_sAPPLY.sImpacts[m_nIndexImpact].fADDON_VAR[m_nLevel] );
	SetWin_Num_float( this, IDC_EDIT_IMPACT_VAR2, m_pDummySkill->m_sAPPLY.sImpacts[m_nIndexImpact].fADDON_VAR2[m_nLevel] );
}

void CPageEdit2::ImpactDataSave()
{
	if ( !m_pDummySkill )	return;
	if ( m_nLevel < 0 )		return;
	if ( m_nLevel >= SKILL::MAX_LEVEL )	return;

	if ( m_nIndexImpact < 0 )	return;
	if ( m_nIndexImpact >= SKILL::MAX_IMPACT )	return;

	m_pDummySkill->m_sAPPLY.sImpacts[m_nIndexImpact].fADDON_VAR[m_nLevel] = GetWin_Num_float( this, IDC_EDIT_IMPACT_VAR );
	m_pDummySkill->m_sAPPLY.sImpacts[m_nIndexImpact].fADDON_VAR2[m_nLevel] = GetWin_Num_float( this, IDC_EDIT_IMPACT_VAR2 );
}