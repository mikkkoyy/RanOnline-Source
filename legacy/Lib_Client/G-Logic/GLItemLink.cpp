///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GLItemLink.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void SITEMLINK::BuildFilter( CString strItemName, BYTE uGrade, BOOL bWrap, CString strWrapItem )
{
	CString str;
	str += "< ";

	CString strItemText;
	if ( uGrade != 0 )
	{
		CString strGrade;
		strGrade.Format ( "+%d ", uGrade );
		strItemText += strGrade;
	}

	strItemText += strItemName.GetString();
	//strItemText.Replace( " ", "_" );

	/*item wrapper, Juver, 2018/01/12 */
	if ( bWrap && strWrapItem.GetLength() )
	{
		strItemText += " ";
		strItemText += strWrapItem.GetString();
	}

	str += strItemText.GetString();
	str += " >";

	StringCchCopy( szName, MAX_ITEMLINK_NAME_SIZE, str.GetString() );
};