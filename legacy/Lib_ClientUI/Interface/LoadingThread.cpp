#include "StdAfx.h"
#include <process.h>
#include "LoadingThread.h"

#include "SubPath.h"
#include "DxGrapUtils.h"
#include "DxResponseMan.h"
#include "UITextControl.h"
#include "GameTextControl.h"

#include "../../Lib_Engine/DxCommon/TextureManager.h"
#include "../../Lib_Engine/DxCommon/DxFontMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace NLOADINGTHREAD;
using namespace NLOADINGTIP;

//	-------------------- [ CUSTOM VERTEX 설정 ] --------------------
const	DWORD TEXTUREVERTEXFVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
struct TEXTUREVERTEX
{
	union
	{
		struct { D3DXVECTOR4 vPos; };
		struct { float x, y, z, rhw; };
	};

	DWORD Diffuse;

	union
	{
		struct { D3DXVECTOR2 vUV; };
		struct { float tu, tv; };
	};

	TEXTUREVERTEX () :
		x(0.0f),
		y(0.0f),
		z(0.0f),
		rhw(1.0f),
		Diffuse(0xFFFFFFFF),
		tu(0.0f),
		tv(0.0f)
	{		
	}

	TEXTUREVERTEX ( float _x, float _y, float _tu, float _tv ) :
		x(_x),
		y(_y),
		z(0.0f),
		rhw(1.0f),
		Diffuse(0xFFFFFFFF),
		tu(_tu),
		tv(_tv)
	{		
	}

	void	SetPos ( float _x, float _y )
	{
		x = _x;
		y = _y;
	}

	void	SetTexPos ( float _tu, float _tv )
	{
		tu = _tu;
		tv = _tv;
	}
};

//	-------------------- [ CUSTOM VERTEX 제어 메소드 설정 ] --------------------

static HRESULT	CreateVB ( LPDIRECT3DDEVICEQ pd3dDevice, LPDIRECT3DVERTEXBUFFERQ& pTextureVB, TEXTUREVERTEX VertexArray[6] )
{	
	HRESULT hr = S_OK;
	hr = pd3dDevice->CreateVertexBuffer( 6*sizeof(TEXTUREVERTEX), 0, TEXTUREVERTEXFVF,
											D3DPOOL_MANAGED, &pTextureVB, NULL );
    if( FAILED ( hr ) )	return hr;    

    VOID* pVertices;
	hr = pTextureVB->Lock( 0, sizeof ( TEXTUREVERTEX ) * 6, (VOID**)&pVertices, 0 );
    if( FAILED ( hr ) ) return hr;

    memmove( pVertices, VertexArray, sizeof ( TEXTUREVERTEX ) * 6 );

    hr = pTextureVB->Unlock();
	if ( FAILED ( hr ) ) return hr;

	return S_OK;
}

static void SetVertexPos ( TEXTUREVERTEX VertexArray[6], float LEFT, float TOP, float SIZEX, float SIZEY )
{
	float RIGHT = LEFT + SIZEX;
	float BOTTOM = TOP + SIZEY;

	VertexArray[0].SetPos ( LEFT,	TOP );
	VertexArray[1].SetPos ( RIGHT,	TOP);
	VertexArray[2].SetPos ( LEFT,	BOTTOM );

	VertexArray[3].SetPos ( LEFT,	BOTTOM );
	VertexArray[4].SetPos ( RIGHT,	TOP);
	VertexArray[5].SetPos ( RIGHT,	BOTTOM );
}

static void SetTexturePos ( TEXTUREVERTEX VertexArray[6], float LEFT, float TOP, float SIZEX, float SIZEY, float TEX_SIZEX, float TEX_SIZEY )
{
	float RIGHT = LEFT + SIZEX;
	float BOTTOM = TOP + SIZEY;

	VertexArray[0].SetTexPos ( LEFT / TEX_SIZEX,	TOP / TEX_SIZEY );
	VertexArray[1].SetTexPos ( RIGHT / TEX_SIZEX,	TOP / TEX_SIZEY);
	VertexArray[2].SetTexPos ( LEFT / TEX_SIZEX,	BOTTOM / TEX_SIZEY );

	VertexArray[3].SetTexPos ( LEFT / TEX_SIZEX,	BOTTOM / TEX_SIZEY );
	VertexArray[4].SetTexPos ( RIGHT / TEX_SIZEX,	TOP / TEX_SIZEY );
	VertexArray[5].SetTexPos ( RIGHT / TEX_SIZEX,	BOTTOM / TEX_SIZEY );
}

static HRESULT Render ( LPDIRECT3DDEVICEQ pd3dDevice, LPDIRECT3DTEXTUREQ pLoadingTexture, LPDIRECT3DVERTEXBUFFERQ pTextureVB )
{
	HRESULT hr = S_OK;

	hr = pd3dDevice->SetTexture ( 0, pLoadingTexture );
	if ( FAILED ( hr ) ) return hr;

    hr = pd3dDevice->SetStreamSource( 0, pTextureVB, 0, sizeof(TEXTUREVERTEX) );
	if ( FAILED ( hr ) ) return hr;

	hr = pd3dDevice->SetFVF( TEXTUREVERTEXFVF );
	if ( FAILED ( hr ) ) return hr;

	hr = pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 2 );
	if ( FAILED ( hr ) ) return hr;

	hr = pd3dDevice->SetTexture ( 0, NULL );
	if ( FAILED ( hr ) ) return hr;

	return S_OK;
}

BOOL MessagePump()
{
   MSG msg;

   while(::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
   {
       if(AfxGetApp() -> PumpMessage())
       {
             ::PostQuitMessage(0);
             return FALSE;
       }
   }

   return TRUE;
}

unsigned int WINAPI	LoadingThread( LPVOID pData )
{
	if( n_strTextureName.IsEmpty() || n_strTextureName == _T("null") )
	{
		srand ( (UINT)time( NULL ) );
		int nIndex = (rand() % 2) + 11;

		n_strTextureName.Format( "loading_%03d.dds", nIndex );	
	}

	TCHAR szTexture[256] = {0};
	StringCchPrintf ( szTexture, 256, n_strTextureName.GetString(), n_szAppPath, SUBPATH::TEXTURE_FILE_ROOT );

	LPDIRECT3DDEVICEQ& pd3dDevice = *n_ppd3dDevice;
	LPDIRECT3DTEXTUREQ pLoadingTexture = NULL;
	LPDIRECT3DTEXTUREQ pLoadingUnderTex = NULL;
	LPDIRECT3DTEXTUREQ pLoadingTopTex = NULL;
	LPDIRECT3DTEXTUREQ pLoadingStepTex = NULL;
	LPDIRECT3DTEXTUREQ pLoadingBackTex = NULL;
	LPDIRECT3DTEXTUREQ pHintIconTex = NULL;
	LPDIRECT3DTEXTUREQ pMapNameBackTex = NULL;

	std::string	  m_strLoadingStepTex= "loading_st.dds";
	std::string	  m_strLoadingBackTex = "ld_back.dds";
	std::string	  m_strLoadingUnderTex = "ld_under.dds";
	std::string	  m_strLoadingTopTex = "ld_top.dds";
	std::string	  m_strHintIconTex = "HintIcon.dds";
	std::string	  m_strMapNameBackTex = "mapnameback.dds";

	LPD3DXSPRITE pLoadingStepSprite = NULL;

	HRESULT hr = S_OK;

	if ( FAILED ( TextureManager::LoadTexture( szTexture, pd3dDevice, pLoadingTexture, 0, 0 ) ) )	
	{
		StringCchPrintf ( szTexture, 256, "loading_000.dds", n_szAppPath, SUBPATH::TEXTURE_FILE_ROOT );

		if ( FAILED ( TextureManager::LoadTexture( szTexture, pd3dDevice, pLoadingTexture, 0, 0 ) ) )	
		{
			return	ErrorLoadingTexture();
		}
	}

	if ( FAILED ( TextureManager::LoadTexture( m_strLoadingStepTex.c_str(), pd3dDevice, pLoadingStepTex, 0, 0 ) ) )	
		return	ErrorLoadingTexture();
	if ( FAILED ( TextureManager::LoadTexture( m_strLoadingBackTex.c_str(), pd3dDevice, pLoadingBackTex, 0, 0 ) ) )	
		return	ErrorLoadingTexture();
	if ( FAILED ( TextureManager::LoadTexture( m_strLoadingUnderTex.c_str(), pd3dDevice, pLoadingUnderTex, 0, 0 ) ) )	
		return	ErrorLoadingTexture();
	if ( FAILED ( TextureManager::LoadTexture( m_strLoadingTopTex.c_str(), pd3dDevice, pLoadingTopTex, 0, 0 ) ) )	
		return	ErrorLoadingTexture();
	if ( FAILED ( TextureManager::LoadTexture( m_strHintIconTex.c_str(), pd3dDevice, pHintIconTex, 0, 0 ) ) )	
		return	ErrorLoadingTexture();
	if ( FAILED ( TextureManager::LoadTexture( m_strMapNameBackTex.c_str(), pd3dDevice, pMapNameBackTex, 0, 0 ) ) )	
		return	ErrorLoadingTexture();

	DeWait ();

	RECT rect;
	GetClientRect ( n_hWnd, &rect );

	const float fWidth = float(rect.right - rect.left);
	const float fHeight = float(rect.bottom - rect.top);

	const float	fRealImageX = 1024.0f;
	const float	fRealImageY = 768.0f;
	const float	fImageSize = 1023.0f;

	const float fWidthRatio = fRealImageX / fImageSize;
	const float fHeightRatio= fRealImageY / fImageSize;

	const float LeftPos  = 0.0f;
	const float TopPos   = 0.0f;
	const float RightPos = fWidth;
	const float BottomPos= fHeight;

	float ROOT_LEFT		= LeftPos;
	float ROOT_TOP		= TopPos;
	float ROOT_SIZEX	= fWidth;
	float ROOT_SIZEY	= fHeight;

	const D3DXVECTOR2 vld_topTexPos ( 0, 0 );
	const D3DXVECTOR2 vld_topTexSize ( 1024, 256 );
	const D3DXVECTOR2 vld_topTexRecSize ( 1024, 140 );
	const D3DXVECTOR2 vld_topRenderPos ( 0, 0 );
	const D3DXVECTOR2 vld_topRenderSize ( 1024, 128 );

	const D3DXVECTOR2 vld_midTexPos ( 0, 0 );
	const D3DXVECTOR2 vld_midTexSize ( 1024, 512 );
	const D3DXVECTOR2 vld_midTexRecSize ( 1024, 512 );
	const D3DXVECTOR2 vld_midRenderPos ( 0, 128 );
	const D3DXVECTOR2 vld_midRenderSize ( 1024, 512 );

	const D3DXVECTOR2 vld_underTexPos ( 0, 7 );
	const D3DXVECTOR2 vld_underTexSize ( 1024, 256 );
	const D3DXVECTOR2 vld_underTexRecSize ( 1024, 140 );
	const D3DXVECTOR2 vld_underRenderPos ( 0, 640 );
	const D3DXVECTOR2 vld_underRenderSize ( 1024, 128 );

	const D3DXVECTOR2 vld_backTexPos ( 0, 0 );
	const D3DXVECTOR2 vld_backTexSize ( 128, 128 );
	const D3DXVECTOR2 vld_backTexRecSize ( 128, 128 );
	const D3DXVECTOR2 vld_backRenderSize ( 128, 128 );
	const D3DXVECTOR2 vld_backRenderPos ( fWidth - (vld_backRenderSize.x+15.0f), fHeight -(vld_backRenderSize.y+5.0f) );

	const D3DXVECTOR2 vld_HintIconTexPos ( 0, 0 );
	const D3DXVECTOR2 vld_HintIconTexSize ( 128, 64 );
	const D3DXVECTOR2 vld_HintIconTexRecSize ( 100, 60 );
	const D3DXVECTOR2 vld_HintIconRenderPos ( 15, 645 );
	const D3DXVECTOR2 vld_HintIconRenderSize ( 100, 60 );

	const D3DXVECTOR2 vld_MapNameBackTexPos ( 0, 0 );
	const D3DXVECTOR2 vld_MapNameBackTexSize ( 512, 64 );
	const D3DXVECTOR2 vld_MapNameBackTexRecSize ( 336, 57 );
	const D3DXVECTOR2 vld_MapNameBackRenderSize ( 336, 57 );
	const D3DXVECTOR2 vld_MapNameBackRenderPos( (fWidth - vld_MapNameBackRenderSize.x) /2 , (vld_topRenderSize.y - vld_MapNameBackRenderSize.y) /2);

	D3DXVECTOR2 vCopyrightAlign;
	vCopyrightAlign.x = 15.0f;
	vCopyrightAlign.y = fHeight - 30.0f;

	D3DXVECTOR2 vld_topAlignSize;
	vld_topAlignSize.x = static_cast<float>(floor(vld_topRenderSize.x * fWidth / fRealImageX));
	vld_topAlignSize.y = static_cast<float>(floor(vld_topRenderSize.y * fHeight / fRealImageY));

	D3DXVECTOR2 vld_topAlignPos;
	vld_topAlignPos.x = static_cast<float>(floor(vld_topRenderPos.x * fWidth / fRealImageX));
	vld_topAlignPos.y = static_cast<float>(floor(vld_topRenderPos.y * fHeight / fRealImageY));

	D3DXVECTOR2 vld_midAlignSize;
	vld_midAlignSize.x = static_cast<float>(floor(vld_midRenderSize.x * fWidth / fRealImageX));
	vld_midAlignSize.y = static_cast<float>(floor(vld_midRenderSize.y * fHeight / fRealImageY));
	D3DXVECTOR2 vld_midAlignPos;
	vld_midAlignPos.x = static_cast<float>(floor(vld_midRenderPos.x * fWidth / fRealImageX));
	vld_midAlignPos.y = static_cast<float>(floor(vld_midRenderPos.y * fHeight / fRealImageY));

	D3DXVECTOR2 vld_underAlignSize;
	vld_underAlignSize.x = static_cast<float>(floor(vld_underRenderSize.x * fWidth / fRealImageX));
	vld_underAlignSize.y = static_cast<float>(floor(vld_underRenderSize.y * fHeight / fRealImageY));

	D3DXVECTOR2 vld_underAlignPos;
	vld_underAlignPos.x = static_cast<float>(floor(vld_underRenderPos.x * fWidth / fRealImageX));
	vld_underAlignPos.y = static_cast<float>(floor(vld_underRenderPos.y * fHeight / fRealImageY));

	D3DXVECTOR2 vld_HintIconAlignPos;
	vld_HintIconAlignPos.x = static_cast<float>(floor(vld_HintIconRenderPos.x * fWidth / fRealImageX));
	vld_HintIconAlignPos.y = static_cast<float>(floor(vld_HintIconRenderPos.y * fHeight / fRealImageY));

	LPDIRECT3DVERTEXBUFFERQ pTextureVB	= NULL; 
	{
		TEXTUREVERTEX VertexArray[6];
		SetVertexPos ( VertexArray, ROOT_LEFT + vld_midAlignPos.x, ROOT_TOP + vld_midAlignPos.y, vld_midAlignSize.x, vld_midAlignSize.y );
		SetTexturePos( VertexArray, vld_midTexPos.x, vld_midTexPos.y, vld_midTexRecSize.x, vld_midTexRecSize.y, vld_midTexSize.x, vld_midTexSize.y );

		if ( FAILED ( CreateVB ( pd3dDevice, pTextureVB, VertexArray ) ) )	
			return ErrorCreateVB();
	}

	LPDIRECT3DVERTEXBUFFERQ pldTopVB	= NULL; 
	{
		TEXTUREVERTEX VertexArray[6];
		SetVertexPos ( VertexArray, ROOT_LEFT + vld_topAlignPos.x, ROOT_TOP + vld_topAlignPos.y, vld_topAlignSize.x, vld_topAlignSize.y );
		SetTexturePos( VertexArray, vld_topTexPos.x, vld_topTexPos.y, vld_topTexRecSize.x, vld_topTexRecSize.y, vld_topTexSize.x, vld_topTexSize.y );

		if ( FAILED ( CreateVB ( pd3dDevice, pldTopVB, VertexArray ) ) )	
			return ErrorCreateVB();
	}

	LPDIRECT3DVERTEXBUFFERQ pldUnderVB	= NULL; 
	{
		TEXTUREVERTEX VertexArray[6];
		SetVertexPos ( VertexArray, ROOT_LEFT + vld_underAlignPos.x, ROOT_TOP + vld_underAlignPos.y, vld_underAlignSize.x, vld_underAlignSize.y );
		SetTexturePos( VertexArray, vld_underTexPos.x, vld_underTexPos.y, vld_underTexRecSize.x, vld_underTexRecSize.y, vld_underTexSize.x, vld_underTexSize.y );

		if ( FAILED ( CreateVB ( pd3dDevice, pldUnderVB, VertexArray ) ) )	
			return ErrorCreateVB();
	}

	LPDIRECT3DVERTEXBUFFERQ pldbackVB	= NULL; 
	{
		TEXTUREVERTEX VertexArray[6];
		SetVertexPos ( VertexArray, ROOT_LEFT + vld_backRenderPos.x, ROOT_TOP + vld_backRenderPos.y, vld_backRenderSize.x, vld_backRenderSize.y );
		SetTexturePos( VertexArray, vld_backTexPos.x, vld_backTexPos.y, vld_backTexRecSize.x, vld_backTexRecSize.y, vld_backTexSize.x, vld_backTexSize.y );

		if ( FAILED ( CreateVB ( pd3dDevice, pldbackVB, VertexArray ) ) )	
			return ErrorCreateVB();
	}

	LPDIRECT3DVERTEXBUFFERQ pldHintIconVB	= NULL; 
	{
		TEXTUREVERTEX VertexArray[6];
		SetVertexPos ( VertexArray, ROOT_LEFT + vld_HintIconAlignPos.x, ROOT_TOP + vld_HintIconAlignPos.y, vld_HintIconRenderSize.x, vld_HintIconRenderSize.y );
		SetTexturePos( VertexArray, vld_HintIconTexPos.x, vld_HintIconTexPos.y, vld_HintIconTexRecSize.x, vld_HintIconTexRecSize.y, vld_HintIconTexSize.x, vld_HintIconTexSize.y );

		if ( FAILED ( CreateVB ( pd3dDevice, pldHintIconVB, VertexArray ) ) )	
			return ErrorCreateVB();
	}

	LPDIRECT3DVERTEXBUFFERQ pldMapNameBackVB	= NULL; 
	{
		TEXTUREVERTEX VertexArray[6];
		SetVertexPos ( VertexArray, ROOT_LEFT + vld_MapNameBackRenderPos.x, ROOT_TOP + vld_MapNameBackRenderPos.y, vld_MapNameBackRenderSize.x, vld_MapNameBackRenderSize.y );
		SetTexturePos( VertexArray, vld_MapNameBackTexPos.x, vld_MapNameBackTexPos.y, vld_MapNameBackTexRecSize.x, vld_MapNameBackTexRecSize.y, vld_MapNameBackTexSize.x, vld_MapNameBackTexSize.y );

		if ( FAILED ( CreateVB ( pd3dDevice, pldMapNameBackVB, VertexArray ) ) )	
			return ErrorCreateVB();
	}

	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );
	if( pFont )
		pFont->UsageCS( TRUE );

	CD3DFontPar* pFont12 = DxFontMan::GetInstance().LoadDxFont( _DEFAULT_FONT, 12, _DEFAULT_FONT_FLAG );
	if( pFont12 )
		pFont12->UsageCS( TRUE );

	int	ldstep = 0;
	int ldstepmulti = 1;
	int nTipIndex = 0;
	
	if ( n_bTIP )
	{
		if( NLOADINGTIP::GetTipSize() > 0 )
		{
			srand ( (UINT)time( NULL ) );
			nTipIndex = rand () % NLOADINGTIP::GetTipSize();
		}
	}

	while ( n_bRender )
	{
		if( pLoadingStepSprite == NULL )
		{
			if ( FAILED (D3DXCreateSprite(pd3dDevice, &pLoadingStepSprite)))
				MessageBoxA( NULL, "Cannot Create Sprite", 0, 0 );
		}

		HRESULT hr;

		if( FAILED( hr = pd3dDevice->TestCooperativeLevel() ) )
		{
			if( D3DERR_DEVICELOST == hr )
			{
				CDebugSet::ToListView ( "[ERROR] D3DERR_DEVICELOST _ LoadingThread() FAILED" );
				continue;
			}

			if( D3DERR_DEVICENOTRESET == hr )
				continue;
		}

		hr = pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0L );

		if( SUCCEEDED( pd3dDevice->BeginScene() ) )
		{
			hr = Render ( pd3dDevice, pLoadingTopTex, pldTopVB );
			hr = Render ( pd3dDevice, pLoadingTexture, pTextureVB );
			hr = Render ( pd3dDevice, pLoadingUnderTex, pldUnderVB );

			DWORD dwAlphaBlendEnable;
			pd3dDevice->GetRenderState ( D3DRS_ALPHABLENDENABLE, &dwAlphaBlendEnable );

			pd3dDevice->SetRenderState ( D3DRS_ALPHABLENDENABLE, TRUE );
			pd3dDevice->SetRenderState ( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
			pd3dDevice->SetRenderState ( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

			DWORD dwAlphaOP;
			pd3dDevice->GetTextureStageState( 0, D3DTSS_ALPHAOP,   &dwAlphaOP );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );

			DWORD dwMin, dwMag, dwMip;
			pd3dDevice->GetSamplerState( 0, D3DSAMP_MINFILTER,	&dwMin );
			pd3dDevice->GetSamplerState( 0, D3DSAMP_MAGFILTER,	&dwMag );
			pd3dDevice->GetSamplerState( 0, D3DSAMP_MIPFILTER,	&dwMip );

			pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER,	D3DTEXF_POINT );
			pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER,	D3DTEXF_POINT );
			pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER,	D3DTEXF_NONE );

			hr = Render ( pd3dDevice, pLoadingBackTex, pldbackVB );

			RECT srcRectLoadingSprite;
			srcRectLoadingSprite.top    = (( ldstep / 4 ) * 105);
			srcRectLoadingSprite.left   = (( ldstep % 4 ) * 105);
			srcRectLoadingSprite.bottom = (( ldstep / 4 ) * 105) + 105;
			srcRectLoadingSprite.right  = (( ldstep % 4 ) * 105) + 105;

			D3DXVECTOR3 vPosition( vld_backRenderPos.x + 12.0f, vld_backRenderPos.y + 12.0f, 0.0f );
			pLoadingStepSprite->Begin( D3DXSPRITE_ALPHABLEND );
			pLoadingStepSprite->Draw( pLoadingStepTex, &srcRectLoadingSprite, &D3DXVECTOR3( 0.0f, 0.0f, 0.0f ), &vPosition, D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f) );

			if ( n_bTIP )
			{
				hr = Render ( pd3dDevice, pHintIconTex, pldHintIconVB );

				pFont->DrawText( vld_HintIconAlignPos.x+85.0f, vld_HintIconAlignPos.y+20.0f, NS_UITEXTCOLOR::WHITE, NLOADINGTIP::GetTip( nTipIndex ) );	
				pFont->DrawText( vld_HintIconAlignPos.x+85.0f, vld_HintIconAlignPos.y+35.0f, NS_UITEXTCOLOR::WHITE, NLOADINGTIP::GetTip( nTipIndex+1 ) );
			}
			
			if ( !n_strMapName.IsEmpty() )
			{
				hr = Render ( pd3dDevice, pMapNameBackTex, pldMapNameBackVB );

				CString strMapName("");
				strMapName.Format("< %s >", n_strMapName.GetString() );
				if( pFont12 )
				{
					SIZE textsizeMapName;
					HRESULT hrGetExtent = pFont12->GetTextExtent ( strMapName.GetString(), textsizeMapName );
					if ( hrGetExtent == S_OK )
					{
						float fCenterScreenX = ( ROOT_SIZEX / 2.f ) - float( textsizeMapName.cx / 2 );
						pFont12->DrawText( fCenterScreenX, vld_MapNameBackRenderPos.y+20.0f, NS_UITEXTCOLOR::WHITE, strMapName.GetString() );
					}
				}
			}

			if( pFont )
				pFont->DrawText( vCopyrightAlign.x, vCopyrightAlign.y, NS_UITEXTCOLOR::WHITE, ID2GAMEWORD("COPYRIGHT_COMPANY_LOAD") );

			pLoadingStepSprite->End();

			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   dwAlphaOP );
			pd3dDevice->SetRenderState ( D3DRS_ALPHABLENDENABLE, dwAlphaBlendEnable );

			pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER,	dwMin );
			pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER,	dwMag );
			pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER,	dwMip );

			pd3dDevice->EndScene();
			pd3dDevice->Present( NULL, NULL, NULL, NULL );
		}

		++ldstep;

		if( ldstep > 3 )	
		{
			ldstep = 0;
			++ldstepmulti;

			if ( ldstepmulti > 8 )
				ldstepmulti = 0;
		}

		Sleep(100*ldstepmulti);
	}

	if( pFont )
		pFont->UsageCS( FALSE );

	if( pFont12 )
		pFont12->UsageCS( FALSE );

	SAFE_RELEASE ( pTextureVB );
	SAFE_RELEASE ( pldTopVB );
	SAFE_RELEASE ( pldUnderVB );
	SAFE_RELEASE ( pldbackVB );
	SAFE_RELEASE ( pldHintIconVB );
	SAFE_RELEASE ( pldMapNameBackVB );
	SAFE_RELEASE ( pLoadingStepSprite );

	TextureManager::ReleaseTexture( szTexture, pLoadingTexture );
	TextureManager::ReleaseTexture( m_strLoadingTopTex.c_str(), pLoadingTopTex );
	TextureManager::ReleaseTexture( m_strLoadingUnderTex.c_str(), pLoadingUnderTex );
	TextureManager::ReleaseTexture( m_strLoadingStepTex.c_str(), pLoadingStepTex );
	TextureManager::ReleaseTexture( m_strLoadingBackTex.c_str(), pLoadingBackTex );
	TextureManager::ReleaseTexture( m_strLoadingStepTex.c_str(), pLoadingStepTex );
	TextureManager::ReleaseTexture( m_strHintIconTex.c_str(), pHintIconTex );
	TextureManager::ReleaseTexture( m_strMapNameBackTex.c_str(), pMapNameBackTex );

	DeWait ();

	n_ExitState = eNORMAL;

	return 0;
}

namespace	NLOADINGTIP
{
	LOADING_TIP_VEC		n_vecTip;

	int GetTipSize()
	{
		return (int)n_vecTip.size();
	}

	void InsertTip( CString strText )
	{
		if ( strText.GetLength() <= 0 )	return;
		n_vecTip.push_back( strText );
	}

	CString GetTip( int nIndex )
	{
		if ( n_vecTip.empty() )	return "";
		if ( nIndex < 0 )		return "";
		if ( nIndex >= (int)n_vecTip.size() )	return "";
		return n_vecTip[nIndex];
	};

	void Clear()
	{
		n_vecTip.clear();
	}
};

namespace	NLOADINGTHREAD
{
	DWORD				n_dwThreadID;
	LPDIRECT3DDEVICEQ*	n_ppd3dDevice;
	HWND				n_hWnd;
	BOOL				n_bWait;
	BOOL				n_bRender;
	char				n_szAppPath[MAX_PATH] = {0};
	int					n_ExitState;
	int					n_Step = 0;
	HANDLE				n_hThread = NULL;
	CString				n_strTextureName = "";
	CString				n_strMapName = "";
	BOOL				n_bTIP = FALSE;

	HRESULT	StartThreadLOAD(LPDIRECT3DDEVICEQ* ppd3dDevice, 
							HWND hWnd, 
							char* szAppPath, 
							const CString & strTextureName, 
							const CString & strMapName,
							BOOL bTIP )
	{
		n_ppd3dDevice = ppd3dDevice;
		n_hWnd = hWnd;
		StringCchCopy ( n_szAppPath, MAX_PATH, szAppPath );
		n_strTextureName = strTextureName;
		n_strMapName = strMapName;
		n_bTIP = bTIP;
		
		n_bRender = TRUE;	//	렌더
		EnWait ();
		n_ExitState = eNORMAL;

		//n_hThread = CreateThread( NULL, 0, LoadingThread, NULL, 0, &n_dwThreadID );
		n_hThread = (HANDLE) ::_beginthreadex(
									NULL,
									0,
									LoadingThread,
									NULL,
									0,
									(unsigned int*) &n_dwThreadID );
		if ( !n_hThread )
		{
			DeWait();
			return E_FAIL;
		}

		return S_OK;
	}

	void	WaitThread ()
	{
		while ( GetWait () )
		{		
			Sleep( 1 );
		}
		EnWait ();
	}

	void	EndThread ()
	{		
		n_bRender = FALSE;

		if ( n_ExitState == eNORMAL )
		{
			WaitThread ();
		}

		CloseHandle( n_hThread );
		n_hThread = NULL;

		n_strTextureName.Empty();
	}

	BOOL	GetWait ()
	{
		return n_bWait;
	}

	void	DeWait  ()
	{
		n_bWait = FALSE;
	}

	void	EnWait ()
	{
		n_bWait = TRUE;
	}

	void	SetStep ( int step )
	{
		n_Step = step;
	}

	int		GetStep ()
	{
		return n_Step;
	}

	int		ErrorLoadingTexture()
	{
		DeWait ();
		n_ExitState = eERROR;
		return 0;
	}

	int		ErrorCreateVB()
	{
		n_ExitState = eERROR;
		return 0;
	}
};