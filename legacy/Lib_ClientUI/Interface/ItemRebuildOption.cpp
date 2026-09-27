/*dmk14 reform*/
#include "StdAfx.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "BasicTextBox.h"
#include "BasicButton.h"
#include "GameTextControl.h"
#include "UITextControl.h"
#include "ItemRebuildOption.h"
#include "../Lib_Engine/Common/StringFormat.h"
#include "../Lib_Client/G-Logic/GLogicData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CItemRebuildOption::CItemRebuildOption()
	: m_pBasicInformation ( NULL )
	, m_pAdditionalInformation ( NULL )
	, m_bDisplayResist( false )
	, m_pResistSpirit( NULL )
	, m_pResistFire( NULL )
	, m_pResistIce( NULL )
	, m_pResistElectric( NULL )
	, m_pResistPoison( NULL )
	, m_pResistSpiritText( NULL )
	, m_pResistFireText( NULL )
	, m_pResistIceText( NULL )
	, m_pResistElectricText( NULL )
	, m_pResistPoisonText( NULL )
	, m_pOptionTextBox( NULL )
	, m_pOptionCheckBox( NULL )
	, m_bFlip( false )
	, m_emOpt( EMR_OPT_NULL )
	, m_fVal( 0.0f )
{
}

CItemRebuildOption::~CItemRebuildOption()
{
}

void CItemRebuildOption::CreateSubControl()
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );
	
	m_pOptionCheckBox = CreateFlipButton( "ITEM_REBUILD_RENEWAL_RANDOM_OPTION_CHECK_BOX", "ITEM_REBUILD_RENEWAL_RANDOM_OPTION_CHECK_BOX_F" );
	m_pOptionCheckBox->SetVisibleSingle(FALSE);
	
	m_pBasicInformation = new CBasicTextBox;
	m_pBasicInformation->CreateSub ( this, "ITEM_REBUILD_RENEWAL_BASIC_OPTION_TEXT_BOX" );
	m_pBasicInformation->SetFont ( pFont );
	m_pBasicInformation->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );	
	RegisterControl ( m_pBasicInformation );

	m_pAdditionalInformation = new CBasicTextBox;
	m_pAdditionalInformation->CreateSub ( this, "ITEM_REBUILD_RENEWAL_BASIC_OPTION_TEXT_BOX" );
	m_pAdditionalInformation->SetFont ( pFont );
	m_pAdditionalInformation->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );	
	RegisterControl ( m_pAdditionalInformation );

	m_pOptionTextBox = new CBasicTextBox;
	m_pOptionTextBox->CreateSub ( this, "ITEM_REBUILD_RENEWAL_RANDOM_OPTION_TEXT_BOX" );
	m_pOptionTextBox->SetFont ( pFont );
	m_pOptionTextBox->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP );	
	RegisterControl ( m_pOptionTextBox );

	
	{
		CUIControl* pControl = new CUIControl;
		pControl->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_SPIRIT" );
		pControl->SetVisibleSingle( FALSE );
		RegisterControl( pControl );
		m_pResistSpirit = pControl;

		pControl = new CUIControl;
		pControl->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_FIRE" );
		pControl->SetVisibleSingle( FALSE );
		RegisterControl( pControl );
		m_pResistFire = pControl;

		pControl = new CUIControl;
		pControl->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_ICE" );
		pControl->SetVisibleSingle( FALSE );
		RegisterControl( pControl );
		m_pResistIce = pControl;

		pControl = new CUIControl;
		pControl->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_ELECTRIC" );
		pControl->SetVisibleSingle( FALSE );
		RegisterControl( pControl );
		m_pResistElectric = pControl;

		pControl = new CUIControl;
		pControl->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_POISON" );
		pControl->SetVisibleSingle( FALSE );
		RegisterControl( pControl );
		m_pResistPoison = pControl;
	}

	{
		CBasicTextBox* pBasicTextBox = new CBasicTextBox;
		pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_SPIRIT_TEXT" );
		pBasicTextBox->SetFont( pFont );
		pBasicTextBox->SetTextAlign( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_TOP );
		RegisterControl( pBasicTextBox );
		m_pResistSpiritText = pBasicTextBox;

		pBasicTextBox = new CBasicTextBox;
		pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_FIRE_TEXT" );
		pBasicTextBox->SetFont( pFont );
		pBasicTextBox->SetTextAlign( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_TOP );
		RegisterControl( pBasicTextBox );
		m_pResistFireText = pBasicTextBox;

		pBasicTextBox = new CBasicTextBox;
		pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_ICE_TEXT" );
		pBasicTextBox->SetFont( pFont );
		pBasicTextBox->SetTextAlign( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_TOP );
		RegisterControl( pBasicTextBox );
		m_pResistIceText = pBasicTextBox;

		pBasicTextBox = new CBasicTextBox;
		pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_ELECTRIC_TEXT" );
		pBasicTextBox->SetFont( pFont );
		pBasicTextBox->SetTextAlign( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_TOP );
		RegisterControl( pBasicTextBox );
		m_pResistElectricText = pBasicTextBox;

		pBasicTextBox = new CBasicTextBox;
		pBasicTextBox->CreateSub( this, "ITEM_INFOR_TOOLTIP_REGIST_POISON_TEXT" );
		pBasicTextBox->SetFont( pFont );
		pBasicTextBox->SetTextAlign( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_TOP );
		RegisterControl( pBasicTextBox );
		m_pResistPoisonText = pBasicTextBox;
	}

}

CBasicButton* CItemRebuildOption::CreateFlipButton ( char* szButton, char* szButtonFlip )
{
	CBasicButton* pButton = new CBasicButton;
	pButton->CreateSub ( this, szButton, UI_FLAG_DEFAULT );
	pButton->CreateFlip ( szButtonFlip, CBasicButton::RADIO_FLIP );
	pButton->SetControlNameEx ( szButton );
	RegisterControl ( pButton );
	return pButton;
}

void CItemRebuildOption::SetVisibleCheckBox( BOOL bVisible )
{
	m_pOptionCheckBox->SetVisibleSingle( bVisible );
}

void CItemRebuildOption::Init( bool bClear )
{
	m_emOpt = EMR_OPT_NULL;
	m_fVal = 0.0f;
	m_bFlip = false;
	m_pOptionCheckBox->SetFlip( FALSE );
	if( bClear ) 
	{
		m_pOptionTextBox->ClearText();
	}
}

void CItemRebuildOption::ResetData()
{
	m_emOpt = EMR_OPT_NULL;
	m_fVal = 0.0f;
}

void CItemRebuildOption::IsClicked()
{
	m_bFlip = !m_bFlip;
	m_pOptionCheckBox->SetFlip ( m_bFlip );
}

void CItemRebuildOption::SetFlip( bool bSetFlip )
{
	m_bFlip = bSetFlip;
	m_pOptionCheckBox->SetFlip( bSetFlip );
}

void CItemRebuildOption::SetItemInformation(SITEMCUSTOM sItemCustom)
{
	//const SITEM* const pItemData = GLogicData::GetInstance().GetItem(itemCustom.GetNativeID());
	SITEM* pItemData = GLItemMan::GetInstance().GetItem ( sItemCustom.sNativeID );
	if ( pItemData == NULL )
		return;

	SetBasicInformation(sItemCustom);
	SetAdditionalInformation(sItemCustom);
	//ItemInfoRebuildToolTip::_setItemAdditionalInformation(itemCustom, pItemData);
	

	_resizeInformationBox();	

	//m_ItemCustom = itemCustom;
}

void CItemRebuildOption::_cutZero(CString& cstr, int ncount)
{
	// '.' 점이 없다는 것은 소수점 이하 자리가 없으므로 리턴
	if (cstr.Find('.') == -1)
		return;

	CString temp = cstr;

	//ncount 소숫점 몇째 자리 까지 잘라낼찌 정한다
	for (int i = 0; i < ncount + 1; i++ )
	{
		temp = cstr.Right( 1 );

		if ( temp.Find('0') != -1 || temp.Find('.') != -1 )
			cstr = cstr.Left( cstr.GetLength() - 1 );
		else
			break;
	}
}

void CItemRebuildOption::SetBasicInformation ( SITEMCUSTOM sItemCustom )
{
	SITEM* pItemData = GLItemMan::GetInstance().GetItem ( sItemCustom.sNativeID );
	m_pBasicInformation->ClearText();
	std::string strTemp = "";

	const ITEM::SSUIT* psDisguiseSuit = NULL;
	const SITEM* pDisguise = GLItemMan::GetInstance().GetItem( sItemCustom.nidDISGUISE );
	if( NULL != pDisguise )
		psDisguiseSuit = &pDisguise->sSuitOp;

	{
		GLDWDATA &sDamage = sItemCustom.getdamage();
		const SITEM* pTempITEM = GLItemMan::GetInstance().GetItem(sItemCustom.sNativeID); 

		int nMinDmg = static_cast<int>( pTempITEM->sSuitOp.gdDamage.dwLow ); 
		int nMaxDmg = static_cast<int>( pTempITEM->sSuitOp.gdDamage.dwHigh );
		int nRndOptDamage = static_cast<int> ( sDamage.dwLow - pTempITEM->sSuitOp.gdDamage.dwLow);
		int nExtraValue = static_cast<int> ( sItemCustom.GETGRADE_DAMAGE() );
		BYTE uGRADE = sItemCustom.GETGRADE(EMGRINDING_DAMAGE);

		DWORD dwDefault = NS_UITEXTCOLOR::dwDefault;
		DWORD dwInchant1 = NS_UITEXTCOLOR::dwInchant1;
		DWORD dwInchant2 = NS_UITEXTCOLOR::dwInchant2;

		if ( nMinDmg || nMaxDmg || nRndOptDamage || nExtraValue || uGRADE )
		{
			DWORD dwColor = NS_UITEXTCOLOR::dwDefault;
			if( NULL != psDisguiseSuit )
			{
				if( 0 != psDisguiseSuit->gdDamage.dwNow && 0 != psDisguiseSuit->gdDamage.dwMax )
					dwColor = ITEM_INFOR_COLOR::dwAddDisguise;
			}
			if ( uGRADE )
			{
				strTemp = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO", 0) );
				int nIndex = m_pBasicInformation->AddText( strTemp.c_str(), dwColor );

				strTemp = sc::string::format( "%d", nMinDmg );
				m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwColor );

				if(nRndOptDamage > 0 )
				{				
					strTemp = sc::string::format( "(+%d)", nRndOptDamage );
					m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}
				else if(nRndOptDamage < 0)
				{				
					strTemp = sc::string::format( "(%d)", nRndOptDamage );
					m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}

				if ( nExtraValue > 0 )
					strTemp = sc::string::format( "(+%d)", nExtraValue );
				else if ( nExtraValue < 0 )
					strTemp = sc::string::format( "(%d)", nExtraValue );
				m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwInchant2 );

				m_pBasicInformation->AddString( nIndex, " ~ ", dwColor );

				strTemp = sc::string::format( "%d", nMaxDmg );
				m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwColor );

				if(nRndOptDamage > 0)
				{			
					strTemp = sc::string::format( "(+%d)", nRndOptDamage );
					m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}
				else if(nRndOptDamage < 0)
				{				
					strTemp = sc::string::format( "(%d)", nRndOptDamage );
					m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}

				if ( nExtraValue > 0 )
					strTemp = sc::string::format( "(+%d)", nExtraValue );
				else if ( nExtraValue < 0 )
					strTemp = sc::string::format( "(%d)", nExtraValue );
				m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwInchant2 );
			}
			else
			{
				strTemp = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO", 0) );
				int nIndex = m_pBasicInformation->AddText( strTemp.c_str(), dwColor );

				strTemp = sc::string::format( "%d", nMinDmg );
				m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwColor );

				if(nRndOptDamage > 0)
				{			
					strTemp = sc::string::format( "(+%d)", nRndOptDamage );
					m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}
				else if(nRndOptDamage < 0)
				{
					strTemp = sc::string::format( "(%d)", nRndOptDamage );
					m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}

				m_pBasicInformation->AddString( nIndex, " ~ ", dwColor );

				strTemp = sc::string::format( "%d", nMaxDmg );
				m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwColor );

				if(nRndOptDamage > 0)
				{			
					strTemp = sc::string::format( "(+%d)", nRndOptDamage );
					m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}
				else if(nRndOptDamage < 0)
				{
					strTemp = sc::string::format( "(%d)", nRndOptDamage );
					m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}
			}
		}
	}

	{
		const SITEM* pTempITEM = GLItemMan::GetInstance().GetItem(sItemCustom.sNativeID); 

		int nBasicDef = static_cast<int>( pTempITEM->sSuitOp.nDefense);
		int nRndOptDef = static_cast<int>( sItemCustom.getdefense() - pTempITEM->sSuitOp.nDefense );
		int nExtraValue = sItemCustom.GETGRADE_DEFENSE();
		BYTE uGRADE = sItemCustom.GETGRADE(EMGRINDING_DEFENSE);

		DWORD dwDefault = ITEM_INFOR_COLOR::dwDefault;
		DWORD dwInchant1 = ITEM_INFOR_COLOR::dwInchant1;
		DWORD dwInchant2 = ITEM_INFOR_COLOR::dwInchant2;

		
		DWORD dwColor = ITEM_INFOR_COLOR::dwDefault;
		if( NULL != psDisguiseSuit )
		{
			if( 0 != psDisguiseSuit->nDefense )
				dwColor = ITEM_INFOR_COLOR::dwAddDisguise;
		}

		if ( nBasicDef || uGRADE || nRndOptDef )
		{
			if ( uGRADE )
			{
				strTemp = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO", 1) );
				int nIndex = m_pBasicInformation->AddText( strTemp.c_str(), dwColor );

				strTemp = sc::string::format( "%d", nBasicDef );
				m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwColor );

				if(nRndOptDef > 0)
				{
					strTemp = sc::string::format( "(+%d)", nRndOptDef );
					m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}
				else if(nRndOptDef < 0)
				{
					strTemp = sc::string::format( "(%d)", nRndOptDef );
					m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}

				if ( nExtraValue > 0 )
				{
					strTemp = sc::string::format( "(+%d)", nExtraValue );
					m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwInchant2 );
				}
				else if ( nExtraValue < 0 )
				{
					strTemp = sc::string::format( "(%d)", nExtraValue );
					m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwInchant2 );
				}
					
			}
			else
			{
				strTemp = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO", 1) );
				int nIndex = m_pBasicInformation->AddText( strTemp.c_str(), dwColor );

				strTemp = sc::string::format( "%d", nBasicDef );
				m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwColor );

				if(nRndOptDef > 0)
				{
					strTemp = sc::string::format( "(+%d)", nRndOptDef );
					m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}
				else if(nRndOptDef < 0)
				{
					strTemp = sc::string::format( "(%d)", nRndOptDef );
					m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwInchant1 );
				}
			}
		}
	}
	{
		//SITEM* pItemData = GLItemMan::GetInstance().GetItem ( sItemCustom.sNativeID );

		DWORD dwDefault = NS_UITEXTCOLOR::dwDefault;
		DWORD dwRndOptColor = NS_UITEXTCOLOR::dwInchant1;
		
		const SITEM* pTempITEM = GLItemMan::GetInstance().GetItem(sItemCustom.sNativeID); 

		
		int nBasicHitRate = static_cast<int>( pTempITEM->sSuitOp.nHitRate );
		int nRndOptHitRate = static_cast<int>( sItemCustom.GETHITRATE() - pItemData->sSuitOp.nHitRate );

		if ( nBasicHitRate )
		{
			DWORD dwColor = ITEM_INFOR_COLOR::dwDefault;
			if( NULL != psDisguiseSuit )
			{
				if( 0 != psDisguiseSuit->nHitRate )
					dwColor = ITEM_INFOR_COLOR::dwAddDisguise;
			}

			strTemp = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO", 3) );
			int nIndex = m_pBasicInformation->AddText( strTemp.c_str(), dwColor );

			strTemp = sc::string::format( "%d", nBasicHitRate );
			m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwColor );

			if(nRndOptHitRate > 0)
			{	
				strTemp = sc::string::format( "(+%d)", nRndOptHitRate );
				m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwRndOptColor );
			}
			else if(nRndOptHitRate < 0)
			{
				strTemp = sc::string::format( "(%d)", nRndOptHitRate );
				m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwRndOptColor );
			}
		}

		int nBasicAvoidRate = static_cast<int>( pTempITEM->sSuitOp.nAvoidRate );
		int nRndOptAvoidRate = static_cast<int>( sItemCustom.GETAVOIDRATE() - pItemData->sSuitOp.nAvoidRate );

		if ( nBasicAvoidRate || nRndOptAvoidRate )
		{
			DWORD dwColor = ITEM_INFOR_COLOR::dwDefault;
			if( NULL != psDisguiseSuit )
			{
				if( 0 != psDisguiseSuit->nAvoidRate )
					dwColor = ITEM_INFOR_COLOR::dwAddDisguise;
			}

			strTemp = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO", 4) );
			int nIndex =  m_pBasicInformation->AddText( strTemp.c_str(), dwColor );

			strTemp = sc::string::format( "%d", nBasicAvoidRate );
			m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwColor );

			if(nRndOptAvoidRate > 0)
			{	
				strTemp = sc::string::format( "(+%d)", nRndOptAvoidRate );
				m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwRndOptColor );
			}
			else if(nRndOptAvoidRate < 0)
			{
				strTemp = sc::string::format( "(%d)", nRndOptAvoidRate );
				m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwRndOptColor );
			}
		}

		//	소모 SP
		int nBasicReqSP = static_cast<int>( pTempITEM->sSuitOp.wReqSP );
		int nRndOptReqSP = static_cast<int>( sItemCustom.GETREQ_SP() - pTempITEM->sSuitOp.wReqSP);

		if ( nBasicReqSP || nRndOptReqSP )
		{
			DWORD dwColor = ITEM_INFOR_COLOR::dwDefault;
			if( NULL != psDisguiseSuit )
			{
				if( 0 != psDisguiseSuit->wReqSP )
					dwColor = ITEM_INFOR_COLOR::dwAddDisguise;
			}

			strTemp = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO", 7)  );
			int nIndex = m_pBasicInformation->AddText( strTemp.c_str(), dwColor );

			strTemp = sc::string::format( "%d", nBasicReqSP );
			m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwColor );

			if(nRndOptReqSP > 0)
			{	
				strTemp = sc::string::format( "(+%d)", nRndOptReqSP );
				m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwRndOptColor );
			}
			else if(nRndOptReqSP < 0)
			{
				strTemp = sc::string::format( "(%d)", nRndOptReqSP );
				m_pBasicInformation->AddString( nIndex, strTemp.c_str(), dwRndOptColor );
			}
		}
	}
}

void CItemRebuildOption::SetAdditionalInformation ( SITEMCUSTOM sItemCustom )
{
	m_pAdditionalInformation->ClearText();

	SITEM* pItemData = GLItemMan::GetInstance().GetItem ( sItemCustom.sNativeID );
	const ITEM::SSUIT& suitBasic = pItemData->sSuitOp;
	std::string strText = "";
	int nValueArray[EMADD_SIZE];
	SecureZeroMemory(nValueArray, sizeof(nValueArray));

	for ( unsigned int _i(ITEM::SSUIT::ADDON_SIZE); _i--; )
	{
		const EMITEM_ADDON emType = suitBasic.sADDON[_i].emTYPE;

		if ( emType != EMADD_NONE )
		{
			const int nValue(suitBasic.sADDON[_i].nVALUE);

			switch ( emType )
			{
				//	밑에서 한번에 계산
			case EMADD_HP:
			case EMADD_MP:
			case EMADD_SP:
			case EMADD_MA:
				break;
			default:
				nValueArray[emType] += nValue;
			};
		}
	}

	nValueArray[EMADD_HP] = static_cast<int>( sItemCustom.GETADDHP() );
	nValueArray[EMADD_MP] = static_cast<int>( sItemCustom.GETADDMP() );
	nValueArray[EMADD_SP] = static_cast<int>( sItemCustom.GETADDSP() );
	nValueArray[EMADD_MA] = static_cast<int>( sItemCustom.GETADDMA() );

	if ( sItemCustom.nidDISGUISE != NATIVEID_NULL() )
	{
		SITEM* pCostume = GLItemMan::GetInstance().GetItem ( sItemCustom.nidDISGUISE );

		if ( pCostume && ( pCostume->sBasicOp.IsDISGUISE() != NULL ) )
		{
			const ITEM::SSUIT& suitCostume = pCostume->sSuitOp;
			for ( unsigned int _i(ITEM::SSUIT::ADDON_SIZE); _i--; )
			{
				const EMITEM_ADDON emType(suitCostume.sADDON[_i].emTYPE);

				if ( emType != EMADD_NONE )
					continue;

				nValueArray[emType] += pCostume->sSuitOp.sADDON[_i].nVALUE;
			}
		}
	}

	const DWORD dwAddAbility(ITEM_INFOR_COLOR::dwAddAbility);

	for ( unsigned int _i(0); _i < EMADD_SIZE; ++_i )
	{
		const int nValue(nValueArray[_i]);
		if( nValue == 0 )
			continue;

		strText = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADDON_INFO", _i));

		const int nIndex(m_pAdditionalInformation->AddText(strText.c_str(), ITEM_INFOR_COLOR::dwAddAbility));

		strText = sc::string::format(nValue > 0 ? "+%d" : "%d", nValue);

		m_pAdditionalInformation->AddString( nIndex, strText.c_str(), dwAddAbility);
	}

	// Info : 특수 기능
	{
		float fValueArray[EMVAR_SIZE];

		for ( DWORD svaron=0; svaron<ITEM::SSUIT::VARIATION_SIZE; ++svaron )
		{
			const EMITEM_VAR emITEM_VAR(suitBasic.sVARIATE[svaron].emTYPE);
			SecureZeroMemory(fValueArray, sizeof(fValueArray));

			//	특수기능 ( 변화율 )
			//
			if ( emITEM_VAR != EMVAR_NONE )
			{
				switch ( emITEM_VAR )
				{
				case EMVAR_HP:
				case EMVAR_MP:
				case EMVAR_SP:
				case EMVAR_AP:
					break;
				default:
					fValueArray[emITEM_VAR] = suitBasic.sVARIATE[svaron].fVariate;
					break;
				};
			}
		}

			fValueArray[EMVAR_HP] = sItemCustom.GETINCHP();
			fValueArray[EMVAR_MP] = sItemCustom.GETINCMP();
			fValueArray[EMVAR_SP] = sItemCustom.GETINCSP();
			fValueArray[EMVAR_AP] = sItemCustom.GETINCAP(); 

		

		//	코스튬 능력치;
		const EMITEM_VAR emITEM_VOL(suitBasic.sVOLUME.emTYPE);
		float fCostumVolume = 0.0f;
		if ( sItemCustom.nidDISGUISE != NATIVEID_NULL() )
		{
			SITEM* pCostume = GLItemMan::GetInstance().GetItem ( sItemCustom.nidDISGUISE );

			//SITEMCUSTOM sCustomeItem(sItemCustom.nidDISGUISE);
			if ( pCostume && pCostume->sBasicOp.IsDISGUISE())
			{
				for ( DWORD svaron=0; svaron<ITEM::SSUIT::VARIATION_SIZE; ++svaron )
				{
					EMITEM_VAR emITEM_VAR_COS = pCostume->sSuitOp.sVARIATE[svaron].emTYPE;

					if ( emITEM_VAR_COS != EMVAR_NONE )
					{
						float fVariate = pCostume->sSuitOp.sVARIATE[svaron].fVariate;		

						switch ( emITEM_VAR_COS )
						{
						case EMVAR_HP:
						case EMVAR_MP:
						case EMVAR_SP:
						case EMVAR_AP:
							fValueArray[emITEM_VAR_COS] += fVariate;
							break;
						default:
							fValueArray[emITEM_VAR_COS] += fVariate;
							break;
						};
					}
				}

				EMITEM_VAR emITEM_VOL_COS = pCostume->sSuitOp.sVOLUME.emTYPE;

				if ( emITEM_VOL_COS != EMVAR_NONE)
				{
					fCostumVolume = pCostume->sSuitOp.sVOLUME.fVolume;
				}
			}
		}


		// 내구도 컬러;
		//const DWORD dwAddAbility(m_fDurabilityPerformance < 1.0f ? ItemDurability::GetInstance().GetTextColor(itemCustom, ITEM_INFOR_COLOR::dwAddAbility) : ITEM_INFOR_COLOR::dwAddAbility);
		const DWORD dwAddAbility( ITEM_INFOR_COLOR::dwAddAbility );

		BOOL bVAR_HP(FALSE), bVAR_MP(FALSE), bVAR_SP(FALSE), bVAR_AP(FALSE);

		for ( DWORD svaron=0; svaron<ITEM::SSUIT::VARIATION_SIZE; ++svaron )
		{
			EMITEM_VAR emITEM_VAR = pItemData->sSuitOp.sVARIATE[svaron].emTYPE;
			float fVAR_SCALE(0);
			if (emITEM_VAR != EMVAR_NONE)
			{
				float fVAR_SCALE(0);
				if ( emITEM_VAR != EMVAR_NONE )
				{
					switch ( emITEM_VAR )
					{
					case EMVAR_HP:
						bVAR_HP = TRUE;
						fVAR_SCALE = sItemCustom.GETINCHP();
						break;
					case EMVAR_MP:
						bVAR_MP = TRUE;
						fVAR_SCALE = sItemCustom.GETINCMP();
						break;
					case EMVAR_SP:
						bVAR_SP = TRUE;
						fVAR_SCALE = sItemCustom.GETINCSP();
						break;
					case EMVAR_AP:
						bVAR_AP = TRUE;
						fVAR_SCALE = sItemCustom.GETINCAP();
						break;
					default:
						fVAR_SCALE = pItemData->sSuitOp.sVARIATE[svaron].fVariate;
						break;
					};
					fVAR_SCALE *= COMMENT::ITEMVAR_SCALE[emITEM_VAR];
					//strText.Format("%s %2.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD", emITEM_VAR ), fVAR_SCALE );
					strText = sc::string::format( "%s : %2.2f", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD", emITEM_VAR ), fVAR_SCALE );
					if ( COMMENT::IsITEMVAR_SCALE(emITEM_VAR) )		strText += "%";
					//AddTextNoSplit(strText,NS_UITEXTCOLOR::GREEN);
					m_pAdditionalInformation->AddText( strText.c_str(), dwAddAbility );
					//m_pAdditionalInformation->AddString( nIndex, strText.c_str(), dwAddAbility );
									
				}
			}
		}

		/*for ( unsigned int _i = 1; _i  < EMVAR_SIZE; ++_i  )
		{
			if ( fValueArray[_i] != 0.0f ) 
			{
				if ( pItemData->sBasicOp.emItemType != ITEM_PET_FOOD && pItemData->sBasicOp.emItemType != ITEM_VEHICLE_OIL )
				{
					fValueArray[_i] *= COMMENT::ITEMVAR_SCALE[_i];
				}

				CString strTemp = sc::string::format( "%.2f", fValueArray[_i] ).c_str();

				_cutZero(strTemp,2);

				strText = sc::string::format( "%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD", _i) );
				int nIndex = m_pAdditionalInformation->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwAddAbility );

				strText = sc::string::format( "%s", strTemp);

				if ( sItemCustom.nidDISGUISE != NATIVEID_NULL() )
				{
					SITEM* pItem = GLItemMan::GetInstance().GetItem ( sItemCustom.nidDISGUISE );
					//const SITEM* pItem = GLogicData::GetInstance().GetItem( itemCustom.nidDISGUISE );
					//SITEMCUSTOM sCustomeItem(itemCustom.nidDISGUISE);
					if ( pItem && ( pItem->sBasicOp.IsDISGUISE() != NULL ) )
					{
						//const SITEM* pNativeItem = GLogicData::GetInstance().GetItem( itemCustom.GetNativeID() );
						SITEM* pNativeItem = GLItemMan::GetInstance().GetItem ( sItemCustom.sNativeID );
						if( pNativeItem )
						{
							// 문자열이 아닌 데이터는 CommetStrTable.xml에 없다;
							// 데이터는 기존 Comment데이터를 참조한다;
							
							for ( DWORD svaron=0; svaron<ITEM::SSUIT::VARIATION_SIZE; ++svaron )
							{
								EMITEM_VAR emITEM_VAR_COS = pNativeItem->sSuitOp.sVARIATE[svaron].emTYPE;
	// 							if ( GLUseFeatures::GetInstance().IsUsingDataIntegrate() )
	// 							{
	// 								if (atof(CGameTextMan::GetInstance().GetCommentText("ITEMVAR", emITEM_VAR_COS*2+1).GetString()) == 100.0f)
	// 									strText += "%";
	// 							}
	// 							else
 								{
									if ( COMMENT::IsITEMVAR_SCALE(emITEM_VAR_COS) )
										strText += "%";
								}
							}
						}
					}
				}
				else
				{
					for ( DWORD svaron=0; svaron<ITEM::SSUIT::VARIATION_SIZE; ++svaron )
					{
						EMITEM_VAR emITEM_VAR(suitBasic.sVARIATE[svaron].emTYPE);
						// 문자열이 아닌 데이터는 CommetStrTable.xml에 없다;
						// 데이터는 기존 Comment데이터를 참조한다;
	// 					if ( GLUseFeatures::GetInstance().IsUsingDataIntegrate() )
	// 					{
	// 						if (atof(CGameTextMan::GetInstance().GetCommentText("ITEMVAR", emITEM_VAR*2+1).GetString()) == 100.0f)
	// 							strText += "%";
	// 					}
	// 					else
						{
							if ( COMMENT::IsITEMVAR_SCALE(emITEM_VAR) )
								strText += "%";
						}
					}
				}

				m_pAdditionalInformation->AddString( nIndex, strText.c_str(), dwAddAbility );
			}
		}*/


		// 경험치 배율
		float fExpMultiple = pItemData->GetExpMultiple();
		
		//내구도 적용
		if( (fExpMultiple > 1.0f) )
		{
			fExpMultiple = (fExpMultiple - 1.0f)+ 1.0f;
		}

		//const SITEM* pItem = GLogicData::GetInstance().GetItem( itemCustom.nidDISGUISE );
		SITEM* pItem = GLItemMan::GetInstance().GetItem ( sItemCustom.nidDISGUISE );
		//SITEMCUSTOM sCustomeItem(itemCustom.nidDISGUISE);
		if ( pItem && pItem->sBasicOp.IsDISGUISE() )
		{
			fExpMultiple += pItem->GetExpMultiple() - 1.0f;
			fExpMultiple = max( fExpMultiple, 1.0f );
		}

		if ( fExpMultiple != 1.0f )
		{
			CString strTemp = sc::string::format( "%.2f", fExpMultiple * 100.0f - 100.0f ).c_str();

			_cutZero(strTemp,2);

			strText = sc::string::format( "%s :", ID2GAMEWORD( "ITEM_ADVANCED_INFO_SPECIAL_EX", 0 ));
			int nIndex = m_pAdditionalInformation->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwAddAbility );

			strText = sc::string::format( "+%s%%", strTemp);
			m_pAdditionalInformation->AddString( nIndex, strText.c_str(), dwAddAbility );
		}

		//	특수기능 ( 변화량 )
		//
		if ( emITEM_VOL != EMVAR_NONE)
		{
			if ( emITEM_VOL == EMVAR_MOVE_SPEED )
			{
				float fVOLUME = sItemCustom.GETMOVESPEED();
				CString strTemp = sc::string::format( "%.4f", fVOLUME ).c_str();
				_cutZero(strTemp,4);
				strText = sc::string::format("%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD_VOL", emITEM_VOL ));
				int nIndex = m_pAdditionalInformation->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwAddAbility );

				strText = sc::string::format( "%s", strTemp);
				m_pAdditionalInformation->AddString( nIndex, strText.c_str(), dwAddAbility );
			}
			else
			{
				float fVOLUME = (pItemData->sSuitOp.sVOLUME.fVolume) + fCostumVolume;
				CString strTemp = sc::string::format( "%.4f", fVOLUME ).c_str();
				_cutZero(strTemp,4);
				strText = sc::string::format("%s :", ID2GAMEWORD("ITEM_ADVANCED_INFO_SPECIAL_ADD_VOL", emITEM_VOL ));
				int nIndex = m_pAdditionalInformation->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwAddAbility );

				strText = sc::string::format( "%s", strTemp);
				m_pAdditionalInformation->AddString( nIndex, strText.c_str(), dwAddAbility );
			}
		}
	}

	
	// Info : 저항
	{
		int nELEC   = sItemCustom.GETRESIST_ELEC();
		int nFIRE   = sItemCustom.GETRESIST_FIRE();
		int nICE    = sItemCustom.GETRESIST_ICE();
		int nPOISON = sItemCustom.GETRESIST_POISON();
		int nSPIRIT = sItemCustom.GETRESIST_SPIRIT();

		if ( sItemCustom.nidDISGUISE != NATIVEID_NULL() )
		{
			//const SITEM* pItem = GLogicData::GetInstance().GetItem( itemCustom.nidDISGUISE );
			SITEM* pItem = GLItemMan::GetInstance().GetItem ( sItemCustom.nidDISGUISE );

			//코스튬 옵션 추가 - 코스튬 옵션은 내구도 영향을 받지 않음
			if ( pItem && ( pItem->sBasicOp.IsDISGUISE() != NULL ) )
			{
				nELEC       += pItem->sSuitOp.sResist.nElectric;
				nFIRE       += pItem->sSuitOp.sResist.nFire;
				nICE        += pItem->sSuitOp.sResist.nIce;
				nPOISON     += pItem->sSuitOp.sResist.nPoison;
				nSPIRIT     += pItem->sSuitOp.sResist.nSpirit;
			}
		}

		if ( nELEC || nFIRE || nICE || nPOISON || nSPIRIT )
		{
			// 내구도 컬러
			DWORD dwAddAbility = NS_UITEXTCOLOR::dwAddAbility;
			
			m_bDisplayResist = true;

			strText = sc::string::format("%s :", ID2GAMEWORD ("ITEM_BASIC_INFO", 12) );
			m_pAdditionalInformation->AddText( strText.c_str(), ITEM_INFOR_COLOR::dwAddAbility );
			
			//0이라도 출력
			//if ( nSPIRIT )
			{
				strText = sc::string::format("%d", nSPIRIT );
				m_pResistSpiritText->AddText( strText.c_str(), dwAddAbility );
			}
			//if ( nFIRE )
			{
				strText = sc::string::format("%d", nFIRE );
				m_pResistFireText->AddText( strText.c_str(), dwAddAbility );
			}
			//if ( nICE )
			{
				strText = sc::string::format("%d", nICE );
				m_pResistIceText->AddText( strText.c_str(), dwAddAbility );
			}
			//if ( nELEC )
			{
				strText = sc::string::format("%d", nELEC );
				m_pResistElectricText->AddText( strText.c_str(), dwAddAbility );
			}
			//if ( nPOISON )
			{
				strText = sc::string::format("%d", nPOISON );
				m_pResistPoisonText->AddText( strText.c_str(), dwAddAbility );
			}
		}
		else
		{
			m_bDisplayResist = false;

			m_pResistSpiritText->ClearText();
			m_pResistFireText->ClearText();
			m_pResistIceText->ClearText();
			m_pResistElectricText->ClearText();
			m_pResistPoisonText->ClearText();
		}
	}
}

bool CItemRebuildOption::SetData( SITEMCUSTOM sItemCust, int nIndex )
{
	m_pOptionTextBox->ClearText();
	DWORD dwRndOptColor = ITEM_INFOR_COLOR::dwRandomAbility;

	switch( nIndex )
	{
	case 0:
		{
			m_emOpt = sItemCust.GETOptTYPE1();
			m_fVal = sItemCust.GETOptVALUE1();
		}
		break;
	case 1:
		{
			m_emOpt = sItemCust.GETOptTYPE2();
			m_fVal = sItemCust.GETOptVALUE2();
		}
		break;
	case 2:
		{
			m_emOpt = sItemCust.GETOptTYPE3();
			m_fVal = sItemCust.GETOptVALUE3();
		}
		break;
	case 3:
		{
			m_emOpt = sItemCust.GETOptTYPE4();
			m_fVal = sItemCust.GETOptVALUE4();
		}
		break;
	}

	if( m_fVal == 0.0f )	return false;

	CString strDescText;
	strDescText.Format( "%s: ", ID2GAMEWORD("ITEM_RANDOM_OPTION", m_emOpt ) );
	if( m_fVal > 0.0f) strDescText += _T('+');
	strDescText.AppendFormat( "%.2f", m_fVal );
	if( sItemCust.IsPerRandOpt( m_emOpt ) ) strDescText += _T('%');

	m_pOptionTextBox->SetText( strDescText, dwRndOptColor );

	return true;
}
void CItemRebuildOption::_resizeInformationBox(void)
{
	enum
	{
		BASIC, ADDITIONAL, NSIZE
	};

	const int nLine[NSIZE] =
	{
		m_pBasicInformation->GetTotalLine(),
		m_pAdditionalInformation->GetTotalLine()
	};

	const float fInterval(5.0f);

	UIRECT uiPreviousGlobalSize;
	float fPreviousHeight = 0.0f;

	UIRECT uiBasicGlobalSize(m_pBasicInformation->GetGlobalPos());
	UIRECT uiBasicLocalSize(m_pBasicInformation->GetLocalPos());
	uiBasicLocalSize.top = 5.0f;
	const float fBasicHeight(m_pBasicInformation->CalcMaxHEIGHT(0, nLine[BASIC]));	
	if ( nLine[BASIC] != 0 )
	{	
		uiBasicGlobalSize.sizeY = uiBasicLocalSize.sizeY = fBasicHeight;		
		m_pBasicInformation->SetLocalPos(uiBasicLocalSize);
		m_pBasicInformation->SetGlobalPos(uiBasicGlobalSize);

		uiPreviousGlobalSize = uiBasicGlobalSize;
		fPreviousHeight = fBasicHeight;
	}	

	UIRECT uiAdditionalGlobalSize(m_pAdditionalInformation->GetGlobalPos());
	UIRECT uiAdditionalLocalSize(m_pAdditionalInformation->GetLocalPos());
	const float fAdditionalHeight(m_pAdditionalInformation->CalcMaxHEIGHT(0, nLine[ADDITIONAL]));
	if ( nLine[ADDITIONAL] != 0 )
	{	
		uiAdditionalGlobalSize.sizeY = uiAdditionalLocalSize.sizeY = fAdditionalHeight;
		uiAdditionalLocalSize.top = fAdditionalHeight;
		uiAdditionalGlobalSize.top = uiBasicGlobalSize.top + fBasicHeight + fInterval;
		m_pAdditionalInformation->SetLocalPos(uiAdditionalLocalSize);
		m_pAdditionalInformation->SetGlobalPos(uiAdditionalGlobalSize);

		uiPreviousGlobalSize = uiAdditionalGlobalSize;
		fPreviousHeight = fAdditionalHeight;
	}

	uiPreviousGlobalSize.top += 10.0f;
	 //must be manually reworked
	/*for ( unsigned int i = 0; i < 5; ++i )
	{
		const int nLine(m_pOptionTextBox->GetTotalLine());
		if (  nLine == 0 )
		{
			m_pOptionTextBox->SetVisibleSingle(FALSE);
			//m_stateCheckBox -= (1 << _i); // 플래그 해제;
			continue;	
		}
		
		UIRECT uiRandomGlobalSize(m_pOptionTextBox->GetGlobalPos());
		UIRECT uiRandomLocalSize(m_pOptionTextBox->GetLocalPos());

		UIRECT uiRandomGlobalPosition(m_pOptionCheckBox->GetGlobalPos());

		const float fAdditionalHeight(m_pOptionTextBox->CalcMaxHEIGHT(0, nLine));
		
		uiRandomGlobalSize.sizeY = uiRandomLocalSize.sizeY = fAdditionalHeight;
		uiRandomLocalSize.top = fAdditionalHeight;
		uiRandomGlobalSize.top = uiPreviousGlobalSize.top + fPreviousHeight + 1.0f;
		uiRandomGlobalPosition.top = uiPreviousGlobalSize.top + fPreviousHeight + 1.0f;

		m_pOptionTextBox->SetLocalPos(uiRandomLocalSize);
		m_pOptionTextBox->SetGlobalPos(uiRandomGlobalSize);
		m_pOptionCheckBox->SetGlobalPos(uiRandomGlobalPosition);

		uiPreviousGlobalSize.top = uiRandomGlobalSize.top;
		fPreviousHeight = fAdditionalHeight;

		m_pOptionTextBox->SetVisibleSingle(TRUE);		
	}*/

	if( true == m_bDisplayResist )
	{
		const float fResistOffset1 = 5.0f;
		const float fResistOffset2 = 3.0f;

		const UIRECT& AdditionalInfoGlobalSize = m_pAdditionalInformation->GetGlobalPos();
		const float fResistSizeX = m_pResistFire->GetLocalPos().sizeX;
		const float fResistSizeY = m_pResistFire->GetLocalPos().sizeY;

		int nTotalLine = m_pAdditionalInformation->GetTotalLine() - 2;

		D3DXVECTOR2 vPos;
		vPos.x = AdditionalInfoGlobalSize.right - fResistOffset1 * 14 - fResistSizeX * 10;
		vPos.y = AdditionalInfoGlobalSize.top + m_pAdditionalInformation->CalcMaxHEIGHT( 0, nTotalLine ) + 1.0f;

		vPos.x += fResistOffset1;
		vPos.y += fResistOffset2;
		m_pResistSpirit->SetLocalPos( vPos );
		m_pResistSpirit->SetGlobalPos( vPos );
		vPos.x += fResistSizeX;
		vPos.y -= fResistOffset2;
		m_pResistSpiritText->SetLocalPos( vPos );
		m_pResistSpiritText->SetGlobalPos( vPos );
		vPos.x += fResistSizeX;

		vPos.x += fResistOffset1;
		vPos.y += fResistOffset2;
		m_pResistFire->SetLocalPos( vPos );
		m_pResistFire->SetGlobalPos( vPos );
		vPos.x += fResistSizeX;
		vPos.y -= fResistOffset2;
		m_pResistFireText->SetLocalPos( vPos );
		m_pResistFireText->SetGlobalPos( vPos );
		vPos.x += fResistSizeX;

		vPos.x += fResistOffset1;
		vPos.y += fResistOffset2;
		m_pResistIce->SetLocalPos( vPos );
		m_pResistIce->SetGlobalPos( vPos );
		vPos.x += fResistSizeX;
		vPos.y -= fResistOffset2;
		m_pResistIceText->SetLocalPos( vPos );
		m_pResistIceText->SetGlobalPos( vPos );
		vPos.x += fResistSizeX;

		vPos.x += fResistOffset1;
		vPos.y += fResistOffset2;
		m_pResistElectric->SetLocalPos( vPos );
		m_pResistElectric->SetGlobalPos( vPos );
		vPos.x += fResistSizeX;
		vPos.y -= fResistOffset2;
		m_pResistElectricText->SetLocalPos( vPos );
		m_pResistElectricText->SetGlobalPos( vPos );
		vPos.x += fResistSizeX;

		vPos.x += fResistOffset1;
		vPos.y += fResistOffset2;
		m_pResistPoison->SetLocalPos( vPos );
		m_pResistPoison->SetGlobalPos( vPos );
		vPos.x += fResistSizeX;
		vPos.y -= fResistOffset2;
		m_pResistPoisonText->SetLocalPos( vPos );
		m_pResistPoisonText->SetGlobalPos( vPos );
	}

	m_pResistSpirit->SetVisibleSingle( m_bDisplayResist );
	m_pResistFire->SetVisibleSingle( m_bDisplayResist );
	m_pResistIce->SetVisibleSingle( m_bDisplayResist );
	m_pResistElectric->SetVisibleSingle( m_bDisplayResist );
	m_pResistPoison->SetVisibleSingle( m_bDisplayResist );
}