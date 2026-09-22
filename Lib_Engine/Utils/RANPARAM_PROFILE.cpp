#include "stdafx.h"
#include "./RANPARAM.h"

#include "../Lib_Engine/Common/Rijndael.h"
#include "../Lib_Engine/Common/SUBPATH.h"
#include "../Lib_Engine/Common/gltexfile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace RANPARAM
{
	void INIT_PROFILEDATA()
	{
		bDIS_CONFT			= FALSE;		//	'자동' 대련 거부.
		bDIS_TRADE			= FALSE;		//	'자동' 거래 거부.
		bDIS_PARTY			= FALSE;		//	'자동' 파티 거부.
		bDIS_RIDE			= FALSE;		//	'자동' 파티 거부.
		bDIS_FRIEND			= FALSE;		//	'자동' 친구 요청 거부.
		bSHOW_SIMPLEHP		= TRUE;
		b3D_SOUND			= FALSE;
		bSHOW_TIP			= TRUE;			//	팁 보여주기
		bMOVABLE_ON_CHAT	= FALSE;		//	채팅창 위에서 움직이기
		bFORCED_ATTACK		= TRUE;			//	강제 공격 - 중국
		bNAME_DISPLAY		= TRUE;			//	이름 항상 표시
		bNON_Rebirth		= FALSE;		//  부활 스킬 금지
		bPrivateStats		= TRUE;			/*charinfoview , Juver, 2017/11/12 */

		bCheckUsableItemMarketPage = TRUE;      // NPC 상점 ui에서 사용 가능한 아이템 체크.
		bCheckBuyableItemMarketPage =  TRUE;    // NPC 상점 ui에서 구입 가능한 아이템 체크.
		bCheckRebuySuppliesRebuyPage = TRUE; // NPC 재매입 ui에서 소모품 아이템 체크.
		bCheckAllDelItemsRebuyPage = TRUE; // NPC 재매입 ui에서 접속종료 시 재매입 아이템 확인.
		bCheckSellItemRButton = FALSE; // NPC 상점 ui에서 마우스 우클릭으로 아이템 판매 체크.
		bCheckConfirmSellItem = TRUE; // NPC 상점 ui에서 아이템 판매 시 확인.

		bCONFT_BOUND		= TRUE;			//	대련시 영역 제한 여부.
		wCONFT_RECOVER		= 0;			//	대련시 회복약 제한.
		fCONFT_HP_RATE		= 1.0f;			//	대련시 체력 확대 비율.
		fCONFT_TAR_HP_RATE	= 1.0f;			//	대련시 체력 확대 비율 [ 상대방(개인대련시만) ].

		dwPARTY_GET_ITEM	= 3;			//	파티옵션 아이템 분배 규칙.(디폴트는 순차습득)
		dwPARTY_GET_MONEY	= 3;			//	파티옵션 돈 분배 규칙.(디폴트는 균등분배)

		for( int i = 0; i < QUICK_SLOT_NUM; ++i){
			QuickSlot[i]  =	DefaultShotcutKey[i];
		}

		for( int i = 0; i < QUICK_SKILL_NUM; ++i){
			SkillSlot[i]  =	DefaultShotcutKey[i+QUICK_SLOT_NUM];
		}

		for( int i = 0; i < QUICK_MENU_NUM; ++i){
			MenuShotcut[i] = DefaultShotcutKey[i+QUICK_SLOT_NUM+QUICK_SKILL_NUM];
		}

		for( int i = 0; i < CHAT_MACRO_NUM; ++i){
			ChatMacro[i] = "";
		}
	}

	BOOL SAVE_PLAYERPROFILE ()
	{
		std::string strUSERID = GETCHARID(); // 파일명 가져오는 부분...
		if ( strUSERID.empty() )
		{
			return FALSE;
		}

		TCHAR szPROFILE[MAX_PATH]={0};
		SHGetSpecialFolderPath( NULL, szPROFILE, CSIDL_PERSONAL, FALSE );

		std::string strOPTFILE(szPROFILE);
		strOPTFILE += SUBPATH::PLAYINFO_ROOT;
		strOPTFILE += strUSERID;
		strOPTFILE += _T(".gameopt");

		FILE* file=NULL;
		file = fopen ( strOPTFILE.c_str(), "wt" );
		if ( !file )	return FALSE;

		fprintf ( file, "\n" );
		fprintf ( file, "// GAME OPTION\n" );

		fprintf ( file, "EMGAMEOPT_VER\t%d\n", EMGAMEOPT_VER );
		fprintf ( file, "bDIS_CONFT\t%d\n", bDIS_CONFT );
		fprintf ( file, "bDIS_TRADE\t%d\n", bDIS_TRADE );
		fprintf ( file, "bDIS_PARTY\t%d\n", bDIS_PARTY );
		fprintf ( file, "bDIS_RIDE\t%d\n", bDIS_RIDE );
		fprintf ( file, "bDIS_FRIEND\t%d\n", bDIS_FRIEND );	
		fprintf ( file, "bSHOW_SIMPLEHP\t%d\n", bSHOW_SIMPLEHP );	
		fprintf ( file, "bSHOW_TIP\t%d\n", bSHOW_TIP );
		fprintf ( file, "bMOVABLE_ON_CHAT\t%d\n", bMOVABLE_ON_CHAT );
		fprintf ( file, "bFORCED_ATTACK\t%d\n", bFORCED_ATTACK );
		fprintf ( file, "bNAME_DISPLAY\t%d\n", bNAME_DISPLAY );
		fprintf ( file, "bNON_Rebirth\t%d\n", bNON_Rebirth );
		fprintf ( file, "bPrivateStats\t%d\n", bPrivateStats ); /*charinfoview , Juver, 2017/11/12 */

		fprintf( file, "bCheckUsableItemMarketPage\t%d\n", bCheckUsableItemMarketPage );
		fprintf( file, "bCheckBuyableItemMarketPage\t%d\n", bCheckBuyableItemMarketPage );
		fprintf( file, "bCheckSellItemRButton\t%d\n", bCheckSellItemRButton );
		fprintf( file, "bCheckConfirmSellItem\t%d\n", bCheckConfirmSellItem );

		fprintf ( file, "bCONFT_BOUND\t%d\n", bCONFT_BOUND );
		fprintf ( file, "wCONFT_RECOVER\t%d\n", wCONFT_RECOVER );
		fprintf ( file, "fCONFT_HP_RATE\t%1.1f\n", fCONFT_HP_RATE );
		fprintf ( file, "fCONFT_TAR_HP_RATE\t%1.1f\n", fCONFT_TAR_HP_RATE );

		fprintf ( file, "dwPARTY_GET_ITEM\t%d\n", dwPARTY_GET_ITEM );
		fprintf ( file, "dwPARTY_GET_MONEY\t%d\n", dwPARTY_GET_MONEY );


		CString strTemp;

		// QUICK_POTION_SHOTCUT
		for ( int i=0; i< QUICK_SLOT_NUM; ++i){
			if(QuickSlot[i] > 0){
				strTemp.Format("QUICK_SLOT%d\t%d\n", i, QuickSlot[i]);			
				fprintf ( file, strTemp);
			}
		}

		// QUICK_SKILL_SHOTCUT
		for ( int i=0; i< QUICK_SKILL_NUM; ++i){
			if(SkillSlot[i] > 0){
				strTemp.Format("SKILL_SLOT%d\t%d\n", i, SkillSlot[i]);
				fprintf ( file, strTemp);
			}
		}

		// MENU_SHOTCUT
		fprintf ( file, "SHOTCUT_INVEN\t%d\n", MenuShotcut[SHOTCUT_INVEN] );
		fprintf ( file, "SHOTCUT_CHAR\t%d\n", MenuShotcut[SHOTCUT_CHAR] );
		fprintf ( file, "SHOTCUT_SKILL\t%d\n", MenuShotcut[SHOTCUT_SKILL] );
		fprintf ( file, "SHOTCUT_PARTY\t%d\n", MenuShotcut[SHOTCUT_PARTY] );
		fprintf ( file, "SHOTCUT_QUEST\t%d\n", MenuShotcut[SHOTCUT_QUEST] );
		fprintf ( file, "SHOTCUT_CLUB\t%d\n", MenuShotcut[SHOTCUT_CLUB] );
		fprintf ( file, "SHOTCUT_FRIEND\t%d\n", MenuShotcut[SHOTCUT_FRIEND] );
		fprintf ( file, "SHOTCUT_MAP\t%d\n", MenuShotcut[SHOTCUT_MAP] );
		fprintf ( file, "SHOTCUT_ITEMBANK\t%d\n", MenuShotcut[SHOTCUT_ITEMBANK] );
		fprintf ( file, "SHOTCUT_RUN\t%d\n", MenuShotcut[SHOTCUT_RUN] );
		fprintf ( file, "SHOTCUT_HELP\t%d\n", MenuShotcut[SHOTCUT_HELP] );
		fprintf ( file, "SHOTCUT_PET\t%d\n", MenuShotcut[SHOTCUT_PET] );
		fprintf ( file, "SHOTCUT_ATTACKMODE\t%d\n", MenuShotcut[SHOTCUT_ATTACKMODE] );
		fprintf ( file, "SHOTCUT_PKMODE\t%d\n", MenuShotcut[SHOTCUT_PKMODE] );
		fprintf ( file, "SHOTCUT_ITEMSHOP\t%d\n", MenuShotcut[SHOTCUT_ITEMSHOP] );
		fprintf ( file, "SHOTCUT_CHATMACRO\t%d\n", MenuShotcut[SHOTCUT_CHATMACRO] );
		fprintf ( file, "SHOTCUT_SUMMON\t%d\n", MenuShotcut[SHOTCUT_SUMMON] );

		// 채팅 매크로
		CString strMacro;
		for ( int i=0; i< CHAT_MACRO_NUM; ++i){
			if(ChatMacro[i].GetLength() > 0){
				strMacro = ChatMacro[i];
				strMacro.Replace("%", "%%");
				strTemp.Format("CHATMACRO%d\t=\t%s\n", i, strMacro);
				fprintf ( file, strTemp);
			}
		}

		fclose ( file );

		return TRUE;
	}

	BOOL LOAD_PLAYERPROFILE ()
	{
		INIT_PROFILEDATA();
		std::string strUSERID = GETCHARID(); // 파일명 가져오는 부분...
		if ( strUSERID.empty() )
		{
			MessageBox ( NULL, "LOAD_PLAYERPROFILE Empty UserID!", "ERROR", MB_OK );
			return FALSE;
		}

		TCHAR szPROFILE[MAX_PATH]={0};
		SHGetSpecialFolderPath( NULL, szPROFILE, CSIDL_PERSONAL, FALSE );

		std::string strOPTFILE(szPROFILE);
		strOPTFILE += SUBPATH::PLAYINFO_ROOT;
		strOPTFILE += strUSERID;
		strOPTFILE += _T(".gameopt");

		gltexfile cFILE;
		cFILE.reg_sep ( '\t' );
		//		cFILE.reg_sep ( ' ' );

		if ( !cFILE.open(strOPTFILE.c_str(),false) )	return FALSE;

		DWORD dwVER;
		bool bok = cFILE.getflag ( "EMGAMEOPT_VER", 1, 1, dwVER );
		if ( !bok )		return FALSE;		


		if( dwVER == EMGAMEOPT_VER ) // 채팅 매크로 버전
		{
			cFILE.getflag ( "bDIS_CONFT", 1, 1, bDIS_CONFT );
			cFILE.getflag ( "bDIS_TRADE", 1, 1, bDIS_TRADE );
			cFILE.getflag ( "bDIS_PARTY", 1, 1, bDIS_PARTY );
			cFILE.getflag ( "bDIS_RIDE", 1, 1, bDIS_RIDE );
			cFILE.getflag ( "bDIS_FRIEND", 1, 1, bDIS_FRIEND );		
			cFILE.getflag ( "bSHOW_SIMPLEHP", 1, 1, bSHOW_SIMPLEHP );
			cFILE.getflag ( "bSHOW_TIP", 1, 1, bSHOW_TIP );
			cFILE.getflag ( "bMOVABLE_ON_CHAT", 1, 1, bMOVABLE_ON_CHAT );
			cFILE.getflag ( "bFORCED_ATTACK", 1, 1, bFORCED_ATTACK );
			cFILE.getflag ( "bNAME_DISPLAY", 1, 1, bNAME_DISPLAY );
			cFILE.getflag ( "bNON_Rebirth", 1, 1, bNON_Rebirth );
			cFILE.getflag ( "bPrivateStats", 1, 1, bPrivateStats ); /*charinfoview , Juver, 2017/11/12 */
			
			cFILE.getflag( "bCheckUsableItemMarketPage", 1, 1, bCheckUsableItemMarketPage );
			cFILE.getflag( "bCheckBuyableItemMarketPage", 1, 1, bCheckBuyableItemMarketPage );
			cFILE.getflag( "bCheckSellItemRButton", 1, 1, bCheckSellItemRButton );
			cFILE.getflag( "bCheckConfirmSellItem", 1, 1, bCheckConfirmSellItem );

			cFILE.getflag ( "bCONFT_BOUND", 1, 1, bCONFT_BOUND );
			cFILE.getflag ( "wCONFT_RECOVER", 1, 1, wCONFT_RECOVER );
			cFILE.getflag ( "fCONFT_HP_RATE", 1, 1, fCONFT_HP_RATE );
			cFILE.getflag ( "fCONFT_TAR_HP_RATE", 1, 1, fCONFT_TAR_HP_RATE );

			cFILE.getflag ( "dwPARTY_GET_ITEM", 1, 1, dwPARTY_GET_ITEM );
			cFILE.getflag ( "dwPARTY_GET_MONEY", 1, 1, dwPARTY_GET_MONEY );

			CString strTemp;

			// QUICK_POTION_SHOTCUT
			for ( int i = 0; i< QUICK_SLOT_NUM; ++i){
				strTemp.Format("QUICK_SLOT%d", i);
				cFILE.getflag ( (LPCTSTR)strTemp, 1, 1, QuickSlot[i] );
			}

			// QUICK_SKILL_SHOTCUT
			for ( int i = 0; i< QUICK_SKILL_NUM; ++i){
				strTemp.Format("SKILL_SLOT%d", i);
				cFILE.getflag ( (LPCTSTR)strTemp, 1, 1, SkillSlot[i] );
			}

			// MENU_SHOTCUT
			cFILE.getflag ("SHOTCUT_INVEN", 1, 1, MenuShotcut[SHOTCUT_INVEN] );
			cFILE.getflag ("SHOTCUT_CHAR", 1, 1, MenuShotcut[SHOTCUT_CHAR] );
			cFILE.getflag ("SHOTCUT_SKILL", 1, 1, MenuShotcut[SHOTCUT_SKILL] );
			cFILE.getflag ("SHOTCUT_PARTY", 1, 1, MenuShotcut[SHOTCUT_PARTY] );
			cFILE.getflag ("SHOTCUT_QUEST", 1, 1, MenuShotcut[SHOTCUT_QUEST] );
			cFILE.getflag ("SHOTCUT_CLUB", 1, 1, MenuShotcut[SHOTCUT_CLUB] );
			cFILE.getflag ("SHOTCUT_FRIEND", 1, 1, MenuShotcut[SHOTCUT_FRIEND] );
			cFILE.getflag ("SHOTCUT_MAP", 1, 1, MenuShotcut[SHOTCUT_MAP] );
			cFILE.getflag ("SHOTCUT_ITEMBANK", 1, 1, MenuShotcut[SHOTCUT_ITEMBANK] );
			cFILE.getflag ("SHOTCUT_RUN", 1, 1, MenuShotcut[SHOTCUT_RUN] );
			cFILE.getflag ("SHOTCUT_HELP", 1, 1, MenuShotcut[SHOTCUT_HELP] );
			cFILE.getflag ("SHOTCUT_PET", 1, 1, MenuShotcut[SHOTCUT_PET] );
			cFILE.getflag ("SHOTCUT_ATTACKMODE", 1, 1, MenuShotcut[SHOTCUT_ATTACKMODE] );
			cFILE.getflag ("SHOTCUT_PKMODE", 1, 1, MenuShotcut[SHOTCUT_PKMODE] );
			cFILE.getflag ("SHOTCUT_ITEMSHOP", 1, 1, MenuShotcut[SHOTCUT_ITEMSHOP] );
			cFILE.getflag ("SHOTCUT_CHATMACRO", 1, 1, MenuShotcut[SHOTCUT_CHATMACRO] );
			cFILE.getflag ("SHOTCUT_SUMMON", 1, 1, MenuShotcut[SHOTCUT_SUMMON] );

			for ( int i = 0; i< CHAT_MACRO_NUM; ++i){
				strTemp.Format("CHATMACRO%d", i);
				cFILE.getflag ( (LPCTSTR)strTemp, 2, 2, ChatMacro[i] );
			}
		}
		else if( dwVER == 0x0111 ) // 채팅 매크로 버전
		{
			cFILE.getflag ( "bDIS_CONFT", 1, 1, bDIS_CONFT );
			cFILE.getflag ( "bDIS_TRADE", 1, 1, bDIS_TRADE );
			cFILE.getflag ( "bDIS_PARTY", 1, 1, bDIS_PARTY );
			cFILE.getflag ( "bDIS_FRIEND", 1, 1, bDIS_FRIEND );		
			cFILE.getflag ( "bSHOW_SIMPLEHP", 1, 1, bSHOW_SIMPLEHP );
			cFILE.getflag ( "bSHOW_TIP", 1, 1, bSHOW_TIP );
			cFILE.getflag ( "bMOVABLE_ON_CHAT", 1, 1, bMOVABLE_ON_CHAT );
			cFILE.getflag ( "bFORCED_ATTACK", 1, 1, bFORCED_ATTACK );
			cFILE.getflag ( "bNAME_DISPLAY", 1, 1, bNAME_DISPLAY );
			cFILE.getflag ( "bNON_Rebirth", 1, 1, bNON_Rebirth );

			cFILE.getflag ( "bCONFT_BOUND", 1, 1, bCONFT_BOUND );
			cFILE.getflag ( "wCONFT_RECOVER", 1, 1, wCONFT_RECOVER );
			cFILE.getflag ( "fCONFT_HP_RATE", 1, 1, fCONFT_HP_RATE );
			cFILE.getflag ( "fCONFT_TAR_HP_RATE", 1, 1, fCONFT_TAR_HP_RATE );

			cFILE.getflag ( "dwPARTY_GET_ITEM", 1, 1, dwPARTY_GET_ITEM );
			cFILE.getflag ( "dwPARTY_GET_MONEY", 1, 1, dwPARTY_GET_MONEY );

			CString strTemp;

			// QUICK_POTION_SHOTCUT
			for ( int i = 0; i< QUICK_SLOT_NUM; ++i){
				strTemp.Format("QUICK_SLOT%d", i);
				cFILE.getflag ( (LPCTSTR)strTemp, 1, 1, QuickSlot[i] );
			}

			// QUICK_SKILL_SHOTCUT
			for ( int i = 0; i< QUICK_SKILL_NUM; ++i){
				strTemp.Format("SKILL_SLOT%d", i);
				cFILE.getflag ( (LPCTSTR)strTemp, 1, 1, SkillSlot[i] );
			}

			// MENU_SHOTCUT
			cFILE.getflag ("SHOTCUT_INVEN", 1, 1, MenuShotcut[SHOTCUT_INVEN] );
			cFILE.getflag ("SHOTCUT_CHAR", 1, 1, MenuShotcut[SHOTCUT_CHAR] );
			cFILE.getflag ("SHOTCUT_SKILL", 1, 1, MenuShotcut[SHOTCUT_SKILL] );
			cFILE.getflag ("SHOTCUT_PARTY", 1, 1, MenuShotcut[SHOTCUT_PARTY] );
			cFILE.getflag ("SHOTCUT_QUEST", 1, 1, MenuShotcut[SHOTCUT_QUEST] );
			cFILE.getflag ("SHOTCUT_CLUB", 1, 1, MenuShotcut[SHOTCUT_CLUB] );
			cFILE.getflag ("SHOTCUT_FRIEND", 1, 1, MenuShotcut[SHOTCUT_FRIEND] );
			cFILE.getflag ("SHOTCUT_MAP", 1, 1, MenuShotcut[SHOTCUT_MAP] );
			cFILE.getflag ("SHOTCUT_ITEMBANK", 1, 1, MenuShotcut[SHOTCUT_ITEMBANK] );
			cFILE.getflag ("SHOTCUT_RUN", 1, 1, MenuShotcut[SHOTCUT_RUN] );
			cFILE.getflag ("SHOTCUT_HELP", 1, 1, MenuShotcut[SHOTCUT_HELP] );
			cFILE.getflag ("SHOTCUT_PET", 1, 1, MenuShotcut[SHOTCUT_PET] );
			cFILE.getflag ("SHOTCUT_ATTACKMODE", 1, 1, MenuShotcut[SHOTCUT_ATTACKMODE] );
			cFILE.getflag ("SHOTCUT_PKMODE", 1, 1, MenuShotcut[SHOTCUT_PKMODE] );
			cFILE.getflag ("SHOTCUT_ITEMSHOP", 1, 1, MenuShotcut[SHOTCUT_ITEMSHOP] );
			cFILE.getflag ("SHOTCUT_CHATMACRO", 1, 1, MenuShotcut[SHOTCUT_CHATMACRO] );
			cFILE.getflag ("SHOTCUT_SUMMON", 1, 1, MenuShotcut[SHOTCUT_SUMMON] );

			for ( int i = 0; i< CHAT_MACRO_NUM; ++i){
				strTemp.Format("CHATMACRO%d", i);
				cFILE.getflag ( (LPCTSTR)strTemp, 2, 2, ChatMacro[i] );
			}
		}
		else if( dwVER == 0x0110 ) // 채팅 매크로 버전
		{
			cFILE.getflag ( "bDIS_CONFT", 1, 1, bDIS_CONFT );
			cFILE.getflag ( "bDIS_TRADE", 1, 1, bDIS_TRADE );
			cFILE.getflag ( "bDIS_PARTY", 1, 1, bDIS_PARTY );
			cFILE.getflag ( "bDIS_FRIEND", 1, 1, bDIS_FRIEND );		
			cFILE.getflag ( "bSHOW_SIMPLEHP", 1, 1, bSHOW_SIMPLEHP );
			cFILE.getflag ( "bSHOW_TIP", 1, 1, bSHOW_TIP );
			cFILE.getflag ( "bMOVABLE_ON_CHAT", 1, 1, bMOVABLE_ON_CHAT );
			cFILE.getflag ( "bFORCED_ATTACK", 1, 1, bFORCED_ATTACK );
			cFILE.getflag ( "bNAME_DISPLAY", 1, 1, bNAME_DISPLAY );
			cFILE.getflag ( "bNON_Rebirth", 1, 1, bNON_Rebirth );

			cFILE.getflag ( "bCONFT_BOUND", 1, 1, bCONFT_BOUND );
			cFILE.getflag ( "wCONFT_RECOVER", 1, 1, wCONFT_RECOVER );
			cFILE.getflag ( "fCONFT_HP_RATE", 1, 1, fCONFT_HP_RATE );
			cFILE.getflag ( "fCONFT_TAR_HP_RATE", 1, 1, fCONFT_TAR_HP_RATE );

			cFILE.getflag ( "dwPARTY_GET_ITEM", 1, 1, dwPARTY_GET_ITEM );
			cFILE.getflag ( "dwPARTY_GET_MONEY", 1, 1, dwPARTY_GET_MONEY );

			CString strTemp;

			// QUICK_POTION_SHOTCUT
			for ( int i = 0; i< QUICK_SLOT_NUM; ++i){
				strTemp.Format("QUICK_SLOT%d", i);
				cFILE.getflag ( (LPCTSTR)strTemp, 1, 1, QuickSlot[i] );
			}

			// QUICK_SKILL_SHOTCUT
			for ( int i = 0; i< QUICK_SKILL_NUM; ++i){
				strTemp.Format("SKILL_SLOT%d", i);
				cFILE.getflag ( (LPCTSTR)strTemp, 1, 1, SkillSlot[i] );
			}

			// MENU_SHOTCUT
			cFILE.getflag ("SHOTCUT_INVEN", 1, 1, MenuShotcut[SHOTCUT_INVEN] );
			cFILE.getflag ("SHOTCUT_CHAR", 1, 1, MenuShotcut[SHOTCUT_CHAR] );
			cFILE.getflag ("SHOTCUT_SKILL", 1, 1, MenuShotcut[SHOTCUT_SKILL] );
			cFILE.getflag ("SHOTCUT_PARTY", 1, 1, MenuShotcut[SHOTCUT_PARTY] );
			cFILE.getflag ("SHOTCUT_QUEST", 1, 1, MenuShotcut[SHOTCUT_QUEST] );
			cFILE.getflag ("SHOTCUT_CLUB", 1, 1, MenuShotcut[SHOTCUT_CLUB] );
			cFILE.getflag ("SHOTCUT_FRIEND", 1, 1, MenuShotcut[SHOTCUT_FRIEND] );
			cFILE.getflag ("SHOTCUT_MAP", 1, 1, MenuShotcut[SHOTCUT_MAP] );
			cFILE.getflag ("SHOTCUT_ITEMBANK", 1, 1, MenuShotcut[SHOTCUT_ITEMBANK] );
			cFILE.getflag ("SHOTCUT_RUN", 1, 1, MenuShotcut[SHOTCUT_RUN] );
			cFILE.getflag ("SHOTCUT_HELP", 1, 1, MenuShotcut[SHOTCUT_HELP] );
			cFILE.getflag ("SHOTCUT_PET", 1, 1, MenuShotcut[SHOTCUT_PET] );
			cFILE.getflag ("SHOTCUT_ATTACKMODE", 1, 1, MenuShotcut[SHOTCUT_ATTACKMODE] );
			cFILE.getflag ("SHOTCUT_PKMODE", 1, 1, MenuShotcut[SHOTCUT_PKMODE] );
			cFILE.getflag ("SHOTCUT_ITEMSHOP", 1, 1, MenuShotcut[SHOTCUT_ITEMSHOP] );
			cFILE.getflag ("SHOTCUT_CHATMACRO", 1, 1, MenuShotcut[SHOTCUT_CHATMACRO] );

			// 채팅 매크로
			for ( int i = 0; i< CHAT_MACRO_NUM; ++i){
				strTemp.Format("CHATMACRO%d", i);
				cFILE.getflag ( (LPCTSTR)strTemp, 2, 2, ChatMacro[i] );
			}
		}
		else if( dwVER == 0x0109 ) // 채팅 매크로 버전
		{
			cFILE.getflag ( "bDIS_CONFT", 1, 1, bDIS_CONFT );
			cFILE.getflag ( "bDIS_TRADE", 1, 1, bDIS_TRADE );
			cFILE.getflag ( "bDIS_PARTY", 1, 1, bDIS_PARTY );
			cFILE.getflag ( "bDIS_FRIEND", 1, 1, bDIS_FRIEND );		
			cFILE.getflag ( "bSHOW_SIMPLEHP", 1, 1, bSHOW_SIMPLEHP );
			cFILE.getflag ( "bSHOW_TIP", 1, 1, bSHOW_TIP );
			cFILE.getflag ( "bMOVABLE_ON_CHAT", 1, 1, bMOVABLE_ON_CHAT );
			cFILE.getflag ( "bFORCED_ATTACK", 1, 1, bFORCED_ATTACK );
			cFILE.getflag ( "bNAME_DISPLAY", 1, 1, bNAME_DISPLAY );

			cFILE.getflag ( "bCONFT_BOUND", 1, 1, bCONFT_BOUND );
			cFILE.getflag ( "wCONFT_RECOVER", 1, 1, wCONFT_RECOVER );
			cFILE.getflag ( "fCONFT_HP_RATE", 1, 1, fCONFT_HP_RATE );
			cFILE.getflag ( "fCONFT_TAR_HP_RATE", 1, 1, fCONFT_TAR_HP_RATE );

			cFILE.getflag ( "dwPARTY_GET_ITEM", 1, 1, dwPARTY_GET_ITEM );
			cFILE.getflag ( "dwPARTY_GET_MONEY", 1, 1, dwPARTY_GET_MONEY );

			CString strTemp;

			// QUICK_POTION_SHOTCUT
			for ( int i = 0; i< QUICK_SLOT_NUM; ++i){
				strTemp.Format("QUICK_SLOT%d", i);
				cFILE.getflag ( (LPCTSTR)strTemp, 1, 1, QuickSlot[i] );
			}

			// QUICK_SKILL_SHOTCUT
			for ( int i = 0; i< QUICK_SKILL_NUM; ++i){
				strTemp.Format("SKILL_SLOT%d", i);
				cFILE.getflag ( (LPCTSTR)strTemp, 1, 1, SkillSlot[i] );
			}

			// MENU_SHOTCUT
			cFILE.getflag ("SHOTCUT_INVEN", 1, 1, MenuShotcut[SHOTCUT_INVEN] );
			cFILE.getflag ("SHOTCUT_CHAR", 1, 1, MenuShotcut[SHOTCUT_CHAR] );
			cFILE.getflag ("SHOTCUT_SKILL", 1, 1, MenuShotcut[SHOTCUT_SKILL] );
			cFILE.getflag ("SHOTCUT_PARTY", 1, 1, MenuShotcut[SHOTCUT_PARTY] );
			cFILE.getflag ("SHOTCUT_QUEST", 1, 1, MenuShotcut[SHOTCUT_QUEST] );
			cFILE.getflag ("SHOTCUT_CLUB", 1, 1, MenuShotcut[SHOTCUT_CLUB] );
			cFILE.getflag ("SHOTCUT_FRIEND", 1, 1, MenuShotcut[SHOTCUT_FRIEND] );
			cFILE.getflag ("SHOTCUT_MAP", 1, 1, MenuShotcut[SHOTCUT_MAP] );
			cFILE.getflag ("SHOTCUT_ITEMBANK", 1, 1, MenuShotcut[SHOTCUT_ITEMBANK] );
			cFILE.getflag ("SHOTCUT_RUN", 1, 1, MenuShotcut[SHOTCUT_RUN] );
			cFILE.getflag ("SHOTCUT_HELP", 1, 1, MenuShotcut[SHOTCUT_HELP] );
			cFILE.getflag ("SHOTCUT_PET", 1, 1, MenuShotcut[SHOTCUT_PET] );
			cFILE.getflag ("SHOTCUT_ATTACKMODE", 1, 1, MenuShotcut[SHOTCUT_ATTACKMODE] );
			cFILE.getflag ("SHOTCUT_PKMODE", 1, 1, MenuShotcut[SHOTCUT_PKMODE] );
			cFILE.getflag ("SHOTCUT_ITEMSHOP", 1, 1, MenuShotcut[SHOTCUT_ITEMSHOP] );
			cFILE.getflag ("SHOTCUT_CHATMACRO", 1, 1, MenuShotcut[SHOTCUT_CHATMACRO] );

			// 채팅 매크로
			for ( int i = 0; i< CHAT_MACRO_NUM; ++i){
				strTemp.Format("CHATMACRO%d", i);
				cFILE.getflag ( (LPCTSTR)strTemp, 2, 2, ChatMacro[i] );
			}
		}
		else if ( dwVER==0x0108 ) // 단축키 저장 버전
		{ 
			cFILE.getflag ( "bDIS_CONFT", 1, 1, bDIS_CONFT );
			cFILE.getflag ( "bDIS_TRADE", 1, 1, bDIS_TRADE );
			cFILE.getflag ( "bDIS_PARTY", 1, 1, bDIS_PARTY );
			cFILE.getflag ( "bDIS_FRIEND", 1, 1, bDIS_FRIEND );		
			cFILE.getflag ( "bSHOW_SIMPLEHP", 1, 1, bSHOW_SIMPLEHP );
			cFILE.getflag ( "bSHOW_TIP", 1, 1, bSHOW_TIP );
			cFILE.getflag ( "bMOVABLE_ON_CHAT", 1, 1, bMOVABLE_ON_CHAT );
			cFILE.getflag ( "bFORCED_ATTACK", 1, 1, bFORCED_ATTACK );
			cFILE.getflag ( "bNAME_DISPLAY", 1, 1, bNAME_DISPLAY );

			cFILE.getflag ( "bCONFT_BOUND", 1, 1, bCONFT_BOUND );
			cFILE.getflag ( "wCONFT_RECOVER", 1, 1, wCONFT_RECOVER );
			cFILE.getflag ( "fCONFT_HP_RATE", 1, 1, fCONFT_HP_RATE );
			cFILE.getflag ( "fCONFT_TAR_HP_RATE", 1, 1, fCONFT_TAR_HP_RATE );

			cFILE.getflag ( "dwPARTY_GET_ITEM", 1, 1, dwPARTY_GET_ITEM );
			cFILE.getflag ( "dwPARTY_GET_MONEY", 1, 1, dwPARTY_GET_MONEY );

			CString strTemp;

			// QUICK_POTION_SHOTCUT
			for ( int i = 0; i< QUICK_SLOT_NUM; ++i){
				strTemp.Format("QUICK_SLOT%d", i);
				cFILE.getflag ( (LPCTSTR)strTemp, 1, 1, QuickSlot[i] );
			}

			// QUICK_SKILL_SHOTCUT
			for ( int i = 0; i< QUICK_SKILL_NUM; ++i){
				strTemp.Format("SKILL_SLOT%d", i);
				cFILE.getflag ( (LPCTSTR)strTemp, 1, 1, SkillSlot[i] );
			}

			// QUICK_MENU_SHOTCUT
			for ( int i = 0; i< QUICK_MENU_NUM; ++i){
				strTemp.Format("MENU_SHOTCUT%d", i);
				cFILE.getflag ( (LPCTSTR)strTemp, 1, 1, MenuShotcut[i] );
			}
		}
		else if ( dwVER==0x0107 )
		{
			cFILE.getflag ( "bDIS_CONFT", 1, 1, bDIS_CONFT );
			cFILE.getflag ( "bDIS_TRADE", 1, 1, bDIS_TRADE );
			cFILE.getflag ( "bDIS_PARTY", 1, 1, bDIS_PARTY );
			cFILE.getflag ( "bDIS_FRIEND", 1, 1, bDIS_FRIEND );		
			cFILE.getflag ( "bSHOW_SIMPLEHP", 1, 1, bSHOW_SIMPLEHP );
			cFILE.getflag ( "bSHOW_TIP", 1, 1, bSHOW_TIP );
			cFILE.getflag ( "bMOVABLE_ON_CHAT", 1, 1, bMOVABLE_ON_CHAT );
			cFILE.getflag ( "bFORCED_ATTACK", 1, 1, bFORCED_ATTACK );
			cFILE.getflag ( "bNAME_DISPLAY", 1, 1, bNAME_DISPLAY );

			cFILE.getflag ( "bCONFT_BOUND", 1, 1, bCONFT_BOUND );
			cFILE.getflag ( "wCONFT_RECOVER", 1, 1, wCONFT_RECOVER );
			cFILE.getflag ( "fCONFT_HP_RATE", 1, 1, fCONFT_HP_RATE );
			cFILE.getflag ( "fCONFT_TAR_HP_RATE", 1, 1, fCONFT_TAR_HP_RATE );

			cFILE.getflag ( "dwPARTY_GET_ITEM", 1, 1, dwPARTY_GET_ITEM );
			cFILE.getflag ( "dwPARTY_GET_MONEY", 1, 1, dwPARTY_GET_MONEY );
		}
		else if ( dwVER==0x0106 )
		{
			cFILE.getflag ( "bDIS_CONFT", 1, 1, bDIS_CONFT );
			cFILE.getflag ( "bDIS_TRADE", 1, 1, bDIS_TRADE );
			cFILE.getflag ( "bDIS_PARTY", 1, 1, bDIS_PARTY );
			cFILE.getflag ( "bDIS_FRIEND", 1, 1, bDIS_FRIEND );		
			cFILE.getflag ( "bSHOW_SIMPLEHP", 1, 1, bSHOW_SIMPLEHP );
			cFILE.getflag ( "bSHOW_TIP", 1, 1, bSHOW_TIP );
			cFILE.getflag ( "bMOVABLE_ON_CHAT", 1, 1, bMOVABLE_ON_CHAT );

			cFILE.getflag ( "bCONFT_BOUND", 1, 1, bCONFT_BOUND );
			cFILE.getflag ( "wCONFT_RECOVER", 1, 1, wCONFT_RECOVER );
			cFILE.getflag ( "fCONFT_HP_RATE", 1, 1, fCONFT_HP_RATE );
			cFILE.getflag ( "fCONFT_TAR_HP_RATE", 1, 1, fCONFT_TAR_HP_RATE );

			cFILE.getflag ( "dwPARTY_GET_ITEM", 1, 1, dwPARTY_GET_ITEM );
			cFILE.getflag ( "dwPARTY_GET_MONEY", 1, 1, dwPARTY_GET_MONEY );
		}
		else if ( dwVER==0x0105 )
		{
			cFILE.getflag ( "bDIS_CONFT", 1, 1, bDIS_CONFT );
			cFILE.getflag ( "bDIS_TRADE", 1, 1, bDIS_TRADE );
			cFILE.getflag ( "bDIS_PARTY", 1, 1, bDIS_PARTY );
			cFILE.getflag ( "bDIS_FRIEND", 1, 1, bDIS_FRIEND );		
			cFILE.getflag ( "bSHOW_SIMPLEHP", 1, 1, bSHOW_SIMPLEHP );		

			cFILE.getflag ( "bCONFT_BOUND", 1, 1, bCONFT_BOUND );
			cFILE.getflag ( "wCONFT_RECOVER", 1, 1, wCONFT_RECOVER );
			cFILE.getflag ( "fCONFT_HP_RATE", 1, 1, fCONFT_HP_RATE );
			cFILE.getflag ( "fCONFT_TAR_HP_RATE", 1, 1, fCONFT_TAR_HP_RATE );

			cFILE.getflag ( "dwPARTY_GET_ITEM", 1, 1, dwPARTY_GET_ITEM );
			cFILE.getflag ( "dwPARTY_GET_MONEY", 1, 1, dwPARTY_GET_MONEY );
		}
		else if ( dwVER==0x0104 || dwVER==0x0103 )
		{
			cFILE.getflag ( "bDIS_CONFT", 1, 1, bDIS_CONFT );
			cFILE.getflag ( "bDIS_TRADE", 1, 1, bDIS_TRADE );
			cFILE.getflag ( "bDIS_PARTY", 1, 1, bDIS_PARTY );
			cFILE.getflag ( "bDIS_FRIEND", 1, 1, bDIS_FRIEND );			

			cFILE.getflag ( "bCONFT_BOUND", 1, 1, bCONFT_BOUND );
			cFILE.getflag ( "wCONFT_RECOVER", 1, 1, wCONFT_RECOVER );
			cFILE.getflag ( "fCONFT_HP_RATE", 1, 1, fCONFT_HP_RATE );
			cFILE.getflag ( "fCONFT_TAR_HP_RATE", 1, 1, fCONFT_TAR_HP_RATE );

			cFILE.getflag ( "dwPARTY_GET_ITEM", 1, 1, dwPARTY_GET_ITEM );
			cFILE.getflag ( "dwPARTY_GET_MONEY", 1, 1, dwPARTY_GET_MONEY );
		}
		else if ( dwVER==0x0102 )
		{
			cFILE.getflag ( "bDIS_CONFT", 1, 1, bDIS_CONFT );
			cFILE.getflag ( "bDIS_TRADE", 1, 1, bDIS_TRADE );
			cFILE.getflag ( "bDIS_PARTY", 1, 1, bDIS_PARTY );

			cFILE.getflag ( "bCONFT_BOUND", 1, 1, bCONFT_BOUND );
			cFILE.getflag ( "wCONFT_RECOVER", 1, 1, wCONFT_RECOVER );
			cFILE.getflag ( "fCONFT_HP_RATE", 1, 1, fCONFT_HP_RATE );
			cFILE.getflag ( "fCONFT_TAR_HP_RATE", 1, 1, fCONFT_TAR_HP_RATE );

			cFILE.getflag ( "dwPARTY_GET_ITEM", 1, 1, dwPARTY_GET_ITEM );
			cFILE.getflag ( "dwPARTY_GET_MONEY", 1, 1, dwPARTY_GET_MONEY );
		}
		else if ( dwVER==0x0101 )
		{
			cFILE.getflag ( "bDIS_CONFT", 1, 1, bDIS_CONFT );
			cFILE.getflag ( "bDIS_TRADE", 1, 1, bDIS_TRADE );
			cFILE.getflag ( "bDIS_PARTY", 1, 1, bDIS_PARTY );

			cFILE.getflag ( "bCONFT_BOUND", 1, 1, bCONFT_BOUND );
			cFILE.getflag ( "wCONFT_RECOVER", 1, 1, wCONFT_RECOVER );
			cFILE.getflag ( "fCONFT_HP_RATE", 1, 1, fCONFT_HP_RATE );
			cFILE.getflag ( "fCONFT_TAR_HP_RATE", 1, 1, fCONFT_TAR_HP_RATE );

			cFILE.getflag ( "dwPARTY_GET_ITEM", 1, 1, dwPARTY_GET_ITEM );
			cFILE.getflag ( "dwPARTY_GET_MONEY", 1, 1, dwPARTY_GET_MONEY );
		}

		return TRUE;
	}
};

