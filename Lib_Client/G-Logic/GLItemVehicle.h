#ifndef GLITEM_VEHICLE_H_
#define GLITEM_VEHICLE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "../../Lib_Engine/Common/BaseStream.h"
#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/SerialFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"
#include "../../Lib_Engine/G-Logic/GLDefine.h"

enum VEHICLE_TYPE
{
	VEHICLE_TYPE_NONE		= -1,
	VEHICLE_TYPE_BOARD		= 0,
	VEHICLE_TYPE_BIKE		= 1,
	VEHICLE_TYPE_CAR		= 2,	/* vehicle car, cart, Juver, 2018/02/13 */
	VEHICLE_TYPE_CART		= 3,	/* vehicle car, cart, Juver, 2018/02/13 */
	VEHICLE_TYPE_SIZE		= 4,
};

enum VEHICLE_SUBTYPE
{
	VEHICLE_SUBTYPE_00		= 0,
	VEHICLE_SUBTYPE_01		= 1,
	VEHICLE_SUBTYPE_02		= 2,
	VEHICLE_SUBTYPE_03		= 3,
	VEHICLE_SUBTYPE_04		= 4,
	VEHICLE_SUBTYPE_05		= 5,
	VEHICLE_SUBTYPE_06		= 6,
	VEHICLE_SUBTYPE_07		= 7,
	VEHICLE_SUBTYPE_08		= 8,
	VEHICLE_SUBTYPE_09		= 9,
	VEHICLE_SUBTYPE_SIZE	= 10,
};

enum VEHICLE_ACCESSORY_TYPE
{
	VEHICLE_ACCETYPE_SKIN		= 0, // skin
	VEHICLE_ACCETYPE_PARTS_A	= 1, // parts
	VEHICLE_ACCETYPE_PARTS_B	= 2, // parts
	VEHICLE_ACCETYPE_PARTS_C	= 3, // parts
	VEHICLE_ACCETYPE_PARTS_D	= 4, // parts
	VEHICLE_ACCETYPE_PARTS_E	= 5, // parts
	VEHICLE_ACCETYPE_PARTS_F	= 6, // parts
	VEHICLE_ACCETYPE_SIZE		= 7,
};

/*bike color , Juver, 2017/11/13 */
enum BIKE_COLOR_SLOT_PART
{
	BIKE_COLOR_SLOT_PART_A1 = 0,
	BIKE_COLOR_SLOT_PART_A2 = 1,
	BIKE_COLOR_SLOT_PART_B1 = 2,
	BIKE_COLOR_SLOT_PART_B2 = 3,
	BIKE_COLOR_SLOT_PART_C1 = 4,
	BIKE_COLOR_SLOT_PART_C2 = 5,
	BIKE_COLOR_SLOT_PART_D1 = 6,
	BIKE_COLOR_SLOT_PART_D2 = 7,
	BIKE_COLOR_SLOT_PART_E1 = 8,
	BIKE_COLOR_SLOT_PART_E2 = 9,
	BIKE_COLOR_SLOT_PART_F1 = 10,
	BIKE_COLOR_SLOT_PART_F2 = 11,
	BIKE_COLOR_SLOT_PART_SIZE	= 12,
};

namespace ITEM
{
	struct SVEHICLE_100
	{
		VEHICLE_TYPE	emVehicleType; // TYPE
	};

	struct SVEHICLE	// VehicleData
	{
		enum { VERSION = 0x0200 };

		VEHICLE_TYPE	emVehicleType; // TYPE
		VEHICLE_SUBTYPE emVehicleSubType;

		/* vehicle no accessory, Juver, 2018/02/14 */
		BOOL			bNoAcc;

		/*bike color , Juver, 2017/11/13 */
		WORD			wDefaultColor1;
		WORD			wDefaultColor2;

		SVEHICLE() 
			: emVehicleType(VEHICLE_TYPE_SIZE)
			, emVehicleSubType( VEHICLE_SUBTYPE_SIZE )

			/* vehicle no accessory, Juver, 2018/02/14 */
			, bNoAcc( FALSE )

			/*bike color , Juver, 2017/11/13 */
			, wDefaultColor1(32767)
			, wDefaultColor2(32767)
		{
		}

		BOOL LOAD ( basestream &SFile );
		BOOL SAVE ( CSerialFile &SFile );

		static VOID SaveCsvHead ( std::fstream &SFile );
		VOID SaveCsv ( std::fstream &SFile );
		VOID LoadCsv ( CStringArray &StrArray, int &iCsvCur );
	};
};

namespace COMMENT
{
	extern std::string ITEMSLOT_VEHICLE[VEHICLE_ACCETYPE_SIZE];

	/*bike color , Juver, 2017/11/13 */
	extern std::string BIKE_COLOR_SLOT[BIKE_COLOR_SLOT_PART_SIZE];
};

#endif // GLITEM_VEHICLE_H_