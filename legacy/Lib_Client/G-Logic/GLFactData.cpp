#include "stdafx.h"
#include "GLFactData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void SSKILLFACT::RESET ()
{
	sNATIVEID  = NATIVEID_NULL();
	wLEVEL     = 0;
	fAGE       = 0.0f;
	emTYPE	   = SKILL::EMFOR_VARHP;
	fMVAR	   = 0.0f;
	dwSpecialSkill = 0;
	bRanderSpecialEffect = FALSE;

	for( int i=0; i<SKILL::MAX_IMPACT; ++i )
	{
		sImpacts[i].emADDON    = EMIMPACTA_NONE;
		sImpacts[i].fADDON_VAR = 0.0f;
	}

	for( int i=0; i<SKILL::MAX_SPEC; ++i )
	{
		sSpecs[i].emSPEC	 = EMSPECA_NULL;
		sSpecs[i].fSPECVAR1  = 0;
		sSpecs[i].fSPECVAR2  = 0;
		sSpecs[i].dwSPECFLAG = NULL;
		sSpecs[i].dwNativeID = NATIVEID_NULL();
	}
}
