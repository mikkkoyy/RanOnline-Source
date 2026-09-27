#include "stdafx.h"
#include "./GLCharData.h"
#include "./GLItemMan.h"

#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/* PANDORA DB BASED SYSTEM, ARWEKAJ09 9/16/2024 */
BOOL SCHARDATA2::GETPANDORA_BYBUF(CByteStream& ByteStream) const
{
	ByteStream.ClearBuffer();

	ByteStream << (DWORD)0x0001;

	ByteStream << (DWORD)m_mapPandoraResults.size();
	for (PANDORABOXMAP_CITER pos = m_mapPandoraResults.begin();
		pos != m_mapPandoraResults.end(); ++pos)
	{
		const GLPANDORA_BOX* pPandora = &pos->second;
		ByteStream.WriteBuffer((LPBYTE)pPandora, sizeof(GLPANDORA_BOX));
	}


	return TRUE;
}

/* PANDORA DB BASED SYSTEM, ARWEKAJ09 9/16/2024 */
BOOL SCHARDATA2::SETPANDORA_BYBUF(CByteStream& ByteStream)
{
	if (ByteStream.IsEmpty())	return TRUE;

	DWORD dwVersion, dwNum;
	ByteStream >> dwVersion;

	if (dwVersion == 0x0001)
	{
		ByteStream >> dwNum;
		for (DWORD i = 0; i < dwNum; i++)
		{
			GLPANDORA_BOX sPandora;
			ByteStream.ReadBuffer((LPBYTE)&sPandora, sizeof(GLPANDORA_BOX));
			m_mapPandoraResults.insert(std::make_pair(sPandora.nIndex, sPandora));
		}
	}
	else
	{
		CDebugSet::ErrorVersion("SCHARDATA2::SETPANDORA_BYBUF", dwVersion);
	}

	return TRUE;
}