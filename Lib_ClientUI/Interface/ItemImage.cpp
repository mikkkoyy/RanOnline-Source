#include "StdAfx.h"
#include "ItemImage.h"
#include "GLGaeaClient.h"
#include "GLItemMan.h"
#include "SkillFunc.h"
#include "./BasicProgressBar.h"
#include "./BasicTextBox.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "d3dfont.h"
#include "UITextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//	리차징 컬러
const DWORD	RECHARGECOLOR = D3DCOLOR_ARGB(150,0,0,0);

CItemImage::CItemImage () 
	: m_pItemImageBack(NULL)
	, m_pItemImage ( NULL )
    , m_pItemImageOverlaped( NULL )
	, m_pOverImage(NULL)
	, m_pd3dDevice ( NULL )
	, m_sICONINDEXBACK ( NATIVEID_NULL () )
	, m_sICONINDEX ( NATIVEID_NULL () )
	, m_TotElapsedTime(0.0f)
	, m_bItemUsed(false)
	, m_bItemEnd(false)
	, m_bTwinkle(true)
	, m_pItemProcess( NULL )
	, m_sNativeID(false)
    , m_sNativeID_Overlaped(false)
	, m_pNumberText( NULL )
	, m_pTextBoxUpLeft( NULL )
	, m_pTextBoxUpRight( NULL )
	, m_pTextBoxDownLeft( NULL )
	, m_pTextBoxDownRight( NULL )
{
}

CItemImage::~CItemImage ()
{
}

HRESULT CItemImage::InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;
	hr = CUIGroup::InitDeviceObjects ( pd3dDevice );
	if ( FAILED ( hr ) ) return hr;

	m_pd3dDevice = pd3dDevice;

	return S_OK;
}

void	CItemImage::CreateSubControl (const std::string& overImageKey ,DWORD color, const char* szControlKeyword)
{
	m_pItemImageBack = new CUIControl;
	m_pItemImageBack->CreateSub ( this, szControlKeyword );	
	m_pItemImageBack->SetUseRender ( TRUE );
	m_pItemImageBack->SetVisibleSingle ( FALSE );
	RegisterControl ( m_pItemImageBack );

	m_pItemImage = new CUIControl();
	m_pItemImage->CreateSub ( this, szControlKeyword, UI_FLAG_DEFAULT, ITEM_IMAGE_SLOT );	
	m_pItemImage->SetUseRender ( TRUE );
	m_pItemImage->SetVisibleSingle ( FALSE );
	RegisterControl ( m_pItemImage );

    m_pItemImageOverlaped = new CUIControl();
    m_pItemImageOverlaped->CreateSub ( this, szControlKeyword, UI_FLAG_DEFAULT, ITEM_IMAGE_OVER );	
    m_pItemImageOverlaped->SetUseRender ( TRUE );
    m_pItemImageOverlaped->SetVisibleSingle ( FALSE );
    RegisterControl ( m_pItemImageOverlaped );

	if (!overImageKey.empty())
	{
		m_pOverImage = new CUIControl; 
		m_pOverImage->CreateSub( this, overImageKey.c_str() );	
		m_pOverImage->SetVisibleSingle(FALSE);
		m_pOverImage->SetDiffuse( color );
		RegisterControl(m_pOverImage);
	}

	CBasicProgressBar* pItemProcess = new CBasicProgressBar;
	pItemProcess->CreateSub ( this, "ITEM_IMAGE_PROGRESS" );
	pItemProcess->CreateOverImage ( "ITEM_IMAGE_PROGRESS_OVER" );
	pItemProcess->SetType ( CBasicProgressBar::VERTICAL );
	pItemProcess->SetOverImageUseRender ( TRUE );
	pItemProcess->SetOverImageDiffuse ( 0 );
	pItemProcess->SetPercent( 0 );
	pItemProcess->SetControlNameEx ( "프로그래스바" );	
	pItemProcess->SetVisibleSingle( FALSE );
	RegisterControl ( pItemProcess );
	m_pItemProcess = pItemProcess;

	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont( _DEFAULT_FONT, 8, D3DFONT_SHADOW | D3DFONT_ASCII );

	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, "ITEM_NUMBER0" );
	pTextBox->SetFont ( pFont8 );
	pTextBox->SetTextAlign ( TEXT_ALIGN_RIGHT );
	RegisterControl ( pTextBox );

	m_pNumberText = pTextBox;

	if( m_pNumberText ) m_pNumberText->SetVisibleSingle(FALSE);

}

void CItemImage::SetNumber ( int nNumber, int nMaxNumber )
{
	CString strNumber;

	if( nNumber > 1 )
	{		
		strNumber.Format ( "%d", nNumber );
		DWORD dwColor = NS_UITEXTCOLOR::GREENYELLOW;
		//if ( nNumber == nMaxNumber ) dwColor = NS_UITEXTCOLOR::ORANGE;
		m_pNumberText->SetOneLineText ( strNumber, dwColor );
		m_pNumberText->SetVisibleSingle ( TRUE );

		return;
	}	

	ResetNumber();
}

void CItemImage::ResetNumber()
{
	m_pNumberText->ClearText ();
	m_pNumberText->SetVisibleSingle ( FALSE );
}


void	CItemImage::CreateTextBoxUpLeft ()
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG );

	m_pTextBoxUpLeft = new CBasicTextBox;
	m_pTextBoxUpLeft->CreateSub ( this, "ITEM_IMAGE_TEXT_UP_LEFT" );
	m_pTextBoxUpLeft->SetFont ( pFont );
	m_pTextBoxUpLeft->SetTextAlign ( TEXT_ALIGN_LEFT );
	RegisterControl ( m_pTextBoxUpLeft );
}

void	CItemImage::CreateTextBoxUpRight ()
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG );

	m_pTextBoxUpRight = new CBasicTextBox;
	m_pTextBoxUpRight->CreateSub ( this, "ITEM_IMAGE_TEXT_UP_RIGHT" );
	m_pTextBoxUpRight->SetFont ( pFont );
	m_pTextBoxUpRight->SetTextAlign ( TEXT_ALIGN_RIGHT );
	RegisterControl ( m_pTextBoxUpRight );
}

void	CItemImage::CreateTextBoxDownLeft ()
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG );

	m_pTextBoxDownLeft = new CBasicTextBox;
	m_pTextBoxDownLeft->CreateSub ( this, "ITEM_IMAGE_TEXT_DOWN_LEFT" );
	m_pTextBoxDownLeft->SetFont ( pFont );
	m_pTextBoxDownLeft->SetTextAlign ( TEXT_ALIGN_LEFT );
	RegisterControl ( m_pTextBoxDownLeft );
}

void	CItemImage::CreateTextBoxDownRight ()
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG );

	m_pTextBoxDownRight = new CBasicTextBox;
	m_pTextBoxDownRight->CreateSub ( this, "ITEM_IMAGE_TEXT_DOWN_RIGHT" );
	m_pTextBoxDownRight->SetFont ( pFont );
	m_pTextBoxDownRight->SetTextAlign ( TEXT_ALIGN_RIGHT );
	RegisterControl ( m_pTextBoxDownRight );
}


void CItemImage::Update( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if( !m_pItemImage->GetTexture() )
		m_pItemImage->InitDeviceObjects ( m_pd3dDevice );

    if( !m_pItemImageOverlaped->GetTexture() )
        m_pItemImageOverlaped->InitDeviceObjects ( m_pd3dDevice );

	if( m_bItemUsed && m_bItemEnd )
	{
		m_TotElapsedTime += fElapsedTime;

		if( m_TotElapsedTime < 0.3f )		m_pItemProcess->SetPercent(0);
		else if( m_TotElapsedTime < 0.6f )	m_pItemProcess->SetPercent(1);
		else if( m_TotElapsedTime < 0.9f )	m_pItemProcess->SetPercent(0);
		else if( m_TotElapsedTime < 1.2f )	m_pItemProcess->SetPercent(1);
		else
		{
			m_pItemProcess->SetPercent(0);
			m_TotElapsedTime = 0.0f;
			m_bItemEnd = false;
			m_bItemUsed = false;
		}
	}

	//	쿨타임 표시
    if ( !m_pItemProcess )
	{
		GASSERT ( 0 && "프로그래스바가 만들어지지 않았습니다." );
		return ;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem( m_sNativeID );
	if ( !pItem )	return ;

	if ( !GLGaeaClient::GetInstance().GetCharacter()->IsCoolTime( m_sNativeID ) )
	{
		m_bItemEnd = true;
		return;
	}	

	ITEM_COOLTIME* pItemCoolTime = GLGaeaClient::GetInstance().GetCharacter()->GetCoolTime( m_sNativeID );
	if ( !pItemCoolTime )
	{
		m_bItemEnd = true;
		return;
	}

	m_pItemProcess->SetVisibleSingle( TRUE );
	m_pItemProcess->SetOverImageDiffuse ( RECHARGECOLOR );

	m_bItemEnd = false;

	__time64_t tCurTime = GLGaeaClient::GetInstance().GetCurrentTime().GetTime();
	__time64_t tUseTime = pItemCoolTime->tUseTime;
	__time64_t tCoolTime = pItemCoolTime->tCoolTime;

	float fCoolTime = tCoolTime - tUseTime;
	float fCurTime = fCoolTime - ( tCurTime - tUseTime );
	
	if ( fCoolTime == 0.0f )
	{
		fCoolTime = 1.0f;
	}

	const float fPercent = fCurTime / fCoolTime;
	m_pItemProcess->SetPercent ( fPercent );
	m_bItemUsed = true;
}

const bool	CItemImage::SetItem ( const SITEMCUSTOM& itemCustom )
{
    // 박스 포장품이라면 포장지이미지를 포장된아이템 이미지위에 덧씌우는 방식으로 표현;
    if ( itemCustom.IsBoxWrapped() )
    {
       SITEM* pItemData = GLItemMan::GetInstance().GetItem ( itemCustom.nidDISGUISE );
       //SITEM* pItemData = GLItemMan::GetInstance().GetItem ( itemCustom.GetBoxWrappedItemID() );
       if ( pItemData == NULL )
            return false;

        SITEM* pBoxWrapperItemData = GLItemMan::GetInstance().GetItem ( itemCustom.GetRealNativeID() );
        //SITEM* pBoxWrapperItemData = GLItemMan::GetInstance().GetItem ( itemCustom.sNativeID );
        if ( pBoxWrapperItemData == NULL )
            return false;

        return SetItem(pItemData->sBasicOp.sICONID, pItemData->GetInventoryFile(), pItemData->sBasicOp.sNativeID,
                pBoxWrapperItemData->sBasicOp.sICONID, pBoxWrapperItemData->GetInventoryFile(), pBoxWrapperItemData->sBasicOp.sNativeID);
    }
    return SetItem(itemCustom.GetNativeID());
}

const bool CItemImage::SetItem( SNATIVEID sNativeID )
{
	if( NATIVEID_NULL() == sNativeID )
	{
		ResetItem();
		return false;
	}

	SITEM* pItemData = GLItemMan::GetInstance().GetItem ( sNativeID );
    if ( pItemData == NULL )
        return false;

    return SetItem( pItemData->sBasicOp.sICONID, pItemData->GetInventoryFile(), pItemData->sBasicOp.sNativeID );
}

const bool CItemImage::SetItem ( SNATIVEID sICONINDEX, const char* szTexture, SNATIVEID sNativeID)
{	
	if ( m_pItemImage == NULL )
		return false;

	//	NOTE
	//		이전 프레임과 아이콘 ID가 같고,
	//		텍스쳐가 동일하다면 같은 이미지 이므로,
	//		더 이상 작업을 수행할 필요가 없다.

	if( m_sNativeID == sNativeID && sNativeID != NATIVEID_NULL() )
		return false;

	// 여기서 리턴하면 sNativeID가 적용안된다
// 	if ( (sICONINDEX == m_sICONINDEX) && (szTexture == m_pItemImage->GetTextureName ()) )
// 		return ;

	ResetItem ();
	SetItemProcess( sNativeID );

    m_pItemImage->SetTexturePos ( NS_SKILL::GetIconTexurePos ( sICONINDEX ) );
    m_pItemImage->SetTextureName ( szTexture );	

    //		if ( m_pd3dDevice )
    {	
        //			m_pItemImage->InitDeviceObjects ( m_pd3dDevice );
        m_pItemImage->SetVisibleSingle ( TRUE );

        m_sICONINDEX = sICONINDEX;
        m_strTextureName = szTexture;
    }

    if ( m_pd3dDevice )
        m_pItemImage->InitDeviceObjects ( m_pd3dDevice );

	return true;
}

const bool CItemImage::SetItem ( SNATIVEID sICONINDEX, const char* szTexture, SNATIVEID sNativeID, SNATIVEID sICONINDEX_Overlaped, const char* szTexture_Overlaped, SNATIVEID sNativeID_Overlaped )
{
    if ( m_pItemImage == NULL )
        return false;

    if ( m_pItemImageOverlaped == NULL )
        return false;

    //	NOTE
    //		이전 프레임과 아이콘 ID가 같고,
    //		텍스쳐가 동일하다면 같은 이미지 이므로,
    //		더 이상 작업을 수행할 필요가 없다.

    if( m_sNativeID == sNativeID && sNativeID != NATIVEID_NULL() )
        return false;

	// 여기서 리턴하면 포장된 아이템끼리 교환시 이미지가 변경되지 않는다;
	//if( m_sNativeID_Overlaped == sNativeID_Overlaped && sNativeID_Overlaped != NATIVEID_NULL() )
	//	return false;

    // 여기서 리턴하면 sNativeID가 적용안된다
    // 	if ( (sICONINDEX == m_sICONINDEX) && (szTexture == m_pItemImage->GetTextureName ()) )
    // 		return ;

    ResetItem ();
    SetItemProcess( sNativeID );
    
    // 내용물 이미지
    {
        m_pItemImage->SetTexturePos ( NS_SKILL::GetIconTexurePos ( sICONINDEX ) );
        m_pItemImage->SetTextureName ( szTexture );	

        //		if ( m_pd3dDevice )
        {	
            //			m_pItemImage->InitDeviceObjects ( m_pd3dDevice );
            m_pItemImage->SetVisibleSingle ( TRUE );

            m_sICONINDEX = sICONINDEX;
            m_strTextureName = szTexture;
        }

        if ( m_pd3dDevice )
            m_pItemImage->InitDeviceObjects ( m_pd3dDevice );
    }

    // 포장지 이미지 덧씌움;
    {
        m_sNativeID_Overlaped = sNativeID_Overlaped;

        m_pItemImageOverlaped->SetTexturePos ( NS_SKILL::GetIconTexurePos ( sICONINDEX_Overlaped ) );
        m_pItemImageOverlaped->SetTextureName ( szTexture_Overlaped );

        //		if ( m_pd3dDevice )
        {	
            //			m_pItemImage->InitDeviceObjects ( m_pd3dDevice );
            m_pItemImageOverlaped->SetVisibleSingle ( TRUE );
        }

        if ( m_pd3dDevice )
            m_pItemImageOverlaped->InitDeviceObjects ( m_pd3dDevice );
    }

    return true;
}

void CItemImage::SetSkill(SNATIVEID sICONINDEX, const char* szTexture, SNATIVEID sNativeID)
{
	if (!m_pItemImage)	return;

	//	NOTE
	//		이전 프레임과 아이콘 ID가 같고,
	//		텍스쳐가 동일하다면 같은 이미지 이므로,
	//		더 이상 작업을 수행할 필요가 없다.

	if (m_sNativeID == sNativeID && sNativeID != NATIVEID_NULL())	return;

	if ((sICONINDEX == m_sICONINDEX) && (szTexture == m_pItemImage->GetTextureName()))	return;


	ResetItem();
	SetItemProcess(sNativeID);

	if (m_pItemImage)
	{
		m_pItemImage->SetTexturePos(NS_SKILL::GetIconTexurePos(sICONINDEX));
		m_pItemImage->SetTextureName(szTexture);

		if (m_pd3dDevice)
		{
			m_pItemImage->InitDeviceObjects(m_pd3dDevice);
			m_pItemImage->SetVisibleSingle(TRUE);

			m_sICONINDEX = sICONINDEX;
			m_strTextureName = szTexture;
		}
	}
}

void CItemImage::SetBackItem( SNATIVEID sICONINDEX, const char* szTexture )
{	
	if ( m_pItemImageBack )
	{
		if ( (sICONINDEX == m_sICONINDEXBACK) && (szTexture == m_pItemImageBack->GetTextureName ()) )	return ;

		m_pItemImageBack->SetTexturePos ( NS_SKILL::GetIconTexurePos ( sICONINDEX ) );
		m_pItemImageBack->SetTextureName ( szTexture );	

		if ( m_pd3dDevice )
		{	
			m_sICONINDEXBACK = sICONINDEX;

			m_pItemImageBack->InitDeviceObjects ( m_pd3dDevice );
			m_pItemImageBack->SetVisibleSingle ( TRUE );
		}
	}
}

SNATIVEID	 CItemImage::GetItemImageSlotPos ()
{
	return m_sICONINDEX;
}

void CItemImage::ResetItem ()
{
	if ( m_pItemImage )
	{
		if ( m_sICONINDEX != NATIVEID_NULL () )
		{
			m_pItemImage->DeleteDeviceObjects ();
			m_pItemImage->SetTextureName ( NULL );
			m_pItemImage->SetVisibleSingle ( FALSE );

			m_sICONINDEX = NATIVEID_NULL ();

			ReSetItemProcess();

			m_strTextureName.Empty ();
		}		
	}

	if ( m_pItemImageBack )
	{
		m_sICONINDEXBACK = NATIVEID_NULL ();

		m_pItemImageBack->DeleteDeviceObjects ();
		m_pItemImageBack->SetTextureName ( NULL );
		m_pItemImageBack->SetVisibleSingle ( FALSE );
	}

    if ( m_pItemImageOverlaped )
    {
        m_pItemImageOverlaped->DeleteDeviceObjects ();
        m_pItemImageOverlaped->SetTextureName ( NULL );
        m_pItemImageOverlaped->SetVisibleSingle(FALSE);
    }

	m_sNativeID = NATIVEID_NULL();
    m_sNativeID_Overlaped = NATIVEID_NULL();

	ResetNumber();
}

void CItemImage::SetItemProcess( SNATIVEID sNativeID )
{
	m_bItemUsed = false;	
	m_sNativeID = sNativeID;

	if ( sNativeID == NATIVEID_NULL() )
	{
		m_pItemProcess->SetVisibleSingle( FALSE );
	}
	else
	{
		m_pItemProcess->SetVisibleSingle( TRUE );
	}

	return;
}

void CItemImage::ReSetItemProcess()
{
	m_bItemUsed = false;
	m_sNativeID = NATIVEID_NULL();
	m_pItemProcess->SetVisibleSingle( FALSE );
	m_pItemProcess->SetOverImageDiffuse ( 0 );
	m_pItemProcess->SetPercent( 0.0f );
	SetVisibleOverImage(false);
	return;
}

void CItemImage::SetDiffuse(D3DCOLOR _diffuse)
{
	CUIGroup::SetDiffuse( _diffuse );
	m_pItemProcess->SetOverImageDiffuse ( RECHARGECOLOR );
}

void CItemImage::SetVisibleOverImage( bool isVisible )
{
	if(m_pOverImage)
		m_pOverImage->SetVisibleSingle(isVisible);
}

bool CItemImage::IsVisibleOverImage()
{
	return m_pOverImage->IsVisible() ? true : false;
}

SNATIVEID CItemImage::GetItem()
{
	return m_sNativeID;
}

const CString& CItemImage::GetItemTextureName() const
{
	return m_strTextureName;
}

void CItemImage::SetTextUpLeft( CString strText, DWORD dwColor )
{
	if ( m_pTextBoxUpLeft )	m_pTextBoxUpLeft->SetText( strText.GetString(), dwColor );
}

void CItemImage::SetTextUpRight( CString strText, DWORD dwColor )
{
	if ( m_pTextBoxUpRight )	m_pTextBoxUpRight->SetText( strText.GetString(), dwColor );
}

void CItemImage::SetTextDownLeft( CString strText, DWORD dwColor )
{
	if ( m_pTextBoxDownLeft )	m_pTextBoxDownLeft->SetText( strText.GetString(), dwColor );
}

void CItemImage::SetTextDownRight( CString strText, DWORD dwColor )
{
	if ( m_pTextBoxDownRight )	m_pTextBoxDownRight->SetText( strText.GetString(), dwColor );
}

void CItemImage::SetTurnNum( int nNum )
{
}