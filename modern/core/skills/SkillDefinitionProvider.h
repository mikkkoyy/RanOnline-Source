#pragma once

// VERTICAL-003: resolving a skill ID to its definition.
//
// Deliberately not a database. RAN loads skill data from packed binary
// skill tables through GLSkillMan; parsing that is out of scope. What is
// needed here is the *shape* of the lookup, so a future database or resource
// provider can satisfy the same interface without anything above it changing.

#include "skills/SkillDefinition.h"
#include "types/Result.h"

#include <algorithm>
#include <vector>

namespace Modern
{
	// Read-only access to skill definitions.
	//
	// Returning a pointer rather than a copy keeps a definition shared, which
	// is what a definition is; the caller must not modify it.
	class SkillDefinitionProvider
	{
	public:
		virtual ~SkillDefinitionProvider() = default;

		// The definition for an id, or nullptr when this provider does not know
		// it. A missing definition is not an error the caller has to handle
		// differently from an unknown id, so the lookup reports absence.
		virtual const SkillDefinition* Find(const SkillId& id) const = 0;
	};

	// A provider over a fixed set of definitions.
	//
	// This is the implementation for a milestone with no database: the tests and
	// any headless tool register the definitions they need and the aggregation
	// behaves exactly as it would against a real provider. It is a real type
	// rather than a test fixture, so a future server can hold one without the
	// interface changing.
	class InMemorySkillDefinitions final : public SkillDefinitionProvider
	{
	public:
		// Adds or replaces a definition. A definition that is not valid is
		// refused and nothing changes.
		//
		// Adding is not reserved: the container grows, so a caller cannot
		// invalidate an existing definition by adding another.
		Status Add(const SkillDefinition& definition);

		// SKILL-001: admits a definition recovered from the skill export's
		// SSKILLBASIC + SLEARN half ALONE.
		//
		// Add requires IsValid(), which additionally asks that some level
		// contributes something - a question only the SAPPLY half can answer.
		// That half is NOT recovered yet, so every recovered-basic definition
		// would be refused. Rather than weaken IsValid() for every caller, this
		// is a second, explicitly named admission path with the narrower rule
		// HasRecoveredBasic().
		//
		// The consequence is deliberate and must not be forgotten: a definition
		// admitted this way has ZERO levelData, impacts and specs. It is
		// learnable data, not a castable one. GetLevelData already returns a
		// zeroed record rather than another skill's, so a caller that reaches for
		// effects finds nothing rather than something wrong.
		Status AddRecoveredBasic(const SkillDefinition& definition);

		// Removes a definition, which is how a test empties a provider without
		// rebuilding it. Returns NotFound when the id was not registered.
		Status Remove(const SkillId& id);

		const SkillDefinition* Find(const SkillId& id) const override;

		size_t GetCount() const noexcept { return m_definitions.size(); }
		void   Clear() noexcept { m_definitions.clear(); }

	private:
		// Kept sorted by id so Find is a binary search and registration order
		// cannot change what a lookup returns.
		std::vector<SkillDefinition> m_definitions;
	};
}