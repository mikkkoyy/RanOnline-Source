#include "stdafx.h"
#include "GameClient2.h"
#include "GameClient2Wnd.h"

#include "HShield.h"

#include "DxGlobalStage.h"


const char* CGameClient2App::hs_dll_name = "EhSvc.dll";
const char* CGameClient2App::hs_folder = "\\HackShield\\";

int CGameClient2App::hs_gamecode = 1000;
const char* CGameClient2App::hs_key = "B228F2916A48AC24";

DWORD CGameClient2App::hs_option = AHNHS_CHKOPT_ALL | AHNHS_CHKOPT_LOCAL_MEMORY_PROTECTION;
DWORD CGameClient2App::hs_sh_option = AHNHS_SPEEDHACK_SENSING_RATIO_HIGHEST;

int __stdcall HS_CallbackProc(long lCode, long lParamSize, void* pParam)
{
	CGameClient2App::hs_output( lCode, lParamSize, pParam );

    return TRUE;
}

void CGameClient2App::hs_output( long code, long param_size, void* pparam )
{
	bool bAppExit( false );
	LPCTSTR lpszMsg( NULL );
    switch(code)
    {
	case AHNHS_ENGINE_DETECT_GAME_HACK:
		{
			std::string detected = (char*)pparam;
			if ( detected.find( "WS2_32.dll" ) != std::string::npos )
				break;

			HS_GLOBAL_DATA::hs_detect_ret = (int)code;
			HS_GLOBAL_DATA::hs_detect_string = detected;
			DxGlobalStage::GetInstance().hs_send_callback( HS_GLOBAL_DATA::hs_detect_ret, HS_GLOBAL_DATA::hs_detect_string.c_str() );
		}break;

	case AHNHS_ACTAPC_DETECT_KDTRACE:            
	case AHNHS_ACTAPC_DETECT_AUTOMACRO:
	case AHNHS_ACTAPC_DETECT_ABNORMAL_FUNCTION_CALL:
	case AHNHS_ACTAPC_DETECT_ABNORMAL_MEMORY_ACCESS:
	case AHNHS_ACTAPC_DETECT_DRIVERFAILED:
	case AHNHS_ACTAPC_DETECT_ENGINEFAILED:
	case AHNHS_ACTAPC_DETECT_MEM_MODIFY_FROM_LMP:
	case AHNHS_ACTAPC_DETECT_LMP_FAILED:
	case AHNHS_ACTAPC_DETECT_ABNORMAL_HACKSHIELD_STATUS:
		{
			HS_GLOBAL_DATA::hs_detect_ret = (int)code;
			HS_GLOBAL_DATA::hs_detect_string = "";
			DxGlobalStage::GetInstance().hs_send_callback( HS_GLOBAL_DATA::hs_detect_ret, HS_GLOBAL_DATA::hs_detect_string.c_str() );
		}break;

	case AHNHS_ACTAPC_STATUS_HACKSHIELD_RUNNING:
		{
			_AhnHS_GetSDKVersion( &HS_GLOBAL_DATA::hs_sdk_version );
			DxGlobalStage::GetInstance().hs_send_heartbeat( HS_GLOBAL_DATA::hs_sdk_version );
		}break;
    }
}

BOOL CGameClient2App::hs_start()
{
	TCHAR fullpath[MAX_PATH];
	StringCchCopy( fullpath, MAX_PATH, m_szAppPath );
	StringCchCat( fullpath, MAX_PATH, hs_folder );
	StringCchCat( fullpath, MAX_PATH, hs_dll_name );

	HS_GLOBAL_DATA::hs_start_ret = _AhnHS_Initialize ( fullpath, HS_CallbackProc, hs_gamecode, hs_key, hs_option, hs_sh_option );
	if ( HS_GLOBAL_DATA::hs_start_ret != HS_ERR_OK )
		return FALSE;

	_AhnHS_GetSDKVersion( &HS_GLOBAL_DATA::hs_sdk_version );

	return TRUE;
}

BOOL CGameClient2App::hs_stop()
{
	int ret = _AhnHS_Uninitialize();
	if ( ret != HS_ERR_OK )	
		return FALSE;
	return TRUE;
}

BOOL CGameClient2App::hs_start_service()
{
	HS_GLOBAL_DATA::hs_service_ret = _AhnHS_StartService();
	if ( HS_GLOBAL_DATA::hs_service_ret != HS_ERR_OK )
		return FALSE;

	HS_GLOBAL_DATA::hs_monitor_ret = _AhnHS_CheckHackShieldRunningStatus();
	if ( HS_GLOBAL_DATA::hs_monitor_ret != HS_ERR_OK )
		return FALSE;

	return TRUE;
}

BOOL CGameClient2App::hs_stop_service()
{
	int ret = _AhnHS_StopService();
	if ( ret != HS_ERR_OK )	
		return FALSE;
	return TRUE;
}