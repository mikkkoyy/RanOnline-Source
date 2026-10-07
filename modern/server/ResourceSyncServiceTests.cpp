// WORLD-ENTRY-002h: headless rule tests for ResourceSyncService.

#include "TestHarness.h"
#include "world/ResourceSyncService.h"
#include "world/CharacterRepository.h"
#include "world/WorldCharacter.h"
#include "stats/StatCalculator.h"

using Modern::Server::World::ResourceSyncService;
using Modern::Server::World::InMemoryCharacterRepository;
using Modern::Server::World::WorldCharacter;
using Modern::Server::World::WorldCharacterId;
using Modern::Network::WireU32;
using Modern::Resources::ResourceKind;
using Modern::Stats::RecoveryRateConstant;

namespace
{
    using Modern::Network::RanWire::NativeId;

    // Minimal valid character for registration.
    WorldCharacter MakeCharacter(WireU32 gaeaId = 1,
                                  WireU32 charId = 10,
                                  const std::string& name = "TestChar",
                                  WireU32 hpNow = 500, WireU32 hpMax = 1000,
                                  WireU32 mpNow = 300, WireU32 mpMax = 800,
                                  WireU32 spNow = 200, WireU32 spMax = 600)
    {
        WorldCharacter c;
        c.gaeaId = gaeaId;
        c.id = WorldCharacterId(charId);
        c.name = name;
        c.hp = { hpNow, hpMax };
        c.mp = { mpNow, mpMax };
        c.sp = { spNow, spMax };
        c.actState = 0;
        c.savePosition = { 0, 0, 0 };
        c.saveMapId = NativeId(7u);
        return c;
    }
}

MODERN_TEST(ResourceSyncService_Register_AdoptsPartialPools_NotFull)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 47, 100, 0, 80, 50, 60);

    bool selfCalled = false, hpCalled = false, wbCalled = false;
    CHECK(svc.RegisterSession(c,
        [&](const auto&) { selfCalled = true; },
        [&](const auto&) { hpCalled = true; },
        [&](const auto& hp, const auto& mp, const auto& sp) {
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

    const auto* state = svc.Find(1);
    REQUIRE(state != nullptr);
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
    CHECK(svc.RegisterSession(c, {}, {}, {}).IsError());
}

MODERN_TEST(ResourceSyncService_Register_Refuses_DuplicateGaeaId)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test");
    REQUIRE(svc.RegisterSession(c, {}, {}, {}).IsOk());
    CHECK(svc.RegisterSession(c, {}, {}, {}).IsError());
}

MODERN_TEST(ResourceSyncService_Unregister_RemovesSession)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test");
    REQUIRE(svc.RegisterSession(c, {}, {}, {}).IsOk());
    REQUIRE(svc.UnregisterSession(1).IsOk());
    CHECK(svc.Find(1) == nullptr);
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
    REQUIRE(svc.RegisterSession(c, {}, {}, {}).IsOk());

    svc.Advance(1.0f);

    const auto* state = svc.Find(1);
    CHECK_EQ(state->GetCurrent(ResourceKind::Hp), 1000u);
    CHECK_EQ(state->GetCurrent(ResourceKind::Mp), 800u);
    CHECK_EQ(state->GetCurrent(ResourceKind::Sp), 600u);
}

MODERN_TEST(ResourceSyncService_Recovery_NoRecoveryWhenDead_HpZero)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 0, 1000, 400, 800, 300, 600);
    REQUIRE(svc.RegisterSession(c, {}, {}, {}).IsOk());

    svc.Advance(1.0f);

    const auto* state = svc.Find(1);
    CHECK_EQ(state->GetCurrent(ResourceKind::Hp), 0u);
    CHECK_EQ(state->GetCurrent(ResourceKind::Mp), 400u);
    CHECK_EQ(state->GetCurrent(ResourceKind::Sp), 300u);
}

MODERN_TEST(ResourceSyncService_Recovery_HpRecovers_WithLegacyRate)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 100, 1000, 800, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c, {}, {}, {}).IsOk());

    svc.Advance(1.0f);

    const auto* state = svc.Find(1);
    CHECK_EQ(state->GetCurrent(ResourceKind::Hp), 103u);
}

MODERN_TEST(ResourceSyncService_Recovery_MpRecovers_WithLegacyRate)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 100, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c, {}, {}, {}).IsOk());

    svc.Advance(1.0f);

    const auto* state = svc.Find(1);
    CHECK_EQ(state->GetCurrent(ResourceKind::Mp), 102u);
}

MODERN_TEST(ResourceSyncService_Recovery_SpRecovers_WithLegacyRate)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 400, 800, 100, 600);
    REQUIRE(svc.RegisterSession(c, {}, {}, {}).IsOk());

    svc.Advance(1.0f);

    const auto* state = svc.Find(1);
    CHECK_EQ(state->GetCurrent(ResourceKind::Sp), 103u);
}

MODERN_TEST(ResourceSyncService_Recovery_FractionalCarry)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 0, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c, {}, {}, {}).IsOk());

    svc.Advance(1.0f);
    CHECK_EQ(svc.Find(1)->GetCurrent(ResourceKind::Mp), 2u);

    svc.Advance(1.0f);
    CHECK_EQ(svc.Find(1)->GetCurrent(ResourceKind::Mp), 4u);

    svc.Advance(1.0f);
    CHECK_EQ(svc.Find(1)->GetCurrent(ResourceKind::Mp), 7u);
}

MODERN_TEST(ResourceSyncService_Timer_FiresAfter1Point6Seconds_NotAt)
{
    ResourceSyncService svc;
    bool selfEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 100, 1000, 800, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) {},
        [&](const auto&, const auto&, const auto&) {}).IsOk());

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
    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) { ++fireCount; },
        [&](const auto&) {},
        [&](const auto&, const auto&, const auto&) {}).IsOk());

    svc.Advance(10.0f);
    CHECK_EQ(fireCount, 1);
}

MODERN_TEST(ResourceSyncService_Timer_NoTimerWhenDead)
{
    ResourceSyncService svc;
    bool selfEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 0, 1000, 800, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) {},
        [&](const auto&, const auto&, const auto&) {}).IsOk());

    svc.Advance(10.0f);
    CHECK(!selfEmitted);
}

MODERN_TEST(ResourceSyncService_Timer_ZeroAndNegativeElapsedIgnored)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 100, 1000, 800, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c, {}, {}, {}).IsOk());

    svc.Advance(0.0f);
    svc.Advance(-1.0f);
    svc.Advance(NAN);

    CHECK_EQ(svc.Find(1)->GetCurrent(ResourceKind::Hp), 100u);
}

MODERN_TEST(ResourceSyncService_Spend_MpSpend_Emits3046_Not3053)
{
    ResourceSyncService svc;
    bool selfEmitted = false, hpEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 400, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) { hpEmitted = true; },
        [&](const auto&, const auto&, const auto&) {}).IsOk());

    WireU32 spent = 0;
    REQUIRE(svc.Spend(1, ResourceKind::Mp, 50, spent).IsOk());
    CHECK_EQ(spent, 50u);

    CHECK(selfEmitted);
    CHECK(!hpEmitted);
}

MODERN_TEST(ResourceSyncService_Spend_SpSpend_Emits3046_Not3053)
{
    ResourceSyncService svc;
    bool selfEmitted = false, hpEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 400, 800, 200, 600);
    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) { hpEmitted = true; },
        [&](const auto&, const auto&, const auto&) {}).IsOk());

    WireU32 spent = 0;
    REQUIRE(svc.Spend(1, ResourceKind::Sp, 30, spent).IsOk());
    CHECK_EQ(spent, 30u);

    CHECK(selfEmitted);
    CHECK(!hpEmitted);
}

MODERN_TEST(ResourceSyncService_Spend_HpSpend_EmitsBoth3046And3053)
{
    ResourceSyncService svc;
    bool selfEmitted = false, hpEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 400, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) { hpEmitted = true; },
        [&](const auto&, const auto&, const auto&) {}).IsOk());

    WireU32 spent = 0;
    REQUIRE(svc.Spend(1, ResourceKind::Hp, 100, spent).IsOk());
    CHECK_EQ(spent, 100u);

    CHECK(selfEmitted);
    CHECK(hpEmitted);
}

MODERN_TEST(ResourceSyncService_Spend_SaturatesAtCurrent)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 30, 1000, 400, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c, {}, {}, {}).IsOk());

    WireU32 spent = 0;
    REQUIRE(svc.Spend(1, ResourceKind::Hp, 100, spent).IsOk());
    CHECK_EQ(spent, 30u);
    CHECK_EQ(svc.Find(1)->GetCurrent(ResourceKind::Hp), 0u);
}

MODERN_TEST(ResourceSyncService_Spend_ZeroAmount_NoOp)
{
    ResourceSyncService svc;
    bool selfEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 400, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) {},
        [&](const auto&, const auto&, const auto&) {}).IsOk());

    WireU32 spent = 0;
    REQUIRE(svc.Spend(1, ResourceKind::Hp, 0, spent).IsOk());
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
    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) {},
        [&](const auto&, const auto&, const auto&) {}).IsOk());

    REQUIRE(svc.Restore(1, ResourceKind::Hp, 100).IsOk());
    CHECK(selfEmitted);
    CHECK_EQ(svc.Find(1)->GetCurrent(ResourceKind::Hp), 600u);
}

MODERN_TEST(ResourceSyncService_Restore_HpRestore_AlsoEmits3053)
{
    ResourceSyncService svc;
    bool hpEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 400, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) {},
        [&](const auto&) { hpEmitted = true; },
        [&](const auto&, const auto&, const auto&) {}).IsOk());

    REQUIRE(svc.Restore(1, ResourceKind::Hp, 100).IsOk());
    CHECK(hpEmitted);
}

MODERN_TEST(ResourceSyncService_Restore_NoEmitWhenNoChange)
{
    ResourceSyncService svc;
    bool selfEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 1000, 1000, 800, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) {},
        [&](const auto&, const auto&, const auto&) {}).IsOk());

    REQUIRE(svc.Restore(1, ResourceKind::Hp, 100).IsOk());
    CHECK(!selfEmitted);
}

MODERN_TEST(ResourceSyncService_ApplyDamage_EmitsBoth_AndSaturates)
{
    ResourceSyncService svc;
    bool selfEmitted = false, hpEmitted = false;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 400, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) { selfEmitted = true; },
        [&](const auto&) { hpEmitted = true; },
        [&](const auto&, const auto&, const auto&) {}).IsOk());

    WireU32 applied = svc.ApplyDamage(1, 100);
    CHECK_EQ(applied, 100u);
    CHECK(selfEmitted);
    CHECK(hpEmitted);
    CHECK_EQ(svc.Find(1)->GetCurrent(ResourceKind::Hp), 400u);

    applied = svc.ApplyDamage(1, 1000);
    CHECK_EQ(applied, 400u);
    CHECK_EQ(svc.Find(1)->GetCurrent(ResourceKind::Hp), 0u);
}

MODERN_TEST(ResourceSyncService_ApplyDamage_ReturnsZeroWhenAlreadyDead)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 0, 1000, 400, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c, {}, {}, {}).IsOk());

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

    REQUIRE(svc.RegisterSession(c,
        [&](const auto&) {},
        [&](const auto&) {},
        [&](const auto& hp, const auto& mp, const auto& sp) {
            wbCalled = true;
            lastWb = { hp.now, hp.max, mp.now, mp.max, sp.now, sp.max };
        }).IsOk());

    WireU32 spent = 0;
    REQUIRE(svc.Spend(1, ResourceKind::Hp, 100, spent).IsOk());

    REQUIRE(wbCalled);
    CHECK_EQ(std::get<0>(lastWb), 400u);
    CHECK_EQ(std::get<1>(lastWb), 1000u);
    CHECK_EQ(std::get<2>(lastWb), 400u);
    CHECK_EQ(std::get<3>(lastWb), 800u);
    CHECK_EQ(std::get<4>(lastWb), 600u);
    CHECK_EQ(std::get<5>(lastWb), 600u);

    auto found = repo.Find(WorldCharacterId(10));
    REQUIRE(found.IsOk());
    CHECK_EQ(found.GetValue().hp.now, 400u);
    CHECK_EQ(found.GetValue().hp.max, 1000u);
}

MODERN_TEST(ResourceSyncService_MultiSession_Isolated)
{
    ResourceSyncService svc;
    WorldCharacter c1 = MakeCharacter(1, 10, "One", 100, 1000, 800, 800, 600, 600);
    WorldCharacter c2 = MakeCharacter(2, 20, "Two", 500, 1000, 100, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c1, {}, {}, {}).IsOk());
    REQUIRE(svc.RegisterSession(c2, {}, {}, {}).IsOk());

    svc.Advance(1.0f);

    CHECK_EQ(svc.Find(1)->GetCurrent(ResourceKind::Hp), 103u);
    CHECK_EQ(svc.Find(2)->GetCurrent(ResourceKind::Hp), 503u);
}

MODERN_TEST(ResourceSyncService_MpFloor_RecoversFromZero)
{
    ResourceSyncService svc;
    WorldCharacter c = MakeCharacter(1, 10, "Test", 500, 1000, 0, 800, 600, 600);
    REQUIRE(svc.RegisterSession(c, {}, {}, {}).IsOk());

    svc.Advance(1.0f);
    CHECK_EQ(svc.Find(1)->GetCurrent(ResourceKind::Mp), 2u);
}

MODERN_TEST(ResourceSyncService_SessionCount_IncrementsAndDecrements)
{
    ResourceSyncService svc;
    CHECK_EQ(svc.SessionCount(), 0u);

    REQUIRE(svc.RegisterSession(MakeCharacter(1, 10, "A"), {}, {}, {}).IsOk());
    CHECK_EQ(svc.SessionCount(), 1u);

    REQUIRE(svc.RegisterSession(MakeCharacter(2, 20, "B"), {}, {}, {}).IsOk());
    CHECK_EQ(svc.SessionCount(), 2u);

    REQUIRE(svc.UnregisterSession(1).IsOk());
    CHECK_EQ(svc.SessionCount(), 1u);

    REQUIRE(svc.UnregisterSession(2).IsOk());
    CHECK_EQ(svc.SessionCount(), 0u);
}