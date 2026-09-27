//
//	[class DxSkinCharData], (2002.12.09), JDH.
//
//
#pragma once

#include <map>
#include <string>
#include <vector>

#include "DxSkinAniMan.h"
#include "DxSkinPieceContainer.h"

/*ABL system, Juver, 2017/06/01 */
#include "DxAttBoneData.h"
#include "DxAttBoneLink.h"

/*vehicle system, Juver, 2017/08/03 */
#include "DxVehicle.h"
#include "DxVehicleData.h"


#include "basestream.h"

#define FG_BONEFILE_CHECK	(0x0001)
#define FG_MUSTNEWLOAD		(0x0002)

struct SBONESCALE_100
{
	enum{ BONESCALE_LEN = 16 };

	D3DXVECTOR3		vScale;
	DWORD			dwVal1;
	char			szBone[BONESCALE_LEN];
	DWORD			dwVal2;
	DWORD			dwVal3;

	SBONESCALE_100()
		: vScale( 0.0f, 0.0f, 0.0f )
		, dwVal1( 0 )
		, dwVal2( 0 )
		, dwVal3( 0 )
	{
		memset( szBone, 0, sizeof(char)*BONESCALE_LEN );
	}
};

struct SBONESCALE
{
	D3DXVECTOR3		vScale;
	std::string		strBONE;

	SBONESCALE()
		: vScale( 0.0f, 0.0f, 0.0f )
	{
		strBONE = "";
	}

	void Assign( SBONESCALE_100 sBone )
	{
		vScale = sBone.vScale;
		strBONE = sBone.szBone;
	};
};

class DxSkinCharData : public DxSkinAniControl
{
public:
	enum 
	{ 
		VERSION_CHF		= 0x0200,
		VERSION_ENCRYPT	= 0x0200,
	};

	typedef std::vector<std::string>	VECANIFILE;
	typedef VECANIFILE::iterator		VECANIFILE_ITER;

private:
	enum		{ FILE_LENGTH = 64 };
	char		m_szFileName[FILE_LENGTH];

	std::string			m_strPIECE[PIECE_SIZE];
	VECANIFILE			m_vecANIFILE;

	PDXSKINPIECE		m_pPieceArray[PIECE_SIZE];

	DxAttBoneLink*		m_pAttBoneData[PIECE_SIZE];/*ABL system, Juver, 2017/06/01 */

	DxVehicleData*		m_pVehicle; /*vehicle system, Juver, 2017/08/03 */
public:
	DWORD				m_dwRef;

	BOOL				m_bWorldObj;
	float				m_fScale;
	D3DXVECTOR3			m_vMax, m_vMin;

	float				m_fHeight;
	float				m_fRadius;

public:
	const char* GetFileName()		{ return m_szFileName; }

protected:
	BOOL LOAD_0100( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	BOOL LOAD_0101_0102( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	BOOL LOAD_0103( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	BOOL LOAD_0104( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	BOOL LOAD_0106( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	BOOL LOAD_0107_0108( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );
	BOOL LOAD_0200( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice );

public:
	BOOL LoadFile ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread, bool bREPRESH=false  );
	BOOL SaveFile ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice );

protected:
	 HRESULT CalculateBoundingBox ( LPDIRECT3DDEVICEQ pd3dDevice, D3DXVECTOR3 &vMax, D3DXVECTOR3 &vMin );

public:
	HRESULT SetPiece ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread, DWORD dwFlag = 0x0 );
	void ResetPiece ( LPDIRECT3DDEVICEQ pd3dDevice, int i );
	PDXSKINPIECE GetPiece ( int i )		{ return m_pPieceArray[i]; }
	void SetRadiusHeight();
	void RenderAABBBox( LPDIRECT3DDEVICEQ pd3dDevice );
	HRESULT ClearAll ();

	/*ABL system, Juver, 2017/06/01 */
	HRESULT SetAttBone( DxAttBoneLink* pBoneLink, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread, DWORD dwFlag = 0x0 );
	DxAttBoneLink* GetAttBone ( int i )		{ return m_pAttBoneData[i]; }

	/*vehicle system, Juver, 2017/08/03 */
	HRESULT SetVehicle( DxVehicleData* pVehicle, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread, DWORD dwFlag = 0x0 );
	DxVehicleData* GetVehicle ()		{ return m_pVehicle; }

public:
	DxSkinCharData ();
	~DxSkinCharData ();
};


class DxSkinCharDataContainer
{
private:
	typedef std::map<std::string,DxSkinCharData*>			SKINCHARDATAMAP;
	typedef std::map<std::string,DxSkinCharData*>::iterator	SKINCHARDATAMAP_ITER;

protected:
	char			m_szPath[MAX_PATH];

	SKINCHARDATAMAP	m_mapSkinCharData;

public:
	void			SetPath ( char* szPath ) { StringCchCopy( m_szPath, MAX_PATH, szPath ); }
	char*			GetPath ()	{ return m_szPath; }

public:
	DxSkinCharData* FindData ( const char* szFile );
	DxSkinCharData* LoadData ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );

	void ReleaseData ( const char* szFile );
	BOOL DeleteData ( const char* szFile );

	HRESULT CleanUp ();

protected:
	DxSkinCharDataContainer ();

public:
	~DxSkinCharDataContainer ();

public:
	static DxSkinCharDataContainer& GetInstance();
};

