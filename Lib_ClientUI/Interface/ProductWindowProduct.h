/*!
 * \file ProductWindowType.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */
#pragma	once

#include "BasicLineBoxSmart.h"
#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLItemMix.h"

#define PRODUCT_INFO_UPDATE_TIME 0.1f

class CBasicTextBox;
class CItemImage;
class CBasicTextButton;
class CUIEditBox;
class RnButton;
class CBasicProgressBar;

class CProductWindowProduct : public CBasicLineBoxSmart
{
public:
    enum
    {
		LIMIT_COLUMN_CNT = 5,
    };
private:
	enum
	{
		PRODUCT_ITEM_IMAGE_RES = NO_ID + 1,
		PRODUCT_ITEM_IMAGE_REQ_0,
		PRODUCT_ITEM_IMAGE_REQ_1,
		PRODUCT_ITEM_IMAGE_REQ_2,
		PRODUCT_ITEM_IMAGE_REQ_3,
		PRODUCT_ITEM_IMAGE_REQ_4,
		PRODUCT_BUTTON_PRODUCTION,
		PRODUCT_GOODS_EDITBOX_MANAGER,
	};
    enum
    {
        ID_PRODUCT_GOODS_RESULT_ITEMSLOT = NO_ID + 1,
        ID_PRODUCT_GOODS_EDITBOX_MANAGER,
		ID_PRODUCT_PRODUCTION_BUTTON,
        ID_PRODUCT_GOODS_PRODUCT_COUNT_EDIT,
        ID_PRODUCT_GOODS_PRODUCT_COUNT_BOX,
    };
    enum
    {
		// nLIMIT_PRODUCT_CNT개수만큼 할당하기 때문에 시작과 끝을 지정하였다.
        ID_PRODUCT_GOODS_ITEMSLOT_START = ID_PRODUCT_GOODS_RESULT_ITEMSLOT + 1000,
        ID_PROCUCT_GOODS_ITEMSLOT_END   = ID_PRODUCT_GOODS_ITEMSLOT_START + LIMIT_COLUMN_CNT,
        ID_PRODUCT_GOODS_ITEMSLOT_TEXT_START = ID_PROCUCT_GOODS_ITEMSLOT_END + 1,
        ID_PRODUCT_GOODS_ITEMSLOT_TEXT_END = ID_PRODUCT_GOODS_ITEMSLOT_START+LIMIT_COLUMN_CNT,
    };


public:
	CProductWindowProduct ();
	virtual	~CProductWindowProduct ();


	void	CreateSubControl ();

private:
	ITEM_MIX			m_sItemMix;
	CBasicTextBox*		m_pTextProductLevelChar;
	CBasicTextBox*		m_pTextProductChance;
	CBasicTextBox*		m_pTextProductTime;
	CBasicTextBox*		m_pTextMaterial;
	CBasicTextBox*		m_pTextMoney;
	CBasicTextBox*		m_pTextResultNum;
	CBasicTextBox*		m_pTextResultCost;
	CBasicTextBox*		m_pTextInfo;
	CItemImage*			m_pItemImageResult;
	CItemImage*			m_pItemImageRequire[ITEMMIX_ITEMNUM];
	
    CItemImage*	        m_pResulteItemImage;
	CBasicTextBox*      m_pResultItemText;
    CBasicTextBox*      m_pProductTimeText;
    CItemImage*	        m_pItemImageArray[LIMIT_COLUMN_CNT];
    CBasicTextBox*      m_pMaterialText[LIMIT_COLUMN_CNT];
    unsigned int        m_nMaterialCount[LIMIT_COLUMN_CNT];
    CBasicTextBox*      m_pProductMessageText;
    CBasicTextBox*      m_pCostSumText;
    CBasicTextBox*      m_pMoneyText;

	//hsshin 숙련도 시스템 추가
	CBasicTextBox*      m_pReqLevelText;
	
    RnButton*               m_pProductionButton;


	CBasicTextBox*		m_pTextItemRequire[ITEMMIX_ITEMNUM];
	CBasicTextButton*	m_pButtonProduction;
	CBasicProgressBar*	m_pTimer;

	CUIEditBox*			m_pProductEditBox;
	int					m_nProductCount;
	int					m_nProductCountLast;
	DWORD				m_dwProductID;

private:
	float				m_fUpdateTime;

	
public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

public:
	void	ResetData();
	void	SetData( DWORD dwID );
	void	UpdateInfo();
};