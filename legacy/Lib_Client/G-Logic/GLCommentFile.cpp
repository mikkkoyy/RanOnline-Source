#include "stdafx.h"
#include "./GLCommentFile.h"
#include "./GLCharacter.h"
#include "./GLItem.h"
#include "./GLLevelFile.h"
#include "./GLMobSchedule.h"
#include "./GLOGIC.h"
#include "./GLSkill.h"
#include "../NpcTalk/NpcTalk.h"
#include "./GLQuest.h"
#include "GLSummonDef.h"
#include "./GLActivityDefine.h"

#include "../../Lib_Engine/Common/IniLoader.h"
#include "../../Lib_Engine/DxCommon/DxFontMan.h"

/*product item, Juver, 2017/10/15 */
#include "./GLProductDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CGLCommentFile::CGLCommentFile()
{
}

CGLCommentFile::~CGLCommentFile()
{
}

bool CGLCommentFile::LOADFILE( const char * szFileName )
{
	if( !szFileName )				return false;
	if( strlen(szFileName) == 0 )	return false;

	std::string strPath;
	strPath = GLOGIC::GetPath();
	strPath += szFileName;	

	CIniLoader cFILE;

	if( GLOGIC::bGLOGIC_ZIPFILE )
		cFILE.SetZipFile( GLOGIC::strGLOGIC_ZIPFILE );

	std::string strSep( ",[]\t" );
	cFILE.reg_sep( strSep );

	if( !cFILE.open( strPath, true, GLOGIC::bGLOGIC_PACKFILE ) )
	{
		CDebugSet::ToLogFile ( "ERROR : CGLCommentFile::LOADFILE(), File Open %s", szFileName );
		return false;
	}

	for ( int i=0; i<SKILL::EMROLE_NSIZE; ++i )
		cFILE.getflag( "SKILL_ROLE", i, 0, 1, COMMENT::SKILL_ROLE[i] );

	for (int i=0; i<SKILL::EMAPPLY_NSIZE; ++i )
		cFILE.getflag( "SKILL_APPLY", i, 0, 1, COMMENT::SKILL_APPLY[i]);

	for (int i=0; i<SKILL::EMACTION_NSIZE; ++i )
		cFILE.getflag( "SKILL_ACTION_TYPE", i, 0, 1, COMMENT::SKILL_ACTION_TYPE[i]);

	for (int i=0; i<SKILL::EMEFFECT_TYPE_SIZE; ++i )
		cFILE.getflag( "SKILL_EFFECT_TYPE", i, 0, 1, COMMENT::SKILL_EFFECT_TYPE[i]);

	for (int i=0; i<SKILL::FOR_TYPE_SIZE; ++i )
	{
		cFILE.getflag( "SKILL_TYPES", i, 0, 2, COMMENT::SKILL_TYPES[i]);
		cFILE.getflag( "SKILL_TYPES", i, 1, 2, COMMENT::SKILL_TYPES_SCALE[i]);
	}

	for (int i=0; i<SKILL::MAX_LEVEL; ++i )
		cFILE.getflag( "SKILL_LEVEL", i, 0, 1, COMMENT::SKILL_LEVEL[i]);

	for (int i=0; i<SKILL::EMTIME_NSIZE; ++i )
		cFILE.getflag( "SKILL_EFFTIME", i, 0, 1, COMMENT::SKILL_EFFTIME[i]);

	for (int i=0; i<SKILL::EMPOS_NSIZE; ++i )
		cFILE.getflag( "SKILL_EFFPOS", i, 0, 1, COMMENT::SKILL_EFFPOS[i]);

	for (int i=0; i<SKILL::EMSSTYPE_NSIZE; ++i )
		cFILE.getflag( "SPECIAL_SKILL_TYPE", i, 0, 1, COMMENT::SPECIAL_SKILL_TYPE[i]);

	for (int i=0; i<EMSIGN_SIZE; ++i )
	{
		cFILE.getflag( "CDT_SIGN", i, 0, 2, COMMENT::CDT_SIGN[i]);
		cFILE.getflag( "CDT_SIGN", i, 1, 2, COMMENT::CDT_SIGN_ID[i]);
	}

	for (int i=0; i<ITEM::SSUIT::ADDON_SIZE; ++i )
		cFILE.getflag( "ADDON_NO", i, 0, 1, COMMENT::ADDON_NO[i]);

	for (int i=0; i<ITEM::SSUIT::VARIATION_SIZE; ++i )
		cFILE.getflag( "SVARON_NO", i, 0, 1, COMMENT::SVARON_NO[i]);

	for (int i=0; i<EMCROWACT_UP_NSIZE; ++i )
		cFILE.getflag( "CROWACT_UP", i, 0, 1, COMMENT::szCROWACT_UP[i]);

	for (int i=0; i<EMCROWACT_DN_NSIZE; ++i )
		cFILE.getflag( "CROWACT_DN", i, 0, 1, COMMENT::szCROWACT_DN[i]);

	for (int i=0; i<CROWSKTAR_SIZE; ++i )
		cFILE.getflag( "CROWSKTAR", i, 0, 1, COMMENT::CROWSKTAR[i]);

	for (int i=0; i<REACT_SIZE+1; ++i )
		cFILE.getflag( "EMREACTION", i, 0, 1, COMMENT::szEMREACTION[i]);

	for (int i=0; i<EMACTION_SIZE; ++i )
		cFILE.getflag( "MOBACTIONS", i, 0, 1, COMMENT::MOBACTIONS[i]);

	DWORD dwColor_R;
	DWORD dwColor_G;
	DWORD dwColor_B;

	for (int i=0; i<LEVEL_NSIZE; ++i )
	{
		cFILE.getflag( "ITEMLEVEL", i, 0, 4, COMMENT::ITEMLEVEL[i]);
		cFILE.getflag( "ITEMLEVEL", i, 1, 4, dwColor_R);
		cFILE.getflag( "ITEMLEVEL", i, 2, 4, dwColor_G);
		cFILE.getflag( "ITEMLEVEL", i, 3, 4, dwColor_B);

		COMMENT::ITEMCOLOR[i] = D3DCOLOR_ARGB(0xff,dwColor_R,dwColor_G,dwColor_B);
	}

	for (int i=0; i<ITEM_NSIZE; ++i )
		cFILE.getflag( "ITEMTYPE", i, 0, 1, COMMENT::ITEMTYPE[i]);

	for (int i=0; i<SUIT_NSIZE; ++i )
		cFILE.getflag( "ITEMSUIT", i, 0, 1, COMMENT::ITEMSUIT[i]);

	for (int i=0; i<ITEM_DRUG_SIZE; ++i )
		cFILE.getflag( "ITEMDRUG", i, 0, 1, COMMENT::ITEMDRUG[i]);

	for (int i=0; i<ITEMATT_NSIZE; ++i )
		cFILE.getflag( "ITEMATTACK", i, 0, 1, COMMENT::ITEMATTACK[i]);

	for (int i=0; i<EMADD_SIZE; ++i )
		cFILE.getflag( "ITEMADDON", i, 0, 1, COMMENT::ITEMADDON[i]);

	for (int i=0; i<EMVAR_SIZE; ++i )
	{
		cFILE.getflag( "ITEMVAR", i, 0, 2, COMMENT::ITEMVAR[i]);
		cFILE.getflag( "ITEMVAR", i, 1, 2, COMMENT::ITEMVAR_SCALE[i]);
	}

	for (int i=0; i<EMVAR_SIZE; ++i )
	{
		cFILE.getflag( "ITEMVOL", i, 0, 1, COMMENT::ITEMVOL[i]);
	}

	for (int i=0; i<QUESTION_SIZE; ++i )
	{
		cFILE.getflag( "ITEM_QUE_TYPE", i, 0, 3, COMMENT::ITEM_QUE_TYPE[i] );
		cFILE.getflag( "ITEM_QUE_TYPE", i, 1, 3, COMMENT::ITEM_QUE_VAR1[i] );
		cFILE.getflag( "ITEM_QUE_TYPE", i, 2, 3, COMMENT::ITEM_QUE_VAR2[i] );
	}

	for (int i=0; i<EMGRINDING_NSIZE; ++i )
		cFILE.getflag( "GRINDING_TYPE", i, 0, 1, COMMENT::GRINDING_TYPE[i]);

	for (int i=0; i<EMGRINDING_RS_NSIZE; ++i )
		cFILE.getflag( "GRINDING_RESIST", i, 0, 1, COMMENT::GRINDING_RESIST[i]);

	for (int i=0; i<EMGRINDING_LEVEL_SIZE; ++i )
		cFILE.getflag( "GRINDING_LEVEL", i, 0, 1, COMMENT::GRINDING_LEVEL[i]);

	for (int i=0; i<EMGRINDING_CLASS_SIZE; ++i )
		cFILE.getflag( "GRINDING_CLASS", i, 0, 1, COMMENT::GRINDING_CLASS[i]);

	for (int i=0; i<EMCOOL_SIZE; ++i )
		cFILE.getflag( "COOLTYPE", i, 0, 1, COMMENT::COOLTYPE[i]);

	for (int i=0; i<EMR_OPT_SIZE; ++i )
		cFILE.getflag( "EMRANDOM_OPT", i, 0, 1, COMMENT::ITEM_RANDOM_OPT[i]);

    for (int i=0; i<EMR_OPT_SIZE; ++i )
		cFILE.getflag( "EMRANDOM_OPT_SCALE", i, 0, 1, COMMENT::ITEM_RANDOM_OPT_SCALE[i]);

	for (int i=0; i<SLOT_NSIZE_S_2; ++i )
		cFILE.getflag( "ITEMSLOT_CHAR", i, 0, 1, COMMENT::ITEMSLOT_CHAR[i]);

	for (int i=0; i<PET_ACCETYPE_SIZE; ++i )
		cFILE.getflag( "ITEMSLOT_PET", i, 0, 1, COMMENT::ITEMSLOT_PET[i]);

	for (int i=0; i<VEHICLE_ACCETYPE_SIZE; ++i )
		cFILE.getflag( "ITEMSLOT_VEHICLE", i, 0, 1, COMMENT::ITEMSLOT_VEHICLE[i]);

	/*bike color , Juver, 2017/11/13 */
	for (int i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
		cFILE.getflag( "BIKE_COLOR_SLOT", i, 0, 1, COMMENT::BIKE_COLOR_SLOT[i]);

	for (int i=0; i<GLAT_SIZE; ++i )
		cFILE.getflag( "ACTIONTYPE", i, 0, 1, COMMENT::ACTIONTYPE[i]);

	for (int i=0; i<TRIBE_NSIZE; ++i )
		cFILE.getflag( "TRIBE", i, 0, 1, COMMENT::TRIBE[i]);

	for (int i=0; i<MOVETYPE_NSIZE; ++i )
		cFILE.getflag( "MOVETYPE", i, 0, 1, COMMENT::MOVETYPE[i]);

	for (int i=0; i<EMELEMENT_MAXNUM2; ++i )
		cFILE.getflag( "ELEMENT", i, 0, 1, COMMENT::ELEMENT[i]);

	for (int i=0; i<BRIGHT_SIZE; ++i )
		cFILE.getflag( "BRIGHT", i, 0, 1, COMMENT::BRIGHT[i]);

	for (int i=0; i<GLCI_NUM_8CLASS; ++i )
	{
		cFILE.getflag( "CHARCLASS", i, 0, 2, COMMENT::CHARCLASS[i]);
		cFILE.getflag( "CHARCLASS", i, 1, 2, COMMENT::CHARCOMMENT[i]);
	}

	for (int i=0; i<GLCI2_NUM_8CLASS+1; ++i )
	{
		cFILE.getflag( "CHARCLASS2", i, 0, 1, COMMENT::CHARCLASS2[i]);
	}

	for (int i=0; i<EMBLOW_SIZE; ++i )
	{
		cFILE.getflag( "BLOW", i, 0, 5, COMMENT::BLOW[i]);
		cFILE.getflag( "BLOW", i, 1, 5, COMMENT::BLOW_VAR1[i]);
		cFILE.getflag( "BLOW", i, 2, 5, COMMENT::BLOW_VAR1_SCALE[i]);
		cFILE.getflag( "BLOW", i, 3, 5, COMMENT::BLOW_VAR2[i]);
		cFILE.getflag( "BLOW", i, 4, 5, COMMENT::BLOW_VAR2_SCALE[i]);
	}

	for (int i=0; i<EIMPACTA_SIZE; ++i )
	{
		cFILE.getflag( "IMPACT_ADDON", i, 0, 2, COMMENT::IMPACT_ADDON[i]);
		cFILE.getflag( "IMPACT_ADDON", i, 1, 2, COMMENT::IMPACT_ADDON_SCALE[i]);
	}

	for (int i=0; i<TAR_SIZE; ++i )
		cFILE.getflag( "IMPACT_TAR", i, 0, 1, COMMENT::IMPACT_TAR[i]);

	for (int i=0; i<REALM_SIZE; ++i )
		cFILE.getflag( "IMPACT_REALM", i, 0, 1, COMMENT::IMPACT_REALM[i]);

	for (int i=0; i<SIDE_SIZE; ++i )
		cFILE.getflag( "IMPACT_SIDE", i, 0, 1, COMMENT::IMPACT_SIDE[i]);

	for (int i=0; i<EMSPECA_NSIZE; ++i )
	{
		cFILE.getflag( "SPEC_ADDON", i, 0, 5, COMMENT::SPEC_ADDON[i]);
		cFILE.getflag( "SPEC_ADDON", i, 1, 5, COMMENT::SPEC_ADDON_VAR1[i]);
		cFILE.getflag( "SPEC_ADDON", i, 2, 5, COMMENT::SPEC_ADDON_VAR1_SCALE[i]);
		cFILE.getflag( "SPEC_ADDON", i, 3, 5, COMMENT::SPEC_ADDON_VAR2[i]);
		cFILE.getflag( "SPEC_ADDON", i, 4, 5, COMMENT::SPEC_ADDON_VAR2_SCALE[i]);
	}

	for (int i=0; i<SKILL::EMSPEC_STIGMA_TYPE_SIZE; ++i )
		cFILE.getflag( "SPEC_STIGMA_TYPE", i, 0, 1, COMMENT::SPEC_STIGMA_TYPE[i]);

	for (int i=0; i<SKILL::EMSPEC_HALLUCINATE_TYPE_SIZE; ++i )
		cFILE.getflag( "SPEC_HALLUCINATE_TYPE", i, 0, 1, COMMENT::SPEC_HALLUCINATE_TYPE[i]);

	for (int i=0; i<SKILL::EMSPEC_DOMINATE_TYPE_SIZE; ++i )
		cFILE.getflag( "SPEC_DOMINATE_TYPE", i, 0, 1, COMMENT::SPEC_DOMINATE_TYPE[i]);

	for (int i=0; i<SKILL::EMSPEC_ASIGN_TYPE_SIZE; ++i )
		cFILE.getflag( "SPEC_ASIGN_TYPE", i, 0, 1, COMMENT::SPEC_ASIGN_TYPE[i]);

	for (int i=0; i<EMSPEC_INC_TYPE_NSIZE; ++i )
		cFILE.getflag( "SPEC_INC_TYPE", i, 0, 1, COMMENT::SPEC_INC_TYPE[i]);

	//for ( i = 0; i<EMSPEC_INC_TYPE_NSIZE; ++i )
	//	cFILE.getflag( "CharDefine", "SPEC_INC_TYPE", i, EMSPEC_INC_TYPE_NSIZE, COMMENT::SPEC_INC_TYPE[i]);

	for (int i=0; i<SKILL::EMSPEC_INVISIBLE_USE_TYPE_SIZE; ++i )
		cFILE.getflag( "SPEC_INVISIBLE_USE_TYPE", i, 0, 1, COMMENT::SPEC_INVISIBLE_USE_TYPE[i]);

	for (int i=0; i<SKILL::EMSPEC_INVISIBLE_ANI_TYPE_SIZE; ++i )
		cFILE.getflag( "SPEC_INVISIBLE_ANI_TYPE", i, 0, 1, COMMENT::SPEC_INVISIBLE_ANI_TYPE[i]);

	for (int i=0; i<SKILL::EMSPEC_SWAPPOS_DIR_TYPE_SIZE; ++i )
		cFILE.getflag( "SPEC_SWAPPOS_DIR_TYPE", i, 0, 1, COMMENT::SPEC_SWAPPOS_DIR_TYPE[i]);


	for (int i=0; i<EMSKILLCLASS_NSIZE; ++i )
		cFILE.getflag( "SKILLCLASS", i, 0, 1, COMMENT::SKILLCLASS[i]);

	for (int i=0; i<EMLANDEFFECT_SIZE; ++i )
		cFILE.getflag( "LANDEFFECT_TYPE", i, 0, 1, COMMENT::LANDEFFECT_TYPE[i]);

	for (int i=0; i<CROW_NUM; ++i )
		cFILE.getflag( "CROW", i, 0, 1, COMMENT::CROW[i]);

	for (int i=0; i<EMNPC_TYPE_SIZE; ++i )
		cFILE.getflag( "NPC_TYPE", i, 0, 1, COMMENT::NPC_TYPE[i]);

	/*pvp capture the flag, Juver, 2018/02/03 */
	for (int i=0; i<EMNPC_CTF_FLAG_TYPE_SIZE; ++i )
		cFILE.getflag( "CTF_TYPE", i, 0, 1, COMMENT::CTF_TYPE[i]);

	for (int i=0; i<EMATT_SIZE; ++i )
		cFILE.getflag( "ATTACK_RGTYPE", i, 0, 1, COMMENT::ATTACK_RGTYPE[i]);

	for (int i=0; i<2; ++i )
		cFILE.getflag( "FONTSYS", i, 0, 1, COMMENT::FONTSYS[i]);

	for (int i=0; i<AN_TYPE_SIZE; ++i )
		cFILE.getflag( "ANI_MAINTYPE_CHAR", i, 0, 1, COMMENT::ANI_MAINTYPE_CHAR[i]);

	for (int i=0; i<AN_TYPE_SIZE; ++i )
		cFILE.getflag( "ANI_MAINTYPE_ABL", i, 0, 1, COMMENT::ANI_MAINTYPE_ABL[i]);

	for (int i=0; i<AN_TYPE_SIZE; ++i )
		cFILE.getflag( "ANI_MAINTYPE_VEHICLE", i, 0, 1, COMMENT::ANI_MAINTYPE_VEHICLE[i]);

	for (int i=0; i<AN_SUB_00_SIZE; ++i )
		cFILE.getflag( "ANI_SUBTYPE_WEAPON", i, 0, 1, COMMENT::ANI_SUBTYPE_WEAPON[i] );

	for (int i=0; i<AN_SUB_00_SIZE; ++i )
		cFILE.getflag( "ANI_SUBTYPE_SKILL", i, 0, 1, COMMENT::ANI_SUBTYPE_SKILL[i] );

	for (int i=0; i<AN_SUB_00_SIZE; ++i )
		cFILE.getflag( "ANI_SUBTYPE_VEHICLE", i, 0, 1, COMMENT::ANI_SUBTYPE_VEHICLE[i] );

	for (int i=0; i<EMSF_SIZE; ++i )
		cFILE.getflag( "STRIKE_EFF", i, 0, 1, COMMENT::STRIKE_EFF[i]);

	for (int i=0; i<SNpcTalk::EM_TALK_SIZE; ++i )
		cFILE.getflag( "NPC_ACTIONTYPE", i, 0, 1, SNpcTalk::strACTIONTYPE[i]);

	for (int i=0; i<SNpcTalk::EM_BASIC_SIZE; ++i )
		cFILE.getflag( "NPC_BASICDESC", i, 0, 1, SNpcTalk::szBASICDESC[i]);

	for (int i=0; i < EMQP_SIZE; ++i )
		cFILE.getflag( "QUEST_PROGRESS", i, 0, 1, COMMENT::QUEST_PROGRESS[i]);

	for (int i=0; i < EMQUEST_AREA_SIZE; ++i )
		cFILE.getflag( "QUEST_AREA", i, 0, 1, COMMENT::QUEST_AREA[i]);

	for (int i=0; i<3; ++i ) 
	{
		cFILE.getflag( "SCHOOL", i, 0, 1, GLCONST_CHAR::strSCHOOLNAME[i] );
		cFILE.getflag( "SCHOOL", i, 0, 1, COMMENT::EMSCHOOL[i] );
	}

	for (int i=0; i<PET_TYPE_SIZE; ++i )
		cFILE.getflag( "PET_TYPE", i, 0, 1, COMMENT::PET_TYPE[i] );

	for (int i=0; i<VEHICLE_TYPE_SIZE; ++i )
		cFILE.getflag( "VEHICLE_TYPE", i, 0, 1, COMMENT::VEHICLE_TYPE[i] );

	for (int i=0; i<VEHICLE_SUBTYPE_SIZE; ++i )
		cFILE.getflag( "VEHICLE_SUBTYPE_HOVER", i, 0, 1, COMMENT::VEHICLE_SUBTYPE_HOVER[i] );

	for (int i=0; i<VEHICLE_SUBTYPE_SIZE; ++i )
		cFILE.getflag( "VEHICLE_SUBTYPE_BIKE", i, 0, 1, COMMENT::VEHICLE_SUBTYPE_BIKE[i] );

	/* vehicle car, cart, Juver, 2018/02/13 */
	for (int i=0; i<VEHICLE_SUBTYPE_SIZE; ++i )
		cFILE.getflag( "VEHICLE_SUBTYPE_CAR", i, 0, 1, COMMENT::VEHICLE_SUBTYPE_CAR[i] );

	/* vehicle car, cart, Juver, 2018/02/13 */
	for (int i=0; i<VEHICLE_SUBTYPE_SIZE; ++i )
		cFILE.getflag( "VEHICLE_SUBTYPE_CART", i, 0, 1, COMMENT::VEHICLE_SUBTYPE_CART[i] );

	for (int i=0; i<GLGENDER_NUM; ++i )
		cFILE.getflag( "EMCHARGENDER", i, 0, 1, COMMENT::EMGENDER[i] );

	for (int i=0; i<SKILLATT_NSIZE; ++i )
		cFILE.getflag( "SKILLATTACK", i, 0, 1, COMMENT::SKILLATTACK[i]);

	for (int i=0; i<SUMMON_TYPE_SIZE; ++i )
		cFILE.getflag( "SUMMON_TYPE", i, 0, 1, COMMENT::SUMMON_TYPE[i] );

	/*activity system, Juver, 2017/10/27 */
	for (int i=0; i<EMACTIVITY_NOTIFY_SIZE; ++i )
		cFILE.getflag( "ACTIVITY_NOTIFY", i, 0, 1, COMMENT::ACTIVITY_NOTIFY[i]);

	/*activity system, Juver, 2017/10/27 */
	for (int i=0; i<EMACTIVITY_TYPE_SIZE; ++i )
		cFILE.getflag( "ACTIVITY_MID", i, 0, 1, COMMENT::ACTIVITY_MID[i]);

	/*Codex System, Jhoniex 2/14/2024 */
	for (int i=0; i<EMCODEX_NOTIFY_SIZE; ++i )
		cFILE.getflag( "CODEX_NOTIFY", i, 0, 1, COMMENT::CODEX_NOTIFY[i]);
	for (int i=0; i<EMCODEX_TYPE_SIZE; ++i )
		cFILE.getflag( "CODEX_MID", i, 0, 1, COMMENT::CODEX_MID[i]);
	
	/*product item, Juver, 2017/10/15 */
	for (int i=0; i<PRODUCT_TYPE_SIZE; ++i )
		cFILE.getflag( "PRODUCT_TYPE_NAME", i, 0, 1, COMMENT::PRODUCT_TYPE_NAME[i]);

	return true;
}