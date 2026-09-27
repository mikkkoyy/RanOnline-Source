#include "stdafx.h"

#include "SerialFile.h"

#include "DxSkinAniMan.h"
#include "../Common/StringUtils.h"
#include "../Common/StlFunctions.h"
#include "../Common/StringFile.h"

#include "../DxEffect/Effani/DxEffAni.h"

#include <algorithm>
#include "../DxFrame/dsutil.h"
#include "../DxCommon/DxViewPort.h"
#include "../DxSound/DxSoundMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace COMMENT
{
	TSTRING ANI_MAINTYPE_CHAR[AN_TYPE_SIZE] =
	{
		_T("(00) Guard (Neutral)"),
		_T("(01) Rest"),
		_T("(02) Walk"),
		_T("(03) Run"),
		_T("(04) Attack"),
		_T("(05) Shock"),
		_T("(06) Die"),
		_T("(07) Produce"),
		_T("(08) Talk"),
		_T("(09) SKILL-A"),
		_T("(10) Action(Gesture)"),
		_T("(11) Shock(MIX)"),
		_T("(12) Guard (Insufficient SP)"),
		_T("(13) Duel Winner"),
		_T("(14) Duel Looser"),
		_T("(15) Special"),
		_T("(16) SKILL-B"),
		_T("(17) SKILL-C"),
		_T("(18) SKILL-D"),
		_T("(19) SKILL-E"),
		_T("(20) Gathering"),
		_T("(21) BIKE-A"),
		_T("(22) BIKE-B"),
		_T("(23) BIKE-C"),
		_T("(24) BIKE-EV1"),
		_T("(25) Flight"),
		_T("(26) SKILL-F"),
		_T("(27) SKILL-G"),
		_T("(28) SKILL-H"),
		_T("(29) Car-A"),
		_T("(30) Car-B"),
		_T("(31) Car-C"),
		_T("(32) Car-D"),
		_T("(33) RearCar"),
	};
	
	TSTRING ANI_MAINTYPE_ABL[AN_TYPE_SIZE] =
	{
		_T("(00) Abl Stay"),
		_T("(01) Abl Attack"),
		_T("(02) Abl Free"),
		_T("(03) Abl Anim"),
		_T("(04) Abl Anim"),
		_T("(05) Abl Anim"),
		_T("(06) Abl Anim"),
		_T("(07) Abl Anim"),
		_T("(08) Abl Anim"),
		_T("(09) Abl Anim"),
		_T("(10) Abl Anim"),
		_T("(11) Abl Anim"),
		_T("(12) Abl Anim"),
		_T("(13) Abl Anim"),
		_T("(14) Abl Anim"),
		_T("(15) Abl Anim"),
		_T("(16) Abl Anim"),
		_T("(17) Abl Anim"),
		_T("(18) Abl Anim"),
		_T("(19) Abl Anim"),
		_T("(20) Abl Anim"),
		_T("(21) Abl Anim"),
		_T("(22) Abl Anim"),
		_T("(23) Abl Anim"),
		_T("(24) Abl Anim"),
		_T("(25) Abl Anim"),
		_T("(26) Abl Anim"),
		_T("(27) Abl Anim"),
		_T("(28) Abl Anim"),
		_T("(29) Abl Anim"),
		_T("(30) Abl Anim"),
		_T("(31) Abl Anim"),
		_T("(32) Abl Anim"),
		_T("(33) Abl Anim"),
	};

	TSTRING ANI_MAINTYPE_VEHICLE[AN_TYPE_SIZE] =
	{
		_T("(00) Vehicle Stay"),
		_T("(01) Vehicle Walk"),
		_T("(02) Vehicle Booster"),
		_T("(03) Vehicle Run"),
		_T("(04) Vehicle Down"),
		_T("(05) Vehicle Shock"),
		_T("(06) Vehicle Attack"),
		_T("(07) Vehicle Skill-A"),
		_T("(08) Vehicle Skill-B"),
		_T("(09) Vehicle Skill-C"),
		_T("(10) Vehicle Skill-D"),
		_T("(11) Vehicle Anim 01"),
		_T("(12) Vehicle Anim 02"),
		_T("(13) Vehicle Anim 03"),
		_T("(14) Vehicle Anim 04"),
		_T("(15) Vehicle Anim 05"),
		_T("(16) Vehicle Anim None"),
		_T("(17) Vehicle Anim"),
		_T("(18) Vehicle Anim"),
		_T("(19) Vehicle Anim"),
		_T("(20) Vehicle Anim"),
		_T("(21) Vehicle Anim"),
		_T("(22) Vehicle Anim"),
		_T("(23) Vehicle Anim"),
		_T("(24) Vehicle Anim"),
		_T("(25) Vehicle Anim"),
		_T("(26) Vehicle Anim"),
		_T("(27) Vehicle Anim"),
		_T("(28) Vehicle Anim"),
		_T("(29) Vehicle Anim"),
		_T("(30) Vehicle Anim"),
		_T("(31) Vehicle Anim"),
		_T("(32) Vehicle Anim"),
		_T("(33) Vehicle Anim"),
	};

	//character weapon sub animation
	TSTRING ANI_SUBTYPE_WEAPON[AN_SUB_00_SIZE] =
	{
		_T("(00) None"),
		_T("(01) Single Handed Sword"),
		_T("(02) Two Handed Sword"),
		_T("(03) Two Single Handed Sword"),
		_T("(04) Dagger"),
		_T("(05) Spear"),
		_T("(06) Bow"),
		_T("(07) Throw"),
		_T("(08) Magic"),
		_T("(09) Big Two Handed Sword"),
		_T("(10) Stick"),
		_T("(11) Single Handed Blade"),
		_T("(12) Two Handed Blade"),
		_T("(13) Big Two Handed Blade"),
		_T("(14) Two Single Handed Blade"),
		_T("(15) Guant"),
		_T("(16) Broom"),
		_T("(17) HoverBoard"),
		_T("(18) Gunner Dual Gun"),
		_T("(19) Gunner Gun"),
		_T("(20) Assassin Scythe"),
		_T("(21) Assassin Dual Spear"),
		_T("(22) Assassin Shuriken"),
		_T("(23) Extreme Fist"),
		_T("(24) Magician Wand"),
		_T("(25) Magician Cube"),
		_T("(26) Magician Rope"),
		_T("(27) Shaper Hammer"),
		_T("(28) Shaper Shield"),
		_T("(29) Shaper Umbrella"),
		_T("SUB_30"),
		_T("SUB_31"),
		_T("SUB_32"),
		_T("SUB_33"),
		_T("SUB_34"),
		_T("SUB_35"),
		_T("SUB_36"),
		_T("SUB_37"),
		_T("SUB_38"),
		_T("SUB_39"),
		_T("SUB_40"),
		_T("SUB_41"),
		_T("SUB_42"),
		_T("SUB_43"),
		_T("SUB_44"),
		_T("SUB_45"),
		_T("SUB_46"),
		_T("SUB_47"),
		_T("SUB_48"),
		_T("SUB_49"),
		_T("SUB_50"),
		_T("SUB_51"),
		_T("SUB_52"),
		_T("SUB_53"),
		_T("SUB_54"),
		_T("SUB_55"),
		_T("SUB_56"),
		_T("SUB_57"),
		_T("SUB_58"),
		_T("SUB_59"),
		_T("SUB_60"),
		_T("SUB_61"),
		_T("SUB_62"),
		_T("SUB_63"),
		_T("SUB_64"),
		_T("SUB_65"),
		_T("SUB_66"),
		_T("SUB_67"),
		_T("SUB_68"),
		_T("SUB_69"),
		_T("SUB_70"),
		_T("SUB_71"),
		_T("SUB_72"),
		_T("SUB_73"),
		_T("SUB_74"),
		_T("SUB_75"),
		_T("SUB_76"),
		_T("SUB_77"),
		_T("SUB_78"),
		_T("SUB_79"),
		_T("SUB_80"),
		_T("SUB_81"),
		_T("SUB_82"),
		_T("SUB_83"),
		_T("SUB_84"),
		_T("SUB_85"),
		_T("SUB_86"),
		_T("SUB_87"),
		_T("SUB_88"),
		_T("SUB_89"),
		_T("SUB_90"),
		_T("SUB_91"),
		_T("SUB_92"),
		_T("SUB_93"),
		_T("SUB_94"),
		_T("SUB_95"),
		_T("SUB_96"),
		_T("SUB_97"),
		_T("SUB_98"),
		_T("SUB_99")
	};

	//character skill sub animation
	TSTRING ANI_SUBTYPE_SKILL[AN_SUB_00_SIZE] =
	{
		_T("SUB_00"),
		_T("SUB_01"),
		_T("SUB_02"),
		_T("SUB_03"),
		_T("SUB_04"),
		_T("SUB_05"),
		_T("SUB_06"),
		_T("SUB_07"),
		_T("SUB_08"),
		_T("SUB_09"),
		_T("SUB_10"),
		_T("SUB_11"),
		_T("SUB_12"),
		_T("SUB_13"),
		_T("SUB_14"),
		_T("SUB_15"),
		_T("SUB_16"),
		_T("SUB_17"),
		_T("SUB_18"),
		_T("SUB_19"),
		_T("SUB_20"),
		_T("SUB_21"),
		_T("SUB_22"),
		_T("SUB_23"),
		_T("SUB_24"),
		_T("SUB_25"),
		_T("SUB_26"),
		_T("SUB_27"),
		_T("SUB_28"),
		_T("SUB_29"),
		_T("SUB_30"),
		_T("SUB_31"),
		_T("SUB_32"),
		_T("SUB_33"),
		_T("SUB_34"),
		_T("SUB_35"),
		_T("SUB_36"),
		_T("SUB_37"),
		_T("SUB_38"),
		_T("SUB_39"),
		_T("SUB_40"),
		_T("SUB_41"),
		_T("SUB_42"),
		_T("SUB_43"),
		_T("SUB_44"),
		_T("SUB_45"),
		_T("SUB_46"),
		_T("SUB_47"),
		_T("SUB_48"),
		_T("SUB_49"),
		_T("SUB_50"),
		_T("SUB_51"),
		_T("SUB_52"),
		_T("SUB_53"),
		_T("SUB_54"),
		_T("SUB_55"),
		_T("SUB_56"),
		_T("SUB_57"),
		_T("SUB_58"),
		_T("SUB_59"),
		_T("SUB_60"),
		_T("SUB_61"),
		_T("SUB_62"),
		_T("SUB_63"),
		_T("SUB_64"),
		_T("SUB_65"),
		_T("SUB_66"),
		_T("SUB_67"),
		_T("SUB_68"),
		_T("SUB_69"),
		_T("SUB_70"),
		_T("SUB_71"),
		_T("SUB_72"),
		_T("SUB_73"),
		_T("SUB_74"),
		_T("SUB_75"),
		_T("SUB_76"),
		_T("SUB_77"),
		_T("SUB_78"),
		_T("SUB_79"),
		_T("SUB_80"),
		_T("SUB_81"),
		_T("SUB_82"),
		_T("SUB_83"),
		_T("SUB_84"),
		_T("SUB_85"),
		_T("SUB_86"),
		_T("SUB_87"),
		_T("SUB_88"),
		_T("SUB_89"),
		_T("SUB_90"),
		_T("SUB_91"),
		_T("SUB_92"),
		_T("SUB_93"),
		_T("SUB_94"),
		_T("SUB_95"),
		_T("SUB_96"),
		_T("SUB_97"),
		_T("SUB_98"),
		_T("SUB_99")
	};

	TSTRING ANI_SUBTYPE_VEHICLE[AN_SUB_00_SIZE] =
	{
		_T("(00) Vehicle Front Board"),
		_T("(01) Vehicle Front Stay"),
		_T("(02) Vehicle Front Walk"),
		_T("(03) Vehicle Front Run"),
		_T("(04) Vehicle Front Boost"),
		_T("(05) Vehicle Front Death"),
		_T("(06) Vehicle Back Board"),
		_T("(07) Vehicle Back Stay"),
		_T("(08) Vehicle Back Walk"),
		_T("(09) Vehicle Back Run"),
		_T("(10) Vehicle Back Boost"),
		_T("(11) Vehicle Back Death"),
		_T("SUB_12"),
		_T("SUB_13"),
		_T("SUB_14"),
		_T("SUB_15"),
		_T("SUB_16"),
		_T("SUB_17"),
		_T("SUB_18"),
		_T("SUB_19"),
		_T("SUB_20"),
		_T("SUB_21"),
		_T("SUB_22"),
		_T("SUB_23"),
		_T("SUB_24"),
		_T("SUB_25"),
		_T("SUB_26"),
		_T("SUB_27"),
		_T("SUB_28"),
		_T("SUB_29"),
		_T("SUB_30"),
		_T("SUB_31"),
		_T("SUB_32"),
		_T("SUB_33"),
		_T("SUB_34"),
		_T("SUB_35"),
		_T("SUB_36"),
		_T("SUB_37"),
		_T("SUB_38"),
		_T("SUB_39"),
		_T("SUB_40"),
		_T("SUB_41"),
		_T("SUB_42"),
		_T("SUB_43"),
		_T("SUB_44"),
		_T("SUB_45"),
		_T("SUB_46"),
		_T("SUB_47"),
		_T("SUB_48"),
		_T("SUB_49"),
		_T("SUB_50"),
		_T("SUB_51"),
		_T("SUB_52"),
		_T("SUB_53"),
		_T("SUB_54"),
		_T("SUB_55"),
		_T("SUB_56"),
		_T("SUB_57"),
		_T("SUB_58"),
		_T("SUB_59"),
		_T("SUB_60"),
		_T("SUB_61"),
		_T("SUB_62"),
		_T("SUB_63"),
		_T("SUB_64"),
		_T("SUB_65"),
		_T("SUB_66"),
		_T("SUB_67"),
		_T("SUB_68"),
		_T("SUB_69"),
		_T("SUB_70"),
		_T("SUB_71"),
		_T("SUB_72"),
		_T("SUB_73"),
		_T("SUB_74"),
		_T("SUB_75"),
		_T("SUB_76"),
		_T("SUB_77"),
		_T("SUB_78"),
		_T("SUB_79"),
		_T("SUB_80"),
		_T("SUB_81"),
		_T("SUB_82"),
		_T("SUB_83"),
		_T("SUB_84"),
		_T("SUB_85"),
		_T("SUB_86"),
		_T("SUB_87"),
		_T("SUB_88"),
		_T("SUB_89"),
		_T("SUB_90"),
		_T("SUB_91"),
		_T("SUB_92"),
		_T("SUB_93"),
		_T("SUB_94"),
		_T("SUB_95"),
		_T("SUB_96"),
		_T("SUB_97"),
		_T("SUB_98"),
		_T("SUB_99")
	};

	TSTRING STRIKE_EFF[EMSF_SIZE] =
	{
		_T("NULL"),
		_T("SELFBODY"),
		_T("TARGET")
	};
};

CRITICAL_SECTION		DxSkinAniMan::m_criticalCONT;

DxSkinAniMan& DxSkinAniMan::GetInstance()
{
	static DxSkinAniMan Instance;
	return Instance;
}

DxSkinAniMan::DxSkinAniMan(void) :
	m_dwBinFileThreadID(0),
	m_hBinFileThread(NULL)
{
	memset( m_szPath, 0, sizeof(char)*MAX_PATH );

	InitializeCriticalSection(&m_criticalCONT);
	InitializeCriticalSection(&m_criticalLOAD);
}

DxSkinAniMan::~DxSkinAniMan(void)
{
	DeleteCriticalSection(&m_criticalLOAD);
	DeleteCriticalSection(&m_criticalCONT);
}

void DxSkinAniMan::OneTimeSceneInit ( char* szPath )
{
	StringCchCopy( m_szPath, MAX_PATH, szPath );
}

void DxSkinAniMan::CleanUp ()
{
	EnterCriticalSection(&m_criticalCONT);
	{
		if ( !m_mapAnimCont.empty() )
		{
			std::for_each ( m_mapAnimCont.begin(), m_mapAnimCont.end(), std_afunc::DeleteMapObject() );
			m_mapAnimCont.clear ();
		}
	}
	LeaveCriticalSection(&m_criticalCONT);
}

SAnimContainer* DxSkinAniMan::FindAnim ( const char *szFile )
{
	SAnimContainer* pAnimCon = NULL;

	EnterCriticalSection(&m_criticalCONT);
	{
		ANIMCONTMAP_ITER iter = m_mapAnimCont.find ( TSTRING(szFile) );
		if ( iter!=m_mapAnimCont.end() )	pAnimCon = iter->second;
	}
	LeaveCriticalSection(&m_criticalCONT);

	return pAnimCon;
}

bool DxSkinAniMan::SetBinAnim ( const char *szFileName, SAnimContainer::SANILIST &listANI, SAnimContainer::SANILIST &listANIUPBODY )
{
	bool bOK(false);

	EnterCriticalSection(&m_criticalCONT);
	{
		ANIMCONTMAP_ITER iter = m_mapAnimCont.find ( TSTRING(szFileName) );
		if ( iter!=m_mapAnimCont.end() )
		{
			bOK = true;
			SAnimContainer* pCONTAINER = (*iter).second;

			pCONTAINER->SetAnimFromList ( listANI, listANIUPBODY );
		}
	}
	LeaveCriticalSection(&m_criticalCONT);


	return bOK;
}

SAnimContainer* DxSkinAniMan::LoadAnimContainer ( const TCHAR *_szFileName, const TCHAR *_szSkeleton, LPDIRECT3DDEVICEQ pd3dDevice, bool bREPRESH /*=false*/ )
{
	HRESULT hr;
	GASSERT(_szFileName);
	GASSERT(_szSkeleton);

	CString strFileName(_szFileName);
	strFileName.MakeLower();

	CString strSkeleton(_szSkeleton);
	strSkeleton.MakeLower();

	//	Note : 이미 로드된 경우.
	//
	SAnimContainer* pAnim;
	pAnim = FindAnim ( strFileName.GetString() );
	if ( pAnim )
	{
		pAnim->AddRef();
		return pAnim;
	}

	//CDebugSet::ToListView ( "SAnimContainer '%s' Load", strFileName.GetString() );

	//	Note : 설정 파일 읽기 시도.
	//
	bool bValidCfgFile(true);
	CString strCfgFile;
	STRUTIL::ChangeExt ( strFileName.GetString(), _T(".x"), strCfgFile, _T(".cfg") );

	SANIMCONINFO AnimInfo;
	if ( AnimInfo.LoadFile ( strCfgFile.GetString(), pd3dDevice ) )
	{
		CString strINFO_Skeleton = AnimInfo.m_szSkeletion;
		strINFO_Skeleton.MakeLower();
		if ( strINFO_Skeleton!=strSkeleton )
		{
			CDebugSet::ToLogFile ( _T("본이름 불일치 : %s, %s, %s"), strCfgFile.GetString(), strINFO_Skeleton.GetString(), _szSkeleton );
		}
	}
	else
	{
		bValidCfgFile = false;
		StringCchCopy( AnimInfo.m_szName, ACF_SZNAME, strFileName.GetString() );
		StringCchCopy( AnimInfo.m_szSkeletion, ACF_SZNAME, strSkeleton.GetString() );
	}

	//	Note : 에니메이션 로드.
	//
	pAnim = new SAnimContainer;
	hr = pAnim->SetInfo ( AnimInfo, pd3dDevice );
	if ( FAILED(hr) )
	{
		SAFE_DELETE(pAnim);
		return NULL;
	}

	hr = pAnim->CreateAnimationData ( AnimInfo, pd3dDevice, bREPRESH );
	if ( FAILED(hr) )
	{
		SAFE_DELETE(pAnim);
		return NULL;
	}

	//	Note : if cfg file not created
	//
	if ( !bValidCfgFile )
	{
		pAnim->SANIMCONINFO::SaveFile(strCfgFile.GetString());
	}

	//	Note : 리스트에 삽입.
	//
	EnterCriticalSection(&m_criticalCONT);
	{
		m_mapAnimCont[TSTRING(pAnim->m_szName)] = pAnim;
	}
	LeaveCriticalSection(&m_criticalCONT);

	return pAnim;
}

BOOL DxSkinAniMan::SaveAnimConInfo ( TCHAR *szFileName )
{
	GASSERT(szFileName);

	//	Note : 이미 로드된 경우.
	//
	SAnimContainer* pAnim;
	pAnim = FindAnim ( szFileName );
	if ( !pAnim )	return FALSE;

	CString strCfgFile;
	STRUTIL::ChangeExt ( szFileName, ".x", strCfgFile, ".cfg" );
	
	pAnim->SaveFile ( strCfgFile.GetString() );

	return TRUE;
}

BOOL DxSkinAniMan::PreLoad ( TCHAR* szAniContList, LPDIRECT3DDEVICEQ pd3dDevice )
{
	CString strPath;
	strPath = GetPath();
	strPath += szAniContList;	

//	float __fOldTime = DXUtil_Timer ( TIMER_GETAPPTIME );

	CStringFile StrFile;
	if ( !StrFile.Open ( strPath.GetString() ) )	return FALSE;

	STRUTIL::ClearSeparator ();
	STRUTIL::RegisterSeparator ( "\t" );
	STRUTIL::RegisterSeparator ( " " );
	STRUTIL::RegisterSeparator ( "," );

	CString strLine;
	while ( StrFile.GetNextLine(strLine) )
	{
		if ( strLine.GetLength() == 0 )		continue;

		CStringArray *pStrArray = new CStringArray;
		STRUTIL::StringSeparate ( strLine, *pStrArray );

		if ( pStrArray->GetSize() != 2 )
		{
			SAFE_DELETE(pStrArray);
			continue;
		}

		CString strAniCont = pStrArray->GetAt(0);
		CString strBone = pStrArray->GetAt(1);

		strAniCont.MakeLower();
		strBone.MakeLower();

		SAnimContainer* pAnimCont = LoadAnimContainer ( strAniCont.GetString(), strBone.GetString(), pd3dDevice );
		if ( !pAnimCont )
		{
			CString strMessage;
			strMessage.Format (_T("DxSkinAniMan::PreLoad() [%s] LoadAnimContainer Failed."),
				               strAniCont);
			MessageBox (NULL, strMessage, _T("ERROR"), MB_OK);

			SAFE_DELETE(pStrArray);
			continue;
		}

		m_mapPreLoadAnimCont.insert (std::make_pair(TSTRING(strAniCont.GetString()), pAnimCont));

		SAFE_DELETE(pStrArray);
	};

//	float __fNowTime = DXUtil_Timer ( TIMER_GETAPPTIME ) - __fOldTime;
//	CDebugSet::ToLogFile ( "PreLoad : %f", __fNowTime );

	return TRUE;
}

HRESULT DxSkinAniMan::DoInterimClean ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	ANIMCONTLIST listInterimClean;
	
	//	Note : 제거할 리스트 작성.
	//
	EnterCriticalSection(&m_criticalCONT);
	{
		ANIMCONTMAP_ITER found;
		ANIMCONTMAP_ITER iter = m_mapAnimCont.begin();
		ANIMCONTMAP_ITER iter_end = m_mapAnimCont.end();
		for ( ; iter!=iter_end; ++iter )
		{
			//	Note : 사전 로딩 리스트에 있는 파일 제외.
			//
			found = m_mapPreLoadAnimCont.find ( (*iter).first );
			if ( found!=m_mapPreLoadAnimCont.end() )	continue;

			//	제거 리스트에 등록.
			listInterimClean.push_back ( (*iter).second );
		}
	}
	LeaveCriticalSection(&m_criticalCONT);

	//	Note : 데이터 정리.
	//
	EnterCriticalSection(&m_criticalCONT);
	{
		ANIMCONTLIST_ITER iter = listInterimClean.begin();
		ANIMCONTLIST_ITER iter_end = listInterimClean.end();
		for ( ; iter!=iter_end; ++iter )
		{
			SAnimContainer* pAnimCont = (*iter);
			TSTRING strname = pAnimCont->m_szName;

			//	데이터 삭제.
			SAFE_DELETE(pAnimCont);

			//	로드 파일 리스트에서 제거.
			ANIMCONTMAP_ITER iter_del = m_mapAnimCont.find ( strname );
			if ( iter_del!=m_mapAnimCont.end() )	m_mapAnimCont.erase ( iter_del );
		}
	}
	LeaveCriticalSection(&m_criticalCONT);

	return S_OK;
}



