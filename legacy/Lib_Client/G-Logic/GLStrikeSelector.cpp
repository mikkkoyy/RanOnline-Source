#include "StdAfx.h"
#include "GLStrikeSelector.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace STRIKE_SELECT
{
	BOOL	SelectStrike( DxSkinChar* pSKIN, D3DXVECTOR3 vPOS, D3DXVECTOR3 &vStrike, const SANIMSTRIKE &sStrikeEff )
	{
		if ( !pSKIN )	return FALSE;

		BOOL bFOUND = FALSE;

		STRIKE::SSTRIKE sStrike;

		if ( sStrikeEff.m_emPiece == PIECE_RHAND || sStrikeEff.m_emPiece == PIECE_LHAND )
		{
			if ( !bFOUND && ( pSKIN->m_pAttBone[ PIECE_RHAND ] && pSKIN->m_pAttBone[ PIECE_LHAND ] ) )
			{
				STRIKE::CSELECTOR Selector ( vPOS, pSKIN->m_pAttBone[ sStrikeEff.m_emPiece ]->m_PartArray );
				bFOUND = Selector.SELECT ( sStrikeEff, sStrike, pSKIN->m_pAttBone[ sStrikeEff.m_emPiece ]->m_emPieceStrike );
				if ( !bFOUND ) bFOUND = Selector.SELECT ( sStrikeEff, sStrike );
			}

			if ( !bFOUND && pSKIN->m_pAttBone[ PIECE_RHAND ] )
			{
				STRIKE::CSELECTOR Selector ( vPOS, pSKIN->m_pAttBone[ PIECE_RHAND ]->m_PartArray );
				bFOUND = Selector.SELECT ( sStrikeEff, sStrike, pSKIN->m_pAttBone[ PIECE_RHAND ]->m_emPieceStrike );
				if ( !bFOUND ) bFOUND = Selector.SELECT ( sStrikeEff, sStrike );
			}

			if ( !bFOUND && pSKIN->m_pAttBone[ PIECE_LHAND ] )
			{
				STRIKE::CSELECTOR Selector ( vPOS, pSKIN->m_pAttBone[ PIECE_LHAND ]->m_PartArray );
				bFOUND = Selector.SELECT ( sStrikeEff, sStrike, pSKIN->m_pAttBone[ PIECE_LHAND ]->m_emPieceStrike );
				if ( !bFOUND ) bFOUND = Selector.SELECT ( sStrikeEff, sStrike );
			}

			if ( !bFOUND )
			{
				STRIKE::CSELECTOR Selector ( vPOS, pSKIN->m_PartArray );

				PDXCHARPART pRHANDPART = pSKIN->GetPiece( PIECE_RHAND );
				PDXCHARPART pLHANDPART = pSKIN->GetPiece( PIECE_LHAND );

				if ( pRHANDPART && pLHANDPART )
				{
					bFOUND = Selector.SELECT ( sStrikeEff, sStrike );
				}

				if ( !bFOUND ) bFOUND = Selector.SELECT ( sStrikeEff, sStrike, PIECE_LHAND );
				if ( !bFOUND ) bFOUND = Selector.SELECT ( sStrikeEff, sStrike, PIECE_RHAND );
				if ( !bFOUND ) bFOUND = Selector.SELECT ( sStrikeEff, sStrike );
			}
		}
		else
		{
			if ( !bFOUND && pSKIN->m_pAttBone[ sStrikeEff.m_emPiece ] )
			{
				STRIKE::CSELECTOR Selector ( vPOS, pSKIN->m_pAttBone[ sStrikeEff.m_emPiece ]->m_PartArray );
				bFOUND = Selector.SELECT ( sStrikeEff, sStrike, pSKIN->m_pAttBone[ sStrikeEff.m_emPiece ]->m_emPieceStrike );
				if ( !bFOUND ) bFOUND = Selector.SELECT ( sStrikeEff, sStrike );
			}

			if ( !bFOUND )
			{
				STRIKE::CSELECTOR Selector ( vPOS, pSKIN->m_PartArray );
				bFOUND = Selector.SELECT ( sStrikeEff, sStrike );
			}
		}

		if ( bFOUND )
		{
			vStrike = sStrike.vPos;
		}
		else
		{
			CDebugSet::ToFileWithTime( "StrikeSelect.txt", "File %s no strike", pSKIN->m_szFileName );
		}

		return bFOUND;
	}


	BOOL	SelectArrow( DxSkinChar* pSKIN, D3DXVECTOR3 vPOS, D3DXVECTOR3 &vArrow )
	{
		if ( !pSKIN )	return FALSE;

		BOOL bFOUND = FALSE;
		IMPACT_ARROW::SSTRIKE sStrike;
		IMPACT_ARROW::CSELECTOR Selector ( vPOS, pSKIN->m_PartArray );
		bFOUND = Selector.SELECT ( sStrike );

		if ( !bFOUND )
		{
			for ( int i=0; i<PIECE_SIZE; ++i )
			{
				if ( !pSKIN->m_pAttBone[i] )	continue;

				IMPACT_ARROW::CSELECTOR SelectorAbl ( vPOS, pSKIN->m_pAttBone[i]->m_PartArray );
				bFOUND = SelectorAbl.SELECT ( sStrike );
				if ( bFOUND )
					break;
			}
		}
		
		if ( bFOUND )
		{
			vArrow = sStrike.vPos;
		}
		else
		{
			CDebugSet::ToFileWithTime( "StrikeSelect.txt", "File %s no arrow", pSKIN->m_szFileName );
		}

		return bFOUND;
	}
};