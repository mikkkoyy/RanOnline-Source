#include "item/ItemDefinitionTable.h"

#include <cstddef>
#include <fstream>
#include <string_view>
#include <vector>

namespace Modern::Item
{
	namespace
	{
		// The SITEMBASIC and SSUIT column names this loader reads, in the order
		// `SITEM::SaveCsv` writes them (GLItem.cpp:644). Only the ones read here
		// are named; the rest of the 399 are ignored.
		//
		// Every position is resolved FROM THE HEADER rather than trusted, so a
		// re-export that reorders the columns fails loudly instead of silently
		// mis-parsing. `emItemType` is in SITEMBASIC at 18 and the rest are the
		// SSUIT block at 91..101; none of them is hard-coded at the read site.
		struct SuitColumns
		{
			// `emItemType` lives in SITEMBASIC (the block above SSUIT), which is
			// why it is not adjacent to the rest. It is resolved from the header
			// on the same terms as every other column - see `ResolveColumns`.
			std::size_t emItemType = 18;

			std::size_t emSuit    = 91;
			std::size_t dwHand    = 92;
			std::size_t emHand    = 93;
			std::size_t emAttack  = 94;
			std::size_t wAttRange = 95;
			std::size_t wReqSp    = 96;
			std::size_t nHitRate  = 97;
			std::size_t nAvoid = 98;
			std::size_t damageLow  = 99;
			std::size_t damageHigh = 100;
			std::size_t nDefense   = 101;
		};

		std::size_t LastColumns = 0;

		// Splits one CSV line on commas.
		//
		// Used for the HEADER and for every DATA ROW, deliberately. The two used
		// to be split by different code, and that was the whole bug:
		//
		//   * rows went through the hand-rolled `find` pass below, which keeps a
		//     trailing empty field (the export writes a final comma, so a row has
		//     399 fields for 398 named columns), giving 399;
		//   * the header went through `std::getline(stream, field, ',')`, which
		//     does NOT produce a final empty field when the text ends with the
		//     delimiter, giving 398.
		//
		// The per-row width check then called all 18,447 rows malformed. One
		// splitter, used for both, is what makes the counts comparable; the
		// trailing empty field is a real field and is counted on both sides.
		//
		// The hand-rolled pass is also the faster one: the deployed export is
		// 18,447 lines of 399 fields, about 7.4M field reads, which is the
		// difference between about a second and a watchdog timeout.
		//
		// The export quotes string fields that contain a separator, so a plain
		// split is wrong in general. This loader reads only numeric SSUIT
		// columns and the two leading native ids, and it verifies the field
		// COUNT on every line - so a quoted comma inside a NAME shows up as a
		// wrong count and the row is rejected rather than mis-parsed. That is
		// the detection mechanism, not an oversight.
		//
		// The offsets are recorded rather than the fields themselves. The
		// deployed export is 18,447 lines of 399 fields - about 7.4M fields -
		// and materialising each one as a `std::string` cost 7.4M allocations,
		// which is the difference between loading the table and blowing a test's
		// 45 s stall watchdog. A `string_view` into the line costs nothing, and
		// `starts` is reused across rows so its capacity is paid once.
		void SplitCsv(const std::string& line, std::vector<std::size_t>& starts)
		{
			starts.clear();
			std::size_t start = 0;
			for (;;)
			{
				starts.push_back(start);
				const std::size_t comma = line.find(',', start);
				if (comma == std::string::npos)
				{
					return;
				}
				start = comma + 1;
			}
		}

		// The text of one field, without its delimiter.
		std::string_view FieldAt(const std::string& line,
		                         const std::vector<std::size_t>& starts,
		                         std::size_t index) noexcept
		{
			if (index >= starts.size())
			{
				return {};
			}

			std::size_t end = (index + 1 < starts.size()) ? starts[index + 1]
			                                               : line.size();
			if (end > starts[index])
			{
				--end; // drop the trailing comma
			}

			// A CRLF file leaves a CR on the last field, and the export pads
			// some cells with spaces. Both are trailing whitespace to a numeric
			// field, so they are trimmed here rather than per parse.
			std::string_view text(line.data() + starts[index], end - starts[index]);
			while (!text.empty() &&
			       (text.back() == '\r' || text.back() == ' ' || text.back() == '\t'))
			{
				text.remove_suffix(1);
			}
			return text;
		}

		bool ResolveColumns(const std::string& header, SuitColumns& out) noexcept
		{
			std::vector<std::size_t> starts;
			SplitCsv(header, starts);

			const std::size_t columnCount = starts.size();

			// A file that is not the RAN item export at all must be refused
			// rather than read at guessed positions.
			if (columnCount < 107)
			{
				return false;
			}

			const char* kNames[] = {
				"emItemType",
				"emSuit", "dwHAND", "emHand", "emAttack", "wAttRange", "wReqSP",
				"nHitRate", "nAvoidRate", "gdDamage wLow", "gdDamage wHigh",
				"nDefense",
			};
			std::size_t* slots[] = {
				&out.emItemType,
				&out.emSuit, &out.dwHand, &out.emHand, &out.emAttack,
				&out.wAttRange, &out.wReqSp, &out.nHitRate, &out.nAvoid,
				&out.damageLow, &out.damageHigh, &out.nDefense,
			};

			for (std::size_t i = 0; i < sizeof(kNames) / sizeof(kNames[0]); ++i)
			{
				bool found = false;
				for (std::size_t c = 0; c < columnCount; ++c)
				{
					if (FieldAt(header, starts, c) == kNames[i])
					{
						*slots[i] = c;
						found = true;
						break;
					}
				}
				if (!found)
				{
					return false;
				}
			}

			LastColumns = columnCount;
			return true;
		}

		// Strict decimal parse of one field: the WHOLE field must be consumed, and
		// an empty field is an error rather than a zero.
		//
		// Hand-rolled rather than `strtol`, because `strtol` needs a
		// NUL-terminated buffer and every field is a `string_view` into the
		// line - copying each one just to parse it would put back the
		// allocations the span pass exists to avoid.
		//
		// The accepted grammar is the same one the export writes, so validation
		// is not loosened: optional sign, then one or more digits, then
		// nothing. Trailing junk and embedded junk are both refused, which is
		// what kept a corrupt cell from being read as zero.
		bool ParseLong(std::string_view text, long& out) noexcept
		{
			if (text.empty())
			{
				return false;
			}

			std::size_t i = 0;
			bool negative = false;
			if (text[0] == '-' || text[0] == '+')
			{
				negative = (text[0] == '-');
				i = 1;
			}
			if (i >= text.size())
			{
				return false;
			}

			long value = 0;
			for (; i < text.size(); ++i)
			{
				const char digit = text[i];
				if (digit < '0' || digit > '9')
				{
					return false;
				}

				value = value * 10 + (digit - '0');
				if (value > 2147483647L)
				{
					return false; // would overflow `long` on the next digit
				}
			}

			out = negative ? -value : value;
			return true;
		}

		LegacyItemType ToLegacyItemType(long value) noexcept
		{
			switch (value)
			{
			case 0:  return LegacyItemType::Suit;
			case 1:  return LegacyItemType::Arrow;
			case 7:  return LegacyItemType::Charm;
			case 34: return LegacyItemType::Revive;
			case 43: return LegacyItemType::AntiDisappear;
			case 45: return LegacyItemType::Vehicle;
			case 57: return LegacyItemType::Bullet;
			default: return LegacyItemType::Other;
			}
		}

		LegacySuit ToLegacySuit(long value) noexcept
		{
			switch (value)
			{
			case 0:  return LegacySuit::Headgear;
			case 1:  return LegacySuit::Upper;
			case 2:  return LegacySuit::Lower;
			case 3:  return LegacySuit::Hand;
			case 4:  return LegacySuit::Foot;
			case 5:  return LegacySuit::Handheld;
			case 6:  return LegacySuit::Neck;
			case 7:  return LegacySuit::Wrist;
			case 8:  return LegacySuit::Finger;
			case 9:  return LegacySuit::PetA;
			case 10: return LegacySuit::PetB;
			case 11: return LegacySuit::Vehicle;
			case 19: return LegacySuit::Belt;
			case 20: return LegacySuit::Earring;
			case 21: return LegacySuit::Accessory;
			case 22: return LegacySuit::Ornament;
			case 23: return LegacySuit::Face;
			case 24: return LegacySuit::Misc;
			default: return LegacySuit::None;
			}
		}

		LegacyItemAtt ToLegacyItemAtt(long value) noexcept
		{
			if (value < 0 || value > 25)
			{
				return LegacyItemAtt::Nothing;
			}
			return static_cast<LegacyItemAtt>(static_cast<uint8_t>(value));
		}

		LegacyHand ToLegacyHand(long value) noexcept
		{
			switch (value)
			{
			case 2: return LegacyHand::Left;
			case 3: return LegacyHand::Both;
			default: return LegacyHand::Right;
			}
		}

		// The one place an item's modern `ItemKind` is decided.
		//
		// `CHECKSLOT_ITEM` only ever accepts the seven `LegacyItemType` values
		// named above, and of those only `Suit` carries a real stat block - an
		// arrow, a charm or a bullet contributes nothing until it is consumed, and
		// a vehicle is not a combat item at all. So the split is by suit, and a
		// `Suit` that is also `Handheld` is a weapon, which is the only kind that
		// can set a range or an attack type.
		ItemKind DecideKind(LegacyItemType type, LegacySuit suit) noexcept
		{
			if (type != LegacyItemType::Suit)
			{
				// Still equipment-the-item-can-be-worn, so a charm that somehow
				// carries a stat block still aggregates; see ItemKind::Accessory.
				return (type == LegacyItemType::Arrow ||
				        type == LegacyItemType::Charm ||
				        type == LegacyItemType::Bullet)
				           ? ItemKind::Accessory
				           : ItemKind::Misc;
			}

			switch (suit)
			{
			case LegacySuit::Handheld:
				return ItemKind::Weapon;
			case LegacySuit::Upper:
			case LegacySuit::Lower:
			case LegacySuit::Headgear:
			case LegacySuit::Hand:
			case LegacySuit::Foot:
				return ItemKind::Armor;
			default:
				return ItemKind::Accessory;
			}
		}
	}

	std::size_t LastHeaderColumns() noexcept
	{
		return LastColumns;
	}

	Result<ItemTableLoadResult> LoadItemCsv(const std::string& path,
	                                        InMemoryItemDefinitions& provider)
	{
		std::ifstream file(path.c_str(), std::ios::binary);
		if (!file.is_open())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		std::string header;
		if (!std::getline(file, header))
		{
			return Status(ErrorCode::InvalidArgument);
		}

		SuitColumns columns;
		if (!ResolveColumns(header, columns))
		{
			return Status(ErrorCode::InvalidArgument);
		}
		const std::size_t columnCount = LastColumns;

		ItemTableLoadResult result;
	// Collected in file order and handed to the provider in one call; see the
	// comment at the `AddAll` site.
	std::vector<ItemDefinition> definitions;
		definitions.reserve(18447);

	// Reused across every row, so the export does not reallocate once per
	// line. This is the only per-row buffer the loader has.
	std::vector<std::size_t> starts;

		std::string line;
		while (std::getline(file, line))
		{
			if (!line.empty() && line.back() == '\r')
			{
				line.pop_back();
			}
			if (line.empty())
			{
				continue;
			}

			SplitCsv(line, starts);
			if (starts.size() != columnCount)
			{
				++result.rejectedFieldCount;
				continue;
			}

			long mainId = 0;
			long subId  = 0;
			if (!ParseLong(FieldAt(line, starts, 0), mainId) ||
			    !ParseLong(FieldAt(line, starts, 1), subId))
			{
				++result.rejectedIdentity;
				continue;
			}
			if (mainId < 0 || mainId > 65535 || subId < 0 || subId > 65535)
			{
				++result.rejectedIdentity;
				continue;
			}
			// (0xFFFF,0xFFFF) packs into the invalid sentinel, so it cannot name
			// an item. The deployed table's maximum is (1996,906), so this is a
			// guard and not a filter that rejects anything real.
			if (!CanPackItemId(static_cast<uint16_t>(mainId),
			                   static_cast<uint16_t>(subId)))
			{
				++result.rejectedIdentity;
				continue;
			}

			// The name is column 4, and `SITEM::IsValid()` requires one
			// (`InMemoryItemDefinitions::Add` requires whole-definition validity).
			// The export writes an empty field for a missing name, and a row
			// without one is not a usable definition.
			const std::string_view name = FieldAt(line, starts, 4);
			if (name.empty())
			{
				++result.rejectedName;
				continue;
			}

			ItemDefinition definition;
			definition.id   = PackItemId(static_cast<uint16_t>(mainId),
			                             static_cast<uint16_t>(subId));
			definition.name = std::string(name);

			long itemTypeValue = 0;
			long suitValue     = 0;
			long attackValue   = 0;
			long handValue     = 0;
			long handFlags     = 0;
			long attackRange   = 0;
			long requiredSp    = 0;
			long hitRate       = 0;
			long avoidRate     = 0;
			long damageLow     = 0;
			long damageHigh    = 0;
			long defense       = 0;

			const bool parsed =
			    ParseLong(FieldAt(line, starts, columns.emSuit), suitValue) &&
			    ParseLong(FieldAt(line, starts, columns.dwHand), handFlags) &&
			    ParseLong(FieldAt(line, starts, columns.emHand), handValue) &&
			    ParseLong(FieldAt(line, starts, columns.emAttack), attackValue) &&
			    ParseLong(FieldAt(line, starts, columns.wAttRange), attackRange) &&
			    ParseLong(FieldAt(line, starts, columns.wReqSp), requiredSp) &&
			    ParseLong(FieldAt(line, starts, columns.nHitRate), hitRate) &&
			    ParseLong(FieldAt(line, starts, columns.nAvoid), avoidRate) &&
			    ParseLong(FieldAt(line, starts, columns.damageLow), damageLow) &&
			    ParseLong(FieldAt(line, starts, columns.damageHigh), damageHigh) &&
			    ParseLong(FieldAt(line, starts, columns.nDefense), defense) &&
			    ParseLong(FieldAt(line, starts, columns.emItemType), itemTypeValue);

			// A non-numeric cell in a column this loader reads is a corrupt
			// export. The row is refused rather than read as zero, because a
			// zero damage range on a real weapon would be indistinguishable from
			// a weapon that deals no damage.
			if (!parsed)
			{
				++result.rejectedFieldCount;
				continue;
			}

			// A negative damage, hit, defence or range is not a value RAN's
			// writer can emit for these columns; `nAvoidRate` IS allowed to be
			// negative, and it is in the deployed data (row 1 of the export
			// carries -5), so it is the one signed column here and it is not
			// clamped.
			if (attackRange < 0 || attackRange > 65535 ||
			    damageLow < 0 || damageHigh > 2147483647L || defense < -2147483647L - 1 ||
			    requiredSp < 0 || requiredSp > 65535 || hitRate < -2147483647L - 1)
			{
				++result.rejectedRange;
				continue;
			}

			definition.itemType   = ToLegacyItemType(itemTypeValue);
			definition.suit       = ToLegacySuit(suitValue);
			definition.attack     = ToLegacyItemAtt(attackValue);
			definition.hand       = ToLegacyHand(handValue);
			definition.handFlags  = static_cast<uint16_t>(handFlags);
			definition.attackRange = static_cast<uint16_t>(attackRange);
			definition.kind       = DecideKind(definition.itemType, definition.suit);

			// The stat block, from the same SSUIT columns. `gdDamage` is a
			// low/high pair and `nDefense` is flat; the hit and avoid rates are
			// the ITEM's own contribution, which is what `SUM_ITEM` aggregates
			// (GLogixExPC.cpp:463-467, :656-660).
			definition.stats.damageLow  = static_cast<int32_t>(damageLow);
			definition.stats.damageHigh = static_cast<int32_t>(damageHigh);
			definition.stats.defense    = static_cast<int32_t>(defense);
			definition.stats.hit        = static_cast<int32_t>(hitRate);
			definition.stats.avoid      = static_cast<int32_t>(avoidRate);
			definition.stats.requiredSP = static_cast<uint16_t>(requiredSp);

			// A definition the provider would refuse is not loaded at all - a
			// partial definition silently refusing later is worse than a counted
			// rejection here.
			if (!definition.IsValid())
			{
				++result.rejectedName;
				continue;
			}

			definitions.push_back(definition);
			++result.loaded;
		}

		// One bulk insert. `InMemoryItemDefinitions::Add` keeps the container
		// sorted, so registering 18,447 rows one at a time is quadratic and
		// exceeds a test's per-case budget. `AddAll` sorts once.
		//
		// A row that fails validation HERE is refused rather than counted twice
		// above: the loader has already checked `IsValid`, and a non-finite stat
		// block is the only other reason the provider refuses one.
		const auto bulk = provider.AddAll(definitions);
		result.rejectedRange += bulk.refused;

		return result;
	}
} // namespace Modern::Item
