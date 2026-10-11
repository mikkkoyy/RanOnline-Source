// VERTICAL-003: passive skill definition implementation.

#include "skills/SkillDefinition.h"

#include <algorithm>

namespace Modern
{
	namespace
	{
		constexpr bool IsInRange(int value, int min, int max)
		{
			return value >= min && value <= max;
		}
	}

	SkillWeaponType LegacyWeaponTypeToModern(int legacyType)
	{
		// Map from SKILL::GLSKILL_ATT (GLSkillBasic.h) to modern enum.
		// Only map values that actually exist in the modern enum.
		switch (legacyType)
		{
			case 0:  return SkillWeaponType::None;
			case 1:  return SkillWeaponType::Sword;
			case 2:  return SkillWeaponType::Blade;
			case 3:  return SkillWeaponType::SwordBlade;
			case 4:  return SkillWeaponType::Dagger;
			case 5:  return SkillWeaponType::Spear;
			case 6:  return SkillWeaponType::Stick;
			case 7:  return SkillWeaponType::Gauntlet;
			case 8:  return SkillWeaponType::Bow;
			case 9:  return SkillWeaponType::Throw;
			case 10: return SkillWeaponType::DualGun;
			case 11: return SkillWeaponType::RailGun;
			case 12: return SkillWeaponType::PortalGun;
			case 13: return SkillWeaponType::Gun;
			case 14: return SkillWeaponType::Shotgun;
			case 15: return SkillWeaponType::Scythe;
			case 16: return SkillWeaponType::DualSpear;
			case 17: return SkillWeaponType::Shuriken;
			case 18: return SkillWeaponType::Fist;
			case 19: return SkillWeaponType::Wand;
			case 20: return SkillWeaponType::Cube;
			case 21: return SkillWeaponType::Whip;
			case 22: return SkillWeaponType::NoCare;
			case 23: return SkillWeaponType::Shield;
			case 24: return SkillWeaponType::Hammer;
			case 25: return SkillWeaponType::Umbrella;
		}
		return SkillWeaponType::NoCare;
	}

	int ModernWeaponTypeToLegacy(SkillWeaponType type)
	{
		switch (type)
		{
			case SkillWeaponType::None:          return 0;
			case SkillWeaponType::Sword:         return 1;
			case SkillWeaponType::Blade:         return 2;
			case SkillWeaponType::SwordBlade:    return 3;
			case SkillWeaponType::Dagger:        return 4;
			case SkillWeaponType::Spear:         return 5;
			case SkillWeaponType::Stick:         return 6;
			case SkillWeaponType::Gauntlet:      return 7;
			case SkillWeaponType::Bow:           return 8;
			case SkillWeaponType::Throw:         return 9;
			case SkillWeaponType::DualGun:       return 10;
			case SkillWeaponType::RailGun:       return 11;
			case SkillWeaponType::PortalGun:     return 12;
			case SkillWeaponType::Gun:           return 13;
			case SkillWeaponType::Shotgun:       return 14;
			case SkillWeaponType::Scythe:        return 15;
			case SkillWeaponType::DualSpear:     return 16;
			case SkillWeaponType::Shuriken:      return 17;
			case SkillWeaponType::Fist:          return 18;
			case SkillWeaponType::Wand:          return 19;
			case SkillWeaponType::Cube:          return 20;
			case SkillWeaponType::Whip:          return 21;
			case SkillWeaponType::NoCare:        return 22;
			case SkillWeaponType::Shield:        return 23;
			case SkillWeaponType::Hammer:        return 24;
			case SkillWeaponType::Umbrella:      return 25;
		}
		return 22; // NoCare
	}

	PassiveApplyType LegacyBasicTypeToModern(int legacyType)
	{
		// Map from SKILL::EMTYPES (GLSkillApply.h) to modern enum.
		if (legacyType >= 0 && legacyType <= 31)
		{
			return static_cast<PassiveApplyType>(legacyType);
		}
		return PassiveApplyType::Hp;
	}

	PassiveImpactType LegacyImpactTypeToModern(int legacyType)
	{
		// Map from SKILL::EMIMPACT_ADDON (GLSkillApply.h) to modern enum.
		// Only the values that exist in our enum.
		switch (legacyType)
		{
			case 0:  return PassiveImpactType::None;
			case 1:  return PassiveImpactType::HitRate;
			case 2:  return PassiveImpactType::AvoidRate;
			case 3:  return PassiveImpactType::Damage;
			case 4:  return PassiveImpactType::Defense;
			case 5:  return PassiveImpactType::VarHp;
			case 6:  return PassiveImpactType::VarMp;
			case 7:  return PassiveImpactType::VarSp;
			case 8:  return PassiveImpactType::VarAp;
			case 9:  return PassiveImpactType::DamageRate;
			case 10: return PassiveImpactType::DefenseRate;
			case 11: return PassiveImpactType::Pa;
			case 12: return PassiveImpactType::Sa;
			case 13: return PassiveImpactType::Ma;
			case 14: return PassiveImpactType::HpRate;
			case 15: return PassiveImpactType::MpRate;
			case 16: return PassiveImpactType::SpRate;
			case 17: return PassiveImpactType::Resist;
		}
		return PassiveImpactType::None;
	}

	PassiveSpecType LegacySpecTypeToModern(int legacyType)
	{
		// Most legacy specs don't feed the stat pipeline.
		// If we add specs that do, map them here.
		(void)legacyType;
		return PassiveSpecType::None;
	}

	const char* ToString(SkillWeaponType type) noexcept
	{
		switch (type)
		{
			case SkillWeaponType::None:          return "None";
			case SkillWeaponType::Sword:         return "Sword";
			case SkillWeaponType::Blade:         return "Blade";
			case SkillWeaponType::SwordBlade:    return "SwordBlade";
			case SkillWeaponType::Dagger:        return "Dagger";
			case SkillWeaponType::Spear:         return "Spear";
			case SkillWeaponType::Stick:         return "Stick";
			case SkillWeaponType::Gauntlet:      return "Gauntlet";
			case SkillWeaponType::Bow:           return "Bow";
			case SkillWeaponType::Throw:         return "Throw";
			case SkillWeaponType::DualGun:       return "DualGun";
			case SkillWeaponType::RailGun:       return "RailGun";
			case SkillWeaponType::PortalGun:     return "PortalGun";
			case SkillWeaponType::Gun:           return "Gun";
			case SkillWeaponType::Shotgun:       return "Shotgun";
			case SkillWeaponType::Scythe:        return "Scythe";
			case SkillWeaponType::DualSpear:     return "DualSpear";
			case SkillWeaponType::Shuriken:      return "Shuriken";
			case SkillWeaponType::Fist:          return "Fist";
			case SkillWeaponType::Wand:          return "Wand";
			case SkillWeaponType::Cube:          return "Cube";
			case SkillWeaponType::Whip:          return "Whip";
			case SkillWeaponType::NoCare:        return "NoCare";
			case SkillWeaponType::Shield:        return "Shield";
			case SkillWeaponType::Hammer:        return "Hammer";
			case SkillWeaponType::Umbrella:      return "Umbrella";
		}
		return "Unknown";
	}

	const char* ToString(PassiveApplyType type) noexcept
	{
		switch (type)
		{
			case PassiveApplyType::Hp:          return "Hp";
			case PassiveApplyType::Mp:          return "Mp";
			case PassiveApplyType::Sp:          return "Sp";
			case PassiveApplyType::VarHp:       return "VarHp";
			case PassiveApplyType::VarMp:       return "VarMp";
			case PassiveApplyType::VarSp:       return "VarSp";
			case PassiveApplyType::Defense:     return "Defense";
			case PassiveApplyType::HitRate:     return "HitRate";
			case PassiveApplyType::AvoidRate:   return "AvoidRate";
			case PassiveApplyType::VarAp:       return "VarAp";
			case PassiveApplyType::VarDamage:   return "VarDamage";
			case PassiveApplyType::VarDefense:  return "VarDefense";
			case PassiveApplyType::Pa:          return "Pa";
			case PassiveApplyType::Sa:          return "Sa";
			case PassiveApplyType::Ma:          return "Ma";
			case PassiveApplyType::HpRate:      return "HpRate";
			case PassiveApplyType::MpRate:      return "MpRate";
			case PassiveApplyType::SpRate:      return "SpRate";
			case PassiveApplyType::Resist:      return "Resist";
			case PassiveApplyType::SummonTime:  return "SummonTime";
		}
		return "Unknown";
	}

	const char* ToString(PassiveImpactType type) noexcept
	{
		switch (type)
		{
			case PassiveImpactType::None:         return "None";
			case PassiveImpactType::HitRate:      return "HitRate";
			case PassiveImpactType::AvoidRate:    return "AvoidRate";
			case PassiveImpactType::Damage:       return "Damage";
			case PassiveImpactType::Defense:      return "Defense";
			case PassiveImpactType::VarHp:        return "VarHp";
			case PassiveImpactType::VarMp:        return "VarMp";
			case PassiveImpactType::VarSp:        return "VarSp";
			case PassiveImpactType::VarAp:        return "VarAp";
			case PassiveImpactType::DamageRate:   return "DamageRate";
			case PassiveImpactType::DefenseRate:  return "DefenseRate";
			case PassiveImpactType::Pa:           return "Pa";
			case PassiveImpactType::Sa:           return "Sa";
			case PassiveImpactType::Ma:           return "Ma";
			case PassiveImpactType::HpRate:       return "HpRate";
			case PassiveImpactType::MpRate:       return "MpRate";
			case PassiveImpactType::SpRate:       return "SpRate";
			case PassiveImpactType::Resist:       return "Resist";
		}
		return "Unknown";
	}

	const char* ToString(PassiveSpecType type) noexcept
	{
		switch (type)
		{
			case PassiveSpecType::None: return "None";
		}
		return "Unknown";
	}

	// VERTICAL-011: active-skill enum names.
	const char* ToString(SkillRole type) noexcept
	{
		switch (type)
		{
			case SkillRole::Normal:  return "Normal";
			case SkillRole::Passive: return "Passive";
		}
		return "Unknown";
	}

	const char* ToString(SkillApply type) noexcept
	{
		switch (type)
		{
			case SkillApply::PhysicalMelee:  return "PhysicalMelee";
			case SkillApply::PhysicalRanged: return "PhysicalRanged";
			case SkillApply::Magic:          return "Magic";
		}
		return "Unknown";
	}

	const char* ToString(SkillTargetKind type) noexcept
	{
		switch (type)
		{
			case SkillTargetKind::Self:       return "Self";
			case SkillTargetKind::Spec:       return "Spec";
			case SkillTargetKind::SelfToSpec: return "SelfToSpec";
			case SkillTargetKind::Zone:       return "Zone";
			case SkillTargetKind::Specific:   return "Specific";
		}
		return "Unknown";
	}

	const char* ToString(SkillImpactSide type) noexcept
	{
		switch (type)
		{
			case SkillImpactSide::Our:     return "Our";
			case SkillImpactSide::Enemy:   return "Enemy";
			case SkillImpactSide::Anybody: return "Anybody";
		}
		return "Unknown";
	}

	const char* ToString(SkillElement type) noexcept
	{
		switch (type)
		{
			case SkillElement::Spirit:   return "Spirit";
			case SkillElement::Fire:     return "Fire";
			case SkillElement::Ice:      return "Ice";
			case SkillElement::Electric: return "Electric";
			case SkillElement::Stone:    return "Stone";
			case SkillElement::Mad:      return "Mad";
			case SkillElement::Poison:   return "Poison";
			case SkillElement::Curse:    return "Curse";
			case SkillElement::Zen:      return "Zen";
			case SkillElement::ArmWeapon: return "ArmWeapon";
		}
		return "Unknown";
	}
}
// SKILL-002: EMELEMENT -> SkillElement. See the declaration for why this
// cannot be a cast.
bool Modern::LegacyElementToModern(int legacyElement, Modern::SkillElement& out) noexcept
{
	switch (legacyElement)
	{
	case 0: out = Modern::SkillElement::Spirit;   return true;   // EMELEMENT_SPIRIT
	case 1: out = Modern::SkillElement::Fire;     return true;   // EMELEMENT_FIRE
	case 2: out = Modern::SkillElement::Ice;      return true;   // EMELEMENT_ICE
	case 3: out = Modern::SkillElement::Electric; return true;   // EMELEMENT_ELECTRIC
	case 4: out = Modern::SkillElement::Poison;   return true;   // EMELEMENT_POISON
	case 5: out = Modern::SkillElement::Stone;    return true;   // EMELEMENT_STONE
	case 6: out = Modern::SkillElement::Mad;      return true;   // EMELEMENT_MAD
	case 8: out = Modern::SkillElement::Curse;    return true;   // EMELEMENT_CURSE
	case 9: out = Modern::SkillElement::ArmWeapon; return true;  // EMELEMENT_ARM
	default: return false;  // EMELEMENT_STUN (7) and anything unknown
	}
}

// SKILL-003: EMIMPACT_ADDON -> PassiveImpactType. Identical for 0..17; legacy
// 18..23 have no modern name and are reported rather than guessed.
bool Modern::LegacyImpactTypeToModern(int legacyType, Modern::PassiveImpactType& out) noexcept
{
	if (legacyType < 0 || legacyType > 17)
	{
		return false;
	}
	out = static_cast<Modern::PassiveImpactType>(legacyType);
	return true;
}
