#include "stdafx.h"
#include "GLItemBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ITEM
{

	SBOX::SBOX ()
	{
		CLEAR ();
	}

	void SBOX::CLEAR ()
	{
		//Select Box
		m_bSelect = FALSE;
		m_nMaxSelect = 0;

		bData = false;

		for ( int i=0; i<ITEM_SIZE; ++i )
		{
			sITEMS[i].nidITEM = SNATIVEID(false);
			sITEMS[i].dwAMOUNT = 0;
		}
	}

	bool SBOX::INSERT ( const SNATIVEID &nidITEM, DWORD dwAMOUNT )
	{
		if ( nidITEM==SNATIVEID(false) )	return false;

		for ( int i=0; i<ITEM_SIZE; ++i )
		{
			if ( sITEMS[i].nidITEM==SNATIVEID(false) )
			{
				sITEMS[i].nidITEM = nidITEM;
				sITEMS[i].dwAMOUNT = dwAMOUNT;
				return true;
			}
		}

		//std::vector<DWORD>	vecTEMP;
		//for ( i=0; i<ITEM_SIZE; ++i )	vecTEMP.push_back ( sITEMS[i].dwID );
		//std::sort ( vecTEMP.begin(), vecTEMP.end() );

		//CLEAR ();
		//for ( i=0; i<ITEM_SIZE; ++i )	sITEMS[i].dwID = vecTEMP[i];

		return false;
	}

	bool SBOX::DEL ( int nIndex )
	{
		if ( nIndex < 0 )				return false;
		if ( ITEM_SIZE <= nIndex )		return false;

		sITEMS[nIndex].nidITEM = SNATIVEID(false);
		sITEMS[nIndex].dwAMOUNT = 1;

		//std::vector<DWORD>	vecTEMP;
		//for ( int i=0; i<ITEM_SIZE; ++i )	vecTEMP.push_back ( sITEMS[i].dwID );
		//std::sort ( vecTEMP.begin(), vecTEMP.end() );

		//CLEAR ();
		//for ( i=0; i<ITEM_SIZE; ++i )	sITEMS[i].dwID = vecTEMP[i];

		return true;
	}

	bool SBOX::VALID ()
	{
		for ( int i=0; i<ITEM_SIZE; ++i )
		{
			if ( sITEMS[i].nidITEM!=SNATIVEID(false) )	return true;
		}

		return false;
	}

	BOOL SBOX::LOAD ( basestream &SFile )
	{
		DWORD dwVER(0), dwSIZE(0);
		SFile >> dwVER;
		SFile >> dwSIZE;

		switch (dwVER)
		{
		case VERSION:
			{
				SFile.ReadBuffer ( sITEMS, sizeof(SBOX_ITEM)*ITEM_SIZE );
				SFile >> bData;
				//Select Box
				SFile >> m_bSelect;
				SFile >> m_nMaxSelect;
			}break;

		case 0x103:
			{
				SFile.ReadBuffer ( sITEMS, sizeof(SBOX_ITEM)*ITEM_SIZE );
				SFile >> bData;
			}
			break;

		case 0x102:
			{
				SFile.ReadBuffer ( sITEMS, sizeof(SBOX_ITEM)*ITEM_SIZE );
			}break;

		case 0x101:
			{
				SBOX_ITEM_101 _sOLD_ITEMS[ITEM_SIZE];
				SFile.ReadBuffer ( _sOLD_ITEMS, sizeof(SBOX_ITEM_101)*ITEM_SIZE );

				for ( int i=0; i<ITEM_SIZE; ++i )
				{
					sITEMS[i].nidITEM = _sOLD_ITEMS[i].nidITEM;
					sITEMS[i].dwAMOUNT = _sOLD_ITEMS[i].dwAMOUNT;
				}
			}break;

		case 0x100:
			{
				SNATIVEID _sOLD_ITEMS[ITEM_SIZE];
				SFile.ReadBuffer ( _sOLD_ITEMS, sizeof(DWORD)*ITEM_SIZE );

				for ( int i=0; i<ITEM_SIZE; ++i )
				{
					sITEMS[i].nidITEM = _sOLD_ITEMS[i];
					sITEMS[i].dwAMOUNT = 1;
				}
			}break;

		default:
			{
				CDebugSet::ErrorVersion( "ITEM::SBOX", dwVER );
				SFile.SetOffSet ( SFile.GetfTell()+dwSIZE );
			}break;
		};

		return TRUE;
	}

	BOOL SBOX::SAVE ( CSerialFile &SFile )
	{
		SFile << DWORD(VERSION);
		SFile.BeginBlock();
		{
			SFile.WriteBuffer ( sITEMS, sizeof(SBOX_ITEM)*ITEM_SIZE );
			SFile << bData;
			//Select Box
			SFile << m_bSelect;
			SFile << m_nMaxSelect;
		}
		SFile.EndBlock();

		return TRUE;
	}

	VOID SBOX::SaveCsvHead ( std::fstream &SFile )
	{
		//Select Box
		SFile << "sBOX bSelectBox" << ",";
		SFile << "sBOX nMaxSelect" << ",";

		for( int i=0; i<ITEM_SIZE; ++i )
		{
			SFile << "sITEMS " << i << " nidITEM wMainID" << ",";
			SFile << "sITEMS " << i << " nidITEM wSubID" << ",";
			SFile << "sITEMS " << i << " dwAMOUNT" << ",";
		}

		SFile << "bData" << ",";
	}

	VOID SBOX::SaveCsv ( std::fstream &SFile )
	{
		//Select Box
		SFile << m_bSelect << ",";
		SFile << m_nMaxSelect << ",";

		for( int i=0; i<ITEM_SIZE; ++i )
		{
			SFile << sITEMS[i].nidITEM.wMainID << ",";
			SFile << sITEMS[i].nidITEM.wSubID << ",";
			SFile << sITEMS[i].dwAMOUNT << ",";
		}

		SFile << bData << ",";
	}

	VOID SBOX::LoadCsv ( CStringArray &StrArray, int &iCsvCur  )
	{
		//Select Box
		m_bSelect = (BOOL)atol( StrArray[ iCsvCur++ ] );
		m_nMaxSelect = atoi( StrArray[ iCsvCur++ ] );

		for( int i=0; i<ITEM_SIZE; ++i )
		{
			sITEMS[i].nidITEM.wMainID = (WORD)atoi( StrArray[ iCsvCur++ ] );
			sITEMS[i].nidITEM.wSubID = (WORD)atoi( StrArray[ iCsvCur++ ] );
			sITEMS[i].dwAMOUNT = (DWORD)atol( StrArray[ iCsvCur++ ] );
		}

		bData = (bool)atoi( StrArray[ iCsvCur++ ] );
	}

};