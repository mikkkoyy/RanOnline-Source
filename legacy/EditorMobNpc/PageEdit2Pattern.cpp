#include "stdafx.h"
#include "EditorMobNpc.h"
#include "PageEdit2.h"
#include "EtcFunction.h"
#include "HLibDataConvert.h"


void CPageEdit2::InitPattern()
{
	m_listPattern.DeleteAllItems();

	if ( !m_pDummyCrow )	return;

	int nSIZE = (int)m_pDummyCrow->m_sAction.m_vecPatternList.size();

	m_listPattern.SetRedraw( FALSE );

	for ( int i=0; i<nSIZE; i++ )
	{
		SCROWPATTERN sPattern = m_pDummyCrow->m_sAction.m_vecPatternList[i];
		m_listPattern.InsertItem( i, _HLIB::cstring_dword(i).GetString() );
		m_listPattern.SetItemText( i, 1, COMMENT::szCROWACT_UP[sPattern.m_emActPattern].c_str() );
		m_listPattern.SetItemText( i, 2, _HLIB::cstring_floatg(sPattern.m_fPatternDNRate).GetString() );
		m_listPattern.SetItemText( i, 3, _HLIB::cstring_dword(sPattern.m_dwPatternAttackSet).GetString() );
	}

	m_listPattern.SetRedraw( TRUE );

	SetWin_Enable( this, IDC_COMBO_PATTERN_ACTION, FALSE );
	SetWin_Enable( this, IDC_EDIT_PATTERN_DNRATE, FALSE );
	SetWin_Enable( this, IDC_EDIT_PATTERN_ATTACKSET, FALSE );
	SetWin_Enable( this, IDC_BUTTON_PATTERN_SAVE, FALSE );

	SetWin_Num_int( this, IDC_EDIT_PATTERN_NUM, -1 );
	SetWin_Num_float( this, IDC_EDIT_PATTERN_DNRATE, 0.0f );
	SetWin_Num_int( this, IDC_EDIT_PATTERN_ATTACKSET, SCROWDATA::EMMAXATTACK );
	SetWin_Combo_Sel( this, IDC_COMBO_PATTERN_ACTION, 0 );
}

void CPageEdit2::OnBnClickedButtonPatternAdd()
{
	SetWin_Enable( this, IDC_COMBO_PATTERN_ACTION, TRUE );
	SetWin_Enable( this, IDC_EDIT_PATTERN_DNRATE, TRUE );
	SetWin_Enable( this, IDC_EDIT_PATTERN_ATTACKSET, TRUE );
	SetWin_Enable( this, IDC_BUTTON_PATTERN_SAVE, TRUE );

	SetWin_Num_int( this, IDC_EDIT_PATTERN_NUM, -1 );
	SetWin_Num_float( this, IDC_EDIT_PATTERN_DNRATE, 0.0f );
	SetWin_Num_int( this, IDC_EDIT_PATTERN_ATTACKSET, SCROWDATA::EMMAXATTACK );
	SetWin_Combo_Sel( this, IDC_COMBO_PATTERN_ACTION, 0 );
}

void CPageEdit2::OnBnClickedButtonPatternEdit()
{
	if ( !m_pDummyCrow )	return;

	int nSelect = m_listPattern.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	SetWin_Enable( this, IDC_COMBO_PATTERN_ACTION, TRUE );
	SetWin_Enable( this, IDC_EDIT_PATTERN_DNRATE, TRUE );
	SetWin_Enable( this, IDC_EDIT_PATTERN_ATTACKSET, TRUE );
	SetWin_Enable( this, IDC_BUTTON_PATTERN_SAVE, TRUE );

	SCROWPATTERN sPATTERN = m_pDummyCrow->m_sAction.m_vecPatternList[nSelect];
	SetWin_Num_int( this, IDC_EDIT_PATTERN_NUM, nSelect );
	SetWin_Num_float( this, IDC_EDIT_PATTERN_DNRATE, sPATTERN.m_fPatternDNRate );
	SetWin_Num_int( this, IDC_EDIT_PATTERN_ATTACKSET, sPATTERN.m_dwPatternAttackSet );
	SetWin_Combo_Sel( this, IDC_COMBO_PATTERN_ACTION, COMMENT::szCROWACT_UP[sPATTERN.m_emActPattern].c_str() );
}

void CPageEdit2::OnBnClickedButtonPatternDelete()
{
	if ( !m_pDummyCrow )	return;

	int nSelect = m_listPattern.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	if ( MessageBox ( "Do you want to delete Selected Pattern?", "WARNING", MB_YESNO ) == IDYES )
	{
		if ( m_pDummyCrow->m_sAction.m_vecPatternList.size() == 1 )
		{
			if ( MessageBox ( "Require 1 Remaining pattern!, Continue Delete?", "WARNING", MB_YESNO ) == IDNO )
				return;
		}

		m_pDummyCrow->m_sAction.m_vecPatternList.erase( m_pDummyCrow->m_sAction.m_vecPatternList.begin()+nSelect );
		InitPattern();
	}
}

void CPageEdit2::OnBnClickedButtonPatternSave()
{
	if ( !m_pDummyCrow )	return;

	int nEDITNUM = GetWin_Num_int( this, IDC_EDIT_PATTERN_NUM );
	EMCROWACT_UP emUP = static_cast<EMCROWACT_UP> ( GetWin_Combo_Sel( this, IDC_COMBO_PATTERN_ACTION ));
	float fDNRATE = GetWin_Num_float( this, IDC_EDIT_PATTERN_DNRATE );
	int nATTACKSET = GetWin_Num_int( this, IDC_EDIT_PATTERN_ATTACKSET );
	BOOL bEDIT = ( nEDITNUM >= 0 ); //-1 is add

	if ( nATTACKSET > SCROWDATA::EMMAXATTACK )
	{
		CString strTEMP;
		strTEMP.Format( "Maximum AttackSet is:%d ( Use:%d For Random Attack )", SCROWDATA::EMMAXATTACK, SCROWDATA::EMMAXATTACK );
		MessageBox( strTEMP.GetString() );
		return;
	}

	if ( nATTACKSET < 0 )
	{
		MessageBox( "Invalid AttackSet" );
		return;
	}

	int nSIZE = (int)m_pDummyCrow->m_sAction.m_vecPatternList.size();
	if ( !bEDIT && nSIZE >= SCROWACTION::PATTERNNUM )
	{
		CString strTEMP;
		strTEMP.Format( "Maximum Number of Pattern is:%d", SCROWACTION::PATTERNNUM );
		MessageBox( strTEMP.GetString() );
		InitPattern();
		return;
	}


	if ( fDNRATE > 100.0f )
	{
		CString strTEMP;
		strTEMP.Format( "Maximum DN Rate is:%g", 100.0f );
		MessageBox( strTEMP.GetString() );
		return;
	}

	if ( bEDIT )
	{
		SCROWPATTERN &sPATTERN = m_pDummyCrow->m_sAction.m_vecPatternList[nEDITNUM];
		sPATTERN.m_emActPattern = emUP;
		sPATTERN.m_dwPatternAttackSet = nATTACKSET;
		sPATTERN.m_fPatternDNRate = fDNRATE;
		InitPattern();
	}else{
		SCROWPATTERN sPATTERN;
		sPATTERN.m_emActPattern = emUP;
		sPATTERN.m_dwPatternAttackSet = nATTACKSET;
		sPATTERN.m_fPatternDNRate = fDNRATE;
		m_pDummyCrow->m_sAction.m_vecPatternList.push_back( sPATTERN );
		InitPattern();
	}
}