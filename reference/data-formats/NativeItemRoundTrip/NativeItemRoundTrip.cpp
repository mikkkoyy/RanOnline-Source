// NativeItemRoundTrip.cpp
//
// Standalone native item.isf round-trip validator.
//
// Uses the AUTHORITATIVE legacy RAN implementation from the prebuilt static
// libraries (Lib_Client.lib / Lib_Engine.lib):
//
//     GLItemMan::LoadFile()  ->  SITEM database  ->  GLItemMan::SaveFile()
//
// This tool performs NO conversion to CSV/JSON/SQLite. The only artifacts are
// native item.isf files.
//
// It does NOT require DirectX, DxResponseMan, GMTOOL or any GUI subsystem.
// GLItemMan::LoadFile() self-initialises: it calls CleanUp() (a no-op on a
// fresh singleton, since m_ppItem is NULL) and then OneTimeSceneInit(), which
// only allocates the item pointer table.
//
// Initialisation pattern copied from the existing proven usage in
// EditorItem/EditorItemDlg.cpp (OnInitDialog) and EditorItem/EditorItem.cpp
// (ExportItemCsv), reduced to the minimum needed for a headless native
// load/save:
//     CDebugSet::OneTimeSceneInit( dir )  -> makes ToLogFile/ErrorFile safe
//     GLItemMan::GetInstance()            -> LoadFile() self-initialises
// DxResponseMan and GMTOOL are intentionally NOT initialised: neither is
// required by LoadFile/SaveFile.

#include "stdafx.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <type_traits>

#include "GLItemMan.h"
#include "GLOGIC.h"

// ---------------------------------------------------------------------------
// Comparison helpers
// ---------------------------------------------------------------------------

static int  g_nMismatch = 0;
static int  g_nCompared = 0;
static char g_szCurrentId[64] = "";

static void ReportMismatch(const char* szField)
{
	if ( g_nMismatch < 40 )
		printf( "    MISMATCH [%s] field=%s\n", g_szCurrentId, szField );
	++g_nMismatch;
}

template<typename T>
static void CmpPod(const char* szField, const T& a, const T& b)
{
	static_assert( std::is_trivially_copyable<T>::value,
		"CmpPod requires a trivially copyable type" );
	++g_nCompared;
	if ( memcmp( &a, &b, sizeof(T) ) != 0 )
		ReportMismatch( szField );
}

static void CmpStr(const char* szField, const std::string& a, const std::string& b)
{
	++g_nCompared;
	if ( a != b )
		ReportMismatch( szField );
}

static void CmpScalar(const char* szField, long long a, long long b)
{
	++g_nCompared;
	if ( a != b )
		ReportMismatch( szField );
}

// ---------------------------------------------------------------------------
// SITEMBASIC : contains std::string members, so compare field by field.
// Field list taken verbatim from GLItemBasic.h (struct SITEMBASIC, VERSION 0x0205).
// ---------------------------------------------------------------------------

static void CompareBasic(const ITEM::SITEMBASIC& a, const ITEM::SITEMBASIC& b)
{
	CmpPod  ( "sBasicOp.sNativeID",     a.sNativeID,     b.sNativeID );
	CmpPod  ( "sBasicOp.sGroupID",      a.sGroupID,      b.sGroupID );
	CmpScalar( "sBasicOp.emLevel",      a.emLevel,       b.emLevel );
	CmpScalar( "sBasicOp.emItemType",   a.emItemType,    b.emItemType );
	CmpStr  ( "sBasicOp.strName",      a.strName,       b.strName );
	CmpPod  ( "sBasicOp.fExpMultiple",  a.fExpMultiple,  b.fExpMultiple );
	CmpPod  ( "sBasicOp.wGradeAttack",  a.wGradeAttack,  b.wGradeAttack );
	CmpPod  ( "sBasicOp.wGradeDefense", a.wGradeDefense, b.wGradeDefense );
	CmpPod  ( "sBasicOp.dwFlags",       a.dwFlags,       b.dwFlags );
	CmpPod  ( "sBasicOp.dwBuyPrice",    a.dwBuyPrice,    b.dwBuyPrice );
	CmpPod  ( "sBasicOp.dwSellPrice",   a.dwSellPrice,   b.dwSellPrice );
	CmpPod  ( "sBasicOp.wReserved1",    a.wReserved1,    b.wReserved1 );
	CmpPod  ( "sBasicOp.wReserved2",    a.wReserved2,    b.wReserved2 );
	CmpPod  ( "sBasicOp.wReserved3",    a.wReserved3,    b.wReserved3 );
	CmpPod  ( "sBasicOp.wReserved4",    a.wReserved4,    b.wReserved4 );
	CmpPod  ( "sBasicOp.wReserved5",    a.wReserved5,    b.wReserved5 );
	CmpPod  ( "sBasicOp.dwReqCharClass", a.dwReqCharClass, b.dwReqCharClass );
	CmpPod  ( "sBasicOp.dwReqSchool",   a.dwReqSchool,   b.dwReqSchool );
	CmpPod  ( "sBasicOp.sReqStats",     a.sReqStats,     b.sReqStats );
	CmpPod  ( "sBasicOp.wReqLevelDW",   a.wReqLevelDW,   b.wReqLevelDW );
	CmpPod  ( "sBasicOp.wReqLevelUP",   a.wReqLevelUP,   b.wReqLevelUP );
	CmpPod  ( "sBasicOp.wReqPA",        a.wReqPA,        b.wReqPA );
	CmpPod  ( "sBasicOp.wReqSA",        a.wReqSA,        b.wReqSA );
	CmpPod  ( "sBasicOp.emReqBright",   a.emReqBright,   b.emReqBright );
	CmpPod  ( "sBasicOp.dwCoolTime",    a.dwCoolTime,    b.dwCoolTime );
	CmpPod  ( "sBasicOp.emCoolType",    a.emCoolType,    b.emCoolType );
	CmpPod  ( "sBasicOp.wInvenSizeX",   a.wInvenSizeX,   b.wInvenSizeX );
	CmpPod  ( "sBasicOp.wInvenSizeY",   a.wInvenSizeY,   b.wInvenSizeY );
	CmpPod  ( "sBasicOp.sICONID",       a.sICONID,       b.sICONID );
	CmpStr  ( "sBasicOp.strFieldFile",     a.strFieldFile,     b.strFieldFile );
	CmpStr  ( "sBasicOp.strInventoryFile", a.strInventoryFile, b.strInventoryFile );
	CmpStr  ( "sBasicOp.strTargBodyEffect", a.strTargBodyEffect, b.strTargBodyEffect );
	CmpStr  ( "sBasicOp.strTargetEffect",  a.strTargetEffect,  b.strTargetEffect );
	CmpStr  ( "sBasicOp.strSelfBodyEffect", a.strSelfBodyEffect, b.strSelfBodyEffect );

	for ( int i = 0; i < GLCI_NUM_8CLASS; ++i )
	{
		char szField[64];
		sprintf_s( szField, sizeof(szField), "sBasicOp.strWearingFileRight[%d]", i );
		CmpStr( szField, a.strWearingFileRight[i], b.strWearingFileRight[i] );
		sprintf_s( szField, sizeof(szField), "sBasicOp.strWearingFileLeft[%d]", i );
		CmpStr( szField, a.strWearingFileLeft[i], b.strWearingFileLeft[i] );
	}

	CmpStr  ( "sBasicOp.strPetWearingFile", a.strPetWearingFile, b.strPetWearingFile );
	CmpStr  ( "sBasicOp.strComment",        a.strComment,        b.strComment );
	CmpPod  ( "sBasicOp.sSubID",            a.sSubID,            b.sSubID );
	CmpPod  ( "sBasicOp.wPosX",             a.wPosX,             b.wPosX );
	CmpPod  ( "sBasicOp.wPosY",             a.wPosY,             b.wPosY );
	CmpPod  ( "sBasicOp.bEnable",           a.bEnable,           b.bEnable );
	CmpPod  ( "sBasicOp.dwReqActivityPoint",    a.dwReqActivityPoint,    b.dwReqActivityPoint );
	CmpPod  ( "sBasicOp.dwReqContributionPoint", a.dwReqContributionPoint, b.dwReqContributionPoint );
	CmpPod  ( "sBasicOp.dwReqUserNum",          a.dwReqUserNum,          b.dwReqUserNum );
	CmpPod  ( "sBasicOp.bItemColor",            a.bItemColor,            b.bItemColor );
	CmpPod  ( "sBasicOp.wItemColor1",           a.wItemColor1,           b.wItemColor1 );
	CmpPod  ( "sBasicOp.wItemColor2",           a.wItemColor2,           b.wItemColor2 );
	CmpPod  ( "sBasicOp.sidWrapperBox",         a.sidWrapperBox,         b.sidWrapperBox );
	CmpPod  ( "sBasicOp.bItemTransfer",         a.bItemTransfer,         b.bItemTransfer );
}

// ---------------------------------------------------------------------------
// SGRINDING : contains std::string strGrind, so compare field by field.
// ---------------------------------------------------------------------------

static void CompareGrinding(const ITEM::SGRINDING& a, const ITEM::SGRINDING& b)
{
	CmpPod   ( "sGrindingOp.emLEVEL",   a.emLEVEL,  b.emLEVEL  );
	CmpPod   ( "sGrindingOp.emTYPE",    a.emTYPE,   b.emTYPE   );
	CmpPod   ( "sGrindingOp.emCLASS",   a.emCLASS,  b.emCLASS  );
	CmpStr   ( "sGrindingOp.strGrind", a.strGrind, b.strGrind );
	CmpScalar( "sGrindingOp.bNoFail",   a.bNoFail,  b.bNoFail  );
}

// SRANDOMBOX holds std::vector<SRANDOMITEM> vecBOX
static void CompareRandomBox(const ITEM::SRANDOMBOX& a, const ITEM::SRANDOMBOX& b)
{
	if ( a.vecBOX.size() != b.vecBOX.size() )
	{
		ReportMismatch( "sRandomBox.vecBOX.size" );
		return;
	}
	for ( size_t i = 0; i < a.vecBOX.size(); ++i )
	{
		CmpPod( "sRandomBox.vecBOX[].nidITEM", a.vecBOX[i].nidITEM, b.vecBOX[i].nidITEM );
		CmpPod( "sRandomBox.vecBOX[].fRATE",   a.vecBOX[i].fRATE,   b.vecBOX[i].fRATE );
	}
}

// SPETSKINPACKITEM holds std::vector<SPETSKINPACKITEMDATA> vecPetSkinData
static void ComparePetSkinPack(const ITEM::SPETSKINPACKITEM& a, const ITEM::SPETSKINPACKITEM& b)
{
	CmpPod( "sPetSkinPack.dwPetSkinTime", a.dwPetSkinTime, b.dwPetSkinTime );
	if ( a.vecPetSkinData.size() != b.vecPetSkinData.size() )
	{
		ReportMismatch( "sPetSkinPack.vecPetSkinData.size" );
		return;
	}
	for ( size_t i = 0; i < a.vecPetSkinData.size(); ++i )
	{
		CmpPod( "sPetSkinPack.vec[].sMobID", a.vecPetSkinData[i].sMobID, b.vecPetSkinData[i].sMobID );
		CmpPod( "sPetSkinPack.vec[].fScale", a.vecPetSkinData[i].fScale, b.vecPetSkinData[i].fScale );
		CmpPod( "sPetSkinPack.vec[].fRate",  a.vecPetSkinData[i].fRate,  b.vecPetSkinData[i].fRate );
	}
}

// SRVCARD declares a user-defined operator=, so it is not trivially copyable,
// but every one of its members is POD.
static void CompareRvCard(const ITEM::SRVCARD& a, const ITEM::SRVCARD& b)
{
	CmpPod( "sRvCard.emOption",    a.emOption,    b.emOption );
	CmpPod( "sRvCard.wValue",      a.wValue,      b.wValue );
	CmpPod( "sRvCard.bCheckExist", a.bCheckExist, b.bCheckExist );
	CmpPod( "sRvCard.bReplaceOpt", a.bReplaceOpt, b.bReplaceOpt );
	CmpPod( "sRvCard.bUseSuit",    a.bUseSuit,    b.bUseSuit );
}

// ---------------------------------------------------------------------------
// Full SITEM comparison.
//
// SSUIT carries sADDON[ADDON_SIZE] (5), sVARIATE[VARIATION_SIZE] (5) and
// sVOLUME, so comparing it byte-wise explicitly covers ADDON / VARIATE /
// VOLUME truncation, plus damage, defense, hit, avoid, elemental resistances,
// equipment position and hand information.
// The other POD blocks cover drug, skillbook, generate, box, question item,
// random option, pet, vehicle and RVCARD data.
// ---------------------------------------------------------------------------

static void CompareItem(const SITEM& a, const SITEM& b)
{
	CompareBasic( a.sBasicOp, b.sBasicOp );

	CmpPod( "sSuitOp",      a.sSuitOp,      b.sSuitOp );
	CmpPod( "sDrugOp",      a.sDrugOp,      b.sDrugOp );
	CmpPod( "sSkillBookOp", a.sSkillBookOp, b.sSkillBookOp );

	CompareGrinding( a.sGrindingOp, b.sGrindingOp );

	CmpPod( "sGenerateOp",   a.sGenerateOp,   b.sGenerateOp );
	CmpPod( "sBox",          a.sBox,          b.sBox );
	CompareRandomBox( a.sRandomBox, b.sRandomBox );
	CmpPod( "sQuestionItem", a.sQuestionItem, b.sQuestionItem );
	CmpPod( "sRandomOpt",    a.sRandomOpt,    b.sRandomOpt );
	CmpPod( "sPet",          a.sPet,          b.sPet );
	CmpPod( "sVehicle",      a.sVehicle,      b.sVehicle );
	ComparePetSkinPack( a.sPetSkinPack, b.sPetSkinPack );
	CompareRvCard( a.sRvCard, b.sRvCard );
}

// ---------------------------------------------------------------------------
// Database snapshot
// ---------------------------------------------------------------------------

typedef std::map<DWORD, SITEM> ItemMap;

static DWORD MakeKey( const SNATIVEID& sID )
{
	return ( (DWORD)sID.wMainID << 16 ) | (DWORD)sID.wSubID;
}

static ItemMap SnapshotDatabase()
{
	ItemMap map;
	GLItemMan& man = GLItemMan::GetInstance();

	for ( int i = 0; i < GLItemMan::MAX_MID; ++i )
	{
		for ( int j = 0; j < GLItemMan::MAX_SID; ++j )
		{
			// public accessor; m_ppItem itself is protected
			SITEM* pItem = man.GetItem( SNATIVEID( (WORD)i, (WORD)j ) );
			if ( pItem )
				map[ MakeKey( pItem->sBasicOp.sNativeID ) ] = *pItem;
		}
	}
	return map;
}

static bool FilesIdentical( const char* pa, const char* pb )
{
	FILE* fa = fopen( pa, "rb" );
	if ( !fa ) return false;
	fseek( fa, 0, SEEK_END );
	long sa = ftell( fa );
	fseek( fa, 0, SEEK_SET );

	FILE* fb = fopen( pb, "rb" );
	if ( !fb ) { fclose( fa ); return false; }
	fseek( fb, 0, SEEK_END );
	long sb = ftell( fb );
	fclose( fb );

	if ( sa != sb ) { fclose( fa ); return false; }

	bool bSame = true;
	const size_t CHUNK = 1 << 20;
	std::vector<char> ba( CHUNK ), bb( CHUNK );
	for ( long off = 0; off < sa; off += (long)CHUNK )
	{
		size_t want = (size_t)( ( sa - off < (long)CHUNK ) ? ( sa - off ) : (long)CHUNK );
		if ( fread( &ba[0], 1, want, fa ) != want ) { bSame = false; break; }
		if ( fread( &bb[0], 1, want, fb ) != want ) { bSame = false; break; }
		if ( memcmp( &ba[0], &bb[0], want ) != 0 )    { bSame = false; break; }
	}
	fclose( fa );
	return bSame;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main( int argc, char** argv )
{
	if ( argc < 3 )
	{
		printf( "usage: NativeItemRoundTrip.exe <original.isf> <roundtrip.isf>\n" );
		return 2;
	}

	const char* szOriginal  = argv[1];
	const char* szRoundtrip = argv[2];

	setvbuf( stdout, NULL, _IONBF, 0 );

	// CDebugSet is a NAMESPACE, not a class. This makes ToLogFile()/ErrorFile()
	// safe, which GLItemMan::LoadFile() uses internally. No DirectX, no HWND.
	CDebugSet::OneTimeSceneInit( "." );

	GLItemMan& man = GLItemMan::GetInstance();

	// ---- 1. Load original -------------------------------------------------
	printf( "Loading original...\n" );
	HRESULT hr = man.LoadFile( szOriginal, TRUE, TRUE );
	if ( FAILED( hr ) )
	{
		printf( "FAIL: GLItemMan::LoadFile(original) hr=0x%08lX\n", (unsigned long)hr );
		return 3;
	}
	ItemMap mapA = SnapshotDatabase();
	printf( "PASS (original loaded, %u items)\n", (unsigned)mapA.size() );

	// ---- 2. Save round-trip ----------------------------------------------
	printf( "Saving round-trip...\n" );
	hr = man.SaveFile( szRoundtrip );
	if ( FAILED( hr ) )
	{
		printf( "FAIL: GLItemMan::SaveFile hr=0x%08lX\n", (unsigned long)hr );
		return 4;
	}
	printf( "PASS (round-trip saved)\n" );

	// ---- 3. Load round-trip ----------------------------------------------
	// LoadFile() internally calls CleanUp(), so the previous database is freed.
	printf( "Loading round-trip...\n" );
	hr = man.LoadFile( szRoundtrip, TRUE, TRUE );
	if ( FAILED( hr ) )
	{
		printf( "FAIL: GLItemMan::LoadFile(roundtrip) hr=0x%08lX\n", (unsigned long)hr );
		return 5;
	}
	ItemMap mapB = SnapshotDatabase();
	printf( "PASS (round-trip loaded, %u items)\n", (unsigned)mapB.size() );

	// ---- 4. Compare databases --------------------------------------------
	printf( "Comparing database...\n" );

	if ( mapA.size() != mapB.size() )
	{
		printf( "    MISMATCH item count original=%u roundtrip=%u\n",
			(unsigned)mapA.size(), (unsigned)mapB.size() );
		++g_nMismatch;
	}

	for ( ItemMap::const_iterator it = mapA.begin(); it != mapA.end(); ++it )
	{
		if ( mapB.find( it->first ) == mapB.end() )
		{
			sprintf_s( g_szCurrentId, sizeof(g_szCurrentId), "%u/%u",
				(unsigned)( it->first >> 16 ), (unsigned)( it->first & 0xFFFF ) );
			ReportMismatch( "MISSING_IN_ROUNDTRIP" );
		}
	}
	for ( ItemMap::const_iterator it = mapB.begin(); it != mapB.end(); ++it )
	{
		if ( mapA.find( it->first ) == mapA.end() )
		{
			sprintf_s( g_szCurrentId, sizeof(g_szCurrentId), "%u/%u",
				(unsigned)( it->first >> 16 ), (unsigned)( it->first & 0xFFFF ) );
			ReportMismatch( "EXTRA_IN_ROUNDTRIP" );
		}
	}

	for ( ItemMap::const_iterator it = mapA.begin(); it != mapA.end(); ++it )
	{
		ItemMap::const_iterator jt = mapB.find( it->first );
		if ( jt == mapB.end() )
			continue;

		sprintf_s( g_szCurrentId, sizeof(g_szCurrentId), "%u/%u",
			(unsigned)( it->first >> 16 ), (unsigned)( it->first & 0xFFFF ) );
		CompareItem( it->second, jt->second );
	}

	if ( g_nMismatch == 0 )
		printf( "PASS\n" );
	else
		printf( "FAIL (%d mismatches across %d field comparisons)\n",
			g_nMismatch, g_nCompared );

	printf( "\n" );
	printf( "ITEM COUNT: original=%u roundtrip=%u\n",
		(unsigned)mapA.size(), (unsigned)mapB.size() );
	printf( "FIELD COMPARISONS: %d\n", g_nCompared );
	printf( "MISMATCHES: %d\n", g_nMismatch );
	printf( "BYTE-IDENTICAL FILES: %s\n",
		FilesIdentical( szOriginal, szRoundtrip ) ? "yes" : "no" );

	man.FinalCleanup();

	printf( "\nRESULT: %s\n", ( g_nMismatch == 0 ) ? "PASS" : "FAIL" );
	return ( g_nMismatch == 0 ) ? 0 : 1;
}
