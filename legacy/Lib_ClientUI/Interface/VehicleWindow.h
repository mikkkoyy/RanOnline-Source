/*!
 * \file VehicleWindow.h
 *
 * \author Juver
 * \date August 2017
 *
 * \vehicle system
 */

#pragma	once

#include "UIWindowEx.h"
#include "GLItem.h"

const DWORD UIMSG_MOUSEIN_VEHICLE_WEARSLOT = UIMSG_USER1;

class	CBasicProgressBar;
class	CBasicTextBox;
class	CItemImage;
class	CBasicButton;
class	CVehicleWindowRender;

class	CVehicleWindow : public CUIWindowEx
{
	enum
	{
		SLOT_SKIN_IMAGE_TYPE0 = ET_CONTROL_NEXT,
		SLOT_PARTS_IMAGE_TYPE0,
		SLOT_PARTS_IMAGE_TYPE1,
		SLOT_PARTS_IMAGE_TYPE2,	
		SLOT_PARTS_IMAGE_TYPE3,
		SLOT_PARTS_IMAGE_TYPE4,
		SLOT_PARTS_IMAGE_TYPE5,
		VEHICLE_BATTERY_TEXT,
		VEHICLE_RENDER_BUTTON_LEFT,
		VEHICLE_RENDER_BUTTON_RIGHT,
		VEHICLE_RENDER_BUTTON_RUN,

		SLOT_IMAGE_SIZE = 7
	};


	SITEMCUSTOM	m_ItemCustomArray[SLOT_IMAGE_SIZE];

	CUIControl*	m_pSlotDisplay[SLOT_IMAGE_SIZE];
	CItemImage*	m_pSlotImage[SLOT_IMAGE_SIZE];
	CUIControl*	m_pMouseOver;

	CBasicProgressBar * m_pBattery;

	CBasicTextBox* m_pVehicleName;
	CBasicTextBox* m_pVehicleType; 
	CBasicTextBox* m_pBatteryStatic;
	CBasicTextBox* m_pBatteryText;
	CBasicTextBox* m_pSpeedText; 
	CBasicTextBox* m_pAcceText;	/* vehicle no accessory, Juver, 2018/02/14 */

	/*vehicle booster system, Juver, 2017/08/10 */
	CBasicTextBox*	m_pBoosterText; 
	CUIControl*		m_pBoosterImage;
	CUIControl*		m_pBoosterOver;

	CBasicButton*	m_pButtonLeft;
	CBasicButton*	m_pButtonRight;
	CBasicButton*	m_pButtonRun;
	CBasicTextBox*	m_pRunText;

	CVehicleWindowRender*	m_pRender;
	
	UIGUID	m_RollOverID;

public:
	CVehicleWindow ();
	virtual	~CVehicleWindow ();

public:
	void CreateSubControl ();


public:
	virtual	void SetVisibleSingle ( BOOL bVisible );
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

public:
	void LoadItem ( int nIndex, SITEMCUSTOM& ref_sItemCustom );
	void UnLoadItem ( int nIndex );
	SITEMCUSTOM& GetItem ( int nIndex )			{ return m_ItemCustomArray[nIndex]; }

private:
	int		m_nBatteryBack;
	float	m_fSpeedBack;

	/*vehicle booster system, Juver, 2017/08/11 */
	bool	m_bBoosterBack;

public:
	EMSUIT IMAGE2EMSLOT ( UIGUID ControlID );

public:
	void LoadData();

	/*vehicle booster system, Juver, 2017/08/11 */
	void LoadBoosterInfo( VEHICLE_TYPE emType, bool bBooster );
};