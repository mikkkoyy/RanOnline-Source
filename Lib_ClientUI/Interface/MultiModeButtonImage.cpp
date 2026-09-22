#include "StdAfx.h"
#include "MultiModeButtonImage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMultiModeButtonImage::CMultiModeButtonImage ()
{
}

CMultiModeButtonImage::~CMultiModeButtonImage ()
{
}

void CMultiModeButtonImage::CreateButtonImage ( const char* szLeft, const char* szCenter, const char* szRight )
{
	CUIControl* pLeft = new CUIControl;
	pLeft->CreateSub ( this, szLeft, UI_FLAG_YSIZE );
	RegisterControl ( pLeft );

	CUIControl* pCenter = new CUIControl;
	pCenter->CreateSub ( this, szCenter, UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	RegisterControl ( pCenter );

	CUIControl* pRight = new CUIControl;
	pRight->CreateSub ( this, szRight, UI_FLAG_RIGHT | UI_FLAG_YSIZE );
	RegisterControl ( pRight );
}