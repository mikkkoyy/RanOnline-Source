#include "stdafx.h"
#include "GLLevelEtcFunction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL SLEVEL_ETC_FUNC::LOAD ( basestream &SFile )
{
	DWORD dwVersion, dwSize;
	SFile >> dwVersion;
	SFile >> dwSize;

	if ( dwVersion==VERSION )
	{
		LOAD_0200( SFile, dwSize );
	}
	else if ( dwVersion==0x0105 )
	{
		LOAD_0105( SFile, dwSize );
	}
	else if ( dwVersion==0x0104 )
	{
		LOAD_0104( SFile, dwSize );
	}
	else if ( dwVersion==0x0103 )
	{
		LOAD_0103( SFile, dwSize );
	}
	else if ( dwVersion==0x0102 )
	{
		LOAD_0102( SFile, dwSize );
	}
	else if ( dwVersion==0x0101 )
	{
		LOAD_0101( SFile, dwSize );
	}
	else if ( dwVersion==0x0100 )
	{
		LOAD_0100( SFile, dwSize );
	}
	else
	{
		CDebugSet::ErrorVersion( "SLEVEL_ETC_FUNC::LOAD", dwVersion );
		SFile.SetOffSet ( SFile.GetfTell()+dwSize );
	}

	return TRUE;
}

void SLEVEL_ETC_FUNC::LOAD_0100 ( basestream &SFile, DWORD dwSIZE )
{
	DWORD dwRead = SFile.GetfTell();

	//SFile.ReadBuffer( &m_bUseFunction, sizeof(m_bUseFunction) );
	SFile >> m_bUseFunction[EMETCFUNC_LIMITTIME];
	SFile >> m_bUseFunction[EMETCFUNC_CONTROLCAM];
	SFile >> m_bUseFunction[EMETCFUNC_LANDEFFECT];
	SFile >> m_bUseFunction[EMETCFUNC_DIFFICULTY];
	SFile >> m_bUseFunction[EMETCFUNC_AROUNDREGEN];

	// Load Limit Time
	if( m_bUseFunction[EMETCFUNC_LIMITTIME] ) 
	{
		SFile >> m_sLimitTime.nLimitMinute;
		SFile >> m_sLimitTime.nTellTermMinute;
	}

	// Load Camera Control
	if( m_bUseFunction[EMETCFUNC_CONTROLCAM] ) 
	{
		SFile >> m_sControlCam.fUp;
		SFile >> m_sControlCam.fDown;
		SFile >> m_sControlCam.fLeft;
		SFile >> m_sControlCam.fRight;
		SFile >> m_sControlCam.fFar;
		SFile >> m_sControlCam.fNear;
		SFile >> m_sControlCam.vCamPos.x;			
		SFile >> m_sControlCam.vCamPos.y;
		SFile >> m_sControlCam.vCamPos.z;
	}

	m_vecLandEffect.clear();
	// Load Land Effect
	if( m_bUseFunction[EMETCFUNC_LANDEFFECT] )
	{
		int iSize, EffectType;
		SFile >> iSize;
		for( int i = 0; i < iSize; i++ )
		{
			SLANDEFFECT landEffect;
			SFile >> EffectType; landEffect.emLandEffectType = (EMLANDEFFECT_TYPE)EffectType;
			SFile >> landEffect.fValue;
			SFile >> landEffect.vMinPos.x;
			SFile >> landEffect.vMinPos.y;
			SFile >> landEffect.vMaxPos.x;
			SFile >> landEffect.vMaxPos.y;

			D3DXVECTOR2 vMin, vMax;
			vMin.x = min( landEffect.vMinPos.x, landEffect.vMaxPos.x );
			vMin.y = min( landEffect.vMinPos.y, landEffect.vMaxPos.y );
			vMax.x = max( landEffect.vMinPos.x, landEffect.vMaxPos.x );
			vMax.y = max( landEffect.vMinPos.y, landEffect.vMaxPos.y );

			landEffect.vMinPos = vMin;
			landEffect.vMaxPos = vMax;

			m_vecLandEffect.push_back(landEffect);
		}
	}

	dwRead = SFile.GetfTell() - dwRead;
	
	if( dwRead != dwSIZE )
		CDebugSet::MsgBox( "SLEVEL_ETC_FUNC::LOAD_0101 Incorrect Read Size:%d Expected:%d", dwRead, dwSIZE );
}

void SLEVEL_ETC_FUNC::LOAD_0101 ( basestream &SFile, DWORD dwSIZE )
{
	DWORD dwRead = SFile.GetfTell();

	//SFile.ReadBuffer( &m_bUseFunction, sizeof(m_bUseFunction) );
	SFile >> m_bUseFunction[EMETCFUNC_LIMITTIME];
	SFile >> m_bUseFunction[EMETCFUNC_CONTROLCAM];
	SFile >> m_bUseFunction[EMETCFUNC_LANDEFFECT];
	SFile >> m_bUseFunction[EMETCFUNC_DIFFICULTY];
	SFile >> m_bUseFunction[EMETCFUNC_AROUNDREGEN];
	SFile >> m_bUseFunction[EMETCFUNC_MOBCHASE];

	// Load Limit Time
	if( m_bUseFunction[EMETCFUNC_LIMITTIME] ) 
	{
		SFile >> m_sLimitTime.nLimitMinute;
		SFile >> m_sLimitTime.nTellTermMinute;
	}

	// Load Camera Control
	if( m_bUseFunction[EMETCFUNC_CONTROLCAM] ) 
	{
		SFile >> m_sControlCam.fUp;
		SFile >> m_sControlCam.fDown;
		SFile >> m_sControlCam.fLeft;
		SFile >> m_sControlCam.fRight;
		SFile >> m_sControlCam.fFar;
		SFile >> m_sControlCam.fNear;
		SFile >> m_sControlCam.vCamPos.x;			
		SFile >> m_sControlCam.vCamPos.y;
		SFile >> m_sControlCam.vCamPos.z;
	}

	m_vecLandEffect.clear();
	// Load Land Effect
	if( m_bUseFunction[EMETCFUNC_LANDEFFECT] )
	{
		int iSize, EffectType;
		SFile >> iSize;
		for( int i = 0; i < iSize; i++ )
		{
			SLANDEFFECT landEffect;
			SFile >> EffectType; landEffect.emLandEffectType = (EMLANDEFFECT_TYPE)EffectType;
			SFile >> landEffect.fValue;
			SFile >> landEffect.vMinPos.x;
			SFile >> landEffect.vMinPos.y;
			SFile >> landEffect.vMaxPos.x;
			SFile >> landEffect.vMaxPos.y;

			D3DXVECTOR2 vMin, vMax;
			vMin.x = min( landEffect.vMinPos.x, landEffect.vMaxPos.x );
			vMin.y = min( landEffect.vMinPos.y, landEffect.vMaxPos.y );
			vMax.x = max( landEffect.vMinPos.x, landEffect.vMaxPos.x );
			vMax.y = max( landEffect.vMinPos.y, landEffect.vMaxPos.y );

			landEffect.vMinPos = vMin;
			landEffect.vMaxPos = vMax;

			m_vecLandEffect.push_back(landEffect);
		}
	}

	dwRead = SFile.GetfTell() - dwRead;
	
	if( dwRead != dwSIZE )
		CDebugSet::MsgBox( "SLEVEL_ETC_FUNC::LOAD_0101 Incorrect Read Size:%d Expected:%d", dwRead, dwSIZE );
}

void SLEVEL_ETC_FUNC::LOAD_0102 ( basestream &SFile, DWORD dwSIZE )
{
	DWORD dwRead = SFile.GetfTell();

	//SFile.ReadBuffer( &m_bUseFunction, sizeof(m_bUseFunction) );
	bool bUnknown1(false);
	SFile >> m_bUseFunction[EMETCFUNC_LIMITTIME];
	SFile >> m_bUseFunction[EMETCFUNC_CONTROLCAM];
	SFile >> m_bUseFunction[EMETCFUNC_LANDEFFECT];
	SFile >> m_bUseFunction[EMETCFUNC_DIFFICULTY];
	SFile >> m_bUseFunction[EMETCFUNC_AROUNDREGEN];
	SFile >> m_bUseFunction[EMETCFUNC_MOBCHASE];
	SFile >> bUnknown1;

	// Load Limit Time
	if( m_bUseFunction[EMETCFUNC_LIMITTIME] ) 
	{
		SFile >> m_sLimitTime.nLimitMinute;
		SFile >> m_sLimitTime.nTellTermMinute;
	}

	// Load Camera Control
	if( m_bUseFunction[EMETCFUNC_CONTROLCAM] ) 
	{
		SFile >> m_sControlCam.fUp;
		SFile >> m_sControlCam.fDown;
		SFile >> m_sControlCam.fLeft;
		SFile >> m_sControlCam.fRight;
		SFile >> m_sControlCam.fFar;
		SFile >> m_sControlCam.fNear;
		SFile >> m_sControlCam.vCamPos.x;			
		SFile >> m_sControlCam.vCamPos.y;
		SFile >> m_sControlCam.vCamPos.z;
	}

	m_vecLandEffect.clear();
	// Load Land Effect
	if( m_bUseFunction[EMETCFUNC_LANDEFFECT] )
	{
		int iSize, EffectType;
		SFile >> iSize;
		for( int i = 0; i < iSize; i++ )
		{
			SLANDEFFECT landEffect;
			SFile >> EffectType; landEffect.emLandEffectType = (EMLANDEFFECT_TYPE)EffectType;
			SFile >> landEffect.fValue;
			SFile >> landEffect.vMinPos.x;
			SFile >> landEffect.vMinPos.y;
			SFile >> landEffect.vMaxPos.x;
			SFile >> landEffect.vMaxPos.y;

			D3DXVECTOR2 vMin, vMax;
			vMin.x = min( landEffect.vMinPos.x, landEffect.vMaxPos.x );
			vMin.y = min( landEffect.vMinPos.y, landEffect.vMaxPos.y );
			vMax.x = max( landEffect.vMinPos.x, landEffect.vMaxPos.x );
			vMax.y = max( landEffect.vMinPos.y, landEffect.vMaxPos.y );

			landEffect.vMinPos = vMin;
			landEffect.vMaxPos = vMax;

			m_vecLandEffect.push_back(landEffect);
		}
	}

	dwRead = SFile.GetfTell() - dwRead;

	if( dwRead != dwSIZE )
		CDebugSet::MsgBox( "SLEVEL_ETC_FUNC::LOAD_0102 Incorrect Read Size:%d Expected:%d", dwRead, dwSIZE );
}

void SLEVEL_ETC_FUNC::LOAD_0103 ( basestream &SFile, DWORD dwSIZE )
{
	DWORD dwRead = SFile.GetfTell();

	//SFile.ReadBuffer( &m_bUseFunction, sizeof(m_bUseFunction) );
	bool bUnknown1(false);
	SFile >> m_bUseFunction[EMETCFUNC_LIMITTIME];
	SFile >> m_bUseFunction[EMETCFUNC_CONTROLCAM];
	SFile >> m_bUseFunction[EMETCFUNC_LANDEFFECT];
	SFile >> m_bUseFunction[EMETCFUNC_DIFFICULTY];
	SFile >> m_bUseFunction[EMETCFUNC_AROUNDREGEN];
	SFile >> m_bUseFunction[EMETCFUNC_MOBCHASE];
	SFile >> bUnknown1;

	// Load Camera Control
	if( m_bUseFunction[EMETCFUNC_CONTROLCAM] ) 
	{
		SFile >> m_sControlCam.fUp;
		SFile >> m_sControlCam.fDown;
		SFile >> m_sControlCam.fLeft;
		SFile >> m_sControlCam.fRight;
		SFile >> m_sControlCam.fFar;
		SFile >> m_sControlCam.fNear;
		SFile >> m_sControlCam.vCamPos.x;			
		SFile >> m_sControlCam.vCamPos.y;
		SFile >> m_sControlCam.vCamPos.z;
	}

	// Load Limit Time
	if( m_bUseFunction[EMETCFUNC_LIMITTIME] ) 
	{
		SFile >> m_sLimitTime.nLimitMinute;
		SFile >> m_sLimitTime.nTellTermMinute;
	}

	m_vecLandEffect.clear();
	// Load Land Effect
	if( m_bUseFunction[EMETCFUNC_LANDEFFECT] )
	{
		int iSize, EffectType;
		SFile >> iSize;
		for( int i = 0; i < iSize; i++ )
		{
			SLANDEFFECT landEffect;
			SFile >> EffectType; landEffect.emLandEffectType = (EMLANDEFFECT_TYPE)EffectType;
			SFile >> landEffect.fValue;
			SFile >> landEffect.vMinPos.x;
			SFile >> landEffect.vMinPos.y;
			SFile >> landEffect.vMaxPos.x;
			SFile >> landEffect.vMaxPos.y;

			D3DXVECTOR2 vMin, vMax;
			vMin.x = min( landEffect.vMinPos.x, landEffect.vMaxPos.x );
			vMin.y = min( landEffect.vMinPos.y, landEffect.vMaxPos.y );
			vMax.x = max( landEffect.vMinPos.x, landEffect.vMaxPos.x );
			vMax.y = max( landEffect.vMinPos.y, landEffect.vMaxPos.y );

			landEffect.vMinPos = vMin;
			landEffect.vMaxPos = vMax;

			m_vecLandEffect.push_back(landEffect);
		}
	}

	dwRead = SFile.GetfTell() - dwRead;

	if( dwRead != dwSIZE )
		CDebugSet::MsgBox( "SLEVEL_ETC_FUNC::LOAD_0103 Incorrect Read Size:%d Expected:%d", dwRead, dwSIZE );
}

void SLEVEL_ETC_FUNC::LOAD_0104 ( basestream &SFile, DWORD dwSIZE )
{
	DWORD dwRead = SFile.GetfTell();

	//SFile.ReadBuffer( &m_bUseFunction, sizeof(m_bUseFunction) );
	bool bUnknown1(false);
	SFile >> m_bUseFunction[EMETCFUNC_LIMITTIME];
	SFile >> m_bUseFunction[EMETCFUNC_CONTROLCAM];
	SFile >> m_bUseFunction[EMETCFUNC_LANDEFFECT];
	SFile >> m_bUseFunction[EMETCFUNC_DIFFICULTY];
	SFile >> m_bUseFunction[EMETCFUNC_AROUNDREGEN];
	SFile >> m_bUseFunction[EMETCFUNC_MOBCHASE];
	SFile >> bUnknown1;

	// Load Camera Control
	if( m_bUseFunction[EMETCFUNC_CONTROLCAM] ) 
	{
		SFile >> m_sControlCam.fUp;
		SFile >> m_sControlCam.fDown;
		SFile >> m_sControlCam.fLeft;
		SFile >> m_sControlCam.fRight;
		SFile >> m_sControlCam.fFar;
		SFile >> m_sControlCam.fNear;
		SFile >> m_sControlCam.vCamPos.x;			
		SFile >> m_sControlCam.vCamPos.y;
		SFile >> m_sControlCam.vCamPos.z;
	}

	// Load Limit Time
	if( m_bUseFunction[EMETCFUNC_LIMITTIME] ) 
	{
		SFile >> m_sLimitTime.nLimitMinute;
		SFile >> m_sLimitTime.nTellTermMinute;
	}

	m_vecLandEffect.clear();
	// Load Land Effect
	if( m_bUseFunction[EMETCFUNC_LANDEFFECT] )
	{
		int iSize, EffectType;
		SFile >> iSize;
		for( int i = 0; i < iSize; i++ )
		{
			SLANDEFFECT landEffect;
			SFile >> EffectType; landEffect.emLandEffectType = (EMLANDEFFECT_TYPE)EffectType;
			SFile >> landEffect.fValue;
			SFile >> landEffect.vMinPos.x;
			SFile >> landEffect.vMinPos.y;
			SFile >> landEffect.vMaxPos.x;
			SFile >> landEffect.vMaxPos.y;

			//unknown data
			DWORD dwUnknown1(0);
			float fUnknown1(0.0f);
			SFile >> dwUnknown1;
			SFile >> fUnknown1;

			D3DXVECTOR2 vMin, vMax;
			vMin.x = min( landEffect.vMinPos.x, landEffect.vMaxPos.x );
			vMin.y = min( landEffect.vMinPos.y, landEffect.vMaxPos.y );
			vMax.x = max( landEffect.vMinPos.x, landEffect.vMaxPos.x );
			vMax.y = max( landEffect.vMinPos.y, landEffect.vMaxPos.y );

			landEffect.vMinPos = vMin;
			landEffect.vMaxPos = vMax;

			m_vecLandEffect.push_back(landEffect);
		}
	}

	dwRead = SFile.GetfTell() - dwRead;

	if( dwRead != dwSIZE )
		CDebugSet::MsgBox( "SLEVEL_ETC_FUNC::LOAD_0104 Incorrect Read Size:%d Expected:%d", dwRead, dwSIZE );
}

void SLEVEL_ETC_FUNC::LOAD_0105 ( basestream &SFile, DWORD dwSIZE )
{
	DWORD dwRead = SFile.GetfTell();

	//SFile.ReadBuffer( &m_bUseFunction, sizeof(m_bUseFunction) );
	bool bUnknown1(false);
	bool bUnknown2(false);
	SFile >> m_bUseFunction[EMETCFUNC_LIMITTIME];
	SFile >> m_bUseFunction[EMETCFUNC_CONTROLCAM];
	SFile >> m_bUseFunction[EMETCFUNC_LANDEFFECT];
	SFile >> m_bUseFunction[EMETCFUNC_DIFFICULTY];
	SFile >> m_bUseFunction[EMETCFUNC_AROUNDREGEN];
	SFile >> m_bUseFunction[EMETCFUNC_MOBCHASE];
	SFile >> bUnknown1;
	SFile >> bUnknown2;

	// Load Camera Control
	if( m_bUseFunction[EMETCFUNC_CONTROLCAM] ) 
	{
		SFile >> m_sControlCam.fUp;
		SFile >> m_sControlCam.fDown;
		SFile >> m_sControlCam.fLeft;
		SFile >> m_sControlCam.fRight;
		SFile >> m_sControlCam.fFar;
		SFile >> m_sControlCam.fNear;
		SFile >> m_sControlCam.vCamPos.x;			
		SFile >> m_sControlCam.vCamPos.y;
		SFile >> m_sControlCam.vCamPos.z;
	}

	// Load Limit Time
	if( m_bUseFunction[EMETCFUNC_LIMITTIME] ) 
	{
		SFile >> m_sLimitTime.nLimitMinute;
		SFile >> m_sLimitTime.nTellTermMinute;
	}

	m_vecLandEffect.clear();
	// Load Land Effect
	if( m_bUseFunction[EMETCFUNC_LANDEFFECT] )
	{
		int iSize, EffectType;
		SFile >> iSize;
		for( int i = 0; i < iSize; i++ )
		{
			SLANDEFFECT landEffect;
			SFile >> EffectType; landEffect.emLandEffectType = (EMLANDEFFECT_TYPE)EffectType;
			SFile >> landEffect.fValue;
			SFile >> landEffect.vMinPos.x;
			SFile >> landEffect.vMinPos.y;
			SFile >> landEffect.vMaxPos.x;
			SFile >> landEffect.vMaxPos.y;

			//unknown data
			DWORD dwUnknown1(0);
			float fUnknown1(0.0f);
			SFile >> dwUnknown1;
			SFile >> fUnknown1;

			D3DXVECTOR2 vMin, vMax;
			vMin.x = min( landEffect.vMinPos.x, landEffect.vMaxPos.x );
			vMin.y = min( landEffect.vMinPos.y, landEffect.vMaxPos.y );
			vMax.x = max( landEffect.vMinPos.x, landEffect.vMaxPos.x );
			vMax.y = max( landEffect.vMinPos.y, landEffect.vMaxPos.y );

			landEffect.vMinPos = vMin;
			landEffect.vMaxPos = vMax;

			m_vecLandEffect.push_back(landEffect);
		}
	}

	//unknown data
	DWORD dwUnknown2(0);
	if( bUnknown2 ) 
	{
		SFile >> dwUnknown2;
	}

	dwRead = SFile.GetfTell() - dwRead;

	if( dwRead != dwSIZE )
		CDebugSet::MsgBox( "SLEVEL_ETC_FUNC::LOAD_0105 Incorrect Read Size:%d Expected:%d", dwRead, dwSIZE );
}


void SLEVEL_ETC_FUNC::LOAD_0200 ( basestream &SFile, DWORD dwSIZE )
{
	DWORD dwRead = SFile.GetfTell();

	SFile.ReadBuffer( &m_bUseFunction, sizeof(m_bUseFunction) );

	// Load Camera Control
	if( m_bUseFunction[EMETCFUNC_CONTROLCAM] ) 
	{
		SFile >> m_sControlCam.fUp;
		SFile >> m_sControlCam.fDown;
		SFile >> m_sControlCam.fLeft;
		SFile >> m_sControlCam.fRight;
		SFile >> m_sControlCam.fFar;
		SFile >> m_sControlCam.fNear;
		SFile >> m_sControlCam.vCamPos.x;			
		SFile >> m_sControlCam.vCamPos.y;
		SFile >> m_sControlCam.vCamPos.z;
	}

	// Load Limit Time
	if( m_bUseFunction[EMETCFUNC_LIMITTIME] ) 
	{
		SFile >> m_sLimitTime.nLimitMinute;
		SFile >> m_sLimitTime.nTellTermMinute;
	}

	m_vecLandEffect.clear();
	// Load Land Effect
	if( m_bUseFunction[EMETCFUNC_LANDEFFECT] )
	{
		int iSize, EffectType;
		SFile >> iSize;
		for( int i = 0; i < iSize; i++ )
		{
			SLANDEFFECT landEffect;
			SFile >> EffectType; landEffect.emLandEffectType = (EMLANDEFFECT_TYPE)EffectType;
			SFile >> landEffect.fValue;
			SFile >> landEffect.vMinPos.x;
			SFile >> landEffect.vMinPos.y;
			SFile >> landEffect.vMaxPos.x;
			SFile >> landEffect.vMaxPos.y;

			D3DXVECTOR2 vMin, vMax;
			vMin.x = min( landEffect.vMinPos.x, landEffect.vMaxPos.x );
			vMin.y = min( landEffect.vMinPos.y, landEffect.vMaxPos.y );
			vMax.x = max( landEffect.vMinPos.x, landEffect.vMaxPos.x );
			vMax.y = max( landEffect.vMinPos.y, landEffect.vMaxPos.y );

			landEffect.vMinPos = vMin;
			landEffect.vMaxPos = vMax;

			m_vecLandEffect.push_back(landEffect);
		}
	}

	dwRead = SFile.GetfTell() - dwRead;

	if( dwRead != dwSIZE )
		CDebugSet::MsgBox( "SLEVEL_ETC_FUNC::LOAD_0200 Incorrect Read Size:%d Expected:%d", dwRead, dwSIZE );
}

BOOL SLEVEL_ETC_FUNC::SAVE ( CSerialFile &SFile )
{
	SFile << DWORD(VERSION);
	SFile.BeginBlock();
	{
		SFile.WriteBuffer( &m_bUseFunction, sizeof(m_bUseFunction) );

		// Save Camera Control
		if( m_bUseFunction[EMETCFUNC_CONTROLCAM] ) 
		{
			SFile << m_sControlCam.fUp;
			SFile << m_sControlCam.fDown;
			SFile << m_sControlCam.fLeft;
			SFile << m_sControlCam.fRight;
			SFile << m_sControlCam.fFar;
			SFile << m_sControlCam.fNear;
			SFile << m_sControlCam.vCamPos.x;			
			SFile << m_sControlCam.vCamPos.y;
			SFile << m_sControlCam.vCamPos.z;
		}

		// Save Limit Time
		if( m_bUseFunction[EMETCFUNC_LIMITTIME] ) 
		{
			SFile << m_sLimitTime.nLimitMinute;
			SFile << m_sLimitTime.nTellTermMinute;
		}

		// Save Land Effect
		if( m_bUseFunction[EMETCFUNC_LANDEFFECT] )
		{
			SFile << m_vecLandEffect.size();
			for( int i = 0; i < (int)m_vecLandEffect.size(); i++ )
			{
				SLANDEFFECT landEffect = m_vecLandEffect[i];
				SFile << (int)landEffect.emLandEffectType;
				SFile << landEffect.fValue;
				SFile << landEffect.vMinPos.x;
				SFile << landEffect.vMinPos.y;
				SFile << landEffect.vMaxPos.x;
				SFile << landEffect.vMaxPos.y;
			}
		}
	}
	SFile.EndBlock();

	return TRUE;
}