
#include "stdafx.h"
#include "EditorNpcAction.h"
#include "PageAction.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"

#include "NpcTalkControl.h"
#include "NpcTalk.h"

#include "HLibDataConvert.h"

void CPageAction::UpdateList()
{
	CString strTemp;

	SetWin_Num_int ( this, IDC_EDIT_STATEMENT_ID, m_DummyDialogue.GetDlgNID() );

	m_listctrlCase.DeleteAllItems ();
	m_listctrlCase.SetRedraw( FALSE );

	const CNpcDialogue::DLGCASE& sVecDlgCase = m_DummyDialogue.GetDlgCaseVec();
	CNpcDialogue::DLGCASE_CITER iter = sVecDlgCase.begin();
	CNpcDialogue::DLGCASE_CITER iter_end = sVecDlgCase.end();
	for ( DWORD dwIndex=0; iter!=iter_end; ++iter, ++dwIndex )
	{
		const CNpcDialogueCase* pCase = (*iter);
		SNpcTalkCondition *pCondition = pCase->GetCondition();
		std::string strCondition;
		if ( pCondition )	pCondition->GetConditionText( strCondition );

		int nItem = m_listctrlCase.InsertItem ( dwIndex, _HLIB::cstring_dword(dwIndex).GetString() );
		m_listctrlCase.SetItemData ( nItem, dwIndex );
		m_listctrlCase.SetItemText ( dwIndex, 1, pCase->GetBasicTalk().c_str() );
		m_listctrlCase.SetItemText ( dwIndex, 2, strCondition.c_str() );
		m_listctrlCase.SetItemText ( dwIndex, 3, _HLIB::cstring_dword(pCase->GetTalkNum()).GetString() );

	}

	m_listctrlCase.SetRedraw( TRUE );

	InformationPreview();
}

void CPageAction::InformationPreview ()
{
	m_listctrlAnswer.DeleteAllItems ();
	m_listctrlAnswer.SetRedraw( FALSE );

	POSITION pos = m_listctrlCase.GetFirstSelectedItemPosition();
	if ( pos != NULL)
	{
		int nItem = m_listctrlCase.GetNextSelectedItem(pos);
		DWORD dwData = (DWORD) m_listctrlCase.GetItemData ( nItem );

		CNpcDialogueCase* pCase = m_DummyDialogue.GetDlgCase ( dwData );
		if ( pCase )
		{
			std::string strTalk = pCase->GetBasicTalk();

			std::string strCondition;
			SNpcTalkCondition* pCondition = pCase->GetCondition();
			if ( pCondition )	pCondition->GetConditionText( strCondition );

			SetWin_Text ( this, IDC_EDIT_STATEMENT, strTalk.c_str() );
			SetWin_Text ( this, IDC_EDIT_CONDITION, strCondition.c_str() );

			CNpcTalkControl* pTalkCtrl = pCase->GetTalkControl();
			if ( pTalkCtrl )
			{
				TCHAR szTemp[32] = {0};

				const CNpcTalkControl::NPCMAP &mapTALK = *pTalkCtrl->GetTalkMap();
				CNpcTalkControl::NPCMAP_CIT iter = mapTALK.begin();
				CNpcTalkControl::NPCMAP_CIT iter_end = mapTALK.end();
				for ( int nIndex=0; iter!=iter_end; ++iter, ++nIndex )
				{
					SNpcTalk* pTalk = (*iter).second;

					pTalk->GetTalk();

					_snprintf( szTemp, 32, "%u", pTalk->m_dwNID );

					int nItem = m_listctrlAnswer.InsertItem ( nIndex, szTemp );		
					m_listctrlAnswer.SetItemText ( nIndex, 1, pTalk->GetTalk() );
					m_listctrlAnswer.SetItemText ( nIndex, 2, SNpcTalk::strACTIONTYPE[pTalk->m_nACTION].c_str() );

					std::string strACT_NO;
					switch ( pTalk->m_nACTION )
					{
					case SNpcTalk::EM_PAGE_MOVE:
						m_pNpcDialogueSet->GetDlgText ( pTalk->m_dwACTION_NO, strACT_NO );
						break;

					case SNpcTalk::EM_BASIC:
						strACT_NO = SNpcTalk::szBASICDESC[pTalk->m_dwACTION_NO].c_str();
						break;

					case SNpcTalk::EM_QUEST_START:
						m_pNpcDialogueSet->GetDlgText ( pTalk->m_dwACTION_NO, strACT_NO );
						break;

					case SNpcTalk::EM_QUEST_STEP:
						m_pNpcDialogueSet->GetDlgText ( pTalk->m_dwACTION_NO, strACT_NO );
						break;

					default:
						_snprintf( szTemp, 32, "%u", pTalk->m_dwACTION_NO );
						strACT_NO = szTemp;
						break;
					};

					m_listctrlAnswer.SetItemText ( nIndex, 3, strACT_NO.c_str() );
					m_listctrlAnswer.SetItemData ( nItem, (DWORD_PTR)pTalk->m_dwNID );
				}
			}
		}
	}
	else
	{
		SetWin_Text ( this, IDC_EDIT_STATEMENT, "" );
		SetWin_Text ( this, IDC_EDIT_CONDITION, "" );
	}

	m_listctrlAnswer.SetRedraw( TRUE );
}