
#pragma once

#include "./DxAniKeys.h"
#include "./CharacterSound.h"
#include "./DxPieceDefine.h"
#include "./DxAniBoneScale.h"
#include "../DxEffect/EffAni/DxEffAni.h"

enum EMANICONINFO
{
	ACF_NEWINFO		= 0x0001,
	ACF_LOOP		= 0x0010,
	ACF_UPBODY		= 0x0100,	// UPBODY 셋팅.		// UP DOWN 이 셋팅 안될 경우는 디폴트로 전체다.
	ACF_DOWNBODY	= 0x0200,	// UP이 없고 DOWN만 셋팅 될 경우 UP은 DOWN에 영향을 받아야 한다.
	ACF_SZNAME		= 128,
	ACF_DIV			= 8,
	ACF_STRIKE		= 9,
};

struct SANIMCONINFO_101
{
	TCHAR			m_szName[ACF_SZNAME];		//	이름.
	TCHAR			m_szSkeletion[ACF_SZNAME];	//	뼈다귀 이름.

	DWORD			m_dwFlag;					//	기타 속성.
	DWORD			m_dwSTime;					//	시작 시간.
	DWORD			m_dwETime, m_dwETimeOrig;	//	끝 시간.
	DWORD			m_UNITTIME;					//	키단위 시간.

	EMANI_MAINTYPE	m_MainType;
	EMANI_SUBTYPE	m_SubType;

	WORD			m_wDivCount;				//	에니메이션 분할 키 갯수.
	WORD			m_wDivFrame[ACF_DIV];		//	에니메이션 분할 키들..

	SANIMCONINFO_101()
	{
		memset( m_szName, 0, sizeof(TCHAR)*ACF_SZNAME );
		memset( m_szSkeletion, 0, sizeof(TCHAR)*ACF_SZNAME );
	}
};

struct SANIMCONINFO_102
{
	TCHAR			m_szName[ACF_SZNAME];		//	이름.
	TCHAR			m_szSkeletion[ACF_SZNAME];	//	뼈다귀 이름.

	DWORD			m_dwFlag;					//	기타 속성.
	DWORD			m_dwSTime;					//	시작 시간.
	DWORD			m_dwETime, m_dwETimeOrig;	//	끝 시간.
	DWORD			m_UNITTIME;					//	키단위 시간.

	EMANI_MAINTYPE	m_MainType;
	EMANI_SUBTYPE	m_SubType;

	WORD			m_wDivCount;				//	에니메이션 분할 키 갯수.
	WORD			m_wDivFrame[ACF_DIV];		//	에니메이션 분할 키들..

	SChaSoundData_102	m_ChaSoundData;				//	사운드 데이타

	SANIMCONINFO_102()
	{
		memset( m_szName, 0, sizeof(TCHAR)*ACF_SZNAME );
		memset( m_szSkeletion, 0, sizeof(TCHAR)*ACF_SZNAME );
	}
};

struct SANIMCONINFO_103
{
	TCHAR			m_szName[ACF_SZNAME];
	TCHAR			m_szSkeletion[ACF_SZNAME];

	DWORD			m_dwFlag;
	DWORD			m_dwSTime;
	DWORD			m_dwETime, m_dwETimeOrig;
	DWORD			m_UNITTIME;

	EMANI_MAINTYPE	m_MainType;
	EMANI_SUBTYPE	m_SubType;

	WORD			m_wDivCount;
	WORD			m_wDivFrame[ACF_DIV];

	SChaSoundData_103	m_ChaSoundData;

	SANIMCONINFO_103()
	{
		memset( m_szName, 0, sizeof(TCHAR)*ACF_SZNAME );
		memset( m_szSkeletion, 0, sizeof(TCHAR)*ACF_SZNAME );
	}
};

enum EMSTRIKE_EFF
{
	EMSF_NULL		= 0,
	EMSF_SELFBODY	= 1,
	EMSF_TARGET		= 2,

	EMSF_SIZE		= 3
};

struct SANIMSTRIKE
{
	EMPIECECHAR		m_emPiece;
	EMSTRIKE_EFF	m_emEffect;
	DWORD			m_dwFrame;

	SANIMSTRIKE () :
	m_emPiece(PIECE_RHAND),
		m_emEffect(EMSF_TARGET),
		m_dwFrame(0)
	{
	}
};

struct SANIMCONINFO_104
{
	TCHAR			m_szName[ACF_SZNAME];		//	이름.
	TCHAR			m_szSkeletion[ACF_SZNAME];	//	뼈다귀 이름.

	DWORD			m_dwFlag;					//	기타 속성.
	DWORD			m_dwSTime;					//	시작 시간.
	DWORD			m_dwETime, m_dwETimeOrig;	//	끝 시간.
	DWORD			m_UNITTIME;					//	키단위 시간.

	EMANI_MAINTYPE	m_MainType;
	EMANI_SUBTYPE	m_SubType;

	WORD			m_wDivCount;				//	에니메이션 분할 키 갯수.
	WORD			m_wDivFrame[ACF_DIV];		//	에니메이션 분할 키들.

	WORD			m_wStrikeCount;
	SANIMSTRIKE		m_sStrikeEff[ACF_STRIKE];	//	타격 이팩트 시점들.

	SChaSoundData_103	m_ChaSoundData;				//	사운드 데이타

	SANIMCONINFO_104()
	{
		memset( m_szName, 0, sizeof(TCHAR)*ACF_SZNAME );
		memset( m_szSkeletion, 0, sizeof(TCHAR)*ACF_SZNAME );
	}
};

struct SANIMCONINFO_105
{
	typedef std::list<DxEffAniData*>				EFFANILIST;			
	typedef std::list<DxEffAniData*>::iterator		EFFANILIST_ITER;

	TCHAR			m_szName[ACF_SZNAME];		//	이름.
	TCHAR			m_szSkeletion[ACF_SZNAME];	//	뼈다귀 이름.

	DWORD			m_dwFlag;					//	기타 속성.
	DWORD			m_dwSTime;					//	시작 시간.
	DWORD			m_dwETime, m_dwETimeOrig;	//	끝 시간.
	DWORD			m_UNITTIME;					//	키단위 시간.

	EMANI_MAINTYPE	m_MainType;
	EMANI_SUBTYPE	m_SubType;

	WORD			m_wDivCount;				//	에니메이션 분할 키 갯수.
	WORD			m_wDivFrame[ACF_DIV];		//	에니메이션 분할 키들.

	WORD			m_wStrikeCount;
	SANIMSTRIKE		m_sStrikeEff[ACF_STRIKE];	//	타격 이팩트 시점들.

	SChaSoundData_103	m_ChaSoundData;				//	사운드 데이타

	EFFANILIST		m_listEffAni;				// 모션 효과 !

	SANIMCONINFO_105()
	{
		memset( m_szName, 0, sizeof(TCHAR)*ACF_SZNAME );
		memset( m_szSkeletion, 0, sizeof(TCHAR)*ACF_SZNAME );
	}
};

struct SANIMCONINFO_106
{
	typedef std::list<DxEffAniData*>				EFFANILIST;			
	typedef std::list<DxEffAniData*>::iterator		EFFANILIST_ITER;

	TCHAR			m_szName[ACF_SZNAME];		//	이름.
	TCHAR			m_szSkeletion[ACF_SZNAME];	//	뼈다귀 이름.

	DWORD			m_dwFlag;					//	기타 속성.
	DWORD			m_dwSTime;					//	시작 시간.
	DWORD			m_dwETime, m_dwETimeOrig;	//	끝 시간.
	DWORD			m_UNITTIME;					//	키단위 시간.

	EMANI_MAINTYPE	m_MainType;
	EMANI_SUBTYPE	m_SubType;

	WORD			m_wDivCount;				//	에니메이션 분할 키 갯수.
	WORD			m_wDivFrame[ACF_DIV];		//	에니메이션 분할 키들.

	WORD			m_wStrikeCount;
	SANIMSTRIKE		m_sStrikeEff[ACF_STRIKE];	//	타격 이팩트 시점들.

	SChaSoundData	m_ChaSoundData;				//	사운드 데이타

	EFFANILIST		m_listEffAni;				// 모션 효과 !

	SANIMCONINFO_106 () :
		m_dwFlag(ACF_LOOP|ACF_NEWINFO),
		m_dwSTime(0),
		m_dwETime(0),
		m_dwETimeOrig(0),
		m_UNITTIME(0),
		m_MainType(AN_GUARD_N),
		m_SubType(AN_SUB_NONE),

		m_wDivCount(0),
		m_wStrikeCount(0)
	{
		memset( m_szName, 0, sizeof(TCHAR)*ACF_SZNAME );
		memset( m_szSkeletion, 0, sizeof(TCHAR)*ACF_SZNAME );
		memset ( m_wDivFrame, 0, sizeof(WORD)*ACF_DIV );
	}
};

struct SANIMCONINFO	
{
	enum 
	{ 
		VERSION			= 0x0200,
		VERSION_ENCODE	= 0x0200, 
	};

	typedef std::list<DxEffAniData*>				EFFANILIST;			
	typedef std::list<DxEffAniData*>::iterator		EFFANILIST_ITER;

	TCHAR			m_szName[ACF_SZNAME];		//	이름.
	TCHAR			m_szSkeletion[ACF_SZNAME];	//	뼈다귀 이름.

	DWORD			m_dwFlag;					//	기타 속성.
	DWORD			m_dwSTime;					//	시작 시간.
	DWORD			m_dwETime, m_dwETimeOrig;	//	끝 시간.
	DWORD			m_UNITTIME;					//	키단위 시간.

	EMANI_MAINTYPE	m_MainType;
	EMANI_SUBTYPE	m_SubType;

	WORD			m_wDivCount;				//	에니메이션 분할 키 갯수.
	WORD			m_wDivFrame[ACF_DIV];		//	에니메이션 분할 키들.

	WORD			m_wStrikeCount;
	SANIMSTRIKE		m_sStrikeEff[ACF_STRIKE];	//	타격 이팩트 시점들.

	SChaSoundData	m_ChaSoundData;				//	사운드 데이타

	EFFANILIST		m_listEffAni;				// 모션 효과 !
	DxAniScale*		m_pAniScale;				// 애니메이션 스케일.~!

	SANIMCONINFO();
	~SANIMCONINFO();
	void CleanUp();

	BOOL LoadFile ( const TCHAR *szFileName, LPDIRECT3DDEVICEQ pd3dDevice=NULL );
	void LoadFile_0101 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	void LoadFile_0102 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	void LoadFile_0103 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	void LoadFile_0104 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	void LoadFile_0105 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	void LoadFile_0106 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	void LoadFile_0107 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	
	//official version partial read only
	void LoadFile_0108_0109_0110 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	void LoadFile_0111_0112_0114_0115 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	
	void LoadFile_0200 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );

	BOOL SaveFile ( const TCHAR *szFileName );

	SANIMCONINFO& operator = ( SANIMCONINFO &value );
};