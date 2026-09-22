#include "stdafx.h"
#include "NSPCID.h"
#include "Hash/MD52.h"
//#include <intrin.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define INFO_UINT_SIZE 10
#define INFO_MACSTRING_SIZE 14
#define INFO_BUFFER_SIZE 32767

namespace _NSPCID
{
	USHORT  _getVolumeHash()
	{
		DWORD serialNum = 0;
		GetVolumeInformation( "c:\\", NULL, 0, &serialNum, NULL, NULL, NULL, 0 );
		USHORT hash = (USHORT)(( serialNum + ( serialNum >> 16 )) & 0xFFFF );
		return hash;
	};

	/*USHORT _getCpuHash()
	{
		int cpuinfo[4] = { 0, 0, 0, 0 };
		__cpuid( cpuinfo, 0 );
		USHORT hash = 0;
		USHORT* ptr = (USHORT*)(&cpuinfo[0]);
		for ( int i = 0; i < 8; i++ )
			hash += ptr[i];

		return hash;
	};*/

	std::string IDStringGet()
	{
		TCHAR szINFO[INFO_BUFFER_SIZE] = {0};

		//build string from network mac
		IP_ADAPTER_INFO AdapterInfo[18];
		DWORD dwBufLen = sizeof(AdapterInfo);
		DWORD dwStatus = GetAdaptersInfo( AdapterInfo, &dwBufLen );
		if(dwStatus == NO_ERROR)
		{
			PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
			while( pAdapterInfo )
			{
				TCHAR szTemp[INFO_MACSTRING_SIZE] = {0};
				sprintf( szTemp, "%02X%02X%02X%02X%02X%02X", 
				pAdapterInfo->Address[0], 
				pAdapterInfo->Address[1], 
				pAdapterInfo->Address[2], 
				pAdapterInfo->Address[3], 
				pAdapterInfo->Address[4], 
				pAdapterInfo->Address[5]);

				strcat( szINFO, szTemp );
				pAdapterInfo = pAdapterInfo->Next;
			}
		}


		DWORD  dwbufCharCount = INFO_BUFFER_SIZE;
		TCHAR  szinfoBuf[INFO_BUFFER_SIZE];

		//build string from computer name
		dwbufCharCount = INFO_BUFFER_SIZE;
		if( GetComputerName( szinfoBuf, &dwbufCharCount ) ){
			strcat( szINFO, szinfoBuf );
		}
		 
		//build string from volume serial
		TCHAR szvolumeSerial[INFO_UINT_SIZE] = {0};
		sprintf( szvolumeSerial, "%x", _getVolumeHash() );
		strcat( szINFO, szvolumeSerial );

		////build string from cpuid
		//TCHAR szcpuID[INFO_UINT_SIZE] = {0};
		//sprintf( szcpuID, "%x", _getCpuHash() );
		//strcat( szINFO, szcpuID );
	
		//convert string to md5
		std::string strHash = md5( szINFO, INFO_BUFFER_SIZE );
		return strHash;
	};
};