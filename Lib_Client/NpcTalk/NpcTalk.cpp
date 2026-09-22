#include "StdAfx.h"
#include "NpcTalk.h"
#include "NpcTalkCondition.h"
#include "GLQUEST.h"
#include "GLQUESTMAN.h"
#include "GLogicEx.h"
#include "SerialFile.h"
#include "GLItemMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::string	SNpcTalk::strACTIONTYPE[EM_TALK_SIZE] = 
{
	"Nothing",
	"Next Page",
	"Function",
	"Start Quest",
	"Process Quest",
};

std::string	SNpcTalk::szBASICDESC[EM_BASIC_SIZE] = 
{
	"Open Locker",
	"Shop",
	"Heal",
	"Save Position",
	"Reset Stats/Skill",
	"Trade Item",
	"Bus Station",
	"Create Club",
	"Upgrade Club",
	"CD Certify",
	"Change Tax Rate",
	"Use Club Locker",
	"Item Rebuild",
	"Simple Gamble",
	"EXP Recovery",
	"Random NPCTalk",
	"Item Search",
	"Attendance Function",
	"Item Mix",
	"Reserve00",
	"Reserve01",
	"Reserve02",
	"Reserve03",
	"Reserve04",
	"Reserve05",
	"Reserve06",
	"Reserve07",
	"Reserve08",
	"Reserve09",
	"NPCShop",
	"Item Exchange",
};

SNpcTalk::SNpcTalk () :
	m_dwNID(UINT_MAX),
	m_dwGLOB_ID(0),

	m_pCondition(NULL),
	m_nACTION(EM_DO_NOTHING),
	m_dwACTION_NO(UINT_MAX),
	
	m_dwACTION_PARAM1(UINT_MAX),
	m_dwACTION_PARAM2(UINT_MAX),
	m_dwRandomTime ( 0 )
{
	for( int i = 0; i < MAX_NEEDITEM_COUNT; ++i )
	{
		m_sNeedItem[i].Reset();
		m_sResultItem[i].Reset();
	}

	for ( int i = 0; i < MAX_QUEST_START; ++i )
	{
		m_dwQuestStartID[i] = UINT_MAX;
	}

	for ( int i = 0; i < MAX_RANDOM_PAGE; ++i )
	{
		m_dwRandomPageID[i] = UINT_MAX;
	}
}

SNpcTalk::~SNpcTalk ()
{
	ResetCondition ();
}

void SNpcTalk::SetGlobID ( DWORD dwID )
{
	m_dwGLOB_ID = dwID;
}

BOOL SNpcTalk::LOAD_0001 ( basestream& SFile )
{
	SFile >> m_dwNID;
	SFile >> m_strTalk;
	SFile >> m_nACTION;
	SFile >> m_dwACTION_NO;

	return TRUE;
}

BOOL SNpcTalk::LOAD_0002 ( basestream& SFile )
{
	SFile >> m_dwNID;
	SFile >> m_strTalk;
	SFile >> m_nACTION;
	SFile >> m_dwACTION_NO;
	SFile >> m_dwACTION_PARAM1;
	m_dwACTION_PARAM1 = UINT_MAX;

	if( m_nACTION == EM_BASIC && m_dwACTION_NO == EM_ITEM_TRADE )
	{
		SNPC_ITEM snpc_item;
		snpc_item.dwItemID = m_dwACTION_PARAM1;

		SNATIVEID sidItem(snpc_item.dwItemID);
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sidItem );
		if ( pItem )
			snpc_item.wItemNum = pItem->sDrugOp.wPileNum;
		else
			snpc_item.wItemNum = 1;

		m_sNeedItem[0] = snpc_item;
	}

	if ( m_nACTION == EM_QUEST_START )
		m_dwQuestStartID[0] = m_dwACTION_PARAM1;


	BOOL bExist(FALSE);
	SFile >> bExist;
	if ( bExist )
	{
		m_pCondition = new SNpcTalkCondition;
		m_pCondition->LOAD ( SFile );
	}

	return TRUE;
}

BOOL SNpcTalk::LOAD_0003 ( basestream& SFile )
{
	SFile >> m_dwNID;
	SFile >> m_strTalk;
	SFile >> m_nACTION;
	SFile >> m_dwACTION_NO;
	SFile >> m_dwACTION_PARAM1;
	SFile >> m_dwACTION_PARAM2;

	if( m_nACTION == EM_BASIC && m_dwACTION_NO == EM_ITEM_TRADE )
	{
		SNPC_ITEM snpc_item;
		snpc_item.dwItemID = m_dwACTION_PARAM1;

		SNATIVEID sidItem(snpc_item.dwItemID);
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sidItem );
		if ( pItem )
			snpc_item.wItemNum = pItem->sDrugOp.wPileNum;
		else
			snpc_item.wItemNum = 1;

		m_sNeedItem[0] = snpc_item;
	}

	if ( m_dwACTION_PARAM2 != UINT_MAX )
	{
		SNPC_ITEM snpc_item;
		snpc_item.dwItemID = m_dwACTION_PARAM2;
		snpc_item.wItemNum = 1;
		m_sResultItem[0] = snpc_item;
	}

	if ( m_nACTION == EM_QUEST_START )
		m_dwQuestStartID[0] = m_dwACTION_PARAM1;

	BOOL bExist(FALSE);
	SFile >> bExist;
	if ( bExist )
	{
		m_pCondition = new SNpcTalkCondition;
		m_pCondition->LOAD ( SFile );
	}

	return TRUE;
}

BOOL SNpcTalk::LOAD_0004 ( basestream& SFile )
{
	SFile >> m_dwNID;
	SFile >> m_strTalk;
	SFile >> m_nACTION;
	SFile >> m_dwACTION_NO;
	SFile >> m_dwACTION_PARAM1;
	SFile >> m_dwACTION_PARAM2;

	if( m_nACTION == EM_BASIC && m_dwACTION_NO == EM_ITEM_TRADE )
	{
		SNPC_ITEM snpc_item;
		snpc_item.dwItemID = m_dwACTION_PARAM1;

		SNATIVEID sidItem(snpc_item.dwItemID);
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sidItem );
		if ( pItem )
			snpc_item.wItemNum = pItem->sDrugOp.wPileNum;
		else
			snpc_item.wItemNum = 1;

		m_sNeedItem[0] = snpc_item;
	}

	if ( m_dwACTION_PARAM2 != UINT_MAX )
	{
		SNPC_ITEM snpc_item;
		snpc_item.dwItemID = m_dwACTION_PARAM2;
		snpc_item.wItemNum = 1;
		m_sResultItem[0] = snpc_item;
	}

	if ( m_nACTION == EM_QUEST_START )
		m_dwQuestStartID[0] = m_dwACTION_PARAM1;

	SFile >> m_strBusFile;

	BOOL bExist(FALSE);
	SFile >> bExist;
	if ( bExist )
	{
		m_pCondition = new SNpcTalkCondition;
		m_pCondition->LOAD ( SFile );
	}

	return TRUE;
}

BOOL SNpcTalk::LOAD_0005 ( basestream& SFile )
{
	SFile >> m_dwNID;
	SFile >> m_strTalk;
	SFile >> m_nACTION;
	SFile >> m_dwACTION_NO;

	if( m_nACTION == EM_BASIC && m_dwACTION_NO == EM_ITEM_TRADE )
	{
		for( BYTE i = 0; i < MAX_NEEDITEM_COUNT; ++i )
		{
			SNPC_ITEM snpc_item;
			SFile >> snpc_item.dwItemID;

			SNATIVEID sidItem(snpc_item.dwItemID);
			SITEM* pItem = GLItemMan::GetInstance().GetItem( sidItem );
			if ( pItem )
				snpc_item.wItemNum = pItem->sDrugOp.wPileNum;
			else
				snpc_item.wItemNum = 1;

			m_sNeedItem[i] = snpc_item;
		}
	}
	else
	{
		SFile >> m_dwACTION_PARAM1;

		if ( m_nACTION == EM_QUEST_START )
		{
			m_dwQuestStartID[0] = m_dwACTION_PARAM1;
		}
	}

	SFile >> m_dwACTION_PARAM2;

	if ( m_dwACTION_PARAM2 != UINT_MAX )
	{
		SNPC_ITEM snpc_item;
		snpc_item.dwItemID = m_dwACTION_PARAM2;
		snpc_item.wItemNum = 1;
		m_sResultItem[0] = snpc_item;
	}

	SFile >> m_strBusFile;

	BOOL bExist(FALSE);
	SFile >> bExist;
	if ( bExist )
	{
		m_pCondition = new SNpcTalkCondition;
		m_pCondition->LOAD ( SFile );
	}

	return TRUE;
}

BOOL SNpcTalk::LOAD_0006 ( basestream& SFile )
{
	SFile >> m_dwNID;
	SFile >> m_strTalk;

	SFile >> m_nACTION;
	SFile >> m_dwACTION_NO;

	for ( int i = 0; i < MAX_NEEDITEM_COUNT; ++i )
	{
		SNPC_ITEM snpc_item;
		SFile >> snpc_item.dwItemID;

		SNATIVEID sidItem(snpc_item.dwItemID);
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sidItem );
		if ( pItem )
			snpc_item.wItemNum = pItem->sDrugOp.wPileNum;
		else
			snpc_item.wItemNum = 1;

		m_sNeedItem[i] = snpc_item;
	}

	for ( int i = 0; i < MAX_QUEST_START; ++i )
	{
		SFile >> m_dwQuestStartID[i];
	}
	
	SFile >> m_dwACTION_PARAM1;
	SFile >> m_dwACTION_PARAM2;

	if ( m_dwACTION_PARAM2 != UINT_MAX )
	{
		SNPC_ITEM snpc_item;
		snpc_item.dwItemID = m_dwACTION_PARAM2;
		snpc_item.wItemNum = 1;
		m_sResultItem[0] = snpc_item;
	}

	SFile >> m_strBusFile;

	BOOL bExist(FALSE);
	SFile >> bExist;
	if ( bExist )
	{
		m_pCondition = new SNpcTalkCondition;
		m_pCondition->LOAD ( SFile );
	}

	return TRUE;
}

BOOL SNpcTalk::LOAD_0007 ( basestream& SFile )
{
	SFile >> m_dwNID;
	SFile >> m_strTalk;

	SFile >> m_nACTION;
	SFile >> m_dwACTION_NO;

	for ( int i = 0; i < MAX_NEEDITEM_COUNT; ++i )
	{
		SNPC_ITEM snpc_item;
		SFile >> snpc_item.dwItemID;

		SNATIVEID sidItem(snpc_item.dwItemID);
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sidItem );
		if ( pItem )
			snpc_item.wItemNum = pItem->sDrugOp.wPileNum;
		else
			snpc_item.wItemNum = 1;

		m_sNeedItem[i] = snpc_item;
	}

	for ( int i = 0; i < MAX_QUEST_START; ++i )
	{
		SFile >> m_dwQuestStartID[i];
	}

	for ( int i = 0; i < MAX_RANDOM_PAGE; ++i )
	{
		SFile >> m_dwRandomPageID[i];
	}

	SFile >> m_dwRandomTime;
	
	SFile >> m_dwACTION_PARAM1;
	SFile >> m_dwACTION_PARAM2;

	if ( m_dwACTION_PARAM2 != UINT_MAX )
	{
		SNPC_ITEM snpc_item;
		snpc_item.dwItemID = m_dwACTION_PARAM2;
		snpc_item.wItemNum = 1;
		m_sResultItem[0] = snpc_item;
	}

	SFile >> m_strBusFile;

	BOOL bExist(FALSE);
	SFile >> bExist;
	if ( bExist )
	{
		m_pCondition = new SNpcTalkCondition;
		m_pCondition->LOAD ( SFile );
	}

	return TRUE;
}

BOOL SNpcTalk::LOAD_0008 ( basestream& SFile )
{
	SFile >> m_dwNID;
	SFile >> m_strTalk;
	SFile >> m_nACTION;
	SFile >> m_dwACTION_NO;
	SFile >> m_dwACTION_PARAM1;
	SFile >> m_dwACTION_PARAM2;

	for ( int i = 0; i < MAX_NEEDITEM_COUNT; ++i )
	{
		SNPC_ITEM snpc_item;
		SFile >> snpc_item.dwItemID;

		SNATIVEID sidItem(snpc_item.dwItemID);
		SITEM* pItem = GLItemMan::GetInstance().GetItem( sidItem );
		if ( pItem )
			snpc_item.wItemNum = pItem->sDrugOp.wPileNum;
		else
			snpc_item.wItemNum = 1;

		m_sNeedItem[i] = snpc_item;
	}

	if ( m_dwACTION_PARAM2 != UINT_MAX )
	{
		SNPC_ITEM snpc_item;
		snpc_item.dwItemID = m_dwACTION_PARAM2;
		snpc_item.wItemNum = 1;
		m_sResultItem[0] = snpc_item;
	}

	for ( int i = 0; i < MAX_QUEST_START; ++i )
	{
		SFile >> m_dwQuestStartID[i];
	}

	for ( int i = 0; i < MAX_RANDOM_PAGE; ++i )
	{
		SFile >> m_dwRandomPageID[i];
	}

	SFile >> m_dwRandomTime;
	SFile >> m_strBusFile;

	BOOL bExist(FALSE);
	SFile >> bExist;
	if ( bExist )
	{
		m_pCondition = new SNpcTalkCondition;
		m_pCondition->LOAD ( SFile );
	}

	return TRUE;
}

BOOL SNpcTalk::LOAD_0009 ( basestream& SFile )
{
	SFile >> m_dwNID;
	SFile >> m_strTalk;
	SFile >> m_nACTION;
	SFile >> m_dwACTION_NO;
	SFile >> m_dwACTION_PARAM1;
	SFile >> m_dwACTION_PARAM2;

	DWORD dwnum(0);
	SFile >> dwnum;
	for ( DWORD i = 0; i < dwnum; ++i )
	{
		SNPC_ITEM snpc_item;
		SFile.ReadBuffer ( &snpc_item, sizeof(snpc_item) );

		if ( i >= MAX_NEEDITEM_COUNT )
			continue;

		m_sNeedItem[i] = snpc_item;
	}

	SFile >> dwnum;
	for ( DWORD i = 0; i < dwnum; ++i )
	{
		SNPC_ITEM snpc_item;
		SFile.ReadBuffer ( &snpc_item, sizeof(snpc_item) );

		if ( i >= MAX_NEEDITEM_COUNT )
			continue;

		m_sResultItem[i] = snpc_item;
	}

	for ( int i = 0; i < MAX_QUEST_START; ++i )
	{
		SFile >> m_dwQuestStartID[i];
	}

	for ( int i = 0; i < MAX_RANDOM_PAGE; ++i )
	{
		SFile >> m_dwRandomPageID[i];
	}

	SFile >> m_dwRandomTime;
	SFile >> m_strBusFile;

	BOOL bExist(FALSE);
	SFile >> bExist;
	if ( bExist )
	{
		m_pCondition = new SNpcTalkCondition;
		m_pCondition->LOAD ( SFile );
	}

	return TRUE;
}

/*dmk14 ver10 npc loader*/
BOOL SNpcTalk::LOAD_0010 ( basestream& SFile )
{
	SFile >> m_dwNID;
	SFile >> m_strTalk;
	SFile >> m_nACTION;
	SFile >> m_dwACTION_NO;
	SFile >> m_dwACTION_PARAM1;
	SFile >> m_dwACTION_PARAM2;

	DWORD dwnum(0);
	SFile >> dwnum;
	for ( DWORD i = 0; i < dwnum; ++i )
	{
		SNPC_ITEM snpc_item;
		SFile.ReadBuffer ( &snpc_item, sizeof(snpc_item) );

		if ( i >= MAX_NEEDITEM_COUNT )
			continue;

		m_sNeedItem[i] = snpc_item;
	}

	SFile >> dwnum;
	for ( DWORD i = 0; i < dwnum; ++i )
	{
		SNPC_ITEM snpc_item;
		SFile.ReadBuffer ( &snpc_item, sizeof(snpc_item) );

		if ( i >= MAX_NEEDITEM_COUNT )
			continue;

		m_sResultItem[i] = snpc_item;
	}

	for ( int i = 0; i < MAX_QUEST_START; ++i )
	{
		SFile >> m_dwQuestStartID[i];
	}

	for ( int i = 0; i < MAX_RANDOM_PAGE; ++i )
	{
		SFile >> m_dwRandomPageID[i];
	}

	SFile >> m_dwRandomTime;
	SFile >> m_strBusFile;

	SFile >> m_sHireSummon.dwID;

	BOOL bExist(FALSE);
	SFile >> bExist;
	if ( bExist )
	{
		m_pCondition = new SNpcTalkCondition;
		m_pCondition->LOAD ( SFile );
	}

	return TRUE;
}

BOOL SNpcTalk::LOAD_0200 ( basestream& SFile )
{
	SFile >> m_dwNID;
	SFile >> m_strTalk;
	SFile >> m_nACTION;
	SFile >> m_dwACTION_NO;
	SFile >> m_dwACTION_PARAM1;
	SFile >> m_dwACTION_PARAM2;

	for ( int i = 0; i < MAX_NEEDITEM_COUNT; ++i )
	{
		SNPC_ITEM snpc_item;
		SFile.ReadBuffer ( &snpc_item, sizeof(snpc_item) );
		m_sNeedItem[i] = snpc_item;
	}

	for ( int i = 0; i < MAX_NEEDITEM_COUNT; ++i )
	{
		SNPC_ITEM snpc_item;
		SFile.ReadBuffer ( &snpc_item, sizeof(snpc_item) );
		m_sResultItem[i] = snpc_item;
	}

	for ( int i = 0; i < MAX_QUEST_START; ++i )
	{
		SFile >> m_dwQuestStartID[i];
	}

	for ( int i = 0; i < MAX_RANDOM_PAGE; ++i )
	{
		SFile >> m_dwRandomPageID[i];
	}

	SFile >> m_dwRandomTime;
	SFile >> m_strBusFile;

	BOOL bExist(FALSE);
	SFile >> bExist;
	if ( bExist )
	{
		m_pCondition = new SNpcTalkCondition;
		m_pCondition->LOAD ( SFile );
	}

	return TRUE;
}

BOOL SNpcTalk::SAVE ( CSerialFile& SFile )
{	
	if ( !SFile.IsOpen () )
	{
		GASSERT ( 0 && "파일이 열려있지 않습니다." );
		return FALSE;
	}

	SFile << (WORD)VERSION;

	SFile << m_dwNID;
	SFile << m_strTalk;
	SFile << m_nACTION;
	SFile << m_dwACTION_NO;
	SFile << m_dwACTION_PARAM1;
	SFile << m_dwACTION_PARAM2;

	for( BYTE i = 0; i < MAX_NEEDITEM_COUNT; ++i )
	{
		const SNPC_ITEM snpc_item = m_sNeedItem[i];
		SFile.WriteBuffer ( &snpc_item, sizeof ( SNPC_ITEM ) );
	}

	for( BYTE i = 0; i < MAX_NEEDITEM_COUNT; ++i )
	{
		const SNPC_ITEM snpc_item = m_sResultItem[i];
		SFile.WriteBuffer ( &snpc_item, sizeof ( SNPC_ITEM ) );
	}

	for ( BYTE i = 0; i < MAX_QUEST_START; ++i ) 
	{
		SFile << m_dwQuestStartID[i];
	}

	for ( BYTE i = 0; i < MAX_RANDOM_PAGE; ++i ) 
	{
		SFile << m_dwRandomPageID[i];
	}

	SFile << m_dwRandomTime;
	SFile << m_strBusFile;

	SFile << BOOL(m_pCondition!=NULL);
	if ( m_pCondition )		m_pCondition->SAVE ( SFile );

	return TRUE;
}


BOOL SNpcTalk::LOAD ( basestream& SFile )
{
	if ( !SFile.IsOpen () )
	{
		GASSERT ( 0 && "파일이 열려있지 않습니다." );
		return FALSE;
	}

	WORD wVer = 0;
	SFile >> wVer;

	switch ( wVer )
	{
	case 0x0001:	LOAD_0001 ( SFile );	break;
	case 0x0002:	LOAD_0002 ( SFile );	break;
	case 0x0003:	LOAD_0003 ( SFile );	break;
	case 0x0004:	LOAD_0004 ( SFile );	break;
	case 0x0005:	LOAD_0005 ( SFile );	break;
	case 0x0006:	LOAD_0006 ( SFile );	break;
	case 0x0007:	LOAD_0007 ( SFile );	break;
	case 0x0008:	LOAD_0008 ( SFile );	break;
	case 0x0009:	LOAD_0009 ( SFile );	break;
		/*dmk14 ver10 npc loader*/
	case 0x000A:	LOAD_0010 ( SFile );	break;
	case SNpcTalk::VERSION:	LOAD_0200 ( SFile );	break;

	default:
		CDebugSet::ErrorVersion( "SNpcTalk::LOAD", wVer );
		break;
	};


	//jdevtodo
	//fix later when the new logic for item result is implemented
	if( m_nACTION == EM_BASIC && m_dwACTION_NO == EM_ITEM_TRADE )
	{
		if ( m_dwACTION_PARAM2 == UINT_MAX  && m_sResultItem[0].dwItemID != UINT_MAX )
		{
			m_dwACTION_PARAM2 = m_sResultItem[0].dwItemID;

			CDebugSet::ToFile( "npctalklog.txt", "[id:%04d][gid:%04d] param2 == null param2 item corrected:%03d~%03d talk %s ", m_dwNID, m_dwGLOB_ID, 
				SNATIVEID(m_dwACTION_PARAM2).wMainID, SNATIVEID(m_dwACTION_PARAM2).wSubID, SFile.GetFileName() );
		}

		if ( m_dwACTION_PARAM2 != UINT_MAX && m_sResultItem[0].dwItemID != UINT_MAX &&
			m_dwACTION_PARAM2 != m_sResultItem[0].dwItemID  )
		{
			DWORD dwORIG = m_dwACTION_PARAM2;
			m_dwACTION_PARAM2 = m_sResultItem[0].dwItemID;

			CDebugSet::ToFile( "npctalklog.txt", "[id:%04d][gid:%04d] param2 != resultitem[0] param2 item corrected:%03d~%03d orig:%03d~%03d  talk %s ", m_dwNID, m_dwGLOB_ID, 
				SNATIVEID(m_dwACTION_PARAM2).wMainID, SNATIVEID(m_dwACTION_PARAM2).wSubID, 
				SNATIVEID(dwORIG).wMainID, SNATIVEID(dwORIG).wSubID, 
				SFile.GetFileName() );
		}


		WORD wItem = 0;
		for( int i=0; i<MAX_NEEDITEM_COUNT; ++i ){
			if ( m_sResultItem[i].dwItemID != UINT_MAX )
				wItem++;
		}

		if ( wItem > 1 )
		{
			CDebugSet::ToFile( "npctalklog.txt", "[id:%04d][gid:%04d] multiple result item:%d talk %s ", m_dwNID, m_dwGLOB_ID, 
				wItem, SFile.GetFileName() );
		}
	}

	return TRUE;
}

SNpcTalk&	SNpcTalk::operator= ( const SNpcTalk& rNpcTalk )
{
	m_dwNID = rNpcTalk.m_dwNID;
	m_strTalk = rNpcTalk.m_strTalk;

    m_nACTION = rNpcTalk.m_nACTION;
	m_dwACTION_NO = rNpcTalk.m_dwACTION_NO;
	
	for ( int i = 0 ; i < MAX_NEEDITEM_COUNT; ++i )
		m_sNeedItem[i]  = rNpcTalk.m_sNeedItem[i];

	for ( int i = 0 ; i < MAX_NEEDITEM_COUNT; ++i )
		m_sResultItem[i]  = rNpcTalk.m_sResultItem[i];

	for ( int i = 0 ; i < MAX_QUEST_START; ++i )
		m_dwQuestStartID[i]  = rNpcTalk.m_dwQuestStartID[i];

	for ( int i = 0 ; i < MAX_RANDOM_PAGE; ++i )
		m_dwRandomPageID[i]  = rNpcTalk.m_dwRandomPageID[i];

	m_dwRandomTime = rNpcTalk.m_dwRandomTime;

	m_dwACTION_PARAM1 = rNpcTalk.m_dwACTION_PARAM1;
	m_dwACTION_PARAM2 = rNpcTalk.m_dwACTION_PARAM2;

	m_strBusFile = rNpcTalk.m_strBusFile;

	ResetCondition ();
	if ( rNpcTalk.m_pCondition )
	{
		SetCondition ( *rNpcTalk.m_pCondition );
	}

	return *this;
}

BOOL SNpcTalk::SetCondition ( const SNpcTalkCondition &sCondition )
{
	ResetCondition ();

	m_pCondition = new SNpcTalkCondition;
	*m_pCondition = sCondition;

	return TRUE;
}

void SNpcTalk::ResetCondition ()
{
	SAFE_DELETE ( m_pCondition );
}
void SNpcTalk::SetTalk( const std::string &strTalk )
{
	m_strTalk = strTalk;
}

SNpcTalkCondition*	SNpcTalk::GetCondition () const
{	
	return m_pCondition;
}

void SNpcTalk::Init ()
{
	m_dwNID = UINT_MAX;
	m_strTalk = "";

    m_nACTION = EM_DO_NOTHING;
	m_dwACTION_NO = UINT_MAX;
	m_dwACTION_PARAM1 = UINT_MAX;
	m_dwACTION_PARAM2 = UINT_MAX;

	m_strBusFile = "";

	for ( int i = 0; i < MAX_NEEDITEM_COUNT; ++i )
		m_sNeedItem[i].Reset();

	for ( int i = 0; i < MAX_NEEDITEM_COUNT; ++i )
		m_sResultItem[i].Reset();

	for ( int i = 0; i < MAX_QUEST_START; ++i )
		m_dwQuestStartID[i] = UINT_MAX;

	for ( int i = 0; i < MAX_RANDOM_PAGE; ++i )
		m_dwRandomPageID[i] = UINT_MAX;

	m_dwRandomTime = 0;

}

BOOL SNpcTalk::DoTEST ( GLCHARLOGIC *pCHARLOGIC, DWORD dwIndex )
{
	if ( !pCHARLOGIC )		return FALSE;

	if ( m_pCondition )
	{
		BOOL bCONDITION = m_pCondition->DoTEST ( pCHARLOGIC );
		if ( !bCONDITION )	return FALSE;
	}

	BOOL bOK = FALSE;

	switch ( m_nACTION )
	{
	case EM_QUEST_START:
		{
			if ( dwIndex == UINT_MAX ) 
			{
				for ( int i = 0; i < MAX_QUEST_START; ++i )
				{
					if ( m_dwQuestStartID[i] == UINT_MAX ) continue;

					//	이미 진행중인 퀘스트는 새로이 시작할 수 없다.
					GLQUESTPROG *pQUEST_PROG = pCHARLOGIC->m_cQuestPlay.FindProc(m_dwQuestStartID[i]);
					if ( pQUEST_PROG )	continue;

					GLQUESTPROG *pQUEST_END = pCHARLOGIC->m_cQuestPlay.FindEnd(m_dwQuestStartID[i]);
					if ( pQUEST_END )
					{
						GLQUEST *pQUEST = GLQuestMan::GetInstance().Find ( m_dwQuestStartID[i] );
						if ( !pQUEST )	continue;

						if ( pQUEST_END->m_bCOMPLETE )
						{
							//	이미 완료한 퀘스트의 경우에는 반복 수행이 가능할때만 가능.
							if ( !pQUEST->IsREPEAT() )	continue;

							/*dmk14 daily quest*/
							if ( pQUEST->IsDaily() )
							{
								CTime cTime = CTime::GetCurrentTime();
								CTime cTimeEnd(pQUEST_END->m_tEndTime);

								if (cTime.GetDay() == cTimeEnd.GetDay() &&
									cTime.GetMonth() == cTimeEnd.GetMonth() &&
									cTime.GetYear() == cTimeEnd.GetYear() )
									continue;
							}
						}
						else
						{
							//	이미 포기한 퀘스트의 경우에는제시도 수행이 가능할때만 가능.
							if ( !pQUEST->IsAGAIN() )	continue;
						}
					}

					bOK = TRUE;
				}
			}
			else
			{
				if ( m_dwQuestStartID[dwIndex] == UINT_MAX ) return FALSE;

				//	이미 진행중인 퀘스트는 새로이 시작할 수 없다.
				GLQUESTPROG *pQUEST_PROG = pCHARLOGIC->m_cQuestPlay.FindProc(m_dwQuestStartID[dwIndex]);
				if ( pQUEST_PROG )	return FALSE;

				GLQUESTPROG *pQUEST_END = pCHARLOGIC->m_cQuestPlay.FindEnd(m_dwQuestStartID[dwIndex]);
				if ( pQUEST_END )
				{
					GLQUEST *pQUEST = GLQuestMan::GetInstance().Find ( m_dwQuestStartID[dwIndex] );
					if ( !pQUEST )	return FALSE;

					if ( pQUEST_END->m_bCOMPLETE )
					{
						//	이미 완료한 퀘스트의 경우에는 반복 수행이 가능할때만 가능.
						if ( !pQUEST->IsREPEAT() )	return FALSE;

						/*dmk14 daily quest*/
						if ( pQUEST->IsDaily() )
						{
							CTime cTime = CTime::GetCurrentTime();
							CTime cTimeEnd(pQUEST_END->m_tEndTime);

							if (cTime.GetDay() == cTimeEnd.GetDay() &&
								cTime.GetMonth() == cTimeEnd.GetMonth() &&
								cTime.GetYear() == cTimeEnd.GetYear() )
								return FALSE;
						}
					}
					else
					{
						//	이미 포기한 퀘스트의 경우에는제시도 수행이 가능할때만 가능.
						if ( !pQUEST->IsAGAIN() )	return FALSE;
					}
				}

				bOK = TRUE;

			}
		}
		break;

	case EM_QUEST_STEP:
		{
			//	진행중인 퀘스트일때만.
			GLQUESTPROG *pQUEST_PROG = pCHARLOGIC->m_cQuestPlay.FindProc(m_dwACTION_PARAM1);
			if ( !pQUEST_PROG )	return FALSE;
			
			if ( pQUEST_PROG->m_dwSTEP!=m_dwACTION_PARAM2 )		return FALSE;

			bOK = TRUE;
		}
		break;
	default:
		{
			bOK = TRUE;
		}
		break;
	};
	
	return bOK;
}