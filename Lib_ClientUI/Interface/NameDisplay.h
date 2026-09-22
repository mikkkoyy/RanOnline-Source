#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "glcrowrenlist.h"

#include "GLPVPCaptureTheFlagDefine.h"

class CBasicTextBox;
class CBasicLineBoxEx;

class CNameDisplay : public CUIGroup
{
public:
	enum
	{
		nSCHOOLMARK		= 3,
		nPARTYMARK		= 5,
        nSkillBuff		= 4,
	};

public:
	CNameDisplay ();
	virtual	~CNameDisplay ();

public:
	void CreateSubControl ();

//public:
//	void SetNameColor ( const D3DCOLOR& dwColor );
//	void SetClubNameColor ( const D3DCOLOR& dwColor );

public:
	/*pvp capture the flag, Juver, 2018/01/30 */
	void SetName ( const CString& strName, const CString& strBadge, const D3DCOLOR& dwColor, WORD wSchoolMark = NODATA, DISP_PARTY emPartyMark = NODATA, 
		WORD wCaptureTheFlagTeam = CAPTURE_THE_FLAG_TEAM_SIZE );
	void SetClubName ( const CString& strName, const D3DCOLOR& dwColor );
	void SetPetOwnerName( const CString& strName, const D3DCOLOR& dwColor );
	void SetBuffIcon( int nIndex, bool bVisible );

	bool IsMemPool()					{ return m_bUsedMemPool; }
	void SetMemPool()					{ m_bUsedMemPool = true; }

	void INITIALIZE();

protected:
    CUIControl* CreateControl ( const char* szControl, WORD wAlignFlag );

public:
	virtual HRESULT Render ( LPDIRECT3DDEVICEQ pd3dDevice );

private:
	CBasicTextBox*		m_pNameBox;
	CBasicLineBoxEx*	m_pNameLineBox;
	CBasicLineBoxEx*	m_pNameLineBoxCD;
	UIRECT				m_rcNameBox; // MEMO : static 변수는...?

private:
	CUIControl*			m_pSchoolMark[nSCHOOLMARK];
	CUIControl*			m_pPartyMark[nPARTYMARK];
	CUIControl*			m_pClubMark;
    CUIControl*			m_pBuffIcon[nSkillBuff];

	UIRECT				m_rcSchoolMark[nSCHOOLMARK];
	UIRECT				m_rcPartyMark[nPARTYMARK];
	UIRECT				m_rcBuffIcon[nSkillBuff];

	/*pvp capture the flag, Juver, 2018/01/30 */
	CUIControl*			m_pCaptureTheFlagTeamMark[CAPTURE_THE_FLAG_TEAM_SIZE];
	UIRECT				m_rcCaptureTheFlagTeamMark[CAPTURE_THE_FLAG_TEAM_SIZE];

private:
	bool				m_bDISP;
	CROWREN				m_sDISP_INFO;
	bool				m_bUsedMemPool; // MEMO : 메모리 풀을 사용하기 위한 플래그]
	bool				m_bClub;

public:
	bool DIFFERENT ( const CROWREN &sINFO );
	void UPDATE ( const CROWREN &sINFO );
	void SET_INFO ( CROWREN &sINFO )					{ m_sDISP_INFO = sINFO; }
	void SET_DISP ( bool bDISP )						{ m_bDISP = bDISP; }

public:
	DWORD GETCOUNT ()			{ return m_sDISP_INFO.m_dwCOUNT; }
	bool GET_DISP ()			{ return m_bDISP; }
	EMCROW GETCROW ()			{ return m_sDISP_INFO.m_emCROW; }
	DWORD GETCTRLID()			{ return m_sDISP_INFO.m_dwID; }
	const CROWREN* GET_INFO ()	{ return &m_sDISP_INFO; }

	/*hide crow name, Juver, 2017/09/01 */
	bool GET_HIDENAME()			{ return m_sDISP_INFO.m_bHideName; }
	
public:
	BOOL IsRELAY ()				{ return NULL != (m_sDISP_INFO.m_dwTYPE&(TYPE_TARGET|TYPE_OURS)); }
	BOOL IsTARGET ()			{ return NULL != (m_sDISP_INFO.m_dwTYPE&(TYPE_TARGET)); }
	BOOL IsClubTARGET ()		{ return NULL != (m_sDISP_INFO.m_dwTYPE&(TYPE_CLUBBATTLE)); }
	BOOL IsPMARKET ()			{ return m_sDISP_INFO.m_bPMARKET; }
	BOOL IsCDCERTIFY ()			{ return NULL != (m_sDISP_INFO.m_dwTYPE&(TYPE_CLUBCD)); }
};
