/////////////////////////////////////////////////////////////////////////////
//	FileName	: CWRankingDisplay.h
//	Project		: RanClientUILib
//	Coder		: LG-7

#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CBasicLineBox;
class CBasicTextBox;
struct SCW_RANKING;
class CCWRankingDisplay : public CUIGroup
{
private:
	enum
	{
		nMAX_LBOX			= 6,
		nMAX_TEXT_TITLE2	= 3,
		nMAX_TEXT_RANK		= 11
	};

private:
	CBasicLineBox*	m_pLBox_[nMAX_LBOX];
	CBasicTextBox*	m_pText_Title1;
	CBasicTextBox*	m_pText_Title2_[nMAX_TEXT_TITLE2];
	CBasicTextBox*	m_pText_Rank_[nMAX_TEXT_RANK];
	CBasicTextBox*	m_pText_Name_[nMAX_TEXT_RANK];
	CBasicTextBox*	m_pText_KD_[nMAX_TEXT_RANK];

public:
	CCWRankingDisplay();
	virtual ~CCWRankingDisplay();

public:
	void CreateSubControl();
	virtual HRESULT RestoreDeviceObjects(LPDIRECT3DDEVICEQ pd3dDevice);

private:
	CBasicTextBox* CreateStaticControl(char* szControlKeyword, CD3DFontPar* pFont, int nAlign, const UIGUID& cID = NO_ID);
	void DefaultPos();

public:
	void ResetAll();
	void SetCWRankingTitle(CString strTitle);
	void AddCWRanking(SCW_RANKING sCWRanking);
	void AddMyCWRanking(SCW_RANKING sCWRanking);
};
