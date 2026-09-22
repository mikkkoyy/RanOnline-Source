/*!
 * \file HLibTimeFunctions.h
 *
 * \author Juver
 * \date April 2017
 *
 * 
 */

#pragma once

#include "sqltypes.h"

namespace _HLIB
{	
	BOOL timestamp_correction( TIMESTAMP_STRUCT& _tsTime );
	__time64_t timestamp_timet64( TIMESTAMP_STRUCT _tsTime );
};