#include "stdafx.h"
#include "EditorItem.h"
#include "PageEdit1.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "GLStringTable.h"
#include "GLItemMan.h"
#include "DxSkinPieceContainer.h"
#include "GLPet.h"
#include "GLVEHICLE.h"

BOOL CPageEdit1::DataDefault ()
{
	SetWin_Combo_Init( this, IDC_COMBO_GRADE, COMMENT::ITEMLEVEL, LEVEL_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_ITEMTYPE, COMMENT::ITEMTYPE, ITEM_NSIZE );
	SetWin_Combo_Init( this, IDC_COMBO_BRIGHT, COMMENT::BRIGHT, BRIGHT_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_COOLDOWN_TYPE, COMMENT::COOLTYPE, EMCOOL_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_PET, COMMENT::PET_TYPE, PET_TYPE_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_VEHICLE, COMMENT::VEHICLE_TYPE, VEHICLE_TYPE_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_VEHICLE2, COMMENT::VEHICLE_SUBTYPE_HOVER, VEHICLE_SUBTYPE_SIZE );

	InitVehicleTypes();

	return TRUE;
}

BOOL CPageEdit1::DataShow ()
{
	SetWin_Num_int( this, IDC_EDIT_GROUP_MID, m_pDummyItem->sBasicOp.sGroupID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_GROUP_SID, m_pDummyItem->sBasicOp.sGroupID.wSubID );
	SetWin_Num_int( this, IDC_EDIT_ITEMID_MID, m_pDummyItem->sBasicOp.sNativeID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_ITEMID_SID, m_pDummyItem->sBasicOp.sNativeID.wSubID );

	SetWin_Text( this, IDC_EDIT_NAME, m_pDummyItem->sBasicOp.strName.c_str() );
	SetWin_Text( this, IDC_EDIT_COMMENT, m_pDummyItem->sBasicOp.strComment.c_str() );

	const char* szName =  GLStringTable::GetInstance().GetString( GetWin_Text( this, IDC_EDIT_NAME ).GetString(),GLStringTable::ITEM );
	const char* szComment =  GLStringTable::GetInstance().GetString( GetWin_Text( this, IDC_EDIT_COMMENT ).GetString(),GLStringTable::ITEM );
	SetWin_Text ( this, IDC_EDIT_NAME2, szName? szName: "" );	
	SetWin_Text ( this, IDC_EDIT_COMMENT2, szComment? szComment: "" );	

	SetWin_Combo_Sel( this, IDC_COMBO_GRADE, COMMENT::ITEMLEVEL[m_pDummyItem->sBasicOp.emLevel] );
	SetWin_Combo_Sel( this, IDC_COMBO_ITEMTYPE, COMMENT::ITEMTYPE[m_pDummyItem->sBasicOp.emItemType] );

	SetWin_Num_int( this, IDC_EDIT_PRICE_BUY, m_pDummyItem->sBasicOp.dwBuyPrice );
	SetWin_Num_int( this, IDC_EDIT_PRICE_SELL, m_pDummyItem->sBasicOp.dwSellPrice );

	SetWin_Check ( this, IDC_CHECK_FLAG_SALE, m_pDummyItem->sBasicOp.dwFlags&TRADE_SALE );
	SetWin_Check ( this, IDC_CHECK_FLAG_EXCHANGE, m_pDummyItem->sBasicOp.dwFlags&TRADE_EXCHANGE );
	SetWin_Check ( this, IDC_CHECK_FLAG_THROW, m_pDummyItem->sBasicOp.dwFlags&TRADE_THROW );
	SetWin_Check ( this, IDC_CHECK_FLAG_DISGUISE, m_pDummyItem->sBasicOp.dwFlags&ITEM_DISGUISE );
	SetWin_Check ( this, IDC_CHECK_FLAG_TIMELMT, m_pDummyItem->sBasicOp.dwFlags&ITEM_TIMELMT );
	SetWin_Check ( this, IDC_CHECK_FLAG_GARBAGE, m_pDummyItem->sBasicOp.dwFlags&TRADE_GARBAGE );
	SetWin_Check ( this, IDC_CHECK_FLAG_EVENTITEM, m_pDummyItem->sBasicOp.dwFlags&TRADE_EVENT_SGL );
	SetWin_Check ( this, IDC_CHECK_FLAG_CHANNELALL, m_pDummyItem->sBasicOp.dwFlags&ITEM_CHANNEL_ALL );
	SetWin_Check ( this, IDC_CHECK_CONSUMABLE , m_pDummyItem->sDrugOp.bInstance );

	SetWin_Check ( this, IDC_CHECK_FLAG_EX_1, m_pDummyItem->sBasicOp.dwFlags&ITEM_FLAG_EX_1 );
	SetWin_Check ( this, IDC_CHECK_FLAG_EX_2, m_pDummyItem->sBasicOp.dwFlags&ITEM_FLAG_EX_2 );
	SetWin_Check ( this, IDC_CHECK_FLAG_EX_3, m_pDummyItem->sBasicOp.dwFlags&ITEM_FLAG_EX_3 );
	SetWin_Check ( this, IDC_CHECK_FLAG_EX_4, m_pDummyItem->sBasicOp.dwFlags&ITEM_FLAG_EX_4 );
	
	SetWin_Check ( this, IDC_CHECK_FLAG_EX_6, m_pDummyItem->sBasicOp.dwFlags&ITEM_FLAG_EX_5 );

	SetWin_Combo_Sel( this, IDC_COMBO_BRIGHT, COMMENT::BRIGHT[m_pDummyItem->sBasicOp.emReqBright] );
	SetWin_Num_int( this, IDC_EDIT_REQ_POW, m_pDummyItem->sBasicOp.sReqStats.wPow );
	SetWin_Num_int( this, IDC_EDIT_REQ_DEX, m_pDummyItem->sBasicOp.sReqStats.wDex );
	SetWin_Num_int( this, IDC_EDIT_REQ_SPI, m_pDummyItem->sBasicOp.sReqStats.wSpi );
	SetWin_Num_int( this, IDC_EDIT_REQ_STR, m_pDummyItem->sBasicOp.sReqStats.wStr );
	SetWin_Num_int( this, IDC_EDIT_REQ_STA, m_pDummyItem->sBasicOp.sReqStats.wSta );
	SetWin_Num_int( this, IDC_EDIT_REQ_INT, m_pDummyItem->sBasicOp.sReqStats.wInt );
	SetWin_Num_int( this, IDC_EDIT_REQ_PA, m_pDummyItem->sBasicOp.wReqPA );
	SetWin_Num_int( this, IDC_EDIT_REQ_SA, m_pDummyItem->sBasicOp.wReqSA );
	SetWin_Num_int( this, IDC_EDIT_REQ_LEVEL_MIN, m_pDummyItem->sBasicOp.wReqLevelDW );
	SetWin_Num_int( this, IDC_EDIT_REQ_LEVEL_MAX, m_pDummyItem->sBasicOp.wReqLevelUP );
	SetWin_Num_int( this, IDC_EDIT_REQ_USERNUM, m_pDummyItem->sBasicOp.dwReqUserNum ); /*UserNum ItemReq, Juver, 2017/06/27 */

	SetWin_Check ( this, IDC_CHECK_REQ_SCHOOL_SG, m_pDummyItem->sBasicOp.dwReqSchool&GLSCHOOL_00 );
	SetWin_Check ( this, IDC_CHECK_REQ_SCHOOL_MP, m_pDummyItem->sBasicOp.dwReqSchool&GLSCHOOL_01 );
	SetWin_Check ( this, IDC_CHECK_REQ_SCHOOL_PHX, m_pDummyItem->sBasicOp.dwReqSchool&GLSCHOOL_02 );

	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_BRAWLER_M, m_pDummyItem->sBasicOp.dwReqCharClass&GLCC_BRAWLER_M );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_BRAWLER_F, m_pDummyItem->sBasicOp.dwReqCharClass&GLCC_BRAWLER_W );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SWORDSMAN_M, m_pDummyItem->sBasicOp.dwReqCharClass&GLCC_SWORDSMAN_M );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SWORDSMAN_F, m_pDummyItem->sBasicOp.dwReqCharClass&GLCC_SWORDSMAN_W );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ARCHER_M, m_pDummyItem->sBasicOp.dwReqCharClass&GLCC_ARCHER_M );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ARCHER_F, m_pDummyItem->sBasicOp.dwReqCharClass&GLCC_ARCHER_W );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SHAMAN_M, m_pDummyItem->sBasicOp.dwReqCharClass&GLCC_SHAMAN_M );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_SHAMAN_F, m_pDummyItem->sBasicOp.dwReqCharClass&GLCC_SHAMAN_W );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_EXTREME_M, m_pDummyItem->sBasicOp.dwReqCharClass&GLCC_EXTREME_M );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_EXTREME_F, m_pDummyItem->sBasicOp.dwReqCharClass&GLCC_EXTREME_W );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_GUNNER_M, m_pDummyItem->sBasicOp.dwReqCharClass&GLCC_GUNNER_M );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_GUNNER_F, m_pDummyItem->sBasicOp.dwReqCharClass&GLCC_GUNNER_W );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ASSASSIN_M, m_pDummyItem->sBasicOp.dwReqCharClass&GLCC_ASSASSIN_M );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_ASSASSIN_F, m_pDummyItem->sBasicOp.dwReqCharClass&GLCC_ASSASSIN_W );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_TRICKER_M, m_pDummyItem->sBasicOp.dwReqCharClass&GLCC_TRICKER_M );
	SetWin_Check ( this, IDC_CHECK_REQ_CLASS_TRICKER_F, m_pDummyItem->sBasicOp.dwReqCharClass&GLCC_TRICKER_W );

	SetWin_Text( this, IDC_EDIT_WEAR_BRAWLER_M, m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_BRAWLER_M].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_BRAWLER_F, m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_BRAWLER_W].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SWORDSMAN_M, m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_SWORDSMAN_M].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SWORDSMAN_F, m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_SWORDSMAN_W].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ARCHER_M, m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_ARCHER_M].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ARCHER_F, m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_ARCHER_W].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SHAMAN_M, m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_SHAMAN_M].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SHAMAN_F, m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_SHAMAN_W].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_EXTREME_M, m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_EXTREME_M].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_EXTREME_F, m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_EXTREME_W].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_GUNNER_M, m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_GUNNER_M].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_GUNNER_F, m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_GUNNER_W].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ASSASSIN_M, m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_ASSASSIN_M].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ASSASSIN_F, m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_ASSASSIN_W].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_TRICKER_M, m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_TRICKER_M].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_TRICKER_F, m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_TRICKER_W].c_str() );
	SetWin_Text( this, IDC_EDIT_APPLY_M, "" );
	SetWin_Text( this, IDC_EDIT_APPLY_F, "" );

	SetWin_Text( this, IDC_EDIT_WEAR_BRAWLER_M2, m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_BRAWLER_M].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_BRAWLER_F2, m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_BRAWLER_W].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SWORDSMAN_M2, m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_SWORDSMAN_M].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SWORDSMAN_F2, m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_SWORDSMAN_W].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ARCHER_M2, m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_ARCHER_M].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ARCHER_F2, m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_ARCHER_W].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SHAMAN_M2, m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_SHAMAN_M].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_SHAMAN_F2, m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_SHAMAN_W].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_EXTREME_M2, m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_EXTREME_M].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_EXTREME_F2, m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_EXTREME_W].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_GUNNER_M2, m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_GUNNER_M].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_GUNNER_F2, m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_GUNNER_W].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ASSASSIN_M2, m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_ASSASSIN_M].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_ASSASSIN_F2, m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_ASSASSIN_W].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_TRICKER_M2, m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_TRICKER_M].c_str() );
	SetWin_Text( this, IDC_EDIT_WEAR_TRICKER_F2, m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_TRICKER_W].c_str() );
	SetWin_Text( this, IDC_EDIT_APPLY_M2, "" );
	SetWin_Text( this, IDC_EDIT_APPLY_F2, "" );

	SetWin_Text( this, IDC_EDIT_EFFECT_SELFBODY, m_pDummyItem->sBasicOp.strSelfBodyEffect.c_str() );
	SetWin_Text( this, IDC_EDIT_EFFECT_TARGETBODY, m_pDummyItem->sBasicOp.strTargBodyEffect.c_str() );
	SetWin_Text( this, IDC_EDIT_EFFECT_TARGET, m_pDummyItem->sBasicOp.strTargetEffect.c_str() );
	SetWin_Text( this, IDC_EDIT_PETWEAR, m_pDummyItem->sBasicOp.strPetWearingFile.c_str() );
	SetWin_Text( this, IDC_EDIT_RANDOM_OPTION, m_pDummyItem->sRandomOpt.szNAME );
	SetWin_Text( this, IDC_EDIT_INVENTORY, m_pDummyItem->sBasicOp.strInventoryFile.c_str() );
	SetWin_Text( this, IDC_EDIT_FIELD, m_pDummyItem->sBasicOp.strFieldFile.c_str() );

	SetWin_Num_int( this, IDC_EDIT_INVEN_SIZE_X, m_pDummyItem->sBasicOp.wInvenSizeX );
	SetWin_Num_int( this, IDC_EDIT_INVEN_SIZE_Y, m_pDummyItem->sBasicOp.wInvenSizeY );
	SetWin_Num_int( this, IDC_EDIT_ICON_MID, m_pDummyItem->sBasicOp.sICONID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_ICON_SID, m_pDummyItem->sBasicOp.sICONID.wSubID );

	SetWin_Combo_Sel( this, IDC_COMBO_COOLDOWN_TYPE, COMMENT::COOLTYPE[m_pDummyItem->sBasicOp.emCoolType] );
	SetWin_Num_int( this, IDC_EDIT_COOLDOWN_VALUE, m_pDummyItem->sBasicOp.dwCoolTime );

	SetWin_Num_int( this, IDC_EDIT_GEN_SPECID, m_pDummyItem->sGenerateOp.dwSpecID );
	SetWin_Num_int( this, IDC_EDIT_GEN_LIMITTIME, m_pDummyItem->sGenerateOp.dwLimitTime );
	SetWin_Num_int( this, IDC_EDIT_GEN_LIMITTIMEGEN, m_pDummyItem->sGenerateOp.dwLimitTimeGen);
	SetWin_Check( this, IDC_CHECK_ENABLE_SPEC, m_pDummyItem->sGenerateOp.bGenEnable );

	CTimeSpan cSPAN(m_pDummyItem->sDrugOp.tTIME_LMT);
	SetWin_Num_int ( this, IDC_EDIT_EXPIRY_D, (int)cSPAN.GetDays() );
	SetWin_Num_int ( this, IDC_EDIT_EXPIRY_H, cSPAN.GetHours() );
	SetWin_Num_int ( this, IDC_EDIT_EXPIRY_M, cSPAN.GetMinutes() );

	SetWin_Combo_Sel( this, IDC_COMBO_PET, int(m_pDummyItem->sPet.emPetType) );
	SetWin_Num_int ( this, IDC_EDIT_PET_MID, m_pDummyItem->sPet.sPetID.wMainID );
	SetWin_Num_int ( this, IDC_EDIT_PET_SID, m_pDummyItem->sPet.sPetID.wSubID );

	SetWin_Combo_Sel( this, IDC_COMBO_VEHICLE, int( m_pDummyItem->sVehicle.emVehicleType) );
	SetWin_Combo_Sel ( this, IDC_COMBO_VEHICLE2, (int)m_pDummyItem->sVehicle.emVehicleSubType );
	SetWin_Check ( this, IDC_CHECK_VEHICLE_NOACC, m_pDummyItem->sVehicle.bNoAcc );

	/*bike color , Juver, 2017/11/13 */
	SetWin_Num_int ( this, IDC_EDIT_VEHICLE_COLOR_1, m_pDummyItem->sVehicle.wDefaultColor1 );
	SetWin_Num_int ( this, IDC_EDIT_VEHICLE_COLOR_2, m_pDummyItem->sVehicle.wDefaultColor2 );

	SetWin_Num_int ( this, IDC_EDIT_SKILL_MID, m_pDummyItem->sSkillBookOp.sSkill_ID.wMainID );
	SetWin_Num_int ( this, IDC_EDIT_SKILL_SID, m_pDummyItem->sSkillBookOp.sSkill_ID.wSubID );

	SetWin_Num_int ( this, IDC_EDIT_MAP_MID, m_pDummyItem->sBasicOp.sSubID.wMainID );
	SetWin_Num_int ( this, IDC_EDIT_MAP_SID, m_pDummyItem->sBasicOp.sSubID.wSubID );
	SetWin_Num_int ( this, IDC_EDIT_MAP_MID2, m_pDummyItem->sBasicOp.sSubID.wMainID );
	SetWin_Num_int ( this, IDC_EDIT_MAP_SID2, m_pDummyItem->sBasicOp.sSubID.wSubID );
	SetWin_Num_int ( this, IDC_EDIT_MAP_POSX, m_pDummyItem->sBasicOp.wPosX );
	SetWin_Num_int ( this, IDC_EDIT_MAP_POSY, m_pDummyItem->sBasicOp.wPosY);

	/*activity point, Juver, 2017/08/23 */
	SetWin_Num_int ( this, IDC_EDIT_ACTIVITY_POINT_REQ, m_pDummyItem->sBasicOp.dwReqActivityPoint );

	/*contribution point, Juver, 2017/08/23 */
	SetWin_Num_int ( this, IDC_EDIT_CONTRIBUTION_POINT_REQ, m_pDummyItem->sBasicOp.dwReqContributionPoint );

	SetWin_Check( this, IDC_CHECK_FLAG_EX_5, m_pDummyItem->sBox.bData );

	InitType();
	CheckPetCrow();
	CheckSkillScroll();
	CheckNpcRecCrow();
	CheckSpecMap();
	InitVehicleTypes();

	SetWin_Num_int ( this, IDC_EDIT_SUIT_DUMMY_1, m_pDummyItem->sSuitOp.dummy_var_0);
	SetWin_Num_int ( this, IDC_EDIT_SUIT_DUMMY_2, m_pDummyItem->sSuitOp.dummy_var_1);
	SetWin_Num_int ( this, IDC_EDIT_SUIT_DUMMY_3, m_pDummyItem->sSuitOp.dummy_var_2);

	/*item color, Juver, 2018/01/05 */
	SetWin_Check ( this, IDC_ITEM_COLOR_USE, m_pDummyItem->sBasicOp.bItemColor);
	SetWin_Num_int ( this, IDC_EDIT_ITEM_COLOR_1, m_pDummyItem->sBasicOp.wItemColor1 );
	SetWin_Num_int ( this, IDC_EDIT_ITEM_COLOR_2, m_pDummyItem->sBasicOp.wItemColor2 );

	/*item transfer card, Juver, 2018/01/18 */
	SetWin_Check ( this, IDC_CHECK_ITEM_TRANSFER, m_pDummyItem->sBasicOp.bItemTransfer );

	return TRUE;
}

BOOL CPageEdit1::DataSave ()
{
	m_pDummyItem->sBasicOp.sGroupID.wMainID = GetWin_Num_int( this, IDC_EDIT_GROUP_MID );
	m_pDummyItem->sBasicOp.sGroupID.wSubID = GetWin_Num_int( this, IDC_EDIT_GROUP_SID );
	m_pDummyItem->sBasicOp.strName = GetWin_Text( this, IDC_EDIT_NAME );
	m_pDummyItem->sBasicOp.strComment = GetWin_Text( this, IDC_EDIT_COMMENT );

	m_pDummyItem->sBasicOp.emLevel = static_cast<EMITEMLEVEL>( GetWin_Combo_Sel( this, IDC_COMBO_GRADE ) );
	m_pDummyItem->sBasicOp.emItemType = static_cast<EMITEM_TYPE>( GetWin_Combo_Sel( this, IDC_COMBO_ITEMTYPE ) );

	m_pDummyItem->sBasicOp.dwBuyPrice = GetWin_Num_int( this, IDC_EDIT_PRICE_BUY );
	m_pDummyItem->sBasicOp.dwSellPrice = GetWin_Num_int( this, IDC_EDIT_PRICE_SELL );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_FLAG_SALE ), m_pDummyItem->sBasicOp.dwFlags, TRADE_SALE );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_FLAG_EXCHANGE ), m_pDummyItem->sBasicOp.dwFlags, TRADE_EXCHANGE );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_FLAG_THROW ), m_pDummyItem->sBasicOp.dwFlags, TRADE_THROW );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_FLAG_DISGUISE ), m_pDummyItem->sBasicOp.dwFlags, ITEM_DISGUISE );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_FLAG_TIMELMT ), m_pDummyItem->sBasicOp.dwFlags, ITEM_TIMELMT );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_FLAG_GARBAGE ), m_pDummyItem->sBasicOp.dwFlags, TRADE_GARBAGE );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_FLAG_EVENTITEM ), m_pDummyItem->sBasicOp.dwFlags, TRADE_EVENT_SGL );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_FLAG_CHANNELALL ), m_pDummyItem->sBasicOp.dwFlags, ITEM_CHANNEL_ALL );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_FLAG_EX_1 ), m_pDummyItem->sBasicOp.dwFlags, ITEM_FLAG_EX_1 );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_FLAG_EX_2 ), m_pDummyItem->sBasicOp.dwFlags, ITEM_FLAG_EX_2 );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_FLAG_EX_3 ), m_pDummyItem->sBasicOp.dwFlags, ITEM_FLAG_EX_3 );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_FLAG_EX_4 ), m_pDummyItem->sBasicOp.dwFlags, ITEM_FLAG_EX_4 );

	
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_FLAG_EX_6 ), m_pDummyItem->sBasicOp.dwFlags, ITEM_FLAG_EX_5 );

	m_pDummyItem->sDrugOp.bInstance = GetWin_Check ( this, IDC_CHECK_CONSUMABLE );

	m_pDummyItem->sBasicOp.emReqBright = static_cast<EMBRIGHT>( GetWin_Combo_Sel( this, IDC_COMBO_BRIGHT ) );
	m_pDummyItem->sBasicOp.sReqStats.wPow = GetWin_Num_int( this, IDC_EDIT_REQ_POW );
	m_pDummyItem->sBasicOp.sReqStats.wDex = GetWin_Num_int( this, IDC_EDIT_REQ_DEX );
	m_pDummyItem->sBasicOp.sReqStats.wSpi = GetWin_Num_int( this, IDC_EDIT_REQ_SPI );
	m_pDummyItem->sBasicOp.sReqStats.wStr = GetWin_Num_int( this, IDC_EDIT_REQ_STR );
	m_pDummyItem->sBasicOp.sReqStats.wSta = GetWin_Num_int( this, IDC_EDIT_REQ_STA );
	m_pDummyItem->sBasicOp.sReqStats.wInt = GetWin_Num_int( this, IDC_EDIT_REQ_INT );
	m_pDummyItem->sBasicOp.wReqPA = GetWin_Num_int( this, IDC_EDIT_REQ_PA );
	m_pDummyItem->sBasicOp.wReqSA = GetWin_Num_int( this, IDC_EDIT_REQ_SA );
	m_pDummyItem->sBasicOp.wReqLevelDW = GetWin_Num_int( this, IDC_EDIT_REQ_LEVEL_MIN );
	m_pDummyItem->sBasicOp.wReqLevelUP = GetWin_Num_int( this, IDC_EDIT_REQ_LEVEL_MAX );
	m_pDummyItem->sBasicOp.dwReqUserNum = GetWin_Num_int( this, IDC_EDIT_REQ_USERNUM ); /*UserNum ItemReq, Juver, 2017/06/27 */

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_SCHOOL_SG ), m_pDummyItem->sBasicOp.dwReqSchool, GLSCHOOL_00 );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_SCHOOL_MP ), m_pDummyItem->sBasicOp.dwReqSchool, GLSCHOOL_01 );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_SCHOOL_PHX ), m_pDummyItem->sBasicOp.dwReqSchool, GLSCHOOL_02 );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_CLASS_BRAWLER_M ),m_pDummyItem->sBasicOp.dwReqCharClass, GLCC_BRAWLER_M );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_CLASS_BRAWLER_F ),m_pDummyItem->sBasicOp.dwReqCharClass, GLCC_BRAWLER_W );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_CLASS_SWORDSMAN_M ),m_pDummyItem->sBasicOp.dwReqCharClass, GLCC_SWORDSMAN_M );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_CLASS_SWORDSMAN_F ),m_pDummyItem->sBasicOp.dwReqCharClass, GLCC_SWORDSMAN_W );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_CLASS_ARCHER_M ),m_pDummyItem->sBasicOp.dwReqCharClass, GLCC_ARCHER_M );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_CLASS_ARCHER_F ),m_pDummyItem->sBasicOp.dwReqCharClass, GLCC_ARCHER_W );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_CLASS_SHAMAN_M ),m_pDummyItem->sBasicOp.dwReqCharClass, GLCC_SHAMAN_M );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_CLASS_SHAMAN_F ),m_pDummyItem->sBasicOp.dwReqCharClass, GLCC_SHAMAN_W );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_CLASS_EXTREME_M ),m_pDummyItem->sBasicOp.dwReqCharClass, GLCC_EXTREME_M );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_CLASS_EXTREME_F ),m_pDummyItem->sBasicOp.dwReqCharClass, GLCC_EXTREME_W );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_CLASS_GUNNER_M ),m_pDummyItem->sBasicOp.dwReqCharClass, GLCC_GUNNER_M );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_CLASS_GUNNER_F ),m_pDummyItem->sBasicOp.dwReqCharClass, GLCC_GUNNER_W );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_CLASS_ASSASSIN_M ),m_pDummyItem->sBasicOp.dwReqCharClass, GLCC_ASSASSIN_M );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_CLASS_ASSASSIN_F ),m_pDummyItem->sBasicOp.dwReqCharClass, GLCC_ASSASSIN_W );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_CLASS_TRICKER_M ),m_pDummyItem->sBasicOp.dwReqCharClass, GLCC_TRICKER_M );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REQ_CLASS_TRICKER_F ),m_pDummyItem->sBasicOp.dwReqCharClass, GLCC_TRICKER_W );

	m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_BRAWLER_M] = GetWin_Text( this, IDC_EDIT_WEAR_BRAWLER_M ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_BRAWLER_W] = GetWin_Text( this, IDC_EDIT_WEAR_BRAWLER_F ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_SWORDSMAN_M] = GetWin_Text( this, IDC_EDIT_WEAR_SWORDSMAN_M ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_SWORDSMAN_W] = GetWin_Text( this, IDC_EDIT_WEAR_SWORDSMAN_F ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_ARCHER_M] = GetWin_Text( this, IDC_EDIT_WEAR_ARCHER_M ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_ARCHER_W] = GetWin_Text( this, IDC_EDIT_WEAR_ARCHER_F ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_SHAMAN_M] = GetWin_Text( this, IDC_EDIT_WEAR_SHAMAN_M ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_SHAMAN_W] = GetWin_Text( this, IDC_EDIT_WEAR_SHAMAN_F ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_EXTREME_M] = GetWin_Text( this, IDC_EDIT_WEAR_EXTREME_M ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_EXTREME_W] = GetWin_Text( this, IDC_EDIT_WEAR_EXTREME_F ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_GUNNER_M] = GetWin_Text( this, IDC_EDIT_WEAR_GUNNER_M ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_GUNNER_W] = GetWin_Text( this, IDC_EDIT_WEAR_GUNNER_F ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_ASSASSIN_M] = GetWin_Text( this, IDC_EDIT_WEAR_ASSASSIN_M ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_ASSASSIN_W] = GetWin_Text( this, IDC_EDIT_WEAR_ASSASSIN_F ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_TRICKER_M] = GetWin_Text( this, IDC_EDIT_WEAR_TRICKER_M ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileRight[GLCI_TRICKER_W] = GetWin_Text( this, IDC_EDIT_WEAR_TRICKER_F ).GetString();

	m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_BRAWLER_M] = GetWin_Text( this, IDC_EDIT_WEAR_BRAWLER_M2 ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_BRAWLER_W] = GetWin_Text( this, IDC_EDIT_WEAR_BRAWLER_F2 ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_SWORDSMAN_M] = GetWin_Text( this, IDC_EDIT_WEAR_SWORDSMAN_M2 ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_SWORDSMAN_W] = GetWin_Text( this, IDC_EDIT_WEAR_SWORDSMAN_F2 ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_ARCHER_M] = GetWin_Text( this, IDC_EDIT_WEAR_ARCHER_M2 ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_ARCHER_W] = GetWin_Text( this, IDC_EDIT_WEAR_ARCHER_F2 ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_SHAMAN_M] = GetWin_Text( this, IDC_EDIT_WEAR_SHAMAN_M2 ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_SHAMAN_W] = GetWin_Text( this, IDC_EDIT_WEAR_SHAMAN_F2 ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_EXTREME_M] = GetWin_Text( this, IDC_EDIT_WEAR_EXTREME_M2 ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_EXTREME_W] = GetWin_Text( this, IDC_EDIT_WEAR_EXTREME_F2 ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_GUNNER_M] = GetWin_Text( this, IDC_EDIT_WEAR_GUNNER_M2 ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_GUNNER_W] = GetWin_Text( this, IDC_EDIT_WEAR_GUNNER_F2 ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_ASSASSIN_M] = GetWin_Text( this, IDC_EDIT_WEAR_ASSASSIN_M2 ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_ASSASSIN_W] = GetWin_Text( this, IDC_EDIT_WEAR_ASSASSIN_F2 ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_TRICKER_M] = GetWin_Text( this, IDC_EDIT_WEAR_TRICKER_M2 ).GetString();
	m_pDummyItem->sBasicOp.strWearingFileLeft[GLCI_TRICKER_W] = GetWin_Text( this, IDC_EDIT_WEAR_TRICKER_F2 ).GetString();

	m_pDummyItem->sBasicOp.strSelfBodyEffect = GetWin_Text( this, IDC_EDIT_EFFECT_SELFBODY ).GetString();
	m_pDummyItem->sBasicOp.strTargBodyEffect = GetWin_Text( this, IDC_EDIT_EFFECT_TARGETBODY ).GetString();
	m_pDummyItem->sBasicOp.strTargetEffect = GetWin_Text( this, IDC_EDIT_EFFECT_TARGET ).GetString();
	m_pDummyItem->sBasicOp.strPetWearingFile = GetWin_Text( this, IDC_EDIT_PETWEAR ).GetString();
	StringCchCopy ( m_pDummyItem->sRandomOpt.szNAME, SRANDOM_DATA::NAME_LEN, GetWin_Text(this,IDC_EDIT_RANDOM_OPTION).GetString() );
	m_pDummyItem->sBasicOp.strInventoryFile = GetWin_Text( this, IDC_EDIT_INVENTORY ).GetString();
	m_pDummyItem->sBasicOp.strFieldFile = GetWin_Text( this, IDC_EDIT_FIELD ).GetString();

	m_pDummyItem->sBasicOp.wInvenSizeX = GetWin_Num_int( this, IDC_EDIT_INVEN_SIZE_X );
	m_pDummyItem->sBasicOp.wInvenSizeY = GetWin_Num_int( this, IDC_EDIT_INVEN_SIZE_Y );
	m_pDummyItem->sBasicOp.sICONID.wMainID = GetWin_Num_int( this, IDC_EDIT_ICON_MID );
	m_pDummyItem->sBasicOp.sICONID.wSubID = GetWin_Num_int( this, IDC_EDIT_ICON_SID );

	m_pDummyItem->sBasicOp.emCoolType = static_cast<EMCOOL_TYPE>( GetWin_Combo_Sel( this, IDC_COMBO_COOLDOWN_TYPE ) );
	m_pDummyItem->sBasicOp.dwCoolTime = GetWin_Num_int( this, IDC_EDIT_COOLDOWN_VALUE );

	m_pDummyItem->sGenerateOp.dwSpecID = GetWin_Num_int( this, IDC_EDIT_GEN_SPECID );
	m_pDummyItem->sGenerateOp.dwLimitTime = GetWin_Num_int( this, IDC_EDIT_GEN_LIMITTIME );
	m_pDummyItem->sGenerateOp.dwLimitTimeGen = GetWin_Num_int( this, IDC_EDIT_GEN_LIMITTIMEGEN );
	m_pDummyItem->sGenerateOp.bGenEnable = GetWin_Check( this, IDC_CHECK_ENABLE_SPEC );

	LONG lDays = GetWin_Num_int ( this, IDC_EDIT_EXPIRY_D );
	int nHours = GetWin_Num_int ( this, IDC_EDIT_EXPIRY_H );
	int nMins = GetWin_Num_int ( this, IDC_EDIT_EXPIRY_M );
	CTimeSpan cSPAN( lDays, nHours, nMins, 0 );
	m_pDummyItem->sDrugOp.tTIME_LMT = cSPAN.GetTimeSpan();

	m_pDummyItem->sPet.emPetType = static_cast<PETTYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_PET ) );
	m_pDummyItem->sPet.sPetID.wMainID = GetWin_Num_int ( this, IDC_EDIT_PET_MID );
	m_pDummyItem->sPet.sPetID.wSubID = GetWin_Num_int ( this, IDC_EDIT_PET_SID );

	m_pDummyItem->sVehicle.emVehicleType = static_cast<VEHICLE_TYPE> ( GetWin_Combo_Sel( this, IDC_COMBO_VEHICLE ) );
	m_pDummyItem->sVehicle.emVehicleSubType = static_cast<VEHICLE_SUBTYPE> ( GetWin_Combo_Sel ( this, IDC_COMBO_VEHICLE2 ) );
	m_pDummyItem->sVehicle.bNoAcc = (BOOL) GetWin_Check ( this, IDC_CHECK_VEHICLE_NOACC );

	/*bike color , Juver, 2017/11/13 */
	m_pDummyItem->sVehicle.wDefaultColor1 = GetWin_Num_int ( this, IDC_EDIT_VEHICLE_COLOR_1 );
	m_pDummyItem->sVehicle.wDefaultColor2 = GetWin_Num_int ( this, IDC_EDIT_VEHICLE_COLOR_2 );

	m_pDummyItem->sSkillBookOp.sSkill_ID.wMainID = GetWin_Num_int ( this, IDC_EDIT_SKILL_MID );
	m_pDummyItem->sSkillBookOp.sSkill_ID.wSubID = GetWin_Num_int ( this, IDC_EDIT_SKILL_SID );

	m_pDummyItem->sBasicOp.sSubID.wMainID = GetWin_Num_int ( this, IDC_EDIT_MAP_MID );
	m_pDummyItem->sBasicOp.sSubID.wSubID = GetWin_Num_int ( this, IDC_EDIT_MAP_SID );
	m_pDummyItem->sBasicOp.wPosX = GetWin_Num_int ( this, IDC_EDIT_MAP_POSX );
	m_pDummyItem->sBasicOp.wPosY = GetWin_Num_int ( this, IDC_EDIT_MAP_POSY );

	if ( m_pDummyItem->sBasicOp.emItemType == ITEM_NPC_RECALL )
	{
		m_pDummyItem->sBasicOp.sSubID.wMainID = GetWin_Num_int ( this, IDC_EDIT_MAP_MID2 );
		m_pDummyItem->sBasicOp.sSubID.wSubID = GetWin_Num_int ( this, IDC_EDIT_MAP_SID2 );
	}

	/*activity point, Juver, 2017/08/23 */
	m_pDummyItem->sBasicOp.dwReqActivityPoint = GetWin_Num_int ( this, IDC_EDIT_ACTIVITY_POINT_REQ );

	/*contribution point, Juver, 2017/08/23 */
	m_pDummyItem->sBasicOp.dwReqContributionPoint = GetWin_Num_int ( this, IDC_EDIT_CONTRIBUTION_POINT_REQ );

	m_pDummyItem->sBox.bData = GetWin_Check( this, IDC_CHECK_FLAG_EX_5 )? true:false;

	m_pDummyItem->sSuitOp.dummy_var_0 = GetWin_Num_int ( this, IDC_EDIT_SUIT_DUMMY_1 );
	m_pDummyItem->sSuitOp.dummy_var_1 = GetWin_Num_int ( this, IDC_EDIT_SUIT_DUMMY_2 );
	m_pDummyItem->sSuitOp.dummy_var_2 = GetWin_Num_int ( this, IDC_EDIT_SUIT_DUMMY_3 );

	/*item color, Juver, 2018/01/05 */
	m_pDummyItem->sBasicOp.bItemColor = GetWin_Check ( this, IDC_ITEM_COLOR_USE )? true:false;
	m_pDummyItem->sBasicOp.wItemColor1 = GetWin_Num_int ( this, IDC_EDIT_ITEM_COLOR_1 );
	m_pDummyItem->sBasicOp.wItemColor2 = GetWin_Num_int ( this, IDC_EDIT_ITEM_COLOR_2 );

	/*item transfer card, Juver, 2018/01/18 */
	m_pDummyItem->sBasicOp.bItemTransfer = GetWin_Check ( this, IDC_CHECK_ITEM_TRANSFER )? true:false;

	return TRUE;
}

void CPageEdit1::LoadWearingfile( int nID )
{
	CString szFilter = "(*.cps)|*.cps|(*.abl)|*.abl|(*.vcf)|*.vcf|(*.vps)|*.vps|";
	CFileDialog dlg(TRUE,".cps",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );

	dlg.m_ofn.lpstrInitialDir = DxSkinPieceContainer::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, nID, dlg.GetFileName().GetString() );
}

void CPageEdit1::InitVehicleTypes()
{
	int nSelect = GetWin_Combo_Sel ( this, IDC_COMBO_VEHICLE );
	int nLastSelected = GetWin_Combo_Sel ( this, IDC_COMBO_VEHICLE2 );

	switch ( nSelect )
	{
	case VEHICLE_TYPE_BOARD:
		{
			SetWin_Combo_Init ( this, IDC_COMBO_VEHICLE2, COMMENT::VEHICLE_SUBTYPE_HOVER, VEHICLE_SUBTYPE_SIZE );
		}break;
	case VEHICLE_TYPE_BIKE:
		{
			SetWin_Combo_Init ( this, IDC_COMBO_VEHICLE2, COMMENT::VEHICLE_SUBTYPE_BIKE, VEHICLE_SUBTYPE_SIZE );
		}break;

		/* vehicle car, cart, Juver, 2018/02/13 */
	case VEHICLE_TYPE_CAR:
		{
			SetWin_Combo_Init ( this, IDC_COMBO_VEHICLE2, COMMENT::VEHICLE_SUBTYPE_CAR, VEHICLE_SUBTYPE_SIZE );
		}break;

		/* vehicle car, cart, Juver, 2018/02/13 */
	case VEHICLE_TYPE_CART:
		{
			SetWin_Combo_Init ( this, IDC_COMBO_VEHICLE2, COMMENT::VEHICLE_SUBTYPE_CART, VEHICLE_SUBTYPE_SIZE );
		}break;
	};

	SetWin_Combo_Sel( this, IDC_COMBO_VEHICLE2, nLastSelected );
}