#include "stdafx.h"
#include "DxSTrace.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern char szTRACE_NAME[MAX_TRACE][STRACE_NSIZE] = 
{
	STRACE_IMAGE01,
	STRACE_IMAGE02,
	STRACE_IMAGE11,
	STRACE_IMAGE12,
	STRACE_IMAGE21,
	STRACE_IMAGE22,

	STRACE_STRIKE01,
	STRACE_STRIKE02,
	STRACE_STRIKE11,
	STRACE_STRIKE12,

	STRACE_ARROW01,

	STRACE_RAIN01,
	STRACE_RAIN02,
	STRACE_RAIN03,

	STRACE_BLOOD01,

	STRACE_ENERGY01,
	STRACE_ENERGY02,
	STRACE_ENERGY03,
};

#define D3DFVF_MESHVERTEX (D3DFVF_XYZ | D3DFVF_TEX1)

STRACOR::STRACOR () :
	m_vPos(0,0,0),
	m_vNormal(1,0,0)
{
	memset( m_szName, 0, sizeof(char)*STRACE_NSIZE );
}

STRACOR::STRACOR ( const char* _szName, const SVERTEXINFLU &_sVertexInflu, D3DXVECTOR3 _vPos ) :
	m_vPos(_vPos),
	m_vNormal(1,0,0)
{
	StringCchCopy ( m_szName, STRACE_NSIZE, _szName );
	m_sVertexInflu = _sVertexInflu;
}

STRACOR::STRACOR ( const STRACOR& _sTracor ) :
	m_vNormal(1,0,0)
{
	StringCchCopy ( m_szName, STRACE_NSIZE, _sTracor.m_szName );
	m_vPos = _sTracor.m_vPos;
	m_sVertexInflu = _sTracor.m_sVertexInflu;
}

bool STRACOR::operator== ( const STRACOR& lvalue )
{
	return !strcmp(m_szName,lvalue.m_szName);
}