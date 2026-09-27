#include "stdafx.h"
#include "EditorQuest.h"
#include "PageQuestTree.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "DlgInput.h"

BOOL CPageQuestTree::AddQuest()
{
	CDlgInput dlg;
	
	if ( dlg.DoModal() != IDOK ) return FALSE;

	DWORD dwMID = dlg.m_nNEWID;
	std::string strNAME = dlg.m_strNEWNAME.GetString();
	
	QUESTNODE sQuestNode;
	sQuestNode.pQUEST = new GLQUEST;
	sQuestNode.pQUEST->m_sNID = dwMID;
	sQuestNode.strFILE = strNAME.c_str();

	GLQuestMan::GetInstance().Insert( dwMID, sQuestNode );
	GLQuestMan::MAPQUEST& m_mapQuestMap = GLQuestMan::GetInstance().GetMap();
	GLQuestMan::MAPQUEST_ITER iter_pos = m_mapQuestMap.find( dwMID );

	if ( iter_pos == m_mapQuestMap.end() ) return FALSE;;
	PQUESTNODE pQuestNode = (PQUESTNODE) &iter_pos->second;

	pQuestNode->pQUEST->SAVE( pQuestNode->strFILE.c_str() );
	UpdateItem( pQuestNode, NULL );

	return TRUE;
}

BOOL CPageQuestTree::DelQuest()
{
	if ( GetSelectItem() )
	{
		CString strTemp = GLQuestMan::GetInstance().GetPath();

		strTemp += GetSelectItem()->strFILE.c_str();
		if ( ! DeleteFile( strTemp ) )
		{
			CString strERROR;
			strERROR.Format( "Unable to Delete File: %s", GetSelectItem()->strFILE.c_str() );
			MessageBox( strERROR.GetString() );
			return FALSE;
		}

		DWORD WID = GetSelectItem()->pQUEST->m_sNID.dwID;
		GLQuestMan::GetInstance().Delete( WID );
		m_ctrlQuestTree.DeleteItem ( m_ctrlQuestTree.GetSelectedItem () );

		return TRUE;	
	}

	return FALSE;
}

BOOL CPageQuestTree::CopyQuest()
{
	HTREEITEM hMainItem = m_ctrlQuestTree.GetSelectedItem ();
	if ( hMainItem && hMainItem != m_TreeRoot )
	{
		PQUESTNODE pQuestNode = (PQUESTNODE)m_ctrlQuestTree.GetItemData ( hMainItem );
		*(m_pDummyQuestNode->pQUEST) = *(pQuestNode->pQUEST);
		m_pDummyQuestNode->strFILE = pQuestNode->strFILE;
		m_bDummyHasQuest = TRUE;
		return TRUE;
	}
	return FALSE;
}

BOOL CPageQuestTree::PasteQuestNew ()
{
	if ( !m_bDummyHasQuest )
		return FALSE;

	if ( GetSelectItem () )
	{
		CDlgInput dlg;
		if ( dlg.DoModal() != IDOK ) return FALSE;

		DWORD dwMID = dlg.m_nNEWID;
		std::string strNAME = dlg.m_strNEWNAME.GetString();

		QUESTNODE sQuestNode;
		sQuestNode.pQUEST = new GLQUEST;
		*(sQuestNode.pQUEST) = *(GetSelectItem()->pQUEST);
		sQuestNode.pQUEST->m_sNID = dwMID;
		sQuestNode.strFILE = strNAME.c_str();

		GLQuestMan::GetInstance().Insert( dwMID, sQuestNode );
		GLQuestMan::MAPQUEST& m_mapQuestMap = GLQuestMan::GetInstance().GetMap();
		GLQuestMan::MAPQUEST_ITER iter_pos = m_mapQuestMap.find( dwMID );

		if ( iter_pos == m_mapQuestMap.end() ) return FALSE;
		PQUESTNODE pQuestNode = (PQUESTNODE) &iter_pos->second;

		pQuestNode->pQUEST->SAVE( pQuestNode->strFILE.c_str() );
		UpdateItem( pQuestNode, NULL );

		return TRUE;
	}

	return FALSE;
}

BOOL CPageQuestTree::PasteQuestValue ()
{
	if ( !m_bDummyHasQuest )
		return FALSE;

	HTREEITEM hMainItem = m_ctrlQuestTree.GetSelectedItem ();
	if ( hMainItem && GetSelectItem () )
	{		
		DWORD dwMID = GetSelectItem()->pQUEST->m_sNID.dwID;
		PQUESTNODE pQuestNode = GetSelectItem();

		*(pQuestNode->pQUEST) = *(m_pDummyQuestNode->pQUEST);

		pQuestNode->pQUEST->m_sNID.dwID = dwMID;
		pQuestNode->pQUEST->SAVE( pQuestNode->strFILE.c_str() );
		m_ctrlQuestTree.SetItemText ( hMainItem, GetFormatName ( GetSelectItem() ) );

		return TRUE;
	}

	return FALSE;
}

