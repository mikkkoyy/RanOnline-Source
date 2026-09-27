#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLDefine.h"

struct SITEMCUSTOM;
class CItemImage;
class CBasicTextBox;

class	CWeaponDisplay : public CUIGroup
{
private:
	CItemImage*		m_pWeaponImage;
	CBasicTextBox*	m_pWeaponText;
	SNATIVEID		m_sNativeID;
	CString			m_sWeaponStr;
	CUIControl*		m_pDummyControl;
public:
	CWeaponDisplay ();
	virtual	~CWeaponDisplay ();

public:
	void	CreateSubControl ();
public:
	virtual void	Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

protected:
	void	SetItem ( const SITEMCUSTOM& sItemCustom );	
	void	ResetItem ();

	void SetItemNativeID ( const SNATIVEID& sNativeID )		{ m_sNativeID = sNativeID; }
	const SNATIVEID& GetItemID () const						{ return m_sNativeID; }
};