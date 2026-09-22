#include "stdafx.h"
#include "EditorMobNpc.h"
#include "PageEdit2.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"

BOOL CPageEdit2::DataDefault()
{
	SetWin_Combo_Init( this, IDC_COMBO_PATTERN_ACTION, COMMENT::szCROWACT_UP, EMCROWACT_UP_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_ATTACK_TYPE, COMMENT::ATTACK_RGTYPE, EMATT_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_ATTACK_SKILL_TARGET, COMMENT::CROWSKTAR, CROWSKTAR_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_BLOW_TYPE, COMMENT::BLOW, EMBLOW_SIZE );

	m_nAttack = 0;

	return TRUE;
}

BOOL CPageEdit2::DataShow()
{
	if ( !m_pDummyCrow )	return FALSE;

	InitPattern();
	InitAniList();
	AttackShow( m_nAttack );

	return TRUE;
}

BOOL CPageEdit2::DataSave()
{
	if ( !m_pDummyCrow )	return FALSE;

	AttackSave( m_nAttack );

	if ( m_pDummyCrow->m_sAction.m_vecPatternList.size() <= 0 )
	{
		if ( MessageBox ( "Pattern Empty! Require 1 Remaining Pattern!, Create New Pattern?", "WARNING", MB_YESNO ) == IDYES )
		{
			SCROWPATTERN sPattern;
			sPattern.m_fPatternDNRate = 90.0f;
			sPattern.m_emActPattern = EMCROWACT_UP_FIRSTSTRIKE;
			sPattern.m_dwPatternAttackSet = SCROWDATA::EMMAXATTACK;
			m_pDummyCrow->m_sAction.m_vecPatternList.push_back( sPattern );
			InitPattern();
		}else
			return FALSE;
	}

	return TRUE;
}

void CPageEdit2::AttackShow( int nATTACK )
{
	if ( nATTACK < 0 )	return;
	if ( nATTACK >= SCROWDATA::EMMAXATTACK )	return;
	if ( !m_pDummyCrow )	return;

	CString strNUM;
	strNUM.Format( "Attack Number:%d", nATTACK );
	SetWin_Text( this, IDC_EDIT_ATTACK, strNUM.GetString() );

	SCROWATTACK sATTACK = m_pDummyCrow->m_sCrowAttack[nATTACK];

	SetWin_Check( this, IDC_CHECK_ATTACK_USE, sATTACK.bUsed );
	SetWin_Combo_Sel( this, IDC_COMBO_ATTACK_TYPE, COMMENT::ATTACK_RGTYPE[sATTACK.emAttRgType].c_str() );
	SetWin_Num_int( this, IDC_EDIT_ATTACK_RANGE, sATTACK.wRange );
	SetWin_Num_int( this, IDC_EDIT_ATTACK_DAMAGELOW, sATTACK.sDamage.wLow );
	SetWin_Num_int( this, IDC_EDIT_ATTACK_DAMAGEHIGH, sATTACK.sDamage.wHigh );
	SetWin_Num_float( this, IDC_EDIT_ATTACK_DELAY, sATTACK.fDelay );
	SetWin_Num_int( this, IDC_EDIT_ATTACK_SPUSAGE, sATTACK.wUse_SP );

	SetWin_Num_int( this, IDC_EDIT_ATTACK_SKILL_MID, sATTACK.skill_id.wMainID );
	SetWin_Num_int( this, IDC_EDIT_ATTACK_SKILL_SID, sATTACK.skill_id.wSubID );
	SetWin_Num_int( this, IDC_EDIT_ATTACK_SKILL_LEVEL, sATTACK.skill_lev );
	SetWin_Combo_Sel( this, IDC_COMBO_ATTACK_SKILL_TARGET, COMMENT::CROWSKTAR[sATTACK.skill_tar].c_str() );

	SetWin_Combo_Sel( this, IDC_COMBO_BLOW_TYPE, COMMENT::BLOW[sATTACK.emBLOW_TYPE].c_str() );
	SetWin_Num_float( this, IDC_EDIT_BLOW_TIME, sATTACK.fBLOW_LIFE );
	SetWin_Num_float( this, IDC_EDIT_BLOW_CHANCE, sATTACK.fBLOW_RATE );
	SetWin_Num_float( this, IDC_EDIT_BLOW_VAR1, sATTACK.fBLOW_VAR1 );
	SetWin_Num_float( this, IDC_EDIT_BLOW_VAR2, sATTACK.fBLOW_VAR2 );

	SetWin_Text( this, IDC_EDIT_ATTACK_SELFBODY, sATTACK.strSelfBodyEffect.c_str() );
	SetWin_Text( this, IDC_EDIT_ATTACK_TARGETBODY, sATTACK.strTargBodyEffect.c_str() );
	SetWin_Text( this, IDC_EDIT_ATTACK_TARGET, sATTACK.strTargetEffect.c_str() );

	SetWin_Text( this, IDC_EDIT_ANI_NAME, sATTACK.strAniFile.c_str() );

	sAniAttack = SANIATTACK();
	sAniAttack = sATTACK.sAniAttack;

	InitBlow();
	InitSkill();
	InitAniInfo();
}

void CPageEdit2::AttackSave( int nATTACK )
{
	if ( nATTACK < 0 )	return;
	if ( nATTACK >= SCROWDATA::EMMAXATTACK )	return;
	if ( !m_pDummyCrow )	return;

	m_pDummyCrow->m_sCrowAttack[nATTACK].bUsed = GetWin_Check( this, IDC_CHECK_ATTACK_USE );
	m_pDummyCrow->m_sCrowAttack[nATTACK].emAttRgType = static_cast<EMATT_RGTYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_ATTACK_TYPE ) );
	m_pDummyCrow->m_sCrowAttack[nATTACK].wRange = GetWin_Num_int( this, IDC_EDIT_ATTACK_RANGE );
	m_pDummyCrow->m_sCrowAttack[nATTACK].sDamage.wLow = GetWin_Num_int( this, IDC_EDIT_ATTACK_DAMAGELOW );
	m_pDummyCrow->m_sCrowAttack[nATTACK].sDamage.wHigh = GetWin_Num_int( this, IDC_EDIT_ATTACK_DAMAGEHIGH );
	m_pDummyCrow->m_sCrowAttack[nATTACK].fDelay = GetWin_Num_float( this, IDC_EDIT_ATTACK_DELAY );
	m_pDummyCrow->m_sCrowAttack[nATTACK].wUse_SP = GetWin_Num_int( this, IDC_EDIT_ATTACK_SPUSAGE );

	m_pDummyCrow->m_sCrowAttack[nATTACK].skill_id.wMainID = GetWin_Num_int( this, IDC_EDIT_ATTACK_SKILL_MID );
	m_pDummyCrow->m_sCrowAttack[nATTACK].skill_id.wSubID = GetWin_Num_int( this, IDC_EDIT_ATTACK_SKILL_SID );
	m_pDummyCrow->m_sCrowAttack[nATTACK].skill_lev = GetWin_Num_int( this, IDC_EDIT_ATTACK_SKILL_LEVEL );
	m_pDummyCrow->m_sCrowAttack[nATTACK].skill_tar = static_cast<EMCROWSKTAR> ( GetWin_Combo_Sel( this, IDC_COMBO_ATTACK_SKILL_TARGET ) );

	m_pDummyCrow->m_sCrowAttack[nATTACK].emBLOW_TYPE = static_cast<EMSTATE_BLOW> ( GetWin_Combo_Sel( this, IDC_COMBO_BLOW_TYPE ) );
	m_pDummyCrow->m_sCrowAttack[nATTACK].fBLOW_LIFE = GetWin_Num_float( this, IDC_EDIT_BLOW_TIME );
	m_pDummyCrow->m_sCrowAttack[nATTACK].fBLOW_RATE = GetWin_Num_float( this, IDC_EDIT_BLOW_CHANCE );
	m_pDummyCrow->m_sCrowAttack[nATTACK].fBLOW_VAR1 = GetWin_Num_float( this, IDC_EDIT_BLOW_VAR1 );
	m_pDummyCrow->m_sCrowAttack[nATTACK].fBLOW_VAR2 = GetWin_Num_float( this, IDC_EDIT_BLOW_VAR2 );

	m_pDummyCrow->m_sCrowAttack[nATTACK].strSelfBodyEffect = GetWin_Text( this, IDC_EDIT_ATTACK_SELFBODY ).GetString();
	m_pDummyCrow->m_sCrowAttack[nATTACK].strTargBodyEffect = GetWin_Text( this, IDC_EDIT_ATTACK_TARGETBODY ).GetString();
	m_pDummyCrow->m_sCrowAttack[nATTACK].strTargetEffect = GetWin_Text( this, IDC_EDIT_ATTACK_TARGET ).GetString();

	m_pDummyCrow->m_sCrowAttack[nATTACK].strAniFile = GetWin_Text( this, IDC_EDIT_ANI_NAME ).GetString();
	m_pDummyCrow->m_sCrowAttack[nATTACK].sAniAttack = sAniAttack;
}