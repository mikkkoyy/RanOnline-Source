#ifndef STRIKE_SELECT_H_
#define STRIKE_SELECT_H_

/*!
 * \file GLStrikeSelector.h
 *
 * \author Juver
 * \date June 2017
 * part of abl system
 * 
 */

#include "DxSkinChar.h"
#include "DxAttBone.h"
#include "./GLStrikeM.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

using namespace STRIKE;

namespace STRIKE_SELECT
{
	BOOL	SelectStrike( DxSkinChar* pSKIN, D3DXVECTOR3 vPOS, D3DXVECTOR3 &vStrike, const SANIMSTRIKE &sStrikeEff );
	BOOL	SelectArrow( DxSkinChar* pSKIN, D3DXVECTOR3 vPOS, D3DXVECTOR3 &vArrow);
};


#endif // STRIKE_SELECT_H_