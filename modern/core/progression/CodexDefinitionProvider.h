#pragma once

// VERTICAL-004: resolving a codex id to its definition.
//
// Same shape and same reason as ItemDefinitionProvider and
// SkillDefinitionProvider: RAN loads codex entries from a packed, encrypted
// GLogic resource through `GLCodex` (GLCodex.cpp:97-147) or from a flat INI
// list through `GLCodex::Import` (GLCodex.cpp:34-96), and parsing either is out
// of scope. What the rest of the tree needs is the *shape* of the lookup, so a
// future resource provider can satisfy it without anything above changing.

#include "progression/CodexDefinition.h"
#include "types/Result.h"

#include <vector>

namespace Modern
{
	// Read-only access to codex definitions.
	//
	// Returning a pointer rather than a copy keeps a definition shared, which is
	// what a definition is; the caller must not modify it.
	class CodexDefinitionProvider
	{
	public:
		virtual ~CodexDefinitionProvider() = default;

		// The definition for an id, or nullptr when this provider does not know
		// it. A missing definition is not an error the caller has to handle
		// differently from an unknown id, so the lookup reports absence.
		virtual const CodexDefinition* Find(CodexId id) const = 0;

		// Every definition, in ascending id order.
		//
		// This is not a convenience. Unlike item and skill definitions, a codex
		// table is walked as a whole: character load reconciles every entry
		// against every definition and seats what is missing
		// (GLCharDataCodex.cpp:72-93), and a definition that disappears has to be
		// noticed so its record can be dropped (GLCharDataCodex.cpp:95-132).
		// Neither is expressible as a sequence of `Find` calls over ids nobody
		// has enumerated.
		//
		// The order is specified because a provider that returned them in
		// registration order would let insertion order change what a
		// reconciliation produces, and reconciliation is supposed to be a pure
		// function of the definition set.
		virtual const std::vector<CodexDefinition>& GetAll() const = 0;
	};

	// A provider over a fixed set of definitions.
	//
	// This is the implementation for a milestone with no database: the tests and
	// any headless tool register the entries they need and everything behaves
	// exactly as it would against a real provider. It is a real type rather than
	// a test fixture, so a future server can hold one without the interface
	// changing.
	class InMemoryCodexDefinitions final : public CodexDefinitionProvider
	{
	public:
		// Adds or replaces a definition. A definition that is not valid is
		// refused and nothing changes.
		//
		// Adding is not reserved: the container grows, so a caller cannot
		// invalidate an existing definition by adding another.
		Status Add(const CodexDefinition& definition);

		// Removes a definition, which is how a test empties a provider without
		// rebuilding it. Returns NotFound when the id was not registered.
		Status Remove(CodexId id);

		const CodexDefinition* Find(CodexId id) const override;

		// The stored definitions, which Add keeps sorted by id.
		const std::vector<CodexDefinition>& GetAll() const override { return m_definitions; }

		size_t GetCount() const noexcept { return m_definitions.size(); }
		void   Clear() noexcept { m_definitions.clear(); }

	private:
		// Kept sorted by id so Find is a binary search and registration order
		// cannot change what a lookup returns.
		std::vector<CodexDefinition> m_definitions;
	};
}
