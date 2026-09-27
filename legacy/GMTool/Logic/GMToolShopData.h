#ifndef _GMTOOL_SHOP_DATA_H_
#define _GMTOOL_SHOP_DATA_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GMToolData.h"

namespace GMTOOL_SHOP_DATA
{
	extern SGMTOOL_SHOP_ITEM_MAP gmtoolSHOPDATA;

	SGMTOOL_SHOP_ITEM* FindShopItem( DWORD _dwProductNum );
	BOOL DeleteShopItem( DWORD _dwProductNum );
};


#endif // _GMTOOL_SHOP_DATA_H_