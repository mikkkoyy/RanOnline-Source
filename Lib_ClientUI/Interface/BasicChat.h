#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"

/*item link, Juver, 2017/07/31 */
#include "../Lib_Client/G-Logic/GLItem.h"
#include "../Lib_Client/G-Logic/GLItemLink.h"

class CBasicChatLeftBar;
class CBasicChatRightBody;
class CUIChatShowFlag;
class CBasicButton;

////////////////////////////////////////////////////////////////////
//	사용자 메시지 정의
const DWORD UIMSG_MOUSEIN_BLOCK_CHARACTER_MOVE = UIMSG_USER1;
////////////////////////////////////////////////////////////////////

class	CBasicChat : public CUIGroup
{
private:
	enum
	{
		LEFT_BAR = NO_ID + 1,
		RIGHT_BODY,
		CHAT_SHOW_FLAG,
		CHAT_FLAG_BUTTON,
	};

public:
	CBasicChat ();
	virtual	~CBasicChat ();

public:
	void	CreateSubControl ();

public:
	virtual	void	Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual HRESULT RestoreDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );
	virtual	void	TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

public:
	/*item link, Juver, 2017/07/31 */
	void	AddChat ( const CString& strName, const CString& strMsg, const WORD& wType, SITEMLINK* pItemLink ); 

	/*item link, Juver, 2017/07/31 */
	void	AddStringToChatEx ( CString strTemp, WORD wType, SITEMLINK* pItemLink );

	/*item link, Juver, 2017/07/31 */
	void	AddStringToSystemMessage ( CString strTemp, D3DCOLOR dwColor, SITEMLINK* pItemLink ); 

	/*item link, Juver, 2017/07/31 */
	void	AddItemLink( SITEMCUSTOM* pItemCustom );

	void	AddChatMacro (CString strMsg );  // 매크로 형
	void	ChatLog( bool bChatLog, int nChatLogType );
	CString GET_RECORD_CHAT();

public:
	void	BEGIN_NORMAL_CHAT ( const CString& strMessage = "" ); 
	void	BEGIN_PRIVATE_CHAT ( const CString& strName = "", const CString& strMessage = "" );
	void	BEGIN_PARTY_CHAT ( const CString& strMessage = "" );
	void	BEGIN_TOALL_CHAT ( const CString& strMessage = "" );
	void	BEGIN_GUILD_CHAT ( const CString& strMessage = "" );
	void	BEGIN_ALLIANCE_CHAT ( const CString& strMessage = "" );
	void	BEGIN_REGIONAL_CHAT ( const CString& strMessage = "" ); /*regional chat, Juver, 2017/12/06 */

	bool	IsCHAT_BEGIN ();

public:
	void	REFRESH_FRIEND_LIST ();
	void	FRIEND_LIST( CONST CString & strName, bool bOnline );

private:
	CBasicChatLeftBar*		m_pBasicChatLeftBar;
	CBasicChatRightBody*	m_pBasicChatRightBody;
private:
	BOOL		m_bFirstGap;
	D3DXVECTOR2	m_vGap;

	int			m_PosX;
	int			m_PosY;

	BOOL		m_bCheckButton;

private:
	CUIChatShowFlag* m_pUIChatShowFlag;
	CBasicButton*	m_pChatShowFlagButton;
};