// SerialFile.cpp: implementation of the CSerialFile class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "./SerialFile.h"
#include "./CompByte.h"
#include "./ByteCrypt.h"

#include "./WLDCrypt.h"		//wld crypt

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSerialFile::CSerialFile() :
	m_FileStream(NULL),
	m_nLastDump(0)
{
}

CSerialFile::CSerialFile ( FOPENTYPE _Type, const char *_szFileName, EMBYTECRYPT emCrypt /*= EMBYTECRYPT_NONE*/ ) :
	m_FileStream(NULL)
{
	GASSERT(_szFileName);

	OpenFile ( _Type, _szFileName, emCrypt );
}

CSerialFile::~CSerialFile()
{
	CloseFile ();
}

void CSerialFile::SetFileType( const char *szFileType, DWORD FileID )
{
	StringCchCopy( m_szFileType, FILETYPESIZE, szFileType );
	m_FileID = FileID;
}

void CSerialFile::GetFileType( char *szFileType, int nStrLen, DWORD &FileID )
{
	StringCchCopy( szFileType, nStrLen, m_szFileType );
	FileID = m_FileID;
}

void CSerialFile::ReadFileType ()
{
	GASSERT(m_OpenType==FOT_READ);
	int Num = 0;

	Num = (int)fread( &m_szFileType, sizeof( char ), FILETYPESIZE, m_FileStream );
	Num = (int)fread( &m_FileID, sizeof( DWORD ), 1, m_FileStream );

	m_DefaultOffSet = ftell ( m_FileStream );
}

void CSerialFile::WriteFileType ()
{
	GASSERT(m_OpenType==FOT_WRITE);
	int Num = 0;

	Num = (int)fwrite( &m_szFileType, sizeof( char ), FILETYPESIZE, m_FileStream );
	Num = (int)fwrite( &m_FileID, sizeof( DWORD ), 1, m_FileStream );

	m_DefaultOffSet = ftell ( m_FileStream );
}

BOOL CSerialFile::NewFile ( const char* pName )
{
	int fh;

	fh = _creat( pName, _S_IREAD | _S_IWRITE );
	if( fh == -1 )	return FALSE;	//perror( "Couldn't create data file" );
	else			
	{
		//printf( "Created data file.\n" );
		_close( fh );
		return TRUE;
	}
}

BOOL CSerialFile::OpenFile ( const FOPENTYPE _Type, const char *_szFileName, EMBYTECRYPT emCrypt /*= EMBYTECRYPT_NONE*/ )
{
	GASSERT(_szFileName);

	m_OpenType = _Type;
	m_emCrypt = emCrypt;
	StringCchCopy( m_szFileName, _MAX_FNAME, _szFileName );

	// 이미 열려 있을때 파일 닫음.
	CloseFile ();

	switch ( m_OpenType )
	{
	case FOT_READ:
		m_FileStream = fopen ( m_szFileName, "rb" );
		if ( !m_FileStream ) break;
		
		//if( m_bencode )
		//{
		//	int nVersion;
		//	fread ( &nVersion, sizeof(int), 1, m_FileStream );
		//	if( nVersion > 0 && CRijndael::VERSION >= nVersion )
		//	{
		//		m_nVersion = nVersion;
		//		m_oRijndael.Initialize( CRijndael::sm_Version[nVersion-1], CRijndael::DEFAULT_KEY_LENGTH );
		//	}
		//	else
		//	{
		//		fseek ( m_FileStream, 0, SEEK_SET );
		//	}
		//}

		ReadFileType ();
		break;

	case FOT_WRITE:
		m_FileStream = fopen ( m_szFileName, "wb" );
		if ( !m_FileStream )
		{
			if( NewFile( _szFileName ) )	// New File
			{
				m_FileStream = fopen ( m_szFileName, "wb" );
				if ( !m_FileStream )	break;
			}
			else	break;					// New File Not Create
		}

		//int nVersion = CRijndael::VERSION;
		//m_oRijndael.Initialize( CRijndael::sm_Version[nVersion-1], CRijndael::DEFAULT_KEY_LENGTH );

		//size_t wtcount = fwrite ( &nVersion, sizeof(int), 1, m_FileStream );
		//GASSERT(wtcount==1&&"CSerialFile::OpenFile()");

		WriteFileType ();
		break;
	};

	if ( m_FileStream )
	{
		m_IsFileOpened = TRUE;
		return TRUE;
	}

	int nErrNo = errno;
	return FALSE;
}

void CSerialFile::CloseFile ()
{
	if ( m_FileStream )		fclose( m_FileStream );

	m_FileStream	= NULL;
	m_IsFileOpened	= FALSE;
	m_DefaultOffSet = 0;
}

// Note : SerialFile의 기본 파일 HEADER영역을 제외한 곳 부터.
//		0 - Position.
//
BOOL CSerialFile::SetOffSet ( long _OffSet )
{
	if ( fseek ( m_FileStream, _OffSet + m_DefaultOffSet, SEEK_SET ) ) return FALSE;

	return TRUE;
}

//	Note : 파일의 m_DefaultOffSet를 제외한 현제 위치를 가져온다.
//
long CSerialFile::GetfTell ()
{
	return ftell ( m_FileStream ) - m_DefaultOffSet;
}

BOOL CSerialFile::BeginBlock ( EMSFILE_BLOCK _emBLOCK )
{
//	if ( m_dwBlockMarkPOS >= EMBLOCK_SIZE )	return FALSE;

	GASSERT(m_nMarkPos[m_dwBlockMarkPOS]==0&&"CSerialFile::BeginBlock");

	m_nMarkPos[m_dwBlockMarkPOS] = GetfTell();	//	현재 위치 저장.
	*this << DWORD(-1);					//	MARK 저장.

	++m_dwBlockMarkPOS;

	return TRUE;
}

BOOL CSerialFile::EndBlock ( EMSFILE_BLOCK _emBLOCK )
{
//	if ( m_dwBlockMarkPOS > EMBLOCK_SIZE )	return FALSE;

	--m_dwBlockMarkPOS;

	GASSERT(m_nMarkPos[m_dwBlockMarkPOS]!=0&&"CSerialFile::EndBlock");

	long nEndPos = GetfTell();
	DWORD dwTotalSize = DWORD ( ( nEndPos - m_nMarkPos[m_dwBlockMarkPOS] ) - sizeof(DWORD) );

	SetOffSet ( m_nMarkPos[m_dwBlockMarkPOS] );
	*this << dwTotalSize;

	SetOffSet ( nEndPos );

	m_nMarkPos[m_dwBlockMarkPOS] = 0;

	return TRUE;
}

DWORD CSerialFile::ReadBlockSize ()
{
	DWORD dwSize;
	*this >> dwSize;
	return dwSize;
}

inline size_t CSerialFile::read ( void* pbuffer, DWORD dwSize )
{
	GASSERT(m_FileStream&&"CSerialFile::read()");
	GASSERT(pbuffer&&"CSerialFile::read()");
	
	size_t rdcount;

	if ( m_emCrypt != EMBYTECRYPT_NONE )
	{
		rdcount = fread ( pbuffer, dwSize, 1, m_FileStream );
		//wld crypt
		if( m_emEncodeType == EMENCODE_WLD ) CWLDCrypt::Decryption_WLD ( (BYTE*)pbuffer, dwSize );
		else if( m_emEncodeType == EMENCODE_WLD0 ) CWLDCrypt::Decryption_WLD0 ( (BYTE*)pbuffer, dwSize );
		else BYTECRYPT::byte_decode ( (BYTE*)pbuffer, dwSize, m_emCrypt );
	}
	else
	{
		rdcount = fread ( pbuffer, dwSize, 1, m_FileStream );
	}

	GASSERT(rdcount==1&&"CSerialFile::read()");

	if ( rdcount!=1 )
	{
		CDebugSet::ToLogFile ( "file read error : %s", m_szFileName );
	}

	return rdcount;
}

inline size_t CSerialFile::write ( const void* pbuffer, DWORD dwSize )
{
	GASSERT(m_FileStream&&"CSerialFile::write()");
	GASSERT(pbuffer&&"CSerialFile::write()");

	size_t wtcount(0);

	if ( m_emCrypt != EMBYTECRYPT_NONE )
	{
		BYTE *ptemp = new BYTE[dwSize];
		memcpy ( ptemp, pbuffer, sizeof(BYTE)*dwSize );
		//wld crypt
		if( m_emEncodeType == EMENCODE_WLD ) CWLDCrypt::Encryption_WLD ( (BYTE*)ptemp, dwSize );
		else if( m_emEncodeType == EMENCODE_WLD0 ) CWLDCrypt::Encryption_WLD0 ( (BYTE*)ptemp, dwSize );
		else BYTECRYPT::byte_encode ( (BYTE*)ptemp, dwSize, m_emCrypt );


		wtcount = fwrite ( ptemp, dwSize, 1, m_FileStream );
		GASSERT(wtcount==1&&"CSerialFile::write()");

		if ( wtcount!=1 )
		{
			CDebugSet::ToLogFile ( "file write error : %s", m_szFileName );
		}

		delete[] ptemp;
	}
	else
	{
		wtcount = fwrite ( pbuffer, dwSize, 1, m_FileStream );
		GASSERT(wtcount==1&&"CSerialFile::write()");
	}

	return wtcount;
}

BOOL CSerialFile::WriteBuffer ( const void *pBuffer, DWORD Size )
{
	GASSERT(m_OpenType==FOT_WRITE);
	GASSERT(pBuffer);

	int Num = 0;

	Num = (int)write( pBuffer, Size );

	return TRUE;
}

BOOL CSerialFile::ReadBuffer ( void* pBuffer, DWORD Size )
{
	GASSERT(m_OpenType==FOT_READ);
	GASSERT(pBuffer);

	int Num = 0;

	Num = (int)read ( pBuffer, Size );

	return TRUE;
}

BOOL CSerialFile::operator << ( short Value )
{
	GASSERT(m_OpenType==FOT_WRITE);
	int Num = 0;

	Num = (int)write ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator << ( int Value )
{
	GASSERT(m_OpenType==FOT_WRITE);
	int Num = 0;

	Num = (int)write ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator << ( WORD Value )
{
	GASSERT(m_OpenType==FOT_WRITE);
	int Num = 0;

	Num = (int)write ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator << ( DWORD Value )
{
	GASSERT(m_OpenType==FOT_WRITE);
	int Num = 0;

	Num = (int)write ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator << ( UINT Value )
{
	GASSERT(m_OpenType==FOT_WRITE);
	int Num = 0;

	Num = (int)write ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator << ( float Value )
{
	GASSERT(m_OpenType==FOT_WRITE);
	int Num = 0;

	Num = (int)write ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator << ( double Value )
{
	GASSERT(m_OpenType==FOT_WRITE);
	int Num = 0;

	Num = (int)write ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator << ( D3DVECTOR Value )
{
	GASSERT(m_OpenType==FOT_WRITE);
	int Num = 0;

	Num = (int)write ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator << ( D3DCOLORVALUE &Value )
{
	GASSERT(m_OpenType==FOT_WRITE);
	int Num = 0;

	Num = (int)write ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator << ( D3DBLEND &Value )
{
	GASSERT(m_OpenType==FOT_WRITE);
	int Num = 0;

	Num = (int)write ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator << ( BYTE Value )
{
	GASSERT(m_OpenType==FOT_WRITE);
	int Num = 0;

	Num = (int)write ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator << ( char Value )
{
	GASSERT(m_OpenType==FOT_WRITE);
	int Num = 0;

	Num = (int)write ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator << ( bool Value )
{
	GASSERT(m_OpenType==FOT_WRITE);
	int Num = 0;

	Num = (int)write ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator << ( const std::string &str )
{
	GASSERT(m_OpenType==FOT_WRITE);
	int Num = 0;

	*this << DWORD(str.length()+1);
	
	Num = (int)write ( str.c_str(), sizeof(char)*(DWORD(str.length())+1) );

	return TRUE;
}

BOOL CSerialFile::operator >> ( short &Value )
{
	GASSERT(m_OpenType==FOT_READ);
	int Num = 0;

	Value = FALSE;
	Num = (int)read ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator >> ( int &Value )
{
	GASSERT(m_OpenType==FOT_READ);
	int Num = 0;

	Value = FALSE;
	Num = (int)read ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator >> ( WORD &Value )
{
	GASSERT(m_OpenType==FOT_READ);
	int Num = 0;

	Value = FALSE;
	Num = (int)read ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator >> ( DWORD &Value )
{
	GASSERT(m_OpenType==FOT_READ);
	int Num = 0;

	Value = FALSE;
	Num = (int)read ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator >> ( UINT &Value )
{
	GASSERT(m_OpenType==FOT_READ);
	int Num = 0;

	Num = (int)read ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator >> ( float &Value )
{
	GASSERT(m_OpenType==FOT_READ);
	int Num = 0;

	Num = (int)read ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator >> ( double &Value )
{
	GASSERT(m_OpenType==FOT_READ);
	int Num = 0;

	Num = (int)read ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator >> ( D3DVECTOR &Value )
{
	GASSERT(m_OpenType==FOT_READ);
	int Num = 0;

	Num = (int)read ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator >> ( D3DCOLORVALUE &Value )
{
	GASSERT(m_OpenType==FOT_READ);
	int Num = 0;

	Num = (int)read ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator >> ( D3DBLEND &Value )
{
	GASSERT(m_OpenType==FOT_READ);
	int Num = 0;

	Num = (int)read ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator >> ( BYTE &Value )
{
	GASSERT(m_OpenType==FOT_READ);
	int Num = 0;

	Value = FALSE;
	Num = (int)read ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator >> ( char &Value )
{
	GASSERT(m_OpenType==FOT_READ);
	int Num = 0;

	Value = FALSE;
	Num = (int)read ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator >> ( bool &Value )
{
	GASSERT(m_OpenType==FOT_READ);
	int Num = 0;

	Value = FALSE;
	Num = (int)read ( &Value, sizeof(Value) );

	return TRUE;
}

BOOL CSerialFile::operator >> ( std::string &str )
{
	GASSERT(m_OpenType==FOT_READ);
	int Num = 0;

	DWORD dwSize;
	*this >> dwSize;
	
	char *szBuffer = new char[dwSize];
	Num = (int)read ( szBuffer, sizeof(char)*(dwSize) );

	str = szBuffer;
	delete[] szBuffer;

	return TRUE;
}


void CSerialFile::DumpToDisk_Increment( std::string strPATH )
{
	CString strTEXT;
	strTEXT.Format( "FILE_DUMP_%d", m_nLastDump );
	DumpToDisk( strPATH.c_str(), strTEXT.GetString() );
	++ m_nLastDump;
}

void CSerialFile::DumpToDisk( std::string strPATH, std::string strFILE )
{
	if ( strPATH.size() == 0 )
	{
		CString strAppPath;

		TCHAR szPath[MAX_PATH] = {0};
		GetModuleFileName(::AfxGetInstanceHandle(), szPath, MAX_PATH);
		strAppPath = szPath;

		if ( !strAppPath.IsEmpty() )
		{
			DWORD dwFind = strAppPath.ReverseFind ( '\\' );
			if ( dwFind != -1 )
			{
				CString m_strAppPath = strAppPath.Left ( dwFind );

				if ( !m_strAppPath.IsEmpty() )
					if ( m_strAppPath.GetAt(0) == '"' )
						m_strAppPath = m_strAppPath.Right ( m_strAppPath.GetLength() - 1 );

				m_strAppPath += "\\";
				strPATH =  m_strAppPath.GetString() ;
			}
		}
	}

	char szPathName[MAX_PATH] = "";
	StringCchCopy( szPathName, MAX_PATH, strPATH.c_str() );
	StringCchCat( szPathName, MAX_PATH, "_DUMP_" );
	StringCchCat( szPathName, MAX_PATH, "\\" );
	StringCchCat( szPathName, MAX_PATH, strFILE.c_str() );

	FILE* pFileOutput = NULL;
	pFileOutput = fopen( szPathName, "wb" );
	if ( !pFileOutput )
	{
		CDebugSet::MsgBox( "CSerialFile::DumpToDisk Unable to create File %s", szPathName );
		return;
	}

	long lCurPos = GetfTell();
	fseek ( m_FileStream, 0, SEEK_END );
	long lsize = ftell ( m_FileStream );
	SetOffSet( lCurPos );
	long lCurPos2 = GetfTell();

	if ( lCurPos != lCurPos2 )
	{
		CDebugSet::MsgBox( "Offset Error1! %lu~%lu", lCurPos, lCurPos2 );
		return;
	}

	long lRead = lsize-lCurPos;
	BYTE* pBuffer = new BYTE[lsize-lCurPos];
	size_t s = fread ( pBuffer, (size_t)lRead, 1, m_FileStream );

	fwrite( pBuffer, sizeof(BYTE), lRead, pFileOutput );
	fclose( pFileOutput );

	delete[] pBuffer;

	SetOffSet( lCurPos );
	long lCurPos3 = GetfTell();
	if ( lCurPos != lCurPos3)
	{
		CDebugSet::MsgBox( "Offset Error2! %lu~%lu", lCurPos, lCurPos3 );
		return;
	}

}