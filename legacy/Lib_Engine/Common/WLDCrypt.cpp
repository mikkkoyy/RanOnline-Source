#include "stdafx.h"
#include "WLDCrypt.h"
#include "SUBPATH.h"
#include "GLOGIC.h"
#include "IniLoader.h"
#include "GLTexFile.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace CWLDCrypt
{
	//wld crypt
	void Encryption_WLD(BYTE *B ,int dwSize)
	{
		for( int i = 0 ; i < dwSize ; i ++ )
		{
			if ( i < 128 )
			{
				B[i] = EnPacketMap[i];
			}else if ( i > 131 ) {
				B[i] = B[i] ^ WLD_XOR_DATA;
				B[i] = B[i] ^ WLD_XOR_DATA2;
				B[i] += ( WLD_XOR_DATA ^ WLD_XOR_DATA2 );
			}
		};

		return ;
	}
	void Decryption_WLD(BYTE *B,int dwSize, bool bTool)
	{
		for( int i = 0 ; i < dwSize ; i ++ )
		{
			if ( !bTool ) 
			{
				B[i] -= ( WLD_XOR_DATA ^ WLD_XOR_DATA2 );
				B[i] = B[i] ^ WLD_XOR_DATA;
				B[i] = B[i] ^ WLD_XOR_DATA2;
			}
			else if ( i < 128 )
			{
				B[i] = defaultMap[i];
			}else if ( i > 131 ) {
				B[i] -= ( WLD_XOR_DATA ^ WLD_XOR_DATA2 );
				B[i] = B[i] ^ WLD_XOR_DATA;
				B[i] = B[i] ^ WLD_XOR_DATA2;
			}
		};
		return ;
	}
	void Encryption_WLD0(BYTE *B ,int dwSize)
	{
		for( int i = 0 ; i < dwSize ; i ++ )
		{
			if ( i < 128 )
			{
				B[i] = EnPacketMap0[i];
			}else if ( i > 131 ) {
				B[i] = B[i] ^ WLD_XOR_DATA;
				B[i] = B[i] ^ WLD_XOR_DATA2;
				B[i] += ( WLD_XOR_DATA ^ WLD_XOR_DATA2 );
			}
		};

		return ;
	}
	void Decryption_WLD0(BYTE *B,int dwSize, bool bTool)
	{
		for( int i = 0 ; i < dwSize ; i ++ )
		{
			if( !bTool )
			{
				B[i] -= ( WLD_XOR_DATA ^ WLD_XOR_DATA2 );
				B[i] = B[i] ^ WLD_XOR_DATA;
				B[i] = B[i] ^ WLD_XOR_DATA2;
			}
			else if ( i < 128 )
			{
				B[i] = defaultMap0[i];
			}else if ( i > 131 ) {
				B[i] -= ( WLD_XOR_DATA ^ WLD_XOR_DATA2 );
				B[i] = B[i] ^ WLD_XOR_DATA;
				B[i] = B[i] ^ WLD_XOR_DATA2;		
			}
		};
		return ;
	}
};
