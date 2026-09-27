#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "../Lib_Client/G-Logic/GLCharDefine.h"

class CCharacterCreatePageSet : public CUIGroup
{
private:
	enum
	{
		CHARACTER_CREATE_PAGE_TYPE_SCHOOL_0 = NO_ID + 1,
		CHARACTER_CREATE_PAGE_TYPE_SCHOOL_1,
		CHARACTER_CREATE_PAGE_TYPE_SCHOOL_2,
		CHARACTER_CREATE_PAGE_TYPE_SEX_MALE,
		CHARACTER_CREATE_PAGE_TYPE_SEX_FEMALE,
		CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_BRAWLER,
		CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_SWORDSMAN,
		CHARACTER_CREATE_PAGE_TYPE_CLASS_FEMALE_ARCHER,
		CHARACTER_CREATE_PAGE_TYPE_CLASS_FEMALE_SHAMAN,
		CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_EXTREME,
		CHARACTER_CREATE_PAGE_TYPE_CLASS_FEMALE_EXTREME,
		CHARACTER_CREATE_PAGE_TYPE_CLASS_FEMALE_BRAWLER,
		CHARACTER_CREATE_PAGE_TYPE_CLASS_FEMALE_SWORDSMAN,
		CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_ARCHER,
		CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_SHAMAN,
		CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_GUNNER,
		CHARACTER_CREATE_PAGE_TYPE_CLASS_FEMALE_GUNNER,
		CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_ASSASSIN,
		CHARACTER_CREATE_PAGE_TYPE_CLASS_FEMALE_ASSASSIN,
		CHARACTER_CREATE_PAGE_TYPE_CLASS_MALE_TRICKER,
		CHARACTER_CREATE_PAGE_TYPE_CLASS_FEMALE_TRICKER,
	};

public:
	CCharacterCreatePageSet ();
	virtual	~CCharacterCreatePageSet ();

public:
	void CreateSubControl ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

private:
	CUIControl*		m_pTypeSchool_Off[GLSCHOOL_NUM];
	CUIControl*		m_pTypeSchool_On[GLSCHOOL_NUM];
	CUIControl*		m_pTypeSchool_Box[GLSCHOOL_NUM];
	CUIControl*		m_pTypeSchool_Focus[GLSCHOOL_NUM];

	CUIControl*		m_pTypeSex_Off[2];
	CUIControl*		m_pTypeSex_On[2];
	CUIControl*		m_pTypeSex_Box[2];
	CUIControl*		m_pTypeSex_Focus[2];

	CUIControl*		m_pTypeClass_Off[GLCI_NUM_8CLASS];
	CUIControl*		m_pTypeClass_On[GLCI_NUM_8CLASS];
	CUIControl*		m_pTypeClass_Box[GLCI_NUM_8CLASS];
	CUIControl*		m_pTypeClass_Focus[GLCI_NUM_8CLASS];


public:
	BOOL	m_bUseCancel;
	BOOL	m_bCreateClassExtremeM;
	BOOL	m_bCreateClassExtremeW;
	BOOL	m_bCreateClassGunner;
	BOOL	m_bCreateClassAssassin;
	BOOL	m_bCreateClassMagician;

	int		m_nSelectedLastSchool;
	int		m_nSelectedLastSex;
	int		m_nSelectedLastClass;
	int		m_nSelectedSchool;
	int		m_nSelectedSex;
	int		m_nSelectedClass;

public:
	void	ResetAll();
	void	SetCreateExtremeM( BOOL bCreate ) { m_bCreateClassExtremeM = bCreate; }
	void	SetCreateExtremeW( BOOL bCreate ) { m_bCreateClassExtremeW = bCreate; }
	void	UpdateUI();
	void	SchoolSelect( int nSelect );
	void	SexSelect( int nSelect );
	void	ClassSelect( int nSelect );
	void	SchoolMouseIn( int nSelect );
	void	SexMouseIn( int nSelect );
	void	ClassMouseIn( int nSelect );
};