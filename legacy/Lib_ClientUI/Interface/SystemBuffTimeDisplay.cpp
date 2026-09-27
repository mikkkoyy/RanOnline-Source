#include "StdAfx.h"
#include "SystemBuffTimeDisplay.h"
#include "GLGaeaClient.h"
#include "SkillTimeUnit.h"
#include "GLSkill.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSystemBuffTimeDisplay::CSystemBuffTimeDisplay () :
	m_NEWID ( SKILLTIME_UNIT )
{
}

CSystemBuffTimeDisplay::~CSystemBuffTimeDisplay ()
{
}

void CSystemBuffTimeDisplay::CreateSubControl ()
{	
	const CString strKeyword = "SYSTEMBUFF_TIME_UNIT";
	CString strCombine;

	for ( int i = 0; i < SYSTEM_BUFF_SIZE; i++ )
	{
		strCombine.Format ( "%s%d", strKeyword, i );
		CUIControl* pDummyControl = new CUIControl;
		pDummyControl->CreateSub ( this, strCombine.GetString(), UI_FLAG_DEFAULT, SKILLTIME_DUMMY_BASE + i );
		RegisterControl ( pDummyControl );

		m_pSkillUnitDummy[i] = pDummyControl;
	}
}

UIGUID CSystemBuffTimeDisplay::GET_EXIST_PLAY_SKILL_CONTROL ( const DWORD& dwSkillID )
{
	SKILLTIME_DISPLAY_LIST_ITER iter = m_SkillPlayList.begin ();
	SKILLTIME_DISPLAY_LIST_ITER iter_end = m_SkillPlayList.end ();
	for ( ; iter != iter_end; ++iter )
	{
		const SKILLTIME_PAIR& pair = (*iter);
		const DWORD& dwID = pair.first;
		const UIGUID& cID = pair.second;
		if ( dwID == dwSkillID ) return cID;
	}

	return NO_ID;
}

void CSystemBuffTimeDisplay::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	GLCharacter* const pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pCharacter ) return;

	//	삭제
	if ( !m_SkillPlayList.empty () )
	{
		SKILLTIME_DISPLAY_LIST_ITER iter = m_SkillPlayList.begin ();
		SKILLTIME_DISPLAY_LIST_ITER iter_end = m_SkillPlayList.end ();
		
		for ( ; iter != iter_end; )
		{
			const SKILLTIME_PAIR& skill_pair = (*iter);
			const DWORD& dwID = skill_pair.first;
			const UIGUID& cID = skill_pair.second;

			//	찾기			
			SSYSTEM_BUFF ssystem_buff;
			if ( !GET_PLAY_SKILLFACT ( dwID, ssystem_buff ) )
			{				
				DeleteControl ( cID, 0 );
				iter = m_SkillPlayList.erase ( iter );
			}
			else ++iter;
		}
	}

	//	등록
	{		
		for ( int i = 0; i < SYSTEM_BUFF_SIZE; i++ )
		{
			const SSYSTEM_BUFF& ssystem_buff = pCharacter->m_sSYSTEM_BUFF[i];
			const DWORD& dwID = ssystem_buff.sNATIVEID.dwID;
			ssystem_buff.wLEVEL;

			if ( dwID == NATIVEID_NULL().dwID ) continue;

			UIGUID cID = GET_EXIST_PLAY_SKILL_CONTROL ( dwID );
			if ( cID == NO_ID )	NEW_PLAY_SKILL ( dwID, ssystem_buff.wLEVEL );
		}
	}

	if ( m_SkillPlayList.empty () ) return ;

	//	업데이트
	int nPLAY_SKILL = 0;
	SKILLTIME_DISPLAY_LIST_ITER iter = m_SkillPlayList.begin ();
	SKILLTIME_DISPLAY_LIST_ITER iter_end = m_SkillPlayList.end ();

	for ( ; iter != iter_end; ++iter )
	{
		const SKILLTIME_PAIR& skill_pair = (*iter);
		const DWORD& dwID = skill_pair.first;
		const UIGUID& cID = skill_pair.second;			

		CSkillTimeUnit* pControl = (CSkillTimeUnit*) FindControl ( cID );
		if ( !pControl )
		{
			GASSERT ( 0 && "컨트롤 찾기 실패" );
			continue;
		}

		//	찾기			
		SSYSTEM_BUFF ssystem_buff;
		if ( !GET_PLAY_SKILLFACT ( dwID, ssystem_buff ) )
		{
			GASSERT ( 0 && "스킬 데이타 찾기 실패" );
			continue;
		}

		pControl->SetGlobalPos ( m_pSkillUnitDummy[nPLAY_SKILL]->GetGlobalPos () );
		pControl->SetLeftTime ( 1.0f );

		nPLAY_SKILL++;
	}

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

bool CSystemBuffTimeDisplay::GET_PLAY_SKILLFACT ( const DWORD& dwSkillID, SSYSTEM_BUFF& ssystem_buff )
{
	GLCharacter* const pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pCharacter ) return false;

	for ( int i = 0; i < SYSTEM_BUFF_SIZE; i++ )
	{
		const SSYSTEM_BUFF& sSkillFactIter = pCharacter->m_sSYSTEM_BUFF[i];
		if ( sSkillFactIter.sNATIVEID == dwSkillID )
		{
			ssystem_buff = sSkillFactIter;
			return true;
		}
	}

	return false;
}

HRESULT CSystemBuffTimeDisplay::InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	m_pd3dDevice = pd3dDevice;
	return CUIGroup::InitDeviceObjects ( pd3dDevice );
}

bool CSystemBuffTimeDisplay::NEW_PLAY_SKILL ( const DWORD& dwSkillID, const WORD wLEVEL )
{
	m_NEWID++;
	if ( SKILLTIME_UNIT_END <= m_NEWID ) m_NEWID = SKILLTIME_UNIT;

	float fLifeTime = 1.0f;
	
	SNATIVEID sNativeID ( dwSkillID );
	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sNativeID );
	if ( !pSkill ) return false;

	CSkillTimeUnit* pSkillTimeUnit = new CSkillTimeUnit;
	pSkillTimeUnit->CreateSub ( this, "SKILLTIME_UNIT", UI_FLAG_DEFAULT, m_NEWID );
	pSkillTimeUnit->CreateSubControl ();
	pSkillTimeUnit->InitDeviceObjects ( m_pd3dDevice );
	pSkillTimeUnit->RestoreDeviceObjects ( m_pd3dDevice );	
	pSkillTimeUnit->SetSkill ( dwSkillID, fLifeTime, pSkill->GetName() );
	RegisterControl ( pSkillTimeUnit );	

	SKILLTIME_PAIR pair (dwSkillID,m_NEWID);
	m_SkillPlayList.push_back ( pair );

	return true;
}
