#include "StdAfx.h"
#include "./DxLobyStage.h"

#include "DxGlobalStage.h"
#include "../Lib_ClientUI/Interface/OuterInterface.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


WORD DxLobyStage::ShiftCharClass ( EMSHIFT emShift, WORD wClassMax )
{
	WORD nRet(wClassMax);

	switch ( emShift )
	{
	case EM_PREV:
		{
			WORD wIndex = (WORD)m_sCharNewInfo.m_emIndex;
			if ( (WORD)m_sCharNewInfo.m_emIndex > 0 )			wIndex--;
			else												wIndex = wClassMax;
			SelCharClass ( wIndex );

			nRet = wIndex;
		}
		break;

	case EM_NEXT:
		{
			WORD wIndex = (WORD)m_sCharNewInfo.m_emIndex;
			if ( (WORD)m_sCharNewInfo.m_emIndex < wClassMax )	wIndex++;
			else												wIndex = 0;
			SelCharClass ( wIndex );

			nRet = wIndex;
		}
		break;
	};

	return nRet;
}

void DxLobyStage::ShiftCharSchool ( EMSHIFT emShift )
{
	switch ( emShift )
	{
	case EM_PREV:
		{
			WORD wIndex = m_sCharNewInfo.m_wSchool;
			if ( wIndex>0 )										wIndex--;
			else												wIndex = (GLCONST_CHAR::wSCHOOLNUM-1);
			SelCharSchool ( wIndex );
		}
		break;

	case EM_NEXT:
		{
			WORD wIndex = m_sCharNewInfo.m_wSchool;
			if ( (WORD)wIndex < (GLCONST_CHAR::wSCHOOLNUM-1) )	wIndex++;
			else												wIndex = 0;
			SelCharSchool ( wIndex );
		}
		break;
	};
}

void DxLobyStage::ShiftCharFace ( EMSHIFT emShift )
{
	const GLCONST_CHARCLASS &sCONST = GLCONST_CHAR::cCONSTCLASS[m_sCharNewInfo.m_emIndex];

	switch ( emShift )
	{
	case EM_PREV:
		{
			WORD wIndex = (WORD)m_sCharNewInfo.m_wFace;
			if ( wIndex > 0 )					wIndex--;
			else								wIndex = (WORD) (sCONST.dwHEADNUM_SELECT-1);

			SelCharFace ( wIndex );
		}
		break;

	case EM_NEXT:
		{
			WORD wIndex = (WORD)m_sCharNewInfo.m_wFace;
			if ( wIndex < WORD(sCONST.dwHEADNUM_SELECT-1) )	wIndex++;
			else											wIndex = 0;

			SelCharFace ( wIndex );
		}
		break;
	};
}

void DxLobyStage::ShiftCharHair ( EMSHIFT emShift )
{
	const GLCONST_CHARCLASS &sCONST = GLCONST_CHAR::cCONSTCLASS[m_sCharNewInfo.m_emIndex];

	switch ( emShift )
	{
	case EM_PREV:
		{
			WORD wIndex = (WORD)m_sCharNewInfo.m_wHair;
			if ( wIndex > 0 )					wIndex--;
			else								wIndex = (WORD) (sCONST.dwHAIRNUM_SELECT-1);

			WORD wHairColor = HAIRCOLOR::GetHairColor ( m_sCharNewInfo.m_emIndex, wIndex );

			SelCharHair ( wIndex );
			SelCharHairColor ( wHairColor );
		}
		break;

	case EM_NEXT:
		{
			WORD wIndex = (WORD)m_sCharNewInfo.m_wHair;
			if ( wIndex < WORD(sCONST.dwHAIRNUM_SELECT-1) )	wIndex++;
			else											wIndex = 0;

			WORD wHairColor = HAIRCOLOR::GetHairColor ( m_sCharNewInfo.m_emIndex, wIndex );

			SelCharHair ( wIndex );
			SelCharHairColor ( wHairColor );
		}
		break;
	};
}

void DxLobyStage::SelCharClass ( WORD wIndex )
{
	//	GASSERT(wIndex<(nClassMax+1));

	//	케릭터 선택.
	m_sCharNewInfo.m_emIndex = (EMCHARINDEX) wIndex;

	m_sCharNewInfo.m_wHair = 0;
	m_sCharNewInfo.m_wFace = 0;
	m_sCharNewInfo.m_wHairColor = HAIRCOLOR::GetHairColor ( m_sCharNewInfo.m_emIndex, 0 );

	// 성별 설정
	if ( m_sCharNewInfo.m_emIndex != GLCI_NUM_8CLASS ){
		m_sCharNewInfo.m_wSex = (WORD)CharIndexToGender( m_sCharNewInfo.m_emIndex );
	}

	m_sCharNewInfo.m_fScaleRange = 1.0f;
}

void DxLobyStage::SelCharSchool ( WORD wIndex )
{
	//	학교 선택.
	GASSERT(wIndex<GLCONST_CHAR::wSCHOOLNUM);
	m_sCharNewInfo.m_wSchool = (WORD)wIndex;

	//	선택된 케릭터 클레스 리셋.
	SelCharClass( GLCI_NUM_8CLASS );

	m_sCharNewInfo.m_wHair = 0;
	m_sCharNewInfo.m_wFace = 0;
	m_sCharNewInfo.m_fScaleRange = 1.0f;
}

void  DxLobyStage::SetStyleStep( bool bStyle )
{
	m_bStyle = bStyle;

	if ( m_bStyle )
	{
		SCHARINFO_LOBBY sCharInfo;
		sCharInfo.m_emClass = CharIndexToClass( m_sCharNewInfo.m_emIndex ); // 직업
		sCharInfo.m_wSchool = m_sCharNewInfo.m_wSchool; // 학원.
		sCharInfo.m_wHair = m_sCharNewInfo.m_wHair; // 헤어스타일
		sCharInfo.m_wFace = m_sCharNewInfo.m_wFace;
		sCharInfo.m_wHairColor = m_sCharNewInfo.m_wHairColor;
		sCharInfo.m_fScaleRange = m_sCharNewInfo.m_fScaleRange;

		SCHARDATA2& sCharData = GLCONST_CHAR::GET_CHAR_DATA2(m_sCharNewInfo.m_wSchool,m_sCharNewInfo.m_emIndex);

		sCharInfo.m_PutOnItems[SLOT_UPPER].Assign( sCharData.m_PutOnItems[SLOT_UPPER] );
		sCharInfo.m_PutOnItems[SLOT_LOWER].Assign( sCharData.m_PutOnItems[SLOT_LOWER] );
		sCharInfo.m_PutOnItems[SLOT_FOOT].Assign( sCharData.m_PutOnItems[SLOT_FOOT] );	
		sCharInfo.m_PutOnItems[SLOT_HAND].Assign( sCharData.m_PutOnItems[SLOT_HAND] );	
		sCharInfo.m_PutOnItems[SLOT_RHAND].Assign( sCharData.m_PutOnItems[SLOT_RHAND] );	
		sCharInfo.m_PutOnItems[SLOT_LHAND].Assign( sCharData.m_PutOnItems[SLOT_LHAND] );	
		sCharInfo.m_PutOnItems[SLOT_RHAND_S].Assign( sCharData.m_PutOnItems[SLOT_RHAND_S] );	
		sCharInfo.m_PutOnItems[SLOT_LHAND_S].Assign( sCharData.m_PutOnItems[SLOT_LHAND_S] );	

		m_NewCharSham.SetCharInfo( sCharInfo, FALSE );

		m_vDir = D3DXVECTOR3( 0.1f, 0.097f, 2.00f );

		InitRenderSkip();
	}
	else
	{
		m_NewCharSham.ResetCharInfo();
	}
}

void  DxLobyStage::RotateChar( bool bLeft )
{
	if ( bLeft )
	{
		m_vDir.x += 0.1f;
		m_vDir.z -= 0.05f;

		if ( m_vDir.x > 4.0f ) m_vDir.x = 4.0f;
		if ( m_vDir.z < 0.5f ) m_vDir.z = 0.5f;

	}
	else
	{
		m_vDir.x -= 0.1f;
		m_vDir.z -= 0.05f;

		if ( m_vDir.x < -4.0f ) m_vDir.x = -4.0f;
		if ( m_vDir.z < 0.5f ) m_vDir.z = 0.5f;
	}	
}
