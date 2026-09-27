#include "StdAfx.h"
#include "./DxLobyStage.h"

#include "DxGlobalStage.h"
#include "../Lib_Engine/DxCommon/DxInputDevice.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void DxLobyStage::DelChar ( int nCharID )
{
	//	Note : 기존 캐릭터 정보 수신시.
	int nIndex = -1;
	for ( int i=0; i<m_nChaSNum; i++ )
	{
		if ( m_nChaIDs[i]== nCharID )	nIndex = i;
	}
	if ( nIndex == -1 )		return;

	//	Note : 캐릭터 선택을 취소.
	m_dwCharSelect = MAX_SELECTCHAR;

	//	Note : 삭제된걸 지우고 재정렬.
	for (int i=nIndex+1; i<m_nChaSNum; i++ )
	{
		m_nChaIDs[i-1] = m_nChaIDs[i];
		m_CharInfo[i-1] = m_CharInfo[i];
	}
	m_nChaSNum--;

	//	Note : 디스플레이 케릭터 형상 갱신.
	for( int i=0; i<MAX_SELECTCHAR; i++ )
	{
		m_CharSham[i].ResetCharInfo ();
		m_CharSham[i].SetSelect ( FALSE );

		InitRenderSkip();
	}

	if( m_nCharStart >= m_nChaSNum )
	{
		ShiftLBDown();
		return;
	}

	for( int i=m_nCharStart, j=0; i<m_nChaSNum; i++, j++ )
	{
		if( j >= MAX_SELECTCHAR ) 
			break;

		m_CharSham[j].SetCharInfo( m_CharInfo[i], FALSE );
	}
}

BOOL DxLobyStage::ShiftRightChar()
{
	if( (m_nCharStart+MAX_SELECTCHAR) < m_nChaSNum )
	{
		m_nCharStart += MAX_SELECTCHAR;
		return TRUE;
	}

	return FALSE;
}

BOOL DxLobyStage::ShiftLeftChar()
{
	if( (m_nCharStart-MAX_SELECTCHAR) >= 0 )
	{
		m_nCharStart -= MAX_SELECTCHAR;
		return TRUE;
	}

	return FALSE;
}

void DxLobyStage::ShiftCharInfo()
{
	for ( int i=m_nCharStart, j=0; i<m_nChaSNum; i++, j++ )
	{
		if( j >= MAX_SELECTCHAR ) 
			break;

		m_CharSham[j].SetCharInfo( m_CharInfo[i], FALSE );
	}
}

void DxLobyStage::ShiftChar( float fElapsedTime )
{
	DxInputDevice &dxInputDev = DxInputDevice::GetInstance();

	if( !m_bShiftButton )
	{
		if( (dxInputDev.GetKeyState( DIK_LEFT )&DXKEY_DOWN) || m_bShiftLBDown )
		{
			if( ShiftLeftChar() )
			{
				m_bShiftButton = TRUE;
			}
		}
		else if( (dxInputDev.GetKeyState( DIK_RIGHT )&DXKEY_DOWN) || m_bShiftRBDown )
		{
			if( ShiftRightChar() )
			{
				m_bShiftButton = TRUE;
			}
		}

		if( m_bShiftButton )
		{
			//	Note : 캐릭터 선택을 취소.
			m_dwCharSelect = MAX_SELECTCHAR;

			for ( int i=0; i<MAX_SELECTCHAR; i++ )
			{
				m_CharSham[i].ResetCharInfo ();
				m_CharSham[i].SetSelect ( FALSE );

				InitRenderSkip();
			}
		}
	}

	if( m_bShiftButton )
	{
		m_fShiftTime += fElapsedTime;

		if( m_fShiftTime > 0.5f )
		{
			ShiftCharInfo();

			m_fShiftTime = 0;
			m_bShiftButton = FALSE;
			m_bShiftLBDown = FALSE;
			m_bShiftRBDown = FALSE;
		}
	}
}

void  DxLobyStage::SelectChar( DWORD dwCharID )
{
	//0 3 6 9 12 15

	SCHARINFO_LOBBY* pCharInfo = GetSelectCharInfo();
	if ( pCharInfo && pCharInfo->m_dwCharID != 0
		&& pCharInfo->m_dwCharID == dwCharID ){
		return;
	}

	DWORD dwSelectedNum = MAX_SERVERCHAR;
	for( int i=0; i<m_nChaSNum; ++ i )
	{
		if ( m_CharInfo[i].m_dwCharID == 0 )	continue;
		if ( m_CharInfo[i].m_dwCharID == dwCharID )
			dwSelectedNum = i;
	}

	if ( dwSelectedNum >= MAX_SERVERCHAR )	return;

	DWORD dwPage = MAX_SERVERCHAR;
	if ( dwSelectedNum == 0 || dwSelectedNum == 1 || dwSelectedNum == 2 ){
		dwPage = 0;
	}else if ( dwSelectedNum == 3 || dwSelectedNum == 4 || dwSelectedNum == 5 ){
		dwPage = 1;
	}else if ( dwSelectedNum == 6 || dwSelectedNum == 7 || dwSelectedNum == 8 ){
		dwPage = 2;
	}else if ( dwSelectedNum == 9 || dwSelectedNum == 10 || dwSelectedNum == 11 ){
		dwPage = 3;
	}else if ( dwSelectedNum == 12 || dwSelectedNum == 13 || dwSelectedNum == 14 ){
		dwPage = 4;
	}else if ( dwSelectedNum == 15 ){
		dwPage = 5;
	}

	if ( dwPage >= MAX_SERVERCHAR )	return;

	int nCharStart = (int)dwPage * MAX_SELECTCHAR;
	if ( nCharStart < 0 || nCharStart >= MAX_SERVERCHAR )	return;

	DWORD dwCharSelect = (DWORD)dwSelectedNum - nCharStart;

	if ( dwCharSelect >= MAX_SELECTCHAR )	return;
	if ( (dwCharSelect+nCharStart) >= MAX_SERVERCHAR )	return;

	if ( nCharStart != m_nCharStart )
	{
		m_nCharStart = nCharStart;

		m_dwCharSelect = MAX_SELECTCHAR;
		for ( int i=0; i<MAX_SELECTCHAR; i++ )
		{
			m_CharSham[i].ResetCharInfo ();
			m_CharSham[i].SetSelect ( FALSE );
			InitRenderSkip();
		}

		m_bShiftButton = TRUE;
	}

	DxGlobalStage::GetInstance().GetGameStage()->SetSelectChar ( &m_CharInfo[dwCharSelect+m_nCharStart] );

	if( m_dwCharSelect != dwCharSelect )
	{
		m_dwCharSelect = dwCharSelect;

		for ( int i=0; i<MAX_SELECTCHAR; i++ )	
			m_CharSham[i].SetSelect ( FALSE, TRUE );
		m_CharSham[m_dwCharSelect].SetSelect ( TRUE );
	}
}