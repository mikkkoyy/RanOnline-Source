#include "stdafx.h"
#include "EditorMobNpc.h"
#include "PageEdit1.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "GLPet.h"

BOOL CPageEdit1::DataDefault()
{
	if ( !m_pDummyCrow )	return FALSE;

	SetWin_Combo_Init( this, IDC_COMBO_TRIBE, COMMENT::TRIBE, TRIBE_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_CROW, COMMENT::CROW, CROW_NUM );
	SetWin_Combo_Init( this, IDC_COMBO_BRIGHT, COMMENT::BRIGHT, BRIGHT_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_PET, COMMENT::PET_TYPE, PET_TYPE_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_SUMMON, COMMENT::SUMMON_TYPE, SUMMON_TYPE_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_GATHER_ANI, COMMENT::ANI_SUBTYPE_SKILL, AN_SUB_00_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_NPCTYPE, COMMENT::NPC_TYPE, EMNPC_TYPE_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_CTFTYPE, COMMENT::CTF_TYPE, EMNPC_CTF_FLAG_TYPE_SIZE ); /*pvp capture the flag, Juver, 2018/02/03 */

	return TRUE;
}

BOOL CPageEdit1::DataShow()
{
	if ( !m_pDummyCrow )	return FALSE;

	//basic
	SetWin_Num_int( this, IDC_EDIT_MID, m_pDummyCrow->m_sBasic.sNativeID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_SID, m_pDummyCrow->m_sBasic.sNativeID.wSubID );
	SetWin_Text( this, IDC_EDIT_NAME, m_pDummyCrow->m_sBasic.m_szName );
	SetWin_Text( this, IDC_EDIT_NAME2, m_pDummyCrow->GetName() );

	SetWin_Combo_Sel( this, IDC_COMBO_TRIBE, COMMENT::TRIBE[m_pDummyCrow->m_sBasic.m_emTribe].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_CROW, COMMENT::CROW[m_pDummyCrow->m_sBasic.m_emCrow].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_BRIGHT, COMMENT::BRIGHT[m_pDummyCrow->m_sBasic.m_emBright].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_PET, COMMENT::PET_TYPE[m_pDummyCrow->m_sBasic.m_emPetType].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_SUMMON, COMMENT::SUMMON_TYPE[m_pDummyCrow->m_sBasic.m_emSummonType].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_NPCTYPE, COMMENT::NPC_TYPE[m_pDummyCrow->m_sBasic.m_emNPCType].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_CTFTYPE, COMMENT::CTF_TYPE[m_pDummyCrow->m_sBasic.m_emCTFType].c_str() ); /*pvp capture the flag, Juver, 2018/02/03 */

	SetWin_Num_int( this, IDC_EDIT_HP, m_pDummyCrow->m_sBasic.m_dwHP );
	SetWin_Num_int( this, IDC_EDIT_MP, m_pDummyCrow->m_sBasic.m_wMP );
	SetWin_Num_int( this, IDC_EDIT_SP, m_pDummyCrow->m_sBasic.m_wSP );
	SetWin_Num_float( this, IDC_EDIT_HP_REC, m_pDummyCrow->m_sBasic.m_fIncHP );
	SetWin_Num_float( this, IDC_EDIT_MP_REC, m_pDummyCrow->m_sBasic.m_fIncMP );
	SetWin_Num_float( this, IDC_EDIT_SP_REC, m_pDummyCrow->m_sBasic.m_fIncSP );
	SetWin_Num_int( this, IDC_EDIT_LEVEL, m_pDummyCrow->m_sBasic.m_wLevel );
	SetWin_Num_int( this, IDC_EDIT_DEF, m_pDummyCrow->m_sBasic.m_wDefense );
	SetWin_Num_int( this, IDC_EDIT_AVOIDRATE, m_pDummyCrow->m_sBasic.m_wAvoidRate );
	SetWin_Num_int( this, IDC_EDIT_HITRATE, m_pDummyCrow->m_sBasic.m_wHitRate );
	SetWin_Num_int( this, IDC_EDIT_VIEWDISTANCE, m_pDummyCrow->m_sBasic.m_wViewRange );
	SetWin_Num_int( this, IDC_EDIT_GENTIME, m_pDummyCrow->m_sBasic.m_dwGenTime );
	SetWin_Num_int( this, IDC_EDIT_ADDEXP, m_pDummyCrow->m_sBasic.m_dwBonusExp );
	SetWin_Num_int( this, IDC_EDIT_ADDCP, m_pDummyCrow->m_sBasic.m_wBonusCP );
	SetWin_Num_int( this, IDC_EDIT_RES_FIRE, m_pDummyCrow->m_sBasic.m_sResist.nFire );
	SetWin_Num_int( this, IDC_EDIT_RES_ICE, m_pDummyCrow->m_sBasic.m_sResist.nIce );
	SetWin_Num_int( this, IDC_EDIT_RES_ELECT, m_pDummyCrow->m_sBasic.m_sResist.nElectric );
	SetWin_Num_int( this, IDC_EDIT_RES_POI, m_pDummyCrow->m_sBasic.m_sResist.nPoison );
	SetWin_Num_int( this, IDC_EDIT_RES_SPIRIT, m_pDummyCrow->m_sBasic.m_sResist.nSpirit );

	SetWin_Num_int( this, IDC_EDIT_GATHER_TIME_LOW, m_pDummyCrow->m_sBasic.m_wGatherTimeLow );
	SetWin_Num_int( this, IDC_EDIT_GATHER_TIME_HIGH, m_pDummyCrow->m_sBasic.m_wGatherTimeHigh );
	SetWin_Num_float( this, IDC_EDIT_GATHER_RATE, m_pDummyCrow->m_sBasic.m_fGatherRate );
	SetWin_Combo_Sel( this, IDC_COMBO_GATHER_ANI, COMMENT::ANI_SUBTYPE_SKILL[m_pDummyCrow->m_wGatherAnimation].c_str() );

	//action
	SetWin_Text( this, IDC_EDIT_SKINFILE, m_pDummyCrow->m_sAction.m_strSkinObj.c_str() );
	SetWin_Text( this, IDC_EDIT_TALKFILE, m_pDummyCrow->m_sAction.m_strTalkFile.c_str() );
	SetWin_Text( this, IDC_EDIT_SALEFILE_00, m_pDummyCrow->m_sAction.m_strSaleFile[0].c_str() );
	SetWin_Text( this, IDC_EDIT_SALEFILE_01, m_pDummyCrow->m_sAction.m_strSaleFile[1].c_str() );
	SetWin_Text( this, IDC_EDIT_SALEFILE_02, m_pDummyCrow->m_sAction.m_strSaleFile[2].c_str() );
	SetWin_Text( this, IDC_EDIT_NPCSHOP, m_pDummyCrow->m_sAction.m_strShopFile.c_str() ); /*npc shop, Juver, 2017/07/25 */
	SetWin_Text( this, IDC_EDIT_NPC_ITEM_EXCHANGE, m_pDummyCrow->m_sAction.m_strItemExchangeFile.c_str() );	/*item exchange, Juver, 2017/10/11 */

	SetWin_Text( this, IDC_EDIT_BIRTHEFFECT, m_pDummyCrow->m_sAction.m_strBirthEffect.c_str() );
	SetWin_Text( this, IDC_EDIT_DEATHEFFECT, m_pDummyCrow->m_sAction.m_strFallingEffect.c_str() );
	SetWin_Text( this, IDC_EDIT_BLOWEFFECT, m_pDummyCrow->m_sAction.m_strBlowEffect.c_str() );
	SetWin_Check( this, IDC_CHECK_DEATH_NOBODY, m_pDummyCrow->m_sAction.m_bAfterFall_NoBody );

	SetWin_Num_int( this, IDC_EDIT_BODYRADIUS, m_pDummyCrow->m_sAction.m_wBodyRadius );
	SetWin_Num_float( this, IDC_EDIT_WALKSPEED, m_pDummyCrow->m_sAction.m_fWalkVelo );
	SetWin_Num_float( this, IDC_EDIT_RUNSPEED, m_pDummyCrow->m_sAction.m_fRunVelo );
	SetWin_Check( this, IDC_CHECK_RUN, m_pDummyCrow->m_sAction.m_bRun );
	SetWin_Num_float( this, IDC_EDIT_LIVETIME, m_pDummyCrow->m_sAction.m_fLiveTime );


	SetWin_Num_int( this, IDC_EDIT_MOBLINK_MID, m_pDummyCrow->m_sAction.m_sMobLinkID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_MOBLINK_SID, m_pDummyCrow->m_sAction.m_sMobLinkID.wSubID );
	SetWin_Num_float( this, IDC_EDIT_MOBLINK_SCALE, m_pDummyCrow->m_sAction.m_fMobLinkScale );
	SetWin_Num_float( this, IDC_EDIT_MOBLINK_DELAY, m_pDummyCrow->m_sAction.m_fMobLinkDelay );
	SetWin_Check( this, IDC_CHECK_MOBLINK_ENABLE, m_pDummyCrow->m_sAction.m_bMobLink );

	//gen
	SetWin_Text( this, IDC_EDIT_GEN_ITEM, m_pDummyCrow->m_sGenerate.m_strGenItem.c_str() );
	SetWin_Text( this, IDC_EDIT_GEN_QITEM, m_pDummyCrow->m_sGenerate.m_strQtGenItem.c_str() );
	SetWin_Num_int( this, IDC_EDIT_GEN_ITEM_MID, m_pDummyCrow->m_sGenerate.m_sGenItemID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_GEN_ITEM_SID, m_pDummyCrow->m_sGenerate.m_sGenItemID.wSubID );
	SetWin_Num_int( this, IDC_EDIT_GEN_ITEM_RATE, m_pDummyCrow->m_sGenerate.m_wGenItem_Rate );
	SetWin_Num_int( this, IDC_EDIT_GEN_MONEY_VALUE, m_pDummyCrow->m_sGenerate.m_dwGenMoney );
	SetWin_Num_int( this, IDC_EDIT_GEN_MONEY_RATE, m_pDummyCrow->m_sGenerate.m_wGenMoney_Rate );

	/*Money Luck, Juver, 2017/08/24 */
	SetWin_Num_float( this, IDC_EDIT_GEN_MONEY_LUCK_RATE, m_pDummyCrow->m_sGenerate.m_fMoneyLuckRate );
	SetWin_Num_int( this, IDC_EDIT_GEN_MONEY_LUCK_MAX, m_pDummyCrow->m_sGenerate.m_wMoneyLuckMax );


	//flags
	SetWin_Check ( this, IDC_CHECK_OVERLAPATTACK, m_pDummyCrow->m_sBasic.m_bOverlapAttack );
	SetWin_Check ( this, IDC_CHECK_EMFLAG_TARSHORT, m_pDummyCrow->m_sAction.m_dwActFlag&EMCROWACT_TARSHORT );
	SetWin_Check ( this, IDC_CHECK_EMFLAG_CDCERTIFY, m_pDummyCrow->m_sAction.m_dwActFlag&EMCROWACT_CDCERTIFY );
	SetWin_Check ( this, IDC_CHECK_EMFLAG_INVISIBLE, m_pDummyCrow->m_sAction.m_dwActFlag&EMCROWACT_INVISIBLE );
	SetWin_Check ( this, IDC_CHECK_EMFLAG_RECINVISIBLE, m_pDummyCrow->m_sAction.m_dwActFlag&EMCROWACT_RECVISIBLE );
	SetWin_Check ( this, IDC_CHECK_EMFLAG_BARRIER, m_pDummyCrow->m_sAction.m_dwActFlag&EMCROWACT_BARRIER );
	SetWin_Check ( this, IDC_CHECK_EMFLAG_POSHOLD, m_pDummyCrow->m_sAction.m_dwActFlag&EMCROWACT_POSHOLD );
	SetWin_Check ( this, IDC_CHECK_EMFLAG_DIRHOLD, m_pDummyCrow->m_sAction.m_dwActFlag&EMCROWACT_DIRHOLD );
	SetWin_Check ( this, IDC_CHECK_EMFLAG_KNOCK, m_pDummyCrow->m_sAction.m_dwActFlag&EMCROWACT_KNOCK );
	SetWin_Check ( this, IDC_CHECK_EMFLAG_BOSS, m_pDummyCrow->m_sAction.m_dwActFlag&EMCROWACT_BOSS );
	SetWin_Check ( this, IDC_CHECK_EMFLAG_BUSUNIT, m_pDummyCrow->m_sAction.m_dwActFlag&EMCROWACT_BUSUNIT );
	SetWin_Check ( this, IDC_CHECK_EMFLAG_IGNORESHOCK, m_pDummyCrow->m_sAction.m_dwActFlag&EMCROWACT_IGNORE_SHOCK );
	SetWin_Check ( this, IDC_CHECK_EMFLAG_AUTODROPOUT, m_pDummyCrow->m_sAction.m_dwActFlag&EMCROWACT_AUTODROP );

	/*hide crow name, Juver, 2017/09/01 */
	SetWin_Check ( this, IDC_CHECK_HIDENAME, m_pDummyCrow->m_sBasic.m_bHideName );

	/*crow notice, Juver, 2017/11/25 */
	SetWin_Check ( this, IDC_CHECK_NOTICE_RESPAWN, m_pDummyCrow->m_sBasic.m_dwNotice&EMCROW_NOTICE_RESPAWN );
	SetWin_Check ( this, IDC_CHECK_NOTICE_RESPAWN2, m_pDummyCrow->m_sBasic.m_dwNotice&EMCROW_NOTICE_RESPAWN2 );
	SetWin_Check ( this, IDC_CHECK_NOTICE_DEFEAT, m_pDummyCrow->m_sBasic.m_dwNotice&EMCROW_NOTICE_DEFEAT );
	SetWin_Check ( this, IDC_CHECK_NOTICE_DEFEAT2, m_pDummyCrow->m_sBasic.m_dwNotice&EMCROW_NOTICE_DEFEAT2 );

	SetWin_Num_int( this, IDC_EDIT_DUMMY_0, m_pDummyCrow->m_sBasic.dummy_var_0 );
	SetWin_Check ( this, IDC_CHECK_DUMMY_1, m_pDummyCrow->m_sBasic.dummy_var_1 );
	SetWin_Check ( this, IDC_CHECK_DUMMY_2, m_pDummyCrow->m_sBasic.dummy_var_2 );

	InitMobLink();
	InitGenItem();

	return TRUE;
}

BOOL CPageEdit1::DataSave()
{
	if ( !m_pDummyCrow )	return FALSE;

	//basic
	m_pDummyCrow->m_sBasic.sNativeID.wMainID = GetWin_Num_int( this, IDC_EDIT_MID );
	m_pDummyCrow->m_sBasic.sNativeID.wSubID = GetWin_Num_int( this, IDC_EDIT_SID );

	m_pDummyCrow->m_sBasic.m_emTribe = static_cast<EMTRIBE> ( GetWin_Combo_Sel( this, IDC_COMBO_TRIBE ) );
	m_pDummyCrow->m_sBasic.m_emCrow = static_cast<EMCROW> ( GetWin_Combo_Sel( this, IDC_COMBO_CROW ) );
	m_pDummyCrow->m_sBasic.m_emBright = static_cast<EMBRIGHT> ( GetWin_Combo_Sel( this, IDC_COMBO_BRIGHT ) );
	m_pDummyCrow->m_sBasic.m_emPetType = static_cast<PETTYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_PET ) );
	m_pDummyCrow->m_sBasic.m_emSummonType = static_cast<SUMMON_TYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_SUMMON ) );
	m_pDummyCrow->m_sBasic.m_emNPCType = static_cast<EMNPC_TYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_NPCTYPE ) );
	m_pDummyCrow->m_sBasic.m_emCTFType = static_cast<EMNPC_CTF_FLAG_TYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_CTFTYPE ) ); /*pvp capture the flag, Juver, 2018/02/03 */

	m_pDummyCrow->m_sBasic.m_dwHP = GetWin_Num_int( this, IDC_EDIT_HP );
	m_pDummyCrow->m_sBasic.m_wMP = GetWin_Num_int( this, IDC_EDIT_MP );
	m_pDummyCrow->m_sBasic.m_wSP = GetWin_Num_int( this, IDC_EDIT_SP );
	m_pDummyCrow->m_sBasic.m_fIncHP = GetWin_Num_float( this, IDC_EDIT_HP_REC );
	m_pDummyCrow->m_sBasic.m_fIncMP = GetWin_Num_float( this, IDC_EDIT_MP_REC );
	m_pDummyCrow->m_sBasic.m_fIncSP = GetWin_Num_float( this, IDC_EDIT_SP_REC );
	m_pDummyCrow->m_sBasic.m_wLevel = GetWin_Num_int( this, IDC_EDIT_LEVEL );
	m_pDummyCrow->m_sBasic.m_wDefense = GetWin_Num_int( this, IDC_EDIT_DEF );
	m_pDummyCrow->m_sBasic.m_wAvoidRate = GetWin_Num_int( this, IDC_EDIT_AVOIDRATE );
	m_pDummyCrow->m_sBasic.m_wHitRate = GetWin_Num_int( this, IDC_EDIT_HITRATE );
	m_pDummyCrow->m_sBasic.m_wViewRange = GetWin_Num_int( this, IDC_EDIT_VIEWDISTANCE );
	m_pDummyCrow->m_sBasic.m_dwGenTime = GetWin_Num_int( this, IDC_EDIT_GENTIME );
	m_pDummyCrow->m_sBasic.m_dwBonusExp = GetWin_Num_int( this, IDC_EDIT_ADDEXP );
	m_pDummyCrow->m_sBasic.m_wBonusCP = GetWin_Num_int( this, IDC_EDIT_ADDCP );
	m_pDummyCrow->m_sBasic.m_sResist.nFire = GetWin_Num_int( this, IDC_EDIT_RES_FIRE );
	m_pDummyCrow->m_sBasic.m_sResist.nIce = GetWin_Num_int( this, IDC_EDIT_RES_ICE );
	m_pDummyCrow->m_sBasic.m_sResist.nElectric = GetWin_Num_int( this, IDC_EDIT_RES_ELECT );
	m_pDummyCrow->m_sBasic.m_sResist.nPoison = GetWin_Num_int( this, IDC_EDIT_RES_POI );
	m_pDummyCrow->m_sBasic.m_sResist.nSpirit = GetWin_Num_int( this, IDC_EDIT_RES_SPIRIT );

	m_pDummyCrow->m_sBasic.m_wGatherTimeLow = GetWin_Num_int( this, IDC_EDIT_GATHER_TIME_LOW );
	m_pDummyCrow->m_sBasic.m_wGatherTimeHigh = GetWin_Num_int( this, IDC_EDIT_GATHER_TIME_HIGH );
	m_pDummyCrow->m_sBasic.m_fGatherRate = GetWin_Num_float( this, IDC_EDIT_GATHER_RATE );
	m_pDummyCrow->m_wGatherAnimation = (WORD)GetWin_Combo_Sel( this, IDC_COMBO_GATHER_ANI );

	//action
	m_pDummyCrow->m_sAction.m_strSkinObj = GetWin_Text( this, IDC_EDIT_SKINFILE ).GetString();
	m_pDummyCrow->m_sAction.m_strTalkFile = GetWin_Text( this, IDC_EDIT_TALKFILE ).GetString();
	m_pDummyCrow->m_sAction.m_strSaleFile[0] = GetWin_Text( this, IDC_EDIT_SALEFILE_00 ).GetString();
	m_pDummyCrow->m_sAction.m_strSaleFile[1] = GetWin_Text( this, IDC_EDIT_SALEFILE_01 ).GetString();
	m_pDummyCrow->m_sAction.m_strSaleFile[2] = GetWin_Text( this, IDC_EDIT_SALEFILE_02 ).GetString();
	m_pDummyCrow->m_sAction.m_strShopFile = GetWin_Text( this, IDC_EDIT_NPCSHOP ).GetString(); /*npc shop, Juver, 2017/07/25 */
	m_pDummyCrow->m_sAction.m_strItemExchangeFile = GetWin_Text( this, IDC_EDIT_NPC_ITEM_EXCHANGE ).GetString(); /*item exchange, Juver, 2017/10/11 */

	m_pDummyCrow->m_sAction.m_strBirthEffect = GetWin_Text( this, IDC_EDIT_BIRTHEFFECT ).GetString();
	m_pDummyCrow->m_sAction.m_strFallingEffect = GetWin_Text( this, IDC_EDIT_DEATHEFFECT ).GetString();
	m_pDummyCrow->m_sAction.m_strBlowEffect = GetWin_Text( this, IDC_EDIT_BLOWEFFECT ).GetString();
	m_pDummyCrow->m_sAction.m_bAfterFall_NoBody = GetWin_Check( this, IDC_CHECK_DEATH_NOBODY );

	m_pDummyCrow->m_sAction.m_wBodyRadius = GetWin_Num_int( this, IDC_EDIT_BODYRADIUS );
	m_pDummyCrow->m_sAction.m_fWalkVelo = GetWin_Num_float( this, IDC_EDIT_WALKSPEED );
	m_pDummyCrow->m_sAction.m_fRunVelo = GetWin_Num_float( this, IDC_EDIT_RUNSPEED );
	m_pDummyCrow->m_sAction.m_bRun = GetWin_Check( this, IDC_CHECK_RUN );
	m_pDummyCrow->m_sAction.m_fLiveTime = GetWin_Num_float( this, IDC_EDIT_LIVETIME );

	m_pDummyCrow->m_sAction.m_sMobLinkID.wMainID = GetWin_Num_int( this, IDC_EDIT_MOBLINK_MID );
	m_pDummyCrow->m_sAction.m_sMobLinkID.wSubID = GetWin_Num_int( this, IDC_EDIT_MOBLINK_SID );
	m_pDummyCrow->m_sAction.m_fMobLinkScale = GetWin_Num_float( this, IDC_EDIT_MOBLINK_SCALE );
	m_pDummyCrow->m_sAction.m_fMobLinkDelay = GetWin_Num_float( this, IDC_EDIT_MOBLINK_DELAY );
	m_pDummyCrow->m_sAction.m_bMobLink = GetWin_Check( this, IDC_CHECK_MOBLINK_ENABLE );

	//gen
	m_pDummyCrow->m_sGenerate.m_strGenItem = GetWin_Text( this, IDC_EDIT_GEN_ITEM ).GetString();
	m_pDummyCrow->m_sGenerate.m_strQtGenItem = GetWin_Text( this, IDC_EDIT_GEN_QITEM ).GetString();
	m_pDummyCrow->m_sGenerate.m_sGenItemID.wMainID = GetWin_Num_int( this, IDC_EDIT_GEN_ITEM_MID );
	m_pDummyCrow->m_sGenerate.m_sGenItemID.wSubID = GetWin_Num_int( this, IDC_EDIT_GEN_ITEM_SID );
	m_pDummyCrow->m_sGenerate.m_wGenItem_Rate = GetWin_Num_int( this, IDC_EDIT_GEN_ITEM_RATE );
	m_pDummyCrow->m_sGenerate.m_dwGenMoney = GetWin_Num_int( this, IDC_EDIT_GEN_MONEY_VALUE );
	m_pDummyCrow->m_sGenerate.m_wGenMoney_Rate = GetWin_Num_int( this, IDC_EDIT_GEN_MONEY_RATE );

	/*Money Luck, Juver, 2017/08/24 */
	m_pDummyCrow->m_sGenerate.m_fMoneyLuckRate = GetWin_Num_float( this, IDC_EDIT_GEN_MONEY_LUCK_RATE );
	m_pDummyCrow->m_sGenerate.m_wMoneyLuckMax = GetWin_Num_int( this, IDC_EDIT_GEN_MONEY_LUCK_MAX );


	//flags
	m_pDummyCrow->m_sBasic.m_bOverlapAttack = ( GetWin_Check ( this, IDC_CHECK_OVERLAPATTACK ) == TRUE );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_EMFLAG_TARSHORT ), m_pDummyCrow->m_sAction.m_dwActFlag,EMCROWACT_TARSHORT );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_EMFLAG_CDCERTIFY ), m_pDummyCrow->m_sAction.m_dwActFlag,EMCROWACT_CDCERTIFY );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_EMFLAG_INVISIBLE ), m_pDummyCrow->m_sAction.m_dwActFlag,EMCROWACT_INVISIBLE );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_EMFLAG_RECINVISIBLE ), m_pDummyCrow->m_sAction.m_dwActFlag,EMCROWACT_RECVISIBLE );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_EMFLAG_BARRIER ), m_pDummyCrow->m_sAction.m_dwActFlag,EMCROWACT_BARRIER );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_EMFLAG_POSHOLD ), m_pDummyCrow->m_sAction.m_dwActFlag,EMCROWACT_POSHOLD );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_EMFLAG_DIRHOLD ), m_pDummyCrow->m_sAction.m_dwActFlag,EMCROWACT_DIRHOLD );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_EMFLAG_KNOCK ), m_pDummyCrow->m_sAction.m_dwActFlag,EMCROWACT_KNOCK );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_EMFLAG_BOSS ), m_pDummyCrow->m_sAction.m_dwActFlag,EMCROWACT_BOSS );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_EMFLAG_BUSUNIT ), m_pDummyCrow->m_sAction.m_dwActFlag,EMCROWACT_BUSUNIT );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_EMFLAG_IGNORESHOCK ), m_pDummyCrow->m_sAction.m_dwActFlag,EMCROWACT_IGNORE_SHOCK );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_EMFLAG_AUTODROPOUT ), m_pDummyCrow->m_sAction.m_dwActFlag,EMCROWACT_AUTODROP );

	/*hide crow name, Juver, 2017/09/01 */
	m_pDummyCrow->m_sBasic.m_bHideName = GetWin_Check ( this, IDC_CHECK_HIDENAME )? true:false;


	/*crow notice, Juver, 2017/11/25 */
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_NOTICE_RESPAWN ), m_pDummyCrow->m_sBasic.m_dwNotice,EMCROW_NOTICE_RESPAWN );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_NOTICE_RESPAWN2 ), m_pDummyCrow->m_sBasic.m_dwNotice,EMCROW_NOTICE_RESPAWN2 );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_NOTICE_DEFEAT ), m_pDummyCrow->m_sBasic.m_dwNotice,EMCROW_NOTICE_DEFEAT );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_NOTICE_DEFEAT2 ), m_pDummyCrow->m_sBasic.m_dwNotice,EMCROW_NOTICE_DEFEAT2 );

	m_pDummyCrow->m_sBasic.dummy_var_0 = GetWin_Num_int( this, IDC_EDIT_DUMMY_0 );
	m_pDummyCrow->m_sBasic.dummy_var_1 = GetWin_Check ( this, IDC_CHECK_DUMMY_1 )? true:false;
	m_pDummyCrow->m_sBasic.dummy_var_2 = GetWin_Check ( this, IDC_CHECK_DUMMY_2 )? true:false;

	return TRUE;
}
