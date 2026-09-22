#ifndef NEOEASYCRYPTION_TEST_
#define NEOEASYCRYPTION_TEST_

#pragma once

#define	HEADER_SIZE		128

const char EnPacketMap[HEADER_SIZE] = "Land.Man";
const char EnPacketMap0[HEADER_SIZE] = "Default_Crypt";
const char defaultMap[HEADER_SIZE] = "LAND.MAN";
const char defaultMap0[HEADER_SIZE] = "default";

namespace CWLDCrypt
{

	enum 
	{
		WLD_XOR_DATA = 0x99701AE,
		WLD_XOR_DATA2 = 0x92617BE,
	};

	//wld crypt
	void Encryption_WLD(BYTE* B,int dwSize);
	void Decryption_WLD(BYTE* B,int dwSize, bool bTool = false);

	void Encryption_WLD0(BYTE* B,int dwSize);
	void Decryption_WLD0(BYTE* B,int dwSize, bool bTool = false);

	void LoadSetting( const TCHAR *szRootPath );
};

#endif
