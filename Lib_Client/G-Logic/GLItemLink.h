/*!
 * \file GLItemLink.h
 *
 * \author Juver
 * \date July 2017
 *
 * 
 */

#ifndef GLITEMLINK_H_
#define GLITEMLINK_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//originally SITEMCUSTOM
//changes in SITEMCUSTOM must be applied here too
#include "../Lib_Engine/G-Logic/GLDefine.h"

/*item color, Juver, 2018/01/04 */
#define ITEMCOLOR_WHITE_ITEM_LINK 32767

enum 
{
	MAX_ITEMLINK_NAME_SIZE = 100,
};

struct SITEMLINK
{
public:
	bool		bUSE;
	SNATIVEID	sNativeID;
	SNATIVEID	nidDISGUISE;
	__time64_t	tBORNTIME;
	__time64_t	tDISGUISE;

	LONGLONG	lnGenNum;
	WORD		wTurnNum;
	BYTE		cGenType;
	BYTE		cChnID;

	BYTE		cFieldID;

public:
	BYTE		cDAMAGE;
	BYTE		cDEFENSE;
	BYTE		cRESIST_FIRE;
	BYTE		cRESIST_ICE;
	BYTE		cRESIST_ELEC;
	BYTE		cRESIST_POISON;
	BYTE		cRESIST_SPIRIT;

public:
	BYTE		cOptTYPE1;
	BYTE		cOptTYPE2;
	BYTE		cOptTYPE3;
	BYTE		cOptTYPE4;

	short		nOptVALUE1;
	short		nOptVALUE2;
	short		nOptVALUE3;
	short		nOptVALUE4;

public:
	DWORD		dwPetID;
	DWORD		dwVehicleID;

	bool		bVietnamGainItem;

	/*item color, Juver, 2018/01/04 */
	WORD		wColor1;
	WORD		wColor2;

	bool		isBoxLock;

	SNATIVEID	sSkillLinkID;

	char		szName[MAX_ITEMLINK_NAME_SIZE];

	SITEMLINK()
		: bUSE( false )
		, sNativeID(false)
		, nidDISGUISE(false)
		, tBORNTIME(0)
		, tDISGUISE(0)
		, lnGenNum(0)
		, wTurnNum(1)
		, cGenType(0)
		, cChnID(0)

		, cFieldID(0)

		, cDAMAGE(0)
		, cDEFENSE(0)
		, cRESIST_FIRE(0)

		, cRESIST_ICE(0)
		, cRESIST_ELEC(0)
		, cRESIST_POISON(0)
		, cRESIST_SPIRIT(0)

		, cOptTYPE1(0)
		, cOptTYPE2(0)
		, cOptTYPE3(0)
		, cOptTYPE4(0)

		, nOptVALUE1(0)
		, nOptVALUE2(0)
		, nOptVALUE3(0)
		, nOptVALUE4(0)

		, dwPetID(0)
		, dwVehicleID( 0 )
		, bVietnamGainItem( false )

		/*item color, Juver, 2018/01/04 */
		, wColor1(ITEMCOLOR_WHITE_ITEM_LINK)
		, wColor2(ITEMCOLOR_WHITE_ITEM_LINK)
		, isBoxLock ( true )
		, sSkillLinkID(NATIVEID_NULL())
	{
		StringCchCopy( szName, MAX_ITEMLINK_NAME_SIZE, "" );
	}

	SITEMLINK& operator= ( const SITEMLINK &value )
	{
		bUSE			= value.bUSE;
		sNativeID		= value.sNativeID;
		nidDISGUISE		= value.nidDISGUISE;
		tBORNTIME		= value.tBORNTIME;
		tDISGUISE		= value.tDISGUISE;

		lnGenNum		= value.lnGenNum;
		wTurnNum		= value.wTurnNum;
		cGenType		= value.cGenType;
		cChnID			= value.cChnID;

		cFieldID		= value.cFieldID;

		cDAMAGE			= value.cDAMAGE;
		cDEFENSE		= value.cDEFENSE;
		cRESIST_FIRE	= value.cRESIST_FIRE;
		cRESIST_ICE		= value.cRESIST_ICE;
		cRESIST_ELEC	= value.cRESIST_ELEC;
		cRESIST_POISON	= value.cRESIST_POISON;
		cRESIST_SPIRIT	= value.cRESIST_SPIRIT;

		cOptTYPE1		= value.cOptTYPE1;
		cOptTYPE2		= value.cOptTYPE2;
		cOptTYPE3		= value.cOptTYPE3;
		cOptTYPE4		= value.cOptTYPE4;

		nOptVALUE1		= value.nOptVALUE1;
		nOptVALUE2		= value.nOptVALUE2;
		nOptVALUE3		= value.nOptVALUE3;
		nOptVALUE4		= value.nOptVALUE4;

		dwPetID			= value.dwPetID;
		dwVehicleID		= value.dwVehicleID;

		bVietnamGainItem	= value.bVietnamGainItem;

		/*item color, Juver, 2018/01/04 */
		wColor1		= value.wColor1;
		wColor2		= value.wColor2;

		isBoxLock 	= value.isBoxLock;
		sSkillLinkID = value.sSkillLinkID;

		StringCchCopy( szName, MAX_ITEMLINK_NAME_SIZE, value.szName );

		return *this;
	};

	/*item wrapper, Juver, 2018/01/12 */
	void	BuildFilter( CString strItemName, BYTE uGrade, BOOL bWrap, CString strWrapItem );
	
	CString GetFilter()
	{
		CString str = szName;
		return str;
	};
	
};


#endif // GLITEMLINK_H_




//                                      /|
//                                      ||
//                                      ||
//                                      ||
//                                      ||
//       .--._                          ||
//  _.-"" (   /                         ||                        .-.
// (_.-(   `. \                         ||                        )  `-._
//(  (   __.   \                        ||                   __.-' ` .)._)
//(  (  _.-'  `.`-._                    ||               _.-"    )  )     )
// (  (   (         `.                  ||            .-'   .'  `.   )` - .)
// ((  .-'   .-'  `.  `-.               ||          .'   .'     )  ) - . )  )
//  (  ( .-'       `-.   \            __/\__       / .'     `-.   `. )     )
//   `. ( _.-'         `. `.          _(  |       :     `-.       )   `-.  )
//    (   (   .'.-'         `.       /  ) /     .'  .'`.     `-.     ) _.-'
//    (.-' (      .-'    `.   `.    | ,'| |   .' .'                )  )
//     (     (     .'        `. `.,' /  |  \.'  : `-._`-.  `-.  `.)`.)
//      `-._  (       .'  .     ,'  /    \  \ .'               )`.  )
//          (      (       .'  /   ,-""-./\  \ `.  `-.  )   )`-._.-'
//           `-.      .'       |  / __.. `|  /               .-'
//              \   .'     .'  | | /_  _\/  / `-. `.    ) .-'
//               `-._.'        | \ )    (|  |            /
//                   `-._   ,; |.' \    /   |   ` .  _.-'
//                       ""-._ /   )`--'|   |  `._.-'
//                            /   /`-..-'  /_..-'
//                           ;  _/        |
//                            ,'          `.
//                            |            |
//                            `.__.-' '.__.'
//                               `.       |
//                                 \       `.
//                                  |         .
//                                  |          \
//                                  |           .
//                                 ,'           |
//                                .'            |
//                               ,'             |
//                              .'             /
//                              |              /
//                              |             |
//                              |             /
//                             |             |
//                             |             |
//                            /               \
//                           /    /            |
//                          /           .:     |
//                         /    .      .:      |
//                        /    .:      .:     ,'
//                      ,'    .:      .::__.-|
//                     -..__   : __,,..-' \  |
//                       /  |''''         /  (
//                     .'  ,'             \_  `.
//                     `..'                 `.__;