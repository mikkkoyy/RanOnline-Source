#pragma once

#include <map>
#include <deque>
#include <set>
#include <map>

#include "./GLAGLandMan.h"
#include "./GLCharAG.h"
#include "./GLFreePK.h"
#include "./GLMapList.h"
#include "./GLPartyMan.h"

#include "../../Lib_Engine/G-Logic/DxMsgServer.h"
#include "../../Lib_Engine/G-Logic/DxConsoleMsg.h"
#include "../../Lib_Engine/G-Logic/GLDBMan.h"

#include "../../Lib_Network/s_NetGlobal.h"
#include "../../Lib_Network/s_CConsoleMessage.h"


typedef std::set<DWORD>	   MAPIDSET;
typedef MAPIDSET::iterator MAPIDSET_ITER;

struct STestInstantMapCharInfo 
{
	DWORD		dwGaeaID;
	SNATIVEID	sBaseMapID;
	float		fElapsedTime;

	STestInstantMapCharInfo() 
		: dwGaeaID(0)
		, sBaseMapID(NATIVEID_NULL())
		, fElapsedTime(0.0f)
	{
	}

};

struct SInsertInstantMapID
{
	MAPIDSET		useMapSubIDSet;
	DWORD			nInsertMapSubID;
	enum			{ MAX_INSERTMAP_NUM = MAX_CREATE_INSTANT_MAP_NUM }; // �����Ҽ� ������ �δ� ����

	SInsertInstantMapID()	: nInsertMapSubID(1)
	{
		useMapSubIDSet.clear();
	}
	
	const int InsertInduMapID()
	{
		if( useMapSubIDSet.size() == MAX_INSERTMAP_NUM - 1 )
			return SNATIVEID::ID_NULL;

		MAPIDSET_ITER iter;
		while(1)
		{
			if( nInsertMapSubID >= MAX_INSERTMAP_NUM ) nInsertMapSubID = 1;

			iter = useMapSubIDSet.find(nInsertMapSubID);
			if( iter == useMapSubIDSet.end() ) break;
			nInsertMapSubID++;

		}
		useMapSubIDSet.insert(nInsertMapSubID);		
		nInsertMapSubID++;

		return nInsertMapSubID-1;
	}

	const bool DeleteInstantMapID( const int iMapNum )
	{
		MAPIDSET_ITER iter = useMapSubIDSet.find(iMapNum);
		if( iter == useMapSubIDSet.end() ) return FALSE;
		useMapSubIDSet.erase(iter);
		return TRUE;
	}

	const bool FindID( const int iMapNum )
	{
		MAPIDSET_ITER iter = useMapSubIDSet.find(iMapNum);
		if( iter == useMapSubIDSet.end() ) return FALSE;
		return TRUE;
	}
};


struct SWorkingEvent
{
	GLMSG::SNET_GM_EVENT_EXP		 exp_Event;
    GLMSG::SNET_GM_EVENT_ITEM_GEN	 item_Event;
	GLMSG::SNET_GM_EVENT_MONEY_GEN    money_Event;
	
	GLMSG::SNET_GM_EVENT_EX			 ex_Event;	
	GLMSG::SNET_GM_LIMIT_EVENT_BEGIN limit_Event;
	bool bWorkEvent[8];				 // 0 : ����ġ, 1 : ������,  2 : �� �����, 3 : EX �̺�Ʈ,  4 : ����Ʈ  5: Ŭ������  ����ġ 6: Ŭ������ ������ 7: Ŭ������ ��

	SWorkingEvent::SWorkingEvent()
	{
		Init();
	}

	void SWorkingEvent::Init()
	{
		bWorkEvent[0] = FALSE;
		bWorkEvent[1] = FALSE;
		bWorkEvent[2] = FALSE;
		bWorkEvent[3] = FALSE;
		bWorkEvent[4] = FALSE;
		bWorkEvent[5] = FALSE;
		bWorkEvent[6] = FALSE;
		bWorkEvent[7] = FALSE;
	}
};

class GLAgentServer
{
public:
	// LG-7 GlobalRanking
	VEC_TOP_KILL m_vecTopKill[10];
	VEC_TOP_RICH m_vecTopRich;
	VEC_TOP_GUILD m_vecTopGuild;

public:
	typedef std::deque<DWORD>	PCGID;
	typedef PCGID::iterator		PCGID_ITER;

	typedef std::map<DWORD,DWORD>	CLIENTMAP;
	typedef CLIENTMAP::iterator				CLIENTMAP_ITER;

	typedef std::map<std::string,PGLCHARAG>	CHAR_MAP;
	typedef CHAR_MAP::iterator				CHAR_MAP_ITER;

	typedef std::vector<DWORD>				VEC_INSTANT_MAPID;
	typedef VEC_INSTANT_MAPID::iterator		VEC_INSTANT_MAPID_ITER;


private:
	DxMsgServer*			m_pMsgServer;
	DxConsoleMsg*			m_pConsoleMsg;
	GLDBMan*				m_pDBMan;

	CRITICAL_SECTION		m_CSPCLock;

private:
	DWORD					m_dwMaxClient;
	PGLCHARAG*				m_PCArray;				//	PC �迭.
	PCGID					m_FreePCGIDs;			//	�̻��� PC GlobID ��.

	AGCHARLIST				m_PCList;				//	Ȱ������ �ɸ��� ����Ʈ.

	SWorkingEvent			m_WorkEvent;			//  ���� �������� �̺�Ʈ��

	CHAR_MAP				m_UAccountMap;			//	User Account map.
	CHAR_MAP				m_PCNameMap;			//	PC Name map.
	CLIENTMAP				m_PCClientIDMAP;		//	PC ClientID map.
	CLIENTMAP				m_mapCharID;			//	CID map.
	CLIENTMAP				m_UserNumberMap;		//  UserNumber Map.
	CHAR_MAP				m_PCIpMap;

	SInsertInstantMapID		m_sInsertInstantMapId;	// ������ �δ��� ���� �� ���̵�

	PCGID					m_RegDropOutPC;

	GLPartyMan				m_cPartyMan;
	GLClubMan				m_cClubMan;

	GLFreePK				m_cFreePK;				// ���� �̺�Ʈ��

	float					m_fPERIOD_TIMER;
	DWORD					m_dwFIELDSVR_NUM;
	bool					m_bFIELDSVR[MAX_CHANNEL_NUMBER][FIELDSERVER_MAX];

	CTime					m_PresetTime;

	SEventState				m_sEventState;

	STestInstantMapCharInfo		m_sTestInstantMapCharInfo;


private:
	GLMapList				m_sMapList;
	GLAGLandMan*			m_pLandMan[MAXLANDMID][MAXLANDSID];	// ���� ������.
	VEC_INSTANT_MAPID		m_vecInstantMapId;						// �δ� �� ���̵� ����

	/*PartySearch - Added By Bubu*/
	MAPPARTYSEARCH			m_mapPartySearch;

	VECPARTYSEARCH			m_vecPartySearch;
	///////////////////////////////////////
private:
	bool StartPartyConfront ( const  DWORD dwPartyA, const DWORD dwPartyB,
		const SNATIVEID &sMapID, const SCONFT_OPTION &sOption,
		const DWORD* pMBR_A, const DWORD* pMBR_B, const D3DXVECTOR3 vPos );

public:
	HRESULT StartClubBattle( DWORD dwClubID, GLCLUBBATTLE sClubBattle );

public:
	DxMsgServer* GetMsgServer ()	{ return m_pMsgServer; }

public:
	void SENDTOCLIENT ( DWORD dwClientID, LPVOID nmg );
	void SENDTOFIELD ( DWORD dwClientID, LPVOID nmg );
	
	void SENDTOFIELDSVR ( int nChannel, int nField, LPVOID nmg );
	void SENDTOCHANNEL ( LPVOID nmg, int nChannel );
	void SENDTOALLCHANNEL ( LPVOID nmg );

	void SENDTOALLCLIENT ( LPVOID nmg );
	void SENDTOALLCLIENT ( LPVOID nmg, int nChannel );

	void SENDTOPARTY ( DWORD dwPartyID, LPVOID nmg );
	void SENDTOPARTY ( DWORD dwGaeaID, DWORD dwPartyID, LPVOID nmg );

	void SENDTOCLUBCLIENT ( DWORD dwClubID, LPVOID nmg );
	void SENDTOALLIANCECLIENT ( DWORD dwClubID, LPVOID nmg );

	// ��Ʋ ����� party ��� Ȯ��
	void ClubBattlePartyCheck( DWORD dwClubID, DWORD dwClubIDTar );
	void AllianceBattlePartyCheck( DWORD dwClubID, DWORD dwClubIDTar );

	BOOL SendToGMMsg( PGLCHARAG pChar, NET_CHAT_FB NetChatFB );

	void CONSOLEMSG_WRITE ( const char* msg, ... );
	void DEBUGMSG_WRITE ( const char* msg, ... );
	void TEXTCONSOLEMSG_WRITE ( const char* msg, ... );

public:
	PGLCHARAG GetChar ( const DWORD dwGaeaID );
	PGLCHARAG GetChar ( const char* szName );
	PGLCHARAG GetCharUA ( const char* szUAccount );
	PGLCHARAG GetCharID ( const DWORD dwCharID );

	DWORD GetFieldServer ( SNATIVEID &sMapID );
	GLAGLandMan* GetByMapID ( SNATIVEID &sMapID );
	GLDBMan* GetDBMan ()						{ return m_pDBMan; }
	const char* GetMapName ( SNATIVEID sMapID )	{ return m_sMapList.GetMapName(sMapID); }

	GLPARTY* GetParty ( DWORD dwPartyID )		{ return m_cPartyMan.GetParty(dwPartyID); }
	GLClubMan& GetClubMan ()					{ return m_cClubMan; }

	DWORD GetFieldNum()		{ return m_dwFIELDSVR_NUM; }

	/*dmk14 fast teleport fix*/
	void  Do2FriendReq( DWORD dwGaeaID , DWORD dwFriendID, WORD wPosX, WORD wPosY );
public:
	PGLCHARAG CreatePC ( GLCHARAG_DATA *pchar_data, DWORD dwClientID, int nChannel, int dwThaiCCafeClass, SChinaTime chinaTime, int nMyCCafeClass );
	HRESULT DropOutPC ( DWORD dwGaeaID );

public:
	BOOL ClearReservedDropOutPC ();
	void ChangeNameMap ( PGLCHARAG pChar, const char* pszOldName, const char* pszNewName );
	void ChangeNameMap ( PGLCHARAG pChar, const TCHAR* pszPhoneNumber );

public:
	BOOL ReserveDropOutPC ( DWORD dwGaeaID );

public:
	void ResetFieldInfo ();

public:
	HRESULT FrameMove ( float fTime, float fElapsedTime );

private:
	BOOL ReActionMoveFieldSvr ( DWORD dwClientID, DWORD dwGaeaID );

	void GameJoinToFieldSvr ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

public:
	BOOL CheckStartMap( DWORD dwGaeaID );

private:
	BOOL MsgReqFieldSvrCharChkFb ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgReqGenItemAgt ( NET_MSG_GENERIC* nmg, DWORD dwClientID );

private:

	BOOL MsgReqCreateInstantMap ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgReqDeleteInstantMap ( NET_MSG_GENERIC* nmg );
	
	BOOL ChatMsgProc ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL ChatMsgLoudSpeakerAgt ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgReqGateOut ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgReqReBirth ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgReqReGenGate ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgFieldSvrOutFb ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgReBirthOutFb ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgReCallAg ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgTeleportAg ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );


	BOOL MsgLvUpMustLeaveMap ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL Msg2FriendAG ( NET_MSG_GENERIC* nmg );
	BOOL Msg2FriendReq ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgMove2Char ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgMove2CharAG ( NET_MSG_GENERIC* nmg );

	BOOL MsgPartyLure ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	/*PartySearch - Added By Bubu*/
	BOOL MsgPartySearchLure ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgPartyLureSearchTarAns ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgPartyLureTarAns ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgGetWhisperMsg( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgPartySecede ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgPartyAuthority( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgPartyDissolve ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgReqGateOutFB ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgReqReBirthFB ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgReqCreateInstantMapFB ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgReqConfront ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgAnsConfront ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgAnsConfrontFB ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgConfrontEnd2Agt ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgClubNew2Agt ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgClubNewDb2Agt ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgClubDissolution ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgClubMemberDb2Del ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgClubCertified ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgClubIncomeMoney ( NET_MSG_GENERIC* nmg );
	BOOL MsgClubIncomeMoneyUp ( NET_MSG_GENERIC* nmg );
	//BOOL MsgClubIncomeMoneyFb ( NET_MSG_GENERIC* nmg );

	BOOL MsgServerCtrlWeather ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgServerCtrlGenHold ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgServerCtrlTime ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgServerCtrlMonth ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgBlockDetected ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmKickUser ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgServerGeneralChat ( NET_MSG_GENERIC* nmg );
	BOOL MsgConfrontPtyCheckMber ( NET_MSG_GENERIC* nmg );
	BOOL MsgConfrontClbCheckMber ( NET_MSG_GENERIC* nmg );

	BOOL MsgGmEventExp ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmEventExpEnd ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmEventItemGen ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmEventItemGenEnd ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmEventMoneyGen ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmEventMoneyGenEnd ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	
	BOOL MsgGmEventEx ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmEventExEnd ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgGmViewWorkEvent ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );


	BOOL MsgGmLimitEventBegin ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmLimitEventEnd   ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmLimitEventRestart   ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgGmLimitEventTimeReq   ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgCyberCafeClassUpdate ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgGmBigHead ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmBigHand ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgGmChatBlockUA ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmChatBlockCN ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmChatBlockCI ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgGmCharInfo4NAME ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmCharInfo4CID ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmCharInfoFldFB ( NET_MSG_GENERIC* nmg );

	BOOL MsgUserCharInfo4NAME ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgUserCharInfoFldFB ( NET_MSG_GENERIC* nmg );

	/*private market set, Juver, 2018/01/02 */
	BOOL MsgGmSetPrivateMarket ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	/*megaphone set, Juver, 2018/01/02 */
	BOOL MsgGmSetMegaPhone ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgGmFreePK ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmViewAllPlayer ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID ); 

	BOOL MsgGmGenChar ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID ); 
	BOOL MsgGmGenMob ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmGenMobEx ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmDelMobEx ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmDelMob ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmMobLevel ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID ); 
	BOOL MsgGmMobLevelClear ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID ); 
	BOOL MsgGmWhereNPC ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID ); 
	BOOL MsgGMMove2Gate ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGMMove2Pos ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgGmWarningMSG ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID ); 
	BOOL MsgGmShowMeTheMoney ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID ); 

	BOOL MsgGmPrintCrowList ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID ); 

	BOOL MsgClubBattleRemainTime ( NET_MSG_GENERIC* nmg );
	BOOL MsgClubDeathMatchRemainTime( NET_MSG_GENERIC* nmg );
	BOOL MsgClubCDCertifyIngBrd ( NET_MSG_GENERIC* nmg );
	BOOL MsgClubCDCertifyBrd ( NET_MSG_GENERIC* nmg );

	BOOL MsgMarketOpenCheck  ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	BOOL MsgClubBattleKillUpdate ( NET_MSG_GENERIC* nmg );
	BOOL MsgClubBattleLastKillUpdate ( NET_MSG_GENERIC* nmg );

	BOOL MsgReqTaxi ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgReqTaxiNpcPos ( NET_MSG_GENERIC* nmg );

	BOOL MsgNpcCommission( NET_MSG_GENERIC* nmg, DWORD dwClientID );

	/////////////////////////////////////////////////////////////////////////////
	// LG-7 GlobalRanking
	BOOL MsgReqGlobalRanking(NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID);
	BOOL MsgReqGlobalRankingKillUpdate(NET_MSG_GENERIC* nmg);
	BOOL MsgReqGlobalRankingRichUpdate(NET_MSG_GENERIC* nmg);
	/////////////////////////////////////////////////////////////////////////////
	BOOL MsgDropCharFromField ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );
	BOOL MsgReqCloseClientAgt(NET_MSG_GENERIC* nmg);

	//fastrevive fix
	BOOL MsgDeathTimerAG ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	/*PartySearch - Added By Bubu*/
	BOOL ReqSearchPartyList(NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID);
	BOOL MsgReqPartySearchUpdate(NET_MSG_GENERIC* nmg);
	/*game stats, Juver, 2017/06/21 */
	BOOL MsgReplyPing ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	/*event map move, Juver, 2017/08/25 */
	BOOL MsgGMMove2_MapPos ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

	/*activity system, Juver, 2017/10/30 */
	BOOL MsgActivityBroadcast( NET_MSG_GENERIC* nmg );

	/*Codex System, Jhoniex 2/14/2024 */
	BOOL MsgCodexBroadcast( NET_MSG_GENERIC* nmg );

public:
	/*pvp tyranny, Juver, 2017/08/25 */
	void TyrannyResetBuffs();
	void TyrannySetRewardBuffs();

	/*pvp capture the flag, Juver, 2018/01/30 */
	void CaptureTheFlagRemoveParty( DWORD dwCharID );

public:
	HRESULT MsgProcess ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

private:
	HRESULT InsertMap ( GLAGLandMan* pNewLandMan );
	HRESULT CreateInstantMap ( SNATIVEID sDestMapID, SNATIVEID sInstantMapID, DWORD dwGaeaID, DWORD dwPartyID );

public:
	HRESULT Create ( DWORD dwMaxClient, DxMsgServer *pMsgServer, DxConsoleMsg *pConsoleMsg, GLDBMan *pDBMan, char* szMapList, F_SERVER_INFO* pFieldInfo );
	HRESULT CleanUp ();

public:
	DWORD	GetInstantMapNum2()		{ return (DWORD)m_vecInstantMapId.size(); }

public:
	~GLAgentServer(void);

private:
	GLAgentServer(void);

public:
	static GLAgentServer& GetInstance();
};

inline PGLCHARAG GLAgentServer::GetChar ( const DWORD dwGaeaID )
{
	if ( dwGaeaID >= m_dwMaxClient )	return NULL;
	return m_PCArray[dwGaeaID];
}

inline PGLCHARAG GLAgentServer::GetCharID ( const DWORD dwCharID )
{
	CLIENTMAP_ITER iter = m_mapCharID.find(dwCharID);
	if ( iter==m_mapCharID.end() )		return NULL;

	return GetChar((*iter).second);
}

inline GLAGLandMan* GLAgentServer::GetByMapID ( SNATIVEID &sMapID )
{
	if ( sMapID.wMainID >= MAXLANDMID )		return NULL;
	if ( sMapID.wSubID >= MAXLANDSID )		return NULL;

	return m_pLandMan[sMapID.wMainID][sMapID.wSubID];
}

inline void GLAgentServer::SENDTOCLIENT ( DWORD dwClientID, LPVOID nmg )
{
	if ( !m_pMsgServer )	return;
	m_pMsgServer->SendClient ( dwClientID, nmg );
}

inline void GLAgentServer::SENDTOFIELD ( DWORD dwClientID, LPVOID nmg )
{
	if ( !m_pMsgServer )	return;
	m_pMsgServer->SendField ( dwClientID, nmg );
}

inline void GLAgentServer::SENDTOFIELDSVR ( int nChannel, int nField, LPVOID nmg )
{
	if ( !m_pMsgServer )	return;
	m_pMsgServer->SendFieldSvr ( nField, nmg, nChannel );
}

inline void GLAgentServer::SENDTOCHANNEL ( LPVOID nmg, int nChannel )
{
	if ( !m_pMsgServer )	return;
	m_pMsgServer->SendChannel ( nmg, nChannel );
}

inline void GLAgentServer::SENDTOALLCHANNEL ( LPVOID nmg )
{
	if ( !m_pMsgServer )	return;
	m_pMsgServer->SendAllChannel ( nmg );
}

inline void GLAgentServer::SENDTOALLCLIENT ( LPVOID nmg )
{
	AGCHARNODE *pCharNode = m_PCList.m_pHead;
	for ( ; pCharNode; pCharNode=pCharNode->pNext )
	{
		PGLCHARAG pCharAG = pCharNode->Data;
		if( !pCharAG ) continue;

		SENDTOCLIENT ( pCharAG->m_dwClientID, nmg );
	}
}

inline void GLAgentServer::SENDTOALLCLIENT ( LPVOID nmg, int nChannel )
{
	AGCHARNODE *pCharNode = m_PCList.m_pHead;
	for ( ; pCharNode; pCharNode=pCharNode->pNext )
	{
		PGLCHARAG pCharAG = pCharNode->Data;
		if( !pCharAG ) continue;

		if ( pCharAG->m_nChannel == nChannel )
			SENDTOCLIENT ( pCharAG->m_dwClientID, nmg );
	}
}

inline void GLAgentServer::SENDTOCLUBCLIENT ( DWORD dwClubID, LPVOID nmg )
{
	GLCLUB *pCLUB = m_cClubMan.GetClub ( dwClubID );
	if ( !pCLUB )	return;

	PGLCHARAG pCHAR = NULL;
	CLUBMEMBERS_ITER pos = pCLUB->m_mapMembers.begin();
	CLUBMEMBERS_ITER end = pCLUB->m_mapMembers.end();
	for ( ; pos!=end; ++pos )
	{
		pCHAR = GetCharID ( (*pos).first );
		if ( !pCHAR )
		{
			continue;
		}
		else
		{
			SENDTOCLIENT ( pCHAR->m_dwClientID, nmg );
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	// LG-7 GlobalRanking
	GLMSG::SNET_CLUB_BATTLE_OVER_CLT* pNetMsg = (GLMSG::SNET_CLUB_BATTLE_OVER_CLT*)nmg;
	if (pNetMsg->nmg.nType == NET_MSG_GCTRL_CLUB_BATTLE_OVER_CLT)
	{
		switch (pNetMsg->emFB)
		{
		case EMCLUB_BATTLE_OVER_DRAW:
		case EMCLUB_BATTLE_OVER_WIN:
		case EMCLUB_BATTLE_OVER_LOSE:
		{
			BOOL bFound					= FALSE;
			VEC_TOP_GUILD_ITER iter		= m_vecTopGuild.begin();
			VEC_TOP_GUILD_ITER iter_end	= m_vecTopGuild.end();

			for (; iter != iter_end; iter++)
			{
				if ((*iter).wGuNum == pCLUB->m_dwID)
				{
					bFound = TRUE;

					if ((*iter).wRank != (WORD)pCLUB->m_dwRank)
						(*iter).wRank = (WORD)pCLUB->m_dwRank;

					if ((*iter).wGuMarkVer != (WORD)pCLUB->m_dwMarkVER)
						(*iter).wGuMarkVer = (WORD)pCLUB->m_dwMarkVER;

					(*iter).dwWin	= pCLUB->m_dwBattleWin;
					(*iter).dwLoss	= pCLUB->m_dwBattleLose;
					(*iter).dwDraw	= pCLUB->m_dwBattleDraw;

					break;
				}
			}

			if (!bFound)
			{
				STOP_RANK_GUILD sTEMP;
				sTEMP.wGuNum		= (WORD)pCLUB->m_dwID;
				StringCchCopy(sTEMP.szGuName, CHAR_SZNAME, pCLUB->m_szName);
				sTEMP.wRank			= (WORD)pCLUB->m_dwRank;
				sTEMP.wGuMarkVer	= (WORD)pCLUB->m_dwMarkVER;
				sTEMP.dwWin			= pCLUB->m_dwBattleWin;
				sTEMP.dwLoss		= pCLUB->m_dwBattleLose;
				sTEMP.dwDraw		= pCLUB->m_dwBattleDraw;
				m_vecTopGuild.push_back(sTEMP);
			}
		}
		break;
		}
	}
	/////////////////////////////////////////////////////////////////////////////

	return;
}

inline void GLAgentServer::SENDTOALLIANCECLIENT ( DWORD dwClubID, LPVOID nmg )
{
	GLCLUB *pCLUB = m_cClubMan.GetClub ( dwClubID );
	if ( !pCLUB )	return;

	CLUB_ALLIANCE_ITER pos = pCLUB->m_setAlliance.begin();
	CLUB_ALLIANCE_ITER end = pCLUB->m_setAlliance.end();
	for ( ; pos!=end; ++pos )
	{
		const GLCLUBALLIANCE &sALLIANCE = *pos;	
		SENDTOCLUBCLIENT( sALLIANCE.m_dwID, nmg );
	}
}


inline void GLAgentServer::SENDTOPARTY ( DWORD dwPartyID, LPVOID nmg )
{
	m_cPartyMan.SendMsgToMember ( dwPartyID, (NET_MSG_GENERIC*) nmg );
}

inline void GLAgentServer::SENDTOPARTY ( DWORD dwGaeaID, DWORD dwPartyID, LPVOID nmg )
{
	m_cPartyMan.SendMsgToMember ( dwGaeaID, dwPartyID, (NET_MSG_GENERIC*) nmg );
}

inline void GLAgentServer::CONSOLEMSG_WRITE ( const char* msg, ... )
{
	if ( !m_pConsoleMsg )		return;

	char sbuf[C_BUFFER_SIZE];
	
	va_list ap;
	va_start(ap, msg);
	StringCbVPrintf ( sbuf, C_BUFFER_SIZE, msg, ap);
	va_end(ap);

	m_pConsoleMsg->Write ( LOG_CONSOLE, sbuf );
}

inline void GLAgentServer::DEBUGMSG_WRITE ( const char* msg, ... )
{
	char sbuf[C_BUFFER_SIZE];
	
	va_list ap;
	va_start(ap, msg);
	StringCbVPrintf ( sbuf, C_BUFFER_SIZE, msg, ap);
	va_end(ap);

	CDebugSet::ToLogFile ( sbuf );

	if ( !m_pConsoleMsg )		return;
	m_pConsoleMsg->Write ( LOG_CONSOLE, sbuf );
}

inline void GLAgentServer::TEXTCONSOLEMSG_WRITE ( const char* msg, ... )
{
	GASSERT ( msg && "CONSOLEMSG_WRITE()" );

	char sbuf[C_BUFFER_SIZE];

	va_list ap;
	va_start(ap, msg);
	StringCbVPrintf ( sbuf, C_BUFFER_SIZE, msg, ap);
	va_end(ap);	

	m_pConsoleMsg->Write( LOG_TEXT_CONSOLE, sbuf );
}
