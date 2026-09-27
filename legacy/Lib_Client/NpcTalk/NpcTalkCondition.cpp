#include "StdAfx.h"
#include <strstream>
#include <algorithm>

#include "GLPartyClient.h"
#include "GLItemMan.h"
#include "NpcTalkCondition.h"
#include "SerialFile.h"
#include "basestream.h"
#include "GLogicEx.h"
#include "GLogicData.h"

#include "GLQUEST.h"
#include "GLQUESTMAN.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::string	strCONDITIONSIGN[CONDITIONSIGN_SIZE] =
{
	"N",
	"=",
	"<",
	">",
	"<=",
	">=",
};

SNpcTalkCondition::SNpcTalkCondition () :
	m_signLevel ( CONDITIONSIGN_NONE ),
	m_wLevel ( 0 ),
	m_dwClass ( GLCC_NONE ),
	m_wSchool ( MAX_SCHOOL ),
	m_signElemental ( CONDITIONSIGN_NONE ),
	m_nElemental ( 0 ),
	m_signActionPoint ( CONDITIONSIGN_NONE ),
	m_nActionPoint ( 0 ),
	m_bTime ( FALSE ),
	m_dwTimeBegin ( 0 ),
	m_dwTimeEnd ( 0 ),
	m_dwMONEY(0),
	m_dwPARTYMEN_NUM(0),
	m_dwQUEST_NID(UINT_MAX),
	m_dwQUEST_STEP(0),
	m_bEnable(TRUE)
{
}

BOOL SNpcTalkCondition::SAVE ( CSerialFile& SFile )
{
	if ( !SFile.IsOpen () )
	{
		GASSERT ( 0 && "파일이 열려있지 않습니다." );
		return FALSE;
	}

	SFile << (WORD)VERSION;

	SFile << m_bEnable;

	SFile << (DWORD)m_signLevel;
	SFile << m_wLevel;

	SFile << m_dwClass;
	SFile << m_wSchool;

	SFile << (DWORD)m_signElemental;
	SFile << m_nElemental;

	SFile << (DWORD)m_signActionPoint;
	SFile << m_nActionPoint;

	SFile << m_bTime;
	SFile << m_dwTimeBegin;
	SFile << m_dwTimeEnd;
	SFile << m_dwMONEY;
	SFile << m_dwPARTYMEN_NUM;
	SFile << m_dwQUEST_NID;
	SFile << m_dwQUEST_STEP;

	{
		SFile << (int)m_vecCompleteQuest.size();
		PROCSKILL_ITER iter = m_vecCompleteQuest.begin();
		PROCSKILL_ITER iter_end = m_vecCompleteQuest.end();
		for ( ; iter!=iter_end; ++iter )
		{
			SFile << (DWORD) (*iter);
		}
	}

	{
		SFile << (int)m_vecHaveItem.size();
		HAVEITEM_ITER iter = m_vecHaveItem.begin();
		HAVEITEM_ITER iter_end = m_vecHaveItem.end();
		for ( ; iter!=iter_end; ++iter )
		{
			const SNPC_ITEM &snpc_item = (*iter);
			SFile.WriteBuffer ( &snpc_item, sizeof ( SNPC_ITEM ) );
		}
	}

	{
		SFile << (int)m_vecLearnSkill.size();
		LEARNSKILL_ITER iter = m_vecLearnSkill.begin();
		LEARNSKILL_ITER iter_end = m_vecLearnSkill.end();
		for ( ; iter!=iter_end; ++iter )
		{
			const SSkillCondition &sSkillCondition = (*iter);
			SFile.WriteBuffer ( &sSkillCondition, sizeof ( SSkillCondition ) );
		}
	}

	{
		SFile << (int)m_vecDisQuest.size();
		PROCQUEST_ITER iter = m_vecDisQuest.begin();
		PROCQUEST_ITER iter_end = m_vecDisQuest.end();
		for ( ; iter!=iter_end; ++iter )
		{
			SFile << (DWORD) (*iter);
		}
	}

	{
		SFile << (int)m_vecDisSkill.size();
		PROCSKILL_ITER iter = m_vecDisSkill.begin();
		PROCSKILL_ITER iter_end = m_vecDisSkill.end();
		for ( ; iter!=iter_end; ++iter )
		{
			SFile << (DWORD) (*iter);
		}
	}

	{
		/*npc talk buff condition, Juver, 2017/10/02 */
		SFile << (int)m_vecBuff.size();
		HAVEBUFF_ITER iter = m_vecBuff.begin();
		HAVEBUFF_ITER iter_end = m_vecBuff.end();
		for ( ; iter!=iter_end; ++iter )
		{
			const SNPC_BUFF &sDataEx = (*iter);
			SFile.WriteBuffer ( &sDataEx, sizeof ( SNPC_BUFF ) );
		}
	} 


	return TRUE;
}

BOOL SNpcTalkCondition::LOAD_0001 ( basestream& SFile )
{
	DWORD dwData;
	BOOL bLevel(FALSE);
	BOOL bClass(FALSE);
	BOOL bElemental(FALSE);
	BOOL bActionPoint(FALSE);
	
	BOOL bHaveItem(FALSE);
	BOOL bLearnSkill(FALSE);
	int nCount = 0;

	SFile >> bLevel;
	SFile >> m_wLevel;
	SFile >> dwData;	m_signLevel = (EM_CONDITIONSIGN) dwData;
	if ( !bLevel )		m_signLevel = CONDITIONSIGN_NONE;

	SFile >> bClass;
	SFile >> m_dwClass;
	if ( !bClass )		m_dwClass = GLCC_ALL_8CLASS;

	SFile >> bElemental;
	SFile >> m_nElemental;

	SFile >> bElemental;
	SFile >> m_nElemental;
	SFile >> dwData; m_signElemental = (EM_CONDITIONSIGN) dwData;
	if ( !bElemental )		m_signElemental = CONDITIONSIGN_NONE;

	SFile >> bActionPoint;
	SFile >> m_nActionPoint;
	SFile >> dwData; m_signActionPoint = (EM_CONDITIONSIGN) dwData;
	if ( !bActionPoint )		m_signActionPoint = CONDITIONSIGN_NONE;

	SFile >> bHaveItem;
	SFile >> nCount;
	m_vecHaveItem.clear ();
	for ( int i = 0; i < nCount; i++ )
	{
		SNPC_ITEM snpc_item;
		SFile >> snpc_item.dwItemID;

		SNATIVEID sidItem(snpc_item.dwItemID);
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sidItem );
		if ( pItem )
			snpc_item.wItemNum = pItem->sDrugOp.wPileNum;
		else
			snpc_item.wItemNum = 1;

		m_vecHaveItem.push_back ( snpc_item );			
	}
	if ( !bHaveItem )		m_vecHaveItem.clear();

	SFile >> bLearnSkill;
	SFile >> nCount;
	m_vecLearnSkill.clear ();
	for ( int i = 0; i < nCount; i++ )
	{
		SSkillCondition sSkillCondition;
		SFile.ReadBuffer ( &sSkillCondition, sizeof ( SSkillCondition ) );
		m_vecLearnSkill.push_back ( sSkillCondition );
	}
	if ( !bLearnSkill )		m_vecLearnSkill.clear();

	SFile >> m_bTime;
	SFile >> m_dwTimeBegin;
	SFile >> m_dwTimeEnd;

	return TRUE;
}

BOOL SNpcTalkCondition::LOAD_0002 ( basestream& SFile )
{
	DWORD dwData;
	int nCount(0);
	
	SFile >> dwData;	m_signLevel = (EM_CONDITIONSIGN) dwData;
	SFile >> m_wLevel;

	SFile >> m_dwClass;

	SFile >> dwData;	m_signElemental = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nElemental;

	SFile >> dwData;	m_signActionPoint = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nActionPoint;

	SFile >> m_bTime;
	SFile >> m_dwTimeBegin;
	SFile >> m_dwTimeEnd;

	SFile >> m_dwQUEST_NID;
	SFile >> m_dwQUEST_STEP;

	SFile >> nCount;
	for ( int i=0; i<nCount; ++i)
	{
		SNPC_ITEM snpc_item;
		SFile >> snpc_item.dwItemID;

		SNATIVEID sidItem(snpc_item.dwItemID);
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sidItem );
		if ( pItem )
			snpc_item.wItemNum = pItem->sDrugOp.wPileNum;
		else
			snpc_item.wItemNum = 1;

		m_vecHaveItem.push_back ( snpc_item );		
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SSkillCondition sSkillCondition;
		SFile.ReadBuffer ( &sSkillCondition, sizeof(sSkillCondition) );
		m_vecLearnSkill.push_back ( sSkillCondition );
	}

	return TRUE;
}

BOOL SNpcTalkCondition::LOAD_0003 ( basestream& SFile )
{
	DWORD dwData;
	int nCount(0);
	
	SFile >> dwData;	m_signLevel = (EM_CONDITIONSIGN) dwData;
	SFile >> m_wLevel;

	SFile >> m_dwClass;

	SFile >> dwData;	m_signElemental = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nElemental;

	SFile >> dwData;	m_signActionPoint = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nActionPoint;

	SFile >> m_bTime;
	SFile >> m_dwTimeBegin;
	SFile >> m_dwTimeEnd;

	SFile >> m_dwQUEST_NID;
	SFile >> m_dwQUEST_STEP;

	SFile >> nCount;
	for ( int i=0; i<nCount; ++i)
	{
		SNPC_ITEM snpc_item;
		SFile >> snpc_item.dwItemID;

		SNATIVEID sidItem(snpc_item.dwItemID);
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sidItem );
		if ( pItem )
			snpc_item.wItemNum = pItem->sDrugOp.wPileNum;
		else
			snpc_item.wItemNum = 1;

		m_vecHaveItem.push_back ( snpc_item );		
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SSkillCondition sSkillCondition;
		SFile.ReadBuffer ( &sSkillCondition, sizeof(sSkillCondition) );
		m_vecLearnSkill.push_back ( sSkillCondition );
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecDisSkill.push_back(dwData);
	}
	
	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecDisQuest.push_back(dwData);
	}
	
	SFile >> m_dwMONEY;
	SFile >> m_dwPARTYMEN_NUM;

	return TRUE;
}

BOOL SNpcTalkCondition::LOAD_0004 ( basestream& SFile )
{
	DWORD dwData;
	int nCount(0);
	
	SFile >> dwData;	m_signLevel = (EM_CONDITIONSIGN) dwData;
	SFile >> m_wLevel;

	SFile >> m_dwClass;

	SFile >> dwData;	m_signElemental = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nElemental;

	SFile >> dwData;	m_signActionPoint = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nActionPoint;

	SFile >> m_bTime;
	SFile >> m_dwTimeBegin;
	SFile >> m_dwTimeEnd;

	SFile >> m_dwQUEST_NID;
	SFile >> m_dwQUEST_STEP;

	SFile >> nCount;
	for ( int i=0; i<nCount; ++i)
	{
		SNPC_ITEM snpc_item;
		SFile >> snpc_item.dwItemID;

		SNATIVEID sidItem(snpc_item.dwItemID);
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sidItem );
		if ( pItem )
			snpc_item.wItemNum = pItem->sDrugOp.wPileNum;
		else
			snpc_item.wItemNum = 1;

		m_vecHaveItem.push_back ( snpc_item );		
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SSkillCondition sSkillCondition;
		SFile.ReadBuffer ( &sSkillCondition, sizeof(sSkillCondition) );
		m_vecLearnSkill.push_back ( sSkillCondition );
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecCompleteQuest.push_back(dwData);
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecDisSkill.push_back(dwData);
	}
	
	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecDisQuest.push_back(dwData);
	}
	
	SFile >> m_dwMONEY;
	SFile >> m_dwPARTYMEN_NUM;

	return TRUE;
}

BOOL SNpcTalkCondition::LOAD_0005 ( basestream& SFile )
{
	DWORD dwData;
	int nCount(0);
	
	SFile >> dwData;	m_signLevel = (EM_CONDITIONSIGN) dwData;
	SFile >> m_wLevel;

	SFile >> m_dwClass;
	SFile >> m_wSchool;

	SFile >> dwData;	m_signElemental = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nElemental;

	SFile >> dwData;	m_signActionPoint = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nActionPoint;

	SFile >> m_bTime;
	SFile >> m_dwTimeBegin;
	SFile >> m_dwTimeEnd;

	SFile >> m_dwQUEST_NID;
	SFile >> m_dwQUEST_STEP;

	SFile >> nCount;
	for ( int i=0; i<nCount; ++i)
	{
		SNPC_ITEM snpc_item;
		SFile >> snpc_item.dwItemID;

		SNATIVEID sidItem(snpc_item.dwItemID);
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sidItem );
		if ( pItem )
			snpc_item.wItemNum = pItem->sDrugOp.wPileNum;
		else
			snpc_item.wItemNum = 1;

		m_vecHaveItem.push_back ( snpc_item );		
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SSkillCondition sSkillCondition;
		SFile.ReadBuffer ( &sSkillCondition, sizeof(sSkillCondition) );
		m_vecLearnSkill.push_back ( sSkillCondition );
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecCompleteQuest.push_back(dwData);
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecDisSkill.push_back(dwData);
	}
	
	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecDisQuest.push_back(dwData);
	}
	
	SFile >> m_dwMONEY;
	SFile >> m_dwPARTYMEN_NUM;


	// 클래스 자동 체크 기능 ...필요할때만 풀어서 사용한다.
/*	
	// 모든 클래스
	if ( m_dwClass == GLCC_ALL_OLD_EX )
	{
		m_dwClass = GLCC_ALL_NEWSEX;			
	}
	// 클래스 하나
	else
	{
		if ( m_dwClass&GLCC_BRAWLER_M ) m_dwClass |= GLCC_BRAWLER_W;
		if ( m_dwClass&GLCC_SWORDSMAN_M )	m_dwClass |= GLCC_SWORDSMAN_W;
		if ( m_dwClass&GLCC_ARCHER_W )	m_dwClass |= GLCC_ARCHER_M;
		if ( m_dwClass&GLCC_SHAMAN_W )	m_dwClass |= GLCC_SHAMAN_M;
	}
*/	
	return TRUE;
}

BOOL SNpcTalkCondition::LOAD_0006 ( basestream& SFile )
{
	DWORD dwData;
	int nCount(0);
	
	SFile >> dwData;	
	m_signLevel = (EM_CONDITIONSIGN) dwData;
	SFile >> m_wLevel;

	SFile >> m_dwClass;
	SFile >> m_wSchool;

	SFile >> dwData;	
	m_signElemental = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nElemental;

	SFile >> dwData;	
	m_signActionPoint = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nActionPoint;

	SFile >> m_bTime;
	SFile >> m_dwTimeBegin;
	SFile >> m_dwTimeEnd;

	SFile >> m_dwQUEST_NID;
	SFile >> m_dwQUEST_STEP;

	SFile >> nCount;
	for ( int i=0; i<nCount; ++i)
	{
		SNPC_ITEM snpc_item;
		SFile >> snpc_item.dwItemID;

		SNATIVEID sidItem(snpc_item.dwItemID);
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sidItem );
		if ( pItem )
			snpc_item.wItemNum = pItem->sDrugOp.wPileNum;
		else
			snpc_item.wItemNum = 1;

		m_vecHaveItem.push_back ( snpc_item );		
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SSkillCondition sSkillCondition;
		SFile.ReadBuffer ( &sSkillCondition, sizeof(sSkillCondition) );
		m_vecLearnSkill.push_back ( sSkillCondition );
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecCompleteQuest.push_back(dwData);
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecDisSkill.push_back(dwData);
	}
	
	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecDisQuest.push_back(dwData);
	}
	
	SFile >> m_dwMONEY;
	SFile >> m_dwPARTYMEN_NUM;

	return TRUE;
}

BOOL SNpcTalkCondition::LOAD_0007 ( basestream& SFile )
{
	DWORD dwData;
	int nCount(0);
	
	SFile >> dwData;	
	m_signLevel = (EM_CONDITIONSIGN) dwData;
	SFile >> m_wLevel;

	SFile >> m_dwClass;
	SFile >> m_wSchool;

	SFile >> dwData;	
	m_signElemental = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nElemental;

	SFile >> dwData;	
	m_signActionPoint = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nActionPoint;

	SFile >> m_bTime;
	SFile >> m_dwTimeBegin;
	SFile >> m_dwTimeEnd;
	SFile >> m_dwMONEY;
	SFile >> m_dwPARTYMEN_NUM;
	SFile >> m_dwQUEST_NID;
	SFile >> m_dwQUEST_STEP;

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecCompleteQuest.push_back(dwData);
	}

	SFile >> nCount;
	for ( int i=0; i<nCount; ++i)
	{
		SNPC_ITEM snpc_item;
		SFile >> snpc_item.dwItemID;

		SNATIVEID sidItem(snpc_item.dwItemID);
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sidItem );
		if ( pItem )
			snpc_item.wItemNum = pItem->sDrugOp.wPileNum;
		else
			snpc_item.wItemNum = 1;

		m_vecHaveItem.push_back ( snpc_item );		
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SSkillCondition sSkillCondition;
		SFile.ReadBuffer ( &sSkillCondition, sizeof(sSkillCondition) );
		m_vecLearnSkill.push_back ( sSkillCondition );
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecDisQuest.push_back(dwData);
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecDisSkill.push_back(dwData);
	}

	return TRUE;
}

BOOL SNpcTalkCondition::LOAD_0008 ( basestream& SFile )
{
	DWORD dwData;
	int nCount(0);

	SFile >> m_bEnable;

	SFile >> dwData;	
	m_signLevel = (EM_CONDITIONSIGN) dwData;
	SFile >> m_wLevel;

	SFile >> m_dwClass;
	SFile >> m_wSchool;

	SFile >> dwData;	
	m_signElemental = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nElemental;

	SFile >> dwData;	
	m_signActionPoint = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nActionPoint;

	SFile >> m_bTime;
	SFile >> m_dwTimeBegin;
	SFile >> m_dwTimeEnd;
	SFile >> m_dwMONEY;
	SFile >> m_dwPARTYMEN_NUM;
	SFile >> m_dwQUEST_NID;
	SFile >> m_dwQUEST_STEP;

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecCompleteQuest.push_back(dwData);
	}

	SFile >> nCount;
	for ( int i=0; i<nCount; ++i)
	{
		SNPC_ITEM snpc_item;
		SFile >> snpc_item.dwItemID;

		SNATIVEID sidItem(snpc_item.dwItemID);
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sidItem );
		if ( pItem )
			snpc_item.wItemNum = pItem->sDrugOp.wPileNum;
		else
			snpc_item.wItemNum = 1;

		m_vecHaveItem.push_back ( snpc_item );		
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SSkillCondition sSkillCondition;
		SFile.ReadBuffer ( &sSkillCondition, sizeof(sSkillCondition) );
		m_vecLearnSkill.push_back ( sSkillCondition );
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecDisQuest.push_back(dwData);
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecDisSkill.push_back(dwData);
	}

	return TRUE;
}

BOOL SNpcTalkCondition::LOAD_0009 ( basestream& SFile )
{
	DWORD dwData;
	int nCount(0);

	SFile >> m_bEnable;

	SFile >> dwData;	
	m_signLevel = (EM_CONDITIONSIGN) dwData;
	SFile >> m_wLevel;

	SFile >> m_dwClass;
	SFile >> m_wSchool;

	SFile >> dwData;	
	m_signElemental = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nElemental;

	SFile >> dwData;	
	m_signActionPoint = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nActionPoint;

	SFile >> m_bTime;
	SFile >> m_dwTimeBegin;
	SFile >> m_dwTimeEnd;
	SFile >> m_dwMONEY;
	SFile >> m_dwPARTYMEN_NUM;
	SFile >> m_dwQUEST_NID;
	SFile >> m_dwQUEST_STEP;

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecCompleteQuest.push_back(dwData);
	}

	SFile >> nCount;
	for ( int i=0; i<nCount; ++i)
	{
		SNPC_ITEM snpc_item;
		SFile.ReadBuffer ( &snpc_item, sizeof(snpc_item) );
		m_vecHaveItem.push_back(snpc_item);
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SSkillCondition sSkillCondition;
		SFile.ReadBuffer ( &sSkillCondition, sizeof(sSkillCondition) );
		m_vecLearnSkill.push_back ( sSkillCondition );
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecDisQuest.push_back(dwData);
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecDisSkill.push_back(dwData);
	}

	return TRUE;
}

BOOL SNpcTalkCondition::LOAD_0010 ( basestream& SFile )
{
	DWORD dwData;
	int nCount(0);

	SFile >> m_bEnable;

	SFile >> dwData;	
	m_signLevel = (EM_CONDITIONSIGN) dwData;
	SFile >> m_wLevel;

	SFile >> m_dwClass;
	SFile >> m_wSchool;

	SFile >> dwData;	
	m_signElemental = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nElemental;

	SFile >> dwData;	
	m_signActionPoint = (EM_CONDITIONSIGN) dwData;
	SFile >> m_nActionPoint;

	SFile >> m_bTime;
	SFile >> m_dwTimeBegin;
	SFile >> m_dwTimeEnd;
	SFile >> m_dwMONEY;
	SFile >> m_dwPARTYMEN_NUM;
	SFile >> m_dwQUEST_NID;
	SFile >> m_dwQUEST_STEP;

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecCompleteQuest.push_back(dwData);
	}

	SFile >> nCount;
	for ( int i=0; i<nCount; ++i)
	{
		SNPC_ITEM snpc_item;
		SFile.ReadBuffer ( &snpc_item, sizeof(snpc_item) );
		m_vecHaveItem.push_back(snpc_item);
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SSkillCondition sSkillCondition;
		SFile.ReadBuffer ( &sSkillCondition, sizeof(sSkillCondition) );
		m_vecLearnSkill.push_back ( sSkillCondition );
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecDisQuest.push_back(dwData);
	}

	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SFile >> dwData;
		m_vecDisSkill.push_back(dwData);
	}

	/*npc talk buff condition, Juver, 2017/10/02 */
	SFile >> nCount;
	for (int i=0; i<nCount; ++i )
	{
		SNPC_BUFF sBuffCondition;
		SFile.ReadBuffer ( &sBuffCondition, sizeof(sBuffCondition) );
		m_vecBuff.push_back ( sBuffCondition );
	}

	return TRUE;
}

BOOL SNpcTalkCondition::LOAD ( basestream& SFile )
{
	if ( !SFile.IsOpen () )
	{
		GASSERT ( 0 && "파일이 열려있지 않습니다." );
		return FALSE;
	}

	WORD wVer(0);
	SFile >> wVer;

	switch ( wVer )
	{
	case 0x0001:	LOAD_0001(SFile);	break;
	case 0x0002:	LOAD_0002(SFile);	break;
	case 0x0003:	LOAD_0003(SFile);	break;
	case 0x0004:	LOAD_0004(SFile);	break;
	case 0x0005:	LOAD_0005(SFile);	break;
	case 0x0006:	LOAD_0006(SFile);	break;
	case 0x0007:	LOAD_0007(SFile);	break;
	case 0x0008:	LOAD_0008(SFile);	break;
	case 0x0009:	LOAD_0009(SFile);	break;
	case SNpcTalkCondition::VERSION:	LOAD_0010(SFile);	break;

	default:
		CDebugSet::ErrorVersion( "SNpcTalkCondition::LOAD", wVer );
		break;
	};

	return TRUE;
}

void SNpcTalkCondition::Reset ()
{
	m_bEnable = TRUE;

	m_signLevel = CONDITIONSIGN_NONE;
	m_wLevel = 0;

	m_dwClass = GLCC_NONE;
	m_wSchool = MAX_SCHOOL;

	m_signElemental = CONDITIONSIGN_NONE;
	m_nElemental = 0;

	m_signActionPoint = CONDITIONSIGN_NONE;
	m_nActionPoint = 0;

	m_bTime = FALSE;
	m_dwTimeBegin = 0;
	m_dwTimeEnd = 0;

	m_dwQUEST_NID = UINT_MAX;
	m_dwQUEST_STEP = 0;

	m_vecHaveItem.clear ();
	m_vecLearnSkill.clear ();

	m_vecCompleteQuest.clear ();

	m_vecDisSkill.clear ();
	m_vecDisQuest.clear ();

	/*npc talk buff condition, Juver, 2017/10/02 */
	m_vecBuff.clear();

	m_dwMONEY = 0;
	m_dwPARTYMEN_NUM = 0;
}

SNpcTalkCondition&	SNpcTalkCondition::operator= ( const SNpcTalkCondition& rNpcTalkCondition )
{
	m_bEnable = rNpcTalkCondition.m_bEnable;

	m_signLevel = rNpcTalkCondition.m_signLevel;
	m_wLevel = rNpcTalkCondition.m_wLevel;

	m_dwClass = rNpcTalkCondition.m_dwClass;
	m_wSchool = rNpcTalkCondition.m_wSchool;

	m_signElemental = rNpcTalkCondition.m_signElemental;
	m_nElemental = rNpcTalkCondition.m_nElemental;

	m_signActionPoint = rNpcTalkCondition.m_signActionPoint;
	m_nActionPoint = rNpcTalkCondition.m_nActionPoint;

	m_bTime = rNpcTalkCondition.m_bTime;
	m_dwTimeBegin = rNpcTalkCondition.m_dwTimeBegin;
	m_dwTimeEnd = rNpcTalkCondition.m_dwTimeEnd;

	m_dwQUEST_NID = rNpcTalkCondition.m_dwQUEST_NID;
	m_dwQUEST_STEP = rNpcTalkCondition.m_dwQUEST_STEP;

	m_vecHaveItem = rNpcTalkCondition.m_vecHaveItem;
	m_vecLearnSkill = rNpcTalkCondition.m_vecLearnSkill;

	m_vecCompleteQuest = rNpcTalkCondition.m_vecCompleteQuest;

	m_vecDisSkill = rNpcTalkCondition.m_vecDisSkill;
	m_vecDisQuest = rNpcTalkCondition.m_vecDisQuest;

	m_dwMONEY = rNpcTalkCondition.m_dwMONEY;
	m_dwPARTYMEN_NUM = rNpcTalkCondition.m_dwPARTYMEN_NUM;

	/*npc talk buff condition, Juver, 2017/10/02 */
	m_vecBuff = rNpcTalkCondition.m_vecBuff;

	return *this;
}

BOOL SNpcTalkCondition::ADDHAVEITEM ( const SNPC_ITEM &sNpcItem )
{
	bool bfound = std::binary_search ( m_vecHaveItem.begin(), m_vecHaveItem.end(), sNpcItem );
	if ( bfound )	return FALSE;

	m_vecHaveItem.push_back ( sNpcItem );
	std::sort ( m_vecHaveItem.begin(), m_vecHaveItem.end() );

	return TRUE;
}

BOOL SNpcTalkCondition::DELHAVEITEM ( DWORD dwNID )
{
	SNPC_ITEM snpc_item;
	snpc_item.dwItemID = dwNID;

	m_vecHaveItem.erase ( std::remove(m_vecHaveItem.begin(),m_vecHaveItem.end(),snpc_item), m_vecHaveItem.end() );

	return TRUE;
}

BOOL SNpcTalkCondition::ADDLEARNSKILL ( const SSkillCondition &sSkillCondition )
{
	bool bfound = std::binary_search ( m_vecLearnSkill.begin(), m_vecLearnSkill.end(), sSkillCondition );
	if ( bfound )	return FALSE;

	m_vecLearnSkill.push_back ( sSkillCondition );
	std::sort ( m_vecLearnSkill.begin(), m_vecLearnSkill.end() );

	return TRUE;
}

BOOL SNpcTalkCondition::DELLEARNSKILL ( DWORD dwNID )
{
	SSkillCondition sSkillCondition;
	sSkillCondition.dwNID = dwNID;

	m_vecLearnSkill.erase ( std::remove(m_vecLearnSkill.begin(),m_vecLearnSkill.end(),sSkillCondition), m_vecLearnSkill.end() );

	return TRUE;
}

BOOL SNpcTalkCondition::ADDDISSKILL ( DWORD dwNID )
{
	bool bfound = std::binary_search ( m_vecDisSkill.begin(), m_vecDisSkill.end(), dwNID );
	if ( bfound )	return FALSE;

	m_vecDisSkill.push_back ( dwNID );
	std::sort ( m_vecDisSkill.begin(), m_vecDisSkill.end() );

	return TRUE;
}

BOOL SNpcTalkCondition::DELDISSKILL ( DWORD dwNID )
{
	m_vecDisSkill.erase ( std::remove(m_vecDisSkill.begin(),m_vecDisSkill.end(),dwNID), m_vecDisSkill.end() );

	return TRUE;
}

BOOL SNpcTalkCondition::ADDDISQUEST ( DWORD dwNID )
{
	bool bfound = std::binary_search ( m_vecDisQuest.begin(), m_vecDisQuest.end(), dwNID );
	if ( bfound )	return FALSE;

	m_vecDisQuest.push_back ( dwNID );
	std::sort ( m_vecDisQuest.begin(), m_vecDisQuest.end() );

	return TRUE;
}

BOOL SNpcTalkCondition::DELDISQUEST ( DWORD dwNID )
{
	m_vecDisQuest.erase ( std::remove(m_vecDisQuest.begin(),m_vecDisQuest.end(),dwNID), m_vecDisQuest.end() );

	return TRUE;
}

BOOL SNpcTalkCondition::ADDCOMQUEST ( DWORD dwNID )
{
	bool bfound = std::binary_search ( m_vecCompleteQuest.begin(), m_vecCompleteQuest.end(), dwNID );
	if ( bfound )	return FALSE;

	m_vecCompleteQuest.push_back ( dwNID );
	std::sort ( m_vecCompleteQuest.begin(), m_vecCompleteQuest.end() );

	return TRUE;
}

BOOL SNpcTalkCondition::DELCOMQUEST ( DWORD dwNID )
{
	m_vecCompleteQuest.erase ( std::remove(m_vecCompleteQuest.begin(),m_vecCompleteQuest.end(),dwNID), m_vecCompleteQuest.end() );

	return TRUE;
}

/*npc talk buff condition, Juver, 2017/10/02 */
BOOL SNpcTalkCondition::ADDBUFF ( const SNPC_BUFF &sNpcBuff )
{
	bool bfound = std::binary_search ( m_vecBuff.begin(), m_vecBuff.end(), sNpcBuff );
	if ( bfound )	return FALSE;

	m_vecBuff.push_back ( sNpcBuff );
	std::sort ( m_vecBuff.begin(), m_vecBuff.end() );

	return TRUE;
}

BOOL SNpcTalkCondition::DELBUFF ( DWORD dwNID )
{
	SNPC_BUFF snpc_buff;
	snpc_buff.dwID = dwNID;

	m_vecBuff.erase ( std::remove(m_vecBuff.begin(),m_vecBuff.end(),snpc_buff), m_vecBuff.end() );

	return TRUE;
}

BOOL SNpcTalkCondition::DoTEST ( GLCHARLOGIC *pCHAR )
{
	BOOL bValid(FALSE);
	SNATIVEID sNID;
	
	//	레벨
	bValid = NumericCompare ( m_wLevel, pCHAR->m_wLevel, m_signLevel );
	if ( !bValid )	return FALSE;

	//	클래스
	bValid = (m_dwClass&pCHAR->m_emClass)!=NULL;
	if ( !bValid )	return FALSE;

	if ( m_wSchool!=MAX_SCHOOL )
	{
		if ( m_wSchool!=pCHAR->m_wSchool )		return FALSE;
	}

	//	Elemental
	bValid = NumericCompare ( m_nElemental, pCHAR->m_nBright, m_signElemental );
	if ( !bValid )	return FALSE;

	//	생활점수
	bValid = NumericCompare ( m_nActionPoint, pCHAR->m_nBright, m_signActionPoint );
	if ( !bValid )	return FALSE;

	DWORD dwCurHour = GLPeriod::GetInstance().GetHour ();
	if ( m_bTime && !((m_dwTimeBegin<=dwCurHour)&&(dwCurHour<=m_dwTimeEnd)) )
	{
		return FALSE;
	}

	//	퀘스트 진행중인지 검사.
	if ( m_dwQUEST_NID!=UINT_MAX )
	{
		GLQUESTPROG *pQPROC = pCHAR->m_cQuestPlay.FindProc ( m_dwQUEST_NID );
		if ( !pQPROC )								return FALSE;
		if ( pQPROC->m_dwSTEP!=m_dwQUEST_STEP )		return FALSE;
	}

	//	소지 아이템 검사.
	{
		HAVEITEM_ITER iter = m_vecHaveItem.begin ();
		HAVEITEM_ITER iter_end = m_vecHaveItem.end ();
		for ( ; iter!=iter_end; ++iter )
		{
			SNPC_ITEM snpc_item = (*iter);
			SNATIVEID sid_item( snpc_item.dwItemID );
			if ( !pCHAR->ISHAVEITEM ( sid_item ) )	return FALSE;
		}
	}
	
	//	소지 스킬 검사.
	{
		LEARNSKILL_ITER iter = m_vecLearnSkill.begin ();
		LEARNSKILL_ITER iter_end = m_vecLearnSkill.end ();
		for ( ; iter!=iter_end; ++iter )
		{
			sNID.dwID = (*iter).dwNID;
			if ( !pCHAR->ISLEARNED_SKILL ( sNID, (*iter).wLevel ) )	return FALSE;
		}
	}

	//	이 퀘스트를 받기 위해서는 이미 수행했어야 하는 퀘스트.
	{
		GLQUESTPROG *pQUEST(NULL);

		PROCQUEST_ITER iter = m_vecCompleteQuest.begin();
		PROCQUEST_ITER iter_end = m_vecCompleteQuest.end();
		for ( ; iter!=iter_end; ++iter )
		{
			pQUEST = pCHAR->m_cQuestPlay.FindEnd ( (*iter) );
			if ( !pQUEST || !pQUEST->m_bCOMPLETE )	return FALSE;
		}
	}

	//	이 퀘스트를 받기 위해서는 습득하지 않아야할 스킬.
	{
		PROCSKILL_ITER iter = m_vecDisSkill.begin();
		PROCSKILL_ITER iter_end = m_vecDisSkill.end();
		for ( ; iter!=iter_end; ++iter )
		{
			BOOL bLEARNED = pCHAR->ISLEARNED_SKILL(SNATIVEID(*iter));
			if ( bLEARNED )		return FALSE;
		}
	}

	//	이 퀘스트를 받기 위해서는 수행하지 않아야할 퀘스트.
	{
		GLQUESTPROG *pQUEST(NULL);

		PROCQUEST_ITER iter = m_vecDisQuest.begin();
		PROCQUEST_ITER iter_end = m_vecDisQuest.end();
		for ( ; iter!=iter_end; ++iter )
		{
			pQUEST = pCHAR->m_cQuestPlay.FindProc ( (*iter) );
			if ( pQUEST )		return FALSE;
			
			pQUEST = pCHAR->m_cQuestPlay.FindEnd ( (*iter) );
			if ( pQUEST && pQUEST->m_bCOMPLETE )	return FALSE;
		}
	}

	/*npc talk buff condition, Juver, 2017/10/03 */
	{
		HAVEBUFF_ITER iter = m_vecBuff.begin ();
		HAVEBUFF_ITER iter_end = m_vecBuff.end ();
		for ( ; iter!=iter_end; ++iter )
		{
			sNID.dwID = (*iter).dwID;
			if ( !pCHAR->ISHAVE_BUFF ( sNID, (*iter).wLevel ) )	return FALSE;
		}
	}

	//	필요 소지 금액.
	if ( pCHAR->m_lnMoney < m_dwMONEY )		return FALSE;

	//	필요 파티 인원.
	DWORD dwPARTYNUM = GLPartyClient::GetInstance().GetMemberNum();
	if ( dwPARTYNUM < m_dwPARTYMEN_NUM )	return FALSE;

	return TRUE;
}

BOOL SNpcTalkCondition::EmptyCondition()
{
	if( m_signLevel != CONDITIONSIGN_NONE )
		return FALSE;
	if( m_wLevel )
		return FALSE;

	if( m_dwClass != GLCC_NONE )
		return FALSE;
	if( m_wSchool != MAX_SCHOOL )
		return FALSE;

	if( m_signElemental != CONDITIONSIGN_NONE )
		return FALSE;
	if( m_nElemental )
		return FALSE;

	if( m_signActionPoint != CONDITIONSIGN_NONE )
		return FALSE;
	if( m_nActionPoint )
		return FALSE;

	if( m_bTime )
		return FALSE;
	if( m_dwTimeBegin )
		return FALSE;
	if( m_dwTimeEnd )
		return FALSE;

	if( m_dwQUEST_NID != UINT_MAX )
		return FALSE;
	if( m_dwQUEST_STEP )
		return FALSE;

	if( !m_vecHaveItem.empty() )
		return FALSE;
	if( !m_vecLearnSkill.empty() )
		return FALSE;

	if( !m_vecCompleteQuest.empty() )
		return FALSE;

	if( !m_vecDisSkill.empty() )
		return FALSE;
	if( !m_vecDisQuest.empty() )
		return FALSE;

	/*npc talk buff condition, Juver, 2017/10/02 */
	if( !m_vecBuff.empty() )
		return FALSE;

	if( m_dwMONEY )
		return FALSE;
	if( m_dwPARTYMEN_NUM )
		return FALSE;

	return TRUE;
}

std::string SNpcTalkCondition::GetBoolString ( BOOL bVALUE )
{
	const static	char szTRUE[] = "TRUE";
	const static	char szFALSE[] = "FALSE";
	
	if ( bVALUE )	return szTRUE;
	else			return szFALSE;
}

void SNpcTalkCondition::GetConditionText( std::string& strName )
{
	std::strstream strCondition;

	strCondition << "Enable : " << GetBoolString(m_bEnable) << " " << "\r\n";

	if ( m_signLevel!=CONDITIONSIGN_NONE )
	{
		strCondition << "Level : " << m_wLevel << strCONDITIONSIGN[m_signLevel] << "Character Value" << "\r\n";
	}

	if ( m_dwClass!=GLCC_ALL_8CLASS )
	{
		strCondition << "Class : Brawler_M[" << GetBoolString(m_dwClass&GLCC_BRAWLER_M) << "], "					 
					 << "Brawler_F[" << GetBoolString(m_dwClass&GLCC_BRAWLER_W) << "], "
					 << "Swordsman_M[" << GetBoolString(m_dwClass&GLCC_SWORDSMAN_M) << "], "
					 << "Swordsman_W[" << GetBoolString(m_dwClass&GLCC_SWORDSMAN_W) << "], "
					 << "Archer_M[" << GetBoolString(m_dwClass&GLCC_ARCHER_M) << "], "
					 << "Archer_W[" << GetBoolString(m_dwClass&GLCC_ARCHER_W) << "], "
					 << "Shaman_M[" << GetBoolString(m_dwClass&GLCC_SHAMAN_M) << "], "
					 << "Shaman_W[" << GetBoolString(m_dwClass&GLCC_SHAMAN_W) << "], "
					 << "Extreme_M[" << GetBoolString(m_dwClass&GLCC_EXTREME_M) << "], "
					 << "Extreme_W[" << GetBoolString(m_dwClass&GLCC_EXTREME_W) << "], "
					 << "Gunner_M[" << GetBoolString(m_dwClass&GLCC_GUNNER_M) << "], "
					 << "Gunner_W[" << GetBoolString(m_dwClass&GLCC_GUNNER_W) << "], "
					 << "Assassin_M[" << GetBoolString(m_dwClass&GLCC_ASSASSIN_M) << "], "
					 << "Assassin_W[" << GetBoolString(m_dwClass&GLCC_ASSASSIN_W) << "], "
					 << "Tricker_M[" << GetBoolString(m_dwClass&GLCC_TRICKER_M) << "], "
					 << "Tricker_W[" << GetBoolString(m_dwClass&GLCC_TRICKER_W) << "]" << "\r\n";
	}

	if ( m_wSchool!=MAX_SCHOOL )
	{
		strCondition << "School : " << GLCONST_CHAR::strSCHOOLNAME[m_wSchool] << "\r\n";
	}

	if ( m_signElemental!=CONDITIONSIGN_NONE )
	{
		strCondition << "Elemental : " << m_nElemental << strCONDITIONSIGN[m_signElemental] << "Character Value" << "\r\n";
	}

	if ( m_signActionPoint!=CONDITIONSIGN_NONE )
	{
		strCondition << "Living Score : " << m_nActionPoint << strCONDITIONSIGN[m_signActionPoint] << "Character Value" << "\r\n";
	}

	if ( m_bTime )
	{
		strCondition << "Time Start: " << m_dwTimeBegin << "~" << "Time End: " << m_dwTimeEnd  << "\r\n";
	}

	GLQUEST *pQUEST = GLQuestMan::GetInstance().Find ( m_dwQUEST_NID );
	if ( pQUEST )
	{
		strCondition << "Progressing Quest : " << pQUEST->GetTITLE() << " Step : " << m_dwQUEST_STEP << "\r\n";
	}

	if ( !m_vecHaveItem.empty() )
	{
		strCondition << "Own's Item Condition" << "\r\n";

		HAVEITEM_ITER iter = m_vecHaveItem.begin();
		HAVEITEM_ITER iter_end = m_vecHaveItem.end();
		for ( ; iter!=iter_end; ++iter )
		{	
			SNPC_ITEM snp_item = (*iter);
			SNATIVEID sID( snp_item.dwItemID );

			CString strNAME = "NO ITEM";
			SITEM *pItem = GLItemMan::GetInstance().GetItem ( sID.wMainID, sID.wSubID );
			if ( pItem )	strNAME = pItem->GetName();

			strCondition << "[" << sID.wMainID << "/" << sID.wSubID << "] " << strNAME.GetString() << " Num." << snp_item.wItemNum << "\r\n";
  		}

		strCondition << "\r\n";
	}

	if ( !m_vecLearnSkill.empty() )
	{
		strCondition << "Learn Skill Condition" << "\r\n";

		SNpcTalkCondition::LEARNSKILL_ITER iter = m_vecLearnSkill.begin();
		SNpcTalkCondition::LEARNSKILL_ITER iter_end = m_vecLearnSkill.end();
		for ( ; iter!=iter_end; ++iter )
		{		
			SSkillCondition sSkillCondition = (*iter);
			SNATIVEID sID;
			sID.dwID = sSkillCondition.dwNID;

			CString strNAME = "NO SKILL";
			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sID.wMainID, sID.wSubID );
			if ( pSkill )	strNAME = pSkill->GetName();
			
			strCondition << "[" << sID.wMainID << "/" << sID.wSubID << "] "
				<< strNAME.GetString() << " Lev." << sSkillCondition.wLevel << "\r\n";
		}

		strCondition << "\r\n";
	}

	//	이미 수행한 퀘스트
	if ( !m_vecCompleteQuest.empty() )
	{
		strCondition << "Completion Quest" << "\r\n";

		PROCQUEST_ITER iter = m_vecCompleteQuest.begin();
		PROCQUEST_ITER iter_end = m_vecCompleteQuest.end();
		for ( ; iter!=iter_end; ++iter )
		{
			DWORD dwNID = (*iter);

			CString strNAME = "OFF SKILL";
			GLQUEST *pQuest = GLQuestMan::GetInstance().Find ( dwNID );
			if ( pQuest )	strNAME = pQuest->GetTITLE();

			strCondition << "[" << dwNID << "]"
				<< strNAME.GetString() << "\r\n";
		}

		strCondition << "\r\n";
	}

	//	이 퀘스트를 받기 위해서는 습득하지 않아야할 스킬.
	if ( !m_vecDisSkill.empty() )
	{
		strCondition << "Don't Gain Skill :" << "\r\n";

		PROCSKILL_ITER iter = m_vecDisSkill.begin();
		PROCSKILL_ITER iter_end = m_vecDisSkill.end();
		for ( ; iter!=iter_end; ++iter )
		{
			SNATIVEID sID = SNATIVEID(*iter);

			CString strNAME = "OFF SKILL";
			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sID.wMainID, sID.wSubID );
			if ( pSkill )	strNAME = pSkill->GetName();
			
			strCondition << "[" << pSkill->m_sBASIC.sNATIVEID.wMainID << "/" << pSkill->m_sBASIC.sNATIVEID.wSubID << "] "
				<< strNAME.GetString() << "\r\n";
		}

		strCondition << "\r\n";
	}

	//	이 퀘스트를 받기 위해서는 수행하지 않아야할 퀘스트.
	if ( !m_vecDisQuest.empty() )
	{
		strCondition << "Don't Progress Quest" << "\r\n";

		PROCQUEST_ITER iter = m_vecDisQuest.begin();
		PROCQUEST_ITER iter_end = m_vecDisQuest.end();
		for ( ; iter!=iter_end; ++iter )
		{
			DWORD dwNID = (*iter);

			CString strNAME = "OFF SKILL";
			GLQUEST *pQuest = GLQuestMan::GetInstance().Find ( dwNID );
			if ( pQuest )	strNAME = pQuest->GetTITLE();

			strCondition << "[" << dwNID << "]"
				<< strNAME.GetString() << "\r\n";
		}

		strCondition << "\r\n";
	}

	/*npc talk buff condition, Juver, 2017/10/02 */
	if ( !m_vecBuff.empty() )
	{
		strCondition << "Buff Condition" << "\r\n";

		SNpcTalkCondition::HAVEBUFF_ITER iter = m_vecBuff.begin();
		SNpcTalkCondition::HAVEBUFF_ITER iter_end = m_vecBuff.end();
		for ( ; iter!=iter_end; ++iter )
		{		
			SNPC_BUFF snpc_buff = (*iter);
			SNATIVEID sID;
			sID.dwID = snpc_buff.dwID;

			CString strNAME = "NO BUFF";
			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sID.wMainID, sID.wSubID );
			if ( pSkill )	strNAME = pSkill->GetName();

			strCondition << "[" << sID.wMainID << "/" << sID.wSubID << "] "
				<< strNAME.GetString() << " Level." << snpc_buff.wLevel << "\r\n";
		}

		strCondition << "\r\n";
	}



	//	필요 소지 금액.
	if ( m_dwMONEY>0 )
		strCondition << "Request Own's Money : " << m_dwMONEY << "\r\n";
	
	if ( m_dwPARTYMEN_NUM>0 )
		strCondition << "Request Party Members : " << m_dwPARTYMEN_NUM << "\r\n";

	strCondition << std::ends;

	strName = strCondition.str();
	strCondition.freeze( false );	// Note : std::strstream의 freeze. 안 하면 Leak 발생.
}