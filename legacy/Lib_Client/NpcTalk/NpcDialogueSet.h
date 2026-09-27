///////////////////////////////////////////////////////////////////////
//	CNpcDialogue
//
//	최초 작성자 : 성기엽
//	이후 수정자 : 
//	로그
//
//
///////////////////////////////////////////////////////////////////////

#pragma	once

#include <vector>
#include <map>

#include "basestream.h"

//	CNpcDialogueSet <- CNpcDialogue <- CNpcDialogueCase <- CNpcTalkControl <- SNpcTalk
//
//
class CNpcDialogue;
class CSerialFile;
struct SNpcTalk;

class	CNpcDialogueSet
{
public:
	CNpcDialogueSet ();
	virtual	~CNpcDialogueSet ();

public:
	enum 
	{ 
		VERSION		= 0x0203,
		ENCODE_VER	= 0x0200 
	};	// 랜덤 대화문


	typedef std::map<DWORD,CNpcDialogue*>	DIALOGMAP;
	typedef DIALOGMAP::iterator				DIALOGMAP_IT;

	typedef std::vector<SNpcTalk*>			VECTALK;

	CString m_strFileName;

protected:
	void RegistTalk ( SNpcTalk* pNpcTalk );

	void ReportAction ();
	void AssignTalkGlobID ();

	BOOL LOAD_0001 ( basestream &SFile );
	BOOL LOAD_0002 ( basestream &SFile );
	BOOL LOAD_0003 ( basestream &SFile );
	BOOL LOAD_0004 ( basestream &SFile );
	BOOL LOAD_0005 ( basestream &SFile );
	BOOL LOAD_0006 ( basestream &SFile );
	BOOL LOAD_0007 ( basestream &SFile );	// ITEMREBUILD_MARK
	BOOL LOAD_0008 ( basestream &SFile );	// 홀짝게임
	BOOL LOAD_0009 ( basestream &SFile );	// 경험치 회복
	BOOL LOAD_0010 ( basestream &SFile );	// 임의대화
	BOOL LOAD_0011 ( basestream &SFile );	// 아이템 검색 추가
	BOOL LOAD_0012 ( basestream &SFile );	// 출석부 기능 추가
	BOOL LOAD_0013 ( basestream &SFile );	// 아이템 조합 추가
	BOOL LOAD_0014 ( basestream &SFile );
	BOOL LOAD_0015 ( basestream &SFile );
	BOOL LOAD_0200 ( basestream &SFile );
	BOOL LOAD_0201 ( basestream &SFile );
	BOOL LOAD_0202 ( basestream &SFile );
	BOOL LOAD_0203 ( basestream &SFile );

	static CString m_strPath;

public:
	BOOL LOAD_0016 ( basestream &SFile );
	/*dmk14 ver17 npc loader*/
	BOOL LOAD_0017 ( basestream &SFile );
private:
	bool		m_bModify;
	DIALOGMAP	m_mapDialogue;

	//official now follow EM_BASIC order
	bool		m_bStorage; // 개인 락커
	bool		m_bMarket; // 상점 물건판매
	bool		m_bCure; // 치료
	bool		m_bStartPoint; // 시작지점 설정
	bool		m_bCharReset; // 스텟 스킬 초기화
	bool		m_bItemTrade;
	bool		m_bBusStation; // 버스 단말기
	bool		m_bClubNew; // 클럽생성
	bool		m_bClubRankUp; // 클럽 랭크 업
	bool		m_bCDCertify; // CD 인증
	bool		m_bCommission; // 선도지역 수수료 설정
	bool		m_bClubStorage; // 클럽 창고
	bool		m_bItemRebuild;	// 아이템 개조	// ITEMREBUILD_MARK
	bool		m_bOddEven; // 홀짝게임
	bool		m_bRecoveryExp;	// 경험치 부활
	bool		m_bRandomPage;
	bool		m_bItemSearch; // 아이템 검색
	bool		m_bAttendanceBook;	// 출석부
	bool		m_bItemMix;		// 아이템 조합
	bool		m_bNPCShop;				/*npc shop, Juver, 2017/07/25 */
	bool		m_bNPCItemExchange;		/*item exchange, Juver, 2017/10/11 */
	bool		m_bHireSummon;

	VECTALK		m_vecTALK;

public:
	DWORD MakeDlgNID ();

	BOOL AddDialogue ( CNpcDialogue* pDialogue );
	void DelDialogue ( DWORD NativeID );

	CNpcDialogue* GetDialogue ( DWORD NativeID );
	CNpcDialogue* GetDialogue2 ( int nDialogue );
	void CNpcDialogueSet::GetDlgText( DWORD dwIndex, std::string& strName );

	void RemoveAllDialogue ();

	SNpcTalk* GetTalk ( DWORD dwGlobID );

	DIALOGMAP* GetDialogueMap ();

	BOOL Save ( const char* szFileName );
	BOOL Load ( const char* szFileName );

	BOOL SimpleEdit ( CWnd* pWnd );

	void DoModified ()		{ m_bModify = true; }
	void ResetModified ()	{ m_bModify = false; }
	bool IsModify ()		{ return m_bModify; }

	bool IsSTORAGE ()		{ return m_bStorage; }
	bool IsMARKET ()		{ return m_bMarket; }
	bool IsCURE ()			{ return m_bCure; }
	bool IsSTARTPOINT ()	{ return m_bStartPoint; }
	bool IsCHARRESET () 	{ return m_bCharReset; }
	bool IsBUSSTATION ()	{ return m_bBusStation; }
	bool IsCLUBNEW ()		{ return m_bClubNew; }
	bool IsCLUBRANKUP ()	{ return m_bClubRankUp; }
	bool IsCDCERTIFY ()		{ return m_bCDCertify; }
	bool IsCOMMISSION ()	{ return m_bCommission; }
	bool IsCLUBSTORAGE ()	{ return m_bClubStorage; }
	bool IsITEMREBUILD()	{ return m_bItemRebuild; }	// ITEMREBUILD_MARK
	bool IsODDEVEN()		{ return m_bOddEven; }
	bool IsRecoveryExp()	{ return m_bRecoveryExp; }
	bool IsRandomPage()		{ return m_bRandomPage; }
	bool IsItemSearch()		{ return m_bItemSearch; }
	bool IsAttendanceBook() { return m_bAttendanceBook; }
	bool IsItemMix()		{ return m_bItemMix; }
	bool IsItemTrade()		{ return m_bItemTrade; }
	bool IsNPCShop()		{ return m_bNPCShop; } /*npc shop, Juver, 2017/07/25 */
	bool IsNPCItemExchange()	{ return m_bNPCItemExchange; }	/*item exchange, Juver, 2017/10/11 */

	static const char* GetPath ()			{ return m_strPath.GetString(); }
	static void SetPath ( char* szPath )	{ m_strPath = szPath; }

	CNpcDialogueSet& operator= ( CNpcDialogueSet &rvalue );
};