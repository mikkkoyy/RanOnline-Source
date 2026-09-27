

#pragma once

//character Setting
struct SCHARSET
{
	int nUserLevel;
	BOOL bMaxLevel;
	BOOL bMaxStats;
	BOOL bAllSkills;
	BOOL b1BGold;
	BOOL b100KContrib;
	BOOL b100KActivity;
	std::string strCharSet;
	std::string strCharName;

	SCHARSET()
		: nUserLevel( 0 )
		, bMaxLevel( FALSE )
		, bMaxStats( FALSE )
		, bAllSkills( FALSE )
		, b1BGold( FALSE )
		, b100KContrib( FALSE )
		, b100KActivity( FALSE )
	{
		strCharSet = "class00.charset";
		strCharName = "Admin";
	};

	void Assign( SCHARSET sProp )
	{
		nUserLevel	= sProp.nUserLevel;
		bMaxLevel	= sProp.bMaxLevel;
		bMaxStats	= sProp.bMaxStats;
		bAllSkills	= sProp.bAllSkills;
		b1BGold		= sProp.b1BGold;
		b100KContrib = sProp.b100KContrib;
		b100KActivity	= sProp.b100KActivity;
		strCharSet	= sProp.strCharSet;
		strCharName	= sProp.strCharName;
	};
};