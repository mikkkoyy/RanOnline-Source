/*!
 * \file GLBikeColorDef.h
 *
 * \author Juver
 * \date November 2017
 *
 * 
 */

#pragma once

#include "../../Lib_Engine/Meshs/DxPieceDefine.h"
#include "GLItemVehicle.h"

inline EMPIECECHAR BIKE_COLOR_SLOT_TO_PIECE ( BIKE_COLOR_SLOT_PART emSlot )
{
	switch( emSlot )
	{
	case BIKE_COLOR_SLOT_PART_A1:
	case BIKE_COLOR_SLOT_PART_A2:
		return PIECE_UPBODY;
		break;

	case BIKE_COLOR_SLOT_PART_B1:
	case BIKE_COLOR_SLOT_PART_B2:
		return PIECE_LOBODY;
		break;

	case BIKE_COLOR_SLOT_PART_C1:
	case BIKE_COLOR_SLOT_PART_C2:
		return PIECE_GLOVE;
		break;

	case BIKE_COLOR_SLOT_PART_D1:
	case BIKE_COLOR_SLOT_PART_D2:
		return PIECE_RHAND;
		break;

	case BIKE_COLOR_SLOT_PART_E1:
	case BIKE_COLOR_SLOT_PART_E2:
		return PIECE_LHAND;
		break;

	case BIKE_COLOR_SLOT_PART_F1:
	case BIKE_COLOR_SLOT_PART_F2:
		return PIECE_FOOT;
		break;

	default:		return PIECE_SIZE;
	}
};

inline BOOL IS_BIKECOLOR_1( BIKE_COLOR_SLOT_PART emSlot )
{
	switch( emSlot )
	{
	case BIKE_COLOR_SLOT_PART_A1:
	case BIKE_COLOR_SLOT_PART_B1:
	case BIKE_COLOR_SLOT_PART_C1:
	case BIKE_COLOR_SLOT_PART_D1:
	case BIKE_COLOR_SLOT_PART_E1:
	case BIKE_COLOR_SLOT_PART_F1:
		return TRUE;
		break;

	default:		return FALSE;
	}

	return FALSE;
};