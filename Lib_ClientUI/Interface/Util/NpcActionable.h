#pragma	once

class GLGaeaClient;

class CUIControlNpcActionable
{

public:
    CUIControlNpcActionable();
    ~CUIControlNpcActionable();

public:
    void SetNpcActionableNPCID( const SNATIVEID& sNPCID, const DWORD dwNPCGlobalID );
    void SetNpcActionableRemote();

public:
    const bool IsUseable ();

public:
    SNATIVEID  GetNPCID       () { return m_sNPCID; }
    DWORD      GetNPCGlobalID () { return m_dwNPCGlobalID; }

public:
    const bool IsRemoteOpen () { return m_bRemoteOpen; }

private:
    SNATIVEID        m_sNPCID;
    DWORD            m_dwNPCGlobalID;
    bool             m_bRemoteOpen;

};
