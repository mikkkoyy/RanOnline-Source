/*!
 * \file DxAttBoneLink.h
 *
 * \author Juver
 * \date May 2017
 * ABL system
 * 
 */

#pragma once

#include <map>
#include <string>
#include <vector>

#include "basestream.h"
#include "DxMethods.h"
#include "DxSkinAniMan.h"
#include "DxSkinPieceContainer.h"
#include "DxAttBoneData.h"

class DxAttBoneLink
{
public:
	enum 
	{ 
		VERSION_ABL		= 0x0201,
		VERSION_ENCRYPT = 0x0200,
	};

private:
	enum		{ FILE_LENGTH = 64 };
	char		m_szFileName[FILE_LENGTH];
	
private:
	DxAttBoneData*		m_pAttBoneData;
	

public:
	DWORD				m_dwRef;
	std::string			m_strAttBoneData;
	std::string			m_strSkeleton;
	EMPIECECHAR			m_emPieceStrike;
	EMPIECECHAR			m_emPieceType;
	EMPEACEZONEWEAPON	m_emWeaponWhereBack;
	std::string			m_strMaskPiece;

	std::string			m_strBoneTrans;
	DXAFFINEPARTS		m_affBoneTrans;
	DXAFFINEPARTS		m_affPeaceZone;

public:
	DxAttBoneData*		GetData()	{ return m_pAttBoneData; }
	void	SetData( DxAttBoneData* pAttBoneData );

public:
	const char*		GetFileName(){ return m_szFileName; }

protected:
	//official versions
	BOOL LOAD_Ver_0100( basestream &SFile );
	BOOL LOAD_Ver_0101( basestream &SFile );
	BOOL LOAD_Ver_0102( basestream &SFile );

	//own version
	BOOL LOAD_Ver_0200( basestream &SFile );
	BOOL LOAD_Ver_0201( basestream &SFile );

public:
	BOOL LoadFile ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	BOOL SaveFile ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice );

public:
	DxAttBoneLink ();
	~DxAttBoneLink ();
};

class DxAttBoneLinkContainer
{
private:
	typedef std::map<std::string,DxAttBoneLink*>			ATTBONELINKMAP;
	typedef std::map<std::string,DxAttBoneLink*>::iterator	ATTBONELINKMAP_ITER;

protected:
	char				m_szPath[MAX_PATH];
	ATTBONELINKMAP		m_mapAttBoneLink;

public:
	void			SetPath ( char* szPath ) { StringCchCopy( m_szPath, MAX_PATH, szPath ); }
	char*			GetPath ()	{ return m_szPath; }

public:
	DxAttBoneLink* FindData ( const char* szFile );
	DxAttBoneLink* LoadData ( const char* szFile, LPDIRECT3DDEVICEQ pd3dDevice, const BOOL bThread );
	void ReleaseData ( const char* szFile );
	BOOL DeleteData ( const char* szFile );
	HRESULT CleanUp ();

protected:
	DxAttBoneLinkContainer ();

public:
	~DxAttBoneLinkContainer ();

public:
	static DxAttBoneLinkContainer& GetInstance();
};