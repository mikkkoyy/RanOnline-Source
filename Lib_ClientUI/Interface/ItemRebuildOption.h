#pragma	once
/*dmk14 reform*/

#include "BasicTextBox.h"
#include "UIWindowEx.h"
#include "GLCharDefine.h"
#include "GLItem.h"
#include "GLItemMan.h"

class CBasicTextBox;
class CItemRebuildOption : public CUIWindowEx
{
protected:

public:
	CItemRebuildOption ();
	virtual	~CItemRebuildOption ();
	void CreateSubControl();

public:
	void Init( bool bClear );
	void IsClicked();
	bool IsCheck()		{ return m_bFlip; }
	void SetFlip( bool bFlip );

	void SetVisibleCheckBox( BOOL bVisible );
	
	bool SetData( SITEMCUSTOM sItemCust, int nIndex );
	void ResetData();

	void SetItemInformation( SITEMCUSTOM sItemCustom );
	void SetBasicInformation( SITEMCUSTOM sItemCustom );
	void SetAdditionalInformation( SITEMCUSTOM sItemCustom );
	
	bool IsRandOptNull()	{ return (m_emOpt == EMR_OPT_NULL || m_fVal == 0 ); }
	EMRANDOM_OPT GetOpt()	{ return m_emOpt; }
private:
	bool					m_bFlip;

	CBasicButton*			m_pOptionCheckBox;
	CBasicTextBox*			m_pOptionTextBox;

	CBasicTextBox*			m_pBasicInformation;
	CBasicTextBox*			m_pAdditionalInformation;

	
	CUIControl* m_pResistSpirit;
	CUIControl* m_pResistFire;
	CUIControl* m_pResistIce;
	CUIControl* m_pResistElectric;
	CUIControl* m_pResistPoison;

	CBasicTextBox* m_pResistSpiritText;
	CBasicTextBox* m_pResistFireText;
	CBasicTextBox* m_pResistIceText;
	CBasicTextBox* m_pResistElectricText;
	CBasicTextBox* m_pResistPoisonText;
	
	bool m_bDisplayResist;
	

	EMRANDOM_OPT			m_emOpt;
	float					m_fVal;

	void					_cutZero(CString& cstr, int ncount);
	void					_resizeInformationBox(void);
private:
	CBasicButton*	CreateFlipButton ( char* szButton, char* szButtonFlip );
};