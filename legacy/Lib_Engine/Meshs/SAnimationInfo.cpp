#include "stdafx.h"
#include "SAnimationInfo.h"

#include "../Common/stlfunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SANIMCONINFO::SANIMCONINFO() :
	m_dwFlag(ACF_LOOP|ACF_NEWINFO),
	m_dwSTime(0),
	m_dwETime(0),
	m_dwETimeOrig(0),
	m_UNITTIME(0),
	m_MainType(AN_GUARD_N),
	m_SubType(AN_SUB_NONE),
	m_wDivCount(0),
	m_wStrikeCount(0),
	m_pAniScale(NULL)
{
	memset( m_szName, 0, sizeof(TCHAR)*ACF_SZNAME );
	memset( m_szSkeletion, 0, sizeof(TCHAR)*ACF_SZNAME );
	memset ( m_wDivFrame, 0, sizeof(WORD)*ACF_DIV );
	m_pAniScale = new DxAniScale;
}

SANIMCONINFO::~SANIMCONINFO ()
{
	CleanUp();

	SAFE_DELETE( m_pAniScale );
}

void SANIMCONINFO::CleanUp()
{
	std::for_each ( m_listEffAni.begin(), m_listEffAni.end(), std_afunc::DeleteObject() );
	m_listEffAni.clear();

	m_pAniScale->CleanUp();
}

SANIMCONINFO& SANIMCONINFO::operator = ( SANIMCONINFO &value )
{
	CleanUp();

	StringCchCopy( m_szName, ACF_SZNAME, value.m_szName );
	StringCchCopy( m_szSkeletion, ACF_SZNAME, value.m_szSkeletion );

	m_dwFlag = value.m_dwFlag;					//	기타 속성.
	m_dwSTime = value.m_dwSTime;					//	시작 시간.
	m_dwETime = value.m_dwETime;
	m_dwETimeOrig = value.m_dwETimeOrig;	//	끝 시간.
	m_UNITTIME = value.m_UNITTIME;					//	키단위 시간.

	m_MainType = (EMANI_MAINTYPE)value.m_MainType;
	m_SubType = (EMANI_SUBTYPE)value.m_SubType;

	m_wDivCount = value.m_wDivCount;				//	에니메이션 분할 키 갯수.
	for ( DWORD i=0;i<m_wDivCount; ++i )
	{
		m_wDivFrame[i] = value.m_wDivFrame[i];
	}

	m_wStrikeCount = value.m_wStrikeCount;
	for ( DWORD i=0;i<m_wStrikeCount; ++i )
	{
		m_sStrikeEff[i] = value.m_sStrikeEff[i];
	}

	m_ChaSoundData = value.m_ChaSoundData;

	EFFANILIST_ITER iter = value.m_listEffAni.begin();
	EFFANILIST_ITER iter_end = value.m_listEffAni.end();
	for ( ; iter!=iter_end; ++iter )
	{
		DxEffAniData* pEff = DxEffAniMan::GetInstance().CreateEffInstance ( (*iter)->GetTypeID() );
		if ( pEff )
		{
			pEff->SetProperty ( (*iter)->GetProperty() );
			pEff->SetEffAniData ( (*iter) );
			m_listEffAni.push_back ( pEff );
		}
	}

	*m_pAniScale = *value.m_pAniScale;

	return *this;
}