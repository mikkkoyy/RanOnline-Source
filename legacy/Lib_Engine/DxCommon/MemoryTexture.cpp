#include "stdafx.h"
#include "MemoryTexture.h"
#include "DebugSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


char	CMemoryTexture::m_szFileExt[TEXTURE_TYPE][MAX_PATH] =
{
	".dds",
	".tga",
	".bmp"
};

CMemoryTexture::CMemoryTexture()
{
	m_Buffer = NULL;
	m_nSize = 0;
}

CMemoryTexture::~CMemoryTexture()
{
	if (m_Buffer)	SAFE_DELETE_ARRAY( m_Buffer );
}

bool CMemoryTexture::LoadFileDec( const char* szFileName )
{
	FILE* pFile = NULL;
	pFile = fopen( szFileName, "rb" );
	if ( !pFile )
	{
		CDebugSet::ToLogFile( "Texture File Open Error", szFileName );	
		return false;
	}

	int nFileType = 0;
	if ( !DecryptTexture( pFile, nFileType ) )
	{
		fclose ( pFile );
		CDebugSet::ToLogFile( "Texture File Decrypt Error", szFileName );	
		return false;
	}

	fclose ( pFile );
	
	return true;
}

bool CMemoryTexture::SaveFileEnc( const char* szFileName )
{
	FILE* pFile = NULL;
	pFile = fopen( szFileName, "rb" );
	if ( !pFile )
	{
		CDebugSet::ToLogFile( "Texture File Open Error", szFileName );	
		return false;
	}

	if ( !EncryptTexture( pFile, szFileName ) )
	{
		fclose ( pFile );
		CDebugSet::ToLogFile( "Texture File Decrypt Error", szFileName );	
		return false;
	}

	fclose ( pFile );
	
	return true;
}

bool CMemoryTexture::DecryptTexture( FILE* pFile, int&nFileType )
{
	fseek ( pFile, 0, SEEK_END );
	int nSize = ftell ( pFile );

	fseek ( pFile, 0, SEEK_SET );	
	
	int nVersion = 0;
	int nReadSize = 0;

	fread( &nVersion, sizeof(int), 1, pFile );
    fread( &nReadSize, sizeof(int), 1, pFile );
	fread( &nFileType, sizeof(int), 1, pFile );

	if ( nVersion != TEX_VERSION || nSize != nReadSize + TEX_HEADER_SIZE )
	{
		return false;
	}	
	
	m_nSize = nReadSize;
	
	m_Buffer = new BYTE[nReadSize];
	fread( m_Buffer, sizeof(BYTE), nReadSize, pFile );
	
	for ( int i = 0; i < nReadSize; ++i )
	{
		m_Buffer[i] += TEX_DIFF_DATA;
		m_Buffer[i] = m_Buffer[i] ^ TEX_XOR_DATA;
	}

	return true;
}

bool CMemoryTexture::EncryptTexture( FILE* pFile, const char* szFileName )
{
	fseek ( pFile, 0, SEEK_END );
	int nSize = ftell ( pFile );

	fseek ( pFile, 0, SEEK_SET );	
		
	BYTE* pBuffer = new BYTE[TEX_HEADER_SIZE+nSize];

	int nVersion = TEX_VERSION;
	int nFileType = TEXTURE_DDS;

	CString strFileExt = szFileName;

	strFileExt = strFileExt.Right( strFileExt.GetLength() - strFileExt.ReverseFind( '.' ) );
	strFileExt.MakeLower();

	if ( !strcmp( strFileExt, m_szFileExt[TEXTURE_DDS] ) )	nFileType = TEXTURE_DDS;
	else if ( !strcmp( strFileExt, m_szFileExt[TEXTURE_TGA] ) ) nFileType = TEXTURE_TGA;
	else if ( !strcmp( strFileExt, m_szFileExt[TEXTURE_BMP] ) ) nFileType = TEXTURE_BMP;
	
	memcpy( pBuffer, &nVersion, sizeof( int ) );
	memcpy( pBuffer+4 , &nSize, sizeof(int) );
	memcpy( pBuffer+8, &nFileType, sizeof(int) );	
	
	fread( pBuffer+TEX_HEADER_SIZE, sizeof(BYTE), nSize, pFile );
	fclose( pFile );

	//DeleteFile( szFileName );

	for ( int i = 0; i < nSize; ++i )
	{
		pBuffer[TEX_HEADER_SIZE+i] = pBuffer[TEX_HEADER_SIZE+i] ^ TEX_XOR_DATA;
		pBuffer[TEX_HEADER_SIZE+i] -= TEX_DIFF_DATA;
	}

	CString strOutFile = szFileName;
	strOutFile = strOutFile.Left( strOutFile.ReverseFind('.'));
	strOutFile += ".mtf";

	FILE* pFileOutput = NULL;
	pFileOutput = fopen( strOutFile, "wb" );
	if ( !pFileOutput )
	{
		delete[] pBuffer;
		return FALSE;
	}
	
	fwrite( pBuffer, sizeof(BYTE), nSize+TEX_HEADER_SIZE, pFileOutput );
	fclose( pFileOutput );

	delete[] pBuffer;
	return TRUE;
}

bool CMemoryTexture::DecryptTextureToFile( FILE* pFile, const char* szFileName )
{
	if ( !pFile )	return FALSE;

	int nFileType = 0;
	bool bDEC = DecryptTexture( pFile, nFileType );

	if ( !bDEC )	return FALSE;
	fclose( pFile );

	//DeleteFile( szFileName );

	CString strOutFile = szFileName;
	strOutFile = strOutFile.Left( strOutFile.ReverseFind( '.' ) );
	strOutFile += m_szFileExt[nFileType];

	FILE* pFileOutput = NULL;
	pFileOutput = fopen( strOutFile, "wb" );
	if ( !pFileOutput )	return FALSE;

	fwrite( GetData(), sizeof(BYTE), GetSize(), pFileOutput );
	fclose( pFileOutput );

	return true;
}