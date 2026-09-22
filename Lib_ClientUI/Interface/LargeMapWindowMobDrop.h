#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GlogicData.h"

class CBasicTextButton;
class CBasicTextBox;
class CBasicButtonText;
class CBasicTextBoxEx;
class CBasicScrollBarEx;

typedef std::map<DWORD,SNATIVEID>	MOB_DROPITEM_MAP;
typedef MOB_DROPITEM_MAP::iterator 	MOB_DROPITEM_MAP_ITER;

//--------------------------------------------------------------------
// CLargeMapWindowMobDrop : //
//--------------------------------------------------------------------
class CLargeMapWindowMobDrop : public CUIGroup	
{
protected:
	enum
	{
		ITEM_LIST_TEXTBOX = NO_ID + 1,
		ITEM_LIST_SCROLLBAR
	};
	
///////////////////////////////////////////////////////////////////
public:
	MOB_DROPITEM_MAP 			m_mapMobDropItemList_tmp;
	
///////////////////////////////////////////////////////////////////


protected:
	CUIControl*			m_pBackImage;

	bool 	m_RenderInitialized; 
	
private:
	CBasicTextBox*						CreateStaticControl ( char* szControlKeyword, CD3DFontPar* pFont, int nAlign );
	CBasicTextBox*						m_pResourceText;
	
	CBasicTextBoxEx*					m_pListText;	//--original
	CBasicScrollBarEx*					m_pListScrollBar;

	int									m_nSelectIndex;
	bool								m_bRefresh;

protected:
	SNATIVEID							m_sNativeID;

public:
	DWORD	m_dwKey;

public:
	CLargeMapWindowMobDrop();
	virtual	~CLargeMapWindowMobDrop();

public:
	VOID CreateSubControl();
	VOID CreateSubControlEx();
	void Reset();
	void Insert( DWORD dwID, SNATIVEID sID );
	void LoadItemList( bool bRefresh );

protected:
	CBasicTextBox*	CreateTextBox ( const char* szTextBox );

public:
	virtual	VOID Update( INT x, INT y, BYTE LB, BYTE MB, BYTE RB, INT nScroll, FLOAT fElapsedTime, BOOL bFirstControl );
	virtual	VOID TranslateUIMessage( UIGUID ControlID, DWORD dwMsg );
	virtual void SetVisibleSingle( BOOL bVisible );
};

/////////////////////////////////////
class MOB_DROPITEM_MAP_CMP
{
public:
	bool operator() ( const SNATIVEID &lvalue, const SNATIVEID &rvalue )
	{
		if ( lvalue.wMainID < rvalue.wMainID ) return true;
		else if ( lvalue.wMainID == rvalue.wMainID ) 
		{	
			if ( lvalue.wSubID < rvalue.wSubID ) return true;
		}
		
		return false;
	}
};
/////////////////////////////////////