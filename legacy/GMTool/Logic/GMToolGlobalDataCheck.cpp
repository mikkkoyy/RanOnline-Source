
#include "stdafx.h"
#include "GMToolGlobal.h"
#include "EtcFunction.h"
#include "GLItemMan.h"
#include "GLSkill.h"
#include "GLCrowData.h"

namespace GMTOOL_GLOBAL
{
	GLMapList gmtoolMapList;

	void	DataCheckMap( CDialog *pDlg, int nIDM, int nIDS, int nIDN )
	{
		if ( !pDlg ){
			return;
		}

		WORD wMID = GetWin_Num_int( pDlg, nIDM );
		WORD wSID = GetWin_Num_int( pDlg, nIDS );

		std::string strName = "Unknown Map";
		SMAPNODE* pNode = gmtoolMapList.FindMapNode( SNATIVEID( wMID, wSID ) );
		if ( pNode ){
			strName = pNode->strMapName.c_str();
		}

		SetWin_Text( pDlg, nIDN, strName.c_str() );
	}

	void	DataCheckItem( CDialog *pDlg, int nIDM, int nIDS, int nIDN )
	{
		if ( !pDlg ){
			return;
		}

		int wMID = GetWin_Num_int( pDlg, nIDM );
		int wSID = GetWin_Num_int( pDlg, nIDS );

		std::string strName = "Unknown Item";
		SITEM* pItem = GLItemMan::GetInstance().GetItem( wMID, wSID );
		if ( pItem ){
			strName = pItem->GetName();
		}

		SetWin_Text( pDlg, nIDN, strName.c_str() );
	}

	void	DataCheckSkill( CDialog *pDlg, int nIDM, int nIDS, int nIDN )
	{
		if ( !pDlg ){
			return;
		}

		int wMID = GetWin_Num_int( pDlg, nIDM );
		int wSID = GetWin_Num_int( pDlg, nIDS );

		std::string strName = "Unknown Skill";
		PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( wMID, wSID );
		if ( pSkill ){
			strName = pSkill->GetName();
		}

		SetWin_Text( pDlg, nIDN, strName.c_str() );
	}

	void	DataCheckCrow( CDialog *pDlg, int nIDM, int nIDS, int nIDN )
	{
		if ( !pDlg ){
			return;
		}

		int wMID = GetWin_Num_int( pDlg, nIDM );
		int wSID = GetWin_Num_int( pDlg, nIDS );

		std::string strName = "Unknown Crow";
		SCROWDATA* pCrow = GLCrowDataMan::GetInstance().GetCrowData( wMID, wSID );
		if ( pCrow ){
			strName = pCrow->GetName();
		}

		SetWin_Text( pDlg, nIDN, strName.c_str() );
	}

	void	SkillLevelCheck( WORD& wLevel, SNATIVEID sSkillID )
	{
		PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( sSkillID );
		if ( !pSKILL ){
			return;
		}

		if ( wLevel >= SKILL::MAX_LEVEL ){
			wLevel = SKILL::MAX_LEVEL-1;
		}

		if ( wLevel >= pSKILL->m_sBASIC.dwMAXLEVEL ){
			wLevel = (WORD)pSKILL->m_sBASIC.dwMAXLEVEL-1;
		}
	}
};

