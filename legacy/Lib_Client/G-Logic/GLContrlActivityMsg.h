#pragma once

#include "GLContrlBaseMsg.h"
#include "GLActivityData.h"

enum EMFB_CHAR_TITLE
{
	EMFB_CHAR_TITLE_FAIL	= 0,
	EMFB_CHAR_TITLE_SAME	= 1,
	EMFB_CHAR_TITLE_DONE	= 2,
};

namespace GLMSG
{
	#pragma pack(1)

	struct SNETPC_ACTIVITY_COMPLETE_BRD : public SNETPC_BROAD
	{
		SNETPC_ACTIVITY_COMPLETE_BRD ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_ACTIVITY_COMPLETE_BRD;
		}
	};

	struct SNETPC_ACTIVITY_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		SACTIVITY_CHAR_DATA sData;

		SNETPC_ACTIVITY_UPDATE () 
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_ACTIVITY_UPDATE;
		}
	};

	struct SNETPC_ACTIVITY_COMPLETE
	{
		NET_MSG_GENERIC		nmg;
		SACTIVITY_CHAR_DATA sData;

		SNETPC_ACTIVITY_COMPLETE () 
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_ACTIVITY_COMPLETE;
		}
	};

	
	struct SNETPC_ACTIVITY_NOTIFY_AGENT
	{
		NET_MSG_GENERIC		nmg;
		char	szCharacterName[CHAR_SZNAME];
		char	szActivityTitle[ACTIVITY_TITLE_MSG_SIZE];

		SNETPC_ACTIVITY_NOTIFY_AGENT ()
		{
			memset (szCharacterName, 0, sizeof(char) * CHAR_SZNAME);
			memset (szActivityTitle, 0, sizeof(char) * ACTIVITY_TITLE_MSG_SIZE );

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_ACTIVITY_NOTIFY_AGENT;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_ACTIVITY_NOTIFY_CLIENT
	{
		NET_MSG_GENERIC		nmg;
		char	szCharacterName[CHAR_SZNAME];
		char	szActivityTitle[ACTIVITY_TITLE_MSG_SIZE];

		SNETPC_ACTIVITY_NOTIFY_CLIENT ()
		{
			memset (szCharacterName, 0, sizeof(char) * CHAR_SZNAME);
			memset (szActivityTitle, 0, sizeof(char) * ACTIVITY_TITLE_MSG_SIZE);

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_ACTIVITY_NOTIFY_CLIENT;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_REQ_CHARACTER_BADGE_CHANGE
	{
		NET_MSG_GENERIC		nmg;

		DWORD		dwBadgeID;

		SNETPC_REQ_CHARACTER_BADGE_CHANGE () 
			: dwBadgeID(UINT_MAX)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_CHARACTER_BADGE_CHANGE);
			nmg.nType = NET_MSG_GCTRL_CHARACTER_BADGE_CHANGE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_REQ_CHARACTER_BADGE_CHANGE_FB
	{
		NET_MSG_GENERIC			nmg;
		EMFB_CHAR_TITLE			emFB;
		char		szBadge[CHAR_SZNAME];

		SNETPC_REQ_CHARACTER_BADGE_CHANGE_FB()
			: emFB(EMFB_CHAR_TITLE_FAIL)
		{
			nmg.dwSize = sizeof(SNETPC_REQ_CHARACTER_BADGE_CHANGE_FB);
			nmg.nType = NET_MSG_GCTRL_CHARACTER_BADGE_CHANGE_FB;
			memset (szBadge, 0, sizeof(char) * CHAR_SZNAME);
			GASSERT( nmg.dwSize <= NET_DATA_BUFSIZE );
		}
	};

	struct SNETPC_REQ_CHARACTER_BADGE_CHANGE_BRD : public SNETPC_BROAD
	{
		char	szBadge[CHAR_SZNAME];

		SNETPC_REQ_CHARACTER_BADGE_CHANGE_BRD ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CHARACTER_BADGE_CHANGE_BRD;
			memset (szBadge, 0, sizeof(char) * CHAR_SZNAME);
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	#pragma pack()
};

