#pragma once

#include <list>
#include <map>
#include <string>

#include "bytestream.h"
#include "SerialFile.h"
#include "DxEffChar.h"

#include "DxSkinChar.h"
#include "DxAttBone.h"
#include "DxAttBoneRender.h"

/*vehicle system, Juver, 2017/08/05 */
#include "DxVehicle.h"

class DxEffSelfEffect
{
public:
	struct SEFFDATA
	{
		CByteStream	m_byteStream;

		SEFFDATA ()
		{
		}
	};

private:
	typedef std::list<SEFFDATA*>			EFFDATALIST;
	typedef std::list<SEFFDATA*>::iterator	EFFDATALIST_ITER;

protected:
	std::string		m_strFileName;
	EFFDATALIST		m_listEffData;
	EMPIECECHAR		m_emPieceType;

public:
	void SetFileName ( std::string strFileName )			{ m_strFileName = strFileName; }
	BOOL Import ( EMPIECECHAR _emType, GLEFFCHAR_VEC &vecEFFECT );

public:
	BOOL SetPart ( DxCharPart *pCharPart, LPDIRECT3DDEVICEQ	pd3dDevice );

public:
	void CleanUp ();

public:
	BOOL LoadFile ( basestream &SFile );
	BOOL SaveFile ( CSerialFile &SFile );
	BOOL LoadFile ( const char* szFile );
	BOOL SaveFile ( const char* szFile );


public:
	DxEffSelfEffect(void);
	~DxEffSelfEffect(void);
};
typedef DxEffSelfEffect* PEFFCHARDATA_ABL;

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

class DxEffSelfEffectMan
{
protected:
	std::string			m_strPath;
	LPDIRECT3DDEVICEQ	m_pd3dDevice;

public:
	void SetPath ( char *szPath )	{ m_strPath = szPath; }
	const char* GetPath ()			{ return m_strPath.c_str(); }
	
private:
	typedef std::map<std::string,DxEffSelfEffect*>					EFFDATA_MAP;
	typedef std::map<std::string,DxEffSelfEffect*>::iterator		EFFDATA_MAP_ITER;

protected:
	EFFDATA_MAP			m_mapEffData;

public:
	HRESULT InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice );

public:
	BOOL PutEffect ( DxCharPart *pCharPart, const char* szEffFile );
	BOOL PutEffect ( DxSkinChar *pSkinChar, const char* szEffFile );
	BOOL PutEffect ( DxAttBone *pAttBone, const char* szEffFile );
	BOOL PutEffect ( DxAttBoneRender *pAttBone, const char* szEffFile );
	BOOL PutEffect ( DxVehicle *pVehicle, const char* szEffFile ); /*vehicle system, Juver, 2017/08/05 */

	void CleanUp ();

private:
	DxEffSelfEffectMan ()
	{
	}

public:
	~DxEffSelfEffectMan ()
	{
		CleanUp ();
	}

public:
	static DxEffSelfEffectMan& GetInstance();
};


