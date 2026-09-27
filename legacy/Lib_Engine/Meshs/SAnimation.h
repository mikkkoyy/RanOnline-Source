
#pragma once

#include <GASSERT.h>

#include <map>
#include <list>
#include <string>
#include <vector>

#include "../Common/SerialFile.h"
#include "./DxBoneCollector.h"

#include "./SAnimationInfo.h"

//struct DxBoneTrans;
//class DxEffAniData;
//class DxAniScale;

struct SAnimation
{
	enum { VERSION = 0x0103, };

	// animation information
	SPositionKey	*m_pPositionKeys;	//	이동 에니키.
	UINT			m_cPositionKeys;
	
	SRotateKey		*m_pRotateKeys;		//	회전 에니키.
	UINT			m_cRotateKeys;
	
	SScaleKey		*m_pScaleKeys;		//	크기 에니키.
	UINT			m_cScaleKeys;
	
	SMatrixKey		*m_pMatrixKeys;		// Matrix
	UINT			m_cMatrixKeys;

	SQuatPosKey*	m_pQuatPosKeys;		// Quaternion 테스트 할 것이다..  완성되면 Matrix는 사용 안 할 것임.
	UINT			m_cQuatPosKeys;
	D3DXQUATERNION*	m_pQuatOrigRot;		// 순수한 회전.. 아주 원조이다. MIX 시만 사용한다. ~!!

	TSTRING		m_strBone;
	DxBoneTrans		*pBoneToAnimate;	//	적용될 BONE 과 연결.

	DWORD			m_dwCurKey;
	float			m_fDivFullTime;		// Opitimize - 원래는 이 값 1.f/m_pMatrixKeys[m_cMatrixKeys-1].fTime
	float			m_fMatrixKeys;		// Opitimize - 원래는 이 값 m_cMatrixKeys

	SAnimation() :
			m_pPositionKeys(NULL),
			m_cPositionKeys(0),
			m_pScaleKeys(NULL),
			m_cScaleKeys(0),
			m_pRotateKeys(NULL),
			m_cRotateKeys(0),
			m_pMatrixKeys(NULL),
			m_cMatrixKeys(0),
			m_pQuatPosKeys(NULL),
			m_cQuatPosKeys(0),
			m_pQuatOrigRot(NULL),

			pBoneToAnimate(NULL),
			m_dwCurKey(0),
			m_fDivFullTime(0.f),
			m_fMatrixKeys(0.f)
	{
	}

	~SAnimation()
	{
		SAFE_DELETE_ARRAY(m_pPositionKeys);
		SAFE_DELETE_ARRAY(m_pRotateKeys);
		SAFE_DELETE_ARRAY(m_pScaleKeys);
		SAFE_DELETE_ARRAY(m_pMatrixKeys);
		SAFE_DELETE_ARRAY(m_pQuatPosKeys);
		SAFE_DELETE_ARRAY(m_pQuatOrigRot);

		// do NOT delete pToAnimate
	}

	void SetTime ( float fTime, float fWeight, BOOL bFirst );
	void SetMix ( float fTime );

	UINT GetThisKey ( float fTime ) const;
	const SMatrixKey* GetMapThisKey ( float fGlobalTime ) const;

	void SetMatrix ( UINT iKey );
	void SetMatrix ( const SMatrixKey* pMatrixKey )	{ pBoneToAnimate->matRot = pMatrixKey->mat; }

	void ConvertMatToQuat();

	HRESULT LoadFromFile ( CSerialFile &SFile, const DxSkeleton* pSkeleton );
	HRESULT Load_0100( CSerialFile &SFile, const DxSkeleton* pSkeleton );
	HRESULT Load_0101( CSerialFile &SFile, const DxSkeleton* pSkeleton );
	HRESULT Load_0102( CSerialFile &SFile, const DxSkeleton* pSkeleton );
	HRESULT Load_0103( CSerialFile &SFile, const DxSkeleton* pSkeleton );
	HRESULT SaveToFile ( CSerialFile &SFile );
};



// 한 프레임에서 각각의 본의 애니메이션 정보
struct SAnimContainer : public SANIMCONINFO
{
public:
	enum 
	{ 
		VERSION			= 0x0200, 
		VERSION_ENCRYPT = 0x0200, 
	};

	const static TCHAR szFILETYPE[];

	typedef std::list<SAnimation*>		SANILIST;
	typedef SANILIST::iterator			SANILIST_ITER;

protected:
	DWORD					m_dwRefCount;

	bool					m_bLOAD;
	bool					m_bVALID;
	SANILIST				m_listAnimation;	// 하체 관련 애니메이션 값.
	SANILIST				m_listAniUPBODY;	// 상체 관련 애니메이션 값.	// Ver.101에서 들어간 값.

public:
	DxSkeleton*				m_pSkeleton;		// 본 포인터.

public:
	DWORD AddRef()		{ return m_dwRefCount++; }
	DWORD Release()		{ if (m_dwRefCount!=0) m_dwRefCount--; return m_dwRefCount; }

protected:
	HRESULT LoadAnimationSet ( LPDIRECTXFILEDATA pxofobjCur );
	HRESULT LoadAnimation ( LPDIRECTXFILEDATA pxofobjCur );

public:
	HRESULT CreateAnimationData ( SANIMCONINFO &AnimInfo, LPDIRECT3DDEVICEQ pd3dDevice, bool bREPRESH=false );
	BOOL	IsValidateANI();

public:
	HRESULT SaveToFile ();

public:
	//	기본 정보 설정.
	HRESULT SetInfo ( SANIMCONINFO &sAnimInfo, LPDIRECT3DDEVICEQ pd3dDevice );

	//	파일에서 읽기.
	HRESULT LoadAnimFromFile ( const TSTRING &strFilePath );
	void SetAnimFromList ( SANILIST &listANI, SANILIST &listANIUPBODY );

	//	파일에서 읽기. ( 스레드용. )
	static HRESULT LoadAnimFromFile ( const TSTRING &strFilePath, const DxSkeleton* pSkeleton, SANILIST &listANI, SANILIST &listANIUPBODY );

	HRESULT  Load_0100( CSerialFile &SFile );
	HRESULT  Load_0101( CSerialFile &SFile );
	HRESULT  Load_0102( CSerialFile &SFile );
	HRESULT  Load_0103( CSerialFile &SFile );
	HRESULT  Load_0104( CSerialFile &SFile );
	HRESULT  Load_0200( CSerialFile &SFile );

	//	쓰레드용 
	static HRESULT  Load_0100( CSerialFile &SFile, const DxSkeleton* pSkeleton, SANILIST &listANI, SANILIST &listANIUPBODY );
	static HRESULT  Load_0101( CSerialFile &SFile, const DxSkeleton* pSkeleton, SANILIST &listANI, SANILIST &listANIUPBODY );
	static HRESULT  Load_0102( CSerialFile &SFile, const DxSkeleton* pSkeleton, SANILIST &listANI, SANILIST &listANIUPBODY );
	static HRESULT  Load_0103( CSerialFile &SFile, const DxSkeleton* pSkeleton, SANILIST &listANI, SANILIST &listANIUPBODY );
	static HRESULT  Load_0104( CSerialFile &SFile, const DxSkeleton* pSkeleton, SANILIST &listANI, SANILIST &listANIUPBODY );
	static HRESULT  Load_0200( CSerialFile &SFile, const DxSkeleton* pSkeleton, SANILIST &listANI, SANILIST &listANIUPBODY );

public:
	bool UpdateTime ( float fCurTime, float fWeight=1.f, BOOL bFirst=TRUE, EMBODY emBody=EMBODY_DEFAULT );
	bool UpdateMix ( float fCurMixTime );
	void UpdateBoneScale( float fWeight, BOOL bFirst, float fTime );

public:
	SAnimContainer();
	virtual ~SAnimContainer();
};
typedef SAnimContainer* PSANIMCONTAINER;
