#include "stdafx.h"
#include "EditorSkill.h"
#include "PageTree.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "GLSkill.h"
#include "GLStringTable.h"
#include "GLItem.h"


const char* CPageTree::GetFormatName ( PGLSKILL pSKILL )
{
	GASSERT ( pSKILL );

	static	CString strSkill;
	if ( pSKILL )
	{	
		strSkill.Format ( "[%04d/%04d][Grade:%02d] %s                       [ ANI_MAIN:%s ] [ ANI_SUB:%s ]", 
		pSKILL->m_sBASIC.sNATIVEID.wMainID, pSKILL->m_sBASIC.sNATIVEID.wSubID, pSKILL->m_sBASIC.dwGRADE, pSKILL->GetName(),
		COMMENT::ANI_MAINTYPE_CHAR[pSKILL->m_sEXT_DATA.emANIMTYPE].c_str(), COMMENT::ANI_SUBTYPE_SKILL[pSKILL->m_sEXT_DATA.emANISTYPE].c_str() );
	}

	return strSkill.GetString ();
}

BOOL CPageTree::IsParent( HTREEITEM	hITEM )
{
	if ( hITEM )
	{
		HTREEITEM hParentItem = m_ctrlTree.GetParentItem ( hITEM );
		if ( hParentItem )
			return FALSE;
	}

	return TRUE;
}

WORD CPageTree::GetParentID( HTREEITEM	hITEM )
{
	if ( hITEM )
	{
		std::string strTEXT = m_ctrlTree.GetItemText( hITEM );
		HTREEITEM hParentItem = m_ctrlTree.GetParentItem ( hITEM );
		if ( hParentItem )
			strTEXT = m_ctrlTree.GetItemText( hParentItem );

		if ( strTEXT.size() > 0 )
		{
			for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
			{
				if ( strcmp ( COMMENT::SKILLCLASS[MID].c_str(), strTEXT.c_str() ) == 0 )
				{
					return MID;
					break;
				}
			}
		}
	}

	return ITEMID_NOTFOUND;
}

BOOL CPageTree::AddSkill()
{
	HTREEITEM hITEM = m_ctrlTree.GetSelectedItem ();
	if ( !hITEM )	return FALSE;

	WORD MID = GetParentID( hITEM );
	if ( MID == ITEMID_NOTFOUND )	return FALSE;
	WORD SID = GLSkillMan::GetInstance().FindFreeSkillIndex ( MID );
	if ( SID == ITEMID_NOTFOUND )	return FALSE;
	if ( !m_pDummySkill )	return FALSE;

	PGLSKILL pSKILLNEW = new GLSKILL;
	*pSKILLNEW = *m_pDummySkill;
	pSKILLNEW->m_sBASIC.sNATIVEID.wMainID = MID;
	pSKILLNEW->m_sBASIC.sNATIVEID.wSubID = SID;

	char szNewName[SKILL::MAX_SZNAME];
	sprintf ( szNewName, "SN_%03d_%03d", MID, SID );
	StringCchCopy( pSKILLNEW->m_sBASIC.szNAME, SKILL::MAX_SZNAME, szNewName );
	sprintf ( szNewName, "SD_%03d_%03d", MID, SID );
	pSKILLNEW->m_sEXT_DATA.strCOMMENTS = szNewName;

	GLSkillMan::GetInstance().SetData ( MID, SID, pSKILLNEW );
	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData ( MID, SID );

	UpdateItem( pSKILL, hITEM );

	return TRUE;
}

BOOL CPageTree::DelSkill()
{
	if ( GetSelectSkill() )
	{
		WORD MID = GetSelectSkill()->m_sBASIC.sNATIVEID.wMainID;
		WORD SID = GetSelectSkill()->m_sBASIC.sNATIVEID.wSubID;

		std::string strDELNAME = GetSelectSkill()->m_sBASIC.szNAME;
		std::string strDELDESC = GetSelectSkill()->m_sEXT_DATA.strCOMMENTS.c_str();

		GLStringTable::GetInstance().DeleteString( strDELNAME, strDELDESC, GLStringTable::SKILL );
		GLSkillMan::GetInstance().DeleteSkill( MID, SID );

		m_ctrlTree.DeleteItem ( m_ctrlTree.GetSelectedItem () );

		return TRUE;	
	}

	return FALSE;
}

BOOL CPageTree::CopySkill()
{
	HTREEITEM hItem = m_ctrlTree.GetSelectedItem ();
	if ( hItem && !IsParent( hItem ) )
	{
		*m_pDummySkill = *((PGLSKILL)m_ctrlTree.GetItemData ( hItem ));
		m_bDummyHasSkill = TRUE;
		return TRUE;
	}
	return FALSE;
}

BOOL CPageTree::PasteSkillNew()
{
	if ( !m_bDummyHasSkill )	return FALSE;
	if ( !m_pDummySkill )	return FALSE;

	HTREEITEM hITEM = m_ctrlTree.GetSelectedItem ();
	if ( !hITEM )	return FALSE;

	WORD MID = GetParentID( hITEM );
	if ( MID == ITEMID_NOTFOUND )	return FALSE;
	WORD SID = GLSkillMan::GetInstance().FindFreeSkillIndex ( MID );
	if ( SID == ITEMID_NOTFOUND )	return FALSE;

	PGLSKILL pSKILLNEW = new GLSKILL;
	*pSKILLNEW = *m_pDummySkill;
	pSKILLNEW->m_sBASIC.sNATIVEID.wMainID = MID;
	pSKILLNEW->m_sBASIC.sNATIVEID.wSubID = SID;

	char szNewName[SKILL::MAX_SZNAME];
	sprintf ( szNewName, "SN_%03d_%03d", MID, SID );
	StringCchCopy( pSKILLNEW->m_sBASIC.szNAME, SKILL::MAX_SZNAME, szNewName );
	sprintf ( szNewName, "SD_%03d_%03d", MID, SID );
	pSKILLNEW->m_sEXT_DATA.strCOMMENTS = szNewName;

	GLSkillMan::GetInstance().SetData ( MID, SID, pSKILLNEW );

	std::string strDELNAME = pSKILLNEW->m_sBASIC.szNAME;
	std::string strDELDESC = pSKILLNEW->m_sEXT_DATA.strCOMMENTS.c_str();
	std::string strNEWNAME = m_pDummySkill->GetName();
	std::string strNEWDESC = "";

	const char* pComment =  m_pDummySkill->GetDesc();
	if ( pComment )	strNEWDESC = pComment;

	GLStringTable::GetInstance().DeleteString( strDELNAME, strDELDESC, GLStringTable::SKILL );
	GLStringTable::GetInstance().InsertString ( strDELNAME, strNEWNAME, GLStringTable::SKILL );
	GLStringTable::GetInstance().InsertString ( strDELDESC, strNEWDESC, GLStringTable::SKILL );

	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData ( MID, SID );

	UpdateItem( pSKILL, hITEM );

	return TRUE;
}

BOOL CPageTree::PasteSkillValue()
{
	if ( !m_bDummyHasSkill )	return FALSE;
	if ( !m_pDummySkill )	return FALSE;

	HTREEITEM hITEM = m_ctrlTree.GetSelectedItem ();
	if ( hITEM && GetSelectSkill() )
	{		
		WORD MID = GetSelectSkill()->m_sBASIC.sNATIVEID.wMainID;
		WORD SID = GetSelectSkill()->m_sBASIC.sNATIVEID.wSubID;
		*(GetSelectSkill()) = *m_pDummySkill;
		GetSelectSkill()->m_sBASIC.sNATIVEID.wMainID = MID;
		GetSelectSkill()->m_sBASIC.sNATIVEID.wSubID = SID;

		char szNewName[SKILL::MAX_SZNAME];
		sprintf ( szNewName, "SN_%03d_%03d", MID, SID );
		StringCchCopy( GetSelectSkill()->m_sBASIC.szNAME, SKILL::MAX_SZNAME, szNewName );
		sprintf ( szNewName, "SD_%03d_%03d", MID, SID );
		GetSelectSkill()->m_sEXT_DATA.strCOMMENTS = szNewName;

		std::string strDELNAME = GetSelectSkill()->m_sBASIC.szNAME;
		std::string strDELDESC = GetSelectSkill()->m_sEXT_DATA.strCOMMENTS.c_str();
		std::string strNEWNAME = m_pDummySkill->GetName();
		std::string strNEWDESC = "";

		const char* pComment =  m_pDummySkill->GetDesc();
		if ( pComment )	strNEWDESC = pComment;

		GLStringTable::GetInstance().DeleteString( strDELNAME, strDELDESC, GLStringTable::SKILL );
		GLStringTable::GetInstance().InsertString ( strDELNAME, strNEWNAME, GLStringTable::SKILL );
		GLStringTable::GetInstance().InsertString ( strDELDESC, strNEWDESC, GLStringTable::SKILL );

		m_ctrlTree.SetItemText ( hITEM, GetFormatName ( GetSelectSkill() ) );
		return TRUE;
	}
	return FALSE;
}