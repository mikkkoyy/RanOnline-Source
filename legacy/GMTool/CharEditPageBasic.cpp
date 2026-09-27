// CharEditPageBasic.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "CharEditPageBasic.h"

#include "CharEditTab.h"
#include "EtcFunction.h"
#include "GLCharData.h"
#include "GLogicData.h"

#include "GMToolDlg.h"
#include "GLMapList.h"

#include "Logic/GMToolGlobal.h"
#include "Logic/GMToolOdbcBase.h"

#include "HLibColorValue.h"

#include "DlgMap.h"
#include "SelColorDlg.h"

// CCharEditPageBasic dialog

IMPLEMENT_DYNAMIC(CCharEditPageBasic, CPropertyPage)

CCharEditPageBasic::CCharEditPageBasic(LOGFONT logfont, SCHARDATA2* pData)
	: CPropertyPage(CCharEditPageBasic::IDD)
	, m_pFont ( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pData = pData;
}

CCharEditPageBasic::~CCharEditPageBasic()
{
	SAFE_DELETE ( m_pFont );
}

void CCharEditPageBasic::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCharEditPageBasic, CPropertyPage)
	ON_BN_CLICKED(IDC_CHAREDIT_BASIC_BUTTON_CHANAME_CHECK, OnBnClickedChareditBasicButtonChanameCheck)
	ON_BN_CLICKED(IDC_CHAREDIT_BASIC_BUTTON_MAPSTART_SET, OnBnClickedChareditBasicButtonMapstartSet)
	ON_BN_CLICKED(IDC_CHAREDIT_BASIC_BUTTON_MAPSAVE_SET, OnBnClickedChareditBasicButtonMapsaveSet)
	ON_BN_CLICKED(IDC_CHAREDIT_BASIC_BUTTON_MAPCALL_SET, OnBnClickedChareditBasicButtonMapcallSet)
	ON_BN_CLICKED(IDC_CHAREDIT_BASIC_BUTTON_MAPSTART_RESET, OnBnClickedChareditBasicButtonMapstartReset)
	ON_BN_CLICKED(IDC_CHAREDIT_BASIC_BUTTON_MAPSAVE_RESET, OnBnClickedChareditBasicButtonMapsaveReset)
	ON_BN_CLICKED(IDC_CHAREDIT_BASIC_BUTTON_MAPCALL_RESET, OnBnClickedChareditBasicButtonMapcallReset)
	ON_EN_CHANGE(IDC_CHAREDIT_BASIC_EDIT_MAPSTART_MID, OnEnChangeChareditBasicEditMapstartMid)
	ON_EN_CHANGE(IDC_CHAREDIT_BASIC_EDIT_MAPSTART_SID, OnEnChangeChareditBasicEditMapstartSid)
	ON_EN_CHANGE(IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_MID, OnEnChangeChareditBasicEditMapsaveMid)
	ON_EN_CHANGE(IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_SID, OnEnChangeChareditBasicEditMapsaveSid)
	ON_EN_CHANGE(IDC_CHAREDIT_BASIC_EDIT_MAPCALL_MID, OnEnChangeChareditBasicEditMapcallMid)
	ON_EN_CHANGE(IDC_CHAREDIT_BASIC_EDIT_MAPCALL_SID, OnEnChangeChareditBasicEditMapcallSid)
	ON_BN_CLICKED(IDC_CHAREDIT_BASIC_BUTTON_HAIRCOLOR, OnBnClickedChareditBasicButtonHaircolor)
END_MESSAGE_MAP()


// CCharEditPageBasic message handlers
BOOL CCharEditPageBasic::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;


	std::string strHairype[MAX_HAIR];
	for( int i=0; i<MAX_HAIR; ++i ){
		CString strText;
		strText.Format( "Hair Type:%02d", i );
		strHairype[i] = strText.GetString();
	}
	
	std::string strFaceType[MAX_HEAD];
	for( int i=0; i<MAX_HEAD; ++i ){
		CString strText;
		strText.Format( "Face Type:%02d", i );
		strFaceType[i] = strText.GetString();
	}

	std::string strInvenLine[EM_INVENSIZE_Y-EM_INVEN_DEF_SIZE_Y];
	for( int i=0; i<EM_INVENSIZE_Y-EM_INVEN_DEF_SIZE_Y; ++i ){
		CString strText;
		strText.Format( "Line :%02d", ( i + EM_INVEN_DEF_SIZE_Y ) );
		strInvenLine[i] = strText.GetString();
	}

	SetWin_Combo_Init( this, IDC_CHAREDIT_BASIC_COMBO_CHACLASS, COMMENT::CHARCLASS, GLCI_NUM_8CLASS );
	SetWin_Combo_Init( this, IDC_CHAREDIT_BASIC_COMBO_CHASCHOOL, COMMENT::EMSCHOOL, GLSCHOOL_NUM );
	SetWin_Combo_Init( this, IDC_CHAREDIT_BASIC_COMBO_CHAHAIR, strHairype, MAX_HAIR );
	SetWin_Combo_Init( this, IDC_CHAREDIT_BASIC_COMBO_CHAFACE, strFaceType, MAX_HEAD );
	SetWin_Combo_Init( this, IDC_CHAREDIT_BASIC_COMBO_INVENLINE, strInvenLine, EM_INVENSIZE_Y-EM_INVEN_DEF_SIZE_Y );

	DataDefault();
	DataShow();

	return TRUE;  
}

void CCharEditPageBasic::DataDefault()
{
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_USERNUM, 0 );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHANUM, 0 );
	SetWin_Text( this, IDC_CHAREDIT_BASIC_EDIT_CHANAME, "" );
	SetWin_Text( this, IDC_CHAREDIT_BASIC_EDIT_CHAGUNAME, "" );

	SetWin_Combo_Sel( this, IDC_CHAREDIT_BASIC_COMBO_CHACLASS, 0 );
	SetWin_Combo_Sel( this, IDC_CHAREDIT_BASIC_COMBO_CHASCHOOL, 0 );
	SetWin_Combo_Sel( this, IDC_CHAREDIT_BASIC_COMBO_CHAHAIR, 0 );
	SetWin_Combo_Sel( this, IDC_CHAREDIT_BASIC_COMBO_CHAFACE, 0 );
	SetWin_Combo_Sel( this, IDC_CHAREDIT_BASIC_COMBO_INVENLINE, 0 );

	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHABRIGHT, 0 );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHALIVING, 0 );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHAHAIRCOLOR, 0 );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHAPK, 0 );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHALEVEL, 0 );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_CHASCALERANGE, 1.0f );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHACP, 1 ); /*combatpoint logic, Juver, 2017/05/28 */

	SetWin_Num_LONGLONG( this, IDC_CHAREDIT_BASIC_EDIT_CHAMONEY, 0 );
	SetWin_Num_LONGLONG( this, IDC_CHAREDIT_BASIC_EDIT_CHAEXP, 0 );
	SetWin_Num_LONGLONG( this, IDC_CHAREDIT_BASIC_EDIT_CHAREEXP, 0 );

	/*contribution point, Juver, 2017/08/23 */
	SetWin_Num_LONGLONG( this, IDC_CHAREDIT_BASIC_EDIT_CHA_CONTRIBUTION, 0 );

	/*activity point, Juver, 2017/08/23 */
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHA_ACTIVITY, 0 );

	/*activity system, Juver, 2017/11/04 */
	SetWin_Text( this, IDC_CHAREDIT_BASIC_EDIT_CHA_BADGE_ID, "" );

	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_POWER, 0 );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_DEX, 0 );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_SPIRIT, 0 );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_STRONG, 0 );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_STRENGTH, 0 );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_INTEL, 0 );

	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHASTREMAIN, 0 );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHASKILLPOINTS, 0 );

	/*pk info, Juver, 2017/11/17 */
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHA_PK_SCORE, 0 );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHA_PK_DEATH, 0 );

	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_GATE, 0 );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_MID, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_SID, NATIVEID_NULL().wSubID  );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_POSX, 0.0f );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_POSY, 0.0f );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_POSZ, 0.0f );

	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_MID, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_SID, NATIVEID_NULL().wSubID  );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_POSX, 0.0f );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_POSY, 0.0f );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_POSZ, 0.0f );

	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_MID, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_SID, NATIVEID_NULL().wSubID  );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_POSX, 0.0f );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_POSY, 0.0f );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_POSZ, 0.0f );

	/*equipment lock, Juver, 2018/01/14 */
	SetWin_Check( this, IDC_CHAREDIT_BASIC_CHECK_EQUIPMENT_LOCK_ENABLED, FALSE );
	SetWin_Check( this, IDC_CHAREDIT_BASIC_CHECK_EQUIPMENT_LOCK_STATUS, FALSE );
}

void CCharEditPageBasic::DataShow()
{
	if ( !m_pData ){
		return;
	}

	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_USERNUM, m_pData->GetUserID() );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHANUM, m_pData->m_dwCharID );
	SetWin_Text( this, IDC_CHAREDIT_BASIC_EDIT_CHANAME, m_pData->m_szName );
	SetWin_Text( this, IDC_CHAREDIT_BASIC_EDIT_CHAGUNAME, m_pData->m_szNick );

	SetWin_Combo_Sel( this, IDC_CHAREDIT_BASIC_COMBO_CHACLASS, (int)CharClassToIndex(m_pData->m_emClass) );
	SetWin_Combo_Sel( this, IDC_CHAREDIT_BASIC_COMBO_CHASCHOOL, m_pData->m_wSchool );
	SetWin_Combo_Sel( this, IDC_CHAREDIT_BASIC_COMBO_CHAHAIR, m_pData->m_wHair );
	SetWin_Combo_Sel( this, IDC_CHAREDIT_BASIC_COMBO_CHAFACE, m_pData->m_wFace );
	SetWin_Combo_Sel( this, IDC_CHAREDIT_BASIC_COMBO_INVENLINE, m_pData->m_wINVENLINE );

	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHABRIGHT, m_pData->m_nBright );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHALIVING, m_pData->m_nLiving );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHAHAIRCOLOR, m_pData->m_wHairColor );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHAPK, m_pData->m_wPK );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHALEVEL, m_pData->m_wLevel );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_CHASCALERANGE, m_pData->m_fScaleRange );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHACP, m_pData->m_sCombatPoint.dwNow ); /*combatpoint logic, Juver, 2017/05/28 */

	SetWin_Num_LONGLONG( this, IDC_CHAREDIT_BASIC_EDIT_CHAMONEY, m_pData->m_lnMoney );
	SetWin_Num_LONGLONG( this, IDC_CHAREDIT_BASIC_EDIT_CHAEXP, m_pData->m_sExperience.lnNow );
	SetWin_Num_LONGLONG( this, IDC_CHAREDIT_BASIC_EDIT_CHAREEXP, m_pData->m_lnReExp );

	/*contribution point, Juver, 2017/08/23 */
	SetWin_Num_LONGLONG( this, IDC_CHAREDIT_BASIC_EDIT_CHA_CONTRIBUTION, m_pData->m_llContributionPoint );

	/*activity point, Juver, 2017/08/23 */
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHA_ACTIVITY, m_pData->m_dwActivityPoint );

	/*activity system, Juver, 2017/11/04 */
	SetWin_Text( this, IDC_CHAREDIT_BASIC_EDIT_CHA_BADGE_ID, m_pData->m_szBadge );

	/*pk info, Juver, 2017/11/17 */
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHA_PK_SCORE, m_pData->m_dwPKScore );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHA_PK_DEATH, m_pData->m_dwPKDeath );

	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_POWER, m_pData->m_sStats.wPow );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_DEX, m_pData->m_sStats.wDex );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_SPIRIT, m_pData->m_sStats.wSpi );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_STRONG, m_pData->m_sStats.wStr );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_STRENGTH,m_pData->m_sStats.wSta );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_INTEL, m_pData->m_sStats.wInt );

	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHASTREMAIN, m_pData->m_wStatsPoint );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHASKILLPOINTS, m_pData->m_dwSkillPoint );

	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_GATE, m_pData->m_dwStartGate );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_MID, m_pData->m_sStartMapID.wMainID );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_SID, m_pData->m_sStartMapID.wSubID );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_POSX, m_pData->m_vStartPos.x );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_POSY, m_pData->m_vStartPos.y );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_POSZ, m_pData->m_vStartPos.z );

	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_MID, m_pData->m_sSaveMapID.wMainID );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_SID, m_pData->m_sSaveMapID.wSubID );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_POSX, m_pData->m_vSavePos.x );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_POSY, m_pData->m_vSavePos.y );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_POSZ, m_pData->m_vSavePos.z );

	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_MID, m_pData->m_sLastCallMapID.wMainID );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_SID, m_pData->m_sLastCallMapID.wSubID );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_POSX, m_pData->m_vLastCallPos.x );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_POSY, m_pData->m_vLastCallPos.y );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_POSZ, m_pData->m_vLastCallPos.z );

	/*equipment lock, Juver, 2018/01/14 */
	SetWin_Check( this, IDC_CHAREDIT_BASIC_CHECK_EQUIPMENT_LOCK_ENABLED, m_pData->m_bEnableEquipmentLock );
	SetWin_Check( this, IDC_CHAREDIT_BASIC_CHECK_EQUIPMENT_LOCK_STATUS, m_pData->m_bEquipmentLockStatus );
}

BOOL CCharEditPageBasic::DataSave()
{
	if ( !m_pData ){
		return FALSE;
	}

	std::string strName = GetWin_Text( this, IDC_CHAREDIT_BASIC_EDIT_CHANAME );
	std::string strGUName = GetWin_Text( this, IDC_CHAREDIT_BASIC_EDIT_CHAGUNAME );

	EMCHARINDEX emCharIndex = static_cast<EMCHARINDEX> ( GetWin_Combo_Sel( this, IDC_CHAREDIT_BASIC_COMBO_CHACLASS ) );
	
	WORD wSchool = (WORD)GetWin_Combo_Sel( this, IDC_CHAREDIT_BASIC_COMBO_CHASCHOOL );
	WORD wHair = (WORD)GetWin_Combo_Sel( this, IDC_CHAREDIT_BASIC_COMBO_CHAHAIR );
	WORD wFace =(WORD) GetWin_Combo_Sel( this, IDC_CHAREDIT_BASIC_COMBO_CHAFACE );
	WORD wInvenLine = (WORD)GetWin_Combo_Sel( this, IDC_CHAREDIT_BASIC_COMBO_INVENLINE );

	int nBright = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHABRIGHT );
	int nLiving = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHALIVING );
	WORD wHairColor = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHAHAIRCOLOR );
	WORD wPK = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHAPK );
	WORD wLevel = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHALEVEL );
	float fScaleRange = GetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_CHASCALERANGE );
	WORD wCombatPoint = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHACP ); /*combatpoint logic, Juver, 2017/05/28 */

	LONGLONG llMoney = GetWin_Num_LONGLONG( this, IDC_CHAREDIT_BASIC_EDIT_CHAMONEY );
	LONGLONG llEXP = GetWin_Num_LONGLONG( this, IDC_CHAREDIT_BASIC_EDIT_CHAEXP );
	LONGLONG llRecEXP = GetWin_Num_LONGLONG( this, IDC_CHAREDIT_BASIC_EDIT_CHAREEXP );

	/*contribution point, Juver, 2017/08/23 */
	LONGLONG llContributionPoint = GetWin_Num_LONGLONG( this, IDC_CHAREDIT_BASIC_EDIT_CHA_CONTRIBUTION );

	/*activity point, Juver, 2017/08/23 */
	DWORD dwActivityPoint = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHA_ACTIVITY );

	/*activity system, Juver, 2017/11/04 */
	std::string strBadge = GetWin_Text( this, IDC_CHAREDIT_BASIC_EDIT_CHA_BADGE_ID );

	/*pk info, Juver, 2017/11/17 */
	DWORD dwPKScore = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHA_PK_SCORE );
	DWORD dwPKDeath = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHA_PK_DEATH );

	WORD wStatPOW = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_POWER );
	WORD wStatDEX = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_DEX );
	WORD wStatSPI = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_SPIRIT );
	WORD wStatSTR = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_STRONG );
	WORD wStatSTA = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_STRENGTH );
	WORD wStatINT = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_STAT_INTEL );

	WORD wStatPoint = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHASTREMAIN );
	DWORD dwSkillPoint = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHASKILLPOINTS );

	WORD wStartGate = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_GATE );
	WORD wStartMapMID = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_MID );
	WORD wStartMapSID = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_SID );
	float fStartMapPosX = GetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_POSX );
	float fStartMapPosY = GetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_POSY );
	float fStartMapPosZ = GetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_POSZ );

	WORD wSaveMapMID = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_MID );
	WORD wSaveMapSID = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_SID );
	float fSaveMapPosX = GetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_POSX );
	float fSaveMapPosY = GetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_POSY );
	float fSaveMapPosZ = GetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_POSZ );

	WORD wCallMapMID = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_MID );
	WORD wCallMapSID = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_SID );
	float fCallMapPosX = GetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_POSX );
	float fCallMapPosY = GetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_POSY );
	float fCallMapPosZ = GetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_POSZ );

	/*equipment lock, Juver, 2018/01/14 */
	BOOL bEnableEquipmentLock = GetWin_Check( this, IDC_CHAREDIT_BASIC_CHECK_EQUIPMENT_LOCK_ENABLED );
	BOOL bEquipmentLockStatus = GetWin_Check( this, IDC_CHAREDIT_BASIC_CHECK_EQUIPMENT_LOCK_STATUS );

	if ( strName.size() < MIN_CHAR_ID || strName.size() > MAX_CHAR_ID ){
		CDebugSet::MsgBox( GetSafeHwnd(), "CharName can only have %d to %d characters", MIN_CHAR_ID, MAX_CHAR_ID );
		GotoDlgCtrl(GetDlgItem(IDC_CHAREDIT_BASIC_EDIT_CHANAME)); 
		return FALSE;
	}

	/*
	if ( GMTOOL_GLOBAL::IsBadString( strName.c_str() ) ){
		CDebugSet::MsgBox( GetSafeHwnd(), "CharName contains invalid characters!" );
		GotoDlgCtrl(GetDlgItem(IDC_CHAREDIT_BASIC_EDIT_CHANAME)); 
		return FALSE;
	}
	*/

	if ( strGUName.size() > CHAR_SZNAME ){
		CDebugSet::MsgBox( GetSafeHwnd(), "NickName can only have %d to %d characters", 0, CHAR_SZNAME );
		GotoDlgCtrl(GetDlgItem(IDC_CHAREDIT_BASIC_EDIT_CHAGUNAME)); 
		return FALSE;
	}

	if ( GMTOOL_GLOBAL::IsBadString3( strGUName.c_str() ) ){
		CDebugSet::MsgBox( GetSafeHwnd(), "NickName contains invalid characters!" );
		GotoDlgCtrl(GetDlgItem(IDC_CHAREDIT_BASIC_EDIT_CHAGUNAME)); 
		return FALSE;
	}

	if ( GMTOOL_GLOBAL::IsBadString3( strBadge.c_str() ) ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Badge contains invalid characters!" );
		GotoDlgCtrl(GetDlgItem(IDC_CHAREDIT_BASIC_EDIT_CHA_BADGE_ID)); 
		return FALSE;
	}

	if ( fScaleRange < SCALERANGE_MIN || fScaleRange > SCALERANGE_MAX )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Scale Range Value can only be %g~%g", SCALERANGE_MIN, SCALERANGE_MAX );
		GotoDlgCtrl(GetDlgItem(IDC_CHAREDIT_BASIC_EDIT_CHASCALERANGE)); 
		return FALSE;
	}
	
	/*combatpoint logic, Juver, 2017/05/28 */
	if ( wCombatPoint > GLCONST_CHAR::wCombatPoint_MAX )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Combat Point Value Limit %d", GLCONST_CHAR::wCombatPoint_MAX );
		GotoDlgCtrl(GetDlgItem(IDC_CHAREDIT_BASIC_EDIT_CHACP)); 
		return FALSE;
	}

	if ( strcmp( m_pData->m_szName, strName.c_str() ) != 0 ){
		BOOL bExist = NameCheck( m_pData, strName.c_str() );
		if ( bExist ){
			CDebugSet::MsgBox( GetSafeHwnd(), "Name:%s Already used! Try Another Name!", strName.c_str() );
			return FALSE;
		}
	}

	StringCchCopy( m_pData->m_szName, CHAR_SZNAME, strName.c_str() );
	StringCchCopy( m_pData->m_szNick, CHAR_SZNAME, strGUName.c_str() );

	/*activity system, Juver, 2017/11/05 */
	StringCchCopy( m_pData->m_szBadge, CHAR_SZNAME, strBadge.c_str() );

	m_pData->m_emClass = CharIndexToClass( emCharIndex );
	m_pData->m_wSchool = wSchool;
	m_pData->m_wHair = wHair;
	m_pData->m_wFace = wFace;
	m_pData->m_wINVENLINE = wInvenLine;

	m_pData->m_nBright = nBright;
	m_pData->m_nLiving =  nLiving;
	m_pData->m_wHairColor = wHairColor;
	m_pData->m_wPK = wPK;
	m_pData->m_wLevel = wLevel;
	m_pData->m_fScaleRange = fScaleRange;
	m_pData->m_sCombatPoint.dwNow = wCombatPoint; /*combatpoint logic, Juver, 2017/05/28 */

	m_pData->m_lnMoney = llMoney;
	m_pData->m_sExperience.lnNow = llEXP;
	m_pData->m_lnReExp = llRecEXP;

	/*contribution point, Juver, 2017/08/23 */
	m_pData->m_llContributionPoint = llContributionPoint; 

	/*activity point, Juver, 2017/08/23 */
	m_pData->m_dwActivityPoint = dwActivityPoint;

	/*pk info, Juver, 2017/11/17 */
	m_pData->m_dwPKScore = dwPKScore;
	m_pData->m_dwPKDeath = dwPKDeath;

	m_pData->m_sStats.wPow = wStatPOW;
	m_pData->m_sStats.wDex = wStatDEX;
	m_pData->m_sStats.wSpi = wStatSPI;
	m_pData->m_sStats.wStr = wStatSTR;
	m_pData->m_sStats.wSta = wStatSTA;
	m_pData->m_sStats.wInt = wStatINT;

	m_pData->m_wStatsPoint = wStatPoint;
	m_pData->m_dwSkillPoint = dwSkillPoint;

	m_pData->m_dwStartGate = wStartGate;
	m_pData->m_sStartMapID.wMainID = wStartMapMID;
	m_pData->m_sStartMapID.wSubID = wStartMapSID;
	m_pData->m_vStartPos.x = fStartMapPosX;
	m_pData->m_vStartPos.y = fStartMapPosY;
	m_pData->m_vStartPos.z = fStartMapPosZ;

	m_pData->m_sSaveMapID.wMainID = wSaveMapMID;
	m_pData->m_sSaveMapID.wSubID = wSaveMapSID;
	m_pData->m_vSavePos.x = fSaveMapPosX;
	m_pData->m_vSavePos.y = fSaveMapPosY;
	m_pData->m_vSavePos.z = fSaveMapPosZ;

	m_pData->m_sLastCallMapID.wMainID = wCallMapMID;
	m_pData->m_sLastCallMapID.wSubID = wCallMapSID;
	m_pData->m_vLastCallPos.x = fCallMapPosX;
	m_pData->m_vLastCallPos.y = fCallMapPosY;
	m_pData->m_vLastCallPos.z = fCallMapPosZ;

	/*equipment lock, Juver, 2018/01/14 */
	m_pData->m_bEnableEquipmentLock = bEnableEquipmentLock;
	m_pData->m_bEquipmentLockStatus = bEquipmentLockStatus;

	return TRUE;
}

void CCharEditPageBasic::OnBnClickedChareditBasicButtonChanameCheck()
{
	if ( !m_pData ){
		return;
	}

	std::string strName = GetWin_Text( this, IDC_CHAREDIT_BASIC_EDIT_CHANAME );

	if ( strName.size() < MIN_CHAR_ID || strName.size() > MAX_CHAR_ID ){
		CDebugSet::MsgBox( GetSafeHwnd(), "CharName can only have %d to %d characters", MIN_CHAR_ID, MAX_CHAR_ID );
		GotoDlgCtrl(GetDlgItem(IDC_CHAREDIT_BASIC_EDIT_CHANAME)); 
		return;
	}

	if ( GMTOOL_GLOBAL::IsBadString( strName.c_str() ) ){
		CDebugSet::MsgBox( GetSafeHwnd(), "CharName contains invalid characters!" );
		GotoDlgCtrl(GetDlgItem(IDC_CHAREDIT_BASIC_EDIT_CHANAME)); 
		return;
	}

	if ( strcmp( m_pData->m_szName, strName.c_str() ) == 0 ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Name unchanged, no checking needed!" );
		return;
	}

	BOOL bExist = NameCheck( m_pData, strName.c_str() );
	if ( bExist ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Name:%s Already used! Try Another Name!", strName.c_str() );
	}else{
		CDebugSet::MsgBox( GetSafeHwnd(), "Name:%s is Valid!", strName.c_str() );
	}
}

void CCharEditPageBasic::OnBnClickedChareditBasicButtonMapstartSet()
{
	CDlgMap dlg(this);
	if( dlg.DoModal() == IDOK ){
		SMAPNODE* pNode = GMTOOL_GLOBAL::gmtoolMapList.FindMapNode( dlg.m_SELECTED );
		if ( pNode ){
			SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_MID, pNode->sNativeID.wMainID );
			SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_SID, pNode->sNativeID.wSubID );
		}
	}
}

void CCharEditPageBasic::OnBnClickedChareditBasicButtonMapsaveSet()
{
	CDlgMap dlg(this);
	if( dlg.DoModal() == IDOK ){
		SMAPNODE* pNode = GMTOOL_GLOBAL::gmtoolMapList.FindMapNode( dlg.m_SELECTED );
		if ( pNode ){
			SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_MID, pNode->sNativeID.wMainID );
			SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_SID, pNode->sNativeID.wSubID );
		}
	}
}

void CCharEditPageBasic::OnBnClickedChareditBasicButtonMapcallSet()
{
	CDlgMap dlg(this);
	if( dlg.DoModal() == IDOK ){
		SMAPNODE* pNode = GMTOOL_GLOBAL::gmtoolMapList.FindMapNode( dlg.m_SELECTED );
		if ( pNode ){
			SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_MID, pNode->sNativeID.wMainID );
			SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_SID, pNode->sNativeID.wSubID );
		}
	}
}

void CCharEditPageBasic::OnBnClickedChareditBasicButtonMapstartReset()
{
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_MID, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_SID, NATIVEID_NULL().wSubID  );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_POSX, 0.0f );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_POSY, 0.0f );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_POSZ, 0.0f );
}

void CCharEditPageBasic::OnBnClickedChareditBasicButtonMapsaveReset()
{
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_MID, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_SID, NATIVEID_NULL().wSubID  );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_POSX, 0.0f );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_POSY, 0.0f );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_POSZ, 0.0f );
}

void CCharEditPageBasic::OnBnClickedChareditBasicButtonMapcallReset()
{
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_MID, NATIVEID_NULL().wMainID );
	SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_SID, NATIVEID_NULL().wSubID  );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_POSX, 0.0f );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_POSY, 0.0f );
	SetWin_Num_float( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_POSZ, 0.0f );
}

void CCharEditPageBasic::OnEnChangeChareditBasicEditMapstartMid()
{
	GMTOOL_GLOBAL::DataCheckMap( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_MID, 
		IDC_CHAREDIT_BASIC_EDIT_MAPSTART_SID,
		IDC_CHAREDIT_BASIC_EDIT_MAPSTART_NAME );
}

void CCharEditPageBasic::OnEnChangeChareditBasicEditMapstartSid()
{
	GMTOOL_GLOBAL::DataCheckMap( this, IDC_CHAREDIT_BASIC_EDIT_MAPSTART_MID, 
		IDC_CHAREDIT_BASIC_EDIT_MAPSTART_SID,
		IDC_CHAREDIT_BASIC_EDIT_MAPSTART_NAME );
}

void CCharEditPageBasic::OnEnChangeChareditBasicEditMapsaveMid()
{
	GMTOOL_GLOBAL::DataCheckMap( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_MID, 
		IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_SID,
		IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_NAME );
}

void CCharEditPageBasic::OnEnChangeChareditBasicEditMapsaveSid()
{
	GMTOOL_GLOBAL::DataCheckMap( this, IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_MID, 
		IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_SID,
		IDC_CHAREDIT_BASIC_EDIT_MAPSAVE_NAME );
}

void CCharEditPageBasic::OnEnChangeChareditBasicEditMapcallMid()
{
	GMTOOL_GLOBAL::DataCheckMap( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_MID, 
		IDC_CHAREDIT_BASIC_EDIT_MAPCALL_SID,
		IDC_CHAREDIT_BASIC_EDIT_MAPCALL_NAME );
}

void CCharEditPageBasic::OnEnChangeChareditBasicEditMapcallSid()
{
	GMTOOL_GLOBAL::DataCheckMap( this, IDC_CHAREDIT_BASIC_EDIT_MAPCALL_MID, 
			IDC_CHAREDIT_BASIC_EDIT_MAPCALL_SID,
			IDC_CHAREDIT_BASIC_EDIT_MAPCALL_NAME );
}

BOOL CCharEditPageBasic::NameCheck( SCHARDATA2* pData, std::string strNewName )
{
	if ( !pData ){
		return TRUE;
	}

	BOOL bExist = FALSE;
	int nChaNum = 0;

	int nReturn = CGMToolOdbcBase::GetInstance()->CharNameCheck( strNewName.c_str(), bExist, nChaNum );
	if ( nReturn == DB_ERROR ){
		CDebugSet::MsgBox( GetSafeHwnd(), "Unable to check database for existing name! Report this bug!" );
		return FALSE;
	}

	return bExist;
}

void CCharEditPageBasic::OnBnClickedChareditBasicButtonHaircolor()
{
	WORD wColor = GetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHAHAIRCOLOR );
	DWORD dwColor = _HLIB::color_dword_word( wColor );

	CSelColorDlg dlg( BYTE( _HLIB::color_get_int_r( dwColor) ), 
		BYTE( _HLIB::color_get_int_g( dwColor ) ), 
		BYTE( _HLIB::color_get_int_b( dwColor ) ), 
		this );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_CHAREDIT_BASIC_EDIT_CHAHAIRCOLOR, _HLIB::color_word_dword( _HLIB::color_argb_rgb(crNewColor) ) );
	}
}
