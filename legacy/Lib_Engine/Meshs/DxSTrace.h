/*!
 * \file DxSTrace.h
 *
 * \author Juver
 * \date December 2016
 *
 * 
 */

#pragma once

#include"./DxSkinMeshContainer9.h"

#define STRACE_NSIZE	(32)
#define STRACE_IMAGE01 ("IMAGE01")		// 0
#define STRACE_IMAGE02 ("IMAGE02")
#define STRACE_IMAGE11 ("IMAGE11")
#define STRACE_IMAGE12 ("IMAGE12")
#define STRACE_IMAGE21 ("IMAGE21")
#define STRACE_IMAGE22 ("IMAGE22")		//	5

#define STRACE_STRIKE01 ("STRIKE01")	//	6
#define STRACE_STRIKE02 ("STRIKE02")
#define STRACE_STRIKE11 ("STRIKE11")
#define STRACE_STRIKE12 ("STRIKE12")	//	9

#define STRACE_ARROW01 ("ARROW01")		//	10

#define STRACE_RAIN01 ("RAIN01")		//	11
#define STRACE_RAIN02 ("RAIN02")
#define STRACE_RAIN03 ("RAIN03")		//	13

#define STRACE_BLOOD01 ("BLOOD01")		//	14

#define STRACE_ENERGY01 ("ENERGY01")	//	15
#define STRACE_ENERGY02 ("ENERGY02")
#define STRACE_ENERGY03 ("ENERGY03")	//	17

#define MAX_TRACE	(18)
extern char szTRACE_NAME[MAX_TRACE][STRACE_NSIZE];

struct STRACOR
{
	char			m_szName[STRACE_NSIZE];
	SVERTEXINFLU	m_sVertexInflu;
	D3DXVECTOR3		m_vPos;
	D3DXVECTOR3		m_vNormal;
	D3DXMATRIX		m_BoneMatrice;

	STRACOR ();
	STRACOR ( const char* _szName, const SVERTEXINFLU &_sVertexInflu, D3DXVECTOR3 _vPos );
	STRACOR ( const STRACOR& sTracor );

	bool operator== ( const STRACOR& lvalue );
};
typedef std::vector<STRACOR>	VECTRACE;
typedef VECTRACE::iterator		VECTRACE_ITER;

struct STRACOR_OPER
{
	bool operator() ( const STRACOR& lvalue, const STRACOR& rvalue )
	{
		return strcmp(lvalue.m_szName,rvalue.m_szName)<0;
	}

	bool operator() ( const STRACOR& lvalue, const char* szName )
	{
		return strcmp(lvalue.m_szName,szName)<0;
	}
};
