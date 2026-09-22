#include "StdAfx.h"
#include "GLPVPCaptureTheFlagClient.h"

#include "GLGaeaClient.h"
#include "DxGlobalStage.h"

#include "InnerInterface.h"
#include "UITextControl.h"
#include "GameTextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void GLPVPCaptureTheFlagClient::FlagActionCapture( DWORD dwCharID, EMNPC_CTF_FLAG_TYPE emCTFType, DWORD dwGlobID )
{
	if ( !IsBattle() )	return;

	if ( emCTFType != EMNPC_CTF_FLAG_TYPE_A && emCTFType != EMNPC_CTF_FLAG_TYPE_B )		return;
	if ( emCTFType == EMNPC_CTF_FLAG_TYPE_A  && m_wTeam != CAPTURE_THE_FLAG_TEAM_B )	return;
	if ( emCTFType == EMNPC_CTF_FLAG_TYPE_B  && m_wTeam != CAPTURE_THE_FLAG_TEAM_A )	return;
	if ( m_wTeam < CAPTURE_THE_FLAG_TEAM_SIZE && m_bFlagHold[m_wTeam] == TRUE )			return;

	GLMSG::SNETPC_CAPTURE_THE_FLAG_C2F_FLAG_ACTION_CAPTURE netmsgflag;
	netmsgflag.dwGlobID = dwGlobID;
	netmsgflag.dwCharID = dwCharID;
	NETSENDTOFIELD ( &netmsgflag );
}

