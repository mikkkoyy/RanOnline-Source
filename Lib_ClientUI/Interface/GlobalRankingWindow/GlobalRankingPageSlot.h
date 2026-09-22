/////////////////////////////////////////////////////////////////////////////
//	FileName	: GlobalRankingPageSlot.h
//	Project		: RanClientUILib
//	Coder		: LG-7

#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "DxGlobalStage.h"

class CBasicLineBox;
class CBasicTextBox;
class CGlobalRankingPageSlot : public CUIGroup
{
private:
	enum
	{
		DUMMY = NO_ID + 1,
		CLASS_DUMMY,
		SCHOOL_DUMMY,
		GUILD_DUMMY
	};

	enum { nMAX_TEXT = 6 };

private:
	int					m_nIndex1;
	int					m_nIndex2;
	BOOL				m_bGuild;
	int					m_nSize;
	BOOL				m_bHaveData;
	LPDIRECT3DDEVICEQ	m_pd3dDevice;

private:
	CBasicLineBox*	m_pLBoxA_[8];
	CBasicLineBox*	m_pLBoxB_[8];
	CUIControl*		m_pDummy;
	CBasicTextBox*	m_pText_[nMAX_TEXT];
	CUIControl*		m_pClassImage;
	CUIControl*		m_pSchoolImage;
	CUIControl*		m_pGuildImage;
	STOP_RANK_KILL	m_sTopKill;
	STOP_RANK_RICH	m_sTopRich;
	STOP_RANK_GUILD	m_sTopGuild;

public:
	CGlobalRankingPageSlot();
	virtual ~CGlobalRankingPageSlot();

public:
	void CreateSubControl(int nIndex1, int nIndex2, BOOL bGuild);
	virtual	void TranslateUIMessage(UIGUID ControlID, DWORD dwMsg);
	virtual HRESULT InitDeviceObjects(LPDIRECT3DDEVICEQ pd3dDevice);
	virtual HRESULT Render(LPDIRECT3DDEVICEQ pd3dDevice);

private:
	CBasicTextBox* CreateStaticControl(char* szControlKeyword, CD3DFontPar* pFont, int nAlign, const UIGUID& cID = NO_ID);
	void DefaultPos();

public:
	void RESET();
	void SetRankingKill(STOP_RANK_KILL sTopKill);
	void SetRankingRich(STOP_RANK_RICH sTopRich);
	void SetRankingGuild(STOP_RANK_GUILD sTopGuild);

	BOOL IsHaveData() { return m_bHaveData; }
};