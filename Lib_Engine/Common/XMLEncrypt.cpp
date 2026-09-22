#include "stdafx.h"
#include "XMLEncrypt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace NS_XMLCRYPT
{
	void Encode(BYTE* buff, int nSize)
	{
		for( int i=0; i<nSize; i++ )
		{
			buff[i] = buff[i] ^ XMLCRYPT_KEY_1;
			buff[i] += ( XMLCRYPT_KEY_1 ^ XMLCRYPT_KEY_2 );
		};
	}

	void Decode(BYTE* buff, int nSize)
	{
		for( int i=0; i<nSize; i++ )
		{
			buff[i] -= ( XMLCRYPT_KEY_1 ^ XMLCRYPT_KEY_2 );
			buff[i] = buff[i] ^ XMLCRYPT_KEY_1;
		};
	}
};
