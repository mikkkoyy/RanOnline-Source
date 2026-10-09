#include "stats/ClassConstantTable.h"

#include <cmath>

namespace Modern::Stats
{
	const char* ToString(CoefficientSource source) noexcept
	{
		switch (source)
		{
		case CoefficientSource::Unavailable: return "Unavailable";
		case CoefficientSource::Recovered:   return "Recovered";
		}
		return "Unrecognised";
	}

	namespace
	{
		// The reason EVERY row of the shipped table is unavailable. Stated once,
		// here, rather than sixteen times inline, so the wording cannot drift
		// between rows and so changing the reason is a one-line edit.
		//
		// It names the missing artifact and the legacy code that would have
		// supplied it, because a reader needs to know what to go and find.
		constexpr const char* kMissingDataNote =
		    "RAN loads this row from class<N>.classconst via default.charclass "
		    "(GLogicDataLoad.cpp:1131-1214); that data file is not in this "
		    "repository and the GLogicData.cpp:609 constructor values are proven "
		    "overwritten at startup (MovementSpeed.h:8-26), so no coefficient is "
		    "recovered. Keep the caller's named prototype fallback.";

		// Finite means usable. RAN's data loader accepts any parseable number,
		// including one a NaN would produce downstream; `Stats::IsFinite` is the
		// same predicate the calculator itself applies before it will calculate.
		bool AllCoefficientsFinite(const ClassConstants& value) noexcept
		{
			return IsFinite(value);
		}
	}

	bool ValidateRow(ClassConstantRow& row) noexcept
	{
		// An index outside the sixteen legacy values is not a row of this table
		// at all. Checked before the source, because "recovered and invalid" is
		// a worse state to leave a row in than "unavailable".
		if (!IsValidClass(row.index))
		{
			row.source = CoefficientSource::Unavailable;
			row.note   = "refused: index is not one of the sixteen legacy "
			             "EMCHARINDEX values";
			return false;
		}

		if (row.source != CoefficientSource::Recovered)
		{
			// Already unavailable. Left alone, except that a row claiming to be
			// unavailable while carrying a note that says nothing is tidied so
			// the table never presents a blank explanation.
			if (row.note == nullptr || row.note[0] == '\0')
			{
				row.note = kMissingDataNote;
			}
			return false;
		}

		if (row.note == nullptr || row.note[0] == '\0')
		{
			// Provenance that cannot be stated cannot be audited, so a
			// value-bearing row without one is not a recovered row.
			row.source = CoefficientSource::Unavailable;
			row.note   = "refused: a Recovered row must name its source file and row";
			return false;
		}

		if (!AllCoefficientsFinite(row.constants))
		{
			// Demoted, not repaired. See the header: a table that repairs itself
			// cannot be audited.
			row.source = CoefficientSource::Unavailable;
			row.note   = "refused: a coefficient is non-finite (IsFinite)";
			return false;
		}

		return true;
	}

	const ClassConstantTable& ClassConstantTable::Verified() noexcept
	{
		// Built once, never mutated. The rows are data, and the function returns
		// a reference to a function-local constant so there is exactly one
		// instance with no initialisation-order question.
		static const ClassConstantTable table = []
		{
			ClassConstantTable built;

			for (std::size_t position = 0; position < kRowCount; ++position)
			{
				ClassConstantRow& row = built.m_rows[position];

				row.index = static_cast<CharClassIndex>(
					static_cast<uint8_t>(position));
				row.source = CoefficientSource::Unavailable;
				row.note   = kMissingDataNote;

				// `constants` is left at its struct defaults on purpose. Nothing
				// reads them while `source` is Unavailable, and leaving them zero
				// makes an accidental read of an unavailable row produce an
				// obvious zero rather than an RAN-looking number that came from
				// the wrong place.
				row.constants = ClassConstants{};
			}

			return built;
		}();

		return table;
	}

	const ClassConstantRow* ClassConstantTable::Find(CharClassIndex index) const noexcept
	{
		const auto position = static_cast<uint8_t>(index);
		if (position >= kRowCount)
		{
			return nullptr;
		}

		// The row's own index is checked, so a table whose rows were somehow
		// reordered cannot answer for the wrong class.
		const ClassConstantRow& row = m_rows[position];
		return static_cast<uint8_t>(row.index) == position ? &row : nullptr;
	}

	const ClassConstantRow& ClassConstantTable::RowAt(std::size_t position) const noexcept
	{
		return m_rows[position < kRowCount ? position : 0];
	}

	std::size_t ClassConstantTable::RecoveredCount() const noexcept
	{
		std::size_t recovered = 0;
		for (std::size_t position = 0; position < kRowCount; ++position)
		{
			if (m_rows[position].source == CoefficientSource::Recovered)
			{
				++recovered;
			}
		}
		return recovered;
	}

} // namespace Modern::Stats
