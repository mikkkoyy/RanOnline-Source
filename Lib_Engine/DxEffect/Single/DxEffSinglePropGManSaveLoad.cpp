#include "stdafx.h"

#include "./DxEffSinglePropGMan.h"
#include "../Common/SerialFile.h"
#include "../G-Logic/GLogic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HRESULT EFF_PROPERTY::LoadFile ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	BOOL bValid = FALSE;

	WORD MovSoundVER;
	WORD SoundVER;

	SFile >> MovSoundVER;
	SFile >> SoundVER;

	m_MovSound.LoadSet ( SFile, MovSoundVER, SoundVER );

	//	Note : 형제 읽기.
	SFile >> bValid;
	if ( bValid )
	{
		DWORD dwTypeID;
		SFile >> dwTypeID;
		EFF_PROPERTY* pNewProp = NEW_PROP ( dwTypeID );
		GASSERT(pNewProp);

		pNewProp->LoadFile ( SFile, pd3dDevice );
		m_pSibling = pNewProp;
	}

	//	Note : 자식 읽기.
	SFile >> bValid;
	if ( bValid )
	{
		DWORD dwTypeID;
		SFile >> dwTypeID;
		EFF_PROPERTY* pNewProp = NEW_PROP ( dwTypeID );
		GASSERT(pNewProp);

		pNewProp->LoadFile ( SFile, pd3dDevice );
		m_pChild = pNewProp;
	}

	return S_OK;
}

HRESULT EFF_PROPERTY::SaveFile ( CSerialFile &SFile )
{
	SFile << m_MovSound.m_MovSoundVER;
	SFile << m_MovSound.m_SoundVER;

	m_MovSound.SaveSet ( SFile );

	//	Note : 형제 저장.
	if ( m_pSibling )	SFile << TRUE;
	else			SFile << FALSE;

	if ( m_pSibling )	m_pSibling->SaveFile ( SFile );

	//	Note : 자식 저장.
	if ( m_pChild )	SFile << TRUE;
	else			SFile << FALSE;

	if ( m_pChild )		m_pChild->SaveFile ( SFile );

	return S_OK;
}

HRESULT EFF_PROPGROUP::LOAD_0100 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;

	SFile >> m_dwFlag;
	SFile >> m_vMax;
	SFile >> m_vMin;

	m_vLocal = D3DXVECTOR3 ( 0.f, 0.f, 0.f );

	BOOL bNeverDie	= FALSE;
	BOOL bTargetPOS	= FALSE;
	BOOL bTargetID	= FALSE;
	BOOL bNoneMove	= FALSE;
	BOOL bNone		= FALSE;
	BOOL bEvent		= FALSE;

	if ( m_dwFlag & 0x10000 )		bEvent = TRUE;
	if ( m_dwFlag & 0x01000 )		bNeverDie = TRUE;
	else if ( m_dwFlag & 0x02000 )	bTargetPOS = TRUE;
	else if ( m_dwFlag & 0x04000 )	bTargetID = TRUE;
	else if ( m_dwFlag & 0x08000 )	bNoneMove = TRUE;
	else							bNone = TRUE;

	m_dwFlag = 0;

	if ( bNeverDie )	m_dwFlag |= (EFF_CFG_NEVERDIE | EFF_CFG_NONE);
	if ( bTargetPOS )	m_dwFlag |= EFF_CFG_TARGETPOS;
	if ( bTargetID )	m_dwFlag |= EFF_CFG_TARGETID;
	if ( bNoneMove )	m_dwFlag |= EFF_CFG_MOVE;
	if ( bNone )		m_dwFlag |= EFF_CFG_NONE;
	if ( bEvent )		m_dwFlag |= EFF_CFG_EVENT;

	m_dwFlag |= EFF_CFG_AVERAGE;	// 데이터 추가
	m_dwFlag |= EFF_CFG_AUTOAABBBOX;	// 데이터 추가

	BOOL bValid(FALSE);
	SFile >> bValid;
	if ( bValid )
	{
		DWORD dwTypeID(0);
		SFile >> dwTypeID;

		EFF_PROPERTY* pNewProp = EFF_PROPERTY::NEW_PROP ( dwTypeID );
		GASSERT(pNewProp);

		hr = pNewProp->LoadFile ( SFile, pd3dDevice );
		if ( FAILED(hr) )
		{
			SAFE_DELETE(pNewProp);
			return hr;
		}

		m_pPropRoot = pNewProp;
	}

	return S_OK;
}

HRESULT EFF_PROPGROUP::LOAD_0101 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;

	SFile >> m_dwFlag;
	SFile >> m_vMax;
	SFile >> m_vMin;

	m_vLocal = D3DXVECTOR3 ( 0.f, 0.f, 0.f );

	BOOL bNoneMove	= FALSE;

	if ( m_dwFlag & 0x08000 )
	{
		m_dwFlag |= EFF_CFG_TARGETID;
		m_dwFlag &= ~EFF_CFG_MOVE;
	}

	m_dwFlag |= EFF_CFG_AVERAGE;	// 데이터 추가
	m_dwFlag |= EFF_CFG_AUTOAABBBOX;	// 데이터 추가

	BOOL bValid(FALSE);
	SFile >> bValid;
	if ( bValid )
	{
		DWORD dwTypeID(0);
		SFile >> dwTypeID;

		EFF_PROPERTY* pNewProp = EFF_PROPERTY::NEW_PROP ( dwTypeID );
		GASSERT(pNewProp);

		hr = pNewProp->LoadFile ( SFile, pd3dDevice );
		if ( FAILED(hr) )
		{
			SAFE_DELETE(pNewProp);
			return hr;
		}

		m_pPropRoot = pNewProp;
	}

	return S_OK;
}

HRESULT EFF_PROPGROUP::LOAD_0102 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;

	SFile >> m_dwFlag;
	SFile >> m_vMax;
	SFile >> m_vMin;
	SFile >> m_vLocal;

	if ( m_dwFlag == 0L )	m_dwFlag |= EFF_CFG_NONE;

	m_dwFlag |= EFF_CFG_AVERAGE;	// 데이터 추가
	m_dwFlag |= EFF_CFG_AUTOAABBBOX;	// 데이터 추가

	BOOL bValid(FALSE);
	SFile >> bValid;
	if ( bValid )
	{
		DWORD dwTypeID(0);
		SFile >> dwTypeID;

		EFF_PROPERTY* pNewProp = EFF_PROPERTY::NEW_PROP ( dwTypeID );
		GASSERT(pNewProp);

		hr = pNewProp->LoadFile ( SFile, pd3dDevice );
		if ( FAILED(hr) )
		{
			SAFE_DELETE(pNewProp);
			return hr;
		}

		m_pPropRoot = pNewProp;
	}

	return S_OK;
}

HRESULT EFF_PROPGROUP::LOAD_0103 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;

	SFile >> m_dwFlag;
	SFile >> m_vMax;
	SFile >> m_vMin;
	SFile >> m_vLocal;

	m_dwFlag |= EFF_CFG_AUTOAABBBOX;	// 데이터 추가

	BOOL bValid(FALSE);
	SFile >> bValid;
	if ( bValid )
	{
		DWORD dwTypeID(0);
		SFile >> dwTypeID;

		EFF_PROPERTY* pNewProp = EFF_PROPERTY::NEW_PROP ( dwTypeID );
		GASSERT(pNewProp);

		hr = pNewProp->LoadFile ( SFile, pd3dDevice );
		if ( FAILED(hr) )
		{
			SAFE_DELETE(pNewProp);
			return hr;
		}

		m_pPropRoot = pNewProp;
	}

	return S_OK;
}

HRESULT EFF_PROPGROUP::LOAD_0104 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;

	SFile >> m_dwFlag;
	SFile >> m_vMax;
	SFile >> m_vMin;
	SFile >> m_vLocal;

	m_dwFlag |= EFF_CFG_AUTOAABBBOX;	// 데이터 추가

	BOOL bValid(FALSE);
	SFile >> bValid;
	if ( bValid )
	{
		DWORD dwTypeID(0);
		SFile >> dwTypeID;

		EFF_PROPERTY* pNewProp = EFF_PROPERTY::NEW_PROP ( dwTypeID );
		GASSERT(pNewProp);

		hr = pNewProp->LoadFile ( SFile, pd3dDevice );
		if ( FAILED(hr) )
		{
			SAFE_DELETE(pNewProp);
			return hr;
		}

		m_pPropRoot = pNewProp;
	}

	return S_OK;
}


HRESULT EFF_PROPGROUP::LOAD_0105 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;

	SFile >> m_dwFlag;
	SFile >> m_vMax;
	SFile >> m_vMin;
	SFile >> m_vLocal;

	BOOL bValid(FALSE);
	SFile >> bValid;
	if ( bValid )
	{
		DWORD dwTypeID(0);
		SFile >> dwTypeID;

		EFF_PROPERTY* pNewProp = EFF_PROPERTY::NEW_PROP ( dwTypeID );
		GASSERT(pNewProp);

		hr = pNewProp->LoadFile ( SFile, pd3dDevice );
		if ( FAILED(hr) )
		{
			SAFE_DELETE(pNewProp);
			return hr;
		}

		m_pPropRoot = pNewProp;
	}

	return S_OK;
}

HRESULT EFF_PROPGROUP::LOAD_0106 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;

	SFile >> m_dwFlag;
	SFile >> m_vLocal;
	SFile >> m_vMin;
	SFile >> m_vMax;
	
	BOOL bValid;
	SFile >> bValid;
	if ( bValid )
	{
		DWORD dwTypeID(0);
		SFile >> dwTypeID;

		EFF_PROPERTY* pNewProp = EFF_PROPERTY::NEW_PROP ( dwTypeID );
		GASSERT(pNewProp);

		hr = pNewProp->LoadFile ( SFile, pd3dDevice );
		if ( FAILED(hr) )
		{
			SAFE_DELETE(pNewProp);
			return hr;
		}

		m_pPropRoot = pNewProp;
	}

	return S_OK;
}

HRESULT EFF_PROPGROUP::LOAD_0200 ( basestream &SFile, LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;

	SFile >> m_dwFlag;
	SFile >> m_vLocal;
	SFile >> m_vMin;
	SFile >> m_vMax;

	BOOL bValid;
	SFile >> bValid;
	if ( bValid )
	{
		DWORD dwTypeID(0);
		SFile >> dwTypeID;

		EFF_PROPERTY* pNewProp = EFF_PROPERTY::NEW_PROP ( dwTypeID );
		GASSERT(pNewProp);

		hr = pNewProp->LoadFile ( SFile, pd3dDevice );
		if ( FAILED(hr) )
		{
			SAFE_DELETE(pNewProp);
			return hr;
		}

		m_pPropRoot = pNewProp;
	}

	return S_OK;
}

HRESULT EFF_PROPGROUP::SaveFile ( const char* szFileName )
{
	GASSERT(szFileName);

	m_strFileName = szFileName;
	std::transform ( m_strFileName.begin(), m_strFileName.end(), m_strFileName.begin(), tolower );

	CSerialFile SFile;
	SFile.SetFileType ( "EFF_PROPGROUP", EFF_PROPGROUP::FILEVERSION );

	if ( SFile.OpenFile(FOT_WRITE,m_strFileName.c_str()) )
	{
		//	Note : EFF_PROPGROUP
		//
		SFile << (DWORD)EFF_PROPGROUP::VERSION;

		SFile.SetEncodeType ( EMBYTECRYPT_EGP );

		SFile << m_dwFlag;
		SFile << m_vLocal;
		SFile << m_vMin;
		SFile << m_vMax;
		
		//	Note : EFF_PROP
		//
		if ( m_pPropRoot )	SFile << TRUE;
		else				SFile << FALSE;

		if ( m_pPropRoot )	m_pPropRoot->SaveFile ( SFile );
	}

	return S_OK;
}

HRESULT EFF_PROPGROUP::LoadFile ( const char* szFileName, LPDIRECT3DDEVICEQ pd3dDevice )
{
	if ( !szFileName )		return S_FALSE;

	m_strFileName = szFileName;
	std::transform ( m_strFileName.begin(), m_strFileName.end(), m_strFileName.begin(), tolower );

	HRESULT hr = S_OK;

	char szFullPath[MAX_PATH] = "";
	StringCchCopy( szFullPath, MAX_PATH, DxEffSinglePropGMan::GetInstance().GetPath() );
	StringCchCat( szFullPath, MAX_PATH, m_strFileName.c_str() );

	std::auto_ptr<basestream> pBStream( GLOGIC::openfile_basestream(GLOGIC::bENGLIB_ZIPFILE, 
										GLOGIC::strEFFECT_ZIPFILE.c_str(),
										szFullPath, 
										szFileName ) );

	if ( !pBStream.get() )
	{
		if( strlen( szFileName ) > 0 )
			CDebugSet::ErrorFile( "EFF_PROPGROUP::LoadFile", szFileName );

		return E_FAIL;
	}

	basestream &SFile = *pBStream;

	DWORD dwFileVer(0);
	char szFileType[FILETYPESIZE] = "";
	SFile.GetFileType( szFileType, FILETYPESIZE, dwFileVer );
	if ( dwFileVer > EFF_PROPGROUP::FILEVERSION )
	{
		CDebugSet::ErrorVersion( "EFF_PROPGROUP::LoadFile", dwFileVer );
		return E_FAIL;
	}

	DWORD dwVer(0);
	SFile >> dwVer;

	if ( dwVer>=VERSION_ENCODE )
		SFile.SetEncodeType ( EMBYTECRYPT_EGP );

	if ( dwVer == EFF_PROPGROUP::VERSION )
	{
		hr = LOAD_0200( SFile, pd3dDevice );
	}
	else if ( dwVer == 0x0106 )
	{
		hr = LOAD_0106( SFile, pd3dDevice );
	}
	else if ( dwVer == 0x0105 )
	{
		hr = LOAD_0105( SFile, pd3dDevice );
	}
	else if ( dwVer == 0x0104 )
	{
		hr = LOAD_0104( SFile, pd3dDevice );
	}
	else if ( dwVer == 0x0103 )
	{
		hr = LOAD_0103( SFile, pd3dDevice );
	}
	else if ( dwVer == 0x0102 )
	{
		hr = LOAD_0102( SFile, pd3dDevice );
	}
	else if ( dwVer == 0x0101 )
	{
		hr = LOAD_0101( SFile, pd3dDevice );
	}
	else if ( dwVer == 0x0100 )
	{
		hr = LOAD_0100( SFile, pd3dDevice );
	}
	else
	{
		CDebugSet::ErrorVersion( "EFF_PROPGROUP::LoadFile", dwVer );
		return E_FAIL;
	}

	return hr;
}
