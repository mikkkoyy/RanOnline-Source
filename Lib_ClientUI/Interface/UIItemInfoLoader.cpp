#include "StdAfx.h"
#include "UIInfoLoader.h"
#include "GLGaeaClient.h"
#include "GLItemMan.h"
#include "GLItemDef.h"
#include "GLItem.h"
#include "GameTextControl.h"
#include "UITextControl.h"
#include "InnerInterface.h"
#include "BasicTextBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace NS_ITEMINFO
{
    SITEMCUSTOM	m_sItemCustomBACK;
	BOOL		m_bShopOpenBACK;
    BOOL		m_bInMarketBACK;
	BOOL		m_bInPrivateMarketBACK;
	BOOL		m_bIsWEAR_ITEMBACK;
	WORD		m_wPosXBACK;
	WORD		m_wPosYBACK;

	 /*npc shop, Juver, 2017/07/26 */
	BOOL		m_bNPCShopBACK;
	WORD		m_wNPCShopTypeBACK;

	/*item link, Juver, 2017/07/31 */
	BOOL		m_bItemLinkBACK;
	BOOL		m_bInInventoryBACK;

	/*product item, Juver, 2017/10/15 */
	BOOL		m_bInCompoundResultBACK;
	BOOL		m_bCompoundGenerateRandomOptionBACK;

	void	RESET ()
	{
		m_sItemCustomBACK.sNativeID = NATIVEID_NULL ();
		m_bShopOpenBACK = FALSE;
		m_bInMarketBACK = FALSE;
		m_bInPrivateMarketBACK = FALSE;
		m_bIsWEAR_ITEMBACK = FALSE;

		 /*npc shop, Juver, 2017/07/26 */
		m_bNPCShopBACK = FALSE;
		m_wNPCShopTypeBACK = 0;

		/*item link, Juver, 2017/07/31 */
		m_bItemLinkBACK = FALSE;
		m_bInInventoryBACK = FALSE;

		/*product item, Juver, 2017/10/15 */
		m_bInCompoundResultBACK = FALSE;
		m_bCompoundGenerateRandomOptionBACK = FALSE;

	}

	void AddTextNoSplit ( CString strText, D3DCOLOR dwColor )
	{
		CInnerInterface::GetInstance().ADDTEXT_NOSPLIT ( strText, dwColor );
	}

	void AddTextLongestLineSplit ( CString strText, D3DCOLOR dwColor )
	{
		CInnerInterface::GetInstance().ADDTEXT_LONGESTLINE_SPLIT ( strText, dwColor );
	}

	void AddMultiText ( CBasicTextBox::MULTITEXT strText )								
	{ 
		CInnerInterface::GetInstance().ADDMULTITEXT ( strText ); 
	}

	void AddMultiTextLongestLineSplit ( CBasicTextBox::MULTITEXT mlText )				
	{ 
		CInnerInterface::GetInstance().ADDMULTITEXTLONGESTLINESPLIT( mlText ); 
	}

	void RemoveAllInfo ()
	{
		CInnerInterface::GetInstance().CLEAR_TEXT ();
	}

	char*	GetNumberWithSign ( int nValue )
	{
		static	char szNumber[128];

		if ( 0 < nValue )
		{
			StringCchPrintf ( szNumber, 128, "(+%d)", nValue );
		}
		else if ( nValue < 0 )
		{
			StringCchPrintf ( szNumber, 128, "(%d)", nValue );
		}

		return szNumber;
	}

	void AddInfoItemAddon ( int nBasic, int nAddon, CString strFormat )
	{
		CString Text;
		if ( nAddon )
		{
			Text.Format ( "%s:%d%s", strFormat, nBasic, GetNumberWithSign ( nAddon ) );
			AddTextNoSplit ( Text, NS_UITEXTCOLOR::PRIVATE );
		}
		else if ( nBasic )
		{
			Text.Format ( "%s:%d", strFormat, nBasic );
			AddTextNoSplit ( Text, NS_UITEXTCOLOR::WHITE );
		}
	}

	void APPEND_ITEM_GRADE ( CString& strOrigin, BYTE uGRADE )
	{
		if ( !uGRADE ) return ;

		CString strGRADE;
		strGRADE.Format ( "[+%d]", uGRADE );
		strOrigin += strGRADE;
	}

	void AddInfoItemAddon ( int nBasic, int nAddon, BYTE uGRADE, CString strFormat )
	{
		CString Text;
		if ( nAddon )
		{
			Text.Format ( "%s:%d%s", strFormat, nBasic, GetNumberWithSign( nAddon ) );
			APPEND_ITEM_GRADE ( Text, uGRADE );
			AddTextNoSplit ( Text, NS_UITEXTCOLOR::PRIVATE );
		}
		else if ( nBasic )
		{
			Text.Format ( "%s:%d", strFormat, nBasic );
			APPEND_ITEM_GRADE ( Text, uGRADE );
			AddTextNoSplit ( Text, NS_UITEXTCOLOR::WHITE );
		}
	}
	
	void AddInfoItemAddon (  int nBasic, int nAddon, BYTE uGRADE,int nDirectDef, CString strFormat )
	{
		CString Text;
		CString sText;
		CString sText2;
		CString gText;
		CBasicTextBox::MULTITEXT mlText;

		gText.Format("[+%d]",uGRADE);	
		if ( nDirectDef && nAddon )
		{
			Text.Format ( "%s:%d%s", strFormat, nBasic,GetNumberWithSign( nAddon ));
			sText.Format ( "%s", GetNumberWithSign( nDirectDef ) );
			mlText.strText[1] = Text;
			mlText.strText[2] = sText;
			mlText.strText[3] = gText;
			mlText.dwColor[1] = NS_UITEXTCOLOR::PRIVATE;
			mlText.dwColor[2] = NS_UITEXTCOLOR::DARKTURQUOISE;
			mlText.dwColor[3] = NS_UITEXTCOLOR::PRIVATE;
			AddMultiText( mlText );
			//AddTextMultiColorSplit ( Text,NS_UITEXTCOLOR::PRIVATE, sText,NS_UITEXTCOLOR::DARKTURQUOISE );
		}
		else if ( nAddon )
		{
			Text.Format ( "%s:%d%s", strFormat, nBasic, GetNumberWithSign ( nAddon ) );
			APPEND_ITEM_GRADE ( Text, uGRADE );
			AddTextNoSplit ( Text, NS_UITEXTCOLOR::PRIVATE );
		}
		else if ( nDirectDef && nBasic )
		{
			Text.Format ( "%s:%d", strFormat, nBasic );
			sText.Format ( "%s", GetNumberWithSign( nDirectDef ) );
			mlText.strText[1] = Text;
			mlText.strText[2] = sText;
			mlText.strText[3] = gText;
			mlText.dwColor[1] = NS_UITEXTCOLOR::WHITE;
			mlText.dwColor[2] = NS_UITEXTCOLOR::DARKTURQUOISE;
			mlText.dwColor[3] = NS_UITEXTCOLOR::PRIVATE;
			AddMultiText( mlText );
			//AddTextMultiColorSplit
			//AddTextMultiColorSplit ( Text, NS_UITEXTCOLOR::DEFAULT, sText, NS_UITEXTCOLOR::DARKTURQUOISE );
		}
		else if ( nBasic )
		{
			Text.Format ( "%s:%d", strFormat, nBasic );
			AddTextNoSplit ( Text, NS_UITEXTCOLOR::WHITE );
		}
	}

	void AddInfoItemAddonRangeNew ( int nBasicMin, int nBasicMax, int nAddon, BYTE uGRADE,int nDirect, CString strFormat )
	{
		CString Text;
		CString sText;
		CString ssText;
		CString sssText;
		CString dText;
		CString ddText;

		CString gText;

		CBasicTextBox::MULTITEXT mlText;
		if ( nDirect && nAddon )
		{
			
			Text.Format ( "%s: %d", strFormat, nBasicMin );
			sText.Format ( "%s", GetNumberWithSign( nAddon ) );
			dText.Format ("%s", GetNumberWithSign ( nDirect ) );

			ssText.Format ( "~%d",  nBasicMax );
			sssText.Format ( "%s", GetNumberWithSign( nAddon ) );
			ddText.Format ("%s", GetNumberWithSign ( nDirect ) );
			
			
			gText.Format("[+%d]",uGRADE);

			mlText.strText[1] = Text;
			mlText.strText[2] = dText;
			mlText.strText[3] = sText;
			mlText.strText[4] = ssText;
			mlText.strText[5] = dText;
			mlText.strText[6] = sssText;
			mlText.strText[7] = gText;
			mlText.dwColor[1] = NS_UITEXTCOLOR::PRIVATE;
			mlText.dwColor[2] = NS_UITEXTCOLOR::DARKTURQUOISE;
			mlText.dwColor[3] = NS_UITEXTCOLOR::PRIVATE;
			mlText.dwColor[4] = NS_UITEXTCOLOR::PRIVATE;
			mlText.dwColor[5] = NS_UITEXTCOLOR::DARKTURQUOISE;
			mlText.dwColor[6] = NS_UITEXTCOLOR::PRIVATE;
			mlText.dwColor[7] = NS_UITEXTCOLOR::PRIVATE;

			AddMultiText( mlText );
			//AddTextMultiColorSplit6( Text , NS_UITEXTCOLOR::PRIVATE , dText, NS_UITEXTCOLOR::DARKTURQUOISE, sText , NS_UITEXTCOLOR::PRIVATE ,  ssText , NS_UITEXTCOLOR::PRIVATE, dText, NS_UITEXTCOLOR::DARKTURQUOISE , sssText , NS_UITEXTCOLOR::PRIVATE );
		} 
		else if ( nAddon )
		{
			//Text.Format ( "%s: %d%s~%d%s", strFormat, nBasicMin, GetNumberWithSign( nAddon ) );
			Text.Format ( "%s: %d", strFormat, nBasicMin );
			sText.Format ( "%s", GetNumberWithSign( nAddon ) );

			ssText.Format ( "~%d",  nBasicMax );
			sssText.Format ( "%s", GetNumberWithSign( nAddon ) );
			gText.Format("[+%d]",uGRADE);
			mlText.strText[1] = Text;
			mlText.strText[2] = sText;
			mlText.strText[3] = ssText;
			mlText.strText[4] = sssText;
			mlText.strText[5] = gText;
			mlText.dwColor[1] = NS_UITEXTCOLOR::PRIVATE;
			mlText.dwColor[2] = NS_UITEXTCOLOR::PRIVATE;
			mlText.dwColor[3] = NS_UITEXTCOLOR::PRIVATE;
			mlText.dwColor[4] = NS_UITEXTCOLOR::PRIVATE;
			mlText.dwColor[5] = NS_UITEXTCOLOR::PRIVATE;

			AddMultiText( mlText );
			
			//AddTextMultiColorSplit4( Text , NS_UITEXTCOLOR::PRIVATE , sText , NS_UITEXTCOLOR::PRIVATE , ssText , NS_UITEXTCOLOR::PRIVATE , sssText , NS_UITEXTCOLOR::PRIVATE );
			//AddTextMultiColorSplit( ssText , sssText );
            //APPEND_ITEM_GRADE ( Text, uGRADE );
			//AddTextNoSplit ( Text, NS_UITEXTCOLOR::WHITE );
		}
		else if ( nDirect && (nBasicMin || nBasicMax) )
		{
			Text.Format ( "%s: %d", strFormat, nBasicMin );
			dText.Format ( "%s", GetNumberWithSign( nDirect ) );
			ssText.Format ( "~%d",  nBasicMax );
			ddText.Format ("%s", GetNumberWithSign ( nDirect ) );
			//APPEND_ITEM_GRADE ( Text, uGRADE );
			mlText.strText[1] = Text;
			mlText.strText[2] = dText;
			mlText.strText[3] = ssText;
			mlText.strText[4] = ddText;
			mlText.dwColor[1] = NS_UITEXTCOLOR::PRIVATE;
			mlText.dwColor[2] = NS_UITEXTCOLOR::DARKTURQUOISE;
			mlText.dwColor[3] = NS_UITEXTCOLOR::PRIVATE;
			mlText.dwColor[4] = NS_UITEXTCOLOR::DARKTURQUOISE;

			AddMultiText( mlText );
			//AddTextMultiColorSplit4 ( Text, NS_UITEXTCOLOR::PRIVATE, dText, NS_UITEXTCOLOR::DARKTURQUOISE,ssText, NS_UITEXTCOLOR::PRIVATE, ddText, NS_UITEXTCOLOR::DARKTURQUOISE );
		}

		else if ( nBasicMin || nBasicMax  )
		{
			Text.Format ( "%s: %d~%d",strFormat, nBasicMin, nBasicMax );
			APPEND_ITEM_GRADE ( Text, uGRADE );
			AddTextNoSplit ( Text, NS_UITEXTCOLOR::WHITE );
		}

	}
	
	void AddInfoItemAddonRange ( int nBasicMin, int nBasicMax, int nAddon, BYTE uGRADE, CString strFormat )
	{
		CString Text;

		if ( nAddon )
		{
			Text.Format ( "%s:%d%s~%d%s", strFormat, nBasicMin, GetNumberWithSign( nAddon ), nBasicMax, GetNumberWithSign( nAddon ) );
            APPEND_ITEM_GRADE ( Text, uGRADE );
			AddTextNoSplit ( Text, NS_UITEXTCOLOR::PRIVATE );
		}
		else if ( nBasicMin || nBasicMax )
		{
			Text.Format ( "%s:%d~%d", strFormat, nBasicMin, nBasicMax );
			APPEND_ITEM_GRADE ( Text, uGRADE );
			AddTextNoSplit ( Text, NS_UITEXTCOLOR::WHITE );
		}
	}

	void AddItemTurnInfo ( const SITEMCUSTOM &sItemCustom, const BOOL bInMarket, const BOOL bInPrivateMarket )
	{
		SITEM* pItemData = GLItemMan::GetInstance().GetItem ( sItemCustom.sNativeID );

		CString strText;


			if ( pItemData->sDrugOp.wPileNum > 1 )
			{
				WORD wPileNum = pItemData->sDrugOp.wPileNum;
				WORD wTurnNum = sItemCustom.wTurnNum;				
				if ( bInPrivateMarket )	//	개인 상점
				{
					strText.Format("%s:%d", ID2GAMEWORD("ITEM_TURN_INFO", 0 ), wTurnNum);
					AddTextNoSplit(strText,NS_UITEXTCOLOR::CHARTREUSE);

					return ;
				}

				if ( bInMarket )	wTurnNum = pItemData->GETAPPLYNUM();
				strText.Format("%s:%d/%d", ID2GAMEWORD("ITEM_TURN_INFO", 0 ), wTurnNum, wPileNum);
				AddTextNoSplit(strText,NS_UITEXTCOLOR::WHITE);
			}
			//break;
		//}
	}

	void AddInfoPetSkillItem( const SITEMCUSTOM &sItemCustom )
	{
		CString strText;

		SNATIVEID sNativeID = sItemCustom.sNativeID;
		GLPetClient* pPetClient = GLGaeaClient::GetInstance().GetPetClient();
		SITEM* pItemData = GLItemMan::GetInstance().GetItem ( sNativeID );
		SNATIVEID sSkillID = pItemData->sSkillBookOp.sSkill_ID;

		//	Note : 스킬 정보 가져옴.
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sSkillID.wMainID, sSkillID.wSubID );
		if ( pSkill )
		{
			if ( pPetClient->ISLEARNED_SKILL( sSkillID ) )
			{
				AddTextNoSplit ( ID2GAMEWORD ( "ITEM_SKILL_CONDITION", 0 ), NS_UITEXTCOLOR::RED );	
				return ;
			}
		}
	}

	void AddInfoSkillItem ( const SITEMCUSTOM &sItemCustom )
	{
		CString strText;

		SNATIVEID sNativeID = sItemCustom.sNativeID;
		GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();	
		SITEM* pItemData = GLItemMan::GetInstance().GetItem ( sNativeID );
		SNATIVEID sSkillID = pItemData->sSkillBookOp.sSkill_ID;

		//	Note : 스킬 정보 가져옴.
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sSkillID.wMainID, sSkillID.wSubID );
		if ( pSkill )
		{
			//	기본 정보
			{
				//	2. 등급
				strText.Format("%s:%d",ID2GAMEWORD("SKILL_BASIC_INFO", 1), pSkill->m_sBASIC.dwGRADE);
				AddTextNoSplit(strText,NS_UITEXTCOLOR::PALEGREEN);

				//	3. 속성
				strText.Format("%s:%s",ID2GAMEWORD("SKILL_BASIC_INFO", 2), COMMENT::BRIGHT[pSkill->m_sLEARN.emBRIGHT].c_str());
				AddTextNoSplit(strText,NS_UITEXTCOLOR::PALEGREEN);

				//	4. 직업
				strText.Format ("%s:", ID2GAMEWORD("SKILL_BASIC_INFO", 3) );
				if ( pSkill->m_sLEARN.dwCLASS & GLCC_BRAWLER_M )	strText.AppendFormat ( "%s", COMMENT::CHARCLASS[0].c_str() );
				if ( pSkill->m_sLEARN.dwCLASS & GLCC_BRAWLER_W )	strText.AppendFormat ( "%s", COMMENT::CHARCLASS[6].c_str() );
				if ( pSkill->m_sLEARN.dwCLASS & GLCC_SWORDSMAN_M )	strText.AppendFormat ( "%s", COMMENT::CHARCLASS[1].c_str() );
				if ( pSkill->m_sLEARN.dwCLASS & GLCC_SWORDSMAN_W )	strText.AppendFormat ( "%s", COMMENT::CHARCLASS[7].c_str() );
				if ( pSkill->m_sLEARN.dwCLASS & GLCC_ARCHER_M )		strText.AppendFormat ( "%s", COMMENT::CHARCLASS[8].c_str() );				
				if ( pSkill->m_sLEARN.dwCLASS & GLCC_ARCHER_W )		strText.AppendFormat ( "%s", COMMENT::CHARCLASS[2].c_str() );
				if ( pSkill->m_sLEARN.dwCLASS & GLCC_SHAMAN_M )		strText.AppendFormat ( "%s", COMMENT::CHARCLASS[9].c_str() );				
				if ( pSkill->m_sLEARN.dwCLASS & GLCC_SHAMAN_W )		strText.AppendFormat ( "%s", COMMENT::CHARCLASS[3].c_str() );
				if ( pSkill->m_sLEARN.dwCLASS & GLCC_EXTREME_M )	strText.AppendFormat ( "%s", COMMENT::CHARCLASS[4].c_str() );
				if ( pSkill->m_sLEARN.dwCLASS & GLCC_EXTREME_W )	strText.AppendFormat ( "%s", COMMENT::CHARCLASS[5].c_str() );
				if ( pSkill->m_sLEARN.dwCLASS & GLCC_GUNNER_M )		strText.AppendFormat ( "%s", COMMENT::CHARCLASS[10].c_str() );
				if ( pSkill->m_sLEARN.dwCLASS & GLCC_GUNNER_W )		strText.AppendFormat ( "%s", COMMENT::CHARCLASS[11].c_str() );
				if ( pSkill->m_sLEARN.dwCLASS & GLCC_ASSASSIN_M )	strText.AppendFormat ( "%s", COMMENT::CHARCLASS[12].c_str() );
				if ( pSkill->m_sLEARN.dwCLASS & GLCC_ASSASSIN_W )	strText.AppendFormat ( "%s", COMMENT::CHARCLASS[13].c_str() );
				if ( pSkill->m_sLEARN.dwCLASS & GLCC_TRICKER_M )	strText.AppendFormat ( "%s", COMMENT::CHARCLASS[14].c_str() );
				if ( pSkill->m_sLEARN.dwCLASS & GLCC_TRICKER_W )	strText.AppendFormat ( "%s", COMMENT::CHARCLASS[15].c_str() );

				AddTextNoSplit(strText,NS_UITEXTCONTROL::GetEvaluateColor ( pItemData->sBasicOp.dwReqCharClass & pCharacter->m_emClass ) );
			}

			{
				//	목표 가능거리, 왼손, 오른손 도구
				const SKILL::SSKILLBASIC& sBASIC = pSkill->m_sBASIC;		

				if ( sBASIC.wTARRANGE )
				{
					strText.Format("%s:%d",ID2GAMEWORD( "SKILL_ADVANCED_TARGET_RANGE", 0), sBASIC.wTARRANGE);
					AddTextNoSplit(strText, NS_UITEXTCOLOR::DEFAULT );
				}

				if ( (sBASIC.emUSE_LITEM && (sBASIC.emUSE_LITEM != SKILLATT_NOCARE)) || 
					(sBASIC.emUSE_RITEM && (sBASIC.emUSE_RITEM != SKILLATT_NOCARE)) )
				{
					AddTextNoSplit(ID2GAMEWORD("SKILL_CATEGORY", 1),NS_UITEXTCOLOR::LIGHTSKYBLUE);
				}

				CString strUSEITEM;
				if ( sBASIC.emUSE_LITEM )
				{
					if ( sBASIC.emUSE_LITEM != SKILLATT_NOCARE )
					{
						strText.Format( "%s:%s", ID2GAMEWORD( "SKILL_ADVANCED_USE_ITEM_LR_HAND", 0), COMMENT::SKILLATTACK[sBASIC.emUSE_LITEM].c_str() );
						AddTextNoSplit(strText, NS_UITEXTCOLOR::DEFAULT );
					}
				}

				if ( sBASIC.emUSE_RITEM )
				{
					if ( sBASIC.emUSE_RITEM != SKILLATT_NOCARE )
					{
						strText.Format ( "%s:%s", ID2GAMEWORD( "SKILL_ADVANCED_USE_ITEM_LR_HAND", 1), COMMENT::SKILLATTACK[sBASIC.emUSE_RITEM].c_str() );
						AddTextNoSplit(strText, NS_UITEXTCOLOR::DEFAULT );
					}
				}
			}

			//	이미 배운 스킬
			if ( pCharacter->GETLEARNED_SKILL ( sSkillID ) )
			{
				AddTextNoSplit ( ID2GAMEWORD ( "ITEM_SKILL_CONDITION", 0 ), NS_UITEXTCOLOR::RED );	
				return ;
			}

			//	요구치 정보
			{
				AddTextNoSplit( ID2GAMEWORD("SKILL_CATEGORY", 7), NS_UITEXTCOLOR::LIGHTSKYBLUE);

				const WORD wLevel = 0;
				SKILL::SLEARN& sLEARN = pSkill->m_sLEARN;
				SKILL::SLEARN_LVL& sLVL = sLEARN.sLVL_STEP[wLevel];
			
				//	1. 요구보유스킬
				SNATIVEID NeedSkillID = pSkill->m_sLEARN.sSKILL;			
				if ( NeedSkillID != NATIVEID_NULL() )
				{
					BOOL bVALID = FALSE;
					BOOL bNeedSkillLevel = FALSE;

					CString strNeedSkillName;
					CString strNeedSkillLevel;

					PGLSKILL pNeedSkill = GLSkillMan::GetInstance().GetData ( NeedSkillID.wMainID, NeedSkillID.wSubID );
					strNeedSkillName.Format("%s:%s", ID2GAMEWORD( "SKILL_ADVANCED_INFO", 0), pNeedSkill->GetName() );
					bVALID = pCharacter->ISLEARNED_SKILL ( NeedSkillID );			

					//	2. 요구보유스킬레벨
					if ( 0 < sLVL.dwSKILL_LVL )
					{
						strNeedSkillLevel.Format("%s:%d",ID2GAMEWORD( "SKILL_ADVANCED_INFO", 1), (sLVL.dwSKILL_LVL + 1) );
						bNeedSkillLevel = TRUE;

						SCHARDATA2::SKILL_MAP& map = pCharacter->m_ExpSkills;				
						SCHARDATA2::SKILL_MAP_ITER iter = map.find ( NeedSkillID.dwID );
						if ( iter != map.end() )
						{
							SCHARSKILL& rCharSkill = (*iter).second;

							//	색 조절
							bVALID = rCharSkill.wLevel >= sLVL.dwSKILL_LVL;						
						}
					}

					AddTextNoSplit(strNeedSkillName,NS_UITEXTCONTROL::GetEvaluateColor ( bVALID ));

					if ( bNeedSkillLevel )
						AddTextNoSplit(strNeedSkillLevel, NS_UITEXTCONTROL::GetEvaluateColor ( bVALID ) );
				}

				//	3. 요구경험치
				if ( 0 < sLVL.dwSKP )
				{
					strText.Format("%s:%d",ID2GAMEWORD( "SKILL_ADVANCED_INFO", 2), sLVL.dwSKP);				
					AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_dwSkillPoint >= sLVL.dwSKP ) );
				}

				//	4. 요구레벨
				if ( 0 < sLVL.dwLEVEL )
				{
					strText.Format("%s:%d",ID2GAMEWORD( "SKILL_ADVANCED_INFO", 3), sLVL.dwLEVEL);
					AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->GETLEVEL () >= int(sLVL.dwLEVEL) ) );
				}

				//	5. 조건 - 암광
				BOOL bValue = TRUE;
				strText.Format ( "%s", COMMENT::BRIGHT[pItemData->sBasicOp.emReqBright].c_str() );
				if ( pItemData->sBasicOp.emReqBright != BRIGHT_BOTH )
				{
					if ( pCharacter->GETBRIGHT() != pItemData->sBasicOp.emReqBright )
					{
						bValue = FALSE;
					}
				}
				AddTextNoSplit ( strText, NS_UITEXTCONTROL::GetEvaluateColor ( bValue ) );


				/*UserNum ItemReq, Juver, 2017/06/27 */
				if ( pItemData->sBasicOp.dwReqUserNum != 0 )
				{
					strText.Format("%s:%d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_EXTRA", 0 ), pItemData->sBasicOp.dwReqUserNum );
					AddTextNoSplit(strText,NS_UITEXTCONTROL::GetEvaluateColor ( pItemData->sBasicOp.dwReqUserNum == pCharacter->GetUserID() ));
				}

				//	Stats
				//	1. 요구힘
				if ( 0 < sLVL.sSTATS.wPow )
				{
					strText.Format("%s:%d",ID2GAMEWORD( "SKILL_ADVANCED_INFO_STATS", 0), sLVL.sSTATS.wPow);
					AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wPow >= sLVL.sSTATS.wPow ) );
				}

				//	2. 요구체력
				if ( 0 < sLVL.sSTATS.wStr )
				{
					strText.Format("%s:%d",ID2GAMEWORD( "SKILL_ADVANCED_INFO_STATS", 1), sLVL.sSTATS.wStr);
					AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wStr >= sLVL.sSTATS.wStr ) );
				}

				//	3. 요구정신
				if ( 0 < sLVL.sSTATS.wSpi )
				{
					strText.Format("%s:%d",ID2GAMEWORD( "SKILL_ADVANCED_INFO_STATS", 2), sLVL.sSTATS.wSpi);
					AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wSpi >= sLVL.sSTATS.wSpi ));
				}

				//	4. 요구민첩
				if ( 0 < sLVL.sSTATS.wDex )
				{
					strText.Format("%s:%d",ID2GAMEWORD( "SKILL_ADVANCED_INFO_STATS", 3), sLVL.sSTATS.wDex);
					AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wDex >= sLVL.sSTATS.wDex ) );
				}

				//	5. 요구지력
				if ( 0 < sLVL.sSTATS.wInt )
				{
					strText.Format("%s:%d",ID2GAMEWORD( "SKILL_ADVANCED_INFO_STATS", 4), sLVL.sSTATS.wInt);
					AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wInt >= sLVL.sSTATS.wInt ) );
				}

				//	6. 요구근력
				if ( 0 < sLVL.sSTATS.wSta )
				{
					strText.Format("%s:%d",ID2GAMEWORD( "SKILL_ADVANCED_INFO_STATS", 5), sLVL.sSTATS.wSta);
					AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wPow >= sLVL.sSTATS.wSta ) );
				}
			}
		}
	}

	void AddTextAddValue( const SITEMCUSTOM & sItemCustom, const ITEM::SSUIT & sSUIT )
	{
		CString strText;
		BOOL bLEAST_HAVE_ONE(TRUE);
		BOOL bADD_HP(FALSE), bADD_MP(FALSE), bADD_SP(FALSE), bADD_MA(FALSE);
		INT arrVALUE[EMADD_SIZE];
		SecureZeroMemory( arrVALUE, sizeof(arrVALUE) );

		for ( int i = 0; i < ITEM::SSUIT::ADDON_SIZE; ++i )
		{
			EMITEM_ADDON emTYPE = sSUIT.sADDON[i].emTYPE;

			if ( emTYPE != EMADD_NONE )
			{
				//	NOTE
				if ( bLEAST_HAVE_ONE )
				{
					AddTextNoSplit( " ",NS_UITEXTCOLOR::LIGHTSKYBLUE);
					bLEAST_HAVE_ONE = FALSE;
				}

				int nVALUE = sSUIT.sADDON[i].nVALUE;

				switch ( emTYPE )
				{
				case EMADD_HP:
					if( !bADD_HP )
					{
						arrVALUE[EMADD_HP] = sItemCustom.GETADDHP();
						bADD_HP = TRUE;
					}
					break;
				case EMADD_MP:
					if( !bADD_MP )
					{
						arrVALUE[EMADD_MP] = sItemCustom.GETADDMP();
						bADD_MP = TRUE;
					}
					break;
				case EMADD_SP:
					if( !bADD_SP )
					{
						arrVALUE[EMADD_SP] = sItemCustom.GETADDSP();
						bADD_SP = TRUE;
					}
					break;
				case EMADD_MA:
					if( !bADD_MA )
					{
						arrVALUE[EMADD_MA] = sItemCustom.GETADDMA();
						bADD_MA = TRUE;
					}
					break;
				case EMADD_HITRATE:		arrVALUE[EMADD_HITRATE] += nVALUE;		break;
				case EMADD_AVOIDRATE:	arrVALUE[EMADD_AVOIDRATE] += nVALUE;	break;
				case EMADD_DAMAGE:		arrVALUE[EMADD_DAMAGE] += nVALUE;		break;
				case EMADD_DEFENSE:		arrVALUE[EMADD_DEFENSE] += nVALUE;		break;
				case EMADD_STATS_POW:	arrVALUE[EMADD_STATS_POW] += nVALUE;	break;
				case EMADD_STATS_STR:	arrVALUE[EMADD_STATS_STR] += nVALUE;	break;
				case EMADD_STATS_SPI:	arrVALUE[EMADD_STATS_SPI] += nVALUE;	break;
				case EMADD_STATS_DEX:	arrVALUE[EMADD_STATS_DEX] += nVALUE;	break;
				case EMADD_STATS_INT:	arrVALUE[EMADD_STATS_INT] += nVALUE;	break;
				case EMADD_STATS_STA:	arrVALUE[EMADD_STATS_STA] += nVALUE;	break;
				case EMADD_PA:			arrVALUE[EMADD_PA] += nVALUE;			break;
				case EMADD_SA:			arrVALUE[EMADD_SA] += nVALUE;			break;
				case EMADD_POWINTDEX:	arrVALUE[EMADD_POWINTDEX] += nVALUE;	break;
				case EMADD_STMVIT:		arrVALUE[EMADD_STMVIT] += nVALUE;		break;
				case EMADD_PASA:		arrVALUE[EMADD_PASA] += nVALUE;			break;
				case EMADD_PASAMA:		arrVALUE[EMADD_PASAMA] += nVALUE;		break;
				};
			}
		}
		if (sItemCustom.nidDISGUISE != SNATIVEID(false))
		{
			SITEM* pJDisguiseData = GLItemMan::GetInstance().GetItem(sItemCustom.nidDISGUISE);
			if (pJDisguiseData)
			{
				if (pJDisguiseData->sBasicOp.IsDISGUISE())
				{
					//add jitem
					BOOL bLEAST_HAVE_ONE(TRUE);
					INT arrVALUECOS[EMADD_SIZE];
					SecureZeroMemory(arrVALUECOS, sizeof(arrVALUECOS));

					for (int j = 0; j < ITEM::SSUIT::ADDON_SIZE; ++j)
					{
						EMITEM_ADDON emTYPEJ = pJDisguiseData->sSuitOp.sADDON[j].emTYPE;

						if (emTYPEJ != EMADD_NONE)
						{
							if (bLEAST_HAVE_ONE)
							{
								bLEAST_HAVE_ONE = FALSE;
							}

							int nVALUECOS = pJDisguiseData->sSuitOp.sADDON[j].nVALUE;
							switch (emTYPEJ)
							{
							case EMADD_HP:			arrVALUECOS[EMADD_HP] += nVALUECOS;			break;
							case EMADD_MP:			arrVALUECOS[EMADD_MP] += nVALUECOS;			break;
							case EMADD_SP:			arrVALUECOS[EMADD_SP] += nVALUECOS;			break;
							case EMADD_HITRATE:		arrVALUECOS[EMADD_HITRATE] += nVALUECOS;		break;
							case EMADD_AVOIDRATE:	arrVALUECOS[EMADD_AVOIDRATE] += nVALUECOS;		break;
							case EMADD_DAMAGE:		arrVALUECOS[EMADD_DAMAGE] += nVALUECOS;		break;
							case EMADD_DEFENSE:		arrVALUECOS[EMADD_DEFENSE] += nVALUECOS;		break;
							case EMADD_STATS_POW:	arrVALUECOS[EMADD_STATS_POW] += nVALUECOS;		break;
							case EMADD_STATS_STR:	arrVALUECOS[EMADD_STATS_STR] += nVALUECOS;		break;
							case EMADD_STATS_SPI:	arrVALUECOS[EMADD_STATS_SPI] += nVALUECOS;		break;
							case EMADD_STATS_DEX:	arrVALUECOS[EMADD_STATS_DEX] += nVALUECOS;		break;
							case EMADD_STATS_INT:	arrVALUECOS[EMADD_STATS_INT] += nVALUECOS;		break;
							case EMADD_STATS_STA:	arrVALUECOS[EMADD_STATS_STA] += nVALUECOS;		break;
							case EMADD_PA:			arrVALUECOS[EMADD_PA] += nVALUECOS;			break;
							case EMADD_SA:			arrVALUECOS[EMADD_SA] += nVALUECOS;			break;
							case EMADD_MA:			arrVALUECOS[EMADD_MA] += nVALUECOS;			break;
							case EMADD_POWINTDEX:	arrVALUECOS[EMADD_POWINTDEX] += nVALUECOS;	break;
							case EMADD_STMVIT:		arrVALUECOS[EMADD_STMVIT] += nVALUECOS;		break;
							case EMADD_PASA:		arrVALUECOS[EMADD_PASA] += nVALUECOS;			break;
							case EMADD_PASAMA:		arrVALUECOS[EMADD_PASAMA] += nVALUECOS;		break;
							};
						}
					}

					for (int i = 1; i<EMADD_SIZE; ++i)
					{
						if (arrVALUE[i] != 0)
						{
							if (arrVALUECOS[i] != 0)
							{
								CString strText2;
								CString strText3;
								CBasicTextBox::MULTITEXT mlText,mlText2;
								strText2.Format("%s :%s", ID2GAMEWORD("ITEM_ADDON_INFO", i), GetNumberWithSign(arrVALUE[i]));
								strText3.Format(" %s", GetNumberWithSign(arrVALUECOS[i]));
								mlText.strText[1] = strText2; 
								mlText.strText[2] = strText3; 
								mlText.dwColor[1] = NS_UITEXTCOLOR::GREEN; 
								mlText.dwColor[2] = NS_UITEXTCOLOR::PALEGREEN; 
								AddMultiText( mlText );
							}
							else
							{
								CString strText;
								strText.Format("%s :%s", ID2GAMEWORD("ITEM_ADDON_INFO", i), GetNumberWithSign(arrVALUE[i]));
								AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
							}
						}
						else
						{
							if (arrVALUECOS[i] != 0)
							{
								CString strTextCos;
								strTextCos.Format("%s :%s", ID2GAMEWORD("ITEM_ADDON_INFO", i), GetNumberWithSign(arrVALUECOS[i]));
								AddTextNoSplit(strTextCos, NS_UITEXTCOLOR::PALEGREEN);
							}
						}
					}
				}
			}
		}
		else
		{
			for (int i = 1; i<EMADD_SIZE; ++i)
			{
				if (arrVALUE[i] != 0)
				{
					strText.Format("%s :%s", ID2GAMEWORD("ITEM_ADDON_INFO", i), GetNumberWithSign(arrVALUE[i]));
					AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
				}
			}
		}

		
		
		/*for (int i = 1; i<EMADD_SIZE; ++i)
		{
			if (arrVALUE[i] != 0)
			{
				strText.Format("%s :%s", ID2GAMEWORD("ITEM_ADDON_INFO", i), GetNumberWithSign(arrVALUE[i]));
				AddTextNoSplit(strText, NS_UITEXTCOLOR::LIME);
			}
		}*/

		/*if (sItemCustom.nidDISGUISE != SNATIVEID(false))
		{
			SITEM* pJDisguiseData = GLItemMan::GetInstance().GetItem(sItemCustom.nidDISGUISE);
			if (pJDisguiseData)
			{
				if (pJDisguiseData->sBasicOp.IsDISGUISE())
				{
					//add jitem
					BOOL bLEAST_HAVE_ONE(TRUE);
					INT arrVALUECOS[EMADD_SIZE];
					SecureZeroMemory(arrVALUECOS, sizeof(arrVALUECOS));

					for (int j = 0; j < ITEM::SSUIT::ADDON_SIZE; ++j)
					{
						EMITEM_ADDON emTYPEJ = pJDisguiseData->sSuitOp.sADDON[j].emTYPE;

						if (emTYPEJ != EMADD_NONE)
						{
							if (bLEAST_HAVE_ONE)
							{
								AddTextNoSplit(ID2GAMEWORD("ITEM_DISGUISE_BASIC_INFO",0), NS_UITEXTCOLOR::FORESTGREEN);
								bLEAST_HAVE_ONE = FALSE;
							}

							int nVALUECOS = pJDisguiseData->sSuitOp.sADDON[j].nVALUE;
							switch (emTYPEJ)
							{
							case EMADD_HP:			arrVALUECOS[EMADD_HP] += nVALUECOS;			break;
							case EMADD_MP:			arrVALUECOS[EMADD_MP] += nVALUECOS;			break;
							case EMADD_SP:			arrVALUECOS[EMADD_SP] += nVALUECOS;			break;
							case EMADD_HITRATE:		arrVALUECOS[EMADD_HITRATE] += nVALUECOS;		break;
							case EMADD_AVOIDRATE:	arrVALUECOS[EMADD_AVOIDRATE] += nVALUECOS;		break;
							case EMADD_DAMAGE:		arrVALUECOS[EMADD_DAMAGE] += nVALUECOS;		break;
							case EMADD_DEFENSE:		arrVALUECOS[EMADD_DEFENSE] += nVALUECOS;		break;
							case EMADD_STATS_POW:	arrVALUECOS[EMADD_STATS_POW] += nVALUECOS;		break;
							case EMADD_STATS_STR:	arrVALUECOS[EMADD_STATS_STR] += nVALUECOS;		break;
							case EMADD_STATS_SPI:	arrVALUECOS[EMADD_STATS_SPI] += nVALUECOS;		break;
							case EMADD_STATS_DEX:	arrVALUECOS[EMADD_STATS_DEX] += nVALUECOS;		break;
							case EMADD_STATS_INT:	arrVALUECOS[EMADD_STATS_INT] += nVALUECOS;		break;
							case EMADD_STATS_STA:	arrVALUECOS[EMADD_STATS_STA] += nVALUECOS;		break;
							case EMADD_PA:			arrVALUECOS[EMADD_PA] += nVALUECOS;			break;
							case EMADD_SA:			arrVALUECOS[EMADD_SA] += nVALUECOS;			break;
							case EMADD_MA:			arrVALUECOS[EMADD_MA] += nVALUECOS;			break;
							case EMADD_CP:			arrVALUECOS[EMADD_CP] += nVALUECOS;			break; //add cp
							};
						}
					}

					CString strTextCos;
					for (int i = 1; i<EMADD_SIZE; ++i)
					{
						if (arrVALUECOS[i] != 0)
						{
							strTextCos.Format("%s :%s", ID2GAMEWORD("ITEM_ADDON_INFO", i), GetNumberWithSign(arrVALUECOS[i]));
							AddTextNoSplit(strTextCos, NS_UITEXTCOLOR::LIME);
						}
					}
				}
			}
		}*/
	}
	
	void LOAD_SIMPLE ( const SITEMCUSTOM &sItemCustom )
	{
		if ( m_sItemCustomBACK == sItemCustom ) return ;

		m_sItemCustomBACK = sItemCustom;

		RemoveAllInfo();

		SITEM* pItemData = GLItemMan::GetInstance().GetItem ( sItemCustom.sNativeID );
		if ( !pItemData ) return ;
		
		DWORD dwLevel = pItemData->sBasicOp.emLevel;
		//	이름
		AddTextNoSplit ( pItemData->GetName(), COMMENT::ITEMCOLOR[dwLevel] );
	}

	void LOADRANDOMOPTION ( const SITEMCUSTOM &sItemCustom )
	{
		SITEMCUSTOM sCust;

		CString strText,strTex,strTe;
		strText.Format ("[");
		strTex.Format (ID2GAMEWORD("ITEM_REFORM_SHOW",2));
		strTe.Format ("]");

		strText += strTex;
		strText += strTe;
		AddTextNoSplit (ID2GAMEWORD("ITEM_REFORM_SHOW",1), NS_UITEXTCOLOR::DEFAULT );
		AddTextNoSplit (strText, NS_UITEXTCOLOR::dwItemRank2 );
		AddTextNoSplit (ID2GAMEWORD("ITEM_REFORM_SHOW",3), NS_UITEXTCOLOR::DISABLE );

		//SRANDOM_GEN::sSETR;
		SRANDOM_GEN *pRANDOM_SET = GLItemMan::GetInstance().GetItemRandomOpt( sItemCustom.sNativeID );
		if( !pRANDOM_SET )	return;
		
		for( int i=0;i<pRANDOM_SET->dwSET_NUM;++i)
		{
			if( pRANDOM_SET->sSETR[i].wTYPE == EMR_OPT_NULL ) continue;
			const SRANDOM_OPT_SET* pSET = &pRANDOM_SET->sSETR[i];

			CString strDescText;
			INT nType = pSET->wTYPE;
			float fHigh = 0.0f;
			float fLow = 9999.0f;

			for( WORD j=0; j<pSET->wSECT; ++j )
			{
				if( pSET->pSECT[j].fLOW < fLow ) fLow = pSET->pSECT[j].fLOW;
				if( pSET->pSECT[j].fHIGH > fHigh ) fHigh = pSET->pSECT[j].fHIGH;
			}			
			strDescText.Format( "%s: %.2f~%.2f", ID2GAMEWORD("ITEM_RANDOM_OPTION", nType ), fLow, fHigh );
			if( sItemCustom.IsPerRandOpt( nType ) ) strDescText += _T('%');
			
			AddTextNoSplit( strDescText, NS_UITEXTCOLOR::DARKTURQUOISE );
		}
		
	}

	/*npc shop, Juver, 2017/07/26 */
	/*item link, Juver, 2017/07/31 */
	/*product item, Juver, 2017/10/15 */
	void LOAD ( const SITEMCUSTOM &sItemCustom_base, const BOOL bShopOpen, const BOOL bInMarket, const BOOL bInPrivateMarket, const BOOL bIsWEAR_ITEM, WORD wPosX, WORD wPosY, SNATIVEID sNpcNativeID,
		const BOOL bNPCShop /*= FALSE*/, const WORD wNPCShopType /*= 0*/, const BOOL bItemLink /*= FALSE*/, const BOOL bInInventory /*= FALSE*/, BOOL bInCompoundResult /*= FALSE*/, BOOL bCompoundGenerateRandomOption /*= FALSE*/  )  
	{	
		 /*npc shop, Juver, 2017/07/26 */
		/*item link, Juver, 2017/07/31 */
		/*product item, Juver, 2017/10/15 */
		if ( m_sItemCustomBACK == sItemCustom_base && m_bShopOpenBACK == bShopOpen
			&& m_bInMarketBACK == bInMarket && m_bInPrivateMarketBACK == bInPrivateMarket
			&& m_bIsWEAR_ITEMBACK == bIsWEAR_ITEM && m_wPosXBACK == wPosX && m_wPosYBACK == wPosY
			&& m_bNPCShopBACK == bNPCShop && m_wNPCShopTypeBACK == wNPCShopType
			&& m_bItemLinkBACK == bItemLink && m_bInInventoryBACK == bInInventory
			&& m_bInCompoundResultBACK == bInCompoundResult &&  m_bCompoundGenerateRandomOptionBACK == bCompoundGenerateRandomOption )	
			return ;

		m_sItemCustomBACK = sItemCustom_base;
		m_bShopOpenBACK = bShopOpen;
		m_bInMarketBACK = bInMarket;
		m_bInPrivateMarketBACK = bInPrivateMarket;
		m_bIsWEAR_ITEMBACK = bIsWEAR_ITEM;
		m_wPosXBACK = wPosX;
		m_wPosYBACK = wPosY;

		/*npc shop, Juver, 2017/07/26 */
		m_bNPCShopBACK = bNPCShop; 
		m_wNPCShopTypeBACK = wNPCShopType;

		/*item link, Juver, 2017/07/31 */
		m_bItemLinkBACK = bItemLink;
		m_bInInventoryBACK = bInInventory;

		/*product item, Juver, 2017/10/15 */
		m_bInCompoundResultBACK = bInCompoundResult;
		m_bCompoundGenerateRandomOptionBACK = bCompoundGenerateRandomOption;

		RemoveAllInfo();

		/*item wrapper, Juver, 2018/01/12 */
		SITEMCUSTOM sItemCustom = sItemCustom_base;

		CString strText, strText2;
		BOOL bValue = FALSE;
		int nExtraValue = 0;

		GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
		SITEM* pItemData = GLItemMan::GetInstance().GetItem ( sItemCustom.sNativeID );

		/*item wrapper, Juver, 2018/01/12 */
		BOOL bWrapped = FALSE;
		std::string strorigname = "";
		if ( pItemData && pItemData->sBasicOp.emItemType == ITEM_WRAPPER_BOX && sItemCustom.nidDISGUISE != NATIVEID_NULL() )
		{
			strorigname = pItemData->GetName();
			bWrapped = TRUE;
		}

		
		if ( bWrapped )
		{
			pItemData = NULL;
			pItemData = GLItemMan::GetInstance().GetItem( sItemCustom.nidDISGUISE );
			sItemCustom.sNativeID = sItemCustom.nidDISGUISE;
			sItemCustom.nidDISGUISE = NATIVEID_NULL();
			sItemCustom.tDISGUISE = 0;
		}

		if ( !pItemData )
		{
			bWrapped = FALSE;
			pItemData = GLItemMan::GetInstance().GetItem ( sItemCustom.sNativeID );
		}

		if ( pItemData )
		{
			AddTextNoSplit ( "", NS_UITEXTCOLOR::ORANGE );

			SCHARSTATS& rItemStats = pItemData->sBasicOp.sReqStats;
			SCHARSTATS& rCharStats = pCharacter->m_sSUMSTATS;
			EMITEM_TYPE emItemType = pItemData->sBasicOp.emItemType;

			CString strSchool, strSchoolText, strClass, strClassText;
			CString strCombineText;
			BOOL bReqSchool, bNeedDisguise, bReqClass = FALSE;

			DWORD dwCLASSFIVE = (GLCC_BRAWLER_M|GLCC_SWORDSMAN_M|GLCC_ARCHER_W|GLCC_SHAMAN_W|GLCC_BRAWLER_W|GLCC_SWORDSMAN_W|GLCC_ARCHER_M|GLCC_SHAMAN_M|
				GLCC_EXTREME_M|GLCC_EXTREME_W);

			DWORD dwCLASSSIX = (GLCC_BRAWLER_M|GLCC_SWORDSMAN_M|GLCC_ARCHER_W|GLCC_SHAMAN_W|GLCC_BRAWLER_W|GLCC_SWORDSMAN_W|GLCC_ARCHER_M|GLCC_SHAMAN_M|
				GLCC_EXTREME_W|GLCC_GUNNER_M|GLCC_GUNNER_W|GLCC_ASSASSIN_M|GLCC_ASSASSIN_W|GLCC_TRICKER_M|GLCC_TRICKER_W);
						
			DWORD	dwALLMAN5		= (GLCC_BRAWLER_M | GLCC_SWORDSMAN_M | GLCC_ARCHER_M | GLCC_SHAMAN_M | GLCC_EXTREME_M | GLCC_GUNNER_M | GLCC_ASSASSIN_M | GLCC_TRICKER_M);
			DWORD	dwALLWOMAN5		= (GLCC_BRAWLER_W | GLCC_SWORDSMAN_W | GLCC_ARCHER_W | GLCC_SHAMAN_W | GLCC_EXTREME_W | GLCC_GUNNER_W | GLCC_ASSASSIN_W | GLCC_TRICKER_W);

			DWORD	dwALLMAN6		= (GLCC_BRAWLER_M | GLCC_SWORDSMAN_M | GLCC_ARCHER_M | GLCC_SHAMAN_M | GLCC_EXTREME_M | GLCC_GUNNER_M | GLCC_ASSASSIN_M | GLCC_TRICKER_M);
			DWORD	dwALLWOMAN6		= (GLCC_BRAWLER_W | GLCC_SWORDSMAN_W | GLCC_ARCHER_W | GLCC_SHAMAN_W | GLCC_EXTREME_W | GLCC_GUNNER_W | GLCC_ASSASSIN_W | GLCC_TRICKER_W);

			DWORD dwReqSchool = pItemData->sBasicOp.dwReqSchool;

			if ( pItemData->sBasicOp.dwReqSchool!=GLSCHOOL_ALL )
			{
				if ( pItemData->sBasicOp.dwReqSchool & GLSCHOOL_00)
				{
					//strSchool += ID2GAMEWORD ( "ITEM_REQ_SCHOOL", 0 );
					strSchool += GLCONST_CHAR::strSCHOOLNAME[school2index(GLSCHOOL_00)].c_str();
					bReqSchool = TRUE;
				}
				if ( pItemData->sBasicOp.dwReqSchool & GLSCHOOL_01)
				{
					//strSchool += ID2GAMEWORD ( "ITEM_REQ_SCHOOL", 1 );
					strSchool += GLCONST_CHAR::strSCHOOLNAME[school2index(GLSCHOOL_01)].c_str();
					bReqSchool = TRUE;
				}
				if ( pItemData->sBasicOp.dwReqSchool & GLSCHOOL_02)
				{
					//strSchool += ID2GAMEWORD ( "ITEM_REQ_SCHOOL", 2 );
					strSchool += GLCONST_CHAR::strSCHOOLNAME[school2index(GLSCHOOL_02)].c_str();
					bReqSchool = TRUE;
				}						
			}
			else
			{
				//strSchool = ID2GAMEWORD ((char*)"ITEM_REQ_SCHOOL", 3 );
				//strSchool = COMMENT::EMSCHOOL2[4].c_str();
				strSchool = ID2GAMEWORD ((char*)"ITEM_REQ_SCHOOL", 3 );
				bReqSchool = TRUE;
			}

			if( sItemCustom.nidDISGUISE != NATIVEID_NULL() )
			{
				bNeedDisguise = TRUE;
				SITEM* pItemDisguise = GLItemMan::GetInstance().GetItem ( sItemCustom.nidDISGUISE );
				DWORD dwReqCharClass_Disguise = pItemDisguise->sBasicOp.dwReqCharClass;
				DWORD dwReqCharClass = pItemData->sBasicOp.dwReqCharClass;
				CString szClass;
				if( dwReqCharClass )
				{
					//if ( (dwReqCharClass==GLCC_ALL_NEWSEX) && (dwReqCharClass_Disguise==GLCC_ALL_NEWSEX) )
					//add class
					if ( (dwReqCharClass==dwCLASSSIX) && (dwReqCharClass_Disguise==dwCLASSSIX) || (dwReqCharClass==dwCLASSFIVE) && (dwReqCharClass_Disguise==dwCLASSFIVE) || (dwReqCharClass==GLCC_ALL_8CLASS) && (dwReqCharClass_Disguise==GLCC_ALL_8CLASS) )
					{
						//szClass = ID2GAMEWORD((char*)"ITEM_ALL_CLASS", 0);
						szClass = COMMENT::CHARCLASS2[9].c_str();
						bReqClass = TRUE;
					}
					else if ( ( dwReqCharClass == dwALLMAN6 && dwReqCharClass_Disguise == dwALLMAN6 ) || ( dwReqCharClass == dwALLMAN5 && dwReqCharClass_Disguise == dwALLMAN5 ) || ( dwReqCharClass == GLCC_MAN_8CLASS && dwReqCharClass_Disguise == GLCC_MAN_8CLASS )  )
					{
						//szClass = ID2GAMEWORD((char*)"ITEM_CLASS_GENDER", 0);
						szClass = COMMENT::EMGENDER[1].c_str();
						bReqClass = TRUE;
					}
					else if ( ( dwReqCharClass == dwALLWOMAN6 && dwReqCharClass_Disguise == dwALLWOMAN6 ) || ( dwReqCharClass == dwALLWOMAN5 && dwReqCharClass_Disguise == dwALLWOMAN5 ) || ( dwReqCharClass == GLCC_WOMAN_8CLASS && dwReqCharClass_Disguise == GLCC_WOMAN_8CLASS )  )
					{
						//szClass = ID2GAMEWORD((char*)"ITEM_CLASS_GENDER", 1);
						szClass = COMMENT::EMGENDER[0].c_str();
						bReqClass = TRUE;
					}
					else
					{
						if ( (dwReqCharClass&GLCC_BRAWLER_M) && (dwReqCharClass_Disguise&GLCC_BRAWLER_M) &&
							(dwReqCharClass&GLCC_BRAWLER_W) && (dwReqCharClass_Disguise&GLCC_BRAWLER_W))
						{
							szClass+=" ";
							//szClass+=ID2GAMEWORD((char*)"ITEM_CLASS_NAME", 0);
							szClass+=COMMENT::CHARCLASS2[0].c_str();
							bReqClass = TRUE;
						}
						if ( (dwReqCharClass&GLCC_SWORDSMAN_M)  && (dwReqCharClass_Disguise&GLCC_SWORDSMAN_M) &&
							(dwReqCharClass&GLCC_SWORDSMAN_W)  && (dwReqCharClass_Disguise&GLCC_SWORDSMAN_W))
						{
							szClass+=" ";
							//szClass+=ID2GAMEWORD((char*)"ITEM_CLASS_NAME", 1);
							szClass+=COMMENT::CHARCLASS2[1].c_str();
							bReqClass = TRUE;
						}
						if ( (dwReqCharClass&GLCC_ARCHER_M)  && (dwReqCharClass_Disguise&GLCC_ARCHER_M) &&
							(dwReqCharClass&GLCC_ARCHER_W)  && (dwReqCharClass_Disguise&GLCC_ARCHER_W))
						{
							szClass+=" ";
							//szClass+=ID2GAMEWORD((char*)"ITEM_CLASS_NAME", 2);
							szClass+=COMMENT::CHARCLASS2[2].c_str();
							bReqClass = TRUE;
						}
						if ( (dwReqCharClass&GLCC_SHAMAN_M)  && (dwReqCharClass_Disguise&GLCC_SHAMAN_M) &&
							(dwReqCharClass&GLCC_SHAMAN_W)  && (dwReqCharClass_Disguise&GLCC_SHAMAN_W))
						{
							szClass+=" ";
							//szClass+=ID2GAMEWORD((char*)"ITEM_CLASS_NAME", 3);
							szClass+=COMMENT::CHARCLASS2[3].c_str();
							bReqClass = TRUE;
						}
						if ( (dwReqCharClass&GLCC_EXTREME_M)  && (dwReqCharClass_Disguise&GLCC_EXTREME_M) &&
							(dwReqCharClass&GLCC_EXTREME_W)  && (dwReqCharClass_Disguise&GLCC_EXTREME_W))
						{
							szClass+=" ";
							//szClass+=ID2GAMEWORD((char*)"ITEM_CLASS_NAME", 4);
							szClass+=COMMENT::CHARCLASS2[4].c_str();
							bReqClass = TRUE;
						}
						//add class
						if ( (dwReqCharClass&GLCC_GUNNER_M)  && (dwReqCharClass_Disguise&GLCC_GUNNER_M) &&
							(dwReqCharClass&GLCC_GUNNER_W)  && (dwReqCharClass_Disguise&GLCC_GUNNER_W))
						{
							szClass+=" ";
							//szClass+=ID2GAMEWORD((char*)"ITEM_CLASS_NAME", 5);
							szClass+=COMMENT::CHARCLASS2[5].c_str();
							bReqClass = TRUE;
						}
						if ( (dwReqCharClass&GLCC_ASSASSIN_M)  && (dwReqCharClass_Disguise&GLCC_ASSASSIN_M) &&
							(dwReqCharClass&GLCC_ASSASSIN_W)  && (dwReqCharClass_Disguise&GLCC_ASSASSIN_W))
						{
							szClass+=" ";
							//szClass+=ID2GAMEWORD((char*)"ITEM_CLASS_NAME", 6);
							szClass+=COMMENT::CHARCLASS2[6].c_str();
							bReqClass = TRUE;
						}
						if ( (dwReqCharClass&GLCC_TRICKER_M)  && (dwReqCharClass_Disguise&GLCC_TRICKER_M) &&
							(dwReqCharClass&GLCC_TRICKER_W)  && (dwReqCharClass_Disguise&GLCC_TRICKER_W))
						{
							szClass+=" ";
							//szClass+=ID2GAMEWORD((char*)"ITEM_CLASS_NAME", 7);
							szClass+=COMMENT::CHARCLASS2[7].c_str();
							bReqClass = TRUE;
						}
					}
					strClass.Format( "%s", szClass.GetString() );
				}
			}
			else
			{
				DWORD dwReqCharClass = pItemData->sBasicOp.dwReqCharClass;
				CString szClass;
				if( dwReqCharClass )
				{
					if ( (dwReqCharClass==dwCLASSSIX) && (dwReqCharClass==dwCLASSSIX) || (dwReqCharClass==dwCLASSFIVE) && (dwReqCharClass==dwCLASSFIVE) || (dwReqCharClass==GLCC_ALL_8CLASS) && (dwReqCharClass==GLCC_ALL_8CLASS)  )
					{
						//szClass = ID2GAMEWORD((char*)"ITEM_ALL_CLASS", 0);
						szClass = COMMENT::CHARCLASS2[8].c_str();
						bReqClass = TRUE;
					}
					else if ( ( dwReqCharClass == dwALLMAN6 ) || ( dwReqCharClass == dwALLMAN5 ) || ( dwReqCharClass == GLCC_MAN_8CLASS )  )
					{
						//szClass = ID2GAMEWORD((char*)"ITEM_CLASS_GENDER", 0);
						szClass = COMMENT::EMGENDER[1].c_str();
						bReqClass = TRUE;
					}
					else if ( ( dwReqCharClass == dwALLWOMAN6 ) || ( dwReqCharClass == dwALLWOMAN5 ) || ( dwReqCharClass == GLCC_WOMAN_8CLASS )  )
					{
						//szClass = ID2GAMEWORD((char*)"ITEM_CLASS_GENDER", 1);
						szClass = COMMENT::EMGENDER[0].c_str();
						bReqClass = TRUE;
					}
					else
					{
						if (( dwReqCharClass & GLCC_BRAWLER_M)&&( dwReqCharClass & GLCC_BRAWLER_W))
						{
							szClass+=" ";
							//szClass+=ID2GAMEWORD((char*)"ITEM_CLASS_NAME", 0);
							szClass+=COMMENT::CHARCLASS2[0].c_str();
							bReqClass = TRUE;
						}
						if (( dwReqCharClass & GLCC_SWORDSMAN_M)&&( dwReqCharClass & GLCC_SWORDSMAN_W))
						{
							szClass+=" ";
							//szClass+=ID2GAMEWORD((char*)"ITEM_CLASS_NAME", 1);
							szClass+=COMMENT::CHARCLASS2[1].c_str();
							bReqClass = TRUE;
						}
						if (( dwReqCharClass & GLCC_ARCHER_M)&&( dwReqCharClass & GLCC_ARCHER_W))
						{
							szClass+=" ";
							//szClass+=ID2GAMEWORD((char*)"ITEM_CLASS_NAME", 2);
							szClass+=COMMENT::CHARCLASS2[2].c_str();
							bReqClass = TRUE;
						}
						if (( dwReqCharClass & GLCC_SHAMAN_M)&&( dwReqCharClass & GLCC_SHAMAN_W))
						{
							szClass+=" ";
							//szClass+=ID2GAMEWORD((char*)"ITEM_CLASS_NAME", 3);
							szClass+=COMMENT::CHARCLASS2[3].c_str();
							bReqClass = TRUE;
						}
						if (( dwReqCharClass & GLCC_EXTREME_M)&&( dwReqCharClass & GLCC_EXTREME_W))
						{
							szClass+=" ";
							//szClass+=ID2GAMEWORD((char*)"ITEM_CLASS_NAME", 4);
							szClass+=COMMENT::CHARCLASS2[4].c_str();
							bReqClass = TRUE;
						}
						//add class
						if (( dwReqCharClass & GLCC_GUNNER_M)&&( dwReqCharClass & GLCC_GUNNER_W))
						{
							szClass+=" ";
							//szClass+=ID2GAMEWORD((char*)"ITEM_CLASS_NAME", 5);
							szClass+=COMMENT::CHARCLASS2[5].c_str();
							bReqClass = TRUE;
						}
						if (( dwReqCharClass & GLCC_ASSASSIN_M)&&( dwReqCharClass & GLCC_ASSASSIN_W))
						{
							szClass+=" ";
							//szClass+=ID2GAMEWORD((char*)"ITEM_CLASS_NAME", 6);
							szClass+=COMMENT::CHARCLASS2[6].c_str();
							bReqClass = TRUE;
						}
						if (( dwReqCharClass & GLCC_TRICKER_M)&&( dwReqCharClass & GLCC_TRICKER_W))
						{
							szClass+=" ";
							//szClass+=ID2GAMEWORD((char*)"ITEM_CLASS_NAME", 7);
							szClass+=COMMENT::CHARCLASS2[7].c_str();
							bReqClass = TRUE;
						}
					}
					strClass.Format( "%s", szClass.GetString() );
				}
			}
			strCombineText.Format ( "          %s / %s", strSchool, strClass );
			strSchoolText.Format ( "          %s", strSchool );
			strClassText.Format ( "          %s", strClass );
			CString strCombine;

			if ( bReqSchool )
			{
				CString strCombine;
				BOOL bSCHOOLVALID;
				const GLCHARLOGIC& sCharData = GLGaeaClient::GetInstance().GetCharacterLogic();
				EMSCHOOLFLAG emSCHOOL = index2school(sCharData.m_wSchool);
				DWORD dwReqSchool = pItemData->sBasicOp.dwReqSchool;

				if( sItemCustom.nidDISGUISE != NATIVEID_NULL() )
				{
					SITEM* pItemDisguise = GLItemMan::GetInstance().GetItem ( sItemCustom.nidDISGUISE );
					DWORD dwReqSchool_B = pItemDisguise->sBasicOp.dwReqSchool;
					bSCHOOLVALID =	(dwReqSchool & index2school(pCharacter->m_wSchool) &&
					dwReqSchool_B & index2school(pCharacter->m_wSchool)) ? TRUE : FALSE;
				}
				else bSCHOOLVALID = dwReqSchool & index2school(pCharacter->m_wSchool);

				SITEM* pItemDisguise = GLItemMan::GetInstance().GetItem ( sItemCustom.nidDISGUISE );
							
				if ( bNeedDisguise && bReqClass )
				{
					if( sItemCustom.nidDISGUISE != NATIVEID_NULL() ) //With Costume
					{
						AddTextNoSplit(strCombineText, NS_UITEXTCONTROL::GetEvaluateColor((pItemData->sBasicOp.dwReqCharClass&pCharacter->m_emClass)&& 
																						   (pItemDisguise->sBasicOp.dwReqCharClass&pCharacter->m_emClass)) );
					}
					else
					{
						AddTextNoSplit(strCombineText, NS_UITEXTCONTROL::GetEvaluateColor((pItemData->sBasicOp.dwReqCharClass&pCharacter->m_emClass)) );
					}
				}
				else if ( !bNeedDisguise && bReqClass )
				{
					AddTextNoSplit(strCombineText, NS_UITEXTCONTROL::GetEvaluateColor((pItemData->sBasicOp.dwReqCharClass&pCharacter->m_emClass)) );
							
				}
				else if ( !bNeedDisguise && !bReqClass )
				{
					AddTextNoSplit(strSchoolText, NS_UITEXTCONTROL::GetEvaluateColor( (pItemData->sBasicOp.dwReqSchool & index2school(pCharacter->m_wSchool))));
				}
			}

			//Level Requirement
			{
				if ( pItemData->sBasicOp.wReqLevelDW || pItemData->sBasicOp.wReqLevelUP )
				{
					bool bReqLevel = true;
					strText.Format( "          %s %d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION", 0), pItemData->sBasicOp.wReqLevelDW );
													
					if ( pItemData->sBasicOp.wReqLevelUP )
					{
						strText2.Format ( "~%d",pItemData->sBasicOp.wReqLevelUP );
						strText += strText2; 
						bReqLevel = (pItemData->sBasicOp.wReqLevelUP >= pCharacter->m_wLevel);
					}
					AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pItemData->sBasicOp.wReqLevelDW <= pCharacter->m_wLevel && bReqLevel ) );
				}
			}

			{	
				switch ( emItemType )
				{
				case ITEM_SUIT:
				case ITEM_REVIVE:
				case ITEM_ANTI_DISAPPEAR:
				case ITEM_VEHICLE:
					{
						//Official Stats Requirement UI By CNDev
						{
							CString strText;
							if ( 0 != rItemStats.wPow )
							{
								if ( 0 != rItemStats.wPow && 0 != rItemStats.wSpi )
								{
									CString strText2;
									CString strText3;
									CBasicTextBox::MULTITEXT mlText,mlText2;
									strText2.Format("         %s %d",ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 0 ), rItemStats.wPow);
									strText3.Format("%s %d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 2 ), rItemStats.wSpi);
									mlText.strText[1] = strText2; 
									mlText.strText[2] = " / ";
									mlText.strText[3] = strText3; 
									mlText.dwColor[1] = NS_UITEXTCONTROL::GetEvaluateColor ( rItemStats.wPow <= rCharStats.wPow );
									mlText.dwColor[2] = NS_UITEXTCOLOR::WHITE; 
									mlText.dwColor[3] = NS_UITEXTCONTROL::GetEvaluateColor ( rItemStats.wSpi <= rCharStats.wSpi );
									AddMultiText( mlText );
								}
								else if ( 0 != rItemStats.wPow && 0 != rItemStats.wDex )
								{
									CString strText2;
									CString strText3;
									CBasicTextBox::MULTITEXT mlText,mlText2;
									strText2.Format("          %s %d",ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 0 ), rItemStats.wPow);
									strText3.Format("%s %d",ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 3 ), rItemStats.wDex);
									mlText.strText[1] = strText2; 
									mlText.strText[2] = " / ";
									mlText.strText[3] = strText3; 
									mlText.dwColor[1] = NS_UITEXTCONTROL::GetEvaluateColor ( rItemStats.wPow <= rCharStats.wPow );
									mlText.dwColor[2] = NS_UITEXTCOLOR::WHITE; 
									mlText.dwColor[3] = NS_UITEXTCONTROL::GetEvaluateColor ( rItemStats.wDex <= rCharStats.wDex );
									AddMultiText( mlText );
								}
								else
								{
									strText.Format( "           %s %d ", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 0 ), rItemStats.wPow );
									AddTextNoSplit ( strText, NS_UITEXTCONTROL::GetEvaluateColor ( rItemStats.wPow <= rCharStats.wPow ) );
								}
							}
							else if ( 0 != rItemStats.wDex )
							{
								if ( 0 != rItemStats.wDex && 0 != rItemStats.wSpi  )
								{
									CString strText2;
									CString strText3;
									CBasicTextBox::MULTITEXT mlText,mlText2;
									strText2.Format("          %s %d",ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 3 ), rItemStats.wDex);
									strText3.Format("%s %d",ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 2 ), rItemStats.wSpi);
									mlText.strText[1] = strText2; 
									mlText.strText[2] = " / ";
									mlText.strText[3] = strText3; 
									mlText.dwColor[1] = NS_UITEXTCONTROL::GetEvaluateColor ( rItemStats.wDex <= rCharStats.wDex );
									mlText.dwColor[2] = NS_UITEXTCOLOR::WHITE; 
									mlText.dwColor[3] = NS_UITEXTCONTROL::GetEvaluateColor ( rItemStats.wSpi <= rCharStats.wSpi );
									AddMultiText( mlText );
								}
								else
								{
									strText.Format("          %s %d ", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 3 ), rItemStats.wDex );
									AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( rItemStats.wDex <= rCharStats.wDex ) );
								}
							}
							else if ( 0 != rItemStats.wSpi)
							{
								strText.Format("          %s %d ", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 2 ), rItemStats.wSpi );
								AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( rItemStats.wSpi <= rCharStats.wSpi ) );
							}
							else if ( 0 != rItemStats.wStr )
							{
								strText.Format("          %s %d ", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 1 ), rItemStats.wStr );
								AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( rItemStats.wStr <= rCharStats.wStr ) );
							}
							else if ( 0 != rItemStats.wInt  )
							{
								strText.Format("          %s %d ", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 4 ), rItemStats.wInt );
								AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( rItemStats.wInt <= rCharStats.wInt ) );
							}
							else if ( 0 != rItemStats.wSta )
							{
								strText.Format("          %s %d ", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION_STATS", 5 ), rItemStats.wSta );
								AddTextNoSplit ( strText, NS_UITEXTCONTROL::GetEvaluateColor ( rItemStats.wSta <= rCharStats.wSta ) );
							}
						}
						/*{
							DWORD dwUSERLVL = GLGaeaClient::GetInstance().GetCharacter()->m_dwUserLvl; 
							if ( dwUSERLVL > USER_GM3 )
							{
								if ( sItemCustom.sNativeID.wMainID == 1022 ||
									sItemCustom.sNativeID.wMainID == 1023 )
								{
									if ( sItemCustom.tBORNTIME!=0 )
									{				
										CTime cTime(sItemCustom.tBORNTIME);
										if ( cTime.GetYear()!=1970 )
										{
											CString strBornTime;
											strBornTime = CInnerInterface::GetInstance().MakeString( "%02d/%02d/%02d %02d:%02d",
																												cTime.GetMonth (),
																												cTime.GetDay (), 
																												(cTime.GetYear ()%2000),  
																												cTime.GetHour (), 
																												cTime.GetMinute () );
											strText.Format ( "Date Crafted: %s", strBornTime );			
											AddTextNoSplit ( strText, NS_UITEXTCOLOR::DARKORANGE );
										}
									}
								}
							}
						}*/
						
						
						//Official Costume UI by CNDev
						{
							if ( sItemCustom.tDISGUISE!=0 )
							{				
								CTime cTime(sItemCustom.tDISGUISE);
								if ( cTime.GetYear()!=1970 )
								{
									CString strExpireDate;
									strExpireDate = CInnerInterface::GetInstance().MakeString( ID2GAMEWORD("ITEM_EXPIRE_DATE"),
																										(cTime.GetYear ()%2000), 
																										cTime.GetMonth (), 
																										cTime.GetDay (), 
																										cTime.GetHour (), 
																										cTime.GetMinute () );
									strText.Format ( "%s:%s", ID2GAMEWORD ( "ITEM_BASIC_INFO", 7 ), strExpireDate );			
									AddTextNoSplit ( strText, NS_UITEXTCOLOR::DARKORANGE );
								}
							}
							bool bCostume = pItemData->sBasicOp.IsDISGUISE();
							CString strText, strTextRange;
							if ( pItemData->sSuitOp.emSuit == SUIT_HANDHELD )
							{
								if ( bCostume )
								{
									if ( pItemData->sSuitOp.emAttack != ITEMATT_NOTHING )
									{
										CBasicTextBox::MULTITEXT mlText,mlText2;
										strText.Format("%s(%s)",COMMENT::ITEMSUIT[pItemData->sSuitOp.emSuit].c_str(),COMMENT::ITEMATTACK[pItemData->sSuitOp.emAttack].c_str());
										mlText.strText[1] = strText; 
										mlText.strText[2] = ID2GAMEWORD("ITEM_DISGUISE_BASIC_INFO",0); 
										mlText.dwColor[1] = NS_UITEXTCOLOR::DEFAULT;
										mlText.dwColor[2] = NS_UITEXTCOLOR::PALEGREEN;
										AddMultiText( mlText );

									}
								}
								else
								{
									if ( pItemData->sSuitOp.emAttack != ITEMATT_NOTHING )
									{
										if ( sItemCustom.nidDISGUISE!=SNATIVEID(false) )
										{
											CBasicTextBox::MULTITEXT mlText,mlText2;
											strText.Format("%s(%s)",COMMENT::ITEMSUIT[pItemData->sSuitOp.emSuit].c_str(),COMMENT::ITEMATTACK[pItemData->sSuitOp.emAttack].c_str());
											mlText.strText[1] = strText; 
											mlText.strText[2] = ID2GAMEWORD("ITEM_DISGUISE_BASIC_INFO",0); 
											mlText.dwColor[1] = NS_UITEXTCOLOR::DEFAULT;
											mlText.dwColor[2] = NS_UITEXTCOLOR::PALEGREEN;
											AddMultiText( mlText );

											if ( sItemCustom.nidDISGUISE!=SNATIVEID(false) )
											{
												SITEM* pDisguiseData = GLItemMan::GetInstance().GetItem ( sItemCustom.nidDISGUISE );

												strText.Format ( "%s :(%s)", ID2GAMEWORD ( "ITEM_BASIC_INFO", 3 ), pDisguiseData->GetName() );			
												AddTextNoSplit ( strText, NS_UITEXTCOLOR::PALEGREEN );
											}
										}
										else
										{
											strText.Format("%s(%s)",COMMENT::ITEMSUIT[pItemData->sSuitOp.emSuit].c_str(), COMMENT::ITEMATTACK[pItemData->sSuitOp.emAttack].c_str());
											AddTextNoSplit (strText, NS_UITEXTCOLOR::WHITE );
										}
									}
								}
							}
							else
							{
								if ( bCostume )
								{
									CBasicTextBox::MULTITEXT mlText,mlText2;
									strText.Format("%s", COMMENT::ITEMSUIT[pItemData->sSuitOp.emSuit].c_str() );
									mlText.strText[1] = strText; 
									mlText.strText[2] = ID2GAMEWORD("ITEM_DISGUISE_BASIC_INFO",0); 
									mlText.dwColor[1] = NS_UITEXTCOLOR::DEFAULT;
									mlText.dwColor[2] = NS_UITEXTCOLOR::PALEGREEN;
									AddMultiText( mlText );
								}
								else
								{
									if ( sItemCustom.nidDISGUISE!=SNATIVEID(false) )
									{
										CBasicTextBox::MULTITEXT mlText,mlText2;
										strText.Format("%s", COMMENT::ITEMSUIT[pItemData->sSuitOp.emSuit].c_str() );
										mlText.strText[1] = strText; 
										mlText.strText[2] = ID2GAMEWORD("ITEM_DISGUISE_BASIC_INFO",0); 
										mlText.dwColor[1] = NS_UITEXTCOLOR::DEFAULT;
										mlText.dwColor[2] = NS_UITEXTCOLOR::PALEGREEN;
										AddMultiText( mlText );

										if ( sItemCustom.nidDISGUISE!=SNATIVEID(false) )
										{
											SITEM* pDisguiseData = GLItemMan::GetInstance().GetItem ( sItemCustom.nidDISGUISE );

											strText.Format ( "%s :%s", ID2GAMEWORD ( "ITEM_BASIC_INFO", 3 ), pDisguiseData->GetName() );			
											AddTextNoSplit ( strText, NS_UITEXTCOLOR::PALEGREEN );
										}
									}
									else
									{
										strText.Format("%s", COMMENT::ITEMSUIT[pItemData->sSuitOp.emSuit].c_str() );
										AddTextNoSplit (strText, NS_UITEXTCOLOR::WHITE );
									}
								}
							}
							GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
							if ( pCharacter && pCharacter->IsCoolTime( pItemData->sBasicOp.sNativeID ) )
							{
								CString strTime = "";
								__time64_t tCurTime =  GLGaeaClient::GetInstance().GetCurrentTime().GetTime();
								__time64_t tCoolTime = pCharacter->GetMaxCoolTime( pItemData->sBasicOp.sNativeID );				

								CTimeSpan cReTime( tCoolTime - tCurTime );

								if ( cReTime.GetHours() > 0 )	
									strTime += CInnerInterface::GetInstance().MakeString( "%d%s ", cReTime.GetHours(), ID2GAMEWORD ( "ITEM_BASIC_TIME", 0 ) );
								if ( cReTime.GetMinutes() > 0 )	
									strTime += CInnerInterface::GetInstance().MakeString( "%d%s ", cReTime.GetMinutes(), ID2GAMEWORD ( "ITEM_BASIC_TIME", 1 ) );
								if ( cReTime.GetSeconds() > 0 )	
									strTime += CInnerInterface::GetInstance().MakeString( "%d%s ", cReTime.GetSeconds(), ID2GAMEWORD ( "ITEM_BASIC_TIME", 2 ) );

								strText.Format ( "%s : %s", ID2GAMEWORD ( "ITEM_BASIC_INFO", 9 ), strTime );			
								AddTextNoSplit ( strText, NS_UITEXTCOLOR::DARKORANGE );
							}
						}

						{
							
							bool bCostume = pItemData->sBasicOp.IsDISGUISE();

							if ( !bCostume )
							{
								BYTE uGRADE = 0;
								
								GLDWDATA &sDamage = sItemCustom.getdamage();
								nExtraValue = sItemCustom.GETGRADE_DAMAGE();
								uGRADE = sItemCustom.GETGRADE(EMGRINDING_DAMAGE);
			//					int nDirect = sItemCustom.GETDAMAGE_DIR();
								AddInfoItemAddonRangeNew ( sDamage.dwLow, sDamage.dwHigh, nExtraValue, uGRADE,0, ID2GAMEWORD("ITEM_ADVANCED_INFO", 0) );

								//	기력치
								nExtraValue = sItemCustom.GETGRADE_DAMAGE();
								int nDirectEnergy = sItemCustom.GETADDENERGY();
								CString strText2;
								CString strText3;
								
								CBasicTextBox::MULTITEXT mlText;
								if ( nExtraValue && nDirectEnergy )
								{
									uGRADE = sItemCustom.GETGRADE(EMGRINDING_DAMAGE);

									
									strText.Format ( "%s",ID2GAMEWORD("ITEM_ADVANCED_INFO",8) );
									strText3.Format ( "+%d", nExtraValue );
									strText2.Format ("%s", GetNumberWithSign(nDirectEnergy));
									//APPEND_ITEM_GRADE ( strText, uGRADE );
									mlText.strText[1] = strText;
									mlText.strText[2] = strText3;
									mlText.strText[3] = strText2;
									mlText.dwColor[1] = NS_UITEXTCOLOR::WHITE;
									mlText.dwColor[2] = NS_UITEXTCOLOR::PRIVATE;
									mlText.dwColor[3] = NS_UITEXTCOLOR::DARKTURQUOISE;
									AddMultiText( mlText );
									//AddTextMultiColorSplit3 ( strText, NS_UITEXTCOLOR::DEFAULT,strText3, NS_UITEXTCOLOR::PRIVATE, strText2 ,NS_UITEXTCOLOR::DARKTURQUOISE  );
								} 
								else if ( nDirectEnergy  )
								{
									uGRADE = sItemCustom.GETGRADE(EMGRINDING_DAMAGE);
									strText.Format ( "%s",ID2GAMEWORD("ITEM_ADVANCED_INFO",8) );
									strText3.Format ( "+%d", nExtraValue );
									strText2.Format ("%s", GetNumberWithSign(nDirectEnergy));
									//APPEND_ITEM_GRADE ( strText, uGRADE );
									mlText.strText[1] = strText;
									mlText.strText[2] = strText3;
									mlText.strText[3] = strText2;
									mlText.dwColor[1] = NS_UITEXTCOLOR::WHITE;
									mlText.dwColor[2] = NS_UITEXTCOLOR::PRIVATE;
									mlText.dwColor[3] = NS_UITEXTCOLOR::DARKTURQUOISE;
									AddMultiText( mlText );
									//AddTextMultiColorSplit3 ( strText, NS_UITEXTCOLOR::DEFAULT,strText3, NS_UITEXTCOLOR::PRIVATE, strText2 ,NS_UITEXTCOLOR::DARKTURQUOISE  );
								}	
								else if ( nExtraValue )
								{
									uGRADE = sItemCustom.GETGRADE(EMGRINDING_DAMAGE);
									strText.Format ( "%s",ID2GAMEWORD("ITEM_ADVANCED_INFO",8) );
									strText2.Format ("+%d", nExtraValue);
									//APPEND_ITEM_GRADE ( strText, uGRADE );
									mlText.strText[1] = strText;
									mlText.strText[2] = strText2;
									mlText.dwColor[1] = NS_UITEXTCOLOR::DEFAULT;
									mlText.dwColor[2] = NS_UITEXTCOLOR::PRIVATE;
									AddMultiText( mlText );
									//AddTextMultiColorSplit ( strText, NS_UITEXTCOLOR::DEFAULT,strText2, NS_UITEXTCOLOR::PRIVATE  );
								}



								//	방어력
								short nDefense = sItemCustom.getdefense();
								nExtraValue = sItemCustom.GETGRADE_DEFENSE();
								uGRADE = sItemCustom.GETGRADE(EMGRINDING_DEFENSE);
			//7					int nDirectDef = sItemCustom.GETDEFENSE_PER();
								AddInfoItemAddon ( nDefense, nExtraValue, uGRADE, 0, ID2GAMEWORD("ITEM_ADVANCED_INFO", 1) );

								if ( sItemCustom.GETHITRATE() )
								{
									nExtraValue = 0;
									AddInfoItemAddon ( sItemCustom.GETHITRATE(), nExtraValue, ID2GAMEWORD("ITEM_ADVANCED_INFO", 3) );		
								}

								if ( sItemCustom.GETAVOIDRATE() )
								{
									nExtraValue = 0;
									AddInfoItemAddon ( sItemCustom.GETAVOIDRATE(), nExtraValue, ID2GAMEWORD("ITEM_ADVANCED_INFO", 4) );
								}

								const WORD wReqSP = sItemCustom.GETREQ_SP();
								if ( 0 < wReqSP )
								{
									strText.Format("%s :%d", ID2GAMEWORD("ITEM_ADVANCED_INFO", 7), wReqSP );
									AddTextNoSplit ( strText, NS_UITEXTCOLOR::WHITE  );
								}				

								if ( pItemData->sBasicOp.wReqPA )
								{
									strText.Format("%s:%d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION", 1), pItemData->sBasicOp.wReqPA );
									AddTextNoSplit ( strText, NS_UITEXTCONTROL::GetEvaluateColor ( pItemData->sBasicOp.wReqPA <= pCharacter->m_wSUM_PA ) );
								}
								if ( pItemData->sBasicOp.wReqSA )
								{
									strText.Format("%s:%d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION", 2 ), pItemData->sBasicOp.wReqSA );
									AddTextNoSplit(strText,NS_UITEXTCONTROL::GetEvaluateColor ( pItemData->sBasicOp.wReqSA <= pCharacter->m_wSUM_SA ));
								}
							}
						}
					}
					break;
				};
			}
			
			AddTextNoSplit ( "", NS_UITEXTCOLOR::DARKORANGE );	
			AddItemTurnInfo ( sItemCustom, bInMarket, bInPrivateMarket );

			if ( pItemData->sBasicOp.IsCoolTime() )
			{
				CString strTime = "";
				CTimeSpan cCoolTime( pItemData->sBasicOp.dwCoolTime );
				
				if ( cCoolTime.GetHours() > 0 )	
					strTime += CInnerInterface::GetInstance().MakeString( "%d%s ", cCoolTime.GetHours(), ID2GAMEWORD ( "ITEM_BASIC_TIME", 0 ) );
				if ( cCoolTime.GetMinutes() > 0 )	
					strTime += CInnerInterface::GetInstance().MakeString( "%d%s ", cCoolTime.GetMinutes(), ID2GAMEWORD ( "ITEM_BASIC_TIME", 1 ) );
				if ( cCoolTime.GetSeconds() > 0 )	
					strTime += CInnerInterface::GetInstance().MakeString( "%d%s ", cCoolTime.GetSeconds(), ID2GAMEWORD ( "ITEM_BASIC_TIME", 2 ) );

				strText.Format ( "%s : %s", ID2GAMEWORD ( "ITEM_BASIC_INFO", 8 ), strTime );			
				AddTextNoSplit ( strText, NS_UITEXTCOLOR::DARKORANGE );	
			}
			switch ( emItemType )
			{
			case ITEM_SUIT:
			case ITEM_REVIVE:
			case ITEM_ANTI_DISAPPEAR:
				{
					{
						for(int i = 0; i < 4; i++)
						{
							if ( !sItemCustom.IsSetRandOpt() ) 
								break;

							CString strDescText;
							EMRANDOM_OPT emOpt[4] = 
							{
								sItemCustom.GETOptTYPE1()
								, sItemCustom.GETOptTYPE2()
								, sItemCustom.GETOptTYPE3()
								, sItemCustom.GETOptTYPE4()
							};

							INT nRandOptType = emOpt[i];

							if( (EMR_OPT_NULL < nRandOptType) && (nRandOptType < EMR_OPT_SIZE) )
							{
								float fVal[4] =
								{
									sItemCustom.GETOptVALUE1()
									, sItemCustom.GETOptVALUE2()
									, sItemCustom.GETOptVALUE3()
									, sItemCustom.GETOptVALUE4()
								};

								strDescText.Format( "%s:", ID2GAMEWORD("ITEM_RANDOM_OPTION", nRandOptType ) );
								if( fVal[i] != 0.0f )
								{
									if ( nRandOptType == EMR_OPT_HMS_INC || 
										nRandOptType == EMR_OPT_HP_INC  || 
										nRandOptType == EMR_OPT_MP_INC  || 
										nRandOptType == EMR_OPT_SP_INC	 )
									{
										fVal[i] = (fVal[i]*100.0f);
									}

									

									if( fVal[i] > 0.0f) 
										strDescText += _T('+');

									if( sItemCustom.IsPerRandOpt( nRandOptType ) )
									{
										strDescText.AppendFormat( "%.2f", fVal[i] );
										strDescText += _T('%');
									}
									else if ( nRandOptType == EMR_OPT_HIT_VOL || 
										nRandOptType == EMR_OPT_AVOID_VOL 	 )
									{
										strDescText.AppendFormat( "%.2f", fVal[i] );
									}
									else
										strDescText.AppendFormat( "%d", int(fVal[i] ));

									AddTextNoSplit( strDescText, NS_UITEXTCOLOR::DARKTURQUOISE );
								}
							}
						}
					}
				}
				break;
			case ITEM_CHARM:
				{
					AddTextNoSplit( "",NS_UITEXTCOLOR::WHITE);
					if ( pItemData->sBasicOp.wReqLevelDW || pItemData->sBasicOp.wReqLevelUP )
					{
						bool bReqLevel = true;
						strText.Format( "%s:%d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION", 0), pItemData->sBasicOp.wReqLevelDW );
											
						if ( pItemData->sBasicOp.wReqLevelUP )
						{
							strText2.Format ( "~%d",pItemData->sBasicOp.wReqLevelUP );
							strText += strText2; 
							bReqLevel = (pItemData->sBasicOp.wReqLevelUP >= pCharacter->m_wLevel);
						}

						AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pItemData->sBasicOp.wReqLevelDW <= pCharacter->m_wLevel && bReqLevel ) );
					}



					GLDWDATA &sDAMAGE = sItemCustom.GETDAMAGE();
					if ( sDAMAGE.dwLow || sDAMAGE.dwHigh  )
					{
						strText.Format("Talisman %s :%s ~ %s ", ID2GAMEWORD("ITEM_ARROW_INFO", 0 ), 
													GetNumberWithSign ( sDAMAGE.dwLow ), 
													GetNumberWithSign ( sDAMAGE.dwHigh ) );

						AddTextNoSplit(strText,NS_UITEXTCOLOR::WHITE);
					}


 
					AddTextAddValue( sItemCustom, pItemData->sSuitOp );					
				}
				break;

			case ITEM_ARROW:
				{
					AddTextNoSplit( "",NS_UITEXTCOLOR::WHITE);
					if ( pItemData->sBasicOp.wReqLevelDW || pItemData->sBasicOp.wReqLevelUP )
					{
						bool bReqLevel = true;
						strText.Format( "%s:%d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION", 0), pItemData->sBasicOp.wReqLevelDW );
											
						if ( pItemData->sBasicOp.wReqLevelUP )
						{
							strText2.Format ( "~%d",pItemData->sBasicOp.wReqLevelUP );
							strText += strText2; 
							bReqLevel = (pItemData->sBasicOp.wReqLevelUP >= pCharacter->m_wLevel);
						}

						AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pItemData->sBasicOp.wReqLevelDW <= pCharacter->m_wLevel && bReqLevel ) );
					}

					GLDWDATA &sDAMAGE = sItemCustom.GETDAMAGE();
					if ( sDAMAGE.dwLow || sDAMAGE.dwHigh  )
					{
						strText.Format("Arrow %s :%s ~ %s ", ID2GAMEWORD("ITEM_ARROW_INFO", 0 ), 
													GetNumberWithSign ( sDAMAGE.dwLow ), 
													GetNumberWithSign ( sDAMAGE.dwHigh ) );

						AddTextNoSplit(strText,NS_UITEXTCOLOR::WHITE);
					}
 
					AddTextAddValue( sItemCustom, pItemData->sSuitOp );					
				}
				break;
				//add bullet
			case ITEM_BULLET:
				{
					AddTextNoSplit( "",NS_UITEXTCOLOR::WHITE);
					if ( pItemData->sBasicOp.wReqLevelDW || pItemData->sBasicOp.wReqLevelUP )
					{
						bool bReqLevel = true;
						strText.Format( "%s:%d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION", 0), pItemData->sBasicOp.wReqLevelDW );
											
						if ( pItemData->sBasicOp.wReqLevelUP )
						{
							strText2.Format ( "~%d",pItemData->sBasicOp.wReqLevelUP );
							strText += strText2; 
							bReqLevel = (pItemData->sBasicOp.wReqLevelUP >= pCharacter->m_wLevel);
						}

						AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pItemData->sBasicOp.wReqLevelDW <= pCharacter->m_wLevel && bReqLevel ) );
					}

					GLDWDATA &sDAMAGE = sItemCustom.GETDAMAGE();
					if ( sDAMAGE.dwLow || sDAMAGE.dwHigh  )
					{
						strText.Format("Bullet %s :%s ~ %s ", ID2GAMEWORD("ITEM_ARROW_INFO", 0 ), 
													GetNumberWithSign ( sDAMAGE.dwLow ), 
													GetNumberWithSign ( sDAMAGE.dwHigh ) );

						AddTextNoSplit(strText,NS_UITEXTCOLOR::WHITE);
					}
 
					AddTextAddValue( sItemCustom, pItemData->sSuitOp );					
				}
				break;
				case ITEM_REMODEL:
				case ITEM_SEALED_CARD:
					break;

			case ITEM_CURE:
				{
					if( pItemData->sDrugOp.wCureVolume != 0 )
					{
						strText.Format("%s :%d",COMMENT::ITEMDRUG[pItemData->sDrugOp.emDrug].c_str(), pItemData->sDrugOp.wCureVolume);
					}

					AddTextNoSplit(strText, NS_UITEXTCOLOR::WHITE);
				}
				break;

			case ITEM_SKILL:
				{
					AddInfoSkillItem ( sItemCustom );
				}
				break;

			case ITEM_PET_SKILL:
				{
					AddTextNoSplit( "",NS_UITEXTCOLOR::WHITE);
					AddInfoPetSkillItem( sItemCustom );
				}
				break;


			case ITEM_GRINDING:
			{
				AddTextNoSplit(ID2GAMEWORD("ITEM_CATEGORY_EX", 0), NS_UITEXTCOLOR::LIGHTSKYBLUE);
			}
			break;

			case ITEM_VIETNAM_EXPGET:
				{
					if ( pItemData->sDrugOp.bRatio )
					{
						strText.Format(ID2GAMEWORD ("ITEM_CATEGORY_VIETNAM_EXP", 1) , pItemData->sDrugOp.wCureVolume );						
					}
					else
					{
						strText.Format(ID2GAMEWORD ("ITEM_CATEGORY_VIETNAM_EXP", 0), pItemData->sDrugOp.wCureVolume );
					}

					AddTextNoSplit(strText,NS_UITEXTCOLOR::LIGHTSKYBLUE);
				}
				break;
			case ITEM_VIETNAM_ITEMGET:
				{
					strText.Format(ID2GAMEWORD ("ITEM_CATEGORY_VIETNAM_ITEM", 0) , pItemData->sDrugOp.wCureVolume );
					AddTextNoSplit(strText,NS_UITEXTCOLOR::LIGHTSKYBLUE);
				}
				break;

			case ITEM_VEHICLE:
				{					
					EMITEM_VAR emITEM_VOL = pItemData->sSuitOp.sVOLUME.emTYPE;

					if ( emITEM_VOL != EMVAR_NONE)
					{
						if ( emITEM_VOL == EMVAR_MOVE_SPEED )
						{
							float fVOLUME = sItemCustom.GETMOVESPEED();
							strText.Format("%s :%.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD_VOL", emITEM_VOL ), fVOLUME );
							AddTextNoSplit(strText,NS_UITEXTCOLOR::GREEN );
						}
						else
						{
							float fVOLUME = pItemData->sSuitOp.sVOLUME.fVolume;
							strText.Format("%s :%.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD_VOL", emITEM_VOL ), fVOLUME );
							AddTextNoSplit(strText,NS_UITEXTCOLOR::PRIVATE);
						}
					}


					bool bInfo(true);

					
					VEHICLEITEMINFO_MAP_ITER iter = GLGaeaClient::GetInstance().GetCharacter()->m_mapVEHICLEItemInfo.find ( sItemCustom.dwVehicleID );
					if ( iter == GLGaeaClient::GetInstance().GetCharacter()->m_mapVEHICLEItemInfo.end() )
					{
						iter = GLGaeaClient::GetInstance().GetCharacter()->m_mapVEHICLEItemInfoTemp.find ( sItemCustom.dwVehicleID );
						if ( iter == GLGaeaClient::GetInstance().GetCharacter()->m_mapVEHICLEItemInfoTemp.end() )	bInfo = false;
					}


					if ( bInfo ) 
					{
						SVEHICLEITEMINFO sVehicleItemInfo = (*iter).second;
						for ( int i = 0; i < VEHICLE_ACCETYPE_SIZE; ++i ) 
						{
							SITEM* pItem = GLItemMan::GetInstance().GetItem ( sVehicleItemInfo.m_PutOnItems[i].sNativeID );
							if ( pItem )
							{
								strText.Format("%s %s", ID2GAMEWORD ("ITEM_VEHICLE_SLOT",i), pItem->GetName () );
								AddTextNoSplit(strText,NS_UITEXTCOLOR::WHITE);
							}
						}

						strText.Format( "%s %.2f%%", ID2GAMEWORD ("ITEM_VEHICLE_BATTERY",0), sVehicleItemInfo.m_nFull/10.0f );
						AddTextNoSplit(strText,NS_UITEXTCOLOR::WHITE);
					}			

					AddTextAddValue( sItemCustom, pItemData->sSuitOp );

					for(int i = 0; i < 4; i++)
					{
						if ( !sItemCustom.IsSetRandOpt() ) 
							break;

						CString strDescText;
						EMRANDOM_OPT emOpt[4] = 
						{
							sItemCustom.GETOptTYPE1()
							, sItemCustom.GETOptTYPE2()
							, sItemCustom.GETOptTYPE3()
							, sItemCustom.GETOptTYPE4()
						};

						INT nRandOptType = emOpt[i];

						if( (EMR_OPT_NULL < nRandOptType) && (nRandOptType < EMR_OPT_SIZE) )
						{
							float fVal[4] =
							{
								sItemCustom.GETOptVALUE1()
								, sItemCustom.GETOptVALUE2()
								, sItemCustom.GETOptVALUE3()
								, sItemCustom.GETOptVALUE4()
							};

							strDescText.Format( "%s:", ID2GAMEWORD("ITEM_RANDOM_OPTION", nRandOptType ) );
							if( fVal[i] != 0.0f )
							{
								if ( nRandOptType == EMR_OPT_HMS_INC || 
									nRandOptType == EMR_OPT_HP_INC  || 
									nRandOptType == EMR_OPT_MP_INC  || 
									nRandOptType == EMR_OPT_SP_INC	 )
								{
									fVal[i] = (fVal[i]*100.0f);
								}

								if( fVal[i] > 0.0f) 
									strDescText += _T('+');

								if( sItemCustom.IsPerRandOpt( nRandOptType ) )
								{
									strDescText.AppendFormat( "%.2f", fVal[i] );
									strDescText += _T('%');
								}
								else
									strDescText.AppendFormat( "%d", int(fVal[i] ));

								AddTextNoSplit( strDescText, NS_UITEXTCOLOR::DARKTURQUOISE );
							}
						}
					}
				}
				break;

			case ITEM_FOOD:
				{
					//const TCHAR * pszComment(NULL);
					SNATIVEID sNativeID = sItemCustom.sNativeID;
					SITEM* pItemData = GLItemMan::GetInstance().GetItem ( sNativeID );
					SNATIVEID sSkillID = pItemData->sSkillBookOp.sSkill_ID;

					PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sSkillID.wMainID, sSkillID.wSubID );
					//if ( pSkill ) pszComment = pSkill->GetDesc();
					const SKILL::SAPPLY& sAPPLY = pSkill->m_sAPPLY;
					if ( pSkill )	
					{
						BOOL bADDED = FALSE;
						for( int nImpact=0; nImpact<SKILL::MAX_IMPACT; ++nImpact )
						{
							if ( sAPPLY.sImpacts[nImpact].emADDON != EMIMPACTA_NONE )
							{
								WORD wLevel = 0;

								const float fADDON_VAR = sAPPLY.sImpacts[nImpact].fADDON_VAR[wLevel];
								if ( 0 != fADDON_VAR )
								{
									float fADDON_VAR_SCALE = fADDON_VAR * COMMENT::IMPACT_ADDON_SCALE[sAPPLY.sImpacts[nImpact].emADDON];

									if ( COMMENT::IsIMPACT_ADDON_PER(sAPPLY.sImpacts[nImpact].emADDON) )
									{
										strText.Format ( "%s:%2.2f%%", COMMENT::IMPACT_ADDON[sAPPLY.sImpacts[nImpact].emADDON].c_str(), fADDON_VAR_SCALE );
									}
									else
									{
										strText.Format ( "%s:%2.2f", COMMENT::IMPACT_ADDON[sAPPLY.sImpacts[nImpact].emADDON].c_str(), fADDON_VAR_SCALE );
									}

									AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE );
								}
							}
						}
					}
					if (pSkill)
					{
						WORD wLevel = 0;

						BOOL bADDED = FALSE;
						for( int nSpec=0; nSpec<SKILL::MAX_SPEC; ++nSpec )
						{
							const SKILL::SSPEC& sSPEC = sAPPLY.sSpecs[nSpec].sSPEC[wLevel];

							if ( sAPPLY.sSpecs[nSpec].emSPEC != EMSPECA_NULL )
							{
								if ( sAPPLY.sSpecs[nSpec].emSPEC == EMSPECA_PROHIBIT_POTION ) continue;

								strText.Format ( "%s:%s", ID2GAMEWORD("SKILL_SPEC_TYPE", 0), COMMENT::SPEC_ADDON[sAPPLY.sSpecs[nSpec].emSPEC].c_str() );
								AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE );

								strText.Empty();
								CString strTemp;

								if ( sAPPLY.sSpecs[nSpec].emSPEC == EMSPECA_NONBLOW || sAPPLY.sSpecs[nSpec].emSPEC == EMSPECA_RECBLOW )
								{
									if ( sSPEC.dwFLAG & DIS_NUMB )
									{
										strTemp.Format ( "%s ", COMMENT::BLOW[EMBLOW_NUMB].c_str() );
										strText += strTemp;
									}
									if ( sSPEC.dwFLAG & DIS_STUN )
									{
										strTemp.Format ( "%s ", COMMENT::BLOW[EMBLOW_STUN].c_str() );
										strText += strTemp;
									}
									if ( sSPEC.dwFLAG & DIS_STONE )
									{
										strTemp.Format ( "%s ", COMMENT::BLOW[EMBLOW_STONE].c_str() );
										strText += strTemp;
									}
									if ( sSPEC.dwFLAG & DIS_BURN )
									{
										strTemp.Format ( "%s ", COMMENT::BLOW[EMBLOW_BURN].c_str() );
										strText += strTemp;
									}
									if ( sSPEC.dwFLAG & DIS_FROZEN )
									{
										strTemp.Format ( "%s ", COMMENT::BLOW[EMBLOW_FROZEN].c_str() );
										strText += strTemp;
									}
									if ( sSPEC.dwFLAG & DIS_MAD )
									{
										strTemp.Format ( "%s ", COMMENT::BLOW[EMBLOW_MAD].c_str() );
										strText += strTemp;
									}
									if ( sSPEC.dwFLAG & DIS_POISON )
									{
										strTemp.Format ( "%s ", COMMENT::BLOW[EMBLOW_POISON].c_str() );
										strText += strTemp;
									}
									if ( sSPEC.dwFLAG & DIS_CURSE )
									{
										strTemp.Format ( "%s ", COMMENT::BLOW[EMBLOW_CURSE].c_str() );
										strText += strTemp;
									}
								}
								// 발동 스킬
								else if ( sAPPLY.sSpecs[nSpec].emSPEC == EMSPECA_DEFENSE_SKILL_ACTIVE )
								{
									GLSKILL* pSkillTemp = GLSkillMan::GetInstance().GetData( sSPEC.dwNativeID );
									if ( pSkillTemp )
									{
										strTemp.Format ( "%s Lv%d", pSkillTemp->GetName(),sSPEC.dwFLAG );
										strText += strTemp;
									}
								}
								/*debuff skill logic, Juver, 2017/06/10 */
								else if ( sAPPLY.sSpecs[nSpec].emSPEC == EMSPECA_BUFF_REMOVE )
								{
									strTemp.Format ( "%s", COMMENT::SKILL_ACTION_TYPE[sSPEC.dwFLAG].c_str() );
									strText += strTemp;
								}

								if ( strText.GetLength () )
								{
									AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE );
								}

								// 특수효과가 없음 일때는 화면에 보여주지 않도록 임시 수정~
								// 특수효과가 없음 일때는 툴에서 직접 값에 0을 넣도록 바꾸는게 좋다 ( 준혁 )
								if ( 0 != sSPEC.fVAR1 )
								{
									if( COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[sAPPLY.sSpecs[nSpec].emSPEC] )
									{
										float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[sAPPLY.sSpecs[nSpec].emSPEC];

										if ( COMMENT::IsSPEC_ADDON1_PER(sAPPLY.sSpecs[nSpec].emSPEC) )
											strText.Format ( "%s:%2.2f%%", COMMENT::SPEC_ADDON_VAR1[sAPPLY.sSpecs[nSpec].emSPEC].c_str(), fVAR1_SCALE );
										else
											strText.Format ( "%s:%2.2f", COMMENT::SPEC_ADDON_VAR1[sAPPLY.sSpecs[nSpec].emSPEC].c_str(), fVAR1_SCALE );

										AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE );
									}
								}

								if ( 0 != sSPEC.fVAR2 )
								{
									if( COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[sAPPLY.sSpecs[nSpec].emSPEC] )
									{
										float fVAR2_SCALE = sSPEC.fVAR2 * COMMENT::SPEC_ADDON_VAR2_SCALE[sAPPLY.sSpecs[nSpec].emSPEC];

										if ( COMMENT::IsSPEC_ADDON2_PER(sAPPLY.sSpecs[nSpec].emSPEC) )
											strText.Format ( "%s:%2.1f%%", COMMENT::SPEC_ADDON_VAR2[sAPPLY.sSpecs[nSpec].emSPEC].c_str(), fVAR2_SCALE );
										else
											strText.Format ( "%s:%2.1f", COMMENT::SPEC_ADDON_VAR2[sAPPLY.sSpecs[nSpec].emSPEC].c_str(), fVAR2_SCALE );

										AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE );
									}
								}				
							}
						}
					}
				}
				break;

			case ITEM_PET_CARD:
				{
					AddTextNoSplit( "",NS_UITEXTCOLOR::WHITE);
					if ( sItemCustom.dwPetID == 0 ) break;
					PETCARDINFO_MAP_ITER iter = GLGaeaClient::GetInstance().GetCharacter()->m_mapPETCardInfo.find ( sItemCustom.dwPetID );
					if ( iter == GLGaeaClient::GetInstance().GetCharacter()->m_mapPETCardInfo.end() )
					{
						iter = GLGaeaClient::GetInstance().GetCharacter()->m_mapPETCardInfoTemp.find ( sItemCustom.dwPetID );
						if ( iter == GLGaeaClient::GetInstance().GetCharacter()->m_mapPETCardInfoTemp.end() ) break;
					}

					SPETCARDINFO sPetCardInfo = (*iter).second;

					strText.Format( "%s %s", ID2GAMEWORD ("ITEM_PET_NAME",0), sPetCardInfo.m_szName );
					AddTextNoSplit(strText,NS_UITEXTCOLOR::WHITE);

					strText.Format( "%s %s", ID2GAMEWORD ("ITEM_PET_TYPE",0), COMMENT::PET_TYPE[sPetCardInfo.m_emTYPE].c_str() );
					AddTextNoSplit(strText,NS_UITEXTCOLOR::WHITE);

					strText.Format( "%s %.2f%%", ID2GAMEWORD ("ITEM_PET_FULL",0), sPetCardInfo.m_nFull/10.0f );
					AddTextNoSplit(strText,NS_UITEXTCOLOR::WHITE);
					
					SITEM* pItem = GLItemMan::GetInstance().GetItem ( sPetCardInfo.m_PutOnItems[PET_ACCETYPE_A].sNativeID );
					if ( pItem )
					{
						strText.Format("%s %s", ID2GAMEWORD ("ITEM_PET_SLOTA",0), pItem->GetName () );
						AddTextNoSplit(strText,NS_UITEXTCOLOR::WHITE);
					}

					pItem = GLItemMan::GetInstance().GetItem ( sPetCardInfo.m_PutOnItems[PET_ACCETYPE_B].sNativeID );
					if ( pItem )
					{
						strText.Format("%s %s", ID2GAMEWORD ("ITEM_PET_SLOTB",0), pItem->GetName () );
						AddTextNoSplit(strText,NS_UITEXTCOLOR::WHITE);
					}

					if ( sItemCustom.tDISGUISE != 0 )
					{
						CTime currentTime = GLGaeaClient::GetInstance().GetCurrentTime();
						CTime startTime   = sItemCustom.tBORNTIME;
						CTimeSpan timeSpan = currentTime - startTime;
						if( timeSpan.GetTotalSeconds() < sItemCustom.tDISGUISE )
						{
							strText.Format("[%s]", ID2GAMEWORD ("ITEM_PET_USE_SKINPACK",0) );
							AddTextNoSplit(strText,NS_UITEXTCOLOR::GOLD);
							strText.Format("%s : %dsec", ID2GAMEWORD ("CLUB_BATTLE_TIME",0), sItemCustom.tDISGUISE - timeSpan.GetTotalSeconds() );
							AddTextNoSplit(strText,NS_UITEXTCOLOR::GOLD);
						}
					}

					AddTextNoSplit( ID2GAMEWORD ("ITEM_PET_SKILLS", 0 ), NS_UITEXTCOLOR::WHITE );

					PETSKILL_MAP_CITER pos = sPetCardInfo.m_ExpSkills.begin();
					PETSKILL_MAP_CITER pos_end = sPetCardInfo.m_ExpSkills.end();
					for ( ;pos != pos_end; ++pos )
					{
						const PETSKILL& sPetSkill = (*pos).second;
						PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sPetSkill.sNativeID );
						if ( pSKILL )
						{
							strText.Format("%s", pSKILL->GetName() );
							AddTextNoSplit(strText,NS_UITEXTCOLOR::WHITE);
						}
					}
				}
				break;

			case ITEM_CARD_DUALPETSKILL: 
				break;

				/*case ITEM_SUMMON_CARD:
				{
					AddTextNoSplit("",NS_UITEXTCOLOR::WHITE);
					if ( pItemData->sBasicOp.sSumID!=SNATIVEID(false) )
					{
						SCROWDATA* pCrow =GLCrowDataMan::GetInstance().GetCrowData ( pItemData->sBasicOp.sSumID );

						if (pCrow)
						{
							{
								strText.Format("%s %s ", ID2GAMEWORD ("ITEM_SUMMON_INFO",0), pCrow->GetName () );
								AddTextNoSplit(strText,NS_UITEXTCOLOR::WHITE);
							}

							{
								strText.Format("%s %s ", ID2GAMEWORD ("ITEM_SUMMON_INFO",1),COMMENT::SUMMONTYPE[pItemData->sBasicOp.m_emSumType].c_str() );
								AddTextNoSplit(strText,NS_UITEXTCOLOR::WHITE);
							}

							{
								strText.Format("%s %d", ID2GAMEWORD ("ITEM_SUMMON_INFO",2), pCrow->m_sBasic.m_dwHP );
								AddTextNoSplit(strText,NS_UITEXTCOLOR::WHITE);
							}

							{
									strText.Format("%s %d ~ %d"
									, ID2GAMEWORD ("ITEM_SUMMON_INFO",3)
									, pCrow->m_sCrowAttack[0].sDamage.dwLow
									, pCrow->m_sCrowAttack[0].sDamage.dwHigh
									);
								AddTextNoSplit(strText,NS_UITEXTCOLOR::WHITE);
							}
						}
					}

				}
				break;*/
			};
			if( emItemType != ITEM_PET_SKILL )
			{
				if( pItemData->sDrugOp.emDrug == ITEM_DRUG_CALL_REGEN )
				{
					AddTextNoSplit("",NS_UITEXTCOLOR::WHITE);
					GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();	

					CString strName( GLGaeaClient::GetInstance().GetMapName( pCharacter->m_sStartMapID ) );
					if( strName == _T("(null)") ) strName.Empty();

					strText.Format( "%s :%s", ID2GAMEWORD("ITEM_CALL_MAP", 0 ), strName );
					AddTextNoSplit( strText, NS_UITEXTCOLOR::WHITE);

					SMAPNODE *pMapNode = GLGaeaClient::GetInstance().FindMapNode ( pCharacter->m_sStartMapID );
					if( pMapNode )
					{
						GLLevelFile cLevelFile;
						BOOL bOk = cLevelFile.LoadFile( pMapNode->strFile.c_str(), TRUE, NULL );
						if( bOk )
						{	
							D3DXVECTOR3 vStartPos;
							PDXLANDGATE pGate = cLevelFile.GetLandGateMan().FindLandGate( pCharacter->m_dwStartGate );
							if( pGate )
							{
								vStartPos = pGate->GetGenPos ( DxLandGate::GEN_RENDUM );
							}
							else
							{
								pGate = cLevelFile.GetLandGateMan().FindLandGate( DWORD(0) );
								if( pGate ) vStartPos = pGate->GetGenPos ( DxLandGate::GEN_RENDUM );
							}

							int nPosX(0), nPosY(0);
							cLevelFile.GetMapAxisInfo().Convert2MapPos( vStartPos.x, vStartPos.z, nPosX, nPosY );

							strText.Format( "%s:%d,%d", ID2GAMEWORD("ITEM_CALL_MAP", 1 ), nPosX, nPosY );
							AddTextNoSplit( strText, NS_UITEXTCOLOR::WHITE);
						}
					}
				}
				else if( pItemData->sDrugOp.emDrug == ITEM_DRUG_CALL_LASTCALL )
				{
					AddTextNoSplit("",NS_UITEXTCOLOR::WHITE);
					GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();

					if( pCharacter->m_sLastCallMapID.IsValidNativeID() )
					{
						CString strName( GLGaeaClient::GetInstance().GetMapName( pCharacter->m_sLastCallMapID ) );
						if( strName == _T("(null)") ) strName.Empty();

						strText.Format( "%s:%s", ID2GAMEWORD("ITEM_CALL_MAP", 0 ), strName );
						AddTextNoSplit( strText, NS_UITEXTCOLOR::WHITE);

						SMAPNODE *pMapNode = GLGaeaClient::GetInstance().FindMapNode ( pCharacter->m_sLastCallMapID );
						if( pMapNode )
						{
							GLLevelFile cLevelFile;
							BOOL bOk = cLevelFile.LoadFile( pMapNode->strFile.c_str(), TRUE, NULL );
							if( bOk )
							{	
								int nPosX(0), nPosY(0);
								cLevelFile.GetMapAxisInfo().Convert2MapPos( pCharacter->m_vLastCallPos.x, pCharacter->m_vLastCallPos.z, nPosX, nPosY );

								strText.Format( "%s:%d,%d", ID2GAMEWORD("ITEM_CALL_MAP", 1 ), nPosX, nPosY );
								AddTextNoSplit( strText, NS_UITEXTCOLOR::WHITE);
							}
						}
					}
					else
					{
						AddTextNoSplit("",NS_UITEXTCOLOR::WHITE);
						AddTextNoSplit( ID2GAMEWORD("ITEM_CALL_MAP", 2 ), NS_UITEXTCOLOR::RED);
					}
				}
				else if ( pItemData->sDrugOp.emDrug == ITEM_DRUG_CALL_TELEPORT )
				{
					CString strName( GLGaeaClient::GetInstance().GetMapName( pItemData->sBasicOp.sSubID ) );
					if( strName == _T("(null)") ) strName.Empty();

					strText.Format( "           %s :%s", ID2GAMEWORD("ITEM_TELEPORT_MAP", 0 ), strName );
					AddTextNoSplit( strText, NS_UITEXTCOLOR::WHITE);

					strText.Format( "%s :%d,%d", ID2GAMEWORD("ITEM_TELEPORT_MAP", 1 ), pItemData->sBasicOp.wPosX, pItemData->sBasicOp.wPosY );
					AddTextNoSplit( strText, NS_UITEXTCOLOR::WHITE);
				}

				BOOL bAccept(FALSE);

				if( sItemCustom.nidDISGUISE != NATIVEID_NULL() )
				{
					bAccept = pCharacter->ACCEPT_ITEM( sItemCustom.sNativeID, sItemCustom.nidDISGUISE );
				}
				else
				{
					bAccept = pCharacter->ACCEPT_ITEM( sItemCustom.sNativeID );
				}

				if ( bAccept )
				{
					DWORD dwNeedSP = pCharacter->CALC_ACCEPTP ( sItemCustom.sNativeID );
					if ( 0 < dwNeedSP )
					{
						strText.Format ( "%s:%d", ID2GAMEWORD("ITEM_NEEDSP" ), dwNeedSP );
						AddTextNoSplit(strText, NS_UITEXTCOLOR::RED );
					}
				}
			}
			{
				switch ( emItemType )
				{
				case ITEM_SUIT:
				case ITEM_REVIVE:
				case ITEM_ANTI_DISAPPEAR:
					{
						AddTextAddValue( sItemCustom, pItemData->sSuitOp );

						{
							BOOL bSPAC = sItemCustom.GETINCHP() || sItemCustom.GETINCMP() || sItemCustom.GETINCSP() || sItemCustom.GETINCAP();
							BOOL bVAR_HP(FALSE), bVAR_MP(FALSE), bVAR_SP(FALSE), bVAR_AP(FALSE);

							//
							EMITEM_VAR emITEM_VOL = pItemData->sSuitOp.sVOLUME.emTYPE;
							if ( (emITEM_VOL!=EMVAR_NONE) || bSPAC )
							{
							}
							
							if ( sItemCustom.nidDISGUISE!=SNATIVEID(false) )
							{
								SITEM* pJDisguiseData = GLItemMan::GetInstance().GetItem ( sItemCustom.nidDISGUISE );
								if ( pJDisguiseData )
								{
							//		if ( pJDisguiseData->sBasicOp.IsCostumeCombine() )
									for ( DWORD svaron=0; svaron<ITEM::SSUIT::VARIATION_SIZE; ++svaron )
									{
										EMITEM_VAR emITEM_VAR = pItemData->sSuitOp.sVARIATE[svaron].emTYPE;
										float fVAR_SCALE(0);
										if (emITEM_VAR != EMVAR_NONE)
										{
											EMITEM_VAR emITEMCOS_VAR = pJDisguiseData->sSuitOp.sVARIATE[svaron].emTYPE;
									//add jitem

											float fVAR_SCALE(0);
											if ( emITEMCOS_VAR != EMVAR_NONE )
											{			
												
												fVAR_SCALE = pJDisguiseData->sSuitOp.sVARIATE[svaron].fVariate;
												CString strText2;
												fVAR_SCALE *= COMMENT::ITEMVAR_SCALE[emITEMCOS_VAR];
												strText2.Format("Costume ADD:%s %2.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD", emITEMCOS_VAR ), fVAR_SCALE );
												if ( COMMENT::IsITEMVAR_SCALE(emITEMCOS_VAR) )		strText2 += "%";
												AddTextNoSplit(strText2,NS_UITEXTCOLOR::GREEN);
											}
										}
									}

									EMITEM_VAR emITEMCOS_VOL = pJDisguiseData->sSuitOp.sVOLUME.emTYPE;

									if ( emITEMCOS_VOL != EMVAR_NONE)
									{
										CString strText3;
										float fVOLUME = pJDisguiseData->sSuitOp.sVOLUME.fVolume;
										strText3.Format("Costume ADD:%s %.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD_VOL", emITEMCOS_VOL ), fVOLUME );
										AddTextNoSplit(strText3,NS_UITEXTCOLOR::GREEN);
									}
								}
							}

											
							for ( DWORD svaron=0; svaron<ITEM::SSUIT::VARIATION_SIZE; ++svaron )
							{
								EMITEM_VAR emITEM_VAR = pItemData->sSuitOp.sVARIATE[svaron].emTYPE;
								float fVAR_SCALE(0);
								if (emITEM_VAR != EMVAR_NONE)
								{
									float fVAR_SCALE(0);
									if ( emITEM_VAR != EMVAR_NONE )
									{
										switch ( emITEM_VAR )
										{
										case EMVAR_HP:
											bVAR_HP = TRUE;
											fVAR_SCALE = sItemCustom.GETINCHP();
											break;
										case EMVAR_MP:
											bVAR_MP = TRUE;
											fVAR_SCALE = sItemCustom.GETINCMP();
											break;
										case EMVAR_SP:
											bVAR_SP = TRUE;
											fVAR_SCALE = sItemCustom.GETINCSP();
											break;
										case EMVAR_AP:
											bVAR_AP = TRUE;
											fVAR_SCALE = sItemCustom.GETINCAP();
											break;
										default:
											fVAR_SCALE = pItemData->sSuitOp.sVARIATE[svaron].fVariate;
											break;
										};

										fVAR_SCALE *= COMMENT::ITEMVAR_SCALE[emITEM_VAR];
										strText.Format("%s %2.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD", emITEM_VAR ), fVAR_SCALE );
										if ( COMMENT::IsITEMVAR_SCALE(emITEM_VAR) )		strText += "%";
										AddTextNoSplit(strText,NS_UITEXTCOLOR::GREEN);
									}

									if ( !bVAR_HP && sItemCustom.GETINCHP() )
									{
										fVAR_SCALE = sItemCustom.GETINCHP();
										emITEM_VAR = EMVAR_HP;
										fVAR_SCALE *= COMMENT::ITEMVAR_SCALE[emITEM_VAR];
										strText.Format("%s %2.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD", emITEM_VAR ), fVAR_SCALE );
										if ( COMMENT::IsITEMVAR_SCALE(emITEM_VAR) )		strText += "%";
										AddTextNoSplit(strText,NS_UITEXTCOLOR::GREEN);
									}

									if ( !bVAR_MP && sItemCustom.GETINCMP() )
									{
										fVAR_SCALE = sItemCustom.GETINCMP();
										emITEM_VAR = EMVAR_MP;
										fVAR_SCALE *= COMMENT::ITEMVAR_SCALE[emITEM_VAR];
										strText.Format("%s %2.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD", emITEM_VAR ), fVAR_SCALE );
										if ( COMMENT::IsITEMVAR_SCALE(emITEM_VAR) )		strText += "%";
										AddTextNoSplit(strText,NS_UITEXTCOLOR::GREEN);
									}

									if ( !bVAR_SP && sItemCustom.GETINCSP() )
									{
										fVAR_SCALE = sItemCustom.GETINCSP();
										emITEM_VAR = EMVAR_SP;
										fVAR_SCALE *= COMMENT::ITEMVAR_SCALE[emITEM_VAR];
										strText.Format("%s %2.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD", emITEM_VAR ), fVAR_SCALE );
										if ( COMMENT::IsITEMVAR_SCALE(emITEM_VAR) )		strText += "%";
										AddTextNoSplit(strText,NS_UITEXTCOLOR::GREEN);
									}

									if ( !bVAR_AP && sItemCustom.GETINCAP() )
									{
										fVAR_SCALE = sItemCustom.GETINCAP();
										emITEM_VAR = EMVAR_AP;
										fVAR_SCALE *= COMMENT::ITEMVAR_SCALE[emITEM_VAR];
										strText.Format("%s %2.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD", emITEM_VAR ), fVAR_SCALE );
										if ( COMMENT::IsITEMVAR_SCALE(emITEM_VAR) )		strText += "%";
										AddTextNoSplit(strText,NS_UITEXTCOLOR::GREEN);
									}

									float fExpMultiple = pItemData->GetExpMultiple();
									if( fExpMultiple != 1.0f )
									{
										strText.Format("%s :+%2.2f", ID2GAMEWORD( "ITEM_ADVANCED_INFO_SPECIAL_EX", 0 ), fExpMultiple);
										strText += "%";

										AddTextNoSplit(strText,NS_UITEXTCOLOR::GREEN);
									}
								}
							}

							if ( emITEM_VOL != EMVAR_NONE)
							{
								if ( emITEM_VOL == EMVAR_MOVE_SPEED )
								{
									float fVOLUME = sItemCustom.GETMOVESPEED();
									strText.Format("%s %.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD_VOL", emITEM_VOL ), fVOLUME );
									AddTextNoSplit(strText,NS_UITEXTCOLOR::GREEN);
								}
								else
								{
									float fVOLUME = pItemData->sSuitOp.sVOLUME.fVolume;
									strText.Format("%s %.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD_VOL", emITEM_VOL ), fVOLUME );
									AddTextNoSplit(strText,NS_UITEXTCOLOR::GREEN);
								}
								
							}
						}


						const int nELEC   = sItemCustom.GETRESIST_ELEC();
						const int nFIRE   = sItemCustom.GETRESIST_FIRE();
						const int nICE    = sItemCustom.GETRESIST_ICE();
						const int nPOISON = sItemCustom.GETRESIST_POISON();
						const int nSPIRIT = sItemCustom.GETRESIST_SPIRIT();

						if ( nELEC || nFIRE || nICE || nPOISON || nSPIRIT )
						{
							strText.Format ("");

							if ( nELEC )
							{
								strText.AppendFormat("%s:%d", ID2GAMEWORD("ITEM_ADVANCED_INFO_RESIST", 0 ), nELEC );	
								BYTE uGRADE = sItemCustom.GETGRADE(EMGRINDING_RESIST_ELEC);
								APPEND_ITEM_GRADE ( strText, uGRADE );
							}
							if ( nFIRE )
							{
								strText.AppendFormat(" %s:%d", ID2GAMEWORD("ITEM_ADVANCED_INFO_RESIST", 1 ), nFIRE );
								BYTE uGRADE = sItemCustom.GETGRADE(EMGRINDING_RESIST_FIRE);
								APPEND_ITEM_GRADE ( strText, uGRADE );
							}
							if ( nICE )
							{
								strText.AppendFormat(" %s:%d", ID2GAMEWORD("ITEM_ADVANCED_INFO_RESIST", 2 ), nICE );
								BYTE uGRADE = sItemCustom.GETGRADE(EMGRINDING_RESIST_ICE);
								APPEND_ITEM_GRADE ( strText, uGRADE );
							}
							if ( nPOISON )
							{
								strText.AppendFormat(" %s:%d", ID2GAMEWORD("ITEM_ADVANCED_INFO_RESIST", 3 ), nPOISON );
								BYTE uGRADE = sItemCustom.GETGRADE(EMGRINDING_RESIST_POISON);
								APPEND_ITEM_GRADE ( strText, uGRADE );
							}
							if ( nSPIRIT )
							{
								strText.AppendFormat(" %s:%d", ID2GAMEWORD("ITEM_ADVANCED_INFO_RESIST", 4 ), nSPIRIT );
								BYTE uGRADE = sItemCustom.GETGRADE(EMGRINDING_RESIST_SPIRIT);
								APPEND_ITEM_GRADE ( strText, uGRADE );
							}

							CString strGet;
							strGet.Format ("%s:%s", ID2GAMEWORD("ITEM_CATEGORY",3),strText);

							AddTextNoSplit( "", NS_UITEXTCOLOR::RED );
							AddTextNoSplit(strGet,NS_UITEXTCOLOR::GREEN);
						}
					}					
					break;
				};
			}
			{
					
				//Item Weapon Effect Official UI By CNDev
				EMSTATE_BLOW emBLOW = pItemData->sSuitOp.sBLOW.emTYPE;
				float fVAR1 = pItemData->sSuitOp.sBLOW.fVAR1 * COMMENT::BLOW_VAR1_SCALE[emBLOW];
				float fVAR2 = pItemData->sSuitOp.sBLOW.fVAR2 * COMMENT::BLOW_VAR2_SCALE[emBLOW];
				if ( emBLOW !=EMBLOW_NONE )
				{
					AddTextNoSplit(" ", NS_UITEXTCOLOR::LIGHTSKYBLUE);
					if (emBLOW == EMBLOW_NUMB)
					{
						strText.Format("%.0f", pItemData->sSuitOp.sBLOW.fRATE);
						strText += (ID2GAMEWORD("ITEM_EFFECT_NUMB", 0));
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						strText.Format(ID2GAMEWORD("ITEM_EFFECT_NUMB", 1), fVAR1);
						strText += (ID2GAMEWORD("ITEM_EFFECT_NUMB", 2));
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						strText.Format(ID2GAMEWORD("ITEM_EFFECT_NUMB", 3), fVAR2, pItemData->sSuitOp.sBLOW.fLIFE);
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						AddTextNoSplit(ID2GAMEWORD("ITEM_EFFECT_NUMB", 4), NS_UITEXTCOLOR::LIGHTYELLOW);
					}
					else if (emBLOW == EMBLOW_FROZEN)
					{
						strText.Format("%.0f", pItemData->sSuitOp.sBLOW.fRATE);
						strText += (ID2GAMEWORD("ITEM_EFFECT_FROZEN", 0));
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						strText.Format(ID2GAMEWORD("ITEM_EFFECT_FROZEN", 1), fVAR1);
						strText += ID2GAMEWORD("ITEM_EFFECT_FROZEN", 2);
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						strText.Format(ID2GAMEWORD("ITEM_EFFECT_FROZEN", 3), pItemData->sSuitOp.sBLOW.fLIFE, fVAR2);
						strText += ID2GAMEWORD("ITEM_EFFECT_FROZEN", 4);
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
					}
					else if (emBLOW == EMBLOW_STUN)
					{
						strText.Format("%.0f", pItemData->sSuitOp.sBLOW.fRATE);
						strText += (ID2GAMEWORD("ITEM_EFFECT_STUN", 0));
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						strText.Format(ID2GAMEWORD("ITEM_EFFECT_STUN", 1), pItemData->sSuitOp.sBLOW.fLIFE);
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
					}
					else if (emBLOW == EMBLOW_BURN)
					{
						strText.Format("%.0f", pItemData->sSuitOp.sBLOW.fRATE);
						strText += (ID2GAMEWORD("ITEM_EFFECT_BURN", 0));
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						strText.Format(ID2GAMEWORD("ITEM_EFFECT_BURN", 1), pItemData->sSuitOp.sBLOW.fLIFE, fVAR2);
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						strText.Format(ID2GAMEWORD("ITEM_EFFECT_BURN", 2));
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
					}
					else if (emBLOW == EMBLOW_POISON)
					{
						strText.Format("%.0f", pItemData->sSuitOp.sBLOW.fRATE);
						strText += (ID2GAMEWORD("ITEM_EFFECT_POISON", 0));
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						strText.Format(ID2GAMEWORD("ITEM_EFFECT_POISON", 1), pItemData->sSuitOp.sBLOW.fLIFE, fVAR2);
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
					}
					else if (emBLOW == EMBLOW_STONE)
					{
						strText.Format("%.0f", pItemData->sSuitOp.sBLOW.fRATE);
						strText += ID2GAMEWORD("ITEM_EFFECT_STONE", 0);
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						strText.Format(ID2GAMEWORD("ITEM_EFFECT_STONE", 1), fVAR1);
						strText += ID2GAMEWORD("ITEM_EFFECT_STONE", 2);
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						strText.Format(ID2GAMEWORD("ITEM_EFFECT_STONE", 3), fVAR2, pItemData->sSuitOp.sBLOW.fLIFE);
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
					}
					else if (emBLOW == EMBLOW_MAD)
					{
						strText.Format(ID2GAMEWORD("ITEM_EFFECT_MAD", 0), pItemData->sSuitOp.sBLOW.fRATE);
						strText += ID2GAMEWORD("ITEM_EFFECT_MAD", 1);
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						strText.Format(ID2GAMEWORD("ITEM_EFFECT_MAD", 2));
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						strText.Format(ID2GAMEWORD("ITEM_EFFECT_MAD", 3));
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						strText.Format(ID2GAMEWORD("ITEM_EFFECT_MAD", 4), pItemData->sSuitOp.sBLOW.fLIFE, fVAR2);
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
					}
					else 
					{
						strText.Format("%s:%s", ID2GAMEWORD("ITEM_ADVANCED_INFO_ABNORMAL", 0), COMMENT::BLOW[emBLOW].c_str());
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						strText.Format("%s:%.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_ABNORMAL", 1), pItemData->sSuitOp.sBLOW.fLIFE);
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						strText.Format("%s:%.2f%%", ID2GAMEWORD("ITEM_ADVANCED_INFO_ABNORMAL", 2), pItemData->sSuitOp.sBLOW.fRATE);
						AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
						{
							float fVAR1 = pItemData->sSuitOp.sBLOW.fVAR1 * COMMENT::BLOW_VAR1_SCALE[emBLOW];
							if (fVAR1 != 0.0f)
							{
								if (COMMENT::IsBLOW1_PER(emBLOW))
									strText.Format("%s:%.2f%%", COMMENT::BLOW_VAR1[emBLOW].c_str(), fVAR1);
								else
									strText.Format("%s:%.2f", COMMENT::BLOW_VAR1[emBLOW].c_str(), fVAR1);
								AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
							}
						}
						{
							float fVAR2 = pItemData->sSuitOp.sBLOW.fVAR2 * COMMENT::BLOW_VAR2_SCALE[emBLOW];
							if (fVAR2 != 0.0f)
							{
								if (COMMENT::IsBLOW2_PER(emBLOW))
									strText.Format("%s:%.2f%%", COMMENT::BLOW_VAR2[emBLOW].c_str(), fVAR2);
								else
									strText.Format("%s:%.2f", COMMENT::BLOW_VAR2[emBLOW].c_str(), fVAR2);
								AddTextNoSplit(strText, NS_UITEXTCOLOR::LIGHTYELLOW);
							}
						}
					}
				}

				
				if( pItemData->sBasicOp.emItemType == ITEM_SUIT )
				{
				   if ( pItemData->sSuitOp.emSuit == SUIT_HANDHELD)
				   {
					   SNATIVEID sSkillLinkID = sItemCustom.sSkillLinkID;
					   if(sSkillLinkID.IsValidNativeID()) 
					   {
							CString strSkillName;
							PGLSKILL pSkill = GLSkillMan::GetInstance().GetData (sSkillLinkID);
							if(pSkill)
							{
								strSkillName.Format("%s", pSkill->GetName());
								float fRate = 0.0f;
								std::vector<GLWEAPON_LINKSKILL>::size_type i = 0, j = GLCONST_CHAR::vecWeaponLinkSkill.size();
								for( ; i < j; ++i )
								{
									GLWEAPON_LINKSKILL glSkill = GLCONST_CHAR::vecWeaponLinkSkill[i];
									if ( glSkill.sNativeID != sSkillLinkID )
										continue;

									fRate = glSkill.fRate;
									break;
								}
							
								CString strSkillLink;
								strSkillLink.Format("If used on enemy, there is a %0.2f", fRate);
								strSkillLink += "%";

								CString strSkillLink1;
								strSkillLink1.Format("chance of");
								strSkillLink += strSkillLink1;

								CString strCon;
								strCon.Format("'%s' skill effect to take place.", strSkillName);


								AddTextNoSplit (strSkillLink, NS_UITEXTCOLOR::BLUE_WOW );
								AddTextNoSplit (strCon, NS_UITEXTCOLOR::BLUE_WOW );
							}
					   }

				   }
				}
				else if ( pItemData->sBasicOp.isHaveSkill() )
				{
					CString strTest;
					strTest.Format("Able to receive 1 skill.");
					AddTextNoSplit (strTest, NS_UITEXTCOLOR::BLUE_WOW );
				}

				if ( pItemData->sBasicOp.dwReqContributionPoint )
				{
					strText.Format("%s:%d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION", 4 ), pItemData->sBasicOp.dwReqContributionPoint );
					AddTextNoSplit(strText,NS_UITEXTCONTROL::GetEvaluateColor ( pItemData->sBasicOp.dwReqContributionPoint <= pCharacter->m_llContributionPoint ));
				}
				if ( pItemData->sBasicOp.dwReqActivityPoint )
				{
					strText.Format("%s:%d", ID2GAMEWORD("ITEM_ADVANCED_INFO_CONDITION", 3 ), pItemData->sBasicOp.dwReqActivityPoint );
					AddTextNoSplit(strText,NS_UITEXTCONTROL::GetEvaluateColor ( pItemData->sBasicOp.dwReqActivityPoint <= pCharacter->m_dwActivityPoint ));
				}
			}

			if( strlen( pItemData->sRandomOpt.szNAME ) > 3 )
			{
				//AddTextNoSplit("", NS_UITEXTCOLOR::DEFAULT );
				//AddTextNoSplit(ID2GAMEWORD("ITEM_REFORM_SHOW",0), NS_UITEXTCOLOR::DEFAULT );
			}

			

			/*item color, Juver, 2018/01/10 */
			bool bColoring = pItemData->sBasicOp.bItemColor;
			SITEM* pitem_data_disguise = GLItemMan::GetInstance().GetItem( sItemCustom.nidDISGUISE );
			if ( pitem_data_disguise && pitem_data_disguise->sBasicOp.bItemColor )
				bColoring = true;
			
			if ( bColoring )
			{
				AddTextNoSplit ( "", NS_UITEXTCOLOR::WHITE );
				AddTextNoSplit ( ID2GAMEWORD ( "ITEM_INFO_COLOR", 0 ), NS_UITEXTCOLOR::WHITE );

				{
					WORD wCOLOR = sItemCustom.wColor1;
					WORD wR, wG, wB;
					wR = wCOLOR >> 10;
					wG = wCOLOR >> 5;
					wG = wG&0x1f;
					wB = wCOLOR&0x1f;

					wR = (WORD)( wR * 8.225806f );
					wG = (WORD)( wG * 8.225806f );
					wB = (WORD)( wB * 8.225806f );

					DWORD dwColor = 0xff000000 + (wR<<16) + (wG<<8) + wB;
					AddTextNoSplit ( ID2GAMEWORD ( "ITEM_INFO_COLOR", 1 ), dwColor );
				}
				
				{
					WORD wCOLOR = sItemCustom.wColor2;
					WORD wR, wG, wB;
					wR = wCOLOR >> 10;
					wG = wCOLOR >> 5;
					wG = wG&0x1f;
					wB = wCOLOR&0x1f;

					wR = (WORD)( wR * 8.225806f );
					wG = (WORD)( wG * 8.225806f );
					wB = (WORD)( wB * 8.225806f );

					DWORD dwColor = 0xff000000 + (wR<<16) + (wG<<8) + wB;
					AddTextNoSplit ( ID2GAMEWORD ( "ITEM_INFO_COLOR", 2 ), dwColor );
				}
			}

			if ( pItemData->sRandomOpt.bUnableRandomCard )
			{
				AddTextNoSplit ( ID2GAMEINTEXT( "ITEM_INFO_IMPOSSIBLE_REFORM" ), NS_UITEXTCOLOR::RED );
			}

			/*item wrapper, Juver, 2018/01/12 */
			if ( !bWrapped )
			{
				BOOL bWrappable = pItemData->isWrappable();
				if ( sItemCustom.nidDISGUISE != NATIVEID_NULL() )	bWrappable = FALSE;
				if ( !sItemCustom.IsGM_GENITEM() && pItemData->sBasicOp.IsEXCHANGE() )	bWrappable = FALSE;

				if ( bWrappable )
				{
					AddTextNoSplit ( ID2GAMEINTEXT( "ITEM_INFO_POSSIBLE_WRAP" ), NS_UITEXTCOLOR::PRIVATE );
				}
			}
			else
			{
				AddTextNoSplit ( ID2GAMEINTEXT( "ITEM_INFO_POSSIBLE_UNWRAP" ), NS_UITEXTCOLOR::PRIVATE );
			}

			/*item transfer card, Juver, 2018/01/18 */
			if ( pItemData->sBasicOp.bItemTransfer )
			{
				AddTextNoSplit ( ID2GAMEINTEXT( "ITEM_INFO_ITEM_TRANSFER" ), NS_UITEXTCOLOR::PRIVATE );
			}

			{ // ??
				const TCHAR * pszComment(NULL);

				if ( (emItemType != ITEM_SKILL) || (emItemType != ITEM_PET_SKILL) ) pszComment = pItemData->GetComment();
				else
				{
					SNATIVEID sNativeID = sItemCustom.sNativeID;
					SITEM* pItemData = GLItemMan::GetInstance().GetItem ( sNativeID );
					SNATIVEID sSkillID = pItemData->sSkillBookOp.sSkill_ID;

					PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sSkillID.wMainID, sSkillID.wSubID );
					if ( pSkill ) pszComment = pSkill->GetDesc();
				}
				/*nondrop card, Juver, 2017/11/26 */
				if ( sItemCustom.IsGM_GENITEM() )
				{
					AddTextNoSplit( "", NS_UITEXTCOLOR::WHITE );
					AddTextNoSplit ( ID2GAMEINTEXT( "ITEM_INFO_GM_GEN"),  NS_UITEXTCOLOR::GOLD);
				}
				else
				{	//itemflags
					DWORD dwFlags = pItemData->sBasicOp.dwFlags;
					CString strFlag;

					bool bTrash(true);

					bool bSale = pItemData->sBasicOp.IsSALE();
					bool bExchange = pItemData->sBasicOp.IsEXCHANGE();
					bool bLocker = bExchange;
					bool bThrow = pItemData->sBasicOp.IsTHROW();
					bool bCostume = pItemData->sBasicOp.IsDISGUISE();
					bool bTrashCan = bTrash ;

					AddTextNoSplit( "", NS_UITEXTCOLOR::WHITE );

					strFlag.Format("%s %s",ID2GAMEWORD ( "ITEM_INFO_TRADE", 0 ),ID2GAMEWORD ( "ITEM_INFO_TRADE_OPTION", bSale? 0:1 ) );
					AddTextNoSplit ( strFlag.GetString(), bSale? NS_UITEXTCOLOR::WHITE : NS_UITEXTCOLOR::ENABLE );

					strFlag.Format("%s %s",ID2GAMEWORD ( "ITEM_INFO_TRADE", 1 ),ID2GAMEWORD ( "ITEM_INFO_TRADE_OPTION", bThrow? 0:1 ) );
					AddTextNoSplit ( strFlag.GetString(), bThrow? NS_UITEXTCOLOR::WHITE : NS_UITEXTCOLOR::ENABLE );

					strFlag.Format("%s %s",ID2GAMEWORD ( "ITEM_INFO_TRADE", 2 ),ID2GAMEWORD ( "ITEM_INFO_TRADE_OPTION", bLocker? 0:1 ) );
					AddTextNoSplit ( strFlag.GetString(), bLocker? NS_UITEXTCOLOR::WHITE : NS_UITEXTCOLOR::ENABLE );

					strFlag.Format("%s %s",ID2GAMEWORD ( "ITEM_INFO_TRADE", 3 ),ID2GAMEWORD ( "ITEM_INFO_TRADE_OPTION", bExchange? 0:1 ) );
					AddTextNoSplit ( strFlag.GetString(), bExchange? NS_UITEXTCOLOR::WHITE : NS_UITEXTCOLOR::ENABLE );
				}

				if ( pItemData->IsTIMELMT () )
				{		
					AddTextNoSplit ("", NS_UITEXTCOLOR::DARKORANGE );
					CTime cTime(sItemCustom.tBORNTIME);
					if ( cTime.GetYear()!=1970 )
					{
						CTimeSpan sLMT(pItemData->sDrugOp.tTIME_LMT);
						cTime += sLMT;

						CString strExpireDate;
						strExpireDate = CInnerInterface::GetInstance().MakeString ( ID2GAMEWORD("ITEM_EXPIRE_DATE"),
						(cTime.GetYear ()%2000), cTime.GetMonth (), cTime.GetDay (), cTime.GetHour (), cTime.GetMinute () );
						strText.Format ( "%s:%s", ID2GAMEWORD ( "ITEM_BASIC_INFO", 4 ), strExpireDate );			
						AddTextNoSplit ( strText, NS_UITEXTCOLOR::DARKORANGE );
					}
				}
			}
			{
				LONGLONG dwCOMMISSION_MONEY = 0;

				if ( pItemData->sBasicOp.dwBuyPrice || bInPrivateMarket )
				{
					if ( bInPrivateMarket )
					{
						AddTextNoSplit ( "", NS_UITEXTCOLOR::GOLD );
						bool bOPENER;
						DWORD dwPrivateMarketID;
						CInnerInterface::GetInstance().GetPrivateMarketInfo ( bOPENER, dwPrivateMarketID );
						if ( bOPENER )	
						{
							GLPrivateMarket &sPMarket = GLGaeaClient::GetInstance().GetCharacter()->m_sPMarket;

							const SSALEITEM *pSALE = sPMarket.GetItem ( SNATIVEID(wPosX,wPosY) ); 
							if ( pSALE )
							{
								CString strMoney = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT ( pSALE->llPRICE, 3, "," );
								strText.Format ( "%s:%s", ID2GAMEWORD ( "ITEM_BASIC_INFO", 5 ), strMoney );			
								AddTextNoSplit ( strText, NS_UITEXTCOLOR::GOLD );
							}						
						}
						else
						{
							PGLCHARCLIENT pCLIENT = GLGaeaClient::GetInstance().GetChar ( dwPrivateMarketID );
							if ( !pCLIENT ) return ;

							GLPrivateMarket &sPMarket = pCLIENT->m_sPMarket;

							const SSALEITEM *pSALE = sPMarket.GetItem ( SNATIVEID(wPosX,wPosY) ); 
							if ( pSALE )
							{
								CString strMoney = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT ( pSALE->llPRICE, 3, "," );
								strText.Format ( "%s:%s", ID2GAMEWORD ( "ITEM_BASIC_INFO", 5 ), strMoney );			
								AddTextNoSplit ( strText, NS_UITEXTCOLOR::GOLD );
							}
						}
					}				
					else if ( bShopOpen && bInMarket )	// npc open , check item inside inventory ( item price inside npc)
					{
						AddTextNoSplit ( "", NS_UITEXTCOLOR::GOLD );
						LONGLONG dwNpcSellPrice = 0;

						volatile LONGLONG dwPrice = 0;
						volatile float fSHOP_RATE = GLGaeaClient::GetInstance().GetCharacter()->GetBuyRate();
						volatile float fSHOP_RATE_C = fSHOP_RATE * 0.01f;

						PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( sNpcNativeID );
						if( pCrowData )
						{
							LONGLONG dwNpcPrice = pCrowData->GetNpcSellPrice( pItemData->sBasicOp.sNativeID.dwID );		
							if( dwNpcPrice == 0 )
							{								
								dwNpcSellPrice = pItemData->sBasicOp.dwBuyPrice;
								dwPrice = DWORD ( (float)dwNpcSellPrice * fSHOP_RATE_C );
							}
							else
							{
								dwNpcSellPrice = dwNpcPrice;
								dwPrice = dwNpcSellPrice;								
							}
						}
						else
						{
							dwNpcSellPrice = pItemData->sBasicOp.dwBuyPrice;
							dwPrice = DWORD ( (float)dwNpcSellPrice * fSHOP_RATE_C );
						}
									
						CString strMoney = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT ( dwPrice, 3, "," );

						dwCOMMISSION_MONEY = dwPrice - dwNpcSellPrice;
									
						D3DCOLOR dwColor = NS_UITEXTCOLOR::RED;
						strText.Format ( "%s :%s", ID2GAMEWORD ( "ITEM_BASIC_INFO", 1 ), strMoney );	
						if( dwPrice - dwCOMMISSION_MONEY <= GLGaeaClient::GetInstance().GetCharacterLogic().m_lnMoney )
						{
							dwColor = NS_UITEXTCOLOR::PALEGREEN;
						}
						AddTextNoSplit ( strText, dwColor );
					}
					else if ( bShopOpen ) // npc open (check item price of items inside inventory not inside npc)	// 
					{
						volatile float fSHOP_RATE = GLGaeaClient::GetInstance().GetCharacter()->GetSaleRate();
						volatile float fSALE_DISCOUNT = fSHOP_RATE * 0.01f;

						volatile DWORD dwPrice = pItemData->GETSELLPRICE ( sItemCustom.wTurnNum );
						volatile DWORD dwSALE_PRICE = DWORD ( dwPrice * fSALE_DISCOUNT );					

						//	??? ??
						volatile DWORD dwDISPRICE = pItemData->GETSELLPRICE ( sItemCustom.wTurnNum );
						dwCOMMISSION_MONEY = dwDISPRICE - dwSALE_PRICE;

						CString strMoney = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT ( dwSALE_PRICE, 3, "," );
						strText.Format( "%s :%s", ID2GAMEWORD("ITEM_BASIC_INFO", 1), strMoney );

						AddTextNoSplit ( strText, NS_UITEXTCOLOR::PALEGREEN );
					}
					/*npc shop, Juver, 2017/07/26 */
					else if( bNPCShop )
					{
						//from npc shop
						volatile LONGLONG dwPrice = pItemData->sBasicOp.dwBuyPrice;

						CString strMoney = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT ( dwPrice, 3, "," );
						strText.Format( "%s:%s", ID2GAMEWORD("NPC_SHOP_TYPE_PRICE", wNPCShopType ), strMoney );

						D3DCOLOR dwColor = NS_UITEXTCOLOR::RED;
						if( dwPrice <= GLGaeaClient::GetInstance().GetCharacterLogic().m_lnMoney )
						{
							dwColor = NS_UITEXTCOLOR::PALEGREEN;
						}
						AddTextNoSplit ( strText, dwColor );
					}
				}

				if ( 0 != dwCOMMISSION_MONEY )
				{
					CString strMoney = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT ( dwCOMMISSION_MONEY, 3, "," );
					strText.Format( "%s :%s", ID2GAMEWORD("ITEM_BASIC_INFO", 6), strMoney );
					AddTextNoSplit ( strText, NS_UITEXTCOLOR::PALEGREEN );
				}

				{	//Name Display
					CString strName, strCombine;
					DWORD dwLevel = (DWORD)pItemData->sBasicOp.emLevel;
					BYTE uGRADE = sItemCustom.GETGRADE(EMGRINDING_NONE);

					SITEMCUSTOM sCustomWrap = sItemCustom;
					if ( bIsWEAR_ITEM )
					{
						strName.Format ( "        %s(%s)", pItemData->GetName(),ID2GAMEWORD("WEAR_ITEM",0));
					}
					else
					{
						if ( uGRADE == 0 ) 
						{
							if ( pItemData->sBasicOp.emItemType == ITEM_MYSTERY_BOX && sCustomWrap.isBoxLock )
							{
								strName.Format ( "         %s [%s]", pItemData->GetName(), ID2GAMEWORD("ITEM_INFO_MYSTERYBOX",1));
							}
							else if( bWrapped && strorigname.size() )
							{
								strName.Format ( "        %s %s", strorigname.c_str(), pItemData->GetName());
							}
							else
							{
								strName.Format ( "        %s", pItemData->GetName());
							}
						}
						else
						{
							if( bWrapped && strorigname.size() )
							{
								strName.Format ( "        +%d %s %s", uGRADE, strorigname.c_str(), pItemData->GetName());
							}
							else
							{
								strName.Format ( "        +%d %s", uGRADE, pItemData->GetName());
							}
						}
					}

					D3DCOLOR dwCOLOR;
					switch ( pItemData->sBasicOp.emLevel )
					{
						case LEVEL_NORMAL:			{	dwCOLOR = NS_UITEXTCOLOR::dwItemRank0;		}	break;
						case LEVEL_RARE:			{	dwCOLOR = NS_UITEXTCOLOR::dwItemRank1;			}	break;
						case LEVEL_UNIQUE:			{	dwCOLOR = NS_UITEXTCOLOR::dwItemRank2;		}	break;
						case LEVEL_INFINITY:		{	dwCOLOR = NS_UITEXTCOLOR::dwItemRank3;			}	break;
						case LEVEL_FERVOR:			{	dwCOLOR = NS_UITEXTCOLOR::dwItemRank4;		}	break;
						
					}
				}
				{
					if ( sNpcNativeID != sItemCustom_base.sNativeID ) 
					{
						sNpcNativeID = sItemCustom_base.sNativeID;
				
						if ( sNpcNativeID.IsValidNativeID() )
						{
							SITEM* pItem = GLItemMan::GetInstance().GetItem ( sNpcNativeID );
							if ( !pItem )
							{
								GASSERT ( 0 && "" );
								return ;
							}
							SITEMCUSTOM sCUSTOM;
						}
					}
				}
			}
		}
	}
};
