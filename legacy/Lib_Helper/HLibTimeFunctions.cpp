#include "stdafx.h"
#include "HLibTimeFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace _HLIB
{
	BOOL timestamp_correction( TIMESTAMP_STRUCT& _tsTime )
	{
		TIMESTAMP_STRUCT tOrig = _tsTime;

		if (_tsTime.year < 1970 || _tsTime.year >= 2999){
			_tsTime.year = 1970;
			_tsTime.month = 2;
			_tsTime.day = 1;
			_tsTime.hour = 1;
			_tsTime.minute = 1;
			_tsTime.second = 1;
		}

		if (_tsTime.month <= 0 || _tsTime.month > 12){
			_tsTime.month = 2;
		}

		if (_tsTime.day <= 0 || _tsTime.day > 31){
			_tsTime.day = 1;
		}

		if (_tsTime.hour < 0 || _tsTime.hour > 23 ){
			_tsTime.hour = 1;
		}

		if (_tsTime.minute < 0 || _tsTime.minute > 59 ){
			_tsTime.minute = 1;
		}

		if (_tsTime.second < 0 || _tsTime.second > 59 ){
			_tsTime.second = 1;
		}

		if ( tOrig.year != _tsTime.year || 
			tOrig.month != _tsTime.month || 
			tOrig.day != _tsTime.day || 
			tOrig.hour != _tsTime.hour || 
			tOrig.minute != _tsTime.minute || 
			tOrig.second != _tsTime.second ){
			return FALSE;
		}	

		return TRUE;
	};

	__time64_t timestamp_timet64( TIMESTAMP_STRUCT _tsTime )
	{
		timestamp_correction( _tsTime );

		CTime cTime( _tsTime.year, _tsTime.month, _tsTime.day, 
			_tsTime.hour, _tsTime.minute, _tsTime.second );

		return cTime.GetTime();
	};
};