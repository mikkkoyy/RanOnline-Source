#include "skills/SkillBasicTable.h"

#include <cstddef>
#include <cstdio>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace Modern::Skill
{
	namespace
	{
		// The columns this loader reads, by VERIFIED HEADER NAME.
		//
		// Names rather than indexes throughout: the file repeats its headers,
		// so the layout is proved per block instead of assumed once. Two header
		// names in this export are NOT unique - `sADDSKILL` and `sLINKSKILL`
		// each occupy two columns, because each is a two-WORD `SNATIVEID`
		// written as two same-named columns. Neither is read here. The
		// `Resolve` step refuses a name that is missing OR ambiguous rather
		// than silently taking the first of several.
		struct BasicColumns
		{
			std::size_t idMain, idSub;
			std::size_t name, grade, maxLevel, role, apply;
			std::size_t impactTarget, impactSide, tarRange;
			std::size_t useLeftItem, useRightItem;
			std::size_t brightness, charClass, learnSkillMain, learnSkillSub;

			// sLVL_STEP <n>, for n in 1..9. The nine field names per step.
			std::size_t step[kMaxSkillLevel + 1][9] = {};
		};

		// Field names inside one sLVL_STEP block, in the order the export
		// writes them. Note this is NOT `SLEARN_LVL`'s declaration order:
		// the struct is {dwLEVEL, sSTATS, dwSKP, dwSKILL_LVL}
		// (GLSkillLearn.h:28-40) while the export writes dwSKP first. Resolved
		// by name precisely so the difference cannot become a silent
		// off-by-one.
		const char* const kStepNames[9] = {
			"dwSKP", "dwLEVEL", "sSTATS wPow", "sSTATS wStr", "sSTATS wSpi",
			"sSTATS wDex", "sSTATS wInt", "sSTATS wSta", "dwSKILL_LVL",
		};

		std::size_t LastColumns = 0;

		// Records field START offsets. Reused across rows so a large export
		// does not reallocate once per line.
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

		// One field's text, without its delimiter and without trailing
		// whitespace. The export pads cells with spaces and is CRLF, so both
		// are trimmed here rather than at every parse.
		std::string_view FieldAt(const std::string& line,
		                         const std::vector<std::size_t>& starts,
		                         std::size_t index) noexcept
		{
			if (index >= starts.size())
			{
				return {};
			}

			std::size_t end =
			    (index + 1 < starts.size()) ? starts[index + 1] : line.size();
			if (end > starts[index])
			{
				--end; // drop the trailing comma
			}

			std::string_view text(line.data() + starts[index], end - starts[index]);
			while (!text.empty() &&
			       (text.back() == '\r' || text.back() == ' ' || text.back() == '\t'))
			{
				text.remove_suffix(1);
			}
			return text;
		}

		// Strict decimal parse: the WHOLE field must be consumed, and an empty
		// field is an error rather than a zero. Hand-rolled because
		// `strtol` needs a NUL-terminated buffer and every field is a view
		// into the line.
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
					return false; // would overflow on the next digit
				}
			}

			out = negative ? -value : value;
			return true;
		}

		// Finds a column by name. `ambiguous` is set when the name occurs more
		// than once, because picking the first would be a guess.
		bool FindColumn(const std::string& header,
		                const std::vector<std::size_t>& starts,
		                const char* name,
		                std::size_t& index,
		                bool& ambiguous) noexcept
		{
			ambiguous = false;
			index = 0;
			bool found = false;

			for (std::size_t c = 0; c < starts.size(); ++c)
			{
				if (FieldAt(header, starts, c) == name)
				{
					if (found)
					{
						ambiguous = true;
						return false;
					}
					index = c;
					found = true;
				}
			}
			return found;
		}

		// Resolves every column this loader reads. Refuses a header that does
		// not name them all, or that names one ambiguously - an export whose
		// layout changed must fail loudly, not half-read.
		bool ResolveColumns(const std::string& header,
		                    const std::vector<std::size_t>& starts,
		                    BasicColumns& out) noexcept
		{
			const struct
			{
				const char* name;
				std::size_t BasicColumns::*member;
			} scalars[] = {
				{ "sNATIVEID wMainID",  &BasicColumns::idMain },
				{ "sNATIVEID wSubID",   &BasicColumns::idSub },
				{ "szNAME",             &BasicColumns::name },
				{ "dwGRADE",            &BasicColumns::grade },
				{ "dwMAXLEVEL",         &BasicColumns::maxLevel },
				{ "emROLE",             &BasicColumns::role },
				{ "emAPPLY",            &BasicColumns::apply },
				{ "emIMPACT_TAR",       &BasicColumns::impactTarget },
				{ "emIMPACT_SIDE",      &BasicColumns::impactSide },
				{ "wTARRANGE",          &BasicColumns::tarRange },
				{ "emUSE_LITEM",        &BasicColumns::useLeftItem },
				{ "emUSE_RITEM",        &BasicColumns::useRightItem },
				{ "emBRIGHT",           &BasicColumns::brightness },
				{ "dwCLASS",            &BasicColumns::charClass },
				{ "sSKILL wMainID",     &BasicColumns::learnSkillMain },
				{ "sSKILL wSubID",      &BasicColumns::learnSkillSub },
			};

			for (const auto& scalar : scalars)
			{
				std::size_t index = 0;
				bool ambiguous = false;
				if (!FindColumn(header, starts, scalar.name, index, ambiguous))
				{
					return false;
				}
				out.*(scalar.member) = index;
			}

			for (uint8_t step = 1; step <= kMaxSkillLevel; ++step)
			{
				for (int field = 0; field < 9; ++field)
				{
					// The step block's field names are prefixed by their index,
					// e.g. "sLVL_STEP 1 dwSKP".
					std::string wanted = "sLVL_STEP " + std::to_string(step) +
					                     " " + kStepNames[field];

					std::size_t index = 0;
					bool ambiguous = false;
					if (!FindColumn(header, starts, wanted.c_str(), index, ambiguous))
					{
						return false;
					}
					out.step[step][field] = index;
				}
			}

			LastColumns = starts.size();
			return true;
		}

		// Is this row the verified 322-column definition header?
		//
		// The signature is the column COUNT together with the two identity
		// column NAMES. Count alone would also match any future 322-column
		// table; name alone would match a reordered one. Both together is the
		// evidence that this really is SSKILLBASIC.
		bool IsCanonicalHeader(const std::string& line,
		                       const std::vector<std::size_t>& starts) noexcept
		{
			if (starts.size() != 322)
			{
				return false;
			}
			return FieldAt(line, starts, 0) == "sNATIVEID wMainID" &&
			       FieldAt(line, starts, 1) == "sNATIVEID wSubID";
		}

		// Is this row the verified SAPPLY (719-column) header?
		//
		// Counted so the exclusion is REPORTED rather than silent, and never
		// parsed. SAPPLY starts at `emBASIC_TYPE` / `emELEMENT`
		// (SKILL::SAPPLY, GLSkillApply.h), which is how it is told apart from
		// a 719-column row of anything else.
		bool IsLegacyHeader(const std::string& line,
		                    const std::vector<std::size_t>& starts) noexcept
		{
			if (starts.size() != 719)
			{
				return false;
			}
			return FieldAt(line, starts, 0) == "emBASIC_TYPE" &&
			       FieldAt(line, starts, 1) == "emELEMENT";
		}

		bool ToSkillId(long mainId, long subId, SkillId& out) noexcept
		{
			if (mainId < 0 || mainId > 65535 || subId < 0 || subId > 65535)
			{
				return false;
			}
			// The pair is carried whole. `SkillId` is (wMainID, wSubID) as two
			// uint16_t, so this is lossless and collision-free - no packing,
			// no truncation.
			out = SkillId{ static_cast<uint16_t>(mainId),
			               static_cast<uint16_t>(subId) };
			return out.IsValid();
		}
	}

	std::size_t LastCanonicalHeaderColumns() noexcept
	{
		return LastColumns;
	}

	Result<SkillTableLoadResult> LoadSkillCsv(const std::string& path,
	                                          InMemorySkillDefinitions& provider)
	{
		std::ifstream file(path.c_str(), std::ios::binary);
		if (!file.is_open())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		SkillTableLoadResult result;
		BasicColumns columns{};

		// The widths of the headers seen so far. A data row is classified by
		// MATCHING one of them, not by position - see below.
		std::size_t canonicalWidth = 0;
		std::size_t legacyWidth = 0;

		std::vector<std::size_t> starts;
		std::vector<SkillDefinition> definitions;
		std::vector<SkillId> seen;

		std::string line;
		while (std::getline(file, line))
		{
			SplitCsv(line, starts);

			if (starts.size() == 1 && FieldAt(line, starts, 0).empty())
			{
				++result.blankLines;
				continue;
			}

			// A header makes its SCHEMA available for the rows beneath it.
			//
			// Note what this does NOT do: it does not become a "current
			// schema" for everything that follows. This export writes BOTH
			// headers before EITHER data row -
			//
			//     canonical header, SAPPLY header, canonical row, SAPPLY row
			//
			// - so a most-recent-header rule sends the canonical row to the
			// SAPPLY branch and loses every skill in the file. That is exactly
			// the bug line parity invites and why neither is used here.
			if (IsCanonicalHeader(line, starts))
			{
				++result.canonicalHeaders;
				columns = BasicColumns{};
				if (!ResolveColumns(line, starts, columns))
				{
					// A 322-column header that does not name the columns this
					// loader reads is a changed export. Refuse the file rather
					// than guess positions.
					return Status(ErrorCode::InvalidArgument);
				}
				canonicalWidth = starts.size();
				continue;
			}

			if (IsLegacyHeader(line, starts))
			{
				++result.legacyHeaders;
				legacyWidth = starts.size();
				continue;
			}

			if (canonicalWidth == 0 && legacyWidth == 0)
			{
				// A data row before any header. Refused: nothing establishes
				// its schema.
				++result.rejectedUnknownSchema;
				continue;
			}

			// Classify by the verified header whose width this row matches. The
			// two schemas are 322 and 719 columns and cannot collide, so this
			// is structural evidence rather than a guess about ordering.
			if (legacyWidth != 0 && starts.size() == legacyWidth)
			{
				// SAPPLY / CDATA_LVL: per-level effect data. Excluded BY
				// DESIGN - it is a different legacy struct and a different
				// milestone. Counted, never interpreted.
				++result.excludedLegacyRows;
				continue;
			}

			if (canonicalWidth == 0 || starts.size() != canonicalWidth)
			{
				++result.rejectedFieldCount;
				continue;
			}

			long mainId = 0;
			long subId  = 0;
			if (!ParseLong(FieldAt(line, starts, columns.idMain), mainId) ||
			    !ParseLong(FieldAt(line, starts, columns.idSub), subId))
			{
				++result.rejectedIdentity;
				continue;
			}

			SkillId id;
			if (!ToSkillId(mainId, subId, id))
			{
				++result.rejectedIdentity;
				continue;
			}

			// A repeated id is REPORTED, never applied. Letting the second
			// row overwrite the first would make which row wins depend on
			// file order, which is exactly the kind of silent choice a data
			// recovery must not make.
			if (std::find(seen.begin(), seen.end(), id) != seen.end())
			{
				++result.duplicateIds;
				continue;
			}

			const std::string_view name = FieldAt(line, starts, columns.name);
			if (name.empty())
			{
				++result.rejectedName;
				continue;
			}

			long grade = 0;
			long maxLevel = 0;
			long role = 0;
			long apply = 0;
			long impactTarget = 0;
			long impactSide = 0;
			long tarRange = 0;
			long useLeft = 0;
			long useRight = 0;
			long brightness = 0;
			long charClass = 0;
			long learnMain = 0;
			long learnSub = 0;

			if (!ParseLong(FieldAt(line, starts, columns.grade), grade) ||
			    !ParseLong(FieldAt(line, starts, columns.maxLevel), maxLevel) ||
			    !ParseLong(FieldAt(line, starts, columns.role), role) ||
			    !ParseLong(FieldAt(line, starts, columns.apply), apply) ||
			    !ParseLong(FieldAt(line, starts, columns.impactTarget), impactTarget) ||
			    !ParseLong(FieldAt(line, starts, columns.impactSide), impactSide) ||
			    !ParseLong(FieldAt(line, starts, columns.tarRange), tarRange) ||
			    !ParseLong(FieldAt(line, starts, columns.useLeftItem), useLeft) ||
			    !ParseLong(FieldAt(line, starts, columns.useRightItem), useRight) ||
			    !ParseLong(FieldAt(line, starts, columns.brightness), brightness) ||
			    !ParseLong(FieldAt(line, starts, columns.charClass), charClass) ||
			    !ParseLong(FieldAt(line, starts, columns.learnSkillMain), learnMain) ||
			    !ParseLong(FieldAt(line, starts, columns.learnSkillSub), learnSub))
			{
				++result.rejectedFieldCount;
				continue;
			}

			// The learning table. Parsed with the same strictness: a corrupt
			// cell anywhere in it refuses the row rather than teaching a
			// character to spend the wrong number of skill points.
			std::array<SkillLearnLevel, kMaxSkillLevel + 1> learn{};
			bool learnOk = true;
			for (uint8_t step = 1; step <= kMaxSkillLevel && learnOk; ++step)
			{
				const std::size_t* at = columns.step[step];
				long skp = 0;
				long level = 0;
				long pow = 0;
				long str = 0;
				long spi = 0;
				long dex = 0;
				long intel = 0;
				long sta = 0;
				long skillLevel = 0;

				learnOk =
				    ParseLong(FieldAt(line, starts, at[0]), skp) &&
				    ParseLong(FieldAt(line, starts, at[1]), level) &&
				    ParseLong(FieldAt(line, starts, at[2]), pow) &&
				    ParseLong(FieldAt(line, starts, at[3]), str) &&
				    ParseLong(FieldAt(line, starts, at[4]), spi) &&
				    ParseLong(FieldAt(line, starts, at[5]), dex) &&
				    ParseLong(FieldAt(line, starts, at[6]), intel) &&
				    ParseLong(FieldAt(line, starts, at[7]), sta) &&
				    ParseLong(FieldAt(line, starts, at[8]), skillLevel);

				if (learnOk)
				{
					learn[step].skillPointCost = static_cast<uint32_t>(skp);
					learn[step].requiredLevel = static_cast<uint32_t>(level);
					learn[step].requiredStats.pow = static_cast<uint16_t>(pow);
					learn[step].requiredStats.str = static_cast<uint16_t>(str);
					learn[step].requiredStats.spi = static_cast<uint16_t>(spi);
					learn[step].requiredStats.dex = static_cast<uint16_t>(dex);
					learn[step].requiredStats.intel = static_cast<uint16_t>(intel);
					learn[step].requiredStats.sta = static_cast<uint16_t>(sta);
					learn[step].resultingSkillLevel =
					    static_cast<uint32_t>(skillLevel);
				}
			}
			if (!learnOk)
			{
				++result.rejectedFieldCount;
				continue;
			}

			// Ranges. `dwMAXLEVEL` is checked against the level RAN can
			// actually index (MAX_LEVEL, 9), because the learn table and the
			// level tables are all that size - a value above it would index
			// past the array.
			if (maxLevel < 0 || maxLevel > kMaxSkillLevel ||
			    grade < 0 || tarRange < 0 || tarRange > 65535 ||
			    role < 0 || role > 1 ||
			    apply < 0 || apply > 2 ||
			    impactTarget < 0 || impactTarget > 4 ||
			    impactSide < 0 || impactSide > 2 ||
			    brightness < 0 || brightness > 2 ||
			    useLeft < 0 || useRight < 0)
			{
				++result.rejectedRange;
				continue;
			}

			SkillDefinition definition;
			definition.id = id;
			definition.name = std::string(name);
			definition.grade = static_cast<uint32_t>(grade);
			definition.maxLevel = static_cast<uint8_t>(maxLevel);
			definition.role = static_cast<SkillRole>(role);
			definition.apply = static_cast<SkillApply>(apply);
			definition.targetKind = static_cast<SkillTargetKind>(impactTarget);
			definition.impactSide = static_cast<SkillImpactSide>(impactSide);
			definition.targetRange = static_cast<uint16_t>(tarRange);
			// `GLSKILL_ATT` (GLSkillBasic.h:97-126) is SKILL's own weapon
			// enum, not the items' `GLITEM_ATT`, and the converter here is the
			// one that already maps it.
			definition.leftWeapon =
			    LegacyWeaponTypeToModern(static_cast<int>(useLeft));
			definition.rightWeapon =
			    LegacyWeaponTypeToModern(static_cast<int>(useRight));
			definition.learn = learn;
			definition.learnClassMask = static_cast<uint32_t>(charClass);
			definition.brightness = static_cast<SkillBrightness>(brightness);

			SkillId learnSkill;
			if (ToSkillId(learnMain, learnSub, learnSkill))
			{
				definition.learnSkill = learnSkill;
			}
			// A null `sSKILL` (0xFFFF, 0xFFFF) leaves `learnSkill` at
			// `SkillId::Invalid()`, which is the same "no skill" marker -
			// so no fallback value is invented.

			if (!definition.HasRecoveredBasic())
			{
				++result.rejectedRange;
				continue;
			}

			seen.push_back(id);
			definitions.push_back(definition);
			++result.accepted;
		}

		if (result.canonicalHeaders == 0)
		{
			// A file with no verified definition header is not this export.
			return Status(ErrorCode::InvalidArgument);
		}

		for (const SkillDefinition& definition : definitions)
		{
			// `AddRecoveredBasic` cannot refuse a definition this loader
			// already checked, so a refusal here would be a real defect
			// rather than bad data; it is surfaced through the count instead
			// of being swallowed.
			if (provider.AddRecoveredBasic(definition).IsOk())
			{
				++result.loaded;
			}
			else
			{
				++result.rejectedRange;
			}
		}

		return result;
	}
} // namespace Modern::Skill