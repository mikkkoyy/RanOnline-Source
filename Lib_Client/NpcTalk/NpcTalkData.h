/*!
 * \file NpcTalkData.h
 *
 * \author Juver
 * \date May 2017
 *
 * 
 */
#pragma	once

struct SNPC_ITEM
{
	DWORD dwItemID;
	WORD wItemNum;

	SNPC_ITEM()
		: dwItemID(UINT_MAX)
		, wItemNum(1)
	{
	}

	void Reset()
	{
		dwItemID = UINT_MAX;
		wItemNum = 1;
	}

	bool operator < ( const SNPC_ITEM& rvalue ) const
	{
		return dwItemID < rvalue.dwItemID;
	}

	bool operator == ( const SNPC_ITEM& rvalue ) const
	{
		return dwItemID == rvalue.dwItemID;
	}
};

struct	SSkillCondition
{
	DWORD	dwNID;
	WORD	wLevel;

	SSkillCondition () :
		dwNID(UINT_MAX),
		wLevel(0xFFFF)
	{
	}

	bool operator < ( const SSkillCondition& rvalue ) const
	{
		return dwNID < rvalue.dwNID;
	}

	bool operator == ( const SSkillCondition& rvalue ) const
	{
		return dwNID == rvalue.dwNID;
	}
};

/*npc talk buff condition, Juver, 2017/10/02 */
struct SNPC_BUFF
{
	DWORD dwID;
	WORD wLevel;

	SNPC_BUFF()
		: dwID(UINT_MAX)
		, wLevel(1)
	{
	}

	bool operator < ( const SNPC_BUFF& rvalue ) const
	{
		return dwID < rvalue.dwID;
	}

	bool operator == ( const SNPC_BUFF& rvalue ) const
	{
		return dwID == rvalue.dwID;
	}
};