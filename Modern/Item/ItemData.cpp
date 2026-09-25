#include "ItemData.h"

#include <algorithm>
#include <vector>
#include <cstdint>

namespace Modern
{
	// ItemAddon and ItemVariate operators
	ItemContribution& ItemContribution::operator+=(const ItemContribution& other)
	{
		hp += other.hp;
		mp += other.mp;
		sp += other.sp;

		hpRate += other.hpRate;
		mpRate += other.mpRate;
		spRate += other.spRate;

		hpVolume += other.hpVolume;
		mpVolume += other.mpVolume;
		spVolume += other.spVolume;

		pow += other.pow;
		str += other.str;
		spi += other.spi;
		dex += other.dex;
		intel += other.intel;
		sta += other.sta;

		damageLow += other.damageLow;
		damageHigh += other.damageHigh;
		defense += other.defense;
		hitRate += other.hitRate;
		avoidRate += other.avoidRate;

		pa += other.pa;
		sa += other.sa;
		ma += other.ma;

		resistFire += other.resistFire;
		resistIce += other.resistIce;
		resistElec += other.resistElec;
		resistPoison += other.resistPoison;
		resistSpirit += other.resistSpirit;

		hpIncRate += other.hpIncRate;
		mpIncRate += other.mpIncRate;
		spIncRate += other.spIncRate;

		hpPotionRate += other.hpPotionRate;
		mpPotionRate += other.mpPotionRate;
		spPotionRate += other.spPotionRate;

		moveSpeedRate += other.moveSpeedRate;
		atkSpeedRate += other.atkSpeedRate;

		criticalRate += other.criticalRate;
		crushingBlowRate += other.crushingBlowRate;

		decDmgMelee += other.decDmgMelee;
		decDmgRange += other.decDmgRange;
		decDmgMagic += other.decDmgMagic;

		hitRatePer += other.hitRatePer;
		avoidRatePer += other.avoidRatePer;

		return *this;
	}

	// Helper: map EquipPosition to EquipSlot
	static EquipSlot EquipPosToSlot(EquipPosition pos)
	{
		switch (static_cast<int>(pos))
		{
			case static_cast<int>(EquipPosition::Headgear):   return EquipSlot::Headgear;
			case static_cast<int>(EquipPosition::Upper):      return EquipSlot::Upper;
			case static_cast<int>(EquipPosition::Lower):      return EquipSlot::Lower;
			case static_cast<int>(EquipPosition::Hand):       return EquipSlot::Hand;
			case static_cast<int>(EquipPosition::Foot):       return EquipSlot::Foot;
			case static_cast<int>(EquipPosition::RHand):      return EquipSlot::RHand;
			case static_cast<int>(EquipPosition::LHand):      return EquipSlot::LHand;
			case static_cast<int>(EquipPosition::Neck):       return EquipSlot::Neck;
			case static_cast<int>(EquipPosition::Wrist):      return EquipSlot::Wrist;
			case static_cast<int>(EquipPosition::RFinger):    return EquipSlot::RFinger;
			case static_cast<int>(EquipPosition::Lfinger):    return EquipSlot::Lfinger;
			case static_cast<int>(EquipPosition::RHandS):     return EquipSlot::RHandS;
			case static_cast<int>(EquipPosition::LHandS):     return EquipSlot::LHandS;
			case static_cast<int>(EquipPosition::Vehicle):    return EquipSlot::Vehicle;
			case static_cast<int>(EquipPosition::LEarring):   return EquipSlot::LEarring;
			case static_cast<int>(EquipPosition::LAccessory): return EquipSlot::LAccessory;
			case static_cast<int>(EquipPosition::RAccessory): return EquipSlot::RAccessory;
			case static_cast<int>(EquipPosition::Ornament):   return EquipSlot::Ornament;
			case static_cast<int>(EquipPosition::Waist):      return EquipSlot::Waist;
			case static_cast<int>(EquipPosition::Face):       return EquipSlot::Face;
			case static_cast<int>(EquipPosition::Misc):       return EquipSlot::Misc;
			default:                        return EquipSlot::Invalid;
		}
	}

	bool ItemBaseData::CanEquipInSlot(EquipSlot slot) const
	{
		if (!IsEquipment()) return false;
		EquipSlot validSlot = EquipPosToSlot(equipPos);
		return validSlot == slot;
	}

	// Test provider implementation
	TestItemDataProvider::TestItemDataProvider()
	{
		m_items.reserve(10);

		// Item 1: STR +5, HP +20 ring (accessory)
		{
			ItemBaseData item;
			item.mid = 1; item.sid = 1;
			item.SetNativeID(item.GetNativeID());
			item.name = "Test STR Ring";
			item.itemType = ItemType::Suit;
			item.itemLevel = 1;
			item.equipPos = EquipPosition::RAccessory;
			item.flags = 0x03; // TRADE_ALL
			item.reqLevelMin = 1;
			item.reqLevelMax = 255;
			item.suitType = 21; // SUIT_ACCESSORY (SUIT_RACCESSORY)
			item.addons[0] = ItemAddon(AddonType::StatsStr, 5);
			item.addons[1] = ItemAddon(AddonType::HP, 20);
			m_items.push_back(item);
		}

		// Item 2: SPI +3, MP +15 necklace
		{
			ItemBaseData item;
			item.mid = 1; item.sid = 2;
			item.SetNativeID(item.GetNativeID());
			item.name = "Test SPI Necklace";
			item.itemType = ItemType::Suit;
			item.itemLevel = 1;
			item.equipPos = EquipPosition::Neck;
			item.flags = 0x03;
			item.reqLevelMin = 1;
			item.reqLevelMax = 255;
			item.suitType = 6; // SUIT_NECK
			item.addons[0] = ItemAddon(AddonType::StatsSpi, 3);
			item.addons[1] = ItemAddon(AddonType::MP, 15);
			m_items.push_back(item);
		}

		// Item 3: STA +4, SP +10 gloves
		{
			ItemBaseData item;
			item.mid = 1; item.sid = 3;
			item.SetNativeID(item.GetNativeID());
			item.name = "Test STA Gloves";
			item.itemType = ItemType::Suit;
			item.itemLevel = 1;
			item.equipPos = EquipPosition::Hand;
			item.flags = 0x03;
			item.reqLevelMin = 1;
			item.reqLevelMax = 255;
			item.suitType = 3; // SUIT_HAND
			item.addons[0] = ItemAddon(AddonType::StatsSta, 4);
			item.addons[1] = ItemAddon(AddonType::SP, 10);
			m_items.push_back(item);
		}

// Item 4: HP +6 flat armor (upper) - for testing flat HP bonus
		{
			ItemBaseData item;
			item.mid = 2; item.sid = 1;
			item.SetNativeID(item.GetNativeID());
			item.name = "Test Flat HP Armor";
			item.itemType = ItemType::Suit;
			item.itemLevel = 2;
			item.equipPos = EquipPosition::Upper;
			item.flags = 0x03;
			item.reqLevelMin = 1;
			item.reqLevelMax = 255;
			item.suitType = 1; // SUIT_UPPER
			item.addons[0] = ItemAddon(AddonType::HP, 6); // +6 flat HP
			m_items.push_back(item);
		}

		// Item 5: STR +10, Damage +50 weapon (RHAND)
		{
			ItemBaseData item;
			item.mid = 3; item.sid = 1;
			item.SetNativeID(item.GetNativeID());
			item.name = "Test Sword";
			item.itemType = ItemType::Suit;
			item.itemLevel = 3;
			item.equipPos = EquipPosition::RHand;
			item.flags = 0x03;
			item.reqLevelMin = 20;
			item.reqLevelMax = 255;
			item.suitType = 5; // SUIT_HANDHELD
			item.handFlags = 1; // right hand
			item.handType = 0;  // right hand
			item.damageLow = 50;
			item.damageHigh = 60;
			item.attackType = 1; // ITEMATT_SWORD
			item.addons[0] = ItemAddon(AddonType::StatsStr, 10);
			item.addons[1] = ItemAddon(AddonType::Damage, 50);
			m_items.push_back(item);
		}

		// Item 6: Defense +20, Avoid +5 boots
		{
			ItemBaseData item;
			item.mid = 4; item.sid = 1;
			item.SetNativeID(item.GetNativeID());
			item.name = "Test Boots";
			item.itemType = ItemType::Suit;
			item.itemLevel = 1;
			item.equipPos = EquipPosition::Foot;
			item.flags = 0x03;
			item.reqLevelMin = 1;
			item.reqLevelMax = 255;
			item.suitType = 4; // SUIT_FOOT
			item.defense = 20;
			item.addons[0] = ItemAddon(AddonType::AvoidRate, 5);
			m_items.push_back(item);
		}
	}

	const ItemBaseData* TestItemDataProvider::GetItemData(uint32_t itemId) const
	{
		for (const auto& item : m_items)
		{
			if (item.GetNativeID() == itemId)
				return &item;
		}
		return nullptr;
	}

	const ItemBaseData* TestItemDataProvider::GetItemData(uint16_t mid, uint16_t sid) const
	{
		return GetItemData((static_cast<uint32_t>(mid) << 16) | sid);
	}

	bool TestItemDataProvider::HasItem(uint32_t itemId) const
	{
		return GetItemData(itemId) != nullptr;
	}

	// RAN provider stub
	bool RANItemDataProvider::LoadFromDirectory(const char* dataDir)
	{
		// TODO: Implement loading from portable text exports
		// Expected format: one item per file or single file with blocks
		// For now, return false - real data extraction is a separate task
		(void)dataDir;
		return false;
	}

	const ItemBaseData* RANItemDataProvider::GetItemData(uint32_t itemId) const
	{
		for (const auto& item : m_items)
		{
			if (item.GetNativeID() == itemId)
				return &item;
		}
		return nullptr;
	}

	const ItemBaseData* RANItemDataProvider::GetItemData(uint16_t mid, uint16_t sid) const
	{
		return GetItemData((static_cast<uint32_t>(mid) << 16) | sid);
	}

	bool RANItemDataProvider::HasItem(uint32_t itemId) const
	{
		return GetItemData(itemId) != nullptr;
	}

	// Calculate item contribution from a single equipped item
	static void AddItemBaseContribution(const ItemBaseData* base, ItemContribution& out)
	{
		if (!base) return;

		// Flat addons
		for (size_t ai = 0; ai < 5; ++ai)
		{
			const ItemAddon& addon = base->addons[ai];
			if (addon.type == AddonType::None) continue;
			switch (static_cast<int>(addon.type))
			{
				case static_cast<int>(AddonType::HP):         out.hp += addon.value; break;
				case static_cast<int>(AddonType::MP):         out.mp += addon.value; break;
				case static_cast<int>(AddonType::SP):         out.sp += addon.value; break;
				case static_cast<int>(AddonType::StatsPow):   out.pow += addon.value; break;
				case static_cast<int>(AddonType::StatsStr):   out.str += addon.value; break;
				case static_cast<int>(AddonType::StatsSpi):   out.spi += addon.value; break;
				case static_cast<int>(AddonType::StatsDex):   out.dex += addon.value; break;
				case static_cast<int>(AddonType::StatsInt):   out.intel += addon.value; break;
				case static_cast<int>(AddonType::StatsSta):   out.sta += addon.value; break;
				case static_cast<int>(AddonType::PA):         out.pa += addon.value; break;
				case static_cast<int>(AddonType::SA):         out.sa += addon.value; break;
				case static_cast<int>(AddonType::PowIntDex):  out.pow += addon.value; out.spi += addon.value; out.dex += addon.value; break;
				case static_cast<int>(AddonType::StmVit):     out.str += addon.value; out.sta += addon.value; break;
				case static_cast<int>(AddonType::Pasa):       out.pa += addon.value; out.sa += addon.value; break;
				case static_cast<int>(AddonType::PASAMA):     out.pa += addon.value; out.sa += addon.value; out.ma += addon.value; break;
				case static_cast<int>(AddonType::Damage):     out.damageLow += addon.value; out.damageHigh += addon.value; break;
				case static_cast<int>(AddonType::Defense):    out.defense += addon.value; break;
				case static_cast<int>(AddonType::HitRate):    out.hitRate += addon.value; break;
				case static_cast<int>(AddonType::AvoidRate):  out.avoidRate += addon.value; break;
				default: break;
			}
		}

		// Variates (rate bonuses)
		for (size_t vi = 0; vi < 5; ++vi)
		{
			const ItemVariate& var = base->variates[vi];
			if (var.type == VariateType::None) continue;
			switch (static_cast<int>(var.type))
			{
				case static_cast<int>(VariateType::HP):         out.hpRate += var.value; break;
				case static_cast<int>(VariateType::MP):         out.mpRate += var.value; break;
				case static_cast<int>(VariateType::SP):         out.spRate += var.value; break;
				case static_cast<int>(VariateType::AP):         out.hpRate += var.value; out.mpRate += var.value; out.spRate += var.value; break;
				case static_cast<int>(VariateType::MoveSpeed):  out.moveSpeedRate += var.value; break;
				case static_cast<int>(VariateType::AtkSpeed):   out.atkSpeedRate += var.value; break;
				case static_cast<int>(VariateType::CriticalRate): out.criticalRate += var.value; break;
				case static_cast<int>(VariateType::CrushingBlow): out.crushingBlowRate += var.value; break;
				case static_cast<int>(VariateType::DecDmgMelee): out.decDmgMelee += var.value; break;
				case static_cast<int>(VariateType::DecDmgRange): out.decDmgRange += var.value; break;
				case static_cast<int>(VariateType::DecDmgMagic): out.decDmgMagic += var.value; break;
				default: break;
			}
		}

		// Volume (flat HP/MP/SP/AP)
		if (base->volume.type != VariateType::None)
		{
			switch (static_cast<int>(base->volume.type))
			{
				case static_cast<int>(VariateType::HP):  out.hpVolume += base->volume.value; break;
				case static_cast<int>(VariateType::MP):  out.mpVolume += base->volume.value; break;
				case static_cast<int>(VariateType::SP):  out.spVolume += base->volume.value; break;
				case static_cast<int>(VariateType::AP):  out.hpVolume += base->volume.value; out.mpVolume += base->volume.value; out.spVolume += base->volume.value; break;
				default: break;
			}
		}

		// Random options
		const auto& ro = base->randomOpts;
		if (ro.type1 != RandomOptType::None) { if (ro.type1 == RandomOptType::HP) out.hp += ro.value1; else if (ro.type1 == RandomOptType::MP) out.mp += ro.value1; else if (ro.type1 == RandomOptType::SP) out.sp += ro.value1; }
		if (ro.type2 != RandomOptType::None) { if (ro.type2 == RandomOptType::MP) out.mp += ro.value2; }
		if (ro.type3 != RandomOptType::None) { if (ro.type3 == RandomOptType::SP) out.sp += ro.value3; }
		if (ro.type1 == RandomOptType::HPInc)      out.hpIncRate += ro.value1 * 0.01f;
		if (ro.type1 == RandomOptType::MPInc)      out.mpIncRate += ro.value1 * 0.01f;
		if (ro.type1 == RandomOptType::SPInc)      out.spIncRate += ro.value1 * 0.01f;
		if (ro.type1 == RandomOptType::HMSInc)     { out.hpIncRate += ro.value1 * 0.01f; out.mpIncRate += ro.value1 * 0.01f; out.spIncRate += ro.value1 * 0.01f; }
		if (ro.type1 == RandomOptType::BlowRate)   out.criticalRate += ro.value1 * 0.01f;
		if (ro.type1 == RandomOptType::StrikeRate) out.crushingBlowRate += ro.value1 * 0.01f;
		if (ro.type1 == RandomOptType::MoveSpeed)  out.moveSpeedRate += ro.value1 * 0.01f;

// Grinding grades -> damage/defense/resist
		if (base->grinding.damageGrade > 0)
		{
			out.damageLow += base->grinding.damageGrade * 2;
			out.damageHigh += base->grinding.damageGrade * 2;
		}
		if (base->grinding.defenseGrade > 0)
		{
			out.defense += base->grinding.defenseGrade * 2;
		}
		if (base->grinding.resistFire > 0) out.resistFire += base->grinding.resistFire;
		if (base->grinding.resistIce > 0) out.resistIce += base->grinding.resistIce;
		if (base->grinding.resistElec > 0) out.resistElec += base->grinding.resistElec;
		if (base->grinding.resistPoison > 0) out.resistPoison += base->grinding.resistPoison;
		if (base->grinding.resistSpirit > 0) out.resistSpirit += base->grinding.resistSpirit;

		// Base damage/defense/hit/avoid from SSUIT
		if (base->damageLow > 0 || base->damageHigh > 0)
		{
			out.damageLow += base->damageLow;
			out.damageHigh += base->damageHigh;
		}
		if (base->defense > 0) out.defense += base->defense;
		if (base->hitRate > 0) out.hitRate += base->hitRate;
		if (base->avoidRate > 0) out.avoidRate += base->avoidRate;

		// Resists from SSUIT
		if (base->resistFire > 0) out.resistFire += base->resistFire;
		if (base->resistIce > 0) out.resistIce += base->resistIce;
		if (base->resistElec > 0) out.resistElec += base->resistElec;
		if (base->resistPoison > 0) out.resistPoison += base->resistPoison;
		if (base->resistSpirit > 0) out.resistSpirit += base->resistSpirit;
	}

	static void AddInstanceContribution(const ItemInstanceData* inst, const ItemBaseData* base, ItemContribution& out)
	{
		if (!inst || inst->itemId == 0) return;
		if (!base) return;

		// Instance random options (override base)
		const auto& ro = inst->randomOpts;
		if (ro.type1 != RandomOptType::None) { if (ro.type1 == RandomOptType::HP) out.hp += ro.value1; else if (ro.type1 == RandomOptType::MP) out.mp += ro.value1; else if (ro.type1 == RandomOptType::SP) out.sp += ro.value1; }
		if (ro.type2 != RandomOptType::None) { if (ro.type2 == RandomOptType::MP) out.mp += ro.value2; }
		if (ro.type3 != RandomOptType::None) { if (ro.type3 == RandomOptType::SP) out.sp += ro.value3; }
		if (ro.type1 == RandomOptType::HPInc)      out.hpIncRate += ro.value1 * 0.01f;
		if (ro.type1 == RandomOptType::MPInc)      out.mpIncRate += ro.value1 * 0.01f;
		if (ro.type1 == RandomOptType::SPInc)      out.spIncRate += ro.value1 * 0.01f;
		if (ro.type1 == RandomOptType::HMSInc)     { out.hpIncRate += ro.value1 * 0.01f; out.mpIncRate += ro.value1 * 0.01f; out.spIncRate += ro.value1 * 0.01f; }
		if (ro.type1 == RandomOptType::BlowRate)   out.criticalRate += ro.value1 * 0.01f;
		if (ro.type1 == RandomOptType::StrikeRate) out.crushingBlowRate += ro.value1 * 0.01f;
		if (ro.type1 == RandomOptType::MoveSpeed)  out.moveSpeedRate += ro.value1 * 0.01f;

		// Instance grinding grades
		if (inst->damageGrade > 0)
		{
			out.damageLow += inst->damageGrade * 2;
			out.damageHigh += inst->damageGrade * 2;
		}
		if (inst->defenseGrade > 0) out.defense += inst->defenseGrade * 2;
		if (inst->resistFire > 0) out.resistFire += inst->resistFire;
		if (inst->resistIce > 0) out.resistIce += inst->resistIce;
		if (inst->resistElec > 0) out.resistElec += inst->resistElec;
		if (inst->resistPoison > 0) out.resistPoison += inst->resistPoison;
		if (inst->resistSpirit > 0) out.resistSpirit += inst->resistSpirit;
	}

	ItemContribution CalculateItemContribution(
		const IItemDataProvider* provider,
		const ItemInstanceData* equippedItems,
		size_t slotCount)
	{
		ItemContribution total{};

		if (!provider) return total;

		for (size_t i = 0; i < slotCount; ++i)
		{
			const ItemInstanceData& inst = equippedItems[i];
			if (inst.itemId == 0) continue;

			const ItemBaseData* base = provider->GetItemData(inst.itemId);
			if (!base) continue;

			// Add base item contribution
			AddItemBaseContribution(base, total);

			// Add instance-specific overrides
			AddInstanceContribution(&inst, base, total);
		}

		return total;
	}
}