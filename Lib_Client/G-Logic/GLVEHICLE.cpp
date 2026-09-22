#include "stdafx.h"
#include "./GLVEHICLE.h"
#include "./GLItemMan.h"
#include "./GlItemLMT.h"
#include "./GlogicData.h"
#include "./GLChar.h"
#include "./GLGaeaServer.h"

#include "../../Lib_Engine/Common/GLTexFile.h"
#include "../../Lib_Engine/G-Logic/Glogic.h"


SVEHICLEITEMINFO& SVEHICLEITEMINFO::operator= ( const SVEHICLEITEMINFO& rvalue )
{
	m_emTYPE	= rvalue.m_emTYPE;
	m_nFull		= rvalue.m_nFull;
	m_bBooster	= rvalue.m_bBooster; /*vehicle booster system, Juver, 2017/08/10 */

	for ( WORD i = 0; i < VEHICLE_ACCETYPE_SIZE; ++i )
	{
		m_PutOnItems[i] = rvalue.m_PutOnItems[i];
	}

	/*bike color , Juver, 2017/11/13 */
	for( WORD i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
		m_wColor[i] =  rvalue.m_wColor[i];

	return *this;
}

GLVEHICLE& GLVEHICLE::operator= ( const GLVEHICLE& rvalue )
{
	m_emTYPE		= rvalue.m_emTYPE;
	m_dwGUID		= rvalue.m_dwGUID;					// 탈것의 GUID
	m_dwOwner		= rvalue.m_dwOwner;					// 주인의 charID
	m_sVehicleID	= rvalue.m_sVehicleID;				// 소환을 요청한 탈것 아이템의 Mid/Sid
	m_nFull			= rvalue.m_nFull;
	m_fSpeedVol		= rvalue.m_fSpeedVol;					// 탈것의 속도 ( 파츠 부분을 합친값 )
	m_fSpeedRate	= rvalue.m_fSpeedRate;					// 탈것의 속도 ( 파츠 부분을 합친값 )
	m_sSkinID		= rvalue.m_sSkinID;
	m_bBooster		= rvalue.m_bBooster; /*vehicle booster system, Juver, 2017/08/10 */

	for ( WORD i = 0; i < VEHICLE_ACCETYPE_SIZE; ++i )
	{
		m_PutOnItems[i] = rvalue.m_PutOnItems[i];
	}

	/*bike color , Juver, 2017/11/13 */
	for( WORD i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
		m_wColor[i] =  rvalue.m_wColor[i];

	ITEM_UPDATE();

	return *this;
}

BOOL GLVEHICLE::GETPUTONITEMS_BYBUF ( CByteStream &ByteStream ) const
{
	ByteStream.ClearBuffer ();

	ByteStream << SITEMCUSTOM::VERSION;
	ByteStream << (DWORD)sizeof(SITEMCUSTOM);
	
	DWORD dwSize = (DWORD) VEHICLE_ACCETYPE_SIZE;
	ByteStream << dwSize;

	ByteStream.WriteBuffer ( (LPBYTE)&m_PutOnItems[0], (DWORD)sizeof(SITEMCUSTOM)*dwSize );

	return true;
}

/*bike color , Juver, 2017/11/13 */
BOOL GLVEHICLE::GETCOLORS_BYBUF ( CByteStream &ByteStream ) const
{
	ByteStream.ClearBuffer ();

	ByteStream << (DWORD) GLVEHICLE::COLOR_VERSION;
	ByteStream << (DWORD) BIKE_COLOR_SLOT_PART_SIZE;

	for ( DWORD i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
	{
		ByteStream << m_wColor[i];
	}

	return true;
}

/*bike color , Juver, 2017/11/13 */
BOOL GLVEHICLE::SETCOLORS_BYBUF ( CByteStream &ByteStream )
{
	if ( ByteStream.IsEmpty() )	return TRUE;

	DWORD dwVer, dwSize;

	ByteStream >> dwVer;
	ByteStream >> dwSize;

	if ( dwVer == GLVEHICLE::COLOR_VERSION )
	{
		for ( DWORD i=0; i<dwSize; ++i )
		{
			if( i >= BIKE_COLOR_SLOT_PART_SIZE )
				continue;

			ByteStream >> m_wColor[i];
		}
	}
	else
	{
		CDebugSet::ErrorVersion( "GLVEHICLE::SETCOLORS_BYBUF", dwVer );
	}
	

	return TRUE;
}

void GLVEHICLE::ASSIGN ( const GLVEHICLE& sVehicleData )
{
	m_emTYPE		= sVehicleData.m_emTYPE;
	m_dwGUID		= sVehicleData.m_dwGUID;				// 탈것의 GUID
	m_dwOwner		= sVehicleData.m_dwOwner;			// 주인의 charID
	m_sVehicleID	= sVehicleData.m_sVehicleID;		// 소환을 요청한 탈것 아이템의 Mid/Sid
	m_nFull			= sVehicleData.m_nFull;
	m_fSpeedVol		= sVehicleData.m_fSpeedVol;					// 탈것의 속도 ( 파츠 부분을 합친값 )
	m_fSpeedRate	= sVehicleData.m_fSpeedRate;				// 탈것의 속도 ( 파츠 부분을 합친값 )
	m_sSkinID		= sVehicleData.m_sSkinID;
	m_bBooster		= sVehicleData.m_bBooster; /*vehicle booster system, Juver, 2017/08/10 */

	for ( WORD i = 0; i < VEHICLE_ACCETYPE_SIZE; ++i )
	{
		m_PutOnItems[i] = sVehicleData.m_PutOnItems[i];
	}

	/*bike color , Juver, 2017/11/13 */
	for( WORD i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
		m_wColor[i] =  sVehicleData.m_wColor[i];

	ITEM_UPDATE();
}

void GLVEHICLE::RESET ()
{
	m_emTYPE =  VEHICLE_TYPE_NONE;
	m_dwGUID = UINT_MAX;
	m_dwOwner = 0;
	m_nFull = 0;
	m_sVehicleID = NATIVEID_NULL();
	m_fSpeedRate = 0.0f;
	m_fSpeedVol = 0.0f;
	m_sSkinID = NATIVEID_NULL();
	m_bActiveValue = false;
	m_bBooster = false; /*vehicle booster system, Juver, 2017/08/10 */

	for ( WORD i = 0; i < VEHICLE_ACCETYPE_SIZE; ++i )
	{
		m_PutOnItems[i].sNativeID = NATIVEID_NULL();
	}

	/*bike color , Juver, 2017/11/13 */
	for( WORD i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
		m_wColor[i] = 0;
}

bool GLVEHICLE::IncreaseFull ( const WORD volume, const BOOL bRatio)
{
	int nMaxFull = GLCONST_VEHICLE::pGLVEHICLE[m_emTYPE]->m_nFull;

	// 포만감이 가득찼으면
	if ( m_nFull >= nMaxFull ) return false;
	if ( bRatio )
	{
		m_nFull += ( nMaxFull*volume )/100;
		if ( m_nFull > nMaxFull ) m_nFull = nMaxFull;
	}
	else
	{
		m_nFull += volume;
		if ( m_nFull > nMaxFull ) m_nFull = nMaxFull;
	}

	return true;
}

SITEMCUSTOM GLVEHICLE::GetSlotitembySuittype ( EMSUIT emSuit )
{
	WORD i = (WORD)emSuit-(WORD)SUIT_VEHICLE_SKIN;
	if ( i >= VEHICLE_ACCETYPE_SIZE ) return SITEMCUSTOM ();

	return m_PutOnItems[i];
}

void GLVEHICLE::SetSlotItem ( EMSUIT emType, SITEMCUSTOM sCustomItem)
{ 
	m_PutOnItems[(WORD)emType-(WORD)SUIT_VEHICLE_SKIN] = sCustomItem; 

	ITEM_UPDATE();
}

void GLVEHICLE::ReSetSlotItem ( EMSUIT emType )
{
	WORD i = (WORD)emType-(WORD)SUIT_VEHICLE_SKIN;
	if ( i >= VEHICLE_ACCETYPE_SIZE ) return;
	m_PutOnItems[i] = SITEMCUSTOM ( NATIVEID_NULL() );

	ITEM_UPDATE();
}

BOOL GLVEHICLE::CheckSlotItem ( SNATIVEID sNativeID, EMSUIT emSUIT )
{
	if ( sNativeID==NATIVEID_NULL() )		return FALSE;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( sNativeID );
	if ( !pItem )							return FALSE;

	// 복장류 아이템이어야 한다.
	if ( pItem->sBasicOp.emItemType != ITEM_SUIT ) return FALSE;

	// 팻타입과 아이템의 팻타입이 일치해야 한다.
	if ( pItem->sVehicle.emVehicleType != m_emTYPE )		return FALSE;

	// 악세서리 슬롯이 일치해야 한다.
	if ( pItem->sSuitOp.emSuit != emSUIT )  return FALSE;

	/* vehicle no accessory, Juver, 2018/02/14 */
	SITEM* pitem_vehicle = GLItemMan::GetInstance().GetItem( m_sVehicleID );
	if ( pitem_vehicle && pitem_vehicle->sVehicle.bNoAcc )	return FALSE;

	return TRUE;
}

/*
EMPIECECHAR GLVEHICLE::GetPieceFromSlot ( VEHICLE_ACCESSORYTYPE emType )
{

	if ( emType == ACCETYPESIZE ) return PIECE_SIZE;
	if ( emType == ACCETYPEA ) return PIECE_HEAD;
	else					   return PIECE_LOBODY;
    
	return PIECE_SIZE;
}
*/


// access
BOOL GLVEHICLE::LoadFile ( const char* szFileName, VEHICLE_TYPE emType )
{
	if ( !szFileName )				  return FALSE;
	if ( strlen ( szFileName ) == 0 ) return FALSE;

	std::string strPath;
	strPath = GLOGIC::GetPath();
	strPath += szFileName;	

	gltexfile cFILE;
	cFILE.reg_sep( '\t' );
	cFILE.reg_sep( ' ' );
	cFILE.reg_sep( ',' );
	cFILE.reg_sep( '{' );
	cFILE.reg_sep( '}' );
	cFILE.reg_sep( '[' );
	cFILE.reg_sep( ']' );
	cFILE.reg_sep( '(' );
	cFILE.reg_sep( ')' );
	cFILE.reg_sep( '|' );

	if( GLOGIC::bGLOGIC_ZIPFILE ) // by 경대
		cFILE.SetZipFile( GLOGIC::strGLOGIC_ZIPFILE );

	if( !cFILE.open( strPath, true, GLOGIC::bGLOGIC_PACKFILE ) )
	{
		CDebugSet::ToLogFile( "ERROR : GLPET::LoadData (), %s", szFileName );
		return FALSE;
	}

	WORD dwVERSION = 0;
	cFILE.getflag( "VERSION", 1, 1, dwVERSION );

	cFILE.getflag( "emTYPE",	1, 1, m_emTYPE );
	cFILE.getflag( "nFull",		1, 1, m_nFull );
	cFILE.getflag( "bBooster",	1, 1, m_bBooster ); /*vehicle booster system, Juver, 2017/08/10 */

	GLCONST_VEHICLE::pGLVEHICLE[emType] = this;

	// 포만도 감소치
	cFILE.getflag( "nFullDecrement", 1, 1, GLCONST_VEHICLE::nFullDecrVehicle[emType] );
	
	return true;
}

void GLVEHICLE::UpdateTimeLmtItem ( GLChar* pChar )
{
	const CTime cTIME_CUR = CTime::GetCurrentTime();

	for ( WORD i = 0; i < VEHICLE_ACCETYPE_SIZE; ++i )
	{
		SITEMCUSTOM sVehicleItem = m_PutOnItems[i];
		if ( sVehicleItem.sNativeID == NATIVEID_NULL () ) continue;

		SITEM* pITEM = GLItemMan::GetInstance().GetItem ( sVehicleItem.sNativeID );
		if ( !pITEM )	continue;

		// 시한부 아이템
		if ( pITEM->IsTIMELMT() )
		{
			CTimeSpan cSPAN(pITEM->sDrugOp.tTIME_LMT);
			CTime cTIME_LMT(sVehicleItem.tBORNTIME);
			cTIME_LMT += cSPAN;

			if ( cTIME_CUR > cTIME_LMT )
			{
				//	시간 제한으로 아이템 삭제 로그 남김.
				GLITEMLMT::GetInstance().ReqItemRoute ( sVehicleItem, ID_CHAR, pChar->m_dwCharID, ID_CHAR, 0, EMITEM_ROUTE_DELETE, 0 );

				//	아이템 삭제.
				m_PutOnItems[i] = SITEMCUSTOM ();

				//	[자신에게] 해당 아이탬을 인밴에서 삭제.
				GLMSG::SNET_VEHICLE_ACCESSORY_DELETE NetMsg;
				NetMsg.dwVehicleNum	= m_dwGUID;
				NetMsg.accetype = (VEHICLE_ACCESSORY_TYPE)i;
				GLGaeaServer::GetInstance().SENDTOCLIENT(pChar->m_dwClientID,&NetMsg);

				// 주변에 알림.
				GLMSG::SNET_VEHICLE_ACCESSORY_DELETE_BRD NetMsgBrd;
				NetMsgBrd.dwGaeaID = pChar->m_dwGaeaID;
				NetMsgBrd.dwVehicleNum = m_dwGUID;
				NetMsgBrd.accetype = (VEHICLE_ACCESSORY_TYPE)i;
				pChar->SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&NetMsgBrd) );

				//	시간 제한으로 아이템 삭제 알림.
				GLMSG::SNET_INVEN_DEL_ITEM_TIMELMT NetMsgInvenDelTimeLmt;
				NetMsgInvenDelTimeLmt.nidITEM = sVehicleItem.sNativeID;
				GLGaeaServer::GetInstance().SENDTOCLIENT(pChar->m_dwClientID,&NetMsgInvenDelTimeLmt);
			}
		}
	}
}

void GLVEHICLE::ITEM_UPDATE()
{
    m_fSpeedRate = 0.0f;
	m_fSpeedVol = 0.0f;


	SITEM* pITEM;
//	SITEM* pITEM = GLItemMan::GetInstance().GetItem ( m_sVehicleID );
//	if ( pITEM )
//	{
//		if ( pITEM->sSuitOp.sVARIATE.emTYPE == EMVAR_MOVE_SPEED )	m_fSpeedRate += pITEM->sSuitOp.sVARIATE.fVariate;
//		if ( pITEM->sSuitOp.sVOLUME.emTYPE == EMVAR_MOVE_SPEED )	m_fSpeedVol += pITEM->sSuitOp.sVOLUME.fVolume;		
//	}
	/* vehicle no accessory, Juver, 2018/02/14 */
	SITEM* pitem_vehicle = GLItemMan::GetInstance().GetItem( m_sVehicleID );
	if ( pitem_vehicle && !pitem_vehicle->sVehicle.bNoAcc )
	{
		for ( int i = 0; i < VEHICLE_ACCETYPE_SIZE; ++i )
		{
			if ( m_PutOnItems[i].sNativeID == NATIVEID_NULL() ) continue;
			pITEM = GLItemMan::GetInstance().GetItem ( m_PutOnItems[i].sNativeID );
			if ( !pITEM ) continue;

			//if ( pITEM->sSuitOp.sVARIATE.emTYPE == EMVAR_MOVE_SPEED )
			//{
			//	m_fSpeedRate += pITEM->sSuitOp.sVARIATE.fVariate;
			//}

			for ( DWORD svaron=0; svaron<ITEM::SSUIT::VARIATION_SIZE; ++svaron )
			{
				if ( pITEM->sSuitOp.sVARIATE[svaron].emTYPE == EMVAR_MOVE_SPEED )
				{
					m_fSpeedRate += pITEM->sSuitOp.sVARIATE[svaron].fVariate;
				}
			}

			if ( pITEM->sSuitOp.sVOLUME.emTYPE == EMVAR_MOVE_SPEED )
			{
				m_fSpeedVol += m_PutOnItems[i].GETMOVESPEED();
			}        				
		}
	}

	m_sSkinID = m_sVehicleID;
	
	// cps 미리 계산
	if ( m_PutOnItems[VEHICLE_ACCETYPE_SKIN].sNativeID != NATIVEID_NULL() )
	{
		m_sSkinID = m_PutOnItems[VEHICLE_ACCETYPE_SKIN].sNativeID;
	}
}

namespace COMMENT
{
	extern std::string VEHICLE_TYPE[VEHICLE_TYPE_SIZE] = 
	{ 
		"Type_A(Hover Board)",
		"Type_B(Bike)",
		"Type_C(Car)" 
		"Type_D(Cart)" 
	};

	extern std::string VEHICLE_SUBTYPE_HOVER[VEHICLE_SUBTYPE_SIZE] = 
	{ 
		"HoverBoard",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
	};

	extern std::string VEHICLE_SUBTYPE_BIKE[VEHICLE_SUBTYPE_SIZE] = 
	{ 
		"Bike Type BT-5",
		"Bike Type BT-7",
		"Bike Type BT-9",
		"Bike Type EV-1",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
	};

	/* vehicle car, cart, Juver, 2018/02/13 */
	extern std::string VEHICLE_SUBTYPE_CAR[VEHICLE_SUBTYPE_SIZE] = 
	{ 
		"Car-A",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
	};

	/* vehicle car, cart, Juver, 2018/02/13 */
	extern std::string VEHICLE_SUBTYPE_CART[VEHICLE_SUBTYPE_SIZE] = 
	{ 
		"Cart-A",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
		"None",
	};
};