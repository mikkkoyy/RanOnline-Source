/*!
 * \file MemoryTexture.h
 *
 * \author Juver
 * \date August 2017
 *
 * 
 */


class CMemoryTexture
{
public:
	CMemoryTexture();
	~CMemoryTexture();

public:
	enum 
	{
		TEXTURE_DDS		= 0,
		TEXTURE_TGA		= 1,
		TEXTURE_BMP		= 2,
		TEXTURE_TYPE	= 3,
	};

	enum 
	{
		TEX_HEADER_SIZE = 12,
		TEX_VERSION		= 0x100,
		TEX_XOR_DATA	= 0x26,
		TEX_DIFF_DATA	= 0x09,
	};

	static char m_szFileExt[TEXTURE_TYPE][MAX_PATH];

public:
	BYTE* m_Buffer;
	int m_nSize;

public:
	bool LoadFileDec( const char* szFileName );
	bool SaveFileEnc( const char* szFileName );
	bool DecryptTexture( FILE* pFile, int&nFileType );
	bool EncryptTexture( FILE* pFile, const char* szFileName );
	bool DecryptTextureToFile( FILE* pFile, const char* szFileName );
	BYTE* GetData() { return m_Buffer; }
	int	GetSize() { return m_nSize; }
};