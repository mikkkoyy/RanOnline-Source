// WORLD-ENTRY-002h: headless rule tests for ResourceSyncService.
#include "TestHarness.h"

#include "world/ResourceSyncService.h"
#include "world/CharacterRepository.h"
#include "world/WorldCharacter.h"
#include "stats/StatCalculator.h"

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

using Modern::Server::World::ResourceSyncService;
using Modern::Server::World::InMemoryCharacterRepository;
using Modern::Server::World::WorldCharacter;
using Modern::Server::World::WorldCharacterId;
using Modern::Server::World::WorldAccountId;
using Modern::Network::WireU32;
using Modern::Resources::ResourceKind;
using Modern::Status;

namespace
{
    using Modern::Network::RanWire::NativeId;

    // Minimal valid character for registration.
    //
    // The account and user fields are populated because
    // `InMemoryCharacterRepository::Add` runs `WorldCharacter::Validate`, which
    // refuses a zero account id and an empty userId. `ResourceSyncService`
    // itself never reads either - but a character that cannot be stored is a
    // character whose write-back cannot be observed, so a fixture that omitted
    // them produced a repository that was silently empty rather than a service
    // that was wrong.
    WorldCharacter MakeCharacter(WireU32 gaeaId = 1,
                                  WireU32 charId = 10,
                                  const std::string& name = "TestChar",
                                  WireU32 hpNow = 500, WireU32 hpMax = 1000,
                                  WireU32 mpNow = 300, WireU32 mpMax = 800,
                                  WireU32 spNow = 200, WireU32 spMax = 600)
    {
        WorldCharacter c;
        c.gaeaId = gaeaId;
        c.id = WorldCharacterId{ charId };
        c.accountId = WorldAccountId{ charId };
        c.userId = "tester";
        c.name = name;
        c.hp = { hpNow, hpMax };
        c.mp = { mpNow, mpMax };
        c.sp = { spNow, spMax };
        c.actState = 0;
        c.savePosition = { 0, 0, 0 };
        c.saveMapId = NativeId{ 7u };
        // Level 1, because `Validate` refuses level 0 and the RAN tables start at
        // 1. A default-constructed character is therefore not a valid record, and
        // a fixture that left it alone produced a repository that silently
        // rejected every `Add`.
        c.level = 1;
        return c;
    }

    // RegisterSession REFUSES an empty sink, on purpose: a session that cannot
    // send is a wiring fault, and accepting one would turn that fault into a
    // silent no-op at the first recovery tick. So a case that is not about
    // delivery passes these rather than `{}` - which is what the service means
    // by "registered but silent".
    ResourceSyncService::SelfFrameSink   NoopSelf       = [](const auto&) {};
    ResourceSyncService::HpFrameSink     NoopHp         = [](const auto&) {};
    ResourceSyncService::WriteBackSink   NoopWriteBack  = [](const auto&, const auto&, const auto&, const auto&) {};
}

MODERN_TEST(ResourceSyncService_Register_AdoptsPartialPools_NotFull)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 47, 100, 0, 80, 50, 60);

    bool selfCalled = false, hpCalled = false, wbCalled = false;
    CHECK(svc.RegisterSession(c,
        [&](const auto&) { selfCalled = true; },
        [&](const auto&) { hpCalled = true; },
        [&](const auto& hp, const auto& mp, const auto& sp, const auto& cp) {
            wbCalled = true;
            CHECK_EQ(hp.now, 47u);
            CHECK_EQ(hp.max, 100u);
            CHECK_EQ(mp.now, 0u);
            CHECK_EQ(mp.max, 80u);
            CHECK_EQ(sp.now, 50u);
            CHECK_EQ(sp.max, 60u);
        }).IsOk());

    CHECK(!selfCalled);
    CHECK(!hpCalled);
    CHECK(!wbCalled);

    const auto state = svc.Find(1);
    REQUIRE(state.has_value());
    CHECK_EQ(state->GetCurrent(ResourceKind::Hp), 47u);
    CHECK_EQ(state->GetMaximum(ResourceKind::Hp), 100u);
    CHECK_EQ(state->GetCurrent(ResourceKind::Mp), 0u);
    CHECK_EQ(state->GetMaximum(ResourceKind::Mp), 80u);
    CHECK_EQ(state->GetCurrent(ResourceKind::Sp), 50u);
    CHECK_EQ(state->GetMaximum(ResourceKind::Sp), 60u);
}

MODERN_TEST(ResourceSyncService_Register_Refuses_GaeaIdZero)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(0, 10, "Test");
    CHECK(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsError());
}

MODERN_TEST(ResourceSyncService_Register_Refuses_DuplicateGaeaId)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test");
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());
    CHECK(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsError());
}

MODERN_TEST(ResourceSyncService_Unregister_RemovesSession)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test");
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());
    CHECK(svc.UnregisterSession(1).IsOk());
    CHECK(!svc.Find(1).has_value());
    CHECK_EQ(svc.SessionCount(), 0u);
}

MODERN_TEST(ResourceSyncService_Unregister_NotFound_ForMissing)
{
    ResourceSyncService svc;
    CHECK(svc.UnregisterSession(999).IsError());
}

MODERN_TEST(ResourceSyncService_Recovery_NoRecoveryWhenFull)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 1000, 1000, 800, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());

    svc.Advance(1.0f);

    const auto state = svc.Find(1);
    CHECK_EQ(state->GetCurrent(ResourceKind::Hp), 1000u);
    CHECK_EQ(state->GetCurrent(ResourceKind::Mp), 800u);
    CHECK_EQ(state->GetCurrent(ResourceKind::Sp), 600u);
}

MODERN_TEST(ResourceSyncService_Recovery_NoRecoveryWhenDead_HpZero)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 0, 1000, 400, 800, 300, 600);
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());

    svc.Advance(1.0f);

    const auto state = svc.Find(1);
    CHECK_EQ(state->GetCurrent(ResourceKind::Hp), 0u);
    CHECK_EQ(state->GetCurrent(ResourceKind::Mp), 400u);
    CHECK_EQ(state->GetCurrent(ResourceKind::Sp), 300u);
}

MODERN_TEST(ResourceSyncService_Recovery_HpRecovers_WithLegacyRate)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 100, 1000, 800, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());

    svc.Advance(1.0f);

    const auto state = svc.Find(1);
    CHECK_EQ(state->GetCurrent(ResourceKind::Hp), 103u);
}

MODERN_TEST(ResourceSyncService_Recovery_MpRecovers_WithLegacyRate)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 100, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());

    svc.Advance(1.0f);

    const auto state = svc.Find(1);
    CHECK_EQ(state->GetCurrent(ResourceKind::Mp), 102u);
}

MODERN_TEST(ResourceSyncService_Recovery_SpRecovers_WithLegacyRate)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 400, 800, 100, 600);
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());

    svc.Advance(1.0f);

    // 102, not 103, and the difference is load-bearing rather than a rounding
    // mistake in the test.
    //
    // The legacy rate is `0.5f * 0.01f`, and `0.005f` is not 0.005: it is
    // 0.004999999888241291 in IEEE single precision. Over a 600-point maximum
    // that is 2.99999993... points, and UPDATE_POINT truncates the accumulated
    // amount toward zero, so this tick pays out two whole points and keeps the
    // 0.99999... as remainder. The next tick collects it.
    //
    // Asserting 103 here would require rounding the rate up, which would make the
    // server disagree with legacy in the client's favour by one point - the exact
    // kind of "improvement" that desynchronises a client from its authority.
    const auto state = svc.Find(1);
    REQUIRE(state.has_value());
    CHECK_EQ(state->GetCurrent(ResourceKind::Sp), 102u);

    // The carried remainder is what keeps the deficit from accumulating. The
    // second tick collects the 0.99999... held over PLUS its own 2.99999..., so
    // it truncates to three whole points and pays 105 in total.
    //
    // The property that matters is the SUM, not the per-tick figure: after N
    // ticks the pool is within one point of N * 2.99999... however the
    // truncation fell. A pool that rounded every tick independently would drift
    // a point per tick and fall far behind legacy over a long session.
    svc.Advance(1.0f);
    const auto second = svc.Find(1);
    REQUIRE(second.has_value());
    CHECK_EQ(second->GetCurrent(ResourceKind::Sp), 105u);
}

MODERN_TEST(ResourceSyncService_Recovery_FractionalCarry)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 0, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());

    svc.Advance(1.0f);
    CHECK_EQ(svc.Current(1, ResourceKind::Mp), 2u);

    svc.Advance(1.0f);
    CHECK_EQ(svc.Current(1, ResourceKind::Mp), 4u);

    svc.Advance(1.0f);
    CHECK_EQ(svc.Current(1, ResourceKind::Mp), 7u);
}

MODERN_TEST(ResourceSyncService_Timer_FiresAfter1Point6Seconds_NotAt)
{
    ResourceSyncService svc;
    bool selfEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 100, 1000, 800, 800, 600, 600);
    CHECK(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) {},
        [&](const auto&, const auto&, const auto&, const auto&) {}).IsOk());

    svc.Advance(1.6f);
    CHECK(!selfEmitted);

    svc.Advance(0.01f);
    CHECK(selfEmitted);
}

MODERN_TEST(ResourceSyncService_Timer_ResetsToZero_NotSubtract)
{
    ResourceSyncService svc;
    int fireCount = 0;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 100, 1000, 800, 800, 600, 600);
    CHECK(svc.RegisterSession(c,
        [&](const auto&) { ++fireCount; },
        [&](const auto&) {},
        [&](const auto&, const auto&, const auto&, const auto&) {}).IsOk());

    svc.Advance(10.0f);
    CHECK_EQ(fireCount, 1);
}

MODERN_TEST(ResourceSyncService_Timer_NoTimerWhenDead)
{
    ResourceSyncService svc;
    bool selfEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 0, 1000, 800, 800, 600, 600);
    CHECK(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) {},
        [&](const auto&, const auto&, const auto&, const auto&) {}).IsOk());

    svc.Advance(10.0f);
    CHECK(!selfEmitted);
}

MODERN_TEST(ResourceSyncService_Timer_ZeroAndNegativeElapsedIgnored)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 100, 1000, 800, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());

    svc.Advance(0.0f);
    svc.Advance(-1.0f);
    svc.Advance(NAN);

    CHECK_EQ(svc.Current(1, ResourceKind::Hp), 100u);
}

MODERN_TEST(ResourceSyncService_Spend_MpSpend_Emits3046_Not3053)
{
    ResourceSyncService svc;
    bool selfEmitted = false, hpEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 400, 800, 600, 600);
    CHECK(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) { hpEmitted = true; },
        [&](const auto&, const auto&, const auto&, const auto&) {}).IsOk());

    WireU32 spent = 0;
    CHECK(svc.Spend(1, ResourceKind::Mp, 50, spent).IsOk());
    CHECK_EQ(spent, 50u);

    CHECK(selfEmitted);
    CHECK(!hpEmitted);
}

MODERN_TEST(ResourceSyncService_Spend_SpSpend_Emits3046_Not3053)
{
    ResourceSyncService svc;
    bool selfEmitted = false, hpEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 400, 800, 200, 600);
    CHECK(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) { hpEmitted = true; },
        [&](const auto&, const auto&, const auto&, const auto&) {}).IsOk());

    WireU32 spent = 0;
    CHECK(svc.Spend(1, ResourceKind::Sp, 30, spent).IsOk());
    CHECK_EQ(spent, 30u);

    CHECK(selfEmitted);
    CHECK(!hpEmitted);
}

MODERN_TEST(ResourceSyncService_Spend_HpSpend_EmitsBoth3046And3053)
{
    ResourceSyncService svc;
    bool selfEmitted = false, hpEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 400, 800, 600, 600);
    CHECK(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) { hpEmitted = true; },
        [&](const auto&, const auto&, const auto&, const auto&) {}).IsOk());

    WireU32 spent = 0;
    CHECK(svc.Spend(1, ResourceKind::Hp, 100, spent).IsOk());
    CHECK_EQ(spent, 100u);

    CHECK(selfEmitted);
    CHECK(hpEmitted);
}

MODERN_TEST(ResourceSyncService_Spend_SaturatesAtCurrent)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 30, 1000, 400, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());

    WireU32 spent = 0;
    CHECK(svc.Spend(1, ResourceKind::Hp, 100, spent).IsOk());
    CHECK_EQ(spent, 30u);
    CHECK_EQ(svc.Current(1, ResourceKind::Hp), 0u);
}

MODERN_TEST(ResourceSyncService_Spend_ZeroAmount_NoOp)
{
    ResourceSyncService svc;
    bool selfEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 400, 800, 600, 600);
    CHECK(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) {},
        [&](const auto&, const auto&, const auto&, const auto&) {}).IsOk());

    WireU32 spent = 0;
    CHECK(svc.Spend(1, ResourceKind::Hp, 0, spent).IsOk());
    CHECK_EQ(spent, 0u);
    CHECK(!selfEmitted);
}

MODERN_TEST(ResourceSyncService_Spend_NotFound_ReturnsError)
{
    ResourceSyncService svc;
    WireU32 spent = 0;
    CHECK(svc.Spend(999, ResourceKind::Hp, 10, spent).IsError());
}

MODERN_TEST(ResourceSyncService_Restore_Emits3046OnChange)
{
    ResourceSyncService svc;
    bool selfEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 400, 800, 600, 600);
    CHECK(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) {},
        [&](const auto&, const auto&, const auto&, const auto&) {}).IsOk());

    CHECK(svc.Restore(1, ResourceKind::Hp, 100).IsOk());
    CHECK(selfEmitted);
    CHECK_EQ(svc.Current(1, ResourceKind::Hp), 600u);
}

MODERN_TEST(ResourceSyncService_Restore_HpRestore_AlsoEmits3053)
{
    ResourceSyncService svc;
    bool hpEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 400, 800, 600, 600);
    CHECK(svc.RegisterSession(c,
        [&](const auto&) {},
        [&](const auto&) { hpEmitted = true; },
        [&](const auto&, const auto&, const auto&, const auto&) {}).IsOk());

    CHECK(svc.Restore(1, ResourceKind::Hp, 100).IsOk());
    CHECK(hpEmitted);
}

MODERN_TEST(ResourceSyncService_Restore_NoEmitWhenNoChange)
{
    ResourceSyncService svc;
    bool selfEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 1000, 1000, 800, 800, 600, 600);
    CHECK(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) {},
        [&](const auto&, const auto&, const auto&, const auto&) {}).IsOk());

    CHECK(svc.Restore(1, ResourceKind::Hp, 100).IsOk());
    CHECK(!selfEmitted);
}

MODERN_TEST(ResourceSyncService_ApplyDamage_EmitsBoth_AndSaturates)
{
    ResourceSyncService svc;
    bool selfEmitted = false, hpEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 400, 800, 600, 600);
    CHECK(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) { hpEmitted = true; },
        [&](const auto&, const auto&, const auto&, const auto&) {}).IsOk());

    WireU32 applied = svc.ApplyDamage(1, 100);
    CHECK_EQ(applied, 100u);
    CHECK(selfEmitted);
    CHECK(hpEmitted);
    CHECK_EQ(svc.Current(1, ResourceKind::Hp), 400u);

    applied = svc.ApplyDamage(1, 1000);
    CHECK_EQ(applied, 400u);
    CHECK_EQ(svc.Current(1, ResourceKind::Hp), 0u);
}

MODERN_TEST(ResourceSyncService_ApplyDamage_ReturnsZeroWhenAlreadyDead)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 0, 1000, 400, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());

    WireU32 applied = svc.ApplyDamage(1, 100);
    CHECK_EQ(applied, 0u);
}

MODERN_TEST(ResourceSyncService_ApplyDamage_NotFound_ReturnsZero)
{
    ResourceSyncService svc;
    CHECK_EQ(svc.ApplyDamage(999, 100), 0u);
}

MODERN_TEST(ResourceSyncService_WriteBack_UpdatesRepositoryPools)
{
    InMemoryCharacterRepository repo;
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 400, 800, 600, 600);
    repo.Add(c);

    std::tuple<WireU32, WireU32, WireU32, WireU32, WireU32, WireU32> lastWb;
    bool wbCalled = false;

    // The write-back sink REPLACES the record, exactly as
    // `FieldRoleRuntime::WriteBackPools` does. The service itself never touches
    // the repository - it hands the pools to the sink and that is the whole
    // contract - so a sink that merely recorded them left the repository
    // untouched and the assertions below could never have held. The test is
    // named for the repository being updated, so the sink must be the thing that
    // updates it.
    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) {},
        [&](const auto&) {},
        [&](const auto& hp, const auto& mp, const auto& sp, const auto& cp) {
            wbCalled = true;
            lastWb = { hp.now, hp.max, mp.now, mp.max, sp.now, sp.max };

            WorldCharacter updated = c;
            updated.hp = hp;
            updated.mp = mp;
            updated.sp = sp;
            updated.cp = cp;
            CHECK(repo.Replace(updated).IsOk());
        }).IsOk());

    WireU32 spent = 0;
    CHECK(svc.Spend(1, ResourceKind::Hp, 100, spent).IsOk());

    CHECK(wbCalled);
    CHECK_EQ(std::get<0>(lastWb), 400u);
    CHECK_EQ(std::get<1>(lastWb), 1000u);
    CHECK_EQ(std::get<2>(lastWb), 400u);
    CHECK_EQ(std::get<3>(lastWb), 800u);
    CHECK_EQ(std::get<4>(lastWb), 600u);
    CHECK_EQ(std::get<5>(lastWb), 600u);

    auto found = repo.Find(WorldCharacterId{ 10 });
    CHECK(found.IsOk());
    CHECK_EQ(found.GetValue().hp.now, 400u);
    CHECK_EQ(found.GetValue().hp.max, 1000u);
}

MODERN_TEST(ResourceSyncService_MultiSession_Isolated)
{
    ResourceSyncService svc;
    WorldCharacter c1 = MakeCharacter(1, 10, "One", 100, 1000, 800, 800, 600, 600);
    WorldCharacter c2 = MakeCharacter(2, 20, "Two", 500, 1000, 100, 800, 600, 600);
    CHECK(svc.RegisterSession(c1, NoopSelf, NoopHp, NoopWriteBack).IsOk());
    CHECK(svc.RegisterSession(c2, NoopSelf, NoopHp, NoopWriteBack).IsOk());

    svc.Advance(1.0f);

    CHECK_EQ(svc.Current(1, ResourceKind::Hp), 103u);
    CHECK_EQ(svc.Current(2, ResourceKind::Hp), 503u);
}

MODERN_TEST(ResourceSyncService_MpFloor_RecoversFromZero)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 0, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());

    svc.Advance(1.0f);
    CHECK_EQ(svc.Current(1, ResourceKind::Mp), 2u);
}

MODERN_TEST(ResourceSyncService_SessionCount_IncrementsAndDecrements)
{
    ResourceSyncService svc;
    CHECK_EQ(svc.SessionCount(), 0u);

    CHECK(svc.RegisterSession(MakeCharacter(1, 10, "A"), NoopSelf, NoopHp, NoopWriteBack).IsOk());
    CHECK_EQ(svc.SessionCount(), 1u);

    CHECK(svc.RegisterSession(MakeCharacter(2, 20, "B"), NoopSelf, NoopHp, NoopWriteBack).IsOk());
    CHECK_EQ(svc.SessionCount(), 2u);

    CHECK(svc.UnregisterSession(1).IsOk());
    CHECK_EQ(svc.SessionCount(), 1u);

    CHECK(svc.UnregisterSession(2).IsOk());
    CHECK_EQ(svc.SessionCount(), 0u);
}

// =========================================================================
// SKILL-027: strict atomic multi-resource spending (TrySpend) and the CP pool.
// =========================================================================
//
// TrySpend is the deliberate opposite of Spend: Spend saturates (a short pool
// pays what it has and still reports Ok), while TrySpend compares FIRST and
// deducts only when every nonzero cost is covered - inside one lock hold, so
// no interleaving operation can invalidate the verdict. A refusal touches
// nothing and emits nothing; there is no rollback because there is nothing
// to roll back.
//
// The comparison operators are legacy's, reproduced exactly
// (GLogixExPC.cpp:4240-4261): HP refuses at equality (`<=`), MP/SP/CP pass
// through it (`<`). The boundary tests below pin that asymmetry, because
// getting it backwards is the most likely regression here.

// A character with a CP pool: MakeCharacter leaves cp at {0,0}, so tests
// that spend CP set it explicitly. Seeding through the record - rather than
// poking the service state - is what keeps these honest about the
// registration path.
WorldCharacter MakeCharacterWithCp(WireU32 gaeaId, WireU32 charId,
                                   WireU32 hpNow, WireU32 hpMax,
                                   WireU32 mpNow, WireU32 mpMax,
                                   WireU32 spNow, WireU32 spMax,
                                   WireU32 cpNow, WireU32 cpMax)
{
    WorldCharacter c = MakeCharacter(gaeaId, charId, "Test", hpNow, hpMax,
                                     mpNow, mpMax, spNow, spMax);
    c.cp = { cpNow, cpMax };
    return c;
}

using ResourceCost = ResourceSyncService::ResourceCost;

MODERN_TEST(ResourceSyncService_TrySpend_SufficientFunds_DeductsExactAmounts)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacterWithCp(1, 10,
        500, 1000, 400, 800, 600, 600, 50, 200);
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());

    CHECK(svc.TrySpend(1, ResourceCost{ 100, 50, 60, 10 }).IsOk());

    CHECK_EQ(svc.Current(1, ResourceKind::Hp), 400u);
    CHECK_EQ(svc.Current(1, ResourceKind::Mp), 350u);
    CHECK_EQ(svc.Current(1, ResourceKind::Sp), 540u);
    CHECK_EQ(svc.Current(1, ResourceKind::Cp), 40u);
    // Maxima untouched by spending.
    CHECK_EQ(svc.Find(1)->GetMaximum(ResourceKind::Cp), 200u);
}

MODERN_TEST(ResourceSyncService_TrySpend_OneUnitShortagePerPool_RefusesAll)
{
    // Each pool starved in turn while the other three are abundant: the
    // verdict must name the short pool by refusing, and the abundant pools
    // must not move.
    struct Shortage
    {
        ResourceCost cost;
        WireU32 hpNow, mpNow, spNow, cpNow;
    };
    const Shortage cases[] = {
        // HP one short: 99 < 100.
        { ResourceCost{ 100, 10, 10, 5 }, 99, 500, 500, 100 },
        // MP one short.
        { ResourceCost{ 10, 100, 10, 5 }, 500, 99, 500, 100 },
        // SP one short.
        { ResourceCost{ 10, 10, 100, 5 }, 500, 500, 99, 100 },
        // CP one short.
        { ResourceCost{ 10, 10, 10, 50 }, 500, 500, 500, 49 },
    };

    for (const Shortage& probe : cases)
    {
        ResourceSyncService svc;
        WorldCharacter c = MakeCharacterWithCp(1, 10,
            probe.hpNow, 1000, probe.mpNow, 800, probe.spNow, 600,
            probe.cpNow, 200);
        REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());

        const Status refused = svc.TrySpend(1, probe.cost);
        CHECK(refused.IsError());
        CHECK_EQ(refused.GetCode(), Modern::ErrorCode::NotAllowed);

        // Nothing moved - not the short pool, and not the abundant ones.
        CHECK_EQ(svc.Current(1, ResourceKind::Hp), probe.hpNow);
        CHECK_EQ(svc.Current(1, ResourceKind::Mp), probe.mpNow);
        CHECK_EQ(svc.Current(1, ResourceKind::Sp), probe.spNow);
        CHECK_EQ(svc.Current(1, ResourceKind::Cp), probe.cpNow);
    }
}

MODERN_TEST(ResourceSyncService_TrySpend_LegacyBoundary_HpEqualRefuses_OthersPass)
{
    // GLogixExPC.cpp:4240 uses `<=` for HP, so a pool exactly equal to the
    // cost is refused. :4241/:4258/:4261 use `<` for MP/SP/CP, so equality
    // there succeeds and empties the pool.
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacterWithCp(1, 10,
        100, 1000, 100, 800, 100, 600, 50, 200);
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());

    // HP equal to cost: refused, everything unchanged.
    CHECK(svc.TrySpend(1, ResourceCost{ 100, 0, 0, 0 }).IsError());
    CHECK_EQ(svc.Current(1, ResourceKind::Hp), 100u);

    // MP/SP/CP equal to cost: accepted, pools left at zero.
    CHECK(svc.TrySpend(1, ResourceCost{ 0, 100, 100, 50 }).IsOk());
    CHECK_EQ(svc.Current(1, ResourceKind::Hp), 100u);
    CHECK_EQ(svc.Current(1, ResourceKind::Mp), 0u);
    CHECK_EQ(svc.Current(1, ResourceKind::Sp), 0u);
    CHECK_EQ(svc.Current(1, ResourceKind::Cp), 0u);
}

MODERN_TEST(ResourceSyncService_TrySpend_JointAtomicity_CpShortLeavesOthersUnchanged)
{
    // The transaction is joint, not sequential: a CP shortage must not leave
    // behind HP/MP/SP deductions that "already happened".
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacterWithCp(1, 10,
        500, 1000, 400, 800, 600, 600, 10, 200);
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());

    CHECK(svc.TrySpend(1, ResourceCost{ 100, 50, 60, 50 }).IsError());

    CHECK_EQ(svc.Current(1, ResourceKind::Hp), 500u);
    CHECK_EQ(svc.Current(1, ResourceKind::Mp), 400u);
    CHECK_EQ(svc.Current(1, ResourceKind::Sp), 600u);
    CHECK_EQ(svc.Current(1, ResourceKind::Cp), 10u);
}

MODERN_TEST(ResourceSyncService_TrySpend_UnknownSession_ReturnsNotFound)
{
    ResourceSyncService svc;
    const Status missing = svc.TrySpend(999, ResourceCost{ 10, 10, 10, 10 });
    CHECK(missing.IsError());
    CHECK_EQ(missing.GetCode(), Modern::ErrorCode::NotFound);
}

MODERN_TEST(ResourceSyncService_TrySpend_AllZeroRequest_IsSilentNoOp)
{
    // Costs are unsigned, so a negative amount is unrepresentable rather
    // than refused - the type is the validation. The all-zero request is
    // the explicit edge: Ok(), no frames, no write-back, pools untouched.
    ResourceSyncService svc;
    int selfFrames = 0, hpFrames = 0, writeBacks = 0;
    WorldCharacter c = MakeCharacterWithCp(1, 10,
        500, 1000, 400, 800, 600, 600, 50, 200);
    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) { ++selfFrames; },
        [&](const auto&) { ++hpFrames; },
        [&](const auto&, const auto&, const auto&, const auto&) { ++writeBacks; })
        .IsOk());

    CHECK(svc.TrySpend(1, ResourceCost{ 0, 0, 0, 0 }).IsOk());

    CHECK_EQ(selfFrames, 0);
    CHECK_EQ(hpFrames, 0);
    CHECK_EQ(writeBacks, 0);
    CHECK_EQ(svc.Current(1, ResourceKind::Hp), 500u);
    CHECK_EQ(svc.Current(1, ResourceKind::Cp), 50u);
}

MODERN_TEST(ResourceSyncService_TrySpend_ZeroCostPool_NeverRefusesOnEmptyPool)
{
    // A zero cost names no shortage even when its pool is empty: only the
    // nonzero costs participate in the verdict.
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacterWithCp(1, 10,
        500, 1000, 0, 800, 0, 600, 0, 200);
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());

    CHECK(svc.TrySpend(1, ResourceCost{ 100, 0, 0, 0 }).IsOk());
    CHECK_EQ(svc.Current(1, ResourceKind::Hp), 400u);
    CHECK_EQ(svc.Current(1, ResourceKind::Mp), 0u);
    CHECK_EQ(svc.Current(1, ResourceKind::Sp), 0u);
    CHECK_EQ(svc.Current(1, ResourceKind::Cp), 0u);
}

MODERN_TEST(ResourceSyncService_TrySpend_Success_EmitsOneSelfFrame_OneWriteBack)
{
    // One transaction, one 3046, one write-back - never one per pool. The
    // HP cost additionally carries exactly one 3053.
    ResourceSyncService svc;
    int selfFrames = 0, hpFrames = 0, writeBacks = 0;
    Modern::Network::RanWire::DwPair wbHp{}, wbMp{}, wbSp{}, wbCp{};
    WorldCharacter c = MakeCharacterWithCp(1, 10,
        500, 1000, 400, 800, 600, 600, 50, 200);
    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) { ++selfFrames; },
        [&](const auto&) { ++hpFrames; },
        [&](const auto& hp, const auto& mp, const auto& sp, const auto& cp) {
            ++writeBacks;
            wbHp = hp; wbMp = mp; wbSp = sp; wbCp = cp;
        }).IsOk());

    CHECK(svc.TrySpend(1, ResourceCost{ 100, 50, 60, 10 }).IsOk());

    CHECK_EQ(selfFrames, 1);
    CHECK_EQ(hpFrames, 1);
    CHECK_EQ(writeBacks, 1);
    CHECK_EQ(wbHp.now, 400u);
    CHECK_EQ(wbMp.now, 350u);
    CHECK_EQ(wbSp.now, 540u);
    CHECK_EQ(wbCp.now, 40u);
}

MODERN_TEST(ResourceSyncService_TrySpend_Success_WithoutHpCost_EmitsNoBroadcast)
{
    // The 3053 rule is preserved: it fires if and only if HP was actually
    // deducted. An MP/SP/CP-only success still sends its single 3046 and
    // single write-back.
    ResourceSyncService svc;
    int selfFrames = 0, hpFrames = 0, writeBacks = 0;
    WorldCharacter c = MakeCharacterWithCp(1, 10,
        500, 1000, 400, 800, 600, 600, 50, 200);
    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) { ++selfFrames; },
        [&](const auto&) { ++hpFrames; },
        [&](const auto&, const auto&, const auto&, const auto&) { ++writeBacks; })
        .IsOk());

    CHECK(svc.TrySpend(1, ResourceCost{ 0, 50, 60, 10 }).IsOk());

    CHECK_EQ(selfFrames, 1);
    CHECK_EQ(hpFrames, 0);
    CHECK_EQ(writeBacks, 1);
    CHECK_EQ(svc.Current(1, ResourceKind::Hp), 500u);
    CHECK_EQ(svc.Current(1, ResourceKind::Cp), 40u);
}

MODERN_TEST(ResourceSyncService_TrySpend_Refusal_EmitsNothing_WritesBackNothing)
{
    // A refused transaction must be indistinguishable from no call at all:
    // same pools, same frame counts, same write-back count.
    ResourceSyncService svc;
    int selfFrames = 0, hpFrames = 0, writeBacks = 0;
    WorldCharacter c = MakeCharacterWithCp(1, 10,
        500, 1000, 400, 800, 600, 600, 10, 200);
    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) { ++selfFrames; },
        [&](const auto&) { ++hpFrames; },
        [&](const auto&, const auto&, const auto&, const auto&) { ++writeBacks; })
        .IsOk());

    CHECK(svc.TrySpend(1, ResourceCost{ 100, 50, 60, 50 }).IsError());

    CHECK_EQ(selfFrames, 0);
    CHECK_EQ(hpFrames, 0);
    CHECK_EQ(writeBacks, 0);
    CHECK_EQ(svc.Current(1, ResourceKind::Hp), 500u);
    CHECK_EQ(svc.Current(1, ResourceKind::Mp), 400u);
    CHECK_EQ(svc.Current(1, ResourceKind::Sp), 600u);
    CHECK_EQ(svc.Current(1, ResourceKind::Cp), 10u);
}

MODERN_TEST(ResourceSyncService_TrySpend_ConcurrentSpenders_CannotOverspend)
{
    // Two workers each spend 60 from a pool of 100. The mutex serializes the
    // two verdicts, so exactly one wins: final 40, one Ok, one NotAllowed -
    // regardless of scheduling. No sleeps: both threads join before any
    // assertion.
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacterWithCp(1, 10,
        100, 1000, 500, 800, 500, 600, 500, 800);
    REQUIRE(svc.RegisterSession(c, NoopSelf, NoopHp, NoopWriteBack).IsOk());

    std::atomic<int> okCount{ 0 };
    std::atomic<int> refusedCount{ 0 };

    auto spender = [&] {
        const Status result = svc.TrySpend(1, ResourceCost{ 60, 0, 0, 0 });
        if (result.IsOk())
        {
            ++okCount;
        }
        else
        {
            ++refusedCount;
        }
    };

    std::thread first(spender);
    std::thread second(spender);
    first.join();
    second.join();

    CHECK_EQ(okCount.load(), 1);
    CHECK_EQ(refusedCount.load(), 1);
    CHECK_EQ(svc.Current(1, ResourceKind::Hp), 40u);
}

MODERN_TEST(ResourceSyncService_Cp_RegistersSyncsAndWritesBack)
{
    // CP end to end through the existing paths: seeded from the character
    // record at registration, readable through Find/Current, carried on the
    // 3046 self frame, and written back to the repository on mutation.
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacterWithCp(1, 10,
        500, 1000, 400, 800, 600, 600, 50, 200);

    Modern::Network::RanWire::DwPair seenCp{ 0, 0 };
    int writeBacks = 0;
    REQUIRE(svc.RegisterSession(c,
        NoopSelf, NoopHp,
        [&](const auto&, const auto&, const auto&, const auto& cp) {
            ++writeBacks;
            seenCp = cp;
        }).IsOk());

    // Seeded, not zeroed: the record's values are the pool's values.
    const auto state = svc.Find(1);
    REQUIRE(state.has_value());
    CHECK_EQ(state->GetCurrent(ResourceKind::Cp), 50u);
    CHECK_EQ(state->GetMaximum(ResourceKind::Cp), 200u);
    CHECK_EQ(svc.Current(1, ResourceKind::Cp), 50u);

    // A CP-only spend still emits the single 3046 (captured here) with the
    // live CP pair, and no 3053 (no HP deducted). The write-back is captured
    // on THIS session - the one that actually spends - not on `svc` above.
    std::vector<Modern::Network::WireU8> selfFrame;
    bool hpEmitted = false;
    int tickedWriteBacks = 0;
    Modern::Network::RanWire::DwPair tickedCp{ 0, 0 };
    ResourceSyncService ticker;
    WorldCharacter ticked = MakeCharacterWithCp(2, 20,
        500, 1000, 400, 800, 600, 600, 50, 200);
    REQUIRE(ticker.RegisterSession(ticked,
        [&](const auto& frame) { selfFrame = frame; },
        [&](const auto&) { hpEmitted = true; },
        [&](const auto&, const auto&, const auto&, const auto& cp) {
            ++tickedWriteBacks;
            tickedCp = cp;
        }).IsOk());
    CHECK(ticker.TrySpend(2, ResourceCost{ 0, 0, 0, 20 }).IsOk());
    REQUIRE(!selfFrame.empty());
    CHECK(!hpEmitted);

    Modern::Network::UpdateState::StateUpdate decoded;
    REQUIRE(Modern::Network::UpdateState::UpdateStateCodec::DecodeStateUpdate(
        selfFrame, decoded).IsOk());
    CHECK_EQ(decoded.cp.now, 30u);
    CHECK_EQ(decoded.cp.max, 200u);
    CHECK_EQ(decoded.hp.now, 500u);

    // The write-back carried CP to the repository exactly once. Asserted
    // against the spending session's own sink: `svc` above never spent, so
    // its counters must stay at zero - which is itself the proof that an
    // idle session emits nothing.
    CHECK_EQ(writeBacks, 0);
    CHECK_EQ(tickedWriteBacks, 1);
    CHECK_EQ(tickedCp.now, 30u);
    CHECK_EQ(tickedCp.max, 200u);
}
