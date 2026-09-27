#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CCharacterCreateInfoSchool;
class CCharacterCreateInfoSex;
class CCharacterCreateInfoClass;

class CCharacterCreateInfo : public CUIGroup
{
private:
	enum
	{
		CHARACTER_CREATE_INFO_SCHOOL = NO_ID + 1,
		CHARACTER_CREATE_INFO_SEX,
		CHARACTER_CREATE_INFO_CLASS,
	};

public:
	CCharacterCreateInfo ();
	virtual	~CCharacterCreateInfo ();

public:
	void CreateSubControl ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

private:
	int m_nSchoolLast;
	int m_nSexLast;
	int m_nClassLast;

private:
	CCharacterCreateInfoSchool*		m_pInfoSchool;
	CCharacterCreateInfoSex*		m_pInfoSex;
	CCharacterCreateInfoClass*		m_pInfoClass;

public:
	void ResetAll();
	void SetData( int nSchool, int nSex, int nClass );

};