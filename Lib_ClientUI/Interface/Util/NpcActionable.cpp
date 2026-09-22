#include "StdAfx.h"

#include "../Lib_Engine/G-Logic/GLDefine.h"

#include "../Lib_Client/G-Logic/GLGaeaClient.h"
#include "../Lib_Client/G-Logic/GLLandManClient.h"
#include "../Lib_Client/G-Logic/GLCrowDataAction.h"
#include "../Lib_Client/G-Logic/GLCharacter.h"

#include "../InnerInterface.h"

#include "NpcActionable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CUIControlNpcActionable::CUIControlNpcActionable()
: m_sNPCID(SNATIVEID(false))
, m_dwNPCGlobalID(EMTARGET_NULL)
, m_bRemoteOpen(false)
{
    //Blank
}

CUIControlNpcActionable::~CUIControlNpcActionable()
{
    //Blank
}

const bool CUIControlNpcActionable::IsUseable ()
{
    if ( !m_bRemoteOpen )
    {
		GLLandManClient* pLandMan = GLGaeaClient::GetInstance().GetActiveMap();

        if ( !pLandMan )
        {
            return false;
        }

        PGLCROWCLIENT pCrow = pLandMan->GetCrow( m_dwNPCGlobalID );
        if ( !pCrow )
        {
            return false;
        }

        /*else if ( !pCrow->IsNpcActionable(pGaeaClient->GetCharacter()->GetPosition(), pGaeaClient->GetCharacter()->GETBODYRADIUS() ) )
        {
            return false;
        }*/
    }

    return true;
}

void CUIControlNpcActionable::SetNpcActionableNPCID( const SNATIVEID& sNPCID, const DWORD dwNPCGlobalID )
{
    m_sNPCID = sNPCID;
    m_dwNPCGlobalID = dwNPCGlobalID;
    m_bRemoteOpen = false;
}

void CUIControlNpcActionable::SetNpcActionableRemote()
{
    m_sNPCID = SNATIVEID(false);
    m_dwNPCGlobalID = EMTARGET_NULL;
    m_bRemoteOpen = true;
}
