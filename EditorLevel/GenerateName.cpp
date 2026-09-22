
#include "stdafx.h"
#include "GenerateName.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define GENERATE_NAME_MAX 9999

std::string CrowNameNew( GLLandMan*	pLand )
{
	if ( !pLand )
	{
		CDebugSet::MsgBox( "CrowNameNew Invalid pLand!" );
		return "";
	}

	for( int i=0; i<GENERATE_NAME_MAX; ++i )
	{
		char szTempChar[GLMobSchedule::EM_MAXSZNAME];
		sprintf( szTempChar, "CROW_%04d", i );
		SGLNODE<GLMobSchedule*>* p = pLand->GetMobSchMan()->FindMobSch( szTempChar );
		if ( !p )
		{
			return szTempChar;
			break;
		}
	}

	CDebugSet::MsgBox( "CrowNameNew Unable to find new name!" );
	return "";
}

bool CrowNameCheck( GLLandMan*	pLand, const char* szName )
{
	if ( pLand && szName )
	{
		SGLNODE<GLMobSchedule*>* p = pLand->GetMobSchMan()->FindMobSch( szName );
		return ( p != NULL );
	}

	CDebugSet::MsgBox( "CrowNameCheck error invalid input parameter" );
	return TRUE;
}

std::string GateNameNew( GLLandMan*	pLand )
{
	if ( !pLand )
	{
		CDebugSet::MsgBox( "GateNameNew Invalid pLand!" );
		return "";
	}

	for( int i=0; i<GENERATE_NAME_MAX; ++i )
	{
		char szTempChar[DxLandGate::MAX_SZNAME];
		sprintf( szTempChar, "GATE_%04d", i );
		PDXLANDGATE p = pLand->GetLandGateMan().FindLandGate( szTempChar );
		if ( !p )
		{
			return szTempChar;
			break;
		}
	}

	CDebugSet::MsgBox( "GateNameNew Unable to find new name!" );
	return "";
}

DWORD GateNewID( GLLandMan* pLand )
{
	int i = 0;

	if ( !pLand )
	{
		CDebugSet::MsgBox( "GateNewID Invalid pLand!" );
		return i;
	}

	for( i=0; i<GENERATE_NAME_MAX; ++i )
	{
		PDXLANDGATE p = pLand->GetLandGateMan().FindLandGate( i );
		if ( !p )
		{
			return i;
			break;
		}
	}

	return i;
}

bool GateNameCheck( GLLandMan*	pLand, const char* szName )
{
	if ( pLand && szName )
	{
		PDXLANDGATE p = pLand->GetLandGateMan().FindLandGate( szName );
		return ( p != NULL );
	}
	
	CDebugSet::MsgBox( "GateNameCheck error invalid input parameter" );
	return TRUE;
}

bool GateIDCheck( GLLandMan* pLand, DWORD dwID )
{
	if ( pLand  )
	{
		PDXLANDGATE p = pLand->GetLandGateMan().FindLandGate( dwID );
		return ( p != NULL );
	}
	
	return TRUE;
}

std::string EffectNameNew( GLLandMan* pLand )
{
	if ( !pLand )
	{
		CDebugSet::MsgBox( "EffectNameNew Invalid pLand!" );
		return "";
	}

	for( int i=0; i<GENERATE_NAME_MAX; ++i )
	{
		char szTempChar[DXLANDEFF::LANDEFF_MAXSZ];
		sprintf( szTempChar, "LANDEFF_%04d", i );
		PLANDEFF p = pLand->FindLandEff( szTempChar );
		if ( !p )
		{
			return szTempChar;
			break;
		}
	}

	CDebugSet::MsgBox( "EffectNameNew Unable to find new name!" );
	return "";
}

bool EffectNameCheck( GLLandMan*	pLand, const char* szName )
{
	if ( pLand && szName )
	{
		PLANDEFF p = pLand->FindLandEff( (char*)szName );
		return ( p != NULL );
	}

	CDebugSet::MsgBox( "EffectNameCheck error invalid input parameter" );
	return TRUE;
}

std::string LandMarkNameNew( GLLandMan*	pLand )
{
	if ( !pLand )
	{
		CDebugSet::MsgBox( "LandMarkNameNew Invalid pLand!" );
		return "";
	}

	for( int i=0; i<GENERATE_NAME_MAX; ++i )
	{
		char szTempChar[DXLANDEFF::LANDEFF_MAXSZ];
		sprintf( szTempChar, "LANDMARK_%04d", i );
		PLANDMARK p = pLand->FindLandMark( szTempChar );
		if ( !p )
		{
			return szTempChar;
			break;
		}
	}

	CDebugSet::MsgBox( "LandMarkNameNew Unable to find new name!" );
	return "";
}

bool LandMarkNameCheck( GLLandMan* pLand, const char* szName )
{
	if ( pLand && szName )
	{
		PLANDMARK p = pLand->FindLandMark( (char*)szName );
		return ( p != NULL );
	}

	CDebugSet::MsgBox( "LandMarkNameCheck error invalid input parameter" );
	return TRUE;
}