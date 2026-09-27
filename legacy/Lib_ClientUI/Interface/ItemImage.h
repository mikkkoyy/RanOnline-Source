//	기본 정보 표시
//
//	최초 작성자 : 성기엽
//	이후 수정자 : 
//	로그
//		[2003.11.27]
//			@ 작성
//

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLDefine.h"

struct SITEMCUSTOM;
class	CBasicProgressBar;
class	CBasicTextBox;

class	CItemImage : public CUIGroup
{
private:
	enum {
		ITEM_IMAGE_SLOT = NO_ID + 1,
		ITEM_IMAGE_OVER,
	};
public:
	CItemImage ();
	virtual	~CItemImage ();

public:
	void	CreateSubControl (const std::string& overImageKey = "" ,DWORD color = D3DCOLOR_XRGB(255,255,255), const char* szControlKeyword = "ITEM_IMAGE");
	void	CreateTextBoxUpLeft();
	void	CreateTextBoxUpRight();
	void	CreateTextBoxDownLeft();
	void	CreateTextBoxDownRight();

public:
	virtual HRESULT InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );
	virtual void Update( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void SetDiffuse(D3DCOLOR _diffuse);

public:
    const bool	SetItem ( const SITEMCUSTOM& itemCustom );
	const bool	SetItem ( SNATIVEID sNativeID  );

	void	SetSkill(SNATIVEID sICONINDEX, const char* szTexture, SNATIVEID sNativeID = false);
	void	SetBackItem( SNATIVEID sICONINDEX, const char* szTexture );

	void    SetVisibleOverImage(bool isVisible);
	bool    IsVisibleOverImage();

	SNATIVEID   GetItem();
	SNATIVEID	GetItemImageSlotPos();

	SNATIVEID	GetItemID()		{ return m_sNativeID; }

	const CString&	GetItemTextureName () const;

	void SetUseTwinkle( bool bUse )					{ m_bTwinkle = bUse; }
	
	void SetNumber ( int nNumber, int nMaxNumber );
	void ResetNumber (  );

private:
    const bool	SetItem ( SNATIVEID sICONINDEX, const char* szTexture, SNATIVEID sNativeID );
    const bool	SetItem ( SNATIVEID sICONINDEX, const char* szTexture, SNATIVEID sNativeID, SNATIVEID sICONINDEX_Overlaped, const char* szTexture_Overlaped, SNATIVEID sNativeID_Overlaped );

public:
	virtual void	ResetItem ();
	virtual void	SetItemProcess( SNATIVEID sNativeID );
	virtual void	ReSetItemProcess();


private:
	LPDIRECT3DDEVICEQ	m_pd3dDevice;

protected:
	SNATIVEID	m_sICONINDEX;
	SNATIVEID	m_sICONINDEXBACK;
	SNATIVEID	m_sNativeID;
    SNATIVEID	m_sNativeID_Overlaped;
	CString		m_strTextureName;

	CBasicProgressBar*	m_pItemProcess;
	CBasicTextBox*		m_pTextBoxUpLeft;
	CBasicTextBox*		m_pTextBoxUpRight;
	CBasicTextBox*		m_pTextBoxDownLeft;
	CBasicTextBox*		m_pTextBoxDownRight;

	float m_TotElapsedTime;
	bool m_bItemUsed;
	bool m_bItemEnd;

	bool m_bTwinkle;

	
	CUIControl*		m_pItemImage;
    CUIControl*		m_pItemImageOverlaped;
	CUIControl*		m_pOverImage;
	CBasicTextBox*  m_pNumberText;


private:
	CUIControl*		m_pItemImageBack;
	//CUIControl*		m_pItemImage;

public:
	void	SetTextUpLeft( CString strText, DWORD dwColor );
	void	SetTextUpRight( CString strText, DWORD dwColor );
	void	SetTextDownLeft( CString strText, DWORD dwColor );
	void	SetTextDownRight( CString strText, DWORD dwColor );

	void	SetTurnNum( int nNum );
};