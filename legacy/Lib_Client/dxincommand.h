#ifndef DXINCOMMAND_H_
#define DXINCOMMAND_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*item link, Juver, 2017/07/31 */
struct SITEMLINK;

namespace dxincommand
{
	extern bool bDISP_FPS;
	extern bool bDISP_CONSOLE;

	/*item link, Juver, 2017/07/31 */
	bool command( const char* szcommand, SITEMLINK* pItemLink ); 
};

#endif // DXINCOMMAND_H_