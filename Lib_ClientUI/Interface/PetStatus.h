#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLDefine.h"

struct SITEMCUSTOM;
class CItemImage;
class CBasicTextBox;
class CBasicMiniBarGauge;

class	CPetStatus : public CUIGroup
{

protected:
	enum
	{
		PET_FOOD_GAUGE = NO_ID + 1,
	};

private:
	CBasicMiniBarGauge*		m_pGauge;
	CItemImage*		m_pPetImage;
	CUIControl*		m_pDummyControl;

public:
	SNATIVEID	m_sNativeID;
	CString		m_strName;
	float		m_fTotal;
	float		m_fRemain;
	float		m_fRemainBack;
	float		m_fCheckTimer;

public:
	CPetStatus ();
	virtual	~CPetStatus ();

public:
	void	CreateSubControl ();
public:
	virtual void	Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

protected:
	void	SetItem ( const SITEMCUSTOM& sItemCustom );	
	void	ResetItem ();

public:
	void	SetItemNativeID ( const SNATIVEID& sNativeID )		{ m_sNativeID = sNativeID; }
	const	SNATIVEID& GetItemID () const						{ return m_sNativeID; }
};
