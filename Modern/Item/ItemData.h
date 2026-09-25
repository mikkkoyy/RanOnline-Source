#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Modern
{
	// Equipment slot indices matching RAN's EMSLOT (GLItemDef.h)
	enum class EquipSlot : uint8_t
	{
		Headgear    = 0,  // SLOT_HEADGEAR
		Upper       = 1,  // SLOT_UPPER
		Lower       = 2,  // SLOT_LOWER
		Hand        = 3,  // SLOT_HAND
		Foot        = 4,  // SLOT_FOOT
		RHand       = 5,  // SLOT_RHAND
		LHand       = 6,  // SLOT_LHAND
		Neck        = 7,  // SLOT_NECK
		Wrist       = 8,  // SLOT_WRIST
		RFinger     = 9,  // SLOT_RFINGER
		Lfinger     = 10, // SLOT_LFINGER
		RHandS      = 11, // SLOT_RHAND_S (extreme)
		LHandS      = 12, // SLOT_LHAND_S (extreme)
		Vehicle     = 13, // SLOT_VEHICLE
		LEarring    = 14, // SLOT_LEARRING
		LAccessory  = 15, // SLOT_LACCESSORY
		RAccessory  = 16, // SLOT_RACCESSORY
		Ornament    = 17, // SLOT_ORNAMENT
		Waist       = 18, // SLOT_WAIST
		Face        = 19, // SLOT_FACE
		Misc        = 20, // SLOT_MISC

		Count       = 21, // SLOT_TSIZE = 22 (including SLOT_HOLD)
		Invalid     = 0xFF
	};

	// Item type matching RAN's EMITEM_TYPE
	enum class ItemType : uint8_t
	{
		Suit        = 0,  // ITEM_SUIT (equipment)
		Arrow       = 1,
		Cure        = 2,
		Skill       = 3,
		Recall      = 4,
		Key         = 5,
		Grinding    = 6,
		Charm       = 7,
		Ticket      = 8,
		SkillReset  = 9,
		StatReset   = 10,
		SkillStatReset = 11,
		Box         = 12,
		Cleanser    = 13,
		Loudspeaker = 14,
		Firecracker = 15,
		// ... other types omitted for brevity
		SuitDisguise = 23, // ITEM_DISJUNCTION (costume)
	};

	// EMADDON types from legacy (EMADD_*)
	enum class AddonType : uint8_t
	{
		None          = 0,
		HitRate       = 1,   // EMADD_HITRATE
		AvoidRate     = 2,   // EMADD_AVOIDRATE
		Damage        = 3,   // EMADD_DAMAGE
		Defense       = 4,   // EMADD_DEFENSE
		HP            = 5,   // EMADD_HP
		MP            = 6,   // EMADD_MP
		SP            = 7,   // EMADD_SP
		StatsPow      = 8,   // EMADD_STATS_POW
		StatsStr      = 9,   // EMADD_STATS_STR
		StatsSpi      = 10,  // EMADD_STATS_SPI
		StatsDex      = 11,  // EMADD_STATS_DEX
		StatsInt      = 12,  // EMADD_STATS_INT
		StatsSta      = 13,  // EMADD_STATS_STA
		PA            = 14,  // EMADD_PA
		SA            = 15,  // EMADD_SA
		PowIntDex     = 16,  // EMADD_POWINTDEX
		StmVit        = 17,  // EMADD_STMVIT
		Pasa          = 18,  // EMADD_PASA
		PASAMA        = 19,  // EMADD_PASAMA
		MA            = 20,  // EMADD_MA (handled via GETMaDAMAGE)
		Count
	};

	// EMVARIATE types (EMVAR_*)
	enum class VariateType : uint8_t
	{
		None          = 0,
		HP            = 1,   // EMVAR_HP
		MP            = 2,   // EMVAR_MP
		SP            = 3,   // EMVAR_SP
		AP            = 4,   // EMVAR_AP (all three)
		MoveSpeed     = 5,   // EMVAR_MOVE_SPEED
		AtkSpeed      = 6,   // EMVAR_ATTACK_SPEED
		CriticalRate  = 7,   // EMVAR_CRITICAL_RATE
		CrushingBlow  = 8,   // EMVAR_CRUSHING_BLOW
		DecDmgMelee   = 9,   // EMVAR_DECR_DAMAGE_MELEESKILL
		DecDmgRange   = 10,  // EMVAR_DECR_DAMAGE_RANGESKILL
		DecDmgMagic   = 11,  // EMVAR_DECR_DAMAGE_MAGICSKILL
		Count
	};

	// Random option types (EMRANDOM_OPT)
	enum class RandomOptType : uint8_t
	{
		None        = 0,
		HP          = 1,   // EMR_OPT_HP
		MP          = 2,   // EMR_OPT_MP
		SP          = 3,   // EMR_OPT_SP
		HPInc       = 4,   // EMR_OPT_HP_INC
		MPInc       = 5,   // EMR_OPT_MP_INC
		SPInc       = 6,   // EMR_OPT_SP_INC
		HMSInc      = 7,   // EMR_OPT_HMS_INC
		BlowRate    = 8,   // EMR_OPT_BLOW_RATE
		StrikeRate  = 8,   // EMR_OPT_STRIKE_RATE
		MoveSpeed   = 9,   // EMR_OPT_MOVE_SPEED
		Count
	};

	// Equipment slot requirement (which slots an item can be equipped in)
	enum class EquipPosition : uint8_t
	{
		None        = 0,
		Headgear    = 1,
		Upper       = 2,
		Lower       = 3,
		Hand        = 4,  // gloves (SUIT_HAND)
		Foot        = 5,
		RHand       = 6,
		LHand       = 7,
		Neck        = 8,
		Wrist       = 9,
		RFinger     = 10,
		Lfinger     = 11,
		RHandS      = 12, // extreme
		LHandS      = 13, // extreme
		Vehicle     = 14,
		LEarring    = 15,
		LAccessory  = 16,
		RAccessory  = 17,
		Ornament    = 18,
		Waist       = 19,
		Face        = 20,
		Misc        = 21,
		Count       = 22
	};

	// Item addon (flat bonus) - corresponds to SADDON
	struct ItemAddon
	{
		AddonType type = AddonType::None;
		int32_t   value = 0;  // nVALUE from SADDON

		ItemAddon() = default;
		ItemAddon(AddonType t, int32_t v) : type(t), value(v) {}
	};

	// Item variate (rate/percentage bonus) - corresponds to SVARIATE
	struct ItemVariate
	{
		VariateType type = VariateType::None;
		float       value = 0.0f;  // fVariate from SVARIATE

		ItemVariate() = default;
		ItemVariate(VariateType t, float v) : type(t), value(v) {}
	};

	// Item volume (flat amount) - corresponds to SVOLUME
	struct ItemVolume
	{
		VariateType type = VariateType::None;  // reuses same enum for HP/MP/SP/AP
		float       value = 0.0f;  // fVolume from SVOLUME

		ItemVolume() = default;
		ItemVolume(VariateType t, float v) : type(t), value(v) {}
	};

	// Random option - corresponds to SRANDOM_OPT
	struct ItemRandomOption
	{
		RandomOptType type1 = RandomOptType::None;
		RandomOptType type2 = RandomOptType::None;
		RandomOptType type3 = RandomOptType::None;
		RandomOptType type4 = RandomOptType::None;

		int16_t value1 = 0;  // nOptVALUE1
		int16_t value2 = 0;  // nOptVALUE2
		int16_t value3 = 0;  // nOptVALUE3
		int16_t value4 = 0;  // nOptVALUE4

		ItemRandomOption() = default;
	};

	// Grinding/enchantment grades
	struct ItemGrinding
	{
		uint8_t damageGrade = 0;        // cDAMAGE
		uint8_t defenseGrade = 0;       // cDEFENSE
		uint8_t resistFire = 0;         // cRESIST_FIRE
		uint8_t resistIce = 0;          // cRESIST_ICE
		uint8_t resistElec = 0;         // cRESIST_ELEC
		uint8_t resistPoison = 0;       // cRESIST_POISON
		uint8_t resistSpirit = 0;       // cRESIST_SPIRIT

		ItemGrinding() = default;
	};

	// Static item definition data (from SITEM/SITEMBASIC/SSUIT)
	// Only fields verified to affect character stat calculation
	struct ItemBaseData
	{
		uint32_t itemId = 0;          // SNATIVEID (MID<<16 | SID)
		uint16_t mid = 0;             // main ID
		uint16_t sid = 0;             // sub ID

		// Basic properties (from SITEMBASIC)
		std::string name;
		ItemType itemType = ItemType::Suit;
		uint8_t itemLevel = 0;        // EMITEMLEVEL
		uint32_t reqCharClass = 0;    // dwReqCharClass (bitflags)
		uint32_t reqSchool = 0;       // dwReqSchool (bitflags)
		uint16_t reqLevelMin = 0;     // wReqLevelDW
		uint16_t reqLevelMax = 0;     // wReqLevelUP
		uint16_t reqPA = 0;           // wReqPA
		uint16_t reqSA = 0;           // wReqSA
		EquipPosition equipPos = EquipPosition::None;  // derived from emSuit/emItemType
		uint16_t invenSizeX = 1;
		uint16_t invenSizeY = 1;
		uint32_t flags = 0;           // dwFlags (trade, dispose, etc.)

		// Equipment stats (from SSUIT) - only for ITEM_SUIT type
		uint8_t suitType = 0;         // emSuit (EMSUIT)
		uint32_t handFlags = 0;       // dwHAND
		uint8_t handType = 0;         // emHand (EMITEM_HAND)
		uint32_t damageLow = 0;       // gdDamage.dwLow
		uint32_t damageHigh = 0;      // gdDamage.dwMax
		int16_t defense = 0;          // nDefense
		int16_t hitRate = 0;          // nHitRate
		int16_t avoidRate = 0;        // nAvoidRate
		uint16_t resistFire = 0;      // sResist.nFire
		uint16_t resistIce = 0;       // sResist.nIce
		uint16_t resistElec = 0;      // sResist.nElec
		uint16_t resistPoison = 0;    // sResist.nPoison
		uint16_t resistSpirit = 0;    // sResist.nSpirit
		uint8_t attackType = 0;       // emAttack (GLITEM_ATT)
		uint16_t attackRange = 0;     // wAttRange
		uint16_t reqSP = 0;           // wReqSP

		// Addons (flat bonuses) - up to 5 from sADDON
		ItemAddon addons[5] = {};

		// Variates (rate/percentage bonuses) - up to 5 from sVARIATE
		ItemVariate variates[5] = {};

		// Volume (flat HP/MP/SP/AP) - from sVOLUME
		ItemVolume volume = {};

		// Random options (up to 4) - from sRandomOpt
		ItemRandomOption randomOpts = {};

		// Grinding/enchantment
		ItemGrinding grinding = {};

		// Costume/disguise combine (nidDISGUISE) - for costume stats
		uint32_t disguiseItemId = 0;  // nidDISGUISE

		// Requirements from SITEMBASIC
		uint16_t reqLevelMinDW = 0;   // wReqLevelDW
		uint16_t reqLevelMaxUP = 0;   // wReqLevelUP
		uint16_t reqPAStat = 0;       // wReqPA
		uint16_t reqSAStat = 0;       // wReqSA
		uint8_t reqBright = 0;        // emReqBright

		// Helper methods
		bool IsEquipment() const { return itemType == ItemType::Suit; }
		bool CanEquipInSlot(EquipSlot slot) const;
		uint32_t GetNativeID() const { return (static_cast<uint32_t>(mid) << 16) | sid; }
		void SetNativeID(uint32_t id) { mid = static_cast<uint16_t>(id >> 16); sid = static_cast<uint16_t>(id & 0xFFFF); }
	};

	// Equipped item instance (runtime state from SITEMCUSTOM)
	// Only fields that affect character stat calculation
	struct ItemInstanceData
	{
		uint32_t itemId = 0;              // base item ID (SNATIVEID)
		uint32_t disguiseItemId = 0;      // nidDISGUISE (costume)

		// Grinding/enchantment state
		uint8_t damageGrade = 0;          // cDAMAGE
		uint8_t defenseGrade = 0;         // cDEFENSE
		uint8_t resistFire = 0;           // cRESIST_FIRE
		uint8_t resistIce = 0;            // cRESIST_ICE
		uint8_t resistElec = 0;           // cRESIST_ELEC
		uint8_t resistPoison = 0;         // cRESIST_POISON
		uint8_t resistSpirit = 0;         // cRESIST_SPIRIT

		// Random options (up to 4)
		ItemRandomOption randomOpts = {};

		// Costume combine
		bool isCostume = false;

		// Usage/turns
		uint16_t turnNum = 0;             // wTurnNum

		// Generation info
		uint8_t genType = 0;              // cGenType
		uint8_t fieldId = 0;              // cFieldID
		uint64_t genNum = 0;              // lnGenNum

		ItemInstanceData() = default;
		explicit ItemInstanceData(uint32_t id) : itemId(id) {}
	};

	// Aggregated item contributions for character calculation
	// Matches legacy SSUM_ITEM structure
	struct ItemContribution
	{
		// Flat HP/MP/SP
		int32_t hp = 0;
		int32_t mp = 0;
		int32_t sp = 0;

		// Rate % (multiplier applied after base calc)
		float hpRate = 0.0f;
		float mpRate = 0.0f;
		float spRate = 0.0f;

		// Volume (flat added after rate)
		float hpVolume = 0.0f;
		float mpVolume = 0.0f;
		float spVolume = 0.0f;

		// Stats
		int32_t pow = 0;
		int32_t str = 0;
		int32_t spi = 0;
		int32_t dex = 0;
		int32_t intel = 0;
		int32_t sta = 0;

		// Combat
		uint32_t damageLow = 0;
		uint32_t damageHigh = 0;
		int32_t defense = 0;
		int32_t hitRate = 0;
		int32_t avoidRate = 0;

		// PA/SA/MA
		int32_t pa = 0;
		int32_t sa = 0;
		int32_t ma = 0;

		// Resists
		int32_t resistFire = 0;
		int32_t resistIce = 0;
		int32_t resistElec = 0;
		int32_t resistPoison = 0;
		int32_t resistSpirit = 0;

		// Rate multipliers (percentage)
		float hpIncRate = 0.0f;
		float mpIncRate = 0.0f;
		float spIncRate = 0.0f;

		// Potion bonuses
		int32_t hpPotionRate = 0;
		int32_t mpPotionRate = 0;
		int32_t spPotionRate = 0;

		// Move/atk speed
		float moveSpeedRate = 0.0f;
		float atkSpeedRate = 0.0f;

		// Crit/crush
		float criticalRate = 0.0f;
		float crushingBlowRate = 0.0f;

		// Damage reduction rates
		float decDmgMelee = 0.0f;
		float decDmgRange = 0.0f;
		float decDmgMagic = 0.0f;

		// Hit/avoid rate multipliers
		float hitRatePer = 0.0f;
		float avoidRatePer = 0.0f;

		void Reset() { *this = {}; }

		// Add another contribution
		ItemContribution& operator+=(const ItemContribution& other);
	};

	// Provider interface
	class IItemDataProvider
	{
	public:
		virtual ~IItemDataProvider() = default;

		virtual const ItemBaseData* GetItemData(uint32_t itemId) const = 0;
		virtual const ItemBaseData* GetItemData(uint16_t mid, uint16_t sid) const = 0;
		virtual bool HasItem(uint32_t itemId) const = 0;
	};

	// Test provider with deterministic items
	class TestItemDataProvider : public IItemDataProvider
	{
	public:
		TestItemDataProvider();

		const ItemBaseData* GetItemData(uint32_t itemId) const override;
		const ItemBaseData* GetItemData(uint16_t mid, uint16_t sid) const override;
		bool HasItem(uint32_t itemId) const override;

	private:
		std::vector<ItemBaseData> m_items;
	};

	// RAN provider (loads from portable text exports)
	class RANItemDataProvider : public IItemDataProvider
	{
	public:
		RANItemDataProvider() = default;

		// Load from portable text export (one item per block, generated from legacy loader)
		bool LoadFromDirectory(const char* dataDir);

		const ItemBaseData* GetItemData(uint32_t itemId) const override;
		const ItemBaseData* GetItemData(uint16_t mid, uint16_t sid) const override;
		bool HasItem(uint32_t itemId) const override;

	private:
		std::vector<ItemBaseData> m_items;
	};

	// Calculate total item contribution from equipped items
	ItemContribution CalculateItemContribution(
		const IItemDataProvider* provider,
		const ItemInstanceData* equippedItems,
		size_t slotCount);
}