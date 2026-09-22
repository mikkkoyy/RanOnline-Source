#include "stdafx.h"
#include "GLItemVehicle.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ITEM
{
	BOOL SVEHICLE::LOAD( basestream &SFile )	// VehicleData
	{
		DWORD dwVer( 0 ), dwSize( 0 );
		SFile >> dwVer;
		SFile >> dwSize;

		switch( dwVer )
		{
		case VERSION:
			{
				int nType ( 0 );
				SFile >> nType;	emVehicleType = static_cast<VEHICLE_TYPE>(nType);
				SFile >> nType;	emVehicleSubType = static_cast<VEHICLE_SUBTYPE>(nType);

				/* vehicle no accessory, Juver, 2018/02/14 */
				SFile >> bNoAcc;	

				/*bike color , Juver, 2017/11/13 */
				SFile >> wDefaultColor1;
				SFile >> wDefaultColor2;
			}break;

		case 0x0103:
			{
				int nType ( 0 );
				SFile >> nType;	emVehicleType = static_cast<VEHICLE_TYPE>(nType);
				SFile >> nType;	emVehicleSubType = static_cast<VEHICLE_SUBTYPE>(nType);

				/* vehicle no accessory, Juver, 2018/02/14 */
				SFile >> bNoAcc;
			}break;

		case 0x0102:
			{
				int nType ( 0 );
				SFile >> nType;	emVehicleType = static_cast<VEHICLE_TYPE>(nType);
				SFile >> nType;	emVehicleSubType = static_cast<VEHICLE_SUBTYPE>(nType);
			}break;

		case 0x0101:
		case 0x0100:
			{
				int nType ( 0 );
				SFile >> nType;	emVehicleType = static_cast<VEHICLE_TYPE>(nType);
			}break;

		default:
			{
				CDebugSet::ErrorVersion( "ITEM::SVEHICLE", dwVer );
				SFile.SetOffSet( SFile.GetfTell() + dwSize );
				return FALSE;
			}break;
		}

		return TRUE;
	}

	BOOL SVEHICLE::SAVE( CSerialFile &SFile )	// VehicleData
	{
		SFile << static_cast<int>( ITEM::SVEHICLE::VERSION );
		SFile.BeginBlock(); // 구조체 사이즈를 저장한다.
		{
			SFile << (DWORD)emVehicleType;
			SFile << (DWORD)emVehicleSubType;

			/* vehicle no accessory, Juver, 2018/02/14 */
			SFile << bNoAcc;	

			/*bike color , Juver, 2017/11/13 */
			SFile << wDefaultColor1;
			SFile << wDefaultColor2;
		}
		SFile.EndBlock();

		return TRUE;
	}

	VOID SVEHICLE::SaveCsvHead ( std::fstream &SFile )	// VehicleData
	{
		SFile << "emVehicleType" << ",";
		SFile << "emVehicleSubType" << ",";

		/* vehicle no accessory, Juver, 2018/02/14 */
		SFile << "bNoAcc" << ",";

		/*bike color , Juver, 2017/11/13 */
		SFile << "wDefaultColor1" << ",";
		SFile << "wDefaultColor2" << ",";
	}

	VOID SVEHICLE::SaveCsv ( std::fstream &SFile )	// VehicleData
	{
		SFile << emVehicleType << ",";
		SFile << emVehicleSubType << ",";

		/* vehicle no accessory, Juver, 2018/02/14 */
		SFile << bNoAcc << ",";	

		/*bike color , Juver, 2017/11/13 */
		SFile << wDefaultColor1 << ",";
		SFile << wDefaultColor2 << ",";
	}

	VOID SVEHICLE::LoadCsv ( CStringArray &StrArray, int &iCsvCur )	// VehicleData
	{
		emVehicleType = (VEHICLE_TYPE)atoi( StrArray[ iCsvCur++ ] );
		emVehicleSubType = (VEHICLE_SUBTYPE)atoi( StrArray[ iCsvCur++ ] );

		/* vehicle no accessory, Juver, 2018/02/14 */
		bNoAcc = (BOOL)atol( StrArray[ iCsvCur++ ] );	

		/*bike color , Juver, 2017/11/13 */
		wDefaultColor1 = (WORD)atoi( StrArray[ iCsvCur++ ] );
		wDefaultColor2 = (WORD)atoi( StrArray[ iCsvCur++ ] );
	}
};

namespace COMMENT
{
	std::string ITEMSLOT_VEHICLE[VEHICLE_ACCETYPE_SIZE] =
	{
		"Vehicle Skin",
		"Vehicle Parts-A",
		"Vehicle Parts-B",
		"Vehicle Parts-C",
		"Vehicle Parts-D",
		"Vehicle Parts-E",
		"Vehicle Parts-F",
	};

	/*bike color , Juver, 2017/11/13 */
	std::string BIKE_COLOR_SLOT[BIKE_COLOR_SLOT_PART_SIZE] =
	{
		"Part A Color 1",
		"Part A Color 2",
		"Part B Color 1",
		"Part B Color 2",
		"Part C Color 1",
		"Part C Color 2",
		"Part D Color 1",
		"Part D Color 2",
		"Part E Color 1",
		"Part E Color 2",
		"Part F Color 1",
		"Part F Color 2",
	};
};