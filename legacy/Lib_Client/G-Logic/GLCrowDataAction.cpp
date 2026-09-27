#include "stdafx.h"
#include "GLCrowDataAction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SCROWACTION& SCROWACTION::operator= ( const SCROWACTION_100 &OldData )
{
	m_wBodyRadius			= OldData.m_wBodyRadius;
	m_strSkinObj			= OldData.m_szSkinObj;
	m_strTalkFile			= OldData.m_szTalkFile;

	m_strSaleFile[0]		= OldData.m_szSaleFile;

	m_bAfterFall_NoBody		= OldData.m_bAfterFall_NoBody;

	m_strFallingEffect		= OldData.m_szFallingEffect;
	m_strBlowEffect			= OldData.m_szBlowEffect;

	m_emMoveType			= OldData.m_emMoveType;
	m_fDriftHeight			= OldData.m_fDriftHeight;

	m_fWalkVelo				= OldData.m_fWalkVelo;
	m_bRun					= OldData.m_bRun;
	m_fRunVelo				= OldData.m_fRunVelo;

	m_bMobLink			= FALSE;
	m_sMobLinkID		= NATIVEID_NULL();

	SCROWPATTERN sCrowTemp;
	sCrowTemp.m_emActPattern = EMCROWACT_UP_IDLE;
	sCrowTemp.m_dwPatternAttackSet = ACTION_MAXATTACK;
	sCrowTemp.m_fPatternDNRate = 90;
	m_vecPatternList.clear();
	m_vecPatternList.push_back( sCrowTemp );

	return *this;
}

SCROWACTION& SCROWACTION::operator= ( const SCROWACTION_101 &OldData )
{
	m_wBodyRadius			= OldData.m_wBodyRadius;
	m_strSkinObj			= OldData.m_szSkinObj;
	m_strTalkFile			= OldData.m_szTalkFile;

	m_dwActFlag				= OldData.m_dwActFlag;
	m_strSaleFile[0]		= OldData.m_szSaleFile;
	//m_szSaleFile2;
	//m_szSaleFile3;

	m_bAfterFall_NoBody		= OldData.m_bAfterFall_NoBody;
	m_strBirthEffect		= OldData.m_szBirthEffect;
	m_strFallingEffect		= OldData.m_szFallingEffect;
	m_strBlowEffect			= OldData.m_szBlowEffect;

	m_emMoveType			= OldData.m_emMoveType;
	m_fDriftHeight			= OldData.m_fDriftHeight;

	m_fWalkVelo				= OldData.m_fWalkVelo;
	m_bRun					= OldData.m_bRun;
	m_fRunVelo				= OldData.m_fRunVelo;

	m_bMobLink			= FALSE;
	m_sMobLinkID		= NATIVEID_NULL();

	SCROWPATTERN sCrowTemp;
	sCrowTemp.m_emActPattern = EMCROWACT_UP_IDLE;
	sCrowTemp.m_dwPatternAttackSet = ACTION_MAXATTACK;
	sCrowTemp.m_fPatternDNRate = 90;
	m_vecPatternList.clear();
	m_vecPatternList.push_back( sCrowTemp );

	return *this;
}

SCROWACTION& SCROWACTION::operator= ( const SCROWACTION_102 &OldData )
{
	m_wBodyRadius		 = OldData.m_wBodyRadius;
	m_strSkinObj		 = OldData.m_szSkinObj;
	m_strTalkFile		 = OldData.m_szTalkFile;

	m_dwActFlag			 = OldData.m_dwActFlag;

	for ( int i=0; i<SALENUM; ++i )
		m_strSaleFile[i] = OldData.m_szSaleFile[i];

	m_bAfterFall_NoBody  = OldData.m_bAfterFall_NoBody;
	m_strBirthEffect	 = OldData.m_szBirthEffect;
	m_strFallingEffect   = OldData.m_szFallingEffect;
	m_strBlowEffect		 = OldData.m_szBlowEffect;

	m_emMoveType		 = OldData.m_emMoveType;
	m_fDriftHeight		 = OldData.m_fDriftHeight;

	m_fWalkVelo			 = OldData.m_fWalkVelo;
	m_bRun				 = OldData.m_bRun;
	m_fRunVelo			 = OldData.m_fRunVelo;

	m_bMobLink			= FALSE;
	m_sMobLinkID		= NATIVEID_NULL();

	SCROWPATTERN sCrowTemp;
	sCrowTemp.m_emActPattern = EMCROWACT_UP_IDLE;
	sCrowTemp.m_dwPatternAttackSet = ACTION_MAXATTACK;
	sCrowTemp.m_fPatternDNRate = 90;
	m_vecPatternList.clear();
	m_vecPatternList.push_back( sCrowTemp );

	return *this;
}

SCROWACTION& SCROWACTION::operator= ( const SCROWACTION &value )
{
	m_wBodyRadius		 = value.m_wBodyRadius;
	m_strSkinObj		 = value.m_strSkinObj;
	m_strTalkFile		 = value.m_strTalkFile;

	m_dwActFlag = value.m_dwActFlag;

	for ( int i=0; i<SALENUM; ++i )
		m_strSaleFile[i] = value.m_strSaleFile[i];

	m_strShopFile		= value.m_strShopFile; /*npc shop, Juver, 2017/07/25 */

	m_bAfterFall_NoBody = value.m_bAfterFall_NoBody;
	m_strBirthEffect    = value.m_strBirthEffect;
	m_strFallingEffect  = value.m_strFallingEffect;
	m_strBlowEffect		= value.m_strBlowEffect;

	m_emMoveType		= value.m_emMoveType;
	m_fDriftHeight		= value.m_fDriftHeight;

	m_fWalkVelo			= value.m_fWalkVelo;
	m_bRun				= value.m_bRun;
	m_fRunVelo			= value.m_fRunVelo;
	m_fLiveTime			= value.m_fLiveTime;

	m_bMobLink			= value.m_bMobLink;
	m_sMobLinkID		= value.m_sMobLinkID;
	m_fMobLinkScale		= value.m_fMobLinkScale;
	m_fMobLinkDelay		= value.m_fMobLinkDelay;

	m_vecPatternList	= value.m_vecPatternList;

	m_strItemExchangeFile = value.m_strItemExchangeFile; /*item exchange, Juver, 2017/10/11 */

	return *this;
}

bool SCROWACTION::LOAD103 ( basestream &SFile )
{
	DWORD dwDATA(0);

	SFile >> m_wBodyRadius;
	SFile >> m_strSkinObj;
	SFile >> m_strTalkFile;

	SFile >> m_dwActFlag;

	for ( int i=0; i<SALENUM; ++i )
		SFile >> m_strSaleFile[i];

	SFile >> m_bAfterFall_NoBody;
	SFile >> m_strBirthEffect;
	SFile >> m_strFallingEffect;
	SFile >> m_strBlowEffect;

	SFile >> dwDATA;
	SCROWPATTERN sCrowTemp;
	sCrowTemp.m_emActPattern = ( EMCROWACT_UP ) dwDATA;
	sCrowTemp.m_dwPatternAttackSet = ACTION_MAXATTACK;
	sCrowTemp.m_fPatternDNRate = 90;
	m_vecPatternList.push_back( sCrowTemp );

	SFile >> dwDATA; //m_emActionDN = ( EMCROWACT_DN ) dwDATA;


	SFile >> dwDATA; m_emMoveType = (EMMOVETYPE) dwDATA;
	SFile >> m_fDriftHeight;

	SFile >> m_fWalkVelo;
	SFile >> m_bRun;
	SFile >> m_fRunVelo;

	return true;
}

bool SCROWACTION::LOAD104 ( basestream &SFile )
{
	DWORD dwDATA(0);

	SFile >> m_wBodyRadius;
	SFile >> m_strSkinObj;
	SFile >> m_strTalkFile;

	SFile >> m_dwActFlag;

	for ( int i = 0; i < SALENUM; ++i )
		SFile >> m_strSaleFile[i];

	SFile >> m_bAfterFall_NoBody;
	SFile >> m_strBirthEffect;
	SFile >> m_strFallingEffect;
	SFile >> m_strBlowEffect;

	SFile >> dwDATA;
	SCROWPATTERN sCrowTemp;
	sCrowTemp.m_emActPattern = ( EMCROWACT_UP ) dwDATA;
	sCrowTemp.m_dwPatternAttackSet = ACTION_MAXATTACK;
	sCrowTemp.m_fPatternDNRate = 90;
	m_vecPatternList.push_back( sCrowTemp );

	SFile >> dwDATA; //m_emActionDN = ( EMCROWACT_DN ) dwDATA;


	SFile >> dwDATA; m_emMoveType = ( EMMOVETYPE ) dwDATA;
	SFile >> m_fDriftHeight;

	SFile >> m_fWalkVelo;
	SFile >> m_bRun;
	SFile >> m_fRunVelo;
	SFile >> m_fLiveTime;

	return true;
}

bool SCROWACTION::LOAD105 ( basestream &SFile )
{
	DWORD dwDATA(0);
	float fActionDNRate(0);

	SFile >> m_wBodyRadius;
	SFile >> m_strSkinObj;
	SFile >> m_strTalkFile;

	SFile >> m_dwActFlag;

	for ( int i = 0; i < SALENUM; ++i )
		SFile >> m_strSaleFile[i];

	SFile >> m_bAfterFall_NoBody;
	SFile >> m_strBirthEffect;
	SFile >> m_strFallingEffect;
	SFile >> m_strBlowEffect;

	SFile >> dwDATA;
	SCROWPATTERN sCrowTemp;
	sCrowTemp.m_emActPattern = ( EMCROWACT_UP ) dwDATA;
	sCrowTemp.m_dwPatternAttackSet = ACTION_MAXATTACK;
	sCrowTemp.m_fPatternDNRate = 90;
	m_vecPatternList.push_back( sCrowTemp );

	SFile >> dwDATA; //m_emActionDN = ( EMCROWACT_DN ) dwDATA;


	SFile >> dwDATA; m_emMoveType = ( EMMOVETYPE ) dwDATA;
	SFile >> m_fDriftHeight;

	SFile >> m_fWalkVelo;
	SFile >> m_bRun;
	SFile >> m_fRunVelo;
	SFile >> m_fLiveTime;

	SFile >> fActionDNRate;

	return true;
}

bool SCROWACTION::LOAD106 ( basestream &SFile )
{
	DWORD dwDATA(0);
	float fActionDNRate(0);

	SFile >> m_wBodyRadius;
	SFile >> m_strSkinObj;
	SFile >> m_strTalkFile;

	SFile >> m_dwActFlag;

	for ( int i = 0; i < SALENUM; ++i )
		SFile >> m_strSaleFile[i];

	SFile >> m_bAfterFall_NoBody;
	SFile >> m_strBirthEffect;
	SFile >> m_strFallingEffect;
	SFile >> m_strBlowEffect;

	SFile >> dwDATA;
	SCROWPATTERN sCrowTemp;
	sCrowTemp.m_emActPattern = ( EMCROWACT_UP ) dwDATA;
	sCrowTemp.m_dwPatternAttackSet = ACTION_MAXATTACK;
	sCrowTemp.m_fPatternDNRate = 90;
	m_vecPatternList.push_back( sCrowTemp );

	SFile >> dwDATA; //m_emActionDN = ( EMCROWACT_DN ) dwDATA;

	SFile >> dwDATA; m_emMoveType = ( EMMOVETYPE ) dwDATA;
	SFile >> m_fDriftHeight;

	SFile >> m_fWalkVelo;
	SFile >> m_bRun;
	SFile >> m_fRunVelo;
	SFile >> m_fLiveTime;

	SFile >> fActionDNRate;

	SFile >> m_bMobLink;
	SFile >> m_sMobLinkID.dwID;

	return true;
}

bool SCROWACTION::LOAD107 ( basestream &SFile )
{
	DWORD dwDATA(0);
	float fActionDNRate(0);

	SFile >> m_wBodyRadius;
	SFile >> m_strSkinObj;
	SFile >> m_strTalkFile;

	SFile >> m_dwActFlag;

	for ( int i = 0; i < SALENUM; ++i )
		SFile >> m_strSaleFile[i];

	SFile >> m_bAfterFall_NoBody;
	SFile >> m_strBirthEffect;
	SFile >> m_strFallingEffect;
	SFile >> m_strBlowEffect;

	SFile >> dwDATA; m_emMoveType = ( EMMOVETYPE ) dwDATA;
	SFile >> m_fDriftHeight;

	SFile >> m_fWalkVelo;
	SFile >> m_bRun;
	SFile >> m_fRunVelo;
	SFile >> m_fLiveTime;

	SFile >> fActionDNRate;

	SFile >> m_bMobLink;
	SFile >> m_sMobLinkID.dwID;

	SCROWPATTERN sPattern;
	DWORD dwNUM(0);
	SFile >> dwNUM;
	for( DWORD i = 0; i < dwNUM; i++ )
	{
		SFile.ReadBuffer ( &sPattern, sizeof( SCROWPATTERN ) );

		if ( sPattern.m_dwPatternAttackSet >= 3 )
			sPattern.m_dwPatternAttackSet = ACTION_MAXATTACK;

		m_vecPatternList.push_back ( sPattern );
	}

	return true;
}

bool SCROWACTION::LOAD108 ( basestream &SFile )
{
	DWORD dwDATA(0);
	float fActionDNRate(0);

	SFile >> m_wBodyRadius;
	SFile >> m_strSkinObj;
	SFile >> m_strTalkFile;

	SFile >> m_dwActFlag;

	for ( int i = 0; i < SALENUM; ++i )
		SFile >> m_strSaleFile[i];

	SFile >> m_bAfterFall_NoBody;
	SFile >> m_strBirthEffect;
	SFile >> m_strFallingEffect;
	SFile >> m_strBlowEffect;


	SFile >> dwDATA; m_emMoveType = ( EMMOVETYPE ) dwDATA;
	SFile >> m_fDriftHeight;

	SFile >> m_fWalkVelo;
	SFile >> m_bRun;
	SFile >> m_fRunVelo;
	SFile >> m_fLiveTime;

	SFile >> fActionDNRate;

	SFile >> m_bMobLink;
	SFile >> m_sMobLinkID.dwID;
	SFile >> m_fMobLinkScale;
	SFile >> m_fMobLinkDelay;

	SCROWPATTERN sPattern;
	DWORD dwNUM(0);
	SFile >> dwNUM;
	for( DWORD i = 0; i < dwNUM; i++ )
	{
		SFile.ReadBuffer ( &sPattern, sizeof( SCROWPATTERN ) );

		if ( sPattern.m_dwPatternAttackSet >= 3 )
			sPattern.m_dwPatternAttackSet = ACTION_MAXATTACK;

		m_vecPatternList.push_back ( sPattern );
	}

	return true;
}

bool SCROWACTION::LOAD110 ( basestream &SFile )
{
	DWORD dwDATA(0);
	SFile >> m_dwActFlag;
	SFile >> dwDATA; m_emMoveType = ( EMMOVETYPE ) dwDATA;
	SFile >> m_fDriftHeight;
	SFile >> m_fWalkVelo;
	SFile >> m_bRun;
	SFile >> m_fRunVelo;
	SFile >> m_wBodyRadius;
	SFile >> m_strSkinObj;
	SFile >> m_strTalkFile;
	SFile >> m_bAfterFall_NoBody;
	SFile >> m_strBirthEffect;
	SFile >> m_strFallingEffect;
	SFile >> m_strBlowEffect;

	for ( int i = 0; i < SALENUM; ++i )
		SFile >> m_strSaleFile[i];

	SFile >> m_fLiveTime;
	SFile >> m_bMobLink;
	SFile >> m_sMobLinkID.dwID;
	SFile >> m_fMobLinkScale;
	SFile >> m_fMobLinkDelay;

	SCROWPATTERN sPattern;
	DWORD dwNUM(0);
	SFile >> dwNUM;
	for( DWORD i = 0; i < dwNUM; i++ )
	{
		SFile.ReadBuffer ( &sPattern, sizeof( SCROWPATTERN ) );
		m_vecPatternList.push_back ( sPattern );
	}

	return true;
}

bool SCROWACTION::LOAD200 ( basestream &SFile )
{
	DWORD dwDATA(0);
	SFile >> m_dwActFlag;
	SFile >> dwDATA; m_emMoveType = ( EMMOVETYPE ) dwDATA;
	SFile >> m_fDriftHeight;
	SFile >> m_fWalkVelo;
	SFile >> m_bRun;
	SFile >> m_fRunVelo;
	SFile >> m_wBodyRadius;
	SFile >> m_strSkinObj;
	SFile >> m_strTalkFile;
	SFile >> m_bAfterFall_NoBody;
	SFile >> m_strBirthEffect;
	SFile >> m_strFallingEffect;
	SFile >> m_strBlowEffect;

	for ( int i = 0; i < SALENUM; ++i )
		SFile >> m_strSaleFile[i];

	SFile >> m_strShopFile; /*npc shop, Juver, 2017/07/25 */
	SFile >> m_fLiveTime;
	SFile >> m_bMobLink;
	SFile >> m_sMobLinkID.dwID;
	SFile >> m_fMobLinkScale;
	SFile >> m_fMobLinkDelay;

	SCROWPATTERN sPattern;
	DWORD dwNUM(0);
	SFile >> dwNUM;
	for( DWORD i = 0; i < dwNUM; i++ )
	{
		SFile.ReadBuffer ( &sPattern, sizeof( SCROWPATTERN ) );
		m_vecPatternList.push_back ( sPattern );
	}

	return true;
}

bool SCROWACTION::LOAD ( basestream &SFile )
{
	DWORD dwDATA(0);
	SFile >> m_dwActFlag;
	SFile >> dwDATA; m_emMoveType = ( EMMOVETYPE ) dwDATA;
	SFile >> m_fDriftHeight;
	SFile >> m_fWalkVelo;
	SFile >> m_bRun;
	SFile >> m_fRunVelo;
	SFile >> m_wBodyRadius;
	SFile >> m_strSkinObj;
	SFile >> m_strTalkFile;
	SFile >> m_bAfterFall_NoBody;
	SFile >> m_strBirthEffect;
	SFile >> m_strFallingEffect;
	SFile >> m_strBlowEffect;

	for ( int i = 0; i < SALENUM; ++i )
		SFile >> m_strSaleFile[i];

	SFile >> m_strShopFile; /*npc shop, Juver, 2017/07/25 */
	SFile >> m_fLiveTime;
	SFile >> m_bMobLink;
	SFile >> m_sMobLinkID.dwID;
	SFile >> m_fMobLinkScale;
	SFile >> m_fMobLinkDelay;

	SCROWPATTERN sPattern;
	DWORD dwNUM(0);
	SFile >> dwNUM;
	for( DWORD i = 0; i < dwNUM; i++ )
	{
		SFile.ReadBuffer ( &sPattern, sizeof( SCROWPATTERN ) );
		m_vecPatternList.push_back ( sPattern );
	}

	SFile >> m_strItemExchangeFile; /*item exchange, Juver, 2017/10/11 */

	return true;
}

bool SCROWACTION::SAVE ( CSerialFile &SFile )
{
	SFile << m_dwActFlag;
	SFile << (DWORD)m_emMoveType;
	SFile << m_fDriftHeight;
	SFile << m_fWalkVelo;
	SFile << m_bRun;
	SFile << m_fRunVelo;
	SFile << m_wBodyRadius;
	SFile << m_strSkinObj;
	SFile << m_strTalkFile;
	SFile << m_bAfterFall_NoBody;
	SFile << m_strBirthEffect;
	SFile << m_strFallingEffect;
	SFile << m_strBlowEffect;

	for ( int i = 0; i < SALENUM; ++i )
		SFile << m_strSaleFile[i];

	SFile << m_strShopFile; /*npc shop, Juver, 2017/07/25 */
	SFile << m_fLiveTime;
	SFile << m_bMobLink;
	SFile << m_sMobLinkID.dwID;
	SFile << m_fMobLinkScale;
	SFile << m_fMobLinkDelay;

	SFile << (DWORD)m_vecPatternList.size();
	if ( m_vecPatternList.size() > 0 )
	{
		SFile.WriteBuffer ( &(m_vecPatternList[0]), DWORD( sizeof( SCROWPATTERN ) * m_vecPatternList.size() ) );
	}

	SFile << m_strItemExchangeFile; /*item exchange, Juver, 2017/10/11 */

	return true;
}

VOID SCROWACTION::SaveCsvHead ( std::fstream &SFile )
{
	SFile << "wBodyRadius" << ",";

	SFile << "strSkinObj" << ",";
	SFile << "strTalkFile" << ",";

	SFile << "dwActFlag" << ",";
	for( int i=0; i<SALENUM; ++i )
		SFile << "strSaleFile" << i << ",";

	SFile << "strShopFile" << ","; /*npc shop, Juver, 2017/07/25 */

	SFile << "bAfterFall_NoBody" << ",";

	SFile << "strBirthEffect" << ",";
	SFile << "strFallingEffect" << ",";
	SFile << "strBlowEffect" << ",";

	SFile << "emMoveType" << ",";
	SFile << "fDriftHeight" << ",";

	SFile << "fWalkVelo" << ",";
	SFile << "bRun" << ",";
	SFile << "fRunVelo" << ",";
	SFile << "m_fLiveTime" << ",";

	SFile << "m_bMobLink" << ",";
	SFile << "m_sMobLinkID wMainID" << ",";
	SFile << "m_sMobLinkID wSubID" << ",";
	SFile << "m_fMobLinkScale" << ",";
	SFile << "m_fMobLinkDelay" << ",";

	for( int i = 0; i < PATTERNNUM; i++ )
	{
		SFile << "m_fPatternDNRate" << i << ",";
		SFile << "m_emPatternDN" << i << ",";
		SFile << "m_dwPatternAttackSet" << i << ",";
	}

	SFile << "strItemExchangeFile" << ","; /*item exchange, Juver, 2017/10/11 */
}

VOID SCROWACTION::SaveCsv ( std::fstream &SFile )
{
	SFile << m_wBodyRadius << ",";

	STRUTIL::OutputStrCsv( SFile, m_strSkinObj );
	STRUTIL::OutputStrCsv( SFile, m_strTalkFile );

	SFile << m_dwActFlag << ",";
	for( int i=0; i<SALENUM; ++i )
		STRUTIL::OutputStrCsv( SFile, m_strSaleFile[i] );

	STRUTIL::OutputStrCsv( SFile, m_strShopFile ); /*npc shop, Juver, 2017/07/25 */

	SFile << m_bAfterFall_NoBody << ",";

	STRUTIL::OutputStrCsv( SFile, m_strBirthEffect );
	STRUTIL::OutputStrCsv( SFile, m_strFallingEffect );
	STRUTIL::OutputStrCsv( SFile, m_strBlowEffect );

	SFile << m_emMoveType << ",";
	SFile << m_fDriftHeight << ",";

	SFile << m_fWalkVelo << ",";
	SFile << m_bRun << ",";
	SFile << m_fRunVelo << ",";
	SFile << m_fLiveTime << ",";

	SFile << m_bMobLink << ",";
	SFile << m_sMobLinkID.wMainID << ",";
	SFile << m_sMobLinkID.wSubID << ",";
	SFile << m_fMobLinkScale << ",";
	SFile << m_fMobLinkDelay << ",";

	const int iMaxSize = ( int ) PATTERNNUM;
	int iSize = ( int ) m_vecPatternList.size();

	if( iSize > iMaxSize )
	{
		iSize = iMaxSize;
		MessageBox( NULL, _T("몬스터 AI는 최대 10개까지만 저장합니다."), _T("ERROR"), MB_OK );
	}

	for( int i = 0; i < iMaxSize; i++ )
	{
		if( i < iSize )
		{
			SFile << m_vecPatternList[ i ].m_fPatternDNRate << ",";
			SFile << m_vecPatternList[ i ].m_emActPattern << ",";
			SFile << m_vecPatternList[ i ].m_dwPatternAttackSet << ",";
		}
		else
		{
			SFile << "0" << ",";
			SFile << "0" << ",";
			SFile << "0" << ",";
		}
	}

	STRUTIL::OutputStrCsv( SFile, m_strItemExchangeFile ); /*item exchange, Juver, 2017/10/11 */
}

VOID SCROWACTION::LoadCsv ( CStringArray &StrArray, int& iCsvCur )
{
	m_wBodyRadius = ( WORD ) atoi( StrArray[ iCsvCur++ ] );

	STRUTIL::InputStrCsv( StrArray[ iCsvCur++ ], m_strSkinObj );
	STRUTIL::InputStrCsv( StrArray[ iCsvCur++ ], m_strTalkFile );

	m_dwActFlag = ( DWORD ) atol( StrArray[ iCsvCur++ ] );
	for( int i = 0; i < SALENUM; ++i )
		STRUTIL::InputStrCsv( StrArray[ iCsvCur++ ], m_strSaleFile[i] );

	STRUTIL::InputStrCsv( StrArray[ iCsvCur++ ], m_strShopFile ); /*npc shop, Juver, 2017/07/25 */

	m_bAfterFall_NoBody = ( BOOL ) atol( StrArray[ iCsvCur++ ] );

	STRUTIL::InputStrCsv( StrArray[ iCsvCur++ ], m_strBirthEffect );
	STRUTIL::InputStrCsv( StrArray[ iCsvCur++ ], m_strFallingEffect );
	STRUTIL::InputStrCsv( StrArray[ iCsvCur++ ], m_strBlowEffect );

	m_emMoveType = ( EMMOVETYPE ) atoi( StrArray[ iCsvCur++ ] );
	m_fDriftHeight = ( float ) atof( StrArray[ iCsvCur++ ] );

	m_fWalkVelo = ( float ) atof( StrArray[ iCsvCur++ ] );
	m_bRun = ( BOOL ) atol( StrArray[ iCsvCur++ ] );
	m_fRunVelo = ( float ) atof( StrArray[ iCsvCur++ ] );
	m_fLiveTime = ( float ) atof( StrArray[ iCsvCur++ ] );

	m_bMobLink = ( BOOL ) atol( StrArray[ iCsvCur++ ] );
	m_sMobLinkID.wMainID = ( WORD ) atoi( StrArray[ iCsvCur++ ] );
	m_sMobLinkID.wSubID = ( WORD ) atoi( StrArray[ iCsvCur++ ] );
	m_fMobLinkScale = ( float ) atof( StrArray[ iCsvCur++ ] );
	m_fMobLinkDelay = ( float ) atof( StrArray[ iCsvCur++ ] );

	const int iMaxSize = ( int ) PATTERNNUM;

	m_vecPatternList.clear();

	for( int i = 0; i < iMaxSize; i++ )
	{
		SCROWPATTERN sPattern;
		sPattern.m_fPatternDNRate = ( float ) atof( StrArray[ iCsvCur++ ] );
		sPattern.m_emActPattern = ( EMCROWACT_UP ) atoi( StrArray[ iCsvCur++ ] );
		sPattern.m_dwPatternAttackSet = atoi( StrArray[ iCsvCur++ ] );

		if( sPattern.m_fPatternDNRate == 0.0f &&
			sPattern.m_emActPattern == EMCROWACT_UP_IDLE &&
			sPattern.m_dwPatternAttackSet == 0 )
			continue;

		m_vecPatternList.push_back( sPattern );
	}

	STRUTIL::InputStrCsv( StrArray[ iCsvCur++ ], m_strItemExchangeFile ); /*item exchange, Juver, 2017/10/11 */
}

namespace COMMENT
{
	std::string szCROWACT_UP[EMCROWACT_UP_NSIZE] =
	{
		"Standby",
		"Attack Nearest Enemy",
		"Attack Lower Level",
		"Attack Lower HP",
		"Attack High BrightPoints",
		"Attack Low BrightPoints",
		"Attack Enemy With Abnormal State",
		"Attack Enemy With Weapon",
		"Attack Running Enemy",
		"Run Away From Enemy",
	};

	std::string szCROWACT_DN[EMCROWACT_DN_NSIZE] =
	{
		"No Action",
		"Look For Lower HP",
		"Run Away From Enemy",
	};
};