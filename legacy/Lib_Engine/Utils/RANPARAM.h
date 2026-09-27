#pragma once
#include "d3dapp.h"
#include <string>

#define STRING_NUM_128	128
#define	QUICK_SLOT_NUM	6
#define	QUICK_SKILL_NUM	10
#define	QUICK_MENU_NUM	17
#define	QUICK_ALL_NUM	(QUICK_SLOT_NUM + QUICK_SKILL_NUM + QUICK_MENU_NUM)
#define CHAT_MACRO_NUM	10
#define BASICMENU_NUM	12

enum EMSERVICE_TYPE
{
	EMSERVICE_DEFAULT		= 0,
	EMSERVICE_KOREA			= 1,
	EMSERVICE_FEYA			= 2,
	EMSERVICE_MALAYSIA_CN	= 3,
	EMSERVICE_MALAYSIA_EN	= 4,
	EMSERVICE_JAPAN			= 5,
	EMSERVICE_THAILAND		= 6,
	EMSERVICE_CHINA			= 7,
	EMSERVICE_INDONESIA		= 8,
	EMSERVICE_PHILIPPINES	= 9,
	EMSERVICE_VIETNAM		= 10,
	EMSERVICE_GLOBAL		= 11,	//	글로벌 서비스
};

// SHOTCUT KEY
enum
{
	SHOTCUT_INVEN = 0,
	SHOTCUT_CHAR,
	SHOTCUT_SKILL,
	SHOTCUT_PARTY,
	SHOTCUT_QUEST,
	SHOTCUT_CLUB,
	SHOTCUT_FRIEND,
	SHOTCUT_MAP,
	SHOTCUT_ITEMBANK,
	SHOTCUT_RUN,
	SHOTCUT_HELP,
	SHOTCUT_PET,
	SHOTCUT_ATTACKMODE,
	SHOTCUT_PKMODE,
	SHOTCUT_ITEMSHOP,
	SHOTCUT_CHATMACRO,
	SHOTCUT_SUMMON
};

namespace RANPARAM
{
	enum
	{ 
		MAX_FTP = 24, 
		EMGAMEOPT_VER = 0x0112, 
		MAX_HTTP = 24,
		MAX_SERVER = 20,
		MAX_CHINA_REGION = 8
	};

	enum EMTEXTUREFILTER
	{
		EMTEXTUREFILTER_TRILINEAR		= 0,
		EMTEXTUREFILTER_ANISOTROPIC_2	= 1,
		EMTEXTUREFILTER_ANISOTROPIC_4	= 2,
		EMTEXTUREFILTER_ANISOTROPIC_8	= 3,
		EMTEXTUREFILTER_ANISOTROPIC_16	= 4,
		EMTEXTUREFILTER_NSIZE			= 5,
	};

	struct SERVER_SET
	{
		CString	strName;
		CString	LoginAddress;
		CString	HttpAddressTable[MAX_HTTP];
		CString	strNewsURL;
		CString	ServerName[MAX_SERVER];
		CString strItemShop;
		UINT	nLoginPort;
	};

	extern DWORD	dwPotsCount;

	extern EMSERVICE_TYPE	emSERVICE_TYPE;
	extern BOOL				bXML_USE;

	//	게임 설정.
	extern BOOL				bSAVE_USERID;

	extern BOOL				bDIS_CONFT;			//	'자동' 대련 거부.
	extern BOOL				bDIS_TRADE;			//	'자동' 거래 거부.
	extern BOOL				bDIS_PARTY;			//	'자동' 파티 거부.
	extern BOOL				bDIS_RIDE;			//	'자동' 파티 거부.
	extern BOOL				bDIS_FRIEND;
	extern BOOL				bSHOW_SIMPLEHP;
	extern BOOL				b3D_SOUND;			//	3차원 사운드
	extern BOOL				bSHOW_TIP;			//	팁 보여주기
	extern BOOL				bMOVABLE_ON_CHAT;	//	채팅창 위에서 움직이기
	extern BOOL				bFORCED_ATTACK;		//	강제 공격
	extern BOOL				bNAME_DISPLAY;		//	이름 항상 표시
	extern BOOL				bNON_Rebirth;		//	부활 스킬 금지
	extern BOOL				bPrivateStats;		/*charinfoview , Juver, 2017/11/12 */

    extern BOOL             bCheckUsableItemMarketPage; // NPC 상점 ui에서 사용 가능한 아이템 체크.
    extern BOOL             bCheckBuyableItemMarketPage; // NPC 상점 ui에서 구입 가능한 아이템 체크.
    extern BOOL             bCheckRebuySuppliesRebuyPage; // NPC 재매입 ui에서 소모품 아이템 체크.
    extern BOOL             bCheckAllDelItemsRebuyPage; // NPC 재매입 ui에서 접속종료 시 재매입 아이템 확인.
    extern BOOL             bCheckSellItemRButton; // NPC 상점 ui에서 마우스 우클릭으로 아이템 판매 체크.
    extern BOOL             bCheckConfirmSellItem; // NPC 상점 ui에서 아이템 판매 시 확인.

	extern BOOL				bCONFT_BOUND;		//	대련시 영역 제한 여부.
	extern WORD				wCONFT_RECOVER;		//	대련시 회복약 제한.
	extern float			fCONFT_HP_RATE;		//	대련시 체력 확대 비율.
	extern float			fCONFT_TAR_HP_RATE;	//	대련시 체력 확대 비율 [ 상대방(개인대련시만) ].

	extern DWORD			dwPARTY_GET_ITEM;	//	파티옵션 아이템 분배 규칙.
	extern DWORD			dwPARTY_GET_MONEY;	//	파티옵션 돈 분배 규칙.
	
	extern int				QuickSlot[QUICK_SLOT_NUM]; // 퀵슬롯
	extern int				SkillSlot[QUICK_SKILL_NUM]; // 퀵스킬
	extern int				MenuShotcut[QUICK_MENU_NUM]; // 메뉴단축키
	extern int				DefaultShotcutKey[QUICK_ALL_NUM]; // 디폴트 키 타입
	extern int				KeySettingToRanparam[QUICK_MENU_NUM];
	extern int				BasicMenuToRanparam[BASICMENU_NUM];

	extern CString			ChatMacro[CHAT_MACRO_NUM];

	//	디스플레이 설정.
	extern BOOL				bCHECKWHQL;
	extern DWORD			dwScrWidth;
	extern DWORD			dwScrHeight;
	extern EMSCREEN_FORMAT	emScrFormat;
	extern UINT				uScrRefreshHz;
	extern BOOL				bScrWindowed;
	extern BOOL				bScrWndHalfSize;
	extern BOOL				bScrWndFullSize;
	extern BOOL				bGameCursor;

	extern bool				bDISP_CONSOLE;
	extern bool				bDISP_FPS;

	//	그래픽 설정.
	extern DWORD			dwVideoLevel;
	extern DWORD			dwShadowChar;	//	0~4
	extern BOOL				bBuff;
	extern BOOL				bShadowLand;
	extern BOOL				bRealReflect;
	extern BOOL				bRefract;
	extern BOOL				bGlow;
	extern BOOL				bPost;
	extern BOOL				bFrameLimit;
	extern DWORD			dwSkinDetail;	// 0~2
	extern DWORD			dwTextureFilter;

	extern BOOL				bBUFF_SFX;
	extern BOOL				bEMOTICON;

	extern BOOL				bClickEffect;
	extern BOOL				bTargetEffect;
	extern BOOL				bMineEffect;

	
	extern BOOL				bCharacterSimple;	/*character simple, Juver, 2017/10/01 */
	extern BOOL				bHideSkillEffect;	/*skill effect setting, Juver, 2017/10/01 */

	extern BOOL				bHideDamageDisplay;
	extern BOOL				bCharacterMisc;
	extern BOOL				bSpecialName;
	extern BOOL				bClassicNameDisplay;
	extern BOOL				bKillFeedDisplay;

	extern BOOL				bHideDeadName;
	extern BOOL				bHideDeadBody;
	extern BOOL				bPartyDisplay;
	extern BOOL				bPartyBuffDisplay;
	//	시야
	extern DWORD			dwFogRange;

	extern LONG				nSndSfx;
	extern LONG				nSndMap;
	extern LONG				nSndMusic;
	extern DWORD			dw3DAlgorithm;
	extern DWORD			dwSndBufferSize;

	extern BOOL				bSndMuteSfx;
	extern BOOL				bSndMuteMap;
	extern BOOL				bSndMuteMusic;	

	//	서버 설정.
	extern TCHAR			LoginAddress[STRING_NUM_128];
	extern UINT				nLoginPort; /*login port, Juver, 2017/11/16 */
	extern TCHAR			FtpAddressTable[MAX_FTP][STRING_NUM_128];
	extern TCHAR			HttpAddressTable[MAX_HTTP][STRING_NUM_128];	
	extern CString			strNewsURL;
	extern CString			strConnectionURL;
	extern BOOL				bUsePassiveDN;
	extern UINT				nChinaRegion; // 중국 - 패치서버 지역(MAX_CHINA_REGION:물어보기, 0~3:선택지역)
	
	// 중국 지역에 따른 서버 설정
	extern SERVER_SET		China_Region[MAX_CHINA_REGION];

	//	언어 폰트 설정.
	extern DWORD			dwLangSet;
	extern CString			strGDIFont;
	extern CString			strFontType;

	//	기본 설정 파일. ( 언어별 txt )
	extern CString			strCountry;
	extern CString			strLauncherText;
	extern CString			strGameInText;
	extern CString			strGameExText;
	extern CString			strGameWord;
	extern CString			strServerText;
	//xml translation
	extern CString			strGameWordNpc;
	extern CString			strGameWordQuest;
	
	extern CString			strLangPlacer;

	// 도움말 웹 페이지 주소 ( 준혁 )
	extern TCHAR			HelpAddress[STRING_NUM_128];
	extern TCHAR			ItemShopAddress[STRING_NUM_128];
	extern TCHAR			RPFDataPath[STRING_NUM_128];
	extern TCHAR			ClientWindowTitle[STRING_NUM_128];


	//temp variable
	extern BOOL bNPCShopConfirm; /*npc shop, Juver, 2017/07/27 */
	extern BOOL bItemPreviewStance; /*item preview, Juver, 2017/07/28 */
	extern BOOL	bVehicleWindowRun; /*vehicle system, Juver, 2017/08/08 */


	void SETIDPARAM ( std::string strPARAM );
	void SETIDPARAM2 ( std::string strPARAM );
	void SETIDPARAM3 ( std::string strPARAM );
	void SETJAPANUUID( std::string strUUID );

	const TCHAR* GETIDPARAM ();
	const TCHAR* GETIDPARAM2 ();
	const TCHAR* GETIDPARAM3 ();
	const TCHAR* GETJAPANUUID();

	/* anti-zoomout cheat fix, SeiferXIII 2020/05/17 */
	float GETZOOMOUTVALUE();
	void SETZOOMOUTVALUE( float fZoomOutValue );

	bool VALIDIDPARAM ();

	BOOL SETUSERID ( const TCHAR* szUSERID );
	BOOL SETCHARID ( const TCHAR* szCHARID );
	std::string GETCHARID();
	std::string GETUSERID_ENC ();
	std::string GETUSERID_DEC ();


	BOOL LOAD ( const TCHAR *szRootPath, bool bServer = false );
	BOOL LOAD_PARAM( const TCHAR *szRootPath );	

	//game options
	BOOL LOAD_GAMEOPTION( const TCHAR *szRootPath, bool bServer = false );	
	BOOL SAVE_GAMEOPTION ();

	//player profile
	void INIT_PROFILEDATA();
	BOOL SAVE_PLAYERPROFILE ();
	BOOL LOAD_PLAYERPROFILE ();

	//texture filter
	DWORD GetTextureFilter();
	DWORD GetTextureAF();

	//game feature
	BOOL LOAD_FEATURE( const TCHAR *szRootPath );	

	/*product item, Juver, 2017/10/18 */
	extern BOOL bFeatureProduct;

	/*quick slot delay, Juver, 2017/10/29 */
	extern BOOL bFeatureQuickSlotDelay;
	extern DWORD dwFeatureQuickSlotDelay;

	/*student record ui, Juver, 2017/10/30 */
	extern BOOL bFeatureStudentRecord;

	/*activity system, Juver, 2017/11/04 */
	extern BOOL bFeatureActivity;

	extern BOOL bFeatureCTF;
	extern BOOL bFeaturePVPInterface;
	extern BOOL bFeatureSW;

	extern float fFeatureSkillRangeClientMax;

	/*quest ui, Juver, 2017/11/06 */
	extern BOOL	bFeatureModernQuestWindow;

	/*charinfoview , Juver, 2017/11/11 */
	extern BOOL bFeatureViewCharInfo;

	/*register page, Juver, 2017/11/18 */
	extern BOOL bFeatureRegister;
	extern BOOL bFeatureRegisterUseMD5;

	/*character delete, Juver, 2017/11/18 */	
	extern BOOL bFeatureCharacterDelete;
	extern BOOL bFeatureCharacterDeleteMD5;

	/*recovery exp setting, Juver, 2017/11/18 */
	extern BOOL bFeatureUseRecoveryEXP;

	/*hide gm info, Juver, 2017/11/26 */
	extern BOOL bFeatureHideGMInfo;

	/*skill validity check, Juver, 2017/11/27 */
	extern WORD wFeatureSkillRangeCheckAdd;

	/*pet skill info display, Juver, 2017/12/01 */
	extern BOOL bFeaturePetSkilDisplay;

	/*regional chat, Juver, 2017/12/06 */
	extern BOOL bFeatureRegionalChatColor;
	extern BYTE nFeatureRegionalChatColorR;
	extern BYTE nFeatureRegionalChatColorG;
	extern BYTE nFeatureRegionalChatColorB;
	extern DWORD dwFeatureRegionalChatCost;

	/*block back damage, Juver, 2017/12/12 */
	extern BOOL bFeatureBlockDamageBack;
	extern float fFeatureBlockDamageBackTimer;

	/*clubwar verify non push or pull, Juver, 2017/12/13 */
	extern BOOL bFeatureClubWarVerifyDisablePushPull;

	/*staff name in chat, Juver, 2017/12/26 */
	extern BOOL bFeatureChatShowStaffName;

	/*change school card, Juver, 2018/01/12 */
	extern BOOL bFeatureChangeSchoolReqCleanQuest;
	extern BOOL bFeatureChangeSchoolReqCleanGuild;
	extern BOOL bFeatureChangeSchoolReqMaxLevel;
	extern BOOL bFeatureChangeSchoolWipeQuest;

	/*item transfer card, Juver, 2018/01/18 */
	extern DWORD dwFeatureItemTransferCost;

    extern BOOL bFeatureDisplayCP;

	/*dmk14 terrain check*/
	extern BOOL bTerrainCheck;
};
