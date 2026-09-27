/////////////////////////////////////////////////////////////////////////////
//	FileName	: GLAutoSystemMsg.h
//	Project		: Lib -- RanClient
//	Coder		: LG-7

#pragma once

#include "GLAutoSystem.h"

enum EMTYPE
{
	AUTOSYSTEM_TYPE_NONE	= 0,
	AUTOSYSTEM_TYPE_POTION	= 1,
	AUTOSYSTEM_TYPE_FOLLOW	= 2
};

enum EMREQ
{
	EMREQ_NONE					= 0,
	EMREQ_POTION_DELAY			= 1,
	EMREQ_FOLLOW				= 2,
	EMREQ_FOLLOW_UPDATE			= 3,
	EMREQ_FOLLOW_START			= 4,
	EMREQ_FOLLOW_STOP			= 5,
	EMREQ_FOLLOW_STOP_NO_USER	= 6
};

namespace GLMSG
{
#pragma pack(1)

	struct SNETPC_REQ_AUTOSYSTEM
	{
		NET_MSG_GENERIC	nmg;
		EMTYPE			emTYPE;
		EMREQ			emREQ;

		BOOL			bPotionActive;
		int				nPotionTab;
		int				nHPSet;
		int				nMPSet;
		int				nSPSet;
		float			fAutoPotDelay;
	
		SAUTO_FOLLOW	sAutoFollow;

		SNETPC_REQ_AUTOSYSTEM()
			: emTYPE(AUTOSYSTEM_TYPE_NONE)
			, emREQ(EMREQ_NONE)

			, bPotionActive(FALSE)
			, nPotionTab(-1)
			, nHPSet(-1)
			, nMPSet(-1)
			, nSPSet(-1)			
			, fAutoPotDelay(0.0f)
		{
			sAutoFollow.RESET();

			nmg.dwSize	= sizeof(*this);
			nmg.nType	= NET_MSG_GCTRL_REQ_AUTOSYSTEM;
			GASSERT(nmg.dwSize <= NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_REQ_AUTOSYSTEM_FB
	{
		NET_MSG_GENERIC	nmg;
		EMTYPE			emTYPE;
		EMREQ			emREQ;

		float			fAutoPotDelay;

		SAUTO_FOLLOW	sAutoFollow;

		SNETPC_REQ_AUTOSYSTEM_FB()
			: emTYPE(AUTOSYSTEM_TYPE_NONE)
			, emREQ(EMREQ_NONE)

			, fAutoPotDelay(0.0f)
		{
			sAutoFollow.RESET();

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_REQ_AUTOSYSTEM_FB;
			GASSERT(nmg.dwSize <= NET_DATA_BUFSIZE);
		}
	};

#pragma pack()
};
