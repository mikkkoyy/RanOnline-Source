#ifndef GLITEMMIX_MAN_H_
#define GLITEMMIX_MAN_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <string>

#include "./GLItem.h"
#include "./GLItemMix.h"

typedef std::map<std::string,ITEM_MIX> ITEMMIX_STRMAP;
typedef std::map<DWORD,ITEM_MIX> ITEMMIX_KEYMAP;

typedef ITEMMIX_STRMAP::iterator	ITEMMIX_STRMAP_ITER;
typedef ITEMMIX_KEYMAP::iterator	ITEMMIX_KEYMAP_ITER;

/*product item, Juver, 2017/10/15 */
typedef std::vector<ITEM_MIX>		ITEMMIX_VEC;

class GLItemMixMan
{
public:
	enum
	{
		VERSION				= 0x0200,	
		VERSION_ENCODE		= 0x0200,
		VERSION_ENCODE_OLD	= 0x0100,
		ITEMMIX_MAXKEY		= 1000,
	};

public:
	static const char* _FILEHEAD;

	char			m_szFileName[MAX_PATH];
	ITEMMIX_STRMAP	m_mapStrItemMix;
	ITEMMIX_KEYMAP	m_mapKeyItemMix;

	/*product item, Juver, 2017/10/15 */
	ITEMMIX_KEYMAP	m_mapKeyProduct;

	std::string				m_strPATH;

public:
	void SetPath ( std::string strPATH )	{ m_strPATH = strPATH; }
	std::string GetPath ()					{ return m_strPATH.c_str(); }

protected:
	bool	m_bServer;

public:
	HRESULT Import ( const char* szFile );

	HRESULT LoadFile ( const char* szFile, BOOL bServer );
	HRESULT SaveFile ( const char* szFile );

	void insert ( ITEM_MIX& sItemMix );

	void SortMeterialItem( ITEM_MIX& sItemMix );	//	재료아이템 정렬
	void SortInvenItem ( SINVENITEM* pInvenItem );

	const ITEM_MIX* GetItemMix( const ITEM_MIX& sItemMix );
	const ITEM_MIX* GetItemMix( DWORD dwKey );

	/*product item, Juver, 2017/10/15 */
	const ITEM_MIX* GetProduct( DWORD dwKey );

public:
	GLItemMixMan ();

public:
	~GLItemMixMan ();
	void CleanUp();

public:
	static GLItemMixMan& GetInstance();
};

class ITEMMIX_DATA_CMP
{
public:
	bool operator() ( const ITEMMIX_DATA &lvalue, const ITEMMIX_DATA &rvalue )
	{
		if ( lvalue.sNID.dwID < rvalue.sNID.dwID ) return true;
		else if ( lvalue.sNID.dwID == rvalue.sNID.dwID ) 
		{	
			if ( lvalue.nNum < rvalue.nNum ) return true;
		}
		
		return false;
	}
};

class ITEMMIX_INVEN_CMP
{
public:
	bool operator() ( const SINVENITEM &lvalue, const SINVENITEM &rvalue )
	{
		if ( lvalue.sItemCustom.sNativeID.dwID < rvalue.sItemCustom.sNativeID.dwID ) return true;
		else if ( lvalue.sItemCustom.sNativeID.dwID == rvalue.sItemCustom.sNativeID.dwID ) 
		{	
			if ( lvalue.sItemCustom.wTurnNum < rvalue.sItemCustom.wTurnNum ) return true;
		}
		
		return false;
	}
};


#endif // GLITEMMIX_MAN_H_