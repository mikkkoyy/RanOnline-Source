/////////////////////////////////////////////////////////////////////////////
//	FileName	: LG7DLL.h
//	Project		: RanClientLib
//	Coder		: LG-7

#pragma once

#include "../Lib_Engine/GUInterface/UIDataType.h"
#include "s_NetClient.h"
#include "./GLItem.h"

#include <vector>

/////////////////////////////////////////////////////////////////////////////
// LG-7 GlobalRanking
//#define GLOBAL_RANKING_SIZE	12	//shaper ranking
#define GLOBAL_RANKING_SIZE	9	// 4class

#define MAX_TOP_RANK		50
#define MAX_ONE_VIEW_RANK	10

struct STOP_RANK_KILL
{
	DWORD	dwChaNum;
	char	szChaName[33];
	WORD	wChaLevel;
	int		nChaClass;
	WORD	wChaSchool;
	BOOL	bChaOnline;
	WORD	wGuNum;
	WORD	wGuMarkVer;
	char	szGuName[33];
	DWORD	dwChaPKWin;
	DWORD	dwChaPKLoss;

	STOP_RANK_KILL()
		: dwChaNum(0)
		, wChaLevel(0)
		, nChaClass(0)
		, wChaSchool(0)
		, bChaOnline(FALSE)
		, wGuNum(0)
		, wGuMarkVer(0)
		, dwChaPKWin(0)
		, dwChaPKLoss(0)
	{
		SecureZeroMemory(szChaName,	sizeof(char) * 33);
		SecureZeroMemory(szGuName,	sizeof(char) * 33);
	}
};

struct STOP_RANK_RICH
{
	DWORD		dwChaNum;
	char		szChaName[33];
	WORD		wChaLevel;
	int			nChaClass;
	WORD		wChaSchool;
	BOOL		bChaOnline;
	WORD		wGuNum;
	WORD		wGuMarkVer;
	char		szGuName[33];
	LONGLONG	llnMoney;

	STOP_RANK_RICH()
		: dwChaNum(0)
		, wChaLevel(0)
		, nChaClass(0)
		, wChaSchool(0)
		, bChaOnline(FALSE)
		, wGuNum(0)
		, wGuMarkVer(0)
		, llnMoney(0)
	{
		SecureZeroMemory(szChaName,	sizeof(char) * 33);
		SecureZeroMemory(szGuName,	sizeof(char) * 33);
	}
};

struct STOP_RANK_GUILD
{
	WORD	wGuNum;
	char	szGuName[33];
	WORD	wRank;
	WORD	wGuMarkVer;
	WORD	wAlliance;
	WORD	wOnline;
	DWORD	dwWin;
	DWORD	dwLoss;
	DWORD	dwDraw;

	STOP_RANK_GUILD()
		: wGuNum(0)
		, wRank(0)
		, wGuMarkVer(0)
		, wAlliance(0)
		, wOnline(0)
		, dwWin(0)
		, dwLoss(0)
		, dwDraw(0)
	{
		SecureZeroMemory(szGuName, sizeof(char) * 33);
	}
};

typedef std::vector<STOP_RANK_KILL>		VEC_TOP_KILL;
typedef VEC_TOP_KILL::iterator			VEC_TOP_KILL_ITER;

typedef std::vector<STOP_RANK_RICH>		VEC_TOP_RICH;
typedef VEC_TOP_RICH::iterator			VEC_TOP_RICH_ITER;

typedef std::vector<STOP_RANK_GUILD>	VEC_TOP_GUILD;
typedef VEC_TOP_GUILD::iterator			VEC_TOP_GUILD_ITER;
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// LG-7 CharView
struct SCHAR_VIEW
{
	SITEMCUSTOM	m_PutOnItems_[SLOT_NSIZE_S_2];
	WORD		wChaHair;
	WORD		wChaHairColor;
	WORD		wChaFace;
	WORD		wChaLevel;
	int			nChaClass;
	WORD		wChaSchool;

	WORD		wGuNum;
	WORD		wGuMarkVer;

	char		szChaName[33];
	char		szGuName[33];

	int			nAttLow;
	int			nAttHigh;
	int			nDef;
	WORD		wMelee;
	WORD		wRange;
	WORD		wMagic;
	DWORD		dwMaxHP;
	int			nMaxMP;
	int			nMaxSP;
	int			nAcc;
	int			nEva;

	SCHAR_VIEW()
		: wChaHair(-1)
		, wChaHairColor(-1)
		, wChaFace(-1)
		, wChaLevel(-1)
		, nChaClass(0)
		, wChaSchool(0)

		, wGuNum(0)
		, wGuMarkVer(0)

		, nAttLow(-1)
		, nAttHigh(-1)
		, nDef(-1)
		, wMelee(-1)
		, wRange(-1)
		, wMagic(-1)
		, dwMaxHP(-1)
		, nMaxMP(-1)
		, nMaxSP(-1)
		, nAcc(-1)
		, nEva(-1)
	{
		SecureZeroMemory(szChaName,	sizeof(char) * 33);
		SecureZeroMemory(szGuName,	sizeof(char) * 33);
	}
};
/////////////////////////////////////////////////////////////////////////////