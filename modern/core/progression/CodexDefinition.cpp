#include "progression/CodexDefinition.h"

namespace Modern
{
	const char* ToString(CodexType type) noexcept
	{
		switch (type)
		{
		case CodexType::ReachLevel:    return "ReachLevel";
		case CodexType::KillMob:       return "KillMob";
		case CodexType::KillPlayer:    return "KillPlayer";
		case CodexType::ReachMap:      return "ReachMap";
		case CodexType::TakeItem:      return "TakeItem";
		case CodexType::UseItem:       return "UseItem";
		case CodexType::ReachCodex:    return "ReachCodex";
		case CodexType::CompleteQuest: return "CompleteQuest";
		case CodexType::CodexPoint:    return "CodexPoint";
		case CodexType::QuestionBox:   return "QuestionBox";
		case CodexType::Etc:           return "Etc";
		case CodexType::Size:          break;
		}
		return "<invalid codex type>";
	}
}
