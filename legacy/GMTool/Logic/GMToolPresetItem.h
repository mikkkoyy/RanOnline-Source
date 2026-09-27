#ifndef GMTOOL_PRESET_ITEM
#define GMTOOL_PRESET_ITEM

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Logic/GMToolData.h"

class CGMToolPresetItem
{

public:
	std::string				m_strPATH;
	SGMTOOL_PRESET_ITEM_MAP	m_MapPresetItem;

public:
	void	SetPath( std::string strPATH )	{	m_strPATH = strPATH.c_str();	}
	BOOL	PresetLoad();
	BOOL	PresetSave();

public:
	SGMTOOL_PRESET_ITEM*	PresetItemGet( WORD wID );
	BOOL PresetItemAdd( SGMTOOL_PRESET_ITEM sPRESET );
	void PresetItemDel( WORD wID );
	void PresetItemDelAll();
	WORD PresetItemNewID();

public:
	CGMToolPresetItem();
	~CGMToolPresetItem();

public:
	static CGMToolPresetItem& GetInstance ();

};

#endif // GMTOOL_PRESET_ITEM