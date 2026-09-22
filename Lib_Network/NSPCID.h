/*!
 * \file NSPCID.h
 *
 * \author Juver
 * \date May 2017
 *
 * 
 */

#pragma once

#include <IPTypes.h>
#include <IPHlpApi.h>
#pragma comment(lib, "iphlpapi.lib")

namespace _NSPCID
{
	USHORT _getVolumeHash();
	/*USHORT _getCpuHash();*/

	std::string IDStringGet();
};