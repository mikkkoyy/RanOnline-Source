/*!
 * \file MemoryXFile.h
 *
 * \author Juver
 * \date August 2017
 *
 * 
 */

class CMemoryXFile
{
public:
	CMemoryXFile();
	~CMemoryXFile();

	enum 
	{
		SKIN_HEADER_SIZE = 12,
		SKIN_VERSION	= 0x100,
		SKIN_XOR_DATA	= 0x92617EB,
		SKIN_DIFF_DATA	= 0x99701EA,
	};

public:
	BYTE* m_Buffer;
	int m_nSize;

public:
	bool LoadFileDec( const char* szFileName );
	bool SaveFileEnc( const char* szFileName );
	bool DecryptSkin( FILE* pFile );
	bool EncryptSkin( FILE* pFile, const char* szFileName );
	bool DecryptSkinToTile( FILE* pFile, const char* szFileName );
	BYTE* GetData() { return m_Buffer; }
	int	GetSize() { return m_nSize; }
};