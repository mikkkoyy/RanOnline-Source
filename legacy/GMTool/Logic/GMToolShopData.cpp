
#include "stdafx.h"
#include "GMToolShopData.h"

namespace GMTOOL_SHOP_DATA
{
	SGMTOOL_SHOP_ITEM_MAP gmtoolSHOPDATA;

	SGMTOOL_SHOP_ITEM* FindShopItem( DWORD _dwProductNum )
	{
		SGMTOOL_SHOP_ITEM_MAP_ITER iter = gmtoolSHOPDATA.find( _dwProductNum );
		if ( iter != gmtoolSHOPDATA.end() ){
			return &(*iter).second;
		}

		return NULL;
	};

	BOOL DeleteShopItem( DWORD _dwProductNum )
	{
		SGMTOOL_SHOP_ITEM_MAP_ITER iter = gmtoolSHOPDATA.find( _dwProductNum );
		if ( iter != gmtoolSHOPDATA.end() ){
			gmtoolSHOPDATA.erase( iter );
			return TRUE;
		}

		return FALSE;
	};
};