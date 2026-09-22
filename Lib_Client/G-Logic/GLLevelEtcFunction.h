#ifndef GLLEVEL_ETCFUNC_H
#define GLLEVEL_ETCFUNC_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"

#include "./GLCharData.h"

struct SLIMITTIME
{
	int nLimitMinute;
	int nTellTermMinute;

	SLIMITTIME()
		: nLimitMinute(0)
		, nTellTermMinute(0)
	{
	}

	void Init()
	{
		nLimitMinute	= 0;
		nTellTermMinute = 0;
	}
};

struct SCONTROLCAM
{
	float		fUp;
	float		fDown;
	float		fLeft;
	float		fRight;
	float		fFar;
	float		fNear;
	D3DXVECTOR3 vCamPos;

	SCONTROLCAM()
		: fUp(0.0f)
		, fDown(0.0f)
		, fLeft(0.0f)
		, fRight(0.0f)
		, fNear(0.0f)
		, fFar(0.0f)
		, vCamPos( 0.0f, 0.0f, 0.0f )
	{
	}

	void Init()
	{
		vCamPos	   = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		fUp		   = 0.0f;
		fDown	   = 0.0f;
		fLeft	   = 0.0f;
		fRight	   = 0.0f;
		fFar	   = 0.0f;
		fNear	   = 0.0f;
	}
};

struct SLEVEL_ETC_FUNC
{
	enum { VERSION = 0x0200, };

	bool		m_bUseFunction[EMETCFUNC_SIZE];
	SLIMITTIME  m_sLimitTime;
	SCONTROLCAM m_sControlCam;
	VEC_LANDEFF m_vecLandEffect;

	SLEVEL_ETC_FUNC()
		/*:*/
	{
		ZeroMemory( m_bUseFunction, sizeof(m_bUseFunction) );
		m_vecLandEffect.clear();
	}

	SLANDEFFECT FindLandEffect( DWORD dwNum )
	{
		SLANDEFFECT landEffect;
		if( dwNum >= m_vecLandEffect.size() ) return landEffect;

		landEffect = m_vecLandEffect[dwNum];

		return landEffect;
	};

	BOOL SAVE ( CSerialFile &SFile );
	BOOL LOAD ( basestream &SFile );

	void LOAD_0100 ( basestream &SFile, DWORD dwSIZE );
	void LOAD_0101 ( basestream &SFile, DWORD dwSIZE );
	void LOAD_0102 ( basestream &SFile, DWORD dwSIZE );
	void LOAD_0103 ( basestream &SFile, DWORD dwSIZE );
	void LOAD_0104 ( basestream &SFile, DWORD dwSIZE );
	void LOAD_0105 ( basestream &SFile, DWORD dwSIZE );
	void LOAD_0200 ( basestream &SFile, DWORD dwSIZE );
};

#endif // GLLEVEL_ETCFUNC_H