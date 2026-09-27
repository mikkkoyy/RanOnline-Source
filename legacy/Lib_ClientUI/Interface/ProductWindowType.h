/*!
 * \file ProductWindowType.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */
#pragma	once

#include "BasicLineBoxSmart.h"
#include "../Lib_Engine/GUInterface/UIGroup.h"

class	CProductWindowTypeBase;
class	CBasicLineBoxSmart;

const DWORD UIMSG_PRODUCT_WINDOW_TYPE_SELECT = UIMSG_USER1;

class CProductWindowType : public CBasicLineBoxSmart
{
private:
	enum
	{
		PRODUCT_WINDOW_TYPE_ALL = NO_ID + 1,
		PRODUCT_WINDOW_TYPE_HATS,
		PRODUCT_WINDOW_TYPE_UPPER,
		PRODUCT_WINDOW_TYPE_LOWER,
		PRODUCT_WINDOW_TYPE_GLOVES,
		PRODUCT_WINDOW_TYPE_SHOES,
		PRODUCT_WINDOW_TYPE_WEAPONS,
		PRODUCT_WINDOW_TYPE_NECK,
		PRODUCT_WINDOW_TYPE_BRACELET,
		PRODUCT_WINDOW_TYPE_RING,
		PRODUCT_WINDOW_TYPE_ETC,
		PRODUCT_WINDOW_TYPE_BOX,
		PRODUCT_WINDOW_TYPE_COSTUME,
	};

public:
	enum
	{
		PRODUCT_WINDOW_TYPE_SIZE = 13,
	};

public:
	CProductWindowType ();
	virtual	~CProductWindowType ();

public:
	CProductWindowTypeBase*		m_pIconType[PRODUCT_WINDOW_TYPE_SIZE];
	
public:
	int			m_nSelect;

public:
	void	CreateSubControl ();
	int		GetSelect()	{ return m_nSelect; }
	void	DefaultSelect();

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

};