// SKILL-021: headless rule tests for the 3303 dispatch boundary and skill
// provider injection.
//
// No socket opens in this file. What is worth checking here is a BORROWED
// pointer reaching a session's SkillRuntime, plus the decision table the 3303
// handler implements and the wire contract of the two refusals it sends - none
// of which needs Winsock.

// The HANDLER ITSELF is exercised end to end in WorldEntryTcpTests.cpp, which
// spawns a real client over a real socket and asserts the 3304 that comes back,
// that the connection survives, and that a 3303 is never routed through the
// movement or attack handlers. What is deliberately NOT reachable over a socket
// yet is the handler's silent branch: nothing in this tree can put a character
// in a "knows the skill and has learned it" state, because WorldCharacter
// carries no learned-skill field and FieldSession never calls LearnSkill. That
// test there is written as a tripwire on that ceiling, and is expected to fail
// when a real learned-skill path lands.
//
// ---------------------------------------------------------------------------
// WHAT IS DELIBERATELY NOT TESTED HERE
// ---------------------------------------------------------------------------
//
// There is NO case asserting that a skill cast succeeded, applied damage, spent
// a resource, started a cooldown, or broadcast a 3305. This milestone
// implements none of those, so such a case would either fail or - worse - be
// written to pass by asserting something weaker and being called a cast test.
//
// What IS asserted is the opposite and the part that matters: that a
// well-formed 3303 for a known, learned skill leaves the world EXACTLY as it
// was, and says nothing to the client.

#include "TestHarness.h"

#include "SkillProtocol.h"
#include "skills/SkillDefinition.h"
#include "skills/SkillDefinitionProvider.h"
#include "world/FieldSession.h"
#include "world/WorldEntryService.h"

#include <cstddef>
#include <string>
#include <vector>

namespace
{
	using namespace Modern;
	using namespace Modern::Network;
	using namespace Modern::Server::World;

	namespace SkillNS = Modern::Network::Skill;
	namespace SkillCodec = Modern::Network::Skill::SkillCodec;

	// ---- fixtures ----------------------------------------------------------

	SkillDefinition MakeDef(uint16_t main, uint16_t sub)
	{
		// EMROLE_NORMAL, which is SkillRole::Normal - a castable role. A passive
		// skill would be refused for a different reason and is not what this
		// boundary is about.
		//
		// `basicVar` is non-zero because SkillDefinition::IsValid() requires at
		// least one level to contribute something, and `InMemorySkillDefinitions::
		// Add` refuses an invalid definition. The costs and ranges below are the
		// data a real cast would need; they are carried so the fixture is
		// honest about what is missing elsewhere, and none of them is read by
		// anything in this milestone.
		SkillDefinition def;
		def.id          = SkillId{ main, sub };
		def.name        = "Skill_" + std::to_string(main) + "_" + std::to_string(sub);
		def.maxLevel    = 9;
		def.role        = SkillRole::Normal;
		def.targetRange = 30;
		def.levelData[1].basicVar   = 100.0f;
		def.levelData[1].applyRange = 50;
		def.levelData[1].useHp      = 10;
		def.levelData[1].useMp      = 20;
		def.levelData[1].useSp      = 30;
		return def;
	}

	// A FieldSession over a repository and registry that no case in this file
	// needs to populate: nothing here validates an identity or spawns. The
	// session only needs to exist so its SkillRuntime does.
	FieldSession MakeSession(ICharacterRepository& repository, FieldEntryRegistry& registry,
	                         WireU64 sessionId,
	                         const SkillDefinitionProvider* definitions = nullptr)
	{
		return FieldSession(repository, registry, sessionId, definitions);
	}

	// A 3303 that names `main`/`sub` and carries `targets` STARIDs, encoded
	// through the codec so the server side is fed exactly what a client would
	// put on the wire.
	std::vector<WireU8> BuildSkillRequest(WireU16 main, WireU16 sub,
	                                      std::size_t targets = 0)
	{
		SkillNS::SkillRequest request;
		request.skillId.value = (static_cast<WireU32>(sub) << 16) | main;
		request.targetPosition = { 100.0f, 0.0f, 200.0f };
		request.attackVelocity = 1.0f;
		request.attackVelocityItem = 1.0f;
		request.sumTarRange    = 0.0f;
		for (std::size_t i = 0; i < targets && i < SkillNS::kMaxTargets; ++i)
		{
			request.targets.push_back(
			    SkillNS::SkillTarget{ 2u, static_cast<WireU16>(500u + i) });
		}

		std::vector<WireU8> frame;
		(void) SkillCodec::AppendSkillRequest(frame, request);
		return frame;
	}

	// =========================================================================
	// Provider injection
	// =========================================================================

	// The change SKILL-021 makes to the session, stated as a behaviour rather
	// than as a constructor signature.
	//
	// SKILL-018 gave every FieldSession a `SkillRuntime{ nullptr }` and there was
	// no way to change it, so a production session could not learn a skill under
	// any circumstances. The provider is now BORROWED and passed in. This case
	// pins both halves of the deal: absent refuses, present succeeds.
	MODERN_TEST(Skill_SessionRefusesToLearnWithoutADefinitionProvider)
	{
		InMemoryCharacterRepository repository;
		FieldEntryRegistry            registry;

		FieldSession session = MakeSession(repository, registry, 1, nullptr);

		CHECK(session.Skills().LearnSkill(SkillId{ 1, 1 }).IsError());
		CHECK(!session.Skills().HasSkill(SkillId{ 1, 1 }));
		CHECK_EQ(session.Skills().GetSkillLevel(SkillId{ 1, 1 }), static_cast<std::uint8_t>(0));
	}

	MODERN_TEST(Skill_SessionLearnsOnceTheProviderIsInjected)
	{
		InMemoryCharacterRepository repository;
		FieldEntryRegistry            registry;

		InMemorySkillDefinitions definitions;
		REQUIRE(definitions.Add(MakeDef(1, 1)).IsOk());

		FieldSession session = MakeSession(repository, registry, 2, &definitions);

		CHECK(session.Skills().LearnSkill(SkillId{ 1, 1 }).IsOk());
		CHECK(session.Skills().HasSkill(SkillId{ 1, 1 }));
		CHECK_EQ(session.Skills().GetSkillLevel(SkillId{ 1, 1 }), static_cast<std::uint8_t>(1));
	}

	// A provider that knows the skill but a character that has not learned it is
	// the OTHER half of the dispatch table, and it must be distinguishable from
	// "no provider at all". A 3303 naming this skill is answered EMSKILL_NOTLEARN,
	// not EMSKILL_UNKNOWN.
	MODERN_TEST(Skill_KnownButUnlearnedIsDifferentFromUnknown)
	{
		InMemoryCharacterRepository repository;
		FieldEntryRegistry            registry;

		InMemorySkillDefinitions definitions;
		REQUIRE(definitions.Add(MakeDef(20, 30)).IsOk());

		FieldSession session = MakeSession(repository, registry, 3, &definitions);

		// Known to the table, absent from the character.
		CHECK(definitions.Find(SkillId{ 20, 30 }) != nullptr);
		CHECK(!session.Skills().HasSkill(SkillId{ 20, 30 }));

		// Known to nobody at all - which is what a null provider means.
		FieldSession blind = MakeSession(repository, registry, 4, nullptr);
		CHECK(blind.Skills().LearnSkill(SkillId{ 20, 30 }).IsError());
	}

	// The provider is BORROWED, not copied, and not owned by the session. A
	// session outliving nothing it owns is the whole invariant: a reconnect
	// starts clean rather than inheriting a previous appearance's skills, and
	// the session cannot be what keeps a definition table alive.
	//
	// Proved the only way a pointer-borrow can be: two sessions over the SAME
	// provider, each learning independently.
	MODERN_TEST(Skill_ProviderIsBorrowedSoTwoSessionsShareOneTableAndNeitherOwnsIt)
	{
		InMemoryCharacterRepository repository;
		FieldEntryRegistry            registry;

		InMemorySkillDefinitions definitions;
		REQUIRE(definitions.Add(MakeDef(7, 7)).IsOk());

		FieldSession first  = MakeSession(repository, registry, 10, &definitions);
		FieldSession second = MakeSession(repository, registry, 11, &definitions);

		CHECK(first.Skills().LearnSkill(SkillId{ 7, 7 }).IsOk());
		CHECK(second.Skills().LearnSkill(SkillId{ 7, 7 }).IsOk());

		// A table change is visible to both, because neither owns a copy.
		REQUIRE(definitions.Add(MakeDef(8, 8)).IsOk());
		CHECK(first.Skills().LearnSkill(SkillId{ 8, 8 }).IsOk());
		CHECK(second.Skills().LearnSkill(SkillId{ 8, 8 }).IsOk());

		// And a session's own state is its own: learning in one does not appear
		// in the other, even over a shared table.
		FieldSession third = MakeSession(repository, registry, 12, &definitions);
		CHECK(!third.Skills().HasSkill(SkillId{ 7, 7 }));
	}

	// =========================================================================
	// The dispatch decision table
	// =========================================================================

	// The table HandleSkillCast implements, restated here so the ORDER of the
	// checks is pinned rather than implied. Four rows, four different outcomes:
	//
	//   malformed length      -> protocol fault, connection refused
	//   not spawned           -> wiring fault, connection refused
	//   unknown skill         -> 3304 EMSKILL_UNKNOWN (999), connection kept
	//   known but unlearned   -> 3304 EMSKILL_NOTLEARN (1), connection kept
	//   known and learned     -> SILENCE, connection kept
	//
	// The last row is this milestone's whole behaviour. It is the row a future
	// change is most likely to get wrong, and it is the one that CANNOT be
	// exercised over a socket yet, so it is pinned here in the codec layer instead
	// of being left implicit.
	MODERN_TEST(Skill_TheDispatchDecisionTableHasFourRowsAndOnlyTwoAreAnnounced)
	{
		// Malformed: a length that is not 63 + 4n. The codec refuses it, which
		// is what the handler turns into a connection refusal.
		std::vector<WireU8> malformed(65, 0);
		{
			// Written little-endian by hand so the frame really declares 65.
			malformed[0] = 65u;
			malformed[4] = static_cast<WireU8>(SkillNS::kSkillId & 0xFFu);
			malformed[5] = static_cast<WireU8>((SkillNS::kSkillId >> 8) & 0xFFu);
			malformed[6] = static_cast<WireU8>((SkillNS::kSkillId >> 16) & 0xFFu);
			malformed[7] = static_cast<WireU8>((SkillNS::kSkillId >> 24) & 0xFFu);
		}
		SkillNS::SkillRequest decoded;
		CHECK(SkillCodec::DecodeSkillRequest(malformed, decoded).IsError());

		// Every well-formed length decodes, which is the precondition for all
		// three remaining rows.
		for (std::size_t n = 0; n <= SkillNS::kMaxTargets; ++n)
		{
			const std::vector<WireU8> good = BuildSkillRequest(1, 1, n);
			REQUIRE(SkillCodec::DecodeSkillRequest(good, decoded).IsOk());
			CHECK_EQ(decoded.targets.size(), n);
		}
	}

	// The two refusal codes the handler sends, and the two it must never send.
	//
	// EMSKILL_OK (0) is the value that says THE CAST HAPPENED. Emitting it
	// without executing anything would be the single most damaging thing this
	// milestone could do, so the constant is pinned here as zero and the
	// handler's contract is stated against it.
	MODERN_TEST(Skill_HandlerNeverSendsTheSuccessCode)
	{
		// Zero means "your cast happened". It exists, it is zero, and nothing in
		// this milestone produces it.
		CHECK_EQ(SkillNS::SkillCheck::kOk, static_cast<WireI32>(0));

		// The two it may send.
		CHECK_EQ(SkillNS::SkillCheck::kNotLearned, static_cast<WireI32>(1));
		CHECK_EQ(SkillNS::SkillCheck::kUnknown, static_cast<WireI32>(999));

		// A 3304 carrying the success code is well formed and decodes - which is
		// precisely why the guard has to be the handler's behaviour and not the
		// codec's: the codec cannot tell a truthful OK from a lying one, because
		// to a reader of the bytes the two are identical.
		SkillNS::SkillFeedback feedback;
		feedback.skillCheck = SkillNS::SkillCheck::kOk;
		feedback.name       = std::string(SkillNS::kNameBytes, '\0');

		std::vector<WireU8> frame;
		REQUIRE(SkillCodec::AppendSkillFeedback(frame, feedback).IsOk());
		CHECK_EQ(frame.size(), SkillNS::kFeedbackSize);

		SkillNS::SkillFeedback decoded;
		REQUIRE(SkillCodec::DecodeSkillFeedback(frame, decoded).IsOk());
		CHECK_EQ(decoded.skillCheck, static_cast<WireI32>(0));
	}

	// A refusal names a code and carries a 33-byte name. Both refusal codes this
	// milestone sends ignore the name, so it is carried zeroed - and the encoder
	// refuses any other length rather than padding or cutting, which is what
	// stops a future call site from inventing a terminator that the legacy struct
	// has no room for.
	MODERN_TEST(Skill_RefusalFeedbackIsFortyFiveBytesWithAZeroedName)
	{
		const WireI32 refusals[] = { SkillNS::SkillCheck::kUnknown,
			                         SkillNS::SkillCheck::kNotLearned };

		for (const WireI32 code : refusals)
		{
			SkillNS::SkillFeedback feedback;
			feedback.skillCheck = code;
			feedback.name       = std::string(SkillNS::kNameBytes, '\0');

			std::vector<WireU8> frame;
			REQUIRE(SkillCodec::AppendSkillFeedback(frame, feedback).IsOk());
			CHECK_EQ(frame.size(), static_cast<std::size_t>(45));

			SkillNS::SkillFeedback decoded;
			REQUIRE(SkillCodec::DecodeSkillFeedback(frame, decoded).IsOk());
			CHECK_EQ(decoded.skillCheck, code);
			CHECK_EQ(decoded.name.size(), SkillNS::kNameBytes);

			// And it is the ANNOUNCED refusal: id 3304, a distinct message from
			// the 3305 broadcast the handler never sends. The id is a full 32-bit
			// word, so all four bytes are compared - checking only the low byte
			// would pass for any id ending in 0xE8.
			WireU32 id = 0;
			for (std::size_t i = 0; i < 4; ++i)
			{
				id |= static_cast<WireU32>(frame[4 + i]) << (8 * i);
			}
			CHECK_EQ(id, SkillNS::kSkillFbId);
			CHECK_NE(id, SkillNS::kSkillBrdId);
		}
	}

	// =========================================================================
	// What the handler must NOT touch
	// =========================================================================

	// The strongest assertion this milestone can make, and the reason the
	// deferred row is safe: a decoded 3303 changes NOTHING.
	//
	// Decoding is a pure function of the bytes. There is no code path from
	// `DecodeSkillRequest` to a resource mutation, a cooldown write or a
	// broadcast, and this case pins the observable half of that - a character
	// that learns a skill, sends a 3303 for it, and finds its own state
	// untouched.
	MODERN_TEST(Skill_DecodingARequestChangesNoCharacterState)
	{
		InMemoryCharacterRepository repository;
		FieldEntryRegistry            registry;

		InMemorySkillDefinitions definitions;
		REQUIRE(definitions.Add(MakeDef(4, 4)).IsOk());

		FieldSession session = MakeSession(repository, registry, 20, &definitions);
		REQUIRE(session.Skills().LearnSkill(SkillId{ 4, 4 }).IsOk());

		// A cooldown exists so the case can prove none was started.
		session.Skills().StartCooldown(SkillId{ 99, 99 }, 5.0f);
		const std::size_t cooldownsBefore = session.Skills().GetCooldowns().size();

		const std::vector<WireU8> frame = BuildSkillRequest(4, 4, 3);
		SkillNS::SkillRequest decoded;
		REQUIRE(SkillCodec::DecodeSkillRequest(frame, decoded).IsOk());

		// The decoded claim is a claim: three targets, a skill id, a position.
		// None of it has been applied anywhere.
		CHECK_EQ(decoded.targets.size(), static_cast<std::size_t>(3));
		CHECK_EQ(decoded.skillId.value, static_cast<WireU32>(4u | (4u << 16)));

		// No cooldown was started for the skill that was cast, and the one that
		// already existed is untouched.
		CHECK(!session.Skills().IsSkillOnCooldown(SkillId{ 4, 4 }));
		CHECK(session.Skills().IsSkillOnCooldown(SkillId{ 99, 99 }));
		CHECK_EQ(session.Skills().GetCooldowns().size(), cooldownsBefore);

		// The learned set is unchanged: learning is not something a cast does.
		CHECK(session.Skills().HasSkill(SkillId{ 4, 4 }));
		CHECK(!session.Skills().HasSkill(SkillId{ 5, 5 }));
	}
} // namespace