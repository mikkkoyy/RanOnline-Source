#pragma once

#include "GLContrlBaseMsg.h"
#include "GLCodexData.h"

namespace GLMSG
{
	#pragma pack(1)

	struct SNETPC_CODEX_COMPLETE_BRD : public SNETPC_BROAD
	{
		SNETPC_CODEX_COMPLETE_BRD ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CODEX_COMPLETE_BRD;
		}
	};

	struct SNETPC_CODEX_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		SCODEX_CHAR_DATA sData;

		SNETPC_CODEX_UPDATE () 
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CODEX_UPDATE;
		}
	};

	struct SNETPC_CODEX_COMPLETE
	{
		NET_MSG_GENERIC		nmg;
		SCODEX_CHAR_DATA sData;

		SNETPC_CODEX_COMPLETE () 
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CODEX_COMPLETE;
		}
	};

	
	struct SNETPC_CODEX_NOTIFY_AGENT
	{
		NET_MSG_GENERIC		nmg;
		char	szCharacterName[CHAR_SZNAME];
		char	szCodexTitle[CODEX_TITLE_MSG_SIZE];

		SNETPC_CODEX_NOTIFY_AGENT ()
		{
			memset (szCharacterName, 0, sizeof(char) * CHAR_SZNAME);
			memset (szCodexTitle, 0, sizeof(char) * CODEX_TITLE_MSG_SIZE );

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CODEX_NOTIFY_AGENT;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CODEX_NOTIFY_CLIENT
	{
		NET_MSG_GENERIC		nmg;
		char	szCharacterName[CHAR_SZNAME];
		char	szCodexTitle[CODEX_TITLE_MSG_SIZE];

		SNETPC_CODEX_NOTIFY_CLIENT ()
		{
			memset (szCharacterName, 0, sizeof(char) * CHAR_SZNAME);
			memset (szCodexTitle, 0, sizeof(char) * CODEX_TITLE_MSG_SIZE);

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CODEX_NOTIFY_CLIENT;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};
	
	/*Codex, jhoniex 2/5/2024*/
	struct SNET_REGISTER_CODEX
	{
		NET_MSG_GENERIC		nmg;

		//WORD				wPosX;
		//WORD				wPosY;
		SNATIVEID		sItemID;
		DWORD			sActivityID;
		WORD			wGrade;
		WORD			wQuantity;

		SNET_REGISTER_CODEX () 
			: sItemID(NATIVEID_NULL())
			, sActivityID(0)
			, wGrade(0)
			, wQuantity(0)
		{
			nmg.dwSize = sizeof(SNET_REGISTER_CODEX);
			nmg.nType = NET_MSG_GCTRL_REGISTER_CODEX;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_REGISTER_CODEX_FB
	{
		NET_MSG_GENERIC		nmg;

		EMREQ_REGISTER_CODEX_FB	emFB;

		SNET_REGISTER_CODEX_FB () 
			: emFB(EMREQ_REGISTER_CODEX_FB_FAIL)
		{
			nmg.dwSize = sizeof(SNET_REGISTER_CODEX_FB);
			nmg.nType = NET_MSG_GCTRL_REGISTER_CODEX_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	#pragma pack()
};

