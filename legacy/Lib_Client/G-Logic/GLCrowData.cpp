#include "stdafx.h"
#include "./GLCrowData.h"
#include "./GLItemMan.h"
#include "./GLogic.h"
#include "./GLogicData.h"
#include "./GLStringTable.h"

#include "../../Lib_Engine/Common/StringFile.h"
#include "../../Lib_Engine/Common/STRINGUTILS.h"
#include "../../Lib_Engine/Meshs/DxSkinDataDummy.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


const char* GLCrowDataMan::_FILEHEAD = "GLCROW";
const char* GLCrowDataMan::_LOGFILE = "_CrowData_Synce.txt";
const char* GLCrowDataMan::_STRINGTABLE = "CrowStrTable.txt";

std::vector< string > StringExplode(const string& str, const char& ch);
std::vector< string > StringExplode(const string& str, const char& ch) {
    string next;
    vector<string> result;

    // For each character in the string
    for (string::const_iterator it = str.begin(); it != str.end(); it++) {
        // If we've hit the terminal character
        if (*it == ch) {
            // If we have some characters accumulated
            if (!next.empty()) {
                // Add them to the result vector
                result.push_back(next);
                next.clear();
            }
        } else {
            // Accumulate the next character into the sequence
            next += *it;
        }
    }
    if (!next.empty())
         result.push_back(next);
    return result;
}

GLCrowDataMan& GLCrowDataMan::GetInstance()
{
	static GLCrowDataMan Instance;
	return Instance;
}

BOOL SGENITEM::ISGEN (  float fGenRate )	//	발생 확율.
{
	if ( m_fGenRate==0 )	return FALSE;

	float fGEN_RATE = m_fGenRate*GLCONST_CHAR::fITEM_DROP_SCALE;
	if( fGenRate != 0.0f )
		fGEN_RATE *= fGenRate;
	else 
		return FALSE;
	return RANDOM_GEN(fGEN_RATE);
}

WORD SGENITEM::SELECTNUM ()	//	발생 갯수.
{
	WORD wNum = 0;
	float fNowRate = seqrandom::getpercent();
	for ( int i=0; i<EMGNUM; ++i )
	{
		if ( m_sGenNum[i].wNum==0 )		break;

		if ( (m_sGenNum[i].fRateL<=fNowRate) && (fNowRate<m_sGenNum[i].fRateH) )
		{
			wNum = m_sGenNum[i].wNum;
			break;
		}
	}

	return wNum;
}

DWORD SGENITEM::SELECTSPECID ( float fGEN_RATE, bool& bSPECID )	//	발생 군.
{
	bool bSCALE = GLCONST_CHAR::fITEM_DROP_LOWER_SCALE!=1.0f || fGEN_RATE!=1.0f;
	
	float fNowRate = seqrandom::getpercent();
	DWORD dwSpecID = EMSPEC_NULL;
	float fRateL = m_sItemSpec[0].fRateL;
	for ( int i=0; i<EMGSPEC; ++i )
	{
		float fRate = (m_sItemSpec[i].fRateH-m_sItemSpec[i].fRateL);
		if ( bSCALE && fRate < 2.0f )
		{
			fRate = fRate * (GLCONST_CHAR::fITEM_DROP_LOWER_SCALE*fGEN_RATE);
		}

		if ( (fRateL<=fNowRate) && fNowRate<(fRateL+fRate) )
		{
			dwSpecID = m_sItemSpec[i].dwSPECID;
			bSPECID = m_sItemSpec[i].bSPECID;
			break;
		}

		fRateL += fRate;
	}

	return dwSpecID;
}

SNATIVEID SQTGENITEM::SELECTITEM () // Question 아이템 발생 by 경대
{
	float fNowRate = seqrandom::getpercent();
	float fRateL(0);

	for ( int i=0; i<m_nItemNum; ++i )
	{
		float fRate = m_sItem[i].m_fGenRate;
		
		if ( (fRateL<=fNowRate) && fNowRate<(fRateL+fRate) )
		{
			return m_sItem[i].m_sGenItemID;
		}

		fRateL += fRate;
	}

	return SNATIVEID(false);
}

SCROWDATA& SCROWDATA::Assign ( SCROWDATA &Data )
{
	m_sBasic = Data.m_sBasic;
	m_sAction = Data.m_sAction;
	m_sGenerate = Data.m_sGenerate;

	for ( int i=0; i<EMMAXATTACK; ++i )
		m_sCrowAttack[i] = Data.m_sCrowAttack[i];
	
	for ( int i = 0; i<10; ++i )	m_sGenItem[i] = Data.m_sGenItem[i];
	m_sQtGenItem = Data.m_sQtGenItem;

	m_sNpcTalkDlg = Data.m_sNpcTalkDlg;

	for ( int i=0; i<SCROWACTION::SALENUM; ++i )
	{
		m_strSaleType[i] = Data.m_strSaleType[i];
		m_sSaleItems[i].Assign ( Data.m_sSaleItems[i] );
	}

	m_mapNpcSellPrice = Data.m_mapNpcSellPrice;

	m_mapMobDropSpecID = Data.m_mapMobDropSpecID;

	if ( Data.m_pANIMATION )
	{
		SAFE_DELETE_ARRAY ( m_pANIMATION );
		m_pANIMATION = new VECANIATTACK[AN_TYPE_SIZE*AN_SUB_00_SIZE];

		for (int i=0; i<AN_TYPE_SIZE*AN_SUB_00_SIZE; ++i )
		{
			m_pANIMATION[i] = Data.m_pANIMATION[i];
		}
	}
	else
	{
		SAFE_DELETE_ARRAY ( m_pANIMATION );
	}

	/*npc shop, Juver, 2017/07/26 */
	m_sNPCShop = Data.m_sNPCShop;

	/*item exchange, Juver, 2017/10/11 */
	m_sNPCItemExchange = Data.m_sNPCItemExchange;

	return *this;
}

HRESULT SCROWDATA::LoadFile ( basestream &SFile, BOOL bServer, bool bPastLoad )
{
	DWORD dwDataVer, dwDataSize;

	SFile >> dwDataVer;
	if ( dwDataVer > VERSION )
	{
		CDebugSet::ErrorVersion( "SCROWDATA::LoadFile", dwDataVer );
		return E_FAIL;
	}

	DWORD dwDATATYPE;
	SFile >> dwDATATYPE;
	while ( dwDATATYPE!=FILE_END_DATA )
	{
		switch ( dwDATATYPE )
		{
		case FILE_SBASIC:
			{
				SFile >> dwDataVer;
				SFile >> dwDataSize;

				if( bPastLoad )
				{
					if ( dwDataVer==0x0100 )
					{
						SCROWBASIC_100 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0101 )
					{
						SCROWBASIC_101 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0102 )
					{
						SCROWBASIC_102 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0103 )
					{
						SCROWBASIC_103 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0104 )
					{
						SCROWBASIC_104 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0105 )
					{
						SCROWBASIC_105 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0106)
					{
						SCROWBASIC_105 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0107)
					{
						SCROWBASIC_106 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0108)
					{
						SCROWBASIC_108 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0109)
					{
						SCROWBASIC_109 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0110)
					{
						SCROWBASIC_110 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0113)
					{
						SCROWBASIC_113 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0116)
					{
						SCROWBASIC_116 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0117)
					{
						SCROWBASIC_117 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0200)
					{
						SCROWBASIC_200 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0201)
					{
						SCROWBASIC_201 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0202)
					{
						SCROWBASIC_202 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0203)
					{
						SCROWBASIC_203 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==0x0204)
					{
						SCROWBASIC_204 sCrowBasicOld;
						GASSERT(sizeof(sCrowBasicOld)==dwDataSize);
						SFile.ReadBuffer ( &sCrowBasicOld, sizeof(sCrowBasicOld) );

						m_sBasic = sCrowBasicOld;
					}
					else if ( dwDataVer==SCROWBASIC::VERSION )
					{
						GASSERT(sizeof(SCROWBASIC)==dwDataSize);
						SFile.ReadBuffer ( &m_sBasic, sizeof(SCROWBASIC) );
					}
					else
					{
						CDebugSet::ErrorVersion( "SCROWDATA::SCROWBASIC Pastload", dwDataVer );
						SFile.SetOffSet ( SFile.GetfTell() + dwDataSize );
					}
				}else{
					if ( dwDataVer==SCROWBASIC::VERSION )
					{
						GASSERT(sizeof(SCROWBASIC)==dwDataSize);
						SFile.ReadBuffer ( &m_sBasic, sizeof(SCROWBASIC) );
					}
					else
					{
						CDebugSet::ErrorVersion( "SCROWDATA::SCROWBASIC", dwDataVer );
						SFile.SetOffSet ( SFile.GetfTell() + dwDataSize );
						return E_FAIL;
					}
				}
			}
			break;

		case FILE_SACTION:
			{
				SFile >> dwDataVer;
				SFile >> dwDataSize;

				if( dwDataVer==0x0100 )
				{
					SCROWACTION_100 sOldData;

					GASSERT(sizeof(sOldData)==dwDataSize);
					SFile.ReadBuffer ( &sOldData, sizeof(sOldData) );
				
					m_sAction = sOldData;
				}
				else if( dwDataVer==0x0101 )
				{
					SCROWACTION_101 sOldData;

					GASSERT(sizeof(sOldData)==dwDataSize);
					SFile.ReadBuffer ( &sOldData, sizeof(sOldData) );
				
					m_sAction = sOldData;
				}
				else if( dwDataVer==0x0102 )
				{
					SCROWACTION_102 sOldData;

					GASSERT(sizeof(sOldData)==dwDataSize);
					SFile.ReadBuffer ( &sOldData, sizeof(sOldData) );
				
					m_sAction = sOldData;
				}
				else if( dwDataVer==0x0103 )
				{
					m_sAction.LOAD103 ( SFile );
				}
				else if( dwDataVer == 0x0104 )
				{
					m_sAction.LOAD104( SFile );
				}
				else if( dwDataVer == 0x0105 )
				{
					m_sAction.LOAD105( SFile );
				}
				else if( dwDataVer == 0x0106 )
				{
					m_sAction.LOAD106( SFile );
				}
				else if( dwDataVer == 0x0107 )
				{
					m_sAction.LOAD107( SFile );
				}
				else if( dwDataVer == 0x0108 )
				{
					m_sAction.LOAD108( SFile );
				}
				else if( dwDataVer == 0x0110 )
				{
					m_sAction.LOAD110( SFile );
				}
				else if( dwDataVer == 0x0200 )
				{
					m_sAction.LOAD200( SFile );
				}
				else if( dwDataVer==SCROWACTION::VERSION )
				{
					m_sAction.LOAD ( SFile );
				}
				else
				{
					CDebugSet::ErrorVersion( "SCROWDATA::SCROWACTION", dwDataVer );
					SFile.SetOffSet ( SFile.GetfTell() + dwDataSize );
				}
			}
			break;

		case FILE_SGEN:
			{
				SFile >> dwDataVer;
				SFile >> dwDataSize;

				if ( dwDataVer==0x0100 )
				{
					SCROWGEN_100 sOldData;
					GASSERT(sizeof(sOldData)==dwDataSize);
					SFile.ReadBuffer ( &sOldData, sizeof(sOldData) );
					
					m_sGenerate.Assign ( sOldData );
				}
				else if ( dwDataVer==0x0101 )
				{
					SCROWGEN_101 sOldData;
					GASSERT(sizeof(sOldData)==dwDataSize);
					SFile.ReadBuffer ( &sOldData, sizeof(sOldData) );
					
					m_sGenerate.Assign ( sOldData );
				}
				else if ( dwDataVer==0x0102 )
				{
					m_sGenerate.LOAD_0102(SFile);
				}
				else if ( dwDataVer==0x0103 )
				{
					m_sGenerate.LOAD_0103(SFile);
				}
				else if ( dwDataVer==0x0104 )
				{
					m_sGenerate.LOAD_0104(SFile);
				}
				else if ( dwDataVer==SCROWGEN::VERSION )
				{
					m_sGenerate.LOAD(SFile);
				}
				else
				{
					CDebugSet::ErrorVersion( "SCROWDATA::SCROWGEN", dwDataVer );
					SFile.SetOffSet ( SFile.GetfTell() + dwDataSize );
				}
			}
			break;

		case FILE_SATTACK:
			{
				SFile >> dwDataVer;
				SFile >> dwDataSize;

				if ( dwDataVer == 0x0100 )
				{
					SCROWATTACK_100 sOLDATTACK[3];
					SFile.ReadBuffer ( &sOLDATTACK[0], sizeof(SCROWATTACK_100) );
					SFile.ReadBuffer ( &sOLDATTACK[1], sizeof(SCROWATTACK_100) );
					SFile.ReadBuffer ( &sOLDATTACK[2], sizeof(SCROWATTACK_100) );

					m_sCrowAttack[0] = sOLDATTACK[0];
					m_sCrowAttack[1] = sOLDATTACK[1];
					m_sCrowAttack[2] = sOLDATTACK[2];

					for( int i=3; i<EMMAXATTACK; ++i )
						m_sCrowAttack[i] = SCROWATTACK();
				}
				else if ( dwDataVer == 0x0101 )
				{
					SCROWATTACK_101 sOLDATTACK[3];
					SFile.ReadBuffer ( &sOLDATTACK[0], sizeof(SCROWATTACK_101) );
					SFile.ReadBuffer ( &sOLDATTACK[1], sizeof(SCROWATTACK_101) );
					SFile.ReadBuffer ( &sOLDATTACK[2], sizeof(SCROWATTACK_101) );

					m_sCrowAttack[0] = sOLDATTACK[0];
					m_sCrowAttack[1] = sOLDATTACK[1];
					m_sCrowAttack[2] = sOLDATTACK[2];

					for( int i=3; i<EMMAXATTACK; ++i )
						m_sCrowAttack[i] = SCROWATTACK();
				}
				else if ( dwDataVer == 0x0102 )
				{
					SCROWATTACK_102 sOLDATTACK[3];
					SFile.ReadBuffer ( &sOLDATTACK[0], sizeof(SCROWATTACK_102) );
					SFile.ReadBuffer ( &sOLDATTACK[1], sizeof(SCROWATTACK_102) );
					SFile.ReadBuffer ( &sOLDATTACK[2], sizeof(SCROWATTACK_102) );

					m_sCrowAttack[0] = sOLDATTACK[0];
					m_sCrowAttack[1] = sOLDATTACK[1];
					m_sCrowAttack[2] = sOLDATTACK[2];

					for( int i=3; i<EMMAXATTACK; ++i )
						m_sCrowAttack[i] = SCROWATTACK();
				}
				else if ( dwDataVer==0x0103 )
				{
					SCROWATTACK_103 sOLDATTACK[3];
					SFile.ReadBuffer ( &sOLDATTACK[0], sizeof(SCROWATTACK_103) );
					SFile.ReadBuffer ( &sOLDATTACK[1], sizeof(SCROWATTACK_103) );
					SFile.ReadBuffer ( &sOLDATTACK[2], sizeof(SCROWATTACK_103) );

					m_sCrowAttack[0] = sOLDATTACK[0];
					m_sCrowAttack[1] = sOLDATTACK[1];
					m_sCrowAttack[2] = sOLDATTACK[2];

					for( int i=3; i<EMMAXATTACK; ++i )
						m_sCrowAttack[i] = SCROWATTACK();
				}
				else if ( dwDataVer==0x0104 )
				{
					SCROWATTACK_104 sOLDATTACK[3];
					SFile.ReadBuffer ( &sOLDATTACK[0], sizeof(SCROWATTACK_104) );
					SFile.ReadBuffer ( &sOLDATTACK[1], sizeof(SCROWATTACK_104) );
					SFile.ReadBuffer ( &sOLDATTACK[2], sizeof(SCROWATTACK_104) );

					m_sCrowAttack[0] = sOLDATTACK[0];
					m_sCrowAttack[1] = sOLDATTACK[1];
					m_sCrowAttack[2] = sOLDATTACK[2];

					for( int i=3; i<EMMAXATTACK; ++i )
						m_sCrowAttack[i] = SCROWATTACK();
				}
				else if ( dwDataVer==0x0105 )
				{
					m_sCrowAttack[0].LOAD_0105 ( SFile );
					m_sCrowAttack[1].LOAD_0105 ( SFile );
					m_sCrowAttack[2].LOAD_0105 ( SFile );

					for( int i=3; i<EMMAXATTACK; ++i )
						m_sCrowAttack[i] = SCROWATTACK();
				}
				else if ( dwDataVer==0x0107 )
				{
					for( int i=0; i<EMMAXATTACK; ++i )
						m_sCrowAttack[i] = SCROWATTACK();

					DWORD dwNum(0);
					SFile >> dwNum;
					for( DWORD i=0; i<dwNum; ++ i )
					{
						SCROWATTACK	sCrowAttack;
						sCrowAttack.LOAD_0107( SFile );
						if ( i >=EMMAXATTACK )	continue;
						m_sCrowAttack[i] = sCrowAttack;
					}
				}
				else if ( SCROWATTACK::VERSION==dwDataVer )
				{
					for( int i=0; i<EMMAXATTACK; ++i )
						m_sCrowAttack[i].LOAD ( SFile );
				}
				else
				{
					CDebugSet::ErrorVersion( "SCROWDATA::SCROWATTACK", dwDataVer );
					SFile.SetOffSet ( SFile.GetfTell() + dwDataSize );
				}
			}
			break;
		};

		SFile >> dwDATATYPE;
	};

	//	Note : 만약 Crow가 판매 목록을 가지고 있다면 읽어온다.
	if ( GLItemMan::GetInstance().ValidTable() )
	{
		for ( int i=0; i<SCROWACTION::SALENUM; ++i )
		{
			SaleInvenLoadFile ( m_sAction.m_strSaleFile[i].c_str(), m_sSaleItems[i], m_strSaleType[i], m_mapNpcSellPrice );
		}

		/*npc shop, Juver, 2017/07/25 */
		m_sNPCShop.LoadData( m_sAction.m_strShopFile );

		/*item exchange, Juver, 2017/10/11 */
		m_sNPCItemExchange.LoadData( m_sAction.m_strItemExchangeFile );
	}

	if ( bServer )
	{
		if ( !m_sGenerate.m_strGenItem.empty() )
		{
			std::vector< string > results;
			results = StringExplode(m_sGenerate.m_strGenItem.c_str(), ';');
			for ( int i=0; i<results.size(); i++ )
			{
			//m_sGenItem.LOADFILE ( m_sGenerate.m_strGenItem.c_str() );
				m_sGenItem[i].LOADFILE ( results[i].c_str() );
			}
		}

		// Note : Question Item LOADFILE by 경대
		//
		if ( !m_sGenerate.m_strQtGenItem.empty() )
		{
			m_sQtGenItem.LOADFILE ( m_sGenerate.m_strQtGenItem.c_str() );
		}

		LoadAniSet ( m_sAction.m_strSkinObj.c_str() );
	
		m_sNpcTalkDlg.Load ( GetTalkFile() );
	}

	return S_OK;
}

void SCROWDATA::LoadAniSet ( const char* szSkinObj )
{
	SAFE_DELETE_ARRAY(m_pANIMATION);
	m_pANIMATION = new VECANIATTACK[AN_TYPE_SIZE*AN_SUB_00_SIZE];

	if ( szSkinObj[0]=='\0' )	return;

	DxSkinDataDummy DataDummy;
	BOOL bOK = DataDummy.LoadFile ( szSkinObj );
	if ( !bOK )
	{
		CString StrMsg;
		StrMsg.Format ( "[%s] Skin File Load Fail.", szSkinObj );
		MessageBox ( NULL, StrMsg.GetString(), "ERROR", MB_OK );
		return;
	}

	size_t nSIZE = DataDummy.m_vecANIMINFO.size();
	for ( size_t n=0; n<nSIZE; ++n )
	{
		const SANIMCONINFO *pAnimConInfo = DataDummy.m_vecANIMINFO[n];

		SANIATTACK sAniAttack;
		sAniAttack.Assign ( pAnimConInfo );

		int nIndex = pAnimConInfo->m_MainType*AN_SUB_00_SIZE + pAnimConInfo->m_SubType;
		m_pANIMATION[nIndex].push_back ( sAniAttack );
	}
}

HRESULT SCROWDATA::SaveFile ( CSerialFile &SFile )
{
	SFile << static_cast<DWORD> ( VERSION );

	SFile << static_cast<DWORD> ( FILE_SBASIC );
	{
		SFile << static_cast<DWORD> ( SCROWBASIC::VERSION );
		SFile << static_cast<DWORD> ( sizeof(SCROWBASIC) );

		CString cstrName;
		cstrName.Format( _T("CN_%03d_%03d"), sNativeID.wMainID, sNativeID.wSubID ); // by 경대
		StringCchCopy ( m_szName, CHAR_SZNAME, cstrName.GetString() );
		SFile.WriteBuffer ( &m_sBasic, sizeof(SCROWBASIC) );
	}

	SFile << static_cast<DWORD> ( FILE_SACTION );
	{
		SFile << static_cast<DWORD> ( SCROWACTION::VERSION );
		
		SFile.BeginBlock ();
		{
			m_sAction.SAVE ( SFile );
		}
		SFile.EndBlock ();
	}

	SFile << static_cast<DWORD> ( FILE_SGEN );
	{
		SFile << static_cast<DWORD> ( SCROWGEN::VERSION );
		SFile << static_cast<DWORD> ( sizeof(SCROWGEN) );
		m_sGenerate.SAVE(SFile);
	}

	SFile << static_cast<DWORD> ( FILE_SATTACK );
	{
		SFile << static_cast<DWORD> ( SCROWATTACK::VERSION );
		SFile.BeginBlock();
		{
			for( int i=0; i<EMMAXATTACK; ++i )
				m_sCrowAttack[i].SAVE ( SFile );
		}
		SFile.EndBlock();
	}

	SFile << static_cast<DWORD> ( FILE_END_DATA );

	return S_OK;
}

VECANIATTACK& SCROWDATA::GetAnimation ( EMANI_MAINTYPE emMType, EMANI_SUBTYPE emSType )
{
	static VECANIATTACK nullANIMATION;
	if ( !m_pANIMATION )							return nullANIMATION;

	int nIndex = emMType*AN_SUB_00_SIZE + emSType;
	if ( nIndex >= AN_TYPE_SIZE*AN_SUB_00_SIZE )	return nullANIMATION;

	return m_pANIMATION[nIndex];
}

const char* SCROWDATA::GetName()
{
	const char *szName = GLStringTable::GetInstance().GetString ( m_szName, GLStringTable::CROW );
	if ( !szName )	return m_szName;
	
	return szName;
}

VOID SCROWDATA::SaveCsvHead ( std::fstream &SFile )
{
	SCROWBASIC::SaveCsvHead( SFile );
	SCROWACTION::SaveCsvHead( SFile );
	SCROWGEN::SaveCsvHead( SFile );
	for( int i=0; i<EMMAXATTACK; ++i )
		SCROWATTACK::SaveCsvHead( SFile, i+1 );

	SFile << std::endl;
}

VOID SCROWDATA::SaveCsv ( std::fstream &SFile )
{
	m_sBasic.SaveCsv( SFile );
	m_sAction.SaveCsv( SFile );
	m_sGenerate.SaveCsv( SFile );
	for( int i=0; i<EMMAXATTACK; ++i )
		m_sCrowAttack[i].SaveCsv( SFile );

	SFile << std::endl;
}

VOID SCROWDATA::LoadCsv ( CStringArray &StrArray )
{
	// Csv 로딩할 때 사용
	int iCsvCur = 0;

	m_sBasic.LoadCsv( StrArray, iCsvCur );
	m_sAction.LoadCsv( StrArray, iCsvCur );
	m_sGenerate.LoadCsv( StrArray, iCsvCur );
	for( int i=0; i<EMMAXATTACK; ++i )
		m_sCrowAttack[i].LoadCsv( StrArray, iCsvCur );
}

////////////////////////////////////////////////////////////////////////////////////
//	$1.	2003-01-17 Txx:xx	[최초 릴리즈]
////////////////////////////////////////////////////////////////////////////////////


GLCrowDataMan::GLCrowDataMan () 
	: m_ppPCROWDATA(NULL)
	, m_pd3dDevice(NULL)
	, m_bModify(false)
{
	memset(m_szFileName, 0, sizeof(char) * (MAX_PATH));
}

GLCrowDataMan::~GLCrowDataMan ()
{
	FinalCleanup ();
}

HRESULT GLCrowDataMan::OneTimeSceneInit ()
{
	m_ppPCROWDATA = new PCROWDATA*[MAX_CROW_MID];
	for ( WORD i=0; i<MAX_CROW_MID; i++ )
	{
		m_ppPCROWDATA[i] = new PCROWDATA[MAX_CROW_SID];
		SecureZeroMemory ( m_ppPCROWDATA[i], sizeof(PCROWDATA)*MAX_CROW_SID );
	}

	return S_OK;
}

HRESULT GLCrowDataMan::FinalCleanup ()
{
	if ( m_ppPCROWDATA )
	{
		for ( WORD i=0; i<MAX_CROW_MID; i++ )
		{
			for ( WORD j=0; j<MAX_CROW_SID; j++ )
			{
				SAFE_DELETE(m_ppPCROWDATA[i][j]);
			}
			SAFE_DELETE_ARRAY(m_ppPCROWDATA[i]);
		}
		SAFE_DELETE_ARRAY(m_ppPCROWDATA);
	}

	return S_OK;
}

PCROWDATA GLCrowDataMan::GetMainCrowData ( WORD wMID )
{
	GASSERT(wMID<MAX_CROW_MID);

	return m_ppPCROWDATA[wMID][0];
}

PCROWDATA GLCrowDataMan::GetCrowData ( WORD wMID, WORD wSID )
{
	if ( wMID>=MAX_CROW_MID )		return NULL;
	if ( wSID>=MAX_CROW_SID )		return NULL;

	return m_ppPCROWDATA[wMID][wSID];
}

PCROWDATA GLCrowDataMan::GetCrowData ( SNATIVEID sNativeID )
{
	if ( sNativeID.wMainID>=MAX_CROW_MID )		return NULL;
	if ( sNativeID.wSubID>=MAX_CROW_SID )		return NULL;

	return m_ppPCROWDATA[sNativeID.wMainID][sNativeID.wSubID];
}

WORD GLCrowDataMan::FindFreeCrowDataMID ()
{
	for ( WORD i=0; i<MAX_CROW_MID; i++ )
	{
		if ( !m_ppPCROWDATA[i][0] )			return i;
	}

	return ITEMID_NOTFOUND;
}

WORD GLCrowDataMan::FindFreeCrowDataSID ( WORD wMID )
{
	GASSERT(wMID<MAX_CROW_MID);
	
	for ( WORD i=0; i<MAX_CROW_SID; i++ )
	{
		if ( !m_ppPCROWDATA[wMID][i] )	return i;
	}

	return ITEMID_NOTFOUND;
}

BOOL GLCrowDataMan::InsertCrowData ( WORD wMID, WORD wSID, PCROWDATA pCrowData, bool binner )
{
	GASSERT(wMID<MAX_CROW_MID);
	GASSERT(wSID<MAX_CROW_SID);

	if ( !pCrowData->m_sGenerate.m_strGenItem.empty() )
	{
		DWORD dwID;
		SGENITEM sGenItem[10];
		std::vector< string > results;
		results = StringExplode(pCrowData->m_sGenerate.m_strGenItem.c_str(), ';');
		for ( int i=0; i<results.size(); i++ )
		{
			sGenItem[i].LOADFILE ( results[i].c_str() );
			
			for( int _b = 0; _b < SGENITEM::EMGSPEC; _b++ )
			{
				dwID = sGenItem[i].m_sItemSpec[_b].dwSPECID;
				if ( dwID < 1 ) continue;
				
				if( pCrowData->m_mapMobDropSpecID.count( dwID ) == 0 )	//--check duplicate
				{
					pCrowData->m_mapMobDropSpecID.insert( std::make_pair( dwID, dwID ));	//--serverside do not insert this map
				}
			}
		}
	}


	if ( !binner )		m_bModify = true;

	if ( m_ppPCROWDATA[wMID][wSID] )
	{
		m_ppPCROWDATA[wMID][wSID]->Assign ( *pCrowData );
		return FALSE;
	}

	m_ppPCROWDATA[wMID][wSID] = new SCROWDATA;
	m_ppPCROWDATA[wMID][wSID]->Assign ( *pCrowData );

	return TRUE;
}

BOOL GLCrowDataMan::DeleteCrowData ( WORD wMID, WORD wSID )
{
	GASSERT(wMID<MAX_CROW_MID);
	GASSERT(wSID<MAX_CROW_SID);

	SAFE_DELETE ( m_ppPCROWDATA[wMID][wSID] );

	m_bModify = true;
	return TRUE;
}

BOOL GLCrowDataMan::DeleteCrowData ( SNATIVEID sNativeID )
{
	DeleteCrowData ( sNativeID.wMainID, sNativeID.wSubID );

	return TRUE;
}

HRESULT GLCrowDataMan::LoadFile ( const char* szFile, BOOL bServer, bool bPastLoad )
{
	FinalCleanup ();
	OneTimeSceneInit ();

	StringCchCopy(m_szFileName,MAX_PATH,szFile);

	char szFullPath[_MAX_PATH];
	StringCchCopy ( szFullPath, _MAX_PATH, GLOGIC::GetPath() );
	StringCchCat ( szFullPath, _MAX_PATH, m_szFileName );

	std::auto_ptr<basestream> pBStream( GLOGIC::openfile_basestream(GLOGIC::bGLOGIC_ZIPFILE, 
																	GLOGIC::strGLOGIC_ZIPFILE.c_str(), 
																	szFullPath, 
																	szFile,
																	EMBYTECRYPT_NONE,
																	GLOGIC::bGLOGIC_PACKFILE ) );

	if ( !pBStream.get() )
	{
		if ( strlen( szFile ) > 0 )
			CDebugSet::ErrorFile( "GLCrowDataMan::LoadFile", szFile );

		return E_FAIL;
	}

	basestream &SFile = *pBStream;
	
	char szFILEHEAD[FILETYPESIZE];
	DWORD dwFILEVER;
	SFile.GetFileType( szFILEHEAD, FILETYPESIZE, dwFILEVER );
	if ( dwFILEVER == 0 )								return E_FAIL;

	if ( dwFILEVER>=ENCODE_VER_OLD )
		SFile.SetEncodeType ( EMBYTECRYPT_OLD );

	if ( dwFILEVER>=ENCODE_VER )
		SFile.SetEncodeType ( EMBYTECRYPT_CROW );

	DWORD dwVer;
	SFile >> dwVer;
	
	DWORD dwNumCrow;
	SFile >> dwNumCrow;

	for ( DWORD i = 0; i < dwNumCrow; i++ )
	{
		SCROWDATA sCrowData;

		if( sCrowData.LoadFile ( SFile, bServer, bPastLoad ) == E_FAIL )
			return E_FAIL;
		InsertCrowData ( sCrowData.sNativeID, &sCrowData, true );
	}

	return S_OK;
}


HRESULT GLCrowDataMan::SaveFile ( const char* szFile )
{
	StringCchCopy(m_szFileName,MAX_PATH,szFile);

	char szFullPath[MAX_PATH] = {0};
	StringCchCopy ( szFullPath, MAX_PATH, GLOGIC::GetPath() );
	StringCchCat ( szFullPath, MAX_PATH, m_szFileName );

	CSerialFile SFile;

	SFile.SetFileType ( _FILEHEAD, VERSION );
	if ( !SFile.OpenFile ( FOT_WRITE, szFullPath ) )	return E_FAIL;
	SFile.SetEncodeType ( EMBYTECRYPT_CROW );

	SFile << (DWORD)VERSION;
	
	DWORD dwNumCrow = 0;
	for ( int i = 0; i < MAX_CROW_MID; i++ )
	{
		for ( int j = 0; j < MAX_CROW_SID; j++ )
		{
			if ( m_ppPCROWDATA[i][j] )	dwNumCrow++;
		}
	}

	SFile << dwNumCrow;

	for ( int i = 0; i < MAX_CROW_MID; i++ )
	{
		for ( int j = 0; j < MAX_CROW_SID; j++ )
		{
			if ( m_ppPCROWDATA[i][j] )
			{
				m_ppPCROWDATA[i][j]->SaveFile ( SFile );
			}
		}
	}

	m_bModify = false;
	return S_OK;
}

bool GLCrowDataMan::ValidData ()
{
	for ( int i = 0; i < MAX_CROW_MID; i++ )
	for ( int j = 0; j < MAX_CROW_SID; j++ )
	{
		if ( m_ppPCROWDATA[i][j] )		return true;
	}

	return false;
}

HRESULT GLCrowDataMan::SyncUpdateData ()
{
	CDebugSet::ClearFile ( _LOGFILE );

	for ( int i = 0; i < MAX_CROW_MID; i++ )
	for ( int j = 0; j < MAX_CROW_SID; j++ )
	{
		if ( m_ppPCROWDATA[i][j] )
		{
			if ( m_ppPCROWDATA[i][j]->GetSkinObjFile()==NULL )
			{
				CDebugSet::ToFile ( _LOGFILE, "[%03d][%03d] 스킨 파일이 지정되지 않았습니다.", i, j );
				continue;
			}

			DxSkinDataDummy DataDummy;
			BOOL bOK = DataDummy.LoadFile ( m_ppPCROWDATA[i][j]->GetSkinObjFile() );
			if ( !bOK )
			{
				CDebugSet::ToFile ( _LOGFILE, "[%03d][%03d] 스킨 파일 [%s]이 존재하지 않습니다.", i, j, m_ppPCROWDATA[i][j]->GetSkinObjFile() );
				continue;
			}

			for ( int A=0; A<SCROWDATA::EMMAXATTACK; A++ )
			{
				SCROWATTACK &sCrowAttack = m_ppPCROWDATA[i][j]->m_sCrowAttack[A];
				if ( !sCrowAttack.bUsed )	continue;

				if ( sCrowAttack.strAniFile.empty() )
				{
					CDebugSet::ToFile ( _LOGFILE, "[%03d][%03d] 공격샛[%02d]에 '모션'이 없음.", i, j, A );

					if ( A > 0 )
					{
						CDebugSet::ToFile ( _LOGFILE, "[%03d][%03d] 공격샛[%02d]에 '모션'이 없어 '비활성화' 합니다.", i, j, A );
						sCrowAttack.bUsed = FALSE;
					}

					continue;
				}

				SANIATTACK &sAniAttack = sCrowAttack.sAniAttack;
				if ( SANIMCONINFO *pAnimConInfo=DataDummy.FindAniInfo ( sCrowAttack.strAniFile.c_str() ) )
				{
					sAniAttack.Assign ( pAnimConInfo );
				}
				else
				{
					CDebugSet::ToFile ( _LOGFILE, "[%03d][%03d] 공격샛[%02d/%s]에 '모션'이 없음.", i, j, A, sCrowAttack.strAniFile.c_str() );
					if ( A > 0 )
					{
						CDebugSet::ToFile ( _LOGFILE, "[%03d][%03d] 공격샛[%02d/%s]에 '모션'이 없어 '비활성화' 합니다.", i, j, A, sCrowAttack.strAniFile.c_str() );
						sCrowAttack.bUsed = FALSE;
					}
				}
			
				if ( sAniAttack.m_wDivCount==0 )
				{
					CDebugSet::ToFile ( _LOGFILE, "[%03d][%03d] 공격샛[%02d/%s] '모션'에 '타격지점'이 없음.", i, j, A, sCrowAttack.strAniFile.c_str() );
				}
			}
		}
	}

	CDebugSet::ToFile ( _LOGFILE, "---------------------sync-completion--------------------------------" );

	return S_OK;
}

HRESULT	GLCrowDataMan::SyncStringTable()
{
	char	szFullPathFileName[MAX_PATH] = "";
	StringCchCopy ( szFullPathFileName, MAX_PATH, GLOGIC::GetPath() );
	StringCchCat ( szFullPathFileName, MAX_PATH, _STRINGTABLE );

	//DeleteFile ( szFullPathFileName );

	CString strBuffer1, strBuffer;
	//std::string strTemp;
	//std::string::size_type idx;

	strBuffer = "// File : Crow String Table\r\n";
	strBuffer += "// Note : 키(ID)와 내용은 반드시 탭으로 구분되어야 합니다.\r\n//\r\n";
	
	for ( int i = 0; i < MAX_CROW_MID; i++ )
	for ( int j = 0; j < MAX_CROW_SID; j++ )
	{
		if ( !m_ppPCROWDATA[i][j] )		continue;

		//CItemNode *pITEMNODE = m_ppItem[i][j];
		//SITEM &sITEM = pITEMNODE->m_sItem;
		
		if( m_ppPCROWDATA[i][j]->m_szName[0] != _T('\0') )
		{
			strBuffer1.Format( _T("CN_%03d_%03d\t%s\r\n"), i, j, m_ppPCROWDATA[i][j]->m_szName );
			strBuffer += strBuffer1;
		}
		
		//if( !sITEM.sBasicOp.strComment.empty() )
		//{
		//	strTemp = sITEM.sBasicOp.strComment;
		//	idx = strTemp.find( "\r\n" );
		//	while ( idx != std::string::npos )
		//	{
		//		strTemp.replace( idx, 2, " " );
		//		idx = strTemp.find( "\r\n" );
		//	}
		//
		//	strBuffer1.Format( _T("CD_%03d_%03d\t%s\r\n"), i, j, strTemp.c_str() );
		//	strBuffer += strBuffer1;
		//}
	}
	//strBuffer.SetAt(strBuffer.GetLength()-1, NULL);
	//strBuffer.SetAt(strBuffer.GetLength()-2, NULL); // 마지막 \n은 제거한다.

	//DWORD dwLength = strBuffer.GetLength() + 1;
	//char * szEncode = new char[dwLength]; // 인코딩배열에 스트링 길이만큼 메모리 할당
	//StringCchCopy( szEncode, dwLength, strBuffer.GetString() ); // 인코딩배열에 스트링 복사
	//compbyte::encode ( (BYTE*)szEncode, dwLength ); // 인코딩

	CFile file;
	file.Open( _T(szFullPathFileName), CFile::modeCreate|CFile::modeWrite ); // 파일 열기
	file.Write( strBuffer.GetString(), strBuffer.GetLength()-2 ); // 파일 쓰기
	file.Close();

	//delete [] szEncode;
	return S_OK;
}

HRESULT GLCrowDataMan::SaveCsvFile ( CWnd* pWnd )
{
	CFileDialog dlg( FALSE, ".csv", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		"csv file (*.csv)|*.csv|", pWnd );

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() != IDOK )
		return S_FALSE;

	std::fstream streamFILE;
	streamFILE.open ( dlg.GetPathName().GetString(), std::ios_base::out );

	// Csv Head
	SCROWDATA::SaveCsvHead( streamFILE );

	for( int i=0; i<MAX_CROW_MID; ++i )
	{
		for( int j=0; j<MAX_CROW_SID; ++j )
		{
			if( !m_ppPCROWDATA[i][j] )
				continue;

			// Csv Data
			SCROWDATA *pCROWDATA = m_ppPCROWDATA[i][j];
			pCROWDATA->SaveCsv( streamFILE );
		}
	}

	streamFILE.close();

	return S_OK;
}

HRESULT GLCrowDataMan::LoadCsvFile ( CWnd* pWnd )
{
	CFileDialog dlg( TRUE, ".csv", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		"csv file (*.csv)|*.csv|", pWnd );

	dlg.m_ofn.lpstrInitialDir = GLOGIC::GetPath ();
	if ( dlg.DoModal() != IDOK )
		return S_FALSE;

	CStringFile StrFile( 10240 );
	if( !StrFile.Open ( dlg.GetPathName().GetString() ) )
		return S_FALSE;

	STRUTIL::ClearSeparator();
	STRUTIL::RegisterSeparator( "," );

	CString strLine;
	CStringArray StrArray;

	// Csv Head
	StrFile.GetNextLine( strLine );
	STRUTIL::StringSeparate( strLine, StrArray );
	int iHead = (int)StrArray.GetCount();	

	int iLine = 1;
	char szError[ 256 ];

	while( StrFile.GetNextLine( strLine ) )
	{
		++iLine;

		STRUTIL::StringSeparate( strLine, StrArray );

		int iCount = (int)StrArray.GetCount();
		if( iCount < iHead )
		{
			StringCchPrintf( szError, 256, "%d Line = include blank column, next item load failed", iLine );
			MessageBox( pWnd->GetSafeHwnd(), _T(szError), _T("Fail"), MB_OK );
			return S_FALSE;
		}

		int iMID = atoi( StrArray[0] );
		int iSID = atoi( StrArray[1] );

		if( iMID >= MAX_CROW_MID || iSID >= MAX_CROW_SID )
			continue;

		if( !m_ppPCROWDATA[ iMID ][ iSID ] )
		{
			SCROWDATA pCROWDATA;
			if( !InsertCrowData( iMID, iSID, &pCROWDATA ) )
				continue;
		}

		// Csv Data
		SCROWDATA *pCROWDATA = m_ppPCROWDATA[ iMID ][ iSID ];
		pCROWDATA->LoadCsv( StrArray );
	}

	return S_OK;
}


