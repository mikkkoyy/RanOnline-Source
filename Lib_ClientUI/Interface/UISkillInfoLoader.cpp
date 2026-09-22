#include "StdAfx.h"
#include "UIInfoLoader.h"
#include "InnerInterface.h"
#include "GLGaeaClient.h"
#include "UITextControl.h"
#include "GameTextControl.h"
#include "GLogicEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace NS_SKILLINFO
{
	SNATIVEID	m_sNativeIDBack;
	BOOL		m_bNextLevel_BEFORE_FRAME = FALSE;

	bool		m_bAdditional = false;
	bool		m_bLink = false;
	bool		m_bWeaponSkill = false; //Weapon Skill 

	GLCHARLOGIC*	m_pDefaultCharacter;	// Default Character

	void RESET ()
	{
		CInnerInterface::GetInstance().CLEAR_TEXT();
		m_sNativeIDBack = NATIVEID_NULL();
		m_bNextLevel_BEFORE_FRAME = FALSE;
		m_bAdditional = false;
		m_bLink = false;
		m_bWeaponSkill = false; //Weapon Skill 
	}

	void AddTextNoSplit ( CString strText, D3DCOLOR dwColor )
	{
		CInnerInterface::GetInstance().ADDTEXT_NOSPLIT ( strText, dwColor );
	}
	     
	void AddTextLongestLineSplit ( CString strText, D3DCOLOR dwColor )
	{
		CInnerInterface::GetInstance().ADDTEXT_LONGESTLINE_SPLIT ( strText, dwColor );
	}

	void RemoveAllInfo ()
	{
		CInnerInterface::GetInstance().CLEAR_TEXT ();
	}

	void LOADLINKSKILL(SNATIVEID sSKILLID, SCHARSKILL* const pCharSkill, const BOOL bNextLevel, bool bLink)
	{
		if (m_bLink == bLink) return;

		RESET();

		m_bLink = bLink;

		CString	strText;
		CString strTemp;

		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData(sSKILLID.wMainID, sSKILLID.wSubID);
		WORD wDispLevel = 0;
		WORD wNextLevel = wDispLevel;
		SKILL::SLEARN& sLEARN = pSkill->m_sLEARN;
		SKILL::SLEARN_LVL& sLVL = sLEARN.sLVL_STEP[wNextLevel];
		if (pCharSkill)
			wDispLevel = pCharSkill->wLevel + 1;
		else
		{
			if (bNextLevel) wDispLevel = 0;
		}

		if (!pSkill)
		{
			return;
		}

		BOOL bMASTER = GLGaeaClient::GetInstance().GetCharacter()->ISMASTER_SKILL(pSkill->m_sBASIC.sNATIVEID);
		{
			CString Filler;
			CString Fill;
			CString strSkillLevel;

			Filler.Format("           ");
			Fill.Format(ID2GAMEWORD("LINK_SKILL_TEXT", 0));
			strTemp.Format("         %s", pSkill->GetName());
			strSkillLevel.Format(" (%d/%s)", wDispLevel, ID2GAMEWORD("SKILL_LEVEL", 3));
			strTemp += strSkillLevel;
			Filler += Fill;
			AddTextNoSplit("", NS_UITEXTCOLOR::GREEN);
			AddTextNoSplit(Filler, NS_UITEXTCOLOR::DARKLBUE);
			AddTextNoSplit("", NS_UITEXTCOLOR::GREEN);
		}

		//normal or passive
		{
			CString strText;
			switch ( pSkill->m_sBASIC.emACTION )
			{
			case SKILL::EMACTION_NORMAL:
				{
					strText = ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 0 );
				}
				break;

			case SKILL::EMACTION_BUFF:
				{
					strText = ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 1  );
				}
				break;

			case SKILL::EMACTION_DEBUFF:
				{
					strText = ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 2  );
				}
				break;

			case SKILL::EMACTION_LIMIT:
				{
					strText = ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 3  );
				}
				break;
			}

			if ( pSkill->m_sBASIC.emROLE == SKILL::EMROLE_NORMAL )
			{
				switch ( pSkill->m_sBASIC.emAPPLY )
				{
				case SKILL::EMAPPLY_PHY_SHORT:
					{
						strText += "/";
						strText += ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 4 );
					}
					break;

				case SKILL::EMAPPLY_PHY_LONG:
					{
						strText += "/";
						strText += ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 5 );
					}
					break;

				case SKILL::EMAPPLY_MAGIC:
					{
						strText += "/";
						strText += ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 6 );
					}
					break;
				}
			}
			else if ( pSkill->m_sBASIC.emROLE == SKILL::EMROLE_PASSIVE )
			{
				strText += "/";
				strText += ID2GAMEWORD( "SKILL_BASIC_INFO", 8 );
			}
			AddTextNoSplit (strText, NS_UITEXTCOLOR::DEFAULT);
		}
		AddTextNoSplit("", NS_UITEXTCOLOR::GREEN);

		const SKILL::SAPPLY& sAPPLY = pSkill->m_sAPPLY;

		//Skill Range
		{
			WORD wLevel = 0;
			if (pCharSkill)
				wLevel = !bNextLevel ? pCharSkill->wLevel : pCharSkill->wLevel + 1;

			AddTextNoSplit(ID2GAMEWORD("SKILL_BASIC_INFO", 12), NS_UITEXTCOLOR::WHITE);

			const SKILL::CDATA_LVL& sDATA_LVL = sAPPLY.sDATA_LVL[wLevel];
			const SKILL::SSKILLBASIC& sBASIC = pSkill->m_sBASIC;


			CString str;
			CString strText;
			switch ( pSkill->m_sBASIC.emIMPACT_TAR )
			{
			case TAR_SELF: // 자신
				switch ( pSkill->m_sBASIC.emIMPACT_REALM )
				{
				case REALM_SELF: // '목표' 자신
					strText += ID2GAMEINTEXT( "SKILL_AREA_SELF_SELF" );
					break;

				case REALM_ZONE: // '목표' 주위
					switch ( pSkill->m_sBASIC.emIMPACT_TAR )
					{
					case SIDE_OUR: // 자기편에게
						if ( 0 < sDATA_LVL.wTARNUM && 0 < sDATA_LVL.wAPPLYRANGE )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_SELF_ZONE_OUR" ),
							sDATA_LVL.wTARNUM, sDATA_LVL.wAPPLYRANGE);
							strText += str;
						}
						break;

					case SIDE_ENEMY: // 적에게
						if ( 0 < sDATA_LVL.wTARNUM && 0 < sBASIC.wTARRANGE )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_SELF_ZONE_ENEMY" ),
							sDATA_LVL.wTARNUM, sBASIC.wTARRANGE );
							strText += str;
						}
						break;
					case SIDE_ANYBODY: // 모두에게
						if ( 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_SELF_ZONE_ANYBODY" ),
							sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
							strText += str;
						}
						break;
					};
				};
				break;

			case TAR_SPEC: // 타겟
				{
					switch ( pSkill->m_sBASIC.emIMPACT_REALM )
					{
					case REALM_SELF: // '목표' 자신
						switch ( pSkill->m_sBASIC.emIMPACT_TAR )
						{
						case SIDE_OUR: // 자기편에게
							if ( 0 < sBASIC.wTARRANGE )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_SELF_OUR" ),
								sBASIC.wTARRANGE );
								strText += str;
							}
							break;
						case SIDE_ENEMY: // 적에게
							if ( 0 < sBASIC.wTARRANGE )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_SELF_ENEMY" ),
								sBASIC.wTARRANGE );
								strText += str;
							}
							break;
						case SIDE_ANYBODY: // 모두에게
							if ( 0 < sBASIC.wTARRANGE )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_SELF_ANYBODY" ),
								sBASIC.wTARRANGE );
								strText += str;
							}
							break;
						};

					case REALM_ZONE: // '목표' 주위
						switch ( pSkill->m_sBASIC.emIMPACT_TAR )
						{
						case SIDE_OUR: // 자기편에게
							if ( 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_ZONE_OUR" ),
								sBASIC.wTARRANGE, sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
								strText += str;
							}
							break;
						case SIDE_ENEMY: // 적에게
							if ( 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_ZONE_ENEMY" ),
								sBASIC.wTARRANGE, sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
								strText += str;
							}
							break;
						case SIDE_ANYBODY: // 모두에게
							if ( 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_ZONE_ANYBODY" ),
								sBASIC.wTARRANGE, sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM  );
								strText += str;
							}
							break;
						};

					case REALM_FANWIZE:
						switch ( pSkill->m_sBASIC.emIMPACT_TAR )
						{
						case SIDE_ENEMY:
							if ( 0 < sDATA_LVL.wTARNUM && 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wAPPLYANGLE )
							{
								//made an exception for disabling 2 aoe desc
								if ( pSkill->m_sBASIC.sNATIVEID.wMainID == 6 && pSkill->m_sBASIC.sNATIVEID.wSubID == 4 ||
									pSkill->m_sBASIC.sNATIVEID.wMainID == 30 && pSkill->m_sBASIC.sNATIVEID.wSubID == 15 ||
									pSkill->m_sBASIC.sNATIVEID.wMainID == 30 && pSkill->m_sBASIC.sNATIVEID.wSubID == 21 ||
									pSkill->m_sBASIC.sNATIVEID.wMainID == 32 && pSkill->m_sBASIC.sNATIVEID.wSubID == 8 )	break;
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_FANWIZE_ENEMY" ),
								sDATA_LVL.wTARNUM, sBASIC.wTARRANGE % 10, sDATA_LVL.wAPPLYANGLE );
								strText += str;
							}
							break;
						};

					};
				}
				break;

			case TAR_SELF_TOSPEC: // 자신부터 타겟까지
				switch ( pSkill->m_sBASIC.emIMPACT_TAR )
				{
				case SIDE_ENEMY:
					//WORD wPierce = sDATA_LVL.wPIERCENUM + pTarget->GETSUM_PIERCE() + 1;
					WORD wPierce = sDATA_LVL.wPIERCENUM + 1;

					if ( 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wTARNUM && 0 < wPierce )
					{
						str.Format( ID2GAMEINTEXT( "SKILL_AREA_TOSPEC_ENEMY" ),
						sBASIC.wTARRANGE, sDATA_LVL.wTARNUM, wPierce );
						strText += str;
					}
					break;
				};
				break;

			case TAR_ZONE: // 자신
				switch ( pSkill->m_sBASIC.emIMPACT_REALM )
				{
				case REALM_ZONE: // '목표' 주위
					if ( 0 < sDATA_LVL.wTARNUM && 0 < sDATA_LVL.wAPPLYRANGE )
					{
						str.Format( ID2GAMEINTEXT( "SKILL_AREA_ZONE_ENEMY" ),
						sDATA_LVL.wTARNUM, sDATA_LVL.wAPPLYRANGE );
						strText += str;
					}
					break;
				};
				break;

			/*case TAR_ZONE: // 지면 위치의 주변 타겟
				switch ( pSkill->m_sBASIC.emIMPACT_REALM )
				{
				case REALM_ZONE:
					switch ( pSkill->m_sBASIC.emIMPACT_TAR )
					{
					case SIDE_OUR:
						if ( 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_ZONE_OUR" ),
							sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
							strText += str;
						}
						break;
					case SIDE_ENEMY:
						if ( 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_ZONE_ENEMY" ),
							sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
							strText += str;
						}
						break;
					case SIDE_ANYBODY:
						if ( 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_ZONE_ANYBODY" ),
							sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
							strText += str;
						}
						break;
					};
					break;
				};
				break;*/

			case TAR_SPECIFIC: // 지면 위치
				strText += ID2GAMEINTEXT( "SKILL_AREA_LAND" );
				break;
			};

			AddTextNoSplit (strText, NS_UITEXTCOLOR::WHITE);
		}

		//Skill Effect
		WORD wLevel = 0;

		if (pCharSkill)
			wLevel = !bNextLevel ? pCharSkill->wLevel : pCharSkill->wLevel + 1;

		const SKILL::CDATA_LVL& sDATA_LVL = sAPPLY.sDATA_LVL[wLevel];

		bool bEFFECT = false;
		bool bEFFIMPACT = false;

		AddTextNoSplit("", NS_UITEXTCOLOR::GREEN);
		if (sDATA_LVL.fBASIC_VAR != 0 || sAPPLY.emSTATE_BLOW != EMBLOW_NONE || bEFFECT || bEFFIMPACT )
			AddTextNoSplit(ID2GAMEWORD("SKILL_BASIC_INFO", 13), NS_UITEXTCOLOR::GREEN);

							
		//Skill Damage
		{
			SKILL::EMROLE emRole = pSkill->m_sBASIC.emROLE;

			bool bNormalDurationPlus = false;
			bool bNormalDurationMinus = false;
			bool bNormalNonDurationPlus = false;
			bool bNormalNonDurationMinus = false;
			bool bPassiveDurationPlus = false;
			bool bPassiveDurationMinus = false;
			bool bPassiveNonDurationPlus = false;
			bool bPassiveNonDurationMinus = false;

			if ( emRole == SKILL::EMROLE_NORMAL )
			{
				if ( sDATA_LVL.fLIFE )
				{
					if ( sDATA_LVL.fBASIC_VAR > 0.0f )
						bNormalDurationPlus = true;
					else if ( sDATA_LVL.fBASIC_VAR < 0.0f )
						bNormalDurationMinus = true;
				}
				else
				{
					if ( sDATA_LVL.fBASIC_VAR > 0.0f )
						bNormalNonDurationPlus = true;
					else if ( sDATA_LVL.fBASIC_VAR < 0.0f )
						bNormalNonDurationMinus = true;
				}
			}
			else if ( emRole == SKILL::EMROLE_PASSIVE )
			{
				if ( sDATA_LVL.fLIFE )
				{
					if ( sDATA_LVL.fBASIC_VAR > 0.0f )
						bPassiveDurationPlus = true;
					else if ( sDATA_LVL.fBASIC_VAR < 0.0f )
						bPassiveDurationMinus = true;
				}
				else
				{
					if ( sDATA_LVL.fBASIC_VAR > 0.0f )
						bPassiveNonDurationPlus = true;
					else if ( sDATA_LVL.fBASIC_VAR < 0.0f )
						bPassiveNonDurationMinus = true;
				}
			}

			CString strTemp;
			CString strText;

            float fBASIC_VAR_SCALE;

			BOOL bSKILL_PER;
			std::string strSKILL_TYPE;
			
			fBASIC_VAR_SCALE = sDATA_LVL.fBASIC_VAR * COMMENT::SKILL_TYPES_SCALE[sAPPLY.emBASIC_TYPE];
			bSKILL_PER = COMMENT::IsSKILL_PER(sAPPLY.emBASIC_TYPE);

			switch ( sAPPLY.emBASIC_TYPE )
            {
				case SKILL::EMFOR_HP://	??? ??.
					{
						if ( bNormalDurationPlus || bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus || bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
								/* for ep11 soon
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_2")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5)
									, static_cast< float >( nSkillDmg )
									, static_cast< float >( gdDamage.dwNow )
									, static_cast< float >( gdDamage.dwMax ) );
									*/
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_MP://	??? ??.
					{
						if ( bNormalDurationPlus || bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus || bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_SP://	??? ??.
					{
						if ( bNormalDurationPlus || bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus || bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_VARHP://	??? ??.
					{
						if ( bNormalDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus ) 
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[3].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[3].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[3].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[3].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[3].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[3].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[3].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[3].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_VARMP://	??? ??.
					{
						if ( bNormalDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus ) 
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[4].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[4].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[4].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[4].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[4].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[4].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[4].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[4].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_VARSP://	??? ??.
					{
						if ( bNormalDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus ) 
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[5].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[5].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[5].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[5].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[5].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[5].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[5].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[5].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_DEFENSE://	??? ??.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[6].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[6].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[6].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[6].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[6].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[6].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[6].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[6].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_CURE://	??. ( ?? ?? ?? )
					{
						strTemp.Format( "%s", ID2GAMEINTEXT("SKILL_INFOR_CURE") );
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_HITRATE://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[8].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[8].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[8].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[8].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[8].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[8].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[8].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[8].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_AVOIDRATE://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[9].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[9].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[9].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[9].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[9].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[9].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[9].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[9].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_VARAP://	?? (??,??,??) ???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2_1")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2_1")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[10], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[10], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[10], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[10], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[10], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[10], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[10], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[10], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_VARDAMAGE://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[11].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[11].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[11].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[11].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[11].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[11].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[11].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[11].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_VARDEFENSE://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[12].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[12].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[12].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[12].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[12].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[12].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[12].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[12].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_PA://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[13].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[13].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[13].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[13].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[13].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[13].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[13].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[13].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_SA://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[14].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[14].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[14].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[14].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[14].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[14].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[14].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[14].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_MA://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[15].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[15].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[15].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[15].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[15].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[15].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[15].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[15].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_HP_RATE://	HP ???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 5), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_MP_RATE://	HP ???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 6), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_SP_RATE://	HP ???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 7), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_RESIST: //	?? ???
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[30], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[30], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[30], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[30], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[30], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[30], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[30], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[30], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_SUMMONTIME: //	???? ??
					{
						if ( bPassiveNonDurationPlus )
                        {
                            strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE5")
                                , COMMENT::SKILL_TYPES[31], sDATA_LVL.fBASIC_VAR );
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE5")
                                , COMMENT::SKILL_TYPES[31], -sDATA_LVL.fBASIC_VAR );
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
			//end of line here to avoid confusion
			};


		}
		// impact add on
		{
			for(int nImpact = 0; nImpact < SKILL::MAX_IMPACT; nImpact++ )
			{
				const EMIMPACT_ADDON emADDON = sAPPLY.sImpacts[nImpact].emADDON;

				float fADDON_VAR = sAPPLY.sImpacts[nImpact].fADDON_VAR[wLevel];
				float fADDON_VAR_SCALE = fADDON_VAR * COMMENT::IMPACT_ADDON_SCALE[sAPPLY.sImpacts[nImpact].emADDON];
				
				SKILL::EMROLE emRole = pSkill->m_sBASIC.emROLE;

				bool bImpactNormalDurationPlus = false;
				bool bImpactNormalDurationMinus = false;
				bool bImpactNormalNonDurationPlus = false;
				bool bImpactNormalNonDurationMinus = false;
				bool bImpactPassiveDurationPlus = false;
				bool bImpactPassiveDurationMinus = false;
				bool bImpactPassiveNonDurationPlus = false;
				bool bImpactPassiveNonDurationMinus = false;

				BOOL bSKILL_PER;
				std::string strIMPACT_ADDON;
				strIMPACT_ADDON = COMMENT::IMPACT_ADDON[emADDON].c_str();

				if ( emRole == SKILL::EMROLE_NORMAL )
				{
					if ( sDATA_LVL.fLIFE )
					{
						if ( fADDON_VAR > 0.0f )
							bImpactNormalDurationPlus = true;
						else if ( fADDON_VAR < 0.0f )
							bImpactNormalDurationMinus = true;
					}
					else
					{
						if ( fADDON_VAR > 0.0f )
							bImpactNormalNonDurationPlus = true;
						else if ( fADDON_VAR < 0.0f )
							bImpactNormalNonDurationMinus = true;
					}
				}
				else if ( emRole == SKILL::EMROLE_PASSIVE )
				{
					if ( sDATA_LVL.fLIFE )
					{
						if ( fADDON_VAR > 0.0f )
							bImpactPassiveDurationPlus = true;
						else if ( fADDON_VAR < 0.0f )
							bImpactPassiveDurationMinus = true;
					}
					else
					{
						if ( fADDON_VAR > 0.0f )
							bImpactPassiveNonDurationPlus = true;
						else if ( fADDON_VAR < 0.0f )
							bImpactPassiveNonDurationMinus = true;
					}
				}

				BOOL bIsIMPACT_ADDON_PER = FALSE;
				bIsIMPACT_ADDON_PER = COMMENT::IsIMPACT_ADDON_PER(emADDON);

				if ( sAPPLY.sImpacts[nImpact].emADDON != EMIMPACTA_NONE )
				{

					if (0 != fADDON_VAR)
					{
						if (!bEFFIMPACT)AddTextLongestLineSplit("", NS_UITEXTCOLOR::LIGHTSKYBLUE);
						bEFFIMPACT = true;

						switch ( emADDON )
						{
							case EMIMPACTA_HITRATE:     // 명중율 변화량.
							case EMIMPACTA_AVOIDRATE:   // 회피율 변화량.
							case EMIMPACTA_DAMAGE:  // 공격치 변화량.
							case EMIMPACTA_DEFENSE: // 방어치 변화량.
							case EMIMPACTA_VARHP:   // HP 변화율.
							case EMIMPACTA_VARMP:   // MP 변화율.
							case EMIMPACTA_VARSP:   // SP 변화율.
							case EMIMPACTA_VARAP:   // HP,MP,SP 변화율.
							case EMIMPACTA_DAMAGE_RATE:     // 공격치 변화율.
							case EMIMPACTA_DEFENSE_RATE:    // 방어치 변화율.
							case EMIMPACTA_PA:      //	격투치.
							case EMIMPACTA_SA:      //	사격치.
							case EMIMPACTA_MA:      //	방어치.
							case EMIMPACTA_HP_RATE: //	HP 증폭율.
							case EMIMPACTA_MP_RATE: //	MP 증폭율.
							case EMIMPACTA_SP_RATE: //	SP 증폭율.
							case EMIMPACTA_RESIST:  //	저항 수치 변경
							case EMIMPACTA_CHANGESTATS:   //	스탯 수치 변경 ( 힘,민첩,정신 )
							case EMIMPACTA_HP_RECOVERY_VAR:
							case EMIMPACTA_MP_RECOVERY_VAR:
							case EMIMPACTA_SP_RECOVERY_VAR:
							case EMIMPACTA_CP_RECOVERY_VAR:
							case EMIMPACTA_CP_AUTO_VAR:
							{
									CString strTemp;
									CString strText;
									if ( bImpactNormalDurationPlus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), fADDON_VAR );
										}
									}
									else if ( bImpactNormalDurationMinus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), -fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), -fADDON_VAR );
										}
									}
									else if ( bImpactNormalNonDurationPlus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
												, strIMPACT_ADDON.c_str(), fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
												, strIMPACT_ADDON.c_str(), fADDON_VAR );
										}
									}
									else if ( bImpactNormalNonDurationMinus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
												, strIMPACT_ADDON.c_str(), -fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
												, strIMPACT_ADDON.c_str(), -fADDON_VAR );
										}
									}
									else if ( bImpactPassiveDurationPlus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), fADDON_VAR );
										}
									}
									else if ( bImpactPassiveDurationMinus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), -fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), -fADDON_VAR );
										}
									}
									else if ( bImpactPassiveNonDurationPlus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
												, strIMPACT_ADDON.c_str(), fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
												, strIMPACT_ADDON.c_str(), fADDON_VAR );
										}
									}
									else if ( bImpactPassiveNonDurationMinus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
												, strIMPACT_ADDON.c_str(), -fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
												, strIMPACT_ADDON.c_str(), -fADDON_VAR );
										}
									}
									strText += strTemp;
									AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
							}
							break;
						};
					}
				}
			}
		}
				{
					for(int nSpec = 0; nSpec < SKILL::MAX_SPEC; ++nSpec)
					{
						const SKILL::SSPEC& sSPEC = sAPPLY.sSpecs[nSpec].sSPEC[wLevel];
						const EMSPEC_ADDON emSPEC = sAPPLY.sSpecs[nSpec].emSPEC;

						
						bool bAddon = false;

						if (emSPEC != EMSPECA_NULL)
						{
							if (!bAddon)AddTextLongestLineSplit("", NS_UITEXTCOLOR::LIGHTSKYBLUE);
							bAddon = true;
							{
								//teleport,dash,shift
								if (emSPEC == EMSPECA_PUSHPULL)
								{
									CString strText1;
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_1"), fVAR2_SCALE);
											}
											else
											{
												strText1.Format(ID2GAMEWORD("SKILL_SPEC_ADDON_F_1_RATE"), fVAR2_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText1, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_REFDAMAGE)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_2"),sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_REBIRTH)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_3"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//hp,mp,sp gather
								if (emSPEC == EMSPECA_HP_GATHER)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_4_RATE"), fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_4"), fVAR1_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_MP_GATHER)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_5_RATE"), fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_5"), fVAR1_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_SP_GATHER)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_6_RATE"), fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_6"), fVAR1_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_HP_DIV)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_7"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_MP_DIV)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_8"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_SP_DIV)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_9"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_NONBLOW )
								{
									CString strTemp;
									CString strText;
									if (sSPEC.dwFLAG & DIS_NUMB)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_NUMB].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_STUN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_STUN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_STONE)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_STONE].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_BURN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_BURN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_FROZEN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_FROZEN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_MAD)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_MAD].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_POISON)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_POISON].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_CURSE)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_CURSE].c_str());
										strText += strTemp;
									}
									CString strTemper;
									strTemper.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_10"), strText);
									AddTextLongestLineSplit(strTemper, NS_UITEXTCOLOR::GREEN);
								}
								
								if (emSPEC == EMSPECA_RECBLOW )
								{
									CString strTemp;
									CString strText;
									if (sSPEC.dwFLAG & DIS_NUMB)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_NUMB].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_STUN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_STUN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_STONE)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_STONE].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_BURN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_BURN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_FROZEN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_FROZEN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_MAD)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_MAD].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_POISON)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_POISON].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_CURSE)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_CURSE].c_str());
										strText += strTemp;
									}
									CString strTemper;
									strTemper.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_11"), strText);
									AddTextLongestLineSplit(strTemper, NS_UITEXTCOLOR::GREEN);
								}
								//emspeca pierce to do by CNDev
								if (emSPEC == EMSPECA_PIERCE)
								{
									CString strText;
									strText.Format("Fix Pierce Skill Description.");
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_TARRANGE )
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_MOVEVELO)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}

								if (emSPEC == EMSPECA_ONWARD)
								{
									CString TextShift;
									TextShift.Format(ID2GAMEWORD("EFF_SKILL_ONWARD"));
									AddTextNoSplit(TextShift, NS_UITEXTCOLOR::GREEN);

									/*if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * -100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_MYSKILLDELAYRATE",1),fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_MYSKILLDELAY",1),fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * COMMENT::SPEC_ADDON_VAR2_SCALE[emSPEC];;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format(ID2GAMEWORD("EFF_SKILL_MYSKILLDELAYRATE",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
											}
											else
											{
												strText1.Format(ID2GAMEWORD("EFF_SKILL_MYSKILLDELAY",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
											}
										}
									}*/
								}
								if (emSPEC == EMSPECA_INVISIBLE)
								{
									CString strText;
									if ( sDATA_LVL.fLIFE < 0 )
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_16_1"));
									}
									else
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_16"), sDATA_LVL.fLIFE);
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_RECVISIBLE)
								{
									CString strText;
									if ( sDATA_LVL.fLIFE < 0 )
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_17_1"));
									}
									else
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_17"), sDATA_LVL.fLIFE);
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
									
								}
								if (emSPEC == EMSPECA_ATTACKVELO)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * -100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//skill delay
								if (emSPEC == EMSPECA_SKILLDELAY)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * -100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_7"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_6"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_5"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_4"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_7_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_6_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_5_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_4_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_CRUSHING_BLOW)
								{
									CString strText;
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_20_1_RATE",0), fVAR2_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_20_1",0), fVAR2_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_PSY_DAMAGE_REDUCE)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_21_1"), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_21"), sDATA_LVL.fLIFE, fVAR1_SCALE);
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_21_1_RATE"), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_21_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_MAGIC_DAMAGE_REDUCE)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_22_1_RATE"), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_22_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_22_1"), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_22"), sDATA_LVL.fLIFE, fVAR1_SCALE);
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								
								if (emSPEC == EMSPECA_PSY_DAMAGE_REFLECTION)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * 100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format("%2.2f%%", fVAR1_SCALE);
											}
											else
											{
												strText.Format("%2.2f", fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format("%2.2f%%", fVAR2_SCALE);
											}
											else
											{
												strText1.Format("%2.2f", fVAR2_SCALE);
											}
										}
									}
									CString strConnect;
									strConnect.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_23"),sDATA_LVL.fLIFE, strText1, strText);
									AddTextLongestLineSplit(strConnect, NS_UITEXTCOLOR::GREEN);
								}

								if (emSPEC == EMSPECA_MAGIC_DAMAGE_REFLECTION)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * 100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format("%2.2f%%", fVAR1_SCALE);
											}
											else
											{
												strText.Format("%2.2f", fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format("%2.2f%%", fVAR2_SCALE);
											}
											else
											{
												strText1.Format("%2.2f", fVAR2_SCALE);
											}
										}
									}
									CString strConnect;
									strConnect.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_24"),sDATA_LVL.fLIFE, strText1, strText);
									AddTextLongestLineSplit(strConnect, NS_UITEXTCOLOR::GREEN);
								}

								//debuffs / dedebuffs
								if (emSPEC == EMSPECA_BUFF_REMOVE)
								{
									CString strText;
									if (pSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF)
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_25_2"));
									}
									else
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_25_1"));
									}
									AddTextLongestLineSplit(strText,NS_UITEXTCOLOR::GREEN);
								}
								//stun
								if (emSPEC == EMSPECA_STUN)
								{
									CString strText;
									if (pSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF)
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_26_2"), sDATA_LVL.fLIFE);
									}
									else
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_26"), sDATA_LVL.fLIFE);
									}
									AddTextLongestLineSplit(strText,NS_UITEXTCOLOR::GREEN);
								}
								//todo cndev defense skill active
								//pull,push,shock
								if (emSPEC == EMSPECA_PULL)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * 100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format("%2.2f%%", fVAR1_SCALE);
											}
											else
											{
												strText.Format("%2.2f", fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format("%2.2f%%", fVAR2_SCALE);
											}
											else
											{
												strText1.Format("%2.2f", fVAR2_SCALE);
											}
										}
									}
									CString strConnect;
									strConnect.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_28"), strText, strText1);
									AddTextLongestLineSplit(strConnect, NS_UITEXTCOLOR::GREEN);
								}
								//pull,push,shock
								if (emSPEC == EMSPECA_PUSH)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * 100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format("%2.2f%%", fVAR1_SCALE);
											}
											else
											{
												strText.Format("%2.2f", fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format("%2.2f%%", fVAR2_SCALE);
											}
											else
											{
												strText1.Format("%2.2f", fVAR2_SCALE);
											}
										}
									}
									CString strConnect;
									strConnect.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_29"), strText, strText1);
									AddTextLongestLineSplit(strConnect, NS_UITEXTCOLOR::GREEN);
								}
								//continous damage
								if (emSPEC == EMSPECA_CONTINUOUS_DAMAGE)
								{
									CString strText;
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_30_RATE",0), sDATA_LVL.fLIFE, fVAR2_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_30",0), sDATA_LVL.fLIFE, fVAR2_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//i left it here
								//curse
								if (emSPEC == EMSPECA_CURSE )
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_31_RATE",0), sDATA_LVL.fLIFE, fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_31",0), sDATA_LVL.fLIFE, fVAR1_SCALE);
											}

											AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
										}
									}
								}
								//range
								if (emSPEC == EMSPECA_CHANGE_ATTACK_RANGE )
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}

								if (emSPEC == EMSPECA_CHANGE_APPLY_RANGE)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_PROHIBIT_POTION)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_34"), sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_PROHIBIT_SKILL)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_35"), sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//emspeca_ignore_damage here
								//start tw buffs no need description
								//EMSPECA_ITEMDROP_RATE
								//EMSPECA_MONEYDROP_RATE
								//end tw buffs no need description
								//emspeca_getexp_rate here -- need rework
								if (emSPEC == EMSPECA_GETEXP_RATE)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_EXPRATE",0), sDATA_LVL.fLIFE,fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_EXP",0), sDATA_LVL.fLIFE,fVAR1_SCALE);
											}

											AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
										}
									}
								}
								//start tw buffs no need description
								//EMSPECA_TALK_TO_NPC
								//EMSPECA_SPECIAL_ITEM_DROP
								//end tw buffs no need description
								if (emSPEC == EMSPECA_TELEPORT)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_42"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_POSITION_SHIFT)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_43_4"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//emspeca_enhancement reinforcement rework
								if (emSPEC == EMSPECA_ENHANCEMENT)
								{
									CString strText;
									CString strText1;
									//if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_REINFORCEMENTRATE",0), sDATA_LVL.fLIFE, fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_REINFORCEMENT",0), sDATA_LVL.fLIFE, fVAR1_SCALE);
											}
										}
									}
									//if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * COMMENT::SPEC_ADDON_VAR2_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format(ID2GAMEWORD("EFF_SKILL_REINFORCEMENTRATE",1), fVAR2_SCALE);
											}
											else
											{
												strText1.Format(ID2GAMEWORD("EFF_SKILL_REINFORCEMENT",1), fVAR2_SCALE);
											}
										}
									}
									AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
									AddTextNoSplit(strText1, NS_UITEXTCOLOR::GREEN);
								}
								//emspeca_illusion to do
								if (emSPEC == EMSPECA_VEHICLE_OFF)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_46_RATE"), fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_46"), fVAR1_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//duration inc
								if (emSPEC == EMSPECA_DURATION_CHANGE)
								{
									CString strText;
									CString strText1;

									if (pSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF)
									{

										if (0 != sSPEC.fVAR1)
										{
											if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
											{
												float fVAR1_SCALE = sSPEC.fVAR1 * 100;

												if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
												{
													strText.Format(ID2GAMEWORD("EFF_SKILL_MYDURATIONINCRATE",1),fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEWORD("EFF_SKILL_MYDURATIONINC",1),fVAR1_SCALE);
												}
											}
										}
										if (0 != sSPEC.fVAR2)
										{
											if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
											{
												float fVAR2_SCALE = sSPEC.fVAR2 * COMMENT::SPEC_ADDON_VAR2_SCALE[emSPEC];;

												if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
												{
													strText1.Format(ID2GAMEWORD("EFF_SKILL_MYDURATIONINCRATE",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
												}
												else
												{
													strText1.Format(ID2GAMEWORD("EFF_SKILL_MYDURATIONINC",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
												}
											}
										}
										AddTextNoSplit(strText1, NS_UITEXTCOLOR::GREEN);
										AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
									}
									else
									{
										if (0 != sSPEC.fVAR1)
										{
											if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
											{
												float fVAR1_SCALE = sSPEC.fVAR1 * 100;

												if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
												{
													strText.Format(ID2GAMEWORD("EFF_SKILL_ENEMYDURATIONINCRATE",1), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEWORD("EFF_SKILL_ENEMYDURATIONINC",1),fVAR1_SCALE);
												}
											}
										}
										if (0 != sSPEC.fVAR2)
										{
											if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
											{
												float fVAR2_SCALE = sSPEC.fVAR2 * COMMENT::SPEC_ADDON_VAR2_SCALE[emSPEC];;

												if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
												{
													strText1.Format(ID2GAMEWORD("EFF_SKILL_ENEMYDURATIONINCRATE",0), sDATA_LVL.fLIFE, fVAR2_SCALE);
												}
												else
												{
													strText1.Format(ID2GAMEWORD("EFF_SKILL_ENEMYDURATIONINC",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
												}
											}
										}
										AddTextNoSplit(strText1, NS_UITEXTCOLOR::GREEN);
										AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
									}
								}
								if (emSPEC == EMSPECA_STIGMA)
								{
									CString strText;
									CString strText1;
									CString strText2;
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												for ( int i = 0; i <SKILL::EMSPEC_STIGMA_TYPE_DWORD_DAMAGE_SKILL; i++ )
												{
													if ( sSPEC.dwFLAG  & ( 1 <<  i   ) )
													{
														strText.AppendFormat( "%s, ", COMMENT::SPEC_STIGMA_TYPE[ i ].c_str());
													}
												}
												//strText.Format(ID2GAMEWORD("EFF_SKILL_STIGMA_RATE",0), sDATA_LVL.fLIFE);
												//strText1.Format(ID2GAMEWORD("EFF_SKILL_STIGMA_RATE",1), fVAR2_SCALE);
												//strText2.Format(ID2GAMEWORD("EFF_SKILL_STIGMA_RATE",2));
												strText2.Format (ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_48"), sDATA_LVL.fLIFE, strText, fVAR2_SCALE);
											}
											else
											{
												for ( int i = 0; i <SKILL::EMSPEC_STIGMA_TYPE_DWORD_DAMAGE_SKILL; i++ )
												{
													if ( sSPEC.dwFLAG  & ( 1 <<  i   ) )
													{
														strText.AppendFormat( "%s, ", COMMENT::SPEC_STIGMA_TYPE[ i ].c_str());
													}
												}
												//strText.Format(ID2GAMEWORD("EFF_SKILL_STIGMA",0), sDATA_LVL.fLIFE);
												//strText1.Format(ID2GAMEWORD("EFF_SKILL_STIGMA",1), fVAR2_SCALE);
												//strText2.Format(ID2GAMEWORD("EFF_SKILL_STIGMA",2));
												strText2.Format (ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_48_RATE"), sDATA_LVL.fLIFE, strText, fVAR2_SCALE);
											}

											AddTextLongestLineSplit (strText2, NS_UITEXTCOLOR::GREEN);

											//AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
											//AddTextNoSplit(strText1, NS_UITEXTCOLOR::GREEN);
											//AddTextNoSplit(strText2, NS_UITEXTCOLOR::GREEN);
										}
									}
								}
								if (emSPEC == EMSPECA_TRANSFORM)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_49"), sDATA_LVL.fLIFE);
									AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_EFFECT_DURATION)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_50"));
									AddTextLongestLineSplit (strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_DAMAGE_LOOP)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_51"));
									AddTextLongestLineSplit (strText, NS_UITEXTCOLOR::GREEN);
								}
								//todo EMSPECA_PET_OFF
								/*if (emSPEC == EMSPECA_PET_OFF)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_52"));
									AddTextLongestLineSplit (strText, NS_UITEXTCOLOR::GREEN);
								}*/
								//immune
								if (emSPEC == EMSPECA_IMMUNE)
								{
									//CString strText;
									//strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_53"), sDATA_LVL.fLIFE);
									//AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
									CString strText;
									CString strText1;
									CString strText2;
									float fVAR1_SCALE = sSPEC.fVAR1;
									float fVAR2_SCALE = sSPEC.fVAR2;

									for ( int i = 0; i < SKILL::EMAPPLY_NSIZE; i++ )
									{
										if ( static_cast<DWORD>(fVAR1_SCALE) & ( 1 <<  i   ) )
										{
											strText1.AppendFormat( "%s, ", COMMENT::SKILL_APPLY[ i ].c_str());
										}
									}
									for ( int i = 0; i < SKILL::EMACTION_NSIZE; i++ )
									{
										if ( static_cast<DWORD>(fVAR2_SCALE) & ( 1 <<  i  ) )
										{
											strText2.AppendFormat( "%s, ", COMMENT::SKILL_ACTION_TYPE[ i ].c_str());
										}
									}
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_53_1"),strText1, strText2, sDATA_LVL.fLIFE);
									AddTextLongestLineSplit ( strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_HOSTILE_ON)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_54"), sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//todo EMSPECA_HOSTILE_OFF
								/*
								if (emSPEC == EMSPECA_HOSTILE_OFF)
								{
									CString strText;
									strText.Format(ID2GAMEWORD("EFF_SKILL_COMBAT_OFF"));
									AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
								}*/
								if (emSPEC == EMSPECA_MANIPULATE)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_57"), sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_RANDOM_EXP_RATE)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_42"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
							}
						}
					}
				}
		if (sAPPLY.emSTATE_BLOW != EMBLOW_NONE)
		{
			const SKILL::SSTATE_BLOW& sSTATE_BLOW = sAPPLY.sSTATE_BLOW[wLevel];

			CString strText;
			CString strText1;

			if (sAPPLY.emSTATE_BLOW == EMBLOW_NUMB)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					float fVAR = float(sSTATE_BLOW.fVAR1 * -100);
					float nVAR = float(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_NUMB_RN", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_NUMB_RN", 1), fVAR, nVAR);
					}
					else
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_NUMB_RN_1", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_NUMB_RN_1", 1), fVAR, nVAR);
					}

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
					AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_STUN)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					int fVAR = int(sSTATE_BLOW.fVAR1);
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_STUN"), sSTATE_BLOW.fRATE);
					else
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_STUN_RN"), sSTATE_BLOW.fRATE);

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_STONE)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					float fVAR = sSTATE_BLOW.fVAR1 * -100;
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_STONE_1", 0), sSTATE_BLOW.fRATE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_STONE_1", 1), fVAR);
					}
					else
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_STONE_2", 0), sSTATE_BLOW.fRATE, fVAR);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_STONE_2", 1), fVAR);
					}

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
					AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_BURN)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					float fVAR = sSTATE_BLOW.fVAR1 * -100;
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_BURN", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_BURN", 1), nVAR);
					}
					else
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_BURN_2", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE, nVAR);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_BURN_2", 1), nVAR);
					}

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
					AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_FROZEN)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					float fVAR = sSTATE_BLOW.fVAR1 * -100;
					float nVAR = float(sSTATE_BLOW.fVAR2 * 100);

					if (nRATE >= 100)
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_FROZEN_RN", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_FROZEN_RN", 1), fVAR, nVAR);
					}
					else
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_FROZEN_RN_1", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_FROZEN_RN_1", 1), fVAR, nVAR);
					}

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
					AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_MAD)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					int fVAR = int(sSTATE_BLOW.fVAR1);
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_MAD_TEST"));
					else
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_MAD_TEST"));

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_POISON)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					int fVAR = int(sSTATE_BLOW.fVAR1);
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_POISON", 0), sSTATE_BLOW.fRATE, nVAR);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_POISON", 1), sDATA_LVL.fLIFE);
					}
					else
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_POISON_2", 0), sSTATE_BLOW.fRATE, nVAR);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_POISON_2", 1), sDATA_LVL.fLIFE);
					}

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
					AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_CURSE)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					int fVAR = int(sSTATE_BLOW.fVAR1);
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_CURSE_TEST"));
					else
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_CURSE_TEST"));

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
				}
			}
		}
		if (bNextLevel)		 return;
		if (!pSkill->GetDesc()) return;
	}

	void LOADADDITIONAL(SNATIVEID sSKILLID, SCHARSKILL* const pCharSkill, BOOL bNextLevel, bool bAdditional)
	{
		if (m_bAdditional == bAdditional) return;

		RESET();

		m_bAdditional = bAdditional;

		CString	strText;
		CString strTemp;

		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData(sSKILLID.wMainID, sSKILLID.wSubID);
		WORD wDispLevel = 0;
		WORD wNextLevel = wDispLevel;
		SKILL::SLEARN& sLEARN = pSkill->m_sLEARN;
		SKILL::SLEARN_LVL& sLVL = sLEARN.sLVL_STEP[wNextLevel];
		if (pCharSkill)
			wDispLevel = pCharSkill->wLevel + 1;
		else
		{
			if (bNextLevel) wDispLevel = 0;
		}

		if (!pSkill)
		{
			return;
		}


		BOOL bMASTER = GLGaeaClient::GetInstance().GetCharacter()->ISMASTER_SKILL(pSkill->m_sBASIC.sNATIVEID);
		{
			CString Filler;
			CString Fill;
			CString strSkillLevel;

			Filler.Format("           ");
			Fill.Format(ID2GAMEWORD("MULTI_SKILL_TEXT", 0));
			strTemp.Format("         %s", pSkill->GetName());
			strSkillLevel.Format(" (%d/%s)", wDispLevel, ID2GAMEWORD("SKILL_LEVEL", 3));
			strTemp += strSkillLevel;
			Filler += Fill;
			AddTextNoSplit("", NS_UITEXTCOLOR::GREEN);
			AddTextNoSplit(Filler, NS_UITEXTCOLOR::DARKLBUE);
			AddTextNoSplit("", NS_UITEXTCOLOR::GREEN);
		}

		//normal or passive
		{
			CString strText;
			switch ( pSkill->m_sBASIC.emACTION )
			{
			case SKILL::EMACTION_NORMAL:
				{
					strText = ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 0 );
				}
				break;

			case SKILL::EMACTION_BUFF:
				{
					strText = ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 1  );
				}
				break;

			case SKILL::EMACTION_DEBUFF:
				{
					strText = ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 2  );
				}
				break;

			case SKILL::EMACTION_LIMIT:
				{
					strText = ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 3  );
				}
				break;
			}

			if ( pSkill->m_sBASIC.emROLE == SKILL::EMROLE_NORMAL )
			{
				switch ( pSkill->m_sBASIC.emAPPLY )
				{
				case SKILL::EMAPPLY_PHY_SHORT:
					{
						strText += "/";
						strText += ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 4 );
					}
					break;

				case SKILL::EMAPPLY_PHY_LONG:
					{
						strText += "/";
						strText += ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 5 );
					}
					break;

				case SKILL::EMAPPLY_MAGIC:
					{
						strText += "/";
						strText += ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 6 );
					}
					break;
				}
			}
			else if ( pSkill->m_sBASIC.emROLE == SKILL::EMROLE_PASSIVE )
			{
				strText += "/";
				strText += ID2GAMEWORD( "SKILL_BASIC_INFO", 8 );
			}
			AddTextNoSplit (strText, NS_UITEXTCOLOR::DEFAULT);
		}
		AddTextNoSplit("", NS_UITEXTCOLOR::GREEN);

		const SKILL::SAPPLY& sAPPLY = pSkill->m_sAPPLY;

		//Skill Range
		{
			WORD wLevel = 0;
			if (pCharSkill)
				wLevel = !bNextLevel ? pCharSkill->wLevel : pCharSkill->wLevel + 1;

			AddTextNoSplit(ID2GAMEWORD("SKILL_BASIC_INFO", 12), NS_UITEXTCOLOR::WHITE);

			const SKILL::CDATA_LVL& sDATA_LVL = sAPPLY.sDATA_LVL[wLevel];
			const SKILL::SSKILLBASIC& sBASIC = pSkill->m_sBASIC;


			CString str;
			CString strText;
			switch ( pSkill->m_sBASIC.emIMPACT_TAR )
			{
			case TAR_SELF: // 자신
				switch ( pSkill->m_sBASIC.emIMPACT_REALM )
				{
				case REALM_SELF: // '목표' 자신
					strText += ID2GAMEINTEXT( "SKILL_AREA_SELF_SELF" );
					break;

				case REALM_ZONE: // '목표' 주위
					switch ( pSkill->m_sBASIC.emIMPACT_TAR )
					{
					case SIDE_OUR: // 자기편에게
						if ( 0 < sDATA_LVL.wTARNUM && 0 < sDATA_LVL.wAPPLYRANGE )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_SELF_ZONE_OUR" ),
							sDATA_LVL.wTARNUM, sDATA_LVL.wAPPLYRANGE);
							strText += str;
						}
						break;

					case SIDE_ENEMY: // 적에게
						if ( 0 < sDATA_LVL.wTARNUM && 0 < sBASIC.wTARRANGE )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_SELF_ZONE_ENEMY" ),
							sDATA_LVL.wTARNUM, sBASIC.wTARRANGE );
							strText += str;
						}
						break;
					case SIDE_ANYBODY: // 모두에게
						if ( 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_SELF_ZONE_ANYBODY" ),
							sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
							strText += str;
						}
						break;
					};
				};
				break;

			case TAR_SPEC: // 타겟
				{
					switch ( pSkill->m_sBASIC.emIMPACT_REALM )
					{
					case REALM_SELF: // '목표' 자신
						switch ( pSkill->m_sBASIC.emIMPACT_TAR )
						{
						case SIDE_OUR: // 자기편에게
							if ( 0 < sBASIC.wTARRANGE )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_SELF_OUR" ),
								sBASIC.wTARRANGE );
								strText += str;
							}
							break;
						case SIDE_ENEMY: // 적에게
							if ( 0 < sBASIC.wTARRANGE )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_SELF_ENEMY" ),
								sBASIC.wTARRANGE );
								strText += str;
							}
							break;
						case SIDE_ANYBODY: // 모두에게
							if ( 0 < sBASIC.wTARRANGE )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_SELF_ANYBODY" ),
								sBASIC.wTARRANGE );
								strText += str;
							}
							break;
						};

					case REALM_ZONE: // '목표' 주위
						switch ( pSkill->m_sBASIC.emIMPACT_TAR )
						{
						case SIDE_OUR: // 자기편에게
							if ( 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_ZONE_OUR" ),
								sBASIC.wTARRANGE, sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
								strText += str;
							}
							break;
						case SIDE_ENEMY: // 적에게
							if ( 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_ZONE_ENEMY" ),
								sBASIC.wTARRANGE, sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
								strText += str;
							}
							break;
						case SIDE_ANYBODY: // 모두에게
							if ( 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_ZONE_ANYBODY" ),
								sBASIC.wTARRANGE, sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM  );
								strText += str;
							}
							break;
						};

					case REALM_FANWIZE:
						switch ( pSkill->m_sBASIC.emIMPACT_TAR )
						{
						case SIDE_ENEMY:
							if ( 0 < sDATA_LVL.wTARNUM && 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wAPPLYANGLE )
							{
								//made an exception for disabling 2 aoe desc
								if ( pSkill->m_sBASIC.sNATIVEID.wMainID == 6 && pSkill->m_sBASIC.sNATIVEID.wSubID == 4 ||
									pSkill->m_sBASIC.sNATIVEID.wMainID == 30 && pSkill->m_sBASIC.sNATIVEID.wSubID == 15 ||
									pSkill->m_sBASIC.sNATIVEID.wMainID == 30 && pSkill->m_sBASIC.sNATIVEID.wSubID == 21 ||
									pSkill->m_sBASIC.sNATIVEID.wMainID == 32 && pSkill->m_sBASIC.sNATIVEID.wSubID == 8 )	break;
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_FANWIZE_ENEMY" ),
								sDATA_LVL.wTARNUM, sBASIC.wTARRANGE % 10, sDATA_LVL.wAPPLYANGLE );
								strText += str;
							}
							break;
						};

					};
				}
				break;

			case TAR_SELF_TOSPEC: // 자신부터 타겟까지
				switch ( pSkill->m_sBASIC.emIMPACT_TAR )
				{
				case SIDE_ENEMY:
					//WORD wPierce = sDATA_LVL.wPIERCENUM + pTarget->GETSUM_PIERCE() + 1;
					WORD wPierce = sDATA_LVL.wPIERCENUM + 1;

					if ( 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wTARNUM && 0 < wPierce )
					{
						str.Format( ID2GAMEINTEXT( "SKILL_AREA_TOSPEC_ENEMY" ),
						sBASIC.wTARRANGE, sDATA_LVL.wTARNUM, wPierce );
						strText += str;
					}
					break;
				};
				break;

			case TAR_ZONE: // 자신
				switch ( pSkill->m_sBASIC.emIMPACT_REALM )
				{
				case REALM_ZONE: // '목표' 주위
					if ( 0 < sDATA_LVL.wTARNUM && 0 < sDATA_LVL.wAPPLYRANGE )
					{
						str.Format( ID2GAMEINTEXT( "SKILL_AREA_ZONE_ENEMY" ),
						sDATA_LVL.wTARNUM, sDATA_LVL.wAPPLYRANGE );
						strText += str;
					}
					break;
				};
				break;

			/*case TAR_ZONE: // 지면 위치의 주변 타겟
				switch ( pSkill->m_sBASIC.emIMPACT_REALM )
				{
				case REALM_ZONE:
					switch ( pSkill->m_sBASIC.emIMPACT_TAR )
					{
					case SIDE_OUR:
						if ( 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_ZONE_OUR" ),
							sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
							strText += str;
						}
						break;
					case SIDE_ENEMY:
						if ( 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_ZONE_ENEMY" ),
							sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
							strText += str;
						}
						break;
					case SIDE_ANYBODY:
						if ( 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_ZONE_ANYBODY" ),
							sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
							strText += str;
						}
						break;
					};
					break;
				};
				break;*/

			case TAR_SPECIFIC: // 지면 위치
				strText += ID2GAMEINTEXT( "SKILL_AREA_LAND" );
				break;
			};

			AddTextNoSplit (strText, NS_UITEXTCOLOR::WHITE);
		}

		//Skill Effect
		WORD wLevel = 0;

		if (pCharSkill)
			wLevel = !bNextLevel ? pCharSkill->wLevel : pCharSkill->wLevel + 1;

		const SKILL::CDATA_LVL& sDATA_LVL = sAPPLY.sDATA_LVL[wLevel];

		bool bEFFECT = false;
		bool bEFFIMPACT = false;

		AddTextNoSplit("", NS_UITEXTCOLOR::GREEN);
		if (sDATA_LVL.fBASIC_VAR != 0 || sAPPLY.emSTATE_BLOW != EMBLOW_NONE || bEFFECT || bEFFIMPACT )
			AddTextNoSplit(ID2GAMEWORD("SKILL_BASIC_INFO", 13), NS_UITEXTCOLOR::GREEN);

					
		//Skill Damage
		{
			SKILL::EMROLE emRole = pSkill->m_sBASIC.emROLE;

			bool bNormalDurationPlus = false;
			bool bNormalDurationMinus = false;
			bool bNormalNonDurationPlus = false;
			bool bNormalNonDurationMinus = false;
			bool bPassiveDurationPlus = false;
			bool bPassiveDurationMinus = false;
			bool bPassiveNonDurationPlus = false;
			bool bPassiveNonDurationMinus = false;

			if ( emRole == SKILL::EMROLE_NORMAL )
			{
				if ( sDATA_LVL.fLIFE )
				{
					if ( sDATA_LVL.fBASIC_VAR > 0.0f )
						bNormalDurationPlus = true;
					else if ( sDATA_LVL.fBASIC_VAR < 0.0f )
						bNormalDurationMinus = true;
				}
				else
				{
					if ( sDATA_LVL.fBASIC_VAR > 0.0f )
						bNormalNonDurationPlus = true;
					else if ( sDATA_LVL.fBASIC_VAR < 0.0f )
						bNormalNonDurationMinus = true;
				}
			}
			else if ( emRole == SKILL::EMROLE_PASSIVE )
			{
				if ( sDATA_LVL.fLIFE )
				{
					if ( sDATA_LVL.fBASIC_VAR > 0.0f )
						bPassiveDurationPlus = true;
					else if ( sDATA_LVL.fBASIC_VAR < 0.0f )
						bPassiveDurationMinus = true;
				}
				else
				{
					if ( sDATA_LVL.fBASIC_VAR > 0.0f )
						bPassiveNonDurationPlus = true;
					else if ( sDATA_LVL.fBASIC_VAR < 0.0f )
						bPassiveNonDurationMinus = true;
				}
			}

			CString strTemp;
			CString strText;

            float fBASIC_VAR_SCALE;

			BOOL bSKILL_PER;
			std::string strSKILL_TYPE;
			
			fBASIC_VAR_SCALE = sDATA_LVL.fBASIC_VAR * COMMENT::SKILL_TYPES_SCALE[sAPPLY.emBASIC_TYPE];
			bSKILL_PER = COMMENT::IsSKILL_PER(sAPPLY.emBASIC_TYPE);

			switch ( sAPPLY.emBASIC_TYPE )
            {
				case SKILL::EMFOR_HP://	??? ??.
					{
						if ( bNormalDurationPlus || bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus || bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
								/* for ep11 soon
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_2")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5)
									, static_cast< float >( nSkillDmg )
									, static_cast< float >( gdDamage.dwNow )
									, static_cast< float >( gdDamage.dwMax ) );
									*/
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_MP://	??? ??.
					{
						if ( bNormalDurationPlus || bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus || bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_SP://	??? ??.
					{
						if ( bNormalDurationPlus || bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus || bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_VARHP://	??? ??.
					{
						if ( bNormalDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus ) 
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[3].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[3].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[3].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[3].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[3].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[3].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[3].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[3].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_VARMP://	??? ??.
					{
						if ( bNormalDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus ) 
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[4].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[4].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[4].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[4].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[4].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[4].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[4].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[4].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_VARSP://	??? ??.
					{
						if ( bNormalDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus ) 
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[5].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[5].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[5].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[5].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[5].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[5].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[5].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[5].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_DEFENSE://	??? ??.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[6].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[6].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[6].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[6].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[6].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[6].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[6].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[6].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_CURE://	??. ( ?? ?? ?? )
					{
						strTemp.Format( "%s", ID2GAMEINTEXT("SKILL_INFOR_CURE") );
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_HITRATE://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[8].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[8].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[8].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[8].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[8].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[8].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[8].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[8].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_AVOIDRATE://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[9].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[9].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[9].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[9].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[9].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[9].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[9].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[9].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_VARAP://	?? (??,??,??) ???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2_1")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2_1")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[10], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[10], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[10], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[10], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[10], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[10], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[10], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[10], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_VARDAMAGE://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[11].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[11].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[11].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[11].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[11].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[11].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[11].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[11].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_VARDEFENSE://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[12].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[12].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[12].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[12].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[12].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[12].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[12].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[12].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_PA://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[13].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[13].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[13].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[13].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[13].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[13].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[13].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[13].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_SA://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[14].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[14].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[14].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[14].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[14].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[14].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[14].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[14].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_MA://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[15].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[15].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[15].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[15].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[15].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[15].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[15].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[15].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_HP_RATE://	HP ???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 5), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_MP_RATE://	HP ???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 6), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_SP_RATE://	HP ???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 7), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_RESIST: //	?? ???
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[30], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[30], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[30], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[30], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[30], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[30], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[30], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[30], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_SUMMONTIME: //	???? ??
					{
						if ( bPassiveNonDurationPlus )
                        {
                            strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE5")
                                , COMMENT::SKILL_TYPES[31], sDATA_LVL.fBASIC_VAR );
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE5")
                                , COMMENT::SKILL_TYPES[31], -sDATA_LVL.fBASIC_VAR );
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
			//end of line here to avoid confusion
			};


		}
		// impact add on
		{
			for(int nImpact = 0; nImpact < SKILL::MAX_IMPACT; nImpact++ )
			{
				const EMIMPACT_ADDON emADDON = sAPPLY.sImpacts[nImpact].emADDON;

				float fADDON_VAR = sAPPLY.sImpacts[nImpact].fADDON_VAR[wLevel];
				float fADDON_VAR_SCALE = fADDON_VAR * COMMENT::IMPACT_ADDON_SCALE[sAPPLY.sImpacts[nImpact].emADDON];
				
				SKILL::EMROLE emRole = pSkill->m_sBASIC.emROLE;

				bool bImpactNormalDurationPlus = false;
				bool bImpactNormalDurationMinus = false;
				bool bImpactNormalNonDurationPlus = false;
				bool bImpactNormalNonDurationMinus = false;
				bool bImpactPassiveDurationPlus = false;
				bool bImpactPassiveDurationMinus = false;
				bool bImpactPassiveNonDurationPlus = false;
				bool bImpactPassiveNonDurationMinus = false;

				BOOL bSKILL_PER;
				std::string strIMPACT_ADDON;
				strIMPACT_ADDON = COMMENT::IMPACT_ADDON[emADDON].c_str();

				if ( emRole == SKILL::EMROLE_NORMAL )
				{
					if ( sDATA_LVL.fLIFE )
					{
						if ( fADDON_VAR > 0.0f )
							bImpactNormalDurationPlus = true;
						else if ( fADDON_VAR < 0.0f )
							bImpactNormalDurationMinus = true;
					}
					else
					{
						if ( fADDON_VAR > 0.0f )
							bImpactNormalNonDurationPlus = true;
						else if ( fADDON_VAR < 0.0f )
							bImpactNormalNonDurationMinus = true;
					}
				}
				else if ( emRole == SKILL::EMROLE_PASSIVE )
				{
					if ( sDATA_LVL.fLIFE )
					{
						if ( fADDON_VAR > 0.0f )
							bImpactPassiveDurationPlus = true;
						else if ( fADDON_VAR < 0.0f )
							bImpactPassiveDurationMinus = true;
					}
					else
					{
						if ( fADDON_VAR > 0.0f )
							bImpactPassiveNonDurationPlus = true;
						else if ( fADDON_VAR < 0.0f )
							bImpactPassiveNonDurationMinus = true;
					}
				}

				BOOL bIsIMPACT_ADDON_PER = FALSE;
				bIsIMPACT_ADDON_PER = COMMENT::IsIMPACT_ADDON_PER(emADDON);

				if ( sAPPLY.sImpacts[nImpact].emADDON != EMIMPACTA_NONE )
				{

					if (0 != fADDON_VAR)
					{
						if (!bEFFIMPACT)AddTextLongestLineSplit("", NS_UITEXTCOLOR::LIGHTSKYBLUE);
						bEFFIMPACT = true;

						switch ( emADDON )
						{
							case EMIMPACTA_HITRATE:     // 명중율 변화량.
							case EMIMPACTA_AVOIDRATE:   // 회피율 변화량.
							case EMIMPACTA_DAMAGE:  // 공격치 변화량.
							case EMIMPACTA_DEFENSE: // 방어치 변화량.
							case EMIMPACTA_VARHP:   // HP 변화율.
							case EMIMPACTA_VARMP:   // MP 변화율.
							case EMIMPACTA_VARSP:   // SP 변화율.
							case EMIMPACTA_VARAP:   // HP,MP,SP 변화율.
							case EMIMPACTA_DAMAGE_RATE:     // 공격치 변화율.
							case EMIMPACTA_DEFENSE_RATE:    // 방어치 변화율.
							case EMIMPACTA_PA:      //	격투치.
							case EMIMPACTA_SA:      //	사격치.
							case EMIMPACTA_MA:      //	방어치.
							case EMIMPACTA_HP_RATE: //	HP 증폭율.
							case EMIMPACTA_MP_RATE: //	MP 증폭율.
							case EMIMPACTA_SP_RATE: //	SP 증폭율.
							case EMIMPACTA_RESIST:  //	저항 수치 변경
							case EMIMPACTA_CHANGESTATS:   //	스탯 수치 변경 ( 힘,민첩,정신 )
							case EMIMPACTA_HP_RECOVERY_VAR:
							case EMIMPACTA_MP_RECOVERY_VAR:
							case EMIMPACTA_SP_RECOVERY_VAR:
							case EMIMPACTA_CP_RECOVERY_VAR:
							case EMIMPACTA_CP_AUTO_VAR:
							{
									CString strTemp;
									CString strText;
									if ( bImpactNormalDurationPlus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), fADDON_VAR );
										}
									}
									else if ( bImpactNormalDurationMinus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), -fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), -fADDON_VAR );
										}
									}
									else if ( bImpactNormalNonDurationPlus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
												, strIMPACT_ADDON.c_str(), fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
												, strIMPACT_ADDON.c_str(), fADDON_VAR );
										}
									}
									else if ( bImpactNormalNonDurationMinus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
												, strIMPACT_ADDON.c_str(), -fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
												, strIMPACT_ADDON.c_str(), -fADDON_VAR );
										}
									}
									else if ( bImpactPassiveDurationPlus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), fADDON_VAR );
										}
									}
									else if ( bImpactPassiveDurationMinus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), -fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), -fADDON_VAR );
										}
									}
									else if ( bImpactPassiveNonDurationPlus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
												, strIMPACT_ADDON.c_str(), fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
												, strIMPACT_ADDON.c_str(), fADDON_VAR );
										}
									}
									else if ( bImpactPassiveNonDurationMinus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
												, strIMPACT_ADDON.c_str(), -fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
												, strIMPACT_ADDON.c_str(), -fADDON_VAR );
										}
									}
									strText += strTemp;
									AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
							}
							break;
						};
					}
				}
			}
		}
				{
					for(int nSpec = 0; nSpec < SKILL::MAX_SPEC; ++nSpec)
					{
						const SKILL::SSPEC& sSPEC = sAPPLY.sSpecs[nSpec].sSPEC[wLevel];
						const EMSPEC_ADDON emSPEC = sAPPLY.sSpecs[nSpec].emSPEC;

						
						bool bAddon = false;

						if (emSPEC != EMSPECA_NULL)
						{
							if (!bAddon)AddTextLongestLineSplit("", NS_UITEXTCOLOR::LIGHTSKYBLUE);
							bAddon = true;
							{
								//teleport,dash,shift
								if (emSPEC == EMSPECA_PUSHPULL)
								{
									CString strText1;
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_1"), fVAR2_SCALE);
											}
											else
											{
												strText1.Format(ID2GAMEWORD("SKILL_SPEC_ADDON_F_1_RATE"), fVAR2_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText1, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_REFDAMAGE)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_2"),sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_REBIRTH)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_3"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//hp,mp,sp gather
								if (emSPEC == EMSPECA_HP_GATHER)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_4_RATE"), fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_4"), fVAR1_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_MP_GATHER)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_5_RATE"), fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_5"), fVAR1_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_SP_GATHER)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_6_RATE"), fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_6"), fVAR1_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_HP_DIV)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_7"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_MP_DIV)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_8"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_SP_DIV)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_9"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_NONBLOW )
								{
									CString strTemp;
									CString strText;
									if (sSPEC.dwFLAG & DIS_NUMB)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_NUMB].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_STUN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_STUN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_STONE)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_STONE].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_BURN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_BURN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_FROZEN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_FROZEN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_MAD)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_MAD].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_POISON)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_POISON].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_CURSE)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_CURSE].c_str());
										strText += strTemp;
									}
									CString strTemper;
									strTemper.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_10"), strText);
									AddTextLongestLineSplit(strTemper, NS_UITEXTCOLOR::GREEN);
								}
								
								if (emSPEC == EMSPECA_RECBLOW )
								{
									CString strTemp;
									CString strText;
									if (sSPEC.dwFLAG & DIS_NUMB)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_NUMB].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_STUN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_STUN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_STONE)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_STONE].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_BURN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_BURN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_FROZEN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_FROZEN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_MAD)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_MAD].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_POISON)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_POISON].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_CURSE)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_CURSE].c_str());
										strText += strTemp;
									}
									CString strTemper;
									strTemper.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_11"), strText);
									AddTextLongestLineSplit(strTemper, NS_UITEXTCOLOR::GREEN);
								}
								//emspeca pierce to do by CNDev
								if (emSPEC == EMSPECA_PIERCE)
								{
									CString strText;
									strText.Format("Fix Pierce Skill Description.");
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_TARRANGE )
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_MOVEVELO)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}

								if (emSPEC == EMSPECA_ONWARD)
								{
									CString TextShift;
									TextShift.Format(ID2GAMEWORD("EFF_SKILL_ONWARD"));
									AddTextNoSplit(TextShift, NS_UITEXTCOLOR::GREEN);

									/*if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * -100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_MYSKILLDELAYRATE",1),fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_MYSKILLDELAY",1),fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * COMMENT::SPEC_ADDON_VAR2_SCALE[emSPEC];;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format(ID2GAMEWORD("EFF_SKILL_MYSKILLDELAYRATE",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
											}
											else
											{
												strText1.Format(ID2GAMEWORD("EFF_SKILL_MYSKILLDELAY",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
											}
										}
									}*/
								}
								if (emSPEC == EMSPECA_INVISIBLE)
								{
									CString strText;
									if ( sDATA_LVL.fLIFE < 0 )
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_16_1"));
									}
									else
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_16"), sDATA_LVL.fLIFE);
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_RECVISIBLE)
								{
									CString strText;
									if ( sDATA_LVL.fLIFE < 0 )
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_17_1"));
									}
									else
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_17"), sDATA_LVL.fLIFE);
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
									
								}
								if (emSPEC == EMSPECA_ATTACKVELO)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * -100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//skill delay
								if (emSPEC == EMSPECA_SKILLDELAY)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * -100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_7"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_6"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_5"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_4"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_7_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_6_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_5_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_4_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_CRUSHING_BLOW)
								{
									CString strText;
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_20_1_RATE",0), fVAR2_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_20_1",0), fVAR2_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_PSY_DAMAGE_REDUCE)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_21_1"), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_21"), sDATA_LVL.fLIFE, fVAR1_SCALE);
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_21_1_RATE"), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_21_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_MAGIC_DAMAGE_REDUCE)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_22_1_RATE"), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_22_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_22_1"), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_22"), sDATA_LVL.fLIFE, fVAR1_SCALE);
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								
								if (emSPEC == EMSPECA_PSY_DAMAGE_REFLECTION)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * 100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format("%2.2f%%", fVAR1_SCALE);
											}
											else
											{
												strText.Format("%2.2f", fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format("%2.2f%%", fVAR2_SCALE);
											}
											else
											{
												strText1.Format("%2.2f", fVAR2_SCALE);
											}
										}
									}
									CString strConnect;
									strConnect.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_23"),sDATA_LVL.fLIFE, strText1, strText);
									AddTextLongestLineSplit(strConnect, NS_UITEXTCOLOR::GREEN);
								}

								if (emSPEC == EMSPECA_MAGIC_DAMAGE_REFLECTION)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * 100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format("%2.2f%%", fVAR1_SCALE);
											}
											else
											{
												strText.Format("%2.2f", fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format("%2.2f%%", fVAR2_SCALE);
											}
											else
											{
												strText1.Format("%2.2f", fVAR2_SCALE);
											}
										}
									}
									CString strConnect;
									strConnect.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_24"),sDATA_LVL.fLIFE, strText1, strText);
									AddTextLongestLineSplit(strConnect, NS_UITEXTCOLOR::GREEN);
								}

								//debuffs / dedebuffs
								if (emSPEC == EMSPECA_BUFF_REMOVE)
								{
									CString strText;
									if (pSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF)
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_25_2"));
									}
									else
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_25_1"));
									}
									AddTextLongestLineSplit(strText,NS_UITEXTCOLOR::GREEN);
								}
								//stun
								if (emSPEC == EMSPECA_STUN)
								{
									CString strText;
									if (pSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF)
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_26_2"), sDATA_LVL.fLIFE);
									}
									else
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_26"), sDATA_LVL.fLIFE);
									}
									AddTextLongestLineSplit(strText,NS_UITEXTCOLOR::GREEN);
								}
								//todo cndev defense skill active
								//pull,push,shock
								if (emSPEC == EMSPECA_PULL)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * 100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format("%2.2f%%", fVAR1_SCALE);
											}
											else
											{
												strText.Format("%2.2f", fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format("%2.2f%%", fVAR2_SCALE);
											}
											else
											{
												strText1.Format("%2.2f", fVAR2_SCALE);
											}
										}
									}
									CString strConnect;
									strConnect.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_28"), strText, strText1);
									AddTextLongestLineSplit(strConnect, NS_UITEXTCOLOR::GREEN);
								}
								//pull,push,shock
								if (emSPEC == EMSPECA_PUSH)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * 100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format("%2.2f%%", fVAR1_SCALE);
											}
											else
											{
												strText.Format("%2.2f", fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format("%2.2f%%", fVAR2_SCALE);
											}
											else
											{
												strText1.Format("%2.2f", fVAR2_SCALE);
											}
										}
									}
									CString strConnect;
									strConnect.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_29"), strText, strText1);
									AddTextLongestLineSplit(strConnect, NS_UITEXTCOLOR::GREEN);
								}
								//continous damage
								if (emSPEC == EMSPECA_CONTINUOUS_DAMAGE)
								{
									CString strText;
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_30_RATE",0), sDATA_LVL.fLIFE, fVAR2_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_30",0), sDATA_LVL.fLIFE, fVAR2_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//i left it here
								//curse
								if (emSPEC == EMSPECA_CURSE )
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_31_RATE",0), sDATA_LVL.fLIFE, fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_31",0), sDATA_LVL.fLIFE, fVAR1_SCALE);
											}

											AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
										}
									}
								}
								//range
								if (emSPEC == EMSPECA_CHANGE_ATTACK_RANGE )
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}

								if (emSPEC == EMSPECA_CHANGE_APPLY_RANGE)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_PROHIBIT_POTION)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_34"), sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_PROHIBIT_SKILL)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_35"), sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//emspeca_ignore_damage here
								//start tw buffs no need description
								//EMSPECA_ITEMDROP_RATE
								//EMSPECA_MONEYDROP_RATE
								//end tw buffs no need description
								//emspeca_getexp_rate here -- need rework
								if (emSPEC == EMSPECA_GETEXP_RATE)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_EXPRATE",0), sDATA_LVL.fLIFE,fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_EXP",0), sDATA_LVL.fLIFE,fVAR1_SCALE);
											}

											AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
										}
									}
								}
								//start tw buffs no need description
								//EMSPECA_TALK_TO_NPC
								//EMSPECA_SPECIAL_ITEM_DROP
								//end tw buffs no need description
								if (emSPEC == EMSPECA_TELEPORT)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_42"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_POSITION_SHIFT)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_43_4"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//emspeca_enhancement reinforcement rework
								if (emSPEC == EMSPECA_ENHANCEMENT)
								{
									CString strText;
									CString strText1;
									//if (0 != sSPEC.fVAR1)
									//{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_REINFORCEMENTRATE",0), sDATA_LVL.fLIFE, fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_REINFORCEMENT",0), sDATA_LVL.fLIFE, fVAR1_SCALE);
											}
										}
									//}
									//if (0 != sSPEC.fVAR2)
									//{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format(ID2GAMEWORD("EFF_SKILL_REINFORCEMENTRATE",1), fVAR2_SCALE);
											}
											else
											{
												strText1.Format(ID2GAMEWORD("EFF_SKILL_REINFORCEMENT",1), fVAR2_SCALE);
											}
										}
									//}
									AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
									AddTextNoSplit(strText1, NS_UITEXTCOLOR::GREEN);
								}
								//emspeca_illusion to do
								if (emSPEC == EMSPECA_VEHICLE_OFF)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_46_RATE"), fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_46"), fVAR1_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//duration inc
								if (emSPEC == EMSPECA_DURATION_CHANGE)
								{
									CString strText;
									CString strText1;

									if (pSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF)
									{

										if (0 != sSPEC.fVAR1)
										{
											if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
											{
												float fVAR1_SCALE = sSPEC.fVAR1 * 100;

												if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
												{
													strText.Format(ID2GAMEWORD("EFF_SKILL_MYDURATIONINCRATE",1),fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEWORD("EFF_SKILL_MYDURATIONINC",1),fVAR1_SCALE);
												}
											}
										}
										if (0 != sSPEC.fVAR2)
										{
											if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
											{
												float fVAR2_SCALE = sSPEC.fVAR2 * COMMENT::SPEC_ADDON_VAR2_SCALE[emSPEC];;

												if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
												{
													strText1.Format(ID2GAMEWORD("EFF_SKILL_MYDURATIONINCRATE",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
												}
												else
												{
													strText1.Format(ID2GAMEWORD("EFF_SKILL_MYDURATIONINC",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
												}
											}
										}
										AddTextNoSplit(strText1, NS_UITEXTCOLOR::GREEN);
										AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
									}
									else
									{
										if (0 != sSPEC.fVAR1)
										{
											if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
											{
												float fVAR1_SCALE = sSPEC.fVAR1 * 100;

												if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
												{
													strText.Format(ID2GAMEWORD("EFF_SKILL_ENEMYDURATIONINCRATE",1), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEWORD("EFF_SKILL_ENEMYDURATIONINC",1),fVAR1_SCALE);
												}
											}
										}
										if (0 != sSPEC.fVAR2)
										{
											if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
											{
												float fVAR2_SCALE = sSPEC.fVAR2 * COMMENT::SPEC_ADDON_VAR2_SCALE[emSPEC];;

												if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
												{
													strText1.Format(ID2GAMEWORD("EFF_SKILL_ENEMYDURATIONINCRATE",0), sDATA_LVL.fLIFE, fVAR2_SCALE);
												}
												else
												{
													strText1.Format(ID2GAMEWORD("EFF_SKILL_ENEMYDURATIONINC",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
												}
											}
										}
										AddTextNoSplit(strText1, NS_UITEXTCOLOR::GREEN);
										AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
									}
								}
								if (emSPEC == EMSPECA_STIGMA)
								{
									CString strText;
									CString strText1;
									CString strText2;
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												for ( int i = 0; i <SKILL::EMSPEC_STIGMA_TYPE_DWORD_DAMAGE_SKILL; i++ )
												{
													if ( sSPEC.dwFLAG  & ( 1 <<  i   ) )
													{
														strText.AppendFormat( "%s, ", COMMENT::SPEC_STIGMA_TYPE[ i ].c_str());
													}
												}
												//strText.Format(ID2GAMEWORD("EFF_SKILL_STIGMA_RATE",0), sDATA_LVL.fLIFE);
												//strText1.Format(ID2GAMEWORD("EFF_SKILL_STIGMA_RATE",1), fVAR2_SCALE);
												//strText2.Format(ID2GAMEWORD("EFF_SKILL_STIGMA_RATE",2));
												strText2.Format (ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_48"), sDATA_LVL.fLIFE, strText, fVAR2_SCALE);
											}
											else
											{
												for ( int i = 0; i <SKILL::EMSPEC_STIGMA_TYPE_DWORD_DAMAGE_SKILL; i++ )
												{
													if ( sSPEC.dwFLAG  & ( 1 <<  i   ) )
													{
														strText.AppendFormat( "%s, ", COMMENT::SPEC_STIGMA_TYPE[ i ].c_str());
													}
												}
												//strText.Format(ID2GAMEWORD("EFF_SKILL_STIGMA",0), sDATA_LVL.fLIFE);
												//strText1.Format(ID2GAMEWORD("EFF_SKILL_STIGMA",1), fVAR2_SCALE);
												//strText2.Format(ID2GAMEWORD("EFF_SKILL_STIGMA",2));
												strText2.Format (ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_48_RATE"), sDATA_LVL.fLIFE, strText, fVAR2_SCALE);
											}

											AddTextLongestLineSplit (strText2, NS_UITEXTCOLOR::GREEN);

											//AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
											//AddTextNoSplit(strText1, NS_UITEXTCOLOR::GREEN);
											//AddTextNoSplit(strText2, NS_UITEXTCOLOR::GREEN);
										}
									}
								}
								if (emSPEC == EMSPECA_TRANSFORM)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_49"), sDATA_LVL.fLIFE);
									AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_EFFECT_DURATION)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_50"));
									AddTextLongestLineSplit (strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_DAMAGE_LOOP)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_51"));
									AddTextLongestLineSplit (strText, NS_UITEXTCOLOR::GREEN);
								}
								//todo EMSPECA_PET_OFF
								/*if (emSPEC == EMSPECA_PET_OFF)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_52"));
									AddTextLongestLineSplit (strText, NS_UITEXTCOLOR::GREEN);
								}*/
								//immune
								if (emSPEC == EMSPECA_IMMUNE)
								{
									//CString strText;
									//strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_53"), sDATA_LVL.fLIFE);
									//AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
									CString strText;
									CString strText1;
									CString strText2;
									float fVAR1_SCALE = sSPEC.fVAR1;
									float fVAR2_SCALE = sSPEC.fVAR2;

									for ( int i = 0; i < SKILL::EMAPPLY_NSIZE; i++ )
									{
										if ( static_cast<DWORD>(fVAR1_SCALE) & ( 1 <<  i   ) )
										{
											strText1.AppendFormat( "%s, ", COMMENT::SKILL_APPLY[ i ].c_str());
										}
									}
									for ( int i = 0; i < SKILL::EMACTION_NSIZE; i++ )
									{
										if ( static_cast<DWORD>(fVAR2_SCALE) & ( 1 <<  i  ) )
										{
											strText2.AppendFormat( "%s, ", COMMENT::SKILL_ACTION_TYPE[ i ].c_str());
										}
									}
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_53_1"),strText1, strText2, sDATA_LVL.fLIFE);
									AddTextLongestLineSplit ( strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_HOSTILE_ON)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_54"), sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//todo EMSPECA_HOSTILE_OFF
								/*
								if (emSPEC == EMSPECA_HOSTILE_OFF)
								{
									CString strText;
									strText.Format(ID2GAMEWORD("EFF_SKILL_COMBAT_OFF"));
									AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
								}*/
								if (emSPEC == EMSPECA_MANIPULATE)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_57"), sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_RANDOM_EXP_RATE)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_42"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
							}
						}
					}
				}
		if (sAPPLY.emSTATE_BLOW != EMBLOW_NONE)
		{
			const SKILL::SSTATE_BLOW& sSTATE_BLOW = sAPPLY.sSTATE_BLOW[wLevel];

			CString strText;
			CString strText1;

			if (sAPPLY.emSTATE_BLOW == EMBLOW_NUMB)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					float fVAR = float(sSTATE_BLOW.fVAR1 * -100);
					float nVAR = float(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_NUMB_RN", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_NUMB_RN", 1), fVAR, nVAR);
					}
					else
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_NUMB_RN_1", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_NUMB_RN_1", 1), fVAR, nVAR);
					}

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
					AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_STUN)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					int fVAR = int(sSTATE_BLOW.fVAR1);
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_STUN"), sSTATE_BLOW.fRATE);
					else
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_STUN_RN"), sSTATE_BLOW.fRATE);

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_STONE)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					float fVAR = sSTATE_BLOW.fVAR1 * -100;
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_STONE_1", 0), sSTATE_BLOW.fRATE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_STONE_1", 1), fVAR);
					}
					else
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_STONE_2", 0), sSTATE_BLOW.fRATE, fVAR);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_STONE_2", 1), fVAR);
					}

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
					AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_BURN)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					float fVAR = sSTATE_BLOW.fVAR1 * -100;
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_BURN", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_BURN", 1), nVAR);
					}
					else
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_BURN_2", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE, nVAR);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_BURN_2", 1), nVAR);
					}

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
					AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_FROZEN)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					float fVAR = sSTATE_BLOW.fVAR1 * -100;
					float nVAR = float(sSTATE_BLOW.fVAR2 * 100);

					if (nRATE >= 100)
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_FROZEN_RN", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_FROZEN_RN", 1), fVAR, nVAR);
					}
					else
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_FROZEN_RN_1", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_FROZEN_RN_1", 1), fVAR, nVAR);
					}

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
					AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_MAD)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					int fVAR = int(sSTATE_BLOW.fVAR1);
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_MAD_TEST"));
					else
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_MAD_TEST"));

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_POISON)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					int fVAR = int(sSTATE_BLOW.fVAR1);
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_POISON", 0), sSTATE_BLOW.fRATE, nVAR);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_POISON", 1), sDATA_LVL.fLIFE);
					}
					else
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_POISON_2", 0), sSTATE_BLOW.fRATE, nVAR);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_POISON_2", 1), sDATA_LVL.fLIFE);
					}

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
					AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_CURSE)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					int fVAR = int(sSTATE_BLOW.fVAR1);
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_CURSE_TEST"));
					else
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_CURSE_TEST"));

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
				}
			}
		}
		if (bNextLevel)		 return;
		if (!pSkill->GetDesc()) return;
	}

	void LOADWEAPONSKILL(SNATIVEID sSKILLID, GLSKILL* const pSkill, bool bWeaponSkill)
	{
		if (m_bWeaponSkill == bWeaponSkill) return;

		RESET();

		m_bWeaponSkill = bWeaponSkill;

		CString	strText;
		CString strTemp;

		//PGLSKILL pSkill = GLSkillMan::GetInstance().GetData(sSKILLID.wMainID, sSKILLID.wSubID);
		WORD wDispLevel = 0;
		WORD wNextLevel = wDispLevel;
		SKILL::SLEARN& sLEARN = pSkill->m_sLEARN;
		SKILL::SLEARN_LVL& sLVL = sLEARN.sLVL_STEP[wNextLevel];
		/*if (pCharSkill)
			wDispLevel = 0;
		else
		{
			wDispLevel = 0;
		}*/

		if (!pSkill)
		{
			return;
		}


		BOOL bMASTER = GLGaeaClient::GetInstance().GetCharacter()->ISMASTER_SKILL(pSkill->m_sBASIC.sNATIVEID);
		{
			CString Filler;
			CString Fill;
			CString strSkillLevel;

			Filler.Format("           ");
			Fill.Format(ID2GAMEWORD("MULTI_SKILL_TEXT", 0));
			strTemp.Format("         %s", pSkill->GetName());
			strSkillLevel.Format(" ");
			strTemp += strSkillLevel;
			Filler += Fill;
			AddTextNoSplit("", NS_UITEXTCOLOR::GREEN);
			AddTextNoSplit(Filler, NS_UITEXTCOLOR::DARKLBUE);
			AddTextNoSplit("", NS_UITEXTCOLOR::GREEN);
		}

		//normal or passive
		{
			CString strText;
			switch ( pSkill->m_sBASIC.emACTION )
			{
			case SKILL::EMACTION_NORMAL:
				{
					strText = ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 0 );
				}
				break;

			case SKILL::EMACTION_BUFF:
				{
					strText = ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 1  );
				}
				break;

			case SKILL::EMACTION_DEBUFF:
				{
					strText = ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 2  );
				}
				break;

			case SKILL::EMACTION_LIMIT:
				{
					strText = ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 3  );
				}
				break;
			}

			if ( pSkill->m_sBASIC.emROLE == SKILL::EMROLE_NORMAL )
			{
				switch ( pSkill->m_sBASIC.emAPPLY )
				{
				case SKILL::EMAPPLY_PHY_SHORT:
					{
						strText += "/";
						strText += ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 4 );
					}
					break;

				case SKILL::EMAPPLY_PHY_LONG:
					{
						strText += "/";
						strText += ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 5 );
					}
					break;

				case SKILL::EMAPPLY_MAGIC:
					{
						strText += "/";
						strText += ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 6 );
					}
					break;
				}
			}
			else if ( pSkill->m_sBASIC.emROLE == SKILL::EMROLE_PASSIVE )
			{
				strText += "/";
				strText += ID2GAMEWORD( "SKILL_BASIC_INFO", 8 );
			}
			AddTextNoSplit (strText, NS_UITEXTCOLOR::DEFAULT);
		}
		AddTextNoSplit("", NS_UITEXTCOLOR::GREEN);

		const SKILL::SAPPLY& sAPPLY = pSkill->m_sAPPLY;

		//Skill Range
		{
			WORD wLevel = 0;
			//if (pCharSkill)
			//	wLevel = 0;

			AddTextNoSplit(ID2GAMEWORD("SKILL_BASIC_INFO", 12), NS_UITEXTCOLOR::WHITE);

			const SKILL::CDATA_LVL& sDATA_LVL = sAPPLY.sDATA_LVL[wLevel];
			const SKILL::SSKILLBASIC& sBASIC = pSkill->m_sBASIC;


			CString str;
			CString strText;
			switch ( pSkill->m_sBASIC.emIMPACT_TAR )
			{
			case TAR_SELF: // 자신
				switch ( pSkill->m_sBASIC.emIMPACT_REALM )
				{
				case REALM_SELF: // '목표' 자신
					strText += ID2GAMEINTEXT( "SKILL_AREA_SELF_SELF" );
					break;

				case REALM_ZONE: // '목표' 주위
					switch ( pSkill->m_sBASIC.emIMPACT_TAR )
					{
					case SIDE_OUR: // 자기편에게
						if ( 0 < sDATA_LVL.wTARNUM && 0 < sDATA_LVL.wAPPLYRANGE )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_SELF_ZONE_OUR" ),
							sDATA_LVL.wTARNUM, sDATA_LVL.wAPPLYRANGE);
							strText += str;
						}
						break;

					case SIDE_ENEMY: // 적에게
						if ( 0 < sDATA_LVL.wTARNUM && 0 < sBASIC.wTARRANGE )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_SELF_ZONE_ENEMY" ),
							sDATA_LVL.wTARNUM, sBASIC.wTARRANGE );
							strText += str;
						}
						break;
					case SIDE_ANYBODY: // 모두에게
						if ( 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_SELF_ZONE_ANYBODY" ),
							sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
							strText += str;
						}
						break;
					};
				};
				break;

			case TAR_SPEC: // 타겟
				{
					switch ( pSkill->m_sBASIC.emIMPACT_REALM )
					{
					case REALM_SELF: // '목표' 자신
						switch ( pSkill->m_sBASIC.emIMPACT_TAR )
						{
						case SIDE_OUR: // 자기편에게
							if ( 0 < sBASIC.wTARRANGE )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_SELF_OUR" ),
								sBASIC.wTARRANGE );
								strText += str;
							}
							break;
						case SIDE_ENEMY: // 적에게
							if ( 0 < sBASIC.wTARRANGE )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_SELF_ENEMY" ),
								sBASIC.wTARRANGE );
								strText += str;
							}
							break;
						case SIDE_ANYBODY: // 모두에게
							if ( 0 < sBASIC.wTARRANGE )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_SELF_ANYBODY" ),
								sBASIC.wTARRANGE );
								strText += str;
							}
							break;
						};

					case REALM_ZONE: // '목표' 주위
						switch ( pSkill->m_sBASIC.emIMPACT_TAR )
						{
						case SIDE_OUR: // 자기편에게
							if ( 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_ZONE_OUR" ),
								sBASIC.wTARRANGE, sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
								strText += str;
							}
							break;
						case SIDE_ENEMY: // 적에게
							if ( 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_ZONE_ENEMY" ),
								sBASIC.wTARRANGE, sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
								strText += str;
							}
							break;
						case SIDE_ANYBODY: // 모두에게
							if ( 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_ZONE_ANYBODY" ),
								sBASIC.wTARRANGE, sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM  );
								strText += str;
							}
							break;
						};

					case REALM_FANWIZE:
						switch ( pSkill->m_sBASIC.emIMPACT_TAR )
						{
						case SIDE_ENEMY:
							if ( 0 < sDATA_LVL.wTARNUM && 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wAPPLYANGLE )
							{
								//made an exception for disabling 2 aoe desc
								if ( pSkill->m_sBASIC.sNATIVEID.wMainID == 6 && pSkill->m_sBASIC.sNATIVEID.wSubID == 4 ||
									pSkill->m_sBASIC.sNATIVEID.wMainID == 30 && pSkill->m_sBASIC.sNATIVEID.wSubID == 15 ||
									pSkill->m_sBASIC.sNATIVEID.wMainID == 30 && pSkill->m_sBASIC.sNATIVEID.wSubID == 21 ||
									pSkill->m_sBASIC.sNATIVEID.wMainID == 32 && pSkill->m_sBASIC.sNATIVEID.wSubID == 8 )	break;
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_FANWIZE_ENEMY" ),
								sDATA_LVL.wTARNUM, sBASIC.wTARRANGE % 10, sDATA_LVL.wAPPLYANGLE );
								strText += str;
							}
							break;
						};

					};
				}
				break;

			case TAR_SELF_TOSPEC: // 자신부터 타겟까지
				switch ( pSkill->m_sBASIC.emIMPACT_TAR )
				{
				case SIDE_ENEMY:
					//WORD wPierce = sDATA_LVL.wPIERCENUM + pTarget->GETSUM_PIERCE() + 1;
					WORD wPierce = sDATA_LVL.wPIERCENUM + 1;

					if ( 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wTARNUM && 0 < wPierce )
					{
						str.Format( ID2GAMEINTEXT( "SKILL_AREA_TOSPEC_ENEMY" ),
						sBASIC.wTARRANGE, sDATA_LVL.wTARNUM, wPierce );
						strText += str;
					}
					break;
				};
				break;

			case TAR_ZONE: // 자신
				switch ( pSkill->m_sBASIC.emIMPACT_REALM )
				{
				case REALM_ZONE: // '목표' 주위
					if ( 0 < sDATA_LVL.wTARNUM && 0 < sDATA_LVL.wAPPLYRANGE )
					{
						str.Format( ID2GAMEINTEXT( "SKILL_AREA_ZONE_ENEMY" ),
						sDATA_LVL.wTARNUM, sDATA_LVL.wAPPLYRANGE );
						strText += str;
					}
					break;
				};
				break;

			/*case TAR_ZONE: // 지면 위치의 주변 타겟
				switch ( pSkill->m_sBASIC.emIMPACT_REALM )
				{
				case REALM_ZONE:
					switch ( pSkill->m_sBASIC.emIMPACT_TAR )
					{
					case SIDE_OUR:
						if ( 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_ZONE_OUR" ),
							sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
							strText += str;
						}
						break;
					case SIDE_ENEMY:
						if ( 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_ZONE_ENEMY" ),
							sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
							strText += str;
						}
						break;
					case SIDE_ANYBODY:
						if ( 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_ZONE_ANYBODY" ),
							sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
							strText += str;
						}
						break;
					};
					break;
				};
				break;*/

			case TAR_SPECIFIC: // 지면 위치
				strText += ID2GAMEINTEXT( "SKILL_AREA_LAND" );
				break;
			};

			AddTextNoSplit (strText, NS_UITEXTCOLOR::WHITE);
		}

		//Skill Effect
		WORD wLevel = 0;

		//if (pCharSkill)
		//	wLevel = 0;

		const SKILL::CDATA_LVL& sDATA_LVL = sAPPLY.sDATA_LVL[wLevel];

		bool bEFFECT = false;
		bool bEFFIMPACT = false;

		AddTextNoSplit("", NS_UITEXTCOLOR::GREEN);
		if (sDATA_LVL.fBASIC_VAR != 0 || sAPPLY.emSTATE_BLOW != EMBLOW_NONE || bEFFECT || bEFFIMPACT )
			AddTextNoSplit(ID2GAMEWORD("SKILL_BASIC_INFO", 13), NS_UITEXTCOLOR::GREEN);

					
		//Skill Damage
		{
			SKILL::EMROLE emRole = pSkill->m_sBASIC.emROLE;

			bool bNormalDurationPlus = false;
			bool bNormalDurationMinus = false;
			bool bNormalNonDurationPlus = false;
			bool bNormalNonDurationMinus = false;
			bool bPassiveDurationPlus = false;
			bool bPassiveDurationMinus = false;
			bool bPassiveNonDurationPlus = false;
			bool bPassiveNonDurationMinus = false;

			if ( emRole == SKILL::EMROLE_NORMAL )
			{
				if ( sDATA_LVL.fLIFE )
				{
					if ( sDATA_LVL.fBASIC_VAR > 0.0f )
						bNormalDurationPlus = true;
					else if ( sDATA_LVL.fBASIC_VAR < 0.0f )
						bNormalDurationMinus = true;
				}
				else
				{
					if ( sDATA_LVL.fBASIC_VAR > 0.0f )
						bNormalNonDurationPlus = true;
					else if ( sDATA_LVL.fBASIC_VAR < 0.0f )
						bNormalNonDurationMinus = true;
				}
			}
			else if ( emRole == SKILL::EMROLE_PASSIVE )
			{
				if ( sDATA_LVL.fLIFE )
				{
					if ( sDATA_LVL.fBASIC_VAR > 0.0f )
						bPassiveDurationPlus = true;
					else if ( sDATA_LVL.fBASIC_VAR < 0.0f )
						bPassiveDurationMinus = true;
				}
				else
				{
					if ( sDATA_LVL.fBASIC_VAR > 0.0f )
						bPassiveNonDurationPlus = true;
					else if ( sDATA_LVL.fBASIC_VAR < 0.0f )
						bPassiveNonDurationMinus = true;
				}
			}

			CString strTemp;
			CString strText;

            float fBASIC_VAR_SCALE;

			BOOL bSKILL_PER;
			std::string strSKILL_TYPE;
			
			fBASIC_VAR_SCALE = sDATA_LVL.fBASIC_VAR * COMMENT::SKILL_TYPES_SCALE[sAPPLY.emBASIC_TYPE];
			bSKILL_PER = COMMENT::IsSKILL_PER(sAPPLY.emBASIC_TYPE);

			switch ( sAPPLY.emBASIC_TYPE )
            {
				case SKILL::EMFOR_HP://	??? ??.
					{
						if ( bNormalDurationPlus || bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus || bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
								/* for ep11 soon
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_2")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5)
									, static_cast< float >( nSkillDmg )
									, static_cast< float >( gdDamage.dwNow )
									, static_cast< float >( gdDamage.dwMax ) );
									*/
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_MP://	??? ??.
					{
						if ( bNormalDurationPlus || bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus || bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_SP://	??? ??.
					{
						if ( bNormalDurationPlus || bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus || bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_VARHP://	??? ??.
					{
						if ( bNormalDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus ) 
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[3].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[3].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[3].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[3].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[3].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[3].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[3].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[3].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_VARMP://	??? ??.
					{
						if ( bNormalDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus ) 
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[4].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[4].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[4].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[4].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[4].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[4].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[4].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[4].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_VARSP://	??? ??.
					{
						if ( bNormalDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus ) 
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[5].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[5].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[5].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[5].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[5].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[5].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[5].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[5].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_DEFENSE://	??? ??.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[6].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[6].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[6].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[6].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[6].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[6].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[6].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[6].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_CURE://	??. ( ?? ?? ?? )
					{
						strTemp.Format( "%s", ID2GAMEINTEXT("SKILL_INFOR_CURE") );
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_HITRATE://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[8].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[8].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[8].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[8].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[8].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[8].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[8].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[8].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_AVOIDRATE://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[9].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[9].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[9].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[9].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[9].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[9].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[9].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[9].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_VARAP://	?? (??,??,??) ???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2_1")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2_1")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[10], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[10], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[10], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[10], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[10], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[10], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[10], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[10], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_VARDAMAGE://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[11].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[11].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[11].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[11].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[11].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[11].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[11].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[11].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_VARDEFENSE://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[12].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[12].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[12].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[12].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[12].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[12].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[12].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[12].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_PA://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[13].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[13].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[13].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[13].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[13].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[13].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[13].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[13].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_SA://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[14].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[14].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[14].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[14].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[14].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[14].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[14].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[14].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_MA://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[15].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[15].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[15].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[15].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[15].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[15].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[15].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[15].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_HP_RATE://	HP ???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 5), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_MP_RATE://	HP ???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 6), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_SP_RATE://	HP ???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 7), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_RESIST: //	?? ???
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[30], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[30], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[30], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[30], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[30], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[30], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[30], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[30], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_SUMMONTIME: //	???? ??
					{
						if ( bPassiveNonDurationPlus )
                        {
                            strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE5")
                                , COMMENT::SKILL_TYPES[31], sDATA_LVL.fBASIC_VAR );
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE5")
                                , COMMENT::SKILL_TYPES[31], -sDATA_LVL.fBASIC_VAR );
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
			//end of line here to avoid confusion
			};


		}
		// impact add on
		{
			for(int nImpact = 0; nImpact < SKILL::MAX_IMPACT; nImpact++ )
			{
				const EMIMPACT_ADDON emADDON = sAPPLY.sImpacts[nImpact].emADDON;

				float fADDON_VAR = sAPPLY.sImpacts[nImpact].fADDON_VAR[wLevel];
				float fADDON_VAR_SCALE = fADDON_VAR * COMMENT::IMPACT_ADDON_SCALE[sAPPLY.sImpacts[nImpact].emADDON];
				
				SKILL::EMROLE emRole = pSkill->m_sBASIC.emROLE;

				bool bImpactNormalDurationPlus = false;
				bool bImpactNormalDurationMinus = false;
				bool bImpactNormalNonDurationPlus = false;
				bool bImpactNormalNonDurationMinus = false;
				bool bImpactPassiveDurationPlus = false;
				bool bImpactPassiveDurationMinus = false;
				bool bImpactPassiveNonDurationPlus = false;
				bool bImpactPassiveNonDurationMinus = false;

				BOOL bSKILL_PER;
				std::string strIMPACT_ADDON;
				strIMPACT_ADDON = COMMENT::IMPACT_ADDON[emADDON].c_str();

				if ( emRole == SKILL::EMROLE_NORMAL )
				{
					if ( sDATA_LVL.fLIFE )
					{
						if ( fADDON_VAR > 0.0f )
							bImpactNormalDurationPlus = true;
						else if ( fADDON_VAR < 0.0f )
							bImpactNormalDurationMinus = true;
					}
					else
					{
						if ( fADDON_VAR > 0.0f )
							bImpactNormalNonDurationPlus = true;
						else if ( fADDON_VAR < 0.0f )
							bImpactNormalNonDurationMinus = true;
					}
				}
				else if ( emRole == SKILL::EMROLE_PASSIVE )
				{
					if ( sDATA_LVL.fLIFE )
					{
						if ( fADDON_VAR > 0.0f )
							bImpactPassiveDurationPlus = true;
						else if ( fADDON_VAR < 0.0f )
							bImpactPassiveDurationMinus = true;
					}
					else
					{
						if ( fADDON_VAR > 0.0f )
							bImpactPassiveNonDurationPlus = true;
						else if ( fADDON_VAR < 0.0f )
							bImpactPassiveNonDurationMinus = true;
					}
				}

				BOOL bIsIMPACT_ADDON_PER = FALSE;
				bIsIMPACT_ADDON_PER = COMMENT::IsIMPACT_ADDON_PER(emADDON);

				if ( sAPPLY.sImpacts[nImpact].emADDON != EMIMPACTA_NONE )
				{

					if (0 != fADDON_VAR)
					{
						if (!bEFFIMPACT)AddTextLongestLineSplit("", NS_UITEXTCOLOR::LIGHTSKYBLUE);
						bEFFIMPACT = true;

						switch ( emADDON )
						{
							case EMIMPACTA_HITRATE:     // 명중율 변화량.
							case EMIMPACTA_AVOIDRATE:   // 회피율 변화량.
							case EMIMPACTA_DAMAGE:  // 공격치 변화량.
							case EMIMPACTA_DEFENSE: // 방어치 변화량.
							case EMIMPACTA_VARHP:   // HP 변화율.
							case EMIMPACTA_VARMP:   // MP 변화율.
							case EMIMPACTA_VARSP:   // SP 변화율.
							case EMIMPACTA_VARAP:   // HP,MP,SP 변화율.
							case EMIMPACTA_DAMAGE_RATE:     // 공격치 변화율.
							case EMIMPACTA_DEFENSE_RATE:    // 방어치 변화율.
							case EMIMPACTA_PA:      //	격투치.
							case EMIMPACTA_SA:      //	사격치.
							case EMIMPACTA_MA:      //	방어치.
							case EMIMPACTA_HP_RATE: //	HP 증폭율.
							case EMIMPACTA_MP_RATE: //	MP 증폭율.
							case EMIMPACTA_SP_RATE: //	SP 증폭율.
							case EMIMPACTA_RESIST:  //	저항 수치 변경
							case EMIMPACTA_CHANGESTATS:   //	스탯 수치 변경 ( 힘,민첩,정신 )
							case EMIMPACTA_HP_RECOVERY_VAR:
							case EMIMPACTA_MP_RECOVERY_VAR:
							case EMIMPACTA_SP_RECOVERY_VAR:
							case EMIMPACTA_CP_RECOVERY_VAR:
							case EMIMPACTA_CP_AUTO_VAR:
							{
									CString strTemp;
									CString strText;
									if ( bImpactNormalDurationPlus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), fADDON_VAR );
										}
									}
									else if ( bImpactNormalDurationMinus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), -fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), -fADDON_VAR );
										}
									}
									else if ( bImpactNormalNonDurationPlus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
												, strIMPACT_ADDON.c_str(), fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
												, strIMPACT_ADDON.c_str(), fADDON_VAR );
										}
									}
									else if ( bImpactNormalNonDurationMinus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
												, strIMPACT_ADDON.c_str(), -fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
												, strIMPACT_ADDON.c_str(), -fADDON_VAR );
										}
									}
									else if ( bImpactPassiveDurationPlus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), fADDON_VAR );
										}
									}
									else if ( bImpactPassiveDurationMinus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), -fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), -fADDON_VAR );
										}
									}
									else if ( bImpactPassiveNonDurationPlus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
												, strIMPACT_ADDON.c_str(), fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
												, strIMPACT_ADDON.c_str(), fADDON_VAR );
										}
									}
									else if ( bImpactPassiveNonDurationMinus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
												, strIMPACT_ADDON.c_str(), -fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
												, strIMPACT_ADDON.c_str(), -fADDON_VAR );
										}
									}
									strText += strTemp;
									AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
							}
							break;
						};
					}
				}
			}
		}
				{
					for(int nSpec = 0; nSpec < SKILL::MAX_SPEC; ++nSpec)
					{
						const SKILL::SSPEC& sSPEC = sAPPLY.sSpecs[nSpec].sSPEC[wLevel];
						const EMSPEC_ADDON emSPEC = sAPPLY.sSpecs[nSpec].emSPEC;

						
						bool bAddon = false;

						if (emSPEC != EMSPECA_NULL)
						{
							if (!bAddon)AddTextLongestLineSplit("", NS_UITEXTCOLOR::LIGHTSKYBLUE);
							bAddon = true;
							{
								//teleport,dash,shift
								if (emSPEC == EMSPECA_PUSHPULL)
								{
									CString strText1;
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_1"), fVAR2_SCALE);
											}
											else
											{
												strText1.Format(ID2GAMEWORD("SKILL_SPEC_ADDON_F_1_RATE"), fVAR2_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText1, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_REFDAMAGE)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_2"),sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_REBIRTH)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_3"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//hp,mp,sp gather
								if (emSPEC == EMSPECA_HP_GATHER)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_4_RATE"), fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_4"), fVAR1_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_MP_GATHER)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_5_RATE"), fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_5"), fVAR1_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_SP_GATHER)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_6_RATE"), fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_6"), fVAR1_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_HP_DIV)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_7"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_MP_DIV)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_8"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_SP_DIV)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_9"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_NONBLOW )
								{
									CString strTemp;
									CString strText;
									if (sSPEC.dwFLAG & DIS_NUMB)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_NUMB].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_STUN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_STUN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_STONE)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_STONE].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_BURN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_BURN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_FROZEN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_FROZEN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_MAD)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_MAD].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_POISON)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_POISON].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_CURSE)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_CURSE].c_str());
										strText += strTemp;
									}
									CString strTemper;
									strTemper.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_10"), strText);
									AddTextLongestLineSplit(strTemper, NS_UITEXTCOLOR::GREEN);
								}
								
								if (emSPEC == EMSPECA_RECBLOW )
								{
									CString strTemp;
									CString strText;
									if (sSPEC.dwFLAG & DIS_NUMB)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_NUMB].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_STUN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_STUN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_STONE)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_STONE].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_BURN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_BURN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_FROZEN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_FROZEN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_MAD)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_MAD].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_POISON)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_POISON].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_CURSE)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_CURSE].c_str());
										strText += strTemp;
									}
									CString strTemper;
									strTemper.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_11"), strText);
									AddTextLongestLineSplit(strTemper, NS_UITEXTCOLOR::GREEN);
								}
								//emspeca pierce to do by CNDev
								if (emSPEC == EMSPECA_PIERCE)
								{
									CString strText;
									strText.Format("Fix Pierce Skill Description.");
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_TARRANGE )
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_MOVEVELO)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}

								if (emSPEC == EMSPECA_ONWARD)
								{
									CString TextShift;
									TextShift.Format(ID2GAMEWORD("EFF_SKILL_ONWARD"));
									AddTextNoSplit(TextShift, NS_UITEXTCOLOR::GREEN);

									/*if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * -100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_MYSKILLDELAYRATE",1),fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_MYSKILLDELAY",1),fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * COMMENT::SPEC_ADDON_VAR2_SCALE[emSPEC];;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format(ID2GAMEWORD("EFF_SKILL_MYSKILLDELAYRATE",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
											}
											else
											{
												strText1.Format(ID2GAMEWORD("EFF_SKILL_MYSKILLDELAY",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
											}
										}
									}*/
								}
								if (emSPEC == EMSPECA_INVISIBLE)
								{
									CString strText;
									if ( sDATA_LVL.fLIFE < 0 )
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_16_1"));
									}
									else
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_16"), sDATA_LVL.fLIFE);
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_RECVISIBLE)
								{
									CString strText;
									if ( sDATA_LVL.fLIFE < 0 )
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_17_1"));
									}
									else
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_17"), sDATA_LVL.fLIFE);
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
									
								}
								if (emSPEC == EMSPECA_ATTACKVELO)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * -100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//skill delay
								if (emSPEC == EMSPECA_SKILLDELAY)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * -100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_7"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_6"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_5"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_4"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_7_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_6_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_5_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_4_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_CRUSHING_BLOW)
								{
									CString strText;
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_20_1_RATE",0), fVAR2_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_20_1",0), fVAR2_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_PSY_DAMAGE_REDUCE)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_21_1"), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_21"), sDATA_LVL.fLIFE, fVAR1_SCALE);
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_21_1_RATE"), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_21_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_MAGIC_DAMAGE_REDUCE)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_22_1_RATE"), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_22_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_22_1"), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_22"), sDATA_LVL.fLIFE, fVAR1_SCALE);
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								
								if (emSPEC == EMSPECA_PSY_DAMAGE_REFLECTION)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * 100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format("%2.2f%%", fVAR1_SCALE);
											}
											else
											{
												strText.Format("%2.2f", fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format("%2.2f%%", fVAR2_SCALE);
											}
											else
											{
												strText1.Format("%2.2f", fVAR2_SCALE);
											}
										}
									}
									CString strConnect;
									strConnect.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_23"),sDATA_LVL.fLIFE, strText1, strText);
									AddTextLongestLineSplit(strConnect, NS_UITEXTCOLOR::GREEN);
								}

								if (emSPEC == EMSPECA_MAGIC_DAMAGE_REFLECTION)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * 100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format("%2.2f%%", fVAR1_SCALE);
											}
											else
											{
												strText.Format("%2.2f", fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format("%2.2f%%", fVAR2_SCALE);
											}
											else
											{
												strText1.Format("%2.2f", fVAR2_SCALE);
											}
										}
									}
									CString strConnect;
									strConnect.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_24"),sDATA_LVL.fLIFE, strText1, strText);
									AddTextLongestLineSplit(strConnect, NS_UITEXTCOLOR::GREEN);
								}

								//debuffs / dedebuffs
								if (emSPEC == EMSPECA_BUFF_REMOVE)
								{
									CString strText;
									if (pSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF)
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_25_2"));
									}
									else
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_25_1"));
									}
									AddTextLongestLineSplit(strText,NS_UITEXTCOLOR::GREEN);
								}
								//stun
								if (emSPEC == EMSPECA_STUN)
								{
									CString strText;
									if (pSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF)
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_26_2"), sDATA_LVL.fLIFE);
									}
									else
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_26"), sDATA_LVL.fLIFE);
									}
									AddTextLongestLineSplit(strText,NS_UITEXTCOLOR::GREEN);
								}
								//todo cndev defense skill active
								//pull,push,shock
								if (emSPEC == EMSPECA_PULL)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * 100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format("%2.2f%%", fVAR1_SCALE);
											}
											else
											{
												strText.Format("%2.2f", fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format("%2.2f%%", fVAR2_SCALE);
											}
											else
											{
												strText1.Format("%2.2f", fVAR2_SCALE);
											}
										}
									}
									CString strConnect;
									strConnect.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_28"), strText, strText1);
									AddTextLongestLineSplit(strConnect, NS_UITEXTCOLOR::GREEN);
								}
								//pull,push,shock
								if (emSPEC == EMSPECA_PUSH)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * 100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format("%2.2f%%", fVAR1_SCALE);
											}
											else
											{
												strText.Format("%2.2f", fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format("%2.2f%%", fVAR2_SCALE);
											}
											else
											{
												strText1.Format("%2.2f", fVAR2_SCALE);
											}
										}
									}
									CString strConnect;
									strConnect.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_29"), strText, strText1);
									AddTextLongestLineSplit(strConnect, NS_UITEXTCOLOR::GREEN);
								}
								//continous damage
								if (emSPEC == EMSPECA_CONTINUOUS_DAMAGE)
								{
									CString strText;
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_30_RATE",0), sDATA_LVL.fLIFE, fVAR2_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_30",0), sDATA_LVL.fLIFE, fVAR2_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//i left it here
								//curse
								if (emSPEC == EMSPECA_CURSE )
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_31_RATE",0), sDATA_LVL.fLIFE, fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_31",0), sDATA_LVL.fLIFE, fVAR1_SCALE);
											}

											AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
										}
									}
								}
								//range
								if (emSPEC == EMSPECA_CHANGE_ATTACK_RANGE )
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}

								if (emSPEC == EMSPECA_CHANGE_APPLY_RANGE)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_PROHIBIT_POTION)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_34"), sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_PROHIBIT_SKILL)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_35"), sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//emspeca_ignore_damage here
								//start tw buffs no need description
								//EMSPECA_ITEMDROP_RATE
								//EMSPECA_MONEYDROP_RATE
								//end tw buffs no need description
								//emspeca_getexp_rate here -- need rework
								if (emSPEC == EMSPECA_GETEXP_RATE)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_EXPRATE",0), sDATA_LVL.fLIFE,fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_EXP",0), sDATA_LVL.fLIFE,fVAR1_SCALE);
											}

											AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
										}
									}
								}
								//start tw buffs no need description
								//EMSPECA_TALK_TO_NPC
								//EMSPECA_SPECIAL_ITEM_DROP
								//end tw buffs no need description
								if (emSPEC == EMSPECA_TELEPORT)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_42"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_POSITION_SHIFT)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_43_4"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//emspeca_enhancement reinforcement rework
								if (emSPEC == EMSPECA_ENHANCEMENT)
								{
									CString strText;
									CString strText1;
									//if (0 != sSPEC.fVAR1)
									//{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_REINFORCEMENTRATE",0), sDATA_LVL.fLIFE, fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_REINFORCEMENT",0), sDATA_LVL.fLIFE, fVAR1_SCALE);
											}
										}
									//}
									//if (0 != sSPEC.fVAR2)
									//{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format(ID2GAMEWORD("EFF_SKILL_REINFORCEMENTRATE",1), fVAR2_SCALE);
											}
											else
											{
												strText1.Format(ID2GAMEWORD("EFF_SKILL_REINFORCEMENT",1), fVAR2_SCALE);
											}
										}
									//}
									AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
									AddTextNoSplit(strText1, NS_UITEXTCOLOR::GREEN);
								}
								//emspeca_illusion to do
								if (emSPEC == EMSPECA_VEHICLE_OFF)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_46_RATE"), fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_46"), fVAR1_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//duration inc
								if (emSPEC == EMSPECA_DURATION_CHANGE)
								{
									CString strText;
									CString strText1;

									if (pSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF)
									{

										if (0 != sSPEC.fVAR1)
										{
											if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
											{
												float fVAR1_SCALE = sSPEC.fVAR1 * 100;

												if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
												{
													strText.Format(ID2GAMEWORD("EFF_SKILL_MYDURATIONINCRATE",1),fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEWORD("EFF_SKILL_MYDURATIONINC",1),fVAR1_SCALE);
												}
											}
										}
										if (0 != sSPEC.fVAR2)
										{
											if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
											{
												float fVAR2_SCALE = sSPEC.fVAR2 * COMMENT::SPEC_ADDON_VAR2_SCALE[emSPEC];;

												if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
												{
													strText1.Format(ID2GAMEWORD("EFF_SKILL_MYDURATIONINCRATE",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
												}
												else
												{
													strText1.Format(ID2GAMEWORD("EFF_SKILL_MYDURATIONINC",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
												}
											}
										}
										AddTextNoSplit(strText1, NS_UITEXTCOLOR::GREEN);
										AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
									}
									else
									{
										if (0 != sSPEC.fVAR1)
										{
											if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
											{
												float fVAR1_SCALE = sSPEC.fVAR1 * 100;

												if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
												{
													strText.Format(ID2GAMEWORD("EFF_SKILL_ENEMYDURATIONINCRATE",1), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEWORD("EFF_SKILL_ENEMYDURATIONINC",1),fVAR1_SCALE);
												}
											}
										}
										if (0 != sSPEC.fVAR2)
										{
											if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
											{
												float fVAR2_SCALE = sSPEC.fVAR2 * COMMENT::SPEC_ADDON_VAR2_SCALE[emSPEC];;

												if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
												{
													strText1.Format(ID2GAMEWORD("EFF_SKILL_ENEMYDURATIONINCRATE",0), sDATA_LVL.fLIFE, fVAR2_SCALE);
												}
												else
												{
													strText1.Format(ID2GAMEWORD("EFF_SKILL_ENEMYDURATIONINC",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
												}
											}
										}
										AddTextNoSplit(strText1, NS_UITEXTCOLOR::GREEN);
										AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
									}
								}
								if (emSPEC == EMSPECA_STIGMA)
								{
									CString strText;
									CString strText1;
									CString strText2;
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												for ( int i = 0; i <SKILL::EMSPEC_STIGMA_TYPE_DWORD_DAMAGE_SKILL; i++ )
												{
													if ( sSPEC.dwFLAG  & ( 1 <<  i   ) )
													{
														strText.AppendFormat( "%s, ", COMMENT::SPEC_STIGMA_TYPE[ i ].c_str());
													}
												}
												//strText.Format(ID2GAMEWORD("EFF_SKILL_STIGMA_RATE",0), sDATA_LVL.fLIFE);
												//strText1.Format(ID2GAMEWORD("EFF_SKILL_STIGMA_RATE",1), fVAR2_SCALE);
												//strText2.Format(ID2GAMEWORD("EFF_SKILL_STIGMA_RATE",2));
												strText2.Format (ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_48"), sDATA_LVL.fLIFE, strText, fVAR2_SCALE);
											}
											else
											{
												for ( int i = 0; i <SKILL::EMSPEC_STIGMA_TYPE_DWORD_DAMAGE_SKILL; i++ )
												{
													if ( sSPEC.dwFLAG  & ( 1 <<  i   ) )
													{
														strText.AppendFormat( "%s, ", COMMENT::SPEC_STIGMA_TYPE[ i ].c_str());
													}
												}
												//strText.Format(ID2GAMEWORD("EFF_SKILL_STIGMA",0), sDATA_LVL.fLIFE);
												//strText1.Format(ID2GAMEWORD("EFF_SKILL_STIGMA",1), fVAR2_SCALE);
												//strText2.Format(ID2GAMEWORD("EFF_SKILL_STIGMA",2));
												strText2.Format (ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_48_RATE"), sDATA_LVL.fLIFE, strText, fVAR2_SCALE);
											}

											AddTextLongestLineSplit (strText2, NS_UITEXTCOLOR::GREEN);

											//AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
											//AddTextNoSplit(strText1, NS_UITEXTCOLOR::GREEN);
											//AddTextNoSplit(strText2, NS_UITEXTCOLOR::GREEN);
										}
									}
								}
								if (emSPEC == EMSPECA_TRANSFORM)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_49"), sDATA_LVL.fLIFE);
									AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_EFFECT_DURATION)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_50"));
									AddTextLongestLineSplit (strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_DAMAGE_LOOP)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_51"));
									AddTextLongestLineSplit (strText, NS_UITEXTCOLOR::GREEN);
								}
								//todo EMSPECA_PET_OFF
								/*if (emSPEC == EMSPECA_PET_OFF)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_52"));
									AddTextLongestLineSplit (strText, NS_UITEXTCOLOR::GREEN);
								}*/
								//immune
								if (emSPEC == EMSPECA_IMMUNE)
								{
									//CString strText;
									//strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_53"), sDATA_LVL.fLIFE);
									//AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
									CString strText;
									CString strText1;
									CString strText2;
									float fVAR1_SCALE = sSPEC.fVAR1;
									float fVAR2_SCALE = sSPEC.fVAR2;

									for ( int i = 0; i < SKILL::EMAPPLY_NSIZE; i++ )
									{
										if ( static_cast<DWORD>(fVAR1_SCALE) & ( 1 <<  i   ) )
										{
											strText1.AppendFormat( "%s, ", COMMENT::SKILL_APPLY[ i ].c_str());
										}
									}
									for ( int i = 0; i < SKILL::EMACTION_NSIZE; i++ )
									{
										if ( static_cast<DWORD>(fVAR2_SCALE) & ( 1 <<  i  ) )
										{
											strText2.AppendFormat( "%s, ", COMMENT::SKILL_ACTION_TYPE[ i ].c_str());
										}
									}
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_53_1"),strText1, strText2, sDATA_LVL.fLIFE);
									AddTextLongestLineSplit ( strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_HOSTILE_ON)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_54"), sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//todo EMSPECA_HOSTILE_OFF
								/*
								if (emSPEC == EMSPECA_HOSTILE_OFF)
								{
									CString strText;
									strText.Format(ID2GAMEWORD("EFF_SKILL_COMBAT_OFF"));
									AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
								}*/
								if (emSPEC == EMSPECA_MANIPULATE)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_57"), sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_RANDOM_EXP_RATE)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_42"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
							}
						}
					}
				}
		if (sAPPLY.emSTATE_BLOW != EMBLOW_NONE)
		{
			const SKILL::SSTATE_BLOW& sSTATE_BLOW = sAPPLY.sSTATE_BLOW[wLevel];

			CString strText;
			CString strText1;

			if (sAPPLY.emSTATE_BLOW == EMBLOW_NUMB)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					float fVAR = float(sSTATE_BLOW.fVAR1 * -100);
					float nVAR = float(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_NUMB_RN", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_NUMB_RN", 1), fVAR, nVAR);
					}
					else
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_NUMB_RN_1", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_NUMB_RN_1", 1), fVAR, nVAR);
					}

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
					AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_STUN)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					int fVAR = int(sSTATE_BLOW.fVAR1);
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_STUN"), sSTATE_BLOW.fRATE);
					else
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_STUN_RN"), sSTATE_BLOW.fRATE);

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_STONE)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					float fVAR = sSTATE_BLOW.fVAR1 * -100;
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_STONE_1", 0), sSTATE_BLOW.fRATE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_STONE_1", 1), fVAR);
					}
					else
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_STONE_2", 0), sSTATE_BLOW.fRATE, fVAR);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_STONE_2", 1), fVAR);
					}

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
					AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_BURN)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					float fVAR = sSTATE_BLOW.fVAR1 * -100;
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_BURN", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_BURN", 1), nVAR);
					}
					else
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_BURN_2", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE, nVAR);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_BURN_2", 1), nVAR);
					}

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
					AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_FROZEN)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					float fVAR = sSTATE_BLOW.fVAR1 * -100;
					float nVAR = float(sSTATE_BLOW.fVAR2 * 100);

					if (nRATE >= 100)
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_FROZEN_RN", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_FROZEN_RN", 1), fVAR, nVAR);
					}
					else
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_FROZEN_RN_1", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_FROZEN_RN_1", 1), fVAR, nVAR);
					}

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
					AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_MAD)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					int fVAR = int(sSTATE_BLOW.fVAR1);
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_MAD_TEST"));
					else
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_MAD_TEST"));

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_POISON)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					int fVAR = int(sSTATE_BLOW.fVAR1);
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_POISON", 0), sSTATE_BLOW.fRATE, nVAR);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_POISON", 1), sDATA_LVL.fLIFE);
					}
					else
					{
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_POISON_2", 0), sSTATE_BLOW.fRATE, nVAR);
						strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_POISON_2", 1), sDATA_LVL.fLIFE);
					}

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
					AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
				}
			}
			if (sAPPLY.emSTATE_BLOW == EMBLOW_CURSE)
			{
				if (sSTATE_BLOW.fRATE > 0)
				{
					AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

					int nRATE = int(sSTATE_BLOW.fRATE);
					int fVAR = int(sSTATE_BLOW.fVAR1);
					int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

					if (nRATE >= 100)
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_CURSE_TEST"));
					else
						strText.Format(ID2GAMEWORD("DES_STATE_BLOW_CURSE_TEST"));

					AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
				}
			}
		}
		if (!pSkill->GetDesc()) return;
	}

	void LOAD ( GLSKILL* const pSkill, SCHARSKILL* const pCharSkill, BOOL bNextLevel )
	{
		CString	strText;

		WORD wDispLevel = 0;
		WORD wNextLevel = wDispLevel;
		SKILL::SLEARN& sLEARN = pSkill->m_sLEARN;
		SKILL::SLEARN_LVL& sLVL = sLEARN.sLVL_STEP[wNextLevel];
		if ( pCharSkill )
			wDispLevel = pCharSkill->wLevel + 1;				
		else
		{
			if ( bNextLevel ) wDispLevel = 0;
		}

		if ( !pSkill ) 
        {     
            return ; 
        } 


		BOOL bMASTER = GLGaeaClient::GetInstance().GetCharacter()->ISMASTER_SKILL ( pSkill->m_sBASIC.sNATIVEID );

		{
			strText.Format("         %s", pSkill->GetName());
			CString strSkillLevel;
			strSkillLevel.Format(" (%d/%s)", wDispLevel, ID2GAMEWORD("SKILL_LEVEL",3));
			strText += strSkillLevel;
		}

		CString szSKillFlag;
		CString szSkillType;
		CString strBright;
		CString	strText1;
		if ( bNextLevel )
		{
			if ( bMASTER )
			{
				bNextLevel = FALSE;
				AddTextNoSplit ( "", NS_UITEXTCOLOR::PALEGREEN );
				strText1.Format( "           %d%s", pSkill->m_sBASIC.dwGRADE, ID2GAMEWORD("SKILL_RANK",0));
				AddTextNoSplit ( "", NS_UITEXTCOLOR::PALEGREEN );
			}
			else if (wDispLevel)
			{
				AddTextNoSplit("", NS_UITEXTCOLOR::PALEGREEN);
				strText.Format("           %d%s / %s:%d", pSkill->m_sBASIC.dwGRADE, ID2GAMEWORD("SKILL_RANK",0), ID2GAMEWORD("SKILL_NEXTLEVEL"), wDispLevel + 1);
				AddTextNoSplit(strText, NS_UITEXTCOLOR::RED);
				AddTextNoSplit("", NS_UITEXTCOLOR::PALEGREEN);
			}
			else
			{
				if ( pCharSkill )
				{
					strText1.Format( "           %d%s", pSkill->m_sBASIC.dwGRADE, ID2GAMEWORD("SKILL_RANK",0));
					AddTextNoSplit ( strText1, NS_UITEXTCOLOR::WHITE );
					AddTextNoSplit ( "", NS_UITEXTCOLOR::PALEGREEN );
				}
				else
				{
					AddTextNoSplit ( "", NS_UITEXTCOLOR::PALEGREEN );
					strText1.Format( "           %d%s", pSkill->m_sBASIC.dwGRADE, ID2GAMEWORD("SKILL_RANK",0));
					AddTextNoSplit ( strText1, NS_UITEXTCOLOR::WHITE );
					AddTextNoSplit ( "", NS_UITEXTCOLOR::PALEGREEN );
				}
			}
		}
		else
		{
			AddTextNoSplit ( "", NS_UITEXTCOLOR::PALEGREEN );
			strText1.Format("           %d%s", pSkill->m_sBASIC.dwGRADE, ID2GAMEWORD("SKILL_RANK",0));
			AddTextNoSplit ( strText1, NS_UITEXTCOLOR::WHITE );
			AddTextNoSplit ( "", NS_UITEXTCOLOR::PALEGREEN );
		}
		//normal or passive
		{
			CString strText;
			switch ( pSkill->m_sBASIC.emACTION )
			{
			case SKILL::EMACTION_NORMAL:
				{
					strText = ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 0  );
				}
				break;

			case SKILL::EMACTION_BUFF:
				{
					strText = ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 1  );
				}
				break;

			case SKILL::EMACTION_DEBUFF:
				{
					strText = ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 2  );
				}
				break;

			case SKILL::EMACTION_LIMIT:
				{
					strText = ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 3  );
				}
				break;
			}

			if ( pSkill->m_sBASIC.emROLE == SKILL::EMROLE_NORMAL )
			{
				switch ( pSkill->m_sBASIC.emAPPLY )
				{
				case SKILL::EMAPPLY_PHY_SHORT:
					{
						strText += "/";
						strText += ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 4 );
					}
					break;

				case SKILL::EMAPPLY_PHY_LONG:
					{
						strText += "/";
						strText += ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 5 );
					}
					break;

				case SKILL::EMAPPLY_MAGIC:
					{
						strText += "/";
						strText += ID2GAMEWORD( "SKILL_TOOLTIP_BASIC_TYPE", 6 );
					}
					break;
				}
			}
			else if ( pSkill->m_sBASIC.emROLE == SKILL::EMROLE_PASSIVE )
			{
				strText += "/";
				strText += ID2GAMEWORD( "SKILL_BASIC_INFO", 8 );
			}
			AddTextNoSplit (strText, NS_UITEXTCOLOR::DEFAULT);
		}
		{	
			//Applicable Weapon
			const SKILL::SSKILLBASIC& sBASIC = pSkill->m_sBASIC;		

			if ( sBASIC.emUSE_LITEM )
			{
				if ( sBASIC.emUSE_LITEM != ITEMATT_NOCARE )
				{
					if ( pSkill->m_sLEARN.sHiddenWeapon.wMainID != 65535 && pSkill->m_sLEARN.sHiddenWeapon.wSubID != 65535 ) strText.Format ( "%s:%s / Fist", ID2GAMEWORD( "SKILL_ADVANCED_USE_ITEM_LR_HAND", 0), ID2GAMEWORD( "SKILL_ADVANCED_USE_ITEM", sBASIC.emUSE_LITEM - 1 ) );
					else strText.Format ( "%s : %s",	ID2GAMEWORD( "SKILL_ADVANCED_USE_ITEM_LR_HAND", 0), ID2GAMEWORD( "SKILL_ADVANCED_USE_ITEM", sBASIC.emUSE_LITEM - 1 ) );
					AddTextNoSplit(strText, NS_UITEXTCOLOR::WHITE );
				}
			}

			if ( sBASIC.emUSE_RITEM )
			{
				if ( sBASIC.emUSE_RITEM != ITEMATT_NOCARE )
				{
					if ( pSkill->m_sLEARN.sHiddenWeapon.wMainID != 65535 && pSkill->m_sLEARN.sHiddenWeapon.wSubID != 65535 ) strText.Format ( "%s:%s / Fist", ID2GAMEWORD( "SKILL_ADVANCED_USE_ITEM_LR_HAND", 1), ID2GAMEWORD( "SKILL_ADVANCED_USE_ITEM", sBASIC.emUSE_RITEM - 1 ) );
					else strText.Format ( "%s : %s",	ID2GAMEWORD( "SKILL_ADVANCED_USE_ITEM_LR_HAND", 1), ID2GAMEWORD( "SKILL_ADVANCED_USE_ITEM", sBASIC.emUSE_RITEM - 1 ) );
					AddTextNoSplit(strText, NS_UITEXTCOLOR::WHITE );
				}
			}
		}

		const SKILL::SAPPLY& sAPPLY = pSkill->m_sAPPLY;

		//consumption
		if ( pCharSkill )
		{
			WORD wLevel = pCharSkill->wLevel;
			if ( bNextLevel ) wLevel = pCharSkill->wLevel + 1;
			const SKILL::CDATA_LVL& sDATA_LVL = sAPPLY.sDATA_LVL[wLevel];

			if ( pSkill->m_sBASIC.emROLE != SKILL::EMROLE_PASSIVE ) //Consumption Requirements
			{
				CString strConText; 

				if ( 0 < sDATA_LVL.wUSE_ARROWNUM	)	strConText.AppendFormat("%s(%d)",ID2GAMEWORD( "SKILL_CDATA_CVL_BASIC_USE_VALUE", 0), sDATA_LVL.wUSE_ARROWNUM);
				if ( 0 < sDATA_LVL.wUSE_CHARMNUM	)	strConText.AppendFormat("%s(%d)",ID2GAMEWORD( "SKILL_CDATA_CVL_BASIC_USE_VALUE", 1), sDATA_LVL.wUSE_CHARMNUM);
				if ( 0 < sDATA_LVL.wUSE_HP			)	strConText.AppendFormat("%s(%d)",ID2GAMEWORD( "SKILL_CDATA_CVL_BASIC_USE_VALUE", 3), sDATA_LVL.wUSE_HP);
				if ( 0 < sDATA_LVL.wUSE_MP			)	strConText.AppendFormat("%s(%d)",ID2GAMEWORD( "SKILL_CDATA_CVL_BASIC_USE_VALUE", 4), sDATA_LVL.wUSE_MP);
				if ( 0 < sDATA_LVL.wUSE_SP			)	strConText.AppendFormat("%s(%d)",ID2GAMEWORD( "SKILL_CDATA_CVL_BASIC_USE_VALUE", 5), sDATA_LVL.wUSE_SP);
				if ( 0 < sDATA_LVL.wUSE_CP			)	strConText.AppendFormat("%s(%d)",ID2GAMEWORD( "SKILL_CDATA_CVL_BASIC_USE_VALUE", 6), sDATA_LVL.wUSE_CP);
				if ( 0 < sDATA_LVL.wUSE_BULLETNUM	)	strConText.AppendFormat("%s(%d)",ID2GAMEWORD( "SKILL_CDATA_CVL_BASIC_USE_VALUE", 7), sDATA_LVL.wUSE_BULLETNUM);

				AddTextNoSplit(strConText, NS_UITEXTCOLOR::WHITE);
			}

			if ( 0 < sDATA_LVL.fDELAYTIME ) //Delay / Cooldown
			{
				strText.Format(ID2GAMEWORD("SKILL_EFF_COOLDOWN",0), sDATA_LVL.fDELAYTIME);
				AddTextNoSplit(strText, NS_UITEXTCOLOR::WHITE );
			}
		}
		else
		{
			WORD wLevel = 0;
			const SKILL::CDATA_LVL& sDATA_LVL = sAPPLY.sDATA_LVL[wLevel];

			if ( pSkill->m_sBASIC.emROLE != SKILL::EMROLE_PASSIVE ) //Consumption Requirements
			{
				CString strConText; 

				if ( 0 < sDATA_LVL.wUSE_ARROWNUM	)	strConText.AppendFormat("%s(%d)",ID2GAMEWORD( "SKILL_CDATA_CVL_BASIC_USE_VALUE", 0), sDATA_LVL.wUSE_ARROWNUM);
				if ( 0 < sDATA_LVL.wUSE_CHARMNUM	)	strConText.AppendFormat("%s(%d)",ID2GAMEWORD( "SKILL_CDATA_CVL_BASIC_USE_VALUE", 1), sDATA_LVL.wUSE_CHARMNUM);
				if ( 0 < sDATA_LVL.wUSE_HP			)	strConText.AppendFormat("%s(%d)",ID2GAMEWORD( "SKILL_CDATA_CVL_BASIC_USE_VALUE", 3), sDATA_LVL.wUSE_HP);
				if ( 0 < sDATA_LVL.wUSE_MP			)	strConText.AppendFormat("%s(%d)",ID2GAMEWORD( "SKILL_CDATA_CVL_BASIC_USE_VALUE", 4), sDATA_LVL.wUSE_MP);
				if ( 0 < sDATA_LVL.wUSE_SP			)	strConText.AppendFormat("%s(%d)",ID2GAMEWORD( "SKILL_CDATA_CVL_BASIC_USE_VALUE", 5), sDATA_LVL.wUSE_SP);
				if ( 0 < sDATA_LVL.wUSE_CP			)	strConText.AppendFormat("%s(%d)",ID2GAMEWORD( "SKILL_CDATA_CVL_BASIC_USE_VALUE", 6), sDATA_LVL.wUSE_CP);
				if ( 0 < sDATA_LVL.wUSE_BULLETNUM	)	strConText.AppendFormat("%s(%d)",ID2GAMEWORD( "SKILL_CDATA_CVL_BASIC_USE_VALUE", 7), sDATA_LVL.wUSE_BULLETNUM);

				AddTextNoSplit ( strConText, NS_UITEXTCOLOR::WHITE );
			}

			if ( 0 < sDATA_LVL.fDELAYTIME ) //Delay / Cooldown
			{
				strText.Format(ID2GAMEWORD("SKILL_EFF_COOLDOWN",0), sDATA_LVL.fDELAYTIME);
				AddTextNoSplit(strText, NS_UITEXTCOLOR::WHITE );
			}
		}

		BOOL bMaster = bMASTER;

		BOOL bNOT_LEARN = wDispLevel==0;

		if ( (!bMaster && bNextLevel) || bNOT_LEARN )
		{
			const SKILL::SSKILLBASIC& sBASIC = pSkill->m_sBASIC;		

			if ( sBASIC.emUSE_LITEM || sBASIC.emUSE_RITEM )
			{
				if ( sBASIC.emUSE_LITEM != ITEMATT_NOCARE || sBASIC.emUSE_RITEM != ITEMATT_NOCARE )
				{
					if ( pSkill->m_sBASIC.emROLE != SKILL::EMROLE_PASSIVE )
					{
						AddTextNoSplit("",NS_UITEXTCOLOR::WHITE);
					}
				}
			}

			AddTextNoSplit(ID2GAMEWORD("SKILL_REQUIREMENT" ,0), NS_UITEXTCOLOR::WHITE);
			

			WORD wNextLevel = wDispLevel;
			GLCharacter* const pCharacter = GLGaeaClient::GetInstance().GetCharacter();
			SKILL::SLEARN& sLEARN = pSkill->m_sLEARN;
			SKILL::SLEARN_LVL& sLVL = sLEARN.sLVL_STEP[wNextLevel];			

			SNATIVEID NeedSkillID = pSkill->m_sLEARN.sSKILL;			
			if ( NeedSkillID != NATIVEID_NULL() )
			{
				BOOL bVALID = FALSE;
				BOOL bNeedSkillLevel = FALSE;

				CString strNeedSkillName;
				CString strNeedSkillLevel;

				PGLSKILL pNeedSkill = GLSkillMan::GetInstance().GetData ( NeedSkillID.wMainID, NeedSkillID.wSubID );
				if( pNeedSkill ) strNeedSkillName.Format("%s", pNeedSkill->GetName() );
				bVALID = pCharacter->ISLEARNED_SKILL ( NeedSkillID );			

				if ( 0 < sLVL.dwSKILL_LVL )
				{
					strNeedSkillLevel.Format(ID2GAMEWORD("SKILL_LEVEL", 2), (sLVL.dwSKILL_LVL + 1) );
					bNeedSkillLevel = TRUE;

					SCHARDATA2::SKILL_MAP& map = pCharacter->m_ExpSkills;				
					SCHARDATA2::SKILL_MAP_ITER iter = map.find ( NeedSkillID.dwID );
					if ( iter != map.end() )
					{
						SCHARSKILL& rCharSkill = (*iter).second;

						bVALID = rCharSkill.wLevel >= sLVL.dwSKILL_LVL;						
					}
				}

				CString strCombine;

				if ( bNeedSkillLevel ) strCombine.Format ( "%s (%s)",strNeedSkillName,strNeedSkillLevel);
				else strCombine.Format ( "%s",strNeedSkillName);

				//mlTEXT.strText[1] = strCombine;
				//mlTEXT.dwColor[1] = NS_UITEXTCONTROL::GetEvaluateColor ( bVALID );
				AddTextNoSplit(strCombine, NS_UITEXTCONTROL::GetEvaluateColor ( bVALID ));
			}

			if ( 0 < sLVL.dwSKP )
			{
				strText.Format("%s %d ",ID2GAMEWORD( "SKILL_ADVANCED_INFO", 2), sLVL.dwSKP);		
				//mlTEXT.strText[2] = strText;
				//mlTEXT.dwColor[2] = NS_UITEXTCONTROL::GetEvaluateColor(pCharacter->m_dwSkillPoint >= sLVL.dwSKP);
				AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor(pCharacter->m_dwSkillPoint >= sLVL.dwSKP));
			}

			if ( 0 < sLVL.dwLEVEL )
			{
				strText.Format("%s %d ",ID2GAMEWORD( "SKILL_ADVANCED_INFO", 3), sLVL.dwLEVEL);
				//mlTEXT.strText[3] = strText;
				//mlTEXT.dwColor[3] = NS_UITEXTCONTROL::GetEvaluateColor(pCharacter->GETLEVEL() >= int(sLVL.dwLEVEL));
				AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor(pCharacter->GETLEVEL() >= int(sLVL.dwLEVEL)));
			}

			if ( 0 < sLVL.sSTATS.wPow )
			{
				strText.Format("%s %d ",ID2GAMEWORD( "SKILL_ADVANCED_INFO_STATS", 0), sLVL.sSTATS.wPow);
				//mlTEXT.strText[5] = strText;
				//mlTEXT.dwColor[5] = NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wPow >= sLVL.sSTATS.wPow );	
				AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wPow >= sLVL.sSTATS.wPow ) );
			}

			if ( 0 < sLVL.sSTATS.wStr )
			{
				strText.Format("%s %d ",ID2GAMEWORD( "SKILL_ADVANCED_INFO_STATS", 1), sLVL.sSTATS.wStr);
				//mlTEXT.strText[6] = strText;
				//mlTEXT.dwColor[6] = NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wStr >= sLVL.sSTATS.wStr );	
				AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wStr >= sLVL.sSTATS.wStr ) );
			}

			if ( 0 < sLVL.sSTATS.wSpi )
			{
				strText.Format("%s %d ",ID2GAMEWORD( "SKILL_ADVANCED_INFO_STATS", 2), sLVL.sSTATS.wSpi);
			//	mlTEXT.strText[7] = strText;
				//mlTEXT.dwColor[7] = NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wSpi >= sLVL.sSTATS.wSpi );	
				AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wSpi >= sLVL.sSTATS.wSpi ));
			}

			if ( 0 < sLVL.sSTATS.wDex )
			{
				strText.Format("%s %d ",ID2GAMEWORD( "SKILL_ADVANCED_INFO_STATS", 3), sLVL.sSTATS.wDex);
			//	mlTEXT.strText[8] = strText;
			//	mlTEXT.dwColor[8] = NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wDex >= sLVL.sSTATS.wDex );	
				AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wDex >= sLVL.sSTATS.wDex ) );
			}

			if ( 0 < sLVL.sSTATS.wInt )
			{
				strText.Format("%s %d ",ID2GAMEWORD( "SKILL_ADVANCED_INFO_STATS", 4), sLVL.sSTATS.wInt);
			//	mlTEXT.strText[9] = strText;
			//	mlTEXT.dwColor[9] = NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wInt >= sLVL.sSTATS.wInt );	
				AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wInt >= sLVL.sSTATS.wInt ) );
			}

			if ( 0 < sLVL.sSTATS.wSta )
			{
				strText.Format("%s %d ",ID2GAMEWORD( "SKILL_ADVANCED_INFO_STATS", 5), sLVL.sSTATS.wSta);
			//	mlTEXT.strText[10] = strText;
			//	mlTEXT.dwColor[10] = NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wSta >= sLVL.sSTATS.wSta );	
				AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor ( pCharacter->m_sSUMSTATS.wSta >= sLVL.sSTATS.wSta ) );
			}

			SCHARSKILL* pCharSkill = GLGaeaClient::GetInstance().GetCharacter()->GETLEARNED_SKILL(pSkill->m_sBASIC.sNATIVEID);
			if (!pCharSkill)
			{
				if (0 < GLCONST_CHAR::lnSKILL_PREREQ[pSkill->m_sBASIC.sNATIVEID.wMainID][pSkill->m_sBASIC.sNATIVEID.wSubID])
				{
					strText.Format("%s %d", ID2GAMEWORD("SKILL_ADVANCED_INFO", 5), GLCONST_CHAR::lnSKILL_PREREQ[pSkill->m_sBASIC.sNATIVEID.wMainID][pSkill->m_sBASIC.sNATIVEID.wSubID]);
					//mlTEXT.strText[10] = strText;
					//mlTEXT.dwColor[10] = NS_UITEXTCONTROL::GetEvaluateColor(pCharacter->m_sSUMSTATS.dwSta >= sLVL.sSTATS.dwSta);

					AddTextNoSplit(strText, NS_UITEXTCONTROL::GetEvaluateColor(pCharacter->m_lnMoney >= int(GLCONST_CHAR::lnSKILL_PREREQ[pSkill->m_sBASIC.sNATIVEID.wMainID][pSkill->m_sBASIC.sNATIVEID.wSubID])));
					
					//AddMultiTextLongestLineSplit(mlText);
				}
			}
		}

		AddTextNoSplit("", NS_UITEXTCOLOR::GREEN);
		//Skill Range
		{
			WORD wLevel = 0;
			if ( pCharSkill ) 
				wLevel = !bNextLevel ? pCharSkill->wLevel : pCharSkill->wLevel + 1;

			AddTextNoSplit(ID2GAMEWORD("SKILL_BASIC_INFO", 12), NS_UITEXTCOLOR::WHITE );
			
			const SKILL::CDATA_LVL& sDATA_LVL = sAPPLY.sDATA_LVL[wLevel];
			const SKILL::SSKILLBASIC& sBASIC = pSkill->m_sBASIC;	

			CString str;
			CString strText;
			switch ( pSkill->m_sBASIC.emIMPACT_TAR )
			{
			case TAR_SELF: // 자신
				switch ( pSkill->m_sBASIC.emIMPACT_REALM )
				{
				case REALM_SELF: // '목표' 자신
					strText += ID2GAMEINTEXT( "SKILL_AREA_SELF_SELF" );
					break;

				case REALM_ZONE: // '목표' 주위
					switch ( pSkill->m_sBASIC.emIMPACT_TAR )
					{
					case SIDE_OUR: // 자기편에게
						if ( 0 < sDATA_LVL.wTARNUM && 0 < sDATA_LVL.wAPPLYRANGE )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_SELF_ZONE_OUR" ),
							sDATA_LVL.wTARNUM, sDATA_LVL.wAPPLYRANGE);
							strText += str;
						}
						break;

					case SIDE_ENEMY: // 적에게
						if ( 0 < sDATA_LVL.wTARNUM && 0 < sBASIC.wTARRANGE )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_SELF_ZONE_ENEMY" ),
							sDATA_LVL.wTARNUM, sBASIC.wTARRANGE );
							strText += str;
						}
						break;
					case SIDE_ANYBODY: // 모두에게
						if ( 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_SELF_ZONE_ANYBODY" ),
							sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
							strText += str;
						}
						break;
					};
				};
				break;

			case TAR_SPEC: // 타겟
				{
					switch ( pSkill->m_sBASIC.emIMPACT_REALM )
					{
					case REALM_SELF: // '목표' 자신
						switch ( pSkill->m_sBASIC.emIMPACT_TAR )
						{
						case SIDE_OUR: // 자기편에게
							if ( 0 < sBASIC.wTARRANGE )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_SELF_OUR" ),
								sBASIC.wTARRANGE );
								strText += str;
							}
							break;
						case SIDE_ENEMY: // 적에게
							if ( 0 < sBASIC.wTARRANGE )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_SELF_ENEMY" ),
								sBASIC.wTARRANGE );
								strText += str;
							}
							break;
						case SIDE_ANYBODY: // 모두에게
							if ( 0 < sBASIC.wTARRANGE )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_SELF_ANYBODY" ),
								sBASIC.wTARRANGE );
								strText += str;
							}
							break;
						};

					case REALM_ZONE: // '목표' 주위
						switch ( pSkill->m_sBASIC.emIMPACT_TAR )
						{
						case SIDE_OUR: // 자기편에게
							if ( 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_ZONE_OUR" ),
								sBASIC.wTARRANGE, sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
								strText += str;
							}
							break;
						case SIDE_ENEMY: // 적에게
							if ( 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_ZONE_ENEMY" ),
								sBASIC.wTARRANGE, sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
								strText += str;
							}
							break;
						case SIDE_ANYBODY: // 모두에게
							if ( 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
							{
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_ZONE_ANYBODY" ),
								sBASIC.wTARRANGE, sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM  );
								strText += str;
							}
							break;
						};

					case REALM_FANWIZE:
						switch ( pSkill->m_sBASIC.emIMPACT_TAR )
						{
						case SIDE_ENEMY:
							if ( 0 < sDATA_LVL.wTARNUM && 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wAPPLYANGLE )
							{
								//made an exception for disabling 2 aoe desc
								if ( pSkill->m_sBASIC.sNATIVEID.wMainID == 6 && pSkill->m_sBASIC.sNATIVEID.wSubID == 4 ||
									pSkill->m_sBASIC.sNATIVEID.wMainID == 30 && pSkill->m_sBASIC.sNATIVEID.wSubID == 15 ||
									pSkill->m_sBASIC.sNATIVEID.wMainID == 30 && pSkill->m_sBASIC.sNATIVEID.wSubID == 21 ||
									pSkill->m_sBASIC.sNATIVEID.wMainID == 32 && pSkill->m_sBASIC.sNATIVEID.wSubID == 8 )	break;
								str.Format( ID2GAMEINTEXT( "SKILL_AREA_TARGET_FANWIZE_ENEMY" ),
								sDATA_LVL.wTARNUM, sBASIC.wTARRANGE % 10, sDATA_LVL.wAPPLYANGLE );
								strText += str;
							}
							break;
						};

					};
				}
				break;

			case TAR_SELF_TOSPEC: // 자신부터 타겟까지
				switch ( pSkill->m_sBASIC.emIMPACT_TAR )
				{
				case SIDE_ENEMY:
					//WORD wPierce = sDATA_LVL.wPIERCENUM + pTarget->GETSUM_PIERCE() + 1;
					WORD wPierce = sDATA_LVL.wPIERCENUM + 1;

					if ( 0 < sBASIC.wTARRANGE && 0 < sDATA_LVL.wTARNUM && 0 < wPierce )
					{
						str.Format( ID2GAMEINTEXT( "SKILL_AREA_TOSPEC_ENEMY" ),
						sBASIC.wTARRANGE, sDATA_LVL.wTARNUM, wPierce );
						strText += str;
					}
					break;
				};
				break;

			case TAR_ZONE: // 자신
				switch ( pSkill->m_sBASIC.emIMPACT_REALM )
				{
				case REALM_ZONE: // '목표' 주위
					if ( 0 < sDATA_LVL.wTARNUM && 0 < sDATA_LVL.wAPPLYRANGE )
					{
						str.Format( ID2GAMEINTEXT( "SKILL_AREA_ZONE_ENEMY" ),
						sDATA_LVL.wTARNUM, sDATA_LVL.wAPPLYRANGE );
						strText += str;
					}
					break;
				};
				break;

			/*case TAR_ZONE: // 지면 위치의 주변 타겟
				switch ( pSkill->m_sBASIC.emIMPACT_REALM )
				{
				case REALM_ZONE:
					switch ( pSkill->m_sBASIC.emIMPACT_TAR )
					{
					case SIDE_OUR:
						if ( 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_ZONE_OUR" ),
							sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
							strText += str;
						}
						break;
					case SIDE_ENEMY:
						if ( 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_ZONE_ENEMY" ),
							sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
							strText += str;
						}
						break;
					case SIDE_ANYBODY:
						if ( 0 < sDATA_LVL.wAPPLYRANGE && 0 < sDATA_LVL.wTARNUM )
						{
							str.Format( ID2GAMEINTEXT( "SKILL_AREA_ZONE_ANYBODY" ),
							sDATA_LVL.wAPPLYRANGE, sDATA_LVL.wTARNUM );
							strText += str;
						}
						break;
					};
					break;
				};
				break;*/

			case TAR_SPECIFIC: // 지면 위치
				strText += ID2GAMEINTEXT( "SKILL_AREA_LAND" );
				break;
			};

			AddTextNoSplit (strText, NS_UITEXTCOLOR::WHITE);
		}

		//Skill Effect
		WORD wLevel = 0;

		if ( pCharSkill )
			wLevel = !bNextLevel ? pCharSkill->wLevel : pCharSkill->wLevel + 1;
		
		const SKILL::CDATA_LVL& sDATA_LVL 	= sAPPLY.sDATA_LVL[wLevel];

		bool bEFFECT	= false;
		bool bEFFIMPACT	= false;
		AddTextNoSplit("", NS_UITEXTCOLOR::GREEN);
		if ( sDATA_LVL.fBASIC_VAR != 0 || sAPPLY.emSTATE_BLOW != EMBLOW_NONE || bEFFECT || bEFFIMPACT ) 
			AddTextNoSplit(ID2GAMEWORD("SKILL_BASIC_INFO", 13), NS_UITEXTCOLOR::GREEN );
					
		//Skill Damage
		{
			SKILL::EMROLE emRole = pSkill->m_sBASIC.emROLE;

			bool bNormalDurationPlus = false;
			bool bNormalDurationMinus = false;
			bool bNormalNonDurationPlus = false;
			bool bNormalNonDurationMinus = false;
			bool bPassiveDurationPlus = false;
			bool bPassiveDurationMinus = false;
			bool bPassiveNonDurationPlus = false;
			bool bPassiveNonDurationMinus = false;

			if ( emRole == SKILL::EMROLE_NORMAL )
			{
				if ( sDATA_LVL.fLIFE )
				{
					if ( sDATA_LVL.fBASIC_VAR > 0.0f )
						bNormalDurationPlus = true;
					else if ( sDATA_LVL.fBASIC_VAR < 0.0f )
						bNormalDurationMinus = true;
				}
				else
				{
					if ( sDATA_LVL.fBASIC_VAR > 0.0f )
						bNormalNonDurationPlus = true;
					else if ( sDATA_LVL.fBASIC_VAR < 0.0f )
						bNormalNonDurationMinus = true;
				}
			}
			else if ( emRole == SKILL::EMROLE_PASSIVE )
			{
				if ( sDATA_LVL.fLIFE )
				{
					if ( sDATA_LVL.fBASIC_VAR > 0.0f )
						bPassiveDurationPlus = true;
					else if ( sDATA_LVL.fBASIC_VAR < 0.0f )
						bPassiveDurationMinus = true;
				}
				else
				{
					if ( sDATA_LVL.fBASIC_VAR > 0.0f )
						bPassiveNonDurationPlus = true;
					else if ( sDATA_LVL.fBASIC_VAR < 0.0f )
						bPassiveNonDurationMinus = true;
				}
			}

			CString strTemp;
			CString strText;

            float fBASIC_VAR_SCALE;

			BOOL bSKILL_PER;
			std::string strSKILL_TYPE;
			
			fBASIC_VAR_SCALE = sDATA_LVL.fBASIC_VAR * COMMENT::SKILL_TYPES_SCALE[sAPPLY.emBASIC_TYPE];
			bSKILL_PER = COMMENT::IsSKILL_PER(sAPPLY.emBASIC_TYPE);

			switch ( sAPPLY.emBASIC_TYPE )
            {
				case SKILL::EMFOR_HP://	??? ??.
					{
						if ( bNormalDurationPlus || bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus || bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
								/* for ep11 soon
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_2")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5)
									, static_cast< float >( nSkillDmg )
									, static_cast< float >( gdDamage.dwNow )
									, static_cast< float >( gdDamage.dwMax ) );
									*/
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_MP://	??? ??.
					{
						if ( bNormalDurationPlus || bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus || bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_SP://	??? ??.
					{
						if ( bNormalDurationPlus || bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus || bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE1_1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1_1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
									, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_VARHP://	??? ??.
					{
						if ( bNormalDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus ) 
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[3].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[3].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[3].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[3].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[3].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[3].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[3].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[3].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[3].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_VARMP://	??? ??.
					{
						if ( bNormalDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus ) 
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[4].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[4].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[4].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[4].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[4].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[4].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[4].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[4].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[4].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
					
				case SKILL::EMFOR_VARSP://	??? ??.
					{
						if ( bNormalDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalDurationMinus ) 
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[5].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[5].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bNormalNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[5].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[5].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
									, sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[5].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationPlus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[5].c_str(), fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
									, COMMENT::SKILL_TYPES[5].c_str(), sDATA_LVL.fBASIC_VAR );
							}
						}
						else if ( bPassiveNonDurationMinus )
						{
							if ( bSKILL_PER == TRUE )
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
									, COMMENT::SKILL_TYPES[5].c_str(), -fBASIC_VAR_SCALE );
							}
							else
							{
								strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
									, COMMENT::SKILL_TYPES[5].c_str(), -sDATA_LVL.fBASIC_VAR );
							}
						}
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_DEFENSE://	??? ??.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[6].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[6].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[6].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[6].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[6].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[6].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[6].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[6].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[6].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_CURE://	??. ( ?? ?? ?? )
					{
						strTemp.Format( "%s", ID2GAMEINTEXT("SKILL_INFOR_CURE") );
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_HITRATE://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[8].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[8].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[8].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[8].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[8].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[8].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[8].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[8].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[8].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_AVOIDRATE://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[9].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[9].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[9].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[9].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[9].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[9].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[9].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[9].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[9].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_VARAP://	?? (??,??,??) ???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2_1")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE2")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2_1")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE2")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[10], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[10], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[10], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[10], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    , sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[10], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[10], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[10], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[10], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[10], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_VARDAMAGE://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[11].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[11].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[11].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[11].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[11].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[11].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[11].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[11].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[11].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_VARDEFENSE://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[12].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[12].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[12].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[12].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[12].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[12].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[12].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[12].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[12].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_PA://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[13].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[13].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[13].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[13].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[13].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[13].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[13].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[13].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[13].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_SA://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[14].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[14].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[14].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[14].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[14].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[14].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[14].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[14].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[14].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_MA://	???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[15].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[15].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[15].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[15].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[15].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[15].c_str(), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[15].c_str(), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[15].c_str(), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[15].c_str(), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_HP_RATE://	HP ???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 5), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 5), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 5), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 5), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 5), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_MP_RATE://	HP ???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 6), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 6), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 6), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 6), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 6), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_SP_RATE://	HP ???.
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE6")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, ID2GAMEWORD("ITEM_ADDON_INFO", 7), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 7), fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 7), sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE1_1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 7), -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE1")
                                    , ID2GAMEWORD("ITEM_ADDON_INFO", 7), -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_RESIST: //	?? ???
					{
						if ( bNormalDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[30], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[30], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bNormalNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
                                    , COMMENT::SKILL_TYPES[30], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
                                    , COMMENT::SKILL_TYPES[30], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
                                    ,sDATA_LVL.fLIFE, COMMENT::SKILL_TYPES[30], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationPlus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[30], fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[30], sDATA_LVL.fBASIC_VAR );
                            }
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            if ( bSKILL_PER == TRUE )
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
                                    , COMMENT::SKILL_TYPES[30], -fBASIC_VAR_SCALE );
                            }
                            else
                            {
                                strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
                                    , COMMENT::SKILL_TYPES[30], -sDATA_LVL.fBASIC_VAR );
                            }
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;

				case SKILL::EMFOR_SUMMONTIME: //	???? ??
					{
						if ( bPassiveNonDurationPlus )
                        {
                            strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE5")
                                , COMMENT::SKILL_TYPES[31], sDATA_LVL.fBASIC_VAR );
                        }
						else if ( bPassiveNonDurationMinus )
                        {
                            strTemp.Format( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE5")
                                , COMMENT::SKILL_TYPES[31], -sDATA_LVL.fBASIC_VAR );
                        }
						strText += strTemp;
						AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
					}
					break;
			//end of line here to avoid confusion
			};


		}
		// impact add on
		{
			for(int nImpact = 0; nImpact < SKILL::MAX_IMPACT; nImpact++ )
			{
				const EMIMPACT_ADDON emADDON = sAPPLY.sImpacts[nImpact].emADDON;

				float fADDON_VAR = sAPPLY.sImpacts[nImpact].fADDON_VAR[wLevel];
				float fADDON_VAR_SCALE = fADDON_VAR * COMMENT::IMPACT_ADDON_SCALE[sAPPLY.sImpacts[nImpact].emADDON];
				
				SKILL::EMROLE emRole = pSkill->m_sBASIC.emROLE;

				bool bImpactNormalDurationPlus = false;
				bool bImpactNormalDurationMinus = false;
				bool bImpactNormalNonDurationPlus = false;
				bool bImpactNormalNonDurationMinus = false;
				bool bImpactPassiveDurationPlus = false;
				bool bImpactPassiveDurationMinus = false;
				bool bImpactPassiveNonDurationPlus = false;
				bool bImpactPassiveNonDurationMinus = false;

				BOOL bSKILL_PER;
				std::string strIMPACT_ADDON;
				strIMPACT_ADDON = COMMENT::IMPACT_ADDON[emADDON].c_str();

				if ( emRole == SKILL::EMROLE_NORMAL )
				{
					if ( sDATA_LVL.fLIFE )
					{
						if ( fADDON_VAR > 0.0f )
							bImpactNormalDurationPlus = true;
						else if ( fADDON_VAR < 0.0f )
							bImpactNormalDurationMinus = true;
					}
					else
					{
						if ( fADDON_VAR > 0.0f )
							bImpactNormalNonDurationPlus = true;
						else if ( fADDON_VAR < 0.0f )
							bImpactNormalNonDurationMinus = true;
					}
				}
				else if ( emRole == SKILL::EMROLE_PASSIVE )
				{
					if ( sDATA_LVL.fLIFE )
					{
						if ( fADDON_VAR > 0.0f )
							bImpactPassiveDurationPlus = true;
						else if ( fADDON_VAR < 0.0f )
							bImpactPassiveDurationMinus = true;
					}
					else
					{
						if ( fADDON_VAR > 0.0f )
							bImpactPassiveNonDurationPlus = true;
						else if ( fADDON_VAR < 0.0f )
							bImpactPassiveNonDurationMinus = true;
					}
				}

				BOOL bIsIMPACT_ADDON_PER = FALSE;
				bIsIMPACT_ADDON_PER = COMMENT::IsIMPACT_ADDON_PER(emADDON);

				if ( sAPPLY.sImpacts[nImpact].emADDON != EMIMPACTA_NONE )
				{

					if (0 != fADDON_VAR)
					{
						if (!bEFFIMPACT)AddTextLongestLineSplit("", NS_UITEXTCOLOR::LIGHTSKYBLUE);
						bEFFIMPACT = true;

						switch ( emADDON )
						{
							case EMIMPACTA_HITRATE:     // 명중율 변화량.
							case EMIMPACTA_AVOIDRATE:   // 회피율 변화량.
							case EMIMPACTA_DAMAGE:  // 공격치 변화량.
							case EMIMPACTA_DEFENSE: // 방어치 변화량.
							case EMIMPACTA_VARHP:   // HP 변화율.
							case EMIMPACTA_VARMP:   // MP 변화율.
							case EMIMPACTA_VARSP:   // SP 변화율.
							case EMIMPACTA_VARAP:   // HP,MP,SP 변화율.
							case EMIMPACTA_DAMAGE_RATE:     // 공격치 변화율.
							case EMIMPACTA_DEFENSE_RATE:    // 방어치 변화율.
							case EMIMPACTA_PA:      //	격투치.
							case EMIMPACTA_SA:      //	사격치.
							case EMIMPACTA_MA:      //	방어치.
							case EMIMPACTA_HP_RATE: //	HP 증폭율.
							case EMIMPACTA_MP_RATE: //	MP 증폭율.
							case EMIMPACTA_SP_RATE: //	SP 증폭율.
							case EMIMPACTA_RESIST:  //	저항 수치 변경
							case EMIMPACTA_CHANGESTATS:   //	스탯 수치 변경 ( 힘,민첩,정신 )
							case EMIMPACTA_HP_RECOVERY_VAR:
							case EMIMPACTA_MP_RECOVERY_VAR:
							case EMIMPACTA_SP_RECOVERY_VAR:
							case EMIMPACTA_CP_RECOVERY_VAR:
							case EMIMPACTA_CP_AUTO_VAR:
							{
									CString strTemp;
									CString strText;
									if ( bImpactNormalDurationPlus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), fADDON_VAR );
										}
									}
									else if ( bImpactNormalDurationMinus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4_1")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), -fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE4")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), -fADDON_VAR );
										}
									}
									else if ( bImpactNormalNonDurationPlus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE4_1")
												, strIMPACT_ADDON.c_str(), fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_INCREASE_CASE5")
												, strIMPACT_ADDON.c_str(), fADDON_VAR );
										}
									}
									else if ( bImpactNormalNonDurationMinus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5_1")
												, strIMPACT_ADDON.c_str(), -fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_NORMAL_DECREASE_CASE5")
												, strIMPACT_ADDON.c_str(), -fADDON_VAR );
										}
									}
									else if ( bImpactPassiveDurationPlus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2_1")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE2")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), fADDON_VAR );
										}
									}
									else if ( bImpactPassiveDurationMinus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2_1")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), -fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE2")
												, sDATA_LVL.fLIFE, strIMPACT_ADDON.c_str(), -fADDON_VAR );
										}
									}
									else if ( bImpactPassiveNonDurationPlus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3_1")
												, strIMPACT_ADDON.c_str(), fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_INCREASE_CASE3")
												, strIMPACT_ADDON.c_str(), fADDON_VAR );
										}
									}
									else if ( bImpactPassiveNonDurationMinus )
									{
										if ( bIsIMPACT_ADDON_PER )
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3_1")
												, strIMPACT_ADDON.c_str(), -fADDON_VAR_SCALE );
										}
										else
										{
											strTemp.Format ( ID2GAMEINTEXT("SKILL_INFOR_PASSIVE_DECREASE_CASE3")
												, strIMPACT_ADDON.c_str(), -fADDON_VAR );
										}
									}
									strText += strTemp;
									AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN );
							}
							break;
						};
					}
				}
			}
		}
				{
					for(int nSpec = 0; nSpec < SKILL::MAX_SPEC; ++nSpec)
					{
						const SKILL::SSPEC& sSPEC = sAPPLY.sSpecs[nSpec].sSPEC[wLevel];
						const EMSPEC_ADDON emSPEC = sAPPLY.sSpecs[nSpec].emSPEC;

						
						bool bAddon = false;

						if (emSPEC != EMSPECA_NULL)
						{
							if (!bAddon)AddTextLongestLineSplit("", NS_UITEXTCOLOR::LIGHTSKYBLUE);
							bAddon = true;
							{
								//teleport,dash,shift
								if (emSPEC == EMSPECA_PUSHPULL)
								{
									CString strText1;
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_1"), fVAR2_SCALE);
											}
											else
											{
												strText1.Format(ID2GAMEWORD("SKILL_SPEC_ADDON_F_1_RATE"), fVAR2_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText1, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_REFDAMAGE)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_2"),sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_REBIRTH)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_3"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//hp,mp,sp gather
								if (emSPEC == EMSPECA_HP_GATHER)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_4_RATE"), fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_4"), fVAR1_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_MP_GATHER)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_5_RATE"), fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_5"), fVAR1_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_SP_GATHER)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_6_RATE"), fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_6"), fVAR1_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_HP_DIV)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_7"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_MP_DIV)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_8"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_SP_DIV)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_9"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_NONBLOW )
								{
									CString strTemp;
									CString strText;
									if (sSPEC.dwFLAG & DIS_NUMB)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_NUMB].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_STUN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_STUN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_STONE)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_STONE].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_BURN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_BURN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_FROZEN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_FROZEN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_MAD)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_MAD].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_POISON)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_POISON].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_CURSE)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_CURSE].c_str());
										strText += strTemp;
									}
									CString strTemper;
									strTemper.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_10"), strText);
									AddTextLongestLineSplit(strTemper, NS_UITEXTCOLOR::GREEN);
								}
								
								if (emSPEC == EMSPECA_RECBLOW )
								{
									CString strTemp;
									CString strText;
									if (sSPEC.dwFLAG & DIS_NUMB)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_NUMB].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_STUN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_STUN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_STONE)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_STONE].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_BURN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_BURN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_FROZEN)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_FROZEN].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_MAD)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_MAD].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_POISON)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_POISON].c_str());
										strText += strTemp;
									}
									if (sSPEC.dwFLAG & DIS_CURSE)
									{
										strTemp.Format("%s ", COMMENT::BLOW[EMBLOW_CURSE].c_str());
										strText += strTemp;
									}
									CString strTemper;
									strTemper.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_11"), strText);
									AddTextLongestLineSplit(strTemper, NS_UITEXTCOLOR::GREEN);
								}
								//emspeca pierce to do by CNDev
								if (emSPEC == EMSPECA_PIERCE)
								{
									CString strText;
									strText.Format("Fix Pierce Skill Description.");
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_TARRANGE )
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_13_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_MOVEVELO)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_14"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}

								if (emSPEC == EMSPECA_ONWARD)
								{
									CString TextShift;
									TextShift.Format(ID2GAMEWORD("EFF_SKILL_ONWARD"));
									AddTextNoSplit(TextShift, NS_UITEXTCOLOR::GREEN);

									/*if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * -100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_MYSKILLDELAYRATE",1),fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_MYSKILLDELAY",1),fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * COMMENT::SPEC_ADDON_VAR2_SCALE[emSPEC];;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format(ID2GAMEWORD("EFF_SKILL_MYSKILLDELAYRATE",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
											}
											else
											{
												strText1.Format(ID2GAMEWORD("EFF_SKILL_MYSKILLDELAY",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
											}
										}
									}*/
								}
								if (emSPEC == EMSPECA_INVISIBLE)
								{
									CString strText;
									if ( sDATA_LVL.fLIFE < 0 )
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_16_1"));
									}
									else
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_16"), sDATA_LVL.fLIFE);
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_RECVISIBLE)
								{
									CString strText;
									if ( sDATA_LVL.fLIFE < 0 )
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_17_1"));
									}
									else
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_17"), sDATA_LVL.fLIFE);
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
									
								}
								if (emSPEC == EMSPECA_ATTACKVELO)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * -100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_18"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//skill delay
								if (emSPEC == EMSPECA_SKILLDELAY)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * -100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_7"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_6"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_5"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_4"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_7_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_6_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_5_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_19_4_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_CRUSHING_BLOW)
								{
									CString strText;
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_20_1_RATE",0), fVAR2_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_20_1",0), fVAR2_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_PSY_DAMAGE_REDUCE)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_21_1"), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_21"), sDATA_LVL.fLIFE, fVAR1_SCALE);
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_21_1_RATE"), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_21_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_MAGIC_DAMAGE_REDUCE)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_22_1_RATE"), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_22_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_22_1"), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_22"), sDATA_LVL.fLIFE, fVAR1_SCALE);
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								
								if (emSPEC == EMSPECA_PSY_DAMAGE_REFLECTION)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * 100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format("%2.2f%%", fVAR1_SCALE);
											}
											else
											{
												strText.Format("%2.2f", fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format("%2.2f%%", fVAR2_SCALE);
											}
											else
											{
												strText1.Format("%2.2f", fVAR2_SCALE);
											}
										}
									}
									CString strConnect;
									strConnect.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_23"),sDATA_LVL.fLIFE, strText1, strText);
									AddTextLongestLineSplit(strConnect, NS_UITEXTCOLOR::GREEN);
								}

								if (emSPEC == EMSPECA_MAGIC_DAMAGE_REFLECTION)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * 100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format("%2.2f%%", fVAR1_SCALE);
											}
											else
											{
												strText.Format("%2.2f", fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format("%2.2f%%", fVAR2_SCALE);
											}
											else
											{
												strText1.Format("%2.2f", fVAR2_SCALE);
											}
										}
									}
									CString strConnect;
									strConnect.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_24"),sDATA_LVL.fLIFE, strText1, strText);
									AddTextLongestLineSplit(strConnect, NS_UITEXTCOLOR::GREEN);
								}

								//debuffs / dedebuffs
								if (emSPEC == EMSPECA_BUFF_REMOVE)
								{
									CString strText;
									if (pSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF)
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_25_2"));
									}
									else
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_25_1"));
									}
									AddTextLongestLineSplit(strText,NS_UITEXTCOLOR::GREEN);
								}
								//stun
								if (emSPEC == EMSPECA_STUN)
								{
									CString strText;
									if (pSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF)
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_26_2"), sDATA_LVL.fLIFE);
									}
									else
									{
										strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_26"), sDATA_LVL.fLIFE);
									}
									AddTextLongestLineSplit(strText,NS_UITEXTCOLOR::GREEN);
								}
								//todo cndev defense skill active
								//pull,push,shock
								if (emSPEC == EMSPECA_PULL)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * 100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format("%2.2f%%", fVAR1_SCALE);
											}
											else
											{
												strText.Format("%2.2f", fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format("%2.2f%%", fVAR2_SCALE);
											}
											else
											{
												strText1.Format("%2.2f", fVAR2_SCALE);
											}
										}
									}
									CString strConnect;
									strConnect.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_28"), strText, strText1);
									AddTextLongestLineSplit(strConnect, NS_UITEXTCOLOR::GREEN);
								}
								//pull,push,shock
								if (emSPEC == EMSPECA_PUSH)
								{
									CString strText;
									CString strText1;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * 100;

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format("%2.2f%%", fVAR1_SCALE);
											}
											else
											{
												strText.Format("%2.2f", fVAR1_SCALE);
											}
										}
									}
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format("%2.2f%%", fVAR2_SCALE);
											}
											else
											{
												strText1.Format("%2.2f", fVAR2_SCALE);
											}
										}
									}
									CString strConnect;
									strConnect.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_29"), strText, strText1);
									AddTextLongestLineSplit(strConnect, NS_UITEXTCOLOR::GREEN);
								}
								//continous damage
								if (emSPEC == EMSPECA_CONTINUOUS_DAMAGE)
								{
									CString strText;
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_30_RATE",0), sDATA_LVL.fLIFE, fVAR2_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_30",0), sDATA_LVL.fLIFE, fVAR2_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//i left it here
								//curse
								if (emSPEC == EMSPECA_CURSE )
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_31_RATE",0), sDATA_LVL.fLIFE, fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_31",0), sDATA_LVL.fLIFE, fVAR1_SCALE);
											}

											AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
										}
									}
								}
								//range
								if (emSPEC == EMSPECA_CHANGE_ATTACK_RANGE )
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_32_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}

								if (emSPEC == EMSPECA_CHANGE_APPLY_RANGE)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_3"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_2"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_1"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
											else
											{
												if ( sDATA_LVL.fLIFE < 0 )
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_3_RATE"), fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_2_RATE"), fVAR1_SCALE);
													}
												}
												else
												{
													if ( fVAR1_SCALE < 0 )
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_1_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
													else
													{
														strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_33_RATE"), sDATA_LVL.fLIFE, fVAR1_SCALE);
													}
												}
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_PROHIBIT_POTION)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_34"), sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_PROHIBIT_SKILL)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_35"), sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//emspeca_ignore_damage here
								//start tw buffs no need description
								//EMSPECA_ITEMDROP_RATE
								//EMSPECA_MONEYDROP_RATE
								//end tw buffs no need description
								//emspeca_getexp_rate here -- need rework
								if (emSPEC == EMSPECA_GETEXP_RATE)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_EXPRATE",0), sDATA_LVL.fLIFE,fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_EXP",0), sDATA_LVL.fLIFE,fVAR1_SCALE);
											}

											AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
										}
									}
								}
								//start tw buffs no need description
								//EMSPECA_TALK_TO_NPC
								//EMSPECA_SPECIAL_ITEM_DROP
								//end tw buffs no need description
								if (emSPEC == EMSPECA_TELEPORT)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_42"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_POSITION_SHIFT)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_43_4"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//emspeca_enhancement reinforcement rework
								if (emSPEC == EMSPECA_ENHANCEMENT)
								{
									CString strText;
									CString strText1;
									//if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_REINFORCEMENTRATE",0), sDATA_LVL.fLIFE, fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEWORD("EFF_SKILL_REINFORCEMENT",0), sDATA_LVL.fLIFE, fVAR1_SCALE);
											}
										}
									}
									//if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												strText1.Format(ID2GAMEWORD("EFF_SKILL_REINFORCEMENTRATE",1), fVAR2_SCALE);
											}
											else
											{
												strText1.Format(ID2GAMEWORD("EFF_SKILL_REINFORCEMENT",1), fVAR2_SCALE);
											}
										}
									}
									AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
									AddTextNoSplit(strText1, NS_UITEXTCOLOR::GREEN);
								}
								//emspeca_illusion to do
								if (emSPEC == EMSPECA_VEHICLE_OFF)
								{
									CString strText;
									if (0 != sSPEC.fVAR1)
									{
										if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
										{
											float fVAR1_SCALE = sSPEC.fVAR1 * COMMENT::SPEC_ADDON_VAR1_SCALE[emSPEC];

											if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_46_RATE"), fVAR1_SCALE);
											}
											else
											{
												strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_46"), fVAR1_SCALE);
											}
										}
									}
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//duration inc
								if (emSPEC == EMSPECA_DURATION_CHANGE)
								{
									CString strText;
									CString strText1;

									if (pSkill->m_sBASIC.emIMPACT_TAR == TAR_SELF)
									{

										if (0 != sSPEC.fVAR1)
										{
											if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
											{
												float fVAR1_SCALE = sSPEC.fVAR1 * 100;

												if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
												{
													strText.Format(ID2GAMEWORD("EFF_SKILL_MYDURATIONINCRATE",1),fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEWORD("EFF_SKILL_MYDURATIONINC",1),fVAR1_SCALE);
												}
											}
										}
										if (0 != sSPEC.fVAR2)
										{
											if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
											{
												float fVAR2_SCALE = sSPEC.fVAR2 * COMMENT::SPEC_ADDON_VAR2_SCALE[emSPEC];;

												if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
												{
													strText1.Format(ID2GAMEWORD("EFF_SKILL_MYDURATIONINCRATE",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
												}
												else
												{
													strText1.Format(ID2GAMEWORD("EFF_SKILL_MYDURATIONINC",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
												}
											}
										}
										AddTextNoSplit(strText1, NS_UITEXTCOLOR::GREEN);
										AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
									}
									else
									{
										if (0 != sSPEC.fVAR1)
										{
											if (COMMENT::SPEC_ADDON_VAR1[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR1[emSPEC])
											{
												float fVAR1_SCALE = sSPEC.fVAR1 * 100;

												if (COMMENT::IsSPEC_ADDON1_PER(emSPEC))
												{
													strText.Format(ID2GAMEWORD("EFF_SKILL_ENEMYDURATIONINCRATE",1), fVAR1_SCALE);
												}
												else
												{
													strText.Format(ID2GAMEWORD("EFF_SKILL_ENEMYDURATIONINC",1),fVAR1_SCALE);
												}
											}
										}
										if (0 != sSPEC.fVAR2)
										{
											if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
											{
												float fVAR2_SCALE = sSPEC.fVAR2 * COMMENT::SPEC_ADDON_VAR2_SCALE[emSPEC];;

												if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
												{
													strText1.Format(ID2GAMEWORD("EFF_SKILL_ENEMYDURATIONINCRATE",0), sDATA_LVL.fLIFE, fVAR2_SCALE);
												}
												else
												{
													strText1.Format(ID2GAMEWORD("EFF_SKILL_ENEMYDURATIONINC",0), sDATA_LVL.fLIFE,  fVAR2_SCALE);
												}
											}
										}
										AddTextNoSplit(strText1, NS_UITEXTCOLOR::GREEN);
										AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
									}
								}
								if (emSPEC == EMSPECA_STIGMA)
								{
									CString strText;
									CString strText1;
									CString strText2;
									if (0 != sSPEC.fVAR2)
									{
										if (COMMENT::SPEC_ADDON_VAR2[EMSPECA_NULL] != COMMENT::SPEC_ADDON_VAR2[emSPEC])
										{
											float fVAR2_SCALE = sSPEC.fVAR2 * 100;

											if (COMMENT::IsSPEC_ADDON2_PER(emSPEC))
											{
												for ( int i = 0; i <SKILL::EMSPEC_STIGMA_TYPE_DWORD_DAMAGE_SKILL; i++ )
												{
													if ( sSPEC.dwFLAG  & ( 1 <<  i   ) )
													{
														strText.AppendFormat( "%s, ", COMMENT::SPEC_STIGMA_TYPE[ i ].c_str());
													}
												}
												//strText.Format(ID2GAMEWORD("EFF_SKILL_STIGMA_RATE",0), sDATA_LVL.fLIFE);
												//strText1.Format(ID2GAMEWORD("EFF_SKILL_STIGMA_RATE",1), fVAR2_SCALE);
												//strText2.Format(ID2GAMEWORD("EFF_SKILL_STIGMA_RATE",2));
												strText2.Format (ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_48"), sDATA_LVL.fLIFE, strText, fVAR2_SCALE);
											}
											else
											{
												for ( int i = 0; i <SKILL::EMSPEC_STIGMA_TYPE_DWORD_DAMAGE_SKILL; i++ )
												{
													if ( sSPEC.dwFLAG  & ( 1 <<  i   ) )
													{
														strText.AppendFormat( "%s, ", COMMENT::SPEC_STIGMA_TYPE[ i ].c_str());
													}
												}
												//strText.Format(ID2GAMEWORD("EFF_SKILL_STIGMA",0), sDATA_LVL.fLIFE);
												//strText1.Format(ID2GAMEWORD("EFF_SKILL_STIGMA",1), fVAR2_SCALE);
												//strText2.Format(ID2GAMEWORD("EFF_SKILL_STIGMA",2));
												strText2.Format (ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_48_RATE"), sDATA_LVL.fLIFE, strText, fVAR2_SCALE);
											}

											AddTextLongestLineSplit (strText2, NS_UITEXTCOLOR::GREEN);

											//AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
											//AddTextNoSplit(strText1, NS_UITEXTCOLOR::GREEN);
											//AddTextNoSplit(strText2, NS_UITEXTCOLOR::GREEN);
										}
									}
								}
								if (emSPEC == EMSPECA_TRANSFORM)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_49"), sDATA_LVL.fLIFE);
									AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_EFFECT_DURATION)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_50"));
									AddTextLongestLineSplit (strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_DAMAGE_LOOP)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_51"));
									AddTextLongestLineSplit (strText, NS_UITEXTCOLOR::GREEN);
								}
								//todo EMSPECA_PET_OFF
								/*if (emSPEC == EMSPECA_PET_OFF)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_52"));
									AddTextLongestLineSplit (strText, NS_UITEXTCOLOR::GREEN);
								}*/
								//immune
								if (emSPEC == EMSPECA_IMMUNE)
								{
									//CString strText;
									//strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_53"), sDATA_LVL.fLIFE);
									//AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
									CString strText;
									CString strText1;
									CString strText2;
									float fVAR1_SCALE = sSPEC.fVAR1;
									float fVAR2_SCALE = sSPEC.fVAR2;

									for ( int i = 0; i < SKILL::EMAPPLY_NSIZE; i++ )
									{
										if ( static_cast<DWORD>(fVAR1_SCALE) & ( 1 <<  i   ) )
										{
											strText1.AppendFormat( "%s, ", COMMENT::SKILL_APPLY[i].c_str());
										}
									}
									for ( int i = 0; i < SKILL::EMACTION_NSIZE; i++ )
									{
										if ( static_cast<DWORD>(fVAR2_SCALE) & ( 1 <<  i  ) )
										{
											strText2.AppendFormat( "%s, ", COMMENT::SKILL_ACTION_TYPE[ i ].c_str());
										}
									}
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_53_1"),strText1, strText2, sDATA_LVL.fLIFE);
									AddTextLongestLineSplit ( strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_HOSTILE_ON)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_54"), sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								//todo EMSPECA_HOSTILE_OFF
								/*
								if (emSPEC == EMSPECA_HOSTILE_OFF)
								{
									CString strText;
									strText.Format(ID2GAMEWORD("EFF_SKILL_COMBAT_OFF"));
									AddTextNoSplit(strText, NS_UITEXTCOLOR::GREEN);
								}*/
								if (emSPEC == EMSPECA_MANIPULATE)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_57"), sDATA_LVL.fLIFE);
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
								if (emSPEC == EMSPECA_RANDOM_EXP_RATE)
								{
									CString strText;
									strText.Format(ID2GAMEINTEXT("SKILL_SPEC_ADDON_F_42"));
									AddTextLongestLineSplit(strText, NS_UITEXTCOLOR::GREEN);
								}
							}
						}
					}
				}
				if (sAPPLY.emSTATE_BLOW != EMBLOW_NONE)
				{
					const SKILL::SSTATE_BLOW& sSTATE_BLOW = sAPPLY.sSTATE_BLOW[wLevel];

					CString strText;
					CString strText1;

					if (sAPPLY.emSTATE_BLOW == EMBLOW_NUMB)
					{
						if (sSTATE_BLOW.fRATE > 0)
						{
							AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

							int nRATE = int(sSTATE_BLOW.fRATE);
							float fVAR = float(sSTATE_BLOW.fVAR1 * -100);
							float nVAR = float(sSTATE_BLOW.fVAR2 * -1);

							if (nRATE >= 100)
							{
								strText.Format(ID2GAMEWORD("DES_STATE_BLOW_NUMB_RN", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
								strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_NUMB_RN", 1), fVAR, nVAR);
							}
							else
							{
								strText.Format(ID2GAMEWORD("DES_STATE_BLOW_NUMB_RN_1", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
								strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_NUMB_RN_1", 1), fVAR, nVAR);
							}

							AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
							AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
						}
					}
					if (sAPPLY.emSTATE_BLOW == EMBLOW_STUN)
					{
						if (sSTATE_BLOW.fRATE > 0)
						{
							AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

							int nRATE = int(sSTATE_BLOW.fRATE);
							int fVAR = int(sSTATE_BLOW.fVAR1);
							int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

							if (nRATE >= 100)
								strText.Format(ID2GAMEWORD("DES_STATE_BLOW_STUN"), sSTATE_BLOW.fRATE);
							else
								strText.Format(ID2GAMEWORD("DES_STATE_BLOW_STUN_RN"), sSTATE_BLOW.fRATE);

							AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
						}
					}
					if (sAPPLY.emSTATE_BLOW == EMBLOW_STONE)
					{
						if (sSTATE_BLOW.fRATE > 0)
						{
							AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

							int nRATE = int(sSTATE_BLOW.fRATE);
							float fVAR = sSTATE_BLOW.fVAR1 * -100;
							int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

							if (nRATE >= 100)
							{
								strText.Format(ID2GAMEWORD("DES_STATE_BLOW_STONE_1", 0), sSTATE_BLOW.fRATE);
								strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_STONE_1", 1), fVAR);
							}
							else
							{
								strText.Format(ID2GAMEWORD("DES_STATE_BLOW_STONE_2", 0), sSTATE_BLOW.fRATE, fVAR);
								strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_STONE_2", 1), fVAR);
							}

							AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
							AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
						}
					}
					if (sAPPLY.emSTATE_BLOW == EMBLOW_BURN)
					{
						if (sSTATE_BLOW.fRATE > 0)
						{
							AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

							int nRATE = int(sSTATE_BLOW.fRATE);
							float fVAR = sSTATE_BLOW.fVAR1 * -100;
							int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

							if (nRATE >= 100)
							{
								strText.Format(ID2GAMEWORD("DES_STATE_BLOW_BURN", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
								strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_BURN", 1), nVAR);
							}
							else
							{
								strText.Format(ID2GAMEWORD("DES_STATE_BLOW_BURN_2", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE, nVAR);
								strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_BURN_2", 1), nVAR);
							}

							AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
							AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
						}
					}
					if (sAPPLY.emSTATE_BLOW == EMBLOW_FROZEN)
					{
						if (sSTATE_BLOW.fRATE > 0)
						{
							AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

							int nRATE = int(sSTATE_BLOW.fRATE);
							float fVAR = sSTATE_BLOW.fVAR1 * -100;
							float nVAR = float(sSTATE_BLOW.fVAR2 * 100);

							if (nRATE >= 100)
							{
								strText.Format(ID2GAMEWORD("DES_STATE_BLOW_FROZEN_RN", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
								strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_FROZEN_RN", 1), fVAR, nVAR);
							}
							else
							{
								strText.Format(ID2GAMEWORD("DES_STATE_BLOW_FROZEN_RN_1", 0), sSTATE_BLOW.fRATE, sDATA_LVL.fLIFE);
								strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_FROZEN_RN_1", 1), fVAR, nVAR);
							}

							AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
							AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
						}
					}
					if (sAPPLY.emSTATE_BLOW == EMBLOW_MAD)
					{
						if (sSTATE_BLOW.fRATE > 0)
						{
							AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

							int nRATE = int(sSTATE_BLOW.fRATE);
							int fVAR = int(sSTATE_BLOW.fVAR1);
							int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

							if (nRATE >= 100)
								strText.Format(ID2GAMEWORD("DES_STATE_BLOW_MAD_TEST"));
							else
								strText.Format(ID2GAMEWORD("DES_STATE_BLOW_MAD_TEST"));

							AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
						}
					}
					if (sAPPLY.emSTATE_BLOW == EMBLOW_POISON)
					{
						if (sSTATE_BLOW.fRATE > 0)
						{
							AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

							int nRATE = int(sSTATE_BLOW.fRATE);
							int fVAR = int(sSTATE_BLOW.fVAR1);
							int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

							if (nRATE >= 100)
							{
								strText.Format(ID2GAMEWORD("DES_STATE_BLOW_POISON", 0), sSTATE_BLOW.fRATE, nVAR);
								strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_POISON", 1), sDATA_LVL.fLIFE);
							}
							else
							{
								strText.Format(ID2GAMEWORD("DES_STATE_BLOW_POISON_2", 0), sSTATE_BLOW.fRATE, nVAR);
								strText1.Format(ID2GAMEWORD("DES_STATE_BLOW_POISON_2", 1), sDATA_LVL.fLIFE);
							}

							AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
							AddTextNoSplit(strText1, NS_UITEXTCOLOR::PRIVATE);
						}
					}
					if (sAPPLY.emSTATE_BLOW == EMBLOW_CURSE)
					{
						if (sSTATE_BLOW.fRATE > 0)
						{
							AddTextNoSplit("", NS_UITEXTCOLOR::PRIVATE);

							int nRATE = int(sSTATE_BLOW.fRATE);
							int fVAR = int(sSTATE_BLOW.fVAR1);
							int nVAR = int(sSTATE_BLOW.fVAR2 * -1);

							if (nRATE >= 100)
								strText.Format(ID2GAMEWORD("DES_STATE_BLOW_CURSE_TEST"));
							else
								strText.Format(ID2GAMEWORD("DES_STATE_BLOW_CURSE_TEST"));

							AddTextNoSplit(strText, NS_UITEXTCOLOR::PRIVATE);
						}
					}
				}
				{	
					bool isLinkSkillExist = false;
					bool isTitleDisplayed = false;

					for(int nSpec = 0; nSpec < SKILL::MAX_SPEC; ++nSpec)
					{
						const SKILL::SSPEC& sSPEC = sAPPLY.sSpecs[nSpec].sSPEC[wLevel];
						const EMSPEC_ADDON emSPEC = sAPPLY.sSpecs[nSpec].emSPEC;

						bool bAddon = false;

						if (emSPEC != EMSPECA_NULL)
						{
							if (!bAddon)AddTextLongestLineSplit("", NS_UITEXTCOLOR::LIGHTSKYBLUE);
							bAddon = true;
							{
								//teleport,dash,shift
								if (emSPEC == EMSPECA_SKILLLINK)
								{
									PGLSKILL pMainSkill = GLSkillMan::GetInstance().GetData(sSPEC.dwNativeID.wMainID, sSPEC.dwNativeID.wSubID);
									if (!pMainSkill) continue;

									PGLSKILL pPreReqSkill = GLSkillMan::GetInstance().GetData(sSPEC.dwLinkID.wMainID, sSPEC.dwLinkID.wSubID);
									if (!pPreReqSkill) continue;

									if (!isTitleDisplayed) isLinkSkillExist = true;

									if (!isTitleDisplayed && isLinkSkillExist)
									{
										AddTextNoSplit("", NS_UITEXTCOLOR::GOLD);
										AddTextNoSplit(ID2GAMEWORD("SKILL_BASIC_INFO", 14), NS_UITEXTCOLOR::GOLD);
										isTitleDisplayed = !isTitleDisplayed;
									}

									CString strDesc;
									strDesc.Format("100");
									strDesc += "%";

									CString strContinuation;
									strContinuation.Format(ID2GAMEINTEXT("SKILL_SPEC_F_LINK"),strDesc, pMainSkill->GetName(), pPreReqSkill->GetName());

									AddTextLongestLineSplit(strContinuation, NS_UITEXTCOLOR::GOLD);
								}
							}
						}
					}
				}

		if ( bNextLevel )		 return;
		if( !pSkill->GetDesc() ) return;
	}

	void LOAD ( const SNATIVEID& sNativeID, const BOOL bNextLevel )
	{
		if ( sNativeID != NATIVEID_NULL() )
		{
			BOOL bUPDATE = FALSE;
			if (( bNextLevel != m_bNextLevel_BEFORE_FRAME ) ||
				( sNativeID != m_sNativeIDBack) )
			{			
				bUPDATE = TRUE;
			}

			if ( bUPDATE )
			{
				PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sNativeID.wMainID, sNativeID.wSubID );
				if ( pSkill )	//	스킬이 존재할 경우
				{
					RemoveAllInfo ();

					SCHARSKILL* pCharSkill = GLGaeaClient::GetInstance().GetCharacter()->GETLEARNED_SKILL ( sNativeID );					
					LOAD ( pSkill, pCharSkill, bNextLevel );
				}

				m_sNativeIDBack = sNativeID;
				m_bNextLevel_BEFORE_FRAME = bNextLevel;
			}		
		}
	}
};