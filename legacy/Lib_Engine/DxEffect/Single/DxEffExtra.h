#pragma once

struct EFF_PROPERTY;
struct EFF_PROPGROUP;
class DxEffSingleGroup;

#include <list>
#include <map>
#include "GLDefine.h"

class DxEffExtra
{
protected:
	typedef std::multimap<STARGETID,DxEffSingleGroup*>		EFFSGMAP;
	typedef EFFSGMAP::iterator								EFFSGMAP_ITER;	
	typedef std::pair<EFFSGMAP_ITER,EFFSGMAP_ITER>			EFFSGMAP_PAIR;

protected:
	LPDIRECT3DDEVICEQ	m_pd3dDevice;
	float				m_fTimer;
	EFFSGMAP			m_mapESG;

	bool				bEffClickCreated;
	DxEffSingleGroup*	m_pEffClick;

	bool				bEffTargetCreated;
	DxEffSingleGroup*	m_pEffTarget;

	bool				bEffLocatorACreated;
	bool				bEffLocatorBCreated;
	DxEffSingleGroup*	m_pEffLocatorA;
	DxEffSingleGroup*	m_pEffLocatorB;

protected:
	void	NewAfterEff ( DxEffSingleGroup* pEffSG );

public:
	DWORD	GetAmount ()				{ return DWORD(m_mapESG.size()); }
	void	SetTime ( float fTimer )	{ m_fTimer = fTimer; }
	float	GetTime ()					{ return m_fTimer; }

public:
	DxEffSingleGroup* NewEff ( const char* szFileName, const D3DXMATRIX &matTrans, const STARGETID* pTargetID=NULL );
	DxEffSingleGroup* NewEffGroup ( EFF_PROPGROUP* pPropGroup, const D3DXMATRIX &matTrans, const STARGETID* pTargetID=NULL );
	HRESULT RemoveAllEff ();
	HRESULT DeleteEff ( const char* szFile );
	HRESULT DeleteAllEff ( const char* szFile );

	DxEffSingleGroup* NewClickEff ( const char* szFileName, const D3DXMATRIX &matTrans, const STARGETID* pTargetID=NULL );
	void	RestartClickEffect();
	void	RemoveClickEffect( const char* szFile );

	DxEffSingleGroup* NewTargetEff ( const char* szFileName, const D3DXMATRIX &matTrans, const STARGETID* pTargetID=NULL );
	void	RemoveTargetEffect( const char* szFile );

	DxEffSingleGroup* NewLocatorAEff ( const char* szFileName, const D3DXMATRIX &matTrans, const STARGETID* pTargetID=NULL );
	DxEffSingleGroup* NewLocatorBEff ( const char* szFileName, const D3DXMATRIX &matTrans, const STARGETID* pTargetID=NULL );
	void	RemoveLocatorAEffect( const char* szFile );
	void	RemoveLocatorBEffect( const char* szFile );

	HRESULT PassiveEffect ( const char* szFileName, const D3DXMATRIX &matTrans, const STARGETID &sTargetID );
	HRESULT DeletePassiveEffect ( const char* szFileName, const STARGETID &sTargetID );
	HRESULT DeletePassiveEffect ( const STARGETID &sTargetID );
	BOOL FindPassiveEffect ( const char* szFileName, const STARGETID &sTargetID );

	void	RemoveMineEffect( const char* szFile );

public:
	HRESULT FrameMove ( float fTime, float fElapsedTime );
	HRESULT	Render ( LPDIRECT3DDEVICEQ pd3dDevice );
	HRESULT InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );
	HRESULT RestoreDeviceObjects ();
	HRESULT InvalidateDeviceObjects ();
	HRESULT DeleteDeviceObjects ();

protected:
	DxEffExtra (void);

public:
	~DxEffExtra (void);

public:
	static DxEffExtra& GetInstance();
};
