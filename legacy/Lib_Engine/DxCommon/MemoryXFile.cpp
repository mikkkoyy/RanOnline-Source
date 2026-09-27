#include "stdafx.h"
#include "MemoryXFile.h"
#include "DebugSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CMemoryXFile::CMemoryXFile()
{
	m_Buffer = NULL;
	m_nSize = 0;
}

CMemoryXFile::~CMemoryXFile()
{
	if (m_Buffer)	SAFE_DELETE_ARRAY( m_Buffer );
}

bool CMemoryXFile::LoadFileDec( const char* szFileName )
{
	FILE* pFile = NULL;
	pFile = fopen( szFileName, "rb" );
	if ( !pFile )
	{
		CDebugSet::ToLogFile( "Skin File Open Error", szFileName );	
		return false;
	}

	if ( !DecryptSkin( pFile ) )
	{
		fclose ( pFile );
		CDebugSet::ToLogFile( "Skin File Decrypt Error", szFileName );	
		return false;
	}

	fclose ( pFile );
	
	return true;
}

bool CMemoryXFile::SaveFileEnc( const char* szFileName )
{
	FILE* pFile = NULL;
	pFile = fopen( szFileName, "rb" );
	if ( !pFile )
	{
		CDebugSet::ToLogFile( "Skin File Open Error", szFileName );	
		return false;
	}

	if ( !EncryptSkin( pFile, szFileName ) )
	{
		fclose ( pFile );
		CDebugSet::ToLogFile( "Skin File Decrypt Error", szFileName );	
		return false;
	}

	fclose ( pFile );
	
	return true;
}

bool CMemoryXFile::DecryptSkin( FILE* pFile )
{
	fseek ( pFile, 0, SEEK_END );
	int nSize = ftell ( pFile );

	fseek ( pFile, 0, SEEK_SET );	
	
	int nVersion = 0;
	int nReadSize = 0;
	int nFileType = 0;
	
	fread( &nVersion, sizeof(int), 1, pFile );
    fread( &nReadSize, sizeof(int), 1, pFile );
	fread( &nFileType, sizeof(int), 1, pFile );

	if ( nVersion != SKIN_VERSION || nSize != nReadSize + SKIN_HEADER_SIZE )
	{
		return false;
	}	
	
	m_nSize = nReadSize;
	
	m_Buffer = new BYTE[nReadSize];
	fread( m_Buffer, sizeof(BYTE), nReadSize, pFile );
	
	for ( int i = 0; i < nReadSize; ++i )
	{
		m_Buffer[i] += SKIN_DIFF_DATA;
		m_Buffer[i] = m_Buffer[i] ^ SKIN_XOR_DATA;
	}

	return true;
}

bool CMemoryXFile::EncryptSkin( FILE* pFile, const char* szFileName )
{
	fseek ( pFile, 0, SEEK_END );
	int nSize = ftell ( pFile );

	fseek ( pFile, 0, SEEK_SET );	
	
	BYTE* pBuffer = new BYTE[SKIN_HEADER_SIZE+nSize];

	int nVersion = SKIN_VERSION;
	int nFileType = 0;

	CString strFileExt = szFileName;

	strFileExt = strFileExt.Right( strFileExt.GetLength() - strFileExt.ReverseFind( '.' ) );
	strFileExt.MakeLower();

	memcpy( pBuffer, &nVersion, sizeof( int ) );
	memcpy( pBuffer+4, &nSize, sizeof(int) );
	memcpy( pBuffer+8, &nFileType, sizeof(int) );	
	
	fread( pBuffer+SKIN_HEADER_SIZE, sizeof(BYTE), nSize, pFile );
	fclose( pFile );

	//DeleteFile( szFileName );

	for ( int i = 0; i < nSize; ++i )
	{
		pBuffer[SKIN_HEADER_SIZE+i] = pBuffer[SKIN_HEADER_SIZE+i] ^ SKIN_XOR_DATA;
		pBuffer[SKIN_HEADER_SIZE+i] -= SKIN_DIFF_DATA;
	}

	CString strOutFile = szFileName;
	strOutFile = strOutFile.Left( strOutFile.ReverseFind('.'));
	strOutFile += ".mxf";

	FILE* pFileOutput = NULL;
	pFileOutput = fopen( strOutFile, "wb" );
	if ( !pFileOutput )
	{
		delete[] pBuffer;
		return FALSE;
	}
	
	fwrite( pBuffer, sizeof(BYTE), nSize+SKIN_HEADER_SIZE, pFileOutput );
	fclose( pFileOutput );

	delete[] pBuffer;
	return TRUE;
}

bool CMemoryXFile::DecryptSkinToTile( FILE* pFile, const char* szFileName )
{
	if ( !pFile )	return FALSE;

	bool bDEC = DecryptSkin( pFile );

	if ( !bDEC )	return FALSE;
	fclose( pFile );

	//DeleteFile( szFileName );

	CString strOutFile = szFileName;
	strOutFile = strOutFile.Left( strOutFile.ReverseFind( '.' ) );
	strOutFile += ".x";

	FILE* pFileOutput = NULL;
	pFileOutput = fopen( strOutFile, "wb" );
	if ( !pFileOutput )	return FALSE;

	fwrite( GetData(), sizeof(BYTE), GetSize(), pFileOutput );
	fclose( pFileOutput );

	return true;
}