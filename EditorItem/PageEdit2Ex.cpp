#include "stdafx.h"
#include "EditorItem.h"
#include "PageEdit2.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "GLStringTable.h"
#include "GLItem.h"
#include "GLItemMan.h"
#include "GLItemDef.h"


BOOL CPageEdit2::DataDefault ()
{
	SetWin_Combo_Init( this, IDC_COMBO_SUIT, COMMENT::ITEMSUIT, SUIT_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_ATTACK, COMMENT::ITEMATTACK, ITEMATT_NSIZE );

	SetWin_Combo_Init( this, IDC_COMBO_UPGRADE_TYPE, COMMENT::GRINDING_TYPE, EMGRINDING_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_UPGRADE_LEVEL, COMMENT::GRINDING_LEVEL, EMGRINDING_LEVEL_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_UPGRADE_CLASS, COMMENT::GRINDING_CLASS, EMGRINDING_CLASS_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_ADDON_00, COMMENT::ITEMADDON, EMADD_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_ADDON_01, COMMENT::ITEMADDON, EMADD_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_ADDON_02, COMMENT::ITEMADDON, EMADD_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_ADDON_03, COMMENT::ITEMADDON, EMADD_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_ADDON_04, COMMENT::ITEMADDON, EMADD_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_ITEM_VAR_VAR, COMMENT::ITEMVAR, EMVAR_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_ITEM_VAR_VAR2, COMMENT::ITEMVAR, EMVAR_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_ITEM_VAR_VAR3, COMMENT::ITEMVAR, EMVAR_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_ITEM_VAR_VAR4, COMMENT::ITEMVAR, EMVAR_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_ITEM_VAR_VAR5, COMMENT::ITEMVAR, EMVAR_SIZE );

	SetWin_Combo_Init ( this, IDC_COMBO_ITEM_VAR_VOL, COMMENT::ITEMVOL, EMVAR_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_BLOW_TYPE, COMMENT::BLOW, EMBLOW_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_QUESTION_TYPE, COMMENT::ITEM_QUE_TYPE, QUESTION_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_DRUG_TYPE, COMMENT::ITEMDRUG, ITEM_DRUG_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_RVCARD_TYPE, COMMENT::ITEM_RANDOM_OPT, EMR_OPT_SIZE ); /*rv card, Juver, 2017/11/25 */

	return TRUE;
}

BOOL CPageEdit2::DataShow ()
{
	SetWin_Combo_Sel( this, IDC_COMBO_SUIT, COMMENT::ITEMSUIT[m_pDummyItem->sSuitOp.emSuit] );
	SetWin_Combo_Sel( this, IDC_COMBO_ATTACK, COMMENT::ITEMATTACK[m_pDummyItem->sSuitOp.emAttack] );

	SetWin_Check( this, IDC_CHECK_HAND_TWO, m_pDummyItem->sSuitOp.IsBOTHHAND() );
	SetWin_Check( this, IDC_CHECK_HAND_BIG, m_pDummyItem->sSuitOp.IsBIG() );
	SetWin_Check( this, IDC_CHECK_HAND_BROOM, m_pDummyItem->sSuitOp.IsBROOM() );

	SetWin_Check ( this, IDC_RADIO_HAND_R, m_pDummyItem->sSuitOp.emHand==HAND_RIGHT );
	SetWin_Check ( this, IDC_RADIO_HAND_L, m_pDummyItem->sSuitOp.emHand==HAND_LEFT );
	SetWin_Check ( this, IDC_RADIO_HAND_BOTH, m_pDummyItem->sSuitOp.emHand==HAND_BOTH );

	SetWin_Num_int( this, IDC_EDIT_STAT_DAMAGE_MIN, m_pDummyItem->sSuitOp.gdDamage.dwLow );
	SetWin_Num_int( this, IDC_EDIT_STAT_DAMAGE_MAX, m_pDummyItem->sSuitOp.gdDamage.dwHigh );
	SetWin_Num_int( this, IDC_EDIT_STAT_DEFENSE, m_pDummyItem->sSuitOp.nDefense );
	SetWin_Num_int( this, IDC_EDIT_STAT_HITRATE, m_pDummyItem->sSuitOp.nHitRate );
	SetWin_Num_int( this, IDC_EDIT_STAT_AVOIDRATE, m_pDummyItem->sSuitOp.nAvoidRate);
	SetWin_Num_int( this, IDC_EDIT_STAT_SPUSAGE, m_pDummyItem->sSuitOp.wReqSP );
	SetWin_Num_int( this, IDC_EDIT_STAT_WEAPONRANGE, m_pDummyItem->sSuitOp.wAttRange );
	SetWin_Num_int( this, IDC_EDIT_RES_FIRE, m_pDummyItem->sSuitOp.sResist.nFire );
	SetWin_Num_int( this, IDC_EDIT_RES_ICE, m_pDummyItem->sSuitOp.sResist.nIce );
	SetWin_Num_int( this, IDC_EDIT_RES_ELECT, m_pDummyItem->sSuitOp.sResist.nElectric );
	SetWin_Num_int( this, IDC_EDIT_RES_POISON, m_pDummyItem->sSuitOp.sResist.nPoison );
	SetWin_Num_int( this, IDC_EDIT_RES_SPIRIT, m_pDummyItem->sSuitOp.sResist.nSpirit );

	SetWin_Num_int( this, IDC_EDIT_REMODEL, m_pDummyItem->sSuitOp.wReModelNum );

	SetWin_Num_int( this, IDC_EDIT_GRADE_ATT, m_pDummyItem->sBasicOp.wGradeAttack );
	SetWin_Num_int( this, IDC_EDIT_GRADE_DEF, m_pDummyItem->sBasicOp.wGradeDefense );
	SetWin_Num_float( this, IDC_EDIT_EXPMULTIPLE, m_pDummyItem->sBasicOp.fExpMultiple );

	SetWin_Combo_Sel( this, IDC_COMBO_UPGRADE_TYPE, COMMENT::GRINDING_TYPE[m_pDummyItem->sGrindingOp.emTYPE] );
	SetWin_Combo_Sel( this, IDC_COMBO_UPGRADE_LEVEL, COMMENT::GRINDING_LEVEL[m_pDummyItem->sGrindingOp.emLEVEL] );
	SetWin_Combo_Sel( this, IDC_COMBO_UPGRADE_CLASS, COMMENT::GRINDING_CLASS[m_pDummyItem->sGrindingOp.emCLASS] );
	SetWin_Text( this, IDC_EDIT_GRIND_STRING, m_pDummyItem->sGrindingOp.strGrind.c_str() );
	SetWin_Check ( this, IDC_CHECK_GRIND_NOFAIL, m_pDummyItem->sGrindingOp.bNoFail ); /*no fail upgrade, Juver, 2017/11/26 */

	SetWin_Combo_Sel( this, IDC_COMBO_ADDON_00, COMMENT::ITEMADDON[m_pDummyItem->sSuitOp.sADDON[0].emTYPE] );
	SetWin_Combo_Sel( this, IDC_COMBO_ADDON_01, COMMENT::ITEMADDON[m_pDummyItem->sSuitOp.sADDON[1].emTYPE] );
	SetWin_Combo_Sel( this, IDC_COMBO_ADDON_02, COMMENT::ITEMADDON[m_pDummyItem->sSuitOp.sADDON[2].emTYPE] );
	SetWin_Combo_Sel( this, IDC_COMBO_ADDON_03, COMMENT::ITEMADDON[m_pDummyItem->sSuitOp.sADDON[3].emTYPE] );
	SetWin_Combo_Sel( this, IDC_COMBO_ADDON_04, COMMENT::ITEMADDON[m_pDummyItem->sSuitOp.sADDON[4].emTYPE] );
	SetWin_Num_int( this, IDC_EDIT_ADDON_00, m_pDummyItem->sSuitOp.sADDON[0].nVALUE );
	SetWin_Num_int( this, IDC_EDIT_ADDON_01, m_pDummyItem->sSuitOp.sADDON[1].nVALUE );
	SetWin_Num_int( this, IDC_EDIT_ADDON_02, m_pDummyItem->sSuitOp.sADDON[2].nVALUE );
	SetWin_Num_int( this, IDC_EDIT_ADDON_03, m_pDummyItem->sSuitOp.sADDON[3].nVALUE );
	SetWin_Num_int( this, IDC_EDIT_ADDON_04, m_pDummyItem->sSuitOp.sADDON[4].nVALUE );

	SetWin_Combo_Sel( this, IDC_COMBO_ITEM_VAR_VAR, COMMENT::ITEMVAR[m_pDummyItem->sSuitOp.sVARIATE[0].emTYPE] );
	SetWin_Num_float( this, IDC_EDIT_VALUE_VAR, m_pDummyItem->sSuitOp.sVARIATE[0].fVariate );

	SetWin_Combo_Sel( this, IDC_COMBO_ITEM_VAR_VAR2, COMMENT::ITEMVAR[m_pDummyItem->sSuitOp.sVARIATE[1].emTYPE] );
	SetWin_Num_float( this, IDC_EDIT_VALUE_VAR2, m_pDummyItem->sSuitOp.sVARIATE[1].fVariate );

	SetWin_Combo_Sel( this, IDC_COMBO_ITEM_VAR_VAR3, COMMENT::ITEMVAR[m_pDummyItem->sSuitOp.sVARIATE[2].emTYPE] );
	SetWin_Num_float( this, IDC_EDIT_VALUE_VAR3, m_pDummyItem->sSuitOp.sVARIATE[2].fVariate );

	SetWin_Combo_Sel( this, IDC_COMBO_ITEM_VAR_VAR4, COMMENT::ITEMVAR[m_pDummyItem->sSuitOp.sVARIATE[3].emTYPE] );
	SetWin_Num_float( this, IDC_EDIT_VALUE_VAR4, m_pDummyItem->sSuitOp.sVARIATE[3].fVariate );

	SetWin_Combo_Sel( this, IDC_COMBO_ITEM_VAR_VAR5, COMMENT::ITEMVAR[m_pDummyItem->sSuitOp.sVARIATE[4].emTYPE] );
	SetWin_Num_float( this, IDC_EDIT_VALUE_VAR5, m_pDummyItem->sSuitOp.sVARIATE[4].fVariate );

	SetWin_Combo_Sel( this, IDC_COMBO_ITEM_VAR_VOL, COMMENT::ITEMVOL[m_pDummyItem->sSuitOp.sVOLUME.emTYPE] );
	SetWin_Num_float( this, IDC_EDIT_VALUE_VOL, m_pDummyItem->sSuitOp.sVOLUME.fVolume );

	SetWin_Combo_Sel( this, IDC_COMBO_BLOW_TYPE, COMMENT::BLOW[m_pDummyItem->sSuitOp.sBLOW.emTYPE] );
	SetWin_Num_float( this, IDC_EDIT_BLOW_TIME, m_pDummyItem->sSuitOp.sBLOW.fLIFE );
	SetWin_Num_float( this, IDC_EDIT_BLOW_CHANCE, m_pDummyItem->sSuitOp.sBLOW.fRATE );
	SetWin_Num_float( this, IDC_EDIT_BLOW_VAR1, m_pDummyItem->sSuitOp.sBLOW.fVAR1 );
	SetWin_Num_float( this, IDC_EDIT_BLOW_VAR2, m_pDummyItem->sSuitOp.sBLOW.fVAR2 );

	SetWin_Combo_Sel( this, IDC_COMBO_QUESTION_TYPE, COMMENT::ITEM_QUE_TYPE[m_pDummyItem->sQuestionItem.emType] );
	SetWin_Num_float( this, IDC_EDIT_QUESTION_TIME, m_pDummyItem->sQuestionItem.fTime );
	SetWin_Num_float( this, IDC_EDIT_QUESTION_VAR1, m_pDummyItem->sQuestionItem.fExp );
	SetWin_Num_int( this, IDC_EDIT_QUESTION_VAR2, m_pDummyItem->sQuestionItem.wParam1 );
	SetWin_Num_int( this, IDC_EDIT_QUESTION_VAR3, m_pDummyItem->sQuestionItem.wParam2 );

	SetWin_Combo_Sel( this, IDC_COMBO_DRUG_TYPE, COMMENT::ITEMDRUG[m_pDummyItem->sDrugOp.emDrug] );
	SetWin_Num_int( this, IDC_EDIT_DRUG_PILE, m_pDummyItem->sDrugOp.wPileNum );
	SetWin_Num_int( this, IDC_EDIT_DRUG_VOLUME, m_pDummyItem->sDrugOp.wCureVolume );
	SetWin_Check ( this, IDC_CHECK_DRUG_RATIO, m_pDummyItem->sDrugOp.bRatio );

	SetWin_Check ( this, IDC_CHECK_CURE_NUMB, m_pDummyItem->sDrugOp.dwCureDISORDER&DIS_NUMB );
	SetWin_Check ( this, IDC_CHECK_CURE_STUN, m_pDummyItem->sDrugOp.dwCureDISORDER&DIS_STUN );
	SetWin_Check ( this, IDC_CHECK_CURE_STONE, m_pDummyItem->sDrugOp.dwCureDISORDER&DIS_STONE );
	SetWin_Check ( this, IDC_CHECK_CURE_BURN, m_pDummyItem->sDrugOp.dwCureDISORDER&DIS_BURN );
	SetWin_Check ( this, IDC_CHECK_CURE_FROZEN, m_pDummyItem->sDrugOp.dwCureDISORDER&DIS_FROZEN );
	SetWin_Check ( this, IDC_CHECK_CURE_MAD, m_pDummyItem->sDrugOp.dwCureDISORDER&DIS_MAD );
	SetWin_Check ( this, IDC_CHECK_CURE_POISON, m_pDummyItem->sDrugOp.dwCureDISORDER&DIS_POISON );
	SetWin_Check ( this, IDC_CHECK_CURE_CURSE, m_pDummyItem->sDrugOp.dwCureDISORDER&DIS_CURSE );

	SetWin_Num_int( this, IDC_EDIT_PS_TIME, m_pDummyItem->sPetSkinPack.dwPetSkinTime );

	/*rv card, Juver, 2017/11/25 */
	SetWin_Combo_Sel( this, IDC_COMBO_RVCARD_TYPE, COMMENT::ITEM_RANDOM_OPT[m_pDummyItem->sRvCard.emOption] );
	SetWin_Num_int( this, IDC_EDIT_RVCARD_VALUE, m_pDummyItem->sRvCard.wValue );
	SetWin_Check( this, IDC_CHECK_RVCARD_CHECK_EXIST, m_pDummyItem->sRvCard.bCheckExist );
	SetWin_Check( this, IDC_CHECK_RVCARD_REPLACE_VALUE, m_pDummyItem->sRvCard.bReplaceOpt );
	InitRVCard();
	
	//Select Box
	SetWin_Check ( this, IDC_CHECK_SELECTBOX_CHECK, m_pDummyItem->sBox.m_bSelect );
	SetWin_Num_int ( this, IDC_EDIT_BOX_MAXSELECT, m_pDummyItem->sBox.m_nMaxSelect );

	InitBlow();
	InitQuestion();
	InitDrug();
	InitBox();
	InitBoxR();
	InitPetSkin();

	/*item wrapper, Juver, 2018/01/11 */
	SetWin_Num_int( this, IDC_EDIT_WRAPPER_BOX_MID, m_pDummyItem->sBasicOp.sidWrapperBox.wMainID );
	SetWin_Num_int( this, IDC_EDIT_WRAPPER_BOX_SID, m_pDummyItem->sBasicOp.sidWrapperBox.wSubID );
	
	return TRUE;
}

BOOL CPageEdit2::DataSave()
{
	m_pDummyItem->sSuitOp.emSuit = static_cast<EMSUIT> ( GetWin_Combo_Sel( this, IDC_COMBO_SUIT ) );
	m_pDummyItem->sSuitOp.emAttack = static_cast<GLITEM_ATT> ( GetWin_Combo_Sel( this, IDC_COMBO_ATTACK ) );

	GetWin_Check ( this, IDC_CHECK_HAND_TWO, m_pDummyItem->sSuitOp.dwHAND, ITEM::EMHAND_BOTHHAND );
	GetWin_Check ( this, IDC_CHECK_HAND_BIG, m_pDummyItem->sSuitOp.dwHAND, ITEM::EMHAND_BIG );
	GetWin_Check ( this, IDC_CHECK_HAND_BROOM, m_pDummyItem->sSuitOp.dwHAND, ITEM::EMHAND_BROOM );

	if ( GetWin_Check ( this, IDC_RADIO_HAND_R ) )	m_pDummyItem->sSuitOp.emHand = HAND_RIGHT;
	if ( GetWin_Check ( this, IDC_RADIO_HAND_L ) )	m_pDummyItem->sSuitOp.emHand = HAND_LEFT;
	if ( GetWin_Check ( this, IDC_RADIO_HAND_BOTH ) )	m_pDummyItem->sSuitOp.emHand = HAND_BOTH;

	m_pDummyItem->sSuitOp.gdDamage.dwLow = GetWin_Num_int( this, IDC_EDIT_STAT_DAMAGE_MIN );
	m_pDummyItem->sSuitOp.gdDamage.dwHigh = GetWin_Num_int( this, IDC_EDIT_STAT_DAMAGE_MAX );

	m_pDummyItem->sSuitOp.nDefense = GetWin_Num_int( this, IDC_EDIT_STAT_DEFENSE );
	m_pDummyItem->sSuitOp.nHitRate = GetWin_Num_int( this, IDC_EDIT_STAT_HITRATE );
	m_pDummyItem->sSuitOp.nAvoidRate = GetWin_Num_int( this, IDC_EDIT_STAT_AVOIDRATE );
	m_pDummyItem->sSuitOp.wReqSP = GetWin_Num_int( this, IDC_EDIT_STAT_SPUSAGE );
	m_pDummyItem->sSuitOp.wAttRange = GetWin_Num_int( this, IDC_EDIT_STAT_WEAPONRANGE );
	m_pDummyItem->sSuitOp.sResist.nFire = GetWin_Num_int( this, IDC_EDIT_RES_FIRE );
	m_pDummyItem->sSuitOp.sResist.nIce = GetWin_Num_int( this, IDC_EDIT_RES_ICE );
	m_pDummyItem->sSuitOp.sResist.nElectric = GetWin_Num_int( this, IDC_EDIT_RES_ELECT );
	m_pDummyItem->sSuitOp.sResist.nPoison = GetWin_Num_int( this, IDC_EDIT_RES_POISON );
	m_pDummyItem->sSuitOp.sResist.nSpirit = GetWin_Num_int( this, IDC_EDIT_RES_SPIRIT );

	m_pDummyItem->sSuitOp.wReModelNum = GetWin_Num_int( this, IDC_EDIT_REMODEL );

	m_pDummyItem->sBasicOp.wGradeAttack = GetWin_Num_int( this, IDC_EDIT_GRADE_ATT );
	m_pDummyItem->sBasicOp.wGradeDefense = GetWin_Num_int( this, IDC_EDIT_GRADE_DEF );
	m_pDummyItem->sBasicOp.fExpMultiple = GetWin_Num_float( this, IDC_EDIT_EXPMULTIPLE );

	m_pDummyItem->sGrindingOp.emTYPE = static_cast<EMGRINDING_TYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_UPGRADE_TYPE ) );
	m_pDummyItem->sGrindingOp.emLEVEL = static_cast<EMGRINDING_LEVEL> ( GetWin_Combo_Sel( this, IDC_COMBO_UPGRADE_LEVEL ) );
	m_pDummyItem->sGrindingOp.emCLASS = static_cast<EMGRINDING_CLASS> ( GetWin_Combo_Sel( this, IDC_COMBO_UPGRADE_CLASS ) );
	m_pDummyItem->sGrindingOp.strGrind = GetWin_Text( this, IDC_EDIT_GRIND_STRING ).GetString();
	m_pDummyItem->sGrindingOp.bNoFail = GetWin_Check ( this, IDC_CHECK_GRIND_NOFAIL ); /*no fail upgrade, Juver, 2017/11/26 */

	m_pDummyItem->sSuitOp.sADDON[0].emTYPE = static_cast<EMITEM_ADDON> ( GetWin_Combo_Sel( this, IDC_COMBO_ADDON_00 ) );
	m_pDummyItem->sSuitOp.sADDON[1].emTYPE = static_cast<EMITEM_ADDON> ( GetWin_Combo_Sel( this, IDC_COMBO_ADDON_01 ) );
	m_pDummyItem->sSuitOp.sADDON[2].emTYPE = static_cast<EMITEM_ADDON> ( GetWin_Combo_Sel( this, IDC_COMBO_ADDON_02 ) );
	m_pDummyItem->sSuitOp.sADDON[3].emTYPE = static_cast<EMITEM_ADDON> ( GetWin_Combo_Sel( this, IDC_COMBO_ADDON_03 ) );
	m_pDummyItem->sSuitOp.sADDON[4].emTYPE = static_cast<EMITEM_ADDON> ( GetWin_Combo_Sel( this, IDC_COMBO_ADDON_04 ) );
	m_pDummyItem->sSuitOp.sADDON[0].nVALUE = GetWin_Num_int( this, IDC_EDIT_ADDON_00 );
	m_pDummyItem->sSuitOp.sADDON[1].nVALUE = GetWin_Num_int( this, IDC_EDIT_ADDON_01 );
	m_pDummyItem->sSuitOp.sADDON[2].nVALUE = GetWin_Num_int( this, IDC_EDIT_ADDON_02 );
	m_pDummyItem->sSuitOp.sADDON[3].nVALUE = GetWin_Num_int( this, IDC_EDIT_ADDON_03 );
	m_pDummyItem->sSuitOp.sADDON[4].nVALUE = GetWin_Num_int( this, IDC_EDIT_ADDON_04 );

	m_pDummyItem->sSuitOp.sVARIATE[0].emTYPE = static_cast<EMITEM_VAR> ( GetWin_Combo_Sel( this, IDC_COMBO_ITEM_VAR_VAR ) );
	m_pDummyItem->sSuitOp.sVARIATE[0].fVariate = GetWin_Num_float( this, IDC_EDIT_VALUE_VAR );

	m_pDummyItem->sSuitOp.sVARIATE[1].emTYPE = static_cast<EMITEM_VAR> ( GetWin_Combo_Sel( this, IDC_COMBO_ITEM_VAR_VAR2 ) );
	m_pDummyItem->sSuitOp.sVARIATE[1].fVariate = GetWin_Num_float( this, IDC_EDIT_VALUE_VAR2 );

	m_pDummyItem->sSuitOp.sVARIATE[2].emTYPE = static_cast<EMITEM_VAR> ( GetWin_Combo_Sel( this, IDC_COMBO_ITEM_VAR_VAR3 ) );
	m_pDummyItem->sSuitOp.sVARIATE[2].fVariate = GetWin_Num_float( this, IDC_EDIT_VALUE_VAR3 );

	m_pDummyItem->sSuitOp.sVARIATE[3].emTYPE = static_cast<EMITEM_VAR> ( GetWin_Combo_Sel( this, IDC_COMBO_ITEM_VAR_VAR4 ) );
	m_pDummyItem->sSuitOp.sVARIATE[3].fVariate = GetWin_Num_float( this, IDC_EDIT_VALUE_VAR4 );

	m_pDummyItem->sSuitOp.sVARIATE[4].emTYPE = static_cast<EMITEM_VAR> ( GetWin_Combo_Sel( this, IDC_COMBO_ITEM_VAR_VAR5 ) );
	m_pDummyItem->sSuitOp.sVARIATE[4].fVariate = GetWin_Num_float( this, IDC_EDIT_VALUE_VAR5 );

	m_pDummyItem->sSuitOp.sVOLUME.emTYPE = static_cast<EMITEM_VAR> ( GetWin_Combo_Sel( this, IDC_COMBO_ITEM_VAR_VOL ) );
	m_pDummyItem->sSuitOp.sVOLUME.fVolume = GetWin_Num_float( this, IDC_EDIT_VALUE_VOL );

	m_pDummyItem->sSuitOp.sBLOW.emTYPE = static_cast<EMSTATE_BLOW> ( GetWin_Combo_Sel( this, IDC_COMBO_BLOW_TYPE ) );
	m_pDummyItem->sSuitOp.sBLOW.fLIFE = GetWin_Num_float( this, IDC_EDIT_BLOW_TIME );
	m_pDummyItem->sSuitOp.sBLOW.fRATE = GetWin_Num_float( this, IDC_EDIT_BLOW_CHANCE );
	m_pDummyItem->sSuitOp.sBLOW.fVAR1 = GetWin_Num_float( this, IDC_EDIT_BLOW_VAR1 );
	m_pDummyItem->sSuitOp.sBLOW.fVAR2 = GetWin_Num_float( this, IDC_EDIT_BLOW_VAR2 );

	m_pDummyItem->sQuestionItem.emType = static_cast<EMITEM_QUESTION> ( GetWin_Combo_Sel( this, IDC_COMBO_QUESTION_TYPE ) );
	m_pDummyItem->sQuestionItem.fTime = GetWin_Num_float( this, IDC_EDIT_QUESTION_TIME );
	m_pDummyItem->sQuestionItem.fExp = GetWin_Num_float( this, IDC_EDIT_QUESTION_VAR1 );
	m_pDummyItem->sQuestionItem.wParam1 = GetWin_Num_int( this, IDC_EDIT_QUESTION_VAR2 );
	m_pDummyItem->sQuestionItem.wParam2 = GetWin_Num_int( this, IDC_EDIT_QUESTION_VAR3 );

	m_pDummyItem->sDrugOp.emDrug = static_cast <EMITEM_DRUG> ( GetWin_Combo_Sel( this, IDC_COMBO_DRUG_TYPE ) );
	m_pDummyItem->sDrugOp.wPileNum = GetWin_Num_int( this, IDC_EDIT_DRUG_PILE );
	m_pDummyItem->sDrugOp.wCureVolume = GetWin_Num_int( this, IDC_EDIT_DRUG_VOLUME );
	m_pDummyItem->sDrugOp.bRatio = GetWin_Check ( this, IDC_CHECK_DRUG_RATIO );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_NUMB ), m_pDummyItem->sDrugOp.dwCureDISORDER, DIS_NUMB );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_STUN ), m_pDummyItem->sDrugOp.dwCureDISORDER, DIS_STUN );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_STONE ), m_pDummyItem->sDrugOp.dwCureDISORDER, DIS_STONE );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_BURN ), m_pDummyItem->sDrugOp.dwCureDISORDER, DIS_BURN );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_FROZEN ), m_pDummyItem->sDrugOp.dwCureDISORDER, DIS_FROZEN );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_MAD ), m_pDummyItem->sDrugOp.dwCureDISORDER, DIS_MAD );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_POISON ), m_pDummyItem->sDrugOp.dwCureDISORDER, DIS_POISON );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CURE_CURSE ), m_pDummyItem->sDrugOp.dwCureDISORDER, DIS_CURSE );

	m_pDummyItem->sPetSkinPack.dwPetSkinTime = GetWin_Num_int( this, IDC_EDIT_PS_TIME );

	/*rv card, Juver, 2017/11/25 */
	m_pDummyItem->sRvCard.emOption = static_cast<EMRANDOM_OPT> ( GetWin_Combo_Sel( this, IDC_COMBO_RVCARD_TYPE) );
	m_pDummyItem->sRvCard.wValue = (WORD) GetWin_Num_int( this, IDC_EDIT_RVCARD_VALUE );
	m_pDummyItem->sRvCard.bCheckExist = GetWin_Check( this, IDC_CHECK_RVCARD_CHECK_EXIST );
	m_pDummyItem->sRvCard.bReplaceOpt = GetWin_Check( this, IDC_CHECK_RVCARD_REPLACE_VALUE );

	//Select Box
	m_pDummyItem->sBox.m_bSelect = GetWin_Check ( this, IDC_CHECK_SELECTBOX_CHECK );
	m_pDummyItem->sBox.m_nMaxSelect = GetWin_Num_int ( this, IDC_EDIT_BOX_MAXSELECT );


	if ( m_pDummyItem->sSuitOp.emSuit == SUIT_PET_A )
	{
		 m_pDummyItem->sPet.emType = PET_ACCETYPE_A;
	}
	else if (  m_pDummyItem->sSuitOp.emSuit == SUIT_PET_B )
	{
		m_pDummyItem->sPet.emType = PET_ACCETYPE_B;
	}

	/*item wrapper, Juver, 2018/01/11 */
	m_pDummyItem->sBasicOp.sidWrapperBox.wMainID = GetWin_Num_int( this, IDC_EDIT_WRAPPER_BOX_MID );
	m_pDummyItem->sBasicOp.sidWrapperBox.wSubID = GetWin_Num_int( this, IDC_EDIT_WRAPPER_BOX_SID );

	return TRUE;
}

