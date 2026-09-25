#pragma once

#include <cstdint>

namespace Modern
{
	struct EntityId
	{
		uint32_t value;

		static constexpr uint32_t InvalidValue = 0xFFFFFFFFu;

		constexpr EntityId() : value(InvalidValue) {}
		constexpr explicit EntityId(uint32_t v) : value(v) {}

		constexpr bool IsValid() const { return value != InvalidValue; }
		constexpr bool operator==(const EntityId& o) const { return value == o.value; }
		constexpr bool operator!=(const EntityId& o) const { return value != o.value; }
		constexpr bool operator<(const EntityId& o) const { return value < o.value; }

		static constexpr EntityId MakeInvalid() { return EntityId(InvalidValue); }
	};

	enum class ActionType : uint32_t
	{
		Idle     = 0,
		Move     = 1,
		Attack   = 2,
		Skill    = 3,
		Shock    = 4,
		PushPull = 5,
		SkillMove= 6,
		Gathering= 7,
		Talk     = 8,
		ConftEnd = 9,
		Falling  = 10,
		Die      = 11,
		SkillDash= 12,
		Size     = 13,
	};

	namespace ActState
	{
		constexpr uint32_t None         = 0x00000000u;
		constexpr uint32_t Run          = 0x00000001u;
		constexpr uint32_t ContinueMove = 0x00000002u;
		constexpr uint32_t Peaceful     = 0x00000004u;
		constexpr uint32_t GetViewAfter = 0x00000010u;
		constexpr uint32_t Waiting      = 0x00000020u;
		constexpr uint32_t Dead         = 0x00000040u;
		constexpr uint32_t GateOut      = 0x00000100u;
		constexpr uint32_t Logout       = 0x00000200u;
		constexpr uint32_t VisibleNone  = 0x00001000u;
		constexpr uint32_t VisibleOff   = 0x00002000u;
		constexpr uint32_t ConftWin     = 0x00010000u;
		constexpr uint32_t PkMode       = 0x00100000u;
		constexpr uint32_t VehicleBoost = 0x01000000u;
	}

	constexpr bool IsDeadAction(ActionType a) { return a == ActionType::Die; }
}
