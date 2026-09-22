#include "stdafx.h"
#include "GLItemSuit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ITEM
{
	GLITEM_ATT assign_att ( glold_102::GLITEM_ATT_102 emOld )
	{
		GLITEM_ATT emAttack;
		switch ( emOld )
		{
		case glold_102::ITEMATT_NOTHING:	emAttack = ITEMATT_NOTHING;	break;
		case glold_102::ITEMATT_SWORD:		emAttack = ITEMATT_SWORD;	break;
		case glold_102::ITEMATT_REV00:		emAttack = ITEMATT_SWORD;	break;
		case glold_102::ITEMATT_DAGGER:		emAttack = ITEMATT_DAGGER;	break;
		case glold_102::ITEMATT_SPEAR:		emAttack = ITEMATT_SPEAR;	break;
		case glold_102::ITEMATT_BOW:		emAttack = ITEMATT_BOW;		break;
		case glold_102::ITEMATT_THROW:		emAttack = ITEMATT_THROW;	break;
		case glold_102::ITEMATT_NOCARE:		emAttack = ITEMATT_NOCARE;	break;
		default:	emAttack = ITEMATT_NOTHING;	break;
		};

		return emAttack;
	}

	GLITEM_ATT assign_att ( glold_103::GLITEM_ATT_103 emOld )
	{
		GLITEM_ATT emAttack;
		switch ( emOld )
		{
		case glold_103::ITEMATT_NOTHING:	emAttack = ITEMATT_NOTHING;	break;
		case glold_103::ITEMATT_SWORD:		emAttack = ITEMATT_SWORD;	break;
		case glold_103::ITEMATT_BLADE:		emAttack = ITEMATT_BLADE;	break;
		case glold_103::ITEMATT_DAGGER:		emAttack = ITEMATT_DAGGER;	break;
		case glold_103::ITEMATT_SPEAR:		emAttack = ITEMATT_SPEAR;	break;
		case glold_103::ITEMATT_STICK:		emAttack = ITEMATT_STICK;	break;
		case glold_103::ITEMATT_BOW:		emAttack = ITEMATT_BOW;		break;
		case glold_103::ITEMATT_THROW:		emAttack = ITEMATT_THROW;	break;
		case glold_103::ITEMATT_NOCARE:		emAttack = ITEMATT_NOCARE;	break;
		default:	emAttack = ITEMATT_NOTHING;		break;
		};

		return emAttack;
	}

	GLITEM_ATT assign_att ( glold_108::GLITEM_ATT_108 emOld )
	{
		GLITEM_ATT emAttack;
		switch ( emOld )
		{
		case glold_108::ITEMATT_NOTHING:	emAttack = ITEMATT_NOTHING;	break;
		case glold_108::ITEMATT_SWORD:		emAttack = ITEMATT_SWORD;	break;
		case glold_108::ITEMATT_BLADE:		emAttack = ITEMATT_BLADE;	break;
		case glold_108::ITEMATT_DAGGER:		emAttack = ITEMATT_DAGGER;	break;
		case glold_108::ITEMATT_SPEAR:		emAttack = ITEMATT_SPEAR;	break;
		case glold_108::ITEMATT_STICK:		emAttack = ITEMATT_STICK;	break;
		case glold_108::ITEMATT_GAUNT:		emAttack = ITEMATT_GAUNT;	break;
		case glold_108::ITEMATT_BOW:		emAttack = ITEMATT_BOW;		break;
		case glold_108::ITEMATT_THROW:		emAttack = ITEMATT_THROW;	break;
		case glold_108::ITEMATT_NOCARE:		emAttack = ITEMATT_NOCARE;	break;

		default:
			emAttack = ITEMATT_NOTHING;
			break;
		};

		return emAttack;
	}

	void SSUIT::Assign ( SSUIT_100 &Suit100 )
	{
		emSuit			= Suit100.emSuit;
		SETBOTHHAND ( cast_bool(Suit100.bBothHand) );
		SETBIG ( false );
		emHand			= Suit100.emHand;

		emAttack		= assign_att ( Suit100.emAttack );

		wAttRange		= Suit100.wAttRange;

		nHitRate		= Suit100.nHitRate;
		nAvoidRate		= Suit100.nAvoidRate;

		gdDamage		= Suit100.gdDamage;
		nDefense		= Suit100.nDefense;

		sResist			= Suit100.sResist;

		wReModelNum		= Suit100.wReModelNum;

		//	특수 부가 효과들.
		memcpy( sADDON, Suit100.sADDON, sizeof(SADDON)*ADDON_SIZE);
		memcpy( sADDON, Suit100.sVARIATE, sizeof(SVAR)*VARIATION_SIZE);

		//sVARIATE		= Suit100.sVARIATE;
		sBLOW.emTYPE	= Suit100.sBLOW.emTYPE;
		sBLOW.fRATE		= Suit100.sBLOW.fRATE;
		sBLOW.fLIFE		= Suit100.sBLOW.fLIFE;
		sBLOW.fVAR1		= Suit100.sBLOW.fVALUE;
		sBLOW.fVAR2		= 0.0f;
	}

	void SSUIT::Assign ( SSUIT_101 &Suit101 )
	{
		emSuit			= Suit101.emSuit;

		SETBOTHHAND ( cast_bool(Suit101.bBothHand) );
		SETBIG ( false );

		emHand			= Suit101.emHand;

		emAttack		= assign_att ( Suit101.emAttack );
		if ( emAttack==glold_102::ITEMATT_REV00 )	emAttack = ITEMATT_SWORD;

		wAttRange		= Suit101.wAttRange;

		nHitRate		= Suit101.nHitRate;
		nAvoidRate		= Suit101.nAvoidRate;

		gdDamage		= Suit101.gdDamage;
		nDefense		= Suit101.nDefense;

		sResist			= Suit101.sResist;

		wReModelNum		= Suit101.wReModelNum;

		//	특수 부가 효과들.
		memcpy( sADDON, Suit101.sADDON, sizeof(SADDON)*ADDON_SIZE);
		memcpy( sVARIATE, Suit101.sVARIATE, sizeof (SVAR)*VARIATION_SIZE);

		//sVARIATE		= Suit101.sVARIATE;
		sBLOW			= Suit101.sBLOW;
	}

	void SSUIT::Assign ( SSUIT_102 &Suit102 )
	{
		emSuit			= Suit102.emSuit;
		SETBOTHHAND ( Suit102.bBothHand );
		SETBIG ( Suit102.bBig );

		emHand			= Suit102.emHand;

		emAttack		= assign_att ( Suit102.emAttack );
		wAttRange		= Suit102.wAttRange;
		wReqSP			= Suit102.wReqSP;

		nHitRate		= Suit102.nHitRate;
		nAvoidRate		= Suit102.nAvoidRate;

		gdDamage		= Suit102.gdDamage;
		nDefense		= Suit102.nDefense;

		sResist			= Suit102.sResist;

		wReModelNum		= Suit102.wReModelNum;

		for ( int i=0; i<SSUIT_102::ADDON_SIZE; ++i )		
			sADDON[i] = Suit102.sADDON[i];

		for ( int i=0; i<SSUIT_102::VARIATION_SIZE; ++i )		
			sVARIATE[i] = Suit102.sVARIATE[i];
		
		//sVARIATE		= Suit102.sVARIATE;
		sBLOW			= Suit102.sBLOW;
	}

	void SSUIT::Assign ( SSUIT_103 &Suit103 )
	{
		emSuit			= Suit103.emSuit;
		SETBOTHHAND ( Suit103.bBothHand );
		SETBIG ( Suit103.bBig );

		emHand			= Suit103.emHand;

		emAttack		= assign_att ( Suit103.emAttack );
		wAttRange		= Suit103.wAttRange;
		wReqSP			= Suit103.wReqSP;

		nHitRate		= Suit103.nHitRate;
		nAvoidRate		= Suit103.nAvoidRate;

		gdDamage		= Suit103.gdDamage;
		nDefense		= Suit103.nDefense;

		sResist			= Suit103.sResist;

		wReModelNum		= Suit103.wReModelNum;

		for ( int i=0; i<SSUIT_103::ADDON_SIZE; ++i )		
			sADDON[i] = Suit103.sADDON[i];

		for ( int i=0; i<SSUIT_103::VARIATION_SIZE; ++i )		
			sVARIATE[i] = Suit103.sVARIATE[i];

		//sVARIATE		= Suit103.sVARIATE;
		sBLOW			= Suit103.sBLOW;
	}

	void SSUIT::Assign ( SSUIT_104 &Suit104 )
	{
		emSuit = Suit104.emSuit;
		SETBOTHHAND ( Suit104.bBothHand );
		SETBIG ( Suit104.bBig );

		emHand = Suit104.emHand;

		emAttack = assign_att ( Suit104.emAttack );
		wAttRange = Suit104.wAttRange;
		wReqSP = Suit104.wReqSP;

		nHitRate = Suit104.nHitRate;
		nAvoidRate = Suit104.nAvoidRate;

		gdDamage = Suit104.gdDamage;
		nDefense = Suit104.nDefense;

		sResist.Assign ( Suit104.sResist );

		wReModelNum = Suit104.wReModelNum;

		//	특수 부가 효과들.
		for ( int i=0; i<SSUIT_104::ADDON_SIZE; ++i )		
			sADDON[i] = Suit104.sADDON[i];

		for ( int i=0; i<SSUIT_104::VARIATION_SIZE; ++i )		
			sVARIATE[i] = Suit104.sVARIATE[i];

		//sVARIATE = Suit104.sVARIATE;
		sBLOW = Suit104.sBLOW;
	}

	void SSUIT::Assign ( SSUIT_105 &Suit105 )
	{
		emSuit = Suit105.emSuit;
		SETBOTHHAND ( Suit105.bBothHand );
		SETBIG ( Suit105.bBig );

		emHand = Suit105.emHand;

		emAttack = assign_att ( Suit105.emAttack );
		wAttRange = Suit105.wAttRange;
		wReqSP = Suit105.wReqSP;

		nHitRate = Suit105.nHitRate;
		nAvoidRate = Suit105.nAvoidRate;

		gdDamage = Suit105.gdDamage;
		nDefense = Suit105.nDefense;

		sResist = Suit105.sResist;

		wReModelNum = Suit105.wReModelNum;

		for ( int i=0; i<SSUIT_105::ADDON_SIZE; ++i )		
			sADDON[i] = Suit105.sADDON[i];

		for ( int i=0; i<SSUIT_105::VARIATION_SIZE; ++i )		
			sVARIATE[i] = Suit105.sVARIATE[i];

		//sVARIATE = Suit105.sVARIATE;
		sBLOW = Suit105.sBLOW;
	}

	void SSUIT::Assign ( SSUIT_106 &sSuitOld ) // By 경대
	{
		emSuit = sSuitOld.emSuit;
		//SETBOTHHAND ( sSuitOld.bBothHand );
		//SETBIG ( sSuitOld.bBig );
		dwHAND = sSuitOld.dwHAND;

		emHand = sSuitOld.emHand;

		emAttack = assign_att ( sSuitOld.emAttack );
		wAttRange = sSuitOld.wAttRange;
		wReqSP = sSuitOld.wReqSP;

		nHitRate = sSuitOld.nHitRate;
		nAvoidRate = sSuitOld.nAvoidRate;

		gdDamage = sSuitOld.gdDamage;
		nDefense = sSuitOld.nDefense;

		sResist = sSuitOld.sResist;

		wReModelNum = sSuitOld.wReModelNum;

		for ( int i=0; i<SSUIT_106::ADDON_SIZE; ++i )		
			sADDON[i] = sSuitOld.sADDON[i];

		for ( int i=0; i<SSUIT_106::VARIATION_SIZE; ++i )		
			sVARIATE[i] = sSuitOld.sVARIATE[i];

		//sVARIATE = sSuitOld.sVARIATE;
		sBLOW = sSuitOld.sBLOW;
	}

	void SSUIT::Assign ( SSUIT_107 &sSuitOld )
	{
		emSuit = sSuitOld.emSuit;
		dwHAND = sSuitOld.dwHAND;
		emHand = sSuitOld.emHand;
		emAttack = assign_att ( sSuitOld.emAttack );
		wAttRange = sSuitOld.wAttRange;
		wReqSP = sSuitOld.wReqSP;
		nHitRate = sSuitOld.nHitRate;
		nAvoidRate = sSuitOld.nAvoidRate;
		gdDamage = sSuitOld.gdDamage;
		nDefense = sSuitOld.nDefense;
		sResist = sSuitOld.sResist;
		wReModelNum = sSuitOld.wReModelNum;

		for ( int i=0; i<SSUIT_107::ADDON_SIZE; ++i )		
			sADDON[i] = sSuitOld.sADDON[i];

		for ( int i=0; i<SSUIT_107::VARIATION_SIZE; ++i )		
			sVARIATE[i] = sSuitOld.sVARIATE[i];

		//sVARIATE = sSuitOld.sVARIATE;
		sVOLUME = sSuitOld.sVOLUME;
		sBLOW = sSuitOld.sBLOW;
	}

	void SSUIT::Assign ( SSUIT_108 &sSuitOld )
	{
		emSuit = sSuitOld.emSuit;
		dwHAND = sSuitOld.dwHAND;
		emHand = sSuitOld.emHand;
		emAttack = assign_att ( sSuitOld.emAttack );
		wAttRange = sSuitOld.wAttRange;
		wReqSP = sSuitOld.wReqSP;
		nHitRate = sSuitOld.nHitRate;
		nAvoidRate = sSuitOld.nAvoidRate;
		gdDamage = sSuitOld.gdDamage;
		nDefense = sSuitOld.nDefense;
		sResist = sSuitOld.sResist;
		wReModelNum = sSuitOld.wReModelNum;

		for ( int i=0; i<SSUIT_108::ADDON_SIZE; ++i )		
			sADDON[i] = sSuitOld.sADDON[i];

		for ( int i=0; i<SSUIT_108::VARIATION_SIZE; ++i )		
			sVARIATE[i] = sSuitOld.sVARIATE[i];

		//sVARIATE = sSuitOld.sVARIATE;
		sVOLUME = sSuitOld.sVOLUME;
		sBLOW = sSuitOld.sBLOW;
	}

	void SSUIT::Assign ( SSUIT_109 &sSuitOld )
	{
		emSuit = sSuitOld.emSuit;
		dwHAND = sSuitOld.dwHAND;
		emHand = sSuitOld.emHand;
		emAttack = sSuitOld.emAttack;
		wAttRange = sSuitOld.wAttRange;
		wReqSP = sSuitOld.wReqSP;
		nHitRate = sSuitOld.nHitRate;
		nAvoidRate = sSuitOld.nAvoidRate;
		gdDamage = sSuitOld.gdDamage;
		nDefense = sSuitOld.nDefense;
		sResist = sSuitOld.sResist;
		wReModelNum = sSuitOld.wReModelNum;

		for ( int i=0; i<SSUIT_109::ADDON_SIZE; ++i )		
			sADDON[i] = sSuitOld.sADDON[i];

		for ( int i=0; i<SSUIT_109::VARIATION_SIZE; ++i )		
			sVARIATE[i] = sSuitOld.sVARIATE[i];

		//sVARIATE = sSuitOld.sVARIATE;
		sVOLUME = sSuitOld.sVOLUME;
		sBLOW = sSuitOld.sBLOW;
	}

	void SSUIT::Assign ( SSUIT_110 &sSuitOld )
	{
		emSuit = sSuitOld.emSuit;
		dwHAND = sSuitOld.dwHAND;
		emHand = sSuitOld.emHand;
		gdDamage = sSuitOld.gdDamage;
		nDefense = sSuitOld.nDefense;
		nHitRate = sSuitOld.nHitRate;
		nAvoidRate = sSuitOld.nAvoidRate;
		sResist = sSuitOld.sResist;
		emAttack = sSuitOld.emAttack;
		wAttRange = sSuitOld.wAttRange;
		wReqSP = sSuitOld.wReqSP;
		//sVARIATE = sSuitOld.sVARIATE;
		sVOLUME = sSuitOld.sVOLUME;
		sBLOW = sSuitOld.sBLOW;
		wReModelNum = sSuitOld.wReModelNum;

		for ( int i=0; i<SSUIT_110::ADDON_SIZE; ++i )		
			sADDON[i] = sSuitOld.sADDON[i];

		for ( int i=0; i<SSUIT_110::VARIATION_SIZE; ++i )		
			sVARIATE[i] = sSuitOld.sVARIATE[i];
	}

	VOID SSUIT::SaveCsvHead ( std::fstream &SFile )
	{
		SFile << "emSuit" << ",";
		SFile << "dwHAND" << ",";
		SFile << "emHand" << ",";

		SFile << "emAttack" << ",";
		SFile << "wAttRange" << ",";
		SFile << "wReqSP" << ",";

		SFile << "nHitRate" << ",";
		SFile << "nAvoidRate" << ",";

		SFile << "gdDamage wLow" << ",";
		SFile << "gdDamage wHigh" << ",";
		SFile << "nDefense" << ",";

		SFile << "sResist nFire" << ",";
		SFile << "sResist nIce" << ",";
		SFile << "sResist nElectric" << ",";
		SFile << "sResist nPoison" << ",";
		SFile << "sResist nSpirit" << ",";

		SFile << "wReModelNum" << ",";

		for( int i=0 ;i<ADDON_SIZE; ++i )
		{
			SFile << "sADDON " << i << " emTYPE" << ",";
			SFile << "sADDON " << i << " nVALUE" << ",";
		}

		for( int i=0 ;i<VARIATION_SIZE; ++i )
		{
			SFile << "sVARIATE " << i << " emTYPE" << ",";
			SFile << "sVARIATE " << i << " fVariate" << ",";
		}


		//SFile << "sVARIATE emTYPE" << ",";
		//SFile << "sVARIATE fVariate" << ",";
		SFile << "sVOLUME emTYPE" << ",";
		SFile << "sVOLUME fVolume" << ",";
		SFile << "sBLOW emTYPE" << ",";
		SFile << "sBLOW fRATE" << ",";
		SFile << "sBLOW fLIFE" << ",";
		SFile << "sBLOW fVAR1" << ",";
		SFile << "sBLOW fVAR2" << ",";

		SFile << "dummy_var_0" << ",";
		SFile << "dummy_var_1" << ",";
		SFile << "dummy_var_2" << ",";
	}

	VOID SSUIT::SaveCsv ( std::fstream &SFile )
	{
		SFile << emSuit << ",";
		SFile << dwHAND << ",";
		SFile << emHand << ",";

		SFile << emAttack << ",";
		SFile << wAttRange << ",";
		SFile << wReqSP << ",";

		SFile << nHitRate << ",";
		SFile << nAvoidRate << ",";

		SFile << gdDamage.dwLow << ",";
		SFile << gdDamage.dwHigh << ",";
		SFile << nDefense << ",";

		SFile << sResist.nFire << ",";
		SFile << sResist.nIce << ",";
		SFile << sResist.nElectric << ",";
		SFile << sResist.nPoison << ",";
		SFile << sResist.nSpirit << ",";

		SFile << wReModelNum << ",";

		for( int i=0 ;i<ADDON_SIZE; ++i )
		{
			SFile << sADDON[i].emTYPE << ",";
			SFile << sADDON[i].nVALUE << ",";
		}

		for( int i=0 ;i<VARIATION_SIZE; ++i )
		{
			SFile << sVARIATE[i].emTYPE << ",";
			SFile << sVARIATE[i].fVariate << ",";
		}

		//SFile << sVARIATE.emTYPE << ",";
		//SFile << sVARIATE.fVariate << ",";
		SFile << sVOLUME.emTYPE << ",";
		SFile << sVOLUME.fVolume << ",";
		SFile << sBLOW.emTYPE << ",";
		SFile << sBLOW.fRATE << ",";
		SFile << sBLOW.fLIFE << ",";
		SFile << sBLOW.fVAR1 << ",";
		SFile << sBLOW.fVAR2 << ",";

		SFile << dummy_var_0 << ",";
		SFile << dummy_var_1 << ",";
		SFile << dummy_var_2 << ",";
	}

	VOID SSUIT::LoadCsv ( CStringArray &StrArray, int &iCsvCur )
	{
		emSuit = (EMSUIT)atoi( StrArray[ iCsvCur++ ] );
		dwHAND = (DWORD)atol( StrArray[ iCsvCur++ ] );
		emHand = (EMITEM_HAND)atoi( StrArray[ iCsvCur++ ] );

		emAttack = (GLITEM_ATT)atoi( StrArray[ iCsvCur++ ] );
		wAttRange = (WORD)atoi( StrArray[ iCsvCur++ ] );
		wReqSP = (WORD)atoi( StrArray[ iCsvCur++ ] );

		nHitRate = (short)atoi( StrArray[ iCsvCur++ ] );
		nAvoidRate = (short)atoi( StrArray[ iCsvCur++ ] );

		gdDamage.dwLow = (DWORD)atoi( StrArray[ iCsvCur++ ] );
		gdDamage.dwHigh = (DWORD)atoi( StrArray[ iCsvCur++ ] );
		nDefense = (short)atoi( StrArray[ iCsvCur++ ] );

		sResist.nFire = (short)atoi( StrArray[ iCsvCur++ ] );
		sResist.nIce = (short)atoi( StrArray[ iCsvCur++ ] );
		sResist.nElectric = (short)atoi( StrArray[ iCsvCur++ ] );
		sResist.nPoison = (short)atoi( StrArray[ iCsvCur++ ] );
		sResist.nSpirit = (short)atoi( StrArray[ iCsvCur++ ] );

		wReModelNum = (WORD)atoi( StrArray[ iCsvCur++ ] );

		for( int i=0 ;i<ADDON_SIZE; ++i )
		{
			sADDON[i].emTYPE = (EMITEM_ADDON)atoi( StrArray[ iCsvCur++ ] );
			sADDON[i].nVALUE = (int)atoi( StrArray[ iCsvCur++ ] );
		}

		for( int i=0 ;i<VARIATION_SIZE; ++i )
		{
			sVARIATE[i].emTYPE = (EMITEM_VAR)atoi( StrArray[ iCsvCur++ ] );
			sVARIATE[i].fVariate = (float)atof( StrArray[ iCsvCur++ ] );
		}

		//sVARIATE.emTYPE = (EMITEM_VAR)atoi( StrArray[ iCsvCur++ ] );
		//sVARIATE.fVariate = (float)atof( StrArray[ iCsvCur++ ] );
		sVOLUME.emTYPE = (EMITEM_VAR)atoi( StrArray[ iCsvCur++ ] );
		sVOLUME.fVolume = (float)atof( StrArray[ iCsvCur++ ] );
		sBLOW.emTYPE = (EMSTATE_BLOW)atoi( StrArray[ iCsvCur++ ] );
		sBLOW.fRATE = (float)atof( StrArray[ iCsvCur++ ] );
		sBLOW.fLIFE = (float)atof( StrArray[ iCsvCur++ ] );
		sBLOW.fVAR1 = (float)atof( StrArray[ iCsvCur++ ] );
		sBLOW.fVAR2 = (float)atof( StrArray[ iCsvCur++ ] );

		dummy_var_0 = (DWORD)atoi( StrArray[ iCsvCur++ ] );
		dummy_var_1 = (WORD)atoi( StrArray[ iCsvCur++ ] );
		dummy_var_2 = (DWORD)atoi( StrArray[ iCsvCur++ ] );
	}
};

namespace COMMENT
{
	std::string ADDON_NO[ITEM::SSUIT::ADDON_SIZE] =
	{
		"0",
		"1",
		"2",
		"3",
		"4",
	};

	std::string SVARON_NO[ITEM::SSUIT::VARIATION_SIZE] =
	{
		"0",
		"1",
		"2",
		"3",
		"4",
	};
};