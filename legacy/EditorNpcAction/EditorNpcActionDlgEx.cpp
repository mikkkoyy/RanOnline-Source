#include "stdafx.h"
#include "EditorNpcAction.h"
#include "EditorNpcActionDlg.h"

#include "DxServerInstance.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"

#include "NpcDialogueSet.h"


BOOL CEditorNpcActionDlg::DoNew ()
{
	m_pSheetTab->GetDialogueSet()->DoModified();
	SetWindowText( "EditorNpcAction" );
	m_pSheetTab->GetDialogueSet()->RemoveAllDialogue ();
	m_pSheetTab->m_PageTable.UpdateItems ();
	return TRUE;
}

BOOL CEditorNpcActionDlg::DoLoad ()
{
	CString szFilter = "NPC Talk File (*.ntk)|*.ntk|";
	CFileDialog dlg(TRUE,".ntk",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this );

	dlg.m_ofn.lpstrInitialDir = CNpcDialogueSet::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		m_strFileName = "";
		BOOL bOK = m_pSheetTab->GetDialogueSet()->Load ( dlg.GetFileName().GetString() );
		if( bOK )	
		{
			SetWindowText( dlg.GetFileName().GetString() );
			m_strFileName = dlg.GetFileName().GetString();
		}else{
			MessageBox ( "Failure to Load File." );
			return FALSE;
		}

		m_pSheetTab->GetDialogueSet()->DoModified();
		m_pSheetTab->m_PageTable.UpdateItems ();
		return TRUE;
	}

	return FALSE;
}

BOOL CEditorNpcActionDlg::DoSave ()
{
	CString szFilter = "NPC Talk File (*.ntk)|*.ntk|";
	CFileDialog dlg(FALSE,".ntk",m_strFileName.c_str(),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);

	dlg.m_ofn.lpstrInitialDir = CNpcDialogueSet::GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		BOOL bOK = m_pSheetTab->GetDialogueSet()->Save ( dlg.GetPathName().GetString() );
		if( bOK )	
		{
			SetWindowText( dlg.GetFileName().GetString() );
			return TRUE;
		}
		else
		{
			MessageBox ( "Failure to Save File." );
			return FALSE;
		}
	}
	return FALSE;
}

void CEditorNpcActionDlg::SaveAll ()
{
	CString strPath = CNpcDialogueSet::GetPath ();
	CString strFind = strPath + "*.ntk";

	WIN32_FIND_DATA pInfo;
	lstrcpy( pInfo.cFileName, strFind.GetString() );

	HANDLE hContext = FindFirstFile( strFind.GetString(), &pInfo );
	if( hContext == INVALID_HANDLE_VALUE )
	{
		FindClose( hContext );
		return;
	} 

	CString strFile;
	do
	{
		if( m_pSheetTab->GetDialogueSet()->Load( pInfo.cFileName ) )
		{
			m_pSheetTab->GetDialogueSet()->DoModified();
			m_pSheetTab->m_PageTable.UpdateItems();

			strFile = strPath + pInfo.cFileName;			
			if ( !m_pSheetTab->GetDialogueSet()->Save(strFile) )	
			{
				CString strERROR;
				strERROR.Format( "Error Saving File: %s", strFile );
				MessageBox ( strERROR );
			}	
		}
	}
	while( FindNextFile( hContext, &pInfo ) != 0 );

	FindClose( hContext );

	MessageBox( "Ntk File All Save" );
}

void CEditorNpcActionDlg::ClassConvert ()
{
	CString strPath = CNpcDialogueSet::GetPath ();
	CString strFind = strPath + "*.ntk";
	CString strTemp = "";
	strTemp.Format ("Convert all .ntk Files on this Path: \n [%s] \n to all class?",strPath );

	if ( MessageBox ( strTemp, "Question", MB_YESNO ) == IDYES )
	{
		WIN32_FIND_DATA pInfo;
		lstrcpy( pInfo.cFileName, strFind.GetString() );

		HANDLE hContext = FindFirstFile( strFind.GetString(), &pInfo );
		if( hContext == INVALID_HANDLE_VALUE )
		{
			FindClose( hContext );
			return;
		} 

		CString strFile;
		do
		{
			if( m_pSheetTab->GetDialogueSet()->Load( pInfo.cFileName ) )
			{
				m_pSheetTab->GetDialogueSet()->DoModified();
				m_pSheetTab->m_PageTable.UpdateItems();

				CNpcDialogueSet* pDialogueSet = m_pSheetTab->GetDialogueSet();
				CNpcDialogueSet::DIALOGMAP* pDialogueMap = pDialogueSet->GetDialogueMap();
				CNpcDialogueSet::DIALOGMAP_IT found;

				UINT i;

				for( found = pDialogueMap->begin(); found != pDialogueMap->end(); ++found )
				{
					CNpcDialogue* pNpcDialogue = (*found).second;

					for( i = 0; i < pNpcDialogue->GetDlgCaseVec().size(); i++ )
					{
						SNpcTalkCondition	m_DummyCondition;
						SNpcTalkCondition* pCondition = pNpcDialogue->GetDlgCase( i )->GetCondition();

						if ( pCondition )
						{
							pCondition->m_dwClass = GLCC_ALL_8CLASS;
						}else{
							SNpcTalkCondition sNewCondition;
							sNewCondition.m_dwClass = GLCC_ALL_8CLASS;
							pNpcDialogue->GetDlgCase( i )->SetCondition ( sNewCondition );
						}

						if( pNpcDialogue->GetDlgCase( i )->GetTalkControl() == NULL ) continue;
						if( pNpcDialogue->GetDlgCase( i )->GetTalkControl()->GetTalkMap()->size() == 0 ) continue;

						CNpcTalkControl::NPCMAP_IT iter     = pNpcDialogue->GetDlgCase( i )->GetTalkControl()->GetTalkMap()->begin();
						CNpcTalkControl::NPCMAP_IT iter_end = pNpcDialogue->GetDlgCase( i )->GetTalkControl()->GetTalkMap()->end();	
						for ( ; iter!=iter_end; ++iter )
						{
							SNpcTalk* pNpcTalk = (*iter).second;
							SNpcTalkCondition*	m_test = pNpcTalk->GetCondition();

							if ( m_test )
							{
								pNpcTalk->m_pCondition->m_dwClass = GLCC_ALL_8CLASS;
							}else{
								SNpcTalkCondition sNewCondition;
								sNewCondition.m_dwClass = GLCC_ALL_8CLASS;
								pNpcTalk->SetCondition ( sNewCondition );
							}
						}
					}
				}
				strFile = strPath + pInfo.cFileName;			
				if ( !m_pSheetTab->GetDialogueSet()->Save(strFile) )
				{
					CString strERROR;
					strERROR.Format( "Error Saving File: %s", strFile );
					MessageBox ( strERROR );
				}	
			}
		}
		while( FindNextFile( hContext, &pInfo ) != 0 );
		FindClose( hContext );
		MessageBox( "Ntk File All Converted And Save" );
	}
}