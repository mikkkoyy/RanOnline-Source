#include "stdafx.h"
#include "EditorMobNpc.h"
#include "PageTree.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "GLStringTable.h"

const char* CPageTree::GetFormatName ( PCROWDATA pCROW )
{
	GASSERT ( pCROW );

	static	CString strCrow;
	strCrow.Format ( "[%04d/%04d][Level:%04d] %s", pCROW->m_sBasic.sNativeID.wMainID, pCROW->m_sBasic.sNativeID.wSubID, pCROW->m_sBasic.m_wLevel, pCROW->GetName() );
	return strCrow.GetString();
}

BOOL CPageTree::AddCrowMain( HTREEITEM hMainItem )
{
	WORD MID = GetSelectCrow()->m_sBasic.sNativeID.wMainID;
	WORD SID = GLCrowDataMan::GetInstance().FindFreeCrowDataSID( MID );
	if ( SID == ITEMID_NOTFOUND )	return FALSE;

	SCROWDATA sCROW;
	sCROW.Assign ( *m_pDummyCrow );
	sCROW.m_sBasic.sNativeID.wMainID = MID;
	sCROW.m_sBasic.sNativeID.wSubID = SID;

	char szNewName[CHAR_SZNAME];
	sprintf ( szNewName, "CN_%03d_%03d", MID, SID );
	StringCchCopy( sCROW.m_sBasic.m_szName, CHAR_SZNAME, szNewName );

	GLCrowDataMan::GetInstance().InsertCrowData ( MID, SID, &sCROW );
	PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData ( MID, SID );

	HTREEITEM hParentItem = m_ctrlTree.GetParentItem ( hMainItem );
	if ( hParentItem != m_TreeRoot )
	{
		UpdateItem ( pCROW, hParentItem );
	}else{
		UpdateItem ( pCROW, hMainItem );
	}	

	return TRUE;
}

BOOL CPageTree::AddCrowSub ()
{
	WORD MID = GLCrowDataMan::GetInstance().FindFreeCrowDataMID ();
	if ( MID == ITEMID_NOTFOUND )	return FALSE;
	WORD SID = GLCrowDataMan::GetInstance().FindFreeCrowDataSID( MID );
	if ( SID == ITEMID_NOTFOUND )	return FALSE;

	SCROWDATA sCROW;
	sCROW.Assign ( *m_pDummyCrow );
	sCROW.m_sBasic.sNativeID.wMainID = MID;
	sCROW.m_sBasic.sNativeID.wSubID = SID;

	char szNewName[CHAR_SZNAME];
	sprintf ( szNewName, "CN_%03d_%03d", MID, SID );
	StringCchCopy( sCROW.m_sBasic.m_szName, CHAR_SZNAME, szNewName );

	GLCrowDataMan::GetInstance().InsertCrowData ( MID, SID, &sCROW );
	PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData ( MID, SID );
	UpdateItem ( pCROW, NULL );

	return TRUE;
}

BOOL CPageTree::AddCrow ()
{
	HTREEITEM hMainItem = m_ctrlTree.GetSelectedItem ();
	if ( hMainItem && GetSelectCrow () )
	{		
		return AddCrowMain( hMainItem );
	}else{
		return AddCrowSub ();
	}

	return FALSE;
}

BOOL CPageTree::DelCrow ()
{
	if ( GetSelectCrow() )
	{
		WORD MID = GetSelectCrow()->m_sBasic.sNativeID.wMainID;
		WORD SID = GetSelectCrow()->m_sBasic.sNativeID.wSubID;

		if ( SID == 0 )	
		{
			for ( int i = 0; i < GLCrowDataMan::MAX_CROW_SID; i++ )
			{
				GLStringTable::GetInstance().DeleteString( MID, i, GLStringTable::CROW );
				GLCrowDataMan::GetInstance().DeleteCrowData ( MID, i );
			}
		}else{
			GLStringTable::GetInstance().DeleteString( MID, SID, GLStringTable::CROW );
			GLCrowDataMan::GetInstance().DeleteCrowData ( MID, SID );
		}		

		m_ctrlTree.DeleteItem ( m_ctrlTree.GetSelectedItem () );

		return TRUE;	
	}

	return FALSE;
}

BOOL CPageTree::CopyCrow ()
{
	HTREEITEM hMainItem = m_ctrlTree.GetSelectedItem ();
	if ( hMainItem && hMainItem != m_TreeRoot )
	{
		m_pDummyCrow->Assign( *((PCROWDATA)m_ctrlTree.GetItemData ( hMainItem )) );
		m_bDummyHasCrow = TRUE;
		return TRUE;
	}

	return FALSE;
}

BOOL CPageTree::PasteCrowNew ()
{
	if ( !m_bDummyHasCrow )	return FALSE;

	HTREEITEM hMainItem = m_ctrlTree.GetSelectedItem ();
	if ( hMainItem && GetSelectCrow() )
	{
		WORD MID = GetSelectCrow()->m_sBasic.sNativeID.wMainID;
		WORD SID = GLCrowDataMan::GetInstance().FindFreeCrowDataSID( MID );
		if ( SID == ITEMID_NOTFOUND )	return FALSE;

		SCROWDATA sCROW;
		sCROW.Assign ( *m_pDummyCrow );
		sCROW.m_sBasic.sNativeID.wMainID = MID;
		sCROW.m_sBasic.sNativeID.wSubID = SID;

		char szNewName[CHAR_SZNAME];
		sprintf ( szNewName, "CN_%03d_%03d", MID, SID );
		StringCchCopy( sCROW.m_sBasic.m_szName, CHAR_SZNAME, szNewName );

		GLStringTable::GetInstance().DeleteString ( MID, SID, GLStringTable::CROW );
		GLStringTable::GetInstance().InsertString ( std::string( sCROW.m_sBasic.m_szName ), std::string( m_pDummyCrow->GetName() ), GLStringTable::CROW );
		GLCrowDataMan::GetInstance().InsertCrowData ( MID, SID, &sCROW );
		PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData ( MID, SID );

		HTREEITEM hParentItem = m_ctrlTree.GetParentItem ( hMainItem );
		if ( hParentItem != m_TreeRoot )
		{
			UpdateItem ( pCROW, hParentItem );
		}else{
			UpdateItem ( pCROW, hMainItem );
		}	

		return TRUE;
	}
	else
	{
		WORD MID = GLCrowDataMan::GetInstance().FindFreeCrowDataMID();
		if ( MID == ITEMID_NOTFOUND )	return FALSE;
		WORD SID = GLCrowDataMan::GetInstance().FindFreeCrowDataSID( MID );
		if ( SID == ITEMID_NOTFOUND )	return FALSE;

		SCROWDATA sCROW;
		sCROW.Assign ( *m_pDummyCrow );
		sCROW.m_sBasic.sNativeID.wMainID = MID;
		sCROW.m_sBasic.sNativeID.wSubID = SID;

		char szNewName[CHAR_SZNAME];
		sprintf ( szNewName, "CN_%03d_%03d", MID, SID );
		StringCchCopy( sCROW.m_sBasic.m_szName, CHAR_SZNAME, szNewName );

		GLStringTable::GetInstance().DeleteString ( MID, SID, GLStringTable::CROW );
		GLStringTable::GetInstance().InsertString ( std::string( sCROW.m_sBasic.m_szName ), std::string( m_pDummyCrow->GetName() ), GLStringTable::CROW );
		GLCrowDataMan::GetInstance().InsertCrowData ( MID, SID, &sCROW );

		PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData ( MID, SID );
		UpdateItem ( pCROW, NULL );

		return TRUE;
	}
}

BOOL CPageTree::PasteCrowValue()
{
	if ( !m_bDummyHasCrow )	return FALSE;

	HTREEITEM hMainItem = m_ctrlTree.GetSelectedItem ();
	if ( hMainItem && GetSelectCrow () )
	{		
		WORD MID = GetSelectCrow()->m_sBasic.sNativeID.wMainID;
		WORD SID = GetSelectCrow()->m_sBasic.sNativeID.wSubID;

		GetSelectCrow()->Assign( *m_pDummyCrow );
		GetSelectCrow()->m_sBasic.sNativeID.wMainID = MID;
		GetSelectCrow()->m_sBasic.sNativeID.wSubID = SID;

		char szNewName[CHAR_SZNAME];
		sprintf ( szNewName, "CN_%03d_%03d", MID, SID );
		StringCchCopy( GetSelectCrow()->m_sBasic.m_szName, CHAR_SZNAME, szNewName );

		GLStringTable::GetInstance().DeleteString ( MID, SID, GLStringTable::CROW );
		GLStringTable::GetInstance().InsertString ( std::string( GetSelectCrow()->m_sBasic.m_szName ), std::string( m_pDummyCrow->GetName() ), GLStringTable::CROW );

		m_ctrlTree.SetItemText ( hMainItem, GetFormatName( GetSelectCrow() ) );

		return TRUE;
	}
	return FALSE;
}