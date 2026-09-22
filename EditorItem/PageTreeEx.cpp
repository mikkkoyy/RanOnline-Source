// PageTree.cpp : implementation file
//

#include "stdafx.h"
#include "EditorItem.h"
#include "PageTree.h"
#include "GLStringTable.h"
#include "GLogicData.h"


const char* CPageTree::GetFormatName ( SITEM* pItem )
{
	GASSERT ( pItem );

	CString strCLASS;
	CString strSCHOOL;

	if ( pItem->sBasicOp.dwReqSchool==GLSCHOOL_ALL )
	{
		strSCHOOL = "All School";
	}
	else
	{
		if ( pItem->sBasicOp.dwReqSchool & GLSCHOOL_00)
		{
			if ( strSCHOOL.GetLength() > 0 )	strSCHOOL += " | ";
			strSCHOOL+=GLCONST_CHAR::strSCHOOLNAME[school2index(GLSCHOOL_00)].c_str();
		}
		if ( pItem->sBasicOp.dwReqSchool & GLSCHOOL_01)
		{
			if ( strSCHOOL.GetLength() > 0 )	strSCHOOL += " | ";
			strSCHOOL+=GLCONST_CHAR::strSCHOOLNAME[school2index(GLSCHOOL_01)].c_str();
		}
		if ( pItem->sBasicOp.dwReqSchool & GLSCHOOL_02)
		{
			if ( strSCHOOL.GetLength() > 0 )	strSCHOOL += " | ";
			strSCHOOL+=GLCONST_CHAR::strSCHOOLNAME[school2index(GLSCHOOL_02)].c_str();
		}						
	}

	if ( pItem->sBasicOp.dwReqCharClass==GLCC_NONE)
	{
		strCLASS = "Unusable";
	}
	else if ( pItem->sBasicOp.dwReqCharClass==GLCC_ALL_8CLASS)
	{
		strCLASS = "All Class";
	}
	else if ( pItem->sBasicOp.dwReqCharClass==GLCC_MAN_8CLASS )
	{
		strCLASS = "All Male Class";
	}
	else if ( pItem->sBasicOp.dwReqCharClass==GLCC_WOMAN_8CLASS )
	{
		strCLASS = "All Female Class";
	}
	else
	{
		if ( pItem->sBasicOp.dwReqCharClass&GLCC_BRAWLER_M && pItem->sBasicOp.dwReqCharClass&GLCC_BRAWLER_W )
		{
			if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
			strCLASS += COMMENT::CHARCLASS2[GLCI2_BRAWLER].c_str();	

		}else{
			if ( pItem->sBasicOp.dwReqCharClass&GLCC_BRAWLER_M )	
			{
				if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
				strCLASS += COMMENT::CHARCLASS[GLCI_BRAWLER_M].c_str();	
			}

			if ( pItem->sBasicOp.dwReqCharClass&GLCC_BRAWLER_W )	
			{
				if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
				strCLASS += COMMENT::CHARCLASS[GLCI_BRAWLER_W].c_str();
			}
		}

		if ( pItem->sBasicOp.dwReqCharClass&GLCC_SWORDSMAN_M && pItem->sBasicOp.dwReqCharClass&GLCC_SWORDSMAN_W )
		{
			if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
			strCLASS += COMMENT::CHARCLASS2[GLCI2_SWORDSMAN].c_str();	
		}else{
			if ( pItem->sBasicOp.dwReqCharClass&GLCC_SWORDSMAN_M )	
			{
				if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
				strCLASS += COMMENT::CHARCLASS[GLCI_SWORDSMAN_M].c_str();	
			}

			if ( pItem->sBasicOp.dwReqCharClass&GLCC_SWORDSMAN_W )	
			{
				if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
				strCLASS += COMMENT::CHARCLASS[GLCI_SWORDSMAN_W].c_str();	
			}
		}

		if ( pItem->sBasicOp.dwReqCharClass&GLCC_ARCHER_M && pItem->sBasicOp.dwReqCharClass&GLCC_ARCHER_W )
		{
			if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
			strCLASS += COMMENT::CHARCLASS2[GLCI2_ARCHER].c_str();	
		}else{
			if ( pItem->sBasicOp.dwReqCharClass&GLCC_ARCHER_M )	
			{
				if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
				strCLASS += COMMENT::CHARCLASS[GLCI_ARCHER_M].c_str();	
			}

			if ( pItem->sBasicOp.dwReqCharClass&GLCC_ARCHER_W )	
			{
				if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
				strCLASS += COMMENT::CHARCLASS[GLCI_ARCHER_W].c_str();	
			}
		}

		if ( pItem->sBasicOp.dwReqCharClass&GLCC_SHAMAN_M && pItem->sBasicOp.dwReqCharClass&GLCC_SHAMAN_W )
		{
			if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
			strCLASS += COMMENT::CHARCLASS2[GLCI2_SHAMAN].c_str();	
		}else{
			if ( pItem->sBasicOp.dwReqCharClass&GLCC_SHAMAN_M )	
			{
				if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
				strCLASS += COMMENT::CHARCLASS[GLCI_SHAMAN_M].c_str();	
			}

			if ( pItem->sBasicOp.dwReqCharClass&GLCC_SHAMAN_W )	
			{
				if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
				strCLASS += COMMENT::CHARCLASS[GLCI_SHAMAN_W].c_str();	
			}
		}

		if ( pItem->sBasicOp.dwReqCharClass&GLCC_EXTREME_M && pItem->sBasicOp.dwReqCharClass&GLCC_EXTREME_W )
		{
			if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
			strCLASS += COMMENT::CHARCLASS2[GLCI2_EXTREME].c_str();	
		}else{
			if ( pItem->sBasicOp.dwReqCharClass&GLCC_EXTREME_M )	
			{
				if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
				strCLASS += COMMENT::CHARCLASS[GLCI_EXTREME_M].c_str();	
			}

			if ( pItem->sBasicOp.dwReqCharClass&GLCC_EXTREME_W )	
			{
				if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
				strCLASS += COMMENT::CHARCLASS[GLCI_EXTREME_W].c_str();
			}
		}

		if ( pItem->sBasicOp.dwReqCharClass&GLCC_GUNNER_M && pItem->sBasicOp.dwReqCharClass&GLCC_GUNNER_W )
		{
			if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
			strCLASS += COMMENT::CHARCLASS2[GLCI2_GUNNER].c_str();	
		}else{
			if ( pItem->sBasicOp.dwReqCharClass&GLCC_GUNNER_M )	
			{
				if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
				strCLASS += COMMENT::CHARCLASS[GLCI_GUNNER_M].c_str();	
			}

			if ( pItem->sBasicOp.dwReqCharClass&GLCC_GUNNER_W )	
			{
				if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
				strCLASS += COMMENT::CHARCLASS[GLCI_GUNNER_W].c_str();
			}
		}

		if ( pItem->sBasicOp.dwReqCharClass&GLCC_ASSASSIN_M && pItem->sBasicOp.dwReqCharClass&GLCC_ASSASSIN_W )
		{
			if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
			strCLASS += COMMENT::CHARCLASS2[GLCI2_ASSASSIN].c_str();	
		}else{
			if ( pItem->sBasicOp.dwReqCharClass&GLCC_ASSASSIN_M )	
			{
				if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
				strCLASS += COMMENT::CHARCLASS[GLCI_ASSASSIN_M].c_str();	
			}

			if ( pItem->sBasicOp.dwReqCharClass&GLCC_ASSASSIN_W )	
			{
				if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
				strCLASS += COMMENT::CHARCLASS[GLCI_ASSASSIN_W].c_str();
			}
		}

		if ( pItem->sBasicOp.dwReqCharClass&GLCC_TRICKER_M && pItem->sBasicOp.dwReqCharClass&GLCC_TRICKER_W )
		{
			if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
			strCLASS += COMMENT::CHARCLASS2[GLCI2_TRICKER].c_str();	
		}else{
			if ( pItem->sBasicOp.dwReqCharClass&GLCC_TRICKER_M )	
			{
				if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
				strCLASS += COMMENT::CHARCLASS[GLCI_TRICKER_M].c_str();	
			}

			if ( pItem->sBasicOp.dwReqCharClass&GLCC_TRICKER_W )	
			{
				if ( strCLASS.GetLength() > 0 )	strCLASS += " | ";
				strCLASS += COMMENT::CHARCLASS[GLCI_TRICKER_W].c_str();
			}
		}
	}

	//show some important information in treeview
	CString strDESC;
	switch ( pItem->sBasicOp.emItemType )
	{
	case ITEM_SUIT:
	case ITEM_ARROW:
	case ITEM_CHARM:
	case ITEM_ANTI_DISAPPEAR:
	case ITEM_BULLET: /*gun-bullet logic, Juver, 2017/05/27 */
		{
			strDESC.Format( "Suit:%s Attack:%s", COMMENT::ITEMSUIT[pItem->sSuitOp.emSuit].c_str(), COMMENT::ITEMATTACK[pItem->sSuitOp.emAttack].c_str() );
		}break;

	case ITEM_CURE:
		{
			EMITEM_DRUG emDRUG = pItem->sDrugOp.emDrug;
			if ( emDRUG == ITEM_DRUG_HP || emDRUG == ITEM_DRUG_MP || emDRUG == ITEM_DRUG_SP || emDRUG == ITEM_DRUG_HP_MP ||	emDRUG == ITEM_DRUG_MP_SP ||
				emDRUG == ITEM_DRUG_HP_MP_SP || emDRUG == ITEM_DRUG_HP_CURE || emDRUG == ITEM_DRUG_HP_MP_SP_CURE || 
				/*combatpoint logic, Juver, 2017/05/28 */
				emDRUG == ITEM_DRUG_CP )
			{
				strDESC.Format( "%s %d %s", COMMENT::ITEMDRUG[emDRUG].c_str(), pItem->sDrugOp.wCureVolume, pItem->sDrugOp.bRatio? "Percent":"" );
			}else{
				strDESC.Format( "%s", "Invalid Cure Type" );
			}
		}break;

	case ITEM_SKILL:
	case ITEM_PET_SKILL:
		{
			PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( pItem->sSkillBookOp.sSkill_ID );
			strDESC.Format( "%s", pSKILL? pSKILL->GetName() : "Unknown Skill" );
		}break;

	case ITEM_RECALL:
		{
			EMITEM_DRUG emDRUG = pItem->sDrugOp.emDrug;
			if ( emDRUG == ITEM_DRUG_CALL_SCHOOL || emDRUG == ITEM_DRUG_CALL_REGEN || emDRUG == ITEM_DRUG_CALL_LASTCALL )
			{
				strDESC.Format( "%s", COMMENT::ITEMDRUG[emDRUG].c_str() );
			}else{
				strDESC.Format( "%s", "Invalid Recall Type" );
			}
		}break;

	case ITEM_GRINDING:
		{
			strDESC.Format( "Class:%s | Level:%s | Type:%s", 
				COMMENT::GRINDING_CLASS[pItem->sGrindingOp.emCLASS].c_str(), 
				COMMENT::GRINDING_LEVEL[pItem->sGrindingOp.emLEVEL].c_str(),
				COMMENT::GRINDING_TYPE[pItem->sGrindingOp.emTYPE].c_str() );
		}break;

	case ITEM_SKP_STAT_RESET:
		{
			strDESC.Format( "Reset Skills and %d Stats", pItem->sDrugOp.wCureVolume );
		}break;

	case ITEM_PET_CARD:
		{
			SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData( pItem->sPet.sPetID );
			strDESC.Format( "Pet Type:%s / Pet CrowID:[%04d/%04d]%s", 
				COMMENT::PET_TYPE[pItem->sPet.emPetType].c_str(), 
				pItem->sPet.sPetID.wMainID, 
				pItem->sPet.sPetID.wSubID, 
				pCROW? pCROW->GetName() : "Invalid Crow" );
		}break;

	case ITEM_PET_FOOD:
	case ITEM_VEHICLE_OIL:
		{
			EMITEM_DRUG emDRUG = pItem->sDrugOp.emDrug;
			if ( emDRUG == ITEM_DRUG_HP )
			{
				strDESC.Format( "%s %d %s", COMMENT::ITEMDRUG[emDRUG].c_str(), pItem->sDrugOp.wCureVolume, pItem->sDrugOp.bRatio? "Percent":"" );
			}else{
				strDESC.Format( "%s", "Invalid Type" );
			}
		}break;

	case ITEM_VEHICLE:
		{
			strDESC.Format( "VehicleType:%s Suit:%s Attack:%s ", 
				COMMENT::VEHICLE_TYPE[pItem->sVehicle.emVehicleType].c_str(), 
				COMMENT::ITEMSUIT[pItem->sSuitOp.emSuit].c_str(),
				COMMENT::ITEMATTACK[pItem->sSuitOp.emAttack].c_str() );
		}break;

	case ITEM_TELEPORT_CARD:
		{
			SMAPNODE* pNODE = m_MapsList.FindMapNode( pItem->sBasicOp.sSubID );
			strDESC.Format( "Teleport to Map [%04d/%04d]%s", 
				pItem->sBasicOp.sSubID.wMainID,
				pItem->sBasicOp.sSubID.wSubID,
				pNODE? pNODE->strMapName.c_str() : "Unknown Map" );
		}break;

	case ITEM_NPC_RECALL:
		{
			SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData( pItem->sBasicOp.sSubID );
			strDESC.Format( "NPC CrowID:[%04d/%04d]%s", 
				pItem->sBasicOp.sSubID.wMainID, 
				pItem->sBasicOp.sSubID.wSubID, 
				pCROW? pCROW->GetName() : "Invalid Crow" );
		}break;

		//item types we dont have to add special information
		//case ITEM_KEY:
		//case ITEM_TICKET:
		//case ITEM_SKP_RESET:
		//case ITEM_STAT_RESET:
		//case ITEM_BOX:
		//case ITEM_PREMIUMSET:
		//case ITEM_RANDOMITEM:
		//case ITEM_CLEANSER:
		//case ITEM_LOUDSPEAKER:
		//case ITEM_FIRECRACKER:
		//case ITEM_CHARACTER_CARD:
		//case ITEM_INVEN_CARD:
		//case ITEM_STORAGE_CARD:
		//case ITEM_STORAGE_CONNECT:
		//case ITEM_PRIVATEMARKET:
		//case ITEM_DISJUNCTION:
		//case ITEM_HAIR:
		//case ITEM_FACE:
		//case ITEM_PET_RENAME:
		//case ITEM_PET_COLOR:
		//case ITEM_PET_STYLE:
		//case ITEM_SMS:
		//case ITEM_PET_REVIVE:
		//case ITEM_REMODEL:
		//case ITEM_VIETNAM_ITEMGET:
		//case ITEM_VIETNAM_EXPGET:
		//case ITEM_GENDER_CHANGE:
		//case ITEM_GARBAGE_CARD:
		//case ITEM_PET_SKIN_PACK:
		//case ITEM_FACE_STYLE:
		//case ITEM_TAXI_CARD:
		//case ITEM_MATERIALS:
	};

	CString strTimeLimit("No Expiry");

	if ( pItem->IsTIMELMT() )
	{
		CTimeSpan cSPAN(pItem->sDrugOp.tTIME_LMT);
		strTimeLimit.Format("D:%02d H:%02d M:%02d", (int)cSPAN.GetDays(), (int)cSPAN.GetHours(), (int)cSPAN.GetMinutes() );
	}

	static	CString strItem;

	strItem.Format ( "[%04d/%04d] %s            [ SpecID:%03d ] [ %s | %s ] [ %s ] [ %s ]", pItem->sBasicOp.sNativeID.wMainID, pItem->sBasicOp.sNativeID.wSubID, pItem->GetName(), 
		pItem->sGenerateOp.dwSpecID, strCLASS.GetString(), strSCHOOL.GetString(), strDESC.GetString(), strTimeLimit.GetString() );

	
	CString strNewName;
	strNewName.Format( "IN_%03d_%03d", pItem->sBasicOp.sNativeID.wMainID, pItem->sBasicOp.sNativeID.wSubID );
	pItem->sBasicOp.strName = strNewName;
	strNewName.Format( "ID_%03d_%03d", pItem->sBasicOp.sNativeID.wMainID, pItem->sBasicOp.sNativeID.wSubID );
	pItem->sBasicOp.strComment = strNewName;

	return strItem.GetString ();
}

BOOL CPageTree::AddItemMain( HTREEITEM hMainItem )
{
	WORD MID = GetSelectItem()->m_sItem.sBasicOp.sNativeID.wMainID;
	WORD SID = GLItemMan::GetInstance().FindFreeSID ( MID );
	if ( SID == ITEMID_NOTFOUND )	return FALSE;

	CItemNode sItemNode;
	sItemNode = *m_pDummyItemNode;
	sItemNode.m_sItem.sBasicOp.sNativeID.wMainID = MID;
	sItemNode.m_sItem.sBasicOp.sNativeID.wSubID = SID;

	char szNewName[ITEM_SZNAME];
	sprintf ( szNewName, "IN_%03d_%03d", MID, SID );
	sItemNode.m_sItem.sBasicOp.strName = szNewName;
	sprintf ( szNewName, "ID_%03d_%03d", MID, SID );
	sItemNode.m_sItem.sBasicOp.strComment = szNewName;
	GLItemMan::GetInstance().InsertItem ( MID, SID, &sItemNode );
	SITEM *pITEM = GLItemMan::GetInstance().GetItem ( MID, SID );

	HTREEITEM hParentItem = m_ctrlTree.GetParentItem ( hMainItem );
	if ( hParentItem != m_TreeRoot )
	{
		UpdateItem ( pITEM, hParentItem );
	}else{
		UpdateItem ( pITEM, hMainItem );
	}	

	return TRUE;
}

BOOL CPageTree::AddItemSub ()
{
	WORD MID = GLItemMan::GetInstance().FindFreeMID ();
	if ( MID == ITEMID_NOTFOUND )	return FALSE;
	WORD SID = GLItemMan::GetInstance().FindFreeSID ( MID );
	if ( SID == ITEMID_NOTFOUND )	return FALSE;

	CItemNode sItemNode;
	sItemNode = *m_pDummyItemNode;
	sItemNode.m_sItem.sBasicOp.sNativeID.wMainID = MID;
	sItemNode.m_sItem.sBasicOp.sNativeID.wSubID = SID;

	char szNewName[ITEM_SZNAME];
	sprintf ( szNewName, "IN_%03d_%03d", MID, SID );
	sItemNode.m_sItem.sBasicOp.strName = szNewName;
	sprintf ( szNewName, "ID_%03d_%03d", MID, SID );
	sItemNode.m_sItem.sBasicOp.strComment = szNewName;
	GLItemMan::GetInstance().InsertItem ( MID, SID, &sItemNode );
	SITEM *pITEM = GLItemMan::GetInstance().GetItem ( MID, SID );

	UpdateItem ( pITEM, NULL );

	return TRUE;
}

BOOL CPageTree::AddItem ()
{
	HTREEITEM hMainItem = m_ctrlTree.GetSelectedItem ();
	if ( hMainItem && GetSelectItem () )
	{		
		return AddItemMain( hMainItem );
	}else{
		return AddItemSub ();
	}

	return FALSE;
}

BOOL CPageTree::DelItem ()
{
	if ( GetSelectItem() )
	{
		WORD WID = GetSelectItem()->m_sItem.sBasicOp.sNativeID.wMainID;
		WORD SID = GetSelectItem()->m_sItem.sBasicOp.sNativeID.wSubID;

		if ( SID == 0 )	
		{
			for ( int i = 0; i < GLItemMan::MAX_SID; i++ )
			{
				GLStringTable::GetInstance().DeleteString( WID, i, GLStringTable::ITEM );
				GLItemMan::GetInstance().DeleteItem ( WID, i );
			}
		}else{
			GLStringTable::GetInstance().DeleteString( GetSelectItem()->m_sItem.sBasicOp.strName,
				GetSelectItem()->m_sItem.sBasicOp.strComment, GLStringTable::ITEM );
			GLItemMan::GetInstance().DeleteItem ( GetSelectItem()->m_sItem.sBasicOp.sNativeID );
		}		

		m_ctrlTree.DeleteItem ( m_ctrlTree.GetSelectedItem () );

		return TRUE;	
	}

	return FALSE;
}

BOOL CPageTree::CopyItem ()
{
	HTREEITEM hMainItem = m_ctrlTree.GetSelectedItem ();
	if ( hMainItem && hMainItem != m_TreeRoot )
	{
		*m_pDummyItemNode = *((PITEMNODE)m_ctrlTree.GetItemData ( hMainItem ));
		m_bDummyHasItem = TRUE;
		return TRUE;
	}

	return FALSE;
}

BOOL CPageTree::PasteItemNew ()
{
	if ( !m_bDummyHasItem )
	{
		return FALSE;
	}

	HTREEITEM hMainItem = m_ctrlTree.GetSelectedItem ();
	if ( hMainItem && GetSelectItem () )
	{
		WORD MID = GetSelectItem()->m_sItem.sBasicOp.sNativeID.wMainID;
		WORD SID = GLItemMan::GetInstance().FindFreeSID ( MID );
		if ( SID == ITEMID_NOTFOUND )	return FALSE;

		CItemNode sItemNode;
		sItemNode = *m_pDummyItemNode;
		sItemNode.m_sItem.sBasicOp.sNativeID.wMainID= MID;
		sItemNode.m_sItem.sBasicOp.sNativeID.wSubID = SID;
		CString strTemp;
		strTemp.Format( "IN_%03d_%03d" , MID , SID);
		sItemNode.m_sItem.sBasicOp.strName = strTemp.GetString();
		strTemp.Format( "ID_%03d_%03d" , MID , SID);
		sItemNode.m_sItem.sBasicOp.strComment = strTemp.GetString();
		GLItemMan::GetInstance().InsertItem ( MID, SID, &sItemNode );
		GLStringTable::GetInstance().DeleteString ( sItemNode.m_sItem.sBasicOp.strName, sItemNode.m_sItem.sBasicOp.strComment, GLStringTable::ITEM );
		GLStringTable::GetInstance().InsertString ( sItemNode.m_sItem.sBasicOp.strName, std::string( m_pDummyItemNode->m_sItem.GetName() ), GLStringTable::ITEM );
		const char* pComment =  m_pDummyItemNode->m_sItem.GetComment();
		std::string strComment = pComment? pComment: "";
		GLStringTable::GetInstance().InsertString ( sItemNode.m_sItem.sBasicOp.strComment, strComment, GLStringTable::ITEM );
		SITEM *pITEM = GLItemMan::GetInstance().GetItem ( MID, SID );

		HTREEITEM hParentItem = m_ctrlTree.GetParentItem ( hMainItem );
		if ( hParentItem != m_TreeRoot )
		{
			UpdateItem (pITEM, hParentItem );
		}else{
			UpdateItem ( pITEM, hMainItem );
		}

		return TRUE;
	}
	else
	{
		WORD MID = GLItemMan::GetInstance().FindFreeMID ();
		if ( MID == ITEMID_NOTFOUND )	return FALSE;
		WORD SID = GLItemMan::GetInstance().FindFreeSID ( MID );
		if ( SID == ITEMID_NOTFOUND )	return FALSE;

		CItemNode sItemNode;
		sItemNode = *m_pDummyItemNode;
		sItemNode.m_sItem.sBasicOp.sNativeID.wMainID = MID;
		sItemNode.m_sItem.sBasicOp.sNativeID.wSubID = SID;
		CString strTemp;
		strTemp.Format( "IN_%03d_%03d" , MID , SID);
		sItemNode.m_sItem.sBasicOp.strName = strTemp.GetString();
		strTemp.Format( "ID_%03d_%03d" , MID , SID);
		sItemNode.m_sItem.sBasicOp.strComment = strTemp.GetString();
		GLItemMan::GetInstance().InsertItem ( MID, SID, &sItemNode );
		GLStringTable::GetInstance().DeleteString ( sItemNode.m_sItem.sBasicOp.strName, sItemNode.m_sItem.sBasicOp.strComment, GLStringTable::ITEM );
		GLStringTable::GetInstance().InsertString ( sItemNode.m_sItem.sBasicOp.strName, std::string( m_pDummyItemNode->m_sItem.GetName() ), GLStringTable::ITEM );
		const char* pComment =  m_pDummyItemNode->m_sItem.GetComment();
		std::string strComment = pComment? pComment: "";
		GLStringTable::GetInstance().InsertString ( sItemNode.m_sItem.sBasicOp.strComment, strComment, GLStringTable::ITEM );
		SITEM *pITEM = GLItemMan::GetInstance().GetItem ( MID, SID );
		UpdateItem ( pITEM, NULL );

		return TRUE;
	}
}

BOOL CPageTree::PasteItemValue ()
{
	if ( !m_bDummyHasItem )	return FALSE;

	HTREEITEM hMainItem = m_ctrlTree.GetSelectedItem ();
	if ( hMainItem && GetSelectItem () )
	{		
		WORD MID = GetSelectItem()->m_sItem.sBasicOp.sNativeID.wMainID;
		WORD SID = GetSelectItem()->m_sItem.sBasicOp.sNativeID.wSubID;
		*(GetSelectItem()) = *m_pDummyItemNode;
		GetSelectItem()->m_sItem.sBasicOp.sNativeID.wMainID= MID;
		GetSelectItem()->m_sItem.sBasicOp.sNativeID.wSubID = SID;
		CString strTemp;
		strTemp.Format( "IN_%03d_%03d" , MID , SID);
		GetSelectItem()->m_sItem.sBasicOp.strName = strTemp.GetString();
		strTemp.Format( "ID_%03d_%03d" , MID , SID);
		GetSelectItem()->m_sItem.sBasicOp.strComment = strTemp.GetString();
		GLStringTable::GetInstance().DeleteString ( GetSelectItem()->m_sItem.sBasicOp.strName, GetSelectItem()->m_sItem.sBasicOp.strComment, GLStringTable::ITEM );
		GLStringTable::GetInstance().InsertString ( GetSelectItem()->m_sItem.sBasicOp.strName, std::string( m_pDummyItemNode->m_sItem.GetName() ), GLStringTable::ITEM );
		const char* pComment =  m_pDummyItemNode->m_sItem.GetComment();
		std::string strComment = pComment? pComment: "";
		GLStringTable::GetInstance().InsertString ( GetSelectItem()->m_sItem.sBasicOp.strComment, strComment, GLStringTable::ITEM );
		m_ctrlTree.SetItemText ( hMainItem, GetFormatName ( &GetSelectItem()->m_sItem ) );
		return TRUE;
	}
	return FALSE;
}